/***************************************************************
 *
 * Project:
 *    Ken Expansion Card 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashAccessAMD.c
 * 
 * Description:
 *		This file contains the flash device access code for:
 *
 *				AMD Am29LV800B/Am29LV160BB or
 *				Fujitsu equivalent (to AMD 16Mbit) or
 *				Toshiba equivalent (to AMD 16Mbit)
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
 *    09-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *	  17-Apr-2001	vmk	  Added support for SST32HF162/164 and SST32HF802
 *
 ****************************************************************/

#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>


#include "FlashDriver.h"
#include "FlashDriverCommon.h"
#include "FlashDriverAMDPrv.h"

//------------------------------------------------------------------------
// Private function prototypes
//------------------------------------------------------------------------

static DWord
PrvAMDGetManufAndDevice (struct FlashAMDGlobalsType* gP, void* chipBaseP,
						Byte* manufP, Byte* deviceP);
FlashAMDGetManufAndDeviceFuncType	PrvAMDGetManufAndDevice;


static DWord
PrvAMDChipWrite (FlashAMDGlobalsType* gP, Word* chipBaseWordP,
				 Byte* dstByteP, Byte* srcByteP, UInt32 numBytes);
FlashAMDChipWriteFuncType	PrvAMDChipWrite;


static DWord
PrvAMDSectorErase (FlashAMDGlobalsType* gP, void* chipBaseP,
				   UInt32 sectorOffset);
FlashAMDSectorEraseFuncType	PrvAMDSectorErase;


static DWord
PrvAMDSectorIsLocked (FlashAMDGlobalsType* gP, void* chipBaseP,
					  UInt32 sectorOffset, UInt16* lockedP);
FlashAMDSectorIsLockedFuncType	PrvAMDSectorIsLocked;


static Byte
PrvAMDSameDevice (FlashAMDGlobalsType* gP, void* chip0BaseP,
				  void* chip1BaseP);
FlashAMDSameDeviceFuncType		PrvAMDSameDevice;


// Define entry point for builds via Metrowerks compiler
#define FlashAMDAccessEntryPoint __Startup__

// Define entry point for builds via GNU compiler
DWord
start (UInt32 selId, void* argP, UInt32 argSize);
FlashAMDAccessEntryFuncType	start;


DWord
FlashAMDAccessEntryPoint (UInt32 selId, void* argP, UInt32 argSize);
FlashAMDAccessEntryFuncType	FlashAMDAccessEntryPoint;

// This is not present in the OS 3.5 headers, but we need it here.
// Rather than including private HAL headers, we include a definition
// here for the routine.
// ...for reference, this is located in <HwrTimer.h> in the OS 3.5 HAL...

void HwrDelay (UInt32 microseconds)
	SYS_TRAP(sysTrapHwrDelay);


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
  return FlashAMDAccessEntryPoint (selId, argP, argSize);
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
_FlashCmdUnlockBypassEnter (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdUnlockBypassProgram (volatile Word* chipBaseWordP,
							  Word* dstWordP, Word dataWord);
static __inline void
_FlashCmdUnlockBypassReset (volatile Word* chipBaseWordP);

static __inline void
_FlashCmdProgram (volatile Word* chipBaseWordP, Word* dstWordP,
				  Word dataWord);


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
 *	  25-Aug-1999	VMK		Created
 *    09-May-2001   jmp		Added a HwrDelay to be 100% sure 
 *							compatible with the SST Flash Spec.
 ****************************************************************/
__inline void
_FlashCmdReset (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]  = 0x00F0;
  HwrDelay(1); //minimum is really 15 on current VZ
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
 *	  25-Aug-1999	VMK		Created
 ****************************************************************/
__inline void
_FlashCmdAutoSelectEnter (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x5555]  = 0x00AA;
  chipBaseWordP[0x2AAA]  = 0x0055;
  chipBaseWordP[0x5555]  = 0x0090;

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
 *	  25-Aug-1999	VMK		Created
 ****************************************************************/
__inline void
_FlashCmdCFIQueryEnter (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0055]  = 0x0098;

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
 *	  25-Aug-1999	VMK		Created
 ****************************************************************/
__inline void
_FlashCmdSectorErase (volatile Word* chipBaseWordP, DWord wordOffset)
{
  chipBaseWordP[0x5555]	  = 0x00AA;
  chipBaseWordP[0x2AAA]  = 0x0055;
  chipBaseWordP[0x5555]	  = 0x0080;
  chipBaseWordP[0x5555]	  = 0x00AA;
  chipBaseWordP[0x2AAA]	  = 0x0055;
  chipBaseWordP[wordOffset]  = 0x0030;

} // _FlashCmdSectorErase



/***************************************************************
 *	Function:	_FlashCmdUnlockBypassEnter 
 *
 *	Summary:
 *	  Places the flash device in "Unlock Bypass" mode.
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
 *	  This allows the system to program the device faster than
 *	   the standard program command sequence. Once in this mode,
 *	   the UnlockBypassProgram command must be used to program
 *	   the device.  The UnlockBypassReset command must be executed
 *	   to return to reading array data.
 *
 *	History:
 *	  25-Aug-1999	VMK		Created
 ****************************************************************/
__inline void
_FlashCmdUnlockBypassEnter (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0555]  = 0x00AA;
  chipBaseWordP[0x02AA]  = 0x0055;
  chipBaseWordP[0x0555]  = 0x0020;

} // _FlashCmdUnlockBypassEnter


