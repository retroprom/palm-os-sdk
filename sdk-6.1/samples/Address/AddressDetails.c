/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDetails.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the Address Book application's details screen
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <CatMgr.h>
#include <UIResources.h>
#include <StringMgr.h>
#include <ErrorMgr.h>
#include <PenInputMgr.h>
#include <TraceMgr.h>
#include <string.h>
#include <FormLayout.h>

#include "AddressDetails.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressNote.h"
#include "AddressRsc.h"
#include "AddressDBSchema.h"
#include "AddressEdit.h"

/***********************************************************************
 *
 *   Internal Macros
 *
 ***********************************************************************/
#define kPopupTriggerIndicatorWidth		15

/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/
static char	*		sShowInListPopupListItems[kMaxItemNumberInShowList];
static uint32_t		sShowInListPopupListItemsColIdMap[kMaxItemNumberInShowList];
static char *		sShowInListTriggerLabelP = NULL;
static Coord		sShowInListTriggerLabelMaxWidth = 0;

static CategoryID *	sCurrentCategoriesP = NULL;
static uint32_t		sCurrentNumCategories = 0;

static CategoryID *	sSavedCategoriesP = NULL;
static uint32_t		sSavedNumCategories = 0;

static Boolean		sCategoryChanged = false;

static char			sCategoryTriggerLabel[(catCategoryNameLength * 2) + 1];

static uint16_t		sCurrentNavObjectID;

/***********************************************************************
 *
 * FUNCTION:    PrvAllocAndSetShowInListPopupListItem
 *
 * DESCRIPTION: This allocated required memory and filled a ShowInList
 *				Popup List item using the given parameters.
 *
 * PARAMETERS:  itemIdx - index of the item to allocate and set.
 *				columnLabelP - column label Ptr.
 *				tabNameP - tab Name Ptr.
 *				columnID - Id of the column in the record schema.
 *
 * RETURNED:    true if success, false if error.
 *
 * REVISION HISTORY:
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		11/19/02	Initial revision
 *
 ***********************************************************************/
