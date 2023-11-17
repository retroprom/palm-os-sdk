/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: TelephonyLibTypes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             	This is the header file declaring the data types used by the 
 *				Telephony Manager for Palm OS Wireless Telephony Add-on.
 *
 *****************************************************************************/

#ifndef TelephonyLibTypes_h
#define TelephonyLibTypes_h


/******************************************************************************
 *	Includes
 *****************************************************************************/

#include <PalmTypes.h>
#include <Event.h>
#include <time.h>


/******************************************************************************
 *	Defines
 *****************************************************************************/

// Services id
enum TelServices
{
	kTelCncServiceId,
	kTelNwkServiceId,
	kTelStyServiceId,
	kTelPowServiceId,
	kTelCfgServiceId,
	kTelSmsServiceId,
	kTelEmcServiceId,
	kTelSpcServiceId,
	kTelPhbServiceId,
	kTelSndServiceId,
	kTelInfServiceId,
	kTelOemServiceId,
	kTelGprsServiceId,
	kTelCatServiceId,
	kTelMuxServiceId,

	kTelLastServiceId = kTelMuxServiceId
};

// Messages
enum TelMessages
{
	// Internal
	kTelIncomingMessage,
	kTelPollMessage,
	kTelDtcStartMessage,
	kTelDtcStopMessage,

	// General
	kTelCancelMessage,
	kTelTestPhoneDriverMessage,
	kTelIsServiceAvailableMessage,
	kTelIsFunctionSupportedMessage,
	
	// Connection
	kTelCncOpenMessage,
	kTelCncCloseMessage,
	kTelCncGetStatusMessage,

	// Network
	kTelNwkGetOperatorsMessage,
	kTelNwkGetOperatorMessage,
	kTelNwkSetOperatorMessage,
	kTelNwkGetPreferredOperatorsMessage,
	kTelNwkAddPreferredOperatorMessage,
	kTelNwkDeletePreferredOperatorMessage,
	kTelNwkGetRegistrationModeMessage,
	kTelNwkSetRegistrationMessage,
	kTelNwkGetTypeMessage,
	kTelNwkGetStatusMessage,
	kTelNwkGetProviderIdMessage,
	kTelNwkGetLocationMessage,
	kTelNwkGetSignalLevelMessage,
	kTelNwkCheckUssdMessage,
	kTelNwkSendUssdMessage,
	kTelNwkReceiveUssdMessage,
	kTelNwkCancelUssdMessage,

	// Security
	kTelStyGetAuthenticationStatusMessage,
	kTelStyEnterAuthenticationMessage,
	kTelStyGetFacilitiesMessage,
	kTelStyGetFacilityMessage,
	kTelStyLockFacilityMessage,
	kTelStyUnlockFacilityMessage,
	kTelStyChangeFacilityPasswordMessage,

	// Power
	kTelPowGetBatteryChargeLevelMessage,
	kTelPowGetBatteryConnectionStatusMessage,
	kTelPowSetPhoneFunctionalityMessage,

	// Configuration
	kTelCfgGetSmsCenterMessage,
	kTelCfgSetSmsCenterMessage,
	kTelCfgGetPhoneNumberMessage,
	kTelCfgSetPhoneNumberMessage,
	kTelCfgGetVoiceMailNumberMessage,
	kTelCfgSetVoiceMailNumberMessage,
	kTelCfgGetLoudspeakerVolumeLevelRangeMessage,
	kTelCfgGetLoudspeakerVolumeLevelMessage,
	kTelCfgSetLoudspeakerVolumeLevelMessage,
	kTelCfgGetRingerSoundLevelRangeMessage,
	kTelCfgGetRingerSoundLevelMessage,
	kTelCfgSetRingerSoundLevelMessage,
	kTelCfgGetVibratorModeMessage,
	kTelCfgSetVibratorModeMessage,
	kTelCfgGetAlertSoundModeMessage,
	kTelCfgSetAlertSoundModeMessage,
	kTelCfgGetCallForwardingMessage,
	kTelCfgSetCallForwardingMessage,
	kTelCfgGetCallIdRestrictionStatusMessage,
	kTelCfgSetCallIdRestrictionStatusMessage,

	// SMS
	kTelSmsSendMessageMessage,
	kTelSmsReadMessagesMessage,
	kTelSmsReadMessageMessage,
	kTelSmsDeleteMessageMessage,
	kTelSmsGetUniquePartIdMessage,
	kTelSmsGetDataMaxSizeMessage,
	kTelSmsGetStoragesMessage,
	kTelSmsGetStorageMessage,
	kTelSmsSetStorageMessage,

	// Emergency call
	kTelEmcDialMessage,

