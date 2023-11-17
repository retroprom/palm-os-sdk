/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashDriverIntel.c
 * 
 * Description:
 *    This file contains the flash driver code for:
 *
 *				Intel INTEL E28F640J3
 *
 * ToDo:
 *
 * History:
 *    03-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>


#include "FlashDriver.h"
#include "FlashDriverCommon.h"
#include "FlashDriverIntelPrv.h"




// Must define entry routine so Metrowerks compiler doesn't complain...

#define start	__Startup__			// build as stand-alone code resource

DWord
start (UInt32 selId, void* argP, UInt32 argSize);
FlashEntryFuncType	start;


FlashEntryFuncType			FlashIntelEntryPoint;



/***********************************************************************
 *
 *  FUNCTION:    start
 *
 *  DESCRIPTION:  GCC needs the entry point at the start of the code
 *	  resource. 
 *
 ***********************************************************************/
DWord
start (UInt32 selId, void* argP, UInt32 argSize)
{
	return FlashIntelEntryPoint (selId, argP, argSize);
}


//------------------------------------------------------------------------
// Private macro declarations
//------------------------------------------------------------------------

#if (ERROR_CHECK_LEVEL == ERROR_CHECK_FULL)

	#define PrvDbgValidateGlobals(gP)	PrvErrorCheckGlobals(gP)

#else	// not full error checking

	#define PrvDbgValidateGlobals(gP)

#endif	// not full error checking





//------------------------------------------------------------------------
// Private types
//------------------------------------------------------------------------



//------------------------------------------------------------------------
// Private function prototypes
//------------------------------------------------------------------------


// API
static FlashOpenFuncType			PrvFlashIntelOpen;

static FlashCloseFuncType			PrvFlashIntelClose;

static FlashAttrGetFuncType			PrvFlashIntelAttrGet;

static FlashExistsFuncType			PrvFlashIntelExists;

static FlashSectorEraseFuncType		PrvFlashIntelSectorErase;

static FlashChipWriteFuncType		PrvFlashIntelChipWrite;

static FlashSameDeviceFuncType		PrvFlashIntelSameDevice;


// Helper functions


#if (ERROR_CHECK_LEVEL == ERROR_CHECK_FULL)
  static void
  PrvErrorCheckGlobals (FlashIntelGlobalsType* gP);
#endif


static DWord
PrvGetFlashDeviceID (FlashIntelGlobalsType* gP, void* chipBaseP, Byte* manufP,
					Byte* deviceP);

// <chg 25-apr-00 dia> Only do this in debug builds...
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
  static DWord
  PrvGetSectorInfoFromOffset (FlashIntelGlobalsType* gP, UInt32 offsetFromChipBase,
							  FlashSectorInfoType* sectorInfoP, UInt32* sectorIndexP);
#endif // ERROR_CHECK_LEVEL == ERROR_CHECK_FULL

static const FlashIntelDeviceTableEntryType*
PrvFlashIntelIdentify (FlashIntelGlobalsType* gP, void* chipBaseP, DWord* errP);

static DWord
PrvFlashIntelProgramAPILoad (FlashIntelGlobalsType* gP);

static void
PrvFlashIntelProgramAPIUnload (FlashIntelGlobalsType* gP);


static DWord
PrvFlashIntelAttrGet (void* refP, void* chipBaseP, UInt32 attrId,
			  void* argP, UInt32 argSize);






