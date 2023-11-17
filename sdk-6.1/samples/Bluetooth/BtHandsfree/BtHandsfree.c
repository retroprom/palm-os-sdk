/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtHandsfree.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	Bluetooth HANDSFREE Profile sample
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <StringMgr.h>
#include <SystemMgr.h>
#include <TraceMgr.h>
#include <BtLib.h>
#include <string.h>

#include "BtHandsfreeRsc.h"
#include "BtAudioGateway.h"

#define BT_HANDSFREE_SERVER_AS_SERVICE

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/
typedef struct _HFGlobalsType
{
	DmOpenRef resourceDbRef;				// The local ID of this app's resource data base.
	DmOpenRef dmOpenRef;

	BtLibDeviceAddressType btAddr;			// Remote bluetooth device's address.

	uint16_t supportedFeatures;				// Local features

	uint8_t callCount;						// number of active call

	Boolean networkAvailable;				// is network available

	Boolean remoteVolume;					// is remote volume supported
	uint16_t remoteSupportedFeatures;		// supported features of handsfree device

	uint8_t speakerGain;					// remote speaker volume
	uint8_t mikeGain;						// remote microphone volume

	Boolean inbandRingtone;					// is inband ringtone enabled

	Boolean startedAsServer;				// profile started as server

	uint64_t cmdInfoTimeout;				// command info timeout

	struct pollfd fdSet[10];
	int16_t iFdCount;
} HFGlobalsType;

typedef struct _HFDeviceListType
{
	BtLibDeviceAddressType addr[16];
	char name[16][btLibMaxDeviceNameLength];
	char *nameP[16];
	uint16_t count;
	uint16_t selected;
} HFDeviceListType;

/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
static HFGlobalsType gHF;
static HFDeviceListType gDeviceList;

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define appFileCreator			'BTHF'	// register your own at http://www.palmos.com/dev/creatorid/
#define UNUSED_ARG(x) (x=x)				// To avoid compiler warning about unused arguments

#define kHeadsetCOD 			(btLibCOD_Audio | btLibCOD_Major_Audio | btLibCOD_Minor_Audio_Headset)
#define kHandsfreeCOD 			(btLibCOD_Audio | btLibCOD_Major_Audio | btLibCOD_Minor_Audio_HandFree)

#define TRACE_MODULE			TraceDefine(appErrorClass, 1)

