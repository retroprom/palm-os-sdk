/***************************************************************
 *
 * Project:
 *    Card Updater Core application
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    CardUpdaterCoreApp.c
 *
 * Description:
 *    This file contains application source code for the Card
 *    Updater Core Application.
 *
 * ToDo:
 *
 * History:
 *    14-May-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *    21-Feb-2001	JMP		Modified to support the new fix version schema
 *
 ****************************************************************/

#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>

#include <KeyMgr.h>				// for KeyCurrentState

#include <SysEvtMgr.h>			// for EvtResetAutoOffTimer


// Include Handspring Extensions definitions
#include <HSExt.h>


// Include Flash Manager definitions
#include <FlashMgr.h>


// Include our own resource definitions
#include "CardUpdaterCoreRsc.h"	  // main resource constant definitions
#include "CardUpdaterOvrRsc.h"	  // overridable resource constant definitions

#define OVERLAYS
#ifdef OVERLAYS
#include "OverlayUtils.h"
// Allow VC++ to compile...
#if defined(_MSC_VER)
  #define OVERLAY_BASE ""
#endif
#endif // Overlays


// -------------------------------------------------------------
// Internal constants
// -------------------------------------------------------------

// leave Springboard chip selects at their default value (see
//  resOvrStrIDChipSelectAutoConfigScheme)
#define prvModuleConfigSchemeDefault	0

// instruct the Flash Manager to use it's auto-scan technique
// to detect the configuration of the flash module and configure
// its chip-selects. (see resOvrStrIDChipSelectAutoConfigScheme)
#define prvModuleConfigSchemeAutoScan	1

//
#define prvNewVersionFormatFlag	  0x8000

// Type and starting id number of ROM image segments
// The ROM image is split by the SDK build process into multiple
// segments; each segment is imported into the updater application
// as a 'RomI' type resource; the id's are numbered in the order
// that these segments should appear in the ROM image.
#define prvFlashImageRscType	'RomI'
#define prvFlashImageRscIdFirst	1


// Type and id number of ROM image CRC-16 checksum value
// The PalmOS-compatible CRC-16 checksum is computed by the SDK build
// process for the ROM image and imported into the updater application
// as a numeric string of type 'Cr16' and id of 4000.
//
#define prvFlashImageCRC16RscType	'Crc'
#define prvFlashImageCRC16RscId		1


// Compatibility string field separator
#define prvCompatFieldSeparator		"::"
#define prvCompatVerRangeSeparator	"-"
#define prvCompatAllString			"*"

typedef enum
  {
	justLeft,
	justCenter,
	justRight
  } JustificationEnum;


typedef enum
  {
	updateIsCompatible,		  // update can be applied to the current module
	updateIsOlder,			  // the current module's software is newer than the update
	updateIsNotVerCompatible, // the current module matches at least one of the
							  //  manuf/card name combinations in the update's
							  //  compatibility list, but not the version ranges.
	updateIsNotCompatible,	  // the current module does not match any
							  //  manuf/card name combinations in the update's
							  //  compatibility list.

	updaterIsNotCompatibleROMMayBeDamaged // the current module's ROM image appears
							  //  to be damaged; this may be due to a previously
							  //  failed update attempt.

  } UpdateCompatibilityEnum;


// -------------------------------------------------------------
// Internal types
// -------------------------------------------------------------

// The following structures are used to display the backup / restore progress

typedef struct
  {
	UInt32	  soFar;
	UInt32	  total;
  } ProgrAmountType;

typedef struct
  {
	ProgrAmountType	major;
	ProgrAmountType	minor;
  } ProgrType;

typedef Boolean
ProgressStatusFuncType (VoidPtr progrInfoP, ProgrType* progressP,
						Char* messageP);

typedef struct
  {
	FormPtr			progressForm;		  // Dialog that displays the progress
	Word			gaugeWidth;			  // Width of the gauge in pixels
	Word			gaugeLevel;			  // Filled-in width of the progress gauge
	ProgressStatusFuncType*	statusFunc;		// function that updates the status

	RectangleType	messageR;			  // Rectangle to display message in
	RectangleType	gaugeR;				  // Rectangle to draw bar in

  } ProgressInfoType, * ProgressInfoPtr;


typedef struct
  {
	ProgressInfoPtr	  progrInfoP;
	ProgrType*		  progrP;
  } EraseProgrUpdateType;




/************************************************************
 * Structure of a Card Header.
 * There is 1 card header in every card that has ROM. The
 *  card header is stored at sysCardHeaderOffset into the card.
 *
 * RAM only cards will not have a card header
 *************************************************************/
#define	memMaxNameLen				32		// size of name and manuf fields
											// including null

// CardHeaderType
typedef struct
  {
	UInt32          initStack;				// initial stack pointer
	UInt32          resetVector;			// reset vector
	UInt32          signature;				// must be sysCardSignature
	UInt16          hdrVersion;				// header version
	UInt16          flags;					// card flags;
	UInt8           name[memMaxNameLen];	// card name
	UInt8           manuf[memMaxNameLen];	// card manufacturer's name
	UInt16          version;				// card version
	UInt32          creationDate;			// card creation date
	UInt16          numRAMBlocks;			// number of RAM blocks on card
	UInt32          blockListOffset;		// offset to RAM block list
	UInt32          readWriteParmsOffset;	// v2: offset to read/write
											// system data if any (in ROM)
	UInt32          readWriteParmsSize;		// v2: size of read/write system
											// data if any (in ROM)
	UInt32          readOnlyParmsOffset;	// v2: offset to read-only system
											// data (in ROM)
	UInt32          bigROMOffset;			// v2: in SmallROM header: where SmallROM
											//  expects bigROM to live
											//  in BigROM header: where BigROM expects
											//  itself to live
	UInt32          checksumBytes;			// v2: size of card image in bytes (for
											// checksum)
	UInt16          checksumValue;			// v2: checksum of card image (from
											// Crc16CalcBlock)
	UInt32          readWriteWorkingOffset;	// v3: offset to read/write
											// working area if any (in
											// ROM)
	UInt32          readWriteWorkingSize;	// v3: size of read/write working
											// area if any (in ROM)

	UInt32          halDispatch;			// v4: pointer to HAL dispatch code

	UInt8           reserved[130];			// to bring us to 0x100 alignment
  }
CardHeaderType;


// Function type to patch SysHandleEvent()
typedef Boolean SysHandleEventFuncType ( EventPtr );


// -------------------------------------------------------------
// Internal prototypes
// -------------------------------------------------------------
static int
StartApplication (void);

static void
EventLoop (void);

static void
StopApplication(void);

static Boolean
CardUpdaterHandleEvent (EventPtr event);

static Boolean
ProgressHandleEvent(EventPtr event);

static void
UpdateConfirmAndStart (void);

static void
UpdateFlashCard (ProgressInfoPtr progrInfoP);


//static void
//DumpField (Char* strP, UInt32 num, Boolean decFmt, Word* yP);

// Displays a text message with a Reset button
#define PrvFatalMessageID(strID)  \
  do  \
	{ \
	  VoidHand	  textH;						\
	  Char*		  textP;						\
	  textH = DmGetResource (strRsc, (strID));	\
	  textP = MemHandleLock (textH);			\
	  ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, textP);	\
	  MemPtrUnlock (textP);						\
	} while (0)

// -------------------------------------------------------------
// Global variables
// -------------------------------------------------------------
static Boolean	ModulePresent = false;
static UInt16	ModuleCardNo = 1;

static Boolean	UpdatePresent = false;

// Buffer for compatibility list entry -- reserve enough space for the
//  Manufacturer name, Card Name, version range, the field separators
static Char		CompatEntryBuffer[(memMaxNameLen * 2) + 32];


// ENGINE_BUILD_VERSION and ENGINE_BUILD_DATE are defined and passed to us
//  by the Makefile for GCC builds
#ifndef ENGINE_BUILD_DATE
  #define ENGINE_BUILD_DATE		  "unknown"
#endif
#ifndef ENGINE_BUILD_VERSION
  #define ENGINE_BUILD_VERSION	  "unknown"
#endif

static Char*	EngineBuildDateStrP = ENGINE_BUILD_DATE;
static Char*	EngineBuildVersionStrP = ENGINE_BUILD_VERSION;

#if 0
#pragma mark -------------------------
#endif


/***********************************************************************
 *
 * Function:     PrvSysHandleEvent
 *
 * Summary:		 Use to disable OS processing of the card's application
 *					execution.
 *
 * Parameters:   eventP - the event that was received and sent to
 *					SysHandleEvent
 *
 * Returns:     Boolean, true if SysHandleEvent handled the event.
 *
 * History:
 *	07-July-2000  FV  Created
 *
 *
 ***********************************************************************/
static Boolean PrvSysHandleEvent(EventPtr eventP)
{
  Err err = 0;
  Boolean handled = false;
  SysHandleEventFuncType * OldSysHandleEventPtr = 0;
  Byte	  removable;

//  CALLBACK_PROLOGUE()

  // Todo:
  // This API takes a lot of overhead.  System performance will decrease
  // because SysHandleEvent() is called often.  Need to optimize by caching
  // the old function pointer.  (Global variables are not allowed in Patches
  // because they can be call outside of this application thus rendering the
  // globals out of context.)

	  if (   (eventP->eType == nilEvent)
		  || (eventP->eType == keyDownEvent)
		  || (eventP->eType == winExitEvent)
		 )
		{


	    if (eventP->eType == keyDownEvent)
		  {

		  switch (eventP->data.keyDown.chr)		// Allow for power off/on
			{
				case autoOffChr:		// fall through
				case hardPowerChr:		// fall through
				case powerOffChr:		// fall through
				  goto Exit;			// handled == false, call old SHE
		  
				case hsChrCardStatusChg:
				  if (eventP->data.keyDown.modifiers & commandKeyMask)
				  {

				  //-------------------------------------------------------------------------
				  // check if card is really installed
				  //-------------------------------------------------------------------------

				  err = HsCardAttrGet (1, hsCardAttrRemovable, &removable);

				  // If removable card found, update globals to allow update
				  if (!err && removable)
					{
					ModuleCardNo = 1;
					ModulePresent = removable;
					}
				  } 
				  break;

			} // switch
		  }

		handled = true;
	  }
Exit:

  // ------------------------------------------------------------
  // Call the old handler
  // ------------------------------------------------------------
  if (handled) 
	return true;
  else
	{
    HsCardPatchPrevProc (sysTrapSysHandleEvent, &OldSysHandleEventPtr);
    handled = (OldSysHandleEventPtr)(eventP);
	}

//  CALLBACK_EPILOGUE()

  return handled;
}


#if 0
#pragma mark -------------------------
#endif


/***************************************************************
 *	Function:	  FormatStr
 *
 *	Summary:	  Helper routine for taking a template string and
 *				  a set of substitution tokens and producing a result
 *				  string by applying the tokens to the teplate.
 *
 *				  The template string is in the format:
 *
 *					"abc^1def^2..."
 *
 *				  where ^1, ^2, etc. will be replaced by the
 *				  corresponding replacement token from the tokens
 *				  array.  Presently only ^1-^9 are supported.
 *
 *
 *
 *	Parameters:
 *	  templP	  IN	Template string pointer
 *	  tokens	  IN	Array of replacement tokens
 *	  numTokens	  OUT	# of tokens in the array
 *
 *
 *	Returns
 *	  pointer to result string or NULL if error.  The caller is
 *	  responsible for freeing the result string via MemPtrFree().
 *
 *	Called By:
 *	  FormatRscStr ()
 *
 *  Notes:
 *	  Presently only ^1-^9 are supported, and the function assumes
 *	   that no pattern is specified more than once.
 *
 *	History:
 *	  2-Jun-1999  VMK	Created by Vitaly Kruglikov
 *
 ****************************************************************/
