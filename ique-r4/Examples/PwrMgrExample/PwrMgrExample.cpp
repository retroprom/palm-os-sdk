/*********************************************************************
*
*   MODULE NAME:
*
*       PwrMgrExample.c - Power Manager Example.
*
*   DESCRIPTION:
*       Source file for Power Manager Example project.
*
*   PUBLIC PROCEDURES:
*       Name                        Title
*       -----------------------     --------------------------------------
*       PilotMain                   Pilot Main
*
*   PRIVATE PROCEDURES:
*       Name                        Title
*       -----------------------     --------------------------------------
*
*   LOCAL PROCEDURES:
*       Name                        Title
*       -----------------------     --------------------------------------
*       AppEventLoop                Application Event Loop
*       AppHandleEvent              Application Handle Event
*       AppMain                     Application Main
*       AppStop                     Application Stop
*       FormInit                    Main Form Init
*       MainFormHandleEvent         Main Form Handle Event
*
* Copyright 2003 by Garmin Ltd. or its subsidiaries.
*---------------------------------------------------------------------
* $Log:
*  3    iQue3600  1.2         12/15/2003 9:47:32 AM  Sherry Elder    Added call
*       to update the main form and removed unnecessary variables.
*  2    iQue3600  1.1         12/11/2003 4:00:07 PM  Sherry Elder    Disable
*       low power mode upon exit of app.
*  1    iQue3600  1.0         12/11/2003 3:42:46 PM  Sherry Elder    
* $
* $NoKeywords$
*********************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/
#include <PalmOS.h>
#include <NotifyMgr.h>
#include <PalmUtils.h>

#include "PenInputMgr.h"

#include "PwrMgrExample_rsrc.h"
#include "PwrMgrLib68K.h"

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/
	
/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define			PwrMgrExampleCreator		'PMEX'

/*--------------------------------------------------------------------
                           PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/
static const Char enabledStr[]       = "enabled";
static const Char disabledStr[]      = "disabled";
/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/
UInt16			gPwrMgrLibRef;

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/
static void     AppEventLoop(void);
static Boolean  AppHandleEvent( EventPtr eventP );
static UInt32   AppMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags );
static void     AppStop( void );
static void 	CenterTextField( const RectanglePtr rectP, const UInt16 objectID, const Boolean draw );
static void     FormInit( FormPtr frmP );
static void 	FreeFieldText( const UInt16 objectID );
static Boolean  MainFormDoCommand( UInt16 command );
static Boolean  MainFormHandleEvent( EventPtr eventP );
static void 	SetFieldText( const UInt16 objectID, const Char *s );

/*********************************************************************
*
*   PROCEDURE NAME:
*       PilotMain - Pilot Main
*
*   DESCRIPTION:
*       Main entry point for the application.  Returns the
*       result of the launch.
*
*   PARAMETERS:
*       cmd         - launch code.
*       cmdPBP      - pointer to structure associated with
*                     the launch code.
*       launchFlags - extra information about the launch.
*
*********************************************************************/
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags )
{
return AppMain( cmd, cmdPBP, launchFlags );
} /* PilotMain() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       AppEventLoop - Application Event Loop
*
*   DESCRIPTION:
*       The event loop for the application.
*
*********************************************************************/
static void AppEventLoop(void)
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Err         error;
EventType   event;

/*----------------------------------------------------------
Enter event loop.
----------------------------------------------------------*/
do {
    /*------------------------------------------------------
    Get an event.
    ------------------------------------------------------*/
    EvtGetEvent(&event, 1 );

    /*------------------------------------------------------
    Send to each handler in order, if not already used.
    ------------------------------------------------------*/
    if ( ! SysHandleEvent( &event ) )
        {
        if ( ! MenuHandleEvent( 0, &event, &error ) )
            {
            if ( ! AppHandleEvent( &event ) )
                {
                FrmDispatchEvent(&event);
                }
            }
        }
    } while ( event.eType != appStopEvent );
} /* AppEventLoop() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       AppHandleEvent - Application Handle Event
*
*   DESCRIPTION:
*       Event handler for the application.
*       Returns true if the event was handled, false otherwise.
*
*********************************************************************/
static Boolean AppHandleEvent( EventPtr eventP )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
UInt16  formId;
FormPtr frmP;
Boolean handled;

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Process event.
----------------------------------------------------------*/
if ( eventP->eType == frmLoadEvent )
    {
    /*------------------------------------------------------
    Load the form resource and set the active form.
    ------------------------------------------------------*/
    formId = eventP->data.frmLoad.formID;
    frmP   = FrmInitForm( formId );
    FrmSetActiveForm( frmP );

    /*------------------------------------------------------
    Set the constraints.
    ------------------------------------------------------*/
    WinSetConstraintsSize( WinGetDisplayWindow(), 160, 160, 160, 160, 160, 160 );

    /*------------------------------------------------------
    Set the dynamic input area policy.
    ------------------------------------------------------*/
    FrmSetDIAPolicyAttr( frmP, frmDIAPolicyStayOpen );

    /*------------------------------------------------------
    Enable the input trigger.
    ------------------------------------------------------*/
    PINSetInputTriggerState( pinInputTriggerDisabled );

    /*------------------------------------------------------
    Set the event handler for the form.
    ------------------------------------------------------*/
    if ( formId == MainForm )
        {
        FrmSetEventHandler(frmP, MainFormHandleEvent);
        }

    handled = true;
    }