/***********************************************************************
 *
 * FUNCTION:    PrvInitDeviceList
 *
 * DESCRIPTION:	Set the paired audio device list
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvInitDeviceList(void)
{
	int32_t fdME;
	BtLibDeviceAddressType addr;
	char nameBuffer[btLibMaxDeviceNameLength];
	BtLibClassOfDeviceType cod;
	uint16_t count;
	uint16_t index;
	uint32_t time;
	uint32_t previousTime = 0;
	status_t error;

	MemSet(&gDeviceList.addr[0], sizeof(BtLibDeviceAddressType), 0);
	StrCopy(gDeviceList.name[0], "Use Bt Panel to add device");
	gDeviceList.count = 1;
	gDeviceList.selected = 0;

	error = BtLibOpen(&fdME);
	error = BtLibSecurityNumTrustedDeviceRecords(fdME, true, &count);
	for (index = 0; index < count; index++)
	{
		error = BtLibSecurityGetTrustedDeviceRecordInfo(fdME, index, &addr, nameBuffer, btLibMaxDeviceNameLength, &cod, &time, NULL);
		if (error == btLibErrNoError)
		{
			if ((cod == kHeadsetCOD) || (cod == kHandsfreeCOD))
			{
				gDeviceList.addr[gDeviceList.count] = addr;
				StrCopy(gDeviceList.name[gDeviceList.count], nameBuffer);

				if (previousTime < time)
				{
					previousTime = time;
					gDeviceList.selected = gDeviceList.count;
				}

				gDeviceList.count++;
			}
		}
	}
	BtLibClose(fdME);
}

/***********************************************************************
 *
 * FUNCTION:    PrvDisplayUI
 *
 * DESCRIPTION:	Display the user interface
 *
 * PARAMETERS:	displayButton	show right button depending on HS state
 *				displayVolume	update volume info and buttons
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvDisplayUI(Boolean displayButton, Boolean displayVolume)
{
	Boolean network;
	Boolean call;
	AGCallsetupState callsetup;
	FormPtr frmP = FrmGetActiveForm();
	char str[10];

	if (displayButton)
	{
		switch (AGGetState())
		{
			case AGDisconnected:
				// Reset labels
				FrmCopyLabel(frmP, MainSpeakerGainLabel, "-");
				FrmCopyLabel(frmP, MainMicroGainLabel, "-");
				FrmCopyLabel(frmP, MainCommandInfoLabel, "-");
				// Hide all other
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainDisconnectButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainIncomingButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAnswerButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainRejectButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAlertedButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainFailedButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerGainLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerMoinsRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerPlusRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroGainLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroMoinsRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroPlusRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainRingtoneButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainCommandInfoLabel));
				// Only show "Connect" button and "Select" trigger
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainConnectButton));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSelectTrigger));
				break;

/*			case AGConnecting:
				// UI can not be updated during incoming connecting phase
				if (!gHF.startedAsServer)
				{
					// Hide "Connect" button and "Select" trigger
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainConnectButton));
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSelectTrigger));
				}
				break;
*/
			case AGConnected:
			case AGAudioConnecting:
			case AGAudioConnected:
				// Hide "Connect" button and "Select" trigger
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainConnectButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSelectTrigger));

				AGGetCallStates(NULL, &call, &callsetup);
				// Display "Disconnect" button only if there is no call
				if (!call && (callsetup == AGCallsetupNone))
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainDisconnectButton));
				else
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainDisconnectButton));

				// Display "Incoming" button only if network is available
				if (gHF.networkAvailable)
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainIncomingButton));
				else
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainIncomingButton));

				// Display "Answer" and "Reject" buttons if we are in a call, incoming call or remote alerted
				if (call || (callsetup == AGCallsetupIncoming) || (callsetup == AGCallsetupAlerted))
				{
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainAnswerButton));
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainRejectButton));
				}
				else
				{
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAnswerButton));
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainRejectButton));
				}
				// Display "Alerted" and "Failed" buttons if we are calling
				if (callsetup == AGCallsetupOutgoing)
				{
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainAlertedButton));
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainFailedButton));
				}
				else
				{
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAlertedButton));
					FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainFailedButton));
				}

				// Display the volume labels
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerLabel));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerGainLabel));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroLabel));
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroGainLabel));

				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainCommandInfoLabel));

				if ((gHF.supportedFeatures & kAGHandsfreeGWInbandRingtone) == kAGHandsfreeGWInbandRingtone)
					// Display ring tone button
					FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainRingtoneButton));
				break;

			case AGDisconnecting:
				// Only show "Disconnect" button
				FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainDisconnectButton));
				// Hide all other
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainConnectButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSelectTrigger));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainIncomingButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAnswerButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainRejectButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAlertedButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainFailedButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerGainLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerMoinsRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerPlusRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroGainLabel));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroMoinsRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainMicroPlusRepeating));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainRingtoneButton));
				FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainCommandInfoLabel));
				break;
		}

		// set telephony info		
		AGGetCallStates(&network, NULL, &callsetup);

		if (network)
			FrmCopyLabel(frmP, MainNetworkInfoLabel, " A");
		else
			FrmCopyLabel(frmP, MainNetworkInfoLabel, "NA");

		FrmCopyLabel(frmP, MainCallInfoLabel, StrIToA(str, gHF.callCount));

		switch(callsetup)
		{
			case AGCallsetupNone:
				FrmCopyLabel(frmP, MainCallsetupInfoLabel, "NA");
				break;

			case AGCallsetupIncoming:
				FrmCopyLabel(frmP, MainCallsetupInfoLabel, " I");
				break;

			case AGCallsetupOutgoing:
				FrmCopyLabel(frmP, MainCallsetupInfoLabel, " O");
				break;

			case AGCallsetupAlerted:
				FrmCopyLabel(frmP, MainCallsetupInfoLabel, " A");
				break;
		}
	}

	if (displayVolume)
	{
		FrmCopyLabel(frmP, MainSpeakerGainLabel, StrIToA(str, gHF.speakerGain));
		if (gHF.remoteVolume)
		{
			FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerMoinsRepeating));
			FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainSpeakerPlusRepeating));
		}
		FrmCopyLabel(frmP, MainMicroGainLabel, StrIToA(str, gHF.mikeGain));
		if (gHF.remoteVolume)
		{
			FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroMoinsRepeating));
			FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainMicroPlusRepeating));
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvDisplayInfo
 *
 * DESCRIPTION:	This routine display info 
 *
 * PARAMETERS:	infoStr - text to display in info label
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvDisplayInfo(char* infoStr)
{
	FormPtr frmP = FrmGetActiveForm();

	FrmCopyLabel(frmP, MainInfoLabel, infoStr);
}

