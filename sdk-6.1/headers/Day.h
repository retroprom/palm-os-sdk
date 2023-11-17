/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Day.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the date picker month object's routines.
 *
 *****************************************************************************/

#ifndef	_DAY_H_
#define	_DAY_H_

#include <PalmTypes.h>
#include <Event.h>

enum SelectDayTag
{
	selectDayByDay,			// return d/m/y
	selectDayByWeek,		// return d/m/y with d as same day of the week
	selectDayByMonth		// return d/m/y with d as same day of the month
} ;
typedef Enum8 SelectDayType;


typedef struct DaySelectorType DaySelectorType;

typedef DaySelectorType *DaySelectorPtr;

#ifdef __cplusplus
extern "C" {
#endif

void	DayDrawDaySelector (const DaySelectorPtr selectorP);

Boolean DayHandleEvent (const DaySelectorPtr selectorP, const EventType *pEvent);

void	DayDrawDays (const DaySelectorPtr selectorP);

#ifdef __cplusplus
}
#endif

#endif //_DAY_H_
