/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SysEvtMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header for the System Event Manager
 *
 *****************************************************************************/

#ifndef __SYSEVTMGR_H__
#define __SYSEVTMGR_H__

#include <CmnErrors.h>
#include <PalmTypes.h>
#include <SysEventCompatibility.h>
#include <CmnErrors.h>

/************************************************************
 * Commands for EvtSetAutoOffTimer()
 *************************************************************/
enum EvtSetAutoOffTag
{
	SetAtLeast,		// turn off in at least xxx seconds
	SetExactly,		// turn off in xxx seconds
	SetAtMost,		// turn off in at most xxx seconds
	SetDefault,		// change default auto-off timeout to xxx seconds
	ResetTimer		// reset the timer to the default auto-off timeout
	
} ;
typedef Enum8 EvtSetAutoOffCmd ;


/************************************************************
 * System Event Manager procedures
 *************************************************************/

#ifdef __cplusplus
extern "C" {  
#endif

//-----------------------------------------------------------------
// General Utilities
//------------------------------------------------------------------
// Reset the auto-off timer. This is called by the SerialLink Manager in order
//  so we don't auto-off while receiving data over the serial port.
status_t		EvtResetAutoOffTimer(void);

status_t		EvtSetAutoOffTimer(EvtSetAutoOffCmd cmd, uint16_t timeout);

// Set Graffiti enabled or disabled.
void	EvtEnableGraffiti(Boolean enable);
						
#ifdef __cplusplus
}
#endif

#endif //__SYSEVTMGR_H__
