/***************************************************************
 *
 * Project:
 *    Ken Expansion Card
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999-2000.
 *
 * FileName:
 *    FlashDriverAMDDevTable.c
 *
 * Description:
 *    This file contains the flash device description data and code for
 *	   flash devices that are programatically compatible with AMD Am29LV800B;
 *
 *	  This includes:
 *
 *				AMD Am29LV800B/Am29LV160BB/Am29LV160BT and
 *				Fujitsu MBM29LV160B (equivalent to Am29LV160BB) and
 *				Fujitsu MBM29LV160TE (equivalent to Am29LV160BT) and
 *				Toshiba TC58FYB160 (equivalent to Am29LV160BB) and
 *				SST32HF162/164 and SST32HF802.
 *
 * Adding support for new flash devices:
 *
 *	It should be fairly easy to add support for new flash devices that
 *	 use the same programming algorithms as AMD Am29LV800B. If your flash
 *	 device has the same basic geometry (base sector size, relative boot
 *	 block location -- top or bottom -- and layout of the boot block)
 *	 as one of the supported flash devices (see DeviceDescriptions array
 *	 in this source file), then you only need to add a new entry to the
 *	 DeviceDescriptions array. To do this:
 *		1. Define macros for your flash device's manufacturer id and
 *			device id in the header file FlashDriverCommon.h.
 *		2. Add an entry for your device to the DeviceDescriptions array
 *			in this source file -- FlashDriverAMDDevTable.c, using an
 *			existing entry as a template.
 *		3. Recompile any necessary code and test against your flash device.
 *
 *	 If your flash device's basic geometry differs from those that are
 *	  already supported, then you will need to:
 *		1. Define macros for your flash device's manufacturer id and
 *			device id in the header file FlashDriverCommon.h.
 *		2. Add a new device geometry enum for your device to FlashAMDDevGeomEnum in
 *			FlashDriverAMDPrv.h.
 *		3. Add an entry for your device to the DeviceDescriptions array
 *			in this source file -- FlashDriverAMDDevTable.c.
 *		4. Create two new functions in this source file -- one for computing
 *			sector index (see Prv64KBlkTopBoot32_8_8_16GetSectorIndex), and
 *			another for computing sector offset and size (see
 *			Prv64KBlkBotBoot16_8_8_32GetSectorInfo).
 *		5. Update the functions FlashAMDGetSectorInfo and FlashAMDGetSectorIndex
 *			(in this source file) with an "else if" case for your device type.
 *		6. Recompile any necessary code and test against your flash device.
 *
 *
 * ToDo:
 *
 * History:
 *    14-Apr-2000 - Created by Vitaly Kruglikov
 *	  17-Apr-2001	vmk	  Added support for SST32HF162/164 and SST32HF802
 *
 ****************************************************************/
#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>


#include "FlashDriver.h"
#include "FlashDriverCommon.h"
#include "FlashDriverAMDPrv.h"


//=============================================================================
// Private function prototypes
//=============================================================================



//=============================================================================
// Array of static device descriptions -- the compiler places these in the
//  code resource (keep in mind that code segments do *not* have
//  real, application-like, globals)
//------------------------------------------------------------------------
//=============================================================================
#define prvByteCount16MB	( 16UL * 1024UL * 1024UL)
#define prvByteCount8MB		(  8UL * 1024UL * 1024UL)
#define prvByteCount4MB		(  4UL * 1024UL * 1024UL)
#define prvByteCount2MB		(  2UL * 1024UL * 1024UL)
#define prvByteCount1MB		(  1UL * 1024UL * 1024UL)
#define prvByteCount128KB	(128UL * 1024UL)
#define prvByteCount64KB	( 64UL * 1024UL)
#define prvByteCount32KB	( 32UL * 1024UL)
#define prvByteCount16KB	( 16UL * 1024UL)
#define prvByteCount8KB		(  8UL * 1024UL)
#define prvByteCount4KB		(  4UL * 1024UL)


#define prvDeviceDescEntryCount	  \
		  sizeof(DeviceDescriptions) / sizeof(DeviceDescriptions[0])

#pragma pcrelconstdata on