static Char*
FormatStr (Char* templP, CharPtr tokens[], UInt16 numTokens)
{
  Char*		  tokP;
  Char*		  prevP;
  Char*		  dstP;
  UInt16	  len;
  UInt16	  maxSize;
  UInt16	  i;
  Char*		  strP = NULL;
  VoidHand	  strH;
  const Int	  tokenIdChar = '^';

  // Determine total string size and allocate it
  maxSize = StrLen (templP) + 1;
  for (i=0; i < numTokens; i++)
	maxSize += StrLen (tokens[i]);

  strH = MemHandleNew (maxSize);
  if (!strH)
	  return (NULL);
  strP = MemHandleLock (strH);

  dstP = strP;

  prevP = templP;
  tokP = StrChr (templP, tokenIdChar);

  while (prevP)
	{
	  if (tokP)
		  len = tokP - prevP;
	  else
		  len = StrLen (prevP);

	  if (((dstP - strP) + len) < maxSize)
		{
		  StrNCopy (dstP, prevP, len);
		  dstP += len;
		}

	  if (tokP)
		{
		  Int32		  index;

		  tokP++;	  // advance to the token index char
		  index = StrAToI (tokP);
		  if (index > 0 && index <= (Int32)numTokens)
			{
			  index -= 1;	// make zero-based
			  len = StrLen (tokens[index]);
			  if (((dstP - strP) + len) < maxSize)
				{
				  StrNCopy (dstP, tokens[index], len);
				  dstP += len;
				}
			}

		  tokP++;	// advance past the token index char
		}

	  prevP = tokP;
	  if (tokP)
		  tokP = StrChr (tokP, tokenIdChar);
	}

  *dstP = '\0';

  return (strP);

} // FormatStr


/***************************************************************
 *	Function:	  FormatRscStr
 *
 *	Summary:	  Helper routine for taking a template string and
 *				  a set of substitution tokens and producing a result
 *				  string by applying the tokens to the teplate.
 *
 *				  The template string is in the format:
 *
 *					"abc^1def^2..."
 *
 *				  where ^1, ^2, etc. will be replaced by the
 *				  corresponding replacement token from the tokens
 *				  array.  Presently only ^1-^9 are supported.
 *
 *
 *
 *	Parameters:
 *	  tmplStrRscID	IN	Template string resource ID
 *	  tokens		IN	Array of replacement tokens
 *	  numTokens		OUT	# of tokens in the array
 *
 *
 *	Returns
 *	  pointer to result string or NULL if error.  The caller is
 *	  responsible for freeing the result string via MemPtrFree().
 *
 *	Called By:
 *	  File Mover application code
 *
 *  Notes:
 *	  Presently only ^1-^9 are supported.
 *
 *	History:
 *	  2-Jun-1999  VMK	Created by Vitaly Kruglikov
 *
 ****************************************************************/
static Char*
FormatRscStr (UInt16 tmplStrRscID, CharPtr tokens[], UInt16 numTokens)
{
  Char*		  resultP = NULL;
  VoidHand	  strH;
  Char*		  strP;

  strH = DmGetResource (strRsc, tmplStrRscID);
  ErrNonFatalDisplayIf (!strH, "template str rsc not found");
  if (strH)
	{
	  strP = MemHandleLock (strH);
	  resultP = FormatStr (strP, tokens, numTokens);
	  MemPtrUnlock (strP);
	  DmReleaseResource (strH);
	}

  return (resultP);

} // FormatRscStr



/***********************************************************************
 *
 * FUNCTION:    DrawStringWithEllipsis
 *
 * DESCRIPTION: Draw the string within a limited field, trucating
 *				  the string to fit if necessary
 *
 * PARAMETERS:  strP		IN	pointer to string
 *				fieldWidth	IN	width of field
 *				x			IN	x coordinate of the origin
 *				y			IN	y coordinate of the origin
 *
 * RETURNED:    nothing
 *
 * NOTE:  Assumes the font has been set up by the caller
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			VMK		24-May-1999	Initial version
 *
 ***********************************************************************/
static void
DrawStringWithEllipsis (CharPtr strP, UInt16 fieldWidth, Int16 x, Int16 y,
						JustificationEnum just)
{
  UInt16		strLen;
  UInt16		strWidth;
  UInt16		maxStrWidth;
  UInt16		ellipsWidth = 0;
  const Char	ellips = chrEllipsis;

  ErrNonFatalDisplayIf (!strP, "null arg");

  maxStrWidth = fieldWidth;

  strLen = StrLen (strP);
  strWidth = FntCharsWidth (strP, strLen);

  // If the string fits, draw it entirely
  if (strWidth > fieldWidth)
	{
	  // Take out the width of an ellipsis char
	  ellipsWidth = FntCharWidth (ellips);
	  maxStrWidth -= ellipsWidth;

	  // Keep removing last char until name fits
	  while (strLen > 0 && strWidth > maxStrWidth)
		{
		  strLen--;
		  strWidth -= FntCharWidth (strP[strLen]);
		}
	}

  if (just == justCenter)
	{
	  x += ((fieldWidth - strWidth - ellipsWidth) / 2);
	}
  else if (just == justRight)
	{
	  x += (fieldWidth - strWidth - ellipsWidth);
	}


  // Draw the string
  WinDrawChars(strP, strLen, x, y);

  // Draw the ellipsis, if necessary
  if (ellipsWidth != 0)
	{
	  x += strWidth;
	  WinDrawChars (&ellips, 1, x, y);
	}

  return;

} // DrawStringWithEllipsis



/***************************************************************
 *	Function:	CheckBatteryCharge
 *
 *	Summary:
 *		Verify that the battery is sufficiently charged for
 *				a file transfer operation, alerting the user if not.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		non-zero if there is enough charge
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static Boolean
CheckBatteryCharge (void)
{
  Boolean	enough = true;
  UInt16	curValue, thresholdValue;

  // Check the battery charge
  //
  curValue = SysBatteryInfo(false /*set*/, &thresholdValue, NULL,
							NULL, NULL, NULL, NULL);

  if (curValue <= thresholdValue)
	  enough = false;

  // If not enough, display a warning
  if (!enough)
	  FrmAlert (resAlertIDLowBatteryWarning);


  return (enough);

} // CheckBatteryCharge


/***************************************************************
 *	Function:	ReplaceLabelText
 *
 *	Summary:
 *		Update a label control's text, making sure there is room.
 *
 *	Parameters:
 *		formP		IN	ptr to the form
 *		labelID		IN	id of label control within the form
 *		strP		IN	new text for the label
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Jan-2000	VMK		Created
 *
 ****************************************************************/
static void
ReplaceLabelText (FormPtr formP, Word lableID , CharPtr strP)
{


  // Only copy the label if there is room in the destination
  if (strP)
	{
	  CharPtr	labelTextP;

	  labelTextP = (CharPtr) FrmGetLabel (formP, lableID);
	  if (StrLen(strP) <= StrLen(labelTextP))
		FrmCopyLabel (formP, lableID, strP);
	}
  else
	FrmCopyLabel (formP, lableID, "");

  return;
} // ReplaceLabelText



/***************************************************************
 *	Function:	GetUpdateImageInfo
 *
 *	Summary:
 *		Get information about the update card image.  Pass NULL
 *		 to ignore any optional argument.
 *
 *	Parameters:
 *		cardNameP	OUT	[OPTIONAL] card name -- memMaxNameLen bytes
 *		manufNameP	OUT	[OPTIONAL] manufacturer name -- memMaxNameLen bytes
 *		versionP	OUT	[OPTIONAL] card version
 *		crDateP		OUT [OPTIONAL] card image creation date in TimGetSeconds format
 *		imageSizeP	OUT [OPTIONAL] size of the update card image in # of bytes
 *
 *	Returns:
 *		non-zero if the image is present
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Jan-2000	VMK		Created
 *
 ****************************************************************/
static Boolean
GetUpdateImageInfo (Char* cardNameP, Char* manufNameP,
		  UInt16* versionP, UInt32* crDateP, UInt32* imageSizeP)
{
  Boolean	imagePresent = false;
  UInt32	romSize = 0;
  CardHeaderType* cardHdrP = NULL;
  VoidHand	rscH = 0;
  Int16		i;


  // --------------------------------------------------------------------------
  // First find the head image resource -- if it is present, we assume
  //  that it contains the entire card header (at least)
  // --------------------------------------------------------------------------
  rscH = DmGetResource (prvFlashImageRscType, prvFlashImageRscIdFirst);
  if (!rscH)
	{
	  imagePresent = false;
	  goto Exit;		// no image resource -->
	}

  ErrFatalDisplayIf (MemHandleSize(rscH) < sizeof(CardHeaderType),
					 "head image resource is too small");
  imagePresent = true;

  cardHdrP = MemHandleLock (rscH);

  if (cardNameP)
	StrCopy (cardNameP, (char*)cardHdrP->name);

  if (manufNameP)
	StrCopy (manufNameP, (char*)cardHdrP->manuf);

  if (versionP)
	*versionP = cardHdrP->version;

  if (crDateP)
	*crDateP = cardHdrP->creationDate;


  MemPtrUnlock (cardHdrP);
  cardHdrP = NULL;

  // --------------------------------------------------------------------------
  // Enumerate through all the ROM image resources to discover
  //  the total ROM image size
  // --------------------------------------------------------------------------
  if (imageSizeP)
	{
	  romSize = 0;
	  i = prvFlashImageRscIdFirst;
	  while (true)
		{
		  rscH = DmGetResource (prvFlashImageRscType, i);
		  if (!rscH)
			{
			  //err = DmGetLastErr ();
			  break;
			}

		  romSize += MemHandleSize (rscH);

		  i++;
		}

	  *imageSizeP = romSize;
	}


Exit:
  return imagePresent;
} // GetUpdateImageInfo



/***************************************************************
 *	Function:	MakeCardVersionString
 *
 *	Summary:
 *		Creates an image string suitable for display.
 *
 *	Parameters:
 *		versioNumber		IN	version #
 *
 *	Returns:
 *		ptr to result string; IMPORTANT: the caller is reponsible for
 *		 freeing it via MemPtrFree()
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Jan-2000	VMK		Created
 *    21-Feb-2001	JMP		Modified to support the new fix version
 *							model.
 *						The old version model was:
 *							-Byte [0] -> Major Version.
 *							-Byte [1] -> Minor Version.
 *
 *						The new version model is:
 *							-Bits [0][1] -> bit 0 and 1 (bit 0 set to 1)
 *							-Bits [2][3] and Nibble [1] -> major version
 *							 (range 128)
 *							-Nibble [2] -> minor version (range 16)
 *							-Nibble [3]	-> fix version (range 16)
 *						This was done because of the
 *                      problem we encountered in producing a bug
 *                      fix for ExpCardROM 1.1 after 1.2 had already
 *                      been done and was not compatible with 1.1.
 *
 ****************************************************************/
static CharPtr
MakeCardVersionString (UInt32 versioNumber)
{
  Int32		major, minor;
  CharPtr	strP = NULL;
  Char		majorText[16];
  Char		minorText[16];
  Char		fixText[16];
  CharPtr	tokens[3];

	tokens[0] = majorText;
    tokens[1] = minorText;
    tokens[2] = fixText;

	 major = (versioNumber >> 8) & 0x00FFL;
	 minor = (versioNumber  & 0x00FFL);

	 if (0x80 & major)
	  {
		StrIToA (majorText, (major & 0x3F));
		StrIToA (minorText, (minor & 0xF0)>>4);
		StrIToA (fixText, (minor & 0x0F));
	  }

	else
	  {
		StrIToA (majorText, major);
		StrIToA (minorText, minor);
		StrIToA (fixText, 0);
	  }
	strP = FormatRscStr (resStrCardVersionTemplate, tokens,
  						sizeof(tokens)/sizeof(tokens[0]));

  return strP;

} // MakeCardVersionString



/***************************************************************
 *	Function:	NumStringsInStringList
 *
 *	Summary:
 *		Returns the count of entries (strings) in a given string list
 *		 object.
 *
 *	Parameters:
 *		stlID		IN	resource ID of the string list object
 *
 *	Returns:
 *		count of entries
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    18-Jan-2000	VMK		Created
 *
 ****************************************************************/
