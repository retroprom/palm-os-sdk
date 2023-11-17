/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashMgr.c
 * 
 * Description:
 *    This file contains the code that manages flash drivers. 
 *
 * ToDo:
 *
 * History:
 *    4-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *
 ****************************************************************/
#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>
#include <HsExt.h>


#include "FlashDriver.h"
#include "FlashMgr.h"
#include "FlashMgrPrv.h"


//------------------------------------------------------------------------
// Private macro declarations
//------------------------------------------------------------------------

#if (ERROR_CHECK_LEVEL == ERROR_CHECK_FULL)

	#define PrvDbgValidateGlobals(gP)	PrvErrorCheckGlobals(gP)

#else	// not full error checking

	#define PrvDbgValidateGlobals(gP)

#endif	// not full error checking


//------------------------------------------------------------------------
// Private function prototypes
//------------------------------------------------------------------------

// API:

FlmEntryFuncType	FlmEntryPoint;

static FlmOpenFuncType		PrvFlmOpen;

static FlmCloseFuncType	PrvFlmClose;

static FlmAttrGetFuncType	PrvFlmAttrGet;

static FlmEraseFuncType	PrvFlmErase;

static FlmIsErasedFuncType	PrvFlmIsErased;

static FlmWriteFuncType	PrvFlmWrite;

static FlmMoveFuncType		PrvFlmMove;


// Helper functions:
#if (ERROR_CHECK_LEVEL == ERROR_CHECK_FULL)
  static void
  PrvErrorCheckGlobals (FlmGlobalsType* gP);
#endif

static DWord
PrvFlashDriversLoad (FlmGlobalsType* gP);

static DWord
PrvFlashDriversUnload (FlmGlobalsType* gP);

static DWord
PrvCodeDatabaseOpen (FlmGlobalsType* gP);

static DWord
PrvCodeDatabaseClose (FlmGlobalsType* gP);

static DWord
PrvSectorBackupBuffersAllocate (FlmGlobalsType* gP);

static DWord
PrvSectorBackupBuffersFree (FlmGlobalsType* gP);

static DWord
PrvFlashEraseHelper (FlmGlobalsType* gP, Byte* chipBaseP,
					Byte* eraseP, UInt32 eraseBytes,
					FlmEraseOptType* optP, FlmProgressReqType* progrReqP);

static void
PrvBackupBytes (FlmGlobalsType* gP, UInt32 startOffset, Byte* srcP,
				UInt32 numBytes);

static DWord
PrvRestoreBytes (FlmGlobalsType* gP, Byte* chipBaseP, UInt32 startOffset,
				 Byte* dstP, UInt32 numBytes, Byte* skipP, UInt32 skipBytes);

static DWord
PrvRestoreBytesHelper (FlmGlobalsType* gP, Byte* chipBaseP, UInt32 startOffset,
					   Byte* dstP, UInt32 numBytes);

static DWord
PrvMoveToLower (FlmGlobalsType* gP, void* dstP, void* srcP, UInt32 numBytes,
				FlmMoveOptType* optP);

static DWord
PrvMoveToHigher (FlmGlobalsType* gP, void* dstP, void* srcP, UInt32 numBytes,
				FlmMoveOptType* optP);

static Byte*
PrvAddrToChipBase (FlmGlobalsType* gP, void* startP);


static Boolean 
PrvFindCardNumFromBaseP (FlmGlobalsType* gP, UInt16* cardNumP);

//-----------------------------------------------------------------------------
// Must define entry routine so Metrowerks compiler doesn't complain...
//-----------------------------------------------------------------------------

#define FlmEntryPoint	__Startup__		// build as stand-alone code resource

DWord
start (UInt32 selId, void* argP, UInt32 argSize);
FlmEntryFuncType	start;




FlmEntryFuncType			FlmEntryPoint;


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
	return FlmEntryPoint (selId, argP, argSize);
}



/***************************************************************
 *	Function:	funcname 
 *
 *	Summary:      
 *
 *	Parameters:
 *		gP			IN	Pointer to private Flash Manager Globals 
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/




/***************************************************************
 *	Function:	FlmEntryPoint 
 *
 *	Summary:      
 *		This is the entry point of the Flash Manager code resource.
 *
 *	Parameters:
 *		selId		IN		function selector value (FlmEntrySelEnum)
 *		argP		IN/OUT	pointer to argument structure (type
 *							 and size depend on selId)
 *		argSize		IN		size of argument structure
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *
 *	Notes:
 *		
 *
 *	History:
 *    6-Apr-1999	VMK		Created
 ****************************************************************/
DWord
FlmEntryPoint (UInt32 selId, void* argP, UInt32 argSize)
{
	DWord					err = 0;


	// Handle the request
	switch (selId)
		{
		case flmEntrySelGetInterface:
			{
			FlmInterfaceType*	apiP;
			apiP = (FlmInterfaceType*)argP;

			ErrNonFatalDisplayIf (apiP == NULL, "null arg");
			ErrNonFatalDisplayIf (argSize != sizeof(FlmInterfaceType),
									"wrong arg size");
			
			if (argSize != sizeof(FlmInterfaceType))
				{
				err = flmErrBadArg;
				break;
				}

			// Initialize to all ones so in case we miss one, the call
			//  will bus error at this location
			MemSet(apiP, sizeof(FlmInterfaceType), 0xFF);

			apiP->FlmEntry		= &FlmEntryPoint;
			apiP->FlmOpen		= &PrvFlmOpen;
			apiP->FlmClose		= &PrvFlmClose;
			apiP->FlmAttrGet	= &PrvFlmAttrGet;
			apiP->FlmErase		= &PrvFlmErase;
			apiP->FlmIsErased	= &PrvFlmIsErased;
			apiP->FlmWrite		= &PrvFlmWrite;
			apiP->FlmMove		= &PrvFlmMove;
			}
			break;

		default:
			ErrNonFatalDisplayIf (true, "invalid Flash Manager entry sel id");
			err = flmErrInvalidSelId;
			break;
		}


	return (err);

} // FlmEntryPoint



/***************************************************************
 *	Function:	PrvFlmOpen 
 *
 *	Summary:      
 *		Initializes the flash manager, sets up globals, discovers
 *		and identifies flash components, loads the necessary flash
 *		drivers.
 *
 *	Parameters:
 *		refPP		OUT		Flash Manager open reference to be
 *							passed to other Flash Manager functions
 *		codeDbCardNo
 *					IN		Card number of the Flash Manager code
 *							 resource database
 *		codeDbId	IN		LocalID of the Flash Manager code
 *							 resource database
 *		cardBaseP	IN		pointer to the base of the card that
 *							 contains flash memory to be managed
 *							 by the Flash Manager
 *		flashOffset	IN		byte offset from card base to the
 *							 beginning of flash memory on the card
 *		readOnlySize
 *					IN		number of bytes at the beginning of flash
 *							 that are off limits (i.e. must not change)
 *		maxRange	IN		maximum number of bytes at beginning of flash
 *							 that may be searched for flash chips
 *		maxRangeIsFlashSize
 *					IN		if non-zero, then the total flash size
 *							 is specified by the caller in the maxRange
 *							 parameter; this overrides our on-the-fly
 *							 flash size determination; it is useful
 *							 when the flash card contents are in a
 *							 completely unknown state, such that
 *							 we can't test for address wrap-around
 *							 by comparing with the first chip
 *							 at flash base.
 *		openFlags	IN		Flash Manaer Open Flags (flmOpenFlagRead,
 *							 flmOpenFlagWrite, or flmOpenFlagReadWrite)
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 *	  26-May-1999	vmk		Added openFlags arg and allocatino of
 *							 backup buffers if opening for write
 *
 ****************************************************************/
static DWord
PrvFlmOpen (void** refPP, UInt16 codeDbCardNo, LocalID codeDbId,
			void* cardBaseP, UInt32 flashOffset, UInt32 readOnlySize,
			UInt32 maxRange, Byte maxRangeIsFlashSize, DWord openFlags)
{
	DWord					err = 0;
	FlmGlobalsType*			gP = NULL;
	UInt16					cardNum;
	Boolean					isCardFound = false;

	ErrNonFatalDisplayIf (refPP == NULL, "null arg");
	ErrNonFatalDisplayIf (codeDbId == 0 || cardBaseP == NULL, "null arg");
	ErrNonFatalDisplayIf (!(openFlags & flmOpenFlagReadWrite)
							|| (openFlags & ~flmOpenFlagAllFlags),
						  "bad openFlags");


	//------------------------------------------------------------------------
	// Allocate and init our globals
	//------------------------------------------------------------------------
	gP = MemPtrNew (sizeof(FlmGlobalsType));
	if (!gP) {
		err = flmErrNoMemory;
		goto Exit;
		}
	MemSet (gP, sizeof(FlmGlobalsType), 0);

	// Save the values passed to us by the caller -- we'll need them later
	gP->startSignature = flmGlobalsStartSignature;
	gP->endSignature = flmGlobalsEndSignature;
	gP->common.codeDbCardNo = codeDbCardNo;
	gP->common.codeDbId = codeDbId;
	gP->common.cardBaseP = cardBaseP;
	gP->common.flashOffset = flashOffset;
	gP->common.readOnlySize = readOnlySize;
	gP->common.maxRange = maxRange;
	gP->common.maxRangeIsFlashSize = maxRangeIsFlashSize;
	gP->common.openFlags = openFlags;



	isCardFound = PrvFindCardNumFromBaseP (gP, &cardNum);
	if (isCardFound)
	  {
	  gP->common.flashCardID = cardNum;
	  }
	else
	  err = hsErrInvalidCardNum;
	if (err)
		goto Exit;


	// Identify flash component(s) and load the corresponding flash driver(s)
	err = PrvFlashDriversLoad (gP);
	if (err)
		goto Exit;

	// If we're open for write, allocate our backup buffers now so we will
	//  know up-front whether we have enough memory to support write
	//  operations
	if (openFlags & flmOpenFlagWrite)
	  {
		err = PrvSectorBackupBuffersAllocate (gP);
		if (err)
			goto Exit;
	  }



Exit:
	// Clean up on error
	if (err)
		{
		if (gP)
			{
			PrvFlmClose (gP);
			gP = NULL;
			}
		}

	// Return our globals ptr as a handle that will be passed to all other
	//  FlashMgr routines
	*refPP = gP;

	return (err);

} // PrvFlmOpen



/***************************************************************
 *	Function:	PrvFlmClose 
 *
 *	Summary:      
 *		Frees resources allocated by PrvFlmOpen().  Handles partial
 *		clean-up in case it is called during failure in PrvFlmOpen().
 *
 *	Parameters:
 *		refP		IN		Flash Manager open reference returned
 *							by PrvFlmOpen()
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *		PrvFlmOpen();
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvFlmClose (void* refP)
{
	DWord					err = 0;
	FlmGlobalsType*			gP = (FlmGlobalsType*)refP;


	// Error-check our globals block
	PrvDbgValidateGlobals (gP);


	// Unload the flash driver(s) previously loaded by PrvFlashDriversLoad
	PrvFlashDriversUnload (gP);

	// Free the sector backup buffers
	PrvSectorBackupBuffersFree (gP);

	// Our code resource database should be closed at this point
	ErrNonFatalDisplayIf (gP->common.codeDbOpenCount != 0, "code DB still open");
	ErrNonFatalDisplayIf (gP->common.codeDbRef != 0, "code DB still open");

	// Finally, free our globals block
	//  (for debugging: reset the signatures to detect invalid access after close)
	gP->startSignature = gP->endSignature = flmGlobalsDiscardSignature;
	MemPtrFree (gP);


	return (err);

} // PrvFlmClose



/***************************************************************
 *	Function:	PrvFlmAttrGet
 *
 *	Summary:      
 *		Returns information about the flash memory or the Flash
 *		Driver.
 *
 *	Parameters:
 *		refP		IN		Flash Manager open reference returned
 *							by PrvFlmOpen()
 *		attrId		IN		Attribute to return (flmAttrEnum)
 *		argP		IN/OUT	pointer to argument structure (type
 *							 and size depend on attrId)
 *		argSize		IN		size of argument structure
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *
 *	Notes:
 *		
 *
 *	History:
 *    06-Apr-1999	VMK		Created
 *	  27-Aug-1999	vmk		Implemented flmAttrFlashPartNo
 *
 ****************************************************************/
