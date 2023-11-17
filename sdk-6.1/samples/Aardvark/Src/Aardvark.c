/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Aardvark.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	 	This app demonstrates how to reposition form objects in response to
 *		winResizedEvents. It also shows how to properly draw forms under
 *		Palm OS 6 using the update-based drawing model.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "AppResources.h"


/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

typedef struct {
	uint16_t uwReserved1;
} AppPrefsType;



/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/

DmOpenRef			gAppDB;				// OS6NEW: App database pointer

// Form layout data for automated adjustment by OS of form objects.
// The array must end with a nulled object.
// Note the use of 'frmLayoutGraffitiShiftID' for the GSI.

FormLayoutType gMainFormLayout[] = {
	{ sizeof(FormLayoutType), MainAddButton, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainLookupField, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainFindLabel, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainItemsList, 0, frmFollowTopBottom },
	{ sizeof(FormLayoutType), frmLayoutGraffitiShiftID, 0, frmFollowBottom },
	{ 0,0,0,0 }
};


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define appFileCreator			'AA3d'
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/


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
#if 0
static void * PrvGetObjectPtr(FormType *pForm, uint16_t uwObjectID)
{
	uint16_t	uwObjIndex;

	if (pForm == NULL)
		return NULL;

	uwObjIndex = FrmGetObjectIndex(pForm, uwObjectID);

	return FrmGetObjectPtr(pForm, uwObjIndex);
}
#endif

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
 * REVISION HISTORY:
 *
 *	TEs	03/18/02	Initial Version
 *
 ******************************************************************************/
static void PrvInvalidateFormWindow(uint16_t uwFormID)
{
	WinHandle	hWindow;
	FormType *	pForm;

	//TraceOutput(TL(appErrorClass, "Enter PrvInvalidateFormWindow"));

	if ((pForm = FrmGetFormPtr(uwFormID)) == NULL)
		return;

	if ((hWindow = FrmGetWindowHandle(pForm)) == NULL)
		return;

	//TraceOutput(TL(appErrorClass, "Invalidate window for formID %d", uwFormID));

	WinInvalidateWindow(hWindow);
}


/***********************************************************************
 *
 * FUNCTION:    PrvMoveFormObject
 *
 * DESCRIPTION: Reposition form objects vertically
 *				when the input area collapses/expands.
 *
 * PARAMETERS:	pForm - pointer to form that holds objects
 *				uwObjID - ID of object to move
 *				wDeltaY - amount to shift object up (> 0) or down (< 0)
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 ************************************************************************/
#if 0
static void PrvMoveFormObject(FormType *pForm, uint16_t uwObjID, int16_t wDeltaY)
{
	uint16_t		uwObjIndex;
	RectangleType	rect;

	if (pForm == NULL)
		return;

	// The Add button
	uwObjIndex = FrmGetObjectIndex(pForm, uwObjID);
	FrmGetObjectBounds(pForm, uwObjIndex, &rect);
	rect.topLeft.y += wDeltaY;
	FrmSetObjectBounds(pForm, uwObjIndex, &rect);
}
#endif

/***********************************************************************
 *
 * FUNCTION:    MainFormOpenEvent
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:  frm - pointer to the MainForm form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void MainFormOpenEvent(FormType *pForm)
{
	// Set insertion point in text field (even though
	// we don't use the field in this example).
	FrmSetFocus(pForm, FrmGetObjectIndex(pForm, MainLookupField));

	// OS6NEW: don't draw - invalidate
	PrvInvalidateFormWindow(MainForm);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormWinResizedEvent
 *
 * DESCRIPTION: This routine resize the form and its objects
 *				when the input area collapses/expands. See the
 *				Memo Pad sample code for a more complete example.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *
 ************************************************************************/