/***********************************************************************
 *
 * FUNCTION:    PrvDisplayCmdInfo
 *
 * DESCRIPTION:	This routine display info 
 *
 * PARAMETERS:	formatStr - text to display in command info label, if NULL reset label
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvDisplayCmdInfo(const char* formatStr, ...)
{
	FormPtr frmP = FrmGetActiveForm();
	va_list args;
	char tmpString[24];

	if (formatStr)
	{
		va_start(args, formatStr);
		StrVPrintF(tmpString, formatStr, args);
		va_end(args);

		gHF.cmdInfoTimeout = (uint64_t)SysTimeInSecs(5) + TimGetTicks();
		FrmCopyLabel(frmP, MainCommandInfoLabel, tmpString);
	}
	else
		FrmCopyLabel(frmP, MainCommandInfoLabel, "-");
}

/***********************************************************************
 *
 * FUNCTION:   	PrvRetrieveHFFeatures
 *
 * DESCRIPTION:	This routine retrieves the supported features in the Handsfree
 *
 * PARAMETERS:	gHFSupportedFeatures -The Handsfree Supported Features Value
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
void PrvRetrieveHFFeatures(uint32_t features)
{
	TraceOutput(TL(TRACE_MODULE,"BtHandsfree : PrvRetrieveHFFeature:"));
	TraceOutput(TL(TRACE_MODULE,"    EC and/or NR function %ssupported", (features & kAGHandsfreeECNR) ? "" : "not "));
	TraceOutput(TL(TRACE_MODULE,"    Call waiting and 3-way calling %ssupported", (features & kAGHandsfree3Way) ? "" : "not "));
	TraceOutput(TL(TRACE_MODULE,"    CLI presentation capability %ssupported", (features & kAGHandsfreeCLI) ? "" : "not "));
	TraceOutput(TL(TRACE_MODULE,"    Voice recognition activation %ssupported", (features & kAGHandsfreeVoiceRecognition) ? "" : "not "));
	TraceOutput(TL(TRACE_MODULE,"    Remote volume control %ssupported", (features & kAGHandsfreeRemoteVolume) ? "" : "not "));
}

/***********************************************************************
 *
 * FUNCTION:    PrvUnregisterAndClose
 *
 * DESCRIPTION:	Unregister ME callback and close BT lib
 *
 * PARAMETERS:	None
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void PrvUnregisterAndClose()
{
	TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvUnregisterAndClose: Bye"));

	if (gHF.startedAsServer)
	{
		AGTerminateSession();
		gHF.startedAsServer = false;
	}
	else
	{
		AGTerminateSession();
		AGUnregisterClient();
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvCallback
 *
 * DESCRIPTION:	Audio gateway callback
 *
 * PARAMETERS:	None
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
void PrvCallback(AGEventType *eventP)
{
	int16_t index;
	AGCallsetupState callsetupState;

	switch (eventP->event)
	{
		case AGEventFdOpened:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: new fd %d", eventP->info.socket));
			gHF.fdSet[gHF.iFdCount].fd = eventP->info.socket;
			gHF.fdSet[gHF.iFdCount].events = (short) (POLLIN | POLLHUP);
			
			gHF.iFdCount++;
			break;

		case AGEventFdClosed:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: fd closed %d", eventP->info.socket));
			index = 0;
			while (index < gHF.iFdCount)
			{
				if (gHF.fdSet[index].fd == eventP->info.socket)
				{
					// Remove one entry
					memmove(&gHF.fdSet[index], &gHF.fdSet[index + 1], (gHF.iFdCount - (index + 1)) * sizeof(struct pollfd));
					gHF.iFdCount--;
				}
				else
				{
					index++;
				}
			}
			break;

		case AGEventStateChange:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: state change -> %d (0x%04hX)", eventP->info.state.state, eventP->info.state.status));
			switch (eventP->info.state.state)
			{
				case AGDisconnected:	PrvDisplayInfo("Disconnected");		break;
				case AGStarting:		PrvDisplayInfo("Starting");			break;
				case AGConnecting:		PrvDisplayInfo("Connecting");		break;
				case AGConnected:		PrvDisplayInfo("Connected");		break;
				case AGAudioConnecting:	PrvDisplayInfo("Audio Connecting");	break;
				case AGAudioConnected:	PrvDisplayInfo("Audio Connected");	break;
			}
			PrvDisplayUI(true, false);
			switch (eventP->info.state.state)
			{
				case AGStarting:
					gHF.startedAsServer = true;
					break;

				case AGDisconnected:
					PrvUnregisterAndClose();
					// reset inband ringtone
					gHF.inbandRingtone= true;
					break;
			}
			break;

		case AGEventRemoteVolume:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: remote volume is supported"));
			gHF.remoteVolume = true;
			break;

		case AGEventSupportedFeatures:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: remote supported features : 0x%04hX", eventP->info.features));
			gHF.remoteSupportedFeatures = eventP->info.features;
			if (gHF.remoteSupportedFeatures & kAGHandsfreeRemoteVolume)
				gHF.remoteVolume = true;
			PrvRetrieveHFFeatures(gHF.remoteSupportedFeatures);
			break;

		case AGEventSpeakerVolume:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: new speaker volume is : %d", eventP->info.volume));
			gHF.speakerGain = eventP->info.volume;
			PrvDisplayUI(true, true);
			break;

		case AGEventMicroVolume:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: new micro volume is : %d", eventP->info.volume));
			gHF.mikeGain = eventP->info.volume;
			PrvDisplayUI(true, true);
			break;

		case AGEventCallAnswered:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: call answered by audio device"));
			gHF.callCount++;
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: New call count : %d", gHF.callCount));
			AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupNone);
			PrvDisplayUI(true, true);
			break;

		case AGEventCallHangup:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: call hangup by audio device"));
			AGGetCallStates(NULL, NULL, &callsetupState);
			if ((callsetupState == AGCallsetupNone) && gHF.callCount)
				gHF.callCount--;
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: New call count : %d", gHF.callCount));
			AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupNone);
			PrvDisplayUI(true, true);
			if (!gHF.callCount)
				AGStopCommunication();
			break;

		case AGEventDialNumber:
			PrvDisplayInfo("Dial number");
			PrvDisplayCmdInfo("Dial number : %s", eventP->info.number);
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Dial number: %s", eventP->info.number));
			AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupOutgoing);
			PrvDisplayUI(true, true);
			AGStartCommunication(false, NULL, 0, 0);
			break;

		case AGEventDialMemory:
			PrvDisplayInfo("Dial memory");
			PrvDisplayCmdInfo("Dial memory : %s", eventP->info.number);
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Dial memory: %s", eventP->info.number));
			AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupOutgoing);
			PrvDisplayUI(true, true);
			AGStartCommunication(false, NULL, 0, 0);
			break;

		case AGEventCallHold:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Call hold: %d", eventP->info.holdMode));
			AGGetCallStates(NULL, NULL, &callsetupState);
			if (callsetupState != AGCallsetupNone)
			{
				gHF.callCount++;
				TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: New call count : %d", gHF.callCount));
				AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupNone);
			}
			PrvDisplayUI(true, true);
			AGStartCommunication(false, NULL, 0, 0);
			break;

		case AGEventDTMF:
			PrvDisplayCmdInfo("DTMF : %c", eventP->info.key);
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: DTMF: %c", eventP->info.key));
			break;

		case AGEventGetPhoneNumber:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Get phone number: not supported"));
			eventP->info.phoneP[0] = '\0';
			break;

		case AGEventDialLastNumber:
			PrvDisplayInfo("Dial last number");
			PrvDisplayCmdInfo("Dial last number");
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Dial last number"));
			AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupOutgoing);
			PrvDisplayUI(true, true);
			AGStartCommunication(false, NULL, 0, 0);
			break;

		case AGEventVoiceRecognition:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Voice recognition %s", eventP->info.enabled ? "enabled" : "disabled"));
			break;

		case AGEventECNR:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: ECNR disabled"));
			break;

		case AGEventCommand:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: Receive command: %s", eventP->info.cmd.cmdP));
			if (StrCompare(eventP->info.cmd.cmdP, "AT+CGMI") == 0)
			{
				*eventP->info.cmd.handledP = true;
				AGSendData((uint8_t*)"\r\nPalmsource\r\n", (UInt8)StrLen("\r\nPalmsource\r\n"));
				AGSendData((uint8_t*)"\r\nOK\r\n", (UInt8)StrLen("\r\nOK\r\n"));
			}
			break;

		default:
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : PrvCallback: receives [0x%02lX]", (unsigned long) eventP->event));
	}
}

/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine may initialize. Here, do nothing
 *
 * PARAMETERS:  formptr of active window
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void MainFormInit(FormPtr frmP)
{
	ListPtr lst;
	UInt8 index;

	// Init "Select" list
	lst = (ListType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainSelectList));
	for (index=0; index<16; index++)
		gDeviceList.nameP[index] = gDeviceList.name[index];
	PrvInitDeviceList();
	LstSetListChoices(lst, gDeviceList.nameP, gDeviceList.count);
	LstSetHeight(lst, gDeviceList.count>3 ? 3 : gDeviceList.count);
	LstSetSelection(lst, gDeviceList.selected);
	if (gDeviceList.selected != 0)
		gHF.btAddr = gDeviceList.addr[gDeviceList.selected];
	CtlSetLabel((ControlType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainSelectTrigger)), gDeviceList.nameP[gDeviceList.selected]);

	PrvDisplayUI(true, false);
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
	AGCallsetupState callsetupState;
	Boolean handled = false;
	FormPtr frmP;
	status_t error = errNone;
	
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
				case MainConnectButton:
					// Initiate the Service Level Connection
					error = AGRegisterClient(AGHandsfree);
					if (error != btLibErrNoError)
					{
						TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: ERROR Can not register AG client (0x%08lX)", error));
						break;
					}

					error = AGConnect(&gHF.btAddr);
					if (error != btLibErrPending)
					{
						TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: ERROR Can not connect AG client (0x%04hX)", error));
						AGUnregisterClient();
						break;
					}
					break;

				case MainDisconnectButton:
					// Release the Service Level Connection and leave the application
					AGDisconnect();
					TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: Disconnect ACL on user request"));
					break;

				case MainIncomingButton:		// Incoming call
					switch (AGGetState())
					{
						case AGConnected:
						case AGAudioConnecting:
						case AGAudioConnected:
							AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupIncoming);
							AGStartCommunication(true, "0467523385", 5, 5);
							PrvDisplayUI(true, true);
							break;
					}
					break;

				case MainAnswerButton:			// Locally answers a call
					switch (AGGetState())
					{
						case AGConnected:
							AGGetCallStates(NULL, NULL, &callsetupState);
							if (callsetupState == AGCallsetupNone)
								// if there is no callsetup, give audio line to audio device
								AGStartCommunication(false, NULL, 0, 0);
							else
							{
								// else the call is accepted
								gHF.callCount++;
								TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: New call count : %d", gHF.callCount));
								AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupNone);
								PrvDisplayUI(true, true);
							}
							break;

						case AGAudioConnecting:
						case AGAudioConnected:
							AGGetCallStates(NULL, NULL, &callsetupState);
							if (callsetupState == AGCallsetupNone)
								// if there is no callsetup, take audio line
								AGStopCommunication();
							else
							{
								// else the call is accepted
								gHF.callCount++;
								TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: New call count : %d", gHF.callCount));
								AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupNone);
								PrvDisplayUI(true, true);
							}
							break;
					}
					break;

				case MainRejectButton:			// Locally rejects an incoming call
				case MainFailedButton:			// Simulated call failed
					switch (AGGetState())
					{
						case AGConnected:
						case AGAudioConnecting:
						case AGAudioConnected:
							if (gHF.callCount)
								gHF.callCount--;
							TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: New call count : %d", gHF.callCount));
							AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupNone);
							PrvDisplayUI(true, true);
							if (!gHF.callCount)
								AGStopCommunication();
							break;
					}
					break;

				case MainAlertedButton: 		// Simulate that remote is alerted
					switch (AGGetState())
					{
						case AGConnected:
						case AGAudioConnecting:
						case AGAudioConnected:
							AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, AGCallsetupAlerted);
							PrvDisplayUI(true, true);
							frmP = FrmGetActiveForm();
							FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainAlertedButton));
							FrmHideObject(frmP, FrmGetObjectIndex(frmP, MainFailedButton));
							FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainAnswerButton));
							FrmShowObject(frmP, FrmGetObjectIndex(frmP, MainRejectButton));
							break;
					}
					break;

				case MainRingtoneButton: 		// Change ringtone
					gHF.inbandRingtone = (gHF.inbandRingtone) ? false : true;
					AGActivateInbandRingtone(gHF.inbandRingtone);
					break;
				
				case MainNetworkButton:		// Change network availability
					gHF.networkAvailable = (gHF.networkAvailable) ? false : true;
					AGGetCallStates(NULL, NULL, &callsetupState);
					AGSetCallStates(gHF.networkAvailable, (gHF.callCount > 0) ? true : false, callsetupState);
					PrvDisplayUI(true, true);
					break;
			}    
			break;

		case frmOpenEvent:
			{
				FormPtr frmP = FrmGetFormPtr(eventP->data.frmOpen.formID);
				MainFormInit(frmP);
				WinInvalidateWindow(FrmGetWindowHandle(frmP));
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

		case ctlRepeatEvent:
			// The Gateway controls the remote Handsfree volume using repeating button
			switch (eventP->data.ctlRepeat.controlID)
			{
				case MainSpeakerMoinsRepeating:
					// Decrease the volume : send an +VGS = <value> command, then display the new gain of the speaker
					TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: MainMoinsRepeating"));
					if (gHF.speakerGain > 0)
					{
						gHF.speakerGain--;
						AGChangeSpeakerVolume(gHF.speakerGain);
						PrvDisplayUI(false, true);
						TraceOutput(TL(TRACE_MODULE, "BtHandsfree: MainFormHandleEvent: MainMoinsRepeating :  -1"));
					}
					break;

				case MainSpeakerPlusRepeating:
					// Increase the volume : send an +VGS = <value> command, then display the new gain of the speaker
					TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: MainPlusRepeating"));
					if (gHF.speakerGain < kAGHeadsetSpeakerMaxVolume)
					{
						gHF.speakerGain++;
						AGChangeSpeakerVolume(gHF.speakerGain);
						PrvDisplayUI(false, true);
						TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: MainPlusRepeating :  +1"));
					}
					break;

				case MainMicroMoinsRepeating:
					// Decrease the micro gain : send an +VGM = <value> command, then display the new gain of the micro
					TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: MainMicroMoinsRepeating"));
					if (gHF.mikeGain > 0)
					{
						gHF.mikeGain--;
						AGChangeMicVolume(gHF.mikeGain);
						PrvDisplayUI(false, true);
						TraceOutput(TL(TRACE_MODULE, "BtHandsfree: MainFormHandleEvent: MainMicroMoinsRepeating :  -1"));
					}
					break;

				case MainMicroPlusRepeating:
					// Increase the micro : send an +VGM = <value> command, then display the new gain of the micro
					TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: MainMicroPlusRepeating"));
					if (gHF.mikeGain < kAGHeadsetMicroMaxVolume)
					{
						gHF.mikeGain++;
						AGChangeMicVolume(gHF.mikeGain);
						PrvDisplayUI(false, true);
						TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: MainMicroPlusRepeating :  +1"));
					}
					break;

				default:
					break;
			}
			break;

		case popSelectEvent:
			{
				Int16 sel;

				frmP = FrmGetActiveForm();
				sel = LstGetSelection((ListType *) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, MainSelectList)));
				gDeviceList.selected = sel;
				TraceOutput(TL(TRACE_MODULE, "BtHandsfree : MainFormHandleEvent: Select trigger %d", sel));

				if (sel != 0)
					gHF.btAddr = gDeviceList.addr[sel];
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
	EventType event;
	int32_t oFdCount;			// number of fds we have to handle after the IOSPoll
	status_t error;

	do
	{
		EvtGetEvent(&event, evtNoWait);

		if (!SysHandleEvent(&event))
			if (!MenuHandleEvent(0, &event, &error))
				if (!AppHandleEvent(&event))
					// Add AGEventCallback to manage specific events
					if (!AGEventCallback(&event))
						FrmDispatchEvent(&event);

		if (gHF.iFdCount)
		{
			error = IOSPoll(gHF.fdSet, gHF.iFdCount, 1, &oFdCount);
			if (error != errNone)
			{
				TraceOutput(TL(TRACE_MODULE, "BtHandsfree : AppEventLoop: Error returned by poll. Error=%hx", error));
			}
			else if (oFdCount)
			{
				AGIosCallback(gHF.fdSet, gHF.iFdCount);
			}
		}

		if (gHF.cmdInfoTimeout && (TimGetTicks() >= gHF.cmdInfoTimeout))
		{
			gHF.cmdInfoTimeout = 0;
			PrvDisplayCmdInfo(NULL);
		}
	} while (event.eType != appStopEvent);
}

/***********************************************************************
 *
 * FUNCTION:    GetMyTypeAndCreator
 *
 * DESCRIPTION:	Get the type and creator of this application's resource database.
 *
 * PARAMETERS: 	typeP - pointer to the type
 *				creatorP - pointer t the type
 *
 * RETURNED:    Error
 *
 ***********************************************************************/