/***************************************************************
 *	Function:	_FlashCmdUnlockBypassProgram 
 *
 *	Summary:
 *	  Programs the flash device in "Unlock Bypass" mode.
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
 *	  The device must be in the UnlockBypass when this command is
 *	   executed (see _FlashCmdUnlockBypassEnter).
 *
 *	History:
 *	  25-Aug-1999	VMK		Created
 ****************************************************************/
__inline void
_FlashCmdUnlockBypassProgram (volatile Word* chipBaseWordP,
							  Word* dstWordP, Word dataWord)
{
  chipBaseWordP[0x0000]  = 0x00A0;
  *dstWordP  = dataWord;

} // _FlashCmdUnlockBypassProgram



/***************************************************************
 *	Function:	_FlashCmdUnlockBypassReset 
 *
 *	Summary:
 *	  Resets the flash device from "Unlock Bypass" mode back to
 *	   reading array data.
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
 *
 *	History:
 *	  25-Aug-1999	VMK		Created
 ****************************************************************/
__inline void
_FlashCmdUnlockBypassReset (volatile Word* chipBaseWordP)
{
  chipBaseWordP[0x0000]  = 0x0090;
  chipBaseWordP[0x0000]  = 0x0000;

} // _FlashCmdUnlockBypassReset


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
 *	  27-Aug-1999	VMK		Created
 *
 ****************************************************************/
__inline void
_FlashCmdProgram (volatile Word* chipBaseWordP, Word* dstWordP,
				  Word dataWord)
{
  chipBaseWordP[0x5555]  = 0x00AA;
  chipBaseWordP[0x2AAA]  = 0x0055;
  chipBaseWordP[0x5555]  = 0x00A0;
  *dstWordP  = dataWord;

} // _FlashCmdProgram






/***************************************************************
 *	Function:	FlashAMDAccessEntryPoint 
 *
 *	Summary:
 *		This is the entry point function for the flash access
 *		code resource.  It fills in the interface structure
 *		with pointers to exported functions.
 *
 *	Parameters:
 *		selId		IN		function selector value (AMDAccessEntrySelEnum)
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
 *    6-Apr-1999	VMK		Created
 ****************************************************************/
DWord
FlashAMDAccessEntryPoint (UInt32 selId, void* argP, UInt32 argSize)
{
  DWord		err = 0;


  switch (selId)
	{
	  case amdAccessEntrySelGetInterface:
		{
		FlashAMDAccessInterfaceType* apiP = (FlashAMDAccessInterfaceType*)argP;

		// Error-check args
		ErrNonFatalDisplayIf (apiP == NULL, "null arg");
		ErrNonFatalDisplayIf (argSize != sizeof(FlashAMDAccessInterfaceType),
							  "invalid arg size");

		// Initialize to all ones so in case we miss one, the call
		//  will bus error at this location
		MemSet(apiP, sizeof(FlashAMDAccessInterfaceType), 0xFF);

		apiP->GetManufAndDevice = &PrvAMDGetManufAndDevice;
		apiP->ChipWrite = &PrvAMDChipWrite;
		apiP->SectorErase = &PrvAMDSectorErase;
		apiP->SectorIsLocked = &PrvAMDSectorIsLocked;
		apiP->SameDevice = &PrvAMDSameDevice;
		}
		break;

	  default:
		ErrNonFatalDisplayIf (true, "unhandled selector");
		err = flashErrInvalidSelId;
		break;
	}


  return (err);

} // FlashAMDAccessEntryPoint



