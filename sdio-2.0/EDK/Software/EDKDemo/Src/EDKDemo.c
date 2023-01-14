/***********************************************************************
 *
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *
 ***********************************************************************/

/***********************************************************************
 *
 * EDK Demo EDKDemo.c Src File.
 *
 ***********************************************************************/

#include <PalmOS.h>
#include <StdIOPalm.h>
#include "EDKDemoRsc.h"
#include <VFSMgr.h>
#include <SDIO.h>
#include "CTable.h"



/***********************************************************************
 *      External References
 ***********************************************************************/

extern UInt16  gCardTableCnt;	// no. of text lines in Table


/***********************************************************************
 *      SDIO Card Constants
 ***********************************************************************/

#define	kNumSdioFunctions	1	// Number of SDIO functions on the EDK card

#define kMaxDirPathLen		( 255 )
#define kMaxDirPathSize		( 1 + kMaxDirPathLen )
#define kMaxDirDispNameLen	( 255 )
#define kMaxDirDispNameSize	( 1 + kMaxDirDispNameLen )

#define	kMaxFileSuffixLen	( 15 )		// max size of ".jpg", etc.
#define	kMaxFileSuffixSize	( 1 + kMaxFileSuffixLen )


/***********************************************************************
 *		Slot and Volume Data Structures
 *
 *	gSlotInfo[ kMaxSlots ]	- Information gathered at run-time about
 *							  every non-empty slot in the palmOne Handheld
 *	gVolInfo[ kMaxVolumes ]	- Information gathered at run-time about
 *							  every volume in the palmOne Handheld
 ***********************************************************************/

#define	kMaxSlots		(10)
#define	kMaxVolumes		(kMaxSlots) // 1-to-1 correspondence, must be the same.

typedef struct
{
	UInt16	   slotRefNum;	 // unique slot id
	UInt16	   slotLibRefNum;
	Boolean    hasVolume;	 // true-volume in this slot, false-no volume here
	Boolean    volumeHidden; // true-hide this volume, false-normal volume
	UInt16	   volNdx;		 // index into VolType array for volume in this slot
	ExpCardInfoType cardInfo;
} SlotType;



/***********************************************************************
 * Define at run-time the max. editable size in chars of the Volume Name
 * text field that's on the Rename & Format Forms. For normal text, we
 * picked a value of 31, which seems to cover the average case pretty well.
 * This doesn't mean that the user will always see all the text on screen.
 * For example, only 19 'W' chars fit in the on-screen text field. However,
 * you can type beyond the right end of the physical field (up to
 * kVolNameTextFieldLen chars), but the user cannot see that text. This
 * off-screen-text, which is still within the field width will be included
 * in the volume name.
 ***********************************************************************/

#define kVolNameTextFieldLen		( 31 )  // max. no. of chars in text field


/***********************************************************************
 * Make sure the internal buffer size for volume names is at least a
 * few bytes larger than the volume name text field width in chars
 * (kVolNameFudgeFactorValue). We need to do this to hide a bug in the
 * lower level routines that occurs only when the length of the volume
 * name on the card is >= the internal buffer length specified when calling
 * VFSVolumeGetLabel(). For example, if the volume name on the card is
 * 30 characters, and you ask VFSVolumeGetLabel() to return 31 bytes
 * (30 chars plus null), it returns only 29 chars plus the null (30 bytes),
 * so it's off by 1 char. We can make this bug invisible to the user by
 * making sure the buffer is always a few bytes larger than the maximum no.
 * of chars the user can enter in the on-screen text field. If and when this
 * bug gets fixed, this change to the code to accomodate the bug is harmless.
 ***********************************************************************/

#define kVolNameFudgeFactorValue	( 3 )
#define kMaxVolNameLen				( kVolNameTextFieldLen + kVolNameFudgeFactorValue )
#define kMaxVolNameSize				( 1 + kMaxVolNameLen ) // includes the null


typedef struct
{
	UInt16	   volRefNum;	// unique volume id
	char	   volName[ kMaxVolNameSize ]; // volume name
	UInt32	   volumeUsed;	// no. of bytes used by this volume
	UInt32	   volumeTotal;	// total capacity in bytes of this volume
	UInt16	   slotNdx;		// index into SlotType array of slot
							//  where this volume is located
	VolumeInfoType volInfo; // info on volume returned by VFSVolumeInfo() routine
} VolType;


static SlotType gSlotInfo[ kMaxSlots ];	 // info about every non-empty slot in palmOne Handheld
static UInt16	gActualSlotCnt = 0;

static VolType  gVolInfo[ kMaxVolumes ]; // info about every volume in palmOne Handheld
static UInt16	gActualVolCnt = 0;

// If "Please wait" dialog is displayed, display it for this minimum duration.
#define kMinIntervalWaitDialogOnScreenMsec			( 1000UL )


// Conversion macros for converting tick values to msec
#define GetTickPeriodMsec()		( 1000UL / (UInt32)SysTicksPerSecond() )
#define	ConvMsec2Ticks(msec)	( (UInt32)msec / (UInt32)GetTickPeriodMsec() )



/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

typedef struct
	{
	UInt8 replaceme;
	} StarterPreferenceType;

typedef struct
	{
	UInt8 replaceme;
	} StarterAppInfoType;

typedef StarterAppInfoType* StarterAppInfoPtr;



/***********************************************************************
 *		Global variables
 ***********************************************************************/


Boolean gCardPresent;

static	Int16	gCurSlotListNdx = 1; // The EDK card is always slot 1
static	Int16	gSlotListCnt    = 0;
static	UInt16	gSlotLibRefNum;
static	Boolean	gLedState;			// reflects on/off state of LED
static	UInt16	gTemperature;		// current temperature reading
static	Boolean	gHold;				// update of display is on hold
static	Boolean gNeedsUpdate;		// card inserted/removed event occurred, needs update
static	Char    gTbuf[ kMaxDirPathSize + kMaxDirPathSize ];  // commonly used temp buffer to decrease use of stack

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define appFileCreator			'EDK2'	// register your own at http://www.palmos.com/dev/creatorid/
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

// Define the minimum OS version we support (4.0).
#define ourMinVersion	sysMakeROMVersion(4,0,0,0,0)
#define kPalmOS10Version	sysMakeROMVersion(1,0,0,sysROMStageRelease,0)

#define	edkCommandFlashErase	0xA5	// Erase Flash memory
#define	flashMemoryImageSize	0x0001FFFF	// Flash Memory Image size (bytes)


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

Err 	RegisterForCardNotifications( Boolean registerForNotifications );
void	GetSlotCardInfo( void );


/***********************************************************************
 *
 * FUNCTION:    SdioDebugOptionsSet
 *
 * DESCRIPTION: This routine is used to set the SD/SDIO Debug Slot
 *              Driver's trace options.
 *
 * PARAMETERS:  debugOption - trace option to be set
 *
 * RETURNED:    errNone	- success
 *				expErrNotOpen	- unable to set debug options.
 *
 ***********************************************************************/
