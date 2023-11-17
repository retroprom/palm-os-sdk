/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SelTimeZone.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines select time zone structures and routines.
 *
 *****************************************************************************/

#ifndef	__SELTIMEZONE_H__
#define	__SELTIMEZONE_H__

#include <PalmTypes.h>

#include <DateTime.h>
#include <LocaleMgr.h>		// LmLocaleType

#ifdef __cplusplus
extern "C" {
#endif

enum SelectTimeZoneDisplayTag
{
	showNoTime = 0,
	showCurrentAndNewTimeZone,
	showDeviceTimeZone	
};

typedef Enum8 SelectTimeZoneDisplayType ;

/*
 * New API, please use.
 */
Boolean SelectTimeZone(
	char*						ioZoneIDP,
	const char*					titleP, 
	SelectTimeZoneDisplayType	displayOption);

/*
 * Deprecated API, please do not use
 */
Boolean SelectTimeZoneV50(
	int16_t*		ioTimeZoneP, 
	LmLocaleType*	ioLocaleInTimeZoneP,
	const char*		titleP, 
	Boolean			showTimes, 
	Boolean			anyLocale);

#ifdef __cplusplus 
}
#endif

#endif // __SELTIMEZONE_H__
