/*********************************************************************
*
*   MODULE NAME:
*       GPSInfo - GPS Information Page
*
*   DESCRIPTION:
*       Processing for the GPS Information page.
*
*   PUBLIC PROCEDURES:
*       Name                            Title
*       -----------------------         --------------------------------------
*       PilotMain                       Pilot Main
*
*   PRIVATE PROCEDURES:
*       Name                            Title
*       -----------------------         --------------------------------------
*
*   LOCAL PROCEDURES:
*       Name                            Title
*       -----------------------         --------------------------------------
*       AboutFormHandleEvent            About Form Handle Events
*       AboutFormInit                   About Form Initialization
*       AltitudeToString                Convert Altitude to String
*       AppendIntToStr                  Append Integer to String with Leading Zeros
*       AppHandleEvent                  Applicate Handle Event
*       DrawCenteredString              Draw a Centered String
*       DrawCircle                      Draw a Circle
*       DrawSatData                     Draw Satellite Data Form Graphics
*       DrawSatStatus                   Draw Satellite Status Form Graphics
*       DrawString                      Draw a String
*       EventLoop                       Event Loop
*       LatitudeToString                Convert Latitude to String
*       LongitudeToString               Convert Longitude to String
*       SatDataFormHandleEvent          Satellite Data Form Handle Event
*       SatStatusFormHandleEvent        Satellite Status Form Handle Event
*       SatStatusFormHandleMenuEvent    Satellite Status Form Handle Menu Event
*       StartApp                        Start Application
*       StopApp                         Stop Application
*       VelocityToString                Convert Velocity to String
*
*   NOTES:
*
*   Copyright 2001-2003 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

/*--------------------------------------------------------------------
                            GENERAL INCLUDES
--------------------------------------------------------------------*/

#include <PalmOS.h>
#include <MathLib.h>

#include "GPSInfo_res.h"
#include "GPSLib68K.h"

/*--------------------------------------------------------------------
                                 MACROS
--------------------------------------------------------------------*/

#define cntOfArray( n )                 ( sizeof( n ) / sizeof( ( n ) [ 0 ] ) )
#define min( a, b )                     ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define max( a, b )                     ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define limit( lower, value, upper )    ( min( max( lower, value ), upper ) )
#define round( n )                      ( ( Int32 ) ( ( n ) < 0.0f ) ? ( ( n ) - 0.5f ) : ( ( n ) + 0.5f ) )

/*--------------------------------------------------------------------
                            LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*------------------------------------------------------
General
------------------------------------------------------*/
#define formTitleHeight     15                          /* height of form title             */
#define monoColorDepth      2                           /* 2 bits (4 grayshades)            */
#define colorColorDepth     8                           /* 8 bits (256 colors)              */
#define minDigits           2                           /* 2 digits for whole minutes       */
#define minPrecision        3                           /* 3 digits of minute precision     */
#define minScale            1000                        /* minute scaling                   */
#define degMax              360                         /* max degree value                 */
#define minMax              60                          /* max minute value                 */
#define snrMax              5000                        /* 50 dB Hz                         */
#define snrMin              3000                        /* 30 dB Hz                         */
#define snrRng              ( snrMax - snrMin )         /* signal-to-noise ratio range      */
#define eventWaitSeconds    2                           /* wait 2 seconds for event         */
#define numDisplayableSats  12                          /* max. number of satellites
                                                           that can currently be
                                                           displayed                        */

/*------------------------------------------------------
Conversions
------------------------------------------------------*/
#define pi                  3.1415926535898             /* ratio of the circumference
                                                           to the diameter of a circle      */
#define degToRad            ( pi / 180.0 )              /* degrees to radians               */
#define radToDeg            ( 180.0 / pi )              /* radians to degrees               */
#define semiToDeg           ( 180.0 / 2147483648.0 )    /* semicircles to degrees           */
#define mpsToMph            2.2369                      /* meters per second to mph         */
#define metersToFeet        3.2808                      /* meters to feet                   */
#define secondsPerFrac      ( 1.0 / 4294967296.0 )      /* Seconds per fractional second    */
#define secondsPerMinute    60.0                        /* Seconds per minute               */
#define secondsPerHour      ( secondsPerMinute * 60.0 ) /* Seconds per hour                 */

/*------------------------------------------------------
Screen layout
------------------------------------------------------*/
#define skyvwRadPercent 20                              /* Radius is 20% of width of screen */
#define rectBorderWidth 1                               /* Rectangle border width           */

/*----------------------------------------------------------
GPSSatDataType Status Bitfield
----------------------------------------------------------*/
#define satEphShift     0
#define satNoEph        ( 0 << satEphShift )            /* no ephemeris                     */
#define satEph          ( 1 << satEphShift )            /* has ephemeris                    */

#define satDifShift     1
#define satNoDif        ( 0 << satDifShift )            /* no differential correction       */
#define satDif          ( 1 << satDifShift )            /* has differential correction      */

#define satUsedShift    2
#define satNotUsed      ( 0 << satUsedShift )           /* not used in solution             */
#define satUsed         ( 1 << satUsedShift )           /* used in solution                 */

#define satRisingShift  3
#define satSetting      ( 0 << satRisingShift )         /* satellite setting                */
#define satRising       ( 1 << satRisingShift )         /* satellite rising                 */

/*------------------------------------------------------
List selection items
------------------------------------------------------*/
enum
    {
    satStatusListItem   = 0,
    satDataListItem
    };

/*------------------------------------------------------
Grayscale color values
------------------------------------------------------*/
enum
    {
    grayScaleWhite      = 0,
    grayScaleLightGray,
    grayScaleDarkGray,
    grayScaleBlack
    };

/*--------------------------------------------------------------------
                                 TYPES
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Color index
----------------------------------------------------------*/
typedef UInt8 colorIndexType; enum
    {
    backgroundColor,
    scaleColor,
    noEphColor,
    ephColor,

    ColorCount
    };

/*--------------------------------------------------------------------
                            PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                            MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                               VARIABLES
--------------------------------------------------------------------*/

WinHandle           gOffScreenWindow;           /* handle to offscreen drawing window   */
UInt16              gGpsLibRef;                 /* GPS Library reference                */
UInt32              gOldDepth;                  /* original display depth               */
IndexedColorType    gColorTable[ ColorCount ];  /* colors for various elements          */

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

