/******************************************************************************
 *
 * Copyright (c) 1996-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateMonth.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This is the Datebook application's month view module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>

#include <AboutBox.h>
#include <AppMgr.h>
#include <CatMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <FeatureMgr.h>
#include <PenInputMgr.h>
#include <PrivateRecords.h>
#include <Preferences.h>
#include <SelDay.h>
#include <SchemaDatabases.h>
#include <string.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <SystemResources.h>
#include <Table.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <UIColor.h>
#include <Loader.h>
#include <string.h>

#include "DateGlobals.h"
#include "DateLunar.h"
#include "DateLunarFonts.h"
#include "DateMonth.h"
#include "DateUtils.h"
#include "Datebook.h"
#include "DatebookRsc.h"
#include "DateBackground.h"

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define linesInMonth						6
#define maxWeeksInMonth						6

// Day number's margins
#define textLeftMargin						3
#define textTopMargin						1

// Day number's margins
// --> solar calendar (Gregorian)
#define dateTextLeftMarginMonthView			5
#define dateTextTopMarginMonthView			4

// --> lunar calendar
#define dateTextLeftMarginLunarView			2
#define dateTextTopMarginLunarView			1


// Event indicators
#define lunchStartTime						0x0b1e		// 11:30 am
#define lunchMidTime						0x0c0f		// 12:15 pm
#define lunchEndTime						0xd00		//  1:00 pm

#define timeIndicatorWidth					2
#define timeIndicatorTopMargin				3
#define timeIndicatorRightMargin			2

#define untimeIndicatorTopMargin			13
#define untimeIndicatorRightMargin			5

#define repeatIndicatorBottomMargin			2


#define morningHeight						4
#define lunchHeight							3
#define afternoonHeight						4

#define drawDaySelected						true
#define dontDrawDaySelected					false

#define TodayIsVisible(monthP,today) ((monthP->month == today.month) && (monthP->year == today.year))

#define maxDaysInMonth						32
#define maxDayCellsInMonthGrid				42


#define kDrawNoIndicators					0
#define kDrawRepeatingFirstIndicator		1
#define kDrawRepeatingBetweenIndicator		2
#define kDrawRepeatingLastIndicator			4
#define kDrawNoTimeIndicator				8
#define kDrawMorningIndicator				16
#define kDrawLunchIndicator					32
#define kDrawAfternoonIndicator				64


#define kFocusMonthNoFocus					(noFocus)
#define kFocusMonthFocused 					1
#define kFocusMonthCellFocused				2

// to select mid month cell, coord x = 4, y = 3 
#define kFocusMonthGridMidCell				17 

#define kChineseDayInterLine				4

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/
 
typedef struct MonthTag{
	RectangleType		bounds;								// gadgets bounds
	int16_t				cellWidth;
	int16_t				cellHeight;
	int16_t				month;								// month displayed
	int16_t				year;								// year displayed
	DateTimeType		selected;
	uint16_t			focusState;							// one handed navigation state
	int16_t				focusedCellNum;						// number of focused cell
	uint16_t			drawDays[maxDayCellsInMonthGrid]; 	// draw data indicators
} MonthType;

typedef MonthType * MonthPtr;

/***********************************************************************
 *
 *	Internal Global Variables
 *
 ***********************************************************************/
static FormType*			sFormP 		= NULL;
static MonthType*			sMonthP 	= NULL;

// The following global variable controls whether MonthView 
// or LunarView will be implemented by DateMonth.c.
static uint16_t 			sMonthViewButtonID 	= MonthMonthViewButton;

static int16_t				sTextLeftMargin 	= dateTextLeftMarginMonthView;
static int16_t				sTextTopMargin 		= dateTextTopMarginMonthView;

static RectangleType		sSelectedDayCellBounds = {0,0,0,0};
static Boolean				sDayCellTracking = false;
static Boolean				sDayCellSelected = false;
static DateType				sDayCellSelectedDate;
static uint16_t				sDayCellSelectedGridIndex = (uint16_t) (-1);

static RGBColorType			sFormFill;
static RGBColorType			sObjectFill;
static RGBColorType			sObjectForeground;
static RGBColorType			sObjectSelectedFill;
static RGBColorType			sObjectSelectedForeground;
static RGBColorType			sObjectMonthHighlight;

GcFontHandle				sDayLabelFont = NULL;
FontHeightType				sDayLabelFontHeight;
GcFontHandle				sDayFont = NULL;
FontHeightType				sDayFontHeight;
GcFontHandle				sCurrentDayFont = NULL;
FontHeightType				sCurrentDayFontHeight;

GcFontHandle				sDaySmallFont = NULL;
FontHeightType				sDaySmallFontHeight;
GcFontHandle				sCurrentDaySmallFont = NULL;
FontHeightType				sCurrentDaySmallFontHeight;

static uint16_t				sMonthViewFocusedItemID = frmInvalidObjectId;



DateGadgetCallbackDataType	sGadgetCallbackData;

/***********************************************************************
 *
 * FUNCTION:    	FirstDayOfMonth
 *
 * DESCRIPTION: 	This routine returns the day-of-the-week, adjusted 
 *					by the preference setting that specifies the first 
 *					day-of-the-week.  If the date passed is a Tuesday 
 *					and the start day of week is Monday, this routine 
 *					will return a value of one.
 *
 * PARAMETERS:  ->	monthP:	pointer to MontnType structure.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/8/96		Initial revision.
 *
 ***********************************************************************/
static uint16_t FirstDayOfMonth (MonthPtr monthP)
{
	return (DayOfWeek (monthP->month, 1, monthP->year) 
			- StartDayOfWeek + daysInWeek) % daysInWeek;
}


/***********************************************************************
*
* FUNCTION:		 	MonthDrawLunarCalendar
*
* DESCRIPTION:		tells if Datebook must draw the standard Month view
*					or Lunar Calendar View
*
* PARAMETERS:  	<-> monthP: month data.
* 			 	->  dayR:	bounds of the day's drawing region.
* 			 	->  start:  appointment's start time.
* 			 	->  end:	appointment's end time.
*
* RETURNED:		 	Nothing.
*
* NOTE:			 	None.
*
* REVISION HISTORY:
*  PPL 03/11/04	 	Initial revision for Lunar Calendar.
*
***********************************************************************/
 static Boolean MonthDrawLunarCalendar(void)
 {
 	return (sMonthViewButtonID == MonthMonthLunarViewButton);
 }

/***********************************************************************
 *
 * FUNCTION:    	MonthUpdateAppointmentDrawData
 *
 * DESCRIPTION: 	This routine updates an appointment indicators 
 *					draw Data
 *
 * PARAMETERS:  <->	monthP:	month data.
 *				->	dayR:	bounds of the day's drawing region.
 *             	-> 	start:	appointment's start time.
 *              ->	end:	appointment's end time.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/8/96		Initial revision.
 *
 ***********************************************************************/
