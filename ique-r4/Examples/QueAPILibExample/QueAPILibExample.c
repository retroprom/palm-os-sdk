/*********************************************************************
*
*   MODULE NAME:
*       QueAPILibExample.c - QueAPI Library Example.
*
*   DESCRIPTION:
*       Source file for QueAPI Library Example project.
*
*   PUBLIC PROCEDURES:
*       Name                    Title
*       ----------------------- --------------------------------------
*       PilotMain               Pilot Main
*
*   PRIVATE PROCEDURES:
*       Name                    Title
*       ----------------------- --------------------------------------
*
*   LOCAL PROCEDURES:
*       Name                    Title
*       ----------------------- --------------------------------------
*       AltitudeToString        Convert Altitude to String
*       AppendIntToStr          Append Integer to String with Leading Zeros
*       AppEventLoop            Application Event Loop
*       AppHandleEvent          Application Handle Event
*       AppMain                 Application Main
*       AppStart                Application Start
*       AppStop                 Application Stop
*       GetAddress              Get Address
*       GetAddressFromFields    Get Address From Fields
*       GetObjectPtr            Get Object Pointer
*       GetPoint                Get Point
*       HandleGoTo              Handle GoTo Event
*       InitMainForm            Initialize Main Form
*       LatitudeToString        Convert Latitude to String
*       LongitudeToString       Convert Longitude to String
*       MainFormDoCommand       Main Form Do Command
*       MainFormHandleEvent     Main Form Handle Event
*       SaveAddress             Save Address
*       SavePoint               Save Point
*       SetFieldTextFromHandle  Set Field Text From Handle
*       SetFieldTextFromStr     Set Field Text From String
*       ShowPointInfo           Show Point Info
*
* Copyright 2004 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

#include <PalmOS.h>
#include <NotifyMgr.h>
#include <SystemMgr.h>
#include <VFSMgr.h>

#include "QueAPILibExample.h"
#include "QueAPI.h"


/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

#define kQueAPILibExampleCreator   'QALX'
#define kStreetStrLen               50
#define kCityStrLen                 30
#define kStateStrLen                20
#define kZipStrLen                  10

/*----------------------------------------------------------
prefs
----------------------------------------------------------*/
enum
    { kPointPrefID
    , kAddressPrefID
    };

#define kPointPrefVersion       0
#define kAddressPrefVersion     0

/*----------------------------------------------------------
Conversions
----------------------------------------------------------*/
#define degMax              360                         /* max degree value                 */
#define metersToFeet        3.2808                      /* meters to feet                   */
#define minDigits           2                           /* 2 digits for whole minutes       */
#define minMax              60                          /* max minute value                 */
#define minPrecision        3                           /* 3 digits of minute precision     */
#define minScale            1000                        /* minute scaling                   */
#define semiToDeg           ( 180.0 / 2147483648.0 )    /* semicircles to degrees           */

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

typedef struct
    {
    Char    street[ kStreetStrLen + 1 ];
    Char    city[   kCityStrLen   + 1 ];
    Char    state[  kStateStrLen  + 1 ];
    Char    zip[    kZipStrLen    + 1 ];
    } AddressType;

/*--------------------------------------------------------------------
                           PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

UInt16              gQueAPILibRef;  /* global Library reference     */
QuePointHandle      gPoint;         /* global point                 */
AddressType         gAddress;

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

