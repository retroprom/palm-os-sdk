/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AmPluginCodePrint.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _AM_PLUGIN_CODE_PRINT_H_
#define _AM_PLUGIN_CODE_PRINT_H_

#include <PalmTypes.h>

typedef struct {
	uint32_t	type;		// PRC Type
	uint32_t	creator;	// PRC Creator

	char		name[32];	// DB Name
} AmPluginCodePrintExtInfoType;

#endif /* _AM_PLUGIN_CODE_PRINT_H_ */
