/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoMain.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This is the Memo application's main module.  This module
 *   starts the application, dispatches events, and stops
 *   the application.
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
#include <TextMgr.h>
#include <PrivateRecords.h>
#include <Menu.h>
#include <Form.h>
#include <FormLayout.h>
#include <ScrollBar.h>
#include <Table.h>
#include <string.h>
#include <CmnLaunchCodes.h>
#include <SystemResources.h>
#include <Preferences.h>
#include <SchemaDatabases.h>
#include <ErrorMgr.h>
#include <NotifyMgr.h>
#include <UIResources.h>
#include <FeatureMgr.h>
#include <StringMgr.h>
#include <UIColor.h>
#include <Find.h>
#include <SysEvtMgr.h>
#include <FontSelect.h>
#include <AboutBox.h>
#include <SoundMgr.h>
#include <TraceMgr.h>
#include <ExgMgr.h>
#include <ExgLocalLib.h>
#include <PenInputMgr.h>
#include <AppMgr.h>
#include <CatMgr.h>
#include <Loader.h>
#include <stdlib.h>

#include <SearchLib.h>
#include <PhoneLookup.h>

#include <DebugMgr.h>

#include "MemoTransfer.h"
#include "MemoRsc.h"
#include "MemoMain.h"
#include "MemoList.h"
#include "MemoEdit.h"
#include "MemoPrefs.h"
#include "MemoDB.h"


// exg socket structure for sysAppLaunchCmdExgGetFullLaunch
typedef struct {
	ExgSocketType socket;
	char name[exgMaxTypeLength+1];
	char description[exgMaxTypeLength+1];
	char type[exgMaxTypeLength+1];
} ExgPassableSocketType;

/***********************************************************************
 *	Global variables
 ***********************************************************************/
char					gCategoryName [catCategoryNameLength];

privateRecordViewEnum	gPrivateRecordVisualStatus;
MenuBarPtr				gCurrentMenu;

uint32_t				gTopVisibleRecord = 1;
uint32_t				gCurrentRecord = noRecordSelected;
uint16_t				gCurrentView = ListView;

CategoryID *			gCurrentCategoriesP = NULL;
uint32_t				gCurrentNumCategories = 0;

FontID					gListFont = stdFont;
FontID					gEditFont = stdFont;
uint32_t				gEditScrollPosition = 0;
Boolean					gSaveBackup = true;
Boolean					gSortAlphabetically = true;
uint32_t				gListViewCursorID = dbInvalidCursorID;

Boolean					gAttachRequest = false;				// launch for attachments
char					gTransportPrefix[attachTransportPrefixMaxSize+1];

DmOpenRef				gApplicationDbP = NULL;
DmOpenRef				gMemoDB = NULL;

int32_t 				gEventLoopWaitTime = evtWaitForever;

Boolean 				gForceReload = false ;
Boolean					gCheckingPassword = false;


// current window bounds.
RectangleType 			gCurrentWinBounds;

// new record creation from listview
Boolean					gEnqueueNewChar = false;
EventType				gNewCharBuffer[kNewCharBufSize];
size_t					gNewCharBufferCount = 0;

extern memolist_t *		gMemolistCache;
extern int16_t			gMemolistCacheSize;
extern int16_t			gSelectedRow;

RGBColorType		gMemoListDarkenedListEntry;

RGBColorType	   gEditBackgroundColor;
RGBColorType		gEditSideLineColor;
RGBColorType		gEditFieldLineColor;

status_t PrvToDoGetColorResource(
	DmOpenRef dbP,
	DmResourceID id,
	RGBColorType *color);


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
 *			grant 4/6/99	Moved code to set backup bit into SetDBBackupBit
 *			jmp	10/2/99	Call new MemoGetDataBase() to create database
 *								if it doesn't already exist.
 *
 ***********************************************************************/
