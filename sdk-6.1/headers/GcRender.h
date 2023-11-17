/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: GcRender.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	            	C rendering API.
 *
 *****************************************************************************/

#ifndef _GC_RENDER_H_
#define _GC_RENDER_H_

#include <PalmTypes.h>
#include <Bitmap.h>
#include <Window.h>
#include <GcFont.h>

/*******************************************************************************
 Constants (see GcFont.h  for additional constants)
 *******************************************************************************/

/*! Stroking join styles */
enum GcJoinTag {
	kMiterJoin 	= 0,	/*!< Less segments, but more processing */
	kRoundJoin 	= 1,	/*!< It's better to NOT use kRoundJoin with bezier curves or circles */
	kBevelJoin 	= 2		/*!< Less processing (default) */
};

/*! Stroking cap styles */
enum GcCapTag {
	kButtCap 		= 0,
	kRoundCap		= 1,
	kSquareCap		= 2,
	kArrowCap		= 3
};

/*!	Winding rules */
enum GcWindingRuleTag {
	kEvenWinding = 1,
	kOddWinding,
	kNonZeroWinding,
	kPositiveWinding,
	kNegativeWinding,
	kAbsGEQTwoWinding
};

/*! flags used with GcBeginGroup */
enum GcGroupFlags {
	kModulate = 0x00000001LU,
	kInverse = 0x00000002LU
};

/*! Flags for loading bitmaps */
enum {
	kLoadBitmapUnscaled		= 0x00000001,	/*!< Don't scale to adjust densities */

	kLoadBitmapMask			= 0x00000001	/*!< All the flags we currently know about */
};

/*******************************************************************************
 Typedefs
 *******************************************************************************/

/*! This is a handle on a graphics context. */
struct GcContextType;
typedef struct GcContextType* GcHandle;

/*! This is a handle on a Gc bitmap */
struct GcBitmapType;
typedef struct GcBitmapType* GcBitmapHandle;

/*! A 32-bits color */
typedef struct
{
	uint8_t	red;
	uint8_t	green;
	uint8_t	blue;
	uint8_t	alpha;
} GcColorType;

/*! A gradient */
typedef struct
{
	float		Rdx;		/*!< Change in Red per unit step in the X direction */
	float		Gdx;		/*!< Same for the Green component */
	float		Bdx;		/*!< Same for the Blue component */
	float		Adx;		/*!< Same for the Alpha component */
	float		Rdy;		/*!< Change in Red per unit step in the Y direction */
	float		Gdy;		/*!< Same for the Green component */
	float		Bdy;		/*!< Same for the Blue component */
	float		Ady;		/*!< Same for the Alpha component */
	float		R;			/*!< Initial Red value */
	float		G;			/*!< Same for the Green component */
	float		B;			/*!< Same for the Blue component */
	float		A;			/*!< Same for the Alpha component */
	uint32_t	_reserved;	/*!< Reserved for future use. Always set to 0. */
} GcGradientType;

/*******************************************************************************
 Prototypes
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------------
   Gradients creation
   ----------------------------------------------------------------------------- */

extern	status_t GcInitGradient(	GcGradientType* gradient,
									const GcPointType* points,
									const GcColorType* colors,
									uint32_t num);

/* -----------------------------------------------------------------------------
   Graphic context management
   ----------------------------------------------------------------------------- */

/*!	Create a rendering context in a given bitmap */
extern	GcHandle	GcCreateBitmapContext(const BitmapType* bitmapP);

/*!	Get the current rendering context in the current coordinate system */
extern	GcHandle	GcGetCurrentContext(void);

/*!	Get the current rendering context in native coordinates (1:1) */
extern	GcHandle	GcGetCurrentContextNative(void);

/*!	Release a rendering context */
extern	void		GcReleaseContext(GcHandle ctxt);

/*!	Immediately send any pending drawing commands.  The system may
	not have finished drawing them when you return from this
	function. */
extern	void		GcFlush(GcHandle ctxt);

/*!	Block until all pending drawing commands are completed. */
extern	void		GcCommit(GcHandle ctxt);

/* -----------------------------------------------------------------------------
   Bitmaps
   ----------------------------------------------------------------------------- */

/*! Create a bitmap usable by Gc. win can be null and default to the DisplayWindow.
	The size is not needed if bitmapFileP is a BitmapType, in which case you can
	supply 0.  Otherwise, this is the number of bytes that bitmapFileP points to. */
extern GcBitmapHandle	GcLoadBitmap(WinHandle win, const void* bitmapFileP, size_t size, uint32_t flags);

/*! Retrieve information about a bitmap handle */
extern uint16_t GcGetBitmapDensity(GcBitmapHandle);
extern uint32_t GcGetBitmapWidth(GcBitmapHandle);
extern uint32_t GcGetBitmapHeight(GcBitmapHandle);
extern uint32_t GcGetBitmapDepth(GcBitmapHandle);
extern Boolean GcIsBitmapAlphaOnly(GcBitmapHandle bitmapHandle);
extern Boolean GcBitmapHasAlpha(GcBitmapHandle bitmapHandle);