// NOTE:
// All entries MUST have unique manufId/deviceId pairs; empty part name strings
//  are NOT allowed;
static const FlashAMDDeviceTableEntryType DeviceDescriptions[] = {

	{
	"AMD Am29LV160BB",		  // a part name string
	flashManuAMD,			  // manufacturer id (flashManu...)
	flashDevAMD16MbBB,		  // AMD Am29LV160BB (16 megabits, bottom boot block)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount2MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the bottom boot block is partitioned into 4
							  //  sectors -- 16-8-8-32)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkBotBoot16_8_8_32 // device geometry -- flashAMDDevGeom...
	},

	{
	"AMD Am29LV160BT",		  // a part name string
	flashManuAMD,			  // manufacturer id (flashManu...)
	flashDevAMD16MbBT,		  // AMD Am29LV160BT (16 megabits, top boot block)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount2MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the top block is partitioned into 4
							  //  sectors -- 32-8-8-16)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkTopBoot32_8_8_16 // device geometry -- flashAMDDevGeom...
	},


	{
	"AMD Am29LV800B",		  // a part name string
	flashManuAMD,			  // manufacturer id (flashManu...)
	flashDevAMD8MbBB,		  // AMD Am29LV800B (8 megabits, bottom boot block)
	prvByteCount1MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount1MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the bottom boot block is partitioned into 4
							  //  sectors -- 16-8-8-32)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkBotBoot16_8_8_32 // device geometry -- flashAMDDevGeom...
	},


	{
	"Fujitsu MBM29LV160B",	  // a part name string
	flashManuFujitsu,		  // manufacturer id (flashManu...)
	flashDevFuj16MbBB,		  // Fujitsu MBM29LV160B (16 megabits, bottom boot block)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount2MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the bottom boot block is partitioned into 4
							  //  sectors -- 16-8-8-32)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkBotBoot16_8_8_32 // device geometry -- flashAMDDevGeom...
	},

	{
	"Fujitsu MBM29LV160TE",	  // a part name string
	flashManuFujitsu,		  // manufacturer id (flashManu...)
	flashDevFuj16MbBT,		  // Fujitsu MBM29LV160TE (16 megabits, top boot block)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount2MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the top block is partitioned into 4
							  //  sectors -- 32-8-8-16)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkTopBoot32_8_8_16 // device geometry -- flashAMDDevGeom...
	},


	{
	"Toshiba TC58FYB160",	  // a part name string
	flashManuToshiba,		  // manufacturer id (flashManu...)
	flashDevTosh16MbBB,		  // Toshiba TC58FYB160 (16 megabits, bottom boot block)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount2MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the bottom boot block is partitioned into 4
							  //  sectors -- 16-8-8-32)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkBotBoot16_8_8_32 // device geometry -- flashAMDDevGeom...
	},

	{
	"Hyundai HY29LV160BT",	 // a part name string
	flashManuHyundai,		  // manufacturer id (flashManu...)
	flashDevHY16MbBB,		  // Hyundai HY29LV160BT (16 megabits, bottom boot block)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)((prvByteCount2MB / prvByteCount64KB) - 1 + 4), // number of sectors
							  // (the bottom boot block is partitioned into 4
							  //  sectors -- 16-8-8-32)
	prvByteCount64KB,		  // byte size of the largest flash sector
	flashAMDDevGeom64KBlkBotBoot16_8_8_32 // device geometry -- flashAMDDevGeom...
	},


	{
	"SST SST32HF802",		  // a part name string
	flashManuSST,			  // manufacturer id (flashManu...)
	flashDevSST32HF802,		  // SST32HF802 (8 megabits, 4KByte sectors)
	prvByteCount1MB,		  // size of the flash chip in BYTEs
	(UInt16)(prvByteCount1MB / prvByteCount4KB), // number of sectors
							  // (the flash device is partitioned into
							  //  uniform 4KB sectors)
	prvByteCount4KB,		  // byte size of the largest flash sector
	flashAMDDevGeom4KBlkUniform // device geometry -- flashAMDDevGeom...
	},


	{
	"SST SST32HF162/164",	  // a part name string
	flashManuSST,			  // manufacturer id (flashManu...)
	flashDevSST32HF162_164,	  // SST32HF162 or 164 (16 megabits, 4KB sectors)
	prvByteCount2MB,		  // size of the flash chip in BYTEs
	(UInt16)(prvByteCount2MB / prvByteCount4KB), // number of sectors
							  // (the flash device is partitioned into
							  //  uniform 64KB blocks)
	prvByteCount4KB,		  // byte size of the largest flash sector
	flashAMDDevGeom4KBlkUniform // device geometry -- flashAMDDevGeom...
	}





}; // end of DeviceDescriptions



