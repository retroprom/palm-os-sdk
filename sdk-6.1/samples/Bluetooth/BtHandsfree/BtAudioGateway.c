/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtAudioGateway.c
 *
 * Release: 
 *
 * Description:
 *	Bluetooth Audio gateway Profiles (HSP HFP) - Audio Gateway role sample
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <string.h>
#include <TraceMgr.h>
#include <BtLib.h>

#include "BtAudioGateway.h"
#include "BtAudioGatewayPrv.h"

/***********************************************************************
 *
 *	Entry Points
 *
 ***********************************************************************/

/***********************************************************************
 *
 *	Defines
 *
 ***********************************************************************/

#define TRACE_MODULE TraceDefine(appErrorClass, 2)

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

/***********************************************************************
 *
 *  Global variables
 *
 ***********************************************************************/

/***********************************************************************
 *
 *  Internal Constants
 *
 ***********************************************************************/

static void PrvAGAlarmHandler(void);

#if 0
#pragma mark ------
#endif

/***********************************************************************
 *
 *  Internal Functions
 *
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:    PrvAGNotifyCallback
 *
 * DESCRIPTION:	Call the callback
 *
 * PARAMETERS:	eventP - pointer to event for callback
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvAGNotifyCallback(AGEventType *eventP)
{
	if ((eventP == NULL) || (AG(callback) == NULL))
		return;

	AG(callback)(eventP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvAGChangeState
 *
 * DESCRIPTION: This routine changes the device state
 *
 * PARAMETERS:  state - state of the device
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void PrvAGChangeState(AGState state, status_t status)
{
	AGState previousState = AG(state);
	AGEventType event;

	if (previousState != state)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGChangeState: %d -> %d", previousState, state));
		AG(state) = state;

		event.event = AGEventStateChange;
		event.info.state.state = state;
		event.info.state.profile = SESSION(connectedProfile);
		event.info.state.status = status;
		PrvAGNotifyCallback(&event);
	}
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGMEOpen
 *
 * DESCRIPTION:	This routine open the management entity
 *
 * PARAMETERS:	Nothing
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrvAGMEOpen(void)
{
	AGEventType event;
	status_t error = btLibErrNoError;

	if (AG(managementCallbackCount) == 0)
	{
		// Open a file descriptor to the Management Entity device
		error = BtLibOpen(&AG(fdME));
		if (error)
		{
			TraceOutput(TL(TRACE_MODULE, "PrvAGMEOpen: BtLibOpen() error=0x%08lX", error));
			return error;
		}
		else
		{
			event.event = AGEventFdOpened;
			event.info.socket = AG(fdME);
			PrvAGNotifyCallback(&event);
		}
	}
	AG(managementCallbackCount)++;
	TraceOutput(TL(TRACE_MODULE, "PrvAGMEOpen: managementCallbackCount %d", AG(managementCallbackCount)));

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGMEClose
 *
 * DESCRIPTION:	This routine close the management entity
 *
 * PARAMETERS:	Nothing
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrvAGMEClose(void)
{
	AGEventType event;
	status_t error = btLibErrNoError;

	AG(managementCallbackCount)--;
	if (AG(managementCallbackCount) == 0)
	{
		event.event = AGEventFdClosed;
		event.info.socket = AG(fdME);
		PrvAGNotifyCallback(&event);

		error = BtLibClose(AG(fdME));
		if (error != btLibErrNoError)
		{
			TraceOutput(TL(appErrorClass, "AGUnregisterClient: ERROR Can not close management entity (0x%04hX)", error));
		}
		AG(fdME) = (uint32_t)-1;
	}
	TraceOutput(TL(TRACE_MODULE, "PrvAGMEClose: managementCallbackCount %d", AG(managementCallbackCount)));

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGSocketCreate
 *
 * DESCRIPTION:	This routine create a socket
 *
 * PARAMETERS:	socketP - socket
 *				other idem than BtLibSocketCreate
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrvAGSocketCreate(AGSocketRef * socketP, BtLibProtocolEnum socketProtocol)
{
	AGEventType event;
	status_t error;

	if (BTSOCKET_USED(*socketP))
		return btLibErrBusy;

	error = BtLibSocketCreate(&BTSOCKET(*socketP), socketProtocol);
	if (error == btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCreate: socket %d for protocol %s", BTSOCKET(*socketP),
			(socketProtocol == btLibSdpProtocol) ? "SDP" : (socketProtocol == btLibRfCommProtocol) ? "RFComm" : (socketProtocol == btLibSCOProtocol) ? "SCO" : "Unknown"));
		BTSOCKET_USED(*socketP) = true;

		event.event = AGEventFdOpened;
		event.info.socket = BTSOCKET(*socketP);
		PrvAGNotifyCallback(&event);
	}
	else
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCreate: ERROR failed to create socket (0x%04hX)", error));
	}

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGSocketClose
 *
 * DESCRIPTION:	This routine close a socket
 *
 * PARAMETERS:	socketP - connected socket
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrvAGSocketClose(AGSocketRef * socketP)
{
	AGEventType event;
	status_t error;

	if (!BTSOCKET_USED(*socketP))
		return btLibErrParamError;

	TraceOutput(TL(TRACE_MODULE, "PrvAGSocketClose: socket %d", BTSOCKET(*socketP)));
	
	event.event = AGEventFdClosed;
	event.info.socket = BTSOCKET(*socketP);
	PrvAGNotifyCallback(&event);

	error = BtLibSocketClose(BTSOCKET(*socketP));

	BTSOCKET_USED(*socketP) = false;
	BTSOCKET(*socketP) = -1;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGSocketSendRaw
 *
 * DESCRIPTION:	This routine sends raw data
 *
 * PARAMETERS:	data - buffer containing the data to send
 *				size - size of data to send
 *
 * RETURNED:   	btLibErrPending if successfull
 *
 ***********************************************************************/
static status_t PrvAGSocketSendRaw(uint8_t *data, uint8_t size)
{
	status_t error = btLibErrNoError;

	if (!BTSOCKET_USED(SESSION(rfcSocket)))
		return btLibErrSocket;

	error = BtLibSocketSend(BTSOCKET(SESSION(rfcSocket)), data, size);

	if (!error)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSendRaw:"));
		TraceOutput(B(TRACE_MODULE, data, size));
	}
	else if (error == btLibErrPending)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSendRaw: ERROR - 0x%08lX", error));
	}

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGSocketSend
 *
 * DESCRIPTION:	This routine sends data
 *
 * PARAMETERS:	formatStr - buffer containing the data to send
 *
 * RETURNED:   	btLibErrPending if successfull
 *
 ***********************************************************************/
static status_t PrvAGSocketSend(const char* formatStr, ...)
{
	va_list args;
	char tmpString[kAnswerSize];

	va_start(args, formatStr);
	StrVPrintF(tmpString, formatStr, args);
	va_end(args);

	return PrvAGSocketSendRaw((uint8_t*)tmpString, (uint8_t)StrLen(tmpString));
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGSessionInit
 *
 * DESCRIPTION:	Init session.
 *
 * PARAMETERS:	profile - profile session to init
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
static void PrvAGSessionInit(AGProfile profile)
{
	SESSION(speakerGain) = 0;
	SESSION(mikeGain) = 0;
	SESSION(remoteInfoSize) = 0;
	SESSION(remoteInfoP) = NULL;

	switch (profile)
	{
		case AGHeadset:
			HS_SESSION(remoteVolume) = false;
			break;

		case AGHandsfree:
			HF_SESSION(supportedfeatures) = 0;
			HF_SESSION(eventReporting) = false;
			HF_SESSION(eventReportingMode) = false;
			HF_SESSION(sendCLI) = false;
			if (GW_SUPPORTS(kAGHandsfreeGWInbandRingtone))
				HF_SESSION(inbandRingtone) = true;
			else
				HF_SESSION(inbandRingtone) = false;
			HF_SESSION(phoneStr)[0] = '\0';
			break;
	}
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGStartAudioLink
 *
 * DESCRIPTION:	Start an audio link
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
static status_t PrvAGStartAudioLink(void)
{
	BtLibSocketConnectInfoType connectInfo;
	status_t error;

	connectInfo.remoteDeviceP = &SESSION(btAddr);

	error = PrvAGSocketCreate(&SESSION(scoSocket), btLibSCOProtocol);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGStartAudioLink: ERROR while creating SCO socket (0x%04hX)", error));
		return error;
	}

	TraceOutput(TL(TRACE_MODULE, "PrvAGStartAudioLink: Connect SCO"));

	error = BtLibSocketConnect(BTSOCKET(SESSION(scoSocket)), &connectInfo);
	if ((error != btLibErrNoError) && (error != btLibErrPending))
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGStartAudioLink: ERROR while connecting SCO socket (0x%04hX)", error));
		PrvAGSocketClose(&SESSION(scoSocket));
		return error;
	}

	PrvAGChangeState(AGAudioConnecting, btLibErrNoError);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGStopAudioLink
 *
 * DESCRIPTION:	Stop an audio link
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
static status_t PrvAGStopAudioLink(void)
{
	status_t error;

	error = PrvAGSocketClose(&SESSION(scoSocket));
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGStopAudioLink: ERROR while closing SCO socket (0x%04hX)", error));
		return error;
	}

	PrvAGChangeState(AGConnected, btLibErrNoError);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGAdvertiseSDP
 *
 * DESCRIPTION:	Prepare and advertise SDP server record
 *
 * PARAMETERS:	profile - profile to advertise
 *				socketP - RFComm socket linked to SDP record
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
static status_t PrvAGAdvertiseSDP(AGProfile profile, AGSocketRef *socketP)
{
	MemHandle *handleP;
	MemHandle *descriptorHandleP;
	BtLibSdpUuidType *uuidListP;
	uint8_t *sdpUUID;
	uint16_t sdpVersion;
	const char *sdpName;
	BtLibSdpAttributeDataType *sdpDescriptorP;
	status_t error = btLibErrNoError;

	switch (profile)
	{
		case AGHeadset:
			TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: Init Headset Profile SDP record"));
			handleP = &HS_SERVER(sdpHandle);
			descriptorHandleP = &HS_SERVER(sdpDescriptorHandle);
			uuidListP = HS_SERVER(sdpUUIDList);
			sdpUUID = (uint8_t*)btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY;
			sdpVersion = HEADSET_GW_VERSION;
			sdpName = HEADSET_GW_SERVICE_NAME;
			break;

		case AGHandsfree:
			TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: Init Handsfree Profile SDP record"));
			handleP = &HF_SERVER(sdpHandle);
			descriptorHandleP = &HF_SERVER(sdpDescriptorHandle);
			uuidListP = HF_SERVER(sdpUUIDList);
			sdpUUID = (uint8_t*)btLibSdpUUID_SC_HANDSFREE_AUDIO_GATEWAY;
			sdpVersion = HANDSFREE_GW_VERSION;
			sdpName = HANDSFREE_GW_SERVICE_NAME;
			break;

		default:
			return btLibErrParamError;
	}

	*descriptorHandleP = MemHandleNew(sizeof(BtLibSdpAttributeDataType));
	if (!*descriptorHandleP)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Allocate for service description failed"));
		return btLibErrOutOfMemory;
	}
	sdpDescriptorP = MemHandleLock(*descriptorHandleP);
	BtLibSdpUuidInitialize(sdpDescriptorP->profileDescriptorListEntry.profUUID, sdpUUID, btLibUuidSize16);
	sdpDescriptorP->profileDescriptorListEntry.version = sdpVersion;

	error = BtLibSdpServiceRecordCreate(AG(fdME), handleP);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Unable to create SDP record (0x%04hX)", error));
		goto ServerInit_end3;
	}

	error = BtLibSdpServiceRecordSetAttributesForSocket(BTSOCKET(*socketP), uuidListP, 2, sdpName, StrLen(sdpName), *handleP);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Unable to set attributes (1) (0x%04hX)", error));
		goto ServerInit_end4;
	}

	// Add profile descriptor attribute
	error = BtLibSdpServiceRecordSetAttribute(AG(fdME), *handleP, btLibProfileDescriptorList, sdpDescriptorP, 0, 0);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Unable to set attributes (2) (0x%04hX)", error));
		goto ServerInit_end4;
	}

	if (profile == AGHandsfree)
	{
		uint8_t val_network[2] = {btLibDETD_UINT | btLibDESD_1BYTE, 0x00};
		uint8_t val_features[3] = {btLibDETD_UINT | btLibDESD_2BYTES, 0x00, 0x00};

		if (GW_SUPPORTS(kAGHandsfreeGWRejectCall))
			val_network[1] = HANDSFREE_NETWORK_REJECT_CALL;
		error = BtLibSdpServiceRecordSetRawAttribute(AG(fdME), *handleP, HANDSFREE_REMOTE_NETWORK, val_network, 3);
		if (error != btLibErrNoError)
		{
			TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Unable to set raw attributes (1) (0x%04hX)", error));
			goto ServerInit_end4;
		}

		*((uint16_t*)&val_features[1]) = BtLibSdpHToNS(HF_SESSION(gwSupportedfeatures) & HANDSFREE_SDP_FEATURES_MASK);
		error = BtLibSdpServiceRecordSetRawAttribute(AG(fdME), *handleP, HANDSFREE_REMOTE_SUPPORTED_FEATURES, val_features, 3);
		if (error != btLibErrNoError)
		{
			TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Unable to set raw attributes (2) (0x%04hX)", error));
			goto ServerInit_end4;
		}
	}

	// Advertise record
	error = BtLibSdpServiceRecordStartAdvertising(AG(fdME), *handleP);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGAdvertiseSDP: ERROR Unable to advertise service (0x%04hX)", error));
		goto ServerInit_end4;
	}

	return error;

