/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: KeyMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Include file for Key manager
 *
 *****************************************************************************/

#ifndef __KEYMGR_H__
#define __KEYMGR_H__

// Pilot common definitions
#include <PalmTypes.h>
//#include <CmnKeyTypes.h>

/********************************************************************
 * Key manager Routines
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

uint32_t KeyCurrentState( void );
uint32_t KeySetMask( uint32_t keyMask );
status_t KeyRates( Boolean set, uint16_t *initDelayP, uint16_t *periodP,
			  uint16_t *doubleTapDelayP, Boolean *queueAheadP );


#ifdef __cplusplus
}
#endif

	
#endif	//__KEYMGR_H__