//=============================================================================
//  Device-specific code -- add device-specific routines here
//=============================================================================


/***************************************************************
 *	Function:	Prv64KBlkBotBoot16_8_8_32GetSectorInfo
 *
 *	Summary:
 *		Given a sector index, computes its byte offset and size.
 *
 *		This function supports the following bottom boot block (16-8-8-32)
 *		 devices:
 *
 *		  AMD Am29LV160BB
 *		  AMD Am29LV800B
 *		  Fujitsu MBM29LV160B (Am29LV160BB-compatible)
 *		  Toshiba TC58FYB160 (Am29LV160BB-compatible)
 *
 *		It *should* be able to support any 64KB-sector, bottom boot block (16-8-8-32)
 *		 devices regardless of the overall chip size.
 *
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		infoP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *		DOLATER... this operation could be optimized via a look-up table.
 *
 *
 *	History:
 *    13-Apr-2000	VMK		Created
 *
 ****************************************************************/
static DWord
Prv64KBlkBotBoot16_8_8_32GetSectorInfo (const struct FlashAMDDeviceTableEntryType* descP,
								   FlashSectorInfoType* infoP)
{
  DWord	  err = 0;


  // Error-check args
  ErrNonFatalDisplayIf (!descP || !infoP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount64KB, "bad maxSectorBytes");

  // Validate the sector index against our globals
  if (infoP->index >= descP->numSectors)
	  {
	  ErrNonFatalDisplay ("sector index out of bounds");
	  err = flashErrBadArg;
	  goto Exit;
	  }

  // Calculate the sector offset and size, keeping in mind that the boot block
  //  configuration is 16-8-8-32

  if (infoP->index > 3)
	  {
	  infoP->size = prvByteCount64KB;
	  infoP->offset = (infoP->index - 3) * prvByteCount64KB;
	  }
  else if (infoP->index == 0)
	  {
	  infoP->size = (16UL * 1024UL);
	  infoP->offset = 0;
	  }
  else if (infoP->index == 1)
	  {
	  infoP->size = (8UL * 1024UL);
	  infoP->offset = (16UL * 1024UL);
	  }
  else if (infoP->index == 2)
	  {
	  infoP->size = (8UL * 1024UL);
	  infoP->offset = (24UL * 1024UL);
	  }
  else	// infoP->index == 3
	  {
	  infoP->size = (32UL * 1024UL);
	  infoP->offset = (32UL * 1024UL);
	  }

  ErrNonFatalDisplayIf (infoP->size > descP->maxSectorBytes,
						"infoP->size out of bounds");
  ErrNonFatalDisplayIf (infoP->offset > (descP->numBytes - prvByteCount64KB),
						"infoP->offset out of bounds");

Exit:
  return (err);

} // Prv64KBlkBotBoot16_8_8_32GetSectorInfo


/***************************************************************
 *	Function:	Prv64KBlkBotBoot16_8_8_32GetSectorIndex
 *
 *	Summary:
 *		Given a byte offset from beginning of flash device, computes
 *		the index of the sector that contains this byte offset.
 *
 *		This function supports the following bottom boot block (16-8-8-32)
 *		 devices:
 *
 *		  AMD Am29LV160BB
 *		  AMD Am29LV800B
 *		  Fujitsu MBM29LV160B (Am29LV160BB-compatible)
 *		  Toshiba TC58FYB160 (Am29LV160BB-compatible)
 *
 *		It *should* be able to support any 64KB-sector, bottom boot block (16-8-8-32)
 *		 devices regardless of the overall chip size.
 *
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		queryP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    13-Apr-2000	VMK		Created
 *
 ****************************************************************/
