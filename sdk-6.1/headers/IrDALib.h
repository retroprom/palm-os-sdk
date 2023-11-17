/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IrDALib.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef IRLIB_H
#define IRLIB_H

#include <IrDA.h>

#ifdef __cplusplus
extern "C" {
#endif


/* IrDA device discovery */

status_t IrDADiscoverDevices(uint32_t *ioNumLogs,
							 IrLmpDeviceInfoType *oLogs,
							 Boolean *oCached);


/* IrDA SuperServer client routines */

status_t IrDARegisterService(int listener,
							 uint32_t type,
							 uint32_t creator,
							 uint16_t rsrcID,
							 uint32_t stackSize,
							 Boolean singleLaunch);
status_t IrDAUnregisterService(uint32_t type,
							   uint32_t creator,
							   uint16_t rsrcID);

// prototype for service addon entry point
typedef void (IrDAServiceAddonCallback)(int client);


#ifdef __cplusplus
}	// extern "C"
#endif

#endif	// IRLIB_H
