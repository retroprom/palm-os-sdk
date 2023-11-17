/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Address.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *   This is the Address Book application's main module.  This module
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

#include <PalmTypesCompatibility.h>
#include <Find.h>
#include <NotifyMgr.h>
#include <ErrorMgr.h>
#include <TimeMgr.h>
#include <KeyMgr.h>
#include <Menu.h>
#include <UIResources.h>
#include <SysEvtMgr.h>
#include <PenInputMgr.h>
#include <FeatureMgr.h>
#include <TraceMgr.h>
#include <AppMgr.h>
#include <ExgLocalLib.h>
#include <CmnLaunchCodes.h>
#include <Loader.h>
#include <SystemResources.h>
#include <DataMgr.h>
#include <Chars.h>
#include <TextMgr.h>
#include <SystemMgr.h>
#include <Preferences.h>
#include <SearchLib.h>
#include <FormLayout.h>
#include <StringMgr.h>
#include <string.h>
#include <TextServicesMgr.h>
#include <SysUtils.h>

#include "Address.h"
#include "AddressList.h"
#include "AddressView.h"
#include "AddressEdit.h"
#include "AddressNote.h"
#include "AddressTools.h"
#include "AddressDetails.h"
#include "AddressCustom.h"
#include "AddressDialList.h"
#include "AddressDBSchema.h"

#include "AddressTransfer.h"
#include "AddressLookup.h"
#include "AddressPrefs.h"

#include "AddressAutoFill.h"
#include "AddressRsc.h"
#include "AddressTab.h"

#include "AddressIdLookup.h"


/***********************************************************************
 *
 *	Local declarations
 *
 ***********************************************************************/

// exg socket structure for appLaunchCmdExgGetFullLaunch
typedef struct
{
	ExgSocketType	socket;
	char			name[exgMaxTypeLength + 1];
	char			description[exgMaxTypeLength + 1];
	char			type[exgMaxTypeLength + 1];
} ExgPassableSocketType;

/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Time to depress the app's button to send a business card
#define kAppButtonPushTimeout			SysTimeInSecs(1)

// When the system locale changes, the existing DB is temporarily renamed
#define kSavedAddrDBName				kAddrDBName "_"

// Used to store record ID of address imported by the PhonePad
#define kRecordIDToGoFtrID				0x0100

/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

uint32_t           			gListViewSelectThisRowID = dbInvalidRowID;

DmOpenRef					gAddrDB = NULL;
uint32_t					gCursorID = dbInvalidCursorID;
DatabaseID					gAddrDatabaseID;
privateRecordViewEnum		gPrivateRecordVisualStatus;
char						gCategoryName[kCategoryLabelLength + 1];
uint32_t        			gTopVisibleRowIndex = kFirstRowIndex;
uint32_t					gCurrentRowID = dbInvalidRowID;

LmLocaleType				gFormatLocale;
LmLocaleType				gSystemLocale;

// Set by RecordView when the user taps within the display area
// to edit. It's used by the EditView to give the focus to the
// corresponding field.
uint32_t					gTappedColumnID;

int16_t						gOrderByIndex = 0;
int16_t						gOrderByCount = 0;
char						gCurrentOrderByStr[kMaxOrderBySQLStrSize];
uint32_t					gCurrentOrderByType = kOrderByNameType;

// These are used for controlling the display of the duplicated address records.
uint16_t					gNumCharsToHilite = 0;

char *						gUnnamedRecordStringP = NULL;
// List view : phone label width. This value is computed at application start to
// increase list displaying.
int16_t 					gPhoneLabelWidth = 0;

// The following global variable are saved to a state file.
CategoryID *				gCurrentCategoriesP;
uint32_t					gCurrentNumCategories;
Boolean						gEnableTapDialing = true;	// tap dialing is enabled by default and will be set to false if there is no dialer installed.
Boolean						gSaveBackup = true;
Boolean						gRememberLastCategory = false;
FontID						gNoteFont = stdFont;
FontID						gAddrListFont = stdFont;
FontID						gAddrRecordFont = largeBoldFont;
FontID						gAddrEditFont = largeBoldFont;
uint32_t					gBusinessCardRowID = dbInvalidRowID;

uint8_t						gTransfertMode = kPrefTransfertAllRecord;
Boolean						gTransfertNote[4] = {false, false, false, false};

Boolean						gDialerPresentChecked;
Boolean						gDialerPresent;

// Active Input Area support
RectangleType				gCurrentWinBounds;		// Current window bounds. Set at frmLoadEvent

// For sublaunch main list
Boolean						gCalledFromApp = false;
LocalID						gCallerAppDbID = 0;

// For attach sublaunch
Boolean						gAttachRequest = false;
char						gTransportPrefix[kAttachTransportPrefixMaxSize];

// Application Database
DmOpenRef					gApplicationDbP = 0;

// EvtGetEvent timeout param
int32_t						gEvtGetEventTimeout = evtWaitForever;

// Device has FEP installed
Boolean						gDeviceHasFEP = false;

// Template globals. To avoid to recompute the resource ID each time.
DmResourceID				gFFNLayoutResID = 0;
DmResourceID				gDisplayNameLayoutResID = 0;
DmResourceID				gAddressLayoutResID = 0;
DmResourceID				gFFNColumnListResID = 0;
DmResourceID				gDisplayNameColumnListResID = 0;

/***********************************************************************
 *
 *   Static variables
 *
 ***********************************************************************/

// For business card beaming
static wchar32_t			sAppButtonPushed = nullChr;
static uint16_t				sAppButtonPushedModifiers = 0;
static Boolean				sBusinessCardSentForThisButtonPress = false;
static uint64_t				sTickAppButtonPushed;

// Used by import/export launch codes to reopen/restore the cursor with all categories
static CategoryID *	sOldCategoriesP = NULL;
static uint32_t		sOldNumCategories = 0;
static uint32_t		sOldRowID;
static Boolean		sCursorInfoSaved = false;

