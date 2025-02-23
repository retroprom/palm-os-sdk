/***************************************************************
 *
 *  Project:
 *	  NetPref Library
 *
 * Copyright info:
 *
 *	  Copyright (c) Handspring 2001 -- All Rights Reserved
 *
 *
 *  FileName:
 *	  NetPrefLibErrors.h
 * 
 *  Description:
 *	  This is the error declarations file for the NetPref Library.
 *
 *	Note:
 *
 *	History:
 *	  12-Dec-2001	vmk	  Created by Vitaly Kruglikov
 *
 ****************************************************************/

#ifndef _NET_PREF_LIB_ERRORS_H_
#define _NET_PREF_LIB_ERRORS_H_

#include <HsErrorClasses.h>

#include "OSServices.h"	  // for osSvcErrClassApp


// hsNetMasterErrorClass is defined in HsErrorClasses.h
#define netPrefErrClass	  (hsNetPrefLibErrorClass)	 // 0x7200


#define netPrefErrMemory			(netPrefErrClass + 0x01)  // runtime memory error
#define netPrefErrStorage			(netPrefErrClass + 0x02)  // storage memory error
#define netPrefErrBadArg			(netPrefErrClass + 0x03)  // invalid argument, unknown ID
#define netPrefErrRecNotSaved		(netPrefErrClass + 0x04)  // cannot perform the requested
															  //  operation on the new record
															  //  until it is saved.
#define netPrefErrRecGone			(netPrefErrClass + 0x05)  // record could not be found
#define netPrefErrCCSMIncomplete	(netPrefErrClass + 0x06)  // missing or incomplete CCSM table
#define netPrefErrCSDNotAllowed		(netPrefErrClass + 0x07)  // CSD is not allowed
#define netPrefErrNoCCSMSettings	(netPrefErrClass + 0x08)  // settings were not found in the CCSM Carrier Settings DB
#define netPrefErrNVSync			(netPrefErrClass + 0x09)  // Error synchronizing settings with
															  //  the radio's NV storage
#define netPrefErrNVRead			(netPrefErrClass + 0x0A)  // Error reading setting from
															  //  the radio's NV storage
#define netPrefErrCCSMNoCarrierMatch (netPrefErrClass + 0x0B) // No matching entry in the CCSM Carrier List DB
#define netPrefErrRadioIsOff		(netPrefErrClass + 0x0C)  // Operation failed because the
															  //  radio (wireless mode) is off.
#define netPrefErrPhoneLibOpenFailed  (netPrefErrClass + 0x0D)  // Error while opening PhoneLib

#define netPrefErrNVWrite			(netPrefErrClass + 0x0E)  // Error writing setting to
															  //  the radio's NV storage

#define netPrefErrReadOnlyField		(netPrefErrClass + 0x0F)  // Attempting to modify the data of
															  //  a read-only field.

#define netPrefErrFieldNotInSet		(netPrefErrClass + 0x10)  // Attempting to modify a field that
															  //  is not in the record's set.



#endif // _NET_PREF_LIB_ERRORS_H_
