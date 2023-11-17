/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashMgr.h
 * 
 * Description:
 *    This file contains public definitaions for the Flash
 *    Manager. 
 *
 * ToDo:
 *
 * History:
 *    4-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *    20-Dec-2000 - Registered the FLMG creator and added the write back
 *                  buffer Feature
 *
 ****************************************************************/

// Include only once
#ifndef _FLASH_MGR_H_
#define _FLASH_MGR_H_


#include <HsErrorClasses.h>


// The Flash Manager code resource type and id.  Our client glue
//  code will search for this code resource which contains
//  the Flash Manager's entry point.
#define flmCodeResType			hsFileCFlMgr  //'FLMG' defined in HsCreator
#define flmCodeResId			2000


//	The DataHeap buffer features is used when allocating WriteBack buffer 
//	when more than 64K of WriteBack buffer is required. Each feature is a 4K 
//  (flmSectorBackupChunkSize) buffer and the Flash Manager allocats as  
//  many features as needed to accomodate the write back buffer size 
//  required by the Flash sector size. feature are numbered starting at 100 
//	(100, 101, 102,..., (100+flmMaxBackupDataBuffer)).
#define flmDataBuffCFlMgr		hsFileCFlMgr
#define flmDataBuffFtrBase		100



// Flags passed to FlmOpen()
//
#define flmOpenFlagRead					0x00000001UL
#define flmOpenFlagWrite				0x00000002UL
#define flmOpenChipSelectAutoConfig		0x00000004UL
// <chg 2001-2-13 JMP> changed from flmOpenFlagAutoScan 
//	to flmOpenChipSelectAutoConfig

// For convenience
#define flmOpenFlagReadWrite	(flmOpenFlagRead | flmOpenFlagWrite)

// For error checking
#define flmOpenFlagAllFlags		(flmOpenFlagRead		\
								 | flmOpenFlagWrite		\
								 | flmOpenFlagReadWrite	\
								 | flmOpenChipSelectAutoConfig)

// Size of text buffer (in bytes) for retrieving a flash device's part number
//  text string
#define flmPartNoTextBufSize	64	  // including zero-terminator


// Selector codes for the Flash Manager entry point function
//
typedef enum FlmEntrySelEnum {
	// start at 2000 for error-checking purposes
	//  to avoid collision with flash driver selectors

	flmEntrySelGetInterface	= 2000,		// get the Flash Manager interface
										//  (FlmInterfaceType)

	
	flmEntrySelLAST
	} FlmEntrySelEnum;


// Attribute id's for FlmAttrGet()
//
typedef enum FlmAttrEnum {
	flmAttrFlashConfig = 2100,			// flash configuration information
										//  (FlmFlashConfigInfoType)

	flmAttrFlashPartNo,					// part number text string (zero-terminated)
										//  of the flash device (chip) at a given
										//  base address
										//  (FlmFlashPartNoInfoType)



	flmAttrFlashManagerVersion,			// Flash manager Version is a double Word
										// with Byte 0 reserved for later us, Byte 1 
										// for the Major Version, Byte 2 Minor Version,
										// and Byte 3 for the Revisions. 
										// See ..\Docs\Version.txt for the antology.
	flmAttrLAST
	} FlmAttrEnum;


// Passed to FlmAttrGet(flmAttrFlashConfig):
//
// FlmFlashConfigInfoType
typedef struct {
	UInt16				numFlashChips;	// number of flash chips found

	UInt32				totalFlashSize;	// total byte size of all flash
										//  chips on the card
	UInt32				writableSize;	// byte size of writable region of
										//  flash memory
	UInt32				writableOffset;	// byte offset from logical card base to
										//  the writable flash memory (first
										//  non-read-only sector)


	} FlmFlashConfigInfoType;


// Passed to FlmAttrGet(flmAttrFlashManagerVersion):
//
// FlmFlashManagerVersionInfoType
typedef struct {
	Byte				reservedByte;	// reserved 
	Byte				majorVersion;	// Minor Version Number
	Byte				minorVersion;	// Minor Version Number
	Byte				fixNumber;		// Fix Number

	} FlmFlashManagerVersionInfoType;

// Passed to FlmAttrGet(flmAttrFlashPartNo):
//
// FlmFlashPartNoInfoType
typedef struct {

  // Must be filled in by caller:
  void*					chipBaseP;		// pointer to start of flash chip
										//  that is in the flash range that
										//  was recognized by FlashOpen.  The
										//  size of this range is returned
										//  via flmAttrFlashConfig in the
										//  "totalFlashSize" field.

  // Returned by Flash Manager:
  Byte					partNo[flmPartNoTextBufSize];
										// Zero-terminated text string suitable
										//  for displaying

  DWord					manufId;		// Manufacturer ID
  DWord					deviceId;		// Device ID
  } FlmFlashPartNoInfoType;


