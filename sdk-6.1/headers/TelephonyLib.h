/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: TelephonyLib.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             	This is the ARM API header file for the Palm OS Telephony Manager
 *
 *****************************************************************************/

#ifndef TelephonyLib_h
#define TelephonyLib_h


/******************************************************************************
 *	Includes
 *****************************************************************************/

#include <PalmTypes.h>
#include <Event.h>
#include <SystemMgr.h>
#include <sys/ioccom.h>

#include "TelephonyLibTypes.h"


/******************************************************************************
 *	Defines
 *****************************************************************************/

// SysMakeROMVersion(major, minor, fix, stage, buildNum)
#define kTelMgrVersionMajor								2
#define kTelMgrVersionMinor								0
#define kTelMgrVersionFix								0
#define kTelMgrVersionBuild								0
#define kTelMgrStage									sysROMStageBeta

// Telephony manager shared lib version
#define kTelMgrVersion									sysMakeROMVersion(kTelMgrVersionMajor, kTelMgrVersionMinor, kTelMgrVersionFix, kTelMgrStage, kTelMgrVersionBuild)

// TelMgr shared lib internal name
#define kTelMgrLibName									"Telephony Library"

// TelMgr type and creator
#define kTelMgrDatabaseCreator							sysFileCTelMgrLib
#define kTelMgrDatabaseType								sysFileTLibrary

#define kTelTelephonyNotification						'tmgr'					// Telephony notification
#define kTelTelephonyEvent								telAsyncReplyEvent		// Telephony event

#define telNotifyErrorEvent								'terr'
#define telNotifyEnterCodeEvent							'tpin'

// Telephony notification IDs
#define	kTelSmsLaunchCmdIncomingMessage					0						// An incoming SMS - 'data' is the storage id and 'data2' the message id
#define	kTelSpcLaunchCmdCallConnect						1						// The line is opened - 'data' is the call id and 'data2' a bit mask regrouping mode, direction and multiparty info
#define	kTelSpcLaunchCmdCallHeld						2						// The call has been held - 'data' is the call id and 'data2' a bit mask regrouping mode, direction and multiparty info
#define	kTelSpcLaunchCmdCallDialing						3						// The call is dialing - 'data' is the call id and 'data2' a bit mask regrouping mode, direction and multiparty info
#define	kTelSpcLaunchCmdCallAlerting					4						// The call is alerting - 'data' is the call id and 'data2' a bit mask regrouping mode, direction and multiparty info
#define	kTelSpcLaunchCmdCallIncoming					5						// An incoming voice call - 'data' is the call id and 'data2' a bit mask regrouping mode, direction and multiparty info
#define	kTelSpcLaunchCmdCallWaiting						6						// A waiting voice call - 'data' is the call id and 'data2' a bit mask regrouping mode, direction and multiparty info
#define	kTelSpcLaunchCmdCallReleased					7						// The call has been released - 'data' is the call id and 'data2' the call duration
#define	kTelSpcLaunchCmdCallerIdAvailable				8						// The caller id is available
#define	kTelNwkLaunchCmdSignalLevelChange				9						// The network signal level had changed - 'data' is the new signal level
#define	kTelNwkLaunchCmdUssdAnswer						10						// USSD answer - 'data' is the result code of the USSD sequence and 'data2' the data size if any are availble
#define	kTelNwkLaunchCmdNetworkStatusChange				11						// Network status had changed - 'data' is the new network status
#define	kTelPowLaunchCmdBatteryChargeLevelChange		12						// The battery charge level had changed - 'data' is the new battery charge level
#define	kTelPowLaunchCmdBatteryConnectionStatusChange	13						// The battery connection status had changed - 'data' is the new battery connection status
#define	kTelPowLaunchCmdConnectionOn					14						// The connection is on - 'data' is the authentication status
#define	kTelPowLaunchCmdConnectionOff					15						// The connection is off
#define	kTelPowLaunchCmdPhonebookReady					16						// The phonebook is ready
#define	kTelPowLaunchCmdPhonebookNotReady				17						// The phonebook is not ready
#define	kTelPowLaunchCmdSmsReady						18						// The SMS storage is ready
#define	kTelPowLaunchCmdSmsNotReady						19						// The SMS storage is not ready
#define	kTelStyLaunchCmdAuthenticated					20						// Authentication successfull
#define	kTelStyLaunchCmdAuthenticationCanceled			21						// Authentication canceled by user
#define	kTelStyLaunchCmdPhoneProfileAvailable			22						// A least one Phone profile is available
#define	kTelStyLaunchCmdNoPhoneProfileAvailable			23						// There is no Phone profile available
#define kTelGprsLaunchCmdEventReporting					24						// GPRS Event Reporting
#define kTelGprsLaunchCmdNwkRegistration				25						// GPRS Network Registration Event
//#define	kTelGprsLaunchCmdSessionStarted					26						// GPRS session has started
//#define	kTelGprsLaunchCmdSessionClosed					27 						// GPRS session was closed
#define	kTelGprsLaunchCmdSessionBytesExchanged			28 
#define kTelCatLaunchCmdNoApps							29						// CAT No card application
#define kTelCatLaunchCmdExecCmd							30						// CAT command running
#define kTelCatLaunchCmdEndSession						31						// CAT Session ended
#define kTelMuxLaunchCmdModeStatus						32
#define kTelMuxLaunchCmdChanStatus						33
#define	kTelDtcLaunchCmdStarted							34						// A data call session is started - 'data1' is the type of data connection - 'data2' depend on the connection type
#define	kTelDtcLaunchCmdClosed							35						// A data call session is closed

// Notification priorities
#define kTelCallNotificationPriority					0 						// Higher priority
#define kTelSmsNotificationPriority						1  
#define	kTelCallerNumberNotificationPriority			2
#define	kTelGprsNotificationPriority					3
#define	kTelStkNotificationPriority						3
#define	kTelOtherNotificationPriority					4

// Bit mask for kTelSpcLaunchCmdCallXXX notifications data2 value
#define	kTelNotificationCallTypeMask					0x0000000F				// Call type mask
#define	kTelNotificationCallDirectionMask				0x00000010				// Call direction mask
#define	kTelNotificationCallMultipartyMask				0x00000020				// Call multiparty mask

// Error codes
#define telErrUnknown									(telErrorClass | 0x01)	// Unknown Tel internal error
#define telErrMemAllocation								(telErrorClass | 0x02)	// Memory allocation error
#define telErrResultTimeOut								(telErrorClass | 0x03)	// Time-out was reached
#define telErrResultUserCancel							(telErrorClass | 0x04)	// User cancelled action
#define telErrResultBusyResource						(telErrorClass | 0x05)	// Resource is busy
#define telErrSecurity 									(telErrorClass | 0x06)	// Access to ME has not been granted
#define telErrBufferSize							    (telErrorClass | 0x07)	// Buffer used to retrieve data is too small
#define telErrFeatureNotSupported					 	(telErrorClass | 0x08)	// The feature is not supported by phone/network
#define telErrPhoneComm									(telErrorClass | 0x09)	// The communication link with the phone is down
#define telErrPhoneReply								(telErrorClass | 0x0A)	// The phone reply syntax is incorrect, check the phone driver!
#define telErrCommandFailed								(telErrorClass | 0x0B)	// The phone couldn't achieve the associated command, check the phone driver!
#define telErrSpcLineIsBusy								(telErrorClass | 0x0C)	// The speech call line is busy
#define telErrNoSIMInserted								(telErrorClass | 0x0D)	// No SIM inserted
#define telErrSIMFailure								(telErrorClass | 0x0E)	// The SIM is not working properly
#define telErrSIMBusy									(telErrorClass | 0x0F)	// The SIM couldn't reply
#define telErrSIMWrong									(telErrorClass | 0x10)	// The SIM is not accepted by the phone
#define telErrPassword									(telErrorClass | 0x11)	// Incorrect password
#define telErrPhoneMemAllocation						(telErrorClass | 0x12)	// Phone memory is full
#define telErrInvalidIndex								(telErrorClass | 0x13)	// Invalid index when accessing a storage
#define telErrEntryNotFound								(telErrorClass | 0x14)	// Entry not found
#define telErrPhoneMemFailure							(telErrorClass | 0x15)	// The phone encountered a memory error
#define telErrInvalidString								(telErrorClass | 0x16)	// Bad character in text string
#define telErrInvalidDial								(telErrorClass | 0x17)	// Bad character in dial string
#define telErrNoNetwork									(telErrorClass | 0x18)	// No network available
#define telErrNetworkTimeOut							(telErrorClass | 0x19)	// The network didn't reply within 'normal' time delay
#define telErrInvalidParameter							(telErrorClass | 0x1A)	// Bad parameter passed to an API
#define telErrValidityPeriod							(telErrorClass | 0x1B)	// The specified short message validity period is invalid
#define telErrCodingScheme								(telErrorClass | 0x1C)	// The specified short message coding scheme is invalid
#define telErrPhoneNumber								(telErrorClass | 0x1D)	// The specified short message smsc or destination phone number is invalid
#define telErrValueStale								(telErrorClass | 0x1E)	// Information couldn't be retrieved, a copy of last retrieved value was returned
#define telErrDriverNotFound							(telErrorClass | 0x1F)	// The phone driver specified in the phone profile was not found
#define telErrSpcLineIsReleased							(telErrorClass | 0x20)	// The call has been released
#define telErrSpcCallError								(telErrorClass | 0x21)	// The call has encountered an error
#define telErrVersion									(telErrorClass | 0x22)	// The shared lib version doesn't match the application one
#define telErrSettings									(telErrorClass | 0x23)	// Bad telephony settings: Phone Panel Prefs doesn't exist or Telephony Profile not (correctly) set
#define telErrUnavailableValue							(telErrorClass | 0x24)	// The asked value can't be retrieved at that time (i.e.: TelSpcGetCallerNumber and no active line)
#define telErrLimitedCompatibility						(telErrorClass | 0x25)	// The current driver is partially compatible with the connected phone
#define telErrProfileConflict							(telErrorClass | 0x26)	// The currently used profile conflicts with the requested profile
#define telErrNetworkNotAllowed							(telErrorClass | 0x27)	// Network not allowed - Emergency calls only
#define telErrOperationNotAllowed						(telErrorClass | 0x28)	// Operation not allowed
#define telErrAlreadyAuthenticating						(telErrorClass | 0x29)	// Driver is already authenticating, wait for notification 'kTelStyLaunchCmdAuthenticated'
#define telErrCommunicationPortAlreadyUsed				(telErrorClass | 0x2A)	// Communication port is already used by another application
#define telErrSIMPINRequired							(telErrorClass | 0x2B)	// Mobile is waiting SIM PIN to be given
#define telErrSIMPUKRequired							(telErrorClass | 0x2C)	// Mobile is waiting SIM PUK to be given
#define telErrPhoneToSIMPINRequired						(telErrorClass | 0x2D)	// Mobile is waiting phone-to-SIM card password to be given
#define telErrPhoneToFirstSIMPINRequired				(telErrorClass | 0x2E)	// Mobile is waiting phone-to-very first SIM card password to be given
#define telErrPhoneToFirstSIMPUKRequired				(telErrorClass | 0x2F)	// Mobile is waiting phone-to-very first SIM card unblocking password to be given
#define telErrSIMPIN2Required							(telErrorClass | 0x30)	// Mobile is waiting SIM PIN2 to be given
#define telErrSIMPUK2Required							(telErrorClass | 0x31)	// Mobile is waiting SIM PUK2 to be given
#define telErrNetworkPINRequired						(telErrorClass | 0x32)	// Mobile is waiting network personalisation password to be given
#define telErrNetworkPUKRequired						(telErrorClass | 0x33)	// Mobile is waiting network personalisation unblocking password to be given
#define telErrNetworkSubsetPINRequired					(telErrorClass | 0x34)	// Mobile is waiting network subset personalisation password to be given
#define telErrNetworkSubsetPUKRequired					(telErrorClass | 0x35)	// Mobile is waiting network subset personalisation unblocking password to be given
#define telErrProviderPINRequired						(telErrorClass | 0x36)	// Mobile is waiting service provider personalisation password to be given
#define telErrProviderPUKRequired						(telErrorClass | 0x37)	// Mobile is waiting service provider personalisation unblocking password to be given
#define telErrCorporatePINRequired						(telErrorClass | 0x38)	// Mobile is waiting corporate personalisation password to be given
#define telErrCorporatePUKRequired						(telErrorClass | 0x39)	// Mobile is waiting corporate personalisation unblocking password to be given
#define telErrBatteryLevelTooLow                        (telErrorClass | 0x3A)  // The device battery level is too low to allow opening the phone connection
#define telErrGprsIllegalMS								(telErrorClass | 0x3B)
#define telErrGprsIllegalME								(telErrorClass | 0x3C)
#define telErrGprsServicesNotAllowed					(telErrorClass | 0x3D)
#define telErrGprsPLMNNotAllowed						(telErrorClass | 0x3E)
#define telErrGprsLocationAreaNotAllowed				(telErrorClass | 0x3F)
#define telErrGprsRoamingNotAllowedInThisLocationArea	(telErrorClass | 0x40)
#define telErrGprsServiceOptionNotSupported				(telErrorClass | 0x41)
#define telErrGprsRequestedServiceOptionNotSubscribed	(telErrorClass | 0x42)
#define telErrGprsServiceOptionTemporarilyOutOfOrder 	(telErrorClass | 0x43)
#define telErrGprsUnspecifiedError						(telErrorClass | 0x44)
#define telErrGprsPDPAuthenticationFailure				(telErrorClass | 0x45)
#define telErrGprsInvalidMobileClass					(telErrorClass | 0x46)
#define telErrGprsOperatorResourceInsufficient			(telErrorClass | 0x47)
#define telErrGprsUnknowOrMissingAPN					(telErrorClass | 0x48)
#define telErrGprsPdpActivationRejectedGGSN				(telErrorClass | 0x49)
#define telErrGprsPdpActivationRejectedUnspecified		(telErrorClass | 0x4A)
#define telErrGprsPdpDeactivationRegular				(telErrorClass | 0x4B)
#define telErrGprsPdpDeactivationNetworkFailure			(telErrorClass | 0x4C)
#define telErrDataModeActivated							(telErrorClass | 0x4D)
#define telErrMuxNotSupported							(telErrorClass | 0x4E)
#define telErrMuxChanTypeNotSupported					(telErrorClass | 0x4F)
#define telErrMuxChanNotAvailable						(telErrorClass | 0x50)
#define telErrMuxBusy									(telErrorClass | 0x51)
#define telErrAlreadyConnected							(telErrorClass | 0x52)