static Boolean  AboutFormHandleEvent( EventPtr event );
static void     AltitudeToString( float altitude, Char *outStr );
static void     AppendIntToStr( Int32 number, int width, Char *outStr );
static Boolean  AppHandleEvent( EventPtr event );
static void     DrawCenteredString( Char *str, UInt16 boundsLeft, UInt16 boundsRight, UInt16 yPos );
static void     DrawCircle( UInt16 x, UInt16 y, UInt16 radius, IndexedColorType DrawColor, IndexedColorType FillColor );
static void     DrawSatData( void );
static void     DrawSatStatus( void );
static void     DrawString( Char *str, UInt16 x, UInt16 y );
static void     EventLoop( void );
static Err      GpsLibEventCallback( SysNotifyParamType *notifyParamsP );
static void     LatitudeToString( Int32 latitude, Char *outStr );
static void     LongitudeToString( Int32 longitude, Char *outStr );
static Boolean  SatDataFormHandleEvent( EventPtr event );
static Boolean  SatStatusFormHandleEvent( EventPtr event );
static Boolean  SatStatusFormHandleMenuEvent( UInt16 menuID );
static Err      StartApp( void );
static void     StopApp( void );
static void     VelocityToString( float velocity, Char *outStr );


/*********************************************************************
*
*   PROCEDURE NAME:
*       PilotMain - Pilot Main
*
*   DESCRIPTION:
*       Entry point for application.
*
*********************************************************************/
UInt32 PilotMain( UInt16 launchCode, MemPtr cmdPBP, UInt16 launchFlags )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Err err;    /* Error value  */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
err = 0;

/*----------------------------------------------------------
Process
----------------------------------------------------------*/
if ( launchCode == sysAppLaunchCmdNormalLaunch )
    {
    err = StartApp();

    if ( err == 0 )
        {
        EventLoop();
        StopApp();
        }
    }

return err;

} /* PilotMain() */


#pragma mark -


/*********************************************************************
*
*   PROCEDURE NAME:
*       AboutFormHandleEvent - About Form Handle Events
*
*   DESCRIPTION:
*       Handles events for the About Form.
*
*********************************************************************/
static Boolean AboutFormHandleEvent( EventPtr event )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean handled;    /* message handled? */
FormPtr form;       /* pointer to form  */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle events
----------------------------------------------------------*/
switch ( event->eType )
    {
    /*------------------------------------------------------
    Open form
    ------------------------------------------------------*/
    case frmOpenEvent:
        form = FrmGetActiveForm();
        FrmDrawForm( form );
        handled = true;
        break;

    /*------------------------------------------------------
    User selected a control
    ------------------------------------------------------*/
    case ctlSelectEvent:
        if ( event->data.ctlSelect.controlID == AboutOKButton )
            {
            FrmReturnToForm( 0 );
            handled = true;
            }
        break;

    default:
        break;
    }

return handled;

} /* AboutFormHandleEvent() */


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
*       AppHandleEvent - Application Handle Event
*
*   DESCRIPTION:
*       Handles the events for the application.
*
*********************************************************************/
static Boolean AppHandleEvent( EventPtr event )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
FormPtr frm;        /* pointer to form  */
UInt16  form_id;    /* ID of form       */
Boolean handled;    /* event handled?   */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle event.
----------------------------------------------------------*/
if ( event->eType == frmLoadEvent )
    {
    form_id = event->data.frmLoad.formID;
    frm = FrmInitForm( form_id );
    FrmSetActiveForm( frm );

    switch ( form_id )
        {
        case SatStatusForm:
            FrmSetEventHandler( frm, SatStatusFormHandleEvent );
            break;

        case SatDataForm:
            FrmSetEventHandler( frm, SatDataFormHandleEvent );
            break;

        case AboutForm:
            FrmSetEventHandler( frm, AboutFormHandleEvent );
            break;
        }
    handled = true;
    }

return handled;

} /* AppHandleEvent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       DrawCenteredString - Draw a Centered String
*
*   DESCRIPTION:
*       Draws the string at the given y coordinate, centered between
*       the right and left bounds.
*
*********************************************************************/
static void DrawCenteredString( Char *str, UInt16 boundsLeft, UInt16 boundsRight, UInt16 yPos )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
UInt16  length; /* length of the string                 */
UInt16  width;  /* width of the string                  */
UInt16  xPos;   /* horizontal position of the string    */

/*----------------------------------------------------------
Find the width of the string in pixels.
----------------------------------------------------------*/
length = StrLen( str );
width  = FntCharsWidth( str, length );

/*----------------------------------------------------------
Draw the string centered within the bounds.
----------------------------------------------------------*/
xPos = ( boundsLeft + boundsRight - width ) / 2;
WinDrawChars( str, length, xPos, yPos );

} /* DrawCenteredString() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       DrawCircle - Draw a Circle
*
*   DESCRIPTION:
*       Draws a circle with the given radius at the given coordinates
*       in the given color and fills it with the given fill color.
*
*********************************************************************/
static void DrawCircle( UInt16 x, UInt16 y, UInt16 radius, IndexedColorType drawColor, IndexedColorType fillColor )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
RectangleType       circleRect; /* bounds of the circle */
IndexedColorType    oldColor;   /* old foreground color */

circleRect.topLeft.x = x - radius;
circleRect.topLeft.y = y - radius;
circleRect.extent.x  = radius * 2;
circleRect.extent.y  = radius * 2;

/*----------------------------------------------------------
Change the foreground drawing color to the specified color,
but make sure to save the old color.  Then, draw the
rectangle/circle.
----------------------------------------------------------*/
oldColor = WinSetForeColor( drawColor );
WinDrawRectangle( &circleRect, radius );

/*----------------------------------------------------------
If the circle is to be filled with a color different than
the one it was drawn with, draw a slightly smaller
rectangle/circle on top of the original.
----------------------------------------------------------*/
if ( fillColor != drawColor )
    {
    circleRect.topLeft.x += 1;
    circleRect.topLeft.y += 1;
    circleRect.extent.x  -= 2;
    circleRect.extent.y  -= 2;

    WinSetForeColor( fillColor );
    WinDrawRectangle( &circleRect, radius - 1 );
    }

