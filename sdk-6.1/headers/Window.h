/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Window.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *        This file defines window structures and routines that support color.
 *
 *****************************************************************************/

/*!
	\file Window.h
*/

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <Font.h>
#include <Rect.h>
#include <Bitmap.h>
#include <CmnDrawingTypes.h>

#include <limits.h>

//-----------------------------------------------
//  More graphics shapes
//-----------------------------------------------
typedef struct WinLineType 
{
	Coord x1;
	Coord y1;
	Coord x2;
	Coord y2;
} WinLineType;


#define 	kWinVersion			10

// WindowType is no longer public, and looks nothing like it
// did in previous versions of the OS.  Any attempt to use it
// as such with be dealt with severely.
struct WindowType;
typedef struct WindowType* WinHandle;
#define invalidWindowHandle		0

// enum for WinScrollRectangle
enum WinDirectionTag 
{ 
	winUp = 0, 
	winDown, 
	winLeft, 
	winRight 
};
typedef Enum8 WinDirectionType;

// enum for WinCreateOffscreenWindow
enum WindowFormatTag 
{ 
	screenFormat = 0, 
	genericFormat, 
	nativeFormat 
};
typedef Enum8 WindowFormatType;


//! Flags for WinCreateWindowWithConstraints()
enum WinFlagsTag
{
	winFlagModal			= 0x00000001,	//!< Same as "modal" in WinCreateWindow().
	winFlagNonFocusable		= 0x00000002,	//!< Inverse of "focusable" in WinCreateWindow().
	winFlagBackBuffer		= 0x00000004,	//!< Use a backbuffer for this window.
	winFlagVisibilityEvents	= 0x00000008,	//!< Request visibility changed events -- very high overhead, be careful.

	winLayerMask			= 0x000000f0,	//!< Bits used to define window ordering.
	winLayerNormal			= 0x00000000,	//!< Application windows.  \sa winLayerMask
	winLayerSlip			= 0x00000040,	//!< Sliplets -- windows containing Status Bar slips.  \sa winLayerMask
	winLayerPriority		= 0x00000010,	//!< Priority windows -- appear between slips and AIA.  \sa winLayerMask
	winLayerSecurity		= 0x00000050,	//!< Security windows -- only for use by password dialogs etc.  \sa winLayerMask
	winLayerSystem			= 0x00000020,	//!< System window -- hard UI, such as AIA and Status Bar.  \sa winLayerMask
	winLayerMenu			= 0x00000030,	//!< Menu window -- for temporary popup menus.  \sa winLayerMask
	winLayerOverlay			= 0x00000060,	//!< Overlay window -- on top of all other windows.  \sa winLayerMask

	winFlagReserved1		= 0x00000100,	//!< private
	winFlagHidden			= 0x00000200,	//!< \since 6.1 Window starts out hidden

	//!	\since 6.1 Enable window compositing.
	/*!	If this flag is set, the window will be composited with other windows
		behind it.  The way it is composited must be described further by calling
		one of the compositing mode functions: WinSetDirectComposited() or
		WinSetAlphaComposited().  The window will not be displayed until you
		select a compositing mode. */
	winFlagComposited		= 0x00000400,

	//!	\since 6.1 Do not dim out display behind window.
	/*!	Normally a modal window will dim out everything that is behind
		it (in Z order) on the display.  By setting this flag you can disable
		the feature; it is useful, for example, when showing menus, which
		need to be modal but are transient and thus perform this dimming. */
	winFlagNoDimBehind		= 0x00000800,

	//!	\since 6.1 Bits used to control window batching.
	winBatchMask			= 0x00007000,

	//!	\since 6.1 Don't batch this operation.
	/*!	A window transaction will be started immediately, committing this
		and any other pending operations. */
	winBatchImmediate		= 0x00000000,

	//! \since 6.1 Batch this window open operation.
	/*!	If this flag is set, the OS will attempt to batch this
		WinCreateWindowWithConstraints() call with other following window manager
		operations.  The system uses a "reasonable but not intrusive" delay in
		which the batching may occur before the window is ultimately displayed. */
	winBatchShort			= 0x00001000,

	//!	\since 6.1 Like winFlagBatched, but a longer delay.
	winBatchLong			= 0x00002000,

	//!	\since 6.1 Like winFlagBatched, but delay forever.
	/*!	The window will not be shown until some other window is created or
		deleted, a window's constraints change, or something else causes
		the window manager to start a transaction. */
	winBatchInfinite		= 0x00003000,
	
