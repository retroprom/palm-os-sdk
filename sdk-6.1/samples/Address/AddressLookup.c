/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressLookup.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *     This is the Address Book's Lookup module.  This module
 *   handles looking up address book information for other apps.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <ErrorMgr.h>
#include <SoundMgr.h>
#include <StringMgr.h>
#include <TraceMgr.h>
#include <PenInputMgr.h>
#include <TextMgr.h>
#include <SearchLib.h>
#include <Table.h>
#include <string.h>
#include <FormLayout.h>
#include <PhoneLookup.h>
#include <TextServicesMgr.h>
#include <AttentionMgr.h>

#include "AddressPrefs.h"
#include "AddressLookup.h"
#include "AddressDBSchema.h"
#include "AddressTools.h"
#include "AddressRsc.h"
//#include "AddressDefines.h"
#include "Address.h"
#include "AddressFreeFormName.h"
#include "AddressU32List.h"


/***********************************************************************
 *
 *   Constants
 *
 ***********************************************************************/

static const uint32_t addrLookupCompatibilityMap[addrLookupFieldCount] = {
	kFieldFamilyMask | kFieldType_Name	| kFieldKind_LastName					// addrLookupName,
,	kFieldFamilyMask | kFieldType_Name | kFieldKind_FirstName					// addrLookupFirstName,
,	kFieldFamilyMask | kFieldType_Company | kFieldKind_NameCompany				// addrLookupCompany,
,	kFieldFamilyMask | kFieldType_Address | kFieldKind_StreetAddress			// addrLookupAddress,
,	kFieldFamilyMask | kFieldType_Address | kFieldKind_CityAddress				// addrLookupCity,
,	kFieldFamilyMask | kFieldType_Address | kFieldKind_StateAddress				// addrLookupState,
,	kFieldFamilyMask | kFieldType_Address | kFieldKind_ZipCodeAddress			// addrLookupZipCode,
,	kFieldFamilyMask | kFieldType_Address | kFieldKind_CountryAddress			// addrLookupCountry,
,	kFieldFamilyMask | kFieldType_Name | kFieldKind_TitleName					// addrLookupTitle,
,	kFieldFamilyMask | kFieldTypeMask | kFieldKind_CustomExt | 0x00000001		// addrLookupCustom1,
,	kFieldFamilyMask | kFieldTypeMask | kFieldKind_CustomExt | 0x00000002		// addrLookupCustom2,
,	kFieldFamilyMask | kFieldTypeMask | kFieldKind_CustomExt | 0x00000003		// addrLookupCustom3,
,	kFieldFamilyMask | kFieldTypeMask | kFieldKind_CustomExt | 0x00000004		// addrLookupCustom4,
,	0xFFFFFFFF	// kAddrColumnIDNote										// addrLookupNote,
,	kFieldFamily_Corp | kFieldType_Phone | kFieldKindMask						// addrLookupWork,
,	kFieldFamily_Home | kFieldType_Phone | kFieldKindMask						// addrLookupHome,
,	kFieldFamilyMask | kFieldType_Phone | kFieldKind_FaxPhone					// addrLookupFax,
,	kFieldFamily_Other | kFieldType_Phone | kFieldKindMask						// addrLookupOther,
,	kFieldFamilyMask | kFieldType_Internet | kFieldKind_EmailInternet			// addrLookupEmail,
,	kFieldFamilyMask | kFieldType_Phone | kFieldType_Internet | kFieldKindMask	// addrLookupMain,
,	kFieldFamilyMask | kFieldType_Phone | kFieldKind_PagerPhone					// addrLookupPager,
,	kFieldFamilyMask | kFieldType_Phone | kFieldKind_MobilePhone		 		// addrLookupMobile,
,	0xFFFFFFFF	// get columnIDs from current sort ID							// addrLookupSortField,
,	0xFFFFFFFF	// get columnID from kAddrColumnIDDisplayedPhone			// addrLookupListPhone,
};


// display name columnIDs indexed by AddressLookupFieldsTag enum
// The Yomi fields must be searched first. To ensure that fields will be searched in
// the declaration order we use the forceSeekForwardWithinRecords option of the SearchLib
static uint32_t*	sDisplayNameColIDs = NULL;
static uint32_t		sDisplayNameNumCols = 0;


/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Address lookup table columns
#define field1Column	0
#define field2Column	1

/***********************************************************************
 *
 *   Globals
 *
 ***********************************************************************/

static AddrLookupParamsType *	sParamsP;
static FormType *				sFormP;
static uint32_t					sTopVisibleRecord;
static uint32_t					sTopVisibleColumn;
static uint32_t					sCurrentRecord;
static uint32_t					sCurrentColumn;
static Boolean					sIgnoreEmptyLookupField;
static Boolean					sBeepOnFail;
static SearchLibType			sSearchLibInfo;

static uint32_t					sField2NumCols;
static uint32_t *				sField2ColIDsP = NULL;
static Boolean					sField2Indirect;

static uint16_t					sTableNumVisibleRows;

static Boolean					sOneHandedTableHasFocus = false;
static Boolean					sOneHandedRecordSelected = false;
static int16_t					sOneHandedLastSelectedRow;

/***********************************************************************
 *
 * FUNCTION:    PrvLookupFindPhoneField
 *
 * DESCRIPTION: Find a phone field from the record.
 *
 * PARAMETERS:	->	searchDB: the dmOpenRef of the DB to look in.
 *				->	rowID: the record's unique rowID or cursor ID positioned on a valid row
 *				->	lookupField: the phone field to lookup
 *
 * RETURNED:    The field to use or lookupNoField if none found
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/18/96   Initial Revision
 *         LYr    1/7/2003   now return a schema column ID
 *
 ***********************************************************************/
