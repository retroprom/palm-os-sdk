/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Loader.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Declaration of Program Loader external API.
 *
 *****************************************************************************/

#ifndef __LOADER_H__
#define __LOADER_H__

#include <PalmTypes.h>
#include <DataMgr.h>

/************************************************************
 * Program Loader constants
 ************************************************************/

#define sysEntryNumMain             ((uint32_t)0xffffffff)

// Flag bits in the "flags" parameter of SysLoadModule
#define sysDoNotVerifySignature ((uint32_t)0x00000001)  // If set, signed code verification will be omitted

/************************************************************
 * Program Loader data structures
 ************************************************************/
typedef uint32_t (*SysMainEntryPtrType)(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags);

typedef struct SysPatchInfoType {
   uint32_t  refNum;             // refNum identifying the patched shared library
   uint32_t  type;
   uint32_t  creator;
   uint16_t  rsrcID;
   uint16_t  reserved;           // padding
   uint32_t  index;              // where this patch is located in the call chain
   status_t  (*sysGetNextPatchP)(uint32_t, uint32_t, uint32_t, void **);
} SysPatchInfoType;

typedef struct SysModuleInfoType {
   uint32_t	revision;	// Revision number
   uint32_t	entries;	// Total number of exported entry points
   uint32_t	dataSize;	// Size of data segment
	uint32_t	minArch; 	// Required minimum architecture number
	uint32_t	minOS;		// Required minimum OS version
	uint32_t	currArch;	// Current processor architecture this device is running
	uint32_t	currOS;		// Current OS version this device is running
} SysModuleInfoType;

/************************************************************
 * Program Loader functions
 ************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

status_t SysLoadModule(uint32_t dbType, uint32_t dbCreator, uint16_t rsrcID, uint32_t flags, uint32_t *refNumP);

status_t SysLoadModuleByDatabaseID(DatabaseID dbID, uint16_t rsrcID, uint32_t flags, uint32_t *refNumP);

status_t SysUnloadModule(uint32_t refNum);

status_t SysGetEntryAddresses(uint32_t refNum, uint32_t startEntryNum, uint32_t numEntries, void **addressP);

uint32_t SysGetRefNum(void);

status_t SysGetModuleDatabase(uint32_t refNum, DatabaseID *dbIDP, DmOpenRef *openRefP);

status_t SysRegisterPatch(uint32_t type, uint32_t creator, uint16_t rsrcID);

status_t SysUnregisterPatch(uint32_t type, uint32_t creator, uint16_t rsrcID);

status_t SysGetModuleGlobals(uint32_t refNum, Boolean wantStructure, void **globalsP);

status_t SysGetModuleInfo(uint32_t dbType, uint32_t dbCreator, uint16_t rsrcID, SysModuleInfoType *infoP);

status_t SysGetModuleInfoByDatabaseID(DatabaseID dbID, uint16_t rsrcID, SysModuleInfoType *infoP);

#ifdef __cplusplus
}
#endif

#endif  // __LOADER_H__