/***************************************************************
 *	Function:	PrvAMDGetManufAndDevice
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
 *    8-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvAMDGetManufAndDevice (struct FlashAMDGlobalsType* gP, void* chipBaseP,
						 Byte* manufP, Byte* deviceP)
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

} // PrvAMDGetManufAndDevice




/***************************************************************
 *	Function:	PrvAMDChipWrite
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
 *    09-Apr-1999	VMK		Created
 *	  01-Jun-1999	vmk		Optimized to reduce number of operations
 *							in word-write loop
 *	  27-Aug-1999	vmk		Use the "standard" program command for
 *							 Toshiba flash
 *
 ****************************************************************/
static DWord
PrvAMDChipWrite (FlashAMDGlobalsType* gP, Word* chipBaseWordP,
					Byte* dstByteP, Byte* srcByteP, UInt32 numBytes)
{
  DWord			err = 0;
  Word			dataWord, dataBit;
  volatile Word*	dstWordP;
  Word			statusWord;
  UInt32			bytesLeft;
  UInt32			currWordsLeft;
  Byte*			currSrcByteP;
  Byte			tmp2Bytes[2];
  //register Boolean		unlockBypassSupport = false;


  // Error-check args
  //  (our caller, FlashChipWrite(), performs extensive error checking
  //  of the args)
  ErrNonFatalDisplayIf (gP == NULL || chipBaseWordP == NULL || dstByteP == NULL
						  || srcByteP == NULL, "null arg");

  // Take care of degenerate case first
  if (numBytes == 0)
	  return (0);

  // Determine whether the "unlock bypass" feature is supported
  //if (gP->device.manufId == flashManuToshiba)
  //	  unlockBypassSupport = false;
  //else
  //	  unlockBypassSupport = true;


  // NOTE 1:
  //
  // We don't have to worry about chip selects here because they are already
  //  set up for read/write by the Handspring Extensions when the card is
  //  inserted


  // NOTE 2:
  //
  // We have to support even as well as odd source and destination
  //  addresses and ranges.  For this reason, we will need to read the source
  //  data into a word variable one byte at a time, to avoid address error
  //  on the Dragonball, which does not allow WORD reads from odd addresses.
  //  And since we can only write at WORD boundaries to the AMD chip, we will
  //  have to take care of the "odd" bytes at beginning and end of the
  //  destination range by "merging" them with the immediately preceding or
  //  following byte.



  // Reset the flash device
  _FlashCmdReset (chipBaseWordP);


  //-----------------------------------------------
  // Place the flash device in "Unlock Bypass" mode
  //-----------------------------------------------
  //if (unlockBypassSupport)
  //	  _FlashCmdUnlockBypassEnter (chipBaseWordP);

  //-----------------------------------------------
  // Program flash
  //-----------------------------------------------
  bytesLeft = numBytes;
  do
	{
	  ErrNonFatalDisplayIf (!bytesLeft, "logic error");

	  //---------------------------------------------------------------------
	  // Set up for writing data in word increments
	  //---------------------------------------------------------------------

	  // If destination starts at an odd address, adjust to point to the
	  //  even address immediately before it
	  if ((DWord)dstByteP & 0x01)
		{
		  dstWordP = (Word*)(dstByteP - 1);
		  tmp2Bytes[0] = *((Byte*)dstWordP);
		  tmp2Bytes[1] = *srcByteP;
		  currSrcByteP = tmp2Bytes;
		  currWordsLeft = 1;
		  bytesLeft--;					  // for next iteration
		  srcByteP++;
		  dstByteP++;

		} // If destination starts at an odd address

	  // If destination starts at an even address
	  else
		{
		  dstWordP = (Word*)dstByteP;

		  // If there is only one byte left
		  if (bytesLeft == 1)
			{
			  tmp2Bytes[0] = *srcByteP;
			  tmp2Bytes[1] = *(dstByteP + 1);
			  currSrcByteP = tmp2Bytes;
			  currWordsLeft = 1;
			  bytesLeft--;					  // for next iteration
			  srcByteP++;
			  dstByteP++;
			}

		  // If more than one byte left
		  else
			{
			  currSrcByteP = srcByteP;

			  // Even # of bytes left
			  currWordsLeft = (bytesLeft & 0xFFFFFFFEUL);

			  srcByteP += currWordsLeft;	  // for next iteration
			  dstByteP += currWordsLeft;	  // for next iteration
			  bytesLeft &= 0x00000001UL;	  // for next iteration
			  
			  // Convert to # of words
			  currWordsLeft /= 2;
			}

		} // If destination starts at an even address
  


	  //---------------------------------------------------------------------
	  // Program one word at a time
	  //---------------------------------------------------------------------
	  do
		{
		  // Grab the next two bytes from source
		  dataWord = ((((Word)currSrcByteP[0]) << 8) & 0xFF00)
					| (((Word)currSrcByteP[1]) & 0x00FF);

		  // Error-check to make sure we're not writing 1's over 0's, which isn't
		  //  supported by this flash device
		  ErrNonFatalDisplayIf ((*dstWordP & dataWord) != dataWord,
							  "attempt to flash 1's over 0's");

		  // Program the word
		  //  DOLATER... for performance optimization, may want to have
		  //  two functions to avoid making this decision inside the loop
		  //if (unlockBypassSupport)
		  //	  _FlashCmdUnlockBypassProgram (chipBaseWordP, (Word*)dstWordP,
		  //									dataWord);
		  //else
		  //	  _FlashCmdProgram (chipBaseWordP, (Word*)dstWordP, dataWord);

		  _FlashCmdProgram (chipBaseWordP, (Word*)dstWordP, dataWord);

		  dataBit = dataWord & 0x0080;

		  // Use Data# Polling to wait for the program operation to complete
		  do
			{
			  // Read the status word
			  statusWord = *dstWordP;

			  // Check if done
			  if ((statusWord & 0x0080) == dataBit)
				  break;

			  // Check for exceeded timing limit
			  
			  if (statusWord & 0x0020)
				{
				  // Recheck because DQ7 may change simulatneously with DQ5
				  statusWord = *dstWordP;
				  if ((statusWord & 0x0080) == dataBit)
					  break;

				  ErrNonFatalDisplay ("write program failed");
				  err = flashErrWriteFailed;
				  goto Exit;
				}

			}
		  while (1); // Use Data# Polling to wait for completion of the program operation

		  
		  // Error-check contents
		  ErrNonFatalDisplayIf (*dstWordP != dataWord, "dst != src after write");


		  // Adjust for next iteration
		  currSrcByteP += 2;
		  dstWordP++;
		  currWordsLeft--;

		}
	  while (currWordsLeft > 0);

	}
  while (bytesLeft > 0);


  // FALL THROUGH TO Exit:

Exit:
  // Leave the "Unlock Bypass" mode
  //if (unlockBypassSupport)
  //    _FlashCmdUnlockBypassReset (chipBaseWordP);

  // Reset the flash device
  _FlashCmdReset (chipBaseWordP);

  ErrNonFatalDisplayIf (err, "flash write error");

  return (err);

} // PrvAMDChipWrite



