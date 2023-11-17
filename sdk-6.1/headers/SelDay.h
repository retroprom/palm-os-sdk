/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SelDay.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the date picker month object's  structures 
 *   and routines.
 *
 *****************************************************************************/

#ifndef	__SELDAY_H__
#define	__SELDAY_H__

#include <PalmTypes.h>

#include <Day.h>

#define daySelectorMinYear  firstYear
#define daySelectorMaxYear  lastYear

#ifdef __cplusplus
extern "C" {
#endif

Boolean SelectDay (const SelectDayType selectDayBy, int16_t *month, 
	int16_t *day, int16_t *year, const char *title);

#ifdef __cplusplus 
}
#endif

#endif //__SELDAY_H__