ServerInit_end4:
	BtLibSdpServiceRecordDestroy(AG(fdME), *handleP);
	*handleP = NULL;
ServerInit_end3:
	MemHandleUnlock(*descriptorHandleP);
	MemHandleFree(*descriptorHandleP);
	*descriptorHandleP = NULL;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    PrvAGServerInit
 *
 * DESCRIPTION:	BT Audio Gateway server initialisation.
 *
 * PARAMETERS:	profile - profile to init
 *
 * RETURNED:    Err
 *
 ***********************************************************************/
static status_t PrvAGServerInit(AGProfile profile)
{
	AGSocketRef *socketP;
	BtLibSocketListenInfoType listenInfo;
	BtLibRfCommServerIdType serviceID;
	status_t error = btLibErrNoError;

	switch (profile)
	{
		case AGHeadset:
			TraceOutput(TL(TRACE_MODULE, "PrvAGServerInit: Init Headset Profile server"));
			socketP = &HS_SERVER(listenerSocket);
			break;

		case AGHandsfree:
			TraceOutput(TL(TRACE_MODULE, "PrvAGServerInit: Init Handsfree Profile server"));
			socketP = &HF_SERVER(listenerSocket);
			break;

		default:
			return btLibErrParamError;
	}

	// Create incoming RFComm socket
	error = PrvAGSocketCreate(socketP, btLibRfCommProtocol);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGServerInit: ERROR failed to create socket (0x%04hX)", error));
		return error;
	}

	listenInfo.data.RfComm.maxFrameSize = BT_RF_DEFAULT_FRAMESIZE;
	listenInfo.data.RfComm.advancedCredit = 0;
	error = BtLibSocketListen(BTSOCKET(*socketP), &listenInfo);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGServerInit: ERROR Server RFCOMM listen error (0x%04hX)", error));
		goto ServerInit_end;
	}

	BtLibSocketGetInfo(BTSOCKET(*socketP), btLibSocketInfo_RfCommServerId, &serviceID, sizeof(BtLibRfCommServerIdType));
	TraceOutput(TL(TRACE_MODULE, "PrvAGServerInit: Server RFCOMM socket %lu listen on RFComm %lu", (unsigned long) BTSOCKET(*socketP), (unsigned long) serviceID));

	// Advertise service informations
	error = PrvAGAdvertiseSDP(profile, socketP);
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGServerInit: ERROR Advertise SDP record failed"));
		goto ServerInit_end;
	}

	return error;

ServerInit_end:
	PrvAGSocketClose(socketP);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    PrvAGServerDeinit
 *
 * DESCRIPTION:	BT Service de-initialisation.
 *
 * PARAMETERS:	profile - profile to init
 *
 * RETURNED:    None
 *
 ***********************************************************************/
static void PrvAGServerDeinit(AGProfile profile)
{
	AGSocketRef *socketP;
	MemHandle *handleP;
	MemHandle *descriptorHandleP;

	switch (profile)
	{
		case AGHeadset:
			TraceOutput(TL(TRACE_MODULE, "PrvAGServerDeinit: Deinit Headset Profile server"));
			socketP = &HS_SERVER(listenerSocket);
			handleP = &HS_SERVER(sdpHandle);
			descriptorHandleP = &HS_SERVER(sdpDescriptorHandle);
			break;

		case AGHandsfree:
			TraceOutput(TL(TRACE_MODULE, "PrvAGServerDeinit: Deinit Handsfree Profile server"));
			socketP = &HF_SERVER(listenerSocket);
			handleP = &HF_SERVER(sdpHandle);
			descriptorHandleP = &HF_SERVER(sdpDescriptorHandle);
			break;

		default:
			return;
	}

	if (*handleP)
	{
		BtLibSdpServiceRecordStopAdvertising(AG(fdME), *handleP);

		BtLibSdpServiceRecordDestroy(AG(fdME), *handleP);
		*handleP = NULL;
	}

	if (*descriptorHandleP)
	{
		MemHandleUnlock(*descriptorHandleP);
		MemHandleFree(*descriptorHandleP);
		*descriptorHandleP = NULL;
	}

	if (profile == AGHandsfree)
	{
		PrvAGSocketClose(&HF_SERVER(scoListenerSocket));
	}

	PrvAGSocketClose(socketP);
}

/***********************************************************************
 *
 * FUNCTION:	PrvAGStartRinging
 *
 * DESCRIPTION:	Start ringing.
 *
 * PARAMETERS:	maxRing - maximun number of ring
 * 				ringTimeout - ring timeout
 *				dialNumberStr - dial number string for "Line identification".
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void PrvAGStartRinging(uint8_t maxRing, uint8_t ringTimeout, const char dialNumberStr[kAGMaxPhoneNumberLength+1])
{
	EventType event;

	if (maxRing == 0)
		return;

	PrvAGSocketSend(AG_RING);

	// Send "Line identification" if audio device supports it
	if ((SESSION(connectedProfile) == AGHandsfree) && (StrLen(dialNumberStr) != 0) && HF_SESSION(sendCLI))
	{
		StrCopy(HF_SESSION(phoneStr), dialNumberStr);
		PrvAGSocketSend(AG_HF_LINE_IDENT_RES, HF_SESSION(phoneStr), (*HF_SESSION(phoneStr) == '+') ? kGSMInternationalPhoneNumber :  kGSMOtherPhoneNumber);
	}

	memset(&event, 0, sizeof(event));
	event.eType = kAGEventTimerFired;
	if (EvtAddEventToQueueAtTime((uint64_t)SysTimeInSecs(ringTimeout) + TimGetTicks(), &event))
	{
		TraceOutput(TL(TRACE_MODULE, "PrvAGStartRinging: ERROR -- starting ring timer failed"));
	}
	else
	{
		SESSION(ringCount) = 1;
		SESSION(maxRing) = maxRing;
		SESSION(ringTimeout) = ringTimeout;
	}
}

/***********************************************************************
 *
 * FUNCTION:	PrvAGStopRinging
 *
 * DESCRIPTION:	Stop ringing.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void PrvAGStopRinging(void)
{
	SESSION(maxRing) = 0;
	SESSION(ringTimeout) = 0;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAGAlarmHandler
 *
 * DESCRIPTION:	This routine sends "RING" event to audio device when the ring timer expires.
 *				If the max number of "RING" has been sent, disconnect the link.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void PrvAGAlarmHandler(void)
{
	EventType event;

	TraceOutput(TL(TRACE_MODULE, "PrvAGAlarmHander: Ring timer has Fired"));

	if ((SESSION(maxRing) == 0) || (SESSION(ringTimeout) == 0))
		return;

	SESSION(ringCount++);
	if (SESSION(ringCount) > SESSION(maxRing))
	{
		// Timeout, close SCO socket
		PrvAGChangeState(AGConnected, btLibErrNoConnection);

		PrvAGStopRinging();

		PrvAGStopAudioLink();
	}
	else
	{
		// Send another RING message
		PrvAGSocketSend(AG_RING);

		// Send "Line identification" if audio device supports it
		if ((SESSION(connectedProfile) == AGHandsfree) && (StrLen(HF_SESSION(phoneStr)) != 0) && HF_SESSION(sendCLI))
			PrvAGSocketSend(AG_HF_LINE_IDENT_RES, HF_SESSION(phoneStr), (*HF_SESSION(phoneStr) == '+') ? kGSMInternationalPhoneNumber :  kGSMOtherPhoneNumber);

		// Restart the timer
		memset(&event, 0, sizeof(event));
		event.eType = kAGEventTimerFired;
		if (EvtAddEventToQueueAtTime((uint64_t)SysTimeInSecs(SESSION(ringTimeout)) + TimGetTicks(), &event))
		{
			TraceOutput(TL(TRACE_MODULE, "PrvAGAlarmHander: ERROR -- restart ring timer failed"));
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGStripWhitespaces
 *
 * DESCRIPTION:	This routine removes whitespaces froma string
 *
 * PARAMETERS:	string - pointer to the string to process
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
static void PrvAGStripWhitespaces(char * string)
{
	char *strP;

	// Remove whitespaces
	while((strP = StrStr(string, " ")) != NULL)
		StrCopy(strP, strP+1);

	// Remove tabs
	while((strP = StrStr(string, "\t")) != NULL)
		StrCopy(strP, strP+1);
}

/***********************************************************************
 *
 * FUNCTION:   	PrvAGParseCommand
 *
 * DESCRIPTION:	This routine processes commands sent by the audio device over the 
 *				RfComm socket
 *
 * PARAMETERS:	command - pointer to the command to process
 *				len - length of the command
 *
 * RETURNED:   	nothing
 *
 ***********************************************************************/