/*! Convenience function for drawing a GcBitmapHandle using the old
	window manager drawing model.  This is useful to get all of the
	features of GcLoadBitmap (reading PNGs, better performance)
	without having to fully step into the Gc world. */
extern void WinDrawBitmapHandle(GcBitmapHandle, Coord x, Coord y);

/*! Release a bitmap */
extern status_t			GcReleaseBitmap(GcBitmapHandle bitmapHandle);

/* -----------------------------------------------------------------------------
   Path Creation
   ----------------------------------------------------------------------------- */

/*!	GcMoveTo(), GcLineTo(), GcBezierTo() and GcClosePath()
	have direct analogs in PostScript.
*/
extern	void		GcClosePath(GcHandle ctxt);
extern	void		GcMoveTo(GcHandle ctxt, fcoord_t x, fcoord_t y);
extern	void		GcLineTo(GcHandle ctxt, fcoord_t x, fcoord_t y);
extern	void		GcBezierTo(GcHandle ctxt, fcoord_t p1x, fcoord_t p1y, fcoord_t p2x, fcoord_t p2y, fcoord_t p3x, fcoord_t p3y);

/*! Convenience to build an arc */
extern	void		GcArc(GcHandle ctxt, fcoord_t centerX, fcoord_t centerY, fcoord_t radx, fcoord_t rady, float startAngle, float arcLen, Boolean connected);

//! Append an arc to the current path.  
/*! GcArcTo behaves like PostScript \c 'arct'; that is, it appends an
	arc of a circle to the current path, possibly preceded by a straight
	line segment.  The two points given determine the tangent lines of
	the beginning and end of the arc: the line from the current point to
	\c (p1x, p1y) and from \c (p1x,p1y) to (p2x, p2y). 
	
	If the special value \c -1 is given for \a radius, the arc's radius
	will be automatically computed such that no straight line segment
	will be generated and the arc will sweep from the current pen
	position to \c (p2x, p2y).  The tangent line intersection point, \c
	(p1x, p1y) must still be specified, however, and futhermore must be
	equidistant from the current pen point and \c (p2x, p2y).

    /sa GcArc() /sa LineTo() 
 */
extern	void		GcArcTo(GcHandle ctxt, fcoord_t p1x, fcoord_t p1y, fcoord_t p2x, fcoord_t p2y, float radius);

/*! Convenience to build rectangles */
extern	void		GcRect(GcHandle ctxt, fcoord_t left, fcoord_t top, fcoord_t right, fcoord_t bottom);
extern	void		GcRectI(GcHandle ctxt, int32_t left, int32_t top, int32_t right, int32_t bottom);

/*!	Convenience to build rounded rectangles */
extern	void		GcRoundRect(GcHandle ctxt, 	fcoord_t left, fcoord_t top, fcoord_t right, fcoord_t bottom, fcoord_t radx, fcoord_t rady);

/*!	Stroke() is similar to PostScript's "strokepath" function.
	Generated paths are not suited to be stroked again.
*/
extern	void		GcStroke(GcHandle ctxt);

/* -----------------------------------------------------------------------------
   Path filling
   ----------------------------------------------------------------------------- */

/*!	The path is cleared after these calls.
	All of these functions define a color for the entire world
	(GcPaintBitmap does this by tiling its bitmap.)  The pixels that
	are actually touched is constrained by the path and the current
	clipping region.
	GcFill() fills the current path in white. It is mostly used to create clipping region.
	GcPaint() fills the current path with the color set with GcSetColor()
	GcPaintBitmap() fills the current path with the given bitmap.
*/
extern	void		GcFill(GcHandle ctxt);
extern	void		GcPaint(GcHandle ctxt);
extern	void 		GcPaintBitmap(GcHandle ctxt, GcBitmapHandle bitmapHandle);
extern	void		GcPaintRawBitmap(GcHandle ctxt, const BitmapType* bitmapHandle, uint32_t flags);

/*! These are optimizations for drawing non-tiled bitmaps.  They produce the same
	result as the following code:
		GcPushState(...)
			GcTranslate(... ,x-srcRect->left,y-srcRect->top)
			GcRect(... ,0,0,srcRect->right-srcRect->left,srcRect->bottom-srcRect->top)
			GcPaintBitmap( ... )
		GcPopState(...)

	You can supply a NULL srcRect, in which case the rectangle will be the bitmap's
	entire bounds.  Note that srcRect MUST be within the actual bounds of the
	rectangle -- the bitmap will NOT tile if you specify a larger rectangle.
*/
extern	void		GcDrawBitmapAt(		GcHandle ctxt, GcBitmapHandle bitmapHandle,
										const FAbsRectType* srcRect, fcoord_t x, fcoord_t y);
extern	void		GcDrawRawBitmapAt(	GcHandle ctxt, const BitmapType* bitmapP,
										const FAbsRectType* srcRect, fcoord_t x, fcoord_t y,
										uint32_t load_flags);