static UInt16
NumStringsInStringList (Word stlID)
{
  UInt16	numEntries = 0;
  VoidHand	stlH = 0;
  Byte*		stlP = NULL;



  stlH = DmGetResource ('tSTL', stlID);
  ErrFatalDisplayIf (!stlH, "missing string list object");
  stlP = (Byte*) MemHandleLock (stlH);

  // The 'tSTL' string list object is structured as follows:
  //
  // zero-terminated "prefix string"
  // count of entries -- UInt16, alignment depends on length of prefix
  // zero or more zero-terminated strings (# of entries should match the count field)

  // Skip the prefix string, including the zero-terminator
  stlP += (StrLen((Char*)stlP) + 1);

  // Note: need to do hacked way of reading to avoid non-aligned 16-bit access.
  numEntries = ((*stlP) << 8) + (*(stlP + 1));


  // Unlock the string list (can't use stlP because it was changed)
  MemHandleUnlock (stlH);
  DmReleaseResource (stlH);


  return (numEntries);

} // NumStringsInStringList


/***************************************************************
 *	Function:	TextToUInt16
 *
 *	Summary:
 *		Converts a hexadecimal or decimal number from ASCII text to
 *		 a binary interger. If the number is prefixed with 0x or 0X,
 *		 it is assumed to be hexadecimal; otherwise, it is assumed
 *		 to be in decimal notation.
 *
 *	Parameters:
 *		textP		IN	pointer to the text string containing the number
 *		resultP		OUT	the integer result
 *		badIntP		OUT	true if invalid format detected
 *
 *	Returns:
 *		points to the next character within the string immediately after
 *		 the number that was parsed.
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    18-Jan-2000	VMK		Created
 *
 ****************************************************************/
static Char*
TextToUInt16 (Char* textP, UInt16* resultP, Boolean* badIntP)
{
  Char*	  cP = textP;
  UInt16  mult = 0;

  ErrNonFatalDisplayIf (!textP || !resultP || !badIntP, "null ptr arg");

  *resultP = 0;
  *badIntP = true;	  // assume bad

  // First, determine whether the number is hex or decimal
  if (*cP == '0' && (*(cP+1) == 'x' || *(cP+1) == 'X'))
	{
	  // The number is hexadecimal
	  mult = 16;
	  cP += 2;		// skip the 0x to get to the number

	  while (true)
		{
		  UInt16	digit;

		  if (*cP >= '0' && *cP <= '9')
			digit = (*cP - '0');
		  else if (*cP >= 'a' && *cP <= 'f')
			digit = (10 + *cP - 'a');
		  else if (*cP >= 'A' && *cP <= 'F')
			digit = (10 + *cP - 'A');
		  else
			break;

		  *resultP *= mult;
		  *resultP += digit;

		  cP++;
		}

	  if (cP > (textP + 2))
		*badIntP = false;
	}

  else
	{
	  // The number is decimal
	  mult = 10;

	  while (*cP >= '0' && *cP <= '9')
		{
		  *resultP *= mult;
		  *resultP += (*cP - '0');

		  cP++;
		}

	  if (cP > textP)
		*badIntP = false;
	}


  return (cP);
}

/****************************************************************
 *	Function:
 *	  PrvNormalizeVersionNumber
 *
 *	Summary:
 *		Take a version value and normalize
 *		it to match the new Version model.
 *
 *		The old version model was:
 *		-Byte [0] -> Major Version.
 *		-Byte [1] -> Minor Version.
 *
 *		The new version model is:
 *		-Bits [0][1] -> bit 0 and 1 (bit 0 set to 1)
 *		-Bits [2][3] and Nibble [1] -> major version
 *		-Nibble [2] -> minor version
 *		-Nibble [3]	-> fix version
 *
 *	Parameters:
 *	  UInt16 versionVal
 *
 *	Returns:
 *	  static UInt16
 *
 *	Called by:
 *	  TextToVersion16
 *
 *
 *	Notes:
 *
 *
 *	History:
 *	  2001-2-23 	JMP	Created
 *
 *****************************************************************/
static UInt16
PrvNormalizeVersionNumber (UInt16 versionVal)
{
  UInt16 normalizedVersion;

	if ( 0x8000 & versionVal)
	  {
		normalizedVersion = versionVal;
	  }
	else
	  {
		normalizedVersion =	(UInt16) (	( 0x8000 | (versionVal & 0x3F00) ) |
									  ( ((versionVal & 0x000F)<<4) |
										((versionVal & 0x00F0)>>4)	 )	);
	  }

	return (normalizedVersion);

}//	PrvNormalizeVersionNumber

/***************************************************************
 *	Function:	TextToVersion16
 *
 *	Summary:
 *		Take a version string convert it to a UInt16 and normalize
 *		the value to match the new Version model.
 *
 *
 *	Parameters:
 *		versionStringP	IN	pointer to the version string
 *		versionValP		OUT	the integer result
 *		badIntP			OUT	true if invalid format detected
 *
 *	Returns:
 *		points to the next character within the string immediately after
 *		 the number that was parsed.
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    23-Feb-2001	JMP		Created
 *
 ****************************************************************/
static Char*
TextToVersion16 (Char* versionStringP, UInt16* versionValP, Boolean* badIntP)
{
  Char*	  cP;
  UInt16  tmpVersion;

	*versionValP = 0;

	cP = TextToUInt16 (versionStringP, &tmpVersion, badIntP);
	*versionValP = PrvNormalizeVersionNumber (tmpVersion);

	return  (cP);


}//	TextToVersion16

/***************************************************************
 *	Function:	PrvMemCardInfo
 *
 *	Summary:
 *		private version of MemCardInfo to intercept and normalize
 *		the card Version number.
 *
 *	Parameters:
 *		cardNo		- IN	Card Number
 *		cardNameP	- OUT	pointer to cardName string
 *		manufNameP	- OUT	pointer to manufName string
 *		versionP	- OUT	pointer to version value
 *							(see PrvNormalizeVersionNumber)
 *		crDateP		- OUT	pointer to crDate value
 *		romSizeP	- OUT	pointer to romSize value
 *		ramSizeP	- OUT	pointer to ramSize value
 *		freeBytesP	- OUT	pointer to freeBytes value
 *
 *	Returns:
 *		err from MemCardInfo
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    23-Feb-2001	JMP		Created
 *
 ****************************************************************/
static Err
PrvMemCardInfo(	UInt16 cardNo, Char *cardNameP, Char *manufNameP,
					UInt16 *versionP, UInt32 *crDateP, UInt32 *romSizeP,
					UInt32 *ramSizeP, UInt32 *freeBytesP)
{
  Err		err;
  UInt16	tmpVersion;



	err = MemCardInfo ( cardNo, cardNameP, manufNameP, &tmpVersion, crDateP,
						romSizeP, ramSizeP, freeBytesP);


	if (versionP )//  if we are getting the version then Normalize it
	  {
		*versionP = PrvNormalizeVersionNumber (tmpVersion);
	  }

	return (err);

}//	PrvMemCardInfo

/***************************************************************
 *	Function:	GetUpdaterCompatibility
 *
 *	Summary:
 *		Check if the updater is compatible with the current module.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		UpdateCompatibilityEnum value
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    19-Jan-2000	VMK		Created
 *	  01-Feb-2001	vmk		Check for damaged ROM image
 *	  23-Feb-2001	jmp		Swap MemCardInfo for PrvMemCardInfo
 *
 ****************************************************************/
static UpdateCompatibilityEnum
GetUpdaterCompatibility (void)
{
  Err	  err = 0;
  UpdateCompatibilityEnum compat = updateIsNotCompatible;
  Boolean manufCardNameMatchFound = false;
  Boolean moduleIsOlder = false;
  Boolean moduleIsNewer = false;
  Boolean badInt;
  UInt16  numEntries;
  UInt16  i;
  Char	  moduleName[memMaxNameLen];
  Char	  moduleManuf[memMaxNameLen];
  UInt16  moduleVer;
  Char*	  nameP;
  Char*	  manufP;
  Char*	  verRangeP;
  UInt16  minVer, maxVer;


  // Check if the module is even inserted
  if (!ModulePresent)
	return updateIsNotCompatible;

  // Get infromation about the module
  if (MemNumHeaps(ModuleCardNo) <= 0)
	return updaterIsNotCompatibleROMMayBeDamaged;

  err = PrvMemCardInfo (ModuleCardNo, moduleName, moduleManuf, &moduleVer,
					NULL, NULL, NULL, NULL);

  if (err)
	return updateIsNotCompatible;

  numEntries = NumStringsInStringList (resOvrSTLCompatibilityList);

  for (i=0; i < numEntries; i++)
	{
	  // Get the compatibility entry
	  SysStringByIndex (resOvrSTLCompatibilityList, i, CompatEntryBuffer,
						sizeof(CompatEntryBuffer));

	  // Each entry is formatted as:
	  //  <manuf name>::<card name>::<firstVer-lastVer>
	  //
	  //  <card name> and <manuf name> are the card and manufacturer name strings as
	  //  would appear in the card header (each of these names can be a maximum of 31
	  //  ASCII characters); <firstVer-lastVer> is an inclusive range of card version
	  //  numbers (as would appear in the module's card header) where the version
	  //  number can be specified either in decimal or hex (if using hex, the number
	  //  must be prefixed with 0x -- the number zero followed by the letter 'x').
	  //  Keep in mind that the card version number is a two-byte integer.
	  //
	  //  As a special case, the string whose only content is the '*' (asterisk) character
	  //  ("*") matches any module (i.e., allows the update to be applied to any module,
	  //  regardless of its manufacturer, name, and version).

	  // Check for the special case first
	  if (StrCompare(CompatEntryBuffer, prvCompatAllString) == 0)
		return updateIsCompatible;

	  // Get the manufacturer name
	  manufP = CompatEntryBuffer;

	  // Get the card name
	  nameP = StrStr (manufP, prvCompatFieldSeparator);
	  ErrFatalDisplayIf (!nameP, "invalid compatibility list entry: missing card name");
	  if (!nameP)
		continue;
	  nameP += StrLen (prvCompatFieldSeparator);

	  // Get the version range
	  verRangeP = StrStr (nameP, prvCompatFieldSeparator);
	  ErrFatalDisplayIf (!verRangeP, "invalid compatibility list entry: missing version range");
	  if (!verRangeP)
		continue;
	  verRangeP += StrLen (prvCompatFieldSeparator);

	  // Get the firstVer
	  verRangeP = TextToVersion16 (verRangeP, &minVer, &badInt);
	  if (badInt)
		{
		  ErrFatalDisplay ("invalid compatibility list entry: invalid firstVer");
		  continue;
		}
	  if (StrStr(verRangeP, prvCompatVerRangeSeparator) != verRangeP)
		{
		  ErrFatalDisplay ("invalid compatibility list entry: missing version range separator");
		  continue;
		}

	  verRangeP += StrLen (prvCompatVerRangeSeparator);

	  // Get the lastVer
	  verRangeP = TextToVersion16 (verRangeP, &maxVer, &badInt);
	  if (badInt)
		{
		  ErrFatalDisplay ("invalid compatibility list entry: invalid lastVer");
		  continue;
		}
	  if (StrLen(verRangeP) != 0)
		{
		  ErrFatalDisplay ("invalid compatibility list entry: unexpected data after version range");
		  continue;
		}


	  // Check for manufacturer name match
	  if (StrNCompare (moduleManuf, manufP, StrLen(moduleManuf)) != 0)
		continue;

	  // Check for module name match
	  if (StrNCompare (moduleName, nameP, StrLen(moduleName)) != 0)
		continue;

	  manufCardNameMatchFound = true;

	  if (moduleVer >= minVer && moduleVer <= maxVer)
		return updateIsCompatible;

	  if (moduleVer < minVer)
		moduleIsOlder = true;
	  else
		moduleIsNewer = true;
	}

  // If we got here, we didn't find a compatible match

  // If none of the manufacturer/card name combinations matched,
  //  then the products are completely incompatible
  if (!manufCardNameMatchFound)
	compat = updateIsNotCompatible;

  // If the module has an earlier version number than one of the ranges,
  //  then the update is not compatible with the version of the module
  //  software
  else if (moduleIsOlder)
	compat = updateIsNotVerCompatible;

  // Otherwise, the update is older than the software on the module, and
  //  the module does not need to be updated
  else
	compat = updateIsOlder;



  return (compat);

} // UpdateCompatibilityEnum


