/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtHeadset.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	Bluetooth HEADSET Profile
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <StringMgr.h>
#include <SystemMgr.h>
#include <TraceMgr.h>
#include <BtLib.h>
#include <string.h>

#include "BtHeadsetRsc.h"

#define UI_EVENTS_ON_FD 0 // non-zero <=> can poll fd for ui events

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

typedef enum
{
	HSDisconnected,
	HSConnecting,
	HSConnectingSCO,
	HSConnected,
	HSDisconnecting
}
HSState;

typedef struct _HSGlobalsType
{
	int32_t gFdUI;								// fd for UI events
	int32_t gFdME;								// fd to Management Entity device
	int32_t gFdSdp;								// fd to SDP device
	int32_t gFdRfcData;							// fd to RfComm device, for data
	int32_t gFdSocketSCO;						//fd to SCO
	
	PollBox *gPollBox;							// A PollBox is a collection of file descriptors, for polling.

	DmOpenRef gResourceDbRef;					// The local ID of this app's resource data base.

	Boolean gStopFlag;

	BtLibDeviceAddressType gBdAddr;				// Remote bluetooth device's address.

	HSState gHSConnected;						// Headset connection state

	BtLibSdpUuidType gSdpUUIDList[3];			// UUID List

	BtLibSdpRecordHandle gSdpRecordHandle;		// SDP service record handle.

	uint16 gRemoteVolumeSize;					// To know if the Headset supports the remote audio volume control 
	Boolean gRemoteVolume;
	uint8 *gRemoteVolumeP;
	Int32 gSpeakerGain;							// To recuperate the value of the speaker gain
	Int32 gMicroGain;							// To recuperate the value of the micro gain
	
	uint8_t *gNumSelectedPtr;					// For receiving the number of selected devices during the BT Discovering devices

	Char gCommand[200];							// Buffer for AT Commands

	int16_t gPrevFormId; 						// Current form upon entry when server
	uint16_t gAIAState;							// State of active graffiti input are upon entry
	Boolean gService;	 						// if true = our service has been executed

	DmOpenRef gHeadSetDmOpenRef;

} HSGlobalsType;

/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
static	HSGlobalsType gHS;

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

#define appFileCreator			'BTHS'	// register your own at http://www.palmos.com/dev/creatorid/
#define UNUSED_ARG(x) (x=x)		// To avoid compiler warning about unused arguments
#define kMyMtu				100	// Must be less then 672
#define kInitialCredits			4
#define HEADSET_GW_SERVICE_NAME		"Voice gateway"	// Headset gateway service username




static void HandlePbxSCOSocketEvent(struct PollBox *pbx, struct pollfd *pollFd, void *context);
static void BackgroundUIInit(void);
static void PrvRemoteVolumeService(void);