/***************************************************************
 *	Function:	PrvAMDSectorErase
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
 *    8-Apr-1999	VMK		Created
 *	  2-Jun-1999	vmk		Conserve battery power by suspending the
 *							 CPU while the flash erase algorithm is
 *							 at work.
 *	  13-Aug-1999	vmk		Decrease processor sleep time for flashing
 *							 performance optimization
 *
 ****************************************************************/
static DWord
PrvAMDSectorErase (FlashAMDGlobalsType* gP, void* chipBaseP,
					UInt32 sectorOffset)
{
  DWord		  err = 0;
  UInt32		  wordOffset = sectorOffset / 2;
  Int32		  ticksPerSecond;
  Int32		  incWaitTicks;
  volatile Word  				statusWord;
  volatile Word*	volatile	chipBaseWordP = (Word*)chipBaseP;

  // According to the Am29LV160B spec, the typical sector erase time
  //  is 0.7 sec.  Other supported chips have similar or smaller erase
  //  times. To conserve processor power, we will put the processor
  //  to sleep between checks for completion
  const SDWord  minWaitMilliSec = 40;
  const SDWord  incWaitMilliSec = 30;

  // Get this value once
  ticksPerSecond = SysTicksPerSecond ();

  // Error-check args
  //  (our caller, FlashSectorErase(), performs extensive
  //  error checking of the args)
  ErrNonFatalDisplayIf (gP == NULL || chipBaseWordP == NULL, "null arg");


  // Reset the flash device to reading array data
  _FlashCmdReset ((Word*)chipBaseP);

  // Erase the sector
  _FlashCmdSectorErase ((Word*)chipBaseP, wordOffset);

  // Suspend the processor while flash erase algorithm is at work
  //  (battery power optimization)
  SysTaskDelay ((ticksPerSecond * minWaitMilliSec) / 1000);

  // Use Data# Polling to wait for the erase operation to complete
  incWaitTicks = (ticksPerSecond * incWaitMilliSec) / 1000;
  while (1)
	{
	  // Read device status
	  statusWord = chipBaseWordP[wordOffset];

	  // Check if done
	  if (statusWord & 0x0080)
		  break;

	  // Check for exceeded timing limit
	  if (statusWord & 0x0020)
		{
		  // Recheck because DQ7 may change simulatneously with DQ5
		  statusWord = chipBaseWordP[wordOffset];
		  if (statusWord & 0x0080)
			  break;

		  ErrNonFatalDisplay ("Erase failed");
		  err = flashErrEraseFailed;
		  goto Exit;
		}
	  
	  // Suspend the processor while flash erase algorithm is at work
	  SysTaskDelay (incWaitTicks);

	} // Use Data# Polling to wait for the erase operation to complete

  
  // FALL THROUGH TO Exit:
  

Exit:
  // Reset the flash device
  _FlashCmdReset ((Word*)chipBaseP);

  return (err);

} // PrvAMDSectorErase