	winFormWindow			= 0x00008000	//!< \since 6.1 Set if window is a form

};
typedef uint32_t WinFlagsType;

typedef struct WinConstraintsType
{
	uint32_t					x_flags;			// x dimension flags (set to 0 for now)
	int16_t						x_pos;				// x dimension constraints (in dibits)
	int16_t						x_min;
	int16_t						x_pref;
	int16_t						x_max;
	uint32_t					y_flags;			// x dimension flags (set to 0 for now)
	int16_t						y_pos;				// y dimension constraints (in dibits)
	int16_t						y_min;
	int16_t						y_pref;
	int16_t						y_max;
} WinConstraintsType;

// special numbers you can use for constraints.
#define winUndefConstraint	SHRT_MIN
#define winMaxConstraint	SHRT_MAX

// enum for WinLockScreen
enum WinLockInitTag 
{
	winLockCopy, 
	winLockErase, 
	winLockDontCare
};
typedef Enum8 WinLockInitType;


// operations for the WinScreenMode function
enum WinScreenModeOperationTag 
{
	winScreenModeGetDefaults,
	winScreenModeGet,
	winScreenModeSetToDefaults,
	winScreenModeSet,
	winScreenModeGetSupportedDepths,
	winScreenModeGetSupportsColor
};
typedef Enum8 WinScreenModeOperation;


// Operations for the WinPalette function
#define winPaletteGet 				0
#define winPaletteSet 				1
#define winPaletteSetToDefault		2


#define DrawStateStackSize			7		// enough to draw things immediately if we're in an update


// constants used by WinSetCoordinateSystem
#define kCoordinatesNative			0
#define kCoordinatesStandard		72
#define kCoordinatesOneAndAHalf		108
#define kCoordinatesDouble			144
#define kCoordinatesTriple			216
#define kCoordinatesQuadruple		288


// selectors for WinScreenGetAttribute
enum WinScreenAttrTag 
{
	winScreenWidth,
	winScreenHeight,
	winScreenRowBytes,
	winScreenDepth,
	winScreenAllDepths,
	winScreenDensity,
	winScreenPixelFormat,
	winScreenResolutionX,
	winScreenResolutionY
};
typedef Enum8 WinScreenAttrType;

// flags for WinSetScalingMode
#define kBitmapScalingOff		1
#define kTextScalingOff			2
#define kTextPaddingOff			4	


//-----------------------------------------------
// The Window Structures.
//-----------------------------------------------


// Don't use this union for the frames of windows, use the constants below.
// But we have to keep the union because it's how you draw rectangles.
typedef union FrameBitsType 
{
	struct oldBitsTag
	{
		uint16_t cornerDiam		: 8;				// corner radius (not diameter!), max 38
		uint16_t reserved_3		: 3; 
		uint16_t threeD			: 1;				// Draw 3D button    
		uint16_t shadowWidth	: 2;				// Width of shadow
		uint16_t width			: 2;				// Width frame
	} bits;
	uint16_t word;									// IMPORTANT: INITIALIZE word to zero before setting bits!
} FrameBitsType;
typedef uint16_t FrameType;

//  Standard Frame Types
#define simpleFrame     0x4000
#define simple3DFrame   0x8800          // 3d, frame = 2
#define roundFrame      0x4004          // corner = 4, frame = 1
#define boldRoundFrame  0x8007          // corner = 7, frame = 2
#define popupFrame      0x5002          // corner = 2, frame = 1, shadow = 1
#define dialogFrame     0x8003          // corner = 3, frame = 2

#define noFrame				0x0000
#define rectangleFrame		simpleFrame
#define menuFrame			popupFrame	// If you're making a menu, use menuWindowFrame,
										// but since this is the same as popupFrame, we
										// can't change it.

// These frame types can only be used for window decors, they can't be used
// in WinPaintRectangleFrame and friends.  They use the reserved bits to
// say that they're different, and then the rest of the bits just have to
// be unique in this range
#define menuBarFrame	0x0101			// reserved_3 = 1
#define menuWindowFrame	0x0102			// reserved_3 = 1


// ----------------------
// Window manager errors
// ----------------------
#define	winErrPalette					(winErrorClass | 1)
#define	winErrInvalidWindowHandle		(winErrorClass | 2)
#define winErrNoUI						(winErrorClass | 3)