/*----------------------------------------------------------
Return the foreground color to its original value.
----------------------------------------------------------*/
WinSetForeColor( oldColor );

} /* DrawCircle() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       DrawSatData - Draw the Satellite Data Form Graphics
*
*   DESCRIPTION:
*       Draws the Satellite Data Form's graphics.
*
*********************************************************************/
static void DrawSatData( void )
{
/*----------------------------------------------------------
Literal Constants
----------------------------------------------------------*/
enum
    {
    spaceDigits = 2 /* # of digits for column spacing   */
    };

/*----------------------------------------------------------
Types
----------------------------------------------------------*/
typedef UInt8 dataT8; enum
    {
    unsigned8Bit,
    signed16Bit,
    float32Bit
    };

/*----------------------------------------------------------
Constants
----------------------------------------------------------*/
static const struct
    {
    UInt8   dgts;       /* # of digits in column                        */
    Int16   titleId;    /* string id of title of column                 */
    UInt32  dataOffset; /* offset of data from beginning of structure   */
    dataT8  dataType;   /* data type of column                          */
    int     mask;       /* mask for data in column                      */
    int     shift;      /* shift value for data in column               */
    }
layoutTable[] =
    {
    { 3, svidString, OffsetOf( GPSSatDataType, svid       ), unsigned8Bit,  ~0,               0              },
    { 4, snrString,  OffsetOf( GPSSatDataType, snr        ), signed16Bit,   ~0,               0              },
    { 3, elevString, OffsetOf( GPSSatDataType, elevation  ), float32Bit,    ~0,               0              },
    { 4, azimString, OffsetOf( GPSSatDataType, azimuth    ), float32Bit,    ~0,               0              },
    { 1, eString,    OffsetOf( GPSSatDataType, status     ), unsigned8Bit,  gpsSatEphMask,    satEphShift    }, /* has ephemeris?           */
    { 1, dString,    OffsetOf( GPSSatDataType, status     ), unsigned8Bit,  gpsSatDifMask,    satDifShift    }, /* differential correction? */
    { 1, uString,    OffsetOf( GPSSatDataType, status     ), unsigned8Bit,  gpsSatUsedMask,   satUsedShift   }, /* used in solution?        */
    { 1, rString,    OffsetOf( GPSSatDataType, status     ), unsigned8Bit,  gpsSatRisingMask, satRisingShift }  /* rising?                  */
    };
#define layoutTableSize cntOfArray( layoutTable )


/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
RectangleType       drawRect;       /* draw window rectangle data   */
GPSSatDataType      *satData;       /* satellite data               */
UInt8               numSats;        /* max Number of Satellites     */
UnderlineModeType   oldUnderline;   /* old underline                */
WinHandle           oldWindow;      /* old window handle            */
char                buf[ 10 ];      /* string buffer                */
void                *dataAddress;   /* data address                 */
int                 columnStart;    /* column starting pixel        */
int                 i;              /* loop counter                 */
int                 j;              /* another loop counter         */
int                 lineHeight;     /* space between lines          */
int                 nCharWidth;     /* number character width       */
int                 tempVal;        /* temporary value              */

/*----------------------------------------------------------
Drawing setup
----------------------------------------------------------*/
oldWindow = WinGetDrawWindow();
WinSetDrawWindow( gOffScreenWindow );
WinEraseWindow();

/*----------------------------------------------------------
Find the width of a number character
----------------------------------------------------------*/
nCharWidth = FntCharWidth( '0' );

/*----------------------------------------------------------
Determine the line spacing
----------------------------------------------------------*/
WinGetWindowFrameRect( gOffScreenWindow, &drawRect );
lineHeight = drawRect.extent.y / ( numDisplayableSats + 1 );

/*----------------------------------------------------------
Display the headings
----------------------------------------------------------*/
columnStart  = 0;
oldUnderline = WinSetUnderlineMode( solidUnderline );
for ( i = 0; i < layoutTableSize; ++i )
    {
    SysCopyStringResource( buf, layoutTable[ i ].titleId );
    WinDrawChars( buf, StrLen( buf ), columnStart, 0 );
    columnStart += ( ( layoutTable[ i ].dgts + spaceDigits ) * nCharWidth );
    }
WinSetUnderlineMode( oldUnderline );

/*----------------------------------------------------------
Set up the Satellite Array
----------------------------------------------------------*/
numSats = GPSGetMaxSatellites( gGpsLibRef );
satData = MemPtrNew( numSats * sizeof( GPSSatDataType ) );

/*----------------------------------------------------------
Get satellite data
----------------------------------------------------------*/
if ( GPSGetSatellites( gGpsLibRef, satData ) != 0 )
    {
    MemSet( satData, numSats * sizeof( GPSSatDataType ), -1 );
    }

/*----------------------------------------------------------
Iterate through the satellites and draw the satellite data.
----------------------------------------------------------*/
for ( i = 0; i < numDisplayableSats; ++i )
    {
    columnStart = 0;
    for ( j = 0; j < layoutTableSize; ++j )
        {
        /*--------------------------------------------------
        Get data
        --------------------------------------------------*/
        dataAddress = ( void * ) ( ( UInt32 ) &satData[ i ] + layoutTable[ j ].dataOffset );
        switch ( layoutTable[ j ].dataType )
            {
            case unsigned8Bit:
                tempVal = *( ( UInt8 * ) dataAddress );
                break;

            case signed16Bit:
                tempVal = *( ( Int16 * ) dataAddress );
                break;

            case float32Bit:
                /*--------------------------------------------------
                Convert from Radians to Degrees for any data that
                needs it
                --------------------------------------------------*/
                if( ( layoutTable[ j ].dataOffset == OffsetOf( GPSSatDataType, elevation  ) ) ||
                    ( layoutTable[ j ].dataOffset == OffsetOf( GPSSatDataType, azimuth    ) )
                  )
                    {
                    tempVal = ( *( ( float * ) dataAddress ) * radToDeg );
                    }

                break;
            } /* switch */

        /*--------------------------------------------------
        Mask, shift, and convert to a string
        --------------------------------------------------*/
        tempVal = ( tempVal & layoutTable[ j ].mask ) >> layoutTable[ j ].shift;
        StrIToA( buf, tempVal );

        /*--------------------------------------------------
        Draw and determine where next column starts
        --------------------------------------------------*/
        WinDrawChars( buf, StrLen( buf ), columnStart, ( i + 1 ) * lineHeight );
        columnStart += ( ( layoutTable[ j ].dgts + spaceDigits ) * nCharWidth );
        } /* for j */
    } /* for i */

/*----------------------------------------------------------
Squirt it to the screen
----------------------------------------------------------*/
WinSetDrawWindow( oldWindow );
WinCopyRectangle( gOffScreenWindow, oldWindow, &drawRect, 0, formTitleHeight, winPaint );

/*----------------------------------------------------------
Free the Satellite Array memory
----------------------------------------------------------*/
MemPtrFree( satData );
satData = NULL;

} /* DrawSatData() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       DrawSatStatus - Draw the Satellite Status Form Graphics
*
*   DESCRIPTION:
*       Draws the Satellite Status Form's graphics.
*
*********************************************************************/
static void DrawSatStatus( void )
{
/*----------------------------------------------------------
Memory constants
----------------------------------------------------------*/

/*----------------------------------------------------------
Array of status string id's for PVT status. Must be in the
same order as GPSFixType in GPSLib.H.
----------------------------------------------------------*/
static Int16 pvtStatusId[] =
    {
    lostSatRecepString,
    lostSatRecepString,
    d2FixString,
    d3FixString,
    d2DiffFixString,
    d3DiffFixString
    };

/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Char                dateStr[ longDateStrLength ];       /* date string                                              */
Char                svidStr[ numDisplayableSats ][ 4 ]; /* array of SVID strings                                    */
Char                tempStr[ 32 ];                      /* temporary string                                         */
Char                timeStr[ timeStringLength + 6 ];    /* time string                                              */
UInt8               elevDir[ numDisplayableSats ];      /* elevation sort directory array (smallest first)          */
UInt8               svidDir[ numDisplayableSats ];      /* SVID sort directory array (smallest first)               */
UInt8               numSats;                            /* max number of Satellites                                 */
Coord               skyvwRadius;                        /* skyview radius                                           */
Coord               xDiff;                              /* x-coordinate of differential indicator                   */
Coord               xIn;                                /* inner x spacing                                          */
Coord               xOut;                               /* outer x spacing                                          */
Coord               y;                                  /* y-coordinate value                                       */
Coord               yExtra;                             /* extra y spacing                                          */
DateType            date;                               /* date                                                     */
IndexedColorType    color;                              /* drawing color                                            */
PointType           skyvwCenter;                        /* skyview center                                           */
GPSPVTDataType      pvtData;                            /* PVT data                                                 */
RectangleType       drawRect;                           /* drawing window rectangle data                            */
RectangleType       sgnstrRect;                         /* signal strength area                                     */
RectangleType       tempRect;                           /* temporary rectangle for drawing                          */
GPSSatDataType      *satData;                           /* satellite data                                           */
TimeType            time;                               /* time                                                     */
WinDrawOperation    drawMode;                           /* drawing mode                                             */
WinHandle           oldWindow;                          /* old window handle                                        */
double              utcSeconds;                         /* # of UTC seconds since the beginning of the current day  */
float               angleRadians;                       /* angle in radians                                         */
float               snrToBar;                           /* conversion factor from Signal-to-Noise Ratio to signal
                                                           strength bar height                                      */
Int32               seconds;                            /* # of seconds today                                       */
int                 i;                                  /* loop index                                               */
int                 j;                                  /* another loop index                                       */
int                 sortLimit;                          /* sort limit                                               */
int                 temp;                               /* temporary storage                                        */
Int16               signalLevel;                        /* signal level                                             */
Int16               statusHeight;                       /* status string height                                     */
UInt16              adjElev;                            /* elevatiaon adjustment variable                           */
Boolean             changed;                            /* changed during pass through the data?                    */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
for ( i = 0; i < numDisplayableSats; ++i )
    {
    svidDir[ i ] = i;
    elevDir[ i ] = i;
    }

/*----------------------------------------------------------
Drawing setup
----------------------------------------------------------*/
oldWindow = WinGetDrawWindow();
WinSetDrawWindow( gOffScreenWindow );
WinEraseWindow();
WinGetWindowFrameRect( gOffScreenWindow, &drawRect );

/*----------------------------------------------------------
Satellite Array setup
----------------------------------------------------------*/
numSats = GPSGetMaxSatellites( gGpsLibRef );
satData = MemPtrNew( numSats * sizeof( GPSSatDataType ) );

/*----------------------------------------------------------
Get the satellite and PVT data
----------------------------------------------------------*/
if ( GPSGetSatellites( gGpsLibRef, satData ) != 0 )
    {
    MemSet( satData, numSats * sizeof( GPSSatDataType ), -1 );
    }

if ( GPSGetPVT( gGpsLibRef, &pvtData ) != 0 )
    {
    pvtData.status.fix = gpsFixUnusable;
    }

/*----------------------------------------------------------
Fill the SVID and elevation directory arrays so that the
indexes are sorted by the SVID and elevation with the
smallest first. Until there are no more changes left, move
the index of the largest value to the last element, then
look only at the rest of the data.
----------------------------------------------------------*/
sortLimit = numDisplayableSats;
do {
    changed = false;
    for ( i = 1; i < sortLimit; ++i )
        {
        if ( satData[ svidDir[ i ] ].svid < satData[ svidDir[ i - 1 ] ].svid )
            {
            changed          = true;
            temp             = svidDir[ i ];
            svidDir[ i ]     = svidDir[ i - 1 ];
            svidDir[ i - 1 ] = temp;
            }
        }
    --sortLimit;
    }
while ( changed );

sortLimit = numDisplayableSats;
do {
    changed = false;
    for ( i = 1; i < sortLimit; ++i )
        {
        if ( satData[ elevDir[ i ] ].elevation < satData[ elevDir[ i - 1 ] ].elevation )
            {
            changed          = true;
            temp             = elevDir[ i ];
            elevDir[ i ]     = elevDir[ i - 1 ];
            elevDir[ i - 1 ] = temp;
            }
        }
    --sortLimit;
    }
while ( changed );

/*----------------------------------------------------------
Build the svid strings
----------------------------------------------------------*/
for ( i = 0; i < numDisplayableSats; ++i )
    {
    /*------------------------------------------------------
    Invalid id, fill with dashes
    ------------------------------------------------------*/
    if ( satData[ i ].svid == gpsInvalidSVID )
        {
        StrCopy( svidStr[ i ], "--" );
        }
    /*------------------------------------------------------
    Single digit, add leading zero
    ------------------------------------------------------*/
    else if ( satData[ i ].svid < 10 )
        {
        svidStr[ i ][ 0 ] = '0';
        StrIToA( &svidStr[ i ][ 1 ] , satData[ i ].svid );
        }
    /*------------------------------------------------------
    Just convert
    ------------------------------------------------------*/
    else
        {
        StrIToA( svidStr[ i ], satData[ i ].svid );
        }
    }

/*----------------------------------------------------------
Draw the status string
----------------------------------------------------------*/
FntSetFont( boldFont );
SysCopyStringResource( tempStr, pvtStatusId[ pvtData.status.fix ] );
DrawCenteredString( tempStr, drawRect.topLeft.x, ( drawRect.topLeft.x + drawRect.extent.x ), drawRect.topLeft.y );
statusHeight = FntCharHeight();
FntSetFont( stdFont );

/*----------------------------------------------------------
Draw skyview field.  Azimuth is in radians from north
(0-2pi) and elevation is in radians above the horizon
(0-pi/2).
----------------------------------------------------------*/
SysCopyStringResource( tempStr, neswString );

/*----------------------------------------------------------
Determine the radius and center of skyview.
----------------------------------------------------------*/
skyvwRadius = drawRect.extent.x / ( 100 / skyvwRadPercent );
skyvwCenter.x = skyvwRadius + ( FntCharWidth( tempStr[ 3 ] ) / 2 ) + 1;
skyvwCenter.y = skyvwRadius + statusHeight + ( FntCharHeight() / 2 ) + 1;

/*----------------------------------------------------------
Draw the skyview circles.
----------------------------------------------------------*/
DrawCircle( skyvwCenter.x, skyvwCenter.y, skyvwRadius,     gColorTable[ scaleColor ], gColorTable[ backgroundColor ] );
DrawCircle( skyvwCenter.x, skyvwCenter.y, skyvwRadius / 2, gColorTable[ scaleColor ], gColorTable[ backgroundColor ] );
DrawCircle( skyvwCenter.x, skyvwCenter.y, 1,               gColorTable[ scaleColor ], gColorTable[ backgroundColor ] );

/*----------------------------------------------------------
Draw direction characters
----------------------------------------------------------*/
WinPushDrawState();
WinSetTextColor( gColorTable[ scaleColor ] );
tempRect.extent.y = FntCharHeight();
for ( i = 0; i < 4; ++i )
    {
    /*------------------------------------------------------
    Find size of character
    ------------------------------------------------------*/
    tempRect.extent.x = FntCharWidth( tempStr[ i ] );

    /*------------------------------------------------------
    Calculate x,y location
    ------------------------------------------------------*/
    angleRadians = i * 90 * degToRad;
    tempRect.topLeft.x = skyvwCenter.x + ( skyvwRadius * sin( angleRadians ) ) - ( tempRect.extent.x / 2 );
    tempRect.topLeft.y = skyvwCenter.y - ( skyvwRadius * cos( angleRadians ) ) - ( tempRect.extent.y / 2 );

    /*------------------------------------------------------
    Clear a space and draw the character
    ------------------------------------------------------*/
    WinEraseRectangle( &tempRect, 0 );
    WinDrawChar( tempStr[ i ], tempRect.topLeft.x + 1, tempRect.topLeft.y - 1 );
    }
WinPopDrawState();

/*----------------------------------------------------------
Draw svids sorted by elevation
----------------------------------------------------------*/

/*----------------------------------------------------------
Save current drawing state
----------------------------------------------------------*/
WinPushDrawState();

/*----------------------------------------------------------
Set up size of rectangles
----------------------------------------------------------*/
tempRect.extent.y = FntBaseLine() - FntDescenderHeight() + 2;
tempRect.extent.x = FntCharsWidth( "00", 2 ) + 1;

for ( i = 0; i < numDisplayableSats; ++i )
    {
    /*------------------------------------------------------
    Sort by elevation
    ------------------------------------------------------*/
    j = elevDir[ i ];

    if ( satData[ j ].svid < gpsInvalidSVID )
        {
        /*--------------------------------------------------
        Determine location of characters
        --------------------------------------------------*/
        tempRect.topLeft.x  = skyvwCenter.x - ( tempRect.extent.x / 2 );
        tempRect.topLeft.y  = skyvwCenter.y - ( tempRect.extent.y / 2 );
        adjElev             = ( (0.5 * pi) - satData[j].elevation ) * ( skyvwRadius / (0.5 * pi) );
        tempRect.topLeft.x += adjElev * sin( satData[ j ].azimuth );
        tempRect.topLeft.y -= adjElev * cos( satData[ j ].azimuth );

        /*--------------------------------------------------
        Set colors and draw mode:
        1. black on white if no signal
        2. black on lt gray if no ephemeris
        3. white on black if ephemeris
        --------------------------------------------------*/
        if ( satData[ j ].snr < 0 )
            {
            color    = gColorTable[ backgroundColor ];
            drawMode = winOverlay;
            }
        else if ( ( satData[ j ].status & gpsSatEphMask ) != satEph )
            {
            color    = gColorTable[ noEphColor ];
            drawMode = winOverlay;
            }
        else
            {
            color    = gColorTable[ ephColor ];
            drawMode = winMask;
            }

        /*--------------------------------------------------
        Draw rectangle and svid text
        --------------------------------------------------*/
        WinSetForeColor( color );
        WinDrawRectangle( &tempRect, 0 );

        WinSetDrawMode( drawMode );
        WinPaintChars( svidStr[ j ], StrLen( svidStr[ j ] ), tempRect.topLeft.x + 1, tempRect.topLeft.y - 1 );
        } /* if SVID is valid */
    } /* for i */
WinPopDrawState();

/*----------------------------------------------------------
Draw signal strength bars
----------------------------------------------------------*/

/*----------------------------------------------------------
Get the differential character
----------------------------------------------------------*/
SysCopyStringResource( tempStr, diffCharString );

/*----------------------------------------------------------
Determine height of signal strength area and conversion
factor from SNR to bar height.
----------------------------------------------------------*/
sgnstrRect.topLeft.y = skyvwCenter.y + skyvwRadius + ( FntCharHeight() / 2 ) + 1;
sgnstrRect.extent.y  = drawRect.extent.y - sgnstrRect.topLeft.y - FntBaseLine() + 1;
snrToBar             = ( ( float ) sgnstrRect.extent.y ) / ( ( float ) snrRng );

/*----------------------------------------------------------
Draw the scale lines (10, 30, 50, 70, 90)
----------------------------------------------------------*/
WinPushDrawState();
WinSetForeColor( gColorTable[ scaleColor ] );
for ( i = 1; i < 10; i += 2 )
    {
    y = sgnstrRect.topLeft.y + sgnstrRect.extent.y - ( round( i * snrRng / 10 ) * snrToBar );
    WinDrawLine( drawRect.topLeft.x, y, drawRect.topLeft.x + drawRect.extent.x, y );
    }
WinPopDrawState();

/*----------------------------------------------------------
Width of signal strength bar is based on the width of the
SVID string minus the rectangle border.
----------------------------------------------------------*/
tempRect.extent.x -= ( 2 * rectBorderWidth );

/*----------------------------------------------------------
Determine the spacing of the bars, both between bars (inner)
and at the edges (outer).
----------------------------------------------------------*/
xIn  = ( tempRect.extent.x + ( 2 * rectBorderWidth ) ) * numDisplayableSats;
xIn  = ( drawRect.extent.x - xIn ) / ( numDisplayableSats + 1 );
xOut = ( drawRect.extent.x - ( ( tempRect.extent.x + ( 2 * rectBorderWidth ) + xIn ) * numDisplayableSats ) + xIn ) / 2;

/*----------------------------------------------------------
Draw the SVID string & signal strength bars sorted by SVID
----------------------------------------------------------*/
for ( i = 0; i < numDisplayableSats ; ++i )
    {
    /*------------------------------------------------------
    Sort by SVID
    ------------------------------------------------------*/
    j = svidDir[ i ];

    /*------------------------------------------------------
    Draw SVID string
    ------------------------------------------------------*/
    DrawString
        ( svidStr[ j ]
        , ( xOut + ( i * ( tempRect.extent.x + ( 2 * rectBorderWidth ) + xIn ) ) )
        , drawRect.extent.y - FntBaseLine()
        );

    /*------------------------------------------------------
    Draw bar if channel is being used
    ------------------------------------------------------*/
    if ( satData[ j ].svid < gpsInvalidSVID )
        {
        /*--------------------------------------------------
        Determine height of bar.  Compensate for the border
        being drawn outside the rectangle.
        --------------------------------------------------*/
        signalLevel       = limit( snrMin, max( 0, satData[ j ].snr ), snrMax ) - snrMin;
        tempRect.extent.y = ( ( Coord ) round( signalLevel * snrToBar ) ) + 1 - ( 2 * rectBorderWidth );
        tempRect.extent.y = max( 0, tempRect.extent.y );

        tempRect.topLeft.x = xOut + ( i * ( tempRect.extent.x + ( 2 * rectBorderWidth ) + xIn ) ) + 1;
        tempRect.topLeft.y = sgnstrRect.topLeft.y + sgnstrRect.extent.y - tempRect.extent.y - rectBorderWidth;

        /*--------------------------------------------------
        Set colors:
        1. black frame with lt gray fill if no ephemeris
        2. black frame with black fill if ephemeris
        --------------------------------------------------*/
        if ( ( satData[ j ].status & gpsSatEphMask ) != satEph )
            {
            WinSetForeColor( gColorTable[ noEphColor ] );
            }

        /*--------------------------------------------------
        Draw fill rectangle and frame
        --------------------------------------------------*/
        WinDrawRectangle( &tempRect, 0 );
        WinSetForeColor( gColorTable[ ephColor ] );
        WinDrawRectangleFrame( simpleFrame, &tempRect );

        /*--------------------------------------------------
        Draw differential indicator if necessary
        --------------------------------------------------*/
        if ( ( satData[ j ].status & gpsSatDifMask ) == satDif )
            {
            /*----------------------------------------------
            Determine x-coordinate
            ----------------------------------------------*/
            xDiff = tempRect.topLeft.x + ( ( tempRect.extent.x - FntCharWidth( 'D' ) + 1 ) / 2 );

            /*----------------------------------------------
            Put indicator inside or outside the bar based on
            the height of the bar.
            ----------------------------------------------*/
            if ( tempRect.extent.y < FntCharHeight() + ( 2 * rectBorderWidth ) )
                {
                /*------------------------------------------
                Outside: Black on white; also draw a white
                line on the left side of the character.
                ------------------------------------------*/
                y = sgnstrRect.topLeft.y + sgnstrRect.extent.y - ( 2 * FntCharHeight() ) - rectBorderWidth - 3;
                WinDrawChars( tempStr, 1, xDiff, y );
                color = WinSetForeColor( gColorTable[ backgroundColor ] );
                WinDrawLine( xDiff - 1, y, xDiff - 1, y + FntCharHeight() - 1 );
                WinSetForeColor( color );
                }
            else
                {
                /*------------------------------------------
                Inside:
                1. black on lt gray if no ephemeris
                2. white on black if ephemeris
                ------------------------------------------*/
                drawMode = ( ( satData[ j ].status & gpsSatEphMask ) != satEph ) ? winOverlay : winMask;
                WinSetDrawMode( drawMode );
                WinPaintChars( tempStr, 1, xDiff, tempRect.topLeft.y + tempRect.extent.y - FntCharHeight() - 1 );
                }
            } /* if differential */
        } /* if channel used */
    } /* for i */

/*----------------------------------------------------------
Draw text fields
----------------------------------------------------------*/
tempRect.topLeft.x = drawRect.extent.x / 2;
tempRect.topLeft.y = statusHeight;
tempRect.extent.x  = tempRect.topLeft.x;
tempRect.extent.y  = ( 2 * skyvwRadius ) + FntCharHeight();

/*----------------------------------------------------------
Calculate the extra space between groupings
----------------------------------------------------------*/
yExtra = ( tempRect.extent.y - ( 5 * FntLineHeight() ) - FntCharHeight() ) / 2;

/*----------------------------------------------------------
Draw location
----------------------------------------------------------*/
y = tempRect.topLeft.y;
LatitudeToString( pvtData.position.lat, tempStr );
DrawString( tempStr, tempRect.topLeft.x, y );

y += FntLineHeight();
LongitudeToString( pvtData.position.lon, tempStr );
DrawString( tempStr, tempRect.topLeft.x, y );

/*----------------------------------------------------------
Draw date
----------------------------------------------------------*/

/*----------------------------------------------------------
Get today's date
----------------------------------------------------------*/
DateSecondsToDate( TimGetSeconds(), &date );

/*----------------------------------------------------------
Convert to string and draw
----------------------------------------------------------*/
DateTemplateToAscii( "^0z-^2r-^4l", date.month, date.day, date.year + 1904, dateStr, longDateStrLength );
y += FntLineHeight() + yExtra;
DrawString( dateStr, tempRect.topLeft.x, y );

/*----------------------------------------------------------
Draw time
----------------------------------------------------------*/

/*----------------------------------------------------------
Determine number of UTC seconds since the beginning of the
current day.
----------------------------------------------------------*/
utcSeconds
    = pvtData.time.seconds
    + ( pvtData.time.fracSeconds * secondsPerFrac )
    + ( ( ( Int32 ) PrefGetPreference( prefTimeZone )
        + ( Int32 ) PrefGetPreference( prefDaylightSavingAdjustment )
        )
      * secondsPerMinute
      );

/*----------------------------------------------------------
Round the UTC seconds because fractional seconds aren't
needed for this application.
----------------------------------------------------------*/
seconds = round( utcSeconds );

/*----------------------------------------------------------
Get hours, minutes, and seconds, making sure that seconds is
not negative.
----------------------------------------------------------*/
time.hours    = (max( 0, seconds )) / secondsPerHour;
seconds      -= time.hours * secondsPerHour;
time.minutes  = (max( 0, seconds )) / secondsPerMinute;
seconds      -= time.minutes * secondsPerMinute;
seconds       = max( 0, seconds );

/*----------------------------------------------------------
Convert time to string
----------------------------------------------------------*/
TimeToAscii( time.hours, time.minutes, tfColon, timeStr );
StrCat( timeStr, ":" );
AppendIntToStr( seconds, 2, timeStr );
StrCat( timeStr, " " );
SysCopyStringResource( tempStr, ( time.hours < 12 ) ? amString : pmString );
StrCat( timeStr, tempStr );

y += FntLineHeight();
DrawString( timeStr, tempRect.topLeft.x, y );

/*----------------------------------------------------------
Draw speed and elevation
----------------------------------------------------------*/
y += FntLineHeight() + yExtra;
VelocityToString( pvtData.velocity.speed, tempStr );
DrawString( tempStr, tempRect.topLeft.x, y );

y += FntLineHeight();
AltitudeToString( pvtData.position.altMSL, tempStr );
DrawString( tempStr, tempRect.topLeft.x, y );

/*----------------------------------------------------------
Make it visible
----------------------------------------------------------*/
WinSetDrawWindow( oldWindow );
WinGetWindowFrameRect( gOffScreenWindow, &tempRect );
WinCopyRectangle( gOffScreenWindow, oldWindow, &tempRect, 0, formTitleHeight, winPaint );

/*----------------------------------------------------------
Free the Satellite Array memory
----------------------------------------------------------*/
MemPtrFree( satData );
satData = NULL;

} /* DrawSatStatus() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       DrawString - Draw a String
*
*   DESCRIPTION:
*       Draws the string at the given coordinate.
*
*********************************************************************/
static void DrawString( Char *str, UInt16 x, UInt16 y )
{
WinDrawChars( str, StrLen( str ), x, y );

} /* DrawString() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       EventLoop - Event Loop
*
*   DESCRIPTION:
*       The event loop for the application.
*
*********************************************************************/
static void EventLoop( void )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
EventType   event;      /* current event                    */
Err         error;      /* error value                      */
Int32       timeout;    /* timeout between event-grabbing   */
eventsEnum  lastType;   /* type of the last event           */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
timeout  = eventWaitSeconds * SysTicksPerSecond();
lastType = nilEvent;

