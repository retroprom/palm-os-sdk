/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoList.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#ifdef PIM_APPS_PROFILING
#include <PIMAppsProfiling.h>
#else
#define PIMAppProfilingBegin(name)
#define PIMAppProfilingEnd()
#define PIMAppProfilingReturnVoid()	return
#define PIMAppProfilingReturnValue(val)	(return (val))
#endif

#include <PalmTypes.h>
#include <Event.h>
#include <Form.h>
#include <UIResources.h>
#include <FontSelect.h>
#include <ExgLocalLib.h>
#include <Table.h>
#include <CmnRectTypes.h>
#include <ErrorMgr.h>
#include <StringMgr.h>
#include <Control.h>
#include <ScrollBar.h>
#include <CatMgr.h>
#include <UIColor.h>
#include <PenInputMgr.h>
#include <Preferences.h>
#include <DebugMgr.h>
#include <FormLayout.h>
#include <TraceMgr.h>
#include <Loader.h>
#include <string.h>
#include <KeyMgr.h>
#include <PalmTypesCompatibility.h>

#include "MemoRsc.h"
#include "MemoMain.h"
#include "MemoPrefs.h"
#include "MemoDB.h"
#include "MemoTransfer.h"

// change ALERT
extern void TblInvalidate(TableType *);

#define int_abs(a) (((a) >= 0) ? (a) : (-a))

/************************************************************
 *	Global variables
 ************************************************************/
extern uint32_t			gListViewCursorID;
extern Boolean			gCheckingPassword;

/************************************************************
 *	Local Static variables
 ************************************************************/
static uint32_t			sTopRowPositionInCategory = 1;
static Boolean			ProcessingUpdate = false;
//static Boolean			gTableNavFocus = false;

static Boolean			sOneHandedTableHasFocus;
static Boolean			sOneHandedRecordSelected;

// globals used by ListViewStartDragging and ListViewDragMemo
static struct {
	int16_t			row;
	int16_t			selectedRow;
	int16_t			prevMoveRow;
	uint32_t		recordNum;
	uint32_t		selectedRecord;
	Coord			x, y;
	Boolean			moving;
	Boolean			selected;
	Boolean			dragging;
	RectangleType	r;
	RectangleType	indicatorR;
} gListViewSelection ;

int16_t				gSelectedRow = -1;

// the actual cache and its size
memolist_t *		gMemolistCache = NULL;
int16_t				gMemolistCacheSize = 0;

// the offset within the cache list above of the first row in the list
int16_t				gMemolistCacheOffset = 0;



/************************************************************
 *	Local functions
 ************************************************************/
static void		ListViewDisplayMask (RectanglePtr bounds);
static void		ListViewResize(FormType *formP, RectangleType *newBoundsP);
static void		ListViewLoadRecords(FormType* formP, Boolean updateCategoryTrigger);
static void		ListViewDrawRecord (void * table, int16_t row, int16_t column, RectangleType * bounds);
static Boolean	ListViewScroll(int16_t linesToScroll);

/***********************************************************************
 *
 * FUNCTION:    SelectFont
 *
 * DESCRIPTION: This routine handles selection of a font in the List
 *              View.
 *
 * PARAMETERS:  currFontID - id of current font
 *
 * RETURNED:    id of new font
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/10/97	Initial Revision
 *
 ***********************************************************************/
