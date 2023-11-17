/*******************************************************************
 *
 * Project:
 *	  Phone Library
 *
 *  Copyright info:
 *	  Copyright (c) 1999-2002 Handspring, Inc. All Rights Reserved.   
 *
 * FileName:
 *	  PhoneLib.c
 *
 * Description:
 *	  Routine and structure of the GSM interface library.
 *******************************************************************/
  
#ifndef __PHONE_LIB_H
#define __PHONE_LIB_H

// Common definitions
#include <PalmOS.h>
#include <PhoneLibTraps.h>
#include <PhoneLibErrors.h>
// include compatibility with pre-3.5 typedefs
#ifndef __CORE_COMPATIBILITY_H__
#include <CoreCompatibility.h>
	// workaround for differing header files in sdk-3.5 and sdk-internal
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 


// If we're actually compiling the library code, then we need to
// eliminate the trap glue that would otherwise be generated from
// this header file in order to prevent compiler errors in CW Pro 2.
#if defined(BUILDING_GSM_LIBRARY) || defined(USING_STATIC_LIBRARY)
	#define PHN_LIB_TRAP(trapNum)
#else
	#define PHN_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif


/********************************************************************
 * Library names, types, etc.
 * Release version of library written by Option.
 ********************************************************************/
#define phnLibDbType			'libr'
#define phnLibDbCreator			'GSM!'
#define phnLibDbName			"GSM Library"	
#define phnLibName				"GSMLibrary.lib"
#define phnRingsDbName			"System Ring Tones"
#define phnLibRingsDBCreatorID	'GSMr'
#define phnVdrvDbName			"PsaVirtualSerDriver"
#define phnVdrvDbType			'vdrv'
#define phnVdrvDbCreator		'Hpsa'
#define phnNBSEvent             'Hnbs'

// Notification Types
#define phnNotifySubscriber 'CLIP'
#define phnNotifyEquipMode	'Heqp'

/********************************************************************
 * Phone library data type
 ********************************************************************/
typedef Word		PhnConnectionID;
typedef ULong		PhnDatabaseID;
typedef VoidHand	PhnAddressHandle;
typedef VoidHand	PhnAddressList;
typedef ULong		GSMOperatorID;


/********************************************************************
 * Phone library errors
 ********************************************************************/
#define	phnErrorClass				0x04900	
#define	phnErrSerLibAlreadyOpen		(phnErrorClass | 1)
#define	phnErrAlreadyOpen			(phnErrorClass | 2)
#define	phnErrNotOpen				(phnErrorClass | 3)
#define	phnErrStillOpen				(phnErrorClass | 4)
#define	phnLibErrBufferTooSmall		(phnErrorClass | 5)
#define	phnLibCardNotFound			(phnErrorClass | 6)

#define	phnErrUnknownID				(phnErrorClass | 7)
#define	phnErrParseError			(phnErrorClass | 8)
#define phnErrIntermediateResult	(phnErrorClass | 9)
#define phnErrIncorrectPassword		(phnErrorClass | 10)


/********************************************************************
 * Phone library constants
 ********************************************************************/
#define	kMaxPhoneNumberLen	16

// Phone slider switch
#define kSliderLow			0
#define kSliderHigh			1
#define kSliderPositions	2

// Maximum length of a ringer name (string resource)
#define kMaxRingName		16

#define phnLibUnknownID		    0xff000000
#define phnLibNoLookupNeededID	0xff000001

#define phnVolumeMin		0
#define phnVolumeMax		7

// Phone Call Status flags
#define phnVoiceCall1Active	  0x0001 // There is a voice call active on line1
#define phnVoiceCall2Active	  0x0002 // There is a voice call active on line2
#define phnCSDCallActive	  0x0004 // There is a data call currently active
										 // Note: Actually the virtual modem has
										 //	control but does not neccessarily
										 //	have an active data call
#define phnGPRSCallActive	  0x0008 // There is a GPRS session active

// Reserved GPRS Context ID's

// The context ID reserved for the net pref panel to use
// for the default wireless connection. Dont use this id
// unless you really know what you are doing
#define phnNetPrefGPRSPDPContextID  1

/*******************************************************************
 * Phone application's custom action code and data structures.
 *******************************************************************/

// Notification of a phone event
#define	phnLibLaunchCmdEvent	 	0x2bad	// zzz need to reserve a system launch code
#define phnLibLaunchCmdRegister		0x2bae	