static void PrvAGParseCommand(char * command, uint16_t len)
{
	char str[kCommandBufferSize];
	char *strP;
	char *strCommandP;
	char *strNextP;
	uint16_t aLen;
	AGEventType event;
	char phoneNumber[kAGMaxPhoneNumberLength];
	char *phoneP;
	uint8_t val;
	uint8_t i, j;
	status_t error;

	aLen = StrLen(BUFFER(command)) + len;
	StrNCat(BUFFER(command), command, (int16_t) aLen + 1);
	BUFFER(command)[aLen] = 0;
	TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: %s", BUFFER(command)));
	strCommandP = BUFFER(command);

	while (StrLen(strCommandP))
	{
		if ((strNextP = StrStr(strCommandP, "\r")) == NULL)
			break;

		aLen = strNextP - strCommandP;
		StrNCopy(str, strCommandP, aLen);
		str[aLen] = 0;
		PrvAGStripWhitespaces(str);
		TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: found \"%s\"", str));

		//
		// Common commands to headset and handsfree profiles
		//
		if ((strP = StrStr(str, AG_MIC_VOL_CMD)) != NULL)
		{
			// Micro gain has changed on the audio device
			// Notify the audio gateway
			PrvAGSocketSend(AG_OK);
			SESSION(mikeGain) = (uint8_t) StrAToI(strP + StrLen(AG_MIC_VOL_CMD));
			TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: new micro gain is : %d", SESSION(mikeGain)));
			event.event = AGEventMicroVolume;
			event.info.volume = SESSION(mikeGain);
			PrvAGNotifyCallback(&event);
		}
		else if ((strP = StrStr(str, AG_SPK_VOL_CMD)) != NULL)
		{
			// Speaker gain has changed on the audio device
			// Notify the audio gateway
			PrvAGSocketSend(AG_OK);
			SESSION(speakerGain) = (uint8_t) StrAToI(strP + StrLen(AG_SPK_VOL_CMD));
			TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: new speaker gain is : %d", SESSION(speakerGain)));
			event.event = AGEventSpeakerVolume;
			event.info.volume = SESSION(speakerGain);
			PrvAGNotifyCallback(&event);
		}
		//
		// Specific Headset command
		//
		else if (StrCompare(str, AG_BUTTON_CMD) == 0)
		{
			// Headset button has been pushed
			// Depending on the internal state, start the audio connection or stop the headset link
			PrvAGSocketSend(AG_OK);
			switch (AG(state))
			{
				case AGConnected:
					PrvAGStopRinging();

					event.event = AGEventCallAnswered;
					PrvAGNotifyCallback(&event);

					error = PrvAGStartAudioLink();
					if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
					{
						PrvAGSocketClose(&SESSION(rfcSocket));
						BtLibLinkDisconnect(AG(fdME), &SESSION(btAddr));
						TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: ACL Disconnected because Error while starting audio link (0x%04hX)", error));
						break;
					}
					break;

				case AGAudioConnecting:
				case AGAudioConnected:
					TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Disconnect SCO"));
					event.event = AGEventCallHangup;
					PrvAGNotifyCallback(&event);
					break;

				default:
					TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Not in connected state (%d)", AG(state)));
					break;
			}
		}
		//
		// Specific handsfree commands
		//
		else if ((StrCompare(str, AG_HF_CALL_ANSWER_CMD)) == 0)
		{
			// Handsfree accept the call
			// Set "Registration status" call to true and callsetup to none
			// If audio connection is not started, do it
			TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Audio device answer the call"));
			PrvAGStopRinging();

			PrvAGSocketSend(AG_OK);

			event.event = AGEventCallAnswered;
			PrvAGNotifyCallback(&event);

			error = PrvAGStartAudioLink();
			if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: ERROR while starting audio link (0x%04hX)", error));
				break;
			}
		}
		else if ((strP = StrStr(str, AG_HF_CALL_CMD)) != NULL)
		{
			Char tmpString[32];
			uint8_t index = 0;

			// Handsfree wants to place a call
			// Set "Registration status" callsetup to outgoing,
			// start audio connection and notify audio gateway
			StrCopy(tmpString, strP + StrLen(AG_HF_CALL_CMD));
			if ((strP = StrStr(tmpString, "\r\n")) != NULL)
				*strP = '\0';
			if (tmpString[0] == '>')
			{
				index = 1;
				event.event = AGEventDialMemory;
			}
			else
				event.event = AGEventDialNumber;

			TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Place a call %s : %s", (index == 0) ? "to" : "using memory", &tmpString[index]));

			PrvAGSocketSend(AG_OK);

			event.info.number = &tmpString[index];
			PrvAGNotifyCallback(&event);

			error = PrvAGStartAudioLink();
			if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: ERROR while starting audio link (0x%04hX)", error));
				break;
			}
		}
		else if ((strP = StrStr(str, AG_HF_CALL_WAITING_CMD)) != NULL)
		{
			// Handsfree wants to enable/disable "call waiting notification"
			strP += StrLen(AG_HF_CALL_WAITING_CMD);
			if (*strP < '0' || *strP > '9')
			{
				PrvAGSocketSend(AG_ERROR);
			}
			else
			{
				switch (StrAToI(strP))
				{
					case 0:
						PrvAGSocketSend(AG_OK);
						HF_SESSION(supportedfeatures) &= ~kAGHandsfree3Way;
						TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Call waiting is no more supported"));
						break;

					case 1:
						PrvAGSocketSend(AG_OK);
						HF_SESSION(supportedfeatures) |= kAGHandsfree3Way;
						TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Call waiting is now supported"));
						break;

					default:
						PrvAGSocketSend(AG_ERROR);
				}
			}
		}
		else if ((strP = StrStr(str, AG_HF_CALL_HOLD_CMD)) != NULL)
		{
			// Handsfree performs a "call hold" command
			strP += StrLen(AG_HF_CALL_HOLD_CMD);
			if (*(strP) == '?')
			{
				PrvAGSocketSend(AG_HF_CALL_HOLD_RES);
				PrvAGSocketSend(AG_OK);
				
				// if 3 way is supported by both side, and we are connecting, change state to connected
				if (AUDIO_SUPPORTS(kAGHandsfree3Way) && GW_SUPPORTS(kAGHandsfreeGW3Way) && (AG(state) < AGConnected))
					PrvAGChangeState(AGConnected, btLibErrNoError);
			}
			else
			{
				if (*strP < '0' || *strP > '9')
				{
					PrvAGSocketSend(AG_ERROR);
				}
				else
				{
					switch (StrAToI(strP))
					{
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
							PrvAGSocketSend(AG_OK);
							event.event = AGEventCallHold;
							event.info.holdMode = (uint8_t)StrAToI(strP);
							PrvAGNotifyCallback(&event);
							break;

						default:
							PrvAGSocketSend(AG_ERROR);
					}
				}
			}
		}
		else if ((StrCompare(str, AG_HF_HANG_UP_CMD)) == 0)
		{
			// Handsfree wants to stop or reject a call

			if (SESSION(call) || (SESSION(callsetup) != AGCallsetupNone))
			{
				PrvAGStopRinging();
				PrvAGSocketSend(AG_OK);
				event.event = AGEventCallHangup;
				PrvAGNotifyCallback(&event);
			}
			else
				PrvAGSocketSend(AG_ERROR);
		}
		else if ((StrCompare(str, AG_HF_IND_TEST_CMD)) == 0)
		{
			PrvAGSocketSend(AG_HF_IND_TEST_RES);
			PrvAGSocketSend(AG_OK);
		}
		else if ((StrCompare(str, AG_HF_IND_READ_CMD)) == 0)
		{
			PrvAGSocketSend(AG_HF_IND_READ_RES, SESSION(serviceAvailable) ? 1 : 0, SESSION(call) ? 1 : 0, SESSION(callsetup));
			PrvAGSocketSend(AG_OK);
		}
		else if ((strP = StrStr(str, AG_HF_LINE_IDENT_CMD)) != NULL)
		{
			// Handsfree wants to enable/disable "Line identification"
			strP += StrLen(AG_HF_LINE_IDENT_CMD);
			if (*strP < '0' || *strP > '9')
			{
				PrvAGSocketSend(AG_ERROR);
			}
			else
			{
				switch (StrAToI(strP))
				{
					case 0:
						PrvAGSocketSend(AG_OK);
						HF_SESSION(sendCLI) = false;
						TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Call line indentification is no more supported"));
						break;

					case 1:
						PrvAGSocketSend(AG_OK);
						HF_SESSION(sendCLI) = true;
						TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Call line indentification is now supported"));
						break;

					default:
						PrvAGSocketSend(AG_ERROR);
				}
			}
		}
		else if ((strP = StrStr(str, AG_HF_EVENT_REPORTING_CMD)) != NULL)
		{
			// Handsfree wants to change "Event reporting" policy
			Boolean parseOk = true;
			Boolean eventReportingMode = false;
			Boolean eventReporting = false;
			
			// if 3 way is not supported by at least one side, and we are connecting, change state to connected
			if ((!AUDIO_SUPPORTS(kAGHandsfree3Way) || !GW_SUPPORTS(kAGHandsfreeGW3Way)) && (AG(state) < AGConnected))
				PrvAGChangeState(AGConnected, btLibErrNoError);

			// get <mode>
			strP += StrLen(AG_HF_EVENT_REPORTING_CMD);
			if (*strP < '0' || *strP > '9')
			{
				parseOk = false;
				goto AG_HF_EVENT_REPORTING_CMD_END;
			}
			switch (StrAToI(strP))
			{
				case 0:
					eventReportingMode = false;
					break;

				case 3:
					eventReportingMode = true;
					break;

				default:
					parseOk = false;
			}
			// go to next
			strP = StrStr(strP, ",");
			if (strP == NULL)
			{
				parseOk = false;
				goto AG_HF_EVENT_REPORTING_CMD_END;
			}
			strP++;
			// skip <keyp>
			strP = StrStr(strP, ",");
			if (strP == NULL)
			{
				parseOk = false;
				goto AG_HF_EVENT_REPORTING_CMD_END;
			}
			strP++;
			// skip <mode>
			strP = StrStr(strP, ",");
			if (strP == NULL)
			{
				parseOk = false;
				goto AG_HF_EVENT_REPORTING_CMD_END;
			}
			strP++;
			// get <ind>
			if (*strP < '0' || *strP > '9')
			{
				parseOk = false;
				goto AG_HF_EVENT_REPORTING_CMD_END;
			}
			switch (StrAToI(strP))
			{
				case 0:
					eventReporting = false;
					break;

				case 1:
					eventReporting = true;
					break;

				default:
					parseOk = false;
			}

AG_HF_EVENT_REPORTING_CMD_END:
			if (parseOk)
			{
				HF_SESSION(eventReportingMode) = eventReportingMode;
				HF_SESSION(eventReporting) = eventReporting;
				PrvAGSocketSend(AG_OK);
			}
			else
				PrvAGSocketSend(AG_ERROR);
		}
		else if ((strP = StrStr(str, AG_HF_DTMF_CMD)) != NULL)
		{
			// Handsfree wants to send DTMF
			event.event = AGEventDTMF;
			switch (*(strP + StrLen(AG_HF_DTMF_CMD)))
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '#':
				case '*':
				case 'A':
				case 'B':
				case 'C':
				case 'D':
					event.info.key = *(strP + StrLen(AG_HF_DTMF_CMD));
					break;

				case 'a':
					event.info.key = 'A';
					break;

				case 'b':
					event.info.key = 'B';
					break;

				case 'c':
					event.info.key = 'C';
					break;

				case 'd':
					event.info.key = 'D';
					break;

				default:
					event.info.key = 0;
			}

			if (event.info.key != 0)
			{
				PrvAGSocketSend(AG_OK);
				PrvAGNotifyCallback(&event);
			}
			else
				PrvAGSocketSend(AG_ERROR);
		}
		else if ((strP = StrStr(str, AG_HF_INPUT_CMD)) != NULL)
		{
			// Handsfree asks for a phone number
			strP += StrLen(AG_HF_INPUT_CMD);
			if (*strP < '0' || *strP > '9')
			{
				PrvAGSocketSend(AG_ERROR);
			}
			else
			{
				switch (StrAToI(strP))
				{
					case 1:
						event.event = AGEventGetPhoneNumber;
						event.info.phoneP = phoneNumber;

						// Ask to audio gateway for phone number
						PrvAGNotifyCallback(&event);

						// If audio gateway returns a phone number, encode it and send it
						// else send error response
						if ((StrLen(phoneNumber) != 0) && (StrLen(phoneNumber) <= kAGMaxPhoneNumberLength))
						{
							Char tmpString[kAnswerSize];
							
							// Encode phone number conforming with “Digital cellular telecommunication system (Phase 2+);
							// Mobile radio interface layer 3 specification", (GSM 04.08 version 6.11.0), sub-clause 10.5.4.7.
							// (or 3GPP TS 24.008 V6.5.0 (2004-06) - 10.5.4.7 Called party BCD number)
							phoneP = phoneNumber;
							StrPrintF(tmpString, AG_HF_INPUT_PHONE_NUMBER_RES);
							i = (uint8_t)StrLen(AG_HF_INPUT_PHONE_NUMBER_RES);
							tmpString[i++] = 0x5E; // Called party BCD number IEI
							tmpString[i++] = 0; // Length
							if (*phoneP == '+')
							{
								tmpString[i++] = kGSMInternationalPhoneNumber; // International
								phoneP++;
							}
							else
								tmpString[i++] = kGSMOtherPhoneNumber; // Other
							j=0;
							while (*phoneP != '\0')
							{
								switch (*phoneP)
								{
									case '0':
									case '1':
									case '2':
									case '3':
									case '4':
									case '5':
									case '6':
									case '7':
									case '8':
									case '9':
										val = *phoneP - '0';
										break;

									case '*':
										val = 10;
										break;

									case '#':
										val = 11;
										break;

									case 'A':
									case 'a':
										val = 12;
										break;

									case 'B':
									case 'b':
										val = 13;
										break;

									case 'C':
									case 'c':
										val = 14;
										break;

									default:
										TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: ERROR -- Unknow character replaced by 0x00"));
										val = 0;
								}
								tmpString[i] |= (j == 0) ? val & 0x0F : (val << 4) & 0xF0;
								i = i + j;
								j = (j == 0) ? 1 : 0;
							}
							if (j == 0)
								tmpString[i++] |= 0xF0;
							tmpString[StrLen(AG_HF_INPUT_PHONE_NUMBER_RES)+2] = i -StrLen(AG_HF_INPUT_PHONE_NUMBER_RES); // Length
							tmpString[i++] = '\r';
							tmpString[i++] = '\n';
							PrvAGSocketSendRaw((uint8_t*)tmpString, i);
							PrvAGSocketSend(AG_OK);
						}
						else
							PrvAGSocketSend(AG_ERROR);
						break;

					default:
						PrvAGSocketSend(AG_ERROR);
				}
			}
		}
		else if ((StrCompare(str, AG_HF_DIAL_LAST_NUMBER_CMD)) == 0)
		{
			// Handsfree wants to redial last number
			PrvAGSocketSend(AG_OK);
			event.event = AGEventDialLastNumber;
			PrvAGNotifyCallback(&event);
		}
		else if ((strP = StrStr(str, AG_HF_VOICE_RECOGNITION_CMD)) != NULL)
		{
			// Handsfree wants to enable/disable "Voice recognition"
			if (GW_SUPPORTS(kAGHandsfreeGWVoiceRecognition))
			{
				strP += StrLen(AG_HF_VOICE_RECOGNITION_CMD);
				if (*strP < '0' || *strP > '9')
				{
					PrvAGSocketSend(AG_ERROR);
				}
				else
				{
					switch (StrAToI(strP))
					{
						case 0:
							PrvAGSocketSend(AG_OK);
							TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Voice recognition disabled"));
							event.event = AGEventVoiceRecognition;
							event.info.enabled = false;
							PrvAGNotifyCallback(&event);
							break;

						case 1:
							PrvAGSocketSend(AG_OK);
							TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Voice recognition enabled"));
							event.event = AGEventVoiceRecognition;
							event.info.enabled = true;
							PrvAGNotifyCallback(&event);
							// If we activate "Voice recognition", start audio link if needed
							error = PrvAGStartAudioLink();
							if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
							{
								TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: ERROR while starting audio link (0x%04hX)", error));
								break;
							}
							break;

						default:
							PrvAGSocketSend(AG_ERROR);
					}
				}
			}
			else
				// "Voice recognition" not supported by audio gateway
				PrvAGSocketSend(AG_ERROR);
		}
		else if ((strP = StrStr(str, AG_HF_SUPPORTED_FEATURES_CMD)) != NULL)
		{
			// Handsfree informs audio gateway of its supported features
			// and audio gateway should informs it of its own
			HF_SESSION(supportedfeatures) = (uint16_t) StrAToI(strP + StrLen(AG_HF_SUPPORTED_FEATURES_CMD));
			PrvAGSocketSend(AG_HF_SUPPORTED_FEATURES_RES, HF_SESSION(gwSupportedfeatures));
			PrvAGSocketSend(AG_OK);
		}
		else if ((strP = StrStr(str, AG_HF_NOISE_REDUCTION_CMD)) != NULL)
		{
			// Handsfree wants to disable "Echo canceling and noise reduction"
			if (GW_SUPPORTS(kAGHandsfreeGWECNR))
			{
				strP += StrLen(AG_HF_NOISE_REDUCTION_CMD);
				if (*strP < '0' || *strP > '9')
				{
					PrvAGSocketSend(AG_ERROR);
				}
				else
				{
					switch (StrAToI(strP))
					{
						case 0:
							PrvAGSocketSend(AG_OK);
							TraceOutput(TL(TRACE_MODULE, "PrvAGParseCommand: Echo canceling and noise reduction is no more supported"));
							event.event = AGEventECNR;
							PrvAGNotifyCallback(&event);
							break;

						default:
							PrvAGSocketSend(AG_ERROR);
					}
				}
			}
			else
				// "Echo canceling and noise reduction" not supported by audio gateway
				PrvAGSocketSend(AG_ERROR);
		}
		else
		{
			Boolean handled = false;

			// Unknown command
			event.event = AGEventCommand;
			event.info.cmd.cmdP = str;
			event.info.cmd.handledP = &handled;
			PrvAGNotifyCallback(&event);
			if (!handled)
				PrvAGSocketSend(AG_ERROR);
		}

		// Go to next command
		strCommandP = strNextP;
		while ((*strCommandP == '\r') || (*strCommandP == '\n'))
			strCommandP++;
	}

	if (strCommandP != BUFFER(command))
		MemMove(BUFFER(command), strCommandP, StrLen(strCommandP) + 1);
}