// Constants
#define	kTelPhoneNumberMaxLen							32
#define	kTelPhoneNameMaxLen								32

#define kTelGprsMaxLen									64

#define	kTelNumberTypeInternational						145
#define	kTelNumberTypeNational							161
#define	kTelNumberTypeUnknown							129

#define	kTelInvalidTransId								0
#define kTelInvalidAppId								(-1)					// This value can't be returned on TelMgr attachement

#define	kTelInfiniteDelay								0xFFFFFFFF				// infinite time-out delay

// Type of Connection
#define kTelConnectionTypeCommand						0	
#define kTelConnectionTypeModem							1
#define kTelConnectionTypeCSD							2
#define kTelConnectionTypeGPRS							3
#define kTelConnectionTypeBT							4
#define kTelConnectionTypeVC							5
#define kTelConnectionTypeOEM							6	

// Mux Status
#define kTelMuxModeStatusNotif							0
#define kTelMuxChanStatusNotif							1
#define kTelMuxModeDisabled								0
#define kTelMuxModeEnabled								1
#define kTelMuxChanClosed								0
#define kTelMuxChanOpened								1

//#include <sys/ioccom.h>
#define kPhoneMuxType									'pmux'
#define IOC_PMUX										'4'
#define	kPMuxEnable										_IO(IOC_PMUX, 0)
#define	kPMuxDisable									_IO(IOC_PMUX, 1)
#define	kPMuxChanOpen									_IO(IOC_PMUX, 2)
#define	kPMuxChanClose									_IO(IOC_PMUX, 3)


// Configuration
#define kTelCfgAlertSoundModeNormal						0
#define kTelCfgAlertSoundModeSilent						1

#define kTelCfgVibratorModeDisable						0
#define kTelCfgVibratorModeEnable						1

#define kTelCfgForwardingReasonUnconditional			0
#define kTelCfgForwardingReasonMobileBusy				1
#define kTelCfgForwardingReasonNoReply					2
#define kTelCfgForwardingReasonNotReachable				3
#define kTelCfgForwardingReasonAllCallForwarding		4
#define kTelCfgForwardingReasonAllCondCallForwarding	5

#define kTelCfgForwardingModeDisable					0
#define kTelCfgForwardingModeEnable						1
#define kTelCfgForwardingModeRegistration				3
#define kTelCfgForwardingModeErasure					4

#define	kTelCfgForwardingClassVoice						1						// Voice (telephony)
#define	kTelCfgForwardingClassData						2						// Data
#define	kTelCfgForwardingClassFax						4						// Fax (facsimile services)
#define	kTelCfgForwardingClassSms						8						// SMS (short message service)
#define	kTelCfgForwardingClassDataCircuitSync			16						// Data circuit sync
#define	kTelCfgForwardingClassDataCircuitAsync			32						// Data circuit async
#define	kTelCfgForwardingClassDedicatedPacketAccess		64						// Dedicated packet access
#define	kTelCfgForwardingClassDedicatedPADAccess		128						// Dedicated PAD access

// Sound
#define kTelSndMuteStatusOff							0
#define kTelSndMuteStatusOn								1

// Network
#define kTelNwkOperatorStatusUnknow						0						// Unknow status
#define kTelNwkOperatorStatusAvailable					1						// Available operator
#define kTelNwkOperatorStatusCurrent					2						// Current operator
#define kTelNwkOperatorStatusForbidden					3						// Forbidden operator

#define kTelNwkTypeCdma									0						// CDMA network
#define kTelNwkTypeGsmGprs								1						// GSM / GPRS network
#define kTelNwkTypeTdma									2						// TDMA network
#define kTelNwkTypePdc									3						// PDC network
#define kTelNwkTypeCdpd									4						// CDPD network

#define kTelNwkStatusNotRegisteredNotSearching			0						// Not registered, ME is not currently searching a new operator to register to
#define kTelNwkStatusRegisteredHome						1						// Registered to home network
#define kTelNwkStatusNotRegisteredSearching				2						// Not registered, but ME is currently searching a new operator to register to
#define kTelNwkStatusRegistrationDenied					3						// Registration denied
#define kTelNwkStatusUnknow								4						// Unknow status
#define kTelNwkStatusRegisteredRoaming					5						// Registered, roaming

#define kTelNwkRegistrationAutomatic					0						// Automatic search mode
#define kTelNwkRegistrationManual						1						// Manual search mode
#define kTelNwkRegistrationManualAutomatic				4						// If manual fails, automatic mode is entered

#define kTelNwkOperatorNameMaxSize						16						// Max size for operator name

#define kTelNwkUssdNoFurtherUserActionRequired			0						// No further user action required
#define kTelNwkUssdFurtherUserActionRequired			1						// Further user action required
#define kTelNwkUssdTerminatedByNetwork					2						// USSD terminated by network
#define kTelNwkUssdOtherClientResponded					3						// Other local client has responsed
#define kTelNwkUssdOperationNotSupported				4						// Operation not supported
#define kTelNwkUssdNetworkTimeOut						5						// Network timeout

// Security
#define kTelStyAuthReady								0						// Mobile is not pending for any password
#define kTelStyAuthSimPin								1						// Mobile is waiting SIM PIN to be given
#define kTelStyAuthSimPuk								2						// Mobile is waiting SIM PUK to be given
#define kTelStyAuthPhoneToSimPin						3						// Mobile is waiting phone-to-SIM card password to be given
#define kTelStyAuthPhoneToFirstSimPin					4						// Mobile is waiting phone-to-very first SIM card password to be given
#define kTelStyAuthPhoneToFirstSimPuk					5						// Mobile is waiting phone-to-very first SIM card unblocking password to be given
#define kTelStyAuthSimPin2								6						// Mobile is waiting SIM PIN2 to be given
#define kTelStyAuthSimPuk2								7						// Mobile is waiting SIM PUK2 to be given
#define kTelStyAuthNetworkPin							8						// Mobile is waiting network personalisation password to be given
#define kTelStyAuthNetworkPuk							9						// Mobile is waiting network personalisation unblocking password to be given
#define kTelStyAuthNetworkSubsetPin						10						// Mobile is waiting network subset personalisation password to be given
#define kTelStyAuthNetworkSubsetPuk						11						// Mobile is waiting network subset personalisation unblocking password to be given
#define kTelStyAuthProviderPin							12						// Mobile is waiting service provider personalisation password to be given
#define kTelStyAuthProviderPuk							13						// Mobile is waiting service provider personalisation unblocking password to be given
#define kTelStyAuthCorporatePin							14						// Mobile is waiting corporate personalisation password to be given
#define kTelStyAuthCorporatePuk							15						// Mobile is waiting corporate personalisation unblocking password to be given
#define kTelStyAuthNoSim								16						// No SIM inserted (Output information in notification only)
#define kTelStyAuthEmergencyOnly						17						// Emergency only - Connection has been opened to dial emergency service  (Output information in notification only)

