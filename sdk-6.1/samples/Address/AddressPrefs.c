/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressPrefs.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the Address Book application's pref screen
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <ErrorMgr.h>
#include <Preferences.h>
#include <FeatureMgr.h>
#include <SystemMgr.h>
#include <Chars.h>
#include <PenInputMgr.h>
#include <string.h>
#include <SysUtils.h>
#include <TraceMgr.h>
#include <PalmTypesCompatibility.h> // For min & max
#include <FormLayout.h>

#include "AddressPrefs.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressTransfer.h"
#include "AddressRsc.h"
#include "AddressDBSchema.h"
#include "AddressU32List.h"

static 	char	**sOrderByItemsText = NULL;

/***********************************************************************
 *
 * FUNCTION:	AddressLoadPrefs
 *
 * DESCRIPTION:	Load the application preferences and fix them up if
 *				there's a version mismatch.
 *
 * PARAMETERS:	appDbRef	- AddressBook application database
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * BGT		01/08/98	Initial revision
 * LFe		10/21/02	Reset load. As we switch to ARM we cannot get
 *						the compatibility anymore with previous version.
 *
 ***********************************************************************/
void PrefsLoad(DmOpenRef appDbRef)
{
	uint32_t			dFont;
	int16_t				prefsVersion;
	uint32_t			prefsSize;
	AddrPreferenceType	prefs;

	// First initialize the font with the system default one.
	gNoteFont		=
	gAddrListFont	=
	gAddrRecordFont =
	gAddrEditFont	= (FontID)(FtrGet(sysFtrCreator, sysFtrDefaultFont, &dFont) >= errNone ? dFont : stdFont);
	//(FtrGet(sysFtrCreator, sysFtrDefaultBoldFont, &dFont) >= errNone ? (FontID) dFont : largeBoldFont);

	// Read the preferences / saved-state information.
	// New ARM version. Impossible to read previous versions, if any.
	prefsSize = (uint32_t)sizeof (AddrPreferenceType);
	memset(&prefs, 0, prefsSize);
	prefsVersion = PrefGetAppPreferences (sysFileCAddress, kAddrPrefID, &prefs, &prefsSize, true);

	if (prefsVersion == kAddrPrefVersionNum)
	{
		gSaveBackup				= prefs.saveBackup;
		gRememberLastCategory	= prefs.rememberLastCategory;
		gNoteFont				= prefs.noteFont;
		gAddrListFont			= prefs.addrListFont;
		gAddrRecordFont			= prefs.addrRecordFont;
		gAddrEditFont			= prefs.addrEditFont;
		gBusinessCardRowID		= prefs.businessCardRowID;
		gEnableTapDialing		= prefs.enableTapDialing;
		gOrderByIndex			= prefs.orderByIndex;
		gTransfertMode			= prefs.transfertMode;

		memmove (gTransfertNote, prefs.transfertNote, sizeof(gTransfertNote));
	}

	gCurrentOrderByStr[0] = nullChr;
	SysStringByIndex(gApplicationDbP, OrderBySQLQueriesStrList, gOrderByIndex, gCurrentOrderByStr, kMaxOrderBySQLStrSize);

	gCurrentOrderByType = U32ListGetItem(gApplicationDbP, OrderByTypeUInt32List, NULL, gOrderByIndex);

	// The first time this app starts register to handle vCard data.
	if (prefsVersion != kAddrPrefVersionNum)
		TransferRegisterData(appDbRef);
}

/***********************************************************************
 *
 * FUNCTION:    AddressSavePrefs
 *
 * DESCRIPTION: Save the Address preferences with fixups so that
 *				previous versions won't go crazy.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		  Date      Description
 * ----		--------	-----------
 * BGT		01/08/98    Initial Revision
 * SCL		02/12/99    Clear reserved fields before writing saved prefs
 * gap		12/06/00	gEnableTapDialing is now false by default, only
 *						checked if turned on by user
 * LFe		10/21/02	Reset to new ARM version of the Prefs
 *
 ***********************************************************************/
