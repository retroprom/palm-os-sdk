/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressEdit.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *   This is the Address Book application's edit form module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

// Palm OS Includes
#include <AboutBox.h>
#include <CatMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <Field.h>
#include <Form.h>
#include <Loader.h>
#include <Menu.h>
#include <PenInputMgr.h>
#include <SelTime.h>
#include <SelDay.h>
#include <SoundMgr.h>
#include <StringMgr.h>
#include <SystemResources.h>
#include <Table.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <UIColor.h>
#include <string.h>
#include <TextServicesMgr.h>
#include <FeatureMgr.h>

// posix includes
#include <stdio.h>
#include <stdlib.h>

#include "AddressEdit.h"
#include "AddressTab.h"
#include "AddressDBSchema.h"
#include "AddressRsc.h"
#include "AddressTools.h"
#include "Address.h"
#include "AddressFreeFormName.h"
#include "AddressNote.h"

#include "AddressDialList.h"
#include "AddressAutoFill.h"
#include "AddressU32List.h"
#include "AddressDetails.h"
#include "AddressNote.h"
#include "AddressDBSchema.h"
#include "AddressTransfer.h"
#include "AddressExportAsk.h"

//for min/max
#include "PalmTypesCompatibility.h"


/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

#define kAddrEditBlankFont	stdFont

#define kNoFieldIndex		((uint16_t) 0xFFFF)

// Address edit table's rows and columns
#define kEditLabelColumn	0
#define kEditDataColumn		1

#define kSpaceBeforeDesc	2

#define kMaxEditFieldPosition	((uint32_t) 0xFFFFFFFF)

#define kTblNoSelectedRow		((int16_t)0x7FFF)

/***********************************************************************
 *
 *   Local structure
 *
 ***********************************************************************/
typedef struct DateTimeDrawInfoTag
{
	TableType *		tableP;
	int16_t			row;
	int16_t			column;
	RectangleType	bounds;
} DateTimeDrawInfoType;

/***********************************************************************
 *
 *   Static variables
 *
 ***********************************************************************/

static uint16_t			sCurrentFieldIndex;
static uint16_t			sEditLastFieldIndex;	// Number of field in the current tab.

// Handles used by non edit-in-place fields (numerical)
static MemHandle		sEditedFieldH = NULL;
static MemHandle		sNoEditedFieldH = NULL;

static Boolean			sRestoreEditState = false;

static BookType *		sBookP = NULL; // Book data type - BooksLib

static Boolean 			sDataFieldSelected = false;
static Boolean			sDeleteIconSelected = false;

static Boolean			sDataFieldSelection = false;
static Boolean			sDeleteIconSelection = false;
static RectangleType	sSelectedDataFieldBounds = {0, 0, 0, 0};
static RectangleType	sSelectedDeleteIconBounds = {0, 0, 0, 0};
static uint32_t			sSelectedDataFieldColumnId = kInvalidColumnID;
static Boolean			sInDateAndTimeEdition = false;
static int16_t			sSelectedDataFieldRow  = kTblNoSelectedRow;
static uint16_t			sSelectedDataFieldIndex = kNoFieldIndex;

static uint16_t			sHilitedFieldIndex = kNoFieldIndex;
static Boolean			sDuplicateRecord = false;

static char	*			sNoDataStr;
static char *			sDateAndTimeStr = NULL;

static MemHandle		sAlternateDeleteIconH = NULL;
static MemHandle		sAlternateDeleteHighlightedIconH = NULL;

static BitmapType*		sAlternateDeleteIconBmp = NULL;
static BitmapType*		sAlternateDeleteHighlightedIconBmp = NULL;

static MemHandle		sFullnameH = NULL;
static Coord			sFullnameWidth = 0;
static FontID			sFullnameFont = largeBoldFont;

static Boolean			sNamePopupDisplayed = false;
static uint64_t			sNamePopupDisplayTime;
static RectangleType	sNamePopupBounds;

static uint16_t			sTopVisibleFieldIndex = 0;
static uint32_t			sEditFieldPosition = 0;

static Boolean			sRowCategoryChanged = false;

static Boolean			sTblEnterEventOccured = false;
static char *			sYomiTextP = NULL;
static uint16_t			sYomiFieldIndex = kNoFieldIndex;

static Boolean			sEditViewIsFocusable = true;

static Boolean			sOnehandedFieldFocused = false;

static RectangleType	sGadgetWhiteArea = { 0 };

static Boolean			sReloadTable = false;

/***********************************************************************
 *
 * FUNCTION:
 *	PrvEditDialCurrent
 *
 * DESCRIPTION:
 *	This routine number in current field, or default one (show in list)
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
 *	aro			6/27/00		Initial Revision
 *
 ***********************************************************************/
static void PrvEditDialCurrent(void)
{
	uint32_t	colID = 0;
	TablePtr	tableP;
	int16_t		row;
	int16_t		column;
	uint16_t	fieldIndex;
	FormType*	frmP;
	uint32_t	size;

	// Get the columnID of the field that is currently edited
	// if it is a phone dial it, else dial showInlist
	frmP = FrmGetFormPtr(EditView);
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);

	if (TblEditing(tableP))
	{
		TblGetSelection(tableP, &row, &column);
		fieldIndex = TblGetRowID(tableP, row);
		colID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId;
	}

	// If we don't have found a column id or if the column id doesn't contains a valid phone number
	// the we check if the display phone is a valid phone number.
	if ((colID == 0) || !(ToolsIsPhoneIndexSupported(gCurrentRowID, colID)))
	{
		// Get the columnID for the displayed phone
		size = sizeof(colID);

		// If no error when getting the Display Phone, check it if it's a valid phone number.
		if (DbCopyColumnValue(gAddrDB, gCurrentRowID, kAddrColumnIDDisplayedPhone, 0, &colID, &size) >= errNone)
		{
			// Check if the Displayed Phone as a value and is a phone number.
			if (!ToolsIsPhoneIndexSupported(gCurrentRowID, colID))
				colID = 0;
		}
		else colID = 0;
	}

	// CGu
	// If ColId = 0, the function will search for a valid phone number in the record.
	if (!DialListShowDialog(gCurrentRowID, colID))
		SndPlaySystemSound (sndError);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeBuildString
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
 *
 ***********************************************************************/
char* PrvEditDateAndTimeBuildString(DbSchemaColumnType dataType, uint32_t dataValue)
{
	DateTimeType 	dateTime;
	TimeType		time;
	DateType		date;
	DateFormatType 	dateFormat;
	TimeFormatType 	timeFormat;
	char*			resultP = NULL;

	if (dataValue == (uint32_t)noTime)
		return ToolsStrDup(sNoDataStr);

	switch (dataType)
	{
		case dbDateTime:
			TimSecondsToDateTime(dataValue, &dateTime);
			resultP = ToolsBuildDateAndTimeString(&dateTime);
			break;

		case dbDate:
			resultP = MemPtrNew(longDateStrLength + 1);
			date = *((DateType*) &dataValue);
			dateFormat = (DateFormatType) PrefGetPreference(prefLongDateFormat);
			DateToAscii( (uint8_t) date.month, (uint8_t) date.day, (uint16_t) (date.year + firstYear), dateFormat, resultP);
			break;

		case dbTime:
			resultP = MemPtrNew(timeStringLength + 1);
			time = *((TimeType*) &dataValue);
			timeFormat = (TimeFormatType) PrefGetPreference(prefTimeFormat);
			TimeToAscii((uint8_t)time.hours, (uint8_t)time.minutes, timeFormat, resultP);
			break;

		default:
			resultP = ToolsStrDup(sNoDataStr);
			break;
	}

	return resultP;
}

/***********************************************************************
 *
 * FUNCTION:	PrvEditGetColumnText
 *
 * DESCRIPTION:	Retreives the text of an editable column
 *
 * PARAMETERS:	->	rowID - The DB row unique ID
 *				->	columnID - The column ID to get
 *				<-	bufferP - buffer to receive the text
 *				->	bufferSize - the buffer size
 *
 * RETURNED:	The number of charaters written in bufferP not including the terminating '\0'
 *				If the buffer is not large enough, it returns the number of characters actually
 *				needed (not including the trailling '\0')
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	Tes		11/20/2003	Initial revision
 *
 ***********************************************************************/
static size_t PrvEditGetColumnText(uint32_t rowID, uint32_t columnID, char *bufferP, uint32_t bufferSize, Boolean *defaultStrP)
{
	status_t	err;
	uint32_t	dataSize = 0;
	uint8_t	*	dataP = NULL;
	size_t		textLen = 0;
	char *		dateTimeTextP;
	uint32_t	value = (uint32_t)noTime;
	AddressTabColumnPropertiesType *	colPropP;

	*bufferP = nullChr;

	colPropP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID);
	if (!colPropP)
		goto Exit;

	err = DbGetColumnValue(gAddrDB, rowID, columnID, 0, (void**)&dataP, &dataSize);

	// If the column contains data, then we won't return a default value.
	if (defaultStrP)
		*defaultStrP = (Boolean)(dataSize == 0);

	if (colPropP->columnDataType == dbDateTime || colPropP->columnDataType == dbTime || colPropP->columnDataType == dbDate)
	{
		if (!dataP || !dataSize)
			value = (uint32_t)noTime;
		else if (dataSize == sizeof(uint16_t)) // DateType or TimeType
			value = *(uint16_t*)dataP;
		else // DateTimeType
			value = TimDateTimeToSeconds((DateTimeType*)dataP);

		dateTimeTextP = PrvEditDateAndTimeBuildString(colPropP->columnDataType, value);
		if (dateTimeTextP)
		{
			textLen = snprintf(bufferP, bufferSize, "%s", dateTimeTextP);
			MemPtrFree(dateTimeTextP);
		}

		goto Exit;
	}

	if (err < errNone || dataP == NULL)
		goto Exit;

	colPropP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID);
	if (!colPropP)
		goto Exit;

	switch (colPropP->columnDataType)
	{
	case dbVarChar:
	case dbChar:
		textLen = snprintf(bufferP, bufferSize, "%s", (char*)dataP);
		break;
	case dbUInt8:
		textLen = snprintf(bufferP, bufferSize, "%u", *((uint8_t*) dataP));
		break;
	case dbUInt16:
		textLen = snprintf(bufferP, bufferSize, "%u", *((uint16_t*) dataP));
		break;
	case dbUInt32:
		textLen = snprintf(bufferP, bufferSize, "%lu", *((uint32_t*) dataP));
		break;
	case dbUInt64:
		textLen = snprintf(bufferP, bufferSize, "%llu", *((uint64_t*) dataP));
		break;
	case dbInt8:
		textLen = snprintf(bufferP, bufferSize, "%d", *((int8_t*) dataP));
		break;
	case dbInt16:
		textLen = snprintf(bufferP, bufferSize, "%d", *((int16_t*) dataP));
		break;
	case dbInt32:
		textLen = snprintf(bufferP, bufferSize, "%ld", *((int32_t*) dataP));
		break;
	case dbInt64:
		textLen = snprintf(bufferP, bufferSize, "%lld", *((int64_t*) dataP));
		break;
	case dbFloat:
		textLen = snprintf(bufferP, bufferSize, "%f", *((float*) dataP));
		break;
	case dbDouble:
		textLen = snprintf(bufferP, bufferSize, "%f", *((double*) dataP));
		break;

	default:
		break;
	}

Exit:
	if (dataP)
		DbReleaseStorage(gAddrDB, dataP);

	return textLen;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditGetFieldHeight
 *
 * DESCRIPTION: This routine initialize a row in the to do list.
 *				Depending on the database column data type.
 *
 * PARAMETERS:  table        - pointer to the table of to do items
 *              fieldIndex   - the index of the field displayed in the row
 *              columnWidth  - height of the row in pixels
 *
 * RETURNED:    height of the field in pixels
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		06/26/95	Initial Revision
 * art		09/11/97	Add font support.
 * LFe		10/25/02	Use the New Tab Info and Data Mgr.
 * PPL		12/13/02	transform to dispatcher.
 * LFe		09/26/03	Merge and simplify the 5 get Height functions into one
 *
 ***********************************************************************/
static uint16_t PrvEditGetFieldHeight(TableType *tableP, uint16_t fieldIndex, int16_t columnWidth, int16_t maxHeight, FontID * fontIdP)
{
	uint32_t	columnID;
	int16_t		row;
	int16_t		column;
	uint16_t	height;
	uint16_t	lineHeight;
	FontID		currFont;
	char *		textP = NULL;
	Boolean		freeTextP = false;
	FieldType *	fieldP;
	Boolean		defaultStr = false;

	// blob are not yet managed. Will ever be ?
	if (gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType == dbBlob)
		return 0;

	if (TblEditing(tableP))
	{
		TblGetSelection(tableP, &row, &column);

		if (fieldIndex == TblGetRowID(tableP, row) && (fieldP = TblGetCurrentField(tableP)) != NULL)
			textP = FldGetTextPtr(fieldP);
	}

	// If colID is not initialized, load the data for the specified row.
	if (textP == NULL)
	{
		DbgOnlyFatalErrorIf(fieldIndex >= gBookInfo.tabs[gBookInfo.currentTabIndex].numElement, "Invalid field index");
		columnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId;

		textP = MemPtrNew(kStringMaxSize);
		ErrFatalDisplayIf(textP == NULL, "Out of memory!");
		freeTextP = true;
		PrvEditGetColumnText(gCurrentRowID, columnID, textP, kStringMaxSize, &defaultStr);
	}

	// If the field has text, or the field is the current field then used the view's current font setting.
	// else use blank font for empty field
	if ((textP && *textP) || sCurrentFieldIndex == fieldIndex)
		*fontIdP = gAddrEditFont;
	else
		*fontIdP = kAddrEditBlankFont;

	currFont = FntSetFont(*fontIdP);

	height = 1;

	if (textP && *textP)
	{
		Coord bitmapWidth = 0;
		DbSchemaColumnType	columnType;

		columnType = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType;
		if ((columnType == dbDate || columnType == dbTime || columnType == dbDateTime) && !defaultStr)
			BmpGetDimensions(sAlternateDeleteIconBmp, &bitmapWidth, NULL, NULL);

		height = (uint16_t)FldCalcFieldHeight(textP, columnWidth - bitmapWidth);
	}

	lineHeight = FntLineHeight();
	height = min(height, (maxHeight / lineHeight));
	height *= lineHeight;
	FntSetFont(currFont);

	// Deallocate textP only if not yet done when we set strP to textP below.
	if (freeTextP && textP)
		MemPtrFree(textP);

	return height;
}

/***********************************************************************
 *
 * FUNCTION:    PrvInitTableRowStyle
 *
 * DESCRIPTION: This routine initialize Set the correct data viewer.
 *				for the considered Cell.
 *
 * PARAMETERS:  table       - pointer to the table of to do items
 *				column		- column number (first column is zero)
 *              row         - row number (first row is zero)
 *              fieldIndex  - the index of the field displayed in the row
 *              rowHeight   - height of the row in pixels
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * art	06/26/95	Initial Revision
 * LFe	10/25/02	Update to new Tab UI and data manager
 *					Column ID are saved in the Table Data
 *					Remove Popup in the Label column
 * PPl	12/06/02	Add support to alternate to text data.
 *
 ***********************************************************************/
static void PrvInitTableRowStyle(TablePtr table, int16_t row, uint16_t fieldIndex, uint32_t columnId)
{
	TableItemStyleType	tabStyle = textTableItem;
	void*				dataP = NULL;
	uint32_t			dataSize, dateAndTimeValue;
	status_t			err;

	// WARNING: If you update the list of columnDataType, don't forget to also update it in AddressTab.c

	switch(gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType)
	{
		case dbBoolean:
			tabStyle = checkboxTableItem;
			TblSetItemInt (table, row, kEditDataColumn, false);
			err = DbGetColumnValue(gAddrDB, gCurrentRowID, columnId, 0, (void**) &dataP, &dataSize);

			if ((err >= 0) && dataP)
			{
				TblSetItemInt (table, row, kEditDataColumn, *((Boolean*) dataP));
				err = DbReleaseStorage(gAddrDB,dataP);
			}

			break;

		case dbDateTime:
		case dbDate:
		case dbTime:
			tabStyle = tallCustomTableItem;
			TblSetItemPtr (table, row, kEditDataColumn, (void*) noTime);
			err = DbGetColumnValue(gAddrDB, gCurrentRowID, columnId, 0, (void**) &dataP, &dataSize);

			if ((err >= 0) && dataP)
			{
				if (gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType == dbDateTime)
					dateAndTimeValue = TimDateTimeToSeconds((DateTimeType*) dataP);
				else dateAndTimeValue = *((int16_t*) dataP);

				TblSetItemPtr (table, row, kEditDataColumn, (void*) dateAndTimeValue);
				err = DbReleaseStorage(gAddrDB,dataP);
			}

			break;
	}

	TblSetItemStyle (table, row, kEditDataColumn, tabStyle);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditInitTableRow
 *
 * DESCRIPTION: This routine initialize a row in the edit view.
 *
 * PARAMETERS:  table       - pointer to the table of to do items
 *              row         - row number (first row is zero)
 *              fieldIndex  - the index of the field displayed in the row
 *              rowHeight   - height of the row in pixels
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * art	06/26/95	Initial Revision
 * LFe	10/25/02	Update to new Tab UI and data manager
 *					Column ID are saved in the Table Data
 *					Remove Popup in the Label column
 *
 ***********************************************************************/
static void PrvEditInitTableRow( FormType* frmP, TablePtr table, int16_t row, uint16_t fieldIndex, int16_t rowHeight, FontID fontID)
{
	uint32_t	columnId = 0;
	char*		label1P;
	char*		label2P;


	// Make the row usable.
	TblSetRowUsable (table, row, true);

	// Set the height of the row to the height of the desc
	TblSetRowHeight (table, row, rowHeight);

	// Store the record number as the row id.
	TblSetRowID (table, row, fieldIndex);

	// Mark the row invalid so that it will draw when we call the draw routine.
	TblMarkRowInvalid (table, row);

	// Set the text font.
	TblSetItemFont (table, row, kEditDataColumn, fontID);

	// Only Labels in the Label Column. no more popup.
	TblSetItemStyle (table, row, kEditLabelColumn, labelTableItem);

	if (fieldIndex < gBookInfo.tabs[gBookInfo.currentTabIndex].numElement)
	{
		// Get the columnID related to the field index.
		columnId = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId;

		// Get the original field label and the renamed field label (if exist)
		label1P = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].labelP;
		label2P = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].renamedLabelP;

		// If a renamed label exist use it instead of the original field label.
		if (label2P && *label2P)
			label1P = label2P;

		TblSetItemPtr(table, row, kEditLabelColumn, label1P);

		PrvInitTableRowStyle(table, row, fieldIndex, columnId);
	}

	// Store the ColumnID as row data
	TblSetRowData(table, row, columnId);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the edit view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frmP			- pointer to the address edit form
 *              bottomField		- field index of the last visible row
 *              lastItemClipped	- true if the last visible row is clip at
 *                                the bottom
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	06/26/95	Initial Revision
 *
 ***********************************************************************/