#define	kTelStyFacilityTypeControl						0x4353					// CS: Lock CONTROL surface
#define	kTelStyFacilityTypePhoneSim						0x5053					// PS: Lock phone to sim card, ask password when other SIM than current is inserted
#define	kTelStyFacilityTypeFirstSim						0x5046					// PF: First Sim entered
#define	kTelStyFacilityTypeSim							0x5343					// SC: SIM
#define	kTelStyFacilityTypeAllOut						0x414F					// AO: Bar all outgoing calls
#define	kTelStyFacilityTypeOutInt						0x4F49					// OI: Bar outgoing international calls
#define	kTelStyFacilityTypeOutIntExHome					0x4F58					// OX: Bar outgoing international calls except home country
#define	kTelStyFacilityTypeAllIn						0x4149					// AI: Bar all incoming calls
#define	kTelStyFacilityTypeInRoaming					0x4952					// IR: Bar incoming calls when roaming outside the home country
#define	kTelStyFacilityTypeInNotTA						0x4E54					// NT: Bar incoming calls from numbers not stored to TA memory
#define	kTelStyFacilityTypeInNotME						0x4E4D					// NM: Bar incoming calls from numbers not stored to ME memory
#define	kTelStyFacilityTypeInNotSIM						0x4E53					// NS: Bar incoming calls from numbers not stored to SIM memory
#define	kTelStyFacilityTypeInNotAny						0x4E41					// NA: Bar incoming calls from numbers not stored to any memory
#define	kTelStyFacilityTypeAllBar						0x4142					// AB: All barring services
#define	kTelStyFacilityTypeAllOutBar					0x4147					// AG: All outgoing barring services
#define	kTelStyFacilityTypeAllInBar						0x4143					// AC: All incoming barring services
#define	kTelStyFacilityTypeSIMFixDial					0x4644					// FD: SIM fixed dialling memory
#define	kTelStyFacilityTypeNetPerso						0x504E					// PN: Network personnalisation
#define	kTelStyFacilityTypeNetSubPerso					0x5055					// PU: Network subset personnalisation
#define	kTelStyFacilityTypeSerProPerso					0x5050					// PP: Service provider personnalisation
#define	kTelStyFacilityTypeCorpPerso					0x5043					// PC: Corporate personnalisation
#define	kTelStyFacilityTypeSimPin2						0x5032					// P2: SIM PIN 2
#define	kTelStyFacilityTypePhoneLock					0x4D45					// ME: Phone lock feature

#define	kTelStyFacilityStatusNotActive					0
#define	kTelStyFacilityStatusActive						1

#define	kTelStyPasswordMaxSize							8

// Information
#define kTelInfPhoneManufacturer						0						// Phone information type
#define kTelInfPhoneModel								1
#define	kTelInfPhoneRevision							2
#define	kTelInfPhoneSerialNumber						3
#define	kTelInfSubscriberIdentity						4

#define	kTelInfPhoneInformationMaxSize					128

#define	kTelInfCallTypeMissed							0
#define	kTelInfCallTypeReceived							1
#define	kTelInfCallTypeDialed							2

// Phonebook
#define kTelPhbMEDialled								0x4443					// DC: Mobile Equipment (ME) dialled calls list
#define	kTelPhbEmergency								0x454E					// EN: SIM or ME emergency number list
#define kTelPhbSIMFixDialling							0x4644					// FD: SIM fixdialling phonebook
#define	kTelPhbSIMLastDialling							0x4C44					// LD: SIM last dialling phonebook
#define	kTelPhbMEMissed									0x4D43					// MC: ME missed calls list
#define kTelPhbME										0x4D45					// ME: ME phonebook
#define kTelPhbMEAndSIM									0x4D54					// MT: Combined ME and SIM phonebook
#define kTelPhbOwnNumbers								0x4F4E					// ON: SIM or ME own numbers list
#define	kTelPhbMEReceived								0x5243					// RC: ME received calls list
#define kTelPhbSIM										0x534D					// SM: SIM phonebook
#define kTelPhbSD										0x5344					// SD: SIM service number
#define kTelPhbTA										0x5441					// TA: Terminal Adaptor (TA) phonebook

// Speech call
#define	kTelSpcAllCalls									0xF0
#define	kTelSpcAllActiveCalls							0xF1
#define	kTelSpcAllHeldCalls								0xF2
#define	kTelSpcIncomingCall								0xF3
#define	kTelSpcDialingCall								0

#define	kTelSpcDirectionMobileOriginated				0						// Mobile originated call
#define	kTelSpcDirectionMobileTerminated				1						// Mobile terminated call

#define kTelSpcStatusActive								0
#define kTelSpcStatusHeld								1
#define kTelSpcStatusDialing							2						// MO call
#define kTelSpcStatusAlerting							3						// MO call
#define kTelSpcStatusIncoming							4						// MT call
#define kTelSpcStatusWaiting							5						// MT call
#define kTelSpcStatusReleased							6

#define kTelSpcModeVoice								0
#define kTelSpcModeData									1
#define kTelSpcModeFax									2

#define	kTelSpcCallerIdValid							0
#define	kTelSpcCallerIdWithheld							1
#define	kTelSpcCallerIdNotAvailable						2

#define kTelSpcCallingLineId							0xFF					// ID of a calling line. We can't provide a real ID knowing that an error might occur after TelSpcCallNumber return... So use this one to 'close' the line
#define kTelSpcGprsLineId								0xFE					// ID of a GPRS line.

#define kTelSpcMaxCallCount								5						// Max number of calls at the same time

// Power
#define kTelPowBatteryPowered							0						// ME is powered by the battery
#define kTelPowBatteryNotPowered						1						// ME has a battery connected, but is not powered by it
#define kTelPowNoBattery								2						// ME does not have a battery connected
#define kTelPowBatteryFault								3						// Power fault

// Messages types
#define kTelSmsMessageTypeDelivered						0
#define kTelSmsMessageTypeReport						1
#define kTelSmsMessageTypeSubmitted						2
#define kTelSmsMessageTypeManualAck						3
#define kTelSmsMessageAllTypes							4

// MessageStatus
#define kTelSmsStatusReceivedUnread						0
#define kTelSmsStatusReceivedRead						1		
#define kTelSmsStatusStoredUnsent						2
#define kTelSmsStatusStoredSent							3

// Message Extension Types
#define kTelSmsMultiPartExtensionTypeId					0x00					// Multipart short messages - 8 bit concatenation
#define kTelSmsMultiPart2ExtensionTypeId				0x08					// Multipart short messages - 16 bit concatenation
#define kTelSmsNbsExtensionTypeId 						0x04					// NBS message, with port number in short
#define kTelSmsNbs2ExtensionTypeId 						0x05					// NBS message, with port number in long
#define kTelSmsSpecialIndicationExtensionTypeId			0x01					// Special SMS Message Indication

// Special SMS message indication
#define kTelSmsSpecialIndicationTypeVM					0x00					// Voice Mail Message Waiting
#define kTelSmsSpecialIndicationTypeFax					0x01					// Fax Message Waiting
#define kTelSmsSpecialIndicationTypeEmail				0x02					// Email Message Waiting
#define kTelSmsSpecialIndicationTypeOther				0x03					// Other Message Waiting

#define kTelSmsDefaultProtocol							0						// Sms message transport protocol
#define kTelSmsFaxProtocol								1
#define kTelSmsX400Protocol								2
#define kTelSmsPagingProtocol							3
#define kTelSmsEmailProtocol							4
#define kTelSmsErmesProtocol							5
#define kTelSmsVoiceProtocol							6

#define kTelSmsUnknownClass								0xFF					// SMS Message Class: class not specified
#define kTelSmsClass0									0x00					// SMS Message Class: Class 0
#define kTelSmsClass1									0x01					// SMS Message Class: Default meaning ME-specific
#define kTelSmsClass2									0x02					// SMS Message Class: SIM specific message
#define kTelSmsClass3									0x03					// SMS Message Class: Default meaning TE-specific

#define kTelSmsAPIVersion								0x0001					// SMS api version

#define kTelSmsStorageSIM								0x534D
#define kTelSmsStoragePhone								0x4D45
#define kTelSmsStorageAdaptor							0x5341

#define kTelSmsCMTMessageType							0						// Cellular Messaging Teleservice message
#define kTelSmsCPTMessageType							1						// Cellular Paging Teleservice message
#define kTelSmsVMNMessageType							2						// Voice Mail Notification message

// Delivery report Type (uint8_t) - Only used in CDMA & TDMA advanced parameters
#define kTelSmsStatusReportDeliveryType					0						// Status report or delivery acknowledge
#define kTelSmsManualAckDeliveryType					1						// Manual acknowledge delivery

// Data coding scheme (uint8_t)
#define kTelSms8BitsEncoding							0
#define kTelSmsBitsASCIIEncoding						1						// ANSI X3.4
#define kTelSmsIA5Encoding								2						// CCITTT T.50
#define kTelSmsIS91Encoding								3						// TIA/EIA/IS-91 section 3.7.1
#define kTelSmsUCS2Encoding 							4						// Only supported by GSM
#define kTelSmsDefaultGSMEncoding						5						// Only supported by GSM
#define kTelSmsAutomatic								6						// Internal coding: choose the best appropriated encoding

// Message urgency / priority (uint8_t) - Only used in CDMA & TDMA advanced parameters
#define kTelSmsUrgencyNormal 							0
#define kTelSmsUrgencyUrgent							1
#define kTelSmsUrgencyEmergency							2
// Bulk (CDMA) & Interactive mode (TDMA) are not supported

// Privacy message indicator (uint8_t) - Only used in CDMA & TDMA advanced parameters
#define kTelSmsPrivacyNotRestricted						0						// Privacy level 0
#define kTelSmsPrivacyRestricted						1						// Privacy level 1
#define kTelSmsPrivacyConfidential						2						// Privacy level 2
#define kTelSmsPrivacySecret							3						// Privacy level 3

// Delivery status report (uint8_t)
#define kTelSmsDSRSuccess								0
#define kTelSmsDSRMessageReplaced						1
#define kTelSmsDSRMessageForwarded						2						// Unknown delivery result
#define kTelSmsDSRTempCongestion						3
#define kTelSmsDSRTempSMEBusy							4
#define kTelSmsDSRTempServiceRejected					5
#define kTelSmsDSRTempServiceUnavailable				6
#define kTelSmsDSRTempSMEError							7
#define kTelSmsDSRTempOther								8
#define kTelSmsDSRPermRPError							9
#define kTelSmsDSRPermBadDestination					10
#define kTelSmsDSRPermUnobtainable						11
#define kTelSmsDSRPermServiceUnavailable				12
#define kTelSmsDSRPermInternetworkError					13
#define kTelSmsDSRPermValidityExpired					14
#define kTelSmsDSRPermDeletedByOrigSME					15
#define kTelSmsDSRPermDeleteByAdm						16
#define kTelSmsDSRPermSMNotExist						17
#define kTelSmsDSRPermOther								18

