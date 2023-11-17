/***************************************************************
 *
 * Project:
 *    Ken Expansion Card
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 2000.
 *
 * FileName:
 *    FlashAccessIntel.c
 *
 * Description:
 *		This file contains the flash device access code for:
 *
 *				Intel INTEL E28F320J3 E28F640J3 E28F128J3
 *
 *		Contents of this file will be built as a separate code resource
 *		that will be copied to RAM when needed.  This scheme is necessary
 *		because the Flash Driver may be executing from the same chip
 *		that is being modified or polled for information.
 *
 *		This code resource must have the resource type flashAccessCodeResType
 *		(defined in FlashDriver.h).  Its resource id must be the same
 *		as the resource id of the corresponding Flash Driver code resource.
 *
 * ToDo:
 *
 * History:
 *    5-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/

#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>

#include "FlashDriver.h"
#include "FlashDriverCommon.h"
#include "FlashDriverIntelPrv.h"

#define prvWriteBuffSize	  32			 /* Size of the Write Buffer must be a power of 2*/
#define	prvWriteBuffFragment	(prvWriteBuffSize - 1)

#define min(a,b) (a<b) ? (a) : (b)

//------------------------------------------------------------------------
// Private function prototypes
//------------------------------------------------------------------------

static DWord
PrvIntelGetManufAndDevice (struct FlashIntelGlobalsType* gP, void* chipBaseP,
						Byte* manufP, Byte* deviceP);
FlashIntelGetManufAndDeviceFuncType	PrvIntelGetManufAndDevice;


static DWord
PrvIntelChipWrite (FlashIntelGlobalsType* gP, Word* chipBaseWordP,
				 Byte* dstByteP, Byte* srcByteP, UInt32 numBytes);
FlashIntelChipWriteFuncType	PrvIntelChipWrite;


static DWord
PrvIntelSectorErase (FlashIntelGlobalsType* gP, void* chipBaseP,
				   UInt32 sectorOffset);
FlashIntelSectorEraseFuncType	PrvIntelSectorErase;


static DWord
PrvIntelSectorIsLocked (FlashIntelGlobalsType* gP, void* chipBaseP,
					  UInt32 sectorOffset, UInt16* lockedP);
FlashIntelSectorIsLockedFuncType	PrvIntelSectorIsLocked;


static Byte
PrvIntelSameDevice (FlashIntelGlobalsType* gP, void* chip0BaseP,
				  void* chip1BaseP);
FlashIntelSameDeviceFuncType		PrvIntelSameDevice;


// Define entry point for builds via Metrowerks compiler
#define FlashIntelAccessEntryPoint __Startup__

// Define entry point for builds via GNU compiler
DWord
start (UInt32 selId, void* argP, UInt32 argSize);
FlashIntelAccessEntryFuncType	start;


DWord
FlashIntelAccessEntryPoint (UInt32 selId, void* argP, UInt32 argSize);
FlashIntelAccessEntryFuncType	FlashIntelAccessEntryPoint;


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
  return FlashIntelAccessEntryPoint (selId, argP, argSize);
}





//-----------------------------------------------
// Inline function prototypes
//-----------------------------------------------