/***********************************************************************
 *
 * FUNCTION:    PrvAppHandleKeyDown
 *
 * DESCRIPTION: Handle the key being down.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed on
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	10/22/97	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvAppHandleKeyDown (EventType * event)
{
	// Check if a button being held down is released
	if (sTickAppButtonPushed != 0)
	{
		// This is the case when the button is let up
		if ((KeyCurrentState() & (keyBitHard1 | keyBitHard2 | keyBitHard3 | keyBitHard4)) == 0)
		{
			if (sBusinessCardSentForThisButtonPress)
			{
				sBusinessCardSentForThisButtonPress = false;

				sTickAppButtonPushed = 0;

				// Allow the masked off key to now send keyDownEvents.
				KeySetMask(keyBitsAll);
			}
			else if (event->eType == nilEvent)
			{
				// Send the keyDownEvent to the app.  It was stripped out
				// before but now it can be sent over the nullEvent.  It
				// may be nullChr from when the app was launched.  In that case
				// we don't need to send the app's key because the work expected,
				// which was switching to this app, has already been done.
				if (sAppButtonPushed != nullChr)
				{
					event->eType = keyDownEvent;
					event->data.keyDown.chr = sAppButtonPushed;
					event->data.keyDown.modifiers = sAppButtonPushedModifiers;
				}

				sTickAppButtonPushed = 0;

				// Allow the masked off key to now send keyDownEvents.
				KeySetMask(keyBitsAll);
			}
		}
		// This is the case when the button is depresed long enough to send the business card
		else if (sTickAppButtonPushed + kAppButtonPushTimeout <= TimGetTicks() && !sBusinessCardSentForThisButtonPress)
		{
			sBusinessCardSentForThisButtonPress = true;
			ToolsAddrBeamBusinessCard();
		}
	}
	else if (event->eType == keyDownEvent)
	{
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr) &&
			!(event->data.keyDown.modifiers & autoRepeatKeyMask))
		{
			// Remember which hard key is mapped to the Address Book
			// because it may need to be sent later.
			sAppButtonPushed = event->data.keyDown.chr;
			sAppButtonPushedModifiers = event->data.keyDown.modifiers;

			sTickAppButtonPushed = TimGetTicks();

			// Mask off the key to avoid repeat keys causing clicking sounds
			KeySetMask(~(KeyCurrentState() & (keyBitHard1 | keyBitHard2 | keyBitHard3 | keyBitHard4)));

			// Don't process the key
			return true;
		}
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art    	09/11/95   	Initial Revision
 * jmp		09/17/99   	Use NewNoteView instead of NoteView.
 * aro		06/22/00	Add AddrListDialog
 * ppl		02/06/02	add switch
 *
 ***********************************************************************/
