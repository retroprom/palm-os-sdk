/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateWeek.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This is the Datebook application's week view module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <AboutBox.h>
#include <AppMgr.h>
#include <CatMgr.h>
#include <ClipBoard.h>
#include <Control.h>
#include <DataMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <FeatureMgr.h>
#include <FontSelect.h>
#include <List.h>
#include <Menu.h>
#include <PenInputMgr.h>
#include <Preferences.h>
#include <PrivateRecords.h>
#include <SelDay.h>
#include <ScrollBar.h>
#include <string.h>
#include <StringMgr.h>
#include <SysEvtMgr.h>
#include <SystemResources.h>
#include <SysUtils.h> // ABS() macro
#include <Table.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIColor.h>
#include <UIResources.h>
#include <string.h>

#include "DateGlobals.h"

// Lunar Calendar
#include "DateLunar.h"
#include "DateMonth.h"


#include "DateDay.h"
#include "DateUtils.h"
#include "DateWeek.h"
#include "Datebook.h"

#include "DateBackground.h"

#include "DatebookRsc.h"

#ifndef TxtCharIsWheelKey
//*********************************************************
// Macro for detecting if character is a rocker character
//*********************************************************

// <c> is a rocker key if the event modifier <m> has the command bit set
// and <c> is in the proper range
#define	TxtCharIsWheelKey(m, c)		((((m) & commandKeyMask) != 0) && \
									((((c) >= vchrThumbWheelUp) && ((c) <= vchrThumbWheelBack))))

#endif


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

//	WeekView/Day object constants
#define weekViewDayHMargin							4		//	This is the horizontal space
 															//	between the edge of the day's
 															//	bounds and the visible event
 															//	bar.

#define weekViewTimeDisplayWidth					25		//	Width of column of times
#define timeHMargin									2		//	Horiz margin between times
															//	& first day bar.
#define weekHMargin									2		//	Horiz margin between days
															//	and right edge of screen.
#define weekViewDayDisplayHeight					12		//	Height of day labels at top
															//	of view.
#define weekViewDateDisplayHeight					13		//	Height of date labels under
															//	day labels at top of view.
#define weekViewEventFrameWidth						3		//	Width of frame around

#define weekViewActiveInputShownScrollInterval		4		// Scroll Interval in hours whithout AIA
															//	Must be >= 1.
															// (used as divisor operand)

#define weekViewActiveInputHiddenScrollInterval		6		//	Scroll interval, in hours with AIA
															//	Must be >= weekViewAIACondensedScrollInterval.
															//	Must be >= 1.
															// (used as divisor operand)

#define weekViewNumActiveInputShownDispHours		10		//	Number of hours that we display
															//	on one screenful of data.
															// when Active graffiti area is SHOWN

#define weekViewNumActiveInputHiddenDispHours		16		//	Number of hours that we display
															//	on one screenful of data.
															// when Active graffiti area is HIDDEN

#define weekViewHourDisplayInterval					2		//	Used for displaying the time
															//	bar.

#define dataMarkHMargin								4		//	Horizontal margins around
															//	data indicators.
#define dataMarkVMargin								2
#define outOfRangeDay								0		//	illegal date.

#define weekDescMarginX								3		// Margin in popup desc
#define weekDescMarginY								2		// Margin in popup desc

#define moveThreshold								4		// Number of pixels to drag an
															// event before it starts moving

#define weekViewEventbarMinHeight					3		// minimun height wanted in pixels (single density)

#define weekDescDisplayTicks						2000	// number of ms to display the
															// description popup
#define weekDescUnmaskedDisplayTicks				5000	// number of ms to display the
															// description popup for unmasked
															// events
#define descWidthMax								40		// Max width of the pop-up description

#define kWeekFocusNoFocus							noFocus
#define kWeekFocusedState							0x01
#define kWeekFocusDOWState							0x02
#define kWeekFocusEventSlotState					0x0

#define kWeeFocusEmptySlot							0x8000	// event slot is selected or a free slot

// for WeekFocusSetUpFocusTrigger
#define kFocusTriggerDown 							2
#define kFocusTriggerUp  							8
#define kFocusTriggerReset  						1
#define kFocusTriggerGet  							0


#define kMaxRGBValue								0xFF




/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

//	These structures are used by the WeekView of the DateBook
typedef struct
{
	RectangleType			bounds;
	DateType				date;					//	If the 'day' field is set to
													//	'outOfRangeDay', this day is
													//	outside the allowable system range
	uint8_t					dayOfWeek;
	uint8_t					padding;
	int32_t					numAppts;
	MemHandle				apptsH;					//	Use ApptInfoPtr to examine													//	and should not be drawn.
	uint16_t				firstEventHour;
	uint16_t				lastEventHour;			//	The limits of the events which
													//	exist on this day.  Meaningless
													//	if there are no appts.
} DayType;

typedef struct
{
	RectangleType			bounds;
	uint16_t				startHour;				//	first currently displayed hour
	uint16_t				endHour;				//	last currently displayed hour
	DayType					days[daysInWeek];
	DateType				startDate;				//	Start date of week
	uint16_t				firstEventHour;			//	Hour of earliest event
													//	during the current week.
	uint16_t				lastEventHour;			//	Hour of latest event
													//	during the current week.
	uint16_t				dayWidth;				//	width of day column in pixels
	uint16_t				hourHeight;				//	Height, in pixels, of one
													//	hour of time in a day bar.
	uint16_t				hoursDisplayed;			// Number of hour to be displayed
													// on one page of the view when
													// scroll is needed
	uint16_t				scrollInterval;			// Interval to be used for Scrolls
													// depends on the the Active input
													// Area status
	uint16_t				padding1;
	RectangleType			behindRect;				// Rectangle displaying popup description.
	RectangleType			focusedZone;			// Rectangle for currently focused zone
	RectangleType			focusTrigger;			// Rectangle for empty slot
	Boolean 				popupDescDisplayed;
	uint8_t					padding2;
	uint16_t				focusState;				// One-Handed navigation focus state
	uint16_t				focusedDay;				// focused Day of week
	//uint16_t				focusedDOW;				// focused Day of week
	int32_t					focusedApptIndex;		// index of appoitment whithin day
	uint32_t				focusedRowId;			// row Id of event focused
	Boolean					focusedOverlaps;		// overlap status of event
} WeekType;

typedef WeekType * WeekPtr;


typedef struct
{
	TimeType				startTime;
	TimeType				endTime;
	int16_t 				dayOfWeek;
	Boolean 				existingLinkedEvent;
	RectangleType 			eventR;
} LinkedWeekEventType;

typedef LinkedWeekEventType * LinkedWeekEventPtr;

enum TrackingTypeTag {
	nothingSelected = 0,
	emptySlotSelected,
	dateSlotSelected,
	trackingApptSlot,
	dragAborted
} ;

typedef Enum16 TrackingType;


typedef struct
{
	TrackingType				trackingType;
	uint16_t					padding1;

	// Date picking or empty slots: both behaves like buttons
	RectangleType				pickR;			// pickR is the picking area
	RectangleType				highR;			// highR is the highlighted region
	RectangleType				saveR;			// saveR is the saved window (framed highlights only)
	TimeType					newStart;
	TimeType					newEnd;
	Boolean						newAppt;
	Boolean						wasSelected;	// the "button" was selected
	Boolean						isSelected;		// the "button" is selected
	Boolean						drawFrame;		// draw frame of empty slot
	FrameBitsType				eventFrame;

	// Selected Event moving
	int32_t						displayTicks;
	RectangleType				moveR;
	RectangleType				dayR;
	RectangleType	 			weekR;
	RectangleType	 			weekAndMarksR;
	RectangleType				eventR;
	ApptDBRecordType			apptRec;
	TimeType	 				start;
	TimeType					end;
	TimeType 					initialEnd;
	TimeType 					timeDiff;
	uint32_t					rowID;
	DateType					date;
	int16_t 					prevPosInDay;
	int16_t						dayOfWeek;
	int16_t						newDayOfWeek;
	int16_t						savedDayOfWeek;
	int16_t						posInAppt;
	LinkedWeekEventType			linkedEvent;
	MidnightOverlapStateType	overlapState;
	RectangleType				descRect;
	uint8_t						padding2[3];
} WeekViewEventTrackingType;


/***********************************************************************
 *
 *	Internal Global variables
 *
 ***********************************************************************/
static MemHandle					sWeekObjectH = NULL;				//	The currently active week
static FontID						sOrigFont;
static WeekViewEventTrackingType 	sTrackingData;

static DateGadgetCallbackDataType	sGadgetCallbackData;
static RGBColorType					sBackgroundHighlight;
static RGBColorType					sObjectWeekHighlight;

static FormType* 					sFormP = NULL;

static GcColorType 					shadow = {128, 128, 128, 128};
static GcColorType 					highlight = {255, 255, 255, 128};


static uint16_t						sWeekViewFocusedItemID = frmInvalidObjectId;


static void WeekViewFocusResize( Coord xOffset, Coord yOffset);


/***********************************************************************
 *
 * FUNCTION:    	TimeDifference
 *
 * DESCRIPTION: 	Subtract pTime2 from pTime1 and place the result in
 *					pTimeDiff.
 *
 *					Note that TimeType is now unsigned so this doesn't
 *					work for negative times (which no longer exist!).
 *
 * PARAMETERS:  ->	pTime1:				pointer to HMSTime
 *              ->	pTime2:				pointer to HMSTime
 *				->	wrappingMidnight:	the event wrapping around midnight?
 *              ->	pTimeDiff:			pointer to HMSTime
 *
 * RETURNED:	 	pTimeDiff is set to pTime1 - pTime2.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RFL	12/2/94		Initial revision.
 *
 ***********************************************************************/