// GPRS
// GPRS Packet Data Protocol
#define	kTelGprsPdpIP						0	// Internet Protocol
#define	kTelGprsPdpPPP						1	// Point to Point Protocol
#define	kTelGprsPdpOSPIH					2	// Internet Hosted Octet Stream Protocol (IHOSP)
#define kTelGprsValueUnknown				0xFF

// GPRS Compression
#define	kTelGprsDataCompressionSetOff	  	0	// No data compression
#define	kTelGprsDataCompressionSetOn  		1	// V42 bis data compression
#define	kTelGprsHdrCompressionSetOff  		0	// No header compression
#define	kTelGprsHdrCompressionSetOn  		1	// V42 bis header compression

// GPRS OSPIH Protocol
#define	kTelGprsOSPIHProtocolUDP	  		0	// UDP used over IP on GPRS OSPIH
#define	kTelGprsOSPIHProtocolTCP	  		1	// TCP used over IP on GPRS OSPIH

// GPRS Attachment
#define	kTelGprsDetached	  				0	// GPRS Detached
#define	kTelGprsAttached			  		1	// GPRS Attached

// GPRS PDP Activation State
#define	kTelGprsPdpDeactivated				0	// PDP Deactivated
#define	kTelGprsPdpActivated				1	// PDP Activated

// GPRS Network registration
#define	kTelGprsNwkRegistrationDisable							0	// Diasble notifications on GPRS network Registration status change
#define	kTelGprsNwkRegistrationNwkEnable						1	// Notifications on GPRS network Registration status change
#define	kTelGprsNwkRegistrationCellEnable						2	// Notifications on GPRS network and Cell change
#define	kTelGprsNwkRegistrationCellSupportingStatusEnable		3	// Notifications on GPRS network registration status change, cell change, GPRS supporting status of service cell change

// GPRS Network registration status
#define	kTelGprsNwkRegistrationStatusNotRegistered			0	// Not currently searching a new operator to register to
#define	kTelGprsNwkRegistrationStatusRegistered				1	// Home Network
#define	kTelGprsNwkRegistrationStatusSearching				2	// Not registered but currently searching a new operator to register to
#define	kTelGprsNwkRegistrationStatusDenied					3	// Registration denied
#define	kTelGprsNwkRegistrationStatusUnknown				4	// Unknown
#define	kTelGprsNwkRegistrationStatusRoaming				5	// Registered roaming

// GPRS SMS service
#define	kTelGprsSmsGprsOnly						0	// SMS Over GPRS only
#define	kTelGprsSmsGsmOnly						1	// SMS Over GSM only
#define	kTelGprsSmsGprsPreferred					2	// SMS Over GPRS preferred (use GSM if GPRS not available)
#define	kTelGprsSmsGsmPreferred					3	// SMS Over GSM preferred (use GPRS if GSM not available)

// GPRS Layer 2 Protocol
#define	kTelGprsLayer2ProtocolPPP					0	// PPP for a PDP such as IP
#define	kTelGprsLayer2ProtocolNull					1	// none, for PDP type OSP:IHOSS

// GPRS Quality of Service
#define	kTelGprsQosPrecedenceDefault		0
#define	kTelGprsQosPrecedenceHigh			1
#define	kTelGprsQosPrecedenceNormal		2
#define	kTelGprsQosPrecedenceLow			3
#define	kTelGprsQosDelayDefault					0
#define	kTelGprsQosDelayClass1					1	// <2 seconds for a 1024 SDU size
#define	kTelGprsQosDelayClass2					2	// <15 seconds for a 1024 SDU size
#define	kTelGprsQosDelayClass3					3	// <75 seconds for a 1024 SDU size
#define	kTelGprsQosDelayBestEffort				4	// Best Effort
#define	kTelGprsQosReliabilityDefault			0	
#define	kTelGprsQosReliabilityClass1			1	//  GTP Mode Acknowledged, LLC Mode Acknowledged, LLC Data Protected, RLC Block Acknowledged
#define	kTelGprsQosReliabilityClass2			2	//  GTP Mode Unacknowledged, LLC Mode Acknowledged, LLC Data Protected, RLC Block Acknowledged
#define	kTelGprsQosReliabilityClass3			3	//  GTP Mode Unacknowledged, LLC Mode Unacknowledged, LLC Data Protected, RLC Block Acknowledged
#define	kTelGprsQosReliabilityClass4			4	//  GTP Mode Unacknowledged, LLC Mode Unacknowledged, LLC Data Protected, RLC Block Unacknowledged
#define	kTelGprsQosReliabilityClass5			5	//  GTP Mode Unacknowledged, LLC Mode Unacknowledged, LLC Data Unprotected, RLC Block Unacknowledged
#define	kTelGprsQosPeakDefault			0
#define	kTelGprsQosPeakClass1			1	// Up to 1 000 octets/s (8 kbit/s)
#define	kTelGprsQosPeakClass2			2	// Up to 2 000 octets/s (16 kbit/s)
#define	kTelGprsQosPeakClass3			3	// Up to 4 000 octets/s (32 kbit/s)
#define	kTelGprsQosPeakClass4			4	// Up to 8 000 octets/s (64 kbit/s)
#define	kTelGprsQosPeakClass5			5	// Up to 16 000 octets/s (128 kbit/s)
#define	kTelGprsQosPeakClass6			6	// Up to 32 000 octets/s (256 kbit/s)
#define	kTelGprsQosPeakClass7			7	// Up to 64 000 octets/s (512 kbit/s)
#define	kTelGprsQosPeakClass8			8	// Up to 128 000 octets/s (1024 kbit/s)
#define	kTelGprsQosPeakClass9			9	// Up to 256 000 octets/s (2048 kbit/s)
#define	kTelGprsQosMeanDefault				0
#define	kTelGprsQosMeanClass1				1	// 100 octets/hour (~0.22 bit/s)
#define	kTelGprsQosMeanClass2				2	// 200 octets/hour (~0.44 bit/s)
#define	kTelGprsQosMeanClass3				3	// 500 octets/hour (~1.11 bit/s)
#define	kTelGprsQosMeanClass4				4	// 1 000 octets/hour (~2.2 bit/s)
#define	kTelGprsQosMeanClass5				5	// 2 000 octets/hour (~4.4 bit/s)
#define	kTelGprsQosMeanClass6				6	// 5 000 octets/hour (~11.1 bit/s)
#define	kTelGprsQosMeanClass7				7	// 10 000 octets/hour (~22 bit/s)
#define	kTelGprsQosMeanClass8				8	// 20 000 octets/hour (~44 bit/s)
#define	kTelGprsQosMeanClass9				9	// 50 000 octets/hour (~111 bit/s)
#define	kTelGprsQosMeanClass10				10	// 100 000 octets/hour (~0.22 kbit/s)
#define	kTelGprsQosMeanClass11				11	// 200 000 octets/hour (~0.44 kbit/s)
#define	kTelGprsQosMeanClass12				12	// 500 000 octets/hour (~1.11 kbit/s)
#define	kTelGprsQosMeanClass13				13	// 1 000 000 octets/hour (~2.2 kbit/s)
#define	kTelGprsQosMeanClass14				14	// 2 000 000 octets/hour (~4.4 kbit/s)
#define	kTelGprsQosMeanClass15				15	// 5 000 000 octets/hour (~11.1 kbit/s)
#define	kTelGprsQosMeanClass16				16	// 10 000 000 octets/hour (~22 kbit/s)
#define	kTelGprsQosMeanClass17				17	// 20 000 000 octets/hour (~44 kbit/s)
#define	kTelGprsQosMeanClass18				18	// 50 000 000 octets/hour (~111 kbit/s)
#define	kTelGprsQosMeanClassBestEffort		31	// Best Effort

// GPRS Event Reporting
#define	kTelGprsEventReject						0	// PDP context activation rejected
#define	kTelGprsEventNwReact					1	// PDP context activation reactivated by network
#define	kTelGprsEventNwDeact					2	// PDP context activation deactivated by network
#define	kTelGprsEventMeDeact					3	// PDP context activation deactivated by mobile
#define	kTelGprsEventNwDetach					4	// GPRS detached by network
#define	kTelGprsEventMeDetach					5	// GPRS detached by mobile
#define	kTelGprsEventNwClass					6	// MS Class changed by network
#define	kTelGprsEventMeClass					7	// MS Class changed by mobile
#define	kTelGprsEventReportingDisabledMode			0	// No event reporting forwarded
#define	kTelGprsEventReportingEnabledMode			1	// Event reporting forwarded if link OK
#define	kTelGprsEventReportingBufferedMode			2	// Event reporting forwarded if link OK or buffered and then forwared when link is OK again
#define	kTelGprsEventReportingClearBuffer			0	// Mobile buffer of unsolicited result code is cleared when kTelOemGprsEventReportingEnabledMode or kTelOemGprsEventReportingBufferedMode is chosen
#define	kTelGprsEventReportingFlushBuffer			1	// Mobile buffer of unsolicited result code is flushed to Telephony when kTelOemGprsEventReportingEnabledMode or kTelOemGprsEventReportingBufferedMode is chosen

// GSM CSD: Bearer service speed
#define	kTelDtcBearerDataRateAuto				0	// autobauding (automatic selection of the speed; this setting is possible in case of 3.1 kHz modem	and non-transparent service)
#define	kTelDtcBearerDataRate300bpsV21			1	// 300 bps (V.21)
#define	kTelDtcBearerDataRate1200bpsV22		2	// 1200 bps (V.22)
#define	kTelDtcBearerDataRate1200_75bpsV23	3	// 1200/75 bps (V.23)
#define	kTelDtcBearerDataRate2400bpsV22bis		4	// 2400 bps (V.22bis)
#define	kTelDtcBearerDataRate2400bpsV26ter		5	// 2400 bps (V.26ter)
#define	kTelDtcBearerDataRate4800bpsV32		6	// 4800 bps (V.32)
#define	kTelDtcBearerDataRate9600bpsV32		7	// 9600 bps (V.32)
#define	kTelDtcBearerDataRate9600bpsV34		12	// 9600 bps (V.34)
#define	kTelDtcBearerDataRate14400bpsV34		14 	// 14400 bps (V.34)
#define	kTelDtcBearerDataRate19200bpsV34		15 	// 19200 bps (V.34)
#define	kTelDtcBearerDataRate28800bpsV34		16	// 28800 bps (V.34)
#define	kTelDtcBearerDataRate1200bpsV120		34	// 1200 bps (V.120)
#define	kTelDtcBearerDataRate2400bpsV120		36	// 2400 bps (V.120)
#define	kTelDtcBearerDataRate4800bpsV120		38	// 4800 bps (V.120)
#define	kTelDtcBearerDataRate9600bpsV120		39	// 9600 bps (V.120)
#define	kTelDtcBearerDataRate14400bpsV120		43	// 14400 bps (V.120)
#define	kTelDtcBearerDataRate19200bpsV120		47	// 19200 bps (V.120)
#define	kTelDtcBearerDataRate28800bpsV120		48	// 28800 bps (V.120)
#define	kTelDtcBearerDataRate38400bpsV120		49	// 38400 bps (V.120)
#define	kTelDtcBearerDataRate48000bpsV120		50	// 48000 bps (V.120)
#define	kTelDtcBearerDataRate56000bpsV120		51	// 56000 bps (V.120)
#define	kTelDtcBearerDataRate300bpsV110		65	// 300 bps (V.110)
#define	kTelDtcBearerDataRate1200bpsV110		66	// 1200 bps (V.110)
#define	kTelDtcBearerDataRate2400bpsV110		68	// 2400 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate4800bpsV110		70	// 4800 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate9600bpsV110		71	// 9600 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate14400bpsV110		75	// 14400 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate19200bpsV110		79	// 19200 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate28800bpsV110		80	// 28800 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate38400bpsV110		81	// 38400 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate48000bpsV110		82	// 48000 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate56000bpsV110		83	// 56000 bps (V.110 or X.31 flag stuffing)
#define	kTelDtcBearerDataRate56000bpsTrans	115	// 56000 bps (bit transparent)
#define	kTelDtcBearerDataRate64000bpsTrans	116	// 64000 bps (bit transparent)

