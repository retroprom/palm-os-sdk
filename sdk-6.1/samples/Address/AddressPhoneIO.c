/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressPhoneIO.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      Address Book routines to transfer records.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <UIResources.h>
#include <SystemResources.h>
#include <PhoneBookIOLib.h>

#include "Address.h"
#include "AddressPhoneIO.h"
#include "AddressTools.h"

#include "AddressRsc.h"

/***********************************************************************
 *
 *   Internal statics
 *
 ***********************************************************************/

static CategoryID 	sPhoneIOCategory = catIDUnfiled;
static char			sPhoneCategoryName[kCategoryLabelLength];
static Boolean		sShowEditingInCatTrigger = false;
static uint16_t		sDialogResID = PhoneIOExportPhoneDirectoryDialog;

/***********************************************************************
 *
 * FUNCTION:    PrvAddressPhoneIOHandleEvent
 *
 * DESCRIPTION: The event handler
 *
 * PARAMETERS:  eventP -> pointer on the event.
 *
 * RETURNED:    handled state
 *
 * REVISION HISTORY:
 *		Name	Date			Description
 *		----	----			-----------
 *		PLe		12/18/2002		Initial Revision
 *
 ***********************************************************************/
static Boolean PrvAddressPhoneIOHandleEvent(EventType * eventP)
{
	Boolean			handled = false;
	FormType* 		dialogP;
	Boolean			categoryChanged;
	CategoryID *	selectedCategoriesP = NULL;
	uint32_t		selectedNumCategories = 0;
	CategoryID *	currentCategoryP = NULL;
	uint32_t		currentNumCategory = 0;
		
	switch(eventP->eType)
	{
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID == PhoneIOCategoryTrigger)
			{
				dialogP = FrmGetFormPtr(sDialogResID);

				// Leave currentCategoryP to NULL if sPhoneIOCategory is Unfiled
				if (sPhoneIOCategory != catIDUnfiled)
				{
					currentCategoryP = &sPhoneIOCategory;
					currentNumCategory = 1;
				}
				
				categoryChanged = CatMgrSelectEdit(gAddrDB, dialogP,
					PhoneIOCategoryTrigger, sPhoneCategoryName, PhoneIOCategoryList, false, currentCategoryP, currentNumCategory,
					&selectedCategoriesP, &selectedNumCategories, sShowEditingInCatTrigger, NULL);
				
				if (categoryChanged)
					sPhoneIOCategory = (selectedCategoriesP != NULL) ? *selectedCategoriesP : catIDUnfiled;
				
				if (selectedCategoriesP)
					CatMgrFreeSelectedCategories(gAddrDB, &selectedCategoriesP);
				
				handled = true;
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		PrvAddressPhoneIOGetUserSettings
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
 * PLe		03/14/03	Created
 *
 ***********************************************************************/
static Boolean PrvAddressPhoneIOGetUserSettings(uint8_t *phoneDirectoryP, CategoryID *categoryIDP)
{
	ControlType * 	phoneDirectoryControlP;
	ListType * 		phoneDirectoryListP;
	ControlType * 	categoryControlP;
	FormType* 		dialogP;
	Boolean			result = false;
	
	dialogP = FrmInitForm(gApplicationDbP, sDialogResID);
	
	// Set SIM / Internal
	phoneDirectoryControlP = (ControlType*) ToolsGetFrmObjectPtr(dialogP, PhoneIOPhoneDirectoryTrigger);
	phoneDirectoryListP = (ListType*) ToolsGetFrmObjectPtr(dialogP, PhoneIOPhoneDirectoryList);
	LstSetSelection(phoneDirectoryListP, kPhoneBkSelectSIM);
	CtlSetLabel(phoneDirectoryControlP, LstGetSelectionText(phoneDirectoryListP, kPhoneBkSelectSIM));
	
	// Set initial category between gCurrentCategoriesP,
	sPhoneIOCategory = (sDialogResID == PhoneIOExportPhoneDirectoryDialog && gCurrentCategoriesP && *gCurrentCategoriesP != catIDAll) ?
				*gCurrentCategoriesP : catIDUnfiled;
	categoryControlP = (ControlType*) ToolsGetFrmObjectPtr(dialogP, PhoneIOCategoryTrigger);
	CatMgrSetTriggerLabel(gAddrDB, &sPhoneIOCategory, 1, categoryControlP, sPhoneCategoryName);
	
	// Set the event handler
	FrmSetEventHandler(dialogP, PrvAddressPhoneIOHandleEvent);
	
	if (FrmDoDialog (dialogP) == PhoneIOPhoneCatOkButton)
	{
		// Update selection
		*phoneDirectoryP = (uint8_t) LstGetSelection(phoneDirectoryListP);
		*categoryIDP = sPhoneIOCategory;

		result = true;
	}

	// Switch back to Interaction mode.
	FrmSetNavState(FrmGetFormPtr(sDialogResID), kFrmNavStateFlagsInteractionMode);
	
	FrmDeleteForm(dialogP);
	
	return result;	
}



/***********************************************************************
 *
 * FUNCTION:		AddressPhoneIOGetUserImportSettings
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
 * PLe		03/14/03	Created
 *
 ***********************************************************************/
Boolean AddressPhoneIOGetUserImportSettings(uint8_t* phoneDirectoryP, CategoryID *categoryIDP)
{
	// Import : the user can create a category to put his phone directory
	sShowEditingInCatTrigger = true;
	sDialogResID = PhoneIOImportPhoneDirectoryDialog;
	
	return PrvAddressPhoneIOGetUserSettings(phoneDirectoryP, categoryIDP);
}

/***********************************************************************
 *
 * FUNCTION:		AddressPhoneIOGetUserExportSettings
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
 * PLe		03/14/03	Created
 *
 ***********************************************************************/
Boolean AddressPhoneIOGetUserExportSettings(uint8_t *phoneDirectoryP, CategoryID *categoryIDP)
{
	// Export : No need to create a category
	sShowEditingInCatTrigger = false;
	sDialogResID = PhoneIOExportPhoneDirectoryDialog;
	
	return PrvAddressPhoneIOGetUserSettings(phoneDirectoryP, categoryIDP);
}