/***********************************************************************
 *
 * FUNCTION:    PrvDisplayRemoteName
 *
 * DESCRIPTION: This routine gets the user-friendly name of the Headset 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
void PrvDisplayRemoteName(void)
{

	status_t error;
	FormPtr frmP;

	error = BtLibGetRemoteDeviceName(gHS.gFdME, &gHS.gBdAddr, btLibCachedOnly);
	if (error != btLibErrPending)
	{
		frmP = FrmGetActiveForm();
		FrmCopyLabel(frmP, MainRemoteLabel, "can not get name");
	}	

}

/***********************************************************************
 *
 * FUNCTION:    PrvChangeHSState
 *
 * DESCRIPTION: This routine changes the device state
 *
 * PARAMETERS:  state - state of the device
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
void PrvChangeHSState(HSState state)
{
	if (gHS.gHSConnected != state)
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : PrvChangeHSState: %d -> %d", gHS.gHSConnected, state));
		gHS.gHSConnected = state;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvDisplayUI
 *
 * DESCRIPTION:	Display the user interface
 *
 * PARAMETERS:	displayButton - show right button depending on HS state
 *				displayVolume - update volume info and buttons
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvDisplayUI(Boolean displayButton, Boolean displayVolume)
{
	FormPtr frmP = FrmGetActiveForm();

	if (displayButton)
	{
		switch(gHS.gHSConnected)
		{
			case HSDisconnected:
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainDisconnectButton));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainConnectButton));
				break;

			case HSConnecting:
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSelectButton));
				break;

			case HSConnectingSCO:
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainConnectButton));
				if (gHS.gService == true)
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSelectButton));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainDisconnectButton));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerLabel));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerGainLabel));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroLabel));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroGainLabel));
				break;

			case HSConnected:
			case HSDisconnecting:
			
				break;
		}
	}

	if (displayVolume)
	{
		FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMoinsRepeating));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainPlusRepeating));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroMoinsRepeating));
		FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroPlusRepeating));
	}
}

/***********************************************************************
 *
 * FUNCTION:   	PrvSocketSend
 *
 * DESCRIPTION:	This routine sends data over the RfCommsocket
 *
 * PARAMETERS:	socket - connected RfComm socket
 *				data - buffer containing the data to send
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
status_t PrvSocketSend(BtLibSocketRef socket, char *data)
{
	TraceOutput(TL(appErrorClass, "BtHeadset : PrvSocketSend: %s", data));
	return BtLibSocketSend(socket, (uint8_t*)data, strlen(data));
}

/***********************************************************************
 *
 * FUNCTION:   	PrvParseCommand
 *
 * DESCRIPTION:	This routine processes commands sent by the Headset over the 
 *				RfCommsocket
 *
 * PARAMETERS:	command - pointer to the command to process
 *				len - length of the command
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
void PrvParseCommand(Char * command, uint16_t len)
{
	BtLibSocketConnectInfoType connectInfo;
	FormPtr frmP;
	Char str[200];
	Char *strP;
	Char *strCommandP;
	Char *strNextP;
	uint16_t aLen;
	status_t error;

	aLen = StrLen(gHS.gCommand) + len;
	StrNCat(gHS.gCommand, command, (Int16) aLen + 1);
	gHS.gCommand[aLen] = 0;
	TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: %s", gHS.gCommand));
	strCommandP = gHS.gCommand;

	while (StrLen(strCommandP))
	{
		if ((strNextP = StrStr(strCommandP, "\r")) == NULL)
			break;

		StrNCopy(str, strCommandP, 3);

		str[3] = 0;
		if (StrCompare(str, "AT+") != 0)
		{
			PrvSocketSend(gHS.gFdRfcData, "\r\nERROR\r\n");
		}
		else
		{
			strCommandP += 3;
			aLen = strNextP - strCommandP;
			StrNCopy(str, strCommandP, aLen);
			str[aLen] = 0;
			TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: found \"%s\"", str));

			if (StrCompare(str, "CKPD=200") == 0)
			{

				PrvSocketSend(gHS.gFdRfcData, "\r\nOK\r\n");
				switch (gHS.gHSConnected)
				{
					case HSConnecting:
						BtLibSocketGetInfo(gHS.gFdRfcData, btLibSocketInfo_RemoteDeviceAddress, &gHS.gBdAddr, sizeof(BtLibDeviceAddressType));
						connectInfo.remoteDeviceP = &gHS.gBdAddr;
						error = BtLibSocketCreate(&gHS.gFdSocketSCO, btLibSCOProtocol);
						if (error)
						{
							TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: Error while creating SCO socket [%lu]", (unsigned long) error));
							BtLibSocketClose(gHS.gFdRfcData);
							PbxRemoveFd(gHS.gPollBox, gHS.gFdRfcData);
							gHS.gFdRfcData = -1;
							TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: RFComm socket closed because Error while creating SCO Socket"));
							BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
							TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: ACL Disconnected because Error while creating SCO Socket"));
							break;
						}

						PbxAddFd(gHS.gPollBox, gHS.gFdSocketSCO, POLLIN, HandlePbxSCOSocketEvent, &gHS.gFdSocketSCO);

						error = BtLibSocketConnect(gHS.gFdSocketSCO, &connectInfo);
						if (error != btLibErrPending)
						{
							TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: Error while connecting SCO socket [%lu]", (unsigned long) error));
							BtLibSocketClose(gHS.gFdSocketSCO);
							PbxRemoveFd(gHS.gPollBox, gHS.gFdSocketSCO);
							gHS.gFdSocketSCO = -1;

							BtLibSocketClose(gHS.gFdRfcData);
							PbxRemoveFd(gHS.gPollBox, gHS.gFdRfcData);
							gHS.gFdRfcData = -1;

							BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
							TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: ACL Disconnected because Error while connecting SCO Socket"));
							PrvChangeHSState(HSDisconnecting);
						}
						else
							PrvChangeHSState(HSConnectingSCO);
						break;

					case HSConnected:
						TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: Disconnect SCO"));
						BtLibSocketClose(gHS.gFdSocketSCO);
						PbxRemoveFd(gHS.gPollBox, gHS.gFdSocketSCO);
						gHS.gFdSocketSCO = -1;

						BtLibSocketClose(gHS.gFdRfcData);
						PbxRemoveFd(gHS.gPollBox, gHS.gFdRfcData);
						gHS.gFdRfcData = -1;

						BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
						TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: ACL Disconnected because AT+CKPD=200 received"));
						PrvChangeHSState(HSDisconnecting);
						break;
					default:
						TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: gHSConnected ->%d",gHS.gHSConnected));
						break;

						
				}
			}
			else if ((strP = StrStr(str, "VGM=")) != NULL)
			{
				PrvSocketSend(gHS.gFdRfcData, "\r\nOK\r\n");
				strP += 4;
				gHS.gMicroGain = StrAToI(strP);
				TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: new micro gain is : %d", gHS.gMicroGain));
				frmP = FrmGetActiveForm();
				FrmCopyLabel(frmP, MainMicroGainLabel, StrIToA(str, gHS.gMicroGain));

			}
			else if ((strP = StrStr(str, "VGS=")) != NULL)
			{
				PrvSocketSend(gHS.gFdRfcData, "\r\nOK\r\n");
				strP += 4;
				gHS.gSpeakerGain = StrAToI(strP);
				TraceOutput(TL(appErrorClass, "BtHeadset : PrvParseCommand: new speaker gain is : %d", gHS.gSpeakerGain));
				frmP = FrmGetActiveForm();
				FrmCopyLabel(frmP, MainSpeakerGainLabel, StrIToA(str, gHS.gSpeakerGain));
			}
			else
			{
				PrvSocketSend(gHS.gFdRfcData, "\r\nERROR\r\n");
			}
		}

		// Go to next command
		strCommandP = strNextP;
		while ((*strCommandP == '\r') || (*strCommandP == '\n'))
			strCommandP++;
	}

	if (strCommandP != gHS.gCommand)
		MemMove(gHS.gCommand, strCommandP, (int32_t) StrLen(strCommandP) + 1);

}

/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the main form.
 *
 * PARAMETERS:  frmP - formptr of active window
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void MainFormInit(FormPtr frmP)
{
	MemSet(&gHS.gBdAddr, sizeof(gHS.gBdAddr), 0);
	
}

/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    true if handled
 *
 ***********************************************************************/