static DWord
PrvFlmAttrGet (void* refP, UInt32 attrId, void* argP, UInt32 argSize)
{
	DWord					err = 0;
	FlmGlobalsType*			gP = (FlmGlobalsType*)refP;


	// Error-check our globals block
	PrvDbgValidateGlobals (gP);


	// Handle the request
	switch (attrId)
		{
		case flmAttrFlashConfig:
		  {
			FlmFlashConfigInfoType*	dataP;
			dataP = (FlmFlashConfigInfoType*)argP;

			ErrNonFatalDisplayIf (dataP == NULL, "null arg");
			ErrNonFatalDisplayIf (argSize != sizeof(FlmFlashConfigInfoType),
									"wrong arg size");
			
			dataP->numFlashChips = gP->common.numFlashChips;
			dataP->totalFlashSize = gP->common.totalFlashSize;
			dataP->writableSize = gP->common.writableSize;
			dataP->writableOffset = gP->common.writableOffset;
		  }
		  break;
		
		case flmAttrFlashPartNo:
		  {
			FlashPartNoInfoType	  flashInfo;
			FlmFlashPartNoInfoType*	dataP;
			dataP = (FlmFlashPartNoInfoType*)argP;

			ErrNonFatalDisplayIf (dataP == NULL, "null arg");
			ErrNonFatalDisplayIf (argSize != sizeof(FlmFlashPartNoInfoType),
									"wrong arg size");

			if (dataP->chipBaseP < gP->common.flashBaseP)
			  {
				ErrNonFatalDisplay ("chipBaseP out of bounds");
				err = flmErrBadArg;
				break;
			  }

			if ((Byte*)dataP->chipBaseP >= ((Byte*)gP->common.flashBaseP
											  + gP->common.totalFlashSize))
			  {
				ErrNonFatalDisplay ("chipBaseP out of bounds");
				err = flmErrBadArg;
				break;
			  }

			if ( (((Byte*)dataP->chipBaseP - (Byte*)gP->common.flashBaseP)
												% gP->driver.chipSize) != 0)
			  {
				ErrNonFatalDisplay ("chipBaseP not on device boundary");
				err = flmErrBadArg;
				break;
			  }

			err = gP->driver.api.FlashAttrGet (gP->driver.refP, dataP->chipBaseP,
											   flashAttrFlashPartNo,
											   &flashInfo,
											   sizeof (flashInfo));
			if (err)
				break;

			StrNCopy ((Char*)dataP->partNo, (Char*)flashInfo.partNo, sizeof (dataP->partNo));
			dataP->manufId = flashInfo.manufId;
			dataP->deviceId = flashInfo.deviceId;
		  }
		  break;

		case flmAttrFlashManagerVersion:
		  {
			FlmFlashManagerVersionInfoType*	  dataP;
			dataP = (FlmFlashManagerVersionInfoType*)argP;

			ErrNonFatalDisplayIf (dataP == NULL, "null arg");
			ErrNonFatalDisplayIf (argSize != sizeof(FlmFlashManagerVersionInfoType),
									"wrong arg size");

			dataP->reservedByte = flmFlashManagerVersionReservedByte;
			dataP->majorVersion = flmFlashManagerMajorVersion;
			dataP->minorVersion = flmFlashManagerMinorVersion;
			dataP->fixNumber	= flmFlashManagerFixVersion;
		  }
		  break;

		default:
			ErrNonFatalDisplayIf (true, "invalid Flash Manager attribute id");
			err = flmErrInvalidAttrId;
			break;
		}


	return (err);

} // PrvFlmAttrGet


/****************************************************************
 *	Function: PrvCheckIfErased	
 *	
 *	Summary:
 *	  
 *	
 *	Parameters:
 *	  startP		IN	Start of region to check	
 *	  bytesCount	IN	Number of bytes to check		
 *	
 *	Returns:
 *	  true if region is erased false otherwise.
 *	
 *	Called by:	PrvFlmErase
 *	  
 *	
 *	Notes:	This routine checks to see if we really need to erase
 *			the flash range, since it also means caching the rest of the
 *			sector and writting it back.
 *	  
 *	
 *	History:
 *	  13-Feb-2001 	JMP	Created
 *		
 *****************************************************************/
static Boolean
PrvCheckIfErased (void* startP, UInt32 bytesCount)
{
  //	unitP and unitEndP MUST be of same type
  Word*					unitP;
  Word*					unitEndP;
  UInt32				checkUnits;

  Byte*					startByteP = startP;
  Byte*					endByteP = startByteP + bytesCount;

  // NOTE: For currently used flash components, erased means a valud of 0xFF

  if (bytesCount == 0)
	return (true);

  // If we are on an odd boundary, let's check the first Byte
  // and advance next position (also decrement the bytesCount).
  if ((DWord)startByteP & 0x01)
	{
	  if (*startByteP != 0xFF)
		return(false);
	  startByteP++;
	  bytesCount--;
	}

  // Now that we are on an even boundary, let's optimize by 
  // checking multiple bytes at a time. 
  unitP = (Word*) startByteP;
  checkUnits = bytesCount / sizeof(* unitP);
  unitEndP = unitP + checkUnits;

  for (/*empty*/; unitP < unitEndP; unitP++)
	{
	  if (*unitP != 0xFFFF)
		return(false);
	}

  // Finally we may have some left over bytes to check, 
  //  so go back to byte check mode
  startByteP = (Byte *) unitEndP;

  for (/*empty*/; startByteP < endByteP; startByteP++)
	{
	  if (*startByteP != 0xFF)
		  return(false);
	}

  // If we made it all the way out to here, the whole region is already erased		
  return(true);
}// PrvCheckIfErased



/***************************************************************
 *	Function:	PrvFlmIsErased
 *
 *	Summary:      
 *		Checks if the specified area of flash is erased
 *
 *
 *	Parameters:
 *		refP		IN	Flash Manager open reference returned
 *						 by PrvFlmOpen()
 *		startP		IN	start address
 *		numBytes	IN	number of bytes to check
 *		isErasedP	OUT	non-zero if area is erased  
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *
 *	Notes:
 *		
 *
 *	History:
 *    26-May-1999	VMK		Created
 *
 ****************************************************************/
static DWord
PrvFlmIsErased (void* refP, void* startP, UInt32 numBytes, Byte* isErasedP)
{
	DWord					err = 0;
	FlmGlobalsType*			gP = (FlmGlobalsType*)refP;

	// Error-check our globals block
	PrvDbgValidateGlobals (gP);

	ErrNonFatalDisplayIf (!isErasedP, "null arg");


	// Validate the address range
	//
	//  Out of bounds conditions here should never occur, and indicate
	//  that our client is really messed up.  This condition justifies
	//  forcing the user to reset the device rather than risking having
	//  corrupted flash image -- so we use ErrFatalDisplayIf().
	ErrFatalDisplayIf (startP < gP->common.writableBaseP
							|| startP > gP->common.writableEndP,
						"out of bounds");
	ErrFatalDisplayIf (((Byte*)startP + numBytes) > (Byte*)gP->common.writableEndP,
						"out of bounds");


	*isErasedP  = PrvCheckIfErased (startP, numBytes);

	return (err);

} // PrvFlmIsErased



/***************************************************************
 *	Function:	PrvFlmErase
 *
 *	Summary:      
 *		Erases the specified range of flash.  Handles flash chip
 *		and sector boundaries.
 *
 *		Validates erase range; calls the flash driver's "erase"
 *		function for every chip intercepted by the erase range.
 *
 *	Parameters:
 *		refP		IN		Flash Manager open reference returned
 *							by PrvFlmOpen()
 *		startP		IN		erase start address
 *		eraseBytes	IN		number of bytes to erase
 *		optP		IN		[OPTIONAL] erase optimizations (see
 *							 structure definition in FlashMgr.h)
 *		progrReqP	IN		[OPTIONAL] progress report request
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *
 *	Notes:
 *		Pass NULL to ignore the optional parameter
 *
 *		progrReqP support is not implemented yet;
 *		
 *
 *	History:
 *    06-Apr-1999	VMK		Created
 *	  28-May-1999	vmk		Added support for the restoreOff field
 *							 of the optimization parameter.
 *	  27-Aug-1999	vmk		Added the progrReqP arg
 *
 ****************************************************************/
static DWord
PrvFlmErase (void* refP, void* startP, UInt32 eraseBytes,
			 FlmEraseOptType* optP, FlmProgressReqType* progrReqP)
{
	DWord					err = 0;
	FlmGlobalsType*			gP = (FlmGlobalsType*)refP;
	Byte*					chipBaseP;
	Byte*					chipEndP;
	Byte*					eraseP;
	Byte*					eraseEndP;
	UInt32					bytesLeft;


	// Error-check our globals block
	PrvDbgValidateGlobals (gP);


 	// Validate the erase range
	//
	//  Out of bounds conditions here should never occur, and indicate
	//  that our client is really messed up.  This condition justifies
	//  forcing the user to reset the device rather than risking having
	//  corrupted flash image -- so we use ErrFatalDisplayIf().
	ErrFatalDisplayIf (startP < gP->common.writableBaseP
							|| startP > gP->common.writableEndP,
						"erase request start out of bounds");
	ErrFatalDisplayIf (((Byte*)startP + eraseBytes) > (Byte*)gP->common.writableEndP,
						"erase request range out of bounds");


	// Make sure the backup buffers are available
	if (gP->backupInfo.numHeapSeg == 0 && gP->backupInfo.numDataSeg == 0)
		{
		err = PrvSectorBackupBuffersAllocate (gP);
		if (err)
			goto Exit;
		}


	// If nothing to erase, bail out now
	if (eraseBytes == 0)
		goto Exit;


 	// Initialize the progress data structure
	gP->progress.flmRefP = refP;
	gP->progress.soFar = 0;
	gP->progress.total = eraseBytes;
	if (progrReqP)
		gP->progress.userRefP = progrReqP->userRefP;
	else
		gP->progress.userRefP = NULL;
	gP->progress.reserved = 0;


	// Determine the base address of the first chip intersected by the range
	//  and start erasing from one chip at a time
	chipBaseP = PrvAddrToChipBase (gP, startP);
	chipEndP = chipBaseP + gP->driver.chipSize;
	eraseP = (Byte*)startP;
	eraseEndP = eraseP + eraseBytes;
	bytesLeft = eraseBytes;
	while (bytesLeft > 0)
		{
		UInt32		eraseChunkSize;

		// Validate our logic
		ErrNonFatalDisplayIf ((UInt32)(eraseEndP - eraseP) != bytesLeft,
							"logic error");


		// Determine number of bytes to erase in the current chip,
		//  making sure it's not bigger than the amount that remains
		//  to be erased
		eraseChunkSize = chipEndP - eraseP;
		if (eraseChunkSize > bytesLeft)
			eraseChunkSize = bytesLeft;
 
		// Now, call the driver to erase the desired range from the current chip
		err = PrvFlashEraseHelper (gP, chipBaseP, eraseP, eraseChunkSize,
								  optP, progrReqP);
		if (err)
			break;

		
		// On to next chip
		chipBaseP += gP->driver.chipSize;
		chipEndP += gP->driver.chipSize;
		eraseP = chipBaseP;
		bytesLeft -= eraseChunkSize;
		} // while (bytesLeft > 0)


Exit:
	return (err);

} // PrvFlmErase



/***************************************************************
 *	Function:	PrvFlashEraseHelper
 *
 *	Summary:      
 *		Helper function that turns chip-centric flash erase requests
 *		into flash sector erase requests to the flash driver, and
 *		takes care of backing up and restoring surrounding
 *		data.
 *
 *		Validates erase range; calls the flash driver's "erase"
 *		function for every chip intercepted by the erase range.
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *		chipBaseP	IN		base address of the chip
 *		eraseP		IN		erase start address
 *		eraseBytes	IN		number of bytes to erase
 *		optP		IN		[OPTIONAL] erase optimizations (see
 *							 structure definition in FlashMgr.h)
 *		progrReqP	IN		[OPTIONAL] progress report request
 *
 *		The "progress" field of our globals has been initialized by
 *		 our caller with its "total" field set to the total number
 *		 of bytes to be erased.
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmErase()
 *
 *	Notes:
 *		
 *
 *	History:
 *    6-Apr-1999	VMK		Created
 *	  28-May-1999	vmk		Added support for the restoreOff field
 *							 of the optimization parameter.
 *
 ****************************************************************/