//-----------------------------------------------
//  Macros
//-----------------------------------------------

// The window handle used to be a pointer to a window structure,
// however now, it is not.  This macro checks to see if the 
// handle represends a valid window.

#if BUILD_TYPE == BUILD_TYPE_DEBUG
#	define ECWinValidateHandle(winHandle) if(!WinValidateHandle(winHandle)) DbgOnlyFatalError("[ECWinValidateHandle] Invalid window handle.")
#else
#	define ECWinValidateHandle(winHandle) 
#endif
	
#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
// Multithreading
//-----------------------------------------------

// Start/stop UI context for thread.  These calls nest, so you
// can call them multiple times on the same thread (and call
// them in the UI thread).  Once the UI is finally finished, all
// UI state associated with the thread will be deallocated.
// These will be called automatically for the main UI thread
// (that is, normal applications).
extern status_t			WinStartThreadUI(void);
extern status_t			WinFinishThreadUI(void);

// Win*UserData lets you associate arbitrary data with a drawing context.
status_t WinAddUserData(uint32_t code, void* data);
void* WinGetUserData(uint32_t code);
status_t WinRemoveUserData(uint32_t code);

//-----------------------------------------------
// Window management
//-----------------------------------------------

extern Boolean WinValidateHandle (WinHandle winHandle);

extern WinHandle WinCreateWindow (	const RectangleType *bounds, FrameType frame, 
									Boolean modal, Boolean focusable, status_t *error);

extern WinHandle WinCreateWindowWithConstraints (	FrameType frame, WinFlagsType flags,
													const WinConstraintsType* constraints, status_t *error);

extern WinHandle WinCreateOffscreenWindow (	Coord width, Coord height, 
											WindowFormatType format, status_t *error);

extern WinHandle WinCreateBitmapWindow (BitmapType *bitmapP, status_t *error);

extern void WinDeleteWindow(WinHandle winHandle, Boolean eraseIt);

// Return the flags given to WinCreateWindowWithConstraints()
extern WinFlagsType WinGetWindowFlags(WinHandle winHandle);

// Returns the current size and position of the window --
// if your window supports constraints, this is the "drawing frame"
// otherwise, your bounds are fixed, and this is still your "drawing frame"
extern void WinGetBounds (WinHandle winH, RectangleType *rP);

// Set window to a fixed size.  This API is only for legacy windows.
// Transitional windows (those that have constraints) can not use it.
// You can achieve the same effect by calling WinSetConstraints() with
// your minimum, preferred, and maximums all set to the extents you
// would supply here.
extern void WinSetBounds (WinHandle winHandle, const RectangleType *rP);

#define WinGetWindowBounds(rP)         (WinGetDrawWindowBounds((rP)))
#define WinSetWindowBounds(winH, rP)   (WinSetBounds((winH), (rP)))

/*!	This function sets the constraints.  A layout transaction will be
	triggered, and eventually your size will be changed, and an update
	will be issued. */
extern status_t	WinSetConstraints(WinHandle winHandle, const WinConstraintsType* constraints);

/*!	\since 6.1
	Like WinSetConstraints(), but the constraints take effect immediately
	without batching. */
extern status_t	WinSetConstraintsImmediate(WinHandle winHandle, const WinConstraintsType* constraints);

// Call this to request that a window receive input focus.  You will
// later receive a winFocusGained event if the system decides you get it.
// For now, always pass in 0 for flags.
extern status_t	WinRequestFocus(WinHandle winHandle, uint32_t flags);

extern Boolean WinModal(WinHandle winHandle);

extern void WinSetFrameType(const WinHandle winH, FrameType frame);
extern FrameType WinGetFrameType(const WinHandle winH);

extern void WinGetWindowFrameRect(WinHandle winHandle, RectangleType *r);

extern void WinSetActiveWindow (WinHandle winHandle);
extern WinHandle WinGetActiveWindow (void);

//!	(post-6.0.1) Make the given window visible.
extern void WinShow(WinHandle winHandle);

//!	(post-6.0.1) Make the given window hidden.
extern void WinHide(WinHandle winHandle);

//-----------------------------------------------
// Window compositing API
//-----------------------------------------------

