/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IOSInstall.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Public Header file (for User) containing Install library functions declerations.
 *
 *****************************************************************************/

#ifndef __IOSInstall_H__
#define	__IOSInstall_H__

#include "PalmTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

status_t IOSInstallDriver(uint32_t iType, uint32_t iCreator, uint32_t iRsrcID);
status_t IOSRemoveDriver(uint32_t iType, uint32_t iCreator, uint32_t iRsrcID);

#ifdef __cplusplus
}
#endif

#endif 	// __IOSInstall_H__