static DWord
PrvFlashEraseHelper (FlmGlobalsType* gP, Byte* chipBaseP,
					Byte* eraseP, UInt32 eraseBytes,
					FlmEraseOptType* optP, FlmProgressReqType* progrReqP)
{
  DWord		err = 0;
  Err		osErr;
  FlashSectorIndexQueryType	offsetToSector;
  FlashSectorInfoType			sectorInfo;
  Byte*		sectorBaseP;
  Byte*		sectorEndP;
  Byte*		eraseEndP;
  UInt32	bytesLeft;
  UInt32	sectorIndex;
  Byte		restoreOff = false;
  UInt32	skipBytes = 0;
  Byte*		skipP = NULL;
  Boolean	taskSwitchingOn = true;

  // Do some error checking first
  ErrNonFatalDisplayIf (PrvAddrToChipBase (gP, chipBaseP) != chipBaseP,
						  "invalid chipBaseP");
  ErrNonFatalDisplayIf (eraseP < chipBaseP, "eraseP before chipBaseP");
  ErrNonFatalDisplayIf ((eraseP + eraseBytes) > (chipBaseP + gP->driver.chipSize),
						  "erase range out of bounds");

  // See if surrounding data restoration is being turned off
  if (optP)
	{
	  ErrNonFatalDisplayIf (optP->reserved != 0, "not zero");
	  restoreOff = optP->restoreOff;
	  if (!restoreOff)
		{
		  skipP = (Byte*)optP->skipP;
		  skipBytes = optP->skipBytes;
		}
	}


  // Get index of the first sector to erase from
  offsetToSector.offset = eraseP - chipBaseP;
  gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
	  flashAttrSectorIndex, &offsetToSector, sizeof(offsetToSector));
  sectorIndex = offsetToSector.index;

  // Erase from one sector at a time
  bytesLeft = eraseBytes;
  while (bytesLeft > 0)
	{
	  UInt32		eraseChunkSize;
	  UInt32		bytesBefore, bytesAfter;	// # of backed up bytes
											  // before and after the erase
											  // region
	  Boolean		alreadyErased;


	  // Get the offset and size of this sector
	  sectorInfo.index = sectorIndex;
	  gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
		  flashAttrSectorInfo, &sectorInfo, sizeof(sectorInfo));

	  // Calculate sector base and end pointers
	  sectorBaseP = chipBaseP + sectorInfo.offset;
	  sectorEndP = sectorBaseP + sectorInfo.size;

	  // For subsequent sectors, eraseP must be same as sectorBaseP
	  if (sectorIndex != offsetToSector.index)
		  {
		  ErrNonFatalDisplayIf(eraseP != sectorBaseP, "logic error");
		  }
	  
	  // Determine amount to erase from this sector
	  eraseChunkSize = sectorEndP - eraseP;
	  if (eraseChunkSize > bytesLeft)
		  eraseChunkSize = bytesLeft;

	  eraseEndP = eraseP + eraseChunkSize;

	  // Erase only if needed -- optimization
	  alreadyErased = PrvCheckIfErased (eraseP, eraseChunkSize);
	  if (!alreadyErased)
		{
		  // Backup data in the sector around the region to be erased
		  bytesBefore = eraseP - sectorBaseP;
		  bytesAfter = sectorEndP - eraseEndP;
		  if (!restoreOff)
			{
			  PrvBackupBytes (gP, 0 /*offset*/, sectorBaseP, bytesBefore);
			  PrvBackupBytes (gP, bytesBefore, eraseEndP, bytesAfter);
			}


		  // Disable task switching while flash is in transition to prevent
		  //  other tasks from accessing the area of flash being modified and
		  //  crashing the device
		  osErr = SysTaskSwitching (false /*enable*/);
		  ErrNonFatalDisplayIf (osErr, "SysTaskSwitching(off) failed");
		  taskSwitchingOn = false;

		  // Erase the sector
		  err = gP->driver.api.FlashSectorErase (gP->driver.refP, chipBaseP,
												  sectorBaseP);
		  if (err)
			  break;


		  // Restore data in the sector around the erased region
		  if (!restoreOff)
			{
			  //-------------------------------------------------------------------
			  // Restore the "before erase" region
			  //-------------------------------------------------------------------

			  err = PrvRestoreBytes (gP, chipBaseP, 0 /*buf offset*/,
									 sectorBaseP, bytesBefore, skipP, skipBytes);
			  if (err)
				  break;


			  //-------------------------------------------------------------------
			  // Restore the "after erase" region
			  //-------------------------------------------------------------------

			  err = PrvRestoreBytes (gP, chipBaseP, bytesBefore /*buf offset*/,
									 eraseEndP, bytesAfter, skipP, skipBytes);
			  if (err)
				  break;

			} // Restore data in the sector around the erased region


		  // Restore task switching
		  osErr = SysTaskSwitching (true /*enable*/);
		  ErrNonFatalDisplayIf (osErr, "SysTaskSwitching(on) failed");
		  taskSwitchingOn = true;
		} // Erase only if needed


	  // Report progress
	  if (progrReqP)
		{
		  ErrNonFatalDisplayIf (!progrReqP->callbackP, "null progress callbackP");
		  gP->progress.soFar += eraseChunkSize;
		  progrReqP->callbackP (&gP->progress);
		}

	  // On to the next sector
	  sectorIndex++;
	  eraseP += eraseChunkSize;
	  bytesLeft -= eraseChunkSize;
	}


  // Restore task switching if we didn't get a chance to do it
  if (!taskSwitchingOn)
	{
	  osErr = SysTaskSwitching (true /*enable*/);
	  ErrNonFatalDisplayIf (osErr, "SysTaskSwitching(on) failed");
	}

  return (err);

} // PrvFlashEraseHelper




/***************************************************************
 *	Function:	PrvFlmWrite
 *
 *	Summary:      
 *		Writes the specified range of flash.  Handles flash chip
 *		and sector boundaries.
 *
 *		***IMPORTANT***
 *		The caller must ensure that writing is happening over an
 *		erased area -- this is required by the flash chip hardware.
 *
 *		Validates write range; calls the flash driver's "write"
 *		function for every chip intercepted by the write range.
 *
 *	Parameters:
 *		refP		IN		Flash Manager open reference returned
 *							by PrvFlmOpen()
 *		dstP		IN		flash write address
 *		srcP		IN		pointer to source data
 *		numBytes	IN		number of bytes to write
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *
 *	Notes:
 *		
 *
 *	History:
 *    6-Apr-1999	VMK		Created
 *	  1-Jun-1999	vmk		Implemented "skip region" optimization
 *
 ****************************************************************/
static DWord
PrvFlmWrite (void* refP, void* dstP, void* srcP, UInt32 numBytes)
{
	DWord					err = 0;
	FlmGlobalsType*			gP = (FlmGlobalsType*)refP;
	Byte*					chipBaseP;
	Byte*					chipEndP;
	Byte*					writeP;
	Byte*					writeEndP;
	Byte*					fromP;
	UInt32					bytesLeft = numBytes;


	// Error-check our globals block
	PrvDbgValidateGlobals (gP);


	// Validate the write range
	//
	//  Out of bounds conditions here should never occur, and indicate
	//  that our client is really messed up.  This condition justifies
	//  forcing the user to reset the device rather than risking having
	//  corrupted flash image -- so we use ErrFatalDisplayIf().
	ErrFatalDisplayIf (dstP < gP->common.writableBaseP
							|| dstP > gP->common.writableEndP,
						"write request start out of bounds");
	ErrFatalDisplayIf (((Byte*)dstP + numBytes) > (Byte*)gP->common.writableEndP,
						"write request range out of bounds");


	// Make sure the backup buffers are available
	if (gP->backupInfo.numHeapSeg == 0 && gP->backupInfo.numDataSeg == 0)
		{
		err = PrvSectorBackupBuffersAllocate (gP);
		if (err)
			goto Exit;
		}


	// If nothing to write, bail out now
	if (numBytes == 0)
		goto Exit;


	ErrNonFatalDisplayIf(srcP == NULL, "null source data");


	// Determine the base address of the first chip intersected by the range
	//  and start writing to one chip at a time
	chipBaseP = PrvAddrToChipBase (gP, dstP);
	chipEndP = chipBaseP + gP->driver.chipSize;
	fromP = (Byte*)srcP;
	writeP = (Byte*)dstP;
	writeEndP = writeP + numBytes;
	bytesLeft = numBytes;
	while (bytesLeft > 0)
		{
		UInt32		writeChunkSize;

		// Validate our logic
		ErrNonFatalDisplayIf ((UInt32)(writeEndP - writeP) != bytesLeft,
							"logic error");


		// Determine number of bytes to write in the current chip,
		//  making sure it's not bigger than the amount that remains
		//  to be written
		writeChunkSize = chipEndP - writeP;
		if (writeChunkSize > bytesLeft)
			writeChunkSize = bytesLeft;

		// Now, call the driver to erase the desired range from the current chip
		err = gP->driver.api.FlashChipWrite (gP->driver.refP, chipBaseP,
										writeP, fromP, writeChunkSize);
		if (err)
			break;

		
		// On to next chip
		chipBaseP += gP->driver.chipSize;
		chipEndP += gP->driver.chipSize;
		writeP = chipBaseP;
		fromP += writeChunkSize;
		bytesLeft -= writeChunkSize;
		} // while (bytesLeft > 0)


Exit:
	return (err);

} // PrvFlmWrite



/***************************************************************
 *	Function:	PrvFlmMove
 *
 *	Summary:      
 *		Moves the specified range of flash to another range in flash.
 *		It handles overlapping moves across flash chip and sector
 *		boundaries.  Performs necessary sector erasing, unless erasing
 *		is suppressed by the caller (i.e., the caller knows what
 *		he's doing).
 *
 *		***IMPORTANT***
 *		Following the move, the contents of the portion of the source region
 *		that does not overlap with the destination region are
 *		undefined -- i.e., this function is optimized such that it will
 *		not preserve some parts of the source region during move
 *		operations where parts of the move occur within the same sector.
 *
 *
 *	Parameters:
 *		refP		IN		Flash Manager open reference returned
 *							by PrvFlmOpen()
 *		dstP		IN		flash write address
 *		srcP		IN		pointer to source data
 *		numBytes	IN		number of bytes to write
 *		optP		IN		[OPTIONAL] optimization info
 *		progrReqP	IN		[OPTIONAL] progress report request
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		Clients of Flash Manager;
 *
 *	Notes:
 *		Pass NULL to ignore optional parameters;
 *
 *		progrReqP support is not implemented yet;
 *
 *	History:
 *    07-Apr-1999	VMK		Created
 *	  27-Aug-1999	vmk		Added the progrReqP arg
 *
 ****************************************************************/
static DWord
PrvFlmMove (void* refP, void* dstP, void* srcP, UInt32 numBytes,
			FlmMoveOptType* optP, FlmProgressReqType* progrReqP)
{
	DWord				err = 0;
	FlmGlobalsType*		gP = (FlmGlobalsType*)refP;


	// Error-check our globals block
	PrvDbgValidateGlobals (gP);


	// Validate the write range
	//
	//  Out of bounds conditions here should never occur, and indicate
	//  that our client is really messed up.  This condition justifies
	//  forcing the user to reset the device rather than risking having
	//  corrupted flash image -- so we use ErrFatalDisplayIf().
	ErrFatalDisplayIf (dstP < gP->common.writableBaseP
							|| dstP > gP->common.writableEndP,
						"write request start out of bounds");
	ErrFatalDisplayIf (((Byte*)dstP + numBytes) > (Byte*)gP->common.writableEndP,
						"write request range out of bounds");

	ErrNonFatalDisplayIf(srcP == NULL, "null source data");


	// If nothing to move, bail out now
	if (numBytes == 0 || dstP == srcP)
		goto Exit;


	// Make sure the backup buffers are available
	if (gP->backupInfo.numHeapSeg == 0 && gP->backupInfo.numDataSeg == 0)
		{
		err = PrvSectorBackupBuffersAllocate (gP);
		if (err)
			goto Exit;
		}

	// Call helper functions to do the actual work
	if (dstP < srcP)
		err = PrvMoveToLower (gP, dstP, srcP, numBytes, optP);
	else
		err = PrvMoveToHigher (gP, dstP, srcP, numBytes, optP);

Exit:
	return (err);

} // PrvFlmMove