/*----------------------------------------------------------
Process
----------------------------------------------------------*/
do
    {
    EvtGetEvent( &event, timeout );

    if ( ! SysHandleEvent( &event ) )
        {
        if ( ! MenuHandleEvent( 0, &event, &error ) )
            {
            if ( ! AppHandleEvent( &event ) )
                {
                FrmDispatchEvent( &event );
                }
            }
        }

    lastType = event.eType;
    }
while ( event.eType != appStopEvent );

} /* EventLoop() */


/******************************************************************************
*
*   PROCEDURE NAME:
*       GpsLibEventCallback - Display Change Event Callback
*
*   DESCRIPTION:
*       Gets called when a GPS Library Event Notification occurs.
*
******************************************************************************/
static Err GpsLibEventCallback( SysNotifyParamType *notifyParamsP )
{
/*----------------------------------------------------------
Draw the appropriate form on notification of a change
----------------------------------------------------------*/
switch( FrmGetActiveFormID() )
{
    case SatDataForm:
        DrawSatData();
        break;

    case SatStatusForm:
        DrawSatStatus();
        break;

    case AboutForm:
    default:
        /*--------------------------------------------------
        Do nothing if status or data form not active.
        --------------------------------------------------*/
        break;
}

/*----------------------------------------------------------
Per the documentation return 0.
----------------------------------------------------------*/
return 0;

} /* GpsLibEventCallback() */

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
*       SatDataFormHandleEvent - Satellite Data Form Handle Event
*
*   DESCRIPTION:
*       Handles events for the Satellite Data Form.
*
*********************************************************************/
static Boolean SatDataFormHandleEvent( EventPtr event )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean handled;        /* event handled?       */
ListPtr popupListPtr;   /* display mode popup   */
FormPtr frm;            /* pointer to form      */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle events
----------------------------------------------------------*/
switch ( event->eType )
    {
    /*------------------------------------------------------
    Open form
    ------------------------------------------------------*/
    case frmOpenEvent:
        frm = FrmGetActiveForm();
        popupListPtr = FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, SatDataModeList ) );
        if ( popupListPtr )
            {
            LstSetSelection( popupListPtr, satDataListItem );
            }
        FrmDrawForm( frm );

        /*--------------------------------------------------
        Draw
        --------------------------------------------------*/
        DrawSatData();
        handled = true;
        break;

    /*------------------------------------------------------
    User selected an item in a popup list
    ------------------------------------------------------*/
    case popSelectEvent:
        if ( ( event->data.popSelect.listID == SatDataModeList ) && ( event->data.popSelect.selection == satStatusListItem ) )
            {
            FrmGotoForm( SatStatusForm );
            }
        handled = true;
        break;

    /*------------------------------------------------------
    Draw processing
    ------------------------------------------------------*/
    case nilEvent:
        handled = true;
        break;
    }

