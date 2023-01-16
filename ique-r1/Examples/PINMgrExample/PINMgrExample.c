/*********************************************************************
*
*   MODULE NAME:
*       PINMgrExample.c - Pen Input Manager Example.
*
*   DESCRIPTION:
*       Source file for Pen Input Manager Example project.
*
*   PUBLIC PROCEDURES:
*       Name                    Title
*       ----------------------- --------------------------------------
*       PilotMain                   Pilot Main
*
*   PRIVATE PROCEDURES:
*       Name                    Title
*       ----------------------- --------------------------------------
*
*   LOCAL PROCEDURES:
*       Name                    Title
*       ----------------------- --------------------------------------
*       AppEventLoop                Application Event Loop
*       AppHandleEvent              Application Handle Event
*       AppMain                     Application Main
*       AppStop                     Application Stop
*       CenterObject                Center Object
*       CenterTextField             Center Text Field
*       DisplayResizedEventCallback Display Resized Event Callback
*       FormInit                    Main Form Init
*       FormResize                  Form Resize
*       GetObjectPtr                Get Object Pointer
*       MainFormDoCommand           Main Form Do Command
*       MainFormHandleEvent         Main Form Handle Event
*       MoveObject                  Move Object
*       SetFieldText                Set Field Text
*       SetDIAStateText             Set Dynamic Input Area State Text
*
*   PROCEDURES:
*       Name                        Title
*       -----------------------     --------------------------------------
*
* Copyright 2002-2003 by Garmin Ltd. or its subsidiaries.
*---------------------------------------------------------------------
* $Log:
*  16   iQue3600  1.15        2/27/2003 12:31:41 PM  Paul McAlister  
*       Changed to remove the refNum parameter from the Pen Input Manager APIs.
*       
*  15   iQue3600  1.14        2/4/2003 1:18:33 PM    Paul McAlister  
*       Added a comment about how to close the input area upon initialization.
*  14   iQue3600  1.13        1/24/2003 2:59:15 PM   Paul McAlister  
*       Used constant pinMaxConstraintSize when specifying constraint sizes.
*  13   iQue3600  1.12        1/24/2003 10:54:48 AM  Paul McAlister  
*       Cleaned up a bit.
*  12   iQue3600  1.11        1/24/2003 9:26:02 AM   Paul McAlister  
*       Made compatible with the new Dynamic Input Area scheme.
*  11   iQue3600  1.10        12/17/2002 12:05:28 PM Paul McAlister  
*       Corrected maxHeight parameter in PINSetConstraints() to reflect
*       corrected height of status bar.
*  10   iQue3600  1.9         11/21/2002 10:18:29 AM Paul McAlister  
*       Simplified font changing logic; moved text field to prevent errors when
*       set to a larger font.
*  9    iQue3600  1.8         11/19/2002 10:46:47 AM Chris Kellogg   Fixed a
*       screen refresh issue after the font dialog is displayed.
*  8    iQue3600  1.7         11/11/2002 1:45:51 PM  Paul McAlister  
*       "Palmized" a variable name.
*  7    iQue3600  1.6         11/8/2002 1:41:46 PM   Paul McAlister  
*       Corrected a comment.
*  6    iQue3600  1.5         11/8/2002 11:16:33 AM  Paul McAlister  
*       Don't redraw when a display change notification is received and we are
*       not the active form.
*  5    iQue3600  1.4         10/17/2002 4:37:04 PM  Paul McAlister  
*       Supply reasonable values for all parameters of PINSetConstraints().
*  4    iQue3600  1.3         10/17/2002 9:16:20 AM  Paul McAlister  
*       Corrected spacing after renaming the type.
*  3    iQue3600  1.2         10/16/2002 1:17:25 PM  Paul McAlister  
*       Renamed a type to match GARMIN naming conventions.
*  2    iQue3600  1.1         10/16/2002 7:01:30 AM  Paul McAlister  
*       Corrected type name capitalization.
*  1    iQue3600  1.0         10/15/2002 2:31:28 PM  Paul McAlister  
* $
* $NoKeywords$
*********************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

#include <PalmOS.h>
#include <NotifyMgr.h>
#include "PenInputMgr.h"

#include "PINMgrExample.h"

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

static const Char openStr[]          = "open";
static const Char closedStr[]        = "closed";
static const Char enabledStr[]       = "enabled";
static const Char disabledStr[]      = "disabled";
static const Char stayOpenStr[]      = "stay open";
static const Char customStr[]        = "custom";

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

static Boolean      gDisplayNeedsRedrawn;   /* Display needs to be redrawn? */
static FontID       gCurrentFont;
static FormPtr      gOurActiveForm;         /* Our active form              */

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/
static void     AppEventLoop(void);
static Boolean  AppHandleEvent( EventPtr eventP );
static UInt32   AppMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags );
static void     AppStop( void );
static void     CenterObject( FormPtr frmP, const UInt16 objIndex, const Boolean erase );
static void     CenterTextField( const RectanglePtr rectP, const UInt16 objectID, const Boolean draw );
static Err      DisplayResizedEventCallback( SysNotifyParamType *notifyParamsP );
static void     FormInit( FormPtr frmP );
static void     FormResize( FormPtr frmP, const Boolean draw );
static void     FreeFieldText( const UInt16 objectID );
static void     *GetObjectPtr( const UInt16 objectID );
static Boolean  MainFormDoCommand( const UInt16 command );
static Boolean  MainFormHandleEvent( EventPtr eventP );
static void     MoveObject( FormPtr frmP, const UInt16 objIndex, const Coord y_diff, const Boolean erase );
static void     SetFieldText( const UInt16 objectID, const Char *s );
static void     SetDIAStateText( void );

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
    EvtGetEvent(&event, evtWaitForever);

    /*------------------------------------------------------
    If we get an event we must be active; redraw the display
    if needed.
    ------------------------------------------------------*/
    if ( gDisplayNeedsRedrawn )
        {
        /*--------------------------------------------------
        Clear flag and call the display resized callback to
        redraw.
        --------------------------------------------------*/
        gDisplayNeedsRedrawn = false;

        /*--------------------------------------------------
        Resize the main form.
        --------------------------------------------------*/
        SetDIAStateText();
        FormResize( FrmGetActiveForm(), true );
        }

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
    Capture our active form and window handle.
    ------------------------------------------------------*/
    gOurActiveForm   = frmP;

    /*------------------------------------------------------
    Set this forms's constraints.
    ------------------------------------------------------*/
    WinSetConstraintsSize( WinGetDisplayWindow(), 160, 160, pinMaxConstraintSize, 160, 160, 160 );

    /*------------------------------------------------------
    Set the form's dynamic Input Area Policy.
    ------------------------------------------------------*/
    FrmSetDIAPolicyAttr( gOurActiveForm, frmDIAPolicyCustom );

    /*------------------------------------------------------
    Enable the input trigger.
    ------------------------------------------------------*/
    PINSetInputTriggerState( pinInputTriggerEnabled );

    /*------------------------------------------------------
    If your application wanted to close the input area to
    have as much space available as possible, you would
    use the following call.
    ------------------------------------------------------*/
    // PINSetInputAreaState( pinInputAreaClosed );

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
UInt16  cardNo;
LocalID dbID;
Err     error;

