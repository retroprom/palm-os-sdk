/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtChat.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#define DO_NOT_ALLOW_ACCESS_TO_INTERNALS_OF_STRUCTS // To ensure future OS compatibility

#include <AboutBox.h>
#include <Chars.h>
#include <Control.h>
#include <Font.h>
#include <CmnLaunchCodes.h>
#include <DataMgr.h>
#include <ErrorMgr.h>
#include <Event.h>
#include <FeatureMgr.h>
#include <Form.h>
#include <IOS.h>
#include <Loader.h>
#include <MemoryMgr.h>
#include <Menu.h>
#include <PenInputMgr.h>
#include <PollBox.h>
#include <ScrollBar.h>
#include <StdIO.h>
#include <string.h>
#include <StringMgr.h>
#include <SoundMgr.h>
#include <SystemMgr.h>
#include <SysUtils.h>
#include <Table.h>
#include <TraceMgr.h>
#include <TimeMgr.h>
#include <UIResources.h>


#include <BtLib.h>
#include "BtChatRsc.h"

/*---------------------------------------+
 | Private global constants, types, data |
 +---------------------------------------*/

// Macro to avoid compiler warning about unused arguments
//
#define UNUSED_ARG(x) (x)

// Some macros to manage byte ordering
//
#if CPU_ENDIAN == CPU_ENDIAN_BIG
#	define NToHS(x)	(x)					// convert network to host int16_t
#	define HToNS(x)	(x)					// convert host to network int16_t
#elif CPU_ENDIAN == CPU_ENDIAN_LITTLE
#	define NToHS(x)	_BtLibNetSwap16(x)	// convert network to host int16_t
#	define HToNS(x)	_BtLibNetSwap16(x)	// convert host to network int16_t
#else
#	error "No CPU_ENDIAN defined"
#endif

// Trace module id.
//
#define TRACE_MODULE (appErrorClass+5)

// Etc.
//
#define kAppFileCreator			'BtCh'
#define kChatServiceName		"Chat"
#define kMaxSizeConversation	4096
#define kMaxSizeSentence		128
#define kInitialCredits			4
#define kBdAddrStringSize		18

// Service class UUID for this app: 83E087AE-4E18-46be-83E0-7B3DE6A1C33C
//
static BtLibSdpUuidType gChatUUID = {
	btLibUuidSize128, 
	{0x83,0xe0,0x87,0xae,0x4e,0x18,0x46,0xbe,0x83,0xe0,0x7b,0x3d,0xe6,0xa1,0xc3,0x3c}
};

// Class-of-device for this app, for filtering during device discovery.
//
static BtLibClassOfDeviceType gChatClassOfDevice = {
	(btLibCOD_ServiceAny | btLibCOD_Major_Computer | btLibCOD_Minor_Comp_Palm)
};

// Major states of the application.
//
typedef enum
{
	kStateIdle,					// Initial state
	kStateInitiatorConnect, 	// Sent request, waiting for the acceptor confirmation
	kStateInitiatorChat,		// Chatting, and was the initiator
	kStateInitiatorDisconnect,	// Local disconnection, waiting for ACL link drop
	kStateAcceptorInit, 		// Received connection, waiting for UI update event
	kStateAcceptorChat,			// Chatting, and was the acceptor
	kStateAcceptorDisconnect	// Local disconnection, waiting for ACL link drop
}
ChatStateEnum;

static Boolean					gStopFlag = false;	// application stop flag
static Boolean					gService = false;	// application running as service in background
static ChatStateEnum			gState;				// major state of the application
static uint16_t					gAIAState;			// state of active graffiti input are upon entry
static DmOpenRef				gDbResId;			// id of this app's resource database
static BtLibDeviceAddressType	gBtAddress;			// remote device's bluetooth address
static PollBox* 				gPollBox;			// a collection of file descriptors, for polling.
static int16_t					gPrevFormId;		// current form upon entry when server
static int32_t					gFdUI = -1;			// fd to UI events
static int32_t					gFdME = -1;			// fd to Management Entity device
static int32_t		 			gFdSdp = -1;		// fd to SDP socket
static int32_t 					gFdListen = -1;		// fd to RfComm listener socket
static int32_t 					gFdData = -1;		// fd to RfComm data socket
static DmOpenRef 			gBtChatDmOpenRef;

/*-------------------------------+
 | Private function declarations |
 +-------------------------------*/

static void HandlePbxSocketEvent(struct PollBox* pbx, struct pollfd* pollFd, void* context);
static status_t	InitiatorDeinit(void);
static status_t	InitiatorClose(void);
static status_t	AcceptorDeinit(void);
static void BeginChat(void);
static void EndChat(Boolean isByMe);
static void AddToConversation(const char *messP, uint8_t type);

/*-------------------+
 | Private functions |
 +-------------------*/

