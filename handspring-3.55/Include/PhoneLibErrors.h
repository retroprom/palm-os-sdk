/**
 * \file PhoneLibErrors.h
 *
 * Handspring Phone Library
 *
 * Error code returned by the Phone Library
 *
 * Copyright (c) 2002 Handspring Inc. & Option International
 * All Rights Reserved
 *
 * $Id: $
 *
 ***************************************************************/

#ifndef _PHONELIBERRORS_H_
#define _PHONELIBERRORS_H_

// GSM Library result codes
typedef enum GSMErrorCode
 {
	gsmErrorClass = 0x4000,
	gsmErrParam,                    // 0x4001
	gsmErrUnknownError,             // 0x4002
	gsmErrNoResponse,               // 0x4003
	gsmErrNotOpen,                  // 0x4004
	gsmErrStillOpen,                // 0x4005
	gsmErrMemory,                   // 0x4006
	gsmErrUnknownID,                // 0x4007
	gsmErrNoPower,                  // 0x4008
	gsmErrNoNetwork,                // 0x4009
	gsmErrNoConnection,             // 0x400a
	gsmErrNotAllowed,               // 0x400b
	gsmErrIllegalFacility,          // 0x400c
	gsmErrIllegalCondition,         // 0x400d
	gsmErrIllegalStatus,            // 0x400e
	gsmErrIllegalIndex,             // 0x400f
	gsmErrIllegalChars,             // 0x4010
	gsmErrIllegalMsg,               // 0x4011
	gsmErrIllegalType,              // 0x4012
	gsmErrIllegalNumber,            // 0x4013
	gsmErrTimeout,                  // 0x4014
	gsmErrUnknownApp,               // 0x4015
	gsmErrUnknownNumber,            // 0x4016
	gsmErrBufferTooSmall,           // 0x4017
	gsmErrPasswordRequired,         // 0x4018
	gsmErrResponsePending,          // 0x4019
	gsmErrCancelled,                // 0x401a
	gsmErrNoRecipient,              // 0x401b
		// Errors defined in the GSM recommendations
	gsmErrPhoneFailure,             // 0x401c
	gsmErrPhoneNotConnected,        // 0x401d
	gsmErrPhoneAdaptorLinkReserved, // 0x401e
	gsmErrNotSupported,             // 0x401f
	gsmErrPhPINRequired,            // 0x4020
	gsmErrPhFPINRequired,           // 0x4021
	gsmErrPhFPUKRequired,           // 0x4022
	gsmErrNoSIM,                    // 0x4023
	gsmErrPINRequired,              // 0x4024
	gsmErrPUKRequired,              // 0x4025
	gsmErrSIMFailure,               // 0x4026
	gsmErrSIMBusy,                  // 0x4027
	gsmErrSIMWrong,                 // 0x4028
	gsmErrIncorrectPassword,        // 0x4029
	gsmErrPIN2Required,             // 0x402a
	gsmErrPUK2Required,             // 0x402b
	gsmErrMemoryFull,               // 0x402c
	gsmErrInvalidMemIndex,          // 0x402d
	gsmErrNotFound,                 // 0x402e
	gsmErrMemFailure,               // 0x402f
	gsmErrStringTooLong,            // 0x4030
	gsmErrInvalidTextChars,         // 0x4031
	gsmErrDialStringTooLong,        // 0x4032
	gsmErrInvalidDialChars,         // 0x4033
	gsmErrNoNetworkService,         // 0x4034
	gsmErrNetworkTimeout,           // 0x4035
	gsmErrNetworkNotAllowed,        // 0x4036
	gsmErrNetPINRequired,           // 0x4037
	gsmErrNetPUKRequired,           // 0x4038
	gsmErrNetSubPINRequired,        // 0x4039
	gsmErrNetSubPUKRequired,        // 0x403a
	gsmErrSPPINRequired,            // 0x403b
	gsmErrSPPUKRequired,            // 0x403c
	gsmErrCorpPINRequired,          // 0x403d
	gsmErrCorpPUKRequired,          // 0x403e
	gsmErrIllegalMS,                // 0x403f
	gsmErrIllegalME,                // 0x4040
	gsmErrGPRSNotAllowed,           // 0x4041
	gsmErrPLMNNotAllowed,           // 0x4042
	gsmErrLocAreaNotAllowed,        // 0x4043
	gsmErrRoamingNotAllowed,        // 0x4044
	gsmErrOptionNotSupported,       // 0x4045
	gsmErrReqOptionNotSubscribed,   // 0x4046
	gsmErrOptionTempOutOfOrder,     // 0x4047
	gsmErrUnspecifiedGPSRError,     // 0x4048
	gsmErrAuthenticationFailure,    // 0x4049
	gsmErrInvalidMobileClass,       // 0x404a
	gsmErrUnassignedNumber,         // 0x404b
	gsmErrOperDeterminedBarring,    // 0x404c
	gsmErrCallBarred,               // 0x404d
	gsmErrSMSXferRejected,          // 0x404e
	gsmErrDestOutOfService,         // 0x404f
	gsmErrUnidentifedSubscriber,    // 0x4050
	gsmErrFacRejected,              // 0x4051
	gsmErrUnknownSubscriber,        // 0x4052
	gsmErrNetworkOutOfOrder,        // 0x4053
	gsmErrTemporaryFailure,         // 0x4054
	gsmErrCongestion,               // 0x4055
	gsmErrResourcesUnavailable,     // 0x4056
	gsmErrReqFacNotSubscribed,      // 0x4057
	gsmErrReqFacNotImplemented,     // 0x4058
	gsmErrInvalidSMSReference,      // 0x4059
	gsmErrInvalidMsg,               // 0x405a
	gsmErrInvalidMandInfo,          // 0x405b
	gsmErrMsgTypeNonExistent,       // 0x405c
	gsmErrMsgNoCompatible,          // 0x405d
	gsmErrInfoElemNonExistent,      // 0x405e
	gsmErrProtocolError,            // 0x405f
	gsmErrInterworking,             // 0x4060
	gsmErrTelematicIWNotSupported,  // 0x4061
	gsmErrSMType0NotSupported,      // 0x4062
	gsmErrCannotReplaceMsg,         // 0x4063
	gsmErrUnspecifiedTPPIDError,    // 0x4064
	gsmErrAlphabetNotSupported,     // 0x4065
	gsmErrMsgClassNotSupported,     // 0x4066
	gsmErrUnspecifiedTPDCSError,    // 0x4067
	gsmErrCmdCannotBeActioned,      // 0x4068
	gsmErrCmdUnsupported,           // 0x4069
	gsmErrUnspecifiedTPCmdError,    // 0x406a
	gsmErrTPDUNotSupported,         // 0x406b
	gsmErrSCBusy,                   // 0x406c
	gsmErrNoSCSubscription,         // 0x406d
	gsmErrSCSystemFailure,          // 0x406e
	gsmErrInvalidSMEAddr,           // 0x406f
	gsmErrDestSMEBarred,            // 0x4070
	gsmErrSMRejectedDuplicate,      // 0x4071
	gsmErrTPVPFNotSupported,        // 0x4072
	gsmErrTPVPNotSupported,         // 0x4073
	gsmErrSMSStorageFull,           // 0x4074
	gsmErrNoSMSStorage,             // 0x4075
	gsmErrErrorInMS,                // 0x4076
	gsmErrSIMApplToolkitBusy,       // 0x4077
	gsmErrMEFailure,                // 0x4078
	gsmErrSMSServReserved,          // 0x4079
	gsmErrInvalidParameter,         // 0x407a
	gsmErrFiller,                   // 0x407b
	gsmErrFiller2,                  // 0x407c
	gsmErrFiller3,                  // 0x407d
	gsmErrMemoryFailure,            // 0x407e
	gsmErrSCAddrUnknown,            // 0x407f
	gsmErrNoCNMAAckExpected,        // 0x4080
		// Errors returned by the firmware (NO CARRIER)
	gsmErrFDNMismatch,              // 0x4081
	gsmErrEmergencyCallsOnly,       // 0x4082
	gsmErrACMLimitExceeded,         // 0x4083
	gsmErrHoldError,                // 0x4084
	gsmErrNumberBlacklisted,        // 0x4085
	gsmErrLidClosed,                // 0x4086
		// Errors for SIM Application Toolkit
	gsmErrSATUnavailable,           // 0x4087
	gsmErrSATInactive,              // 0x4088
	gsmErrUNUSED,              // 0x4089
		// Library loading errors
	gsmErrRadioNotAvailable,        // 0x408a
		// Used internally
	gsmErrReserved_408b,
	gsmErrReserved_408c,
	gsmErrReserved_408d,
		// Firmware boot synchonization
	gsmErrFirmwareBootNotInprogress,// 0x408e
	gsmErrFirmwareBootInprogress,	// 0x408f
		// These error codes map directly to Wismo error
		// codes, but maybe could be used by other radios?
	gsmErrMMFailed,					// 0x4090
	gsmErrLowerLayer,				// 0x4091
	gsmErrCPError,					// 0x4092
	gsmErrCommandInProgress,		// 0x4093
	gsmErrSATNotSupported,			// 0x4094
	gsmErrSATNoInd,					// 0x4095
	gsmErrNeedResetModule,			// 0x4096
	gsmErrCOPSAbort,				// 0x4097


	// CSD CEER Errors
	gsmErrCSDUnassignedNumber,
	gsmErrCSDNoRouteToDestination,
	gsmErrCSDChannelUnacceptable,
	gsmErrCSDOperatorBarring,
	gsmErrCSDNumberBusy,
	gsmErrCSDNoUserResponse,
	gsmErrCSDNoAnswer,
	gsmErrCSDCallRejected,
	gsmErrCSDNumberChanged,
	gsmErrCSDDestinationOutOfOrder,
	gsmErrCSDInvalidNumberFormat,
	gsmErrCSDFacilityRejected,
	gsmErrCSDNoCircuitAvailable,
	gsmErrCSDNetworkOutOfOrder,
	gsmErrCSDTempFailure,
	gsmErrCSDSwitchingCongestion,
	gsmErrCSDAccessInfoDiscarded,
	gsmErrCSDReqCircuitNotAvailable,
	gsmErrCSDResourceNotAvailable,
	gsmErrCSDQOSNotAvailable,
	gsmErrCSDFacilityNotSubscribed,
	gsmErrCSDIncomingCallsBarredCUG,
	gsmErrCSDBearerNotCapable,
	gsmErrCSDBearerNotAvailable,
	gsmErrCSDServiceNotAvailble,
	gsmErrCSDBearerNotImplemented,
	gsmErrCSDACMOutOfRange,
	gsmErrCSDFacilityNotImplemented,
	gsmErrCSDRestrictedBearer,
	gsmErrCSDServiceNotImplented,
	gsmErrCSDInvalidTransID,
	gsmErrCSDUserNotMemberCUG,
	gsmErrCSDIncompatibleDestination,
	gsmErrCSDInvalidTransitNetwork,
	gsmErrCSDSemanticallyIncorrectMessage,
	gsmErrCSDInvalidMandatoryInfo,
	gsmErrCSDMessageTypeNotImplemented,
	gsmErrCSDMessageTypeNotCompatible,
	gsmErrCSDInfoNotImplemented,
	gsmErrCSDIEError,
	gsmErrCSDMessageNotCompatible,
	gsmErrCSDRecoveryOnTimerExpiry,
	gsmErrCSDProtocolError,
	// GPRS CEER Errors
	gsmErrGPRSRoamingNotAllowed,
	gsmErrGPRSNetworkRequestDetach,
	gsmErrGPRSNoService,            
	gsmErrGPRSNoAccess,             
	gsmErrGPRSServiceRefused,       
	gsmErrGPRSNetworkRequestPDPDeactivate,
	gsmErrGPRSPDPDactivateLCCLinkActivation,
	gsmErrGPRSPDPDeactivateNwkRactivate,
	gsmErrGPRSPDPDactivateGMMAbort,
	gsmErrGPRSPDPDeactivateSNDCPFailure,
	gsmErrGPRSPDPActivateFailGMMError,
	gsmErrGPRSPDPActivateFailNetReject,
	gsmErrGPRSPDPActivateFailNoNSAPI,
	gsmErrGPRSPDPActivateFailSMRefuse,
	// MORE CSD Errors
	gsmErrCSDFDNError,
	gsmErrCSDCallOperationNotAllowed,
	gsmErrCSDCallBarringOutgoing,
	gsmErrCSDCallBarringIncoming,
	gsmErrCSDCallImpossible,
	gsmErrCSDLowerLayerFailure,
	// More GPRS Errors
	gsmErrGPRSAPNMissing,
	// More Errors
	gsmErrNoCarrier,
	gsmErrSMSFDNError,			//SMS Fixed Dialing error (CMS ERROR: 531)
	gsmErrNullParam,			// provided parameter is unexpectedly null
	gsmErrBadServiceCode,		// intermediate response has unexpected service code
	gsmErrBadATResult,			// AT response from radio is missing or invalid
	gsmErrBadATCmd,				// AT cmd to radio is missing or invalid


	
	/***********************************************
	* WARNING:  If adding a new error code, you 
	* must update Errors.h, GSMLibraryErrors.h, 
	* and PhoneLibErrors.h.  
	************************************************/
} GSMErrorCode;


#endif  // _PHONELIBERRORS_H_