static DWord
Prv64KBlkBotBoot16_8_8_32GetSectorIndex (const struct FlashAMDDeviceTableEntryType* descP,
										 FlashSectorIndexQueryType* queryP)
{
  DWord	  err = 0;


  ErrNonFatalDisplayIf (!descP || !queryP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount64KB, "bad maxSectorBytes");
  ErrNonFatalDisplayIf (queryP->offset >= descP->numBytes, "queryP->offset out of bounds");

  // Compute sector index, keeping in mind that the boot block configuration is
  //  16-8-8-32
  queryP->index = queryP->offset / prvByteCount64KB;  // the compiler should
													  //  optimize this to a
													  //  shift-right operation
  if (queryP->index > 0)
	{
	  // We're in a sector after the boot block, so compensate for the boot sectors
	  queryP->index += (4 - 1);
	}
  else
	{
	  // We're in the boot block -- do a range comparison
	  if (queryP->offset < (16UL * 1024UL))
		  queryP->index = 0;
	  else if (queryP->offset < (24UL * 1024UL))
		  queryP->index = 1;
	  else if (queryP->offset < (32UL * 1024UL))
		  queryP->index = 2;
	  else
		  queryP->index = 3;
	}

  // Now, validate the result against our globals
  if (queryP->index >= descP->numSectors)
	{
	  ErrNonFatalDisplay ("index out of bounds");
	  err = flashErrBadArg;
	}


  return (err);

} // Prv64KBlkBotBoot16_8_8_32GetSectorIndex



/***************************************************************
 *	Function:	Prv64KBlkTopBoot32_8_8_16GetSectorInfo
 *
 *	Summary:
 *		Given a sector index, computes its byte offset and size.
 *
 *		This function supports the following top boot block (32-8-8-16)
 *		 devices:
 *
 *		  AMD Am29LV160BT
 *		  Fujitsu MBM29LV160TE
 *
 *		It *should* be able to support any 64KB-sector, top boot block (32-8-8-16)
 *		 devices regardless of the overall chip size.
 *
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		infoP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *		DOLATER... this operation could be optimized via a look-up table.
 *
 *
 *	History:
 *    17-Apr-2000	VMK		Created
 *
 ****************************************************************/
static DWord
Prv64KBlkTopBoot32_8_8_16GetSectorInfo (const struct FlashAMDDeviceTableEntryType* descP,
								   FlashSectorInfoType* infoP)
{
  DWord	  err = 0;
  UInt16  atEnd;	// # of sectors between the passed in sector index and end
					//  of chip


  // Error-check args
  ErrNonFatalDisplayIf (!descP || !infoP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount64KB, "bad maxSectorBytes");

  // Validate the sector index against our globals
  if (infoP->index >= descP->numSectors)
	  {
	  ErrNonFatalDisplay ("sector index out of bounds");
	  err = flashErrBadArg;
	  goto Exit;
	  }

  // Calculate the sector offset and size, keeping in mind that the boot block
  //  configuration is 32-8-8-16

  atEnd = descP->numSectors - (UInt16)infoP->index;

  if (atEnd > 4)
	  {
	  // We're in a sector before the boot block -- this is simple
	  infoP->size = prvByteCount64KB;
	  infoP->offset = infoP->index * prvByteCount64KB;
	  }
  else if (atEnd == 1)
	  {
	  // 4th (i.e. last) boot block sector
	  infoP->size = (16UL * 1024UL);
	  infoP->offset = descP->numBytes - (16UL * 1024UL);
	  }
  else if (atEnd == 2)
	  {
	  // 3rd boot block sector
	  infoP->size = (8UL * 1024UL);
	  infoP->offset = descP->numBytes - ((16UL + 8UL) * 1024UL);
	  }
  else if (atEnd == 3)
	  {
	  // 2nd boot block sector
	  infoP->size = (8UL * 1024UL);
	  infoP->offset = descP->numBytes - ((16UL + 8UL + 8UL) * 1024UL);
	  }
  else	// atEnd == 4
	  {
	  // 1st boot block sector
	  infoP->size = (32UL * 1024UL);
	  infoP->offset = descP->numBytes - ((16UL + 8UL + 8UL + 32UL) * 1024UL);
	  }

  ErrNonFatalDisplayIf (infoP->size > descP->maxSectorBytes,
						"infoP->size out of bounds");
  ErrNonFatalDisplayIf (infoP->offset > (descP->numBytes - prvByteCount16KB),
						"infoP->offset out of bounds");

Exit:
  return (err);

} // Prv64KBlkTopBoot32_8_8_16GetSectorInfo