//! (post-6.0.1) Set direct compositing mode for a winFlagComposited window.
/*!	In direct compositing, your window draws directly on top
	of whatever is behind it.  You control compositing by varying
	the alpha level of your drawing.  This is the most efficient
	compositing mode (it does not require any extra graphics
	memory like WinSetAlphaComposited() does), but it can not be used with forms.*/
extern status_t WinSetDirectComposited(WinHandle winHandle);

//!	(post-6.0.1) Set alpha compositing mode for a winFlagComposited window.
/*!	In alpha compositing, you select an alpha value for the
	entire window, and the contents of that window will be modulated
	by the alpha value before ultimately being composited to the screen.
	This allows you to use a form in the window, since the alpha
	value supplied here controls transparency.  It requires an
	extra graphics buffer in which to render the window before
	it is composited to the screen, and so is heavier-weight than
	WinSetDirectComposited().  The alpha value can range from 0 to 1. */
extern status_t WinSetAlphaComposited(WinHandle winHandle, float alpha);

//-----------------------------------------------
// Screen-related stuff
//-----------------------------------------------

extern WinHandle WinGetDisplayWindow (void);

extern void WinGetDisplayExtent (Coord *extentX, Coord *extentY);

extern void WinGetWindowExtent (Coord *extentX, Coord *extentY);
extern void WinDisplayToWindowPt (Coord *x, Coord *y);
extern void WinWindowToDisplayPt (Coord *x, Coord *y);

// #pragma mark New WinScreen functions
// #pragma -

extern status_t WinScreenMode(WinScreenModeOperation operation, 
						uint32_t *widthP,
						uint32_t *heightP, 
						uint32_t *depthP, 
						Boolean *enableColorP);

// #pragma mark Screen tracking (double buffering) support
// #pragma -

extern uint8_t *WinScreenLock(WinLockInitType initMode);
extern void WinScreenUnlock(void);

//-----------------------------------------------
// Update control
//-----------------------------------------------

enum {
	winInvalidateDestroy = 0,		// Invalidate failed -- clean up state
	winInvalidateExecute = 1		// Invalidate performed -- execute; DON'T FREE THE STATE.
};

typedef Boolean (*winInvalidateFunc)(int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state);

extern void WinInvalidateWindow(WinHandle window);
extern void WinInvalidateRect(WinHandle window, const RectangleType *dirtyRect);
extern void WinInvalidateRectFunc(WinHandle window, const RectangleType *dirtyRect,
								  winInvalidateFunc func, void *state);

// Request a scroll operation in the window.  Unlike the old
// WinScrollRectangle API, this one is asynchronous and performs
// no drawing -- you must wait for an update event.
extern status_t WinScrollRectangleAsync(WinHandle winHandle, const RectangleType *rP, Coord dx, Coord dy);

// WinScrollRectangleAsyncWithCallback() lets you have control over how
// you invalidate the region exposed by the scroll (really, the move).
// The Field code uses it to call WinInvalidateRectFunc() and bypass
// much of the dispatch mechanics for back-buffered windows.
typedef void (WinScrollInvalidateFunc)(WinHandle winHandle, const RectangleType *r, void *cookie);
extern status_t WinScrollRectangleAsyncFunc(WinHandle winHandle, const RectangleType *rP, Coord dx, Coord dy, WinScrollInvalidateFunc *invalidate, void *cookie);

extern void WinFlush(void);

//-----------------------------------------------
// The fine line between window management and drawing
//-----------------------------------------------

extern WinHandle WinSetDrawWindow (WinHandle winHandle);
extern WinHandle WinGetDrawWindow (void);

extern void WinGetDrawWindowBounds (RectangleType *rP);

extern void WinEraseWindow (void);

extern BitmapType *WinGetBitmap (WinHandle winHandle);

// Many, many parts of this API have been deprecated.  srcWin can not
// be an update-based window.  Only use the winPaint drawing mode.
extern void WinCopyRectangle(	WinHandle srcWin, WinHandle dstWin, 
								const RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode);

// Traditional scrolling API, performs drawing during the call
// Use WinScrollRectangleAsync() in update-based windows.
extern void WinScrollRectangle(	const RectangleType *rP, WinDirectionType direction,
								Coord distance, RectangleType *vacatedP);

// Get the dimensions of a bitmap, but scaled if necessary to the "active"
// coordinate system.  That is, given a bitmap, it will tell you how big
// the bitmap will be when it's actually drawn.  All the other functions
// to get the size of the bitmap give you the dimension in pixels...
extern void WinGetBitmapDimensions(const BitmapType *bmP, Coord *widthP, Coord *heightP);