/***************************************************************
 *	Function:	IsAdvFuncKeyDown
 *
 *	Summary:
 *		Check if the advanced function key (pg dn key) is activated.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		true if activated, false if not
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    21-Jan-2000	VMK		Created
 *
 ****************************************************************/
static Boolean
IsAdvFuncKeyDown (void)
{
  if (KeyCurrentState() & keyBitPageDown)
	return (true);
  else
	return (false);
}



/***************************************************************
 *	Function:	IsAdv2FuncKeyDown 
 *
 *	Summary:      
 *		Check if the advanced2 function key (pg up & pg dn keys) 
 *		is activated.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		true if activated, false if not
 *  
 *	Called By: 
 *		internal;
 *
 *	Notes:
 *		
 *
 *	History:
 *    21-Jan-2000	VMK		Created
 *
 ****************************************************************/
static Boolean
IsAdv2FuncKeyDown (void)
{
  DWord	keyActive = KeyCurrentState();

  if (keyActive & keyBitPageUp)
	return (true);
  else
	return (false);
}




/***************************************************************
 *	Function:	IsPointInObject
 *
 *	Summary:
 *		Check if a point is inside an object's bounds.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		true if inside, false if not
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    21-Jan-2000	VMK		Created
 *
 ****************************************************************/
static Boolean
IsPointInObject (FormPtr frmP, Word objID, SWord x, SWord y)
{
  Boolean ptInObject = false;
  Word	  objIndex;
  RectangleType	objR;


  // Get the control's index
  objIndex = FrmGetObjectIndex (frmP, objID);

  // Get its boudns
  FrmGetObjectBounds (frmP, objIndex, &objR);

  // See if the point is inside the object
  ptInObject = RctPtInRectangle (x, y, &objR);


  return (ptInObject);

} // IsPointInObject



/***************************************************************
 *	Function:	PrvCrc16CalcBigBlock
 *
 *	Summary:
 *		Utility function for computing CRC-16 of "big" blocks --
 *		i.e. - those whose size is greater than or equal to 64K.
 *		The system function takes a 16 bit block size value, so
 *		we break a large block into multiple sub-blocks and
 *		call the system function for each sub-block.
 *
 *	Parameters:
 *		bufP	  - IN	pointer to block
 *		count	  - IN	size of block
 *		crcSeed	  - IN	result of previous CRC-16 computation if
 *						iterating through sub-blocks of a larger
 *						block, or 0 if calling for the first sub-block.
 *
 *	Returns:
 *		CRC-16 value of the block
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    30-Jan-2001	vmk		Created
 *
 ****************************************************************/
static UInt16
PrvCrc16CalcBigBlock (const void* bufP, UInt32 count, UInt16 crcSeed)
{
  UInt8*		  bufByteP = (UInt8*) bufP;
  const UInt16	  maxChunkSize = (UInt16)32 * (UInt16)1024;
  UInt16		  chunkSize;


  while (count > 0)
	{
	  if (count > maxChunkSize)
		chunkSize = maxChunkSize;
	  else
		chunkSize = (UInt16) count;

	  // Call system to compute the CRC of this sub-block and combine
	  //  it with that of the previous sub-block
	  crcSeed = Crc16CalcBlock (bufByteP, chunkSize, crcSeed);

	  // Update counters for next iterations
	  count -= chunkSize;
	  bufByteP += chunkSize;
	}


  return (crcSeed);
} // PrvCrc16CalcBigBlock


#if 0
#pragma mark -------------------------
#endif


/***********************************************************************
 *
 * FUNCTION:		PilotMain
 *
 * DESCRIPTION:	This function is the equivalent of a main() function
 *				under standard "C".  It is called by the Startup
 *				code to begin execution of this application.
 *
 * PARAMETERS:		cmd - command specifying how to launch the application.
 *						cmdPBP - parameter block for the command.
 *						launchFlags - flags used to configure the launch.
 *
 * RETURNED:		Any applicable error codes.
 *
 ***********************************************************************/
DWord
PilotMain (Word cmd, Ptr cmdPBP, Word launchFlags)
{
  int error;

  if (cmd == sysAppLaunchCmdNormalLaunch)
	{
#ifdef OVERLAYS
// This code uses dougs overlays to allow for internationalization.
// The code installs the overlays from resources that are built
// into the updater prc. See maint/modules/FlashUpdater for an
// example
	  DmOpenRef		  OverlayDbP = NULL;

	  Err err;
	  UInt16 cardNo;
	  LocalID dbID;
	  err = SysCurAppDatabase ( &cardNo, &dbID );
	  if ( err == errNone )
		{
		  Char overlayBaseName[32];
		  err = DmDatabaseInfo ( cardNo, dbID, overlayBaseName,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		  if ( err == errNone )
			{
			  // Install Overlays if needed
			  CountryType country = cUnitedStates;
			  UInt32 language = lEnglish;
			  UInt32 temp;
			  // Find the country/language of the ROM...
			  // <chg 09-dec-99 dia> Allow overriding using 'LanguageSpoofer'...
			  if (FtrGet (hsFtrCreatorOverlayOverrides, hsFtrIDCountryOverride, &temp) != 0)
				{
				  FtrGet(sysFtrCreator, sysFtrNumCountry, &temp);
				}
			  country = (CountryType) temp;

			  if (FtrGet (hsFtrCreatorOverlayOverrides, hsFtrIDLanguageOverride, &language) != 0)
				{
				  FtrGet (sysFtrCreator, sysFtrNumLanguage, &language);
				}
			if ( language != lEnglish && country != cUnitedStates )
			  {
				MemHandle       resH;

				resH = DmGetResource ('HsOl', country*10+language );
				if (resH)
				  {
					DmCreateDatabaseFromImage (MemHandleLock (resH));
					MemHandleUnlock (resH);
					DmReleaseResource (resH);
				  }
				else
				  {
					switch (language)
					  {
						case lEnglish:
						  country = cUnitedKingdom;
						  break;
						case lFrench:
						  country = cFrance;
						  break;
						case lGerman:
						  country = cGermany;
						  break;
						case lItalian:
						  country = cItaly;
						  break;
						case lSpanish:
						  country = cSpain;
						  break;
						case lJapanese:
						  country = cJapan;
						  break;
						default:
						  break;
					  }
					resH = DmGetResource ('HsOl', country*10+language );
				  if (resH)
					{
					  DmCreateDatabaseFromImage (MemHandleLock (resH));
					  MemHandleUnlock (resH);
					  DmReleaseResource (resH);
					}
				  }
			  }

			  OverlayDbP = HsUtilOverlayInitialize (overlayBaseName);
			}
		}
#endif
	  error = StartApplication();	// Application start code
	  if (error) return error;

	  EventLoop();					// Event loop

	  StopApplication ();			// Application stop code
#ifdef OVERLAYS
	  // Delete Overlays
	  if ( OverlayDbP )
		HsUtilOverlayCleanup (OverlayDbP);
#endif
	}
  return 0;

} // PilotMain



/***************************************************************
 *	Function:	StartApplication
 *
 *	Summary:
 *		Start the application.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		0 if no error
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-May-1999	VMK		Created
 *	  14-Jan-2000	vmk		Added removable card lookup
 *
 ****************************************************************/
static int
StartApplication (void)
{
  Err	  err = 0;
  UInt16  cardNo;
  Byte	  removable;

  // -----------------------------------------------------------
  // Look for the first removable card.
  // -----------------------------------------------------------
  err = 0;
  removable = false;
  cardNo = 0;
  for (cardNo = 0; cardNo < MemNumCards(); cardNo++)
	{
	  err = HsCardAttrGet (cardNo, hsCardAttrRemovable, &removable);
	  if (err || removable)
		break;
	}

  // If removable card found, save its index in our globals
  if (!err && removable)
	{
	  ModulePresent = true;
	  ModuleCardNo = cardNo;
	}

  // Check if the update card image is present
  UpdatePresent = GetUpdateImageInfo (NULL, NULL, NULL, NULL, NULL);


  // -----------------------------------------------------------
  // Bring up the main form.
  // -----------------------------------------------------------
  FrmGotoForm (resFormIDCardUpdater);
  return 0;

} // StartApplication



/***************************************************************
 *	Function:	StopApplication
 *
 *	Summary:
 *		Stop the application.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-May-1999	VMK		Created
 *	  26-Jan-2000	vmk		Call FrmCloseAllForms so that our main
 *							 form receives the frmCloseEvent and
 *							 performs necessary cleanup.
 ****************************************************************/
static void
StopApplication (void)
{
	// Close all open forms,  this will send frmCloseEvent to any open
	//  forms, allowing them to clean up.  frmCloseEvent is *not* sent
	//  automatically
	FrmCloseAllForms ();

	return;

} // StopApplication



/***************************************************************
 *	Function:	ApplicationHandleEvent
 *
 *	Summary:
 *		This routine loads form resources and sets the event
 *              handler for the form loaded.
 *
 *	Parameters:
 *		event		IN	a pointer to an EventType structure
 *
 *	Returns:
 *		true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static Boolean
ApplicationHandleEvent (EventPtr event)
{
  Word	  formID;
  FormPtr frmP;

  if (event->eType == frmLoadEvent)
	{
	  // Load the form resource.
	  formID = event->data.frmLoad.formID;
	  frmP = FrmInitForm (formID);
	  FrmSetActiveForm (frmP);

	  // Set the event handler for the form.  The handler of the currently
	  // active form is called by FrmHandleEvent each time it receives an
	  // event.
	  switch (formID)
		{

		  case resFormIDCardUpdater:
			FrmSetEventHandler (frmP, (FormEventHandlerPtr) CardUpdaterHandleEvent);
			break;

		  case resFormIDProgress:
			FrmSetEventHandler (frmP, (FormEventHandlerPtr) ProgressHandleEvent);
			break;

		  default:
			ErrNonFatalDisplayIf (true, "Unexpected formID");
		}
	  return (true);
	}

  return (false);

} // ApplicationHandleEvent


/***************************************************************
 *	Function:	GetAndHandleEvent
 *
 *	Summary:
 *		Gets and handles a single event.
 *
 *	Parameters:
 *		timeout	  IN	timeout (evtWaitForever to wait for next
 *								 event, 0 to return immediately)
 *
 *	Returns:
 *		id of event that was handled (nilEvent, if none)
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static DWord
GetAndHandleEvent (SDWord timeout)
{
  Word error;
  EventType event;

  EvtGetEvent (&event, timeout);

  if (! SysHandleEvent (&event))

	  if (! MenuHandleEvent (0, &event, &error))

		  if (! ApplicationHandleEvent (&event))

			  FrmDispatchEvent (&event);

  return (event.eType);

} // GetAndHandleEvent



/***************************************************************
 *	Function:	EventLoop
 *
 *	Summary:
 *		Event loop of the application.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static void
EventLoop (void)
{
  DWord	eventId;

  do
	{
	  eventId = GetAndHandleEvent (evtWaitForever);
	}
  while (eventId != appStopEvent);
}


/***************************************************************
 *	Function:	PrvProcessCurrentEvents
 *
 *	Summary:
 *		Process all events currently in the event queue and check
 *		whether there is an appStopEvent.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		non-zero if an appStopEvent was detected
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static Boolean
PrvProcessCurrentEvents (void)
{
  Boolean	appExitRequest = false;
  DWord		eventId;
  UInt32	startTicks;
  UInt32	maxTicks;

  startTicks = TimGetTicks ();
  maxTicks = SysTicksPerSecond () * 1;

  do
	{
	  eventId = GetAndHandleEvent (0 /*timeout*/);

	  if (eventId == appStopEvent)
		appExitRequest = true;
	}
  while (EvtEventAvail() && ((TimGetTicks() - startTicks) <= maxTicks));


  return (appExitRequest);

} // PrvProcessCurrentEvents