	// Speech call
	kTelSpcGetCallsMessage,
	kTelSpcGetCallMessage,
	kTelSpcInitiateCallMessage,
	kTelSpcAcceptCallMessage,
	kTelSpcReleaseCallMessage,
	kTelSpcHoldActiveCallsMessage,
	kTelSpcAddHeldCallMessage,
	kTelSpcPrivateCallMessage,
	kTelSpcPlayToneMessage,
	kTelSpcGetToneDurationRangeMessage,
	kTelSpcGetToneDurationMessage,
	kTelSpcSetToneDurationMessage,

	// Phonebook
	kTelPhbGetPhonebooksMessage,
	kTelPhbGetPhonebookMessage,
	kTelPhbSetPhonebookMessage,
	kTelPhbGetEntriesMessage,
	kTelPhbGetEntryMessage,
	kTelPhbAddEntryMessage,
	kTelPhbDeleteEntryMessage,

	// Sound
	kTelSndGetMuteStatusMessage,
	kTelSndSetMuteStatusMessage,

	// Information
	kTelInfGetIdentificationMessage,
	kTelInfGetCallsDurationMessage,
	kTelInfResetCallsDurationMessage,
	kTelInfGetCallsListMessage,
	kTelInfResetCallsListMessage,

	// OEM
	kTelOemCallMessage,

	// GPRS
	kTelGprsGetAvailableContextIdMessage,
	kTelGprsSetContextMessage,
	kTelGprsGetContextMessage,
	kTelGprsGetDefinedCidsMessage,
	kTelGprsSetAttachMessage,
	kTelGprsGetAttachMessage,
	kTelGprsSetPdpActivationMessage,
	kTelGprsGetPdpActivationMessage,
	kTelGprsGetPdpAddressMessage,
	kTelGprsSetNwkRegistrationMessage,
	kTelGprsGetNwkRegistrationMessage,
	kTelGprsSetSmsServiceMessage,
	kTelGprsGetSmsServiceMessage,
	kTelGprsSetQosRequestedMessage,
	kTelGprsGetQosRequestedMessage,
	kTelGprsSetQosMinimumMessage,
	kTelGprsGetQosMinimumMessage,
	kTelGprsGetQosCurrentMessage,
	kTelGprsSetEventReportingMessage,
	kTelGprsGetEventReportingMessage,
	kTelGprsGetDataCounterMessage,


	kTelCardGetFileMessage,
	kTelCatSetConfigMessage,
	kTelCatGetConfigMessage,
	kTelCatTerminateMessage,
	kTelCatGetCmdParametersMessage,
	kTelCatSetCmdResponseMessage,
	kTelCatMenuSelectionMessage,
	kTelCatCallActionMessage,
	kTelCatNotifyCardOfEventMessage,	

	// Mux
	kTelMuxEnableMessage,
	kTelMuxChanAllocateMessage,
	kTelMuxChanFreeMessage,
	kTelMuxChanSetIdMessage,
	
	kTelLastMessage = kTelMuxChanSetIdMessage
};

/******************************************************************************
 *	Structures
 *****************************************************************************/

// Notification structure
typedef struct _TelNotificationType
{
   	uint32_t					data; 		// associated data if any
	uint32_t					data2; 		// associated data if any
	uint32_t					timeStamp;	// time stamp
	uint16_t					id;			// what was the associated telephony event
	uint8_t						priority;	// notification priority 0 == max, 255 == min
	uint8_t						padding;
} TelNotificationType, *TelNotificationPtr;

typedef struct TelNotifyErrorDetailsTag
{
	uint16_t					version;
	uint16_t					padding;
	status_t					error;	
	uint32_t					ioFlags;	
	char*						messageP;
} TelNotifyErrorDetailsType, *TelNotifyErrorDetailsPtr;

// Event structure
typedef struct _TelEventType
{
   	eventsEnum					eType;
	int16_t						screenX;
   	int16_t						screenY;
   	Boolean						penDown;
   	uint8_t						tapCount;
	uint16_t					padding;

	MemPtr						paramP;				// parameter passed at asynchronous function call
	uint16_t					functionId;			// ID of the message associated to the asynchronous function call
	uint16_t					transId;			// transId returned on asynchronous function call return
	status_t					returnCode;			// function return code, errNone if ok, else an error
} TelEventType, *TelEventPtr;

// Number
typedef struct _TelNumberType
{
	char*						numberP;
	size_t						size;
	uint16_t					type;
	uint16_t					padding;
} TelNumberType, *TelNumberPtr;

// Network
typedef struct _TelNwkOperatorType
{
	uint32_t					id;
	char*						nameP;
	size_t						nameSize;
	uint8_t						type;
	uint8_t						status;
	uint8_t						padding[2];
} TelNwkOperatorType, *TelNwkOperatorPtr;

