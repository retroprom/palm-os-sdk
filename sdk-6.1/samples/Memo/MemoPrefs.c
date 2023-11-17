/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoPrefs.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
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
#include <PhoneLookup.h>
#include <SoundMgr.h>
#include <FeatureMgr.h>
#include <string.h>

#include "MemoRsc.h"
#include "MemoMain.h"
#include "MemoPrefs.h"
#include "MemoDB.h"


/************************************************************
 *	Internal constants
 ************************************************************/
#define memoPrefsVersionNum				3
#define memoPrefID						0x00
#define memoCurCategoriesPrefID			0x01

#define kSortByListItemManual			0
#define kSortByListItemAlphabetic		1

/************************************************************
 *	Internal structures
 ************************************************************/
typedef struct {
	uint32_t		topVisibleRecord;
	uint32_t		currentRecord;		// word1
	uint16_t		currentView;
	FontID			v20editFont;
	uint8_t			reserved1;
	uint32_t		editScrollPosition;	// word3
	Boolean			showAllCategories;
	Boolean			saveBackup;

	// Version 2 preferences
	FontID			v20listFont;

	// Version 3 preferences
	FontID			editFont;			// word4	
	FontID			listFont;
	uint8_t			reserved[3];
	
	// Version 6 preferences
	Boolean			alphaSort;
	
} MemoPreferenceType;

/************************************************************
 *	Global variables
 ************************************************************/
extern uint32_t			gListViewCursorID;

/***********************************************************************
 *
 * FUNCTION:    MemoLoadPrefs
 *
 * DESCRIPTION: Load the preferences and do any fixups needed for backwards
 *					 and forwards compatibility
 *
 * PARAMETERS:  currentRecordID <- returned record id from preferences.
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		01/13/98	BGT	Initial Revision.
 *		08/04/99	kwk	Cleaned up setting EditFont/ListFont from prefs.
 *
 ***********************************************************************/
void MemoLoadPrefs(void)
{
	MemoPreferenceType	prefs;
	uint32_t			prefsSize;
	int16_t				prefsVersion;
	Boolean				needFontInfo = false;

	// Read the preferences / saved-state information.
	prefsSize = (uint32_t)sizeof (MemoPreferenceType);

	prefsVersion = PrefGetAppPreferences (sysFileCMemo, memoPrefID, &prefs, &prefsSize, true);
	if (prefsVersion > memoPrefsVersionNum)
		prefsVersion = noPreferenceFound;

	if (prefsVersion > noPreferenceFound)
	{
		// Try to carry forward the version 2 preferences for the font
		if (prefsVersion < 2)
		{
			// No font data in original prefs
			needFontInfo = true;
		}
		else if (prefsVersion == 2)
		{
			prefs.editFont = prefs.v20editFont;
			prefs.listFont = prefs.v20listFont;

			// Use the 'better' large font if we've got it, since version 2
			// prefs would have been created on an older version of the OS
			// which didn't have the largeBoldFont available.
			if (prefs.editFont == largeFont)
				prefs.editFont = largeBoldFont;

			if (prefs.listFont == largeFont)
				prefs.listFont = largeBoldFont;
		}

		gTopVisibleRecord	= prefs.topVisibleRecord;
		gCurrentRecord		= prefs.currentRecord;
		gCurrentView		= prefs.currentView;
		gEditScrollPosition	= prefs.editScrollPosition;
		gSaveBackup			= prefs.saveBackup;
		gSortAlphabetically = prefs.alphaSort;
	}
	else
	{
		needFontInfo = true;
	}

	// If the prefs didn't supply us with font info, we'll need to get it ourselves.
	if (needFontInfo)
	{
		uint32_t defaultFont;
		FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
		gEditFont = (FontID)defaultFont;
		gListFont = (FontID)defaultFont;
	}
	else
	{
		gEditFont = prefs.editFont;
		gListFont = prefs.listFont;
	}

	// The first time this app starts register to handle .txt and text/plain.
	if (prefsVersion != memoPrefsVersionNum)
		RegisterData();
}


/***********************************************************************
 *
 * FUNCTION:    MemoSavePrefs
 *
 * DESCRIPTION: Save the preferences and do any fixups needed for backwards
 *					 and forwards compatibility
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			BGT	1/13/98	Initial Revision
 *
 ***********************************************************************/
