/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Schemer.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              App to demonstrate simple Data Mgr. calls using a schema
 *				and cursors. The name comes from the way people where I
 *				live (New Hampshire) pronounce the word 'schema.'
 *
 * Release: Palm OS SDK 6.0
 *
 *****************************************************************************/

#include <PalmOS.h>			// Most Palm OS APIs come from here
#include <string.h>			// sprintf, strlen, strcpy, etc.
#include "AppResources.h"	// IDs for our UI resources



/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

// DATABASE DEFINITIONS
// (register Creator ID at http://www.palmos.com/dev/creatorid/)
#define kAppCreatorID			'scDB'
#define kAppDBTypeID			'DATA'

// Database names must now be unique only for a given creator ID
#define kDatabaseName			"SchemerData"

// Column IDs
#define kColSecondsColID	100
#define kColRandNumColID	200

#define gNCols				2

// Schema
#define kDbTableID			0
#define kDbTableName		"MainTable"

// SQL statement pieces
#define kSQL_SelectAllFrom	"SELECT * FROM "
#define kSQL_OrderBySecs	" ORDER BY col100"
#define kSQL_OrderByRandNum	" ORDER BY col200"

// Our database has two indexes
#define kOrderBySecs		0
#define kOrderByRandNum		1

// Preference macros
#define kAppPrefsID				0x00
#define kAppPrefsVersion		0x01
#define kSyncedPrefsDB			true
#define kUnsyncedPrefsDB		false

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

// Application Preferences
typedef struct  {
	uint32_t	selectedRowID;	// Which row was last selected?
	uint16_t 	sortOrder;		// Which ordering (cursor) did we last use?
	uint16_t	pad;			// padding
} SchemerPrefType;


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
DmOpenRef		gResourcesDB;	// Application resource database pointer
DmOpenRef		gDataDB;		// Schema database pointer
uint32_t		gCursor;		// Cursor for view of database
SchemerPrefType gPrefs;			// App prefs (sort order, current row)

// Array of form items we want the OS to move for resize events.
const FormLayoutType gMainFormLayout[] = {
	{ sizeof(FormLayoutType), MainNewButton, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainDeleteButton, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainRowsList, 0, frmFollowTopBottom },
	{ 0,0,0,0 }		// Last item must be all zeros
};


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:    TestGetCursorPosRowID
 *
 * DESCRIPTION: Test routine to play with cursors
 *
 * PARAMETERS:  <-	pos: current cursor's position in view.
 *				<-	rowID: current cursor's row ID
 *
 * RETURNED:    error
 *
 ***********************************************************************/
#if 0
static status_t TestGetCursorPosRowID(uint32_t *pos, uint32_t *rowID)
{
	status_t	err;

	err = DbCursorGetCurrentPosition(gCursor, pos);
	if (err < 0)
		return err;

	err = DbCursorGetCurrentRowID(gCursor, rowID);
	if (err < 0)
		return err;

	return err;
}
#endif

/***********************************************************************
 *
 * FUNCTION:    TestCursorFun
 *
 * DESCRIPTION: Test routine to play with cursors
 *
 * PARAMETERS:   ->	startRowID: initial row ID for newly-added row
 *
 * RETURNED:    void
 *
 * NOTE:		Set breakpoints in here to examine what happens as you
 *				use various DbCursorXXX() functions.
 *				NOTE: restore the cursor to its entry position when you
 *				exit this function.
 *
 ***********************************************************************/
