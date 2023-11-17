/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File:		SmsLib.h
 *
 * Release:		Palm OS 6.0
 *
 * Description:	Include file for PalmOS SMS Library
 *
 * Notes:		The Sms library is used as an Exchange library. ExgLib.h defines all the
 *				primary entrypoints into the library. The rest of this include file defines the
 *				specials opCodes used in the ExgControl function and the structure used in the
 *				socketRef field of the Exchange Manager Socket structure.
 *
 *****************************************************************************/

#ifndef SmsLib_h
#define SmsLib_h


/******************************************************************************
 *	Includes
 *****************************************************************************/

#include <PalmTypes.h>
#include <TelephonyLib.h>


/******************************************************************************
 *	Defines
 *****************************************************************************/

// Name of Sms library
#define kSmsLibName 						"SMS Library"

// ExgLibControl opcodes
#define exgLibSmsPrefGetOp					(exgLibCtlSpecificOp | 1)		// SmsPrefType as parameter
#define exgLibSmsPrefGetDefaultOp			(exgLibCtlSpecificOp | 2)		// SmsPrefType as parameter
#define exgLibSmsPrefSetOp					(exgLibCtlSpecificOp | 3)		// SmsPrefType as parameter
#define exgLibSmsPrefDisplayOp				(exgLibCtlSpecificOp | 4)		// Network type constant as parameter kSmsNetworkXxx
#define exgLibSmsIncompleteGetCountOp		(exgLibCtlSpecificOp | 5)		// uint16_t as parameter
#define exgLibSmsIncompleteDeleteOp			(exgLibCtlSpecificOp | 6)		// SmsID (uint16_t) as parameter

// Registration type
#define kSmsRegExtensionTypeMessage			"sms"
#define kSmsRegExtensionTypeReport			"rps"
#define kSmsRegExtensionTypeFlash			"fhs"
#define kSmsRegExtensionTypeVoiceMailInd	"vwi"
#define kSmsRegExtensionTypeFaxInd			"fwi"
#define kSmsRegExtensionTypeEMailInd		"ewi"
#define kSmsRegExtensionTypeOtherInd		"owi"
#define kSmsRegExtensionTypeLength			3

// SMS scheme
#define kSmsScheme							"_sms"

// Message type
#define	kSmsMessageTypeIn					((uint16_t) 0x0001)
#define	kSmsMessageTypeReport				((uint16_t) 0x0002)
#define	kSmsMessageTypePart					((uint16_t) 0x0004)		// Internal: Used for multipart reassembly
#define	kSmsMessageTypeMultipart			((uint16_t) 0x0008)
#define	kSmsMessageTypeFlash				((uint16_t) 0x0010)
#define	kSmsMessageTypeIndication			((uint16_t) 0x0020)
#define	kSmsMessageTypeIncoming				((uint16_t) 0xFFFF)		// Internal: Used to get incoming message


/******************************************************************************
 *	Structures
 *****************************************************************************/

// SMS Parameters
typedef struct SmsParamsType
{
	uint32_t			creator;									// MUST ALWAYS BE SET TO sysFileCSmsLib

	char*				extension;									// Extension type of the data - Optionel		(Output)
	char*				mimeTypes;									// Mime type of object - Optionel				(Output)
	uint32_t			appCreator;									// Application Creator of the target - Optionel	(Output)

	TelSmsMessageType	message;									// SMS

	uint16_t			requestType;								// Kind of message the application request

	uint16_t			storageId;									// Internal: Used to retrieved a specific incoming message

	Boolean				leaveOnPhone;								// Received messages won't be deleted on the phone (Input)
	Boolean				forceSlotMode;								// Force parsing methode to Slot Mode (default is Block mode) (Input)
	Boolean				ignoreDefaultValue;

	uint8_t				padding[1];
} SmsParamsType, *SmsParamsPtr;


// Preferences
typedef struct SmsPrefType
{
	uint32_t			validity;									// Validity period of SMS (relatif) in seconds
	uint16_t			warnOver;									// Display an alert if sending more Sms than this value.
	Boolean				leaveOnPhone;								// Leave SMS on Phone
	Boolean				report;										// Ask for a network delivery report
	Boolean				autoSmsCenter;								// If set, don't use the value stored in smscNumber field
	uint8_t				padding[3];
	char				smsCenterNumberP[kTelPhoneNumberMaxLen];	// SMS Service Center. Could be null
} SmsPrefType, *SmsPrefPtr;


#endif	// SmsLib_h