//------------------------------------------------------------------------------
// Handle events from the Management Entity device.
//
static void HandlePbxMeEvent(
	struct PollBox*					pbx,	// the PollBox
	struct pollfd*					pollFd,	// fd, event mask, and received events
	void*							context	// not used here
) {
	status_t						error;
	int32_t 						flags;

	static BtLibManagementEventType mEvent;
	static char 					mData[sizeof(BtLibFriendlyNameType)];
	static struct strbuf			ctlBuf = { sizeof(mEvent), 0, (char*)&mEvent };
	static struct strbuf			datBuf = { sizeof(mData),  0, (char*)&mData[0] };

	// We must be here for a reason ...
	DbgOnlyFatalErrorIf( !(pollFd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL) ), "no event flag" );

	// We must have the Management Entity file descriptor.
	DbgOnlyFatalErrorIf( pollFd->fd != gFdME, "not the FdMe" );
	DbgOnlyFatalErrorIf( pollFd->fd < 0, "gFdMe closed" );

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if (
		(pollFd->revents & (POLLERR | POLLHUP | POLLNVAL)) ||			// poll error or hangup
		IOSGetmsg( pollFd->fd, &ctlBuf, &datBuf, &flags, &error ) != 0	// getmsg error
	) {
		TraceOutput(TL( TRACE_MODULE, "BtChat: HandlePbxMeEvent: closing fd %ld", pollFd->fd ));
		BtLibClose( pollFd->fd ); 	
		PbxRemoveFd( pbx, pollFd->fd );
		gFdME = -1;
		return;
	}

	// We must have an event struct in the control part.
	DbgOnlyFatalErrorIf( ctlBuf.len != sizeof(BtLibManagementEventType), "no event struct" );

	TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxMEEvent: event %s, status 0x%hx", BtLibMEEventName(mEvent.event), mEvent.status));

	switch( mEvent.event )
	{
	case btLibManagementEventRadioState:
		{
			const char* s;

			switch ( mEvent.status )
			{
			case btLibErrNoError:
			case btLibErrRadioInitialized:	s = "HandlePbxMeEvent: Radio Initialized";	break;
			case btLibErrRadioInitFailed:	s = "HandlePbxMeEvent: Radio Init Failed";	break;
			case btLibErrRadioFatal:		s = "HandlePbxMeEvent: Radio Fatal Error";	break;
			case btLibErrRadioSleepWake:	s = "HandlePbxMeEvent: Radio Wakeup";		break;
			default:						s = "HandlePbxMeEvent: Radio Event 0x%lx";	break;
			}
			TraceOutput(TL( TRACE_MODULE, s, mEvent.status));
		}
		break;	

	case btLibManagementEventACLConnectInbound:
		// Just memorise the peer BT address
		memmove(&gBtAddress, &mEvent.eventData.bdAddr, sizeof(BtLibDeviceAddressType));
		break;

	case btLibManagementEventACLConnectOutbound:
		if (mEvent.status)
		{
			// Outbound ACL connection failed.
			FrmAlert(gDbResId, NoConnectionAlert);
			gState = kStateIdle;
		}
		else
		{
			// Outbound ACL connection succeeded.
			// Create an SDP socket to query the RfComm channel of chat service
			error = BtLibSocketCreate( &gFdSdp, btLibSdpProtocol );
			ErrFatalErrorIf( error, "can't create SDP socket" );
			error = PbxAddFd( gPollBox, gFdSdp, POLLIN, HandlePbxSocketEvent, &gFdSdp );
			ErrFatalErrorIf( error, "can't add SDP socket to pollbox" );
			error = BtLibSdpGetServerChannelByUuid( gFdSdp, &gBtAddress, &gChatUUID, 1 );
			ErrFatalErrorIf( error != btLibErrPending, "BtLibSdpGetServerChannelByUuid() failed" );
		}
		break;
			
	case btLibManagementEventACLDisconnect:
		// RfComm socket is already closed if the local device initiates the disconnection
		// and is still open if the remote device closes its socket and ACL disconnect
		// Close the RfComm socket in case
		switch (gState)
		{
		case kStateInitiatorConnect:
				FrmAlert(gDbResId, ChatDeclinedAlert);
				break;
		case kStateInitiatorChat:
				InitiatorDeinit();
				break;
		case kStateAcceptorChat:
				AcceptorDeinit();
				break;
			// In disconnecting state, Deinit already called and RfComm socket already closed
		}
		gState = kStateIdle;
		break;
	}
}