static Boolean MainFormDoCommand(uint16_t command)
{
	Boolean handled = false;


	switch (command)
	{
		case MainOptionsAboutStarterApp:
			AbtShowAbout(appFileCreator);
			handled = true;
			break;

		default:
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr eventP)
{
	Boolean handled = false;
	status_t error = 0;
	FormPtr frmP;
	Char str[15];
	BtLibClassOfDeviceType codType = btLibCOD_Audio | btLibCOD_Major_Audio | btLibCOD_Minor_Audio_Headset;

	switch (eventP->eType)
	{
		case menuEvent:
			{
				handled = MainFormDoCommand(eventP->data.menu.itemID);
				break;
			}

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case MainSelectButton:
					// Perform the Bluetooth Headset Devices discovery and recuperate the name of the selected device using the 
					// PrvDisplayRemoteName function. Else, the name is displayed. 
					error = BtLibDiscoverDevices(gHS.gFdME, "Headset Devices", NULL, false, &codType, 1, false, &gHS.gBdAddr, 1, gHS.gNumSelectedPtr);
					if (error == btLibErrNoError)
					{
						// Display the Connect Button in order to permit the user to begin the connection
						PrvDisplayUI(true, false);
						PrvDisplayRemoteName();
					}
					break;
					
				case MainConnectButton:	// Begin the communication if the Gateway is disconnected
				case MainDisconnectButton:	// Stop the communication if the Gateway is connected
					switch (gHS.gHSConnected)
					{
						case HSDisconnected:
							// The Gateway initiates an ACL connection.
							PrvChangeHSState(HSConnecting);
							BtLibLinkConnect(gHS.gFdME, &gHS.gBdAddr);
							PrvDisplayUI(true, false);
							break;

						case HSConnected:
							// The Gateway initiates a disconnection : release the SCO Link, close the RfComm Socket & close the ACL link  
							BtLibSocketClose(gHS.gFdSocketSCO);
							PbxRemoveFd(gHS.gPollBox, gHS.gFdSocketSCO);
							gHS.gFdSocketSCO = -1;

							BtLibSocketClose(gHS.gFdRfcData);
							PbxRemoveFd(gHS.gPollBox, gHS.gFdRfcData);
							gHS.gFdRfcData = -1;

							BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
							PrvChangeHSState(HSDisconnecting);
							TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: ACL Disconnected because MainDisconnectButton selected"));
							break;
					}
					break;
			}
			break;

		case frmOpenEvent:
			{
				FormPtr frmP = FrmGetFormPtr(eventP->data.frmOpen.formID);
				MainFormInit(frmP);
				WinInvalidateWindow( FrmGetWindowHandle(frmP));
				handled = true;
				break;
			}

		case frmCloseEvent:
			{
				FormPtr frmP = FrmGetFormPtr(eventP->data.frmOpen.formID);
				FrmEraseForm(frmP);
				FrmDeleteForm(frmP);
				handled = true;
				break;
			}

		case winUpdateEvent:
			{
			FormPtr	frmP = FrmGetFormPtr(MainForm);
			FrmDrawForm (frmP);
			handled = true;
			break;
			}

		case appStopEvent:
			gHS.gStopFlag = true;
			break;


		case ctlRepeatEvent:
			// The Gateway controls the remote Headset volume using repeating button
			switch (eventP->data.ctlRepeat.controlID)
			{
				case MainMoinsRepeating:
					// Diminuate the volume : send an +VGS = <value> command, then display the new gain of the speaker
					TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainMoinsRepeating"));
					if (gHS.gSpeakerGain > 0)
					{
						gHS.gSpeakerGain--;
						StrPrintF(str, "\r\n+VGS=%d\r\n", gHS.gSpeakerGain);
						PrvSocketSend(gHS.gFdRfcData, str);
						frmP = FrmGetActiveForm();
						FrmCopyLabel(frmP, MainSpeakerGainLabel, StrIToA(str, gHS.gSpeakerGain));
						TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainMoinsRepeating :  -1"));
					}
					break;

				case MainPlusRepeating:
					// Increase the volume : send an +VGS = <value> command, then display the new gain of the speaker
					TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainPlusRepeating"));
					if (gHS.gSpeakerGain < 15)
					{
						gHS.gSpeakerGain++;
						StrPrintF(str, "\r\n+VGS=%d\r\n", gHS.gSpeakerGain);
						PrvSocketSend(gHS.gFdRfcData, str);
						frmP = FrmGetActiveForm();
						FrmCopyLabel(frmP, MainSpeakerGainLabel, StrIToA(str, gHS.gSpeakerGain));
						TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainPlusRepeating :  +1")); 
					}
					break;

				case MainMicroMoinsRepeating:
					// Diminuate the micro gain : send an +VGM = <value> command, then display the new gain of the micro
					TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainMicroMoinsRepeating"));
					if (gHS.gMicroGain > 0)
					{
						gHS.gMicroGain--;
						StrPrintF(str, "\r\n+VGM=%d\r\n", gHS.gMicroGain);
						PrvSocketSend(gHS.gFdRfcData, str);
						frmP = FrmGetActiveForm();
						FrmCopyLabel(frmP, MainMicroGainLabel, StrIToA(str, gHS.gMicroGain));
						TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainMicroMoinsRepeating :  -1"));
					}
					break;

				case MainMicroPlusRepeating:
					// Increase the micro gain : send an +VGM = <value> command, then display the new gain of the micro
					TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainMicroPlusRepeating"));
					if (gHS.gMicroGain < 15)
					{
						gHS.gMicroGain++;
						StrPrintF(str, "\r\n+VGM=%d\r\n", gHS.gMicroGain);
						PrvSocketSend(gHS.gFdRfcData, str);
						frmP = FrmGetActiveForm();
						FrmCopyLabel(frmP, MainMicroGainLabel, StrIToA(str, gHS.gMicroGain));
						TraceOutput(TL(appErrorClass, "BtHeadset : MainFormHandleEvent: MainMicroPlusRepeating :  +1")); 
					}
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	if (eventP->eType == frmLoadEvent)
	{
		DmOpenRef formDb = eventP->data.frmLoad.formDatabase;
		uint16_t formID = eventP->data.frmLoad.formID;
		FormType *formP = FrmInitForm(formDb, formID);
		WinHandle winH = FrmGetWindowHandle(formP);

		WinSetDrawWindow(winH);
		FrmSetActiveForm(formP);
		if (formID == MainForm)
		{
			FrmSetEventHandler(formP, MainFormHandleEvent);
		}
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    HandlePbxSCOSocketEvent
 *
 * DESCRIPTION: Handle events from "SCO socket" devices.
 *
 * PARAMETERS:  pbx		- the PollBox
 *				pollFd	- fd, event mask, and received events
 *				context	- in this app, ptr to global fd variable
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void HandlePbxSCOSocketEvent(struct PollBox *pbx, struct pollfd *pollFd, void *context)
{
	int32_t *fdPtr = (int32_t *) context;
	status_t error;
	int32_t flags;
	static BtLibSocketEventType sEvent;
	static char sData[kMyMtu];
	static struct strbuf ctlBuf = { sizeof(sEvent), 0, (char *) &sEvent };
	static struct strbuf datBuf = { sizeof(sData), 0, (char *) &sData[0] };

	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSCOSocketEvent: fd=%d, *context=%d", pollFd->fd, *fdPtr));

	// We must be here for a reason ...
	ErrFatalErrorIf(!(pollFd->revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL)), "no event flag");

	// Our context variable points to our global variable containing the file descriptor.
	ErrFatalErrorIf(fdPtr != &gHS.gFdSocketSCO, "bad context");
	ErrFatalErrorIf(pollFd->fd < 0, "bad fd value");
	ErrFatalErrorIf(pollFd->fd != *fdPtr, "context not fd");

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if ((pollFd->revents & (POLLERR | POLLHUP | POLLNVAL)) ||	// poll error or hangup
		IOSGetmsg(pollFd->fd, &ctlBuf, &datBuf, &flags, &error) != 0	// getmsg error
		)
	{
		PbxRemoveFd(pbx, pollFd->fd);
		BtLibSocketClose(pollFd->fd);
		*fdPtr = -1;
		return;
	}

	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSCOSocketEvent: %ld bytes ctl, %ld bytes data", ctlBuf.len, datBuf.len));

	// Check for pure data.
	if (ctlBuf.len == -1)
	{
		return;
	}


	// Since it's not pure data, we must have an event struct in the control part.
	ErrFatalErrorIf(ctlBuf.len != sizeof(BtLibSocketEventType), "no event struct");
	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSCOSocketEvent: event %s, status 0x%lx", BtLibSocketEventName(sEvent.event), sEvent.status));

	// Decode the event.
	switch (sEvent.event)
	{
		case btLibSocketEventConnectedOutbound:
			// Outbound SCO connection succeeded.
			if (sEvent.status == btLibErrNoError)
			{
				// Display the appropriate ressources
				TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSCOSocketEvent: SCO connected [0x%04lX]", (unsigned long) sEvent.status));
				PrvDisplayUI(true, gHS.gRemoteVolume);
				PrvRemoteVolumeService(); // Get the capability to support the remote audio service control
				// Audio Gateway Connected
				PrvChangeHSState(HSConnected);
				break;
			}

		default:
			TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSCOSocketEvent: socket receives [0x%02lX]", (unsigned long) sEvent.event));
			break;
	}

}

/***********************************************************************
 *
 * FUNCTION:    HandlePbxSocketEvent
 *
 * DESCRIPTION: Handle events from "socket" devices (RfComm, SDP).
 *
 * PARAMETERS:	pbx		- the PollBox
 *				pollFd	- fd, event mask, and received events
 *				context	- in this app, ptr to global fd variable
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void HandlePbxSocketEvent(struct PollBox *pbx,	struct pollfd *pollFd, void *context)
{
	int32_t *fdPtr = (int32_t *) context;
	status_t error;
	int32_t flags;

	BtLibSocketConnectInfoType connectInfo;

	static BtLibSocketEventType sEvent;
	static char sData[kMyMtu];
	static struct strbuf ctlBuf = { sizeof(sEvent), 0, (char *) &sEvent };
	static struct strbuf datBuf = { sizeof(sData), 0, (char *) &sData[0] };

	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: fd=%d, *context=%d", pollFd->fd, *fdPtr));

	// We must be here for a reason ...
	ErrFatalErrorIf(!(pollFd->revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL)), "no event flag");

	// Our context variable points to our global variable containing the file descriptor.
	ErrFatalErrorIf(fdPtr != &gHS.gFdSdp && fdPtr != &gHS.gFdRfcData, "bad context");
	ErrFatalErrorIf(pollFd->fd < 0, "bad fd value");
	ErrFatalErrorIf(pollFd->fd != *fdPtr, "context not fd");

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if ((pollFd->revents & (POLLERR | POLLHUP | POLLNVAL)) ||	// poll error or hangup
		IOSGetmsg(pollFd->fd, &ctlBuf, &datBuf, &flags, &error) != 0	// getmsg error
		)
	{
		PbxRemoveFd(pbx, pollFd->fd);
		BtLibSocketClose(pollFd->fd);
		*fdPtr = -1;
		return;
	}

	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: %ld bytes ctl, %ld bytes data", ctlBuf.len, datBuf.len));

	// Check for pure data.
	// Reception of the messages sent by the Headset
	if (ctlBuf.len == -1)
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: Data:"));
		TraceOutput(B(appErrorClass, (uint8_t *) datBuf.buf, (uint32_t) datBuf.len));

		// Advance one more credit.
		BtLibSocketAdvanceCredit(pollFd->fd, 1);
		// Process the message
		PrvParseCommand((char *) datBuf.buf, (uint16_t) datBuf.len);
		return;
	}


	// Since it's not pure data, we must have an event struct in the control part.
	ErrFatalErrorIf(ctlBuf.len != sizeof(BtLibSocketEventType), "no event struct");
	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: event %s, status 0x%lx", BtLibSocketEventName(sEvent.event), sEvent.status));

	// Decode the event.
	switch (sEvent.event)
	{
	/***********************
  	**	  SDP Events	  **
	***********************/
		case btLibSocketEventSdpGetServerChannelByUuid:
			//  The Gateway received the RfComm Server Channel, it can perform a RfComm connection using the channel 
			if (sEvent.status != btLibErrNoError)
			{
				BtLibSocketClose(gHS.gFdSdp);
				PbxRemoveFd(gHS.gPollBox, gHS.gFdSdp);
				gHS.gFdSdp = -1;
				BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
				TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: ACL Disconnected because Error while getting server channel"));
			}
			else
			{
				//  Close the SDP socket
				BtLibSocketClose(gHS.gFdSdp);
				PbxRemoveFd(gHS.gPollBox, gHS.gFdSdp);
				gHS.gFdSdp = -1;
				// Create a RfComm Socket
				error = BtLibSocketCreate(&gHS.gFdRfcData, btLibRfCommProtocol);
				if (error)
				{
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: Error while creating RfComm socket [%lu]", (unsigned long) error));
					BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: ACL Disconnected because Error while creating RfComm socket"));
					break;
				}

				PbxAddFd(gHS.gPollBox, gHS.gFdRfcData, POLLIN, HandlePbxSocketEvent, &gHS.gFdRfcData);
				// Configure the RfComm Socket
				connectInfo.remoteDeviceP = &gHS.gBdAddr;
				connectInfo.data.RfComm.remoteService = sEvent.eventData.sdpByUuid.param.channel;	//using the appropriate channel
				connectInfo.data.RfComm.maxFrameSize = 500;
				connectInfo.data.RfComm.advancedCredit = 1;
				TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: calling SocketConnect, requesting maxFrameSize=%hu", (uint16_t) 500));
				// Perform the RfComm Socket connection 
				error = BtLibSocketConnect(gHS.gFdRfcData, &connectInfo);

				if (error != btLibErrPending)
				{
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: Socket RFComm - error [0x%lX] during connect", (unsigned long) error));
				}
			}
			break;


		case btLibSocketEventSdpServiceRecordHandle:
			// A handle to the remote service record has been received. The remote Handle is contained in the Data part of the event
			if (sEvent.status == btLibErrNoError)
			{
				error = BtLibSdpServiceRecordCreate(gHS.gFdME, &gHS.gSdpRecordHandle);
				// Map the empty, local service record created (gSdpRecordHandle) to the given remote service record
				error = BtLibSdpServiceRecordMapRemote(gHS.gFdSdp, &gHS.gBdAddr, *(BtLibSdpRemoteServiceRecordHandle *) sData /*remoteHandle */ , gHS.gSdpRecordHandle);
				gHS.gRemoteVolumeSize = (uint16_t) sizeof(Boolean);
				// 0x302 = Remote audio volume control Attribute ID
				error = BtLibSdpServiceRecordGetSizeOfRawAttribute(gHS.gFdME, gHS.gSdpRecordHandle, 0x302, &gHS.gRemoteVolumeSize);

			}
			else
			{
				BtLibSocketClose(gHS.gFdSdp);
				PbxRemoveFd(gHS.gPollBox, gHS.gFdSdp);
				gHS.gFdSdp = -1;
			}
			break;


		case btLibSocketEventSdpGetRawAttributeSize:
			// The size of value of the remote audio volume control attribute has been received.
			if (sEvent.status == btLibErrNoError)
			{
				gHS.gRemoteVolumeP = MemPtrNew(sEvent.eventData.sdpAttribute.info.valSize);
				// Get the value of the remote audio volume control attribute
				error = BtLibSdpServiceRecordGetRawAttribute(gHS.gFdME, gHS.gSdpRecordHandle, 0x302, gHS.gRemoteVolumeP, &sEvent.eventData.sdpAttribute.info.valSize);

			}
			else
			{
				TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: Error while getting the size of the raw attribute - error [0x%lX] ", (unsigned long) sEvent.status));
				BtLibSocketClose(gHS.gFdSdp);
				PbxRemoveFd(gHS.gPollBox, gHS.gFdSdp);
				gHS.gFdSdp = -1;
			}
			break;

		case btLibSocketEventSdpGetRawAttribute:
			// The value of the remote service audio control attribute has been received.
			if (sEvent.status == btLibErrNoError)
			{
				// Verify that the value received has the format expected.
				// Recuperate the value and display the repeating buttons if the Audio Gateway is connected
				if (sData[0] != (btLibDETD_BOOL | btLibDESD_1BYTE))
				{
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: Socket SDP - Wrong type"));
					BtLibLinkDisconnect(gHS.gFdME, &gHS.gBdAddr);
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: ACL Disconnect because Socket SDP - Wrong type"));

				}
				else
				{
					gHS.gRemoteVolume = sData[1];
					if (gHS.gRemoteVolume)
						PrvDisplayUI(false, true && gHS.gService); 
				}
			}

			MemPtrFree(gHS.gRemoteVolumeP);
			BtLibSocketClose(gHS.gFdSdp);
			PbxRemoveFd(gHS.gPollBox, gHS.gFdSdp);
			gHS.gFdSdp = -1;
			break;



	/***********************
  	**	 RfComm Events	  **
	***********************/
		case btLibSocketEventConnectedOutbound:
			// Outbound connection succeeded.
			// The Gateway has initiated a RfComm Socket (Data) Connection, once connected the gateway sends a "RING" and waits the 
			//"AT+CKPD=200" response before establish the Audio (SCO) connection. 
			TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: RFComm connected (Outbound Connection)"));
			PrvDisplayRemoteName();
			PrvSocketSend(gHS.gFdRfcData, "\r\nRING\r\n");
			// To recuperate the remote audio service control, the Gateway uses a SDP connection
			error = BtLibSocketCreate(&gHS.gFdSdp, btLibSdpProtocol);
			PbxAddFd(gHS.gPollBox, gHS.gFdSdp, POLLIN, HandlePbxSocketEvent, &gHS.gFdSdp);
			BtLibSdpServiceRecordsGetByServiceClass(gHS.gFdSdp, &gHS.gBdAddr, gHS.gSdpUUIDList, 3);

			break;

		case btLibSocketEventSendComplete:
			TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxSocketEvent: Socket Send Complete"));
			break;

		default:
			break;

	}
}

/***********************************************************************
 *
 * FUNCTION:    HandlePbxMEEvent
 *
 * DESCRIPTION: Handle events from the Management Entity device.
 *
 * PARAMATERS: pbx		- the PollBox
 *				pollFd	- fd, event mask, and received events
 *				context	- in this app, ptr to global fd variable
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void HandlePbxMEEvent(struct PollBox *pbx, struct pollfd *pollFd, void *context)
{
	status_t error;
	int32_t flags;
	FormPtr frmP;

	static BtLibManagementEventType mEvent;
	static char mData[sizeof(BtLibFriendlyNameType)];
	static struct strbuf ctlBuf = { sizeof(mEvent), 0, (char *) &mEvent };
	static struct strbuf datBuf = { sizeof(mData), 0, (char *) &mData[0] };


	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: fd=%d", pollFd->fd));

	// We must be here for a reason ...
	ErrFatalErrorIf(!(pollFd->revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL)), "no event flag");

	// We must have the Management Entity file descriptor.
	ErrFatalErrorIf(pollFd->fd != gHS.gFdME, "not the ME fd");
	ErrFatalErrorIf(pollFd->fd < 0, "ME fd closed");

	// Check for error/eof from poll, read the event message.
	flags = 0;
	if ((pollFd->revents & (POLLERR | POLLHUP | POLLNVAL)) ||	// poll error or hangup
		IOSGetmsg(pollFd->fd, &ctlBuf, &datBuf, &flags, &error) != 0	// getmsg error
		)
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: closing fd=%d", pollFd->fd));
		PbxRemoveFd(pbx, pollFd->fd);
		BtLibClose(pollFd->fd);
		gHS.gFdME = -1;
		return;
	}

	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: %d bytes ctl, %d bytes data", ctlBuf.len, datBuf.len));

	// We must have an event struct in the control part.
	ErrFatalErrorIf(ctlBuf.len != sizeof(BtLibManagementEventType), "no event struct");

	TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: event %s, status 0x%lx", BtLibMEEventName(mEvent.event), mEvent.status));

	// Decode the event.
	switch (mEvent.event)
	{
		case btLibManagementEventNameResult:
			frmP = FrmGetActiveForm();
			// Display the Headset's name (or the address) in the appropriate label
			if (mEvent.status == btLibErrNoError)
			{
				if (((BtLibFriendlyNameType *) mData)->nameLength <= 1)
				{
					BtLibAddrBtdToA(&gHS.gBdAddr, (char *)mData, btLibMaxDeviceNameLength);
					FrmCopyLabel(frmP, MainRemoteLabel, mData);
				}
				else
					FrmCopyLabel(frmP, MainRemoteLabel, (char*)((BtLibFriendlyNameType*)mData)->name);
			}
			else
			{
				BtLibAddrBtdToA(&gHS.gBdAddr, (char *)mData, btLibMaxDeviceNameLength);
				FrmCopyLabel(frmP, MainRemoteLabel, mData);
			}
			break;


		case btLibManagementEventACLConnectOutbound:
			// The Gateway has initiated an ACL connection. The Gateway recuperates, using a SDP socket, the server channel corresponding to services (UUIDs List) it needs
			if (mEvent.status == btLibErrNoError)
			{
				TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: Connected"));
				error = BtLibSocketCreate(&gHS.gFdSdp, btLibSdpProtocol);

				if (error)
				{
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: Error while creating SDP socket [%lu]", (unsigned long) error));
					break;
				}

				PbxAddFd(gHS.gPollBox, gHS.gFdSdp, POLLIN, HandlePbxSocketEvent, &gHS.gFdSdp);
				error = BtLibSdpGetServerChannelByUuid(gHS.gFdSdp, &gHS.gBdAddr, gHS.gSdpUUIDList, 3);
				if (error != btLibErrPending)
				{
					TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: Error while getting server channel by UUID [%lu]", (unsigned long) error));
				}
			}
			else
			{
				TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: Connect failed"));
				PrvChangeHSState(HSDisconnected);
				PrvDisplayUI(true, false);
			}
			break;


		case btLibManagementEventACLConnectInbound:
			// The Headset has initiated an ACL connection
			TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: ACL Inbound Connection"));
			PrvChangeHSState(HSConnecting);
			break;


		case btLibManagementEventACLDisconnect:
			// ACL link disconnected : display the initial form
			TraceOutput(TL(appErrorClass, "BtHeadset : HandlePbxMEEvent: ACL Disconnected"));

			gHS.gStopFlag = true;

			break;


	}
}

/***********************************************************************
 *
 * FUNCTION:    HandlePbxUIEvent
 *
 * DESCRIPTION: Handle UI events.
 *
 * PARAMATERS: pbx		- the PollBox
 *				pollFd	- fd, event mask, and received events
 *				context	- in this app, ptr to global fd variable
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void HandlePbxUIEvent( struct PollBox *pbx, struct pollfd *pollFd, void *context)
{
	EventType		event;
	status_t		error;

	UNUSED_ARG(pbx);
	UNUSED_ARG(pollFd);
	UNUSED_ARG(context);

	while ( EvtEventAvail() )
	{
	
		EvtGetEvent( &event, evtNoWait );

		if ( event.eType == appStopEvent ) {
			gHS.gStopFlag = true;
		}
		
		if (
			!SysHandleEvent( &event )				&&
			!MenuHandleEvent( 0, &event, &error )	&&
			!AppHandleEvent( &event )
		) {
			FrmDispatchEvent( &event );
		}
	}
	#if UI_EVENTS_ON_FD
	EvtFinishLastEvent();
#endif
}


//-----------------------------------------
// Hack until polling fd 0 works correctly.
//
static status_t MyPbxPoll(PollBox * pbx, int32_t * nReadyP)
{
#if UI_EVENTS_ON_FD
	return PbxPoll(pbx, -1, nReadyP);
#else
	HandlePbxUIEvent(0, 0, 0);
	return PbxPoll(pbx, 250, nReadyP);
#endif
}

/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	int32_t nReady;
	status_t error;

	while (!gHS.gStopFlag)
	{
		error = MyPbxPoll(gHS.gPollBox, &nReady);
		//TraceOutput(TL(appErrorClass, "BtHeadset : AppEventLoop: MyPbxPoll returns nReady=%d, error=0x%lx", nReady, error));
		ErrFatalErrorIf(error, "poll error");
		ErrFatalErrorIf(gHS.gPollBox->count == 0, "empty pollbox");
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvRemoteVolumeService
 *
 * DESCRIPTION: Determine if the Headset supports the remote audio service control
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
void PrvRemoteVolumeService(void)
{
	status_t error;
	
	TraceOutput(TL(appErrorClass, "BtHeadset : PrvRemoteVolumeService : ENTER"));
	
	// Recuperate the remote audio service control
	error = BtLibSocketCreate(&gHS.gFdSdp, btLibSdpProtocol);
	if (error)
		TraceOutput(TL(appErrorClass, "BtHeadset : PrvRemoteVolumeService: Sdp Socket create error [0x%lX]", (unsigned long) error));
	PbxAddFd(gHS.gPollBox, gHS.gFdSdp, POLLIN, HandlePbxSocketEvent, &gHS.gFdSdp);
	BtLibSdpServiceRecordsGetByServiceClass(gHS.gFdSdp, &gHS.gBdAddr, gHS.gSdpUUIDList, 3);
	if (error)
		TraceOutput(TL(appErrorClass, "BtHeadset : PrvRemoteVolumeService: Get Sdp Service Records error [0x%lX]", (unsigned long) error));
}

/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Initialize the application.
 *
 * PARAMETERS:   Nothing
 *
 * RETURNED:     BtLibErrNoError if the file descriptor to the Management Entity
 *				 device opened successfully
 *
 ***********************************************************************/
static status_t AppStart(void)
{
	status_t error;

	TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: ENTER"));

	// Open our resource database.
	error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gHS.gResourceDbRef);
	if (!gHS.gResourceDbRef)
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: DmOpenDatabaseByTypeCreator() error=0x%lX", error));
		return error;
	}
	
	// Create our pollbox, and put the UI event file descriptor into it.
	gHS.gPollBox = PbxCreate();
	
	if ( !gHS.gPollBox )
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: PbxCreate() error Not enough memory"));
		error = memErrNotEnoughSpace;
		return error;
	}

	gHS.gFdUI = IOSOpen("NULL", O_RDWR, &error);
	ErrFatalErrorIf(gHS.gFdUI < 0, "can't open NULL dev");
	error = PbxAddFd(gHS.gPollBox, gHS.gFdUI, POLLIN, NULL, NULL);
	if (error)
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: PbxAddFd(fdUI) error=0x%lX", error));
		PbxDestroy( gHS.gPollBox );		
		return error; 	
	}


	// Open a file descriptor to the Management Entity device

	error = BtLibOpen(&gHS.gFdME);

	if (error)
	{
		TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: BtLibOpen() error=0x%lX", error));
		FrmAlert(gHS.gResourceDbRef, BtNotAvailableAlert);
		return error;
	}
	else
	{
		error = PbxAddFd(gHS.gPollBox, gHS.gFdME, POLLIN, HandlePbxMEEvent, 0);
		if (error)
		{
			TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: PbxAddFd(fdME) error=0x%lX", error));
			BtLibClose( gHS.gFdME );
			PbxDestroy( gHS.gPollBox );		
			return error;
		}
		//Initialize the UUID
		TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: UUID initialized"));
		BtLibSdpUuidInitialize(gHS.gSdpUUIDList[0], btLibSdpUUID_SC_HEADSET, btLibUuidSize16);
		BtLibSdpUuidInitialize(gHS.gSdpUUIDList[1], btLibSdpUUID_PROT_L2CAP, btLibUuidSize16);
		BtLibSdpUuidInitialize(gHS.gSdpUUIDList[2], btLibSdpUUID_PROT_RFCOMM, btLibUuidSize16);

		
	}

	TraceOutput(TL(appErrorClass, "BtHeadset : AppStart: Error returned error=0x%lX", error));
	return error;
	
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