static __inline void
_FlashCmdReset (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdAutoSelectEnter (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdCFIQueryEnter (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdSectorErase (volatile Word* chipBaseWordP, DWord wordOffset);

static __inline void
_FlashCmdBlockUnlock (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdReadStatusRegister (volatile Word* chipBaseWordP, DWord wordOffset, volatile Word* statusWordP);

static __inline void
_FlashCmdClearStatusRegister (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdProgram (volatile Word* chipBaseWordP, volatile Word* dstWordP, Word dataWord);

static __inline Int16
_FlashCmdWriteBufferInit (volatile Word* chipBaseWordP, DWord wordOffset, DWord tickPerSec);

static __inline Int16
_FlashWaitTillDone(volatile Word* chipBaseWordP, DWord waitTicks, Word errCheck);


/***************************************************************
 *	Function:	_FlashCmdReset
 *
 *	Summary:
 *	  Resets the flash device to Reading Array Data.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  Must be issued to get out of AutoSelect mode and if DQ5
 *	   goes high during a program or erase operation.
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdReset (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]  = 0x00FF;

} // _FlashCmdReset


/***************************************************************
 *	Function:	_FlashCmdAutoSelectEnter
 *
 *	Summary:
 *	  Places the device in AutoSelect mode.  This mode allows
 *	   us to access the Manufcaturer/Device codes and determine
 *	   protected status of sectors.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  The Reset command must be written to return to reading array
 *	   data.
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdAutoSelectEnter (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]  = 0x0090;

} // _FlashCmdAutoSelectEnter


/***************************************************************
 *	Function:	_FlashCmdCFIQueryEnter
 *
 *	Summary:
 *	  Places the device in the CFI Query mode.  This mode allows
 *	   us to access CFI information.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  The Reset command must be written to return to reading array
 *	   data.  The CFI Query mode may be entered directly from the
 *	   AutoSelect mode, in which case the Reset command will return
 *	   device back to the AutoSelect mode (not the reading array data
 *	   mode)
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdCFIQueryEnter (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]  = 0x0098;

} // _FlashCmdCFIQueryEnter


/***************************************************************
 *	Function:	_FlashCmdSectorErase
 *
 *	Summary:
 *	  Issues the Sector Erase command.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  After the Sector Erase command is written, the system must
 *	   wait for the erase operation to complete before issuing
 *	   other commands.
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdSectorErase (volatile Word* chipBaseWordP, DWord wordOffset)
{
  chipBaseWordP[wordOffset]	 = 0x0020;
  chipBaseWordP[wordOffset]  = 0x00D0;

} // _FlashCmdSectorErase


/***************************************************************
 *	Function:	_FlashCmdBlockUnlock
 *
 *	Summary:
 *	  Issues the Clear Block Lock-Bit Cmd.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  This unlocks the whole chip, No sector locking/Unlocking 
 *	  available for the StrataFlash.
 *	  
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdBlockUnlock (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]	 = 0x0060;
  chipBaseWordP[0x0000]  = 0x00D0;

} // _FlashCmdBlockUnlock


/***************************************************************
 *	Function:	_FlashCmdReadStatusRegister
 *
 *	Summary:
 *	  Reads the Status Register.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  The status register may be read to determine when a block erase,
 *    program, or lock-bit configuration is complete and whether
 *    the operation completed successfully.
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdReadStatusRegister (volatile Word* chipBaseWordP, DWord wordOffset, volatile Word* statusWordP)
{
  // Set the register
  chipBaseWordP[0x0000]  = 0x0070;
  *statusWordP = chipBaseWordP[wordOffset];

} // _FlashCmdReadStatusRegister

/***************************************************************
 *	Function:	_FlashCmdClearStatusRegister
 *
 *	Summary:
 *	  Clears the Status Register.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *	  Status register bits SR.5, SR.4, SR.3, and SR.1 are set to
 *   “1”s by the Write State Machine and can only be reset by the 
 *    Clear Status Register command.
 *
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
__inline void
_FlashCmdClearStatusRegister (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]  = 0x0050;

} // _FlashCmdClearStatusRegister



/***************************************************************
 *	Function:	_FlashCmdProgram
 *
 *	Summary:
 *	  Programs the flash device in "standard" mode.
 *
 *	Parameters:
 *
 *	Returns:
 *	  nothing
 *
 *	Called By:
 *
 *
 *	Notes:
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
__inline void
_FlashCmdProgram (volatile Word* chipBaseWordP, volatile Word* dstWordP, Word dataWord)
{
  chipBaseWordP[0x00000]  = 0x0010;
  *dstWordP  = dataWord;

} // _FlashCmdProgram


/***************************************************************
 *	Function:	_FlashCmdWriteBufferInit
 *
 *	Summary:
 *	  Initiate the Write to Buffer Cmd.
 *
 *	Parameters:
 *
 *	Returns:
 *	  0 on successfull; (-1) on fail;
 *
 *	Called By:
 *
 *
 *	Notes:
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
__inline  
Int16 _FlashCmdWriteBufferInit (volatile Word* chipBaseWordP, DWord wordOffset,
								DWord tickPerSec)
{
Word	statusWord;
Int32	timer=0;
	

 	while(1) 
	  {
	  	// Issue Write to Buffer Cmd
		chipBaseWordP[wordOffset]  = 0x00E8;

		// Get the Status
		_FlashCmdReadStatusRegister (chipBaseWordP, wordOffset, &statusWord);

		if (statusWord & 0x0080)  // done, no error
		  {
			return (0);	
		  }
		//	The chances are we only get here to check a bad status if something 
		//	is wrong, so it is better not to call TimGetTicks if we don't have 
		//	to, On the other hand if something is wrong then it probably won't 
		//	matter. 
		//	NOTE that if the initialization is slightly delayed, the first 
		//	time through the loop will take very little time, and on the next loop 
		//	we will then pass the status Test. The WriteToBuffer flowchart is very 
		//	specific on using a timeout...
		if (!timer)
		  {
			// We are waiting a maximum of a second
			timer = TimGetTicks();
		  }
		if (tickPerSec > TimGetTicks() - timer)
		  {
			ErrNonFatalDisplay ("Error In Issue Write to Buffer Cmd");
			return (-1);	// done - failed!
		  }
	  }			
} // _FlashCmdWriteBufferInit



/***************************************************************
 *	Function:	_FlashWaitTillDone
 *
 *	Summary:
 *	  Wait until the flash device is "done" by checking the SR.
 *
 *	Parameters:
 *
 *	Returns:
 *	  0 on successfull; (-1) on fail;
 *
 *	Called By:
 *
 *
 *	Notes:
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
 __inline 
Int16 _FlashWaitTillDone(volatile Word* chipBaseWordP, DWord waitTicks,
						 Word errCheck)
{
Word statusWord;
	// wait for the Operation to complete
	while (1) {

		// Read device status
		_FlashCmdReadStatusRegister (chipBaseWordP, 0, &statusWord);

		if (statusWord & 0x0080)  { // done, no error
			break;					// exit while
		}
		if ((statusWord & 0x0080) == 0) {

			// Suspend the processor while flash erase algorithm is at work
			if (waitTicks) //only if waitTicks != 0
				SysTaskDelay (waitTicks);
			continue;
		}

		if (statusWord & errCheck) {
			ErrNonFatalDisplay ("Error In Access");
			return (-1);	// done - failed!
		}

		// Check if done
	}	
		
	return (0);
}	// _FlashWaitTillDone



/****************************************************************
 *	Function:	PrvFlashWriteBuffered
 *	
 *	Summary:
 *	 helper function to write all the whole buffers to flash
 *	 (it assumes that the destination address is on a buffer boundary and that
 *	 it is asked to write an integral number of buffers, so it doesn't need to deal
 *	 with special cases pertaining to destination; however, it needs to deal with
 *	 the odd source address case) 
 *	
 *	Parameters:
 *	  FlashIntelGlobalsType* gP			
 *	  Word* chipBaseWordP			
 *	  Byte* dstByteP			
 *	  Byte* srcByteP			
 *	  UInt32 numBytes			
 *	
 *	Returns:
 *	  static DWord 
 *	
 *	Called by: _PrvIntelChipWrite
 *	  
 *	
 *	Notes:
 *	  
 *	
 *	History:
 *	  2001-1-16 	JMP	Created
 *		
 *****************************************************************/
 __inline static DWord 
_PrvFlashWriteBuffered(FlashIntelGlobalsType* gP, Word* chipBaseWordP, 
					  Byte* dstByteP, Byte* srcByteP, UInt32 currBuffsLeft)
{
  DWord				err = 0;
  volatile Word*	dstWordP;
  // used to point to either the inBuff.dataWord or an updated (Word*) srcByteP
  volatile Word*	srcWordP = 0;
  Word				i=0;
  UInt32			wordOffset; 
  union
	{
	  Byte			dataByte[prvWriteBuffSize]; 
	  Word			dataWord[prvWriteBuffSize/2]; 
	} inBuff;


  while (currBuffsLeft)
	{
	  dstWordP = (Word*)dstByteP; //Set the destination Ptr
	  wordOffset = dstWordP - chipBaseWordP;

	  if ((DWord)srcByteP & 0x01) // Src is misaligned use the buffer
		{
		  for ( i = 0; i< prvWriteBuffSize; i++)		
			 inBuff.dataByte[i] =  *(srcByteP + i);
		  srcWordP = &inBuff.dataWord[0];
		}
	  else
		{
		  srcWordP = (Word*)srcByteP;
		}

	  if (_FlashCmdWriteBufferInit (chipBaseWordP, wordOffset, gP->ticksPerSecond)) 
		{	
		  ErrNonFatalDisplay ("write program failed");
		  err = flashErrWriteFailed;
		  goto Exit;
		}

	  //Write the wordCount 0 based, so we put 0x1F 
	  chipBaseWordP[wordOffset]  = (Word)((prvWriteBuffSize/ 2) - 1);
	  
	  //Write the Data
	  for ( i = 0; i< prvWriteBuffSize/2; i++)	
		*(dstWordP++) =  *(srcWordP++);	  //Write the Words				
		
	  // confirm buffer to flash
	  chipBaseWordP[wordOffset]  = 0x00D0;

	  //Done With writting, Now Check to see that SR7 is OK
	  if (_FlashWaitTillDone(chipBaseWordP, 0, 0x0010) ) 	// program error
		{
		  ErrNonFatalDisplay ("write program failed");
		  err = flashErrWriteFailed;
		  goto Exit;
		}		

	  #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
		  //We better get into read Array mode before comparing anything
		  _FlashCmdReset (chipBaseWordP); 
		  // Error-check contents
		 for ( i = 1; i<= (prvWriteBuffSize/2); i++)	
		  {
			ErrNonFatalDisplayIf (*(dstWordP-i) != *(srcWordP-i), 
								"Bad Word Copy in _PrvFlashWriteBuffered");
		  }
	  #endif

	  // Update the counters for the next operation
	  srcByteP += (DWord)prvWriteBuffSize;
	  dstByteP += (DWord)prvWriteBuffSize; 
	  currBuffsLeft--;

	}

Exit:
  _FlashCmdReset (chipBaseWordP); 

  return (err);
} //_PrvFlashWriteBuffered


/****************************************************************
 *	Function:	_PrvFlashWriteUnbufferred
 *	
 *	Summary:
 *	  Helper function to write a data to the the flash 1 word at
 *	  a time. This routine is used to write less than 32 Byte, and
 *	  takes care of boundary cases such as odd number of byes to
 *	  copy, and misaligned destination address.
 *	  
 *	
 *	Parameters:
 *	  FlashIntelGlobalsType* gP			
 *	  Word* chipBaseWordP			
 *	  Byte* dstByteP			
 *	  Byte* srcByteP			
 *	  UInt32 numBytes			
 *	
 *	Returns:
 *	  static DWord 
 *	
 *	Called by:	PrvIntelChipWrite
 *	  
 *	
 *	Notes:
 *	  
 *	
 *	History:
 *	  2001-1-16 	JMP	Created
 *		
 *****************************************************************/
 __inline 
static DWord 
_PrvFlashWriteUnbufferred(FlashIntelGlobalsType* gP, Word* chipBaseWordP, 
						  Byte* dstByteP, Byte* srcByteP, UInt32 numBytes)
{
  DWord				err = 0;
  volatile Word*	dstWordP = 0;
  Word*				srcWordP;	//used to point either the inBuff.dataWord
  UInt32			i = 0; 
  UInt32			paddingCnt = 0; 
  UInt32			bytesToCopy = 0;
  union
	{
	  Byte		dataByte[prvWriteBuffSize]; 
	  Word		dataWord[prvWriteBuffSize/2]; 
	} inBuff;


	ErrNonFatalDisplayIf (numBytes >= prvWriteBuffSize,
						  "Too Many bytes For Unbufferred Write");

	// Set the Destination Word Pointer
	if ((DWord)dstByteP & 0x01) // Destination starts at an odd address
	  {	
		//	Set the srcWordP to point to dataWord = to the previous 
		//	Flash Byte and the next srcByte
		dstWordP = (Word*)(dstByteP - 1);
		inBuff.dataByte[paddingCnt] = *((Byte*)dstWordP);
		paddingCnt++;
	  }
	else //	No Padding In Front
	  {
		dstWordP = (Word*)(dstByteP);
	  }
	
	//	Copy the Data to the Buffer
	for ( i=0; i< numBytes; i++)		  
	   inBuff.dataByte[paddingCnt + i] =  *(srcByteP++);

	//Set The Source Word Pointer
	srcWordP = &inBuff.dataWord[0];

	// If we have an odd number of bytes we should pad with the
	// next byte from the destination
	if ( (numBytes + paddingCnt) & 0x01)							 
	  {					
		inBuff.dataByte[numBytes + paddingCnt] =  *(dstByteP + numBytes);
		//Increase the byte count to copy
		paddingCnt++; 
	  }
	
	//Total the byte count to copy
	bytesToCopy = numBytes + paddingCnt; 
	//now write the buffer using the Word Write
	for ( i = 0; i< (bytesToCopy/2); i++)	
	  {	
		_FlashCmdProgram (chipBaseWordP, (Word*)(dstWordP), *(srcWordP));
		if (_FlashWaitTillDone(chipBaseWordP, 0, 0x0010) ) 	// program error
		  {
			ErrNonFatalDisplay ("write program failed");
			err = flashErrWriteFailed;
			goto Exit;
		  }
		#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
		  // We need to get into read Array mode before comparing anything
		  _FlashCmdReset (chipBaseWordP); 
		  ErrNonFatalDisplayIf((Word)*(dstWordP) != (Word)*(srcWordP), 
								 "Bad Word Copy in _PrvFlashWriteUnbufferred");
		#endif
		dstWordP++;
		srcWordP++;
	  }

Exit:

	  _FlashCmdReset (chipBaseWordP); 
	 return (err);
} //_PrvFlashWriteUnbufferred

/***************************************************************
 *	Function:	FlashIntelAccessEntryPoint
 *
 *	Summary:
 *		This is the entry point function for the flash access
 *		code resource.  It fills in the interface structure
 *		with pointers to exported functions.
 *
 *	Parameters:
 *		selId		IN		function selector value (IntelAccessEntrySelEnum)
 *		argP		IN/OUT	pointer to argument structure (type
 *							 and size depend on selId)
 *		argSize		IN		size of argument structure
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
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
DWord
FlashIntelAccessEntryPoint (UInt32 selId, void* argP, UInt32 argSize)
{
  DWord		err = 0;


  switch (selId)
	{
	  case IntelAccessEntrySelGetInterface:
		{
		FlashIntelAccessInterfaceType* apiP = (FlashIntelAccessInterfaceType*)argP;

		// Error-check args
		ErrNonFatalDisplayIf (apiP == NULL, "null arg");
		ErrNonFatalDisplayIf (argSize != sizeof(FlashIntelAccessInterfaceType),
							  "invalid arg size");

		// Initialize to all ones so in case we miss one, the call
		//  will bus error at this location
		MemSet(apiP, sizeof(FlashIntelAccessInterfaceType), 0xFF);

		apiP->GetManufAndDevice = &PrvIntelGetManufAndDevice;
		apiP->ChipWrite = &PrvIntelChipWrite;
		apiP->SectorErase = &PrvIntelSectorErase;
		apiP->SectorIsLocked = &PrvIntelSectorIsLocked;
		apiP->SameDevice = &PrvIntelSameDevice;
		}
		break;

	  default:
		ErrNonFatalDisplayIf (true, "unhandled selector");
		err = flashErrInvalidSelId;
		break;
	}


  return (err);

} // FlashIntelAccessEntryPoint



/***************************************************************
 *	Function:	PrvIntelGetManufAndDevice
 *
 *	Summary:
 *		Gets manufacturer and device ID from the flash device at
 *		 chipBaseP.
 *
 *	Parameters:
 *		chipBaseP	IN		Pointer to base of chip
 *		manufP		OUT		For returning manufcturer ID
 *		deviceP		OUT		For returning device ID
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
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvIntelGetManufAndDevice (struct FlashIntelGlobalsType* gP, void* chipBaseP, Byte* manufP, Byte* deviceP)
{
  DWord	err = 0;
  volatile Word*	chipBaseWordP = (Word*)chipBaseP;


  // Reset the device to reading array data
  _FlashCmdReset ((Word*)chipBaseP);

  // Switch to the AutoSelect mode
  _FlashCmdAutoSelectEnter ((Word*)chipBaseP);

  // Get the manufacturer and device IDs
  *manufP	= (Byte)(chipBaseWordP[0] & 0x00FF);
  *deviceP	= (Byte)(chipBaseWordP[1] & 0x00FF);

  // Reset the device
  _FlashCmdReset ((Word*)chipBaseP);

  return (err);

} // PrvIntelGetManufAndDevice


/***************************************************************
 *	Function:	PrvIntelChipWrite
 *
 *	Summary:
 *		Programs data within a single flash device.
 *
 *		***IMPORTANT***
 *		The entire destination region must be within the single flash
 *		chip pointed to by chipBaseWordP.
 *
 *
 *	Parameters:
 *		gP				IN		Flash Driver's private globals
 *		chipBaseWordP	IN		Pointer to base of flash chip
 *		dstByteP		IN		Pointer to destination of write operation
 *		srcByteP		IN		Pointer to source of write opreation
 *		numBytes		IN		Number of bytes to write to flash
 *
 *	Returns:
 *		0 if both manufacturer and device were recognized and
 *		are supported by this driver; error code otherwise
 *
 *	Called By:
 *		FlashChipWrite()
 *
 *	Notes:
 *
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
static DWord
PrvIntelChipWrite (FlashIntelGlobalsType* gP, Word* chipBaseWordP, 
					Byte* dstByteP, Byte* srcByteP, UInt32 numBytes)
{
  DWord				err = 0;
  UInt32			bytesLeft;
  UInt32			fullBuffsToWrite = 0;
  UInt32			bytesFromPrevBoundary;
  UInt32			bytesToNextBoundary;
  UInt32			bytesToWrite;

  #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
  Byte*				savedsrcByteP = srcByteP;
  Byte*				saveddstByteP = dstByteP;
  UInt32			savednumBytes = numBytes;
  #endif


  //  (our caller, FlashChipWrite(), performs extensive error checking
  //  of the args)
  ErrNonFatalDisplayIf (gP == NULL || chipBaseWordP == NULL || dstByteP == NULL
						  || srcByteP == NULL, "null arg");

  // Take care of degenerate case first
  if (numBytes == 0)
	  return (0);

  // NOTE 1:
  //
  // We don't have to worry about chip selects here because they are already
  //  set up for read/write by the Handspring Extensions when the card is
  //  inserted


  // NOTE 2:
  //
  //  We have to support even as well as odd source and destination
  //  addresses and ranges.  For this reason, we will need to read the source
  //  data into a buffer variable (single Word or 16 Word) one byte at a time, 
  //  to avoid address error on the Dragonball, which does not allow WORD reads
  //  from odd addresses.
  //  And since we can only write at WORD boundaries to the Intel chip, we will
  //  have to take care of the "odd" bytes at beginning and end of the
  //  destination range by "merging" them with the immediately preceding or
  //  following byte. 
  //  We Also want to take advantage of the 32 byte Buffer write so we need
  //  to align ourselves to the 16 byte flash boundary as quickly as possible. 
  //  Also to optimize for speed we will only make use of buffered data when the
  //  source is mis-aligned and/or when we have an odd destination Pointer. 
  //  In other cases, full 32 Bytes buffer with no src or dst misalignement
  //  we will use pointer arithmetic.


  //-----------------------------------------------
  // Program flash
  //-----------------------------------------------
  bytesLeft = numBytes;
  ErrNonFatalDisplayIf (!bytesLeft, "logic error");

  //---------------------------------------------------------------------
  // Set up for writing data in word increments
  //---------------------------------------------------------------------
  //Find out how far we are from a write to Buffer boundary (every 32 Bytes)
  bytesFromPrevBoundary =  (Byte)((DWord)dstByteP & prvWriteBuffFragment); 
  ErrNonFatalDisplayIf (bytesFromPrevBoundary > prvWriteBuffSize, 
						"Too many bytes to reach next Write Boundary");
  // If destination starts at an odd address or we need to adjust 
  //  to the next buffer write boundary,adjust to point to the next 1F boundary.

 if (bytesFromPrevBoundary > 0)
	{
	  //Write boundary every 32 Bytes
	  bytesToNextBoundary = prvWriteBuffSize - bytesFromPrevBoundary;

	  bytesToWrite= min(bytesLeft, bytesToNextBoundary);
	  err = _PrvFlashWriteUnbufferred (gP, chipBaseWordP, dstByteP, srcByteP,
									   bytesToWrite);
	  if (err)
		goto Exit;
	  // Update the counters for the next operation
	  bytesLeft -= bytesToWrite;					  
	  srcByteP += bytesToWrite;
	  dstByteP += bytesToWrite;
	}
 
  // Write full buffers, if any, starting at buffer boundary - 
  // this is a StrataFlash performance optimization
  fullBuffsToWrite = bytesLeft / (DWord)prvWriteBuffSize;

 if (fullBuffsToWrite > 0)
	{

	  bytesToWrite = fullBuffsToWrite * (DWord)prvWriteBuffSize;
 	  // Call a helper function to write all the whole buffers to flash
	  // (it assumes that the destination address is on a buffer boundary
	  // and thatit is asked to write an integral number of buffers, so it 
	  // doesn't need to deal with special cases pertaining to destination; 
	  // however, it needs to deal with the odd source address case)
	  err = _PrvFlashWriteBuffered (gP, chipBaseWordP, dstByteP, 
									srcByteP, fullBuffsToWrite);
	  if (err)
		  goto Exit;

	  // Update counters for the next operation
	  srcByteP += bytesToWrite;
	  dstByteP += bytesToWrite;
	  bytesLeft -= bytesToWrite;
	}


  // Write remaining bytes, if any
  if (bytesLeft > 0)
	{
	  bytesToWrite = bytesLeft;
	  // Call a helper function to write out the data one word at a time - it
	  // takes care of odd address and other border conditions
	  err = _PrvFlashWriteUnbufferred (gP, chipBaseWordP, dstByteP, 
									  srcByteP, bytesToWrite);
	  bytesLeft -= bytesToWrite;
	  if (err)
		  goto Exit;
	}
Exit:

  // Reset the flash device
  _FlashCmdReset (chipBaseWordP);

  #if  ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
		// Check and see if we wrote the preoper bytes
	while (savednumBytes--)
	  {
		ErrNonFatalDisplayIf (*savedsrcByteP != *saveddstByteP, 
							  "Bad Byte Copy in PrvIntelChipWrite"); 
		savedsrcByteP++;
		saveddstByteP++; 
	  }
  #endif

  ErrNonFatalDisplayIf (err, "flash write error");

  return (err);

} // PrvIntelChipWrite


/***************************************************************
 *	Function:	PrvIntelSectorErase
 *
 *	Summary:
 *		Erases a single sector on the flash device.
 *
 *	Parameters:
 *		gP				IN		Flash Driver's private globals
 *		chipBaseP		IN		Pointer to base of flash chip
 *		sectorOffset	IN		Byte offset to base of sector to erase
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
 *    5-Oct-2000 - Created by Jan Piper
 *
 ****************************************************************/
static DWord
PrvIntelSectorErase (FlashIntelGlobalsType* gP, void* chipBaseP, 
					 UInt32 sectorOffset)
{
  DWord		  err = 0;
  UInt32	  wordOffset = sectorOffset / 2;
  UInt32	  incWaitTicks;

  // According to the E28F640J3 spec, the typical sector erase time
  //  is 1 sec.  Other supported chips have similar or smaller erase
  //  times. To conserve processor power, we will put the processor
  //  to sleep between checks for completion
  const SDWord  minWaitMilliSec = 800;
  const SDWord  incWaitMilliSec = 30;


  // Error-check args
  //  (our caller, FlashSectorErase(), performs extensive
  //  error checking of the args)
  ErrNonFatalDisplayIf (gP == NULL || chipBaseP == NULL, "null arg");

  // Reset the flash device to reading array data
  _FlashCmdReset ((Word*)chipBaseP);

  // Erase the sector
  _FlashCmdSectorErase ((Word*)chipBaseP, wordOffset );

  // Suspend the processor while flash erase algorithm is at work
  //  (battery power optimization)
  SysTaskDelay ((gP->ticksPerSecond * minWaitMilliSec) / 1000);
  // Use Data# Polling to wait for the erase operation to complete
  incWaitTicks = (gP->ticksPerSecond * incWaitMilliSec) / 1000;

  if (_FlashWaitTillDone((volatile unsigned short*)chipBaseP, incWaitTicks, 0x0032) ) 	// erase errors
  	{
	  ErrNonFatalDisplay ("Erase failed In Access");
	  _FlashCmdClearStatusRegister((Word*)chipBaseP);
	  err = flashErrEraseFailed;
	}


  // Reset the flash device to read array mode
  _FlashCmdReset ((Word*)chipBaseP);

  return (err);

} // PrvIntelSectorErase




/***************************************************************
 *	Function:	PrvIntelSectorIsLocked
 *
 *	Summary:
 *		Checks if the given sector is locked.
 *
 *	Parameters:
 *		gP				IN		Flash Driver's private globals
 *		chipBaseP		IN		Pointer to base of flash chip
 *		sectorOffset	IN		Pointer to base of sector to erase
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		Clients of Flash Driver;
 *
 *	Notes:
 *		This feature is not available on the E28F...J3 
 *		so just unlock the whole chip and return.
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static DWord
PrvIntelSectorIsLocked (FlashIntelGlobalsType* gP, void* chipBaseP, 
						 UInt32 sectorOffset, UInt16* lockedP)
{
  //  for the E28F...J3 Intel Family Flash.
  //  Locking and Unlocking is only supported for the whole chip.
  //  Also, qwery status is not supported 
	return (flmErrOperationNotSupported);    

} // PrvIntelSectorIsLocked



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
 *		Clients of Flash Access;
 *
 *	Notes:
 *
 *
 *	History:
 *    5-Oct-2000 - Created by Jan Piper
 ****************************************************************/
static Byte
PrvIntelSameDevice (FlashIntelGlobalsType* gP, void* chip0BaseP, void* chip1BaseP)
{
  Byte		same = true;
  UInt32	wordOffset;
  Word*		chip0BaseWordP = (Word*)chip0BaseP;
  Word*		chip1BaseWordP = (Word*)chip1BaseP;
  Word		cfi0Data, cfi1Data;


  // Error-check args
  //  (our caller, FlashAttrGet(), performs extensive
  //  error checking of the args)
  ErrNonFatalDisplayIf (!gP || !chip0BaseP || !chip1BaseP, "null arg");


  // Reset both (hypothetical) devices to reading array data
  _FlashCmdReset (chip0BaseWordP);
  _FlashCmdReset (chip1BaseWordP);

  // Place device #1 into the AutoSelect mode (get them into different
  //  modes to make the test more reliable)
  _FlashCmdAutoSelectEnter (chip1BaseWordP);

  // Place device #0 into the CFI Query mode
  //  (we must do this after placing device #1 into AutoSelect
  //  mode because the CFI mode may be entered directly from the AutoSelect
  //  mode in the event they are the same physical device)
  _FlashCmdCFIQueryEnter (chip0BaseWordP);


  // Compare the CFI info of device #0 with the data at the same
  //  addresses on device #1 -- if they are the same, then the different 
  //  addresses must both refer to the same physical device
  for (wordOffset = 0x10; wordOffset <= 0x46; wordOffset++)
	{
	  cfi1Data = chip1BaseWordP[wordOffset];
	  cfi0Data = chip0BaseWordP[wordOffset];
	  if (cfi1Data != cfi0Data)
		{
		  same = false;
		  goto Exit;
		}
	}


Exit:
  // Reset both (or the same) device(s) to reading array data
  //  (in the event they were the same device, the both resets
  //  are necessary -- the first to get out of the CFI Query mode,
  //  and the second to get out of AutoSelect mode)
  _FlashCmdReset (chip0BaseWordP);
  _FlashCmdReset (chip1BaseWordP);


  return (same);

} // PrvIntelSameDevice