/***************************************************************
 *	Function:	FlashIntelEntryPoint 
 *
 *	Summary:      
 *		This is the entry point of the Flash Driver code resource.
 *
 *	Parameters:
 *		selId		IN		function selector value (FlashEntrySelEnum)
 *		argP		IN/OUT	pointer to argument structure (type
 *							 and size depend on selId)
 *		argSize		IN		size of argument structure
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
DWord
FlashIntelEntryPoint (UInt32 selId, void* argP, UInt32 argSize)
{
  DWord					err = 0;


  // Handle the request
  switch (selId)
	  {
	  case flashEntrySelGetInterface:
		  {
		  FlashInterfaceType*	apiP;
		  apiP = (FlashInterfaceType*)argP;

		  ErrNonFatalDisplayIf (apiP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(FlashInterfaceType),
								  "wrong arg size");
		  
		  // Initialize to all ones so in case we miss one, the call
		  //  will bus error at this location
		  MemSet(apiP, sizeof(FlashInterfaceType), 0xFF);

		  apiP->FlashEntry		= &FlashIntelEntryPoint;
		  apiP->FlashOpen			= &PrvFlashIntelOpen;
		  apiP->FlashClose		= &PrvFlashIntelClose;
		  apiP->FlashAttrGet		= &PrvFlashIntelAttrGet;
		  apiP->FlashExists		= &PrvFlashIntelExists;
		  apiP->FlashSectorErase	= &PrvFlashIntelSectorErase;
		  apiP->FlashChipWrite	= &PrvFlashIntelChipWrite;
		  apiP->FlashSameDevice	= &PrvFlashIntelSameDevice;
		  }
		  break;

	  default:
		  ErrNonFatalDisplayIf (true, "invalid Flash Driver entry sel id");
		  err = flashErrInvalidSelId;
		  break;
	  }


  return (err);

} // FlashIntelEntryPoint



/***************************************************************
 *	Function:	PrvFlashIntelOpen
 *
 *	Summary:      
 *		Performs an identification check of the flash chip at
 *		chipBaseP, and if it matches this driver, initializes
 *		the driver's private globals and other needed resources.
 *
 *	Parameters:
 *		refPP		OUT		For returning the Flash Driver open
 *							 reference (returned only if error
 *							 code is zero)
 *		codeDbCardNo
 *					IN		Card number of this driver's database
 *		codeDbId	IN		Local id of this driver's database --
 *							 the database is protected and closed
 *							 by the caller; if the driver needs
 *							 to access the database, it should
 *							 open it temporarily, use it, and close
 *							 it before returning to caller;
 *		codeResId	IN		Resource id of this Flash Driver's code
 *							 resource
 *		chipBaseP	IN		Pointer to base of the first flash chip
 *
 *	Returns:
 *		0 if the driver matched the chip and there was no error
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelOpen (void** refPP, UInt16 codeDbCardNo, LocalID codeDbId,
				 UInt16 codeResId, void* chipBaseP)
{
  DWord		err = 0;
  FlashIntelGlobalsType*	gP = NULL;
  const FlashIntelDeviceTableEntryType*	  descP = NULL;


  // Error check args
  ErrNonFatalDisplayIf (refPP == NULL || codeDbId == 0 || chipBaseP == NULL,
						  "null arg");

  // Init return value
  *refPP = NULL;


  
  //-------------------------------------------------------------------------
  // Allocate our globals and identify the chip
  //-------------------------------------------------------------------------

  // Allocate and initialize our private globals
  gP = (FlashIntelGlobalsType*)MemPtrNew (sizeof(FlashIntelGlobalsType));
  if (gP == NULL)
	  {
	  err = flashErrNoMemory;
	  goto Exit;
	  }
  MemSet (gP, sizeof(FlashIntelGlobalsType), 0);

  // Initialize signatures for debugging
  gP->startSignature = flashIntelGlobalsStartSignature;
  gP->endSignature = flashIntelGlobalsEndSignature;

  // Save arguments passed by caller
  gP->common.codeDbCardNo = codeDbCardNo;
  gP->common.codeDbId = codeDbId;
  gP->common.codeResId = codeResId;
  gP->common.flashBaseP = chipBaseP;

  // Initialize the device description table logic
  FlashIntelDeviceTableInit ();

 
  // Load the flash device access API
  err = PrvFlashIntelProgramAPILoad (gP);
  if (err)
	  goto Exit;


  // Identify the flash device and check if we support it
  gP->device.deviceEntryP = PrvFlashIntelIdentify (gP, chipBaseP, &err);
  if (err)
	  goto Exit;


  // We must now have a valid device entry
  ErrNonFatalDisplayIf (!gP->device.deviceEntryP, "null deviceEntryP");


  // Set variables we use all over 
  gP->ticksPerSecond = SysTicksPerSecond();

  // Save device values for access efficiency
  gP->device.identified = true;
  descP = gP->device.deviceEntryP;		// for convenience
  gP->device.manufId = descP->manufId;
  gP->device.deviceId = descP->deviceId;
  gP->device.numBytes = descP->numBytes;
  gP->device.numWords = descP->numBytes / 2;
  gP->device.numSectors = descP->numSectors;
  gP->device.maxSectorBytes = descP->maxSectorBytes;
  gP->device.sharedAttrs = 0; // no "shared" attributes defined yet

Exit:
  // Clean up on error
  if (err)
	  {
	  if (gP != NULL)
		  {
		  PrvFlashIntelClose (gP);
		  gP = NULL;
		  }
	  }


  *refPP = gP;

  return (err);

} // PrvFlashIntelOpen



/***************************************************************
 *	Function:	PrvFlashIntelClose
 *
 *	Summary:      
 *		Closes the flash driver, releasing any system resources that
 *		were allocated while the driver was open.
 *
 *	Parameters:
 *		refP		IN		Flash Driver open reference returned
 *							by PrvFlashIntelOpen()
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelClose (void* refP)
{
  DWord		err = 0;
  FlashIntelGlobalsType*	gP = (FlashIntelGlobalsType*)refP;

  
  // Error-check our globals
  PrvDbgValidateGlobals (gP);

  if (gP == NULL)
	  return (flashErrBadArg);

  // Unload the flash access API
  PrvFlashIntelProgramAPIUnload (gP);

  // Finally, free our private globals chunk
  
  // But first, reset the signatures (for debugging)
  gP->startSignature = flashIntelGlobalsDiscardSignature;
  gP->endSignature = flashIntelGlobalsDiscardSignature;

  MemPtrFree (gP);

  return (err);

} // PrvFlashIntelClose





/***************************************************************
 *	Function:	PrvFlashIntelAttrGet
 *
 *	Summary:      
 *		Returns information about the flash memory or the Flash
 *		Driver.
 *
 *	Parameters:
 *		refP		IN		Flash Driver open reference returned
 *							by PrvFlashIntelOpen()
 *		chipBaseP	IN		Pointer to base of chip
 *		attrId		IN		Attribute to return (FlashAttrEnum)
 *		argP		IN/OUT	pointer to argument structure (type
 *							 and size depend on attrId)
 *		argSize		IN		size of argument structure
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelAttrGet (void* refP, void* chipBaseP, UInt32 attrId,
			  void* argP, UInt32 argSize)
{
  DWord					err = 0;
  FlashIntelGlobalsType*	gP = (FlashIntelGlobalsType*)refP;

  
  // Error-check our globals
  PrvDbgValidateGlobals (gP);


  // Handle the request
  switch (attrId)
	  {
	  case flashAttrChipSize:
		{
		  UInt32*		dataP;
		  dataP = (UInt32*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(UInt32),
								  "wrong arg size");
		  
		  *dataP = gP->device.numBytes; 
		}
		break;

	  case flashAttrMaxSectorSize:
		{
		  UInt32*		dataP;
		  dataP = (UInt32*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(UInt32),
								  "wrong arg size");
		  
		  *dataP = gP->device.maxSectorBytes;
		}
		break;

	  case flashAttrNumSectors:
		{
		  UInt32*		dataP;
		  dataP = (UInt32*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(UInt32),
								  "wrong arg size");
		  
		  *dataP = gP->device.numSectors;
		}
		break;

	  case flashAttrSectorIndex:
		{
		  const FlashIntelDeviceTableEntryType*  entryP = gP->device.deviceEntryP;
		  FlashSectorIndexQueryType*	dataP;
		  dataP = (FlashSectorIndexQueryType*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(FlashSectorIndexQueryType),
								  "wrong arg size");

		  // Use device table-based logic to get the index
		  err = FlashIntelGetSectorIndex (entryP, dataP);
		  ErrNonFatalDisplayIf (err, "FlashIntelGetSectorIndex failed");
		}
		break;

	  case flashAttrSectorInfo:
		{
		  const FlashIntelDeviceTableEntryType*  entryP = gP->device.deviceEntryP;
		  FlashSectorInfoType*	dataP;
		  dataP = (FlashSectorInfoType*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(FlashSectorInfoType),
								  "wrong arg size");

		  // Use device table-based logic to get the info
		  err = FlashIntelGetSectorInfo (entryP, dataP);
		  ErrNonFatalDisplayIf (err, "FlashIntelGetSectorInfo failed");
		}
		break;

	  case flashAttrSectorLock:
		{
		  FlashSectorInfoType	info;
		  const FlashIntelDeviceTableEntryType*  entryP = gP->device.deviceEntryP;
		  FlashSectorLockQueryType*	dataP;
		  dataP = (FlashSectorLockQueryType*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(FlashSectorLockQueryType),
								  "wrong arg size");
		  
		  // Get the sector offset
		  info.index = dataP->index;
		  err = FlashIntelGetSectorInfo (entryP, &info);
		  ErrNonFatalDisplayIf (err, "FlashIntelGetSectorInfo failed");
		  if (err)
			  break;
		  
		  // Get the sector lock status
		  err = gP->program.SectorIsLocked (gP, chipBaseP, info.offset,
											&dataP->locked);
		}
		break;

	  case flashAttrFlashPartNo:
		{
		  FlashPartNoInfoType*	dataP;
		  const FlashIntelDeviceTableEntryType*	entryP;
		  const char*	  nameP = "";
		  Byte	  manufId = 0, deviceId = 0;

		  dataP = (FlashPartNoInfoType*)argP;

		  ErrNonFatalDisplayIf (dataP == NULL, "null arg");
		  ErrNonFatalDisplayIf (argSize != sizeof(FlashPartNoInfoType),
								  "wrong arg size");

		  dataP->manufId = dataP->deviceId = 0;
		  dataP->partNo[0] = '\0';

		  // Attempt to get the flash manfucaturer and device IDs
		  err = PrvGetFlashDeviceID (gP, chipBaseP, &manufId,
					&deviceId);
		  if (err)
			  break;

		  dataP->manufId = manufId;
		  dataP->deviceId = deviceId;

		  // Check if we support it
		  entryP = FlashIntelDeviceTableEntryFind (manufId, deviceId, &err);
		  if (entryP)
			{
			  // Keep FlashIntelDeviceTableEntryFind honest
			  ErrNonFatalDisplayIf (err != 0, "unexpected error code");
			  ErrNonFatalDisplayIf (entryP->manufId != manufId
									|| entryP->deviceId != deviceId,
									"wrong entryP");

			  // Get name pointer
			  nameP = entryP->partName;
			}


		  StrNCopy ((Char*)dataP->partNo, nameP, sizeof (dataP->partNo));
		}
		break;

	  default:
		  ErrNonFatalDisplayIf (true, "invalid Flash Driver attribute id");
		  err = flashErrInvalidAttrId;
		  break;
	  }


  return (err);

} // PrvFlashIntelAttrGet



/***************************************************************
 *	Function:	PrvFlashIntelExists
 *
 *	Summary:      
 *		Checks if a matching flash chip exists at the specified
 *		address.
 *
 *	Parameters:
 *		refP		IN		Flash Driver open reference returned
 *							by PrvFlashIntelOpen()
 *		chipBaseP	IN		Pointer to base of chip
 *
 *	Returns:
 *		0 if supported chip found at chipBaseP, error code otherwise
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelExists (void* refP, void* chipBaseP)
{
  DWord		err = 0;
  FlashIntelGlobalsType*	gP = (FlashIntelGlobalsType*)refP;
  

  // Error check args
  PrvDbgValidateGlobals (gP);
  ErrNonFatalDisplayIf (chipBaseP == NULL, "null arg");


  // Try to identify the flash device
  PrvFlashIntelIdentify (gP, chipBaseP, &err);

  return (err);

} // PrvFlashIntelExists



/***************************************************************
 *	Function:	PrvFlashIntelSectorErase
 *
 *	Summary:      
 *		Erases a sector of flash.
 *
 *	Parameters:
 *		refP		IN		Flash Driver open reference returned
 *							by PrvFlashIntelOpen()
 *		chipBaseP	IN		Pointer to base of chip
 *		sectorBaseP	IN		Pointer to base of sector to erase
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelSectorErase (void* refP, void* chipBaseP, void* sectorBaseP)
{
  DWord		err = 0;
  FlashIntelGlobalsType*	gP = (FlashIntelGlobalsType*)refP;
  UInt32		sectorOffset = ((Byte*)sectorBaseP - (Byte*)chipBaseP);

  
  // Error-check args
  PrvDbgValidateGlobals (gP);
  ErrNonFatalDisplayIf (chipBaseP == NULL || sectorBaseP == NULL,
					  "null arg");

  // Validate the sector address -- it must be within this chip and
  //  on a valid sector boundary
  ErrNonFatalDisplayIf (sectorBaseP < chipBaseP
						  || (Byte*)sectorBaseP >= ((Byte*)chipBaseP
											  + gP->device.numBytes),
					  "erase dest out of range");

  // Make sure that the sector base address is on a valid sector boundary
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
  {
	DWord						dbgErr;
	FlashSectorInfoType		sectorInfo;

	// Get offset of this sector and use it to validate the passed in
	//  sector address
	dbgErr = PrvGetSectorInfoFromOffset (gP, sectorOffset,
							  &sectorInfo, NULL /*sectorIndexP*/);
	ErrNonFatalDisplayIf (dbgErr, "GetSectorInfoFromOffset failed");

	ErrNonFatalDisplayIf (sectorOffset != sectorInfo.offset,
						  "not start of sector");
  }