/***************************************************************
 *	Function:	PrvAMDSectorIsLocked
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
 *		
 *
 *	History:
 *    8-Apr-1999	VMK		Created
 ****************************************************************/
static DWord
PrvAMDSectorIsLocked (FlashAMDGlobalsType* gP, void* chipBaseP,
					UInt32 sectorOffset, UInt16* lockedP)
{
  DWord		err = 0;
  Word		dataWord;
  UInt32	wordOffset = sectorOffset / 2;
  Word*		chipBaseWordP = (Word*)chipBaseP;


  // Error-check args
  //  (our caller, FlashAttrGet(), performs extensive
  //  error checking of the args)
  ErrNonFatalDisplayIf (gP == NULL || chipBaseWordP == NULL
						  || lockedP == NULL, "null arg");


  // Reset the device to reading array data
  _FlashCmdReset (chipBaseWordP);

  // Place device into the AutoSelect mode
  _FlashCmdAutoSelectEnter (chipBaseWordP);


  // Get the sector's "Protect Verify" status
  dataWord = chipBaseWordP[wordOffset + 2];

  // Reset the device to reading array data
  _FlashCmdReset (chipBaseWordP);

  // Determine if locked
  if ((dataWord & 0x00FF) == 1)
	  *lockedP = true;
  else
	  *lockedP = false;

  return (err);

} // PrvAMDSectorIsLocked



/***************************************************************
 *	Function:	PrvAMDSameDevice
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
 *    25-Aug-1999	VMK		Created
 ****************************************************************/
static Byte
PrvAMDSameDevice (FlashAMDGlobalsType* gP, void* chip0BaseP,
				  void* chip1BaseP)
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
  //  addresses on device #1 -- if they are the same, then the different addresses must both
  //  refer to the same physical device
  for (wordOffset = 0x10; wordOffset <= 0x3C; wordOffset++)
	{
	  cfi1Data = chip1BaseWordP[wordOffset];
	  cfi0Data = chip0BaseWordP[wordOffset];
	  if (cfi1Data != cfi0Data)
		{
		  same = false;
		  goto Exit;
		}
	}

  for (wordOffset = 0x40; wordOffset <= 0x4C; wordOffset++)
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

} // PrvAMDSameDevice


