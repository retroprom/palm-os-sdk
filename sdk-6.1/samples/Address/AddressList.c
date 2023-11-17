/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressList.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *   This is the Address Book application's list form module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <SystemMgr.h>
#include <TextMgr.h>
#include <ErrorMgr.h>
#include <SoundMgr.h>
#include <TimeMgr.h>
#include <AboutBox.h>
#include <CatMgr.h>
#include <Menu.h>
#include <UIResources.h>
#include <FeatureMgr.h>
#include <PenInputMgr.h>
#include <ExgLocalLib.h>
#include <Table.h>
#include <StringMgr.h>
#include <TraceMgr.h>
#include <Loader.h>
#include "SearchLib.h"
#include <CatMgr.h>
#include <string.h>
#include <TextServicesMgr.h>
#include <PhoneBookIOLib.h>

#include "AddressList.h"
#include "AddressEdit.h"
#include "AddressDialList.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressNote.h"
#include "AddressRsc.h"
#include "AddressDBSchema.h"

#include "AddressTransfer.h"
#include "AddressExportAsk.h"
#include "AddressU32List.h"

#define kLookupFieldMaxChars		8
#define kLookupTextBufferSize		((uint16_t)(kLookupFieldMaxChars * 2 + 1))
#define kInvalidCategoryID			catCategoryIDPositiveUpperBound

/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

// These are used for accelerated scrolling
static uint32_t			sLastSeconds = 0;
static uint32_t			sScrollUnits = 0;

// These are used for phone number selection in list
static Boolean			sPhoneNumberSelection;
static Boolean			sPhoneNumberSelected;
static int16_t			sPhoneNumberSelectedTableRow = 0x7FFF;

static RectangleType	sSelectedPhoneBounds;
static uint32_t			sSelectedRecordPhoneColumnID;

static char				sLookupText[kLookupTextBufferSize];

static CategoryID		sSavedCategoryID = kInvalidCategoryID;

static const RGBColorType sListDarkenedListEntry = { 0, 255, 251, 230 };

static Boolean			sOnehandedTableFocused = false;
static Boolean			sOnehandedRecordSelected = false;
static Boolean			sOneHandedPhoneSelected = false;

/***********************************************************************
 *
 * FUNCTION:    PrvListClearLookupString
 *
 * DESCRIPTION: Clears the ListLookupField.  Does not unhighlight the item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * -----	--------	-----------
 * roger	06/16/95	Initial Revision
 *
 ***********************************************************************/
