/*
 * JotCtrlMain.c
 *
 * main file for JotCtrl
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#include <PalmOS.h>
#include <StdIOPalm.h>

#define NON_PORTABLE 1

#include "WristPDA.h"

#include "JotAPI.h"
#include "JotCtrl.h"
#include "JotCtrlRsc.h"

/*********************************************************************
 * Internal Constants
 *********************************************************************/

/* Define the minimum OS version we support */
#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

/*********************************************************************
 * Internal Functions
 *********************************************************************/

/*
 * FUNCTION: JotAPICall
 *
 * DESCRIPTION:
 *
 * This routine calls the Jot API with the specified fFeature value.
 *
 * PARAMETERS:
 *
 * fFeature
 *     Feature flags to pass to Jot.
 *
 * RETURNED:
 *     fFeature value retured by Jot, or -1 on failure.
 */

UInt32 JotAPICall( UInt32 fFeature )
{

	Err					err;
	UInt16				cardNo;
	LocalID				dbID = NULL;
	DmSearchStateType	searchState;
	UInt32				fFeatureLocal;

	err = DmGetNextDatabaseByTypeCreator( true,
										  & searchState,
										  sysFileTApplication,
										  JOTCREATOR,
										  true,
										  & cardNo,
										  & dbID);

	if ( ! err ) {

		fFeatureLocal = fFeature;

		SysAppLaunch( cardNo,
					  dbID,
					  0,
					  32801,
					  (char *) & fFeatureLocal,
					  NULL );

		return fFeatureLocal;
	
	}
	
	return 0xFFFFFFFF;

}

/*
 * FUNCTION: MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:
 *
 * command
 *     menu item id
 */

static Boolean MainFormDoCommand(UInt16 command)
{
    Boolean handled = false;
	FormType * frmP;

    switch (command)
    {
    case AboutAboutJotCtrl:
        MenuEraseStatus(0);
        /* Display the About Box. */
        frmP = FrmInitForm (AboutForm);
        FrmDoDialog (frmP);                    
        FrmDeleteForm (frmP);
        handled = true;
        break;
    }
    
    return handled;
}

/*
 * FUNCTION: MainFormHandleEvent
 *
 * DESCRIPTION:
 *
 * This routine is the event handler for the "MainForm" of this 
 * application.
 *
 * PARAMETERS:
 *
 * eventP
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed to
 *     FrmHandleEvent
 */

static Boolean MainFormHandleEvent(EventType * eventP)
{

    Boolean		handled = false;
    FormType	* frmP;

    switch (eventP->eType) {

		case ctlSelectEvent:

			switch ( eventP->data.ctlSelect.controlID ) {

				case MainEnableJotCheckbox:
					JotAPICall( ( eventP->data.ctlSelect.on ) ?
									ENABLE_JOT : DISABLE_JOT );
					handled = true;
					break;

				case MainEnableWritingCheckbox:
					JotAPICall( ( eventP->data.ctlSelect.on ) ?
									ENABLE_WRITING : DISABLE_WRITING );
					handled = true;
					break;

				case MainEnableInkingCheckbox:
					JotAPICall( ( eventP->data.ctlSelect.on ) ?
									ENABLE_INKING : DISABLE_INKING );
					handled = true;
					break;

				case MainEnableModeMarkCheckbox:
					JotAPICall( ( eventP->data.ctlSelect.on ) ?
									ENABLE_MODEMARK : DISABLE_MODEMARK );
		            frmP = FrmGetActiveForm();
            		FrmDrawForm(frmP);
					handled = true;
					break;

				case MainDoneButton:
					{
						EventType Event;
						MemSet( & Event, sizeof( Event ), 0 );
						Event.eType = appStopEvent;
						EvtAddEventToQueue( & Event );
					}
					handled = true;
					break;

			}

			break;

        case menuEvent:
            return MainFormDoCommand(eventP->data.menu.itemID);

        case frmOpenEvent:
            frmP = FrmGetActiveForm();
            FrmDrawForm(frmP);
            handled = true;
            break;
            
        default:
            break;

        }
    
    return handled;

}