typedef struct _TelNwkOperatorsType
{
	TelNwkOperatorPtr			listP;		// Array of network
	size_t						count;		// Count of networks
} TelNwkOperatorsType, *TelNwkOperatorsPtr;

typedef struct _TelNwkPreferredOperatorType
{
	uint32_t					id;
	char*						nameP;
	size_t						nameSize;
	uint16_t					index;
	uint16_t					padding;
} TelNwkPreferredOperatorType, *TelNwkPreferredOperatorPtr;

typedef struct _TelNwkPreferredOperatorsType
{
	TelNwkPreferredOperatorPtr	listP;
	size_t						count;
} TelNwkPreferredOperatorsType, *TelNwkPreferredOperatorsPtr;

typedef struct _TelNwkLocationType
{
	char*						areaCodeP;
	size_t						areaCodeSize;
	char*						cellIdP;
	size_t						cellIdSize;
} TelNwkLocationType, *TelNwkLocationPtr;

typedef struct _TelNwkUssdType
{
	char*						bufferP;
	size_t						bufferSize;
	uint8_t						result;
	uint8_t						dataCodingScheme;
	uint8_t						padding[2];
} TelNwkUssdType, *TelNwkUssdPtr;

typedef struct _TelNwkRegistrationType
{
	uint32_t					operatorId;		// operatorId if mode is kTelNwkRegistrationManual or kTelNwkRegistrationManualAutomatic
	uint8_t					mode;			// registration mode
	uint8_t					padding[3];
} TelNwkRegistrationType, *TelNwkRegistrationPtr;


// Security
typedef struct _TelStyAuthenticationType
{
	char*						passwordP;			// For internal usage
	size_t						passwordSize;		// For internal usage
	char*						newPasswordP;		// For internal usage
	size_t						newPasswordSize;	// For internal usage
	uint16_t					type;				// Input: Type of authentification
	uint16_t					reserved;			// For internal usage
} TelStyAuthenticationType, *TelStyAuthenticationPtr;

typedef struct _TelStyFacilitiesType
{
	uint16_t*					idP;
	size_t						count;
} TelStyFacilitiesType, *TelStyFacilitiesPtr;

typedef struct _TelStyFacilityType
{
	char*						passwordP;
	size_t						passwordSize;
	uint16_t					type;
	uint8_t						status;
	uint8_t						classType;
} TelStyFacilityType, *TelStyFacilityPtr;

typedef struct _TelStyFacilityPasswordType
{
	char*						passwordP;
	size_t						passwordSize;
	char*						newPasswordP;
	size_t						newPasswordSize;
	uint16_t					type;
	uint16_t					padding;
} TelStyFacilityPasswordType, *TelStyFacilityPasswordPtr;

// Configuration
typedef struct _TelCfgPhoneNumberType
{
	TelNumberType				voice;
	TelNumberType				fax;
	TelNumberType				data;
} TelCfgPhoneNumberType, *TelCfgPhoneNumberPtr;

typedef struct _TelCfgLevelRangeType
{
	uint8_t						min;
	uint8_t						max;
	uint8_t						padding[2];
} TelCfgLevelRangeType, *TelCfgLevelRangePtr;

typedef struct _TelCfgCallForwardingType
{
	TelNumberType				number;		// Number of forwarding address
	TelNumberType				subAddr;	// 
	uint8_t						reason;		// Call forwarding reason: see define kTelCfgForwardingReasonXXX in TelephonyLib.h
	uint8_t						mode;		// Call forwarding mode: see define kTelCfgForwardingModeXXX in TelephonyLib.h
	uint8_t						classType;	// Class of information: see define kTelCfgForwardingClassXXX in TelephonyLib.h
	uint8_t						time;		// When reason is "No reply)", this is the time in seconds to wait before call is forwarded (default value 20)
	uint8_t						status;		// 0 means "not active", and 1 means "active"
	uint8_t						padding[3];
} TelCfgCallForwardingType, *TelCfgCallForwardingPtr;

// SMS
typedef struct _TelSmsDateTimeType 
{
	uint32_t					dateTime;	// relative time from now, or Palm absolute time
	Boolean						absolute;
	uint8_t						padding[3];
} TelSmsDateTimeType, *TelSmsDateTimePtr;

typedef struct _TelSmsGsmSubmitMessageType
{
	uint8_t						protocolId;
	uint8_t						messageClass;	// SMS Message Class
	Boolean						rejectDuplicateRequest;
	Boolean						replyPath;
} TelSmsGsmSubmitMessageType, *TelSmsGsmSubmitMessagePtr;

typedef struct _TelSmsSubmitMessageType
{
	TelSmsDateTimeType			validityPeriod;
	Boolean						networkDeliveryRequest;
	uint8_t						networkType;
	uint8_t						padding[2];
	union
	{
		TelSmsGsmSubmitMessageType	gsm;
	} networkParams;
} TelSmsSubmitMessageType, *TelSmsSubmitMessagePtr;

