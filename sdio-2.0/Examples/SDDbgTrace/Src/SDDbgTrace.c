/******************************************************************************
 *
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Starter.c
 *
 * Release: palmOne SDIO SDK 2.0
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "SDDbgTraceRsc.h"
#include <VFSMgr.h>
#include <SDIO.h>


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
typedef struct
	{
	UInt8 replaceme;
	} StarterPreferenceType;

typedef struct
	{
	UInt8 replaceme;
	} StarterAppInfoType;

typedef StarterAppInfoType* StarterAppInfoPtr;


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
static	SDIODebugOptionType	CurrentTraceOption;	// Current trace option selected
static	Int16	CurrentTraceOptionIndex;	// index in list of current trace option
static	Char	textNotSupported[] = "Not supported";


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define appFileCreator			'SDdt'	// register your own at http://www.palmos.com/dev/creatorid/
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

// Define the minimum OS version we support (4.0).
#define ourMinVersion	sysMakeROMVersion(4,0,0,0,0)
#define kPalmOS10Version	sysMakeROMVersion(1,0,0,sysROMStageRelease,0)


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:    SdioDebugOptionsSet
 *
 * DESCRIPTION: This routine is used to set the SD/SDIO Debug Slot
 *              Driver's trace options.
 *
 * PARAMETERS:  debugOption - trace option to be set
 *
 * RETURNED:    errNone	- success
 *				expErrNotOpen	- unable to set debug options.
 *
 ***********************************************************************/
static Err SdioDebugOptionsSet( SDIODebugOptionType debugOption )
{
	Err err = errNone;
	UInt32 slotIterator;
	UInt16 slotRefNum;
	UInt16 slotLibRefNum;
	UInt32 mediaType;
	UInt16 count=0;

	// Check each slot
	slotIterator = expIteratorStart;
	while( slotIterator != expIteratorStop )
	{
		err =  ExpSlotEnumerate( &slotRefNum, &slotIterator );
		if ( err )
		{
			break;
		}
		// Find the slot driver for this slot
		err = ExpSlotLibFind(slotRefNum,  &slotLibRefNum);
		if ( !err )
		{
			err = SlotMediaType( slotLibRefNum, slotRefNum, &mediaType );
			if ( !err )
			{
				// Is this Slot Driver an SD slot driver?
				if ( mediaType == expMediaType_SecureDigital )
				{
					// Set the debug trace options
					err = SDIODebugOptions( slotLibRefNum, &debugOption );
					if ( err == errNone )
						count++;
				}
			}
		}
	}
	if ( count == 0 )
		err = expErrNotOpen;
	return( err );
}


/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h
 *                                for format)
 *              launchFlags     - flags that indicate if the application
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
		{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			FrmAlert (RomIncompatibleAlert);
		
			// Palm OS 1.0 will continuously relaunch this app unless we switch to
			// another safe one.
			if (romVersion <= kPalmOS10Version)
				{
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
				}
			}
		
		return sysErrRomIncompatible;
		}

	return errNone;
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
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void * GetObjectPtr(UInt16 objectID)
{
	FormPtr frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}