/*
 * FUNCTION: AppHandleEvent
 *
 * DESCRIPTION: 
 *
 * This routine loads form resources and set the event handler for
 * the form loaded.
 *
 * PARAMETERS:
 *
 * event
 *     a pointer to an EventType structure
 *
 * RETURNED:
 *     true if the event was handled and should not be passed
 *     to a higher level handler.
 */

static Boolean AppHandleEvent(EventType * eventP)
{
    UInt16 formId;
    FormType * frmP;

    if (eventP->eType == frmLoadEvent)
    {
        /* Load the form resource. */
        formId = eventP->data.frmLoad.formID;
        frmP = FrmInitForm(formId);
        FrmSetActiveForm(frmP);

        /* Set the event handler for the form.  The handler of the
         * currently active form is called by FrmHandleEvent each
         * time is receives an event. */
        switch (formId)
        {
        case MainForm:
            FrmSetEventHandler(frmP, MainFormHandleEvent);
            break;

        default:
            break;

        }
        return true;
    }

    return false;
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 */

static void AppEventLoop(void)
{
    UInt16 error;
    EventType event;

    do {
        /* change timeout if you need periodic nilEvents */
        EvtGetEvent(&event, evtWaitForever);

        if (! SysHandleEvent(&event))
        {
            if (! MenuHandleEvent(0, &event, &error))
            {
                if (! AppHandleEvent(&event))
                {
                    FrmDispatchEvent(&event);
                }
            }
        }
    } while (event.eType != appStopEvent);
}

void HookSysBatteryDialog( void )
{
}

/*
 * FUNCTION: AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * RETURNED:
 *     errNone - if nothing went wrong
 */

static Err AppStart(void)
{
    return errNone;
}

/*
 * FUNCTION: AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 */

static void AppStop(void)
{
    /* Close all the open forms. */
    FrmCloseAllForms();
}

/* all code from here to end of file should use no global variables */
#pragma warn_a5_access on

/*
 * FUNCTION: RomVersionCompatible
 *
 * DESCRIPTION: 
 *
 * This routine checks that a ROM version is meet your minimum 
 * requirement.
 *
 * PARAMETERS:
 *
 * requiredVersion
 *     minimum rom version required
 *     (see sysFtrNumROMVersion in SystemMgr.h for format)
 *
 * launchFlags
 *     flags that indicate if the application UI is initialized
 *     These flags are one of the parameters to your app's PilotMain
 *
 * RETURNED:
 *     error code or zero if ROM version is compatible
 */

static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32 romVersion;

    /* See if we're on in minimum required version of the ROM or later. */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion)
    {
        if ((launchFlags & 
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            FrmAlert (RomIncompatibleAlert);

            /* Palm OS versions before 2.0 will continuously relaunch this
             * app unless we switch to another safe one. */
            if (romVersion < kPalmOS20Version)
            {
                AppLaunchWithCommand(
                    sysFileCDefaultApp, 
                    sysAppLaunchCmdNormalLaunch, NULL);
            }
        }

        return sysErrRomIncompatible;
    }

    return errNone;
}

/*
 * FUNCTION: JotCtrlPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:
 *
 * cmd
 *     word value specifying the launch code. 
 *
 * cmdPB
 *     pointer to a structure that is associated with the launch code
 *
 * launchFlags
 *     word value providing extra information about the launch
 *
 * RETURNED:
 *     Result of launch, errNone if all went OK
 */

static UInt32 JotCtrlPalmMain(
    UInt16 cmd, 
    MemPtr /*cmdPBP*/, 
    UInt16 launchFlags)
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

        /* start application by opening the main form
         * and then entering the main event loop */
        FrmGotoForm(MainForm);
        AppEventLoop();
        
        AppStop();
        break;

    default:
        break;
    }

    return errNone;
}

/*
 * FUNCTION: PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 * 
 * PARAMETERS:
 *
 * cmd
 *     word value specifying the launch code. 
 *
 * cmdPB
 *     pointer to a structure that is associated with the launch code
 *
 * launchFlags
 *     word value providing extra information about the launch.
 *
 * RETURNED:
 *     Result of launch, errNone if all went OK
 */
 
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return JotCtrlPalmMain(cmd, cmdPBP, launchFlags);
}

/* turn a5 warning off to prevent it being set off by C++
 * static initializer code generation */
#pragma warn_a5_access reset