#if 0
static void TestCursorFun(uint32_t startRowID)
{
	status_t	err;
	uint32_t	rowID, pos, count;
	Boolean		fResult;

	count = DbCursorGetRowCount(gCursor);

	// Go to first row
	err = DbCursorMoveFirst(gCursor);
	err = TestGetCursorPosRowID(&pos, &rowID);

	// Try to move to the previous row
	err = DbCursorMovePrev(gCursor);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 1);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 2);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 3);
	err = TestGetCursorPosRowID(&pos, &rowID);

	// Try to move to the second row
	err = DbCursorSetAbsolutePosition(gCursor, 2);
	err = TestGetCursorPosRowID(&pos, &rowID);

	// Remove it
	err = DbRemoveRow(gDataDB, rowID);
	count = DbCursorGetRowCount(gCursor);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 1);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 2);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 3);
	err = TestGetCursorPosRowID(&pos, &rowID);

	// Requery and see where we are then
	err = DbCursorRequery(gCursor);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 1);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 2);
	err = TestGetCursorPosRowID(&pos, &rowID);

	err = DbCursorSetAbsolutePosition(gCursor, 3);
	err = TestGetCursorPosRowID(&pos, &rowID);

	// Go to the end
	count = DbCursorGetRowCount(gCursor);
	err = DbCursorMoveLast(gCursor);
	fResult = DbCursorIsEOF(gCursor);
	err = DbCursorGetCurrentPosition(gCursor, &pos);

	// Try to move anyway
	err = DbCursorMoveNext(gCursor);
	fResult = DbCursorIsEOF(gCursor);
	err = DbCursorGetCurrentPosition(gCursor, &pos);

	// Try to move to the previous row
	err = DbCursorMovePrev(gCursor);
	fResult = DbCursorIsBOF(gCursor);
	err = DbCursorGetCurrentPosition(gCursor, &pos);

}
#endif

/***********************************************************************
 *
 * FUNCTION:    PrvGetObjectPtr
 *
 * DESCRIPTION: Returns a pointer to an object on the specified form.
 *
 * PARAMETERS:  pForm - pointer to form
 *				uwObjectID - ID of desired object
 *
 * RETURNED:    Pointer to object, or NULL if no form specified.
 *
 * NOTE:		We no longer use FrmGetActiveForm(), since there may be
 *				other active forms now. We could also pass the form ID
 *				as an argument and use FrmGetFormPtr(), but it is more
 *				efficient for the caller to get the form pointer one
 *				time and pass it instead.
 *
 ***********************************************************************/
static void * PrvGetObjectPtr(FormType *pForm, uint16_t uwObjectID)
{
	uint16_t	uwObjIndex;

	if (pForm == NULL)
		return NULL;

	uwObjIndex = FrmGetObjectIndex(pForm, uwObjectID);

	return FrmGetObjectPtr(pForm, uwObjIndex);
}


/******************************************************************************
 *
 * FUNCTION:		PrvInvalidateFormWindow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	uwFormID - the ID of the form to be invalidate
 *
 * RETURNED:    	nothing.
 *
 ******************************************************************************/
static void PrvInvalidateFormWindow(uint16_t uwFormID)
{
	WinHandle	hWindow;
	FormType *	pForm;

	if ((pForm = FrmGetFormPtr(uwFormID)) == NULL)
		return;

	if ((hWindow = FrmGetWindowHandle(pForm)) == NULL)
		return;

	WinInvalidateWindow(hWindow);
}


/***********************************************************************
 *
 * FUNCTION:     PrvSetDBBackupBit
 *
 * DESCRIPTION:  This routine sets the backup bit on the given database.
 *					  This is to aid syncs with non Palm software.
 *					  If no DB is given, open the app's default database and set
 *					  the backup bit on it.
 *
 * PARAMETERS:   pDB -	the database to set backup bit,
 *						can be NULL to indicate app's default database
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	4/1/99	Initial Revision
 *
 ***********************************************************************/
static void PrvSetDBBackupBit(DmOpenRef pDB)
{
	DmOpenRef 			pLocalDBRef;
	DatabaseID			dbID;
	DmDatabaseInfoType	dbInfo;
	uint16_t 			attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (pDB == NULL) {
		pLocalDBRef = DbOpenDatabaseByName(kAppCreatorID, kDatabaseName,
										   dmModeReadWrite, dbShareRead);
		if (pLocalDBRef == NULL)
			return;
	} else
		pLocalDBRef = pDB;

	// Now set the backup bit on pLocalDBRef
	DmGetOpenInfo(pLocalDBRef, &dbID, NULL, NULL, NULL);

	MemSet(&dbInfo, sizeof(DmDatabaseInfoType), 0);
	dbInfo.pAttributes = &attributes ;
	DmDatabaseInfo(dbID, &dbInfo);
	attributes |= dmHdrAttrBackup;
	DmSetDatabaseInfo(dbID, &dbInfo);

	// Close database if we had to open it in the beginning.
	if (pDB == NULL)
   		DbCloseDatabase(pLocalDBRef);
}