void PrefsSave(void)
{
	AddrPreferenceType	prefs;

	DbgOnlyFatalErrorIf(gNoteFont > largeBoldFont, "Note font invalid");

	// Write the preferences / saved-state information.
	prefs.noteFont				= gNoteFont;
	prefs.addrListFont			= gAddrListFont;
	prefs.addrRecordFont		= gAddrRecordFont;
	prefs.addrEditFont			= gAddrEditFont;
	prefs.saveBackup			= gSaveBackup;
	prefs.rememberLastCategory	= gRememberLastCategory;
	prefs.businessCardRowID		= gBusinessCardRowID;
	prefs.enableTapDialing		= gEnableTapDialing;
	prefs.orderByIndex			= gOrderByIndex;
	prefs.transfertMode			= gTransfertMode;

	memmove (prefs.transfertNote, gTransfertNote, sizeof(gTransfertNote));

	// Write the state information.
	PrefSetAppPreferences (sysFileCAddress, kAddrPrefID, kAddrPrefVersionNum, &prefs, sizeof (AddrPreferenceType), true);
}


/***********************************************************************
 *
 * FUNCTION:    PrefsLoadCurrentCategories
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
void PrefsLoadCurrentCategories(uint32_t *numCategoriesP, CategoryID **categoriesPP)
{
	uint32_t	size;
	int16_t	prefsVersion;

	// No preferences saved. Set current category to catIDAll
	if (!gRememberLastCategory)
	{
		*categoriesPP = MemPtrNew(sizeof(CategoryID));
		**categoriesPP = catIDAll;
		*numCategoriesP = 1;
		return;
	}

	// Get pref version and size
	prefsVersion = PrefGetAppPreferences(sysFileCAddress, kAddrCurrentCategoriesPrefID, NULL, &size, true);

	// Preference not found. Set current category to Unfiled
	if (prefsVersion == noPreferenceFound || !size)
	{
		*categoriesPP = NULL;
		*numCategoriesP = 0;
		return;
	}

	// Load them from prefs.
	*categoriesPP = MemPtrNew(size);
	PrefGetAppPreferences(sysFileCAddress, kAddrCurrentCategoriesPrefID, *categoriesPP, &size, true);
	*numCategoriesP = size / sizeof(CategoryID);
}

/***********************************************************************
 *
 * FUNCTION:    PrefsSaveCurrentCategories
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
void PrefsSaveCurrentCategories(uint32_t numCategories, CategoryID *categoriesP)
{
	PrefSetAppPreferences(sysFileCAddress, kAddrCurrentCategoriesPrefID, kAddrPrefVersionNum, categoriesP,
						numCategories * sizeof(CategoryID), true);
}

/***********************************************************************
 *
 * FUNCTION:	PrvPrefsInit
 *
 * DESCRIPTION:	Initialize the dialog's ui.  Sets the database sort by
 *				buttons.
 *
 * PARAMETERS:	formP
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *
 * Name   	  Date		Description
 * -----	--------	-----------
 * roger  	08/02/95   	Initial Revision
 * FPa    	11/23/00   	Added PreferencesEnableTapDialingHeightGadget
 *						handling
 * ppl		02/06/02	Active input area support
 * LFe		10/23/02	Use the new sort global
 *
 ***********************************************************************/
