/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TimeMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Time manager functions
 *
 *****************************************************************************/

#ifndef __TIMEMGR_H__
#define __TIMEMGR_H__


// Include elementary types
#include <PalmTypes.h>
#include <CmnErrors.h>


/************************************************************
 * Time Manager result codes
 * (timErrorClass is defined in SystemMgr.h)
 *************************************************************/
#define timErrMemory					(timErrorClass | 1)
#define timErrNoLunarCalendarSupport	(timErrorClass | 2)
#define timErrBadParam					(timErrorClass | 3)


/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------------------
// Initialization
//-------------------------------------------------------------------

// seconds since 1/1/1904
uint32_t		TimGetSeconds(void);
					
// seconds since 1/1/1904
status_t		TimSetSeconds(uint32_t seconds);


//-------------------------------------------------------------------
// Backwards compatibility APIs
//-------------------------------------------------------------------

uint64_t		TimGetTicks(void);

#ifdef __cplusplus 
}
#endif

#endif // __TIMEMGR_H__
