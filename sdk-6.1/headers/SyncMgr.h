/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SyncMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Header file for Sync Manager
 *
 *****************************************************************************/

#ifndef __SYNC_MGR_H__
#define __SYNC_MGR_H__

#include <PalmTypes.h>				// Basic types
#include <CmnErrors.h>

// Security policies enforced by Sync Manager
//		1. Restricts apps that are allowed to register to only ones specified in policy
#define	kSyncPolicyRestrictAppRegistration	'sync'
//		2. Allows specified apps to register non-intrusively (without UI)
#define	kSyncPolicyNonUiAuthentication		'noui'


#define	kSyncProductNameMaxLen			(43 + 1)
#define kSyncAppDescriptionMaxLen		(95 + 1)
// Maximum number of unique Sync apps that can have active sessions
#define	kSyncMaxActiveSessions			(32)

// Sync Manager Errors
// syncMgrErrorClass defined in CmnErrors.h
#define	syncMgrErrOperationNotSupported		(syncMgrErrorClass | 0x01)
#define	syncMgrErrSystemErr					(syncMgrErrorClass | 0x02)
#define	syncMgrErrMemAllocFailure			(syncMgrErrorClass | 0x03)
#define	syncMgrErrAccessDenied				(syncMgrErrorClass | 0x04)
#define	syncMgrErrUserRefusedSyncApp		(syncMgrErrorClass | 0x05)	// User said "No" to registering sync app
#define	syncMgrErrMaxSessionsActive			(syncMgrErrorClass | 0x06)

#ifdef __cplusplus
extern "C" 
{
#endif	// #ifdef __cplusplus

status_t SyncAddSynchronizer (const char *displayNameP, const char *descriptionP);

status_t SyncSessionGetAccess (void);

status_t SyncSessionReleaseAccess (void);

#ifdef __cplusplus
}	// extern "C"
#endif	// #ifdef __cplusplus

#endif	// __SYNC_MGR_H__