static void             AltitudeToString( float altitude, Char *outStr );
static void             AppendIntToStr( Int32 number, int width, Char *outStr );
static void             AppEventLoop(void);
static Boolean          AppHandleEvent( EventPtr aEventP );
static UInt32           AppMain( UInt16 aCmd, MemPtr aCmdPBP, UInt16 aLaunchFlags );
static void             AppStart( void );
static void             AppStop( void );
static void             GetAddress( AddressType *aAddress );
static void             GetAddressFromFields( AddressType *aAddress );
static void             *GetObjectPtr( const UInt16 aObjectID );
static QuePointHandle   GetPoint( void );
static void             HandleGoTo( GoToParamsPtr aGoToParams, const UInt16 aFormID );
static void             InitMainForm( void );
static void             LatitudeToString( Int32 latitude, Char *outStr );
static void             LongitudeToString( Int32 longitude, Char *outStr );
static Boolean          MainFormDoCommand( const UInt16 command );
static Boolean          MainFormHandleEvent( EventPtr aEventP );
static void             SaveAddress( const AddressType *aAddress );
static void             SavePoint( const QuePointHandle aPoint );
static void             SetFieldTextFromHandle( const UInt16 aFieldID, MemHandle aTextHandle );
static void             SetFieldTextFromStr( const UInt16 aFieldID, const Char *aStr );
static void             ShowPointInfo( const QuePointHandle aPoint );

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
*       aCmd         - launch code.
*       aCmdPBP      - pointer to structure associated with
*                      the launch code.
*       aLaunchFlags - extra information about the launch.
*
*********************************************************************/
UInt32 PilotMain( UInt16 aCmd, MemPtr aCmdPBP, UInt16 aLaunchFlags )
{

return AppMain( aCmd, aCmdPBP, aLaunchFlags );

} /* PilotMain() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       AltitudeToString - Convert Altitude to String
*
*   DESCRIPTION:
*       Converts the altitude in meters/sec to a string in feet.
*
*********************************************************************/
static void AltitudeToString( float altitude, Char *outStr )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
float alt;      /* local altitude                 */
Int32 whole;    /* whole part of location         */
Int32 frac;     /* fractional part of location    */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
*outStr = NULL;

/*----------------------------------------------------------
Convert to feet and round
----------------------------------------------------------*/
alt = ( altitude * metersToFeet ) + 0.05;

/*----------------------------------------------------------
Get whole altitude
----------------------------------------------------------*/
whole = ( Int32 ) alt;

/*----------------------------------------------------------
Get fractional altitude
----------------------------------------------------------*/
frac = ( Int32 ) ( ( alt - ( float ) whole ) * 10.0 );

/*----------------------------------------------------------
Build into string
----------------------------------------------------------*/
AppendIntToStr( whole, 0, outStr );
StrCat( outStr, "." );
AppendIntToStr( frac, 0, outStr );
StrCat( outStr, " ft" );

} /* AltitudeToString() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       AppendIntToStr - Append Integer to String with Leading Zeros
*
*   DESCRIPTION:
*       Appends the integer to a string, padding with leading zeros
*       the width specified.
*
*********************************************************************/
static void AppendIntToStr( Int32 number, int width, Char *outStr )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Char    buf[ 10 ];  /* temporary buffer             */
int     padCount;   /* count of characters to pad   */
int     i;          /* loop counter                 */

/*----------------------------------------------------------
Convert number to a string and determine the number of
characters that need to be padded.
----------------------------------------------------------*/
StrIToA( buf, number );
padCount = width - StrLen( buf );

/*----------------------------------------------------------
Write padCount zeros to output string
----------------------------------------------------------*/
for ( i = 0; i < padCount; ++i )
    {
    StrCat( outStr, "0" );
    }

