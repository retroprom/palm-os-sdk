/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Whiteboard.c
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Bluetooth Whiteboard sample application
 *
 *****************************************************************************/

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
#include <PollBox.h>
#include <ScrollBar.h>
#include <StdIO.h>
#include <StringMgr.h>
#include <SystemMgr.h>
#include <Table.h>
#include <TraceMgr.h>
#include <TimeMgr.h>
#include <string.h>
#include <BtLib.h>
//#include <unix_stdarg.h>
#include "WhiteboardRsc.h"


// #### SysTicksPerSecond not yet defined
#ifndef SysTicksPerSecond
#define SysTicksPerSecond() (1000)
#endif

#define CAN_POLL_FD_0	true 		// does polling fd 0 for UI events work?

#define UNUSED_ARG(x) (x) // To avoid compiler warning about unused arguments

// Some macros to manage byte ordering
#if  (CPU_ENDIAN == CPU_ENDIAN_BIG)
// convert network int16_t to host int16_t
#define 	NToHS(x)	(x)
// convert host int16_t to network int16_t
#define 	HToNS(x)	(x)
#elif  (CPU_ENDIAN == CPU_ENDIAN_LITTLE)
// convert network int16_t to host int16_t
#define 	NToHS(x)	_BtLibNetSwap16(x)
// convert host int16_t to network int16_t
#define 	HToNS(x)	_BtLibNetSwap16(x)
#else
#error "No CPU_ENDIAN defined"
#endif


/***********************************************************************
 *   Constants
 ***********************************************************************/

#define kAppFileCreator				'btwb'	// register your own at http://www.palmos.com/dev/creatorid/
#define kAppBtName					"Whiteboard Piconet Example Application"

#define kAppWhiteBoardErrorClass	appErrorClass + 5
#define kIOsFdNull									-1

#define kL2CapSocketMtu				500
#define kMaxDevices 				7
#define kBdAddrStrSize 				18
#define kQueueSendTriggerSecs	 	1
#define kMaxQueueSize				100

// {83E087AE-4E18-46be-83E0-7B3DE6A1C33B}
static const BtLibSdpUuidType kWhiteboardUUID = {btLibUuidSize128, 
{ 0x83, 0xe0, 0x87, 0xae, 0x4e, 0x18, 0x46, 0xbe, 0x83, 0xe0, 0x7b, 0x3d, 0xe6, 0xa1, 0xc3, 0x3b } };
	
static const BtLibClassOfDeviceType kDeviceFilter = (btLibCOD_ServiceAny | btLibCOD_Major_Computer | btLibCOD_Minor_Comp_Palm);

static const char kLockStr[] = "Lock";
static const char kUnlockStr[] = "Unlock";	

static const RectangleType kDrawRect = {0,0,160,143}; // Draw area

#define kMyMtu				100


/***********************************************************************
 *   Structures
 ***********************************************************************/

typedef	struct _WhiteBoardRemoteDrawEvent
{
	Boolean		clear;		//if clear is set, lines values are meaningless
	PointType	lineStart;
	PointType	lineStop;
} WhiteBoardRemoteDrawEvent;

typedef struct _BtDeviceInfoType
{
	BtLibDeviceAddressType	bdAddr;
	BtLibSocketRef 			connectionSocket;
} BtDeviceInfoType;
	
typedef struct _DeviceAddList
{
	BtLibDeviceAddressType	selectedDevices[kMaxDevices];
	uint8_t					numSelectedDevices;
} DeviceAddList;


/***********************************************************************
 *   Entry Points
 ***********************************************************************/

uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags);


/***********************************************************************
 *   Global variables
 ***********************************************************************/

static DmOpenRef				gDbResId;

static Boolean						gIsMaster;
static uint16_t						gSavedAutoOff;

static BtLibSdpRecordHandle 	gSdpRecordH = 0;
static BtDeviceInfoType		gDevices[kMaxDevices] ={ {{0}, kIOsFdNull}, 
											{{0}, kIOsFdNull}, {{0}, kIOsFdNull}, 
											{{0}, kIOsFdNull}, {{0}, kIOsFdNull}, 
											{{0}, kIOsFdNull}, {{0}, kIOsFdNull} };
static uint16_t  					gNumDevices = 0;
static uint16_t  					gNumConnections = 0;
static uint8_t   					gPendingLinks = 0;

// For adding devices to the piconet
static DeviceAddList 			gSelectedDevicesList = { {{0},{0},{0},{0},{0},{0}, {0}}, 0};
static char 							gBdAddrStr[kBdAddrStrSize];

/* WhiteBoard Data */
static PointType 					gLastLocalPenLocation = {0,0};
static Boolean						gPenIsDown = false;
static uint32_t  					gLastPenMoveTicks = 0;
static WhiteBoardRemoteDrawEvent gRemoteDrawingQueue[kMaxQueueSize]; //the queue of events to send
static uint16_t						gRemoteQueueFisrtIndex = 0;
static uint16_t						gRemoteQueueLength = 0;
static Boolean						gPendingQueueSend = false;
 
static WinHandle 				gOffscreenWinH = 0; // Draw to this window so data is stored off screen

static char 							gStatus[100];		// String to store status information for display
static uint32_t						gTimeToClearError = 0;	// Errors are cleared after some elapsed period

// A PollBox is a collection of file descriptors, for polling.
//
static PollBox*						gPollBox;

static int32_t						gFdUI 	=  kIOsFdNull;	 // fd for UI events
static int32_t 						gFdME 	= kIOsFdNull; // fd to Management Entity device
static int32_t 						gFdListener = kIOsFdNull; 	// fd to L2CAP or RfComm device, for listening

static Boolean 					gStopFlag = false;


/***********************************************************************
 *   Internal Functions
 ***********************************************************************/
 
static Boolean	MainFormHandleEvent(EventPtr eventP);
static Boolean	DrawingFormHandleEvent(EventPtr eventP);

static status_t 		BluetoothInit(void);
static void 	BluetoothCleanup(void);
 
static Boolean	L2CapValidChannel(void);
static Boolean	L2CapSendPending(void);
static status_t		L2CapConnect(uint8_t index, BtLibL2CapPsmType psm);
static status_t		L2CapFindPsm(void);
static void		L2CapDisconnect(BtLibSocketRef socket);

static void		DiscoverAndAdd(void);
static void		AclCreateLinks(void);
static void 	AclCreateNextLink(void);
static void		SocketHandleDisconnect(BtLibDeviceAddressTypePtr bdAddr);
static void		DestroyPiconet(void);

static Boolean  AddDrawEventToQueue(uint8_t clear, uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);
static int16_t  	IndexOfHeadOfQueue(void);
static Boolean	RemoveHeadOfQueue(uint16_t num);
static void 	ResetQueue(void);
static uint16_t 	AttemptToSendNext_N_EnquedEvents(uint16_t num);
static void 	AttemptQueueSend(void);

static void		OffscreenDrawLine(Coord x1, Coord y1, Coord x2, Coord y2, uint16_t newForeColor);
static void		ClearDrawArea(void);
static uint8_t	FindDeviceIndex(BtLibDeviceAddressTypePtr bdAddrP);
static uint8_t	FindSocketIndex(BtLibSocketRef socket);
static void		ChangeLabel( uint16_t labelID, const char *newLabel);
static void		UpdateStatusDisplay(void);
static void 	StatusMsg(const char *formatStr, ...);
static void 	ErrorMsg(const char *formatStr, ...);

 static void HandlePbxMeEvent( struct PollBox* pbx, struct pollfd* pollFd, void* context);
 static void HandlePbxSocketEvent( struct PollBox* pbx, struct pollfd* pollFd, void* context	);