// Phone Provider Configuration
//
// The phone provider can configure the phone application to
// enable / disable the following feature:
// 
//	 o require password to turn on phone
//	 o call waiting disabling
//	 o caller ID blocking
//	 o outgoing call blocking
//	 o incoming call blocking
//	 o require password to block calls
//	 o call forwarding (unconditional)
//	 o call forwarding when phone is busy
//	 o call forward when call is missed
//	 o call forwarding when phone is off
//	 o voicemail support
//	 o voicemail number is editable
#define phnConfigPowerOnPassword		0x0001
#define phnConfigCallWaiting			0x0002
#define phnConfigCallerID				0x0004
#define phnConfigBlockOutgoing			0x0008
#define phnConfigBlockIncoming			0x0010
#define phnConfigBlockingPassword		0x0020
#define phnConfigForward				0x0040
#define phnConfigForwardBusy			0x0080
#define phnConfigForwardMissed			0x0100
#define phnConfigForwardOff				0x0200
#define phnConfigVoicemail				0x0400
#define phnConfigVoicemailEditable		0x0800


/********************************************************************
 * Minimum and maximum password lengths.
 ********************************************************************/
#define minPasswordLen					4
#define maxPasswordLen					8

/********************************************************************
 * Enumeration types
 ********************************************************************/

// Phone Connection Type
typedef enum
 {
  voiceConnection	= 1,
  csdConnection		= 2,
  gprsConnection    = 3
}
PhnConnectionEnum;

// Phone service types
typedef enum
 {
	kVoice			= 1, 
	phnServeData	= 2, 
	phnServeTelefax	= 4
} PhnServiceType;



// Phone "equipment modes"
typedef enum {
	phnHandsetMode 		= 0, 
	phnSpeakerPhoneMode	= 1, 
	phnCarKitMode		= 2,
	phnHeadsetMode		= 3,
	phnHandsetLidCloseMode 	= 4
} PhnEquipmentMode;


// Phone event types
typedef enum {
    phnEvtCardInsertion,
	phnEvtRegistration,
	phnEvtError,
	phnEvtKeyPress,								// Phone or data button pressed.							
	phnEvtPower,
	phnEvtPassword,
	phnEvtProgress,
	phnEvtIndication,
	phnEvtConnectInd,							//	Incoming call
	phnEvtConnectConf,							// ConnectConf,
	phnEvtSubscriber,
	phnEvtDisconnectInd,
	phnEvtDisconnectConf,
	phnEvtBusy,
	phnEvtUpdate,
	phnEvtConference,
	phnEvtVoiceMail,
		
	phnEvtMessageInd,							// Events used by the SMS library
	phnEvtSegmentInd,
	phnEvtMessageStat,
	phnEvtMessageDel,
	phnEvtMessageMoved,

	phnEvtSATNotification,						// Events used by the SIM Application Toolkit

	// Events added after GSMLibrary went public
	phnEvtUSSDInd,
	phnEvtPhoneEquipmentMode,
	phnEvtGPRSRegistration,

	// Events used by MMS
	phnEvtMMSInd

} PhnEventCode;


// Phone service classes
typedef enum {
	phnServiceVoice		 = 1,
	phnServiceSMS		 = 2,
	phnServiceTelefax	 = 4,
	phnServiceData		 = 8,
	phnServiceMail		 = 16,
	phnServiceSIMToolkit = 32,
	phnServiceAll		= phnServiceVoice | phnServiceSMS | phnServiceTelefax | phnServiceData | phnServiceMail | phnServiceSIMToolkit
} PhoneServiceClassType;


// Phone connection state
typedef enum {
	phnConnectionActive,
	phnConnectionHeld,
	phnConnectionDialing,
	phnConnectionAlerting,
	phnConnectionIncoming,
	phnConnectionWaiting,
	phnConnectionUnknown
} PhnConnectStateType;