static FontID SelectFont (FontID currFontID)
{
	uint16_t formID;
	FontID fontID;

	formID = FrmGetFormId(FrmGetFormPtr(ListView));

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (currFontID);

	if (fontID != currFontID)
		UtilsFrmInvalidateWindow(formID);

	return (fontID);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/29/95	Initial Revision
 *			kcr	11/7/95	converted to common about box
 *			jmp	10/02/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *								MemoGetDatabase().
 *			jmp	03/19/00	Fixed bug #23669:  Adjust the number of memos currently
 *								available so that scrollbar will be updated correctly
 *								after a change in security level.
 *
 ***********************************************************************/
static Boolean ListViewDoCommand (uint16_t command)
{
	Boolean		wasHiding;
	uint16_t 	mode;
	EventType	newEvent;
	FormType	*frmP;
	Boolean		handled = true;
	FontID		selectedFont;

	switch (command)
	{
		case ListOptionsFontsCmd:
			MenuEraseStatus(0);
			selectedFont = SelectFont(gListFont);
			if (gListFont == selectedFont)
				break ;
			// reload the list view
			gListFont = selectedFont;
			frmP = FrmGetFormPtr(ListView);
			ListViewLoadRecords(frmP, true);
			UtilsFrmInvalidateWindow(ListView);
			break;

		case ListRecordBeamCategoryCmd:
			MenuEraseStatus(0);
			MemoSendCategory(gMemoDB, gCurrentNumCategories, gCurrentCategoriesP, exgBeamPrefix, NoDataToBeamAlert);
			break;

		case ListRecordSendCategoryCmd:
			MenuEraseStatus(0);
			MemoSendCategory(gMemoDB, gCurrentNumCategories, gCurrentCategoriesP, exgSendPrefix, NoDataToSendAlert);
			break;

		case ListRecordAttachCategoryCmd:
			MenuEraseStatus(0);
			MemoSendCategory(gMemoDB, gCurrentNumCategories, gCurrentCategoriesP, gTransportPrefix, NoDataToSendAlert);
			// Send quit event to quit application
			MemSet(&newEvent, sizeof(newEvent), 0);
			newEvent.eType = appStopEvent;
			EvtAddEventToQueue(&newEvent);
			break;

		case ListOptionsSecurityCmd:
			MenuEraseStatus(0);
			wasHiding = (gPrivateRecordVisualStatus == hidePrivateRecords);

			// the DB must be closed so that private records can be removed if the user
			// lost his password.
			DbCursorClose(gListViewCursorID);
			gListViewCursorID = dbInvalidCursorID;
			DbCloseDatabase(gMemoDB);
			gMemoDB = NULL;

			gCheckingPassword = true;
			gPrivateRecordVisualStatus = SecSelectViewStatus();
			gCheckingPassword = false;

			if (!gMemoDB)
				gMemoDB = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			ErrFatalDisplayIf(!gMemoDB, "Can't reopen DB");

			if (wasHiding ^ (gPrivateRecordVisualStatus == hidePrivateRecords)) //xor on two logical values - mode to open DB has changed
			{
				// Close the application's data file.
				MemoSavePrefs(0);

				DbCursorClose(gListViewCursorID);
				gListViewCursorID = dbInvalidCursorID;
				DbCloseDatabase(gMemoDB);

				mode = (gPrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

				gMemoDB = MemoDBOpenDatabase(mode);
				ErrFatalDisplayIf(!gMemoDB, "Can't reopen DB");
				// Read the preferences.
				MemoLoadPrefs();
			}

			//For safety, simply reset the gCurrentRecord
			gCurrentRecord = noRecordSelected;
			frmP = FrmGetFormPtr(ListView);
			ListViewLoadRecords(frmP, true);
			UtilsFrmInvalidateWindow(ListView);
			break;

		case ListOptionsPreferencesCmd:
			MenuEraseStatus(0);
			FrmPopupForm (gApplicationDbP, PreferencesDialog);
			break;

		case ListOptionsAboutCmd:
			MenuEraseStatus(gCurrentMenu);
			AbtShowAbout(sysFileCMemo);
			break;

		default:
			handled = false;
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewNumberOfRows
 *
 * DESCRIPTION: This routine return the maximun number of visible rows,
 *              with the current list view font setting.
 *
 * PARAMETERS:  table - List View table
 *
 * RETURNED:    maximun number of displayable rows
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/28/97	Initial Revision
 *
 ***********************************************************************/
static uint16_t ListViewNumberOfRows (TablePtr table)
{
	uint16_t		rows;
	uint16_t		rowsInTable;
	uint16_t		tableHeight;
	FontID			currFont;
	RectangleType	r;

	rowsInTable = TblGetNumberOfRows (table);

	TblGetBounds (table, &r);
	tableHeight = r.extent.y;

	currFont = FntSetFont (gListFont);
	rows = tableHeight / FntLineHeight ();
	FntSetFont (currFont);

	if (rows <= rowsInTable)
		return (rows);
	else
		return (rowsInTable);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawRecord
 *
 * DESCRIPTION: This routine draws the title memo record in the list
 *              view.  This routine is called by the table routine,
 *              TblDrawTable, each time a line of the table needs to
 *              be drawn.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - bound to the draw region
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			ADH	7/21/99	Increased the size of the posStr character array
 *								to allow for display of five digit numbers.
 *								Previously only four digit numbers could be
 *								displayed.
 *			ryw	1/11/01	use global sTopRowPositionInCategory to determine numbering
 *
 ***********************************************************************/
static void ListViewDrawRecord (void * table, int16_t row, int16_t column,
								RectangleType * bounds)
{
	int16_t			x, y;
	static char		posStr[12];
	uint32_t		pos;
	uint16_t		attr = 0;
	RectangleType	maskRectangle;
	int16_t			offset;
	RGBColorType lastColor;
	Boolean restoreLastColor = FALSE;;

	PIMAppProfilingBegin("ListViewDrawRecord");

	// Get the record number that corresponds to the table item to draw.
	// The record number is stored in the "intValue" field of the item.
	pos = sTopRowPositionInCategory + row;
	DbCursorSetAbsolutePosition(gListViewCursorID, pos);

	// Don't draw the alternating darkened bars if the current
	// row is the selected row in the table.
	if( row != gSelectedRow )
	{
		// Only darken every other row.
		if( ((pos-1) % 2) == 0 )
		{
			restoreLastColor = TRUE;

			WinSetBackColorRGB( &gMemoListDarkenedListEntry, &lastColor );
			WinEraseRectangle( bounds, 0 );
		}
	}

	// derive the cache location
	offset = (gMemolistCacheOffset+row) % gMemolistCacheSize;

TraceOutput(TL(appErrorClass, "Memo drawing record at row %d cache offset %d", row, offset));

	// fetch the cached attributes
	attr = gMemolistCache[offset].attr;

	// If the record is private and we are to hide private records, then get out of here.
	// This should be taken care of by the calling function, but we will go ahead and
	// take care of it here also.
	if ((attr & dbRecAttrSecret) && gPrivateRecordVisualStatus == hidePrivateRecords)
	{
		goto Exit;
	}

	x = bounds->topLeft.x + 1;
	y = bounds->topLeft.y;

	FntSetFont (gListFont);

	// If we are here then we either we either mask the memo out or display the
	// memo title.
	if (((attr & dmRecAttrSecret) && gPrivateRecordVisualStatus == maskPrivateRecords))
	{
		size_t	posStrLen;
		Coord	charWidth;

		memmove (&maskRectangle, bounds, sizeof (RectangleType));
		maskRectangle.topLeft.x = x;
		maskRectangle.extent.x = bounds->extent.x - x;
		StrIToA(posStr, pos);
		posStrLen = StrLen(posStr);
		posStr[posStrLen++] = '.';
		posStr[posStrLen] = '\0';
		charWidth = FntCharsWidth(posStr, posStrLen) + gMemolistCache[offset].indent;
		maskRectangle.topLeft.x += charWidth ;
		maskRectangle.extent.x -= charWidth;
		WinDrawChars(gMemolistCache[offset].title, (int16_t)posStrLen,
			x + gMemolistCache[offset].indent, y);
		ListViewDisplayMask (&maskRectangle);
	}
	else
	{
		// Fix refresh issues
		if (gListViewSelection.dragging)
			WinEraseRectangle( bounds, 0 );
		// Display the memo's title from the cache
		WinDrawChars(gMemolistCache[offset].title,
			strlen(gMemolistCache[offset].title),
			x + gMemolistCache[offset].indent, y);
	}
	PIMAppProfilingEnd();

 Exit:
	if( restoreLastColor )
	{
		WinSetBackColorRGB( &lastColor, NULL );
	}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewDisplayMask
 *
 * DESCRIPTION: Draws the masked display for the record.
 *
 * PARAMETERS:  bounds (Input):  The bounds of the table item to display.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         css    06/21/99   Initial Revision
 *
 ***********************************************************************/
static void ListViewDisplayMask (RectanglePtr bounds)
{
	RectangleType tempRect;
	CustomPatternType origPattern;
	MemHandle	bitmapH;
	BitmapType * bitmapP;
	DmOpenRef	uiLibDBP = NULL;
	uint32_t	uiLibRefNum = 0;

	CustomPatternType pattern = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

	memmove (&tempRect, bounds, sizeof (RectangleType));
	// Make sure it fits nicely into the display.
	tempRect.topLeft.y++;
	tempRect.extent.y --;
	tempRect.extent.x -= SecLockWidth + 5;

	WinGetPattern(&origPattern);
	WinSetPattern (&pattern);
	WinFillRectangle (&tempRect, 0);
	WinSetPattern(&origPattern);

	// Load UILIb to get its module DB
	SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
	SysGetModuleDatabase(uiLibRefNum, NULL, &uiLibDBP);

	//draw lock icon
	bitmapH = DmGetResource(uiLibDBP, bitmapRsc, SecLockBitmap);
	if (bitmapH)
	{
		bitmapP = MemHandleLock (bitmapH);
		WinDrawBitmap (bitmapP, (Coord)(tempRect.topLeft.x + tempRect.extent.x + 1),
					(Coord)(tempRect.topLeft.y + ((tempRect.extent.y - SecLockHeight) / 2)));
		MemPtrUnlock (bitmapP);
	}

	// Unload the UILib
	SysUnloadModule(uiLibRefNum);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the list view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  formP          -  pointer to the to do list form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewUpdateScrollers (FormType *formP)
{
	uint32_t	pos;
	uint16_t	rows;
	uint32_t	maxValue;
	uint32_t	memosInCategory;

	memosInCategory = MemoDBNumVisibleRecordsInCategory(gListViewCursorID);

	rows = ListViewNumberOfRows (GetObjectPtr(ListTable, ListView));
	if (memosInCategory > rows)
	{
		pos = gTopVisibleRecord - 1;
		maxValue = memosInCategory - rows;
		if (maxValue > 0x7FFF)
			maxValue = 0x7FFF;
	}
	else
	{
		pos = 0;
		maxValue = 0;
	}

	SclSetScrollBar(GetObjectPtr(ListScrollBar, ListView), (int16_t)pos, 0, (int16_t)maxValue, (int16_t)rows);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewLoadTable
 *
 * DESCRIPTION: This routine loads memo database records into
 *              the list view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/16/95	Initial Revision
 *			grant	1/29/99	Set the heights of unused rows
 *			ryw	1/11/01	update global sTopRowPositionInCategory on table load
 *
 ***********************************************************************/
static void ListViewLoadTable(FormType *formP)
{
	int16_t			row, offset, chr, len;
	uint32_t		recordNum;
	uint32_t		CurrentRecordIndex = noRecordSelected;
	uint16_t		lineHeight;
	uint16_t		dataHeight;
	uint16_t		tableHeight;
	uint16_t		numRows;
	uint32_t		numChars;
	int32_t			width, ellipsisWidth, indentWidth;
	FontID			currFont;
	TablePtr 		table;
	RectangleType	r;
	status_t		err;
	uint16_t		attr;
	char			posStr[8];
	char			*p, *buffer, *bufferP;

	PIMAppProfilingBegin("ListViewLoadTable");

	table = GetObjectPtr (ListTable, ListView);

	TblGetBounds (table, &r);
	tableHeight = r.extent.y;

	currFont = FntSetFont (gListFont);

	ellipsisWidth = FntCharWidth(chrEllipsis);
	indentWidth = FntCharWidth('1');
	lineHeight = FntLineHeight ();

	// The table width now includes the scrollbar.
	width = (int32_t)(r.extent.x - 1);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, ListScrollBar), &r);
	width -= r.extent.x;

	FntSetFont (currFont);

	dataHeight = 0;

	recordNum = gTopVisibleRecord;

	// For each row in the table, store the record number in the table item
	// that will dispaly the record.
	numRows = TblGetNumberOfRows (table);

	// if necessary, expand the size of the cache
	if (gMemolistCacheSize < numRows)
	{
		UInt16 cacheSize = numRows * kNumPagesInCache;
		gMemolistCache = (memolist_t *) MemPtrRealloc(gMemolistCache, (uint32_t)(cacheSize * sizeof(memolist_t)));
		for (row = gMemolistCacheSize; row < cacheSize; row++)
		{
			gMemolistCache[row].title = NULL;
			gMemolistCache[row].titleAlloc = 0;
			gMemolistCache[row].attr = 0;
			gMemolistCache[row].cursorPos = 0;
		}

		gMemolistCacheSize = cacheSize;
	}

	row = 0;

	if (gCurrentRecord != noRecordSelected)
	{
		err = DbCursorGetPositionForRowID(gListViewCursorID, gCurrentRecord, &CurrentRecordIndex);
		if (err < errNone)
			CurrentRecordIndex = noRecordSelected;
	}

	err = DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	// store the position of the first row so we can use sTopRowPositionInCategory+row
	// when drawing
	if (err >= errNone)
		DbCursorGetCurrentPosition(gListViewCursorID, &sTopRowPositionInCategory);
	else
	{
		sTopRowPositionInCategory = 1;
		err = DbCursorSetAbsolutePosition(gListViewCursorID, sTopRowPositionInCategory);
	}

	currFont = FntSetFont (gListFont);
	while (row < gMemolistCacheSize && err >= errNone)
	{
		// Check to see if this record is already in the cache
		// If not, load it in
		offset = (gMemolistCacheOffset+row) % gMemolistCacheSize;
		if (gMemolistCache[offset].cursorPos != (sTopRowPositionInCategory + row))
		{
			if ((err = DbGetRowAttr(gMemoDB, gListViewCursorID, &gMemolistCache[offset].attr)) >= errNone)
			{
				if ((gMemolistCache[offset].attr & dbRecAttrSecret) &&
					(gPrivateRecordVisualStatus == hidePrivateRecords))
				{
					// Skip secret records when the user asks for them to be hidden
					DbCursorMoveNext(gListViewCursorID);
					continue;
				}

				// label the cache position with this position in the cursor
				gMemolistCache[offset].cursorPos = (sTopRowPositionInCategory + row);
				gMemolistCache[offset].indent = 0;

				StrIToA (posStr, (int32_t) (sTopRowPositionInCategory + row));
				chr = strlen(posStr);

				if (chr < 2)
					gMemolistCache[offset].indent = (int16_t)indentWidth;

				posStr[chr++] = '.';
				posStr[chr++] = ' ';
				posStr[chr] = 0;

				err = DbGetColumnValue(gMemoDB, gListViewCursorID, kMemoDBColumnID, 0, (void**)&buffer, &numChars);
				if (err == dmErrOneOrMoreFailed || !buffer)
				{
					DbWriteColumnValue(gMemoDB, gListViewCursorID, kMemoDBColumnID, 0, -1, "", 1);
					DbGetColumnValue(gMemoDB, gListViewCursorID, kMemoDBColumnID, 0, (void**)&buffer, &numChars);
				}


				bufferP = StrChr(buffer, linefeedChr);
				if (bufferP) numChars = bufferP - buffer;
				else numChars--;

				len = FntWidthToOffset (buffer, numChars, (uint16_t)(width - ellipsisWidth - gMemolistCache[offset].indent - FntLineWidth(posStr,chr)), NULL, NULL);

				if (gMemolistCache[offset].titleAlloc < (chr+len+2))
				{
					gMemolistCache[offset].titleAlloc = chr+len+2;

					if (gMemolistCache[offset].title)
							MemPtrFree(gMemolistCache[offset].title);

					gMemolistCache[offset].title = MemPtrNew((uint32_t)(chr + len + 2));
					ErrFatalDisplayIf(!gMemolistCache[offset].title, "couldn't allocate title for cached row");
				}

				ErrFatalDisplayIf(!gMemolistCache[offset].title, "non allocated title for cached row");
				p = gMemolistCache[offset].title;

				strcpy(p,posStr);
				p += chr;
				strncpy(p,buffer,len);
				p += len;

				if (len != numChars)
					*p++ = chrEllipsis;

				*p++ = 0;

				DbReleaseStorage(gMemoDB,buffer);
			}
			else
				*gMemolistCache[offset].title = '\x0';
		}

		attr = gMemolistCache[offset].attr;

		if (row < numRows) // keep on loading cache only.
		{
			// If the record was found, store the record number in the table item,
			// otherwise set the table row unusable.
			if (err >= errNone && !(attr & dbRecAttrDelete) && (tableHeight >= dataHeight + lineHeight))
			{
				TblSetRowData(table, row, sTopRowPositionInCategory + row);
				TblSetItemStyle(table, row, 0, customTableItem);
				TblSetItemFont(table, row, 0, gListFont);

				TblSetRowHeight (table, row, lineHeight);

				TblSetRowUsable (table, row, true);

				// Mark the row invalid so that it will draw when we call the
				// draw routine.
				TblMarkRowInvalid (table, row);

				dataHeight += lineHeight;
			}
			else
			{
				// Set the row height - when scrolling winDown, the heights of the last rows of
				// the table are used to determine how far to scroll.  As rows are deleted
				// from the top of the table, formerly unused rows scroll into view, and the
				// height is used before the next call to ListViewLoadTable (which would set
				// the height correctly).
				TblSetRowHeight (table, row, lineHeight);

				TblSetRowUsable (table, row, false);
			}
		}
		row++;

		// Get the next record in the current category.
		err = DbCursorMoveNext(gListViewCursorID);
	}
	FntSetFont (currFont);

	// If we hit the end of the cursor before filling all rows, fill them in as blank here
	for (; row < numRows; row++) {
		TblSetRowHeight(table, row, lineHeight);
		TblSetRowUsable(table, row, false);
	}

	// Select current record
	if (CurrentRecordIndex != noRecordSelected
	&& TblFindRowData(table, CurrentRecordIndex, &row))
	{
		gSelectedRow = row;
		TblSelectItem (table, gSelectedRow, 0);
	}

	// Update the scroll arrows.
	ListViewUpdateScrollers (formP);

	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:    ListViewLoadRecords
 *
 * DESCRIPTION: This routine loads memo database records into
 *              the list view form.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/16/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewLoadRecords(FormType* formP, Boolean updateCategoryTrigger)
{
	ControlPtr	ctl;
	TableType *	tableP;
	uint32_t	CurrentRecordIndex = noRecordSelected;
	uint16_t	rowsInTable, i;
	uint32_t	memosInCategory = 0;
	status_t	err;

	tableP = FrmGetObjectPtr(formP, FrmGetObjectIndex (formP, ListTable));
	rowsInTable = ListViewNumberOfRows(tableP);

	for (i=0;i<gMemolistCacheSize;i++) gMemolistCache[i].cursorPos = 0;

	// Set the label of the category trigger.
	if (updateCategoryTrigger)
	{
		ctl = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, ListCategoryTrigger));
		CatMgrSetTriggerLabel(gMemoDB, gCurrentCategoriesP, 1, ctl, gCategoryName);
	}

	err = MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;

	DbgOnlyFatalErrorIf(err, "Could not open cursor for the MemoList");

	if (gCurrentRecord != noRecordSelected)
	{
		DbCursorGetPositionForRowID(gListViewCursorID, gCurrentRecord, &CurrentRecordIndex);

		// Make the current record the first visible record?
		if (gTopVisibleRecord + (uint32_t)(rowsInTable - 1) < CurrentRecordIndex || CurrentRecordIndex < gTopVisibleRecord)
			gTopVisibleRecord = CurrentRecordIndex;
	}

	// Make sure we show a full display of records.
	memosInCategory = MemoDBNumVisibleRecordsInCategory(gListViewCursorID);

	if (memosInCategory)
	{
		if (memosInCategory > rowsInTable)
			gTopVisibleRecord = min(gTopVisibleRecord, memosInCategory - (uint32_t)(rowsInTable - 1));
		else
			gTopVisibleRecord = 1;
	}
	else
	{
		gSelectedRow = -1;
		gTopVisibleRecord = 1;
	}

	ListViewLoadTable(formP);

	// Set the callback routine that will draw the records.
	TblSetCustomDrawProcedure(tableP, 0, ListViewDrawRecord);

	TblSetColumnUsable(tableP, 0, true);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    The index of the new category.
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							ShowAllCategories
 *							gCategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static void ListViewSelectCategory(void)
{
	FormType *		formP;
	Boolean			categoryChanged;
	CategoryID *	categoriesP;
	uint32_t		numCategories;

	formP = FrmGetFormPtr(ListView);

	// Process the category popup list.
	categoryChanged = CatMgrSelectFilter(gMemoDB, formP, ListCategoryTrigger, gCategoryName, ListCategoryList,
							gCurrentCategoriesP, gCurrentNumCategories, &categoriesP, &numCategories, true, NULL);

	if (!categoryChanged)
		return;

	ChangeCategory(categoriesP, numCategories);
	CatMgrFreeSelectedCategories(gMemoDB, &categoriesP);

	// Display the new category.
	ListViewLoadRecords(formP, true);
	UtilsFrmInvalidateWindow(ListView);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewNextCategory
 *
 * DESCRIPTION: This routine display the next category,  if the last
 *              catagory isn't being displayed
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							ShowAllCategories
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/15/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewNextCategory (void)
{
	CategoryID 	category = catIDAll;
	FormType* 	formP;
	ControlPtr 	ctl;

	if (gCurrentCategoriesP)
		CatMgrGetNext (gMemoDB, gCurrentCategoriesP[0], &category);

	if (!gCurrentCategoriesP || category != gCurrentCategoriesP[0])
	{
		ChangeCategory (&category,1);

		// Set the label of the category trigger.
		ctl = GetObjectPtr (ListCategoryTrigger, ListView);
		CatMgrSetTriggerLabel(gMemoDB, gCurrentCategoriesP, 1, ctl, gCategoryName);

		// Display the new category.
		gTopVisibleRecord = 1;
		formP = FrmGetFormPtr(ListView);
		ListViewLoadRecords(formP, true);
		UtilsFrmInvalidateWindow(ListView);
	}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the list of of memo titles
 *              in the direction specified.
 *
 * PARAMETERS:  direction - winUp or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			roger 7/27/95	Copied fixed code from Address Book
 *
 ***********************************************************************/
static void ListViewPageScroll(WinDirectionType direction)
{
	TablePtr 		table;
	int16_t 		linesToScroll;

	table = GetObjectPtr (ListTable, ListView);

	if (direction == winDown)
		linesToScroll = ListViewNumberOfRows(table) - 1;
	else
		linesToScroll = -(ListViewNumberOfRows(table) - 1);

	if (linesToScroll)
		gSelectedRow = -1;

	ListViewScroll(linesToScroll);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewScroll
 *
 * DESCRIPTION: This routine scrolls the list of of memo titles
 *              in the direction specified.
 *
 * PARAMETERS:  direction - winUp or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			roger 7/27/95	Copied fixed code from Address Book
 *
 ***********************************************************************/
static Boolean ListViewScroll(int16_t linesToScroll)
{
	uint16_t			rows;
	uint32_t 			newTopVisibleRecord;
	TablePtr 			table;
	RectangleType		rect;
	int16_t				seekOffset;
	int16_t				seekDirection;
	uint32_t			numRecords;
	Boolean				forceRedraw = false;

	if (!linesToScroll)
		return false;

    table = GetObjectPtr (ListTable, ListView);
	rows = ListViewNumberOfRows (table);
	TblGetBounds (table, &rect);

	numRecords = MemoDBNumVisibleRecordsInCategory(gListViewCursorID);

	// Don't scroll if we're already at top or bottom
	if ((linesToScroll > 0 && gTopVisibleRecord + rows > numRecords) ||
		(linesToScroll < 0 && gTopVisibleRecord==1))
		return false;

	gCurrentRecord = noRecordSelected;
	gSelectedRow = -1;
	TblSelectItem (table, -1, 0);

	// Find the new top visible record
	newTopVisibleRecord = gTopVisibleRecord;

	// Scroll down.
	if (linesToScroll > 0)
	{
		seekOffset = linesToScroll;
		seekDirection = dmSeekForward;
	}
	else // linesToScroll < 0
	{
		seekOffset = -linesToScroll;
		seekDirection = dmSeekBackward;
		if (newTopVisibleRecord <= (uint32_t)seekOffset)
		{
			forceRedraw = true;
			seekOffset = 0;
			newTopVisibleRecord = 1;
		}
	}

	if (!SeekRecord(&newTopVisibleRecord, seekOffset, seekDirection))
		return false;

	// Make sure we show a full display of records.
	if (linesToScroll > 0 && newTopVisibleRecord + rows > numRecords)
	{
		forceRedraw = true;
		newTopVisibleRecord = numRecords ? min(newTopVisibleRecord, numRecords - (uint32_t)(rows - 1)) : 1;
	}

	gTopVisibleRecord = newTopVisibleRecord;

	// nudge the cache offset so that the overlap between the previously displayed records
	// and the newly displayed records will be at the right location, so that only the newly
	// displayed records will need to be loaded
	gMemolistCacheOffset = (gMemolistCacheOffset + linesToScroll) % gMemolistCacheSize;
	if (gMemolistCacheOffset < 0) gMemolistCacheOffset += gMemolistCacheSize;

	ListViewLoadTable(FrmGetFormPtr(ListView));
	TblMarkTableInvalid(table);

	// Force a redraw of the whole table.  The scroll bar is now on top
	// of the table.  We introduce artifacts into the scroll bar when we
	// try to use winscroll... to scroll the table.  Invalidating the table
	// with transparent mode on and both the table and the scroll bar will
	// be drawn correctly.
	TblInvalidate(table);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewResize
 *
 * DESCRIPTION: This routine resize the list view fomr and its objects
 *				when the input area appears or disappears
 *
 *
 * PARAMETERS:	->	formP - ListView form ptr
 *				->	newBoundsP - new form bounds
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		1/18/02		Initial Revision
 *			PPL		05/04/04	Finalize Landscape mode.
 *
 **********************************************************************/
void ListViewResize(FormType *formP, RectangleType *newBoundsP)
{
	uint16_t		objIndex;
	Coord			offsetX;
	Coord			offsetY;
	Coord			x;
	Coord			y;
	RectangleType	rect;
	FontID			currFont;
	Coord			width;
	Coord			height;
	Coord			buttonY;
	Coord			tableHeight;
	TablePtr		tableP;

	// get the new window extent
	width = newBoundsP->extent.x;
	height = newBoundsP->extent.y;

	// get the delta between the old window bounding rect and the new display extent.
	offsetY = height - gCurrentWinBounds.extent.y;
	offsetX = width  - gCurrentWinBounds.extent.x;

	if (!offsetX && !offsetY)
		return ;

	FrmClearFocusHighlight();
	gCurrentRecord = noRecordSelected;
	gSelectedRow = -1;

	// The button (can't filter on offsetY as we need buttonY)
	objIndex = FrmGetObjectIndex(formP, ListNewButton);
	FrmGetObjectPosition (formP, objIndex, &x, &y);
	y += offsetY;
	buttonY = y;
	FrmSetObjectPosition (formP, objIndex, x, y);


	// The table is resized on both x and y
	objIndex = FrmGetObjectIndex(formP, ListTable);
	tableP = FrmGetObjectPtr(formP, objIndex);
	TblSelectItem (tableP, gSelectedRow, 0);

	FrmGetObjectBounds(formP, objIndex, &rect);

	// Stick the table bottom to the 'New' button.
	rect.extent.y = buttonY - rect.topLeft.y;

	// Remove the last incomplete line
	currFont = FntSetFont (gListFont);
	rect.extent.y -= (rect.extent.y % FntLineHeight());
	rect.extent.x += offsetX;
	FntSetFont (currFont);
	tableHeight = rect.extent.y;
	FrmSetObjectBounds(formP, objIndex, &rect);

	// The scrollbar, moves on X, resizes on y
	objIndex = FrmGetObjectIndex(formP, ListScrollBar);

	FrmGetObjectPosition (formP, objIndex, &x, &y);
	x += offsetX;
	FrmSetObjectPosition (formP, objIndex, x, y);

	FrmGetObjectBounds(formP, objIndex, &rect);
	rect.extent.y = tableHeight;
	FrmSetObjectBounds(formP, objIndex, &rect);

	if (offsetX)
	{
		// The ListCategoryTrigger, moves on X
		objIndex = FrmGetObjectIndex(formP, ListCategoryTrigger);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (formP, objIndex, x, y);

		// The ListCategoryList, moves on X
		objIndex = FrmGetObjectIndex(formP, ListCategoryList);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (formP, objIndex, x, y);

		// Apply horizontal offset on X
		TblSetColumnWidth(tableP, 0, TblGetColumnWidth(tableP, 0) + offsetX);
	}

	// keep the window bounding rect.
	gCurrentWinBounds = *newBoundsP;
}

/***********************************************************************
 *
 * FUNCTION:    ListViewInit
 *
 * DESCRIPTION: This routine initializes the "List View" of the
 *              Memo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewInit(FormType* formP)
{
	gCurrentView = ListView;
	gForceReload = false ;

	// Make the form transparent so that the table can show through the
	// scroll bar.
	FrmSetTransparentObjects( formP, TRUE );

	MemSet(&gListViewSelection, sizeof(gListViewSelection), 0) ;

	if (gCurrentNumCategories > 1)
		gCurrentNumCategories = 1;

	ListViewLoadRecords(formP, true);

//	FrmSetNavEntry(formP, ListTable, ListCategoryTrigger, ListCategoryTrigger, ListNewButton, 0);

	sOneHandedTableHasFocus = false;
	sOneHandedRecordSelected = false;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectTableItem
 *
 * DESCRIPTION: This routine either selects or unselects the specified
 *					 table item.
 *
 * PARAMETERS:	 selected - specifies whether an item should be selected or
 *									unselected
 *				 table	 - pointer to a table object
 *              row      - row of the item (zero based)
 *              rP       - pointer to a structure that will hold the bound
 *                         of the item
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			jmp	10/29/99	Initial Revision
 *			jmp	11/12/99	While a table item is "on the move," having it be selected
 *								can cause the Table code grief.  So, instead of using
 *								the TblSelectItem()/TblUnhighlightSelect() calls, we now
 *								manually select/unselect the table's row.  Before color,
 *								only WinInvertRectangle() was called, so this is now in line
 *								again with the way things used to work.  Sigh.
 *
 ***********************************************************************/
static void ListViewSelectTableItem (Boolean selected, TablePtr table,
	int16_t row, RectangleType *r)
{
	// Get the item's rectangle.
	//
	TblGetItemBounds (table, row, 0, r);

	// Set up the drawing state the way we want it.
	//
	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

	if (ProcessingUpdate)
	{
		// Erase and (re)draw the item.
		//
		WinEraseRectangle(r, 0);
		ListViewDrawRecord(table, row, 0, r);
	}

	gSelectedRow = row;
	TblSelectItem (table, gSelectedRow, 0);

	// Restore the previous drawing state.
	//
	WinPopDrawState();
}

/***********************************************************************
 *
 * FUNCTION:    ListViewGoToSelectedMemo
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		01/23/02	Initial Revision
 *
 ***********************************************************************/
static void ListViewGoToSelectedMemo(int16_t row)
{
	uint16_t 	attr;
	TablePtr	tableP;
	Boolean		viewRecord;
	FormPtr     formP;

	gSelectedRow = row;
	tableP = GetObjectPtr (ListTable, ListView);
	DbCursorSetAbsolutePosition(gListViewCursorID, TblGetRowData(tableP, row));
	DbCursorGetCurrentRowID(gListViewCursorID, &gCurrentRecord);
	gEditScrollPosition = 0;

	// Get the category and secret attribute of the current record.
	DbGetRowAttr(gMemoDB, gCurrentRecord, &attr);

	// If this is a "private" record, then determine what is to be shown.
	if (attr & dbRecAttrSecret)
	{
		switch (gPrivateRecordVisualStatus)
		{
		case showPrivateRecords:
			FrmGotoForm(gApplicationDbP, EditView);
			break;

		case maskPrivateRecords:
			// the DB must be closed so that private records can be removed if the user
			// lost his password.
			DbCursorClose(gListViewCursorID);
			gListViewCursorID = dbInvalidCursorID;
			DbCloseDatabase(gMemoDB);
			gMemoDB = NULL;

			gCheckingPassword = true;
			viewRecord = SecVerifyPW(showPrivateRecords);
			gCheckingPassword = false;

			if (!gMemoDB)
				gMemoDB = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			ErrFatalDisplayIf(!gMemoDB, "Can't reopen DB");

			// Reopen cursor and re-position on the right record
			if (gListViewCursorID == dbInvalidCursorID)
				MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
			DbCursorMoveToRowID(gListViewCursorID, gCurrentRecord) ;

			if (viewRecord)
			{
				// We only want to unmask this one record, so restore the preference.
				PrefSetPreference(prefShowPrivateRecords, maskPrivateRecords);

				FrmGotoForm(gApplicationDbP, EditView);
			}
			else
			{
				//For safety, simply reset the gCurrentRecord
				gCurrentRecord = noRecordSelected;
				formP = FrmGetFormPtr(ListView);
				ListViewLoadRecords(formP, true);
				UtilsFrmInvalidateWindow(ListView);
			}
			break;

			// This case should never be executed!!!!!!!
		case hidePrivateRecords:
		default:
			break;
		}
	}
	else
	{
		FrmGotoForm(gApplicationDbP, EditView);
	}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewDrawMoveIndicator
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
static void ListViewDrawMoveIndicator (void)
{
	CustomPatternType 	pattern, savedPattern;
	uint16_t 			i;
	TablePtr			tableP = GetObjectPtr(ListTable, ListView);
	RectangleType		rect;

	TraceOutput(TL(appErrorClass, ">>>>>>>>>>> DRAW MOVE INDICATOR row= %d SELECTED = %d",
		gListViewSelection.row, gListViewSelection.selectedRow));

	if (gListViewSelection.selectedRow != -1)
		TblMarkRowInvalid(tableP, gListViewSelection.selectedRow);

	// Redraw the table
	TblRedrawTable(tableP);

	// Redraw the selection
	if (gListViewSelection.selectedRow != -1)
		ListViewSelectTableItem(gListViewSelection.selected, tableP, gListViewSelection.selectedRow, &rect);

	// Draw the insersion bar
	WinGetPattern (&savedPattern);
	for (i = 0; i < sizeof (CustomPatternType) / sizeof (*pattern); i++)
		pattern[i] = 0x55;
	WinSetPattern(&pattern);
	WinFillRectangle (&gListViewSelection.indicatorR, 0);

	// Restore old pattern
	WinSetPattern (&savedPattern);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewInvertMoveIndicator
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	 itemR - bounds of the move indicator
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
static void ListViewSetMoveIndicator (int16_t row, RectanglePtr itemR)
{
	gListViewSelection.indicatorR.topLeft.x = itemR->topLeft.x;
	gListViewSelection.indicatorR.topLeft.y = itemR->topLeft.y + itemR->extent.y - 2;
	gListViewSelection.indicatorR.extent.x = itemR->extent.x;
	gListViewSelection.indicatorR.extent.y = 2;

	if (row == -1)
		gListViewSelection.indicatorR.topLeft.y += 2;
}

/***********************************************************************
 *
 * FUNCTION:    ListViewStartDragging
 *
 * DESCRIPTION: This routine tracks a Memo item for either selection
 *					 to go to EditView, or movement in the ListView.
 *
 *
 * PARAMETERS:	 event
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
static void ListViewStartDragging(EventType * event)
{
	Boolean penDown;

	// Initial call after a tblEnterEvent
	MemSet(&gListViewSelection, sizeof(gListViewSelection), 0) ;
	gListViewSelection.selected = true ;
	gListViewSelection.row		= event->data.tblSelect.row;
	gListViewSelection.selectedRow = event->data.tblSelect.row;
	gListViewSelection.prevMoveRow = event->data.tblSelect.row;

	// Clear focus highlight
	FrmClearFocusHighlight();

	// Highlight the item the pen when winDown on.
	DbCursorSetAbsolutePosition(gListViewCursorID, TblGetRowData(event->data.tblSelect.pTable, gListViewSelection.row));
	DbCursorGetCurrentRowID(gListViewCursorID, &(gListViewSelection.selectedRecord));
	ListViewSelectTableItem(gListViewSelection.selected, event->data.tblSelect.pTable,
		gListViewSelection.selectedRow, &gListViewSelection.r);

	gCurrentRecord = gListViewSelection.selectedRecord;

	EvtGetPen(&gListViewSelection.x, &gListViewSelection.y, &penDown);
	if (penDown)
	{
		// Pen down dragging handled at penMoveEvent until we get a penUpEvent
		gListViewSelection.dragging = true ;
	}

	// Set the event loop wait time to get nil events event if we don't move
	// during drag: this is needed if we move the pen outside (up or down) the
	// table and the table still needs to scroll.
	gEventLoopWaitTime = scrollTimeOutWhileDragging;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewMoveDragging
 *
 * DESCRIPTION: Handles Memo dragging for manual sort.
 *
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *		02/04/03	LYr	Extracted from ListViewSelectMemo.
 *
 ***********************************************************************/
void ListViewMoveDragging(Coord x, Coord y)
{
	uint32_t 	index;
	WinHandle	winH;
	TablePtr	tableP = GetObjectPtr(ListTable, ListView);
	RectangleType rect;
	Boolean		scrolled = false;

	gListViewSelection.x = x;
	gListViewSelection.y = y;

	winH = FrmGetWindowHandle(FrmGetFormPtr(ListView));

	if (! gListViewSelection.moving)
	{
		// Is the pen still within the bounds of the item it went winDown on,
		// if not draw the move indicator.
		if (! RctPtInRectangle (gListViewSelection.x, gListViewSelection.y, &gListViewSelection.r))
		{
			gListViewSelection.moving = true;
			TblGetItemBounds (tableP, gListViewSelection.row, 0, &gListViewSelection.r);
			ListViewSetMoveIndicator(gListViewSelection.row, &gListViewSelection.r);
		}
	}

	else if (! RctPtInRectangle (gListViewSelection.x, gListViewSelection.y, &gListViewSelection.r))
	{
		// Save previous selection position
		gListViewSelection.prevMoveRow = gListViewSelection.row;

		// Above the first item ?
		if (gListViewSelection.row < 0)
		{
			TraceOutput(TL(appErrorClass, "ListViewDragMemo above TOP"));
			if (gListViewSelection.y >= gListViewSelection.r.topLeft.y)
			{
				(gListViewSelection.row)++;
				gListViewSelection.r.topLeft.y += gListViewSelection.r.extent.y;
			}
		}

		// Move winUp.
		else if (gListViewSelection.y < gListViewSelection.r.topLeft.y)
		{
			TraceOutput(TL(appErrorClass, "ListViewDragMemo UP"));
			index = TblGetRowData(tableP, gListViewSelection.row);
			DbCursorGetRowIDForPosition(gListViewCursorID, index, &(gListViewSelection.recordNum));
			if (SeekRecord(&index, 1, dmSeekBackward))
			{
				if (gListViewSelection.row)
					(gListViewSelection.row)--;
				else
				{
					ListViewScroll (-1);
					scrolled = true;
				}
				TblGetItemBounds (tableP, gListViewSelection.row, 0, &gListViewSelection.r);
			}
			else if (gListViewSelection.row == 0)
			{
				(gListViewSelection.row)--;
				gListViewSelection.r.topLeft.y -= gListViewSelection.r.extent.y;
			}
		}

		// Move winDown
		else
		{
			TraceOutput(TL(appErrorClass, "ListViewDragMemo DOWN"));
			index = TblGetRowData(tableP, gListViewSelection.row);
			DbCursorGetRowIDForPosition(gListViewCursorID, index, &(gListViewSelection.recordNum));

			if (SeekRecord(&index, 0, dmSeekForward))
			{
				if (gListViewSelection.row < TblGetLastUsableRow (tableP))
					(gListViewSelection.row)++;
				else
				{
					ListViewScroll (1);
					scrolled = true;
				}
				TblGetItemBounds (tableP, gListViewSelection.row, 0, &gListViewSelection.r);
			}
		}

		// Get the position of the initialy selected row
		if (scrolled)
		{
			if (! TblFindRowData (tableP, gListViewSelection.selectedRecord, &gListViewSelection.selectedRow))
				gListViewSelection.selectedRow = -1;
		}

		ListViewSetMoveIndicator(gListViewSelection.row, &gListViewSelection.r);
		WinInvalidateRect(winH, &gListViewSelection.r);

		// Specific case for first row (because uppest bar is also on this row)
		if (gListViewSelection.row == 0)
		{
			TblMarkRowInvalid(tableP, 0);
			TblGetItemBounds (tableP, 0, 0, &rect);
			WinInvalidateRect(winH, &rect);
		}

		// Test if we skipped one row while moving
		if (int_abs(gListViewSelection.prevMoveRow - gListViewSelection.row) > 1)
		{
			TblMarkTableInvalid(tableP);
			TblGetBounds(tableP, &rect);
			WinInvalidateRect(winH, &rect);
		}
		else
		{
			TblMarkRowInvalid(tableP, gListViewSelection.prevMoveRow);
			TblGetItemBounds (tableP, gListViewSelection.prevMoveRow, 0, &rect);
			WinInvalidateRect(winH, &rect);
		}

	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewStopDragging
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	 event
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
static void ListViewStopDragging(void)
{
	uint32_t 	recPos, selPos ;
	TablePtr	tableP = GetObjectPtr(ListTable, ListView);

	gListViewSelection.dragging = false ;

	// If the highlighted item is visible, unhighlight it.
	if (TblFindRowData (tableP, gListViewSelection.selectedRecord, &gListViewSelection.selectedRow))
		ListViewSelectTableItem (false, tableP, gListViewSelection.selectedRow, &gListViewSelection.r);

	if (gListViewSelection.moving)
	{
		DbCursorGetPositionForRowID(gListViewCursorID,gListViewSelection.selectedRecord, &selPos);

		if (gListViewSelection.row >= 0)
		{
			recPos = TblGetRowData(tableP, gListViewSelection.row);
			DbCursorGetRowIDForPosition(gListViewCursorID, recPos, &(gListViewSelection.recordNum));
			if (gListViewSelection.selectedRecord == gListViewSelection.recordNum)
			{
				TblMarkRowInvalid(tableP, gListViewSelection.selectedRow);
				//TblRedrawTable(tableP);
				TblInvalidate(tableP);
				return;
			}

			if (selPos > recPos)
				recPos++;

		}
		else
		{
			recPos = TblGetRowData(tableP, 0);
			DbCursorGetRowIDForPosition(gListViewCursorID, recPos, &(gListViewSelection.recordNum));
		}

		TraceOutput(TL(appErrorClass, "ListViewSelectMemo %d (%d) => %d (%d)",gListViewSelection.selectedRecord,selPos,gListViewSelection.recordNum,recPos));
		DbCursorRelocateRow(gListViewCursorID, selPos, recPos);
		ListViewLoadRecords(FrmGetFormPtr(ListView), true);

		// Redraw the full window
		UtilsFrmInvalidateWindow(ListView);
	}

	// If we didn't move the item then it's been selected for editing, go to the
	// edit view.
	else if (gSortAlphabetically || gListViewSelection.selected)
	{
		ListViewGoToSelectedMemo(gListViewSelection.row);
	}

	// Come back to common event loop
	gEventLoopWaitTime = evtWaitForever;

}

/***********************************************************************
 *
 * FUNCTION:    ListViewDrawTableItemFocusRing
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
static void ListViewDrawTableItemFocusRing(FormType* formP, TableType *tableP, int16_t tableRowIndex)
{
	RectangleType	bounds;

	if (!sOneHandedTableHasFocus || !sOneHandedRecordSelected)
		return;

	FrmClearFocusHighlight();
	TblGetItemBounds(tableP, tableRowIndex, 0, &bounds);
	FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List View"
 *              of the Memo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * HISTORY:
 *		02/21/95	art	Created by Art Lamb.
 *		11/22/98	kwk	Handle command keys in separate code block so that
 *							TxtCharIsPrint doesn't get called w/virtual chars.
 *		09/25/99	kwk	Use TxtGlueUpperChar to capitalize initial char for
 *							memo that's autocreated by writing a printable char.
 *
 ***********************************************************************/
Boolean ListViewHandleEvent(EventPtr eventP)
{
	FormType	*formP;
	TableType	*tableP;
	RectangleType rect;
	Boolean		handled = false;
	Boolean		repeat;
	Boolean		scrolled;
	uint32_t	numLibs;
	uint32_t	recordID;
	DmOpenRef	uiLibDBP = NULL;
	uint32_t	uiLibRefNum = 0;
	size_t		length;
	FrmNavStateFlagsType	navState;

	switch (eventP->eType)
	{
		case frmObjectFocusTakeEvent:
			if (eventP->data.frmObjectFocusTake.objectID == ListTable)
			{
				uint16_t		objIndex;

				sOneHandedTableHasFocus = true;

				if (!sOneHandedRecordSelected)
				{
					formP = FrmGetFormPtr(ListView);
					objIndex = FrmGetObjectIndex(formP, ListTable);
					FrmSetFocus(formP, objIndex);
					FrmGetObjectBounds(formP, objIndex, &rect);
					FrmSetFocusHighlight(FrmGetWindowHandle(formP), &rect, 0);
				}
				handled = true;
			}
			else
				sOneHandedTableHasFocus = false;
			break;

		case frmObjectFocusLostEvent:
			if (eventP->data.frmObjectFocusLost.objectID == ListTable)
			{
				sOneHandedTableHasFocus = false;

				if (sOneHandedRecordSelected)
				{
					formP = FrmGetFormPtr(ListView);
					sOneHandedRecordSelected = false;
					WinInvalidateWindow(FrmGetWindowHandle(formP));
				}
			}
			break;



		case keyDownEvent:
			 // Memo button pressed?
			if (TxtCharIsHardKey(eventP->data.keyDown.modifiers, eventP->data.keyDown.chr))
			{
				if (!(eventP->data.keyDown.modifiers & poweredOnKeyMask))
					ListViewNextCategory ();
				handled = true;
			}
			else if (EvtKeydownIsVirtual(eventP))
			{
				formP = FrmGetFormPtr(ListView);
				tableP = GetObjectPtr (ListTable, ListView);
				repeat = eventP->data.keyDown.modifiers & autoRepeatKeyMask;

				// Scroll up key pressed? Or Rocker up with no line focus and enough records to scroll up ?
				if ((eventP->data.keyDown.chr == vchrPageUp && (KeyCurrentState() & keyBitPageUp))
				|| (!sOneHandedRecordSelected && sOneHandedTableHasFocus && gTopVisibleRecord > 1 && (eventP->data.keyDown.chr == vchrRockerUp || eventP->data.keyDown.chr == vchrThumbWheelUp)))
				{
					ListViewPageScroll (winUp);
					handled = true;
				}

				// Scroll down key pressed? Or Rocker down with no line focus and enough records to scroll down ?
				else if ((eventP->data.keyDown.chr == vchrPageDown && (KeyCurrentState() & keyBitPageDown))
				|| (!sOneHandedRecordSelected && sOneHandedTableHasFocus && gTopVisibleRecord + ListViewNumberOfRows(tableP) < MemoDBNumVisibleRecordsInCategory(gListViewCursorID) && (eventP->data.keyDown.chr == vchrRockerDown || eventP->data.keyDown.chr == vchrThumbWheelDown)))
				{
					ListViewPageScroll (winDown);
					handled = true;
				}

				else if (sOneHandedTableHasFocus && sOneHandedRecordSelected && (eventP->data.keyDown.chr == vchrRockerUp || eventP->data.keyDown.chr == vchrThumbWheelUp))
				{
					if (gSelectedRow >= 0)
					{
						if (gSelectedRow == 0)
						{
							scrolled = ListViewScroll(-1);
							if (scrolled && gSelectedRow < 0)
								gSelectedRow = 0;
						}
						else
						{
							gSelectedRow--;
							scrolled = true;
						}

						if (scrolled)
						{
							ListViewSelectTableItem(true, tableP, gSelectedRow, &rect);
							ListViewDrawTableItemFocusRing(formP, tableP, gSelectedRow);
						}

						handled = scrolled || repeat;
					}
				}

				else if (sOneHandedTableHasFocus && sOneHandedRecordSelected && (eventP->data.keyDown.chr == vchrRockerDown || eventP->data.keyDown.chr == vchrThumbWheelDown))
				{
					if (gSelectedRow >= 0)
					{
						if (gSelectedRow == TblGetLastUsableRow(tableP))
						{
							scrolled = ListViewScroll(1);
							if (scrolled && gSelectedRow < 0)
								gSelectedRow = TblGetLastUsableRow(tableP);
						}
						else
						{
							gSelectedRow++;
							scrolled = true;
						}

						if (scrolled)
						{
							ListViewSelectTableItem(true, tableP, gSelectedRow, &rect);
							ListViewDrawTableItemFocusRing(formP, tableP, gSelectedRow);
						}

						handled = scrolled || repeat;
					}
				}
				else if (sOneHandedTableHasFocus && sOneHandedRecordSelected && eventP->data.keyDown.chr == vchrRockerLeft)
				{
					sOneHandedRecordSelected = false;
					// Give the focus back to the whole table.
					FrmSetFocus(formP, FrmGetObjectIndex(formP, ListTable));
					FrmNavObjectTakeFocus(formP, ListTable);
					handled = true;
				}
				else if (eventP->data.keyDown.chr == vchrRockerCenter || eventP->data.keyDown.chr == vchrThumbWheelPush)
				{
					if (sOneHandedTableHasFocus)
					{
						if (MemoDBNumVisibleRecordsInCategory(gListViewCursorID) == 0)
						{	// create a new record if list view is empty
							if (MemoDBCreateRecord(gMemoDB, gListViewCursorID, &gCurrentRecord, gCurrentNumCategories, gCurrentCategoriesP) == errNone)
								FrmGotoForm (gApplicationDbP, EditView);
							else
								gCurrentRecord = noRecordSelected;
						}
						else
						{
							if (sOneHandedRecordSelected && gSelectedRow >= 0)
								ListViewGoToSelectedMemo(gSelectedRow);
							else
							{
								int16_t	row = (gSelectedRow >= 0) ? gSelectedRow : 0;

								ListViewSelectTableItem(true, tableP, row, &rect);
								sOneHandedRecordSelected = true;
								ListViewDrawTableItemFocusRing(formP, tableP, row);
							}
						}
						handled = true;
					}
					else
					{
						FrmGetNavState(formP, &navState);

						// If we are in Interaction mode, explicilty give the focus to the table.
						if ((navState & kFrmNavStateFlagsObjectFocusMode) == 0)
						{
							FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
							FrmNavObjectTakeFocus(formP, ListTable);
							WinInvalidateWindow(FrmGetWindowHandle(formP));
							handled = true;
						}
					}
				}

				// Send Data key presed?
				else if (eventP->data.keyDown.chr == vchrSendData)
				{
					handled = ListViewDoCommand(ListRecordBeamCategoryCmd);
				}
			}
			// If printable character, create a new record.
			else if (TxtCharIsPrint(eventP->data.keyDown.chr))
			{
				TraceOutput(TL(appErrorClass, "Fake evtQueue (ListView): Enqueue char: %c", (char)eventP->data.keyDown.chr));

				if (!gEnqueueNewChar && MemoDBCreateRecord(gMemoDB, gListViewCursorID, &recordID, gCurrentNumCategories, gCurrentCategoriesP) >= errNone)
				{
					char buffer[maxCharBytes];

					gCurrentRecord = recordID;
					FrmGotoForm (gApplicationDbP, EditView);

					// Uppercase the character.
					length = TxtSetNextChar (buffer, 0, eventP->data.keyDown.chr);
					if (TxtTransliterate (buffer, length, buffer, &length, translitOpUpperCase) == 0)
						TxtGetNextChar (buffer, 0, &eventP->data.keyDown.chr);

					// Buffer the event to re-enqueue in the edit form.
					MemMove(&gNewCharBuffer[0], eventP, sizeof(EventType));
					gNewCharBufferCount = 1;
					gEnqueueNewChar = true;
				}

				handled = true;
			}
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case ListNewButton:
					if (MemoDBCreateRecord(gMemoDB, gListViewCursorID, &recordID, gCurrentNumCategories, gCurrentCategoriesP) == errNone)
					{
						gCurrentRecord = recordID;
						FrmGotoForm (gApplicationDbP, EditView);
					}
					handled = true;
					break;

				case ListCategoryTrigger:
					ListViewSelectCategory();
					handled = true;
					break;
			}
			break;

		case tblEnterEvent:
//			gTableNavFocus = true;
			if (!gSortAlphabetically)
			{
				ListViewStartDragging (eventP);
				handled = true;
			}
			break;

		case tblSelectEvent:
			ListViewGoToSelectedMemo(eventP->data.tblSelect.row);
			handled = true;
			break;

		case menuOpenEvent:
			if (! gAttachRequest)
			{
				if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
					MenuHideItem(ListRecordSendCategoryCmd);
				else
					MenuShowItem(ListRecordSendCategoryCmd);
			}
			else
			{
				// Hide send & Beam commands
				MenuHideItem(ListRecordSendCategoryCmd);
				MenuHideItem(ListRecordBeamCategoryCmd);

				// Show attach command
				MenuShowItem(ListRecordAttachCategoryCmd);
			}
			// don't set handled = true
			break;

		case menuEvent:
			handled = ListViewDoCommand (eventP->data.menu.itemID);
			break;

		case frmOpenEvent:
			formP = FrmGetFormPtr(ListView);
			ListViewInit(formP);
			handled = true;
			break;

		case winResizedEvent:
			formP = FrmGetFormPtr(ListView);
			if (eventP->data.winResized.window != FrmGetWindowHandle(formP))
				break;
			ListViewResize(formP, &eventP->data.winResized.newBounds);
			ListViewLoadRecords(formP, false);
			handled = true;
			break;

		case winUpdateEvent:
			formP = FrmGetFormPtr(ListView);
			if (eventP->data.winUpdate.window != FrmGetWindowHandle(formP))
				break;
			ProcessingUpdate = true;
			// Reload data if preferences changed
			if (gForceReload)
			{
				ListViewLoadRecords(formP, false);
				gForceReload = false ;
			}

			if (gListViewSelection.dragging)
			{
				// Add the dragging bar if needed
				ListViewDrawMoveIndicator();
				handled = true;
			}
			ProcessingUpdate = false;
			break;

		case menuCmdBarOpenEvent:
			// Load UILIb to get its module DB
			SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
			SysGetModuleDatabase(uiLibRefNum, NULL, &uiLibDBP);
			MenuCmdBarAddButton(menuCmdBarOnLeft, uiLibDBP, BarSecureBitmap, menuCmdBarResultMenuItem, ListOptionsSecurityCmd, 0);
			// Unload the UILib
			SysUnloadModule(uiLibRefNum);

			// tell the field package to not add buttons automatically; we've done it all ourselves.
			eventP->data.menuCmdBarOpen.preventFieldButtons = true;

			// don't set handled to true; this eventP must fall through to the system.
			break;

		case sclRepeatEvent:
			ListViewScroll ((int16_t)(eventP->data.sclRepeat.newValue - eventP->data.sclRepeat.value));
			break;

		case penMoveEvent:
			if (gListViewSelection.dragging)
			{
				ListViewMoveDragging(eventP->screenX, eventP->screenY);
				handled = true;
			}
			break;

		case penUpEvent:
			if (gListViewSelection.dragging)
			{
				ListViewStopDragging();
				handled = true;
			}
			break;

		case nilEvent:
			if (gListViewSelection.dragging)
			{
				Coord x, y;
				Boolean penDown;
				EvtGetPen(&x, &y, &penDown);
				if (penDown)
				{
					ListViewMoveDragging(x, y);
					handled = true;
				}
			}
			break;
	}

	return handled;
}
