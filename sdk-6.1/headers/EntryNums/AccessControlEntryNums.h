/******************************************************************************
 *
 *	Copyright (c) 2004 PalmSource, Inc. or its subsidiaries.
 *	All rights reserved.
 *
 *	File: AccessControlEntryNums.h
 *
 *	Description:
 *		This file is automatically generated by the
 *		PalmSource Shared Library Generator.
 *
 *	History:
 *		Generated on: Tue Aug 31 14:16:01 2004
 *****************************************************************************/
#ifndef __ACCESSCONTROLENTRYNUMS_H__	// Avoid multiple inclusion
#define __ACCESSCONTROLENTRYNUMS_H__

// System header file
#include <PalmTypes.h>

enum accesscontrolEntryNumTag {
	accesscontrolEntryNumAmInitializeUIContextEnum = 0,
	accesscontrolEntryNumAmReleaseUIContextEnum,
	accesscontrolEntryNumAmMemHandleNewEnum,
	accesscontrolEntryNumAmMemHandleFreeEnum,
	accesscontrolEntryNumAmMemHandleLockEnum,
	accesscontrolEntryNumAmMemHandleUnlockEnum,
};

#define accesscontrolEntryNumAmInitializeUIContext	((uint32_t)accesscontrolEntryNumAmInitializeUIContextEnum)
#define accesscontrolEntryNumAmReleaseUIContext   	((uint32_t)accesscontrolEntryNumAmReleaseUIContextEnum)
#define accesscontrolEntryNumAmMemHandleNew       	((uint32_t)accesscontrolEntryNumAmMemHandleNewEnum)
#define accesscontrolEntryNumAmMemHandleFree      	((uint32_t)accesscontrolEntryNumAmMemHandleFreeEnum)
#define accesscontrolEntryNumAmMemHandleLock      	((uint32_t)accesscontrolEntryNumAmMemHandleLockEnum)
#define accesscontrolEntryNumAmMemHandleUnlock    	((uint32_t)accesscontrolEntryNumAmMemHandleUnlockEnum)
#endif // __ACCESSCONTROLENTRYNUMS_H__