static Boolean PrvAppHandleEvent (EventType * event)
{
	uint16_t	formId;
	FormType *	formP;
	Boolean		handled = false;

 	switch (event->eType)
 	{
 		case frmLoadEvent:
		{
			Boolean	getInitialBounds = false;

			// Load the form resource.
			formId = event->data.frmLoad.formID;
			formP = FrmInitForm(gApplicationDbP, formId);
			FrmSetActiveForm(formP);
			handled = true;

			// Set the event handler for the form.  The handler of the currently
			// active form is called by FrmHandleEvent each time is receives an
			// event.
			switch (formId)
			{
				case ListView:
					getInitialBounds = true;
					FrmSetEventHandler(formP, ListHandleEvent);
					break;

				case RecordView:
					getInitialBounds = true;
					FrmSetEventHandler(formP, ViewHandleEvent);
					break;

				case EditView:
					getInitialBounds = true;
					FrmSetEventHandler(formP, EditHandleEvent);
					break;

				case DetailsDialog:
					FrmSetEventHandler(formP, DetailsHandleEvent);
					break;

				case CustomEditDialog:
					FrmSetEventHandler(formP, CustomEditHandleEvent);
					break;

				case PreferencesDialog:
					FrmSetEventHandler(formP, PrefsHandleEvent);
					break;

				case DialListDialog:
					FrmSetEventHandler(formP, DialListHandleEvent);
					break;

				default:
					ErrNonFatalDisplay("Invalid Form Load Event");
					getInitialBounds = true;
					FrmGotoForm(gApplicationDbP, ListView);
					break;
			}

			if (getInitialBounds)
				FrmGetFormInitialBounds(formP, &gCurrentWinBounds);
		}
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the Address Book
 *              aplication.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * roger	06/05/95	Initial Revision
 *
 ***********************************************************************/
//Boolean SysHandleEvent(EventType * eventP);

static void PrvAppEventLoop (void)
{
	status_t	error;
	EventType	event;

	do
	{
		EvtGetEvent (&event, (sTickAppButtonPushed == 0) ? gEvtGetEventTimeout : 2);

		// Tells the EditTable to release the focus and save the default phone column ID
		// for new records. This avoids a fatal alert while searching the Address DB.
		if (event.eType == keyDownEvent && EvtKeydownIsVirtual(&event) && event.data.keyDown.chr == vchrFind)
			PrvPostUserEvent(kEditViewReleaseFocusEvent);

		if (! SysHandleEvent (&event))
			if (! PrvAppHandleKeyDown (&event))
				if (! MenuHandleEvent (0, &event, &error))
					if (! PrvAppHandleEvent (&event))
						FrmDispatchEvent (&event);
	}
	while (event.eType != appStopEvent);
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppSearch
 *
 * DESCRIPTION: This routine searches the the address database for records
 *              contains the string passed.
 *
 * PARAMETERS:  findParams
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * art		06/05/95	Created by Art Lamb.
 * jmp		10/21/99	Changed params to findParams to match other
 *						routines like this one.
 * kwk		11/30/00	Use TxtFindString to avoid trap dispatch from FindStrInStr.
 *						Save returned match length as appCustom in FindSaveMatch.
 *
 ***********************************************************************/
static void PrvAppSearch(FindParamsType *findParamsP)
{
	Boolean				done;
	Boolean				match;
	char *				header;
	MemHandle			headerStringH;
	DatabaseID			dbID;
	RectangleType		r;
	SearchLibType		search;
	CategoryID			cat = catIDAll;
	uint16_t			attr;
	AddressTabColumnPropertiesType * colTabP;

	findParamsP->more = false;

	memset(&search, 0, sizeof(SearchLibType));

	// Get the DatabaseID
	DmGetOpenInfo(gAddrDB, &dbID, NULL, NULL, NULL);

	search.searchDB			= gAddrDB;
	search.cursorID			= dbInvalidCursorID; // SearchLib to create the cursor
	search.schemaName		= kAddressDefaultTableName;
	search.orderBy			= gCurrentOrderByStr;
	search.cursorFlag		= dbCursorEnableCaching;
	search.startRowIndex	= findParamsP->recordNum + 1; // +1 to begin search at index 1 or after the last found entry
	search.startColumnIndex	= 0;
	search.numCols			= 0;
	search.colIDsP			= NULL;
	search.numCategories	= 1;
	search.catIDsP			= &cat;
	search.matchMode		= DbMatchAny;
	search.textToSearch		= findParamsP->strToFind;
	search.origTextToSearch	= findParamsP->strAsTyped;
	search.recordDirection	= dmSeekForward;
	search.columnDirection	= dmSeekForward;
	search.interruptible	= true;
	search.interruptCheck	= 500;	// Every 500 ms, check if interrupted

	if (SearchLibInit(&search) < errNone)
		goto Exit;

	// Display the heading line.
	headerStringH = DmGetResource(gApplicationDbP, strRsc, FindAddrHeaderStr);
	header = DmHandleLock(headerStringH);
	done = FindDrawHeader(findParamsP, header);
	DmHandleUnlock(headerStringH);
	DmReleaseResource(headerStringH);

	if (done)
		goto Exit;

	// PrvAppSearch the description and note fields for the "find" string.
	while (true)
	{
		match = SearchLibSearch(&search);

		if (!match)
		{
			findParamsP->more = search.interrupted;
			break;
		}

		// Verify the secret attribute.
		// This could happen when the application is already launched. That cost less than creating another cursor.
		if ((DbGetRowAttr(gAddrDB, search.foundRowID, &attr) >= errNone) && (attr & dbRecAttrSecret) && (gPrivateRecordVisualStatus != showPrivateRecords))
		{
			search.resumePolicy = kSearchResumeChangeRecord;
			continue;
		}

		// Skip non visible rows (but the note)
		colTabP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, search.foundColID);
		if (colTabP == NULL || (!colTabP->visible && colTabP->columnId != kAddrColumnIDNote))
		{
			search.resumePolicy = kSearchResumeChangeColumn;
			continue;
		}

		// Add the match to the find paramter block, if there is no room to
		// display the match the following function will return true.
		done = FindSaveMatch(findParamsP, search.foundRowIndex, search.foundRowID, search.matchPos, search.matchLength, search.foundColID, 0, dbID);

		if (done)
		{
			findParamsP->more = true;
			break;
		}

		// Get the bounds of the region where we will draw the results.
		FindGetLineBounds(findParamsP, &r);

		// Display the title of the description.
		FntSetFont(stdFont);
		ToolsDrawRecordNameAndPhoneNumber(search.foundRowID, &r, stdFont, false, false);

		findParamsP->lineNumber++;
		search.resumePolicy = kSearchResumeChangeRecord;
	}

Exit:
	SearchLibRelease(&search);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppGoToItem
 *
 * DESCRIPTION: This routine is a entry point of this application.
 *              It is generally call as the result of hiting of
 *              "Go to" button in the text PrvAppSearch dialog.
 *
 * PARAMETERS:  recordNum -
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * rsf		07/12/95	Created by Roger Flores.
 * jmp		09/17/99	Use NewNoteView instead of NoteView.
 * kwk		11/30/00	Set frmGoto.matchLen to be matchCustom, since we
 *						pass the match length returned by TxtFindString to
 *						FindSaveMatch in the appCustom parameter.
 *
 ***********************************************************************/
static void PrvAppGoToItem(GoToParamsType *goToParamsP, Boolean closeAllForms)
{
	CategoryID	catID = catIDAll;
	uint16_t	attr;
	Boolean		verifyPW = false;
	status_t	err;

	// Change the current category if necessary.
	if (!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll)
		ToolsChangeCategory(&catID, 1);

	// If the application is already running, close all the open forms.  If
	// the current record is blank, then it will be deleted, so we'll
	// the record's unique id to find the record index again, after all
	// the forms are closed.
	if (closeAllForms)
	{
		FrmSaveAllForms();
		FrmCloseAllForms();
	}

	// Set global variables that keep track of the currently record.
	gCurrentRowID = goToParamsP->recordID;
	err = DbCursorMoveToRowID(gCursorID, gCurrentRowID);
	ErrNonFatalDisplayIf(err < errNone, "DbCursorMoveToRowID failed");

	if (DbGetRowAttr(gAddrDB, gCursorID, &attr) >= errNone)
		verifyPW = (Boolean) ((attr & dbRecAttrSecret) ? AddressDBVerifyPassword() : true);

	if (verifyPW)
		RecordViewGotoItem(goToParamsP);
	else
		FrmGotoForm(gApplicationDbP, ListView);
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppInitUnnamedString
 *
 * DESCRIPTION: Load the "Unnamed" string into memory at launch.
 *
 * PARAMETERS:	dbRef		- Applicattion database ref
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		11/18/02	Initial revision
 *
 ***********************************************************************/
void PrvAppInitUnnamedString(void)
{
	if (!gUnnamedRecordStringP)
		gUnnamedRecordStringP = ToolsCopyStringResource(UnnamedRecordStr);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppReleaseUnnamedString
 *
 * DESCRIPTION: Release the "Unnamed" string previously loaded.
 *
 * PARAMETERS:	None.
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		11/18/02	Initial revision
 *
 ***********************************************************************/
void PrvAppReleaseUnnamedString(void)
{
	if (gUnnamedRecordStringP)
	{
		MemPtrFree(gUnnamedRecordStringP);
		gUnnamedRecordStringP = NULL;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppOpenDatabase
 *
 * DESCRIPTION: Load the Application info and open the database.
 *
 * PARAMETERS:	dbP		- Applicattion data database ref
 *				mode	- how to open the database
 *
 * RETURNED:	a negative value if an error occurs
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		10/31/02	Initial revision
 *
 ***********************************************************************/
status_t PrvAppOpenDatabase(uint16_t mode)
{
	status_t	err;

	// Find the application's data file.  If it doesn't exist create it.
	err = AddressDBOpenDatabase(gApplicationDbP, mode, &gAddrDB);

	if (err >= errNone)
		err = AddressDBOpenCursor();

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppLaunchCmdDatabaseInit
 *
 * DESCRIPTION: Initialize an empty database.
 *
 * PARAMETERS:	dbP - pointer to database opened for read & write
 *
 * RETURNED:	true if successful
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	10/19/95	Initial Revision
 * LFe		10/23/02	Update to manage new status_t type definition
 *
 ***********************************************************************/
static Boolean PrvAppLaunchCmdDatabaseInit(DmOpenRef dbP)
{
	DatabaseID	dbID;

	if (!dbP)
		return false;

	// Set the backup bit.  This is to aid syncs with non Palm software.
	if (DmGetOpenInfo(dbP, &dbID, NULL, NULL, NULL) >= errNone)
		AddressDBSetDatabaseAttributes(dbID, dmHdrAttrBackup);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvAppStop
 *
 * DESCRIPTION: This routine close the application's database
 *              and save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name   	Date		Description
 * ----   	--------    -----------
 * art   	06/05/95	Initial Revision
 * ppl		02/13/02	Add Active Input Area Support :
 *						a feature to remember the AIA state
 *
 ***********************************************************************/
static void PrvAppStop(Boolean closeForm)
{
	// Write the preferences / saved-state information.
	PrefsSave();

	if (closeForm)
	{
		// Send a frmSave event to all the open forms.
		FrmSaveAllForms ();

		// Close all the open forms.
		FrmCloseAllForms ();
	}

	PrvAppReleaseUnnamedString();

	PrefsSaveCurrentCategories(gCurrentNumCategories, gCurrentCategoriesP);

	if (gCurrentCategoriesP)
	{
		MemPtrFree(gCurrentCategoriesP);
		gCurrentCategoriesP = NULL;
	}

	AddressTabReleaseSchemaInfoFromDB(&gBookInfo, gAddrDB);
	AddressDBCloseDatabase();
	AddressDBDeletePreviewDatabase();

	if (SysAreWeUIThread())
		KeySetMask(keyBitsAll);
}

/***********************************************************************
 *
 * FUNCTION:     PrvAppStart
 *
 * DESCRIPTION:  This routine opens the application's resource file and
 *               database.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     status_t - standard error code
 *
 * REVISION HISTORY:
 *
 * Name		  Date		Description
 * -----	--------	-----------
 * art		06/05/95	Initial Revision
 * vmk		12/12/97	Get amplitude for scroll sound
 * BGT		01/08/98	Use AddressLoadPrefs to load and fix up the
 *						application preferences
 * grant	04/06/99	Moved code to set backup bit into SetDBAttrBits.
 * jmp		10/01/99	Call new AddrGetDataBase() to create database
 *						if it doesn't already exist.
 * ppl		02/13/02	Add Active Input Area Support :
 *						a feature to remember the AIA state
 * LFe		10/23/02	Update to manage new status_t type definition
 *
 ***********************************************************************/
static status_t PrvAppStart(uint16_t dbAccesMode, uint16_t cmd, Boolean forceCatIDAll)
{
	status_t	err;
	Boolean		savedCatPref = false;
	uint32_t	flags;

	// Determime if secret records should be shown.
	gPrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);

	if (!dbAccesMode)
		dbAccesMode = dmModeReadWrite;

	if (gPrivateRecordVisualStatus != hidePrivateRecords)
		dbAccesMode |= dmModeShowSecret;

	// Load the application preferences and fix them up if need be.	(BGT)
	PrefsLoad(gApplicationDbP);

	// Check the dialing abilities
	// Only the first call is long and further called are fast
	// So it's better to do it the first time the form is drawn
	if ((cmd != AddressBookIdLookupLaunch)
		&& (cmd != sysAppLaunchCmdExgAskUser)
		&& (cmd != sysAppLaunchCmdExgGetData)
		&& (cmd != sysAppLaunchCmdExgReceiveData)
		&& (cmd != sysAppLaunchCmdExgPreview)
		&& (cmd != sysAppLaunchCmdFind))
			gEnableTapDialing = ToolsIsDialerPresent();

	// Overwrite the pref gRememberLastCategory if we want to load the All category
	// if gRememberLastCategory is set to false, "All" will be used as default.
	// This will be mandatory for search/idlookup.
	if (forceCatIDAll)
	{
		savedCatPref = gRememberLastCategory;
		gRememberLastCategory = false;
	}

	// Current categories
	PrefsLoadCurrentCategories(&gCurrentNumCategories, &gCurrentCategoriesP);

	// Restore the preference
	if (forceCatIDAll)
		gRememberLastCategory = savedCatPref;

	// Get the format locale set by the user in the Format preference dialog.
	LmGetFormatsLocale(&gFormatLocale);
	LmGetSystemLocale(&gSystemLocale);

	// Find the application's data file.  If it doesn't exist create it.
	if (!gAddrDB && (err = PrvAppOpenDatabase(dbAccesMode)) < errNone)
		return err;

	PrvAppInitUnnamedString();

	if (cmd != AddressBookIdLookupLaunch)
		ToolsComputePhoneLabelWidth();

	// For a same locale, some fields can have different names.
	// (e.g.: in enUS locale, zipcode is "Postal Code" for Canada and "Post Code" for UK).
	// So we need to update the schema database to look properly for the system selected
	// country, if it doesn't already the same.
	// Perform this check BEFORE initializing the Tabs.
	// Only check the country if the DB is opened with write access because it can be called from a Find
	// which opens the DB in read only mode.
	if ((dbAccesMode & dmModeWrite) == dmModeWrite && (err = AddressDBCheckCountry(gAddrDB)) < errNone)
		return err;

	// Init the internal book structure with the schema definition.
	if ((err = AddressTabInitSchemaInfoFromDB(&gBookInfo, gAddrDB)) < errNone)
		return err;

	// Start watching the button pressed to get into this app.  If it's held down
	// long enough then we need to send the business card.
	sTickAppButtonPushed = TimGetTicks();

	// Mask off the key to avoid repeat keys causing clicking sounds
	if (SysAreWeUIThread())
		KeySetMask(~(KeyCurrentState() & (keyBitHard1 | keyBitHard2 | keyBitHard3 | keyBitHard4)));

	// Device Has FEP?
	gDeviceHasFEP = (Boolean) ((FtrGet(tsmFtrCreator, tsmFtrNumFlags, &flags) >= errNone) ?
					((flags & tsmFtrFlagsHasFep) == tsmFtrFlagsHasFep) :
					false);

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

	if (!socketP || !attachTransportScheme)
		return false;

	*attachTransportScheme = 0;

	if (socketP->name)
	{
		endP = StrChr(socketP->name, chrColon);

		// Copy including colon
		if (endP)
			StrNCopy(attachTransportScheme,socketP->name, max((endP-socketP->name) + 1, kAttachTransportPrefixMaxSize - 1));
	}
	if (*attachTransportScheme == 0)
		StrCopy(attachTransportScheme, exgLocalPrefix); // assume local by default

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

	if (!newSockP)
		return NULL;

	memset(newSockP,0, sizeof(ExgPassableSocketType)); // fill  null
	// copy the socket data
	memmove(&newSockP->socket, socketP, sizeof(ExgSocketType));
	newSockP->socket.socketRef = 0; // can't duplicate socketRef...
	newSockP->socket.componentIndex  = 0; // or this

	// now copy the string data from the source to the buffer after the new socket
	// and make the new socket point to those strings
	if (socketP->name)
		StrLCopy(&newSockP->name[0], socketP->name, exgMaxTypeLength + 1);

	newSockP->socket.name = &newSockP->name[0];

	if (socketP->description)
		StrLCopy(&newSockP->description[0],socketP->description, exgMaxTypeLength + 1);

	newSockP->socket.description = &newSockP->description[0];

	if (socketP->type)
		StrLCopy(&newSockP->type[0],socketP->type, exgMaxTypeLength + 1);

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
 * FUNCTION:    PrvImpExpReopenCursor
 *
 * DESCRIPTION: Reopens the current global cursor including all cateogries
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if correctly saved. false if already saved (no reentrance)
 *
 ***********************************************************************/
static Boolean PrvImpExpReopenCursor(void)
{
    CategoryID	categoryIDAll = catIDAll;

	// No reentrance
	if (sCursorInfoSaved)
	{
        ErrNonFatalDisplay("AddressBook already sublaucnhed for import/export. Can't save categories twice");
		return false;
	}

	sOldNumCategories = gCurrentNumCategories;

	if (sOldNumCategories)
	{
		uint32_t	size = sizeof(uint32_t) * sOldNumCategories;

		sOldCategoriesP = MemPtrNew(size);
		ErrFatalDisplayIf(!sOldCategoriesP, "Out of memory!");

		memmove(sOldCategoriesP, gCurrentCategoriesP, size);
	}

	sOldRowID = gCurrentRowID;
	gCurrentRowID = dbInvalidRowID;

	// The cursor will be re-opened by this call
	ToolsChangeCategory(&categoryIDAll, 1);

	sCursorInfoSaved = true;

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvImpExpRestoreCursor
 *
 * DESCRIPTION: Restore old cursor categories (prior to PrvImpExpReopenCursor call)
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 ***********************************************************************/
static void PrvImpExpRestoreCursor(void)
{
	if (!sCursorInfoSaved)
		return;

	// The cursor will be re-opened by this call
	ToolsChangeCategory(sOldCategoriesP, sOldNumCategories);

	if (sOldCategoriesP)
	{
		MemPtrFree(sOldCategoriesP);
		sOldCategoriesP = NULL;
	}

	if (sOldRowID != dbInvalidRowID)
		DbCursorMoveToRowID(gCursorID, sOldRowID);

	gCurrentRowID = sOldRowID;

	sCursorInfoSaved = false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvCheckDBSchemaLocale
 *
 * DESCRIPTION: Verify the current overlay locale and rebuild the
 *				database schema if necessary
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		02/20/04	Initial Revision
 *
 ***********************************************************************/
void PrvCheckDBSchemaLocale(void)
{
	DatabaseID			addrDbID = 0;
	DatabaseID			tmpDbID;
	DmOpenRef			addrDbP = 0;
	DmOpenRef			tmpDbP = 0;
	DmDatabaseInfoType	dbInfo;
	status_t			err;
	uint32_t			columnID;

	uint32_t			colIndex;
	uint32_t			propIndex;

	uint32_t			numBytes;
	void *				dataP;

	uint32_t					newNumColumns = 0;
	DbSchemaColumnDefnType	*	newColumnsP = NULL;
	uint32_t					numProps = 0;
	DbColumnPropertyValueType *	propValuesP = NULL;

	LmLocaleType		savedLocale;
	LmLocaleType		ovlyLocale;

	TraceOutput(TL(appErrorClass, "PrvCheckDBSchemaLocale"));

	AddressDBGetSavedOverlayLocale(&savedLocale);
	DmGetOverlayLocale(&ovlyLocale);

	// No change. Nothing to do
	if (savedLocale.country == ovlyLocale.country && savedLocale.language == ovlyLocale.language)
		return;

	// AddressDBSaveOverlayLocale returning false means that the corresponding localized DB doesn't exist
	if (!AddressDBSaveOverlayLocale(&ovlyLocale))
		return;

	// No existing DB
	if ((addrDbID = DmFindDatabase(kAddrDBName, sysFileCAddress, dmFindSchemaDB, NULL)) == 0)
		return;

	// First rename the current address DB
	memset(&dbInfo, 0, sizeof(dbInfo));
	dbInfo.size = sizeof(dbInfo);
	dbInfo.pName = kSavedAddrDBName;
	err = DmSetDatabaseInfo(addrDbID, &dbInfo);
	if (err < errNone)
		return;

	// Open it
	addrDbP = DbOpenDatabaseByName(sysFileCAddress, kSavedAddrDBName, dmModeReadWrite, dbShareRead);
	if (!addrDbP)
		goto Exit;

    // Create a new database. It will contain the new localized schema
	err = AddressDBCreateDefaultDatabase(gApplicationDbP);
	if (err < errNone)
		goto Exit;

	// Open it
	tmpDbP = DbOpenDatabaseByName(sysFileCAddress, kAddrDBName, dmModeReadOnly, dbShareRead);
	if (!tmpDbP)
		goto Exit;

	// We retreives all the column definition for the new schema
    err = DbGetAllColumnDefinitions(tmpDbP, kAddressDefaultTableName, &newNumColumns, &newColumnsP);
	if (err < errNone)
		goto Exit;

	// Then, retreives all the column properties
	DbGetAllColumnPropertyValues(tmpDbP, kAddressDefaultTableName, false, &numProps, &propValuesP);

	// Loop on each schema columns and replace all their properties
	for (colIndex = 0; colIndex < newNumColumns; colIndex++)
	{
		columnID = newColumnsP[colIndex].id;

		for (propIndex = 0; propIndex < numProps; propIndex++)
		{
			numBytes = 0;
			dataP = NULL;

			if (propValuesP[propIndex].columnID != columnID)
				continue;

			err = DbGetColumnPropertyValue(tmpDbP, kAddressDefaultTableName,
									  columnID,
									  propValuesP[propIndex].propertyID,
									  &numBytes,
									  &dataP);

			if (err >= errNone)
			{
				DbSetColumnPropertyValue(addrDbP, kAddressDefaultTableName,
									columnID,
									propValuesP[propIndex].propertyID,
									numBytes,
									dataP);
			}

			if (dataP)
				DbReleaseStorage(tmpDbP, dataP);
		}
	}

Exit:
	if (newColumnsP)
		DbReleaseStorage(tmpDbP, newColumnsP);

	if (propValuesP)
		DbReleaseStorage(tmpDbP, propValuesP);

	if (tmpDbP)
		err = DbCloseDatabase(tmpDbP);

	// Delete the new database. We use it only to get its schema properties
	if ((tmpDbID = DmFindDatabase(kAddrDBName, sysFileCAddress, dmFindSchemaDB, NULL)) != 0)
		err = DmDeleteDatabase(tmpDbID);

	if (addrDbP)
		DbCloseDatabase(addrDbP);

	// Rename the database to its normal name
	dbInfo.pName = kAddrDBName;
	DmSetDatabaseInfo(addrDbID, &dbInfo);
}

/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the Address Book
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * art	06/05/95	Initial Revision
 * jmp	10/01/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *					AddrGetDatabase().
 * jmp	10/02/99	Made the support for the sysAppLaunchCmdExgReceiveData
 *					sysAppLaunchCmdExgAskUser launch codes more like their
 *					counterparts in Datebook, Memo, and ToDo.
 * jmp	10/13/99	Fix bug #22832:  Call AddrGetDatabase() on look-up
 *					sublaunch to create default database if it doesn't
 *					exists (at least the user can now see that nothing
 *					exists rather than just having nothing happen).
 * jmp	10/14/99	Oops... wasn't closing the database when we opened it
 *					in the previous change!  Fixes bug #22944.
 * jmp	10/16/99	Just create a database on hard reset if the default
 *					database doesn't exist.
 * jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from
 *					sysAppLaunchCmdExgAskUser since it was already being done
 *					in sysAppLaunchCmdExgReceiveData if the user affirmed
 *					sysAppLaunchCmdExgAskUser.
 * LFe	10/23/02	Update to manage new status_t type definition.
 *					Back to PilotMain
 *
 ***********************************************************************/
uint32_t PilotMain (uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t				error = errNone;
	ExgPassableSocketType *	passableSocketP;
	ExgSocketType *			socketP;
	Boolean					restoreCursor;
	Boolean					dbOpened;
	LmLocaleType			ovlyLocale;

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Init( "Address", cmd );
	#endif

	TraceOutput(TL(appErrorClass, "====> Address entering PilotMain cmd=%d",cmd));

	switch (cmd)
	{
	case AddressBookIdLookupLaunch:
		dbOpened = false;

		if (!gAddrDB && !(launchFlags & sysAppLaunchFlagSubCall))
		{
			dbOpened = true;
			error = PrvAppStart(dmModeReadOnly, cmd, true);
		}

		if (error >= errNone)
			AddressIdLookup((AddressIdLookupLibSearchTypePtr) cmdPBP);

		if (dbOpened && !(launchFlags & sysAppLaunchFlagSubCall))
			PrvAppStop(false);
		break;

	case sysAppLaunchCmdPanelCalledFromApp:
		{
			status_t	err = errNone;
			uint32_t	ftrValue;

			gCalledFromApp = true;
			gCallerAppDbID = cmdPBP ? *(LocalID*)cmdPBP : 0;

			if (gCallerAppDbID &&
				(err = FtrGet(sysFileCAddress, kRecordIDToGoFtrID, &ftrValue)) >= errNone)
			{
				// Get rid of the feature.
				FtrUnregister(sysFileCAddress, kRecordIDToGoFtrID);

				PrvCheckDBSchemaLocale();

				if ((error = PrvAppStart(0, cmd, false)) < errNone)
					goto ExitError;
				else
				{
					GoToParamsType * goToParamsP = MemPtrNew(sizeof(GoToParamsType));
					if (goToParamsP)
					{
						MemSet(goToParamsP, sizeof(GoToParamsType), 0);
						goToParamsP->recordID = ftrValue;
						RecordViewGotoItem(goToParamsP);
						MemPtrFree(goToParamsP);
					}
				}

				PrvAppEventLoop ();
				PrvAppStop (true);
			}
			else
			{
				PrvCheckDBSchemaLocale();

				if ((error = PrvAppStart(0, cmd, false)) < errNone)
					goto ExitError;

				FrmGotoForm (gApplicationDbP, ListView);
				PrvAppEventLoop ();
				PrvAppStop (true);
			}
		}
		break;

	case sysAppLaunchCmdNormalLaunch:
		PrvCheckDBSchemaLocale();

		if ((error = PrvAppStart(0, cmd, false)) < errNone)
			goto ExitError;

		FrmGotoForm (gApplicationDbP, ListView);
		PrvAppEventLoop ();
		PrvAppStop (true);
		break;

	case sysAppLaunchCmdExgGetData:
		// Handling get request - since we want to be fully launched, clone the socket and
		// AppSwitch to ourselves, otherwise we could respond here
		passableSocketP = PrvExgMakePassableSocket((ExgSocketPtr) cmdPBP);

		if (passableSocketP)
		{
			// we really only need the name and goToCreatorID fields, but we'll copy the whole struture here.
			error = PrvAppSwitchByCreator(sysFileCAddress, appLaunchCmdExgGetFullLaunch, (MemPtr) passableSocketP , sizeof(ExgPassableSocketType));
			if (error < errNone)
			{
				error = exgErrUserCancel; // return cancel error to let caller skip reading
				break;
			}
			else
				MemPtrFree(passableSocketP);
		}
		else
			error = exgErrAppError;
		break;

	case appLaunchCmdExgGetFullLaunch:
		// This is our custom full launch code
		// Repair socket from transit
		socketP = PrvExgFixupPassableSocket((ExgPassableSocketType *)cmdPBP);

		// Test if the PIM is called for attach
		gAttachRequest = PrvCheckForAttachURL(socketP, gTransportPrefix, NULL);

		error = PrvAppStart(0, cmd, false);
		if (error < errNone)
			break;

		// Switch to day view (not saved view because the attach can only be performed from this one)
		FrmGotoForm (gApplicationDbP, ListView);
		PrvAppEventLoop ();
		PrvAppStop (true);

     	ExgNotifyGoto(socketP, 0);
		break;

	case sysAppLaunchCmdFind:
		PIMAppProfilingBegin("Global Find");

		dbOpened = false;
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			error = PrvAppStart((uint16_t)((FindParamsPtr) cmdPBP)->dbAccesMode, cmd, true);
		}
		else if (!gAddrDB)
		{
			uint16_t	dbAccessMode;

			dbAccessMode = (gPrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
			error = PrvAppOpenDatabase(dbAccessMode);
			dbOpened = true;
		}


		if (error >= errNone)
			PrvAppSearch((FindParamsType*)cmdPBP);

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrvAppStop(false);
		}
		else if (dbOpened)
		{
			DbCursorClose(gCursorID);
			DbCloseDatabase(gAddrDB);

			gCursorID = dbInvalidCursorID;
			gAddrDB = NULL;
		}

		PIMAppProfilingEnd();
		break;

	// This action code could be sent to the app when it's already running.
	case sysAppLaunchCmdGoTo:
		dbOpened = false;
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			error = PrvAppStart(0, cmd, true);
		}
		else if (!gAddrDB)
		{
			uint16_t	dbAccessMode;

			dbAccessMode = (gPrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
			error = PrvAppOpenDatabase(dbAccessMode);
			dbOpened = true;
		}

		if (error >= errNone)
		{
			Boolean	closeAllForms;

			// Close all form if already launched
			closeAllForms = (Boolean)((launchFlags & sysAppLaunchFlagSubCall) && !dbOpened);
            PrvAppGoToItem((GoToParamsPtr)cmdPBP, closeAllForms);
		}

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			PrvAppEventLoop ();
			PrvAppStop (true);
		}
		else if (dbOpened)
		{
			DbCursorClose(gCursorID);
			DbCloseDatabase(gAddrDB);

			gCursorID = dbInvalidCursorID;
			gAddrDB = NULL;
		}
		break;


	// Launch code sent to running app before sysAppLaunchCmdFind
	// or other action codes that will cause data PrvAppSearches or manipulation.
	case sysAppLaunchCmdSaveData:
		FrmSaveAllForms ();
		break;


	// We are requested to initialize an empty database (by sync app).
	case sysAppLaunchCmdInitDatabase:
		PrvAppLaunchCmdDatabaseInit (((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
		break;


	// This launch code is sent after the system is reset.  We use this time
	// to create our default database.  If there is no default database image,
	// then we create an empty database.
	case sysAppLaunchCmdSystemReset:
		// Register to receive vcf files
		TransferRegisterData(gApplicationDbP);

		// Check if we have to create default DB
		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
		{
			DmOpenRef dbP;

			// Get formats locale so it will be saved in the DB.
			LmGetFormatsLocale(&gFormatLocale);

			// By using AddressDBOpenDatabase instead of ToolsCreateDefaultDatabase
			// we ensure that the app info schema and sort indexes are set correctly
			if ((error = AddressDBOpenDatabase(gApplicationDbP, dmModeReadWrite, &dbP)) >= errNone)
			{
				// Initialize the schema. That way, AutoFillDBs will be created and it won't
				// take too much time at AddressBook first launch
				AddressTabInitSchemaInfoFromDB(&gBookInfo, dbP);
				AddressTabReleaseSchemaInfoFromDB(&gBookInfo, dbP);
				DbCloseDatabase(dbP);
			}

			// Save the current locale
			DmGetOverlayLocale(&ovlyLocale);
			AddressDBSaveOverlayLocale(&ovlyLocale);
		}
		// If the system locale has changed, we must update the schema properties so the fields
		// will be changed to the new language
		else
		{
			PrvCheckDBSchemaLocale();
		}
		break;


		// Present the user with ui to perform a lookup and return a string
		// with information from the selected record.
	case sysAppLaunchCmdLookup:
		dbOpened = false;

		if (!gAddrDB && !(launchFlags & sysAppLaunchFlagSubCall))
		{
			dbOpened = true;
			error = PrvAppStart(dmModeReadOnly, cmd, true);
		}

		if (error >= errNone)
			Lookup((AddrLookupParamsPtr) cmdPBP);

		if (dbOpened && !(launchFlags & sysAppLaunchFlagSubCall))
            PrvAppStop(false);
		break;


	case sysAppLaunchCmdExgAskUser:
		dbOpened = false;

		if (!gAddrDB && !(launchFlags & sysAppLaunchFlagSubCall))
		{
			dbOpened = true;
			error = PrvAppStart(0, cmd, true);
		}

		if (error >= errNone)
			error = ToolsCustomAcceptBeamDialog(gAddrDB, (ExgAskParamPtr)cmdPBP);

		if (dbOpened && !(launchFlags & sysAppLaunchFlagSubCall))
            PrvAppStop(false);

		break;

	case sysAppLaunchCmdExgReceiveData:
		dbOpened = false;

		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!gAddrDB && !(launchFlags & sysAppLaunchFlagSubCall))
		{
			dbOpened = true;
			error = PrvAppStart(0, cmd, true);
		}

		if (error >= errNone)
		{
			Boolean	 saveRecordID = false;

			if (((ExgSocketPtr)cmdPBP)->goToParams.matchCustom == sysFileCAddress)
			{
				// Store the record Unique ID
				saveRecordID = true;
				((ExgSocketPtr)cmdPBP)->goToParams.matchCustom = 0;
			}

			error = TransferReceiveData(gAddrDB, gCursorID, (ExgSocketPtr) cmdPBP, true, true, NULL); // Accept before receiving

			if ((error >= errNone) && (saveRecordID))
				error = FtrSet(sysFileCAddress, kRecordIDToGoFtrID, ((ExgSocketPtr)cmdPBP)->goToParams.uniqueID);
		}

		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			if (dbOpened)
				PrvAppStop(false);
		}
		else // The AddressBook is launched
		{
			FormType *	formP;

			// Refresh the ListView if it's currently displayed.
			if ((formP = FrmGetFormPtr(ListView)) != NULL && FrmVisible(formP))
				ListViewUpdateDisplay(true);
		}
		break;

	case sysAppLaunchCmdExgPreview:
		dbOpened = false;

		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!gAddrDB && !(launchFlags & sysAppLaunchFlagSubCall))
		{
			// if gAddrDB != NULL => ExgPreview during an exgAccept for ex.
			dbOpened = true;
			error = PrvAppStart(dmModeReadOnly, cmd, true);
		}

		if (error >= errNone)
			TransferPreview((ExgPreviewInfoType*)cmdPBP);

		if (dbOpened && !(launchFlags & sysAppLaunchFlagSubCall))
			PrvAppStop(false);

		break;


	// Import/Export launch codes
	case sysAppLaunchCmdExportRecordGetCount:
	case sysAppLaunchCmdExportRecord:
	case sysAppLaunchCmdImportRecord:
	case sysAppLaunchCmdDeleteRecord:
	case sysAppLaunchCmdMoveRecord:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		restoreCursor = false;
		if (!(launchFlags & sysAppLaunchFlagSubCall))
			error = PrvAppStart(0, cmd, true);
		else
			restoreCursor = PrvImpExpReopenCursor();

		if (error >= errNone)
		{
			switch (cmd)
			{
			case sysAppLaunchCmdExportRecordGetCount:
				*(uint32_t*)cmdPBP = DbCursorGetRowCount(gCursorID);
				break;

			case sysAppLaunchCmdExportRecord:
				error = TransferExportData(gAddrDB, (ImportExportRecordParamsPtr)cmdPBP);
				break;

			case sysAppLaunchCmdImportRecord:
				error = TransferImportData(gAddrDB, gCursorID, (ImportExportRecordParamsPtr)cmdPBP);
				break;

			case sysAppLaunchCmdDeleteRecord:
				error = TransfertDeleteData(gAddrDB, (ImportExportRecordParamsPtr)cmdPBP);
				break;

			case sysAppLaunchCmdMoveRecord:
				error = TransfertMoveData(gAddrDB, (ImportExportRecordParamsPtr)cmdPBP);
				break;
			}
		}

		if (!(launchFlags & sysAppLaunchFlagSubCall))
			PrvAppStop(false);
		else if (restoreCursor)
			PrvImpExpRestoreCursor();

		break;

	case sysLaunchCmdInitialize:
		// Get the DatabaseID & DBRef
		error = SysGetModuleDatabase(SysGetRefNum(), &gAddrDatabaseID, &gApplicationDbP);
		break;

	}

ExitError:

	TraceOutput(TL(appErrorClass, "<==== Address exiting PilotMain cmd=%d errCode=%d",cmd,error));

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Exit();
	#endif

	return error;
}