static status_t StartApplication(void)
{
	status_t	err = errNone;
	uint16_t	attr;
	uint16_t	mode;
	Boolean		recordFound = false;
	uint16_t	catIndex;

	PrvToDoGetColorResource(
		gApplicationDbP,
		MemoListDarkenedEntryColorStr,
		&gMemoListDarkenedListEntry );

	PrvToDoGetColorResource(
		gApplicationDbP,
		EditBackgroundColorStr,
		&gEditBackgroundColor);

	PrvToDoGetColorResource(
		gApplicationDbP,
		EditSideLineColorStr,
		&gEditSideLineColor);

	PrvToDoGetColorResource(
		gApplicationDbP,
		EditFieldLineColorStr,
		&gEditFieldLineColor);

	// Determime if secret record should be shown.
	gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
	if (gPrivateRecordVisualStatus == hidePrivateRecords)
		mode = dmModeReadWrite;
	else
		mode = dmModeReadWrite | dmModeShowSecret;

	// Get schema DB
	gMemoDB = MemoDBOpenDatabase(mode);

	gCurrentRecord = noRecordSelected;

	// Read the preferences.
	MemoLoadPrefs();

	// Current categories
	MemoLoadCurrentCategories(&gCurrentNumCategories, &gCurrentCategoriesP);

	// Open a default cursor
	err = MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;


	// The file may have been synchronized since the last time we used it,
	// check that the current record and the currrent category still
	// exist.  Also, if secret records are being hidden, check if the
	// the current record is marked secret.
	catIndex = 0;
	while (catIndex < gCurrentNumCategories)
	{
		CatMgrGetName(gMemoDB, gCurrentCategoriesP[catIndex], gCategoryName);
		if (*gCategoryName == 0)
		{
			CategoryID	cat = catIDAll;

			// Reset to catIDAll
			ChangeCategory(&cat, 1);
			break;
		}

		catIndex++;
	}

	if (DbGetRowAttr(gMemoDB, gCurrentRecord, &attr) == errNone)
	{
		recordFound = (gPrivateRecordVisualStatus == showPrivateRecords || !(attr & dbRecAttrSecret));
	}

	if (! recordFound)
	{
		gTopVisibleRecord = 1;
		gCurrentRecord = noRecordSelected;
		gCurrentView = ListView;
		gEditScrollPosition = 0;
	}

	return (err);
}


/***********************************************************************
 *
 * FUNCTION:    StopApplication
 *
 * DESCRIPTION: This routine closes the application's database
 *              and saves the current state of the application.
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
static void StopApplication(void)
{
	uint32_t	scrollPosition = 0;
	uint16_t	i;
	FormType *	formP;
	FieldType *	fieldP;

	// If we are in the "edit view", get the current scroll position.
	if ((gCurrentView == EditView) && (gCurrentRecord != noRecordSelected))
	{
		formP = FrmGetFormPtr (EditView);
		fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex (formP, EditMemoField));
		scrollPosition = FldGetScrollPosition(fieldP);
	}

	// Close all open forms,  this will force any unsaved data to
	// be written to the database.
	FrmSaveAllForms();
	FrmCloseAllForms();

	// Write the preferences / saved-state information.
	MemoSavePrefs(scrollPosition);

	MemoSaveCurrentCategories(gCurrentNumCategories, gCurrentCategoriesP);
	if (gCurrentCategoriesP)
		MemPtrFree(gCurrentCategoriesP);

	// Close the application's data file.
	if (gListViewCursorID != dbInvalidCursorID)
	{
		DbCursorClose(gListViewCursorID);
		gListViewCursorID = dbInvalidCursorID;
	}

	DbCloseDatabase(gMemoDB);
	gMemoDB = NULL;

	// free the cache...
	for (i = 0; i < gMemolistCacheSize; i++)
	{
		if (gMemolistCache[i].titleAlloc)
			MemPtrFree(gMemolistCache[i].title);
	}

	MemPtrFree(gMemolistCache);
}


/***********************************************************************
 *
 * FUNCTION:    SyncNotification
 *
 * DESCRIPTION: This routine is an entry point of the memo application.
 *              It is called when the application's database is
 *              synchronized.  This routine will resort the database.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/17/96	Initial Revision
 *		jmp	10/02/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *								MemoGetDatabase().
 *
 ***********************************************************************/
static void SyncNotification (void)
{
	DmOpenRef dbP;

	// Find the application's data file.
	dbP = MemoDBOpenDatabase(dmModeReadWrite);
	if (!dbP)
		return;

	DbCloseDatabase (dbP);
}

/***********************************************************************
 *
 * FUNCTION:    RegisterData
 *
 * DESCRIPTION: Register with the Exchange Manager to receive .txt and
 *				text/plain.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		7/28/00		dje	Initial Revision.
 *
 ***********************************************************************/