static void PrvEditUpdateScrollers(FormType * frmP)
{
	uint16_t	upIndex;
	uint16_t	downIndex;
	Boolean		scrollableUp = false;
	Boolean		scrollableDown = false;
	TableType *	tableP;
	int16_t		lastRowIndex;
	uint32_t	fieldIndex;

	// If the first field displayed is not the fist field in the record,
	// enable the up scroller.
	scrollableUp = (Boolean) (sTopVisibleFieldIndex > 0);

	// If the last field displayed is not the last field in the record,
	// enable the down scroller.
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);

	if ((lastRowIndex = TblGetLastUsableRow(tableP)) != kTblInvalidRowIndex)
	{
		RectangleType	bounds;
		Coord			columnWidth;
		FontID			font;
		uint16_t		neededHeight;

		fieldIndex = TblGetRowID(tableP, lastRowIndex);
		columnWidth = TblGetColumnWidth(tableP, kEditDataColumn);
		TblGetBounds(tableP, &bounds);
		neededHeight = PrvEditGetFieldHeight(tableP, (uint16_t)fieldIndex, columnWidth, bounds.extent.y, &font);
		scrollableDown = (Boolean) (fieldIndex < sEditLastFieldIndex || TblGetRowHeight(tableP, lastRowIndex) < neededHeight);
	}

	// Update the scroll button.
	upIndex = FrmGetObjectIndex (frmP, EditUpButton);
	downIndex = FrmGetObjectIndex (frmP, EditDownButton);
	FrmUpdateScrollers(frmP, upIndex, downIndex, scrollableUp, scrollableDown);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditLoadTable
 *
 * DESCRIPTION: This routine reloads to do database records into
 *              the edit view.  This routine is called when:
 *                 o A field height changes (Typed text wraps to the next line)
 *                 o Scrolling
 *                 o Advancing to the next field causes scrolling
 *                 o The focus moves to another field
 *                 o A custom label changes
 *                 o The form is first opened.
 *
 *                The row ID is an index into gFieldMap.
 *
 * PARAMETERS:  frmP - Edit form pointer.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		05/01/95	Initial Revision
 * art		09/10/97	Rewrote to support user selectable fonts.
 * aro		09/21/00	Add frmP as an argument for the updateEvent
 * LFe		10/25/02	Updated to support new Data Mgr and Tab UI.
 *
 ***********************************************************************/