/***************************************************************
 *	Function:	Prv64KBlkTopBoot32_8_8_16GetSectorIndex
 *
 *	Summary:
 *		Given a byte offset from beginning of flash device, computes
 *		the index of the sector that contains this byte offset.
 *
 *		This function supports the following top boot block (32-8-8-16)
 *		 devices:
 *
 *		  AMD Am29LV160BT
 *		  Fujitsu MBM29LV160TE
 *
 *		It *should* be able to support any 64KB-sector, top boot block (32-8-8-16)
 *		 devices regardless of the overall chip size.
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		queryP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    17-Apr-2000	VMK		Created
 *
 ****************************************************************/
static DWord
Prv64KBlkTopBoot32_8_8_16GetSectorIndex (const struct FlashAMDDeviceTableEntryType* descP,
										 FlashSectorIndexQueryType* queryP)
{
  DWord	  err = 0;
  DWord	  bytesAtEnd;	// # of bytes between the passed offset and end
						//  of chip


  ErrNonFatalDisplayIf (!descP || !queryP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount64KB, "bad maxSectorBytes");
  ErrNonFatalDisplayIf (queryP->offset >= descP->numBytes, "queryP->offset out of bounds");


  // Compute sector index, keeping in mind that the boot block configuration is
  //  32-8-8-16

  bytesAtEnd = descP->numBytes - queryP->offset;

  // If this is before the boot block, then we're in a regular sector
  if (bytesAtEnd > prvByteCount64KB)
	{
	  // We're in a sector before the boot block -- this is simple
	  queryP->index = queryP->offset / prvByteCount64KB;  // the compiler should
														  //  optimize this to a
														  //  shift-right operation
	}
  else
	{
	  // We're in the boot block -- do a range comparison
	  if (bytesAtEnd <= (16UL * 1024UL))
		{
		  queryP->index = descP->numSectors - 1;
		}
	  else if (bytesAtEnd <= ((16UL + 8UL) * 1024UL))
		{
		  queryP->index = descP->numSectors - 2;
		}
	  else if (bytesAtEnd <= ((16UL + 8UL + 8UL) * 1024UL))
		{
		  queryP->index = descP->numSectors - 3;
		}
	  else
		{
		  queryP->index = descP->numSectors - 4;
		}
	}

  // Now, validate the result against our globals
  if (queryP->index >= descP->numSectors)
	{
	  ErrNonFatalDisplay ("index out of bounds");
	  err = flashErrBadArg;
	}


  return (err);

} // Prv64KBlkTopBoot32_8_8_16GetSectorIndex



/***************************************************************
 *	Function:	Prv4KBlkUniformGetSectorInfo
 *
 *	Summary:
 *		Given a sector index, computes its byte offset and size.
 *
 *		This function supports devices with uniform 4KByte sectors,
 *		regardless of the overall chip size.
 *
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		infoP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *	History:
 *    17-Apr-2001	VMK		Created
 *
 ****************************************************************/
static DWord
Prv4KBlkUniformGetSectorInfo (const struct FlashAMDDeviceTableEntryType* descP,
								   FlashSectorInfoType* infoP)
{
  DWord	  err = 0;

  // Error-check args
  ErrNonFatalDisplayIf (!descP || !infoP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount4KB, "bad maxSectorBytes");

  // Validate the sector index against our globals
  if (infoP->index >= descP->numSectors)
	  {
	  ErrNonFatalDisplay ("sector index out of bounds");
	  err = flashErrBadArg;
	  goto Exit;
	  }

  // Calculate the sector offset and size

  // We don't have any boot blocks -- this is simple
  infoP->size = prvByteCount4KB;
  infoP->offset = infoP->index * prvByteCount4KB;

  ErrNonFatalDisplayIf (infoP->size > descP->maxSectorBytes,
						"infoP->size out of bounds");
  ErrNonFatalDisplayIf (infoP->offset > (descP->numBytes - prvByteCount4KB),
						"infoP->offset out of bounds");

Exit:
  return (err);

} // Prv4KBlkUniformGetSectorInfo



/***************************************************************
 *	Function:	Prv4KBlkUniformGetSectorIndex
 *
 *	Summary:
 *		Given a byte offset from beginning of flash device, computes
 *		the index of the sector that contains this byte offset.
 *
 *		This function supports devices with uniform 4KByte sectors,
 *		regardless of the overall chip size.
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		queryP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    17-Apr-2001	VMK		Created
 *
 ****************************************************************/