// Phone password types
typedef enum {
	phnPasswordUnknown,					// FAULT or none of the strings below
	// See GSM 07.07, section 8.3 for a list of passwords. The following
	// list is based on on version 6.2.0 of the recommendation.
	phnPasswordNone,					// READY
	phnPasswordSIMPIN,					// SIM PIN
	phnPasswordSIMPUK,					// SIM PUK
	phnPasswordPhSIMPIN,				// PH-SIM PIN
	phnPasswordPh1SIMPIN,				// PH-FSIM PIN
	phnPasswordPh1SIMPUK,				// PH-FSIM PUK
	phnPasswordSIMPIN2,					// SIM PIN2
	phnPasswordSIMPUK2,					// SIM PUK2
		// It is assumed that all PINs and PUKs below are network-based
		// PINS and PUKs. Function Change() uses a different timeout for
		// such PINs and/or PUKs.
	phnPasswordNetworkPIN,				// PH-NET PIN
	phnPasswordNetworkPUK,				// PH-NET PUK
	phnPasswordNetworkSubsetPIN,		// PH-NETSUB PIN
	phnPasswordNetworkSubsetPUK,		// PH-NETSUB PUK
	phnPasswordServiceProviderPIN,		// PH-SP PIN
	phnPasswordServiceProviderPUK,		// PH-SP PUK
	phnPasswordCorporatePIN,			// PH-CORP PIN
	phnPasswordCorporatePUK,			// PH-CORP PUK
		// The passwords below are supported by the +CPWD command. See GSM 07.07
		// section 7.4 and 7.5 for a list of facility-specific passwords.
	phnPasswordBarrAO,				// all outgoing call
	phnPasswordBarrOI,				// outgoing int’l calls
	phnPasswordBarrOX,				// outgoing int’l calls except to home country
	phnPasswordBarrAI,				// all incoming calls
	phnPasswordBarrIR,				// incoming calls when roaming outside home country
	phnPasswordBarrAB,				// all barring services
	phnPasswordBarrAG,				// all outgoing barring services
	phnPasswordBarrAC					// all incoming barring services
} PhnPasswordType;


// Phone operator status
typedef enum {
	phnOpUnknown, 
	phnOpAvailable, 
	phnOpCurrent, 
	phnOpForbidden
} PhnOperatorStatus;


// Phone address fields
typedef enum {
	phnAddrFldPhone,
	phnAddrFldFirstName,
	phnAddrFldLastName
} PhnAddressField;


// Volume of phone ringer
typedef enum { 
	phnRingerLoud	= 7, 
	phnRingerSoft	= 1, 
	phnRingerOff	= 0
} PhnRingerVolumeType;


// Vibrate mode while ringing
typedef enum { 
	phnVibrateOff, 
	phnVibrateOn
} PhnVibrateType;


// Phone buttons
typedef enum  {
	phnButtonPhoneApp,
	phnButtonDataApp,
	phnButtonHeadset
} PhnModuleButtonType;


// Phone button modifers
typedef enum {
	phnButtonPowerOnMask = 0x8000,	// button cause device to power on
	phnButtunUpMask		 = 0x4000,	// the button is released
	phnButtonHeld		 = 0x1000	// the button is held down
} PhnModuleButtonModifersType;


typedef enum  {
	kBoxVoice, kBoxTelefax, kBoxEMail, kBoxOther, kBoxData
	// other values reserved for future expansion
}PhnMsgBoxType;


typedef enum  {
	phnPowerOff, 
	phnPowerOn, 
	phnPowerStartCharging, 
	phnPowerStopCharging, 
	phnPowerLow
} PhnPowerType;


typedef enum  {
	kOpenDialog, 
	kCloseDialog, 
	kSetText, 
	kSetRecipient, 
	kShowSegment
} PhnProgressType;


typedef enum  {
	kDialogSetBarring, kDialogGetBarring,
	kDialogSetForwarding, kDialogGetForwarding,
	kDialogSetCallWaiting, kDialogGetCallWaiting,
	kDialogGetOperatorList, kDialogSetOperator,
	kDialogOperatorSelection,
	kDialogImmediateSend, kDialogDeferredSend,
	kDialogSendUSSD
} PhnOpenDialogType;



typedef enum {
	clirNotProvisioned,			// sent: restricted presentation of the calling line
	clirProvisioned,			// not sent: don't restrict presentation of the calling line
	clirUnknown,				// status not available
	clirTemporaryRestricted,	// not sent, override allowed
	clirTemporaryAllowed		// sent: override allowed
} PhnCLIRStatus;


typedef enum  {
	gsmDialCLIRDefault, 
	gsmDialCLIRTemporaryInvocation, 
	gsmDialCLIRTemporarySuppression
} GSMDialCLIRMode;


typedef enum  {
	registrationNone, registrationHome, registrationSearch, registrationDenied,
	registrationUnknown, registrationRoaming
} PhnRegistrationStatus;