//------------------------------------------------------------------------------
// Handle RfComm and SDP events.
// Common to Initiator and Acceptor side
//
static void HandlePbxSocketEvent(
	struct PollBox*				pbx,	// the PollBox
	struct pollfd*				pollFd,	// fd, event mask, and received events
	void*						context	// in this app, ptr to global fd variable
) {
	status_t					error;
	int32_t 					flags;

	static BtLibSocketEventType sEvent;
	static char 				sData[kMaxSizeSentence];
	static struct strbuf		ctlBuf = { sizeof(sEvent), 0, (char*)&sEvent	 };
	static struct strbuf		datBuf = { sizeof(sData),  0, (char*)&sData[0] };

	TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: fd=%lx, *context=%lx", pollFd->fd, context ? *((int32_t*)context) : 0));

	// We must be here for a reason ...
	DbgOnlyFatalErrorIf( !(pollFd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL) ), "no event flag" );

	// Our context variable is just the global variable containing the file descriptor.
	DbgOnlyFatalErrorIf( context != &gFdData && context != &gFdListen && context != &gFdSdp, "bad context" );
	DbgOnlyFatalErrorIf( pollFd->fd != *((int32_t*)context), "context not fd" );
	DbgOnlyFatalErrorIf( pollFd->fd < 0, "bad fd" );

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if (
		(pollFd->revents & (POLLERR | POLLHUP | POLLNVAL)) ||			// poll error or hangup
		IOSGetmsg( pollFd->fd, &ctlBuf, &datBuf, &flags, &error ) != 0	// getmsg error
	) {
		TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: closing fd %ld", pollFd->fd));
		PbxRemoveFd( pbx, pollFd->fd );
		IOSClose( pollFd->fd );
		*((int32_t*)context) = -1;
		return;
	}

	TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: fd=%lx, ctlBuf.len=%hd, datBuf.len=%hd", pollFd->fd, ctlBuf.len, datBuf.len));

	// Check for pure data.
	if ( ctlBuf.len == -1 )
	{
		TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: Data:"));
		TraceOutput(B(TRACE_MODULE, (uint8_t*)datBuf.buf, (uint32_t)datBuf.len));
		// Received a sentence, display in in the conversation field
		AddToConversation((char*)datBuf.buf, 2);
		// Advance one more credit.
		BtLibSocketAdvanceCredit( pollFd->fd, 1 );
		return;
	}

	// Since it's not pure data, we must have an event struct in the control part.
	ErrFatalErrorIf( ctlBuf.len != sizeof(BtLibSocketEventType), "no event struct" );
	TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: event %s, status 0x%lx", BtLibSocketEventName(sEvent.event), sEvent.status));

	switch( sEvent.event )
	{
	/*------------+
	 | SDP events |
	 +------------*/

	case btLibSocketEventSdpGetServerChannelByUuid:
	{
		BtLibSocketConnectInfoType	connectInfo;
		BtLibRfCommServerIdType		rfCommChannel;
		
		// If acceptor does not have the Bt Chat service, end gracefully
		if ( sEvent.status != 0 )
		{
			TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: SdpGetAcceptorChannelByUuid: No remote chat service"));
			FrmAlert(gDbResId, NoChatServiceAlert);
			InitiatorClose();
			return;
		}

		// Now get the RfComm channel, close the SDP socket, and open the RfComm socket

		rfCommChannel = sEvent.eventData.sdpByUuid.param.channel;

		ErrFatalErrorIf( gFdSdp < 0, "no sdp fd" );
		PbxRemoveFd( gPollBox, gFdSdp );
		BtLibSocketClose( gFdSdp );
		gFdSdp = -1;
		
		error = BtLibSocketCreate( &gFdData, btLibRfCommProtocol );
		ErrFatalErrorIf( error, "can't create RfComm data socket" );
		error = PbxAddFd( gPollBox, gFdData, POLLIN, HandlePbxSocketEvent, &gFdData );
		ErrFatalErrorIf( error, "can't add RfComm data socket to pollbox" );

		connectInfo.remoteDeviceP = &gBtAddress;
		connectInfo.data.RfComm.remoteService = rfCommChannel;
		connectInfo.data.RfComm.maxFrameSize = kMaxSizeSentence; 
		connectInfo.data.RfComm.advancedCredit = kInitialCredits;
		TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: SdpGetAcceptorChannelByUuid: requesting maxFrameSize=%hu", (uint16_t)kMaxSizeSentence));
		error = BtLibSocketConnect( gFdData, &connectInfo );
		ErrFatalErrorIf( error && error != btLibErrPending, "BtLibSocketConnect() failed");
		gState = kStateInitiatorConnect;
		return;
	}
	  
	/*---------------+
	 | RfComm events |
	 +---------------*/

	case btLibSocketEventConnectedOutbound:
		// The acceptor has accepted. Chat begins.
		ErrFatalErrorIf( sEvent.status, "outbound RfComm connect failed" );
		TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: ConnectedOutbound: Socket %d", pollFd->fd));
		gState = kStateInitiatorChat;
		BeginChat();
		return;

	case btLibSocketEventDisconnected:
		TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: Disconnected: Socket %d", sEvent.eventData.newSocket));
		if (gState == kStateInitiatorConnect) 
		{
			// The Acceptor has declined the chat invitation, close the ACL link
			BtLibLinkDisconnect( gFdME, &gBtAddress );
			TraceOutput(TL(TRACE_MODULE, "BtChat: HandlePbxSocketEvent: Disconnected: Chat invitation declined"));
		}
		return;
	}
}

//------------------------------------------------------------------------------
// Initiator-side initialization.
// Discover remote device, and request ACL link establishment to it.
//
static status_t InitiatorInit( void )
{
	status_t		err;

	TraceOutput(TL(TRACE_MODULE, "BtChat: InitiatorInit"));

	if ( (err = BtLibDiscoverDevices( gFdME, NULL, NULL, false, &gChatClassOfDevice, 1, false, &gBtAddress, 1, NULL )) != 0 ) {
		return err;
	}

	if ( (err = BtLibLinkConnect( gFdME, &gBtAddress )) != 0 && err != btLibErrPending ) {
		FrmAlert(gDbResId, NoConnectionAlert);
		return err;
	}

	return 0;
}

