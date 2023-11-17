/***************************************************************
 *
 * Project:
 *    Ken Expansion Card
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999-2000.
 *
 * FileName:
 *    FlashDriverIntelDevTable.c
 *
 * Description:
 *    This file contains the flash device description data and code for
 *	   flash devices that are programatically compatible with Intel E28F640J3;
 *
 *	  This includes:
 *
 *				Intel E28F320J3/E28F640J3/E28F132J3
 *
 * Adding support for new flash devices:
 *
 *	It should be fairly easy to add support for new flash devices that
 *	 use the same programming algorithms as Intel E28F640J3. If your flash
 *	 device has the same basic geometry (base sector size, relative boot
 *	 block location -- top or bottom -- and layout of the boot block)
 *	 as one of the supported flash devices (see DeviceDescriptions array
 *	 in this source file), then you only need to add a new entry to the
 *	 DeviceDescriptions array. To do this:
 *		1. Define macros for your flash device's manufacturer id and
 *			device id in the header file FlashDriverCommon.h.
 *		2. Add an entry for your device to the DeviceDescriptions array
 *			in this source file -- FlashDriverIntelDevTable.c, using an
 *			existing entry as a template.
 *		3. Recompile any necessary code and test against your flash device.
 *
 *	 If your flash device's basic geometry differs from those that are
 *	  already supported, then you will need to:
 *		1. Define macros for your flash device's manufacturer id and
 *			device id in the header file FlashDriverCommon.h.
 *		2. Add a new device geometry enum for your device to FlashIntelDevGeomEnum in
 *			FlashDriverIntelPrv.h.
 *		3. Add an entry for your device to the DeviceDescriptions array
 *			in this source file -- FlashDriverIntelDevTable.c.
 *		4. Create two new functions in this source file -- one for computing
 *			sector index (see Prv128KBBoot128GetSectorIndex), and
 *			another for computing sector offset and size (see
 *			Prv128KBBoot128GetSectorInfo).
 *		5. Update the functions FlashIntelGetSectorInfo and FlashIntelGetSectorIndex
 *			(in this source file) with an "else if" case for your device type.
 *		6. Recompile any necessary code and test against your flash device.
 *
 *
 * ToDo:
 *
 * History:
 *    14-Apr-2000 - Created by Vitaly Kruglikov
 *
 ****************************************************************/
#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>


#include "FlashDriver.h"
#include "FlashDriverCommon.h"
#include "FlashDriverIntelPrv.h"


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


#define prvDeviceDescEntryCount	  \
		  sizeof(DeviceDescriptions) / sizeof(DeviceDescriptions[0])

#pragma pcrelconstdata on

// NOTE:
// All entries MUST have unique manufId/deviceId pairs; empty part name strings
//  are NOT allowed;
static const FlashIntelDeviceTableEntryType DeviceDescriptions[] = {

	{
	"Intel E28F320J3",		  // a part name string
	flashManuIntel,			  // manufacturer id (flashManu...)
	flashDevIntel32Mb,		  // Intel E28F320J3A (32 megabits)
	prvByteCount4MB,		  // size of the flash chip in BYTEs
	(UInt16)(prvByteCount4MB / prvByteCount128KB), // number of sectors
	prvByteCount128KB,		  // byte size of the largest flash sector
	flashIntelDevGeom128KBBoot128 // device geometry -- flashIntelDevGeom...
	},

	{
	"Intel E28F640J3",		  // a part name string
	flashManuIntel,			  // manufacturer id (flashManu...)
	flashDevIntel64Mb,	      // Intel E28F640J3A (64 megabits\)
	prvByteCount8MB,		  // size of the flash chip in BYTEs
	(UInt16)(prvByteCount8MB / prvByteCount128KB), // number of sectors
	prvByteCount128KB,		  // byte size of the largest flash sector
	flashIntelDevGeom128KBBoot128 // device geometry -- flashIntelDevGeom...
	},

	{
	"Intel E28F128J3",		  // a part name string
	flashManuIntel,			  // manufacturer id (flashManu...)
	flashDevIntel128Mb,	      // Intel E28F128J3A (128 megabits)
	prvByteCount16MB,		  // size of the flash chip in BYTEs
	(UInt16)(prvByteCount16MB / prvByteCount128KB), // number of sectors
	prvByteCount128KB,		  // byte size of the largest flash sector
	flashIntelDevGeom128KBBoot128 // device geometry -- flashIntelDevGeom...
	}



}; // end of DeviceDescriptions



//=============================================================================
//  Device-specific code -- add device-specific routines here
//=============================================================================


