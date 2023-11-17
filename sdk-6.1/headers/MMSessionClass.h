/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. and Sony Inc.  All rights reserved.
 *
 * File: MMSessionClass.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _MM_SESSION_CLASS_H
#define _MM_SESSION_CLASS_H

#include <MMDefs.h>
#include <MMFormat.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	--------------------------------------------------------------------------------
	standard session classes
*/

enum
{
	// Standard session classes defined by PalmSource.
	// They will be published for 3rd party use & implementation.
	P_MM_STANDARD_SESSION_CLASS_BASE =	0x10010000L,

	// These session classes are implementation-defined.  Developers
	// can use session classes within this base however they see fit.
	// They should be published by each developer for public use.
	P_MM_USER_SESSION_CLASS_BASE =		0x10020000L,

	// Session classes that are private to the particular developer.
	P_MM_PRIVATE_SESSION_CLASS_BASE =	0x10030000L
};

#define P_MM_SESSION_CLASS_DEFAULT_PLAYBACK	(P_MM_STANDARD_SESSION_CLASS_BASE | 0x0001L)
#define P_MM_SESSION_CLASS_DEFAULT_CAPTURE	(P_MM_STANDARD_SESSION_CLASS_BASE | 0x0002L)

/*	--------------------------------------------------------------------------------
	SessionClass Info Selectors
*/

// string: user-readable session class name
#define P_MM_SESSION_CLASS_PROP_NAME		(P_MM_SESSION_CLASS_PROP_BASE | 0x0001L)
// string
#define P_MM_SESSION_CLASS_PROP_VERSION		(P_MM_SESSION_CLASS_PROP_BASE | 0x0002L)
// string
#define P_MM_SESSION_CLASS_PROP_CREATOR		(P_MM_SESSION_CLASS_PROP_BASE | 0x0003L)

/*	--------------------------------------------------------------------------------
    MMSessionClass functions
*/

/*	MMSessionClassEnumerate: iterate through the available session types.
	IN
		*ioIterate: value returned by previous call, or P_MM_ENUM_BEGIN for the first call
	OUT
		*outSessionClassID: ID of the next available codec
	ERRORS
		sysErrBadIndex: invalid iterator or no more session types to list
*/
extern status_t MMSessionClassEnumerate(int32_t * ioIterator, MMSessionClassID * outSessionClassID);

#ifdef __cplusplus
}
#endif
#endif /* _MM_SESSION_CLASS_H */

