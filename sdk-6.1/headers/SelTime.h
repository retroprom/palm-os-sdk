/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SelTime.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines select time structures and routines.
 *
 *****************************************************************************/

#ifndef	__SELTIME_H__
#define	__SELTIME_H__

#include <PalmTypes.h>
#include <DateTime.h>

//-------------------------------------------------------------------
// structures
//-------------------------------------------------------------------


typedef struct {
	 uint8_t hours;
	 uint8_t minutes;
	 uint8_t seconds;
	 uint8_t reserved;
} HMSTime;

#ifdef __cplusplus
extern "C" {
#endif

Boolean SelectTime (TimeType *startTimeP, TimeType *EndTimeP,
	const Boolean untimed, const char *titleP, int16_t startOfDay, int16_t endOfDay, 
	int16_t startOfDisplay);

Boolean SelectOneTime(int16_t *hour, int16_t *minute, const char *titleP);

#ifdef __cplusplus 
}
#endif

#endif // __SELTIME_H__