// GSM CSD: Bearer service name
#define	kTelDtcBearerDataAsynchronousUDI				0	// data circuit asynchronous (UDI or 3.1 kHz modem)
#define	kTelDtcBearerDataSynchronousUDI				1	// data circuit synchronous (UDI or 3.1 kHz modem)
#define	kTelDtcBearerPADAccessAsynchronousUDI		2	// PAD Access (asynchronous) (UDI)
#define	kTelDtcBearerPacketAccessSynchronousUDI		3	// Packet Access (synchronous) (UDI)
#define	kTelDtcBearerDataAsynchronousRDI				4	// data circuit asynchronous (RDI)
#define	kTelDtcBearerDataSynchronousRDI				5	// data circuit synchronous (RDI)
#define	kTelDtcBearerPADAccessAsynchronousRDI			6	// PAD Access (asynchronous) (RDI)
#define	kTelDtcBearerPacketAccessSynchronousRDI		7	// Packet Access (synchronous) (RDI)

// GSM CSD: Bearer service cconnection element
#define	kTelDtcBearerConnectionTransparent						0	// transparent 
#define	kTelDtcBearerConnectionNonTransparent					1	// non-transparent 
#define	kTelDtcBearerConnectionBothTransparentPreferred		2	// both, transparent preferred 
#define	kTelDtcBearerConnectionBothNonTransparentPreferred		3	// both, non-transparent preferred 

#define kTelSpeechCallClass								0						// Call classes
#define kTelDataCallClass								1
#define kTelFaxCallClass								2

#define kTelCallIdle									0						// Call states
#define kTelCallConnecting								1
#define kTelCallConnected								2
#define kTelCallRedial									3
#define kTelCallIncoming								4
#define kTelCallIncomingAck								5
#define kTelCallDisconnecting							6

#define kTelCallTypeOutgoing							0						// Call type
#define kTelCallTypeIncoming							1

#define kTelCallServiceVoice							0						// Call service type
#define kTelCallServiceData								1


// Card EF access mode
#define	kTelCardModeGetInfo								0						// Get EF information
#define	kTelCardModeReadFile							1						// Read EF body 
#define	kTelCardModeReadPart							2						// Read EF part 
#define	kTelCardModeReadRec								3						// Read EF record

// Card EF structure
#define kTelCardFileStructTransparent		0x00
#define kTelCardFileStructLinearFixed		0x01
#define kTelCardFileStructCyclic			0x03

// Commands Id
#define kTelCatCmdRefresh					0x01
#define kTelCatCmdSetUpEventList			0x05
#define kTelCatCmdSetUpCall					0x10
#define kTelCatCmdSendSS					0x11
#define kTelCatCmdSendUSSD					0x12
#define kTelCatCmdSendShortMessage			0x13
#define kTelCatCmdSendDTMF					0x14
#define kTelCatCmdLaunchBrowser				0x15
#define kTelCatCmdPlayTone					0x20
#define kTelCatCmdDisplayText				0x21
#define kTelCatCmdGetInkey					0x22
#define kTelCatCmdGetInput					0x23
#define kTelCatCmdSelectItem				0x24
#define kTelCatCmdSetUpMenu					0x25
#define kTelCatCmdSetUpIdleModeText			0x28
#define kTelCatCmdRunATCommand				0x34
#define kTelCatCmdOpenChannel				0x40
#define kTelCatCmdCloseChannel				0x41
#define kTelCatCmdReceiveData				0x42
#define kTelCatCmdSendData					0x43

#define kTelCatEndOfProactiveSession		0x81
////////////////////////////
// Cmd: Refresh
#define kTelCatRefreshInitAndFullFileChange	0x00
#define kTelCatRefreshFileChange			0x01
#define kTelCatRefreshInitAndFileChange		0x02
#define kTelCatRefreshInitialization		0x03
#define kTelCatRefreshHardReset				0x04
////////////////////////////
// Cmd: SetUpEventList
#define kTelCatEventUserActivity			0x04
#define kTelCatEventIdleScreenAvailable		0x05
#define kTelCatEventLanguageSelection		0x07
#define kTelCatEventBrowserTermination		0x08
////////////////////////////
// Cmd: SetUpCall
#define kTelCatCallNotBusy					0x00
#define kTelCatCallNotBusyRedial			0x01
#define kTelCatCallHoldOthers				0x02
#define kTelCatCallHoldOthersRedial			0x03
#define kTelCatCallCloseOthers				0x04
#define kTelCatCallCloseOthersRedial		0x05
////////////////////////////
// Bearer codes
#define kTelCatBearerCSD					0x1		// Applies to Launch Browser, Open Channel
#define kTelCatBearerGPRS					0x2		// Applies to Launch Browser, Open Channel
#define kTelCatBearerSMS					0x3		// Applies to Launch Browser
#define kTelCatBearerUSSD					0x4		// Applies to Launch Browser
////////////////////////////
// Cmd: LaunchBrowser (conditions)
#define kTelCatBrowserLaunchIfNotLaunched		0x00
#define kTelCatBrowserUseExisting				0x02
#define kTelCatBrowserCloseExistingLaunchNew	0x03
////////////////////////////
// Cmd: PlayTone			
#define kTelCatSoundStdDial					0x01
#define kTelCatSoundStdCalledPartyBusy		0x02
#define kTelCatSoundStdCongestion			0x03
#define kTelCatSoundStdRadioPathAck			0x04
#define kTelCatSoundStdCallDropped			0x05
#define kTelCatSoundStdError				0x06
#define kTelCatSoundStdCallWaiting			0x07
#define kTelCatSoundStdRing					0x08
#define kTelCatSoundGeneralBeep				0x10
#define kTelCatSoundPositiveAck				0x11
#define kTelCatSoundError					0x12
////////////////////////////
// Cmd: GetInkey - GetInput
#define kTelCatRespTypeYesOrNo				0x01	// Applies to GetInkey
#define kTelCatRespTypeDigitsGSM			0x02	// Applies to GetInkey, GetInput
#define kTelCatRespTypeDigitsGSMPacked		0x03	// Applies to GetInput
#define kTelCatRespTypeDigitsUCS2			0x04	// Applies to GetInkey, GetInput
#define kTelCatRespTypeTextGSM				0x05	// Applies to GetInkey, GetInput
#define kTelCatRespTypeTextGSMPacked 		0x06	// Applies to GetInput
#define kTelCatRespTypeTextUCS2				0x07	// Applies to GetInkey, GetInput
////////////////////////////
// Menu Selection
#define kTelCatMenuSelAppLaunch				0x01
#define kTelCatMenuSelHelpInfoRequest		0x02
#define kTelCatMenuSelAppMenuRequest		0x03
////////////////////////////
// Event to card
#define kTelCatBrowserTerminationUser		0x00
#define kTelCatBrowserTerminationError		0x01
///////////////////////////
// Cmd: Open Chanel
#define kTelCatAddressIPv4					0x21
#define kTelCatAddressIPv6					0x97

#define kTelCatTransportUDP					0x01
#define	kTelCatTransportTCP					0x02

///////////////////////////
// Terminate reasons
#define kTelCatTerminateUserStoppedRedialing	0
#define kTelCatTerminateEndOfRedialingReached	1
#define kTelCatTerminateUserEndsSession			2

///////////////////////////
// Setup Call actions
#define kTelCatCallReject						0
#define kTelCatCallAccept						1

////////////////////////////
// General result codes
#define kTelCatResSuccess					0x00
#define kTelCatResOkPartialComprehension	0x01
#define kTelCatResOkMissingInfo				0x02
#define kTelCatResOkAdditionalEfsRead		0x03
#define kTelCatResOkIconNotDisplayed		0x04
#define kTelCatResOkModifiedBySim			0x05
#define kTelCatResOkLimitedService			0x06
#define kTelCatResOkWithModification		0x07
#define kTelCatResUserTermination			0x10
#define kTelCatResBackwardMove				0x11
#define kTelCatResNoResponseFromUser		0x12
#define kTelCatResHelpInfoRequest			0x13
#define kTelCatResTransactionTermination	0x14
#define kTelCatResMeUnableNow				0x20
#define kTelCatResNetworkUnableNow			0x21
#define kTelCatResUserDismissal				0x22
#define kTelCatResCallClearedByUser			0x23
#define kTelCatResTimerContradiction		0x24
#define kTelCatResSimControlInteraction		0x25
#define kTelCatResBrowserGenericError		0x26
#define kTelCatResBeyondMeCapabilities		0x30
#define kTelCatResCmdTypeNotUnderstood		0x31
#define kTelCatResCmdDataNotUnderstood		0x32
#define kTelCatResUnknownCmdNumber			0x33
#define kTelCatResSuppSvcReturnError		0x34
#define kTelCatResSmsRpError				0x35
#define kTelCatResMissingValues				0x36
#define kTelCatResUssdReturnError			0x37
#define kTelCatResMultipleCardError			0x38
#define kTelCatResSimControlFault			0x39
#define kTelCatResBearerIndProtocolError	0x3A
////////////////////////////
// Additional Information codes for miscellaneous errors
#define kTelCatAddGeNoSpecificCause			0x00
////////////////////////////
// Additional Information codes for Launch Browser generic error
#define kTelCatAddLbBearerUnavailable		0x01
#define kTelCatAddLbBrowserUnavailable		0x02
#define kTelCatAddLbDataReadError			0x03
////////////////////////////
// Additional Information codes for "ME currently unable to process command" error
#define kTelCatAddUnScreenBusy				0x01
#define kTelCatAddUnMeBusyOnCall			0x02
#define kTelCatAddUnMeBusyOnSuppSvc			0x03
#define kTelCatAddUnNoService				0x04
#define kTelCatAddUnAccessControlBar		0x05
#define kTelCatAddUnNoRadioResource			0x06
#define kTelCatAddUnNotInSpeechCall			0x07
#define kTelCatAddUnMeBusyOnUssd			0x08
#define kTelCatAddUnMeBusyOnSendDtmf		0x09
////////////////////////////
// Additional Information codes for "Interaction with Call Control by SIM
// or MO Short Message Control by SIM, permanent problem" error (SIM Control fault)
#define kTelCatAddCsActionNotAllowed		0x01
#define kTelCatAddCsRequestTypeChange		0x02
////////////////////////////
// Additional Information codes for "Bearer Independent Protocol error"
#define kTelCatAddBiNoChannelAvailable		0x01
#define kTelCatAddBiChannelClosed			0x02
#define kTelCatAddBiInvalidChannelId		0x03
#define kTelCatAddBiBufSizeUnavailable		0x04
#define kTelCatAddBiSecurityError			0x05
#define kTelCatAddBiTransportUnavailable	0x06

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *	Prototypes
 *****************************************************************************/

