/**
 * \file SMSLibrary.h
 *
 * Handspring Phone Library
 *
 * Public Interface for the SMS portion of the Phone Library
 *
 * \license
 *
 * Copyright (c) 2002 Handspring Inc. & Option International
 * All Rights Reserved
 *
 * $Id: $
 *
 ***************************************************************/

#ifndef _SMSLIBRARY_H_
#define _SMSLIBRARY_H_

// Common definitions
#include <PalmOS.h>
#include <PhoneLibTraps.h>
#include <PhoneLibErrors.h>

// If we're actually compiling the library code, then we need to
// eliminate the trap glue that would otherwise be generated from
// this header file in order to prevent compiler errors in CW Pro 2.
#if defined(BUILDING_GSM_LIBRARY) || defined(USING_STATIC_LIBRARY)
	#define PHN_LIB_TRAP(trapNum)
#else
	#define PHN_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif


/********************************************************************
 * Constants
 ********************************************************************/
#define SMSLibDBTypeID		  'Msgs'				// Type for the message database
#define SMSLibDBName		  "SMS Messages"		// passed to DmFindDatabase()

/********************************************************************
 * Phone library data type
 ********************************************************************/
typedef MemHandle SMSMessageHandle;

/*******************************************************************
 * Enumeration types
 *******************************************************************/

typedef enum
  {
	kMTIncoming, kMTOutgoing
  }
  SMSMessageType;

typedef enum
  {
	kNone,
	kReceiving, kReceived,
	kPending, kSending, kSent
  }
  SMSMessageStatus;

typedef enum
  {
	kGreekSymbols = 1L << 0,
	kMissingPart =	1L << 1,
	kAutoDelete =	1L << 2,
	kNotification = 1L << 3,
	kDontEncode =	1L << 4,
	kSubstitution = 1L << 5,
	kFailed =		1L << 6,
	kStatusReport = 1L << 7,
	kFreeReply =	1L << 8,
	kInternetEMail =1L << 9,
	kTextSegments =	1L << 10,
	kSMSErrorType1 = 1L << 11,
	kSMSErrorType2 = 1L << 12,
	kSMSErrorType3 = 1L << 13,
	// application-level flags
	kRead = 1L << 16
  }
  SMSMessageFlags;

// Structured types
typedef struct
  {
	Boolean freeReply;
	Boolean statusReport;
	unsigned char validity;
  }
  SMSSendOptions;

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * SMS functions
 ********************************************************************/
extern DmOpenRef PhnLibGetDBRef(UInt16 refNum)
				PHN_LIB_TRAP(PhnLibTrapGetDBRef);

extern Err PhnLibReleaseDBRef(UInt16 refNum, DmOpenRef db)
				PHN_LIB_TRAP(PhnLibTrapReleaseDBRef);

extern PhnDatabaseID PhnLibNewMessage(UInt16 refNum, SMSMessageType type)
				PHN_LIB_TRAP(PhnLibTrapNewMessage);

extern Err PhnLibDeleteMessage(UInt16 refNum, PhnDatabaseID msgID, Boolean archive)
				PHN_LIB_TRAP(PhnLibTrapDeleteMessage);

extern Err PhnLibSendMessage(UInt16 refNum, PhnDatabaseID msgID, Boolean progress)
				PHN_LIB_TRAP(PhnLibTrapSendMessage);

extern Err PhnLibSetText(UInt16 refNum, PhnDatabaseID msgID, const char* data, int size)
				PHN_LIB_TRAP(PhnLibTrapSetText);

extern Err PhnLibSetDate(UInt16 refNum, PhnDatabaseID msgID, UInt32 date)
				PHN_LIB_TRAP(PhnLibTrapSetDate);

extern Err PhnLibSetOptions(UInt16 refNum, PhnDatabaseID msgID, const SMSSendOptions* options)
				PHN_LIB_TRAP(PhnLibTrapSetOptions);

extern Err PhnLibSetAddresses(UInt16 refNum, PhnDatabaseID msgID, const PhnAddressList list)
				PHN_LIB_TRAP(PhnLibTrapSetAddresses);

extern Err PhnLibSetStatus(UInt16 refNum, PhnDatabaseID msgID, SMSMessageStatus status)
				PHN_LIB_TRAP(PhnLibTrapSetStatus);

extern Err PhnLibSetFlags(UInt16 refNum, PhnDatabaseID msgID, UInt32 flags)
				PHN_LIB_TRAP(PhnLibTrapSetFlags);

extern Err PhnLibSetOwner(UInt16 refNum, PhnDatabaseID msgID, UInt32 owner)
				PHN_LIB_TRAP(PhnLibTrapSetOwner);

extern Err PhnLibGetText(UInt16 refNum, PhnDatabaseID msgID, MemHandle* data)
				PHN_LIB_TRAP(PhnLibTrapGetText);

extern Err PhnLibGetDate(UInt16 refNum, PhnDatabaseID msgID, UInt32* date)
				PHN_LIB_TRAP(PhnLibTrapGetDate);

extern Err PhnLibGetOptions(UInt16 refNum, PhnDatabaseID msgID, SMSSendOptions* options)
				PHN_LIB_TRAP(PhnLibTrapGetOptions);

extern Err PhnLibGetAddresses(UInt16 refNum, PhnDatabaseID msgID, PhnAddressList* list)
				PHN_LIB_TRAP(PhnLibTrapGetAddresses);

extern Err PhnLibGetStatus(UInt16 refNum, PhnDatabaseID msgID, SMSMessageStatus* status)
				PHN_LIB_TRAP(PhnLibTrapGetStatus);

extern Err PhnLibGetFlags(UInt16 refNum, PhnDatabaseID msgID, UInt32* flags)
				PHN_LIB_TRAP(PhnLibTrapGetFlags);

extern Err PhnLibGetOwner(UInt16 refNum, PhnDatabaseID msgID, UInt32* owner)
				PHN_LIB_TRAP(PhnLibTrapGetOwner);

extern Err PhnLibGetType(UInt16 refNum, PhnDatabaseID msgID, SMSMessageType* type)
				PHN_LIB_TRAP(PhnLibTrapGetType);

extern Err PhnLibSetServiceCentreAddress(UInt16 refNum, const PhnAddressHandle address)
				PHN_LIB_TRAP(PhnLibTrapSetServiceCentreAddress);

extern Err PhnLibGetServiceCentreAddress(UInt16 refNum, PhnAddressHandle* address)
				PHN_LIB_TRAP(PhnLibTrapGetServiceCentreAddress);


#ifdef __cplusplus 
}
#endif


#endif  // _SMSLIBRARY_H_