static void PrvListClearLookupString(void)
{
	FormType*		formP;
	FieldType*		fieldP;

	formP = FrmGetFormPtr(ListView);
	fieldP = ToolsGetFrmObjectPtr(formP, ListLookupField);

	FldFreeMemory(fieldP);
	FldSetInsertionPoint(fieldP, 0);
	FldRecalculateField(fieldP, true);

	FrmSetFocus(formP, noFocus);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListChangeCategories
 *
 * DESCRIPTION: Changes the current categories
 *
 * PARAMETERS:  ->	categoriesP: array of categoryIDs
 *				->	numCategories: the number of categoryIDs in categoriesP
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	------	--------	-----------
 *	TEs		02/20/03	Initial Revision
 *
 ***********************************************************************/
static void PrvListChangeCategories(CategoryID *categoriesP, uint32_t numCategories, Boolean updateTrigger)
{
	PrvListClearLookupString();
	ToolsChangeCategory(categoriesP, numCategories);

	// Set the trigger label if needed
	if (updateTrigger)
	{
		ControlType *	ctlP;

		ctlP = (ControlType*) ToolsGetFrmObjectPtr(FrmGetFormPtr(ListView), ListCategoryTrigger);
		CatMgrSetTriggerLabel(gAddrDB, categoriesP, numCategories, ctlP, gCategoryName);
	}

	// By changing the category the current record is lost.
	gCurrentRowID = dbInvalidRowID;

	// Update display
	ListViewUpdateDisplay(true);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListResetScrollRate
 *
 * DESCRIPTION: This routine resets the scroll rate
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		 Date		Description
 * ------	-------		-----------
 * frigino	8/14/97		Initial Revision
 *
 ***********************************************************************/
static void PrvListResetScrollRate(void)
{
	// Reset last seconds
	sLastSeconds = TimGetSeconds();
	// Reset scroll units
	sScrollUnits = 1;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListAdjustScrollRate
 *
 * DESCRIPTION: This routine adjusts the scroll rate based on the current
 *              scroll rate, given a certain delay, and plays a sound
 *              to notify the user of the change
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *	Name		  Date		Description
 *	-------		--------	-----------
 *	frigino		08/14/97	Initial Revision
 *	vmk			12/02/97	Fix crash from uninitialized sndCmd and
 *							derive sound amplitude from system amplitude
 *
 ***********************************************************************/
static void PrvListAdjustScrollRate(void)
{
	// Accelerate the scroll rate every 3 seconds if not already at max scroll speed
	uint32_t newSeconds = TimGetSeconds();

	if ((sScrollUnits < kScrollSpeedLimit) && ((newSeconds - sLastSeconds) > kScrollDelay))
	{
		// Save new seconds
		sLastSeconds = newSeconds;

		// increase scroll units
		sScrollUnits += kScrollAcceleration;
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvListDrawRecord
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
 *
 * Name		  Date      Description
 * ----		--------	-----------
 * roger	06/21/95	Initial Revision
 * peter	05/09/00	Store phoneX in row data for tap test
 * LFe		10/16/02	Use new Database Manager.
 *						Store phoneX in row ID instead of row data
 *						as record index are now stored in row data.
 *
 ***********************************************************************/
static void PrvListDrawRecord(void * table, int16_t row, int16_t column, RectanglePtr bounds)
{
	uint32_t	rowIndex;
	char		noteChar;
	FontID		currFont;
	uint16_t	phoneX;
	Boolean		customDraw;
	int16_t		selectedRow = -1;
	int16_t		selectedCol;
	RGBColorType oldBackColor;
	Boolean		changedBackColor = false;
	uint32_t	savedRowIndex;

	// In some case, like for Security, we close the database and the current cursor
	// but the security dialog post an update event. We can't do anything while the database is closed.
	if (gCursorID == dbInvalidCursorID)
		return;

	// Save the current cursor position
	if (DbCursorGetCurrentPosition(gCursorID, &savedRowIndex) < errNone)
		savedRowIndex = 0;

	// Get the record number that corresponds to the table item to draw.
	// The record number is stored in the "intValue" field of the item.
	rowIndex = TblGetRowData(table, row);
	DbCursorSetAbsolutePosition(gCursorID, rowIndex);

	// Don't draw the alternating darkened bars if the current
	// row is the selected row in the table.
	if(!TblGetSelection(table, &selectedRow, &selectedCol) || row != selectedRow)
	{
		// Only darken every other row.
		if( ((rowIndex-1) % 2) == 0 )
		{
			changedBackColor = true;
			WinSetBackColorRGB( &sListDarkenedListEntry, &oldBackColor );
			WinEraseRectangle( bounds, 0 );
		}
	}

	switch (column)
	{
		case kNameAndNumColumn:
			// We will use a custom draw only if the user has enbaled the tap dialing and he has currently
			// taped on a phone number.
			customDraw = (Boolean) (gEnableTapDialing && sPhoneNumberSelectedTableRow == row && sPhoneNumberSelection);
			phoneX = ToolsDrawRecordNameAndPhoneNumber(gCursorID, bounds, gAddrListFont, customDraw, sPhoneNumberSelected);
			TblSetRowID(table, row, phoneX);			// Store in table for later tap testing
			break;

		case kNoteColumn:
			// Draw a note symbol if the field has a note.
			if (AddressExistingNote(gCursorID))
			{
				currFont = FntSetFont (symbolFont);
				noteChar = symbolNote;
				WinDrawChars (&noteChar, 1, bounds->topLeft.x, bounds->topLeft.y);
				FntSetFont (currFont);
			}
			break;
	}

	// Set back to the old background if we changed it
	if(changedBackColor)
		WinSetBackColorRGB(&oldBackColor, NULL);

	// Move the cursor back to its previous position
	if (savedRowIndex > 0)
		DbCursorSetAbsolutePosition(gCursorID, savedRowIndex);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListNumberOfRows
 *
 * DESCRIPTION: This routine return the maximun number of visible rows,
 *              with the current list view font setting.
 *
 * PARAMETERS:  table - List View table
 *
 * RETURNED:    maximun number of displayable rows
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * art	08/28/97	Initial Revision
 *
 ***********************************************************************/
static uint16_t PrvListNumberOfRows(TablePtr table)
{
	uint16_t		rows;
	uint16_t		rowsInTable;
	uint16_t		tableHeight;
	FontID			currFont;
	RectangleType	r;

	rowsInTable = TblGetNumberOfRows (table);

	TblGetBounds (table, &r);
	tableHeight = r.extent.y;

	currFont = FntSetFont (gAddrListFont);
	rows = tableHeight / FntLineHeight ();
	FntSetFont (currFont);

	return (rows <= rowsInTable) ? rows : rowsInTable;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListLookupSearchInit
 *
 * DESCRIPTION: Initializes the SearchLib structure for lookup entries
 *
 * PARAMETERS:  ->	searchP: pointer to the SearchLib struct
 *				->	startRecordIndex: the record index to search search from
 *				->	direction: dmSeekForward or dmSeekBackward
 *
 * RETURNED:    errNone or an error code
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		02/21/03	Initial Revision
 *
 ***********************************************************************/
static status_t PrvListLookupSearchInit(SearchLibType *searchP, uint32_t startRecordIndex, SearchDirectionType direction)
{
	FieldType *	fieldP;
	char *		fieldTextP;
	uint16_t	lookupColumnIDsRscID;
	MemHandle	listH = NULL;
	status_t	err;

	fieldP = (FieldType*) ToolsGetFrmObjectPtr(FrmGetFormPtr(ListView), ListLookupField);
	fieldTextP = FldGetTextPtr(fieldP);
	StrCopy(sLookupText, fieldTextP);

	memset(searchP, 0, sizeof(SearchLibType));

	// Load the u32l resource list that contain the resource ID of the u32l that contains the list of coluns ID to use
	// The index in this u32l have to match the index of the sort index.
	lookupColumnIDsRscID = (uint16_t)U32ListGetItem(gApplicationDbP, OrderByLookupColumnIDResIDUIn32List, NULL, gOrderByIndex);
	if (!lookupColumnIDsRscID)
		return dmErrResourceNotFound;

	// Load the Column ID resource list. If the list doesn't exist, search in all the table's columns
	if ((listH = U32ListGetList(gApplicationDbP, lookupColumnIDsRscID)) != NULL)
	{
		// Get the list items number.
		searchP->numCols = (uint16_t)U32ListGetItemNum(NULL, 0, listH);
		searchP->colIDsP = (uint32_t*)DmHandleLock(listH);
	}

	searchP->searchDB			= gAddrDB;
	searchP->cursorID			= gCursorID;
	searchP->schemaName			= kAddressDefaultTableName;
	searchP->startRowIndex		= startRecordIndex;
	searchP->startColumnIndex	= 0;
	searchP->numCategories		= gCurrentNumCategories;
	searchP->catIDsP			= gCurrentCategoriesP;
	searchP->matchMode			= DbMatchAny;
	searchP->textToSearch		= sLookupText;
	searchP->recordDirection	= direction;
	searchP->columnDirection	= direction;
	searchP->interruptible		= false;
	searchP->resumePolicy		= kSearchResumeChangeRecord;
	searchP->hidePrivate		= (Boolean) (gPrivateRecordVisualStatus == maskPrivateRecords);
	// Use TxtCompare instead of TxtFindString. This will match double-bytes
	// column not depending of their caracters set (Hiragana, Katakana, etc.)
	searchP->useCompare			= true;
	searchP->minCompareMatch	= strlen(sLookupText);

	err = SearchLibInit(searchP);

	DmHandleUnlock(listH);
	U32ListReleaseList(listH);

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListUpdateScrollButtonsLookup
 *
 * DESCRIPTION: Show or hide the list view scroll buttons
 *				when table entries are filtered by lookup field content
 *
 * PARAMETERS:  frmP  - Application List form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		02/21/03	Initial Revision
 *
 ***********************************************************************/
static void PrvListUpdateScrollButtonsLookup(FormType* frmP)
{
	SearchLibType	search;
	uint32_t		rowIndex;
	Boolean			scrollableUp = false;
	Boolean			scrollableDown = false;
	TableType *		tableP;
	uint16_t		upIndex;
	uint16_t		downIndex;
	int16_t			row;
	FormType *		formP;

	if (PrvListLookupSearchInit(&search, 1, kSearchMoveBackward) < errNone)
		return;

	formP = FrmGetFormPtr(ListView);
	tableP = ToolsGetFrmObjectPtr(formP, ListTable);

	if (TblRowUsable(tableP, 0) &&
		(rowIndex = TblGetRowData(tableP, 0)) != kInvalidRowIndex &&
		rowIndex > kFirstRowIndex)
	{
		search.startRowIndex = rowIndex - 1;
		SearchLibReset(&search);
		scrollableUp = SearchLibSearch(&search);
	}

	// Find the record in the last row of the table
	row = TblGetLastUsableRow(tableP);
	if (row != tblUnusableRow)
	{
		rowIndex = TblGetRowData(tableP, row);

		if (rowIndex < DbCursorGetRowCount(search.cursorID))
		{
			search.recordDirection = dmSeekForward;
			search.startRowIndex = rowIndex + 1;
			SearchLibReset(&search);

			scrollableDown = SearchLibSearch(&search);
		}
	}

	SearchLibRelease(&search);

	// Update the scroll button.
	upIndex = FrmGetObjectIndex(frmP, ListUpButton);
	downIndex = FrmGetObjectIndex(frmP, ListDownButton);
	FrmUpdateScrollers(formP, upIndex, downIndex, scrollableUp, scrollableDown);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListUpdateScrollButtons
 *
 * DESCRIPTION: Show or hide the list view scroll buttons.
 *
 * PARAMETERS:  frmP  - Application List form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date      Description
 * -----	--------	-----------
 * roger	06/21/95   	Initial Revision
 * aro		09/25/00	Adding frmP as a parameter for frmUpdateEvent
 *
 ***********************************************************************/
static void PrvListUpdateScrollButtons(FormType* frmP)
{
	uint16_t	row;
	uint16_t	upIndex;
	uint16_t	downIndex;
	Boolean		scrollableUp = false;
	Boolean		scrollableDown = false;
	TableType*	tblP;
	FieldType *	fieldP;
	uint32_t	rowIndex;

	// Check lookup field content.
	fieldP = ToolsGetFrmObjectPtr(frmP, ListLookupField);
	if (FldGetTextLength(fieldP) > 0)
	{
		PrvListUpdateScrollButtonsLookup(frmP);
		return;
	}

	// Update the button that scroll the list.
	//
	// If the first record displayed is not the fist record in the category,
	// enable the up scroller.
	scrollableUp = (Boolean) (gTopVisibleRowIndex > kFirstRowIndex);

	// Find the record in the last row of the table
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
	row = TblGetLastUsableRow(tblP);
	if (row != tblUnusableRow)
	{
		rowIndex = TblGetRowData(tblP, row);
		scrollableDown = (Boolean) (rowIndex < DbCursorGetRowCount(gCursorID));
	}

	// Update the scroll button.
	upIndex = FrmGetObjectIndex(frmP, ListUpButton);
	downIndex = FrmGetObjectIndex(frmP, ListDownButton);
	FrmUpdateScrollers(frmP, upIndex, downIndex, scrollableUp, scrollableDown);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListIsRowIDInTable
 *
 * DESCRIPTION: Check if the passed rowID id part (currently displayed)
 *				of the table
 *
 * PARAMETERS:  ->	formP
 *				->	rowID: the databas row ID to check
 *				<-	foundRowIndexP: the table row index is returned here
 *
 * RETURNED:    True if found, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		10/09/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvListIsRowIDInTable(FormType *formP, uint32_t rowID, int16_t *foundRowIndexP)
{
	TableType *	tableP;
	uint32_t	rowIndex;
	int16_t		foundRowIndex;

	if (gCursorID == dbInvalidCursorID || DbCursorGetPositionForRowID(gCursorID, rowID, &rowIndex) < errNone)
		goto Exit;

	tableP = ToolsGetFrmObjectPtr(formP, ListTable);
	if (TblFindRowData(tableP, rowIndex, &foundRowIndex))
	{
		if (foundRowIndexP)
			*foundRowIndexP = foundRowIndex;

		return true;
	}

Exit:

	if (foundRowIndexP)
		*foundRowIndexP = tblUnusableRow;

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListLoadTableLookup
 *
 * DESCRIPTION: This routine loads address book database records into
 *              the list view form by filtering them with the content
 *				of the lookup field
 *
 * PARAMETERS:  frmP  - Application List form
 *
 * RETURNED:    true if table loaded
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		02/19/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvListLoadTableLookup(FormType *formP)
{
	SearchLibType	search;
	uint16_t		visibleRows;
	uint16_t		row = 0;
	TableType *		tableP;
	FontID			currFont;
	Boolean			found = false;
	int16_t			lineHeight;
	uint16_t		attr;
	Boolean			masked;
	uint32_t		length;
	uint32_t		dataSize;
	FieldType *		fieldP;

	fieldP = ToolsGetFrmObjectPtr(formP, ListLookupField);
	// Nohting in the lookup field. Let the table be loaded normally
	if ((length = FldGetTextLength(fieldP)) == 0)
		return false;

	tableP = ToolsGetFrmObjectPtr(formP, ListTable);
	visibleRows = PrvListNumberOfRows(tableP);

	currFont = FntSetFont(gAddrListFont);
	lineHeight = FntLineHeight();
	FntSetFont(currFont);

	// Exit if no row in cursor. We need to delete the entered char
	if (! DbCursorGetRowCount(gCursorID))
		goto Exit;

	if (PrvListLookupSearchInit(&search, gTopVisibleRowIndex, kSearchMoveForward) < errNone)
		return false;

	// Load the table with search result
	while (row < visibleRows)
	{
		if (!SearchLibSearch(&search))
			break;

		// We only select entries where found text is displayed
		dataSize = 0;
		switch (search.foundColID)
		{
		case kAddrColumnIDWorkCompany:
		case kAddrColumnIDYomiCompanyName:
			DbCopyColumnValue(search.searchDB, search.foundRowID, kAddrColumnIDLastName, 0, NULL, &dataSize);
			if (gCurrentOrderByType == kOrderByNameType && dataSize)
				continue;
			break;

		case kAddrColumnIDFirstName:
		case kAddrColumnIDYomiFirstName:
			DbCopyColumnValue(search.searchDB, search.foundRowID, kAddrColumnIDWorkCompany, 0, NULL, &dataSize);
			if (gCurrentOrderByType == kOrderByCompanyType && dataSize)
				continue;
			break;

		default: // kAddrColumnIDLastName || kAddrColumnIDYomiLastName
			break;
		}

		if (!found)
		{
			// If the onehanded navigation focus is on the table, we must not change
			// gCurrentRowID otherwise the ring won't be drawn on the correct record
			if (!sOnehandedTableFocused)
				gCurrentRowID = search.foundRowID;

			found = true;
		}

		// Make the row usable.
		TblSetRowUsable(tableP, row, true);

		DbGetRowAttr(search.searchDB, search.foundRowID, &attr);

		masked = (Boolean) (((attr & dbRecAttrSecret) && (gPrivateRecordVisualStatus == maskPrivateRecords)));
		TblSetRowMasked(tableP, row, masked);

		// Mark the row invalid so that it will draw when we call the draw routine.
		TblMarkRowInvalid(tableP, row);

		// Store the record number as the row id.
		TblSetRowData(tableP, row, search.foundRowIndex);

		TblSetItemFont(tableP, row, kNameAndNumColumn, gAddrListFont);
		TblSetRowHeight(tableP, row, lineHeight);

		row++;
	}

	SearchLibRelease(&search);

Exit:
	if (found)
	{
		// Set remaining rows unusable only if found something (which means
		// that there are some modifications in the table
		while (row < visibleRows)
		{
			TblSetRowUsable(tableP, row, false);
			++row;
		}
	}
	else if (!gDeviceHasFEP || TsmGetFepMode() == tsmFepModeOff)
	{
		// No record matches. Delete the last character in the field.
		// Remember to use the Text Mgr to figure out how many bytes
		// this last character occupies in the string.
		// If the Fep mode is on, we don't delete the last char to
		// be able to enter double bytes characters
		char * textP = FldGetTextPtr(fieldP);
		size_t charSize = TxtPreviousCharSize(textP, length);

		FldDelete(fieldP, length - charSize, length);

		SndPlaySystemSound (sndError);

		// No more text in the lookup field. Proceed a normal LoadTable
		if (length == charSize)
			return false;
	}

	PrvListUpdateScrollButtons(formP);

	if (!PrvListIsRowIDInTable(formP, gCurrentRowID, NULL))
		gCurrentRowID = dbInvalidRowID;
	// If the onehanded navigation focus is on the table, we must not move the cursor
	// otherwise the ring won't be drawn on the correct record
	else if (!sOnehandedTableFocused)
		DbCursorMoveToRowID(gCursorID, gCurrentRowID);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListLoadTable
 *
 * DESCRIPTION: This routine loads address book database records into
 *              the list view form.
 *
 * PARAMETERS:  frmP  - Application List form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name   	  Date		Description
 * ----		--------	-----------
 * art  	06/05/95    Initial Revision
 * aro		09/25/00	Adding frmP as a parameter for frmUpdateEvent
 * LFe		10/16/02	Use new Database Manager
 *						Change TblXXXRowID by TblXXXRowData as record
 *						index are now uint32_t and no more uint16_t
 *
 ***********************************************************************/
static void PrvListLoadTable(FormType* frmP)
{
	uint16_t	row;
	uint16_t	numRows;
	uint16_t	lineHeight;
	uint16_t	visibleRows;
	FontID		currFont;
	TableType *	tblP;
	uint16_t	attr;
	Boolean		masked;
	uint32_t	numCursorRows;
	uint32_t	rowID;
	uint32_t	currentPos;

	// In some case, like for Security, we close the database and the current cursor
	// but the security dialog post an update event. We can't do anything while the database is closed.
	if (gCursorID == dbInvalidCursorID)
		return;

	// Load the table by filtering entries with the lookup field content
	if (PrvListLoadTableLookup(frmP))
		return;

	// For each row in the table, store the record number as the row id.
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);

	visibleRows = PrvListNumberOfRows(tblP);

	// Make the current selected record visible
	if (gCurrentRowID != dbInvalidRowID &&
		DbCursorGetPositionForRowID(gCursorID, gCurrentRowID, &currentPos) >= errNone)
	{
		if (currentPos < gTopVisibleRowIndex)
			gTopVisibleRowIndex = currentPos;
		else if (currentPos >= gTopVisibleRowIndex + visibleRows)
			gTopVisibleRowIndex = (currentPos >= visibleRows) ? currentPos - visibleRows + kFirstRowIndex : kFirstRowIndex;
	}

	// Adjust the top visible index so we always have a full page of records displayed.
	numCursorRows = DbCursorGetRowCount(gCursorID);
	if (gTopVisibleRowIndex + visibleRows - 1 > numCursorRows)
	{
		// We have at least one line without a record.  Fix it.
		gTopVisibleRowIndex = (numCursorRows <= visibleRows) ? kFirstRowIndex : numCursorRows - visibleRows + 1;
	}

	currFont = FntSetFont (gAddrListFont);
	lineHeight = FntLineHeight ();
	FntSetFont (currFont);

	// Set cursor position to the top record.
	DbCursorSetAbsolutePosition(gCursorID, gTopVisibleRowIndex);

	// And loop on records
	for (row = 0; row < visibleRows && !DbCursorIsEOF(gCursorID); row++)
	{
		DbCursorGetCurrentRowID(gCursorID, &rowID);

		// Make the row usable.
		TblSetRowUsable (tblP, row, true);

		DbGetRowAttr(gAddrDB, rowID, &attr);

		masked = (Boolean) (((attr & dbRecAttrSecret) && (gPrivateRecordVisualStatus == maskPrivateRecords)));
		TblSetRowMasked(tblP, row, masked);

		// Mark the row invalid so that it will draw when we call the draw routine.
		TblMarkRowInvalid(tblP, row);

		// Store the record number as the row id.
		TblSetRowData(tblP, row, gTopVisibleRowIndex + row);

		TblSetItemFont(tblP, row, kNameAndNumColumn, gAddrListFont);
		TblSetRowHeight(tblP, row, lineHeight);

		DbCursorMoveNext(gCursorID);
	}

	// Hide the item that don't have any data.
	numRows = TblGetNumberOfRows(tblP);
	while (row < numRows)
	{
		TblSetRowUsable (tblP, row, false);
		row++;
	}

	PrvListUpdateScrollButtons(frmP);

	if (!PrvListIsRowIDInTable(frmP, gCurrentRowID, NULL))
		gCurrentRowID = dbInvalidRowID;
	else
		DbCursorMoveToRowID(gCursorID, gCurrentRowID);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:	nothing
 *
 * RETURNED:	nothing.
 *
 *				The following global variables are modified:
 *					gCurrentCategory
 *					gCategoryName
 *
 * REVISION HISTORY:
 *
 * Name   	Date      	Description
 * ----  	---------	-----------
 * art   	06/05/95   	Initial Revision
 * gap	  	08/13/99   	Update to use new constant categoryDefaultEditCategoryString.
 * ppl		02/14/02	Add active input area support (AIA)
 * LFe		10/16/02	Use new Category Mgr and Database Schema
 *						Remove the return value. Unused.
 *
 ***********************************************************************/
static void PrvListSelectCategory (void)
{
	FormType *		frmP;
	CategoryID *	categoriesP = NULL;
	uint32_t		numCategories = 0;
	Boolean			categoryChanged;

	frmP = FrmGetFormPtr(ListView);

	// We need to close the cursor in case of categories merge.
	DbCursorClose(gCursorID);
	gCursorID = dbInvalidCursorID;

	categoryChanged = CatMgrSelectFilter(gAddrDB, frmP, ListCategoryTrigger, gCategoryName, ListCategoryList,
										gCurrentCategoriesP, gCurrentNumCategories, &categoriesP, &numCategories, true, NULL);

	if (categoryChanged)
	{
		// Cursor will be re-open here by ToolsChangeCategory
		PrvListChangeCategories(categoriesP, numCategories, false);
	}
	else
	{
		// Re-open the cursor
		AddressDBOpenCursor();
	}

	if (categoriesP)
		CatMgrFreeSelectedCategories(gAddrDB, &categoriesP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListNextCategory
 *
 * DESCRIPTION: This routine display the next category,  if the last
 *              catagory is being displayed
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 *              The following global variables are modified:
 *                     gCurrentCategory
 *                     gCategoryName
 *
 * REVISION HISTORY:
 *
 * Name		  Date      Description
 * ----		--------	-----------
 * art		09/15/95	Initial Revision
 * rsf		09/20/95	Copied from To Do
 * LFe		10/29/02	New Category Manager
 *
 ***********************************************************************/
static void PrvListNextCategory (void)
{
	CategoryID		categoryID;
	CategoryID		currentCategoryID;

	currentCategoryID = gCurrentCategoriesP ? *gCurrentCategoriesP : catIDUnfiled;
	CatMgrGetNext(gAddrDB, currentCategoryID, &categoryID);

	if (categoryID != currentCategoryID)
		PrvListChangeCategories(&categoryID, 1, true);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListScrollLookup
 *
 * DESCRIPTION: This routine scrolls the list of names and phone numbers
 *              in the direction specified. Because table entries are
 *				filtered it uses the SearchLib to seek records.
 *
 * PARAMETERS:  direction	- up or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		02/21/03	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvListScrollLookup(WinDirectionType direction)
{
	FormType *		formP;
	TableType *		tableP;
	int16_t			row;
	uint32_t		newTopRecordIndex = kFirstRowIndex;
	uint16_t		numVisibleRecords;
	SearchLibType	search;
	uint16_t		rowsInTable;

	formP = FrmGetFormPtr(ListView);
	tableP = ToolsGetFrmObjectPtr(formP, ListTable);
	rowsInTable = PrvListNumberOfRows(tableP);

	if (PrvListLookupSearchInit(&search, 1, (direction == winDown) ? kSearchMoveForward : kSearchMoveBackward) < errNone)
		return false;

	if (direction == winDown) // dmSeekForward
	{
		// Find the record in the last row of the table
		row = TblGetLastUsableRow(tableP);

		if (row != tblUnusableRow)
		{
			newTopRecordIndex = TblGetRowData(tableP, row);

			search.startRowIndex = newTopRecordIndex;
			SearchLibReset(&search);

			numVisibleRecords = 0;
			while (SearchLibSearch(&search) && numVisibleRecords < rowsInTable)
				numVisibleRecords++;

			// We have to go backward to fill an entire page
            if (numVisibleRecords < rowsInTable && newTopRecordIndex > 0)
			{
				search.startRowIndex = newTopRecordIndex - 1;
				search.recordDirection = kSearchMoveBackward;
				SearchLibReset(&search);

				while (SearchLibSearch(&search) && numVisibleRecords < rowsInTable)
				{
					numVisibleRecords++;
					newTopRecordIndex = search.foundRowIndex;
				}
			}
		}
	}

	// winUp
	else if (TblRowUsable(tableP, 0) && (newTopRecordIndex = TblGetRowData(tableP, 0)) != kInvalidRowIndex)
	{
		search.startRowIndex = newTopRecordIndex;
		SearchLibReset(&search);

		numVisibleRecords = 0;
		while (SearchLibSearch(&search) && numVisibleRecords < rowsInTable)
		{
			numVisibleRecords++;
			newTopRecordIndex = search.foundRowIndex;
		}
	}

	SearchLibRelease(&search);

	// Avoid redraw if no change
	if (gTopVisibleRowIndex != newTopRecordIndex)
	{
		gTopVisibleRowIndex = newTopRecordIndex;
		ListViewUpdateDisplay(true);
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListScroll
 *
 * DESCRIPTION: This routine scrolls the list of names and phone numbers
 *              in the direction specified.
 *
 * PARAMETERS:  direction	- up or dowm
 *              units		- unit amount to scroll
 *              byLine		- if true, list scrolls in line units
 *							- if false, list scrolls in page units
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 * art		06/05/95	Initial Revision
 * frigino	08/14/97	Modified to scroll by line or page in units
 * gap   	10/12/99	Close command bar before processing scroll
 * gap   	10/15/99	Clean up selection handling after scroll
 * gap   	10/25/99	Optimized scrolling to only redraw if item position changed
 * LFe		10/16/02	Use new Database Manager
 *
 ***********************************************************************/
static Boolean PrvListScroll(WinDirectionType direction, uint32_t units, Boolean byLine)
{
	TableType *	tblP;
	FormType *	frmP;
	FieldType *	fieldP;
	uint16_t	rowsInPage;
	uint32_t	newTopVisibleRecordIndex;
	uint32_t	numCursorRows;
	uint32_t	numLines;

	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus(0);

	frmP = FrmGetFormPtr(ListView);
	fieldP = ToolsGetFrmObjectPtr(frmP, ListLookupField);

	if (FldGetTextLength(fieldP) > 0)
		return PrvListScrollLookup(direction);

	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
	// Safe. There must be at least one row in the table.
	rowsInPage = PrvListNumberOfRows(tblP) - 1;
	newTopVisibleRecordIndex = gTopVisibleRowIndex;
	numCursorRows = DbCursorGetRowCount(gCursorID);

	// Scroll the table down.
	if (direction == winDown)
	{
		numLines = (byLine) ? units : units * rowsInPage;

		// Scroll down by the number of lines
		newTopVisibleRecordIndex += numLines;

		// Adjust newTopVisibleRecordIndex to fill one full page
		if (newTopVisibleRecordIndex + rowsInPage > numCursorRows)
			newTopVisibleRecordIndex = (numCursorRows > rowsInPage + kFirstRowIndex) ? numCursorRows - rowsInPage : kFirstRowIndex;
	}
	// Scroll the table up
	else
	{
		numLines = (byLine) ? units : units * rowsInPage;

		if (newTopVisibleRecordIndex > numLines)
			newTopVisibleRecordIndex -= numLines;
		else
			newTopVisibleRecordIndex = 1;
	}

	// Avoid redraw if no change
	if (gTopVisibleRowIndex != newTopVisibleRecordIndex)
	{
		gTopVisibleRowIndex = newTopVisibleRecordIndex;
		ListViewUpdateDisplay(true);
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListSelectTableRow
 *
 * DESCRIPTION: Select (highlight) the passed row index in the table
 *
 * PARAMETERS:  ->	formP
 *				->	rowIndex: the table row index to select
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		10/09/03	Initial revision
 *
 ***********************************************************************/
static void PrvListSelectTableRow(FormType *formP, int16_t rowIndex)
{
	TableType *	tableP;
	uint32_t	dbRowIndex;

	if (sOnehandedRecordSelected || rowIndex == tblUnusableRow)
		return;

	tableP = ToolsGetFrmObjectPtr(formP, ListTable);

	if (TblRowMasked(tableP, rowIndex))
	{
		gCurrentRowID = dbInvalidRowID;
		return;
	}

	dbRowIndex = TblGetRowData(tableP, rowIndex);
	DbCursorSetAbsolutePosition(gCursorID, dbRowIndex);
	DbCursorGetCurrentRowID(gCursorID, &gCurrentRowID);

	TblSetSelection(tableP, rowIndex, kNameAndNumColumn);
	TblMarkRowInvalid(tableP, rowIndex);
	TblRedrawTable(tableP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListFormResize
 *
 * DESCRIPTION: Update the display and handles resizing
 *
 * PARAMETERS:  frmP  - Application List form to resize
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name   	  Date      Description
 * ----   	--------	-----------
 * ppl		02/06/02	Initial release - Active InputArea Support
 *
 ***********************************************************************/
static void PrvListFormResize(FormType *frmP, RectangleType *newBoundsP)
{
	RectangleType 	objBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	uint16_t		numberOfRows;
	int16_t			row;
	int16_t			column;
	int16_t			xOffset;
	int16_t			yOffset;
	TableType*		tableP;
	Coord			width;
	FontID			currFont;

	xOffset =  newBoundsP->extent.x - gCurrentWinBounds.extent.x;
	yOffset =  newBoundsP->extent.y - gCurrentWinBounds.extent.y;

	// if the window has the height of the screen
	// then the windows is already at the right size
	if (!yOffset && !xOffset)
		return;

	tableP = (TableType*) FrmGetObjectPtr( frmP, FrmGetObjectIndex (frmP, ListTable));

	// These objects move only horizontally
	if (xOffset)
	{
		// Set column width
		width = TblGetColumnWidth(tableP, kNameAndNumColumn);
		TblSetColumnWidth(tableP, kNameAndNumColumn, width + xOffset);

		// Category trigger
		objIndex =  FrmGetObjectIndex (frmP, ListCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += xOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// Category list
		objIndex =  FrmGetObjectIndex (frmP, ListCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += xOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	// These objects only move vertically
	if (yOffset)
	{
		//ListLookUpLabel
		objIndex =  FrmGetObjectIndex (frmP, ListLookUpLabel);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// ListLookupField
		objIndex =  FrmGetObjectIndex (frmP, ListLookupField);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// ListNewButton
		objIndex =  FrmGetObjectIndex (frmP, ListNewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// ListDoneButton
		objIndex =  FrmGetObjectIndex (frmP, ListDoneButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// ListAttachButton
		objIndex =  FrmGetObjectIndex (frmP, ListAttachButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	// ListTable
	// Stick the table bottom to the 'New' button.
	// -3: want a minimum of 3 pixel between the bottom of the table and the top of the buttom
	currFont = FntSetFont(gAddrListFont);
	objIndex =  FrmGetObjectIndex (frmP, ListNewButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	TblGetBounds(tableP, &objBounds);
	objBounds.extent.y = y - 3 - objBounds.topLeft.y;
	objBounds.extent.x += xOffset;
	objBounds.extent.y -= (objBounds.extent.y % FntLineHeight());
	TblSetBounds(tableP, &objBounds);
	FntSetFont(currFont);

	if (TblGetSelection(tableP, &row, &column))
		TblUnhighlightSelection(tableP);

	numberOfRows = TblGetNumberOfRows(tableP);

	for (row = 0; row < numberOfRows; row++)
		TblSetRowUsable(tableP, row, false);

	// ListUpButton
	objIndex =  FrmGetObjectIndex (frmP, ListUpButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y += yOffset;
	x += xOffset;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// ListDownButton
	objIndex =  FrmGetObjectIndex (frmP, ListDownButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y += yOffset;
	x += xOffset;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// GSI  is not always the last item in form
	objIndex =  ToolsGetGraffitiObjectIndex(frmP);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y += yOffset;
	x += xOffset;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	gCurrentWinBounds = *newBoundsP;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListDrawPhoneSel
 *
 * DESCRIPTION: This callback draws a phone number inverted or not,
 *				depending on the pen position
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
Boolean PrvListDrawPhoneSel(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	char *	phoneP = NULL;
	Coord	phoneWidth = 0;
	FontID	currFont;

	if (!sPhoneNumberSelection || cmd == winInvalidateDestroy)
		return true;

	phoneP = ToolsGetPhoneNumber(gCursorID, sSelectedRecordPhoneColumnID, gAddrListFont, kMaxPhoneColumnWidth, &phoneWidth, NULL);
	if (!phoneP)
		return true;

	WinPushDrawState();

	currFont = FntSetFont(gAddrListFont);
	ToolsDrawTextLabel(phoneP, strlen(phoneP), diryRect->topLeft.x, diryRect->topLeft.y, sPhoneNumberSelected);
	FntSetFont(currFont);
	MemPtrFree(phoneP);

	WinPopDrawState();

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateDisplay
 *
 * DESCRIPTION: This routine update the display of the list view
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 * art		06/05/95	Initial Revision
 * ppo		10/13/99	Fixed bug #22753 (selection not redrawn)
 * jmp		10/19/99	Changed previous fix to actually to set everything
 *						back up.  The previous change caused bug #23053, and
 *						didn't work in several cases anyway!  Also, optimized
 *						this routine space-wise.
 * aro		09/26/00	Don't use GetActiveForm for frmRedrawUpdateCode
 *						Fix bug in debug ROM: selection was not restored after Dial or About
 * fpa		10/26/00	Fixed bug #44352 (Selected line display problem when tapping
 *						menu | Dial, then cancel into Dial Number screen)
 * LFe		10/16/02	Use new Category Mgr
 *
 ***********************************************************************/
void ListViewUpdateDisplay(Boolean reloadTable)
{
	FormType *	frmP;
	TableType *	tblP;

	// The user is selecting a phone number
	if (sPhoneNumberSelection)
	{
		// Only invalidate the selected item
		ToolsFrmInvalidateRectFunc(ListView, &sSelectedPhoneBounds, PrvListDrawPhoneSel, NULL);
		return;
	}

	frmP = FrmGetFormPtr(ListView);
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);

	// Unselected current item and reload table
	TblUnhighlightSelection(tblP);
	if (reloadTable)
		PrvListLoadTable(frmP);

	// Invalidate the whole window
	ToolsFrmInvalidateWindow(ListView);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListDeleteRecord
 *
 * DESCRIPTION: This routine deletes an address record. This routine is
 *              called when the delete button in the command bar is
 *              pressed when address book is in list view.  The benefit
 *				this routine proides over DetailsDeleteRecord is that the
 *				selection is maintained right up to the point where the address
 *				is being deleted.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the record was delete or archived.
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 * gap		11/01/99	new
 *
 ***********************************************************************/
static Boolean PrvListDeleteRecord(void)
{
	TablePtr	table;

	if (!ToolsConfirmDeletion())
		return false;

	// Clear the highlight on the selection before deleting the item.
	table = (TableType*) ToolsGetFrmObjectPtr(FrmGetFormPtr(ListView), ListTable);
	TblUnhighlightSelection(table);

	// Set cursor pos to the current record.
	DbCursorMoveToRowID(gCursorID, gCurrentRowID);
	AddressDBDeleteRecord(gSaveBackup);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    PrvListDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 * art		06/05/95	Initial Revision
 * jmp		10/01/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *						AddrGetDatabase().
 * jwm		08/10/99	After deleting a record, highlight the new selection and
 *						clear the now possibly completely wrong lookup field.
 * hou		11/28/00	bug #44076 correction: ListOptionsSecurityCmd returns true
 *
 ***********************************************************************/
static Boolean PrvListDoCommand (uint16_t command)
{
	uint32_t 	newRowID;
	uint32_t	size;
	uint16_t 	numCharsToHilite;
	uint32_t	columnIDToHilite;
	uint16_t 	mode;
	CategoryID	phoneCategory;
	EventType	newEvent;
	FormType*	frmP;

	switch (command)
	{
		case ListRecordDuplicateAddressCmd:
			MenuEraseStatus (0);

			if (gCurrentRowID == dbInvalidRowID)
			{
				SndPlaySystemSound(sndError);
				return true;
			}

			// Set cursor pos to the current record.
			DbCursorMoveToRowID(gCursorID, gCurrentRowID);
			AddressDBDuplicateRecord(gCursorID, &newRowID, &numCharsToHilite, &columnIDToHilite);
			// If we have a new record take the user to be able to edit it automatically.
			if (newRowID != dbInvalidRowID)
			{
				// Set gCurrentRowID now so AddressDBOpenCursor will set the cursor position to newRowID
				gCurrentRowID = newRowID;
				gNumCharsToHilite = numCharsToHilite;
				sSavedCategoryID = gCurrentCategoriesP ? *gCurrentCategoriesP : catIDUnfiled;
				gTappedColumnID = columnIDToHilite;
				AddressTabFindColIDTab(&gBookInfo, gTappedColumnID, &gBookInfo.currentTabIndex, &gBookInfo.currentTabId);
				FrmGotoForm (gApplicationDbP, EditView);
			}
			return true;

		case ListRecordDialCmd:
			MenuEraseStatus (0);

			if (gCurrentRowID == dbInvalidRowID)
			{
				SndPlaySystemSound (sndError);
				return true;
			}

			// Set cursor pos to the current record.
			DbCursorMoveToRowID(gCursorID, gCurrentRowID);

			// Get the columnID for the displayed phone
			size = sizeof(sSelectedRecordPhoneColumnID);

			if (DbCopyColumnValue(gAddrDB, gCursorID, kAddrColumnIDDisplayedPhone, 0, &sSelectedRecordPhoneColumnID, &size) < errNone)
			{
				SndPlaySystemSound (sndError);
				return true;
			}

			// Check if the Displayed Phone as a value and is a phone number, if the phone is not part of supported
			// dial phone number, let the table handle selection.
			if (!ToolsIsPhoneIndexSupported(gCursorID, sSelectedRecordPhoneColumnID))
			{
				SndPlaySystemSound (sndError);
				return true;
			}

			DialListShowDialog(gCursorID, sSelectedRecordPhoneColumnID);
			return true;

		case ListRecordImportPhoneCatCmd:
			MenuEraseStatus (0);

			// Call to ExgGet to get the directory from the phone
			if (TransferGetPhoneCategory(gAddrDB, &phoneCategory) >= errNone)
				PrvListChangeCategories((phoneCategory == catIDUnfiled) ? NULL : &phoneCategory,
										(phoneCategory == catIDUnfiled) ? 0 : 1, true);
			return true;

		case ListRecordExportPhoneCatCmd:
			MenuEraseStatus (0);
			TransferSendPhoneCategory(gAddrDB);
			ListViewUpdateDisplay(true);
			return true;

		case ListRecordDeleteRecordCmd:
			MenuEraseStatus(0);
			if (gCurrentRowID != dbInvalidRowID)
			{
				if (PrvListDeleteRecord())
					ListViewUpdateDisplay(true);
			}
			else
			{
				SndPlaySystemSound (sndError);
			}
			return true;

		case ListRecordBeamRecordCmd:
			MenuEraseStatus(0);

			if (gCurrentRowID != dbInvalidRowID)
				TransferSendRecord(gAddrDB, gCurrentRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(kTabAllId));
			else
				SndPlaySystemSound (sndError);

			return true;

		case ListRecordBeamBusinessCardCmd:
			MenuEraseStatus(0);
			ToolsAddrBeamBusinessCard();
			return true;


		case ListRecordBeamCategoryCmd:
			MenuEraseStatus (0);
			TransferSendCategory(gAddrDB, gCurrentNumCategories, gCurrentCategoriesP, exgBeamPrefix, NoDataToBeamAlert);
			return true;

		case ListRecordSendCategoryCmd:
			MenuEraseStatus (0);
			TransferSendCategory(gAddrDB, gCurrentNumCategories, gCurrentCategoriesP, exgSendPrefix, NoDataToBeamAlert);
			return true;

		case ListRecordAttachRecordCmd:
			MenuEraseStatus (0);
			ToolsAddrAttachRecord(gAddrDB, gCurrentRowID);
			return true;

		case ListRecordAttachCategoryCmd:
			MenuEraseStatus (0);
			TransferSendCategory(gAddrDB, gCurrentNumCategories, gCurrentCategoriesP, gTransportPrefix, NoDataToSendAlert);

			memset(&newEvent, 0, sizeof(newEvent));
			newEvent.eType = appStopEvent;
			EvtAddEventToQueue(&newEvent);
			return true;

		case ListOptionsFontCmd:
			{
				FontID newFont;

				MenuEraseStatus(0);
				newFont = ToolsSelectFont(ListView, gAddrListFont);

				// The update event for font changed is post so when the
				// item is highlighted in the updateEvent handler, the drawing was made with a bad font
				// So force unhighlight here
				if (newFont != gAddrListFont)
				{
					frmP = FrmGetFormPtr(ListView);
					TblUnhighlightSelection((TableType*)ToolsGetFrmObjectPtr(frmP, ListTable));
					// now set the new font
					gAddrListFont = newFont;
					ToolsComputePhoneLabelWidth();
					PrvListLoadTable(frmP);
				}
				return true;
			}

		case ListOptionsListByCmd:
			MenuEraseStatus (0);
			PrvListClearLookupString();
			FrmPopupForm (gApplicationDbP, PreferencesDialog);
			return true;

		case ListOptionsEditCustomFldsCmd:
			MenuEraseStatus (0);
			FrmPopupForm (gApplicationDbP, CustomEditDialog);
			return true;

		case ListOptionsSecurityCmd:
			MenuEraseStatus (0);
			// Because Address is the running applications, we must close the DB
			// in case the user taps on 'Lost password'. That way, the system will
			// be able to open the AddressDB for write operation and delete the
			// records marked as private.
			AddressTabReleaseSchemaInfoFromDB(&gBookInfo, gAddrDB);
			AddressDBCloseDatabase();

			gPrivateRecordVisualStatus = SecSelectViewStatus();

			mode = (gPrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

			AddressDBOpenDatabase(gApplicationDbP, mode, &gAddrDB);
			ErrFatalDisplayIf(!gAddrDB, "Can't reopen DB");
			AddressDBOpenCursor();
			AddressTabInitSchemaInfoFromDB(&gBookInfo, gAddrDB);

			//For safety, simply reset the currentRecord
			TblReleaseFocus (ToolsGetFrmObjectPtr(FrmGetFormPtr(ListView), ListTable));
			ListViewUpdateDisplay(true);
			//updateSelectCurrentRecord will cause currentRecord to be reset to dbInvalidRowID if hidden or masked
			return true; // Hou: bug #44076 correction: used to be "break;" -> caused the event to not be handled

		case ListOptionsAboutCmd:
			MenuEraseStatus (0);
			AbtShowAbout (sysFileCAddress);
			return true;
	}
	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListHandleTableEnter
 *
 * DESCRIPTION: This routine handles table selection in the list view,
 *					 either selecting the name to go to RecordView, or selecting
 *					 the phone number to dial.
 *
 *
 * PARAMETERS:	 event	- pointer to the table enter event
 *
 * RETURNED:	 whether the event was handled
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * ----		--------	-----------
 * peter	05/08/00	Initial Revision
 * aro		06/22/00	Add dialing checking and feature
 * aro		09/25/00	Disable selection when entering the table
 * LFe		10/16/02	Use new Database Manager
 *						switch TblXXXRowID by TblXXXRowData as record
 *						index are now stored in row data and phoneX in
 *						row ID
 *
 ***********************************************************************/
static void PrvListHandleTableEnter(EventType* eventP)
{
	TableType *			tableP;
	int16_t				row, column;
	Coord				startPhoneX;
	Coord				endPhoneX;
	uint32_t			size = 0;
	uint32_t			rowIndex;

	tableP = eventP->data.tblEnter.pTable;

	sPhoneNumberSelectedTableRow = eventP->data.tblEnter.row;

	gCurrentRowID = dbInvalidRowID;

	// Deselect the row if it is selected.
	TblUnhighlightSelection(tableP);

	// Check if "Enable tap-dialing" is set in prefs
	if (!gEnableTapDialing)
		return;

	row = eventP->data.tblSelect.row;
	column = eventP->data.tblSelect.column;

	// If the column being tapped on isn't the name and number column,
	// let the table handle selection to view the note.
	if (column != kNameAndNumColumn)
		return;

	// If the record is masked, dialing is impossible, so just let
	//	the table handle selection.
	if (TblRowMasked(tableP, row))
		return;

	// Extract the x coordinate of the start of the phone number for this row.
	// This was computed and stored in the row id when the row was drawn.
	startPhoneX = TblGetRowID(tableP, row);
	if (startPhoneX == 0) // 0 means no phone displayed
		return;

	// Get the record phone bounds
	TblGetItemBounds(tableP, row, column, &sSelectedPhoneBounds);
	sSelectedPhoneBounds.extent.x -= startPhoneX + gPhoneLabelWidth - sSelectedPhoneBounds.topLeft.x;		// Maintain same right side.
	sSelectedPhoneBounds.topLeft.x = startPhoneX;
	endPhoneX = startPhoneX + sSelectedPhoneBounds.extent.x;

	// If the user tapped on the name rather than the number, or on the space between them,
	// let the table handle selection so the user gets to view the record.
	if (eventP->screenX < startPhoneX || eventP->screenX > endPhoneX)
		return;

	// Move cursor to the selected row
	rowIndex = TblGetRowData(tableP, row);
	// Move cursor
	DbCursorSetAbsolutePosition(gCursorID, rowIndex);

	// The columnID for the displayed phone
	size = sizeof(sSelectedRecordPhoneColumnID);

	if (DbCopyColumnValue(gAddrDB, gCursorID, kAddrColumnIDDisplayedPhone, 0, &sSelectedRecordPhoneColumnID, &size) < errNone)
		return;

	// If there is no phone number, if the phone is not part of supported
	// dial phone number, let the table handle selection.
	if (!ToolsIsPhoneIndexSupported(gCursorID, sSelectedRecordPhoneColumnID))
		return;

	// At this point, the user tapped on the phone number, so instead of letting
	// the table handle selection, we highlight just the phone number.

	// Store the newly selected rowID
	DbCursorGetCurrentRowID(gCursorID, &gCurrentRowID);

	sPhoneNumberSelection = true;
	sPhoneNumberSelected = true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListInit
 *
 * DESCRIPTION: This routine initializes the "List View" of the
 *              Address application.
 *
 * PARAMETERS:  frmP  - Application List form
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name   	 Date      		Description
 * ----		------      	-----------
 * art   	06/05/95      	Initial Revision
 * ppl		02/06/02		Add active Input Area support (AIA)
 * LFe		10/16/02		Use new Category Mgr and Database Schema
 *
 ***********************************************************************/
static void PrvListInit( FormType* frmP )
{
	uint16_t 		row;
	uint16_t 		rowsInTable;
	TableType*	 	tblP;
	ControlType*	ctl;

	// Initialize the address list table.
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
	rowsInTable = TblGetNumberOfRows(tblP);

	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle(tblP, row, kNameAndNumColumn, customTableItem);
		TblSetItemStyle(tblP, row, kNoteColumn, customTableItem);
		TblSetRowUsable(tblP, row, false);
	}

	TblSetColumnUsable(tblP, kNameAndNumColumn, true);
	TblSetColumnUsable(tblP, kNoteColumn, true);

	TblSetColumnMasked(tblP, kNameAndNumColumn, true);
	TblSetColumnMasked(tblP, kNoteColumn, true);

	// Set the callback routine that will draw the records.
	TblSetCustomDrawProcedure (tblP, kNameAndNumColumn, PrvListDrawRecord);
	TblSetCustomDrawProcedure (tblP, kNoteColumn, PrvListDrawRecord);

	// Set the label of the category trigger.
	ctl = ToolsGetFrmObjectPtr( frmP, ListCategoryTrigger );
	if (gCurrentNumCategories > 1)
	{
		while (gCurrentNumCategories--)
		{
			if (gCurrentCategoriesP[gCurrentNumCategories] == sSavedCategoryID)
			{
				gCurrentCategoriesP[0] = sSavedCategoryID;
				break;
			}
		}
		gCurrentNumCategories = 1;
	}

	CatMgrSetTriggerLabel(gAddrDB, gCurrentCategoriesP, gCurrentNumCategories, ctl, gCategoryName);

	if (gCalledFromApp)
	{
		uint16_t	doneIndex = FrmGetObjectIndex(frmP, ListDoneButton);
		FrmShowObject(frmP, doneIndex);
	}

	if (gAttachRequest)
	{
		uint16_t	attachIndex = FrmGetObjectIndex(frmP, ListAttachButton);
		FrmShowObject(frmP, attachIndex);
	}

	// Select the record. This finds which row to select it and does it.
	if (gListViewSelectThisRowID != dbInvalidRowID)
	{
		gCurrentRowID = gListViewSelectThisRowID;
		gListViewSelectThisRowID = dbInvalidRowID;
	}

	// Focused lookup field
	FrmSetFocus(frmP, FrmGetObjectIndex(frmP, ListLookupField));

	// Reset scroll rate
	PrvListResetScrollRate();

	// Onehanded navigation stuff
	sOnehandedTableFocused = false;
	sOnehandedRecordSelected = false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListViewRecord
 *
 * DESCRIPTION: Check is the current record can be displayed and
 *				go to the RecordView if ok.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if record can be displayed
 *              false otherwise
 *
 * REVISION HISTORY:
 *
 * Name   	 Date      		Description
 * ----		------      	-----------
 * TEs   	09/08/2003     	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvListViewRecord(void)
{
	if (!AddressDBRecordContainsData(gCursorID))
	{
		FormType *	formP = FrmGetFormPtr(ListView);
		TableType *	tableP = ToolsGetFrmObjectPtr(formP, ListTable);

		FrmAlert(gApplicationDbP, EmptyRecordAlert);
		AddressDBDeleteRecord(false);

		TblUnhighlightSelection(tableP);
		PrvListLoadTable(formP);
		ToolsFrmInvalidateWindow(ListView);

		return false;
	}

	FrmGotoForm(gApplicationDbP, RecordView);
	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvListSetTableRowFocusRing
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
static void PrvListSetTableRowFocusRing(FormType *formP, int16_t tableRowIndex)
{
	TableType *		tableP;
	RectangleType	bounds;
	Coord			phoneX;

	if (!sOnehandedTableFocused || !sOnehandedRecordSelected)
		return;

	FrmClearFocusHighlight();
	tableP = ToolsGetFrmObjectPtr(formP, ListTable);
	TblGetItemBounds(tableP, tableRowIndex, 0, &bounds);

	if (gEnableTapDialing && (phoneX = TblGetRowID(tableP, tableRowIndex)) > 0)
	{
		if (sOneHandedPhoneSelected)
		{
			bounds.extent.x = bounds.topLeft.x + bounds.extent.x - phoneX;
			bounds.topLeft.x = phoneX;
		}
		else
		{
			bounds.extent.x = phoneX - bounds.topLeft.x;
		}
	}

	FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
}

/***********************************************************************
 *
 * FUNCTION:    PrvListHandleOnehandNavigation
 *
 * DESCRIPTION: Handle onehanded navigation events for the ListTable object
 *
 * PARAMETERS:  ->	eventP: the event containing the keyDown structure
 *
 * RETURNED:    true if handled, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name   	 Date      		Description
 * ----		------      	-----------
 * TEs   	03/23/2004     	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvListHandleOnehandNavigation(EventType *eventP)
{
	Boolean					handled = false;
	FormType *				formP;
	uint16_t				lastTblRowIndex;
	uint32_t				cursorPosition;
	TableType *				tableP;
	WinDirectionType		direction;
	Boolean					scroll;
	Boolean					redraw;
	FrmNavStateFlagsType	navState = 0;
	int16_t					rowIndex;
	uint32_t				cursorIndex;
	uint32_t				phoneColID;
	uint32_t				size;
	uint32_t				savedRowID;
	uint16_t				objID;

	formP = FrmGetFormPtr(ListView);

	switch (eventP->data.keyDown.chr)
	{
	case vchrRockerUp:
	case vchrRockerDown:
		direction = (eventP->data.keyDown.chr == vchrRockerUp) ? winUp : winDown;

		if (!sOnehandedTableFocused || !sOnehandedRecordSelected)
		{
			// If we are in interaction mode, release field focus
			// so the application will get vchrPageUp/Down
			if (FrmGetNavState(formP, &navState) >= errNone &&
				(navState & kFrmNavStateFlagsObjectFocusMode) == 0)
			{
				savedRowID = gCurrentRowID;
				gCurrentRowID = dbInvalidRowID;

				if (!PrvListScroll(direction, 1, false) && (eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
				{
					gCurrentRowID = savedRowID;

					objID = (direction == winUp) ? ListCategoryTrigger : ListNewButton;
					FrmSetFocus(formP, FrmGetObjectIndex(formP, objID));
					FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
					FrmNavObjectTakeFocus(formP, objID);
				}

				handled = true;
			}
			else if (sOnehandedTableFocused)
			{
				savedRowID = gCurrentRowID;
				gCurrentRowID = dbInvalidRowID;

				handled = (Boolean)(PrvListScroll(direction, 1, false) || (eventP->data.keyDown.modifiers & autoRepeatKeyMask) != 0);

				if (!handled)
					gCurrentRowID = savedRowID;
			}

			break;
		}

		// No focused entry in the table. Skip it.
		if (gCurrentRowID == dbInvalidRowID)
			break;

		// Move cursor position to the current record
		DbCursorMoveToRowID(gCursorID, gCurrentRowID);
		DbCursorGetCurrentPosition(gCursorID, &cursorPosition);

		if (direction == winUp)
		{
			// First table row selected: scroll one line up
			scroll = (Boolean)(cursorPosition == gTopVisibleRowIndex);

			// Move cursor to the previous row.
			if (cursorPosition > kFirstRowIndex)
				redraw = (Boolean)(DbCursorMovePrev(gCursorID) == errNone);
			else
				redraw = scroll = false;
		}
		else
		{
			// Last table row selected: go to the next form object
			tableP = ToolsGetFrmObjectPtr(formP, ListTable);
			lastTblRowIndex = TblGetLastUsableRow(tableP);
			scroll = (Boolean) (TblGetRowData(tableP, lastTblRowIndex) == cursorPosition);

			// Move cursor one row ahead.
			if (cursorPosition < DbCursorGetRowCount(gCursorID))
				redraw = (Boolean)(DbCursorMoveNext(gCursorID) == errNone);
			else
				scroll = redraw = false;
		}

		DbCursorGetCurrentRowID(gCursorID, &gCurrentRowID);

		if (scroll)
		{
			// No need to update display. This is done by PrvListScroll
			PrvListScroll(direction, 1, true);
		}
		else if (redraw)
		{
			if (PrvListIsRowIDInTable(formP, gCurrentRowID, &rowIndex))
				PrvListSetTableRowFocusRing(formP, rowIndex);
		}
		else if ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0) // Exit from table only if it's not a repeating keyDown
			break;

		handled = true;
		break;

	case vchrRockerRight:
		if (sOnehandedTableFocused)
		{
			handled = true;

			if (gEnableTapDialing && sOnehandedRecordSelected && !sOneHandedPhoneSelected)
			{
				sOneHandedPhoneSelected = true;

				formP = FrmGetFormPtr(ListView);
				tableP = ToolsGetFrmObjectPtr(formP, ListTable);

				if (PrvListIsRowIDInTable(formP, gCurrentRowID, &rowIndex) && TblGetRowID(tableP, rowIndex) > 0)
				{
					// Draw the focus ring around the phone number
					PrvListSetTableRowFocusRing(formP, rowIndex);
					break;
				}
			}

			// Focus goes to the "New" button
			FrmNavObjectTakeFocus(formP, ListNewButton);
		}
		// If not in object focus mode, rockerRight gives the focus to the "New" button
		else if (FrmGetNavState(formP, &navState) >= errNone &&
				(navState & kFrmNavStateFlagsObjectFocusMode) == 0)
		{
			// force interaction mode to the 'New' button.
			FrmSetFocus(formP, FrmGetObjectIndex(formP, ListNewButton));
			FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
			FrmNavObjectTakeFocus(formP, ListNewButton);
			handled = true;
		}
		break;

	case vchrRockerLeft:
		if (sOnehandedTableFocused && sOnehandedRecordSelected)
		{
			handled = true;

			// Handle navigation between name and phone number within the same row
			if (gEnableTapDialing && sOneHandedPhoneSelected)
			{
				sOneHandedPhoneSelected = false;

				formP = FrmGetFormPtr(ListView);
				tableP = ToolsGetFrmObjectPtr(formP, ListTable);

				if (PrvListIsRowIDInTable(formP, gCurrentRowID, &rowIndex) && TblGetRowID(tableP, rowIndex) > 0)
				{
					PrvListSetTableRowFocusRing(formP, rowIndex);
					break;
				}
			}

			// Give the focus back to the whole table.
			sOnehandedRecordSelected = false;
			FrmSetFocus(formP, FrmGetObjectIndex(formP, ListTable));
			FrmNavObjectTakeFocus(formP, ListTable);
		}
		// If not in object focus mode, rockerLeft gives focus to the Tab group
		else if (FrmGetNavState(formP, &navState) >= errNone &&
				(navState & kFrmNavStateFlagsObjectFocusMode) == 0)
		{
			// force interaction mode to the 'New' button.
			FrmSetFocus(formP, FrmGetObjectIndex(formP, ListCategoryTrigger));
			FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
			FrmNavObjectTakeFocus(formP, ListCategoryTrigger);
			handled = true;
		}
		break;

	case vchrRockerCenter:
		// The table is focused
		if (sOnehandedTableFocused)
		{
			// Nothing to select. Exit.
			if (gCursorID == dbInvalidCursorID || DbCursorGetRowCount(gCursorID) == 0)
				break;

			if (!sOnehandedRecordSelected)
			{
				if (gCurrentRowID == dbInvalidRowID || !PrvListIsRowIDInTable(formP, gCurrentRowID, &rowIndex))
					rowIndex = 0;

				tableP = ToolsGetFrmObjectPtr(formP, ListTable);
				cursorIndex = TblGetRowData(tableP, rowIndex);
				DbCursorSetAbsolutePosition(gCursorID, cursorIndex);
				DbCursorGetCurrentRowID(gCursorID, &gCurrentRowID);

				sOnehandedRecordSelected = true;
				sOneHandedPhoneSelected = true;
				PrvListSetTableRowFocusRing(formP, rowIndex);
			}
			else if (gCurrentRowID != dbInvalidRowID)
			{
				DbCursorMoveToRowID(gCursorID, gCurrentRowID);

				size = sizeof(phoneColID);

				// If tap dialing is disabled, go to the record view.
				if (!gEnableTapDialing || !sOneHandedPhoneSelected ||
					DbCopyColumnValue(gAddrDB, gCursorID, kAddrColumnIDDisplayedPhone, 0, &phoneColID, &size) < errNone ||
                    !DialListShowDialog(gCursorID, phoneColID))
					PrvListViewRecord();
			}

			handled = true;
		}

		// The table is not focused
		else
		{
			FrmGetNavState(formP, &navState);

			// If we are in Interaction mode, explicilty give the focus to
			// the table. Otherwise the field would have taken the focus.
			if ((navState & kFrmNavStateFlagsObjectFocusMode) == 0)
			{
				FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
				FrmNavObjectTakeFocus(formP, ListTable);
				handled = true;
			}
		}
		break;

	default:
		break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    ListHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List View"
 *              of the Address Book application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *	Name	 Date		Description
 *	----	------		-----------
 *	art		06/05/95	Initial Revision
 *	frigino	08/15/97	Added scroll rate acceleration using
 *						PrvListResetScrollRate() and PrvListAdjustScrollRate()
 *	jmp		09/17/99	Use NewNoteView instead of NoteView.
 *	peter	04/25/00	Add support for un-masking just the selected record.
 *	peter	05/08/00	Add support for tapping on phone numbers to dial.
 *	aro		06/22/00	Add check for dialing abilities
 *	aro		09/25/00	use cmdTextH rather than cmdText to avoir unreleased resource
 *  LFe		10/16/02	Use new Category Mgr and Database Schema
 *
 ***********************************************************************/
Boolean ListHandleEvent(EventType* eventP)
{
	FormType *	frmP;
	Boolean 	handled = false;
	TablePtr 	table;
	int16_t 	row;
	int16_t 	column;
	MemHandle 	cmdTextH;
	uint32_t 	numLibs;
	Boolean		selected;
	uint32_t	rowIndex;
	int16_t		tableRowIndex = tblUnusableRow;
	uint32_t	savedRowID;

	switch (eventP->eType)
	{
		case frmOpenEvent:
			frmP = FrmGetFormPtr(ListView);
			PrvListInit(frmP);
			handled = true;
			break;

		case frmCloseEvent:
			// Switch to interaction form before change form
			FrmSetNavState(FrmGetFormPtr(ListView), kFrmNavStateFlagsInteractionMode);
			break;

		case tblEnterEvent:
			PrvListHandleTableEnter(eventP);
			break;

		case tblExitEvent:
			sPhoneNumberSelection = false;
			sPhoneNumberSelected = false;
			sPhoneNumberSelectedTableRow = 0x7FFF;

			gCurrentRowID = dbInvalidRowID;
			TblUnhighlightSelection(eventP->data.tblExit.pTable);
			break;

		case tblSelectEvent:
			if (sPhoneNumberSelection)
			{
				sPhoneNumberSelection = false;

				if (sPhoneNumberSelected)
				{
					sPhoneNumberSelected = false;
					sPhoneNumberSelectedTableRow = 0x7FFF;

					if (!DialListShowDialog(gCursorID, sSelectedRecordPhoneColumnID))
						PrvListViewRecord();
					else
						ListViewUpdateDisplay(false);
				}
				else
				{
					// Discard selection
					gCurrentRowID = dbInvalidRowID;
					TblUnhighlightSelection(eventP->data.tblSelect.pTable);
				}

				break;
			}

			// Get current rowID
			rowIndex = TblGetRowData(eventP->data.tblSelect.pTable, eventP->data.tblSelect.row);
			DbCursorSetAbsolutePosition(gCursorID, rowIndex);
			DbCursorGetCurrentRowID(gCursorID, &gCurrentRowID);

			if (TblRowMasked(eventP->data.tblSelect.pTable, eventP->data.tblSelect.row))
			{
				if (!AddressDBVerifyPassword())
				{
					// Update display to remove the deleted masked records (if user taps on 'Lost password')
					ListViewUpdateDisplay(true);
					break;
				}

				eventP->data.tblSelect.column = kNameAndNumColumn; //force non-note view
			}

			// An item in the list of names and phone numbers was selected, go to
			// the record view.
			if (eventP->data.tblSelect.column == kNameAndNumColumn)
			{
				sSavedCategoryID = gCurrentCategoriesP ? *gCurrentCategoriesP : catIDUnfiled;
				PrvListViewRecord();
			}
			else
			{
				AddressEditNote(gCursorID);

				if (!AddressExistingNote(gCursorID) && !AddressDBRecordContainsData(gCursorID))
				{
					// The note has been removed and the record is empty
					AddressDBDeleteRecord(false);
					ListViewUpdateDisplay(true);
				}
			}
			handled = true;
			break;

		case penMoveEvent:
			if (!sPhoneNumberSelection)
				break;

			selected = RctPtInRectangle(eventP->screenX, eventP->screenY, &sSelectedPhoneBounds);

			if (sPhoneNumberSelected != selected)
			{
				sPhoneNumberSelected = selected;
				ListViewUpdateDisplay(false);
			}
			break;

		case ctlEnterEvent:
			switch (eventP->data.ctlEnter.controlID)
			{
				case ListUpButton:
				case ListDownButton:
					// Reset scroll rate
					PrvListResetScrollRate();
					// leave unhandled so the buttons can repeat
					break;
			}
			break;


		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case ListCategoryTrigger:
					PrvListSelectCategory();
					ListViewUpdateDisplay(true);
					handled = true;
					break;

				case ListNewButton:
					EditNewRecord();
					handled = true;
					break;

				case ListDoneButton:
					// Go back to the caller application
					if (gCalledFromApp && gCallerAppDbID != 0)
						SysUIAppSwitch(gCallerAppDbID, sysAppLaunchCmdReturnFromPanel, NULL, 0);
					break;

				case ListAttachButton:
					if (!gAttachRequest)
						break;

					ToolsAddrAttachRecord(gAddrDB, gCurrentRowID);
					handled = true;
					break;
			}
			break;

		case ctlRepeatEvent:
			// Adjust the scroll rate if necessary
			PrvListAdjustScrollRate();

			switch (eventP->data.ctlRepeat.controlID)
			{
				case ListUpButton:
					gCurrentRowID = dbInvalidRowID;
					PrvListScroll (winUp, sScrollUnits, false);
					// leave unhandled so the buttons can repeat
					break;

				case ListDownButton:
					gCurrentRowID = dbInvalidRowID;
					PrvListScroll (winDown, sScrollUnits, false);
					// leave unhandled so the buttons can repeat
					break;
			}
			break;


		case keyDownEvent:
			// Address Book key pressed for the first time?
			if (TxtCharIsHardKey(eventP->data.keyDown.modifiers, eventP->data.keyDown.chr))
			{
				if (! (eventP->data.keyDown.modifiers & poweredOnKeyMask))
				{
					PrvListClearLookupString ();
					PrvListNextCategory ();
					handled = true;
				}
			}

			else if (EvtKeydownIsVirtual(eventP))
			{
				handled = PrvListHandleOnehandNavigation(eventP);
				if (handled)
					break;

				switch (eventP->data.keyDown.chr)
				{
					case vchrPageUp:
						// Reset scroll rate if not auto repeating
						if ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
						{
							PrvListResetScrollRate();
						}
						// Adjust the scroll rate if necessary
						PrvListAdjustScrollRate();
						savedRowID = gCurrentRowID;
						gCurrentRowID = dbInvalidRowID;
						if (PrvListScroll(winUp, sScrollUnits, false))
						{
							sOnehandedTableFocused = false;
							sOnehandedRecordSelected = false;
							FrmSetNavState(FrmGetFormPtr(ListView), kFrmNavStateFlagsInteractionMode);
						}
						else
							gCurrentRowID = savedRowID;

						handled = true;
						break;

					case vchrPageDown:
						// Reset scroll rate if not auto repeating
						if ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
						{
							PrvListResetScrollRate();
						}
						// Adjust the scroll rate if necessary
						PrvListAdjustScrollRate();
						savedRowID = gCurrentRowID;
						gCurrentRowID = dbInvalidRowID;
						if (PrvListScroll(winDown, sScrollUnits, false))
						{
							sOnehandedTableFocused = false;
							sOnehandedRecordSelected = false;
							FrmSetNavState(FrmGetFormPtr(ListView), kFrmNavStateFlagsInteractionMode);
						}
						else
							gCurrentRowID = savedRowID;

						handled = true;
						break;

					case vchrSendData:
						if (gCurrentRowID != dbInvalidRowID)
						{
							MenuEraseStatus(0);
							TransferSendRecord(gAddrDB, gCurrentRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(kTabAllId));
						}
						else
						{
							SndPlaySystemSound(sndError);
						}
						handled = true;
						break;

					default:
						break;
				}
			}

			else if (eventP->data.keyDown.chr == linefeedChr)
			{
				frmP = FrmGetFormPtr(ListView);
				table = ToolsGetFrmObjectPtr(frmP, ListTable);

				if (TblGetSelection (table, &row, &column))
				{
					sSavedCategoryID = gCurrentCategoriesP ? *gCurrentCategoriesP : catIDUnfiled;
					PrvListViewRecord();
				}
				handled = true;
			}

			else
			{
				FieldType *	fieldP;
				char *		textP;
				uint32_t	len;

				frmP = FrmGetFormPtr(ListView);

				FrmSetFocus(frmP, FrmGetObjectIndex(frmP, ListLookupField));

				fieldP = (FieldType*)ToolsGetFrmObjectPtr(frmP, ListLookupField);
				len = FldGetTextLength(fieldP);
				textP = FldGetTextPtr(fieldP);

				// Handle backspace.
				if (eventP->data.keyDown.chr == chrBackspace &&
					(!textP || len <= TxtGetNextChar(textP, 0, NULL)))
				{
					// Unhilight selection
					gCurrentRowID = dbInvalidRowID;

					// If the field was already empty, don't update the table to avoid flickering
					if (!len)
					{
						handled = true;
						break;
					}
				}

				// Reset top visible record index
				gTopVisibleRowIndex = kFirstRowIndex;

				handled = FldHandleEvent((FieldType*)ToolsGetFrmObjectPtr(frmP, ListLookupField), eventP);
				// Send an update. The table will be reload using the content of the lookup field.
				ListViewUpdateDisplay(true);
			}

			break;

		case tsmConfirmEvent:
			ListViewUpdateDisplay(true);
			break;

		case fldChangedEvent:
			ListViewUpdateDisplay(true);
			handled = true;
			break;

		case menuEvent:
			handled =  PrvListDoCommand (eventP->data.menu.itemID);
			break;


		case menuCmdBarOpenEvent:
		{
			DmOpenRef	uilibdbP = NULL;
			uint32_t	uiLibRefNum = 0;
			status_t	err;

			err = SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
			if (err >= errNone)
				err = SysGetModuleDatabase(uiLibRefNum, NULL, &uilibdbP);
			ErrNonFatalDisplayIf(err < errNone, "Can't get UILibRefNum");

			if (gCurrentRowID != dbInvalidRowID)
			{
				// because this isn't a real menu command, get the text for the button from a resource
				cmdTextH = DmGetResource(gApplicationDbP, strRsc, DeleteRecordStr);
				MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarDeleteBitmap, menuCmdBarResultMenuItem, ListRecordDeleteRecordCmd, DmHandleLock(cmdTextH));
				DmHandleUnlock(cmdTextH);
				DmReleaseResource(cmdTextH);
			}

			MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarSecureBitmap, menuCmdBarResultMenuItem, ListOptionsSecurityCmd, 0);

			if (gCurrentRowID != dbInvalidRowID)
			{
				// because this isn't a real menu command, get the text for the button from a resource
				cmdTextH = DmGetResource(gApplicationDbP, strRsc, BeamRecordStr);
				MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarBeamBitmap, menuCmdBarResultMenuItem, ListRecordBeamRecordCmd, DmHandleLock(cmdTextH));
				DmHandleUnlock(cmdTextH);
				DmReleaseResource(cmdTextH);
			}


			// tell the field package to not add cut/copy/paste buttons automatically; we
			// don't want it for the lookup field since it'd cause confusion.
			eventP->data.menuCmdBarOpen.preventFieldButtons = true;

			if (err >= errNone)
				SysUnloadModule(uiLibRefNum);

			// don't set handled to true; this eventP must fall through to the system.
			break;
		}


		case menuOpenEvent:
			if (! gAttachRequest)
			{
				//SysModuleInfoType	moduleInfo;

				if(!ToolsIsDialerPresent())
					MenuHideItem(ListRecordDialCmd);

				numLibs = 0;
				if ((ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) >= errNone)  && numLibs)
					MenuShowItem(ListRecordSendCategoryCmd);
				else
					MenuHideItem(ListRecordSendCategoryCmd);

				numLibs = 0;
				if ((ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgPhoneBookIOScheme) >= errNone) && numLibs)
				//if (SysGetModuleInfo(sysFileTExgLib, sysFileCPhoneBookIOLib, 0, &moduleInfo) >= errNone)
				{
					MenuShowItem(ListRecordImportPhoneCatCmd);
					MenuShowItem(ListRecordExportPhoneCatCmd);
					MenuShowItem(ListRecordSeparator2);
				}
				else
				{
					MenuHideItem(ListRecordImportPhoneCatCmd);
					MenuHideItem(ListRecordExportPhoneCatCmd);
					MenuHideItem(ListRecordSeparator2);
				}
			}
			else
			{
				// Hide send, import & export menu items
				MenuHideItem(ListRecordDialCmd);
				MenuHideItem(ListRecordSendCategoryCmd);
				MenuHideItem(ListRecordImportPhoneCatCmd);
				MenuHideItem(ListRecordExportPhoneCatCmd);
				MenuHideItem(ListRecordSeparator2);
				MenuHideItem(ListRecordBeamCategoryCmd);
				MenuHideItem(ListRecordBeamBusinessCardCmd);

				// Show attachments menu items
				MenuShowItem(ListRecordAttachRecordCmd);
				MenuShowItem(ListRecordAttachCategoryCmd);

			}
			// don't set handled = true
			break;

		case winUpdateEvent:
			frmP = FrmGetFormPtr(ListView);
			if (eventP->data.winUpdate.window != FrmGetWindowHandle(frmP))
				break;

			// In some case, like for Security, we close the database and the current cursor
			// but the security dialog post an update event. We can't do anything while the database is closed.
			if (gCursorID == dbInvalidCursorID)
				break;

			// Avoid the system to redraw by itself
			handled = true;

			// If gCurrentRowID is valid, but is not visible in the table, post
			// a kListViewReloadTableEvent to reload the table. This done this way to avoid
			// table reload within an update event.
			if (gCurrentRowID != dbInvalidRowID &&
				!PrvListIsRowIDInTable(frmP, gCurrentRowID, &tableRowIndex) &&
				DbCursorGetPositionForRowID(gCursorID, gCurrentRowID, &rowIndex) >= errNone)
			{
				gTopVisibleRowIndex = rowIndex;
				PrvPostUserEvent(kListViewReloadTableEvent);
				break;
			}

			FrmDrawForm(frmP);

			// Do we have something to select
			PrvListSelectTableRow(frmP, tableRowIndex);

			// Draw the onehanded focus ring if needed
			PrvListSetTableRowFocusRing(frmP, tableRowIndex);
			break;

		case winResizedEvent:
			frmP = FrmGetFormPtr(ListView);
			if (eventP->data.winResized.window != FrmGetWindowHandle(frmP))
				break;

			PrvListFormResize(frmP, &eventP->data.winResized.newBounds);
			PrvListLoadTable(frmP);
			handled = true;
			break;

		case kListViewReloadTableEvent:
			frmP = FrmGetFormPtr(ListView);
			TblUnhighlightSelection((TableType*)ToolsGetFrmObjectPtr(frmP, ListTable));
			PrvListLoadTable(frmP);
			ToolsFrmInvalidateWindow(ListView);
			break;

		case frmObjectFocusTakeEvent:
			if (eventP->data.frmObjectFocusTake.objectID == ListTable)
			{
				RectangleType	bounds;
				uint16_t		objIndex;

				sOnehandedTableFocused = true;
				if (!sOnehandedRecordSelected)
				{
					frmP = FrmGetFormPtr(ListView);
					objIndex = FrmGetObjectIndex(frmP, ListTable);
					FrmSetFocus(frmP, objIndex);
					FrmGetObjectBounds(frmP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(frmP), &bounds, 0);
				}

				handled = true;
			}
			break;

		case frmObjectFocusLostEvent:
			if (eventP->data.frmObjectFocusLost.objectID == ListTable)
			{
				int16_t	tableRowIndex;
				frmP = FrmGetFormPtr(ListView);

				sOnehandedTableFocused = false;
				if (sOnehandedRecordSelected && PrvListIsRowIDInTable(frmP, gCurrentRowID, &tableRowIndex))
				{
					sOnehandedRecordSelected = false;
					ListViewUpdateDisplay(false);
				}
			}
			break;
	}

	return (handled);
}
