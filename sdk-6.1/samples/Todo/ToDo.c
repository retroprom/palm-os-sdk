/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDo.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This is the ToDo application's main module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#include <DebugAppReporting.h>
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
#include <PalmTypesCompatibility.h>
#include <AboutBox.h>
#include <AppMgr.h>
#include <CatMgr.h>
#include <Control.h>
#include <ClipBoard.h>
#include <DateTime.h>
#include <DataMgr.h>
#include <DebugMgr.h>
#include <ErrorMgr.h>
#include <ExgMgr.h>
#include <ExgLocalLib.h>
#include <Event.h>
#include <Field.h>
#include <Find.h>
#include <Form.h>
#include <FormLayout.h>
#include <Font.h>
#include <FontSelect.h>
#include <List.h>
#include <Loader.h>
#include <Menu.h>
#include <NotifyMgr.h>
#include <PenInputMgr.h>
#include <PhoneLookup.h>
#include <Preferences.h>
#include <PrivateRecords.h>
#include <Rect.h>
#include <SelDay.h>
#include <ScrollBar.h>
#include <StringMgr.h>
#include <SoundMgr.h>
#include <SysEventCompatibility.h>
#include <Table.h>
#include <TimeMgr.h>
#include <TextMgr.h>
#include <TraceMgr.h>
#include <Window.h>
#include <Loader.h>
#include "NoteViewLib.h"
#include <SearchLib.h>
#include <string.h>
#include <stdlib.h>

#include "ToDo.h"
#include "ToDoDB.h"
#include "ToDoTransfer.h"
#include "ToDoRsc.h"
#include "ToDoExports.h"

#include "UIColor.h"
#include "UIResources.h"

/***********************************************************************
 *
 *	Local declarations
 *
 ***********************************************************************/
// exg socket structure for appLaunchCmdExgGetFullLaunch
typedef struct {
	ExgSocketType socket;
	char name[exgMaxTypeLength+1];
	char description[exgMaxTypeLength+1];
	char type[exgMaxTypeLength+1];
} ExgPassableSocketType;

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

// Details timeZone display: space to right border
#define detailsTriggerSelectorsSpaceOnRight	10
#define tblDraw		1
#define tblRedraw	2

/***********************************************************************
 *
 *	Globals variables exported outside ToDo.c
 *
 ***********************************************************************/
// Reference to the application
DmOpenRef				gApplicationDbP = 0;

// DB
DmOpenRef				ToDoDB;								// ToDo database
privateRecordViewEnum	PrivateRecordVisualStatus = hidePrivateRecords;		// applies to all other records

// List view
uint32_t				TopVisibleRecord;					// top visible record in list view
uint32_t				LastVisibleRecord;					// last record loaded in list view
FontID					ListFont;							// font used to draw to do item
FontID					ToDoListFont;						// restore default ListFont when changed from Datebook
DateType				Today;								// Date when the device was powered on.
uint32_t				gListViewCursorID = dbInvalidCursorID;
CategoryID				CurrentCategory = catIDAll;			// currently displayed category
MemHandle				EditedItemH = NULL;					// for table load callback
#ifndef BACKBUFFERED
int16_t					gCrossOutRow = -1;					// delay cross-out items when update based
#endif

// true if categories are displayed in the list view
Boolean					ShowCategories = false;

// re-mask record when coming back from details dialog
Boolean					gMaskingRecord = false;

// Is the ToDo launched in stand-alone mode or as a shared library from the Datebook ?
Boolean 				LoadedAsSharedLib;
ToDoSharedDataType		ToDoSharedData;

// Sorting
uint8_t					CurrentSortId = defaultSortId ;

// Attach
Boolean					gAttachRequest = false;			// launched for attachments
char					gTransportPrefix[attachTransportPrefixMaxSize+1];

RGBColorType gAlternatingListHighlightColor;
/***********************************************************************
 *
 *	Local Global variables
 *
 ***********************************************************************/
static char						sCategoryName [maxCategoryNameLength];
static char						NoteTitle[toDoDescriptionMaxSize];		// item description used as note title
static char						CategoryName [maxCategoryNameLength];	// name of the current category
static char						DetailsCatName [maxCategoryNameLength];	// category changed in Details dialog
static privateRecordViewEnum	CurrentRecordVisualStatus = hidePrivateRecords;		// applies to current record
static MenuBarPtr				CurrentMenu;							// pointer to the current menu bar
static uint16_t					PendingUpdate = 0;						// code of pending list view update
static DateFormatType			DateFormat = dfMDYWithSlashes;
static CategoryID *				sDetailsCategoryIDsP = NULL ;			// Used for CategoryMgr in the details dialog
static uint32_t					sDetailsNumCategories = 0 ;
static CategoryID *				sDetailsSavedCategoriesIDP = NULL;
static uint32_t					sDetailsSavedNumCategories = 0;
static CategoryID *				sListViewCategoryIDsP = NULL ;			// Used for CategoryMgr in the list view
static uint32_t					sListViewNumCategories = 0;
static Boolean					sToDoAppNormalLaunched = false;			// Set to true when ToDo is launched with
																		// sysAppLaunchCmdNormalLaunch launch code.
// The following global variables are used to keep track of the edit
// state of the application.
static uint32_t					CurrentRecord = noRecordSelected;		// record being edited
static CategoryID				CurrentRecordCategory = catIDAll;		// used for selection when sorting by categories
static Boolean					ItemSelected = false;					// true if a list view item is selected
static Boolean					RecordDirty = false;					// true if a record has been modified
static size_t					ListEditPosition = 0;					// position of the insertion point in the desc field
static size_t					ListEditSelectionLength;				// length of the current selection.
static RectangleType 			gListViewWinBounds;						// list view window bounds.
static WinHandle				gListViewWinH;							// list view window handle, to check win handle upon winResizedEvent receipt.
static WinHandle				gDetailsDialogWinH;
static WinHandle				gOptionsDialogWinH;
static uint32_t					gMatchColumn;							// ColumnID set by GoToItem
static uint32_t					gTblDrawCode = 0;


static FontID					NoteFont = stdFont;						// font used in note view
static Boolean					ShowAllCategories = true;				// true if all categories are being displayed
static Boolean 					ShowCompletedItems = true;				// true if completed items are being displayed
static Boolean 					ShowOnlyDueItems = false;				// true if only due items are displayed
static Boolean					ShowDueDates = false;					// true if due dates are displayed in the list view
static Boolean					ShowPriorities = true;					// true if priorities are displayed in the list view
static Boolean					SaveBackup = true;						// true if save backup to PC is the default
static Boolean					ChangeDueDate = false;					// true if due date is changed to completion date when completed
static Boolean					InNoteView = false;						// true if we've called ToDoEditNote
static Boolean					InDetails = false;						// true if the details dialog is opened
static Boolean					gCheckingPassword = false;				// true if we've called SecSelectViewStatus or SecVerifyPW
static Boolean					gTableNavFocus = false;					// true if the main table has nav focus


#define viewSortingByCategories (CurrentCategory == catIDAll && sortingByCategories(CurrentSortId))

/***********************************************************************
 *
 *	Pre-declaration of Functions
 *
 ***********************************************************************/
void 		ListViewInit (FormPtr frm);
void 		ListViewLoadTable (Boolean fillTable);
void 		ListViewDrawTable (uint16_t updateCode);
void 		ListViewRedrawTable (Boolean fillTable);
Boolean 	ListViewUpdateDisplay (uint16_t updateCode);
void 		ListViewResizeForm(EventType *eventP, Boolean scroll);

Boolean 	ClearEditState (void);

void 		ToDoLoadPrefs(void);				// (BGT)
void 		ToDoSavePrefs(void);				// (BGT)

void		ChangeCategory (CategoryID category);

static Boolean PrvBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);
static status_t PrvToDoGetColorResource(
	DmOpenRef dbP,
	DmResourceID id,
	RGBColorType *color);


/***********************************************************************
 *
 * FUNCTION:     ToDoDeleteNote
 *
 * DESCRIPTION:  DeleteNoteCallback for ToDoEditNote
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			LYr		12/24/03		Initial Revision
 *
 ***********************************************************************/
Boolean ToDoDeleteNote(DmOpenRef dbRef, uint32_t rowID, uint32_t rowPos, uint32_t columnId)
{
	uint32_t 	noteViewLibRefNum = 0;
	DmOpenRef 	noteViewLibDBP = NULL;

	SysLoadModule(sysFileTLibrary, sysFileCNoteViewLib, 0, 0, &noteViewLibRefNum);
	SysGetModuleDatabase(noteViewLibRefNum, NULL, &noteViewLibDBP);

	if (FrmAlert(noteViewLibDBP, DeleteNoteAlert) != DeleteNoteYes)
	{
		SysUnloadModule(noteViewLibRefNum);
		return false;
	}

	DbCursorSetAbsolutePosition(rowID, rowPos);
	DbWriteColumnValue(dbRef, rowID, columnId, 0, -1, NULL, 0);
	SysUnloadModule(noteViewLibRefNum);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:     ToDoEditNote
 *
 * DESCRIPTION:  Calls NoteView, sets InNoteView as true
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			LYr		9/16/03		Initial Revision
 *
 ***********************************************************************/
Boolean ToDoEditNote(const char *title, DmOpenRef dbRef, uint32_t rowID, uint32_t columnId, DeleteNoteCallbackPtr deleteCallbackP)
{
	Boolean result;
	uint32_t pos = 0;

	DbCursorMoveToRowID(gListViewCursorID, rowID);
	DbCursorGetCurrentPosition(gListViewCursorID, &pos);

	InNoteView = true;
	result = EditNote(title, dbRef, gListViewCursorID, pos, columnId, deleteCallbackP);
	InNoteView = false;
	return result;
}

/***********************************************************************
 *
 * FUNCTION:     CreateDefaultDatabase
 *
 * DESCRIPTION:  This routine creates the default database from the
 *					  saved image in a resource in the application.
 *
 * PARAMETERS:   none
 *
 * RETURNED:     0 - if no error
 *					  otherwise appropriate error value
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vivek	8/17/00	Initial Revision
 *
 ***********************************************************************/
status_t CreateDefaultDatabase(void)
{
	MemHandle	resH;
	DmOpenRef	dbP;
	status_t			error = errNone;
	DatabaseID	dbID;

	// Attempt to get our default data image and create our
	// database.
	if ((resH = DmGetResource(gApplicationDbP, 'scdb', ToDoDefaultDBResId)) == NULL)
		return DmGetLastErr();

	if ((error = DmCreateDatabaseFromImage(MemHandleLock(resH), &dbID)) < errNone)
		goto Exit ;

	// Open the DB to add sort indexes
	if ((dbP = DbOpenDatabaseByName (sysFileCToDo, kToDoDBName, dmModeReadWrite, dbShareRead)) == NULL)
	{
		error = DmGetLastErr();
		goto Exit ;
	}

	// set the backup bit on the new database
	ToDoSetDBBackupBit(dbP);

	error = DbCloseDatabase (dbP);

Exit:
	MemHandleUnlock(resH);
	DmReleaseResource(resH);
	return error;
}

/***********************************************************************
 *
 * FUNCTION:     StartApplication
 *
 * DESCRIPTION:  This routine opens the application's database, loads the
 *               saved-state information and initializes global variables.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			grant	4/6/99	Move code to set backup bit into SetDBBackupBit
 *			jmp	10/4/99	Call ToDoGetDatabase() in place of similar in-line code.
 *
 ***********************************************************************/
status_t StartApplication ()
{
	status_t	err = errNone;
	uint16_t	mode;

	PrvToDoGetColorResource(
		gApplicationDbP,
		AlternatingListHighlightColorStr,
		&gAlternatingListHighlightColor);

	// If this application is executed in stand-alone, sets its Ids, otherwise it
	// has already been set by the application which execute the ToDo as a shared library
	if (!LoadedAsSharedLib)
	{
		ToDoSharedData.listViewId = ListView;
		ToDoSharedData.listTableId = ListTable;
		ToDoSharedData.ListCategoryListId = ListCategoryList;
		ToDoSharedData.ListCategoryTriggerId = ListCategoryTrigger;

		// Get today's date. Use to know if items are due, and to determine if passed
		// due items need to be redrawn when the device is powered on the next time.
		DateSecondsToDate (TimGetSeconds (), &Today);
	}

	// Determime if secret record should be shown.
	PrivateRecordVisualStatus = CurrentRecordVisualStatus =
		(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
	if (LoadedAsSharedLib)
		mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
						dmModeReadOnly : (dmModeReadOnly | dmModeShowSecret);
	else
		mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
						dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	// Get the date format from the system preferences.
	DateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);

	// Read the preferences.
	ToDoLoadPrefs();
	TopVisibleRecord = 1;
	CurrentRecord = noRecordSelected;

	// Find the application's data file.  If it doesn't exist create it.
	err = ToDoGetDatabase(&ToDoDB, mode);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    StopApplication
 *
 * DESCRIPTION: This routine closes the application's database
 *              and saves the current state of the application.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
void StopApplication (void)
{
	ToDoSavePrefs();

	// Do not save and close forms if ToDo has been loaded
	// as a shared library
	if (! LoadedAsSharedLib)
	{
		// Send a frmSave event to all the open forms.
		FrmSaveAllForms ();

		// Close all the open forms.
		FrmCloseAllForms ();
	}

	// Close the cursor & application's data file.
	ToDoCloseCursor(&gListViewCursorID);
	DbCloseDatabase (ToDoDB);
}


/***********************************************************************
 *
 * FUNCTION:    SyncNotification
 *
 * DESCRIPTION: This routine is a entry point of the ToDo application.
 *              It is called when the ToDo application's database is
 *              synchronized.  This routine will re-sort the database and
 *              reset the state file info if necessary.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/6/95	Initial Revision
 *			jmp	10/4/99	Replaced call to DmOpenDatabaseByTypeCreator() with
 *								ToDoGetDatabase().
 *
 ***********************************************************************/
static void SyncNotification (void)
{
	status_t			err = errNone;
	char				name [catCategoryNameLength];
	DmOpenRef			dbP;
	ToDoPreferenceType 	prefs;
	uint32_t			prefsSize;

	// Open the application's data file.
	err = ToDoGetDatabase(&dbP, dmModeReadWrite);
	if (err)
		return;

	// Check if the currrent category still exists.
	prefsSize = (uint32_t)sizeof (ToDoPreferenceType);

	if (PrefGetAppPreferences (sysFileCToDo, todoPrefID, &prefs, &prefsSize, true) != noPreferenceFound)
	{

		err = CatMgrGetName (dbP, prefs.currentCategory, name);
		if (err < errNone || *name == 0)
		{
			prefs.currentCategory = catIDAll;
			prefs.showAllCategories = true;

			PrefSetAppPreferences (sysFileCToDo, todoPrefID, toDoVersionNum, &prefs, sizeof (ToDoPreferenceType), true);
		}
	}

	DbCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:	RegisterData
 *
  * DESCRIPTION:	Register with the Exchange Manager for attach mechanism.
 *
 * PARAMETERS:		none
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			PLe			04/21/03	Initial Revision
 *
 ***********************************************************************/
static void RegisterData(void)
{
	// Get application (icon name)
	MemHandle resH = DmGetResource(gApplicationDbP, ainRsc, ainID );
	char * descP = MemHandleLock(resH);

	// Register for the attach feature
	ExgRegisterDatatype(sysFileCToDo, exgRegSchemeID, exgGetScheme, descP, 0);

	// Register for the old todo creator code.  If we don't, todo objects get sent
	// to address book.  Todo and address book share a common file extension.
	ExgRegisterDatatype(sysFileCToDo, exgRegCreatorID, "ramt", NULL, 0 );
	ExgSetDefaultApplication(sysFileCToDo,exgRegCreatorID, "ramt" );

	MemHandleUnlock(resH);
	DmReleaseResource(resH);
}

/***********************************************************************
 *
 * FUNCTION:    SearchDraw
 *
 * DESCRIPTION: This routine draws the description of a ToDo item found
 *              by the text search routine
 *
 * PARAMETERS:	 desc  - pointer to a description field
 *              x     - draw position
 *              y     - draw position
 *              width - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 * HISTORY:
 *		04/18/95	art	Created by Art Lamb.
 *		12/10/00	kwk	Use WinDrawTruncChars, versus byte-by-byte processing
 *							of text that doesn't work with Japanese.
 *
 ***********************************************************************/
static void SearchDraw (char* desc, int16_t x, int16_t y, int16_t width)
{
	char * ptr = StrChr (desc, linefeedChr);
	uint16_t titleLen = (ptr == NULL ? strlen (desc) : (uint16_t) (ptr - desc));
	if (FntWidthToOffset (desc, titleLen, width, NULL, NULL) == titleLen)
	{
		WinDrawChars (desc, titleLen, x, y);
	}
	else
	{
		int16_t titleWidth;
		titleLen = FntWidthToOffset (desc, titleLen, (uint16_t)(width - FntCharWidth(chrEllipsis)), NULL, &titleWidth);
		WinDrawChars (desc, titleLen, x, y);
		WinDrawChar(chrEllipsis, (Coord)(x + titleWidth), y);
	}
}

/***********************************************************************
 *
 * FUNCTION:    Search
 *
 * DESCRIPTION: This routine searchs the ToDo database for records
 *              that contain the findParams string.
 *
 * PARAMETERS:	 findParams - text search parameter block
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/18/95	Initial Revision
 *			jmp   10/21/99	Changed params to findParams to match other routines
 *								like this one.
 *
 ***********************************************************************/
static void Search (FindParamsPtr findParams)
{
	char *				descP = NULL;
	uint32_t			descLen;
	status_t			err;
	char *				headerStrP;
	MemHandle			headerStrH;
	RectangleType		r;
	Boolean				done;
	Boolean				match;
	DatabaseID			dbID;
	DmOpenRef			dbP = NULL;
	SearchLibType		search;
	CategoryID			cat = catIDAll;
	uint32_t			searchableColIDs[] = { todoDescriptionColId, todoNoteColId };
	uint16_t			attr;

	PIMAppProfilingBegin("Search");

	findParams->more = false;

	memset(&search, 0x0, sizeof(SearchLibType));

	if (ToDoDB)
	{
		search.searchDB = ToDoDB;
	}
	else
	{
		ToDoLoadPrefs(); // For CurrentSortID
		ToDoGetDatabase(&dbP, (uint16_t)findParams->dbAccesMode);
		search.searchDB = dbP;
		PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
	}

	if (!search.searchDB)
		PIMAppProfilingReturnVoid();

	// Get the DatabaseID
	DmGetOpenInfo(search.searchDB, &dbID, NULL, NULL, NULL);

	search.cursorID			= dbInvalidCursorID; // SearchLib to create the cursor
	search.schemaName		= kToDoDBSchemaName;
	search.orderBy			= CurrentSortId == noSortOrder ? "" : SortOrderStr[CurrentSortId % 2];
	search.cursorFlag		= dbCursorEnableCaching;

	if (sortingByCategories(CurrentSortId))
		search.cursorFlag	|= dbCursorSortByCategory;

	search.startRowIndex	= findParams->recordNum + 1;
	search.numCols			= sizeof(searchableColIDs) / sizeof(searchableColIDs[0]);
	search.colIDsP			= searchableColIDs;
	search.numCategories	= 1;
	search.catIDsP			= &cat;
	search.matchMode		= DbMatchAny;
	search.textToSearch		= findParams->strToFind;
	search.origTextToSearch	= findParams->strAsTyped;
	search.recordDirection	= dmSeekForward;
	search.columnDirection	= dmSeekForward;
	search.interruptible	= true;
	search.interruptCheck	= 500;	// Every 16th record, check if interrupted

	if (SearchLibInit(&search) < errNone)
		goto Exit;

	// Display the heading line.
	headerStrH = DmGetResource(gApplicationDbP, strRsc, FindToDoHeaderStr);
	headerStrP = MemHandleLock (headerStrH);
	done = FindDrawHeader(findParams, headerStrP);
	MemHandleUnlock(headerStrH);
	DmReleaseResource(headerStrH);
	if (done)
		goto Exit;

	// Search the description and note fields for the "find" string.
	while (true)
	{
		match = SearchLibSearch(&search);

		if (!match)
		{
			findParams->more = search.interrupted;
			break;
		}

		// Verify the secret attribute.
		// This could happen when the application is already launched. That cost less than creating another cursor.
		if ((DbGetRowAttr(search.searchDB, search.foundRowID, &attr) >= errNone) && (attr & dbRecAttrSecret) && (PrivateRecordVisualStatus != showPrivateRecords))
		{
			search.resumePolicy = kSearchResumeChangeRecord;
			continue;
		}

		// Add the match to the find paramter block,  if there is no room to
		// display the match the following function will return true.
		done = FindSaveMatch(findParams, search.foundRowIndex, search.foundRowID, search.matchPos, search.matchLength, search.foundColID, 0, dbID);
		if (done)
		{
			findParams->more = true;
			break;
		}

		// Get the bounds of the region where we will draw the results.
		FindGetLineBounds (findParams, &r);

		// Load the record
		err = DbGetColumnValue(search.searchDB, search.foundRowID, todoDescriptionColId, 0, (void**)&descP, &descLen);

		if (err >= errNone && descP)
		{
			// Display the description.
			SearchDraw(descP, (int16_t)(r.topLeft.x + 1), r.topLeft.y, (int16_t)(r.extent.x - 2));
			DbReleaseStorage(search.searchDB, descP);
			descP = NULL;
		}

		findParams->lineNumber++;
		search.resumePolicy = kSearchResumeChangeRecord;
	}

Exit:
	SearchLibRelease(&search);

	if (dbP)
		DbCloseDatabase(dbP);

	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:    GoToItem
 *
 * DESCRIPTION: This routine is called when the "Go to" button
 *              in the text search dialog is pressed.
 *
 * PARAMETERS:	 goToParams   - where to go to
 *              launchingApp - true is the application is being launched
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	06/06/95	Initial Revision
 *			kwk	12/03/98	Fixed param order in call to MemSet.
 *			jmp	09/17/99 Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
static void GoToItem(GoToParamsPtr goToParams, Boolean launchingApp)
{
    status_t		err;
	uint32_t		recordNum;
	int32_t			dateL;
	int32_t			todayL;
	uint32_t		uniqueID;
	EventType		event;
	DateTimeType	today;
	ToDoItemType	item;
	DbSchemaColumnValueType * colVals = NULL;
	CategoryID 		dbSelectAllCategoriesID[] = {catIDAll};
	Boolean			recordIsInCategory = false;

	uniqueID = goToParams->recordID;

	// Close main cursor in case current record is in cache
	ToDoCloseCursor(&gListViewCursorID);

	if (!ToDoDB)
		ToDoGetDatabase(&ToDoDB, (PrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));

	// Check that the record exists
	err = DbIsRowInCategory(ToDoDB, uniqueID, 1, dbSelectAllCategoriesID, DbMatchAny, &recordIsInCategory);

	if ((err < errNone) || (!recordIsInCategory))
	{
		// Reopen the main cursor
		ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
		FrmUIAlert(secGotoInvalidRecordAlert);
		FrmGotoForm(gApplicationDbP, ToDoSharedData.listViewId);
		return;
	}

	// Change the current category if necessary.
	if (CurrentCategory != catIDAll)
	{
		uint32_t		numCategories = 0;
		CategoryID *	categoriesP = NULL;
		Boolean			inCategory;

		DbGetCategory(ToDoDB, uniqueID, &numCategories, &categoriesP);

		inCategory = (CurrentCategory == catIDUnfiled && !numCategories);
		while (!inCategory && numCategories--)
		{
			inCategory = (CurrentCategory == categoriesP[numCategories]);
		}

		if (!inCategory)
			ChangeCategory(catIDAll);

		if (categoriesP)
			DbReleaseStorage(ToDoDB, categoriesP);
	}

	// Reopen the main cursor
	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	// Get the position from uniqueID
	DbCursorGetPositionForRowID(gListViewCursorID, uniqueID, &recordNum);

	// If the application is already running, close all the open forms.  If
	// the current record is blank, then it will be deleted, so we'll get
	// the unique id of the record and use it to find the record index
	// after all the forms are closed.
	if (! launchingApp && !gCheckingPassword)
	{
		FrmCloseAllForms();
		ClearEditState();
	}

	// Make the item the first item displayed, using first category assigned to it
	TopVisibleRecord = recordNum;

	// If the item is not displayable given the current display options,
	// change the display options so that it will be.
	DbGetColumnValues(ToDoDB, uniqueID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &item, gNCols);

	// If only completed items are not being displayed, and the item is complete,
	// change the "show completed" option setting.
	if (!ShowCompletedItems && item.completed)
		ShowCompletedItems = true;

	// If only due items are being show, and the item is not due,
	// change the "show only due items" option.
	if (ShowOnlyDueItems && DateToInt(item.dueDate) != toDoNoDueDate)
	{
		// Check if the item is due.
		TimSecondsToDateTime(TimGetSeconds(), &today);
		todayL = ( ((int32_t) today.year) << 16) +
					( ((int32_t) today.month) << 8) +
						((int32_t) today.day);

		dateL = ( ((int32_t) item.dueDate.year + firstYear) << 16) +
					( ((int32_t) item.dueDate.month) << 8) +
						((int32_t) item.dueDate.day);

		if (dateL > todayL)
			ShowOnlyDueItems = false;
	}

	// Send an event to goto a form and select the matching text.
	MemSet (&event, sizeof(EventType), 0);

	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = ToDoSharedData.listViewId;
	EvtAddEventToQueue (&event);

	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = ToDoSharedData.listViewId;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchLen;
	event.data.frmGoto.matchFieldNum =  goToParams->matchFieldNum;
	EvtAddEventToQueue (&event);

	gMatchColumn = goToParams->matchFieldNum ;

	DbReleaseStorage(ToDoDB, colVals);
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void * GetObjectPtr(uint16_t objectID, uint16_t formID)
{
	FormType *	formP;

	formP = FrmGetFormPtr(formID);
	return FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, objectID));
}


/***********************************************************************
 *
 * FUNCTION:    SetObjectValue
 *
 * DESCRIPTION: Assign a value to the object with the given ID
 *
 * PARAMETERS:  objectID  - id of the object to change
 *              value     - new value of the object
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *
 ***********************************************************************/
static void SetObjectValue (uint16_t formID, uint16_t objectID, int16_t value)
{
	ControlPtr ctl;

	ctl = GetObjectPtr (objectID, formID);
	CtlSetValue (ctl, value);
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectValue
 *
 * DESCRIPTION: Return the value of the object with the given ID
 *
 * PARAMETERS:  objectID  - id of the object to change
 *
 * RETURNED:    value of the object
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *
 ***********************************************************************/
static int16_t GetObjectValue (uint16_t formID, uint16_t objectID)
{
	ControlPtr ctl;

	ctl = GetObjectPtr (objectID, formID);
	return CtlGetValue (ctl);
}


/***********************************************************************
 *
 * FUNCTION:    ChangeCategory
 *
 * DESCRIPTION: This routine updates the global variables that keep track
 *              of category information.
 *
 * PARAMETERS:  category  - new category (index)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
void ChangeCategory (CategoryID category)
{
	if (CurrentCategory == category)
		return ;
	CurrentCategory = category;
	TopVisibleRecord = 1;
	ShowAllCategories = (CurrentCategory == catIDAll) ;
}


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
static void SelectFont (FontID * currFontID)
{
	FontID fontID;

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (*currFontID);

	if (fontID != *currFontID)
	{
		*currFontID = fontID;
		ListViewUpdateDisplay(updateFontChanged);
	}
}


/***********************************************************************
 *
 * FUNCTION:    SeekRecord
 *
 * DESCRIPTION: Given the index of a ToDo record, this routine scans
 *              forwards or backwards for displayable ToDo records.
 *
 * PARAMETERS:  indexP  - pointer to the index of a record to start from;
 *                        the index of the record sought is returned in
 *                        this parameter.
 *
 *              offset  - number of records to skip:
 *                        	0 - seek from the current record to the
 *                             next display record, if the current record is
 *                             a display record, its index is retuned.
 *                         1 - seek foreward, skipping one displayable
 *                             record
 *                        -1 - seek backwards, skipping one displayable
 *                             record
 *
 *
 * RETURNED:    false is return if a displayable record was not found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/11/95	Initial Revision
 *
 ***********************************************************************/
Boolean SeekRecord (uint32_t* indexP, int32_t offset, int32_t direction)
{
	status_t 					err ;
	int32_t 					dateL, todayL;
	DbSchemaColumnValueType * 	colValsP = NULL ;
	ToDoItemType 				item;

	if (DbCursorSetAbsolutePosition(gListViewCursorID, * indexP) < errNone)
		goto ExitWithFailure;

	// Loop on records now...
	while (true)
	{
		if (offset && DbCursorMove(gListViewCursorID, direction == dmSeekForward ? offset : -offset, dbFetchRelative) < errNone)
			goto ExitWithFailure;

		DbCursorGetCurrentPosition(gListViewCursorID, indexP);
		if ( ShowCompletedItems && (! ShowOnlyDueItems))
			goto ExitWithSuccess;

		err = DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colValsP);
		if (dbGetDataFailed(err))
			goto ExitWithFailure;

		ToDoColumnValuesToItem(colValsP, &item, gNCols);

		if ( (ShowCompletedItems) || (! item.completed))
		{
			if (! ShowOnlyDueItems)
				break;

			if (DateToInt (item.dueDate) == toDoNoDueDate)
				break;

			// Check if the item is due.
			todayL = ( ((int32_t) Today.year + firstYear) << 16) +
					  ( ((int32_t) Today.month) << 8) +
						 ((int32_t) Today.day);

			dateL = ( ((int32_t) item.dueDate.year + firstYear) << 16) +
					  ( ((int32_t) item.dueDate.month) << 8) +
						 ((int32_t) item.dueDate.day);

			if (dateL <= todayL)
				break;
		}

		if (offset == 0)
			offset = 1;
		DbReleaseStorage(ToDoDB, colValsP);
		colValsP = NULL;
	}

ExitWithSuccess:
	if (colValsP)
		DbReleaseStorage(ToDoDB, colValsP);
	return true;

ExitWithFailure:
	if (colValsP)
		DbReleaseStorage(ToDoDB, colValsP);
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    DeleteRecord
 *
 * DESCRIPTION: This routine deletes the selected ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the delete occurred,  false if it was canceled.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/30/95		Initial Revision
 *
 ***********************************************************************/
static Boolean DeleteRecord (uint32_t index)
{
	status_t err;
	uint16_t ctlIndex;
	uint16_t buttonHit;
	FormPtr alert;
	Boolean saveBackup;
	uint32_t uniqueID;

	// Display an alert to comfirm the operation.
	alert = FrmInitForm(gApplicationDbP, DeleteToDoDialog);

	ctlIndex = FrmGetObjectIndex (alert, DeleteToDoSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);
	buttonHit = FrmDoDialog (alert);
	saveBackup = (Boolean)FrmGetControlValue (alert, ctlIndex);;

	FrmDeleteForm (alert);

	if (buttonHit == DeleteToDoCancel)
		return (false);

	SaveBackup = saveBackup;
	DbCursorGetRowIDForPosition(gListViewCursorID, index, &uniqueID);

	// Delete or archive the record.
	FrmSetFocus(FrmGetFormPtr(ToDoSharedData.listViewId), noFocus);
	if (SaveBackup)
	{
		DbCursorFlushCache(gListViewCursorID);
		err = DbArchiveRow (ToDoDB, uniqueID);
		if (err) {
			return false;
		}
	}
	else
	{
		DbCursorFlushCache(gListViewCursorID);
		err = DbDeleteRow (ToDoDB, uniqueID);
		if (err) {
			return false;
		}
	}

	err = ToDoDbCursorRequery(&gListViewCursorID);

	return err == errNone;
}


/***********************************************************************
 *
 * FUNCTION:    ClearEditState
 *
 * DESCRIPTION: This routine take the application out of edit mode.
 *              The edit state of the current record is remember until
 *              this routine is called.
 *
 *              If the current record is empty, it will be deleted
 *              by this routine.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true is current record is deleted by this routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/14/95	Initial Revision
 *
 ***********************************************************************/
Boolean ClearEditState (void)
{
	uint32_t recordNum, uid;
	Boolean empty;
	ToDoItemType item;
	DbSchemaColumnValueType * colVals;
	FormType *	formP;

	TraceOutput(TL(appErrorClass, "ClearEditState CurrentRecord: %d itemSelected : %d",CurrentRecord,ItemSelected));

	formP = FrmGetFormPtr(ToDoSharedData.listViewId);
	if (formP)
	{
		TblUnhighlightSelection(FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, ToDoSharedData.listTableId)));
		TblReleaseFocus(FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, ToDoSharedData.listTableId)));
		FrmSetFocus(formP, noFocus);
	}
	else
		ItemSelected = false;

	if (!ItemSelected)
	{
		CurrentRecord = noRecordSelected;
		CurrentRecordCategory = catIDAll;
		return (false);
	}

	recordNum = CurrentRecord;

	// Clear the global variables that keeps track of the edit start of the
	// current record.
	ItemSelected = false;
	CurrentRecord = noRecordSelected;
	CurrentRecordCategory = catIDAll;
	ListEditPosition = 0;
	ListEditSelectionLength = 0;
	PendingUpdate = 0;

	// If the description field is empty and the note field is empty, delete
	// the ToDo record.
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &item, gNCols);
	empty = itemIsEmpty(item);
	DbReleaseStorage(ToDoDB, colVals);
	if (empty)
	{
		// If the description was not modified, and the description and
		// note fields are empty, remove the record from the database.
		// This can occur when a new empty record is deleted.
		DbCursorGetCurrentRowID(gListViewCursorID, &uid);
		TraceOutput(TL(appErrorClass, "ClearEditState deleting record ID: %d index: %d", uid, recordNum));
		FrmSetFocus(FrmGetFormPtr(ToDoSharedData.listViewId), noFocus);
		if (RecordDirty)
			DbDeleteRow (ToDoDB, gListViewCursorID);
		else
			DbRemoveRow (ToDoDB, gListViewCursorID);

		ToDoDbCursorRequery(&gListViewCursorID);

		return (true);
	}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    DetemineDueDate
 *
 * DESCRIPTION: This routine is called when an item of the "due date"
 *              popup list is selected.  For items such as "today" and
 *              "end of week" the due date is computed,  for "select
 *              date" the date picker is displayed.
 *
 * PARAMETERS:  item selected in due date popup list.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/31/95	Initial Revision
 *
 ***********************************************************************/