static Err SdioDebugOptionsSet( SDIODebugOptionType debugOption )
{
	Err err = errNone;
	UInt32 slotIterator;
	UInt16 slotRefNum;
	UInt16 slotLibRefNum;
	UInt32 mediaType;
	UInt16 count=0;

	// Check each slot
	slotIterator = expIteratorStart;
	while( slotIterator != expIteratorStop )
	{
		err =  ExpSlotEnumerate( &slotRefNum, &slotIterator );
		if ( err )
		{
			break;
		}
		// Find the slot driver for this slot
		err = ExpSlotLibFind(slotRefNum,  &slotLibRefNum);
		if ( !err )
		{
			err = SlotMediaType( slotLibRefNum, slotRefNum, &mediaType );
			if ( !err )
			{
				// Is this Slot Driver an SD slot driver?
				if ( mediaType == expMediaType_SecureDigital )
				{
					// Set the debug trace options
					err = SDIODebugOptions( slotLibRefNum, &debugOption );
					if ( err == errNone )
						count++;
				}
			}
		}
	}
	if ( count == 0 )
		err = expErrNotOpen;
	return( err );
}


/***********************************************************************
 *
 * FUNCTION:    CheckMyCardInfo
 *
 * DESCRIPTION: This routine is used to check if the card inserted is
 *				my card. This checks an "autorun" parameter block to see
 *				if this card is for this program.
 *
 *				This is used for the autorun event AND for detecting the
 *				initial state of an already inserted card.
 *
 * PARAMETERS:  AutoRunInfoType*	autoRunPtr
 *
 * RETURNED:    true	- success, correct card
 *				false	- not my card.
 *
 ***********************************************************************/
static Boolean CheckMyCardInfo( AutoRunInfoType* autoRunPtr )
{
	//The SDIO cards Manufacturer & ID numbers
	#define MySdioCardOemManufacturer	0x00000296L	//palmOne card
	#define MySdioCardOemID				0x00005347L	//palmOne EDK card
	#define MySdioCardOemFunctionNum 	1			//We only check function 1
	
	Boolean result = false;

	if ( autoRunPtr->media != autoRunMediaSDIO )
		goto Skip;
	
	// Check the AutoRun parameters to see if it is our card
	// First, check the manufacturer id
	if ( autoRunPtr->oemManufacturer != MySdioCardOemManufacturer )
		goto Skip;
	// Check the OEM id
	if ( autoRunPtr->oemID != MySdioCardOemID )
		goto Skip;	
	// This card is an SDIO custom device
	if ( autoRunPtr->oemFunctionStandard != autoRunFunctionStandardSDIOCustom )
		goto Skip;
	//We are only checking function 1
	if ( autoRunPtr->oemFunctionNum != MySdioCardOemFunctionNum)
		goto Skip;
	if ( autoRunPtr->sourceStruct != autoRunSourceSlotDriverType )
		goto Skip;
	
	//
	// This is the correct SDIO card
	//	
	result = true;
	
Skip:
	return( result );
}


/***********************************************************************
 *
 * FUNCTION:    CheckCardInserted
 *
 * DESCRIPTION: This routine is used to check if a card is inserted and
 *				that it is the correct card.
 *
 * PARAMETERS:  void
 *
 * RETURNED:    errNone	- success, correct card
 *				expErrCardNotPresent	- no expansion cards inserted.
 *				expErrEnumerationEmpty	- no matching card found
 *
 ***********************************************************************/
static Err CheckCardInserted( void )
{
	Err err = errNone;
	UInt32 slotIterator;
	UInt16 slotRefNum;
	UInt32 mediaType;
	UInt16 count = 0;
	SDIOAutoRunInfoType	autoRunInfo;

	// Check each slot
	slotIterator = expIteratorStart;
	while( slotIterator != expIteratorStop )
	{
		err =  ExpSlotEnumerate( &slotRefNum, &slotIterator );
		if ( err )
		{
			break;
		}
		
		// Find the slot driver for this slot
		err = ExpSlotLibFind(slotRefNum,  &gSlotLibRefNum);
		if ( !err )
		{
			err = SlotMediaType( gSlotLibRefNum, slotRefNum, &mediaType );
			if ( !err )
			{
				// Is this Slot Driver an SD slot driver?
				if ( mediaType == expMediaType_SecureDigital )
				{
					// Is the card inserted?
					err = ExpCardPresent( slotRefNum );
					if ( !err )
					{
						// Count the number of cards we have found
						count++;
						
						// Get the AutoRun Information from function 1 of
						// the card. The autorun information contains fields
						// that identify the card.
						autoRunInfo.sdioSlotNum = sdioSlotFunc1;
						err = SDIOGetAutoRun( gSlotLibRefNum, &autoRunInfo );
						if ( err == errNone )
						{
							if ( CheckMyCardInfo( &autoRunInfo.autoRun ) )
							{
								// We found it!
								err = errNone;
								goto Exit;
							}
						}
					}
				}
			}
		}
	}
	if ( count == 0 )
		err = expErrCardNotPresent;
	else
		err = expErrEnumerationEmpty;
Exit:
	return( err );
}




/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h
 *                                for format)
 *              launchFlags     - flags that indicate if the application
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
		{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			FrmAlert (RomIncompatibleAlert);
		
			// Palm OS 1.0 will continuously relaunch this app unless we switch to
			// another safe one.
			if (romVersion <= kPalmOS10Version)
				{
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
				}
			}
		
		return sysErrRomIncompatible;
		}

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void * GetObjectPtr(UInt16 objectID)
{
	FormPtr frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}


/***********************************************************************
 *
 * FUNCTION:	ResetCard
 *
 * DESCRIPTION:	Resets the SDIO card.
 *
 * PARAMETERS:		void.
 *
 * RETURN CODES:	void.
 *
 ***********************************************************************/
static void ResetCard(void)
{
	Err	err;
	SDIOPowerType power;
	UInt16	i;
	
	for( i=0; i<kNumSdioFunctions; i++ )
	{
		power.sdioSlotNum = i;
		power.powerOnCard = sdioCardPowerOff;
		err = SDIOSetPower( gSlotLibRefNum, &power );
	}
	
	for( i=0; i<kNumSdioFunctions; i++ )
	{
		power.sdioSlotNum = i;
		power.powerOnCard = sdioCardPowerOn;
		err = SDIOSetPower( gSlotLibRefNum, &power );
	}
}


/***********************************************************************
 *
 * FUNCTION:    DoCmd53Test
 *
 * DESCRIPTION: This functions tests Cmd53 reads/writes.
 *
 * PARAMETERS:  void
 *
 * RETURNED:    errNone	- success
 *
 ***********************************************************************/