/***************************************************************
 *	Function:	DisplayCardHeaderInfo
 *
 *	Summary:
 *		Display Card Header information either for the module
 *		or for the update.
 *
 *	Parameters:
 *		forModule	IN  if ture, displays info for the module; otherwise
 *						 for the update
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		CardUpdaterHandlePenDown()
 *
 *	Notes:
 *
 *
 *	History:
 *    21-Jan-2000	VMK		Created
 *
 ****************************************************************/
static void
DisplayCardHeaderInfo (Boolean forModule)
{
  VoidHand	textH;
  CharPtr	textP;
  Char	  manufName[memMaxNameLen];
  Char	  cardName[memMaxNameLen];
  UInt16  cardVer;
  Char	  cardVerText[32];
  UInt32  crDate;
  Char	  crDateText[longDateStrLength+1];
  Char	  crTimeText[timeStringLength+1];
  Char	  crDateTimeHexText[32];
  UInt32  imageSize = 0;
  Char	  imageSizeText[32];
  DateTimeType	  dateTime;

  FormPtr frmP = NULL;
  CharPtr dateTimeTextP = NULL;
  CharPtr dateTimeTokens[3] ;
  
  dateTimeTokens[1] = crDateText;
  dateTimeTokens[2] = crTimeText;
  dateTimeTokens[3] = crDateTimeHexText;

  if (forModule)
	{
	  if (!ModulePresent)
		{
		  FrmAlert (resAlertIDErrNoExpCardGeneric);
		  return;
		}

	  PrvMemCardInfo (ModuleCardNo, cardName, manufName, &cardVer, &crDate,
				   NULL, NULL, NULL);
	}
  else
	{
	  if (!UpdatePresent)
		{
		  FrmAlert (resAlertIDErrNoFlashImage);
		  return;
		}

	  GetUpdateImageInfo (cardName, manufName, &cardVer, &crDate, &imageSize);
	}


  frmP = FrmInitForm (resFormIDCardInfo);
  if (!frmP)
	goto Exit;

  if (forModule)
    textH = DmGetResource (strRsc, resStrIDCardInfoModuleTitle);
  else
    textH = DmGetResource (strRsc, resStrIDCardInfoUpdateTitle);
  FrmSetTitle (frmP, MemHandleLock(textH));


  StrPrintF (cardVerText, "%4x", (int)cardVer);

  TimSecondsToDateTime(crDate, &dateTime);
  TimeToAscii((Byte)dateTime.hour, (Byte)dateTime.minute, tfColon24h, crTimeText);
  DateToAscii ((Byte)dateTime.month, (Byte)dateTime.day, (Word)dateTime.year,
				dfMDYLongWithComma, crDateText);
  StrPrintF (crDateTimeHexText, "%lx", (long)crDate);
  dateTimeTextP = FormatRscStr (resStrCardInfoDateTimeTemplate, dateTimeTokens,
						sizeof(dateTimeTokens)/sizeof(dateTimeTokens[0]));


  ReplaceLabelText (frmP, resLabelIDCardInfoManufName,	manufName);
  ReplaceLabelText (frmP, resLabelIDCardInfoCardName,	cardName);

  ReplaceLabelText (frmP, resLabelIDCardInfoVerNum,		cardVerText);
  ReplaceLabelText (frmP, resLabelIDCardInfoCrDate,		dateTimeTextP);

  StrPrintF (imageSizeText, "%ld", (long)imageSize);

  if (forModule)
	{
	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDCardInfoSizeTag));
	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDCardInfoSize));

	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDUpdaterBuildDateTag));
	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDUpdaterBuildDate));

	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDEngineBuildDateTag));
	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDEngineBuildDate));

	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDEngineBuildVersionTag));
	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDEngineBuildVersion));

	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDEngineBuildTypeTag));
	  FrmHideObject (frmP, FrmGetObjectIndex (frmP, resLabelIDEngineBuildType));
	}
  else
	{
	  Char*		  strP;
	  MemHandle	  strH;
	  UInt16	  buildTypeID;

	  ReplaceLabelText (frmP, resLabelIDCardInfoSize,		imageSizeText);

	  strH = DmGetResource (strRsc, resOvrStrIDBuildDate);
	  if (strH)
		{
		  strP = MemHandleLock (strH);
		  ReplaceLabelText (frmP, resLabelIDUpdaterBuildDate, strP);
		  MemPtrUnlock (strP);
		}
	  ReplaceLabelText (frmP, resLabelIDEngineBuildDate,	EngineBuildDateStrP);
	  ReplaceLabelText (frmP, resLabelIDEngineBuildVersion,	EngineBuildVersionStrP);

	  #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
		buildTypeID = resStrCardInfoBuildTypeDebug;
	  #else
		buildTypeID = resStrCardInfoBuildTypeRelease;
	  #endif
	  strH = DmGetResource (strRsc, buildTypeID);
	  strP = MemHandleLock (strH);
	  ReplaceLabelText (frmP, resLabelIDEngineBuildType, strP);
	  MemPtrUnlock (strP);
	}

  FrmDoDialog (frmP);


Exit:
  if (frmP)
	{
	  // Release the form's title text string
	  textP = (CharPtr) FrmGetTitle (frmP);
	  FrmSetTitle (frmP, "");
	  if (textP)
		MemPtrUnlock (textP);

	  if (dateTimeTextP)
		MemPtrFree (dateTimeTextP);

	  // Delete the form
	  FrmDeleteForm (frmP);

	}


  return;

} // DisplayCardHeaderInfo



/***************************************************************
 *	Function:	CardUpdaterHandlePenDown
 *
 *	Summary:
 *		Perform advanced functions, such as displaying extra
 *		update or module information, if requested.
 *
 *		If the user taps on the Update or Module label while
 *		pressing the down key, the info will be displayed.
 *
 *	Parameters:
 *		event	IN	event to handle
 *
 *	Returns:
 *		non-zero if handled
 *
 *	Called By:
 *		CardUpdaterHandleEvent()
 *
 *	Notes:
 *
 *
 *	History:
 *    21-Jan-2000	VMK		Created
 *
 ****************************************************************/
static Boolean
CardUpdaterHandlePenDown (EventPtr eventP)
{
  Boolean	handled = false;
  FormPtr	frmP;

  ErrNonFatalDisplayIf (eventP->eType != penDownEvent, "bad event");

  frmP = FrmGetActiveForm ();


  // Check if the "advanced functions" key is activated
  if (!IsAdvFuncKeyDown())
	return (false);	  // no? -->


  // Check if the pen landed in one of our objects
  if (IsPointInObject (frmP, resLabelVerUpdateLabel, eventP->screenX,
						eventP->screenY))
	{
	  // Display info for update ROM image
	  DisplayCardHeaderInfo (false /*forModule*/);
	  handled = true;
	}
  else if (IsPointInObject (frmP, resLabelVerModuleLabel, eventP->screenX,
							eventP->screenY))
	{
	  // Display info for ROM image on the module
	  DisplayCardHeaderInfo (true /*forModule*/);
	  handled = true;
	}


  return (handled);

} // CardUpdaterHandlePenDown


/***************************************************************
 *	Function:	CardUpdaterHandleEvent
 *
 *	Summary:
 *		Main Form Event Handler.
 *
 *	Parameters:
 *		event	IN	event to handle
 *
 *	Returns:
 *		non-zero if handled
 *
 *	Called By:
 *		As a callback by the PalmOS Event Manager;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-May-1999	VMK		Created
 *	  18-Jan-2000	vmk		Check compatibility of updater with
 *							 the current module, and use the
 *							 appropriate labels
 *
 ****************************************************************/
static Boolean
CardUpdaterHandleEvent (EventPtr event)
{
  Err		err = 0;
  UInt16	cardVer;
  CharPtr	strP;
  FormPtr   formP;
  Boolean   handled = false;

  switch (event->eType)
	{
	  case frmOpenEvent:
		{
		VoidHand  textH;
		//FieldPtr  fldP;

		formP = FrmGetActiveForm();

		#if 0
			// Set the date/time text
			fldP = (FieldPtr)FrmGetObjectPtr (formP, FrmGetObjectIndex (formP,
													  resFieldIDBuildDate));

			textH = DmGetResource (strRsc, resOvrStrIDBuildDate);
			ErrFatalDisplayIf (!textH, "resOvrStrIDBuildDate resource missing");

			FldSetTextHandle (fldP, (Handle)textH);

			// Set the description text
			fldP = (FieldPtr)FrmGetObjectPtr (formP, FrmGetObjectIndex (formP,
													  resFieldIDDescription));
			textH = DmGetResource (strRsc, resOvrStrIDMainFormDescription);
			ErrFatalDisplayIf (!textH,
							  "resOvrStrIDMainFormDescription resource missing");

			FldSetTextHandle (fldP, (Handle)textH);
		#endif

		// Set the form's title
		textH = DmGetResource (strRsc, resOvrStrIDMainFormTitleText);
		ErrFatalDisplayIf (!textH, "resOvrStrIDMainFormTitleText resource missing");
		FrmSetTitle (formP, MemHandleLock(textH));



		// --------------------------------------------------------------------
		// Set the software version labels
		// --------------------------------------------------------------------

		// Set the module version label
		strP = NULL;
		if (ModulePresent)
		  {
			UpdateCompatibilityEnum	comp;

			comp = GetUpdaterCompatibility ();

			if (comp == updateIsCompatible || comp == updateIsOlder)
			  {
				err = PrvMemCardInfo (ModuleCardNo, NULL, NULL, &cardVer, NULL,
								  NULL, NULL, NULL);
				ErrFatalDisplayIf (err, "MemCardInfo failed");

				strP = MakeCardVersionString ((UInt32) cardVer);
			  }
			else if (comp == updaterIsNotCompatibleROMMayBeDamaged)
			  strP = FormatRscStr (resStrCardVersionROMIsDamaged, NULL, 0);
			else
			  strP = FormatRscStr (resStrCardVersionIncompatibleWithUpdate, NULL, 0);

		  }
		else
		  {
			strP = FormatRscStr (resStrCardVersionNoModule, NULL, 0);
		  }

		ReplaceLabelText (formP, resLabelVerModuleInfo, strP);
		if (strP)
		  {
			MemPtrFree (strP);
			strP = NULL;
		  }


		// Set the update version label
		strP = NULL;
		if (UpdatePresent)
		  {
			GetUpdateImageInfo (NULL, NULL, &cardVer, NULL, NULL);
			strP = MakeCardVersionString ((UInt32) cardVer);
		  }
		else
		  {
			strP = FormatRscStr (resStrUpdateVersionNoImage, NULL, 0);
		  }

		ReplaceLabelText (formP, resLabelVerUpdateInfo, strP);
		if (strP)
		  {
			MemPtrFree (strP);
			strP = NULL;
		  }

		// --------------------------------------------------------------------
		// Finally, draw the form
		// --------------------------------------------------------------------
		FrmDrawForm(formP);
		handled = true;
		}
		break;

	  case frmCloseEvent:
 		{
		//FieldPtr  fldP;
		CharPtr	  textP;

		formP = FrmGetActiveForm();

		#if 0
			fldP = (FieldPtr)FrmGetObjectPtr (formP, FrmGetObjectIndex (formP,
													  resFieldIDBuildDate));

			FldSetTextHandle (fldP, 0);	  // so it doesn't get deleted when form is destroyed

			fldP = (FieldPtr)FrmGetObjectPtr (formP, FrmGetObjectIndex (formP,
													  resFieldIDDescription));
			FldSetTextHandle (fldP, 0);	  // so it doesn't get deleted when form is destroyed
		#endif

		// Release the form's title text string
		textP = (CharPtr) FrmGetTitle (formP);
		FrmSetTitle (formP, "");
		MemPtrUnlock (textP);


		handled = false;
		}
		break;

	  case ctlSelectEvent:  // A control button was pressed and released.
		if (event->data.ctlEnter.controlID == resButtonIDUpdateFlashCard)
		  {
			UpdateConfirmAndStart ();
			handled = true;
		  }
		else
		if (event->data.ctlEnter.controlID == resButtonIDHelp)
		  {
			FrmHelp (resOvrStrIDHelpText);
			handled = true;
		  };
		break;

	  case menuEvent:
		switch (event->data.menu.itemID)
		  {
			case resMenuItemUpdateFlashCard:
			  UpdateConfirmAndStart ();
			  break;

		  }
		handled = true;
		break;

	  case penDownEvent:
		handled = CardUpdaterHandlePenDown (event);
		break;

	  default:
		break;
	}

  return handled;

} // CardUpdaterHandleEvent





