/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ScrollBar.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines scroll bar routines.
 *
 *****************************************************************************/

#ifndef __SCROLLBAR_H__
#define __SCROLLBAR_H__

#include <PalmTypes.h>

#include <Event.h>

typedef struct ScrollBarType ScrollBarType;
typedef ScrollBarType *ScrollBarPtr;

#ifdef __cplusplus
extern "C" {
#endif

void 	SclGetScrollBar (const ScrollBarPtr bar, int32_t *valueP, 
	int32_t *minP, int32_t *maxP, int32_t *pageSizeP);

void	SclSetScrollBar (const ScrollBarPtr bar, int32_t value, 
						const int32_t min, const int32_t max, const int32_t pageSize);

void	SclDrawScrollBar (const ScrollBarPtr bar);

Boolean	SclHandleEvent (const ScrollBarPtr bar, const EventType *event);

#ifdef __cplusplus 
}
#endif


#endif //__SCROLLBAR_H__
