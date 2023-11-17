/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateLunar.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	This file contains the (temporary) lunar calendar support for the
 *	Datebook application.  These routines will be part of Time Manager 6.0.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <Chars.h>
#include <DateTime.h>
#include <DataMgr.h>					// For DmGetResource
#include <ErrorMgr.h>
#include <MemoryMgr.h>					// For memErrInvalidParam
#include <string.h>
#include <StringMgr.h>					// For StrNCopy, StrIToA, etc
#include <TextMgr.h>					// For TxtReplaceStr
#include <SysUtils.h>					// For SysStringByIndex

#include <UIResources.h>				// For strListRscType/ ResLoadConstant

#include "DateLunarFonts.h"
#include "DateLunar.h"

// Drop this in here so that the table processing code can be more easily
// ported to Palm OS 6.1.

#ifndef BUS_ALIGN

#define  BUS_ALIGN_16       		16    			// 16-bit data must be read/written at 16-bit address
#define  BUS_ALIGN_32				32     			// 32-bit data must be read/written at 32-bit address
#define  BUS_ALIGN 					BUS_ALIGN_32	// defined alignment to be used

#endif


/***********************************************************************
 *
 *	Private constants
 *
 ***********************************************************************/

// These bitmaps are used to draw squeezed versions of the 11th and 12th "lunar
// month beginning" strings
#define lunarMonth11BitmapID				1700
#define lunarMonth12BitmapID				1701


// Resource types and IDs
#define kLunarCalendarSupportedID	10200
#define kMiscStrListID				10208
#define kLunarStemsStrListID		10209
#define kLunarBranchesStrListID		10210
#define kLunarAnimalsStrListID		10211
#define kLunarMonthsStrListID		10212
#define kLunarDaysStrListID			10213
#define kLunarCalendarRscType		'luca'		// Lunar calendar data
#define kChineseLunarCalendarID		19000		// Chinese lunar calendar data


// Max length of any date template string (before expansion), excluding the null.
#define	maxDateTemplateLen			31

// Maximum substring lengths for building Chinese lunar date string
#define	kMaxLunarStemStrLen			15
#define	kMaxLunarBranchStrLen		15
#define	kMaxLunarAnimalStrLen		15
#define	kMaxLunarMonthStrLen		15
#define	kMaxLunarDayStrLen			15
#define	kMaxLunarLeapMonthStrLen	15


// Lunar Calendar Version
const uint16_t	cLunarCalendarDataVersion		= 1;


// Miscellaneous string list resource (kMiscStrListID) indexes
const uint16_t cLunarTemplateMiscStrListIndex	= 0;
const uint16_t cLunarLeapMonthMiscStrListIndex	= 1;


// Substring parameter indexes for building Chinese lunar date string
const uint16_t cLunarStemParamIndex				= 0;
const uint16_t cLunarBranchParamIndex			= 1;
const uint16_t cLunarAnimalParamIndex			= 2;
const uint16_t cLunarMonthParamIndex			= 3;
const uint16_t cLunarDayParamIndex				= 4;
const uint16_t cLunarLeapMonthParamIndex		= 5;


/***********************************************************************
 *
 *	Private macros
 *
 ***********************************************************************/

/*
	Convert <inputValue> from 68K format to ARM format (or vice-versa)
	and put the result in <outputValue>.

WARNING!	The <inputValue> and <outputValue> arguments are evaluated more
			than once, and cannot refer to the same area of memory.
*/

#define	SwapBytes_(inputValue, outputValue)									\
	switch (sizeof(outputValue)) 											\
	{																		\
		case 1:																\
			outputValue = inputValue;										\
		break;																\
		case 2:																\
			((uint8_t*)&outputValue)[0] = ((uint8_t*)(&inputValue))[1];		\
			((uint8_t*)&outputValue)[1] = ((uint8_t*)(&inputValue))[0];		\
		break;																\
		case 4:																\
			((uint8_t*)&outputValue)[0] = ((uint8_t*)(&inputValue))[3];		\
			((uint8_t*)&outputValue)[1] = ((uint8_t*)(&inputValue))[2];		\
			((uint8_t*)&outputValue)[2] = ((uint8_t*)(&inputValue))[1];		\
			((uint8_t*)&outputValue)[3] = ((uint8_t*)(&inputValue))[0];		\
		break;																\
	}