/***************************************************************
 *	Function:	Prv128KBBoot128GetSectorInfo
 *
 *	Summary:
 *		Given a sector index, computes its byte offset and size.
 *
 *		This function supports the following devices:
 *
 *		  Intel E28F320J3
 *		  Intel E28F640J3
 *		  Intel E28F128J3
 *
 *		It *should* be able to support any 128KB-sector
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
 *		Other Intel Flash Driver functions
 *
 *	Notes:
 *		DOLATER... this operation could be optimized via a look-up table.
 *
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
static DWord
Prv128KBBoot128GetSectorInfo (const struct FlashIntelDeviceTableEntryType* descP, FlashSectorInfoType* infoP)
{
  DWord	  err = 0;


  // Error-check args
  ErrNonFatalDisplayIf (!descP || !infoP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount128KB, "bad maxSectorBytes");

  // Validate the sector index against our globals
  if (infoP->index >= descP->numSectors)
	  {
	  ErrNonFatalDisplay ("sector index out of bounds");
	  err = flashErrBadArg;
	  goto Exit;
	  }

  // Calculate the sector offset and size

  infoP->size = prvByteCount128KB;
  infoP->offset = infoP->index * prvByteCount128KB;

  ErrNonFatalDisplayIf (infoP->size > descP->maxSectorBytes,
						"infoP->size out of bounds");
  ErrNonFatalDisplayIf (infoP->offset > (descP->numBytes - prvByteCount128KB),
						"infoP->offset out of bounds");

Exit:
  return (err);

} // Prv128KBBoot128GetSectorInfo


/***************************************************************
 *	Function:	Prv128KBBoot128GetSectorIndex
 *
 *	Summary:
 *		Given a byte offset from beginning of flash device, computes
 *		the index of the sector that contains this byte offset.
 *
 *		This function supports the following devices:
 *
 *		  Intel E28F320J3
 *		  Intel E28F640J3
 *		  Intel E28F128J3
 *
 *		It *should* be able to support any 128KB-sector
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
 *		Other Intel Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
static DWord
Prv128KBBoot128GetSectorIndex (const struct FlashIntelDeviceTableEntryType* descP, FlashSectorIndexQueryType* queryP)
{
  DWord	  err = 0;


  ErrNonFatalDisplayIf (!descP || !queryP, "null arg");
  ErrNonFatalDisplayIf (descP->maxSectorBytes != prvByteCount128KB, "bad maxSectorBytes");
  ErrNonFatalDisplayIf (queryP->offset >= descP->numBytes, "queryP->offset out of bounds");

  // Compute sector index, keeping in mind that the boot block configuration is
  //  16-8-8-32
  queryP->index = queryP->offset / prvByteCount128KB;  // the compiler should
													  //  optimize this to a
													  //  shift-right operation

  // Now, validate the result against our globals
  if (queryP->index >= descP->numSectors)
	{
	  ErrNonFatalDisplay ("offset out of bounds");
	  err = flashErrBadArg;
	}


  return (err);

} // Prv128KBBoot128GetSectorIndex


//=============================================================================
// This is the support logic -- no need to modify this
//=============================================================================


/***************************************************************
 *	Function:	PrvDeviceIntelDescriptionTableValidate
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
 *		FlashIntelDeviceTableInit
 *
 *	Notes:
 *
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
static void
PrvDeviceIntelDescriptionTableValidate (void)
{
  const FlashIntelDeviceTableEntryType* tableP = DeviceDescriptions;
  const FlashIntelDeviceTableEntryType* endP = &tableP[prvDeviceDescEntryCount];
  const FlashIntelDeviceTableEntryType* topP = NULL;
  const FlashIntelDeviceTableEntryType* curP = NULL;

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
	  ErrFatalDisplayIf (topP->devGeometry <= flashIntelDevGeomFIRST
						 || topP->devGeometry >= flashIntelDevGeomLAST,
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
} // PrvDeviceIntelDescriptionTableValidate





/***************************************************************
 *	Function:	FlashIntelDeviceTableInit
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
 *		PrvFlashIntelOpen in FlashDriverIntel.c
 *
 *	Notes:
 *
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
void
FlashIntelDeviceTableInit (void)
{

  //---------------------------------------------------------------------------
  // Validate the device description array
  //---------------------------------------------------------------------------
  PrvDeviceIntelDescriptionTableValidate ();


  return;

} // FlashIntelDeviceTableInit



/***************************************************************
 *	Function:	FlashIntelDeviceTableEntryFind
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
 *		Other Intel Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
const FlashIntelDeviceTableEntryType*
FlashIntelDeviceTableEntryFind (Byte manufId, Byte deviceId, DWord* errP)
{
  DWord	  err = flashErrUnknownManuf;
  const FlashIntelDeviceTableEntryType* tableP = DeviceDescriptions;
  const FlashIntelDeviceTableEntryType* endP = &tableP[prvDeviceDescEntryCount];
  const FlashIntelDeviceTableEntryType* entryP = NULL;


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

} // FlashIntelDeviceTableEntryFind



/***************************************************************
 *	Function:	FlashIntelGetSectorInfo
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
 *		Other Intel Flash Driver functions
 *
 *	Notes:
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
DWord
FlashIntelGetSectorInfo (const struct FlashIntelDeviceTableEntryType* descP, FlashSectorInfoType* infoP)
{
  DWord	  err = 0;

  ErrNonFatalDisplayIf (!descP, "null arg");


  if (descP->devGeometry == flashIntelDevGeom128KBBoot128)
	{
	  err = Prv128KBBoot128GetSectorInfo (descP, infoP);
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

} // FlashIntelGetSectorInfo




/***************************************************************
 *	Function:	FlashIntelGetSectorIndex
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
 *		Other Intel Flash Driver functions
 *
 *	Notes:
 *
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
DWord
FlashIntelGetSectorIndex (const struct FlashIntelDeviceTableEntryType* descP, FlashSectorIndexQueryType* queryP)
{
  DWord	  err = 0;

  ErrNonFatalDisplayIf (!descP, "null arg");


  if (descP->devGeometry == flashIntelDevGeom128KBBoot128)
	{
	  err = Prv128KBBoot128GetSectorIndex (descP, queryP);
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

} // FlashIntelGetSectorIndex