/*----------------------------------------------------------
Perform command.
----------------------------------------------------------*/
if ( cmd == sysAppLaunchCmdNormalLaunch )
    {
    /*------------------------------------------------------
    Initialize.
    ------------------------------------------------------*/
    gOurActiveForm       = NULL;
    gDisplayNeedsRedrawn = false;

    /*------------------------------------------------------
    Register for the Display Resized Event notification.
    ------------------------------------------------------*/
    error = SysCurAppDatabase( &cardNo, &dbID );
    ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

    error = SysNotifyRegister( cardNo, dbID, sysNotifyDisplayResizedEvent, DisplayResizedEventCallback, sysNotifyNormalPriority, NULL );
    ErrFatalDisplayIf( ( error != errNone ), "can't register" );

    /*------------------------------------------------------
    Goto the main form.
    ------------------------------------------------------*/
    FrmGotoForm( MainForm );

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
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
UInt16  cardNo;
LocalID dbID;
Err     error;

/*----------------------------------------------------------
Unregister for the Display Resized Event notification.
----------------------------------------------------------*/
error = SysCurAppDatabase( &cardNo, &dbID );
ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

error = SysNotifyUnregister( cardNo, dbID, sysNotifyDisplayResizedEvent, sysNotifyNormalPriority );
ErrFatalDisplayIf( ( error != errNone ), "can't unregister" );

/*----------------------------------------------------------
Clean up fields.
----------------------------------------------------------*/
FreeFieldText( MainAreaStateField );
FreeFieldText( MainTriggerStateField );

/*----------------------------------------------------------
Close all the open forms.
----------------------------------------------------------*/
FrmCloseAllForms();

} /* AppStop() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       CenterObject - Center Object
*
*   DESCRIPTION:
*       Centers the specified object horizontally and verically
*       within the specified form. The object will be erased at
*       the initial location if specified.
*
*********************************************************************/
static void CenterObject( FormPtr frmP, const UInt16 objIndex, const Boolean erase )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
RectangleType   formBoundsR;
RectangleType   objectBoundsR;