// General API
status_t	TelOpen									(uint32_t iVersnum, int32_t* telDescP);
status_t	TelOpenPhoneProfile						(uint32_t iVersnum, int32_t* telDescP, uint32_t iProfileId);
status_t	TelClose								(int32_t telDesc);

status_t	TelCancel								(int32_t telDesc, uint16_t iCanceledTransId, uint16_t* ioTransIdP);
status_t	TelTestPhoneDriver						(int32_t telDesc, TelInfIdentificationPtr ioNameP, uint16_t* ioTransIdP);

// Events
void		TelEvtGetEvent							(int32_t telDesc, EventPtr oEventP, int32_t iTimeOut);
void		TelEvtGetTelephonyEvent					(int32_t telDesc, EventPtr oEventP, int32_t iTimeOut);

// Connection
status_t	TelCncOpen								(int32_t telDesc, uint16_t* ioTransIdP);
status_t	TelCncClose								(int32_t telDesc);
status_t	TelCncGetStatus							(int32_t telDesc, uint8_t* oStatusP);

// Network
status_t	TelNwkGetOperators						(int32_t telDesc, TelNwkOperatorsPtr ioOperatorP, uint16_t* ioTransIdP);
status_t	TelNwkGetOperator						(int32_t telDesc, TelNwkOperatorPtr ioOperatorP, uint16_t* ioTransIdP);
status_t	TelNwkSetOperator						(int32_t telDesc, uint32_t iOperatorId, uint16_t* ioTransIdP);
status_t	TelNwkGetPreferredOperators				(int32_t telDesc, TelNwkPreferredOperatorsPtr ioPreferedOperatorsP, uint16_t* ioTransIdP);
status_t	TelNwkAddPreferredOperator				(int32_t telDesc, TelNwkPreferredOperatorPtr iPreferedOperatorP, uint16_t* ioTransIdP);
status_t	TelNwkDeletePreferredOperator			(int32_t telDesc, uint16_t iIndex, uint16_t* ioTransIdP);
status_t	TelNwkGetRegistrationMode				(int32_t telDesc, uint8_t* oRegistrationModeP, uint16_t* ioTransIdP);
status_t	TelNwkSetRegistration					(int32_t telDesc, TelNwkRegistrationType* iRegistrationP, uint16_t* ioTransIdP);
status_t	TelNwkGetType							(int32_t telDesc, uint8_t* oTypeP, uint16_t* ioTransIdP);
status_t	TelNwkGetStatus							(int32_t telDesc, uint8_t* oStatusP, uint16_t* ioTransIdP);
status_t	TelNwkGetProviderId						(int32_t telDesc, uint32_t* oProviderIdP, uint16_t* ioTransIdP);
status_t	TelNwkGetLocation						(int32_t telDesc, TelNwkLocationPtr ioLocationP, uint16_t* ioTransIdP);
status_t	TelNwkGetSignalLevel					(int32_t telDesc, uint8_t* oSignalLevelP, uint16_t* ioTransIdP);
status_t	TelNwkCheckUssd							(int32_t telDesc, TelNwkUssdPtr iUssdP, uint16_t* ioTransIdP);
status_t	TelNwkSendUssd							(int32_t telDesc, TelNwkUssdPtr iUssdP, uint16_t* ioTransIdP);
status_t	TelNwkReceiveUssd						(int32_t telDesc, TelNwkUssdPtr ioUssdP, uint16_t* ioTransIdP);
status_t	TelNwkCancelUssd						(int32_t telDesc, uint16_t* ioTransIdP);

// Security
status_t	TelStyGetAuthenticationStatus			(int32_t telDesc, uint8_t* oAuthenticationP, uint16_t* ioTransIdP);
status_t	TelStyEnterAuthentication				(int32_t telDesc, TelStyAuthenticationPtr iAuthenticationP, uint16_t* ioTransIdP);
status_t	TelStyGetFacilities						(int32_t telDesc, TelStyFacilitiesPtr ioFacilitiesP, uint16_t* ioTransIdP);
status_t	TelStyGetFacility						(int32_t telDesc, TelStyFacilityPtr iFacilityP, uint16_t* ioTransIdP);
status_t	TelStyLockFacility						(int32_t telDesc, TelStyFacilityPtr iFacilityP, uint16_t* ioTransIdP);
status_t	TelStyUnlockFacility					(int32_t telDesc, TelStyFacilityPtr iFacilityP, uint16_t* ioTransIdP);
status_t	TelStyChangeFacilityPassword			(int32_t telDesc, TelStyFacilityPasswordPtr iFacilityPasswordP, uint16_t* ioTransIdP);

// Power
status_t	TelPowGetBatteryChargeLevel				(int32_t telDesc, uint8_t* oBatteryChargeLevelP, uint16_t* ioTransIdP);
status_t	TelPowGetBatteryConnectionStatus		(int32_t telDesc, uint8_t* oBatteryConnectionStatusP, uint16_t* ioTransIdP);
status_t	TelPowSetPhoneFunctionality				(int32_t telDesc, uint8_t iPhoneFunctionality, uint16_t* ioTransIdP);

// Configuration
status_t	TelCfgGetSmsCenter						(int32_t telDesc, TelNumberPtr ioSmsCenterP, uint16_t* ioTransIdP);
status_t	TelCfgSetSmsCenter						(int32_t telDesc, TelNumberPtr iSmsCenterP, uint16_t* ioTransIdP);
status_t	TelCfgGetPhoneNumber					(int32_t telDesc, TelCfgPhoneNumberPtr ioPhoneNumberP, uint16_t* ioTransIdP);
status_t	TelCfgSetPhoneNumber					(int32_t telDesc, TelCfgPhoneNumberPtr iPhoneNumberP, uint16_t* ioTransIdP);
status_t	TelCfgGetVoiceMailNumber				(int32_t telDesc, TelNumberPtr ioVoiceMailNumberP, uint16_t* ioTransIdP);
status_t	TelCfgSetVoiceMailNumber				(int32_t telDesc, TelNumberPtr iVoiceMailNumberP, uint16_t* ioTransIdP);
status_t	TelCfgGetLoudspeakerVolumeLevelRange	(int32_t telDesc, TelCfgLevelRangePtr oLoudspeakerVolumeLevelRangeP, uint16_t* ioTransIdP);
status_t	TelCfgGetLoudspeakerVolumeLevel			(int32_t telDesc, uint8_t* oLoudspeakerVolumeLevelP, uint16_t* ioTransIdP);
status_t	TelCfgSetLoudspeakerVolumeLevel			(int32_t telDesc, uint8_t iLoudspeakerVolumeLevel, uint16_t* ioTransIdP);
status_t	TelCfgGetRingerSoundLevelRange			(int32_t telDesc, TelCfgLevelRangePtr oRingerSoundLevelRangeP, uint16_t* ioTransIdP);
status_t	TelCfgGetRingerSoundLevel				(int32_t telDesc, uint8_t* oRingerSoundLevelP, uint16_t* ioTransIdP);
status_t	TelCfgSetRingerSoundLevel				(int32_t telDesc, uint8_t iRingerSoundLevel, uint16_t* ioTransIdP);
status_t	TelCfgGetVibratorMode					(int32_t telDesc, uint8_t* oVibratorModeP, uint16_t* ioTransIdP);
status_t	TelCfgSetVibratorMode					(int32_t telDesc, uint8_t iVibratorMode, uint16_t* ioTransIdP);
status_t	TelCfgGetAlertSoundMode					(int32_t telDesc, uint8_t* oAlertSoundModeP, uint16_t* ioTransIdP);
status_t	TelCfgSetAlertSoundMode					(int32_t telDesc, uint8_t iAlertSoundMode, uint16_t* ioTransIdP);
status_t	TelCfgGetCallForwarding					(int32_t telDesc, TelCfgCallForwardingPtr ioCallForwardingP, uint16_t* ioTransIdP);
status_t	TelCfgSetCallForwarding					(int32_t telDesc, TelCfgCallForwardingPtr iCallForwardingP, uint16_t* ioTransIdP);
status_t	TelCfgGetCallIdRestrictionStatus		(int32_t telDesc, uint8_t* oCallIdRestrictionP, uint16_t* ioTransIdP);
status_t	TelCfgSetCallIdRestrictionStatus		(int32_t telDesc, uint8_t iCallIdRestriction, uint16_t* ioTransIdP);

// SMS
status_t	TelSmsSendMessage						(int32_t telDesc, TelSmsMessagePtr ioMessageP, uint16_t* ioTransIdP);
status_t	TelSmsReadMessages						(int32_t telDesc, TelSmsMessagesPtr ioMessagesP, uint16_t* ioTransIdP);
status_t	TelSmsReadMessage						(int32_t telDesc, TelSmsMessagePtr ioMessageP, uint16_t* ioTransIdP);
status_t	TelSmsDeleteMessage						(int32_t telDesc, uint16_t iMessageIndex, uint16_t* ioTransIdP);
status_t	TelSmsGetUniquePartId					(int32_t telDesc, uint16_t* oUniquePartIdP, uint16_t* ioTransIdP);
status_t	TelSmsGetDataMaxSize					(int32_t telDesc, size_t* oDataMaxSizeP, uint16_t* ioTransIdP);
status_t	TelSmsGetStorages						(int32_t telDesc, TelSmsStoragesPtr ioStoragesP, uint16_t* ioTransIdP);
status_t	TelSmsGetStorage						(int32_t telDesc, TelSmsStoragePtr oStorageP, uint16_t* ioTransIdP);
status_t	TelSmsSetStorage						(int32_t telDesc, uint16_t iStorageId, uint16_t* ioTransIdP);