#define min(a, b) (((a) < (b)) ? (a) : (b))



/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: Event handler of the main form
 *
 * PARAMETERS:  The event
 *
 * RETURNED:    true if event was handled
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr eventP)
{
	Boolean		handled = true;
  	FormPtr		frmP;
	status_t			error;

	switch (eventP->eType)
	{
		case menuEvent:
			if (eventP->data.menu.itemID == MainOptionsAboutWhiteboard)
			{
				MenuEraseStatus(0);	// Clear the menu status from the display.
				AbtShowAbout(kAppFileCreator);
			}
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
					
				case MainHostButton:
					gIsMaster = true;	
					error = BtLibPiconetCreate(gFdME, true, true);
					if(!error)
						FrmGotoForm(gDbResId, DrawingForm); 
					else
						ErrorMsg("Failed to create piconet");
					break;		

				case MainConnectButton:
					gIsMaster = false;
					error = BtLibDiscoverDevices(gFdME, NULL, NULL, false, (BtLibClassOfDeviceType*)&kDeviceFilter, 1,
								false, &(gSelectedDevicesList.selectedDevices[0]), 1, NULL);
					
					if( !error)
					{
						gSelectedDevicesList.numSelectedDevices = 1;
						AclCreateLinks();
					}
					else
						ErrorMsg("Discovery result: 0x%hx", error);
					break;
			
			}
			break;
			
		case frmOpenEvent:	
        	frmP = FrmGetActiveForm();
			FrmDrawForm (frmP);
			gIsMaster = false;
			break;
	
		default:
			handled = false;
	}

   	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    DrawingFormHandleEvent
 *
 * DESCRIPTION: Event handler of the drawing form
 *
 * PARAMETERS:  The event
 *
 * RETURNED:    true if event was handled
 *
 ***********************************************************************/
static Boolean DrawingFormHandleEvent(EventPtr eventP)
{
	FormPtr frmP = FrmGetActiveForm();
	ControlPtr ctrlP;   
	Boolean handled = true;
   	status_t error;
  
	switch (eventP->eType)
	{
		case nilEvent:
			if (L2CapValidChannel())
			{
				uint32_t timeSinceLastMove =(uint32_t)((TimGetTicks() - gLastPenMoveTicks)/SysTicksPerSecond());
				if (gPenIsDown && timeSinceLastMove >= kQueueSendTriggerSecs)
					gPendingQueueSend = true;
				if (gPendingQueueSend)
					AttemptQueueSend();
			}
			handled = false;
			break;
			
		case penDownEvent:
			if (L2CapValidChannel())
			{
				gPenIsDown = true;
				gLastLocalPenLocation.x = eventP->screenX;
				gLastLocalPenLocation.y = eventP->screenY;	
			}
			handled = false;
			break;
			
		case penUpEvent:
			if (L2CapValidChannel())
			{
				gPenIsDown = false;
				AttemptQueueSend();
			}
			handled = false;
			break;
			
		case penMoveEvent:
			if ( L2CapValidChannel() && gOffscreenWinH &&
				eventP->screenY < kDrawRect.topLeft.y + kDrawRect.extent.y)
			{
				gLastPenMoveTicks = (uint32_t)TimGetTicks();

				OffscreenDrawLine(gLastLocalPenLocation.x, gLastLocalPenLocation.y, eventP->screenX, eventP->screenY, 0);
								
				AddDrawEventToQueue( false, gLastLocalPenLocation.x, gLastLocalPenLocation.y, eventP->screenX, eventP->screenY);
				AttemptQueueSend();
				gLastLocalPenLocation.x = eventP->screenX;
				gLastLocalPenLocation.y = eventP->screenY;
			}
			handled = false;
			break;	
		   
	   case ctlSelectEvent:		   	
		   	if(gPendingLinks != 0)
		   		break; // No UI interaction while adding devices
		   		
			switch (eventP->data.ctlSelect.controlID)
			{
			
				case DrawingClearScreenButton:
					ClearDrawArea();
					AddDrawEventToQueue( true, 0,0,0,0);
					AttemptQueueSend();
					break;
					
				case DrawingLeaveButton:
					if (gIsMaster)
						DestroyPiconet();
					else
						L2CapDisconnect(gDevices[0].connectionSocket);
			   		break;
			   		
				case DrawingAddDevicesButton:
					DiscoverAndAdd();
					break;
					 
				case DrawingToggleLockButton: 
				{
					ctrlP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, DrawingToggleLockButton));
					if(!StrCompare(CtlGetLabel(ctrlP), kLockStr))
						BtLibPiconetLockInbound(gFdME);
					else
						BtLibPiconetUnlockInbound(gFdME, true);		
					break;
				}								
				  
        		default:
  	    	  		handled = false;
	     	   		break;
			}
			break;

     	case frmOpenEvent:
			if (!gIsMaster)
			{
				// We are a slave so no add or lock buttons
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, DrawingAddDevicesButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, DrawingToggleLockButton));		
			}		
			gOffscreenWinH = WinCreateOffscreenWindow(kDrawRect.extent.x, kDrawRect.extent.y, nativeFormat, &error);
			if ((error) || (!gOffscreenWinH))
			{
				TraceOutput(TL( kAppWhiteBoardErrorClass, "Could not create offscreen window (height=%d width=%d) error=0x%lX= ", kDrawRect.extent.x, kDrawRect.extent.y, error ));
				ErrFatalDisplay("Could not create offscreen window");
			}
			ErrFatalDisplayIf(gOffscreenWinH == 0,"Could not create offscreen window");
			FrmDrawForm (frmP);
			break;
			
		case frmCloseEvent:	
			WinDeleteWindow(gOffscreenWinH, false);				
		    handled = false;
		    break;
		
		case frmUpdateEvent:
			//Copy Offscreen window to the display
			FrmDrawForm(frmP);
			WinCopyRectangle(gOffscreenWinH, NULL, &kDrawRect,  0, 0, winPaint);
			break;
			
		default:
			handled = false;
			break;
   }
	return handled;
}

#if 0
#pragma mark -
#endif

/***********************************************************************
 *
 * FUNCTION:     BluetoothInit
 *
 * DESCRIPTION:  Find/Load/Open the lib and setup our listener
 *
 * PARAMETERS:   Nothing
 *
 * RETURNED:     status_t value 0 if nothing went wrong
 *
 ***********************************************************************/
