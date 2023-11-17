/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtAudioGatewayPrv.h
 *
 * Release: 
 *
 * Description:
 *	Bluetooth Audio gateway Profiles (HSP HFP) - Audio Gateway role sample
 *
 *****************************************************************************/

#ifndef __BTAUDIOGATEAWAYPRV_H__
#define __BTAUDIOGATEAWAYPRV_H__

//
// Headset Profile service constants
//
#define HEADSET_GW_SERVICE_NAME	"Voice gateway"	// Headset gateway service username
#define HEADSET_GW_VERSION 0x0100
#define HEADSET_REMOTE_AUDIO_VOLUME_CONTROL 0x0302

//
// Handsfree Profile service constants
//
#define HANDSFREE_GW_SERVICE_NAME	"Voice gateway"	// Headset gateway service username
#define HANDSFREE_GW_VERSION 0x0100
#define HANDSFREE_REMOTE_NETWORK 0x0301
#define HANDSFREE_REMOTE_SUPPORTED_FEATURES 0x0311
#define HANDSFREE_NETWORK_REJECT_CALL 0x01
#define HANDSFREE_SDP_FEATURES_MASK (kAGHandsfreeGW3Way | kAGHandsfreeGWECNR | \
			kAGHandsfreeGWVoiceRecognition | kAGHandsfreeGWInbandRingtone | kAGHandsfreeGWVoiceTagAttach)

//
// AT commands
//
  // Headset
#define AG_BUTTON_CMD "AT+CKPD=200"
  // Handsfree
#define AG_HF_IND_SERVICE 1
#define AG_HF_IND_CALL 2
#define AG_HF_IND_CALLSETUP 3
#define AG_HF_CALL_ANSWER_CMD "ATA"
#define AG_HF_CALL_CMD "ATD"
#define AG_HF_CALL_WAITING_CMD "AT+CCWA="
#define AG_HF_CALL_HOLD_CMD "AT+CHLD="
#define AG_HF_HANG_UP_CMD "AT+CHUP"
#define AG_HF_IND_TEST_CMD "AT+CIND=?"
#define AG_HF_IND_READ_CMD "AT+CIND?"
#define AG_HF_LINE_IDENT_CMD "AT+CLIP="
#define AG_HF_EVENT_REPORTING_CMD "AT+CMER="
#define AG_HF_DTMF_CMD "AT+VTS="
#define AG_HF_INPUT_CMD "AT+BINP="
#define AG_HF_DIAL_LAST_NUMBER_CMD "AT+BLDN"
#define AG_HF_VOICE_RECOGNITION_CMD "AT+BVRA="
#define AG_HF_SUPPORTED_FEATURES_CMD "AT+BRSF="
#define AG_HF_NOISE_REDUCTION_CMD "AT+NREC="
#define AG_HF_CALL_WAITING_RES "\r\n+CCWA:%s,%d,%d\r\n"
#define AG_HF_CALL_HOLD_RES "\r\n+CHLD:(0,1,2,3,4)\r\n"
#define AG_HF_IND_TEST_RES "\r\n+CIND:(\"service\",(0-1)),(\"call\",(0-1)),(\"callsetup\",(0-3))\r\n"
#define AG_HF_IND_READ_RES "\r\n+CIND:%d,%d,%d\r\n"
#define AG_HF_LINE_IDENT_RES "\r\n+CLIP:%s,%d\r\n"
#define AG_HF_IND_EVT_RES "\r\n+CIEV:%d,%d\r\n"
#define AG_HF_INPUT_PHONE_NUMBER_RES "\r\n+BINP:"
#define AG_HF_VOICE_RECOGNITION_RES "\r\n+BVRA:%d\r\n"
#define AG_HF_SUPPORTED_FEATURES_RES "\r\n+BRSF:%d\r\n"
#define AG_HF_INBAND_RINGTONE_RES "\r\n+BSIR:%d\r\n"
  // Common
#define AG_SPK_VOL_CMD "AT+VGS="
#define AG_MIC_VOL_CMD "AT+VGM="
#define AG_SPK_VOL_RES "\r\n+VGS=%d\r\n"
#define AG_MIC_VOL_RES "\r\n+VGM=%d\r\n"
#define AG_RING "\r\nRING\r\n"
#define AG_OK "\r\nOK\r\n"
#define AG_ERROR "\r\nERROR\r\n"

//
// GSM constants
//
#define kGSMInternationalPhoneNumber 145
#define kGSMOtherPhoneNumber 129
#define kGSMVoice 1

//
// Internal constants
//
#define kCommandBufferSize 200
#define kAnswerSize 100
#define kAnswerBufferSize 3*kAnswerSize
#define kRFCommMaxFrameSize 500

#define kHSPipeName "headset_pipe"
#define kHFPipeName "handsfree_pipe"

// Event for timer
#define kAGEventTimerFired (eventsEnum)(firstUserEvent)

// Audio Gateway Handsfree commands
typedef enum
{
	AG_HFGetSupportedFeatures,
	AG_HFConnectedInbound
} AGHFEvents;

//
// Internal socket type
//
typedef struct _AGSocketRef
{
	BtLibSocketRef socket;
	Boolean used;
} AGSocketRef;

