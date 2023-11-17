/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PhoneLookup.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines phone number lookup structures and routines.
 *
 *****************************************************************************/

#ifndef __PHONE_LOOKUP_H__
#define __PHONE_LOOKUP_H__

#include <PalmTypes.h>
#include <CmnLaunchCodes.h>

#include <Field.h>
#include <DataMgr.h>
#include <AddressFields.h>

// This is a list of fields by which data may be looked up.
enum AddressLookupFieldsTag {
	addrLookupName,
	addrLookupFirstName,
	addrLookupCompany,
	addrLookupAddress,
	addrLookupCity,
	addrLookupState,
	addrLookupZipCode,
	addrLookupCountry,
	addrLookupTitle,
	addrLookupCustom1,
	addrLookupCustom2,
	addrLookupCustom3,
	addrLookupCustom4,
	addrLookupNote,			// This field is assumed to be < 4K 
	addrLookupWork,
	addrLookupHome,
	addrLookupFax,
	addrLookupOther,
	addrLookupEmail,
	addrLookupMain,
	addrLookupPager,
	addrLookupMobile,
	addrLookupSortField,
	addrLookupListPhone,
	addrLookupFieldCount,	// add new fields above this one
	
	addrLookupNoField = 0xff
} ;
typedef Enum8 AddressLookupFields ;

#define addrLookupStringLength 20

typedef struct 
	{
	char *title;					
		// Title to appear in the title bar.  If NULL the default is used.
		
	char *pasteButtonText;
		// Text to appear in paste button.  If NULL "paste" is used.
		
	char lookupString[addrLookupStringLength];
		// Buffer containing string to lookup.  If the string matches
		// only one record then that record is used without 
		// presenting the user with the lookup dialog.
		
	AddressLookupFields field1;
		// Field to search by.  This field appears on the left side
		// of the lookup dialog.  If the field is the sort field then
		// searches use a binary search.  If the field isn't the sort
		// field then the data does appear in sorted order and searching
		// is performed by a linear search (can get slow).
		
	AddressLookupFields field2;
		// Field to display on the right.  Often displays some 
		// information about the person.  If it is a phone field
		// and a record has multiple instances of the phone type
		// then the person appears once per instance of the phone
		// type. Either field1 or field2 may be a phone field but
		// not both.

	Boolean field2Optional;
		// True means that the record need not have field2 for
		// the record to be listed.  False means that field2 is
		// required in the record for it to be listed. 
		
	Boolean userShouldInteract;
		// True means that the user should resolve non unique 
		// lookups.  False means a non unique and complete lookup
		// returns resultStringH set to 0 and recordID set to 0;
		
	char *formatStringP;
		// When the user selects the paste button a string is generated
		// to return data from the record.  The format of the result string
		// is controlled by this string.  All characters which appear
		// in this string are copied straight to the result string unless
		// they are a field (a '^' follow by the field name).  For 
		// example, the format string "^first - ^home" might result in
		// "Roger - 123-4567".
		
		// The field arguments are name, first, company, address, city
		// state, zipcode, country, title, custom1, custom2, custom3,
		// custom4, work, home, fax, other, email, main, pager, mobile, 
		// and listname.
		
	MemHandle resultStringH;
		// If there is a format string a result string is allocated on 
		// the dynamic heap and it's MemHandle is returned here.
	
	uint32_t uniqueID;
		// The unique ID of the found record or 0 if none was found.
		
	} AddrLookupParamsType;

typedef AddrLookupParamsType *AddrLookupParamsPtr;
#ifdef __cplusplus
extern "C" {
#endif

extern void PhoneNumberLookup (FieldType *fldP);

extern void PhoneNumberLookupCustom (FieldType *fldP, AddrLookupParamsType* params, Boolean useClipboard);

#ifdef __cplusplus 
}
#endif


#endif	// __PHONE_LOOKUP_H__
