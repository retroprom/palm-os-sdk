/******************************************************************************
 *
 *	Copyright (c) 2004 PalmSource, Inc. or its subsidiaries.
 *	All rights reserved.
 *
 *	File: SyncMgrLibEntryNums.h
 *
 *	Description:
 *		This file is automatically generated by the
 *		PalmSource Shared Library Generator.
 *
 *	History:
 *		Generated on: Tue Aug 31 14:15:34 2004
 *****************************************************************************/
#ifndef __SYNCMGRLIBENTRYNUMS_H__	// Avoid multiple inclusion
#define __SYNCMGRLIBENTRYNUMS_H__

// System header file
#include <PalmTypes.h>

enum syncmgrlibEntryNumTag {
	syncmgrlibEntryNumSyncAddSynchronizerEnum = 0,
	syncmgrlibEntryNumSyncSessionGetAccessEnum,
	syncmgrlibEntryNumSyncSessionReleaseAccessEnum,
	syncmgrlibEntryNumSyncTokenAcquireEnum,
	syncmgrlibEntryNumSyncTokenReleaseEnum,
	syncmgrlibEntryNumSyncTokenIsAvailableEnum,
};

#define syncmgrlibEntryNumSyncAddSynchronizer     	((uint32_t)syncmgrlibEntryNumSyncAddSynchronizerEnum)
#define syncmgrlibEntryNumSyncSessionGetAccess    	((uint32_t)syncmgrlibEntryNumSyncSessionGetAccessEnum)
#define syncmgrlibEntryNumSyncSessionReleaseAccess	((uint32_t)syncmgrlibEntryNumSyncSessionReleaseAccessEnum)
#define syncmgrlibEntryNumSyncTokenAcquire        	((uint32_t)syncmgrlibEntryNumSyncTokenAcquireEnum)
#define syncmgrlibEntryNumSyncTokenRelease        	((uint32_t)syncmgrlibEntryNumSyncTokenReleaseEnum)
#define syncmgrlibEntryNumSyncTokenIsAvailable    	((uint32_t)syncmgrlibEntryNumSyncTokenIsAvailableEnum)
#endif // __SYNCMGRLIBENTRYNUMS_H__