/***********************************************************************
 *
 * FUNCTION:    PrvAGSocketSCOCallback
 *
 * DESCRIPTION: Handle events from "SCO socket" devices.
 *
 * PARAMETERS:  sEvent	- SCO event
 *				refCon	- user data
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void PrvAGSocketSCOCallback(BtLibSocketEventType * sEvent, char* data, uint32_t refCon)
{
	AGEventType event;

	switch (sEvent->event)
	{
		case btLibSocketEventConnectRequest:
			if (!BTSOCKET_USED(SESSION(scoSocket)))
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSCOCallback: SCO connect request accepted"));
				BtLibSocketRespondToConnection(BTSOCKET(HF_SERVER(scoListenerSocket)), true);
				PrvAGChangeState(AGAudioConnecting, btLibErrNoError);
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSCOCallback: SCO connect request rejected"));
				BtLibSocketRespondToConnection(BTSOCKET(HF_SERVER(scoListenerSocket)), false);
			}
			break;

		case btLibSocketEventConnectedInbound:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSCOCallback: SCO connected inbound (%d)", sEvent->eventData.newSocket));
			BTSOCKET(SESSION(scoSocket)) = sEvent->eventData.newSocket;
			BTSOCKET_USED(SESSION(scoSocket)) = true;
			event.event = AGEventFdOpened;
			event.info.socket = BTSOCKET(SESSION(scoSocket));
			PrvAGNotifyCallback(&event);
			PrvAGChangeState(AGAudioConnected, btLibErrNoError);
			break;

		case btLibSocketEventConnectedOutbound:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSCOCallback: SCO connected (0x%04hX)", sEvent->status));
			if (sEvent->status == btLibErrNoError)
			{
				// Outbound SCO connection succeeded.
				// Audio Gateway Connected
				PrvAGChangeState(AGAudioConnected, btLibErrNoError);
			}
			else
			{
				PrvAGStopAudioLink();

				// Can't connect SCO, close all sockets and disconnect
				PrvAGChangeState(AGDisconnecting, sEvent->status);

				PrvAGStopRinging();

// TODO:
				if (BTSOCKET_USED(HF_SERVER(scoListenerSocket)))
					PrvAGSocketClose(&HF_SERVER(scoListenerSocket));
				PrvAGSocketClose(&SESSION(rfcSocket));
				PrvAGSocketClose(&SESSION(sdpSocket));
				BtLibLinkDisconnect(AG(fdME), &SESSION(btAddr));
			}
			break;

		case btLibSocketEventDisconnected:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSCOCallback: SCO disconnected (0x%04hX)", sEvent->status));
			PrvAGStopAudioLink();
			break;

		default:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketSCOCallback: socket receives [0x%02lX]", (unsigned long) sEvent->event));
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvAGSocketCallback
 *
 * DESCRIPTION:	Handle events from "socket" devices (RfComm, SDP).
 *
 * PARAMETERS:	sEvent	- event
 *				refCon	- user data
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void PrvAGSocketCallback(BtLibSocketEventType * sEvent, char* data, uint32_t refCon)
{
	BtLibSocketConnectInfoType connectInfo;
	AGEventType event;
	uint16_t attributeID;
	BtLibSdpUuidType *uuidListP;
	status_t error;

	switch (sEvent->event)
	{
		/***********************
 	 	**	  SDP Events	  **
		***********************/
		case btLibSocketEventSdpGetServerChannelByUuid:
			//  The Gateway received the RfComm Server Channel, it can perform a RfComm connection using the channel 
			if (sEvent->status != btLibErrNoError)
			{
				// if get server channel failed for profile handsfree, and client supports headset, try it
				if ((SESSION(connectedProfile) == AGHandsfree) && CLIENT_SUPPORTS(AGHeadset))
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Search for server failed - Try Headset server"));
					error = BtLibSdpGetServerChannelByUuid(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), SESSION(sdpHSUUIDList), 3);
					SESSION(connectedProfile) = AGHeadset;
					if (error == btLibErrPending)
						break;
				}

				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Search for server failed - Disconnect"));
				PrvAGChangeState(AGDisconnecting, sEvent->status);
				PrvAGSocketClose(&SESSION(sdpSocket));
				BtLibLinkDisconnect(AG(fdME), &SESSION(btAddr));
			}
			else
			{
				//  Close the SDP socket
				PrvAGSocketClose(&SESSION(sdpSocket));
				// Create a RfComm Socket
				error = PrvAGSocketCreate(&SESSION(rfcSocket), btLibRfCommProtocol);
				if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to create RFComm socket (0x%04hX)", error));
					PrvAGChangeState(AGDisconnecting, error);
					BtLibLinkDisconnect(AG(fdME), &SESSION(btAddr));
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ACL Disconnected because Error while creating RfComm socket"));
					break;
				}

				// Configure the RfComm Socket
				connectInfo.remoteDeviceP = &SESSION(btAddr);
				connectInfo.data.RfComm.remoteService = sEvent->eventData.sdpByUuid.param.channel;
				connectInfo.data.RfComm.maxFrameSize = kRFCommMaxFrameSize;
				connectInfo.data.RfComm.advancedCredit = 1;
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: calling SocketConnect, requesting maxFrameSize=%hu", connectInfo.data.RfComm.maxFrameSize));
				// Perform the RfComm Socket connection 
				error = BtLibSocketConnect(BTSOCKET(SESSION(rfcSocket)), &connectInfo);
				if (error != btLibErrPending)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to connect on RFComm socket %lu (0x%04hX)", (unsigned long) refCon, error));
					PrvAGChangeState(AGDisconnecting, error);
					BtLibLinkDisconnect(AG(fdME), &SESSION(btAddr));
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ACL Disconnected because Error while connecting RfComm socket"));
				}
			}
			break;

		case btLibSocketEventSdpServiceRecordHandle:
			// A handle to the remote service record has been received. The remote Handle is contained in the Data part of the event
			if (sEvent->status == btLibErrNoError)
			{
				error = BtLibSdpServiceRecordCreate(AG(fdME), &SESSION(sdpRecordHandle));
				if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to create remote record handle (0x%04hX)", error));
					PrvAGSocketClose(&SESSION(sdpSocket));
					break;
				}
				// Map the empty, local service record created (SESSION(sdpRecordHandle)) to the given remote service record
				error = BtLibSdpServiceRecordMapRemote(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), *(BtLibSdpRemoteServiceRecordHandle*)data, SESSION(sdpRecordHandle));
				if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to map remote record (0x%04hX)", error));
					PrvAGSocketClose(&SESSION(sdpSocket));
					break;
				}
				switch (SESSION(connectedProfile))
				{
					case AGHeadset:
						SESSION(remoteInfoSize) = (uint16_t) sizeof(Boolean);
						attributeID = HEADSET_REMOTE_AUDIO_VOLUME_CONTROL;
						break;

					case AGHandsfree:
						SESSION(remoteInfoSize) = (uint16_t) sizeof(uint16_t);
						attributeID = HANDSFREE_REMOTE_SUPPORTED_FEATURES;
						break;
				}
				error = BtLibSdpServiceRecordGetSizeOfRawAttribute(AG(fdME), SESSION(sdpRecordHandle), attributeID, &SESSION(remoteInfoSize));
				if (error == btLibErrPending)
					break;
				else if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to get remote attribute size (0x%04hX)", error));
					PrvAGSocketClose(&SESSION(sdpSocket));
					break;
				}
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR while getting the size of the remote service record handle (0x%04hX)", sEvent->status));
				PrvAGSocketClose(&SESSION(sdpSocket));
				break;
			}
			// fall thru

		case btLibSocketEventSdpGetRawAttributeSize:
			// The size of value of the remote audio attribute has been received.
			if (sEvent->status == btLibErrNoError)
			{
				switch (SESSION(connectedProfile))
				{
					case AGHeadset:
						attributeID = HEADSET_REMOTE_AUDIO_VOLUME_CONTROL;
						break;

					case AGHandsfree:
						attributeID = HANDSFREE_REMOTE_SUPPORTED_FEATURES;
						break;
				}
				SESSION(remoteInfoP) = MemPtrNew(sEvent->eventData.sdpAttribute.info.valSize);
				// Get the value of the remote attribute
				error = BtLibSdpServiceRecordGetRawAttribute(AG(fdME), SESSION(sdpRecordHandle), attributeID, SESSION(remoteInfoP), &sEvent->eventData.sdpAttribute.info.valSize);
				if (error == btLibErrPending)
					break;
				else if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to get remote attribute (0x%04hX)", error));
					PrvAGSocketClose(&SESSION(sdpSocket));
					break;
				}
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR while getting the size of the raw attribute (0x%04hX)", sEvent->status));
				PrvAGSocketClose(&SESSION(sdpSocket));
				break;
			}
			// fall thru

		case btLibSocketEventSdpGetRawAttribute:
			// The value of the remote service audio control attribute has been received.
			if (sEvent->status == btLibErrNoError)
			{
				switch (SESSION(connectedProfile))
				{
					case AGHeadset:
						// Verify that the value received has the format expected.
						if (data[0] != (btLibDETD_BOOL | btLibDESD_1BYTE))
						{
							TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Socket %lu SDP - Wrong type", (unsigned long) refCon));
						}
						else
						{
							HS_SESSION(remoteVolume) = data[1];
							TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Remote volume is%s supported", HS_SESSION(remoteVolume) ? "" : " not"));

							if (HS_SESSION(remoteVolume))
							{
								event.event = AGEventRemoteVolume;
								PrvAGNotifyCallback(&event);
							}
						}
						break;

					case AGHandsfree:
						// Verify that the value received has the format expected.
						if (data[0] != (btLibDETD_UINT | btLibDESD_2BYTES))
						{
							TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Socket %lu SDP - Wrong type", (unsigned long) refCon));
						}
						else
						{
							HF_SESSION(supportedfeatures) = BtLibSdpNToHS(*((uint16_t*)&data[1]));
							TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Remote supported features : 0x%04hX", HF_SESSION(supportedfeatures)));
							event.event = AGEventSupportedFeatures;
							event.info.features = HF_SESSION(supportedfeatures);
							PrvAGNotifyCallback(&event);
						}
						break;
				}
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR while getting the raw attribute (0x%04hX)", sEvent->status));
			}
			MemPtrFree(SESSION(remoteInfoP));
			PrvAGSocketClose(&SESSION(sdpSocket));
			break;

		/***********************
  		**	 RfComm Events	  **
		***********************/
		case btLibSocketEventConnectRequest:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: RFComm connect request"));
			PrvAGChangeState(AGConnecting, btLibErrNoError);
			BtLibSocketRespondToConnection(refCon, true);
			break;

		case btLibSocketEventConnectedInbound:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: RFComm connected inbound (%d)", sEvent->eventData.newSocket));
			BTSOCKET(SESSION(rfcSocket)) = sEvent->eventData.newSocket;
			BTSOCKET_USED(SESSION(rfcSocket)) = true;
			event.event = AGEventFdOpened;
			event.info.socket = BTSOCKET(SESSION(rfcSocket));
			PrvAGNotifyCallback(&event);
			PrvAGChangeState(AGConnected, btLibErrNoError);

			BtLibSocketAdvanceCredit(BTSOCKET(SESSION(rfcSocket)), 1);

			// Get the remote device's bluetooth address.
			BtLibSocketGetInfo(BTSOCKET(SESSION(rfcSocket)), btLibSocketInfo_RemoteDeviceAddress, &SESSION(btAddr), sizeof(SESSION(btAddr)));

			PrvAGSessionInit(SESSION(connectedProfile));

			error = PrvAGSocketCreate(&SESSION(sdpSocket), btLibSdpProtocol);
			if (error != btLibErrNoError)
			{
				// Failed to create a SDP socket to retrieve the remote service record
				// The remote attibutes will not be managed
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to create SDP socket to get remote service record (0x%04hX)", error));
				break;
			}
			SESSION(numRemoteHandles) = 1;

			if (BTSOCKET_USED(HS_SERVER(listenerSocket)) && (refCon == BTSOCKET(HS_SERVER(listenerSocket))))
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback:    -> Headset"));
				SESSION(connectedProfile) = AGHeadset;
				error = BtLibSdpServiceRecordsGetByServiceClass(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), SESSION(sdpHSUUIDList), 3);
				if (error != btLibErrPending)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to get remote service record (0x%04hX)", error));
				}
			}
			else if (BTSOCKET_USED(HF_SERVER(listenerSocket)) && (refCon == BTSOCKET(HF_SERVER(listenerSocket))))
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback:    -> Handsfree"));
				SESSION(connectedProfile) = AGHandsfree;
				error = BtLibSdpServiceRecordsGetByServiceClass(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), SESSION(sdpHFUUIDList), 3);
				if (error != btLibErrPending)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to get remote service record (0x%04hX)", error));
				}
				
				// Create incoming SCO socket
				error = PrvAGSocketCreate(&HF_SERVER(scoListenerSocket), btLibSCOProtocol);
				if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to create SCO socket (0x%04hX)", error));
				}
				else
				{
					error = BtLibSocketListen(BTSOCKET(HF_SERVER(scoListenerSocket)), NULL);
					if (error != btLibErrNoError)
					{
						TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR Server SCO listen error (0x%04hX)", error));
						PrvAGSocketClose(&HF_SERVER(scoListenerSocket));
					}
					else
					{
						TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: SCO socket %lu listen", (unsigned long) BTSOCKET(HF_SERVER(scoListenerSocket))));
					}
				}
			}
			break;

		case btLibSocketEventConnectedOutbound:
			// Outbound connection succeeded.
			// The Gateway has initiated a RfComm Socket (Data) Connection
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: RFComm connected outbound"));

			switch (SESSION(connectedProfile))
			{
				case AGHeadset:
					PrvAGChangeState(AGConnected, btLibErrNoError);
					uuidListP = SESSION(sdpHSUUIDList);
					break;
			
				case AGHandsfree:
					uuidListP = SESSION(sdpHFUUIDList);
					break;
			}

			PrvAGSessionInit(SESSION(connectedProfile));

			// To recuperate the remote audio service control, the Gateway uses a SDP connection
			error = PrvAGSocketCreate(&SESSION(sdpSocket), btLibSdpProtocol);
			if (error != btLibErrNoError)
			{
				// Failed to create a SDP socket to retrieve the remote service record
				// The remote attributes will not be managed
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to create SDP socket to get remote service record (0x%04hX)", error));
				break;
			}
			SESSION(numRemoteHandles) = 1;
			error = BtLibSdpServiceRecordsGetByServiceClass(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), uuidListP, 3);
			if (error != btLibErrPending)
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to get remote service record (0x%04hX)", error));
			}

			if (SESSION(connectedProfile) == AGHandsfree)
			{
				// Create incoming SCO socket
				error = PrvAGSocketCreate(&HF_SERVER(scoListenerSocket), btLibSCOProtocol);
				if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR failed to create SCO socket (0x%04hX)", error));
				}
				else
				{
					error = BtLibSocketListen(BTSOCKET(HF_SERVER(scoListenerSocket)), NULL);
					if (error != btLibErrNoError)
					{
						TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: ERROR Server SCO listen error (0x%04hX)", error));
						PrvAGSocketClose(&HF_SERVER(scoListenerSocket));
					}
					else
					{
						TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: SCO socket %lu listen", (unsigned long) BTSOCKET(HF_SERVER(scoListenerSocket))));
					}
				}
			}
			break;

		case btLibSocketEventDisconnected:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Socket %lu Disconnected", (unsigned long)refCon));
			if (BTSOCKET(SESSION(sdpSocket)) == refCon)
				PrvAGSocketClose(&SESSION(sdpSocket));
			else if (BTSOCKET(SESSION(rfcSocket)) == refCon)
			{
				if (BTSOCKET_USED(HF_SERVER(scoListenerSocket)))
					PrvAGSocketClose(&HF_SERVER(scoListenerSocket));
				PrvAGSocketClose(&SESSION(rfcSocket));
				PrvAGChangeState(AGDisconnecting, btLibErrNoError);
			}
			break;

		case btLibSocketEventSendComplete:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: Socket %lu Send Complete", (unsigned long) refCon));
			break;
	
		default:
			TraceOutput(TL(TRACE_MODULE, "PrvAGSocketCallback: socket receives [0x%02lX]", (unsigned long) sEvent->event));
			break;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvAGManagement
 *
 * DESCRIPTION:	Handle events from the Management Entity device.
 *
 * PARAMETERS:	mEvent - event
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
static void PrvAGManagement(BtLibManagementEventType * mEvent)
{
	Char name[18];
	status_t error;

	switch (mEvent->event)
	{
		case btLibManagementEventACLConnectInbound:
			SESSION(btAddr) = mEvent->eventData.bdAddr;
			BtLibAddrBtdToA(&SESSION(btAddr), name, 18);
			TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: ACL Inbound Connection from %s", name));
			SERVER(running) = true;
			PrvAGChangeState(AGStarting, btLibErrNoError);
			break;

		case btLibManagementEventACLConnectOutbound:
			// The Gateway has initiated an ACL connection. The Gateway recuperates, using a SDP socket, the server channel corresponding to services (UUIDs List) it needs
			if (mEvent->status == btLibErrNoError)
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: Connected"));
				error = PrvAGSocketCreate(&SESSION(sdpSocket), btLibSdpProtocol);
				if (error != btLibErrNoError)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: ERROR while creating SDP socket (0x%04hX)", error));
					BtLibLinkDisconnect(AG(fdME), &mEvent->eventData.bdAddr);
					break;
				}

				if (CLIENT_SUPPORTS(AGHandsfree))
				{
					// if client support handsfree profile, try it first
					TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: Search for Handsfree server"));
					error = BtLibSdpGetServerChannelByUuid(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), SESSION(sdpHFUUIDList), 3);
					SESSION(connectedProfile) = AGHandsfree;
				}
				else if (CLIENT_SUPPORTS(AGHeadset))
				{
					// if client support headset profile
					TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: Search for Headset server"));
					error = BtLibSdpGetServerChannelByUuid(BTSOCKET(SESSION(sdpSocket)), &SESSION(btAddr), SESSION(sdpHSUUIDList), 3);
					SESSION(connectedProfile) = AGHeadset;
				}
				else
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: ERROR -- Unsupported profile"));
					error = btLibErrParamError;
				}

				if (error != btLibErrPending)
				{
					TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: ERROR while getting server channel by UUID (0x%08lX)", error));
					PrvAGSocketClose(&SESSION(sdpSocket));
					BtLibLinkDisconnect(AG(fdME), &mEvent->eventData.bdAddr);
					break;
				}
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: Connect failed"));
				PrvAGChangeState(AGDisconnected, mEvent->status);
			}
			break;

		case btLibManagementEventACLDisconnect:
			// ACL link disconnected : display the initial form
			TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: ACL Disconnected"));

			HS_SESSION(remoteVolume) = false;
			SESSION(connectedProfile) = AGNone;
			PrvAGChangeState(AGDisconnected, mEvent->status);
			break;

		default:
			TraceOutput(TL(TRACE_MODULE, "PrvAGManagement: ME receives [0x%02lX]", (unsigned long) mEvent->event));
	}
}

