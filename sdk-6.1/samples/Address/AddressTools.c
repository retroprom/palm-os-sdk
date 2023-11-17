/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressTools.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the place for all misc functions
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <AppMgr.h>
#include <ErrorMgr.h>
#include <ExgLocalLib.h>
#include <FontSelect.h>
#include <Form.h>
#include <Helper.h>
#include <HelperServiceClass.h>
#include <KeyMgr.h>
#include <Loader.h>
#include <NotifyMgr.h>
#include <StringMgr.h>
#include <SystemMgr.h>
#include <SystemResources.h>
#include <SysUtils.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <UIColor.h>
#include <LocaleMgr.h>
#include <Preferences.h>
#include <string.h>
#include <PalmTypesCompatibility.h> // For min & max
#include <Form.h>

#include "AddressCustom.h"
#include "AddressRsc.h"
#include "AddressTools.h"
#include "Address.h"
#include "AddressTransfer.h"
//#include "AddressDefines.h"
#include "AddressDBSchema.h"
#include "AddressExportAsk.h"
#include "AddressU32List.h"

/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Abbreviations: String list indexes
#define kHomeFamilySuffixIndex		0
#define kCorpFamilySuffixIndex		1
#define kOtherFamilySuffixIndex		2

#define kPhoneVoiceStrIndex			0
#define kPhoneMobileStrIndex		1
#define kPhoneFaxStrIndex			2
#define kPhonePagerStrIndex			3
#define kPhoneAssistantStrIndex		4
#define kInternetEmailStrIndex		5
#define kInstantMessagingStrIndex	6

/***********************************************************************
 *
 * FUNCTION:	ToolsIsDialerPresent
 *
 * DESCRIPTION:	This routine check if a dialer is present
 *				Once check has been made, a global stores this info so
 *				that further callss are immediate
 *
 * PARAMETERS:	none
 *
 * RETURNED:	true if a dialer is present
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * aro		06/12/00	Initial Revision
 * LFe		10/21/02	Use new Notify Manager.
 *
 ***********************************************************************/