// Optional optimization parameter for FlmErase(); if passed,
//  declares a region of flash memory that need not be restored.
//
// FlmEraseOptType
typedef struct {
  Byte					restoreOff;	  // if non-zero, turns off
									  //  restoration of data in regions
									  //  affected by the erase operation,
									  //  and the other fields are ignored

  Byte					reserved;	  // set to zero

  void*					skipP;		  // pointer to block of memory that need
									  //  not be restored
  UInt32				skipBytes;	  // # of bytes in the block that need not
									  //  be restored

  } FlmEraseOptType;


// Optional optimization parameter for FlmMove(); if passed,
//  declares a region of flash memory that need not be restored.
//
// FlmMoveOptType
typedef struct {
  Byte					dontErase;	  // if non-zero, skips pre-erasing
									  //  of destination region (default
									  //  behaviour is to pre-erase)

  Byte					reserved;	  // set to zero

  void*					skipP;		  // pointer to block of memory that need
									  //  not be restored
  UInt32				skipBytes;	  // # of bytes in the block that need not
									  //  be restored

  } FlmMoveOptType;



// ----------------------------------------------------------------------------
// Progress reporting types
// ----------------------------------------------------------------------------

// FlmProgressReportType
typedef struct
  {
	void*				flmRefP;
	UInt32				soFar;			// completed so far
	UInt32				total;			// total (denominator)

	void*				userRefP;		// caller's info

	DWord				reserved;		// reserved, FlmMgr will set to 0
  } FlmProgressReportType;

// Returing a non-zero value from this callback function signals a pending
//  abort request -- however, none of the current FlmMgr operations may be
//  aborted, and the operation will complete anyway.
typedef Byte FlmProgressCBFuncType (FlmProgressReportType* reportP);


// FlmProgressReqType
typedef struct
  {
	FlmProgressCBFuncType*	callbackP;
	void*				userRefP;		// caller's info to be passed
										//  to the callback inside the
										//  FlmProgressReportType structure
  } FlmProgressReqType;



// Flash Manager error codes
enum {
	flmErrNone = 0,
	flmErrBadArg				  = hsFlmErrorClass + 1,
	flmErrInvalidSelId			  = hsFlmErrorClass + 2,
	flmErrNoMemory				  = hsFlmErrorClass + 3,
	flmErrInvalidAttrId			  = hsFlmErrorClass + 4,
	flmErrNoDriver				  = hsFlmErrorClass + 5,
	flmErrFlashEnumError		  = hsFlmErrorClass + 6,
	flmErrIntTooManyBackupChunks  = hsFlmErrorClass + 7
  };



//-----------------------------------------------------------------------------
// Flash Manager API function type definitions
//-----------------------------------------------------------------------------

typedef DWord FlmEntryFuncType		(UInt32 selId, void* argP, UInt32 argSize);

typedef DWord FlmOpenFuncType		(void** refPP, UInt16 codeDbCardNo,
									LocalID codeDbId, void* cardBaseP,
									UInt32 flashOffset, UInt32 readOnlySize,
									UInt32 maxRange, Byte maxRangeIsFlashSize,
									DWord openFlags);

typedef DWord FlmCloseFuncType		(void* refP);

typedef DWord FlmAttrGetFuncType	(void* refP, UInt32 attrId, void* argP,
									UInt32 argSize);

// progrReqP is optional (pass NULL to ignore), and support for it is not
//  implemented in the current version of Flash Manager
typedef DWord FlmEraseFuncType		(void* refP, void* startP, UInt32 eraseBytes,
									FlmEraseOptType* optP,
									FlmProgressReqType* progrReqP);

typedef DWord FlmIsErasedFuncType	(void* refP, void* startP, UInt32 numBytes,
									 Byte* isErasedP);

typedef DWord FlmWriteFuncType		(void* refP, void* dstP, void* srcP,
									UInt32 numBytes);

// progrReqP is optional (pass NULL to ignore), and support for it is not
//  implemented in the current version of Flash Manager
typedef DWord FlmMoveFuncType		(void* refP, void* dstP, void* srcP,
									UInt32 numBytes, FlmMoveOptType* optP,
									FlmProgressReqType* progrReqP);

typedef struct FlmInterfaceType {
	FlmEntryFuncType*	  FlmEntry;
	FlmOpenFuncType*	  FlmOpen;
	FlmCloseFuncType*	  FlmClose;
	FlmAttrGetFuncType*	  FlmAttrGet;
	FlmEraseFuncType*	  FlmErase;
	FlmIsErasedFuncType*  FlmIsErased;
	FlmWriteFuncType*	  FlmWrite;
	FlmMoveFuncType*	  FlmMove;
	} FlmInterfaceType;



//-----------------------------------------------------------------------------
// Flash Manager Routines
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus 
}
#endif





#endif	// _FLASH_MGR_H_ -- include only once