static uint32_t PrvLookupFindPhoneField(DmOpenRef searchDB, uint32_t rowID, uint32_t lookupField)
{
	DbSchemaColumnValueType	*colVals;
	uint32_t				i, n, foundColID, *colIDs;
	status_t				err;

	if (lookupField >= addrLookupFieldCount)
		return addrLookupNoField;

	if ((n = AddressTabMatchColIds(addrLookupCompatibilityMap[lookupField], &colIDs)) == 0)
		return addrLookupNoField;

	err = DbGetColumnValues(searchDB, rowID, n, colIDs, &colVals);

	if (err < errNone && err != dmErrOneOrMoreFailed)
		return addrLookupNoField;

	foundColID = addrLookupNoField;

	for (i = 0; i < n; i++)
	{
		if (colVals[i].errCode >= errNone && colVals[i].dataSize)
		{
			if (foundColID == addrLookupNoField)
				foundColID = colVals[i].columnID;

			if (colVals[i].columnID == sCurrentColumn)
			{
				foundColID = sCurrentColumn;
				break;
			}
		}
	}

	DbReleaseStorage(searchDB, colVals);
	return foundColID;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupResizeResultString
 *
 * DESCRIPTION: Resize the lookup a result string
 *
 * PARAMETERS:  resultStringH - result string handle
 *              newSize       - new size
 *
 * RETURNED:    pointer to the resized result, or NULL if the resize failed.
 *
 ***********************************************************************/
static char * PrvLookupResizeResultString(MemHandle resultStringH, size_t newSize)
{
	MemHandleUnlock(resultStringH);

	if (MemHandleResize(resultStringH, newSize) < errNone)
		return NULL;

	return (MemHandleLock(resultStringH));
}


/***********************************************************************
 *
 * FUNCTION:    LookupCreateResultString
 *
 * DESCRIPTION: Create a result string which includes data from the record.
 *
 * PARAMETERS:	->	searchDB: the DB open ref to search in
 *				->	formatStringP: the format string
 *				->	rowID: the row's unique ID or cursor ID positioned on a valid row
 *
 * RETURNED:    The MemHandle to the string created or NULL.
 *                vars->paramsP->resultStringH is also set
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *         Ludovic 4/27/00  Fix bug - formatStringP increment invalid for some fields
 *
 ***********************************************************************/
MemHandle LookupCreateResultString(DmOpenRef searchDB, char * formatStringP, uint32_t rowID)
{
	MemHandle					resultStringH;
	char*						resultStringP;
	size_t						resultSize = 0;
	size_t						nextChunkSize;
	char*						nextFieldP;
	uint32_t					field; // current data field column ID
	char*						fieldP;
	status_t					error;
	uint16_t					separatorLength;
	uint32_t					dataSize, * listPhoneColID;
	Boolean						addPhoneAbbreviation;

	// Check if a format string was specified
	if (!formatStringP)
		return 0;

	// Allocate the string on the dynamic heap
	resultStringH = MemHandleNew(32);      // Allocate extra so there's room to grow

	if (!resultStringH)
		return 0;            // not enough memory?

	resultStringP = MemHandleLock(resultStringH);

	while (*formatStringP)
	{
		// Copy the next chunk (the string until a '^' or '\0'
		nextFieldP = strchr(formatStringP, chrCircumflexAccent);

		if (nextFieldP)
			nextChunkSize = nextFieldP - formatStringP;
		else
			nextChunkSize = strlen(formatStringP);

		if (nextChunkSize > 0)
		{
			resultStringP = PrvLookupResizeResultString(resultStringH, resultSize + nextChunkSize);

			if (!resultStringP)
				goto exit;

			memmove(resultStringP + resultSize, formatStringP, nextChunkSize);

			resultSize += nextChunkSize;
			formatStringP += nextChunkSize;
			nextChunkSize = 0;
		}

		// determine which field to copy next
		if (*formatStringP == chrCircumflexAccent)
		{
			formatStringP++;
			field = addrLookupNoField;
			addPhoneAbbreviation = false;

			// Decode which field to copy next.

			// Remember that the strings below can't be put into a table
			// because the lookup function runs without global variables
			// available with which we would store the table.
			if (strncmp(formatStringP, "name", 4) == 0)
			{
				field = kAddrColumnIDLastName;
				formatStringP += 4;
			}
			else if (strncmp(formatStringP, "first", 5) == 0)
			{
				field = kAddrColumnIDFirstName;
				formatStringP += 5;
			}
			else if (strncmp(formatStringP, "company", 7) == 0)
			{
				field = kAddrColumnIDWorkCompany;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "address", 7) == 0)
			{
				field = kAddrColumnIDHomeStreet;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "city", 4) == 0)
			{
				field = kAddrColumnIDHomeCity;
				formatStringP += 4;
			}
			else if (strncmp(formatStringP, "state", 5) == 0)
			{
				field = kAddrColumnIDHomeState;
				formatStringP += 5;
			}
			else if (strncmp(formatStringP, "zipcode", 7) == 0)
			{
				field = kAddrColumnIDHomeZipCode;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "country", 7) == 0)
			{
				field = kAddrColumnIDHomeCountry;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "title", 5) == 0)
			{
				field = kAddrColumnIDWorkTitle;
				formatStringP += 5;
			}
			else if (strncmp(formatStringP, "custom1", 7) == 0)
			{
				field = kAddrColumnIDOtherCustom1;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "custom2", 7) == 0)
			{
				field = kAddrColumnIDOtherCustom2;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "custom3", 7) == 0)
			{
				field = kAddrColumnIDOtherCustom3;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "custom4", 7) == 0)
			{
				field = kAddrColumnIDOtherCustom4;
				formatStringP += 7;
			}
			else if (strncmp(formatStringP, "work", 4) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupWork);
				formatStringP += 4;
			}
			else if (strncmp(formatStringP, "home", 4) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupHome);
				formatStringP += 4;
			}
			else if (strncmp(formatStringP, "fax", 3) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupFax);
				formatStringP += 3;
			}
			else if (strncmp(formatStringP, "other", 5) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupOther);
				formatStringP += 5;
			}
			else if (strncmp(formatStringP, "email", 5) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupEmail);
				formatStringP += 5;
			}
			else if (strncmp(formatStringP, "main", 4) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupMain);
				formatStringP += 4;
			}
			else if (strncmp(formatStringP, "pager", 5) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupPager);
				formatStringP += 5;
			}
			else if (strncmp(formatStringP, "mobile", 6) == 0)
			{
				field = PrvLookupFindPhoneField(searchDB, rowID, addrLookupMobile);
				formatStringP += 6;
			}
			else if (strncmp(formatStringP, "listname", 8) == 0)
			{
				char *	nameP;

				nameP = ToolsGetRecordNameStr(gAddrDB, rowID, stdFont, 0);

				formatStringP += 8;
				separatorLength = 0;

				if (nameP)
				{
					nextChunkSize = strlen(nameP);
					resultStringP = PrvLookupResizeResultString(resultStringH, resultSize + nextChunkSize);
					memmove(resultStringP + resultSize, nameP, nextChunkSize);
					resultSize += nextChunkSize;
					nextChunkSize = 0;

					MemPtrFree(nameP);
				}

				// We are done adding the data requested.  Continue to the next chunk
				continue;
			}
			else if (strncmp(formatStringP, "fullname", 8) == 0)
			{
				MemHandle	nameH = NULL;
				char *		nameP = NULL;

				formatStringP += 8;
				separatorLength = 0;

				if ((nameH = AddrFreeFormNameBuild(searchDB, rowID)) != NULL)
					nameP = MemHandleLock(nameH);

				if (nameP)
				{
					nextChunkSize = strlen(nameP);
					resultStringP = PrvLookupResizeResultString(resultStringH, resultSize + nextChunkSize);
					memmove(resultStringP + resultSize, nameP, nextChunkSize);
					resultSize += nextChunkSize;
					nextChunkSize = 0;

					MemHandleUnlock(nameH);
				}

				if (nameH)
					AddrFreeFormNameFree(nameH);

				// We are done adding the data requested.  Continue to the next chunk
				continue;
			}
			else if (strncmp(formatStringP, "listphone", 9) == 0)
			{
				formatStringP += 9;
				separatorLength = 0;

				// Set the field variable to the correct column ID
				error = DbGetColumnValue(searchDB, rowID, kAddrColumnIDDisplayedPhone, 0, (void**) &listPhoneColID, &dataSize);

				if (error < errNone)
					continue;

				field = * listPhoneColID;
				addPhoneAbbreviation = true;
				DbReleaseStorage(searchDB, listPhoneColID);
			}
			else if (strncmp(formatStringP, "selectedRow", 11) == 0)
			{
				formatStringP += 11;
				separatorLength = 0;

				field = sCurrentColumn;
			}

			// Now copy in the correct field.  lookupNoField can result from
			// asking for a phone which isn't used.
			if (field != addrLookupNoField)
			{
				error = DbGetColumnValue(searchDB, rowID, field, 0, (void**) &fieldP, &dataSize);

				if (error < errNone)
					continue;

				if (fieldP)
				{
					nextChunkSize = strlen(fieldP);

					if (nextChunkSize > 0)
					{
						resultStringP = PrvLookupResizeResultString(resultStringH, resultSize + nextChunkSize);

						if (!resultStringP)
						{
							DbReleaseStorage(searchDB, fieldP);
							goto exit;
						}

						memmove(resultStringP + resultSize, fieldP, nextChunkSize);

						resultSize += nextChunkSize;
						nextChunkSize = 0;
					}

					DbReleaseStorage(searchDB, fieldP);
				}

				if (addPhoneAbbreviation)
				{
					uint32_t	size;
					uint32_t *	phonePropertyP = NULL;
					char		abbreviation[kMaxAbbreviationSize + 1];

					DbGetColumnPropertyValue(gAddrDB, kAddressDefaultTableName, field,
									kAddrColumnPropertyFieldInfo, &size, (void**)&phonePropertyP);

					if (phonePropertyP)
					{
						size_t	len;

						*abbreviation = chrSpace;
						ToolsGetAbbreviationFromProperty(abbreviation + 1, *phonePropertyP);

						if (*(abbreviation + 1))
						{
							len = strlen(abbreviation);
							resultStringP = PrvLookupResizeResultString(resultStringH, resultSize + len);

							if (!resultStringP)
								goto exit;

							memmove(resultStringP + resultSize, abbreviation, len);
							resultSize += len;
						}
					}
				}
			}
		}
	}

exit:

	// Now null terminate the result string
	if (resultStringP && resultSize)
	{
		resultStringP = PrvLookupResizeResultString(resultStringH, resultSize + 1);

		if (resultStringP)
		{
			resultStringP[resultSize] = chrNull;
			MemHandleUnlock(resultStringH);

			return resultStringH;
		}
	}

	// Error return
	MemHandleFree(resultStringH);
	return NULL;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupDrawRecordFields
 *
 * DESCRIPTION: Draws the name and phone number (plus which phone)
 * within the screen bounds passed.
 *
 * PARAMETERS:	 ->	rowID: the DB rowID or cursor ID positioned on a valid row
 *				 ->	columnID: the columnID to draw
 *				 ->	fieldType: the column field type
 *				 ->	x: x pos to strar drawing
 *				 ->	y: y pos to start drawing
 *				<->	widthP: the available with at input. The used width at output
 *				 ->	leftAlign: true if leftalign, rigth align otherwise
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/21/95   Initial Revision
 *
 ***********************************************************************/
