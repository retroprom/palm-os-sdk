/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: WindowCompatibility.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *        Deprecated window APIs.
 *
 *****************************************************************************/

#ifndef __WINDOW_COMPATIBILITY_H__
#define __WINDOW_COMPATIBILITY_H__

#include <Window.h>

#ifdef __cplusplus
extern "C" {
#endif

// This was part of the old PINS API.  It is no longer used,
// and does nothing.
extern status_t	WinSetConstraintsSizeV50(	WinHandle winHandle,
	Coord minH, Coord prefH, Coord maxH,
	Coord minV, Coord prefV, Coord maxV);

#ifdef __cplusplus 
}
#endif

#endif //__WINDOW_COMPATIBILITY_H__
