/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AppMain.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "AppResources.h"


/***********************************************************************
 *
 *	Entry Points
 *
 ***********************************************************************/


/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/
typedef struct AppPrefsType {
	uint32_t 	replaceme;
} AppPrefsType;




/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
// register Creator ID at http://www.palmos.com/dev/creatorid/
#define kAppCreatorID			'STRT'

#define kAppPrefsID				0x00
#define kAppPrefsVersion		0x01
#define kSyncedPrefsDB			true
#define kUnsyncedPrefsDB		false


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
DmOpenRef	gAppDB;		// OS6NEW: Application database pointer

// These arrays allow the Form Manager to automatically reposition form
// objects on receipt of winResizedEvents.
FormLayoutType gMainFormLayout[] = {
	{ sizeof(FormLayoutType), MainForm2Button, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainDialogButton, 0, frmFollowBottom },
	{ 0,0,0,0 }		// Last line must be all NULLs
};

FormLayoutType gForm2FormLayout[] = {
	{ sizeof(FormLayoutType), Form2DoneButton, 0, frmFollowBottom },
	{ 0,0,0,0 }		// Last line must be all NULLs
};


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
 * DESCRIPTION:		In Palm OS 6, drawing should only occur during update
 *					events. This routine forces a winUpdateEvent for the
 *					specified window.
 *
 * PARAMETERS:	->	uwFormID - the ID of the form to be invalidate
 *
 * RETURNED:    	nothing.
 *
 * REVISION HISTORY:
 *
 *		jbp		6/2/03	Initial Version
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
 * FUNCTION:    DisplayAboutDialog
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *		jbp		6/2/03	Initial Version
 *
 ***********************************************************************/
static void DisplayAboutDialog(void)
{
	FormType	*pForm;

	// OS6NEW: FrmInitForm() needs app db ptr as first arg now
	pForm = FrmInitForm(gAppDB, AboutForm);
	FrmDoDialog(pForm);					// Display the About Box.
	FrmDeleteForm(pForm);
}


/***********************************************************************
 *
 * FUNCTION:    DisplayTheDialog
 *
 * DESCRIPTION: Display a modal dialog.
 *
 * PARAMETERS:  none.
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *
 *		jbp		6/2/03	Initial Version
 *
 ***********************************************************************/
static void DisplayTheDialog(void)
{
	FormType	*pForm;
	uint16_t	uwButtonHit;

	// OS6NEW: FrmInitForm() needs app db ptr as first arg now
	pForm = FrmInitForm(gAppDB, DialogForm);

	// You can initialize form objects here, but do not use
	// FrmGetActiveForm() - the dialog is not yet the active form.


	// Display the dialog - OS will handle basic events.
	uwButtonHit = FrmDoDialog(pForm);

	// Do the appropriate thing if the user taps 'OK'
	if (uwButtonHit == DialogOKButton) {


	}

	// Hide form and delete form data structure.
	FrmDeleteForm(pForm);
}