static void PrvLookupDrawRecordFieldsPart(uint32_t rowID, uint32_t columnID, AddressLookupFields fieldType, Coord x, Coord y, Coord *widthP, Boolean leftAlign)
{
	Coord		realSize;
	char*		nameP = NULL;
	char*		fieldStr = NULL;
	status_t	error = errNone;
	char		abbreviation[kMaxAbbreviationSize];
	uint32_t*	phonePropertyP = NULL;
	uint32_t	dataSize = 0;
	size_t		fieldLength = 0;
	Boolean		ignored;

	// First draw the field 2. If it doesn't use all its reserved space, transfert it to field 1 and draw it.
	if (*widthP > 0)
	{
		realSize = *widthP;

		if (fieldType == addrLookupSortField)
		{
			// nameP has already been truncated to fit in field2Width
			if ((nameP = ToolsGetRecordNameStr(gAddrDB, rowID, stdFont, *widthP)) == NULL)
				error = dmErrNoData;
		}
		else
		{
			// Use the lookup field as a colID, directly read the relevant data
			if ((error = DbGetColumnValue(sSearchLibInfo.searchDB, rowID, columnID, 0, (void**) &fieldStr, &dataSize)) >= errNone && dataSize)
			{
				nameP = ToolsStrDup(fieldStr);
				DbReleaseStorage(sSearchLibInfo.searchDB, fieldStr);
				FntTruncateString(nameP, nameP, stdFont, *widthP, true);
			}
		}

		if ((error >= errNone) && nameP)
		{
			fieldLength = strlen(nameP);
			realSize = *widthP;

			FntCharsInWidth (nameP, &realSize, &fieldLength, &ignored);//lint !e64
			// Recompute the pos and size of field
			if (leftAlign)
				x += *widthP - realSize;

			*widthP = realSize;
		}
		else
		{
			fieldLength = 0;
			*widthP = 0;
		}

		if (fieldLength)
		{
			WinDrawChars(nameP, (int16_t)fieldLength, x, y);

			if (fieldType == addrLookupListPhone)
			{
				DbGetColumnPropertyValue(sSearchLibInfo.searchDB, kAddressDefaultTableName, columnID, kAddrColumnPropertyFieldInfo, &dataSize, (void**)&phonePropertyP);

				if (phonePropertyP)
				{
					size_t	len;

					ToolsGetAbbreviationFromProperty(abbreviation, *phonePropertyP);

					if (*abbreviation)
					{
						len = strlen(abbreviation);
						WinDrawChars(abbreviation, (int16_t)len, x + realSize + 1, y);
					}

					DbReleaseStorage(sSearchLibInfo.searchDB, phonePropertyP);
				}
			}
		}

		if (nameP)
			MemPtrFree(nameP);
	}
}