static void DetermineDueDate (uint16_t itemSelected, DateType * dueDateP)
{
	int16_t month, day, year;
	int32_t adjustment=0;
	char* titleP;
	MemHandle titleH;

	// "No due date" items selected?
	if (itemSelected == noDueDateItem)
	{
		*((int16_t*) dueDateP) = -1;
		return;
	}

	// "Select date" item selected?
	else if (itemSelected == selectDateItem)
	{
		if ( *((int16_t*) dueDateP) == -1)
		{
			year = Today.year + firstYear;
			month = Today.month;
			day = Today.day;
		}
		else
		{
			year = dueDateP->year + firstYear;
			month = dueDateP->month;
			day = dueDateP->day;
		}

		titleH = DmGetResource(gApplicationDbP, strRsc, DueDateTitleStr);
		titleP = (char*) MemHandleLock (titleH);

		if (SelectDay (selectDayByDay, &month, &day, &year, titleP))
		{
			dueDateP->day = day;
			dueDateP->month = month;
			dueDateP->year = year - firstYear;
		}

		MemHandleUnlock (titleH);
		return;
	}


	// "Today" item seleted?
	else if (itemSelected == dueTodayItem)
		adjustment = 0;

	// "Tomorrow" item selected?
	else if (itemSelected == dueTomorrowItem)
		adjustment = 1;

	// "One week later" item selected?
	else if (itemSelected == dueOneWeekLaterItem)
	{
		adjustment = (int32_t) daysInWeek;
	}

	*dueDateP = Today;
	DateAdjust (dueDateP, adjustment);
}

/***********************************************************************
 *
 * FUNCTION:    OptionsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Options Dialog
 *              (aka Preferences).
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95	Initial Revision
 *			rbb	4/14/99	Uses GetObjectValue (trimming a few bytes of code)
 *
 ***********************************************************************/
static void OptionsApply (void)
{
	uint8_t sortOrder;
	int16_t val;

	// Update the sort order.  Reset the ToDo list to the top.
	sortOrder = (uint8_t) LstGetSelection (GetObjectPtr (OptionsSortByList, OptionsDialog));

	if (CurrentSortId != sortOrder)
	{
		ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, sortOrder);
		TopVisibleRecord = 1;
	}

	// Show or hide items marked complete.  Reset the list to the top.
	val = GetObjectValue (OptionsDialog, OptionsShowCompleted);
	if (ShowCompletedItems != val)
	{
		ShowCompletedItems = (Boolean)val;
		TopVisibleRecord = 1;
	}

	// Show only items due today or show all items (in the current
	// category).
	val = GetObjectValue (OptionsDialog, OptionsShowDueItems);
	if (ShowOnlyDueItems != val)
	{
		ShowOnlyDueItems = (Boolean)val;
		TopVisibleRecord = 1;
	}

	// Change the due date field, in the record, to the completion
	// date when the item is mark complete.
	ChangeDueDate	= (Boolean)GetObjectValue(OptionsDialog, OptionsChangeDueDate);

	// Show or hide the due date, priorities, and categories columns
	ShowDueDates	= (Boolean)GetObjectValue(OptionsDialog, OptionsShowDueDates);
	ShowPriorities	= (Boolean)GetObjectValue(OptionsDialog, OptionsShowPriorities);
	ShowCategories	= (Boolean)GetObjectValue(OptionsDialog, OptionsShowCategories);
}


/***********************************************************************
 *
 * FUNCTION:    OptionsInit
 *
 * DESCRIPTION: This routine initializes the Options Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95	Initial Revision
 *			rbb	4/14/99	Uses SetObjectValue (trimming a few bytes of code)
 *
 ***********************************************************************/
static void OptionsInit (void)
{
	char* label;
	ListPtr lst;
	ControlPtr ctl;

	// Set the trigger and popup list that indicates the sort order.

	lst = GetObjectPtr (OptionsSortByList, OptionsDialog);
	label = LstGetSelectionText (lst, (uint16_t)CurrentSortId);
	ctl = GetObjectPtr (OptionsSortByTrigger, OptionsDialog);
	CtlSetLabel (ctl, label);
	LstSetSelection (lst, (uint16_t)CurrentSortId);

	// Initialize the checkboxes in the dialog box.
	SetObjectValue (OptionsDialog, OptionsShowCompleted, ShowCompletedItems);
	SetObjectValue (OptionsDialog, OptionsShowDueItems, ShowOnlyDueItems);
	SetObjectValue (OptionsDialog, OptionsChangeDueDate, ChangeDueDate);
	SetObjectValue (OptionsDialog, OptionsShowDueDates, ShowDueDates);
	SetObjectValue (OptionsDialog, OptionsShowPriorities, ShowPriorities);
	SetObjectValue (OptionsDialog, OptionsShowCategories, ShowCategories);
}


/***********************************************************************
 *
 * FUNCTION:    OptionsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Options
 *              Dialog Box" of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95		Initial Revision
 *
 ***********************************************************************/
static Boolean OptionsHandleEvent (EventPtr event)
{
	Boolean handled = false;
	FormPtr frm;
	static UInt16 objectID;

	switch(event->eType)
	{
		case frmObjectFocusTakeEvent:
			if (event->data.frmObjectFocusTake.formID == OptionsDialog)
				objectID = event->data.frmObjectFocusTake.objectID;
			break;

		case keyDownEvent:
			if (EvtKeydownIsVirtual(event)
			&& event->data.keyDown.chr == vchrRockerRight
			&& objectID >= OptionsSortByTrigger
			&& objectID <= OptionsShowCategories )
			{
				frm = FrmGetFormPtr(OptionsDialog);
				FrmNavObjectTakeFocus (frm, OptionsOkButton);
				handled = true;
			}
			break;

		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case OptionsOkButton:
					OptionsApply ();
				case OptionsCancelButton:
					FrmReturnToForm (ToDoSharedData.listViewId);
					ListViewUpdateDisplay(updateDisplayOptsChanged);
					handled = true;
					break;

				case OptionsShowCategories:
					if ((CurrentSortId == categoryPriorityItem) ||
						(CurrentSortId == categoryDueDateItem))
					{
						// Do not permit to uncheck this item if we sort by categories
						FrmAlert(gApplicationDbP, UncheckShowCategoryAlert);
						SetObjectValue (OptionsDialog, OptionsShowCategories, true);
						handled = true;
					}
					break;
			}

		case popSelectEvent:
			switch (event->data.popSelect.listID)
			{
				case OptionsSortByList:
					// Force the show category check box if needed
					CurrentSortId = (uint8_t) event->data.popSelect.selection;
					if ((CurrentSortId == categoryPriorityItem) ||
						(CurrentSortId == categoryDueDateItem))
						SetObjectValue (OptionsDialog, OptionsShowCategories, true);
					break;
			}
			break;

		case frmOpenEvent:
			OptionsInit ();
			objectID = 0xffff;
			handled = true;
			break;

		case winUpdateEvent:
			if (event->data.winUpdate.window != gOptionsDialogWinH)
				break;
			frm = FrmGetFormPtr(OptionsDialog);
			FrmDrawForm (frm);
			handled = true;
			break;

	}

	return (handled);
}

/***********************************************************************
 *
 * FUNCTION:    DetailsSetDateTrigger
 *
 * DESCRIPTION: This routine sets the date trigger, in the details dialog,
 *              to the specified date.
 *
 * PARAMETERS:  year	  - years (since 1904)
 *              month  - months (1-12)
 *              day    - days (1-31)
 *
 * RETURNED:    nothing
 *
 * NOTES:
 *      This routine assumes that the memory allocated for the label of
 *      the due date trigger is large enough to hold the largest possible
 *      label.  This label memory is reserved by initializing the label
 *      in the resource file.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/31/95	Initial Revision
 *
 ***********************************************************************/
static void DetailsSetDateTrigger (DateType date)
{
	int16_t dayOfWeek;
	char * str;
	char* label;
	ListPtr lst;
	ControlPtr ctl;


	// Label storage is located into label control.
	// we have to reserve enoiught space for that string in resource
	// due to DateToDOWDMFormat usage label must have dowDateStringLength
	// unless the pop up list opens only one time while the dialog is opened.

	lst = GetObjectPtr (DetailsDueDateList, DetailsDialog);
	LstSetSelection (lst, noDueDateItem);

	ctl = GetObjectPtr (DetailsDueDateTrigger, DetailsDialog);
	label = (char *)CtlGetLabel (ctl);

	// Minus one means no date.  Set the label of the trigger to the
	// first choice of the popup list, which is "no date".
	if (DateToInt (date) == toDoNoDueDate)
	{
		str = LstGetSelectionText (lst, noDueDateItem);
		strcpy (label, str);
		CtlSetLabel (ctl, label);
		LstSetSelection (lst, noDueDateItem);
	}

	// Format the date into a string and set it as the label of the trigger.
	else
	{
		// Format the date into a string.
		dayOfWeek = DayOfWeek (date.month, date.day, (int16_t) (date.year + firstYear));

		DateToDOWDMFormat ((uint8_t)date.month, (uint8_t)date.day, (uint16_t) (date.year + firstYear), DateFormat, label);

		CtlSetLabel (ctl, label);
		LstSetSelection (lst, selectDateItem);
	}
	WinInvalidateWindow(gDetailsDialogWinH);
}

/***********************************************************************
 *
 * FUNCTION:    DetailsSetCategoryTrigger
 *
 * DESCRIPTION: This routine sets the category trigger, in the details dialog.
 *
 * PARAMETERS:
 *
 * RETURNED:    nothing
 *
 * NOTES:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			Ple		5/06/03		Initial Revision
 *
 ***********************************************************************/
static void DetailsSetCategoryTrigger (void)
{
	FormPtr 	detailsFrmP;
	uint16_t	maxCatWidth, catTriggerIndex;
	Coord		catPosx, catPosy;
	RectangleType detailsRect;

	// Get the details form ptr
	detailsFrmP = FrmGetFormPtr (DetailsDialog);

	catTriggerIndex = FrmGetObjectIndex (detailsFrmP, DetailsCategoryTrigger);
	FrmGetObjectPosition(detailsFrmP, catTriggerIndex, &catPosx, &catPosy);
	FrmGetFormInitialBounds(detailsFrmP, &detailsRect);
	maxCatWidth = detailsRect.extent.x - catPosx - detailsTriggerSelectorsSpaceOnRight;

	CatMgrTruncateName(ToDoDB, sDetailsCategoryIDsP, sDetailsNumCategories,
		maxCatWidth, DetailsCatName);
	CtlSetLabel (FrmGetObjectPtr (detailsFrmP, catTriggerIndex), DetailsCatName);
}

/***********************************************************************
 *
 * FUNCTION:    DetailsSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories in the Details Dialog.
 *
 * PARAMETERS:  category - the current catagory, returns to new
 *                         category
 *
 * RETURNED:    true if the category was changed in a way that
 *              require the list view to be redrawn.
 *
 *              The following global variables are modified:
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *			mchen 12/20/00 Changed to sort after CategorySelect() returns
 *
 ***********************************************************************/
static Boolean DetailsSelectCategory (uint32_t currentUID, CategoryID * category)
{
	char* name;
	Boolean categoryEdited = false;
	CategoryID *	selectedCategoryIDsP = NULL;
	CategoryID		currentCategory;
	uint32_t		selectedNumCategories = 0;

	name = (char *)CtlGetLabel (GetObjectPtr (DetailsCategoryTrigger, DetailsDialog));

	// Close our cursor in case current category was deleted
	ToDoCloseCursor(&gListViewCursorID);

	categoryEdited = CatMgrSelectEdit(ToDoDB, FrmGetFormPtr(DetailsDialog),
			DetailsCategoryTrigger, name, DetailsCategoryList, true,
			sDetailsCategoryIDsP, sDetailsNumCategories,
			&selectedCategoryIDsP, &selectedNumCategories,
			true, NULL);

	// If it was deleted move to catIDAll
	if (CurrentCategory != catIDUnfiled && 
		CatMgrFind(ToDoDB, CategoryName, &currentCategory) < errNone)
		ChangeCategory(catIDAll);

	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	if (categoryEdited)
	{
		if (sDetailsCategoryIDsP)
			DbReleaseStorage(ToDoDB, sDetailsCategoryIDsP);

		// Reopen the cursor with catIDAll so we are sure to find the current record
		currentCategory = catIDAll;
		if (CurrentCategory != catIDAll)
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &currentCategory, 1, priorityDueDateItem);
		DbCursorMoveToRowID(gListViewCursorID, currentUID);
		DbSetCategory(ToDoDB, gListViewCursorID, selectedNumCategories, selectedCategoryIDsP);
		DbGetCategory(ToDoDB, gListViewCursorID, &sDetailsNumCategories, &sDetailsCategoryIDsP);
		// Reopen the cursor with the current category
		if (CurrentCategory != catIDAll)
		{
			ChangeCategory(sDetailsCategoryIDsP ? sDetailsCategoryIDsP[0] : catIDUnfiled);
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
		}

		*category = sDetailsCategoryIDsP ? sDetailsCategoryIDsP[0] : catIDUnfiled ;
		DetailsSetCategoryTrigger();

		CatMgrFreeSelectedCategories(ToDoDB, &selectedCategoryIDsP);
	}

	return (categoryEdited);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsDeleteToDo
 *
 * DESCRIPTION: This routine deletes a ToDo item.  This routine is called
 *              when the delete button in the details dialog is pressed.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the record was delete or archived.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static Boolean DetailsDeleteToDo (uint32_t recordID)
{
	CategoryID category;
	uint32_t recordNum;
	Boolean empty, result;
	ToDoItemType item;
	DbSchemaColumnValueType * colVals;

	result = true;

	// Reopen the cursor with catIDAll so we are sure to find the current record
	category = catIDAll;
	if (CurrentCategory != catIDAll)
		ToDoOpenCursor(ToDoDB, &gListViewCursorID, &category, 1, priorityDueDateItem);

	// Check if the record is empty. If it is, clear the edit state,
	// this will delete the current record if it is blank.
	DbCursorMoveToRowID(gListViewCursorID, recordID);
	DbCursorGetCurrentPosition(gListViewCursorID, &recordNum);
	DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &item, gNCols);
	empty = itemIsEmpty(item);
	DbReleaseStorage(ToDoDB, colVals);

	if (empty)
	{
		CurrentRecord = recordNum;
		category = CurrentCategory;
		CurrentCategory = catIDAll;
		ClearEditState ();
		CurrentCategory = category;
		result = true;
		goto Reopen;
	}

	// Display an alert to confirm the delete operation, and delete the
	// record if the alert is confirmed.
	if (! DeleteRecord (recordNum) )
	{
		result = false;
		goto Reopen;
	}

	ClearEditState ();
	ItemSelected = false;

Reopen:
	// Reopen the cursor with the current category
	if (CurrentCategory != catIDAll)
		ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	return result;
}


/***********************************************************************
 *
 * FUNCTION:    DetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  category        - new catagory
 *              dueDateP        - new due date
 *              categoryEdited  - true if current category has been moved,
 *              deleted, renamed, or merged with another category
 *
 * RETURNED:    code which indicates how the ToDo list was changed,  this
 *              code is sent as the update code in a frmUpdate event.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *			kcr	10/9/95	Added 'private records' alert
 *			rbb	4/14/99	Uses GetObjectValue (trimming a few bytes of code)
 *			jmp	11/15/99	Don't clear the edit state here as we could be
 *								going into NoteView, and NoteView needs to be
 *								associated with a record.  Instead, just return
 *								update code, and let ListViewUpdateDisplay() handle
 *								clearing the edit state if necessary.
 *
 ***********************************************************************/