static void MainFormWinResizedEvent(FormType *pForm, RectangleType *newBoundsP)
{
	uint16_t		objIndex;
	int16_t			offset;
	RectangleType	rOrigBounds, rObj;
	Coord			height;

	// Get the new window height
	height = newBoundsP->extent.y;

	FrmGetFormInitialBounds(pForm, &rOrigBounds);

	// Get delta between the old window bottom and the new window bottom.
	offset =  height - rOrigBounds.extent.y;
	if (offset == 0)
		return; // No need to resize

	// The Graffiti Shift Indicator isn't accessible via
	// FrmGetObjectIndex() because it no longer has an object ID,
	// so loop through all objects on the form and get the index that way.
	objIndex = FrmGetNumberOfObjects(pForm);
	while (objIndex--) {
		if (FrmGetObjectType(pForm, objIndex) == frmGraffitiStateObj) {
			FrmGetObjectBounds(pForm, objIndex, &rObj);
			rObj.topLeft.y += offset;
			FrmSetObjectBounds(pForm, objIndex, &rObj);
			break;
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    MainFormMenuEvent
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormMenuEvent(uint16_t command)
{
	Boolean handled = false;
	FormType * pForm;

	switch (command) {
		case MainOptionsAboutAardvark:
			pForm = FrmInitForm(gAppDB, AboutForm);
			FrmDoDialog(pForm);
			FrmDeleteForm(pForm);
			handled = true;
			break;

	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: Handle all form events
 *
 * PARAMETERS:  pEvent  - a pointer to an EventType structure
 *
 * RETURNED:   	true if event should not be passed to default OS
 *				event handler; else, false.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventType * pEvent)
{
	Boolean 	handled = false;
	FormType * 	pForm;

	switch (pEvent->eType) {
		case frmOpenEvent:
			pForm = FrmGetFormPtr(MainForm);
			MainFormOpenEvent(pForm);
			handled = true;
			break;

		case winUpdateEvent:
			// If not handled here (return false), OS will call FrmDrawForm().
			break;

		case winResizedEvent:
			pForm = FrmGetFormPtr(MainForm);
			// Let the OS move the form objects...
			FrmPerformLayout(pForm, &pEvent->data.winResized.newBounds);

			// ...but it won't move the GSI, so we have to. We can also take
			// care of other objects here, such as tables (add rows).
			MainFormWinResizedEvent(pForm, &pEvent->data.winResized.newBounds);
			handled = true;
			break;

		case ctlSelectEvent:
			if (pEvent->data.ctlSelect.controlID == MainAddButton) {
				FrmAlert(gAppDB, PlaceholderAlert);
				handled = true;
			}
			break;

		case menuEvent:
			handled = MainFormMenuEvent(pEvent->data.menu.itemID);
			break;

		default:
			break;

	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static status_t AppStart(void)
{
	AppPrefsType 	prefs;
	uint32_t 		prefsSize;

	// Read the saved preferences / saved-state information.
	// OS6NEW: PrefGetAppPreferences() size is now 32-bit
	prefsSize = sizeof(AppPrefsType);
	if (PrefGetAppPreferences(appFileCreator, appPrefID, &prefs, &prefsSize, true)
				!= noPreferenceFound) {

	}

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppStop(void)
{
	AppPrefsType prefs;

	// Write the saved preferences / saved-state information.  This data
	// will saved during a HotSync backup.
	PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersionNum,
		&prefs, sizeof (prefs), true);

	// Close all the open forms.
	FrmCloseAllForms ();
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
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventType * pEvent)
{
	uint16_t	uwFormID;
	FormType* 	pForm;

	if (pEvent->eType == frmLoadEvent) {
		uwFormID = pEvent->data.frmLoad.formID;
		pForm = FrmInitForm(gAppDB, uwFormID);
		FrmSetActiveForm(pForm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (uwFormID) {
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
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	status_t	error;
	EventType	event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (SysHandleEvent(&event))
			continue;

		if (MenuHandleEvent(0, &event, &error))
			continue;

		if (AppHandleEvent(&event))
			continue;

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
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t	status;

	// OS6NEW: Get app database ref - needed for Form Mgr calls now.
	if ((status = SysGetModuleDatabase(SysGetRefNum(), NULL, &gAppDB)) < errNone)
		return status;

	switch (cmd) {
		case sysAppLaunchCmdNormalLaunch:
			status = AppStart();
			if (status < errNone)
				return status;

			FrmGotoForm(gAppDB, MainForm);
			AppEventLoop();
			AppStop();
			break;

		default:
			break;

	}

	return errNone;
}