//------------------------------------------------------------------------------
// Initiator-side de-initialization.
// Close the RfComm data socket and end the chat.
//
static status_t InitiatorDeinit( void )
{
	Boolean			isByMe = (gState == kStateInitiatorDisconnect);

	TraceOutput(TL(TRACE_MODULE, "BtChat: InitiatorDeinit: closing %s", isByMe ? "by me" : "by peer"));

	if ( gFdData >= 0 ) {
		PbxRemoveFd( gPollBox, gFdData );
		BtLibSocketClose( gFdData );
		gFdData = -1;
	}

	EndChat( isByMe );

	return 0;
}

//------------------------------------------------------------------------------
//
static status_t InitiatorClose(void)
{
	gState = kStateInitiatorDisconnect;
	InitiatorDeinit();
	BtLibLinkDisconnect( gFdME, &gBtAddress );
	return 0;
}

//------------------------------------------------------------------------------
// Acceptor-side de-nitialization.
// Stop advertising service, close opened sockets and end the chat.
//
static status_t AcceptorDeinit(void)
{
	// Closing after a chat
	TraceOutput(TL(TRACE_MODULE, "BtChat: AcceptorDeinit: closing %s", gState == kStateAcceptorDisconnect ? "by me" : "by peer"));
	PbxRemoveFd( gPollBox, gFdData );
	BtLibSocketClose( gFdData );
	EndChat((Boolean)(gState == kStateAcceptorDisconnect));
	return 0;
}

//------------------------------------------------------------------------------
//
static status_t AcceptorClose(void)
{
	gState = kStateAcceptorDisconnect;
	(void)AcceptorDeinit();
	BtLibLinkDisconnect( gFdME, &gBtAddress );
	return 0;
} 

//------------------------------------------------------------------------------
// Show/Hide buttons and fields to enter chat mode.
//
static void BeginChat( void )
{
	status_t	error;
	FormPtr		frmP = FrmGetFormPtr(MainForm);
	char		sentence[kMaxSizeSentence];
	static char	prefix[] = "Connected to ";

	FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainStartButton));	
	FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSendButton));
	FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainStopButton));
	FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSentenceField));
	FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSentenceLabel));
	FrmSetFocus  (frmP, FrmGetObjectIndex(frmP, MainSentenceField));
	strcpy( sentence, prefix );

	// Get the peer's device name.
	error = BtLibGetRemoteDeviceNameSynchronous( gFdME, &gBtAddress, btLibCachedOnly, &sentence[strlen(sentence)], kMaxSizeSentence - strlen(sentence) );
	if ( error || strlen(sentence) == strlen(prefix) ) {
		TraceOutput(TL(blthErrorClass, "BtChat: BeginChat: ERROR 0x%lx from BtLibGetRemoteDeviceNameSynchronous()", error));
		BtLibAddrBtdToA( &gBtAddress, &sentence[strlen(sentence)], kMaxSizeSentence - strlen(sentence) );
	}

	AddToConversation(sentence, 0);
}

//------------------------------------------------------------------------------
// Show/Hide buttons and fields to enter idle mode.
//
static void EndChat(Boolean isByMe)
{
	FormPtr frmP = FrmGetFormPtr(MainForm);
	FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSendButton));
	FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainStopButton));
	FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSentenceField));
	FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSentenceLabel));
	FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainStartButton));	

	AddToConversation( isByMe ? "Disconnected locally" : "Disconnected by peer", 0 );

	if ( gService ) {
		gStopFlag = true;
	}
}

//------------------------------------------------------------------------------
// Erase sentence field.
//
static void ClearSentence(void)
{
	FormPtr		frmP = FrmGetFormPtr(MainForm);
	FieldPtr	fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainSentenceField));
	MemHandle	oldTextH = FldGetTextHandle(fldP);
	
	FldSetTextHandle(fldP, NULL);
	WinInvalidateWindow(FrmGetWindowHandle(FrmGetActiveForm()));
	if (oldTextH)
		MemHandleFree(oldTextH);
}

//------------------------------------------------------------------------------
// Update the scroll bar related to the sentence field.
//
static void UpdateScrollbar(void)
{
	uint32_t 			currentPosition;
	uint32_t 			textHeigth;
	uint32_t 			fieldHeigth;
	uint32_t			maxValue;
	FormPtr			frmP = FrmGetFormPtr(MainForm);
	ScrollBarPtr		scrollP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainConversationScrollBar));
	FieldPtr			fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainConversationField));
	
	FldGetScrollValues(fldP, &currentPosition, &textHeigth, &fieldHeigth);
	if (textHeigth > fieldHeigth)
		maxValue = textHeigth - fieldHeigth;
	else if (currentPosition)
		maxValue = currentPosition;
	else
		maxValue = 0;
		
	SclSetScrollBar(scrollP, (int16_t)currentPosition, 0, (int16_t)maxValue, (int16_t)(fieldHeigth - 1));
}

