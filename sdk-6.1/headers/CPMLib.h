/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CPMLib.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef CPMLIB_H_
#define	CPMLIB_H_

#include <PalmTypes.h>

#if TARGET_PLATFORM == TARGET_PLATFORM_DEVICE_68K
#	include "CPMLib68KInterface.h"
#else
#	include "CPMLibARMInterface.h"
#endif 

#endif /* CPMLIB_H_ */
