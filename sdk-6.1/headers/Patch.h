/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Patch.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Declarations for building patches.
 *
 *****************************************************************************/

#ifndef __PATCH_H__
#define __PATCH_H__

#include <PalmTypes.h>

// Type of each element of the entry list array following SysPatchTargetHeaderType
typedef uint32_t SysPatchEntryNumType;

typedef struct SysPatchTargetHeaderType {
   uint32_t    type;         // type of the shared library to patch
   uint32_t    creator;      // creator ID of the shared library to patch
   uint16_t    rsrcID;       // resource ID of the shared library to patch
   uint16_t    flags;        // bits indicating if head-patching or tail-patching is required
   uint32_t    numEntries;   // total number of entries listed in following entryNums array
} SysPatchTargetHeaderType;

// Flag bits
#define patchFlagHead            ((uint16_t)0x0001)
#define patchFlagTail            ((uint16_t)0x0002)
#define patchFlagReservedMask    ((uint16_t)0xfffc)

#define patchIndexHead   ((uint16_t)0xc000)
#define patchIndexTail      ((uint16_t)0x4000)

#endif  // __PATCH_H__
