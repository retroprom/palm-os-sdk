/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BtAudioGateway.h
 *
 * Release: 
 *
 * Description:
 *	Bluetooth Audio gateway Profiles (HSP HFP) - Audio Gateway role sample
 *
 *****************************************************************************/

#ifndef __BTAUDIOGATEAWAY_H__
#define __BTAUDIOGATEAWAY_H__

// Maximum value of volume from the Bluetooth Headset Profile
#define kAGHeadsetSpeakerMaxVolume				15
#define kAGHeadsetMicroMaxVolume				15

// Maximum length of phone number
#define kAGMaxPhoneNumberLength					32

// Handsfree SDP record constant
#define kAGHandsfreeECNR						0x01
#define kAGHandsfree3Way						0x02
#define kAGHandsfreeCLI							0x04
#define kAGHandsfreeVoiceRecognition			0x08
#define kAGHandsfreeRemoteVolume				0x10

#define kAGHandsfreeGW3Way						0x01
#define kAGHandsfreeGWECNR						0x02
#define kAGHandsfreeGWVoiceRecognition			0x04
#define kAGHandsfreeGWInbandRingtone			0x08
#define kAGHandsfreeGWVoiceTagAttach			0x10
#define kAGHandsfreeGWRejectCall				0x20

// Audio Gateway supported profiles
typedef enum
{
	AGNone,
	AGHeadset,
	AGHandsfree
} AGProfile;

// Event codes for Audio Gateway callbacks
typedef enum _AGEventEnum
{
	// the Audio Gateway state changed
	AGEventStateChange,

	// the Audio Gateway opened a file descriptor
	AGEventFdOpened,

	// the Audio Gateway closed a file descriptor
	AGEventFdClosed,

	// Remote headset service audio control is supported
	AGEventRemoteVolume,

	// Remote handsfree supported features has been updated
	AGEventSupportedFeatures,

	// Speaker volume changed
	AGEventSpeakerVolume,

	// Microphone volume changed
	AGEventMicroVolume,
		
	// Call answered by audio device
	AGEventCallAnswered,

	// Call Hangup by audio device
	AGEventCallHangup,

	// Dial number
	AGEventDialNumber,
	
	// Dial using memory
	AGEventDialMemory,

	// Call hold requested by audio device
	AGEventCallHold,

	// DTMF keypad received
	AGEventDTMF,

	// Get phone number to attach voice tag into Hands-free device
	AGEventGetPhoneNumber,

	// Dial last number
	AGEventDialLastNumber,

	// Audio device change "Voice recognition" state
	// if enabled, the audio link is automatically started
	AGEventVoiceRecognition,

	// Audio device disable "Echo canceling and noise reduction"
	AGEventECNR,

	// Unknown command
	AGEventCommand
} AGEventEnum;

// States of Audio Gateway
typedef enum
{
	AGDisconnected,
	AGStarting,
	AGConnecting,
	AGConnected,
	AGAudioConnecting,
	AGAudioConnected,
	AGDisconnecting
} AGState;

//
// CallSetup states
//
typedef enum
{
	AGCallsetupNone,
	AGCallsetupIncoming,
	AGCallsetupOutgoing,
	AGCallsetupAlerted
} AGCallsetupState;

// Event for the Audio Gateway callbacks
typedef struct _AGEventType
{
	// event
 	//     Description: Event code for callback.
 	//	   Events: All
	AGEventEnum event;

	union
	{
		// socket
		//	   Description: a socket has been opened or closed.
		//	   Events: AGEventSocketOpened
		//				AGEventSocketClosed
		BtLibSocketRef socket;

		// state
		//	   Description: state changed.
		//	   Events: AGEventStateChange
		struct
		{
			AGState state;
			AGProfile profile;
			status_t status;
		} state;

		// features
		//	   Description: remote supported features has been updated.
		//	   Events:	AGEventSupportedFeatures
		uint16_t features;

		// volume
		//	   Description: audio device volume changed.
		//	   Events:	AGEventSpeakerVolume
		//				AGEventMicroVolume
		uint8_t volume;

		// number
		//	   Description: Dial number.
		//	   Events:	AGEventDialNumber
		//				AGEventDialMemory
		char* number;

		// holdMode
		//	   Description: Call hold mode.
		//	   Events:	AGEventCallHold
		uint8_t holdMode;

		// key
		//	   Description: DTMF keypad.
		//	   Events:	AGEventDTMF
		char key;

		// phoneP
		//	   Description: pointer to store phone number (max 32 digits, kAGMaxPhoneNumberLength).
		//	   Events:	AGEventGetPhoneNumber
		char *phoneP;

		// enabled
		//	   Description: is feature enbled.
		//	   Events:	AGEventVoiceRecognition
		Boolean enabled;

		// cmdP
		//	   Description: Unknown command.
		//	   Events:	AGEventCommand
		struct
		{
			char *cmdP;
			Boolean *handledP;
		} cmd;
	} info;
} AGEventType;