/*----------------------------------------------------------
Write converted number to output string
----------------------------------------------------------*/
StrCat( outStr, buf );

} /* AppendIntToStr() */


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
static Boolean AppHandleEvent( EventPtr aEventP )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FormPtr frmP;
Boolean handled;

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Process event.
----------------------------------------------------------*/
if ( aEventP->eType == frmLoadEvent )
    {
    /*------------------------------------------------------
    Load the form resource and set the active form.
    ------------------------------------------------------*/
    frmP = FrmInitForm( MainForm );
    FrmSetActiveForm( frmP );

    /*------------------------------------------------------
    Set the event handler for the form.
    ------------------------------------------------------*/
    FrmSetEventHandler(frmP, MainFormHandleEvent);

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
*       aCmd         - launch code.
*       aCmdPBP      - pointer to structure associated with
*                      the launch code.
*       aLaunchFlags - extra information about the launch.
*
*********************************************************************/
static UInt32 AppMain( UInt16 aCmd, MemPtr aCmdPBP, UInt16 aLaunchFlags )
{
/*----------------------------------------------------------
Handle command.
----------------------------------------------------------*/
switch ( aCmd )
    {
    /*------------------------------------------------------
    Normal launch.
    ------------------------------------------------------*/
    case sysAppLaunchCmdNormalLaunch:
        /*--------------------------------------------------
        Start the application.
        --------------------------------------------------*/
        AppStart();

        /*--------------------------------------------------
        Goto the main form.
        --------------------------------------------------*/
        FrmGotoForm( MainForm );

        /*--------------------------------------------------
        Enter the event loop.
        --------------------------------------------------*/
        AppEventLoop();

        /*--------------------------------------------------
        Stop the application when done.
        --------------------------------------------------*/
        AppStop();
        break;

    /*------------------------------------------------------
    Goto launch.
    ------------------------------------------------------*/
    case sysAppLaunchCmdGoTo:
        /*--------------------------------------------------
        If we have just been launched.
        --------------------------------------------------*/
        if ( aLaunchFlags & sysAppLaunchFlagNewGlobals )
            {
            /*----------------------------------------------
            Start the application.
            ----------------------------------------------*/
            AppStart();

            /*----------------------------------------------
            Goto the main form.
            ----------------------------------------------*/
            FrmGotoForm( MainForm );

            /*----------------------------------------------
            Send the goto data to the main form.
            ----------------------------------------------*/
            HandleGoTo( ( GoToParamsPtr ) aCmdPBP, MainForm );

            /*----------------------------------------------
            Enter the event loop.
            ----------------------------------------------*/
            AppEventLoop();

            /*----------------------------------------------
            Stop the application when done.
            ----------------------------------------------*/
            AppStop();
            }

        /*--------------------------------------------------
        Otherwise we are already the current application.
        --------------------------------------------------*/
        else
            {
            /*----------------------------------------------
            Just send the goto data to the main form.
            ----------------------------------------------*/
            HandleGoTo( ( GoToParamsPtr ) aCmdPBP, MainForm );
            }
        break;

    } /* switch() */

return errNone;

} /* AppMain() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       AppStart - Application Start
*
*   DESCRIPTION:
*       Starts the application.
*
*********************************************************************/
static void AppStart( void )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Err     error;

/*----------------------------------------------------------
Find and open QueAPILib.  If not found, load it.
----------------------------------------------------------*/
error = SysLibFind( queAPILibName, &gQueAPILibRef );
if ( error != errNone )
    {
    error = SysLibLoad( queAPILibType, queAPILibCreator, &gQueAPILibRef );
    ErrFatalDisplayIf( ( error != errNone ), "Can't open QueAPILib" );
    }
error = QueAPIOpen( gQueAPILibRef, queAPIVersion );
ErrFatalDisplayIf( ( error == queErrInvalidVersion ), "Incompatible version of QueAPILib." );

/*----------------------------------------------------------
Get the saved address and point.
----------------------------------------------------------*/
GetAddress( &gAddress );
gPoint = GetPoint();

} /* AppStart() */


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
Close all the open forms.
----------------------------------------------------------*/
FrmCloseAllForms();

/*----------------------------------------------------------
Save then close the point.
----------------------------------------------------------*/
SavePoint( gPoint );
QueClosePoint( gQueAPILibRef, gPoint );

/*----------------------------------------------------------
Save the address.
----------------------------------------------------------*/
SaveAddress( &gAddress );

/*----------------------------------------------------------
Close QueAPILib; remove it if no longer in use.
----------------------------------------------------------*/
if ( QueAPIClose( gQueAPILibRef ) == queErrNone )
    {
    SysLibRemove( gQueAPILibRef );
    }

} /* AppStop() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       GetAddress - Get Address
*
*   DESCRIPTION:
*       Gets the address data from prefs.
*
*********************************************************************/
static void GetAddress( AddressType *aAddress )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Int16           prefsVersion;
UInt16          prefsSize;

/*----------------------------------------------------------
Get the address data from prefs.
----------------------------------------------------------*/
prefsSize = sizeof( AddressType );
prefsVersion = PrefGetAppPreferences
    ( kQueAPILibExampleCreator
    , kAddressPrefID
    , aAddress
    , &prefsSize
    , false
    );

/*----------------------------------------------------------
If it wasn't found, initialize to defaults.
----------------------------------------------------------*/
if ( prefsVersion == noPreferenceFound )
    {
    MemSet( aAddress, sizeof( *aAddress ), 0 );
    }

} /* GetAddress() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       GetAddressFromFields - Get Address From Fields
*
*   DESCRIPTION:
*       Gets the address data from the fields on the main form.
*
*********************************************************************/
static void GetAddressFromFields( AddressType *aAddress )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Char    *fldText;

/*----------------------------------------------------------
Get the street.
----------------------------------------------------------*/
fldText = FldGetTextPtr( GetObjectPtr( MainStreetField ) );
if ( fldText != NULL )
    {
    StrNCopy( gAddress.street, fldText, kStreetStrLen );
    gAddress.street[ kStreetStrLen ] = ( Char ) NULL;
    }

