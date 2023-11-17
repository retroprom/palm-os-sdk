/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressIdLookup.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSIDLOOKUP_H_
#define _ADDRESSIDLOOKUP_H_

#include "IdLookupLib.h"

/***********************************************************************
 *   Internal Constants
 ***********************************************************************/
// Id Lookup launch code
#define AddressBookIdLookupLaunch			(sysAppLaunchCmdCustomBase | 0x0001)

#define kIdLookupLibSearchTypePhoneNumber	0x01
#define kIdLookupLibSearchTypeEmail			0x02
#define kIdLookupLibSearchTypeUrl			0x03


/************************************************************
 *	Internal Structures
 *************************************************************/

typedef struct AddressIdLookupLibSearchTag
{
	void * 		addressIdLookupLibSearchTypeP;
	uint8_t 	addressIdSearchType;
	uint8_t 	padding1;
	uint16_t 	padding2;

} AddressIdLookupLibSearchType, *AddressIdLookupLibSearchTypePtr;


/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void	AddressIdLookup(AddressIdLookupLibSearchType *addressIdLookupP);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSIDLOOKUP_H_