#if 0
#pragma mark -------------------------
#endif


/***************************************************************
 *	Function:	UpdateConfirmAndStart
 *
 *	Summary:
 *		Confirm and start an Update operation.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 *	  18-Jan-2000	vmk		Check for compatibility of updater with
 *							 the current module; check for "force update"
 ****************************************************************/
static void
UpdateConfirmAndStart (void)
{
  Word		buttonHit;
  VoidHand	textH;
  UpdateCompatibilityEnum compatibility;
  Boolean	forceUpdate = false;


  Err err = 0;

  // Check if the user wants to enter the recover mode to be able to handle 
  // corrupted modules (without crashing the system); the backdoor for entering 
  // this recover mode is is to tap the "Update Now" button while pressing 
  // BOTH the "page down" key and the "page up" key.
  if (IsAdv2FuncKeyDown())
	{

	// ---------------------------------------------------------
	// Install our SysHandleEvent patch so that the System Event 
	// Handler can be detect card removal and close any libraries
	// ---------------------------------------------------------
	err = HsCardPatchInstall (sysTrapSysHandleEvent, (void*)PrvSysHandleEvent); 
	ErrNonFatalDisplayIf (err, "patch install err");

    FrmAlert (resAlertIDInsertModulePrompt);

	// Check if a module is present
	if (ModulePresent)
		buttonHit = FrmAlert (resAlertIDWarnBlindUpdate);
	else
	  {
		// The module is not present -- inform the user and bail out
		textH = DmGet1Resource (strRsc, resOvrStrIDModuleProductName);
		ErrFatalDisplayIf (!textH, "resOvrStrIDModuleProductName resource is missing");
		FrmCustomAlert (resAlertIDErrNoExpCard, MemHandleLock(textH), 
									NULL, NULL);
		MemHandleUnlock (textH);

		buttonHit = 1;
	  }
  

	err = HsCardPatchRemove (sysTrapSysHandleEvent);
	ErrNonFatalDisplayIf (err, "patch remove err");

	if (buttonHit != 0)
	  {
		EventType event;
		event.eType = appStopEvent;

		EvtAddEventToQueue( &event );	// Too be safe from side-effects, 
		return;						// quit application
	  }

	goto Update;

	}
  // Check if the user wants to force the update, regardless of the compatibility
  //  settings; the backdoor for forcing the update is to tap the "Update Now"
  //  button while pressing the "page down" key.
  else if (IsAdvFuncKeyDown())
	{
	forceUpdate = true;
	}

  // Check if a module is present
  if (!ModulePresent)
	{
	  // The module is not present -- inform the user and bail out
	  textH = DmGetResource (strRsc, resOvrStrIDModuleProductName);
	  ErrFatalDisplayIf (!textH, "resOvrStrIDModuleProductName resource is missing");
	  FrmCustomAlert (resAlertIDErrNoExpCard, MemHandleLock(textH),
								  NULL, NULL);
	  MemHandleUnlock (textH);

	  return;
	}


  // Determine if the update is compatible with the current module
  compatibility = GetUpdaterCompatibility ();


  // If ROM appears to be damaged, automatically offer the user to
  //  update it (with appropriate explanation)
  if (compatibility == updaterIsNotCompatibleROMMayBeDamaged)
	forceUpdate = true;


  // If the update is not compatible with the current module and
  //  it is not being forced, the display the appropriate error
  //  message and bail out
  if (compatibility != updateIsCompatible && !forceUpdate)
	{
	  if (compatibility == updateIsOlder)
		{
		  FrmAlert (resAlertIDInfoModuleIsNewer);
		}
	  else if (compatibility == updateIsNotVerCompatible)
		{
		  FrmAlert (resAlertIDInfoModuleIsNotVerCompatible);
		}
	  else
		{
		  FrmAlert (resAlertIDInfoModuleIsNotCompatible);
		}

	  return;	  // bail out
	}


  // Prompt the user before proceeding with the update
  if (compatibility == updateIsCompatible)
	{
	  // Prompt if the user really wants to do this
	  textH = DmGetResource (strRsc, resOvrStrIDStartUpdateAlertMsg);
	  ErrFatalDisplayIf (!textH, "resOvrStrStartUpdateAlertMsg resource is missing");
	  buttonHit = FrmCustomAlert (resAlertStartFlashing, MemHandleLock(textH),
								  NULL, NULL);
	  MemHandleUnlock (textH);

	  if (buttonHit != 0)
		  return;				// if not, then -->
	}

  else
	{
	  if (compatibility == updaterIsNotCompatibleROMMayBeDamaged)
		{
		  // Display the generic warning about re-flashing a damaged module
		  buttonHit = FrmAlert (resAlertIDWarnStartDamagedROMUpdate);
		  if (buttonHit != 0)
			  return;				// if not, then -->
		}
	  else
		{
		  // Display the generic warning about re-flashing an incompatible module
		  buttonHit = FrmAlert (resAlertIDWarnStartIncompatibleUpdate);
		  if (buttonHit != 0)
			  return;				// if not, then -->
		}
	}

Update:
  // Erase the main form so its image won't get saved
  //FrmEraseForm (FrmGetActiveForm ());

  // Pop up the Progress form -- the logic will continue when
  //  the receives the "form open" event
  FrmPopupForm (resFormIDProgress);

  return;

} // UpdateConfirmAndStart



/***************************************************************
 *	Function:	ProgressUpdateGauge
 *
 *	Summary:
 *		Update the progress indicator.
 *
 *	Parameters:
 *		gaugeRP		IN  gauge rectangle
 *		gaugeLevel	IN	completion level (in pixels)
 *
 *	Returns:
 *		void
 *
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static void
ProgressUpdateGauge (RectangleType*	gaugeRP, ULong gaugeLevel)
{
  RectangleType 	eraseR;
  RectangleType 	completeR;


  ErrNonFatalDisplayIf (!gaugeRP, "null arg");


  // Draw the frame of gadget
  WinDrawRectangleFrame (simpleFrame, gaugeRP);


  // Fill in the gauge.
  if (gaugeLevel > (ULong)gaugeRP->extent.x)
	{
	  ErrNonFatalDisplay ("gaugeLevel out of bounds");
	  gaugeLevel = gaugeRP->extent.x;
	}

  completeR.topLeft.x = gaugeRP->topLeft.x;
  completeR.topLeft.y = gaugeRP->topLeft.y;
  completeR.extent.x = (Word)gaugeLevel;
  completeR.extent.y = gaugeRP->extent.y;
  WinDrawRectangle (&completeR, 0);


  // Erase the empty portion of the gauge (not really necessary).
  eraseR.topLeft.x = (Word)(gaugeRP->topLeft.x + gaugeLevel);
  eraseR.topLeft.y = gaugeRP->topLeft.y;
  eraseR.extent.x = (Word)(gaugeRP->extent.x - gaugeLevel);
  eraseR.extent.y = gaugeRP->extent.y;
  WinEraseRectangle (&eraseR, 0);

  return;

} // ProgressUpdateGauge



/***************************************************************
 *	Function:	ProgressCallback
 *
 *	Summary:
 *		Update the progress indicator and/or message.
 *
 *	Parameters:
 *		progressInfoP	IN  progress info
 *		progressP		IN	[OPTIONAL] new progress values
 *		messageP		IN	[OPTIONAL] new progress message
 *
 *	Returns:
 *		non-zero, if stop is pending
 *
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 *	  12-Sep-2000	vmk		Disable event handling
 ****************************************************************/
static Boolean
ProgressCallback (void* progressInfoP, ProgrType* progressP, Char* messageP)
{
  Word		gaugeLevel = 0;
  ProgressInfoPtr infoP = (ProgressInfoPtr) progressInfoP;
  Word		itemWidth = 0;



  // Reset the auto-off timer to make sure we don't fall asleep in the
  //  middle of the update
  EvtResetAutoOffTimer ();

  #if 0	  // Disable event handling to prevent alarms and other code that may
		  //  execute on our task from accessing the filesystem while the
		  //  module's filesystem is in flux!!!
	// --------------------------------------------------------------------------
	// Process any pending events, such as those for our Stop button
	// --------------------------------------------------------------------------
	while (GetAndHandleEvent (0 /*timeout*/) != nilEvent)
  		{/*intentionally left blank*/}
  #endif


  // Update the progress gauge
  if (progressP)
	{
	  // Update the progress gauge counts
	  ErrNonFatalDisplayIf (!progressP->major.total, "major denominator is zero");

	  if (progressP->major.total)
		  gaugeLevel = (Word) ((progressP->major.soFar * infoP->gaugeWidth)
								  / progressP->major.total);

	  if (progressP->minor.soFar)
		{
		  ErrNonFatalDisplayIf (!progressP->minor.total, "minor denominator is zero");

		  if (progressP->major.total)
			  itemWidth = (Word)(infoP->gaugeWidth / progressP->major.total);
		  if (itemWidth && progressP->minor.total)
			  gaugeLevel += (Word) ((progressP->minor.soFar * itemWidth)
									  / progressP->minor.total);
		}

	  if (gaugeLevel != infoP->gaugeLevel)
		{
		  infoP->gaugeLevel = gaugeLevel;

		  ProgressUpdateGauge (&infoP->gaugeR, gaugeLevel);

		}

	} // Update the progress gauge


  // Update the progress message
  if (messageP)
	{
	  FontID	currFont;


	  // Switch to bold, saving the current font
	  currFont = FntSetFont (largeBoldFont);

	  WinEraseRectangle (&infoP->messageR, 0 /*cornerDiam*/);

	  DrawStringWithEllipsis (messageP, infoP->messageR.extent.x,
							  infoP->messageR.topLeft.x,
							  infoP->messageR.topLeft.y, justCenter);

	  // Restore font
	  FntSetFont (currFont);
	}


  return (false);

} // ProgressCallback



/***************************************************************
 *	Function:	ProgressFormInitAndDraw
 *
 *	Summary:
 *		Initialize and draw the progress form.
 *
 *	Parameters:
 *		frmP	  IN  pointer to the allocated progress form
 *
 *	Returns:
 *		pointer to the progress info structures.  The caller
 *		is responsible for freeing this structure via MemPtrFree
 *		when it's no longer needed
 *
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static ProgressInfoPtr
ProgressFormInitAndDraw (FormPtr frmP)
{
  ProgressInfoPtr infoP = NULL;


  ErrNonFatalDisplayIf (!frmP, "nil form");


  // --------------------------------------------------------------------------
  // Allocate and initialize the Progress Info structure
  // --------------------------------------------------------------------------
  infoP = MemPtrNew (sizeof (ProgressInfoType));
  ErrNonFatalDisplayIf (!infoP, "alloc failed");
  if (!infoP)
	  return (NULL);
  MemSet (infoP, sizeof (*infoP), 0);

  // Get bounds of the progress message and gauge objects once
  FrmGetObjectBounds (frmP, FrmGetObjectIndex (frmP, resGadgetIDProgressMessage),
					  &infoP->messageR);

  FrmGetObjectBounds (frmP, FrmGetObjectIndex (frmP, resGadgetIDProgressBar),
					  &infoP->gaugeR);


  // Init the rest of the progress info structure
  infoP->progressForm = frmP;
  infoP->gaugeWidth = infoP->gaugeR.extent.x;
  infoP->gaugeLevel = 0;
  infoP->statusFunc = ProgressCallback;

  // Draw the form
  FrmDrawForm (frmP);

  // Initialize the gauge
  ProgressUpdateGauge (&infoP->gaugeR, 0);

  return (infoP);

} // ProgressFormInitAndDraw



/***************************************************************
 *	Function:	ProgressHandleEvent
 *
 *	Summary:
 *		This routine is the event handler for the progress dialog.
 *
 *	Parameters:
 *		none
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-Sep-1999	VMK		Created
 ****************************************************************/