void RegisterData(void)
{
	MemHandle resH;
	char * descP;

	resH = DmGetResource(gApplicationDbP, strRsc, ExgDescriptionStr);
	descP = MemHandleLock(resH);
	ExgRegisterDatatype(sysFileCMemo, exgRegExtensionID, memoExtension, descP, 0);
	ExgRegisterDatatype(sysFileCMemo, exgRegTypeID, memoMIMEType, descP, 0);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);

	// Get application (icon name)
	resH = DmGetResource(gApplicationDbP, ainRsc, ainID );
	descP = MemHandleLock(resH);

	// Register for the get feature
	ExgRegisterDatatype(sysFileCMemo, exgRegSchemeID, exgGetScheme, descP, 0);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);
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
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
void * GetObjectPtr(uint16_t objectID, uint16_t formID)
{
	FormType *	formP;

	formP = FrmGetFormPtr(formID);
	return FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, objectID));
}

/***********************************************************************
 *
 * FUNCTION:    GetFocusObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to the field object, in
 *              the current form, that has the focus.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    pointer to a field object or NULL of there is no fucus
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
FieldType * GetFocusObjectPtr(uint16_t formID)
{
	FormType *	formP;
	uint16_t		focus;

	formP = FrmGetFormPtr(formID);
	focus = FrmGetFocus(formP);
	if (focus == noFocus)
		return (NULL);

	return (FrmGetObjectPtr(formP, focus));
}


/***********************************************************************
 *
 * FUNCTION:    SeekRecord
 *
 * DESCRIPTION: Given the index of a 'to do' record, this routine scans
 *              forwards or backwards for displayable 'to do' records.
 *
 * PARAMETERS:  indexP  - pointer to the index of a record to start from;
 *                        the index of the record sought is returned in
 *                        this parameter.
 *
 *              offset  - number of records to skip:
 *                        	0 - mean seek from the current record to the
 *                             next display record, if the current record is
 *                             a displayable record, its index is retuned.
 *                         1 - mean seek foreward, skipping one displayable
 *                             record
 *                        -1 - menas seek backwards, skipping one
 *                             displayable record
 *
 *
 * RETURNED:    false is return if a displayable record was not found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/5/95	Initial Revision
 *
 ***********************************************************************/
