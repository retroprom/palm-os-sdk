/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: FatalAlert.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *        This file defines the system Fatal Alert support.
 *
 *****************************************************************************/

#ifndef __FATALALERT_H__
#define __FATALALERT_H__

#include <PalmTypes.h>


// Value returned by SysFatalAlert
#define fatalReset				0
#define fatalEnterDebugger		1
#define fatalEnter68KDebugger	2
#define fatalEnterBothDebugger	3
#define fatalDoNothing		0xFFFFU

#ifdef __cplusplus
extern "C" {
#endif

// Flags are as per CmnErrors.h (as used in ErrorMgr.h)
uint16_t SysFatalAlert(const char* msg, uint32_t flags);

uint16_t SysFatalAlertV60(const char* msg);
void SysFatalAlertInit(void);

#ifdef __cplusplus 
}
#endif

#endif  // __FATALALERT_H__