//------------------------------------------------------------------------------
// Scroll sentence field and update its scroll bar.
//
static void ScrollLines(uint32_t numLinesToScroll, WinDirectionType direction, Boolean update)
{
	FormPtr		frmP = FrmGetFormPtr(MainForm);
	FieldPtr		fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainConversationField));

	if (numLinesToScroll == 0)
		numLinesToScroll = FldGetVisibleLines(fldP) - 1;
	FldScrollField(fldP, numLinesToScroll, direction);
	if (update || (FldGetNumberOfBlankLines(fldP) && direction == winUp))
		UpdateScrollbar();
}

//------------------------------------------------------------------------------
// Add a text line to the conversation field.
//
static void AddToConversation(const char *messP, uint8_t type)
{
	static uint16_t 	gLenConv = 0;
	const char *	headP;
	uint16_t			lenMess;
	FormPtr			frmP = FrmGetFormPtr(MainForm);
	FieldPtr			fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainConversationField));
	MemHandle 	txtH = FldGetTextHandle(fldP);
	char *				txtP;

	// The first time, allocate the field with the maximum supported field size
	if (!txtH)
	{
		txtH = MemHandleNew(kMaxSizeConversation);
		txtP = MemHandleLock (txtH);
		*txtP = '\0';
	}
	else
		txtP = MemHandleLock (txtH);

	// When field does not have enough room, keep only last half of the conversation
	// Move the last half in replacement of first half
	lenMess = (uint16_t) strlen(messP) + 5;
	if (gLenConv + lenMess >= kMaxSizeConversation)
	{
		gLenConv >>= 1;
		memmove(txtP, txtP + gLenConv + (gLenConv & 1), gLenConv );
	}
	// Now add the sentence to the conversation
	switch (type)
	{
		case 0: headP = "***"; break;
		case 1: headP = "(L)"; break;
		case 2: headP = "(R)"; break;
		default:
			headP = "ERROR not define"; 
			break;
	}
	sprintf(txtP + gLenConv, "%s %s\n", headP, messP);
	gLenConv += lenMess;

	MemHandleUnlock (txtH);
	FldSetTextHandle (fldP, txtH);
	// And scroll up so that last lines are visible
	FldRecalculateField(fldP, false);
	FldScrollField(fldP, (uint16_t) kMaxSizeConversation, winDown);
	UpdateScrollbar();
	WinInvalidateWindow(FrmGetWindowHandle(FrmGetActiveForm()));
}

//------------------------------------------------------------------------------
// This is the event handler for the main form.
//
static Boolean MainFormHandler(EventPtr eventP)
{
	FormPtr	frmP = FrmGetFormPtr(MainForm);
	MemHandle	txtH;
	char *	txtP;
	FieldType *fldP;
	status_t		err;
	
	switch (eventP->eType) 
	{
	case menuEvent:
		switch (eventP->data.menu.itemID)
		{

		case MainOptionsStartanewchat:
			if ( gState == kStateIdle ) {
				InitiatorInit();
			} else {
				SndPlaySystemSound( sndWarning );
			}
			return true;
	
		case MainOptionsStopchatting:
			TraceOutput(TL(TRACE_MODULE, "BtChat: MainFormHandler: MainOptionsStopchatting"));
			switch (gState)
			{
			case kStateInitiatorChat:
				TraceOutput(TL (TRACE_MODULE, "BtChat: MainFormHandler: kStateInitiatorChat"));
				InitiatorClose();
				return true;
			case kStateAcceptorChat:
				TraceOutput(TL (TRACE_MODULE, "BtChat: MainFormHandler: kStateAcceptorChat"));
				(void)AcceptorClose();
				return true;
			default:
				TraceOutput(TL (TRACE_MODULE, "BtChat: MainFormHandler: default"));
				SndPlaySystemSound(sndWarning);
				return true;
			}

		case MainOptionsAboutBtChat:		
			MenuEraseStatus(0);
			AbtShowAbout(kAppFileCreator);
			return true;

		default:
			return false;				
		}
		
	case sclRepeatEvent:
	  {
		int32_t val = eventP->data.sclRepeat.newValue - eventP->data.sclRepeat.value;
		if (val > 0)
			ScrollLines((uint32_t)val,  winDown, false);
		else
			ScrollLines((uint32_t)-val, winUp,   false);
		return true;
	  }
		
	case keyDownEvent:
		switch (eventP->data.keyDown.chr)
		{
		case pageUpChr:
			ScrollLines(0, winUp, true);
			return true;
		case pageDownChr:
			ScrollLines(0, winDown, true);
			return true;
		default:
			return false;
		}		
	case winUpdateEvent:
		FrmDrawForm (frmP);
		return true;

	case frmOpenEvent:
		TraceOutput(TL(TRACE_MODULE,"BtChat: MainFormHandler: frmOpenEvent gState=%d", gState));
		UpdateScrollbar();
		if ( gState == kStateAcceptorInit ) {
			gState = kStateAcceptorChat;
			BeginChat();
		}
		WinInvalidateWindow(FrmGetWindowHandle(FrmGetActiveForm()));
		return true;

	case frmCloseEvent:
		TraceOutput(TL(TRACE_MODULE,"BtChat: MainFormHandler: frmCloseEvent"));
		fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainConversationField));
		txtH = FldGetTextHandle(fldP);
		if (txtH)
		{
			FldSetTextHandle (fldP, NULL);
			MemHandleFree (txtH);
		}
		return false;
		
	case ctlSelectEvent:
		switch (eventP->data.ctlSelect.controlID)
		{
		// Change Start/Stop button events in Start/Stop menu events
		case MainStartButton:
			eventP->eType = menuEvent;
			eventP->data.menu.itemID = MainOptionsStartanewchat;
			EvtAddEventToQueue(eventP);
			return true;

		case MainStopButton:
			eventP->eType = menuEvent;
			eventP->data.menu.itemID = MainOptionsStopchatting;
			EvtAddEventToQueue(eventP);
			return true;

		case MainSendButton:
			fldP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainSentenceField));
			txtH = FldGetTextHandle(fldP);
			if (txtH)
			{
				txtP = MemHandleLock (txtH);
				TraceOutput(TL(TRACE_MODULE, "BtChat: MainFormHandler: sending %d bytes", strlen(txtP)+1));
				err = BtLibSocketSend( gFdData, (uint8_t*)txtP, strlen(txtP)+1 );
				if ( err ) {
					TraceOutput(TL(TRACE_MODULE, "BtChat: MainFormHandler: ERROR 0x%lx sending %d bytes on socket %d", err, strlen(txtP)+1, gFdData));
				} else {
					// Sentence succesfully sent, echo it locally
					TraceOutput(TL(TRACE_MODULE, "BtChat: MainFormHandler: sent %d bytes on socket %d", strlen(txtP)+1, gFdData));
					AddToConversation((char*)txtP, 1);
					MemHandleUnlock(txtH);
					ClearSentence();
				}
			}
			else
			{
				SndPlaySystemSound(sndWarning);
			}
			return true;

		default:
			return false;				
		}
				
	default:
		return false;		
	}
}