Boolean	ToolsIsDialerPresent( void )
{
	SysNotifyParamType			param;
	HelperNotifyEventType		details;
	HelperNotifyValidateType	validate;

	if (!gDialerPresentChecked)
	{
		param.notifyType		= sysNotifyHelperEvent;
		param.broadcaster		= sysFileCAddress;
		param.notifyDetailsP	= &details;
		param.handled			= false;

		details.version			= kHelperNotifyCurrentVersion;
		details.actionCode		= kHelperNotifyActionCodeValidate;
		details.data.validateP	= &validate;

		validate.serviceClassID = kHelperServiceClassIDVoiceDial;
		validate.helperAppID	= 0;

		SysNotifyBroadcast(&param);

		gDialerPresent = param.handled;

		gDialerPresentChecked = true;
	}

	return gDialerPresent;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetAbbreviationFromProperty
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:    The family suffix (H / W / O)
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PLe		12/03/02	Initial Revision
 *
 ***********************************************************************/
static void PrvToolsGetFamilySuffix(uint32_t property, char * familySuffixStrP)
{
	// Get the suffix using the family
	switch (property & kFieldFamilyMask)
	{
		case kFieldFamily_Home:
			SysStringByIndex(gApplicationDbP, FamilySuffixStrList, kHomeFamilySuffixIndex, familySuffixStrP, kMaxFamilySuffixSize);
			break;
		case kFieldFamily_Corp:
			SysStringByIndex(gApplicationDbP, FamilySuffixStrList, kCorpFamilySuffixIndex, familySuffixStrP, kMaxFamilySuffixSize);
			break;
		case kFieldFamily_Other:
			SysStringByIndex(gApplicationDbP, FamilySuffixStrList, kOtherFamilySuffixIndex, familySuffixStrP, kMaxFamilySuffixSize);
			break;
		default:
			strcpy(familySuffixStrP, "");
	}
}

/***********************************************************************
 *
 * FUNCTION:    ToolsBuildDateAndTimeString
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		09/25/03	Initial Revision
 *
 ***********************************************************************/
char* ToolsBuildDateAndTimeString(DateTimeType* dateTimeP)
{
	DateFormatType 	dateFormat;
	TimeFormatType 	timeFormat;
	MemHandle		templateH;
	char*			resultP = NULL;
	char			dateStr[dateStringLength + 1];
	char			timeStr[timeStringLength + 1];

	dateFormat = (DateFormatType) PrefGetPreference(prefDateFormat);
	DateToAscii( (uint8_t) dateTimeP->month, (uint8_t) dateTimeP->day, (uint16_t) dateTimeP->year,	dateFormat, dateStr);

	timeFormat = (TimeFormatType) PrefGetPreference(prefTimeFormat);
	TimeToAscii((uint8_t) dateTimeP->hour, (uint8_t)  dateTimeP->minute,  timeFormat, timeStr);

	if ((templateH = DmGetResource(gApplicationDbP, strRsc, DateAndTimeTemplate)) != NULL)
	{
		resultP = TxtParamString(DmHandleLock(templateH), dateStr, timeStr, NULL, NULL);
		DmHandleUnlock(templateH);
		DmReleaseResource(templateH);
	}

	return resultP;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetAbbreviationFromProperty
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PLe		12/03/02	Initial Revision
 *
 ***********************************************************************/
void ToolsGetAbbreviationFromProperty(char * abbrevStringP, uint32_t property)
{
	uint16_t	indexInStringList = 0xFFFF;
	char		familySuffixStr[kMaxFamilySuffixSize];
	char*		abbrevResultP;

	// Get the abbreviation index using the kind property
	switch (property & kFieldTypeMask)
	{
		case kFieldType_Extended | kFieldType_Phone:
		case kFieldType_Phone:
			switch (property & kFieldKindMask)
			{
				case kFieldKind_VoicePhone:
					indexInStringList = kPhoneVoiceStrIndex;
					break;
				case kFieldKind_MobilePhone:
					indexInStringList = kPhoneMobileStrIndex;
					break;
				case kFieldKind_FaxPhone:
					indexInStringList = kPhoneFaxStrIndex;
					break;
				case kFieldKind_PagerPhone:
					indexInStringList = kPhonePagerStrIndex;
					break;
				case kFieldKind_AssistantPhone:
					indexInStringList = kPhoneAssistantStrIndex;
					break;
			}
			break;

		case kFieldType_Extended | kFieldType_Internet:
		case kFieldType_Internet:
			if ((property & kFieldKindMask) == kFieldKind_EmailInternet)
				indexInStringList = kInternetEmailStrIndex;
			break;

		case kFieldType_Extended | kFieldType_InstantMessaging:
		case kFieldType_InstantMessaging:
			indexInStringList = kInstantMessagingStrIndex;
			break;
	}

	// Get the abbreviation from resources
	if (indexInStringList != 0xFFFF)
	{
		SysStringByIndex(gApplicationDbP, AbbreviationsByKindStrList, indexInStringList, abbrevStringP, kMaxAbbreviationSize);
		PrvToolsGetFamilySuffix(property, familySuffixStr);
		abbrevResultP = TxtParamString(abbrevStringP, familySuffixStr, NULL, NULL, NULL);
		StrLCopy(abbrevStringP, abbrevResultP, kMaxAbbreviationSize);
		MemPtrFree(abbrevResultP);
	}
	else *abbrevStringP = chrNull;
}


/***********************************************************************
 *
 * FUNCTION:    ToolsComputePhoneLabelWidth
 *
 * DESCRIPTION: Compute the phone label width acording to the list font.
 *
 * PARAMETERS:	dbRef		- Application database ref
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * PLe		04/24/03	Initial revision
 *
 ***********************************************************************/
void ToolsComputePhoneLabelWidth(void)
{
	char		abbrStr[kMaxAbbreviationSize];
	char		familyStr[kMaxAbbreviationSize];
	FontID		fontID;
	uint16_t	abbrIndex;
	uint16_t	familyIndex;
	char *		resultP;
	Coord		width;

	fontID = FntSetFont(gAddrListFont);

	gPhoneLabelWidth = 0;

	abbrIndex = 0;

	while(true)
	{
		SysStringByIndex(gApplicationDbP, AbbreviationsByKindStrList, abbrIndex++, abbrStr, kMaxAbbreviationSize);

		if (*abbrStr == chrNull)
			break;

        familyIndex = 0;

		while(true)
		{
			SysStringByIndex(gApplicationDbP, FamilySuffixStrList, familyIndex++, familyStr, kMaxAbbreviationSize);

			if (*familyStr == chrNull)
				break;

			resultP = TxtParamString(abbrStr, familyStr, NULL, NULL, NULL);

			if (!resultP)
				continue;

			if (*resultP)
			{
				width = FntCharsWidth(resultP, strlen(resultP));

				if (width > gPhoneLabelWidth)
					gPhoneLabelWidth = width;
			}

			MemPtrFree(resultP);
		}
	}

	FntSetFont(fontID);
}

/***********************************************************************
 *
 * FUNCTION:    ToolsStrDup
 *
 * DESCRIPTION: Duplicate a string
 *
 * PARAMETERS:	->	srcP - The string to duplicate
 *
 * RETURNED:	the duplicated string
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * PLe		04/14/03	Initial revision
 *
 ***********************************************************************/
char *ToolsStrDup(char *srcP)
{
	char *dstP;

	if (!srcP)
	{
		ErrNonFatalDisplay("ToolsStrDup: NULL string passed!");
		return NULL;
	}

	if ((dstP = MemPtrNew((uint32_t)(strlen(srcP) + 1))) != NULL)
		strcpy(dstP, srcP);

	return dstP;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsCopyStringResource
 *
 * DESCRIPTION: Duplicate a resource string
 *
 * PARAMETERS:	->	stringResourceID - ID of the string resource to duplicate
 *
 * RETURNED:	the allocated & duplicated string
 *
 * REVISION HISTORY:
 *
 * Name 	Date		Description
 * -----	--------	-----------
 * LFe		09/24/03	Initial revision
 *
 ***********************************************************************/
char* ToolsCopyStringResource (DmResourceID stringResourceID)
{
	MemHandle 	stringH;
	char*		stringP = NULL;

	if ((stringH = DmGetResource(gApplicationDbP, strRsc, stringResourceID)) != NULL)
	{
		stringP = DmHandleLock (stringH);
		stringP = ToolsStrDup(stringP);
		DmHandleUnlock (stringH);
		DmReleaseResource (stringH);
	}

	return stringP;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetRecordNameStr
 *
 * DESCRIPTION: Returns an allocated string containing the record name
 *				formated for the ListView. Also used by the DialList dialog.
 *				It's the caller responsability to freed the returned string.
 *
 * PARAMETERS:	->	dbP: Database to get the record in.
 				->	rowID: the row ID
 *				->	fontID: the font to use
 *				->	maxWidth: the maximum width for the name in pixel (or 0 if no limit)
 *
 * RETURNED:	The string containing the name
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		04/30/03	Initial revision
 *
 ***********************************************************************/
char *ToolsGetRecordNameStr(DmOpenRef dbP, uint32_t rowID, FontID fontID, Coord maxWidth)
{
	uint32_t*	columnIDsP = NULL;
	MemHandle	columnIDResH = NULL;
	uint32_t	numColumns = 0;
	uint16_t	layoutRscID;
	uint16_t	i;
	char *		nameP;
	FontID		currFont;
	Boolean		addEllipsis;

	ViewDisplayLayoutInfoType	layoutInfo;
	ViewDiplayLayoutRowTextType	rowText;

	currFont = FntSetFont(fontID);

	// Load the u32l resource list that contain the NameDisplaySort Layout u32l resouce ID to use
	// The index in this u32l have to match the index of the sort index.
	// This resource contains the DisplayName base ID
	layoutRscID = (uint16_t)U32ListGetItem(gApplicationDbP, OrderByDisplayLayoutResIDUIn32List, NULL, gOrderByIndex);
	if (!layoutRscID)
		return NULL;

	if (!gDisplayNameColumnListResID)
		gDisplayNameColumnListResID = ToolsGetCountryBaseResID(kU32ListResType, layoutRscID, kAddrTemplateResIDIncrement);

	if ((columnIDResH = U32ListGetList(gApplicationDbP, gDisplayNameColumnListResID)) != NULL)
	{
		columnIDsP = DmHandleLock(columnIDResH);
		numColumns = U32ListGetItemNum(NULL, 0, columnIDResH);
	}

	// Get the DisplayName Layout, country base.
	if (!gDisplayNameLayoutResID)
		gDisplayNameLayoutResID = ToolsGetCountryBaseResID(kViewBlockLayoutRscType, layoutRscID, kAddrTemplateResIDIncrement);

	layoutInfo.dbP			= dbP;
	layoutInfo.rowID		= rowID;
	layoutInfo.columnIDsP	= columnIDsP;
	layoutInfo.numColumns	= numColumns;
	layoutInfo.layoutRscID	= gDisplayNameLayoutResID;
	layoutInfo.fontID		= fontID;

	if (AddrDisplayLayoutsInit(&layoutInfo))
		AddrDisplayLayoutsGetRowText(&layoutInfo, 0, &rowText);
	else
		// No layout found. Display fields with a blank space as separator
		AddrDisplayLayoutsGetDefaultText(&layoutInfo, &rowText);

	// Nothing to display. Load the Unnamed string
	if (!rowText.numFields)
	{
		size_t len = gUnnamedRecordStringP ? strlen(gUnnamedRecordStringP) : 0;

		if (!rowText.fieldTextPP)
			rowText.fieldTextPP = MemPtrNew(sizeof(char*));

		rowText.fieldTextPP[0] = ToolsStrDup(gUnnamedRecordStringP);
		rowText.textWidth = gUnnamedRecordStringP ? FntCharsWidth(gUnnamedRecordStringP, len) : 0;
		rowText.numFields = 1;
		rowText.numBytes = len;
	}

	nameP = MemPtrNew((uint32_t)rowText.numBytes + 1);
	ErrFatalDisplayIf(!nameP, "Out of memory!");
	*nameP = 0;

	// maxWidth == 0 || Enough room to draw the entire name
	if (!maxWidth || rowText.textWidth <= maxWidth)
	{
		for (i = 0; i < rowText.numFields; i++)
			strcat(nameP, rowText.fieldTextPP[i]);
	}
	else
	{
		// Not enough room... Truncate strings
		Coord		remainingWidth;
		Coord		secondPartNameWidth;
		Coord		width;
		Boolean		truncated = false;

		// Get max width (in percent) to use for the first name. Use 2/3 if not specified in resource
		if (!rowText.fieldMaxWidthPercentP ||
			rowText.fieldMaxWidthPercentP[0] < 1 || rowText.fieldMaxWidthPercentP[0] > 100)
		{
			// If rowText.fieldMaxWidthPercentP is not null, an incorrect value has been specified in resources
            ErrNonFatalDisplayIf(rowText.fieldMaxWidthPercentP, "Bad value for name first field max width");
			width = maxWidth * 2 / 3;
		}
		else
		{
			width = maxWidth * rowText.fieldMaxWidthPercentP[0] / 100;
		}

		// Grow up the first column width if the remaining name fits in less than 1/3 width
		remainingWidth = maxWidth - width;
		secondPartNameWidth = rowText.textWidth - FntCharsWidth(rowText.fieldTextPP[0], strlen(rowText.fieldTextPP[0]));

		if (remainingWidth > secondPartNameWidth)
			width += remainingWidth - secondPartNameWidth;

		remainingWidth = maxWidth;

		for (i = 0; i < rowText.numFields; i++)
		{
			addEllipsis = (Boolean) (i == 0 && rowText.numFields > 1); // Ellipsis only for first part and it's followed by a second part.
			truncated = FntTruncateString(rowText.fieldTextPP[i], rowText.fieldTextPP[i], fontID, width, addEllipsis);
			strcat(nameP, rowText.fieldTextPP[i]);
			remainingWidth -= FntCharsWidth(rowText.fieldTextPP[i], strlen(rowText.fieldTextPP[i]));
			width = remainingWidth;

			if (truncated && i > 0)
				break;
		}
	}

	AddrDisplayLayoutsFreeRowText(&rowText);
	AddrDisplayLayoutsFree(&layoutInfo);

	if (columnIDResH)
	{
		DmHandleUnlock(columnIDResH);
		U32ListReleaseList(columnIDResH);
	}

	FntSetFont(currFont);

	return nameP;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetPhoneNumber
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		04/30/03	Initial revision
 *
 ***********************************************************************/
char* ToolsGetPhoneNumber(uint32_t recordID, uint32_t phoneColID, FontID fontID, Coord maxWidth, Coord* phoneWidth, char** phoneAbbrev)
{
	FontID		currFont;
	uint32_t	size;
	void *		colDataP;
	char		abbreviation[kMaxAbbreviationSize];
	char *		phoneP = NULL;
	uint32_t *	phonePropertyP = NULL;

	currFont = FntSetFont(fontID);

	*phoneWidth = 0;

	if (phoneColID != kInvalidColumnID && DbGetColumnValue(gAddrDB, recordID, phoneColID, 0, &colDataP, &size) >= errNone)
	{
		phoneP = ToolsStrDup(colDataP);
		DbReleaseStorage(gAddrDB, colDataP);

		ToolsStripNonPrintableChar(phoneP);

		FntTruncateString(phoneP, phoneP, fontID, maxWidth, true);
		*phoneWidth = FntCharsWidth(phoneP, strlen(phoneP));

		if (phoneAbbrev)
		{
			// Get phone fieldType
			DbGetColumnPropertyValue(gAddrDB, kAddressDefaultTableName, phoneColID,	kAddrColumnPropertyFieldInfo, &size, (void**)&phonePropertyP);

			if (phonePropertyP)
			{
				ToolsGetAbbreviationFromProperty(abbreviation, *phonePropertyP);
				DbReleaseStorage(gAddrDB, phonePropertyP);
				*phoneAbbrev = ToolsStrDup(abbreviation);
			}
		}
	}

	FntSetFont(currFont);

	return phoneP;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsDrawRecordNameAndPhoneNumber
 *
 * DESCRIPTION: Draws the name and phone number (plus which phone)
 *					 within the screen bounds passed.
 *
 * PARAMETERS:  rowID - row ID or cursor ID positioned on a valid row
 *              bounds - bounds of the draw region
 *              phoneLabelLetters - the first letter of each phone label
 *              sortByCompany - true if the database is sorted by company
 *              unnamedRecordStringPtr - string to use for unnamed records
 *
 * RETURNED:    x coordinate where phone number starts
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 * roger	06/21/95	Initial Revision
 * peter	05/09/00	Added result and eliminated destructive change to bounds
 * fpa		11/02/00	Added unnamedRecordStringH parameter in order to prevent memory leaks
 * LFe		11/05/02	Remove record/phoneLabel/sortByCompany/unnamed Ptr&H as parameters
 *						Add recordNum and sortID as new parameters
 *						Use the new Data Mgr schema database.
 *
 ***********************************************************************/
uint16_t ToolsDrawRecordNameAndPhoneNumber(uint32_t rowID, RectangleType *boundsP, FontID fontID, Boolean customDraw, Boolean phoneHighlight)
{
	FontID					currFont;
	Coord					x, y;
	Coord					phonePosX;
	char *					phoneP = NULL;
	char *					nameP = NULL;
	char *					abbreviationP = NULL;
	Coord					phoneWidth = 0;
	uint32_t				phoneColID;
	RectangleType			nameBounds;

	// If customDraw is false, then we let the system handle the highlight of the row. We just draw the text.
	// If customDraw is true, the line is drawned unhighlighted except the phone number, depending of the phoneHighlight parameter.

	// Get the phone number
	// Load the column in which the phone column ID is store...
	phoneColID = AddressDBCheckAndSetDisplayedPhoneColumn(gAddrDB, rowID);
	phoneP = ToolsGetPhoneNumber(rowID, phoneColID, fontID, kMaxPhoneColumnWidth, &phoneWidth, &abbreviationP);

	x = boundsP->topLeft.x;
	y = boundsP->topLeft.y;

	nameBounds = *boundsP;
	nameBounds.extent.x -= gPhoneLabelWidth + phoneWidth + kSpaceBetweenPhoneNumbersAndAbbreviation;
	phonePosX = 0;

	if (phoneWidth)
	{
		phonePosX = x + nameBounds.extent.x - nameBounds.topLeft.x;
		nameBounds.extent.x -= kSpaceBetweenNamesAndPhoneNumbers;
	}

	// Draw the name
	nameP = ToolsGetRecordNameStr(gAddrDB, rowID, fontID, nameBounds.extent.x);

	currFont = FntSetFont(fontID);

	if (customDraw)
	{
		// Set back to unselected values
		WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
		WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
		WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));
		WinEraseRectangle(boundsP, 0);
	}

	if (nameP)
	{
		if (customDraw)
			ToolsDrawTextLabel(nameP, strlen(nameP), nameBounds.topLeft.x, nameBounds.topLeft.y, false);
		else
			WinDrawChars(nameP, (int16_t)strlen(nameP), nameBounds.topLeft.x, nameBounds.topLeft.y);

		MemPtrFree(nameP);
	}

	// Draw the phone number
	if (phoneP)
	{
		if (customDraw)
			ToolsDrawTextLabel(phoneP, strlen(phoneP), phonePosX, y, phoneHighlight);
		else
			WinDrawChars(phoneP, (int16_t)strlen(phoneP), phonePosX, y);

		MemPtrFree(phoneP);

		x = phonePosX + phoneWidth + kSpaceBetweenPhoneNumbersAndAbbreviation;

		// find out if the first letter of the phone label is an O(ther) or
		// E(mail). If it is email don't draw the letter. If it is other, and the
		// contents of the phone field is not a number, don't draw the letter.
		if (abbreviationP)
		{
			if (customDraw)
				ToolsDrawTextLabel(abbreviationP, strlen(abbreviationP), x, y, false);
			else
				WinDrawChars(abbreviationP, (int16_t)strlen(abbreviationP), x, y);

			MemPtrFree(abbreviationP);
		}
	}

	FntSetFont(currFont);

	return phonePosX;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetLabelColumnWidth
 *
 * DESCRIPTION: Calculate the width of the widest field label plus a ':'.
 *
 * PARAMETERS:	rowID:				DB row unique ID or cursor ID
 *				bookInfo:			Pointer to the tabs book info structure
 *				tabIndex:			The tab index to show
 *             		labelFontID:			font used to draw the label
 *				onlyViewableLabel:		Get width for the label that will
 *									be displayed in the display view.
 *
 * RETURNED:    width of the widest label.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		01/30/98	Initial Revision
 * TEs		07/22/02	Added tab support
 * LFe		10/28/02	New tab info structure
 *
 ***********************************************************************/
Coord ToolsGetLabelColumnWidth(uint32_t rowID, AddressBookInfoType* bookInfo, uint32_t tabIndex, FontID labelFontID, Boolean onlyViewableLabel)
{
	int32_t		i;
	uint16_t	labelWidth;    	 // Width of a field label
	uint16_t	columnWidth;    	// Width of the label column (fits all label)
	uint16_t	numElement;
	FontID		curFont;
	char *		label;
	uint32_t	size;
	MemHandle	colonResourceH;
	char*		colonStrP;
	AddressTabColumnPropertiesType *	columnPropertiesP;

	if ((colonResourceH = DmGetResource(gApplicationDbP, strRsc, LabelSuffixColonStr)) != NULL)
		colonStrP = DmHandleLock(colonResourceH);
	else colonStrP = ":";

	// Calculate column width of the label column which is used by the Record View and the Edit View.
	curFont = FntSetFont (labelFontID);
	columnWidth = 0;
	numElement = bookInfo->tabs[tabIndex].numElement;

	for (i = 0; i < numElement; i++)
	{
		columnPropertiesP = &bookInfo->tabs[tabIndex].columnsPropertiesP[i];

		size = 0;
		DbCopyColumnValue(gAddrDB, rowID, columnPropertiesP->columnId, 0, NULL, &size);

		if (onlyViewableLabel && (columnPropertiesP->noLabel || !size))
			continue;

		if (columnPropertiesP->renamedLabelP && *columnPropertiesP->renamedLabelP)
			label = columnPropertiesP->renamedLabelP;
		else
			label = columnPropertiesP->labelP;

		if (!label)
			continue;

		labelWidth = FntCharsWidth(label, strlen(label));
		columnWidth = max(columnWidth, labelWidth);
	}

	// Label width + colon str size + space size between label & data
	columnWidth += FntCharsWidth(colonStrP, strlen(colonStrP)) + FntCharWidth(spaceChr);

	FntSetFont (curFont);

	if (colonResourceH)
	{
		DmHandleUnlock(colonResourceH);
		DmReleaseResource(colonResourceH);
	}

	return columnWidth;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsChangeCategory
 *
 * DESCRIPTION: This routine updates the global varibles that keep track
 *              of category information.
 *
 * PARAMETERS:  category  - new category (id)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name   Date      Description
 * ----  ------		-----------
 * art   6/5/95     Initial Revision
 *
 ***********************************************************************/
void ToolsChangeCategory(CategoryID *categoriesP, uint32_t numCategories)
{
	uint32_t		size;
	CategoryID *	newCategoriesP = NULL;

	size = numCategories * sizeof(CategoryID);

	if (size)
	{
		if ((newCategoriesP = MemPtrNew(size)) == NULL)
			return;

		memmove(newCategoriesP, categoriesP, size);
	}

	gCurrentNumCategories = numCategories;

	if (gCurrentCategoriesP)
	{
		MemPtrFree(gCurrentCategoriesP);
		gCurrentCategoriesP = NULL;
	}

	gCurrentCategoriesP = newCategoriesP;

	// Re-open the cursor so it only contains rows in the new category set
	AddressDBOpenCursor();

	gTopVisibleRowIndex = kFirstRowIndex;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsCursorMoveNext
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 *
 ***********************************************************************/
Boolean ToolsCursorMoveNext(void)
{
	if (DbCursorMoveNext(gCursorID) < errNone)
	{
		DbCursorMoveLast(gCursorID);
		return false;
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsCursorMovePrevious
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 *
 ***********************************************************************/
Boolean ToolsCursorMovePrevious(void)
{
	if (DbCursorMovePrev(gCursorID) < errNone)
	{
		DbCursorMoveFirst(gCursorID);
		return false;
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsSelectFont
 *
 * DESCRIPTION: This routine handles selection of a font
 *
 * PARAMETERS:  currFontID - id of current font
 *
 * RETURNED:    id of new font
 *
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		09/10/97	Initial Revision
 *
 ***********************************************************************/
FontID ToolsSelectFont (uint16_t formID, FontID currFontID)
{
	FontID fontID;

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (currFontID);

	if (fontID != currFontID)
		ToolsFrmInvalidateWindow(formID);

	return fontID;
}

/***********************************************************************
 *
 * FUNCTION:    PrvToolsIsPhoneField
 *
 * DESCRIPTION: Test if a field is a "phone" field, id a field with the
 *				falg Phone or Internet set in the fieldinfo property.
 *
 * PARAMETERS:  ->	fieldInfo: the field info from the schema definiton
 *				->	strictPhone: if true, ignore email and instant messaging fields
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		9/9/2003	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvToolsIsPhoneField(uint32_t fieldInfo, Boolean strictPhone)
{
	return	(Boolean)	((fieldInfo & kFieldType_Phone) == kFieldType_Phone ||
						(!strictPhone &&
							((fieldInfo & (kFieldType_Internet | kFieldKind_EmailInternet)) == (kFieldType_Internet | kFieldKind_EmailInternet) ||
							(fieldInfo & kFieldType_InstantMessaging) == kFieldType_InstantMessaging)));
}

/***********************************************************************
 *
 * FUNCTION:    ToolsIsPhoneFieldByIndex
 *
 * DESCRIPTION: Test if a field is a "phone" field, id a field with the
 *				falg Phone or Internet set in the fieldinfo property.
 *
 * PARAMETERS:  ->	tabIndex:	In which tab is the field to test
 *				->	fieldIndex:	Index of the field in the tab to test
 *				->	strictPhone: if true, ignore email and instant messaging fields
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		11/18/02	Initial Revision
 *
 ***********************************************************************/
Boolean ToolsIsPhoneFieldByIndex(uint16_t tabIndex, uint16_t fieldIndex, Boolean strictPhone)
{
	uint32_t	fieldInfo;

	if (fieldIndex > gBookInfo.tabs[tabIndex].numElement)
	{
		DbgOnlyFatalError("Field index out of range");
		return false;
	}

	fieldInfo = gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].fieldInfo;

	return PrvToolsIsPhoneField(fieldInfo, strictPhone);
}

/***********************************************************************
 *
 * FUNCTION:    ToolsIsPhoneFieldByColId
 *
 * DESCRIPTION: Test if a field is a "phone" field, id a field with the
 *				flag Type Phone or Type Internet and kind mail set in the
 *				 'fieldinfo' property.
 *
 * PARAMETERS:  ->	columnID:	Test if a column is a "phone" field.
 *				->	strictPhone: if true, ignore email and instant messaging fields
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		11/18/02	Initial Revision
 *
 ***********************************************************************/
Boolean ToolsIsPhoneFieldByColId(uint32_t columnID, Boolean strictPhone)
{
	AddressTabColumnPropertiesType*	propP;

	if ((propP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID)) != NULL)
		return PrvToolsIsPhoneField(propP->fieldInfo, strictPhone);

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsIsYomiFieldByIndex
 *
 * DESCRIPTION: Test if a field is a "Yomi" field, id a field with the
 *				flag kFieldType_Yomi set in the fieldinfo property.
 *
 * PARAMETERS:  tabIndex:	In which tab is the field to test
 *				fieldIndex:	Index of the field in the tab to test
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		02/25/03	Initial Revision
 *
 ***********************************************************************/
Boolean ToolsIsYomiFieldByIndex(uint16_t tabIndex, uint16_t fieldIndex)
{
	uint32_t	fieldInfo;

	if (fieldIndex > gBookInfo.tabs[tabIndex].numElement)
	{
		DbgOnlyFatalError("Field index out of range");
		return false;
	}

	fieldInfo = gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].fieldInfo;

	return (Boolean) ((fieldInfo & kFieldType_Yomi) == kFieldType_Yomi);
}

/***********************************************************************
 *
 * FUNCTION:    ToolsIsYomiFieldByColId
 *
 * DESCRIPTION: Test if a field is a "Yomi" field, id a field with the
 *				flag kFieldType_Yomi set in the fieldinfo property.
 *
 * PARAMETERS:  columnID:	Test if a column is a "phone" field.
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		02/25/03	Initial Revision
 *
 ***********************************************************************/
Boolean ToolsIsYomiFieldByColId(uint32_t columnID)
{
	AddressTabColumnPropertiesType*	propP;

	if ((propP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID)) != NULL)
		return (Boolean) ((propP->fieldInfo & kFieldType_Yomi) == kFieldType_Yomi);

	return false;
}
/***********************************************************************
 *
 * FUNCTION:    ToolsIsTextFieldByIndex
 *
 * DESCRIPTION: Test if a field is a Text field, id a field that is
 *				edited using a Field.
 *
 * PARAMETERS:  tabIndex:	In which tab is the field to test
 *				fieldIndex:	Index of the field in the tab to test
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		02/25/03	Initial Revision
 *
 ***********************************************************************/
Boolean ToolsIsTextFieldByIndex(uint16_t tabIndex, uint16_t fieldIndex)
{
	if (fieldIndex > gBookInfo.tabs[tabIndex].numElement)
	{
		DbgOnlyFatalError("Field index out of range");
		return false;
	}

	switch (gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].columnDataType)
	{
		case dbBoolean:
		case dbDateTime:
		case dbDate:
		case dbTime:
			return false;
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsIsTextFieldByIndex
 *
 * DESCRIPTION: Test if a field is a Text field, id a field that is
 *				edited using a Field.
 *
 * PARAMETERS:  tabIndex:	In which tab is the field to test
 *				fieldIndex:	Index of the field in the tab to test
 *
 * RETURNED:    true if it's a phone field, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		02/25/03	Initial Revision
 *
 ***********************************************************************/
Boolean ToolsIsTextFieldByColId(uint32_t columnID)
{
	AddressTabColumnPropertiesType*	propP;

	if ((propP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID)) != NULL)
	{
		switch (propP->columnDataType)
		{
			case dbBoolean:
			case dbDateTime:
			case dbDate:
			case dbTime:
				return false;
		}

		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsCustomAcceptBeamDialog
 *
 * DESCRIPTION: This routine uses uses a new exchange manager function to
 *				Ask the user if they want to accept the data as well as set
 *				the category to put the data in. By default all data will go
 *				to the unfiled category, but the user can select another one.
 *				We store the selected category index in the appData field of
 *				the exchange socket so we have it at the when we get the receive
 *				data launch code later.
 *
 * PARAMETERS:  dbP - open database that holds category information
 *				askInfoP - structure passed on exchange ask launchcode
 *
 * RETURNED:    Error if any
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * bhall	09/07/99	Initial Revision
 * gavin	11/09/99	Rewritten to use new ExgDoDialog function
 *
 ***********************************************************************/
status_t ToolsCustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP)
{
	ExgDialogInfoType	exgInfo;
	status_t			err = errNone;
	Boolean				result = false;

	memset(&exgInfo, 0, sizeof(ExgDialogInfoType));

	if (dbP)
	{
		// set default category to unfiled
		exgInfo.categoryIndex = dmUnfiledCategory;
		// Store the database ref into a gadget for use by the event handler
		exgInfo.db = dbP;

		// Let the exchange manager run the dialog for us
		result = ExgDoDialog(askInfoP->socketP, &exgInfo, &err);
	}

	if (err >= errNone && result)
	{
		// pretend as if user hit OK, we'll now accept the data
		askInfoP->result = exgAskOk;

		// Stuff the category index into the appData field
		askInfoP->socketP->appData = exgInfo.categoryIndex;
	}
	else
	{
		// pretend as if user hit cancel, we won't accept the data
		askInfoP->result = exgAskCancel;
	}

	return err;
}



/***********************************************************************
 *
 * FUNCTION:    ToolsGetGraffitiObjectIndex
 *
 * DESCRIPTION: This routine returns the Object Indew of the
 *				GSI (Graffiti shift indicator) in a Form
 *				needed oto move GSI for Active Input Area Support
 *
 * PARAMETERS:	frm FormPtr
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		02/01/02	Initial Revision
 *
 ***********************************************************************/
uint16_t ToolsGetGraffitiObjectIndex (FormType* formP)
{
	uint16_t i;
	uint16_t howMany;

	howMany = FrmGetNumberOfObjects(formP);

	for ( i = 0 ; i < howMany ; i ++)
	{
		if (FrmGetObjectType(formP, i) == frmGraffitiStateObj)
		{
			return (i);
		}
	}

	return frmInvalidObjectId;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetFrmObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object
 *
 * PARAMETERS:  frmP - form
 *				Id - id of the object to display,
 *
 * RETURNED:    pointer to the object
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			aro			6/12/00		Initial Revision
 *
 ***********************************************************************/

void* ToolsGetFrmObjectPtr( FormType* frmP, DmResourceID objectID)
{
	return (FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID)));
}


/***********************************************************************
 *
 * FUNCTION:    ToolsAddrBeamBusinessCard
 *
 * DESCRIPTION: Send the Business Card record or complain if none selected.
 *
 * PARAMETERS:  dbP - the database
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  10/20/97  Initial Revision
 *
 ***********************************************************************/
Boolean ToolsAddrBeamBusinessCard(void)
{
	Boolean		verifyPW;
	uint16_t	attr;

	if (gBusinessCardRowID != dbInvalidRowID)
	{
		if (DbGetRowAttr(gAddrDB, gBusinessCardRowID, &attr) >= errNone)
		{
			verifyPW = (Boolean) ((attr & dbRecAttrSecret) ? AddressDBVerifyPassword() : true);

			if (verifyPW)
				TransferSendRecord(gAddrDB, gBusinessCardRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(kTabAllId));
        }

		return true;
    }

	FrmAlert(gApplicationDbP, SendBusinessCardAlert);

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsAddrAttachRecord
 *
 * DESCRIPTION: Attach the current record
 *
 * PARAMETERS:  dbP - the database
 *				recordToSend - record to attach
 *
 * RETURNED:   Nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  10/20/97  Initial Revision
 *
 ***********************************************************************/
void ToolsAddrAttachRecord(DmOpenRef dbP, uint32_t rowIDToSend)
{
	EventType	newEvent;

	if (rowIDToSend == dbInvalidRowID)
	{
		FrmAlert(gApplicationDbP, AttachNoRecordSelectedAlert);
		return;
	}

	TransferSendRecord(dbP, rowIDToSend, gTransportPrefix, NoDataToSendAlert, AddressExportGetFamily(kTabAllId));
	memset(&newEvent, 0, sizeof(newEvent));
	newEvent.eType = appStopEvent;
	EvtAddEventToQueue(&newEvent);
}

/***********************************************************************
 *
 * FUNCTION:
 *	ToolsIsPhoneIndexSupported
 *
 * DESCRIPTION:
 *	This routine check if a the phone index of the given record has
 *	a type that allows dialing (ie for now all except email)
 *
 * PARAMETERS:
 *	recordP		IN	record pointer
 *	phoneIndex	IN	index of the phone to check, kDialListShowInListPhoneIndex
 *					for show in list
 *
 * RETURNED:
 *  true if the type is supported and if its content is not null
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/27/00		Initial Revision
 *	TEs			11/28/02	Rewrote for new DB Mgr support
 *
 ***********************************************************************/
Boolean	ToolsIsPhoneIndexSupported(uint32_t rowID, uint32_t columnID)
{
	status_t		err = errNone;
	uint32_t		size = 0;
	uint32_t		fieldInfo;
	AddressTabColumnPropertiesType*	propP;

	err = DbCopyColumnValue(gAddrDB, rowID, columnID, 0, NULL, &size);

	if (err < errNone || size == 0)
		return false;

	propP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID);
	if (propP != NULL)
		fieldInfo = propP->fieldInfo;
	else
		return false;

	// Check if it is a phone type field
	return (Boolean) ((fieldInfo & kFieldType_Phone) == kFieldType_Phone);
}

/***********************************************************************
 *
 * FUNCTION:	PrvToolsFrmInvalidate
 *
 * DESCRIPTION:	Invalidate the window of the passed form ID
 *
 * PARAMETERS:	->	formID: the formID
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	TEs			02/17/03	Initial Revision
 *
 ***********************************************************************/
static void PrvToolsFrmInvalidate(uint16_t formID, RectangleType *rectP, winInvalidateFunc callbackP, void *userParamP)
{
	FormType *	formP;
	WinHandle	winH;

	formP = FrmGetFormPtr(formID);
	if (!formP)
		return;

	winH = FrmGetWindowHandle(formP);
	if (!winH)
		return;

	if (rectP && callbackP)
		WinInvalidateRectFunc(winH, rectP, callbackP, userParamP);
    else if (rectP)
		WinInvalidateRect(winH, rectP);
	else
		WinInvalidateWindow(winH);
}

/***********************************************************************
 *
 * FUNCTION:	ToolsFrmInvalidateRectFunc
 *
 * DESCRIPTION:	Invalidate the window of the passed form ID
 *
 * PARAMETERS:	->	formID: the formID
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	TEs			02/17/03	Initial Revision
 *
 ***********************************************************************/
void ToolsFrmInvalidateRectFunc(uint16_t formID, RectangleType *rectP, winInvalidateFunc callbackP, void *userParamP)
{
	PrvToolsFrmInvalidate(formID, rectP, callbackP, userParamP);
}

/***********************************************************************
 *
 * FUNCTION:	ToolsFrmInvalidateWindow
 *
 * DESCRIPTION:	Invalidate the window of the passed form ID
 *
 * PARAMETERS:	->	formID: the formID
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	TEs			02/17/03	Initial Revision
 *
 ***********************************************************************/
void ToolsFrmInvalidateWindow(uint16_t formID)
{
	PrvToolsFrmInvalidate(formID, NULL, NULL, NULL);
}

/***********************************************************************
 *
 * FUNCTION:	ToolsFrmInvalidateRect
 *
 * DESCRIPTION:	Invalidate the rectangle for the window of the
 *				passed form ID
 *
 * PARAMETERS:	->	formID: the formID
 *				->	rectP: the rectangle to invalidate
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	TEs			02/17/03	Initial Revision
 *
 ***********************************************************************/
void ToolsFrmInvalidateRect(uint16_t formID, RectangleType *rectP)
{
	PrvToolsFrmInvalidate(formID, rectP, NULL, NULL);
}

/***********************************************************************
 *
 * FUNCTION:    	ToolsDrawTextLabel
 *
 * DESCRIPTION: 	Draw a text Label using old Win Api.
 *
 * PARAMETERS: 	->	textLabel: 	text.
 *				->	textLen:	tetxLen.
 *				-> 	highlight:	hightligth yes or no.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	02/21/03	Creation.
 * 	PPL	02/21/03	Implementation.
 *
 ***********************************************************************/
void ToolsDrawTextLabel(char* textLabelP, size_t textLen, Coord x, Coord y, Boolean highlight)
{
	UIColorTableEntries	backIndex;
	UIColorTableEntries	textIndex;
	uint16_t			cornerDiam;
	RGBColorType 		backRGB;
	RGBColorType 		textRGB;
	RGBColorType 		savedBackRGB;
	RGBColorType 		savedTextRGB;
	RGBColorType 		savedForeRGB;
	RectangleType		textZone;
	Coord 				width;
	Coord				height;

	if (highlight)
	{
		backIndex = UIObjectSelectedFill;
		textIndex = UIObjectSelectedForeground;
		cornerDiam 	= 2;
	}
	else
	{
		backIndex = UIFieldBackground;
		textIndex = UIObjectForeground;
		cornerDiam 	= 0;
	}

	UIColorGetTableEntryRGB(backIndex, &backRGB);
	UIColorGetTableEntryRGB(textIndex,&textRGB);

	width = FntCharsWidth(textLabelP, textLen);
	height = FntLineHeight();

	RctSetRectangle(&textZone, x, y, width, height);

	WinSetBackColorRGB(&backRGB, &savedBackRGB);
	WinSetTextColorRGB(&textRGB, &savedTextRGB);
	WinSetForeColorRGB(&textRGB, &savedForeRGB);

	WinEraseRectangle(&textZone, cornerDiam);

	WinDrawChars(textLabelP, (int16_t)textLen, x, y);

	WinSetBackColorRGB(&savedBackRGB, &backRGB);
	WinSetTextColorRGB(&savedTextRGB, &textRGB);
	WinSetForeColorRGB(&savedForeRGB, &textRGB);
}

/***********************************************************************
 *
 * FUNCTION:	ToolsDrawFullnamePopup
 *
 * DESCRIPTION:	This routine draws a popup on screen displaying
 *				the record fullname. It's called by the Record and
 *				Edit views when the FFN is truncated and the user tap
 *				on it.
 *
 * PARAMETERS:	->	fullnameH: Handle of the FFN string
 *				->	popupBoundsP: FFN bounds
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/01/03	Initial revision
 *	TEs		10/08/03	Convert as a callback for WinInvalidateRectFunc (Update based form)
 *	TEs		07/27/04	Convert back to a direct drawing function
 *
 ***********************************************************************/
void ToolsDrawFullnamePopup(MemHandle fullnameH, RectangleType *popupBoundsP)
{
	Coord		x;
	Coord		y;
	Coord		maxWidth;
	size_t		length;
	uint16_t	numLines;
	uint16_t	maxNumLines;
	size_t *	offsetsP;
	FontID		currFont;
	Coord		lineHeight;
	char *		fullnameP;
	uint16_t	i;
	RectangleType	popupBounds;

	fullnameP = MemHandleLock((MemHandle)fullnameH);

	popupBounds = *popupBoundsP;

	currFont = FntSetFont(largeBoldFont);
	lineHeight = FntLineHeight();

	maxWidth = popupBounds.extent.x - 10;

	maxNumLines = popupBounds.extent.y - 10;
	maxNumLines /= lineHeight;

	offsetsP = MemPtrNew((uint32_t)(sizeof(size_t) * maxNumLines));
	ErrFatalDisplayIf(!offsetsP, "Out of memory!");

	numLines = 0;
	offsetsP[0] = 0;
	while (numLines < maxNumLines && *(fullnameP + offsetsP[numLines]))
	{
		length = FntWordWrap(fullnameP + offsetsP[numLines], maxWidth);
		numLines++;
		offsetsP[numLines] = offsetsP[numLines - 1] + length;
	}

	popupBounds.extent.y = numLines * lineHeight + 10;
	RctInsetRectangle(&popupBounds, 1);
	x = popupBounds.topLeft.x + 5;
	y = popupBounds.topLeft.y + 5;

    WinEraseRectangle(&popupBounds, 0);
	WinDrawRectangleFrame(roundFrame, &popupBounds);

	for (i = 0; i < numLines; i++)
	{
		if (i < maxNumLines - 1)
			WinDrawChars(fullnameP + offsetsP[i], (int16_t) (offsetsP[i + 1] - offsetsP[i]), x, y);
		else
			WinDrawTruncChars(fullnameP + offsetsP[i], (int16_t)strlen(fullnameP + offsetsP[i]), x, y, maxWidth);

		y += lineHeight;
	}

	FntSetFont(currFont);

	MemHandleUnlock(fullnameH);
	MemPtrFree(offsetsP);
}

/***********************************************************************
 *
 * FUNCTION:	ToolsStripNonPrintableChar
 *
 * DESCRIPTION:	This routine removes all non printable chars in a string.
 *				This occurs in place. No new string.
 *				  If a non printable char is not surrounded by any space,
 *				the non printable char is replaced by a space. Doing so,
 *				carriage returns with no space before or after are replaced
 *				by a space char.
 *
 * PARAMETERS:	->	stringToStrip: Inout string to check and update.
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/01/03	Initial revision
 *	PLe		09/11/03	Take care of spaces before or after non printable char.
 *
 ***********************************************************************/
char* ToolsStripNonPrintableChar(char* stringToStrip)
{
	size_t		offset, offsetDest;
	wchar32_t	outChar;
	Boolean		previousCharIsSpace;

	// Remove all non printable char from the string before to compute anything.
	offset = 0;
	offsetDest = 0;
	outChar = chrNull;

	while(true)
	{
		previousCharIsSpace = (Boolean) (outChar == chrSpace);
		offset += TxtGetNextChar(stringToStrip, offset, &outChar);

		if (outChar == chrNull)
			break;

		if (TxtCharIsPrint(outChar))
		{
			offsetDest += TxtSetNextChar(stringToStrip, offsetDest, outChar);
		}
		else if (! previousCharIsSpace)
		{
			// Check for next char
			TxtGetNextChar(stringToStrip, offset, &outChar);

			if (outChar != chrSpace)
			{
				// The next char is also not a space, so replace the current not
				// printable char by a space.
				offsetDest += TxtSetNextChar(stringToStrip, offsetDest, chrSpace);
			}
		}
	}

	TxtSetNextChar(stringToStrip, offsetDest, outChar);

	return stringToStrip;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsCountryToResourceID
 *
 * DESCRIPTION:	Implemen the ISO 3166 method to convert a 2 letter
 *				country code to an uint16 (range 1101 to 1876)
 *				if the country code is not a double valid char ie between
 *				'AA and 'ZZ' return 0
 *
 * PARAMETERS:	country: 2 letter country code returned by the Local Manager
 *
 * RETURNED:	Converted country code, 0 if not a valid country code.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		09/12/03	Use new Database Manager. - Add parameter
 *
 ***********************************************************************/
DmResourceID ToolsCountryToResourceID(LmCountryType country)
{
	// From ISO 3166
	// There is also a numeric version of the two letter code not given here, calculated as:
	// 1070+30a+b, where a and b are the two letters of the code converted by A=1, B=2, etc. So AA=1101, AB=1102, BA=1131, and ZZ=1876'

	// If the country set in the preference is not a 'valid' 2 letter country code (letter from A..Z only), return.
	if ((country < 'AA') || (country > 'ZZ'))
		return 0;

	return 1070 + 30 * (((country & 0xFF00) >> 8) - '@') + ((country & 0x00FF) - '@');
}

/***********************************************************************
 *
 * FUNCTION:	ToolsGetCountryBaseResID
 *
 * DESCRIPTION:	Return a resource ID base first on the 'Format' country code.
 *				If not found serach based on the 'System' country code.
 *				If not found, try the default resource.
 *
 * PARAMETERS:	resType:	Resource type to search
 *				baseID: 	Base ID for the resource. A value is added to this
 *							base to compute a real resource ID. The base is
 *							different depending of the kind of resource.
 *				defaultIncrement:	If no resource related to a country code have been found,
 *									return the default resource ID = Base + Increment
 *
 * RETURNED:	A resource ID
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		06/09/04	Initial revision
 *
 ***********************************************************************/
DmResourceID ToolsGetCountryBaseResID(DmResourceType resType, DmResourceID baseID, uint16_t defaultIncrement)
{
	DmResourceID	resID;
	MemHandle		resH = NULL;

	// First check if the layout for the 'format country' exists.
	resID = baseID + ToolsCountryToResourceID(gFormatLocale.country);

	// If format country layout not found, select the System Layout
	if ((resH = DmGetResource(gApplicationDbP, resType, resID)) == NULL)
	{
		resID = baseID + ToolsCountryToResourceID(gSystemLocale.country);

		if ((resH = DmGetResource(gApplicationDbP, resType, resID)) == NULL)
			resID = baseID + defaultIncrement;
	}

	if (resH)
		DmReleaseResource(resH);

	return resID;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsSetBusinessCardIndicatorPosition
 *
 * DESCRIPTION: Set the Business vCard Indicator Bitmap 3 pixels after the
 *				Form Title. It's done dynamically because the Form Title can
 *				now display or not the Application Small Icon.
 *
 * PARAMETERS:  formP - the form containing the business card indicator
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		06/07/04	Initial Revision
 *
 ***********************************************************************/
void ToolsSetBusinessCardIndicatorPosition(FormType *formP)
{
	RectangleType	iconBounds, titleBounds;
	uint16_t		objectCount = FrmGetNumberOfObjects(formP);
	uint16_t		objIndex = FrmGetObjectIndex(formP, AddressBusinessCardBmp);
	uint16_t		i;

	// Compute maximum label width
	FrmGetObjectBounds(formP, objIndex, &iconBounds);

 	for (i = 0; i < objectCount; i++)
 	{
		if (FrmGetObjectType(formP, i) == frmTitleObj)
		{
			FrmGetObjectBounds(formP, i, &titleBounds);
			iconBounds.topLeft.x = titleBounds.topLeft.x + titleBounds.extent.x + 3;
			FrmSetObjectBounds(formP, objIndex, &iconBounds);
			break;
		}
 	}
}

/***********************************************************************
 *
 * FUNCTION:    ToolsBackgroundGadgetHandler
 *
 * DESCRIPTION: Gadget handler callback for the background gadget.  We
 *              draw the gadget here.
 *
 * PARAMETERS:  gadgetP - the gadget being drawn
 *              cmd - command for this call
 *              paramP - unused
 *
 * RETURNED:    nothing
 *
 * Name		Date		Description
 * -----	--------	-----------
 *
 *
 ***********************************************************************/
Boolean ToolsBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP,
									 uint16_t cmd, void *paramP)
{
	GcGradientType	grad;
	GcPointType		pnt[2];
	GcColorType		col[2];
	GcHandle		gc;
	RectanglePtr	whiteRect;
	RectanglePtr	bounds;

	if(cmd != formGadgetDrawCmd)
		return false;

	// Draw the gradient background
	bounds = &gadgetP->rect;

	whiteRect = (RectanglePtr)(gadgetP->data);

	col[0].red = col[1].red = kBackgroundColorRed;
	col[0].green = col[1].green = kBackgroundColorGreen;
	col[0].blue = col[1].blue = kBackgroundColorBlue;
	col[0].alpha = 64;
	col[1].alpha = 255;

	pnt[0].x = (fcoord_t)(bounds->topLeft.x);
	pnt[0].y = (fcoord_t)(bounds->topLeft.y);
	pnt[1].x = (fcoord_t)(bounds->topLeft.x);
	pnt[1].y = (fcoord_t)(bounds->topLeft.y + bounds->extent.y);

	gc = GcGetCurrentContext();

	GcPushState(gc);

	GcInitGradient(&grad, pnt, col, 2);
	GcSetGradient(gc, &grad);

	GcSetDithering(gc, true);

	if(whiteRect)
		GcSetWindingRule(gc, kOddWinding);

	GcRect(gc, pnt[0].x, pnt[0].y,
		   pnt[1].x + (fcoord_t)bounds->extent.x, pnt[1].y);

	// Exclude whiteRect from gradinet just to save time with dithering
	if(whiteRect)
		GcRectI(gc, whiteRect->topLeft.x, whiteRect->topLeft.y,
				(int32_t)(whiteRect->topLeft.x + whiteRect->extent.x),
				(int32_t)(whiteRect->topLeft.y + whiteRect->extent.y));

	GcPaint(gc);

	GcPopState(gc);

	// Now draw the white rectangle
	if(whiteRect)
	{
		GcSetColor(gc, 255, 255, 255, 255);
		GcRectI(gc, whiteRect->topLeft.x, whiteRect->topLeft.y,
				(int32_t)(whiteRect->topLeft.x + whiteRect->extent.x),
				(int32_t)(whiteRect->topLeft.y + whiteRect->extent.y));
		GcPaint(gc);
	}

	GcReleaseContext(gc);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    ToolsConfirmDeletion
 *
 * DESCRIPTION: Display the dialog askinf the user to confirm deletion
 *				of the current record
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if the user taps Yes, false otherwies
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		07/07/04	Extracted from DetailsDeleteRecord
 *
 ***********************************************************************/
Boolean ToolsConfirmDeletion(void)
{
	uint16_t	ctlIndex;
	uint16_t	buttonHit;
	FormPtr		alert;
	Boolean		archive;

	// Display an alert to comfirm the operation.
	alert = FrmInitForm(gApplicationDbP, DeleteAddrDialog);

	// Set the "save backup" checkbox to its previous setting.
	ctlIndex = FrmGetObjectIndex(alert, DeleteAddrSaveBackup);

	FrmSetControlValue(alert, ctlIndex, gSaveBackup);

	buttonHit = FrmDoDialog(alert);

	archive = (Boolean)FrmGetControlValue(alert, ctlIndex);

	FrmDeleteForm(alert);

	if (buttonHit == DeleteAddrCancel)
		return false;

	// Remember the "save backup" checkbox setting.
	gSaveBackup = archive;

	return true;
}

/***********************************************************************
 *
 * FUNCTION:	ToolsCheckCategories
 *
 * DESCRIPTION:	Restores the saved categories. It also checks if the saved
 *				categories are still valid (i.e they haven't been deleted
 *				from the detail dialog).
 *
 * PARAMETERS:	None
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		07/07/2004	Extracted from DetailsHandleEvent
 *
 ***********************************************************************/
void ToolsCheckCategories(CategoryID *categoriesP, uint32_t *numCategoriesP)
{
	char			name[catCategoryNameLength+1];
	uint16_t		i;
	CategoryID *	validCategoriesP;
	uint32_t		numCategories = 0;

	// Check validity of original categories before restoring them
	validCategoriesP = categoriesP;
	for (i = 0; i < *numCategoriesP; i++)
	{
		// No more exists. Skip it.
		if (categoriesP[i] != catIDAll && CatMgrGetName(gAddrDB, categoriesP[i], name) < errNone)
			continue;

		numCategories++;
		*validCategoriesP = categoriesP[i];
		validCategoriesP++;
	}

	// numCategories reflects the number of remaining categories in the original set.
	*numCategoriesP = numCategories;
}