#endif								// full error checking


  // Now, call our RAM-resident worker function
  err = gP->program.SectorErase (gP, chipBaseP, sectorOffset);
  if (err)
	  goto Exit;


  // Double-check flash erasure
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL

  {
	DWord						dbgErr;
	FlashSectorInfoType			sectorInfo;
	UInt32						wordsLeft;
	Word*						dstWordP;

	// Get offset of this sector and use it to validate the passed in
	//  sector address
	dbgErr = PrvGetSectorInfoFromOffset (gP, sectorOffset,
							  &sectorInfo, NULL /*sectorIndexP*/);
	ErrNonFatalDisplayIf (dbgErr, "GetSectorInfoFromOffset failed");

	wordsLeft = sectorInfo.size / 2;
	dstWordP = (Word*)sectorBaseP;
	while (wordsLeft--)
		{
		ErrNonFatalDisplayIf (*dstWordP++ != 0xFFFF, "erase failed Still some Zeros");
		}
  }

#endif								// full error checking


Exit:
  return (err);

} // PrvFlashIntelSectorErase



/***************************************************************
 *	Function:	PrvFlashIntelChipWrite
 *
 *	Summary:      
 *		Writes data to the flash chip from an external source.
 *		Manages sector boundaries as necessary.  All of the
 *		destination must be in the chip at chipBaseP.
 *
 *		***IMPORTANT***
 *		The destination region is assumed to have been erased by
 *		the caller.  The debug code will validate this assumption
 *		and fail an assertion if this isn't the case.
 *
 *	Parameters:
 *		refP		IN		Flash Driver open reference returned
 *							by PrvFlashIntelOpen()
 *		chipBaseP	IN		Pointer to base of chip
 *		dstP		IN		Pointer to start of destination region
 *		srcP		IN		Pointer to start of source data
 *		numBytes	IN		Number of bytes to write
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelChipWrite (void* refP, void* chipBaseP, void* dstP,
					  void* srcP, UInt32 numBytes)
{
  DWord						err = 0;
  FlashIntelGlobalsType*	gP = (FlashIntelGlobalsType*)refP;
  //UInt32					sectorOffset = ((Byte*)dstP - (Byte*)chipBaseP);
  //FlashSectorInfoType		sectorInfo;

  
  // Error-check args
  PrvDbgValidateGlobals (gP);
  ErrNonFatalDisplayIf (chipBaseP == NULL || dstP == NULL
					  || srcP == NULL, "null arg");
  ErrNonFatalDisplayIf (numBytes > gP->device.numBytes, "too many bytes");
  // Validate the range -- the entire destination range must be within
  //  a single chip pointed to by chipBaseWordP
  ErrNonFatalDisplayIf (dstP < chipBaseP
						  || ((Byte*)dstP + numBytes) > ((Byte*)chipBaseP
													  + gP->device.numBytes),
					  "write dest out of range");

  // Take care of the degenerate case
  if (numBytes == 0 || srcP == dstP)
	  return (0);

  // Now, call our RAM-resident worker function
  err = gP->program.ChipWrite (gP, (Word*)chipBaseP, (Byte*)dstP,
							   (Byte*)srcP, numBytes);


  return (err);

} // PrvFlashIntelChipWrite



/***************************************************************
 *	Function:	PrvIntelSameDevice
 *
 *	Summary:      
 *		Checks whether the two chips are at the same physical
 *		 address.  Due to address wrap-around on the Flash Module,
 *		 what appears as two devices at distinct logical addresses,
 *		 may actuall be a single device.  Returns non-zero if they
 *		 are the same physical device.
 *
 *	Parameters:
 *		gP				IN		Flash Driver's private globals
 *		chip0BaseP		IN		Pointer to base of flash chip #0
 *		chip1BaseP		IN		Pointer to base of flash chip #1
 *
 *	Returns:
 *		non-zero if they are the same physical device.
 *  
 *	Called By: 
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static Byte
PrvFlashIntelSameDevice (void* refP, void* chip0BaseP, void* chip1BaseP)
{
  Byte		same;
  FlashIntelGlobalsType*	gP = (FlashIntelGlobalsType*)refP;

  
  // Error-check args
  PrvDbgValidateGlobals (gP);
  ErrNonFatalDisplayIf (chip0BaseP == NULL || chip1BaseP == NULL, "null arg");
  ErrNonFatalDisplayIf (chip0BaseP == chip1BaseP, "same address");


  // Now, call our RAM-resident worker function
  same = gP->program.SameDevice (gP, chip0BaseP, chip1BaseP);


  return (same);

} // PrvFlashIntelSameDevice



/***************************************************************
 *	Function:	PrvFlashIntelIdentify
 *
 *	Summary:
 *		This function determines whether the given flash device
 *		 is supported by this flash driver:
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *		chipBaseP	IN		Pointer to base of the flash chip
 *		manufP		OUT		For returning manufcturer ID (flashManu...)
 *		deviceP		OUT		For returning device ID (flashDev...)
 *		errP		OUT		0 if the flash device was identified as
 *							 supported by this driver
 *
 *	Returns:
 *		ptr to device entry structure if the flash device was identified
 *		 as supported by this driver, NULL otherwise
 *  
 *	Called By: 
 *		PrvFlashIntelOpen()
 *		PrvFlashIntelExists()
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static const FlashIntelDeviceTableEntryType*
PrvFlashIntelIdentify (FlashIntelGlobalsType* gP, void* chipBaseP, DWord* errP)
{
  const FlashIntelDeviceTableEntryType* entryP = NULL;
  Byte	  manufId = 0, deviceId = 0;


  // Error-check args
  ErrNonFatalDisplayIf (gP == NULL || chipBaseP == NULL || errP == NULL,
						"null arg");


  //-------------------------------------------------------------------------
  // Get the chip manufcturer and device ID
  //-------------------------------------------------------------------------
  *errP = PrvGetFlashDeviceID (gP, chipBaseP, &manufId, &deviceId);
  if (*errP)
	  goto Exit;


  // See if we recognize the manufacturer/device IDs
  entryP = FlashIntelDeviceTableEntryFind (manufId, deviceId, errP);
  if (*errP)
	  goto Exit;

  // Keep FlashIntelDeviceTableEntryFind honest
  ErrNonFatalDisplayIf (!entryP, "null device entry");
  ErrNonFatalDisplayIf (entryP->manufId != manufId
						|| entryP->deviceId != deviceId,
						"wrong entryP");

Exit:
  return (entryP);

} // PrvFlashIntelIdentify



/***************************************************************
 *	Function:	PrvGetFlashDeviceID
 *
 *	Summary:
 *		Attemts to get the device ID:
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *		chipBaseP	IN		Pointer to base of chip
 *		manufP		OUT		For returning manufcturer ID (flashManu...)
 *		deviceP		OUT		For returning device ID (flashDev...)
 *
 *	Returns:
 *		0 if no error getting the manufacturer and device IDs
 *  
 *	Called By: 
 *		internally.
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvGetFlashDeviceID (FlashIntelGlobalsType* gP, void* chipBaseP, Byte* manufP,
					Byte* deviceP)
{
  DWord	  err = 0;


  // Error-check args
  ErrNonFatalDisplayIf (gP == NULL || chipBaseP == NULL || manufP == NULL
						  || deviceP == NULL, "null arg");


  // Initialize return values
  *manufP = *deviceP = 0;

  
  //-------------------------------------------------------------------------
  // Get the chip manufcturer and device ID
  //-------------------------------------------------------------------------

  err = gP->program.GetManufAndDevice (gP, chipBaseP, manufP, deviceP);


  return (err);

} // PrvGetFlashDeviceID


/***************************************************************
 *	Function:	PrvGetSectorInfoFromOffset
 *
 *	Summary:
 *		Convenience function -- given a byte offset to anywhere
 *		within the flash chip (from start of that flash chip),
 *		return information about the sector containing that offset
 *
 *	Parameters:
 *		gP					IN		Our private globals;
 *		offsetFromChipBase	IN		0-based byte offset from start of the
 *									 chip -- does not have to be on
 *									 sector boudary;
 *		sectorInfoP			OUT		For returning sector information;
 *		sectorIndexP		OUT		[OPTIONAL] For returning index of
 *									 the sector (pass NULL to ignore);
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		internally.
 *
 *	Notes:
 *		The target flash chip must have the same geometry as that
 *		identified during FlashOpen; otherwise, returned information
 *		will be unreliable.
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/

// <chg 25-apr-00 dia> Only do this in debug builds, otherwise the compiler
// complains that this function isn't used.
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
static DWord
PrvGetSectorInfoFromOffset (FlashIntelGlobalsType* gP, UInt32 offsetFromChipBase,
							FlashSectorInfoType* sectorInfoP, UInt32* sectorIndexP)
{
  DWord						err = 0;
  FlashSectorIndexQueryType	indexInfo;


  ErrNonFatalDisplayIf (!gP || !sectorInfoP, "null arg");


  // Get index of the sector containing the sector offset
  indexInfo.offset = offsetFromChipBase;
  err = FlashIntelGetSectorIndex (gP->device.deviceEntryP, &indexInfo);
  if (err)
	{
	  ErrNonFatalDisplay ("GetSectorIndex failed");
	  return (err);
	}

  // Get sector info
  sectorInfoP->index = indexInfo.index;
  err = FlashIntelGetSectorInfo (gP->device.deviceEntryP, sectorInfoP);
  if (err)
	{
	  ErrNonFatalDisplay ("GetSectorInfo failed");
	  return (err);
	}

  
  // Set up the optional return value
  if (sectorIndexP)
	*sectorIndexP = indexInfo.index;
  
  return (err);

} // PrvGetSectorInfoFromOffset
#endif // ERROR_CHECK_LEVEL == ERROR_CHECK_FULL


/***************************************************************
 *	Function:	PrvFlashProgramAPILoad
 *
 *	Summary:
 *		Makes a copy of the flash access code in the dynamic
 *		heap and calls its entry point to initialize the
 *		gP->program API structure.
 *
 *		This is necessary because we might be executing out of the
 *		same flash chip that we're flashing. The execution and
 *		flashing mode would interfere with each other.
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlashIntelOpen()
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvFlashIntelProgramAPILoad (FlashIntelGlobalsType* gP)
{
  DWord		err = 0;
  FlashIntelAccessEntryFuncType*	entryP = NULL;
  DmOpenRef	dbP = 0;
  VoidHand	h = 0;
  UInt32		resSize;


  // Error-check args
  ErrNonFatalDisplayIf (gP == NULL, "null arg");

  // Make sure we haven't been called already
  ErrNonFatalDisplayIf (gP->common.codeCopyP != NULL,
		  "access API already loaded");
  
  // Make sure we have an id of our code resource database
  ErrNonFatalDisplayIf (gP->common.codeDbId == 0, "no codeDbId");

  
  //-------------------------------------------------------------------------
  // Open our resource database
  //-------------------------------------------------------------------------
  dbP = DmOpenDatabase(gP->common.codeDbCardNo, gP->common.codeDbId,
					   dmModeReadOnly);
  if (dbP == 0)
	  {
	  err = DmGetLastErr ();	// for debugging
	  ErrNonFatalDisplayIf (true, "couldn't open code resource db");
	  err = flashErrCodeDbOpenError;
	  goto Exit;
	  }

  // Find the flash device access code resource
  h = DmGet1Resource (flashAccessCodeResType, gP->common.codeResId);
  if (h == 0)
	  {
	  err = DmGetLastErr ();	// for debugging
	  ErrNonFatalDisplayIf (true, "couldn't get flash access code resource");
	  err = flashErrNoAccessResource;
	  goto Exit;
	  }

  //-------------------------------------------------------------------------
  // Duplicate the resource
  //-------------------------------------------------------------------------
  
  // Get the size of the source resource
  resSize = MemHandleSize (h);

  // Allocate a duplicate in the dynamic heap
  gP->common.codeCopyP = MemPtrNew (resSize);
  if (gP->common.codeCopyP == NULL)
	  {
	  err = flashErrNoMemory;
	  goto Exit;
	  }

  // Copy data from the code resource to the new chunk in the
  //  dynamic heap
  MemMove (gP->common.codeCopyP, MemHandleLock(h), resSize);

  MemHandleUnlock (h);		// unlock db code resource
  

  //-------------------------------------------------------------------------
  // Now, call its entry point to get the interface
  //-------------------------------------------------------------------------
  entryP = (FlashIntelAccessEntryFuncType*)gP->common.codeCopyP;
  err = entryP (IntelAccessEntrySelGetInterface, &gP->program, sizeof(gP->program));


Exit:
  // Clean up
  if (h != 0)
	  DmReleaseResource (h);

  if (dbP != 0)
	  DmCloseDatabase (dbP);

  if (err && gP->common.codeCopyP != NULL)
	  {
	  MemPtrFree (gP->common.codeCopyP);
	  gP->common.codeCopyP = NULL;
	  }

  return (err);

} // PrvFlashProgramAPILoad



/***************************************************************
 *	Function:	PrvFlashIntelProgramAPIUnload
 *
 *	Summary:
 *		Frees the RAM-resident flash access code that was previously
 *		allocated by PrvFlashProgramAPILoad(), and resets the
 *		gP->program structure.
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlashIntelClose()
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static void
PrvFlashIntelProgramAPIUnload (FlashIntelGlobalsType* gP)
{
  ErrNonFatalDisplayIf (gP == NULL, "null arg");


  // Free the code chunk
  if (gP->common.codeCopyP != NULL)
	  {
	  MemPtrFree (gP->common.codeCopyP);
	  gP->common.codeCopyP = NULL;
	  MemSet (&gP->program, sizeof(gP->program), 0x00);
	  }


  return;

} // PrvFlashIntelProgramAPIUnload



/***************************************************************
 *	Function:	PrvErrorCheckGlobals 
 *
 *	Summary:      
 *		Validates our private globals.  This function is intended
 *		to be called in debug builds only, and compiled out of
 *		release builds.  It will caused a failed assertion if an
 *		error is discovered.
 *
 *		*** Call the conditionally-compiled macro PrvDbgValidateGlobals()
 *		instead of calling this function directly.
 *
 *	Parameters:
 *		gP		IN		Our private globals
 *
 *	Returns:
 *		nothing
 *  
 *	Called By: 
 *		Other Intel Flash Driver functions
 *
 *	Notes:
 *		
 *
 *	History:
 *    03-Oct-2000 - Created by Jan Piper
 ****************************************************************/
