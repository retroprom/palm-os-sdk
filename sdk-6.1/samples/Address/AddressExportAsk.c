/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressExportAsk.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the Address Book application's Export Ask dialog
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <UIResources.h>
#include <ErrorMgr.h>
#include <TraceMgr.h>

#include "AddressExportAsk.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressRsc.h"
#include "AddressPrefs.h"

/***********************************************************************
 *
 * FUNCTION:    PrvAddressExportHandleEvent
 *
 * DESCRIPTION: Handle event function to manage the checkbox depending of
 *				popup value.
 *
 * PARAMETERS:  event:	System Event.
 *
 * RETURNED:    Event handled or not.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		04/22/03	Initial revision
 *
 ***********************************************************************/
static Boolean PrvAddressExportHandleEvent(EventType * event)
{

	if ((event->eType == popSelectEvent) &&
		(event->data.popSelect.listID == ExportAskList) &&
		(event->data.popSelect.selection != event->data.popSelect.priorSelection))
	{
		FormPtr		formP = FrmGetFormPtr(ExportAskDialog);
		ControlPtr	noteCheck;

		noteCheck = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, ExportAskAddNoteCheckbox));
		gTransfertNote[event->data.popSelect.priorSelection] = (Boolean) (CtlGetValue(noteCheck) ? true : false);
		CtlSetValue(noteCheck, gTransfertNote[event->data.popSelect.selection] ? 1 : 0);
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvExportAskApply
 *
 * DESCRIPTION: This routine applies the changes made in the Export Ask Dialog.
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * Name   Date      Description
 * ----   ----      -----------
 * CGu		11/19/02	Initial revision
 *
 ***********************************************************************/
static uint32_t PrvAddressExportDisplayDialog(uint16_t currentTabId)
{
	FormType*	frmP;
	uint32_t	transfertMask;
	ListType*	listP;
	int16_t		value;
	ListPtr		exportPartListP;
	ControlPtr	noteCheck;

	frmP = FrmInitForm(gApplicationDbP, ExportAskDialog);

	// Find the current family of the current Tab.
	transfertMask = AddressTabFindFamilyOfTabId(currentTabId);

	// Initialize the popup with this family
	value = 0;	// All
	switch (transfertMask)
	{
		case kFieldFamily_Home:		value = 1;	break;
		case kFieldFamily_Corp:		value = 2;	break;
		case kFieldFamily_Other:	value = 3;	break;
	}

	exportPartListP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, ExportAskList));
	LstSetSelection (exportPartListP, value);
	CtlSetLabel(FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, ExportAskPopTrigger)), LstGetSelectionText(exportPartListP, value));

	noteCheck = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, ExportAskAddNoteCheckbox));
	CtlSetValue(noteCheck, gTransfertNote[value] ? 1 : 0);

	// Reset Family mask value
	transfertMask = 0;

	FrmSetEventHandler(frmP, PrvAddressExportHandleEvent);

	if (FrmDoDialog(frmP) == ExportAskOkButton)
	{
		listP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, ExportAskList));

		switch (LstGetSelection(listP))
		{
			case 0:		transfertMask = kFieldFamilyMask;	break;
			case 1:		transfertMask = kFieldFamily_Home;	break;
			case 2:		transfertMask = kFieldFamily_Corp;	break;
			case 3:		transfertMask = kFieldFamily_Other;	break;
		}

		// Not really optimized C writing, but it removes a lot of warnings/castings :)
		gTransfertNote[value] = (Boolean) (CtlGetValue(noteCheck) ? true : false);

		if (gTransfertNote[value])
			transfertMask |= kFieldType_Binary | kFieldKind_Note;
	}

	FrmDeleteForm(frmP);

	return transfertMask;
}

/***********************************************************************
 *
 * FUNCTION:    PrvExportAskApply
 *
 * DESCRIPTION: This routine applies the changes made in the Export Ask Dialog.
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * Name   Date      Description
 * ----   ----      -----------
 * CGu		11/19/02	Initial revision
 *
 ***********************************************************************/
uint32_t AddressExportGetFamily(uint16_t currentTabId)
{
	uint32_t	transfertMask = 0;

	switch(gTransfertMode)
	{
		case kPrefTransfertAllRecord:
			transfertMask = kFieldFamilyMask | kFieldType_Binary | kFieldKind_Note;
			break;

		case kPrefTransfertCurrentTabRecord:
			transfertMask = AddressTabFindFamilyOfTabId(currentTabId);

			// Attach the Note if the current tab is 'All'
			if (transfertMask == kFieldFamilyMask)
				transfertMask |= kFieldType_Binary | kFieldKind_Note;

			break;

		case kPrefTransfertAskRecord:
			transfertMask = PrvAddressExportDisplayDialog(currentTabId);
			break;
	}

	return transfertMask;
}