// Audio Gateway Callback type
typedef void (*AGCallback) (AGEventType *eventP);

//--------------------------------------------------------------------
// Initializes the audio gateway.
// This function should be called first.
//
// callback - Audio gateway callback.
void AGInit(AGCallback callback);

//--------------------------------------------------------------------
// Manage events.
//
// eventP - pointer to the event.
Boolean AGEventCallback(EventType * eventP);

//--------------------------------------------------------------------
// Manage IOS data.
//
// iFds - file descriptor set.
// iNfds - number of file descriptor.
void AGIosCallback(struct pollfd iFds[], int32_t iNfds);

//--------------------------------------------------------------------
// Prepare audio gateway to be used as client.
// profile - profile to use.
//
// Returns :
//			btLibErrNoError - successfull
//			btLibErrParamError - if profile is not valid
//			btLibErrBusy - client is already registered
//			errors from :
//				BtLibRegisterManagementNotification()
//
// Events :
//			None
//
status_t AGRegisterClient(AGProfile profile);

//--------------------------------------------------------------------
// Stop using audio gateway as client
//
// Returns :
//			btLibErrNoError - successfull
//			errors from :
//				BtLibUnregisterManagementNotification()
//
// Events :
//			None
//
status_t AGUnregisterClient(void);

//--------------------------------------------------------------------
// Prepare audio gateway to be used as server
// The audio gateway server could be started as a normal applicative server or
// as a Bluetooth service.
// When started as a normal applicative server, this function should be called after
// BtLibOpen().
// When started as a Bluetooth service, BtLib should be closed to receive incoming
// connection. The bluetooth audio gateway service will be started automatically.
//
// profile - profile to use.
// asService - true if the server should be started as a Bluetooth service
// type - type of the parent application, only needed if started as service.
// creator - creator of the parent application, only needed if started as service.
// hfFeatures - Handsfree gateway supported features
//
// Returns :
//			btLibErrNoError - successfull
//			btLibErrBusy - server is already registered
//			errors from :
//				as service :
//					SysNotifyRegister()
//				as normal applicative server :
//					BtLibRegisterManagementNotification()
//					BtLibSocketCreate()
//					BtLibSocketListen()
//					BtLibSdpServiceRecordCreate()
//					BtLibSdpServiceRecordSetAttributesForSocket()
//					BtLibSdpServiceRecordSetAttribute()
//					BtLibSdpServiceRecordStartAdvertising()
//
// Events :
//			None
//
status_t AGRegisterServer(AGProfile profile, Boolean asService, uint32_t type, uint32_t creator, uint16_t hfFeatures);

//--------------------------------------------------------------------
// Stop using audio gateway as server
//
// Returns :
//			btLibErrNoError - successfull
//			errors from :
//				as service :
//					SysNotifyUnregister()
//
// Events :
//			None
//
status_t AGUnregisterServer(void);

//--------------------------------------------------------------------
// Connect to audio device
//
// remoteDeviceP -> a remote audio device address 
//
// Returns :
//			btLibErrParamError - Pointer to remote device addresses is NULL.
//			btLibErrPending - The results will be returned through client callback
//			btLibErrBusy - The audio gateway is not disconnected or alreday used as server
//			errors from :
//				BtLibLinkConnect()
//
// Events :
//			AGEventStateChange -Audio Gateway state changed
//			AGEventRemoteVolume - the Remote service audio control is supported
//
status_t AGConnect(BtLibDeviceAddressTypePtr remoteDeviceP);