//------------------------------------------------------------------------------
// Handle application events, after the system but before the forms.
//
static Boolean AppHandleEvent(EventPtr eventP)
{
	uint16_t	frmId;
	FormPtr	frmP;

	switch (eventP->eType)
	{
	case frmLoadEvent:
		TraceOutput(TL(TRACE_MODULE,"BtChat: AppHandleEvent: frmLoadEvent"));
		// Load the requested form resource.
		frmId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(gDbResId, frmId);
		FrmSetActiveForm(frmP);
		// And set its event
		if (frmId == MainForm)
			FrmSetEventHandler(frmP, MainFormHandler);
		return true;

	default:
		return false;
	}
}

//------------------------------------------------------------------------------
// Handle UI events.
//
static void HandleUIEvent( void )
{
	EventType		event;
	status_t		error;
	
	while ( EvtEventAvail() )
	{
		EvtGetEvent( &event, evtNoWait );

		if ( event.eType == appStopEvent ) {
			gStopFlag = true;
		}
		
		if (
			!SysHandleEvent( &event )				&&
			!MenuHandleEvent( 0, &event, &error )	&&
			!AppHandleEvent( &event )
		) {
			FrmDispatchEvent( &event );
		}
	}
}

//------------------------------------------------------------------------------
// The application event loop.
//
static void AppEventLoop(void)
{
	int32_t		nReady;
	status_t	err;
	
	while ( !gStopFlag )
	{
		HandleUIEvent();
		err = PbxPoll( gPollBox, 50, &nReady );
		ErrFatalErrorIf( err, "poll error" );
		ErrFatalErrorIf( gPollBox->count == 0, "empty pollbox" );
	}
}

//------------------------------------------------------------------------------
// Initialize the application.
//
static status_t AppStart(void)
{
	status_t		err = errNone;

	// Open our resource database.
	err = SysGetModuleDatabase( SysGetRefNum(), NULL, &gDbResId );
	if (!gDbResId)
	{
		TraceOutput(TL(TRACE_MODULE, "BtChat: AppStart: DmOpenDatabaseByTypeCreator() error=0x%lX", err));
		return err;
	}

	gPollBox = PbxCreate();
	if ( !gPollBox )
	{
		TraceOutput(TL(TRACE_MODULE, "BtChat: AppStart: PbxCreate() error Not enough memory"));
		err = memErrNotEnoughSpace;
		return err;
	}

	gFdUI = IOSOpen( "NULL", O_RDWR, &err );
	ErrFatalErrorIf( gFdUI < 0, "can't open NULL dev" );
	err = PbxAddFd( gPollBox, gFdUI, POLLIN, 0, 0 );
	if (err)
	{
		TraceOutput(TL(TRACE_MODULE, "BtChat: AppStart: PbxAddFd(fdUI) error=0x%lX", err));
		PbxDestroy( gPollBox );		
		return err; 	
	}
	
	err = BtLibOpen( &gFdME );
	if (err)
	{
		TraceOutput(TL(TRACE_MODULE, "BtChat: AppStart: BtLibOpen() error=0x%lX", err));
		FrmAlert(gDbResId, BtNotAvailableAlert);
		return err;
	}
	
	err = PbxAddFd( gPollBox, gFdME, POLLIN, HandlePbxMeEvent, 0 );
	if (err)
	{
		TraceOutput(TL(TRACE_MODULE, "BtChat: AppStart: PbxAddFd(fdME) error=0x%lX", err));
		BtLibClose( gFdME );
		PbxDestroy( gPollBox );		
		return err;
	}
	
	return err;
}