static Boolean PrvAllocAndSetShowInListPopupListItem(uint16_t itemIdx, char *templateP, char *columnLabelP, char *tabNameP, uint32_t columnID)
{
	if (tabNameP != NULL && columnLabelP != NULL)
	{
		sShowInListPopupListItems[itemIdx] = TxtParamString(templateP, tabNameP, columnLabelP, NULL, NULL);
		if (sShowInListPopupListItems[itemIdx] == NULL)
		{
			ErrNonFatalDisplay("Out of memory !");
			return false;
		}

		sShowInListPopupListItemsColIdMap[itemIdx] = columnID;
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvDetailsSetCategoryTriggerLabel
 *
 * DESCRIPTION: Set the popup trigger label with the categories label.
 *				Don't use CatMgrSetTriggerLabel or CatMgrSelectEdit to
 *				set the mabel. It truncate the string with too small values.
 *
 * PARAMETERS:  formP - Form that contains the popup trigger.
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		05/14/03	Initial revision
 *
 ***********************************************************************/
static void PrvDetailsSetCategoryTriggerLabel(FormType* formP)
{
	uint16_t		maxCatWidth, catTriggerIndex;
	Coord			catPosx, catPosy;
	RectangleType	formRect;

	catTriggerIndex = FrmGetObjectIndex (formP, DetailsCategoryTrigger);
	FrmGetObjectPosition(formP, catTriggerIndex, &catPosx, &catPosy);
	FrmGetFormInitialBounds(formP, &formRect);
	maxCatWidth = formRect.extent.x - catPosx - 1;

	CatMgrTruncateName(gAddrDB, sCurrentCategoriesP, sCurrentNumCategories, maxCatWidth, sCategoryTriggerLabel);
	CtlSetLabel (FrmGetObjectPtr (formP, catTriggerIndex), sCategoryTriggerLabel);
}

/***********************************************************************
 *
 * FUNCTION:    PrvDetailsSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the category was changed in a way that
 *              require the list view to be redrawn.
 *
 *              The following global variables are modified:
 *                     gCategoryName
 *
 * REVISION HISTORY:
 * Name   Date       Description
 * ----   ----       -----------
 * art	  06/05/95   Initial Revision
 * art    09/28/95   Fixed problem with merging and renaming
 * gap	  08/13/99   Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static void PrvDetailsSelectCategory(void)
{
	CategoryID *	selectedCategoriesP = NULL;
	uint32_t		selectedNumCategories = 0;
	FormType*		formP;
	Boolean			categoryChanged;
	status_t		err;

	formP = FrmGetFormPtr(DetailsDialog);

	categoryChanged = CatMgrSelectEdit(gAddrDB, formP,
									DetailsCategoryTrigger, sCategoryTriggerLabel, DetailsCategoryList, true,
									sCurrentCategoriesP, sCurrentNumCategories,
									&selectedCategoriesP, &selectedNumCategories,
									true, NULL);

	if (categoryChanged)
	{
		if (sCurrentCategoriesP)
			DbReleaseStorage(gAddrDB, sCurrentCategoriesP);

		err = DbSetCategory(gAddrDB, gCurrentRowID, selectedNumCategories, selectedCategoriesP);
		ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");
		DbGetCategory(gAddrDB, gCurrentRowID, &sCurrentNumCategories, &sCurrentCategoriesP);
		sCategoryChanged = true;
		PrvDetailsSetCategoryTriggerLabel(formP);
	}

	if (selectedCategoriesP)
		CatMgrFreeSelectedCategories(gAddrDB, &selectedCategoriesP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvDetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  categoryChanged
 *
 * RETURNED:    code which indicates how the to do list was changed,  this
 *              code is send as the update code, in the frmUpdate event.
 *
 * REVISION HISTORY:
 * Name   Date      Description
 * ----   ----      -----------
 * art    6/5/95    Initial Revision
 * kcr    10/9/95   added 'private records' alert
 *
 ***********************************************************************/
static void PrvDetailsApply(FormType* formP)
{
	int16_t		showInListPopupListSel = 0;
	uint32_t	newDispPhoneColID = 0;
	status_t	err = errNone;
	uint32_t	size = 0;
	uint16_t	newPrivateStatus;
	uint16_t	recordAttr = 0;

	// - apply 'Show in list' Popup List selection.

	showInListPopupListSel = LstGetSelection(ToolsGetFrmObjectPtr(formP, DetailsPhoneList));
	newDispPhoneColID = sShowInListPopupListItemsColIdMap[showInListPopupListSel];
	// record the selected columnID
	size = sizeof(uint32_t);
	err = DbWriteColumnValue(gAddrDB, gCurrentRowID, kAddrColumnIDDisplayedPhone,
							0, -1, &newDispPhoneColID, size);
	ErrNonFatalDisplayIf(err < errNone, "Can't write column");

	// - apply 'Private' Check Box state.

	// Get the current setting of the Private checkbox and compare it the
	// the setting of the record.
	// Update the record if the values are different.
	// If the record is being set 'private' for the first time, and the system
	// 'hide secret records' setting is OFF, display an informational alert to the user.
	if ((err = DbGetRowAttr(gAddrDB, gCurrentRowID, &recordAttr)) >= errNone)
	{
		newPrivateStatus = CtlGetValue(ToolsGetFrmObjectPtr(formP, DetailsSecretCheckbox));

		if (((recordAttr & dbRecAttrSecret) == dbRecAttrSecret) != newPrivateStatus)
		{
			if (newPrivateStatus && (gPrivateRecordVisualStatus == showPrivateRecords))
				FrmUIAlert(privateRecordInfoAlert);

			if (newPrivateStatus)
				recordAttr |= dbRecAttrSecret;
			else
				recordAttr &= ~dbRecAttrSecret;

			err = DbSetRowAttr(gAddrDB, gCurrentRowID, &recordAttr);
			ErrNonFatalDisplayIf(err < errNone, "Can't set row attributes");
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvDetailsClose
 *
 * DESCRIPTION: This frees the item allocated for the ShowInList Popup List.
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		11/19/02	Initial revision
 *
 ***********************************************************************/
static void PrvDetailsClose(void)
{
	uint16_t	i;
	FormType *	formP;

	// Free allocated strings.
	for (i = 0; i < kMaxItemNumberInShowList; i++)
	{
		if (sShowInListPopupListItems[i] != NULL)
		{
			MemPtrFree(sShowInListPopupListItems[i]);
			sShowInListPopupListItems[i] = NULL;
		}
	}

	if (sShowInListTriggerLabelP)
	{
		MemPtrFree(sShowInListTriggerLabelP);
		sShowInListTriggerLabelP = NULL;
	}

	if (sSavedCategoriesP)
	{
		DbReleaseStorage(gAddrDB, sSavedCategoriesP);
		sSavedCategoriesP = NULL;
	}

	if (sCurrentCategoriesP)
	{
		DbReleaseStorage(gAddrDB, sCurrentCategoriesP);
		sCurrentCategoriesP = NULL;
	}

	// Restore the focusable state of the EditView. The EditView was set non-focusable
	// to avoid it to give the focus to any table field and consequently to lock the
	// record.
	EditViewSetFocusable(true);

	// Switch back to Interaction mode.
	if ((formP = FrmGetFormPtr(DetailsDialog)) != NULL)
		FrmSetNavState(formP, kFrmNavStateFlagsInteractionMode);
}

/***********************************************************************
 *
 * FUNCTION:    PrvDetailsInit
 *
 * DESCRIPTION: This routine initializes the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 * Name   	Date      		Description
 * ----   	----      		-----------
 * art  	6/5/95      	Initial Revision
 * ppl		02/06/02		Active input area support
 *
 ***********************************************************************/
static void PrvDetailsInit(void)
{
	ControlType *	ctlP;
	ListPtr			showInListPopupListP;
	int16_t			showInListPopupListSel = 0;
	uint16_t		showInListPopupListCount = 0;
	uint16_t		recordAttr = 0;
	uint16_t		tabIndex;
	uint16_t		colPropIndex;
	Boolean			displayAllEntries = false;
	char *			colLabelP = NULL;
	char *			tabNameP = NULL;
	uint32_t		curDispPhoneColID = 0;
	uint32_t		size = 0;
	Boolean			checkCurDispPhoneColID = false;
	FormType *		formP;
	FontID			fontList, curFont;
	uint32_t		strSize;
	Coord			maxListSize = 0;
	Coord			curListSize;
	uint32_t		columnID;
	uint16_t		listIndex;
	RectangleType	listBounds;
	RectangleType	formBounds;
	size_t			length;
	size_t			maxStringLength = 0;
	MemHandle		templateH;
	char *			templateP;

	formP = FrmGetFormPtr(DetailsDialog);

	// - init 'Show in list' Popup List.
	size = sizeof(uint32_t);
	// check if there is already a column value for the 'DisplayedPhone' column for the given record.
	if (DbCopyColumnValue(gAddrDB, gCurrentRowID, kAddrColumnIDDisplayedPhone, 0, &curDispPhoneColID, &size) >= errNone)
		checkCurDispPhoneColID = true;

	listIndex = FrmGetObjectIndex (formP, DetailsPhoneList);
	showInListPopupListP = FrmGetObjectPtr(formP, listIndex);
	fontList = LstGetFont (showInListPopupListP);
	curFont = FntSetFont(fontList);

	// Load the ShowInList template
	templateH = DmGetResource(gApplicationDbP, strRsc, ShowInListTemplate);
	templateP = MemHandleLock(templateH);

	// the list is filled with non empty phone (Type 'Phone' or (Type 'internet' and Kind 'e-mail') fields
	// label and tab name of the record.
	// if all the phone fields of the record are empty, fill the list with all the phone field labels.
	do
	{
		for (tabIndex = 0; tabIndex < gBookInfo.numTabs; tabIndex++)
		{
			for (colPropIndex = 0; colPropIndex < gBookInfo.tabs[tabIndex].numElement; colPropIndex++)
			{
				// Skip non-tel/non-mail entries
				if (! ToolsIsPhoneFieldByIndex(tabIndex, colPropIndex, false))
					continue;

				columnID = gBookInfo.tabs[tabIndex].columnsPropertiesP[colPropIndex].columnId;
				colLabelP = NULL;

				strSize = 0;
				if (displayAllEntries || (DbCopyColumnValue(gAddrDB, gCurrentRowID, columnID, 0, NULL, &strSize) >= errNone && strSize))
				{
					// check if a value exist for this colID in the current record.
					// Get Column label
					colLabelP = gBookInfo.tabs[tabIndex].columnsPropertiesP[colPropIndex].renamedLabelP;
					if (!colLabelP)
						colLabelP = gBookInfo.tabs[tabIndex].columnsPropertiesP[colPropIndex].labelP;

					// Get Tab name
					tabNameP = gBookInfo.tabs[tabIndex].tabInfo.tabNameP;
				}

				// We have something to add and the add doesn't failed
				if (colLabelP && PrvAllocAndSetShowInListPopupListItem(showInListPopupListCount, templateP, tabNameP, colLabelP, columnID))
				{
					if (checkCurDispPhoneColID && curDispPhoneColID == columnID)
						showInListPopupListSel = showInListPopupListCount;

					length = strlen(sShowInListPopupListItems[showInListPopupListCount]);
					if (length > maxStringLength)
						maxStringLength = length;

					curListSize = FntCharsWidth(sShowInListPopupListItems[showInListPopupListCount], length) + 5;	// + 3  = textInsetWidth * 2 + 1;
					if (curListSize > maxListSize)
						maxListSize = curListSize;

					showInListPopupListCount++;
				}
			} // for colPropIndex

			if (showInListPopupListCount >= kMaxItemNumberInShowList)
				break;
		} // for tabIndex

		// Stop if all entries have been added to the list
		if (displayAllEntries)
			break;

		// No field found in the record. Go for another loop and fill the list
		// with all entries.
		if (showInListPopupListCount == 0)
			displayAllEntries = true;

	} while (showInListPopupListCount == 0);

	MemHandleUnlock(templateH);
	DmReleaseResource(templateH);

	FntSetFont(curFont);

	// Compute the max size available for the list.
	FrmGetFormInitialBounds(formP, &formBounds);
	FrmGetObjectBounds(formP, listIndex, &listBounds);
//	curListSize = formBounds.extent.x - (listBounds.topLeft.x / 2);

	// Resize the List with the minimum size between the largest string and the form bounds
//	listBounds.extent.x = (maxListSize < curListSize)? maxListSize : curListSize;
	listBounds.extent.x = maxListSize;
	FrmSetObjectBounds (formP, listIndex, &listBounds);

	sShowInListTriggerLabelP = MemPtrNew((uint32_t)(maxStringLength + 1));
	ErrFatalDisplayIf(sShowInListTriggerLabelP == NULL, "Out of memory!");

	// Compute the max label width, based on the list topLeft bound
	sShowInListTriggerLabelMaxWidth = formBounds.extent.x - listBounds.topLeft.x - kPopupTriggerIndicatorWidth;

	// Initialize the list
	LstSetHeight(showInListPopupListP, showInListPopupListCount);
	LstSetListChoices(showInListPopupListP, sShowInListPopupListItems, showInListPopupListCount);
	LstSetSelection(showInListPopupListP, showInListPopupListSel);
	LstMakeItemVisible (showInListPopupListP, showInListPopupListSel);
	FntTruncateString(sShowInListTriggerLabelP, LstGetSelectionText(showInListPopupListP, showInListPopupListSel), fontList, sShowInListTriggerLabelMaxWidth, true);
	CtlSetLabel(ToolsGetFrmObjectPtr(formP, DetailsPhoneTrigger), sShowInListTriggerLabelP);

	// - init 'Category' Categories List.

	// Get record categories. Restore them if user cancel.
	DbGetCategory(gAddrDB, gCurrentRowID, &sCurrentNumCategories, &sCurrentCategoriesP);
	DbGetCategory(gAddrDB, gCurrentRowID, &sSavedNumCategories, &sSavedCategoriesP);

	// Set the label of the category trigger.
	PrvDetailsSetCategoryTriggerLabel(formP);

	// - init 'Private' Check Box.

	// if the record is mark as secret, turn on the 'private' checkbox.
	DbGetRowAttr(gAddrDB, gCurrentRowID, &recordAttr);
	ctlP = ToolsGetFrmObjectPtr(formP, DetailsSecretCheckbox);
	CtlSetValue(ctlP, (int16_t)(recordAttr & dbRecAttrSecret));

	sCategoryChanged = false;
	sCurrentNavObjectID = frmInvalidObjectId;

	ToolsFrmInvalidateWindow(DetailsDialog);
}

/***********************************************************************
 *
 * FUNCTION:    PrvDetailsDoControl
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box" for controls.
 *
 * PARAMETERS:  controlID  - ID of the control to handle
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		11/19/02	Initial revision
 * LFe		07/21/04	Move from HandleEvent to this function
 *
 ***********************************************************************/
static Boolean PrvDetailsDoControl(uint16_t controlID)
{
	Boolean		handled = false;
	FormType *	formP;
	status_t	err;

	switch (controlID)
	{
		case DetailsOkButton:
			PrvDetailsApply(FrmGetFormPtr(DetailsDialog));
			PrvDetailsClose();

			FrmReturnToForm(0);

			// If categories have changed, post a user event for the EditView
			if (sCategoryChanged)
				PrvPostUserEvent(kFrmDetailsUpdateEvent);

			handled = true;
			break;

		case DetailsCancelButton:
			// If the categories have changed restore the original ones.
			// We need to check that the original categories have not
			// been deleted or merged
			if (sCategoryChanged)
			{
				// Check categories before restoring them
				ToolsCheckCategories(sSavedCategoriesP, &sSavedNumCategories);

				// Free memory if there are no more valid category IDs
				if (sSavedNumCategories == 0 && sSavedCategoriesP != NULL)
				{
					DbReleaseStorage(gAddrDB, sSavedCategoriesP);
					sSavedCategoriesP = NULL;
				}

                err = DbSetCategory(gAddrDB, gCurrentRowID, sSavedNumCategories, sSavedCategoriesP);
				ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");

				// Update the category trigger in the EditView
				PrvPostUserEvent(kFrmDetailsUpdateEvent);
			}

			PrvDetailsClose();

			FrmReturnToForm(0);

			handled = true;
			break;

		case DetailsDeleteButton:
			if (ToolsConfirmDeletion())
			{
				// Delete record
				AddressDBDeleteRecord(gSaveBackup);

				// Cleanup
				PrvDetailsClose();

				// Delete form
				formP = FrmGetFormPtr(DetailsDialog);
				FrmEraseForm(formP);
				FrmDeleteForm(formP);
				FrmCloseAllForms();
				FrmGotoForm(gApplicationDbP, ListView);
			}

			handled = true;
			break;

		case DetailsNoteButton:
			AddressEditNote(gCurrentRowID);
			PrvDetailsApply(FrmGetFormPtr(DetailsDialog));
			PrvDetailsClose();
			FrmReturnToForm(0);

			// If categories have changed, post a user event for the EditView
			if (sCategoryChanged)
				PrvPostUserEvent(kFrmDetailsUpdateEvent);

			handled = true;
			break;

		case DetailsCategoryTrigger:
			PrvDetailsSelectCategory();
			handled = true;
			break;

	} // switch (eventP->data.ctlSelect.controlID)

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    PrvDetailsHandleKeydown
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box" of the Address application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		07/21/04	Initial revision
 *
 ***********************************************************************/
static Boolean PrvDetailsHandleKeydown(EventType * eventP)
{
	if (EvtKeydownIsVirtual(eventP))
	{
 		if (eventP->data.keyDown.chr == vchrRockerRight)
 		{
			switch(sCurrentNavObjectID)
			{
				case DetailsCategoryTrigger:
				case DetailsPhoneTrigger:
				case DetailsSecretCheckbox:
					FrmNavObjectTakeFocus (FrmGetFormPtr(DetailsDialog), DetailsOkButton);
					return true;
			}
 		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box" of the Address application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		11/19/02	Initial revision
 *
 ***********************************************************************/
Boolean DetailsHandleEvent(EventType * eventP)
{
	Boolean		handled = false;
	FontID		fontID;
	char *		itemTextP;
	FormType *	formP;

	switch (eventP->eType)
	{
		case frmOpenEvent:
			PrvDetailsInit();
			handled = true;
			break;

		case frmCloseEvent:
			PrvDetailsClose();
			break;

		case ctlSelectEvent:
			handled = PrvDetailsDoControl(eventP->data.ctlSelect.controlID);
			break;

		case popSelectEvent:
			formP = FrmGetFormPtr(DetailsDialog);
			fontID = LstGetFont(eventP->data.popSelect.listP);
			itemTextP = LstGetSelectionText(eventP->data.popSelect.listP, eventP->data.popSelect.selection);

			FntTruncateString(sShowInListTriggerLabelP, itemTextP, fontID, sShowInListTriggerLabelMaxWidth, true);
			CtlSetLabel(eventP->data.popSelect.controlP, sShowInListTriggerLabelP);
			handled = true;
			break;

		case keyDownEvent:
			handled = PrvDetailsHandleKeydown(eventP);
			break;

		case frmObjectFocusTakeEvent:
			sCurrentNavObjectID = eventP->data.frmObjectFocusTake.objectID;
			break;

	} // switch (eventP->eType)

	return handled;
}