static status_t BluetoothInit(void)
{
   status_t error = errNone;
   BtLibSocketListenInfoType listenInfo;

	TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit:"));
	error = BtLibOpen(&gFdME);
	if (error && error != btLibErrAlreadyOpen)
	{
		TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: BtLibOpen() error=0x%lX", error));
		return error;
	}

	error = PbxAddFd( gPollBox, gFdME, POLLIN, HandlePbxMeEvent, 0 );
	if (error)
	{
		TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: PbxAddFd(fdME) error=0x%lX", error));
		return error;
	}
	
	error = BtLibSocketCreate( &gFdListener, btLibL2CapProtocol);
	if (error)
	{
//		EventType tempoEvent;
		
		TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: BtLibSocketCreate error=%lX", error));
/*
		tempoEvent.eType = frmCloseEvent;
		tempoEvent.data.frmOpen.formID = FrmGetActiveFormID();
		(void) EvtAddEventToQueue(&tempoEvent);
*/		return error;
	}
	ErrFatalDisplayIf(error, "L2CAP Socket Creation Failure");

	error = PbxAddFd( gPollBox, gFdListener, POLLIN, HandlePbxSocketEvent, &gFdListener );
	if (error)
	{
		TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: PbxAddFd(gFdListener) error=%lX", error));
		return error;
	}
	listenInfo.data.L2Cap.localPsm = BT_L2CAP_RANDOM_PSM;
	listenInfo.data.L2Cap.localMtu = kL2CapSocketMtu; 
	listenInfo.data.L2Cap.minRemoteMtu = kL2CapSocketMtu;
	error = BtLibSocketListen( gFdListener, &listenInfo);
	if (!error)
	{
		error = BtLibSdpServiceRecordCreate(gFdME, &gSdpRecordH );
		if (!error)
		{
			status_t err = errNone;
		
			err = BtLibSdpServiceRecordSetAttributesForSocket( gFdListener, 
					(BtLibSdpUuidType*) &kWhiteboardUUID, 1, kAppBtName, strlen(kAppBtName), gSdpRecordH);
			if (err)
				TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: BtLibSdpServiceRecordSetAttributesForSocket(gFdListener) error=%lX", err));
			
			err = BtLibSdpServiceRecordStartAdvertising(gFdME, gSdpRecordH);
			if (err)
				TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: BtLibSdpServiceRecordStartAdvertising(gFdME) error=%lX", err));
		}
		else
		{
			TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: BtLibSdpServiceRecordCreate(gFdME) error=%lX", error));
		}
	}
	else
	{
		TraceOutput(TL(kAppWhiteBoardErrorClass, "BluetoothInit: BtLibSocketListen(gFdListener) error=%lX", error));
	}
	ErrFatalDisplayIf(error, "L2CAP Listen Failure");

   return error;
}


/***********************************************************************
 *
 * FUNCTION:     BluetoothCleanup
 *
 * DESCRIPTION:  Destroy SDP record, close listener socket, 
 *               unregister callback and close lib
 *
 * PARAMETERS:   Nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 ***********************************************************************/
static void BluetoothCleanup(void)
{
	BtLibSdpServiceRecordDestroy(gFdME, gSdpRecordH);
	BtLibSocketClose(gFdListener);
	PbxRemoveFd( gPollBox, gFdListener );
	BtLibClose(gFdME);
}

#if 0
#pragma mark -
#endif

/***********************************************************************
 *
 * FUNCTION:    L2CapValidChannel
 *
 * DESCRIPTION: Check that we are connected
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    true if L2Cap channel is valid
 *
 ***********************************************************************/
static Boolean L2CapValidChannel(void)
{
	BtLibL2CapChannelIdType channel;
	status_t	error;
	
	error = BtLibSocketGetInfo(gDevices[0].connectionSocket, btLibSocketInfo_L2CapChannel, 
		&channel, sizeof(channel));
	return (error == btLibErrNoError);
}


/***********************************************************************
 *
 * FUNCTION:    L2CapSendPending
 *
 * DESCRIPTION: Check if a send operation is pending on any active connection
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    true if a send operation is pending
 *
 ***********************************************************************/
static Boolean L2CapSendPending(void)
{
	Boolean sending = false;
	uint16_t i = 0;
	
	for(i = 0; i < gNumDevices && gDevices[i].connectionSocket != kIOsFdNull; i++)
	{
		BtLibSocketGetInfo(gDevices[i].connectionSocket, btLibSocketInfo_SendPending, 
			&sending, sizeof(sending)); 
		
		if(sending)
			return true;
	}
		
	return false;	
}


/***********************************************************************
 *
 * FUNCTION:    L2CapConnect
 *
 * DESCRIPTION: Connect an L2Cap socket
 *
 * PARAMETERS:  Device index and PSM to connect to
 *
 * RETURNED:    Error code
 *
 ***********************************************************************/
static status_t L2CapConnect(uint8_t index, BtLibL2CapPsmType psm)
{
	status_t error;
	BtLibSocketConnectInfoType connectInfo;
	
	connectInfo.data.L2Cap.remotePsm = psm;
	connectInfo.data.L2Cap.localMtu = kL2CapSocketMtu; 
	connectInfo.data.L2Cap.minRemoteMtu = kL2CapSocketMtu;
					
	if (gDevices[index].connectionSocket != kIOsFdNull)
		ErrorMsg("Trying to connect already valid socket");

	error = BtLibSocketCreate( &gDevices[index].connectionSocket, btLibL2CapProtocol);
	if( error == btLibErrNoError)
	{	
		connectInfo.remoteDeviceP = &gDevices[index].bdAddr;

		error = PbxAddFd( gPollBox, gDevices[index].connectionSocket, POLLIN, HandlePbxSocketEvent, &gDevices[index].connectionSocket );
		if(error)
			return error;
	
		error = BtLibSocketConnect(gDevices[index].connectionSocket, &connectInfo);
		if (error == btLibErrPending) 
			StatusMsg( "Connecting... ");
		else
			ErrorMsg("Socket Connect failed");
	}
	else
		ErrorMsg("L2CAP Socket Creation Failure");
	
	return error;
}


/***********************************************************************
 *
 * FUNCTION:    L2CapFindPsm
 *
 * DESCRIPTION: Master uses SDP to find the Whiteboard psm for l2cap
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    Error code
 *
 ***********************************************************************/
static status_t L2CapFindPsm(void)
{
	status_t error = errNone;
	uint16_t i;

	for(i = 0; i < gNumDevices; i++)
	{ 
		if(gDevices[i].connectionSocket == kIOsFdNull)
		{
			StatusMsg("L2CapFindPsm socket=%d on #%d", gDevices[i].connectionSocket, i);
		
			error = BtLibSocketCreate( &gDevices[i].connectionSocket, btLibSdpProtocol);
			if (!error)
			{
				error = PbxAddFd( gPollBox, gDevices[i].connectionSocket, POLLIN, HandlePbxSocketEvent, &gDevices[i].connectionSocket );
				if(error)
					return error;
			
				// Use SDP to get the psm, the connection is made in the callback
				error = BtLibSdpGetPsmByUuid(gDevices[i].connectionSocket, &gDevices[i].bdAddr,
					(BtLibSdpUuidType*) &kWhiteboardUUID, 1);
			}
			if (error == btLibErrPending) 
				StatusMsg("Getting PSM... ");
			else 
				ErrorMsg("PSM retrieval failed, err 0x%hx", error);
			break;
		}
	}
	return error;
}


