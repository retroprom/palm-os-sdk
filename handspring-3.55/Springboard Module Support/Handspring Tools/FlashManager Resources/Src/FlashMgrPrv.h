/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashMgrPrv.h
 * 
 * Description:
 *    This file contains private definitaions for the code that
 *    manages flash drivers. 
 *
 * ToDo:
 *
 * History:
 *    4-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *    20-Dec-2000 - Added the Heap and data Segment write back buffer definitions
 *
 ****************************************************************/

// Include only once
#ifndef _FLASH_MGR_PRV_H_
#define _FLASH_MGR_PRV_H_


// Signature for Flash Manager globals (for debugging)
#define flmGlobalsStartSignature	'fmgs'
#define flmGlobalsEndSignature		'FMGE'
#define flmGlobalsDiscardSignature	'FMGD'

//  NOTE: The reason for
//  breaking up the backup area into multiple segments is that the
//  maximum memory chunk that can be allocated under the present
//  version of PalmOS is a few bytes less than 64K.  However, we
//  need to support flash configurations that have up to 128KB sectors.
#define flmMaxBackupHeapBuffers		16
#define flmMaxBackupDataBuffers		16

// flmSectorBackupChunkSize: size of each sector backup chunk;
//  one or more of these will be allocated so that the sum is
//  at lease as big as the maximum flash chip sector size
#define flmSectorBackupChunkSize	(4 * 1024)

// Maximum supported flash memory sector size
#define flmMaxSupportedSectorSize	(128UL * 1024UL)


// flmFlashStartCheckBytes:
//  This value is used when searching for flash chips to determine
//  if the address has wrapped around -- this can happen
//  when the flash card ignores some of the high address bits
//  that are not strictly necessary for amount of flash on the
//  card.
//
//  We do this by comparing data at the start of the current
//  chip address with that at flash base.  Since the data at
//  flash base should be the original card header (of the
//  support store), a match at this point will indicate a wrap-
//  around.
#define flmFlashStartCheckBytes		256


// flmAttrFlashManagerVersion refer to 
// FlmFlashManagerVersionInfoType in FlashMgr.h for details
// Other than version 1.0 that spanned the Flash Module
// release 1.0 to 1.3, with some changes, the first real versio'n
// identification came with 1.5 including the release of the Strataflash 
// Drivers, the implementation of flmOpenChipSelectAutoConfig, the usage
// of feature memory segments for writeback buffering, and the implementation
// of the PrvCheckIfErased before erasing sectors optimization.
#define	flmFlashManagerVersionReservedByte	0x00
#define	flmFlashManagerMajorVersion			0x01
#define	flmFlashManagerMinorVersion			0x05
#define	flmFlashManagerFixVersion			0x00 

// FlmCommonInfoType
typedef struct {
	// Passed in by our client to FlmOpen()
	//
	UInt16				codeDbCardNo;	// card number of the resource database
										//  containing the Flash Manager and all
										//  the Flash Drivers
	LocalID				codeDbId;		// local ID of the resource database
										//  containing the Flash Manager and
										//  all the Flash Drivers.
	// <chg 2001-2-20 JMP> Added the Flash Card ID to 
	UInt16				flashCardID;	//	Card ID of the flash card.
	void*				cardBaseP;		// base address of the card
	UInt32				flashOffset;	// byte offset from card base to the
										//  first flash chip
	UInt32				readOnlySize;	// byte size of the pre-flashed, read-
										//  only area beginning at flashOffset
	UInt32				maxRange;		// maximum number of bytes at beginning
										//  of flash that may be searched for
										//  flash chips
	Byte				maxRangeIsFlashSize;
										// if non-zero, then maxRange contains
										//  the actual flash size specified
										//  by the caller
	DWord				openFlags;		// flmOpenFlag... flags



	// Computed when Flash Manager is opened (PrvFlashDriversLoad())
	//
	Byte				flashEnumerated;// is set to non-zero after the
										//  members in this group have been
										//  computed
	UInt16				numFlashChips;	// # of flash chips
	UInt32				totalFlashSize;	// byte size total of all flash chips

	UInt32				writableSize;	// total size of writable area begining
										//  with the first writable sector
	UInt32				writableOffset;	// byte offset from card base to the
										//  first writable sector
	
	
	void*				flashBaseP;		// pointer to the start of flash memory
	void*				writableBaseP;	// pointer to start of writable region
	void*				writableEndP;	// pointer to end of writable region
										//  (one byte past last writable byte)

	// Members managed by PrvOpenCodeDatabase() and PrvCloseCodeDatabase()
	//
	// When the Flash Manager opens its code resource database, it has
	//  to make sure that it is closed before control is returned to client
	//
	UInt16				codeDbOpenCount;// count of nested open requests for the
										//  code resource database; the database
										//  will be opened just once physically
										//  if PrvOpenCodeDatabase() is called
										//  when the count is zero, and will be
										//  closed when the count drops again
										//  to zero.
	DmOpenRef			codeDbRef;		// reference to open code resource
										//  database

	} FlmCommonInfoType;



// FlmDriverInfoType
typedef struct {
	Boolean				loaded;			// driver was loaded
	Word				resId;			// flash driver code resource ID
	FlashEntryFuncType*	entryP;			// flash driver entry point function
	FlashInterfaceType	api;			// flash driver interface functions
	void*				refP;			// flash driver ref value returned by
										//  FlashOpen for passing to all other
										//  flash driver func calls
	// Current assumption: flash consists of 1 or more same flash components
	UInt32				chipSize;		// byte size of each flash chip
	UInt32				numSectors;		// number of sectors on the chip
	UInt32				maxSectorSize;	// byte size of the largest flash
										//  sector -- this determines the size
										//  of the backup area
	} FlmDriverInfoType;


// FlmBackupType: this structure is a descriptor of backup RAM
//  segments used to back up flash sector data around a region
//  that is being erased.
//

// FlmBackupType
typedef struct {
	UInt16				numHeapSeg;		// number of backup RAM segments
	UInt16				numDataSeg;		// number of backup Data segments
	UInt32				segSize;		// size of each segment
	// Array of pointers to backup segments
	Byte*				seg[flmMaxBackupHeapBuffers + flmMaxBackupDataBuffers];
	} FlmBackupType;


// FlmGlobalsType
typedef struct {
	DWord				startSignature;	// must be set to flmGlobalsStartSignature
										//  for debugging
										

	FlmCommonInfoType	common;			// card inormation

	FlmDriverInfoType	driver;			// flash driver information

	// Backup buffers for flash driver(s)
	FlmBackupType		backupInfo;		// Backup RAM allocated
										//  by Flash Manager for write and erase
										//	operations.  It is allocated only when
										//  the first modify operation is
										//  requested -- a ptr to this structure
										//  will be passed to the flash drivers
										//  during modify operations.

	FlmProgressReportType progress;		// for reporting progress

	DWord				endSignature;	// must be set to flmGlobalsEndSignature
										//  for debugging
	} FlmGlobalsType;


#endif	// _FLASH_MGR_PRV_H_ -- include only once