typedef enum {
	indicationSIMReady, 
	indicationSIMMessages, 
	indicationNetworkSearch,
	indicationPasswordAccepted,
	indicationNetworkAvailable	
	} PhnIndicationKind;

typedef enum  {
	gsmMessagesConfirmMove, gsmMessagesCantReceive
} GSMSIMMessagesDialogKind;

typedef enum  {
	simMissing, simFailure, simWrong, simNotReady, simReady,
	simUnknown, simPresent		
} GSMSIMStatus;


typedef enum {
	gsmRegModeAutomatic, gsmRegModeManual, gsmRegModeDeregister, gsmRegModeFormat,
	gsmRegModeManualAutomatic
} GSMRegistrationMode;

typedef enum PhnGPRSPDPType
{
  phnGPRSPDPTypeIP = 1,
  phnGPRSPDPTypePPP = 2,
} PhnGPRSPDPType;

typedef enum PhnGPRSQOSType
{
  kGPRSQOSTypeRequired = 1,
  kGPRSQOSTypeMinimum = 2,
} PhnGPRSQOSType;
/********************************************************************
 * Phone library structures
 ********************************************************************/

// Operator info
typedef struct {
	PhnOperatorStatus	status;
	GSMOperatorID		id;
	CharPtr				longname;
	CharPtr				shortname;
} PhnOperatorType;


// Operator list
typedef struct  {
	short				count;
	PhnOperatorType		opData[1];
} PhnOperatorListType, * PhnOperatorListPtr;


typedef struct  {
	PhnRegistrationStatus status;
} PhnRegistrationType;


typedef struct  {
	Boolean				indicatorOn;
	PhnMsgBoxType		type;
	Int16				messageCount;
	Int16				lineNumber;
} PhnMsgBoxDataType;


typedef struct  {
	PhnPowerType state;
} PhnPowerEventType;



typedef char PhnPassword[maxPasswordLen+2];

typedef struct  {
	PhnPasswordType		type;
	PhnPasswordType		prevType;
	Err					error;
	PhnPassword			pin;
	PhnPassword			puk;
} PhnPasswordEventType;


typedef struct {
	PhnProgressType		progress;
	PhnOpenDialogType	dialog;	
	DWord				data;		// only for SMS progress
} PhnProgressEventType;


typedef struct   {
	PhnIndicationKind kind;
	char filler;
	union
		{
		struct
			{
			Boolean state;
			} simReady;
		struct 
			{
			GSMSIMMessagesDialogKind dialog;
			Boolean moveMessages;
			} simMessages;
		struct	{
			PhnPasswordType type;
			} passwordAccepted;
		struct 
			{
			Boolean state;
			} networkAvailable;
		
	} data;
} PhnIndicationType;


typedef struct {
	PhnDatabaseID msgID;
	DWord msgOwner;
	Err error;
	Byte event;
} PhnMovedMsgDescType;


typedef struct {
	UInt16 count;
	PhnMovedMsgDescType* list;
} PhnMovedMsgsParamsType;

typedef struct {
	long	mode;
} PhnPhoneEquipmentMode;

typedef struct {
	long	result;
	CharPtr	string;
} PhnUSSDEventType;

typedef struct {
	MemHandle	dataH;
	UInt32		notificationType;
} PhnSATEventType;


// Phone event record
typedef struct {
	Byte	/*PhnEventCode*/	eventType;
	Boolean						acknowledge;
	Word						connectionID;
	Word						launchCode;
	Ptr							launchParams;
	union Data {
		struct {
			PhnAddressHandle	caller;		// Address handle
			PhnServiceType		service;
		} info;

		struct {
			PhnConnectionID		call1ID;
			PhnConnectionID		call2ID;
			PhnConnectionID		conferenceID;
		} conference;

		struct {
			Err					code;
			ULong				id;
		} error;

		struct {
			ULong				id;
			char				oldStatus;
			char				newStatus;
		} params;

		struct {
			PhnModuleButtonType	key;				
			Word				modifiers;			
		} keyPressed;

		PhnRegistrationType		registration;
		PhnMsgBoxDataType		msgBox;
		PhnPowerEventType		power;
		PhnPasswordEventType	password;
		PhnProgressEventType	progress;
		PhnIndicationType		indication;
        PhnMovedMsgsParamsType  moved;
		PhnUSSDEventType		ussd;
		PhnSATEventType			sat;
		PhnPhoneEquipmentMode		phoneEquipmentMode;
	} data;
} PhnEventType, * PhnEventPtr;