static void MonthUpdateAppointmentDrawData (MonthPtr monthP, uint16_t day, TimeType start, TimeType end, Boolean noTimeEvent)
{
	uint16_t		dayIndex = day-1;
	
	// Draw the indicator for an untimed appointment.
	
	if (noTimeEvent)
	{
		monthP->drawDays[dayIndex] |= kDrawNoTimeIndicator;
		return;
	}

	// Draw indicator for a morning appointment.
	if (TimeToInt(start) < lunchStartTime)
	{
		monthP->drawDays[dayIndex] |= kDrawMorningIndicator;
	}

	// Draw indicator for a lunch appointment.
	if ((TimeToInt(start) >= lunchStartTime && TimeToInt(start) < lunchEndTime) ||
		 (TimeToInt(start) < lunchStartTime && TimeToInt(end) > lunchMidTime))
	{
		monthP->drawDays[dayIndex] |= kDrawLunchIndicator;
	}

	// Draw indicator for an afternoon appointment.
	if (TimeToInt(start) >= lunchEndTime ||
		(TimeToInt(start) < lunchEndTime && TimeToInt(end) > lunchEndTime))
	{
		monthP->drawDays[dayIndex] |= kDrawAfternoonIndicator;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	MonthUpdateRepeatingAppointmentsDrawData
 *
 * DESCRIPTION: 	This routine draupdates ws appointment indicators 
 *					draw datafor repeating appointments in 
 *					the given month.
 *
 * PARAMETERS:  ->	monthP:		month object.
 *				->  apptRec:	appointement record to draw.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/28/03	Initial revision.
 *
 ***********************************************************************/
static void MonthUpdateRepeatingAppointmentsDrawData (MonthPtr monthP, ApptDBRecordType* apptRec)
{
	DateType			temp;
	DateType			date;
	TimeType			saveEndTime = {0,0};
	TimeType			saveStartTime = {0,0};
	Boolean 			untimed;
	Boolean 			scanForSecondRecordPart;
	Boolean 			first;

	//************************ Draw dayly repeating events *****************
	if (apptRec->repeat.repeatType == repeatDaily && apptRec->repeat.repeatFrequency == 1)
	{
		date.day = 1;
		date.month = monthP->month;
		date.year = monthP->year - firstYear;

		// If the first day of this daily repeating event is in this month
		// then we draw a vertical tick.  If it's not (it's in the prior
		// month) then don't.
		if (DateToInt(apptRec->when.date) >= DateToInt(date))
			first = true;
		else
			first = false;

		//************* Check the day before because of overlapping appointments ********
		if (apptRec->when.midnightWrapped)
		{
			temp = date;
			DateAdjust(&temp, -1);
			if (ApptNextRepeat (apptRec, &temp, true))
			{
				// Test if there was an occurence on the previous month				
				if (date.month == (temp.month % monthsInYear + 1))
				{
					// Underlining...
					monthP->drawDays[date.day-1] |= kDrawRepeatingBetweenIndicator;
					if ((DateToInt (temp) == DateToInt (apptRec->repeat.repeatEndDate)))
						monthP->drawDays[0] |= kDrawRepeatingLastIndicator;

					// Conventionnal drawing
					TimeToInt(saveStartTime) = overlapStartTime;
					MonthUpdateAppointmentDrawData (
								monthP, 
								date.day,
								saveStartTime, 
								apptRec->when.endTime,
								apptRec->when.noTimeEvent);
				}
			}
		}

		//************************ Underlinings ************************************
	 	while (ApptNextRepeat (apptRec, &date, true))
	 	{
	 		if ((date.month != monthP->month) || (date.year != monthP->year - firstYear))
				break;
			
			if (first)
			{
				monthP->drawDays[date.day-1] |= kDrawRepeatingFirstIndicator;
				first = false;
			}
			
			if ((DateToInt (date) == DateToInt (apptRec->repeat.repeatEndDate)))
			{
				if (apptRec->when.midnightWrapped)
				{
					temp = date;
					DateAdjust (&temp, 1);
					if (temp.month == date.month)
						monthP->drawDays[temp.day-1] |= kDrawRepeatingLastIndicator;
				}
				else					
					monthP->drawDays[date.day-1] |= kDrawRepeatingLastIndicator;
			}
			
			monthP->drawDays[date.day-1] |= kDrawRepeatingBetweenIndicator;
			
			temp = date;
			DateAdjust (&date, 1);
			
			if (DateToInt (temp) == DateToInt (date)) 
				break;
		}
	}
	

	//************************ Conventionnal Drawings (no underline) ***********
	untimed = apptRec->when.noTimeEvent;
	date.day = 1;
	date.month = monthP->month;
	date.year = monthP->year - firstYear;
	saveStartTime = apptRec->when.startTime;
	saveEndTime = apptRec->when.endTime;

 	while (ApptNextRepeat (apptRec, &date, true))
	{
		if ((date.month != monthP->month) || (date.year != monthP->year - firstYear))
			break;
		
		scanForSecondRecordPart = false;
		temp = date;
					
		do 
		{
			if (apptRec->when.midnightWrapped)
			{
				// The event overlapps midnight, first add the first day part
				if (! scanForSecondRecordPart)
				{
					// Set the end date to overlapEndTime
					apptRec->when.startTime = saveStartTime;
					TimeToInt(apptRec->when.endTime) = overlapEndTime;
					scanForSecondRecordPart = true;
				}
				else
				// Then, add the second part
				{
					// Set the start date to overlapStartTime, increase the date by one
					apptRec->when.endTime = saveEndTime;
					TimeToInt(apptRec->when.startTime) = overlapStartTime;
					DateAdjust(&temp, +1);
					// Exit if the occurence on next day is next month
					if (temp.month != monthP->month)
						break;
					scanForSecondRecordPart = false;
				}
			}
			
			MonthUpdateAppointmentDrawData (
						monthP, 
						temp.day,
						apptRec->when.startTime, 
						apptRec->when.endTime,
						apptRec->when.noTimeEvent);

		} 
		while (scanForSecondRecordPart);

		temp = date;
		DateAdjust (&date, 1);
		
		if (DateToInt (temp) == DateToInt (date)) 
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	MonthUpdateNonRepeatingAppointmentsDrawData
 *
 * DESCRIPTION: 	This routine update appointment indicators draw 
 *					data for all the non-repeating appointments 
 *					the given month.
 *
 * PARAMETERS:  ->	monthP:		month object.
 *				->  apptRec:	appointement record to draw.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/28/03	Initial revision.
 *
 ***********************************************************************/
static void MonthUpdateNonRepeatingAppointmentsDrawData (MonthPtr monthP, ApptDBRecordType* 	apptRec)
{
	TimeType			saveEndTime = {0,0};
	Boolean 			sameMonth;
	Boolean 			scanForSecondRecordPart;


	scanForSecondRecordPart = false;
				
	do {
		// If the appointment overlaps midnight, split it in two events
		if (apptRec->when.midnightWrapped)
		{
			// The event overlapps midnight, first add the first day part
			if (! scanForSecondRecordPart)
			{
				// Set the end date to overlapEndTime
				saveEndTime = apptRec->when.endTime;
				
				TimeToInt(apptRec->when.endTime) = overlapEndTime;
				
				scanForSecondRecordPart = true;
			}
			else
			// Then, add the second part
			{
				// Set the start date to overlapStartTime, increase the date by one
				apptRec->when.endTime = saveEndTime;
				
				TimeToInt(apptRec->when.startTime) = overlapStartTime;
				
				DateAdjust(&apptRec->when.date, +1);
				
				scanForSecondRecordPart = false;
			}
		}
		
		// Check if the appointment is on the same month
		sameMonth = (apptRec->when.date.month == monthP->month) 
					&& (apptRec->when.date.year == monthP->year - firstYear);


		if (sameMonth )
		{
			MonthUpdateAppointmentDrawData (
						monthP, 
						apptRec->when.date.day, 
						apptRec->when.startTime, 
						apptRec->when.endTime, 
						apptRec->when.noTimeEvent);
		}
					
	}
	while (scanForSecondRecordPart);
}


/***********************************************************************
 *
 * FUNCTION:    	MonthUpdateAppointmentsDrawData
 *
 * DESCRIPTION: 	This routine updates appointment indicators draw data
 *					for all the appointment in a month.
 *
 * PARAMETERS:  ->	monthP:	month object.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/28/03	Initial revision.
 *
 ***********************************************************************/
static void MonthUpdateAppointmentsDrawData (MonthPtr monthP)
{
	status_t			err = errNone;
	ApptDBRecordType 	apptRec;


	// reset Draw data:
	memset (monthP->drawDays, 0x00, sizeof(uint16) * maxDaysInMonth);

	// update draw data:
	if (ShowTimedAppts || ShowUntimedAppts)
	{
		DbCursorMoveFirst(gMonthViewCursorID);
		
		while (!DbCursorIsEOF(gMonthViewCursorID))
		{
			if (ApptEventRepeatIsSet(ApptDB, gMonthViewCursorID))
			{
				if (ShowTimedAppts || ShowUntimedAppts)
				{	
					// Load the repeating appointment
					err = ApptGetRecord(
							ApptDB, 
							gMonthViewCursorID, 
							&apptRec, 
							DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);

					MonthUpdateRepeatingAppointmentsDrawData (monthP, &apptRec);
				}
			}
			else
			{
				// Load the non-repeating appointment
				err = ApptGetRecord(
						ApptDB, 
						gMonthViewCursorID, 
						&apptRec, 
						DBK_SELECT_TIME);

				MonthUpdateNonRepeatingAppointmentsDrawData (monthP, &apptRec);
			}		

			// Free the appointment struct
			ApptFreeRecordMemory (&apptRec);

			DbCursorMoveNext(gMonthViewCursorID);
		}
	}
}

 
/***********************************************************************
 *
 * FUNCTION:    	MonthCalculateStartEndRangeTimes
 *
 * DESCRIPTION: 	Calculate the dates of first day and end day of a 
 *					given month.
 *
 * PARAMETERS:  ->	monthP:
 *				->	date:
 *				->	firstDayDateSec:
 * 				->	lastDayDateSecs:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/25/03		Initial revision.
 *
 ***********************************************************************/
static void MonthCalculateStartEndRangeTimes (
	MonthPtr 			monthP, 
	time_t* 			rangeStartTime, 
	time_t * 			rangeEndTime)
{
	DateType			initialScanDate;
	int32_t 			daysInMonth;

	initialScanDate.day = 1;
	initialScanDate.month = monthP->month;
	initialScanDate.year = monthP->year - firstYear;

	daysInMonth = DaysInMonth (monthP->month, monthP->year);

	CalculateStartEndRangeTimes(
							&initialScanDate, 
							daysInMonth,
 							rangeStartTime,
 							rangeEndTime,
 							NULL); 
}


/***********************************************************************
 *
 * FUNCTION:    	MonthCursorOpenOrRequery
 *
 * DESCRIPTION: 	Opens or Requery the month view cursor with
 *					categories and a range of days using the 
 *					SQL where clause.
 *
 * PARAMETERS:  ->	monthP:
 *				->	date:
 *				->	firstDayDateSec:
 * 				->	lastDayDateSecs:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/25/03	Initial revision.
 *
 ***********************************************************************/
 static status_t MonthCursorOpenOrRequery(MonthPtr monthP)
 {
	time_t		rangeStartTime;
	time_t 		rangeEndTime;
	status_t	err = errNone;

	MonthCalculateStartEndRangeTimes (monthP, &rangeStartTime, &rangeEndTime);

	err = ApptDBOpenOrRequeryWithNewRange(ApptDB, &gMonthViewCursorID, rangeStartTime, rangeEndTime, false);

	MonthUpdateAppointmentsDrawData(monthP);
	
	return err;
 }


/***********************************************************************
 *
 * FUNCTION:    	GetDayBounds
 *
 * DESCRIPTION: 	This routine returns the bounds of the specified day.
 *
 * PARAMETERS:  ->	monthP:	pointer to month object.
 *              ->	day:	day of month.
 *              ->	r:		returned: bounds of the day.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/10/96		Initial revision.
 *
 ***********************************************************************/
static void GetDayBounds (MonthPtr monthP, uint16_t day, RectanglePtr r)
{
	uint16_t 	cell;
	int16_t		cellWidth;
	int16_t 	cellHeight;

	cell = day - 1 + FirstDayOfMonth (monthP);
	
	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonth;
	
	r->topLeft.x = monthP->bounds.topLeft.x + (cell % daysInWeek) * cellWidth;
	r->topLeft.y = monthP->bounds.topLeft.y + (cell / daysInWeek) * cellHeight;
	r->extent.x = cellWidth;
	r->extent.y = cellHeight;
}
	

/***********************************************************************
 *
 * FUNCTION:    	GetDaySelectionBounds
 *
 * DESCRIPTION: 	This routine returns the selection bounds of the 
 *					specified day.
 *
 * PARAMETERS:  ->	monthP:	pointer to month object.
 *              ->	day:	day of month.
 *              ->	r:		returned: bounds of the day selection.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	JMP	11/14/99	Initial revision.
 *
 ***********************************************************************/
static void GetDaySelectionBounds(MonthPtr monthP, uint16_t day, RectanglePtr r)
{
	uint16_t 	cell;
	int16_t		cellWidth;
	int16_t 	cellHeight;
	
	cell = day - 1 + FirstDayOfMonth (monthP);
	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonth;
	
	r->topLeft.x = monthP->bounds.topLeft.x + (cell % daysInWeek) * cellWidth;
	r->topLeft.y = monthP->bounds.topLeft.y + (cell / daysInWeek) * cellHeight;
	r->extent.x = cellWidth + 1;
	r->extent.y = cellHeight + 1;
	
	RctInsetRectangle (r, 1);
}


/***********************************************************************
 *
 * FUNCTION:    	DrawAppointment
 *
 * DESCRIPTION: 	This routine draw an appointment indicator.
 *
 * PARAMETERS:  ->	monthP:		Month data.
 *             	-> 	dayIndex:	Day Index (0 to daysInMonth -1.)
 *              ->	dayBounds:	Day bounds.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/28/03	Initial revision.
 *
 ***********************************************************************/
static void DrawAppointment (MonthPtr monthP, uint16 dayNum, RectangleType* dayBounds)
{
	Coord 			x;
	Coord			x1;
	Coord			x2;
	Coord 			y;
	RectangleType 	r;
	uint16_t		dayIndex = dayNum-1;

	// if there are no indicators on that day we just exit.
	if (monthP->drawDays[dayIndex] == kDrawNoIndicators
	|| (!ShowTimedAppts && !ShowUntimedAppts && !ShowDailyRepeatingAppts) )
		return;

	WinSetCoordinateSystem(kCoordinatesNative);

	if (ShowDailyRepeatingAppts 
	&& (	(monthP->drawDays[dayIndex] & kDrawRepeatingFirstIndicator)
		||	(monthP->drawDays[dayIndex] & kDrawRepeatingBetweenIndicator)
		|| 	(monthP->drawDays[dayIndex] & kDrawRepeatingLastIndicator)))
	{
		y 	= dayBounds->topLeft.y + dayBounds->extent.y - repeatIndicatorBottomMargin;
		x1 	= dayBounds->topLeft.x;
		x2 	= dayBounds->topLeft.x + dayBounds->extent.x;
				
		if (monthP->drawDays[dayIndex] & kDrawRepeatingFirstIndicator)
		{
			WinDrawLine((Coord)(x1 + 1), (Coord)(y - 1),
								(Coord)(x1 + 1), (Coord)(y + 1));
			x1 += 2;
		}
		
		if (monthP->drawDays[dayIndex] & kDrawRepeatingLastIndicator)
		{
			WinDrawLine((Coord)(x2 - 1), (Coord)(y - 1),
						(Coord)(x2 - 1), (Coord)(y + 1));
			x2 -= 2;
		}

		WinDrawGrayLine (x1, y, x2, y);
	}


	// Draw the indicator for an untimed appointment.
	if (ShowUntimedAppts
	&& monthP->drawDays[dayIndex] & kDrawNoTimeIndicator)
	{
		x = dayBounds->topLeft.x + WinScaleCoord(untimeIndicatorRightMargin, false);
		y = dayBounds->topLeft.y + WinScaleCoord(untimeIndicatorTopMargin, false);
		
		//WinDrawLine ((Coord)(x - 1), y, (Coord)(x + 1), y);
		r.topLeft.x = x - WinScaleCoord(1, false);
		r.topLeft.y = y;
		r.extent.x = WinScaleCoord(3, false);
		r.extent.y = WinScaleCoord(1, false);
		WinDrawRectangle(&r, 0);

		//WinDrawLine (x, (Coord)(y - 1), x, (Coord)(y + 1));
		r.topLeft.x = x;
		r.topLeft.y = y - WinScaleCoord(1, false);
		r.extent.x = WinScaleCoord(1, false);
		r.extent.y = WinScaleCoord(3, false);
		WinDrawRectangle(&r, 0);
	}

	if (ShowTimedAppts)
	{
		// Draw the indicator for an timed appointment.
		x = dayBounds->topLeft.x + dayBounds->extent.x 
			- WinScaleCoord(timeIndicatorWidth, false)
			- WinScaleCoord(timeIndicatorRightMargin, false);
			
		y = dayBounds->topLeft.y 
			+ WinScaleCoord(timeIndicatorTopMargin, false);

		// Draw indicator for a morning appointment.
		if (monthP->drawDays[dayIndex] & kDrawMorningIndicator)
		{
			r.topLeft.x = x;
			r.topLeft.y = y;
			r.extent.x = WinScaleCoord(timeIndicatorWidth, false);
			r.extent.y = WinScaleCoord(morningHeight, false);
			
			WinDrawRectangle (&r, 0);
		}

		// Draw indicator for a lunch appointment.
		if (monthP->drawDays[dayIndex] & kDrawLunchIndicator)
		{
			r.topLeft.x = x;
			r.topLeft.y = y + WinScaleCoord(morningHeight + 1, false);
			r.extent.x = WinScaleCoord(timeIndicatorWidth, false);
			r.extent.y = WinScaleCoord(lunchHeight, false);
			
			WinDrawRectangle (&r, 0);
		}

		// Draw indicator for an afternoon appointment.
		if (monthP->drawDays[dayIndex] & kDrawAfternoonIndicator)
		{
			r.topLeft.x = x;
			r.topLeft.y = y + WinScaleCoord(morningHeight + lunchHeight + 2, false);
			r.extent.x = WinScaleCoord(timeIndicatorWidth, false);
			r.extent.y = WinScaleCoord(afternoonHeight, false);
			
			WinDrawRectangle (&r, 0);
		}
	}

	WinSetCoordinateSystem(kCoordinatesStandard);
}


/***********************************************************************
 *
 * FUNCTION:    	DrawAppointmentsInMonth
 *
 * DESCRIPTION: 	This routine draws days number indicators for all 
 *					the appointments in a month.
 *
 * PARAMETERS:  ->	monthP:	month object.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/10/96		Initial revision.
 *	PPL	07/25/03	Cursors !
 *
 ***********************************************************************/
static void DrawAppointmentsInMonth (MonthPtr monthP)
{
	RectangleType	dayBounds;
	int16_t			dayNum;
	int16_t 		daysInMonth;

	daysInMonth = DaysInMonth(monthP->month , monthP->year);
	
	for (dayNum = 1 ; dayNum  <= daysInMonth ; ++dayNum)
	{
		GetDayBounds (monthP, dayNum, &dayBounds);
		DrawAppointment (monthP, dayNum, &dayBounds);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DrawMonthBackground
 *
 * DESCRIPTION: 	Draws Month View Background.
 *
 * PARAMETERS:  ->	monthP:		month object.
 *				-> 	cellWidth: 	day cell width
 *				->	cellHeight: day cell height
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GDA	05/00/04	Initial revision.
 *	PPL	08/05/04	Reduce calls to GcGetCurrentContext/GcReleaseContext
 *					calls to one and use GCPushState()/GcPopState() 
 *					in place.
 *
 ***********************************************************************/

static void DrawMonthBackground(MonthPtr monthP, int16_t cellWidth, int16_t cellHeight)
{
	uint16_t		dow;
	int16_t			lastDay;
	GcGradientType	grad;
	GcPointType		pnt[2];
	GcColorType		col[2];
	GcHandle		gc;
	RectanglePtr	bounds;
	Boolean			fiveRow;

	dow = FirstDayOfMonth (monthP);

	// If the color is black, don't draw gradient background
	if(sObjectMonthHighlight.r == 0 && sObjectMonthHighlight.g == 0 &&
	   sObjectMonthHighlight.b == 0)
		return;

	gc = GcGetCurrentContext();
	
	col[0].red = col[1].red = sObjectMonthHighlight.r;
	col[0].green = col[1].green = sObjectMonthHighlight.g;
	col[0].blue = col[1].blue = sObjectMonthHighlight.b;

	bounds = &monthP->bounds;

	WinEraseRectangle(bounds, 0);

	if(dow != 0)
	{
		pnt[0].x = (fcoord_t)(bounds->topLeft.x);
		pnt[0].y = (fcoord_t)(bounds->topLeft.y);
		pnt[1].x = (fcoord_t)(bounds->topLeft.x + (dow * cellWidth));
		pnt[1].y = pnt[0].y;

		col[0].alpha = 32;
		col[1].alpha = 192;

		GcPushState(gc);

			GcInitGradient(&grad, pnt, col, 2);
			GcSetGradient(gc, &grad);

			GcSetDithering(gc, true);

			GcRect(gc, pnt[0].x, pnt[0].y, pnt[1].x, pnt[1].y + cellHeight);
			GcPaint(gc);

		GcPopState(gc);
	}

	lastDay = DaysInMonth(monthP->month, monthP->year);

	fiveRow = ((dow + lastDay) < 35);

	dow = (dow + lastDay) % 7;

	bounds = &monthP->bounds;

	pnt[0].x = (fcoord_t)(bounds->topLeft.x + (dow * cellWidth));
	pnt[0].y = (fcoord_t)(bounds->topLeft.y + cellHeight*(fiveRow ? 4 : 5));
	pnt[1].x = (fcoord_t)(bounds->topLeft.x + (7 * cellWidth));
	pnt[1].y = pnt[0].y;

	col[0].alpha = 192;

	if(fiveRow)
		col[1].alpha = 192 - (((192 - 32) * (6 - dow)) / (13 - dow));
	else
		col[1].alpha = 32;

	
	GcPushState(gc);

		GcSetDithering(gc, true);

		GcInitGradient(&grad, pnt, col, 2);
		GcSetGradient(gc, &grad);

		GcRect(gc, pnt[0].x, pnt[0].y, pnt[1].x, pnt[1].y + cellHeight);

		GcPaint(gc);

		if(fiveRow)
		{
			pnt[0].x = (fcoord_t)(bounds->topLeft.x);
			pnt[0].y = (fcoord_t)(bounds->topLeft.y + cellHeight*5);
			pnt[1].x = (fcoord_t)(bounds->topLeft.x + (7 * cellWidth));
			pnt[1].y = pnt[0].y;

			col[0].alpha = col[1].alpha;  // Start where last row left off
			col[1].alpha = 32;

			GcInitGradient(&grad, pnt, col, 2);
			GcSetGradient(gc, &grad);

			GcRect(gc, pnt[0].x, pnt[0].y, pnt[1].x, pnt[1].y + cellHeight);

			GcPaint(gc);
		}
	
	GcPopState(gc);
	
	GcReleaseContext(gc);
}

/***********************************************************************
 *
 * FUNCTION:    	DrawMonthSolarDaysNumber
 *
 * DESCRIPTION: 	This routine draws appointment indicators in a month.
 *
 * PARAMETERS:  ->	monthP:			Month data.
 *				->  cellWidth:		width for day cell.
 *				->	cellHeight:		height for day cell.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/11/04	Initial revision for Lunar Calendar.
 *	GDA	05/??/04	Add background image support and draw previous 
 *					and next month day nums.
 *	PPL	08/05/04	Remove Drawing of previous month for the very first year 
 *					(January 1904) as we canno't go before 
 *					Remove Drawing of next month for the very firlastst year 
 *					(December 2031) as we canno't go after 
 *					(this was already in CR147 DateMonth.c I did for Sahara.)
 *
 ***********************************************************************/
static void DrawMonthSolarDaysNumber(MonthPtr monthP, int16_t cellWidth, int16_t cellHeight)
{
	int16_t 		drawX;
	int16_t 		drawY;
	int16_t 		lastDay;
	char			dayInAscii[3];
	uint16_t		dow;
	uint16_t		dayNum;
	uint16_t		row = 0;
	RectangleType	todaysBounds;
	DateTimeType	today;
	Boolean 		todayIsVisible;
	int16_t 		currentFontYOffset;
	DateType		date;
	GcHandle		gc;
	int16_t			month;
	int16_t			year;


	gc = GcGetCurrentContext();

	GcSetFont(gc, sDayFont);
	
	dow = FirstDayOfMonth (monthP);
	
	drawX = monthP->bounds.topLeft.x 
		+ sTextLeftMargin;
		
	drawY = monthP->bounds.topLeft.y 
		+ sTextTopMargin 
 		+ (int16_t)sDayFontHeight.ascent 
 		+ (int16_t)sDayFontHeight.leading;

	currentFontYOffset = (int16_t)(sCurrentDayFontHeight.ascent 
		+ sCurrentDayFontHeight.leading 
		- sDayFontHeight.ascent 
		- sDayFontHeight.leading);

	// Display a rectangle around today's day if it's visible.
	TimSecondsToDateTime(TimGetSeconds(), &today);
	todayIsVisible = TodayIsVisible(monthP, today);
	if (todayIsVisible)
		GetDaySelectionBounds(monthP, today.day, &todaysBounds);

	year = monthP->year;
		month = monthP->month -1;
		
	if (month <  january)
	{
		month = december;
		year--;
	}

	// We  draw previous month days only if we are not outside the possible values
	// for our system date coding (1st january 1904)
	if (year >= firstYear)
	{
		// Draw previous month's days
		if(dow)
		{
			GcSetColor(gc, sObjectMonthHighlight.r, sObjectMonthHighlight.g, sObjectMonthHighlight.b, 255);
			date.year = monthP->year; 
			date.month = monthP->month; 
			date.day = 1;
			DateAdjust(&date, -dow);
			lastDay = DaysInMonth(date.month, date.year);

			for(dayNum = lastDay - dow + 1; dayNum <= lastDay ;dayNum++)
			{
				StrIToA (dayInAscii, dayNum);
				GcDrawTextAt(gc, drawX, drawY, dayInAscii, ((dayNum < 10) ? 1 : 2));
				drawX += cellWidth;
			}
		}
	}
	else
		drawX = monthP->bounds.topLeft.x 
			+ (dow * cellWidth)
			+ sTextLeftMargin;

	// Now draw regular days
	GcSetColor(gc, sObjectForeground.r, sObjectForeground.g, sObjectForeground.b, 255);
	lastDay = DaysInMonth(monthP->month, monthP->year);

	for (dayNum =  1; dayNum <= lastDay ; dayNum++, dow++)
	{
		if (dow == daysInWeek)
		{
			drawX = monthP->bounds.topLeft.x + sTextLeftMargin;
			drawY += cellHeight;
			dow = 0;
			row++;
		}
		
		if (todayIsVisible && today.day == dayNum)
		{
			GcSetFont(gc, sCurrentDayFont);
			drawY += currentFontYOffset;
		}
		
		StrIToA (dayInAscii, dayNum);
		GcDrawTextAt( gc, drawX, drawY, dayInAscii, ((dayNum < 10) ? 1 : 2));

		if (todayIsVisible && today.day == dayNum)
		{
			GcSetFont(gc, sDayFont);
			drawY -= currentFontYOffset;
		}
		
		drawX += cellWidth;
	}

	// increment month and eventually year
	month = monthP->month +1;
	year = monthP->year;
	if (month >  december)
	{
		month = january;
		year++;
	}
	
	// We draw next month days only if we are not outside the possible values
	// for our system date coding (31th december 2031)
	if (year <= lastYear)
	{

		GcSetColor(gc, sObjectMonthHighlight.r, sObjectMonthHighlight.g, sObjectMonthHighlight.b, 255);

		// Draw next month's days
		for(dayNum = 1; dow < 7 || row < 5;dayNum++, dow++)
		{
			if (dow == daysInWeek)
			{
				drawX = monthP->bounds.topLeft.x + sTextLeftMargin;
				drawY += cellHeight;
				dow = 0;
				row++;
			}

			StrIToA (dayInAscii, dayNum);
			GcDrawTextAt(gc, drawX, drawY, dayInAscii, ((dayNum < 10) ? 1 : 2));
			drawX += cellWidth;
		}
	}

	GcReleaseContext(gc);
}


/***********************************************************************
 *
 * FUNCTION:    DrawLunarDateText
 *
 * DESCRIPTION: Draw the Chinese text identifying <gregorianDay>'s
 *						position in the Chinese lunar month, aligning its
 *						top left corner with <drawX> and <drawY>.
 *
 * PARAMETERS:  -> monthP:			Month View data
 *				-> gregorianDay:	date number in currently displayed month
 *				-> drawX:			left side of text
 *				-> drawY:			top of text
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *	CS	10/23/02	New today to support Lunar View.
 *	PPL	03/11/04	Lunar Calendar port to Palm OS 6 from Palm OS 5.4
 *
 ***********************************************************************/
static void DrawLunarDateText (MonthPtr monthP, UInt16 gregorianDay, Coord drawX, Coord drawY)
{
	BitmapType*		chineseLunarDayBitmapP = NULL;
	DateTimeType	dateTime;
	UInt32			dateSeconds;
	UInt16			lunarYear;
	UInt16			lunarMonth;
	UInt16			lunarDay;
	Boolean			isLeapMonth;
	Char			lunarDayStr[kMaxLunarDayStrSize];
	MemHandle		resH = NULL;
	Err				tErr = errNone;
	
	FntSetFont((FontID)chineseNumeralFontID);	
	
	dateTime.second = 0;
	dateTime.minute = 0;
	dateTime.hour = 0;
	dateTime.day = gregorianDay;
	dateTime.month = monthP->month;
	dateTime.year = monthP->year;
	
	dateSeconds = TimDateTimeToSeconds(&dateTime);
	
	tErr = DateSecondsToLunarDate(	gApplicationDbP,
									dateSeconds,
									&lunarYear,
									&lunarMonth,
									&lunarDay,
									&isLeapMonth);
	if (tErr != errNone)
	{
		ErrNonFatalDisplay("Error converting seconds to lunar date");
		return;
	}
	
	if (lunarDay == timFirstChineseLunarDay)
	{
		
		if	((lunarMonth == 11) || (lunarMonth == 12))
		{
			chineseLunarDayBitmapP = LunarCalendarGetChineseLargeMonthBitmap(lunarMonth);

			if (chineseLunarDayBitmapP)
				WinDrawBitmap(chineseLunarDayBitmapP, drawX, drawY);
		}
		else
		{
			SysStringByIndex(	gApplicationDbP,
								(isLeapMonth) ? lunarLeapMonthsStrlID :	lunarMonthsStrlID,
								lunarMonth,
								lunarDayStr,
								sizeof(lunarDayStr));
									
			ErrNonFatalDisplayIf(StrLen(lunarDayStr) != 2, "Lunar day string not 2 characters long");

			WinPaintChars(lunarDayStr, StrLen(lunarDayStr), drawX, drawY);
		}
	}
	else
	{
		
		SysStringByIndex(	gApplicationDbP,
							lunarDaysStrlID,
							lunarDay,
							lunarDayStr,
							sizeof(lunarDayStr));
								
		ErrNonFatalDisplayIf(StrLen(lunarDayStr) != 2, "Lunar day string not 2 characters long");
									
		WinPaintChars(lunarDayStr, StrLen(lunarDayStr), drawX, drawY);
	
	}
}


static void DrawLunarDateTextGc (MonthPtr monthP, UInt16 gregorianDay, Coord drawX, Coord drawY)
{
	BitmapType*		chineseLunarDayBitmapP = NULL;
	DateTimeType	dateTime;
	UInt32			dateSeconds;
	UInt16			lunarYear;
	UInt16			lunarMonth;
	UInt16			lunarDay;
	Boolean			isLeapMonth;
	Char			lunarDayStr[kMaxLunarDayStrSize];
	MemHandle		resH = NULL;
	Err				tErr = errNone;
	GcHandle		gc;
	fcoord_t		X = (fcoord_t) drawX;
	fcoord_t		Y =  (fcoord_t) drawY;

	
	dateTime.second = 0;
	dateTime.minute = 0;
	dateTime.hour = 0;
	dateTime.day = gregorianDay;
	dateTime.month = monthP->month;
	dateTime.year = monthP->year;
	
	dateSeconds = TimDateTimeToSeconds(&dateTime);
	
	tErr = DateSecondsToLunarDate(	gApplicationDbP,
									dateSeconds,
									&lunarYear,
									&lunarMonth,
									&lunarDay,
									&isLeapMonth);
	if (tErr != errNone)
	{
		ErrNonFatalDisplay("Error converting seconds to lunar date");
		return;
	}

	gc = GcGetCurrentContext();

	GcSetFont(gc, sDayFont);

	Y += sDayFontHeight.ascent + sDayFontHeight.leading;

	GcSetColor(gc, sObjectForeground.r, sObjectForeground.g, sObjectForeground.b, 255);
	
	if (lunarDay == timFirstChineseLunarDay)
	{
	
		SysStringByIndex(	gApplicationDbP,
							(isLeapMonth) ? lunarLeapMonthsStrlID :	lunarMonthsStrlID,
							lunarMonth,
							lunarDayStr,
							sizeof(lunarDayStr));
									
		ErrNonFatalDisplayIf(StrLen(lunarDayStr) != 2, "Lunar day string not 2 characters long");

			
		GcDrawTextAt(gc, drawX, drawY, lunarDayStr, StrLen(lunarDayStr));
	}
	else
	{
		
		SysStringByIndex(	gApplicationDbP,
							lunarDaysStrlID,
							lunarDay,
							lunarDayStr,
							sizeof(lunarDayStr));
								
		ErrNonFatalDisplayIf(StrLen(lunarDayStr) != 2, "Lunar day string not 2 characters long");
									
		GcDrawTextAt(gc, drawX, drawY, lunarDayStr, StrLen(lunarDayStr));
	}

	GcReleaseContext(gc);
}



/***********************************************************************
 *
 * FUNCTION:    	DrawMonthLunarDaysNumber
 *
 * DESCRIPTION: 	This routine draw Lunar Calendar string 
 *					for each days.
 *
 * PARAMETERS:  ->	monthP:			Month data.
 *				->  cellWidth:		width for day cell.
 *				->	cellHeight:		height for day cell.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/11/04	Initial revision.
 * 	PPL 08/05/04	Adopt similar drawing to Month
 *					add previous and newt day num drawing.
 *					Remove Drawing of previous month for the very first year 
 *					(January 1904) as we canno't go before 
 *					Remove Drawing of next month for the very firlastst year 
 *					(December 2031) as we canno't go after 
 *					(this was already in CR147 DateMonth.c I did for Sahara.)
 *
 ***********************************************************************/
static void DrawMonthLunarDaysNumber(MonthPtr monthP, int16_t cellWidth, int16_t cellHeight)
{
	int16_t 		drawX;
	int16_t 		drawY;
	int16_t 		lastDay;
	int16_t			lineHeight = kChineseDayInterLine;
	char			dayInAscii[3];
	uint16_t		dow;
	uint16_t		dayNum;
	uint16_t		row = 0;
	RectangleType	todaysBounds;
	DateTimeType	today;
	Boolean 		todayIsVisible;
	int16_t 		currentFontYOffset;
	DateType		date;
	GcHandle		gc;
	int16_t			month;
	int16_t			year;


	WinPushDrawState(); 
	WinSetDrawMode(winOverlay);

	gc = GcGetCurrentContext();
	
	GcSetFont(gc, sDaySmallFont);

	dow = FirstDayOfMonth (monthP);
	
	drawX = monthP->bounds.topLeft.x 
		+ sTextLeftMargin;
			
	drawY = monthP->bounds.topLeft.y 
		+ sTextTopMargin 
		+ (int16_t)sDaySmallFontHeight.ascent 
		+ (int16_t)sDaySmallFontHeight.leading;

	currentFontYOffset = (int16_t)(sCurrentDaySmallFontHeight.ascent 
		+ sCurrentDaySmallFontHeight.leading 
		- sDaySmallFontHeight.ascent 
		- sDaySmallFontHeight.leading);
	
	
	// Display a rectangle around today's day if it's visible.
	TimSecondsToDateTime(TimGetSeconds(), &today);
	todayIsVisible = TodayIsVisible(monthP, today);
	if (todayIsVisible)
		GetDaySelectionBounds(monthP, today.day, &todaysBounds);

	year = monthP->year;
	month = monthP->month -1;
		
	if (month <  january)
	{
		month = december;
		year--;
	}

	// We  draw previous month days only if we are not outside the possible values
	// for our system date coding (1st january 1904)
	if (year >= firstYear)
	{
		// Draw previous month's days
		if(dow)
		{
			GcSetColor(gc, sObjectMonthHighlight.r, sObjectMonthHighlight.g, sObjectMonthHighlight.b, 255);
			date.year = monthP->year; 
			date.month = monthP->month; 
			date.day = 1;
			DateAdjust(&date, -dow);
			lastDay = DaysInMonth(date.month, date.year);

			for(dayNum = lastDay - dow + 1; dayNum <= lastDay ;dayNum++)
			{
				StrIToA (dayInAscii, dayNum);
				GcDrawTextAt(gc, drawX, drawY, dayInAscii, ((dayNum < 10) ? 1 : 2));
				drawX += cellWidth;
			}
		}
	}
	else
		drawX = monthP->bounds.topLeft.x 
			+ (dow * cellWidth)
			+ sTextLeftMargin;

	GcSetColor(gc, sObjectForeground.r, sObjectForeground.g, sObjectForeground.b, 255);
	lastDay = DaysInMonth(monthP->month, monthP->year);
	
	for (dayNum =  1; dayNum <= lastDay ; dayNum++, dow++)
	{
		if (dow == daysInWeek)
		{
			drawX = monthP->bounds.topLeft.x + sTextLeftMargin;
			drawY += cellHeight;
			dow = 0;
			row++;
		}

		StrIToA (dayInAscii, dayNum);

		GcPushState(gc);
		
			if ( todayIsVisible && today.day == dayNum)
			{
				GcSetFont(gc, sCurrentDaySmallFont);
				drawY += currentFontYOffset;
			}
	
			GcDrawTextAt( gc, drawX, drawY, dayInAscii, ((dayNum < 10) ? 1 : 2));

			if ( todayIsVisible && today.day == dayNum)
			{
				GcSetFont(gc, sDaySmallFont);
				drawY -= currentFontYOffset;
			}

		GcPopState(gc);

		GcPushState(gc);
	
			DrawLunarDateText ( monthP,  dayNum,  drawX,  drawY + lineHeight);
			
		GcPopState(gc);
		
		drawX += cellWidth;
	}


	// increment month and eventually year
	month = monthP->month +1;
	year = monthP->year;
	if (month >  december)
	{
		month = january;
		year++;
	}
	
	// We draw next month days only if we are not outside the possible values
	// for our system date coding (31th december 2031)
	if (year <= lastYear)
	{
		GcSetColor(gc, sObjectMonthHighlight.r, sObjectMonthHighlight.g, sObjectMonthHighlight.b, 255);
		GcSetFont(gc, sDaySmallFont);
		
		// Draw next month's days
		for( dayNum = 1; dow < 7 || row < 5; dayNum++, dow++ )
		{
			if (dow == daysInWeek)
			{
				drawX = monthP->bounds.topLeft.x + sTextLeftMargin;
				drawY += cellHeight;
				dow = 0;
				row++;
			}

			StrIToA (dayInAscii, dayNum);
			GcDrawTextAt(gc, drawX, drawY, dayInAscii, ((dayNum < 10) ? 1 : 2));
			drawX += cellWidth;
		}
	}

	GcReleaseContext(gc);

	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION: 	 	DrawMonth
 *
 * DESCRIPTION: 	Draw the month object.
 *
 * PARAMETERS:  ->	monthP:		pointer to month object to draw.
 *				->	selectDay:	if true, draw "today" selected.
 *								when possible.
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RFL	04/08/96	Initial revision.
 *	KWK	08/26/99	Use DateTemplateToAscii to get the DOW first letter.
 *	GAP	10/18/99	Get DOW labels from daysOfWeekInitialsStrID resource 
 *					and use StartDayOfWeek to determine which letter is 
 *					start day.
 *	JMP	10/31/99	Eliminate WinInvertRectangle(); draw day as selected 
 * 					instead.Also, always call DrawAppointmentsInMonth(), 
 *					as DrawMonth() was always followed by that call 
 *					-- helps consolidate various drawing problems.
 *	JMP	11/14/99	Updated the drawing such that non-selectable areas 
 *					of the MonthView
 *					are in standard Form colors while selectable areas 
 *					use control style colors.  Also, added selectDay 
 *					argument to allow today's date to either by drawn 
 *					selected or not.
 *
 ***********************************************************************/
static void DrawMonth (MonthPtr monthP, Boolean selectDay)
{
	int16_t			drawX;
	int16_t			drawY;
	int16_t 		cellWidth;
	int16_t 		cellHeight;
	int16_t 		lineHeight;
	uint16_t		i;
	int16_t			x;
	int16_t			y;
	uint8_t 		dayOfWeek;
 	MemHandle		dayLabelsH;
 	char *			dayLabels;
	uint16_t		labelLength;
	char*			label;
	GcHandle		gc;

	PIMAppProfilingBegin("Draw Month Appointments");	

	WinPushDrawState();

	WinSetCoordinateSystem(kCoordinatesNative);

	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonth;

	// and to avoid to have a few extra pixels on right and botton edge
	// we insure the size of the full month view is in Cell Width and height
	monthP->bounds.extent.x = cellWidth * daysInWeek;
	monthP->bounds.extent.y = cellHeight * linesInMonth;

	// Get the resource that contains the first letter of each day.
	dayLabelsH = DmGetResource (gApplicationDbP, strRsc, daysOfWeekInitialsStrID);
 	dayLabels = MemHandleLock (dayLabelsH);

	// Calculate length of one item in string */
	labelLength = strlen (dayLabels) / daysInWeek;

	gc = GcGetCurrentContext();

	// Draw the days of the week labels right justified to the number columns	
	// Be sure to draw the labels with respect to current setting of StartDayOfWeek. Somes locales
	// consider Monday the first day while others use Sunday.  There is also a user preference 
	// selection in Prefs app/Formats panelthat will allow the user to change first day of week.
	//drawY = monthP->bounds.topLeft.y - FntLineHeight() - 2;
	drawY = monthP->bounds.topLeft.y - (int16_t)sDayLabelFontHeight.descent;

	GcSetColor(gc, sObjectForeground.r, sObjectForeground.g, sObjectForeground.b, 255);

	GcSetFont(gc, sDayLabelFont);

	for (i = 0; i <= daysInWeek; i++)
	{
		dayOfWeek = (i + StartDayOfWeek) % daysInWeek;
		label = &dayLabels[labelLength * dayOfWeek];
		drawX = monthP->bounds.topLeft.x + (cellWidth * i) +
			((cellWidth - (int16_t)GcFontStringWidth(sDayLabelFont, label, labelLength)) / 2);

		GcDrawTextAt(gc, drawX, drawY, label, labelLength);
	}

	GcReleaseContext(gc);

	// Unlock the day of week label resource now that we are done with it.
	MemHandleUnlock (dayLabelsH);
   	DmReleaseResource (dayLabelsH);   
	
	DrawMonthBackground(monthP, cellWidth, cellHeight);
	
	gc = GcGetCurrentContext();

	if (sDayCellTracking && sDayCellSelected)
	{
		GcSetColor(gc, sObjectSelectedFill.r, sObjectSelectedFill.g, sObjectSelectedFill.b, 255);

		GcRect(gc, (fcoord_t)sSelectedDayCellBounds.topLeft.x, (fcoord_t)sSelectedDayCellBounds.topLeft.y,
			   (fcoord_t)(sSelectedDayCellBounds.topLeft.x + sSelectedDayCellBounds.extent.x),
			   (fcoord_t)(sSelectedDayCellBounds.topLeft.y + sSelectedDayCellBounds.extent.y));
		GcPaint(gc);
	}

	// Draw the grid.  Change the foreground color temporarily to get the right effect,
	// and put it back when we're done.
	//WinSetForeColorRGB(&colorLine, NULL);
	GcSetColor(gc, colorLine.r, colorLine.g, colorLine.b, 255);
	
	x = monthP->bounds.topLeft.x;
	y = monthP->bounds.topLeft.y;
	
	for (i = 0; i < daysInWeek + 1; i++)
	{
		GcRect (gc, (fcoord_t)x, (fcoord_t)y, (fcoord_t)(x + 1),
				(fcoord_t)(y + (cellHeight * maxWeeksInMonth) + 1));
		x += cellWidth;
	}

	x = monthP->bounds.topLeft.x;
	
	for (i = 0; i < maxWeeksInMonth + 1; i++)
	{
		if(i == maxWeeksInMonth)
			lineHeight = 2;
		else
			lineHeight = 1;
		GcRect(gc, (fcoord_t)x, (fcoord_t)y,
			   (fcoord_t)((x + (cellWidth * daysInWeek)) + 1),
			   (fcoord_t)(y + lineHeight));
		y += cellHeight;
	}
	
	GcPaint(gc);
	GcReleaseContext(gc);

	if (MonthDrawLunarCalendar())
	{
		DrawMonthLunarDaysNumber(monthP, cellWidth, cellHeight);
	}
	else
	{
		DrawMonthSolarDaysNumber(monthP, cellWidth, cellHeight);
		DrawAppointmentsInMonth (monthP);
	}
	
	WinPopDrawState();
	
	PIMAppProfilingEnd();	
}


/***********************************************************************
 *
 * FUNCTION: 	 	MapPointToItem
 *
 * DESCRIPTION: 	Return return the item at x, y.
 *
 * PARAMETERS:  ->	x: 	x-axis coordinates.
 *				-> 	y: 	y-axis coordinates.
 *              	r:  bounds of the item area (not the MTWTFSS area.)
 *
 * RETURNED:	 	item number (doesn't check for invalid bounds!)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RFL 11/15/94	Initial Revision.
 *
 ***********************************************************************/
static int16_t MapPointToItem (int16_t x, int16_t y, RectanglePtr r)
{
	int16_t	itemNumber;

	itemNumber = daysInWeek * ((y - r->topLeft.y) / (r->extent.y / linesInMonth));
	
	itemNumber += ((x - r->topLeft.x) / (r->extent.x / daysInWeek));
	
	return itemNumber;
}

/***********************************************************************
 *
 * FUNCTION: 	 	MapCellNum2Rectangle
 *
 * DESCRIPTION: 	returns rectangle for related cell
 *
 * PARAMETERS:  ->	monthP:  Month data.
 *				-> 	y: 	y-axis coordinates.
 *              	cellBounds:  bounds of the item area (not the MTWTFSS area.)
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			Excludes  the MTWTFSS area.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MapCellNum2Rectangle (MonthPtr monthP, int16_t cellNum, RectangleType* cellBounds)
{
	if (!cellBounds)
		return;

	cellBounds->topLeft.x 	= monthP->bounds.topLeft.x + (cellNum % daysInWeek) * monthP->cellWidth ;
	cellBounds->topLeft.y 	= monthP->bounds.topLeft.y + (cellNum / daysInWeek) * monthP->cellHeight;
	cellBounds->extent.x 	= monthP->cellWidth+1;
	cellBounds->extent.y 	= monthP->cellHeight+1;
}


/***********************************************************************
 *
 * FUNCTION: 	 	MapCellRectangle2Num
 *
 * DESCRIPTION: 	returns cell num  given its rectangle
 *
 * PARAMETERS:  ->	monthP: 		Month data.
 *              ->	cellBounds:  	Bounds of the cell
 *
 * RETURNED:	 	cell num [in 0 to 41].
 *
 * NOTE:			Excludes  the MTWTFSS area.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static int16_t MapCellRectangle2Num (MonthPtr monthP, RectangleType* cellBounds)
{
	Coord 		x;
	Coord 		y;
	int16_t 	cellNum;
	
	if (!cellBounds)
		return (int16_t)noFocus;
		
	x = cellBounds->topLeft.x - monthP->bounds.topLeft.x;
	y = cellBounds->topLeft.y - monthP->bounds.topLeft.y;
	
	cellNum = (x / monthP->cellWidth) + (y / monthP->cellHeight) * daysInWeek;

	return cellNum;
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewUpdateDisplay
 *
 * DESCRIPTION: 	Redraws the Month View.
 *					Handles winUpdateEvt.
 *
 * PARAMETERS:  ->	frmP: form tp update.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	09/12/02	Initial revision.
 *
 ***********************************************************************/
static void MonthViewUpdateDisplay (FormType* frmP, Boolean selectDay)
{
	PIMAppProfilingBegin("Draw Month Form");	
	
	FrmDrawForm (frmP);				
	PIMAppProfilingEnd();

	DrawMonth (sMonthP, drawDaySelected );
}

	
/***********************************************************************
 *
 * FUNCTION:    	MonthViewUpdateDisplayCallback
 *
 * DESCRIPTION:		Window Manager Callback to redraw invalidated
 *					rectangles.
 *
 * PARAMETERS: 	-> 	cmd:		Invalidation command word.
 *				->	window:		Invalidated window.
 *				->	dirtyRect: 	Invalidated Rectangle.
 *				->	state:		Select day
 *
 * RETURNED:   		Boolean that tells if invalidated rectangle was drawn
 *					(true) or not (false.)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/21/03	Initial revision.
 * 	PPL	07/21/03	Implementation.
 *
 ***********************************************************************/
static Boolean MonthViewUpdateDisplayCallback(int32_t cmd, WinHandle window, const RectangleType *dirtyRect, void *state)
{
	Boolean selectDay;
	
	if (cmd == winInvalidateDestroy)
		return true;

	selectDay = *((Boolean*) &state);

	if (ApptDB)
	{
		// we only tries to refresh if the database is opened
		// the database is closed when we are changing the level of security

		MonthViewUpdateDisplay(sFormP, selectDay );
	}
	
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewDisplayInvalidate
 *
 * DESCRIPTION:		Calls WinInvalidateRectFunc() with  
 *					WeekViewDrawMonthCallback()
 *
 * PARAMETERS: 	-> 	dirtyRectP:		Invalidated Rectangle.
 *				->	selectDay:		Day selection indicator.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			Uses gWinHdl, sMonthP,  sFormP  
 *					static global variables.
 *
 * REVISION HISTORY:
 *	PPL	07/21/03	Initial revision.
 * 	PPL	07/21/03	Implementation.
 *	PPL 07/28/03	Change parameters list.
 *
 ***********************************************************************/
static void MonthViewDisplayInvalidate (RectangleType* dirtyRectP, Boolean selectDay)
{
	RectangleType 	dirtyRect;


	if (dirtyRectP == NULL)
		WinGetBounds (gWinHdl, &dirtyRect);
	else
	{
		dirtyRect = *dirtyRectP;
		WinScaleRectangleNativeToActive(&dirtyRect);
		RctInsetRectangle(&dirtyRect, -1);
	}
	
	//Transparency
	//WinInvalidateRectFunc(gWinHdl, &dirtyRect, MonthViewUpdateDisplayCallback, (void*) selectDay);
	WinInvalidateRect(gWinHdl, &dirtyRect);
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewSetSolarTitle
 *
 * DESCRIPTION: 	This routine set the Solar title of the Month View.
 *					(Solar = Gregorian calendar)
 *					(Lunar = Chinese Lunar Calendar)
 *
 * PARAMETERS: 	->	iResourceDbP: 	Database resource open ref.
 *				-> 	monthP:			Pointer to month object (MonthType.)
 *				<-> title:			Must be valid on entry.
 *									On exit contain the form's title.
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			titleSize have to include space for terminal NULL 
 *					to end string.
 *
 * REVISION HISTORY:
 *	PPL 03/15/04	Inital revision. Splitted from MonthViewSetTitle.
 *	PPL	03/15/04 	Port Lunar Calendar form Palm OS 5.4
 *
 ***********************************************************************/
void MonthViewSetSolarTitle(DmOpenRef iResourceDbP, MonthPtr monthP, char* title, uint16_t titleSize)
{
	MemHandle	templateH;
	char		*templateP;

	templateH = DmGetResource (gApplicationDbP, strRsc, MonthViewTitleStrID);
			
	templateP = (char*)MemHandleLock(templateH);
			
	DateTemplateToAscii(templateP, (uint8_t) monthP->month, 1, monthP->year, title, titleSize);
			
	MemHandleUnlock(templateH);
			
	DmReleaseResource(templateH);	
			
	FrmCopyTitle (sFormP, title);
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewSetLunarTitle
 *
 * DESCRIPTION: 	This routine set the Lunar title of the Month View.
 *					(Solar = Gregorian calendar)
 *					(Lunar = Chinese Lunar Calendar)
 *
 * PARAMETERS: 	->	iResourceDbP: 	Database resource open ref
 *				-> 	monthP:			Pointer to month object (MonthType.)
 *				<-> title:			Must be valid on entry.
 *									On exit contain the form's title.
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			titleSize have to include space for terminal NULL 
 *					to end string.
 *
 * REVISION HISTORY:
 *	PPL 03/15/04	Inital revision. Splitted from MonthViewSetTitle.
 *	PPL	03/15/04 	Port Lunar Calendar form Palm OS 5.4
 *	CSc	2004-07-21	Fixed Gregorian month search to also check last day
 *					of the month when looking for the first lunar month
 *					beginning within the Gregorian month.  Fix for
 *					BUG50533.
 *
 ***********************************************************************/
void MonthViewSetLunarTitle(DmOpenRef iResourceDbP, MonthPtr monthP, char* title, uint16_t titleSize)
{
	DateTimeType	dateTime;
	uint32_t		dateSeconds;
	uint16_t		lastDay;
	uint16_t		lunarYear = 0;
	uint16_t		lunarMonth = 0;
	uint16_t		lunarDay;
	Boolean 		isLeapMonth = false;
	Err 			err = errNone;
	

	dateTime.second = 0;
	dateTime.minute = 0;
	dateTime.hour = 0;
	dateTime.month = monthP->month;
	dateTime.year = monthP->year;
	
	// Find the date on which the first lunar month within this Gregorian month
	// begins.  If no such date exists, then just use the lunar month of the last
	// day in this Gregorian month (same as the lunar month for the rest of the
	// days in this Gregorian month).
	
	lastDay = DaysInMonth(monthP->month, monthP->year);
	for (dateTime.day = 1 ; dateTime.day <= lastDay ; dateTime.day++)
	{
		dateSeconds = TimDateTimeToSeconds(&dateTime);
		err = DateSecondsToLunarDate(	iResourceDbP,
										dateSeconds,
										&lunarYear,
										&lunarMonth,
										&lunarDay,
										&isLeapMonth);
		if (err)
		{
			ErrNonFatalDisplay("Error converting seconds to lunar date");
			return;
		}
		
		if (lunarDay == timFirstChineseLunarDay)
		{
			break;
		}
	}
	
	err = DateToLunarStr(	iResourceDbP,
							lunarYear,
							lunarMonth,
							timOmitLunarDay,
							isLeapMonth,
							title,
							titleSize);
	if (err)
	{
		ErrNonFatalDisplay("Error formatting lunar date string");
	}
}

/***********************************************************************
 *
 * FUNCTION:    	MonthViewSetTitle
 *
 * DESCRIPTION: 	This routine set the title of the Month View.
 *
 * PARAMETERS: 	-> 	monthP:	pointer to month object (MonthType.)
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	04/10/96	Initial revision.
 *	KWK	09/08/99	Use DateTemplateToAscii & load format from string 
 *					resource.
 *	JMP	10/6/99		Replace FrmGetActiveForm() with FrmGetFormPtr();
 *	CS 10/23/04		Set title to text describing lunar month for Lunar
 *					View in Palm OS 5.X
 *	PPL 03/15/04	Port Lunar Calendar form Palm OS 5.x to Palm OS 6.x.
 *
 ***********************************************************************/
static void MonthViewSetTitle (MonthPtr monthP)
{
	char title [longDateStrLength];
	
	if (MonthDrawLunarCalendar())
	{
		MonthViewSetLunarTitle(gApplicationDbP, monthP, title, longDateStrLength-1);
	}
	else
	{
		MonthViewSetSolarTitle(gApplicationDbP, monthP, title, longDateStrLength-1);
	}
	
	FrmCopyTitle (sFormP, title);
	
	TimeDisplayed = false;

	// Set the label of the current category
	CatMgrSetTriggerLabel(
			ApptDB, 
			DateBkCurrentCategoriesP, 
			DateBkCurrentCategoriesCount,
			FrmGetObjectPtr(sFormP, FrmGetObjectIndex(sFormP, MonthCategoryTrigger)), 
			DateBkCategoriesName);

	// Now get the new month color and bitmap if there is one
	DateBackgroundGetHighlightColor(&sObjectMonthHighlight, monthP->month);
	sGadgetCallbackData.color = sObjectMonthHighlight;
	DateBackgroundGetBitmapID(&sGadgetCallbackData.bitmapID, monthP->month);
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewHideTime
 *
 * DESCRIPTION: 	If the title of the Day View is displaying the 
 *					current time, this routine will change the title 
 *					to the standard title (the current date).
 *
 * PARAMETERS:  ->	hide:	true to always hide, false hide only if
 *                     		to time has been display for the require
 *                     	 	length of time.
  *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		5/14/96	Initial revision.
 *	GGL		2/2/99	Use TimeToWait(), don't use EvtSetNullEventTick().
 *
 ***********************************************************************/
static void MonthViewHideTime (Boolean hide)
{	
	if (TimeDisplayed)
	{
		if (hide || TimeToWait() == 0)
			MonthViewSetTitle (sMonthP);
	}	
	
	TimeComputeWaitTime = false;
}


/***********************************************************************
 *
 * FUNCTION:   	 	MonthFindDay
 *
 * DESCRIPTION: 	This routine find the day where the pen was tapped.
 *					if any.
 *
 * PARAMETERS:	-> 	monthP:	pointer to control object (ControlType)
 *              ->	eventP:	pointer to an EventType structure.
 *              <->	dateP:	pointer to the date selected
 *
 * RETURNED:		true if the event was handle or false if it was not.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/08/03	Initial revision.
 *
 ***********************************************************************/
static Boolean MonthFindDayCell (MonthPtr monthP, Coord x, Coord y)
{
	int16_t       	daysInMonth;
	int16_t       	firstDayInMonth;
	int16_t			cellWidth;
	int16_t			cellHeight;
	
	x = WinUnscaleCoordActiveToNative(x, false);
	y = WinUnscaleCoordActiveToNative(y, false);

	firstDayInMonth = FirstDayOfMonth ( monthP);
	daysInMonth = DaysInMonth(monthP->month, monthP->year);
	
	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonth;
	
	monthP->bounds.extent.x = cellWidth * daysInWeek;
	monthP->bounds.extent.y = cellHeight * linesInMonth;

	RctCopyRectangle (&(monthP->bounds), &sSelectedDayCellBounds);

	sDayCellTracking = RctPtInRectangle (x, y, &sSelectedDayCellBounds);
	if (!sDayCellTracking)
		return sDayCellTracking;

	// Compute the bounds of the day the pen is on.
	sDayCellSelectedGridIndex = MapPointToItem (x, y, &monthP->bounds);
	
	sSelectedDayCellBounds.topLeft.x = monthP->bounds.topLeft.x + (sDayCellSelectedGridIndex % daysInWeek) * cellWidth + 1;
	sSelectedDayCellBounds.topLeft.y = monthP->bounds.topLeft.y + (sDayCellSelectedGridIndex / daysInWeek) * cellHeight + 1;
	sSelectedDayCellBounds.extent.x = cellWidth - 1;
	sSelectedDayCellBounds.extent.y = cellHeight - 1;

	sDayCellSelected = true;


	// determine the cell day Date
	sDayCellSelectedDate.month = monthP->month;
	sDayCellSelectedDate.year = monthP->year - firstYear;
	
	// Is the selection is outside of the month?
	if ((unsigned) (sDayCellSelectedGridIndex - firstDayInMonth) >= (unsigned) daysInMonth)
	{
		// Move to the month before or after the current month.
		sDayCellSelectedDate.day = 1;
		DateAdjust (&sDayCellSelectedDate, (int32_t)(sDayCellSelectedGridIndex - firstDayInMonth));
	}
	else
	{
		sDayCellSelectedDate.day = sDayCellSelectedGridIndex - firstDayInMonth + 1;
	}

	MonthViewDisplayInvalidate (&sSelectedDayCellBounds, sDayCellSelected);

	return sDayCellSelected;
}

/***********************************************************************
 *
 * FUNCTION:   	 	MonthFindDay
 *
 * DESCRIPTION: 	This routine find the day where the pen was tapped.
 *					if any.
 *
 * PARAMETERS:	-> 	monthP:	pointer to control object (ControlType)
 *              ->	eventP:	pointer to an EventType structure.
 *              <->	dateP:	pointer to the date selected
 *
 * RETURNED:		true if the event was handle or false if it was not.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/08/03	Initial revision.
 *
 ***********************************************************************/

static Boolean MonthViewTrackSelectedDayCell(MonthPtr monthP, Coord x, Coord y)
{
	x = WinUnscaleCoordActiveToNative(x, false);
	y = WinUnscaleCoordActiveToNative(y, false);

	if (sDayCellTracking)
	{
		// Day cell tracking is on going
		if (RctPtInRectangle (x, y, &sSelectedDayCellBounds))
		{
			if (!sDayCellSelected)
			{
				sDayCellSelected = true;
				MonthViewDisplayInvalidate (&sSelectedDayCellBounds, sDayCellSelected);
			}
		}
		else if (sDayCellSelected)
		{
			sDayCellSelected = false;
			MonthViewDisplayInvalidate ( &sSelectedDayCellBounds, sDayCellSelected);
		}
	}

	return sDayCellTracking;
}

/***********************************************************************
 *
 * FUNCTION:   	 	MonthViewSelectDayCell
 *
 * DESCRIPTION: 	Select day cell where x,y is.
 *
 * PARAMETERS:	-> 	monthP:	pointer to control object (ControlType)
 *              ->	x:		x coord.
 *              ->	y:		y coord.
 *
 * RETURNED:		true if the event was handle or false if it was not.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/08/03	Initial revision.
 *
 ***********************************************************************/
static Boolean MonthViewSelectDayCell(MonthPtr monthP, Coord x, Coord y)
{
	x = WinUnscaleCoordActiveToNative(x, false);
	y = WinUnscaleCoordActiveToNative(y, false);

	if (sDayCellTracking)
	{
		if (RctPtInRectangle (x, y, &sSelectedDayCellBounds))
		{
			sDayCellSelected = false;
			MonthViewDisplayInvalidate ( &sSelectedDayCellBounds, sDayCellSelected);

			// selection action
			Date = sDayCellSelectedDate;
			FrmGotoForm(gApplicationDbP, DayView);
		}

		// Reset Day Cell tracking
		sDayCellTracking = false;
		sDayCellSelected = false;
		sDayCellSelectedGridIndex =(uint16_t) -1;
		RctSetRectangle(&sSelectedDayCellBounds,0,0,0,0);
		
		return true;
	}
	
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewShowTime
 *
 * DESCRIPTION: 	This routine display the current time in the title 
 *					of the month view.
 *
 * PARAMETERS:  ->	frm:	pointer to the day view form.
 *
 * RETURNED:   	 	None..
 *
 * NOTE:        	The global variable TimeDisplayed is set to true 
 *					by this routine.
 *
 * REVISION HISTORY:
 *	ART	7/15/96		Initial revision.
 *	JMP	10/6/99		Replace FrmGetActiveForm() with FrmGetFormPtr();
 *
 ***********************************************************************/
static void MonthViewShowTime (void)
{
	char			title[timeStringLength];
	DateTimeType 	dateTime;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	TimeToAscii ((uint8_t) dateTime.hour, (uint8_t) dateTime.minute, TimeFormat, title);
	
	//FrmCopyTitle (FrmGetFormPtr (MonthView), title);
	FrmCopyTitle (sFormP, title);
	
	TimeDisplayed = true;
	TimeDisplayTick = TimGetTicks() + timeDisplayTime;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewGotoDate
 *
 * DESCRIPTION:		This routine displays the date picker so that the 
 *					user can select a month to navigate to.  If the date
 *					picker is confirmed, the month selected month is 
 *                	confirmed
 *
 *					This routine is called when a "go to" button is 
 *					pressed.
 *              
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:  			None.
 *
 * REVISION HISTORY:
 *	ART	5/20/96		Initial revision.
 *	JMP	10/31/99 	Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *					into DrawMonth() itself.
 *	PPL	21/07/03	Uses MonthViewUpdateDisplayInvalidate 
 *					in place of DrawMonth.
 *
 ***********************************************************************/
static void MonthViewGotoDate (void)
{
	char *		title;
	MemHandle	titleH;
	int16_t 	day;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (gApplicationDbP, strRsc, goToDateTitleStrID);
	
	title = MemHandleLock (titleH);
	

	// Display the date picker.
	day = 1;
	if (SelectDay (selectDayByMonth, &sMonthP->month, &day, &sMonthP->year, title))
	{
		MonthCursorOpenOrRequery(sMonthP);
		MonthViewSetTitle (sMonthP);
		MonthViewDisplayInvalidate (NULL, drawDaySelected);
	}

	MemHandleUnlock (titleH);
   	DmReleaseResource (titleH);   
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewScroll
 *
 * DESCRIPTION: 	This routine scrolls the month view foreward or 
 *					backwards by one month.
 *
 * PARAMETERS:  ->	monthP  - pointer to month object (MonthType)
 *              ->	direction - winUp (backwards) or winDown (foreward)
 *
 * RETURNED:	 	nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/8/96		Initial revision.
 *	JMP	10/31/99 	Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *					into DrawMonth() itself.
 *	PPL	21/07/03	Uses MonthViewUpdateDisplayInvalidate 
 *					in place of DrawMonth.
 *
 ***********************************************************************/
static void MonthViewScroll (MonthPtr monthP, WinDirectionType direction)
{
	// Move backward one month.
	if (direction == winUp)
	{
		if (monthP->month > january)
		{
			monthP->month--;
		}
		else if (monthP->year > firstYear)
		{
			monthP->month = december;
			monthP->year--;
		}
		else
		{
			return;
		}
	}
	// Move foreward one month.
	else
	{
		if (monthP->month < december)
		{
			monthP->month++;
		}
		else if (monthP->year < lastYear)
		{
			monthP->month = january;
			monthP->year++;
		}
		else
		{
			return;
		}
	}
	
	MonthViewSetTitle (monthP);
	
	MonthCursorOpenOrRequery(monthP);
	
	MonthViewDisplayInvalidate (NULL, drawDaySelected);
}



/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusSetRing
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusSetRing(MonthPtr monthP)
{
	RectangleType 	focusZone; 
	uint16_t		currentCoordSystem;

	focusZone = monthP->bounds;

	

	// fine tune the size of the rectangle
	++(focusZone.extent.x);
	++(focusZone.extent.y);

	//RctInsetRectangle(&focusZone, -1);

 	currentCoordSystem = WinSetCoordinateSystem(kCoordinatesNative);

	FrmSetFocusHighlight(gWinHdl, &focusZone, 0); 

	WinSetCoordinateSystem(currentCoordSystem);
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusClearRing
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusClearRing(void)
{
	FrmClearFocusHighlight();
}


/***********************************************************************
 *
 * FUNCTION:		MonthFocusActivate
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusActivate(MonthPtr monthP)
{
	EventType 		newEvent;
	RectangleType 	focusZone;

	// send a penDown Event
	memset(&newEvent, 0x00, sizeof(newEvent));

	MapCellNum2Rectangle (monthP, monthP->focusedCellNum, &focusZone);
	
	newEvent.screenX = focusZone.topLeft.x + (focusZone.extent.x >> 1);
	newEvent.screenY = focusZone.topLeft.y + (focusZone.extent.y >> 1);

	newEvent.screenX = WinScaleCoordNativeToActive(newEvent.screenX, false);
	newEvent.screenY = WinScaleCoordNativeToActive(newEvent.screenY, false);

	newEvent.eType = penDownEvent;
	EvtAddEventToQueue (&newEvent);

	newEvent.eType = penUpEvent;
	EvtAddEventToQueue (&newEvent);
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewSetFocusedDayCell
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *				->	modifiers:		Command mofifiers.
 *				-> 	chr:			Command code.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void  MonthViewSetFocusedDayCell(MonthPtr monthP, int16_t focusedCellNum, Boolean redraw)
{
	RectangleType 	focusZone;
	uint16_t		currentCoordSystem;
	
	switch(monthP->focusState)
	{
		case kFocusMonthNoFocus:
			break;
					
		case kFocusMonthFocused:
			break;
				
		case kFocusMonthCellFocused:
			if (monthP->focusedCellNum == (int16_t) kFocusMonthNoFocus &&  focusedCellNum == (int16_t) kFocusMonthNoFocus)
			{
				if (sMonthP->selected.month == monthP->month)
				{
					GetDayBounds (sMonthP, sMonthP->selected.day, &focusZone);

					currentCoordSystem = WinSetCoordinateSystem(kCoordinatesStandard);

					WinScaleRectangle(&focusZone);
					
					monthP->focusedCellNum = MapCellRectangle2Num (sMonthP, &focusZone);

					currentCoordSystem = WinSetCoordinateSystem(currentCoordSystem);
				}
				else
				{
					// go somethere in the month
					monthP->focusedCellNum = kFocusMonthGridMidCell ;
				}
			}
				
			if (focusedCellNum != (int16_t) kFocusMonthNoFocus)
				monthP->focusedCellNum = focusedCellNum;

			MapCellNum2Rectangle (sMonthP, monthP->focusedCellNum, &focusZone);
		
			currentCoordSystem = WinSetCoordinateSystem(kCoordinatesNative);

			FrmSetFocusHighlight(gWinHdl, &focusZone, 0);
			
			WinSetCoordinateSystem(currentCoordSystem);
			
			if (redraw)
			{
				// redraw
				MonthViewDisplayInvalidate (&focusZone, drawDaySelected);
			}
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:		MonthFocusActivate
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusReset(MonthPtr monthP)
{
	// Month gadget still own the focus ring
	monthP->focusState = kFocusMonthFocused;
}


/***********************************************************************
 *
 * FUNCTION:		MonthFocusLost
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusLost(MonthPtr monthP)
{
	MonthViewFocusReset(monthP);
	
	monthP->focusState = kFocusMonthNoFocus;

	monthP->focusedCellNum = (int16_t) kFocusMonthNoFocus;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusTake
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusTake(MonthPtr monthP)
{
	FormType*	formP;

	formP = FrmGetFormPtr(MonthView);
	
	FrmSetFocus(formP, FrmGetObjectIndex(formP, MonthGadget));

	if  (monthP->focusState == kFocusMonthNoFocus)
		monthP->focusState = kFocusMonthFocused;

	switch (monthP->focusState)
	{
		case kFocusMonthNoFocus:
		case kFocusMonthFocused:
			MonthViewFocusSetRing(monthP);
			break;
			
		case kFocusMonthCellFocused:
			MonthViewSetFocusedDayCell( monthP, sMonthP->focusedCellNum, true);
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusUpDayCell
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:		Month View data.
 *				->  focus:  	Current form focused widget index.
 *				->	modifiers:	event modifiers.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean MonthViewFocusUpDayCell(MonthPtr monthP, uint16_t focus, uint16_t modifiers)
{
	int16_t	focusedCell;
	Boolean handled = false;

	
	if (focus == noFocus)
	{
		// Focus Month Gadget
		// on left key press when focusing is not enabled
		// focus week view gadget
		MonthViewFocusLost(monthP);
		FrmNavObjectTakeFocus(sFormP, MonthGadget);
		handled = true;
		return handled;
	}


	switch(monthP->focusState)
	{
		case kFocusMonthNoFocus:
			break;
				
		case kFocusMonthFocused:
			break;
				
		case kFocusMonthCellFocused:
			focusedCell = sMonthP->focusedCellNum;
			focusedCell -= daysInWeek;

			if (focusedCell < 0)
			{
				return	handled;
			}

			MonthViewSetFocusedDayCell(monthP, focusedCell, true);
			handled = true;
			break;
	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusDownDayCell
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:		Month View data.
 *				->  focus:  	Current form focused widget index.
  *				->	modifiers:	event modifiers.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean MonthViewFocusDownDayCell(MonthPtr monthP, uint16_t focus, uint16_t modifiers)
{
	int16_t	focusedCell;
	Boolean handled = false;


	if (focus == noFocus)
	{
		// Focus Statusbar
		// on left key press when focusing is not enabled
		// focus status bar, so we give th efocus to goto button
		// whithout telling we handled the down key 
		// no navigation manager will handle it again 
		// as a result statusbar will be focused
		MonthViewFocusLost(monthP);
		FrmNavObjectTakeFocus(sFormP, MonthGoToButton);
		return handled;
	}
	

	switch(monthP->focusState)
	{
		case kFocusMonthNoFocus:
			break;
				
		case kFocusMonthFocused:
			break;
				
		case kFocusMonthCellFocused:
			focusedCell = sMonthP->focusedCellNum;
			focusedCell += daysInWeek;

			if (focusedCell >=  maxDayCellsInMonthGrid)
			{
				return	handled;
			}

			MonthViewSetFocusedDayCell(monthP, focusedCell, true);
			handled = true;
			break;
	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusRightDayCell
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:		Month View data.
 *				->  focus:  	Current form focused widget index.
 *				->	modifiers:	event modifiers.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean MonthViewFocusRightDayCell(MonthPtr monthP, uint16_t focus, uint16_t modifiers)
{
	int16_t	focusedCell;
	Boolean handled = false;


	if (focus == noFocus)
	{
		// Focus Goto Button
		// on right key press when focusing is not enabled
		// focus Goto button
		MonthViewFocusLost(monthP);
		FrmNavObjectTakeFocus(sFormP, MonthGoToButton);
		handled = true;
	
		return handled;
	}

	

	switch(monthP->focusState)
	{
		case kFocusMonthNoFocus:
			break;
				
		case kFocusMonthFocused:
			// go to next month
			MonthViewScroll (sMonthP, winDown);
			handled = true;
			break;
				
		case kFocusMonthCellFocused:
			focusedCell = sMonthP->focusedCellNum;

			if (focusedCell >= maxDayCellsInMonthGrid 
			|| ((focusedCell % daysInWeek)+1) == daysInWeek )
			{
				// when a cell is the last column is selected ,
				// going left gives focus to the goto Button
				MonthViewFocusLost(monthP);
				FrmNavObjectTakeFocus(sFormP, MonthGoToButton);
				handled = true;
				return	handled;
			}

			focusedCell += 1;

			MonthViewSetFocusedDayCell(monthP, focusedCell, true);
			handled = true;
			break;
	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusLeftDayCell
 *
 * DESCRIPTION: 	
 *
 * PARAMETERS:	->	monthP:		Month View data.
 *				->  focus:  	Current form focused widget index.
  *				->	modifiers:	event modifiers.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean MonthViewFocusLeftDayCell(MonthPtr monthP, uint16_t focus, uint16_t modifiers)
{
	int16_t					focusedCell;
	Boolean 				handled = false;

	
	if (focus == noFocus)
	{
		// Focus Month Gadget
		// on left key press when focusing is not enabled
		// focus week view gadget
		MonthViewFocusLost(monthP);
		FrmNavObjectTakeFocus(sFormP, MonthGadget);
		handled = true;
		return handled;
	}

	switch(monthP->focusState)
	{
		case kFocusMonthNoFocus:
			break;
				
		case kFocusMonthFocused:
			// go to previous month
			MonthViewScroll (sMonthP, winUp);
			handled = true;
			break;

				
		case kFocusMonthCellFocused:
			focusedCell = sMonthP->focusedCellNum;
			
			if ( focusedCell <= 0 
			|| (focusedCell % daysInWeek) == 0)
			{
				// when a cell is the first column  is selected ,
				// going left gives focus back to the month view gadget
				MonthViewFocusLost(monthP);
				FrmNavObjectTakeFocus(sFormP, MonthGadget);
				handled = true;
				return	handled;
			}

			focusedCell -= 1;

			MonthViewSetFocusedDayCell(monthP, focusedCell, true);
			handled = true;
			break;
	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusFirst
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *
 ***********************************************************************/
static void MonthViewFocusFirst(MonthPtr monthP)
{
	// Focus is starting from currently active tab when visible
	// or from the first visible tab if active tab is out of screen.

	switch(monthP->focusState)
	{
		case kFocusMonthNoFocus:
			break;
				
		case kFocusMonthFocused:
			break;
				
		case kFocusMonthCellFocused:
			// we want to focus:
			// 1 - last focused day cell if there was one 
			// 2 - current day 
	
			MonthViewSetFocusedDayCell(monthP, (int16_t)noFocus, true);
			break;
	}
}



/***********************************************************************
 *
 * FUNCTION:		MonthViewFocusEnter
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean MonthViewFocusEnter(MonthPtr monthP, uint16_t focus, uint16_t modifiers)
{
	FormType*				formP;
	FrmNavStateFlagsType 	navStateFlags = 0;
	Boolean 				handled = false;
	Boolean 				exitView = false;
	status_t				err;


	formP = FrmGetFormPtr(MonthView);
	err = FrmGetNavState (formP, &navStateFlags);

	if (focus == noFocus && !navStateFlags)
	{
		// when there no focus and navigation is not yet enabled
		FrmNavObjectTakeFocus(formP, MonthGadget);
		handled = true;
		return handled;
	}
	
	if (navStateFlags & kFrmNavStateFlagsObjectFocusMode)
	{
		// test againts Month solar or Lunar push button
		if (focus == FrmGetObjectIndex(formP, sMonthViewButtonID))
		{
			FrmNavObjectTakeFocus(formP, MonthGadget);
			handled = true;
			return handled;
		}
	}

	switch(monthP->focusState)
	{
		default:
		case kFocusMonthNoFocus:
			break;
			
		case kFocusMonthFocused:
			// Month gadget is already focused
			monthP->focusState = kFocusMonthCellFocused;
		
			// Month gadget was focused
			MonthViewFocusFirst(monthP);
			handled = true;
			break;
			
		case kFocusMonthCellFocused:
			MonthViewFocusActivate(monthP);
			// the effect of activating a cell we be to send a pendown 
			//as if user tap on the cell with the stylus - doing so 
			// the cell will be highlighted and user will be redirected
			// toward Day View for related day.
			
			// there no need to give back the focus to the entire month gadget
			// MonthViewFocusTake(monthP);
			handled = true;
			break;
	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewDoCommand
 *
 * DESCRIPTION: 	This routine preforms the menu command specified.
 *
 * PARAMETERS:  ->	command:	menu item id.
 *
 * RETURNED:    	true when menu command was handled.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	7/17/96		Initial Revision
 *	jmp	10/31/99 	Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *					into DrawMonth() itself.
 *
 ***********************************************************************/
static Boolean MonthViewDoCommand (uint16_t command)
{
	Boolean	handled = false;

	switch (command)
	{
		// Preferences
		case MonthPreferencesCmd:
			MenuEraseStatus(0);
			FrmPopupForm(gApplicationDbP, PreferencesDialog);
			handled = true;
			break;

		// Display Options
		case MonthDisplayOptionsCmd:
			MenuEraseStatus(0);
			FrmPopupForm(gApplicationDbP, DisplayDialog);
			handled = true;
			break;

		// Security
		case MonthSecurityCmd:
			MenuEraseStatus(0);
			DoSecurity();
			handled = true;
			break;

		// About
		case MonthGetInfoCmd:
			MenuEraseStatus(0);
			AbtShowAbout (sysFileCDatebook);
			handled = true;
			break;
	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewSetUpMonth
 *
 * DESCRIPTION: 	This routine resizes the Gadget Month View Data.
 *
 * PARAMETERS:  ->	frm:	pointer of the Month View form.
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ppl	01/31/02	Initial Revision.
 *
 ***********************************************************************/
static void MonthViewSetUpMonth (FormPtr formP)
{
	uint16_t	cellWidth;
	uint16_t	cellHeight;
	uint16_t 	objIndex;
	int16_t 	width;
	int16_t 	newWidth;
		
	if (sMonthP)
	{
		objIndex = FrmGetObjectIndex (formP, MonthGadget);
		FrmGetObjectBounds (formP, objIndex, &sMonthP->bounds);
		
		width = WinUnscaleCoordActiveToNative(sMonthP->bounds.extent.x, false);
		
		newWidth = sMonthP->bounds.extent.x = (width / daysInWeek * daysInWeek) + 1;
		sMonthP->bounds.topLeft.x = (width - newWidth) / 2;

		cellWidth = newWidth / daysInWeek;
		cellHeight = WinUnscaleCoordActiveToNative(sMonthP->bounds.extent.y, false)
		   	/ linesInMonth;
	
		sMonthP->bounds.topLeft.y = WinUnscaleCoordActiveToNative(sMonthP->bounds.topLeft.y, false);
		sMonthP->bounds.extent.x = cellWidth * daysInWeek;
		sMonthP->bounds.extent.y = cellHeight * linesInMonth;

		sMonthP->cellWidth = cellWidth;
		sMonthP->cellHeight= cellHeight;

		TimSecondsToDateTime(TimGetSeconds(), &sMonthP->selected);

		if (sMonthP->focusState != kFocusMonthNoFocus 
		&& sMonthP->focusedCellNum != (int16_t) kFocusMonthNoFocus)
			MonthViewSetFocusedDayCell( sMonthP, sMonthP->focusedCellNum, true);

		MonthViewSetTitle (sMonthP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewInitMonth
 *
 * DESCRIPTION: 	This routine resizes the Gadget Month View Data.
 *
 * PARAMETERS:  ->	frm:	pointer of the Month View form.
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ppl	01/31/02	Initial Revision.
 *
 ***********************************************************************/
static void MonthViewInitMonth (FormPtr formP)
{
	//if (!sMonthP)
	{
		// Create a month object, initialize it, and store a pointer to it in the 
		// month gadget of the Month View.
		sMonthP = MemPtrNew (sizeof (MonthType));
		ErrFatalDisplayIf(!sMonthP, "Month Ptr not Allocated");
			
		if (sMonthP)
		{		
			sMonthP->month = Date.month;
			sMonthP->year = Date.year + firstYear;

			sMonthP->focusState = kFocusMonthNoFocus;
			sMonthP->focusedCellNum = (int16_t) kFocusMonthNoFocus;
		}

		MonthViewSetUpMonth(formP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewFormResize
 *
 * DESCRIPTION: 	This routine resizes the Month View.
 *
 * PARAMETERS:  ->	frm - pointer of the Month View form.
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	01/31/02	Initial revision.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *	PPL 05/04/04	Add Landscape Mode.
 *
 ***********************************************************************/
static void MonthViewFormResize (EventType* eventP)
{
	RectangleType 	objBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	Coord			offsetX;
	Coord			offsetY;
	Coord			width;
	Coord			height;
	FormType* 		frmP;

	width = eventP->data.winResized.newBounds.extent.x;
	height = eventP->data.winResized.newBounds.extent.y;
	
	frmP = sFormP;
	
	offsetY = height - gCurrentWinBounds.extent.y;
	offsetX = width  - gCurrentWinBounds.extent.x;

	if (!offsetX  && !offsetY)
	{	
		// offset X and OffsetY are both zero
		// if the window has the height of the screen
		// then the windows is already at the right size
		goto justSet;
	}

	if (offsetX)
	{
		// MonthCategoryTrigger, moves on x not on y
		objIndex =  FrmGetObjectIndex (frmP, MonthCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// MonthCategoryList, moves on x not on y
		objIndex =  FrmGetObjectIndex (frmP, MonthCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	 }

	// MonthGadget , does not move resized on both x and y
	objIndex =  FrmGetObjectIndex (frmP, MonthGadget);
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x +=   offsetX;
	objBounds.extent.y +=   offsetY;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	// BackgroundGadget
	objIndex =  FrmGetObjectIndex (frmP, MonthBackgroundGadget);
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x +=   offsetX;
	objBounds.extent.y +=   offsetY;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	if (offsetY)
	{
		// MonthDayViewButton, moves on y not on x
		objIndex =  FrmGetObjectIndex (frmP, MonthDayViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		
		// MonthWeekViewButton, moves on y not on x
		objIndex =  FrmGetObjectIndex (frmP, MonthWeekViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// MonthMonthLunarViewButton for Lunar Calendar
		if (DateSupportsLunarCalendar()) 
		{
			objIndex =	FrmGetObjectIndex (frmP, MonthMonthLunarViewButton);
			FrmGetObjectPosition (frmP, objIndex, &x, &y);
			y +=   offsetY;
			FrmSetObjectPosition (frmP, objIndex, x, y);
		}
	

		// MonthMonthViewButton, moves on y not on x
		objIndex =  FrmGetObjectIndex (frmP, MonthMonthViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// MonthAgendaViewButton, moves on y not on x
		objIndex =  FrmGetObjectIndex (frmP, MonthAgendaViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// MonthGoToButton, moves on y not on x
		objIndex =  FrmGetObjectIndex (frmP, MonthGoToButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	// MonthPrevButton, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, MonthPrevButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x +=   offsetX;
	y +=   offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// MonthNextButton, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, MonthNextButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x +=   offsetX;
	y +=   offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// keep the window bounding rect.
	gCurrentWinBounds = eventP->data.winResized.newBounds;
	
justSet:
	// finish initializations due to gadget month view resizing
	MonthViewSetUpMonth(frmP);
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewSelectCategory
 *
 * DESCRIPTION: 	Handles Categories Selection.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	07/29/02	Initial revision, added category support.
 *	PPl	21/07/03	Uses MonthViewUpdateDisplayInvalidate 
 *					in place of DrawMonth.
 *
 ***********************************************************************/
static void MonthViewSelectCategory(void)
{
	Boolean 		categoryEdited;
	CategoryID* 	newCatSelectionP = NULL;
	uint32_t		newCatSelectionNum = 0;
	status_t		err;

	// Close all opened cursors
	// (main cusor and month view cursor)
	// we have to close cursors as Datebook is using cahed cursors
	// unfortunaltely when a record is in such loaded in such a cache
	// when CatMgrSelectFilter is called, it cannot update records.
	
	err = ApptCloseCursor(&gApptCursorID);
	err = ApptCloseCursor(&gMonthViewCursorID);
	
	categoryEdited = CatMgrSelectFilter(
									ApptDB, 
									sFormP, 
									MonthCategoryTrigger,
									DateBkCategoriesName, 
									MonthCategoryList, 
									DateBkCurrentCategoriesP, 
									DateBkCurrentCategoriesCount, 
									&newCatSelectionP, 
									&newCatSelectionNum,
									true, 
									NULL);			
									
	if (categoryEdited)
	{
		// Update current categories
		ChangeCurrentCategories(newCatSelectionNum, newCatSelectionP);
				
		// Free list allocated by CatMgrSelectFilter
		CatMgrFreeSelectedCategories(ApptDB, &newCatSelectionP);

		err = MonthCursorOpenOrRequery(sMonthP);
			
		MonthViewDisplayInvalidate (NULL, drawDaySelected);
	}
}

 /***********************************************************************		
 *
 * FUNCTION:    	MonthViewInitLunarCalendar
 *
 * DESCRIPTION: 	This routine initializes the "Month View" Lunar of the 
 *              	Datebook application.
 *
 * PARAMETERS:  ->	frm:	pointer to the Month view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	03/10/04	Lunar Calendar from PalmOS 5.4.
 *	PPL 08/05/04	Add True Type font for Lunar Month Day Num/
 *
 ***********************************************************************/
void MonthViewInitLunarCalendar(FormPtr formP)
{
	sDaySmallFont = GcCreateFont("7db palmos-plain");
	GcGetFontHeight(sDaySmallFont, &sDaySmallFontHeight);

	sCurrentDaySmallFont = GcCreateFont("9db palmos-bold");
	GcGetFontHeight(sCurrentDaySmallFont, &sCurrentDaySmallFontHeight);
	
 	// Display the Lunar View push button if we've got support for it.
	if (DateSupportsLunarCalendar()) 
	{
		FrmShowObject (formP, FrmGetObjectIndex(formP, MonthMonthLunarViewButton));

		sTextLeftMargin = dateTextLeftMarginLunarView;
	 	sTextTopMargin = dateTextTopMarginLunarView;
	 }	
	 else
	 {
	 	sTextLeftMargin = dateTextLeftMarginMonthView;
	 	sTextTopMargin = dateTextTopMarginMonthView;
	 }
 }


/***********************************************************************
 *
 * FUNCTION:    	MonthViewInit
 *
 * DESCRIPTION: 	This routine initialize the month view.
 *
 * PARAMETERS:  ->	frm: pointer of the Month View form.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	ART	4/8/96		Initial revision.
 *
 ***********************************************************************/
static void MonthViewInit (FormPtr formP)
{
	status_t				err;
	uint16_t				gadgetIndex;
	DmResourceID 			bitmapID = 0;
	FrmNavStateFlagsType 	navStateFlags = 0;
	FrmNavHeaderType		navHeader;

	
	sFormP = formP;

	// Since we have a background image, we need to have transparent widgets
	FrmSetTransparentObjects(formP, true);

	// Get the current highlight color
	DateBackgroundGetHighlightColor(&sObjectMonthHighlight, Date.month);

	UIColorGetTableEntryRGB(UIFormFill, &sFormFill);
	UIColorGetTableEntryRGB(UIObjectFill, &sObjectFill);
	UIColorGetTableEntryRGB(UIObjectForeground, &sObjectForeground);
	UIColorGetTableEntryRGB(UIObjectSelectedFill, &sObjectSelectedFill);
	UIColorGetTableEntryRGB(UIObjectSelectedForeground, &sObjectSelectedForeground);
	
	WinSetBackColorRGB(&sFormFill, NULL);
	WinSetForeColorRGB(&sObjectForeground, NULL);
	WinSetTextColorRGB(&sObjectForeground, NULL);

	sDayLabelFont = GcCreateFont("11db palmos-bold");
	GcGetFontHeight(sDayLabelFont, &sDayLabelFontHeight);

	sDayFont = GcCreateFont("9db palmos-plain");
	GcGetFontHeight(sDayFont, &sDayFontHeight);

	sCurrentDayFont = GcCreateFont("12db palmos-bold");
	GcGetFontHeight(sCurrentDayFont, &sCurrentDayFontHeight);

	gadgetIndex = FrmGetObjectIndex(formP, MonthBackgroundGadget);

	DateBackgroundGetBitmapID(&bitmapID, Date.month);

	memset(&sGadgetCallbackData, 0, sizeof(sGadgetCallbackData));
	sGadgetCallbackData.color = sObjectMonthHighlight;
	sGadgetCallbackData.formP = formP;
	sGadgetCallbackData.bitmapID = bitmapID;

	// Save some info we will need in our callback
	FrmSetGadgetData(formP, gadgetIndex, &sGadgetCallbackData);

	// Set up the callback for the title bar graphic
	FrmSetGadgetHandler(formP, gadgetIndex, DateBackgroundGadgetHandler);

	// Initialize Month gadget data
 	MonthViewInitMonth(formP);

	// Lunar Calendar
 	MonthViewInitLunarCalendar(formP);
 	
	// Highlight the Month View push button.
	FrmSetControlGroupSelection (formP, DayViewGroup, sMonthViewButtonID);

	// initial focused object is Month solar or lunar
 	err = FrmGetNavOrder (formP, &navHeader, NULL, 0);
 	
	navHeader.initialObjectIDHint = sMonthViewButtonID;
	
 	err = FrmSetNavOrder (formP, &navHeader, NULL);
 	
	
	// the month uses a cursor with the current category
	err = MonthCursorOpenOrRequery(sMonthP);
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewClose
 *
 * DESCRIPTION: 	This routine closes the month view.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/15/03	Initial revision.
 *	PPL 08/05/04	Fix a memory Leak Calling GcReleaseFont().
 *
 ***********************************************************************/
static void MonthViewClose(void)
{
	status_t err;

	// release any true type font
	if (sDayLabelFont)
	{
		GcReleaseFont(sDayLabelFont);
		sDayLabelFont = NULL;
	}

	if (sDayFont)
	{
		GcReleaseFont(sDayFont);
		sDayFont = NULL;
	}

	if (sCurrentDayFont)
	{
		GcReleaseFont(sCurrentDayFont);
		sCurrentDayFont = NULL;
	}

	if (sDaySmallFont)
	{
		GcReleaseFont(sDaySmallFont);
		sDaySmallFont = NULL;
	}

	if (sCurrentDaySmallFont)
	{
		GcReleaseFont(sCurrentDaySmallFont);
		sCurrentDaySmallFont = NULL;
	}
		
	err = ApptCloseCursor(&gMonthViewCursorID);
		
	if (sMonthP)
	{
		MemPtrFree (sMonthP);
		sMonthP= NULL;
	}
	
	sFormP = NULL;
}


 /***********************************************************************
 *
 * FUNCTION:    	MonthViewSetLunarCalendar
 *
 * DESCRIPTION: 	Ensure that the next time MonthViewInit is called,
 *					we'll display the view associated with <buttonID>.
 *
 * PARAMETERS:  	pushButtonId:	either MonthMonthViewButton 
 *									or MonthLunarViewButton.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	CS 	10/22/02	New today to support Lunar View.
 * 	PPL	03/10/04	Lunar Calendar from PalmOS 5.4 (was DateSetMonthView.)
 *
 ***********************************************************************/
void MonthViewSetLunarCalendar(uint16_t pushButtonId)
{
	if (pushButtonId == MonthMonthLunarViewButton) 
	{
		sMonthViewButtonID = pushButtonId;
	} 
	else 
	{
		sMonthViewButtonID = MonthMonthViewButton;
	}
} 

/***********************************************************************
 *
 * FUNCTION:		MonthViewHandleNavigation
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	monthP:			Month View data.
 *				->	modifiers:		Command mofifiers.
 *				-> 	chr:			Command code.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/08/04	Creation.
 * 	PPL 06/16/04	Add PalmSource OneHanded Specifications.
 *
 ***********************************************************************/
Boolean MonthViewHandleNavigation( MonthType* monthP, uint16_t	modifiers, wchar32_t chr )
{
	uint16_t	focus	= noFocus;
	Boolean 	handled 	= false;

	TraceOutput(TL(appErrorClass, "MonthViewHandleNavigation start"));

	focus = FrmGetFocus(sFormP);

	switch(chr)
	{
		case vchrRockerUp:	
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerUp, Above Cell is focused"));
			handled = MonthViewFocusUpDayCell(sMonthP, focus, modifiers);
			break;
			
		case vchrRockerDown:	
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerDown, Below Cell is focused"));
			handled = MonthViewFocusDownDayCell(sMonthP, focus, modifiers);
			break;

		case vchrThumbWheelBack:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelBack, next/below item, left not handled"));
			break;

		case vchrThumbWheelDown:	
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelDown as vchrRockerLeft"));

		case vchrRockerLeft:	
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerLeft, Previous tab is focused"));
			// 5-way rocker left
			// move focused tab to left
			handled = MonthViewFocusLeftDayCell(sMonthP, focus, modifiers);
			break;
		
		case vchrThumbWheelUp:	
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelUp Next tab is focused"));

		case vchrRockerRight:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerRight, Next tab is focused"));
			// 5-way rocker right
			// move focused tab to right
			handled = MonthViewFocusRightDayCell(sMonthP, focus, modifiers);
			break;
			
		case vchrThumbWheelPush:
			TraceOutput(TL(appErrorClass, "-------------> keyDownEvent, vchrThumbWheelPush as vchrRockerCenter"));

		case vchrRockerCenter: 
			TraceOutput(TL(appErrorClass, "-------------> keyDownEvent, vchrRockerCenter"));	
			// 5-way rocker center/press
			// active focused tab
			handled = MonthViewFocusEnter(sMonthP, focus, modifiers);
			break;
			
		default:
			TraceOutput(TL( appErrorClass, "-------------> Not a 5-way or Wheel Navigation vchr"));
			break;
	}

	TraceOutput(TL(appErrorClass, "MonthViewHandleNavigation exit"));
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	MonthViewHandleEvent
 *
 * DESCRIPTION: 	This routine is the event handler for the Month View
 *              	of the Datebook application.
 *
 * PARAMETERS:  ->	event:	a pointer to an EventType structure
 *
 * RETURNED:    	true if the event was handled and should not be 
 *					passed to a higher level handler.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/8/96		Initial revision.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *
 ***********************************************************************/
Boolean MonthViewHandleEvent (EventType* eventP)
{
	wchar32_t	chr;
	uint16_t	modifiers;
	uint16_t 	daysInMonth;
	FormPtr 	frm;
	DateType 	date;
	Boolean 	handled = false;
	Boolean 	penDown;
	Coord 		x;
	Coord		y;
	
	static RectangleType titleBounds = {0,0,0,0};

	// Trace the event in debug mode
	DBGTraceEvent("MonthView", eventP);

	switch (eventP->eType)
	{
		case nilEvent:
			
			// Check the draw window then redraw the title.
			if (WinGetDrawWindow() != gWinHdl)
				break;
				
			EvtGetPen(&x, &y, &penDown);
			
			if (penDown)
				MonthViewShowTime ();
			else
				MonthViewHideTime (false);
			break;

			
		case penDownEvent:
			handled = MonthFindDayCell ( sMonthP, eventP->screenX, eventP->screenY);
			break;


		case penMoveEvent:
			handled = MonthViewTrackSelectedDayCell( sMonthP, eventP->screenX, eventP->screenY);
			break;


		case penUpEvent:
			// Check if the time was displayed and the pen in the initial title rectangle
			if (TimeDisplayed && RctPtInRectangle(eventP->screenX, eventP->screenY, &titleBounds))
				MonthViewHideTime(true);

			handled = MonthViewSelectDayCell( sMonthP, eventP->screenX, eventP->screenY);
			break;

			
		case keyDownEvent:
			MonthViewHideTime (true);
			
			chr 		= eventP->data.keyDown.chr;
			modifiers	= eventP->data.keyDown.modifiers;
			
			if (EvtKeydownIsVirtual(eventP))
			{
				if (TxtCharIsRockerKey(modifiers, chr) || TxtCharIsWheelKey(modifiers, chr))
				{
					handled = MonthViewHandleNavigation(sMonthP, modifiers, chr);
				}
 				else if (TxtCharIsHardKey(modifiers, chr))
				{
					DateSecondsToDate (TimGetSeconds (), &date);
					Date = date;
					
					if (EventInCurrentView
						|| (modifiers & poweredOnKeyMask))
					{
						// we were on MonthView we go on DayView at power up
						FrmGotoForm(gApplicationDbP, DayView);
					}
					else if (DateSupportsLunarCalendar() && MonthDrawLunarCalendar())
					{
						// we were on MonthView, Lunar Calendar is available
						// and was the active month view
						// then we go on Month Lunar View when powered back
						MonthViewSetLunarCalendar(MonthMonthLunarViewButton);
						FrmGotoForm (gApplicationDbP, MonthView);
					}
					else
					{
						// go in Agenda view
						FrmGotoForm(gApplicationDbP, AgendaView);	
					}
						
					handled = true;
					EventInCurrentView = true;
				}
				else if (chr == vchrPageUp)
				{
					//MonthViewScroll (sMonthP, winUp);
					
					//handled = true;
					//EventInCurrentView = true;
				}	
				else if (chr == vchrPageDown)
				{
					// this behavior is different from the one one we have with One handed navigation
					// deactivate it - let it commented out in case we have to restore it later
					//MonthViewScroll (sMonthP, winDown);
					
					//handled = true;
					//EventInCurrentView = true;
				}
			}
			break;


		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case MonthCategoryTrigger:
					MonthViewSelectCategory ();
					handled = true;
					break;
			
				case MonthDayViewButton:
				case MonthWeekViewButton:
				case MonthMonthViewButton:
				case MonthMonthLunarViewButton:
				case MonthAgendaViewButton:
					// Solar (Gregorian) Calendar specific
					Date.year = sMonthP->year - firstYear;
					Date.month = sMonthP->month;
					
					daysInMonth = DaysInMonth (Date.month, (int16_t)(Date.year + firstYear));
					Date.day = min (Date.day, daysInMonth);
					break;
					
				default:;
			}

			switch (eventP->data.ctlSelect.controlID)
			{
				case MonthGoToButton:
					MonthViewGotoDate ();
					handled = true;
					break;

				case MonthDayViewButton:
					FrmGotoForm(gApplicationDbP, DayView);
					handled = true;
					break;

				case MonthWeekViewButton:
					FrmGotoForm(gApplicationDbP, WeekView);
					handled = true;
					break;

				case MonthMonthLunarViewButton:
					if (sMonthViewButtonID != MonthMonthLunarViewButton)
					{
						// Lunar Calendar
						MonthViewSetLunarCalendar(MonthMonthLunarViewButton);
						
						FrmGotoForm (gApplicationDbP, MonthView);
						handled = true;
					}
					break;
					
				case MonthMonthViewButton:
					if (sMonthViewButtonID != MonthMonthViewButton)
					{
						// Lunar Calendar
						MonthViewSetLunarCalendar(MonthMonthViewButton);
						
						FrmGotoForm (gApplicationDbP, MonthView);
						handled = true;
					}
					break;

				case MonthAgendaViewButton:
					FrmGotoForm(gApplicationDbP, AgendaView);
					handled = true;
					break;

				default:;
			}
			
			break;


		case ctlRepeatEvent:
			switch (eventP->data.ctlRepeat.controlID)
			{
				case MonthPrevButton:
					// go to previous month
					MonthViewScroll (sMonthP, winUp);
					break;
				
				case MonthNextButton:
					// go to next month
					MonthViewScroll (sMonthP, winDown);
					break;
					
				default:;
			}
			break;


		case  menuEvent:
			handled = MonthViewDoCommand (eventP->data.menu.itemID);
			break;


		case winUpdateEvent:
			if (gWinHdl != eventP->data.winUpdate.window)
				break;
				
			// we only tries to refresh if the database is opened
			// the database is closed when we are changing the level of security
			
			MonthViewUpdateDisplay (sFormP, sDayCellSelected);
			
			handled = true;	
			break;
			

		case winResizedEvent:
			// Active Input Area handling
			if (gWinHdl != eventP->data.winResized.window)
				break;
				
			MonthViewFormResize(eventP);
			
			handled = true;
			break;


		case frmOpenEvent:
			frm = FrmGetFormPtr (MonthView);
			MonthViewInit (frm);
			
			EventInCurrentView = false;
			handled = true;
			break;

			
		case frmCloseEvent:
			MonthViewClose();
			break;


		case frmTitleEnterEvent:
			// Generate a nilEvent within 100 miliseconds
			GetTitleBounds(sFormP, &titleBounds);
			
			TimeComputeWaitTime = true;
			TimeDisplayTick = TimGetTicks() + timeDisplayWaitTime;
			break;


		case frmObjectFocusTakeEvent:
			TraceOutput(TL(appErrorClass, "Month Gadget:  frmObjectFocusTakeEvent: fid=%hu obj=%hu",
						eventP->data.frmObjectFocusTake.formID,
						eventP->data.frmObjectFocusTake.objectID));

			if (eventP->data.frmObjectFocusTake.formID == MonthView)
			{
				sMonthViewFocusedItemID = eventP->data.frmObjectFocusTake.objectID;

				switch(sMonthViewFocusedItemID)
				{
					case MonthGadget:
						MonthViewFocusTake(sMonthP);	
						handled = true;
						break;
				}
			}
			
			// the event has to be handle by standard navigation handler		
			break;


		case frmObjectFocusLostEvent:
			TraceOutput(TL(appErrorClass, "Month Gadget: frmObjectFocusLostEvent: fid=%hu obj=%hu",
						eventP->data.frmObjectFocusLost.formID,
						eventP->data.frmObjectFocusLost.objectID));
						
			if (eventP->data.frmObjectFocusLost.formID == MonthView 
				&& eventP->data.frmObjectFocusLost.objectID == MonthGadget)
			{
				MonthViewFocusLost(sMonthP);	
			}			
			
			// the event has to be handle by standard navigation handler		
			break;


		case datebookRefreshDisplay:
			// tries to update the data 
			MonthCursorOpenOrRequery(sMonthP);
		
			// Invalidate the full window to get it redisplayed
			MonthViewDisplayInvalidate(NULL, sDayCellSelected);	
			break;
	}

	return (handled);
}