/*----------------------------------------------------------
Get the city.
----------------------------------------------------------*/
fldText = FldGetTextPtr( GetObjectPtr( MainCityField ) );
if ( fldText != NULL )
    {
    StrNCopy( gAddress.city, fldText, kCityStrLen );
    gAddress.city[ kCityStrLen ] = ( Char ) NULL;
    }

/*----------------------------------------------------------
Get the state.
----------------------------------------------------------*/
fldText = FldGetTextPtr( GetObjectPtr( MainStateField ) );
if ( fldText != NULL )
    {
    StrNCopy( gAddress.state, fldText, kStateStrLen );
    gAddress.state[ kStateStrLen ] = ( Char ) NULL;
    }

/*----------------------------------------------------------
Get the zip.
----------------------------------------------------------*/
fldText = FldGetTextPtr( GetObjectPtr( MainZipField ) );
if ( fldText != NULL )
    {
    StrNCopy( gAddress.zip, fldText, kZipStrLen );
    gAddress.zip[ kZipStrLen ] = ( Char ) NULL;
    }

} /* GetAddressFromFields() */


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
static void *GetObjectPtr( const UInt16 aObjectID )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FormPtr formP;

/*----------------------------------------------------------
Get active form pointer.
----------------------------------------------------------*/
formP = FrmGetActiveForm();

/*----------------------------------------------------------
Return pointer to the object with the specified ID.
----------------------------------------------------------*/
return FrmGetObjectPtr( formP, FrmGetObjectIndex( formP, aObjectID ) );

} /* GetObjectPtr() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       GetPoint - Get Point
*
*   DESCRIPTION:
*       Returns the point handle from prefs.
*
*********************************************************************/
static QuePointHandle GetPoint( void )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Int16           prefsVersion;
QuePointHandle  point;
UInt16          prefsSize;
UInt8           *buffer;

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
point = queInvalidPointHandle;

/*----------------------------------------------------------
Get the size of the buffer needed for the serialized point
data.
----------------------------------------------------------*/
prefsSize = 0;
prefsVersion = PrefGetAppPreferences
    ( kQueAPILibExampleCreator
    , kPointPrefID
    , NULL
    , &prefsSize
    , false
    );

/*----------------------------------------------------------
Allocate a buffer for the serialized point data.
----------------------------------------------------------*/
buffer = MemPtrNew( prefsSize );
if ( buffer != NULL )
    {
    /*------------------------------------------------------
    Get the serialized point data from the preference.
    ------------------------------------------------------*/
    prefsVersion = PrefGetAppPreferences
        ( kQueAPILibExampleCreator
        , kPointPrefID
        , buffer
        , &prefsSize
        , false
        );

    /*------------------------------------------------------
    If it was found, create a point from the data.
    ------------------------------------------------------*/
    if ( prefsVersion != noPreferenceFound )
        {
        QueDeserializePoint
            ( gQueAPILibRef
            , buffer
            , prefsSize
            , &point
            );
        }

    /*------------------------------------------------------
    Free the buffer.
    ------------------------------------------------------*/
    MemPtrFree( buffer );
    }

/*----------------------------------------------------------
Return the point Handle.
----------------------------------------------------------*/
return point;

} /* GetPoint() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       HandleGoTo - Handle GoTo
*
*   DESCRIPTION:
*       Sends the Goto parameter data from the launch command block
*       to the specified form.
*
*********************************************************************/
static void HandleGoTo( GoToParamsPtr aGoToParams, const UInt16 aFormID )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
EventType   event;