static Err DoCmd53Test( void )
{
	Err err = errNone;
	SDIORWExtendedByteType	rwExtendedByteParams;
	MemHandle	bufH;
	MemPtr		bufP;
	UInt8*		cPtr;
	Int16		i;
	
	bufH = MemHandleNew(32);
	if( bufH == NULL )
		{ return(1); }
		
	bufP = MemHandleLock(bufH);
	if( bufP == NULL )
		{ return(1); }
		
	cPtr = bufP;
	
	for( i=0; i<4; i++ )
		*cPtr++ = 0x69;
		
	//SdioDebugOptionsSet( sdioDebugOptionTraceAll );
		// Do a CMD53 Extended Byte Read
		rwExtendedByteParams.requestingFunc = 1;
		rwExtendedByteParams.mode = sdioRWModeRead;
		rwExtendedByteParams.funcNum = 1;
		rwExtendedByteParams.byteAddress = 32;
		rwExtendedByteParams.bufferP = bufP;
		rwExtendedByteParams.numBytes = (UInt16)8;
		
		err = SDIORWExtendedByte( gSlotLibRefNum, &rwExtendedByteParams );
		if( err != errNone ) {
			PR("Cmd53 SDIO Err");
		}

	cPtr = bufP;
	
	for( i=0; i<4; i++ )
		*cPtr++ = 0x69;
		
		// Do a CMD53 Extended Byte Write
		rwExtendedByteParams.requestingFunc = 1;
		rwExtendedByteParams.mode = sdioRWModeWrite;
		rwExtendedByteParams.funcNum = 0;
		rwExtendedByteParams.byteAddress = 0x100;
		rwExtendedByteParams.bufferP = bufP;
		rwExtendedByteParams.numBytes = (UInt16)4;
		
		err = SDIORWExtendedByte( gSlotLibRefNum, &rwExtendedByteParams );
		if( err != errNone ) {
			PR("Cmd53 SDIO Err");
		}

		// Do a CMD53 Extended Byte Read
		rwExtendedByteParams.requestingFunc = 1;
		rwExtendedByteParams.mode = sdioRWModeRead;
		rwExtendedByteParams.funcNum = 1;
		rwExtendedByteParams.byteAddress = 32;
		rwExtendedByteParams.bufferP = bufP;
		rwExtendedByteParams.numBytes = (UInt16)8;
		
		err = SDIORWExtendedByte( gSlotLibRefNum, &rwExtendedByteParams );
		if( err != errNone ) {
			PR("Cmd53 SDIO Err");
		}

	MemHandleUnlock( bufH );
	MemHandleFree( bufH );

	//SdioDebugOptionsSet( sdioDebugOptionTraceNone );
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:    GetTemperature
 *
 * DESCRIPTION: This routine reads the temperature from the temperature
 *				sensor on the board.
 *
 * PARAMETERS:  temp - ptr to temperature variable for returned value.
 *
 * RETURNED:    void
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void GetTemperature(UInt16 *temp)
{
	Err		err = errNone;
	SDIORWDirectType		rwDirectParams;
	UInt16		wTemp;
	UInt32		dwTemp = 0;
	UInt16		nRetries = 0;

	if( !gCardPresent )
		goto Exit;

	while( nRetries++ < 2 )
	{
		// Read the temperature low-byte
		rwDirectParams.requestingFunc = 1;
		rwDirectParams.mode = sdioRWModeRead;
		rwDirectParams.funcNum = 1;
		rwDirectParams.byteAddress = 0x01;
		rwDirectParams.byteData = 0x00;
		err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );

		if( err == errNone )
		{
			// success
			wTemp = rwDirectParams.byteData;
			*temp = wTemp & (UInt16)0x00FF;

			// Read the temperature high-byte
			rwDirectParams.requestingFunc = 1;
			rwDirectParams.mode = sdioRWModeRead;
			rwDirectParams.funcNum = 1;
			rwDirectParams.byteAddress = 0x02;
			rwDirectParams.byteData = 0x00;
			err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
			if( err == errNone )
			{
				// success
				wTemp = rwDirectParams.byteData;
				wTemp = (wTemp<<8) & (UInt16)0xFF00;
				*temp |= wTemp;
				
				if( *temp >= 283 )
					dwTemp = 70 + (*temp - 283)/4;
				else
					dwTemp = 70 - (283 - *temp)/4;
				
				// We are all done
				goto Exit;
			}

		}
		
		
		// An error occured, attempt to reset the card
		ResetCard();
	}
		
Exit:
	*temp = (UInt16)dwTemp;
}


/***********************************************************************
 *
 * FUNCTION:    SetLedState
 *
 * DESCRIPTION: This routine turns the Led on/off.
 *
 * PARAMETERS:  state - on/off, turn the Led on or off.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void SetLedState(Boolean state)
{
	Err		err;
	SDIORWDirectType		rwDirectParams;
	UInt16	nRetries = 0;

	if( !gCardPresent )
		goto Exit;
		
	// Set the LED state
	while( nRetries++ < 2 )
	{
		rwDirectParams.requestingFunc = 1;
		rwDirectParams.mode = sdioRWModeWriteRead;
		rwDirectParams.funcNum = 1;
		rwDirectParams.byteAddress = 0x00;
		rwDirectParams.byteData = state ? 0x01 : 0x00;
		err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
		if( err == errNone )
			// success
			break;
		
		// Attempt to reset the card
		ResetCard();
	}
	
	// Read back the LED state
	gLedState = (rwDirectParams.byteData == 0x01) ? true : false;
	
Exit:
	CtlSetValue(GetObjectPtr(MainLedControlCheckbox), gLedState);
	return;
}


/***********************************************************************
 *
 * FUNCTION:	EraseFlash
 *
 * DESCRIPTION:	Performs a bulk erase of the flash memory on the EDK card
 *				by sending an "EraseFlash" command to the card.
 *
 * PARAMETERS:		none.
 *
 * RETURN CODES:	void.
 *
 ***********************************************************************/