#if 0
#pragma mark ------
#endif

/***********************************************************************
 *
 *  API
 *
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:    AGInit
 *
 * DESCRIPTION:	Initialize Audio Gateway.
 *
 * PARAMETERS:	
 *				callback - Audio gateway callback.
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGInit(AGCallback callback)
{
	memset(&gInternal, 0, sizeof(AGInternalType));

	BtLibSdpUuidInitialize(SESSION(sdpHSUUIDList[0]), btLibSdpUUID_SC_HEADSET, btLibUuidSize16);
	BtLibSdpUuidInitialize(SESSION(sdpHSUUIDList[1]), btLibSdpUUID_PROT_L2CAP, btLibUuidSize16);
	BtLibSdpUuidInitialize(SESSION(sdpHSUUIDList[2]), btLibSdpUUID_PROT_RFCOMM, btLibUuidSize16);

	BtLibSdpUuidInitialize(HS_SERVER(sdpUUIDList[0]), btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY, btLibUuidSize16);
	BtLibSdpUuidInitialize(HS_SERVER(sdpUUIDList[1]), btLibSdpUUID_SC_GENERIC_AUDIO, btLibUuidSize16);

	BtLibSdpUuidInitialize(SESSION(sdpHFUUIDList[0]), btLibSdpUUID_SC_HANDSFREE, btLibUuidSize16);
	BtLibSdpUuidInitialize(SESSION(sdpHFUUIDList[1]), btLibSdpUUID_PROT_L2CAP, btLibUuidSize16);
	BtLibSdpUuidInitialize(SESSION(sdpHFUUIDList[2]), btLibSdpUUID_PROT_RFCOMM, btLibUuidSize16);

	BtLibSdpUuidInitialize(HF_SERVER(sdpUUIDList[0]), btLibSdpUUID_SC_HANDSFREE_AUDIO_GATEWAY, btLibUuidSize16);
	BtLibSdpUuidInitialize(HF_SERVER(sdpUUIDList[1]), btLibSdpUUID_SC_GENERIC_AUDIO, btLibUuidSize16);

	AG(callback) = callback;
}

/***********************************************************************
 *
 * FUNCTION:    AGEventCallback
 *
 * DESCRIPTION:	Manage events.
 *
 * PARAMETERS:	
 *				eventP - pointer to the event.
 *
 * RETURNED:	true if handled
 *
 ***********************************************************************/