return handled;

} /* AppHandleEvent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       AppMain - Application Main
*
*   DESCRIPTION:
*       Main entry point for the application
*
*   PARAMETERS:
*       cmd         - launch code.
*       cmdPBP      - pointer to structure associated with
*                     the launch code.
*       launchFlags - extra information about the launch.
*
*********************************************************************/
static UInt32 AppMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean protectedTag = true;
UInt16  cardNo;
LocalID dbID;
Err		error;

/*----------------------------------------------------------
Perform command.
----------------------------------------------------------*/
if ( cmd == sysAppLaunchCmdNormalLaunch )
    {

    /*------------------------------------------------------
    Find PwrMgrLib.  If not found, load it.
    ------------------------------------------------------*/
    error = SysLibFind( kPwrMgrLibName, &gPwrMgrLibRef );
    if ( error != errNone )
        {
        error = SysLibLoad( kPwrMgrLibType, kPwrMgrLibCreator, &gPwrMgrLibRef );
        ErrFatalDisplayIf( ( error != errNone ), "Can't open PwrMgrLib" );
        }

    /*------------------------------------------------------
    Register for the Display Resized Event notification.
    ------------------------------------------------------*/
    error = SysCurAppDatabase( &cardNo, &dbID );
    ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

    /*------------------------------------------------------
    Goto the main form.
    ------------------------------------------------------*/
    FrmGotoForm(MainForm);

    /*------------------------------------------------------
    Enter the event loop.
    ------------------------------------------------------*/
    AppEventLoop();

    /*------------------------------------------------------
    Stop the application when done.
    ------------------------------------------------------*/
    AppStop();
    }
return errNone;
} /* AppMain() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       AppStop - Application Stop
*
*   DESCRIPTION:
*       Cleans up before the application stops.
*
*********************************************************************/
static void AppStop( void )
{
FreeFieldText( MainInfoField );

/*----------------------------------------------------------
Remove the ARM-native library. Disable low power mode, 
ensuring that the unit will turn off. 
----------------------------------------------------------*/
PwrSetLowPowerMode( gPwrMgrLibRef, PwrMgrExampleCreator, false );
SysLibRemove( gPwrMgrLibRef );
FrmCloseAllForms();
} /* AppStop() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       CenterTextField - Center Text Field
*
*   DESCRIPTION:
*       Centers the specfied text field horizontally in the specifed
*       rectangle.  The object will be erased at the initial location
*       and redrawn at the new location if specified.
*
*********************************************************************/
static void CenterTextField( const RectanglePtr rectP, const UInt16 objectID, const Boolean draw )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Char            *stringP;
FieldPtr        fldP;
FormPtr         frmP;
Int16           stringWidth;
RectangleType   objectBoundsR;

/*----------------------------------------------------------
Get the active form pointer.
----------------------------------------------------------*/
frmP = FrmGetActiveForm();

/*----------------------------------------------------------
Get the width of the field string in pixels.
----------------------------------------------------------*/
fldP        = ( FieldPtr ) FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, objectID ) );
FntSetFont( FldGetFont( fldP ) );
stringP     = FldGetTextPtr( fldP );
stringWidth = FntCharsWidth( stringP, StrLen( stringP ) );

/*----------------------------------------------------------
Get object bounds.
----------------------------------------------------------*/
FrmGetObjectBounds( frmP, FrmGetObjectIndex( frmP, objectID ), &objectBoundsR );

/*----------------------------------------------------------
Erase the field at the initial location if necessary.
----------------------------------------------------------*/
if ( draw )
    {
    FldEraseField( fldP );
    }

/*----------------------------------------------------------
Center the field based on width of the string in pixels.
----------------------------------------------------------*/
objectBoundsR.topLeft.x = rectP->topLeft.x + ( rectP->extent.x / 2 ) - ( stringWidth / 2 );
objectBoundsR.extent.x = stringWidth;
FrmSetObjectBounds( frmP, FrmGetObjectIndex( frmP, objectID ), &objectBoundsR );

/*----------------------------------------------------------
Draw the field at the new location if necessary.
----------------------------------------------------------*/
if ( draw )
    {
    FldDrawField( fldP );
    }
} /* CenterTextField() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       FormInit - Main Form Init
*
*   DESCRIPTION:
*       Initializes the specified main form of the application.
*
*********************************************************************/
static void FormInit( FormPtr frmP )
{
RectangleType	frmBounds;

FrmGetFormBounds( frmP, &frmBounds );

FrmSetControlValue( frmP, FrmGetObjectIndex( frmP, MainDisablePushButton ), true );
FrmSetControlValue( frmP, FrmGetObjectIndex( frmP, MainEnablePushButton ), false );

FldSetFont( ( FieldPtr ) FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, MainInfoField ) ), stdFont );
FrmDrawForm( frmP );