static DWord
Prv4KBlkUniformGetSectorIndex (const struct FlashAMDDeviceTableEntryType* descP,
										 FlashSectorIndexQueryType* queryP)
{
  DWord	  err = 0;


  ErrNonFatalDisplayIf (!descP || !queryP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount4KB, "bad maxSectorBytes");
  ErrNonFatalDisplayIf (queryP->offset >= descP->numBytes, "queryP->offset out of bounds");


  // Compute sector index

  // We don't have any boot blocks -- this is simple
  queryP->index = queryP->offset / prvByteCount4KB;  // the compiler should
														  //  optimize this to a
														  //  shift-right operation

  // Now, validate the result against our globals
  if (queryP->index >= descP->numSectors)
	{
	  ErrNonFatalDisplay ("index out of bounds");
	  err = flashErrBadArg;
	}


  return (err);

} // Prv4KBlkUniformGetSectorIndex



//=============================================================================
// This is the support logic -- no need to modify this
//=============================================================================


/***************************************************************
 *	Function:	PrvDeviceDescriptionTableValidate
 *
 *	Summary:
 *		Validates the device description table.  Throws a Fatal Error
 *		if programmer errors are found.
 *
 *		Performs rudimentary validation on each table entry as well
 *		as checks for duplicate entries (i.e. entries that have the
 *		same manufacturer/device id pairs)
 *
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		nothing;
 *
 *	Called By:
 *		FlashAMDDeviceTableInit
 *
 *	Notes:
 *
 *
 *	History:
 *    17-Apr-2000	VMK		Created
 *
 ****************************************************************/
static void
PrvDeviceDescriptionTableValidate (void)
{
  const FlashAMDDeviceTableEntryType* tableP = DeviceDescriptions;
  const FlashAMDDeviceTableEntryType* endP = &tableP[prvDeviceDescEntryCount];
  const FlashAMDDeviceTableEntryType* topP = NULL;
  const FlashAMDDeviceTableEntryType* curP = NULL;

  // Check for duplicates and rudimentary data errors
  // (these errors are treated as fatal because they indicate
  //  data integrity problems)
  for (topP = tableP; topP < endP; topP++)
	{
	  // A bit of rudimentary error checking of the entry
	  ErrFatalDisplayIf (topP->partName[0] == 0, "empty partName");
	  ErrFatalDisplayIf (topP->numBytes == 0, "bad numBytes");
	  ErrFatalDisplayIf (topP->numSectors == 0, "bad numSectors");
	  ErrFatalDisplayIf (topP->maxSectorBytes == 0, "bad maxSectorBytes");
	  ErrFatalDisplayIf (topP->maxSectorBytes > topP->numBytes,
						 "maxSectorBytes > numBytes");
	  ErrFatalDisplayIf (topP->devGeometry <= flashAMDDevGeomFIRST
						 || topP->devGeometry >= flashAMDDevGeomLAST,
						 "devGeom out of bounds");

	  // Check the table for duplicates -- all entries must have unique
	  //  manufId/deviceId pairs
	  for (curP = topP + 1; curP < endP; curP++)
		{
		  if (curP->manufId == topP->manufId && curP->deviceId == topP->deviceId)
			ErrFatalDisplay ("duplicate device description found");
		}
	}

  return;
} // PrvDeviceDescriptionTableValidate





/***************************************************************
 *	Function:	FlashAMDDeviceTableInit
 *
 *	Summary:
 *		Validates the device description table.  Throws a Fatal Error
 *		if programmer errors are found.
 *
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		nothing;
 *
 *	Called By:
 *		PrvFlashAMDOpen in FlashDriverAMD.c
 *
 *	Notes:
 *
 *
 *	History:
 *    17-Apr-2000	VMK		Created
 *
 ****************************************************************/
void
FlashAMDDeviceTableInit (void)
{

  //---------------------------------------------------------------------------
  // Validate the device description array
  //---------------------------------------------------------------------------
  PrvDeviceDescriptionTableValidate ();


  return;

} // FlashAMDDeviceTableInit



/***************************************************************
 *	Function:	FlashAMDDeviceTableEntryFind
 *
 *	Summary:
 *		Given a device description table and manufacturer/device IDs,
 *		searches the table for a matching device description.
 *
 *
 *	Parameters:
 *		manufId		IN		flash manufacturer ID
 *		deviceId	IN		flash device ID
 *		errP		OUT		[optional] zero if found
 *
 *	Returns:
 *		ptr to device entry or NULL if not found;
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    13-Apr-2000	VMK		Created
 *
 ****************************************************************/