Boolean AGEventCallback(EventType * eventP)
{
	if (eventP->eType == kAGEventTimerFired)
	{
		PrvAGAlarmHandler();
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    AGIosCallback
 *
 * DESCRIPTION:	Manage IOS data.
 *
 * PARAMETERS:	
 *				iFds - file descriptor set.
 *				iNfds - number of file descriptor.
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGIosCallback(struct pollfd iFds[], int32_t iNfds)
{
	AGEventType event;
	uint16_t index;				// loop index on gFdSet table
	BtLibManagementEventType MEEvent;
	BtLibSocketEventType SocketEvent;
	struct strbuf aDataptr, aCtlptr;
	char buffer[1024];
	int32_t aIoFlags;
	int32_t result = 0;
	struct strrecvfd recvfd;	// the received fd of a new client connection
	AGHFEvents cmd;
	status_t error;

	for (index = 0; index < iNfds; index++)
	{
		if (iFds[index].revents & POLLIN)
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event on %d", iFds[index].fd));
		}

		if ((iFds[index].fd == AG(fdME)) && (iFds[index].revents & POLLIN))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event on ME device"));
			aDataptr.buf = buffer;
			aDataptr.len = 0;
			aDataptr.maxlen = 1024;
			aCtlptr.buf = (char *) &MEEvent;
			aCtlptr.len = 0;
			aCtlptr.maxlen = (int) sizeof(BtLibManagementEventType);
			aIoFlags = 0;
			result = IOSGetmsg(iFds[index].fd, &aCtlptr, &aDataptr, &aIoFlags, &error);

			if ((status_t) result == (status_t) - 1)
			{
				TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Error returned by read. Error=%hx", result));
			}
			else
			{
				if (aCtlptr.len != -1)
					PrvAGManagement(&MEEvent);
			}
		}
		else if ((BTSOCKET_USED(SESSION(sdpSocket)) && (iFds[index].fd == BTSOCKET(SESSION(sdpSocket))) && (iFds[index].revents & POLLIN)) ||
			(BTSOCKET_USED(SESSION(rfcSocket)) && (iFds[index].fd == BTSOCKET(SESSION(rfcSocket))) && (iFds[index].revents & POLLIN)) ||
			(!SERVER(asService) && BTSOCKET_USED(HS_SERVER(listenerSocket)) && (iFds[index].fd == BTSOCKET(HS_SERVER(listenerSocket))) && (iFds[index].revents & POLLIN)) ||
			(!SERVER(asService) && BTSOCKET_USED(HF_SERVER(listenerSocket)) && (iFds[index].fd == BTSOCKET(HF_SERVER(listenerSocket))) && (iFds[index].revents & POLLIN)))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event on socket %d", iFds[index].fd));
			// if another event than POLLIN is selected, (for example POLLHUP), remove the client
			aDataptr.buf = buffer;
			aDataptr.len = 0;
			aDataptr.maxlen = 1024;
			aCtlptr.buf = (char *) &SocketEvent;
			aCtlptr.len = 0;
			aCtlptr.maxlen = (int) sizeof(BtLibSocketEventType);
			aIoFlags = 0;
			result = IOSGetmsg(iFds[index].fd, &aCtlptr, &aDataptr, &aIoFlags, &error);

			if ((status_t) result == (status_t) - 1)
			{
				TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Error returned by read. Error=%hx", result));
			}
			else
			{
				if (aCtlptr.len != -1)
					PrvAGSocketCallback(&SocketEvent, buffer, iFds[index].fd);
				else
				{
					BtLibSocketAdvanceCredit(iFds[index].fd, 1);
					PrvAGParseCommand(buffer, aDataptr.len);
				}
			}
		}
		else if (SERVER(asService) && BTSOCKET_USED(HS_SERVER(listenerSocket)) && (iFds[index].fd == BTSOCKET(HS_SERVER(listenerSocket))) && (iFds[index].revents & POLLIN))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event on headset pipe %d", iFds[index].fd));
			PrvAGChangeState(AGStarting, btLibErrNoError);

			// Get inbound RFComm socket from service
			IOSIoctl(iFds[index].fd, I_RECVFD, (int32_t) &recvfd, &error);
			if (!error)
			{
				error = PrvAGMEOpen();
				if (error)
				{
					PrvAGSocketClose(&HS_SERVER(listenerSocket));
					break;
				}

				SocketEvent.event = btLibSocketEventConnectedInbound;
				SocketEvent.eventData.newSocket = recvfd.fd;
				PrvAGSocketCallback(&SocketEvent, NULL, iFds[index].fd);
			}
		}
		else if (SERVER(asService) && BTSOCKET_USED(HF_SERVER(listenerSocket)) && (iFds[index].fd == BTSOCKET(HF_SERVER(listenerSocket))) && (iFds[index].revents & POLLIN))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event on handsfree pipe %d", iFds[index].fd));
			IOSRead(iFds[index].fd, &cmd, sizeof(cmd), &error);
			if (error != errNone)
			{
				TraceOutput(TL(TRACE_MODULE, "AGIosCallback: ERROR while getting fd (0x%08lX)", error));
			}
			switch (cmd)
			{
				case AG_HFGetSupportedFeatures:
					// Send gateway supported features to service
					IOSWrite(iFds[index].fd, &HF_SESSION(gwSupportedfeatures), sizeof(HF_SESSION(gwSupportedfeatures)), &error);
					PrvAGSocketClose(&HF_SERVER(listenerSocket));
					SERVER(running) = false;
					break;

				case AG_HFConnectedInbound:
					PrvAGChangeState(AGStarting, btLibErrNoError);
					// Get inbound RFComm socket from service
					IOSIoctl(iFds[index].fd, I_RECVFD, (int32_t) &recvfd, &error);
					if (!error)
					{
						error = PrvAGMEOpen();
						if (error)
						{
							PrvAGSocketClose(&HF_SERVER(listenerSocket));
							break;
						}

						SocketEvent.event = btLibSocketEventConnectedInbound;
						SocketEvent.eventData.newSocket = recvfd.fd;
						PrvAGSocketCallback(&SocketEvent, NULL, iFds[index].fd);
					}
					break;

				default:
					TraceOutput(TL(TRACE_MODULE, "AGIosCallback: ERROR Unknown command %d", cmd));
			}
		}
		else if ((BTSOCKET_USED(SESSION(scoSocket)) && (iFds[index].fd == BTSOCKET(SESSION(scoSocket))) && (iFds[index].revents & POLLIN)) ||
			(BTSOCKET_USED(HF_SERVER(scoListenerSocket)) && (iFds[index].fd == BTSOCKET(HF_SERVER(scoListenerSocket))) && (iFds[index].revents & POLLIN)))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event on SCO socket %d", iFds[index].fd));
			// if another event than POLLIN is selected, (for example POLLHUP), remove the client
			aDataptr.buf = buffer;
			aDataptr.len = 0;
			aDataptr.maxlen = 1024;
			aCtlptr.buf = (char *) &SocketEvent;
			aCtlptr.len = 0;
			aCtlptr.maxlen = (int) sizeof(BtLibSocketEventType);
			aIoFlags = 0;
			result = IOSGetmsg(iFds[index].fd, &aCtlptr, &aDataptr, &aIoFlags, &error);

			if ((status_t) result == (status_t) - 1)
			{
				TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Error returned by read. Error=%hx", result));
			}
			else
			{
				if (aCtlptr.len != -1)
					PrvAGSocketSCOCallback(&SocketEvent, buffer, iFds[index].fd);
				else
				{
					TraceOutput(TL(TRACE_MODULE, "AGIosCallback: SCO socket"));
				}
			}
		}
		else if ((iFds[index].fd == HS_SERVER(pipe)[1]) && (iFds[index].revents & POLLIN))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event open on headset pipe"));
			// Accept the client
			IOSIoctl(HS_SERVER(pipe)[1], I_RECVFD, (int32_t) &recvfd, &error);
			if (!error)
			{
				PrvAGChangeState(AGStarting, btLibErrNoError);

				SERVER(running) = true;

				BTSOCKET(HS_SERVER(listenerSocket)) = recvfd.fd;
				BTSOCKET_USED(HS_SERVER(listenerSocket)) = true;
				event.event = AGEventFdOpened;
				event.info.socket = BTSOCKET(HS_SERVER(listenerSocket));
				PrvAGNotifyCallback(&event);
			}
		}
		else if ((iFds[index].fd == HF_SERVER(pipe)[1]) && (iFds[index].revents & POLLIN))
		{
			TraceOutput(TL(TRACE_MODULE, "AGIosCallback: Receiving event open on handsfree pipe"));
			// Accept the client
			IOSIoctl(HF_SERVER(pipe)[1], I_RECVFD, (int32_t) &recvfd, &error);
			if (!error)
			{
				SERVER(running) = true;

				BTSOCKET(HF_SERVER(listenerSocket)) = recvfd.fd;
				BTSOCKET_USED(HF_SERVER(listenerSocket)) = true;
				event.event = AGEventFdOpened;
				event.info.socket = BTSOCKET(HF_SERVER(listenerSocket));
				PrvAGNotifyCallback(&event);
			}
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    AGRegisterClient
 *
 * DESCRIPTION:	Register Audio gateway to be used as client.
 *
 * PARAMETERS:	
 *				callback - Audio gateway client callback.
 *				profile - profile to use.
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
status_t AGRegisterClient(AGProfile profile)
{
	status_t error = btLibErrNoError;

	if (gInternal.clientProfile)
		return btLibErrBusy;

	if (((profile & AGHeadset) != AGHeadset) && ((profile & AGHandsfree) != AGHandsfree))
		return btLibErrParamError;

	error = PrvAGMEOpen();
	if (error)
		return error;

	gInternal.clientProfile = profile;

	return btLibErrNoError;
}

/***********************************************************************
 *
 * FUNCTION:    AGUnregisterClient
 *
 * DESCRIPTION:	Stop using Audio gateway as client.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
status_t AGUnregisterClient(void)
{
	status_t error = btLibErrNoError;

	AGTerminateSession();

	// Unregister management entity
	PrvAGMEClose();

	gInternal.clientProfile = AGNone;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AGRegisterServer
 *
 * DESCRIPTION:	Register Audio gateway to be used as server.
 *
 * PARAMETERS:	
 *				profile - profile to use.
 *				asService - true if the server should be started as a Bluetooth service
 *				type - type of the parent application, only needed if started as service.
 *				creator - creator of the parent application, only needed if started as service.
 *				hfFeatures - Handsfree gateway supported features
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
status_t AGRegisterServer(AGProfile profile, Boolean asService, uint32_t type, uint32_t creator, uint16_t hfFeatures)
{
	BtLibServiceRegistrationParamsType params;
	AGEventType event;
	status_t error = btLibErrNoError;

	if (gInternal.serverProfile)
		return btLibErrBusy;

	if (asService)
	{
		if ((profile & AGHeadset) == AGHeadset)
		{
			AG(serverProfile) |= AGHeadset;

			// create the server pipe
			error = IOSPipe(HS_SERVER(pipe));
			if (error)
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: ERROR Can not create HS pipe (0x%04hX)", error));
				return error;
			}
			// attach a name to the server pipe on first pipe fd
			error = IOSFattach(HS_SERVER(pipe)[0], kHSPipeName);
			if (error)
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: ERROR Can not name HS pipe (0x%04hX)", error));
				return error;
			}
			event.event = AGEventFdOpened;
			event.info.socket = HS_SERVER(pipe)[1];
			PrvAGNotifyCallback(&event);

			params.appType = type;
			params.appCreator = creator;
			params.appCodeRscId = 1;
			params.stackSize = 5000;
			params.protocol = btLibRfCommProtocol;
			params.pushSerialModule = false;
			params.execAsNormalApp = false;
			error = BtLibRegisterService(&params);
		}

		if ((profile & AGHandsfree) == AGHandsfree)
		{
			AG(serverProfile) |= AGHandsfree;

			HF_SESSION(gwSupportedfeatures) = hfFeatures;

			// create the server pipe
			error = IOSPipe(HF_SERVER(pipe));
			if (error)
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: ERROR Can not create HF pipe (0x%04hX)", error));
				return error;
			}
			// attach a name to the server pipe on first pipe fd
			error = IOSFattach(HF_SERVER(pipe)[0], kHFPipeName);
			if (error)
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: ERROR Can not name HF pipe (0x%04hX)", error));
				return error;
			}
			event.event = AGEventFdOpened;
			event.info.socket = HF_SERVER(pipe)[1];
			PrvAGNotifyCallback(&event);

			params.appType = type;
			params.appCreator = creator;
			params.appCodeRscId = 2;
			params.stackSize = 5000;
			params.protocol = btLibRfCommProtocol;
			params.pushSerialModule = false;
			params.execAsNormalApp = false;
			error = BtLibRegisterService(&params);
		}
	}
	else
	{
		error = PrvAGMEOpen();
		if (error)
			return error;
		
		if ((profile & AGHeadset) == AGHeadset)
		{
			error = PrvAGServerInit(AGHeadset);
			if (error != btLibErrNoError)
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: ERROR Can not start Headset Audio Gateway server (0x%04hX)", error));
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: Headset Audio Gateway server started"));
				AG(serverProfile) |= AGHeadset;
			}
		}

		if ((profile & AGHandsfree) == AGHandsfree)
		{
			HF_SESSION(gwSupportedfeatures) = hfFeatures;
			error = PrvAGServerInit(AGHandsfree);
			if (error != btLibErrNoError)
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: ERROR Can not start Handsfree Audio Gateway server (0x%04hX)", error));
			}
			else
			{
				TraceOutput(TL(TRACE_MODULE, "AGRegisterServer: Handsfree Audio Gateway server started"));
				AG(serverProfile) |= AGHandsfree;
			}
		}

		if (AG(serverProfile) == AGNone)
		{
			return error;
		}
	}

	SERVER(asService) = asService;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AGUnregisterServer
 *
 * DESCRIPTION:	Stop using Audio gateway as server.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:   	btLibErrNoError if successfull
 *
 ***********************************************************************/
status_t AGUnregisterServer(void)
{
	AGEventType event;
	status_t error = btLibErrNoError;

	AGTerminateSession();

	if (SERVER(asService))
	{
		if (SERVER_SUPPORTS(AGHeadset))
		{
			event.event = AGEventFdClosed;
			event.info.socket = HS_SERVER(pipe)[1];
			PrvAGNotifyCallback(&event);
			
			IOSFdetach(kHSPipeName);
			IOSClose(HS_SERVER(pipe)[1]);
			IOSClose(HS_SERVER(pipe)[0]);
		}
		
		if (SERVER_SUPPORTS(AGHandsfree))
		{
			event.event = AGEventFdClosed;
			event.info.socket = HF_SERVER(pipe)[1];
			PrvAGNotifyCallback(&event);
			
			IOSFdetach(kHFPipeName);
			IOSClose(HF_SERVER(pipe)[1]);
			IOSClose(HF_SERVER(pipe)[0]);
		}

		SERVER(asService) = false;
	}
	else
	{
		if (SERVER_SUPPORTS(AGHeadset))
			PrvAGServerDeinit(AGHeadset);
		if (SERVER_SUPPORTS(AGHandsfree))
			PrvAGServerDeinit(AGHandsfree);

		// Unregister management entity
		PrvAGMEClose();
	}

	gInternal.serverProfile = AGNone;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AGConnect
 *
 * DESCRIPTION:	Connect audio gateway to remote device.
 *
 * PARAMETERS:	remoteDeviceP - Bluetooth address of the audio device
 *
 * RETURNED:	
 *				btLibErrPending - The results will be returned through a callback.
 *				btLibErrBusy - The audio gateway is not disconnected or alreday used as server.
 *				btLibErrAlreadyConnected - The connection is already in place. (no event returned)
 *				btLibErrTooMany - Reached maximum number of ACL Links allowed. 
 *
 ***********************************************************************/
status_t AGConnect(BtLibDeviceAddressTypePtr remoteDeviceP)
{
	status_t error;

	if (remoteDeviceP == NULL)
		return btLibErrParamError;

	if (AG(state) != AGDisconnected)
	{
		TraceOutput(TL(TRACE_MODULE, "AGConnect: ERROR Audio gateway already started"));
		return btLibErrBusy;
	}

	SESSION(btAddr) = *remoteDeviceP;

	error = BtLibLinkConnect(AG(fdME), &SESSION(btAddr));
	if (error == btLibErrPending)
		PrvAGChangeState(AGConnecting, btLibErrNoError);
	else
	{
		TraceOutput(TL(TRACE_MODULE, "AGConnect: ERROR Connect failed (0x%04hX)", error));
	}

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AGDisconnect
 *
 * DESCRIPTION:	Disconnect audio gateway.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	
 *				btLibErrPending - The results will be returned through a callback.
 *				btLibErrBusy - Audio link is connected with handsfree device.
 *				btLibErrNoError - A connection attempt was canceled before completing. (No event generated) 
 *				btLibErrNoAclLink - No link exists to disconnect. (No event generated) 
 *
 ***********************************************************************/
status_t AGDisconnect(void)
{
	status_t error;

	if (AGGetState() > AGConnected)
		return btLibErrBusy;

	PrvAGStopRinging();

	if (BTSOCKET_USED(HF_SERVER(scoListenerSocket)))
		PrvAGSocketClose(&HF_SERVER(scoListenerSocket));

	error = PrvAGSocketClose(&SESSION(rfcSocket));
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "AGDisconnect: ERROR RFComm socket close failed (0x%04hX)", error));
	}

	error = BtLibLinkDisconnect(AG(fdME), &SESSION(btAddr));
	if (error == btLibErrPending)
	{
		TraceOutput(TL(TRACE_MODULE, "AGDisconnect: Start disconnecting link", error));
	}
	else if (error == btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "AGDisconnect: Link disconnected", error));
		HS_SESSION(remoteVolume) = false;
		PrvAGChangeState(AGDisconnected, btLibErrNoError);
	}
	else
	{
		TraceOutput(TL(TRACE_MODULE, "AGDisconnect: ERROR Link disconnect failed (0x%04hX)", error));
	}

	PrvAGChangeState(AGDisconnecting, btLibErrNoError);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AGTerminateSession
 *
 * DESCRIPTION:	Terminate an audio session.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGTerminateSession(void)
{
	if (SERVER(running))
	{
		if (SERVER(asService))
		{
			if (BTSOCKET_USED(HS_SERVER(listenerSocket)))
				PrvAGSocketClose(&HS_SERVER(listenerSocket));
			if (BTSOCKET_USED(HF_SERVER(listenerSocket)))
				PrvAGSocketClose(&HF_SERVER(listenerSocket));

			PrvAGMEClose();
		}

		SERVER(running) = false;
	}
}