static status_t GetMyTypeAndCreator(uint32_t* typeP, uint32_t* creatorP)
{
	DatabaseID			dbID;
	DmDatabaseInfoType	dbInfo;
	status_t			error;

	MemSet(&dbInfo, sizeof(dbInfo), 0);
	dbInfo.pType = typeP;
	dbInfo.pCreator = creatorP;
	if ( (error = SysGetModuleDatabase( SysGetRefNum(), &dbID, &gHF.dmOpenRef )) == 0 )
	{
		error = DmDatabaseInfo(dbID, &dbInfo);
	}

	return error;
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

	// Open our resource database.
	error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gHF.dmOpenRef);
	if (!gHF.dmOpenRef)
	{
		TraceOutput(TL(TRACE_MODULE, "BtHandsfree : AppStart: DmOpenDatabaseByTypeCreator() error=0x%lX", error));
		return error;
	}

	AGInit(PrvCallback);

//	gHF.supportedFeatures = kAGHandsfreeGW3Way | kAGHandsfreeGWECNR | kAGHandsfreeGWVoiceRecognition |
//		kAGHandsfreeGWInbandRingtone | kAGHandsfreeGWVoiceTagAttach | kAGHandsfreeGWRejectCall;
	gHF.supportedFeatures = kAGHandsfreeGW3Way | kAGHandsfreeGWRejectCall;

	gHF.inbandRingtone= true;
	gHF.networkAvailable = true;
	AGSetCallStates(gHF.networkAvailable, false, AGCallsetupNone);