/***********************************************************************
 *
 * FUNCTION:     PrvGetDatabase
 *
 * DESCRIPTION:  Get the application's database.  Open the database if it
 * exists, create it if neccessary.
 *
 * PARAMETERS:   *dbPP - pointer to a database ref (DmOpenRef) to be set
 *					  mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:     status_t - zero if no error, else the error
 *
 ***********************************************************************/
static status_t PrvGetDatabase(DmOpenRef *dbPP, uint16_t mode)
{
	status_t 				error = 0;
	DmOpenRef 				pDB;
	DatabaseID				dbID;
	DbTableDefinitionType	schema = { 0 };
	DbSchemaColumnDefnType	columns[gNCols];

	*dbPP = NULL;

  	// Find the application's data file.  If it doesn't exist create it.
	pDB = DbOpenDatabaseByName(kAppCreatorID, kDatabaseName, mode, dbShareNone);
	if (!pDB) {
		// Create the database.
		schema.columnListP = columns;
		strcpy(schema.name, kDbTableName);
		schema.numColumns = gNCols;

		// Create the columns:
		// Stores the date/time (in secs) row was created.
		schema.columnListP[0].id = kColSecondsColID;
		schema.columnListP[0].type = dbUInt32;
		schema.columnListP[0].attrib = 0;
		sprintf(schema.columnListP[0].name, "col%d", schema.columnListP[0].id);

		// Stores random number 0..99 generated at row creation time.
		schema.columnListP[1].id = kColRandNumColID;
		schema.columnListP[1].type = dbUInt16;
		schema.columnListP[1].attrib = 0;
		sprintf(schema.columnListP[1].name, "col%d", schema.columnListP[1].id);

		// Now create the database
		error = DbCreateDatabase(kDatabaseName, kAppCreatorID, kAppDBTypeID,
								 1, &schema, &dbID);
		if (error < errNone)
			return error;

		// Created successfully - now open it.
		pDB = DbOpenDatabaseByName(kAppCreatorID, kDatabaseName, mode, dbShareNone);
		if (!pDB)
			return -1;

		// Set the backup bit - we don't have a conduit.
		PrvSetDBBackupBit(pDB);

		// We need to add sort indices in this version of the Data Manager
		// (this may change in the future).

		// Add a sort index on column 100 - ascending
		error = DbAddSortIndex(pDB, kDbTableName kSQL_OrderBySecs " ASC");

		// Add a sort index on column 200 - ascending
		error = DbAddSortIndex(pDB, kDbTableName kSQL_OrderByRandNum " ASC");
	}

	*dbPP = pDB;
	return 0;
}


/***********************************************************************
 *
 * FUNCTION:	PrvOpenCursor
 *
 * DESCRIPTION:	Opens a cursor for database
 *
 * PARAMETERS:	 -> dbP: database ref
 *				<-> pCursorID: ptr to cursor ID
 *				 ->	orderByType: either kOrderBySecs or kOrderByRandNum
 *
 * RETURNED:	err
 *
 ***********************************************************************/
status_t PrvOpenCursor(DmOpenRef pDBRef, uint32_t *pCursorID, uint16_t orderByType)
{
	status_t	err = errNone;

	// Close the last cursor if there was one
	if (*pCursorID != dbInvalidCursorID)
		DbCursorClose(*pCursorID);

	// Open the cursor
	if (orderByType == kOrderBySecs)
		err = DbCursorOpen(pDBRef, kSQL_SelectAllFrom kDbTableName kSQL_OrderBySecs, 0, pCursorID);
	else
		err = DbCursorOpen(pDBRef, kSQL_SelectAllFrom kDbTableName kSQL_OrderByRandNum, 0, pCursorID);

	return err ;
}