static Boolean
ProgressHandleEvent (EventPtr event)
{
  FormPtr frmP;
  Boolean handled = false;


  if (event->eType == frmOpenEvent)
	{
	  ProgressInfoPtr progrInfoP;

	  frmP = FrmGetActiveForm ();

	  // Initialize the form gadgetry and allocate the Progress Info structure
	  progrInfoP = ProgressFormInitAndDraw (frmP);

	  if (progrInfoP)
		{
		  // Start the operation
		  UpdateFlashCard (progrInfoP);

		  // Free the Progress Info structure
		  MemPtrFree (progrInfoP);
		}

	  // Return to main form and update its UI
	  FrmReturnToForm (resFormIDCardUpdater);

	  handled = true;
	}


  else if (event->eType == ctlSelectEvent)
	{
	}


  return (handled);

} // ProgressHandleEvent




#if 0
#pragma mark -------------------------
#endif



/***************************************************************
 *	Function:	UpdateFlashCard
 *
 *	Summary:
 *		Flash a binary image onto a flash card.
 *
 *	Parameters:
 *		progrInfoP	  IN  progress info
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *		If successful, this function doesn't return because it soft-
 *		resets the handheld;
 *
 *		This function uses ErrFatalDisplay(If) to catch programmer
 *		errors -- this will result on a Fatal Error dialog, requiring
 *		a soft-reset of the device.  This should not occur on an
 *		end-user system, since the specific updater is expected to
 *		have been tested and debugged by the time it gets into the end-user's
 *		hands.
 *
 *
 *	History:
 *    14-May-1999	VMK		Created
 *	  14-Sep-1999	vmk		Implemented progress dialog
 *	  14-Sep-1999	vmk		Check battery condition
 *	  14-Sep-1999	vmk		Reserve the Memory Manager semaphore
 *							 while we modify the module's card
 *							 image.
 *	  23-Jun-2000	vmk		Added verification step after flashing.
 *	  30-Jan-2001	vmk		Implement CRC-16 validation
 *	  30-Jan-2001	vmk		Flash ROM signature after the rest of the
 *							ROM image to allow for recovery after
 *							flash failure.
 *	  30-Jan-2001	vmk		Set flmOpenFlagAutoScan based on a
 *							customizeable setting from a resource.
 *	  31-Jan-2001	vmk		Account for post-flashing ROM verification
 *							 the progress bar.
 *	  01-Feb-2001	vmk		Display a Fatal Error message if the
 *							 corresponding alert for damaged updater
 *							 or corrupted flash ROM is ignored by
 *							 the user.
 *	  13-Feb-2001	jmp		Changed from flmOpenFlagAutoScan flag to
 *							  flmOpenChipSelectAutoConfig.
 *
 ****************************************************************/

#define prvEraseProgrMax	64
#define prvWriteProgrMax	64
#define prvValidateROMMax	16

static Byte
EraseProgressCBFunc (FlmProgressReportType* reportP)
{
  EraseProgrUpdateType*	  progrUpdateP;
  ProgressInfoPtr		  progrInfoP;
  ProgrType*			  progrP;

  progrUpdateP = (EraseProgrUpdateType*) reportP->userRefP;
  progrInfoP = progrUpdateP->progrInfoP;
  progrP = progrUpdateP->progrP;

  if (reportP->total)
	  progrP->major.soFar = (prvEraseProgrMax * reportP->soFar) / reportP->total;

  progrInfoP->statusFunc (progrInfoP, progrP, NULL);


  return (false);

} // EraseProgressCBFunc