typedef struct _TelSmsGsmDeliverMessageType
{
	uint8_t						protocolId;
	uint8_t						messageClass;	// SMS Message Class
	Boolean						replyPath;
	uint8_t						padding;
} TelSmsGsmDeliverMessageType, TelSmsGsmDeliverMessagePtr;

typedef struct _TelSmsDeliverMessageType
{
	TelSmsDateTimeType			timeStamp;
	Boolean						otherToReceive;
	Boolean						reportDeliveryIndicator;
	uint8_t						networkType;
	uint8_t						padding;
	union
	{
		TelSmsGsmDeliverMessageType	gsm;
	} networkParams;
} TelSmsDeliverMessageType, *TelSmsDeliverMessagePtr;

typedef struct _TelSmsReportMessageType
{
	TelSmsDateTimeType			timeStamp;
	uint8_t						reportType;
	uint8_t						report;
	uint8_t						padding[2];
} TelSmsReportMessageType, *TelSmsReportMessagePtr;

typedef struct _TelSmsMultiPartInfoType
{
	uint16_t					bytesSent;
	uint16_t					current;
	uint16_t					count;
	uint16_t					id;
} TelSmsMultiPartInfoType, *TelSmsMultiPartInfoPtr;

typedef struct _TelSmsNbsExtensionType
{
	uint16_t					dest;
	uint16_t					source;
} TelSmsNbsExtensionType, *TelSmsNbsExtensionPtr;

typedef struct _TelSmsSpecialIndicationExtensionType
{
	uint8_t						type;				// Type of Special indication
	Boolean						active;				// Determines if indication is active or not
	Boolean						msgStore;			// Determines if the message shall be stored or not
	uint8_t						msgWaitingCount;	// Determines the number of messages of the type specified, if known otherwise zero

} TelSmsSpecialIndicationExtensionType, *TelSmsSpecialIndicationExtensionPtr;

typedef struct _TelSmsUserExtensionType
{
	uint8_t*					headerP;
	size_t						headerSize;
} TelSmsUserExtensionType, *TelSmsUserExtensionPtr;

typedef struct _TelSmsExtensionType
{
	uint8_t						type;
	uint8_t						padding[3];
	union
	{
		TelSmsNbsExtensionType					nbs;
		TelSmsSpecialIndicationExtensionType	ind;	/* Special SMS Message Indication extension */
		TelSmsUserExtensionType					user;

	} extension;
} TelSmsExtensionType, *TelSmsExtensionPtr;

typedef struct _TelSmsMessageType
{
	uint8_t*					dataP;
	uint32_t					messageId;
	size_t						dataSize;
	TelNumberType				address1;		/* Destination address for submitOriginating address for deliver & report */
	TelNumberType				address2;		/* Service center for GSM submit & deliverCallback Number for CDMA & TDMA submit & deliver */
	TelSmsMultiPartInfoType		multiPartInfo;
	TelSmsExtensionPtr			extensionP;
	size_t						extensionCount;
	uint16_t					apiVersion;
	uint16_t					phoneIndex;
	uint8_t						dataCodingScheme;
	uint8_t						messageType;
	uint8_t						status;
	uint8_t						padding;
	union
	{
		TelSmsSubmitMessageType		submit;
		TelSmsDeliverMessageType	deliver;
		TelSmsReportMessageType		report;
	} message;
} TelSmsMessageType, *TelSmsMessagePtr;

typedef struct _TelSmsMessagesType
{
	TelSmsMessagePtr			listP;
	size_t						count;
} TelSmsMessagesType, *TelSmsMessagesPtr;

typedef struct _TelSmsStoragesType
{
	uint16_t*					idP;
	size_t						count;
} TelSmsStoragesType, *TelSmsStoragesPtr;

typedef struct _TelSmsStorageType
{
	size_t						usedSlot;
	size_t						totalSlot;
	uint16_t					id;
	uint16_t					padding;
} TelSmsStorageType, *TelSmsStoragePtr;


// Speech call
typedef struct _TelSpcCallType
{
	char*						dialNameP;
	size_t						dialNameSize;
	TelNumberType				dialNumber;
	Boolean						multiparty;
	uint8_t						callId;
	uint8_t						direction;
	uint8_t						status;
	uint8_t						mode;
	uint8_t						padding[3];
} TelSpcCallType, *TelSpcCallPtr;

typedef struct _TelSpcCallsType
{
	TelSpcCallPtr				listP;
	size_t						count;
} TelSpcCallsType, *TelSpcCallsPtr;

