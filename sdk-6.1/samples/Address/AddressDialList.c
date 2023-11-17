/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDialList.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the Address Book application's dial list form module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <Chars.h>
#include <ErrorMgr.h>
#include <NotifyMgr.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <DataMgr.h>
#include <LocaleMgr.h>
#include <Form.h>
#include <Helper.h>
#include <HelperServiceClass.h>
#include <TextMgr.h>
#include <List.h>
#include <SystemResources.h>
#include <TelephonyLib.h>
#include <string.h>
#include <PenInputMgr.h>
#include <PalmTypesCompatibility.h> // For min & max

#include "AddressDialList.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressRsc.h"
#include "AddressDBSchema.h"
//#include "AddressDefines.h"
#include "AddressTab.h"
#include "AddressLookup.h"

/***********************************************************************
 *
 *   Local macros
 *
 ***********************************************************************/

#define	kPhoneChars	"0123456789,+#*"

/***********************************************************************
 *
 *   Static variables
 *
 ***********************************************************************/

static DialListDataType		sDialListData;

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListPhoneNumberFilter
 *
 * DESCRIPTION:
 *	This routine filter a phone number
 *
 * PARAMETERS:
 *	outString	OUT	filtered phone number
 *	outLen		IN	max text len for outString
 *			 	OUT	phone number len
 *	inString	IN	text to filter
 *	inLen		IN	text len
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			06/12/00	Initial Revision
 *	fpa			11/11/00	Fixed a coherency problem with SMS: now, +++123+++456 -> + 123 456
 *
 ***********************************************************************/
void PrvDialListPhoneNumberFilter( char* outString, uint32_t* outLen, const char* inString, uint32_t inLen )
{
	uint16_t			inOffset;
	uint16_t			outOffset;
	char				curChar;
	LmLocaleType		systemLocale;
	CharEncodingType	systemCharEncoding;
	size_t				outStrLen = *outLen;
	size_t				inStrLen = inLen;

	*outLen = 0;

	// Get the system char encoding to know which one is used to encode the string in the field.
	systemCharEncoding = LmGetSystemLocale(&systemLocale);

	// Convert the string to ASCII. Telephony only accept "ASCII" number (single byte)
	if (TxtConvertEncoding(true, NULL, inString, &inStrLen, systemCharEncoding, outString, &outStrLen, charEncodingAscii + charEncodingDstBestFitFlag, " ", 1) < errNone)
		return;

	outString[outStrLen] = chrNull;
	inOffset = outOffset = 0;

	// If a convertion has occured (jaJP -> ASCII) the result string is smaller than the original string.
	while(outOffset < outStrLen)
	{
		curChar = outString[outOffset++];

		if (strchr(kPhoneChars, curChar))
		{
			// Only + at the beginning
			if (inOffset > 0 && curChar == chrPlusSign)
			{
				if (outString[inOffset - 1] != chrSpace) // Don't add multiple spaces
					outString[inOffset++] = chrSpace;
			}
			else
			{
				outString[inOffset++] = curChar;
			}
		}
		else if (inOffset > 0 && outString[inOffset - 1] != chrSpace)	// No space at the beginning
		{
			outString[inOffset++] = chrSpace;
		}
	}

	// Strip trailling space
	if (inOffset && outString[inOffset - 1] == spaceChr)
		inOffset--;

	outString[inOffset] = chrNull;
	*outLen = inOffset;
}

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListPhoneNumberCanBeDialed
 *
 * DESCRIPTION:
 *	This routine check if a text could be a phone number
 *	ie if it contains phone chars
 *
 * PARAMETERS:
 *	textP	IN	text string to parse
 *	textLen	IN	text len
 *
 * RETURNED:
 *	true if acceptable
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *	kwk			07/26/00	Modified to use Text Mgr, avoid sign extension
 *							problem calling StrChr with signed char value.
 *
 ***********************************************************************/
