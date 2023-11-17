/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TextServicesMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Header file for Text Services Manager. This provides the caller with
 *		an API for interacting with various text services, including front-end
 *		processors (FEPs), which are sometimes known as input methods.
 *
 *****************************************************************************/

#ifndef __TEXTSERVICESMGR_H__
#define __TEXTSERVICESMGR_H__

#include <PalmTypes.h>
#include <SystemResources.h>

/***********************************************************************
 * Public constants
 ***********************************************************************/

// Feature Creators and numbers, for use with the FtrGet() call.
#define	tsmFtrCreator				sysFileCTextServices

// Selector used with call to FtrGet(tsmFtrCreator, xxx) to get the
// Text Services Manager flags.
#define	tsmFtrNumFlags				0

// Flags returned by FtrGet(tsmFtrCreator, tsmFtrNumFlags) call.
#define	tsmFtrFlagsHasFep			0x00000001L		// Bit set if FEP is installed.

// Input mode - used with TsmGet/SetFepMode.
typedef uint16_t TsmFepModeType;

#define tsmFepModeDefault	((TsmFepModeType)0)
#define tsmFepModeOff		((TsmFepModeType)1)
#define tsmFepModeCustom	((TsmFepModeType)128)


/***********************************************************************
 * Public types
 ***********************************************************************/

/***********************************************************************
 * Public routines
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 * Text Manager functions for calling through to FEP Shared Library routines.
 * These functions determine whether the called library is a 68K
 * lib or a native lib and calls the library function appropriately.
 ***********************************************************************/

// Return the current mode for the active FEP.
TsmFepModeType TsmGetFepMode(void);

// Set the mode for the active FEP to be <inNewMode>. The previous mode
// is returned.
TsmFepModeType TsmSetFepMode(TsmFepModeType inNewMode);


#ifdef __cplusplus 
}
#endif

#endif  // __TEXTSERVICESMGR_H__