// Emergency calls
status_t	TelEmcDial								(int32_t telDesc, TelSpcCallPtr oCallP, uint16_t* ioTransIdP);

// Speech call
status_t	TelSpcGetCalls							(int32_t telDesc, TelSpcCallsPtr ioCallsP, uint16_t* ioTransIdP);
status_t	TelSpcGetCall							(int32_t telDesc, TelSpcCallPtr ioCallP, uint16_t* ioTransIdP);
status_t	TelSpcInitiateCall						(int32_t telDesc, TelSpcCallPtr ioCallP, uint16_t* ioTransIdP);
status_t	TelSpcAcceptCall						(int32_t telDesc, TelSpcCallPtr oCallP, uint16_t* ioTransIdP);
status_t	TelSpcReleaseCall						(int32_t telDesc, uint8_t iCallId, uint16_t* ioTransIdP);
status_t	TelSpcHoldActiveCalls					(int32_t telDesc, uint16_t* ioTransIdP);
status_t	TelSpcAddHeldCall						(int32_t telDesc, uint16_t* ioTransIdP);
status_t	TelSpcPrivateCall						(int32_t telDesc, uint8_t iCallId, uint16_t* ioTransIdP);
status_t	TelSpcPlayTone							(int32_t telDesc, char iTone, uint16_t* ioTransIdP);
status_t	TelSpcGetToneDurationRange				(int32_t telDesc, TelSpcToneDurationRangePtr ioToneDurationRangeP, uint16_t* ioTransIdP);
status_t	TelSpcGetToneDuration					(int32_t telDesc, uint16_t* ioToneDurationP, uint16_t* ioTransIdP);
status_t	TelSpcSetToneDuration					(int32_t telDesc, uint16_t iToneDuration, uint16_t* ioTransIdP);

// Phonebook
status_t	TelPhbGetPhonebooks						(int32_t telDesc, TelPhbPhonebooksPtr ioPhonebooksP, uint16_t* ioTransIdP);
status_t	TelPhbGetPhonebook						(int32_t telDesc, TelPhbPhonebookPtr oPhonebookP, uint16_t* ioTransIdP);
status_t	TelPhbSetPhonebook						(int32_t telDesc, TelPhbPhonebookPtr ioPhonebookP, uint16_t* ioTransIdP);
status_t	TelPhbGetEntries						(int32_t telDesc, TelPhbEntriesPtr ioEntriesP, uint16_t* ioTransIdP);
status_t	TelPhbGetEntry							(int32_t telDesc, TelPhbEntryPtr ioEntryP, uint16_t* ioTransIdP);
status_t	TelPhbAddEntry							(int32_t telDesc, TelPhbEntryPtr iEntryP, uint16_t* ioTransIdP);
status_t	TelPhbDeleteEntry						(int32_t telDesc, uint16_t iEntryIndex, uint16_t* ioTransIdP);

// Sound
status_t	TelSndGetMuteStatus						(int32_t telDesc, uint8_t* oMuteStatusP, uint16_t* ioTransIdP);
status_t	TelSndSetMuteStatus						(int32_t telDesc, uint8_t iMuteStatus, uint16_t* ioTransIdP);

// Information
status_t	TelInfGetIdentification					(int32_t telDesc, TelInfIdentificationPtr ioParamP, uint16_t* ioTransIdP);
status_t	TelInfGetCallsDuration					(int32_t telDesc, TelInfCallsDurationPtr ioCallsDurationP, uint16_t* ioTransIdP);
status_t	TelInfResetCallsDuration				(int32_t telDesc, uint16_t* ioTransIdP);
status_t	TelInfGetCallsList						(int32_t telDesc, TelInfCallsListPtr ioCallsListP, uint16_t* ioTransIdP);
status_t	TelInfResetCallsList					(int32_t telDesc, uint8_t iCallTypeP, uint16_t* ioTransIdP);

// Service/function availability
status_t	TelIsServiceAvailable					(int32_t telDesc, uint16_t iServiceId);
status_t	TelIsFunctionSupported					(int32_t telDesc, uint16_t iFunctionId);

// OEM
status_t	TelOemCall								(int32_t telDesc, TelOemCallPtr ioOemCallP, uint16_t* ioTransIdP);

// UI
status_t	TelUiManageError						(status_t iError, Boolean* ioRetryP);

// GPRS
status_t	TelGprsGetAvailableContextId(int32_t telDesc, uint8_t* oContextIdP, uint16_t* ioTransIdP);
status_t	TelGprsSetContext(int32_t telDesc, TelGprsContextPtr iContextP, uint16_t* ioTransIdP);
status_t	TelGprsGetContext(int32_t telDesc, TelGprsContextPtr ioContextP, uint16_t* ioTransIdP);
status_t	TelGprsGetDefinedCids(int32_t telDesc, TelGprsDefinedCidsPtr ioCidsP, uint16_t* ioTransIdP);
status_t	TelGprsSetAttach(int32_t telDesc, uint8_t iAttach, uint16_t* ioTransIdP);
status_t	TelGprsGetAttach(int32_t telDesc, uint8_t* oAttach, uint16_t* ioTransIdP);
status_t	TelGprsSetPdpActivation(int32_t telDesc, TelGprsPdpActivationPtr iPdpActivationP, uint16_t* ioTransIdP);
status_t	TelGprsGetPdpActivation(int32_t telDesc, TelGprsPdpActivationPtr ioPdpActivationP, uint16_t* ioTransIdP);
status_t	TelGprsGetPdpAddress(int32_t telDesc, TelGprsPdpAddressPtr ioPdpAddressP, uint16_t* ioTransIdP);
status_t	TelGprsSetNwkRegistration(int32_t telDesc, uint8_t iRegistrationType, uint16_t* ioTransIdP);
status_t	TelGprsGetNwkRegistration(int32_t telDesc, TelGprsNwkRegistrationPtr ioRegistrationP, uint16_t* ioTransIdP);
status_t	TelGprsSetSmsService(int32_t telDesc, uint8_t iSMSService, uint16_t* ioTransIdP);
status_t	TelGprsGetSmsService(int32_t telDesc, uint8_t* oSMSService, uint16_t* ioTransIdP);
status_t	TelGprsSetQosRequested(int32_t telDesc, TelGprsQosPtr iQosRequestedP, uint16_t* ioTransIdP);
status_t	TelGprsGetQosRequested(int32_t telDesc, TelGprsQosPtr ioQosRequestedP, uint16_t* ioTransIdP);
status_t	TelGprsSetQosMinimum(int32_t telDesc, TelGprsQosPtr iQosMinimumP, uint16_t* ioTransIdP);
status_t	TelGprsGetQosMinimum(int32_t telDesc, TelGprsQosPtr ioQosMinimumP, uint16_t* ioTransIdP);
status_t	TelGprsGetQosCurrent(int32_t telDesc, TelGprsQosPtr ioQosCurrentP, uint16_t* ioTransIdP);
status_t	TelGprsSetEventReporting(int32_t telDesc, TelGprsEventReportingPtr iEvtReportP, uint16_t* ioTransIdP);
status_t	TelGprsGetEventReporting(int32_t telDesc, TelGprsEventReportingPtr oEvtReportP, uint16_t* ioTransIdP);
status_t	TelGprsGetDataCounter(int32_t telDesc, TelGprsDataCounterPtr oDataCounterP, uint16_t* ioTransIdP);

// Card Application Toolkit
status_t	TelCardGetFile(int32_t iTelDesc, TelCardFileType* ioFileP, uint16_t* ioTransIdP);
status_t	TelCatSetConfig(int32_t iTelDesc, TelCatConfigType* iCfgP, uint16_t* ioTransIdP);
status_t	TelCatGetConfig(int32_t iTelDesc, TelCatConfigType* ioCfgP, uint16_t* ioTransIdP);
status_t	TelCatTerminate(int32_t iTelDesc, uint8_t iReason, uint16_t* ioTransIdP);
status_t	TelCatGetCmdParameters(int32_t iTelDesc, TelCatCmdParamsType* ioParamsP, uint16_t* ioTransIdP);
status_t	TelCatSetCmdResponse(int32_t iTelDesc, TelCatCmdResponseType* iResponseP, uint16_t* ioTransIdP);
status_t	TelCatMenuSelection(int32_t iTelDesc, TelCatMenuSelectionType* iSelectionP, uint16_t* ioTransIdP);
status_t	TelCatCallAction(int32_t iTelDesc, uint8_t iAction, uint16_t* ioTransIdP);
status_t	TelCatNotifyCardOfEvent(int32_t iTelDesc, TelCatEventToCardType* iEventP, uint16_t* ioTransIdP);

// Mux
status_t	TelMuxEnable(int32_t telDesc, uint8_t iStatus, uint16_t* ioTransIdP);
status_t	TelMuxChanAllocate(int32_t telDesc, TelMuxChanPtr ioChanP, uint16_t* ioTransIdP);
status_t	TelMuxChanFree(int32_t telDesc, TelMuxChanPtr iChanP, uint16_t* ioTransIdP);
status_t	TelMuxChanSetId(int32_t telDesc, uint32_t iChanId, uint16_t* ioTransIdP);

// MACROS for checking service and function availability
#define TelIsCncServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelCncServiceId)
#define TelIsNwkServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelNwkServiceId)
#define TelIsStyServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelStyServiceId)
#define TelIsPowServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelPowServiceId)
#define TelIsCfgServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelCfgServiceId)
#define TelIsSmsServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelSmsServiceId)
#define TelIsEmcServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelEmcServiceId)
#define TelIsSpcServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelSpcServiceId)
#define TelIsPhbServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelPhbServiceId)
#define TelIsSndServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelSndServiceId)
#define TelIsInfServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelInfServiceId)
#define TelIsOemServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelOemServiceId)
#define TelIsGprsServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelGprsServiceId)
#define TelIsCatServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelCatServiceId)
#define TelIsMuxServiceAvailable(telDesc)						TelIsServiceAvailable(telDesc, kTelMuxServiceId)

#define TelIsCancelSupported(telDesc)							TelIsFunctionSupported(telDesc, kTelCancelMessage)
#define TelIsTestPhoneDriverSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelTestPhoneDriverMessage)

#define TelIsNwkGetOperatorsSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkGetOperatorsMessage)
#define TelIsNwkGetOperatorSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkGetOperatorMessage)
#define TelIsNwkSetOperatorSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkSetOperatorMessage)
#define TelIsNwkGetPreferredOperatorsSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelNwkGetPreferredOperatorsMessage)
#define TelIsNwkAddPreferredOperatorSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelNwkAddPreferredOperatorMessage)
#define TelIsNwkDeletePreferredOperatorSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelNwkDeletePreferredOperatorMessage)
#define TelIsNwkGetRegistrationModeSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelNwkGetRegistrationModeMessage)
#define TelIsNwkSetRegistrationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelNwkSetRegistrationMessage)
#define TelIsNwkGetTypeSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelNwkGetTypeMessage)
#define TelIsNwkGetStatusSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelNwkGetStatusMessage)
#define TelIsNwkGetProviderIdSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkGetProviderIdMessage)
#define TelIsNwkGetLocationSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkGetLocationMessage)
#define TelIsNwkGetSignalLevelSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelNwkGetSignalLevelMessage)
#define TelIsNwkCheckUssdSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelNwkCheckUssdMessage)
#define TelIsNwkSendUssdSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelNwkSendUssdMessage)
#define TelIsNwkReceiveUssdSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkReceiveUssdMessage)
#define TelIsNwkCancelUssdSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelNwkCancelUssdMessage)

#define TelIsStyGetAuthenticationStatusSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelStyGetAuthenticationStatusMessage)
#define TelIsStyEnterAuthenticationSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelStyEnterAuthenticationMessage)
#define TelIsStyGetFacilitiesSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelStyGetFacilitiesMessage)
#define TelIsStyGetFacilitySupported(telDesc)					TelIsFunctionSupported(telDesc, kTelStyGetFacilityMessage)
#define TelIsStyLockFacilitySupported(telDesc)					TelIsFunctionSupported(telDesc, kTelStyLockFacilityMessage)
#define TelIsStyUnlockFacilitySupported(telDesc)				TelIsFunctionSupported(telDesc, kTelStyUnlockFacilityMessage)
#define TelIsStyChangeFacilityPasswordSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelStyChangeFacilityPasswordMessage)

#define TelIsPowGetBatteryChargeLevelSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelPowGetBatteryChargeLevelMessage)
#define TelIsPowGetBatteryConnectionStatusSupported(telDesc)	TelIsFunctionSupported(telDesc, kTelPowGetBatteryConnectionStatusMessage)
#define TelIsPowSetPhoneFunctionalitySupported(telDesc)			TelIsFunctionSupported(telDesc, kTelPowSetPhoneFunctionalityMessage)

#define TelIsCfgGetSmsCenterSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelCfgGetSmsCenterMessage)
#define TelIsCfgSetSmsCenterSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelCfgSetSmsCenterMessage)
#define TelIsCfgGetPhoneNumberSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgGetPhoneNumberMessage)
#define TelIsCfgSetPhoneNumberSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgSetPhoneNumberMessage)
#define TelIsCfgGetVoiceMailNumberSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelCfgGetVoiceMailNumberMessage)
#define TelIsCfgSetVoiceMailNumberSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelCfgSetVoiceMailNumberMessage)
#define TelIsCfgGetLoudspeakerVolumeLevelRangeSupported(telDesc)TelIsFunctionSupported(telDesc, kTelCfgGetLoudspeakerVolumeLevelRangeMessage)
#define TelIsCfgGetLoudspeakerVolumeLevelSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelCfgGetLoudspeakerVolumeLevelMessage)
#define TelIsCfgSetLoudspeakerVolumeLevelSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelCfgSetLoudspeakerVolumeLevelMessage)
#define TelIsCfgGetRingerSoundLevelRangeSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelCfgGetRingerSoundLevelRangeMessage)
#define TelIsCfgGetRingerSoundLevelSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelCfgGetRingerSoundLevelMessage)
#define TelIsCfgSetRingerSoundLevelSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelCfgSetRingerSoundLevelMessage)
#define TelIsCfgGetVibratorModeSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgGetVibratorModeMessage)
#define TelIsCfgSetVibratorModeSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgSetVibratorModeMessage)
#define TelIsCfgGetAlertSoundModeSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgGetAlertSoundModeMessage)
#define TelIsCfgSetAlertSoundModeSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgSetAlertSoundModeMessage)
#define TelIsCfgGetCallForwardingSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgGetCallForwardingMessage)
#define TelIsCfgSetCallForwardingSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCfgSetCallForwardingMessage)
#define TelIsCfgGetCallIdRestrictionStatusSupported(telDesc)	TelIsFunctionSupported(telDesc, kTelCfgGetCallIdRestrictionStatusMessage)
#define TelIsCfgSetCallIdRestrictionStatusSupported(telDesc)	TelIsFunctionSupported(telDesc, kTelCfgSetCallIdRestrictionStatusMessage)

#define TelIsSmsSendMessageSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsSendMessageMessage)
#define TelIsSmsReadMessagesSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsReadMessagesMessage)
#define TelIsSmsReadMessageSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsReadMessageMessage)
#define TelIsSmsDeleteMessageSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsDeleteMessageMessage)
#define TelIsSmsGetUniquePartIdSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelSmsGetUniquePartIdMessage)
#define TelIsSmsGetDataMaxSizeSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelSmsGetDataMaxSizeMessage)
#define TelIsSmsGetStoragesSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsGetStoragesMessage)
#define TelIsSmsGetStorageSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsGetStorageMessage)
#define TelIsSmsSetStorageSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSmsSetStorageMessage)

#define TelIsEmcDialSupported(telDesc)							TelIsFunctionSupported(telDesc, kTelEmcDialMessage)
#define TelIsEmcHangUpSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelEmcHangUpMessage)

#define TelIsSpcGetCallsSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelSpcGetCallsMessage)
#define TelIsSpcGetCallSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelSpcGetCallMessage)
#define TelIsSpcInitiateCallSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSpcInitiateCallMessage)
#define TelIsSpcAcceptCallSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSpcAcceptCallMessage)
#define TelIsSpcReleaseCallSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSpcReleaseCallMessage)
#define TelIsSpcHoldActiveCallsSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelSpcHoldActiveCallsMessage)
#define TelIsSpcAddHeldCallSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSpcAddHeldCallMessage)
#define TelIsSpcPrivateCallSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSpcPrivateCallMessage)
#define TelIsSpcPlayToneSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelSpcPlayToneMessage)
#define TelIsSpcGetToneDurationRangeSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelSpcGetToneDurationRangeMessage)
#define TelIsSpcGetToneDurationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelSpcGetToneDurationMessage)
#define TelIsSpcSetToneDurationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelSpcSetToneDurationMessage)

#define TelIsPhbGetPhonebooksSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelPhbGetPhonebooksMessage)
#define TelIsPhbGetPhonebookSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelPhbGetPhonebookMessage)
#define TelIsPhbSetPhonebookSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelPhbSetPhonebookMessage)
#define TelIsPhbGetEntriesSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelPhbGetEntriesMessage)
#define TelIsPhbGetEntrySupported(telDesc)						TelIsFunctionSupported(telDesc, kTelPhbGetEntryMessage)
#define TelIsPhbAddEntrySupported(telDesc)						TelIsFunctionSupported(telDesc, kTelPhbAddEntryMessage)
#define TelIsPhbDeleteEntrySupported(telDesc)					TelIsFunctionSupported(telDesc, kTelPhbDeleteEntryMessage)

#define TelIsSndGetMuteStatusSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSndGetMuteStatusMessage)
#define TelIsSndSetMuteStatusSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelSndSetMuteStatusMessage)

#define TelIsInfGetIdentificationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelInfGetIdentificationMessage)
#define TelIsInfGetCallsDurationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelInfGetCallsDurationMessage)
#define TelIsInfResetCallsDurationSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelInfResetCallsDurationMessage)
#define TelIsInfGetCallsListSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelInfGetCallsListMessage)
#define TelIsInfResetCallsListSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelInfResetCallsListMessage)

#define TelIsOemCallSupported(telDesc)							TelIsFunctionSupported(telDesc, kTelOemCallMessage)

#define TelIsGprsGetAvailableContextIdSupported(telDesc)		TelIsFunctionSupported(telDesc, kTelGprsGetAvailableContextIdMessage)
#define TelIsGprsSetContextSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelGprsSetContextMessage)
#define TelIsGprsGetContextSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelGprsGetContextMessage)
#define TelIsGprsGetDefinedCidsSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetDefinedCidsMessage)
#define TelIsGprsSetAttachSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelGprsSetAttachMessage)
#define TelIsGprsGetAttachSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelGprsGetAttachMessage)
#define TelIsGprsSetPdpActivationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsSetPdpActivationMessage)
#define TelIsGprsGetPdpActivationSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetPdpActivationMessage)
#define TelIsGprsGetPdpAddressSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetPdpAddressMessage)
#define TelIsGprsSetNwkRegistrationSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelGprsSetNwkRegistrationMessage)
#define TelIsGprsGetNwkRegistrationSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelGprsGetNwkRegistrationMessage)
#define TelIsGprsSetSmsServiceSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsSetSmsServiceMessage)
#define TelIsGprsGetSmsServiceSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetSmsServiceMessage)
#define TelIsGprsSetQosRequestedSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsSetQosRequestedMessage)
#define TelIsGprsGetQosRequestedSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetQosRequestedMessage)
#define TelIsGprsSetQosMinimumSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsSetQosMinimumMessage)
#define TelIsGprsGetQosMinimumSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetQosMinimumMessage)
#define TelIsGprsGetQosCurrentSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetQosCurrentMessage)
#define TelIsGprsSetEventReportingSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelGprsSetEventReportingMessage)
#define TelIsGprsGetEventReportingSupported(telDesc)			TelIsFunctionSupported(telDesc, kTelGprsGetEventReportingMessage)
#define TelIsGprsGetDataCounterSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelGprsGetDataCounterMessage)

#define TelIsCardGetFileSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelCardGetFileMessage)
#define TelIsCatSetConfigSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelCatSetConfigMessage)
#define TelIsCatGetConfigSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelCatGetConfigMessage)
#define TelIsCatTerminateSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelCatTerminateMessage)
#define TelIsCatGetCmdParametersSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCatGetCmdParametersMessage)
#define TelIsCatSetCmdResponseSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCatSetCmdResponseMessage)
#define TelIsCatMenuSelectionSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelCatMenuSelectionMessage)
#define TelIsCatCallActionSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelCatCallActionMessage)
#define TelIsCatNotifyCardOfEventSupported(telDesc)				TelIsFunctionSupported(telDesc, kTelCatNotifyCardOfEventMessage)

#define TelIsMuxEnableSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelMuxEnableMessage)
#define TelIsMuxChanAllocateSupported(telDesc)					TelIsFunctionSupported(telDesc, kTelMuxChanAllocateMessage)
#define TelIsMuxChanFreeSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelMuxChanFreeMessage)
#define TelIsMuxChanSetIdSupported(telDesc)						TelIsFunctionSupported(telDesc, kTelMuxChanSetIdMessage)


#ifdef __cplusplus
}
#endif

#endif	// TelephonyLib_h