static Boolean PrvDialListPhoneNumberCanBeDialed(char *textP, uint32_t textLen)
{
    char*		phoneNumberP;
	uint32_t	outLen = textLen;

	// Check input string
	if (!textLen || !textP)
		return false;

	// Create a new empty string with the same size. The PrvDialListPhoneNumberFilter will return an equal or smaller string.
	phoneNumberP = MemPtrNew(textLen + 1);

	if (!phoneNumberP)
		return false;

	// create a new filtered string with space as subsitution char
	PrvDialListPhoneNumberFilter(phoneNumberP, &outLen, textP, textLen);

	MemPtrFree(phoneNumberP);

	return (Boolean)(outLen > 0);
}

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListBuildDescription
 *
 * DESCRIPTION:
 *	This routine build the description
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
static void PrvDialListBuildDescription(FormType *formP)
{
	uint16_t 		gadgetIndex;
	RectangleType 	gadgetBounds;

	gadgetIndex = FrmGetObjectIndex(formP, DialListDescriptionGadget);
	FrmGetObjectBounds(formP, gadgetIndex, &gadgetBounds);

	sDialListData.displayName = ToolsGetRecordNameStr(gAddrDB, sDialListData.rowID, largeBoldFont, gadgetBounds.extent.x);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListHandleDescriptionEvent
 *
 * DESCRIPTION:
 *	This routine handle gadget event for descrption (mainly drawing)
 *
 * PARAMETERS
 *	gadgetP	IN  gadget pointer
 *	cmd		IN	command
 *	paramP	IN	param (unused)
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name	Date			Description
 *	----	----		-----------
 *	aro		08/02/00	Initial Revision
 *	ppl		02/06/02	remove silly goto
 *
 ***********************************************************************/
static Boolean PrvDialListHandleDescriptionEvent( struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP )
{
	Boolean 		handled = false;
	FormType *		gadgetFormP;
	uint16_t 		gadgetIndex;
	RectangleType 	gadgetBounds;
	FontID			fontId;

	if (cmd == formGadgetDrawCmd)
	{
		// Get the bounds of the gadget from the form
		gadgetFormP = FrmGetFormPtr(DialListDialog);
		gadgetIndex = FrmGetObjectIndexFromPtr(gadgetFormP, gadgetP);

		if ( gadgetIndex != frmInvalidObjectId )
		{
			FrmGetObjectBounds(gadgetFormP, gadgetIndex, &gadgetBounds);
			// The sDialListData.displayName is left-aligned and truncated to fit in the gadget
			fontId = FntSetFont(largeBoldFont);
			WinDrawTruncChars(sDialListData.displayName, (int16_t)strlen(sDialListData.displayName), gadgetBounds.topLeft.x, gadgetBounds.topLeft.y, gadgetBounds.extent.x);
			FntSetFont(fontId);
			handled = true;
		}
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListScroll
 *
 * DESCRIPTION:
 *	This routine scroll the list up or down (page per page)
 *
 * PARAMETERS
 *	direction	IN	direction to scroll (winUp or winDown)
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	aro		06/19/00	Initial Revision
 *
 ***********************************************************************/
static void PrvDialListScroll( WinDirectionType direction )
{
	ListType*	lstP = ToolsGetFrmObjectPtr(FrmGetFormPtr(DialListDialog), DialListList);
	int16_t		count = LstGetVisibleItems(lstP);

	LstScrollList(lstP, direction, count - 1);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListDrawPhoneItem
 *
 * DESCRIPTION:
 *	This routine draws a phone item line (label & number)
 *	It is called as a callback routine by the list object.
 *
 * PARAMETERS:
 *	itenNum		IN	index of the item to draw
 *	boundsP		IN	boundsP of rectangle to draw in
 *	itemsText	IN	data if any
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
static void PrvDialListDrawPhoneItem( int16_t index, RectangleType *boundsP, char **itemsText, ListType *listP)
{
	FontID		currentFont;
	uint32_t	posX;

	currentFont = FntSetFont(LstGetFont(listP));

	// Label has been already computed and truncated. Just draw it.
	WinDrawChars(sDialListData.phones[index].label, (int16_t)strlen(sDialListData.phones[index].label), boundsP->topLeft.x, boundsP->topLeft.y);

	// At least, draw the phone number at this position.
	posX = boundsP->topLeft.x + sDialListData.phones[index].labelWidth + kSpaceBetweenLabelAndNumber;

	//Right align
	if ((uint32_t)(boundsP->topLeft.x + boundsP->extent.x) - sDialListData.phones[index].phoneWidth > posX)
		posX = (uint32_t)(boundsP->topLeft.x + boundsP->extent.x) - sDialListData.phones[index].phoneWidth;

	WinDrawTruncChars(sDialListData.phones[index].number, (int16_t)sDialListData.phones[index].numberLen, (Coord)posX, (Coord)boundsP->topLeft.y,
		(Coord) (boundsP->extent.x - (posX - boundsP->topLeft.x)));


	FntSetFont(currentFont);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListUpdatePhoneNumber
 *
 * DESCRIPTION:
 *	This routine update the number
 *	in the field according to current selection
 *	No drawn is made
 *
 * PARAMETERS:
 *	fldP	IN	field
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
static void PrvDialListUpdatePhoneNumber(FormType* formP, FieldType* fldP )
{
	MemHandle	numberH;
	char*		numberP;
	uint32_t	len, maxLen;

	// First allocate a string long enougth to contains the original string.
	len = sDialListData.phones[sDialListData.selectedIndex].numberLen;
	numberH = MemHandleNew(len + 1);

	if (!numberH)
		return;

	// Filter the original string.
	numberP = MemHandleLock(numberH);
	PrvDialListPhoneNumberFilter(numberP,
								&len,
								sDialListData.phones[sDialListData.selectedIndex].number,
								sDialListData.phones[sDialListData.selectedIndex].numberLen);


	// Resize the string if necessary. Max allowed limited by the field.
	maxLen = FldGetMaxChars(fldP);
	maxLen = min(len, maxLen);
	numberP[maxLen] = chrNull;
	MemHandleUnlock(numberH);
	MemHandleResize(numberH, maxLen + 1);

	FldFreeMemory((FieldType*)ToolsGetFrmObjectPtr(formP, DialListNumberField));
	FldSetTextHandle(fldP, numberH);
}

/***********************************************************************
 *
 * FUNCTION:	PrvDialListDrawFieldCallback
 *
 * DESCRIPTION:	Used by WinInvalidateRectFunc to draw the phone number field
 *
 *
 * PARAMETERS:	->	cmd: selector
 *				->	winH: Handle of the window to draw into
 *				->	fieldBoundsP: field bounds
 *				->	fieldP: Pointer to the field
 *
 * RETURNED:	Always true
 *
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		10/08/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvDialListDrawFieldCallback(int32_t cmd, WinHandle winH, const RectangleType *fieldBoundsP, void *fieldP)
{
	if (cmd == winInvalidateDestroy)
		return true;

    FldDrawField((FieldType*)fieldP);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListUpdateAfterSelection
 *
 * DESCRIPTION:
 *	This routine update the number
 *	according to the new or current selection
 *	Set focus to the field
 *
 * PARAMETERS:
 *	frmP	IN	form
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
static void PrvDialListUpdateAfterSelection(void)
{
	FormType*	frmP;
	FieldType* 	fldP;
	uint16_t	fieldIndex;

	frmP = FrmGetFormPtr(DialListDialog);

	// Set the number in the field
	// Number is parse according to characters allowed
	fieldIndex = FrmGetObjectIndex(frmP, DialListNumberField);
	fldP = FrmGetObjectPtr(frmP, fieldIndex);
	PrvDialListUpdatePhoneNumber(frmP, fldP);
	ToolsFrmInvalidateRectFunc(DialListDialog, NULL, PrvDialListDrawFieldCallback, NULL);
	FrmSetFocus(frmP, fieldIndex);
}

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListFreeMemory
 *
 * DESCRIPTION:
 *	This routine frees memory allocated by the dialog
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListFreeMemory( void )
{
	uint16_t	i;

	if (sDialListData.displayName)
		MemPtrFree(sDialListData.displayName);

	for (i = 0; i < sDialListData.phonesCount; i++)
	{
		if (sDialListData.phones[sDialListData.phonesCount].number)
			MemPtrFree(sDialListData.phones[sDialListData.phonesCount].number);
	}
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListLeaveDialog
 *
 * DESCRIPTION:
 *	This routine leave the dialog
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
static void PrvDialListLeaveDialog(Boolean exit)
{
	if (exit)
	{
		EventType newEvent;
		memset(&newEvent, 0, sizeof(newEvent));
		newEvent.eType = appStopEvent;
		EvtAddEventToQueue(&newEvent);
	}
	else
	{
		PrvDialListFreeMemory();
		FrmReturnToForm(0);
	}
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListDialSelected
 *
 * DESCRIPTION:
 *	This routine dial selected number
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
static Boolean PrvDialListDialSelected(void)
{
	FormType*				frmP;
	SysNotifyParamType		param;
	HelperNotifyEventType	details;
	HelperNotifyExecuteType execute;

	frmP = FrmGetFormPtr(DialListDialog);

	param.notifyType = sysNotifyHelperEvent;
	param.broadcaster = sysFileCAddress;
	param.notifyDetailsP = &details;
	param.handled = false;

	details.version = kHelperNotifyCurrentVersion;
	details.actionCode = kHelperNotifyActionCodeExecute;
	details.data.executeP = &execute;

	execute.serviceClassID = kHelperServiceClassIDVoiceDial;
	execute.helperAppID = 0;
	execute.dataP = FldGetTextPtr(ToolsGetFrmObjectPtr(frmP, DialListNumberField));

	execute.displayedName = sDialListData.displayName;
	execute.detailsP = 0;
	execute.err = errNone;

	SysNotifyBroadcast(&param);

	// Check error code
	if (param.handled)
		return (Boolean) (execute.err >= errNone);

	// Not handled so exit the list - Unexepected error
	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvDialListInit
 *
 * DESCRIPTION: This routine initializes the "Dial List" dialog of the
 *              Address application.
 *
 * PARAMETERS:  frmP  - a pointer to the Form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date			Description
 *			----		----		-----------
 *			aro			6/12/00		Initial Revision
 *			ppl			02/06/02	Add active input area support
 *
 ***********************************************************************/
static void PrvDialListInit(void)
{
	FormType*	frmP;
	ListType*	lstP;
	FieldType*	fldP;
	uint16_t	fldIndex;
	int16_t		middle;
	int16_t		visibleItems;
	int16_t		topIndex;
	FontID		savedFont, listFont;
	RectangleType	listRect;
	uint32_t	maxPhoneWidth, maxLabelWidth, phoneWidth, suffixWidth;
	uint16_t	i;
	MemHandle		colonResourceH;
	char*			colonStrP;

	// Load the colon suffix string from resource
	if ((colonResourceH = DmGetResource(gApplicationDbP, strRsc, LabelSuffixColonStr)) != NULL)
		colonStrP = DmHandleLock(colonResourceH);
	else colonStrP = ":";

	frmP = FrmGetFormPtr(DialListDialog);

	// Build the description for drawing in the form. It must be done here
	// (instead of DialListShowDialog) because we need the form gadget bounds
	PrvDialListBuildDescription(frmP);

	FrmSetGadgetHandler(frmP, FrmGetObjectIndex(frmP, DialListDescriptionGadget), PrvDialListHandleDescriptionEvent);

	// Initialize the address list.
	lstP = ToolsGetFrmObjectPtr(frmP, DialListList);
	LstSetListChoices(lstP, 0, sDialListData.phonesCount);
	LstSetDrawFunction(lstP, PrvDialListDrawPhoneItem);

	// At this point we have a full length "label". Now we need to compute it to its real display size.
	// First: Get the list info: Size and font.
	listFont = LstGetFont (lstP);
	savedFont = FntSetFont(listFont);
	FrmGetObjectBounds (frmP, FrmGetObjectIndex(frmP, DialListList), &listRect);
	// Max phone with = half list width
	maxPhoneWidth = (uint32_t)((listRect.extent.x - kSpaceBetweenLabelAndNumber) >> 1);

	// Remove the length used by the suffix string.
	suffixWidth = FntCharsWidth(colonStrP, strlen(colonStrP));

	for (i = 0; i < sDialListData.phonesCount; i++)
	{
		phoneWidth = FntCharsWidth(sDialListData.phones[i].number, sDialListData.phones[i].numberLen);
		phoneWidth = min(phoneWidth, maxPhoneWidth);
		sDialListData.phones[i].phoneWidth = phoneWidth;
		// Label width is the remaining space of the list, less the suffix with (always drawned)
		maxLabelWidth = maxPhoneWidth + maxPhoneWidth - phoneWidth - suffixWidth;
		FntTruncateString(sDialListData.phones[i].label, sDialListData.phones[i].label, listFont, (Coord)maxLabelWidth, true);
		sDialListData.phones[i].labelWidth = FntCharsWidth(sDialListData.phones[i].label, strlen(sDialListData.phones[i].label));
		StrLCat(sDialListData.phones[i].label, colonStrP, kMaxCharsLabelLen + 1);
	}

	FntSetFont(savedFont);

	if (colonResourceH)
	{
		DmHandleUnlock(colonResourceH);
		DmReleaseResource(colonResourceH);
	}

	// Set the top item to avoid flickering
	// Try to have the selected one in the middle
	visibleItems = LstGetVisibleItems(lstP);
	middle = (visibleItems - 1) >> 1;

	if ((sDialListData.phonesCount <= visibleItems) || (sDialListData.selectedIndex <= middle))
	{
		// top aligned
		topIndex = 0;
	}
	else if (sDialListData.selectedIndex >= (sDialListData.phonesCount - (visibleItems - middle)))
	{
		// bottom aligned
		topIndex = sDialListData.phonesCount - visibleItems;
	}
	else
	{
		// centered
		topIndex = sDialListData.selectedIndex - middle;
	}

	LstSetTopItem(lstP, topIndex);
	LstSetSelection(lstP, sDialListData.selectedIndex);

	// initiate phone number field
	fldIndex = FrmGetObjectIndex(frmP, DialListNumberField);
	fldP = FrmGetObjectPtr(frmP, fldIndex);
	FldSetMaxChars(fldP, kTelPhoneNumberMaxLen);
	PrvDialListUpdatePhoneNumber(frmP, fldP);
	FrmSetFocus(frmP, fldIndex);

	ToolsFrmInvalidateWindow(DialListDialog);
}

/***********************************************************************
 *
 * FUNCTION:    DialListHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Dial List"
 *              of the Address Book application.
 *
 * PARAMETERS:  evtP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name		Date			Description
 *			----		----		-----------
 *			aro			6/12/00		Initial Revision
 *			ppl			02/06/02	active input area support
 *
 ***********************************************************************/
Boolean DialListHandleEvent( EventType * evtP )
{
	Boolean	handled = false;

	switch (evtP->eType)
	{
		case frmOpenEvent:
			PrvDialListInit();
			handled = true;
			break;

		case frmCloseEvent:
			PrvDialListFreeMemory();
			break;

		case lstSelectEvent:
			// Update phone type & number
			// Set focus to the field
			sDialListData.selectedIndex = evtP->data.lstSelect.selection;
			PrvDialListUpdateAfterSelection();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (evtP->data.ctlSelect.controlID)
			{
				case DialListDialButton:
					if (PrvDialListDialSelected())
						PrvDialListLeaveDialog(gCalledFromApp);

					handled = true;
					break;

				case DialListCancelButton:
					PrvDialListLeaveDialog(false);
					handled = true;
					break;
			}
			break;

		case keyDownEvent:
			if (TxtCharIsHardKey(evtP->data.keyDown.modifiers, evtP->data.keyDown.chr))
			{
				PrvDialListLeaveDialog(false);
				handled = true;
			}
			else if (EvtKeydownIsVirtual(evtP))
			{
				// up and down scroll bar without updating the selection
				if (evtP->data.keyDown.chr == vchrPageUp)
					PrvDialListScroll(winUp);
				else if (evtP->data.keyDown.chr == vchrPageDown)
					PrvDialListScroll(winDown);
			}
			break;
	}

	return (handled);
}

/***********************************************************************
 *
 * FUNCTION:
 *	DialListShowDialog
 *
 * DESCRIPTION:
 *	This routine show the dialog of the given record
 *	if the phoneIndex is not a phone number, first phone number is selected
 *
 * PARAMETERS:
 *	rowID			IN		uinque ID of the record
 *	colID			IN		index of the column
 *
 * RETURNED:
 *	false if the form must not be displayed
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *	TEs			8/09/02		DB schema support
 *
 ***********************************************************************/
Boolean DialListShowDialog(uint32_t rowID, uint32_t colID)
{
	uint16_t	propertyIndex;
	uint16_t	tabIndex;
	char *		phoneP;
	uint32_t	phoneLen;
	char*		colNameP;
	uint32_t	columnID;
	char*		stringP;

	// Check id the selected number is a valid phone number
	if (colID && (DbGetColumnValue(gAddrDB, rowID, colID, 0, (void**)&phoneP, &phoneLen) >= errNone))
	{
		// If the column doesn't contain valid data that can be used as phone number, reset the colID.
		if (!PrvDialListPhoneNumberCanBeDialed(phoneP, phoneLen - 1))
			colID = 0;

		DbReleaseStorage(gAddrDB, phoneP);
	}

    // Build the phone array
	memset(&sDialListData, 0, sizeof(DialListDataType));

	sDialListData.rowID = rowID;

    // Build the phone array
	// Splitting each phone field per line
	sDialListData.phonesCount = 0;
	sDialListData.selectedIndex = 0;

	for (tabIndex = 0; tabIndex < gBookInfo.numTabs && sDialListData.phonesCount < kMaxPhonesCount; tabIndex++)
	{
		// Don't display phone number with the invisible/skip flag set
		if (gBookInfo.tabs[tabIndex].tabInfo.tabId == kBookInvalidTabId)
			continue;

		for (propertyIndex = 0; propertyIndex < gBookInfo.tabs[tabIndex].numElement && sDialListData.phonesCount < kMaxPhonesCount; propertyIndex++)
		{
			// It's not a phone field. Continue
			if (!ToolsIsPhoneFieldByIndex(tabIndex, propertyIndex, true))
				continue;

			phoneP = NULL;
			phoneLen = 0;
			columnID = gBookInfo.tabs[tabIndex].columnsPropertiesP[propertyIndex].columnId;

			if (DbGetColumnValue(gAddrDB, rowID, columnID, 0, (void**)&phoneP, &phoneLen) >= errNone)
			{
				if (PrvDialListPhoneNumberCanBeDialed(phoneP, phoneLen - 1)) // Remove trailling \0
				{
					colNameP = NULL;
					stringP = ToolsStrDup(phoneP);
					stringP = ToolsStripNonPrintableChar(stringP);
					sDialListData.phones[sDialListData.phonesCount].number = stringP;
					sDialListData.phones[sDialListData.phonesCount].numberLen = strlen(stringP);
					sDialListData.phones[sDialListData.phonesCount].label[0] = chrNull;

					StrLCopy(sDialListData.phones[sDialListData.phonesCount].label, gBookInfo.tabs[tabIndex].tabInfo.tabNameP, kMaxCharsLabelLen + 1);

					DbGetColumnPropertyValue(gAddrDB, kAddressDefaultTableName,	columnID, kAddrColumnPropertyName, &phoneLen, (void**) &colNameP);

					if (colNameP != NULL)
					{
						StrLCat(sDialListData.phones[sDialListData.phonesCount].label, " ", kMaxCharsLabelLen + 1);
						StrLCat(sDialListData.phones[sDialListData.phonesCount].label, colNameP, kMaxCharsLabelLen + 1);
                        DbReleaseStorage(gAddrDB, colNameP);
					}

					if (colID && (columnID == colID))
						sDialListData.selectedIndex = sDialListData.phonesCount;

					sDialListData.phonesCount++;
				}

				if (phoneP != NULL)
					DbReleaseStorage(gAddrDB, phoneP);
			}
		}
	}

	// Exit if no phone are available for this record
	if (!(sDialListData.phonesCount))
		goto exit;

	// Ok so no show the dialog...
	FrmPopupForm(gApplicationDbP, DialListDialog);

	return true;

exit:
	PrvDialListFreeMemory();
	return false;
}