typedef struct _TelSpcToneDurationRangeType
{
	uint16_t					min;
	uint16_t					max;
} TelSpcToneDurationRangeType, *TelSpcToneDurationRangePtr;


// Phonebook
typedef struct _TelPhbPhonebooksType
{
	uint16_t*					idP;				/* Phonebook identifier array */
	size_t						count;		 		/* Size of phonebook identifier array (in), number of phonebook identifier (out) */
} TelPhbPhonebooksType, *TelPhbPhonebooksPtr;

typedef struct _TelPhbPhonebookType
{
	size_t						usedSlot;
	size_t						totalSlot;
	size_t						fullNameMaxSize;			/* Maximum size for the full name */
	size_t						dialNumberMaxSize;			/* Maximum size for the dial number */
	uint16_t					id;							/* Phonebook identifier */
	uint16_t					firstIndex;			 		/* First index of the phonebook */
	uint16_t					lastIndex;			 		/* Last index of the phonebook */
	uint16_t					padding;
} TelPhbPhonebookType, *TelPhbPhonebookPtr;

typedef struct _TelPhbEntryType
{
	char*  						fullNameP;
	size_t						fullNameSize; 				/* name len including '\0' (out), name size (in) */
	TelNumberType				dialNumber;		
	uint16_t 					phoneIndex;  				/* entry's index in the phonebook, zero based */
	uint16_t 					padding;
} TelPhbEntryType, *TelPhbEntryPtr;

typedef struct _TelPhbEntriesType
{
	TelPhbEntryPtr				entryP;
	size_t						entryCount;
	uint16_t 					firstIndex;
	uint16_t 					lastIndex;
} TelPhbEntriesType, *TelPhbEntriesPtr;


// Information
typedef struct _TelInfIdentificationType
{
	char*						valueP;					// returned information string
	size_t						size;					// value len including '\0' (out), value size (in)
	uint8_t						type;
	uint8_t						padding[3];
} TelInfIdentificationType, *TelInfIdentificationPtr;

typedef struct _TelInfCallsDurationType
{
	uint32_t					lastCall;
	uint32_t					receivedCalls;
	uint32_t					dialedCalls;
} TelInfCallsDurationType, *TelInfCallsDurationPtr;

typedef struct _TelInfCallType
{
	char*						fullNameP;
	TelNumberType				dialNumber;
	size_t						fullNameSize;
	struct tm					dateTime;
} TelInfCallType, *TelInfCallPtr;

typedef struct _TelInfCallsListType
{
	TelInfCallPtr				listP;
	size_t						count;
	uint8_t						type;
	uint8_t						padding[3];
} TelInfCallsListType, *TelInfCallsListPtr;

// OEM
typedef struct _TelOemCallType
{
	uint32_t					oemId;			// unique ID of OEM function set
	void*						paramP;			// parameters block
	size_t						paramSize;		// Parameters block size
	uint8_t						funcId;			// function ID
	uint8_t						padding[3];
} TelOemCallType, *TelOemCallPtr;

// GPRS
typedef struct _TelGprsContextType
{
	uint8_t	contextID;			// Context ID
	uint8_t	pdpType;			// PDP Type: IP, PPP, OSPIH
	uint8_t	dataCompression;	// Data compression or none
	uint8_t	headerCompression;	// Header compression or none
	char*	accessPointNameP;	// if accessPointNameSize == 0, then default APN requested from network
	size_t	accessPointNameSize;
	char*	pdpAddressP;			// if PdpAddressSize == 0, then PDP Address requested from network
	size_t	pdpAddressSize;
	char*	OSPIHHostP; 			// only required if pdpType OSPIH is chosen: Internet Host
	size_t	OSPIHHostSize; 		// only required if pdpType OSPIH is chosen: Internet Host Size
	uint16_t	OSPIHPort; 			// only required if pdpType OSPIH is chosen: TCP or UDP port on Internet Host
	uint8_t	OSPIHProtocol;		// only required if pdpType OSPIH is chosen: Protocol used over IP, TCP or UDP
	uint8_t	padding;

} TelGprsContextType, *TelGprsContextPtr;

typedef struct _TelGprsDefinedCidsType
{
	size_t	cidCount;          // Number of element in the array, output number if cids count
	uint8_t*	cidsP ;           // output : array of cids

} TelGprsDefinedCidsType, *TelGprsDefinedCidsPtr;

typedef struct _TelGprsPdpActivationType
{
	uint8_t	contextID;		// context ID
	uint8_t	state;			// state
	uint8_t	padding[2];

} TelGprsPdpActivationType, *TelGprsPdpActivationPtr;

typedef struct _TelGprsPdpAddressType
{
	uint8_t	contextID;		// context ID
	uint8_t	padding[3];
	char*	pdpAddressP;		// PDP address
	size_t	pdpAddressSize;	// size of PDP address

} TelGprsPdpAddressType, *TelGprsPdpAddressPtr;

