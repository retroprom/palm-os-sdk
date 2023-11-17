/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ExgHotSync.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Types and functions for applications that use the HotSync Exchange library
 *
 *****************************************************************************/

#ifndef __HOTSYNC_EXCHANGE_H
#define __HOTSYNC_EXCHANGE_H
#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus


#include <ExgMgr.h>

// HotSync Exchange registered creator ID
#define HotSyncExgCreator			'hsxc'

// Schemes supported by the HotSync Exchange Library
#define exgDesktopScheme			"_desktop"
#define exgDesktopPrefix			(exgDesktopScheme ":")

// Max length of file name: the app specifies this in exgSocket.name
// The same name is used for the desktop filename and hence the limitation
#define kHsExgFileNameMaxLen			(255 + 1)

// Errors
#define	kHsExgErrBadFileName		(hsExgErrorClass | 0x01)

#ifdef __cplusplus
}	// extern "C"
#endif	// __cplusplus
#endif  // __HOTSYNC_EXCHANGE_H