/***************************************************************
 *	Function:	PrvMoveToLower
 *
 *	Summary:
 *		Helper function called by PrvFlmMove() to handle flash moves
 *		where the destination is at a lower address than the
 *		source.
 *
 *		Moves the specified range of flash to another range in flash.
 *		It handles overlapping moves across flash chip and sector
 *		boundaries.  Performs necessary sector erasing, unless erasing
 *		is suppressed by the caller (i.e., the caller knows what
 *		he's doing).
 *
 *		***IMPORTANT***
 *		Following the move, the contents of the portion of the source region
 *		that does not overlap with the destination region are
 *		undefined -- i.e., this function is optimized such that it will
 *		not preserve some parts of the source region during move
 *		operations where parts of the move occur within the same sector.
 *
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *		dstP		IN		flash write address
 *		srcP		IN		pointer to source data
 *		numBytes	IN		number of bytes to write
 *		optP		IN		[OPTIONAL] optimization info
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmMove()
 *
 *	Notes:
 *
 *	History:
 *    7-Apr-1999	VMK		Created
 *	  1-Jun-1999	vmk		Implemented "skip region" optimization
 *
 ****************************************************************/
static DWord
PrvMoveToLower (FlmGlobalsType* gP, void* dstP, void* srcP, UInt32 numBytes,
				FlmMoveOptType* optP)
{
  DWord					err = 0;
  FlashSectorIndexQueryType	offsetToSector;
  Byte*					chipBaseP;
  Byte*					chipEndP;
  Byte*					curSrcP;
  Byte*					srcEndP;
  Byte*					curDstP;
  Byte*					dstEndP;
  UInt32					bytesLeft;
  UInt32					sectorIndex;
  Byte*					skipP = NULL;
  UInt32					skipBytes = 0;
  Byte					dontErase = false;


  // Our caller, PrvFlmMove(), has already validated the globals, destination
  //  range, and took care of the degenerate cases (numBytes==0 || srcP == dstP).
  //  It also made sure that backup buffers are set up.

  ErrNonFatalDisplayIf (srcP <= dstP, "src and dst out of order");


  // Check our optional optimization arg
  if (optP)
	{
	  ErrNonFatalDisplayIf (optP->reserved != 0, "not zero");
	  dontErase = optP->dontErase;
	  skipP = optP->skipP;
	  skipBytes = optP->skipBytes;
	}

  // Initialize our source and destination pointers
  curSrcP = (Byte*)srcP;
  srcEndP = curSrcP + numBytes;
  curDstP = (Byte*)dstP;
  dstEndP = curDstP + numBytes;

  // Get the initial destination chip base and end
  chipBaseP = PrvAddrToChipBase (gP, dstP);
  chipEndP = chipBaseP + gP->driver.chipSize;

  // Get the initial destination sector index
  offsetToSector.offset = curDstP - chipBaseP;
  gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
	  flashAttrSectorIndex, &offsetToSector, sizeof(offsetToSector));
  sectorIndex = offsetToSector.index;

  //-------------------------------------------------------------------------
  // Move to one sector at a time
  //-------------------------------------------------------------------------
  bytesLeft = numBytes;
  while (bytesLeft > 0)
	  {
	  FlashSectorInfoType		sectorInfo;
	  UInt32	copyChunkSize;
	  Byte*	sectorBaseP;	// destination sector base pointer
	  Byte*	sectorEndP;		// destination sector end pointer
	  Byte*	curDstEndP;


	  // Validate our logic
	  ErrNonFatalDisplayIf ((UInt32)(dstEndP - curDstP) != bytesLeft,
						  "logic error");


	  //-----------------------------------------------------------
	  // Determine the destination parameters for this iteration
	  //-----------------------------------------------------------

	  // Get the offset and size of the destination sector
	  sectorInfo.index = sectorIndex;
	  gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
								  flashAttrSectorInfo, &sectorInfo,
								  sizeof(sectorInfo));

	  // Calculate sector base and end pointers
	  sectorBaseP = chipBaseP + sectorInfo.offset;
	  sectorEndP = sectorBaseP + sectorInfo.size;

	  // Determine how many bytes will be moved into the current
	  //  destination sector
	  copyChunkSize = sectorEndP - curDstP;
	  if (copyChunkSize > bytesLeft)
		  copyChunkSize = bytesLeft;

	  curDstEndP = curDstP + copyChunkSize;

	  
	  // Some error checking before we continue
	  ErrNonFatalDisplayIf (curDstP < (Byte*)dstP, "curDstP < dstP");
	  ErrNonFatalDisplayIf (curDstEndP > chipEndP, "curDstEndP > chipEndP");
	  ErrNonFatalDisplayIf (curDstEndP > dstEndP, "curDstEndP > dstEndP");
	  ErrNonFatalDisplayIf (curSrcP < (Byte*)srcP, "curSrcP < srcP");
	  ErrNonFatalDisplayIf (curSrcP >= srcEndP, "curSrcP >= srcEndP");
	  ErrNonFatalDisplayIf (curSrcP <= curDstP, "curSrcP <= curDstP");


	  //-----------------------------------------------------------
	  // Move the chunk to be copied into our backup buffers
	  //  in case there is overlap with the destination sector, which
	  //  is about to be erased
	  //-----------------------------------------------------------
	  PrvBackupBytes (gP, 0 /*offset*/, curSrcP, copyChunkSize);
	  

	  //-----------------------------------------------------------
	  // Erase the sector, backing up and restoring surrounding data
	  //  as necessary
	  //-----------------------------------------------------------

	  if (!dontErase)
		  {
		  UInt32	bytesBeforeDst,	// backup bytes between sector base and dest
				  bytesAfterDst,	// backup bytes between dest and source
				  bytesAfterSrc;	// backup bytes between src and sector end

		  // Backup data in the sector around the region being replaced
		  bytesBeforeDst = curDstP - sectorBaseP;
		  bytesAfterDst = sectorEndP - curDstEndP;
		  bytesAfterSrc = 0;

		  // If there is source data in this destination sector, remove it
		  //  from the range being backed up and restored
		  if (bytesAfterDst != 0 && (Byte*)srcP < sectorEndP)
			  {
			  // If source and destination don't overlap, then save the bytes
			  //  between them
			  if (curDstEndP < (Byte*)srcP)
				  {
				  // We must have reached the end of the copy range
				  ErrNonFatalDisplayIf (curDstEndP != dstEndP, "logic error");
				  bytesAfterDst = (Byte*)srcP - curDstEndP;
				  }

			  // Otherwise, destination and source overlap and there is nothing
			  //  between them to preserve
			  else
				  bytesAfterDst = 0;

			  ErrNonFatalDisplayIf (bytesAfterDst >= (UInt32)(sectorEndP - curDstEndP),
								  "logic error");

			  // If source data ends before the end of the current destination
			  //  sector, then save sector bytes following the source data
			  if (srcEndP < sectorEndP)
				  {
				  // We must have reached the end of the copy range
				  ErrNonFatalDisplayIf (curDstEndP != dstEndP, "logic error");
				  bytesAfterSrc = sectorEndP - srcEndP;
				  }

			  // Otherwise, source data runs through the end of current destination
			  //  sector, and there is nothing to preserve
			  else
				  bytesAfterSrc = 0;

			  ErrNonFatalDisplayIf (bytesAfterSrc >= (UInt32)(sectorEndP - curDstEndP),
								  "logic error");
			  } // Remove source range from backup

		  if (bytesBeforeDst)
			  PrvBackupBytes (gP, copyChunkSize /*offset*/,
							  sectorBaseP, bytesBeforeDst);
		  if (bytesAfterDst)
			  PrvBackupBytes (gP, copyChunkSize + bytesBeforeDst,
							  curDstEndP, bytesAfterDst);

		  if (bytesAfterSrc)
			  PrvBackupBytes (gP, copyChunkSize + bytesBeforeDst + bytesAfterDst,
							  srcEndP, bytesAfterSrc);

		  // Erase the sector
		  err = gP->driver.api.FlashSectorErase (gP->driver.refP, chipBaseP,
												  sectorBaseP);
		  if (err)
			  break;

		  // Restore data in the sector around the region being replaced
		  if (bytesBeforeDst)
			  err = PrvRestoreBytes (gP, chipBaseP, copyChunkSize /*offset*/,
									 sectorBaseP, bytesBeforeDst,
									 skipP, skipBytes);
		  
		  if (!err && bytesAfterDst)
			  err = PrvRestoreBytes (gP, chipBaseP, copyChunkSize
													+ bytesBeforeDst,
									 curDstEndP, bytesAfterDst,
									 skipP, skipBytes);

		  if (!err && bytesAfterSrc)
			  err = PrvRestoreBytes (gP, chipBaseP, copyChunkSize
													+ bytesBeforeDst
													+ bytesAfterDst,
									 srcEndP, bytesAfterSrc,
									 skipP, skipBytes);

		  if (err)
			  break;
		  } // Erase the sector, backing up and restoring surrounding data


	  //-----------------------------------------------------------
	  // Now, write the current data chunk from the backup buffer to
	  //  its destination in the sector 
	  //-----------------------------------------------------------
	  err = PrvRestoreBytes (gP, chipBaseP, 0 /*offset*/, curDstP,
							  copyChunkSize, NULL /*skipP*/, 0/*skipBytes*/);
	  if (err)
		  break;


	  // On to the next chunk to copy
	  curDstP = curDstEndP;
	  curSrcP += copyChunkSize;
	  bytesLeft -= copyChunkSize;
	  sectorIndex++;

	  // If we reached the end of the current destination chip, update
	  //  chip base and end pointers for the next chip, and reset the
	  //  sector index
	  if (curDstP == chipEndP)
		  {
		  ErrNonFatalDisplayIf (sectorIndex != gP->driver.numSectors, "logic error");
		  chipBaseP = chipEndP;
		  chipEndP += gP->driver.chipSize;
		  sectorIndex = 0;
		  }

	  // Subsequent moves, if any, should always have their destinations on
	  //  sector boundaries
	  ErrNonFatalDisplayIf (bytesLeft != 0 && curDstP != sectorEndP,
							  "logic error");

	  } // while (bytesLeft > 0)

//Exit:
  return (err);

} // PrvMoveToLower



/***************************************************************
 *	Function:	PrvMoveToHigher
 *
 *	Summary:
 *		Helper function called by PrvFlmMove() to handle flash moves
 *		where the destination is at a higher address than the
 *		source.  To handle overlapping moves, the data is moved
 *		starting from the end.
 *
 *		Moves the specified range of flash to another range in flash.
 *		It handles overlapping moves across flash chip and sector
 *		boundaries.  Performs necessary sector erasing, unless erasing
 *		is suppressed by the caller (i.e., the caller knows what
 *		he's doing).
 *
 *		***IMPORTANT***
 *		Following the move, the contents of the portion of the source region
 *		that does not overlap with the destination region are
 *		undefined -- i.e., this function is optimized such that it will
 *		not preserve some parts of the source region during move
 *		operations where parts of the move occur within the same sector.
 *
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *		dstP		IN		flash write address
 *		srcP		IN		pointer to source data
 *		numBytes	IN		number of bytes to write
 *		optP		IN		[OPTIONAL] optimization info
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmMove()
 *
 *	Notes:
 *
 *	History:
 *    7-Apr-1999	VMK		Created
 *	  1-Jun-1999	vmk		Implemented "skip region" optimization
 *
 ****************************************************************/