Err EraseFlash(void);
static Err EraseFlash(void)
{
	Err		err;
	SDIORWDirectType		rwDirectParams;
	UInt8	cmdBuf	= edkCommandFlashErase;
	long	time;
	
	// Check if the card is inserted
	err = CheckCardInserted();
	switch( err )
	{
		case	errNone:
			// Erase the Flash memory by writing an erase command
			// to address 0x03
			rwDirectParams.requestingFunc = 1;
			rwDirectParams.mode = sdioRWModeWrite;
			rwDirectParams.funcNum = 1;
			rwDirectParams.byteAddress = 3;
			rwDirectParams.byteData = edkCommandFlashErase;

			err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
			if( err != errNone )
			{
				// Pop-up an error dialog
				FrmAlert(ErrorErasingFlashAlert);
			}
			
			// Wait 10 seconds for Flash erase
			time = TimGetSeconds();
			time += 10;
			while( TimGetSeconds() <= time )
				;

			break;
				
		case	expErrCardNotPresent:
			FrmAlert(NoCardInsertedAlert);
			break;
				
		case	expErrEnumerationEmpty:
			FrmAlert(UnrecognizedCardAlert);
			break;
			
		default:
			break;
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:	ProgramFlash
 *
 * DESCRIPTION:	Programs the flash memory on the EDK card. Assumes that the
 *				Flash memory has been erased before calling this function.
 *				Reads a Flash memory image from a File Stream in the
 *				Storage Heap and programs it into the CSA memory
 *				of the EDK. The Flash memory image should contain a
 *				valid FAT file system image.
 *
 * PARAMETERS:		none.
 *
 * RETURN CODES:	void.
 *
 ***********************************************************************/
Err ProgramFlash(void);
static Err ProgramFlash(void)
{
	Err	err;
    FileHand	hFsFile;
	Int32	dwBytesToCopy=flashMemoryImageSize;
	UInt32	dwBytesRead=0;
	SDIORWDirectType		rwDirectParams;
	UInt32	i;
	unsigned char	lpBuf[512];
	UInt32		nFlashAddress;
	Boolean		bValidData;
	
	// Open the File Stream (Flash memory image) file
	hFsFile = FileOpen(0, "EDKFlashImage-EDK2", 'STRM', 'EDK2', fileModeReadOnly, &err);
	if( err != errNone )
	{
		// Try to open the old File Stream (Flash memory image) file
		hFsFile = FileOpen(0, "EDKFileStream-EDK1", 'STRM', 'EDK1', fileModeReadOnly, &err);
		if( err != errNone )
		{
			// Alert the user that a valid Flash image was not found
			FrmAlert(NoFlashImageAlert);
		}
	}
	
	// Check if the card is inserted
	err = CheckCardInserted();
	switch( err )
	{
		case	errNone:
			// Check if the CSA is present
			rwDirectParams.requestingFunc = 1;
			rwDirectParams.mode = sdioRWModeRead;
			rwDirectParams.funcNum = 0;
			rwDirectParams.byteAddress = 0x100;
			rwDirectParams.byteData = 0x00;
			err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
			if( err != errNone )
				FrmAlert(SDIOErrorAlert);
			
			// Only format the card if CSA supported
			if( rwDirectParams.byteData & 0x40 )
			{							
			
		    	// Copy the data from the File Stream in storage heap to
		    	// the CSA on the card
		    	nFlashAddress = 0x00000000L;

				while( (dwBytesToCopy > 0) && (err == errNone) )
				{
					dwBytesRead = FileRead( hFsFile, lpBuf, 1, 512, &err);
					if( err != errNone )
					{
						err = errNone;
						break;
					}
					
					dwBytesToCopy -= dwBytesRead;
					
					// Check if the bytes are all unprogrammed (0xFF)
					bValidData = false;
					for( i=0; i<dwBytesRead; i++ )
					{
						if( lpBuf[i] != 0xFF )
						{
							bValidData = true;
							break;
						}
					}

					// Is there data to program that is not 0xFF?
					if( bValidData )
					{
						// Initialize the CSA pointer to start at last location
						rwDirectParams.requestingFunc = 1;
						rwDirectParams.mode = sdioRWModeWrite;
						rwDirectParams.funcNum = 0;
						rwDirectParams.byteAddress = 0x10c;
						rwDirectParams.byteData = (UInt8)((nFlashAddress) & 0x000000FFL);
						err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
							if( err != errNone ) { FrmAlert(SDIOErrorAlert); break; }
						rwDirectParams.requestingFunc = 1;
						rwDirectParams.mode = sdioRWModeWrite;
						rwDirectParams.funcNum = 0;
						rwDirectParams.byteAddress = 0x10d;
						rwDirectParams.byteData = (UInt8)((nFlashAddress >> 8) & 0x000000FFL);
						err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
							if( err != errNone ) { FrmAlert(SDIOErrorAlert); break; }
						rwDirectParams.requestingFunc = 1;
						rwDirectParams.mode = sdioRWModeWrite;
						rwDirectParams.funcNum = 0;
						rwDirectParams.byteAddress = 0x10e;
						rwDirectParams.byteData = (UInt8)((nFlashAddress >> 16) & 0x000000FFL);
						err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
							if( err != errNone ) { FrmAlert(SDIOErrorAlert); break; }
						
						rwDirectParams.requestingFunc = 1;
						rwDirectParams.mode = sdioRWModeWrite;
						rwDirectParams.funcNum = 0;
						rwDirectParams.byteAddress = 0x10f;
						
						// Actually write the bytes to Flash
						for( i=0; i<dwBytesRead; i++ )
						{
							rwDirectParams.byteData = (UInt8)lpBuf[i];
							err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );
							if( err != errNone ) { FrmAlert(SDIOErrorAlert); break; }
						}
					}
					
					nFlashAddress += dwBytesRead;					
				}

			}
			else
			{
				FrmAlert(FuncNotAvailableAlert);
			}


			break;
				
		case	expErrCardNotPresent:
			FrmAlert(NoCardInsertedAlert);
			break;
				
		case	expErrEnumerationEmpty:
			FrmAlert(UnrecognizedCardAlert);
			break;
			
		default:
			break;
	}
	
	// Close the File Stream file
	FileClose(hFsFile);
	
	return err;
}


/***********************************************************************
 *
 * FUNCTION:	ProgramFlashImage
 *
 * DESCRIPTION:	Erases and re-programs the flash memory on the EDK
 *				card.
 *
 * PARAMETERS:		none.
 *
 * RETURN CODES:	void.
 *
 ***********************************************************************/

void ProgramFlashImage(void);
static void ProgramFlashImage(void)
{
	Err			err;
	Boolean		pleaseWaitOnScreen = false;
	FormPtr		prevFrm = NULL;
	FormPtr		waitFrm = NULL;
	UInt32		minDisplayIntervalTicks;
	UInt32		prevTicks;
	UInt32		curTicks;
	UInt32		actualTicks;
	UInt32		ticksLeft;
    FileHand	hFsFile;
	
	// Check for presence of the File Stream (Flash memory image) file
	hFsFile = FileOpen(0, "EDKFlashImage-EDK2", 'STRM', 'EDK2', fileModeReadOnly, &err);
	if( err != errNone )
	{
		// Try to open the old File Stream (Flash memory image) file
		hFsFile = FileOpen(0, "EDKFileStream-EDK1", 'STRM', 'EDK1', fileModeReadOnly, &err);
		if( err != errNone )
		{
			// Alert the user that a valid Flash image was not found
			FrmAlert(NoFlashImageAlert);
		}
	}
	
	if( err != errNone )
		goto Exit;
	
	// Close the file we just opened
	FileClose( hFsFile );
	
	prevFrm = FrmGetActiveForm();
	waitFrm = FrmInitForm( WaitForm );
	FrmSetActiveForm( waitFrm );
	FrmDrawForm( waitFrm );

	pleaseWaitOnScreen = true;

	// Get time stamp right NOW since we just displayed "Please wait".
	prevTicks = TimGetTicks();
	
	// First, do a bulk erase of Flash memory
	err = EraseFlash();
	if( err == errNone )
	{
		// Then, program the flash image into Flash
		err = ProgramFlash();
		if( err != errNone )
		{
			FrmAlert(ErrorProgrammingFlashAlert);
		}
	}
	
	if( pleaseWaitOnScreen )
	{
		// We're done flashing the memory. Tear down the
		// "Please wait" dialog. However, keep the wait dialog on
		// screen just long enough to allow user to read it.

		// Determine time remaining in the minimum on-screen time interval.
		curTicks = TimGetTicks();
		if( curTicks > prevTicks )
		{
			// prevTicks < curTicks: tick count has NOT wrapped around 0.
			actualTicks = curTicks - prevTicks;
		}
		else
		{
			// prevTicks >= curTicks: tick count HAS wrapped around 0.
			actualTicks = ( (UInt32)0xFFFFFFFF - prevTicks + 1UL )
							+ curTicks;
		}

		// Convert the minimum wait dialog interval from msec to ticks
		minDisplayIntervalTicks =
			ConvMsec2Ticks( kMinIntervalWaitDialogOnScreenMsec );

		if( actualTicks < minDisplayIntervalTicks )
		{
			// The "Please wait" dialog wasn't on screen long enough
			// to keep it from flickering. Keep it on screen just a
			// little bit longer. Subtract elapsed time so far.
			ticksLeft = minDisplayIntervalTicks - actualTicks;

			// WARNING: Future modifications !
			//   actualTicks MUST BE LESS THAN minDisplayIntervalTicks
			//   If this isn't true, ticksLeft becomes a HUGE number,
			//   and EDKDemo will go to sleep forever right here !
//					ErrNonFatalDisplayIf( actualTicks >= minDisplayIntervalTicks,
//								"Err computing ticks" );

			// Sleep long enough to keep dialog on screen for minimum time.
			SysTaskDelay( (Int32)ticksLeft );
		}

		FrmEraseForm( waitFrm );
		FrmDeleteForm( waitFrm );
		if( prevFrm )
			FrmSetActiveForm( prevFrm );
	}
	
	// Notify the user that Flash memory successfully programmed.
	if( err == errNone )
		FrmAlert( SuccessProgrammingFlashAlert );
Exit:
	;
}


/***********************************************************************
 *
 * FUNCTION:	SetTextField
 *
 * DESCRIPTION:	Sets the contents of a text field in the current form.
 *
 * PARAMETERS:		ObjectID	- object ID of text field to set.
 *
 * RETURN CODES:	void.
 *
 ***********************************************************************/
static void SetTextField(Int16 ObjectID, Char *lpszText)
{
	MemHandle	hText;
	MemHandle	hOldText;
	MemPtr		pText;


	// Allocate memory for time remaining field text
	hText = MemHandleNew(8);
	if( hText )
	{
		// Lock down the handle and get pointer to the memory chunk.
		pText = MemHandleLock(hText);
		if( !pText )
		{
			// Internal error
			FrmAlert( InternalErrorAlert );
		}
		else
		{	
			// Set the text
			DmSet(pText, 0, 8, 0);
			DmWrite(pText, 0, lpszText, StrLen(lpszText));
			
			// Unlock the new memory chunk.
			MemHandleUnlock(hText);

			// Save the field's text handle for freeing later.
			hOldText = FldGetTextHandle(GetObjectPtr(ObjectID));

			// Set the field's text handle.
			FldSetTextHandle(GetObjectPtr(ObjectID), hText);
		
			// Free the old text memory
			if( hOldText )
				MemHandleFree( hOldText );
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:	DispCardInfo
 *
 * DESCRIPTION:	Display info about current card in the scrollable text
 *				area on screen including:
 *				- if card is read-only
 *				- display the directory summary information, unless
 *				  the volume has the hidden attribute set
 *				- number of bytes used, free and total capacity of volume
 *				- product and manufacturer string, device class & device id
 *				- display updated scrollbar
 *				
 * INPUT:		Globals:
 *				- gCurSlotListNdx - index of currently selected Slot List entry
 *				  					slot to display card info about
 *				- gActualSlotCnt  - number of cards present in device
 *				- gSlotInfo[]     - used to determine if slot has a volume
 *				- strings[]       - array of string resources for displaying
 *								    text contants on screen
 * OUTPUT:		none
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			------	--------	----------------
 *			BBarry	5/18/00		Initial revision
 *
 ***********************************************************************/

void DispCardInfo( void );
void DispCardInfo( void )
{
	Err		err = errNone;
	SDIORWDirectType		rwDirectParams;
	Int16	wMajor;
	Int16	wMinor;

//	For example:
//	PR(  "MultiMedia Pro Card" );
	PR(  gSlotInfo[ gCurSlotListNdx ].cardInfo.productStr );

//	PR(  "Palm, Inc." );
	StrPrintF( gTbuf, "%s", gSlotInfo[ gCurSlotListNdx ].cardInfo.manufacturerStr );
	PR( gTbuf );

//	PR(  "I/O Card" );
	StrPrintF( gTbuf, "%s", gSlotInfo[ gCurSlotListNdx ].cardInfo.deviceClassStr );
	PR( gTbuf );

//	PR(  "Device Id: 87654321" );
	StrPrintF( gTbuf, "%s: %s",
					"Device Id ",
					gSlotInfo[ gCurSlotListNdx ].cardInfo.deviceUniqueIDStr );
	PR( gTbuf );
	
	// Read the PIC version number
	rwDirectParams.requestingFunc = 1;
	rwDirectParams.mode = sdioRWModeRead;
	rwDirectParams.funcNum = 1;
	rwDirectParams.byteAddress = 0x04;
	rwDirectParams.byteData = 0x00;
	err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );

	wMajor = ((rwDirectParams.byteData >> 4) & 0x0F);
	wMinor = (rwDirectParams.byteData & 0x0F);

	StrPrintF( gTbuf, "%s: %d.%d",
					"FW version \(PIC\) ",
					wMajor, wMinor );
	PR( gTbuf );

	// Read the CPLD version number
	rwDirectParams.requestingFunc = 1;
	rwDirectParams.mode = sdioRWModeRead;
	rwDirectParams.funcNum = 1;
	rwDirectParams.byteAddress = 0x05;
	rwDirectParams.byteData = 0x00;
	err = SDIORWDirect( gSlotLibRefNum, &rwDirectParams );

	wMajor = ((rwDirectParams.byteData >> 4) & 0x0F);
	wMinor = (rwDirectParams.byteData & 0x0F);
	
	StrPrintF( gTbuf, "%s: %d.%d",
					"FW version \(CPLD\) ",
					wMajor, wMinor );
	PR( gTbuf );

	// Load the Palm OS "Card" Table with indexes into
	//  the global "cardTable" array, starting at the top
	CTableLoad( 0 /*topNdx*/ );
	CTableRedraw();
}


/***********************************************************************
 *
 * FUNCTION:	GetSlotCardInfo
 *
 * DESCRIPTION:	Get info on all slots and cards that are currently known
 *				to Palm OS. Procedure for getting slot and card information:
 *				1. Are the VFS routines resident ?
 *				   Display error message and exit if missing.
 *				2. Determine what slots currently exist
 *				   Get information about the card in each slot
 *				   Skip slots where no card is present
 *				3. Determine what volumes exist
 *				   Get the volume name, no. of bytes used and capacity for each
 *				   volume. Determine the corresponding slot for each volume.
 *				
 * NOTE:		When information is gathered from the card, the volume name,
 *				which could be a very long name, is truncated to the length
 *				of the on-screen volume name text field width, which is
 *				defined by the constant: kVolNameTextFieldLen. CardInfo
 *				doesn't support preserving a volume name on the card that's
 *				larger than CardInfo's on-screen volume name text field width.
 *
 * INPUT:		none
 *
 * OUTPUT:		globals:
 *				- gCardsPresent   - true-we have at least one non-empty card slot
 *				- gCurSlotListNdx - validated to ensure it's within range of the
 *									number of entries in the Slot list popup.
 *									It's possible a card was removed.
 *				- gActualSlotCnt  - no. of slots found with a card in it.
 *				- gSlotInfo[]     - updated with info on each slot with a card.
 *							        Empty slots are skipped.
 *				- gActualVolCnt   - no. of volumes found.
 *				- gVolInfo[]      - updated with info on each volume found.
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void GetSlotCardInfo( void )
{
	Err		   err;
	FormPtr    frmP;
	UInt32	   slotIterator;
	UInt32	   volIterator;
	UInt16	   slotRefNum;
	UInt16	   volRefNum;
	UInt16	   rowNdx;
	UInt16	   slotNdx;
	UInt16	   volNdx;
	Boolean    foundSlot;
	UInt32	   VFSMgrVersion;
	UInt32     mediaType;
	SDIOAutoRunInfoType	autoRunInfo;


	gActualSlotCnt = 0;
	for( rowNdx=0  ;  rowNdx < kMaxSlots  ;  ++rowNdx )
		MemSet( &gSlotInfo[ rowNdx ], sizeof( gSlotInfo[ rowNdx ] ), 0 );

	frmP = FrmGetActiveForm();


	// Procedure for getting slot and volume information:
	// (1) Are the VFS routines in memory ?
	// (2) Determine what slots exist
	//     Get Card Info on each slot
	//     Skip slots that where no card is present
	// (3) Determine what volumes are installed
	//     Get Volume Name, bytes used and capacity
	//     Associate each volume with corresponding slot


	//_____(1) Are the VFS routines in memory ?

	err = FtrGet( sysFileCVFSMgr, vfsFtrIDVersion, &VFSMgrVersion );
	if( err )
	{
		// We're using the VFS routines, so we can't run without them.
		PR( "No VFS Library Present." );

		gCardPresent = false;

	    // Load the Palm OS "Card" Table with indexes into
		//  the global "cardTable" array, starting at the top
	    CTableLoad( 0 /*topNdx*/ );
    	CTableRedraw();
		return;
	}


	//_____(2) Look for our card (EDK card)
	//         Get Card Info on each slot
	//         Skip slots where no card is present

	// Check if the card is inserted
	gCardPresent = false;
	err = CheckCardInserted();
	switch( err )
	{
		case	errNone:
			gCardPresent = true;  // we have found our card
			break;
						
		case	expErrCardNotPresent:
			PR("No card inserted");
			break;
						
		case	expErrEnumerationEmpty:
			PR("EDK card not present");
			break;
					
		default:
			break;
	}
	
	if( gCardPresent)
	{
		slotNdx = 0;
		slotRefNum = 0;
		slotIterator = expIteratorStart;
		while( slotIterator != expIteratorStop )
		{
			// Get the slotRefNum for the next slot
			err = ExpSlotEnumerate( &slotRefNum, &slotIterator );
			if( err != errNone )
			{
				// error occurred
				break;
			}
	
			if( errNone != ExpCardPresent( slotRefNum ) )
			{
				// There's no card present, skip this slot.
				continue;
			}
			
			// Find the slot driver for this slot
			err = ExpSlotLibFind(slotRefNum,  &gSlotLibRefNum);
			if ( !err )
			{
				err = SlotMediaType( gSlotLibRefNum, slotRefNum, &mediaType );
				if ( !err )
				{
					// Is this Slot Driver an SD slot driver?
					if ( mediaType == expMediaType_SecureDigital )
					{
						// Is the card inserted?
						err = ExpCardPresent( slotRefNum );
						if ( !err )
						{
							// Get the AutoRun Information from function 1 of
							// the card. The autorun information contains fields
							// that identify the card.
							autoRunInfo.sdioSlotNum = sdioSlotFunc1;
							err = SDIOGetAutoRun( gSlotLibRefNum, &autoRunInfo );
							if ( err == errNone )
							{
								if ( CheckMyCardInfo( &autoRunInfo.autoRun ) )
								{
									// We found it!
									err = errNone;
									gCardPresent = true;  // we have at least one card inserted
									
									gSlotInfo[ slotNdx ].slotRefNum = slotRefNum;
									
									// Get the following information about this slot:
									// manufacturer, product, device class, and unique ID strings
									ExpCardInfo(  gSlotInfo[ slotNdx ].slotRefNum,
												 &gSlotInfo[ slotNdx ].cardInfo );
									
									// Get the slotLibRefNum for this slot
									ExpSlotLibFind(  gSlotInfo[ slotNdx ].slotRefNum,
												    &gSlotInfo[ slotNdx ].slotLibRefNum );
									
									++slotNdx;
									
									// Count the number of cards we have found						
									++gActualSlotCnt;
								}
							}
						}
					}
				}
			}
	
		}
	}

    if( gCurSlotListNdx > gActualSlotCnt-1 )
    {
        // The index value is now out of range. Maybe a card was
        // removed since the last time this routine was called.
        // Start the index back at the first entry.
        gCurSlotListNdx = 0;
    }


	//_____(3) Determine what volumes are installed
	//         Get Volume Name, bytes used and capacity
	//         Associate each volume with corresponding slot

	volNdx = 0;
	volRefNum = 0;
	volIterator = expIteratorStart;
	while( volIterator != expIteratorStop )
	{
		// Get the volRefNum for the next volume
		err = VFSVolumeEnumerate( &volRefNum, &volIterator );
		if( err != errNone )
		{
			// error occurred
			break;
		}

		gVolInfo[ volNdx ].volRefNum = volRefNum;

		// Get the Volume Name for this volume
		err = VFSVolumeGetLabel( gVolInfo[ volNdx ].volRefNum,
							 	 gVolInfo[ volNdx ].volName, kMaxVolNameSize );
		if(  ( ( err != errNone ) &&
			   ( err != vfsErrNameShortened ) &&
			   ( err != vfsErrBufferOverflow )  )
			 || ( ! gVolInfo[ volNdx ].volName )
			 || ( ! gVolInfo[ volNdx ].volName[0] )  )
		{
			// This volume has no name, give it the default name: Card
			StrCopy( gVolInfo[ volNdx ].volName,
						  "Card" );
		}

		// Truncate volume name to the size of the on-screen text field.
		// CardInfo doesn't support preserving a volume name on the card
		// that's larger than CardInfo's on-screen text field width.
		// - Decrease kVolNameTextFieldLen (possibly) to a proper multi-byte
		//   char boundary so we truncate correctly on a char boundary.

		// StrCopy( gTbuf, gVolInfo[ volNdx ].volName );  // make a copy
		// Truncate volume name to the on-screen field length, if needed.
		//  (StrCopyTrunc() supports multibyte char strings.)
		// StrCopyTrunc( gVolInfo[ volNdx ].volName, gTbuf, kVolNameTextFieldLen );


		// Get the size in bytes of this volume
		VFSVolumeSize(  gVolInfo[ volNdx ].volRefNum,
					   &gVolInfo[ volNdx ].volumeUsed,
					   &gVolInfo[ volNdx ].volumeTotal );

		// Get the volume info known by VFSMgr routines
		VFSVolumeInfo(  gVolInfo[ volNdx ].volRefNum,
					   &gVolInfo[ volNdx ].volInfo );

		// Associate this volume with an existing slot
		foundSlot = false;
		for( slotNdx=0  ;  slotNdx < gActualSlotCnt  ;  ++slotNdx )
		{
			if( gSlotInfo[ slotNdx ].slotRefNum
				== gVolInfo[ volNdx ].volInfo.slotRefNum )
			{
				// We found the slot for this volume,
				//  save the slotNdx in the current volume structure
				gVolInfo[ volNdx ].slotNdx = slotNdx;

				// Save the slotLibRefNum for this volume
				gVolInfo[ volNdx ].volInfo.slotLibRefNum =
										gSlotInfo[ slotNdx ].slotLibRefNum;

				// Save the volNdx in the associated slot structure
				gSlotInfo[ slotNdx ].volNdx    = volNdx;
				gSlotInfo[ slotNdx ].hasVolume = true;

				foundSlot = true;
				if( vfsVolumeAttrHidden & gVolInfo[ volNdx ].volInfo.attributes )
				{
					// Treat this card just like an I/O card.
					// Don't display any file system.
					gSlotInfo[ slotNdx ].volumeHidden = true;
				}
				break;
			}
		}

		if( !foundSlot )
		{
			//StrPrintF( gTbuf, "No slot for this volume, slotRefNum: %d",
			//			gVolInfo[ volNdx ].volInfo.slotRefNum );
			//ErrNonFatalDisplayIf( !foundSlot, gTbuf );
		}

		++volNdx;

		++gActualVolCnt;
	}

	// Load the Palm OS "Card" Table with indexes into
	//  the global "gCardTable" array, starting at the top
	// Need these 2 calls to CTableLoad() and CTableRedraw() here in case you
	//  added some PR() calls above and you want the text displayed on screen.
	CTableLoad( 0 /*topNdx*/ );
	CTableRedraw();
}