/*----------------------------------------------------------
Send a form goto event to the form.
----------------------------------------------------------*/
MemSet( &event, sizeof( EventType ), 0 );
event.eType                      = frmGotoEvent;
event.data.frmGoto.formID        = aFormID;
event.data.frmGoto.recordNum     = aGoToParams->recordNum;
event.data.frmGoto.matchPos      = aGoToParams->matchPos;
event.data.frmGoto.matchLen      = aGoToParams->matchCustom;
event.data.frmGoto.matchFieldNum = aGoToParams->matchFieldNum;
event.data.frmGoto.matchCustom   = aGoToParams->matchCustom;
EvtAddEventToQueue( &event );

} /* HandleGoTo() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       InitMainForm - Initialize Main Form
*
*   DESCRIPTION:
*       Initialize the main form.
*
*********************************************************************/
static void InitMainForm( void )
{
/*----------------------------------------------------------
Set the draw window.
----------------------------------------------------------*/
WinSetDrawWindow( FrmGetWindowHandle( FrmGetFormPtr( MainForm ) ) );

/*----------------------------------------------------------
Set the address fields from the address text.
----------------------------------------------------------*/
SetFieldTextFromStr( MainStreetField, gAddress.street );
SetFieldTextFromStr( MainCityField,   gAddress.city   );
SetFieldTextFromStr( MainStateField,  gAddress.state  );
SetFieldTextFromStr( MainZipField,    gAddress.zip    );
} /* InitMainForm() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       LatitudeToString - Convert Latitude to String
*
*   DESCRIPTION:
*       Converts the semicircle latitude into a string.
*
*********************************************************************/
static void LatitudeToString( Int32 latitude, Char *outStr )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Char        neswStr[ 5 ];   /* "NESW" string              */
double      degrees;        /* location in degrees        */
double      minutes;        /* location in minutes        */
Int32       wholeDeg;       /* whole part of degrees      */
Int32       wholeMin;       /* whole part of minutes      */
Int32       fracMin;        /* fractional part of minutes */

/*----------------------------------------------------------
Write hemisphere character into string
----------------------------------------------------------*/
SysCopyStringResource( neswStr, neswString );
outStr[ 0 ] = ( latitude < 0 ) ? neswStr[ 2 ] : neswStr[ 0 ];
outStr[ 1 ] = ' ';
outStr[ 2 ] = NULL;

/*----------------------------------------------------------
Take absolute value and convert to degrees
----------------------------------------------------------*/
degrees = Abs( latitude ) * semiToDeg;

/*----------------------------------------------------------
Format into DD°MM.MMM': get whole degrees
----------------------------------------------------------*/
wholeDeg = ( Int32 ) degrees;

/*----------------------------------------------------------
Get minutes from remaining degrees
----------------------------------------------------------*/
minutes = ( degrees - ( float ) wholeDeg ) * minMax;

/*----------------------------------------------------------
Get whole minutes
----------------------------------------------------------*/
wholeMin = ( Int32 ) minutes;

/*----------------------------------------------------------
Get fractional minutes as an integer by rounding
and scaling.
----------------------------------------------------------*/
fracMin = ( Int32 ) ( ( ( minutes - ( float ) wholeMin ) * minScale ) + 0.5 );

/*----------------------------------------------------------
Handle possible overflow from rounding
----------------------------------------------------------*/
if ( fracMin >= minScale )
    {
    ++wholeMin;
    fracMin -= minScale;
    if ( wholeMin >= minMax )
        {
        ++wholeDeg;
        wholeMin -= minMax;
        if ( wholeDeg >= degMax )
            {
            wholeDeg -= degMax;
            }
        }
    }

/*----------------------------------------------------------
Build into string
----------------------------------------------------------*/
AppendIntToStr( wholeDeg, 2, outStr );
StrCat( outStr, "°" );
AppendIntToStr( wholeMin, minDigits, outStr );
StrCat( outStr, "." );
AppendIntToStr( fracMin, minPrecision, outStr );
StrCat( outStr, "'" );

} /* LatitudeToString() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       LongitudeToString - Convert Longitude to String
*
*   DESCRIPTION:
*       Converts the semicircle longitude into a string.
*
*********************************************************************/
static void LongitudeToString( Int32 longitude, Char *outStr )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Char        neswStr[ 5 ];   /* "NESW" string              */
double      degrees;        /* location in degrees        */
double      minutes;        /* location in minutes        */
Int32       wholeDeg;       /* whole part of degrees      */
Int32       wholeMin;       /* whole part of minutes      */
Int32       fracMin;        /* fractional part of minutes */

/*----------------------------------------------------------
Write hemisphere character into string
----------------------------------------------------------*/
SysCopyStringResource( neswStr, neswString );
outStr[ 0 ] = ( longitude < 0 ) ? neswStr[ 3 ] : neswStr[ 1 ];
outStr[ 1 ] = NULL;

/*----------------------------------------------------------
Take absolute value and convert to degrees
----------------------------------------------------------*/
degrees = Abs( longitude ) * semiToDeg;

/*----------------------------------------------------------
Format into DD°MM.MMM': get whole degrees
----------------------------------------------------------*/
wholeDeg = ( Int32 ) degrees;

/*----------------------------------------------------------
Get minutes from remaining degrees
----------------------------------------------------------*/
minutes = ( degrees - ( float ) wholeDeg ) * minMax;