static uint16_t DetailsApply (uint32_t uniqueID, DateType * dueDateP, Boolean categoryEdited, Boolean noteEdited)
{
	CategoryID			category;
	status_t			err ;
	FormPtr				frm;
	uint16_t			attr;
	uint16_t			index;
	uint16_t			updateCode = 0;
	uint8_t 			curPriority;
	uint8_t 			newPriority;
	Boolean  			secret, empty, deleteItem;
	Boolean				dirty = false;
	DateType			curDueDate;
	DbSchemaColumnValueType * colVals ;
	ToDoItemType toDoRec;

	// Reopen the cursor with catIDAll so we are sure to find the current record
	category = catIDAll;
	if (CurrentCategory != catIDAll)
		ToDoOpenCursor(ToDoDB, &gListViewCursorID, &category, 1, priorityDueDateItem);

	// Get the secret attribute of the current record.
	DbCursorMoveToRowID(gListViewCursorID, uniqueID);
	DbCursorGetCurrentPosition(gListViewCursorID, &CurrentRecord);
	DbGetRowAttr (ToDoDB, gListViewCursorID, &attr);

	// Get the priority setting from the dialog and compare it with the
	// priortity value in the current record.
	frm = FrmGetFormPtr(DetailsDialog);
	index = FrmGetControlGroupSelection (frm, DetailsPrioritiesGroup);

	err = DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ErrFatalDisplayIf (dbGetDataFailed(err), "Record not found");

	ToDoColumnValuesToItem(colVals, &toDoRec, gNCols);
	curPriority = toDoRec.priority;
	DbReleaseStorage(ToDoDB, colVals);

	newPriority = FrmGetObjectId (frm, index) - DetailsPriority1Trigger + 1;

	if (curPriority != newPriority)
	{
		TraceOutput(TL(appErrorClass, "ToDo Priority %d => %d",toDoPriority,newPriority));
		ToDoChangeRecord (ToDoDB, gListViewCursorID, &CurrentRecord, toDoPriority, &newPriority);
		updateCode |= updateItemMove;
		dirty = true;
	}

	// Compare the due date setting in the dialog with the due date in the
	// current record.  Update the record if necessary.
	err = DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ErrFatalDisplayIf (dbGetDataFailed(err), "Record not found");

	ToDoColumnValuesToItem(colVals, &toDoRec, gNCols);
	curDueDate = toDoRec.dueDate;
	empty = itemIsEmpty(toDoRec);
	DbReleaseStorage(ToDoDB, colVals);

	if (DateToInt(*dueDateP) != DateToInt(curDueDate))
	{
		TraceOutput(TL(appErrorClass, "ToDo date %d-%d-%d => %d-%d-%d",
				curDueDate.year,curDueDate.month,curDueDate.day,
				dueDateP->year, dueDateP->month, dueDateP->day));
		ToDoChangeRecord (ToDoDB, gListViewCursorID, &CurrentRecord, toDoDueDate, dueDateP);
		updateCode |= updateItemMove;
		dirty = true;
	}

	// Update the record if the category was edited
	if (categoryEdited)
		updateCode |= updateItemMove;

	// Get the current setting of the secret checkbox and compare it the
	// the setting of the record.  Update the record if the values
	// are different.
	gMaskingRecord = secret = (Boolean)GetObjectValue (DetailsDialog, DetailsSecretCheckbox);
	if (((attr & dbRecAttrSecret) == dbRecAttrSecret) != secret)
	{
		if (PrivateRecordVisualStatus > showPrivateRecords)
			updateCode |= updateItemHide;
		else if (secret)
			FrmUIAlert(privateRecordInfoAlert);

		// Reload
		updateCode |= updateDisplayOptsChanged;

		dirty = true;
		if (secret)
			attr |= dbRecAttrSecret;
		else
			attr &= ~dbRecAttrSecret;
	}

	// If the current category was deleted, renamed, or merged with
	// another category, then the list view needs to be redrawn.
	if (categoryEdited)
		updateCode |= (updateCategoryChanged | updateDisplayOptsChanged);


	// Save the new category and/or secret status
	if (dirty)
	{
		err = DbSetRowAttr (ToDoDB, gListViewCursorID, &attr);
	}


	// Force redraw table for note indicator
	if (noteEdited)
	{
		TraceOutput(TL(appErrorClass, "DETAILS - FORCING TODO NOTE ICON"));
		updateCode |= updateDisplayOptsChanged;
	}


	if (empty)
	{	// If an item is secret and empty, delete it
		if (attr & dbRecAttrSecret)
		{
			TraceOutput(TL(appErrorClass, "DETAILS - DELETING EMPTY MASKED ITEM"));
			err = DbDeleteRow (ToDoDB, gListViewCursorID);
			updateCode |= updateDisplayOptsChanged;
		}
		// also delete an empty item that won't show in the list view because its category changed
		else if (categoryEdited && CurrentCategory != catIDAll
			&& !(sDetailsNumCategories == 0 && CurrentCategory == catIDUnfiled))
		{
			deleteItem = true;
			for (index = 0; index < sDetailsNumCategories; index++)
			{
				if (CurrentCategory == sDetailsCategoryIDsP[index])
				{
					deleteItem = false;
					break;
				}
			}
			if (deleteItem)
			{
				TraceOutput(TL(appErrorClass, "DETAILS - DELETING EMPTY ITEM BECAUSE CATEGORY CHANGED"));
				err = DbDeleteRow (ToDoDB, gListViewCursorID);
				updateCode |= updateDisplayOptsChanged;
			}
		}
	}

	// Reopen the cursor with the current category
	if (CurrentCategory != catIDAll)
		ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	return (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsInit
 *
 * DESCRIPTION: This routine initializes the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *			rbb	4/14/99	Uses SetObjectValue (trimming a few bytes of code)
 *
 ***********************************************************************/
static void DetailsInit (uint32_t * currentUIDP, CategoryID * categoryP, DateType * dueDateP)
{
	uint16_t 	attr;
	uint16_t 	priority;
	DbSchemaColumnValueType * colVals ;
	ToDoItemType toDoRec;
	status_t err;

	// Get current record's unique ID
	DbCursorSetAbsolutePosition(gListViewCursorID, CurrentRecord);
	DbCursorGetCurrentRowID(gListViewCursorID, currentUIDP);

	// If the record is marked secret, turn on the secret checkbox.
	DbGetRowAttr (ToDoDB, gListViewCursorID, &attr);
	SetObjectValue (DetailsDialog, DetailsSecretCheckbox, (int16_t) ((attr & dbRecAttrSecret) != 0));

	// Get record categories
	DbGetCategory(ToDoDB, gListViewCursorID, &sDetailsNumCategories, &sDetailsCategoryIDsP);
	DbGetCategory(ToDoDB, gListViewCursorID, &sDetailsSavedNumCategories, &sDetailsSavedCategoriesIDP);

	// Set the label of the category trigger.
	DetailsSetCategoryTrigger();

	// Get a pointer to the ToDo record.
	err = DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ErrFatalDisplayIf (dbGetDataFailed(err), "Record not found");

	ToDoColumnValuesToItem(colVals, &toDoRec, gNCols);

	// Set the priority push button.
	priority = (uint16_t) toDoRec.priority;
	SetObjectValue (DetailsDialog, (uint16_t) (DetailsPriority1Trigger + priority - 1), true);

	// Set the due date trigger.
	DetailsSetDateTrigger (toDoRec.dueDate);

	// Return the current category and due date.
	*categoryP = sDetailsCategoryIDsP ? sDetailsCategoryIDsP[0] : catIDUnfiled ;
	*dueDateP = toDoRec.dueDate;

	// Unlock the ToDo record
	DbReleaseStorage(ToDoDB, colVals);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box" of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	9/17/99	Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
static Boolean DetailsHandleEvent (EventPtr event)
{
	static uint32_t		currentUID;
	static CategoryID 	category, savedCurrentCategory;
	static DateType 	dueDate;
	static Boolean		categoryEdited;
	static Boolean		noteEdited;
	static UInt16		objectID;
	status_t err;

	uint16_t updateCode;
	Boolean handled = false;
	FormPtr frm;

	uint32_t noteTitleLen = toDoDescriptionMaxSize;

	switch(event->eType)
	{
		case frmObjectFocusTakeEvent:
			if (event->data.frmObjectFocusTake.formID == DetailsDialog)
				objectID = event->data.frmObjectFocusTake.objectID;
			break;

		case keyDownEvent:
			if (EvtKeydownIsVirtual(event)
			&& event->data.keyDown.chr == vchrRockerRight
			&& (objectID == DetailsCategoryTrigger || objectID == DetailsDueDateTrigger || objectID == DetailsSecretCheckbox || objectID == 1207 /* p5 */ ))
			{
				frm = FrmGetFormPtr(DetailsDialog);
				FrmNavObjectTakeFocus (frm, DetailsOkButton);
				handled = true;
			}
			break;

		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case DetailsNoteButton:
					err = DbCopyColumnValue(ToDoDB, currentUID, todoDescriptionColId, 0, (void*)NoteTitle, &noteTitleLen);
					ToDoEditNote((err == errNone && noteTitleLen > 0) ? NoteTitle : NULL, ToDoDB, currentUID, todoNoteColId, &ToDoDeleteNote);
					PendingUpdate = noteEdited = true ;

				case DetailsOkButton:
					updateCode = DetailsApply (currentUID, &dueDate, categoryEdited, noteEdited);
					TraceOutput(TL(appErrorClass, "DetailsOkButton : DetailsApply returned 0x%lx", updateCode));
					ListViewUpdateDisplay(updateCode | updateItemMove);
					InDetails = false;
					FrmReturnToForm (ToDoSharedData.listViewId);
					handled = true;
					break;

				case DetailsCancelButton:
					updateCode = 0;

					if (categoryEdited)
					{
						// user cancelled so we restore previous category choice
						if (CurrentCategory != catIDAll)
						{
							category = catIDAll;
							ToDoOpenCursor(ToDoDB, &gListViewCursorID, &category, 1, priorityDueDateItem);
						}
						DbCursorMoveToRowID(gListViewCursorID, currentUID);
						DbSetCategory(ToDoDB, gListViewCursorID, sDetailsSavedNumCategories, sDetailsSavedCategoriesIDP);
						ChangeCategory(savedCurrentCategory);
						if (CurrentCategory != catIDAll)
							ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
						updateCode |= updateCategoryChanged;
					}

					ListViewUpdateDisplay(updateCode | updateItemMove);
					InDetails = false;
					FrmReturnToForm (ToDoSharedData.listViewId);
					handled = true;
					break;

				case DetailsDeleteButton:
					if ( DetailsDeleteToDo (currentUID))
					{
						InDetails = false;
						FrmReturnToForm (ToDoSharedData.listViewId);
						ListViewUpdateDisplay(updateItemDelete);
					}
					else
						ListViewUpdateDisplay(updateRedrawAll);
					handled = true;
					break;

				case DetailsCategoryTrigger:
					categoryEdited = DetailsSelectCategory (currentUID, &category) || categoryEdited;
					ListViewUpdateDisplay(updateRedrawAll);
					handled = true;
					break;
			}
			break;


		case popSelectEvent:
			if (event->data.popSelect.listID == DetailsDueDateList)
			{
				DetermineDueDate (event->data.popSelect.selection, &dueDate);
				DetailsSetDateTrigger (dueDate);
				handled = true;
			}
			break;

		case frmOpenEvent:
			objectID = 0xffff;
			savedCurrentCategory = CurrentCategory;
			InDetails = true;
			DetailsInit (&currentUID, &category, &dueDate);
			categoryEdited = false;
			noteEdited = false;
			handled = true;
			break;

		case frmCloseEvent:
			if (sDetailsCategoryIDsP)
			{
				DbReleaseStorage(ToDoDB, sDetailsCategoryIDsP);
				sDetailsCategoryIDsP = NULL;
				sDetailsNumCategories = 0;
			}

			if (sDetailsSavedCategoriesIDP)
			{
				DbReleaseStorage(ToDoDB, sDetailsSavedCategoriesIDP);
				sDetailsSavedCategoriesIDP = NULL;
				sDetailsSavedNumCategories = 0;
			}
			InDetails = false;
			break;

		case winUpdateEvent:
			if (event->data.winUpdate.window != gDetailsDialogWinH)
				break;
			frm = FrmGetFormPtr(DetailsDialog);
			FrmDrawForm (frm);
			handled = true;
			break;

	}

	return (handled);
}




/***********************************************************************
 *
 * FUNCTION:    ListViewNavHighlightItem
 *
 * DESCRIPTION: Sets the 5-way nav focus on a ToDo item
 *
 * PARAMETERS:	Form, Table, current field
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			lyr	08/05/04	Initial Revision
 *
 ***********************************************************************/

static void ListViewNavHighlightItem(FormPtr frmP, TablePtr tblP, FieldPtr fldP)
{
	RectangleType tblRect, fldRect;

	TblGetBounds(tblP, &tblRect);
	FldGetBounds(fldP, &fldRect);

	fldRect.topLeft.x = tblRect.topLeft.x;
	fldRect.extent.x = tblRect.extent.x;

	FrmSetFocusHighlight(FrmGetWindowHandle(frmP), &fldRect, 0);
	gTableNavFocus = true;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewRestoreEditState
 *
 * DESCRIPTION: This routine restores the edit state of the ToDo list
 *              if the list is in edit mode. This routine is
 *              called after the priority or due date of an item is
 *              changed, or after returning from the details dialog
 *              or note view.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/17/95	Initial Revision
 *			kwk	12/10/98	Constrain field selection values to mask app bug.
 *
 ***********************************************************************/
static void ListViewRestoreEditState ()
{
	int16_t row;
	FormPtr frm;
	TablePtr table;
	FieldPtr fld;

	TraceOutput(TL(appErrorClass, "ListViewRestoreEditState itemSelected : %d, CurrentRecord : %d",ItemSelected,CurrentRecord));
	if (!ItemSelected || InNoteView || InDetails) return;

	// Find the row that the current record is in.  Its possible
	// that the current record is no longer displayable (ex: only due
	// item are being display and the due date was changed
	// such that the record don't display).
	table = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
	if (!TblFindRowID(table, (uint16_t)CurrentRecord, &row) || TblRowMasked(table,row) || gMaskingRecord)
		{
		if (gMaskingRecord)
		{
			gMaskingRecord = false;
			if (PrivateRecordVisualStatus == maskPrivateRecords)
				TblSetRowMasked(table, row, true);
		}
		ClearEditState ();
		return;
		}

	frm = FrmGetFormPtr(ToDoSharedData.listViewId);
	FrmSetFocus (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));

	// Restore the insertion point position if we don't have to set a
	// selection
	if (ListEditSelectionLength == 0)
	{
		TblGrabFocus (table, row, descColumn);
		if ((fld = TblGetCurrentField (table)) != NULL)
		{
			FldGrabFocus (fld);
			TraceOutput(TL(appErrorClass, "ListViewRestoreEditState grabbed focus"));
			ListViewNavHighlightItem(frm, table, fld);
		}
	}
}



/***********************************************************************
 *
 * FUNCTION:    ListViewClearEditState
 *
 * DESCRIPTION: This routine clears the edit state of the ToDo list.
 *              It is called whenever a table item is selected.
 *
 *              If the new item selected is in a different row than
 *              the current record, the edit state is cleared,  and if
 *              current record is empty it is deleted.
 *
 * PARAMETERS:
 *	-> reMask : re-mask private records
 *
 * RETURNED:    true if the current record is deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/95	Initial Revision
 *			peter	4/24/00	Re-mask private record when leaving it.
 *			lyr	3/27/02	add parameter for re-masking record.
 *
 ***********************************************************************/
static Boolean ListViewClearEditState (Boolean reMask)
{
	int16_t row;
	int16_t rowsInTable;
	FormPtr frm;
	TablePtr tableP;
	uint16_t attr;

	TraceOutput(TL(appErrorClass, "ListViewClearEditState remask : %d, itemSelected : %d",reMask,ItemSelected));

	frm = FrmGetFormPtr (ToDoSharedData.listViewId);
	tableP = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));
	TblUnhighlightSelection(tableP);
	TblReleaseFocus(tableP);
	FrmSetFocus(frm, noFocus);
	FrmClearFocusHighlight();

	if (!ItemSelected)
		return (false);

	if (!TblFindRowID(tableP, (uint16_t)CurrentRecord, &row) )
		return (false);

	TraceOutput(TL(appErrorClass, "ListViewClearEditState record ID: %d index: %d, category: %d, row: %d", TblGetRowData(tableP, row), CurrentRecord, CurrentRecordCategory, row));

	TblUnhighlightSelection (tableP);
	TblReleaseFocus (tableP);

	// We're leaving a record. If it's secret and we're masking secret records
	// (but unmasked just this one), then remask it now.
	if (reMask && (CurrentRecordVisualStatus != PrivateRecordVisualStatus))
	{
		CurrentRecordVisualStatus = PrivateRecordVisualStatus;

		// Is the record still secret? It may have been changed from the
		// details dialog.
		DbCursorSetAbsolutePosition(gListViewCursorID, CurrentRecord);
		DbGetRowAttr(ToDoDB, gListViewCursorID, &attr);

		if (attr & dbRecAttrSecret)
		{
			// Re-mask the current row.
			TblSetRowMasked(tableP, row, true);

			// Draw the row masked.
			TblMarkRowInvalid (tableP, row);
			ListViewUpdateDisplay(updateRedrawAll);
			TraceOutput(TL(appErrorClass, "ListViewClearEditState masking row : %d",row));
		}
	}

	// If a different row has been selected, clear the edit state, this
	// will delete the current record if it's empty.
	if (ClearEditState ())
	{
		gTableNavFocus = false;
		rowsInTable = TblGetNumberOfRows (tableP);
		for (; row < rowsInTable; row++)
			TblSetRowUsable (tableP, row, false);

		ListViewUpdateDisplay(updateCategoryChanged);
		return (true);
	}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGetDescription
 *
 * DESCRIPTION: This routine returns a pointer to the description field
 *              of a ToDo record.  This routine is called by the table
 *              object as a callback routine when it wants to display or
 *              edit a ToDo description.
 *
 * PARAMETERS:  table  - pointer to the ToDo list table (TablePtr)
 *              row    - row of the table
 *              column - column of the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
status_t ListViewGetDescription (void * table, int16_t row, uint16_t column,
	Boolean editable, MemHandle *textHP, uint16_t * textOffset, uint16_t * textAllocSize,
	FieldPtr fld)
{
	uint32_t recordIndex, savedIndex, textSize;
	FieldAttrType attr;
	status_t err = errNone;

	// zero all unused parameters
	*textHP = NULL ;
	*textOffset = *textAllocSize = 0;

	if(!ToDoDB) {
		/*
		 * MJPG: we received a redraw while the database was closed
		 *       (typically because the system is asking for a password
		 *       and probably about to wipe all secret items because
		 *       i cannot never remember passwords.
		 *
		 *       the artifact is that we will see an empty table in some
		 *       cases.
		 */
		return errNone;
	}

	// Get the record number that corresponds to the table item.
	// The record number is stored as the row id.
	recordIndex = TblGetRowID(table, row);

	if (fld)
	{
		if (editable)
		{

			// Set the field to support auto-shift.
			FldGetAttributes (fld, &attr);
			attr.autoShift = true;
			FldSetAttributes (fld, &attr);
			err = DbCursorSetAbsolutePosition(gListViewCursorID, recordIndex);
			FldSetTextColumn(fld, ToDoDB, gListViewCursorID, recordIndex, todoDescriptionColId, 0);
			TraceOutput(TL(appErrorClass, "ListViewGetDescription editing record %d at row %d", recordIndex, row));
		}
		else
		{
			char * descItemP;
			char * handlePtr;
			if (EditedItemH)
			{
				MemHandleFree(EditedItemH);
				EditedItemH = NULL;
			}
			DbCursorGetCurrentPosition(gListViewCursorID, &savedIndex);
			DbCursorSetAbsolutePosition(gListViewCursorID, recordIndex);
			err = DbGetColumnValue(ToDoDB, gListViewCursorID, todoDescriptionColId, 0, (void**)&descItemP, &textSize);
			if (err >= errNone && descItemP)
			{
				// These are needed if there is something to display
				*textAllocSize = (uint16_t) textSize;
				*textHP = EditedItemH = MemHandleNew(*textAllocSize);
				*textOffset = 0;
				handlePtr = MemHandleLock(*textHP);
				StrCopy(handlePtr,descItemP);
				MemHandleUnlock(*textHP);
				DbReleaseStorage(ToDoDB, descItemP);
				TraceOutput(TL(appErrorClass, "ListViewGetDescription drawing record %d at row %d : %s", recordIndex, row, handlePtr));
			}
			DbCursorSetAbsolutePosition(gListViewCursorID, savedIndex);
		}
	}

	return (err < errNone ? err : errNone) ;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSaveDescription
 *
 * DESCRIPTION: This routine is called by the table object, as a callback
 *              routine, when it wants to save a ToDo description.
 *              The description is edit in place (directly in the database
 *              record),  so we don't need to save it here,  we do however
 *              want to capture the current edit state.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    true if the table needs to be redrawn
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static Boolean ListViewSaveDescription (void * table, int16_t row, uint16_t column)
{
	FieldPtr fld;

	// If the description has been modified mark the record dirty, any
	// change make to the ToDo's description were written directly
	// to the ToDo record.
	fld = TblGetCurrentField (table);

	// Save the dirty state, we're need it if we auto-delete an empty record.
	RecordDirty = FldDirty (fld);

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawDescForDatebook
 *
 * DESCRIPTION: Draw the description of a task.  This
 *              routine is called by the table object as a callback
 *              routine. This routine is designed for the Datebook agenda
 *				view such that only a single row is displayed for multi-line
 *				items.
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - region to draw in
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	8/30/99	Initial Revision
 *
 ***********************************************************************/
static void ListViewDrawDescForDatebook (void* inTableP, int16_t inRow,
	int16_t inColumn,	RectangleType* inBoundsP)
{
	uint32_t 		recordNum;
	char * 			descP ;
	uint32_t 		descLen ;
	char*			tempP;
	uint16_t		tempCount = 0;
	FontID 			curFont;

	// Get the record number that corresponds to the table item.
	// The record number is stored as the row id.
	recordNum = TblGetRowID (inTableP, inRow);
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	if (DbGetColumnValue(ToDoDB, gListViewCursorID, todoDescriptionColId, 0, (void**)&descP, &descLen))
		return;

	// Force the font
	curFont = FntSetFont (ListFont);

	tempP = descP;
	while (*tempP && *tempP != linefeedChr)
	{
		tempCount++;
		tempP++;
	}

	{
		WinDrawOperation oldDrawMode = WinSetDrawMode(winOverlay);
		WinPaintTruncChars (descP, tempCount, inBoundsP->topLeft.x, inBoundsP->topLeft.y,
							inBoundsP->extent.x);
		WinSetDrawMode(oldDrawMode);
	}

	DbReleaseStorage(ToDoDB, descP);

	// Restore the font
	FntSetFont (curFont);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewGetDescriptionHeight
 *
 * DESCRIPTION: This routine returns the height, in pixels, of a ToDo
 *              description.
 *
 * PARAMETERS:  recordNum - record index
 *              width     - width of description
 *
 * RETURNED:    height in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	10/27/99	Restore current font in masked-records case; fixes
 *								bug #23276.
 *			peter	4/24/00	Allow multi-line masked records.
 *
 ***********************************************************************/
static uint16_t ListViewGetDescriptionHeight (uint32_t recordNum, uint16_t width, uint16_t maxHeight)
{
	uint16_t height;
	uint16_t lineHeight;
	FontID curFont;
	Boolean masked;
	uint16_t attr;
	privateRecordViewEnum visualStatus;
	ToDoItemType item;
	DbSchemaColumnValueType * colVals ;

	//mask if appropriate
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	DbGetRowAttr(ToDoDB, gListViewCursorID, &attr);
	visualStatus = recordNum == CurrentRecord
		? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
	masked = (((attr & dbRecAttrSecret) && visualStatus == maskPrivateRecords));

	curFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();

	// The following code is commented out since masked records are no longer limited
	// to one line. The reason for this is to keep masking and unmasking of individual
	// records from affecting the position of records on the screen.
	//		if (masked)
	//			{
	//			FntSetFont (curFont);
	//			return lineHeight;
	//			}

	// If ToDo is loaded as a shared library, only one line is authorized for each item
	if (! LoadedAsSharedLib)
	{
		// Get a pointer to the ToDo record.
		DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
		ToDoColumnValuesToItem(colVals, &item, gNCols);

		// If the record has a note, leave space for the note indicator.
		if (itemHasNote(item))
			width -= tableNoteIndicatorWidth;

		// Compute the height of the ToDo item's description.

		height = (uint16_t)FldCalcFieldHeight (item.description, (Coord)width);
		height = min (height, (maxHeight / lineHeight));
		height *= lineHeight;

		DbReleaseStorage(ToDoDB, colVals);
	}
	else
		height = lineHeight;

	FntSetFont (curFont);

	return (height);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewPriorityFontID
 *
 * DESCRIPTION: This routine is called to determine the correct font to
 *						use for drawing the list view priority number - we
 *						want to bold the list view font.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    Font id for list view priority number.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kwk	06/23/99	New today.
 *
 ***********************************************************************/
FontID ListViewPriorityFontID (void)
{
	if (ListFont == stdFont)
		return (boldFont);
	else if (ListFont == largeFont)
		return (largeBoldFont);
	else
		return (ListFont);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewDrawDueDate
 *
 * DESCRIPTION: This routine draws a ToDo items due date.
 *
 * PARAMETERS:	 table  - pointer to a table object
 *              row    - row the item is in
 *              column - column the item is in
 *              bounds - region to draw in
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/14/95	Initial Revision
 *
 ***********************************************************************/
void ListViewDrawDueDate (void * table, int16_t row, int16_t column, RectanglePtr bounds)
{
	char dueChr;
	char dateBuffer [dateStringLength];
	char* dateStr;
	uint16_t dateStrLen;
	uint16_t dueChrWidth;
	int16_t drawX, drawY;
	FontID curFont;
	FontID fontID;
	DateType date, sysToday;
	WinDrawOperation oldDrawMode;

	// Get the due date to the item being drawn.
	*((int16_t *) (&date)) = TblGetItemInt (table, row, dueDateColumn);

	// If there is no date draw a dash to indicate such.
	if (DateToInt (date) == toDoNoDueDate)
	{
		curFont = FntSetFont (stdFont);
		drawX = bounds->topLeft.x + ((bounds->extent.x - 5) >> 1);
		drawY = bounds->topLeft.y + ((FntLineHeight () + 1) / 2);
		WinDrawLine (drawX, drawY, (Coord) (drawX+5), drawY);
		FntSetFont (curFont);
		return;
	}

	// Get the width of the character that indicates the item is due.  Don't
	// count the whitespace in the character.
	fontID = ListViewPriorityFontID();
	curFont = FntSetFont (fontID);
	dueChr = '!';
	dueChrWidth = FntCharWidth (dueChr) - 1;

	FntSetFont (ListFont);

	DateToAscii ((uint8_t)date.month, (uint8_t)date.day, (int16_t) (date.year + firstYear), DateFormat, dateBuffer);

	// Remove the year from the date string.
	dateStr = dateBuffer;
	if ((DateFormat == dfYMDWithSlashes) ||
		 (DateFormat == dfYMDWithDots) ||
		 (DateFormat == dfYMDWithDashes))
		dateStr += 3;
	else
		dateStr[strlen(dateStr) - 3] = 0;

	// Draw the due date, right aligned.
	dateStrLen = strlen (dateStr);
	drawX = bounds->topLeft.x + bounds->extent.x - dueChrWidth -
		FntCharsWidth (dateStr, dateStrLen);
	drawY = bounds->topLeft.y ;

	oldDrawMode = WinSetDrawMode(winOverlay);
	WinPaintChars (dateStr, dateStrLen, drawX, drawY);

	// If the date is on or before today draw an exclamation mark.
	// Get today from system when launched from DateBook as a shared library.
	if (LoadedAsSharedLib)
		DateSecondsToDate (TimGetSeconds(), &sysToday);
	else
		sysToday = Today;

	if (DateToInt(date) < DateToInt(sysToday))
	{
		drawX = bounds->topLeft.x + bounds->extent.x - dueChrWidth;
		FntSetFont (fontID);
		WinPaintChars (&dueChr, 1, drawX, drawY);
	}

	WinSetDrawMode(oldDrawMode);

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawCategory
 *
 * DESCRIPTION: This routine draws a ToDo item's category name.
 *
 * PARAMETERS:	 table  - pointer to a table object
 *              row    - row the item is in
 *              column - column the item is in
 *              bounds - region to draw in
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/21/96	Initial Revision
 *
 ***********************************************************************/
void ListViewDrawCategory (void * table, int16_t row, int16_t column, RectanglePtr bounds)
{
	Coord width;
	size_t length;
	CategoryID * categoriesP = NULL;
	CategoryID indexCategory ;
	uint32_t recordNum, numCategories;
	Boolean fits;
	FontID curFont;
	WinDrawOperation oldDrawMode;

	curFont = FntSetFont (ListFont);

	// Get the category of the item in the specified row.
	recordNum = TblGetRowID (table, row);

	// Get record categories
	if ((indexCategory = (CategoryID) DbCursorSetAbsolutePosition(gListViewCursorID, recordNum)) < errNone
	||	DbGetCategory(ToDoDB, gListViewCursorID, &numCategories, &categoriesP) < errNone)
		goto Exit ;

	// Get the name of the category and truncate it to fix the the
	// column passed.
	if (sortingByCategories(CurrentSortId))
		CatMgrTruncateName(ToDoDB, numCategories ? &indexCategory : NULL,
			numCategories ? 1 : 0,
			truncCategoryMaxWidth, sCategoryName);
	else
		CatMgrTruncateName(ToDoDB, categoriesP, numCategories,
			truncCategoryMaxWidth, sCategoryName);
	width = bounds->extent.x;
	length = strlen(sCategoryName);
	FntCharsInWidth (sCategoryName, &width, &length, &fits);

	// Draw the category name.
	oldDrawMode = WinSetDrawMode(winOverlay);
	WinPaintChars (sCategoryName, length, bounds->topLeft.x,
		bounds->topLeft.y);
	WinSetDrawMode(oldDrawMode);

	FntSetFont (curFont);

Exit:
	if (categoriesP) {
		DbReleaseStorage(ToDoDB, categoriesP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the list view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frm             - pointer to the ToDo list form
 *              bottomRecord    - record index of the last visible record
 *              lastItemClipped - true if the last item display is not fully
 *                                visible
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewUpdateScrollers (FormPtr frm, uint32_t bottomRecord, Boolean lastItemClipped)
{
	uint16_t 	upIndex, downIndex;
	uint32_t 	recordNum;
	Boolean 	scrollableUp, scrollableDown;

	TraceOutput(TL(appErrorClass, ">>> ListViewUpdateScrollers top: %d Bottom: %d ",
		TopVisibleRecord, bottomRecord));

	// If the first record displayed is not the first record in the category,
	// enable the winUp scroller.
	recordNum = TopVisibleRecord;
	scrollableUp = SeekRecord(&recordNum, 1, dmSeekBackward);


	// If the last record displayed is not the last record in the category,
	// or the list item is clipped, enable the winDown scroller.
	recordNum = bottomRecord;
	scrollableDown = SeekRecord(&recordNum, 1, dmSeekForward) || lastItemClipped;


	// Update the scroll button.
	upIndex = FrmGetObjectIndex (frm, ListUpButton);
	downIndex = FrmGetObjectIndex (frm, ListDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    ListInitTableRow
 *
 * DESCRIPTION: This routine initialize a row in the ToDo list.
 *
 * PARAMETERS:  table      - pointer to the table of ToDo items
 *              row        - row number (first row is zero)
 *              recordNum  - the index of the record display in the row
 *              rowHeight  - height of the row in pixels
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void ListInitTableRow (TablePtr table, int16_t row, uint32_t recordNum,
	int16_t rowHeight)
{
	uint32_t	uniqueID;
	DbSchemaColumnValueType * colVals ;
	ToDoItemType item;

	// Get a pointer to the ToDo record.
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &item, gNCols);

	// Make the row usable.
	TblSetRowUsable (table, row, true);

	// Set the height of the row to the height of the description.
	TblSetRowHeight (table, row, rowHeight);

	// Store the record number as the row id.
	TblSetRowID (table, row, (uint16_t)recordNum);

	// Store the unique id of the record in the table.
	DbCursorGetCurrentRowID(gListViewCursorID, &uniqueID);
	TblSetRowData (table, row, uniqueID);

	// Set the checkbox that indicates the completion status.
	TblSetItemInt (table, row, completedColumn,
		(int16_t) item.completed);

	// Store the priority in the table.
	TblSetItemInt (table, row, priorityColumn,
		(int16_t) item.priority);

	// Store the due date in the table.
	TblSetItemInt (table, row, dueDateColumn, (*(int16_t *) &item.dueDate));

	// If ToDo has not been used as a shared library (item are thus editable),
	// set the table item type for the description, it will differ depending
	// on the presents of a note.
	if (! LoadedAsSharedLib)
	{
		if (itemHasNote(item))
			TblSetItemStyle (table, row, descColumn, textWithNoteTableItem);
		else
			TblSetItemStyle (table, row, descColumn, textTableItem);
	}

	TraceOutput(TL(appErrorClass, "ListInitTableRow row: %d index: %d completed: %d priority: %d duedate: %x hasnote: %d %s",
		row, recordNum, item.completed, item.priority, (*(int16_t *) &item.dueDate), item.note ? 1 : 0, item.description));

	DbReleaseStorage(ToDoDB, colVals);

	// Mark the row invalid so that it will drawn when we call the
	// draw routine.
	TblMarkRowInvalid (table, row);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewLoadTable
 *
 * DESCRIPTION: This routine reloads ToDo database records into
 *              the list view.  This routine is called when:
 *              	o A new item is inserted
 *              	o An item is deleted
 *              	o The priority or due date of an items is changed
 *              	o An item is marked complete
 *              	o Hidden items are shown
 *              	o Completed items are hidden
 *
 * PARAMETERS:  fillTable - if true the top visible item will be scroll winDown
 *                          such that a full table is displayed
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
  *		jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *			peter	4/24/00	Add support for unmasking only the selected record.
 *
 ***********************************************************************/
void ListViewLoadTable (Boolean fillTable)
{
	int16_t				row, emptyRow;
	uint32_t			numRows;
	uint32_t			recordNum;
	uint16_t			dataHeight;
	uint16_t			lineHeight;
	uint16_t			tableHeight;
	uint16_t			columnWidth;
	uint32_t			pos, oldPos;
	uint16_t			height, oldHeight;
	uint32_t			uniqueID;
	FontID				curFont;
	Boolean				rowUsable;
	Boolean				rowsInserted = false;
	Boolean				lastItemClipped;
	FormPtr				frm;
	TablePtr			table;
	ControlPtr			ctl;
	RectangleType		r;
	uint16_t 			attr;
	Boolean 			masked;
	privateRecordViewEnum visualStatus;

	PIMAppProfilingBegin("ListViewLoadTable");

	// Initializations
	frm = FrmGetFormPtr (ToDoSharedData.listViewId);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));
	numRows = TblGetNumberOfRows (table);

	// Set the label of the category trigger.
	ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ToDoSharedData.ListCategoryTriggerId));
	CatMgrSetTriggerLabel(ToDoDB, &CurrentCategory, 1, ctl, CategoryName);


	TraceOutput(TL(appErrorClass, "ListViewLoadTable (%s) %d rows", fillTable ? "fill table" : "no fill table", numRows));

	// Make sure the global variable that holds the index of the
	// first visible record has a valid value.
	if (! SeekRecord(&TopVisibleRecord, 0, dmSeekForward))
		if (! SeekRecord(&TopVisibleRecord, 0, dmSeekBackward))
		{
			TopVisibleRecord = 1;
		}

	// If we have a currently selected record, make sure that it is not
	// above the first visible record.
	if (CurrentRecord != noRecordSelected)
	{
		if (CurrentRecord < TopVisibleRecord)
		{
			CurrentRecord = TopVisibleRecord;
		}
	}

	// Get the height of the table and the width of the description
	// column.
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (table, descColumn);

	// Get the height one one line.
	curFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();
	FntSetFont (curFont);

	row = 0;
	dataHeight = 0;
	oldPos = pos = 0;
	recordNum = TopVisibleRecord;
	LastVisibleRecord = recordNum;

	TraceOutput(TL(appErrorClass, "<<<<<< LOAD TABLE - START FROM TOP: %d >>>>", TopVisibleRecord));

	// Load records into the table.
	while (SeekRecord(&recordNum, 0, dmSeekForward))
	{
		// Compute the height of the ToDo item's description.
		height = ListViewGetDescriptionHeight (recordNum, columnWidth, tableHeight);
		TraceOutput(TL(appErrorClass, "load table record %d tableHeight %d dataHeight %d lineHeight %d",
			recordNum, tableHeight, dataHeight, lineHeight));

		// Is there enough room for at least one line of the the decription.
		if (row < (int16_t)numRows && tableHeight >= dataHeight + lineHeight)
		{
			// Get the height of the current row.
			rowUsable = TblRowUsable (table, row);
			if (rowUsable)
				oldHeight = TblGetRowHeight (table, row);
			else
				oldHeight = 0;

			DbGetRowAttr(ToDoDB, gListViewCursorID, &attr);
			visualStatus = recordNum == CurrentRecord
				? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
			masked = (((attr & dbRecAttrSecret) && visualStatus == maskPrivateRecords));

			if (masked != TblRowMasked (table, row))
				TblMarkRowInvalid (table, row);

			TblSetRowMasked (table, row, masked);

			// Determine if the row needs to be initialized.  We will initialize
			// the row if: the row is not usable (not displayed),  the unique
			// id of the record does not match the unique id stored in the
			// row. Or if we're sorting by categories
			DbCursorGetRowIDForPosition(gListViewCursorID, recordNum, &uniqueID);
			if (viewSortingByCategories ||
				(TblGetRowData (table, row) != uniqueID) ||
				(! TblRowUsable (table, row)) ||
				(TblRowInvalid (table, row)))
			{
				ListInitTableRow (table, row, recordNum, height);
			}

			// If the height or the position of the item has changed draw the item.
			else
			{
				TblSetRowID (table, row, (uint16_t)recordNum);
				if (height != oldHeight)
				{
					TblSetRowHeight (table, row, height);
					TblMarkRowInvalid (table, row);
				}
				else if (pos != oldPos)
				{
					TblMarkRowInvalid (table, row);
				}
			}

			LastVisibleRecord = recordNum;

			pos += height;
			oldPos += oldHeight;

			row++;
			recordNum++;
		}

		dataHeight += height;

		// Is the table full?
		if (row >= (int16_t)numRows || dataHeight >= tableHeight)
		{
			TraceOutput(TL(appErrorClass, "load table full"));
			// If we have a currently selected record, make sure that it is
			// not below  the last visible record.
			if (CurrentRecord == noRecordSelected || CurrentRecord < LastVisibleRecord)
				break;

			if (CurrentRecord == LastVisibleRecord &&		// Last visible?
				(CurrentRecord == TopVisibleRecord || dataHeight == tableHeight))
				break;

			TopVisibleRecord++;
			recordNum = TopVisibleRecord;
			TraceOutput(TL(appErrorClass, "load table TopVisibleRecord %d ",TopVisibleRecord));
			row = 0;
			dataHeight = 0;
			oldPos = pos = 0;
		}
	}

	// Hide the items that don't have any data.
	emptyRow = row;
	while (emptyRow < (int16_t)numRows)
	{
		TblSetRowUsable (table, emptyRow, false);
		emptyRow++;
	}

	// If we're not asked to fill the table
	if (!fillTable)
		goto Exit;

	// If the table is not full and the first visible record is
	// not the first record	in the database, displays enough records
	// to fill out the table.
	recordNum = TopVisibleRecord;

	TblUnhighlightSelection(table);
	TblReleaseFocus(table);
	//FrmSetFocus(frm, noFocus);

	while (dataHeight < tableHeight)
	{
		if (row >= (int16_t)numRows || ! SeekRecord(&recordNum, 1, dmSeekBackward))
			break;

		// Compute the height of the ToDo item's description.
		height = ListViewGetDescriptionHeight (recordNum, columnWidth, tableHeight);

		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;

		row++;
		TraceOutput(TL(appErrorClass, "<<<<<< LOAD TABLE - SHIFT UP >>>>"));
		TraceOutput(TL(appErrorClass, "(%d rows) Height of record %d = %d", row, recordNum, height));

		// Insert a row before the first row.
		TblInsertRow (table, 0);
		ListInitTableRow (table, 0, recordNum, height);

		//mask if appropriate
		DbGetRowAttr(ToDoDB, gListViewCursorID, &attr);
		visualStatus = recordNum == CurrentRecord
			? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
   		masked = (((attr & dbRecAttrSecret) && visualStatus == maskPrivateRecords));
		TblSetRowMasked(table,0,masked);

		rowsInserted = true;
		dataHeight += height;

		TopVisibleRecord = recordNum;
		TraceOutput(TL(appErrorClass, "load table TOP: %d ", TopVisibleRecord));
	}

	// If rows were inserted to full out the page, invalidate the whole
	// table, it all needs to be redrawn.
	if (rowsInserted)
		ListViewDrawTable (updateRedrawAll);

Exit:
	// If the height of the data in the table is greater than the height
	// of the table, then the bottom of the last row is clipped and the
	// table is scrollable.
	lastItemClipped = (dataHeight > tableHeight);

	// Update the scroll arrows.
	if (!LoadedAsSharedLib)
		ListViewUpdateScrollers (frm, LastVisibleRecord, lastItemClipped);

	PIMAppProfilingEnd();

	TraceOutput(TL(appErrorClass, "<<<<<< LOAD TABLE - NEW TOP: %d >>>>", TopVisibleRecord));
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawTable
 *
 * DESCRIPTION: Updates the entire list view, such as when changing categories
 *
 * PARAMETERS:  updateCode - indicates how (or whether) to rebuild the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *
 ***********************************************************************/
void ListViewDrawTable (uint16_t updateCode)
{
	WinHandle winHndH;
	FormPtr frm;

	frm = FrmGetFormPtr(ToDoSharedData.listViewId);
	if (updateCode & (updateDisplayOptsChanged | updateFontChanged))
	{
		ListViewInit (frm);
	}
	else if (updateCode & (updateCategoryChanged | updateGoTo))
	{
		ListViewLoadTable (true);
	}

	// Do not use gListViewWinH as argument of WinInvalidateWindow because
	// this function can be called from DateBook as a shared library
	winHndH = FrmGetWindowHandle(frm);
	WinInvalidateWindow(winHndH);
	gTblDrawCode = tblDraw;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewRedrawTable
 *
 * DESCRIPTION: Redraw the rows of the table that are marked invalid
 *
 * PARAMETERS:  fillTable - if true the top visible item will be scroll down
 *                          such that a full table is displayed
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *
 ***********************************************************************/
void ListViewRedrawTable (Boolean fillTable)
{
	TablePtr table;
	FormPtr frm;
	WinHandle winHndH;

	frm = FrmGetFormPtr (ToDoSharedData.listViewId);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));

	ListViewLoadTable (fillTable);

	// Do not use gListViewWinH as argument of WinInvalidateWindow because
	// this function can be called from DateBook as a shared library
	winHndH = FrmGetWindowHandle(frm);
	WinInvalidateWindow(winHndH);
	gTblDrawCode = tblRedraw;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewNewToDo
 *
 * DESCRIPTION: This routine adds a new ToDo item to the ToDo list.
 *              If a ToDo item is currently selected, the new item
 *              will be added after the selected item.  If not, the
 *              new item will be added after the last priority "one" item.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	5/1/95		Initial Revision
 *			peter	4/24/00		Add support for re-masking private record.
 *
 ***********************************************************************/
static void ListViewNewToDo (void)
{
	status_t 	error;
	uint16_t 	i;
	int16_t 	row, column;
	uint32_t 	recordNum;
	int16_t 	rowsInTable;
	CategoryID 	category;
	CategoryID 	* catIDsP;
	uint32_t 	nCatIDs;
	FontID 		curFont;
	FormPtr 	frm;
	TablePtr 	table;
	Boolean 	empty;
	Boolean 	found;
	ToDoItemType newToDo;
	DbSchemaColumnValueType * colVals = NULL ;


	frm = FrmGetFormPtr (ToDoSharedData.listViewId);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));

	// If we're showing all categories the new item will be uncategorized.
	if (CurrentCategory == catIDAll)
		category = catIDUnfiled;
	else
		category = CurrentCategory;

	// If a ToDo item is selected, insert the new item after the current
	// selection.
	if (ItemSelected)
	{
		TblGetSelection (table, &row, &column);
		recordNum = TblGetRowID (table, row);

		// Check if the current record is empty, if it is, don't insert
		// a new record.
		DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
		DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
		ToDoColumnValuesToItem(colVals, &newToDo, gNCols);

		empty = itemIsEmpty(newToDo);
		DbReleaseStorage(ToDoDB, colVals);
		if (empty)
		{
			ListViewRestoreEditState ();
			return ;
		}

		// Save the record.
		// This was done by calling TblReleaseFocus, but that doesn't deal with
		// the possibility that the focus is currently in an unmasked private
		// record which needs to be re-masked.
		ListViewClearEditState (true);

		DbGetCategory(ToDoDB, gListViewCursorID, &nCatIDs, &catIDsP);
		error = ToDoInsertNewRecord (ToDoDB, &gListViewCursorID, &recordNum, catIDsP, nCatIDs);
		if (catIDsP)
			DbReleaseStorage(ToDoDB, catIDsP);

		// Display an alert that indicates that the new record could
		// not be created.
		if (error)
		{
			FrmUIAlert(DeviceFullAlert);
			return ;
		}

		// Insert a row into the table, after the currently selected row.
		rowsInTable = TblGetNumberOfRows (table);
		if (row != TblGetLastUsableRow (table))
		{
			TblInsertRow (table, row);
			// Shift the rows because of insertion
			curFont = FntSetFont (ListFont);
			ListInitTableRow (table, (int16_t) (row+1), recordNum, FntLineHeight() );
			FntSetFont (curFont);

			// Invalidate all the rows from the inserted row to the end of the
			// table so that they will be redrawn.
			for (i = row; i < rowsInTable; i++)
				TblMarkRowInvalid (table, i);
		}

		// If we're inserting after the last visible row, force the table
		// to scroll up one row.
		else if (rowsInTable > 1)
		{
			if (row == 0)
				TopVisibleRecord = recordNum;
			else
				TopVisibleRecord = TblGetRowID (table, 1);
		}
	}


	// Add a new ToDo item after all the priority "one" items.
	else
	{
		newToDo.priority = defaultPriority;
		*((uint16_t *) &newToDo.dueDate) = toDoNoDueDate;
		newToDo.description = NULL;
		newToDo.note = NULL;
		newToDo.completed = false ;

		error = ToDoNewRecord (ToDoDB, &gListViewCursorID, &newToDo, &recordNum, category);

		// Display an alert that indicates that the new record could
		// not be created.
		if (error)
		{
			FrmUIAlert(DeviceFullAlert);
			return ;
		}

		if (TopVisibleRecord == recordNum)
		{
			// Invalidate all the rows so that they will be drawn.
			rowsInTable = TblGetNumberOfRows (table);
			for (i = 0; i < rowsInTable; i++)
				TblSetRowUsable (table, i, false);
		}
		else
			TopVisibleRecord = recordNum;
	}

	CurrentRecord = recordNum;

	ToDoDbCursorRequery(&gListViewCursorID);

	ListViewUpdateDisplay(updateCategoryChanged);

	// Give the focus to the new item.
	found = TblFindRowID (table, (uint16_t)CurrentRecord, &row);
	frm = FrmGetFormPtr(ToDoSharedData.listViewId);
	FrmSetFocus (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));
	TblGrabFocus (table, row, descColumn);
	FldGrabFocus (TblGetCurrentField (table));
	ItemSelected = true;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDeleteToDo
 *
 * DESCRIPTION: This routine deletes the selected ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewDeleteToDo (void)
{
	uint16_t i;
	int16_t row = 0;
	int16_t column;
	uint16_t numRows;
	uint32_t recordNum;
	TablePtr table;

	// If no ToDo item is selected, return.
	table = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);

	if (CurrentRecord == noRecordSelected)
	{
		// Check if we are editing an item.
		if (! TblEditing (table))
			return;

		TblGetSelection (table, &row, &column);
		FrmSetFocus(FrmGetFormPtr(ToDoSharedData.listViewId), noFocus);

		// Check if the record is empty, if it is, clear the edit state,
		// this will delete the current record when it's blank.
		recordNum = TblGetRowID (table, row);
	}
	else
		recordNum = CurrentRecord ;

	if (! ClearEditState ())
		{
		// Display an alert to confirm the delete operation, and delete the
		// record if the alert is confirmed.
		if (! DeleteRecord (recordNum))
			{
				ListViewUpdateDisplay(updateRedrawAll);		// Re-masks the record if necessary.
				return;
			}
		}

	// Invalid the row deleted and all the row following the deleted record so
	// that they will redraw.
	numRows = TblGetNumberOfRows (table);
	for (i = row; i < numRows; i++)
		TblSetRowUsable (table, i, false);

	ListViewUpdateDisplay(updateItemDelete);

	ItemSelected = false;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDeleteNote
 *
 * DESCRIPTION: This routine deletes the note attached to the selected
 *              ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewDeleteNote ()
{
	uint16_t i;
	int16_t row;
	int16_t column;
	uint16_t height;
	uint16_t newHeight;
	uint16_t tableHeight;
	int16_t columnWidth;
	uint32_t recordNum;
	int16_t rowsInTable;
	TablePtr table;
	Boolean empty;
	RectangleType r;
	ToDoItemType item;
	DbSchemaColumnValueType * colVals;

	table = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);

	// Get the selected record.
	TblGetSelection (table, &row, &column);
	recordNum = TblGetRowID (table, row);

	// Check if the record has a note attached.
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &item, gNCols);
	empty = ! itemHasNote(item);
	DbReleaseStorage(ToDoDB, colVals);
	if (empty) return;

	// Confirm that the note should be deleted.
	if (FrmAlert(gApplicationDbP, DeleteNoteAlert) != DeleteNoteYes)
		return;

	TblReleaseFocus (table);

	// Get the current height of the description.
	height = TblGetRowHeight (table, row);

	// Remove the note from the record.
	ToDoChangeRecord (ToDoDB, gListViewCursorID, &recordNum, toDoNote, "");

	// Mark the current row non-usable so the it will redraw.
	TblSetRowUsable (table, row, false);

	// Get the new height of the description, the desciption may be short
	// because we can draw in the space vacated by the note indicator.
	columnWidth = TblGetColumnWidth (table, descColumn);
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	newHeight =  ListViewGetDescriptionHeight (recordNum, columnWidth, tableHeight);

	// If the height of the description has changed, invalid all the row
	// after the current row so that they be redrawn.
	if (height != newHeight)
		{
		rowsInTable = TblGetNumberOfRows (table);
		for (i = row+1; i < rowsInTable; i++)
			TblSetRowUsable (table, i, false);
		}

	ListViewUpdateDisplay(updateCategoryChanged);

	ListViewRestoreEditState ();
}

/***********************************************************************
 *
 * FUNCTION:    ListViewCrossOutItem
 *
 * DESCRIPTION: This routine is called when a ToDo item is marked
 *              complete.  If completed item are not display then
 *              we display an animation of a line being drawn through
 *              the ToDo item.
 *
 * PARAMETERS:  -> tableP : Pointer on main view table
 *				-> row : the row to update
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/21/96	Initial Revision
 *
 ***********************************************************************/
static void ListViewCrossOutItem (TablePtr tableP, int16_t row)
{
	Coord 		lineHeight, maxWidth, lineWidth, y;
	size_t 		length, charsToDraw;
	uint32_t 	recordNum;
	FontID 		curFont;
	char* 		chars;
	RGBColorType lineColor, savedColor;
	RectangleType r;
	RectangleType tableR;
	Boolean		truncated;

	DbSchemaColumnValueType * colVals ;
	ToDoItemType toDoRec;

	// Get a pointer to the ToDo record.
	recordNum = TblGetRowID (tableP, row);
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &toDoRec, gNCols);

	curFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();

	TblGetBounds (tableP, &tableR);

	TblGetItemBounds (tableP, row, descColumn, &r);
	maxWidth = r.extent.x;

	// If the record has a note, leave space for the note indicator.
	if (itemHasNote(toDoRec))
		maxWidth -= tableNoteIndicatorWidth;

	chars = toDoRec.description;
	y = r.topLeft.y + (lineHeight >> 1);

	if (chars)
	{
		lineColor.index = 0xff;
		lineColor.r = lineColor.g = lineColor.b = 0;
		WinSetForeColorRGB(&lineColor, &savedColor);

		if (LoadedAsSharedLib)
		{
			// Crossout only one line (called as a shared lib from the DateBook)
			FntCharsInWidth(chars, &maxWidth, &length, &truncated);
			WinDrawLine (r.topLeft.x, y, r.topLeft.x + maxWidth, y);
		}
		else
		{
			// Crossout multiple lines
			length = 0;
			while (*chars)
			{
				// Get the number of character on each line.
				length = FldWordWrap (chars, maxWidth);

				// Don't draw trailing spaces and linefeed chars
				charsToDraw = length;
				while (	charsToDraw > 0
				&&	((chars[charsToDraw-1] == linefeedChr) || (chars[charsToDraw-1] == spaceChr)))
					charsToDraw--;

				lineWidth = FntCharsWidth(chars, charsToDraw);
				WinDrawLine (r.topLeft.x, y, r.topLeft.x + lineWidth, y);
				chars += length;
				y += FntLineHeight ();
				if (y > tableR.topLeft.y + tableR.extent.y)
					break;
			}
		}
		WinSetForeColorRGB(&savedColor, &lineColor);
	}

	DbReleaseStorage(ToDoDB, colVals);
	FntSetFont (curFont);

	// Flush drawings right now because we'll wait before sending
	// next update event which will make it disappear
	WinFlush();
}


/***********************************************************************
 *
 * FUNCTION:    ListViewChangeCompleteStatus
 *
 * DESCRIPTION: This routine is called when a ToDo item is marked
 *              complete.  If completed items are not displayed
 *              (a preference setting),  this routine will remove the
 *              item from the list.
 *
 * PARAMETERS:  row      - row in the table
 *              complete - true if the item is marked complete
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
void ListViewChangeCompleteStatus (int16_t row, uint16_t complete)
{
	uint32_t 	recordNum;
	Boolean 	deleted = false;
	TablePtr 	tableP;
	Boolean		multipleRowsInvolved = false;

	tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
	recordNum = TblGetRowID (tableP, row);

	// If completed item are not shown then display an animation
	// of a line being drawn through the item.
	if (! ShowCompletedItems)
#ifdef BACKBUFFERED
		ListViewCrossOutItem (tableP, row);
#else
		gCrossOutRow = row;
#endif

	// Update the record to reflect the new completion status.
	ToDoChangeRecord (ToDoDB, gListViewCursorID, &recordNum, toDoComplete, &complete);

	// Should the due date be changed to the completion date?
	if (complete && ChangeDueDate)
	{
		ToDoChangeRecord (ToDoDB, gListViewCursorID, &recordNum, toDoDueDate, &Today);
		CurrentRecord = recordNum;
	}

	// If completed items are shown and the date was changed, redraw
	// the list.
	if (ShowCompletedItems)
	{
		if (LoadedAsSharedLib && multipleRowsInvolved)
			ListViewUpdateDisplay(frmRedrawUpdateCode);
		else
		{
			// If multiple rows are involved, reload the whole tableP
			if (multipleRowsInvolved || (complete && ChangeDueDate) || viewSortingByCategories)
			{
				TblMarkTableInvalid(tableP);
				ListViewUpdateDisplay(updateItemMove);
			}
			else
				ListViewRestoreEditState ();
		}
	}

	// If completed items are hidden, update the tableP.
	else
	{
		if (!LoadedAsSharedLib)
			deleted = ClearEditState ();

#ifdef BACKBUFFERED
		// If the current record wasn't empty, delay before redrawing the
		// tableP so that the crossout animation may be seen.
		if (! deleted)
			SysTaskDelay(crossOutDelay);
#endif

		// Redraw tableP with deleted completed items
		ListViewUpdateDisplay(frmRedrawUpdateCode);
	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectDueDate
 *
 * DESCRIPTION: This routine is called when a "due date" item in the do to list
 *              is selected.  The due date  popup list is displayed; if
 *              the due date of the item is changed, the record is updated
 *              and the re-sorted list is redrawn.
 *
 * PARAMETERS:  table - ToDo table
 *              row   - row in the table
 *
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95	Initial Revision
 *			peter	10/10/00	Don't move record if due date isn't changed.
 *			peter	1/12/01	Restore edit state if due date isn't changed.
 *
 ***********************************************************************/
static void ListViewSelectDueDate (TablePtr table, int16_t row)
{
	int16_t itemSeleted;
	uint32_t recordNum;
	uint32_t newRecordNum;
	ListPtr lst;
	DateType oldDueDate, newDueDate;
	RectangleType r;
	DbSchemaColumnValueType * colVals ;
	ToDoItemType toDoRec;

	lst = GetObjectPtr (ListDueDateList, ListView);

	// Unhighlight the selected item.
	TblUnhighlightSelection (table);

	// Get the due date from the ToDo record.
	recordNum = TblGetRowID (table, row);
	DbCursorSetAbsolutePosition(gListViewCursorID, recordNum);
	DbGetColumnValues(ToDoDB, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &toDoRec, gNCols);
	oldDueDate = newDueDate = toDoRec.dueDate;
	DbReleaseStorage(ToDoDB, colVals);

	// Set the popup list's selection.
	if (DateToInt (oldDueDate) == toDoNoDueDate)
		LstSetSelection (lst, noDueDateItem);
	else
		LstSetSelection (lst, selectDateItem);


	// Position the list.
	TblGetItemBounds (table, row, dueDateColumn, &r);
	LstSetPosition (lst, r.topLeft.x, r.topLeft.y);

	// Display the list until a selection is made.
	itemSeleted = LstPopupList (lst);

	// Minus one indicates the popup list was dismissed without a selection
	// being made.
	if (itemSeleted == -1)
		goto NoChange;

	DetermineDueDate (itemSeleted, &newDueDate);

	// Don't update the record if the due date selected is the same as before.
	if (!memcmp (&oldDueDate, &newDueDate, sizeof (DateType)))
		goto NoChange;

	// Update the database record.
	newRecordNum = recordNum;
	ToDoChangeRecord (ToDoDB, gListViewCursorID, &newRecordNum, toDoDueDate, &newDueDate);

	// Changing the due date may change the record's index.
	CurrentRecord = newRecordNum;

	// Make sure the row is redrawn.
	TblMarkRowInvalid (table, row);

	// Send an event that will cause the view to be redrawn.
	ListViewUpdateDisplay(updateItemMove);
	return;

NoChange:
	ListViewRestoreEditState ();
	return;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectPriority
 *
 * DESCRIPTION: This routine is called when a "priority" item in the do to list
 *              is selected.  A popup list of priority is displayed; if
 *              the priority of an item is changed, the record is updated
 *              and the re-sorted list is redrawn.
 *
 * PARAMETERS:  table - ToDo table
 *              row   - row in the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewSelectPriority (TablePtr table, int16_t row)
{
	int16_t priority;
	int16_t newPriority;
	uint32_t recordNum;
	uint32_t newRecordNum;
	ListPtr lst;
	RectangleType r;

	lst = GetObjectPtr (ListPriorityList, ListView);

	// Unhighlight the priority.
	TblUnhighlightSelection (table);

	// Set the list's selection to the current priority.
	priority = TblGetItemInt (table, row, priorityColumn);
	LstSetSelection (lst, (int16_t) (priority-1));

	// Position the list.
	TblGetItemBounds (table, row, priorityColumn, &r);
	LstSetPosition (lst, r.topLeft.x, r.topLeft.y);

	newPriority = LstPopupList (lst);

	// Minus one indicates the popup list was dismissed without a selection
	// being made.
	if ((newPriority == -1) || (newPriority+1 == priority))
		{
		ListViewRestoreEditState ();
		return;
		}

	// Update the database record.
	newPriority++;									// one base the priority
	recordNum = TblGetRowID (table, row);
	newRecordNum = recordNum;
	ToDoChangeRecord (ToDoDB, gListViewCursorID, &newRecordNum, toDoPriority, &newPriority);

	// Changing the priority may change the record's index.
	CurrentRecord = newRecordNum;

	// Make sure the row is redrawn.
	TblMarkRowInvalid (table, row);

	// Send an event that will cause the view to be redrawn.
	ListViewUpdateDisplay(updateItemMove);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectItemsCategory
 *
 * DESCRIPTION: This routine is called when a "category" item in the to do
 *              list is selected.  A popup list of categories is displayed,
 *              if the category of an item is changed, the record is updated
 *              and the re-sorted list is redrawn.
 *
 * PARAMETERS:  table - ToDo table
 *              row   - row in the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/12/96	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryHideEditCategory
 *
 ***********************************************************************/
static void ListViewSelectItemsCategory (TablePtr table, int16_t row)
{
	int16_t			curSelection;
	int16_t			newSelection;
	CategoryID		category;
	CategoryID*		categories;
	uint32_t		nCategories = 0;
	uint32_t		recordID;
	uint16_t		recordIndex;
	ListPtr			lst;
	char*			name;
	RectangleType	r;
	status_t		err;

	lst = GetObjectPtr (ListItemsCategoryList, ListView);

	// Get the category of the item in the specified row.
	recordIndex = TblGetRowID(table, row);
	recordID = TblGetRowData(table, row);
	DbCursorSetAbsolutePosition(gListViewCursorID, recordIndex);
	err = DbGetCategory(ToDoDB, gListViewCursorID, &nCategories, &categories);
	if (err)
	{
		DbgOnlyFatalError("Failed to get categories from row");
		return;
	}
	category = nCategories ? *categories : catIDUnfiled ;

	// Unhighlight the priority.
	TblUnhighlightSelection (table);

	LstSetPosition (lst, 0, 0);

	// Create a list of categories.
	CatMgrCreateList(ToDoDB, lst, category, false /* show all */, false /* show multiple */,
		true /* show uneditables */, true /* resize list */, false, NULL);

	// Position the list.
	TblGetItemBounds (table, row, categoryColumn, &r);
	LstSetPosition (lst, r.topLeft.x, r.topLeft.y);

	// Display the category list.
	curSelection = LstGetSelection (lst);
	newSelection = LstPopupList (lst);

	// Was a new category selected?
	if ((newSelection != curSelection) && (newSelection != -1))
	{
		name = LstGetSelectionText (lst, newSelection);

		if (CatMgrFind(ToDoDB, name, &category) < errNone)
			category = catIDUnfiled ;

		CatMgrFreeList(ToDoDB, lst, false, false, false);
		if (categories)
			DbReleaseStorage(ToDoDB, categories);

		// Update the database record's category.
		err = DbCursorMoveToRowID(gListViewCursorID, recordID);
		err = DbSetCategory(ToDoDB, gListViewCursorID, category ? 1 : 0, category ? &category : NULL);

		// Changing the category may change the record's index
		ToDoDbCursorRequery(&gListViewCursorID);
		DbCursorMoveToRowID(gListViewCursorID, recordID);
		DbCursorGetCurrentPosition(gListViewCursorID, &CurrentRecord);
		CurrentRecordCategory = category ;

		// Make sure the row is redrawn.
		TblMarkRowInvalid (table, row);

		// Send an event that will cause the view to be redrawn.
		ListViewUpdateDisplay(updateItemMove);
	}
	else
	{
		CatMgrFreeList(ToDoDB, lst, false, false, false);
		if (categories)
			DbReleaseStorage(ToDoDB, categories);
		ListViewRestoreEditState ();
	}

}


/***********************************************************************
 *
 * FUNCTION:    ListViewItemSelected
 *
 * DESCRIPTION: This routine is called when an item in the do to list
 *              is selected.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95	Initial Revision
 *			peter	4/24/00	Add support for un-masking just the selected record.
 *
 ***********************************************************************/
static void ListViewItemSelected (EventPtr event)
{
	uint16_t 		on;
	int16_t 		row;
	int16_t 		column;
	uint16_t 		tableID;
	TablePtr		table;
	FieldPtr 		fldP;
	FormPtr			frmP;
	WinHandle		winH;
	EventType 		newEvent;
	uint16_t 		systemVolume;
	uint16_t 		mutedVolume = 0;
	uint32_t 		noteTitleLen = toDoDescriptionMaxSize;
	uint32_t		currentUID;
	Boolean			viewRecord;
	status_t		error;
	uint16_t		mode;
	Boolean			maskedRecord = false;

	PIMAppProfilingBegin("ListViewItemSelected");

	frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
	table = event->data.tblSelect.pTable;
	row = event->data.tblSelect.row;
	column = event->data.tblSelect.column;
	tableID = event->data.tblSelect.tableID;

	if (TblRowMasked(table,row))
	{
		maskedRecord = true;

		// the DB must be closed so that private records can be removed if the user
		// lost his password.
		DbCursorClose(gListViewCursorID);
		gListViewCursorID = dbInvalidCursorID;
		DbCloseDatabase (ToDoDB);
		ToDoDB= NULL; /* make sure the drawing code notices */

		gCheckingPassword = true;
		viewRecord = SecVerifyPW (showPrivateRecords);
		gCheckingPassword = false;

		if (LoadedAsSharedLib)
			mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
							dmModeReadOnly : (dmModeReadOnly | dmModeShowSecret);
		else
			mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
							dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
		if (!ToDoDB)
			ToDoGetDatabase(&ToDoDB, mode);

		ErrFatalDisplayIf(!ToDoDB, "Can't reopen DB");

		error = ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
		ErrFatalDisplayIf(error < errNone, "Can't reopen DB cursor");

		if (viewRecord)
		{
			// We only want to unmask this one record, so restore the preference.
			PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);

			// Unmask just the current row.
			TblSetRowMasked (table, row, false);

			// Draw the row unmasked.
			TblMarkRowInvalid (table, row);
			WinInvalidateWindow(gListViewWinH);
			gTblDrawCode = tblRedraw;

			// Only change the visual status of this record, leaving all others masked.
			CurrentRecordVisualStatus = showPrivateRecords;
				// Leave PrivateRecordVisualStatus set to maskPrivateRecords

			// Now that the row is unmasked, let the table re-handle the table
			// enter event. This will cause the field to be made fully visible
			// and place the cursor at the start of the field. It is necessary
			// to put the cursor in the field so that tapping outside the field
			// can be used to re-mask the record.

			newEvent.eType = tblEnterEvent;
			newEvent.penDown = event->penDown;
			newEvent.tapCount = 0;						// don't select anything
			newEvent.screenX = 0;						// put cursor at start
			newEvent.screenY = 0;
			newEvent.data.tblEnter.tableID = tableID;
			newEvent.data.tblEnter.pTable = table;
			newEvent.data.tblEnter.row = row;
			newEvent.data.tblEnter.column = descColumn;
			// Never let this event check off the item or view the note.

			// Rather than posting the event, handle it directly to avoid
			// the click produced by it, since a click was already produced.

			SndGetDefaultVolume (NULL, &systemVolume, NULL);
			SndSetDefaultVolume (NULL, &mutedVolume, NULL);
			TblHandleEvent (table, &newEvent);
			SndSetDefaultVolume (NULL, &systemVolume, NULL);

			// The row was masked, set cursor to description field
			column = descColumn ;
		}
		else
		{
			CurrentRecord = noRecordSelected;
			ListViewUpdateDisplay(updateDisplayOptsChanged);
			return;
		}
	}

	if (column == completedColumn)
	{
		on = TblGetItemInt (table, row, column);
		ListViewChangeCompleteStatus (row, on);
	}

	else if (column == priorityColumn)
	{
		ListViewSelectPriority (table, row);
	}

	else if (column == descColumn)
	{
		CurrentRecord = TblGetRowID (table, row);
		currentUID = TblGetRowData(table, row);
		// If the table is in edit mode then the description field
		// was selected, otherwise the note indicator must have
		// been selected.
		if (maskedRecord)
		{
			ItemSelected = true;
			ListEditSelectionLength = ListEditPosition = 0;
		}
		else if (TblEditing (table))
		{
			ItemSelected = true;
			fldP = TblGetCurrentField(table);
			FldGetSelection(fldP, &ListEditPosition, &ListEditSelectionLength);
			ListEditSelectionLength -= ListEditPosition;
			// restore edit state at next winUpdate
			gTblDrawCode = tblRedraw;
			ListViewNavHighlightItem(frmP, table, fldP);
		}

		else
		{
			error = DbCopyColumnValue(ToDoDB, currentUID, todoDescriptionColId, 0, (void*)NoteTitle, &noteTitleLen);
			TblReleaseFocus(table);
			ToDoEditNote((error == errNone && noteTitleLen > 0) ? NoteTitle : NULL, ToDoDB, currentUID, todoNoteColId, &ToDoDeleteNote);
			TblGrabFocus(table, row, column);
			ListViewUpdateDisplay(updateDisplayOptsChanged);
		}
	}

	else if (column == dueDateColumn)
	{
		ListViewSelectDueDate (table, row);
	}

	else if (column == categoryColumn)
	{
		ListViewSelectItemsCategory (table, row);
	}

	if (frmP != NULL
	&& (winH=FrmGetWindowHandle(frmP)) != invalidWindowHandle)
		WinInvalidateWindow(winH);

	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:    ListViewResizeDescription
 *
 * DESCRIPTION: This routine is called when the height of a ToDo item's
 *              description is changed as a result of user input.
 *              If the new height of the field is shorter,  more items
 *              may need to be added to the bottom of the list.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/18/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewResizeDescription (EventPtr event)
{
	uint16_t lastRow;
	uint32_t lastRecord;
	uint32_t topRecord;
	FieldPtr fld;
	TablePtr table;
	Boolean lastItemClipped;
	RectangleType itemR;
	RectangleType tableR;
	RectangleType fieldR;

	// Get the current height of the field;
	fld = event->data.fldHeightChanged.pField;
	FldGetBounds (fld, &fieldR);

	// Have the table object resize the field and move the items below
	// the field winUp or winDown.
	table = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
	TblHandleEvent (table, event);


	// If the field's height has expanded , and there are no items scrolled
	// off the top of the table, just update the scrollers.
	// If sorting on multiple categories, update the whole table, as other
	// items may have changed
	if (event->data.fldHeightChanged.newHeight >= fieldR.extent.y)
	{
		topRecord = TblGetRowID (table, 0);
		if (topRecord != TopVisibleRecord)
			TopVisibleRecord = topRecord;
		else
		{
			// Update the scroll arrows.
			lastRow = TblGetLastUsableRow (table);
			TblGetBounds (table, &tableR);
			TblGetItemBounds (table, lastRow, descColumn, &itemR);
			lastItemClipped = (itemR.topLeft.y + itemR.extent.y >
			 	tableR.topLeft.y + tableR.extent.y);
			lastRecord = TblGetRowID (table, lastRow);
			ListViewUpdateScrollers (FrmGetFormPtr(ToDoSharedData.listViewId), lastRecord, lastItemClipped);
		}
	}

	// Add items to the table to fill in the space made available by the
	// shortening the field.
	ListViewUpdateDisplay(frmRedrawUpdateCode);
	if (ItemSelected)
		ListViewRestoreEditState();
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories in the List View.
 *
 * PARAMETERS:  reOpenCursor: do we need to reopen the cursor
 *
 * RETURNED:    The index of the new category.
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							ShowAllCategories
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95 Initial Revision
 *			rbb	04/14/99 Uses new ListViewDrawTable
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *			mchen	12/20/00 Resort after CategorySelect() because categories
 *								may have changed
 *
 ***********************************************************************/
Boolean ListViewSelectCategory (void)
{
	FormPtr 		frmP;
	CategoryID *	selectedCategoryIDsP = NULL;
	uint32_t		selectedNumCategories = 0;
	CategoryID 		category;
	Boolean 		enableEditing, categoryEdited;
	uint8_t 		updateCode = updateCategoryChanged;

	// Process the category popup list.
	category = CurrentCategory;
	if (sListViewCategoryIDsP)
		sListViewCategoryIDsP[0] = CurrentCategory;
	// Do not enable editing if loaded as shared lib from the DateBook
	enableEditing = LoadedAsSharedLib ? false : true;

	frmP = FrmGetFormPtr(ToDoSharedData.listViewId);

	// Close our cursor in case current category was deleted
	ToDoCloseCursor(&gListViewCursorID);
	categoryEdited = CatMgrSelectFilter(ToDoDB, frmP, ToDoSharedData.ListCategoryTriggerId, CategoryName, ToDoSharedData.ListCategoryListId,
			sListViewNumCategories ? sListViewCategoryIDsP : &category, sListViewNumCategories ? sListViewNumCategories : 1,
			&selectedCategoryIDsP, &selectedNumCategories,
			enableEditing, NULL);

	if (categoryEdited)
	{
		sListViewCategoryIDsP = selectedCategoryIDsP;
		sListViewNumCategories = selectedNumCategories;
		selectedCategoryIDsP = NULL;
		category = sListViewCategoryIDsP ? sListViewCategoryIDsP[0] : catIDUnfiled ;
	}

	// If the option for category column is set and we switched to/from "All",
	// the table will need to be rebuilt with/without the column
	if ( (categoryEdited) || (CurrentCategory != category) || ShowCategories)
	{
		FrmClearFocusHighlight();
		ChangeCategory (category);
		updateCode = updateDisplayOptsChanged;
	}

	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	// Do not redraw if the ToDo was loaded as a shared library: the table
	// must be resized first.
	if (! LoadedAsSharedLib && updateCode == updateDisplayOptsChanged)
	{
		// Display the new category.
		ListViewUpdateDisplay (updateCode);
	}
	else
		ListViewUpdateDisplay (updateRedrawAll);

	return categoryEdited;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewNextCategory
 *
 * DESCRIPTION: This routine displays the next category, if the last
 *              catagory is being displayed we wrap to the first category.
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
 *			art		9/15/95		Initial Revision
 *			rbb		4/14/99		Uses new ListViewDrawTable
 *
 ***********************************************************************/
static void ListViewNextCategory (void)
{
	CategoryID	category;
	ControlPtr	ctl;

	CatMgrGetNext(ToDoDB, CurrentCategory, &category);

	if (category == CurrentCategory) return;

	FrmClearFocusHighlight();

	ChangeCategory (category);

	// Set the label of the category trigger.
	ctl = GetObjectPtr (ToDoSharedData.ListCategoryTriggerId, ToDoSharedData.listViewId);

	CatMgrSetTriggerLabel(ToDoDB, &CurrentCategory, 1, ctl, CategoryName);

	// Display the new category.
	ListViewDrawTable (updateDisplayOptsChanged);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGotoAppointment
 *
 * DESCRIPTION: This routine sets winUp the global variables such that the
 *              list view will display the text found by the text search
 *              command.
 *
 * PARAMETERS:  event - frmGotoEvent
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/11/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewGotoItem (EventPtr event)
{
	ItemSelected = true;
	CurrentRecord = event->data.frmGoto.recordNum;
	TopVisibleRecord = CurrentRecord;
	if (gMatchColumn != todoNoteColId)
	{
		ListEditPosition = event->data.frmGoto.matchPos;
		ListEditSelectionLength = event->data.frmGoto.matchLen;
	}
	else
	{
		ListEditPosition = ListEditSelectionLength = 0 ;
	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewScroll
 *
 * DESCRIPTION: This routine scrolls the list of ToDo items
 *              in the direction specified.
 *
 * PARAMETERS:  direction - winUp or dowm
 *              oneLine   - if true the list is scrolled by a single line,
 *                          if false the list is scrolled by a full screen.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	02/21/95	Initial Revision
 *			rbb	04/14/99	Uses new ListViewDrawTable
 *			gap	10/25/99	Optimized scrolling to only redraw if item position changed.
 *
 ***********************************************************************/
static void ListViewScroll (WinDirectionType direction)
{
	uint16_t		row;
	uint32_t		index;
	uint16_t		height;
	uint32_t 		recordNum;
	uint16_t 		columnWidth;
	uint16_t 		tableHeight;
	TablePtr 		tableP;
	RectangleType	r;
	uint16_t		prevTopVisibleRecord = (uint16_t) TopVisibleRecord;

	tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
	TblUnhighlightSelection(tableP);
	TblReleaseFocus (tableP);
	FrmClearFocusHighlight();

	CurrentRecord = noRecordSelected;
	CurrentRecordCategory = catIDAll;

	// Get the height of the tableP and the width of the description
	// column.
	TblGetBounds (tableP, &r);
	tableHeight = r.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth (tableP, descColumn);

	// Scroll the tableP down.
	if (direction == winDown)
	{
		// Get the record index of the last visible record.  A row
		// number of minus one indicates that there are no visible rows.
		row = TblGetLastUsableRow (tableP);
		if (row == tblUnusableRow) return;

		recordNum = TblGetRowID (tableP, row);

		// If there is only one record visible, this is the case
		// when a record occupies the whole screeen, move to the
		// next record.
		if (row == 0)
			SeekRecord (&recordNum, 1, dmSeekForward);
	}

	// Scroll the tableP up.
	else
	{
		// Scan the records before the first visible record to determine
		// how many record we need to scroll.  Since the heights of the
		// records vary,  we sum the heights of the records until we get
		// a screen full.
		recordNum = TopVisibleRecord;
		height = TblGetRowHeight (tableP, 0);
		if (height >= tableHeight)
			height = 0;

		while (height < tableHeight)
		{
			index = recordNum;
			if ( ! SeekRecord (&index, 1, dmSeekBackward) ) break;
			height += ListViewGetDescriptionHeight (index, columnWidth, tableHeight);
			if ((height <= tableHeight) || (recordNum == TblGetRowID (tableP, 0)))
				recordNum = index;
		}
	}

	TopVisibleRecord = recordNum;
	ListViewLoadTable (true);

	// Need to compare the previous top record to the current after ListViewLoadTable
	// as it will adjust TopVisibleRecord if drawing from recordNum will not fill the
	// whole screen with items.
	if (TopVisibleRecord != prevTopVisibleRecord)
	{
		WinInvalidateWindow(gListViewWinH);
		gTblDrawCode = tblRedraw;
	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDeleteCompleted
 *
 * DESCRIPTION: This routine deletes ToDo items that are marked
 *              complete.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/29/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewDeleteCompleted (void)
{
	uint16_t i;
	uint16_t ctlIndex;
	int16_t rowsInTable;
	uint16_t buttonHit;
	FormPtr alert;
	TablePtr table;
	Boolean saveBackup;
	uint32_t purgeCursorID ;

	table = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
	TblReleaseFocus (table);

	// Display an alert to comfirm the operation.
	alert = FrmInitForm(gApplicationDbP, DeleteCompletedDialog);

	ctlIndex = FrmGetObjectIndex (alert, DeleteCompletedSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);
	buttonHit = FrmDoDialog (alert);
	saveBackup = (Boolean)FrmGetControlValue (alert, ctlIndex);

	FrmDeleteForm (alert);

	if (buttonHit != DeleteCompletedOk)
		return;

	SaveBackup = saveBackup;

	// Make sure no field in the list has focus so edit-in-place won't break
	FrmSetFocus(FrmGetFormPtr(ToDoSharedData.listViewId), noFocus);

	// Close the list view cursor
	ToDoCloseCursor(&gListViewCursorID);
	// Reopen DB if private records were hidden
	if (PrivateRecordVisualStatus == hidePrivateRecords)
	{
		DbCloseDatabase (ToDoDB);
		ToDoGetDatabase(&ToDoDB, (dmModeReadWrite | dmModeShowSecret));
	}

	// Open a temporary cursor for purge, with only completed columns
	DbCursorOpen (ToDoDB, "ToDoDBSchema WHERE Completed = 1", 0, &purgeCursorID);

	// Delete records marked complete.
	for (DbCursorMoveFirst(purgeCursorID); !DbCursorIsEOF(purgeCursorID); DbCursorMoveNext(purgeCursorID))
	{
		if (SaveBackup)
			DbArchiveRow (ToDoDB, purgeCursorID);
		else
			DbDeleteRow (ToDoDB, purgeCursorID);
	}

	// Close the purge cursor
	DbCursorClose(purgeCursorID);
	// Reopen DB if private records are to be hidden
	if (PrivateRecordVisualStatus == hidePrivateRecords)
	{
		DbCloseDatabase (ToDoDB);
		ToDoGetDatabase(&ToDoDB, dmModeReadWrite);
	}

	// Reopen the list view cursor
	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	// Redraw the ToDo list.
	rowsInTable = TblGetNumberOfRows (table);
	for (i = 0; i < rowsInTable; i++)
		TblSetRowUsable (table, i, false);

	ListViewUpdateDisplay(updateCategoryChanged);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDoCommand
 *
 * DESCRIPTION: This routine preforms the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		03/29/95	art	Created by Art Lamb.
 *		09/17/99	jmp	Use NewNoteView instead of NoteView.
 *		10/4/99	jmp	Replaced call to DmOpenDatabaseByTypeCreator() with
 *							ToDoGetDatabase().
 *		11/04/99	jmp	To prevent other sublaunch issues, remind ourselves
 *							that we've sublaunched already into PhoneNumberLookup().
 *		11/16/99	jmp	Release the table focus around the send item and the send
 *							category commands.  This fixes bug #24067 and makes this
 *							code consistent with what the Datebook does.
 *		08/28/00	kwk	Use new FrmGetActiveField call.
 *
 ***********************************************************************/
static Boolean ListViewDoCommand (uint16_t command)
{
	size_t 		pasteLen;
	MemHandle 	pasteCharsH;
	FieldPtr 	fld;
	Boolean 	handled = true;
	EventType	newEvent;
	uint32_t	currentUID;
	status_t	error;

	uint32_t noteTitleLen = toDoDescriptionMaxSize;

	switch (command)
		{
		case DeleteCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				ListViewDeleteToDo ();
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case CreateNoteCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
			{
				DbCursorGetRowIDForPosition(gListViewCursorID, CurrentRecord, &currentUID);
				error = DbCopyColumnValue(ToDoDB, currentUID, todoDescriptionColId, 0, (void*)NoteTitle, &noteTitleLen);
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
				ToDoEditNote((error == errNone && noteTitleLen > 0) ? NoteTitle : NULL, ToDoDB, currentUID, todoNoteColId, &ToDoDeleteNote);
				ListViewRestoreEditState ();
				ListViewUpdateDisplay(updateDisplayOptsChanged);
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case DeleteNoteCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				ListViewDeleteNote ();
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case BeamRecordCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				{
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
				ToDoSendRecord (ToDoDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
				ListViewRestoreEditState ();
				}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case SendRecordCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				{
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
				ToDoSendRecord (ToDoDB, CurrentRecord, exgSendPrefix, NoDataToSendAlert);
				ListViewRestoreEditState ();
				}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case AttachRecordCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				{
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
				ToDoSendRecord (ToDoDB, CurrentRecord, gTransportPrefix, NoDataToSendAlert);
				ListViewRestoreEditState ();

				// Send quit event to quit application
				MemSet(&newEvent, sizeof(newEvent), 0);
				newEvent.eType = appStopEvent;
				EvtAddEventToQueue(&newEvent);
				}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case BeamCategoryCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));

			ToDoSendCategory(ToDoDB, CurrentCategory, exgBeamPrefix, NoDataToBeamAlert);

			if (ItemSelected)
				ListViewRestoreEditState ();
			break;

		case SendCategoryCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));

			ToDoSendCategory(ToDoDB, CurrentCategory, exgSendPrefix, NoDataToSendAlert);

			if (ItemSelected)
				ListViewRestoreEditState ();
			break;

		case AttachCategoryCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));

			ToDoSendCategory(ToDoDB, CurrentCategory, gTransportPrefix, NoDataToSendAlert);

			if (ItemSelected)
				ListViewRestoreEditState ();

			// Send quit event to quit application
			MemSet(&newEvent, sizeof(newEvent), 0);
			newEvent.eType = appStopEvent;
			EvtAddEventToQueue(&newEvent);
			break;

		case FontCmd:
			MenuEraseStatus (0);
			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
			SelectFont (&ListFont);
			if (ItemSelected)
				ListViewRestoreEditState ();
			break;

		case PhoneLookupCmd:
			MenuEraseStatus (0);

			if (ItemSelected)
				{
				ListViewRestoreEditState () ;
				fld = TblGetCurrentField (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
				if (fld)
					PhoneNumberLookup (fld);
				}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		case SecurityCmd:
			MenuEraseStatus (0);
			ListViewClearEditState(true);
			gCheckingPassword = true;
			PrivateRecordVisualStatus = CurrentRecordVisualStatus = SecSelectViewStatus();
			gCheckingPassword = false;

			// the DB must be closed so that private records can be removed if the user
			// lost his password.
			DbCursorClose(gListViewCursorID);
			gListViewCursorID = dbInvalidCursorID;
			DbCloseDatabase (ToDoDB);
			ToDoDB = NULL;

			ToDoSavePrefs();
			ToDoLoadPrefs();

			if (!ToDoDB)
				ToDoGetDatabase(&ToDoDB, (PrivateRecordVisualStatus == hidePrivateRecords) ?
								dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));

			ErrFatalDisplayIf(!ToDoDB, "Can't reopen DB");

			error = ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
			ErrFatalDisplayIf(error < errNone, "Can't reopen DB cursor");

			ListViewUpdateDisplay (updateDisplayOptsChanged);
			break;

		case AboutCmd:
			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId));
			AbtShowAbout (sysFileCToDo);
			if (ItemSelected)
				ListViewRestoreEditState ();
			break;

		case DeleteCompletedCmd:
			MenuEraseStatus (0);
			ListViewClearEditState (true);
			ListViewDeleteCompleted ();
			break;

		case sysEditMenuPasteCmd:
			fld = FrmGetActiveField (NULL);
			if (! fld)
				{
				pasteCharsH = ClipboardGetItem (clipboardText, &pasteLen);
				if (pasteCharsH && pasteLen)
					ListViewNewToDo();
				}
			handled = false;
			break;

		default:
			handled = false;
		}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGetColumnWidth
 *
 * DESCRIPTION: This routine returns the width of the specified
 *              column.
 *
 * PARAMETERS:	 column - column of the list table
 *
 * RETURNED:	 width of the column in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/10/97	Initial Revision
 *
 ***********************************************************************/
uint16_t ListViewGetColumnWidth (int16_t column)
{
	char		chr;
	char		dateBuffer [dateStringLength];
	uint16_t	width=0;
	FontID	curFont=0;
	char*		dateStr=NULL;


	if (column == priorityColumn)
		{
		curFont = FntSetFont (ListViewPriorityFontID());
		}
	else
		{
		curFont = FntSetFont (ListFont);
		}

	if (column == priorityColumn)
		{
		chr = '1';
		width = (FntCharWidth (chr) - 1) + 6;
		}

	else if (column == dueDateColumn)
		{
		DateToAscii (12, 31, 1997,	DateFormat, dateBuffer);

		// Remove the year from the date string.
		dateStr = dateBuffer;
		if ((DateFormat == dfYMDWithSlashes) ||
			 (DateFormat == dfYMDWithDots) ||
			 (DateFormat == dfYMDWithDashes))
			dateStr += 3;
		else
			dateStr[strlen(dateStr) - 3] = 0;

		width = FntCharsWidth (dateStr, strlen (dateStr));

		// Get the width of the character that indicates the item is due.
		// Don't count the whitespace in the character. Handle auto-bolding
		// of list font for the priority number.
		FntSetFont (ListViewPriorityFontID());
		chr = '!';
		width += FntCharWidth (chr) - 1;
		}

	// Size the category column such that is can display about five
	// characters.
	else if (column == categoryColumn)
		{
		chr = '1';
		width = (FntCharWidth (chr) * 5) - 1;
		}

	FntSetFont (curFont);

	return (width);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewResizeTable
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORIC:
 *	PPL 05/04/04	Finalize Landcsape Mode.
 *
 ***********************************************************************/
void ListViewResizeTable(FormType* formP, Coord buttonY, Coord offsetX, Coord offsetY, Boolean scroll)
{
	uint16_t		objIndex;
	RectangleType	objectBounds;
	RectangleType	itemBounds;
	FontID			currFont;
	TableType * 	tableP;
	int16_t 		lineHeight;
	int16_t 		row;
	int16_t 		col;
	uint16_t		numLines;
	int16_t			topRow;

	// The List table
	objIndex = FrmGetObjectIndex(formP, ToDoSharedData.listTableId);
	tableP = FrmGetObjectPtr(formP, objIndex);

	// Get font height
	currFont = FntSetFont (NoteFont);
	lineHeight = FntLineHeight();
	FntSetFont (currFont);

	TblGetSelection(tableP, &row, &col);
	TblGetItemBounds(tableP, row, col, &itemBounds);

	FrmGetObjectBounds(formP, objIndex, &objectBounds);

	// Stick the table bottom to the 'New' button.
	objectBounds.extent.y = buttonY - objectBounds.topLeft.y;

	// Remove the last incomplete line
	objectBounds.extent.y -= (objectBounds.extent.y % lineHeight);

	// Resize table on X
	objectBounds.extent.x += offsetX;

	// Resize the table
	FrmSetObjectBounds(formP, objIndex, &objectBounds);

	if (scroll && itemBounds.topLeft.y + itemBounds.extent.y >= objectBounds.topLeft.y + objectBounds.extent.y)
	{
		numLines = (itemBounds.topLeft.y + itemBounds.extent.y - objectBounds.topLeft.y - objectBounds.extent.y) / lineHeight;
		topRow = TblGetTopRow(tableP);
		topRow += numLines;
		TopVisibleRecord = TblGetRowID(tableP, topRow);
	}

	// Resize description column
	TblSetColumnWidth(tableP, descColumn, TblGetColumnWidth(tableP, descColumn) + offsetX);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewResizeForm
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORIC:
 *	PPL 05/04/04	Finalize Landscape Mode.
 *
 ***********************************************************************/
void ListViewResizeForm(EventType *eventP, Boolean scroll)
{
	uint16_t		objIndex;
	Coord			offsetX;
	Coord			offsetY;
	FormType		*frmP;
	Coord			winHeight;
	Coord			winWidth;
	Coord			buttonY;
	Coord			x;
	Coord			y;


	frmP = FrmGetFormPtr(ToDoSharedData.listViewId);

	// The Window
	winWidth = eventP->data.winResized.newBounds.extent.x;
	winHeight = eventP->data.winResized.newBounds.extent.y;

	offsetY = winHeight - gListViewWinBounds.extent.y;
	offsetX = winWidth  - gListViewWinBounds.extent.x;
	
	if (!offsetX && !offsetY)
		return ;

	FrmClearFocusHighlight();

	// ListNewToDoButton, Moves on y
	// need to set up buttonY
	objIndex = FrmGetObjectIndex(frmP, ListNewToDoButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y += offsetY;
	buttonY = y;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	if (offsetY)
	{
		// ListDetailsButton, moves only on y 
		objIndex = FrmGetObjectIndex(frmP, ListDetailsButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// ListShowButton, moves only on y 
		objIndex = FrmGetObjectIndex(frmP, ListShowButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// ListAttachButton, moves only on y
		objIndex = FrmGetObjectIndex(frmP, ListAttachButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	if (offsetX)
	{
		// ListCategoryTrigger, moves only on x
		objIndex = FrmGetObjectIndex(frmP, ListCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// ListCategoryList, moves only on x
		objIndex = FrmGetObjectIndex(frmP, ListCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	ListViewResizeTable(frmP, buttonY, offsetX, offsetY, scroll);
	
	// The ListUpButton  moves on both x and y
	objIndex = FrmGetObjectIndex(frmP, ListUpButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);
	
	// The ListUpButton  moves on both x and y
	objIndex = FrmGetObjectIndex(frmP, ListDownButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// The Graffiti Shift Indicator, moves on both x and y
	objIndex = FrmGetNumberOfObjects(frmP);
	while (objIndex--)
	{
		if (FrmGetObjectType(frmP, objIndex) == frmGraffitiStateObj)
		{
			FrmGetObjectPosition (frmP, objIndex, &x, &y);
			x += offsetX;
			y += offsetY;
			FrmSetObjectPosition (frmP, objIndex, x, y);
			break;
		}
	}

	
	// Save new list view window bounds
	gListViewWinBounds = eventP->data.winResized.newBounds;
	if (!InNoteView)
		ListViewLoadTable(true);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewInit
 *
 * DESCRIPTION: This routine initializes the "List View" of the
 *              ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			rbb	4/14/99	Only show category column when viewing "All"
 *			jmp	11/27/99	Fix long-standing bug where priority column's spacing
 *								would change depending on whether we were being initialized
 *								for the first time or we were being re-inited (as during
 *								an update event or such).
 *			peter	5/26/00	Mask only the description, not the other columns.
 *			peter	6/12/00	Mask all the columns: Marketing requirement for 3.5
 *
 ***********************************************************************/
void ListViewInit (FormPtr frm)
{
	int16_t row;
	int16_t rowsInTable;
	uint16_t width;
	FontID fontID;
	TablePtr table;
	RectangleType r;
	Boolean showCategories = ShowCategories && (CurrentCategory == catIDAll);

	// Only set the gadget handler if we're a stand alone app.
	if( !LoadedAsSharedLib )
		FrmSetGadgetHandler(
			frm, FrmGetObjectIndex(frm, ListBackground),
			PrvBackgroundGadgetHandler);

	FrmSetTransparentObjects(frm, true);

	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ToDoSharedData.listTableId));

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle (table, row, completedColumn, checkboxTableItem);
		TblSetItemStyle (table, row, priorityColumn, numericTableItem);
		// When the ToDo is loaded as a shared lib, we use a TableDrawItemFuncType
		// callback instead of a TableLoadDataFuncType callback.
		if (!LoadedAsSharedLib)
			TblSetItemStyle (table, row, descColumn, textTableItem);
		else
			TblSetItemStyle (table, row, descColumn, customTableItem);

		TblSetItemStyle (table, row, dueDateColumn, customTableItem);
		TblSetItemStyle (table, row, categoryColumn, customTableItem);

		// Set the font used to draw the text of the row. We automatically
		// bold priority number.
		fontID = ListViewPriorityFontID();

		TblSetItemFont (table, row, priorityColumn, fontID);
		TblSetItemFont (table, row, descColumn, ListFont);
		TblSetItemFont (table, row, dueDateColumn, ListFont);
		TblSetItemFont (table, row, categoryColumn, ListFont);

		TblSetRowUsable (table, row, false);
	}

	TblSetColumnUsable (table, completedColumn, true);
	TblSetColumnUsable (table, priorityColumn, ShowPriorities);
	TblSetColumnUsable (table, descColumn, true);
	TblSetColumnUsable (table, dueDateColumn, ShowDueDates);
	TblSetColumnUsable (table, categoryColumn, showCategories);

	// Set up to mask all columns
	TblSetColumnMasked (table, completedColumn, true);
	TblSetColumnMasked (table, priorityColumn, true);
	TblSetColumnMasked (table, descColumn, true);
	TblSetColumnMasked (table, dueDateColumn, true);
	TblSetColumnMasked (table, categoryColumn, true);

	// Set the spacing after the complete column.
	if (ShowPriorities)
	{
		TblSetColumnSpacing (table, completedColumn, 0);
		TblSetColumnSpacing (table, priorityColumn, spaceBeforeDesc);
	}
	else
	{
		TblSetColumnSpacing (table, completedColumn, spaceBeforeDesc);
		TblSetColumnSpacing (table, priorityColumn, spaceNoPriority);
	}

	if (ShowDueDates && showCategories)
	{
		TblSetColumnSpacing (table, dueDateColumn, spaceBeforeCategory);
	}


	// Set the width of the priorities column.
	if (ShowPriorities)
	{
		width = ListViewGetColumnWidth (priorityColumn);
		TblSetColumnWidth (table, priorityColumn, width);
	}

	// Set the width of the due date column.
	if (ShowDueDates)
	{
		width = ListViewGetColumnWidth (dueDateColumn);
		TblSetColumnWidth (table, dueDateColumn, width);
	}

	// Set the width of the category column.
	if (showCategories)
	{
		width = ListViewGetColumnWidth (categoryColumn);
		TblSetColumnWidth (table, categoryColumn, width);
	}

	// Set the width of the description column.
	TblGetBounds (table, &r);
	width = r.extent.x;
	width -= TblGetColumnWidth (table, completedColumn) +
				TblGetColumnSpacing (table, completedColumn);
	width -= TblGetColumnSpacing (table, descColumn);
	if (ShowPriorities)
		width -= TblGetColumnWidth (table, priorityColumn) +
				   TblGetColumnSpacing (table, priorityColumn);
	if (ShowDueDates)
		width -= TblGetColumnWidth (table, dueDateColumn) +
				   TblGetColumnSpacing (table, dueDateColumn);
	if (showCategories)
		width -= TblGetColumnWidth (table, categoryColumn) +
				   TblGetColumnSpacing (table, categoryColumn);


	TblSetColumnWidth (table, descColumn, width);

	// Set the callback routines that will load the description field.
	// In the datebook, we don't need the save callback
	if (!LoadedAsSharedLib)
	{
		TblSetLoadDataProcedure (table, descColumn, (TableLoadDataFuncPtr) ListViewGetDescription);
		TblSetSaveDataProcedure (table, descColumn, (TableSaveDataFuncPtr) ListViewSaveDescription);
	}
	else
	{
		TblSetCustomDrawProcedure (table, descColumn, (TableDrawItemFuncPtr) ListViewDrawDescForDatebook);
	}

	// Set the callback routine that draws the due date field.
	TblSetCustomDrawProcedure (table, dueDateColumn, (TableDrawItemFuncPtr) ListViewDrawDueDate);

	// Set the callback routine that draws the category field.
	TblSetCustomDrawProcedure (table, categoryColumn, (TableDrawItemFuncPtr) ListViewDrawCategory);

	// Create a cursor
	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	// Do not load records when launched as shared library
	if (!LoadedAsSharedLib)
		ListViewLoadTable (true);

	// Were we requested to add an attachment?
	// if yes, we show the attach button.
	// if not we hide the attach button
	if (!LoadedAsSharedLib)
	{
		if (gAttachRequest)
		{
			// Show Attach Button
			FrmShowObject (frm,FrmGetObjectIndex (frm, ListAttachButton));
		}
		else
		{
			// Hide attach button
			FrmHideObject (frm, FrmGetObjectIndex (frm, ListAttachButton));
		}
	}

	FrmSetFocus(frm, noFocus);
	gTableNavFocus = false;
}

/***********************************************************************
 *
 * FUNCTION:    ListViewSetSelection
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 ***********************************************************************/
static void ListViewSetSelection(void)
{
	FormPtr frmP;
	TablePtr tableP;
	FieldPtr fldP;
	uint32_t uniqueID;
	int16_t row;
	int16_t tableIndex;

	TraceOutput(TL(appErrorClass, "ListViewSetSelection itemSelected : %d, CurrentRecord : %d",ItemSelected,CurrentRecord));
	// Highlight selection if requested
	if (ListEditSelectionLength)
	{
		frmP = FrmGetFormPtr (ToDoSharedData.listViewId);
		tableIndex = FrmGetObjectIndex (frmP, ToDoSharedData.listTableId);
		tableP = FrmGetObjectPtr (frmP, tableIndex);
		DbCursorGetRowIDForPosition(gListViewCursorID, CurrentRecord, &uniqueID);
		if (TblFindRowData (tableP, uniqueID, &row))
		{
TraceOutput(TL(appErrorClass, "ListViewSetSelection row %d pos %d len %d",row,ListEditPosition,ListEditSelectionLength));
			FrmSetFocus (frmP, tableIndex);
			TblGrabFocus(tableP, row, descColumn);
			fldP = TblGetCurrentField(tableP);
			if (fldP && ListEditPosition + ListEditSelectionLength <= FldGetTextLength(fldP))
				FldSetSelection (fldP, ListEditPosition, ListEditPosition + ListEditSelectionLength);
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateDisplay
 *
 * DESCRIPTION: This routine updates the display of the List View
 *
 * PARAMETERS:  updateCode - a code that indicated what changes have been
 *                           made to the ToDo list.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/2/95	Initial Revision
 *			rbb	4/14/99	Uses new ListViewDrawTable and ListViewRedrawTable
 *			jmp	11/01/99	Fixed problem on frmRedrawUpdateCode events when
 *								we're still in the edit state but we weren't redrawing
 *								the edit indicator.  Fixes ToDo part of bug #23235.
 *			jmp	11/15/99	Make sure to clear the edit state on updateItemDelete and
 *								updateItemHide!
 *			jmp	11/27/99	Make sure we call ListViewInit() on frmRedrawUpdateCodes,
 *								otherwise column spacing may not be correct.
 *			jmp	12/09/99	Oops, calling ListViewInit() on frmRedrawUpdateCodes actually
 *								causes us mucho grief.  Also, don't call ListViewRedrawTable()
 *								when TblRedrawTable() will do.  Fixes bug #23914.
 *			peter	12/22/00	Reload the table if ClearEditState deletes item when handling
 *								updateItemMove updateCode, but only redraw the table once.
 *
 ***********************************************************************/
Boolean ListViewUpdateDisplay (uint16_t updateCode)
{
	int16_t 			row;
	int16_t 			column;
	uint16_t 			numRows;
	uint32_t 			recordNum;
	uint32_t 			uniqueID;
	TablePtr 			tableP;
	RectangleType 		itemBounds;
	RectangleType 		tableBounds;
	Boolean 			rowIsEntirelyVisible;
	Boolean				handled = false;
	uint32_t 			numCategories ;
	CategoryID * 		categoryIDsP = NULL;
	CategoryID			category;
	status_t			err = errNone;


	TraceOutput(TL(appErrorClass, "ListViewUpdateDisplay updateCode : 0x%x",updateCode));
	PIMAppProfilingBegin("ListViewUpdateDisplay");

	tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);

	// Just redraw the list view without loading it
	if (updateCode & frmRedrawUpdateCode)
	{
		TraceOutput(TL(appErrorClass, "ListViewUpdateDisplay - frmRedraw"));
		ListViewRedrawTable (false);
		handled = true;
		goto ExitPoint;
	}

	// Were the display options modified (ToDoOption dialog) or was the
	// font changed?
	if (updateCode & (updateDisplayOptsChanged | updateFontChanged))
	{
		TraceOutput(TL(appErrorClass, "ListViewUpdateDisplay - Opts / Font Changed"));
		ListViewDrawTable (updateCode);
		handled = true;
		goto ExitPoint;
	}

	// Was the category of an item changed?
	else if (updateCode & updateCategoryChanged)
	{
		TraceOutput(TL(appErrorClass, "ListViewUpdateDisplay - Category Changed"));
		if (ShowAllCategories)
			ChangeCategory(catIDAll);
		else
		{
			// Post a custom event to update category trigger
			EventType event;
			event.eType = updateCategoryTriggerEvent;

			if (CurrentRecord != noRecordSelected)
			{
				// Get record categories
				err = DbCursorSetAbsolutePosition(gListViewCursorID, CurrentRecord);
				if (err >= errNone)
					err = DbGetCategory(ToDoDB, gListViewCursorID, &numCategories, &categoryIDsP);
				if (err >= errNone)
				{				
					category = categoryIDsP ? categoryIDsP[0] : catIDUnfiled;
					ChangeCategory (category) ;
					if (categoryIDsP)
						DbReleaseStorage(ToDoDB, categoryIDsP);
					TopVisibleRecord = CurrentRecord;
				}
				else
					CurrentRecord = noRecordSelected;
			}
			EvtAddEventToQueue (&event);
		}
	}

	// Was an item deleted or marked secret? If so, invalidate all the rows
	// following the deleted/secret record.  Also, make sure the edit
	// state is now clear.
	if ( (updateCode & updateItemDelete) || (updateCode & updateItemHide))
	{
		TraceOutput(TL(appErrorClass, "ListViewUpdateDisplay - Item Delete / Hide"));
		TblGetSelection (tableP, &row, &column);
		numRows = TblGetNumberOfRows (tableP);
		for ( ; row < numRows; row++)
			TblSetRowUsable (tableP, row, false);

		ClearEditState ();
	}

	// Was the item moved?
	// Items are moved when their priority or due date is changed.
	else if (updateCode & updateItemMove)
	{
		TraceOutput(TL(appErrorClass, "ListViewUpdateDisplay - Item Move"));
		// Always redraw the current record
		DbCursorSetAbsolutePosition(gListViewCursorID, CurrentRecord);
		DbCursorGetCurrentRowID(gListViewCursorID, &uniqueID);
		if (TblFindRowData (tableP, uniqueID, &row))
			TblSetRowUsable (tableP, row, false);

		// We don't want to scroll the current record into view.
		recordNum = CurrentRecord;
		CurrentRecord = noRecordSelected;

		// Update all records, because current item may be duplicated
		if (sortingByCategories(CurrentSortId))
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

		ListViewLoadTable (true);

		// If the item is still entirely visible we will restore the edit state, but
		// If the item is only partially visible, we won't.
		CurrentRecord = recordNum;
		if (!TblFindRowData (tableP, uniqueID, &row))
			rowIsEntirelyVisible = false;		// Row isn't visible at all.
		else
		{
			// Row is at least partially visible, but is all of it visible?
			if (row < TblGetLastUsableRow (tableP))
				rowIsEntirelyVisible = true;	// Row isn't last so all must be visible.
			else
			{
				// Row is last, so it may not be entirely visible.
				TblGetBounds (tableP, &tableBounds);
				TblGetItemBounds (tableP, row, descColumn, &itemBounds);
				rowIsEntirelyVisible = itemBounds.topLeft.y + itemBounds.extent.y <=
					tableBounds.topLeft.y + tableBounds.extent.y;
			}
		}

		if (!rowIsEntirelyVisible)
		{
			// Entire row must be visible in order to safely maintain selection.
			// Since this is not the case, clear the edit state.
			// This will delete the current record if it's empty.
			if (ClearEditState ())
			{
				// The current record was empty, so it was deleted. This means the
				// record numbers stored in the tableP rows are no longer valid, so
				// reload and redraw the tableP.
				ListViewLoadTable (true);
			}
		}

		// Only redraw the tableP once.
		if (gListViewWinH == NULL)
		{
			WinInvalidateWindow(
				FrmGetWindowHandle(
					FrmGetFormPtr (ToDoSharedData.listViewId)));
		}
		else
			WinInvalidateWindow(gListViewWinH);

		gTblDrawCode = tblRedraw;
		handled = true;
		goto ExitPoint;
	}

	ListViewRedrawTable (true);

ExitPoint:
	PIMAppProfilingEnd();
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewHandleKeyDownEvent
 *
 * DESCRIPTION: This routine handles key down events
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *
 ***********************************************************************/
static Boolean ListViewHandleKeyDownEvent(EventPtr eventP)
{
	Boolean 	handled = false;
	Boolean 	isPrint = false;
	Boolean 	deleted = false;
	DateType 	date;
	FormPtr 	frmP;
	TablePtr	tableP;
	uint32_t	currentRecord, prevRecord;

	if (EvtKeydownIsVirtual(eventP))
	{
		// ToDo key pressed?
		if (TxtCharIsHardKey(eventP->data.keyDown.modifiers, eventP->data.keyDown.chr))
		{
			if ((eventP->data.keyDown.modifiers & poweredOnKeyMask))
			{
				// If the date has changed since the last time the device
				// was powered on then redraw the ToDo list so that pass
				// due item will be display correctly.
				DateSecondsToDate (TimGetSeconds (), &date);
				if (DateToInt (date) != DateToInt (Today))
				{
					Today = date;
					ListViewDrawTable (updateRedrawAll);
				}
			}
			// Display the next category
			else
			{
				ListViewClearEditState (true);
				ListViewNextCategory ();
			}
			handled = true;
		}

		// Scroll up key pressed?
		else if (eventP->data.keyDown.chr == vchrPageUp)
		{
			ListViewClearEditState (true);
			ListViewScroll (winUp);
			handled = true;
		}

		// Scroll down key pressed?
		else if (eventP->data.keyDown.chr == vchrPageDown)
		{
			ListViewClearEditState (true);
			ListViewScroll (winDown);
			handled = true;
		}

		else if (gTableNavFocus && ItemSelected && eventP->data.keyDown.chr == vchrRockerUp)
		{
			int16_t row;
			tableP = GetObjectPtr(ToDoSharedData.listTableId, ToDoSharedData.listViewId);
			prevRecord = currentRecord = CurrentRecord;
			deleted = ListViewClearEditState (true);
			if (currentRecord > TopVisibleRecord)
			{	// completed records may not be shown in the table
				do {
					currentRecord--;
				} while (! TblFindRowID(tableP, (uint16_t)currentRecord, &row) && currentRecord > TopVisibleRecord);
			}
			if (currentRecord <= TopVisibleRecord)
			{
				ListViewScroll (winUp);
				CurrentRecord = deleted ? noRecordSelected : min(currentRecord,LastVisibleRecord);
			}
			else
				CurrentRecord = deleted ? noRecordSelected : currentRecord;
			if (!deleted)
			{
				ItemSelected = true;
				ListViewRestoreEditState();
			}
			if (prevRecord == CurrentRecord && CurrentRecord == TopVisibleRecord 
			&& !(eventP->data.keyDown.modifiers & autoRepeatKeyMask))
			{
				ListViewClearEditState (true);
				FrmNavObjectTakeFocus(FrmGetFormPtr(ToDoSharedData.listViewId), ListCategoryTrigger);
			}
			handled = true;
		}

		else if (gTableNavFocus && ItemSelected && eventP->data.keyDown.chr == vchrRockerDown)
		{
			int16_t row;
			tableP = GetObjectPtr(ToDoSharedData.listTableId, ToDoSharedData.listViewId);
			prevRecord = currentRecord = CurrentRecord;
			deleted = ListViewClearEditState (true);
			if (currentRecord < LastVisibleRecord)
			{	// completed records may not be shown in the table
				do {
					currentRecord++;
				} while (! TblFindRowID(tableP, (uint16_t)currentRecord, &row) && currentRecord < LastVisibleRecord);
			}
			if (currentRecord >= LastVisibleRecord)
			{
				ListViewScroll (winDown);
				CurrentRecord = deleted ? noRecordSelected : max(currentRecord,TopVisibleRecord);
			}
			else
				CurrentRecord = deleted ? noRecordSelected : currentRecord;
			if (!deleted)
			{
				ItemSelected = true;
				ListViewRestoreEditState();
			}
			if (prevRecord == CurrentRecord && CurrentRecord == LastVisibleRecord
			&& !(eventP->data.keyDown.modifiers & autoRepeatKeyMask))
			{
				ListViewClearEditState (true);
				FrmNavObjectTakeFocus(FrmGetFormPtr(ToDoSharedData.listViewId), ListNewToDoButton);
			}
			handled = true;
		}

		else if (gTableNavFocus && eventP->data.keyDown.chr == vchrRockerCenter)
		{
			if (ItemSelected)
			{
				int16_t row;
				tableP = GetObjectPtr(ToDoSharedData.listTableId, ToDoSharedData.listViewId);
				if (TblFindRowID(tableP, (uint16_t)CurrentRecord, &row))
				{
					currentRecord = CurrentRecord;
					ClearEditState();
					if (TblGetItemInt (tableP, row, completedColumn))
						ListViewChangeCompleteStatus (row, 0);
					else
						ListViewChangeCompleteStatus (row, 1);
					ItemSelected = true;
					CurrentRecord = currentRecord;
					TblMarkRowInvalid(tableP, row);
					ListViewRedrawTable(true);
					ListViewRestoreEditState();
				}
			}
			else
			{
				ItemSelected = true;
				CurrentRecord = TopVisibleRecord;
				ListViewRestoreEditState();
			}
			handled = true;
		}

		else if (gTableNavFocus && eventP->data.keyDown.chr == vchrRockerLeft)
		{	
			if (!LoadedAsSharedLib)
			{
				frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
				if (ItemSelected)
				{
					FieldPtr fldP;
					tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);			
					if ((fldP = TblGetCurrentField(tableP)) != NULL)
					{
						if (FldGetInsPtPosition(fldP) == 0)
						{	// first character : move to category trigger
							FrmNavObjectTakeFocus(frmP, ListCategoryTrigger);
							handled = true;
						}
						else if (FldGetInsPtPosition(fldP) == FldGetTextLength(fldP))
						{	// last character : move to first character
							FldSetInsPtPosition(fldP, 0);
							handled = true;
						}
						// otherwise let the field handle it
					}
				}
				else // no item selected : focus to category trigger		
				{
					FrmNavObjectTakeFocus(frmP, ListCategoryTrigger);
					handled = true;
				}
			}
		}

		else if (gTableNavFocus && eventP->data.keyDown.chr == vchrRockerRight)
		{
			if (ItemSelected && !LoadedAsSharedLib)
			{
				FieldPtr fldP;
				frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
				tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);			
				if ((fldP = TblGetCurrentField(tableP)) != NULL
				&&	FldGetInsPtPosition(fldP) == FldGetTextLength(fldP))
				{	// last character : move to details button
					FrmNavObjectTakeFocus(frmP, ListDetailsButton);
					handled = true;
				}
				// otherwise let the field handle it
			}
		}

		else if (eventP->data.keyDown.chr == vchrRockerLeft && !LoadedAsSharedLib)
		{	// rocker left from initial nav state ?
			FrmNavStateFlagsType	navState;
			frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
			FrmGetNavState(frmP, &navState);
			if ((navState & kFrmNavStateFlagsObjectFocusMode) == 0)
			{
				FrmNavObjectTakeFocus(frmP, ListCategoryTrigger);
				handled = true;
			}
		}

		else if (eventP->data.keyDown.chr == vchrRockerRight && !LoadedAsSharedLib)
		{	// rocker right from initial nav state with non-empty table?
			int16_t row;
			FrmNavStateFlagsType	navState;
			frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
			tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);			
			FrmGetNavState(frmP, &navState);
			if ((navState & kFrmNavStateFlagsObjectFocusMode) == 0
			&& TblFindRowID(tableP, (uint16_t)TopVisibleRecord, &row))
			{
				ItemSelected = true;
				CurrentRecord = TopVisibleRecord;
				ListViewRestoreEditState();
				handled = true;
			}
		}

		// Send Data key pressed?
		else if (eventP->data.keyDown.chr == vchrSendData)
		{
			if (ItemSelected)
			{
				tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
				TblReleaseFocus (tableP);
				ToDoSendRecord (ToDoDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
				ListViewRestoreEditState ();
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			handled = true;
		}

		// Confirm key pressed?
		else if (eventP->data.keyDown.chr == vchrConfirm)
		{
			ItemSelected = false;
			// Leave handled false so table releases focus.
		}
	}

	// If the character is printable, then auto-create a new entry
	else if ((isPrint = TxtCharIsPrint (eventP->data.keyDown.chr)) || TxtCharIsCntrl(eventP->data.keyDown.chr))
	{
		if (!ItemSelected && isPrint)
		{	// create a new item with printable char keys
			char buffer[maxCharBytes+1];
			size_t length;
			// Make sure events come in the following order :
			// winUpdate, newRecordInsertedEvent, current char
			ListViewUpdateDisplay(updateRedrawAll);

			// If no item is selected and the character is display,
			// create a new ToDo item.
			ListViewNewToDo();
			MemSet(buffer, maxCharBytes+1, 0);
			length = TxtSetNextChar (buffer, 0, eventP->data.keyDown.chr);
			if (TxtTransliterate (buffer, length, buffer, &length, translitOpUpperCase) == 0)
				TxtGetNextChar (buffer, 0, &eventP->data.keyDown.chr);
			EvtAddEventToQueue (eventP);
			handled = true;
		}
		else if (viewSortingByCategories)
		{	// update clones of this item for printable chars, or backspaces
			ListViewRedrawTable(false);
		}
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    ListViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List View"
 *              of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *
 ***********************************************************************/
static Boolean ListViewHandleEvent (EventPtr eventP)
{
	Boolean 	handled = false;
	int16_t 	row;
	FormPtr 	frmP;
	TablePtr 	tableP;
	ControlPtr	ctlP;
	EventType 	newEvent;
	uint32_t 	noteTitleLen = toDoDescriptionMaxSize;
	uint32_t 	numLibs;
	uint32_t 	uiLibRefNum = 0;
	DmOpenRef 	uiLibDBP = NULL;
	RectangleType rect;

	switch (eventP->eType)
	{
		case frmObjectFocusTakeEvent:
			gTableNavFocus = (eventP->data.frmObjectFocusTake.formID == ToDoSharedData.listViewId
							&& eventP->data.frmObjectFocusTake.objectID == ToDoSharedData.listTableId);
			if (gTableNavFocus)
			{
				if (ItemSelected)
					ListViewRestoreEditState();
				else
				{
					frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
					tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);			
					FrmGetObjectBounds(frmP, FrmGetObjectIndex(frmP, ToDoSharedData.listTableId), &rect);
					FrmSetFocusHighlight(FrmGetWindowHandle(frmP), &rect, 0);
				}
			}					
			break;

		case updateCategoryTriggerEvent:
			// Set the label of the category trigger.
			frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
			ctlP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, ToDoSharedData.ListCategoryTriggerId));
			CatMgrSetTriggerLabel(ToDoDB, &CurrentCategory, 1, ctlP, CategoryName);
			handled = true;
			break;

		case keyDownEvent:
			handled = ListViewHandleKeyDownEvent(eventP);
			break;

		case penDownEvent:
			if (! FrmHandleEvent (FrmGetFormPtr(ToDoSharedData.listViewId), eventP))
			{
				int16_t winWidth, winHeight;

				WinGetDisplayExtent(&winWidth, &winHeight);
				if (eventP->screenY >= winHeight)
					return false;

				tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
				TblReleaseFocus(tableP);
				TblUnhighlightSelection(tableP);
				FrmSetFocus(FrmGetFormPtr(ToDoSharedData.listViewId), noFocus);
				ListViewClearEditState (true);
			}
			handled = true;		// Don't let FrmHandleEvent get this eventP again.
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case ListNewToDoButton:
					ListViewNewToDo();
					handled = true;
					break;

				case ListDetailsButton:
					if (ItemSelected)
					{
						tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
						TblReleaseFocus(tableP);
						TblUnhighlightSelection(tableP);
						FrmSetFocus(FrmGetFormPtr(ToDoSharedData.listViewId), noFocus);
						FrmPopupForm(gApplicationDbP, DetailsDialog);
					}
					else
						FrmAlert(gApplicationDbP, SelectItemAlert);
					handled = true;
					break;

				case ListShowButton:
					FrmPopupForm(gApplicationDbP, OptionsDialog);
					handled = true;
					break;

				case ListUpButton:
					ListViewScroll (winUp);
					handled = true;
					break;

				case ListDownButton:
					ListViewScroll (winDown);
					handled = true;
					break;

				case ListCategoryTrigger:
					ListViewSelectCategory ();
					handled = true;
					break;

				case ListAttachButton:
					if (gAttachRequest)
					{
						if (CurrentRecord == noRecordSelected)
						{
							FrmAlert(gApplicationDbP, (SelectItemAlert));
							handled = true;
							break;
						}

						// Send record vcard
						ToDoSendRecord (ToDoDB, CurrentRecord, gTransportPrefix, NoDataToSendAlert);

						// Send quit eventP to quit application
						MemSet(&newEvent, sizeof(newEvent), 0);
						newEvent.eType = appStopEvent;
						EvtAddEventToQueue(&newEvent);
					}
					handled = true;
					break;
			}
			break;

		case ctlEnterEvent:
			switch (eventP->data.ctlEnter.controlID)
			{
				case ListShowButton:
				case ListUpButton:
				case ListDownButton:
				case ListCategoryTrigger:
					ListViewClearEditState (true);
					break;
			}
			break;

		case ctlExitEvent:
			switch (eventP->data.ctlExit.controlID)
			{
				case ListNewToDoButton:
				case ListDetailsButton:
					ListViewRestoreEditState ();
					break;
			}
			break;

		case ctlRepeatEvent:
			switch (eventP->data.ctlRepeat.controlID)
			{
				case ListUpButton:
					ListViewScroll (winUp);
					break;

				case ListDownButton:
					ListViewScroll (winDown);
					break;
			}
			break;

		case tblSelectEvent:
			ListViewItemSelected (eventP);
			handled = true;
			break;

		case tblEnterEvent:
			if (ItemSelected)
			{
				tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
				if (TblFindRowID(tableP, (uint16_t)CurrentRecord, &row))
				{
					if (eventP->data.tblEnter.row != row)
						handled = ListViewClearEditState (true);
				}
			}
			break;

		case tblExitEvent:
			ListViewClearEditState (true);
			handled = true;
			break;

		case fldHeightChangedEvent:
			ListViewResizeDescription (eventP);
			handled = true;
			break;

		case menuOpenEvent:
			if (! gAttachRequest)
			{
				if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
				{
					MenuHideItem(SendRecordCmd);
					MenuHideItem(SendCategoryCmd);
				}
				else
				{
					MenuShowItem(SendRecordCmd);
					MenuShowItem(SendCategoryCmd);
				}
			}
			else
			{
				// Hide send & Beam commands
				MenuHideItem(BeamRecordCmd);
				MenuHideItem(BeamCategoryCmd);
				MenuHideItem(SendRecordCmd);
				MenuHideItem(SendCategoryCmd);

				// Show attach commands
				MenuShowItem(AttachRecordCmd);
				MenuShowItem(AttachCategoryCmd);
			}
			// don't set handled = true
			break;

		case menuEvent:
			handled = ListViewDoCommand (eventP->data.menu.itemID);
			break;

		case menuCmdBarOpenEvent:
			// Add the buttons that we want available on the command bar, based on the current context
			// Load UILIb to get its module DB
			SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
			SysGetModuleDatabase(uiLibRefNum, NULL, &uiLibDBP);

			if (ItemSelected)
			{
				FieldType* fldP;
				size_t startPos, endPos;

				fldP = TblGetCurrentField(GetObjectPtr(ToDoSharedData.listTableId, ToDoSharedData.listViewId));
				FldGetSelection(fldP, &startPos, &endPos);

				if (startPos == endPos)  // there's no highlighted text, but an item is chosen
				{
					// Call directly Field eventP handler so that system edit buttons are added if applicable
					FldHandleEvent(fldP, eventP);

					MenuCmdBarAddButton(menuCmdBarOnRight, uiLibDBP, BarDeleteBitmap, menuCmdBarResultMenuItem, DeleteCmd, 0);
					MenuCmdBarAddButton(menuCmdBarOnLeft, uiLibDBP, BarSecureBitmap, menuCmdBarResultMenuItem, SecurityCmd, 0);
					MenuCmdBarAddButton(menuCmdBarOnLeft, uiLibDBP, BarBeamBitmap, menuCmdBarResultMenuItem, BeamRecordCmd, 0);

					// Prevent the field package to add edit buttons again
					eventP->data.menuCmdBarOpen.preventFieldButtons = true;
				}
			}
			else	// no item is chosen
			{
				MenuCmdBarAddButton(menuCmdBarOnLeft, uiLibDBP, BarSecureBitmap, menuCmdBarResultMenuItem, SecurityCmd, 0);
			}
			// Unload the UILib
			SysUnloadModule(uiLibRefNum);
			// don't set handled to true; this eventP must fall through to the system.
			break;

		case frmOpenEvent:
			frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
			ListViewInit (frmP);

			if (PendingUpdate)
			{
				ListViewUpdateDisplay (PendingUpdate);
				PendingUpdate = 0;
			}
			ListViewRestoreEditState ();
			handled = true;
			break;

	 	case frmGotoEvent:
			frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
			ListViewGotoItem (eventP);
			ListViewInit (frmP);

			if (gMatchColumn == todoNoteColId)
			{
				DbCursorSetAbsolutePosition(gListViewCursorID, CurrentRecord);
				DbCopyColumnValue(ToDoDB, gListViewCursorID, todoDescriptionColId, 0, (void*)NoteTitle, &noteTitleLen);
				InNoteView = true;
				EditNoteSelection(noteTitleLen ? NoteTitle : NULL, ToDoDB, gListViewCursorID, CurrentRecord, todoNoteColId, (uint16_t)eventP->data.frmGoto.matchPos, (uint16_t)eventP->data.frmGoto.matchLen, &ToDoDeleteNote);
				InNoteView = false;
				ListViewUpdateDisplay(updateDisplayOptsChanged);
			}
			else
			{
				// Select text here
				ListViewSetSelection();
			}

			ListViewRestoreEditState ();
			handled = true;
			break;

		case winUpdateEvent:
			if (eventP->data.winUpdate.window != gListViewWinH)
				break;

#ifndef BACKBUFFERED
			// Item crossing has been delayed 
			if (gCrossOutRow != -1)
			{
				frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
				FrmDrawForm(frmP);
				tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
				ListViewCrossOutItem (tableP, gCrossOutRow);
				gCrossOutRow = -1;
				SysTaskDelay(crossOutDelay);
				handled = true;
			}
#endif

			if (gTblDrawCode)
			{
				tableP = GetObjectPtr (ToDoSharedData.listTableId, ToDoSharedData.listViewId);
				ListViewRestoreEditState ();
				gTblDrawCode = 0 ;
				frmP = FrmGetFormPtr(ToDoSharedData.listViewId);
				FrmDrawForm(frmP);
				handled = true;
			}

			break;

		case winResizedEvent:
			if (gListViewWinH != eventP->data.winResized.window)
				break;
			ListViewResizeForm(eventP, false);
			if (!InNoteView && !InDetails && CurrentRecord != noRecordSelected)
			{
				ListViewRestoreEditState();
				ListViewSetSelection();
			}
			break;

		case frmSaveEvent:
			ListViewClearEditState (true);
			// This deletes empty items. It can do this because we don't do a FrmSaveAllForms
			// on a sysAppLaunchCmdSaveData launch.
			break;

		case frmCloseEvent:
			if(EditedItemH)
			{
				MemHandleFree(EditedItemH);
				EditedItemH = NULL;
			}
			break;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    CustomAcceptBeamDialog
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
 *			Name	Date		Description
 *			----	----		-----------
 *			bhall	9/7/99	Initial Revision
 *			gavin 11/9/99  Rewritten to use new ExgDoDialog function
 *
 ***********************************************************************/
static status_t CustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP)
{
	ExgDialogInfoType	exgInfo;
	status_t			err = errNone;
	Boolean				result = false;

	memset(&exgInfo, 0, sizeof(ExgDialogInfoType));

	if (dbP && !gCheckingPassword)
	{
		// set default category to unfiled
		exgInfo.categoryIndex = catIDUnfiled;
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
 * FUNCTION:    ApplicationHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and sets the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/11/95	Initial Revision
 *			jmp	9/17/99	Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
static Boolean ApplicationHandleEvent (EventPtr event)
{
	uint16_t formID;
	FormPtr frm;

	if (event->eType == frmLoadEvent)
	{
		// Load the form resource.
		formID = event->data.frmLoad.formID;
		frm = FrmInitForm(gApplicationDbP, formID);
		FrmSetActiveForm (frm);
		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmDispatchEvent each time is receives an
		// event.
		switch (formID)
		{
			case ListView:
				gListViewWinH = FrmGetWindowHandle(frm);
				FrmGetFormInitialBounds(frm, &gListViewWinBounds);
				FrmSetEventHandler (frm, ListViewHandleEvent);
				break;

			case DetailsDialog:
				gDetailsDialogWinH = FrmGetWindowHandle(frm);
				FrmSetEventHandler (frm, DetailsHandleEvent);
				break;

			case OptionsDialog:
				gOptionsDialogWinH = FrmGetWindowHandle(frm);
				FrmSetEventHandler (frm, OptionsHandleEvent);
				break;
		}
		return (true);
	}
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    EventLoop
 *
 * DESCRIPTION: This routine is the event loop for the ToDo
 *              aplication.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void EventLoop (void)
{
	status_t error;
	EventType event;

	do
	{
		EvtGetEvent (&event, evtWaitForever);

		if (! SysHandleEvent (&event))

			if (! MenuHandleEvent (CurrentMenu, &event, &error))

				if (! ApplicationHandleEvent (&event))

					FrmDispatchEvent (&event);

	}
	while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:    PrvCheckForAttachURL
 *
 * DESCRIPTION: Extracts from the socket URL the attach transport scheme
 *				if the datebook was sublaunched by the exchange manager
 *
 *
 * PARAMETERS:  socketP - exchange socket from exgGet launch code
 *				attachTransportScheme - transport to use for sending data back
 *				targetID - target application to use when transport is _local.
 *
 * RETURNED:    true if we got a scheme
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			PLe		08/13/02	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvCheckForAttachURL(ExgSocketType* socketP, char* attachTransportScheme, uint32_t *targetID)
{
	char *endP;
	if (!socketP || !attachTransportScheme) return false;

	*attachTransportScheme = 0;
	if (socketP->name) {
		endP = StrChr(socketP->name, chrColon);
		if (endP)
			// Copy including colon
			StrNCopy(attachTransportScheme,socketP->name, max((endP-socketP->name)+1, attachTransportPrefixMaxSize));
	}
	if (*attachTransportScheme ==0)
		StrCopy(attachTransportScheme,exgLocalPrefix); // assume local by default
	if (targetID)
		*targetID = socketP->goToCreator;
	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppSwitchByCreator
 *
 * DESCRIPTION: Switches to an application given a creatorID
 *
 * PARAMETERS:  creator - creatorID of app to switch to
 *				cmd     - launch code
 *				cmdPBP  - command parameters
 *				cmdPBPSize  - size of cmdPBP block
 *
 * RETURNED:    err - zero if none.
 *
 ***********************************************************************/
static status_t PrvAppSwitchByCreator(uint32_t creator, uint16_t cmd, MemPtr cmdPBP, uint32_t cmdPBSize)
{
	status_t err;
	DatabaseID appDBID;

	/* Find the application */
	appDBID = DmFindDatabaseByTypeCreator(sysFileTApplication, creator, dmFindExtendedDB, NULL);
	if (appDBID == NULL)
		err = DmGetLastErr();
	else
 		err = SysUIAppSwitch(appDBID, cmd, cmdPBP, cmdPBSize );
	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvExgMakePassableSocket
 *
 * DESCRIPTION: This creates a duplicate copy of an exchange socket
 *              The new socket memory is self contained, all the associated memory
 *               pointers are in the same allocated memory block
 *				Owner is set to system for launch calls
 *
 * PARAMETERS:  socketP - exchange socket to  clone
 *
 * RETURNED:    new passable exchange socket pointer or NULL.
 *
 ***********************************************************************/
static ExgPassableSocketType * PrvExgMakePassableSocket(ExgSocketType * socketP)
{
	ExgPassableSocketType *newSockP;
	// allocate space for new socket and the three associated strings
	newSockP = (ExgPassableSocketType *)MemPtrNew(sizeof(ExgPassableSocketType));
	if (!newSockP) return NULL;

	MemSet(newSockP,sizeof(ExgPassableSocketType),0); // fill  null
	// copy the socket data
	MemMove(&newSockP->socket,socketP,sizeof(ExgSocketType));
	newSockP->socket.socketRef = 0; // can't duplicate socketRef...
	newSockP->socket.componentIndex  = 0; // or this
	// now copy the string data from the source to the buffer after the new socket
	// and make the new socket point to those strings
	if (socketP->name)
		StrNCopy(&newSockP->name[0], socketP->name, exgMaxTypeLength);
	newSockP->socket.name = &newSockP->name[0];
	if (socketP->description)
		StrNCopy(&newSockP->description[0],socketP->description, exgMaxTypeLength);
	newSockP->socket.description = &newSockP->description[0];
	if (socketP->type)
		StrNCopy(&newSockP->type[0],socketP->type, exgMaxTypeLength);
	newSockP->socket.type = &newSockP->type[0];
	MemPtrSetOwner(newSockP,0); // assign to system memory so that we can
								// pass this via an appswitch
	return (newSockP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvExgFixupPassableSocket
 *
 * DESCRIPTION: This repairs pointer references in a passable socket passed through the app manager
 *
 * PARAMETERS:  socketP - passable exchange socket to  clone
 *
 * RETURNED:    repaired exchange socket pointer.
 *
 ***********************************************************************/
static ExgSocketType *PrvExgFixupPassableSocket(ExgPassableSocketType *socketP)
{
    // reconstruct the socket passed by clone - SysUIAppSwitch now makes a copy and munches pointer references.
	socketP->socket.name = socketP->name;
	socketP->socket.description = socketP->description;
	socketP->socket.type = socketP->type;
	return &(socketP->socket);
}

/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the ToDo
 *              application.
 *
 * PARAMETERS:  cmd			 - launch code
 *              cmdPBP      - paramenter block (launch code specific)
 *              launchFlags - SysAppLaunch flags (ses SystemMgr.h)
 *
 * RETURNED:    error code
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	10/4/99	Replace calls to DmOpenDatabaseByTypeCreator() with
 *								ToDoGetDatabase().
 *			jmp	10/18/99	If the default database image doesn't exist, then create
 *								an empty database.
 *			jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from sysAppLaunchCmdExgAskUser
 *								since it was already being done in sysAppLaunchCmdExgReceiveData if
 *								the user affirmed sysAppLaunchCmdExgAskUser.  Also, in sysAppLaunchCmdExgReceiveData
 *								prevent call FrmSaveAllForms() if we're being call back through
 *								PhoneNumberLookup() as the two tasks are incompatible with each other.
 *
 ***********************************************************************/
uint32_t	PilotMain (uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t error = errNone;
	DmOpenRef dbP;

	uint32_t cursorID = dbInvalidCursorID;

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Init( "ToDo", cmd );
	#endif

	TraceOutput(TL(appErrorClass, "====> ToDo entering PilotMain cmd=%d",cmd));

	// Normal Launch
	switch (cmd)
	{
	case sysAppLaunchCmdNormalLaunch:
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdNormalLaunch"));

		// Set the launched flag, it will be used to know if ToDo is normally launched
		sToDoAppNormalLaunched = true;
		LoadedAsSharedLib = false;

		error = StartApplication ();
		if (error < errNone)
			goto ExitError;

		FrmGotoForm(gApplicationDbP, ToDoSharedData.listViewId);
		EventLoop ();
		StopApplication ();

		// Reset the launch flag.
		sToDoAppNormalLaunched = false;
		break;

	case sysAppLaunchCmdExgGetData:
	{
		// handling get request - since we want to be fully launched, clone the socket and
		// AppSwitch to ourselves, otherwise we could respond here
		ExgPassableSocketType * socketP = PrvExgMakePassableSocket((ExgSocketPtr) cmdPBP);
		if (socketP) {
			// we really only need the name and goToCreatorID fields, but we'll copy the whole struture here.
			error = PrvAppSwitchByCreator(sysFileCToDo, appLaunchCmdExgGetFullLaunch, (MemPtr) socketP , sizeof(ExgPassableSocketType));
			if (error < errNone)
			{
				error = exgErrUserCancel; // return cancel error to let caller skip reading
				break;
			}
			else
				MemPtrFree(socketP);
		}
		else error = exgErrAppError;
	}
	break;

	case appLaunchCmdExgGetFullLaunch: // this is our custom full launch code
	{
		// repair socket from transit
		ExgSocketType *socketP = PrvExgFixupPassableSocket((ExgPassableSocketType *)cmdPBP);

		// Test if the PIM is called for attach
		gAttachRequest = PrvCheckForAttachURL(socketP, gTransportPrefix, NULL);

		error = StartApplication ();
		if (error != errNone)
			goto ExitError;

		FrmGotoForm (gApplicationDbP, ListView);
		EventLoop ();
		StopApplication ();

     	ExgNotifyGoto(socketP, 0);
		break;
	}

	case sysAppLaunchCmdFind:
		Search((FindParamsPtr)cmdPBP);
		break;

	// This action code is sent to the app when the user hit the "Go To"
	// button in the Find Results dialog.
	case sysAppLaunchCmdGoTo:
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdGoTo"));
		LoadedAsSharedLib = false;

		if (sToDoAppNormalLaunched)
		{
			// The application is already running
			GoToItem ((GoToParamsPtr) cmdPBP, false);
		}
		else
		{
			// The application was not running.
			sToDoAppNormalLaunched = true;
			error = StartApplication ();
			if (error < errNone)
				goto ExitError;

			GoToItem ((GoToParamsPtr) cmdPBP, true);

			EventLoop ();
			StopApplication ();
			sToDoAppNormalLaunched = false;
		}
		break;

	case sysAppLaunchCmdSyncNotify:
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdSyncNotify"));
		SyncNotification ();
		break;

   // This launch code is sent after the system is reset.  We use this time
   // to create our default database.  If there is no default database image,
   // then we create an empty database.
	case sysAppLaunchCmdSystemReset:
		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->hardReset)
			TraceOutput(TL(appErrorClass, ">>>> TODO : sysAppLaunchCmdSystemReset: HARD RESET"));
		else
			TraceOutput(TL(appErrorClass, ">>>> TODO : sysAppLaunchCmdSystemReset: SOFT RESET"));

		// Register to receive VTODO objects
		RegisterData();

		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
		{
			TraceOutput(TL(appErrorClass, ">>>> TODO : sysAppLaunchCmdSystemReset: CREATE DEFAULT DB"));
			error = CreateDefaultDatabase();
		}
		break;

	case sysAppLaunchCmdExgAskUser:
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdExgAskUser"));

		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			error = ToDoGetDatabase (&dbP, dmModeReadWrite);
		}
		else
			dbP = ToDoDB;

      	if (error >= errNone)
     		CustomAcceptBeamDialog (dbP, (ExgAskParamPtr) cmdPBP);

		if (!(launchFlags & sysAppLaunchFlagSubCall) && dbP)
			DbCloseDatabase(dbP);

		break;

   // Receive the record.  The app will parse the data and add it to the database.
   // This data should be displayed by the app.
   // ABa: todoLaunchCmdImportVObject added in Palm OS 4.0.
	case sysAppLaunchCmdExgReceiveData:
	{
		uint32_t currentUID = 0;

		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdExgReceiveData or todoLaunchCmdImportVObject"));

		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			error = ToDoGetDatabase (&dbP, dmModeReadWrite);
			if (error >= errNone)
				error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}
		else
		{
			dbP = ToDoDB;
			ToDoOpenIOCursor(dbP, &gListViewCursorID);
			// We don't delete the current record if it's empty because the user
			// could cancel the beam receive.

			// ToDoReceiveData() inserts the received record in sorted order. This may change the
			// index of the current record. So we remember its UID here, and refresh our copy of
			// its index afterwards.
			if (CurrentRecord != noRecordSelected)
				DbCursorGetRowIDForPosition(gListViewCursorID, CurrentRecord, &currentUID);
		}

		if (error < errNone || dbP == NULL)
		{
			error = exgErrAppError;	// DOLATER dje - use a new error code - "try again after switching apps"
			break;
		}

		error = ToDoReceiveData(dbP, (ExgSocketPtr) cmdPBP);

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			ToDoCloseCursor(&gListViewCursorID);
			DbCloseDatabase(dbP);
		}
		else
		{
			// restore global cursor settings
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
			if (CurrentRecord != noRecordSelected)
			{
				if (DbCursorGetPositionForRowID(gListViewCursorID, currentUID, &CurrentRecord) != 0)
					CurrentRecord = noRecordSelected;	// Can't happen, but...

				// DOLATER dje -
				//		To fix the off-by-one error, we can decrement exgSocketP->goToParams.recordNum
				//		if it's after the current empty record in order to compensate for the
				//		current empty record getting deleted when we exit before the goto launch.
			}
		}
		break;
	}

	// This action code is sent by the DesktopLink server when it creates
	// a new database.  We will initializes the new database.
	case sysAppLaunchCmdInitDatabase:
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdInitDatabase"));

		// Set the backup bit.  This is to aid syncs with non Palm software.
		ToDoSetDBBackupBit(((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
		break;

	case sysAppLaunchCmdExportRecordGetCount:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
			error = ToDoGetDatabase (&dbP, (PrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
		}
		else
			dbP = ToDoDB;

		// Assign the number of records
		if (dbP != NULL && error >= errNone)
			{
				ToDoOpenIOCursor(dbP, &cursorID);
				*((uint32_t*)cmdPBP) = DbCursorGetRowCount(cursorID);
				ToDoCloseCursor(&cursorID);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
					DbCloseDatabase(dbP);
			}
			break;

	case sysAppLaunchCmdExportRecord:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
			error = ToDoGetDatabase (&dbP, (PrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
		}
		else
			dbP = ToDoDB;

		if (dbP != NULL && error >= errNone)
		{
			ToDoOpenIOCursor(dbP, &gListViewCursorID);
			error = ToDoExportData(dbP, (ImportExportRecordParamsPtr)cmdPBP);
		}

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			ToDoCloseCursor(&gListViewCursorID);
			DbCloseDatabase(dbP);
		}
		else
			// restore global cursor settings
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

		break;

	case sysAppLaunchCmdImportRecord:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
			error = ToDoGetDatabase (&dbP, (PrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			if (error >= errNone)
				error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}
		else
		{
			dbP = ToDoDB;
			error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}

		if (dbP != NULL && error >= errNone)
			error = ToDoImportData(dbP, (ImportExportRecordParamsPtr)cmdPBP);

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			ToDoCloseCursor(&gListViewCursorID);
			DbCloseDatabase(dbP);
		}
		else
			// restore global cursor settings
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
		break;

	case sysAppLaunchCmdDeleteRecord:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
			error = ToDoGetDatabase (&dbP, (PrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			if (error >= errNone)
				error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}
		else
		{
			dbP = ToDoDB;
			error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}

		if (dbP != NULL && error >= errNone)
			error = ToDoDeleteRecord(dbP, (ImportExportRecordParamsPtr)cmdPBP);

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			ToDoCloseCursor(&gListViewCursorID);
			DbCloseDatabase(dbP);
		}
		else
			// restore global cursor settings
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
		break;

	case sysAppLaunchCmdMoveRecord:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
			error = ToDoGetDatabase (&dbP, (PrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			if (error >= errNone)
				error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}
		else
		{
			dbP = ToDoDB;
			error = ToDoOpenIOCursor(dbP, &gListViewCursorID);
		}

		if (dbP != NULL && error >= errNone)
			error = ToDoMoveRecord(dbP, (ImportExportRecordParamsPtr)cmdPBP);

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			ToDoCloseCursor(&gListViewCursorID);
			DbCloseDatabase(dbP);
		}
		else
			// restore global cursor settings
			ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
		break;

	case sysLaunchCmdInitialize:
		// Get the application dBRef
		error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gApplicationDbP);
		break;

	default:
		break;
	}

ExitError:

	TraceOutput(TL(appErrorClass, "<==== ToDo exiting PilotMain cmd=%d errCode=%d",cmd,error));

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Exit();
	#endif

	return error;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoLoadPrefs
 *
 * DESCRIPTION: Read the preferences and handle previous and future
 *					 versions of the prefs.
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			BGT	01/08/98	Initial Revision
 *			kwk	06/23/99	Use glue code for default Note/ListFont if prefs
 *								don't provide that information.
 *			grant	01/18/01	Only change noteFont from largeFont to largeBoldFont
 *								if we are updating prefs from an earlier version.
 *
 ***********************************************************************/
void ToDoLoadPrefs(void)
{
	ToDoPreferenceType prefs;
	uint32_t prefsSize = (uint32_t)noPreferenceFound;
	int16_t prefsVersion;

	// Read the preferences / saved-state information. If we get an older version of
	// the prefs, sync our new note font field with the original pref field.
	prefsSize = (uint32_t)sizeof (ToDoPreferenceType);

	prefsVersion = PrefGetAppPreferences (sysFileCToDo, todoPrefID, &prefs, &prefsSize, true);
	if (prefsVersion > toDoPrefsVersionNum)
	{
		prefsVersion = noPreferenceFound;
	}
	if (prefsVersion > noPreferenceFound)
	{
		if (prefsVersion <= toDoPrefsVerUpdateFonts)
		{
			prefs.noteFont = prefs.v20NoteFont;

			// Use the 'better' large font if we've got it, since version 2
			// prefs would have been created on an older version of the OS
			// which didn't have the largeBoldFont available.
			if (prefs.noteFont == largeFont)
				prefs.noteFont = largeBoldFont;
		}

		CurrentCategory = prefs.currentCategory;
		NoteFont = prefs.noteFont;
		ShowAllCategories = prefs.showAllCategories;
		ShowCompletedItems = prefs.showCompletedItems;
		ShowOnlyDueItems = prefs.showOnlyDueItems;
		ShowDueDates = prefs.showDueDates;
		ShowPriorities = prefs.showPriorities;
		ShowCategories = prefs.showCategories;
		ChangeDueDate = prefs.changeDueDate;
		SaveBackup = prefs.saveBackup;
		CurrentSortId = prefs.sortId;


		// Support transferal of preferences from the second version of the preferences.
		if (prefsVersion == toDoPrefsVersionNum)
		{
			ToDoListFont = ListFont = prefs.listFont;
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    ToDoSavePrefs
 *
 * DESCRIPTION: Save the preferences and handle previous and future
 *					 versions of the prefs.
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			BGT	1/8/98	Initial Revision
 *
 ***********************************************************************/
void ToDoSavePrefs(void)
{
	ToDoPreferenceType prefs;

	// Write the preferences / saved-state information.
	prefs.currentCategory = (uint16_t)CurrentCategory;
	prefs.noteFont = NoteFont;

	if (prefs.noteFont > largeFont) {
		prefs.v20NoteFont = stdFont;
	}
	else {
		prefs.v20NoteFont = prefs.noteFont;
	}

	prefs.showAllCategories = ShowAllCategories;
	prefs.showCompletedItems = ShowCompletedItems;
	prefs.showOnlyDueItems = ShowOnlyDueItems;
	prefs.showDueDates = ShowDueDates;
	prefs.showPriorities = ShowPriorities;
	prefs.showCategories = ShowCategories;
	prefs.changeDueDate = ChangeDueDate;
	prefs.saveBackup = SaveBackup;
	prefs.listFont = LoadedAsSharedLib ? ToDoListFont : ListFont;
	prefs.sortId = CurrentSortId ;

	PrefSetAppPreferences (sysFileCToDo, todoPrefID, toDoPrefsVersionNum, &prefs, sizeof (ToDoPreferenceType), true);
}

/***********************************************************************
 *
 * FUNCTION:    PrvBackgroundGadgetHandler
 *
 * DESCRIPTION: Draws the background for the todo app.
 *
 * PARAMETERS:  gadgetP - points to the gadget structure
 *					 cmd - gadget rendering command
 *					 paramP - gadget command parameters
 *
 * RETURNED:    TRUE
 *
 ***********************************************************************/
Boolean PrvBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP)
{
	if( cmd == formGadgetDrawCmd )
	{
		RectangleType gadgetRect;
		RectangleType tableRect;
		FormPtr frmP = FrmGetActiveForm();
		TableType *table = NULL;

		GcHandle gc = NULL;

		// Get a copy of the gadget's rect
		memcpy( &gadgetRect, &gadgetP->rect, sizeof(gadgetRect) );
		WinSetCoordinateSystem(kCoordinatesNative);
		WinScaleRectangle( &gadgetRect );

		// Get a copy of the table bounds
		FrmGetObjectBounds(
			frmP,
			FrmGetObjectIndex(
				frmP,
				ListTable),
			&tableRect);
		WinScaleRectangle( &tableRect );


		gc = GcGetCurrentContext();

		GcPushState(gc);

		GcSetColor(
			gc,
			gAlternatingListHighlightColor.r,
			gAlternatingListHighlightColor.g,
			gAlternatingListHighlightColor.b,
			255/*alpha*/ );

		table = FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, ListTable ) );
		{
			int16_t rowCount;
			int16_t rowIndex;

			// How many rows are displayed in the table?
			rowCount = TblGetLastUsableRow(table);
			for( rowIndex = 0; rowIndex <= rowCount; rowIndex += 2 )
			{
				RectangleType itemBounds;

				// Get the item boundss...
				TblGetItemBounds( table, rowIndex, 2, &itemBounds );
				WinScaleRectangle( &itemBounds );

				// Adjust the bounds rect so that it spans the entire
				itemBounds.topLeft.x = tableRect.topLeft.x;
				itemBounds.extent.x = tableRect.extent.x;

				GcRect(
					gc,
					itemBounds.topLeft.x,
					itemBounds.topLeft.y,
					itemBounds.topLeft.x + itemBounds.extent.x,
					itemBounds.topLeft.y + itemBounds.extent.y);
				GcPaint(gc);
			}
		}

		GcPopState(gc);
		GcReleaseContext(gc);
	}

	return TRUE;
}

status_t PrvToDoGetColorResource(
	DmOpenRef dbP,
	DmResourceID id,
	RGBColorType *color)
{
	status_t err = errNone;
	char		*strP = NULL;
	uint8_t	colorValue;
	char		parseStr[4];
	MemHandle resH = NULL;

	if(!dbP)
	{
		err = dmErrNoOpenDatabase;
		goto Exit;
	}

	resH = DmGetResource( dbP, strRsc, id );
	strP = MemHandleLock( resH );

	if(!strP)
	{
		err = dmErrResourceNotFound;
		goto Exit;
	}

	if(strlen(strP) != 7 )  // Has to be #123456 format
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	parseStr[2] = '\0';

	parseStr[0] = strP[1];
	parseStr[1] = strP[2];
	colorValue = (uint8_t)strtol(parseStr, NULL, 16);
	color->r = colorValue;

	parseStr[0] = strP[3];
	parseStr[1] = strP[4];
	colorValue = (uint8_t)strtol(parseStr, NULL, 16);
	color->g = colorValue;

	parseStr[0] = strP[5];
	parseStr[1] = strP[6];
	colorValue = (uint8_t)strtol(parseStr, NULL, 16);
	color->b = colorValue;

 Exit:
	if( strP )
	{
		MemPtrUnlock(strP);
		strP = NULL;
	}

	if( resH )
	{
		DmReleaseResource( resH );
		resH = NULL;
	}

	return err;
}