/***********************************************************************
 *
 *	Private types
 *
 ***********************************************************************/

//Description of a single month in the Chinese lunar calendar

typedef struct _LunarMonthTag {
	uint8_t		monthNumber;		// 1..13 (0 if unused)
	uint8_t		daysInThisMonth;	// 29..30 (0 if unused)
} LunarMonthType;


// Description of a single year in the Chinese solar/lunar calendar

typedef struct _LunarYearTag {
	int32_t				yearStartDay;							// Offset from 1904-1-1, -337..46409 (1903-1-29..2031-1-23)
	LunarMonthType		lunarMonths[timMaxChineseLunarMonths];	// Month info array (last might not be used)
} LunarYearType;


// Database of lunar calendar information
// The complete database is (2+4+(13*2))*128=4096 bytes.

typedef struct _LunarCalendarDataTag {
	uint16_t			version;
	uint16_t			lunarYearOfYearIndex0;	// Position of lunarYear[0] in sexigesimal cycle 1..60
	uint16_t			numKnownYears;			// # elements in lunarYears
	LunarYearType		lunarYears[1];			// Variable length record
}LunarCalendarDataType;


/***********************************************************************
 *
 *	Private Globale Variables
 *
 ***********************************************************************/
static LunarCalendarDataType*	pLunarCalendarP = NULL;
static Boolean					systemSupportsLunarCalendar = false;

static MemHandle				sLunarMonth11BitmapResH = NULL;
static MemHandle				sLunarMonth12BitmapResH = NULL;

static BitmapType*				sLunarMonth11BitmapP = NULL;
static BitmapType*				sLunarMonth12BitmapP = NULL;