/***********************************************************************
 *
 * FUNCTION:    Form2FormOpenEvent
 *
 * DESCRIPTION: This routine initializes the Form2 form.
 *
 * PARAMETERS:  pForm - pointer to the Form2 form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void Form2FormOpenEvent(FormType * pForm)
{
	// OS6NEW: don't draw - invalidate and let OS redraw in
	// response to winUpdateEvent.
	PrvInvalidateFormWindow(Form2Form);
}


/***********************************************************************
 *
 * FUNCTION:    Form2FormMenuEvent
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
static Boolean Form2FormCtlSelectEvent(EventType *pEvent)
{
	Boolean		fHandled = false;

	switch (pEvent->data.ctlSelect.controlID ) {
		case Form2DoneButton:
			FrmGotoForm(gAppDB, MainForm);
			fHandled = true;
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    Form2FormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "Form2Form" of this application.
 *
 * PARAMETERS:  pEvent  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean Form2FormHandleEvent(EventType* pEvent)
{
	Boolean 	fHandled = false;
	FormType* 	pForm;

	switch (pEvent->eType) {
		case frmOpenEvent:
			pForm = FrmGetFormPtr(Form2Form);
			Form2FormOpenEvent(pForm);
			fHandled = true;
			break;

		case ctlSelectEvent:
			fHandled = Form2FormCtlSelectEvent(pEvent);
			break;

		default:
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormOpenEvent
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:  pForm - pointer to the MainForm form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void MainFormOpenEvent(FormType * pForm)
{
	// OS6NEW: don't draw - invalidate and let OS redraw in
	// response to winUpdateEvent.
	PrvInvalidateFormWindow(MainForm);
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
static Boolean MainFormMenuEvent(uint16_t uwMenuCmdID)
{
	Boolean		fHandled = false;

	switch (uwMenuCmdID) {
		case MainOptionsAboutBasicApp:
			DisplayAboutDialog();
			fHandled = true;
			break;
	}

	return fHandled;
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
static Boolean MainFormCtlSelectEvent(EventType *pEvent)
{
	Boolean		fHandled = false;

	switch (pEvent->data.ctlSelect.controlID ) {
		case MainForm2Button:
			FrmGotoForm(gAppDB, Form2Form);
			fHandled = true;
			break;

		case MainDialogButton:
			DisplayTheDialog();
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
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventType* pEvent)
{
	Boolean 	fHandled = false;
	FormType* 	pForm;

	switch (pEvent->eType) {
		case frmOpenEvent:
			pForm = FrmGetFormPtr(MainForm);
			MainFormOpenEvent(pForm);
			fHandled = true;
			break;

		case ctlSelectEvent:
			fHandled = MainFormCtlSelectEvent(pEvent);
			break;

		case menuEvent:
			fHandled = MainFormMenuEvent(pEvent->data.menu.itemID);
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
 * REVISION HISTORY:
 *
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
		pForm = FrmInitForm(gAppDB, formId);
		FrmSetActiveForm(pForm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId) {
			case MainForm:
				FrmSetEventHandler(pForm, MainFormHandleEvent);
				FrmInitLayout(pForm, gMainFormLayout);
				break;

			case Form2Form:
				FrmSetEventHandler(pForm, Form2FormHandleEvent);
				FrmInitLayout(pForm, gForm2FormLayout);
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
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     status_t value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static status_t AppStart(void)
{

#if 0
	AppPrefsType prefs;
	// OS6NEW: PrefGetAppPreferences() size is now 32-bit
	uint32_t	prefsSize;

	// Read the saved preferences / saved-state information.
	prefsSize = sizeof(AppPrefsType);
	if (PrefGetAppPreferences(kAppCreatorID, kAppPrefsID, &prefs, &prefsSize,
			kSyncedPrefsDB) != noPreferenceFound) {
			// Init globals, etc.
	}
#endif

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
#if 0
	AppPrefsType prefs;

	// Write the saved preferences / saved-state information.  This data
	// will saved during a HotSync backup.
	PrefSetAppPreferences(kAppCreatorID, kAppPrefsID, kAppPrefsVersion,
		&prefs, sizeof(prefs), kSyncedPrefsDB);
#endif

	// Close all the open forms.
	FrmCloseAllForms();
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

		// OS6NEW: error codes are now status_t (32 bit), not Err (16 bit)
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
 *		6/2/03	jb	Initial version
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	// OS6NEW: New error type
	status_t error = errNone;

	// OS6NEW: Get app database ref - needed for Form Mgr calls, resources.
	if ((error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gAppDB)) < errNone)
		return error;

	// Handle launch code
	switch (cmd) {
		case sysAppLaunchCmdNormalLaunch:
			// Perform app initialization.
			error = AppStart();
			if (error)
				return error;

			// OS6NEW: FrmGotoForm() now requires app db ref argument
			FrmGotoForm(gAppDB, MainForm);

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