//------------------------------------------------------------------------------
// Terminate the application.
//
static void AppStop(void)
{
	if ( gPollBox != NULL ) {
		PbxDestroy( gPollBox );		// this closes all contained fds
	}
}

//------------------------------------------------------------------------------
// Get the type and creator of this application's resource database.
//
static status_t GetMyTypeAndCreator( 
	uint32_t*			typeP,
	uint32_t*			creatorP
) {
	DatabaseID			dbID;
	DmDatabaseInfoType	dbInfo;
	status_t			err;

	MemSet( &dbInfo, sizeof(dbInfo), 0 );
	dbInfo.pType    = typeP;
	dbInfo.pCreator = creatorP;
	if ( (err = SysGetModuleDatabase( SysGetRefNum(), &dbID, &gBtChatDmOpenRef )) == 0 ) {
		err = DmDatabaseInfo( dbID, &dbInfo );
	}
	return err;
}

//------------------------------------------------------------------------------
// Register ourselves as a Bluetooth service.
// This need be done only once after the system boots; redundant registrations
// are ignored. After registering a service, the Bluetooth system must be
// (re)started in order for that service to actually start functioning.
//
static void RegisterService( void )
{
	BtLibServiceRegistrationParamsType	params;
	status_t							err;

	TraceOutput(TL(TRACE_MODULE, "BtChat: RegisterService: ENTER"));

	if ( (err = GetMyTypeAndCreator( &params.appType, &params.appCreator )) == 0 ) {
		params.appCodeRscId	    = sysResIDDefault;
		params.stackSize        = 5000;
		params.protocol         = btLibRfCommProtocol;
		params.pushSerialModule = false;
		params.execAsNormalApp  = false;
		err = BtLibRegisterService( &params );
	}

	TraceOutput(TL(TRACE_MODULE, "BtChat: RegisterService: EXIT err=0x%lx", err));
}

//------------------------------------------------------------------------------
// Prepare our service.  Upon entry, we are given
//     params->serviceRecH ... handle on an empty local SDP service record
//     params->fdListener .... file descriptor to a bluetooth listener socket
// Upon exit, the service record must be set up to describe our service as being
// available through the listener socket. The caller of this function will then
// start advertising the service record.
//
static status_t PrepareService(
	BtLibServicePreparationParamsType*	params
) {
	status_t							err;

	TraceOutput(TL(TRACE_MODULE, "BtChat: PrepareService: ENTER"));

	err = BtLibSdpServiceRecordSetAttributesForSocket(
		params->fdListener,			// listener socket
		&gChatUUID,					// service class uuid list
		1,							// service class uuid list length
		kChatServiceName,			// service name
		strlen(kChatServiceName)+1,	// service name length including null
		params->serviceRecH			// the service record
	);

	TraceOutput(TL(TRACE_MODULE, "BtChat: PrepareService: EXIT err=0x%lx", err));
	return err;
}

//------------------------------------------------------------------------------
//
static void BackgroundUIInit( void )
{
	FormType*	formP;
	status_t	err;

	// Initialize the Autonomous Window.
	err = WinStartThreadUI();
	ErrFatalErrorIf( err, "can't initialize autonomous window" );

	// Get and save id of the current form.
	formP = FrmGetActiveForm();
	gPrevFormId = formP ? FrmGetFormId(formP) : 0;
	TraceOutput(TL(TRACE_MODULE, "BtChat: BackgroundUIInit: gPrevFormId=%d", gPrevFormId));

	// Close current menu.
	MenuEraseStatus(0);

	// Save current AIA state, and set it to 'open'.
	gAIAState = PINGetInputAreaState();
	if ( gAIAState != pinInputAreaOpen ) {
		PINSetInputAreaState( pinInputAreaOpen );
	}	

	// Display our main form.
	if ( gPrevFormId ) {
		FrmPopupForm( gDbResId, MainForm );
	} else {
		FrmGotoForm( gDbResId, MainForm );
	}
}


//------------------------------------------------------------------------------
//
static void BackgroundUITerm( void )
{
	FormType*	formP;
	status_t	err;

	// Restore AIA state.
	PINSetInputAreaState( gAIAState );

	// Restore form.
	if ( gPrevFormId ) {
		FrmReturnToForm( gPrevFormId );
	} else {
		formP = FrmGetFormPtr( MainForm );
		if ( formP && FrmValidatePtr( formP ) ) {
			FrmEraseForm( formP );
			FrmDeleteForm( formP );
		}
	}

	// Terminate the Autonomous Window.
	err = WinFinishThreadUI();
	ErrFatalErrorIf( err, "can't terminate autonomous window" );
}