/***********************************************************************
 *
 *	Private Functions
 *
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:    	PrvStringByIndex
 *
 * DESCRIPTION: 	Place the <iStringIndex>th string from the tSTL
 *					resource <stringListID> (including the prefix string)
 *					into <oResultStr>, and return the result.
 *
 *					Copy at most <iMaxSize> bytes of it
 *  				(including the chrNull).
 *
 * PARAMETERS:  ->	iResourceDbP:	database open ref for resources.
 *				->	iStringListID:	ID of string list to load
 *				->	iStringIndex:	Index of string to retrieve.
 *				<->	oResultStr:		Buffer wher to store  the string.
 *				->	iMaxSize:		Max buffer size.
 *
 * RETURNED:    	same as SysStringByIndex(). wanted string.
 *					pointer to oResultString
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
static char* PrvStringByIndex(
	DmOpenRef				iResourceDbP,
	uint16_t				iStringListID,
	uint16_t				iStringIndex,
	char*					oResultStr,
	uint16_t				iMaxSize)

{
	return SysStringByIndex(iResourceDbP, iStringListID, iStringIndex, oResultStr, iMaxSize);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvGetLunarCalendarP
 *
 * DESCRIPTION: 	Return a pointer to the locked lunar calendar
 *					data resource.
 *
 * PARAMETERS:  ->	iResourceDbP:	Database open ref for resources.
 *
 * RETURNED:    	Return a pointer to the locked lunar calendar
 *					data resource. (LunarCalendarDataType*)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
static LunarCalendarDataType* PrvGetLunarCalendarP(DmOpenRef iResourceDbP)
{
	MemHandle	lunarCalendarH = NULL;
	uint16_t	version;

	if (!pLunarCalendarP)
	{
		lunarCalendarH = DmGetResource(	iResourceDbP, kLunarCalendarRscType, kChineseLunarCalendarID);
		if (lunarCalendarH)
		{
			pLunarCalendarP = MemHandleLock(lunarCalendarH);
#if BUS_ALIGN == BUS_ALIGN_32
			version = pLunarCalendarP->version;
#else
			SwapBytes_(pLunarCalendarP->version, version);
#endif
			if (version != cLunarCalendarDataVersion)
			{
				return NULL;
			}
		}
	}

	return pLunarCalendarP;
}


/***********************************************************************
 *
 * FUNCTION:		PrvFindLunarYearIndex
 *
 * DESCRIPTION: 	Return the index into
 *						<iLunarCalendarDataP>->lunarYears
 *					of the lunar year in which
 *						<iDayCount> (since 1904-1-1)
 *					appears, where
 *						*<iLunarCalendarDataP>
 *					contains a description of all supported
 *					lunar years.
 *
 * PARAMETERS:	->	iLunarCalendarDataP: 	see above.
 *				->  iDayCount: 				see above.
 *
 * RETURNED:		The index found.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
static uint16_t PrvFindLunarYearIndex(
	const LunarCalendarDataType*	iLunarCalendarDataP,
	int32_t							iDayCount)

{
	uint16_t						lunarYearIndex;
	uint16_t						numKnownYears;
	int32_t							yearStartDay;
	uint16_t						monthIndex;
	uint16_t						daysInLastYear = 0;


#if BUS_ALIGN == BUS_ALIGN_32
	numKnownYears = pLunarCalendarP->numKnownYears;
#else
	SwapBytes_(pLunarCalendarP->numKnownYears, numKnownYears);
#endif

	for( lunarYearIndex = numKnownYears-1 ; lunarYearIndex >= 0 ; lunarYearIndex--)
	{

#if BUS_ALIGN == BUS_ALIGN_32
		yearStartDay = iLunarCalendarDataP->lunarYears[lunarYearIndex].yearStartDay;
#else
		SwapBytes_(	iLunarCalendarDataP->lunarYears[lunarYearIndex].yearStartDay, yearStartDay);
#endif
		if	(iDayCount >= yearStartDay)
		{

			// If date is beyond beginning of last year in data, then we need to
			// make sure it's not beyond the end of the last year.

			if (lunarYearIndex == (numKnownYears-1))
			{
				for( monthIndex = 0 ; monthIndex < timMaxChineseLunarMonths ; monthIndex++)
				{
					daysInLastYear += iLunarCalendarDataP->lunarYears[lunarYearIndex].lunarMonths[monthIndex].daysInThisMonth;
				}

				if (iDayCount >= (yearStartDay + daysInLastYear))
				{
					break;
				}
			}
			return lunarYearIndex;
		}
	}

	ErrNonFatalDisplay("Unable to find lunar year for input date.");

	return 0 ;
}


/***********************************************************************
 *
 *	Public Functions
 *
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:		DateSecondsToLunarDate
 *
 * DESCRIPTION: 	Converts
 *						<iSeconds> (since 1/1/1904)
 *					To the Chinese
 *						*<oLunarYear>, <oLunarMonth> and *<oLunarDay>.
 *					Set
 *						*<oIsLeapMonth>
 *					To true if this is the second month in this year with
 *						*<oLunarMonth>.
 *
 * PARAMETERS:	->	iResourceDbP:	database open ref for resources.
 *				->	iSeconds:		see above.
 *				<-	oLunarYear:		see above.
 *				<-	oLunarMonth:	see above.
 *				<-	oLunarDay:		see above.
 *				<-	oIsLeapMonth:	see above.
 *
 * RETURNED:		Return a pointer to the locked lunar calendar
 *					data resource. (LunarCalendarDataType*)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
status_t DateSecondsToLunarDate(
	DmOpenRef				iResourceDbP,
	uint32_t				iSeconds,
	uint16_t*				oLunarYear,
	uint16_t*				oLunarMonth,
	uint16_t*				oLunarDay,
	Boolean*				oIsLeapMonth)

{
	LunarCalendarDataType*	chineseLunarCalendarP = NULL;
	int32_t 				dayCount;
	LunarYearType*			lunarYearP = NULL;
	uint16_t				lunarYearIndex;
	uint16_t				lunarMonthIndex;
	int32_t 				yearStartDay;
	uint16_t				lunarYearOfYearIndex0;

	// Initialize our outputs in case we're unsuccessful.

	*oLunarYear = 1;
	*oLunarMonth = 1;
	*oLunarDay = 1;
	*oIsLeapMonth = false;

	if (!DateSupportsLunarCalendar())
		return(timErrNoLunarCalendarSupport);

	// Convert the input seconds to days since 1904

	dayCount = iSeconds / daysInSeconds;

	// Grab the lunar calendar resource

	chineseLunarCalendarP = PrvGetLunarCalendarP(iResourceDbP);
	if (!chineseLunarCalendarP)
	{
		ErrNonFatalDisplay("Unable to load lunar calendar data resource.");
		return(timErrNoLunarCalendarSupport);
	}

	// Find the year record that corresponds to this date, and calculate its
	// position in the sexigesimal cycle.

#if BUS_ALIGN == BUS_ALIGN_32
	lunarYearOfYearIndex0 = chineseLunarCalendarP->lunarYearOfYearIndex0;
#else
	SwapBytes_(chineseLunarCalendarP->lunarYearOfYearIndex0, lunarYearOfYearIndex0);
#endif
	lunarYearIndex = PrvFindLunarYearIndex(chineseLunarCalendarP, dayCount);
	lunarYearP =	( chineseLunarCalendarP->lunarYears +	lunarYearIndex);

	*oLunarYear =	(	(
							(lunarYearIndex + lunarYearOfYearIndex0 - timFirstChineseLunarYear)
							%
							(timLastChineseLunarYear - timFirstChineseLunarYear + 1)
						)
						+	timFirstChineseLunarYear
					);

	// We now know the year, and can calculate day number within it.


#if BUS_ALIGN == BUS_ALIGN_32
	yearStartDay = lunarYearP->yearStartDay;
#else
	SwapBytes_(lunarYearP->yearStartDay, yearStartDay);
#endif

	*oLunarDay = (uint16_t)(dayCount - yearStartDay);

	// Search for the correct lunar month, and calculate the lunar day.

	for (	lunarMonthIndex = 0;
			*oLunarDay >= lunarYearP->lunarMonths[lunarMonthIndex].daysInThisMonth;
			lunarMonthIndex++)
	{
		if (	(lunarMonthIndex >= timMaxChineseLunarMonths)
			||	(lunarYearP->lunarMonths[lunarMonthIndex].monthNumber == 0)
			||	(lunarYearP->lunarMonths[lunarMonthIndex].daysInThisMonth == 0))
		{
			ErrNonFatalDisplay("Unable to find correct lunar month in lunar year.");
			return(timErrNoLunarCalendarSupport);
		}
		(*oLunarDay) -= lunarYearP->lunarMonths[lunarMonthIndex].daysInThisMonth;
	}

	(*oLunarDay)++; // First day in month is returned as 1 (not 0)

	// Return its month number and whether it's a leap month

	*oLunarMonth = lunarYearP->lunarMonths[lunarMonthIndex].monthNumber;
	*oIsLeapMonth = (	(lunarMonthIndex > 0)
						&&	(	lunarYearP->lunarMonths[lunarMonthIndex-1].monthNumber
								==	lunarYearP->lunarMonths[lunarMonthIndex].monthNumber));

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:		DateToLunarStr
 *
 * DESCRIPTION: 	Converts the Chinese
 *						<iLunarYear>
 *						<iLunarMonth>
 *						<iLunarDay>
 *					and
 *						<iIsLeapMonth>
 *					into a Chinese string describing this lunar date,
 *					and place the result in
 *						*<oLunarDateStr>,
 *					which can hold
 *						<iMaxStrLen>
 *					bytes (excluding the null).
 *
 *					timOmitLunarYear, timOmitLunarMonth,and timOmitLunarDay
 *					may be passed in <iLunarYear>, <iLunarMonth> and
 *					<iLunarDay> (respectively) in order to omit
 *					these portions of the string.
 *
 * PARAMETERS:	->	iResourceDbP:	Database open ref for resources.
 *				->	iLunarYear:		See above.
 *				->	iLunarMonth:	See above.
 *				->	iLunarDay:		See above.
 *				->	iIsLeapMonth:	See above.
 *				<-	oLunarDateStr:	See above.
 *				->	iMaxStrLen:		See above.
 *
 * RETURNED:		an error code.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *					First released in Palm OS 5.4
 *  CS	10/23/02	Added support for timOmitLunarYear, timOmitLunarMonth,
 *					and timOmitLunarDay.
 *	CS 	01/17/03	Build output in separate buffer in case caller's
 *					't long enough for temporary results (i.e., caller
 *					is omitting one or more fields, but we haven't yet
 *					removed the replacement tokens from the template).
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
status_t DateToLunarStr(
	DmOpenRef			iResourceDbP,
	uint16_t			iLunarYear,
	uint16_t			iLunarMonth,
	uint16_t			iLunarDay,
	Boolean 			iIsLeapMonth,
	char*				oLunarDateStr,
	uint16_t			iMaxStrLen)
{
	char				stemStr[kMaxLunarStemStrLen+1];
	char				branchStr[kMaxLunarBranchStrLen+1];
	char				animalStr[kMaxLunarAnimalStrLen+1];
	char				monthStr[kMaxLunarMonthStrLen+1];
	char				dayStr[kMaxLunarDayStrLen+1];
	char				leapMonthStr[kMaxLunarLeapMonthStrLen+1];
	char				lunarDateStr[timMaxLunarDateStrLen+1];
	uint16_t			truncatedLen;

	stemStr[0]			= 0;
	branchStr[0]		= 0;
	animalStr[0]		= 0;
	monthStr[0] 		= 0;
	dayStr[0]			= 0;
	leapMonthStr[0] 	= 0;

	if (!DateSupportsLunarCalendar())
		return(timErrNoLunarCalendarSupport);

	// Load the template string and then jam it with the year stem & branch,
	// animal, month, optional leap month text, and day.

	PrvStringByIndex(	iResourceDbP,
						kMiscStrListID,
						cLunarTemplateMiscStrListIndex,
						lunarDateStr,
						timMaxLunarDateStrLen+1);

	if (strlen(lunarDateStr) == 0)
	{
		ErrNonFatalDisplay("Unable to load lunar date template.");
		return(timErrNoLunarCalendarSupport);
	}

	if (iLunarYear != timOmitLunarYear)
	{
		PrvStringByIndex(	iResourceDbP,
							kLunarStemsStrListID,
							(uint16_t)	(	(iLunarYear-timFirstChineseLunarYear)
											%	timNumChineseStems),
							stemStr,
							kMaxLunarStemStrLen+1);

		if (strlen(stemStr) == 0)
		{
			ErrNonFatalDisplay("Unable to load lunar stem string.");
			return(timErrNoLunarCalendarSupport);
		}

		PrvStringByIndex(	iResourceDbP,
							kLunarBranchesStrListID,
							(uint16_t)	(	(iLunarYear-timFirstChineseLunarYear)
											%	timNumChineseBranches),
							branchStr,
							kMaxLunarBranchStrLen+1);

		if (strlen(branchStr) == 0)
		{
			ErrNonFatalDisplay("Unable to load lunar branch string.");
			return (timErrNoLunarCalendarSupport);
		}

		PrvStringByIndex(	iResourceDbP,
							kLunarAnimalsStrListID,
							(uint16_t)	(	(iLunarYear-timFirstChineseLunarYear)
										%	timNumChineseBranches),
							animalStr,
							kMaxLunarAnimalStrLen+1);

		if (strlen(animalStr) == 0)
		{
			ErrNonFatalDisplay("Unable to load lunar year animal string.");
			return(timErrNoLunarCalendarSupport);
		}
	}

	TxtReplaceStr(	lunarDateStr,
					timMaxLunarDateStrLen,
					stemStr,
					cLunarStemParamIndex);

	TxtReplaceStr(	lunarDateStr,
					timMaxLunarDateStrLen,
					branchStr,
					cLunarBranchParamIndex);

	TxtReplaceStr(	lunarDateStr,
					timMaxLunarDateStrLen,
					animalStr,
					cLunarAnimalParamIndex);

	if (iLunarMonth != timOmitLunarMonth)
	{
		PrvStringByIndex(	iResourceDbP,
							kLunarMonthsStrListID,
							(uint16_t)(iLunarMonth-timFirstChineseLunarMonth),
							monthStr,
							kMaxLunarMonthStrLen+1);

		if (strlen(monthStr) == 0)
		{
			ErrNonFatalDisplay("Unable to load lunar month string.");
			return(timErrNoLunarCalendarSupport);
		}
	}

	TxtReplaceStr(	lunarDateStr,
					timMaxLunarDateStrLen,
					monthStr,
					cLunarMonthParamIndex);

	if (iLunarDay != timOmitLunarDay)
	{
		PrvStringByIndex(	iResourceDbP,
							kLunarDaysStrListID,
							(uint16_t)(iLunarDay-timFirstChineseLunarDay),
							dayStr,
							kMaxLunarDayStrLen+1);

		if (strlen(dayStr) == 0)
		{
			ErrNonFatalDisplay("Unable to load lunar day string.");
			return(timErrNoLunarCalendarSupport);
		}
	}

	TxtReplaceStr(	lunarDateStr,
					timMaxLunarDateStrLen,
					dayStr,
					cLunarDayParamIndex);

	if	(	(iLunarMonth != timOmitLunarMonth)
		&&	(iIsLeapMonth))
	{
		PrvStringByIndex(	iResourceDbP,
							kMiscStrListID,
							cLunarLeapMonthMiscStrListIndex,
							leapMonthStr,
							kMaxLunarLeapMonthStrLen+1);

		if (strlen(leapMonthStr) == 0)
		{
			ErrNonFatalDisplay("Unable to load lunar leap month string.");
			return(timErrNoLunarCalendarSupport);
		}
	}

	TxtReplaceStr(	lunarDateStr,
					timMaxLunarDateStrLen,
					leapMonthStr,
					cLunarLeapMonthParamIndex);

	// Now that we've built the result, copy it to the caller's buffer.
	// DOLATER CS - Return an error if result doesn't fit?

	if (strlen(lunarDateStr) <= iMaxStrLen)
	{
		strcpy(oLunarDateStr, lunarDateStr);
	}
	else
	{
		truncatedLen = TxtGetTruncationOffset(lunarDateStr, iMaxStrLen);

		MemMove(oLunarDateStr, lunarDateStr, truncatedLen);

		oLunarDateStr[truncatedLen] = chrNull;

		ErrNonFatalDisplay("Lunar date string overflows buffer.");
	}

	return(errNone);

}


/***********************************************************************
 *
 * FUNCTION:		SelectLunarDayIsSupported
 *
 * DESCRIPTION: 	Return true if there is a Chinese lunar calendar
 *					application installed
 *					(used by the DateBook application).
 *
 * PARAMETERS:	->	iResourceDbP:	Database open ref for resources.
 *
 * RETURNED:		true or false.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
Boolean SelectLunarDayIsSupported(DmOpenRef resourceDbP)
{
	// DOLATER CS - Implement this routine.

	return false;
}

/***********************************************************************
 *
 * FUNCTION:		LunarCalendarGetChineseLargeMonthBitmap
 *
 * DESCRIPTION: 	Returns bitmap for large month number taht holds
 *					Chinese char.
 *
 * PARAMETERS:	->	number: number to get bitmap on.
 *
 * RETURNED:		BitmapType*
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
BitmapType*  LunarCalendarGetChineseLargeMonthBitmap(uint16_t number)
{
	BitmapType* dayNumBitmapP = NULL;

	switch(number)
	{
		case 11:
			dayNumBitmapP = sLunarMonth11BitmapP;
			break;
		case 12:
			dayNumBitmapP = sLunarMonth12BitmapP;
			break;
		default:;
	}

	return dayNumBitmapP;
}




/***********************************************************************
 *
 * FUNCTION:		TerminateDateLunar
 *
 * DESCRIPTION: 	Return true if there is a Chinese lunar calendar
 *					application installed
 *					(used by the DateBook application).
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
void TerminateDateLunar(DmOpenRef iResourceDbP)
{
	if (pLunarCalendarP)
	{
		MemPtrUnlock(pLunarCalendarP);
	}
	pLunarCalendarP = NULL;

	if (sLunarMonth11BitmapResH)
	{
		if (sLunarMonth11BitmapP)
			MemHandleUnlock(sLunarMonth11BitmapResH);

		DmReleaseResource(sLunarMonth11BitmapResH);

		sLunarMonth11BitmapResH = NULL;
		sLunarMonth11BitmapP = NULL;
	}


	if (sLunarMonth12BitmapResH)
	{
		if (sLunarMonth12BitmapP)
			MemHandleUnlock(sLunarMonth12BitmapResH);

		DmReleaseResource(sLunarMonth12BitmapResH);

		sLunarMonth12BitmapResH = NULL;
		sLunarMonth12BitmapP = NULL;
	}
}


/***********************************************************************
 *
 * FUNCTION:		TerminateDateLunar
 *
 * DESCRIPTION: 	Return true if the system locale supports
 *					the Chinese lunar calendar.
 *
 * PARAMETERS:	->	iResourceDbP:	Database open ref for resources.
 *
 * RETURNED:		true if supported, false if not supported
 *
 * NOTE:			Currently, it is based on a global boolean
 *					systemSupportsLunarCalendar initilized by
 *					LunarCalendarInit() call.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
Boolean DateSupportsLunarCalendar(void)
{
	return systemSupportsLunarCalendar;
}


/***********************************************************************
 *
 * FUNCTION:		LunarCalendarInit
 *
 * DESCRIPTION: 	Initialize LunarCalendar module.
 *					Loads the 'luca' resource ID 19000
 *					that tells Datebook if Lunar Calendar is around.
 *
 * PARAMETERS:	->	iResourceDbP:	Database open ref for resources.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			Currently, the luca resource is stored as a
 *					simplified Chinese (zhCN) only resource.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
void LunarCalendarInit(DmOpenRef iResourceDbP)
{
	systemSupportsLunarCalendar = (Boolean) ResLoadConstant(iResourceDbP, kLunarCalendarSupportedID);

	if (systemSupportsLunarCalendar)
	{
		sLunarMonth11BitmapResH = DmGetResource(	iResourceDbP, bitmapRsc, lunarMonth11BitmapID);
		ErrNonFatalDisplayIf(!sLunarMonth11BitmapResH, "Unable to load lunar month bitmap");

		if (sLunarMonth11BitmapResH)
			sLunarMonth11BitmapP = (BitmapType*) MemHandleLock(sLunarMonth11BitmapResH);

		sLunarMonth12BitmapResH = DmGetResource(	iResourceDbP, bitmapRsc, lunarMonth12BitmapID);
		ErrNonFatalDisplayIf(!sLunarMonth12BitmapResH, "Unable to load lunar month bitmap");

		if (sLunarMonth12BitmapResH)
			sLunarMonth12BitmapP = (BitmapType*) MemHandleLock(sLunarMonth12BitmapResH);
	}

	LoadLunarViewFonts(iResourceDbP);
}


/***********************************************************************
 *
 * FUNCTION:		LunarCalendarClose
 *
 * DESCRIPTION: 	Closes LunarCalendar module.
 *
 * PARAMETERS:	->	iResourceDbP:	Database open ref for resources.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *					and port for Palm OS 6.0
 *
 ***********************************************************************/
void LunarCalendarClose(DmOpenRef iResourceDbP)
{
	ReleaseLunarViewFonts();

	TerminateDateLunar(iResourceDbP);
}
