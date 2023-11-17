/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateLunar.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This file declares the (temporary) lunar calendar support for the
 *	Datebook application.  These declarations will be part of Time Manager 6.0.
 *
 *****************************************************************************/

#ifndef __DATELUNAR_H__
#define __DATELUNAR_H__

#include <PalmTypes.h>
#include <CmnErrors.h>

#include <Bitmap.h>


#define timErrNoLunarCalendarSupport		(timErrorClass | 2)

// ************************************************************
// * Date Time Constants
// ************************************************************

// Chinese lunar years, months, and days are 1-based

#define timFirstChineseLunarYear		1
#define timLastChineseLunarYear			60	// Sexigesimal (60-year cycle)
#define timNumChineseStems				10	// Celestial stems in sexigesmial cycle
#define timNumChineseBranches			12	// Terrestrial branches in sexigesmial cycle
#define timFirstChineseLunarMonth		1
#define timMaxChineseLunarMonths		13	// Can be either 12 or 13
#define timFirstChineseLunarDay			1
#define timNewMoonChineseLunarDay		timFirstChineseLunarDay
#define timFullMoonChineseLunarDay		15

// Pass these to DateToLunarStr to omit portions of output

#define timOmitLunarDay					0
#define timOmitLunarMonth				0
#define timOmitLunarYear				0

// 17 is the longest possible Chinese date string, but note that this is longer
// than longDateStrLength (15).  Note also that the latter, (unlike our constants)
// includes space for the chrNull.

#define timMaxLunarDateStrLen			31
#define kMaxLunarDayStrSize				8


//************************************************************
//* Date Time procedures
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif

Boolean DateSupportsLunarCalendar(void);

status_t DateSecondsToLunarDate(
	DmOpenRef 		resourceDbP,
	uint32_t		iSeconds,
	uint16_t*		oLunarYear,
	uint16_t*		oLunarMonth,
	uint16_t*		oLunarDay,
	Boolean*		oIsLeapMonth);

status_t DateToLunarStr(
	DmOpenRef 		resourceDbP,
	uint16_t		iLunarYear,
	uint16_t		iLunarMonth,
	uint16_t		iLunarDay,
	Boolean			iMonthIsRepeat,
	char*			oLunarDateStr,
	uint16_t		iMaxStrLen);

Boolean SelectLunarDayIsSupported(DmOpenRef resourceDbP);

BitmapType*  LunarCalendarGetChineseLargeMonthBitmap(uint16_t number);

void TerminateDateLunar(DmOpenRef resourceDbP);

void LunarCalendarInit(DmOpenRef resourceDbP);

void LunarCalendarClose(DmOpenRef resourceDbP);

#ifdef __cplusplus 
}
#endif


#endif
