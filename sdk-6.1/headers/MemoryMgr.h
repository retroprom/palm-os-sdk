/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoryMgr.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Public Include file for Memory Manager. 
 *
 *****************************************************************************/

#ifndef MEMORYMGR_H_
#define MEMORYMGR_H_

// Include elementary types
#include <PalmTypes.h>					// Basic types

/************************************************************
 * Memory Manager Types
 *************************************************************/

#ifndef LocalID
typedef uint32_t  LocalID;
#endif

typedef enum {	memIDPtr, memIDHandle } LocalIDKind;

typedef struct MemHeapInfoType
{
	uint32_t  maxBlockSize;     // The size of the biggest allocatable memory block supported by heap structure
	uint32_t  defaultAlignment; // default alignment of memory chunks

	void *	basePtr;	      // Base address of heap.
    uint32_t 	maxSize;	      // Maximum number of bytes of virtual memory the heap manages

	uint32_t  physMem;          // Amount of physical memory that could be used to extend pool of memory chunks
    uint32_t	physMemUsed;      // Amount of physical memory currently in use by the heap
    uint32_t  physMemUnused;    // the amount of physical memory that could be returned back to OS 

    uint32_t  chunksNum;        // Number of chunks allocated from heap 
    uint32_t  memAllocated;     // The amount of memory used by non-free chunks

    uint32_t  chunksFree;       // Number of free chunks in heap
	uint32_t  freeSpace;        // the amount of uncommitted virtual adress space reserved for chunks
	uint32_t  freeBytes;        // Total amount of bytes that could be potentially used to allocate chunks

    uint32_t  largestBlock;     // Largest allocatable memory block 
    uint32_t  largestCommitted; // Largest allocatble and fully committed memory block
    
    uint32_t  statMaxAllocated; // maximum amount of memory allocated from heap
                              // while recording was activated
}   MemHeapInfoType;

typedef MemHeapInfoType * MemHeapInfoPtr;

/************************************************************
 * Flags accepted as parameter for MemNewChunk.
 *************************************************************/
#define memNewChunkFlagPreLock				0x0100
#define memNewChunkFlagNonMovable			0x0200
#define memNewChunkFlagAtStart				0x0400	// force allocation at front of heap
#define memNewChunkFlagAtEnd				0x0800	// force allocation at end of heap

/************************************************************
 * Memory Manager Debug settings for the MemSetDebugMode function
 *************************************************************/
#define	memDebugModeCheckOnChange			0x0001
#define	memDebugModeCheckOnAll				0x0002
#define	memDebugModeScrambleOnChange		0x0004
#define	memDebugModeScrambleOnAll			0x0008
#define	memDebugModeFillFree				0x0010
#define	memDebugModeAllHeaps				0x0020
#define	memDebugModeRecordMinDynHeapFree	0x0040 
#define memDebugModeRecordMaxDynHeapUsed    memDebugModeRecordMinDynHeapFree
#define memDebugModeValidateParams          0x0100
#define memDebugModeNoDMCall                0x0200

/************************************************************
 * Memory Manager result codes
 *************************************************************/
#define	memErrChunkLocked		 (memErrorClass | 1)
#define	memErrNotEnoughSpace	 (memErrorClass | 2)
#define	memErrInvalidParam		 (memErrorClass | 3)
#define	memErrChunkNotLocked	 (memErrorClass | 4)
#define	memErrCardNotPresent	 (memErrorClass | 5)
#define	memErrAlreadyInitialized (memErrorClass | 13)
#define memErrHeapInvalid        (memErrorClass | 14)
#define memErrEndOfHeapReached   (memErrorClass | 15)

// Obsolete Memory Manager result codes
// These are defined for compatibility
#define	memErrNoCardHeader		 (memErrorClass | 6)
#define	memErrInvalidStoreHeader (memErrorClass | 7)
#define	memErrRAMOnlyDevice		 (memErrorClass | 8)
#define	memErrWriteProtect		 (memErrorClass | 9)
#define	memErrNoRAMOnDevice		 (memErrorClass | 10)
#define	memErrNoStore			 (memErrorClass | 11)
#define	memErrROMOnlyDevice		 (memErrorClass | 12)

// Other result codes 
#define	memErrFirst	memErrChunkLocked
#define	memErrLast	memErrEndOfHeapReached

