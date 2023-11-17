/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: IdLookupLib.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef __IDLOOKUPLIB_H__
#define __IDLOOKUPLIB_H__

#define kIdLookupSearchStrMaxLen				256
#define kIdLookupPhoneLabelAbbreviationMaxLen	6

/************************************************************
 * Function Prototypes
 *************************************************************/
typedef struct IdLookupLibSearchTag
{
	MemHandle 	resultStrH;								// OUT:		The name of the matching record if one is found.
	uint32_t	fieldInfo;								// OUT:		Field Info if the matching record if one is found.
	uint32_t	recordId;								// OUT:		ID of the matching record if one is found.
	Boolean		found;									// OUT:		Set to true if a matching record was found.
	uint8_t		padding1;
	uint16_t	searchTimeOut;							// IN:		Time out in Milliseconds to stop the search
														//			if ID is not yet found, a 'sysErrTimeout' is then returned.
	char* 		formatStr;								// IN:		Template for the result.
	char 		searchStr[kIdLookupSearchStrMaxLen];	// IN:		The string used to search the matching record.
	
} IdLookupLibSearchType, *IdLookupLibSearchTypePtr;


typedef struct IdLookupLibSearchPhoneNumberTag
{
	IdLookupLibSearchType	phoneNumberSearchInfo;
	char					phoneLabelAbbreviation[kIdLookupPhoneLabelAbbreviationMaxLen]; // OUT: 	The phone number label Letters.
	uint16_t				padding2;
} IdLookupLibSearchPhoneNumberType, *IdLookupLibSearchPhoneNumberTypePtr;


typedef struct IdLookupLibSearchEmailTag
{
	IdLookupLibSearchType	eMailSearchInfo;

} IdLookupLibSearchEmailType, *IdLookupLibSearchEmailTypePtr;


typedef struct IdLookupLibSearchUrlTag
{
	IdLookupLibSearchType	urlSearchInfo;

} IdLookupLibSearchUrlType, *IdLookupLibSearchUrlTypePtr;

#ifdef __cplusplus
extern "C" {
#endif

status_t	IdLookupLibSearchPhoneNumber(IdLookupLibSearchPhoneNumberType * searchPhoneNumberP);
	
status_t	IdLookupLibSearchEmail(IdLookupLibSearchEmailType * searchEmailP);
	
status_t	IdLookupLibSearchUrl(IdLookupLibSearchUrlType * searchUrlP);


#ifdef __cplusplus
}
#endif

#endif // __IDLOOKUPLIB_H__