const FlashAMDDeviceTableEntryType*
FlashAMDDeviceTableEntryFind (Byte manufId, Byte deviceId, DWord* errP)
{
  DWord	  err = flashErrUnknownManuf;
  const FlashAMDDeviceTableEntryType* tableP = DeviceDescriptions;
  const FlashAMDDeviceTableEntryType* endP = &tableP[prvDeviceDescEntryCount];
  const FlashAMDDeviceTableEntryType* entryP = NULL;


  // Search the table for the given device
  for ( ; tableP < endP; tableP++)
	{
	  if (tableP->manufId == manufId)
		{
  		  err = flashErrUnknownDevice;

		  if (tableP->deviceId == deviceId)
			{
			  entryP = tableP;	// found our entry
			  err = 0;
			  break;			// ==>
			}
		}
	}


  if (errP)
	*errP = err;

  return (entryP);

} // FlashAMDDeviceTableEntryFind



/***************************************************************
 *	Function:	FlashAMDGetSectorInfo
 *
 *	Summary:
 *		Given a sector index, computes its byte offset and size.
 *
 *		This API function calls the appropriate helper function based
 *		on devType in the device description.
 *
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		infoP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *	History:
 *    17-Apr-2000	VMK		Created
 *
 ****************************************************************/
DWord
FlashAMDGetSectorInfo (const struct FlashAMDDeviceTableEntryType* descP,
										  FlashSectorInfoType* infoP)
{
  DWord	  err = 0;

  ErrNonFatalDisplayIf (!descP, "null arg");


  if (descP->devGeometry == flashAMDDevGeom64KBlkBotBoot16_8_8_32)
	{
	  err = Prv64KBlkBotBoot16_8_8_32GetSectorInfo (descP, infoP);
	}

  else if (descP->devGeometry == flashAMDDevGeom64KBlkTopBoot32_8_8_16)
	{
	  err = Prv64KBlkTopBoot32_8_8_16GetSectorInfo (descP, infoP);
	}

  else if (descP->devGeometry == flashAMDDevGeom4KBlkUniform)
	{
	  err = Prv4KBlkUniformGetSectorInfo (descP, infoP);
	}

  else
	{
	  // This error constitutes programmer error -- it usually means that
	  //  a new device type was added to the device description table,
	  //  but this function was not updated to support it
	  ErrFatalDisplay ("unsupported devGeometry");
	  err = flashErrBadArg;
	}


  return (err);

} // FlashAMDGetSectorInfo




/***************************************************************
 *	Function:	FlashAMDGetSectorIndex
 *
 *	Summary:
 *		Given a byte offset from beginning of flash device, computes
 *		the index of the sector that contains this byte offset.
 *
 *		This API function calls the appropriate helper function based
 *		on devType in the device description.
 *
 *
 *	Parameters:
 *		descP		IN		flash device description
 *		queryP		IN/OUT	query parameters
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Other AMD Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    17-Apr-2000	VMK		Created
 *
 ****************************************************************/
DWord
FlashAMDGetSectorIndex (const struct FlashAMDDeviceTableEntryType* descP,
										  FlashSectorIndexQueryType* queryP)
{
  DWord	  err = 0;

  ErrNonFatalDisplayIf (!descP, "null arg");


  if (descP->devGeometry == flashAMDDevGeom64KBlkBotBoot16_8_8_32)
	{
	  err = Prv64KBlkBotBoot16_8_8_32GetSectorIndex (descP, queryP);
	}

  else if (descP->devGeometry == flashAMDDevGeom64KBlkTopBoot32_8_8_16)
	{
	  err = Prv64KBlkTopBoot32_8_8_16GetSectorIndex (descP, queryP);
	}

  else if (descP->devGeometry == flashAMDDevGeom4KBlkUniform)
	{
	  err = Prv4KBlkUniformGetSectorIndex (descP, queryP);
	}

  else
	{
	  // This error constitutes programmer error -- it usually means that
	  //  a new device type was added to the device description table,
	  //  but this function was not updated to support it
	  ErrFatalDisplay ("unsupported devGeometry");
	  err = flashErrBadArg;
	}


  return (err);

} // FlashAMDGetSectorIndex