Boolean SeekRecord (uint32_t * indexP, int16_t offset, int16_t direction)
{
	status_t		err;

	TraceOutput(TL(appErrorClass, ">>>> SEEKING rec: %d, dir: %d, offset: %d",
		*indexP, direction, offset));

	err = DbCursorSetAbsolutePosition(gListViewCursorID, * indexP);
	if (err != errNone)
		return false;

	TraceOutput(TL(appErrorClass, ">>>> FOUND rec: %d",*indexP));
	err = DbCursorMove(gListViewCursorID, direction == dmSeekForward ? offset : (int32_t)-offset, dbFetchRelative);
	if (err != errNone)
		return false;

	DbCursorGetCurrentPosition(gListViewCursorID, indexP);

	TraceOutput(TL(appErrorClass, ">>>> MOVED TO rec: %d",*indexP));
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    ChangeCategory
 *
 * DESCRIPTION: This routine updates the global varibles that keep track
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
void ChangeCategory(CategoryID *categoriesP, uint32_t numCategories)
{
	uint32_t			size;
	CategoryID *	newCategoriesP = NULL;

	gSelectedRow = -1;
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

	gTopVisibleRecord = 1;
}

/***********************************************************************
 *
 * FUNCTION:    DrawMemoTitle
 *
 * DESCRIPTION: This routine draws the title of the specified memo.
 *
 * PARAMETERS:	 memo  - pointer to a memo
 *              x     - draw position
 *              y     - draw position
 *              width - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	04/18/95	Initial Revision
 *			roger	07/27/95	Combined both cases
 *			kwk	05/15/99	Use Int'l code for truncation of title.
 *
 ***********************************************************************/
void DrawMemoTitle (char * memo, int16_t x, int16_t y, int16_t width)
{
	char * ptr = StrChr (memo, linefeedChr);
	uint16_t titleLen = (ptr == NULL ? strlen (memo) : (uint16_t) (ptr - memo));
	if (FntWidthToOffset (memo, titleLen, width, NULL, NULL) == titleLen)
	{
		WinDrawChars (memo, titleLen, x, y);
	}
	else
	{
		int16_t titleWidth;
		titleLen = FntWidthToOffset (memo, titleLen, (uint16_t)(width - FntCharWidth(chrEllipsis)), NULL, &titleWidth);
		WinDrawChars (memo, titleLen, x, y);
		WinDrawChar(chrEllipsis, (Coord)(x + titleWidth), y);
	}
}

/***********************************************************************
 *
 * FUNCTION:    Search
 *
 * DESCRIPTION: This routine searchs the memo database for records
 *              containing the string passed.
 *
 * PARAMETERS:	findParams - text search parameter block
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		04/18/95	Initial Revision
 *			roger	07/26/95	converted to modern search mechanism
 *			kwk		05/15/99	Use TxtFindString, save match length in custom param.
 *			jmp		10/01/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *								MemoGetDatabase().
 *			jmp		10/21/99	Previous change caused bug #22965, but previous code
 *								caused yet another problem.  Fixed #22965 by using
 *								everyone else's way:  Call DmGetNextDatabaseByTypeCreator()
 *								first, then call DmOpenDatabase() if all is well.
 *
 ***********************************************************************/
static void Search (FindParamsPtr findParams)
{
	char *				headerStrP;
	MemHandle			headerStrH;
	RectangleType		r;
	Boolean				done;
	Boolean				match;
	DmOpenRef			dbP = NULL;
	DatabaseID			dbID;
	char *				textP = NULL;
	status_t			err;
	SearchLibType		search;
	CategoryID			cat = catIDAll;
	uint16_t			attr;

	PIMAppProfilingBegin("Search");

	findParams->more = false;

	MemSet(&search, sizeof(SearchLibType), 0);

	// Open Memo database
	if (gMemoDB)
	{
		// Already opened
		search.searchDB = gMemoDB;
	}
	else
	{
		MemoLoadPrefs(); // For SortID

		// Open it
		dbP = MemoDBOpenDatabase((uint16_t)findParams->dbAccesMode);
		search.searchDB = dbP;
		gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
	}

	// No DB
	if (!search.searchDB)
		PIMAppProfilingReturnVoid();

	// Get the DatabaseID
	DmGetOpenInfo(search.searchDB, &dbID, NULL, NULL, NULL);

	search.cursorID			= dbInvalidCursorID; // SearchLib to create the cursor
	search.schemaName		= kMemoDBSchemaName;
	search.orderBy			= kMemoDBAlphabeticSortString;
	search.cursorFlag		= dbCursorEnableCaching;
	search.startRowIndex	= findParams->recordNum + 1;
	search.startColumnIndex	= 0;
	search.numCols			= 0;
	search.colIDsP			= NULL;
	search.numCategories	= 1;
	search.catIDsP			= &cat;
	search.matchMode		= DbMatchAny;
	search.textToSearch		= findParams->strToFind;
	search.origTextToSearch	= findParams->strAsTyped;
	search.recordDirection	= dmSeekForward;
	search.columnDirection	= dmSeekForward;
	search.interruptible	= true;
	search.interruptCheck	= 500;	// Every 500 ms, check if interrupted

	if (SearchLibInit(&search) < errNone)
		goto Exit;

	// Display the heading line.
	headerStrH = DmGetResource(gApplicationDbP, strRsc, FindMemoHeaderStr);
	headerStrP = MemHandleLock(headerStrH);
	done = FindDrawHeader(findParams, headerStrP);
	MemHandleUnlock(headerStrH);
	DmReleaseResource(headerStrH);
	if (done)
		goto Exit;

	// Search the memos for the "find" string.
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
		if ((DbGetRowAttr(search.searchDB, search.foundRowID, &attr) >= errNone) && (attr & dbRecAttrSecret) && (gPrivateRecordVisualStatus != showPrivateRecords))
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
		err = MemoDBLoadRecord(search.searchDB, search.foundRowID, (void**)&textP, NULL);

		if (err >= errNone && textP)
		{
			// Display the title of the description.
			DrawMemoTitle(textP, (int16_t)(r.topLeft.x + 1), r.topLeft.y, (int16_t)(r.extent.x - 2));
			DbReleaseStorage(search.searchDB, textP);
			textP = NULL;
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
 * DESCRIPTION: This routine is an entry point of the memo application.
 *              It is generally called as the result of hitting a
 *              "Go to" button in the text search dialog.  The record
 *              identifies by the parameter block passed will be display,
 *              with the character range specified highlighted.
 *
 * PARAMETERS:	 goToParams  - parameter block that identifies the record to
 *                             display.
 *              launchingApp - true if the application is being launched.
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	06/06/95	Initial Revision
 *			roger	07/26/95	converted to modern search mechanism
 *			kwk	05/15/99	Use saved match length in matchCustom field, so
 *								that it works with Japanese.
 *
 ***********************************************************************/
static void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	uint32_t		recordNum;
	uint16_t		attr;
	uint32_t		uniqueID;
	EventType		event;
	CategoryID		catID = catIDAll;
	Boolean			inCategory;
	status_t		err;

	uniqueID = goToParams->recordID ;

	// Close main cursor in case current record is in cache
	if (gListViewCursorID != dbInvalidCursorID)
		DbCursorClose(gListViewCursorID);

	// Check that the record exists
	err = DbIsRowInCategory(gMemoDB, uniqueID, 1, &catID, DbMatchAny, &inCategory);

	if ((err < errNone) || (!inCategory))
	{
		// Reopen the main cursor
		MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
		FrmUIAlert(secGotoInvalidRecordAlert);
		FrmGotoForm(gApplicationDbP, ListView);
		return;
	}

	// Change the current category if necessary.
	if (!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll)
	{
		uint32_t		numCategories = 0;
		CategoryID *	categoriesP = NULL;

		err = DbGetCategory(gMemoDB, uniqueID, &numCategories, &categoriesP);
		ErrNonFatalDisplayIf(err < errNone, "Can't read category");

		inCategory = ((!gCurrentCategoriesP || *gCurrentCategoriesP == catIDUnfiled) && !numCategories);
		while (!inCategory && numCategories--)
		{
			inCategory = (gCurrentCategoriesP && *gCurrentCategoriesP == categoriesP[numCategories]);
		}

		if (!inCategory)
			ChangeCategory(&catID, 1);

		if (categoriesP)
			DbReleaseStorage(gMemoDB, categoriesP);
	}

	// Reopen the main cursor
	MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;

	// Get the position from uniqueID
	DbCursorMoveToRowID(gListViewCursorID, uniqueID);
	DbCursorGetCurrentPosition(gListViewCursorID, &recordNum);

	// Make the item the first item displayed.
	gTopVisibleRecord = recordNum;

	DbGetRowAttr(gMemoDB, uniqueID, &attr);

	if ((attr & dbRecAttrSecret) && gPrivateRecordVisualStatus == maskPrivateRecords)
	{
		FrmUIAlert(secGotoInvalidRecordAlert);
		FrmGotoForm(gApplicationDbP, ListView);
		return;
	}

	// If the application is already running, close all open forms.  This
	// may cause in the database record to be reordered, so we'll find the
	// records index by its unique id.
	if (! launchingApp && !gCheckingPassword)
	{
		FrmSaveAllForms();
		FrmCloseAllForms ();
		DbCursorGetPositionForRowID(gListViewCursorID, uniqueID, &recordNum);
	}

	// Send an event to goto a form and select the matching text.
	MemSet (&event, sizeof(EventType), 0);

	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = EditView;
	EvtAddEventToQueue (&event);

	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = EditView;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.recordID = uniqueID ;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchLen;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	EvtAddEventToQueue (&event);

	gCurrentView = EditView;
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
 *			gavin   11/9/99  Rewritten to use new ExgDoDialog function
 *
 ***********************************************************************/
static status_t CustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP)
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
	} else
	{
		// pretend as if user hit cancel, we won't accept the data
		askInfoP->result = exgAskCancel;
	}

	return err;
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
static ExgPassableSocketType *PrvExgMakePassableSocket(ExgSocketType *socketP)
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
 *			art	9/11/95		Initial Revision
 *
 ***********************************************************************/
static Boolean ApplicationHandleEvent (EventType * event)
{
	uint16_t formID;
	FormPtr frm;

	if (event->eType == frmLoadEvent)
	{
		Boolean	getInitialBounds = false;

		// Load the form resource.
		formID = event->data.frmLoad.formID;
		frm = FrmInitForm (gApplicationDbP, formID);
		FrmSetActiveForm (frm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time it receives an
		// event.
		switch (formID)
		{
		case ListView:
			getInitialBounds = true;
			FrmSetEventHandler (frm, ListViewHandleEvent);
			break;

		case EditView:
			getInitialBounds = true;
			EditViewInitLayout(frm);
			FrmSetEventHandler (frm, EditViewHandleEvent);
			break;

		case DetailsDialog:
			FrmSetEventHandler (frm, DetailsHandleEvent);
			break;

		case PreferencesDialog:
			FrmSetEventHandler (frm, PreferencesHandleEvent);
			break;

		default:
			break;
		}

		if (getInitialBounds)
			FrmGetFormInitialBounds(frm, &gCurrentWinBounds);

		return (true);
	}
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    EventLoop
 *
 * DESCRIPTION: This routine is the event loop for the Memo
 *              aplication.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void EventLoop (void)
{
	status_t error;
	EventType event;

	do
	{
		EvtGetEvent (&event, gEventLoopWaitTime);

		// Heavy change to not lost keyDown events while switching from ListView to EditView
		if (gEnqueueNewChar && event.eType == keyDownEvent)
		{
			TraceOutput(TL(appErrorClass, "Fake evtQueue (EventLoop): Enqueue char: %c", (char)event.data.keyDown.chr));

			// Buffer not full
			if (gNewCharBufferCount < kNewCharBufSize)
			{
				MemMove(&gNewCharBuffer[gNewCharBufferCount], &event, sizeof(EventType));
				gNewCharBufferCount++;
			}

			continue;
		}

		if (! SysHandleEvent (&event))

			if (! MenuHandleEvent (gCurrentMenu, &event, &error))

				if (! ApplicationHandleEvent (&event))

					FrmDispatchEvent (&event);

	}
	while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the Memo
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			art	1/3098	Removed sysAppLaunchCmdSaveData logic
 *			grant	6/25/99	In sysAppLaunchCmdInitDatabase, set the backup bit on the DB.
 *								In sysAppLaunchCmdExgReceiveData, update MemosInCategory.
 *			jmp	10/02/99	Made the support for the sysAppLaunchCmdExgReceiveData
 *								sysAppLaunchCmdExgAskUser launch codes more like their
 *								counterparts in Address, Databook, and ToDo.
 *			jmp	10/18/99	If the default "demo" database image doesn't exist, then
 *								create an empty database instead.
 *			jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from sysAppLaunchCmdExgAskUser
 *								since it was already being done in sysAppLaunchCmdExgReceiveData if
 *								the user affirmed sysAppLaunchCmdExgAskUser.  Also, in sysAppLaunchCmdExgReceiveData
 *								prevent call FrmSaveAllForms() if we're being call back through
 *								PhoneNumberLookup() as the two tasks are incompatible with each other.
 *
 ***********************************************************************/
uint32_t PilotMain (uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t 				error = errNone;
	DmOpenRef 				dbP;
	ExgSocketType *			socketP = NULL;
	ExgPassableSocketType * passableSocketP = NULL;
	uint32_t				cursorID = dbInvalidCursorID;

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Init( "Memo", cmd );
	#endif

	TraceOutput(TL(appErrorClass, "====> Memo entering PilotMain cmd=%d",cmd));

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = StartApplication ();
			if (error)
				break;

			FrmGotoForm (gApplicationDbP, gCurrentView);
			EventLoop ();
			StopApplication ();
			break;

		case sysAppLaunchCmdExgGetData:
			// handling get request - since we want to be fully launched, clone the socket and
			// AppSwitch to ourselves, otherwise we could respond here
			passableSocketP = PrvExgMakePassableSocket((ExgSocketPtr) cmdPBP);
			if (passableSocketP)
			{
				// we really only need the name and goToCreatorID fields, but we'll copy the whole struture here.
				error = PrvAppSwitchByCreator(sysFileCMemo, appLaunchCmdExgGetFullLaunch,
					(MemPtr) passableSocketP , sizeof(ExgPassableSocketType));
				if (error < errNone)
				{
					error = exgErrUserCancel; // return cancel error to let caller skip reading
					break;
				}
				else
					MemPtrFree(passableSocketP);
			}
			else error = exgErrAppError;
			break;

		case appLaunchCmdExgGetFullLaunch:
			// this is our custom full launch code
			// repair socket from transit
			socketP = PrvExgFixupPassableSocket((ExgPassableSocketType *)cmdPBP);

			// Test if the PIM is called for attach
			gAttachRequest = PrvCheckForAttachURL(socketP, gTransportPrefix, NULL);

			error = StartApplication ();
			if (error != errNone)
				break;

			FrmGotoForm (gApplicationDbP, ListView);
			EventLoop ();
			StopApplication ();

	     	ExgNotifyGoto(socketP, 0);
			break;

		case sysAppLaunchCmdFind:
			Search((FindParamsPtr)cmdPBP);
			break;

		case sysAppLaunchCmdGoTo:
			// This action code might be sent to the app when it's already running
			//  if the use hits the Find soft key next to the Graffiti area.
			if (launchFlags & sysAppLaunchFlagNewGlobals)
			{
				error = StartApplication ();
				if (error)
					break;

				GoToItem ((GoToParamsPtr) cmdPBP, true);

				EventLoop ();
				StopApplication ();
			}
			else
				GoToItem ((GoToParamsPtr) cmdPBP, false);
			break;

		case sysAppLaunchCmdSystemReset:
			// This launch code is sent after the system is reset.  We use this time
			// to create our default database.  If there is no default database image,
			// then we create an empty database.
			if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->hardReset)
				TraceOutput(TL(appErrorClass, ">>>> MEMO : sysAppLaunchCmdSystemReset: HARD RESET"));
			else
				TraceOutput(TL(appErrorClass, ">>>> MEMO : sysAppLaunchCmdSystemReset: SOFT RESET"));

			// Register to receive .txt and text/plain on database creation.
			RegisterData();

			if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
			{
				TraceOutput(TL(appErrorClass, ">>>> MEMO : sysAppLaunchCmdSystemReset: CREATE DEFAULT DB"));

				error = MemoDBCreateDefaultDatabase();
			}
			break;

		case sysAppLaunchCmdSyncNotify:
			SyncNotification ();
			break;

		case sysAppLaunchCmdExgAskUser:
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdExgAskUser"));

			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!gMemoDB &&!(launchFlags & sysAppLaunchFlagSubCall))
			{
				gMemoDB = dbP = MemoDBOpenDatabase(dmModeReadWrite);
			}
			else
				dbP = gMemoDB;

			CustomAcceptBeamDialog (dbP, (ExgAskParamPtr) cmdPBP);

			if (!(launchFlags & sysAppLaunchFlagSubCall) && dbP)
				error = DbCloseDatabase(dbP);

			break;

		case sysAppLaunchCmdExgReceiveData:
		{
			// Present the user with ui to perform a lookup and return a string
			// with information from the selected record.
			uint16_t numReceived = 0;
			uint32_t unused = 0;
			char * dataP = NULL;
			TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdExgReceiveData"));

			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				dbP = MemoDBOpenDatabase(dmModeReadWrite);
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID) ;
			}
			else
			{
				dbP = gMemoDB;
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID) ;
				// Delete the current record if it's empty
				if (gCurrentRecord != noRecordSelected)
				{
					DbGetColumnValue(dbP, gCurrentRecord, kMemoDBColumnID, 0, (void**)&dataP, &unused);
					if (!dataP || !(*dataP))
					{
						DbDeleteRow(dbP, gCurrentRecord);
						gCurrentRecord = noRecordSelected;
					}
					if (dataP)
						DbReleaseStorage(dbP, dataP);
				}
			}

			if (dbP != NULL)
			{
				error = MemoReceiveData(dbP, (ExgSocketPtr) cmdPBP, &numReceived);

				// We may have just added some memos to the current category.
				// If the app is currently running, update MemosInCategory to reflect this.
				if (launchFlags & sysAppLaunchFlagSubCall)
				{
					// restore global cursor settings
					MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
					if (gCurrentRecord != noRecordSelected)
					{
						if (DbCursorGetRowIDForPosition(gListViewCursorID, gCurrentRecord, &unused) < errNone)
							gCurrentRecord = noRecordSelected;	// Can't happen, but...
					}
				}
				else
				{
					if (gListViewCursorID != dbInvalidCursorID)
					{
						DbCursorClose(gListViewCursorID);
						gListViewCursorID = dbInvalidCursorID;
					}
					DbCloseDatabase(dbP);
					dbP = NULL;
				}
			}
			else
				error = exgErrAppError;
			break;
		}

		case sysAppLaunchCmdExgPreview:
			MemoTransferPreview((ExgPreviewInfoType *)cmdPBP);
			break;

		case sysAppLaunchCmdInitDatabase:
			// This action code is sent by the DesktopLink server when it create
			// a new database.  We will initializes the new database.
			// Set the backup bit.  This is to aid syncs with non Palm software.
			MemoDBSetBackupBit(((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
			break;

		case sysAppLaunchCmdExportRecordGetCount:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
				dbP = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			}
			else
				dbP = gMemoDB;

			if (dbP != NULL)
			{
				// Assign the number of records
				MemoDBOpenIOCursor(&dbP, &cursorID);
				*((uint32_t*)cmdPBP) = DbCursorGetRowCount(cursorID);
				DbCursorClose(cursorID);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
					DbCloseDatabase(dbP);
			}
			break;

		case sysAppLaunchCmdExportRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
				dbP = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
			}
			else
				dbP = gMemoDB;

			if (dbP != NULL)
			{
				error = MemoExportData(dbP, (ImportExportRecordParamsPtr)cmdPBP);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
					DbCloseDatabase(dbP);
			}
			break;

		case sysAppLaunchCmdImportRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
				dbP = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID);
			}
			else
			{
				dbP = gMemoDB;
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID);
			}

			if (dbP != NULL)
			{
				error = MemoImportData(dbP, (ImportExportRecordParamsPtr)cmdPBP);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
				{
					if (gListViewCursorID != dbInvalidCursorID)
					{
						DbCursorClose(gListViewCursorID);
						gListViewCursorID = dbInvalidCursorID;
					}
					DbCloseDatabase(dbP);
					dbP = NULL;
				}
				else
					// restore global cursor settings
					MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
			}
			break;

		case sysAppLaunchCmdDeleteRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
				dbP = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID);
			}
			else
			{
				dbP = gMemoDB;
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID);
			}

			if (dbP != NULL && error >= errNone)
			{
				MemoDeleteData(dbP, (ImportExportRecordParamsPtr)cmdPBP);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
				{
					if (gListViewCursorID != dbInvalidCursorID)
					{
						DbCursorClose(gListViewCursorID);
						gListViewCursorID = dbInvalidCursorID;
					}
					DbCloseDatabase(dbP);
					dbP = NULL;
				}
				else
					// restore global cursor settings
					MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
			}
			break;

		case sysAppLaunchCmdMoveRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
				dbP = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
										 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID);
			}
			else
			{
				dbP = gMemoDB;
				MemoDBOpenIOCursor(&dbP, &gListViewCursorID);
			}

			if (dbP != NULL && error >= errNone)
			{
				MemoMoveData(dbP, (ImportExportRecordParamsPtr)cmdPBP);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
				{
					if (gListViewCursorID != dbInvalidCursorID)
					{
						DbCursorClose(gListViewCursorID);
						gListViewCursorID = dbInvalidCursorID;
					}
					DbCloseDatabase(dbP);
					dbP = NULL;
				}
				else
					// restore global cursor settings
					MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
			}
			break;

		case sysLaunchCmdInitialize:
			// Get application dbP
			error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gApplicationDbP);
			break;
	}

//ExitError:

	TraceOutput(TL(appErrorClass, "<==== Memo exiting PilotMain cmd=%d errCode=%d",cmd,error));

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Exit();
	#endif

	return error;
}

/******************************************************************************
 *
 * FUNCTION:		UtilsFrmInvalidateWindow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	formID - the ID of the form to be invalidate
 *
 * RETURNED:    	nothing.
 *
 * REVISION HISTORY:
 *
 *	TEs	03/18/02	Initial Version
 *
 ******************************************************************************/
void UtilsFrmInvalidateWindow(uint16_t formID)
{
	WinHandle	winH;
	FormType *	formP;

	if ((formP = FrmGetFormPtr(formID)) == NULL)
		return;

	if ((winH = FrmGetWindowHandle(formP)) == NULL)
		return;

	WinInvalidateWindow(winH);
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