typedef struct _TelGprsNwkRegistrationType
{
	uint8_t	registrationType;		// Type of registration: Disable, NwkEnable, CellEnable
	uint8_t	registrationStatus;		// Registration status
	uint8_t	cellSupportingStatus;	// Indicates if cell is supporting GPRS or not (0 - GPRS not supported, 1 - GPRS supported, 255 - unknown)
	uint8_t	padding;
	uint16_t	locationAreaCode;		// Location Information
	uint16_t	cellId;					// Cell ID

} TelGprsNwkRegistrationType, *TelGprsNwkRegistrationPtr;

typedef struct _TelGprsQosType
{
	uint8_t	contextID;			// Context ID
	uint8_t	precedence;			// Precedence
	uint8_t	delay;				// Delay Class
	uint8_t	reliability;		// Reliability Class
	uint8_t	peak;				// Peak Troughput Class
	uint8_t	mean;				// Mean Troughput Class
	uint8_t	padding[2];

} TelGprsQosType, *TelGprsQosPtr;

typedef struct _TelGprsEventReportingType
{
	uint8_t	mode;		// Event reporting mode
	uint8_t	buffer;		// optional: buffer of unsolicited result codes
	uint8_t	padding[2];

} TelGprsEventReportingType, *TelGprsEventReportingPtr;

typedef struct _TelGprsDataCounterType
{
	uint8_t	contextID;	// input: Context ID
	uint8_t	padding[3];
	uint32_t	ulBytes;	// output: Represents the uplink data amount in bytes that has been transmitted by MS to network
	uint32_t	dlBytes;	// output: Represents the downlink data amount in bytes that has been received by MS from network
	uint32_t	ulPackets;	// output: Represents the uplink data amount in packets (NPDUs) that has been transmitted by MS to network
	uint32_t	dlPackets;	// output: Represents the downlink data amount in packets (NPDUs) that has been received by MS from network

} TelGprsDataCounterType, *TelGprsDataCounterPtr;

// Data
typedef struct _TelDtcCsdConnectionType
{
	uint8_t			speed;
	uint8_t			service;
	uint8_t			connection;
	uint8_t			padding;
	TelNumberType	dialNumber;	
} TelDtcCsdConnectionType, *TelDtcCsdConnectionPtr;

typedef struct _TelDtcGprsConnectionPtr
{
	TelGprsContextType	context;
	TelGprsQosType 		qosMinimum;
	TelGprsQosType 		qosRequested;
} TelDtcGprsConnectionType, *TelDtcGprsConnectionPtr;

typedef struct _TelDtcConnectionInfoType
{
	uint8_t						type;
	uint8_t						padding[3];
	union
	{
		TelDtcCsdConnectionType	gsmCsd;
		TelDtcGprsConnectionType	gprs;
	} connection;
} TelDtcConnectionInfoType, *TelDtcConnectionInfoPtr;

// Card Application Toolkit
typedef struct _TelCatConfigType
{
	uint8_t*		profileP;				// Terminal Profile standard parameters
	uint32_t		profileSize;			// Profile data size, in bytes
	char			lanCode[2]; 			// ISO 639 language code
	uint8_t			mode;					// Result code presentation enable/disable
	uint8_t			padding;
} TelCatConfigType, *TelCatConfigPtr;

typedef struct _TelCardFileType
{
	uint16_t*	pathP;			// Absolute path of the file to read in the SIM.
								// Example: { 0x3F00, 0x7F20, 0x6F21 }
								// Consists of file identifiers from the Master File
								// to the Elementary File to be accessed
	uint8_t*	bufP;			// Buffer to be filled in with requested file body bytes.
	size_t		bufSize;		// Buffer size in bytes
	size_t		byteCount;		// Amount of applicable bytes in ioBufP

	uint16_t	partOffset;		// Offset of requested file body part
	uint16_t	partSize;		// Size of requested file body part

	uint16_t	fileSize;		// EF size
	uint8_t		fileStruct;		// EF structure (transparent, linear fixed or cyclic)
	uint8_t		mode;			// Access mode (see EF access mode)

	uint8_t		pathCount;		// File identifiers count in iPathP
	uint8_t		recId;			// Identifier of the record to be read. Range 1..254
	uint8_t		recSize;		// Size of a record, in bytes.
								// 0 if the file is not a Linear Fixed or a Cyclic EF
	uint8_t		pad;

} TelCardFileType, *TelCardFilePtr;

