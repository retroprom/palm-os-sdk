/*
 * WatchMain.c
 *
 * Main file for Watch app.
 *
 */
 
#include <PalmOS.h>
#include <DebugMgr.h>
#include <StdIOPalm.h>

#include "WristPDA.h"

#include "Watch.h"
#include "WatchRsc.h"

// NOTE: This program uses *NO* global variables!  Do not introduce any!
//       This program supports custom launch codes and is launched in
//       contexts where global variables are not available.

// All code from here to end of file should use no global variables
#pragma warn_a5_access on

// Set FAST_TIME to 1 to make the hands move faster than real-time.
// This makes it easy to see how they are drawn in a variety of positions.

#define FAST_TIME 0

/*********************************************************************
 * Type Definitions
 *********************************************************************/

typedef struct {
	UInt32				Timestamp;			// modDate timestamp of creator app
	WatchPreferenceType	g_prefs;			// App preferences cache
	FormPtr				pFrm;				// Form pointer
	MemHandle			hAlarm;				// Alarm triggered icon bitmap handle
	MemHandle			hFace;				// Face background bitmap handle
	MemHandle			hEmptyBatt;			// Empty battery icon bitmap handle
	MemHandle			hLowBatt;			// Low battery icon bitmap handle
	MemHandle			hVLowBatt;			// Very low battery icon bitmap handle
	MemHandle			hNumber[10];		// Digital face digit bitmap handles
	MemHandle			hSeparator;			// Digital face separator bitmap handle
	DateTimeType		PrevDateTime;		// Previous date & time when time was drawn
	UInt32				PrevDay;			// Previos day when time was drawn
	UInt16				PrevHour;			// Analog: Previous hour drawn
	UInt16				PrevMinute;			// Analog: Previous minute drawn
	UInt16				PrevHour10;			// Digital: Previous hour   10 digit drawn
	UInt16				PrevHour1;			// Digital: Previous hour    1 digit drawn
	UInt16				PrevMinute10;		// Digital: Previous minute 10 digit drawn
	UInt16				PrevMinute1;		// Digital: Previous minute  1 digit drawn
	int					isin[61];			// Sin table for computing pts on a circle
	int					icos[61];			// Cos table for computing pts on a circle
	WatchFaceType		WatchFace;			// The current watch face
	WinHandle			PrevDrawWindow;		// The previous draw window handle
	UInt16				NumWatchFaces;		// The number of WatchFaces elements
	// WARNING: WatchFaces must be the last field in this structure!
	WatchFaceType		WatchFaces[1];		// The array of watch faces
} WatchGlobalsType, * WatchGlobalsPtr;

/*********************************************************************
 * Internal Constants
 *********************************************************************/

// Define the minimum OS version we support

#define ourMinVersion    sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
#define kPalmOS20Version sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

// Define the global memory feature creator
// Define the global memory feature id
// Define the default watch face index

#ifdef WATCH_ABACUS
#define WATCH_NAME				"WatchA"
#define GLOBALS_FEATURE_CREATOR	'Abac'
#define GLOBALS_FEATURE_ID		'WA'
#define DEFAULT_WATCH_FACE_INDEX 0
#endif

#ifdef WATCH_FOSSIL
#define WATCH_NAME				"WatchF"
#define GLOBALS_FEATURE_CREATOR	'Foss'
#define GLOBALS_FEATURE_ID		'WF'
#define DEFAULT_WATCH_FACE_INDEX 0
#endif

/*********************************************************************
 * Internal Macros
 *********************************************************************/

// Macros for determining bounding rectangles

#define LeftMost(x1, x2)   ( (x2 < x1) ? x2 : x1 )
#define RightMost(x1, x2)  ( (x2 > x1) ? x2 : x1 )
#define TopMost(y1, y2)    ( (y2 < y1) ? y2 : y1 )
#define BottomMost(y1, y2) ( (y2 > y1) ? y2 : y1 )

/*********************************************************************
 * Macros For Initializing Data Structures
 *********************************************************************/

#define AnalogFace( i, _DayDate, _FaceResId, _BaseWidth, _HourLen, _MinuteLen, \
					_HandOffsetX, _HandOffsetY, _FontIndex, _Inverted ) \
	gP->WatchFaces[i].Analog.Type        = Analog; \
	gP->WatchFaces[i].Analog.DayDate     = _DayDate; \
	gP->WatchFaces[i].Analog.FaceResId   = _FaceResId; \
	gP->WatchFaces[i].Analog.BaseWidth   = _BaseWidth; \
	gP->WatchFaces[i].Analog.HourLen     = _HourLen; \
	gP->WatchFaces[i].Analog.MinuteLen   = _MinuteLen; \
	gP->WatchFaces[i].Analog.HandOffsetX = _HandOffsetX; \
	gP->WatchFaces[i].Analog.HandOffsetY = _HandOffsetY; \
	gP->WatchFaces[i].Analog.FontIndex   = _FontIndex; \
	gP->WatchFaces[i].Analog.Inverted    = _Inverted;

#define DigitalFace( i, _Hour, _DayDate, _FaceResId, _NumberResIdBase, _NumberResIdInc, _SeparatorResId, \
					 _ExtraSpace, _OffsetX,  _OffsetY, _FontIndex, _LogoResId, _LogoStyle, \
					 _LogoYCoord, _Inverted ) \
	gP->WatchFaces[i].Digital.Type				= Digital; \
	gP->WatchFaces[i].Digital.Hour				= _Hour; \
	gP->WatchFaces[i].Digital.DayDate			= _DayDate; \
	gP->WatchFaces[i].Digital.FaceResId         = _FaceResId; \
	gP->WatchFaces[i].Digital.NumberResIdBase	= _NumberResIdBase; \
	gP->WatchFaces[i].Digital.NumberResIdInc	= _NumberResIdInc; \
	gP->WatchFaces[i].Digital.SeparatorResId	= _SeparatorResId; \
	gP->WatchFaces[i].Digital.ExtraSpace		= _ExtraSpace; \
	gP->WatchFaces[i].Digital.OffsetX			= _OffsetX; \
	gP->WatchFaces[i].Digital.OffsetY			= _OffsetY; \
	gP->WatchFaces[i].Digital.FontIndex			= _FontIndex; \
	gP->WatchFaces[i].Digital.LogoResId			= _LogoResId; \
	gP->WatchFaces[i].Digital.LogoStyle			= _LogoStyle; \
	gP->WatchFaces[i].Digital.LogoYCoord		= _LogoYCoord; \
	gP->WatchFaces[i].Digital.Inverted			= _Inverted;

#define InitTable( t, \
				   v00, v01, v02, v03, v04, v05, v06, v07, v08, v09, v10, v11, \
				   v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, \
				   v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35, \
				   v36, v37, v38, v39, v40, v41, v42, v43, v44, v45, v46, v47, \
				   v48, v49, v50, v51, v52, v53, v54, v55, v56, v57, v58, v59, \
				   v60 ) \
	t[ 0]=v00; t[ 1]=v01; t[ 2]=v02; t[ 3]=v03; t[ 4]=v04; t[ 5]=v05; \
	t[ 6]=v06; t[ 7]=v07; t[ 8]=v08; t[ 9]=v09; t[10]=v10; t[11]=v11; \
	t[12]=v12; t[13]=v13; t[14]=v14; t[15]=v15; t[16]=v16; t[17]=v17; \
	t[18]=v18; t[19]=v19; t[20]=v20; t[21]=v21; t[22]=v22; t[23]=v23; \
	t[24]=v24; t[25]=v25; t[26]=v26; t[27]=v27; t[28]=v28; t[29]=v29; \
	t[30]=v30; t[31]=v31; t[32]=v32; t[33]=v33; t[34]=v34; t[35]=v35; \
	t[36]=v36; t[37]=v37; t[38]=v38; t[39]=v39; t[40]=v40; t[41]=v41; \
	t[42]=v42; t[43]=v43; t[44]=v44; t[45]=v45; t[46]=v46; t[47]=v47; \
	t[48]=v48; t[49]=v49; t[50]=v50; t[51]=v51; t[52]=v52; t[53]=v53; \
	t[54]=v54; t[55]=v55; t[56]=v56; t[57]=v57; t[58]=v58; t[59]=v59; \
	t[60]=v60;

/*********************************************************************
 * Function prototypes
 *********************************************************************/