/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:  frm - pointer to the MainForm form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void MainFormInit(FormPtr /*frmP*/)
{
	Err err = errNone;
	
	CTableInit();		// Initialize the Palm OS on-screen table

	FrmDrawForm ( FrmGetActiveForm() );

	GetSlotCardInfo();		// Get info on card slots & cards
	
	if(gCardPresent)
		DispCardInfo();
	
	// Turn the LED off
	SetLedState( false );

Exit:
	return;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormDoCommand(UInt16 command)
{
	Boolean handled = false;
	FormPtr frmP;
	Err 	err = errNone;

	switch (command)
	{
		case HelpAboutEDKDemo:
			MenuEraseStatus(0);					// Clear the menu status from the display.
			frmP = FrmInitForm (AboutForm);
			FrmDoDialog (frmP);					// Display the About Box.
			FrmDeleteForm (frmP);
			if( gNeedsUpdate )
				FrmGotoForm(MainForm);
			gHold = false;						// enable temperature update again
			handled = true;
			break;
			
		case OptionsProgramFlashImage:
			MenuEraseStatus(0);
			
			// Program the flash image
			ProgramFlashImage();
			
			if( gNeedsUpdate )
				FrmGotoForm(MainForm);
			gHold = false;						// enable temperature update again
			handled = true;
			break;

		case OptionsSDIOTraceAll:
			MenuEraseStatus(0);
			
			SdioDebugOptionsSet(sdioDebugOptionTraceAll);
			
			gHold = false;						// enable temperature update again
			handled = true;
			break;

		case OptionsSDIOTraceMost:
			MenuEraseStatus(0);
			
			SdioDebugOptionsSet(sdioDebugOptionTraceMost);
			
			gHold = false;						// enable temperature update again
			handled = true;
			break;

		case OptionsSDIOTraceNone:
			MenuEraseStatus(0);
			
			SdioDebugOptionsSet(sdioDebugOptionTraceNone);
			
			gHold = false;						// enable temperature update again
			handled = true;
			break;

		case HelpSpecial:
			MenuEraseStatus(0);					// Clear the menu status from the display.
			
			// Check CMD53 Write
			err=DoCmd53Test();
			
			if( gNeedsUpdate )
				FrmGotoForm(MainForm);
			gHold = false;						// enable temperature update again
			handled = true;
			break;
			

	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr eventP)
{
	Boolean handled = false;
	FormPtr frmP;
	Err		err = errNone;
	Char	lpszText[16];
	Boolean	bLedState;
	
	switch (eventP->eType)
		{
		case	ctlSelectEvent:
			// LED Checkbox is tapped.
			if (eventP->data.ctlSelect.controlID == MainLedControlCheckbox)
			{
				// The LED checkbox was pressed so retrieve the value.
				// Set the LED accordingly.
				bLedState = CtlGetValue(GetObjectPtr(MainLedControlCheckbox)) ? true : false;
				SetLedState(bLedState);
			}
			handled = true;
			break;
		case menuEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case frmOpenEvent:
			frmP = FrmGetActiveForm();
			MainFormInit( frmP);
			FrmDrawForm ( frmP);
			handled = true;
			break;
			
		case frmUpdateEvent:
			// To do any custom drawing here, first call FrmDrawForm(), then do your
			// drawing, and then set handled to true.
			CtlSetValue(GetObjectPtr(MainLedControlCheckbox), false);
			break;

		case	nilEvent:
			if( !gHold )
			{
				frmP = FrmGetActiveForm();
				// Get the temperature and display it
				GetTemperature(&gTemperature);
				
				// Set the text field to the current temperature
				sprintf( lpszText, "%d", gTemperature );
				SetTextField(MainTemperatureField, lpszText);
				
		        // Draw the form
		        FrmDrawForm(frmP);
	        }

			break;
			
		default:
			break;
		
		}
	
	return handled;
}




/***********************************************************************
 *
 * FUNCTION:     RegisterForCardNotifications
 *
 * DESCRIPTION:  Register to receive card inserted/removed notifications
 *				 from the Notification Manager. We also register for
 *				 volume mounted/unmounted notifications so we don't
 *				 switch to the Launcher when a card is inserted with
 *				 a filesystem on it.
 *
 * INPUT:		 registerForNotifications
 *						- true  - means register for notifications
 *						  false - means unregister for those same events
 *
 * OUTPUT:		 none
 *
 * FN RETURNS:	 none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

Err RegisterForCardNotifications( Boolean registerForNotifications )
{
	UInt16  cardNo;
	LocalID dbID;
	Err		err, err2;

	err2 = errNone;
	err = SysCurAppDatabase( &cardNo, &dbID );
	ErrNonFatalDisplayIf( err != errNone, "Cannot get app DB info" );
	if( err != errNone )
		return err;
	
	if( registerForNotifications )
	{
		// Register for card inserted/removed events:
		err = SysNotifyRegister( cardNo, dbID, sysNotifyCardInsertedEvent,
						NULL, sysNotifyNormalPriority, NULL );
		ErrNonFatalDisplayIf( err != errNone, "Cannot register for card inserted event" );
		if(err != errNone) err2 = dmErrMemError;

		err = SysNotifyRegister( cardNo, dbID, sysNotifyCardRemovedEvent,
						NULL, sysNotifyNormalPriority, NULL);
		ErrNonFatalDisplayIf( err != errNone, "Cannot register for card removed event" );
		if(err != errNone) err2 = dmErrMemError;

		// Register for volume mounted/unmounted events:
		err = SysNotifyRegister( cardNo, dbID, sysNotifyVolumeMountedEvent,
						NULL, sysNotifyNormalPriority, NULL );
		ErrNonFatalDisplayIf( err != errNone, "Cannot register for volume mounted event" );
		if(err != errNone) err2 = dmErrMemError;

		err = SysNotifyRegister( cardNo, dbID, sysNotifyVolumeUnmountedEvent,
						NULL, sysNotifyNormalPriority, NULL);
		ErrNonFatalDisplayIf( err != errNone, "Cannot register for volume unmounted event" );
		if(err != errNone) err2 = dmErrMemError;

		// Register for wake-up events:
		err = SysNotifyRegister( cardNo, dbID, sysNotifyLateWakeupEvent,
						NULL, sysNotifyNormalPriority, NULL );
		ErrNonFatalDisplayIf( err != errNone, "Cannot register for wake-up event" );
		if(err != errNone) err2 = dmErrMemError;
	}
	else
	{
		// Unregister for card inserted/removed events:
		err = SysNotifyUnregister( cardNo, dbID, sysNotifyCardInsertedEvent,
						sysNotifyNormalPriority );
		ErrNonFatalDisplayIf( err != errNone, "Cannot unregister for card inserted event" );

		err = SysNotifyUnregister( cardNo, dbID, sysNotifyCardRemovedEvent,
						sysNotifyNormalPriority );
		ErrNonFatalDisplayIf( err != errNone, "Cannot unregister for card removed event" );
		
		// Unregister for volume mounted/unmounted events:
		err = SysNotifyUnregister( cardNo, dbID, sysNotifyVolumeMountedEvent,
						sysNotifyNormalPriority );
		ErrNonFatalDisplayIf( err != errNone, "Cannot unregister for volume mounted event" );

		err = SysNotifyUnregister( cardNo, dbID, sysNotifyVolumeUnmountedEvent,
						sysNotifyNormalPriority );
		ErrNonFatalDisplayIf( err != errNone, "Cannot unregister for volume unmounted event" );
		
		// Unregister for wake-up events:
		err = SysNotifyUnregister( cardNo, dbID, sysNotifyLateWakeupEvent,
						sysNotifyNormalPriority );
		ErrNonFatalDisplayIf( err != errNone, "Cannot unregister for wake-up event" );
		
	}
	
	if(err2)
		ErrAlert(err2);	
	return err2;
}


/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			default:
//				ErrFatalDisplay("Invalid Form Load Event");
				break;

			}
		return true;
		}
	
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	UInt16		error;
	EventType	event;
	UInt16		tick;

	tick = SysTicksPerSecond();

	// tick = 0;			// full speed
	
	do {
		EvtGetEvent(&event, tick);

		if (! SysHandleEvent(&event))
			if( event.eType == menuOpenEvent )
				gHold = true;
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStart(void)
{
	StarterPreferenceType prefs;
	UInt16 prefsSize;
   	Boolean	registerForNotifications = true;


	// Read the saved preferences / saved-state information.
	prefsSize = sizeof(StarterPreferenceType);
	if (PrefGetAppPreferences(appFileCreator, appPrefID, &prefs, &prefsSize, true) !=
		noPreferenceFound)
		{
		}


	// Register for volume mount/unmount notifications.
	RegisterForCardNotifications( registerForNotifications );

	// Set state of any global variables
	gHold = false;
	gNeedsUpdate = false;

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppStop(void)
{
	StarterPreferenceType prefs;
	Boolean registerForNotifications = false;

	// Write the saved preferences / saved-state information.  This data
	// will saved during a HotSync backup.
	PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersionNum,
		&prefs, sizeof (prefs), true);
		
	// Unregister for volume mount/unmount notifications.
	RegisterForCardNotifications( registerForNotifications );
	
	// Ensure we turn off any tracing
	SdioDebugOptionsSet(sdioDebugOptionTraceNone);

	// Close all the open forms.
	FrmCloseAllForms ();
}


/***********************************************************************
 *
 * FUNCTION:    StarterPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static UInt32 StarterPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;
	FormPtr frmP;
	SysNotifyParamType *notifyP;

	error = RomVersionCompatible (ourMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error)
				return error;
				
			FrmGotoForm(MainForm);
			AppEventLoop();
			AppStop();
			break;

		case sysAppLaunchCmdNotify:
			notifyP = (SysNotifyParamType*) cmdPBP;

			if( ( notifyP->notifyType == sysNotifyCardInsertedEvent ) ||
					( notifyP->notifyType == sysNotifyCardRemovedEvent ) ||
					( notifyP->notifyType == sysNotifyLateWakeupEvent ) )
			{
				if( !gHold )
				{
					frmP = FrmGetActiveForm();
					MainFormInit( frmP);
					FrmDrawForm ( frmP);
				}
				else
				{
					gNeedsUpdate = true;	// A card removed/inserted event happened while About dialog was up.
				}
				break;
			}
			else if( notifyP->notifyType == sysNotifyVolumeMountedEvent )
			{
				// A card with filesystem has been inserted, mark event as handled so that
				// Launcher switch does not occur (otherwise we won't
				// come back, and Launcher will display contents of new card).

				notifyP->handled |= vfsHandledUIAppSwitch;
			}
			
			break;

		default:
			break;

		}
		
	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return StarterPalmMain(cmd, cmdPBP, launchFlags);
}
