/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Rect.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines rectangle structures and routines.
 *
 *****************************************************************************/

#ifndef __RECT_H__
#define __RECT_H__

// Include elementary types
#include <PalmTypes.h>					// Basic types

// Type definitions used by PalmOS and the DAL
#include <CmnRectTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void RctInsetRectangle (RectangleType *rP, Coord insetAmt);

extern Boolean RctPtInRectangle (Coord x, Coord y, const RectangleType *rP);

extern void RctGetIntersection (const RectangleType *r1P, const RectangleType *r2P, 
	RectangleType *r3P);


#ifdef __cplusplus
}
#endif

#define RctSetRectangle(rP,l,t,w,h)		((rP)->topLeft.x = (l), (rP)->topLeft.y = (t), (rP)->extent.x = (w), (rP)->extent.y = (h))
#define RctCopyRectangle(s,d)			(*(d)=*(s))
#define RctOffsetRectangle(rP,dx,dy)	((rP)->topLeft.x += (dx), (rP)->topLeft.y += (dy))


#endif //__RECT_H__