static DWord
PrvMoveToHigher (FlmGlobalsType* gP, void* dstP, void* srcP, UInt32 numBytes,
				FlmMoveOptType* optP)
{
  DWord					err = 0;
  FlashSectorIndexQueryType	offsetToSector;
  Byte*					chipBaseP;
  Byte*					chipEndP;
  Byte*					srcEndP;
  Byte*					curDstEndP;
  Byte*					dstEndP;
  UInt32					dstOffset;		// difference between dest and src
  UInt32					bytesLeft;
  UInt32					sectorIndex;
  Byte*					skipP = NULL;
  UInt32					skipBytes = 0;
  Byte					dontErase = false;


  // Our caller, PrvFlmMove(), has already validated the globals, destination
  //  range, and took care of the degenerate cases (numBytes==0 || srcP == dstP).
  //  It also made sure that backup buffers are set up.

  // To handle overlapping moves from a lower to a higher address, we're
  //  going to move data starting from the end...

  // Compute offset of destination from source (it should always be positive
  //  because we're moving data from a lower to a higher address)
  ErrNonFatalDisplayIf (dstP <= srcP, "src and dst out of order");
  dstOffset = (Byte*)dstP - (Byte*)srcP;

  // Check our optional optimization arg
  if (optP)
	{
	  ErrNonFatalDisplayIf (optP->reserved != 0, "not zero");
	  dontErase = optP->dontErase;
	  skipP = optP->skipP;
	  skipBytes = optP->skipBytes;
	}


  // Initialize our source and destination pointers
  srcEndP = (Byte*)srcP + numBytes;

  dstEndP = (Byte*)dstP + numBytes;
  curDstEndP = dstEndP;

  // Get the initial destination chip base and end
  chipBaseP = PrvAddrToChipBase (gP, dstEndP - 1);
  chipEndP = chipBaseP + gP->driver.chipSize;

  // Get the initial destination sector index
  offsetToSector.offset = dstEndP - chipBaseP - 1;
  gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
	  flashAttrSectorIndex, &offsetToSector, sizeof(offsetToSector));
  sectorIndex = offsetToSector.index;

/*PrvMoveToHigher*/
  //-------------------------------------------------------------------------
  // Move to one sector at a time, starting from the end
  //-------------------------------------------------------------------------
  bytesLeft = numBytes;
  while (bytesLeft > 0)
	  {
	  FlashSectorInfoType		sectorInfo;
	  UInt32	copyChunkSize;
	  Byte*	sectorBaseP;	// start of current destination sector
	  Byte*	sectorEndP;		// end of current destination sector
	  Byte*	curSrcP;
	  Byte*	curDstP;


	  // This loop assumes the following global variables are set up on entry:
	  //
	  //  These are fixed:
	  //		srcP, dstP, dstOffset, srcEndP, dstEndP
	  //
	  //  These will be adjusted at end of loop:
	  //		chipBaseP, sectorIndex, curDstEndP, bytesLeft


	  // Validate our logic
	  ErrNonFatalDisplayIf ((UInt32)(curDstEndP - (Byte*)dstP) != bytesLeft,
						  "logic error");


	  //-----------------------------------------------------------
	  // Determine the destination parameters for this iteration
	  //-----------------------------------------------------------
	  // Get the offset and size of the destination sector
	  sectorInfo.index = sectorIndex;
	  gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
								  flashAttrSectorInfo, &sectorInfo,
								  sizeof(sectorInfo));

	  // Calculate sector base and end pointers
	  sectorBaseP = chipBaseP + sectorInfo.offset;
	  sectorEndP = sectorBaseP + sectorInfo.size;

	  // Determine the number of bytes that will be moved into the
	  //  current destination sector
	  copyChunkSize = curDstEndP - sectorBaseP;
	  if (copyChunkSize > bytesLeft)
		  copyChunkSize = bytesLeft;

	  curDstP = curDstEndP - copyChunkSize;
	  curSrcP = curDstP - dstOffset;

/*PrvMoveToHigher*/
	  ErrNonFatalDisplayIf (curDstP < (Byte*)dstP, "curDstP < dstP");
	  ErrNonFatalDisplayIf (curSrcP < (Byte*)srcP, "curSrcP < srcP");
	  ErrNonFatalDisplayIf (curSrcP >= curDstP, "curSrcP >= curDstP");


	  //-----------------------------------------------------------
	  // Copy the source chunk into our backup buffers
	  //  in case there is overlap with the destination sector, which
	  //  is about to be erased
	  //-----------------------------------------------------------
	  PrvBackupBytes (gP, 0 /*offset*/, curSrcP, copyChunkSize);
	  

	  //-----------------------------------------------------------
	  // Erase the sector, backing up and restoring surrounding data
	  //  as necessary
	  //-----------------------------------------------------------

	  if (!dontErase)
		  {
		  UInt32	bytesAfterDst,	// backup bytes after destination chunk
				  bytesBeforeSrc,	// backup bytes from sector base to skip area
				  bytesAfterSrc;	// backup bytes from end of skip area to dest

		  // Backup data in the sector around the region being replaced
		  bytesAfterDst = sectorEndP - curDstEndP;
		  bytesBeforeSrc = curDstP - sectorBaseP;
		  bytesAfterSrc = 0;


		  // If there is source data in this destination sector, remove it
		  //  from the range being backed up and restored
		  if (bytesBeforeSrc != 0 && srcEndP > sectorBaseP)
			  {
			  // If the source starts after current sector base, we only need
			  //  to preserve the bytes between the sector base and start
			  //  of source
			  if ((Byte*)srcP > sectorBaseP)
				  {
				  // We must have reached the beginning of the copy range
				  ErrNonFatalDisplayIf (curDstP != (Byte*)dstP, "logic error");
				  bytesBeforeSrc = (Byte*)srcP - sectorBaseP;
				  }

			  // Otherwise, the source starts at or before current sector base,
			  //  and there are no bytes to preserve
			  else
				  bytesBeforeSrc = 0;

			  ErrNonFatalDisplayIf (bytesBeforeSrc >= (UInt32)(curDstP - sectorBaseP),
					  "logic error");

/*PrvMoveToHigher*/
			  // If the source does not overlap the destination, we need
			  //  to preserve the bytes between them
			  if (srcEndP < curDstP)
				  {
				  // We must have reached the beginning of the copy range
				  ErrNonFatalDisplayIf (curDstP != (Byte*)dstP, "logic error");
				  bytesAfterSrc = curDstP - srcEndP;
				  }

			  // Otherwise, there is an overlap and there are no bytes between
			  //  them that need preserving
			  else
				  bytesAfterSrc = 0;

			  ErrNonFatalDisplayIf (bytesAfterSrc >= (UInt32)(curDstP - sectorBaseP),
					  "logic error");
			  }


		  if (bytesBeforeSrc)
			  PrvBackupBytes (gP, copyChunkSize /*offset*/,
							  sectorBaseP, bytesBeforeSrc);
		  if (bytesAfterSrc)
			  PrvBackupBytes (gP, copyChunkSize + bytesBeforeSrc,
							  srcEndP, bytesAfterSrc);

		  if (bytesAfterDst)
			  PrvBackupBytes (gP, copyChunkSize + bytesBeforeSrc + bytesAfterSrc,
							  curDstEndP, bytesAfterDst);

		  // Erase the sector
		  err = gP->driver.api.FlashSectorErase (gP->driver.refP, chipBaseP,
												  sectorBaseP);
		  if (err)
			  break;

		  // Restore data in the sector around the region being replaced
		  if (bytesBeforeSrc)
			  err = PrvRestoreBytes (gP, chipBaseP, copyChunkSize /*offset*/,
									 sectorBaseP, bytesBeforeSrc,
									 skipP, skipBytes);
		  
		  if (!err && bytesAfterSrc)
			  err = PrvRestoreBytes (gP, chipBaseP, copyChunkSize
													+ bytesBeforeSrc,
									 srcEndP, bytesAfterSrc,
									 skipP, skipBytes);

		  if (!err && bytesAfterDst)
			  err = PrvRestoreBytes (gP, chipBaseP, copyChunkSize
													+ bytesBeforeSrc
													+ bytesAfterSrc,
									 curDstEndP, bytesAfterDst,
									 skipP, skipBytes);

		  if (err)
			  break;
		  } // Erase the sector, backing up and restoring surrounding data

/*PrvMoveToHigher*/
	  //-----------------------------------------------------------
	  // Now, write the current data chunk from the backup buffer to
	  //  its destination in the sector 
	  //-----------------------------------------------------------
	  err = PrvRestoreBytes (gP, chipBaseP, 0 /*offset*/, curDstP,
							  copyChunkSize, NULL /*skipP*/, 0/*skipBytes*/);
	  if (err)
		  break;


	  // On to the next chunk to copy (remember -- we're copying from the end!)
	  curDstEndP -= copyChunkSize;
	  bytesLeft -= copyChunkSize;

	  // If we're done with the current chip, update
	  //  chip base and end pointers to the previous chip, and reset the
	  //  sector index to last sector
	  if (sectorIndex == 0)
		  {
		  ErrNonFatalDisplayIf (curDstEndP != chipBaseP, "ptr logic error");
		  chipEndP = chipBaseP;
		  chipBaseP -= gP->driver.chipSize;
		  sectorIndex = gP->driver.numSectors - 1;
		  }
	  else
		  sectorIndex--;

	  // Subsequent moves, if any, should always have their destinations end
	  //  on sector boundaries
	  ErrNonFatalDisplayIf (bytesLeft != 0 && curDstEndP != sectorBaseP,
							  "logic error");

	  } // while (bytesLeft > 0)

//Exit:
  return (err);

} // PrvMoveToHigher



/***************************************************************
 *	Function:	PrvBackupBytes
 *
 *	Summary:      
 *		Helper function that backs up a range of flash sector bytes
 *		to our backup RAM buffers.
 *
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *		startOffset	IN		starting offset within backup buffer space
 *		srcP		IN		pointer to data in flash to back up
 *		numBytes	IN		number of bytes to back up
 *
 *	Returns:
 *		nothing
 *  
 *	Called By: 
 *		PrvFlashEraseHelper()
 *
 *	Notes:
 *		
 *
 *	History:
 *    6-Apr-1999	VMK		Created
 *    15-Dec-2000	JMP		Modified to handle the Data and the Heap
 *							memory backup segments.		  
 ****************************************************************/
static void
PrvBackupBytes (FlmGlobalsType* gP, UInt32 startOffset, Byte* srcP,
				UInt32 numBytes)
{
	Byte*		toP;
	UInt32		bufferOffset;
	UInt16		bufferIndex;
	UInt32		bytesLeft;
	Err			err = 0;

	ErrNonFatalDisplayIf (gP == NULL || srcP == NULL, "null arg");
	ErrNonFatalDisplayIf (gP->backupInfo.segSize == 0, "zero buf size");

	bufferIndex = (UInt16)(startOffset / gP->backupInfo.segSize);
	bufferOffset = startOffset % gP->backupInfo.segSize;

	bytesLeft = numBytes;
	while (bytesLeft > 0)
		{
		  UInt32	chunkSize;

		  ErrNonFatalDisplayIf (bufferIndex >= (gP->backupInfo.numHeapSeg + gP->backupInfo.numDataSeg),
							  "bufferIndex out of bounds");
		  ErrNonFatalDisplayIf (gP->backupInfo.seg[bufferIndex] == NULL,
							  "null backup buffer");

		  // Determine amount to copy into current buffer
		  chunkSize = gP->backupInfo.segSize - bufferOffset;
		  if (chunkSize > bytesLeft)
			  chunkSize = bytesLeft;

		  // Copy chunk to backup buffer
		  if (bufferIndex < flmMaxBackupHeapBuffers)
			{
			  toP = (Byte*)(gP->backupInfo.seg[bufferIndex]) + bufferOffset;
			  MemMove (toP, srcP, chunkSize);
			}
		  else
			{
			  err = DmWrite (gP->backupInfo.seg[bufferIndex], bufferOffset, srcP, chunkSize);
			  ErrNonFatalDisplayIf (err, "DmWrite failed");
			}

		  // On to the next backup buffer
		  bufferIndex++;
		  bufferOffset = 0;
		  srcP += chunkSize;
		  bytesLeft -= chunkSize;
		}

	return;

} // PrvBackupBytes



/***************************************************************
 *	Function:	PrvRestoreBytes
 *
 *	Summary:      
 *		Helper function that restores a range of flash sector bytes
 *		from our backup RAM buffers.
 *
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *		chipBaseP	IN		base address of the chip
 *		startOffset	IN		starting offset within backup buffer space
 *							 from which to restore
 *		dstP		IN		pointer to location in flash to restore
 *		numBytes	IN		number of bytes to restore
 *		skipP		IN		if non-zero, start of skip region
 *		skipBytes	IN		size of skip region
 *
 *	Returns:
 *		nothing
 *  
 *	Called By: 
 *		PrvFlashEraseHelper()
 *
 *	Notes:
 *		
 *
 *	History:
 *    6-Apr-1999	VMK		Created
 *	  1-Jun-1999	vmk		Added "skip region" optimization
 *
 ****************************************************************/