//--------------------------------------------------------------------
// Disconnect from audio device
//
// Returns :
//			btLibErrNoError - The results will be returned through a callback
//			btLibErrPending - The results will be returned through a callback
//			errors from :
//				BtLibLinkDisconnect()
//
// Events :
//			AGEventStateChange -Audio Gateway state changed
//
status_t AGDisconnect(void);

//--------------------------------------------------------------------
// Terminate connection from audio device
// This function should be called after receiving AGEventStateChange event with state
// AGDisconnected, but not in the callback function
//
// Returns :
//			None
//
// Events :
//			None
//
void AGTerminateSession(void);

//--------------------------------------------------------------------
// Start communication
//
// incoming - true it is an incoming call.
// dialNumberStr - dial number string for "Line identification" (requested for incoming call).
// maxRing - maximun rings before canceling connection to audio device (requested for incoming call).
// ringTimeout - timeout between rings in seconds (requested for incoming call).
//
// Returns :
//			None
//
// Events :
//			AGEventStateChange -Audio Gateway state changed
//
void AGStartCommunication(Boolean incoming, const char dialNumberStr[kAGMaxPhoneNumberLength+1], uint8_t maxRing, uint8_t ringTimeout);

//--------------------------------------------------------------------
// Stop communication
//
// Returns :
//			None
//
// Events :
//			AGEventStateChange -Audio Gateway state changed
//
void AGStopCommunication(void);

//--------------------------------------------------------------------
// Get call and callsetup states
//
// serviceP - pointer to service available state
// callP - pointer to call state
// callsetupP - pointer to callsetup state
//
// Returns :
//			None
//
// Events :
//			None
//
void AGGetCallStates(Boolean *serviceP, Boolean *callP, AGCallsetupState *callsetupP);

//--------------------------------------------------------------------
// Change call and callsetup states
//
// service - network service available state
// call - call state
// callsetup - callsetup state
//
// Returns :
//			None
//
// Events :
//			None
//
void AGSetCallStates(Boolean service, Boolean call, AGCallsetupState callsetup);

//--------------------------------------------------------------------
// Activate/deactivate "Inband ringtone" if it is supported by audio gateway.
//
// activate - do inband ringtone activation
//
// Returns :
//			btLibErrError - "Inband ringtone" is not suported.
//
// Events :
//			None
//
status_t AGActivateInbandRingtone(Boolean activate);

//--------------------------------------------------------------------
// Activate/deactivate "Voice Recognition" if it is supported by audio gateway
// and audio device.
//
// activate - do inband ringtone activation
//
// Returns :
//			btLibErrError - "Voice Recognition" is not suported.
//
// Events :
//			None
//
status_t AGActivateVoiceRecognition(Boolean activate);

//--------------------------------------------------------------------
// Return audio gateway state
//
// Returns :
//			current Audio gateway state
//
// Events :
//			None
//
AGState AGGetState(void);

//--------------------------------------------------------------------
// Get bluetooth address of remote audio device
//
// addrP - a pointer to get the address of the remote audio device 
//
// Returns :
//			None
//
// Events :
//			None
//
void AGGetRemoteDevice(BtLibDeviceAddressTypePtr addrP);

//--------------------------------------------------------------------
// Change speaker gain of audio device
//
// data - buffer containing the data to send
// size - size of data to send
//
// Returns :
//			errors from :
//				BtLibSocketSend()
//
// Events :
//			None
//
status_t AGSendData(uint8_t *dataP, uint8_t size);

//--------------------------------------------------------------------
// Change speaker gain of audio device
//
// volume - new volume value for the speaker
//
// Returns :
//			btLibErrError - Remote volume control is not supported by audio device.
//			btLibErrParamError - if volume > kAGHeadsetSpeakerMaxVolume
//			errors from :
//				BtLibSocketSend()
//
// Events :
//			None
//
status_t AGChangeSpeakerVolume(uint8_t volume);

//--------------------------------------------------------------------
// Change microphone gain of audio device
//
// volume - new volume value for the microphone
//
// Returns :
//			btLibErrError - Remote volume control is not supported by audio device.
//			btLibErrParamError - if volume > kAGHeadsetMicroMaxVolume
//			errors from :
//				BtLibSocketSend()
//
// Events :
//			None
//
status_t AGChangeMicVolume(uint8_t volume);

#endif /* __BTAUDIOGATEAWAY_H__ */