SetFieldText( MainInfoField, disabledStr );
CenterTextField( &frmBounds, MainInfoField, true );
} /* FormInit() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       FreeFieldText - Free Field Text
*
*   DESCRIPTION:
*       Frees the memory allocated for the specified field's text.
*
*********************************************************************/
static void FreeFieldText( const UInt16 objectID )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FieldPtr  fldP;
Char     *tmp;

/*----------------------------------------------------------
Get the field pointer.
----------------------------------------------------------*/
fldP = ( FieldPtr )  FrmGetObjectPtr( FrmGetActiveForm(), FrmGetObjectIndex( FrmGetActiveForm(), objectID ) );

/*----------------------------------------------------------
Get the text pointer and free it if necessary.
----------------------------------------------------------*/
tmp = FldGetTextPtr( fldP );
if ( tmp != NULL )
    {
    MemPtrFree( tmp );
    }
} /* FreeFieldText() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       MainFormDoCommand - Main Form Do Command
*
*   DESCRIPTION:
*       Does the work for a menu event
*********************************************************************/
static Boolean MainFormDoCommand( const UInt16 command )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean     handled;
FormPtr     aboutFrmP;

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;

switch ( command )
    {
    /*------------------------------------------------------
    Display the about box.
    ------------------------------------------------------*/
	case OptionsAbout:
        MenuEraseStatus( 0 );
        aboutFrmP = FrmInitForm( AboutForm );
        FrmDoDialog( aboutFrmP );
        FrmDeleteForm( aboutFrmP );
        FrmUpdateForm( MainForm, frmRedrawUpdateCode );
        handled = true;
        break;
	}
return handled;
} /* MainFormDoCommand() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       MainFormHandleEvent - Main Form Handle Event
*
*   DESCRIPTION:
*       Event handler for the main form of the application.
*       Returns true if the event was handled, false otherwise.
*
*********************************************************************/
static Boolean MainFormHandleEvent( EventPtr eventP )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean                 handled;
FormPtr                 frmP;
RectangleType			frmBounds;

frmP = FrmGetActiveForm();


/*----------------------------------------------------------
Handle event.
----------------------------------------------------------*/
switch (eventP->eType)
    {
    case frmOpenEvent:
        frmP = FrmGetActiveForm();
        FormInit( frmP );
        FrmDrawForm( frmP );
        handled = true;
        break;
	
    case frmUpdateEvent:
        frmP = FrmGetActiveForm();
        FrmDrawForm( frmP );
        handled = true;
        break;
	
    case ctlSelectEvent:
	    FrmGetFormBounds( frmP, &frmBounds );
        switch ( eventP->data.ctlSelect.controlID )
        	{
        	case MainDisablePushButton:
        		PwrSetLowPowerMode( gPwrMgrLibRef, PwrMgrExampleCreator, false );
        		FrmSetControlValue( frmP, FrmGetObjectIndex( frmP, MainDisablePushButton ), true );
				FrmSetControlValue( frmP, FrmGetObjectIndex( frmP, MainEnablePushButton ), false );
				SetFieldText( MainInfoField, disabledStr );
				CenterTextField( &frmBounds, MainInfoField, true );
        		break;
        	
        	case MainEnablePushButton:
	        	PwrSetLowPowerMode( gPwrMgrLibRef, PwrMgrExampleCreator, true );
        		FrmSetControlValue( frmP, FrmGetObjectIndex( frmP, MainEnablePushButton ), true );
				FrmSetControlValue( frmP, FrmGetObjectIndex( frmP, MainDisablePushButton ), false );
				SetFieldText( MainInfoField, enabledStr );
				CenterTextField( &frmBounds, MainInfoField, true );
        		break;
        	
        	default:
        		break;
            }
        break;
	
	case menuEvent:
        handled = MainFormDoCommand( eventP->data.menu.itemID );
        break;

    default:
        break;
    }
return handled;
} /* MainFormHandleEvent() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       SetFieldText - Set Field Text
*
*   DESCRIPTION:
*       Sets the specified field's text to the specified value.
*
*********************************************************************/
static void SetFieldText( const UInt16 objectID, const Char *s )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FieldPtr  	fldP;
Char     	*tmp;
FormPtr	 	frmP = FrmGetActiveForm();

/*----------------------------------------------------------
Get the field pointer.
----------------------------------------------------------*/
fldP = ( FieldPtr ) FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, objectID ) );

/*----------------------------------------------------------
Get the text pointer and free it if necessary.
----------------------------------------------------------*/
tmp = FldGetTextPtr( fldP );
if ( tmp != NULL )
    {
    MemPtrFree( tmp );
    }

/*----------------------------------------------------------
Allocate memory and initialize the new string.
----------------------------------------------------------*/
tmp = ( Char * ) MemPtrNew( StrLen( s ) + 1 );
StrCopy( tmp, s );

/*----------------------------------------------------------
Set the field's text to the new string.
----------------------------------------------------------*/
FldSetTextPtr( fldP, tmp );
} /* SetFieldText() */