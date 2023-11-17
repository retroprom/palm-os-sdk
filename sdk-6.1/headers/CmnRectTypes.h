/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CmnRectTypes.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  Rectangle structures shared by DAL and Palm OS.
 *
 *****************************************************************************/

#ifndef _CMNRECTTYPES_H_
#define _CMNRECTTYPES_H_

#include <PalmTypes.h>

typedef struct AbsRectType {
  Coord left;
  Coord top;
  Coord right;
  Coord bottom;
} AbsRectType;


typedef struct PointType {
  Coord x;
  Coord y;
} PointType;


typedef struct RectangleType {
  PointType  topLeft;
  PointType  extent;
} RectangleType;

typedef RectangleType *RectanglePtr;

// Conversion macros

#define AbsToRect(a,r) 									\
(																\
	(r)->topLeft.x = (a)->left ,						\
	(r)->topLeft.y = (a)->top ,						\
	(r)->extent.x = (a)->right - (a)->left ,		\
	(r)->extent.y = (a)->bottom - (a)->top			\
)

#define RectToAbs(r,a) 									\
(																\
	(a)->left = (r)->topLeft.x ,						\
	(a)->top = (r)->topLeft.y ,						\
	(a)->right = (r)->topLeft.x + (r)->extent.x,	\
	(a)->bottom = (r)->topLeft.y + (r)->extent.y	\
)


#endif //_CMNRECTTYPES_H_
