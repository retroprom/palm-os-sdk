/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PBkIOTransferPrv.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Private declarations for the Phone Book Import / Export exchange library.
 *
 *****************************************************************************/

#ifndef __PHONEBOOKTRANSFERPRV_H__
#define __PHONEBOOKTRANSFERPRV_H__


#include <TelephonyLib.h>
#include <TelephonyLibTypes.h>

// *******************************************************************************
// Types definition
// *******************************************************************************

// Number of dynamic fields in the AddressBook, each of them can be any type 
#define kPhoneBookIOMaxPhoneNumber		25

typedef struct TelASyncEventInfoTag
{
	int32_t 	telAppId;
	uint16_t	transId;
	uint16_t	padding;
} TelASyncEventInfoType;

typedef struct AddressPhoneRecordTag
{
	char	lastNameP[kTelPhoneNameMaxLen];
	char	firstNameP[kTelPhoneNameMaxLen];
	char	titleP[kTelPhoneNameMaxLen];
	char	companyP[kTelPhoneNameMaxLen];
	char	phoneNbP[kPhoneBookIOMaxPhoneNumber][kTelPhoneNumberMaxLen];
} AddressPhoneRecordType;

// Phone book information
typedef struct AddressPhoneBookInfoTag	
{
	uint32_t	phoneBkUsedSlots;
	uint32_t	phoneBkTotalSlots;
} AddressPhoneBookInfoType;

// *******************************************************************************
// Prototypes
// *******************************************************************************

// Init / Finalize
status_t PBkIOTransferTelInitialize(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, ExgSocketType* socketP);
status_t PBkIOTransferTelFinalize(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, ExgSocketType* socketP);

// Get the used / available entry slots in the phone book
status_t PBkIOTransferGetPhoneBookInfo(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, AddressPhoneBookInfoType * phoneBookInfoP);

// Add a record
status_t PBkIOTransferAddRecordToPhone(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, 
	AddressPhoneRecordType * phoneRecordP, int32_t index,  Boolean * allPhoneEntriesHandledP);

// Get a record
status_t PBkIOTransferReadRecordFromPhone(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, 
	AddressPhoneRecordType * phoneRecord, int32_t index);

// Deletes all records in phone book from first to last
status_t PBkIOTransferDeletePhoneRecord(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, int32_t index);

#endif //__PHONEBOOKTRANSFERPRV_H__