#if (ERROR_CHECK_LEVEL == ERROR_CHECK_FULL)

static void
PrvErrorCheckGlobals (FlashIntelGlobalsType* gP)
{

  ErrNonFatalDisplayIf (gP == NULL, "null Intel Fld globals");
  ErrNonFatalDisplayIf (gP->startSignature != flashIntelGlobalsStartSignature,
		  "bad Fld globals start signature");
  ErrNonFatalDisplayIf (gP->endSignature != flashIntelGlobalsEndSignature,
		  "bad Fld globals end signature");

  //-------------------------------------------------------------------------
  // Check client-supplied values
  //-------------------------------------------------------------------------
  ErrNonFatalDisplayIf (gP->common.codeDbCardNo >= MemNumCards(),
						  "bad cardNo");
  ErrNonFatalDisplayIf (gP->common.codeDbId == 0, "no codeDbID");
  ErrNonFatalDisplayIf (gP->common.flashBaseP == NULL, "no flashBaseP");


  //-------------------------------------------------------------------------
  // Check runtime values
  //-------------------------------------------------------------------------

  // Check our RAM-resident code chunk
  ErrNonFatalDisplayIf (gP->common.codeCopyP == 0, "no RAM-resident code");


  // Check device information structure
  if (gP->device.identified)
	  {
	  UInt32		i;
	  Ptr*		ptrP;
	  ErrNonFatalDisplayIf ((gP->device.numWords * 2) != gP->device.numBytes,
							  "bad device numWords");

	  ErrNonFatalDisplayIf (gP->device.maxSectorBytes != (128UL * 1024UL),
							  "bad maxSectorBytes");


	  ptrP = (Ptr*) &gP->program;
	  for (i=0; i < (sizeof(gP->program) / sizeof(Ptr)); i++, ptrP++)
		  ErrNonFatalDisplayIf (*ptrP == NULL, "null program func ptr");
	  }



  return;

} // PrvErrorCheckGlobals

#endif	// full error checking is on