return handled;

} /* SatDataFormHandleEvent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       SatStatusFormHandleEvent - Satellite Status Form Handle Event
*
*   DESCRIPTION:
*       Handles events for the Satellite Status Form.
*
*********************************************************************/
static Boolean SatStatusFormHandleEvent( EventPtr event )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean handled;        /* event handled?       */
ListPtr popupListPtr;   /* display mode popup   */
FormPtr frm;            /* pointer to form      */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle events
----------------------------------------------------------*/
switch ( event->eType )
    {
    /*------------------------------------------------------
    Open form
    ------------------------------------------------------*/
    case frmOpenEvent:
        frm = FrmGetActiveForm();
        popupListPtr = FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, SatStatusModeList ) );
        if ( popupListPtr )
            {
            LstSetSelection( popupListPtr, satStatusListItem );
            }
        FrmDrawForm( frm );

        /*--------------------------------------------------
        Draw
        --------------------------------------------------*/
        DrawSatStatus();
        handled = true;
        break;

    /*------------------------------------------------------
    User selected an item in a popup list
    ------------------------------------------------------*/
    case popSelectEvent:
        if ( ( event->data.popSelect.listID == SatStatusModeList ) && ( event->data.popSelect.selection == satDataListItem ) )
            {
            FrmGotoForm( SatDataForm );
            }
        handled = true;
        break;

    /*------------------------------------------------------
    Menu event
    ------------------------------------------------------*/
    case menuEvent:
        handled = SatStatusFormHandleMenuEvent( event->data.menu.itemID );
        break;

    /*------------------------------------------------------
    Default processing
    ------------------------------------------------------*/
    case nilEvent:
        /*--------------------------------------------------
        Draw
        --------------------------------------------------*/
        DrawSatStatus();
        handled = true;
        break;
    }