static void PrvEditLoadTable(FormType* frmP, Boolean forceRowInit)
{
	int16_t 		row;
	uint16_t 		numRows;
	uint16_t 		fieldIndex;
	uint16_t 		lastFieldIndex;
	uint16_t 		dataHeight;
	uint16_t 		tableHeight;
	uint16_t 		columnWidth;
	uint16_t 		pos, oldPos;
	uint16_t 		height;
	uint16_t		oldHeight;
	FontID 			fontID;
	FontID			savFontId;
	TablePtr 		tableP;
	Boolean 		rowUsable;
	Boolean 		rowsInserted = false;
	RectangleType 	r;
	Coord			lineHeight;

	// Get the height of the table and the width of the description
	// column.
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
	TblGetBounds (tableP, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (tableP, kEditDataColumn);

	savFontId = FntGetFont();

	// If we currently have a selected record, make sure that it is not
	// above the first visible record.
	if (sCurrentFieldIndex != kNoFieldIndex && sCurrentFieldIndex < sTopVisibleFieldIndex)
		sTopVisibleFieldIndex = sCurrentFieldIndex;

	row = 0;
	dataHeight = 0;
	oldPos = pos = 0;
	fieldIndex = sTopVisibleFieldIndex;
	lastFieldIndex = fieldIndex;

	// Load records into the table.
	while (fieldIndex <= sEditLastFieldIndex)
	{
		// Compute the height of the field's text string.
		height = PrvEditGetFieldHeight (tableP, fieldIndex, columnWidth, tableHeight, &fontID);

		FntSetFont(fontID);
		lineHeight = FntLineHeight();

		// Is there enought room for the decription.
		if (dataHeight + height > tableHeight)
		{
			// If the current selected field will be truncated, move the table one row up
			if (sCurrentFieldIndex == fieldIndex)
			{
				sTopVisibleFieldIndex++;
				fieldIndex = sTopVisibleFieldIndex;
				row = 0;
				dataHeight = 0;
				oldPos = pos = 0;
				continue;
			}

			// Calculate how many description lines can be shown
			height = tableHeight - dataHeight;

			if (height > lineHeight)
				height -= (height % lineHeight);
		}

		// Still enough room to show one description line
		if (height >= lineHeight)
		{
			rowUsable = TblRowUsable (tableP, row);

			// Get the height of the current row.
			if (rowUsable)
				oldHeight = TblGetRowHeight (tableP, row);
			else
				oldHeight = 0;

			// If the field is not already being displayed in the current
			// row, load the field into the table.
			if ((! rowUsable) ||
				(forceRowInit) ||
				(TblGetRowID (tableP, row) != fieldIndex) ||
				(TblGetItemFont (tableP, row, kEditDataColumn) != fontID))
			{
				PrvEditInitTableRow(frmP, tableP, row, fieldIndex, height, fontID);
			}

			// If the height or the position of the item has changed draw the item.
			else if (height != oldHeight)
			{
				TblSetRowHeight (tableP, row, height);
				TblMarkRowInvalid (tableP, row);
			}
			else if (pos != oldPos)
			{
				TblMarkRowInvalid (tableP, row);
			}

			TblSetItemFont (tableP, row, kEditDataColumn, fontID);

			pos += height;
			oldPos += oldHeight;
			lastFieldIndex = fieldIndex;
			fieldIndex++;
			row++;
		}

		dataHeight += height;

		// Is the table full?
		if (dataHeight >= tableHeight)
		{
			// If we have a currently selected field, make sure that it is
			// not below the last visible field.  If the currently selected
			// field is the last visible record, make sure the whole field
			// is visible.
			if (sCurrentFieldIndex == kNoFieldIndex)
				break;

			if  (sCurrentFieldIndex < fieldIndex)	// Above last visible?
				break;

			if (fieldIndex == lastFieldIndex &&		// Last visible?
				(fieldIndex == sTopVisibleFieldIndex || dataHeight == tableHeight))
				break;

			// Remove the top item from the table and reload the table again.
			sTopVisibleFieldIndex++;
			fieldIndex = sTopVisibleFieldIndex;
			row = 0;
			dataHeight = 0;
			oldPos = pos = 0;
		}
	}

	// Hide the item that don't have any data.
	numRows = TblGetNumberOfRows (tableP);

	while (row < numRows)
	{
		TblSetRowUsable (tableP, row, false);
		row++;
	}

	// If the table is not full and the first visible field is
	// not the first field	in the record, displays enough fields
	// to fill out the table by adding fields to the top of the table.
	while (dataHeight < tableHeight)
	{
		fieldIndex = sTopVisibleFieldIndex;

		if (fieldIndex == 0)
			break;

		fieldIndex--;

		// Compute the height of the field.
		height = PrvEditGetFieldHeight (tableP, fieldIndex, columnWidth, tableHeight, &fontID);

		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;

		// Insert a row before the first row.
		TblInsertRow (tableP, 0);

		PrvEditInitTableRow(frmP, tableP, 0, fieldIndex, height, fontID);

		sTopVisibleFieldIndex = fieldIndex;

		rowsInserted = true;

		dataHeight += height;
	}

	if (sCurrentFieldIndex != kNoFieldIndex && TblFindRowID(tableP, sCurrentFieldIndex, &row))
		TblSetSelection(tableP, row, kEditDataColumn);

	// If rows were inserted to full out the page, invalidate the whole
	// table, it all needs to be redrawn.
	if (rowsInserted)
		TblMarkTableInvalid (tableP);

	FntSetFont(savFontId);

	PrvEditUpdateScrollers(frmP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDrawTableCallback
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
 *
 *
 ***********************************************************************/
static Boolean PrvEditDrawTableCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	TableType *	tableP;
	FormType *	formP;

	if (cmd != winInvalidateDestroy)
	{
		formP = FrmGetFormPtr(EditView);
		tableP = ToolsGetFrmObjectPtr(formP, EditTable);
		TblRedrawTable(tableP);
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditIsFieldEditable
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
 *
 ***********************************************************************/
static Boolean PrvEditIsFieldEditable(uint16_t tabIndex, uint16_t fieldIndex)
{
	switch (gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].columnDataType)
	{
	case dbVarChar:
	case dbChar:
	case dbUInt8:
	case dbUInt16:
	case dbUInt32:
	case dbUInt64:
	case dbInt8:
	case dbInt16:
	case dbInt32:
	case dbInt64:
	case dbFloat:
	case dbDouble:
		return true;

	default:
		return false;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditRestoreEditState
 *
 * DESCRIPTION: This routine restores the edit state of the Edit
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		09/12/97	Initial Revision
 *
 ***********************************************************************/
static void PrvEditRestoreEditState(FormType *formP)
{
	int16_t				row;
	TablePtr			tableP;
	FieldPtr			fieldP;

	if (sCurrentFieldIndex == kNoFieldIndex || !formP)
		return;

	// Find the row that the current field is in.
	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	if (!PrvEditIsFieldEditable(gBookInfo.currentTabIndex, sCurrentFieldIndex) || !TblFindRowID(tableP, sCurrentFieldIndex, &row))
		return;

	FrmSetFocus(formP, FrmGetObjectIndex(formP, EditTable));
	TblGrabFocus(tableP, row, kEditDataColumn);

	// Restore the insertion point position.
	if((fieldP = TblGetCurrentField(tableP)) == NULL)
	{
		TableItemStyleType type;

		type = TblGetItemStyle(tableP, row, kEditDataColumn);

		// Don't give the focus to non-editable fields
		if (type != textTableItem && type != textWithNoteTableItem)
			return;

		TblGrabFocus(tableP, row, kEditDataColumn);
		fieldP = TblGetCurrentField(tableP);
	}

	// Discard any selection
	gNumCharsToHilite = 0;

	FldGrabFocus(fieldP);
	FldSetInsertionPoint(fieldP, sEditFieldPosition);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditReloadTable
 *
 * DESCRIPTION: Reloads the table
 *
 * PARAMETERS:  ->	formP
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		08/02/04	Extracted from EditViewHandleEvent
 *
 ***********************************************************************/
static void PrvEditReloadTable(FormType *formP)
{
	FieldType *		fldP;
	size_t			startPosition = 0;
	size_t			stopPosition = 0;
	int16_t			row, col;
	uint16_t		fieldIndex;
	TableType *		tableP;
	RectangleType	bounds;

	tableP = ToolsGetFrmObjectPtr(formP, EditTable);
	TblGetSelection(tableP, &row, &col);
	fldP = TblGetCurrentField(tableP);
	fieldIndex = TblGetRowID(tableP, row);

	if (fldP)
		FldGetSelection(fldP, &startPosition, &stopPosition);	// Save the selection of the field

	TblReleaseFocus(tableP);
	PrvEditLoadTable(formP, false);
	TblFindRowID(tableP, fieldIndex, &row);
	TblGrabFocus(tableP, row, kEditDataColumn);
	TblGetBounds(tableP, &bounds);
	ToolsFrmInvalidateRectFunc(EditView, &bounds, PrvEditDrawTableCallback, NULL);

	// Restore the selection of the field or restore the insertion point
	if (fldP)
	{
		if ( startPosition != stopPosition )
			FldSetSelection(fldP, startPosition, stopPosition);
		else
			FldSetInsPtPosition(fldP, sEditFieldPosition);
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditHandleSelectField
 *
 * DESCRIPTION: Handle the user tapping an edit view field label.
 *   Either the a phone label is changed or the user wants to edit
 * a field by tapping on it's label.
 *
 * PARAMETERS:  row    - row of the item to select (zero based)
 *              column - column of the item to select (zero based)
 *
 * RETURNED:    true if the event was handled and nothing else should
 *              be done
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	11/27/95	Cut from EditViewHandleEvent
 * art		09/02/97	Add multi-font support
 * roger	11/04/97	Changed parameters to support another routine
 * jmp		04/18/00	Fixed bug #23237:  When changing an kEditLabelColumn,
 *						mark the row invalid so that we redraw everything.
 *						If we don't do this, the edit indicator's colors
 *						don't come out correctly.
 * fpa		10/23/00	Fixed bug #42762 - Can't select text when cursor
 *						is in a blank field
 * gap		10/25/00	fix above did not take into account that there are
 *						occasions where fldP is NULL.
 * fpa		11/06/00	Fixed bug #23088 - Cannot insert a cursor between 2
 *						characters. Undid gap above modification because it
 *						was reopening bug #42762
 * ppl		02/13/02	Add Active Input Area Support (AIA)
 *
 ***********************************************************************/
static Boolean PrvEditHandleSelectField(FormType* frmP, TableType* tableP, int16_t row, int16_t column)
{
	int16_t			currRow;
	uint16_t		fieldIndex;
	FontID			currFont;
	FieldType *		fldP = NULL;
	uint32_t		columnID;
	uint32_t		dataSize;
	RectangleType	tableBounds;

	// Make sure the heights the field we are exiting and the one
	// that we are entering are correct.  They may be incorrect if the
	// font used to display blank line is a different height then the
	// font used to display field text.date

	fieldIndex = TblGetRowID (tableP, row);

	sReloadTable = false;

	if (fieldIndex != sCurrentFieldIndex || TblGetCurrentField(tableP) == NULL)
	{
		currFont = FntGetFont();

		// Is there a current field and is it empty?
		if (sCurrentFieldIndex != kNoFieldIndex)
		{
			dataSize = 0;
			columnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[sCurrentFieldIndex].columnId;
			DbCopyColumnValue(gAddrDB, gCurrentRowID, columnID, 0, NULL, &dataSize);

			if (!dataSize && TblFindRowID(tableP, sCurrentFieldIndex, &currRow))
			{
				// Is the height of the field correct?
				FntSetFont(kAddrEditBlankFont);
				sReloadTable = (Boolean)(FntLineHeight() != TblGetRowHeight(tableP, currRow));
			}
		}

		sCurrentFieldIndex = fieldIndex;

		// Is the newly selected field empty?
		dataSize = 0;
		columnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId;
		DbCopyColumnValue(gAddrDB, gCurrentRowID, columnID, 0, NULL, &dataSize);

		if (!dataSize)
		{
			// Is the height of the field correct?
			FntSetFont(gAddrEditFont);
			sReloadTable = (Boolean)(FntLineHeight() != TblGetRowHeight(tableP, row));

			// Redraw a single line if needed
			if (!sReloadTable && gAddrEditFont != kAddrEditBlankFont)
			{
				TblMarkRowInvalid(tableP, row);
				TblGetBounds(tableP, &tableBounds);
				ToolsFrmInvalidateRectFunc(EditView, &tableBounds, PrvEditDrawTableCallback, NULL);
			}
		}

		FntSetFont (currFont);
	}

	// Set the focus on the field if necessary
	if (TblGetCurrentField(tableP) == NULL && ToolsIsTextFieldByIndex(gBookInfo.currentTabIndex, sCurrentFieldIndex))
	{
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditTable));
		TblGrabFocus(tableP, row, kEditDataColumn);
		fldP = TblGetCurrentField(tableP);
		FldGrabFocus(fldP);
		FldMakeFullyVisible (fldP);
		fldP = NULL;
	}

	// If the tblSelectEvent occurs in the label column, reload the table
	// immediatly if needed. This is because when selecting the label column
	// we first receive the penUpEvent the the tblSelectEvent (contrary to
	// the selection of the data column)
	if (column == kEditLabelColumn && sReloadTable)
	{
		PrvEditReloadTable(frmP);
		sReloadTable = false;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSetGraffitiMode
 *
 * DESCRIPTION: Set the graffiti mode based on the field being edited.
 *
 * PARAMETERS:  currentField - the field being edited.
 *
 * RETURNED:    the graffiti mode is set
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	09/20/95	Initial Revision
 *
 ***********************************************************************/
static void PrvEditSetGraffitiMode(uint32_t columnID)
{
	uint16_t	mode;

	mode = PINGetInputMode();

	// Change input mode only if non FEP or FEP is off
	if ((gDeviceHasFEP && TsmGetFepMode() != tsmFepModeOff) ||
		mode == pinInputModeHiragana || mode == pinInputModeKatakana)
		return;

	if (ToolsIsPhoneFieldByColId(columnID, true)) // Phone number
        mode = pinInputModeNumeric;
	else if (ToolsIsPhoneFieldByColId(columnID, false) || mode == pinInputModeNumeric) // email address
		mode = pinInputModeNormal;
	else // Leave in current mode
		return;

	PINSetInputMode(mode);
}

/***********************************************************************
 *
 * FUNCTION:    	PrvEditDrawName
 *
 * DESCRIPTION:
 * 		Draw the Full name contructed with data in the address record.
 *
 * PARAMETERS:
 *		->	formP : Edit form ptr.
 *
 * RETURNED:
 *		None.
 *
 * HISTORY:
 *		Name	Date			Description
 *		----	----		-----------
 *		PPL		11/21/02	Written using address free form name function.
 *
 ***********************************************************************/
static void PrvEditDrawName(FormType *formP)
{
	FontID				currFont;
	RectangleType		r;
	uint16_t			x = 0;
	uint16_t			y;
	char *				textP = NULL;
	size_t				textLen;
	WinDrawOperation	oldDrawMode;

	// Display Full name
	currFont = FntSetFont(sFullnameFont);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditViewName), &r);
	//WinEraseRectangle(&r, 0);
	y = r.topLeft.y + (r.extent.y >> 1) - (FntCharHeight() >> 1);

	if (sFullnameH)
	{
		textP = (char*)MemHandleLock(sFullnameH);
		textLen = strlen(textP);
		oldDrawMode = WinSetDrawMode(winOverlay);
		WinPaintTruncChars(textP, (int16_t)textLen, x, y, r.extent.x);
		WinSetDrawMode(oldDrawMode);
		MemHandleUnlock(sFullnameH);
	}

	FntSetFont(currFont);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDrawNameCallback
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
 *
 *
 ***********************************************************************/
static Boolean PrvEditDrawNameCallback(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP)
{
	FormType *	formP;

	if (cmd == formGadgetDrawCmd)
	{
		formP = FrmGetFormPtr(EditView);
		PrvEditDrawName(formP);
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSetCategoryTriggerLabel
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
 *
 *
 ***********************************************************************/
static void PrvEditSetCategoryTriggerLabel(FormType *formP)
{
	Coord			displayWidth;
	Coord			labelWidth;
	RectangleType	bounds;
	ControlType *	triggerP;
	uint32_t		numCategories = 0;
	CategoryID *	categoriesP = NULL;
    status_t		err;
	FontID			currentFont;

	err = DbGetCategory(gAddrDB, gCurrentRowID, &numCategories, &categoriesP);
	if (err < errNone)
		return;

	WinGetDisplayExtent(&displayWidth, NULL);

	// Compute maximum label width
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, AddressBusinessCardBmp), &bounds);
	labelWidth = displayWidth - bounds.topLeft.x - 5; // Remove 5 pixels to not be too close from the indicator or title bar
	if (gBusinessCardRowID == gCurrentRowID && gBusinessCardRowID != dbInvalidRowID)
		labelWidth -= bounds.extent.x;

	// Get truncated category for maximum width
	CatMgrTruncateName(gAddrDB, categoriesP, numCategories, labelWidth, gCategoryName);

	// Get real width
	currentFont = FntSetFont(stdFont);
	labelWidth = FntCharsWidth(gCategoryName, strlen(gCategoryName));
	FntSetFont(currentFont);

	// Set trigger
	triggerP = ToolsGetFrmObjectPtr(formP, EditCategoryTrigger);
	CtlSetLabel(triggerP, gCategoryName);

	// Cleanup
	if (categoriesP)
		DbReleaseStorage(gAddrDB, categoriesP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDrawBusinessCardIndicator
 *
 * DESCRIPTION: Draw the business card indicator if the current record is
 * the business card.
 *
 * PARAMETERS:  formP - the form containing the business card indicator
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	12/03/97	Initial Revision
 * LFe		10/24/02	Use new Database Manager
 *
 ***********************************************************************/
static void PrvEditDrawBusinessCardIndicator(FormType *formP)
{
	if (gBusinessCardRowID == gCurrentRowID && gBusinessCardRowID != dbInvalidRowID)
	{
		FrmShowObject(formP, FrmGetObjectIndex(formP, AddressBusinessCardBmp));
		// Redraw the category trigger so it overwrites the business card indicator
		FrmShowObject(formP, FrmGetObjectIndex(formP, EditCategoryTrigger));
	}
	else
	{
		FrmHideObject(formP, FrmGetObjectIndex(formP, AddressBusinessCardBmp));
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDrawTblDrawProc
 *
 * DESCRIPTION: This function draws table fields that correspond to
 *				Date, Time Date and Time fields in database record.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision.
 * LFe		10/29/02	Use new Data Manager & Edit in place.
 * PPL		07/07/03	Add data text width calculations stored as Item Ptr.
 *
 *
 ***********************************************************************/
static void PrvEditDateAndTimeDrawTblDrawProc(void *tableP, int16_t row, int16_t column, RectangleType *boundsP)
{
	BitmapType* 	bitmapP;
	Coord			bitmapWidth;
	Coord			bitmapHeight;
	Coord			bitmapY;
	Coord			bitmapX;
	Coord			dataWidth;
	char *			textP;
	FontID			fontId;
	uint32_t		dataValue;
	uint16_t		fieldIndex;
	Coord			textMaxWidth;

	// Set back to unselected values
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

	dataValue = (uint32_t) TblGetItemPtr(tableP,  row,  kEditDataColumn);
	fieldIndex = TblGetRowID(tableP, row);

	textMaxWidth = boundsP->extent.x;
	//WinEraseRectangle(boundsP, 0);

	// we draw only the delete date icon when we have data set
	if (dataValue != (uint32_t)noTime)
	{
		bitmapP = (sDeleteIconSelected && sSelectedDataFieldRow == row) ? sAlternateDeleteHighlightedIconBmp : sAlternateDeleteIconBmp;

		if (bitmapP)
		{
			BmpGetDimensions (bitmapP, &bitmapWidth, &bitmapHeight, NULL);

			// we center vertically the icon
			bitmapY = boundsP->topLeft.y + ((boundsP->extent.y - bitmapHeight) / 3) + 1;
			bitmapX = boundsP->topLeft.x + boundsP->extent.x - bitmapWidth - 1;

			WinDrawBitmap(bitmapP, bitmapX, bitmapY);

			textMaxWidth -= bitmapWidth + 1;
		}
	}

	textP = PrvEditDateAndTimeBuildString(gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType, dataValue);

	if (textP)
	{
		char *	labelP;
		Coord	y;
		Coord	lineHeight;
		size_t	offset = 0;

		fontId = FntSetFont(TblGetItemFont (tableP, row, kEditDataColumn));
		dataWidth = FntCharsWidth(textP, strlen(textP));

		labelP = textP;
		lineHeight = FntCharHeight();
		y = boundsP->topLeft.y;

		while (*labelP && y < boundsP->topLeft.y + boundsP->extent.y)
		{
			offset = FntWordWrap(labelP, textMaxWidth);
			ToolsDrawTextLabel(labelP, offset, boundsP->topLeft.x, y, (Boolean) ((sSelectedDataFieldRow == row) && sDataFieldSelected));
			labelP += offset;
			y += lineHeight;
		}

		TblSetItemInt(tableP, row, kEditDataColumn, dataWidth);
		FntSetFont(fontId);

		MemPtrFree(textP);
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDrawCallback
 *
 * DESCRIPTION: Update callback to draw only the label.
 *
 * PARAMETERS: 	Usual draw callback parmeter
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		06/23/2004	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvEditDateAndTimeDrawCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *userParamP)
{
	DateTimeDrawInfoType *	dtInfoP = (DateTimeDrawInfoType*)userParamP;

	if (cmd == winInvalidateDestroy)
	{
		MemPtrFree(dtInfoP);
	}
	else
	{
		PrvEditDateAndTimeDrawTblDrawProc(dtInfoP->tableP, dtInfoP->row, dtInfoP->column, &dtInfoP->bounds);
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDraw
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		06/23/2004	Initial Revision
 *
 ***********************************************************************/
static void PrvEditDateAndTimeDraw(TableType *tableP, int16_t row, int16_t column, RectangleType *boundsP)
{
	DateTimeDrawInfoType *	dtInfoP;

	dtInfoP = MemPtrNew(sizeof(DateTimeDrawInfoType));
	ErrFatalDisplayIf(!dtInfoP, "Out of memory!");
	dtInfoP->tableP	= tableP;
	dtInfoP->row	= row;
	dtInfoP->column	= column;
	dtInfoP->bounds	= *boundsP;

	ToolsFrmInvalidateRectFunc(EditView, boundsP, PrvEditDateAndTimeDrawCallback, (void*)dtInfoP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeEdit
 *
 * DESCRIPTION: Edit
 *
 * PARAMETERS: 	event: the event to handle.

 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision
 *
 ***********************************************************************/
 static void PrvEditDateAndTimeEdit(TableType* tableP)
 {
 	DateType		date;
	int16_t 		month;
	int16_t 		day;
	int16_t 		year;
	TimeType		time;
	int16_t 		hours = 0;
	int16_t 		minutes = 0;
	void*			dataP = NULL;
	uint32_t		dataSize = 0;
	uint32_t		dataValue;
	DateTimeType	dateTime;
	status_t		err;
	MemHandle		titleH;
	char *			titleP;
	uint16_t		dateInt;
	int16_t			timeInt;
	Boolean			ok  = false;

	dataValue = (uint32_t)TblGetItemPtr(tableP, sSelectedDataFieldRow, kEditDataColumn);

	sInDateAndTimeEdition = true;

	switch( gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[sSelectedDataFieldIndex].columnDataType )
	{
		case dbDate:

			if (dataValue == (uint32_t)noTime)
				DateSecondsToDate (TimGetSeconds(), &date);
			else
				date = *((DateType*)&dataValue);

			month = date.month;
			day = date.day;
			year = date.year + firstYear;

			titleH = DmGetResource(gApplicationDbP, strRsc, SetDateTitleStr);
			titleP = (char*)DmHandleLock(titleH);
 			ok =  SelectDay(selectDayByDay, &month, &day, &year, titleP);
			DmHandleUnlock(titleH);
			DmReleaseResource(titleH);

 			if (ok)
 			{
 				date.month = (uint16_t) month;
				date.day = (uint16_t) day;
				date.year = (uint16_t) year - firstYear;

				dataValue = dateInt = DateToInt(date);

				dataP = (void*)  &dateInt;
				dataSize = sizeof(dateInt);
 			}
			break;

		case dbTime:
			if (dataValue == (uint32_t)noTime)
			{
				TimSecondsToDateTime(TimGetSeconds (), &dateTime);

				hours = (int16_t) dateTime.hour;
				minutes =  (int16_t) dateTime.minute;
			}
			else
			{
				time = *((TimeType*) &dataValue);

				hours = (int16_t) time.hours;
				minutes =  (int16_t) time.minutes;
			}

			titleH = DmGetResource(gApplicationDbP, strRsc, SetTimeTitleStr);
			titleP = (char*)DmHandleLock(titleH);
			ok = SelectOneTime(&hours, &minutes, titleP);
			DmHandleUnlock(titleH);
			DmReleaseResource(titleH);

			if (ok)
			{
				time.hours = (uint8_t) hours;
				time.minutes = (uint8_t) minutes;

				dataValue = timeInt = TimeToInt(time);

				dataP = (void*) &timeInt;
				dataSize = sizeof(timeInt);
 			}
 			break;

		case dbDateTime:

			if (dataValue == (uint32_t)noTime)
				dataValue = TimGetSeconds();

			TimSecondsToDateTime(dataValue, &dateTime);

			month = dateTime.month;
			day =  dateTime.day;
			year =  dateTime.year;
			hours =  dateTime.hour;
			minutes =   dateTime.minute;

			titleH = DmGetResource(gApplicationDbP, strRsc, SetDateTitleStr);
			titleP = (char*)DmHandleLock(titleH);
 			ok =  SelectDay (selectDayByDay, &month, &day, &year, titleP);
			DmHandleUnlock(titleH);
			DmReleaseResource(titleH);

 			if (ok)
 			{
 				dateTime.month = (uint16_t) month;
				dateTime.day = (uint16_t) day;
				dateTime.year = (uint16_t) year;
 			}
 			else
 				// we do not want to ask time without a date
 				break;

			titleH = DmGetResource(gApplicationDbP, strRsc, SetTimeTitleStr);
			titleP = (char*)DmHandleLock(titleH);
			ok = SelectOneTime(&hours, &minutes, titleP);
			DmHandleUnlock(titleH);
			DmReleaseResource(titleH);

			if (ok)
			{
				dateTime.hour = (uint8_t) hours;
				dateTime.minute = (uint8_t) minutes;

 				dataP = (void*) &dateTime;
				dataSize = sizeof(dateTime);

				dataValue = TimDateTimeToSeconds(&dateTime);
 			}
			break;
	}

	sInDateAndTimeEdition = false;

	if (ok)
	{
		TblSetItemPtr(tableP, sSelectedDataFieldRow, kEditDataColumn, (void *)dataValue);
		err = DbWriteColumnValue(gAddrDB, gCurrentRowID, sSelectedDataFieldColumnId, 0, -1, dataP, dataSize);
		ErrNonFatalDisplayIf(err < errNone, "Can't write column");
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditBooleanSelect
 *
 * DESCRIPTION: Edit
 *
 * PARAMETERS: 	event: the event to handle.
 *
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		07/07/03	Initial Revision.
 *
 ***********************************************************************/
 static Boolean PrvEditBooleanSelect(TableType *tableP, int16_t row, int16_t column)
 {
 	Boolean		boolValue;
	void*		dataP;
	uint32_t	dataSize;
	uint32_t	dataFieldColumnId;
	uint16_t	dataFieldIndex;
	status_t	err;
	Boolean		handled = false;

	dataFieldIndex = TblGetRowID(tableP, row );
	dataFieldColumnId = TblGetRowData (tableP, row );

  	if (gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[dataFieldIndex].columnDataType == dbBoolean)
  	{
		// for check boxes , the Table event hanler already handles the event
		// (once we call it... or course)
		// at pen up time, the check box is checked or unchecked
		// we need to save the value
		boolValue = (Boolean)(TblGetItemInt(tableP, row, kEditDataColumn) != 0);

		dataP = (void*) &boolValue;
		dataSize = sizeof(boolValue);

		err = DbWriteColumnValue(gAddrDB, gCurrentRowID, dataFieldColumnId, 0, -1, dataP, dataSize);
		ErrNonFatalDisplayIf(err < errNone, "Can't write column");
		handled = true;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDrawBitmapCallback
 *
 * DESCRIPTION: Update callback to draw only the Bitmap.
 *
 * PARAMETERS: 	.
 *
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		10/03/03	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvEditDateAndTimeDrawBitmapCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	BitmapPtr	bitmapP;

	if (cmd == winInvalidateDestroy)
		return true;

	bitmapP  = (sDeleteIconSelected) ? sAlternateDeleteHighlightedIconBmp : sAlternateDeleteIconBmp;

	if (bitmapP)
		WinDrawBitmap(bitmapP, diryRect->topLeft.x, diryRect->topLeft.y);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDrawTextCallback
 *
 * DESCRIPTION: Update callback to draw only the label.
 *
 * PARAMETERS: 	.
 *
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		10/03/03	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvEditDateAndTimeDrawTextCallback(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	FormPtr		formP;
 	TableType*	tableP;
	uint32_t	dataValue;

	if (cmd == winInvalidateDestroy)
		return true;

	formP = FrmGetFormPtr(EditView);
	tableP = (TableType*) ToolsGetFrmObjectPtr(formP,  EditTable);
	dataValue = (uint32_t) TblGetItemPtr(tableP,  sSelectedDataFieldRow,  kEditDataColumn);

	if (!sDateAndTimeStr)
		sDateAndTimeStr = PrvEditDateAndTimeBuildString(gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[sSelectedDataFieldIndex].columnDataType, dataValue);

	ToolsDrawTextLabel(sDateAndTimeStr, strlen(sDateAndTimeStr), diryRect->topLeft.x, diryRect->topLeft.y, sDataFieldSelected);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDelete
 *
 * DESCRIPTION: Remove data from the database record.
 *
 * PARAMETERS: 	event: the event to handle.
 *
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision
 *
 ***********************************************************************/
 static void PrvEditDateAndTimeDelete(TableType *tableP)
 {
	status_t	err;

	sInDateAndTimeEdition = true;

	if (FrmAlert(gApplicationDbP, DeleteAlternateToTextDataAlert) == DeleteAlternateToTextDataYes)
	{
		err = DbWriteColumnValue(gAddrDB, gCurrentRowID, sSelectedDataFieldColumnId, 0, -1, NULL, 0);
		ErrNonFatalDisplayIf(err < errNone, "Can't write column");

		TblSetItemPtr(tableP, sSelectedDataFieldRow, kEditDataColumn, (void*) noTime);
	}

	sInDateAndTimeEdition = false;
}

 /***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeEnter
 *
 * DESCRIPTION: This function handles tap on Alternate Data type fields
 *				that requires to open a dialog box foe editing.
 *
 * PARAMETERS: 	->	tableP
 *				->	row
 *				<-	dataBoundsP: The date bounds are returned here
 *				<-	deleteIconBoundsP: The delete icon bounds are returned here
 *
 * RETURNED:    true if the passes table item is a tallCustomTableItem
 *				false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static Boolean PrvEditDateAndTimeGetBounds(TableType *tableP, int16_t row, RectangleType *dataBoundsP, RectangleType *deleteIconBoundsP)
{
	int32_t		dataValue;
	Coord		bitmapWidth;
	Coord		bitmapHeight;
	int16_t		width;

	// if the column is an item Text item or if the record is masked, dialing is
	// impossible, so just let the table handle selection.
	if (TblGetItemStyle(tableP, row, kEditDataColumn) != tallCustomTableItem ||
		TblRowMasked(tableP, row))
		return false;

	// Calculate data bounds and delete icon bounds
	// we need to handle delete icon only if we have time/date or datetime data set
	dataValue = (uint32_t)TblGetItemPtr(tableP, row, kEditDataColumn);

	TblGetItemBounds(tableP, row, kEditDataColumn, dataBoundsP);

	if (dataValue != (uint32_t)noTime)
	{
		BmpGetDimensions(sAlternateDeleteIconBmp, &bitmapWidth, &bitmapHeight, NULL);

		// see PrvEditDateAndTimeDraw
		deleteIconBoundsP->topLeft.x = dataBoundsP->topLeft.x
											+ dataBoundsP->extent.x
											- bitmapWidth - 1;

		deleteIconBoundsP->topLeft.y = dataBoundsP->topLeft.y
											+ (dataBoundsP->extent.y  - bitmapHeight) / 3 + 1;


		deleteIconBoundsP->extent.x = bitmapWidth;
		deleteIconBoundsP->extent.y = bitmapHeight;

		if ((width = TblGetItemInt(tableP, row, kEditDataColumn)) > 0)
			dataBoundsP->extent.x = width;
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeEnter
 *
 * DESCRIPTION: This function handles tap on Alternate Data type fields
 *				that requires to open a dialog box foe editing.
 *
 * PARAMETERS: 	->	tableP
 *				->	row
 *				->	deleteIcon: If true, selects the delete icon.
 *								Otherwise, it selected the date
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static void PrvEditDateAndTimeEnter(TableType *tableP, int16_t row, Boolean deleteIcon)
{
	if (!PrvEditDateAndTimeGetBounds(tableP, row, &sSelectedDataFieldBounds, &sSelectedDeleteIconBounds))
		return;

	sSelectedDataFieldRow = row;

    if (deleteIcon)
	{
		// delete icon selected
 		sDeleteIconSelected = true;
 		sDeleteIconSelection = true;
	}
	else
	{
		// date Selected
		sDataFieldSelected = true;
		sDataFieldSelection = true;
	}

	// retrieve fieldIndex
	sSelectedDataFieldIndex = TblGetRowID(tableP, sSelectedDataFieldRow);

	// retrieve fieldColumnId
	sSelectedDataFieldColumnId = TblGetRowData(tableP, sSelectedDataFieldRow);

	// The user tapped on the phone number, so instead of letting the
	// table handle selection, we highlight just the phone number.
	// First, deselect the row if it is selected.
	TblUnhighlightSelection(tableP);
}

 /***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeEnterPoint
 *
 * DESCRIPTION: ->	tableP
 *				->	row
 *				->	penX
 *				->	penY
 *
 * PARAMETERS: 	event: the event to handle.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision
 * PPL		07/07/03 	improve data field rectangle calculation
 *						to improve user feedback : now data fields
 *						is selected only when user tap on data text
 *						and not when he taps on itable item bounds.
 *
 ***********************************************************************/
static void PrvEditDateAndTimeEnterPoint(TableType *tableP, int16_t row, Coord penX, Coord penY)
{
	if (!PrvEditDateAndTimeGetBounds(tableP, row, &sSelectedDataFieldBounds, &sSelectedDeleteIconBounds))
		return;

	sSelectedDataFieldRow = row;

	// Are we in Within the delete icon bounds ?
	// If the pen was not put inside the item bounds let the table act
	if (RctPtInRectangle(penX, penY, &sSelectedDeleteIconBounds))
	{
		// delete icon selected
 		sDeleteIconSelected = true;
 		sDeleteIconSelection = true;
	}
	else if (RctPtInRectangle(penX, penY, &sSelectedDataFieldBounds))
	{
		// date Selected
		sDataFieldSelected = true;
		sDataFieldSelection = true;
	}

	// retrieve fieldIndex
	sSelectedDataFieldIndex = TblGetRowID(tableP, sSelectedDataFieldRow);

	// retrieve fieldColumnId
	sSelectedDataFieldColumnId = TblGetRowData(tableP, sSelectedDataFieldRow);

	// The user tapped on the phone number, so instead of letting the
	// table handle selection, we highlight just the phone number.
	// First, deselect the row if it is selected.
	TblUnhighlightSelection(tableP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimePenMove
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	event: the event to handle.

 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision.
 * PPL 		07/07/03	Now invalidate only when needed.
 *						(when data field or icon selection changed)
 *						And Invalidate only the bounds that needs to.
 *
 ***********************************************************************/
static void PrvEditDateAndTimePenMove(TableType *tableP, Coord penX, Coord penY)
{
	Boolean		selected;

	if (sDataFieldSelection)
	{
		selected = RctPtInRectangle(penX, penY, &sSelectedDataFieldBounds);

		if (sDataFieldSelected != selected)
		{
			sDataFieldSelected = selected;

			if (!sDataFieldSelected)
			{
				TblUnhighlightSelection(tableP);
			}

			ToolsFrmInvalidateRectFunc(EditView, &sSelectedDataFieldBounds, PrvEditDateAndTimeDrawTextCallback, NULL);
		 }
	}
	else if (sDeleteIconSelection)
	{
		selected = RctPtInRectangle(penX, penY, &sSelectedDeleteIconBounds);

		if (sDeleteIconSelected != selected)
		{
			sDeleteIconSelected = selected;

			if (!sDeleteIconSelected)
			{
				TblUnhighlightSelection(tableP);
			}

			ToolsFrmInvalidateRectFunc(EditView, &sSelectedDeleteIconBounds, PrvEditDateAndTimeDrawBitmapCallback, NULL);
		 }
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeSelect
 *
 * DESCRIPTION: This function handles tap on Alternate Data type fields
 *				that requires to open a dialog box foe editing.
 *
 * PARAMETERS: 	event: the event to handle.

 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvEditDateAndTimeSelect(TableType *tableP, int16_t row, int16_t column)
{
	Boolean	handled = false;

	if (sDateAndTimeStr)
	{
		MemPtrFree(sDateAndTimeStr);
		sDateAndTimeStr = NULL;
	}

	if (sDataFieldSelection)
	{
		if (sDataFieldSelected)
		{
			PrvEditDateAndTimeEdit(tableP);
			sDataFieldSelected = false;
		}

		sDataFieldSelection = false;
		handled = true;
	}
	else if (sDeleteIconSelection)
	{
		if (sDeleteIconSelected)
		{
			PrvEditDateAndTimeDelete(tableP);
			sDeleteIconSelected = false;
		}

		sDeleteIconSelection = false;

		handled = true;
	}

	if (handled)
	{
		RectangleType	itemBounds;

		TblGetItemBounds(tableP, row, column, &itemBounds);
		ToolsFrmInvalidateRect(EditView, &itemBounds);
	}

    return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDateAndTimeDiscardSelection
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date			Description
 * ----		--------		-----------
 * TEs		25/03/2004		Initial Revision
 *
 ***********************************************************************/
static void PrvEditDateAndTimeDiscardSelection(void)
{
	sDeleteIconSelected = false;
	sDeleteIconSelection = false;
	sDataFieldSelected = false;
	sDataFieldSelection = false;
	sSelectedDataFieldRow  = kTblNoSelectedRow;
	sSelectedDataFieldIndex = kNoFieldIndex;

	if (sDateAndTimeStr)
	{
		MemPtrFree(sDateAndTimeStr);
		sDateAndTimeStr = NULL;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditGetFullname
 *
 * DESCRIPTION: Stores the fullname (FFN) information into the concerned
 *				static variables (sFullnameH, sFullnameWidth and sFullnameFont)
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/01/2003	Initial revision
 *
 ***********************************************************************/
static void PrvEditGetFullname(void)
{
	FontID			font;
	char *			fullnameP;
	RectangleType	nameBounds;
	FormType *		formP;

	sFullnameH = AddrFreeFormNameBuild(gAddrDB, gCurrentRowID);

	if (!sFullnameH)
	{
		sFullnameWidth = 0;
		sFullnameFont = largeBoldFont;
		return;
	}

	fullnameP = MemHandleLock(sFullnameH);

	sFullnameFont = largeBoldFont;
	font = FntSetFont(sFullnameFont);
	sFullnameWidth = FntCharsWidth(fullnameP, strlen(fullnameP));

	formP = FrmGetFormPtr(EditView);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditViewName), &nameBounds);

	if (sFullnameWidth > nameBounds.extent.x)
	{
		sFullnameFont = stdFont;
		FntSetFont(sFullnameFont);
		sFullnameWidth = FntCharsWidth(fullnameP, strlen(fullnameP));
	}

	MemHandleUnlock(sFullnameH);

	FntSetFont(font);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditLoadDataProc
 *
 * DESCRIPTION: This routine returns a pointer to a field of the
 *              address record.  This routine is called by the table
 *              object as a callback routine when it wants to display or
 *              edit a field.
 *
 *				Called for Text edit field's cell.
 *				dbVarChar, Numeric
 *				dbVarCher uses Edit in place mechanism
 *				numeric data type are edited using a text edit
 *				but do not use edit in place.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		06/06/95	Initial Revision
 * LFe		10/29/02	Use new Data Manager & Edit in place
 *
 ***********************************************************************/
static status_t PrvEditLoadDataProc(void *tableP, int16_t row, int16_t column, Boolean editing, MemHandle *textHP, int16_t *textOffsetP, int16_t *textAllocSizeP, FieldType *fieldP)
{
	MemHandle	numStringH;
	char*		numStringP;
	char*		textP;
	void*		dataP = NULL;
	double		doubleNum;
	float		floatNum;
	uint32_t	colID;
	uint32_t 	tabIndex;
	uint32_t	dataSize;
	uint16_t	fieldIndex;
	uint8_t		uInteger;
	int8_t		integer;
	status_t	err;

	*textOffsetP	= 0;
	*textHP			= NULL;
	*textAllocSizeP	= 0;

	tabIndex = gBookInfo.currentTabIndex;

	// Retriev Columnn ID from the Table Row Data Storage
	colID = TblGetRowData(tableP, row);

	// Retrieve the record number from the Table Row ID Storage
	fieldIndex = TblGetRowID (tableP, row);

	// Editable item
	if (gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].columnDataType == dbVarChar)
	{
		if (!fieldP)
			return errNone;

		// set up edit in place
		FldSetTextColumn(fieldP, gAddrDB, gCurrentRowID, 0, colID, 0);

		// Set the autoShift attribute for the edited field (but not for email fields)
		if (editing && (gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].fieldInfo & kFieldType_Internet) == 0)
		{
			FieldAttrType	attr;

			FldGetAttributes(fieldP, &attr);
			attr.autoShift = true;
			FldSetAttributes(fieldP, &attr);
		}

		return errNone;
	}

	// second : convert date and time type and set them for edit in place
	err = DbGetColumnValue(gAddrDB, gCurrentRowID, colID, 0, (void**) &dataP, &dataSize);

	if (err >= errNone && dataP)
	{
		numStringH = MemHandleNew(kStringMaxSize);

		if (numStringH)
		{
			numStringP = MemHandleLock(numStringH);

			switch( gBookInfo.tabs[tabIndex].columnsPropertiesP[fieldIndex].columnDataType )
			{
				case dbUInt8:
					uInteger = *((uint8_t*) dataP);
					sprintf(numStringP,"%u", uInteger);
					break;

				case dbUInt16:
					sprintf(numStringP,"%u", *((uint16_t*) dataP));
					break;

				case dbUInt32:
					sprintf(numStringP,"%lu", *((uint32_t*) dataP));
					break;

				case dbUInt64:
					sprintf(numStringP, "%llu", *((uint64_t*) dataP));
					break;

				case dbInt8:
					integer = *((int8_t*) dataP);
					sprintf(numStringP,"%d", integer);
					break;

				case dbInt16:
					sprintf(numStringP,"%d", *((int16_t*) dataP));
					break;

				case dbInt32:
					sprintf(numStringP,"%ld", *((int32_t*) dataP));
					break;

				case dbInt64:
					sprintf(numStringP, "%lld", *((int64_t*) dataP));
					break;

				case dbFloat:
					floatNum = *((float*) dataP);
					sprintf(numStringP, "%f", floatNum);
					break;

				case dbDouble:
					doubleNum = *((double*) dataP);
					sprintf(numStringP, "%f", doubleNum);
					break;
			}

			dataSize = (uint32_t)strlen(numStringP) + 1;

			*textOffsetP = 0;
			*textAllocSizeP = (int16_t)dataSize;

			if (editing)
			{
				if (!sEditedFieldH)
					sEditedFieldH = MemHandleNew(dataSize);

				*textHP = sEditedFieldH;
			}
			else
			{
				if (!sNoEditedFieldH)
					sNoEditedFieldH = MemHandleNew(dataSize);

				*textHP = sNoEditedFieldH;
			}

			ErrFatalDisplayIf(*textHP == NULL, "Not enough memory");

			// Resize the handle if needed
			if (dataSize > MemHandleSize(*textHP))
			{
				if (MemHandleResize(*textHP, dataSize) < errNone)
					ErrFatalDisplay("Can't resize MemHandle");
			}

			textP = MemHandleLock(*textHP);
			memmove(textP, numStringP, dataSize);
			MemHandleUnlock(*textHP);

			MemHandleUnlock(numStringH);
			MemHandleFree(numStringH);
		}
	}

	if (dataP)
		err = DbReleaseStorage(gAddrDB, dataP);

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSaveDataProc
 *
 * DESCRIPTION: This routine saves a field of an address to the
 *              database.  This routine is called by the table
 *              object, as a callback routine, when it wants to save
 *              an item.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    true if the table needs to be redrawn
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		02/21/95	Initial Revision
 * LFe		10/29/02	Use new data manager & Edit in place
 *
 ***********************************************************************/
static Boolean PrvEditSaveDataProc(void * tableP, int16_t row, int16_t column)
{
	uint32_t		colID;
	FieldType *		fieldP;
	uint32_t		dataSize = 0;
	void*			dataP  = NULL;
	char*			textP;
	uint32_t		dbType;
	RectangleType	bounds;

	// Brute force first !
	double		doubleNum;
	float		floatNum;
	int8_t		integer;
	uint8_t		uInteger;
	int16_t		shortInt;
	uint16_t	uShortInt;
	int32_t		longInt;
	uint32_t	uLongInt;
	int64_t		longLongInt;
	uint64_t	uLongLongInt;
	MemHandle	fieldH;
	uint16_t	fieldIndex;
	size_t 		startPosition, endPosition;

	status_t	err;

	fieldIndex = TblGetRowID(tableP, row);

	switch( gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType )
	{
		case dbVarChar:
		{
			fieldP = TblGetCurrentField(tableP);

			if (!fieldP)
				return false;

			colID = TblGetRowData(tableP, row);

			// Save the cursor position of the field last edited.
			// Check if the top of the text is scroll off the top of the
			// field, if it is then redraw the field.
			if (FldGetScrollPosition(fieldP))
			{
				FldSetScrollPosition(fieldP, 0);
				sEditFieldPosition = 0;
			}
			else
			{
				FldGetSelection(fieldP, &startPosition, &endPosition);
				sEditFieldPosition = endPosition;
			}

			// Make sure there any selection is removed since we will free
			// the text memory before the callee can remove the selection.
//vsm - this call isn't needed...			FldSetSelection(fieldP, 0, 0);

			textP = FldGetTextPtr(fieldP);
			dbType = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].autoFillDbType;

			if (textP)
			{
				// Erase the column when it only contains a '\0'
				// It has been saved by the edit-in-place mechanism
				if (!*textP)
				{
					FldReleaseStorage(fieldP);
					err = DbWriteColumnValue(gAddrDB, gCurrentRowID, colID, 0, -1, NULL, 0);
					ErrNonFatalDisplayIf(err < errNone, "Can't write column");
				}
				// We've something to store and it's an AutoFilled field
				else if (dbType && (!gDeviceHasFEP || TsmGetFepMode() == tsmFepModeOff))
				{
					AutoFillLookupSave(dbType, textP);
				}
			}

			if (!gFFNColumnListResID)
				gFFNColumnListResID = ToolsGetCountryBaseResID(kU32ListResType, kAddrFFNTemplateBaseID, kAddrTemplateResIDIncrement);

			// Redraw for title if updated column a part of the Free Form Name
			if (U32ListContainsItem(gApplicationDbP, gFFNColumnListResID, NULL, colID))
			{
				FormType *		formP;
				RectangleType	bounds;

				AddrFreeFormNameFree(sFullnameH);
				PrvEditGetFullname();

				formP = FrmGetFormPtr(EditView);
				FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditViewName), &bounds);
				WinInvalidateRect(FrmGetWindowHandle(formP), &bounds);
			}

			break;
		}

		// No use default anymore, in case a new field type is added, prevent to try to manage it as a integer field
		case dbUInt8:
		case dbUInt16:
		case dbUInt32:
		case dbUInt64:
		case dbInt8:
		case dbInt16:
		case dbInt32:
		case dbInt64:
		case dbDouble:
		{
			// now handle only numeric data types

			// we get the field handle
			fieldP = TblGetCurrentField(tableP);

			if (!fieldP)
				return false;

			// We get the text and the Size
			fieldH = FldGetTextHandle(fieldP);

			if (fieldH == NULL || (textP = MemHandleLock(fieldH)) == NULL)
				return false;

			FldSetTextHandle(fieldP, NULL);

			// get the col ID
			colID = TblGetRowData(tableP, row);

			// now we have to convert the data into numeric type
			switch  (gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType )
			{
				case dbUInt8:
					uInteger = (uint8_t) StrAToI (textP);
					dataP = (void*) &uInteger;
					dataSize = sizeof(uInteger);
					break;

				case dbUInt16:
					uShortInt = (uint16_t) StrAToI (textP);
					dataP = (void*) &uShortInt;
					dataSize = sizeof(uShortInt);
					break;

				case dbUInt32:
					uLongInt = strtoul (textP, NULL, 10);
					dataP = (void*) &uLongInt;
					dataSize = sizeof(uLongInt);
					break;

				case dbUInt64:
					uLongLongInt = strtoll (textP, NULL, 10);
					dataP = (void*) &uLongLongInt;
					dataSize = sizeof(uLongLongInt);
					break;

				case dbInt8:
					integer = (int8_t) StrAToI(textP);
					dataP = (void*) &integer;
					dataSize = sizeof(integer);
					break;

				case dbInt16:
					shortInt = (int16_t) StrAToI (textP);
					dataP = (void*) &shortInt;
					dataSize = sizeof(shortInt);
					break;

				case dbInt32:
					longInt = (int32_t) StrAToI (textP);
					dataP = (void*) &longInt;
					dataSize = sizeof(longInt);
					break;

				case dbInt64:
					longLongInt = 	strtoull(textP, NULL, 10);
					dataP = (void*) &longLongInt;
					dataSize = sizeof(longLongInt);
					break;

				case dbFloat:
					floatNum = (float) strtod  (textP, NULL);
					dataP = (void*) &floatNum;
					dataSize = sizeof(floatNum);
					break;

				case dbDouble:
					doubleNum = strtod  (textP, NULL);
					dataP = (void*) &doubleNum;
					dataSize = sizeof(double);
					break;
			}

			if (dataP && dataSize)
			{
				// We write the Numeric Data
				err = DbWriteColumnValue(gAddrDB, gCurrentRowID, colID, 0, -1, dataP, dataSize);
				ErrNonFatalDisplayIf(err < errNone, "Can't write column");
			}

			// Make sure there any selection is removed since we will free
			// the text memory before the callee can remove the selection.
			FldSetSelection(fieldP, 0, 0);

			// Prevent the handle to be deleted by the field
			MemHandleUnlock(fieldH);

			// release memory allocated for fields if it has been changed by
			// someone else.
			if (fieldH != sEditedFieldH)
				MemHandleFree(fieldH);

			TblGetItemBounds(tableP, row, column, &bounds);
			ToolsFrmInvalidateRectFunc(EditView, &bounds, PrvEditDrawTableCallback, NULL);
			break;
		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDisplayNamePopup
 *
 * DESCRIPTION: Display the FFN popup
 *
 * PARAMETERS:	->	formP
 *
 * RETURNED:	true if FFN popup is needed, false otherwise
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/29/2004	Initial revision
 *
 ***********************************************************************/
static Boolean PrvEditDisplayNamePopup(FormType *formP)
{
	RectangleType	bounds;

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditViewName), &sNamePopupBounds);
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditTable), &bounds);

    // Display the popup if pen events occured within the name bounds.
	if (sFullnameWidth > sNamePopupBounds.extent.x)
	{
		sNamePopupBounds.extent.y = bounds.topLeft.y + bounds.extent.y - sNamePopupBounds.topLeft.y;

		FrmClearFocusHighlight();
		sNamePopupDisplayed = true;
		sNamePopupDisplayTime = TimGetTicks();
		gEvtGetEventTimeout = kNamePopupEventTimeOut;

		ToolsFrmInvalidateRect(EditView, &sNamePopupBounds);

		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDiscardNamePopup
 *
 * DESCRIPTION: Discard the FFN popup
 *
 * PARAMETERS:	->	update: if true, invalidate the window
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/01/2003	Initial revision
 *
 ***********************************************************************/
static void PrvEditDiscardNamePopup(Boolean update)
{
	sNamePopupDisplayed = false;
	gEvtGetEventTimeout = evtWaitForever;

	if (update)
		ToolsFrmInvalidateWindow(EditView);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditClose
 *
 * DESCRIPTION: Checks the record and saves it if it's OK
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    The view that should be switched to.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         rsf   9/20/95   Initial Revision
 *
 ***********************************************************************/
static void PrvEditClose(FormPtr formP)
{
	TableType*	tableP;

	// Make sure the field being edited is saved
	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	TblReleaseFocus(tableP);

	// If there is no data then delete the record.
	// If there is data but no name data then demand some.
	if (gCurrentRowID != dbInvalidRowID)
	{
		if (!AddressDBRecordContainsData(gCurrentRowID))
			AddressDBDeleteRecord(false);   // uniq ID wasted?  Yes. We don't care.
		else
			AddressDBCheckAndSetDisplayedPhoneColumn(gAddrDB, gCurrentRowID);
	}

	gListViewSelectThisRowID = gCurrentRowID;

	if (sBookP)
	{
		BookFree(sBookP);
		sBookP = NULL;
	}

	// Frees handles used for non edited in pace fields
	if (sNoEditedFieldH)
	{
		MemHandleFree(sNoEditedFieldH);
		sNoEditedFieldH = NULL;
	}

	if (sEditedFieldH)
	{
		MemHandleFree(sEditedFieldH);
		sEditedFieldH = NULL;
	}

	if (sAlternateDeleteIconH)
	{
		DmHandleUnlock(sAlternateDeleteIconH);
		DmReleaseResource(sAlternateDeleteIconH);
	}

	if (sAlternateDeleteHighlightedIconH)
	{
		DmHandleUnlock(sAlternateDeleteHighlightedIconH);
		DmReleaseResource(sAlternateDeleteHighlightedIconH);
	}

	if (sNoDataStr)
	{
		MemPtrFree(sNoDataStr);
		sNoDataStr = NULL;
	}

	if (sYomiTextP)
	{
		MemPtrFree(sYomiTextP);
		sYomiTextP = NULL;
	}

	AddrFreeFormNameFree(sFullnameH);
	sFullnameH = NULL;

	// Switch to Interaction navigation mode
	FrmSetNavState(FrmGetFormPtr(EditView), kFrmNavStateFlagsInteractionMode);

	if (!sDuplicateRecord)
		gNumCharsToHilite = 0;

	sDuplicateRecord = false;

	PrvEditDiscardNamePopup(false);

	// Change category only if the category in ListView is other than "All"
	if (sRowCategoryChanged && gCurrentRowID != dbInvalidRowID &&
		(!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll))
	{
		uint32_t		numCategories = 0;
		CategoryID *	categoriesP = NULL;

		DbGetCategory(gAddrDB, gCurrentRowID, &numCategories, &categoriesP);
		// ToolsChangeCategory will re-open the cursor
		ToolsChangeCategory(categoriesP, numCategories);

		if (categoriesP)
			DbReleaseStorage(gAddrDB, categoriesP);
	}
	else
	{
		// Re-open the cursor
		AddressDBOpenCursor();
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories from the "Edit View".
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    The index of the new category.
 *
 *              The following global variables are modified:
 *                     gCurrentCategory
 *                     gShowAllCategories
 *                     gCategoryName
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		06/05/95	Initial Revision
 * gap		08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static void PrvEditSelectCategory (FormType *formP)
{
	Boolean		categoryChanged;
	CategoryID*	categoriesP = NULL;
	uint32_t	numCategories = 0;
	CategoryID*	newCategoriesP = NULL;
	uint32_t	newNumCategories = 0;
	FormType*	frmP = NULL;
	status_t	err;

	frmP = FrmGetFormPtr(EditView);

	// Get current record categories
	DbGetCategory(gAddrDB, gCurrentRowID, &numCategories, &categoriesP);

	// Process the category popup list.
	categoryChanged = CatMgrSelectEdit(gAddrDB, frmP,
										EditCategoryTrigger, gCategoryName, EditCategoryList, true,
                                        categoriesP, numCategories,
										&newCategoriesP, &newNumCategories,
										true, NULL);

	if (categoryChanged)
	{
		err = DbSetCategory(gAddrDB, gCurrentRowID, newNumCategories, newCategoriesP);
		ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");

		// Resize the trigger
		PrvEditSetCategoryTriggerLabel(frmP);

		// Redraw the business card indicator in case the
		// previous category label was overlapping it.
		PrvEditDrawBusinessCardIndicator(frmP);

		// Global categories will be updated at frmClose
		sRowCategoryChanged = true;
	}

	if (categoriesP)
		DbReleaseStorage(gAddrDB, categoriesP);

	if (newCategoriesP)
		CatMgrFreeSelectedCategories(gAddrDB, &newCategoriesP);
}

/********************************************************************
 *
 * FUNCTION:    PrvEditGetNumericFieldHeight
 *
 * DESCRIPTION: This routine initialize a Numeric row in the Address
 *				edit view List. Actually a text field with no edit in place.
 *
 * PARAMETERS:  table        - pointer to the table of to do items
 *              fieldIndex   - the index of the field displayed in the row
 *              columnWidth  - height of the row in pixels
 *
 * RETURNED:    height of the field in pixels
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * PPL		12/13/02	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvFilterNumericFields (FormType *frmP, EventPtr  event)
{
	TablePtr	tableP;
	FieldType * fldP;
	uint16_t 	fieldIndex;
	int16_t 	row;
	int16_t 	column;
	Boolean 	handled = true;
	size_t		pos;

	if (!TxtCharIsPrint(event->data.keyDown.chr) ||	FrmGetFocus(frmP) == noFocus)
		return false;

	// Find the table
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);

	// check to see if there really is a field before continuing.
	// in case table has stopped editing but form still thinks table is the focus.
	if ((fldP = TblGetCurrentField(tableP)) == NULL)
		return false;

	// Get the field number that corresponds to the table item to save.
	TblGetSelection(tableP, &row, &column);
	fieldIndex = TblGetRowID(tableP, row);
	pos = FldGetInsPtPosition(fldP);

	// we need numeric filtering
	// WARNING: USE FALLBACK. DON'T ADD BREAK
	switch (gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnDataType)
	{
		// signs:		- + , .
		// only digits:	0,1 2, 3, 4 , 5 , 6 , 7 , 8, 9
		case dbFloat:
		case dbDouble:
			handled = (Boolean) ((event->data.keyDown.chr != chrComma) && (event->data.keyDown.chr != chrFullStop));

		// signs:		- + only as first char
		// no signs:	, .
		// only digits:	0,1 2, 3, 4 , 5 , 6 , 7 , 8, 9
		case dbInt8:
		case dbInt16:
		case dbInt32:
		case dbInt64:
			handled &=	((event->data.keyDown.chr != chrHyphenMinus) && (event->data.keyDown.chr != chrPlusSign)) ||
						(((event->data.keyDown.chr == chrHyphenMinus) || (event->data.keyDown.chr == chrPlusSign)) && pos);

		// no signs:	- + , .
		// only digits:	0,1 2, 3, 4 , 5 , 6 , 7 , 8, 9
		case dbUInt8:
		case dbUInt16:
		case dbUInt32:
		case dbUInt64:
			handled &= !TxtCharIsDigit(event->data.keyDown.chr);
			break;

		// Other field kind accept any char.
		default:
			handled = false;
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditRedrawTableCallback
 *
 * DESCRIPTION: Callback used by WinIvalidateRectFunc. It only redraws the table
 *
 * PARAMETERS:
 *
 * RETURNED:    Always true
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		10/21/2003	Initial draft
 *
 ***********************************************************************/
static Boolean PrvEditRedrawTableCallback(int32_t cmd, WinHandle winH, const RectangleType *diryRectP, void *stateP)
{
	FormType *	formP;
	TableType *	tableP;

	if (cmd != winInvalidateDestroy)
	{
		formP = FrmGetFormPtr(EditView);
		tableP = ToolsGetFrmObjectPtr(formP, EditTable);
		TblRedrawTable(tableP);
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDiscardFocus
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		12/23/2003	Initial Revision
 *
 ***********************************************************************/
static void PrvEditDiscardFocus(FormType *formP)
{
	TablePtr	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	TblReleaseFocus(tableP);
	FrmSetFocus(formP, noFocus);
	gNumCharsToHilite = 0;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditResizeDescription
 *
 * DESCRIPTION: This routine is called when the height of address
 *              field is changed as a result of user input.
 *              If the new height of the field is shorter, more items
 *              may need to be added to the bottom of the list.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/26/95      Initial Revision
 *
 ***********************************************************************/
static void PrvEditResizeDescription(EventType * event)
{
	FieldType *		fieldP;
	TableType *		tableP;
	RectangleType	bounds;
	FormType *		frmP;
	int16_t			row, col;

	frmP = FrmGetFormPtr(EditView);
	// Get the current height of the field;
	fieldP = event->data.fldHeightChanged.pField;
	FldGetBounds(fieldP, &bounds);

	// Have the table object resize the field and move the items below
	// the field up or down.
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
	TblHandleEvent(tableP, event);

	// If the field's height has expanded.
	if (event->data.fldHeightChanged.newHeight >= bounds.extent.y)
	{
		// Get the current selection (will be restored later)
		TblGetSelection(tableP, &row, &col);
		sCurrentFieldIndex = TblGetRowID(tableP, row);
	}

	PrvEditDiscardFocus(frmP);
	sEditFieldPosition = event->data.fldHeightChanged.currentPos;

	// Add items to the table to fill in the space made available by the
	// shorting the field.
	PrvEditLoadTable(frmP, false);

	// Redraw the table
	TblGetBounds(tableP, &bounds);
	ToolsFrmInvalidateRectFunc(EditView, &bounds, PrvEditRedrawTableCallback, NULL);
	PrvEditRestoreEditState(frmP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditScroll
 *
 * DESCRIPTION: This routine scrolls the list of editable fields
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *              oneLine   - if true the list is scroll by a single line,
 *                          if false the list is scroll by a full screen.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   06/26/95	Initial Revision
 *         vmk   02/20/98	Move TblUnhighlightSelection before PrvEditLoadTable
 *         gap   10/12/99	Close command bar before processing scroll
 *
 ***********************************************************************/
static Boolean PrvEditScroll(FormType *frmP, WinDirectionType direction)
{
	uint16_t		row;
	uint32_t		height;
	uint16_t		fieldIndex;
	uint16_t		columnWidth;
	uint16_t		tableHeight;
	TableType *		tableP;
	FontID			curFont;
	RectangleType	bounds;
	uint32_t		columnID;
	char *			textP;
	uint32_t		dataSize;
	Boolean			scrolled = true;
	uint16_t		savedTopItem;

	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus (0);

	curFont = FntSetFont (gAddrEditFont);

	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
	TblReleaseFocus (tableP);

	// Get the height of the table and the width of the description
	// column.
	TblGetBounds(tableP, &bounds);
	tableHeight = bounds.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth (tableP, kEditDataColumn);

	// Scroll the table down.
	if (direction == winDown)
	{
		// Get the index of the last visible field, this will become
		// the index of the top visible field, unless it occupies the
		// whole screeen, in which case the next field will be the
		// top filed.

		row = TblGetLastUsableRow (tableP);
		fieldIndex = TblGetRowID (tableP, row);

		// If the last visible field is also the first visible field
		// then it occupies the whole screeen.
		if (row == 0)
			fieldIndex = min (sEditLastFieldIndex, fieldIndex + 1);
	}

	// Scroll the table up.
	else
	{
		// Scan the fields before the first visible field to determine
		// how many fields we need to scroll.  Since the heights of the
		// fields vary,  we sum the height of the records until we get
		// a screen full.

		fieldIndex = TblGetRowID (tableP, 0);
		ErrFatalDisplayIf(fieldIndex > sEditLastFieldIndex, "Invalid field Index");
		if (fieldIndex == 0)
		{
			scrolled = false;
			goto exit;
		}

		height = TblGetRowHeight (tableP, 0);
		if (height >= tableHeight)
			height = 0;

		while (height < tableHeight && fieldIndex > 0)
		{
			textP = NULL;
			columnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId;
			DbGetColumnValue(gAddrDB, gCurrentRowID, columnID, 0, (void**)&textP, &dataSize);

			height += FldCalcFieldHeight(textP, columnWidth) * FntLineHeight();
			if ((height <= tableHeight) || (fieldIndex == TblGetRowID(tableP, 0)))
				fieldIndex--;

			if (textP)
				DbReleaseStorage(gAddrDB, textP);
		}
	}

	savedTopItem = sTopVisibleFieldIndex;
	TblMarkTableInvalid (tableP);
	sCurrentFieldIndex = kNoFieldIndex;
	sTopVisibleFieldIndex = fieldIndex;
	sEditFieldPosition = 0;

	TblUnhighlightSelection (tableP);

	PrvEditLoadTable(frmP, false);

	// Check here if we were already at bottom when scrolling down
	if (direction == winDown && sTopVisibleFieldIndex == savedTopItem)
		scrolled = false;

	// Avoid redraw if not needed
	if (scrolled)
	{
		// bounds contains the table bounds
		ToolsFrmInvalidateRectFunc(EditView, &bounds, PrvEditDrawTableCallback, NULL);

		// Update the scroller buttons
		PrvEditUpdateScrollers(frmP);
	}

exit:
	FntSetFont (curFont);
	return scrolled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditNextField
 *
 * DESCRIPTION: If a field is being edited, advance the focus to the
 * edit view table's next field.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the focus has moved, false otherwise
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   7/27/95   Initial Revision
 *
 ***********************************************************************/
static Boolean PrvEditNextField(FormType *frmP, WinDirectionType direction, Boolean skipNonEditable, Boolean allowCycle)
{
	TablePtr		tableP;
	int16_t			row;
	int16_t			column;
	uint16_t		newFieldNumIndex;
	uint16_t		currentFieldNumIndex;
	Boolean			inRealField;
	RectangleType	tableBounds;
	RectangleType	bounds;

	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);

	// If the table is not in edit or if there is no field editable in this tab
	// don't try to find a valid field.
	if (skipNonEditable &&
		(!TblEditing(tableP) || !gBookInfo.tabs[gBookInfo.currentTabIndex].editableTextField))
		return false;

	// Find out which field is being edited.
	TblGetSelection (tableP, &row, &column);
	currentFieldNumIndex = newFieldNumIndex = TblGetRowID (tableP, row);

	// when a current field is a real field, skip YOMI field.
	do
	{
		inRealField = !ToolsIsYomiFieldByIndex(gBookInfo.currentTabIndex, newFieldNumIndex);

		if (direction == winDown)
		{
            if (newFieldNumIndex >= sEditLastFieldIndex)
				newFieldNumIndex = 0;
			else
				newFieldNumIndex++;
		}
		else
		{
			if (newFieldNumIndex == 0)
				newFieldNumIndex = sEditLastFieldIndex;
			else
				newFieldNumIndex--;
		}

	} while ((inRealField && ToolsIsYomiFieldByIndex(gBookInfo.currentTabIndex, newFieldNumIndex)) ||
			(skipNonEditable && !ToolsIsTextFieldByIndex(gBookInfo.currentTabIndex, newFieldNumIndex)));

	// Top/Bottom cycle not allowed
	if (!allowCycle &&
		((direction == winDown && newFieldNumIndex < currentFieldNumIndex) ||
		(direction == winUp && newFieldNumIndex > currentFieldNumIndex)))
		return false;

	TblReleaseFocus (tableP);
	TblUnhighlightSelection(tableP);

	TblFindRowID(tableP, currentFieldNumIndex, &row);
	if (TblGetItemStyle(tableP, row, kEditDataColumn) == tallCustomTableItem)
	{
		PrvEditDateAndTimeDiscardSelection();
		TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);
		PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
	}

	// If the new field isn't visible move the edit view and then
	// find the row where the next field is.
	while (!TblFindRowID(tableP, newFieldNumIndex, &row))
	{
		// First, reset the current row as the first row in the table.
		TblSetSelection (tableP, 0, kEditDataColumn);

		// Scroll the view down placing the item on the top row
		sTopVisibleFieldIndex = newFieldNumIndex;
		sCurrentFieldIndex = kNoFieldIndex;
		PrvEditLoadTable(frmP, false);
		TblGetBounds(tableP, &tableBounds);
		ToolsFrmInvalidateRectFunc(EditView, &tableBounds, PrvEditDrawTableCallback, NULL);
	}

	TblSetSelection(tableP, row, kEditDataColumn);

	if (ToolsIsTextFieldByIndex(gBookInfo.currentTabIndex, newFieldNumIndex))
	{
		PrvEditHandleSelectField(frmP, tableP, row, kEditDataColumn);
	}
	else if (TblGetItemStyle(tableP, row, kEditDataColumn) == tallCustomTableItem)
	{
		sCurrentFieldIndex = TblGetRowID(tableP, row);
		TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);
		PrvEditDateAndTimeEnter(tableP, row, false);
		PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSaveSelection
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		04/04/2003	Initial revision
 *
 ***********************************************************************/
static void PrvEditSaveSelection(TablePtr tableP, int16_t row, int16_t column)
{
	FieldType *	fieldP;
	size_t		startPosition, endPosition;

	if (column != kEditDataColumn || (fieldP = TblGetCurrentField(tableP)) == NULL)
		return;

	FldGetSelection(fieldP, &startPosition, &endPosition);

	if (startPosition < endPosition)
	{
		gNumCharsToHilite = (uint16_t) (endPosition - startPosition);
		// Save the position of the insertion point
		sEditFieldPosition = endPosition;
		sHilitedFieldIndex = TblGetRowID(tableP, row);
	}
	else
	{
		gNumCharsToHilite = 0;
		sEditFieldPosition = 0;
		sHilitedFieldIndex = kNoFieldIndex;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditRestoreSelection
 *
 * DESCRIPTION: This routine highlites the last gNumCharsToHilite characters
 *				of the field located in the sHilitedFieldIndex row of the table
 *
 * PARAMETERS:	frmP	- Pointer to the Edit form structure
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		04/04/2003	Initial revision
 *
 ***********************************************************************/
static void PrvEditRestoreSelection(FormType *formP)
{
	TableType *	tableP;
	FieldType *	fldP;
	int16_t		row;

	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	if (!gNumCharsToHilite || sHilitedFieldIndex != sCurrentFieldIndex || !TblFindRowID(tableP, sCurrentFieldIndex, &row))
	{
		sHilitedFieldIndex = kNoFieldIndex;
		gNumCharsToHilite = 0;
		return;
	}

	// Set the focus on the field
	FrmSetFocus(formP, FrmGetObjectIndex(formP, EditTable));
	TblGrabFocus(tableP, row, kEditDataColumn);

	fldP = TblGetCurrentField(tableP);

	if (fldP)
	{
		uint32_t	editFieldPosition = FldGetTextLength(fldP);

		// If gNumCharsToHilite is not 0, then we know that we are displaying
		// a duplicated message for the first time and we must hilite the last
		// gNumCharsToHilite of the field (first name) to indicate the modification
		// to that duplicated field.
		if (editFieldPosition >= gNumCharsToHilite)
		{
			// Now hilite the chars added.
			FldSetInsertionPoint(fldP, sEditFieldPosition);
			FldSetSelection(fldP, (uint16_t)(sEditFieldPosition - gNumCharsToHilite), sEditFieldPosition);
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	06/27/95	Initial Revision
 * jmp		09/17/99	Use NoteView instead of NoteView.
 * aro		06/27/00	Add dialing
 * LFe		10/24/02	Use new Database Manager
 *
 ***********************************************************************/
static Boolean PrvEditDoCommand (FormType *frmP, uint16_t command)
{
	Boolean		handled = false;
	Boolean		deleteIt;
	uint32_t	newRowIDP;
	uint16_t	numCharsToHilite;
	uint32_t	columnIDToHilite;
	FontID		newFont;
	TablePtr	tableP;
	FieldPtr	fieldP;
	int16_t 	row, col;

	switch (command)
	{
		case EditRecordDeleteRecordCmd:
			MenuEraseStatus (0);
			tableP = (TableType*)ToolsGetFrmObjectPtr(frmP, EditTable);
			fieldP = TblGetCurrentField(tableP);

			if (fieldP)
			{
				// Save the selection, if any
				TblGetSelection(tableP, &row, &col);
				PrvEditSaveSelection(tableP, row, col);
			}

			TblReleaseFocus(tableP);
			FrmSetFocus(frmP, noFocus);   // save the field

			EditViewSetFocusable(false);
			deleteIt = ToolsConfirmDeletion();
			EditViewSetFocusable(true);

			if (deleteIt)
			{
				AddressDBDeleteRecord(gSaveBackup);
				FrmGotoForm (gApplicationDbP, ListView);
			}
			else
			{
				// Restore the selection, if any
				PrvEditRestoreSelection(frmP);
				// restore the current field and insertion point position if no selection previously restored.
				sRestoreEditState = (Boolean) (gNumCharsToHilite == 0);
			}

			handled = true;
			break;

		case EditRecordDuplicateAddressCmd:
			MenuEraseStatus (0);
			// Make sure the field being edited is saved
			PrvEditDiscardFocus(frmP);
			AddressDBDuplicateRecord(gCurrentRowID, &newRowIDP, &numCharsToHilite, &columnIDToHilite);

			if (newRowIDP != dbInvalidRowID)
			{
				// Save the Default Phone for the current record. It could be a new one.
				// As the default is set in the frmCloseEvent based on the gCurrentRowID, it will
				// be too late for the duplicated record, as we have change the gCurrentRowID globals.
				AddressDBCheckAndSetDisplayedPhoneColumn(gAddrDB, gCurrentRowID);

				gNumCharsToHilite = numCharsToHilite;
				gCurrentRowID = newRowIDP;
				gTappedColumnID = columnIDToHilite;
				AddressTabFindColIDTab(&gBookInfo, gTappedColumnID, &gBookInfo.currentTabIndex, &gBookInfo.currentTabId);
				sDuplicateRecord = true;
				FrmGotoForm (gApplicationDbP, EditView);
			}

			handled = true;
			break;

		case EditRecordDialCmd:
			MenuEraseStatus (0);
			PrvEditDialCurrent();
			handled = true;
			break;

		case EditRecordAttachNoteCmd:
			MenuEraseStatus (0);
			tableP = (TableType*)ToolsGetFrmObjectPtr(frmP, EditTable);
			fieldP = TblGetCurrentField(tableP);

			if (fieldP)
			{
				// Save the selection, if any
				TblGetSelection(tableP, &row, &col);
				PrvEditSaveSelection(tableP, row, col);
			}

			FrmSetFocus(frmP, noFocus);   // save the field

			EditViewSetFocusable(false);
			AddressEditNote(gCurrentRowID);
			EditViewSetFocusable(true);

			if (fieldP)
			{
				// Restore the selection, if any
				PrvEditRestoreSelection(frmP);
				// restore the current field and insertion point position if no selection previously restored.
				sRestoreEditState = (Boolean) (gNumCharsToHilite == 0);
			}

			handled = true;
			break;

		case EditRecordDeleteNoteCmd:
			MenuEraseStatus (0);

			if (AddressExistingNote(gCurrentRowID) && FrmAlert(gApplicationDbP, DeleteNoteAlert) == DeleteNoteYes)
			{

				tableP = (TableType*)ToolsGetFrmObjectPtr(frmP, EditTable);
				fieldP = TblGetCurrentField(tableP);

				if (fieldP)
				{
					// Save the selection, if any
					TblGetSelection(tableP, &row, &col);
					PrvEditSaveSelection(tableP, row, col);
				}

				FrmSetFocus(frmP, noFocus);   // save the field

				AddressDeleteNote(gCurrentRowID);

				if (fieldP)
				{
					// Restore the selection, if any
					PrvEditRestoreSelection(frmP);
					// restore the current field and insertion point position if no selection previously restored.
					sRestoreEditState = (Boolean) (gNumCharsToHilite == 0);
				}
			}

			handled = true;
			break;

		case EditRecordSelectBusinessCardCmd:
			MenuEraseStatus (0);

			if (FrmAlert(gApplicationDbP, SelectBusinessCardAlert) == SelectBusinessCardYes)
			{
				gBusinessCardRowID = gCurrentRowID;
				PrvEditSetCategoryTriggerLabel(frmP);
				PrvEditDrawBusinessCardIndicator(frmP);
			}

			handled = true;
			break;

		case EditRecordBeamBusinessCardCmd:
			MenuEraseStatus (0);
			// Make sure the field being edited is saved
			PrvEditDiscardFocus(frmP);

			MenuEraseStatus (0);
			ToolsAddrBeamBusinessCard();
			handled = true;
			break;

		case EditRecordAttachRecordCmd:
			// Make sure the field being edited is saved
			PrvEditDiscardFocus(frmP);

			MenuEraseStatus (0);
			ToolsAddrAttachRecord(gAddrDB, gCurrentRowID);
			handled = true;
			break;

		case EditRecordBeamRecordCmd:
			// Make sure the field being edited is saved
			PrvEditDiscardFocus(frmP);

			MenuEraseStatus (0);
			TransferSendRecord(gAddrDB, gCurrentRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(gBookInfo.currentTabId));
			handled = true;
			break;

		case EditRecordSendRecordCmd:
			// Make sure the field being edited is saved
			PrvEditDiscardFocus(frmP);

			MenuEraseStatus (0);
			TransferSendRecord(gAddrDB, gCurrentRowID, exgSendPrefix, NoDataToSendAlert, AddressExportGetFamily(gBookInfo.currentTabId));
			handled = true;
			break;

		case EditOptionsFontCmd:
			MenuEraseStatus (0);
			newFont = ToolsSelectFont (EditView, gAddrEditFont);

			if (newFont != gAddrEditFont)
			{
				gAddrEditFont = newFont;
				PrvEditLoadTable(frmP, true);
			}

			handled = true;
			break;

		case EditOptionsEditCustomFldsCmd:
			MenuEraseStatus (0);
			PrvEditDiscardFocus(frmP);
			FrmPopupForm (gApplicationDbP, CustomEditDialog);
			handled = true;
			break;

		case EditOptionsAboutCmd:
			MenuEraseStatus (0);
			AbtShowAbout (sysFileCAddress);
			handled = true;
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    RecordViewAutoFill
 *
 * DESCRIPTION: This routine handles auto-filling the vendor or city
 *              fields.
 *
 * PARAMETERS:  event  - pointer to a keyDownEvent.
 *
 * RETURNED:    true if the event has been handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	01/15/96		Initial Revision
 *			trm	07/20/97		Fixed Autofill bug
 *			kwk	11/20/98		Check for command key, return false if set.
 *			CSS	06/22/99		Standardized keyDownEvent handling
 *									(TxtCharIsHardKey, commandKeyMask, etc.)
 *			bhall	07/12/99		Modified from Expense.c/DetailsAutoFill
 *			bob	11/17/99		double check that table has a field before using it
 *
 ***********************************************************************/
static Boolean PrvEditAutoFill(FormType *formP, EventType *eventP)
{
	uint16_t			index;
	char *				keyP;
	uint32_t			dbType;
	FieldType *			fieldP;
	DmOpenRef			dbP;
	TableType *			tableP;
	uint16_t	 		fieldIndex;
	size_t				pos;
	int16_t				row;
	int16_t				column;
	MemHandle			recH;
	LookupRecordType *	recP;
	size_t				len;
	char *				strP;

	if (TxtCharIsHardKey(eventP->data.keyDown.modifiers, eventP->data.keyDown.chr) ||
		EvtKeydownIsVirtual(eventP) ||
		!TxtCharIsPrint(eventP->data.keyDown.chr) ||
		FrmGetFocus(formP) == noFocus ||
		(gDeviceHasFEP && TsmGetFepMode() != tsmFepModeOff))
		return false;

	// Find the table
	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	// check to see if there really is a field before continuing.
	// in case table has stopped editing but form still thinks table is the
	// focus.
	if (TblGetCurrentField(tableP) == NULL)
		return false;

	// Get the field number that corresponds to the table item to save.
	TblGetSelection(tableP, &row, &column);
	fieldIndex = TblGetRowID(tableP, row);
	if (fieldIndex == kNoFieldIndex)
		return false;
	dbType = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].autoFillDbType;

	// Let the OS insert the character into the field.
	FrmHandleEvent(formP, eventP);

	// The current value of the field with the focus.
	fieldP = TblGetCurrentField(tableP);
	keyP = FldGetTextPtr(fieldP);
	pos = FldGetInsPtPosition(fieldP);

	// Only auto-fill if the insertion point is at the end.
	if (pos != FldGetTextLength(fieldP))
		return true;

	// Open the database
	if (!dbType || (dbP = DmOpenDatabaseByTypeCreator(dbType, sysFileCAddress, dmModeReadOnly)) == NULL)
		return true;

	// Check for a match.
	if (AutoFillLookupStringInDatabase(dbP, keyP, &index))
	{
		recH = DmQueryRecord(dbP, index);
		recP = MemHandleLock(recH);

		// Auto-fill.
		strP = &recP->text + strlen(keyP);
		len = strlen(strP);

		FldInsert(fieldP, strP, strlen(strP));

		// Highlight the inserted text.
		FldSetSelection(fieldP, pos, pos + len);

		MemHandleUnlock(recH);
	}

	// Close the database
	DmCloseDatabase(dbP);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSetTableColmunWidth
 *
 * DESCRIPTION: Set table column width depending on their content
 *
 * PARAMETERS:  tableP, tabIndex
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		02/25/03	Initial revision
 *
 ***********************************************************************/
static void PrvEditSetTableColmunWidth(TableType *tableP, uint16_t tabIndex)
{
	RectangleType	tableBounds;
	uint16_t		labelColumnWidth;
	uint16_t		dataColumnWidth;

	TblGetBounds(tableP, &tableBounds);

	// Get label coluln width
	labelColumnWidth = ToolsGetLabelColumnWidth(gCurrentRowID, &gBookInfo, tabIndex, stdFont, false);

	if (labelColumnWidth > tableBounds.extent.x >> 1)
		labelColumnWidth = tableBounds.extent.x >> 1;

	// Compute the width of the data column, account for the table column gutter.
	dataColumnWidth = tableBounds.extent.x - kSpaceBeforeDesc - labelColumnWidth;

	// Set table column width
	TblSetColumnWidth(tableP, kEditLabelColumn, labelColumnWidth);
	TblSetColumnWidth(tableP, kEditDataColumn, dataColumnWidth);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditBookActivation
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static void PrvEditBookActivation(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData)
{
	FormType *		formP;
	TableType *		tableP;

	formP = FrmGetFormPtr(EditView);
	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	gBookInfo.currentTabId = tabId;
	gBookInfo.currentTabIndex = tabIndex;

	sTopVisibleFieldIndex = 0;
	sCurrentFieldIndex = kNoFieldIndex;

	sEditLastFieldIndex = gBookInfo.tabs[tabIndex].numElement - 1;

	PrvEditSetTableColmunWidth(tableP, tabIndex);

	PrvEditLoadTable(formP, false);

	// Update the scroll buttons.
	PrvEditUpdateScrollers(formP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditBookDeactivation
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static Boolean PrvEditBookDeactivation(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData)
{
	TablePtr	tableP = ToolsGetFrmObjectPtr(bookP->formP, EditTable);
	int16_t		tableRowCount = TblGetNumberOfRows(tableP);
	int16_t		row;

	for(row = 0; row < tableRowCount; row++)
	{
		TblSetRowUsable(tableP, row, false);
		TblMarkRowInvalid(tableP, row);
		TblSetRowID (tableP, row, kNoFieldIndex);
		TblSetRowData(tableP, row, kInvalidColumnID);
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSetTblItemFocusRing
 *
 * DESCRIPTION:	Draw the focus ring around the edited row
 *
 * PARAMETERS:
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/22/04	Initial revision
 *
 ***********************************************************************/
static void PrvEditSetTblItemFocusRing(TableType *tableP, int16_t row)
{
	RectangleType	bounds;
	Coord			bitmapWidth;

	TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);

	if (TblGetItemStyle(tableP, row, kEditDataColumn) == tallCustomTableItem &&
		(int32_t)TblGetItemPtr(tableP, row, kEditDataColumn) != noTime &&
		sAlternateDeleteIconBmp)
	{
		BmpGetDimensions(sAlternateDeleteIconBmp, &bitmapWidth, NULL, NULL);

		if (sDataFieldSelection)
		{
			bounds.extent.x -= bitmapWidth + 4;
		}
		else if (sDeleteIconSelection)
		{
			bounds.topLeft.x = bounds.topLeft.x + bounds.extent.x - bitmapWidth - 4;
			bounds.extent.x = bitmapWidth + 2;
		}
	}

	FrmSetFocusHighlight(FrmGetWindowHandle(FrmGetFormPtr(EditView)), &bounds, 0);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditFormResize
 *
 * DESCRIPTION: Update the display and handles resizing
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name   	Date      	Description
 * ----   	--------	-----------
 * ppl		02/06/02	Initial release - Active InputArea Support
 *
 ***********************************************************************/
static void PrvEditFormResize(FormType *frmP, RectangleType *newBoundsP)
{
	RectangleType 	objBounds;
	RectangleType	cellBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	uint16_t		fontHeight;
	uint16_t		tableMaxY;
	uint16_t		dataHeight;
	uint16_t		numberOfRows;
	int16_t			lastVisibleRow 	= kTblInvalidRowIndex;
	int16_t			row;
	int16_t			selectedRow 	= kTblInvalidRowIndex;
	int16_t			selectedcolumn 	= kTblInvalidRowIndex;
	int16_t			xOffset;
	int16_t			yOffset;
	FieldType*		fieldP;
	TableType*		tableP = NULL;
	Boolean			selection;
	Boolean			visible = false;

	xOffset = newBoundsP->extent.x - gCurrentWinBounds.extent.x;
	yOffset = newBoundsP->extent.y - gCurrentWinBounds.extent.y;

	if (!xOffset && !yOffset)
	{
		// if the window has the height of the screen
		// then the windows is already at the right size
		goto Exit;
	}

	// Book
	BookResizeBody(sBookP, xOffset, yOffset);

	// EditTable
	objIndex =  FrmGetObjectIndex (frmP, EditTable);
	tableP = (TableType*) FrmGetObjectPtr( frmP, objIndex);

	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	fontHeight = FntLineHeight();
	objBounds.extent.x += xOffset;
	objBounds.extent.y += yOffset;
	selection = TblGetSelection(tableP, &selectedRow, &selectedcolumn);

	if (selectedRow != kTblInvalidRowIndex && selectedcolumn != kTblInvalidRowIndex)
		PrvEditSaveSelection(tableP, selectedRow, selectedcolumn);

	fieldP = TblGetCurrentField(tableP);

	TblUnhighlightSelection(tableP);
	TblReleaseFocus(tableP);
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	numberOfRows = TblGetNumberOfRows(tableP);

	if( (selectedRow != kTblInvalidRowIndex) && (selectedRow >= 0) && (selectedRow < numberOfRows))
	{
		tableMaxY = objBounds.extent.y;

		for (row = 0, dataHeight = 0; row < numberOfRows; row++)
		{
			TblGetItemBounds(tableP, row, kEditDataColumn, &cellBounds);

			dataHeight += max (fontHeight, cellBounds.extent.y);

			if (dataHeight <= tableMaxY)
			{
				lastVisibleRow = row;
				visible = true;
			}
			else
			{
				visible = false;
			}

			TblSetRowUsable(tableP, row, visible);
		}
	}

	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	if (xOffset)
	{
		// Resize table columns
		PrvEditSetTableColmunWidth(tableP, gBookInfo.currentTabIndex);

		// EditViewName
		objIndex = FrmGetObjectIndex (frmP, EditViewName);
		FrmGetObjectBounds(frmP, objIndex, &objBounds);
		objBounds.extent.x += xOffset;
		FrmSetObjectBounds(frmP, objIndex, &objBounds);

		// EditCategoryTrigger
		objIndex = FrmGetObjectIndex (frmP, EditCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += xOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);
		PrvEditSetCategoryTriggerLabel(frmP);

		// EditCategoryList
		objIndex = FrmGetObjectIndex (frmP, EditCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += xOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	if (yOffset)
	{
		// EditNoteButton
		objIndex = FrmGetObjectIndex (frmP, EditNoteButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// EditDetailsButton
		objIndex = FrmGetObjectIndex (frmP, EditDetailsButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// EditDoneButton
		objIndex = FrmGetObjectIndex (frmP, EditDoneButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += yOffset;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	// EditUpButton
	objIndex = FrmGetObjectIndex (frmP, EditUpButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += xOffset;
	y += yOffset;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// EditDownButton
	objIndex = FrmGetObjectIndex (frmP, EditDownButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += xOffset;
	y += yOffset;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// GSI  is not always the last item in form
	objIndex =  ToolsGetGraffitiObjectIndex(frmP);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += xOffset;
	y += yOffset;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// BackgroundGadget
	objIndex =  FrmGetObjectIndex (frmP, EditBackgroundGadget);
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x += xOffset;
	objBounds.extent.y += yOffset;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	sGadgetWhiteArea.extent.x += xOffset;
	sGadgetWhiteArea.extent.y += yOffset;

	// if yOffset < 0 we are about to scroll up table fields
	// located in the extended input area space
	if (yOffset < 0 && fieldP != NULL && selectedRow != kTblInvalidRowIndex &&
		lastVisibleRow != kTblInvalidRowIndex && selectedRow > lastVisibleRow)
	{
		sTopVisibleFieldIndex +=  (selectedRow - lastVisibleRow + 1);

		if (sTopVisibleFieldIndex > numberOfRows)
			sTopVisibleFieldIndex = 0;
	}

	gCurrentWinBounds = *newBoundsP;

Exit:

	PrvEditLoadTable(frmP, false);
	PrvEditRestoreSelection(frmP);
	sRestoreEditState = (Boolean) (gNumCharsToHilite == 0);

	// Redraw focus ring
	if (tableP && sOnehandedFieldFocused && TblFindRowID(tableP, sCurrentFieldIndex, &row))
	{
		sSelectedDataFieldRow = row;
		PrvEditSetTblItemFocusRing(tableP, row);
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditInit
 *
 * DESCRIPTION: This routine initializes the "Edit View" of the
 *              Address application.
 *
 * PARAMETERS:	frmP	- Pointer to the Edit form structure
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		06/05/99	Created by Art Lamb.
 * kwk		07/29/99	Set up locked gFieldMap pointer.
 * aro		09/21/00	GetObjectPtr => GetFrmObjectPtr
 * ppl		02/06/02	Add active input Area support (AIA)
 * LFe		10/29/02	New Category Mgr & Books
 *
 ***********************************************************************/
static void PrvEditInit(FormPtr frmP)
{
	FontID			currFont;
	TableType*		tableP;
	uint16_t		i;
	int16_t			row;
	int16_t			rowsInTable;
	uint16_t		fieldIndex;
	char			chr;
	uint16_t		gadgetIndex;

	// Close the cursor. We're now using gCurrentRowID in the EditView
	if (gCursorID != dbInvalidCursorID)
	{
		DbCursorClose(gCursorID);
		gCursorID = dbInvalidCursorID;
	}

	sRowCategoryChanged = false;

	EditViewSetFocusable(true);

	if (!sBookP)
	{
		sBookP = BookCreate(frmP, EditViewEditBook, EditViewEditBookBody, kTabMinWidth, kTabMaxWidth, gBookInfo.numTabs-1,  kBookStandardLookAndFeel);

		if (sBookP)
		{
			for (i = 0; i < gBookInfo.numTabs; i++)
				if (TabIsVisible(gBookInfo.tabs[i].tabInfo.tabId))
					BookAddTab(sBookP, gBookInfo.tabs[i].tabInfo.tabNameP, NULL, 0, 0, gBookInfo.tabs[i].tabInfo.tabId, (uint16_t)kBookTabMaxIndex, 0, PrvEditBookActivation, PrvEditBookDeactivation, PrvEditDrawTableCallback);
		}
	}

	currFont = FntSetFont (gAddrEditFont);

	// Load "no data" str for alternate data editing - see PrvEditDateAndTimeDraw
	sNoDataStr = ToolsCopyStringResource(AlternateDataFieldNoDataStr);

	sAlternateDeleteIconH  = DmGetResource(gApplicationDbP, bitmapRsc, DeleteAlternateToTextDataIcon);
	sAlternateDeleteHighlightedIconH  = DmGetResource(gApplicationDbP, bitmapRsc, DeleteAlternateToTextDataInvertedIcon);

	if (sAlternateDeleteIconH)
		sAlternateDeleteIconBmp = (BitmapType*) DmHandleLock(sAlternateDeleteIconH);

	if (sAlternateDeleteHighlightedIconH)
		sAlternateDeleteHighlightedIconBmp  = (BitmapType*) DmHandleLock(sAlternateDeleteHighlightedIconH);

	// Set the gadget draw function for the name
	gadgetIndex = FrmGetObjectIndex(frmP, EditViewName);
	FrmSetGadgetHandler(frmP, gadgetIndex, PrvEditDrawNameCallback);

	FrmSetTransparentObjects(frmP, true);

	// Initialize the address list table.
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
	rowsInTable = TblGetNumberOfRows(tableP);

	TblSetColumnUsable(tableP, kEditLabelColumn, true);
	TblSetColumnUsable(tableP, kEditDataColumn, true);

	for (row = 0; row < rowsInTable; row++)
	{
		// This sets the data column
		TblSetItemStyle(tableP, row, kEditLabelColumn, labelTableItem);
		TblSetItemStyle(tableP, row, kEditDataColumn, textTableItem);
		TblSetRowID(tableP, row, kNoFieldIndex);
		TblSetRowUsable(tableP, row, false);
	}

	TblSetColumnSpacing(tableP, kEditLabelColumn, kSpaceBeforeDesc);

	// Set the callback routines that will load and save the description field.
	TblSetLoadDataProcedure(tableP, kEditDataColumn, PrvEditLoadDataProc);
	TblSetSaveDataProcedure(tableP, kEditDataColumn, PrvEditSaveDataProc);

	// we set a Draw function procedure for Date, Time and DateTime types fields
	TblSetCustomDrawProcedure(tableP, kEditDataColumn, (TableDrawItemFuncPtr)PrvEditDateAndTimeDrawTblDrawProc);

	// currentTabId should be initilized but not currentTabIndex because the Books is not yet created
	// When the form is created, initialize the currentTabIndex if currentTabId is defined.
	if (gBookInfo.currentTabId != kBookInvalidTabId)
		gBookInfo.currentTabIndex = BookFindTabIndex(sBookP, gBookInfo.currentTabId);

	// This should never happend. Just to prevent error
	if (gBookInfo.currentTabIndex == kBookInvalidTabIndex)
	{
		ErrNonFatalDisplay("Current Tab should be defined");
		gBookInfo.currentTabIndex = 0;
		gBookInfo.currentTabId = BookGetActiveTabId(sBookP);
		gTappedColumnID = kInvalidColumnID;
	}

	// Activate the Tab also load the table. Call it after the table has been initialized.
	BookSetActiveTabId(sBookP, gBookInfo.currentTabId, false);

	// Set the table columns width
	PrvEditSetTableColmunWidth(tableP, gBookInfo.currentTabIndex);

	// Set the gadget draw function for background
	gadgetIndex = FrmGetObjectIndex(frmP, EditBackgroundGadget);
	TblGetBounds(tableP, &sGadgetWhiteArea);
	FrmSetGadgetData(frmP, gadgetIndex, &sGadgetWhiteArea);
	FrmSetGadgetHandler(frmP, gadgetIndex, ToolsBackgroundGadgetHandler);

	// Get the last field index.
	sEditLastFieldIndex = gBookInfo.tabs[gBookInfo.currentTabIndex].numElement - 1;

	// Restore font
	FntSetFont (currFont);
	fieldIndex = 0;

	// This should always be true.
	if (gTappedColumnID != kInvalidColumnID)
	{
		// retrieve the fieldindex from the column id
		while (fieldIndex < gBookInfo.tabs[gBookInfo.currentTabIndex].numElement)
		{
			if (gTappedColumnID == gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId)
				break;

			fieldIndex++;
		}

		if (fieldIndex >= gBookInfo.tabs[gBookInfo.currentTabIndex].numElement)
			fieldIndex = 0;

		PrvEditSetGraffitiMode(gTappedColumnID);
	}

	// Set the global variable that determines which field is the top visible
	// field in the edit view.  Also done when New is pressed.
	sTopVisibleFieldIndex = 0;
	sEditFieldPosition = 0;

	if (fieldIndex > rowsInTable)
		sTopVisibleFieldIndex = fieldIndex;

	sCurrentFieldIndex = fieldIndex;
	PrvEditLoadTable(frmP, false);

	if (gTappedColumnID != kInvalidColumnID)
		sEditFieldPosition = PrvEditGetColumnText(gCurrentRowID, gTappedColumnID, &chr, 1, NULL);

	if (gNumCharsToHilite)
	{
		sHilitedFieldIndex = sCurrentFieldIndex;
		PrvEditRestoreSelection(frmP);
	}
	else
		sHilitedFieldIndex = kNoFieldIndex;

	// Set the Bitmap 3 pixel right the FormTitle.
	// The form Title can now display or not the application small Icon.
	ToolsSetBusinessCardIndicatorPosition(frmP);

	// Set the label of the category trigger.
	PrvEditSetCategoryTriggerLabel(frmP);

	// Business card indicator
	PrvEditDrawBusinessCardIndicator(frmP);

	PrvEditGetFullname();

	// Onehanded stuff
	sOnehandedFieldFocused = false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditConfirmYomi
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
 *
 ***********************************************************************/
static void PrvEditConfirmYomi(char *yomiTextP, uint16_t fieldIndex)
{
	uint32_t		i;
	uint32_t		dataSize = 0;
	uint32_t		yomiColId = kInvalidColumnID;
	status_t		err;
	int16_t			row;
	int16_t			col;
	TableType*		tableP;
	FieldType *		fieldP = NULL;
	FormType*		formP;
	RectangleType	bounds;
	uint32_t		colMaxSize;
	uint32_t		yomiTextLength;
	uint32_t		columnID = kInvalidColumnID;
	uint32_t		editFieldPosition = 0;
	AddressTabColumnPropertiesType *	yomiColPropP;

	// If there is no Yomi field active, return
	if (!gBookInfo.numYomi)
		return;

	// Get the current colId. If no active field, return.
	if (fieldIndex == kNoFieldIndex)
		return;

	yomiTextLength = strlen(yomiTextP);

	if (!yomiTextLength)
		return;

	columnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[fieldIndex].columnId;

/*
	// Exit if no more data (after a cut operation for example)
	err = DbCopyColumnValue(gAddrDB, gCurrentRowID, columnID, 0, NULL, &yomiTextLength);
	if (err < errNone || yomiTextLength < 2) // Can contain the terminating '\0'
		return;
*/


	// Look for the current ColId in the Yomi Array. Try to find the Non Yomi counter part.
	for (i = 0; i < gBookInfo.numYomi; i++)
	{
		if (columnID == gBookInfo.yomiColIdArray[i * 2])
		{
			yomiColId = gBookInfo.yomiColIdArray[i * 2 + 1];
			break;
		}
	}

	// If not found, return.
	if (yomiColId == kInvalidColumnID)
		return;

	// Add the data to the Yomi column
	if ((err = DbCopyColumnValue(gAddrDB, gCurrentRowID, yomiColId, 0, NULL, &dataSize)) < errNone)
		dataSize = 0;

	formP = FrmGetFormPtr (EditView);
	tableP = ToolsGetFrmObjectPtr(formP, EditTable);

	yomiColPropP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, yomiColId);
	colMaxSize = yomiColPropP ? yomiColPropP->maxDataSize : 0;
//	yomiTextLength = strlen(yomiTextP);

	// If no error getting the the Yomi field size in database
	if (dataSize + yomiTextLength <= colMaxSize && ((err >= errNone) || (err == dmErrNoColumnData)))
	{
		fieldP = TblGetCurrentField(tableP);

		if (fieldP)
		{
			TblGetSelection(tableP, &row, &col);
			columnID = TblGetRowData(tableP, row);

			editFieldPosition = FldGetInsPtPosition(fieldP);
			PrvEditSaveSelection(tableP, row, col);

			// Release storage to allow write operation on the current row
			FldReleaseStorage(fieldP);
		}

		// Save the new Yomy data. If data already exist, overlap the end of string in the DB
		err = DbWriteColumnValue(gAddrDB, gCurrentRowID, yomiColId, dataSize ? dataSize - 1 : 0, 1, yomiTextP, (uint32_t)yomiTextLength + 1);
		ErrNonFatalDisplayIf(err < errNone, "Can't write column");

		if (fieldP && columnID != kInvalidColumnID)
		{
			// Use FldSetTextColumn instead of FldReturnStorage because if the currently
			// edited field is a yomi one, FldReturnStorage will raise an alert saying that
			// the column has been modified between FldReleaseStorage and FldReturnStorage
			FldSetTextColumn(fieldP, gAddrDB, gCurrentRowID, 0, columnID, 0);
		}
	}

	// If an error occured, don't try to update the table field.
	if (err < errNone)
		return;

	// Reload the table if the yomi field is visible.
	if (TblFindRowData(tableP, yomiColId, &row))
	{
		TblUnhighlightSelection(tableP);
		FrmSetFocus(formP, noFocus);
		TblMarkRowInvalid(tableP, row);
		PrvEditLoadTable(formP, false);
		TblGetBounds(tableP, &bounds);
		ToolsFrmInvalidateRectFunc(EditView, &bounds, PrvEditDrawTableCallback, NULL);
	}

	if (!fieldP)
		return;

	if (gNumCharsToHilite)
	{
		PrvEditRestoreSelection(formP);
	}
	else
	{
		sEditFieldPosition = editFieldPosition;
		PrvEditRestoreEditState(formP);
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditDoPendingConfirmYomi
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
 *
 ***********************************************************************/
static void PrvEditDoPendingConfirmYomi(void)
{
	if (!sYomiTextP)
		return;

	// if sYomiTextP not NULL means that a tsmConfirmEvent has occured and the call
	// to PrvEditConfirmYomi is pending.
	PrvEditConfirmYomi(sYomiTextP, sYomiFieldIndex);
	MemPtrFree(sYomiTextP);
	sYomiTextP = NULL;
	sYomiFieldIndex = kNoFieldIndex;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditForceAutoYomi
 *
 * DESCRIPTION: Workaround to 'force' a tsmConfirm event to get pushed into the
 *				event queue before we lose focus for the field.
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 ***********************************************************************/
static void PrvEditForceAutoYomi(FormType* frmP)
{
	FieldType*	fldP;
	uint32_t	i;
	uint32_t	columnID;
	uint32_t	yomiColId = kInvalidColumnID;
	size_t		startPos = 0;
	size_t		endPos = 0;

	// Only do this if we have yomi fields for this database.
	if (gBookInfo.numYomi)
	{
		// Get the current colId. If no active field, return.
		if (sCurrentFieldIndex != kNoFieldIndex)
		{
			columnID = gBookInfo.tabs[gBookInfo.currentTabIndex].columnsPropertiesP[sCurrentFieldIndex].columnId;

			// Look for the current ColId in the Yomi Array. Try to find the Non Yomi counter part.
			for (i = 0; i < gBookInfo.numYomi; i++)
			{
				if (columnID == gBookInfo.yomiColIdArray[i * 2])
				{
					yomiColId = gBookInfo.yomiColIdArray[i * 2 + 1];
					break;
				}
			}

			// If we have a valid field that is associated with the yomi then we
			// can trick the OS into forcing a yomi event to get sent out.
			if (yomiColId != kInvalidColumnID)
			{
				fldP = TblGetCurrentField((TableType*)ToolsGetFrmObjectPtr(frmP, EditTable));

				// By calling FldSetSelection, with a selection that is different than
				// the existing one, we can force the OS to dump the inline session
				// and send out a tsmConfirm event.
				if (fldP)
				{
					// Save the original selection.
					FldGetSelection(fldP, &startPos, &endPos);

					if (startPos)	// If we aren't at the beginning, then its safe to set
					{				// the selection to the beginning.
						FldSetSelection(fldP, 0, 0);
					}
					else			// Otherwise, set it to the end.
					{
						size_t len = FldGetTextLength(fldP);
						FldSetSelection(fldP, len, len);
					}

					FldSetSelection(fldP, startPos, endPos); // restore the old selection
				}
			}
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditHandlePen
 *
 * DESCRIPTION: Handle pen events
 *
 * PARAMETERS:	->	eventP: the event
 *
 * RETURNED:	true if handled, false otherwise
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		07/01/2003	Initial revision
 *
 ***********************************************************************/
static Boolean PrvEditHandlePen(EventType *eventP)
{
	RectangleType	bounds;
	FormType *		formP;
	Boolean			handled = false;

	formP = FrmGetFormPtr(EditView);

	switch (eventP->eType)
	{
		case penDownEvent:
			if (sNamePopupDisplayed)
			{
				FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditTable), &bounds);
				handled = RctPtInRectangle(eventP->screenX, eventP->screenY, &sNamePopupBounds);

	            // Discard the popup if tap within the tab bounds
				if (RctPtInRectangle(eventP->screenX, eventP->screenY, &bounds))
					PrvEditDiscardNamePopup(true);
			}

			PrvEditDateAndTimeDiscardSelection();
			break;

		case penMoveEvent:
			PrvEditDateAndTimePenMove(ToolsGetFrmObjectPtr(formP, EditTable), eventP->screenX, eventP->screenY);
			break;

		case penUpEvent:
			if (sNamePopupDisplayed)
			{
				// Discard the popup if displayed
				PrvEditDiscardNamePopup(true);
			}
			else if (sReloadTable)
			{
				// ### TEs 2004/06/10 - fix for BUG44008
				// We have to reload the table when the table font is large (12pts), when the previously
				// seleted row was empty and its height is changed back to standard font.
				// This must be done at penUpEvent because we receive the tblSelectEvent immediatly
				// after the tblEnterEvent and not before the penUpEvent as it was handled in previous
				// version of the OS.
				PrvEditReloadTable(formP);
				sReloadTable = false;
			}
			else
			{
				FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditViewName), &bounds);

				// Display the popup if pen events occured within the name bounds.
				if (RctPtInRectangle(eventP->data.penUp.start.x, eventP->data.penUp.start.y, &bounds) &&
					RctPtInRectangle(eventP->data.penUp.end.x, eventP->data.penUp.end.y, &bounds))
					PrvEditDisplayNamePopup(formP);
			}
			break;

		default:
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    EditViewSetFocusable
 *
 * DESCRIPTION:	Set the focusable state of the EditView. This routine is
 *				used by the DetailsDialog which is able to modify the edited
 *				record. If the DetailsDialog is popup'd and the EditForm
 *				(which is behind) receives a update event and gives the
 *				focus to any table field, the record will be locked and the
 *				DetailsDialog won't be able to modify it anymore (resulting
 *				in a fatal alert)
 *
 * PARAMETERS:	->	focusable: the new focusable state
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		12/18/2003	Initial revision
 *
 ***********************************************************************/
void EditViewSetFocusable(Boolean focusable)
{
	sEditViewIsFocusable = focusable;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditViewNavigate
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		12/18/2003	Initial revision
 *
 ***********************************************************************/
static Boolean PrvEditViewNavigate(WinDirectionType direction)
{
	Boolean handled = false;

	if (direction == winLeft && gBookInfo.currentTabIndex > 0)
	{
		// Go to prev tab
		handled = BookActiveTabIndex(sBookP, gBookInfo.currentTabIndex - 1);
		BookMakeTabVisibleByIndex(sBookP, gBookInfo.currentTabIndex);
	}
	else if (direction == winRight && gBookInfo.currentTabIndex < BookGetNumberOfTab(sBookP) - 1)
	{
		// Go to next tab
		handled = BookActiveTabIndex(sBookP, gBookInfo.currentTabIndex + 1);
		BookMakeTabVisibleByIndex(sBookP, gBookInfo.currentTabIndex);
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditHandleOnehandedNavigation
 *
 * DESCRIPTION: Handle onehanded navigation events for the EditTable object
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
static Boolean PrvEditHandleOnehandedNavigation(EventType *eventP)
{
	Boolean					handled = false;
	FormType *				formP;
	WinDirectionType		direction;
	TableType *				tableP;
	RectangleType			bounds;
	uint32_t				dateValue;
	Boolean					objectFocusMode;
	FrmNavStateFlagsType	navState = 0;
	int16_t					row, col;
	uint16_t				objID;
	uint16_t				focusedObjIndex;
	uint16_t				focusedObjID = frmInvalidObjectId;
	FieldType *				fieldP;

	formP = FrmGetFormPtr(EditView);
	if ((focusedObjIndex = FrmGetFocus(formP)) != noFocus)
		focusedObjID = FrmGetObjectId(formP, focusedObjIndex);

	objectFocusMode = FrmGetNavState(formP, &navState) >= errNone && (navState & kFrmNavStateFlagsObjectFocusMode) != 0;

	switch (eventP->data.keyDown.chr)
	{
	case vchrRockerUp:
	case vchrRockerDown:
		direction = (eventP->data.keyDown.chr == vchrRockerUp) ? winUp : winDown;

		if (!objectFocusMode)
		{
			if (!PrvEditScroll(formP, direction) && (eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
			{
				objID = (direction == winUp) ? EditCategoryTrigger: EditDoneButton;
				FrmSetFocus(formP, FrmGetObjectIndex(formP, objID));
				FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
				FrmNavObjectTakeFocus(formP, objID);
			}

			handled = true;
			break;
		}

		switch (focusedObjID)
		{
		case EditTable:
			if (sOnehandedFieldFocused)
			{
				tableP = ToolsGetFrmObjectPtr(formP, EditTable);
				if (TblGetCurrentField(tableP) == NULL && !sDataFieldSelection && !sDeleteIconSelection)
					break; // Move to next object

				// Move to next/previous field
				handled = (Boolean) (PrvEditNextField(formP, direction, false, false) ||
									(eventP->data.keyDown.modifiers & autoRepeatKeyMask) == autoRepeatKeyMask);

				TblGetSelection(tableP, &row, &col);
				PrvEditSetTblItemFocusRing(tableP, row);
			}
			else
			{
				handled = PrvEditScroll(formP, direction) ||
						(eventP->data.keyDown.modifiers & autoRepeatKeyMask) != 0;
			}
			break;

		case EditViewName:
			if (sNamePopupDisplayed)
				PrvEditDiscardNamePopup(true);
			break;

		}
		break;

	case vchrRockerRight:
		if (!objectFocusMode)
		{
			if (!PrvEditViewNavigate(winRight) && (eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
			{
				FrmSetFocus(formP, FrmGetObjectIndex(formP, EditDoneButton));
				FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
				FrmNavObjectTakeFocus(formP, EditDoneButton);
			}

			handled = true;
			break;
		}

		switch (focusedObjID)
		{
		case EditTable:
			if (sDataFieldSelection)
			{
				tableP = ToolsGetFrmObjectPtr(formP, EditTable);

				row = sSelectedDataFieldRow;
				dateValue = (uint32_t)TblGetItemPtr(tableP, row, kEditDataColumn);
				if (dateValue != (uint32_t)noTime)
				{
					PrvEditDateAndTimeDiscardSelection();
					PrvEditDateAndTimeEnter(tableP, row, true);
					TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);
					PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
					PrvEditSetTblItemFocusRing(tableP, row);
					handled = true;
				}
			}
			break;

		case EditViewName:
			if (sNamePopupDisplayed)
				PrvEditDiscardNamePopup(true);
			break;
		}
		break;

	case vchrRockerLeft:
		if (!objectFocusMode)
		{
			if (!PrvEditViewNavigate(winLeft) && (eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
			{
				FrmSetFocus(formP, FrmGetObjectIndex(formP, EditCategoryTrigger));
				FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
				FrmNavObjectTakeFocus(formP, EditCategoryTrigger);
			}

			handled = true;
			break;
		}

		switch (focusedObjID)
		{
		case EditTable:
			tableP = ToolsGetFrmObjectPtr(formP, EditTable);

			if ((eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0 &&
				(sDataFieldSelection ||
				((fieldP = TblGetCurrentField(tableP)) != NULL && FldGetInsPtPosition(fieldP) == 0)))
			{
				sOnehandedFieldFocused = false;
				PrvEditDateAndTimeDiscardSelection();
				FrmNavObjectTakeFocus(formP, EditTable);
				handled = true;
			}
			else if (sDeleteIconSelection)
			{
				row = sSelectedDataFieldRow;
				PrvEditDateAndTimeDiscardSelection();
				PrvEditDateAndTimeEnter(tableP, row, false);
				TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);
				PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
				PrvEditSetTblItemFocusRing(tableP, row);
				handled = true;
			}
			break;

		case EditViewName:
			if (sNamePopupDisplayed)
				PrvEditDiscardNamePopup(true);
			break;
		}
		break;

	case vchrRockerCenter:
		if (!objectFocusMode)
		{
			if (sNamePopupDisplayed)
				PrvEditDiscardNamePopup(true);

			sOnehandedFieldFocused = false;
			FrmSetNavState(formP, kFrmNavStateFlagsObjectFocusMode);
			FrmNavObjectTakeFocus(formP, EditTable);
			handled = true;
			break;
		}

		switch (focusedObjID)
		{
		case EditTable:
			tableP = ToolsGetFrmObjectPtr(formP, EditTable);
			TblGetSelection(tableP, &row, &col);

			if (!sOnehandedFieldFocused)
			{
				sOnehandedFieldFocused = true;

				if (TblGetItemStyle(tableP, row, kEditDataColumn) == tallCustomTableItem)
				{
					TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);
					PrvEditDateAndTimeEnter(tableP, row, false);
					PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
				}
				else
				{
					TblGrabFocus(tableP, row, kEditDataColumn);
				}

				PrvEditSetTblItemFocusRing(tableP, row);
			}
			else if (sDataFieldSelection || sDeleteIconSelection)
			{
				row = sSelectedDataFieldRow;
				bounds = sDataFieldSelection ? sSelectedDataFieldBounds : sSelectedDeleteIconBounds;
				PrvEditDateAndTimeEnter(tableP, row, sDeleteIconSelection);
				PrvEditDateAndTimeSelect(tableP, row, kEditDataColumn);

				// Re-enter to draw it selected
				PrvEditDateAndTimeEnter(tableP, row, sDeleteIconSelection);
				PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
			}

			handled = true;
			break;

		case EditViewName:
			if (sNamePopupDisplayed)
			{
				PrvEditDiscardNamePopup(true);

				FrmGetObjectBounds(formP, focusedObjIndex, &bounds);
				FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
			}
			else if (sFullnameH)
			{
				if (PrvEditDisplayNamePopup(formP))
					FrmClearFocusHighlight();
			}
			break;
		}
		break;

	default:
		break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    EditHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Edit View"
 *              of the Address Book application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 *	HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * art	06/05/95	Created by Art Lamb.
 * kwk	07/29/99	Unlock gFieldMap in frmCloseEvent block.
 * jmp	09/17/99	Use NewNoteView instead of NoteView.
 * fpa	10/23/00	Fixed bug #27480 - Font Style: The default font
 *					setting did not get refresh in the Address Edit
 *					View when entering text in the last name field
 *					after you entered the first name and tap done
 * gap	10/27/00	change the command bar initialization to allow field
 *					code to add cut, copy, paste, & undo commands as
 *					appropriate rather than adding a fixed set of selections.
 * ppl	02/06/02	Add Active Input Area support (AIA)
 *
 ***********************************************************************/
Boolean EditHandleEvent(EventType * eventP)
{
	FormType *		frmP;
	TableType *		tableP = NULL;
	int16_t			row;
	int16_t			column;
	Boolean			handled = false;
	FieldType *		fldP;
	uint32_t		numLibs;
	size_t			startPos = 0;
	size_t			endPos = 0;
	status_t		err = errNone;
	CategoryID *	categoriesP = NULL;
	uint32_t		numCategories = 0;
	ControlType *	ctlP;
	uint16_t		fieldIndex;
	uint32_t		columnID;
	uint16_t		objIndex;
	RectangleType	bounds;
	FrmNavStateFlagsType	navState = 0;

	frmP = FrmGetFormPtr(EditView);

	switch (eventP->eType)
	{
		case frmOpenEvent:
			PrvEditInit(frmP);
			handled = true;
			break;

		case frmSaveEvent:
			// Save the field being edited.  Do not delete the record if it's
			// empty because a frmSaveEvent can be sent without the form being
			// closed.  A canceled find does this.
			tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
			TblReleaseFocus(tableP);
			break;

		case frmCloseEvent:
			// Check if the record is empty and should be deleted.  This cannot
			// be done earlier because if the record is deleted there is nothing
			// to display in the table.
			PrvEditClose(frmP);
			break;

		case tblEnterEvent:
			gNumCharsToHilite = 0;
			sHilitedFieldIndex = kNoFieldIndex;

			tableP = eventP->data.tblEnter.pTable;
			row = eventP->data.tblEnter.row;
			column = eventP->data.tblEnter.column;

			// Manage Date, Time like data type. Highlight the right part of the table
			if (column == kEditDataColumn)
				PrvEditDateAndTimeEnterPoint(tableP, row, eventP->screenX, eventP->screenY);

			TblSetItemFont(tableP, row, kEditDataColumn, gAddrEditFont);

			fieldIndex = TblGetRowID(tableP, row);

			if (sCurrentFieldIndex != fieldIndex || column == kEditLabelColumn)
			{
				// Set input mode depending on the selected field content
				columnID = TblGetRowData(tableP, row);
				PrvEditSetGraffitiMode(columnID);

				// There is one case where the order could be enter/yomi/select
				// it's when we select the column label instead of the column edit field.
				// So we should to keep this methode also.
				sTblEnterEventOccured = true;
			}

			// Event order has change AGAIN. Now it's enter/select/yomi
			// So save the current field enter as Yomi (no need to test) and
			// if the yomi event is posted, manage it right now.
			sYomiFieldIndex = sCurrentFieldIndex;
			break;

		case tblSelectEvent:
			tableP = eventP->data.tblEnter.pTable;
			row = eventP->data.tblEnter.row;
			column = eventP->data.tblEnter.column;

			PrvEditSaveSelection(tableP, row, column);
			sTblEnterEventOccured = false;

			if (PrvEditBooleanSelect(tableP, row, column) || PrvEditDateAndTimeSelect(tableP, row, column))
			{
				handled = true;
				sCurrentFieldIndex = TblGetRowID(tableP, row);
				PrvPostUserEvent(kFrmCustomUpdateEvent);
				break;
			}

			// Select the field if it's different than the one selected before.  This means the selection
			// is on a different row or the selection is a phone label.
			if (sCurrentFieldIndex != TblGetRowID(tableP, row) || column == kEditLabelColumn)
				handled = PrvEditHandleSelectField(frmP, tableP, row, column);

			// Do pending confirm yomi
			PrvEditDoPendingConfirmYomi();
			break;

		case tblExitEvent:
 			PrvEditDateAndTimeDiscardSelection();
			sTblEnterEventOccured = false;

			// Do pending confirm yomi
			PrvEditDoPendingConfirmYomi();
			break;

		case ctlSelectEvent:
		{
			switch (eventP->data.ctlSelect.controlID)
			{
				case EditCategoryTrigger:
					PrvEditSelectCategory(frmP);
					PrvEditRestoreEditState(frmP);
					handled = true;
					break;

				case EditDoneButton:
					FrmGotoForm(gApplicationDbP, ListView);
					handled = true;
					break;

				case EditDetailsButton:
					PrvEditDiscardFocus(frmP);
					// Set the EditView non-focusable. That way, EditView won't try
					// to restore the focus when it receives a winUpdateEvent.
					EditViewSetFocusable(false);
					FrmPopupForm(gApplicationDbP, DetailsDialog);
					handled = true;
					break;

				case EditNoteButton:
					PrvEditDiscardFocus(frmP);
					EditViewSetFocusable(false);
					AddressEditNote(gCurrentRowID);
					EditViewSetFocusable(true);
					handled = true;
					break;
			}
			break;
		}

		case ctlRepeatEvent:
			switch (eventP->data.ctlRepeat.controlID)
			{
				case EditUpButton:
					PrvEditScroll (frmP, winUp);
					// leave unhandled so the buttons can repeat
					break;

				case EditDownButton:
					PrvEditScroll (frmP, winDown);
					// leave unhandled so the buttons can repeat
					break;
			}
			break;


		case menuEvent:
			handled = PrvEditDoCommand (frmP, eventP->data.menu.itemID);
			break;

		case menuCmdBarOpenEvent:
		{
			DmOpenRef	uilibdbP = NULL;
			uint32_t	uiLibRefNum = 0;

			if ((err = SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum)) >= errNone)
				err = SysGetModuleDatabase(uiLibRefNum, NULL, &uilibdbP);

			ErrNonFatalDisplayIf(err < errNone, "Can't get UILibRefNum");

			fldP = TblGetCurrentField((TableType*)ToolsGetFrmObjectPtr(frmP, EditTable));

			if (fldP)
				FldGetSelection(fldP, &startPos, &endPos);

			if ((fldP) && (startPos == endPos))  // there's no highlighted text
			{
				// Call directly the Field eventP handler so that edit buttons are added if applicable
				FldHandleEvent(fldP, eventP);

				MenuCmdBarAddButton(menuCmdBarOnRight, uilibdbP, BarDeleteBitmap, menuCmdBarResultMenuItem, EditRecordDeleteRecordCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarBeamBitmap, menuCmdBarResultMenuItem, EditRecordBeamRecordCmd, 0);

				// Prevent the field package to add edit buttons again
				eventP->data.menuCmdBarOpen.preventFieldButtons = true;
			}
			else if (fldP == NULL)	// there is no active text field (none have cursor visible)
			{
				MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarDeleteBitmap, menuCmdBarResultMenuItem, EditRecordDeleteRecordCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarBeamBitmap, menuCmdBarResultMenuItem, EditRecordBeamRecordCmd, 0);

				// Prevent the field package to add edit buttons again
				eventP->data.menuCmdBarOpen.preventFieldButtons = true;
			}
			else
			{
			// When there is a selection range of text (ie startPos != endPos)
			// fall through to the field code to add the appropriate cut, copy,
			// paste, and undo selections to the command bar.
				eventP->data.menuCmdBarOpen.preventFieldButtons = false;
			}

			if (err >= errNone)
				SysUnloadModule(uiLibRefNum);

			// don't set handled to true; this event must fall through to the system.
			break;
		}


		case menuOpenEvent:
			if (! gAttachRequest)
			{
				if(!ToolsIsDialerPresent())
					MenuHideItem(EditRecordDialCmd);

				if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
					MenuHideItem(EditRecordSendRecordCmd);
				else
					MenuShowItem(EditRecordSendRecordCmd);
			}
			else
			{
				// Hide send, beam & business card menu items
				MenuHideItem(EditRecordDialCmd);
				MenuHideItem(EditRecordBeamRecordCmd);
				MenuHideItem(EditRecordSendRecordCmd);
				MenuHideItem(EditRecordSelectBusinessCardCmd);
				MenuHideItem(EditRecordBeamBusinessCardCmd);

				// Show attach record menu
				MenuShowItem(EditRecordAttachRecordCmd);
			}
			// don't set handled = true
			break;


		case fldHeightChangedEvent:
			PrvEditResizeDescription(eventP);
			handled = true;
			break;


		case keyDownEvent:
			if (TxtCharIsHardKey(eventP->data.keyDown.modifiers, eventP->data.keyDown.chr))
			{
				TblReleaseFocus((TableType*)ToolsGetFrmObjectPtr(frmP, EditTable));
				gTopVisibleRowIndex = kFirstRowIndex;      // Same as when app switched to
				sCurrentFieldIndex = kNoFieldIndex;
				FrmGotoForm (gApplicationDbP, ListView);
				return (true);
			}
			else if (EvtKeydownIsVirtual(eventP))
			{
				handled = PrvEditHandleOnehandedNavigation(eventP);
				if (handled)
					break;

				switch (eventP->data.keyDown.chr)
				{
					case vchrPageUp:
						FrmClearFocusHighlight();
						FrmSetNavState(frmP, kFrmNavStateFlagsInteractionMode);
						PrvEditScroll (frmP, winUp);
						handled = true;
						break;

					case vchrPageDown:
						FrmClearFocusHighlight();
						FrmSetNavState(frmP, kFrmNavStateFlagsInteractionMode);
						PrvEditScroll (frmP, winDown);
						handled = true;
						break;

					case vchrNextField:
						FrmClearFocusHighlight();
						FrmSetNavState(frmP, kFrmNavStateFlagsInteractionMode);
						PrvEditNextField(frmP, winDown, true, true);
						handled = true;
						break;

					case vchrPrevField:
						FrmClearFocusHighlight();
						FrmSetNavState(frmP, kFrmNavStateFlagsInteractionMode);
						PrvEditNextField (frmP, winUp, true, true);
						handled = true;
						break;

					case vchrSendData:
						MenuEraseStatus (0);
						// Make sure the field being edited is saved
						tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
						TblReleaseFocus(tableP);
						TransferSendRecord(gAddrDB, gCurrentRowID, exgBeamPrefix, NoDataToBeamAlert, AddressExportGetFamily(gBookInfo.currentTabId));
						handled = true;
						break;
				}
			}
			else
			{
				handled = PrvFilterNumericFields(frmP, eventP);

				if (!handled)
					handled = PrvEditAutoFill(frmP, eventP);
			}
			break;

		case winUpdateEvent:
			if (eventP->data.winUpdate.window != FrmGetWindowHandle(frmP))
				break;

			FrmDrawForm(frmP);

			if (sNamePopupDisplayed)
			{
				ToolsDrawFullnamePopup((void*)sFullnameH, &sNamePopupBounds);
			}
			else if (sEditViewIsFocusable && sRestoreEditState)
			{
				// Don't post this event anymore but call directly PrvEditRestoreEditState
				// This works only on back-buffered window, so it should be changed again if
				// the EditView becomes update-base
				//PrvPostUserEvent(kEditViewRestoreEditStateEvent);
				PrvEditRestoreEditState(frmP);
				sRestoreEditState = false;
			}

			handled = true;
			break;

		case kEditViewRestoreEditStateEvent:
			// User event used to not call PrvRestoreEditState from winUpdateEvent
			if (EvtGetFocusWindow() != FrmGetWindowHandle(frmP))
			{
				// It's not safe to call PrvEditRestoreEditState() when the Details
				// or Note dialogs are displayed, since doing so will cause our
				// table's field object to lock down the current record, which
				// will prevent the Details/Note dialogs from working
				break;
			}

			PrvEditRestoreEditState(frmP);
			break;

		case kFrmCustomUpdateEvent:
			// Event sent by Custom view because when custom fields are renamed,
			// it can be necessary to recalculate view screen display: if the width
			// of the custom field is enlarged (and if its content can only be
			// displayed using 2 lines), its content can be displayed on the next line

			PrvEditLoadTable(frmP, true);
			TblGetBounds(ToolsGetFrmObjectPtr(frmP, EditTable), &bounds);
			ToolsFrmInvalidateRectFunc(EditView, &bounds, PrvEditDrawTableCallback, NULL);
			handled = true;
			break;

		case kFrmDetailsUpdateEvent:
			// Event sent by Details dialog when the category has changed.
			DbGetCategory(gAddrDB, gCurrentRowID, &numCategories, &categoriesP);

			ctlP = ToolsGetFrmObjectPtr(frmP, EditCategoryTrigger);
			CatMgrSetTriggerLabel(gAddrDB, categoriesP, numCategories, ctlP, gCategoryName);

			if (categoriesP)
				DbReleaseStorage(gAddrDB, categoriesP);

			// Global categories will be updated at frmClose
			sRowCategoryChanged = true;

			handled = true;
			break;

		case winResizedEvent:
			if (eventP->data.winResized.window != FrmGetWindowHandle(frmP))
				break;

			PrvEditFormResize(frmP, &eventP->data.winResized.newBounds);
			handled = true;
			break;

		case tsmFepButtonEvent:
			// Tap on the FEP confirm button in the silkscreen area.
			sYomiFieldIndex = sCurrentFieldIndex;
			break;

		case tsmConfirmEvent:
			// If the Tsm event is not for us, return
			if (eventP->data.tsmConfirm.formID != EditView || !eventP->data.tsmConfirm.yomiText)
				break;

			// The application could receive the events in 2 different orders, depending on how the user select a row:
			// 1 - When the user select directly the edit field => tblEnter/tblSelect/tsmConfirm
			// 2 - When the user select the column label to select a new row => tblEnter/tsmConfirm/tblSelect or tblExit.
			if (sTblEnterEventOccured)
			{
				// If a tblEnterEvent has occured, it means that we are changing the table row
				// selection. Because of loosing/gaining focus stuff, we don't want PrvEditConfirmYomi
				// to be called between tblEnter and tblSelect events. So we defer the call after
				// tblSelect/tblExit Event.
				sYomiTextP = ToolsStrDup(eventP->data.tsmConfirm.yomiText);
				sYomiFieldIndex = sCurrentFieldIndex;
			}
			else
			{
				PrvEditConfirmYomi(eventP->data.tsmConfirm.yomiText, sYomiFieldIndex);
			}
			break;

		case nilEvent:
			if (sNamePopupDisplayed &&
				TimGetTicks() >= sNamePopupDisplayTime + SysTimeInMilliSecs(kNamePopupTimeout))
			{
				PrvEditDiscardNamePopup(true);

				// If object focus mode, then draw the focus ring
				if (FrmGetNavState(frmP, &navState) >= errNone &&
					(navState & kFrmNavStateFlagsObjectFocusMode) != 0 &&
					FrmGetFocus(frmP) == (objIndex = FrmGetObjectIndex(frmP, EditViewName)))
				{
					FrmGetObjectBounds(frmP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(frmP), &bounds, 0);
				}
			}
			break;

		case kEditViewReleaseFocusEvent:
			// This user event is send from the main event loop when a vchrFind is
			// received in order to avoid an alert message about locked record. The
			// focus is released and the default phone column ID is stored.

			PrvEditForceAutoYomi(frmP);
			PrvEditDiscardFocus(frmP);
			AddressDBCheckAndSetDisplayedPhoneColumn(gAddrDB, gCurrentRowID);
			break;

		case frmObjectFocusTakeEvent:
			if (eventP->data.frmObjectFocusTake.objectID == EditTable)
			{
				FrmNavStateFlagsType	state = 0;

				frmP = FrmGetFormPtr(EditView);
				tableP = ToolsGetFrmObjectPtr(frmP, EditTable);


				FrmGetNavState(frmP, &state);
				TraceOutput(TL(appErrorClass+6, "navState=%lu", state));
				if (!sOnehandedFieldFocused)
				{
					if (sDataFieldSelection || sDeleteIconSelection)
					{
						sOnehandedFieldFocused = true;

						TblGetSelection(tableP, &row, &column);
						PrvEditSetTblItemFocusRing(tableP, row);
					}
					else
					{
						TblUnhighlightSelection(tableP);
						TblReleaseFocus(tableP);

						objIndex = FrmGetObjectIndex(frmP, EditTable);
						FrmSetFocus(frmP, objIndex);
						FrmGetObjectBounds(frmP, objIndex, &bounds);
						FrmSetFocusHighlight(FrmGetWindowHandle(frmP), &bounds, 0);
					}
				}

				handled = true;
			}
			else if (eventP->data.frmObjectFocusTake.objectID == EditViewName)
			{
				frmP = FrmGetFormPtr(EditView);
				objIndex = FrmGetObjectIndex(frmP, EditViewName);
				FrmSetFocus(frmP, objIndex);

				FrmClearFocusHighlight();

				// If the FFN popup doesn't need to be displayed, draw the focus ring around name
				if (!PrvEditDisplayNamePopup(frmP))
				{
					FrmGetObjectBounds(frmP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(frmP), &bounds, 0);
				}

				handled = true;
			}

			break;

		case frmObjectFocusLostEvent:
			if (eventP->data.frmObjectFocusLost.objectID == EditTable)
			{
				frmP = FrmGetFormPtr(EditView);
				tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
				TblGetSelection(tableP, &row, &column);
				sOnehandedFieldFocused = false;
				if (TblGetItemStyle(tableP, row, kEditDataColumn) == tallCustomTableItem && !sInDateAndTimeEdition)
				{
					PrvEditDateAndTimeDiscardSelection();
					TblGetItemBounds(tableP, row, kEditDataColumn, &bounds);
					PrvEditDateAndTimeDraw(tableP, row, kEditDataColumn, &bounds);
				}
			}
			break;

		default:
			// Check for pen events
			handled = PrvEditHandlePen(eventP);
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    EditNewRecord
 *
 * DESCRIPTION: Makes a new record with some setup
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date      	Description
 * -----	--------	-----------
 * roger	06/13/95   	Initial Revision
 * ppl		02/13/02	Add Active Input Area Support (AIA)
 * LFe		10/24/02	Use new Database manager
 *
 ***********************************************************************/
void EditNewRecord (void)
{
	status_t	err;

	gBookInfo.currentTabId		= gBookInfo.defaultNewTabId;
	gBookInfo.currentTabIndex	= kBookInvalidTabIndex;
	gTappedColumnID				= gBookInfo.defaultNewColumnId;

	if (AddressDBNewRecord(gAddrDB, &gCurrentRowID) < errNone)
	{
		FrmUIAlert(DeviceFullAlert);
		gCurrentRowID = dbInvalidRowID;
		return;
	}

	// Set its category to the category being viewed.
	// If the category is All then set the category to unfiled.
	if (gCurrentNumCategories && *gCurrentCategoriesP == catIDAll)
		err = DbSetCategory(gAddrDB, gCurrentRowID, 0, NULL);
	else
		err = DbSetCategory(gAddrDB, gCurrentRowID, gCurrentNumCategories, gCurrentCategoriesP);

	ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");

	FrmGotoForm (gApplicationDbP, EditView);
}