/************************************************************
 * Other constans 
 *************************************************************/

// heap options
#define	memOptSetAbsMaxMemUsage             1
#define	memOptGetAbsMaxMemUsage             2
#define	memOptSetAbsMinMemUsage             3
#define	memOptGetAbsMinMemUsage             4
#define	memOptSetMaxUnusedMem               5
#define	memOptGetMaxUnusedMem               6
#define	memOptSetForceMemReleaseThreshold 	7
#define	memOptGetForceMemReleaseThreshold 	8

// These flags are returned by MemHeapFlags call 
#define memHeapFlagROMBased   0x0001    // heap is (ROM based)
#define memHeapFlagWritable   0x0002    // heap is writable

// This is for compatibility with 4.x API, which said that it should be used to determine
// if a heap is ROM based or not.
#define memHeapFlagReadOnly   memHeapFlagROMBased

/********************************************************************
 * Memory Manager Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// Heap Info & Utilities
//-------------------------------------------------------------------
uint16_t	MemNumHeaps(void);
	
uint16_t	MemNumRAMHeaps(void);
	
uint16_t	MemHeapID(uint16_t heapIndex);

Boolean		MemHeapDynamic(uint16_t heapID);

status_t	MemHeapFreeBytes(uint16_t heapID, uint32_t *freeP, uint32_t *maxP);

uint32_t	MemHeapSize (uint16_t heapID);

uint16_t	MemHeapFlags(uint16_t heapID);

// Heap utilities
status_t	MemHeapCompact(uint16_t heapID);

// 
status_t    MemDynHeapGetInfo(MemHeapInfoType *oInfo);

uint32_t	MemDynHeapOption(uint32_t cmd, uint32_t value);

void        MemDynHeapReleaseUnused(void);

//-------------------------------------------------------------------
// Low Level Allocation/Deallocation
//-------------------------------------------------------------------
status_t	MemChunkFree(MemPtr chunkDataP);

//-------------------------------------------------------------------
// Pointer (Non-Movable) based Chunk Routines
//-------------------------------------------------------------------
MemPtr		MemPtrNew(uint32_t size);

MemPtr      MemPtrRealloc(MemPtr ptr, uint32_t newSize);

#define		MemPtrFree(p) \
					MemChunkFree(p)

// Getting Attributes
MemHandle	MemPtrRecoverHandle(MemPtr p);

uint32_t	MemPtrSize  (MemPtr p);

uint16_t	MemPtrHeapID(MemPtr p);

Boolean		MemPtrDataStorage(MemPtr p);

// Setting Attributes
status_t	MemPtrSetOwner (MemPtr p, uint16_t owner);

status_t	MemPtrResize   (MemPtr p, uint32_t newSize);

status_t	MemPtrUnlock   (MemPtr p);

//-------------------------------------------------------------------
// MemHandle (Movable) based Chunk Routines
//-------------------------------------------------------------------
MemHandle	MemHandleNew(uint32_t size);

status_t	MemHandleFree(MemHandle h);

// Getting Attributes
uint32_t	MemHandleSize (MemHandle h);

uint16_t	MemHandleHeapID     (MemHandle h);

Boolean		MemHandleDataStorage(MemHandle h);

// Setting Attributes
status_t	MemHandleSetOwner (MemHandle h,  uint16_t owner);

status_t	MemHandleResize   (MemHandle h,  uint32_t newSize);

MemPtr		MemHandleLock     (MemHandle h);

status_t	MemHandleUnlock   (MemHandle h);

//-------------------------------------------------------------------
// Utilities
//-------------------------------------------------------------------
status_t	MemMove(void *dstP, const void *sP, int32_t numBytes);

status_t	MemSet(void *dstP, int32_t numBytes, uint8_t value);

int16_t		MemCmp(const void *s1, const void *s2, int32_t numBytes);

//-------------------------------------------------------------------
// Debugging Support
//-------------------------------------------------------------------
uint16_t	MemDebugMode(void);

status_t	MemSetDebugMode(uint16_t flags);

status_t	MemHeapScramble(uint16_t heapID);
							
status_t	MemHeapCheck(uint16_t heapID);


#ifdef __cplusplus 
}
#endif

#endif  // __MEMORYMGR_H__