//
// Internal buffer type
// Used to receive or send data
//
typedef struct _AGBufferType
{
	char command[kCommandBufferSize];				// Buffer for AT Commands
} AGBufferType;

//
// Server data type
//
typedef struct _AGServerType
{
	BtLibSdpUuidType sdpUUIDList[2];				// UUID list for audio gateway

	MemHandle sdpHandle;							// local SDP record for audio gateway
	MemHandle sdpDescriptorHandle;					// local SDP descriptor entry for audio gateway

	AGSocketRef listenerSocket; 					// RFComm listener socket
	AGSocketRef scoListenerSocket;					// SCO listener socket 

	int32_t pipe[2];								// named pipe for service style
} AGServerType;

//
// Headset session type
//
typedef struct _AGHeadsetSessionType
{
	Boolean remoteVolume;							// is remote volume supported in Handsfree
} AGHeadsetSessionType;

//
// Handsfree session type
//
typedef struct _AGHandsfreeSessionType
{
	uint16_t supportedfeatures;						// supported features in Handsfree
	uint16_t gwSupportedfeatures;					// supported features in Handsfree gateway

	Boolean eventReporting;							// is event reporting enabled
	Boolean eventReportingMode;						// is event reporting mode direct
	Boolean sendCLI;								// is "Call line identification" available
	Boolean inbandRingtone;							// is inband ringtone activated

	Char phoneStr[kAGMaxPhoneNumberLength]; 		// dial number string for "Line identification"
} AGHandsfreeSessionType;

//
// Session type
//
typedef struct _AGSessionType
{
	BtLibSdpUuidType sdpHSUUIDList[3];				// UUID list for Headset audio device
	BtLibSdpUuidType sdpHFUUIDList[3];				// UUID list for Handsfree audio device

	BtLibDeviceAddressType btAddr;					// Remote bluetooth device's address.

	AGProfile connectedProfile;						// profile connected

	uint8_t maxRing;								// maximum number of rings before disconnecting
	uint8_t ringTimeout;							// time in sec between 2 rings
	uint8_t ringCount;								// count number of ring sent

	uint8_t speakerGain;							// remote speaker volume
	uint8_t mikeGain;								// remote microphone volume

	BtLibSdpRecordHandle sdpRecordHandle;			// remote SDP record
	uint16_t numRemoteHandles;						// num of remote SDP handle

	uint16_t remoteInfoSize;						// size of info attribute in remote SDP record
	uint8_t *remoteInfoP;							// info attribute in remote SDP record

	AGSocketRef rfcSocket;							// RFComm socket
	AGSocketRef sdpSocket;							// SDP socket
	AGSocketRef scoSocket;							// SCO socket

	Boolean serviceAvailable;						// is Home/Roam network available
	Boolean call;									// is call active
	AGCallsetupState callsetup;						// state of call setup

	union
	{
		AGHeadsetSessionType headset;				// headset profile session info
		AGHandsfreeSessionType handsfree;			// handsfree profile session info
	} profile;
} AGSessionType;

//
// Audio Gateway data type
//
typedef struct _AGInternalType
{
	AGState state;									// audio gateway connection state

	int32_t fdME;									// management entity file descriptor

	AGSessionType session;							// session info

	struct
	{
		Boolean asService;							// server started as BtLib service
		MemHandle dbH;								// MemHandle for service notification
		
		Boolean running;							// server side is running
		
		AGServerType headset;						// headset profile server info
		AGServerType handsfree;						// handsfree profile server info
	} server;

	AGProfile clientProfile;						// profiles to use for client
	AGProfile serverProfile;						// profiles to use for server

	AGCallback callback;							// audio gateway callback
	uint8_t managementCallbackCount;				// is the Me device opened ?

	AGBufferType buffer;							// internal buffers
} AGInternalType;

static AGInternalType gInternal;

// Shortcuts
#define AG(x) gInternal.x
#define SESSION(x) AG(session.x)
#define HS_SESSION(x) SESSION(profile.headset.x)
#define HF_SESSION(x) SESSION(profile.handsfree.x)
#define SERVER(x) AG(server.x)
#define HS_SERVER(x) SERVER(headset.x)
#define HF_SERVER(x) SERVER(handsfree.x)
#define BUFFER(x) AG(buffer.x)

#define BTSOCKET(x) (x).socket
#define BTSOCKET_USED(x) (x).used

#define AUDIO_SUPPORTS(feature) ((HF_SESSION(supportedfeatures) & feature) == feature)
#define GW_SUPPORTS(feature) ((HF_SESSION(gwSupportedfeatures) & feature) == feature)
#define REMOTE_VOLUME_SUPPORTED ((SESSION(connectedProfile) == AGHeadset) ? HS_SESSION(remoteVolume) : (SESSION(connectedProfile) == AGHandsfree) ? AUDIO_SUPPORTS(kAGHandsfreeRemoteVolume) : false)

#define CLIENT_SUPPORTS(profile) ((AG(clientProfile) & profile) == profile)
#define SERVER_SUPPORTS(profile) ((AG(serverProfile) & profile) == profile)

#endif /* __BTAUDIOGATEAWAYPRV_H__ */