return handled;

} /* SatStatusFormHandleEvent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       SatStatusFormHandleMenuEvent - Satellite Status Form Handle
*           Menu Event
*
*   DESCRIPTION:
*       Handles menu events for the Satellite Status Form.
*
*********************************************************************/
static Boolean SatStatusFormHandleMenuEvent( UInt16 menuID )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Boolean handled;    /* event handled?       */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
handled = false;

/*----------------------------------------------------------
Handle events
----------------------------------------------------------*/
switch ( menuID )
    {
    /*------------------------------------------------------
    About
    ------------------------------------------------------*/
    case OptionsAbout:
        FrmPopupForm( AboutForm );
        handled = true;
        break;
    }

return handled;

} /* SatStatusFormHandleMenuEvent() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       StartApp - Start Application
*
*   DESCRIPTION:
*       Performs start-up tasks for the application
*
*********************************************************************/
static Err StartApp( void )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
Err     error;      /* error value                              */
UInt16  appVersion; /* Application version                      */
UInt16  libVersion; /* GPSLib API version                       */
UInt32  depth;      /* color depth                              */
UInt16  cardNo;     /* Card number used by SysNotifyUnregister  */
LocalID dbID;       /* Database ID used by SysNotifyUnregister  */

/*----------------------------------------------------------
Look for and open the MathLib shared library, so we can
perform trig functions later. MathLib is a freely-usable
library (http://www.probe.net/~rhuebner/mathlib.html).
----------------------------------------------------------*/
error = SysLibFind( MathLibName, &MathLibRef );
if ( error != 0 )
    {
    error = SysLibLoad( LibType, MathLibCreator, &MathLibRef );
    }