// Phone connection info
typedef struct  {
	PhnConnectionID			id;
	PhnConnectStateType		state;
	PhnServiceType			service;
	Boolean					incoming;
	Boolean					multiparty;
	PhnAddressHandle		address;
	DWord					owner;
} PhnConnectionType, * PhnConnectionPtr;



// Phone slider switch setting
typedef struct {
	UInt32				ringerID;	  // unique ID of ringer record
	Word				volume;
	Boolean				vibrate;
} PhnRingingProfileType;

typedef struct {
	PhnRingingProfileType profiles[kSliderPositions];
} PhnRingingInfoType, * PhnRingingInfoPtr;


typedef struct  {
	Word	firstEntry;
	Word	lastEntry;
	Word	maxNameLength;
	Word	maxNumberLength;
} PhnPhoneBookInfoType, *PhnPhoneBookInfoPtr;
	
/* Structure passed to the callbacks registered for incoming NBS notifications */
typedef struct _NBSNotificationEventTag
{
	UInt16 version;  /* version number to provide future backwards compatibility */

	/* helper fields */
	Boolean NBSdatagram;  /* flag if it is an NBS datagram */
	Boolean binary;       /* true if binary data */

	void *headerP;    /* pointer to raw header */
	UInt8 headerLen;  /* length of headerP */
	void *dataP;      /* pointer to data body */
	UInt8 dataLen;    /* length of dataP */

	/* NBS datagram fields */
	UInt8 refNum;    /* NBS reference number */
	UInt8 maxNum;    /* max segment number 1-255 */
	UInt8 seqNum;    /* sequence number    1-255, no more than maxNum */
	Int8  reserved1; /* padding */

	UInt32 srcPort;  /* source port */
	UInt32 dstPort;  /* destination port */

	/* SMS related fields */
	UInt32 msgID; /* ID into the SMS database to reference this
                  * message this ID is not gauranteed to be
                  * valid once the notification callback
                  * returns.  Users should make a copy of the
                  * msg if they want to work on it after the
                  * callback returns.
                  */

	char   *senderP;   /* sender number - null terminated */
	UInt32 datetime;   /* date/time stamp */
	Int32  reserved2;  /* reserved*/
	Int32  reserved3;  /* reserved*/
}
PhnNBSNotificationEventType;

// Used when we get a list of context that are currently
// stored on the radio.
typedef struct PhnGPRSPDPContextType
{
  UInt16 contextId;
  PhnGPRSPDPType pdpType;
  char*	 apn;
  char*	 pdpAddr;

  // Values that control compression options
  // 0 -- off
  // 1 -- on
  UInt16 pdpDataCompression;
  UInt16 pdpHdrCompression;
} PhnGPRSPDPContextType;

// This is a list node that is returened when we
// retreive the current list of PDP contexts stored
// on the device
typedef struct PhnGPRSPDPContextListNodeType
{
  struct PhnGPRSPDPContextType type;
  struct PhnGPRSPDPContextListNodeType* nextP;
} PhnGPRSPDPContextListNodeType;


/********************************************************************
 * Lock Facitity Constances for PhnLibGetOperatorLock()
 ********************************************************************/
#define		GSMLockSelectorOperatorLock		('PN')
#define		GSMLockSelectorProviderLock		('PP')

