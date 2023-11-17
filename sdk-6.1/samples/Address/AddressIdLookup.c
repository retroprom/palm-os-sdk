/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressIdLookup.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <ErrorMgr.h>
#include <StringMgr.h>
#include <TraceMgr.h>
#include <TextMgr.h>
#include <string.h>
#include <TimeMgr.h>
#include <PhoneLookup.h>

#include "SearchLib.h"
#include "AddressIdLookup.h"

#include "AddressTab.h"
#include "AddressPrefs.h"
#include "AddressDBSchema.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressLookup.h"

/***********************************************************************
 *
 * FUNCTION:    PrvAddressIdLookupCommonWrapper
 *
 * DESCRIPTION:	perform actions common to all Wrapper.
 *
 * PARAMETERS:  - <>	IdLookupLibSearchTypeP
 *				- >	columnFilter
 *				- >	charFilter
 *				- >	textToSearchP
 *				- >	reverseString
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static status_t PrvAddressIdLookupCommonWrapper(IdLookupLibSearchType *	IdLookupLibSearchTypeP, uint32_t columnFilter, char * textToSearchP, uint16_t charFilter, Boolean reverseString, uint32_t minCompareMatch)
{
	AddressTabColumnPropertiesType*	propP;
	SearchLibType					idLookupSearchStruct;
	uint16_t						dbOpenMode;
	uint32_t						colIDsListItemCount = 0;
	Boolean							found = false;
	uint32_t *						colIDsListP = NULL;
	status_t						err = errNone;
	CategoryID						cat = catIDAll;

	memset(&idLookupSearchStruct, 0, sizeof(SearchLibType));

	if (!textToSearchP)
		goto exit;

	// if true, reverse string before comparing.
	idLookupSearchStruct.reverseString = reverseString;
	// if != 0, only keep chars that match this char Attrib (charAttr_XD, charAttrAlNum, ...)
	idLookupSearchStruct.charFilter = charFilter;

	// text to search
	idLookupSearchStruct.textToSearch = textToSearchP;

	// Determime if secret/masked record should be shown.
	dbOpenMode = (gPrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	// set Database to search in.
	idLookupSearchStruct.searchDB = gAddrDB;
	idLookupSearchStruct.cursorID = dbInvalidCursorID; // SearchLib to create the cursor
	idLookupSearchStruct.schemaName = kAddressDefaultTableName;
	idLookupSearchStruct.orderBy = gCurrentOrderByStr;
	idLookupSearchStruct.cursorFlag = 0;

	idLookupSearchStruct.numCategories = 1;
	idLookupSearchStruct.catIDsP = &cat;

	colIDsListItemCount = AddressTabMatchColIds(columnFilter, &colIDsListP);

	if (colIDsListItemCount && colIDsListP)
	{
		// number of columns IDs to search in
		idLookupSearchStruct.numCols = colIDsListItemCount;
		// array col IDs to search in
		idLookupSearchStruct.colIDsP = colIDsListP;
		// start from this record index for the first search (1 =  first / dbMaxRowIndex = last)
		idLookupSearchStruct.startRowIndex = 1;
		// Start from this column index for the first search (0 =  first / dbMaxRowIndex = last)
		idLookupSearchStruct.startColumnIndex = 0;
		// match mode to apply to the catgory search
		idLookupSearchStruct.matchMode = DbMatchAny;
		// search direction (seek forward/backward)
		idLookupSearchStruct.recordDirection = kSearchMoveForward;
		idLookupSearchStruct.columnDirection = kSearchMoveForward;
		// if true, assume that columns listed in colIDsP contains columnID as data.
		idLookupSearchStruct.indirect = false;
		// If true, Compare String function will be used instead of Find
		idLookupSearchStruct.useCompare = true;
		// if (!= 0), stop searching when having a least this number of matching chars.
		idLookupSearchStruct.minCompareMatch = minCompareMatch;
		
//###	
		if (IdLookupLibSearchTypeP->searchTimeOut)
		{
			// To enable the searchlib time out mechanism, we need to do the following initialization:
			idLookupSearchStruct.interruptible = false;
			idLookupSearchStruct.interruptCheck = IdLookupLibSearchTypeP->searchTimeOut; // timeout in Ms
			// If the search stop due to the time out, 'interrupted' field will be set to true.
			idLookupSearchStruct.interrupted = false;
		}
		else
		{
			idLookupSearchStruct.interruptible = false;
			idLookupSearchStruct.interrupted = false;
			idLookupSearchStruct.interruptCheck = 0;
		}
//###		

		if ((err = SearchLibInit(&idLookupSearchStruct)) < errNone)
			goto exit;

		while (!found)
		{
			found = SearchLibSearch(&idLookupSearchStruct);

			if (!found)
			{
				//###
				if (idLookupSearchStruct.interrupted)
					err = sysErrTimeout;
				//###
				goto exit;
			}
			else
			{
				// Check if it's a strict search.
				if (minCompareMatch != 0)
				{
					// it's not a strict search, so check that the found item size is correct.
					if (idLookupSearchStruct.strDestLength <= minCompareMatch)
					{
						found = false;
					}
				}

				if (found)
				{
					// generate the result string.
					IdLookupLibSearchTypeP->resultStrH = LookupCreateResultString(idLookupSearchStruct.searchDB,
																IdLookupLibSearchTypeP->formatStr,
																idLookupSearchStruct.foundRowID);

					IdLookupLibSearchTypeP->recordId = idLookupSearchStruct.foundRowID;


					propP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, idLookupSearchStruct.foundColID);
					if (propP != NULL)
						IdLookupLibSearchTypeP->fieldInfo = propP->fieldInfo;
				}
				else
				{
					idLookupSearchStruct.resumePolicy = kSearchResumeContinue;
					found = false;
				}
			}
		}
	}

exit:

	IdLookupLibSearchTypeP->found = found;

	MemPtrFree(idLookupSearchStruct.colIDsP);
	SearchLibRelease(&idLookupSearchStruct);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    PrvAddressIdLookupPhoneNumberWrapper
 *
 * DESCRIPTION:
 *		Search Heuristic:
 *			If the searched number has a len <= 6, we do a strict search,
 *			else we first do a strict search, and if the number isn't found,
 *			we take the first number if it exits that has at least its 6 last chars
 *			matching with the searched number.
 *
 *			The search needs to be performed as quick as possible even if we have a large
 *			 amount of records in the addressBook DB.
 *			We take as hypothesys that we don't have 2 or more records containing numbers
 *			 with their last 7 numbers identicals.
 *
 * PARAMETERS:    searchPhoneNumberP -
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void	PrvAddressIdLookupPhoneNumberWrapper(IdLookupLibSearchPhoneNumberTypePtr searchPhoneNumberP)
{
	status_t					err = errNone;
	IdLookupLibSearchType *		IdLookupLibSearchTypeP = &(searchPhoneNumberP->phoneNumberSearchInfo);
	Boolean						reverseString;
	uint16_t					charFilter;
	uint32_t					columnFilter;
	uint32_t					convertedStrSize;
	char *						convertedStrP = NULL;
	char *						strP = NULL;
	Boolean						found = false;
	uint32_t					minCompareMatch;
	Boolean						fCheckTimeOut = false;
	uint32_t					searchStartTime = 0;
	uint32_t					currentTime;

    reverseString = true;
	charFilter = charAttr_DI;
	columnFilter = kFieldFamilyMask | kFieldType_Phone | kFieldKindMask;

	convertedStrP = SearchLibConvertString(IdLookupLibSearchTypeP->searchStr,
														charFilter,
														reverseString,
														&convertedStrSize);
	if (!convertedStrSize)
		goto exit;
	else
	{
		if (IdLookupLibSearchTypeP->searchTimeOut)
		{
			searchStartTime = (uint32_t) SysTimeInMilliSecs(TimGetTicks());
			fCheckTimeOut = true;
		}

		// Strict search first
		minCompareMatch = 0;
		err = PrvAddressIdLookupCommonWrapper(IdLookupLibSearchTypeP,
														columnFilter,
														convertedStrP,
														charFilter,
														reverseString,
														minCompareMatch
														);
		if (err)
			goto exit;
		else
			found = IdLookupLibSearchTypeP->found;

		if ((!found) && (convertedStrSize > 6))
		{
			if (fCheckTimeOut)
			{
				// Update the search time out, remove time already spent in previous search 
				currentTime = (uint32_t) SysTimeInMilliSecs(TimGetTicks());
				if ((currentTime - searchStartTime) > IdLookupLibSearchTypeP->searchTimeOut)
				{
					err = sysErrTimeout;
					goto exit;
				}
				else
					IdLookupLibSearchTypeP->searchTimeOut = (uint16_t)(currentTime - searchStartTime);
			}

			//  Search for a phone number having at least its 6 last digits identical to those of the given number.
			minCompareMatch = 6;

			// Search number having at least their first "minCompareMatch" char identical.
			// and having a max size of "maxResultLen"

			err = PrvAddressIdLookupCommonWrapper(IdLookupLibSearchTypeP,
																columnFilter,
																convertedStrP,
																charFilter,
																reverseString,
																minCompareMatch
																);
			if (err)
				goto exit;
			else
				found = IdLookupLibSearchTypeP->found;
		}
	}

	// Wrapper specific
	// Get the 'phoneLabelAbbreviation'using the found record id.
	if ((err >= errNone) && (IdLookupLibSearchTypeP->found))
		ToolsGetAbbreviationFromProperty((char *)&(searchPhoneNumberP->phoneLabelAbbreviation), IdLookupLibSearchTypeP->fieldInfo);

exit:
	if (convertedStrP)
		MemPtrFree(convertedStrP);
	if (strP)
		MemPtrFree(strP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAddressIdLookupEmailWrapper
 *
 * DESCRIPTION: .
 *
 * PARAMETERS:    searchEmailP -
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void	PrvAddressIdLookupEmailWrapper(IdLookupLibSearchEmailTypePtr searchEmailP)
{
	status_t					err = errNone;
	IdLookupLibSearchType *		IdLookupLibSearchTypeP = &(searchEmailP->eMailSearchInfo);
	Boolean						reverseString;
	uint16_t					charFilter;
	uint32_t					columnFilter;
	uint32_t					convertedStrSize;
	char *						convertedStrP = NULL;
    uint32_t					minCompareMatch;

	// Wrapper specific search heuristic
	reverseString = false;
	charFilter = charAttrPrint;
	columnFilter = kFieldFamilyMask | kFieldType_Internet | kFieldKind_EmailInternet;

	convertedStrP = SearchLibConvertString(IdLookupLibSearchTypeP->searchStr,
														charFilter,
														reverseString,
														&convertedStrSize);
	if (!convertedStrSize)
		goto exit;
	else
	{
		minCompareMatch = 0;

		// Wrapper common
		err = PrvAddressIdLookupCommonWrapper(IdLookupLibSearchTypeP,
												columnFilter,
												convertedStrP,
												charFilter,
												reverseString,
												minCompareMatch
												);
	}

exit:
	if (convertedStrP)
		MemPtrFree(convertedStrP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAddressIdLookupUrlWrapper
 *
 * DESCRIPTION: .
 *
 * PARAMETERS:    searchUrlP -
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void	PrvAddressIdLookupUrlWrapper(IdLookupLibSearchUrlTypePtr searchUrlP)
{
	status_t					err = errNone;
	IdLookupLibSearchType *		IdLookupLibSearchTypeP = &(searchUrlP->urlSearchInfo);
	Boolean						reverseString;
	uint16_t					charFilter;
	uint32_t					columnFilter;
	uint32_t					convertedStrSize;
	char *						convertedStrP = NULL;
	uint32_t					minCompareMatch;

	// Wrapper specific search heuristic
	reverseString = false;
	charFilter = charAttrPrint;
	columnFilter = kFieldFamilyMask | kFieldType_Internet | kFieldKind_URLInternet;

	convertedStrP = SearchLibConvertString(IdLookupLibSearchTypeP->searchStr,
														charFilter,
														reverseString,
														&convertedStrSize);
	if (!convertedStrSize)
		goto exit;
	else
	{
		// Strict comparaison
		minCompareMatch = 0;

		// Wrapper common
		err = PrvAddressIdLookupCommonWrapper(IdLookupLibSearchTypeP,
												columnFilter,
												convertedStrP,
												charFilter,
												reverseString,
												minCompareMatch
												);
	}

exit:
	if (convertedStrP)
		MemPtrFree(convertedStrP);
}


/***********************************************************************
 *
 * FUNCTION:    AddressIdLookup
 *
 * DESCRIPTION: .
 *
 * PARAMETERS:    addressIdLookupP -
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
void	AddressIdLookup(AddressIdLookupLibSearchType *addressIdLookupP)
{
	switch (addressIdLookupP->addressIdSearchType)
	{
		case kIdLookupLibSearchTypePhoneNumber:
				PrvAddressIdLookupPhoneNumberWrapper((IdLookupLibSearchPhoneNumberTypePtr) (addressIdLookupP->addressIdLookupLibSearchTypeP));
			break;

		case kIdLookupLibSearchTypeEmail:
				PrvAddressIdLookupEmailWrapper((IdLookupLibSearchEmailTypePtr) (addressIdLookupP->addressIdLookupLibSearchTypeP));
			break;

		case kIdLookupLibSearchTypeUrl:
				PrvAddressIdLookupUrlWrapper((IdLookupLibSearchUrlTypePtr) (addressIdLookupP->addressIdLookupLibSearchTypeP));
			break;
	}
}