/* -----------------------------------------------------------------------------
   Clipping
   ----------------------------------------------------------------------------- */

/*!	Start a clipping context.  Clipping regions are created by
	filling a path with GcPaint() between calls to GcBeginClip and GcEndClip.
	It is not allowed to nest clipping contexts, but you can nest clipping
	regions.  That is, this is okay (and produces an intersection of the
	clipping regions):
		GcBeginClip(ctxt);
		...
		GcEndClip(ctxt);
		...
		GcBeginClip(ctxt);
		...
		GcEndClip(ctxt);

	But this is not valid:
		GcBeginClip(ctxt);
		...
		GcBeginClip(ctxt);
		...
		GcEndClip(ctxt);
		...
		GcEndClip(ctxt);
*/
extern	void		GcBeginClip(GcHandle ctxt, Boolean inverse);
extern	void		GcEndClip(GcHandle ctxt);


/* -----------------------------------------------------------------------------
   Groups and Modulation
   ----------------------------------------------------------------------------- */

/*!	Create a group	
	A modulation group is created with GcBeginGroup(ctxt, kModulate), once the group
	is closed (GcEndGroup) it will modulate everything drawn from now on; until GcPopState()
	is called.
	
	To Modulate to the inverse of a group you can use GcBeginGroup(ctxt, kModulate|kInverse).
	
	Modulation is a per-pixel operation that multiply each color component by the corresponding
	color component in the modulation group. This is equivalent to OpenGL's MODULATE texture
	environnement mode.
	
	This now replace GcBeginClip/GcEndClip; GcBeginClip is wrapper
	for GcBeginGroup(ctxt, kModulate) and then drawing everything in *white* (you can
	use GcFill() for that purpose).
*/

extern	void		GcBeginGroup(GcHandle ctxt, uint32_t flags);
extern	void		GcEndGroup(GcHandle ctxt);

/* -----------------------------------------------------------------------------
   Text
   ----------------------------------------------------------------------------- */

/*! Set the font used by GcDrawTextAt() */
extern	void		GcSetFont(GcHandle ctxt, GcFontHandle font);

/*!	An extension of the path-building interface, TextAt() is
	equivalent to "charpath" in PostScript.
	TextAt() implicitly fills a modulation group with the current
	color, generates a path from the supplied text and fills it.
	The path is cleared after this call.
*/
extern	void		GcDrawTextAt(GcHandle ctxt, fcoord_t x, fcoord_t y, const char *text, int32_t length);

/* -----------------------------------------------------------------------------
   State
   ----------------------------------------------------------------------------- */

/*! Rendering quality and Antialiasing */
extern	void		GcSetAntialiasing(GcHandle ctxt, uint32_t value);	// use GcAliasingTag enum
extern	void		GcSetDithering(GcHandle ctxt, uint32_t value);		// use kGcEnable or kGcDisable

/*!	The winding rule determines which pixels are "inside" and "outside" of
	a polygon.  Use one of the GcWindingRuleTag values above. */
extern	void		GcSetWindingRule(GcHandle ctxt, int32_t rule);

/*! These settings are effective at the time of a Stroke() call */
extern	void		GcSetPenSize(GcHandle ctxt, fcoord_t penWidth);
extern	void		GcSetCaps(GcHandle ctxt, int32_t start_cap, int32_t end_cap);
extern	void		GcSetJoin(GcHandle ctxt, int32_t join, float mitter_limit);

/*!	Sets the color used with Paint() */
extern	void		GcSetColor(GcHandle ctxt, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/*!	Sets the gradient used with Paint() */
extern	void		GcSetGradient(GcHandle ctxt, const GcGradientType* gradient);

/*!	Canvas transformations */
extern	void 		GcTranslate(GcHandle ctxt, fcoord_t x, fcoord_t y);
extern	void 		GcScale(GcHandle ctxt, fcoord_t sx, fcoord_t sy);
extern	void 		GcShear(GcHandle ctxt, fcoord_t x, fcoord_t y);
extern	void 		GcRotate(GcHandle ctxt, fcoord_t rad);
extern	void 		GcReflect(GcHandle ctxt, fcoord_t rad);

/*! Apply arbitrary transformation, given array of 6 matrix values as shown. */
enum {
		kGcXX = 0,		kGcXY = 2,		kGcXT = 4,
		kGcYX = 1,		kGcYY = 3,		kGcYT = 5
	/*	0,				0,				1		*/
};
extern	void		GcTransform(GcHandle ctxt, const fcoord_t* matrix);

/* Coordinate system */
/* Valid values are kCoordinatesNative, kCoordinatesStandard, kCoordinatesOneAndAHalf,
 * kCoordinatesDouble, kCoordinatesTriple, kCoordinatesQuadruple
 */
extern void			GcSetCoordinateSystem(GcHandle ctxt, uint32_t coordinateSystem);

/*!	Save/restore state */
extern	void		GcPushState(GcHandle ctxt);
extern	void		GcPopState(GcHandle ctxt);


#ifdef __cplusplus 
}
#endif

#endif // _GC_RENDER_H_