static void PrvLookupDrawRecordFields(uint32_t rowID, uint32_t field, RectanglePtr bounds)
{
	Coord	x;
	Coord	y;
	Coord	field1Width = 0;
	Coord	field2Width = 0;
	Coord	posXField1 = 0;
	Coord	posXField2 = 0;
	Coord	realSize;

	// First compute the space for field1 & field2

	// Half screen for both field. Default.
	x = bounds->topLeft.x;
	y = bounds->topLeft.y;
	field1Width = field2Width = (bounds->extent.x / 2) - kSpaceBetweenNamesAndPhoneNumbers;
	posXField1 = x;
	posXField2 = x + field1Width + kSpaceBetweenNamesAndPhoneNumbers;

	// Manage No Field option
	if (sParamsP->field1 == addrLookupNoField)
	{
		field1Width = 0;
		field2Width = bounds->extent.x;
		posXField1 = 0;
		posXField2 = x;
	}

	if (sParamsP->field2 == addrLookupNoField)
	{
		field2Width = 0;

		if (field1Width)
		{
			field1Width = bounds->extent.x;
			posXField1 = x;
		}

		posXField2 = 0;
	}

	// Reserve space to draw phone abbrevation
	if (sParamsP->field1 == addrLookupListPhone && field1Width)
		field1Width -= gPhoneLabelWidth + 1;

	if (sParamsP->field2 == addrLookupListPhone && field2Width)
		field2Width -= gPhoneLabelWidth + 1;

	realSize = field2Width;
	PrvLookupDrawRecordFieldsPart(rowID, field, sParamsP->field2, posXField2, y, &realSize, true);
	field1Width += field2Width - realSize;
	PrvLookupDrawRecordFieldsPart(rowID, field, sParamsP->field1, posXField1, y, &field1Width, false);
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewDrawRecord
 *
 * DESCRIPTION: This routine draws an address book record.  It is called as
 *              a callback routine by the table object.
 *
 * PARAMETERS:  table  - pointer to the address list table
 *              row    - row number, in the table, of the item to draw
 *              column - column number, in the table, of the item to draw
 *              bounds - bounds of the draw region
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
static void PrvLookupViewDrawRecord(void * tableP, int16_t row, int16_t column, RectanglePtr bounds)
{
	uint32_t rowID;
	uint32_t columnID;

	// Get the record number that corresponds to the table item to draw.
	rowID = TblGetRowData(tableP, row);
	columnID = (uint32_t)TblGetItemPtr(tableP, row, column);

	FntSetFont(stdFont);

	PrvLookupDrawRecordFields(rowID, columnID, bounds);
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupReleaseLookupFields
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *				->  searchStruct
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         LYr   1/29/03  Initial revision
 *
 ***********************************************************************/
static void PrvLookupReleaseLookupFields(uint32_t *colIDsP)
{
	if (colIDsP && colIDsP != sDisplayNameColIDs)
		MemPtrFree(colIDsP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupMapLookupFields
 *
 * DESCRIPTION: Set the mappings from AddressLookupFields to AddressFields.
 *
 * PARAMETERS:
 *				->  field : AddressLookup Field to map
 *				<-  numCols : number of columns mapped to this field
 *				<-  colIDsP : corresponding colIDs. Must be freed using PrvLookupReleaseLookupFields
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         LYr   1/14/03  Initial revision
 *
 ***********************************************************************/
static void PrvLookupMapLookupFields(uint32_t field, uint32_t * numCols, uint32_t ** colIDsP, Boolean * indirect)
{
	(*indirect) = false;

	switch (field)
	{
		case addrLookupNote:		// Note column Id is hardcoded
			(*numCols) = 1;
			(*colIDsP) = (uint32_t *) MemPtrNew((*numCols) * sizeof(uint32_t));
			(*colIDsP)[0] = kAddrColumnIDNote;
		break;

		case addrLookupSortField:	// get columnIDs from current sort ID
			(*numCols) = sDisplayNameNumCols;
			(*colIDsP) = sDisplayNameColIDs;
		break;

		case addrLookupListPhone:	// get columnID from kAddrColumnIDDisplayedPhone
			(*numCols) = 1;
			(*colIDsP) = (uint32_t *) MemPtrNew((*numCols) * sizeof(uint32_t));
			(*colIDsP)[0] = kAddrColumnIDDisplayedPhone;
			(*indirect) = true;
		break;

		case addrLookupNoField:
			(*numCols) = 0;
			(*colIDsP) = NULL;
		break;

		default:
			(*numCols) = AddressTabMatchColIds(addrLookupCompatibilityMap[field], colIDsP);
		break;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupSeekInRecord
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	 ->	rowID: DB row ID or cursor ID positioned on a valid row
 *				 ->	newSeek: true if start a new search
 *				<->	matchColIDP: where to start search at input. where found at output
 *				 ->	direction: search direction
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *
 ***********************************************************************/
static Boolean PrvLookupSeekInRecord(uint32_t rowID, Boolean newSeek, uint32_t *matchColIDP, SearchDirectionType direction)
{
	status_t	err;
	uint32_t	numCols = 0;
	uint32_t *	colIDsP = NULL;
	int16_t		colIndex;
	Boolean		found = false;
	int16_t		currentColIndex = 0;
	uint32_t	size;

	// If there is no field to search into, return true if it's the first time
	// this record is searched, false otherwise.
	if (!sField2NumCols)
		return newSeek;

	// Build the real columnID list if passed list is indirect
	if (sField2Indirect)
	{
		uint32_t	colID;

		colIDsP = MemPtrNew(sizeof(uint32_t) * sField2NumCols);
		ErrFatalDisplayIf(!colIDsP, "Out of memory!");

		colIndex = 0;
		numCols = 0;
		while (colIndex < (int16_t)sField2NumCols)
		{
			size = sizeof(colID);
			err = DbCopyColumnValue(gAddrDB, rowID, sField2ColIDsP[colIndex], 0, &colID, &size);

			if (err >= errNone)
				colIDsP[numCols++] = colID;

			colIndex++;
		}

		if (!numCols)
			goto Exit;
	}
	else
	{
		colIDsP = sField2ColIDsP;
		numCols = sField2NumCols;
	}

	while (currentColIndex < (int16_t)numCols)
	{
		if (colIDsP[currentColIndex] == *matchColIDP)
			break;

		currentColIndex++;
	}

	if (newSeek || currentColIndex >= (int16_t)numCols)
	{
		if (currentColIndex >= (int16_t)numCols)
			currentColIndex = (direction == kSearchMoveForward) ? 0 : (int16_t)numCols - 1;

		size = 0;
		err = DbCopyColumnValue(gAddrDB, rowID, colIDsP[currentColIndex], 0, NULL, &size);
		found = (Boolean) (err >= errNone && size > 0);
	}

	// Search for the next or previous columnID which contains data.
	while (!found)
	{
		currentColIndex += (direction == kSearchMoveForward) ? 1 : -1;
		if (currentColIndex < 0 || currentColIndex >= (int16_t)numCols)
			break;

		size = 0;
		err = DbCopyColumnValue(gAddrDB, rowID, colIDsP[currentColIndex], 0, NULL, &size);
		found = (Boolean) (err >= errNone && size);
	}

	if (found)
		*matchColIDP = colIDsP[currentColIndex];

Exit:
	if (sField2Indirect && colIDsP)
		MemPtrFree(colIDsP);

	return found;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupNextRecord
 *
 * DESCRIPTION: Seeks next record where a field1 column matches
 *		the lookup string and a field2 column is nonempty.
 *
 * PARAMETERS:
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         LYr   1/28/03  Initial revision
 *
 ***********************************************************************/
static Boolean PrvLookupNextRecord(Boolean newSearch, uint32_t *foundRowIDP, uint32_t *phoneColumnIDP)
{
	Boolean		found = false;
	uint32_t	savedRowID, savedPhoneColID;
	uint32_t	dataSize;

	savedRowID	= *foundRowIDP;
	savedPhoneColID	= *phoneColumnIDP;

	if (newSearch)
	{
		sSearchLibInfo.startRowIndex = kFirstRowIndex;
		sSearchLibInfo.startColumnIndex = 0;
		SearchLibSetStartIndexesById(&sSearchLibInfo, *foundRowIDP, 0);

		SearchLibReset(&sSearchLibInfo);
	}
	else
	{
		found = PrvLookupSeekInRecord(*foundRowIDP, false, phoneColumnIDP, sSearchLibInfo.recordDirection);
		if (!found)
			*phoneColumnIDP = kInvalidColumnID;
	}

	// Search for the lookup string
	while (!found)
	{
		if (!SearchLibSearch(&sSearchLibInfo))
			break; // Lookup text not found

		// If we find data in non-yomi fields we must ensure that there is
		// no data in the corresponding yomi field.
		if (sSearchLibInfo.textToSearch && *sSearchLibInfo.textToSearch)
		{
			// We only select entries where found text is displayed
			dataSize = 0;
			switch (sSearchLibInfo.foundColID)
			{
			case kAddrColumnIDWorkCompany:
			case kAddrColumnIDYomiCompanyName:
				DbCopyColumnValue(sSearchLibInfo.searchDB, sSearchLibInfo.foundRowID, kAddrColumnIDLastName, 0, NULL, &dataSize);
				if (gCurrentOrderByType == kOrderByNameType && dataSize)
					continue;
				break;

			case kAddrColumnIDFirstName:
			case kAddrColumnIDYomiFirstName:
				DbCopyColumnValue(sSearchLibInfo.searchDB, sSearchLibInfo.foundRowID, kAddrColumnIDWorkCompany, 0, NULL, &dataSize);
				if (gCurrentOrderByType == kOrderByCompanyType && dataSize)
					continue;
				break;

			default: // kAddrColumnIDLastName || kAddrColumnIDYomiLastName
				break;
			}
		}

		*foundRowIDP = sSearchLibInfo.foundRowID;
		found = PrvLookupSeekInRecord(sSearchLibInfo.foundRowID, true, phoneColumnIDP, sSearchLibInfo.recordDirection);
		if (!found)
			*phoneColumnIDP = kInvalidColumnID;
	}

	// if not found, restore arguments
	if (!found)
	{
		*foundRowIDP = savedRowID;
		*phoneColumnIDP = savedPhoneColID;
	}

	return found;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewUpdateScrollButtons
 *
 * DESCRIPTION: Show or hide the list view scroll buttons.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
static void PrvLookupViewUpdateScrollButtons(void)
{
	int16_t		row;
	uint16_t	upIndex;
	uint16_t	downIndex;
	uint32_t	rowID;
	uint32_t	columnID;
	Boolean		scrollableUp;
	Boolean		scrollableDown;
	TableType *	tableP;

	// Update the buttons that scroll the list.
	//
	// If the first record displayed is not the fist record in the category,
	// enable the up scroller.
	rowID = sTopVisibleRecord;
	columnID = sTopVisibleColumn;

	sSearchLibInfo.recordDirection = kSearchMoveBackward;
	scrollableUp =	PrvLookupNextRecord(true, &rowID, &columnID) &&
					PrvLookupNextRecord(false, &rowID, &columnID);


	// Find the record in the last row of the table
	tableP = ToolsGetFrmObjectPtr(sFormP, LookupTable);
	row = TblGetLastUsableRow(tableP);
	rowID = TblGetRowData(tableP, row);
	columnID = (uint32_t)TblGetItemPtr(tableP, row, 0);

	// If the last record displayed is not the last record in the category,
	// enable the down scroller.
	sSearchLibInfo.recordDirection = kSearchMoveForward;
	scrollableDown =	PrvLookupNextRecord(true, &rowID, &columnID) &&
						PrvLookupNextRecord(false, &rowID, &columnID);

	// Update the scroll button.
	upIndex = FrmGetObjectIndex (sFormP, LookupUpButton);
	downIndex = FrmGetObjectIndex (sFormP, LookupDownButton);
	FrmUpdateScrollers (sFormP, upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupLoadTable
 *
 * DESCRIPTION: This routine loads address book database records into
 *              the lookup view form.  Note that the phone field may
 *                be set to the first or last field if it isn't field1 or
 *                field2 and the table is loaded either forward or backward.
 *                So ignore it if phone are not being displayed.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
static void PrvLookupLoadTable (void)
{
	uint16_t	row;
	uint16_t	numRows;
	TablePtr	table;
	uint32_t	foundRowID;
	uint32_t	phoneColumnID;

	// For each row in the table, store the record number as the row id.
	table = ToolsGetFrmObjectPtr(sFormP, LookupTable);

	// Make sure we haven't scrolled too far down the list of records
	// leaving blank lines in the table.

	sSearchLibInfo.recordDirection = kSearchMoveForward;
	// Initialize data for PrvLookupNextRecord
	foundRowID = sTopVisibleRecord;
	phoneColumnID = sTopVisibleColumn;

	row = 0;
	while (row < sTableNumVisibleRows)
	{
		if (!PrvLookupNextRecord((Boolean)(row == 0), &foundRowID, &phoneColumnID))
			break;

		// Update top record info for 1st row
		if (row == 0)
		{
			sTopVisibleRecord = foundRowID;
			sTopVisibleColumn = phoneColumnID;
		}

		// Make the row usable.
		TblSetRowUsable (table, row, true);

		// Mark the row invalid so that it will draw when we call the
		// draw routine.
		TblMarkRowInvalid (table, row);

		// Store the record id as the row data.
		TblSetRowData(table, row, foundRowID);

		// Store the column id as the item ptr (only uint32_t available)
		// We need to do this because some records have multiple
		// occurances
		TblSetItemPtr (table, row, 0, (void*) phoneColumnID);

		++row;
	}

	// Hide the item that don't have any data.
	numRows = TblGetNumberOfRows(table);
	while (row < numRows)
	{
		TblSetRowUsable (table, row, false);
		row++;
	}

	TblUnhighlightSelection(table);

	PrvLookupViewUpdateScrollButtons();
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewScroll
 *
 * DESCRIPTION: This routine scrolls the list of names and phone numbers
 *              in the direction specified.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              direction - up or dowm
 *              oneLine   - if true the list is scroll by a single line,
 *                          if false the list is scroll by a full screen.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupViewScroll (WinDirectionType direction, Boolean oneLine)
{
	TableType *	tableP;
	uint16_t	rowsToScroll;
	uint16_t	row;
	uint32_t	newTopColID, colID = sTopVisibleColumn;
	uint32_t	newTopRowID, rowID = sTopVisibleRecord;

	tableP = ToolsGetFrmObjectPtr(sFormP, LookupTable);
	rowsToScroll = (oneLine ? 1 : sTableNumVisibleRows);

	if (direction == winDown)
	{
		row = TblGetLastUsableRow(tableP);

		if (row == tblUnusableRow)
			return false;

		newTopRowID = rowID = TblGetRowData(tableP, row);
		newTopColID = colID = (uint32_t)TblGetItemPtr(tableP, row, 0);

		sSearchLibInfo.recordDirection = kSearchMoveForward;

		if (PrvLookupNextRecord(true, &rowID, &colID))
		{
			row = 0;
			while (row < rowsToScroll && PrvLookupNextRecord(false, &rowID, &colID))
				++row;
		}

		rowID = newTopRowID;
		colID = newTopColID;

		if (row < sTableNumVisibleRows - 1)
		{
            sSearchLibInfo.recordDirection = kSearchMoveBackward;

			// Reset the search
			if (PrvLookupNextRecord(true, &rowID, &colID))
			{
				while ((row < sTableNumVisibleRows - 1) && PrvLookupNextRecord(false, &rowID, &colID))
					row++;
			}
		}
	}
	else // winUp
	{
		sSearchLibInfo.recordDirection = kSearchMoveBackward;

		// First call to intiallize the seach
		if (!PrvLookupNextRecord(true, &rowID, &colID))
			return false;

		row = 0;
		while (row < rowsToScroll && PrvLookupNextRecord(false, &rowID, &colID))
			++row;
	}

	// Avoid redraw if no change
	if (sTopVisibleRecord != rowID || sTopVisibleColumn != colID)
	{
		sTopVisibleRecord = rowID;
		sTopVisibleColumn = colID;
		PrvLookupLoadTable();
		ToolsFrmInvalidateWindow(LookupView);
		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewSelectRecord
 *
 * DESCRIPTION: Selects (highlights) a record on the table, scrolling
 *              the record if neccessary.  Also sets the CurrentRecord.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              recordNum - record to select
 *              phoneNum - phone in record to select
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
static void PrvLookupViewSelectRecord(uint32_t rowID, uint32_t colID)
{
	int16_t row, column;
	TablePtr tableP;
	Boolean recordFound;

	tableP = ToolsGetFrmObjectPtr(sFormP, LookupTable);

	// Don't change anything if the same record is selected
	if (TblGetSelection(tableP, &row, &column) &&
		rowID == TblGetRowData (tableP, row) &&
		colID == (uint32_t) TblGetItemPtr (tableP, row, 0))
	{
		return;
	}

	// See if the record is displayed by one of the rows in the table
	// A while is used because if TblFindRowData fails we need to
	// call it again to find the row in the reloaded table.
	while (true)
	{
		recordFound = false;
		if (TblFindRowData(tableP, rowID, &row))
		{
			if (colID == (uint32_t) TblGetItemPtr (tableP, row, 0))
			{
				recordFound = true;
				break; // match found
			}
		}

		// If the record is found in the existing table stop
		// and go select it.  Otherwise position that table to
		// start with the record, reload the table, and look
		// for it again.
		if (recordFound)
			break;

		// Scroll the view down placing the item
		// on the top row
		sTopVisibleRecord = rowID;
		sTopVisibleColumn = colID;
		PrvLookupLoadTable();
	}


	// Select the item if found
	if (recordFound && sSearchLibInfo.textToSearch)
		TblSelectItem (tableP, row, 0);

	sCurrentRecord = rowID;
	sCurrentColumn = colID;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewLookupString
 *
 * DESCRIPTION: Adds a character to LookupLookupField, looks up the
 * string in the database and selects the item that matches.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              event - EventPtr containing character to add to LookupLookupField
 *                        or NULL to use the text there
 *
 * RETURNED:    true if the field handled the event
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *			  meg		2/2/99	 added beepOnFail check before beeping
 ***********************************************************************/
static Boolean PrvLookupViewLookupString (EventType * event)
{
	FormPtr		frm;
	uint16_t	fldIndex;
	FieldPtr	fldP;
	char*		fldTextP = NULL;
	TablePtr	tableP;
	uint32_t	foundRowID;
	uint32_t	phoneColumnID;
	Boolean		found = false;
	size_t		fldTextLen;

	frm = FrmGetFormPtr(LookupView);
	fldIndex = FrmGetObjectIndex(frm, LookupLookupField);
	fldP = FrmGetObjectPtr (frm, fldIndex);

	if (event == NULL || FldHandleEvent (fldP, event))
	{
		if (sSearchLibInfo.textToSearch)
		{
			MemPtrFree(sSearchLibInfo.textToSearch);
			sSearchLibInfo.textToSearch = NULL;
			sSearchLibInfo.minCompareMatch = 0;
		}

		tableP = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, LookupTable));

		fldTextP = FldGetTextPtr(fldP);
		fldTextLen = FldGetTextLength(fldP);
		if (fldTextP && fldTextLen)
		{
			sSearchLibInfo.textToSearch = ToolsStrDup(fldTextP);
			sSearchLibInfo.minCompareMatch = fldTextLen;
		}

		sSearchLibInfo.recordDirection = kSearchMoveForward;

		foundRowID = phoneColumnID = 0;
		found = PrvLookupNextRecord(true, &foundRowID, &phoneColumnID);

		if (found)
		{
			PrvLookupLoadTable();
			PrvLookupViewSelectRecord(foundRowID, phoneColumnID);
			ToolsFrmInvalidateWindow(LookupView);
		}
		else if (!gDeviceHasFEP || TsmGetFepMode() == tsmFepModeOff) // Allow entering double-bytes characters when FEP is on
		{
			TblUnhighlightSelection(tableP);

			if (sBeepOnFail)
				SndPlaySystemSound (sndError);

			if (fldTextP && fldTextLen)
			{
				uint32_t	charSize = TxtGetPreviousChar(fldTextP, fldTextLen, NULL);
				FldDelete(fldP, fldTextLen - charSize, fldTextLen);
			}
		}

		return true;
	}

	// Event not handled
	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewUseSelection
 *
 * DESCRIPTION: Use the record currently selected.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    true if a record was selected and false if not.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   11/11/96   Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupViewUseSelection (void)
{
	TablePtr	table;
	int16_t		row;
	int16_t		column;


	// If a row is selected return the record number else
	// return noRecord.
	table = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, LookupTable));
	if (TblGetSelection (table, &row, &column))
	{
		sCurrentRecord = TblGetRowData(table, row);
		sCurrentColumn = (uint32_t) TblGetItemPtr(table, row, 0);
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewFormResize
 *
 * DESCRIPTION: Resize the form to fit the new bounds
 *
 * PARAMETERS:  newBoundsP: the new window bounds
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *         Name		Date		Description
 *         ----		----		-----------
 *         TEs		6/5/2003	Initial revision
 *
 ***********************************************************************/
static void PrvLookupViewFormResize(RectangleType *newBoundsP)
{
	RectangleType 	objBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	uint16_t		numberOfRows;
	int16_t			row;
	int16_t			xOffset;
	int16_t			yOffset;
	TableType*		tableP;
	FontID			currFont;
	Coord			width;

	xOffset =  newBoundsP->extent.x - gCurrentWinBounds.extent.x;
	yOffset =  newBoundsP->extent.y - gCurrentWinBounds.extent.y;

	// if the window has the height of the screen
	// then the windows is already at the right size
	if (!yOffset && !xOffset)
		return;

	tableP = (TableType*) FrmGetObjectPtr( sFormP, FrmGetObjectIndex (sFormP, LookupTable));

	// These objects move only horizontally
	if (xOffset)
	{
		// Set column width
		width = TblGetColumnWidth(tableP, field1Column);
		TblSetColumnWidth(tableP, field1Column, width + xOffset);
	}

	// These objects only move vertically
	if (yOffset)
	{
		//ListLookUpLabel
		objIndex = FrmGetObjectIndex(sFormP, LookupLookupLabel);
		FrmGetObjectPosition(sFormP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition(sFormP, objIndex, x, y);

		// ListLookupField
		objIndex = FrmGetObjectIndex(sFormP, LookupLookupField);
		FrmGetObjectPosition(sFormP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition(sFormP, objIndex, x, y);

		// LookupPasteButton
		objIndex =  FrmGetObjectIndex (sFormP, LookupPasteButton);
		FrmGetObjectPosition (sFormP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (sFormP, objIndex, x, y);

		// LookupCancelButton
		objIndex =  FrmGetObjectIndex (sFormP, LookupCancelButton);
		FrmGetObjectPosition (sFormP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (sFormP, objIndex, x, y);
	}

	// Stick the table bottom to the 'New' button.
	// -3: want a minimum of 3 pixel between the bottom of the table and the top of the buttom
	currFont = FntSetFont(stdFont);
	objIndex =  FrmGetObjectIndex (sFormP, LookupPasteButton);
	FrmGetObjectPosition (sFormP, objIndex, &x, &y);
	TblGetBounds(tableP, &objBounds);
	objBounds.extent.y = y - 3 - objBounds.topLeft.y;
	objBounds.extent.x += xOffset;
	objBounds.extent.y -= (objBounds.extent.y % FntLineHeight());
	TblSetBounds(tableP, &objBounds);
	FntSetFont(currFont);

	TblUnhighlightSelection(tableP);

	numberOfRows = TblGetNumberOfRows(tableP);

	for (row = 0; row < numberOfRows; row++)
		TblSetRowUsable(tableP, row, false);

	// Update the number of rows visible for the current screen size.
	sTableNumVisibleRows = objBounds.extent.y / TblGetRowHeight(tableP, 0);

	// LookupUpButton
	objIndex =  FrmGetObjectIndex (sFormP, LookupUpButton);
	FrmGetObjectPosition (sFormP, objIndex, &x, &y);
	x += xOffset;
	y += yOffset;
	FrmSetObjectPosition (sFormP, objIndex, x, y);

	// LookupDownButton
	objIndex =  FrmGetObjectIndex (sFormP, LookupDownButton);
	FrmGetObjectPosition (sFormP, objIndex, &x, &y);
	x += xOffset;
	y += yOffset;
	FrmSetObjectPosition (sFormP, objIndex, x, y);

	gCurrentWinBounds = *newBoundsP;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupDrawTableItemFocusRing
 *
 * DESCRIPTION: Draw the focus ring around the current selected row in table
 *
 * PARAMETERS:  formP, rowIndex
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name   	 Date      		Description
 * ----		------      	-----------
 * TEs   	06/23/2004     	Initial Revision
 *
 ***********************************************************************/
static void PrvLookupDrawTableItemFocusRing(TableType *tableP, int16_t tableRowIndex)
{
	RectangleType	bounds;

	if (!sOneHandedTableHasFocus || !sOneHandedRecordSelected)
		return;

	FrmClearFocusHighlight();
	TblGetItemBounds(tableP, tableRowIndex, 0, &bounds);
	FrmSetFocusHighlight(FrmGetWindowHandle(sFormP), &bounds, 0);
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupHandleNavigationUp
 *
 * DESCRIPTION: Handle UI Navigation for the Up key.
 *
 * PARAMETERS:  objectFocusMode:	Object Focus mode on or off
 *				repeat:				Auto repeat on or off
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/16/04	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupHandleNavigationUp(Boolean objectFocusMode, Boolean repeat)
{
	Boolean		scrolled = false;
	TablePtr	tableP;
	uint32_t	data;

	tableP = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, LookupTable));

	if (objectFocusMode)
	{
		if (!sOneHandedTableHasFocus)
			return false;

		if (sOneHandedRecordSelected)
		{
			if (sOneHandedLastSelectedRow > 0)
				sOneHandedLastSelectedRow--;
			else
			{
				data = TblGetRowData(tableP, sOneHandedLastSelectedRow);
				scrolled = PrvLookupViewScroll(winUp, true);

				if (!scrolled)
					goto Exit;
			}

			PrvLookupDrawTableItemFocusRing(tableP, sOneHandedLastSelectedRow);
			TblSelectItem (tableP, sOneHandedLastSelectedRow, 0);
			return true;
		}
		else scrolled = PrvLookupViewScroll(winUp, sOneHandedRecordSelected);
	}
	else scrolled = PrvLookupViewScroll(winUp, sOneHandedRecordSelected);

Exit:
	if (!scrolled && !repeat && (sOneHandedTableHasFocus || !objectFocusMode))
	{
		FrmSetNavState(sFormP, kFrmNavStateFlagsObjectFocusMode);
		FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, frmNavTitleID));
		FrmNavObjectTakeFocus(sFormP, frmNavTitleID);
		return true;
	}

	return scrolled || repeat;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupHandleNavigationDown
 *
 * DESCRIPTION: Handle UI Navigation for the Up key.
 *
 * PARAMETERS:  objectFocusMode:	Object Focus mode on or off
 *				repeat:				Auto repeat on or off
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/16/04	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupHandleNavigationDown(Boolean objectFocusMode, Boolean repeat)
{
	Boolean		scrolled = false;
	TablePtr	tableP;
	int16_t		row;
	uint32_t	data;

	tableP = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, LookupTable));

	if (objectFocusMode)
	{
		if (!sOneHandedTableHasFocus)
			return false;

		if (sOneHandedRecordSelected)
		{
			row = TblGetLastUsableRow(tableP);

			if (sOneHandedLastSelectedRow < row)
				sOneHandedLastSelectedRow++;
			else
			{
				data = TblGetRowData(tableP, sOneHandedLastSelectedRow);
				scrolled = PrvLookupViewScroll(winDown, true);

				if (!scrolled)
					goto Exit;
			}

			PrvLookupDrawTableItemFocusRing(tableP, sOneHandedLastSelectedRow);
			TblSelectItem (tableP, sOneHandedLastSelectedRow, 0);
			return true;
		}
		else scrolled = PrvLookupViewScroll(winDown, false);
	}
	else scrolled = PrvLookupViewScroll(winDown, false);


Exit:
	if (!scrolled && !repeat && (sOneHandedTableHasFocus || !objectFocusMode))
	{
		FrmSetNavState(sFormP, kFrmNavStateFlagsObjectFocusMode);
		FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, LookupCancelButton));
		FrmNavObjectTakeFocus(sFormP, LookupCancelButton);
		return true;
	}

	return scrolled || repeat;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupHandleNavigationRight
 *
 * DESCRIPTION: Handle UI Navigation for the Up key.
 *
 * PARAMETERS:  objectFocusMode:	Object Focus mode on or off
 *				repeat:				Auto repeat on or off
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/16/04	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupHandleNavigationRight(Boolean objectFocusMode, Boolean repeat)
{
	if (objectFocusMode)
	{
		if (!sOneHandedTableHasFocus)
			return false;
	}
	else FrmSetNavState(sFormP, kFrmNavStateFlagsObjectFocusMode);

	FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, LookupCancelButton));
	FrmNavObjectTakeFocus(sFormP, LookupCancelButton);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupHandleNavigationLeft
 *
 * DESCRIPTION: Handle UI Navigation for the Up key.
 *
 * PARAMETERS:  objectFocusMode:	Object Focus mode on or off
 *				repeat:				Auto repeat on or off
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/16/04	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupHandleNavigationLeft(Boolean objectFocusMode, Boolean repeat)
{
	Boolean	handled = false;

	if (objectFocusMode)
	{
		if (sOneHandedTableHasFocus && sOneHandedRecordSelected)
		{
			// Give the focus back to the whole table.
			sOneHandedRecordSelected = false;
			FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, LookupTable));
			FrmNavObjectTakeFocus(sFormP, LookupTable);
			handled = true;
		}
	}
	else
	{
		// force interaction mode to the 'Add' button.
		FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, frmNavTitleID));
		FrmSetNavState(sFormP, kFrmNavStateFlagsObjectFocusMode);
		FrmNavObjectTakeFocus(sFormP, frmNavTitleID);
		handled = true;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupHandleNavigationCenter
 *
 * DESCRIPTION: Handle UI Navigation for the Up key.
 *
 * PARAMETERS:  objectFocusMode:	Object Focus mode on or off
 *				repeat:				Auto repeat on or off
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/16/04	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupHandleNavigationCenter(Boolean objectFocusMode, Boolean repeat)
{
	uint32_t	rowID;
	uint32_t	columnID;
	Boolean		hasRecord;
	TablePtr	tableP;

	if (repeat)
		return true;

	tableP = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, LookupTable));

	if (!sOneHandedTableHasFocus)
		return false;

	rowID = sTopVisibleRecord;
	columnID = sTopVisibleColumn;

	sSearchLibInfo.recordDirection = kSearchMoveBackward;
	hasRecord =	PrvLookupNextRecord(true, &rowID, &columnID);

	if (!sOneHandedRecordSelected)
	{
		sOneHandedRecordSelected = true;
		sOneHandedLastSelectedRow = 0;

		if (hasRecord)
		{
			PrvLookupDrawTableItemFocusRing(tableP, sOneHandedLastSelectedRow);
			TblSelectItem (tableP, sOneHandedLastSelectedRow, 0);
		}

		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupHandleNavigation
 *
 * DESCRIPTION: Handle UI Navigation.
 *
 * PARAMETERS:  modifiers:	keyDown modifier
 *				chr:		keydown value
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/16/04	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvLookupHandleNavigation(uint16_t modifiers, wchar32_t chr)
{
	Boolean		objectFocusMode;
	Boolean		repeat;
	Boolean		handled = false;
	FrmNavStateFlagsType	navState;

	objectFocusMode = (Boolean) (FrmGetNavState(sFormP, &navState) >= errNone && (navState & kFrmNavStateFlagsObjectFocusMode) != 0);
	repeat = modifiers & autoRepeatKeyMask;

	switch(chr)
	{
		case vchrRockerUp:
			handled = PrvLookupHandleNavigationUp(objectFocusMode, repeat);
			break;

		case vchrRockerDown:
			handled = PrvLookupHandleNavigationDown(objectFocusMode, repeat);
			break;

		case vchrRockerLeft:
			handled = PrvLookupHandleNavigationLeft(objectFocusMode, repeat);
			break;

		case vchrRockerRight:
			handled = PrvLookupHandleNavigationRight(objectFocusMode, repeat);
			break;

		case vchrRockerCenter:
			handled = PrvLookupHandleNavigationCenter(objectFocusMode, repeat);
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewHandleEvent
 *
 * DESCRIPTION: This is the event handler for the "Lookup View"
 *              of the Address Book application.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    true if a record was selected and false if not.
 *
 * REVISION HISTORY:
 *         Name   	Date      		Description
 *         ----   	----      		-----------
 *         	Roger   	7/8/96   		Initial Revision
 *		CSS		06/22/99	Standardized keyDownEvent handling
 *									(TxtCharIsHardKey, commandKeyMask, etc.)
 *		ppl		02/06/02	Use switch
 *		ppl		02/06/02	Add Active Input Area Support
 *
 ***********************************************************************/
static Boolean PrvLookupViewHandleEvent (void)
{
	EventType	event;
	Boolean		handled;

	while (true)
	{
		handled = false;
		EvtGetEvent (&event, evtWaitForever);

		// Cancel if something is going to switch apps.

		if ( (event.eType == keyDownEvent)
			 &&	(!TxtCharIsHardKey(	event.data.keyDown.modifiers, event.data.keyDown.chr))
			 &&	(EvtKeydownIsVirtual(&event))
			 &&	(event.data.keyDown.chr == vchrFind))
		{
			EvtAddEventToQueue(&event);
			return false;
		}

		if (SysHandleEvent (&event))
			continue;


		switch (event.eType)
		{
			// Clear the lookup string because the user is selecting an item.
			case appStopEvent:
			{
				EvtAddEventToQueue(&event);
				return false;
			}

			case tblSelectEvent:
			{
				// Doing the next call sends a fldChangedEvent which remove's the
				// table's selection.  Set a flag to not handle the event.
				sIgnoreEmptyLookupField = true;

				handled = true;
				break;
			}

			case ctlSelectEvent:
			{
				switch (event.data.ctlSelect.controlID)
				{
					case LookupPasteButton:
						return PrvLookupViewUseSelection();

					case LookupCancelButton:
						return false;
				}
				break;
			}

			case ctlRepeatEvent:
			{
				switch (event.data.ctlSelect.controlID)
				{
					case LookupUpButton:
					{
						PrvLookupViewScroll (winUp, false);
						// leave unhandled so the buttons can repeat
						break;
					}

					case LookupDownButton:
					{
						PrvLookupViewScroll (winDown, false);
						// leave unhandled so the buttons can repeat
						break;
					}
				}
				break;
			}


			case keyDownEvent:
			{
				if (TxtCharIsHardKey(event.data.keyDown.modifiers, event.data.keyDown.chr))
				{
					// SysHandleEvent saw these keys and is now switching apps.
					// Leave the Lookup function.
					return false;
				}

				else if (EvtKeydownIsVirtual(&event))
				{
					if (TxtCharIsRockerKey(event.data.keyDown.modifiers, event.data.keyDown.chr))
					{
						Boolean		objectFocusMode;
						Boolean		repeat;
						FrmNavStateFlagsType	navState;

						objectFocusMode = (Boolean) (FrmGetNavState(sFormP, &navState) >= errNone && (navState & kFrmNavStateFlagsObjectFocusMode) != 0);
						repeat = event.data.keyDown.modifiers & autoRepeatKeyMask;

						switch(event.data.keyDown.chr)
						{
							case vchrRockerUp:
								handled = PrvLookupHandleNavigationUp(objectFocusMode, repeat);
								break;

							case vchrRockerDown:
								handled = PrvLookupHandleNavigationDown(objectFocusMode, repeat);
								break;

							case vchrRockerLeft:
								handled = PrvLookupHandleNavigationLeft(objectFocusMode, repeat);
								break;

							case vchrRockerRight:
								handled = PrvLookupHandleNavigationRight(objectFocusMode, repeat);
								break;

							case vchrRockerCenter:
								if (sOneHandedTableHasFocus && sOneHandedRecordSelected)
									return PrvLookupViewUseSelection();
								else handled = PrvLookupHandleNavigationCenter(objectFocusMode, repeat);
								break;
						}
					}
					else
					{
						switch(event.data.keyDown.chr)
						{
							case vchrPageUp:
							{
								PrvLookupViewScroll (winUp, false);
								handled = true;
								break;
							}

							case vchrPageDown:
							{
								PrvLookupViewScroll (winDown, false);
								handled = true;
								break;
							}
						}
					}
				}

				else if (event.data.keyDown.chr == chrLineFeed)
				{
					return PrvLookupViewUseSelection();
				}

				else
				{
					//user entered a new char...beep if it's not valid
					sBeepOnFail = true;
					handled = PrvLookupViewLookupString(&event);

					// If the field becomes empty, handle it.
					sIgnoreEmptyLookupField = false;
				}
				break;
			}


			case fldChangedEvent:
			{
				if (!(sIgnoreEmptyLookupField &&
					  FldGetTextLength(FrmGetObjectPtr (sFormP, FrmGetObjectIndex(sFormP, LookupLookupField))) == 0))
				{
					// dont set the beep...this way, if the calling app sent in a multiple char
					// string, we only beep once even if we will remove all chars from the lookup field
					PrvLookupViewLookupString(NULL);
				}

				sIgnoreEmptyLookupField = false;
				handled = true;
				break;
			}

			case winResizedEvent:
				PrvLookupViewFormResize(&event.data.winResized.newBounds);
				PrvLookupLoadTable();
				handled = true;
				break;

			case frmObjectFocusTakeEvent:
				if (event.data.frmObjectFocusTake.objectID == LookupTable)
				{
					RectangleType	bounds;
					uint16_t		objIndex;

					sOneHandedTableHasFocus = true;

					if (!sOneHandedRecordSelected)
					{
						objIndex = FrmGetObjectIndex(sFormP, LookupTable);
						FrmSetFocus(sFormP, objIndex);
						FrmGetObjectBounds(sFormP, objIndex, &bounds);
						FrmSetFocusHighlight(FrmGetWindowHandle(sFormP), &bounds, 0);
					}

					handled = true;
				}
				break;

			case frmObjectFocusLostEvent:
				if (event.data.frmObjectFocusLost.objectID == LookupTable)
				{
					sOneHandedTableHasFocus = false;

					if (sOneHandedRecordSelected)
						sOneHandedRecordSelected = false;
				}
				break;
			}

		// Check if the form can handle the event
		if (!handled)
			FrmHandleEvent (sFormP, &event);
	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewInit
 *
 * DESCRIPTION: This routine initializes the "Lookup View" of the
 *              Address application.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         	Name   	Date      		Description
 *         	----   	----      		-----------
 *         	Roger   	7/9/96   		Initial Revision
 *		ppl		02/06/02	Add Active Input Area support (AIA)
 *
 ***********************************************************************/
static void PrvLookupViewInit (void)
{
	uint16_t		row;
	uint16_t		rowsInTable;
	TablePtr		table;
/*	RectangleType	bounds, titleBounds;
	uint16_t		objectCount = FrmGetNumberOfObjects(sFormP);
	uint16_t		i;
*/
	// Initialize the address list table.
	table = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, LookupTable));
	sTableNumVisibleRows = rowsInTable = TblGetNumberOfRows (table);

	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle (table, row, field1Column, customTableItem);
		TblSetRowUsable (table, row, false);
	}

	TblSetColumnUsable (table, 0, true);

	// Set the callback routine that will draw the records.
	TblSetCustomDrawProcedure (table, field1Column, PrvLookupViewDrawRecord);

	// Load records into the address list.
	PrvLookupLoadTable ();
/*
	// Set the bounds of the title, so that the title will draw across the entire display.
	FrmGetFormInitialBounds(sFormP, &bounds);

 	for (i = 0; i < objectCount; i++)
 	{
		if (FrmGetObjectType(sFormP, i) == frmTitleObj)
		{
			FrmGetObjectBounds(sFormP, i, &titleBounds);
			titleBounds.extent.x = bounds.extent.x;
			FrmSetObjectBounds(sFormP, i, &titleBounds);
			break;
		}
 	}
*/
	// Respond to an empty lookup field.
	sIgnoreEmptyLookupField = false;

	// Onehanded navigation stuff
	sOneHandedTableHasFocus = false;
	sOneHandedRecordSelected = false;
}


/***********************************************************************
 *
 * FUNCTION:    Lookup
 *
 * DESCRIPTION: Present a list of records for the user to select and return
 * a string formatted to include information from the selected record.
 *
 * PARAMETERS:    paramsP - address lookup launch command parameters
 *
 * RETURNED:    nothing
 *                The paramsP will contain a string for the matching record.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/1/96   Initial Revision
 *			  meg		2/3/99	 added beep param to vars struct...default to true
 *			  peter	09/20/00	 Disable attention indicator because title is custom.
 ***********************************************************************/
void Lookup(AddrLookupParamsType * paramsP)
{
	FormPtr			frm;
	FormPtr			originalForm;
	Boolean			foundOne = false;
	CategoryID		cat = catIDAll;
	size_t			lookupTextLen = 0;
	size_t			charSize;
	uint16_t		savedAIAState;
	RectangleType	bounds;
 	uint16_t		lookupColumnIDsRscID;
	MemHandle		listH = NULL;

	if (!gAddrDB)
	{
		paramsP->resultStringH = 0;
		paramsP->uniqueID = 0;
		return;
	}

	// Check the parameters
	ErrFatalDisplayIf(paramsP->field1 > addrLookupListPhone && paramsP->field1 != addrLookupNoField, "Bad Lookup request - field1");
	ErrFatalDisplayIf(paramsP->field2 > addrLookupListPhone && paramsP->field2 != addrLookupNoField, "Bad Lookup request - field2");

	// Load the u32l resource list that contain the resource ID of the u32l that contains the list of coluns ID to use
	// The index in this u32l have to match the index of the sort index.
	lookupColumnIDsRscID = (uint16_t)U32ListGetItem(gApplicationDbP, OrderByLookupColumnIDResIDUIn32List, NULL, gOrderByIndex);
	if (!lookupColumnIDsRscID)
		return;

	// Load the Column ID resource list. If the list doesn't exist, search in all the table's columns
	if ((listH = U32ListGetList(gApplicationDbP, lookupColumnIDsRscID)) != NULL)
	{
		// Get the list items number.
		sDisplayNameNumCols = (uint16_t)U32ListGetItemNum(NULL, 0, listH);
		sDisplayNameColIDs = (uint32_t*)DmHandleLock(listH);
	}

	// Get the field2 column ID to look in.
	PrvLookupMapLookupFields(paramsP->field2, &sField2NumCols, &sField2ColIDsP, &sField2Indirect);

	// Initialize legacy LookupVariablesType structure
	memset(&sSearchLibInfo, 0, sizeof(SearchLibType));
	sParamsP		= paramsP;

	// Prepare lookup string
	if (paramsP->lookupString && *(paramsP->lookupString))
	{
		lookupTextLen = strlen(paramsP->lookupString);
		sSearchLibInfo.textToSearch = ToolsStrDup(paramsP->lookupString);
		sSearchLibInfo.minCompareMatch = lookupTextLen;
	}
	else
	{
		sSearchLibInfo.textToSearch = NULL;
	}

	sSearchLibInfo.searchDB			= gAddrDB;
	sSearchLibInfo.cursorID			= dbInvalidCursorID; // SearchLib to create the cursor
	sSearchLibInfo.schemaName		= kAddressDefaultTableName;
	sSearchLibInfo.orderBy			= gCurrentOrderByStr;
	sSearchLibInfo.startRowIndex	= kFirstRowIndex;
	sSearchLibInfo.cursorFlag		= 0;
	sSearchLibInfo.numCategories	= 1;
	sSearchLibInfo.catIDsP			= &cat;
	sSearchLibInfo.matchMode		= DbMatchAny;
	sSearchLibInfo.recordDirection	= kSearchMoveForward;
	sSearchLibInfo.columnDirection	= kSearchMoveForward;
	sSearchLibInfo.resumePolicy		= kSearchResumeChangeRecord;
	sSearchLibInfo.useCompare		= true;
	sSearchLibInfo.hidePrivate		= (Boolean) (gPrivateRecordVisualStatus == maskPrivateRecords);

	PrvLookupMapLookupFields(sParamsP->field1, &sSearchLibInfo.numCols, &sSearchLibInfo.colIDsP, &sSearchLibInfo.indirect);

	SearchLibInit(&sSearchLibInfo);

	// Only search in records if we have a text to search
	if (sSearchLibInfo.textToSearch && (*sSearchLibInfo.textToSearch))
	{
		// Check to see if we found just one answer
		// If so we skip presenting the user a lookup dialog and just use the match.
		foundOne = PrvLookupNextRecord(true, &sTopVisibleRecord, &sTopVisibleColumn);
		if (foundOne)
		{
			if (!PrvLookupNextRecord(false, &sTopVisibleRecord, &sTopVisibleColumn))
			{
				sCurrentRecord = sTopVisibleRecord;
				sCurrentColumn = sTopVisibleColumn;
				sParamsP->resultStringH = LookupCreateResultString(sSearchLibInfo.searchDB, sParamsP->formatStringP, sCurrentRecord);
				goto Exit;
			}

			sTopVisibleRecord = sTopVisibleColumn = 0;
		}
		else
		{
			if (sBeepOnFail)
				SndPlaySystemSound (sndError);
		}
	}

	// Reduce search string until record found our string empty
	while (!foundOne && sSearchLibInfo.textToSearch && (*sSearchLibInfo.textToSearch))
	{
		if (sSearchLibInfo.textToSearch)
		{
			charSize = TxtGetPreviousChar(paramsP->lookupString, lookupTextLen, NULL);
			lookupTextLen -= charSize;

			if (lookupTextLen)
			{
				sSearchLibInfo.textToSearch[lookupTextLen] = nullChr;
				sSearchLibInfo.minCompareMatch = lookupTextLen;
			}
			else
			{
				MemPtrFree(sSearchLibInfo.textToSearch);
				sSearchLibInfo.textToSearch = NULL;
				sSearchLibInfo.minCompareMatch = 0;
			}
		}

		foundOne = PrvLookupNextRecord(true, &sTopVisibleRecord, &sTopVisibleColumn);
	}

	// If the user isn't allowed to select a record then return without a match.
	if (!paramsP->userShouldInteract)
	{
		paramsP->resultStringH = 0;
		paramsP->uniqueID = 0;
		goto Exit;
	}

	// Show the AIA
	savedAIAState = PINGetInputAreaState();

	// Custom title doesn't support attention indicator.
	// Disable indicator before switching forms.
	AttnIndicatorEnable(false);

	// Remember the original form
	originalForm =  FrmGetActiveForm();

	// Initialize the dialog.
	frm = FrmInitForm (gApplicationDbP, LookupView);
	sFormP = frm;

	// Set the title
	if (paramsP->title)
		FrmSetTitle(frm, paramsP->title);

	// Set the paste button
	if (paramsP->pasteButtonText)
		CtlSetLabel (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, LookupPasteButton)), paramsP->pasteButtonText);

	FrmSetActiveForm(frm);
	FrmGetFormInitialBounds(frm, &gCurrentWinBounds);
	FrmGetFormBounds(frm, &bounds);
	PrvLookupViewFormResize(&bounds);

	PrvLookupViewInit();

	// Enter the lookup string
	if (lookupTextLen && paramsP->lookupString && *paramsP->lookupString)
	{
		FldInsert (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, LookupLookupField)), paramsP->lookupString, lookupTextLen);
		PrvLookupViewLookupString(NULL);
	}

	FrmSetFocus (frm, noFocus);

	// Handle events until the user picks a record or cancels
	if (PrvLookupViewHandleEvent ())
	{
		sParamsP->resultStringH = LookupCreateResultString(sSearchLibInfo.searchDB, sParamsP->formatStringP, sCurrentRecord);
		paramsP->uniqueID = sCurrentRecord;
	}
	else
	{
		paramsP->resultStringH = 0;
		paramsP->uniqueID = 0;
	}

	// Restore AIA
	if (savedAIAState != PINGetInputAreaState())
		PINSetInputAreaState(savedAIAState);

	AttnIndicatorEnable(true);		// Custom title doesn't support attention indicator.

	FrmSetFocus (frm, noFocus);
	FrmEraseForm (frm);
	FrmDeleteForm (frm);
	FrmSetActiveForm (originalForm);

Exit:
	if (sSearchLibInfo.textToSearch)
		MemPtrFree(sSearchLibInfo.textToSearch);

	PrvLookupReleaseLookupFields(sField2ColIDsP);
	PrvLookupReleaseLookupFields(sSearchLibInfo.colIDsP);

	DmHandleUnlock(listH);
	U32ListReleaseList(listH);

	SearchLibRelease(&sSearchLibInfo);
}