/***********************************************************************
 *
 * FUNCTION:     PrvNewRecord
 *
 * DESCRIPTION:  Create a new record in the database.
 *
 * PARAMETERS:   None
 *
 * RETURNED:     status_t - zero if no error, else the error
 *
 ***********************************************************************/
static status_t PrvNewRecord(uint32_t *pRowID)
{
	status_t				err = errNone;
	DbSchemaColumnValueType colVals[gNCols] = { 0 };
	uint16_t				rand;
	uint32_t				rowID;
	uint32_t				secs;

	// Generate column values

	// Store current date/time as seconds since epoch.
	colVals[0].columnID = kColSecondsColID;
	secs = TimGetSeconds();
	colVals[0].data = (DbSchemaColumnData*)&secs;
	colVals[0].dataSize = sizeof(secs);	// only fill in for strings

	// Store random number 0..99
	colVals[1].columnID = kColRandNumColID;
	rand = SysRandom(0) % 100;
	colVals[1].dataSize = sizeof(rand);
	colVals[1].data = (DbSchemaColumnData*)&rand;

	err = DbInsertRow(gDataDB, kDbTableName, gNCols, &colVals[0], &rowID);

	if (err == errNone) {
		err = DbCursorRequery(gCursor);
		*pRowID = rowID;
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    DisplayAboutDialog
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void DisplayAboutDialog(void)
{
	FormType	*pForm;

	// OS6NEW: FrmInitForm() needs app db ptr as first arg now
	pForm = FrmInitForm(gResourcesDB, AboutForm);
	FrmDoDialog(pForm);					// Display the About Box.
	FrmDeleteForm(pForm);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormGetSelectionRowID
 *
 * DESCRIPTION: Get the row ID of the current list selection.
 *
 * PARAMETERS:  <-	pRowID: ptr to rowID
 *
 * RETURNED:    rowID of selection
 *
 ***********************************************************************/
static status_t MainFormGetSelectionRowID(uint32_t *pRowID)
{
	status_t	err;
	FormType	*pForm;
	ListType	*pList;
	int16_t		lstSelIndex;

	pForm = FrmGetActiveForm();
	pList = PrvGetObjectPtr(pForm, MainRowsList);

	// Get the current list selection.
	lstSelIndex = LstGetSelection(pList);

	// Convert to row ID. Remember, cursors are 1-based, but
	// list items are 0-based.
	err = DbCursorGetRowIDForPosition(gCursor, lstSelIndex+1, pRowID);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDrawListItem
 *
 * DESCRIPTION: Called by OS when a list item needs to be drawn.
 *
 * PARAMETERS:   ->	itemNum: 0-based index of list item to draw
 *				 -> pRect: bounds of item in which to draw
 *				 -> unused (normally ptr to array of char ptrs)
 *				 -> pList: ptr to list form object
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void MainFormDrawListItem(int16_t itemNum, RectangleType *pRect,
									   char **unused, struct ListType *pList)
{
	status_t	err;
	Coord		x;
	const Coord	tabPos = 81;
	uint16_t	randVal;
	char		strBuff[16];
	uint32_t	rowID, secs;
	DbSchemaColumnValueType	*pColVals, *p;
	uint32_t	colIDs[2] = { kColSecondsColID, kColRandNumColID };

	// Get row ID for current position (cursor is 1=based, list is 0-based)
	err = DbCursorGetRowIDForPosition(gCursor, itemNum + 1, &rowID);
	if (err < 0)
		return;

	// Get all data at once (we could have used separate DbGetColumnValue()
	// calls, but this is faster).
	err = DbGetColumnValues(gDataDB, rowID, 2,  colIDs, &pColVals);
	if (err < 0)
		return;

	p = pColVals;

	// Get column 100 - the time in seconds
	secs = *(uint32_t *)p->data;
	sprintf(strBuff, "%lu", secs);
	WinDrawTruncChars(strBuff, strlen(strBuff),
					pRect->topLeft.x, pRect->topLeft.y, pRect->extent.x);

	// Get column 200 - a random number (uint16_t)
	++p;
	randVal = *(uint16_t *)p->data;
	sprintf(strBuff, "%u", randVal);
	x = pRect->topLeft.x + tabPos;
	WinDrawTruncChars(strBuff, strlen(strBuff),
					x, pRect->topLeft.y, pRect->extent.x - x);

	// Release storage allocate by Data Mgr.
	DbReleaseStorage(gDataDB, pColVals);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormOpenEvent
 *
 * DESCRIPTION: Set up objects on the main form. Don't draw anything:
 *				just invalidate the form's window when we are done.
 *
 * PARAMETERS:  none.
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void MainFormOpenEvent(void)
{
	FormType		*pForm = FrmGetFormPtr(MainForm);
	ControlType		*pPushButton;
	ListType		*pList;
	uint32_t		numRows;
	status_t		err;
	int16_t			listSelIndex;

	// Start out sorted by date: turn on push button
	if (gPrefs.sortOrder == kOrderBySecs)
		pPushButton = PrvGetObjectPtr(pForm, MainDatePushButton);
	else
		pPushButton = PrvGetObjectPtr(pForm, MainRandomPushButton);
	CtlSetValue(pPushButton, true);

	// Set the draw function for the list.
	pList = PrvGetObjectPtr(pForm, MainRowsList);
	LstSetDrawFunction(pList, MainFormDrawListItem);
	numRows = DbCursorGetRowCount(gCursor);
	LstSetListChoices(pList, NULL, (int16_t)numRows);

	// Initial list selection - use saved rowID
	if (gPrefs.selectedRowID != 0) {
		err = DbCursorGetPositionForRowID(gCursor, gPrefs.selectedRowID,
									 (uint32_t *)&listSelIndex);
		// Cursor row positions are 1-based, list items are 0-based.
		if (err == errNone)
			LstSetSelection(pList, listSelIndex - 1);
	}

	// Invalidate and let OS redraw in response to winUpdateEvent.
	PrvInvalidateFormWindow(MainForm);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormMenuEvent
 *
 * DESCRIPTION: Handle menu selections (and shortcuts).
 *
 * PARAMETERS:   ->	uwMenuCmdID: menu item id
 *
 * RETURNED:    true, if we did something.
 *
 ***********************************************************************/
static Boolean MainFormMenuEvent(uint16_t uwMenuCmdID)
{
	Boolean		fHandled = false;

	switch (uwMenuCmdID) {
		case MainOptionsAboutSchemer:
			DisplayAboutDialog();
			fHandled = true;
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormCtlSelectEvent
 *
 * DESCRIPTION: Handle taps on buttons, pushbuttons, etc.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static Boolean MainFormCtlSelectEvent(EventType *pEvent)
{
	Boolean		fHandled = false;
	status_t	err;
	uint32_t	rowID, pos, count;
	int16_t		listSelIndex;
	FormType	*pForm;
	ListType	*pList;

	switch (pEvent->data.ctlSelect.controlID ) {
		case MainNewButton:
			// Create a new record and requery the cursor
			err = PrvNewRecord(&rowID);

			// Set the cursor to our new row.
			err = DbCursorMoveToRowID(gCursor, rowID);

			// Update the list
			pForm = FrmGetActiveForm();
			pList = PrvGetObjectPtr(pForm, MainRowsList);

			// Update the number of items in the list
			count = DbCursorGetRowCount(gCursor);
			LstSetListChoices(pList, NULL, (uint16_t)count);

			// Make our new item the selection (cursors are 1-based!)
			err = DbCursorGetCurrentPosition(gCursor, &pos);
			LstSetSelection(pList, (uint16_t)(pos - 1));
			LstMakeItemVisible(pList, (uint16_t)(pos - 1));

			// Update the form
			PrvInvalidateFormWindow(MainForm);
			fHandled = true;
			break;

		case MainDeleteButton:
			// Nothing to do if no records
			count = DbCursorGetRowCount(gCursor);
			if (count == 0)
				FrmAlert(gResourcesDB, NoRowsAlert);
			else if (FrmAlert(gResourcesDB, DeleteAlert) == DeleteYes) {
				err = MainFormGetSelectionRowID(&rowID);
				if (err == errNone) {
					// Remove the row and move the cursor and selection
					// to the previous row
					pForm = FrmGetActiveForm();
					pList = PrvGetObjectPtr(pForm, MainRowsList);

					// What item was selected before the delete?
					listSelIndex = LstGetSelection(pList);

					// Remove the row
					err = DbRemoveRow(gDataDB, rowID);

					// Must requery the cursor since we changed it.
					err = DbCursorRequery(gCursor);
					count = DbCursorGetRowCount(gCursor);

					// Hilite same list index unless it was last item
					// in list, else hilite previous item
					if (count > 0) {
						if (listSelIndex == count)
							--listSelIndex;
						LstSetSelection(pList, listSelIndex);
						LstMakeItemVisible(pList, listSelIndex);
					}

					// Update the number of items in the list
					LstSetListChoices(pList, NULL, (uint16_t)count);

					// Update the form
					PrvInvalidateFormWindow(MainForm);
				}
			}
			fHandled = true;
			break;

		case MainDatePushButton:
		case MainRandomPushButton:
			// Save ID of selected row so we can hilite it after reordering.
			err = MainFormGetSelectionRowID(&rowID);

			// Change cursor to use different ordering.
			if (pEvent->data.ctlSelect.controlID == MainRandomPushButton) {
				err = PrvOpenCursor(gDataDB, &gCursor, kOrderByRandNum);
				gPrefs.sortOrder = kOrderByRandNum;
			} else {
				err = PrvOpenCursor(gDataDB, &gCursor, kOrderBySecs);
				gPrefs.sortOrder = kOrderBySecs;
			}

			// Go to the same row, get the position, and subtract one
			// from it to calculate the new list selection index.
			// However, don't do anything else if there are no rows
			count = DbCursorGetRowCount(gCursor);

			if (count > 0) {
				err = DbCursorMoveToRowID(gCursor, rowID);
				err = DbCursorGetCurrentPosition(gCursor, &pos);
				listSelIndex = (int16_t)(pos - 1);

				// Update the list selection.
				pForm = FrmGetActiveForm();
				pList = PrvGetObjectPtr(pForm, MainRowsList);
				LstSetSelection(pList, listSelIndex);
				LstMakeItemVisible(pList, listSelIndex);

				// Force an update.
				PrvInvalidateFormWindow(MainForm);
			}
			fHandled = true;
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 *
 * PARAMETERS:  pEvent  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventType* pEvent)
{
	Boolean 	fHandled = false;
	FormType	*pForm;
	ListType	*pList;
	int16_t		listSelIndex;

	switch (pEvent->eType) {
		case frmOpenEvent:
			MainFormOpenEvent();
			fHandled = true;
			break;

		case ctlSelectEvent:
			fHandled = MainFormCtlSelectEvent(pEvent);
			break;

		case menuEvent:
			fHandled = MainFormMenuEvent(pEvent->data.menu.itemID);
			break;

		case winResizedEvent:
			// Normally, we let the system handle these. But we want to
			// ensure that the list selection is still visible after
			// a resize, so we call FrmPerformLayout(), then update
			// the list selection.
			pForm = FrmGetActiveForm();
			FrmPerformLayout(pForm, &pEvent->data.winResized.newBounds);
			pList = PrvGetObjectPtr(pForm, MainRowsList);
			listSelIndex = LstGetSelection(pList);
			if (listSelIndex != noListSelection)
				LstMakeItemVisible(pList, listSelIndex);
			fHandled = true;	// Override system default handling.
			break;

		default:
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventType* pEvent)
{
	uint16_t 		formId;
	FormType* 	pForm;

	if (pEvent->eType == frmLoadEvent) {
		// Load the form resource.
		formId = pEvent->data.frmLoad.formID;

		// OS6NEW: FrmInitForm() needs app db ptr as first argument
		pForm = FrmInitForm(gResourcesDB, formId);
		FrmSetActiveForm(pForm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId) {
			case MainForm:
				FrmSetEventHandler(pForm, MainFormHandleEvent);
				FrmInitLayout(pForm, gMainFormLayout);
				break;

			default:
				ErrFatalDisplay("Invalid Form Load Event");
				break;
		}

		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    AppStart
 *
 * DESCRIPTION: Get the app's preferences and restore its state,
 *				open (or create) the database, and set the initial
 *				cursor for our data view.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    0 if no errors that would prevent launching the app.
 *
 ***********************************************************************/
static status_t AppStart(void)
{
	status_t	err;
	int16_t		result;
	uint32_t	prefsSize;

	// Restore state of app from preferences. The info is non-critical,
	// so we use the unsynced prefs database.
	prefsSize = sizeof(SchemerPrefType);
	result = PrefGetAppPreferences(kAppCreatorID, kAppPrefsID, &gPrefs,
									&prefsSize, kUnsyncedPrefsDB);

	// Initialize state if first time run.
	if (result == noPreferenceFound) {
		gPrefs.sortOrder = kOrderBySecs;
		gPrefs.selectedRowID = 0;
	}

	// Initialize the random number seed
	SysRandom(TimGetSeconds());

	err = PrvGetDatabase(&gDataDB, dmModeReadWrite);
	if (err != errNone)
		return err;

	// Get cursor for our database view
	gCursor = dbInvalidCursorID;
	err = PrvOpenCursor(gDataDB, &gCursor, gPrefs.sortOrder);
	if (err != errNone)
		DbCloseDatabase(gDataDB);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save app's current state, and close databases, cursors,
 *				etc.
 *				NOTE: the order of the calls below is important.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 ***********************************************************************/
static void AppStop(void)
{
	status_t		err;
//	SchemerPrefType prefs;

	// Save the current sort order and selected item's rowID.
	// The sort order is updated when the push buttons are tapped.
	// We really should be updating the selected rowID as we go,
	// but we aren't at the moment.
	err = MainFormGetSelectionRowID(&gPrefs.selectedRowID);
	if (err < 0)
		gPrefs.selectedRowID = 0;

	// Save our prefs
	PrefSetAppPreferences(kAppCreatorID, kAppPrefsID, kAppPrefsVersion,
							&gPrefs, sizeof(gPrefs), kUnsyncedPrefsDB);

	// Close all the open forms.
	FrmCloseAllForms();

	// Close the cursor to our database view.
	if (gCursor != dbInvalidCursorID)
		err = DbCursorClose(gCursor);

	// Close our schema database.
	if (gDataDB)
		DbCloseDatabase(gDataDB);
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: Event dispatcher. Let system take care of events if
 *				possible, then send to our event handler for the
 *				active form.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	status_t	error;
	EventType	event;

	do {
		// Get next event from the queue...
		EvtGetEvent(&event, evtWaitForever);

		// Is event outside of screen content area?
		if (SysHandleEvent(&event))
			continue;

		// Is event a menu navigation (display, shortcuts) event?
		if (MenuHandleEvent(0, &event, &error))
			continue;

		// Is event a frmLoadEvent? We can handle other events here
		// if we need to...
		if (AppHandleEvent(&event))
			continue;

		// Send event to our current form event handler. If the
		// handler returns false, send the event to the default
		// system event handler in ROM.
		FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t error = errNone;

	// Get app database ref - needed for Form Mgr calls, resources.
	if ((error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gResourcesDB)) < errNone)
		return error;

	// Handle launch code
	switch (cmd) {
		case sysAppLaunchCmdNormalLaunch:
			// Perform app initialization.
			error = AppStart();
			if (error)
				return error;

			// Put a frmLoadEvent in the event queue for our main form
			FrmGotoForm(gResourcesDB, MainForm);

			// Handle events until user switches to another app.
			AppEventLoop();

			// Clean up before exit.
			AppStop();
			break;

		default:
			break;

	}

	return errNone;
}