#if UI_EVENTS_ON_FD
	PbxRemoveFd(gHS.gPollBox, gHS.gFdUI);	// must not close the UI fd
#endif
	PbxDestroy(gHS.gPollBox);		// note this closes any fds still in the box
	
}

/***********************************************************************
 *
 * FUNCTION:    GetMyTypeAndCreator
 *
 * DESCRIPTION: Get the type and creator of this application's resource database.
 *
 * PARAMETERS:  typeP - pointer to the type
 *				 creatorP - pointer t the type
 *
 * RETURNED:    Error
 *
 ***********************************************************************/
static status_t GetMyTypeAndCreator( uint32_t* typeP, uint32_t* creatorP)
{
	DatabaseID			dbID;
	DmDatabaseInfoType	dbInfo;
	status_t			error;

	MemSet( &dbInfo, sizeof(dbInfo), 0 );
	dbInfo.pType    = typeP;
	dbInfo.pCreator = creatorP;
	if ( (error = SysGetModuleDatabase( SysGetRefNum(), &dbID, &gHS.gHeadSetDmOpenRef )) == 0 ) 
	{
		error = DmDatabaseInfo( dbID, &dbInfo );
	}
	return error;
}

/***********************************************************************
 *
 * FUNCTION:    RegisterService
 *
 * DESCRIPTION: Register ourselves as a Bluetooth service.
 * 				This need be done only once after the system boots; redundant registrations
 *				are ignored. After registering a service, the Bluetooth system must be
 *				(re)started in order for that service to actually start functioning.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/

static void RegisterService( void )
{
	BtLibServiceRegistrationParamsType	params;
	status_t							error;

	TraceOutput(TL(appErrorClass, "BtHeadset : RegisterService: ENTER"));

	if ( (error = GetMyTypeAndCreator( &params.appType, &params.appCreator )) == 0 )
	{
		params.appCodeRscId	    = sysResIDDefault;
		params.stackSize        = 5000;
		params.protocol         = btLibRfCommProtocol;
		params.pushSerialModule = false;
		params.execAsNormalApp  = false;
		error = BtLibRegisterService( &params );
	}

	TraceOutput(TL(appErrorClass, "BtHeadset : RegisterService: EXIT error=0x%lx", error));
}

/***********************************************************************
 *
 * FUNCTION:    PrepareService
 *
 * DESCRIPTION: Prepare our service.  Upon entry, we are given
 *				     params->serviceRecH ... handle on an empty local SDP service record
 * 				     params->fdListener .... file descriptor to a bluetooth listener socket
 * 				 Upon exit, the service record must be set up to describe our service as being
 *				 available through the listener socket. The caller of this function will then
 *				 start advertising the service record.
 *
 * PARAMETERS:  params - pointer to a BtLibServicePreparationParamsType
 *
 * RETURNED:    BtLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrepareService( BtLibServicePreparationParamsType*	params )
{
	BtLibSdpUuidType 					gGWSdpUUIDList[3];
	status_t							error;

	TraceOutput(TL(appErrorClass, "BtHeadset : PrepareService: ENTER"));

	// Create a local service record for the RfComm Listener Socket, with the appropriate Universal Service Attribute and then advertise it
	BtLibSdpUuidInitialize(gGWSdpUUIDList[0], btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY, btLibUuidSize16);
	BtLibSdpUuidInitialize(gGWSdpUUIDList[1], btLibSdpUUID_SC_GENERIC_AUDIO, btLibUuidSize16);
	error = BtLibSdpServiceRecordSetAttributesForSocket(
		params->fdListener,			// listener socket
		gGWSdpUUIDList,					// service class uuid list
		2,							// service class uuid list length
		HEADSET_GW_SERVICE_NAME,			// service name
		strlen(HEADSET_GW_SERVICE_NAME)+1,	// service name length including null
		params->serviceRecH			// the service record
		);
	TraceOutput(TL(appErrorClass, "BtHeadset : PrepareService: EXIT error=0x%lx", error));
	return error;

}

/***********************************************************************
 *
 * FUNCTION:    BackgroundUIInit
 *
 * DESCRIPTION: Initialize the autonomous window
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void BackgroundUIInit( void )
{
	FormType*	formP;
	status_t	error;

	// Initialize the Autonomous Window.
	error = WinStartThreadUI();
	ErrFatalErrorIf( error, "can't initialize autonomous window" );

	// Get and save id of the current form.
	formP = FrmGetActiveForm();
	gHS.gPrevFormId = formP ? FrmGetFormId(formP) : 0;
	TraceOutput(TL(appErrorClass, "BtHeadset : BackgroundUIInit: gPrevFormId=%d", gHS.gPrevFormId));

	// Close current menu.
	MenuEraseStatus(0);

	// Save current AIA state, and set it to 'open'.
	gHS.gAIAState = PINGetInputAreaState();
	if ( gHS.gAIAState != pinInputAreaOpen ) {
		PINSetInputAreaState( pinInputAreaOpen );
	}	

	// Display our main form.
	if ( gHS.gPrevFormId ) {
		FrmPopupForm( gHS.gResourceDbRef, MainForm );
	} else {
		FrmGotoForm( gHS.gResourceDbRef, MainForm );
	}
	
}

/***********************************************************************
 *
 * FUNCTION:    BackgroundUITerm
 *
 * DESCRIPTION: Terminate the autonomous window
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void BackgroundUITerm( void )
{
	FormType*	formP;
	status_t	error;


	// Restore AIA state.
	PINSetInputAreaState( gHS.gAIAState );

	// Restore form.
	if ( gHS.gPrevFormId ) {
		FrmReturnToForm( gHS.gPrevFormId );
	} else {
		formP = FrmGetFormPtr( MainForm );
		if ( FrmValidatePtr( formP ) ) {
			FrmEraseForm( formP );
			FrmDeleteForm( formP );
		}
	}

	// Terminate the Autonomous Window.
	error = WinFinishThreadUI();
	ErrFatalErrorIf( error, "can't terminate autonomous window" );
	
}

/***********************************************************************
 *
 * FUNCTION:    ExecuteService
 *
 * DESCRIPTION: Execute our service. The file descriptor params->fdData refers to a connected
 *  			RfComm instance. The remote transmitter has zero flow-control credits.
 *				This function must close params->fdData before exiting.
 *
 * PARAMETERS:  params - Pointer to a BtLibServiceExecutionParamsType
 *
 * RETURNED:    0
 *
 ***********************************************************************/