/*----------------------------------------------------------
Get the form and object bounds.
----------------------------------------------------------*/
FrmGetFormBounds( frmP, &formBoundsR );
FrmGetObjectBounds( frmP, objIndex, &objectBoundsR );

/*----------------------------------------------------------
Erase the object at the initial location if necessary.
----------------------------------------------------------*/
if (erase)
    {
    /*------------------------------------------------------
    Resize the object's bounds to erase the frame as well.
    ------------------------------------------------------*/
    RctInsetRectangle( &objectBoundsR, -2 );

    /*------------------------------------------------------
    Erase the object.
    ------------------------------------------------------*/
    WinEraseRectangle( &objectBoundsR, 0 );

    /*------------------------------------------------------
    Restore the rectangle size.
    ------------------------------------------------------*/
    RctInsetRectangle( &objectBoundsR, 2 );
    }

/*----------------------------------------------------------
Calculate and set the new location of the object.
----------------------------------------------------------*/
objectBoundsR.topLeft.x = ( formBoundsR.extent.x / 2 ) - ( objectBoundsR.extent.x / 2 );
objectBoundsR.topLeft.y = ( formBoundsR.extent.y / 2 ) - ( objectBoundsR.extent.y / 2 );
FrmSetObjectBounds( frmP, objIndex, &objectBoundsR );

} /* CenterObject() */


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
fldP        = GetObjectPtr( objectID );
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
FrmSetObjectBounds( frmP, FrmGetObjectIndex( frmP, objectID ), &objectBoundsR );

/*----------------------------------------------------------
Draw the field at the new location if necessary.
----------------------------------------------------------*/
if ( draw )
    {
    FldDrawField( fldP );
    }

} /* CenterTextField() */


/******************************************************************************
*
*   PROCEDURE NAME:
*       DisplayResizedEventCallback - Display Change Event Callback
*
*   DESCRIPTION:
*       Gets called when a Display Resized Event Notification occurs.
*
******************************************************************************/
static Err DisplayResizedEventCallback( SysNotifyParamType *notifyParamsP )
{
/*----------------------------------------------------------
Constants
----------------------------------------------------------*/
static const EventType lNilEvent = { nilEvent };

/*----------------------------------------------------------
Indicate that the display needs to be redrawn when we are
able, then send ourselves a nil event to force it to be
redrawn.
----------------------------------------------------------*/
gDisplayNeedsRedrawn = true;
EvtAddEventToQueue( &lNilEvent );

/*----------------------------------------------------------
Per the documentation return 0.
----------------------------------------------------------*/
return 0;

} /* DisplayResizedEventCallback() */


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
/*----------------------------------------------------------
Initialize the font for the dynamic input area text fields.
----------------------------------------------------------*/
gCurrentFont = stdFont;
FldSetFont( GetObjectPtr( MainAreaStateField ), gCurrentFont );
FldSetFont( GetObjectPtr( MainTriggerStateField ), gCurrentFont );

/*----------------------------------------------------------
Size the form the first time.
----------------------------------------------------------*/
FormResize( frmP, false );

} /* FormInit() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       FormResize - Form Resize
*
*   DESCRIPTION:
*       Resizes the specified form to fill the display window.
*       The form will be drawn after resizing if specified.
*
*********************************************************************/
static void FormResize( FormPtr frmP, const Boolean draw )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Coord           displayExtentX;
Coord           displayExtentY;
Coord           yDiff;
RectangleType   fldBoundsR;
RectangleType   formBoundsR;
UInt16          ctlID;
WinHandle       oldDrawWin;