/*----------------------------------------------------------
Get whole minutes
----------------------------------------------------------*/
wholeMin = ( Int32 ) minutes;

/*----------------------------------------------------------
Get fractional minutes as an integer by rounding
and scaling.
----------------------------------------------------------*/
fracMin = ( Int32 ) ( ( ( minutes - ( float ) wholeMin ) * minScale ) + 0.5 );

/*----------------------------------------------------------
Handle possible overflow from rounding
----------------------------------------------------------*/
if ( fracMin >= minScale )
    {
    ++wholeMin;
    fracMin -= minScale;
    if ( wholeMin >= minMax )
        {
        ++wholeDeg;
        wholeMin -= minMax;
        if ( wholeDeg >= degMax )
            {
            wholeDeg -= degMax;
            }
        }
    }

/*----------------------------------------------------------
Build into string
----------------------------------------------------------*/
AppendIntToStr( wholeDeg, 3, outStr );
StrCat( outStr, "°" );
AppendIntToStr( wholeMin, minDigits, outStr );
StrCat( outStr, "." );
AppendIntToStr( fracMin, minPrecision, outStr );
StrCat( outStr, "'" );

} /* LongitudeToString() */


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

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle about box command.
----------------------------------------------------------*/
if ( command == OptionsAbout )
    {
    MenuEraseStatus( 0 );
    aboutFrmP = FrmInitForm( AboutForm );
    FrmDoDialog( aboutFrmP );
    FrmDeleteForm( aboutFrmP );
    handled = true;
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
static Boolean MainFormHandleEvent( EventPtr aEventP )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean                 handled;
FormPtr                 frmP;
QuePointType            pointData;
QueSelectAddressType    findAddress;

/*----------------------------------------------------------
Initialize.
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Let the QueAPI library handle this event first.  If this
event contains point data then create a point from the data.
----------------------------------------------------------*/
if ( QueHandleEvent( gQueAPILibRef, aEventP ) )
    {
    QueClosePoint( gQueAPILibRef, gPoint );
    gPoint = queInvalidPointHandle;
    QueCreatePointFromEvent( gQueAPILibRef, aEventP, &gPoint );
    handled = true;
    }

/*----------------------------------------------------------
Handle event.
----------------------------------------------------------*/
if ( ! handled ) switch (aEventP->eType)
    {
    case menuEvent:
        handled = MainFormDoCommand( aEventP->data.menu.itemID );
        break;

        break;

    case frmOpenEvent:
        InitMainForm();
        FrmUpdateForm( MainForm, frmRedrawUpdateCode );
        handled = true;
        break;

    case frmUpdateEvent:
        frmP = FrmGetActiveForm();
        FrmDrawForm( frmP );
        handled = true;
        break;

    case ctlSelectEvent:
        switch( aEventP->data.ctlSelect.controlID )
            {
            /*----------------------------------------------
            Create a point from lat/lon.
            ----------------------------------------------*/
            case MainCreatePointButton:
                StrNCopy( pointData.id, "lat/lon point", quePointIdLen - 1 );
                pointData.id[ quePointIdLen - 1 ] = ( char ) NULL;
                pointData.smbl        = 27;
                pointData.posn.altMSL = 123.4f / metersToFeet;
                pointData.posn.lat    = 0x1BC27C89L;
                pointData.posn.lon    = 0xBC9067A4L;
                QueClosePoint( gQueAPILibRef, gPoint );
                gPoint = queInvalidPointHandle;
                QueCreatePoint( gQueAPILibRef, &pointData, &gPoint );
                break;

            /*----------------------------------------------
            Select a point using the map.
            ----------------------------------------------*/
            case MainSelectPointMapButton:
                QueSelectPointFromMap( gQueAPILibRef, kQueAPILibExampleCreator );
                break;

            /*----------------------------------------------
            Select a point using find.
            ----------------------------------------------*/
            case MainSelectPointFindButton:
                QueSelectPointFromFind( gQueAPILibRef, kQueAPILibExampleCreator );
                break;

            /*----------------------------------------------
            Show the point Info.
            ----------------------------------------------*/
            case MainShowInfoButton:
                ShowPointInfo( gPoint );
                break;

            /*----------------------------------------------
            Show the point details.
            ----------------------------------------------*/
            case MainShowDetailsButton:
                QueViewPointDetails( gQueAPILibRef, gPoint );
                break;

            /*----------------------------------------------
            Show the point on the map.
            ----------------------------------------------*/
            case MainShowOnMapButton:
                QueViewPointOnMap( gQueAPILibRef, gPoint );
                break;

            /*----------------------------------------------
            Route to the point and remain in this app.
            ----------------------------------------------*/
            case MainRouteToPointButton:
                QueRouteToPoint( gQueAPILibRef, gPoint, false );
                break;

            /*----------------------------------------------
            Route to the point and show the map.
            ----------------------------------------------*/
            case MainRouteToShowMapButton:
                QueRouteToPoint( gQueAPILibRef, gPoint, true );
                break;

            /*----------------------------------------------
            Set "Route to" Item.
            ----------------------------------------------*/
            case MainSetRouteToButton:
                QueSetRouteToItem( gQueAPILibRef, gPoint );
                break;

            /*----------------------------------------------
            Clear "Route to" Item.
            ----------------------------------------------*/
            case MainClearRouteToButton:
                QueSetRouteToItem( gQueAPILibRef, queInvalidPointHandle );
                break;

            /*----------------------------------------------
            Process Address.
            ----------------------------------------------*/
            case MainCreateButton:
            case MainSelectButton:
            case MainCreateSelectButton:
                /*------------------------------------------
                Get the address from the fields.
                ------------------------------------------*/
                GetAddressFromFields( &gAddress );

                /*------------------------------------------
                Setup address structure.
                ------------------------------------------*/
                MemSet( &findAddress, sizeof( QueSelectAddressType ), 0 );
                findAddress.streetAddress = gAddress.street;
                findAddress.city          = gAddress.city;
                findAddress.state         = gAddress.state;
                findAddress.postalCode    = gAddress.zip;

                switch( aEventP->data.ctlSelect.controlID )
                    {
                    case MainCreateButton:
                        /*----------------------------------
                        Create a point from the address.
                        ----------------------------------*/
                        QueCreatePointFromAddress
                            ( gQueAPILibRef
                            , &findAddress
                            , kQueAPILibExampleCreator
                            );
                        break;

                    case MainSelectButton:
                        /*----------------------------------
                        Select the address from find.
                        ----------------------------------*/
                        QueSelectAddressFromFind
                            ( gQueAPILibRef
                            , &findAddress
                            , kQueAPILibExampleCreator
                            , false
                            );
                        break;

                    case MainCreateSelectButton:
                        /*----------------------------------
                        Select the address from find if it
                        couldn't be created.
                        ----------------------------------*/
                        QueSelectAddressFromFind
                            ( gQueAPILibRef
                            , &findAddress
                            , kQueAPILibExampleCreator
                            , true
                            );
                        break;
                    }
                handled = true;
                break;

            }
        break;  /* ctlSelectEvent */

    case frmCloseEvent:
        break;

    case menuOpenEvent:
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
*       SaveAddress - Save Address
*
*   DESCRIPTION:
*       Saves the address data in prefs.
*
*********************************************************************/
static void SaveAddress( const AddressType *aAddress )
{
PrefSetAppPreferences
    ( kQueAPILibExampleCreator
    , kAddressPrefID
    , kAddressPrefVersion
    , aAddress
    , sizeof( *aAddress )
    , false
    );
} /* SaveAddress() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       SavePoint - Save Point
*
*   DESCRIPTION:
*       Saves the point in prefs.
*
*********************************************************************/
static void SavePoint( const QuePointHandle aPoint )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
UInt8   *buffer;
UInt32  size;

/*----------------------------------------------------------
Serialize the point data.  Call first with 0 size to
determine the size needed for the buffer.
----------------------------------------------------------*/
size = QueSerializePoint( gQueAPILibRef, aPoint, NULL, 0 );

/*----------------------------------------------------------
Allocate buffer.
----------------------------------------------------------*/
buffer = MemPtrNew( size );
if ( buffer != NULL )
    {
    /*------------------------------------------------------
    Serialize point for real.
    ------------------------------------------------------*/
    QueSerializePoint( gQueAPILibRef, aPoint, buffer, size );

    /*------------------------------------------------------
    Save serialized data in prefs.
    ------------------------------------------------------*/
    PrefSetAppPreferences
        ( kQueAPILibExampleCreator
        , kPointPrefID
        , kPointPrefVersion
        , buffer
        , size
        , false
        );

    /*------------------------------------------------------
    Free buffer memory.
    ------------------------------------------------------*/
    MemPtrFree( buffer );
    }
} /* SavePoint() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       SetFieldTextFromHandle - Set Field Text From Handle
*
*   DESCRIPTION:
*       Sets the text to display in a field based on a memory
*       handle.
*
*********************************************************************/
static void SetFieldTextFromHandle( const UInt16 aFieldID, MemHandle aTextHandle )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FieldType   *field;
MemHandle   oldHandle;

/*----------------------------------------------------------
Get the field.
----------------------------------------------------------*/
field = ( FieldType * ) GetObjectPtr( aFieldID );

/*----------------------------------------------------------
Get the old text handle from the field.
----------------------------------------------------------*/
oldHandle = FldGetTextHandle( field );

/*----------------------------------------------------------
Set the new text handle for the field.
----------------------------------------------------------*/
FldSetTextHandle( field, aTextHandle );

/*----------------------------------------------------------
Draw the field so the new text will show up.
----------------------------------------------------------*/
FldDrawField( field );

/*----------------------------------------------------------
Free the old handle after setting the new one.
----------------------------------------------------------*/
if ( NULL != oldHandle )
    {
    MemHandleFree( oldHandle );
    oldHandle = NULL;
    }
} /* SetFieldTextFromHandle() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       SetFieldTextFromStr - Set Field Text From String
*
*   DESCRIPTION:
*       Sets the text to display in a field based on a string.
*
*********************************************************************/
static void SetFieldTextFromStr( const UInt16 aFieldID, const Char *aStr )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
MemHandle strHandle;