/***********************************************************************
 *
 * FUNCTION:    L2CapDisconnect
 *
 * DESCRIPTION: Close socket and disconnect ACL
 *
 * PARAMETERS:  The socket ref
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void L2CapDisconnect(BtLibSocketRef socket)
{
	status_t error;
	uint8_t index;

	StatusMsg("L2CapDisconnect");

	error = BtLibSocketClose(socket);
	PbxRemoveFd( gPollBox, socket );
	
	index = FindSocketIndex(socket);
	gDevices[index].connectionSocket = kIOsFdNull;

	error = BtLibLinkDisconnect(gFdME, &(gDevices[index].bdAddr));
			
	if(gNumConnections == 0)
		ResetQueue();		
}

#if 0
#pragma mark -
#endif

/***********************************************************************
 *
 * FUNCTION:    DiscoverAndAdd
 *
 * DESCRIPTION: Discover new devices and connect to those selected
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void DiscoverAndAdd(void)
{
	status_t error;
//	Boolean showLastDiscovery = false;
	BtLibClassOfDeviceType deviceFilter = (btLibCOD_ServiceAny | btLibCOD_Major_Computer | btLibCOD_Minor_Comp_Palm);

	// Discover some devices to add to the piconet, but loop if we exceed the maximum			
	error = BtLibDiscoverDevices(gFdME, NULL, NULL, false, &deviceFilter, 1, false, (BtLibDeviceAddressType *)&(gSelectedDevicesList.selectedDevices), kMaxDevices, &(gSelectedDevicesList.numSelectedDevices)); 	
	if(error == btLibErrNoError)
	{			
		AclCreateLinks();
	}								
}


/***********************************************************************
 *
 * FUNCTION:    AclCreateLinks
 *
 * DESCRIPTION: Create an ACL link to 1 or more devices
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void AclCreateLinks(void)
{
	gPendingLinks =	gSelectedDevicesList.numSelectedDevices;	
	if(gPendingLinks != 0)
	{
		StatusMsg("Creating Link(s)");
		AclCreateNextLink();
	}
	else
		StatusMsg("Could not add devices to piconet");
	
}

/***********************************************************************
 *
 * FUNCTION:    AclCreateNextLink
 *
 * DESCRIPTION: Create the next link based on the gSelectedDevicesList and gPendingLinks
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void AclCreateNextLink(void)
{
	status_t  error;
	
	// Connect to the next device. In case it fails or it is already done, try with another device
	do
	{
		error = BtLibLinkConnect( gFdME, 
			&(gSelectedDevicesList.selectedDevices[gSelectedDevicesList.numSelectedDevices - gPendingLinks]) );
		if(error != btLibErrPending)
		{
			gPendingLinks--;	
			BtLibAddrBtdToA(&(gSelectedDevicesList.selectedDevices[gSelectedDevicesList.numSelectedDevices - gPendingLinks]), gBdAddrStr, kBdAddrStrSize);
			StatusMsg( "ACL to %s failed, continuing", gBdAddrStr);
		}
	} while (error != btLibErrPending && gPendingLinks != 0);
			
}


/***********************************************************************
 *
 * FUNCTION:    SocketHandleDisconnect
 *
 * DESCRIPTION: Handle ACL disconnection
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void SocketHandleDisconnect(BtLibDeviceAddressTypePtr bdAddr)
{
	uint8_t i;

	i = FindDeviceIndex(bdAddr);
	if (i < gNumDevices)
	{
		if (gDevices[i].connectionSocket != kIOsFdNull)
		{
			StatusMsg("SocketHandleDisconnect");
		
			gNumConnections--;
			BtLibSocketClose( gDevices[i].connectionSocket);
			PbxRemoveFd( gPollBox, gDevices[i].connectionSocket );
		}
		
		if(i < gNumDevices-1)
		{
			// Compact array of device
			memmove(&gDevices[i], &gDevices[i+1], (gNumDevices - i) * sizeof(BtDeviceInfoType));
		}
		
		gNumDevices--;
		gDevices[gNumDevices].connectionSocket = kIOsFdNull;
	}
		
	if(!gIsMaster)
		FrmGotoForm(gDbResId, MainForm);
}


/***********************************************************************
 *
 * FUNCTION:    DestroyPiconet
 *
 * DESCRIPTION: Close all sockets, ACL connections and destroy the piconet
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void DestroyPiconet(void)
{
	uint16_t i;

	StatusMsg("DestroyPiconet");
	
	for (i=0; i<gNumDevices;i++)
	{
		BtLibSocketClose( gDevices[i].connectionSocket);
		PbxRemoveFd( gPollBox, gDevices[i].connectionSocket );
		BtLibLinkDisconnect(gFdME, &gDevices[i].bdAddr);
	}
	if (BtLibPiconetDestroy(gFdME) == errNone)
		FrmGotoForm(gDbResId, MainForm);
}

#if 0
#pragma mark -
#endif

/***********************************************************************
 *
 * FUNCTION:    AddDrawEventToQueue
 *
 * DESCRIPTION: Add a drawing event (a line) or a clear event to the queue
 *
 * PARAMETERS:  clear: if true, a clear event is enqueued, else a drawing event
 *              startX, startY: the origin point for drawing event
 *              destX, destY: the destination point for drawing event
 *
 * RETURNED:    true if event was added
 *
 ***********************************************************************/