#ifdef BT_HANDSFREE_SERVER_AS_SERVICE
	{
		uint32_t type;
		uint32_t creator;

		error = GetMyTypeAndCreator(&type, &creator);

		error = AGRegisterServer(AGHandsfree, true, type, creator, gHF.supportedFeatures);
		if (error != btLibErrNoError)
		{
			TraceOutput(TL(TRACE_MODULE, "BtHandsfree : AppStart: ERROR Can not register permanent server (0x%04hX)", error));
			return error;
		}
	}
#else /* BT_HANDSFREE_SERVER_AS_SERVICE */
	error = AGRegisterServer(AGHandsfree, false, NULL, NULL, gHF.supportedFeatures);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "BtHandsfree : AppStart: ERROR Can not register permanent server (0x%04hX)", error));
		return error;
	}
#endif /* BT_HANDSFREE_SERVER_AS_SERVICE */

	return errNone;
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
	// Deregister Bluetooth Headset service
	(void) AGUnregisterServer();
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
 * RETURNED:    Result of PrepareService
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	uint32_t	retVal = errNone;

	UNUSED_ARG(cmdPBP);
	UNUSED_ARG(launchFlags);

	TraceOutput(TL(TRACE_MODULE,"BtHandsfree : PilotMain: ENTER cmd=%hu", cmd));
	
	switch ( cmd )
	{
		case sysAppLaunchCmdNormalLaunch:
			// Normal application startup.
			if ( AppStart() == errNone )
			{
				FrmGotoForm(gHF.dmOpenRef, MainForm);
				AppEventLoop();
			}
			AppStop();
			break;

		default :
			TraceOutput(TL(TRACE_MODULE,"BtHandsfree : PilotMain: default"));
			break;
	}

	TraceOutput(TL(TRACE_MODULE,"BtHandsfree : PilotMain: EXIT  cmd=%hu, retVal=%lu", cmd, retVal));
	return retVal;
}