static DWord
PrvRestoreBytes (FlmGlobalsType* gP, Byte* chipBaseP, UInt32 startOffset,
				 Byte* dstP, UInt32 numBytes, Byte* skipP, UInt32 skipBytes)
{
  DWord		err = 0;
  Byte*		dstEndP;
  Byte*		skipEndP;
  UInt32	restOffset1, restOffset2, restBytes1, restBytes2;
  Byte*		restDst1P;
  Byte*		restDst2P;

  ErrNonFatalDisplayIf (gP == NULL || dstP == NULL, "null arg");
  ErrNonFatalDisplayIf (skipP && !skipBytes, "bad arg");
  ErrNonFatalDisplayIf (skipBytes && !skipP, "bad arg");

  // Take care of the degenerate case first
  if (numBytes == 0)
	  return (0);


  dstEndP = dstP + numBytes;
  skipEndP = skipP + skipBytes;

  // Init the restore parameters
  restDst1P = dstP;
  restDst2P = NULL;
  restBytes1 = numBytes;
  restBytes2 = 0;
  restOffset1 = startOffset;
  restOffset2 = 0;

  // If skip and restore regions overlap, adjust the restore parameters
  if (skipP < dstEndP && skipEndP > dstP)
	{
	  // If skip region starts at beginning of or before restore region
	  if (skipP <= dstP)
		  restBytes1 = 0;

	  // If skip region starts after beginning of restore region
	  else
		  restBytes1 = skipP - dstP;

	  
	  // If skip region ends before the end of restore region
	  if (skipEndP < dstEndP)
		{
		  restDst2P = skipEndP;
		  restBytes2 = dstEndP - restDst2P;
		  restOffset2 = startOffset + (restDst2P - dstP);
		}

	}

  if (restBytes1)
	{
	  err = PrvRestoreBytesHelper (gP, chipBaseP, restOffset1, restDst1P,
								   restBytes1);
	  if (err)
		  goto Exit;
	}

  if (restBytes2)
	{
	  err = PrvRestoreBytesHelper (gP, chipBaseP, restOffset2, restDst2P,
								   restBytes2);
	  if (err)
		  goto Exit;
	}

Exit:

  return (err);

} // PrvRestoreBytes

static DWord
PrvRestoreBytesHelper (FlmGlobalsType* gP, Byte* chipBaseP, UInt32 startOffset,
					   Byte* dstP, UInt32 numBytes)
{
	DWord		err = 0;
	Byte*		fromP;
	UInt32		bufferOffset;
	UInt16		bufferIndex;
	UInt32		bytesLeft;

	ErrNonFatalDisplayIf (gP == NULL || dstP == NULL, "null arg");
	ErrNonFatalDisplayIf (gP->backupInfo.segSize == 0, "zero buf size");

	bufferIndex = (UInt16)(startOffset / gP->backupInfo.segSize);
	bufferOffset = startOffset % gP->backupInfo.segSize;

	bytesLeft = numBytes;
	while (bytesLeft > 0)
		{
		UInt32	chunkSize;

		ErrNonFatalDisplayIf (bufferIndex >= (gP->backupInfo.numHeapSeg + gP->backupInfo.numDataSeg),
							"bufferIndex out of bounds");
		ErrNonFatalDisplayIf (gP->backupInfo.seg[bufferIndex] == NULL,
							"null backup buffer");

		// Determine amount to restore from current buffer
		chunkSize = gP->backupInfo.segSize - bufferOffset;
		if (chunkSize > bytesLeft)
			chunkSize = bytesLeft;

		// Now, call the driver to write the chunk to flash
		fromP = (Byte*)(gP->backupInfo.seg[bufferIndex]) + bufferOffset;
		err = gP->driver.api.FlashChipWrite (gP->driver.refP, chipBaseP,
										dstP, fromP, chunkSize);
		if (err)
			break;

		// On to the next backup buffer
		bufferIndex++;
		bufferOffset = 0;
		dstP += chunkSize;
		bytesLeft -= chunkSize;
		}


	return (err);

} // PrvRestoreBytesHelper



/***************************************************************
 *	Function:	PrvAddrToChipBase 
 *
 *	Summary:      
 *		Given an address in flash memory, determines the base
 *		address of the flash chip that contains the given address.
 *:
 *		The first version assumes that all flash chips on the
 *		card are the same as the first flash chip on the card.
 *
 *	Parameters:
 *		refP		IN		Our private globals
 *		startP		IN		an address in flash memory
 *
 *	Returns:
 *		pointer to base of chip
 *  
 *	Called By: 
 *		PrvFlmOpen()
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
static Byte*
PrvAddrToChipBase (FlmGlobalsType* gP, void* startP)
{
	Byte*		chipBaseP = NULL;
	UInt32		offset;			// offset from start of flash
	UInt32		chipIndex;

	
	// Make sure the address is at or after the flash base
	ErrNonFatalDisplayIf (startP < gP->common.flashBaseP, "out of range");

	// Get offset from start of flash to the start address
	offset = (Byte*)startP - (Byte*)gP->common.flashBaseP;

	// Calculate the 0-based flash chip index
	chipIndex = offset / gP->driver.chipSize;
	ErrNonFatalDisplayIf (chipIndex >= gP->common.numFlashChips, "bad chipIndex");

	// Calculate offset to start of desired chip
	offset = chipIndex * gP->driver.chipSize;

	// Add offset to flash base pointer to produce the chip base address
	chipBaseP = (Byte*)gP->common.flashBaseP + offset;


	return (chipBaseP);

} // PrvAddrToChipBase

/****************************************************************
 *	Function:	PrvFindCardNumFromBaseP 
 *	
 *	Summary:	Find the Card Number matching the Base address 
 *				of gP->common.cardBaseP.
 *	  
 *	
 *	Parameters:
 *	  gP			IN		Flash Manager private globals			
 *	  cardNumP		IN/OUT	Card Number		
 *	
 *	Returns:
 *	  static Boolean true if a card was found.
 *	
 *	Called by:	PrvFlmOpen
 *	  
 *	
 *	Notes:		Go through all the cards, identify the removable ones, 
 *				and then match the BaseP to identify the card we are
 *				looking for.
 *	  
 *	
 *	History:
 *	  2001-2-20 	JMP	Created
 *		
 *****************************************************************/
static Boolean PrvFindCardNumFromBaseP (FlmGlobalsType* gP, UInt16* cardNumP)
{
	Byte	  removable;
	UInt32	  err = 0;
	UInt32	  logicalBase;
	UInt16	numCards;
	Boolean	  isCardFound = false;

	numCards = MemNumCards ();
	//  Go through all the cards, identify the removable ones, and then 
	//  match them with the corresponding Chip Select prior to setting the new value.
	for (*cardNumP = 0; *cardNumP < numCards; (*cardNumP)++)
	  {
	    HsCardAttrGet(*cardNumP, hsCardAttrRemovable, &removable);
	    if (removable)
	      {
			// Get expansion card info via Handspring Extensions
			err = HsCardAttrGet (*cardNumP, hsCardAttrLogicalBase,
								&logicalBase);
			ErrNonFatalDisplayIf (err != 0, "LogicalBase failed");
			if (err)
			  break;
			if ( logicalBase == (UInt32)gP->common.cardBaseP)
			  {
				isCardFound = true;
				break;
			  }
		  }
	  }
	return (isCardFound);
}//PrvFindCardNumFromBaseP


/***************************************************************
 *	Function:	PrvSetChipSelectSize 
 *
 *	Summary:      
 *		Given the gP->common.logicalBase, set the identified
 *		Chip Select to the given size. 
 *		HsExt of Palm oS < 3.5 had a bug so we write directly
 *		to the register.
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *		csSize		IN		Chip Select set size
 *
 *	Returns:
 *		set ChipSelectsize error.
 *  
 *	Called By: 
 *		PrvFlashDriversLoad()
 *
 *	Notes:
 *		Go through all the cards, identify the removable ones, 
 *		and then match them with the corresponding Chip Select 
 *		prior to setting the new value. For PalmOS < 3.5.2 we have to 
 *		manually set the register because of a bug in the Hal, 
 *		otherwise we use FtrSet.
 *
 *	History:
 *    15-Dec-2000	JMP		Created
 ****************************************************************/
static DWord PrvSetChipSelectSize (FlmGlobalsType* gP, DWord csSize)
{
	DWord					err = 0;
	DWord					romVersion = 0;
	WordPtr					tmpcsSizeP = NULL;
	Word					wValue;
	UInt16					cardNum;


	  cardNum = gP->common.flashCardID ;

	  //csSize = gP->common.totalFlashSize/gP->common.numFlashChips;
	  // See if we need to patch HsCardArrtSet if ROM version < 3.5.
	  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

 	  //Only go and do the manual H/W Update (didn't work) on less than 3.5 OS 
	  if (romVersion < sysMakeROMVersion(3,5,0,sysROMStageRelease,0))
  		{
 		  tmpcsSizeP =   (WordPtr) 0xFFFFF112UL;	  //Chip Select 2 on EZ/VZ processor

		  if (csSize < 0x20000)				// min allowed is 128KBytes
			{
			  err = hsErrInvalidParam;
			  goto Exit;
			}
		  csSize = csSize >> 18;		// min is 128K
		  wValue = 0;						// register value for 128K
		  while (csSize)
			{
			  csSize >>= 1;
			  wValue++;
			}
		
		  // Can't set greater than 16Meg
		  if (wValue > 7) 
			{
			  err = hsErrInvalidParam; 
			  goto Exit;
			}
		  //Everything is OK, lets update the ChipSelect Register
		  //with the new size
		  *tmpcsSizeP = (UInt16)((*tmpcsSizeP & ~0x0E) | (wValue << 1));
		}
	  else
		{		
		  // This assumes that we HAL knows how	to set card x hsCardAttrCsSize		  
		  err = HsCardAttrSet(cardNum,  hsCardAttrCsSize,  &csSize);  
		}
	
Exit:
	return (err);

} // PrvSetChipSelectSize


/***************************************************************
 *	Function:	PrvGetChipSelectSize 
 *
 *	Summary:      
 *		Given the gP->common.logicalBase, get the identified
 *		Chip Select size. 
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *
 *	Returns:
 *		get ChipSelectsize error.
 *  
 *	Called By: 
 *		PrvFlashDriversLoad()
 *
 *	Notes:
 *				
 *
 *	History:
 *    15-Dec-2000	JMP		Created
 ****************************************************************/
static DWord PrvGetChipSelectSize (FlmGlobalsType* gP)
{
	DWord					err = 0;
	UInt16					cardNum;
	DWord					csSize = 0;

	cardNum = gP->common.flashCardID;

	//  Get the value
	err = HsCardAttrGet(cardNum,  hsCardAttrCsSize,  &csSize);  
	ErrNonFatalDisplayIf (err != 0, "Get CS Size failed");
	
	return (csSize);

} // PrvGetChipSelectSize

/***************************************************************
 *	Function:	PrvFlashConfigureGeometry 
 *
 *	Summary:      
 *		Discover all the chips on the card. 
 *
 *	Parameters:
 *		gP			IN		Flash Manager private globals
 *
 *	Returns:
 *		get ChipSelectsize error.
 *  
 *	Called By: 
 *		PrvFlashDriversLoad()
 *
 *	Notes:
 *	This routine includes all calls relative to the geometry configuration 
 *	of the flash. We also scan the Flash to find out how many chips we have,
 *	First we see if the CS is set to the Maximum range, and if the
 *	flmOpenChipSelectAutoConfig flag was used to open the flash. If so we see how many
 *	chips we can see on Chip Select 0. Then we shrink the Chip Select 0 size
 *	to the total size of the chips we found, and we continue the scan. This will
 *	allow us to detect all the chips on CS0 and CS1 regardless if the 
 *	SpringBoard uses a chip selector internally or not.
 *
 *	History:
 *    15-Dec-2000	JMP		Created
 *    13-Feb-2001	JMP		Added a check to see if the CS size was
 *							initialized smaller than the max value, by the 
 *							calling application. if so, then do NOT autoresize.
 *    20-Feb-2001	JMP		Moved some configuration logic from 
 *							PrvFlashDriversLoad to PrvChipCount and renamed it
 *							to PrvFlashConfigureGeometry.
 ****************************************************************/