////////////////////////////
typedef struct _TelCatCmdParamsType
{
	MemPtr		cmdParamP;				// Structure associated to the command (according to iCmdId)
	size_t		cmdParamSize;			// Size of the specific command parameter buffer
										// Common parameter for all (almost) commands
	char*		textP;					// Text to display

	uint8_t		textSize;						
	uint8_t		iconId;					// Icon identifier
	uint8_t		cmdId;					// See kTelCatCmdXXX for more details
	Boolean		explicitIcon; 			// Icon is explicit

	Boolean		noResponse;				// true iff the command does not need a response
	uint8_t		other1;					// other parameters command dependent
	uint16_t	other2;
} TelCatCmdParamsType;
////////////////////////////
typedef struct _TelCatCmdResponseType
{
	char*		respP;					// Associated text response
	uint32_t	other;					// Specific command identifier parameter
	size_t		respSize;				// Associated text response size in bytes
	uint8_t		cmdId;					// Command Identifier
	uint8_t		respType;				// Expected response type, see kTelCatRespType<type>
	uint8_t		resCode;				// Result codes applicable to command iCmdId, see kTelCatRes<code>
	uint8_t		addInfo;				// Additional information code
										// -> Yes/No response or Item identifier for instance
} TelCatCmdResponseType;
////////////////////////////
typedef struct _TelCatMenuSelectionType
{
	uint8_t		evtCode;				// Menu Selection event code, see kTelCatMenuSel<code>
	uint8_t		appId;					// Associated application identifier
	uint16_t	pad;
} TelCatMenuSelectionType;
////////////////////////////
typedef struct _TelCatEventToCardType
{
	uint8_t		evtCode;					// Event Download code, see kTelCatEvent<type>
	char		lanCode[2];					// ISO 639 language code
	uint8_t		browserTerminationCause;	// Browser Termination cause code, see kTelCatBrowserTermination<cause>
} TelCatEventToCardType;
////////////////////////////
// Refresh
typedef struct _TelCatRefreshType
{
	uint16_t*	filePathP;			// Concatenated absolute paths of the modified EF
										// or NULL if no specific file has been mentioned.
	uint8_t		fileIdCount;		// File identifiers count in filePathP
	uint8_t		opCode;				// Operation code, see kTelCatRefresh<type> code
	uint16_t	pad;
}TelCatRefreshType;
////////////////////////////
// SetUpEventList
typedef struct _TelCatSetUpEventListType
{
	uint8_t*	eventP;				// Events to be monitored, see kTelCatEvent<type>
	uint8_t		eventCount;			// Events number in eventP or 0 to stop monitoring
	uint8_t		pad1;
	uint16_t	pad2;
} TelCatSetUpEventListType;
////////////////////////////
// SetUpCall
typedef struct _TelCatSetUpCallType
{
	uint8_t*	bearerCapP;			// Bearer capability configuration params (GSM 04.08 5.3.0 section 10.5.4.5)
	char*		numberP;			// Dialing number
	char*		userConfTextP;		// Text to display. NULL if not provided
	char*		callEstaTextP;			// Text to display, NULL if not provided
	size_t		userConfTextSize;	// Text buffer size in bytes
	size_t		numberSize;			// Number buffer size in bytes
	size_t		callEstaTextSize;		// Call establishment text buffer size in bytes
	uint8_t		userConfIconId;		// Icon id. 0 if no icon
	Boolean		userConfExplicitIcon;	// Icon is explicit
	Boolean		autoRedial;			// Automatic redial requested by SIM
	uint8_t		bearerCapSize;		// Bearer capability size, in bytes
	uint8_t		condition;			// Set up call conditions, see kTelCatCall<condition>
	uint8_t		callEstaIconId;			// Icon id. 0 if no icon
	Boolean		callEstaExplicitIcon;	// Icon is explicit
	uint8_t		pad;
} TelCatSetUpCallType;
////////////////////////////
// LaunchBrowser 
typedef struct _TelCatLaunchBrowserType
{
	char*		urlP;				// The URL
	size_t		urlSize;
	char*		gatewayP;			// Gateway name, Proxy identity to be used
	size_t		gatewaySize;
	uint16_t*	filePathP;			// Concatenated absolute paths of provisioning EF
										// or NULL if no specific file has been mentioned.
	uint8_t*	prefBearersP;		// Prioritized list of kTelCatBearer<bearer> bearer codes
	uint8_t		fileIdCount;		// File identifiers count in filePathP
	uint8_t		prefBearerCount;	// Number of bearer codes in prefBearersP.
	uint8_t		condition;			// Launch browser conditions, see kTelCatBrowser<condition>
	uint8_t		browserId;			// Browser Identity
} TelCatLaunchBrowserType;
////////////////////////////
// PlayTone
typedef struct _TelCatPlayToneType
{
	uint32_t		sndDuration;		// Sound duration in ms. 0 for default or 100..15300000
	uint8_t			sndCode;			// One of the kTelCatSound<sound> codes
	uint8_t			pad1;
	uint16_t		pad2;
} TelCatPlayToneType;
////////////////////////////
// DisplayText
typedef struct _TelCatDisplayTextType
{
	Boolean		priority;			// Priority level
	Boolean		clearAfterDelay;	// Wait for user's action
	Boolean		immediateResponse;	// Send a response to the SIM ASAP
} TelCatDisplayTextType;
////////////////////////////
// GetInkey
typedef struct _TelCatGetInkeyType
{
	Boolean		helpInfo;			// Help information is provided by the SIM
	uint8_t		respType;			// Expected response type. See kTelCatRespType<type>
	uint16_t	pad;
} TelCatGetInkeyType;
////////////////////////////
// GetInput
typedef struct _TelCatGetInputType
{
	char*		defRespP;			// Default response text to propose
	size_t		defRespSize;		// Default response text size in bytes
	Boolean		hideUserInput;		// Mask entered data
	Boolean		helpInfo;			// Help information is provided by the SIM
	uint8_t		minRespLength;		// Minimum response length, in characters
	uint8_t		maxRespLength;		// Maximum response length, in characters
	uint8_t		respType;			// Expected response type, see kTelCatRespType<type>
	uint8_t		pad1;
	uint16_t	pad2;
} TelCatGetInputType;
////////////////////////////
// SelectItem - SetUpMenu
typedef struct _TelCatItemType
{
	char*		nameP;				// Item name
	size_t		nameSize;			// Item name size in bytes
	uint8_t		id;					// Item identifier
	uint8_t		iconId;				// Icon Identifier
	Boolean		expIcon;			// Icon is explicit
	uint8_t		nextActionInd;		// Identifier of the next command for this item
} TelCatItemType;
//
typedef struct _TelCatItemListType
{
	TelCatItemType* itemsP;				// The items
	uint8_t		itemCount;			// Number of items in itemsP
	Boolean		softKey;			// Item can be selected by tapping on its icon
	Boolean		helpInfo;			// Help information is provided by the SIM
	uint8_t		defItemId;			// Identifier of the item that should be pre-selected.
} TelCatItemListType;
////////////////////////////
// Send Short Message
typedef struct _TelCatSendShortMessageType
{
	char*		addressP;			// RP_Destination_Address (optional)
	uint8_t*	TPDUP;				// SMS TPDU
	uint8_t		TPDUSize;
	uint8_t		addressSize;
	Boolean		packingRequired;
	uint8_t		pad;
} TelCatSendShortMessageType;
////////////////////////////
// Commands with buffer (send commands, runAT)
typedef struct _TelCatBufferType		// used in: SendData, SendDTMF, Send USSD, Send SS
{
	uint8_t*	bufferP;				// the data buffer
	uint8_t		bufferSize;				// the data buffer size
	uint8_t		other;					// other parameter specific to the command (if any)
	uint16_t	pad;
} TelCatBufferType;
////////////////////////////
// OpenChannel
typedef struct _TelCatOpenChanType
{
	char*		addressP;
	char*		subAddressP;
	char*		otherAddressP;
	char*		destinationAddressP;
	char*		loginP;
	char*		passwordP;
	uint8_t*	bearerParamsP;
	char*		accessPointP;
	uint32_t	duration1;				// duration1 in ms
	uint32_t	duration2;				// duration2 in ms
	uint16_t	bufferSize;
	uint16_t	transportPort;
	Boolean		onDemand;				// Is link established immediately
	uint8_t		bearerCode; 			// Bearer code, see kTelCatBearer<bearer>
	uint8_t		otherAddressType;		// see kTelCatAddress<type>
	uint8_t		destinationAddressType; // see kTelCatAddress<type>
	uint8_t		transportType;			// see kTelCatTransport<type>
	uint8_t		addressSize;
	uint8_t		subAddressSize;
	uint8_t		otherAddressSize;
	uint8_t		bearerParamsSize;
	uint8_t		loginSize;
	uint8_t		passwordSize;
	uint8_t		destinationAddressSize;
	uint8_t		accessPointSize;
	uint8_t		pad1;
	uint16_t	pad2;
} TelCatOpenChanType;

// Mux
typedef struct _TelMuxInfoType
{
	uint32_t		type;
	uint32_t		creator;
	uint32_t		nameSize;				// MUX Device name buffer size
	uint8_t*		nameP;					// MUX Device name
} TelMuxInfoType, *TelMuxInfoPtr;

typedef struct _TelMuxChanType
{
	uint32_t*		chanIdP;				// Channel id
	uint8_t			type;					// Channel type
	uint8_t			pad[3];

} TelMuxChanType, *TelMuxChanPtr;


#endif /* TelephonyLibTypes_h */