ErrFatalDisplayIf( error, "Can't find MathLib" );
error = MathLibOpen( MathLibRef, MathLibVersion );
ErrFatalDisplayIf( error, "Can't open MathLib" );

/*----------------------------------------------------------
Look for and open the GARMIN GPS library
----------------------------------------------------------*/
error = SysLibFind( gpsLibName, &gGpsLibRef );
if ( error != 0 )
    {
    error = SysLibLoad( gpsLibType, gpsLibCreator, &gGpsLibRef );
    }
ErrFatalDisplayIf( error, "Can't load GPSLib" );
error = GPSOpen(gGpsLibRef);
ErrFatalDisplayIf( error, "Can't open GPSLib" );

/*----------------------------------------------------------
Register to recieve notifications from the GPS library
----------------------------------------------------------*/
error = SysCurAppDatabase( &cardNo, &dbID );
ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

error = SysNotifyRegister( cardNo, dbID, sysNotifyGPSDataEvent, GpsLibEventCallback, sysNotifyNormalPriority, NULL );
ErrFatalDisplayIf( ( error != errNone ), "can't register" );

/*----------------------------------------------------------
Compare the API version with which this app was built with
the API version of the library.
----------------------------------------------------------*/
appVersion = gpsAPIVersion;
libVersion = GPSGetLibAPIVersion(gGpsLibRef);
if( appVersion > libVersion )
    {
    UInt16  useCount;
    char    errorMsg[] =
        "This application needs a newer version of GPSLib than you currently have. Please get the newest version of GPSLib.";

    SysFatalAlert( errorMsg );

    /*----------------------------------------------------------
    Close MathLib; unload it if we're the last app using it.
    ----------------------------------------------------------*/
    error = MathLibClose( MathLibRef, &useCount );
    ErrFatalDisplayIf( error, "Can't close MathLib" );
    if ( useCount == 0 )
        {
        SysLibRemove( MathLibRef );
        }

    /*----------------------------------------------------------
    Unregister notifications from the GPS library
    ----------------------------------------------------------*/
    error = SysCurAppDatabase( &cardNo, &dbID );
    ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

    error = SysNotifyUnregister( cardNo, dbID, sysNotifyGPSDataEvent, sysNotifyNormalPriority );
    ErrFatalDisplayIf( ( error != errNone ), "can't register" );

    /*----------------------------------------------------------
    Close GPS Library; unload it if we're the last app using it.
    ----------------------------------------------------------*/
    error = GPSClose( gGpsLibRef );
    if ( error == gpsErrNone )
        {
        SysLibRemove( gGpsLibRef );
        }

    return 1;
    }