static DWord 
PrvFlashConfigureGeometry (FlmGlobalsType* gP)
{
	DWord				csSize = 0;
	DWord				err = 0;
	Err					osErr;
	void*				chipBaseP;
	void*				flashBaseP;	  // were the flash chips begin
	void*				flashEndP;	  // where it all ends
	Boolean				csNotYetAutoResized = true;

	//-------------------------------------------------------------------------
	// Now, detemine the number of flash chips and the start of writable flash
	//-------------------------------------------------------------------------
	chipBaseP = (Byte*)gP->common.cardBaseP + gP->common.flashOffset;
	
	// Get chip's total flash memory size in # of bytes
	gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP, flashAttrChipSize,
								&gP->driver.chipSize,
								sizeof(gP->driver.chipSize));

	// Get the number of sectors on the chip
	gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP, flashAttrNumSectors,
								&gP->driver.numSectors,
								sizeof(gP->driver.numSectors));

	// Get maximum sector size in # of bytes
	gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP, flashAttrMaxSectorSize,
								&gP->driver.maxSectorSize,
								sizeof(gP->driver.maxSectorSize));

	flashBaseP = (Byte*)gP->common.cardBaseP + gP->common.flashOffset;
	flashEndP = (Byte*)flashBaseP + gP->common.maxRange;


	MemSemaphoreReserve(false);

	// Disable task switching while flash is in transition to prevent
	//  other tasks from accessing the area of flash being modified and
	//  crashing the device
	osErr = SysTaskSwitching (false /*enable*/);
	ErrNonFatalDisplayIf (osErr, "SysTaskSwitching(off) failed");

	// Discover all the chips on the card -- the assumption is that all the
	//  flash chips on the card are the same as the first flash chip on the
	//  card

	// <chg 2001-2-13 JMP> changed from flmOpenFlagAutoScan 
	//	to flmOpenChipSelectAutoConfig	
	if (gP->common.openFlags & flmOpenChipSelectAutoConfig)
	  {
	  csSize = PrvGetChipSelectSize (gP);  
	  // <chg 2001-2-13 JMP> The csSize was already resized,  
	  //  so we don't do the ChipSelect autosizing.
	  //  This is done if we are looking at a module that specifically
	  //  does not want flmOpenChipSelectAutoConfig but accidently used it.
	  if (csSize == (gP->common.maxRange / 2))
		{
		csNotYetAutoResized = false;
		}
	  }

	//	Now count the chips and set the Chip Select Size if required
	while (1)
	  {
		// Make sure we're still within the allowed range
		if (chipBaseP >= flashEndP)
			break;		// no more flash chips

		// Check if there is a chip at this location
		if (gP->driver.api.FlashExists (gP->driver.refP, chipBaseP) != 0)
			break;		// no more flash chips -->


		// Check if the address has wrapped around -- this can happen
		//  when the flash card ignores some of the high address bits
		//  that are not strictly necessary for amount of flash on the
		//  card.
		//
		//  We do this by comparing data at the start of the current
		//  chip address with that at flash base.  Since the data at
		//  flash base should be the original card header (of the
		//  support store), a match at this point will indicate a wrap-
		//  around.
		if (!gP->common.maxRangeIsFlashSize && chipBaseP != flashBaseP)
		  {
			// If we have a match, the address must have wrapped around
		    if (MemCmp (chipBaseP, flashBaseP, flmFlashStartCheckBytes) == 0)
			  {
				if (gP->driver.api.FlashSameDevice (gP->driver.refP,
													flashBaseP, chipBaseP))
				  {
					// <chg 2001-2-13 JMP> changed from flmOpenFlagAutoScan 
					//	to flmOpenChipSelectAutoConfig
					if (gP->common.openFlags & flmOpenChipSelectAutoConfig &
						csNotYetAutoResized)
					  {
						// Lets check that the totalFlashSize <= to the Chip 
						// select size. If not, we have to set the chip select  
						// size and countinue our chip count.
						if (csSize <= gP->common.totalFlashSize)
						  break;	// no more flash chips -->

						//	Lets squeeze the scSize and see if we can now   
						//	count more chips.
						csSize  = gP->common.totalFlashSize;
						err = PrvSetChipSelectSize (gP, csSize);
						if (err)
						  goto Exit;
						//	Update the new Maximum range that can be scanned
						flashEndP = (Byte*)chipBaseP + (2*csSize); 
					  }
					else
					  {
						break;	// no more flash chips -->
					  }
				  }
			  }
		  }

		// Update totals
		gP->common.numFlashChips++;	// we know we already have one
		gP->common.totalFlashSize += gP->driver.chipSize;

		// If the read-only region ends in this chip, find the
		//  writable offset
		if (gP->common.readOnlySize != 0 && gP->common.writableOffset == 0
				&& gP->common.readOnlySize <= gP->common.totalFlashSize)
		  {
			FlashSectorIndexQueryType	offsetToSectorArg;
			FlashSectorInfoType			sectorInfo;

			// Compute the chip-relative offset of the last byte of read-only area
			offsetToSectorArg.offset = gP->common.readOnlySize
				- (gP->common.totalFlashSize - gP->driver.chipSize) - 1;

			// Get index of the sector containing the last read-only byte
			gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
				flashAttrSectorIndex, &offsetToSectorArg,
				sizeof(offsetToSectorArg));

			// Now, get the offset and size of this sector
			sectorInfo.index = offsetToSectorArg.index;
			gP->driver.api.FlashAttrGet (gP->driver.refP, chipBaseP,
				flashAttrSectorInfo, &sectorInfo, sizeof (sectorInfo));
			
			// Compute the writable offset
			gP->common.writableOffset = gP->common.flashOffset
				+ (gP->common.totalFlashSize - gP->driver.chipSize)
				+ sectorInfo.offset + sectorInfo.size;
		  }

		// On to the next chip (if any)
		chipBaseP = (Byte*)chipBaseP + gP->driver.chipSize;

	  } // Discover all the chips on the card



	// If the size of the read-only area is 0, then set the writableOffset
	//  to flashOffset
	if (gP->common.readOnlySize == 0)
		gP->common.writableOffset = gP->common.flashOffset;

	// Calculate the size of the writable area
	gP->common.writableSize = gP->common.totalFlashSize
						- (gP->common.writableOffset - gP->common.flashOffset);
	ErrNonFatalDisplayIf (gP->common.writableSize > gP->common.totalFlashSize,
						"logic error -- writable size is too big");
	ErrNonFatalDisplayIf (gP->common.writableSize == 0,
						"logic error -- writable size is zero");

	// Calculate other values for convenience, so we don't have to calculate
	//  them every time
	gP->common.flashBaseP = flashBaseP;
	gP->common.writableBaseP = (Byte*)gP->common.cardBaseP
									+ gP->common.writableOffset;
	gP->common.writableEndP = (Byte*)gP->common.writableBaseP
									+ gP->common.writableSize;

	// Now that all flash chips have been enumerated, perform consistency checks
	if (gP->common.numFlashChips == 0)
		{
		ErrNonFatalDisplayIf (true, "chip enum logic error");
		err = flmErrFlashEnumError;
		goto Exit;
		}

	if (gP->common.writableOffset < (gP->common.flashOffset
									+ gP->common.readOnlySize))
		{
		ErrNonFatalDisplayIf (true, "chip enum logic error");
		err = flmErrFlashEnumError;
		goto Exit;
		}

	err = 0;
	gP->common.flashEnumerated = true;

Exit:
		// Restore task switching
		osErr = SysTaskSwitching (true /*enable*/);
		ErrNonFatalDisplayIf (osErr, "SysTaskSwitching(on) failed");
		MemSemaphoreRelease(false);
		return (err);
} //  PrvFlashConfigureGeometry


/***************************************************************
 *	Function:	PrvFlashDriversLoad 
 *
 *	Summary:      
 *		Identifies flash components and loads the corresponding
 *		flash driver(s).
 *:
 *		The first version assumes that all flash chips on the
 *		card are the same as the first flash chip on the card.
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmOpen()
 *
 *	Notes:
 *		
 *
 *	History:
 *	  04-Apr-1999	VMK		Created
 *	  25-Aug-1999	vmk		Call FlashSameDevice() to break ties when
 *							 both chips start with the same card header
 *							 data
 *	  14-Nov-2000   jmp		Added the ChipSelect Size update to 
 *							allow 1 and 2 chip Flash
 *	  20-Feb-2001   jmp		Moved the code relative to Chip geometry
 *							Configuration to PrvFlashConfigureGeometry
****************************************************************/
static DWord
PrvFlashDriversLoad (FlmGlobalsType* gP)
{
	DWord				err = 0;
	Word				codeResId;
	void*				chipBaseP;

	ErrNonFatalDisplayIf(gP == NULL, "null arg");


	// Open our code resource database so that we may access our
	//  code resources
	err = PrvCodeDatabaseOpen (gP);
	if (err != 0)
	  return (err);


	//-------------------------------------------------------------------------
	// Enumerate through all flash drivers until we get to the
	//  one that matches the first flash chip
	//-------------------------------------------------------------------------
	chipBaseP = (Byte*)gP->common.cardBaseP + gP->common.flashOffset;
	codeResId = flashCodeResIdFirst;
	while (1)
	  {
		VoidHand			codeH;	// temp variables
		FlashEntryFuncType*	flashEntryP;
		void*				refP;

		codeH = DmGet1Resource (flashCodeResType, codeResId);
		if (!codeH)
		  {
			err = flmErrNoDriver;		// this should never happen
			goto Exit;					// no more drivers -->
		  }

		// Get the flash driver's entry point
		flashEntryP = (FlashEntryFuncType*)MemHandleLock(codeH);

		// Release the resource
		DmReleaseResource(codeH);

		// Get the flash driver's interface
		flashEntryP (flashEntrySelGetInterface, &gP->driver.api, sizeof(gP->driver.api));


		// Try to open the driver -- if it matches the chip, the open operation
		//  will succeed, otherwise it will fail
		err = gP->driver.api.FlashOpen (&refP, gP->common.codeDbCardNo,
											gP->common.codeDbId, codeResId,
											chipBaseP);

		// If found, we're done
		if (!err)
		  {
			gP->driver.loaded = true;
			gP->driver.resId = codeResId;
			gP->driver.entryP = flashEntryP;
			gP->driver.refP = refP;
			break;
		  }

		// Unlock this code resource
		MemPtrUnlock(flashEntryP);
										
		// On to the next flash driver
		codeResId++;
	  }


	// We're here because the driver *WAS* loaded
	ErrNonFatalDisplayIf (!gP->driver.loaded, "logic error");

	// Scan the module to count the chips assigned to each Chip Select
	err = PrvFlashConfigureGeometry(gP);
	if (err)
		{
		ErrNonFatalDisplayIf (true, "FlashCard Configuration error");
		err = flmErrFlashEnumError;
		goto Exit;
		}

Exit:
	// Close our code resource database
	PrvCodeDatabaseClose (gP);

	return (err);

} // PrvFlashDriversLoad



