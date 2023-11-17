/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: MemoryMgrCompatibility.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Compatibility layer for for Memory Manager. 
 *
 *		This header should be included only when porting existing 
 *		Palm OS applications to Palm OS Cobalt. If this header is 
 *		included, calls to the APIs contained in this header	should 
 *		follow the prototypes of corresponding APIs in Palm OS 4.x.
 *
 *****************************************************************************/

#ifndef __MEMORYMGRCOMPATIBILY_H__
#define __MEMORYMGRCOMPATIBILY_H__

#include <MemoryMgr.h>

/************************************************************
 * Memory Manager Types
 *************************************************************/

/********************************************************************
 * Memory Manager Routines
 ********************************************************************/
 
#ifdef __cplusplus
extern "C" {
#endif

LocalID     MemPtrToLocalID    ( MemPtr p );
LocalID     MemHandleToLocalID ( MemHandle h );
LocalIDKind MemLocalIDKind     ( LocalID local );

void *		MemLocalIDToGlobal    ( LocalID localId );
void *		MemLocalIDToPtr       ( LocalID localId );
void *		MemLocalIDToLockedPtr ( LocalID localId );

//-------------------------------------------------------------------
// CardNo and Local ID based routines (4.0 style routines)
//-------------------------------------------------------------------

uint16_t		MemNumCardsV40(void);
uint16_t		MemNumHeapsV40(uint16_t cardNo);
uint16_t		MemNumRAMHeapsV40(uint16_t cardNo);
uint16_t		MemHeapIDV40(uint16_t cardNo, uint16_t heapIndex);

uint16_t		MemPtrCardNoV40(void * p);
uint16_t		MemHandleCardNoV40(MemHandle h);

void *		MemLocalIDToGlobalV40(LocalID localId,  uint16_t cardNo);
void *		MemLocalIDToPtrV40(LocalID localId,  uint16_t cardNo);
void *		MemLocalIDToLockedPtrV40(LocalID localId, uint16_t cardNo);

status_t			MemCardInfoV40(uint16_t cardNo, char *cardNameP, char *manufNameP,
						   uint16_t *versionP, uint32_t *crDateP, uint32_t *romSizeP,
						   uint32_t *ramSizeP, uint32_t *freeBytesP);

status_t			MemStoreInfoV40(uint16_t cardNo, uint16_t storeNumber,
							uint16_t *versionP, uint16_t *flagsP, char *nameP,
							uint32_t *crDateP, uint32_t *bckUpDateP,
							uint32_t *heapListOffsetP, uint32_t *initCodeOffset1P,
							uint32_t *initCodeOffset2P, LocalID* databaseDirIDP);

#ifdef __cplusplus 
}
#endif

#define		memMaxNameLen		  (32)

#define     MemNumCards           MemNumCardsV40
#define     MemNumHeaps           MemNumHeapsV40
#define     MemNumRAMHeaps        MemNumRAMHeapsV40
#define     MemHeapID             MemHeapIDV40

#define     MemPtrCardNo          MemPtrCardNoV40
#define     MemHandleCardNo       MemHandleCardNoV40

#define     MemLocalIDToGlobal    MemLocalIDToGlobalV40
#define     MemLocalIDToPtr       MemLocalIDToPtrV40
#define     MemLocalIDToLockedPtr MemLocalIDToLockedPtrV40

#endif  // __MEMORYMGRCOMPATIBILY_H__