//------------------------------------------------------------------------------
// Execute our service. The file descriptor params->fdData refers to a connected
// RfComm instance. The remote transmitter has zero flow-control credits.
// This function must close params->fdData before exiting.
//
static status_t ExecuteService(
	BtLibServiceExecutionParamsType*	params
) {
	status_t							err = 0;

	TraceOutput(TL(TRACE_MODULE, "BtChat: ExecuteService: ENTER fd=%d", params->fdData));

	if ( AppStart() == errNone ) {
		// Put the RfComm data file descriptor into the pollbox.
		gFdData = params->fdData;
		err = PbxAddFd( gPollBox, gFdData, POLLIN, HandlePbxSocketEvent, &gFdData );
		ErrFatalErrorIf( err, "can't add RfComm data socket to pollbox" );

		// Get the remote device's bluetooth address, for displaying later.
		BtLibSocketGetInfo( gFdData, btLibSocketInfo_RemoteDeviceAddress, &gBtAddress, sizeof(gBtAddress) );

		// Advance initial credit to the remote transmitter.
		BtLibSocketAdvanceCredit( gFdData, kInitialCredits ); 

		// Initialize UI from background.
		BackgroundUIInit();

		// Start chatting from acceptor side. 
		gState = kStateAcceptorInit;

		// Run the application event loop.
		AppEventLoop();

		// Terminate UI from background.
		BackgroundUITerm();
	}
	AppStop();

	TraceOutput(TL(TRACE_MODULE, "BtChat: ExecuteService: EXIT err=0x%lx", err));
	return 0;
}

/*------------------+
 | Main Entrypoint  |
 +------------------*/

uint32_t PilotMain(
	uint16_t							cmd,
	MemPtr								cmdPBP,
	uint16_t							launchFlags
) {
	uint32_t							retVal = errNone;
	BtLibServiceRegistrationParamsType	params; 	

	TraceOutput(TL(TRACE_MODULE,"BtChat: PilotMain: ENTER cmd=%hu", cmd));

	switch ( cmd )
	{
	case sysLaunchCmdBoot:
	case sysAppLaunchCmdSystemReset:
		// Register ourselves as a persistent Bluetooth service.
		RegisterService();
		break;

	case sysBtLaunchCmdPrepareService:
		// Describe our service to the outside world.
		retVal = (uint32_t) PrepareService( (BtLibServicePreparationParamsType*) cmdPBP );
		break;

	case sysBtLaunchCmdExecuteService:
		// An inbound connection to our service has arrived.
		gService = true;
		ExecuteService( (BtLibServiceExecutionParamsType*)cmdPBP );
		break;

	case sysBtLaunchCmdDescribeService:
		retVal = GetMyTypeAndCreator(&params.appType, &params.appCreator);	// initialize gBtChatDmOpenRef too
		if (retVal == errNone)
		{	
			int			size;
			MemHandle 	theResHdl;
			MemPtr		theResPtr;
			
			// Describe our service for the bluetooth panel services view.

			((BtLibServiceDescriptionType*)cmdPBP)->flags = 0;

			// Get the profile service name str
			theResHdl = DmGetResource(gBtChatDmOpenRef, (DmResourceType) strRsc, ChatServiceInfoNameString);
			theResPtr = MemHandleLock(theResHdl);
			size = strlen( theResPtr ) + 1;
			if ( (((BtLibServiceDescriptionType*)cmdPBP)->nameP = MemPtrNew( size )) == NULL ) {
				return btLibErrOutOfMemory;
			}
			MemMove( ((BtLibServiceDescriptionType*)cmdPBP)->nameP, theResPtr, size );
			(void) MemHandleUnlock(theResHdl);
			(void) DmReleaseResource(theResHdl);
			
			// Get the profile service description str
			theResHdl = DmGetResource(gBtChatDmOpenRef, (DmResourceType) strRsc, ChatServerServiceDescriptionString);
			theResPtr = MemHandleLock(theResHdl);
			size = strlen( theResPtr ) + 1;
			if ( (((BtLibServiceDescriptionType*)cmdPBP)->descriptionP = MemPtrNew( size )) == NULL ) {
				return btLibErrOutOfMemory;
			}
			MemMove( ((BtLibServiceDescriptionType*)cmdPBP)->descriptionP, theResPtr, size );
			(void) MemHandleUnlock(theResHdl);
			(void) DmReleaseResource(theResHdl);

		}
		break;

	case sysAppLaunchCmdNormalLaunch:
		// Normal application startup.
		gService = false;
		if ( AppStart() == errNone ) {
			FrmGotoForm( gDbResId, MainForm );
			AppEventLoop();
		}
		AppStop();
		break;
	}

	TraceOutput(TL(TRACE_MODULE,"BtChat: PilotMain: EXIT  cmd=%hu, retVal=%lu", cmd, retVal));
	return retVal;
}
