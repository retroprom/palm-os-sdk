/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashDriver.h
 * 
 * Description:
 *    This file contains public definitaions for the flash
 *    drivers. 
 *
 * ToDo:
 *
 * History:
 *    4-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *
 ****************************************************************/

// Include only once
#ifndef _FLASH_DRIVER_H_
#define _FLASH_DRIVER_H_

#include <HsErrorClasses.h>


// Flash driver code resource type
#define flashCodeResType		'FLDR'

// Flash device access code resource type
//  (this is the code that needs to execute from RAM)
#define flashAccessCodeResType	'FLAC'


// The first flash driver code resource id.  The Flash Manager
//  will search for flash drivers starting with this resource
//  id.
#define flashCodeResIdFirst		1000


// Selector codes for the Flash Driver entry point function
//
typedef enum FlashEntrySelEnum {
	flashEntrySelGetInterface	= 1,	// Get the flash driver interface
										//  (FlashInterfaceType*)

	
	flashEntrySelLAST
	} FlashEntrySelEnum;


// Size of text buffer (in bytes) for retrieving a flash device's part number
//  text string
#define flashPartNoTextBufSize	64	  // including zero-terminator


// Attribute id's for FashAttrGet()
//
typedef enum FlashAttrEnum {
	flashAttrChipSize = 100,			// total size of flash chip in # of bytes
										//  (UInt32*)

	flashAttrMaxSectorSize,				// size of largest sector in # of bytes
										//  (UInt32*)

	flashAttrNumSectors,				// number of sectors on the flash chip
										//  (UInt32*)

	flashAttrSectorIndex,				// converts byte offset from start of chip
										//  to 0-based sector index
										//  (FlashSectorIndexQueryType*)

	flashAttrSectorInfo,				// sector information (offset, size)
										//  (FlashSectorInfoType*)

	
	flashAttrSectorLock,				// check if sector is locked
										//  (FlashSectorLockQueryType*)

	flashAttrFlashPartNo,				// part number text string (zero-terminated)
										//  of the first flash device (chip) at
										//  chipBaseP.
										//  (chipBaseP=ptr to start of flash device,
										//   argP=ptr to FlashPartNoInfoType,
										//	 argSize=size of FlashPartNoInfoType)

	flashAttrLAST
	} FlashAttrEnum;


typedef struct FlashSectorInfoType {
	// Filled in by caller:
	UInt32				index;			// 0-based sector index

	// Returned by FlashAttrGet(flashAttrSectorInfo):
	UInt32				size;			// byte size of sector
	UInt32				offset;			// byte offset of sector from beginning
										//  of the chip
	} FlashSectorInfoType;


typedef struct FlashSectorLockQueryType {
	// Filled in by caller:
	UInt32				index;			// 0-based sector index

	// Returned by FlashAttrGet(flashAttrSectorLock):
	UInt16				locked;			// non-zero if locked
	} FlashSectorLockQueryType;


typedef struct FlashSectorIndexQueryType {
	// Filled in by caller:
	UInt32				offset;			// byte offset from beginning of chip

	// Returned by FlashAttrGet(flashAttrSectorIndex):
	UInt32				index;			// 0-based index of sector contaning
										//  the byte at specified offset
	} FlashSectorIndexQueryType;


typedef struct {
  // Returned by Flash Driver:
  Byte					partNo[flashPartNoTextBufSize];
										// Zero-terminated text string suitable
										//  for displaying

  DWord					manufId;		// Manufacturer ID
  DWord					deviceId;		// Device ID
  } FlashPartNoInfoType;



// Flash driver error codes
enum {
	flashErrNone = 0,
	flashErrWriteFailed			= hsFlashErrorClass + 1,
	flashErrEraseFailed			= hsFlashErrorClass + 2,
	flashErrInvalidSelId		= hsFlashErrorClass + 3,
	flashErrNoMemory			= hsFlashErrorClass + 4,
	flashErrBadArg				= hsFlashErrorClass + 5,
	flashErrInvalidAttrId		= hsFlashErrorClass + 6,
	flashErrUnknownDevice		= hsFlashErrorClass + 7,
	flashErrUnknownManuf		= hsFlashErrorClass + 8,
	flashErrCodeDbOpenError		= hsFlashErrorClass + 9,
	flashErrNoAccessResource	= hsFlashErrorClass + 10,
	flmErrOperationNotSupported	= hsFlashErrorClass + 11
  };


//-----------------------------------------------------------------------------
// Flash Driver API function type definitions
//-----------------------------------------------------------------------------

typedef DWord FlashEntryFuncType (UInt32 selId, void* argP, UInt32 argSize);

typedef DWord FlashOpenFuncType (void** refPP, UInt16 codeDbCardNo,
									LocalID codeDbId, UInt16 codeResId,
									void* chipBaseP);

typedef DWord FlashCloseFuncType (void* refP);

typedef DWord FlashAttrGetFuncType (void* refP, void* chipBaseP, UInt32 attrId,
									void* argP, UInt32 argSize);

typedef DWord FlashExistsFuncType (void* refP, void* chipBaseP);

typedef DWord FlashSectorEraseFuncType (void* refP, void* chipBaseP,
										void* sectorBaseP);

typedef DWord FlashChipWriteFuncType (void* refP, void* chipBaseP, void* dstP,
									  void* srcP, UInt32 numBytes);

// This function checks whether the two chips are at the same physical
//  address.  Due to address wrap-around on the Flash Module, what appears
//  as two devices at distinct logical addresses, may actuall be a single
//  device.  Returns non-zero if they are the same device.
typedef Byte  FlashSameDeviceFuncType (void* refP, void* chip0BaseP,
									   void* chip1BaseP);



typedef struct FlashInterfaceType {
	FlashEntryFuncType*			FlashEntry;
	FlashOpenFuncType*			FlashOpen;
	FlashCloseFuncType*			FlashClose;
	FlashAttrGetFuncType*		FlashAttrGet;
	FlashExistsFuncType*		FlashExists;
	FlashSectorEraseFuncType*	FlashSectorErase;
	FlashChipWriteFuncType*		FlashChipWrite;
	FlashSameDeviceFuncType*	FlashSameDevice;
	} FlashInterfaceType;


//-----------------------------------------------------------------------------
// Flash Manager Routines
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus 
}
#endif




#endif	// _FLASH_DRIVER_H_ -- include only once