/***********************************************************************
 *
 * FUNCTION:    MainFormInit
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
static void MainFormInit(FormPtr /*frmP*/)
{
	Err err = errNone;
	
	CurrentTraceOption = sdioDebugOptionTraceNone;
	CurrentTraceOptionIndex = 0;
	
	// Initialize the SD/SDIO Debug Tracing feature.
	err = SdioDebugOptionsSet( CurrentTraceOption );
	if( err ) {
		// Set the label of the popup trigger.
		CtlSetLabel(GetObjectPtr(MainSDIOTracePopTrigger), textNotSupported);
		goto Exit;
	}
	
	// Set the label of the popup trigger.
	CtlSetLabel(GetObjectPtr(MainSDIOTracePopTrigger), LstGetSelectionText(GetObjectPtr(MainTraceOptionsList), CurrentTraceOptionIndex));
Exit:
	return;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
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
static Boolean MainFormDoCommand(UInt16 command)
{
	Boolean handled = false;
	FormPtr frmP;

	switch (command)
		{
		case MainOptionsAboutSDIOTrace:
			MenuEraseStatus(0);					// Clear the menu status from the display.
			frmP = FrmInitForm (AboutForm);
			FrmDoDialog (frmP);					// Display the About Box.
			FrmDeleteForm (frmP);
			handled = true;
			break;

		}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr eventP)
{
	Boolean handled = false;
	FormPtr frmP;
	Int16	itemSelected;
	Err err = errNone;
	
	switch (eventP->eType)
		{
		case	ctlSelectEvent:
			// Trace Option Pop Trigger.
			if (eventP->data.ctlSelect.controlID == MainSDIOTracePopTrigger)
			{
				// Get a pointer to the form
				frmP = FrmGetActiveForm();
				
				// Popup the Timeout selector list
				if((itemSelected = LstPopupList(GetObjectPtr(MainTraceOptionsList))) >= 0 )
				{
					CurrentTraceOptionIndex = itemSelected;
					switch( CurrentTraceOptionIndex )
					{
						case	1:
							CurrentTraceOption = sdioDebugOptionTraceCmds;
							break;
						case	2:
							CurrentTraceOption = sdioDebugOptionTraceRejection;
							break;
						case	3:
							CurrentTraceOption = sdioDebugOptionTraceCmdData;
							break;
						case	4:
							CurrentTraceOption = sdioDebugOptionTraceContents;
							break;
						case	5:
							CurrentTraceOption = sdioDebugOptionTraceProgress;
							break;
						case	6:
							CurrentTraceOption = sdioDebugOptionTraceMost;
							break;
						case	7:
							CurrentTraceOption = sdioDebugOptionTraceAll;
							break;
						case	8:
						default:
							CurrentTraceOption = sdioDebugOptionTraceNone;
							break;
					}
					
					// Initialize the SD/SDIO Debug Tracing feature.
					err = SdioDebugOptionsSet( CurrentTraceOption );
					if( err ) {
						// Set the label of the popup trigger.
						CtlSetLabel(GetObjectPtr(MainSDIOTracePopTrigger), textNotSupported);
					}
					else {
						// Set the label of the timeout trigger.
						CtlSetLabel(GetObjectPtr(MainSDIOTracePopTrigger), LstGetSelectionText(GetObjectPtr(MainTraceOptionsList), CurrentTraceOptionIndex));
					}
				}
			}
			handled = true;
			break;
			
		case menuEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			MainFormInit( frmP);
			FrmDrawForm ( frmP);
			handled = true;
			break;
			
		case frmUpdateEvent:
			// To do any custom drawing here, first call FrmDrawForm(), then do your
			// drawing, and then set handled to true.
			break;

		default:
			break;
		
		}
	
	return handled;
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
static Boolean AppHandleEvent(EventPtr eventP)
{
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			default:
//				ErrFatalDisplay("Invalid Form Load Event");
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
	UInt16 error;
	EventType event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
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
static Err AppStart(void)
{
	StarterPreferenceType prefs;
	UInt16 prefsSize;

	// Read the saved preferences / saved-state information.
	prefsSize = sizeof(StarterPreferenceType);
	if (PrefGetAppPreferences(appFileCreator, appPrefID, &prefs, &prefsSize, true) !=
		noPreferenceFound)
		{
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
	StarterPreferenceType prefs;

	// Write the saved preferences / saved-state information.  This data
	// will saved during a HotSync backup.
	PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersionNum,
		&prefs, sizeof (prefs), true);
		
	// Close all the open forms.
	FrmCloseAllForms ();
}


/***********************************************************************
 *
 * FUNCTION:    StarterPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static UInt32 StarterPalmMain(UInt16 cmd, MemPtr /*cmdPBP*/, UInt16 launchFlags)
{
	Err error;

	error = RomVersionCompatible (ourMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error)
				return error;
				
			FrmGotoForm(MainForm);
			AppEventLoop();
			AppStop();
			break;

		default:
			break;

		}
		
	return errNone;
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
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return StarterPalmMain(cmd, cmdPBP, launchFlags);
}
