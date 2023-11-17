/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AboutBox.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines About Box routines
 *
 *****************************************************************************/

#ifndef _ABOUTBOX_H_
#define _ABOUTBOX_H_

#include <PalmTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// WARNING!!! This routine is for the private use of Palm applications.
// It is released with the public headers so that the sample apps
// released with the SDK can be compiled by developers.
void AbtShowAbout (uint32_t creator);
		
#ifdef __cplusplus
}
#endif

#endif // _ABOUTBOX_H_