/*----------------------------------------------------------
Initialize the dynamic input area state text.
----------------------------------------------------------*/
SetDIAStateText();

/*----------------------------------------------------------
Get the current input area state and trigger state and
highlight the corresponding buttons.
----------------------------------------------------------*/
switch ( PINGetInputAreaState() )
    {
    case pinInputAreaOpen:
        ctlID = MainOpenPushButton;
        break;

    case pinInputAreaClosed:
        ctlID = MainClosePushButton;
        break;
    }
FrmSetControlGroupSelection( frmP, MainGroupID, ctlID );

switch ( PINGetInputTriggerState() )
    {
    case pinInputTriggerEnabled:
        ctlID = MainEnablePushButton;
        break;

    case pinInputTriggerDisabled:
        ctlID = MainDisablePushButton;
        break;
    }
FrmSetControlGroupSelection( frmP, MainGroupID2, ctlID );

/*----------------------------------------------------------
set the draw window to get the right coordinates
----------------------------------------------------------*/
oldDrawWin = WinSetDrawWindow( FrmGetWindowHandle( frmP ) );

/*----------------------------------------------------------
Get the new display window extent.
----------------------------------------------------------*/
WinGetDisplayExtent( &displayExtentX, &displayExtentY );

/*----------------------------------------------------------
Get the form's old bounds.
----------------------------------------------------------*/
FrmGetFormBounds( frmP, &formBoundsR );

/*----------------------------------------------------------
Calculate the change in form size.
----------------------------------------------------------*/
yDiff = displayExtentY - ( formBoundsR.topLeft.y + formBoundsR.extent.y );

/*----------------------------------------------------------
Resize the form.
----------------------------------------------------------*/
formBoundsR.extent.y = displayExtentY;
formBoundsR.extent.x = displayExtentX;
WinSetWindowBounds( FrmGetWindowHandle( frmP ), &formBoundsR );

/*----------------------------------------------------------
Set fonts.
----------------------------------------------------------*/
FldSetFont( GetObjectPtr( MainAreaStateField ), gCurrentFont );
FldSetFont( GetObjectPtr( MainTriggerStateField ), gCurrentFont );

/*----------------------------------------------------------
Adjust the location of the items in the form.
----------------------------------------------------------*/
CenterObject( frmP, FrmGetObjectIndex( frmP, MainGlobeBitMap ), draw );

fldBoundsR = formBoundsR;
fldBoundsR.extent.x /= 2;

CenterTextField( &fldBoundsR, MainAreaStateField, false);
MoveObject( frmP, FrmGetObjectIndex( frmP, MainAreaStateField ), yDiff, draw );

fldBoundsR.topLeft.x = formBoundsR.extent.x - fldBoundsR.extent.x;
CenterTextField( &fldBoundsR, MainTriggerStateField, false);
MoveObject( frmP, FrmGetObjectIndex( frmP, MainTriggerStateField ), yDiff, draw );

/*----------------------------------------------------------
Draw the form if necessary, otherwise restore draw window.
----------------------------------------------------------*/
if ( draw )
    {
    FrmDrawForm( frmP );
    }
else
    {
    WinSetDrawWindow( oldDrawWin );
    }

} /* FormResize() */


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
fldP = ( FieldPtr ) GetObjectPtr( objectID );

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
*       GetObjectPtr - Get Object Pointer
*
*   DESCRIPTION:
*       Returns a pointer to the object in the active form that
*       has the specified object ID.
*
*********************************************************************/
static void *GetObjectPtr( const UInt16 objectID )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FormPtr frmP;

/*----------------------------------------------------------
Get active form pointer.
----------------------------------------------------------*/
frmP = FrmGetActiveForm();