extern void WinGetClip (RectangleType *rP);
extern void WinSetClip (const RectangleType *rP);
extern void WinResetClip (void);
extern void WinClipRectangle (RectangleType *rP);

extern void WinTheRatRace(void);

//-----------------------------------------------
// Routines to draw shapes or frames shapes      
//-----------------------------------------------
// #pragma mark Routines to draw shapes or frames shapes
// #pragma -

// Pixel(s)
extern IndexedColorType WinGetPixel (Coord x, Coord y);
extern status_t WinGetPixelRGB (Coord x, Coord y, RGBColorType* rgpP); // Direct color
extern void WinPaintPixel (Coord x, Coord y);				// uses drawing mode
extern void WinDrawPixel (Coord x, Coord y);
extern void WinErasePixel (Coord x, Coord y);
extern void WinInvertPixel (Coord x, Coord y);
extern void WinPaintPixels (uint16_t numPoints, PointType pts[]);

// Line(s)
extern void WinPaintLines (uint16_t numLines, WinLineType lines[]);
extern void WinPaintLine (Coord x1, Coord y1, Coord x2, Coord y2);
extern void WinPaintThinLine (Coord x1, Coord y1, Coord x2, Coord y2);
extern void WinDrawLine (Coord x1, Coord y1, Coord x2, Coord y2);
extern void WinDrawGrayLine (Coord x1, Coord y1, Coord x2, Coord y2);
extern void WinEraseLine (Coord x1, Coord y1, Coord x2, Coord y2);
extern void WinInvertLine (Coord x1, Coord y1, Coord x2, Coord y2);
extern void WinFillLine (Coord x1, Coord y1, Coord x2, Coord y2);


// Rectangle
extern void WinPaintRectangle (const RectangleType *rP, uint16_t cornerDiam);
extern void WinDrawRectangle (const RectangleType *rP, uint16_t cornerDiam);
extern void WinEraseRectangle (const RectangleType *rP, uint16_t cornerDiam);
extern void WinInvertRectangle (const RectangleType *rP, uint16_t cornerDiam);
extern void WinFillRectangle (const RectangleType *rP, uint16_t cornerDiam);

// Rectangle frames
extern void WinPaintRectangleFrame (FrameType frame, const RectangleType *rP);
extern void WinPaintRoundedRectangleFrame (const RectangleType *rP, Coord width, Coord cornerRadius, Coord shadowWidth);
extern void WinDrawRectangleFrame (FrameType frame, const RectangleType *rP);
extern void WinDrawGrayRectangleFrame (FrameType frame, const RectangleType *rP);
extern void WinEraseRectangleFrame (FrameType frame, const RectangleType *rP);
extern void WinInvertRectangleFrame (FrameType frame, const RectangleType *rP);
extern void WinGetFramesRectangle (FrameType  frame, const RectangleType *rP, RectangleType *obscuredRect);

// Bitmap
//! See WinPaintBitmapHandle() in GcRender.h, for a more efficient way to draw bitmaps.
extern void WinDrawBitmap (BitmapPtr bitmapP, Coord x, Coord y);

//! See WinPaintBitmapHandle() in GcRender.h, for a more efficient way to draw bitmaps.
/*! ONLY use the winPaint drawing mode with these API.  All other modes are
	deprecated. */
extern void WinPaintBitmap (const BitmapType *bitmapP, Coord x, Coord y);
extern void WinPaintTiledBitmap(const BitmapType* bitmapP, RectangleType* rectP);		

// Characters 
extern void WinDrawChar (wchar32_t theChar, Coord x, Coord y);
extern void WinDrawChars (const char *chars, int16_t len, Coord x, Coord y);
extern void WinPaintChar (wchar32_t theChar, Coord x, Coord y);
extern void WinPaintChars (const char *chars, int16_t len, Coord x, Coord y);
extern void WinDrawInvertedChars (const char *chars, int16_t len, Coord x, Coord y);
extern void WinDrawTruncChars(const char *chars, int16_t len, Coord x, Coord y, Coord maxWidth);
extern void WinPaintTruncChars(const char *chars, int16_t len, Coord x, Coord y, Coord maxWidth);
extern void WinEraseChars (const char *chars, int16_t len, Coord x, Coord y);
extern void WinInvertChars (const char *chars, int16_t len, Coord x, Coord y);
extern UnderlineModeType WinSetUnderlineMode (UnderlineModeType mode);