/********************************************************************
 * Phone library functions
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// Acquires and opens a serial port.
extern Err PhnLibOpen(UInt refNum)
				SYS_TRAP(sysLibTrapOpen);
				
// Closes the serial connection previously opened with SerOpen.
extern Err PhnLibClose(UInt refNum)
				SYS_TRAP(sysLibTrapClose);

// Puts serial library to sleep
extern Err PhnLibSleep(UInt refNum)
				SYS_TRAP(sysLibTrapSleep);

// Wake Serial library
extern Err PhnLibWake(UInt refNum)
				SYS_TRAP(sysLibTrapWake);


extern Err PhnLibGetLibAPIVersion(UInt refNum, DWordPtr dwVerP)
				PHN_LIB_TRAP(PhnLibTrapGetLibAPIVersion);

extern Err PhnLibRegister(UInt refNum, DWord creator, UInt services)
				PHN_LIB_TRAP(PhnLibTrapRegister);

//extern DWord PhnLibFirstAppForService(UInt refNum, PhnEventClass service)
//				PHN_LIB_TRAP(PhnLibTrapFirstAppForService);

extern PhnAddressHandle PhnLibNewAddress (UInt refNum, CharPtr number, PhnDatabaseID id)
					PHN_LIB_TRAP(PhnLibTrapNewAddress);


extern CharPtr PhnLibGetField (UInt refNum, PhnAddressHandle address, PhnAddressField field)
					PHN_LIB_TRAP(PhnLibTrapGetField);

extern Err PhnLibSetField (UInt, PhnAddressHandle address, PhnAddressField field, CharPtr data)
					PHN_LIB_TRAP(PhnLibTrapSetField);

extern ULong PhnLibGetPhoneBookIndex(UInt, PhnAddressHandle address)
					PHN_LIB_TRAP(PhnLibTrapGetPhoneBookIndex);

extern Err PhnLibSendDTMF (UInt refNum, CharPtr sequence)
					PHN_LIB_TRAP(PhnLibTrapSendDTMF);
													 
extern Err PhnLibSendSilentDTMF (UInt refNum, CharPtr sequence)
					PHN_LIB_TRAP(PhnLibTrapSendSilentDTMF);

extern Err PhnLibPlayDTMF (UInt refNum, CharPtr sequence)
					PHN_LIB_TRAP(PhnLibTrapPlayDTMF);

extern Err PhnLibGetPhoneCallStatus(UInt refNum, UInt32* phnFlags)
					PHN_LIB_TRAP(PhnLibTrapGetPhoneCallStatus);

extern Err PhnLibConnectionAvailable(UInt refNum, PhnConnectionEnum connection)
					PHN_LIB_TRAP(PhnLibTrapConnectionAvailable);

extern Err PhnLibGetCLIP(UInt16 refNum, Boolean* enabled)
				PHN_LIB_TRAP(PhnLibTrapGetCLIP);

extern Err PhnLibGetCLIR (UInt16 refNum, GSMDialCLIRMode* mode, PhnCLIRStatus* status)
				PHN_LIB_TRAP(PhnLibTrapGetCLIR);

extern Err PhnLibSetOperatorLock(UInt refNum, UInt16 facilityType, Boolean enable, CharPtr password)
				PHN_LIB_TRAP(PhnLibTrapSetOperatorLock);

extern Err PhnLibGetOperatorLock(UInt refNum, UInt16 facilityType, Boolean* enabled)
				PHN_LIB_TRAP(PhnLibTrapGetOperatorLock);

extern Err PhnLibHomeOperatorID (UInt16 refNum, char *buffer, Int16* bufferSizeP)
				PHN_LIB_TRAP(PhnLibTrapHomeOperatorID);

extern Err PhnLibCurrentOperatorID (UInt16 refNum, char *buffer, Int16* bufferSizeP)
				PHN_LIB_TRAP(PhnLibTrapCurrentOperatorID);

extern Err PhnLibCurrentOperator (UInt refNum, GSMOperatorID* id, CharPtr* name, GSMRegistrationMode* mode)
				PHN_LIB_TRAP(PhnLibTrapCurrentOperator);

extern Err PhnLibCurrentProvider(UInt refNum, char** name)
				PHN_LIB_TRAP(PhnLibTrapCurrentProvider);

extern Err PhnLibGetOperatorList (UInt refNum, PhnOperatorListPtr * list)
				PHN_LIB_TRAP(PhnLibTrapGetOperatorList);

extern Err PhnLibSetOperator (UInt refNum, PhnOperatorType* op, GSMRegistrationMode	regMode)
				PHN_LIB_TRAP(PhnLibTrapSetOperator);

extern Err PhnLibGetOwnNumbers (UInt refNum, PhnAddressList* ownNumbers)
				PHN_LIB_TRAP(PhnLibTrapGetOwnNumbers);

extern Err PhnLibGetPhoneBook (UInt refNum, PhnAddressList* numbers, PhnPhoneBookInfoPtr info)
				PHN_LIB_TRAP(PhnLibTrapGetPhoneBook);

extern Err PhnLibSetPhoneBook (Word refnum, PhnAddressList numbers)
				PHN_LIB_TRAP(PhnLibTrapSetPhoneBook);

extern Err	PhnLibGetRingingInfo (UInt refNum, PhnRingingInfoPtr info)
				PHN_LIB_TRAP(PhnLibTrapGetRingingInfo);

extern Err	PhnLibSetRingingInfo (UInt refNum, PhnRingingInfoPtr info)
					PHN_LIB_TRAP(PhnLibTrapSetRingingInfo);

extern Err	PhnLibGetSMSRingInfo (UInt refNum, PhnRingingInfoPtr info)
				PHN_LIB_TRAP(PhnLibTrapGetSMSRingInfo);

extern Err	PhnLibSetSMSRingInfo (UInt refNum, PhnRingingInfoPtr info)
					PHN_LIB_TRAP(PhnLibTrapSetSMSRingInfo);

extern Err	PhnLibGetSMSGateway (UInt refNum, char** smsGateway)
					PHN_LIB_TRAP(PhnLibTrapGetSMSGateway);
					
extern Err	PhnLibGetToneIDs (UInt refNum, UInt32** list, WordPtr listLength)
					PHN_LIB_TRAP(PhnLibTrapGetToneIDs);

extern Err	PhnLibGetToneName (UInt refNum, UInt16 toneIndex, CharPtr name, Word maxLength)
					PHN_LIB_TRAP(PhnLibTrapGetToneName);

extern Err	PhnLibPlayTone (UInt refNum, UInt32 tone, int volume)
					PHN_LIB_TRAP(PhnLibTrapPlayTone);

extern Err	PhnLibStopTone (UInt refNum)
					PHN_LIB_TRAP(PhnLibTrapStopTone);

extern Err	PhnLibStartVibrate (UInt refNum, Boolean pulse, Boolean repeat)
					PHN_LIB_TRAP(PhnLibTrapStartVibrate);

extern Err	PhnLibStopVibrate (UInt refNum)
					PHN_LIB_TRAP(PhnLibTrapStopVibrate);

extern Err	PhnLibGetMicrophone (UInt refNum, int* gain)
					PHN_LIB_TRAP(PhnLibTrapGetMicrophone);

extern Err PhnLibSetMicrophone (UInt refNum, int gain)
				PHN_LIB_TRAP(PhnLibTrapSetMicrophone);

extern Err PhnLibGetVolume (UInt refNum, int* volume)
				PHN_LIB_TRAP(PhnLibTrapGetVolume);

extern Err PhnLibSetVolume (UInt refNum, int volume)
				PHN_LIB_TRAP(PhnLibTrapSetVolume);

extern Err PhnLibGetEchoCancellation(UInt refNum, Boolean* echoCancellationOn)
				PHN_LIB_TRAP(PhnLibTrapGetEchoCancellation);
				
extern Err PhnLibSetEchoCancellation(UInt refNum, Boolean echoCancellationOn)
				PHN_LIB_TRAP(PhnLibTrapSetEchoCancellation);

extern Err PhnLibGetEquipmentMode(UInt refNum, PhnEquipmentMode* equipmentMode)
				PHN_LIB_TRAP(PhnLibTrapGetEquipmentMode);
				
extern Err PhnLibSetEquipmentMode(UInt refNum, PhnEquipmentMode equipmentMode)
				PHN_LIB_TRAP(PhnLibTrapSetEquipmentMode);

extern Boolean PhnLibRegistered (UInt refNum)
					PHN_LIB_TRAP(PhnLibTrapRegistered);

extern Boolean PhnLibNetworkAvailable (UInt refNum)
					PHN_LIB_TRAP(PhnLibTrapNetworkAvailable);

extern Boolean PhnLibRoaming(UInt refNum)
				PHN_LIB_TRAP(PhnLibTrapRoaming);

extern Err	PhnLibSignalQuality (UInt refNum, WordPtr guality)
					PHN_LIB_TRAP(PhnLibTrapSignalQuality);

extern Err	PhnLibUsableSignalStrengthThreshold (UInt refNum, WordPtr threshold)
					PHN_LIB_TRAP(PhnLibTrapUsableSignalStrengthThreshold);

extern Err	PhnLibErrorRate (UInt refNum, WordPtr errorRate)
					PHN_LIB_TRAP(PhnLibTrapErrorRate);

extern Err	PhnLibBattery (UInt refNum, WordPtr battery)
					PHN_LIB_TRAP(PhnLibTrapBattery);

extern Err	PhnLibBatteryCharge (UInt refNum, WordPtr charging)
					PHN_LIB_TRAP(PhnLibTrapBatteryCharge);

extern Boolean PhnLibCardInfo (UInt refNum, CharPtr* manufacturer, CharPtr* model, CharPtr* version, CharPtr* serial)
					PHN_LIB_TRAP(PhnLibTrapCardInfo);

extern Boolean PhnLibSIMInfo (UInt refNum, CharPtr* imsi)
				PHN_LIB_TRAP(PhnLibTrapSIMInfo);

extern GSMSIMStatus PhnLibGetSIMStatus(UInt refNum)
				PHN_LIB_TRAP(PhnLibTrapGetSIMStatus);


extern void PhnLibGetErrorText (UInt refNum, Err error, CharPtr buffer, Word bufferLen)
					PHN_LIB_TRAP(PhnLibTrapGetErrorText);


extern PhnAddressList PhnLibNewAddressList (UInt refNum)
					PHN_LIB_TRAP(PhnLibTrapNewAddressList);

extern Err PhnLibAddAddress(UInt refNum, PhnAddressList list, PhnAddressHandle address)
					PHN_LIB_TRAP(PhnLibTrapAddAddress);


extern Err	PhnLibGetNth (UInt refNum, PhnAddressList list, int index, PhnAddressHandle* address)
					PHN_LIB_TRAP(PhnLibTrapGetNth);

extern Err PhnLibCount(UInt refNum, PhnAddressList list, WordPtr count)
					PHN_LIB_TRAP(PhnLibTrapCount);

extern Boolean PhnLibModuleButtonDown(UInt refNum, PhnModuleButtonType button)
				PHN_LIB_TRAP(PhnLibTrapModuleButtonDown);

extern Boolean PhnLibModulePowered(UInt refNum)
				PHN_LIB_TRAP(PhnLibTrapModulePowered);

extern Err PhnLibSetModulePower (UInt refNum, Boolean on)
					PHN_LIB_TRAP(PhnLibTrapSetModulePower);

extern Err PhnLibBoxInformation(UInt refNum, PhnMsgBoxDataType* data)
				PHN_LIB_TRAP(PhnLibTrapBoxInformation);

extern Err PhnLibGetBoxNumber(UInt refNum, PhnMsgBoxType type, Word line, CharPtr * number)
				PHN_LIB_TRAP(PhnLibTrapGetBoxNumber);

// SMS text-processing functions
extern Boolean PhnLibIsLegalCharacter(UInt16 refNum, char c)
				PHN_LIB_TRAP(PhnLibTrapIsLegalCharacter);

extern int PhnLibLength(UInt16 refNum, const char* text, Boolean inMessages, Boolean substitution)
				PHN_LIB_TRAP(PhnLibTrapLength);

// Only for card setup application's use.
extern void PhnLibUninstall (UInt refNum)
				PHN_LIB_TRAP(PhnLibTrapUninstall);

// Use this to get the preferred app for a given service...
extern UInt32 PhnLibFirstAppForService(UInt16 refNum, PhoneServiceClassType service)
				PHN_LIB_TRAP(PhnLibTrapFirstAppForService);


extern Boolean PhnLibGPRSAttached (UInt refNum)
					PHN_LIB_TRAP(PhnLibTrapGPRSAttached);

extern Err PhnLibGPRSPDPContextDefine(UInt16 refNum, 
	UInt16 contextId, 
	PhnGPRSPDPType pdpType, const char* apn, UInt16 apnLen, 
	const char* pdpAddr, UInt16 pdpAddrLen, 
	UInt16 pdpDataCompression, UInt16 pdpHdrCompression)
				PHN_LIB_TRAP(PhnLibTrapGPRSPDPContextDefine);

extern Err PhnLibGPRSPDPContextListConstruct(UInt16 refNum, struct PhnGPRSPDPContextListNodeType** listHeadPP)
			    PHN_LIB_TRAP(PhnLibTrapGPRSPDPContextListConstruct);

extern Err PhnLibGPRSPDPContextListDestruct(UInt16 refNum, struct PhnGPRSPDPContextListNodeType** headNodePP)
				PHN_LIB_TRAP(PhnLibTrapGPRSPDPContextListDestruct);

extern Err PhnLibGPRSQualityOfServiceGet(UInt16 refNum, PhnGPRSQOSType qosType, UInt16 contextId, 
	UInt16* precedence, UInt16* delay, UInt16* reliability, UInt16* peakThroughput,
	UInt16* meanThroughput)
				PHN_LIB_TRAP(PhnLibTrapGPRSQualityOfServiceGet);


#ifdef __cplusplus
}
#endif

	
#endif	//__PHONE_LIB_H