MemHandle LoadResource( DmResType type, DmResID resID );

void SwitchFace( WatchGlobalsPtr gP, Int16 Direction );

/*********************************************************************
 * Internal Functions
 *********************************************************************/

MemPtr InitGlobals( UInt16 cmd )
{

	LocalID dbID;
	UInt16  cardNo, i, WatchFaceSize, WatchFacesSize, NumWatchFaces;
	UInt32  modDate;

	WatchGlobalsPtr gP;

	// Get the modDate timestamp of the current app database

	if ( SysCurAppDatabase( & cardNo, & dbID ) == 0 ) {
		DmDatabaseInfo( cardNo, dbID,
						NULL, NULL, NULL, NULL,
						& modDate,
						NULL, NULL, NULL, NULL, NULL, NULL );
	} else {
		modDate = 0xFFFFFFFF;
	}

	// Check if we have already allocated global memory

	gP = NULL;

	FtrGet( GLOBALS_FEATURE_CREATOR, GLOBALS_FEATURE_ID, (UInt32 *) & gP );
	
	if ( gP != NULL ) {
		if ( modDate != gP->Timestamp ) {
			// The globals do not match our timestamp
			// so deallocate and recreate them
			MemPtrFree( gP );
		} else {
			// The globals match our timestamp so we will use them
			return gP;
		}
	}

	// The number of watch faces

	#ifdef WATCH_ABACUS
	NumWatchFaces = 22;
	#endif

	#ifdef WATCH_FOSSIL
	NumWatchFaces = 24;
	#endif

	// Alocate global variable memory

	gP = MemPtrNew( sizeof( WatchGlobalsType ) + NumWatchFaces * sizeof ( WatchFaceType ) );

	// Set the owner to the OS so this becomes system global memory

	MemPtrSetOwner( gP, 0 );

	// Zero the allocated block of memory

	MemSet( gP, 0, sizeof( WatchGlobalsType ) + NumWatchFaces * sizeof ( WatchFaceType ) );

	// Store the memory pointer in a feature for later reference
	
	FtrSet( GLOBALS_FEATURE_CREATOR, GLOBALS_FEATURE_ID, (UInt32) gP );

	// Record the modDate timestamp of the app that created these globals
	
	gP->Timestamp = modDate;

	// Current watch face resource handles

	gP->hFace = NULL;
	gP->hSeparator = NULL;
	for ( i = 0; i < 10; i++ )
		gP->hNumber[i] = NULL;

	// Cache for application preferences during program execution

	MemSet(  & gP->g_prefs, 0, sizeof( WatchPreferenceType ) );

	// The sin and cos tables
	
	InitTable(  gP->isin,
	               0,   53,  106,  158,  208,  256,  300,  342,  380,  414,
	             443,  467,  486,  500,  509,  512,  509,  500,  486,  467,
                 443,  414,  380,  342,  300,  256,  208,  158,  106,   53,
                   0,  -53, -106, -158, -208, -255, -300, -342, -380, -414,
                -443, -467, -486, -500, -509, -512, -509, -500, -486, -467,
                -443, -414, -380, -342, -300, -256, -208, -158, -106,  -53,
                   0 );

	InitTable(  gP->icos,
                -512, -509, -500, -486, -467, -443, -414, -380, -342, -300,
                -256, -208, -158, -106,  -53,    0,   53,  106,  158,  208,
                 255,  300,  342,  380,  414,  443,  467,  486,  500,  509,
                 512,  509,  500,  486,  467,  443,  414,  380,  342,  300,
                 256,  208,  158,  106,   53,    0,  -53, -106, -158, -208,
                -256, -300, -342, -380, -414, -443, -467, -486, -500, -509,
                -512 );

	// Initialize the watch faces

	gP->NumWatchFaces = NumWatchFaces;

	#ifdef WATCH_ABACUS

	DigitalFace(  0, Hour12, DayDate6,   2911, 2900,   1,  2910, 3,  0,  0, 1,     0, LogoNo,  0, InvertedNo   ); // DM SimpleDigital
	DigitalFace(  1, Hour12, DayDate6,   2911, 2900,   1,  2910, 3,  0,  0, 1,     0, LogoNo,  0, InvertedYes  ); // DM SimpleDigital

	DigitalFace(  2, Hour12, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // Big Sans
	DigitalFace(  3, Hour12, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // Big Sans

	DigitalFace(  4, Hour24, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // Big Sans 24
	DigitalFace(  5, Hour24, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // Big Sans 24

	DigitalFace(  6, Hour12, DayDate5,      0, 2700,   1,  2710, 0,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // Script2
	DigitalFace(  7, Hour12, DayDate5,      0, 2700,   1,  2710, 0,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // Script2

	DigitalFace(  8, Hour12, DayDate2,      0, 2800,   1,  2810, 0,  0,  0, 2, 12000,  Logo2,  0, InvertedNo   ); // Starck
	DigitalFace(  9, Hour12, DayDate2,      0, 2800,   1,  2810, 0,  0,  0, 2, 12000,  Logo2,  0, InvertedYes  ); // Starck

	DigitalFace( 10, Hour12, DayDate10,  3211, 3200,   1,  3210, 10,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // NM Optical
	DigitalFace( 11, Hour12, DayDate10,  3211, 3200,   1,  3210, 10,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // NM Optical

	DigitalFace( 12, Hour12, DayDateNo,  2311, 2300,   1,  2310, 0,  0,  0, 1,     0, LogoNo,  0, InvertedNo   ); // MM Peek
	DigitalFace( 13, Hour12, DayDateNo,  2311, 2300,   1,  2310, 0,  0,  0, 1,     0, LogoNo,  0, InvertedYes  ); // MM Peek

	DigitalFace( 14, Hour12, DayDateNo,  2511, 2500,   1,  2510, 0, 19, 16, 1,     0, LogoNo,  0, InvertedNo   ); // DM DarkCircle
	DigitalFace( 15, Hour12, DayDateNo,  2511, 2500,   1,  2510, 0, 19, 16, 1,     0, LogoNo,  0, InvertedYes  ); // DM DarkCircle

	AnalogFace(  16, DayDate1,    6050, 4, 43, 62, 0,   0,  2, InvertedNo  ); // DC Analog1
	AnalogFace(  17, DayDate1,    6050, 4, 43, 62, 0,   0,  2, InvertedYes ); // DC Analog1

	AnalogFace(  18, DayDate3,    6200, 4, 31, 44, 0, -12,  2, InvertedYes ); // LV Analog2
	AnalogFace(  19, DayDate3,    6200, 4, 31, 44, 0, -12,  2, InvertedNo  ); // LV Analog2

	AnalogFace(  20, DayDateNo,   6100, 4, 43, 62, 0,   0,  1, InvertedNo  ); // MM Analog1
	AnalogFace(  21, DayDateNo,   6100, 4, 43, 62, 0,   0,  1, InvertedYes ); // MM Analog1

	#endif

	#ifdef WATCH_FOSSIL
	
	DigitalFace(  0, Hour12, DayDate1,      0, 2000,   1,  2010, 0,  0,  0, 2, 11000,  Logo1,  0, InvertedNo   ); // Script1
	DigitalFace(  1, Hour12, DayDate1,      0, 2000,   1,  2010, 0,  0,  0, 2, 11000,  Logo1,  0, InvertedYes  ); // Script1

	DigitalFace(  2, Hour12, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // Big Sans
	DigitalFace(  3, Hour12, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // Big Sans

	DigitalFace(  4, Hour24, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // Big Sans 24
	DigitalFace(  5, Hour24, DayDate5,      0, 3100,   1,  3110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // Big Sans 24

	DigitalFace(  6, Hour12, DayDate7,      0, 2200,   1,  2210, 3,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // DM BlkDigital
	DigitalFace(  7, Hour12, DayDate7,      0, 2200,   1,  2210, 3,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // DM BlkDigital

	DigitalFace(  8, Hour12, DayDate3,      0, 2600,   1,  2610, 0,  0,  0, 2, 11003,  Logo3,  0, InvertedNo   ); // Squashed
	DigitalFace(  9, Hour12, DayDate3,      0, 2600,   1,  2610, 0,  0,  0, 2, 11003,  Logo3,  0, InvertedYes  ); // Squashed

	DigitalFace( 10, Hour12, DayDateNo,     0, 2100,   1,  2110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedNo   ); // MM Script
	DigitalFace( 11, Hour12, DayDateNo,     0, 2100,   1,  2110, 0,  0,  0, 2,     0, LogoNo,  0, InvertedYes  ); // MM Script

	DigitalFace( 12, Hour12, DayDate2,      0, 2800,   1,  2810, 0,  0,  0, 2, 11003,  Logo2,  0, InvertedNo   ); // Starck
	DigitalFace( 13, Hour12, DayDate2,      0, 2800,   1,  2810, 0,  0,  0, 2, 11003,  Logo2,  0, InvertedYes  ); // Starck

	DigitalFace( 14, Hour12, DayDate8,   2411, 2400,   1,  2410, 0, -3, 29, 1,     0, LogoNo,  0, InvertedNo   ); // DM Barcode
	DigitalFace( 15, Hour12, DayDate8,   2411, 2400,   1,  2410, 0, -3, 29, 1,     0, LogoNo,  0, InvertedYes  ); // DM Barcode

	DigitalFace( 16, Hour12, DayDate9,   3011, 3000,   1,  3010, 0, -6, 43, 2,     0, LogoNo,  0, InvertedNo   ); // LV Digital1
	DigitalFace( 17, Hour12, DayDate9,   3011, 3000,   1,  3010, 0, -6, 43, 2,     0, LogoNo,  0, InvertedYes  ); // LV Digital1

	AnalogFace(  18, DayDate1,    6000, 4, 43, 62, 0,   0,  2, InvertedNo  ); // DC Analog1
	AnalogFace(  19, DayDate1,    6000, 4, 43, 62, 0,   0,  2, InvertedYes ); // DC Analog1

	AnalogFace(  20, DayDate3,    6300, 4, 31, 44, 0, -12,  2, InvertedYes ); // LV Analog1
	AnalogFace(  21, DayDate3,    6300, 4, 31, 44, 0, -12,  2, InvertedNo  ); // LV Analog1

	AnalogFace(  22, DayDateNo,   6100, 4, 43, 62, 0,   0,  1, InvertedNo  ); // MM Analog1
	AnalogFace(  23, DayDateNo,   6100, 4, 43, 62, 0,   0,  1, InvertedYes ); // MM Analog1

	#endif

	// The current watch face

	MemMove( & ( gP->WatchFace ), ( & gP->WatchFaces[DEFAULT_WATCH_FACE_INDEX] ), sizeof( WatchFaceType ) );

	return gP;

}

void BmpGetDims( BitmapPtr pBmp, Int16 * Width, Int16 * Height )
{

	typedef struct PrvBitmapType
	{
		Int16  	width;
		Int16  	height;
		UInt16  rowBytes;
		UInt16	flags;
		UInt8	pixelSize;
		UInt8	version;
		UInt16	nextDepthOffset;
		UInt8	transparentIndex;
		UInt8	compressionType;
		UInt16	reserved;
	} PrvBitmapType;

	* Width  = ( ( PrvBitmapType * ) pBmp )->width;
	* Height = ( ( PrvBitmapType * ) pBmp )->height;

}

// Draw a radial line segment in a circle

void RadialLine( WatchGlobalsPtr gP,
				 Int32 Sixtieth,
				 Int32 r1, Int32 r2,
				 Int32 CenterX, Int32 CenterY )
{

  Int32 x1, x2, y1, y2;

  x1=( ( gP->isin[Sixtieth] * r1 ) / 512 ) + CenterX;
  y1=( ( gP->icos[Sixtieth] * r1 ) / 512 ) + CenterY;

  x2=( ( gP->isin[Sixtieth] * r2 ) / 512 ) + CenterX;
  y2=( ( gP->icos[Sixtieth] * r2 ) / 512 ) + CenterY;

  WinDrawLine( x1, y1, x2, y2 );
}

// Draw a watch hand

RectangleType DrawHand( WatchGlobalsPtr gP,
						Int32 Sixtieth,
		                Int32 Length, Int32 BaseWidth,
		                Int32 CenterX, Int32 CenterY,
						Boolean Draw )
{

	Int32 x1, x2, x3, y1, y2, y3;
	PointType UpperLeft, LowerRight;
	RectangleType BoundingRect;

	// Compute the hand's coordinates

	x1 = ( ( gP->isin[Sixtieth] * Length ) / 512) + CenterX;
	y1 = ( ( gP->icos[Sixtieth] * Length ) / 512) + CenterY;

	x2 = ( ( gP->isin[(Sixtieth+8) % 60] * BaseWidth ) / 512 ) + CenterX;
	y2 = ( ( gP->icos[(Sixtieth+8) % 60] * BaseWidth ) / 512 ) + CenterY;

	x3 = ( ( gP->isin[(Sixtieth+52) % 60] * BaseWidth ) / 512 ) + CenterX;
	y3 = ( ( gP->icos[(Sixtieth+52) % 60] * BaseWidth ) / 512 ) + CenterY;

	// Compute the hand's bounding rectangle

	UpperLeft.x = LeftMost( x1, x2 );
	UpperLeft.x = LeftMost( UpperLeft.x, x3 );

	UpperLeft.y = TopMost( y1, y2 );
	UpperLeft.y = TopMost( UpperLeft.y, y3 );

	LowerRight.x = RightMost( x1, x2 );
	LowerRight.x = RightMost( LowerRight.x, x3 );

	LowerRight.y = BottomMost( y1, y2 );
	LowerRight.y = BottomMost( LowerRight.y, y3 );

	BoundingRect.topLeft.x = UpperLeft.x;
	BoundingRect.topLeft.y = UpperLeft.y;

	BoundingRect.extent.x = LowerRight.x - UpperLeft.x;
	BoundingRect.extent.y = LowerRight.y - UpperLeft.y;

	// Draw the hand if directed to do so

	if ( Draw == true ) {
		WinDrawLine( x1,y1,x2,y2 );
		WinDrawLine( x1,y1,x3,y3 );
	}

	return BoundingRect;

}

// Determine the bounding rectangle of two rectangles

RectangleType RectBoundingRect( RectangleType Rect1, RectangleType Rect2 )
{

	Coord         x1, x2, x3, x4, y1, y2, y3, y4;
	PointType     UpperLeft, LowerRight;
	RectangleType BoundingRect;

	x1 = Rect1.topLeft.x;
	x2 = x1 + Rect1.extent.x;

	y1 = Rect1.topLeft.y;
	y2 = y1 + Rect1.extent.y;

	x3 = Rect2.topLeft.x;
	x4 = x3 + Rect2.extent.x;

	y3 = Rect2.topLeft.y;
	y4 = y3 + Rect2.extent.y;

	UpperLeft.x = LeftMost( x1, x2 );
	UpperLeft.x = LeftMost( UpperLeft.x, x3 );
	UpperLeft.x = LeftMost( UpperLeft.x, x4 );

	UpperLeft.y = TopMost( y1, y2 );
	UpperLeft.y = TopMost( UpperLeft.y, y3 );
	UpperLeft.y = TopMost( UpperLeft.y, y4 );

	LowerRight.x = RightMost( x1, x2 );
	LowerRight.x = RightMost( LowerRight.x, x3 );
	LowerRight.x = RightMost( LowerRight.x, x4 );

	LowerRight.y = BottomMost( y1, y2 );
	LowerRight.y = BottomMost( LowerRight.y, y3 );
	LowerRight.y = BottomMost( LowerRight.y, y4 );

	BoundingRect.topLeft.x = UpperLeft.x;
	BoundingRect.topLeft.y = UpperLeft.y;

	BoundingRect.extent.x = LowerRight.x - UpperLeft.x + 1;
	BoundingRect.extent.y = LowerRight.y - UpperLeft.y + 1;

	return BoundingRect;

}


// Load a resource from the Watch app's database

MemHandle LoadResource( DmResType type, DmResID resID )
{

#if 1

	DmOpenRef	refWatch;
	LocalID		dbID;
	MemHandle	hRes;
	UInt16		iRes;
				
	dbID = DmFindDatabase( 0 /* cardNo */, WATCH_NAME );
	
	if ( dbID == 0 ) {
		// Failed to find the database.
		return NULL;
	}

	refWatch = DmOpenDatabase( 0 /* cardNo */, dbID, dmModeReadOnly );
										   
	if ( refWatch == 0 ) {
		// Failed to open the database.
		return NULL;
	}
				
	iRes = DmFindResource( refWatch, type, resID, NULL );
				
	if ( iRes == (UInt16) -1 ) {
		// Failed to get index of resource.
		DmCloseDatabase( refWatch );
		return NULL;
	}

	hRes = DmGetResourceIndex( refWatch, iRes );
				
	if ( hRes == NULL ) {
		// Failed to get handle to resource.
		DmCloseDatabase( refWatch );
		return NULL;
	}

	DmCloseDatabase( refWatch );

	return hRes;

#else

	return DmGet1Resource( type, resID );

#endif	

}


// Draw time on analog watch face

void DrawAnalogTime( WatchGlobalsPtr gP, BOOL FullDraw )
{

	BitmapPtr pFace;

	char  Str[32];

	DateTimeType Now;
	DateType	 Date;

	IndexedColorType curForeColor;
	IndexedColorType curBackColor;
	IndexedColorType curTextColor;

	Int16 FaceWidth, FaceHeight;

	Int32 HandCenterX, HandCenterY;

	RectangleType HourRect, MinuteRect;

	UInt16 CenterX, CenterY;
	UInt16 FaceX, FaceY;
	UInt16 SaveError;

	UInt32 Day;

	// Get the current time

	TimSecondsToDateTime ( TimGetSeconds(), & Now );

	Date.month = Now.month;
	Date.day   = Now.day;
	Date.year  = Now.year;

	#if FAST_TIME
	Now.minute = Now.second;
	Now.hour = Now.second % 24;
	#endif

	Day = DateToDays( Date );

	// Determine coordinates where bitmaps should be drawn

	CenterX = 80;
	CenterY = 80;
	
	// Validate the current face before drawing.

	if ( gP->hFace == NULL ) {
		SwitchFace( gP, 0 );
		return;
	}

	pFace = MemHandleLock( gP->hFace );
	BmpGetDims( pFace, & FaceWidth, & FaceHeight );

	FaceX = CenterX - FaceWidth  / 2;
	FaceY = CenterY - FaceHeight / 2;

	// We always do a full draw

	FullDraw = true;

	// If the face is inverted then swap the foreground and background colors

	if ( gP->WatchFace.Analog.Inverted == InvertedYes ) {
		curForeColor = WinSetForeColor( 0 );
		curBackColor = WinSetBackColor( 0 );
		curTextColor = WinSetTextColor( 0 );
		WinSetForeColor( curBackColor );
		WinSetBackColor( curForeColor );
		WinSetTextColor( curBackColor );
	}

	// Erase the background

	if ( FullDraw == true )
		WinEraseWindow();

	// Draw the analog face

	// Draw the face bitmap
    WinDrawBitmap( pFace, FaceX, FaceY );
    
	// Draw the day and date, which may be on the face

	if ( gP->WatchFace.Analog.DayDate == DayDate1 ) {
		if ( ( FullDraw == true ) || ( Day != gP->PrevDay ) ) {
			FntSetFont( FossilLargeFontID( WRISTPDA, gP->WatchFace.Analog.FontIndex ) );
			DateTemplateToAscii( "^3s/^0z/^4s", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
			WinDrawChars( Str, StrLen( Str ), 80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 110 );
		}
	} else if ( gP->WatchFace.Analog.DayDate == DayDate3 ) {
		if ( ( FullDraw == true ) || ( Day != gP->PrevDay ) ) {
			FntSetFont( FossilLargeFontID( WRISTPDA, gP->WatchFace.Analog.FontIndex ) );
			DateTemplateToAscii( "^1r - ^2r ^0z", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
			WinDrawChars( Str, StrLen( Str ),  80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 132 - FntCharHeight() / 2 );
		}
	} else if ( gP->WatchFace.Analog.DayDate == DayDate5 ) {
		if ( ( FullDraw == true ) || ( Day != gP->PrevDay ) ) {
			FntSetFont( FossilLargeFontID( WRISTPDA, gP->WatchFace.Analog.FontIndex ) );
			DateTemplateToAscii( "^2r ^0r", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
			WinDrawChars( Str, StrLen( Str ), 0, 0 );
			DateTemplateToAscii( "^4r", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
			WinDrawChars( Str, StrLen( Str ), 160 - FntCharsWidth( Str, StrLen( Str ) ), 0 );
			DateTemplateToAscii( "^1r", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
			WinDrawChars( Str, StrLen( Str ), 0, 160 - FntCharHeight() );
			StrCopy( Str, ( Now.hour < 12 ) ? "AM" : "PM" );
			WinDrawChars( Str, StrLen( Str ),  160 - FntCharsWidth( Str, StrLen( Str ) ), 160 - FntCharHeight() );
		}
	}

	// Get the bounding rectangle for the new hands

	HandCenterX = (Int32) CenterX + (Int32) gP->WatchFace.Analog.HandOffsetX;
	HandCenterY = (Int32) CenterY + (Int32) gP->WatchFace.Analog.HandOffsetY;

	HourRect = DrawHand( gP, ( ( Now.hour % 12 ) * 5 ) + ( Now.minute / 12 ),
			             gP->WatchFace.Analog.HourLen,
			             gP->WatchFace.Analog.BaseWidth,
			             HandCenterX,
				         HandCenterY,
				         false );

	MinuteRect = DrawHand( gP, Now.minute,
				           gP->WatchFace.Analog.MinuteLen,
				           gP->WatchFace.Analog.BaseWidth,
			               HandCenterX,
				           HandCenterY,
				           false );

	// Draw the new hands

	DrawHand( gP, ( ( Now.hour % 12 ) * 5 ) + ( Now.minute / 12 ),
	          gP->WatchFace.Analog.HourLen,
	          gP->WatchFace.Analog.BaseWidth,
              HandCenterX,
	          HandCenterY,
	          true );

	DrawHand( gP, Now.minute,
	          gP->WatchFace.Analog.MinuteLen,
	          gP->WatchFace.Analog.BaseWidth,
			  HandCenterX,
			  HandCenterY,
	          true );

	// If the face is inverted then restore the foreground and background colors

	if ( gP->WatchFace.Analog.Inverted == InvertedYes ) {
		WinSetForeColor( curForeColor );
		WinSetBackColor( curBackColor );
		WinSetTextColor( curTextColor );
	}

	// Unlock the bitmap resources

	MemHandleUnlock( gP->hFace );

	// Update the previous time
	
	gP->PrevDateTime = Now;

	gP->PrevDay	     = Day;
	gP->PrevHour     = Now.hour;
	gP->PrevMinute   = Now.minute;

}

// Draw time on digital watch face

void DrawDigitalTime( WatchGlobalsPtr gP, BOOL FullDraw )
{

	BitmapPtr pFace, pSeparator, pHour10, pHour1, pMinute10, pMinute1;

	char  DateStr[32], Str[32];

	DateTimeType Now;
	DateType	 Date;

	IndexedColorType curForeColor;
	IndexedColorType curBackColor;
	IndexedColorType curTextColor;

	Int16 FaceWidth, FaceHeight;
	Int16 Hour10Width, Hour10Height;
	Int16 Hour1Width, Hour1Height;
	Int16 Minute10Width, Minute10Height;
	Int16 Minute1Width, Minute1Height;
	Int16 SeparatorWidth, SeparatorHeight;

	UInt16 CenterX, CenterY;
	UInt16 DateX, DateY;
	UInt16 FaceX, FaceY;
	UInt16 Hour10X, Hour10Y;
	UInt16 Hour1X, Hour1Y;
	UInt16 Minute10X, Minute10Y;
	UInt16 Minute1X, Minute1Y;
	UInt16 SeparatorX, SeparatorY;
	UInt16 TimeCenterX, TimeCenterY;

	UInt16 DateWidth, Hour, Hour10, Hour1, Minute10, Minute1;
	
	UInt32 Day;

	// Get the current time

	TimSecondsToDateTime ( TimGetSeconds(), & Now );

	#if FAST_TIME
	Now.minute = Now.second;
	Now.hour = Now.second % 24;
	#endif

	Hour = Now.hour;

	if ( ( gP->WatchFace.Digital.Hour == Hour12 ) && ( Hour > 12 ) ) {
		Hour -= 12;
	}

	if ( Hour == 0 )
		Hour = 12;

	Hour10   = Hour / 10;
	Hour1    = Hour - ( Hour10 * 10 );

	Minute10 = Now.minute / 10;
	Minute1  = Now.minute - ( Minute10 * 10 );

	Date.month = Now.month;
	Date.day   = Now.day;
	Date.year  = Now.year;

	Day = DateToDays( Date );

	// Do a full draw if the am / pm state has changed

	if ( ( Hour10 * 10 + Hour1 ) < ( gP->PrevHour10 * 10 + gP->PrevHour1 ) )
		FullDraw = true;

	// Don't update the display if not doing full draw and time hasn't changed

	#if 0
	// Disable this optimization for now.  When the Watch app is the current app
	// and you are in Watch mode this optimization causes the time to not get
	// updated right away.  Since the wakeup restored an old bitmap, the time
	// may appear to move backwards.
	if ( ( FullDraw == false ) &&
	     ( Now.hour == gP->PrevDateTime.hour ) &&
	     ( Now.minute == gP->PrevDateTime.minute ) )
		return;
	#endif

	// Determine coordinates where bitmaps should be drawn

	CenterX = 79;
	CenterY = 79;
	
	TimeCenterX = (Int16) CenterX + gP->WatchFace.Digital.OffsetX;
	TimeCenterY = (Int16) CenterY + gP->WatchFace.Digital.OffsetY;
	
	// Validate the separator before drawing.

	if ( gP->hSeparator == NULL ) {
		SwitchFace( gP, 0 );
		return;
	}

	// Get face metrics before drawing.

	if ( gP->hFace != 0 ) {
		pFace = MemHandleLock( gP->hFace );
		BmpGetDims( pFace, & FaceWidth, & FaceHeight );
		FaceX = CenterX - FaceWidth  / 2;
		FaceY = CenterY - FaceHeight / 2;
	}

	pSeparator = MemHandleLock( gP->hSeparator );
	BmpGetDims( pSeparator, & SeparatorWidth, & SeparatorHeight );
	SeparatorWidth += gP->WatchFace.Digital.ExtraSpace;

	pHour1 = MemHandleLock( gP->hNumber[Hour1] );
	BmpGetDims( pHour1, & Hour1Width, & Hour1Height );
	Hour1Width += gP->WatchFace.Digital.ExtraSpace;

	pHour10 = MemHandleLock( gP->hNumber[Hour10] );
	BmpGetDims( pHour10, & Hour10Width, & Hour10Height );
	Hour10Width += gP->WatchFace.Digital.ExtraSpace;

	pMinute10 = MemHandleLock( gP->hNumber[Minute10] );
	BmpGetDims( pMinute10, & Minute10Width, & Minute10Height );
	Minute10Width += gP->WatchFace.Digital.ExtraSpace;

	pMinute1 = MemHandleLock( gP->hNumber[Minute1] );
	BmpGetDims( pMinute1, & Minute1Width, & Minute1Height );
	Minute1Width += gP->WatchFace.Digital.ExtraSpace;

	#define Face2LeftEdge  12
	#define Face2RightEdge 90

	switch ( gP->WatchFace.Digital.DayDate ) {
	
		case DayDate2:

			SeparatorX = Face2LeftEdge + Hour10Width - ( SeparatorHeight / 2 );
			SeparatorY = TimeCenterY - ( SeparatorHeight / 2 );

			Hour10X = Face2LeftEdge;
			Hour10Y = TimeCenterY - ( SeparatorHeight / 2 ) - Hour10Height - gP->WatchFace.Digital.ExtraSpace;

			Hour1X = ( Hour10 > 0 ) ? Hour10X + Hour10Width + gP->WatchFace.Digital.ExtraSpace : 10;
			Hour1Y = Hour10Y;

			Minute10X = Face2LeftEdge;
			Minute10Y = TimeCenterY + ( SeparatorHeight / 2 ) + gP->WatchFace.Digital.ExtraSpace;

			Minute1X = Minute10X + Minute10Width;
			Minute1Y = Minute10Y;
			
			break;

		case DayDate10:

			SeparatorX = Face2LeftEdge + Hour10Width - ( SeparatorHeight / 2 );
			SeparatorY = TimeCenterY - ( SeparatorHeight / 2 );

			Hour10X   =  28 - Hour10Width / 2;
			Hour1X    =  65 - Hour1Width / 2;
			Minute10X = 102 - Minute10Width / 2;
			Minute1X  = 139 - Minute1Width / 2;

			Hour10Y   = 77 - Hour10Height / 2;
			Hour1Y    = 77 - Hour1Height / 2;
			Minute10Y = 77 - Minute10Height / 2;
			Minute1Y  = 77 - Minute1Height / 2;

			break;

		default:

			SeparatorX = TimeCenterX - ( SeparatorWidth  / 2 );
			SeparatorY = TimeCenterY - ( SeparatorHeight / 2 );

			Hour1X = SeparatorX - Hour1Width;
			Hour1Y = TimeCenterY - ( Hour1Height / 2 );

			Hour10X = Hour1X - Hour10Width;
			Hour10Y = TimeCenterY - ( Hour10Height / 2 );

			Minute10X = SeparatorX + SeparatorWidth;
			Minute10Y = TimeCenterY - ( Minute10Height / 2 );

			Minute1X = Minute10X + Minute10Width;
			Minute1Y = TimeCenterY - ( Minute1Height / 2 );
			
			break;
			
	}

	// If the face is inverted then swap the foreground and background colors

	if ( gP->WatchFace.Digital.Inverted == InvertedYes ) {
		curForeColor = WinSetForeColor( 0 );
		curBackColor = WinSetBackColor( 0 );
		curTextColor = WinSetTextColor( 0 );
		WinSetForeColor( curBackColor );
		WinSetBackColor( curForeColor );
		WinSetTextColor( curBackColor );
	}

	// If we have a face bitmap then we must always do a full draw

	if ( gP->hFace != 0 )
		FullDraw = true;

	// Erase the background

	if ( FullDraw == true )
		WinEraseWindow();

	if ( gP->hFace != 0 ) {
		// Draw the face bitmap, if there is one
	    WinDrawBitmap( pFace, FaceX, FaceY );
	}

	// For face 2 we need to draw the rectangle frame

	if ( ( FullDraw == true ) && ( gP->WatchFace.Digital.DayDate == DayDate2 ) ) {
		RectangleType r;
		r.topLeft.x = 2;
		r.topLeft.y = 2;
		r.extent.x  = 156;
		r.extent.y  = 156;
		WinDrawRectangleFrame( rectangleFrame, & r );
		r.topLeft.x = 1;
		r.topLeft.y = 1;
		r.extent.x  = 158;
		r.extent.y  = 158;
		WinDrawRectangleFrame( rectangleFrame, & r );
		WinDrawLine( Face2RightEdge - 7, 2, Face2RightEdge - 7, 157 );
		WinDrawLine( Face2RightEdge - 6, 2, Face2RightEdge - 6, 157 );
	}

	// Draw the logo, if one is present

	if ( ( FullDraw == true ) && ( gP->WatchFace.Digital.LogoStyle != LogoNo ) ) {
		BitmapPtr pLogo;
		Int16     LogoWidth, LogoHeight;
		MemHandle hLogo;
		UInt16    LogoX, LogoY;
		hLogo = LoadResource( 'Tbmp', gP->WatchFace.Digital.LogoResId );
		pLogo = MemHandleLock( hLogo );
		BmpGetDims( pLogo, & LogoWidth, & LogoHeight );
		switch ( gP->WatchFace.Digital.LogoStyle ) {
			case Logo1:
				LogoX = CenterX - ( LogoWidth / 2 );
				LogoY = 10;
			    WinDrawBitmap( pLogo, LogoX, LogoY );
			    break;
			case Logo2:
				// LogoX = Face2RightEdge - 5;
				LogoX = Face2RightEdge + ( ( 160 - Face2RightEdge ) / 2 ) - ( LogoWidth / 2 ) - 4;
				LogoY = 18;
			    WinDrawBitmap( pLogo, LogoX, LogoY );
			    break;
			case Logo3:
				LogoX = CenterX - ( LogoWidth / 2 );
				LogoY = 20;
			    WinDrawBitmap( pLogo, LogoX, LogoY );
			    break;
		}
		MemHandleUnlock( hLogo );
	}

	// Draw the day and date

	if ( ( FullDraw == true ) || ( Day != gP->PrevDay ) ) {
		FntSetFont( FossilLargeFontID( WRISTPDA, gP->WatchFace.Digital.FontIndex ) );
		switch ( gP->WatchFace.Digital.DayDate ) {
			case DayDate1:
				DateTemplateToAscii( "^1l", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				WinDrawChars( Str, StrLen( Str ), 80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 159 - ( 2 * FntCharHeight() ) - 5 );
				DateTemplateToAscii( "^3z/^0z/^4s", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				WinDrawChars( Str, StrLen( Str ), 80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 159 - ( 1 * FntCharHeight() ) - 5 );
				break;
			case DayDate2:
				DateTemplateToAscii( "^1l", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				Str[4] = '\0';
				#define toupper( c ) c = ( c >= 'a' ) ? ( c - 'a' + 'A' ) : c;
				toupper( Str[0] );
				toupper( Str[1] );
				toupper( Str[2] );
				toupper( Str[3] );
				WinDrawChars( Str, StrLen( Str ), Face2RightEdge, 95 );
				DateTemplateToAscii( "^3s/^0z/^4s", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				WinDrawChars( Str, StrLen( Str ), Face2RightEdge - 2, 120);
				Str[0] = '\0';
				StrCat( Str, Now.hour < 12 ? "AM" : "PM" );
				WinDrawChars( Str, StrLen( Str ), Minute1X + Minute1Width - 20, Minute1Y + Minute1Height + 5 );
				break;
			case DayDate3:
				DateTemplateToAscii( "^2r  ^0r,  ^4r", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				WinDrawChars( Str, StrLen( Str ), 80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 159 - ( 2 * FntCharHeight() ) - 5 );
				break;
			case DayDate4:
				DateTemplateToAscii( "^1l", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				WinDrawChars( Str, StrLen( Str ), 80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 5 );
				DateTemplateToAscii( "^3z/^0z/^4s", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				WinDrawChars( Str, StrLen( Str ), 80 - FntCharsWidth( Str, StrLen( Str ) ) / 2, 159 - FntCharHeight() - 5 );
				break;
			case DayDate5:
				if ( gP->WatchFace.Digital.Hour == Hour12 ) {
					DateTemplateToAscii( "^1r ^2r ^0r, ^4r ",
					                     Now.month, Now.day, Now.year,
			    		                 DateStr, sizeof( DateStr ) );
					StrCat( DateStr, Now.hour < 12 ? "AM" : "PM" );
				} else {
					DateTemplateToAscii( "^1r  ^2r ^0r, ^4r",
					                     Now.month, Now.day, Now.year,
			    		                 DateStr, sizeof( DateStr ) );
				}
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = CenterX - DateWidth / 2 + 1;
				DateY = Hour1Y - FntCharHeight() - ( FntCharHeight() / 2 );
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
				break;
			case DayDate6:
				// For SimpleDigital face
				FntSetFont( gP->WatchFace.Digital.FontIndex );
				StrCopy( DateStr, "ABACUS" );
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = 25 - DateWidth / 2;
				DateY = 42 - ( FntCharHeight() / 2 );
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
				StrCopy( DateStr, Now.hour < 12 ? "AM" : "PM" );
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = 60 - DateWidth / 2;
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
				DateTemplateToAscii( "^3r-^0z-^4s",
				                     Now.month, Now.day, Now.year,
		    		                 DateStr, sizeof( DateStr ) );
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = 98 - DateWidth / 2;
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
				break;
			case DayDate7:
				// For BlkDigital face
				StrCopy( DateStr, Now.hour < 12 ? "am" : "pm" );
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = Minute1X + Minute1Width / 2 - DateWidth / 2;
				DateY = Minute1Y + Minute1Height + 4;
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
				DateTemplateToAscii( "^3r-^0z-^4s",
				                     Now.month, Now.day, Now.year,
		    		                 DateStr, sizeof( DateStr ) );
				WinDrawChars( DateStr, StrLen( DateStr ), 12, 160 - 8 - FntCharHeight() );
				break;
			case DayDate8:
				// For Barcode face
				FntSetFont( FossilLargeFontID( WRISTPDA, gP->WatchFace.Digital.FontIndex ) );
				StrCopy( DateStr, Now.hour < 12 ? "AM    " : "PM    " );
				DateTemplateToAscii( "^1l  ", Now.month, Now.day, Now.year, Str, sizeof( Str ) );
				Str[4] = '\0';
				#define toupper( c ) c = ( c >= 'a' ) ? ( c - 'a' + 'A' ) : c;
				toupper( Str[0] );
				toupper( Str[1] );
				toupper( Str[2] );
				toupper( Str[3] );
				StrCat( DateStr, Str );
				DateTemplateToAscii( "  ^3r-^0z-^4s",
				                     Now.month, Now.day, Now.year,
		    		                 Str, sizeof( Str ) );
				StrCat( DateStr, Str );
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = CenterX - DateWidth / 2 + 1;
				DateY = Hour10Y + Hour10Height + 2;
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
				break;
			case DayDate9:
				// For LV Digital1 face
				DateTemplateToAscii( "^1r - ^0z ", Now.month, Now.day, Now.year, DateStr, sizeof( DateStr ) );
				Str[4] = '\0';
				#define toupper( c ) c = ( c >= 'a' ) ? ( c - 'a' + 'A' ) : c;
				toupper( Str[0] );
				toupper( Str[1] );
				toupper( Str[2] );
				StrCopy( Str, Now.hour < 12 ? "  AM" : "  PM" );
				StrCat( DateStr, Str );
				DateWidth = FntCharsWidth( DateStr, StrLen( DateStr ) );
				DateX = CenterX - DateWidth / 2 + 1;
				DateY = Hour10Y + Hour10Height + 3;
				WinDrawChars( DateStr, StrLen( DateStr ), DateX, DateY );
		}
	}
		
	// Draw the Hour 10's digit

	if ( ( FullDraw == true ) || ( Hour10 != gP->PrevHour10 ) ) {
		if ( ( Hour10 != 0 ) || ( gP->WatchFace.Digital.DayDate == DayDate10 ) )
		    WinDrawBitmap( pHour10, Hour10X, Hour10Y );
	}

	// If we didn't draw 10's digit then we need to erase a
	// possible left-over 1's digit from the previous time

	if ( ( Hour10 == 0 ) && ( gP->WatchFace.Digital.DayDate == DayDate2 ) ) {
		RectangleType r;
		r.topLeft.x = Hour10X + Hour10Width + gP->WatchFace.Digital.ExtraSpace;
		r.topLeft.y = Hour10Y;
		r.extent.x = Hour10Width;
		r.extent.y = Hour10Height;
		WinEraseRectangle( & r, 0 );
	}
	
	// Draw the Hour 1's digit

	if ( ( FullDraw == true ) || ( Hour1 != gP->PrevHour1 ) )
	    WinDrawBitmap( pHour1, Hour1X, Hour1Y );

	// Draw the separator

	if ( FullDraw == true )
	    WinDrawBitmap( pSeparator, SeparatorX, SeparatorY );

	// Draw the Minute 10's digit

	if ( ( FullDraw == true ) || ( Minute10 != gP->PrevMinute10 ) )
	    WinDrawBitmap( pMinute10, Minute10X, Minute10Y );

	// Draw the Minute 1's digit

	if ( ( FullDraw == true ) || ( Minute1 != gP->PrevMinute1 ) )
	    WinDrawBitmap( pMinute1, Minute1X, Minute1Y );

	// If the face is inverted then restore the foreground and background colors

	if ( gP->WatchFace.Digital.Inverted == InvertedYes ) {
		WinSetForeColor( curForeColor );
		WinSetBackColor( curBackColor );
		WinSetTextColor( curTextColor );
	}

	// Unlock the bitmap resources

	if ( gP->hFace != 0 )
		MemHandleUnlock( gP->hFace );

	MemHandleUnlock( gP->hSeparator );
	MemHandleUnlock( gP->hNumber[Hour1] );
	MemHandleUnlock( gP->hNumber[Hour10] );
	MemHandleUnlock( gP->hNumber[Minute10] );
	MemHandleUnlock( gP->hNumber[Minute1] );

	// Update the previous time
	
	gP->PrevDateTime = Now;

	gP->PrevDay	 	 = Day;
	gP->PrevHour10   = Hour10;
	gP->PrevHour1    = Hour1;
	gP->PrevMinute10 = Minute10;
	gP->PrevMinute1  = Minute1;

}

// Display alarm icon and play sound if alarm has triggered

void HandleAlarmTriggered( WatchGlobalsPtr gP, Boolean PlaySound )
{

	Err		AlarmTriggered;
	UInt32	Ver;

	AlarmTriggered = kFossilSystemStatusAlarmNotTriggered;

	if ( WPdaGetVersion( & Ver ) == errNone )
		AlarmTriggered = FossilGetSystemStatus( kFossilGetAlarmState );

	if ( AlarmTriggered == kFossilSystemStatusAlarmTriggered ) {

		// Draw the alarm triggered icon on top of the current watch face
		if ( gP->hAlarm != NULL ) {
			BitmapPtr pAlarm;
			pAlarm = MemHandleLock( gP->hAlarm );
			if ( pAlarm != NULL ) {
				// Draw the alarm triggered icon bitmap
			    WinDrawBitmap( pAlarm, 0, 0 );
				MemHandleUnlock( gP->hAlarm );
			}
		}

		// Play the alarm sound so the user knows an alarm has triggered
		if ( PlaySound == true ) {
			FossilUpdateDisplay();
			SndPlaySystemSound( sndAlarm );
		}

	}

}
	
// Display low battery icon if the battery is low

void HandleLowBattery( WatchGlobalsPtr gP )
{

	Err		  BatteryState;
	MemHandle hBmp;
	UInt32	  Ver;

	BatteryState = kFossilSystemStatusBatteryOk;

	if ( WPdaGetVersion( & Ver ) == errNone )
		BatteryState = FossilGetSystemStatus( kFossilGetBatteryState );

	if ( BatteryState != kFossilSystemStatusBatteryOk ) {

		// Draw the low battery icon on top of the current watch face
		if ( BatteryState == kFossilSystemStatusBatteryEmpty )
			hBmp = gP->hEmptyBatt;
		else if ( BatteryState == kFossilSystemStatusBatteryVeryLow )
			hBmp = gP->hVLowBatt;
		else
			hBmp = gP->hLowBatt;
		if ( hBmp != NULL ) {
			BitmapPtr pBmp;
			pBmp = MemHandleLock( hBmp );
			if ( pBmp != NULL ) {
				// Draw the low battery icon bitmap
			    WinDrawBitmap( pBmp, 122, 0 );
				MemHandleUnlock( hBmp );
			}
		}

	}

}

// Draw the time on the current watch face

void DrawTime( WatchGlobalsPtr gP, BOOL FullDraw )
{

	if ( gP->WatchFace.Digital.Type == Digital )
		DrawDigitalTime( gP, FullDraw );
	else
		DrawAnalogTime( gP, FullDraw );

}

// Load the resource for the current watch face

void LoadFaceResources( WatchGlobalsPtr gP )
{

	MemHandle h;
	UInt16    i;

	if ( gP->WatchFace.Digital.Type == Digital ) {
		h = LoadResource( 'Tbmp', gP->WatchFace.Digital.FaceResId );
		gP->hFace = h;
		for ( i = 0; i < 10; i++ ) {
			h = LoadResource( 'Tbmp', gP->WatchFace.Digital.NumberResIdBase + ( i * gP->WatchFace.Digital.NumberResIdInc ) );
			gP->hNumber[i] = h;
		}
		h = LoadResource( 'Tbmp', gP->WatchFace.Digital.SeparatorResId );
		gP->hSeparator = h;
	} else {
		h = LoadResource( 'Tbmp', gP->WatchFace.Analog.FaceResId );
		gP->hFace = h;
	}

}

// Free the resources for the current watch face

void FreeFaceResources( WatchGlobalsPtr gP )
{

	UInt16 i;

	if ( gP->WatchFace.Digital.Type == Digital ) {
		if ( gP->hFace != NULL ) {
			DmReleaseResource( gP->hFace );
			gP->hFace = NULL;
		}
		for ( i = 0; i < 10; i++ ) {
			if ( gP->hNumber[i] != NULL ) {
				DmReleaseResource( gP->hNumber[i] );
				gP->hNumber[i] = NULL;
			}
		}
		if ( gP->hSeparator != NULL ) {
			DmReleaseResource( gP->hSeparator );
			gP->hSeparator = NULL;
		}
	
	} else {
		if ( gP->hFace != NULL ) {
			DmReleaseResource( gP->hFace );
			gP->hFace = NULL;
		}
	}

}

// Switch to a new watch face

void SwitchFace( WatchGlobalsPtr gP, Int16 Direction )
{

	if ( Direction < 0 ) {
		// Switch to previous face
		if ( gP->g_prefs.WatchFaceIndex == 0 )
			gP->g_prefs.WatchFaceIndex = gP->NumWatchFaces - 1;
		else
			gP->g_prefs.WatchFaceIndex--;
	} else if ( Direction > 0 ) {
		// Switch to next face
		gP->g_prefs.WatchFaceIndex = ( gP->g_prefs.WatchFaceIndex + 1 ) % gP->NumWatchFaces;
	}

	FreeFaceResources( gP );
	MemMove( & ( gP->WatchFace ), ( & gP->WatchFaces[ gP->g_prefs.WatchFaceIndex ] ), sizeof( WatchFaceType ) );
	LoadFaceResources( gP );

	DrawTime( gP, true );

    // Write the saved preferences / saved-state information.
    // This data will be saved during a HotSync backup.

    PrefSetAppPreferences( GLOBALS_FEATURE_CREATOR, GLOBALS_FEATURE_ID, appPrefVersionNum, 
					       & gP->g_prefs, sizeof(WatchPreferenceType), true );

	#if 0
	if ( GDbgWasEntered ) {
		char s[80];
		sprintf( s, "Watch: SwitchFace: gP->g_prefs.WatchFaceIndex = %ld\n", (UInt32) gP->g_prefs.WatchFaceIndex );
		DbgMessage( s );
	}
	#endif

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

static Boolean AppHandleEvent( WatchGlobalsPtr gP, EventType * eventP )
{

	if (eventP->eType == keyDownEvent) {
		if ( ( eventP->data.keyDown.chr == vchrPageUp ) ||
		     ( eventP->data.keyDown.chr == vchrThumbWheelUp ) ) {
			// Switch to previous face
			SwitchFace( gP, -1 );
			return true;
		}
		else if ( ( eventP->data.keyDown.chr == vchrPageDown ) ||
		          ( eventP->data.keyDown.chr == vchrThumbWheelDown ) ) {
			// Switch to next face
			SwitchFace( gP, +1 );
			return true;
		}
		else if ( ( eventP->data.keyDown.chr == vchrThumbWheelBack ) ||
				  ( eventP->data.keyDown.chr == vchrThumbWheelPush ) ) {
			// Translate the Back and Enter keys to an open launcher event.
			EventType newEvent;
			newEvent = *eventP;
			newEvent.eType = ctlSelectEvent;
			newEvent.tapCount = 1;
			newEvent.eType = keyDownEvent;
			newEvent.data.keyDown.chr = launchChr;
			newEvent.data.keyDown.modifiers = commandKeyMask;
			EvtAddEventToQueue( &newEvent );
			return true;
		}
	}

    return false;

}

/*
 * FUNCTION: FormHandleEvent
 *
 * DESCRIPTION: 
 *
 * Event handler for our dummy form (just calls AppHandleEvent).
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

static Boolean FormHandleEvent( EventType * eventP )
{
	WatchGlobalsPtr gP;
	gP = InitGlobals( 0 );
	if ( gP != NULL )
		return AppHandleEvent( gP, eventP );
	else
		return false;
}

/*
 * FUNCTION: AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 */

static void AppEventLoop( WatchGlobalsPtr gP )
{

    UInt16 error, Timeout;
    EventType event;
	int skipSys;
	volatile UInt32 key;

	// For now we want to see all keys.

	KeySetMask( 0xFFFFFFFF );

	// Update the time once a minute

	#if FAST_TIME
	Timeout = SysTicksPerSecond();
	#else
	Timeout = SysTicksPerSecond() * 60;
	#endif

    do {

        EvtGetEvent(&event, Timeout);

		if ( (event.eType == nilEvent) ||
			 ( (event.eType == keyDownEvent) &&
			   (event.data.keyDown.chr == vchrLateWakeup) ) ) {
			DrawTime( gP, false );
		}

        if ( ! AppHandleEvent( gP, & event ) )
			SysHandleEvent( & event );

    } while (event.eType != appStopEvent);

}

/*
 * FUNCTION: AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * RETURNED:
 *     errNone - if nothing went wrong
 */

static Err AppStart( WatchGlobalsPtr gP )
{

    UInt16 prefsSize;
	UInt32 Width, Height, Depth;
	Boolean Color;

    // Read the saved preferences / saved-state information

    prefsSize = sizeof(WatchPreferenceType);

    if (PrefGetAppPreferences(
        GLOBALS_FEATURE_CREATOR, GLOBALS_FEATURE_ID, &gP->g_prefs, &prefsSize, true) == 
        noPreferenceFound)
    {
		// Failed to get preferences, so use defaults
		gP->g_prefs.WatchFaceIndex = DEFAULT_WATCH_FACE_INDEX;
		#if 0
		if ( GDbgWasEntered ) {
			char s[80];
			sprintf( s, "Watch: AppStart: Using default WatchFaceIndex\n" );
			DbgMessage( s );
		}
		#endif

    }
    
	#if 0
	if ( GDbgWasEntered ) {
		char s[80];
		sprintf( s, "Watch: AppStart: gP->g_prefs.WatchFaceIndex = %ld\n", (UInt32) gP->g_prefs.WatchFaceIndex );
		DbgMessage( s );
	}
	#endif

	// Get the alarm triggered and low battery icons
	gP->hAlarm     = LoadResource( 'Tbmp', AlarmResId );
	gP->hEmptyBatt = LoadResource( 'Tbmp', BatteryEmptyResId );
	gP->hLowBatt   = LoadResource( 'Tbmp', BatteryLowResId );
	gP->hVLowBatt  = LoadResource( 'Tbmp', BatteryVeryLowResId );

	// Save the previous display state
	WinPushDrawState();
	gP->PrevDrawWindow = WinGetDrawWindow();

	// Switch to full screen draw window
	WinSetDrawWindow( WinGetDisplayWindow() );
	
	// Set the default drawing colors
	WinSetForeColor( UIColorGetTableEntryIndex( UIObjectForeground ) );
	WinSetBackColor( UIColorGetTableEntryIndex( UIFieldBackground ) );
	WinSetTextColor( UIColorGetTableEntryIndex( UIFieldText ) );

	LoadFaceResources( gP );

	SwitchFace( gP, 0 );

    return errNone;

}

/*
 * FUNCTION: AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 */

static void AppStop( WatchGlobalsPtr gP )
{

	// Restore the previous display state
	if (WinValidateHandle( gP->PrevDrawWindow ))
		WinSetDrawWindow( gP->PrevDrawWindow );
	WinPopDrawState();

	// Release the alarm triggered and low battery icon resources
	if ( gP->hAlarm != NULL )
		DmReleaseResource( gP->hAlarm );
	if ( gP->hLowBatt != NULL )
		DmReleaseResource( gP->hLowBatt );
	if ( gP->hVLowBatt != NULL )
		DmReleaseResource( gP->hVLowBatt );
	if ( gP->hEmptyBatt != NULL )
		DmReleaseResource( gP->hEmptyBatt );

	// Free the watch face resources
	FreeFaceResources( gP );

}

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

    // See if we're on in minimum required version of the ROM or later.

    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

    if (romVersion < requiredVersion)
    {
        if ((launchFlags & 
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            FrmAlert (RomIncompatibleAlert);
            // Palm OS versions before 2.0 will continuously relaunch
            // this app unless we switch to another safe one.
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

	Boolean				extFontState;
    Err					error;
	LocalID				dbID;
	UInt16				cardNo;
	UInt32				Ver;
	SysNotifyParamType	* pNotify;
	WatchGlobalsPtr 	gP;

	// Initialize global data

	gP = InitGlobals( cmd );

    error = RomVersionCompatible (ourMinVersion, launchFlags);

    if ( error )
    	return error;

    switch (cmd) {

	    case sysAppLaunchCmdNormalLaunch:

			// Init a dummy form to keep Attention Manager from crashing
			// if an alarm triggers while Watch is running as an app
			// Note: Must do this *before* calling AppStart
			gP->pFrm = FrmInitForm( 150 );
			FrmSetActiveForm( gP->pFrm );
			FrmSetEventHandler( gP->pFrm, & FormHandleEvent );

    	    error = AppStart( gP );

	        if ( error ) 
	            return error;

			// Inform the HAL that the current app is a Watch app
			FossilSetWatchApp( true );

			// Register to receive fossilNotifyWatchModeWakeup notification
			error = SysCurAppDatabase( & cardNo, & dbID );
			if ( error == errNone ) {
				SysNotifyRegister( cardNo, dbID,
				                   fossilNotifyWatchModeWakeup, 
								   NULL, sysNotifyNormalPriority, NULL );
			}
			
			// Set automatic display update mode
			FossilDisplayRefreshRateSet( kFossilRefreshAuto );

			// Save the extended font state
			FossilExtendedFontSelectGet( & extFontState );

			// Enable the Fossil 8 point fonts
			FossilExtendedFontSelectSet( true );

			// Indicate we want app-specific Back button handling
			if ( WPdaGetVersion( & Ver ) == errNone )
				FossilBackKeyModeSet( kFossilBackKeyNoAction );

	        // Enter the main event loop
	        AppEventLoop( gP );
        
			// Shutdown gracefully
	        AppStop( gP );

			// Delete the dummy form
			FrmDeleteForm( gP->pFrm );

			// Restore generic Back button handling
			if ( WPdaGetVersion( & Ver ) == errNone )
				FossilBackKeyModeSet( kFossilBackKeyLauncher );

			// Restore the extended font state
			FossilExtendedFontSelectSet( extFontState );

			// Set default display update mode
			FossilDisplayRefreshRateSet( kFossilRefreshDefault );

			// Unregister the fossilNotifyWatchModeWakeup notification
			error = SysCurAppDatabase( & cardNo, & dbID );
			if ( error == errNone ) {
				SysNotifyUnregister( cardNo, dbID,
				                     fossilNotifyWatchModeWakeup, 
								     sysNotifyNormalPriority );
			}
			
			// Inform the HAL that the current app is not a Watch app
			FossilSetWatchApp( false );

	        break;

		case wpdaAppLaunchWatchDrawTime:

			// Draw the time with the current face and immediately return

    	    error = AppStart( gP );

	        if ( error ) 
	            return error;

			FossilExtendedFontSelectGet( & extFontState );

			FossilExtendedFontSelectSet( true );

			DrawTime( gP, true );

			HandleAlarmTriggered( gP, true );

			HandleLowBattery( gP );

	        AppStop( gP );

			FossilExtendedFontSelectSet( extFontState );

			break;

		case wpdaAppLaunchWatchFaceNext:

			// Switch to the next face, draw the time, and immediately return

    	    error = AppStart( gP );

	        if ( error ) 
	            return error;

			FossilExtendedFontSelectGet( & extFontState );

			FossilExtendedFontSelectSet( true );

			SwitchFace( gP, +1 );

			DrawTime( gP, true );

			HandleAlarmTriggered( gP, false );

			HandleLowBattery( gP );

	        AppStop( gP );

			FossilExtendedFontSelectSet( extFontState );

			break;

		case wpdaAppLaunchWatchFacePrev:

			// Switch to the previous face, draw the time, and immediately return

    	    error = AppStart( gP );

	        if ( error ) 
	            return error;

			FossilExtendedFontSelectGet( & extFontState );

			FossilExtendedFontSelectSet( true );

			SwitchFace( gP, -1 );

			DrawTime( gP, true );

			HandleAlarmTriggered( gP, false );

			HandleLowBattery( gP );

	        AppStop( gP );

			FossilExtendedFontSelectSet( extFontState );

			break;

		case sysAppLaunchCmdNotify:

			pNotify = (SysNotifyParamType *) cmdPBP;

			if ( ( pNotify->notifyType == fossilNotifyWatchModeWakeup ) &&
			     ( pNotify->broadcaster == WPdaCreator ) ) {
				// A Watch app may respond to this notification as it sees fit.
				// The reference Watch app exits when it receives this notification.
				EventType Event;
				MemSet( & Event, sizeof( Event ), 0 );
				Event.eType = appStopEvent;
				EvtAddEventToQueue( & Event );
			}
			
			break;

	    default:

	        break;

    }

    return errNone;

}

// Turn a5 warning off to prevent it being set off by C++
// static initializer code generation
#pragma warn_a5_access reset