/*----------------------------------------------------------
Capture color settings, then set new
----------------------------------------------------------*/
WinScreenMode( winScreenModeGet, NULL, NULL, &gOldDepth, NULL );
depth = ( gOldDepth > monoColorDepth ) ? colorColorDepth : monoColorDepth;
WinScreenMode( winScreenModeSet, NULL, NULL, &depth, NULL );

/*----------------------------------------------------------
Initialize colors
----------------------------------------------------------*/
if ( gOldDepth < monoColorDepth )
    {
    gColorTable[ backgroundColor ] = grayScaleWhite;
    gColorTable[ scaleColor      ] = grayScaleDarkGray;
    gColorTable[ noEphColor      ] = grayScaleLightGray;
    gColorTable[ ephColor        ] = grayScaleBlack;
    }
else
    {
    gColorTable[ backgroundColor ] = UIColorGetTableEntryIndex( UIFormFill );
    gColorTable[ scaleColor      ] = UIColorGetTableEntryIndex( UIObjectFrame );
    gColorTable[ noEphColor      ] = 19; /* kind of a light blue */
    gColorTable[ ephColor        ] = UIColorGetTableEntryIndex( UIObjectSelectedFill );
    }

/*----------------------------------------------------------
Create offscreen buffer
----------------------------------------------------------*/
gOffScreenWindow = WinCreateOffscreenWindow( 160, 160 - formTitleHeight, nativeFormat, &error );
ErrFatalDisplayIf( error, "Can't create offscreen buffer" );

/*----------------------------------------------------------
Goto first form
----------------------------------------------------------*/
FrmGotoForm( SatStatusForm );

return 0;

} /* StartApp() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       StopApp - Stop Application
*
*   DESCRIPTION:
*       Cleans stuff up when closing the application
*
*********************************************************************/
static void StopApp( void )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
UInt16  useCount;   /* number of times MathLib has been opened  */
Err     error;      /* error value                              */
UInt16  cardNo;     /* Card number used by SysNotifyUnregister  */
LocalID dbID;       /* Database ID used by SysNotifyUnregister  */

/*----------------------------------------------------------
Restore color settings.
----------------------------------------------------------*/
WinScreenMode( winScreenModeSet, NULL, NULL, &gOldDepth, NULL );

/*----------------------------------------------------------
Close MathLib; unload it if we're the last app using it.
----------------------------------------------------------*/
error = MathLibClose( MathLibRef, &useCount );
ErrFatalDisplayIf( error, "Can't close MathLib" );
if ( useCount == 0 )
    {
    SysLibRemove( MathLibRef );
    }

/*----------------------------------------------------------
Unregister notifications from the GPS library
----------------------------------------------------------*/
error = SysCurAppDatabase( &cardNo, &dbID );
ErrFatalDisplayIf( ( error != errNone ), "can't get app db info" );

error = SysNotifyUnregister( cardNo, dbID, sysNotifyGPSDataEvent, sysNotifyNormalPriority );
ErrFatalDisplayIf( ( error != errNone ), "can't register" );

/*----------------------------------------------------------
Close GPS Library; unload it if we're the last app using it.
----------------------------------------------------------*/
error = GPSClose( gGpsLibRef );
if ( error == gpsErrNone )
    {
    SysLibRemove( gGpsLibRef );
    }

/*----------------------------------------------------------
Close all forms
----------------------------------------------------------*/
FrmCloseAllForms();

/*----------------------------------------------------------
Eliminate offscreen buffer
----------------------------------------------------------*/
WinDeleteWindow( gOffScreenWindow, false );

} /* StopApp() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       VelocityToString - Convert Velocity to String
*
*   DESCRIPTION:
*       Converts the velocity in meters/sec into a string in MPH.
*
*********************************************************************/
static void VelocityToString( float velocity, Char *outStr )
{
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
float   vel;    /* local velocity               */
Int32   whole;  /* whole part of location       */
Int32   frac;   /* fractional part of location  */

/*----------------------------------------------------------
Initialize
----------------------------------------------------------*/
*outStr = NULL;

/*----------------------------------------------------------
Convert to mph and round
----------------------------------------------------------*/
vel = ( velocity * mpsToMph ) + 0.05;

/*----------------------------------------------------------
Get whole velocity
----------------------------------------------------------*/
whole = ( Int32 ) vel;

/*----------------------------------------------------------
Get fractional velocity
----------------------------------------------------------*/
frac = ( Int32 ) ( ( vel - ( float ) whole ) * 10.0 );

/*----------------------------------------------------------
Build into string
----------------------------------------------------------*/
AppendIntToStr( whole, 0, outStr );
StrCat( outStr, "." );
AppendIntToStr( frac, 0, outStr );
StrCat( outStr, " mph" );

} /* VelocityToString() */

