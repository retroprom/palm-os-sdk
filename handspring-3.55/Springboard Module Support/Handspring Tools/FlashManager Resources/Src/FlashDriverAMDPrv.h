/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashDriverAMDPrv.h
 * 
 * Description:
 *    This file contains private definitaions for the AMD Flash
 *    Driver code. 
 *
 * ToDo:
 *
 * History:
 *	  08-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *	  17-Apr-2000	vmk	  Added definitions for FlashAMDDevGeomEnum
 *						   and FlashAMDDeviceTableEntryType as
 *						   part of conversion to device table-based
 *						   sector arithmetic;
 *
 ****************************************************************/

// Include only once
#ifndef _FLASH_DRIVER_AMD_PRV_H_
#define _FLASH_DRIVER_AMD_PRV_H_


// FlashAMDDevGeomEnum: Device geometry -- for determination of
//  sector arithmetic algorithms
typedef enum {
  flashAMDDevGeomFIRST = 0,		// reserve 0 for error checking

  flashAMDDevGeom64KBlkBotBoot16_8_8_32,  // bottom boot block 16-8-8-32
  flashAMDDevGeom64KBlkTopBoot32_8_8_16,  // top boot block 32-8-8-16
  flashAMDDevGeom4KBlkUniform,			  // uniform 4KB blocks


  flashAMDDevGeomLAST			// for error checking
} FlashAMDDevGeomEnum;


typedef struct FlashAMDDeviceTableEntryType {
	char				partName[32];	// a part name string (for example
										//  "AMD Am29LV800B")
	Byte				manufId;		// manufacturer id (flashManu...)
	Byte				deviceId;		// device id (flashDev...)
	UInt32				numBytes;		// size of the flash chip in BYTEs
	UInt16				numSectors;		// number of sectors on the chip
	UInt32				maxSectorBytes;	// byte size of the largest flash
										//  sector -- this determines the size
										//  of the backup ram allocation
	FlashAMDDevGeomEnum	devGeometry;	// device geometry -- flashAMDDevGeom...
	} FlashAMDDeviceTableEntryType;



// Signature for Flash Manager globals (for debugging)
#define flashAMDGlobalsStartSignature		'flda'
#define flashAMDGlobalsEndSignature			'FLDA'
#define flashAMDGlobalsDiscardSignature		'ADLF'



typedef struct FlashAMDCommonInfoType {
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

	} FlashAMDCommonInfoType;



typedef struct FlashAMDDeviceInfoType {
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

	const FlashAMDDeviceTableEntryType* deviceEntryP; // set to the device
										// description structure in PrvFlashAMDOpen
										// if the flash ship is supported by
										// this flash driver; NULL otherwise;
	} FlashAMDDeviceInfoType;


// Selector codes for the Flash Access entry point function
//
typedef enum AMDAccessEntrySelEnum {
	amdAccessEntrySelGetInterface	= 300,	// Get the flash access interface
											//  (FlashAMDAccessInterfaceType*)

	
	amdAccessEntrySelLAST
	} AMDAccessEntrySelEnum;


//-----------------------------------------------------------------------------
// Function types of our device access API
//-----------------------------------------------------------------------------
struct FlashAMDGlobalsType;


// This is the type of the entry point function of the flash access code
//  resource
typedef DWord FlashAMDAccessEntryFuncType (UInt32 selId, void* argP, UInt32 argSize);


// This function identifies the manufacturer and device
typedef DWord FlashAMDGetManufAndDeviceFuncType (struct FlashAMDGlobalsType* gP,
												void* chipBaseP, Byte* manufP,
												Byte* deviceP);


// This function erases a sector of flash
typedef DWord FlashAMDSectorEraseFuncType (struct FlashAMDGlobalsType* gP,
										   void* chipBaseP,
										   UInt32 sectorOffset);


// This function checks a flash sector's lock status
typedef DWord FlashAMDSectorIsLockedFuncType (struct FlashAMDGlobalsType* gP,
											  void* chipBaseP,
											  UInt32 sectorOffset,
											  UInt16* lockedP);


// This function writes to flash.  It is copied and executed from RAM.
typedef DWord FlashAMDChipWriteFuncType (struct FlashAMDGlobalsType* gP,
										Word* chipBaseWordP, Byte* dstByteP,
										Byte* srcByteP, UInt32 numBytes);

// This function checks whether the two chips are at the same physical
//  address.  Due to address wrap-around on the Flash Module, what appears
//  as two devices at distinct logical addresses, may actuall be a single
//  device.  Returns non-zero if they are the same device.
typedef Byte  FlashAMDSameDeviceFuncType (struct FlashAMDGlobalsType* gP,
										  void* chip0BaseP,
										  void* chip1BaseP);


typedef struct FlashAMDAccessInterfaceType {
	FlashAMDGetManufAndDeviceFuncType*		GetManufAndDevice;
	FlashAMDSectorEraseFuncType*			SectorErase;
	FlashAMDSectorIsLockedFuncType*			SectorIsLocked;
	FlashAMDChipWriteFuncType*				ChipWrite;
	FlashAMDSameDeviceFuncType*				SameDevice;
	} FlashAMDAccessInterfaceType;





// This is the type of the private globals for the
//  AMD Flash Driver
typedef struct FlashAMDGlobalsType {
	DWord				startSignature;	// must be set to flashAMDGlobalsStartSignature
										//  for debugging
										
	FlashAMDCommonInfoType	common;		// common information

	FlashAMDDeviceInfoType	device;		// device information

	FlashAMDAccessInterfaceType
							program;	// our RAM-resident device access interface

	DWord				endSignature;	// must be set to flashAMDGlobalsEndSignature
										//  for debugging
	} FlashAMDGlobalsType;



//-----------------------------------------------------------------------------
// AMD Flash Driver Routines
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern void
FlashAMDDeviceTableInit (void);


extern const FlashAMDDeviceTableEntryType*
FlashAMDDeviceTableEntryFind (Byte manufId, Byte deviceId, DWord* errP);

extern DWord
FlashAMDGetSectorIndex (const struct FlashAMDDeviceTableEntryType* descP,
										  FlashSectorIndexQueryType* queryP);

extern DWord
FlashAMDGetSectorInfo (const struct FlashAMDDeviceTableEntryType* descP,
										  FlashSectorInfoType* infoP);

#ifdef __cplusplus 
}
#endif



#endif	// _FLASH_DRIVER_AMD_PRV_H_ -- include only once