static status_t ExecuteService( BtLibServiceExecutionParamsType*	params )
{
	status_t	error = 0;

	TraceOutput(TL(appErrorClass, "BtHeadset : ExecuteService: ENTER fd=%d", params->fdData));
	if ( AppStart() == errNone ) {
		PrvChangeHSState(HSConnecting);
			
		// Put the RfComm data file descriptor into the pollbox.
		gHS.gFdRfcData = params->fdData;
		error = PbxAddFd( gHS.gPollBox, gHS.gFdRfcData, POLLIN, HandlePbxSocketEvent, &gHS.gFdRfcData );
		ErrFatalErrorIf( error, "can't add RfComm data socket to pollbox" );

		// Get the remote device's bluetooth address, for displaying later.
		error = BtLibSocketGetInfo( gHS.gFdRfcData, btLibSocketInfo_RemoteDeviceAddress, &gHS.gBdAddr, sizeof(gHS.gBdAddr) );
		ErrFatalErrorIf( error, "can't get socket info" );

		// Get the remote device's name
		PrvDisplayRemoteName();

		// Advance initial credit to the remote transmitter.
		error = BtLibSocketAdvanceCredit( gHS.gFdRfcData, kInitialCredits );
		ErrFatalErrorIf( error, "can't advance credit" );

		// Initialize UI from background.
		BackgroundUIInit();

		// Run the application event loop.
		AppEventLoop();

		// Terminate UI from background.
		BackgroundUITerm();
	}
	AppStop();
	
	TraceOutput(TL(appErrorClass, "BtHeadset : ExecuteService: EXIT err=0x%lx", error));
	return error;
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
	uint32_t	retVal = errNone;
	uint32_t	type;
	uint32_t	creator;
	UNUSED_ARG(cmdPBP);
	UNUSED_ARG(launchFlags);
#if 0
	uint32_t btVersion;

	// Check that Bluetooth components are intalled, which imply Palm OS 4.0 or upper
	if (FtrGet(btLibFeatureCreator, btLibFeatureVersion, &btVersion) != errNone)
	{
		// Alert the user if it's the active application
		if ((launchFlags & sysAppLaunchFlagNewGlobals) && (launchFlags & sysAppLaunchFlagUIApp))
		{
			FrmAlert(gHS.gResourceDbRef, MissingBtComponentsAlert);
		}
		return sysErrRomIncompatible;
	}
#endif

	TraceOutput(TL(appErrorClass,"BtHeadset : PilotMain: ENTER cmd=%hu", cmd));
	gHS.gService = false;
	gHS.gStopFlag = false;

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
		
		// Execute our service
		gHS.gService = true;
		ExecuteService( (BtLibServiceExecutionParamsType*)cmdPBP );
		break;

	case sysBtLaunchCmdDescribeService:
		retVal = GetMyTypeAndCreator(&type, &creator);	// initialize gHeadSetDmOpenRef too
		if (retVal == errNone)
		{ 
			int 					size;
			MemHandle 	theResHdl;
			MemPtr			theResPtr;
			
			// Describe our service for the bluetooth panel services view.
			((BtLibServiceDescriptionType*)cmdPBP)->flags = 0;

			// Get the profile service name str
			theResHdl = DmGetResource(gHS.gHeadSetDmOpenRef, (DmResourceType) strRsc, HeadSetServiceInfoNameString);
			theResPtr = MemHandleLock(theResHdl);
			size = strlen( theResPtr ) + 1;
			if ( (((BtLibServiceDescriptionType*)cmdPBP)->nameP = MemPtrNew( size )) == NULL ) {
				return btLibErrOutOfMemory;
			}
			MemMove( ((BtLibServiceDescriptionType*)cmdPBP)->nameP, theResPtr, size );
			(void) MemHandleUnlock(theResHdl);
			(void) DmReleaseResource(theResHdl);
			
			// Get the profile service description str
			theResHdl = DmGetResource(gHS.gHeadSetDmOpenRef, (DmResourceType) strRsc, HeadSetServerServiceDescriptionString);
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
		TraceOutput(TL(appErrorClass,"BtHeadset : PilotMain: Normal Launch"));
		if ( AppStart() == errNone ) {
			// Initialize UI from background.
			FrmGotoForm( gHS.gResourceDbRef, MainForm );
			AppEventLoop();
		}
		AppStop();
		break;
	default :
		TraceOutput(TL(appErrorClass,"BtHeadset : PilotMain: default"));
		break;
	}

	TraceOutput(TL(appErrorClass,"BtHeadset : PilotMain: EXIT  cmd=%hu, retVal=%lu", cmd, retVal));
	return retVal;
}
