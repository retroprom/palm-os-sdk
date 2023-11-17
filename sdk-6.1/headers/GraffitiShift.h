/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: GraffitiShift.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *        This file defines Griffiti shift state indicator routines.
 *
 *****************************************************************************/

#ifndef __GRAFFITISHIFT_H__
#define __GRAFFITISHIFT_H__

#include <PalmTypes.h>

// Graffiti lock flags
#define glfCapsLock     0x0001
#define glfNumLock      0x0002
#define glfForceUpdate  0x8000

#define gsiTempShiftNone                    0
#define gsiTempShiftPunctuation             1
#define gsiTempShiftExtended                2
#define gsiTempShiftUpper                   3
#define gsiTempShiftLower                   4

// Limits to size of Graffiti shift indicator.
#define kMaxGsiWidth    9
#define kMaxGsiHeight   10

enum GsiShiftStateTag
{
	gsiShiftNone,				// no indicator
	gsiNumLock,					// numeric lock
	gsiCapsLock,				// capital lock
	gsiShiftPunctuation,		// punctuation shift
	gsiShiftExtended,			// extented punctuation shift
	gsiShiftUpper,				// alpha upper case shift
	gsiShiftLower	 			// alpha lower case
};
typedef Enum8 GsiShiftState;


#ifdef __cplusplus
extern "C" {
#endif

extern void GsiInitialize (void);

extern void GsiSetLocation (int16_t x, int16_t y);

extern void GsiEnable (Boolean enableIt);

extern Boolean GsiEnabled (void);

extern void GsiSetShiftState (uint16_t lockFlags, uint16_t tempShift);

#ifdef __cplusplus 
}
#endif

#endif //__GRAFFITISHIFT_H__