/*----------------------------------------------------------
Allocate space for the field.
----------------------------------------------------------*/
strHandle = MemHandleNew( StrLen( aStr ) + 1 );

/*----------------------------------------------------------
Copy the string.
----------------------------------------------------------*/
StrCopy( ( Char * ) MemHandleLock( strHandle ), aStr );

/*----------------------------------------------------------
Unlock the memory.
----------------------------------------------------------*/
MemHandleUnlock( strHandle );

/*----------------------------------------------------------
Set the field to the handle. We do NOT have to free the
string because it now owned by the field.
----------------------------------------------------------*/
SetFieldTextFromHandle( aFieldID, strHandle );

} /* end of function SetFieldTextFromStr */


/*********************************************************************
*
*   PROCEDURE NAME:
*       ShowPointInfo - Show Point Info
*
*   DESCRIPTION:
*       Shows the information for the point.
*
*********************************************************************/
static void ShowPointInfo( const QuePointHandle aPoint )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
QueErrT16       error;
QuePointType    pointInfo;
Char            *msg;
Char            *altStr;
Char            *latStr;
Char            *lonStr;

/*----------------------------------------------------------
Get the point information.
----------------------------------------------------------*/
error = QueGetPointInfo( gQueAPILibRef, aPoint, &pointInfo );