/***********************************************************************
 *
 * FUNCTION:    AGStartCommunication
 *
 * DESCRIPTION:	.
 *
 * PARAMETERS:	
 *				incoming - true it is an incoming call.
 *				dialNumberStr - dial number string for "Line identification".
 *				maxRing - maximun rings before canceling connection to audio device.
 *				ringTimeout - timeout between rings in seconds.
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGStartCommunication(Boolean incoming, const char dialNumberStr[kAGMaxPhoneNumberLength+1], uint8_t maxRing, uint8_t ringTimeout)
{
	status_t error;

	switch (SESSION(connectedProfile))
	{
		case AGHeadset:
			if (AG(state) == AGConnected)
			{
				// Once connected the gateway sends a "RING" and waits the 
				// "AT+CKPD=200" response before establish the Audio (SCO) connection. 
				PrvAGStartRinging(maxRing, ringTimeout, dialNumberStr);
			}
			break;

		case AGHandsfree:
			if (incoming)
			{
				if (SESSION(call))
				{
					// there is already a communication
					TraceOutput(TL(TRACE_MODULE, "AGStartCommunication: New incoming call"));

					if (AUDIO_SUPPORTS(kAGHandsfree3Way))
					{
						PrvAGSocketSend(AG_HF_CALL_WAITING_RES, dialNumberStr, (*dialNumberStr == '+') ? kGSMInternationalPhoneNumber : kGSMOtherPhoneNumber, kGSMVoice);
					}
				}
				else
				{
					// No previous communication
					TraceOutput(TL(TRACE_MODULE, "AGStartCommunication: First incoming call"));

					if (HF_SESSION(inbandRingtone))
					{
						// Start audio link for Inband ringtone
						error = PrvAGStartAudioLink();
						if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
						{
							TraceOutput(TL(TRACE_MODULE, "AGStartCommunication: ERROR while starting audio link (0x%04hX)", error));
						}
					}

					// Start alerting audio device
					PrvAGStartRinging(maxRing, ringTimeout, dialNumberStr);
				}
			}
			else
			{
				// Start audio link
				error = PrvAGStartAudioLink();
				if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
				{
					TraceOutput(TL(TRACE_MODULE, "AGStartCommunication: ERROR while starting audio link (0x%04hX)", error));
				}
			}
			break;
	}
}

/***********************************************************************
 *
 * FUNCTION:    AGStopCommunication
 *
 * DESCRIPTION:	.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGStopCommunication(void)
{
	status_t error;

	PrvAGStopRinging();

	error = PrvAGStopAudioLink();
	if (error != btLibErrNoError)
	{
		TraceOutput(TL(TRACE_MODULE, "AGStopCommunication: ERROR stop audio link failed (0x%04hX)", error));
	}
}

/***********************************************************************
 *
 * FUNCTION:    AGGetCallStates
 *
 * DESCRIPTION:	Get call and callsetup states.
 *
 * PARAMETERS:	serviceP - pointer to service available state
 *				callP - pointer to call state
 *				callsetupP - pointer to callsetup state
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGGetCallStates(Boolean *serviceP, Boolean *callP, AGCallsetupState *callsetupP)
{
	if (serviceP)
		*serviceP = SESSION(serviceAvailable);
	if (callP)
		*callP = SESSION(call);
	if (callsetupP)
		*callsetupP = SESSION(callsetup);
}

/***********************************************************************
 *
 * FUNCTION:    AGSetCallStates
 *
 * DESCRIPTION:	Change call and callsetup states.
 *
 * PARAMETERS:	service - network service available state
 *				call - call state
 *				callsetup - callsetup state
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGSetCallStates(Boolean service, Boolean call, AGCallsetupState callsetup)
{
	Boolean previousServiceAvailable = SESSION(serviceAvailable);
	Boolean previousCallState = SESSION(call);
	AGCallsetupState previousCallsetupState = SESSION(callsetup);

	// Change Service available state
	if (previousServiceAvailable != service)
	{
		TraceOutput(TL(TRACE_MODULE, "AGSetCallStates: Service:   %d -> %d", previousServiceAvailable, service));
		SESSION(serviceAvailable) = service;
		if ((SESSION(connectedProfile) == AGHandsfree) && HF_SESSION(eventReporting))
			PrvAGSocketSend(AG_HF_IND_EVT_RES, AG_HF_IND_SERVICE, SESSION(serviceAvailable) ? 1 : 0);
	}

	// Change Call state
	if (previousCallState != call)
	{
		TraceOutput(TL(TRACE_MODULE, "AGSetCallStates: Call:      %d -> %d", previousCallState, call));
		SESSION(call) = call;
		if ((SESSION(connectedProfile) == AGHandsfree) && HF_SESSION(eventReporting))
			PrvAGSocketSend(AG_HF_IND_EVT_RES, AG_HF_IND_CALL, SESSION(call) ? 1 : 0);
	}

	// Change Callsetup state
	if (previousCallsetupState != callsetup)
	{
		if (callsetup == AGCallsetupNone)
			PrvAGStopRinging();

		TraceOutput(TL(TRACE_MODULE, "AGSetCallStates: Callsetup: %d -> %d", previousCallsetupState, callsetup));
		SESSION(callsetup) = callsetup;
		if ((SESSION(connectedProfile) == AGHandsfree) && HF_SESSION(eventReporting))
			PrvAGSocketSend(AG_HF_IND_EVT_RES, AG_HF_IND_CALLSETUP, SESSION(callsetup));
	}
}

/***********************************************************************
 *
 * FUNCTION:    AGActivateInbandRingtone
 *
 * DESCRIPTION:	Activate/deactivate "Inband ringtone" if it is supported by
 *				audio gateway.
 *
 * PARAMETERS:	activate - do inband ringtone activation
 *
 * RETURNED:	
 *				btLibErrError - "Inband ringtone" is not suported.
 *
 ***********************************************************************/