static void TimeDifference (
	TimePtr 	pTime1,
	TimePtr 	pTime2,
	Boolean 	wrappingMidnight,
	TimePtr 	pTimeDiff)
{
	if (!wrappingMidnight)
		pTimeDiff->hours = pTime1->hours - pTime2->hours;
	else
		pTimeDiff->hours = (hoursPerDay - pTime2->hours) + pTime1->hours;

	if (pTime1->minutes < pTime2->minutes)
	{
		pTimeDiff->minutes = pTime1->minutes + hoursInMinutes - pTime2->minutes;
		pTimeDiff->hours--;
	}
	else
	{
		pTimeDiff->minutes = pTime1->minutes - pTime2->minutes;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	FirstDayOfYear
 *
 * DESCRIPTION: 	Return the number of day from 1/1/1904 of the first
 *					day of the year passed.
 *
 *					The first day of the year is always a Monday.
 *					The rule for determining the first day of
 *					the year is:
 *
 *					New Years Day	First Day of the Year
 *					------------	---------------------
 *					Monday			Monday Jan 1
 *					Tuesday			Monday Dec 31
 *					Wednesday		Monday Dec 30
 *					Thursday		Monday Dec 29
 *					Friday			Monday Jan 4
 *					Saturday		Monday Jan 3
 *					Sunday			Monday Jan 2
 *
 * PARAMETERS:	 ->	year: year (1904-2031.)
 *
 * RETURNED:	 	Number of days since 1/1/1904.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		4/4/96	Initial revision.
 *
 ***********************************************************************/
static uint32_t FirstDayOfYear (uint16_t year)
{
	uint32_t days;
	uint16_t dayOfWeek;
	DateType date;

	// Get days to January 1st of the year passed.
	date.day = 1;
	date.month = 1;
	date.year = year - firstYear;
	days = DateToDays (date);

	dayOfWeek = DayOfWeek (1, 1, year);

	// Move to monday.
	days++;
	days -= dayOfWeek;


	if (dayOfWeek >= friday)
		days += daysInWeek;

	return (days);
}


/***********************************************************************
 *
 * FUNCTION:    	GetVeryShortDate
 *
 * DESCRIPTION:		Make a very short date (month+day) according to
 *					pref settings.
 *
 * PARAMETERS:	->	date:		The input date.
 *				<-	dateStr:	Output string.
 *				<-	strLen: 	Max size of output string.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	07/01/09	Initial revision.
 *
 ***********************************************************************/
static void GetVeryShortDate(DatePtr date, char * dateStr, int16_t strLen)
{
	switch (ShortDateFormat)
	{
		case dfMDYWithSlashes:
		case dfYMDWithSlashes:
			DateTemplateToAscii("^3z/^0z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
			break;

		case dfDMYWithSlashes:
			DateTemplateToAscii("^0z/^3z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
			break;

		case dfYMDWithDots:
			DateTemplateToAscii("^3z.^0z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
			break;

		case dfDMYWithDots:
			DateTemplateToAscii("^0z.^3z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
			break;

		case dfDMYWithDashes:
			DateTemplateToAscii("^0z-^3z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
			break;

		case dfYMDWithDashes:
		case dfMDYWithDashes:
			DateTemplateToAscii("^3z-^0z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
			break;

		default:
			// By default, use MM/DD
			// This shouldn't be used since all short dates are listed above
			DateTemplateToAscii("^3z/^0z", (uint8_t) date->month, (uint8_t) date->day,
				(uint16_t)(date->year + firstYear), dateStr, strLen);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	GetWeekNumber
 *
 * DESCRIPTION: 	Calculate the week number of the specified date.
 *
 * PARAMETERS:	->	month:	month (1-12.)
 *              ->	day:	day (1-31.)
 *              ->	year:	year (1904-2031.)
 *
 * RETURNED:		Week number (1-53.)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/4/96		Initial revision.
 *
 ***********************************************************************/
static uint16_t GetWeekNumber (uint16_t month, uint16_t day, uint16_t year)
{
	uint16_t dow;
	uint16_t week;
	uint32_t days;
	uint32_t firstOfYear;
	uint32_t firstOfNextYear;
	DateType date;

	// Calculate the julian date of Monday in the same week as the
	// specified date.
	date.day = day;
	date.month = month;
	date.year = year - firstYear;
	days = DateToDays (date);

	// Adjust the day of the week by the preference setting for the first day
	// of the week.
	dow = (DayOfWeek (month, day, year) - StartDayOfWeek + daysInWeek)
				% daysInWeek;

	if (monday < StartDayOfWeek)
		days -= (uint32_t)(dow - (monday + daysInWeek - StartDayOfWeek));
	else
		days -= (uint32_t)(dow - (monday - (int16_t)StartDayOfWeek));


	firstOfYear = FirstDayOfYear (year);

	if (days < firstOfYear)
	{
		// The date passed is in a week that is part of the prior
		//	year, so get the start of the prior year.
		if (year > firstYear)
			firstOfYear = FirstDayOfYear (--year);
	}
	else
		{
		// Make sure the date passed is not in a week that in part
		// of next year.
		if (year < lastYear)
		{
			firstOfNextYear = FirstDayOfYear ((uint16_t)(year + 1));
			if (days == firstOfNextYear)
				firstOfYear = firstOfNextYear;
		}
	}

	// one base
	week = ((int16_t)(days - firstOfYear)) / daysInWeek + 1;

	return (week);
}


/***********************************************************************
 *
 * FUNCTION:    	AdjustEventRight
 *
 * DESCRIPTION:		Shifts a full-width appt rect to the right side of
 *					the event bar.  If the event bar is odd-width, we
 *					will put the smaller portion on the right.
 *
 * PARAMETERS:		<->	event:	Event bounds.
 *
 * RETURNED:		Nothing; event is adjusted.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/5/95		Initial revision.
 *
 ***********************************************************************/
static void AdjustEventRight (RectangleType* event)
{
	int16_t	oldWidth;

	//	Make the bar an extra pix narrower to accomodate it's frame.
	oldWidth = event->extent.x;
	event->extent.x = oldWidth / 2 - 1;
	event->topLeft.x += (oldWidth - event->extent.x);
}


/***********************************************************************
 *
 * FUNCTION:    	AdjustEventLeft
 *
 * DESCRIPTION:		Shifts a full-width appt rect to the left side of
 *					the event bar.  If the event bar is odd-width, the
 *					smaller half of the bar goes on the right.
 *
 * PARAMETERS:	<->	event:	Event bounds.
 *
 * RETURNED:		Nothing; event is adjusted.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/5/95		Initial revision.
 *
 ***********************************************************************/
static void AdjustEventLeft (RectangleType* event)
{
	//	Make the bar an extra pix narrower to accomodate the frame.
	event->extent.x = event->extent.x - (event->extent.x / 2) - 1;
}


/***********************************************************************
 *
 * FUNCTION:    	UpdateEndTime
 *
 * DESCRIPTION:		Compares the passed time with the passed endTime.  If the
 *					time is later than the endTime, the endTime will be
 *					advanced to the earlier of either the time or the
 *					timeLimit.
 *
 * PARAMETERS:	->	time:		Time to compare.
 *				<->	endTime:	Time to update.
 *				->	timeLimit:	Cannot advance *endTime
 *								past this hour value.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/5/95		Initial revision.
 *
 ***********************************************************************/
static void UpdateEndTime (TimeType *time, TimeType *endTime, uint16_t timeLimit)
{
	if (TimeToInt(*time) > TimeToInt (*endTime))
	{
		if (time->hours >= ((uint8_t) timeLimit) )
		{
			endTime->hours = (uint8_t)  timeLimit;
			endTime->minutes = 0;
		}
		else
		{
			endTime->hours = time->hours;
			endTime->minutes = time->minutes;
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewHoursToCoords
 *
 * DESCRIPTION:		Convert a time to a y-coordinate position, based on
 *					a starting hour.  'time' is assumed to be within the
 *					current hour display range.
 *
 * PARAMETERS:	->	time:		The time to convert
 *				->	startHour:	Baseline of time to use
 *								as the zero-coordinate.
 *				->	hourHeight:	Vertical size of an hour-long
 *								event.
 *
 * RETURNED:		yPos:	The difference, in y-coords,
 *							between startHour and time, based on
 *							hourHeight's week data structure.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/3/95		Initial revision.
 *
 ***********************************************************************/
static uint16_t WeekViewHoursToCoords(
	TimeType 	time,
	uint16_t 	startHour,
	uint16_t 	hourHeight)
{
	uint16_t 	yPos;

	yPos = (time.hours - startHour) * hourHeight;
	yPos += (time.minutes * hourHeight) / 60;

	return (yPos);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewResetTracking
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	None
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE		8/9/95	Initial revision.
 *
 ***********************************************************************/
static void WeekViewResetTracking (void)
{
	// Set trackingType to nothingSelected
	memset(&sTrackingData, 0x00, sizeof(WeekViewEventTrackingType));
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewIsTrackActive
 *
 * DESCRIPTION: 	Tell if Tracking is active or not.
 *
 * PARAMETERS: 	->	trackingDataP:	tracking information.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR		8/9/95	Initial revision.
 *
 ***********************************************************************/
static Boolean WeekViewIsTrackActive (void)
{
	if (sTrackingData.trackingType == trackingApptSlot)
		return true;

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewNewAppointment
 *
 * DESCRIPTION:		This routine create a new apointment. It's called
 *                	when the user selects an empty time slot.
 *
 * PARAMETERS:	->	startTime - start time of the new appointment
 *              -> 	endTime   - end time of the new appointment
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	9/25/95		Initial revision.
 *  RYW 2/18/00  	Added cast to satisfy const cstring checking,
 *					should be safe.
 *
 ***********************************************************************/
static void WeekViewNewAppointment (TimePtr startTime, TimePtr endTime)
{
	ApptDBRecordType 	newAppt;
	ApptDateTimeType 	when;
	uint32_t 			rowID;
	status_t 			error;


	// Create a new appointment on the current day.
	memset (&newAppt, 0x00, sizeof (newAppt));

	when.noTimeEvent = false;
	when.startTime = *startTime;
	when.endTime = *endTime;
	when.midnightWrapped = false;

	if (TimeToInt(when.endTime) > apptMaxEndTime)
		TimeToInt (when.endTime) = apptMaxEndTime;

	when.date = Date;
	strncpy(when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);

	newAppt.when = when;
	newAppt.description = (char *)"";

	if (AlarmPreset.advance != apptNoAlarm)
		newAppt.alarm = AlarmPreset;
	else
		newAppt.alarm.advance = apptNoAlarm;

	error = ApptNewRecord (ApptDB, &newAppt, &rowID);
	if (error < errNone)
		return;

	// Assign the current category to the event, if the current
	// category displayed is all, let the new record as unfiled (default value)
	if (! DatebookAllCategorySelected())
		DbSetCategory(ApptDB, rowID, DateBkCurrentCategoriesCount, DateBkCurrentCategoriesP);

	// Set the cursor on the new created record, and activate edit mode
	ApptDBRequery(ApptDB, gApptCursorID, false);
	DbCursorMoveToRowID(gApptCursorID, rowID);
	gItemSelected = true;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewCheckAndSetMinEventBarHeight
 *
 * DESCRIPTION:  	Compute the minimal height for an event bar in order
 *					to be Usable and visible by User.
 *
 * PARAMETERS:  <->	dayBoundsP:		Rectangle to check and set.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	08/06/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewCheckAndSetMinEventBarHeight(RectangleType* bounds)
{
	if (bounds && bounds->extent.y < weekViewEventbarMinHeight)
		bounds->extent.y = weekViewEventbarMinHeight;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewCreateEventBar
 *
 * DESCRIPTION:		This routine calculates the area of a given day's
 *					event bar.
 *
 * PARAMETERS:	->	day:	Full day's bounds
 *				<->	r:		Points to rect to be filled with
 *							event bar's coords.  The resulting
 *							bar does not include the frame around
 *							it.  The top & bottom of the frame
 *							should be added before this is passed
 *							to GetEventBounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/10/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewCreateEventBar (RectangleType* day, RectangleType* r)
{
 	//	NOTE: since this is a framed event bar, the bounds of the bar must
 	//	be shrunk 1 pix on top & sides to keep the frame (drawn outside of passed
 	//	rect) within the bounds.  This is only nesc because we are using
 	//	a frame around the event bar.

 	r->topLeft.x = day->topLeft.x + weekViewDayHMargin + 1;
 	r->topLeft.y = day->topLeft.y +
 						weekViewDayDisplayHeight +
 						weekViewDateDisplayHeight + 1;
 	r->extent.x = day->extent.x - 2*weekViewDayHMargin - 1;
 	r->extent.y = day->extent.y -
 						weekViewDayDisplayHeight -
 						weekViewDateDisplayHeight - 1;
 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewGetEventBounds
 *
 * DESCRIPTION:		Determine the top & bottom bounds of an event.
 *
 * PARAMETERS:	->	startTime:
 *				->	endTime:
 *				->	startHour:	Start of time display
 *				->	endHour:	End of time display
 *				<->	r:			Points to rectangle holding whole
 *								event bar's coords, including top &
 *								bottom of frame.
 *					hourHeight:	Height, in pix, of an hour-long event.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			y-coords of r have been adjusted so that
 *					frame around event (top & bottom) will
 *					include start & end times.
 *
 *
 * REVISION HISTORY:
 *	KCR	08/08/95	Initial revision.
 *	PPL	08/06/03 	Set up a minimal bar height to be usable for
 *					event with 0 minutes or 5 minutes length.
 *
 ***********************************************************************/
static void WeekViewGetEventBounds(
	TimeType 		startTime,
	TimeType 		endTime,
	uint16_t 		startHour,
	uint16_t 		endHour,
	RectangleType*	r,
	uint16_t 		hourHeight)
{
	int16_t			startYOffset = 0;
	int16_t			endYOffset;

	//	Determine start y coord, relative to top of r, inclusive of the
	//	startTime:
	if (startTime.hours > startHour
	||	(startTime.hours == startHour
	&& startTime.minutes != 0))
	{
		//	Need to adjust start y coord.
		startYOffset = WeekViewHoursToCoords(startTime, startHour, hourHeight);
		r->topLeft.y += startYOffset;
	}

	//	Determine ending coord, relative to the top of the event bar,
	//	inclusive of the endTime:
	endYOffset = r->extent.y;

	if (endTime.hours < endHour)
	{
		//	Need to shorten winUp end y-coord
		endYOffset = WeekViewHoursToCoords(endTime, startHour, hourHeight);
 		endYOffset++;				//	Make the range inclusive of the endTime
	}

	r->extent.y = endYOffset - startYOffset;

	r->topLeft.y++;					//	Allow for top & bottom of frame
	r->extent.y -= 2;

	// we want a usable /visible rectangle for user
	WeekViewCheckAndSetMinEventBarHeight(r);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeDateFromLinkedEvent
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	week:
 *				<->	startTimeP:
 *				<->	endTimeP:
 *				<->	dateP:
 *				->	mainDayOfWeek:
 *				->	linkedEventP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE		7/1/02	Support for events wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewComputeDateFromLinkedEvent(
	WeekType* 			week,
	TimePtr 			startTimeP,
	TimePtr 			endTimeP,
	DatePtr 			dateP,
	int16_t 			mainDayOfWeek,
	LinkedWeekEventPtr 	linkedEventP)
{
	if (! linkedEventP->existingLinkedEvent)
		return;

	*dateP = week->days[mainDayOfWeek].date;

	if (linkedEventP->dayOfWeek == mainDayOfWeek - 1)
	{
		// The linked event is before: the record date to save is the linked date,
		// that is, the day before mainDayOfWeek
		*startTimeP = linkedEventP->startTime;
		DateAdjust(dateP, -1);
	}
	else if (linkedEventP->dayOfWeek == mainDayOfWeek + 1)
	{
		// The linked event is after
		*endTimeP = linkedEventP->endTime;
	}
	else
		ErrNonFatalDisplay("WeekViewComputeDateFromLinkedEvent, Linked day of week invalid");
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeStartTimeFromPos
 *
 * DESCRIPTION:  	Compute the start time according to its position.
 *
 * PARAMETERS:  ->	posInDay:
 *				-> 	hourHeight:
 *				<->	newStartTimeP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE		7/1/02	Support for events wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewComputeStartTimeFromPos(
	int16_t 	posInDay,
	uint16_t 	hourHeight,
	TimePtr 	newStartTimeP)
{
	newStartTimeP->hours 	= (posInDay / hourHeight);

	newStartTimeP->minutes	=  (2 * (posInDay % hourHeight) / hourHeight) * (hoursInMinutes / 2);


	if (newStartTimeP->hours >= hoursPerDay)
	{
		newStartTimeP->hours = hoursPerDay - 1;
		newStartTimeP->minutes = hoursInMinutes - 5;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeEndTimeFromPos
 *
 * DESCRIPTION:  	Compute the end time of event wrapping around
 *					midnight.
 *
 * PARAMETERS:  -> 	posInDay:
 *				-> 	initialEndTime:
 *				<-> EndTimeP:
 *				-> 	hourHeight:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE		7/1/02	Support for events wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewComputeEndTimeFromPos(
	int16_t 	posInDay,
	TimeType 	InitialEndTime,
	TimePtr 	endTimeP,
	uint16_t 	hourHeight)
{
	int32_t 	newEndHours;
	int32_t		newEndMinutes;

	newEndHours = (int32_t)((posInDay / hourHeight) + InitialEndTime.hours);
	newEndMinutes =  (2 * ((int32_t)posInDay % (int32_t)hourHeight) /
		(int32_t)hourHeight) * (hoursInMinutes / 2) + (int32_t)InitialEndTime.minutes;

	// The minutes can be negative...

	while (newEndMinutes < 0)
	{
		newEndHours--;
		newEndMinutes += hoursInMinutes;
	}

	// or can exceed hoursInMinutes
	while (newEndMinutes >= hoursInMinutes)
	{
		newEndHours++;
		newEndMinutes -= hoursInMinutes;
	}


	if (newEndHours < 0)
		newEndHours = 0;

	if (newEndHours >= hoursPerDay)
	{
		newEndHours = hoursPerDay - 1;
		newEndMinutes = hoursInMinutes - 5;
	}

	endTimeP->hours = (uint8_t) newEndHours;
	endTimeP->minutes = (uint8_t) newEndMinutes;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeEventBounds
 *
 * DESCRIPTION:  	Calculate the event bounds whithin the Week View.
 *
 * PARAMETERS:  ->	week:
 *				->	dayOfWeek:
 *				->	start:
 *				->	end:
 *				->	hourHeight:
 *				<->	eventRectP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE		7/1/02	Support for events wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewComputeEventBounds(
	WeekType* 		week,
	int16_t 		dayOfWeek,
	TimeType 		start,
	TimeType 		end,
	uint16_t 		hourHeight,
	RectangleType* 	eventRectP)
{
	// Compute the bounds of the event.
	WeekViewCreateEventBar (&(week->days[dayOfWeek].bounds), eventRectP);

	WeekViewGetEventBounds (
				start,
				end,
				week->startHour,
				week->endHour,
				eventRectP,
				hourHeight);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeLinkedEventBounds
 *
 * DESCRIPTION:  	Compute the bounds of the linked event.
 *
 * PARAMETERS:  ->	week:		Week date structure.
 *				->	hourHeight:	height of one hour.
 *				<->	linkedEvtP: Linked event information
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	7/1/02		Support for events wrapping around 12am
 *
 ***********************************************************************/
static void WeekViewComputeLinkedEventBounds(
	WeekType* 			week,
	uint16_t 			hourHeight,
	LinkedWeekEventPtr 	linkedEvtP)
{
	if (!linkedEvtP->existingLinkedEvent
		 	|| (linkedEvtP->dayOfWeek < 0 )
			|| (linkedEvtP->dayOfWeek >= daysInWeek )
			|| (linkedEvtP->endTime.hours < week->startHour)
		 	|| (linkedEvtP->startTime.hours >= week->endHour) )
	{
		memset(&linkedEvtP->eventR, 0x00, sizeof (RectangleType));
		return;
	}

	// Compute the bounds of the event.
	WeekViewCreateEventBar (&(week->days[linkedEvtP->dayOfWeek].bounds), &linkedEvtP->eventR);


	WeekViewGetEventBounds (
			linkedEvtP->startTime,
			linkedEvtP->endTime,
			week->startHour,
			week->endHour,
			&linkedEvtP->eventR,
			hourHeight);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeStrictDayBounds
 *
 * DESCRIPTION:  	Compute the bounds of the linked event.
 *
 * PARAMETERS:  ->	fullDayBoundsP:		Day Rectangle with date.
 *				<-	dayBoundsP:			Calculated day bounds.
 *
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	08/03/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewComputeStrictDayBounds( RectangleType* fullDayBoundsP, RectangleType * dayBoundsP)
{
	*dayBoundsP = *fullDayBoundsP;

 	dayBoundsP->topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight;
 	dayBoundsP->extent.y -= weekViewDayDisplayHeight + weekViewDateDisplayHeight;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewComputeTresholdBounds
 *
 * DESCRIPTION:  	Compute the bounds of the treshold given a point's
 *					x and y coordinate.
 *
 * PARAMETERS:  ->	x,y:			Point coordinates.
 *				->  treshold:		Treshold value;
 *				<-	dayBoundsP:		Calculated treshold rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	08/03/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewComputeTresholdBounds(Coord x, Coord y, Coord treshold, RectangleType* tresholdBounds)
{
	tresholdBounds->topLeft.x 	= x - treshold;
	tresholdBounds->topLeft.y 	= y - treshold;
	tresholdBounds->extent.x 	= treshold << 1;
	tresholdBounds->extent.y 	= treshold << 1;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewPointInDescription
 *
 * DESCRIPTION: 	This routine returns true if the point passed is within
 *              	the bounds if an appointment description popup.
 *
 * PARAMETERS:  	x, y - pen location.
 *
 * RETURNED:    	true if point is in description popup.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	7/15/96		Initial revision.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static Boolean WeekViewPointInDescription (int16_t x, int16_t y)
{
 	WeekType*		weekP;
	Boolean 		inDesc = false;
	RectangleType 	r;

	weekP = MemHandleLock (sWeekObjectH);

	if (weekP->popupDescDisplayed)
	{
		r.topLeft.x = 0;
		r.topLeft.y = 0;
		WinGetWindowExtent (&r.extent.x, &r.extent.y);
		r.extent.y = (FntLineHeight () * 2) + (weekDescMarginY * 3) + 3;

		inDesc = RctPtInRectangle (x, y, &r);
	}

 	MemHandleUnlock(sWeekObjectH);

	return (inDesc);
}

/***********************************************************************
 *
 * FUNCTION:   	 	GetDaysBounds
 *
 * DESCRIPTION:		Calculates and returns the bounds of the
 *					free slot ractangle
 *
 * PARAMETERS:	->	weekP:		Week data (locked handle.)
 *              <-	daysBounds:	Days  bounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			weekP (locked handle)
 *					and dowBounds  have to be valid.
 *
 * REVISION HISTORY:
 *	PPL	04/21/04		Initial revision.
 *
 ***********************************************************************/
static void GetDaysBounds(WeekType* weekP, RectangleType* daysBounds)
{
	daysBounds->topLeft.x = weekP->bounds.topLeft.x + weekViewTimeDisplayWidth;
	daysBounds->extent.x = (weekP->bounds.extent.x - weekViewTimeDisplayWidth);

	daysBounds->topLeft.y = weekP->bounds.topLeft.y + weekViewDayDisplayHeight + weekViewDateDisplayHeight;
	daysBounds->extent.y = weekP->hoursDisplayed * weekP->hourHeight - 1;

	//RctInsetRectangle(daysBounds, +1);
}

/***********************************************************************
 *
 * FUNCTION:   	 	GetDayOfWeekBounds
 *
 * DESCRIPTION:		Calculates and returns the bounds of the
 *					free slot rectangle
 *
 * PARAMETERS:	->	weekP:		Week data (locked handle.)
 *              <-	daysBounds:	Days  bounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			weekP (locked handle)
 *					and dowBounds  have to be valid.
 *
 * REVISION HISTORY:
 *	PPL	04/21/04		Initial revision.
 *
 ***********************************************************************/
static void GetDateAndDayBounds(WeekType* weekP, uint16_t dow, RectangleType* dowBounds)
{
	Coord	dayWidth;
	Coord	headerHeight;

	if (dow >= 0 && dow < daysInWeek)
	{
		dayWidth = (weekP->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek;
		headerHeight = weekViewDayDisplayHeight + weekViewDateDisplayHeight;

		dowBounds->topLeft.x = weekP->bounds.topLeft.x + weekViewTimeDisplayWidth + dow* dayWidth;
		dowBounds->extent.x = dayWidth;

		dowBounds->topLeft.y = weekP->bounds.topLeft.y;
		dowBounds->extent.y = headerHeight;

		RctInsetRectangle(dowBounds, +1);
	}
	else
		RctSetRectangle(dowBounds, 0, 0, 0, 0);
}

/***********************************************************************
 *
 * FUNCTION:   	 	GetDayOfWeekBounds
 *
 * DESCRIPTION:		Calculates and returns the bounds of the
 *					given day of week column.
 *
 * PARAMETERS:	->	weekP:		Week data (locked handle.)
 *              ->	dow:		Day of week.
 *              <-	dowBounds:	dow bounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			weekP (locked handle)
 *					and dowBounds  have to be valid.
 *
 * REVISION HISTORY:
 *	PPL	04/21/04		Initial revision.
 *
 ***********************************************************************/
static void GetDayOfWeekBounds(WeekType* weekP, uint16_t dow, RectangleType* dowBounds)
{
	RectangleType 	daysBounds;
	Coord			dayWidth;

	GetDaysBounds(weekP, &daysBounds);

	if (dow >= 0 && dow < daysInWeek)
	{
		dayWidth = daysBounds.extent.x / daysInWeek;

		dowBounds->topLeft.x = daysBounds.topLeft.x + (dow) * dayWidth;
		dowBounds->extent.x = dayWidth;

		dowBounds->topLeft.y  = daysBounds.topLeft.y;
		dowBounds->extent.y = daysBounds.extent.y;
	}
	else
		RctSetRectangle(dowBounds, 0, 0, 0, 0);
}



/***********************************************************************
 *
 * FUNCTION:   	 	GetFirstFreeSlotBounds
 *
 * DESCRIPTION:		Calculates and returns the bounds of the
 *					first free slot rectangle in the given day column.
 *
 * PARAMETERS:	->	weekP:		Week data (locked handle.)
 *              ->	dow:		Day of week.
 *              <-	dowBounds:	dow bounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			weekP (locked handle)
 *					and dowBounds  have to be valid.
 *
 * REVISION HISTORY:
 *	PPL	04/21/04		Initial revision.
 *
 ***********************************************************************/
static void GetFirstFreeSlotBounds(WeekType* weekP, uint16_t dow, RectangleType* dowBounds)
{
	RectangleType 	daysBounds;
	Coord			dayWidth;

	GetDaysBounds(weekP, &daysBounds);

	if (dow >= 0 && dow < daysInWeek)
	{
		dayWidth = daysBounds.extent.x / daysInWeek;

		dowBounds->topLeft.x = daysBounds.topLeft.x + (dow) * dayWidth;
		dowBounds->extent.x = dayWidth;

		dowBounds->topLeft.y = daysBounds.topLeft.y;
		dowBounds->extent.y = weekP->hourHeight;

		//RctInsetRectangle(dowBounds, +1);
	}
	else
		RctSetRectangle(dowBounds, 0, 0, 0, 0);
}


/***********************************************************************
 *
 * FUNCTION:   	 	DrawTopDataMark
 *
 * DESCRIPTION:		This routine draws a little mark above a given day's
 *					event bar.
 *
 * PARAMETERS:	->	r:		the bounds of the day.
 *              ->	scrool:	true of the view can be scrolled winUp.
 *              ->	noTime:	true of the day has untimed events.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	4/19/95		Initial revision.
 *
 ***********************************************************************/
static void DrawTopDataMark (RectangleType* r, Boolean scroll, Boolean untimed)
{
	int16_t x1, x2, y;

	x1 = r->topLeft.x + dataMarkHMargin;
	x2 = r->topLeft.x + r->extent.x - dataMarkHMargin;
	y = r->topLeft.y + weekViewDayDisplayHeight +
		weekViewDateDisplayHeight - dataMarkVMargin;

	if (scroll)
	{
		// Draw the scroll indicators.
		WinDrawLine (x1, y, x2, y);
		if (untimed)
		{
			// Draw the no-time indicators.
			WinDrawLine (x1, (Coord)(y-1), (Coord)(x1+1), (Coord)(y-1));
		}
		else
		{
			// Erase the no-time indicators.
			//Transparency
			//WinEraseLine (x1, (Coord)(y-1), (Coord)(x1+1), (Coord)(y-1));
		}
	}

	else
	{
		// Draw the no-time indicators and erase the scroll indicators.
		if (untimed)
		{
			WinDrawLine (x1, (Coord)(y-1), (Coord)(x1+1), (Coord)(y-1));
			WinDrawLine (x1, y, (Coord)(x1+1), y);
			//Transparency
			//	WinEraseLine ((Coord)(x1+2), y, x2, y);
		}

		/*
		// Erase both the scroll and no-time indicators.
		else
		{
			//Transparency
			WinEraseLine (x1, (Coord)(y-1), (Coord)(x1+1), (Coord)(y-1));
			WinEraseLine (x1, y, x2, y);
		}
		*/
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DrawBottomDataMark
 *
 * DESCRIPTION:		This routine draws a little mark below a given day's
 *					event bar.
 *
 * PARAMETERS:	->	r:	The bounds of the day to erase.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/3/95		Initial revision.
 *
 ***********************************************************************/
static void DrawBottomDataMark (RectangleType* r)
{
	WinDrawLine((Coord)(r->topLeft.x + dataMarkHMargin),
				(Coord)(r->topLeft.y + r->extent.y + dataMarkVMargin),
				(Coord)(r->topLeft.x + r->extent.x - dataMarkHMargin),
				(Coord)(r->topLeft.y + r->extent.y + dataMarkVMargin));
}


/***********************************************************************
 *
 * FUNCTION:    	EraseBottomDataMark
 *
 * DESCRIPTION:		This routine erases a little mark below a given
 *					day's event bar.
 *
 * PARAMETERS:	->	r:	The bounds of the day to erase.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/3/95		Initial revision.
 *
 ***********************************************************************/
static void EraseBottomDataMark (RectangleType* r)
{
	WinEraseLine((Coord)(r->topLeft.x + dataMarkHMargin),
				(Coord)(r->topLeft.y + r->extent.y + dataMarkVMargin),
				(Coord)(r->topLeft.x + r->extent.x - dataMarkHMargin),
				(Coord)(r->topLeft.y + r->extent.y + dataMarkVMargin));
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawDataMarks
 *
 * DESCRIPTION:		This routine draws little marks above and below each
 *					day's event bar to show that there are more events
 *					in either direction for a given day.
 *
 *					This routine must be broken out separately from the
 *					standard draw routine, because it must function even
 *					when a clip region is set for drawing while scrolling.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/3/95		Initial revision.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewDrawDataMarks (void)
{
	int				i,j;
	Boolean			untimed;
	WeekType*		weekP;
 	ApptInfoPtr		appts;
	Boolean			drawTopMark;
	Boolean			drawBottomMark;

	weekP = MemHandleLock (sWeekObjectH);

	for (i = 0; i < daysInWeek; i++)
	{
		// Check if the are any untimed apppoints.  They'll be first in list
		// of appointment if they exist.
		untimed = false;
		drawTopMark = false;
		drawBottomMark = false;
		if (weekP->days[i].numAppts)
		{
			appts = MemHandleLock (weekP->days[i].apptsH);
			untimed = appts[0].noTimeEvent;
			for (j = 0; j < weekP->days[i].numAppts; j++)
			{
				if (appts[j].overlapState == overlapFirstPart)
					drawBottomMark = true;
				if (appts[j].overlapState == overlapScndPart)
					drawTopMark = true;
			}
			MemHandleUnlock (weekP->days[i].apptsH);
		}

		drawTopMark = drawTopMark || (Boolean)(weekP->days[i].firstEventHour < weekP->startHour);
		DrawTopDataMark(&(weekP->days[i].bounds), drawTopMark, untimed);

		drawBottomMark = drawBottomMark || (Boolean)(weekP->days[i].lastEventHour > weekP->endHour);
		if (drawBottomMark)
			DrawBottomDataMark (&(weekP->days[i].bounds));
		//else
			// Transparency
			//EraseBottomDataMark (&(weekP->days[i].bounds));
	}

	MemHandleUnlock(sWeekObjectH);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawTimes
 *
 * DESCRIPTION:		Draw the time markers winDown the left-hand edge of
 *					the week view display.
 *
 * PARAMETERS:	->	startYPos:	Y position of first hour marker
 *				->	startHour:	first hour marker to draw
 *				->	endHour:	end of display range; may or may not
 *								be the last visible marker.
 *				->	hourHeight:	vertical size of an hour-long event.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/8/95		Initial revision.
 *	KCR	10/3/95		Include minutes in times; right justify times.
 *
 ***********************************************************************/
static void WeekViewDrawTimes (int16_t startYPos, uint16_t startHour, uint16_t endHour, uint16_t hourHeight)
{
 	char				hourStr[timeStringLength + 1];	//	Holds string representing hours.
 	int16_t				hourWidth;
 	int16_t				hourYPos;
 	uint16_t			nextHour;
 	uint16_t			displayHour;
 	FontID				currFont;
 	WinDrawOperation 	mode;


	currFont =  FntSetFont (stdFont);
 	hourYPos = startYPos - FntLineHeight() / 2;

	//	If we are using the 24 hours format
	if (TimeIs24HourFormat(TimeFormat) && endHour > hoursPerDay)
	{
		endHour = hoursPerDay;
	}

	mode = WinSetDrawMode(winOverlay);

	for (nextHour = startHour;
		  nextHour <= endHour;
		  nextHour += weekViewHourDisplayInterval,
		  hourYPos += hourHeight*weekViewHourDisplayInterval)
	{
		char timeSuffix[timeStringLength + 1];

		displayHour = nextHour;
		if (nextHour == hoursPerDay)
			displayHour = 0;

		TimeToAscii ((uint8_t) displayHour, 0, TimeFormat, hourStr);

		if (TimeGetFormatSuffix(TimeFormat, (uint8_t)displayHour, timeSuffix))
		{
			size_t newLength = strlen(hourStr) - strlen(timeSuffix);
			hourStr[newLength] = '\0';
		}

		hourWidth = FntCharsWidth (hourStr, strlen (hourStr));
		WinPaintChars (	hourStr,
	 					strlen (hourStr),
	 					(Coord)(weekViewTimeDisplayWidth - timeHMargin - hourWidth),
						hourYPos);

	}

	WinSetDrawMode(mode);

	FntSetFont (currFont);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawDescription
 *
 * DESCRIPTION:		This routine draws the pop-up description.
 *
 * PARAMETERS:		dirtyRectP: the description pop-up bounds.
 *
 * RETURNED:    	Handle of a window that contains the bits obscured
 *              	by the description.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	3/28/96		Initial revision.
 *	PLE	07/01/09	Added linkedEvtP parameter for event
 *					wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewDrawDescription (const RectangleType * dirtyRectP)
{
	uint16_t 			len;
	uint16_t 			charsToDraw;
	uint16_t 			charsToErase;
	char 				str [descWidthMax];
	char 				dateTimeStr [descWidthMax];
	int16_t 			maxDescWidth;
	char 				*ptr;
	int16_t 			x;
	int16_t				y;
	FontID 				curFont;
	RectangleType 		eraseR;
	char * 				desc;
	WinDrawOperation 	mode;
	RectangleType 		r = *dirtyRectP;
	TimePtr 			startTime = &sTrackingData.start;
	TimePtr 			endTime = &sTrackingData.end;
	DatePtr 			date = &sTrackingData.date;
	LinkedWeekEventPtr 	linkedEvtP = &sTrackingData.linkedEvent;
	RGBColorType		previousBackColor;


	// avoids locking / unlocking the handle when not needed
	if ((!TimeDisplayed )
		&& (sTrackingData.trackingType == nothingSelected
			|| sTrackingData.trackingType == dragAborted))
		return;


	if (sWeekObjectH && !gSecurityPasswordOpened)
	{
		WinPushDrawState();

		mode = WinSetDrawMode(winOverlay);

		curFont = FntSetFont (stdFont);

		// Erase and frame the drawing region with a 3D frame.
		r.topLeft.x++;
		r.topLeft.y++;
		r.extent.x -= 3;
		r.extent.y -= 3;

		WinSetBackColorRGB(&sBackgroundHighlight, &previousBackColor);
		WinEraseRectangle (&r, 2);
		WinSetBackColorRGB(&previousBackColor, NULL);

		WinDrawRectangleFrame (popupFrame , &r);

		// Format and draw the appointments date.
		if (! linkedEvtP->existingLinkedEvent)
		{
			// Draw the date
			DateToDOWDMFormat ((uint8_t) date->month, (uint8_t) date->day, (uint16_t)(date->year + firstYear), ShortDateFormat, str);

			x = r.topLeft.x + weekDescMarginX;
			y = r.topLeft.y + weekDescMarginY;

			WinPaintChars (str, strlen (str), x, y);

			charsToErase = 0;
			if (date->month >= 1 && date->month <= 9)
			{
				charsToErase++;
			}

			if (date->day >= 1 && date->day <= 9)
			{
				charsToErase++;
			}

			// I don't know how mush wider the old day-name was than
			// day-name just drawn, so I'll erase the width of two chatacter
			// just to be save.
			eraseR.topLeft.x = x + FntCharsWidth (str, strlen (str));
			eraseR.topLeft.y = y;
			eraseR.extent.x = FntAverageCharWidth () * 2 +
									FntCharWidth ('1') * charsToErase;
			eraseR.extent.y = FntLineHeight ();

			// Format and draw the appointments start and end times
			TimeToAscii ((uint8_t)startTime->hours, (uint8_t)startTime->minutes, TimeFormat, str);

			len = strlen (str);
			str[len++] = spaceChr;
			str[len++] = '-';
			str[len++] = spaceChr;

			TimeToAscii ((uint8_t)endTime->hours, (uint8_t)endTime->minutes, TimeFormat, &str[len]);

			x = r.topLeft.x + (r.extent.x - weekDescMarginX -
				FntCharsWidth (str, strlen (str)));

			y = r.topLeft.y + weekDescMarginY;

			charsToErase = 0;
			if ((startTime->hours <= 9) ||
				(startTime->hours >= 13 && startTime->hours <= 21))
				charsToErase++;

			if ((endTime->hours <= 9) ||
				(endTime->hours  >= 13 && endTime->hours <= 21))
				charsToErase++;

			if (charsToErase)
			{
				eraseR.topLeft.x = x - (FntCharWidth ('1') * charsToErase);
				eraseR.topLeft.y = y;
				eraseR.extent.x = x - eraseR.topLeft.x;
				eraseR.extent.y = FntLineHeight ();
				// Transparency
				// WinEraseRectangle (&eraseR, 0);
			}

			WinPaintChars (str, strlen (str), x, y);
		}
		else
		// The event is overlapping midnight, print the date accordingly...
		{
			TimeType dispStartTime, dispEndTime;
			DateType dispStartDate, dispEndDate;

			if (TimeToInt(*startTime) == overlapStartTime)
			{
				// The linked event is the day before
				ErrNonFatalDisplayIf(TimeToInt(linkedEvtP->endTime != overlapEndTime),
					"WeekViewDrawDescription: wrong linked event date or time.");

				dispStartTime = linkedEvtP->startTime;
				dispEndTime = *endTime;
				dispEndDate = *date;
				dispStartDate = *date;
				DateAdjust(&dispStartDate, -1);
			}
			else
			{
				// The linked event is the day after
				ErrNonFatalDisplayIf(
					TimeToInt(linkedEvtP->startTime) != overlapStartTime ||
					TimeToInt(*endTime) != overlapEndTime,
					"WeekViewDrawDescription: wrong linked event date or time.");

				dispStartTime = *startTime;
				dispEndTime = linkedEvtP->endTime;
				dispEndDate = *date;
				dispStartDate = *date;
				DateAdjust(&dispEndDate, +1);
			}

			// Draw the separator, erase previous date on same step
			strcpy(dateTimeStr, "           -           ");
			x = r.topLeft.x + (r.extent.x - FntCharsWidth (dateTimeStr, strlen(dateTimeStr))) / 2;
			y = r.topLeft.y + weekDescMarginY;
			WinPaintChars (dateTimeStr, strlen(dateTimeStr), x, y);

			// Now draw the event date
			GetVeryShortDate(&dispStartDate, dateTimeStr, descWidthMax);
			// Add the time
			TimeToAscii ((uint8_t)dispStartTime.hours, (uint8_t)dispStartTime.minutes, TimeFormat, str);
			len = strlen (dateTimeStr);
			dateTimeStr[len++] = ',';
			dateTimeStr[len++] = spaceChr;
			dateTimeStr[len++] = nullChr;
			StrNCat(dateTimeStr, str, descWidthMax);
			x = r.topLeft.x + weekDescMarginX;
			WinPaintChars (dateTimeStr, strlen (dateTimeStr), x, y);

			// Draw the end date
			GetVeryShortDate(&dispEndDate, dateTimeStr, descWidthMax);
			TimeToAscii ((uint8_t)dispEndTime.hours, (uint8_t)dispEndTime.minutes, TimeFormat, str);
			len = strlen (dateTimeStr);
			dateTimeStr[len++] = ',';
			dateTimeStr[len++] = spaceChr;
			dateTimeStr[len++] = nullChr;
			StrNCat(dateTimeStr, str, descWidthMax);
			x = r.topLeft.x + (r.extent.x - weekDescMarginX -
				FntCharsWidth (dateTimeStr, strlen (dateTimeStr)));
			WinPaintChars (dateTimeStr, strlen (dateTimeStr), x, y);
		}

		// Draw the appointment's desciption.
		desc = sTrackingData.apptRec.description;
		if (desc)
		{
			x = r.topLeft.x + weekDescMarginX;
			y = r.topLeft.y + (weekDescMarginY << 1) + FntLineHeight();
			maxDescWidth = r.extent.x - x - weekDescMarginX;

			ptr = StrChr (desc, linefeedChr);

			charsToDraw = (ptr == NULL ? strlen (desc) : ptr - desc);
			WinPaintTruncChars(desc, charsToDraw, x, y, maxDescWidth);
		}

		FntSetFont (curFont);

		WinPopDrawState();
	}
}


/***********************************************************************
 *
 * FUNCTION:     	WeekViewDrawDayLabels
 *
 * DESCRIPTION:  	Draw the day of week and day number labels along the
 *					top of the week view display.
 *
 * PARAMETERS:	 ->	week:	pointer the week object (WeekType.)
 *
 * RETURNED:     	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	8/7/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void WeekViewDayLabels (WeekType *weekP)
{
	int16_t				i;
 	char				dateStr[3];			//	temp storage for string
 											//	representing the date number.
	MemHandle 			dayOfWeekH;
	uint8_t 			dayOfWeek;
	int16_t				x;
	int16_t				y;
	int16_t				dayWidth;
	int16_t				labelWidth;
	FontID 				origFont;
 	char *				dayLabels;
	DayType*			day;
 	DateTimeType		today;
	uint16_t			labelLength;
	char*				label;
	WinDrawOperation 	mode;


	origFont = FntGetFont();

	// Get the resource that contains the first letter of each day.
	dayOfWeekH = DmGetResource(gApplicationDbP, strRsc, daysOfWeekInitialsStrID);
 	dayLabels = MemHandleLock (dayOfWeekH);

	// Calculate length of one item in string */
	labelLength = strlen (dayLabels) / 7;

	// Get today's date.
 	TimSecondsToDateTime(TimGetSeconds(), &today);

 	dayWidth = (weekP->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek;
 	x = weekP->bounds.topLeft.x + weekViewTimeDisplayWidth;
 	y = weekP->bounds.topLeft.y;

	// drawing mode is winOverlay
	mode = WinSetDrawMode(winOverlay);

 	for (i = 0; i < daysInWeek; i++)
 	{
		day = &(weekP->days[i]);

		//	Only draw days which are in range.
		if (day->date.day != outOfRangeDay)
		{
		 	//	If this day's date matches the system date, then we will use
		 	//	boldFont to draw the day & date labels.
		 	if (day->date.year+firstYear == today.year &&
		 		 day->date.month == today.month &&
		 		 day->date.day == today.day)
		 		FntSetFont (boldFont);
		 	else
		 		FntSetFont (stdFont);

		 	//	Draw the day labels over the centers of the bars:
			dayOfWeek = (i + StartDayOfWeek) % daysInWeek;
			label = &dayLabels[labelLength * dayOfWeek];
		 	labelWidth = FntCharsWidth (label, labelLength);
		 	WinPaintChars (label, labelLength, (Coord)(x + (dayWidth - labelWidth) / 2 + 1), y);

		 	//	Draw the date labels under the day labels:
		 	StrIToA (dateStr, day->date.day);
		 	labelWidth = FntCharsWidth (dateStr, strlen(dateStr));
		 	WinPaintChars(dateStr,
		 				strlen(dateStr),
		 				(Coord)(x + (dayWidth - labelWidth) / 2 + 1),
						(Coord)(y + weekViewDayDisplayHeight));
		}
		x += dayWidth;
	}

	MemHandleUnlock(dayOfWeekH);
	DmReleaseResource(dayOfWeekH);

	// restore drawing mode
	WinSetDrawMode(mode);

 	//	Restore the original font
 	FntSetFont (origFont);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDraw3DFrame
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	rect:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
 static void WeekViewDraw3DFrame( const RectangleType* r )
{
	// We are already in natvie coord.
	GcHandle		gc;
	int32_t			offset = 0;
	int32_t			x1;
	int32_t			y1;
	int32_t			x2;
	int32_t			y2;

	x1 = r->topLeft.x - 3;
	y1 = r->topLeft.y - 3;
	x2 = r->topLeft.x + r->extent.x + 3;
	y2 = r->topLeft.y + r->extent.y + 3;

	gc = GcGetCurrentContext();

	// top and left lines
	GcSetColor(gc, shadow.red, shadow.green, shadow.blue, shadow.alpha);
 	GcRectI(gc, x1, y1, x2, y1 + 1);
 	GcRectI(gc, x1, y1, x1 + 1, y2);
	offset++;
 	GcRectI(gc, x1 + offset, y1 + offset, x2 - offset, y1 + offset + 1);
 	GcRectI(gc, x1 + offset, y1 + offset, x1 + offset + 1, y2 - offset);
	offset++;
	GcPaint(gc);

	GcSetColor(gc, 0, 0, 0, 255); // black, opaque
	GcRectI(gc, x1 + offset, y1 + offset, x2 - offset, y1 + offset + 1);
	GcRectI(gc, x1 + offset, y1 + offset, x1 + offset + 1, y2 - offset);
	GcPaint(gc);

	// bottom and right lines
	offset = 0;
	GcSetColor(gc, highlight.red, highlight.green, highlight.blue, highlight.alpha);
 	GcRectI(gc, x1, y2, x2, y2 + 1);
 	GcRectI(gc, x2 - 1, y1, x2, y2);
	offset++;
 	GcRectI(gc, x1 + offset, y2 - offset, x2 - offset, y2 - offset + 1);
 	GcRectI(gc, x2 - offset - 1, y1 + offset, x2 - offset , y2 - offset);
	offset++;
	GcPaint(gc);

	GcSetColor(gc, 0, 0, 0, 255); // black, opaque
	GcRectI(gc, x1 + offset, y2 - offset, x2 - offset, y2 - offset + 1);
	GcRectI(gc, x2 - offset - 1, y1 + offset, x2 - offset , y2 - offset);
	GcPaint(gc);

	GcReleaseContext(gc);

}

/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawExternalGrid
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	weekP:
 *				->	lineYPos:
 *				->	dispStartHour:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void WeekViewDrawExternalGrid (WeekPtr weekP, int16_t lineYPos, uint16_t dispStartHour)
{
	IndexedColorType oldBackColor;
 	int16_t			dayWidth;
 	int16_t			i;
 	int16_t			x;
 	RectangleType	r;

	WinSetCoordinateSystem(kCoordinatesNative);

	//	Draw frame around whole day-area.  Cut rect winDown by one pix on each
	//	side so frame will be in week area, not outside.
	r.topLeft.x = weekP->bounds.topLeft.x + weekViewTimeDisplayWidth + 1;
	r.topLeft.y = lineYPos + 1;		//	Want frame to be on line, not outside.
	r.extent.x = weekP->bounds.extent.x - weekViewTimeDisplayWidth - 2;
	r.extent.y = weekP->hoursDisplayed * weekP->hourHeight - 1;

	WinScaleRectangle(&r);

	oldBackColor = WinSetBackColor(WinRGBToIndex(&WhiteColor));
	WinEraseRectangle(&r, 0);
	WinSetBackColor(oldBackColor);

	//WinDrawRectangleFrame (simpleFrame, &r);
	WeekViewDraw3DFrame(&r);

 	//	Draw background (solid lines).
 	x = WinScaleCoord(weekP->bounds.topLeft.x + weekViewTimeDisplayWidth, false);
 	dayWidth = WinScaleCoord((weekP->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek, false);

 	for (i = 1; i < daysInWeek; i++)
 	{
 		x += dayWidth;
 		WinDrawLine (x, r.topLeft.y, x, (Coord)(r.topLeft.y + r.extent.y));
	}

	WinSetCoordinateSystem(kCoordinatesStandard);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawInternalGrid
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	frm - The currently active (WeekView) form.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void WeekViewDrawInternalGrid (WeekPtr weekP, int16_t lineYPos, uint16_t dispStartHour)
{
	Coord lineStartX;
	Coord lineEndX;
	Coord inc;

 	//	Draw background (dotted lines).  The dotted lines are drawn at
 	//	specific hour intervals from the top of the bar area to the end of
 	//	the hour display.
 	//	Lines will not be drawn for the first & last hours - the frame should
 	//	include this.
	WinSetCoordinateSystem(kCoordinatesNative);

 	if (weekP->startHour == dispStartHour)
 	{
 		lineYPos += weekP->hourHeight*weekViewHourDisplayInterval;
 		dispStartHour += weekViewHourDisplayInterval;
 	}

	lineStartX = WinScaleCoord((Coord)(weekP->bounds.topLeft.x + weekViewTimeDisplayWidth + 1), false);
	lineEndX = WinScaleCoord((Coord)(weekP->bounds.topLeft.x + weekP->bounds.extent.x - 1), false);

	lineYPos = WinScaleCoord(lineYPos, false);
	inc = WinScaleCoord(weekP->hourHeight*weekViewHourDisplayInterval, false);

 	for (;
 		  dispStartHour < weekP->endHour;
 		  dispStartHour += weekViewHourDisplayInterval)
 	{
 		WinDrawLine(lineStartX, lineYPos, lineEndX, lineYPos);

		//	Position of next line
 		lineYPos += inc;
  	}

	WinSetCoordinateSystem(kCoordinatesStandard);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawGrid
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	None
 *
 * RETURNED:    	nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	7/27/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewDrawGrid (void)
{
 	WeekType* 	weekP;
 	int16_t		lineYPos;
 	uint16_t	dispStartHour;


 	WinPushDrawState();

	weekP = MemHandleLock (sWeekObjectH);

 	lineYPos = weekP->bounds.topLeft.y + weekViewDayDisplayHeight + weekViewDateDisplayHeight;
 	dispStartHour = weekP->startHour;

	WinSetForeColor(WinRGBToIndex(&colorLine));

	// Draw the grid with solid lines
	WeekViewDrawExternalGrid (weekP, lineYPos, dispStartHour);

	//	Adjust start hour for times & lines to be on a display interval.
 	while ((dispStartHour % weekViewHourDisplayInterval) != 0)
 	{	//	Display starts on an odd hour
 		lineYPos += weekP->hourHeight;
 		dispStartHour++;
 	}

  	// Draw the grid with dotted lines
  	WeekViewDrawInternalGrid (weekP, lineYPos, dispStartHour);

  	// Restore color
	//WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));

	MemHandleUnlock (sWeekObjectH);

	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDraw
 *
 * DESCRIPTION:		This routine draws the background for the WeekView - the
 *					dotted lines, the hour markers, and then draws each
 *					day.
 *
 * PARAMETERS:  ->	frm:	The currently active (WeekView) form.
 *
 * RETURNED:    	nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	7/27/95		Initial revision
 *	PPL	07/22/03	Use MemHandleUnlock rather than MemPtrUnlock.
 *
 ***********************************************************************/
static void WeekViewDraw (FormPtr frm)
 {
 	int16_t				lineYPos;
 	WeekType			*week;
 	uint16_t			dispStartHour;

	WinPushDrawState();
	week = MemHandleLock (sWeekObjectH);

	// Draw the day numbers and day of week labels.
	WeekViewDayLabels (week);

	//	Determine where start of backround is:
 	lineYPos = week->bounds.topLeft.y
 				+ weekViewDayDisplayHeight
 				+ weekViewDateDisplayHeight;

 	dispStartHour = week->startHour;

	WinSetForeColor(WinRGBToIndex(&colorLine));

	// Draw the grid with solid lines
	WeekViewDrawExternalGrid (week, lineYPos, dispStartHour);

	//	Adjust start hour for times & lines to be on a display interval.
 	while ((dispStartHour % weekViewHourDisplayInterval) != 0)
 	{	//	Display starts on an odd hour
 		lineYPos += week->hourHeight;
 		dispStartHour++;
 	}

  	// Draw the grid with dotted lines
  	WeekViewDrawInternalGrid (week, lineYPos, dispStartHour);

 	//	Draw times
 	WeekViewDrawTimes (lineYPos, dispStartHour, week->endHour, week->hourHeight);

  	// Restore color
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));

 	MemHandleUnlock (sWeekObjectH);

	WinPopDrawState();
 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawDayAppts
 *
 * DESCRIPTION:		Draws the passed appt list within the passed
 *					RectangleType.  This should only be called when there
 *					is a non-0 number of events and a non-0 apptsH
 *					MemHandle.
 *
 * PARAMETERS:	->	dayOfWeek:	day num (0-7]
 *				->	apptsH:		MemHandle of mem chunk containing
 *								the ApptInfoPtr (list).
 *				->	numAppts:	number of entries in apptsH.
 *				->	r:			the day's current event bar; this
 *								includes the top & bottom frame area.
 *								The width is set correctly for frame
 *								drawing over the existing frame, or
 *								rect-filling within the existing frame.
 *				->	startHour:	first displayed hour of the
 *								event bar.
 *				->	endHour:	end of the event bar.
 *				->	hourHeight:	vertical size of an hour-long
 *								event.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/4/95		Initial revision.
 *	KCR	10/4/95		Draw overlapped events side-by-side.
 *	PPL	07/22/03	replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewDayDrawAppts (
	int16_t			dayOfWeek,
	MemHandle 		apptsH,
	int32_t 		numAppts,
	RectangleType* 	r,
	uint16_t 		startHour,
	uint16_t 		endHour,
	uint16_t 		hourHeight)
{
	ApptInfoPtr		appts;
	Boolean			overlapOnLeft;
 	int32_t			i;
 	RectangleType	apptRect;
 	TimeType		lastEndTimeLeft;
 	TimeType		lastEndTimeRight;
 	RGBColorType	highlighFillColor;
 	RGBColorType	overlapFillColor;
 	RGBColorType	overlapFrameColor;
 	RGBColorType	defaultFillColor;
 	RGBColorType	defaultFrameColor;
 	RGBColorType	origRGBBack;
 	RGBColorType	origRGBFore;
 	Boolean			highlighAppt;


	appts = MemHandleLock (apptsH);

	lastEndTimeLeft.hours = 0;
	lastEndTimeLeft.minutes = 0;
	lastEndTimeRight.hours = 0;
	lastEndTimeRight.minutes = 0;

	defaultFrameColor = sBackgroundHighlight;
	defaultFillColor.r = (kMaxRGBValue + sBackgroundHighlight.r) / 2; // lighten the color
	defaultFillColor.g = (kMaxRGBValue + sBackgroundHighlight.g) / 2; // lighten the color
	defaultFillColor.b = (kMaxRGBValue + sBackgroundHighlight.b) / 2; // lighten the color

	// UICaution
	UIColorGetTableEntryRGB(UIWarning, &overlapFillColor);
	UIColorGetTableEntryRGB(UIObjectSelectedFill, &highlighFillColor);

	overlapFrameColor = overlapFillColor;

	overlapFrameColor.r = overlapFrameColor.r >> 1; // div by 2 to have a darker Color
	overlapFrameColor.g = overlapFrameColor.g >> 1; // div by 2 to have a darker Color
	overlapFrameColor.b = overlapFrameColor.b >> 1; // div by 2 to have a darker Color

	WinSetBackColorRGB(&defaultFillColor, &origRGBBack);
	WinSetForeColorRGB(&defaultFrameColor, &origRGBFore );

	for (i = 0; i < numAppts; i++)
	{
		// When we  draw an appointment the user is  currently tracking
		// we draw it using the standard PalmOS highlight color
		highlighAppt = (Boolean)  (WeekViewIsTrackActive()
									&& (sTrackingData.rowID != dbInvalidRowID)
									&& (sTrackingData.rowID == appts[i].rowID));


		//	Only display timed appts.
		if (appts[i].noTimeEvent)
			continue;

		//	Don't display events which end <= startHour
		if (appts[i].endTime.hours < startHour
		|| (appts[i].endTime.hours == startHour
 		&& appts[i].endTime.minutes == 0))
			continue;

		//	Don't display events which start >= endHour
		if (appts[i].startTime.hours >= endHour)
			continue;

		//	Check for overlap with previous appts on both sides:
		if (((appts[i].startTime.hours < lastEndTimeLeft.hours)
 			|| ((appts[i].startTime.hours == lastEndTimeLeft.hours)
 			&& (appts[i].startTime.minutes < lastEndTimeLeft.minutes)))
 		&& ((appts[i].startTime.hours < lastEndTimeRight.hours)
 			|| ((appts[i].startTime.hours == lastEndTimeRight.hours)
 			&& (appts[i].startTime.minutes < lastEndTimeRight.minutes))))
		{
			//	Overlapping appts need to be drawn in two parts - the overlapping
			//	part and the non-overlapping part, if any.
			apptRect = *r;

			//	Select side on which to overlap the bars:
			if (TimeToInt (lastEndTimeLeft) > TimeToInt (lastEndTimeRight))
				overlapOnLeft = false;
			else
				overlapOnLeft = true;

			//	The overlap area ends at the earlier of the appropriate
			//	lastEndTime or the appt's endTime.
			if (overlapOnLeft &&
				 (TimeToInt(appts[i].endTime) > TimeToInt (lastEndTimeLeft)))
				WeekViewGetEventBounds (
							appts[i].startTime,
							lastEndTimeLeft,
							startHour,
							endHour,
							&apptRect,
							hourHeight);
			else if (!overlapOnLeft
 				&& (TimeToInt(appts[i].endTime) > TimeToInt (lastEndTimeRight)))
				WeekViewGetEventBounds (
							appts[i].startTime,
							lastEndTimeRight,
							startHour,
							endHour,
							&apptRect,
							hourHeight);
			else
				WeekViewGetEventBounds (
							appts[i].startTime,
							appts[i].endTime,
							startHour,
							endHour,
							&apptRect,
							hourHeight);

			//	Select side to overlap
			if (!overlapOnLeft)
				AdjustEventRight (&apptRect);
			else
				AdjustEventLeft (&apptRect);

			if (highlighAppt)
				WinSetBackColorRGB(&highlighFillColor, NULL);
			else
				WinSetBackColorRGB(&overlapFrameColor, NULL);

			WinSetForeColorRGB(&overlapFrameColor, NULL);

			WinEraseRectangle(&apptRect, 0);
			WinPaintRectangleFrame(simpleFrame, &apptRect);


			//	Now do the non-overlapping part.
			if ((overlapOnLeft
				&& TimeToInt(appts[i].endTime) <= TimeToInt (lastEndTimeLeft))
			|| (!overlapOnLeft
 				&& TimeToInt(appts[i].endTime) <= TimeToInt (lastEndTimeRight)))
				continue;			//	No non-overlapping part

			apptRect = *r;

			if (overlapOnLeft)
			{
				WeekViewGetEventBounds (
								lastEndTimeLeft,
								appts[i].endTime,
								startHour,
								endHour,
								&apptRect,
								hourHeight);

				AdjustEventLeft (&apptRect);
				UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
			}
			else
			{
				WeekViewGetEventBounds (lastEndTimeRight, appts[i].endTime,
												startHour, endHour, &apptRect,
												hourHeight);
				AdjustEventRight (&apptRect);
				UpdateEndTime (&(appts[i].endTime), &lastEndTimeRight, endHour);
			}


			if (highlighAppt)
				WinSetBackColorRGB(&highlighFillColor, NULL);
			else
				WinSetBackColorRGB(&overlapFillColor, NULL);

			WinEraseRectangle(&apptRect, 0);
			WinPaintRectangleFrame(simpleFrame, &apptRect);

		}

		//	Check for overlap with left side only:
		else if ((appts[i].startTime.hours < lastEndTimeLeft.hours)
			|| ((appts[i].startTime.hours == lastEndTimeLeft.hours)
			&& (appts[i].startTime.minutes < lastEndTimeLeft.minutes)))
		{
			//	draw non-overlapping appt on right side; update right end time

			apptRect = *r;				//	Use pre-calculated x & y coords.

			WeekViewGetEventBounds (
							appts[i].startTime,
							appts[i].endTime,
							startHour,
							endHour,
							&apptRect,
							hourHeight);

			AdjustEventRight (&apptRect);


			if (highlighAppt)
				WinSetBackColorRGB(&highlighFillColor, NULL);
			else
				WinSetBackColorRGB(&overlapFillColor, NULL);

			WinSetForeColorRGB(&overlapFrameColor, NULL);

			WinEraseRectangle(&apptRect, 0);
			WinPaintRectangleFrame(simpleFrame, &apptRect);

			UpdateEndTime (&(appts[i].endTime), &lastEndTimeRight, endHour);
		}

		//	Check for overlap with right side only:
		else if ((appts[i].startTime.hours < lastEndTimeRight.hours)
			|| ((appts[i].startTime.hours == lastEndTimeRight.hours)
			&& (appts[i].startTime.minutes < lastEndTimeRight.minutes)))
		{
			//	draw non-overlapping appt on left side; update left end time

			apptRect = *r;				//	Use pre-calculated x & y coords.

			WeekViewGetEventBounds (
							appts[i].startTime,
							appts[i].endTime,
							startHour,
							endHour,
							&apptRect,
							hourHeight);

			AdjustEventLeft (&apptRect);

			if (highlighAppt)
				WinSetBackColorRGB(&highlighFillColor, NULL);
			else
				WinSetBackColorRGB(&overlapFillColor, NULL);

			WinSetForeColorRGB(&overlapFrameColor, NULL);

			WinEraseRectangle(&apptRect, 0);
			WinPaintRectangleFrame(simpleFrame, &apptRect);

			UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
		}

		//	Appt does not overlap previous appt - check for overlap with
		//	next appt.
		else if (((i + 1) < numAppts)
			&& ((appts[i+1].startTime.hours < appts[i].endTime.hours)
 			|| ((appts[i+1].startTime.hours == appts[i].endTime.hours)
			&& (appts[i+1].startTime.minutes < appts[i].endTime.minutes))))
		{
			//	draw non-overlapping appt on left side; update left end time.

			apptRect = *r;				//	Use pre-calculated x & y coords.

			WeekViewGetEventBounds (
							appts[i].startTime,
							appts[i].endTime,
							startHour,
							endHour,
							&apptRect,
							hourHeight);

			AdjustEventLeft (&apptRect);

			if (highlighAppt)
				WinSetBackColorRGB(&highlighFillColor, NULL);
			else
				WinSetBackColorRGB(&overlapFillColor, NULL);

			WinSetForeColorRGB(&overlapFrameColor, NULL);

			WinEraseRectangle(&apptRect, 0);
			WinPaintRectangleFrame(simpleFrame, &apptRect);

			UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
		}

		else	//	Appt does not overlap previous appts:
		{	//	Draw full width appt; update both left & right end times.
			apptRect = *r;							//	Use pre-calculated x & y coords.
			WeekViewGetEventBounds (
							appts[i].startTime,
							appts[i].endTime,
							startHour,
							endHour,
							&apptRect,
							hourHeight);

			if (highlighAppt)
				WinSetBackColorRGB(&highlighFillColor, NULL);
			else
				WinSetBackColorRGB(&defaultFillColor, NULL);

			WinSetForeColorRGB(&defaultFrameColor, NULL );

			WinEraseRectangle(&apptRect, 0);
			WinPaintRectangleFrame(simpleFrame, &apptRect);

			UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
			UpdateEndTime (&(appts[i].endTime), &lastEndTimeRight, endHour);
		}
	}


	//	Clean up after drawing events
	MemHandleUnlock(apptsH);

	WinSetBackColorRGB(&origRGBBack, NULL);
	WinSetForeColorRGB(&origRGBFore, NULL);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawDay
 *
 * DESCRIPTION:		This routine draws a passed DayObject structure.
 *
 * PARAMETERS:		day:		the day to draw.
 *					startHour:	first displayed hour of the day.
 *					endHour:	end of event bar display.
 *					hourHeight:	vertical size of hour-long event.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	7/31/95		Initial revision.
 * 	KCR	10/3/95		Remove frame around whole day.
 *	KCR	10/13/95	don't draw days which are outside of date range.
 *
 ***********************************************************************/
static void WeekViewDayDraw (
	DayType *		day,
	uint16_t 		startHour,
	uint16_t 		endHour,
	uint16_t 		hourHeight)
 {
 	RectangleType	r;

	//	Only draw days which are in range.
	if (day->date.day == outOfRangeDay)
		return;

 	//	Use day bounds to create event bar bounds
 	WeekViewCreateEventBar (&day->bounds, &r);

 	//	Draw the appointments - including overlaps.  The current appointment
 	//	is highlighted specially.  Only show portions of events which fall
 	//	between startHour & endHour.
	if (day->numAppts != 0)
	{
		//	Pass event bar to the event drawing routine, including the top &
		//	bottom frame areas.
		r.topLeft.y--;
		r.extent.y += 2;

		WeekViewDayDrawAppts (
					day->dayOfWeek,
					day->apptsH,
					day->numAppts,
					&r,
					startHour,
					endHour,
					hourHeight);
 	}
  }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawDays
 *
 * DESCRIPTION:		This routine draws each day.
 *					This should be called after WeekViewInitWeekDays.
 *
 * PARAMETERS:  ->	RectP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	11/13/95	Initial revision.
 *	PPL	07/22/03	Use MemHandleUnlock rather than MemPtrUnlock.
 *
 ***********************************************************************/
static void WeekViewDrawDays (void)
{
	int				i;
 	WeekType		*week;

	week = MemHandleLock (sWeekObjectH);

	for (i = 0; i < daysInWeek; i++)
	{
 		WeekViewDayDraw(
 					&(week->days[i]),
 					week->startHour,
 					week->endHour,
				 	week->hourHeight);
	}

 	MemHandleUnlock (sWeekObjectH);
}


/***********************************************************************
 *
 * FUNCTION:    	DateUtilsWeekViewDrawDate
 *
 * DESCRIPTION:		This routine draws  Day/Date header with
 *					a selected appearance ot not
 *
 * PARAMETERS:	->	r: 			The bounds of the area to be highlighted.
 *				->	inverted:	Do we draw the dateinverted or not?
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GAP	11/16/00	Initial revision.
 *	PPL	07/22/03	Use MemHandleUnlock rather than MemPtrUnlock.
 *	PPL 07/29/03	Add Inverted parameter, and draws date inverted
 *					or not.
 *
 ***********************************************************************/
static void WeekViewDrawDate (const RectangleType *r, Boolean inverted)
{
	WeekType*	weekP;

	WinPushDrawState();

	WinSetPatternType (blackPattern);

	if (inverted)
	{
		WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
		WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
		WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));

		// Erase the hilite bounds to fill it with selected fill color
		// Transparency
		WinEraseRectangle(r, 0);
	}

	// Transparency
	// Erase the hilite bounds to fill it with selected fill color
	//WinEraseRectangle(r, 0);

	// Draw the text.  As we have set the clip to the hilite rect
	// only the selected day/date will be drawn to the screen.
	weekP = MemHandleLock (sWeekObjectH);

	WeekViewDayLabels (weekP);

	MemHandleUnlock (sWeekObjectH);

	// Draw any data marks that may exist in the inverted area
	WeekViewDrawDataMarks();

	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawEmptySlot
 *
 * DESCRIPTION:		This routine draws an empty slot selected or not.
 *
 * PARAMETERS:	->	trackingDataP: event tracking data.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/29/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewDrawEmptySlot(void)
{
	WinPushDrawState();

	if (sTrackingData.isSelected)
	{
		WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
		WinEraseRectangle(&sTrackingData.highR, 0);
	}

	if (sTrackingData.drawFrame)
		WinPaintRectangleFrame(sTrackingData.eventFrame.word, &sTrackingData.highR);

	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:	WeekViewDrawSelection
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	trackingDataP: Tracking information.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR		9/19/95	Initial revision.
 *
 ***********************************************************************/
static void WeekViewDrawSelection (void)
{
	WeekType* 		weekP;
	RectanglePtr	rectP;

	// avoids locking / unlocking the handle when not needed
	if (sTrackingData.trackingType == nothingSelected || sTrackingData.trackingType == dragAborted)
		return;

	weekP = MemHandleLock(sWeekObjectH);

	switch (sTrackingData.trackingType)
	{
		case nothingSelected:
		case dragAborted:
			break;

		case emptySlotSelected:
			WeekViewDrawEmptySlot();
			break;

		case dateSlotSelected:
			WeekViewDrawDate(&sTrackingData.saveR, sTrackingData.isSelected);
			break;

		case trackingApptSlot:
			// Highlight the event.
			WinPaintRectangleFrame (sTrackingData.eventFrame.word, &sTrackingData.eventR);
			rectP = &sTrackingData.linkedEvent.eventR;
			if (sTrackingData.linkedEvent.existingLinkedEvent && (rectP->extent.y != 0))
				WinPaintRectangleFrame (sTrackingData.eventFrame.word, rectP);
			break;


		default:
			ErrNonFatalDisplay("WeekViewDrawSelection: invalid state");

	}

	MemHandleUnlock(sWeekObjectH);
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewUpdateDisplay
 *
 * DESCRIPTION:		Update the Week View display according to the passed
 *					updateCode.
 *
 * PARAMETERS:	->	dirtyRect:	rectangle to invalidate.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/9/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewUpdateDisplay (const RectangleType * dirtyRectP)
{
	FormType* 	frmP;

	PIMAppProfilingBegin("WeekView UpdateDisplay");

	frmP = FrmGetFormPtr(WeekView);

	// Full drawings
	FrmDrawForm (frmP);
	WeekViewDraw (frmP);
	WeekViewDrawDays ();
	WeekViewDrawDataMarks ();

	if (WeekViewIsTrackActive())
		WeekViewDrawDescription(&sTrackingData.descRect);

	PIMAppProfilingEnd();
}

/***********************************************************************
 *
 * FUNCTION:    	WeekViewDrawDescriptionCallback
 *
 * DESCRIPTION:		Window Manager Callback to redraw invalidated
 *					rectangles.
 *
 * PARAMETERS: 	-> 	cmd:		Invalidation command word.
 *				->	window:		Invalidated window.
 *				->	dirtyRectP: Invalidated Rectangle.
 *				->	stateP:		nothing.
 *
 * RETURNED:   		Boolean that tells if invalidated rectangle was drawn
 *					(true) or not (false.)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/29/03	Initial revision.
 *
 ***********************************************************************/
static Boolean WeekViewDrawDescriptionCallback(int32_t cmd, WinHandle window, const RectangleType *dirtyRectP, void *stateP)
{
	if (cmd == winInvalidateDestroy)
		return true;

	// Draw the event description, time, and date.
	WeekViewDrawDescription(dirtyRectP);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:		WeekViewDescriptionInvalidate
 *
 * DESCRIPTION:		Update the Week View display according to the passed
 *					updateCode.
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/9/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewDescriptionInvalidate (void)
{
	RectangleType 	dirtyRect;
	FontID			curFont;

	curFont = FntSetFont (stdFont);

	// Compute the bounds of the drawing region for the description.
	WinGetWindowExtent (&dirtyRect.extent.x, &dirtyRect.extent.y);
	dirtyRect.topLeft.x = 0;
	dirtyRect.topLeft.y = 0;
	dirtyRect.extent.y = (FntLineHeight () * 2) + (weekDescMarginY * 3) + 3;

	FntSetFont (stdFont);

	sTrackingData.descRect = dirtyRect;

	TraceOutput(TL(appErrorClass,"*** DESC Clip = [%d,%d] - [%d,%d] ***",
		dirtyRect.topLeft.x, dirtyRect.topLeft.y, dirtyRect.extent.x, dirtyRect.extent.y));

	// Transparency
	//WinInvalidateRectFunc(gWinHdl, (const RectangleType *) &dirtyRect, WeekViewDrawDescriptionCallback, NULL);
	WinInvalidateRect(gWinHdl, (const RectangleType *) &dirtyRect);
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
 *				->	dirtyRectP: Invalidated Rectangle.
 *				->	stateP:		nothing.
 *
 * RETURNED:   		Boolean that tells if invalidated rectangle was drawn
 *					(true) or not (false.)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/29/03	Initial revision.
 *
 ***********************************************************************/
static Boolean WeekViewUpdateDisplayCallback(int32_t cmd, WinHandle window, const RectangleType *dirtyRectP, void *stateP)
{
	if (cmd == winInvalidateDestroy)
		return true;

	WeekViewUpdateDisplay (dirtyRectP);
	WeekViewDrawSelection ();

	return true;
}

/***********************************************************************
 *
 * FUNCTION:		WeekViewDisplayInvalidate
 *
 * DESCRIPTION:		Update the Week View display according to the passed
 *					updateCode.
 *
 * PARAMETERS:	->	eventP:	pointer to event type structure to handle.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	10/9/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewDisplayInvalidate (RectangleType* dirtyRectP)
{
	RectangleType 	dirtyRect;

	if (dirtyRectP == NULL)
		WinGetBounds (gWinHdl, &dirtyRect);
	else
		dirtyRect = *dirtyRectP;

	TraceOutput(TL(appErrorClass,"*** WEEK Clip = [%d,%d] - [%d,%d] ***",
		dirtyRect.topLeft.x, dirtyRect.topLeft.y, dirtyRect.extent.x, dirtyRect.extent.y));


	WinInvalidateRect(gWinHdl, (const RectangleType *) &dirtyRect);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekEraseDescription
 *
 * DESCRIPTION: 	This routine will erase the description
 *              	popup if been visible.
 *
 * PARAMETERS:  ->	eraseIt:	true to always erase, false erase only if
 *                        		to popup has been display for the require
 *                        		length of time.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/7/96		Initial revision
 *	GGL 2/2/99		Set TimeDisplayed to false to indicate that
 *					the temporary display has been erased.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekEraseDescription (Boolean eraseIt)
{
 	WeekType*		weekP;
	RectangleType 	r;

	weekP = MemHandleLock (sWeekObjectH);

	if (weekP->popupDescDisplayed)
	{
		if ((eraseIt) || TimeToWait() == 0)
		{
			r.topLeft.x = 0;
			r.topLeft.y = 0;

			WinGetWindowExtent (&r.extent.x, &r.extent.y);
			r.extent.y = (FntLineHeight () * 2) + (weekDescMarginY * 3) + 5;

			weekP->popupDescDisplayed = false;

			WeekViewDisplayInvalidate(&r);
		}
	}

 	MemHandleUnlock(sWeekObjectH);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewUpdateScrollers
 *
 * DESCRIPTION:		This routine draws or erases the Week View scroll
 *					arrow buttons depending on whether there is more
 *					data to be displayed in either direction.
 *
 * PARAMETERS:	->	frm:	pointer to the Week View form.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	7/31/95		Initial revision.
 *	KCR	9/20/95		Convert to use FrmUpdateScrollers.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewUpdateScrollers (FormPtr frm)
{
	WeekType*	weekP;
	uint16_t 	upIndex;
	uint16_t 	downIndex;
	Boolean 	scrollableUp;
	Boolean 	scrollableDown;

	weekP = MemHandleLock (sWeekObjectH);

	scrollableUp = (Boolean) (weekP->startHour > weekP->firstEventHour);
	scrollableDown = (Boolean) (weekP->endHour < weekP->lastEventHour);

	upIndex = FrmGetObjectIndex (frm, WeekUpButton);
	downIndex = FrmGetObjectIndex (frm,WeekDownButton);

	FrmUpdateScrollers (frm, upIndex, downIndex, scrollableUp, scrollableDown);

	MemHandleUnlock (sWeekObjectH);
}



/***********************************************************************
 *
 * FUNCTION:    	WeekViewDayInit
 *
 * DESCRIPTION:		This routine initializes a passed DayType structure.
 *					All fields of the DayType struct are filled in and
 *					the week->firstEventHour and week->lastEventHour
 *					vars are maintained.
 *
 * PARAMETERS:	<->	day:			Day structure to init.
 *				->	dayOfWeek 		matches date.
 *				->	bounds:			event's rectangle.
 *				->	firstEventHour:
 *				->	lastEventHour:
 *				->	apptsH:
 *				->	numAppts:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	7/28/95		Initial revision.
 *	PPL	07/22/03	Replaced MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
 static void WeekViewDayInit (
 	DayType*		day,
 	uint8_t 		dayOfWeek,
	RectangleType* 	bounds,
	uint16_t*		firstEventHour,
	uint16_t*		lastEventHour,
	MemHandle 		apptsH,
	int32_t 		numAppts)
 {
 	ApptInfoPtr		appts = NULL;
 	int32_t			i;

 	//	Load day bounds
 	day->bounds = *bounds;

 	//	Load the appt list
 	if (day->date.day != outOfRangeDay)
 	{
	 	day->apptsH = apptsH;
	 	day->numAppts = numAppts;
	 	day->dayOfWeek = dayOfWeek;
	 }
	else	//	Non-displayable date.
	{
		day->apptsH = NULL;
		day->numAppts = 0;
		day->dayOfWeek = 0;
	}

 	day->firstEventHour = 24;
 	day->lastEventHour = 0;

 	//	Stretch the bounds of the scrolling range if there are any events
 	//	outside the current range.  This must be performed for both
 	//	the day and the week.
	if (day->numAppts > 0)
		appts = MemHandleLock (day->apptsH);

 	for (i = 0; i < day->numAppts; i++)
 	{
 		if ((! appts[i].noTimeEvent)
 		&& (appts[i].startTime.hours < (*firstEventHour)))
 		{
 			//	Round winDown to the nearest hour <= start of appt.
 			(*firstEventHour) = appts[i].startTime.hours;
 			day->firstEventHour = appts[i].startTime.hours;
 		}
 		else if ((! appts[i].noTimeEvent)
 		&& (appts[i].startTime.hours < day->firstEventHour))
 			day->firstEventHour = appts[i].startTime.hours;


 		if ((! appts[i].noTimeEvent)
 		&& (appts[i].endTime.hours > (*lastEventHour)
 		|| (appts[i].endTime.hours == (*lastEventHour)
  		&& appts[i].endTime.minutes != 0)))
 		{
 			//	Round winUp to the next hour >= end of the appt.
 			(*lastEventHour) = appts[i].endTime.hours;
 			if (appts[i].endTime.minutes != 0)
 				(*lastEventHour)++;

 			day->lastEventHour = appts[i].endTime.hours;

 			if (appts[i].endTime.minutes != 0)
 				day->lastEventHour++;
 		}

 		else if ((! appts[i].noTimeEvent)
 		&& (appts[i].endTime.hours > day->lastEventHour
 		|| (appts[i].endTime.hours == day->lastEventHour
 		&& appts[i].endTime.minutes != 0)))
 		{
 			//	Round winUp to the next hour >= end of the appt.
 			day->lastEventHour = appts[i].endTime.hours;

 			if (appts[i].endTime.minutes != 0)
 				day->lastEventHour++;
 		}
 	}

 	if (day->numAppts > 0)
 		MemHandleUnlock(day->apptsH);

 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDayClose
 *
 * DESCRIPTION:		This routine frees all allocatd memory involved in
 *					a DayType structure.  It is generally called when a week
 *					is being torn winDown.
 *
 * PARAMETERS:	<->	day: day data structure.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/9/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewDayClose (DayType *day)
 {
 	if (day->numAppts != 0)
 	{
 		MemHandleFree (day->apptsH);
 		day->apptsH = NULL;
 	}
 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewInitWeekDays
 *
 * DESCRIPTION:		Check the data of the days in the current week and
 *					extend the week's time range if nesc.  Assumes the
 *					week has already been initialized.
 *
 *					This routine is performing all of the heavy data
 *					access work, so that the screen can be drawn quickly
 * 					without the burden of the database.
 *
 * PARAMETERS:  ->	frm:		The currently active (WeekView) form.
 *				->	reloadRows: On true reload rows from DB.
 *
 * RETURNED:   	 	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	11/13/95	Initial revision.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewInitWeekDays (FormPtr frm, Boolean reloadRows)
{
 	int				i;
 	RectangleType	dayBounds;
	int16_t			dayWidth;
 	WeekType*		weekP;
	int32_t 		countsA[daysInWeek] = {0};
	MemHandle 		apptListsHA[daysInWeek] = { NULL};
	int32_t			counts = NULL;
	MemHandle 		apptListsH = NULL;


 	ErrFatalDisplayIf(!sWeekObjectH, "Unallocated Week View object");

	weekP = MemHandleLock (sWeekObjectH);

 	//	Initialize the days:
 	dayWidth = (weekP->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek;
 	dayBounds.topLeft.x = weekP->bounds.topLeft.x + weekViewTimeDisplayWidth;
 	dayBounds.topLeft.y = weekP->bounds.topLeft.y;
 	dayBounds.extent.x = dayWidth;
 	dayBounds.extent.y = weekP->hourHeight * weekP->hoursDisplayed +
 								weekViewDayDisplayHeight +
 								weekViewDateDisplayHeight;

	if (reloadRows)
	{
		// Get all the appointments in the week.
		ApptGetAppointments (	ApptDB,
								&gApptCursorID,
								weekP->startDate,
								daysInWeek,
								apptListsHA,
								countsA,
								true);
	}


 	for (i = 0; i < daysInWeek; i++)
 	{
 		if (reloadRows)
 		{
			apptListsH = apptListsHA[i];
			counts = countsA[i];
 		}
 		else
 		{
 		 	apptListsH = weekP->days[i].apptsH;
	 		counts = weekP->days[i].numAppts;
 		}

 		WeekViewDayInit(&(weekP->days[i]),
 						(uint8_t)((i + StartDayOfWeek) % daysInWeek),
 						&dayBounds,
 						&(weekP->firstEventHour),
 						&(weekP->lastEventHour),
 						apptListsH, counts);

		//	Next day bounds
 		dayBounds.topLeft.x += dayWidth;
 	}

 	//	Initialize scroll controls
	WeekViewUpdateScrollers(frm);

 	MemHandleUnlock (sWeekObjectH);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewSetTitle
 *
 * DESCRIPTION: 	This routine set the title of the Week View form.
 *
 * PARAMETERS:  ->	week  - pointer to week object (WeekType.)
 *
 * RETURNED:	 	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	ART	8/7/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewSetTitle (WeekType * weekP)
{
	char			startDate[longDateStrLength];
	char			endDate[longDateStrLength];
	MemHandle 		templateH;
	char* 			templateP;
	char* 			title;
	uint16_t 		startDateID, endDateID, templateID;
	uint16_t 		weekLastDayIndex;
	FormType* 		activeFormP;

	// Get active form
	activeFormP = FrmGetFormPtr(WeekView);

	// Find the last valid day (out of range days are number outOfRangeDay)
	weekLastDayIndex = daysInWeek - 1;
	while (weekLastDayIndex > 0)
	{
		if (weekP->days[weekLastDayIndex].date.day != outOfRangeDay)
		{
			break;
		}
		else
		{
			weekLastDayIndex--;
		}
	}

 	//	Set the title of the form to be the localized month/year date of the
 	//	StartDayOfWeek.  The format shall be "Mmm 'YY - Mmm 'YY", or
 	//	"Mmm - Mmm 'YY" if the year is the same, or "Mmm 'YY" if the month
 	//	is the same.

	if (weekP->startDate.year != weekP->days[weekLastDayIndex].date.year)
	{
		// Start/end year is different, so go for "Mmm 'YY - Mmm 'YY".
		startDateID = WeekViewTitleFullDateStrID;
		endDateID = WeekViewTitleFullDateStrID;
		templateID = WeekViewTitleTwoDatesStrID;
	}
	else if ((weekP->startDate.month != weekP->days[weekLastDayIndex].date.month)
		&& (weekP->days[weekLastDayIndex].date.day != outOfRangeDay))
	{
		// Start/end year is the same, but month is different, so use "Mmm - Mmm 'YY"
		// or "Mmm 'YY - Mmm", depending on weekViewYearFirst soft constant.
		if (ResLoadConstant(gApplicationDbP, weekViewYearFirst) == false)
		{
			startDateID = WeekViewTitleShortDateStrID;
			endDateID = WeekViewTitleFullDateStrID;
		}
		else
		{
			startDateID = WeekViewTitleFullDateStrID;
			endDateID = WeekViewTitleShortDateStrID;
		}

		templateID = WeekViewTitleTwoDatesStrID;
	}
	else
	{
		// Start/end year & month are the same, so use "Mmm 'YY"
		startDateID = WeekViewTitleFullDateStrID;
		endDateID = WeekViewTitleEmptyDateStrID;
		templateID = WeekViewTitleOneDateStrID;
	}

	templateH = DmGetResource(gApplicationDbP, strRsc, startDateID);
	templateP = (char*)MemHandleLock(templateH);


	DateTemplateToAscii(templateP,
						(uint8_t) weekP->startDate.month,
						(uint8_t) weekP->startDate.day,
 						(uint16_t)(weekP->startDate.year + firstYear),
 						startDate,
 						sizeof(startDate) - 1);

 	if (startDateID != endDateID)
 	{
		MemHandleUnlock(templateH);
		DmReleaseResource(templateH);

		templateH = DmGetResource(gApplicationDbP, strRsc, endDateID);

		templateP = (char*)MemHandleLock(templateH);
	}

	DateTemplateToAscii(templateP,
						(uint8_t) weekP->days[weekLastDayIndex].date.month,
						(uint8_t) weekP->days[weekLastDayIndex].date.day,
						(uint16_t)(weekP->days[weekLastDayIndex].date.year + firstYear),
						endDate,
						sizeof(endDate) - 1);

	MemHandleUnlock((MemPtr)templateH);

	templateH = DmGetResource(gApplicationDbP, strRsc, templateID);
	templateP = (char*)MemHandleLock(templateH);

	title = TxtParamString(templateP, startDate, endDate, NULL, NULL);

	MemHandleUnlock(templateH);
	DmReleaseResource(templateH);

	FrmCopyTitle (activeFormP, title);

	MemPtrFree((MemPtr)title);

	TimeDisplayed = false;

	DateUtilsSetCategoryTriggerLabel(activeFormP, WeekCategoryTrigger);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewSetupWeek
 *
 * DESCRIPTION:		This routine initializes the data structures used
 *					for the Week View Gadet used to Draw the week data.
 *
 *					(splitted from WeekViewInitWeek.)
 *
 *					Depending on Active input graffiti area is
 *					-shown :
 *					scrollInterval = weekViewActiveInputShownScrollInterval
 *					hoursDisplayed = weekViewNumActiveInputShownDispHours
 *					-hidden:
 *					scrollInterval = weekViewActiveInpuitHiddenScrollInterval
 *					hoursDisplayed = weekViewNumActiveInputHiddenDispHours
 *
 *
 *
 * PARAMETERS:  ->	frm - The currently active (WeekView) form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	01/31/02	Initial revision.
 *
 ***********************************************************************/
static void WeekViewSetupWeek(FormType* frmP)
{
	int			i;
	int			skipFirstDays = 0;
	int			skipLastDays = 0;
 	DateType	tempDate;
 	WeekType*	weekP;
	uint16_t	index;
 	uint16_t	startDay;

	ErrFatalDisplayIf(!sWeekObjectH, "Week Handle not Allocated");

	weekP = MemHandleLock (sWeekObjectH);

 	//	Set bounds of WeekView
	index  = FrmGetObjectIndex (frmP, WeekDayDisplay);
	FrmGetObjectBounds (frmP, index, &weekP->bounds);

	weekP->bounds.extent.x -= weekHMargin;

 	//	Determine the start date for this week based on the current 'Date' and
 	//	the StartDayOfWeek
 	weekP->startDate.year = Date.year;
 	weekP->startDate.month = Date.month;
 	weekP->startDate.day = Date.day;

 	startDay = DayOfWeek (Date.month, Date.day, (int16_t)(Date.year + firstYear));

	DateBackgroundGetHighlightColor(&sBackgroundHighlight, Date.month);
	sGadgetCallbackData.color = sBackgroundHighlight;

 	//	Current day of week
 	//	Adjust startDate backwards to previous occurrance of StartDayOfWeek
 	if (startDay > StartDayOfWeek)
 	{
 		DateAdjust (&(weekP->startDate), - ((int32_t) (startDay - StartDayOfWeek)));
 	}
 	else if (StartDayOfWeek > startDay)
 	{
 		DateAdjust (&(weekP->startDate), - ((int32_t)(startDay + daysInWeek - StartDayOfWeek)));
 	}


 	//	Make sure we have not run into the start of the legal date
 	//	range.  If this has happened, we will skip the
 	//	dates betweeen the StartDayOfWeek and the beginning of the legal
 	//	date range.
	if ((weekP->startDate.year == 0)
		&& (weekP->startDate.month == 1)
 		&& (weekP->startDate.day == 1))
	{
		startDay = DayOfWeek (1, 1, firstYear);
		if (startDay > StartDayOfWeek)
		{
			skipFirstDays = startDay - StartDayOfWeek;
		}
		else if (StartDayOfWeek > startDay)
		{
			skipFirstDays = daysInWeek + startDay - StartDayOfWeek;
		}
	}
	//	Make sure we have not run into the end of the legal date
	//	range.  If this happens, we will skip the days between the
	//	end of the date range and the week->startDate+daysInWeek.
	else if ((weekP->startDate.year == numberOfYears - 1)
 		&& (weekP->startDate.month == monthsInYear)
 		&& (weekP->startDate.day > (31 - (daysInWeek - 1))))
	{
		startDay = DayOfWeek (1, 1, lastYear);
		if (startDay >= StartDayOfWeek)
		{
			skipLastDays = daysInWeek - (startDay - StartDayOfWeek) - 1;
		}
		else
		{
			skipLastDays = daysInWeek - (startDay + daysInWeek - StartDayOfWeek) - 1;
		}
	}

	// set the number of hours to be displayed depends on available height
	// not on the Pin State (portrait versus landscape mode)
	if (gCurrentWinBounds.extent.y <= 160)
	{
		// Tall height - can show less rows in table
		weekP->hoursDisplayed = weekViewNumActiveInputShownDispHours;
		weekP->scrollInterval = weekViewActiveInputShownScrollInterval;
	}
	else
	{
		// Tall height - can show more rows in table
		weekP->hoursDisplayed = weekViewNumActiveInputHiddenDispHours;
		weekP->scrollInterval =  weekViewActiveInputHiddenScrollInterval;
	}

 	//	week->startDate is now the date of the first instance of
 	//	StartDayOfWeek <= the current Date.  An exception occurs when
 	//	the beginning of the legal date range has been hit.  In this case,
 	//	week->startDate falls on StartDayOfWeek+skipFirstDays.

 	//	Determine the scrolling range, and the initial displayed time range:
 	//
 	//	These vars hold the elligible scrolling range for the current week.
 	//	The range will be expanded as necessary as the days' events are
 	//	examined.
 	weekP->firstEventHour = DayStartHour;
 	weekP->lastEventHour = DayEndHour;

 	//	If range is still less than a screenful, stretch it.  End of display
 	//	may not extend past midnight, however.
 	if ((weekP->lastEventHour - weekP->firstEventHour < weekP->hoursDisplayed)
  		&& (weekP->firstEventHour + weekP->hoursDisplayed > hoursPerDay))
 	{	//	Have to stretch range at both ends
 		weekP->lastEventHour = hoursPerDay;
 		weekP->firstEventHour = weekP->lastEventHour - weekP->hoursDisplayed;
 	}
 	else if (weekP->lastEventHour - weekP->firstEventHour < weekP->hoursDisplayed)
 	{
 		weekP->lastEventHour = weekP->firstEventHour + weekP->hoursDisplayed;
 	}

 	weekP->hourHeight = (weekP->bounds.extent.y -
 								 weekViewDayDisplayHeight -
 								 weekViewDateDisplayHeight) / weekP->hoursDisplayed;
 	//	The display will initially show all events from the start of the
 	//	user's day range to the end of the vertical screen space.  If there
 	//	is room for more hours of data than the user's day range specified,
 	//	a full screenful will be shown anyway.
 	weekP->startHour = weekP->firstEventHour;
 	weekP->endHour = weekP->firstEventHour + weekP->hoursDisplayed;


	//	Set the dates of each day so they can be drawn before initializing the
	//	days.
 	tempDate = weekP->startDate;
	for (i = 0; i < daysInWeek; i++)
	{
 		if ((i < skipFirstDays)
 			|| (i >= (daysInWeek - skipLastDays)))
 		{
 			weekP->days[i].date.day = outOfRangeDay;
 		}
 		else
 		{
			weekP->days[i].date = tempDate;
 			DateAdjust (&tempDate, 1);						//	Next date
		}
	}
	weekP->popupDescDisplayed = false;

	 //	Set the title of the form
 	WeekViewSetTitle (weekP);

 	// set focusing data

 	MemHandleUnlock (sWeekObjectH);
}

 /***********************************************************************
 *
 * FUNCTION:    	WeekViewInitLunarCalendar
 *
 * DESCRIPTION: 	This routine initializes the "Week View" Lunar of the
 *              	Datebook application.
 *
 * PARAMETERS:  ->	frm:	pointer to the week view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	03/10/04	from PalmOS 5.4
 *
 ***********************************************************************/
void WeekViewInitLunarCalendar(FormPtr formP)
{
 	// Display the Lunar View push button if we've got support for it.
	 if (DateSupportsLunarCalendar())
	 {
		 FrmShowObject (formP, FrmGetObjectIndex(formP, WeekMonthLunarViewButton));
	 }
 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewInitWeekUI
 *
 * DESCRIPTION:		This routine initializes week view user interface
 *					and data structure whithout performing any memory
 *					allocation.
 *
 *
 * PARAMETERS:  ->	frm - The currently active (WeekView) form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	16/07/03	Initial revision.
 *					Splitted from WeekViewInitWeekUI
 *					to avoid freeing memory into WeekViewGotoWeek.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *
 ***********************************************************************/
static void WeekViewInitWeekUI (FormPtr formP)
 {
 	uint16_t	weekNumber;
	char 		weekNumStr[16];
	MemHandle	templateH;
	char		*templateP;
	char		*weekString;


	// Create the week number string. We'll load the template string
	// and then substitute in the number string.
	weekNumber = GetWeekNumber (Date.month, Date.day, (uint16_t)(Date.year + firstYear));
	StrIToA(weekNumStr, weekNumber);
	if (strlen(weekNumStr) == 1)
	{
		weekNumStr[1] = weekNumStr[0];
		weekNumStr[0] = chrNumericSpace;
		weekNumStr[2] = '\0';
	}

	templateH = DmGetResource(gApplicationDbP, strRsc, weekNumberTemplateStrID);
	templateP = (char*)MemHandleLock(templateH);

	weekString = TxtParamString(templateP, NULL, weekNumStr, NULL, NULL);
	FrmCopyLabel(formP, WeekNumberLabel, weekString);

	MemPtrFree(weekString);

	MemHandleUnlock(templateH);
	DmReleaseResource(templateH);

	// Lunar Calendar
	WeekViewInitLunarCalendar(formP);

	// Highlight the Week View push button.
	FrmSetControlGroupSelection (formP, DayViewGroup, WeekWeekViewButton);
 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewShowTime
 *
 * DESCRIPTION: 	This routine display the current time in the title
 *					of the month view.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:        	The global variable TimeDisplayed is set to true by this
 *              	routine.
 *
 * REVISION HISTORY:
 *	ART	7/15/96		Initial revision.
 *	GGL	2/2/99		Don't use EvtSetNullEventTick().
 *
 ***********************************************************************/
static void WeekViewShowTime (void)
{
	char			title[timeStringLength];
	DateTimeType 	dateTime;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	TimeToAscii ((uint8_t) dateTime.hour, (uint8_t) dateTime.minute, TimeFormat, title);
	FrmCopyTitle (FrmGetFormPtr(WeekView), title);

	TimeDisplayed = true;
	TimeDisplayTick = TimGetTicks() + timeDisplayTime;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewHideTime
 *
 * DESCRIPTION: 	If the title of the Week View is displaying the current
 *              	time, this routine will change the title to the standard
 *					 title (the current date).
 *
 * PARAMETERS:  	Nothing.
 *
 * PARAMETERS: ->	hide:	true to always hide, false hide only if
 *                     		to time has been display for the require
 *                      	length of time.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/14/96		Initial revision.
 *	GGL	2/2/99		Use TimeToWait(), don't use EvtSetNullEventTick().
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewHideTime (Boolean hide)
{
	WeekType* 	weekP;

	if (TimeDisplayed)
	{
		if (hide || TimeToWait() == 0)
		{
			weekP = MemHandleLock (sWeekObjectH);
			WeekViewSetTitle (weekP);
			MemHandleUnlock (sWeekObjectH);
		}
	}

	TimeComputeWaitTime = false;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewInitWeekWeek
 *
 * DESCRIPTION:		This routine initializes the structures used for the
 *					WeekView.  The start date will be set and the start
 *					and end of the display range will be set.
 *
 *
 * PARAMETERS:  ->	frm:	The currently active (WeekView) form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	7/27/95		Initial revision
 *	KCR	10/4/95		New title date format
 *	KCR	10/9/95		Make sure lastEventHour does not get adjusted past
 *					midnight. Bug #260.
 *	KCR	10/12/95	Handles beginning of date range & end of date
 *					range.
 *	KWK	07/07/99	Creates date range title in localizable manner.
 *	PPL	31/01/02	Added support for active input area.
 *					Split week data init into WeekViewSetupWeek().
 *	PPL 21/04/04	rename into WeekViewInitWeekWeek and
 *					add focusing data init.
 *
 ***********************************************************************/
static void WeekViewInitWeek (FormPtr frmP)
 {
 	WeekType* weekP;

 	//	Allocate currWeek object & day objects
 	sWeekObjectH = MemHandleNew (sizeof (WeekType));
 	ErrFatalDisplayIf(!sWeekObjectH, "Allocation error");

	if (sWeekObjectH)
	{
		weekP = (WeekType*) MemHandleLock(sWeekObjectH);

		// init week focusing data
		RctSetRectangle(&weekP->focusedZone, 0, 0, 0, 0);
		RctSetRectangle(&weekP->focusTrigger, 0, 0, 0, 0);

		weekP->focusState = kWeekFocusNoFocus;
		weekP->focusedDay = 0;
		weekP->focusedRowId = dbInvalidRowID;
		weekP->focusedOverlaps = false;

		MemHandleUnlock(sWeekObjectH);

		WeekViewInitWeekUI(frmP);
	}
 }


/***********************************************************************
 *
 * FUNCTION:    	WeekViewFree
 *
 * DESCRIPTION:		This routine is called when the WeekView is being torn
 *					winDown.  It's primary function is to free allocated
 *					memory.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/9/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewFree (void)
 {
 	int			i;
 	WeekType	*week;

	WeekEraseDescription (true);

	week = MemHandleLock (sWeekObjectH);

	//	Close each day structure
	for (i = 0; i < daysInWeek; i++)
		WeekViewDayClose (&(week->days[i]));

	//	Free WeekType obj.
	MemHandleFree (sWeekObjectH);
	sWeekObjectH = NULL;
 }

/***********************************************************************
 *
 * FUNCTION:    	WeekViewResetLinkedEvent
 *
 * DESCRIPTION:  	Creates or updates a linked event from initial params.
 *
 * PARAMETERS: 	-> 	week:
 *				->	dayOfWeek:
 *				->	timeDiffP:
 *				->	apptRecP:
 *				->	overlapState:
 *				<->	linkedEventP:
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	7/1/02		Support for events wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewResetLinkedEvent(
	WeekType* 					week,
	int16_t 					dayOfWeek,
	TimePtr 					timeDiffP,
	ApptDBRecordPtr 			apptRecP,
	MidnightOverlapStateType 	overlapState,
	LinkedWeekEventPtr 			linkedEventP)
{
	uint32_t 					currDateInDays;

	// Do not create the linked event if the event does not overlapp !
	if (overlapState == overlapNone)
	{
		linkedEventP->dayOfWeek = 0;
		linkedEventP->startTime = apptRecP->when.startTime;
		linkedEventP->endTime = apptRecP->when.endTime;

		TimeDifference (
			&apptRecP->when.endTime,
			&apptRecP->when.startTime,
			false,
			timeDiffP);

		linkedEventP->existingLinkedEvent = false;
	}
	else if (overlapState == overlapFirstPart)
	{
		// Set the linked event, corresponding to the following day
		linkedEventP->dayOfWeek = dayOfWeek + 1;

		// The linked day can be displayed
		linkedEventP->endTime = apptRecP->when.endTime;
		TimeToInt(linkedEventP->startTime) = overlapStartTime;

		// Current is the first part of the appointment (before 12am)
		DateToInt(apptRecP->when.endTime) = overlapEndTime;
		TimeDifference (
			&linkedEventP->endTime,
			&apptRecP->when.startTime,
			true,
			timeDiffP);

		linkedEventP->existingLinkedEvent = true;
	}
	else if (overlapState == overlapScndPart)
	{
		currDateInDays = DateToDays(week->startDate) + dayOfWeek;
		// Set the linked event, corresponding to the previous day
		linkedEventP->dayOfWeek = dayOfWeek - 1;

		// The linked day can be displayed
		linkedEventP->startTime = apptRecP->when.startTime;
		TimeToInt(linkedEventP->endTime) = overlapEndTime;

		// Current is the second part of the appointment (after 12am)
		TimeToInt(apptRecP->when.startTime) = overlapStartTime;
		DateDaysToDate(currDateInDays, &apptRecP->when.date);
		TimeDifference (
			&apptRecP->when.endTime,
			&linkedEventP->startTime,
			true,
			timeDiffP);

		linkedEventP->existingLinkedEvent = true;
	}
	else
	{
		ErrNonFatalDisplay("WeekViewSelectEvent: wrong event date.");
		// the date is wrong: we remove the linked event !

		linkedEventP->dayOfWeek = 0;
		linkedEventP->startTime = apptRecP->when.startTime;
		linkedEventP->endTime = apptRecP->when.endTime;

		TimeDifference (
			&apptRecP->when.endTime,
			&apptRecP->when.startTime,
			false,
			timeDiffP);

		linkedEventP->existingLinkedEvent = false;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewMoveLinkedEvent
 *
 * DESCRIPTION:  	Move day per day the linked event - enabled it to
 *					follow the selected one.
 *
 * PARAMETERS:  ->	prevDayOfWeek:
 *				->	newDayOfWeek:
 *				<->	linkedEventP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE		7/1/02	Support for events wrapping around 12am.
 *
 ***********************************************************************/
static void WeekViewMoveLinkedEventToAnotherDay(
	int16_t 			prevDayOfWeek,
	int16_t 			newDayOfWeek,
	LinkedWeekEventPtr linkedEventP)
{
	if (! linkedEventP->existingLinkedEvent)
		return;

	// Check if the linked event is before or after the main one
	if (linkedEventP->dayOfWeek == prevDayOfWeek - 1)
	{
		// The linked event is before
		linkedEventP->dayOfWeek = newDayOfWeek - 1;
	}
	else if (linkedEventP->dayOfWeek == prevDayOfWeek + 1)
	{
		// The linked event is after
		linkedEventP->dayOfWeek = newDayOfWeek + 1;
	}
	else
	{
		// Tracking was too fast, unable to follow - Reset it now...
		linkedEventP->existingLinkedEvent = false;
		memset(&linkedEventP->eventR, 0x00, sizeof (RectangleType));
	}
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewMoveEvent
 *
 * DESCRIPTION:  	Move the current appointment and its linked event
 *					up and down.
 *
 * PARAMETERS:  -> 	posInDay: the begin position of the selected event.
 *				-> 	prevPosInDay: previous position
 *					(needed to know if we moved)
 *				<-> mainStartP: the selected event start position
 *				<-> mainEndP: the selected event end position
 *				-> 	initialEndTime: initial end postion before drag
 *				-> 	timeDiff: the initial event duration
 *				-> 	hourHeight:
 *				<-> linkedEventP: linked event information
 *
 * RETURNED:    	true if the event really moved.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	7/1/02		Support for events wrapping around 12am.
 *
 ***********************************************************************/
Boolean WeekViewMoveEvent(
	int16_t 			posInDay,
	int16_t 			prevPosInDay,
	int16_t 			mainDayOfWeek,
	TimePtr 			mainStartP,
	TimePtr 			mainEndP,
	TimeType 			initialEndTime,
	TimeType 			timeDiff,
	uint16_t 			hourHeight,
	LinkedWeekEventPtr 	linkedEventP)
{
	int32_t				minutesOnNextDay;
	int32_t				minutesOnPrevDay;

	// Did we move the event ? Granularity is 30 minutes (hourHeight / 2)
	//if (((posInDay  *2 ) / hourHeight) == ((prevPosInDay *2) / hourHeight))
	//	return false;

	if (posInDay  == prevPosInDay)
		return false;

	//  FIRST CASE: NO LINKED EVENT BEFORE MOVE
	if (! linkedEventP->existingLinkedEvent)
	{
		// Do not handle for now back to previous day
		if (posInDay < 0)
			posInDay = 0;

		WeekViewComputeStartTimeFromPos(posInDay, hourHeight, mainStartP);

		// Check if the moved event still stays in a single column
		minutesOnNextDay = (int32_t)(timeDiff.hours * hoursInMinutes
							+ timeDiff.minutes
							- ((hoursPerDay - mainStartP->hours - 1) * hoursInMinutes
								+ hoursInMinutes - mainStartP->minutes));

		if (minutesOnNextDay > 0)
		{
			// The event overlap on next day: create new linked event
			linkedEventP->existingLinkedEvent = true;
			linkedEventP->dayOfWeek = mainDayOfWeek + 1;

			DateToInt(*mainEndP) = overlapEndTime;
			DateToInt(linkedEventP->startTime) = overlapStartTime;

			linkedEventP->endTime.hours = (uint8_t) (minutesOnNextDay / hoursInMinutes);
			linkedEventP->endTime.minutes = (uint8_t) (minutesOnNextDay % hoursInMinutes);

			while (linkedEventP->endTime.minutes >= hoursInMinutes)
			{
				linkedEventP->endTime.hours++;
				linkedEventP->endTime.minutes -= hoursInMinutes;
			}
		}
		else
		{
			// Simple one-day event: update the end hour
			mainEndP->hours = mainStartP->hours + timeDiff.hours;
			mainEndP->minutes = mainStartP->minutes + timeDiff.minutes;

			while (mainEndP->minutes >= hoursInMinutes)
			{
				mainEndP->hours++;
				mainEndP->minutes -= hoursInMinutes;
			}

			if (mainEndP->hours > maxHours)
			{
				mainEndP->hours = maxHours;
				mainEndP->minutes = maxMinutes;
			}
		}
	}
	// SECOND CASE: EXISTED LINKED EVENT BEFORE MOVE
	else
	{
		// Check if the linked event is the day before main or after main
		if (linkedEventP->dayOfWeek == mainDayOfWeek - 1)
		{
			// The linked event is the day before: handle negative posInDay
			WeekViewComputeEndTimeFromPos(
						posInDay,
						initialEndTime,
						mainEndP,
						hourHeight);

			minutesOnPrevDay = (int32_t)(timeDiff.hours * hoursInMinutes + timeDiff.minutes
								- (mainEndP->hours * hoursInMinutes + mainEndP->minutes));

			if (minutesOnPrevDay > 0)
			{
				TimeToInt(*mainStartP) = overlapStartTime;

				linkedEventP->startTime.hours = (uint8_t) ((hoursPerDay-1) - minutesOnPrevDay / hoursInMinutes);
				linkedEventP->startTime.minutes = (uint8_t) (hoursInMinutes - (minutesOnPrevDay % hoursInMinutes));

				if (linkedEventP->startTime.minutes >= hoursInMinutes)
				{
					linkedEventP->startTime.hours++;
					linkedEventP->startTime.minutes -= hoursInMinutes;
				}
			}
			else
			{
				// Returned to 1 column
				mainStartP->hours = mainEndP->hours - timeDiff.hours;
				if (mainEndP->minutes < timeDiff.minutes)
				{
					mainStartP->minutes = hoursInMinutes + mainEndP->minutes - timeDiff.minutes;
					mainStartP->hours--;
				}
				else
				{
					mainStartP->minutes = mainEndP->minutes - timeDiff.minutes;
				}
				linkedEventP->existingLinkedEvent = false;
			}

			// Apply update on current end date

		}
		else if (linkedEventP->dayOfWeek == mainDayOfWeek + 1)
		{
			// The linked event is the day after: Do not handle negative posInDay
			if (posInDay < 0)
				posInDay = 0;

			// Update the start bar time
			WeekViewComputeStartTimeFromPos(posInDay, hourHeight, mainStartP);

			minutesOnNextDay = (int32_t)(timeDiff.hours * hoursInMinutes + timeDiff.minutes
								-((hoursPerDay - mainStartP->hours - 1) * hoursInMinutes
								+ hoursInMinutes - mainStartP->minutes));

			// Check if the event moved from 2 columns to 1
			if (minutesOnNextDay > 0)
			{
				// Still on 2 columns
				linkedEventP->endTime.hours = (uint8_t) (minutesOnNextDay / hoursInMinutes);
				linkedEventP->endTime.minutes = (uint8_t) (minutesOnNextDay % hoursInMinutes);
			}
			else
			{
				// Returned to 1 column
				linkedEventP->existingLinkedEvent = false;
			}
		}
		else
			ErrNonFatalDisplay("WeekViewMoveEvent, Linked day of week invalid");
	}

	// The event moved
	return true;
}

/***********************************************************************
 *
 * FUNCTION:    	WeekViewSelectEventInitialize
 *
 * DESCRIPTION:  	This routine draws the selection highlight around
 *					and move the event.
 *				 	This routine leave the record unlock - take care
 *					to unlock it after.
 *
 * PARAMETERS:  ->	week:
 *              ->	dayOfWeek:
 *              ->	eventP: _>> x, y
 *             	-> 	rowID:
 *				->	overlapState:
 *				->	displayTicks:
 *				->	trackingDataP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	3/29/96		Initial revision.
 *	PLE	7/1/02		Support for events wrapping around 12am.
 *	PPL	07/15/03	cursors !
 * 	PPL 04/23/04	Change event parameter to  x, y codinates.
 *
 ***********************************************************************/
static void WeekViewSelectEventInitialize (
	int16_t 					dayOfWeek,
	Coord						x,
	Coord						y,
	uint32_t 					rowID,
	MidnightOverlapStateType 	overlapState,
	int32_t 					displayTicks)
{
	uint16_t					hourHeight;
	WeekType*					weekP;

	weekP = MemHandleLock (sWeekObjectH);

	// Initializations
	sTrackingData.rowID = rowID;
	sTrackingData.dayOfWeek = dayOfWeek;
	sTrackingData.overlapState = overlapState;
	sTrackingData.displayTicks = displayTicks;

	// Get original record
	ApptGetRecord (
				ApptDB,
				rowID,
				&sTrackingData.apptRec,
				DBK_SELECT_TIME | DBK_SELECT_DESCRIPTION);

	// set again  the linked event if it exists, assign timeDiff from the real record
	WeekViewResetLinkedEvent(
				weekP,
				dayOfWeek,
				&sTrackingData.timeDiff,
				&sTrackingData.apptRec,
				overlapState,
				&sTrackingData.linkedEvent);

	sTrackingData.start = sTrackingData.apptRec.when.startTime;
	sTrackingData.end = sTrackingData.apptRec.when.endTime;
	sTrackingData.initialEnd = sTrackingData.end;
	sTrackingData.date = weekP->days[dayOfWeek].date;

	hourHeight = weekP->hourHeight;

	WeekViewComputeTresholdBounds(x, y, moveThreshold, &sTrackingData.moveR);

	WeekViewComputeStrictDayBounds(&weekP->days[dayOfWeek].bounds, &sTrackingData.dayR);

	sTrackingData.weekR = weekP->bounds;
	sTrackingData.weekR.topLeft.x = weekP->days[0].bounds.topLeft.x;
	sTrackingData.weekR.extent.x = weekP->days[0].bounds.extent.x * daysInWeek;
 	sTrackingData.weekR.topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight;
 	sTrackingData.weekR.extent.y = weekP->hourHeight * weekP->hoursDisplayed;

	sTrackingData.weekAndMarksR = sTrackingData.weekR;
	sTrackingData.weekAndMarksR.topLeft.y -= dataMarkVMargin;
	sTrackingData.weekAndMarksR.extent.y += 2 * dataMarkVMargin + 1;

	sTrackingData.eventFrame.word = 0;
	sTrackingData.eventFrame.bits.width = weekViewEventFrameWidth;

	sTrackingData.newDayOfWeek = dayOfWeek;
	sTrackingData.savedDayOfWeek = dayOfWeek;
	sTrackingData.prevPosInDay = 0;

	// Compute the bounds of the event.
	WeekViewComputeEventBounds(weekP, dayOfWeek, sTrackingData.start, sTrackingData.end,
		hourHeight, &sTrackingData.eventR);

	WeekViewComputeLinkedEventBounds(weekP, hourHeight, &sTrackingData.linkedEvent);

	weekP->popupDescDisplayed = true;

	sTrackingData.posInAppt = y - sTrackingData.dayR.topLeft.y -
					((sTrackingData.start.hours - weekP->startHour) * hourHeight) -
					((sTrackingData.start.minutes * hourHeight) / hoursInMinutes);

	MemHandleUnlock(sWeekObjectH);

	// Invalidate the week display
	WeekViewDisplayInvalidate(&sTrackingData.weekAndMarksR);
	WeekViewDescriptionInvalidate();
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewReselectEvent
 *
 * DESCRIPTION:  	Reselect the event with data  stored in the datadase.
 *
 * PARAMETERS:  ->	weekP: 			weektype (lockled handle) data.
 *				->	trackingDataP: 	tracking information.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	08/05/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewReselectEvent(WeekType* weekP)
{
	// Re-assign initial params
	ApptFreeRecordMemory (&sTrackingData.apptRec);

	ApptGetRecord (
				ApptDB,
				sTrackingData.rowID,
				&sTrackingData.apptRec,
				DBK_SELECT_TIME  | DBK_SELECT_DESCRIPTION);

	sTrackingData.dayOfWeek = sTrackingData.savedDayOfWeek;
	sTrackingData.date = weekP->days[sTrackingData.dayOfWeek].date;

	// set again  the linked event if it exists, assign timeDiff from the real record
	WeekViewResetLinkedEvent(
				weekP,
				sTrackingData.dayOfWeek,
				&sTrackingData.timeDiff,
				&sTrackingData.apptRec,
				sTrackingData.overlapState,
				&sTrackingData.linkedEvent);

	sTrackingData.start = sTrackingData.apptRec.when.startTime;
	sTrackingData.end = sTrackingData.apptRec.when.endTime;
	sTrackingData.initialEnd = sTrackingData.end;

	// Reset the day rect back to the original event so if the pen
	// returns to the week, and that's a different day than the
	// original, the difference will be noted and the date display
	// updated.
	WeekViewComputeStrictDayBounds(&weekP->days[sTrackingData.savedDayOfWeek].bounds, &sTrackingData.dayR);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewMoveEventToAnotherDay
 *
 * DESCRIPTION:  	Actually moves the event and its (eventual) linked
 *					event to another day.
 *
 * PARAMETERS:  ->	weekP: 			weektype (lockled handle) data.
 *				->	trackingDataP: 	tracking information.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	08/05/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewMoveEventToAnotherDay(WeekType* weekP)
{
	WeekViewComputeStrictDayBounds(&weekP->days[sTrackingData.newDayOfWeek].bounds, &sTrackingData.dayR);

	sTrackingData.date = weekP->days[sTrackingData.savedDayOfWeek].date;


	DateAdjust (&sTrackingData.date, (int32_t)sTrackingData.newDayOfWeek - (int32_t)sTrackingData.savedDayOfWeek);

	WeekViewMoveLinkedEventToAnotherDay(
							sTrackingData.dayOfWeek,
							sTrackingData.newDayOfWeek,
							&sTrackingData.linkedEvent);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewSelectEventTrack
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	eventP: 		pointer to Event Structure.
 *				->	trackingDataP: 	tracking information.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	11/19/02	Initial revision.
 *	PPL	08/05/03	More code factorization.
 *
 ***********************************************************************/
static void WeekViewSelectEventTrack (EventType* eventP)
{
	int16_t			x, y;
	int16_t			posInDay;
	uint16_t		hourHeight;
	WeekType*		weekP;

	weekP = MemHandleLock (sWeekObjectH);

	x = eventP->screenX;
	y = eventP->screenY;
	hourHeight = weekP->hourHeight;

	// The pen must be move pass a threshold before we will start
	// moving the event.  This threshold is greater than the distance
	// that would normally be traveled to move an event.
	if (RctPtInRectangle (x, y, &sTrackingData.moveR))
		goto ExitWeekViewSelectEventTrack;
	else
		WeekViewComputeTresholdBounds(x, y, moveThreshold, &sTrackingData.moveR);

	// If we're moved outside the bound of the week, highlight to
	// original event again.
	if (! RctPtInRectangle (x, y, &sTrackingData.weekR))
	{
		if ((TimeToInt (sTrackingData.start) == TimeToInt(sTrackingData.apptRec.when.startTime))
		&& (DateToInt (sTrackingData.date) == DateToInt(weekP->days[sTrackingData.dayOfWeek].date)))
			goto ExitWeekViewSelectEventTrack;

		WeekViewReselectEvent(weekP);
	}

	// Have we moved the event to another day.
	else if (! RctPtInRectangle (x, y, &sTrackingData.dayR))
	{
		// sTrackingData.dayR is a subset of sTrackingData.weekR

		for (sTrackingData.newDayOfWeek = 0;
			sTrackingData.newDayOfWeek < daysInWeek;
			++sTrackingData.newDayOfWeek)
		{
			if (RctPtInRectangle (x, y, &weekP->days[sTrackingData.newDayOfWeek].bounds))
				break;
		}

		WeekViewMoveEventToAnotherDay(weekP);
	}

	// Based on the position of the pen compute the time of the event.
	else
	{
		// We are moving the event inside the day
		posInDay = weekP->startHour * hourHeight
				+ (y  - sTrackingData.dayR.topLeft.y - sTrackingData.posInAppt);

		if (weekP->days[sTrackingData.newDayOfWeek].date.day != outOfRangeDay)
		{
			if (sTrackingData.newDayOfWeek == sTrackingData.dayOfWeek
			&& !WeekViewMoveEvent(
							posInDay,
							sTrackingData.prevPosInDay,
							sTrackingData.newDayOfWeek,
							&sTrackingData.start,
							&sTrackingData.end,
							sTrackingData.initialEnd,
							sTrackingData.timeDiff,
							hourHeight,
							&sTrackingData.linkedEvent))
			{
				// The event did not move, skip the end of the function
				goto ExitWeekViewSelectEventTrack;
			}

			sTrackingData.prevPosInDay = posInDay;
			sTrackingData.dayOfWeek = sTrackingData.newDayOfWeek;
		}
		else
		{
			// We are out of possible range:
			// We've move passed the last valid day (12/31/1931)

			if ((TimeToInt (sTrackingData.start) == TimeToInt(sTrackingData.apptRec.when.startTime))
			&& (DateToInt (sTrackingData.date) == DateToInt(weekP->days[sTrackingData.dayOfWeek].date)))
			{
				goto ExitWeekViewSelectEventTrack;
			}

		 	WeekViewReselectEvent(weekP);
		}
	}

	// Compute the bounds of the event.
	WeekViewComputeEventBounds(
							weekP,
							sTrackingData.dayOfWeek,
							sTrackingData.start,
							sTrackingData.end,
							hourHeight,
							&sTrackingData.eventR);

	WeekViewComputeLinkedEventBounds(weekP, hourHeight, &sTrackingData.linkedEvent);

	// Invalidate the week display
	WeekViewDisplayInvalidate(&sTrackingData.weekAndMarksR);

	// Draw the new date and time.
	WeekViewDescriptionInvalidate();

ExitWeekViewSelectEventTrack:
	MemHandleUnlock (sWeekObjectH);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewSelectEventRelease
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	trackingDataP:	Tracking information.
 *
 * RETURNED:    	true if the appointment moved.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	11/19/02	Initial revision.
 *
 ***********************************************************************/
static Boolean WeekViewSelectEventRelease (void)
{
	WeekType*	weekP;
	Boolean		moved = false;
	Boolean		sameTime;
	Boolean		sameDate;
	int16_t		i;

	weekP = MemHandleLock (sWeekObjectH);

	// Check if the date or time of the appointment has been changed.
	sameTime = (Boolean) (	(TimeToInt(sTrackingData.start) == TimeToInt(sTrackingData.apptRec.when.startTime)) &&
		         			(TimeToInt(sTrackingData.end) == TimeToInt(sTrackingData.apptRec.when.endTime)));

	sameDate = (Boolean) (	DateToInt(sTrackingData.date) ==
							DateToInt(weekP->days[sTrackingData.savedDayOfWeek].date));

	ApptFreeRecordMemory (&sTrackingData.apptRec);

	// If the time hasn't been changed leave the description popup display
	// for a period of time (2 or 5 seconds)
	if (sameTime && sameDate)
	{
		// Generate a nilEvent within  sTrackingData.displayTicks miliseconds
		TimeComputeWaitTime = true;
		TimeDisplayTick = TimGetTicks() + sTrackingData.displayTicks;

		goto ExitWeekViewSelectEventRelease;
	}

	// Set the description area as invalidate
	WeekEraseDescription(true);

	// If the date or time was changed move the appointment and redraw the view.
	// If only the time have changed then split the event into two events.
	WeekViewComputeDateFromLinkedEvent(
			weekP,
			&sTrackingData.start,
			&sTrackingData.end,
			&sTrackingData.date,
			sTrackingData.dayOfWeek,
			&sTrackingData.linkedEvent);

	DbCursorMoveToRowID(gApptCursorID, sTrackingData.rowID);

	MoveEvent (
			gApptCursorID,
			sTrackingData.start,
			sTrackingData.end,
			sTrackingData.date,
			sTrackingData.linkedEvent.existingLinkedEvent,
			false,
			sameDate,
			&moved);

	if (moved)
	{
		for (i = 0; i < daysInWeek; ++i)
			WeekViewDayClose (&(weekP->days[i]));

		WeekViewInitWeekDays (FrmGetFormPtr(WeekView), true);
		WeekViewDisplayInvalidate(&sTrackingData.weekAndMarksR);
	}


ExitWeekViewSelectEventRelease:
	MemHandleUnlock (sWeekObjectH);
	return moved;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewCancelTracking
 *
 * DESCRIPTION: 	Cancel current ongoing event tracking
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/11/04 Initial revision.
 *
 ***********************************************************************/
static void WeekViewCancelTracking (void)
{
	if (sTrackingData.trackingType != nothingSelected
		&& sTrackingData.trackingType != dragAborted)
	{
		sTrackingData.trackingType = nothingSelected;

		WeekEraseDescription (true);
		WeekViewHideTime (true);

		// Reset  tracking information
		WeekViewResetTracking ();
	}
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewFindEventInDay
 *
 * DESCRIPTION:		Check
 *
 * PARAMETERS:	->	weekP: 	Current day.
 *				->	dayP:
 *				->	apptsP:
 *				->	x: 			ScreenX.
 *				->	y:			ScreenY.
 *				-> 	highRP:
 *				->	pickRP:
 *				->	rowIdP:
 *				->	overlapStateP;
 *
 *
 * RETURNED:    	loop index value.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/23/04	Initial revision. Split from WeekViewDayFindEvent.
 *
 ***********************************************************************/
int32_t  WeekViewFindEventInDay(
	WeekType* 					weekP,
	DayType* 					dayP,
	ApptInfoType* 				apptsP,
	Coord						x,
	Coord						y,
	RectangleType* 				highRP,
	RectangleType* 				pickRP,
	uint32_t * 					rowIdP,
	MidnightOverlapStateType* 	overlapStateP)
{
	uint16_t					startHour;
	uint16_t					endHour;
	uint16_t					hourHeight;
	ApptInfoPtr					appts = NULL;
	uint32_t					rowID = dbInvalidRowID;
	MidnightOverlapStateType 	overlapState = overlapNone;
	int32_t 					i = -1;
	Boolean						found = false;

	startHour 	= weekP->startHour;
	endHour 	= weekP->endHour;
	hourHeight 	= weekP->hourHeight;

	for (i = dayP->numAppts -1; i >= 0; i--)
	{
		//	Get the bounds of each appointment and check selected point
		//	against them.  We traverse the list from bottom to top
		//	so that we will find the event with the latest start time
		//	in the event of event overlap.
	 	WeekViewCreateEventBar (&dayP->bounds, highRP);

		WeekViewGetEventBounds (
						apptsP[i].startTime,
						apptsP[i].endTime,
						startHour, endHour,
						highRP, hourHeight);

		RctCopyRectangle(highRP, pickRP);

		pickRP->topLeft.x = dayP->bounds.topLeft.x;
		pickRP->extent.x = dayP->bounds.extent.x;
		pickRP->topLeft.y--;
		pickRP->extent.y += 2;			//	Include top & bottom of frame in pick
										//	area.

		if (RctPtInRectangle (x, y, pickRP))
		{
			rowID = apptsP[i].rowID;
			overlapState = apptsP[i].overlapState;
			break;
		}

	}

	if (rowIdP)
		*rowIdP= rowID;

	if (overlapStateP)
		*overlapStateP = overlapState;

	return i;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDayFindEvent
 *
 * DESCRIPTION:		This routine is called when a pen winDown event
 *					occurs in a week object.  This routine will check
 *					to see if the pen-winDown occurred within the passed
 *					day and track the pen until it is released.
 *
 * PARAMETERS:	->	dayOfWeek: 	current day.
 *				->	x: 	screenX
 *				->	y:	screenY
 *
 *
 * RETURNED:    	true if this day contained the passed
 *					event/point and handled it.  False if
 *					the event/point was outside this day.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/7/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL	07/22/03	Use MemHandleUnlock rather than MemPtrUnlock.
 *	PPL 04/23/04	Change Event parameter to x, y coordinates.
 *
 ***********************************************************************/
static Boolean WeekViewDayFindEvent (
	WeekType*					weekP,
	int16_t 					dayOfWeek,
	Coord						x,
	Coord						y)
{
	DayType *					day;
	uint16_t					startHour;
	uint16_t					endHour;
	uint16_t					hourHeight;
	ApptInfoPtr					appts = NULL;
	uint32_t					rowID = dbInvalidRowID;
	MidnightOverlapStateType 	overlapState = overlapNone;
	uint16_t 					attr;
	Boolean						handled = false;
	Boolean						verifyPW;
	int32_t						apptIndex = -1;

	//weekP = MemHandleLock (sWeekObjectH);

	// Perform some new initialization here
	day = &weekP->days[dayOfWeek];
	startHour = weekP->startHour;
	endHour = weekP->endHour;
	hourHeight = weekP->hourHeight;

	//	If this is the wrong day, give winUp
	if (! RctPtInRectangle (x, y, &day->bounds))
		goto ExitWeekViewDayHandleSelection;

	//	If this day is out of the day range, and therefore inelligible for
	//	events, skip it:
	if (day->date.day == outOfRangeDay)
	{
		handled = true;
		goto ExitWeekViewDayHandleSelection;
	}

	//	Determine the part of the day that is selected:
	//	1.	Day/Date - go to day
	//	2.	single appt. - go to day, edit existing event
	//	3.	empty appt. slot - go to day edit new event

	//	NOTE: we are sometimes using different rectangles for picking &
	//	drawing highlights.  This is so that we can give the user as wide a
	//	margin for picking error as possible.

	Date = day->date;
	if (y <= (day->bounds.topLeft.y
							+ weekViewDayDisplayHeight
							+ weekViewDateDisplayHeight))
	{
		//	Day/Date is selected - highlight day/date
		sTrackingData.pickR = day->bounds;
		sTrackingData.pickR.extent.y = weekViewDayDisplayHeight + weekViewDateDisplayHeight;
		sTrackingData.highR = sTrackingData.pickR;
		sTrackingData.highR.topLeft.x += weekViewDayHMargin-1;
		sTrackingData.highR.extent.x -= 2*(weekViewDayHMargin-1);
		sTrackingData.trackingType = dateSlotSelected;

		sTrackingData.wasSelected = true;
		sTrackingData.isSelected = true;

		//	no saveR needed for this case
	}
	else
	{
		//	Check through all appts
		if (day->numAppts > 0)
			appts = MemHandleLock (day->apptsH);

		apptIndex = WeekViewFindEventInDay(
						weekP, day, appts, x, y,
						&sTrackingData.highR,
						&sTrackingData.pickR,
						&rowID,
						&overlapState);

		if (apptIndex >= 0)
		{
			sTrackingData.isSelected = true;
			sTrackingData.trackingType = trackingApptSlot;
		}
		else
		{

			//	New appt has been selected.  Convert y-coord of pt to
			//	time of new appt & visible region of new appt.
			sTrackingData.isSelected = true;

			sTrackingData.newStart.hours = (y -
									day->bounds.topLeft.y -
									weekViewDayDisplayHeight -
									weekViewDateDisplayHeight) / hourHeight;

			sTrackingData.newStart.hours += startHour;
			sTrackingData.newStart.minutes = 0;

			sTrackingData.newEnd = sTrackingData.newStart;
			sTrackingData.newEnd.hours++;

			// we will draw the frame of the empty slot
			sTrackingData.drawFrame = true;

			// There are occasions, depending on the start/end time settings in
			// preferences, as well as the times of existing events that there will
			// be an "empty" row  at the bottom of the week view grid.  Be sure
			// that the user is not allowed to create an appointment in this area.
			// Times for new events should always have a start hour between 0 and 23.
			if (sTrackingData.newStart.hours < hoursPerDay)
			{
			 	WeekViewCreateEventBar (&day->bounds, &sTrackingData.highR);

				WeekViewGetEventBounds (
							sTrackingData.newStart,
							sTrackingData.newEnd,
							startHour,
							endHour,
							&sTrackingData.highR,
							hourHeight);

				sTrackingData.pickR = sTrackingData.highR;
				sTrackingData.pickR.topLeft.x = day->bounds.topLeft.x;
				sTrackingData.pickR.extent.x = day->bounds.extent.x;
				sTrackingData.pickR.topLeft.y--;
				sTrackingData.pickR.extent.y += 2;		//	Include top & bottom of frame in pick area
				sTrackingData.trackingType = emptySlotSelected;
			}
			else
			{
				// just clean up & return true now as the user has tapped in
				// an "invalid' time & there is nothing to track
				if (day->numAppts > 0)
					MemHandleUnlock(day->apptsH);

				sTrackingData.trackingType = nothingSelected;

				handled = true;
				goto ExitWeekViewDayHandleSelection;
			}
		}

		if (day->numAppts > 0)
			MemHandleUnlock(day->apptsH);

	}

	// Draw the date, time and description of the apointment.
	if (sTrackingData.trackingType == trackingApptSlot)
	{
		// Mask if appropriate
		DbGetRowAttr(ApptDB, rowID, &attr);

	   	if (((attr & dmRecAttrSecret) && CurrentRecordVisualStatus == maskPrivateRecords))
		{
			verifyPW = DateUtilsSecVerifyPW(showPrivateRecords);

			// re-opens cursor and set up week view again (in case something changed
			// due to security.
			WeekViewInitWeekDays (FrmGetFormPtr(WeekView), true);

			if (verifyPW)
			{
				// We only want to unmask this one record, so restore the preference.
				// to its previous state (not maskPrivateRecords as we can have masked records
				// whithout having set the security level)
				PrefSetPreference (prefShowPrivateRecords, PrivateRecordVisualStatus);


				// Only change the visual status of this record, leaving all others masked.
				CurrentRecordVisualStatus = showPrivateRecords;

				WeekViewSelectEventInitialize (
							dayOfWeek,
							x,
							y,
							rowID,
							overlapState,
							weekDescUnmaskedDisplayTicks);
			}

			WeekViewCancelTracking();

			handled = true;
			goto ExitWeekViewDayHandleSelection;
		}
		else
		{
			WeekViewSelectEventInitialize (
							dayOfWeek,
							x,
							y,
							rowID,
							overlapState,
							weekDescDisplayTicks);

			handled = true;
			goto ExitWeekViewDayHandleSelection;
		}
	}

	//	Handle all of the fancy dragging & highlighting:
	//	If invertHighlight, then highlight the selection by inverting the
	//	rectangle; else, highlight by drawing a frame.

	sTrackingData.saveR = sTrackingData.highR;

	if (sTrackingData.trackingType == emptySlotSelected)
	{
		sTrackingData.eventFrame.word = 0;
		sTrackingData.eventFrame.bits.width = weekViewEventFrameWidth;

		//	Expand the drawR to include the frame:
		sTrackingData.saveR.topLeft.x -= weekViewEventFrameWidth;
		sTrackingData.saveR.topLeft.y -= weekViewEventFrameWidth;
		sTrackingData.saveR.extent.x += 2*weekViewEventFrameWidth;
		sTrackingData.saveR.extent.y += 2*weekViewEventFrameWidth;
	}

	// Invalidate the region so it will be redrawn
	WeekViewDisplayInvalidate(&sTrackingData.saveR);

	handled = true;

ExitWeekViewDayHandleSelection:
	//MemHandleUnlock (sWeekObjectH);
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewStartEventTracking
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	event:	a pointer to an EventType structure
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/9/95		Initial revision.
 *	PPL	07/22/03	Use MemHandleUnlock rather than MemPtrUnlock.
 *
 ***********************************************************************/
static Boolean WeekViewStartEventTracking (EventType* event)
{
	WeekType*					weekP;
	Coord						x;
	Coord						y;
	int16_t						dayIndex;
	Boolean						handled = false;

	// Event tracking data is resetted at View Initialization
	// and when tracking stops WeekViewStopEventTracking and WeekViewInitWeek
	weekP = MemHandleLock (sWeekObjectH);

	x = event->screenX;
	y = event->screenY;

	if (RctPtInRectangle (x, y, &weekP->bounds))
	{
		//	Let each day have a crack at the event until one handles it
		for (dayIndex = 0; dayIndex < daysInWeek; ++dayIndex)
		{
			if (WeekViewDayFindEvent (weekP, dayIndex, x, y))
				break;
		}

		handled =  true;
	}

	MemHandleUnlock (sWeekObjectH);

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewUpdateEventTracking
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	event: 			a pointer to an EventType structure.
 *				->	trackingDataP: 	tracking information.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/9/95		Initial revision.
 *	PPL	07/22/03	Palm OS 6.1
 *
 ***********************************************************************/
static Boolean WeekViewUpdateEventTracking (EventType* event)
{
	Boolean 	handled = false;

	switch (sTrackingData.trackingType)
	{
		case nothingSelected:
			handled = false;
			break;

		case emptySlotSelected:
		case dateSlotSelected:
			sTrackingData.isSelected = RctPtInRectangle (event->screenX, event->screenY, &sTrackingData.pickR);

			if (sTrackingData.isSelected != sTrackingData.wasSelected)
			{
				// always need to invalidate the rectangle
				WeekViewDisplayInvalidate(&sTrackingData.saveR);
				sTrackingData.wasSelected = sTrackingData.isSelected;
			}

			handled = true;
			break;

		case trackingApptSlot:
			WeekViewSelectEventTrack(event);
			handled = true;
			break;

		case dragAborted:
			handled = true;
			break;

		default:
			ErrNonFatalDisplay("WeekViewUpdateTracking: invalid state");
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewStopTracking
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	event:  		pointer to an EventType structure
 *				->	trackingDataP:	tracking information
 *
 * RETURNED:    	nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR		8/9/95	Initial revision.
 *	PPL	07/22/03	Palm OS 6.1
 *
 ***********************************************************************/
static Boolean WeekViewStopEventTracking (EventType* event)
{
	Boolean 		handled = false;
	TrackingType 	trackingState = sTrackingData.trackingType;

	// Update tracking value before handling below - This enables the
	// pop-up dialog to be refreshed
	sTrackingData.trackingType = nothingSelected;

	switch (trackingState)
	{
		case emptySlotSelected:
			if (sTrackingData.isSelected)
			{
				// Jump to the day view with a new appointment edited
				WeekViewNewAppointment (&sTrackingData.newStart, &sTrackingData.newEnd);
				FrmGotoForm(gApplicationDbP, DayView);
			}
			else
				// To actually remove the empty slot frame
				WeekViewDisplayInvalidate(&sTrackingData.saveR);

			handled = true;
			break;

		case dateSlotSelected:
			if (sTrackingData.isSelected)
				FrmGotoForm(gApplicationDbP, DayView);

			handled = true;
			break;

		case trackingApptSlot:
			// Redraw the pop-up
			// If the appointement didn't move, update the display
			if (! WeekViewSelectEventRelease())
			{
				// Invalidate the week display
				WeekViewDisplayInvalidate(&sTrackingData.weekAndMarksR);
			}
			handled = true;
			break;

		case nothingSelected:
		case dragAborted:
			break;

		default:
			ErrNonFatalDisplay("WeekViewStopTracking: invalid state");
	}

	// Reset  tracking information
	WeekViewResetTracking ();

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewGoToWeek
 *
 * DESCRIPTION:		This routine moves the Week View to the week
 *					including the current setting of Date.  It is assumed
 *					that the week view is already winUp; it must be erased,
 *					reinitialized, and redrawn.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/21/95		Initial revision.
 *
 ***********************************************************************/
static void WeekViewGoToWeek (void)
{
	FormPtr		formP;

	PIMAppProfilingBegin("Define Week View Layout");

	formP = FrmGetFormPtr(WeekView);

	// Build the new week
	WeekViewInitWeekUI (formP);
	WeekViewSetupWeek(formP);
	WeekViewInitWeekDays (formP, true);

	// Invalidate the window to trigger the drawing
	WeekViewDisplayInvalidate(NULL);

	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewGotoDate
 *
 * DESCRIPTION:		This routine dispay the date picker so that the
 *					user can select a week to navigate to.  If the date
 *					picker is confirmed, the week containing the
 *					day selected is displayed.
 *
 *					This routine is called when a "go to" button is pressed.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR		9/19/95	Initial revision.
 *	KCR		9/19/95	Navigates to a week instead of a day.
 *	KCR		10/5/95	Preserve font around calls to DatePicker.
 *
 ***********************************************************************/
static void WeekViewGotoDate (void)
{
	char 		* title;
	FontID		origFont;
	MemHandle 	titleH;
	int16_t 	month;
	int16_t 	day;
	int16_t 	year;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource(gApplicationDbP, strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	// Display the date picker.
	origFont = FntGetFont ();
	if (SelectDay (selectDayByWeek, &month, &day, &year, title))
	{
		FntSetFont (origFont);
		Date.day = day;
		Date.month = month;
		Date.year = year - firstYear;

		WeekViewGoToWeek ();
	}
	else
	{
		FntSetFont (origFont);
	}

	MemHandleUnlock (titleH);
	DmReleaseResource (titleH);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewGoToAppointment
 *
 * DESCRIPTION: 	Switch to the To Do application and display the given
 *					record On the day view.
 *
 *					If there are any problems retrieving the record,
 *					launch the app without it.
 *
 *					When record is private ans security is set at some levels
 *					(password assigned) calls the security password tyo improve
 *					responsiveness otherwise user see a white screen for a few
 *					seconds while day view is parsing the db, setting up the view
 *					and bringing the seurity password.
 *
 * PARAMETERS:  ->	inDB:		a pointer to an EventType structure
 *				->	rowID:
 *				->	overlapState:
 *
 * RETURNED:    	true if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void WeekViewGoToAppointment (
	uint32_t 					rowID,
	MidnightOverlapStateType 	overlapState)
{
	uint16_t	rowAttributes;
	status_t 	err;

	// Set the record to be edited in the day view
	if (rowID != dbInvalidRowID)
	{
		err = DbCursorMove(gApptCursorID, rowID, dbFetchRowID);

		if (err >= errNone)
		{
			// Set up Day View item selection
			gItemSelected = true;

			// Set the cursor at the end of the line in Day View
			DayEditPosition = mostRightFieldPosition;

			// Mask if appropriate
			err = DbGetRowAttr(ApptDB, rowID, &rowAttributes);

	   		if ( (rowAttributes & dmRecAttrSecret)
	   			&& (CurrentRecordVisualStatus == maskPrivateRecords))
			{
				if (DateUtilsSecVerifyPW(showPrivateRecords))
					CurrentRecordVisualStatus = showPrivateRecords;
				else
				{
					// exit whithout going to day view
					// but we update the agenda view
					// as private events might have disappear (lost password)
					WeekViewGoToWeek ();

					// Reset Day View item selection
					gItemSelected = false;
					return;
				}
			}
		}
	}

	// Switch to the day view
	if (overlapState == overlapScndPart)
		DateAdjust (&Date, -1);
	else if (overlapState == overlapNone)
	{
		Date.day = sTrackingData.date.day;
		Date.month = sTrackingData.date.month;
		Date.year = sTrackingData.date.year;
	}

	FrmGotoForm(gApplicationDbP, DayView);
}



/***********************************************************************
 *
 * FUNCTION:    	WeekViewSelectCategory
 *
 * DESCRIPTION: 	Handles Category Selection UI.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	07/29/02	Initial revision, added category support.
 *
 ***********************************************************************/
static void WeekViewSelectCategory(void)
{
	FormType* 		frmP;
	Boolean 		categoryEdited;
	CategoryID * 	newCatSelectionP = NULL;
	uint32_t		newCatSelectionNum = 0;

	frmP = FrmGetFormPtr(WeekView);

	// Close all opened cursors (main cusor)
	// we have to close cursors as Datebook is using cahed cursors
	// unfortunaltely when a record is in such loaded in such a cache
	// when CatMgrSelectFilter is called, it cannot update records.
	ApptCloseCursor(&gApptCursorID);

	categoryEdited = CatMgrSelectFilter(ApptDB, frmP, WeekCategoryTrigger,
		DateBkCategoriesName, WeekCategoryList, DateBkCurrentCategoriesP,
		DateBkCurrentCategoriesCount, &newCatSelectionP, &newCatSelectionNum,
		true, NULL);

	if (categoryEdited)
	{
		// Update current categories
		ChangeCurrentCategories(newCatSelectionNum, newCatSelectionP);

		// Free list allocated by CatMgrSelectFilter
		CatMgrFreeSelectedCategories(ApptDB, &newCatSelectionP);

		// Display the new category.
		WeekViewGoToWeek ();
	}
	else
		// Rebuild the layout and update display
		WeekViewInitWeekDays (frmP, true);
}

/***********************************************************************
 *
 * FUNCTION:    	WeekViewCanScroll
 *
 * DESCRIPTION:		Tells whether or not week view can be scrolled
 *						in given direction
 *
 * PARAMETERS:		direction:	winUp or down
 *
 * RETURNED:    	Boolean.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY
 *	PPL	06/18/040	Initial revision.
 *
 ***********************************************************************/
static Boolean WeekViewCanScroll (WeekType* weekP, WinDirectionType direction)
{
	Boolean canScroll = false;


	switch(direction)
	{
		case winUp:
			canScroll =(weekP->firstEventHour < weekP->startHour);
			break;

		case winDown:
			canScroll =(weekP->endHour < weekP->lastEventHour);
			break;
	}

	return canScroll;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewScroll
 *
 * DESCRIPTION:		This routine scrolls the Week View in the direction
 *					specified.  It is assumed that there is room to scroll
 *					in the specified direction.  If there is not, the
 *					scroll operation will just redraw the screen in place.
 *
 *					The screen is scrolled on week->scrollInteval  if
 *					byPage is FALSE, and by week->hoursDisplayed if
 *					byPage is TRUE, in the specified direction.
 *
 *					Depending on Active input graffiti area is
 *					shown :
 *						scrollInterval =  weekViewActiveInputShownScrollInterval
 *						hoursDisplayed = weekViewNumActiveInputShownDispHours
 *					hidden:
 *						scrollInterval = weekViewActiveInpuitHiddenScrollInterval
 *						hoursDisplayed = weekViewNumActiveInputHiddenDispHours
 *
 * PARAMETERS:		direction:	winUp or dowm
 *					byPage:		TRUE if scrolling by page, FALSE
 *								if scrolling by weekViewScrollInterval
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	8/7/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL	07/22/03	Replace MemPtrUnlock by MemHandleUnlock.
 *
 ***********************************************************************/
static void WeekViewScroll (WinDirectionType direction, Boolean byPage)
{
	uint16_t		interval;
	int16_t			lineYPos;
	WeekType*		weekP;
	uint16_t		scrollDist;
	uint16_t		dispStartHour;

	weekP = MemHandleLock (sWeekObjectH);

	if (byPage)
		interval = weekP->hoursDisplayed;
	else
		interval = weekP->scrollInterval;

	//	Adjust week's displayed hour range in the proper direction
	switch (direction)
	{
		case winUp:
			if (byPage)
				scrollDist = weekP->hoursDisplayed;
			else
				scrollDist = interval;

			if ((weekP->startHour - weekP->firstEventHour) < scrollDist)
				scrollDist = weekP->startHour - weekP->firstEventHour;

			// only scroll by an even number of hours (otherwise the event fill
			// pattern gets out of sync)
			if (scrollDist % 2 != 0)
			{
				scrollDist++;

				// Don't scroll up (decrement) more than startHour. This field is an
				// unsigned int so wrapping around to the 655xx values is not desired.
				// if the scroll distance is odd, just force a full update of the grid
				// area to keep the patterns in sync.
				if (scrollDist > weekP->startHour)
					scrollDist = weekP->startHour;
			}

			weekP->startHour -= scrollDist;
			weekP->endHour -= scrollDist;
			break;

		case winDown:
			if (byPage)
				scrollDist = weekP->hoursDisplayed;
			else
				scrollDist = interval;


			if ((weekP->lastEventHour - weekP->endHour) < scrollDist)
				scrollDist = weekP->lastEventHour - weekP->endHour;

			// only scroll by an even number of hours (otherwise the event fill
			// pattern gets out of sync)
			if (scrollDist % 2 != 0)
			{
				scrollDist++;
			}

			weekP->startHour += scrollDist;
			weekP->endHour += scrollDist;
			break;
	}

	//	Redraw the hour markers:
	//	Determine where start of backround is:
 	lineYPos = weekP->bounds.topLeft.y +
 						 weekViewDayDisplayHeight +
 						 weekViewDateDisplayHeight;
 	dispStartHour = weekP->startHour;

 	while ((dispStartHour % weekViewHourDisplayInterval) != 0)
 	{
 		//	Display starts on an odd hour
 		lineYPos += weekP->hourHeight;
 		dispStartHour++;
 	}

	MemHandleUnlock(sWeekObjectH);

	WeekViewUpdateScrollers (FrmGetActiveForm());
	WeekViewDisplayInvalidate(NULL);
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDoCommand
 *
 * DESCRIPTION: 	This routine performs the menu command specified.
 *
 * PARAMETERS:  ->	command:	menu item id.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		8/10/95	Initial revision.
 *
 ***********************************************************************/
static Boolean WeekViewDoCommand (uint16_t command)
{
	Boolean	handled = false;

	switch (command)
	{
		case WeekGetInfoCmd:
			MenuEraseStatus(0);
			AbtShowAbout (sysFileCDatebook);
			handled = true;
			break;

		case WeekPreferencesCmd:
			MenuEraseStatus(0);
			FrmPopupForm(gApplicationDbP, PreferencesDialog);
			handled = true;
			break;

		case WeekSecurityCmd:
			MenuEraseStatus(0);
			DoSecurity();
			handled = true;
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	WeekViewFormResize
 *
 * DESCRIPTION:		This routine initializes the structures used for the
 *					WeekView.  The start date will be set and the start
 *					and end of the display range will be set.
 *
 * PARAMETERS:  ->	eventP:	pointer on event structure.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	01/31/02	Initial revision.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *	PPL 05/04/04	Add Landcsape Mode.
 *
 ***********************************************************************/
void WeekViewFormResize(EventType* eventP)
{
	RectangleType 	objBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	Coord			offsetX;
	Coord			offsetY;
	TableType*		tableP;
	Coord			width;
	Coord			height;
	FormType* 		frmP;

	width = eventP->data.winResized.newBounds.extent.x;
	height = eventP->data.winResized.newBounds.extent.y;
	frmP = FrmGetFormPtr(WeekView);

	offsetY = height - gCurrentWinBounds.extent.y;
	offsetX = width  - gCurrentWinBounds.extent.x;

	if (!offsetX && ! offsetY)
	{
		// if the window has the height of the screen
		// then the windows is already at the right size
		goto justset;
	}

	if (offsetX)
	{

		// WeekCategoryTrigger, moves only on x, not on y
		objIndex =  FrmGetObjectIndex (frmP, WeekCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// WeekCategoryList, moves only on x, not on y
		objIndex =  FrmGetObjectIndex (frmP, WeekCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}


	// WeekDayDisplay (Gadget) resized both on x and y
	objIndex =  FrmGetObjectIndex (frmP, WeekDayDisplay);
	tableP = (TableType*) FrmGetObjectPtr( frmP, objIndex);

	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x += offsetX;
	objBounds.extent.y += offsetY;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	// BackgroundGadget
	objIndex =  FrmGetObjectIndex (frmP, WeekBackgroundGadget);
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x +=   offsetX;
	objBounds.extent.y +=   offsetY;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	if (offsetY)
	{
		// WeekDayViewButton, moves only on y
		objIndex =  FrmGetObjectIndex (frmP, WeekDayViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// WeekWeekViewButton, moves only on y
		objIndex =  FrmGetObjectIndex (frmP, WeekWeekViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// WeekMonthLunarViewButton for Lunar Calendar
		if (DateSupportsLunarCalendar())
		{
			objIndex =	FrmGetObjectIndex (frmP, WeekMonthLunarViewButton);
			FrmGetObjectPosition (frmP, objIndex, &x, &y);
			y +=   offsetY;
			FrmSetObjectPosition (frmP, objIndex, x, y);
		}


		// WeekMonthViewButton, moves only on y
		objIndex =  FrmGetObjectIndex (frmP, WeekMonthViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// WeekAgendaViewButton, moves only on y
		objIndex =  FrmGetObjectIndex (frmP, WeekAgendaViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// WeekGoToButton, moves only on y
		objIndex =  FrmGetObjectIndex (frmP, WeekGoToButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	// WeekUpButton, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, WeekUpButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);


	// WeekDownButton, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, WeekDownButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);


	// WeekPrevButton, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, WeekPrevButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);


	// WeekNextButton, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, WeekNextButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);


	// WeekNumberLabel, moves on both x and y
	objIndex =  FrmGetObjectIndex (frmP, WeekNumberLabel);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	x += offsetX;
	y += offsetY;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// keep the window bounding rect.
	gCurrentWinBounds = eventP->data.winResized.newBounds;

justset:
	// finish initializations due to gadget week view resizing
	WeekViewSetupWeek(frmP);
	WeekViewInitWeekDays(frmP, false);

	// focus resizing
	//WeekViewFocusResize(offsetX, offsetY);
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewOpen
 *
 * DESCRIPTION:		Handle Week Opening.
 *
 * PARAMETERS:	->	None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/31/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewOpen(void)
{
	uint16_t	gadgetIndex;

	sFormP = FrmGetFormPtr(WeekView);

	// Since we have a background image, we need to have transparent widgets
	FrmSetTransparentObjects(sFormP, true);

	DateBackgroundGetHighlightColor(&sBackgroundHighlight, Date.month);

	memset(&sGadgetCallbackData, 0, sizeof(sGadgetCallbackData));
	sGadgetCallbackData.color = sBackgroundHighlight;

	gadgetIndex = FrmGetObjectIndex(sFormP, WeekBackgroundGadget);

	// Save some info we will need in our callback
	FrmSetGadgetData(sFormP, gadgetIndex, &sGadgetCallbackData);

	// Set up the callback for the title bar graphic
	FrmSetGadgetHandler(sFormP, gadgetIndex, DateBackgroundGadgetHandler);

	sOrigFont = FntSetFont (stdFont);

	WeekViewResetTracking();

	WeekViewInitWeek (sFormP);

	WeekViewSetupWeek(sFormP);

	WeekViewInitWeekDays (sFormP, true);

	EventInCurrentView = false;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewIsValidFreeSlot
 *
 * DESCRIPTION:		Check focus zone rectangle validity
 *
 * PARAMETERS:	->	weekP:			Week view date (locked handle.)
 *				->	dayOfWeek:		Day of Week
 *				->	focusZone:		size of focused zone.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/31/03	Initial revision.
 *
 ***********************************************************************/
Boolean WeekViewIsValidFreeSlot(WeekType* weekP, uint16_t dayOfWeek, RectangleType* focusZoneP)
{
	DayType *					day;
	RectangleType				dayBounds;
	ApptInfoPtr 				appts = NULL;
	Boolean 					found = false;

	// Perform some new initialization here
	day = &weekP->days[dayOfWeek];

	appts = MemHandleLock (day->apptsH);

	GetDayOfWeekBounds(weekP, dayOfWeek, &dayBounds);

	found = RctPtInRectangle(focusZoneP->topLeft.x +1, focusZoneP->topLeft.y+1, &dayBounds);

	MemHandleUnlock (day->apptsH);

	return found;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewOffsetDays
 *
 * DESCRIPTION:		offset wwek ofthe nupmber of days.
 *					 <0 move to previous date
 *					>0 move to next date.
 *
 * PARAMETERS:	->	None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/31/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewOffsetDays(int32_t dateAdjust)
{
	DateAdjust (&Date, dateAdjust);
	WeekViewGoToWeek ();
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewClose
 *
 * DESCRIPTION:		Handle Week closing.
 *
 * PARAMETERS:	->	None..
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/31/03	Initial revision.
 *
 ***********************************************************************/
static void WeekViewClose(void)
{
	WeekViewFree ();
	FntSetFont (sOrigFont);
	sFormP = NULL;
}


static void WeekViewFocusReset(WeekType* weekP);


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusSetRing
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusSetRing(WeekType*	weekP)
{
	RectangleType 	focusZone;

	focusZone = weekP->bounds;

	

	// fine tune the size of the rectangle
	++(focusZone.topLeft.x);
	++(focusZone.topLeft.y);

	FrmSetFocusHighlight(gWinHdl, &focusZone, 0);
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusClearRing
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
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusClearRing(void)
{
	FrmClearFocusHighlight();
}


/***********************************************************************
 *
 * FUNCTION:		WeekFocusZoneToTargetPoint
 *
 * DESCRIPTION: 	return the target point for the given focused zone rectangle)
 *
 * PARAMETERS:	->	focusZone:	the focused rectangle.
 *				<- x:			x Coordinate.
 *				<- y:			y Coordinate.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			x and y have to be valid.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekFocusZoneToTargetPoint(RectangleType* focusZone, Coord* x, Coord* y)
{
	*x = focusZone->topLeft.x + (focusZone->extent.x >> 1);
	*y= focusZone->topLeft.y + (focusZone->extent.y >> 1);
}


/***********************************************************************
 *
 * FUNCTION:		WeekFocusCopyGetFocusTrigger
 *
 * DESCRIPTION: 	Sets focusTrigger rectangle.
 *					Focus slot rectangle is used to find
 *					slot event.
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static Boolean WeekFocusSetUpFocusTrigger(WeekType* weekP, uint16_t action, RectangleType* focusZoneP)
{
	RectangleType 	focusTrigger;
	RectangleType	slotsBounds;
	Boolean			canMove = false;

	focusTrigger = weekP->focusTrigger;

	focusTrigger.topLeft.x = weekP->focusedZone.topLeft.x;
	focusTrigger.extent.x = weekP->focusedZone.extent.x;
	focusTrigger.extent.y = weekP->hourHeight;

	switch (action)
	{
		case kFocusTriggerDown:
			focusTrigger.topLeft.y += weekP->hourHeight;
			break;

		case kFocusTriggerUp:
			focusTrigger.topLeft.y -= weekP->hourHeight;
			break;

		case kFocusTriggerReset:
			focusTrigger.topLeft.y =  weekP->focusedZone.topLeft.y;
			break;
	}

	// check if focus trigger reactangle remains whithin days rectangle
	GetDaysBounds(weekP, &slotsBounds);

	if ((focusTrigger.topLeft.x
			>= slotsBounds.topLeft.x)
	&& (focusTrigger.topLeft.y
			>= slotsBounds.topLeft.y)
	&& ((focusTrigger.topLeft.x + focusTrigger.extent.x)
			<= (slotsBounds.topLeft.x + slotsBounds.extent.x )
	&& ((focusTrigger.topLeft.y) )
			<= (slotsBounds.topLeft.y + slotsBounds.extent.y)))
	{
		weekP->focusTrigger = focusTrigger;
		canMove = true;
	}

	if (focusZoneP)
		RctCopyRectangle(&weekP->focusTrigger, focusZoneP);

	return canMove;

}


/***********************************************************************
 *
 * FUNCTION:		WeekFocusActivate
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusActivate(WeekType*	weekP)
{
	EventType		newEvent;
	RectangleType 	focusZone;

	RctCopyRectangle(&weekP->focusedZone, &focusZone);

	// send a penDown Event
	memset(&newEvent, 0x00, sizeof(newEvent));

	WeekFocusZoneToTargetPoint(&focusZone, &newEvent.screenX, &newEvent.screenY);

	newEvent.eType = penDownEvent;
	EvtAddEventToQueue (&newEvent);

	newEvent.eType = penUpEvent;
	EvtAddEventToQueue (&newEvent);
}



/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusPopsEventSlotDescription
 *
 * DESCRIPTION: 	Pops out the event description.
 *					Sets track static global data.
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *
 ***********************************************************************/
void WeekViewFocusPopsEventSlotDescription(WeekType* weekP)
{
	ApptInfoType*	apptsP;
	int32_t			index;
	uint16_t		dayOfWeek;


	index 					= weekP->focusedApptIndex;

	sTrackingData.savedDayOfWeek
			= sTrackingData.dayOfWeek
			= dayOfWeek
			= weekP->focusedDay;

	sTrackingData.rowID 	= weekP->focusedRowId;

	weekP->popupDescDisplayed = true;

	apptsP = (ApptInfoType*) MemHandleLock (weekP->days[dayOfWeek].apptsH);

	sTrackingData.isSelected = true;
	sTrackingData.trackingType = trackingApptSlot;

	sTrackingData.start = apptsP[index].startTime;
	sTrackingData.end = apptsP[index].endTime;

	sTrackingData.date = weekP->days[dayOfWeek].date;

	MemHandleUnlock (weekP->days[dayOfWeek].apptsH);

	WeekViewReselectEvent(weekP);

	WeekViewShowTime();

	WeekViewDescriptionInvalidate ();
}

/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusEventSlotDescriptionIsTimed
 *
 * DESCRIPTION: 	the description window is shown.
 *					we want it to vanish by time.
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *
 ***********************************************************************/
void WeekViewFocusEventSlotDescriptionIsTimed(WeekType* weekP)
{
	if (weekP->focusState == kWeekFocusEventSlotState
		&& sTrackingData.trackingType == trackingApptSlot)
	{
		sTrackingData.trackingType = nothingSelected;
		TimeComputeWaitTime = true;
	}
}






/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusReset
 *
 * DESCRIPTION: 	Hide event slot description.
 *					Resets track static global data.
 *
 * PARAMETERS:		None
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocuHideEventSlotDescription(void)
{
	WeekViewCancelTracking ();
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusFindEventSlot
 *
 * DESCRIPTION: 	Found an existing event given the day and the x, y
 *					center of focus target point.
 *
 * PARAMETERS:	->	weekP:			Week view date (locked handle.)
 *				->	dayOfWeek:		Day of Week.
 *				->	x:				x of focus target point.
 *				->	y:				y of focus target point.
 *				<-	slotBoundsP: 	bounds of event.
 *				<-	slotRowIdP:		row Id of event found.
 *				<-	slotOverlapsP:	event is overlapping two days or not.
 *
 * RETURNED:		index of appointment id day data.
 *
 * NOTE:
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/

static int32_t WeekViewFocusFindEventSlot(
	WeekType*					weekP,
	int16_t 					dayOfWeek,
	Coord						x,
	Coord						y,
	RectangleType*				slotBoundsP,
	uint32_t*					slotRowIdP,
	Boolean*					slotOverlapsP)
{
	DayType *					day;
	ApptInfoPtr					appts = NULL;
	RectangleType				highR = {0,0,0,0};
	RectangleType				pickR = {0,0,0,0};
	int32_t						index = -1;
	uint32_t					rowId = dbInvalidRowID;
	Boolean						slotOverlaps = false;

	// Perform some new initialization here
	day = &weekP->days[dayOfWeek];

	//	Check through all appts
	if (day->numAppts > 0)
	{
		appts = MemHandleLock (day->apptsH);

		index = WeekViewFindEventInDay(
					weekP, day, appts, x, y,
					&highR,
					&pickR,
					&rowId,
					NULL);

		if (index >= 0)
		{
			if (slotBoundsP)
				RctCopyRectangle(&pickR, slotBoundsP);

			if (slotRowIdP);
				*slotRowIdP = rowId;

			if (slotOverlapsP);
				*slotOverlapsP = slotOverlaps;
		}

		MemHandleUnlock (day->apptsH);
	}

	return index;
}

/***********************************************************************
 *
 * FUNCTION:		WeekViewFindFocusedEventSlot
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:			Week view date (locked handle.)
 *				->	dayNum:			Day of Week
 *				->	focusZoneP:		Slot of event given for given day of week
 *				->	slotRowIdP:		Bounds for focus zone.
 *				->	slotOverlapsP:	event overlaps on two days or not.
 *
 * RETURNED:		return slot index found (>=0)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static int32_t  WeekViewFindFocusedEventSlot(WeekType* weekP, uint16_t dayNum, RectangleType * focusZoneP, uint32_t* slotRowIdP, Boolean* slotOverlapsP)
{
	Coord		x;
	Coord		y;

	WeekFocusZoneToTargetPoint(focusZoneP, &x, &y);

	return WeekViewFocusFindEventSlot (weekP, dayNum, x, y, focusZoneP, slotRowIdP, slotOverlapsP);
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusEventSlot
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
 *				-> 	redraw:		do we update drawing
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void  WeekViewFocusEventSlot(WeekType* weekP, Boolean redraw)
{
	FrmSetFocusHighlight(gWinHdl, &weekP->focusedZone, 0);

	if (redraw)
	{
		// redraw
		WeekViewDisplayInvalidate (&weekP->focusedZone);
	}
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusDOWSlot
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
 *				-> 	redraw:		do we update drawing
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static void  WeekViewFocusDOWSlot(WeekType* weekP, Boolean redraw)
{
	FrmSetFocusHighlight(gWinHdl, &weekP->focusedZone, 0);

	if (redraw)
	{
		// redraw
		WeekViewDisplayInvalidate (&weekP->focusedZone);
	}
}



/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusFindSlot
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:			Week view date (locked handle.)
 *				->  focusTriggerP:  Trigger rectangle.

 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/18/04	Initial revision.
 *	PPL 06/18/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean WeekViewFocusFindSlot(WeekType* weekP , RectangleType * focusTriggerP)
{
	uint32_t 		focusedRowId;
	int32_t			apptIndex = -1;
	Boolean 		slotOverlaps = false;
	Boolean 		handled = false;

	focusedRowId = weekP->focusedRowId;


	apptIndex = WeekViewFindFocusedEventSlot(
										weekP,
										weekP->focusedDay,
										focusTriggerP,
										&focusedRowId,
										&slotOverlaps);


	// first check if the focus ring overlap an existing
	if (apptIndex >= 0)
	{
		RctCopyRectangle(focusTriggerP, &weekP->focusedZone);

		weekP->focusedOverlaps= slotOverlaps;
		weekP->focusedRowId = focusedRowId;
		weekP->focusedApptIndex = apptIndex;

		WeekViewFocusEventSlot(weekP, true);

		// pops out the event description
		WeekViewFocusPopsEventSlotDescription(weekP);
		handled = true;
	}
	// second check if we are always in the daysbounds
	else if (WeekViewIsValidFreeSlot(weekP, weekP->focusedDay, focusTriggerP))
	{
		WeekViewFocuHideEventSlotDescription();
		RctCopyRectangle(focusTriggerP, &weekP->focusedZone);
		WeekViewFocusEventSlot(weekP, true);
		handled = true;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusUpDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
 *				->  focus:  	Current form focused widget index.
 *				->	modifiers:	event modifiers.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static Boolean WeekViewFocusUpDayCell(WeekType*	weekP, uint16_t focus, uint16_t modifiers)
{
	RectangleType 	focusTrigger;
	uint32_t 		focusedRowId;
	int32_t			apptIndex = -1;
	Boolean 		slotOverlaps = false;
	Boolean 		handled = false;
	Boolean			canScroll = false;


	if (focus == noFocus)
	{
		// page up ( in keydown event handler)
		// handle view when focusing is not eneabled
		// nothing to do : just a page up

		return handled;
	}


	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
		case kWeekFocusedState:
			// nothing
			break;

		case kWeekFocusDOWState:
			// exit (go to above widget) and reset gagdet focus state
			WeekViewFocusReset(weekP);
			break;

		case kWeekFocusEventSlotState:
			// select up event in same column scroll when event is not visible
			//(top of the columnn after scroll)
			handled = WeekFocusSetUpFocusTrigger(weekP, kFocusTriggerUp, &focusTrigger);

			if (handled)
			{
				focusedRowId = weekP->focusedRowId;

				handled = WeekViewFocusFindSlot(weekP, &focusTrigger);

				if (handled)
					break;
			}

			// Can we really scroll ?
			if  (WeekViewCanScroll(weekP, winUp))
			{
				// scrooll first
				WeekViewScroll(winUp, false);

				// event is handled
				handled = true;
			}
			else
			{
				// otherwise we juste exit (below widget) and reset focus state

				WeekViewFocuHideEventSlotDescription();
				weekP->focusState = kWeekFocusDOWState;
				GetDateAndDayBounds(weekP, weekP->focusedDay, &weekP->focusedZone);
				WeekViewFocusDOWSlot(weekP, true);
				handled = true;
			}
			break;
	}

	return handled;
}



/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusDownDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
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
static Boolean WeekViewFocusDownDayCell(WeekType* weekP, uint16_t focus, uint16_t modifiers)
{
	RectangleType 	focusTrigger;
	uint32_t 		focusedRowId;
	Boolean 		handled = false;
	Boolean 		canScroll = false;


	if (focus == noFocus)
	{
		// page down (in keydown event handler)
		// handle view when focusing is not eneabled
		// nothing to do : just a page down
		return handled;
	}


	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
		case kWeekFocusedState:
			// nothing
			break;

		case kWeekFocusDOWState:
			// change state
			weekP->focusState = kWeekFocusEventSlotState;

			// focus first bounds
			GetFirstFreeSlotBounds(weekP, weekP->focusedDay, &weekP->focusedZone);

			canScroll = WeekFocusSetUpFocusTrigger(weekP, kFocusTriggerReset, &focusTrigger);

			focusedRowId = weekP->focusedRowId;

			handled = WeekViewFocusFindSlot(weekP, &focusTrigger);
			break;

		case kWeekFocusEventSlotState:
			// select up event in same column scroll when event is not visible
			//(top of the columnn after scroll)

			handled = WeekFocusSetUpFocusTrigger(weekP, kFocusTriggerDown, &focusTrigger);

			if (handled)
			{
				focusedRowId = weekP->focusedRowId;

				handled = WeekViewFocusFindSlot(weekP, &focusTrigger);

				if (handled)
					break;
			}

			// can we really scroll ?
			if (WeekViewCanScroll(weekP, winDown))
			{
				// scrooll first
				WeekViewScroll(winDown, false);

				// event is handled
				handled = true;
			}
			else
			{
				// otherwise we juste exit (below widget) and reset focus state
				WeekViewFocuHideEventSlotDescription();
				WeekViewFocusReset(weekP);
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusRightDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
 *				->  focus:  	Current form focused widget index.
 *				->	modifiers:	event modifiers.
 *
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean WeekViewFocusRightDayCell(WeekType* weekP, uint16_t focus, uint16_t modifiers)
{
	uint16_t 		day;
	Boolean 		handled = false;


	if (focus == noFocus)
	{
		// Focus Goto button
		// on left key press when focusing is not enabled
		// focus Goto button
		FrmNavObjectTakeFocus(sFormP, WeekGoToButton);
		handled = true;
		return handled;
	}


	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
			break;

		case kWeekFocusedState:
			// Go to next week
			WeekViewOffsetDays(daysInWeek);
			handled = true;
			break;

		case kWeekFocusEventSlotState:
			WeekViewFocuHideEventSlotDescription();
			weekP->focusState = kWeekFocusDOWState;

		case kWeekFocusDOWState:
			day = weekP->focusedDay;
			++day;

			if (day < daysInWeek)
			{
				weekP->focusedDay = day;
				weekP->focusedRowId = dbInvalidRowID;

				GetDateAndDayBounds(weekP, weekP->focusedDay, &weekP->focusedZone);

				WeekViewFocusDOWSlot(weekP, true);

				handled = true;
			}
			else
			{
				// Focus Goto Button
				WeekViewFocuHideEventSlotDescription();
				FrmNavObjectTakeFocus(gCurrentFormPtr, WeekGoToButton);
				WeekViewFocusReset(weekP);
				handled = true;
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusLeftDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
 *				->  focus:  	Current form focused widget index.
 *				->	modifiers:	event modifiers.
 *
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean WeekViewFocusLeftDayCell(WeekType* weekP, uint16_t focus, uint16_t modifiers)
{
	Boolean 		handled = false;

	if (focus == noFocus)
	{
		// Focus Week Day Display gadget
		// on left key press when focusing is not enabled
		// focus week view gadget
		FrmNavObjectTakeFocus(sFormP, WeekDayDisplay);
		handled = true;
		return handled;
	}


	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
			break;

		case kWeekFocusedState:
			// Go to previous week
			WeekViewOffsetDays(-daysInWeek);
			handled = true;
			break;

		case kWeekFocusEventSlotState:
			WeekViewFocuHideEventSlotDescription();
			weekP->focusState = kWeekFocusDOWState;

		case kWeekFocusDOWState:
			if (weekP->focusedDay > 0)
			{
				--(weekP->focusedDay);

				GetDateAndDayBounds(weekP, weekP->focusedDay, &weekP->focusedZone);

				WeekViewFocusDOWSlot(weekP, true);

				handled = true;
			}
			else
			{
				// focus WeekDayDisplay gadget
				WeekViewFocuHideEventSlotDescription();
				FrmNavObjectTakeFocus(sFormP, WeekDayDisplay);
				WeekViewFocusReset(weekP);
				handled = true;
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusResize
 *
 * DESCRIPTION: 	Resize focus zone
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusResize(Coord xOffset, Coord yOffset)
{
	WeekType*		weekP	= NULL;
	RectangleType 	neoFocusedZone = {0,0,0,0};
	//Coord			X;
	//Coord			Y;

	TraceOutput(TL(appErrorClass, "WeekViewFocusResize start"));

	weekP = MemHandleLock (sWeekObjectH);


	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
			break;

		case kWeekFocusedState:
			break;

		case kWeekFocusDOWState:
			break;

		case kWeekFocusEventSlotState:

			GetDayOfWeekBounds(weekP, weekP->focusedDay, &neoFocusedZone);

			weekP->focusedZone.topLeft.x += xOffset;
			weekP->focusedZone.topLeft.y += yOffset;
			weekP->focusedZone.extent.x =  neoFocusedZone.extent.x;
			weekP->focusedZone.extent.y =  weekP->hourHeight;

			WeekViewFocusEventSlot(weekP, false);
			break;
	}


	/*
	neoFocusedZone.topLeft.x += xOffset;
	neoFocusedZone.topLeft.y += yOffset;
	neoFocusedZone.extent.y = weekP->hourHeight;
	neoFocusedZone.topLeft.x + xOffset + (neoFocusedZone.extent.x >> 1);
	neoFocusedZone.topLeft.y + yOffset + (weekP->hourHeight >> 1);

	RctCopyRectangle(&neoFocusedZone, &weekP->focusedZone);

	if (WeekViewFindFocusedEventSlot(weekP, weekP->focusedDay, &neoFocusedZone, NULL, NULL))
	{
		RctCopyRectangle(&weekP->focusedZone, &neoFocusedZone);
	}
	*/
	MemHandleUnlock (sWeekObjectH);

	TraceOutput(TL(appErrorClass, "WeekViewFocusResize end"));
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusFirst
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusFirst(WeekType* weekP)
{
	DateTimeType	today;
	DayType*		day;

	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
			break;

		case kWeekFocusedState:
			break;

		case kWeekFocusDOWState:

			// Focus the current day of week of current week
			// else the middle day of week
			TimSecondsToDateTime(TimGetSeconds(), &today);

			// get current day, day of week
			weekP->focusedDay = DayOfWeek (today.month, today.day, today.year);

			// get that day data
			day = &(weekP->days[weekP->focusedDay]);

			// check if it is actually the current day
			if (( (int16_t)(day->date.year+ firstYear) !=  today.year)
				|| (int16_t) day->date.month != today.month
 				|| (int16_t) day->date.day != today.day)
 			{
 				// when not on the current week then we give the focus
 				// first to the middle day of week
 				weekP->focusedDay = (daysInWeek >>1);
 			}

			GetDateAndDayBounds(weekP, weekP->focusedDay, &weekP->focusedZone);
			WeekViewFocusDOWSlot(weekP, true);
			break;

		case kWeekFocusEventSlotState:
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusTake
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusTake(WeekType*	weekP)
{
	FormType* formP;

	if (weekP->focusState == kWeekFocusNoFocus)
		weekP->focusState = kWeekFocusedState;

	formP = FrmGetFormPtr(WeekView);

	FrmSetFocus(formP, FrmGetObjectIndex(formP, WeekDayDisplay));

	WeekViewFocusSetRing(weekP);
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusEnter
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	weekP:		Week view date (locked handle.)
 *				->  focus:  	Current form focused widget index.
 *				->	modifiers:	event modifiers.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean WeekViewFocusEnter(WeekType*	weekP, uint16_t focus, uint16_t modifiers)
{
	FormType*				formP;
	FrmNavStateFlagsType 	navStateFlags = 0;
	Boolean 				handled = false;
	Boolean 				exitView = false;
	status_t				err;


	formP = FrmGetFormPtr(WeekView);
	err = FrmGetNavState (formP, &navStateFlags);


	if (focus == noFocus && !navStateFlags)
	{
		// when there no focus and navigation is not yet enabled
		// focus the  Week gadget
		FrmNavObjectTakeFocus(formP, WeekDayDisplay);
		handled = true;
		return handled;
	}


	//if (focus == noFocus)
	//{
		// handle view when focusing is not eneabled
		// center press is a normal focusing activation
		// does nothing
	//	return handled;
	//}


	if (navStateFlags & kFrmNavStateFlagsObjectFocusMode)
	{
		if (focus == FrmGetObjectIndex(formP, WeekWeekViewButton))
		{
			FrmNavObjectTakeFocus(formP, WeekDayDisplay);
			handled = true;
			return handled;
		}
	}


	switch (weekP->focusState)
	{
		default:
		case kWeekFocusNoFocus:
			break;

		case kWeekFocusedState:
			// change focus state
			// 1 focus enter
			weekP->focusState = kWeekFocusDOWState;

			// week view has focus ring
			WeekViewFocusFirst(weekP);
			handled = true;
			break;

		case kWeekFocusDOWState:
			// go to day view , Creating a new event
			WeekViewFocusActivate(weekP);
			break;

		case kWeekFocusEventSlotState:

			if (weekP->focusedRowId == dbInvalidRowID)
				WeekViewFocusActivate(weekP);
			else
			{
				WeekViewHideTime(true);
				// go to day view, editing an existing event
				WeekViewGoToAppointment(weekP->focusedRowId, weekP->focusedOverlaps);
			}
			handled = true;
			break;
	}

	return handled;
}





/***********************************************************************
 *
 * FUNCTION:		WeekViewFocusReset
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusReset(WeekType*	weekP)
{
	// Week gadget still own the focus ring
	weekP->focusState = kWeekFocusedState;
	weekP->focusedDay = 0;
	weekP->focusedRowId = dbInvalidRowID;
	weekP->focusedApptIndex = -1;
	weekP->focusedOverlaps = false;

	RctSetRectangle (&weekP->focusedZone, 0, 0, 0, 0);
	WeekViewFocuHideEventSlotDescription();
}


/***********************************************************************
 *
 * FUNCTION:		WeekFocusLost
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	weekP:	Week view date (locked handle.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 *
 ***********************************************************************/
static void WeekViewFocusLost(WeekType*	weekP)
{
	WeekViewFocusReset(weekP);

	weekP->focusState = kWeekFocusNoFocus;
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewHandleNavigation
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	modifiers:		Command mofifiers.
 *				-> 	chr:			Command code.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/20/04	Creation.
 * 	PPL 06/16/04	Add PalmSource OneHanded Specifications.
 *
 /***********************************************************************/
Boolean WeekViewHandleNavigation(uint16_t	modifiers, wchar32_t chr )
{
	WeekType*	weekP	= NULL;
	uint16_t	focus	= noFocus;
	Boolean 	handled = false;


	TraceOutput(TL(appErrorClass, "WeekViewHandleNavigationWhileFocused start"));

	focus = FrmGetFocus(sFormP);
	weekP = MemHandleLock (sWeekObjectH);


	if (focus == noFocus)
	{
		// reset focus when there' still no focused item after
		// calling AgendaViewFocusTableRegainFocus()
		sWeekViewFocusedItemID = frmInvalidObjectId;
	}


	switch(chr)
	{
		case vchrRockerUp:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerUp, Above Cell is focused"));
			handled = WeekViewFocusUpDayCell(weekP, focus, modifiers);
			break;


		case vchrRockerDown:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerDown, Below Cell is focused"));
			handled = WeekViewFocusDownDayCell(weekP, focus, modifiers);
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
			handled = WeekViewFocusLeftDayCell(weekP, focus, modifiers);
			break;


		case vchrThumbWheelUp:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelUp Next tab is focused"));


		case vchrRockerRight:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerRight, Next tab is focused"));
			// 5-way rocker right
			// move focused tab to right
			handled = WeekViewFocusRightDayCell(weekP, focus, modifiers);
			break;


		case vchrThumbWheelPush:
			TraceOutput(TL(appErrorClass, "PrvBookGadgetHandleVirtualKeys: keyDownEvent, vchrThumbWheelPush as vchrRockerCenter"));


		case vchrRockerCenter:
			TraceOutput(TL(appErrorClass, "PrvBookGadgetHandleVirtualKeys: keyDownEvent, vchrRockerCenter"));
			// 5-way rocker center/press
			// active focused tab
			handled = WeekViewFocusEnter(weekP, focus, modifiers);
			break;


		default:
			TraceOutput(TL( appErrorClass, "-------------> Not a 5-way or Wheel Navigation vchr"));
			break;
	}

	MemHandleUnlock (sWeekObjectH);

	TraceOutput(TL(appErrorClass, "WeekViewHandleNavigationWhileFocused start"));

	return handled;
}



/***********************************************************************
 *
 * FUNCTION:    	WeekViewHandleEvent
 *
 * DESCRIPTION: 	This routine is the event handler for the "Week View"
 *              	of the Datebook application.
 *
 * PARAMETERS:  ->	eventP:	a pointer to an EventType structure.
 *
 * RETURNED:    	true if the event has handle and should not be
 *					passed to a higher level handler.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	7/24/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *	PPL 04/20/04	Add focusing event for OneHanded navigation.
 *
 ***********************************************************************/
Boolean WeekViewHandleEvent (EventType* eventP)
{
	WeekType*		weekP 	= NULL;
	wchar32_t		chr;
	uint16_t		modifiers;
	FormPtr 		frm;
	DateType 		date;
	Boolean 		handled = false;
	Boolean 		penDown;
	Coord 			x;
	Coord			y;

	static RectangleType titleBounds = {0,0,0,0};

	// Trace the event in debug mode
	DBGTraceEvent("WeekView", eventP);

	switch (eventP->eType)
	{
		case nilEvent:
			// we try to handle th ependow in the title only
			// if we are not currently tracking the pen to move an
			// appointment in the week view

			if (WeekViewIsTrackActive())
				break;

			// Check the draw window then redraw the title.
			if (WinGetDrawWindow() != gWinHdl)
				break;

			EvtGetPen(&x, &y, &penDown);

			if (penDown && !gSecurityPasswordOpened)
				WeekViewShowTime ();
			else
			{
				// Do WeekEraseDescription first - these both depend on the global TimeDisplayed,
				// but WeekEraseDescription also checks week view internal data before doing anything.
				WeekEraseDescription (false);
				WeekViewHideTime (false);
			}
			break;


		case penDownEvent:
			handled = WeekViewPointInDescription (eventP->screenX, eventP->screenY);
			if (handled)
				WeekEraseDescription (true);
			else
				handled = WeekViewStartEventTracking (eventP);

			EventInCurrentView = true;
			break;


		case penMoveEvent:
			handled = WeekViewUpdateEventTracking(eventP);
			break;


		case penUpEvent:
			// Check if the time was displayed and the pen in the initial title rectangle
			if (TimeDisplayed && RctPtInRectangle(eventP->screenX, eventP->screenY, &titleBounds))
				WeekViewHideTime(true);

			handled = WeekViewStopEventTracking(eventP);
			break;


		case keyDownEvent:
			chr 		= eventP->data.keyDown.chr;
			modifiers	= eventP->data.keyDown.modifiers;

			WeekEraseDescription (true);
			WeekViewHideTime (true);


			if (EvtKeydownIsVirtual(eventP))
			{
				if (TxtCharIsRockerKey(modifiers, chr) || TxtCharIsWheelKey(modifiers, chr))
				{
					handled = WeekViewHandleNavigation(modifiers, chr);
				}
				else if (TxtCharIsHardKey(modifiers, chr))
				{
					DateSecondsToDate (TimGetSeconds (), &date);
					Date = date;

					if (EventInCurrentView
						||(modifiers & poweredOnKeyMask))
						FrmGotoForm(gApplicationDbP, DayView);
					else
						FrmGotoForm(gApplicationDbP, MonthView);

					handled = true;
					EventInCurrentView = true;
				}
				else
				{
					switch(chr)
					{
						case vchrPageUp:
							WeekViewScroll (winUp, false);
							//WeekViewOffsetDays(-daysInWeek);
							handled = true;
							//EventInCurrentView = true;
							break;


 						case vchrPageDown:
							WeekViewScroll (winDown, false);
							//WeekViewOffsetDays(daysInWeek);
							handled = true;
							//EventInCurrentView = true;
							break;
					}
				}
			}
			break;


		case keyUpEvent:
			weekP = MemHandleLock (sWeekObjectH);
			WeekViewFocusEventSlotDescriptionIsTimed(weekP);
			MemHandleUnlock (sWeekObjectH);
			break;


		case ctlSelectEvent:

			switch (eventP->data.ctlSelect.controlID)
			{
				case WeekCategoryTrigger:
					WeekViewSelectCategory ();
					handled = true;
					break;


				case WeekGoToButton:
					WeekViewGotoDate ();
					handled = true;
					break;


				case WeekDayViewButton:
					//	'Date' var should already be set.
					//	This can only happen when navigating from week to week,
					//	with prev/next arrows, or when navigating with the date
					//	picker.
					FrmGotoForm(gApplicationDbP, DayView);
					handled = true;
					break;


				case WeekMonthLunarViewButton:
					// Lunar Calendar
					MonthViewSetLunarCalendar(MonthMonthLunarViewButton);

					FrmGotoForm (gApplicationDbP, MonthView);
					handled = true;
					break;


				case WeekMonthViewButton:
					// Lunar Calendar
					MonthViewSetLunarCalendar(MonthMonthViewButton);

					//	'Date' var should already be set.
					//	This can only happen when navigating from week to week,
					//	with prev/next arrows, or when navigating with the date
					//	picker.
					FrmGotoForm(gApplicationDbP, MonthView);
					handled = true;
					break;


				case WeekAgendaViewButton:
					//	'Date' var should already be set.
					//	This can only happen when navigating from week to week,
					//	with prev/next arrows, or when navigating with the date
					//	picker.
					FrmGotoForm(gApplicationDbP, AgendaView);
					handled = true;
					break;

				default:;

			}
			break;


		// Handle the repeating scroll controls.  NOTE: it
		//	is important not to 'handle' repeating controls so
		//	that the next event will be generated correctly.
		case  ctlRepeatEvent:
			switch (eventP->data.ctlRepeat.controlID)
			{
				case WeekUpButton:
					WeekViewScroll (winUp, false);
					break;

				case WeekDownButton:
					WeekViewScroll (winDown, false);
					break;

				case WeekPrevButton:
					// Go to previous week
					WeekViewOffsetDays(-daysInWeek);
					break;

				case WeekNextButton:
					// Go to next week
					WeekViewOffsetDays(daysInWeek);
					break;

				default:;
			}
			break;


		case menuEvent:
			handled = WeekViewDoCommand (eventP->data.menu.itemID);
			break;


		case winUpdateEvent:
			if (gWinHdl != eventP->data.winUpdate.window)
				break;

			WeekViewUpdateDisplay (&eventP->data.winUpdate.dirtyRect);
			WeekViewDrawSelection ();

			handled = true;
			break;


		case winResizedEvent:
			// Active Input Area handling
			if (gWinHdl != eventP->data.winResized.window)
				break;

			WeekViewFormResize(eventP);
			handled = true;
			break;


		case frmOpenEvent:
			WeekViewOpen();
			handled = true;
			break;


		case frmCloseEvent:
			WeekViewClose();
			break;


		case frmTitleEnterEvent:
			// Generate a nilEvent within 100 miliseconds
			frm = FrmGetFormPtr(WeekView);
			GetTitleBounds(frm, &titleBounds);
			TimeComputeWaitTime = true;
			TimeDisplayTick = TimGetTicks() + timeDisplayWaitTime;
			break;


		case frmObjectFocusTakeEvent:
			TraceOutput(TL(appErrorClass, "WeekViewHandleEvent, frmObjectFocusTakeEvent: fid=%hu obj=%hu",
						eventP->data.frmObjectFocusTake.formID,
						eventP->data.frmObjectFocusTake.objectID));

			if (eventP->data.frmObjectFocusTake.formID == WeekView
				&& eventP->data.frmObjectFocusTake.objectID == WeekDayDisplay)
			{
				weekP = MemHandleLock (sWeekObjectH);
				WeekViewFocusTake (weekP);
				MemHandleUnlock (sWeekObjectH);
				handled = true;
			}
			// the event has to be handle by standard navigation handler
			break;


		case frmObjectFocusLostEvent:
			TraceOutput(TL(appErrorClass, "WeekViewHandleEvent, frmObjectFocusLostEvent: fid=%hu obj=%hu",
						eventP->data.frmObjectFocusLost.formID,
						eventP->data.frmObjectFocusLost.objectID));



			if (eventP->data.frmObjectFocusLost.formID == WeekView)
			{
				// remember last focused object id
				sWeekViewFocusedItemID = eventP->data.frmObjectFocusTake.objectID;

				if (eventP->data.frmObjectFocusLost.objectID == WeekDayDisplay)
				{
					weekP = MemHandleLock (sWeekObjectH);
					WeekViewFocusLost (weekP);
					MemHandleUnlock (sWeekObjectH);
				}
			}

			// the event has to be handle by standard navigation handler
			break;


		case datebookRefreshDisplay:
			// Re-build the layout and send a winUpdateEvent on the table
			WeekViewGoToWeek();
			break;
		default:;
	}

	return (handled);
}