static void
UpdateFlashCard (ProgressInfoPtr progrInfoP)
{
  DWord			  err = 0;
  VoidHand		  textH;
  FlmInterfaceType  flmApi;
  void*			  flmRefP = 0;
  VoidHand		  flmCodeH = 0;
  FlmEntryFuncType* flmEntryFuncP = NULL;
  Boolean		  needReset = false;
  UInt16		  updateImageCRC16 = 0;
  UInt16		  storedImageCRC16 = 0;
  UInt16		  romCRC16 = 0;
  UInt32		  savedROMSignature = 0;
  UInt16		  moduleConfigScheme = prvModuleConfigSchemeDefault;
  FlmFlashConfigInfoType  flashSizeInfo;
  Word			  codeDbCardNo;
  LocalID		  codeDbID;
  UInt32		  flashOffset;
  DWord			  logicalBase;
  DWord			  physicalBase;
  void*			  cardBaseP;
  void*			  flashBaseP;
  void*			  flashP;
  void*			  handlerP;
  UInt8			  intEnable;
  UInt32		  romSize;
  Int16			  i;
  Int16			  lastImageRscId, imageRscCount, numFlashed;
  ProgrType		  progr;
  EraseProgrUpdateType	eraseProgrUpdate;
  FlmProgressReqType  eraseProgrReq;
  UInt32		  progrStart, progrInc;
  Boolean		  badInt;
  UInt32		  flashMgrOpenFlags;




  // --------------------------------------------------------------------------
  // Disable Alarms while we update the module -- we do this in order to prevent
  //  the alarm timer from issuing frequent interrupts in the event an alarm
  //  is triggered while we're flashing our module
  // --------------------------------------------------------------------------
  AlmEnableNotification (false/*enable*/);

  // --------------------------------------------------------------------------
  // Reserve the Memory Manager semaphore in read-access mode to prevent other
  //  tasks from accessing the handheld's overall filesystem and crashing the
  //  system while the module's filesystem is in transition
  // --------------------------------------------------------------------------
  MemSemaphoreReserve (false /*writeAccess*/);


  // Initialize progress structure
  progr.major.soFar = 0;
  progr.major.total = prvEraseProgrMax + prvWriteProgrMax + prvValidateROMMax;
  progr.minor.soFar = 0;
  progr.minor.total = 0;


  // Update progress
  progrInfoP->statusFunc (progrInfoP, NULL /*progressP*/, "Running Flash Updater");


  // --------------------------------------------------------------------------
  // Check how many cards there are
  // --------------------------------------------------------------------------
  if (!ModulePresent)
	  goto Exit;


  // --------------------------------------------------------------------------
  // Check the battery charge (informs the user if not sufficient)
  // --------------------------------------------------------------------------
  if (!CheckBatteryCharge ())
	  goto Exit;

  // --------------------------------------------------------------------------
  // Enumerate through all the ROM image resources to discover
  //  the total ROM image size and compute its CRC-16
  // --------------------------------------------------------------------------
  romSize = 0;
  i = prvFlashImageRscIdFirst;
  updateImageCRC16 = 0;	// init CRC-16 seed to 0
  while (true)
	{
	  VoidHand  rscH;
	  void*		p;
	  UInt32	chunkSize;

	  rscH = DmGetResource (prvFlashImageRscType, i);
	  if (!rscH)
		{
		  //err = DmGetLastErr ();
		  break;
		}

	  p = MemHandleLock (rscH);

	  chunkSize = MemPtrSize (p);
	  romSize += chunkSize;


	  updateImageCRC16 = PrvCrc16CalcBigBlock (p, chunkSize, updateImageCRC16);

	  MemPtrUnlock (p);

	  i++;
	}

  lastImageRscId = i - 1;
  imageRscCount = i - prvFlashImageRscIdFirst;


  // Check if we found any ROM image resources
  if (romSize == 0)
	{
	  FrmAlert (resAlertIDErrNoFlashImage);
	  goto Exit;
	}


  // Get the stored CRC-16 of the update ROM image and validate it against the
  //  computed one
  /*
  textH = DmGetResource (prvFlashImageCRC16RscType, prvFlashImageCRC16RscId);
  ErrFatalDisplayIf (!textH, "FlashImageCRC16Rsc is missing");
  TextToUInt16 ((Char*)MemHandleLock(textH), &storedImageCRC16, &badInt);
  MemHandleUnlock (textH);
  if (badInt)
	{
	  ErrFatalDisplay ("invalid FlashImageCRC16Rsc value");
	  goto Exit;
	}

  // If the stored CRC-16 of the image does not match the computed CRC-16,
  //  then the image must be corrupted
  if (updateImageCRC16 != storedImageCRC16)
	{
	  // Display the error alert and require the user to press the button on the
	  //  dialog before dismissing it
	  FrmAlert (resAlertIDErrUpdaterCorrupted);

	  // If the user ignored the error alert and pulled out
	  //  the module or attempted to switch to another app,
	  //  display a fatal error dialog instead to make sure
	  //  they get the message!
	  if (PrvProcessCurrentEvents() != 0)
		{
		  PrvFatalMessageID (resStrIDUpdaterCorruptedShortMsg);
		}

	  goto Exit;
	}
  */
  err = 0;


  // --------------------------------------------------------------------------
  // Discover card number and database id of the Flash Manager database;
  //  for now, it is in the same database as the app.
  // --------------------------------------------------------------------------
  err = SysCurAppDatabase(&codeDbCardNo, &codeDbID);
  ErrNonFatalDisplayIf (err != 0, "SysCurAppDatabase failed");


  // --------------------------------------------------------------------------
  // Get card info via Handspring Extensions
  // --------------------------------------------------------------------------
  err = HsCardAttrGet (ModuleCardNo, hsCardAttrLogicalBase, &logicalBase);
  ErrFatalDisplayIf (err != 0, "hsCardAttrLogicalBase failed");

  err = HsCardAttrGet (ModuleCardNo, hsCardAttrCsBase, &physicalBase);
  ErrFatalDisplayIf (err != 0, "hsCardAttrCsBase failed");


  cardBaseP = (Byte*)0 + logicalBase;
  flashBaseP = (Byte*)0 + physicalBase;
  flashOffset = physicalBase - logicalBase;


  // --------------------------------------------------------------------------
  // After this point, we will make some changes that will require a soft reset
  //  of the system to get things back to a stable state (we could spend more
  //  effort on better cleanup, but it just isn't worth it)
  // --------------------------------------------------------------------------
  needReset = true;


  // --------------------------------------------------------------------------
  // The FlashManager is about to start placing the flash chip(s) in special
  //  query/erase/program states -- prevent the module's event handlers and
  //  interrupt handler (if any) from executing to ensure that they will not
  //  attempt to access the module during these operations.
  // --------------------------------------------------------------------------

  handlerP = NULL;
  HsCardAttrSet (ModuleCardNo, hsCardAttrPwrHandler, &handlerP);

  handlerP = NULL;
  HsCardAttrSet (ModuleCardNo, hsCardAttrEvtHandler, &handlerP);

  intEnable = false;
  HsCardAttrSet (ModuleCardNo, hsCardAttrIntEnable, &intEnable);
  handlerP = NULL;
  HsCardAttrSet (ModuleCardNo, hsCardAttrIntHandler, &handlerP);


  // --------------------------------------------------------------------------
  // Find and lock FlashMgr code resource
  // --------------------------------------------------------------------------
  flmCodeH = DmGetResource (flmCodeResType, flmCodeResId);
  ErrFatalDisplayIf (!flmCodeH, "no FlashMgr code resource");

  flmEntryFuncP = MemHandleLock (flmCodeH);
  ErrFatalDisplayIf (!flmEntryFuncP, "Locking of FlashMgr code failed");


  // --------------------------------------------------------------------------
  // Call the FlashMgr entry function to get its interface
  // --------------------------------------------------------------------------
  err = flmEntryFuncP (flmEntrySelGetInterface, &flmApi,
							sizeof(flmApi));
  ErrFatalDisplayIf (err != 0, "flmEntrySelGetInterface failed");


  // --------------------------------------------------------------------------
  // "Open" the Flash Manager
  // --------------------------------------------------------------------------

  // Retrieve the Springboard configuration scheme setting
  textH = DmGetResource (strRsc, resOvrStrIDChipSelectAutoConfigScheme);
  ErrFatalDisplayIf (!textH, "ChipSelectAutoConfigScheme rsc is missing");
  TextToUInt16 ((Char*)MemHandleLock(textH), &moduleConfigScheme, &badInt);
  MemHandleUnlock (textH);
  if (badInt)
	{
	  // Shouldn't happen -- programmer error!
	  ErrFatalDisplay ("invalid ChipSelectAutoConfigScheme value format");
	  goto Exit;
	}

  // <chg 18-Dec-2000 JP> Added the flmOpenFlagAutoScan
  // <chg 2001-2-13 JMP> changed from flmOpenFlagAutoScan
  //	to flmOpenChipSelectAutoConfig
  flashMgrOpenFlags = flmOpenFlagReadWrite;

  if (moduleConfigScheme == prvModuleConfigSchemeDefault)
	{;} // INTENTIONALLY BLANK
  else if (moduleConfigScheme == prvModuleConfigSchemeAutoScan)
	// <chg 2001-2-13 JMP> changed from flmOpenFlagAutoScan
	//	to flmOpenChipSelectAutoConfig
	flashMgrOpenFlags |= flmOpenChipSelectAutoConfig;
  else
	ErrFatalDisplay ("invalid ChipSelectAutoConfigScheme value");

  err = flmApi.FlmOpen (&flmRefP, codeDbCardNo,
						codeDbID, cardBaseP,
						flashOffset, 0 /*readOnlySize*/,
						romSize,
						true/*maxRangeIsFlashSize*/,
						flashMgrOpenFlags);
  if (err != 0)
	{
	  //ErrNonFatalDisplay ("flmApi.FlmOpen failed");

	  if (err == flmErrNoMemory)
		FrmAlert (resAlertIDErrNotEnoughMemory);
	  else
		FrmAlert (resAlertIDErrUnsupportedFlashMemory);

	  goto Exit;
	}


  // Get the flash size information via the Flash Manager
  err = flmApi.FlmAttrGet (flmRefP, flmAttrFlashConfig,
									  &flashSizeInfo, sizeof(flashSizeInfo));
  ErrNonFatalDisplayIf (err != 0, "flmAttrFlashSize failed");


  // --------------------------------------------------------------------------
  // Erase the area to be flashed
  // --------------------------------------------------------------------------

  // Update progress
  //progrInfoP->statusFunc (progrInfoP, NULL /*progressP*/, "Erasing");

  eraseProgrUpdate.progrInfoP = progrInfoP;
  eraseProgrUpdate.progrP = &progr;
  eraseProgrReq.callbackP = EraseProgressCBFunc;
  eraseProgrReq.userRefP = &eraseProgrUpdate;

  err = flmApi.FlmErase (flmRefP, flashBaseP, romSize, NULL, &eraseProgrReq);
  ErrFatalDisplayIf (err, "FlmErase failed");

  // Update progress
  progr.major.soFar = prvEraseProgrMax;
  progrInfoP->statusFunc (progrInfoP, &progr, NULL);


  // --------------------------------------------------------------------------
  // Now write out the flash image resources
  // --------------------------------------------------------------------------

  flashP = flashBaseP;
  i = prvFlashImageRscIdFirst;
  numFlashed = 0;
  progrStart = progr.major.soFar;
  while (true)
	{
	  VoidHand  rscH;
	  void*		rscP;
	  UInt32	rscSize;
	  UInt16	offset = 0;

	  // Get the next image resource
	  rscH = DmGetResource (prvFlashImageRscType, i);
	  if (!rscH)
		{
		  // We have gone through all the update ROM image chunks -- now
		  //  flash the ROM signature that we skipped earlier (see comments
		  //  below)
		  if (i > prvFlashImageRscIdFirst)
			{
			  ErrNonFatalDisplayIf (savedROMSignature == 0, "bad saved ROM signature");
			  err = flmApi.FlmWrite (flmRefP, flashBaseP, &savedROMSignature,
									 sizeof (savedROMSignature));
			  ErrFatalDisplayIf (err, "FlmWrite failed");
			}

		  // ... and bail out of the flash loop
		  break;
		}


	  // Lock it and get its size
	  rscP = MemHandleLock (rscH);
	  ErrFatalDisplayIf (!rscP, "MemHandleLock failed");

	  rscSize = MemPtrSize (rscP);

	  // If this is the first chunk, save the ROM signature and adjust
	  //  offset to skip flashing the signature at this time -- we will flash
	  //  it after the rest of the image; this prevents the system from rejecting
	  //  the module should we crash while flashing the rest of the ROM; the
	  //  default state of the erased flash in the first 4 bytes will be 0xFFFFFFFF,
	  //  in this case, and the system will recognize it as a "hardware-only" module;
	  //  this will allow the user to attempt to reflash the module by using the
	  //  override method.
	  if (i == prvFlashImageRscIdFirst)
		{
		  ErrFatalDisplayIf (rscSize < sizeof (savedROMSignature), "1st RomI rsc is too small");
		  MemMove (&savedROMSignature, rscP, sizeof (savedROMSignature));
		  offset = sizeof (savedROMSignature);
		}
	  else
		{
		  offset = 0;
		}

	  // Write it out to flash
	  err = flmApi.FlmWrite (flmRefP, (UInt8*)flashP + offset,
							 (UInt8*)rscP + offset, rscSize - offset);
	  ErrFatalDisplayIf (err, "FlmWrite failed");

	  // Unlock and release the resource
	  MemPtrUnlock (rscP);
	  DmReleaseResource (rscH);

	  // Get ready for next write operation
	  flashP = (Byte*)flashP + rscSize;
	  i++;
	  numFlashed++;

	  // Update progress
	  progrInc = (numFlashed * prvWriteProgrMax) / imageRscCount;
	  progr.major.soFar = progrStart + progrInc;
	  progrInfoP->statusFunc (progrInfoP, &progr, NULL);
	}



  // --------------------------------------------------------------------------
  // Now verify the flashed image
  // --------------------------------------------------------------------------

  // Compute the CRC-16 of the ROM image in flash
  romCRC16 = PrvCrc16CalcBigBlock (flashBaseP, romSize, 0 /*crcSeed*/);

  /*
  // If the stored CRC-16 of the image does not match the computed CRC-16 of the ROM,
  //  then the ROM must be corrupted
  if (romCRC16 != storedImageCRC16)
	{
	  // Display the error alert and require the user to press the button on the
	  //  dialog before dismissing it
	  FrmAlert (resAlertIDErrFlashCRCFailed);
	  // If the user ignored the error alert and pulled out
	  //  the module or attempted to switch to another app,
	  //  display a fatal error dialog instead to make sure
	  //  they get the message!
	  if (PrvProcessCurrentEvents() != 0)
		{
		  PrvFatalMessageID (resStrIDFlashCRCFailedShortMsg);
		}


	  goto Exit;
	}

  */
  // Update the progress bar -- we split the CRC check and the raw comparison
  //  that follows roughly 50/50
  progr.major.soFar += prvValidateROMMax / 2;
  progrInfoP->statusFunc (progrInfoP, &progr, NULL);


  // Compare the ROM to the update
  flashP = flashBaseP;
  i = prvFlashImageRscIdFirst;
  while (true)
	{
	  VoidHand  rscH;
	  void*		rscP;
	  UInt32	rscSize;
	  UInt8*	fP;
	  UInt8*	fendP;
	  UInt8*	rP;
	  Boolean	failed = false;

	  // Get the next image resource
	  rscH = DmGetResource (prvFlashImageRscType, i);
	  if (!rscH)
		{
		  break;
		}


	  // Lock it and get its size
	  rscP = MemHandleLock (rscH);
	  ErrFatalDisplayIf (!rscP, "MemHandleLock failed");

	  rscSize = MemPtrSize (rscP);

	  // Compare it with flash
	  fP = flashP;
	  rP = rscP;
	  fendP = fP + rscSize;
	  for (/*empty*/ ; fP < fendP; fP++, rP++)
		{
		  if (*fP != *rP)
			{
			  Char	  text[32];

			  // Verification failed -- inform the user and bail out

			  failed = true;	// so we'll bail out after unlocking resource

			  StrPrintF (text, "0x%lX", (long) fP);

			  // Display the alert and require the user to press the button on the
			  //  dialog before dismissing it
			  FrmCustomAlert (resAlertIDErrFlashVerifyFailed, text, NULL, NULL);
			  // If the user ignored the error alert and pulled out
			  //  the module or attempted to switch to another app,
			  //  display a fatal error dialog instead to make sure
			  //  they get the message!
			  if (PrvProcessCurrentEvents() != 0)
				{
				  PrvFatalMessageID (resStrIDFlashCRCFailedShortMsg);
				}

			  break;
			}
		}



	  // Unlock and release the resource
	  MemPtrUnlock (rscP);
	  DmReleaseResource (rscH);

	  // Bail out if we failed
	  if (failed)
		goto Exit;

	  // Get ready for the next comparison
	  flashP = (Byte*)flashP + rscSize;
	  i++;
	}



  // Update progress to 100%
  progr.major.soFar = progr.major.total;
  progrInfoP->statusFunc (progrInfoP, &progr, NULL /*"Update is complete"*/);


  // Delay a short while so the user gets to see the progress at 100%;
  //  otherwise, the progress gauge looks incomplete at end of
  //  operation
  SysTaskDelay (SysTicksPerSecond() / 8);


  // Inform the user of update completion and pending reset
  textH = DmGetResource (strRsc, resOvrStrIDResetAlertMsg);
  ErrFatalDisplayIf (!textH, "resOvrStrResetAlertMsg resource is missing");
  FrmCustomAlert (resAlertIDInfoFlashComplete, MemHandleLock(textH),
							  NULL, NULL);
  MemHandleUnlock (textH);



Exit:
  // And reset the device for a clean start -- this call doesn't return
  if (needReset)
	SysReset ();

  if (err)
	{
	  Char	  text[32];
	  StrPrintF (text, "0x%lX", (long) err);

	  FrmCustomAlert (resAlertIDErrInitFlashFailed, text, NULL, NULL);
	}

  // Close the Flash Manager
  if (flmRefP)
	  flmApi.FlmClose (flmRefP);

  // Unlock the Flash Manager code resource
  if (flmEntryFuncP)
	  MemPtrUnlock (flmEntryFuncP);

  // Release the Flash Manager code resource
  if (flmCodeH)
	  DmReleaseResource (flmCodeH);

  // --------------------------------------------------------------------------
  // Release the Memory Manager semaphore
  // --------------------------------------------------------------------------
  MemSemaphoreRelease (false /*writeAccess*/);

  // --------------------------------------------------------------------------
  // Enable Alarms
  // --------------------------------------------------------------------------
  AlmEnableNotification (true/*enable*/);

  return;

} // UpdateFlashCard


#if 0

/***************************************************************
 *	Function:	DumpField
 *
 *	Summary:
 *		Writes a string and number to the display.
 *
 *	Parameters:
 *		???
 *
 *	Returns:
 *		void
 *
 *	Called By:
 *		internal;
 *
 *	Notes:
 *
 *
 *	History:
 *    14-May-1999	VMK		Created
 ****************************************************************/
static void
DumpField (Char* strP, UInt32 num, Boolean decFmt, Word* yP)
{
  Short			strWidth;
  UInt			strLen;
  Char			numText[32];
  FontID			oldFontId;
  Word			numChars;
  const Word		fieldHeight = 10;

  strLen = StrLen (strP);
  //strWidth = FntCharsWidth (strP, strLen);
  strWidth = 80;

  // Convert the number to text (decimal or hex)
  if (decFmt)
	{
	  numText[0] = '=';
	  numText[1] = ' ';
	  numText[2] = '#';
	  StrIToA(numText+3, (Long)num);
	}
  else
	{
	  numText[0] = '=';
	  numText[1] = ' ';
	  numText[2] = '$';
	  StrIToH (numText+3, num);
	}

  numChars = StrLen (numText);

  // Output the text
  oldFontId = FntSetFont (stdFont);				  // save font								// set font
  WinDrawChars (strP, strLen, 0/*x*/, *yP);		  // output string
  WinDrawChars (numText, numChars, strWidth, *yP);  // output number
  FntSetFont (oldFontId);							  // restore font													// restore old font

  // Update the vertical position accumulator
  (*yP) += fieldHeight;


  return;

} // DumpField

#endif	// CASTRATION