status_t AGActivateInbandRingtone(Boolean activate)
{
	if (!GW_SUPPORTS(kAGHandsfreeGWInbandRingtone))
		return btLibErrError;

	PrvAGSocketSend(AG_HF_INBAND_RINGTONE_RES, activate);

	return btLibErrNoError;
}

/***********************************************************************
 *
 * FUNCTION:    AGActivateVoiceRecognition
 *
 * DESCRIPTION:	Activate/deactivate "Voice Recognition" if it is supported by
 *				audio gateway and audio device.
 *
 * PARAMETERS:	activate - do voice recognition activation
 *
 * RETURNED:	
 *				btLibErrError - "Voice Recognition" is not suported.
 *
 ***********************************************************************/
status_t AGActivateVoiceRecognition(Boolean activate)
{
	status_t error;

	if (!GW_SUPPORTS(kAGHandsfreeGWVoiceRecognition))
		return btLibErrError;

	error = PrvAGSocketSend(AG_HF_VOICE_RECOGNITION_RES, activate);
	if (error != btLibErrPending)
		return error;

	if (activate)
	{
		// If we activate "Voice recognition", start audio link if needed
		error = PrvAGStartAudioLink();
		if ((error != btLibErrNoError) && (error != btLibErrPending) && (error != btLibErrBusy))
		{
			TraceOutput(TL(TRACE_MODULE, "AGActivateVoiceRecognition: ERROR while starting audio link (0x%04hX)", error));
			return error;
		}
	}

	return btLibErrNoError;
}

/***********************************************************************
 *
 * FUNCTION:    AGGetState
 *
 * DESCRIPTION:	Return Audio gateway state.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	Audio gateway state
 *
 ***********************************************************************/
AGState AGGetState(void)
{
	return AG(state);
}

/***********************************************************************
 *
 * FUNCTION:    AGGetRemoteDevice
 *
 * DESCRIPTION:	Get remote audio device address.
 *
 * PARAMETERS:	addrP - pointer to bluetooth address structure
 *
 * RETURNED:	nothing
 *
 ***********************************************************************/
void AGGetRemoteDevice(BtLibDeviceAddressTypePtr addrP)
{
	*addrP = SESSION(btAddr);
}

/***********************************************************************
 *
 * FUNCTION:    AGSendData
 *
 * DESCRIPTION:	Send data.
 *
 * PARAMETERS:	data - buffer containing the data to send
 *				size - size of data to send
 *
 * RETURNED:	
 *				btLibErrParamError - bad volume.
 *				btLibErrSocket - bad socket.
 *				btLibErrPending - The results will be returned through a callback.
 *				btLibErrBusy - A send is already in process.  
 *				btLibErrNoAclLink - ACL link for remote device does not exist
 *				btLibErrSocketRole - Socket is not connected.	
 *
 ***********************************************************************/
status_t AGSendData(uint8_t *dataP, uint8_t size)
{
	return PrvAGSocketSendRaw(dataP, size);
}

/***********************************************************************
 *
 * FUNCTION:    AGChangeSpeakerVolume
 *
 * DESCRIPTION:	Change speaker volume.
 *
 * PARAMETERS:	volume - new volume value for the speaker.
 *
 * RETURNED:	
 *				btLibErrError - Remote volume control is not supported by audio device.
 *				btLibErrParamError - bad volume.
 *				btLibErrSocket - bad socket.
 *				btLibErrPending - The results will be returned through a callback.
 *				btLibErrBusy - A send is already in process.  
 *				btLibErrNoAclLink - ACL link for remote device does not exist
 *				btLibErrSocketRole - Socket is not connected.	
 *
 ***********************************************************************/
status_t AGChangeSpeakerVolume(uint8_t volume)
{
	if (!REMOTE_VOLUME_SUPPORTED)
		return btLibErrError;

	if (volume > kAGHeadsetSpeakerMaxVolume)
		return btLibErrParamError;

	SESSION(mikeGain) = volume;

	return PrvAGSocketSend(AG_SPK_VOL_RES, volume);
}

/***********************************************************************
 *
 * FUNCTION:    AGChangeMicVolume
 *
 * DESCRIPTION:	Change microphone volume.
 *
 * PARAMETERS:	volume - new volume value for the microphone.
 *
 * RETURNED:	
 *				btLibErrError - Remote volume control is not supported by audio device.
 *				btLibErrParamError - bad volume.
 *				btLibErrSocket - bad socket.
 *				btLibErrPending - The results will be returned through a callback.
 *				btLibErrBusy - A send is already in process.  
 *				btLibErrNoAclLink - ACL link for remote device does not exist
 *				btLibErrSocketRole - Socket is not connected.	
 *
 ***********************************************************************/
status_t AGChangeMicVolume(uint8_t volume)
{
	if (!REMOTE_VOLUME_SUPPORTED)
		return btLibErrError;

	if (volume > kAGHeadsetMicroMaxVolume)
		return btLibErrParamError;

	SESSION(speakerGain) = volume;

	return PrvAGSocketSend(AG_MIC_VOL_RES, volume);
}