static void PrvPrefsInit(void)
{
	uint16_t	rememberCategoryIndex;
	uint16_t	enableTapDialingIndex;
	ListPtr		exportRecordListP;
	ListPtr		orderByListP;
	ControlPtr	orderByTriggerP;
	FormPtr		formP;
	uint16_t	i;

	formP = FrmGetFormPtr(PreferencesDialog);

	rememberCategoryIndex = FrmGetObjectIndex (formP, PreferencesRememberCategoryCheckbox);
	CtlSetValue(FrmGetObjectPtr (formP, rememberCategoryIndex), gRememberLastCategory);

	enableTapDialingIndex = FrmGetObjectIndex (formP, PreferencesEnableTapDialingCheckbox);

	if (!ToolsIsDialerPresent())
	{
		Coord				x, y, z;
		uint16_t			numberOfObjects;
		uint16_t			index;
		RectangleType		rect;
		uint16_t			prefsHeightDelta;
		WinConstraintsType	constraints;

		// Hide "Enable tap-dialing" control
		FrmHideObject (formP, enableTapDialingIndex);

		// Move "Remember last category" control down
		FrmGetObjectPosition(formP, rememberCategoryIndex, &x, &z);
		FrmGetObjectPosition(formP, enableTapDialingIndex, &x, &y);
		FrmSetObjectPosition(formP, rememberCategoryIndex, x, y);

		// Change form size
		prefsHeightDelta = y - z;
		FrmGetFormInitialBounds(formP, &rect);
		rect.topLeft.y += prefsHeightDelta;
		rect.extent.y -= prefsHeightDelta;

		memset(&constraints, 0, sizeof(WinConstraintsType));
		constraints.x_pos	= -1; // Set a high negative value so the dialog moves with the AIA
		constraints.x_min	= rect.extent.x;
		constraints.x_pref	= rect.extent.x;
		constraints.x_max	= rect.extent.x;
		constraints.y_pos	= -1; // Set a high negative value so the dialog moves with the AIA
		constraints.y_min	= rect.extent.y;
		constraints.y_pref	= rect.extent.y;
		constraints.y_max	= rect.extent.y;

		WinSetConstraints(FrmGetWindowHandle(formP), &constraints);

		// Move all controls up
		numberOfObjects = FrmGetNumberOfObjects(formP);

		for (index = 0 ; index < numberOfObjects ; index++)
		{
			FrmGetObjectBounds(formP, index, &rect);
			rect.topLeft.y -= prefsHeightDelta;
			FrmSetObjectBounds(formP, index, &rect);
		}
	}
	else
		// Check or uncheck "Enable Tap Dialing" checkbox
		CtlSetValue(FrmGetObjectPtr (formP, enableTapDialingIndex), gEnableTapDialing);

	if (gOrderByCount)
	{
		FontID			savedFont;
		RectangleType	listBounds;
		int16_t			maxWidth;
		uint16_t		listIndex;
		int16_t			stringWidth = 0;
		uint16_t		textLength = 0;

		listIndex = FrmGetObjectIndex (formP, PreferencesOrderByList);
		orderByListP	= ToolsGetFrmObjectPtr (formP, PreferencesOrderByList);
		orderByTriggerP	= ToolsGetFrmObjectPtr (formP, PreferencesOrderByTrigger);

		savedFont = FntSetFont(LstGetFont (orderByListP));

		// Compute the max size available for the list.
		FrmGetFormInitialBounds (formP, &listBounds);
		maxWidth = listBounds.extent.x;
		FrmGetObjectBounds (formP, listIndex, &listBounds);
		maxWidth -= listBounds.topLeft.x - 5;

		sOrderByItemsText = MemPtrNew((uint32_t)(sizeof(char*) * gOrderByCount));

		for (i = 0; i < gOrderByCount; i++)
		{
			sOrderByItemsText[i] = MemPtrNew(kMaxOrderBySQLStrSize);
			SysStringByIndex(gApplicationDbP, OrderNameStrList, i, &sOrderByItemsText[i][0], kMaxOrderBySQLStrSize);

			stringWidth = FntCharsWidth(sOrderByItemsText[i], strlen(sOrderByItemsText[i])) + 3;	// + 3  = textInsetWidth + 1
			textLength = max(textLength, stringWidth);
			textLength = min (maxWidth, textLength);
		}

		FntSetFont(savedFont);

		listBounds.extent.x = (int16_t) textLength;
		FrmSetObjectBounds (formP, listIndex, &listBounds);
		LstSetHeight (orderByListP, gOrderByCount);
		LstSetListChoices (orderByListP, sOrderByItemsText, gOrderByCount);
		LstSetSelection (orderByListP, gOrderByIndex);
		CtlSetLabel(orderByTriggerP, sOrderByItemsText[gOrderByIndex]);
	}

	// Set the Export record selected item
	exportRecordListP = ToolsGetFrmObjectPtr(formP, PreferencesExportRecordList);
	LstSetSelection(exportRecordListP, gTransfertMode);
	CtlSetLabel(ToolsGetFrmObjectPtr(formP, PreferencesExportRecordTrigger), LstGetSelectionText(exportRecordListP, gTransfertMode));
}