/***************************************************************
 *	Function:	PrvFlashDriversUnload 
 *
 *	Summary:      
 *		Unloads the flash drivers previously loaded by
 *		PrvFlashDriversLoad().  Frees up the corresponding system
 *		resources that were allocated for supporting the drivers.
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmClose()
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvFlashDriversUnload (FlmGlobalsType* gP)
{
	DWord				err = 0;

	ErrNonFatalDisplayIf(gP == NULL, "null arg");

	//-------------------------------------------------------------------------
	// Close and unload the flash driver, if one was loaded
	//-------------------------------------------------------------------------
	if (gP->driver.loaded)
		{
		gP->driver.api.FlashClose(gP->driver.refP);
		MemPtrUnlock(gP->driver.entryP);
		gP->driver.loaded = false;
		}
	return (err);

} // PrvFlashDriversUnload



/***************************************************************
 *	Function:	PrvCodeDatabaseOpen 
 *
 *	Summary:      
 *		Opens our code resource database so that we may access our
 *		own resources through PalmOS Data Manager calls.  Maintains
 *		an open count to avoid physically opening the database
 *		more than once in case of nested calls.
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlashDriversLoad()
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvCodeDatabaseOpen (FlmGlobalsType* gP)
{
	DWord		err = 0;


	ErrNonFatalDisplayIf (gP == NULL, "null arg");

	// If it hasn't been opened yet, open it now
	if (gP->common.codeDbOpenCount == 0)
		{
		ErrNonFatalDisplayIf (gP->common.codeDbRef != 0, "logic error");
		gP->common.codeDbRef = DmOpenDatabase (gP->common.codeDbCardNo,
												gP->common.codeDbId,
												dmModeReadOnly);
		if (gP->common.codeDbRef == 0)
			err = DmGetLastErr ();
		}

	// Increment the open count
	if (!err)
		{
		gP->common.codeDbOpenCount++;
		ErrNonFatalDisplayIf(gP->common.codeDbRef == 0, "logic error");
		}

	return (err);

} // PrvCodeDatabaseOpen



/***************************************************************
 *	Function:	PrvCodeDatabaseClose 
 *
 *	Summary:
 *		Close our code resource database that was previously
 *		opened by PrvCodeDatabaseOpen().  Maintains
 *		an open count to avoid physically closing the database
 *		more than once in case of nested calls.
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlashDriversLoad()
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvCodeDatabaseClose (FlmGlobalsType* gP)
{
	DWord		err = 0;


	ErrNonFatalDisplayIf (gP == NULL, "null arg");
	ErrNonFatalDisplayIf (gP->common.codeDbOpenCount == 0, "count underflow");
	ErrNonFatalDisplayIf (gP->common.codeDbRef == 0, "null codeDbRef");


	// Decrement the open count
	gP->common.codeDbOpenCount--;

	// If the count is down to zero, really close the database
	if (gP->common.codeDbOpenCount == 0)
		{
		err = DmCloseDatabase (gP->common.codeDbRef);
		ErrNonFatalDisplayIf(err, "error closing code db");
		gP->common.codeDbRef = 0;
		}


	return (err);

} // PrvCodeDatabaseClose



/***************************************************************
 *	Function:	PrvSectorBackupBuffersAllocate 
 *
 *	Summary:      
 *		Allocate flash sector backup buffers if they haven't been
 *		allocated yet
 *		
 *		We allocate multiple smaller chunks instead of allocating
 *		a single large backup buffer to handle memory fragmentation
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmErase()
 *		PrvFlmWrite()
 *		PrvFlmMove()
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 *    12-Mar-2001	JMP		Modified the FtrPtrNew ret val test
 ****************************************************************/
static DWord
PrvSectorBackupBuffersAllocate (FlmGlobalsType* gP)
{
	DWord	err = 0;
	void*	dataSegP = NULL;

	// If they haven't been allocated yet, allocate them now
	if (gP->backupInfo.numHeapSeg == 0 && gP->backupInfo.numDataSeg == 0)
	  {
		UInt16		numSeg, i;

		// Compute the number of RAM chunks to allocate so that the total
		//  will fit the largest flash chip sector
		numSeg = (UInt16)(gP->driver.maxSectorSize / flmSectorBackupChunkSize);
		if (gP->driver.maxSectorSize % flmSectorBackupChunkSize)
			numSeg++;

		if (numSeg > (flmMaxBackupHeapBuffers + flmMaxBackupDataBuffers))
			{
			  ErrNonFatalDisplayIf (true, "too many backup chunks");
			  err = flmErrIntTooManyBackupChunks;		// should never happen
			  goto Exit;
			}

		gP->backupInfo.segSize = flmSectorBackupChunkSize;
		// Allocate the Heap chunks
		for (i=0; i < numSeg  && i < flmMaxBackupHeapBuffers; i++)
			{
			  ErrNonFatalDisplayIf(gP->backupInfo.seg[i] != NULL, "logic error");
			  gP->backupInfo.seg[i] = MemPtrNew(flmSectorBackupChunkSize);
			  if (gP->backupInfo.seg[i] == NULL)
				{
				  err = flmErrNoMemory;
				  break;
				}
			  gP->backupInfo.numHeapSeg++;
			}

		for (i=0;FtrGet(flmDataBuffCFlMgr, (UInt16) (flmDataBuffFtrBase + i), (UInt32*) dataSegP) == 0; i++)
			{
	  		  ErrNonFatalDisplay ("The DataHeap writeback buffers already existed.");
			  err = FtrPtrFree (flmDataBuffCFlMgr, (UInt16) (flmDataBuffFtrBase + i));
			  if (err)
				goto Exit;
			}

		// Allocate the Data chunks out of the DATA segment using featurePointer for each chunck
		for (i = 0; (flmMaxBackupHeapBuffers + i) < numSeg; i++)
			{
			  ErrNonFatalDisplayIf(gP->backupInfo.seg[flmMaxBackupHeapBuffers + i] != NULL, "logic error");
			  err = FtrPtrNew(flmDataBuffCFlMgr, (UInt16) (flmDataBuffFtrBase + i), (UInt32) flmSectorBackupChunkSize , (MemPtr*) &(dataSegP));
			  gP->backupInfo.seg[flmMaxBackupHeapBuffers + i] = dataSegP;
			  // <chg 2001-3-12 JMP> Modified the FtrPtrNew success test
			  if (err || dataSegP == NULL)
				  {
					err = flmErrNoMemory;
					break;
				  }
			  gP->backupInfo.numDataSeg++;
		  }
	  }

Exit:
	// Clean up on error
	if (err)
		PrvSectorBackupBuffersFree (gP);

	return (err);

} // PrvSectorBackupBuffersAllocate



/***************************************************************
 *	Function:	PrvSectorBackupBuffersFree 
 *
 *	Summary:
 *		Frees sector backup buffers, if any were allocated by
 *		PrvSectorBackupBuffersAllocate().
 *
 *	Parameters:
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		0 if no error
 *  
 *	Called By: 
 *		PrvFlmClose()
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvSectorBackupBuffersFree (FlmGlobalsType* gP)
{
	UInt16		i;
	DWord		err;
	UInt32*		dataSegP = NULL;

	ErrNonFatalDisplayIf (gP == NULL, "null arg");

	// Free the Heap backup blocks
	for (i=0; i < gP->backupInfo.numHeapSeg; i++)
		{
		  err = MemPtrFree (gP->backupInfo.seg[i]);
		  if (err)
			return (err);
		  gP->backupInfo.seg[i] = NULL;
		}
	gP->backupInfo.numHeapSeg = 0;

	//  Free the associated Features buffers allocated by the 
	//  Flash Manager when it requires more than 64K of write
	//  back buffer space (i.e. when the sector size of the flash 
	//  used is greater than 64k.
	for (i=0; i < gP->backupInfo.numDataSeg; i++)
		{
		  if (FtrGet(flmDataBuffCFlMgr, (UInt16) (flmDataBuffFtrBase + i), (UInt32*) dataSegP) == 0) 
			{
			err = FtrPtrFree (flmDataBuffCFlMgr, (UInt16) (flmDataBuffFtrBase + i));
			if (err)
			  return (err);
			}
		  gP->backupInfo.seg[flmMaxBackupHeapBuffers + i] = NULL;
		}
	gP->backupInfo.numDataSeg = 0;


	return (0);

} // PrvSectorBackupBuffersFree



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
 *		gP			IN		Our private globals
 *
 *	Returns:
 *		nothing
 *  
 *	Called By: 
 *		Other Flash Manager functions
 *
 *	Notes:
 *		
 *
 *	History:
 *    4-Apr-1999	VMK		Created
 ****************************************************************/
#if (ERROR_CHECK_LEVEL == ERROR_CHECK_FULL)

static void
PrvErrorCheckGlobals (FlmGlobalsType* gP)
{
	UInt32		i, bufTotal;

	ErrNonFatalDisplayIf(gP == NULL, "null Flm globals");
	ErrNonFatalDisplayIf(gP->startSignature != flmGlobalsStartSignature,
			"bad Flm globals start signature");
	ErrNonFatalDisplayIf(gP->endSignature != flmGlobalsEndSignature,
			"bad Flm globals end signature");

	//-------------------------------------------------------------------------
	// Check client-supplied values
	//-------------------------------------------------------------------------
	ErrNonFatalDisplayIf(gP->common.codeDbId == 0, "no codeDbID");
	ErrNonFatalDisplayIf(gP->common.cardBaseP == NULL, "no cardBaseP");
	ErrNonFatalDisplayIf(gP->common.flashOffset == 0, "no flashOffset");


	//-------------------------------------------------------------------------
	// Check runtime values
	//-------------------------------------------------------------------------


	// Check common flash info for consistency
	if (gP->common.flashEnumerated)
		{
		ErrNonFatalDisplayIf (gP->common.numFlashChips == 0, "no flash chips");

		ErrNonFatalDisplayIf (gP->common.writableOffset < (gP->common.flashOffset
										+ gP->common.readOnlySize),
										"writableOffset too small");

		ErrNonFatalDisplayIf (gP->common.writableOffset >= (gP->common.flashOffset
										+ gP->common.totalFlashSize),
										"writableOffset too big");

		ErrNonFatalDisplayIf (gP->common.totalFlashSize != (gP->common.numFlashChips
										* gP->driver.chipSize),
										"bad totalFlashSize");

		ErrNonFatalDisplayIf (gP->common.writableSize > gP->common.totalFlashSize,
										"bad writableSize");

		}


	// Check flash driver info for consistency
	if (gP->driver.loaded)
		{
		ErrNonFatalDisplayIf (gP->driver.resId < flashCodeResIdFirst,
										"bad driver resId");

		ErrNonFatalDisplayIf (gP->driver.chipSize == 0, "bad chipSize");

		ErrNonFatalDisplayIf (gP->driver.numSectors == 0, "bad numSectors");

		ErrNonFatalDisplayIf (gP->driver.maxSectorSize == 0,
								"bad maxSectorSize");
		ErrNonFatalDisplayIf (gP->driver.maxSectorSize > flmMaxSupportedSectorSize,
								"maxSectorSize too big");

		ErrNonFatalDisplayIf (gP->common.flashBaseP != (Byte*)gP->common.cardBaseP
														+ gP->common.flashOffset,
								"bad flashBaseP");
		ErrNonFatalDisplayIf (gP->common.writableBaseP != (Byte*)gP->common.cardBaseP
														+ gP->common.writableOffset,
								"bad writableBaseP");
		ErrNonFatalDisplayIf (gP->common.writableEndP != (Byte*)gP->common.writableBaseP
														+ gP->common.writableSize,
								"bad writableEndP");
		}


	
	// Check backup Heap buffer info for consistency
	bufTotal = 0;
	for (i=0; i < flmMaxBackupHeapBuffers; i++)
		{
		if (i < gP->backupInfo.numHeapSeg)
			{
			ErrNonFatalDisplayIf (gP->backupInfo.segSize
										!= flmSectorBackupChunkSize,
									"bad backupInfo.segSize");

			ErrNonFatalDisplayIf (gP->backupInfo.seg[i] == NULL,
									"null backup buffer");

			ErrNonFatalDisplayIf (MemPtrSize(gP->backupInfo.seg[i])
										!= gP->backupInfo.segSize,
									"bad backup buffer or size");

			bufTotal += gP->backupInfo.segSize;
			}
		else
			{
			ErrNonFatalDisplayIf (gP->backupInfo.seg[i] != NULL,
									"non-null buf ptr after numSeg");
			}
		}


	// Check backup Data buffer info for consistency
	for (i=0; i < flmMaxBackupDataBuffers; i++)
		{
		if (i < gP->backupInfo.numDataSeg)
			{
			ErrNonFatalDisplayIf (gP->backupInfo.segSize
										!= flmSectorBackupChunkSize,
									"bad backupInfo.segSize");

			ErrNonFatalDisplayIf (gP->backupInfo.seg[flmMaxBackupHeapBuffers + i] == NULL,
									"null backup buffer");

			ErrNonFatalDisplayIf (MemPtrSize(gP->backupInfo.seg[flmMaxBackupHeapBuffers + i])
										!= gP->backupInfo.segSize,
									"bad backup buffer or size");

			bufTotal += gP->backupInfo.segSize;
			}
		else
			{
			ErrNonFatalDisplayIf (gP->backupInfo.seg[flmMaxBackupHeapBuffers + i] != NULL,
									"non-null buf ptr after numSeg");
			}
		} 


	if (gP->backupInfo.numHeapSeg || gP->backupInfo.numDataSeg)
		ErrNonFatalDisplayIf (bufTotal < gP->driver.maxSectorSize,
								"bufTotal too small");

	
	// Check our code database open state
	if (gP->common.codeDbOpenCount != 0)
	  {
		ErrNonFatalDisplayIf (gP->common.codeDbRef == 0, "zero codeDbRef");
	  }
	else
	  {
		ErrNonFatalDisplayIf (gP->common.codeDbRef != 0, "non-zero codeDbRef");
	  }

	return;

} // PrvErrorCheckGlobals

#endif	// full error checking is on
