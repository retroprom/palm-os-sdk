/***************************************************************
 *
 * Project:
 *    Ken Expansion Card
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 2000.
 *
 * FileName:
 *    FlashDriverIntelPrv.h
 *
 * Description:
 *    This file contains private definitaions for the Intel Flash
 *    Driver code.
 *
 * ToDo:
 *
 * History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/

// Include only once
#ifndef _FLASH_DRIVER_INTEL_PRV_H_
#define _FLASH_DRIVER_INTEL_PRV_H_


// FlashIntelDevGeomEnum: Device geometry -- for determination of
//  sector arithmetic algorithms
typedef enum {
  flashIntelDevGeomFIRST = 0,		// reserve 0 for error checking

  flashIntelDevGeom128KBBoot128,


  flashIntelDevGeomLAST			// for error checking
} FlashIntelDevGeomEnum;


typedef struct FlashIntelDeviceTableEntryType {
	char				partName[32];	// a part name string (for example
										//  "E28F640J3")
	Byte				manufId;		// manufacturer id (flashManu...)
	Byte				deviceId;		// device id (flashDev...)
	UInt32				numBytes;		// size of the flash chip in BYTEs
	UInt16				numSectors;		// number of sectors on the chip
	UInt32				maxSectorBytes;	// byte size of the largest flash
										//  sector -- this determines the size
										//  of the backup ram allocation
	FlashIntelDevGeomEnum	devGeometry;	// device geometry -- flashIntelDevGeom...
	} FlashIntelDeviceTableEntryType;



// Signature for Flash Manager globals (for debugging)
#define flashIntelGlobalsStartSignature			'flda'
#define flashIntelGlobalsEndSignature			'FLDA'
#define flashIntelGlobalsDiscardSignature		'ADLF'



typedef struct FlashIntelCommonInfoType {
	//-------------------------------------------------------------------------
	// Passed in by our client to FlashOpen():
	//-------------------------------------------------------------------------
	UInt16				codeDbCardNo;	// card number of the resource database
										//  containing the Flash Manager and all
										//  the Flash Drivers
	LocalID				codeDbId;		// local ID of the resource database
										//  containing the Flash Manager and
										//  all the Flash Drivers.
	UInt16				codeResId;		// resource id of our code resourc

	void*				flashBaseP;		// chip base address passed to the driver's
										//  FlashOpen() function

	//-------------------------------------------------------------------------
	// Generated at runtime
	//-------------------------------------------------------------------------

	// Info for RAM-resident copy of flash code:
	//  Because we might be executing out of the same flash chip that we'll
	//	flashing, the execution and flashing mode would interfere
	//  with each other.  So, we're going to copy a small amount of code that
	//	talks to the chip into RAM and execute it from there.
	void*				codeCopyP;		// pointer to code chunk in the dynamic
										//  heap of the device's built-in memory
										//  card

	} FlashIntelCommonInfoType;



typedef struct FlashIntelDeviceInfoType {
	Byte				identified;		// set to non-zero when flash is edentified
	Byte				unused1;
	Byte				manufId;		// manufacturer id (flashManu...)
	Byte				deviceId;		// device id (flashDev...)
	UInt32				numBytes;		// size of the flash chip in BYTEs
	UInt32				numWords;		// size of the flash chip in WORDs
	UInt32				numSectors;		// number of sectors on the chip
	UInt32				maxSectorBytes;	// byte size of the largest flash
										//  sector -- this determines the size
										//  of the backup area
	DWord				sharedAttrs;	// device attributes (flashAttr...)

	const FlashIntelDeviceTableEntryType* deviceEntryP; // set to the device
										// description structure in PrvFlashIntelOpen
										// if the flash ship is supported by
										// this flash driver; NULL otherwise;
	} FlashIntelDeviceInfoType;


// Selector codes for the Flash Access entry point function
//
typedef enum IntelAccessEntrySelEnum {
	IntelAccessEntrySelGetInterface	= 300,	// Get the flash access interface
											//  (FlashIntelAccessInterfaceType*)


	IntelAccessEntrySelLAST
	} IntelAccessEntrySelEnum;


//-----------------------------------------------------------------------------
// Function types of our device access API
//-----------------------------------------------------------------------------
struct FlashIntelGlobalsType;


// This is the type of the entry point function of the flash access code
//  resource
typedef DWord FlashIntelAccessEntryFuncType (UInt32 selId, void* argP, UInt32 argSize);


// This function identifies the manufacturer and device
typedef DWord FlashIntelGetManufAndDeviceFuncType (struct FlashIntelGlobalsType* gP,
												void* chipBaseP, Byte* manufP,
												Byte* deviceP);


// This function erases a sector of flash
typedef DWord FlashIntelSectorEraseFuncType (struct FlashIntelGlobalsType* gP,
										   void* chipBaseP,
										   UInt32 sectorOffset);


// This function checks a flash sector's lock status
typedef DWord FlashIntelSectorIsLockedFuncType (struct FlashIntelGlobalsType* gP,
											  void* chipBaseP,
											  UInt32 sectorOffset,
											  UInt16* lockedP);


// This function writes to flash.  It is copied and executed from RAM.
typedef DWord FlashIntelChipWriteFuncType (struct FlashIntelGlobalsType* gP,
										Word* chipBaseWordP, Byte* dstByteP,
										Byte* srcByteP, UInt32 numBytes);

// This function checks whether the two chips are at the same physical
//  address.  Due to address wrap-around on the Flash Module, what appears
//  as two devices at distinct logical addresses, may actuall be a single
//  device.  Returns non-zero if they are the same device.
typedef Byte  FlashIntelSameDeviceFuncType (struct FlashIntelGlobalsType* gP,
										  void* chip0BaseP,
										  void* chip1BaseP);


typedef struct FlashIntelAccessInterfaceType {
	FlashIntelGetManufAndDeviceFuncType*		GetManufAndDevice;
	FlashIntelSectorEraseFuncType*				SectorErase;
	FlashIntelSectorIsLockedFuncType*			SectorIsLocked;
	FlashIntelChipWriteFuncType*				ChipWrite;
	FlashIntelSameDeviceFuncType*				SameDevice;
	} FlashIntelAccessInterfaceType;





// This is the type of the private globals for the
//  Intel Flash Driver
typedef struct FlashIntelGlobalsType {
	DWord				startSignature;	// must be set to flashIntelGlobalsStartSignature
										//  for debugging

	FlashIntelCommonInfoType	common;		// common information

	FlashIntelDeviceInfoType	device;		// device information

	FlashIntelAccessInterfaceType
							program;	// our RAM-resident device access interface

	DWord				ticksPerSecond; //System ticks per seconds 

	DWord				endSignature;	// must be set to flashIntelGlobalsEndSignature
										//  for debugging
	} FlashIntelGlobalsType;



//-----------------------------------------------------------------------------
// Intel Flash Driver Routines
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern void
FlashIntelDeviceTableInit (void);


extern const FlashIntelDeviceTableEntryType*
FlashIntelDeviceTableEntryFind (Byte manufId, Byte deviceId, DWord* errP);

extern DWord
FlashIntelGetSectorIndex (const struct FlashIntelDeviceTableEntryType* descP,
										  FlashSectorIndexQueryType* queryP);

extern DWord
FlashIntelGetSectorInfo (const struct FlashIntelDeviceTableEntryType* descP,
										  FlashSectorInfoType* infoP);

#ifdef __cplusplus
}
#endif



#endif	// _FLASH_DRIVER_INTEL_PRV_H_ -- include only once