void MemoSavePrefs(uint32_t scrollPosition)
{
	MemoPreferenceType prefs;

	MemSet(&prefs, sizeof(MemoPreferenceType), 0);

	// Write the preferences / saved-state information.
	prefs.topVisibleRecord		= gTopVisibleRecord;
	prefs.currentRecord			= gCurrentRecord;
	prefs.currentView			= gCurrentView;
	prefs.editScrollPosition	= scrollPosition;
	prefs.saveBackup			= gSaveBackup;
	prefs.editFont				= gEditFont;
	prefs.listFont				= gListFont;
	prefs.alphaSort				= gSortAlphabetically;
	
	prefs.v20editFont = stdFont;
	prefs.v20listFont = stdFont;
	
	// Write the state information.
	PrefSetAppPreferences (sysFileCMemo, memoPrefID, memoPrefsVersionNum, &prefs,
						   sizeof (MemoPreferenceType), true);
}

/***********************************************************************
 *
 * FUNCTION:    MemoLoadCurrentCategories
 *
 * DESCRIPTION: Loads the current categories from app preferences
 *
 * PARAMETERS:  <-	numCategoriesP:	number of categories
 *				<-	categoriesPP:	Array of CategoryID
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/24/02	Initial revision
 *
 ***********************************************************************/
void MemoLoadCurrentCategories(uint32_t *numCategoriesP, CategoryID **categoriesPP)
{
	uint32_t			size;
	int16_t			prefsVersion;
	
	prefsVersion = PrefGetAppPreferences(sysFileCMemo, memoCurCategoriesPrefID, NULL, &size, true);
	if (prefsVersion == noPreferenceFound)
	{
		*categoriesPP = MemPtrNew(sizeof(CategoryID));
		*numCategoriesP = 1;
		**categoriesPP = catIDAll;
		return;
	}
	
	*categoriesPP = MemPtrNew(size);
	PrefGetAppPreferences(sysFileCMemo, memoCurCategoriesPrefID, *categoriesPP, &size, true);
	*numCategoriesP = size / sizeof(CategoryID);
}

/***********************************************************************
 *
 * FUNCTION:    MemoSaveCurrentCategories
 *
 * DESCRIPTION: Saves the current categories to app preferences
 *
 * PARAMETERS:  ->	numCategories:	number of categories
 *				->	categoriesP:	Array of CategoryID
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/24/02	Initial revision
 *
 ***********************************************************************/
void MemoSaveCurrentCategories(uint32_t numCategories, CategoryID *categoriesP)
{
	PrefSetAppPreferences (sysFileCMemo, memoCurCategoriesPrefID, memoPrefsVersionNum, categoriesP,
							numCategories * sizeof(CategoryID), true);
}

#if 0
#pragma mark ----
#endif
/***********************************************************************
 *
 * FUNCTION:    PreferencesApply
 *
 * DESCRIPTION: This routine applies the changes made in the Preferences
 *              Dialog
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if update needed
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/95	Initial Revision
 *
 ***********************************************************************/
static Boolean PreferencesApply(void)
{
	uint16_t	sel;
	Boolean		alphaSort;
	
	// Update the sort order.  Reset the To Do list to the top.
	
	sel = LstGetSelection(GetObjectPtr(PreferencesSortByList, PreferencesDialog));
	alphaSort = (sel == kSortByListItemAlphabetic);
	
	if (alphaSort != gSortAlphabetically)
	{
		MemoDBChangeSortOrder(gMemoDB, &gListViewCursorID, alphaSort);
		gForceReload = true ;
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PreferencesInit
 *
 * DESCRIPTION: This routine initializes the Preferences Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/95	Initial Revision
 *
 ***********************************************************************/
static void PreferencesInit (void)
{
	char *		label;
	ListPtr		lst;
	ControlPtr	ctl;
	uint16_t		sel;

	// Set the trigger and popup list that indicates the sort order.
	sel = (gSortAlphabetically ? kSortByListItemAlphabetic : kSortByListItemManual);
	
	lst = GetObjectPtr(PreferencesSortByList, PreferencesDialog);
	label = LstGetSelectionText(lst, sel);

	ctl = GetObjectPtr(PreferencesSortByTrigger, PreferencesDialog);
	CtlSetLabel(ctl, label);
	
	LstSetSelection(lst, sel);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Preferences
 *              Dialog Box" of the Memo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/95	Initial Revision
 *
 ***********************************************************************/
Boolean PreferencesHandleEvent (EventType * event)
{
	Boolean		handled = false;
	FormType *	formP;
	
	switch(event->eType)
	{
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case PreferencesOkButton:
					if (PreferencesApply())
						UtilsFrmInvalidateWindow(ListView);
					FrmReturnToForm(ListView);
					handled = true;
					break;

				case PreferencesCancelButton:
					FrmReturnToForm (ListView);
					handled = true;
					break;
			}
			break;

		case frmOpenEvent:
			PreferencesInit();
			handled = true;
			break;

		case winUpdateEvent:
			formP = FrmGetFormPtr(PreferencesDialog);
			if (event->data.winUpdate.window != FrmGetWindowHandle(formP))
				break;
			FrmDrawForm(formP);
			handled = true;
			break;
	}

	return (handled);
}