static Boolean	AddDrawEventToQueue(uint8_t clear, uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
	uint8_t	firstEmpty;

	// Check if the queue is full
	if ( gRemoteQueueLength == kMaxQueueSize )
	{
		FrmCustomAlert(gDbResId, GeneralAlert, "Queue Overflow", 0, 0);
		return false;
	}

	// Calculate the first empty slot
	firstEmpty = (gRemoteQueueFisrtIndex + gRemoteQueueLength) % kMaxQueueSize;
	gRemoteQueueLength++;
	
	gRemoteDrawingQueue[firstEmpty].clear = clear;
	if( !clear )
	{
		// Add the line data
		gRemoteDrawingQueue[firstEmpty].lineStart.x = HToNS(startX);
		gRemoteDrawingQueue[firstEmpty].lineStart.y = HToNS(startY);
		gRemoteDrawingQueue[firstEmpty].lineStop.x = HToNS(endX);
		gRemoteDrawingQueue[firstEmpty].lineStop.y = HToNS(endY);
	}
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    IndexOfHeadOfQueue
 *
 * DESCRIPTION: Trivial
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    -1 if empty, else the head index
 *
 ***********************************************************************/
static int16_t IndexOfHeadOfQueue(void)
{
	if ( gRemoteQueueLength == 0 )
		return -1;
	else
		return gRemoteQueueFisrtIndex;
}


/***********************************************************************
 *
 * FUNCTION:	RemoveHeadOfQueue
 *
 * DESCRIPTION: Remove the n-th first events of the queue
 *
 * PARAMETERS:	num - the number of events to remove from head
 * 
 * RETURNED:	-1 if empty, else the index
 *
 ***********************************************************************/
static Boolean RemoveHeadOfQueue(uint16_t num)
{	
	// Make sure there are enough to remove
	if ( gRemoteQueueLength < num )
		return false;
	
	// Decrement the length
	gRemoteQueueLength = gRemoteQueueLength - num;
	
	// Update the head index
	gRemoteQueueFisrtIndex = (gRemoteQueueFisrtIndex + num) % kMaxQueueSize;
	
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    ResetQueue
 *
 * DESCRIPTION: Reinitialise queue
 *
 * PARAMETERS:  nothing
 * 
 * RETURNED:   	nothing
 *
 ***********************************************************************/
static void ResetQueue(void)
{
	gPendingQueueSend = false;
	gRemoteQueueLength = 0;
	gRemoteQueueFisrtIndex = 0;
}


/***********************************************************************
 *
 * FUNCTION:    AttemptQueueSend
 *
 * DESCRIPTION: Attempt to send all queued events
 *
 * PARAMETERS:	Nothing
 * 
 * RETURNED:	Nothing
 *
 ***********************************************************************/
static void AttemptQueueSend(void)
{
	if( gRemoteQueueLength == 0)
		return;

	AttemptToSendNext_N_EnquedEvents(gRemoteQueueLength);
	
	gPendingQueueSend = (gRemoteQueueLength == 0);
}


/***********************************************************************
 *
 * FUNCTION:	AttemptToSendNext_N_EnquedEvents
 *
 * DESCRIPTION:	Tries to send the next 'num' events as an L2Cap Packet
 *
 * PARAMETERS:	n - the number of events to try to send
 *
 * RETURNED:	Number sent
 *
 ***********************************************************************/
static uint16_t AttemptToSendNext_N_EnquedEvents(uint16_t n)
{
	status_t    		error;
	uint16_t 		headIndex = IndexOfHeadOfQueue();
	uint16_t		firstSegmentSize; 
	uint16_t		secondSegmentSize = 0;
	uint16_t		num;
	uint16_t      i;
//	Boolean		bAllSuccess = true; // Did all the sends succeed.  
	
	// Don't ask for more than there are and if the queue is empty, return
	num = min( n, gRemoteQueueLength);	
	if( num <= 0)
		return 0;
	
	// Check to see if data to send is wrapping around in the buffer,
	// if so, we will send the first segment and then recurse to send 
	// the second part
	if ( (headIndex + num) > kMaxQueueSize )
	{
		secondSegmentSize = (headIndex + num) % kMaxQueueSize;
	} 
	firstSegmentSize = num - secondSegmentSize;
	
	// Handle case where we might try to send too much
	if(sizeof(WhiteBoardRemoteDrawEvent) * firstSegmentSize > kL2CapSocketMtu)
	{
		// Put the excess in the second segment
		secondSegmentSize += firstSegmentSize - (kL2CapSocketMtu/sizeof(WhiteBoardRemoteDrawEvent));
		
		// Send as much as possible
		firstSegmentSize = kL2CapSocketMtu/sizeof(WhiteBoardRemoteDrawEvent);
	}

	// Make sure we are connected as either the server or the client
	if (!L2CapValidChannel())
	{
		ErrorMsg("L2Cap Session not established");
		return 0;
	}
	
	if(L2CapSendPending())
		return 0;
	for(i = 0; i < gNumDevices; i++)
	{
		error = BtLibSocketSend(gDevices[i].connectionSocket, (uint8_t*) &gRemoteDrawingQueue[headIndex], 
			sizeof(WhiteBoardRemoteDrawEvent) * firstSegmentSize);
			
		if ( (error != errNone) && (error != btLibErrPending) )
		{
			// Send failed.  Find out what happened.  
			BtLibDeviceAddressType device;
			status_t error2;

			error2 = BtLibSocketGetInfo(gDevices[i].connectionSocket, 
				btLibSocketInfo_RemoteDeviceAddress, &device, sizeof(device));
						
			if (!error2)
			{
				BtLibAddrBtdToA( &device, gBdAddrStr, kBdAddrStrSize);	
				ErrorMsg( "Send to %s failed: 0x%hx", gBdAddrStr, error);
	  		}
  			return 0;
		}		
	}
		
	// Dequeue the DrawEvents we just sent
	RemoveHeadOfQueue(firstSegmentSize);
	
	// Recurse if needed
	if(secondSegmentSize)
		return firstSegmentSize + AttemptToSendNext_N_EnquedEvents(secondSegmentSize);

	return firstSegmentSize;
	
}

#if 0
#pragma mark -
#endif

/***********************************************************************
 *
 * FUNCTION:    OffscreenDrawLine
 *
 * DESCRIPTION: Draw a line offscreen and then copy the offscreen window
 *              into the screen window
 *
 * PARAMETERS:  Line start and end.
 *
 * RETURNED:	Nothing
 *
 ***********************************************************************/
static void OffscreenDrawLine(Coord x1, Coord y1, Coord x2, Coord y2, uint16_t newForeColor)
{
	WinHandle oldWinH;
	IndexedColorType aForeColor;
	RGBColorType aRGB[8] = { {0, 0, 0, 0}, {0, 255, 0, 0}, {0, 0, 255, 0}, {0, 0, 0, 255}, {0, 255, 255, 0}, {0, 0, 255, 255}, {0, 255, 0, 255}, {0, 128, 128, 128} };
	uint16_t newColorIndex = newForeColor;

	if (newForeColor >= 8)
		newColorIndex = newForeColor % 8;

	TraceOutput(TL( kAppWhiteBoardErrorClass, "OffscreenDrawLine: index=%hu", newColorIndex ));
	
  	if(gOffscreenWinH)
	{		
 		oldWinH = WinSetDrawWindow(gOffscreenWinH);
		aForeColor = WinSetForeColor (WinRGBToIndex(&aRGB[newColorIndex]));
  		WinDrawLine( x1, y1, x2, y2);
		WinSetForeColor (aForeColor);
  		WinSetDrawWindow(oldWinH);		
 		if(FrmGetActiveFormID() == DrawingForm)
  			WinCopyRectangle(gOffscreenWinH, NULL, &kDrawRect,  0, 0, winPaint);
	}

}


/***********************************************************************
 *
 * FUNCTION:    ClearDrawArea
 *
 * DESCRIPTION: Erase offscreen window and then copy it
 *              into the screen window
 *
 * PARAMETERS:  Line start and end.
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void ClearDrawArea(void)
{  			
	WinHandle oldWinH;
	
	if(gOffscreenWinH)
	{
		oldWinH = WinSetDrawWindow(gOffscreenWinH);	
		WinEraseWindow();		
		WinSetDrawWindow(oldWinH);		
  		if(FrmGetActiveFormID() == DrawingForm)
  			WinCopyRectangle(gOffscreenWinH, NULL, &kDrawRect,  0, 0, winPaint);
	}
}


/***********************************************************************
 *
 * FUNCTION:    FindDeviceIndex
 *
 * DESCRIPTION: Find a device in the array from its address
 *              and return its index
 *
 * PARAMETERS:	The BT address
 *
 * RETURNED:	The index
 *
 ***********************************************************************/
static uint8_t FindDeviceIndex(BtLibDeviceAddressTypePtr bdAddrP)
{
	uint8_t i;
	for(i = 0; i < gNumDevices; i++)
	{
		if(memcmp(&gDevices[i].bdAddr, bdAddrP, sizeof(BtLibDeviceAddressType)) == 0)
			break;
	}	
	return i;
}


/***********************************************************************
 *
 * FUNCTION:    FindSocketIndex
 *
 * DESCRIPTION: Find a device in the array from its socket ref
 *              and return its index
 *
 * PARAMETERS:	The socket ref
 *
 * RETURNED:	The index
 *
 ***********************************************************************/
static uint8_t FindSocketIndex(BtLibSocketRef socket)
{
	uint8_t i;	
	for(i = 0; i < gNumDevices; i++)
	{
		if(gDevices[i].connectionSocket == socket)
			break;
	}
	return i;
}


/***********************************************************************
 *
 * FUNCTION:    ChangeLabel
 *
 * DESCRIPTION: Utility function to change a label
 *				Note: The initial label (done in Constructor) MUST be
 *                    longer than any label you copy over it!!
 *
 * PARAMETERS:  labelID - id of the label to change
 *				char* - new label ptr.  label is copied & need not persist
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void ChangeLabel( uint16_t labelID, const char *newLabel)
{

	FormPtr frm = FrmGetActiveForm();
	uint16_t	labelIndex = FrmGetObjectIndex(frm, labelID);
	
	//Hide the label first to erase old text
	FrmHideObject(frm, labelIndex);
	
	//copy in the new string
	FrmCopyLabel(frm, labelID, newLabel);
	
	//now force the redraw
	FrmShowObject(frm, labelIndex);
}


/***********************************************************************
 *
 * FUNCTION:    UpdateStatusDisplay
 *
 * DESCRIPTION: Displays the status message on the screen.  This is called during a nilEvent
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void UpdateStatusDisplay(void)
{
	// Keep the error displayed for a while
	if (gTimeToClearError != 0)
	{
		if (TimGetSeconds() < gTimeToClearError)
			return;
		gTimeToClearError = 0;
	}
		
	switch (FrmGetActiveFormID())
	{
		case MainForm:
			ChangeLabel(MainStatusLabel, gStatus);
			break;
			
		case DrawingForm:
			ChangeLabel(DrawingStatusLabel, gStatus);
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    StatusMsg
 *
 * DESCRIPTION: Updates the status message string.  The screen is only updated occasionally
 *
 * PARAMETERS:  same as PrintF
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void StatusMsg(const char *formatStr, ...)
{
	_Palm_va_list args;

	va_start(args, formatStr);
	StrVPrintF(gStatus, formatStr, args);
	va_end(args);

	TraceOutput(TL( kAppWhiteBoardErrorClass, "StatusMsg: '%s'", gStatus ));
	
	UpdateStatusDisplay();
}


/***********************************************************************
 *
 * FUNCTION:    ErrorMsg
 *
 * DESCRIPTION: Updates the error message string and display it.
 *
 * PARAMETERS:  same as PrintF
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void ErrorMsg(const char *formatStr, ...)
{
	_Palm_va_list args;

	va_start(args, formatStr);
	StrVPrintF(gStatus, formatStr, args);
	va_end(args);

	TraceOutput(TL( kAppWhiteBoardErrorClass, "ErrorMsg: '%s'", gStatus ));

	UpdateStatusDisplay();
	// Display the error for at least 5 seconds
	gTimeToClearError = TimGetSeconds() + 5;
}

#if 0
#pragma mark -
#endif

/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	uint16_t formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
	{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(gDbResId, formId);
		FrmSetActiveForm(frmP);
		// Set the event handler for the form
		switch (formId)
		{
			case DrawingForm:
		      	FrmSetEventHandler(frmP, DrawingFormHandleEvent);
		        break;
			
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;			
		}
		return true;
	}
	return false;
}


/***********************************************************************
 *
 * FUNCTION:		HandlePbxSocketEvent
 *
 * DESCRIPTION: Handle events from "socket" devices (L2CAP, RfComm, SDP).
 *
 ***********************************************************************/
static void HandlePbxSocketEvent(
	struct PollBox* 			pbx,		// the PollBox
	struct pollfd*				pollFd, 	// fd, event mask, and received events
	void* 					context 	// in this app, ptr to global fd variable
) {
	status_t					error;
	int32_t 					flags;

	static BtLibSocketEventType sEvent;
	static char 				sData[kMyMtu];
	static struct strbuf		ctlBuf = { sizeof(sEvent), 0, (char*)&sEvent	 };
	static struct strbuf		datBuf = { sizeof(sData),  0, (char*)&sData[0] };

	uint8_t index = 0;

	TraceOutput(TL(kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: fd=%lx, *context=%lx", pollFd->fd, (context)?*((int32_t*)context):0 ));

	// We must be here for a reason ...
	DbgOnlyFatalErrorIf( !(pollFd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL) ), "no event flag" );

	// Our context variable is just the global variable containing the file descriptor.
//	DbgOnlyFatalErrorIf( context != &gFdSdp && context != &gFdListener && context != &gFdData, "bad context" );
	DbgOnlyFatalErrorIf( pollFd->fd != *((int32_t*)context), "context not fd" );
	DbgOnlyFatalErrorIf( pollFd->fd < 0, "bad fd" );

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if (
		(pollFd->revents & (POLLERR|POLLHUP|POLLNVAL)) || 			// poll error or hangup
		IOSGetmsg( pollFd->fd, &ctlBuf, &datBuf, &flags, &error ) != 0	// getmsg error
	) {
		TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: closing fd %ld", pollFd->fd ));
		PbxRemoveFd( pbx, pollFd->fd );
		IOSClose( pollFd->fd );
		*(int32_t*)context = -1;
		return;
	}

	TraceOutput(TL(kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: fd=%lx, ctlBuf.len=%hd, datBuf.len=%hd", pollFd->fd, ctlBuf.len, datBuf.len));
	
	// Check for pure data.
	if ( ctlBuf.len == -1 )
//		if ( datBuf.len != -1 )
	{
		// Process a drawing event locally. The master also forwards it to slaves.
		WhiteBoardRemoteDrawEvent *drawDataP;
		uint16_t		i,numEvents;

		TraceOutput(TL(kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Socket %ld: Data: %s", pollFd->fd, datBuf.buf ));
		// For each drawing events
		numEvents = datBuf.len / sizeof(WhiteBoardRemoteDrawEvent);
		for( i = 0; i < numEvents; i++)
		{
			drawDataP = ((WhiteBoardRemoteDrawEvent*) datBuf.buf) + i;	
			// Execute it locally
			if(drawDataP->clear)
				ClearDrawArea();
			else
				OffscreenDrawLine( NToHS(drawDataP->lineStart.x), NToHS(drawDataP->lineStart.y), NToHS(drawDataP->lineStop.x), NToHS(drawDataP->lineStop.y), pollFd->fd);
			// Master also forwards it to slave
			if(gIsMaster)
			{ 
				AddDrawEventToQueue(drawDataP->clear, NToHS(drawDataP->lineStart.x), NToHS(drawDataP->lineStart.y), 
					NToHS(drawDataP->lineStop.x), NToHS(drawDataP->lineStop.y));
			}
		}
		if(gIsMaster)
			AttemptQueueSend();
		// send update to screen
		FrmUpdateForm (FrmGetActiveFormID(), frmRedrawUpdateCode);
		return;
	}

	// Since it's not pure data, we must have an event struct in the control part.
	DbgOnlyFatalErrorIf( ctlBuf.len != sizeof(BtLibSocketEventType), "no event struct" );

	// Decode the event.
	switch ( sEvent.event )
	{
	case btLibSocketEventSdpGetServerChannelByUuid:
		if ( sEvent.status != btLibErrNoError )
		{
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: SDP Get Channel error: 0x%lx", sEvent.status ));
		}
		else
		{
			BtLibSocketConnectInfoType connectInfo;

			index = FindSocketIndex(pollFd->fd);
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Retrieved Channel ID: 0x%ld", (int)sEvent.eventData.sdpByUuid.param.channel ));
			// Close the SDP socket that was used to make the query.
			DbgOnlyFatalErrorIf( pollFd->fd !=  gDevices[index].connectionSocket, "not sdp socket" );
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: closing SDP socket %ld", pollFd->fd ));	

			SocketHandleDisconnect(&gDevices[index].bdAddr );
			
			index = FindSocketIndex(pollFd->fd);
			// Create a data socket.
			error = BtLibSocketCreate( &gDevices[index].connectionSocket, btLibRfCommProtocol );
			if ( error )
			{
				TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: RFCOMM SocketCreate Error: 0x%lx", error )); 
			}
			else
			{
				gDevices[index].bdAddr = sEvent.eventData.requestingDevice;
				PbxAddFd( gPollBox, gDevices[index].connectionSocket, POLLIN, HandlePbxSocketEvent, &gDevices[index].connectionSocket );

				// Connect the data socket.
				connectInfo.remoteDeviceP = &sEvent.eventData.requestingDevice;
				connectInfo.data.RfComm.remoteService  = sEvent.eventData.sdpByUuid.param.channel;
				connectInfo.data.RfComm.maxFrameSize	 = kMyMtu;
				connectInfo.data.RfComm.advancedCredit = 0;
				error = BtLibSocketConnect( gDevices[index].connectionSocket, &connectInfo );
				if ( error != btLibErrPending )
				{
					TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: RFCOMM Connect Error: 0x%lx", error )); 
				}
				else
				{
					TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Socket %ld: connecting...", gDevices[index].connectionSocket )); 
				}
			}
		}
		break;

	case btLibSocketEventSdpGetPsmByUuid:
		{
			if(sEvent.status == btLibErrNoError)
			{ 
				StatusMsg("HandlePbxSocketEvent: btLibSocketEventSdpGetPsmByUuid SDP socket %ld", pollFd->fd);
				index = FindSocketIndex(pollFd->fd); 			
				error = BtLibSocketClose(pollFd->fd);			
				PbxRemoveFd(pbx, pollFd->fd);
				if(error != btLibErrNoError)
					ErrorMsg("Could not close SDP socket"); 	
				gDevices[index].connectionSocket = kIOsFdNull;
				//Found the psm so lets connect to the device
				L2CapConnect(index, sEvent.eventData.sdpByUuid.param.psm);		
			}
			else
			{
				// Treat failed search like a disconnect
				L2CapDisconnect(sEvent.eventData.newSocket);
			}
			break;
		}

	case btLibSocketEventConnectRequest:
		// Slave always accept master connection		
		// Allow only one connection at a time
		DbgOnlyFatalErrorIf( pollFd->fd != gFdListener, "not listener socket" );
		error = BtLibSocketRespondToConnection( pollFd->fd, true );
		if ((error != errNone) && (error != btLibErrPending))
		{
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Inbound Req refused Error: 0x%lx", error )); 
		}
		else
		{
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Socket %ld: Inbound Req Accepted", pollFd->fd )); 
		}
		break;

	case btLibSocketEventConnectedOutbound:
		// Master just connected to a slave
		index = FindSocketIndex(pollFd->fd);
		DbgOnlyFatalErrorIf( pollFd->fd != gDevices[index].connectionSocket, "not data socket" );
		if ( sEvent.status == btLibErrNoError )
		{
			// Connection succeeded.
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Socket %ld: Outbound Conn OK", pollFd->fd )); 
			StatusMsg("Draw");
			gNumConnections++;
		}
		else
		{
			// Connection failed.
			TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Socket %ld: Outbound Conn ERROR 0x%lx", pollFd->fd, sEvent.status )); 
		}
		break;

	case btLibSocketEventConnectedInbound:
		// Slave is connected by master
		index = FindSocketIndex(sEvent.eventData.newSocket);
		DbgOnlyFatalErrorIf( pollFd->fd != gFdListener, "not listener socket" );
		DbgOnlyFatalErrorIf( gDevices[index].connectionSocket != kIOsFdNull, "already connected" );

		BtLibAddrBtdToA(&sEvent.eventData.requestingDevice, gBdAddrStr, kBdAddrStrSize);				
		TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: in socket=%d New Inbound Conn on #%d, addr %s",
			sEvent.eventData.newSocket, index, gBdAddrStr )); 
		gDevices[0].connectionSocket = sEvent.eventData.newSocket;
		PbxAddFd( gPollBox, gDevices[0].connectionSocket, POLLIN, HandlePbxSocketEvent, &gDevices[0].connectionSocket );
		gNumConnections++;	
		FrmGotoForm(gDbResId, DrawingForm);
		StatusMsg("Draw");
		break;

	case btLibSocketEventDisconnected:
		// Master or slave lost a connection. Close socket and disconnect ACL
		TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxSocketEvent: Socket=%ld: Disconnect (event=%d)", pollFd->fd, sEvent.eventData.newSocket )); 
		index = FindSocketIndex(pollFd->fd);
		if ( pollFd->fd == gDevices[index].connectionSocket )
		{
			// An established data connection was broken.
			// Close the data socket.
			StatusMsg("HandlePbxSocketEvent: btLibSocketEventDisconnected");
			PbxRemoveFd( pbx, gDevices[index].connectionSocket );
			IOSClose( gDevices[index].connectionSocket );
			gDevices[index].connectionSocket = kIOsFdNull;
			gNumConnections--;
		}
		else if ( pollFd->fd == gFdListener )
		{
			// An inbound connection failed.
			// Must close both the listener socket and the new socket returned in the event.
			PbxRemoveFd( pbx, gFdListener );
			IOSClose( gFdListener );
			IOSClose( sEvent.eventData.newSocket );
			gFdListener = kIOsFdNull;
			gNumConnections--;
		}
		else
		{
			DbgOnlyFatalError( "wrong socket" );
		}
		break;
	}
}


/***********************************************************************
 *
 * FUNCTION:		HandlePbxMeEvent
 *
 * DESCRIPTION: Handle events from the Management Entity device.
 *
 ***********************************************************************/
static void HandlePbxMeEvent(
	struct PollBox* 				pbx,		// the PollBox
	struct pollfd*					pollFd, 	// fd, event mask, and received events
	void* 						context 	// not used here
) {
	status_t						error;
	int32_t 						flags;

	static BtLibManagementEventType mEvent;
	static char 					mData[sizeof(BtLibFriendlyNameType)];
	static struct strbuf			ctlBuf = { sizeof(mEvent), 0, (char*)&mEvent	 };
	static struct strbuf			datBuf = { sizeof(mData),  0, (char*)&mData[0] };

	// We must be here for a reason ...
	DbgOnlyFatalErrorIf( !(pollFd->revents & (POLLIN|POLLERR|POLLHUP|POLLNVAL) ), "no event flag" );

	// We must have the Management Entity file descriptor.
	DbgOnlyFatalErrorIf( pollFd->fd != gFdME, "not the FdMe" );
	DbgOnlyFatalErrorIf( pollFd->fd < 0, "gFdMe closed" );

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if (
		(pollFd->revents & (POLLERR|POLLHUP|POLLNVAL)) || 			// poll error or hangup
		IOSGetmsg( pollFd->fd, &ctlBuf, &datBuf, &flags, &error ) != 0	// getmsg error
	) {
		TraceOutput(TL( kAppWhiteBoardErrorClass, "HandlePbxBtEvent: closing fd %ld", pollFd->fd ));
		PbxRemoveFd( pbx, pollFd->fd );
		BtLibClose( pollFd->fd );		
		return;
	}

	// We must have an event struct in the control part.
	DbgOnlyFatalErrorIf( ctlBuf.len != sizeof(BtLibManagementEventType), "no event struct" );

	// Decode the event.
	switch ( mEvent.event )
	{
	case btLibManagementEventRadioState:
		{
			const char* s;

			switch ( mEvent.status )
			{
			case btLibErrRadioInitialized: s = "Radio Initialized"; break;
			case btLibErrRadioInitFailed:  s = "Radio Init Failed"; break;
			case btLibErrRadioFatal:			 s = "Radio Fatal Error"; break;
			case btLibErrRadioSleepWake:	 s = "Radio Wakeup";			break;
			default:											 s = "Radio Event 0x%lx"; break;
			}
			TraceOutput(TL( kAppWhiteBoardErrorClass, s, mEvent.status));
			
		}
		break;
		
	case btLibManagementEventPiconetCreated:
		break;
		
	case btLibManagementEventPiconetDestroyed:	
		FrmGotoForm(gDbResId, MainForm);
		break;
		
	case btLibManagementEventACLConnectOutbound:
		gPendingLinks--;				
		if (gIsMaster)
		{
			// Lock when outbound links are complete
			if(gPendingLinks == 0)
				BtLibPiconetLockInbound(gFdME);
			else
				AclCreateNextLink();
		}
		else
		{
			// We should have connected to a master of a Piconet, check that we are really a slave
			// (a master/slave switch should have occured)
			BtLibConnectionRoleEnum role;
			status_t error;
			error = BtLibLinkGetState(gFdME, &mEvent.eventData.bdAddr, btLibLinkPref_LinkRole, (void *)&role, sizeof(role));
			if (error || role != btLibSlaveRole)
			{
				BtLibLinkDisconnect(gFdME, &mEvent.eventData.bdAddr);
				break;
			}
		}
		// Intentional fall through

	case btLibManagementEventACLConnectInbound: 		
		BtLibAddrBtdToA(&mEvent.eventData.bdAddr, gBdAddrStr, kBdAddrStrSize);				
		if(mEvent.status == btLibErrNoError)
		{
			memmove(&gDevices[gNumDevices].bdAddr, &mEvent.eventData.bdAddr, sizeof(BtLibDeviceAddressType));
			StatusMsg("Link to %s Created on #%d", gBdAddrStr, gNumDevices);
			gNumDevices++;
		}
		else
			ErrorMsg("Link to %s Failed", gBdAddrStr);
			
		if (gIsMaster && gPendingLinks == 0)
			L2CapFindPsm();
		break;
		
	case btLibManagementEventACLDisconnect:
		BtLibAddrBtdToA(&mEvent.eventData.bdAddr, gBdAddrStr, 
			kBdAddrStrSize);		
		StatusMsg("%s Disc: %hx", gBdAddrStr, mEvent.status);
		SocketHandleDisconnect(&mEvent.eventData.bdAddr);					
		break;	
		
	case btLibManagementEventAccessibilityChange:
		{
			FormPtr 	frmP = FrmGetActiveForm();
			ControlPtr	ctrlP = NULL;
			if (FrmGetActiveFormID() == DrawingForm)
				ctrlP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, DrawingToggleLockButton));
			switch( mEvent.eventData.accessible)
			{
			case btLibNotAccessible:
				StatusMsg( "Not Accessible");
				if (ctrlP)
					CtlSetLabel(ctrlP, kUnlockStr);
				break;
			case btLibConnectableOnly:
				StatusMsg( "Connectable");
				if (ctrlP)
					CtlSetLabel(ctrlP, kLockStr);
				break;
			case btLibDiscoverableAndConnectable:
				StatusMsg( "Discoverable and Connectable");
				if (ctrlP)
					CtlSetLabel(ctrlP, kLockStr);
				break;
			}
		}
		break;
	}
}


/***********************************************************************
 *
 * FUNCTION:		HandlePbxUIEvent
 *
 * DESCRIPTION: Handle UI events.
 *
 ***********************************************************************/
static void HandlePbxUIEvent(
	PollBox*		pbx,		// the PollBox
	struct pollfd*	pollFd, 	// fd, event mask, and received events
	void* 		context 	// user context
) {
	EventType		event;
	status_t		error;

	UNUSED_ARG( pbx );
	UNUSED_ARG( pollFd );
	UNUSED_ARG( context );
	 
	do
	{
		EvtGetEvent(&event, SysTicksPerSecond() / 10);

		if (event.eType == appStopEvent)
		{
			gStopFlag = true;
		}

		if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);
	} while ( EvtEventAvail());
}

//-----------------------------------------
// until polling fd 0 works correctly.
//
#if CAN_POLL_FD_0
	#define MyPbxPoll( pbx, nReadyP ) PbxPoll( pbx, -1, nReadyP )
#else
static status_t MyPbxPoll(
	PollBox*	pbx,
	int32_t*	nReadyP
) {
	HandlePbxUIEvent( 0, 0, 0 );
		return PbxPoll( pbx, 1, nReadyP );
}
#endif

/***********************************************************************
 *
 * FUNCTION:		AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:		nothing
 *
 ***********************************************************************/
static void AppEventLoop( void )
{
	int32_t 	nReady;
	status_t	err;

	while ( !gStopFlag )
	{
		err = MyPbxPoll( gPollBox, &nReady );
		ErrFatalErrorIf( err, "poll error" );
		ErrFatalErrorIf( gPollBox->count == 0, "empty pollbox" );
	}
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Initialize the application
 *
 * PARAMETERS:   Nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 ***********************************************************************/
static status_t AppStart(void)
{
	status_t err;

    gSavedAutoOff = SysSetAutoOffTime(0);

	// Open our resource database.
	err = SysGetModuleDatabase( SysGetRefNum(), NULL, &gDbResId );
	if (!gDbResId)
	{
		TraceOutput(TL(kAppWhiteBoardErrorClass, "AppStart: DmOpenDatabaseByTypeCreator error"));
	}
	ErrFatalErrorIf( err, "can't get resource db" );


	// Create our pollbox, and put file descriptor 0 into it, for UI events.
	gPollBox = PbxCreate();
#if CAN_POLL_FD_0
	gFdUI = EvtGetEventDescriptor();
	PbxAddFd( gPollBox, gFdUI, POLLIN, HandlePbxUIEvent, NULL );
#else
	{
		uint32_t fd = IOSOpen( "NULL", O_RDWR, &err );
		DbgOnlyFatalErrorIf( fd < 0, "can't open NULL dev" );
		PbxAddFd( gPollBox, fd, POLLIN, NULL, NULL );
	}
#endif
  
	return BluetoothInit();  
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Terminate the application.
 *
 * PARAMETERS:  Nothing
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void AppStop(void)
{ 	
	SysSetAutoOffTime(gSavedAutoOff);

	BluetoothCleanup();

#if CAN_POLL_FD_0
	PbxRemoveFd(gPollBox, gFdUI);
#endif

	FrmCloseAllForms();

	PbxDestroy( gPollBox ); // note this closes any fds still in the box
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t error = 0;
//	uint32_t btVersion; 

	UNUSED_ARG( cmdPBP );
	UNUSED_ARG( launchFlags );
/*	
	// Check that Bluetooth components are intalled, which imply Palm OS 4.0 or upper
	if (FtrGet(btLibFeatureCreator, btLibFeatureVersion, &btVersion) != errNone)
	{
		// Alert the user if it's the active application
		if ((launchFlags & sysAppLaunchFlagNewGlobals) && (launchFlags & sysAppLaunchFlagUIApp))
			FrmAlert (gDbResId, MissingBtComponentsAlert);
		return sysErrRomIncompatible;
	}
*/
	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			TraceOutput(TL( kAppWhiteBoardErrorClass, "PilotMain: error= 0x%lX", error )); 
			if (!error) 
			{
				FrmGotoForm(gDbResId, MainForm);
				AppEventLoop();
			}
			AppStop();
			break;

		default:
			break;
	}
	
	return error;
}