/*----------------------------------------------------------
Return pointer to the object with the specified ID.
----------------------------------------------------------*/
return FrmGetObjectPtr( frmP, FrmGetObjectIndex( frmP, objectID ) );

} /* GetObjectPtr() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       MainFormDoCommand - Main Form Do Command
*
*   DESCRIPTION:
*       Performs the specified main form menu command.
*       Returns true if the command was handled, false otherwise.
*
*********************************************************************/
static Boolean MainFormDoCommand( const UInt16 command )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean         handled;
FormPtr         aboutFrmP;
FormPtr         frmP;
UInt16          formID;

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;
frmP    = FrmGetActiveForm();
formID  = FrmGetFormId (frmP);

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
        handled = true;
        break;

    /*------------------------------------------------------
    Let the user select a font.
    ------------------------------------------------------*/
    case OptionsFont :
        /*--------------------------------------------------
        Save the new font, and set the display needs redrawn
        flag to make the change take effect when possible.
        --------------------------------------------------*/
        gCurrentFont = FontSelect( gCurrentFont );
        gDisplayNeedsRedrawn = true;
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

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle event.
----------------------------------------------------------*/
switch (eventP->eType)
    {
    case menuEvent:
        handled = MainFormDoCommand( eventP->data.menu.itemID );
        break;

    case frmOpenEvent:
        /*--------------------------------------------------
        Open the form.
        --------------------------------------------------*/
        frmP = FrmGetActiveForm();
        FormInit( frmP );
        FrmDrawForm( frmP );
        handled = true;
        break;

    case menuOpenEvent:
        handled = true;
        break;

    case ctlSelectEvent:
        /*--------------------------------------------------
        Set input area or trigger state based on push button
        selected.
        --------------------------------------------------*/
        switch( eventP->data.ctlSelect.controlID )
            {
            case MainOpenPushButton:
                PINSetInputAreaState( pinInputAreaOpen );
                break;

            case MainClosePushButton:
                PINSetInputAreaState( pinInputAreaClosed );
                break;

            case MainEnablePushButton:
                PINSetInputTriggerState( pinInputTriggerEnabled );
                SetDIAStateText();
                FormResize( FrmGetActiveForm(), true );
                break;

            case MainDisablePushButton:
                PINSetInputTriggerState( pinInputTriggerDisabled );
                SetDIAStateText();
                FormResize( FrmGetActiveForm(), true );
                break;
            }
        handled = true;
        break;

    default:
        break;
    }

return handled;

} /* MainFormHandleEvent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       MoveObject - Move Object
*
*   DESCRIPTION:
*       Moves the specified object vertically the amount indicated
*       within the specified form.  The object will be erased
*       at the initial location if specified.
*
*********************************************************************/
static void MoveObject( FormPtr frmP, const UInt16 objIndex, const Coord y_diff, const Boolean erase )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
RectangleType   boundsR;

/*----------------------------------------------------------
Get the object bounds.
----------------------------------------------------------*/
FrmGetObjectBounds( frmP, objIndex, &boundsR );

/*----------------------------------------------------------
Erase the object at the initial location if necessary.
----------------------------------------------------------*/
if ( erase )
    {
    /*------------------------------------------------------
    Resize the object's bounds to erase the frame as well.
    ------------------------------------------------------*/
    RctInsetRectangle( &boundsR, -2 );

    /*------------------------------------------------------
    Erase the object.
    ------------------------------------------------------*/
    WinEraseRectangle( &boundsR, 0 );

    /*------------------------------------------------------
    Restore the rectangle size.
    ------------------------------------------------------*/
    RctInsetRectangle( &boundsR, 2 );
    }

/*----------------------------------------------------------
Change the object's vertical location as specified.
----------------------------------------------------------*/
boundsR.topLeft.y += y_diff;
FrmSetObjectBounds( frmP, objIndex, &boundsR );

} /* MoveObject() */


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
FieldPtr  fldP;
Char     *tmp;

/*----------------------------------------------------------
Get the field pointer.
----------------------------------------------------------*/
fldP = ( FieldPtr ) GetObjectPtr( objectID );

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


/*********************************************************************
*
*   PROCEDURE NAME:
*       SetDIAStateText - Set Dynamic Input Area State Text
*
*   DESCRIPTION:
*       Sets the text of the text fields based on the dynamic
*       input area states.
*
*********************************************************************/
static void SetDIAStateText( void )
{
switch ( PINGetInputAreaState() )
    {
    case pinInputAreaOpen:
        SetFieldText( MainAreaStateField, openStr );
        break;

    case pinInputAreaClosed:
        SetFieldText( MainAreaStateField, closedStr );
        break;
    }

switch ( PINGetInputTriggerState() )
    {
    case pinInputTriggerEnabled:
        SetFieldText( MainTriggerStateField, enabledStr );
        break;

    case pinInputTriggerDisabled:
        SetFieldText( MainTriggerStateField, disabledStr );
        break;
    }

} /* SetDIAStateText() */