//-----------------------------------------------
// Routines for patterns and colors                 
//-----------------------------------------------
// #pragma mark Routines for patterns and colors
// #pragma -

extern void WinPushDrawState (void);	// "save" fore, back, text color, pattern, underline mode, font
extern void WinPopDrawState (void);		// "restore" saved drawing variables
extern WinDrawOperation WinSetDrawMode (WinDrawOperation newMode);
extern IndexedColorType WinSetForeColor (IndexedColorType foreColor);
extern IndexedColorType WinSetBackColor (IndexedColorType backColor);
extern IndexedColorType WinSetTextColor (IndexedColorType textColor);

// Direct color versions
extern void WinSetForeColorRGB (const RGBColorType* newRgbP, RGBColorType* prevRgbP);
extern void WinSetBackColorRGB (const RGBColorType* newRgbP, RGBColorType* prevRgbP);
extern void WinSetTextColorRGB (const RGBColorType* newRgbP, RGBColorType* prevRgbP);
extern void WinGetPattern (CustomPatternType *patternP);
extern PatternType WinGetPatternType (void);
extern void WinSetPattern (const CustomPatternType *patternP);
extern void WinSetPatternType (PatternType newPattern);
extern status_t WinPalette(uint8_t operation, int16_t startIndex, 
			 	  			 uint16_t paletteEntries, RGBColorType *tableP);
extern IndexedColorType WinRGBToIndex(const RGBColorType *rgbP);
extern void WinIndexToRGB(IndexedColorType i, RGBColorType *rgbP);

// "obsolete" color call, supported for backwards compatibility
void WinSetColors(	const RGBColorType *newForeColorP, RGBColorType *oldForeColorP,
					const RGBColorType *newBackColorP, RGBColorType *oldBackColorP);

//-----------------------------------------------
// High Density support functions           
//-----------------------------------------------
extern uint16_t WinSetCoordinateSystem(uint16_t coordSys);
extern uint16_t WinGetCoordinateSystem(void);
extern uint32_t WinSetScalingMode(uint32_t mode);
extern uint32_t WinGetScalingMode(void);

// Standard coordinate space conversions.
extern Coord WinScaleCoord(Coord coord, Boolean ceiling);
extern Coord WinScaleCoordNativeToActive(Coord c, Boolean ceiling);
extern Coord WinUnscaleCoord(Coord coord, Boolean ceiling);
extern Coord WinUnscaleCoordActiveToNative(Coord c, Boolean ceiling);
extern void WinScalePoint(PointType* pointP, Boolean ceiling);
//! (post-6.0.1)
extern void WinScalePointNativeToActive(PointType* pointP, Boolean ceiling);
extern void WinUnscalePoint(PointType* pointP, Boolean ceiling);
//! (post-6.0.1)
extern void WinUnscalePointActiveToNative(PointType* pointP, Boolean ceiling);
extern void WinScaleRectangle(RectangleType* rectP);
//! (post-6.0.1)
extern void WinScaleRectangleNativeToActive(RectangleType* rectP);
extern void WinUnscaleRectangle(RectangleType* rectP);
//! (post-6.0.1)
extern void WinUnscaleRectangleActiveToNative(RectangleType* rectP);

//  Arbitrary coordinate space conversions.
extern void WinConvertRectangle(uint16_t currentDensity, uint16_t newDensity, RectangleType *rectP);
extern void WinConvertPoint(uint16_t currentDensity, uint16_t newDensity, PointType *pointP, Boolean ceiling);
extern Coord WinConvertCoord(uint16_t currentDensity, uint16_t newDensity, Coord coord, Boolean ceiling);

extern status_t WinScreenGetAttribute(WinScreenAttrType selector, uint32_t* attrP);
extern status_t WinGetSupportedDensity(uint16_t* densityP);

// Back door into the modern world.
#ifdef __cplusplus
#if _SUPPORTS_NAMESPACE
namespace palmos {
namespace view {
#endif
class IViewManager;
#if _SUPPORTS_NAMESPACE
} }
extern palmos::view::IViewManager* WinGetIViewManager(WinHandle h);
#else
extern IViewManager* WinGetIViewManager(WinHandle h);
#endif
#endif

#ifdef __cplusplus 
}
#endif

#endif //__WINDOW_H__