/*----------------------------------------------------------
Display the information if no error.
----------------------------------------------------------*/
if ( error == queErrNone )
    {
    /*------------------------------------------------------
    Allocate memory for strings.
    ------------------------------------------------------*/
    msg    = MemPtrNew( 200 );
    altStr = MemPtrNew( 30 );
    latStr = MemPtrNew( 30 );
    lonStr = MemPtrNew( 30 );

    /*------------------------------------------------------
    Build and display message.
    ------------------------------------------------------*/
    if ( pointInfo.posn.altMSL >= queInvalidAltitude )
        {
        StrCopy( altStr, "invalid" );
        }
    else
        {
        AltitudeToString(  pointInfo.posn.altMSL, altStr );
        }
    LatitudeToString(  pointInfo.posn.lat,    latStr );
    LongitudeToString( pointInfo.posn.lon,    lonStr );
    StrPrintF
        ( msg
        , "Name:\n%s\n\nsymbol: %u\nLat: %s\nLon: %0s\nAlt: %s"
        , pointInfo.id
        , pointInfo.smbl
        , latStr
        , lonStr
        , altStr
        );
    FrmCustomAlert( InformationAlert, msg, NULL, NULL );

    /*------------------------------------------------------
    Free memory.
    ------------------------------------------------------*/
    MemPtrFree( msg );
    MemPtrFree( altStr );
    MemPtrFree( latStr );
    MemPtrFree( lonStr );
    }

/*----------------------------------------------------------
Otherwise indicate there was an error.
----------------------------------------------------------*/
else
    {
    FrmCustomAlert( InformationAlert, "Couldn't get point information.", NULL, NULL );
    }


} /* ShowPointInfo() */