/***********************************************************************
 *
 * FUNCTION:    PrvPrefsApply
 *
 * DESCRIPTION: Write the renamed field labels
 *
 * PARAMETERS:  formP - Preference form pointer
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	----		-----------
 * roger	08/02/95	Initial Revision
 * jmp		11/02/99	Don't reset gCurrentRowID to zero if it's been set to
 *						noRecord.  Fixes bug #23571.
 * gap		10/27/00	actually, when the records are resorted, the current
 *						selection should always be cleared.
 * LFe		10/23/02	Use the new sort mechanism.
 *						Remove the Sort dialog as it's only a change of
 *						sort index and not a real db sort.
 *
 ***********************************************************************/
static void PrvPrefsApply(void)
{
	int16_t		transfert;
	int16_t		orderBy;
	FormPtr		formP;

	formP = FrmGetFormPtr(PreferencesDialog);

	gRememberLastCategory	= (Boolean)CtlGetValue(ToolsGetFrmObjectPtr(formP, PreferencesRememberCategoryCheckbox ));
	gEnableTapDialing		= (Boolean)CtlGetValue(ToolsGetFrmObjectPtr(formP, PreferencesEnableTapDialingCheckbox ));
	transfert				= LstGetSelection(ToolsGetFrmObjectPtr(formP, PreferencesExportRecordList));
	orderBy					= LstGetSelection(ToolsGetFrmObjectPtr(formP, PreferencesOrderByList));

	if (transfert != noListSelection)
		gTransfertMode = (uint8_t)transfert;

	if ((orderBy != noListSelection) && (orderBy != gOrderByIndex))
	{
		DbgOnlyFatalErrorIf(orderBy >= gOrderByCount, "Invalid Index");

		// Change the sort.
		gOrderByIndex = orderBy;

		// Reset the cached Resource ID used to build the display name template (rely on sort order).
		gDisplayNameColumnListResID = 0;
		gDisplayNameLayoutResID = 0;

		SysStringByIndex(gApplicationDbP, OrderBySQLQueriesStrList, orderBy, gCurrentOrderByStr, kMaxOrderBySQLStrSize);

		gCurrentOrderByType = U32ListGetItem(gApplicationDbP, OrderByTypeUInt32List, NULL, gOrderByIndex);

		// clear the current selection
		gCurrentRowID = dbInvalidRowID;
		gTopVisibleRowIndex = kFirstRowIndex;

		// Re-open the cursor to reflect changes
		AddressDBOpenCursor();
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvReleaseOrderByList
 *
 * DESCRIPTION: .
 *
 * PARAMETERS:
 *
 * RETURNED:    .
 *
 * REVISION HISTORY:
 *
 * Name		  Date      Description
 * -----	--------	-----------
 *
 ***********************************************************************/
static void PrvReleaseOrderByList(void)
{
	uint16_t	i;

	if (sOrderByItemsText)
	{
		for (i = 0; i < gOrderByCount; i++)
			MemPtrFree(sOrderByItemsText[i]);

		MemPtrFree(sOrderByItemsText);
		sOrderByItemsText = NULL;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrefsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List By
 *              Dialog" of the Address application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		  Date      Description
 * -----	--------	-----------
 * roger   	08/02/95   	Initial Revision
 * ppl		02/06/02	Active input area support
 * LFe		10/18/02	Replace ToolsLeaveForm() by FrmReturnToForm()
 *
 ***********************************************************************/
Boolean PrefsHandleEvent (EventType * event)
{
	Boolean		handled = false;

	switch (event->eType)
	{
		case ctlSelectEvent:
		{
			switch (event->data.ctlSelect.controlID)
			{
				case PreferencesOkButton:
					PrvPrefsApply();
					PrefsSave();
					PrvReleaseOrderByList();
					FrmReturnToForm(0);
					PrvPostUserEvent(kListViewReloadTableEvent);
					handled = true;
					break;

				case PreferencesCancelButton:
					PrvReleaseOrderByList();
					FrmReturnToForm(0);
					handled = true;
					break;
			}
			break;
		}


		case frmOpenEvent:
			PrvPrefsInit();
			handled = true;
			break;

		case frmCloseEvent:
			PrvReleaseOrderByList();
			break;
	}

	return (handled);
}
