/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: CmnBatteryTypes.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Battery related types shared by HAL and Palm OS.
 *
 *****************************************************************************/

#ifndef _CMNBATTERYTYPES_H_
#define _CMNBATTERYTYPES_H_

#include <PalmTypes.h>

// Types of batteries installed.
enum SysBatteryKindTag {
	sysBatteryKindAlkaline=0,
	sysBatteryKindNiCad,
	sysBatteryKindLiIon,
	sysBatteryKindRechAlk,
	sysBatteryKindNiMH,
	sysBatteryKindLiIon1400,
	sysBatteryKindFuelCell,
	sysBatteryKindPlutonium237,
	sysBatteryKindAntiMatter,
	sysBatteryKindLast=0xFF   // insert new battery types BEFORE this one
};

#ifndef SysBatteryKind_defined
typedef Enum8 SysBatteryKind;
#define SysBatteryKind_defined 1
#endif

// Different battery states
enum SysBatteryStateTag {
	sysBatteryStateNormal=0,
	sysBatteryStateLowBattery,
	sysBatteryStateCritBattery,
	sysBatteryStateShutdown
};

typedef Enum8 SysBatteryState;

#endif // _CMNBATTERYTYPES_H_
