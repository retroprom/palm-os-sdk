/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateUtils.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Used to manage repeat info for appt and quiet mode
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>

#include <Control.h>
#include <ErrorMgr.h>
#include <List.h>
#include <PenInputMgr.h>
#include <Preferences.h>
#include <SelDay.h>
#include <string.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <Table.h>
#include <TraceMgr.h>
#include <UIResources.h>

#include "DateGlobals.h"
#include "DateUtils.h"
#include "SelectTime.h"

#include "DatebookRsc.h"

/***********************************************************************
 *
 *	Internal macros
 *
 ***********************************************************************/
#define int_abs(a) (((a) >= 0) ? (a) : (-a))


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define maxPushPopLevels	5
#define countryCodeLength	3	// (2 + chrNull)


/***********************************************************************
 *
 * FUNCTION:    	ShowObject
 *
 * DESCRIPTION: 	This routine set an object usable and draws the
 *					object if the form it is in is visible.
 *
 * PARAMETERS:  ->	frm:		pointer to a form.
 *              ->	objectID:	id of the object to set usable.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	2/21/95		Initial Revision
 *
 ***********************************************************************/
void ShowObject (FormPtr frm, uint16_t objectID)
{
	FrmShowObject (frm, FrmGetObjectIndex (frm, objectID));
}


/***********************************************************************
 *
 * FUNCTION:    	HideObject
 *
 * DESCRIPTION: 	This routine set an object not-usable and erases it
 *              	if the form it is in is visible.
 *
 * PARAMETERS:  ->	frm:      pointer to a form.
 *              ->	objectID: id of the object to set not usable.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	2/21/95		Initial revision.
 *
 ***********************************************************************/
void HideObject (FormPtr frm, uint16_t objectID)
{
	FrmHideObject (frm, FrmGetObjectIndex (frm, objectID));
}



/***********************************************************************
 *
 * FUNCTION:    	GetTitleBounds
 *
 * DESCRIPTION: 	This routine set an object not-usable and erases it
 *              	if the form it is in is visible.
 *
 * PARAMETERS:  ->	formP:      pointer to a form.
 *              <->	titleBoundsP: tiltle's rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			titleBoundsP is set on exit.
 *
 * REVISION HISTORY:
 *	ART	2/21/95		Initial revision.
 *
 ***********************************************************************/
void GetTitleBounds(FormPtr formP, RectanglePtr titleBoundsP)
{
	uint16_t		objIndex, objCount;

	// The Graffiti Shift Indicator
	objCount = FrmGetNumberOfObjects(formP);
	for (objIndex = 0; objIndex < objCount; objIndex++)
	{
		if (FrmGetObjectType(formP, objIndex) == frmTitleObj)
		{
			FrmGetObjectBounds(formP, objIndex, titleBoundsP);
			return;
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DateBkEnqueueEvent
 *
 * DESCRIPTION: 	Enqueue an  event to be processed by the active form.
 *
 * PARAMETERS:  ->	eventTypeVal:      event type to enqueue.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 03/21/03		Initial revision.
 *
 ***********************************************************************/
void DateBkEnqueueEvent(eventsEnum eventTypeVal)
{
	EventType event;

	memset (&event, 0x00, sizeof(EventType));
	event.eType = eventTypeVal;

	// Push the event
	EvtAddEventToQueue (&event);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvDateTime2Date
 *
 * DESCRIPTION: 	Conversion from DateTimeType to DateType
 *
 * PARAMETERS:  ->	dateTimeP:      event type to enqueue.
 *
 * RETURNED:    	DateType.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 03/21/03		Initial revision.
 *
 ***********************************************************************/
DateType CnvDateTime2Date(DateTimeType* dateTimeP)
{
	DateType result;

	result.day = (uint8_t) dateTimeP->day;
	result.month = (uint8_t) dateTimeP->month;
	result.year = (uint8_t) dateTimeP->year - firstYear;

	return result;
}


/***********************************************************************
 *
 * FUNCTION:    	CnvDateTime2Time
 *
 * DESCRIPTION: 	Conversion from DateTimeType to TimeType
 *
 *
 * PARAMETERS:  ->	dateTimeP:      event type to enqueue.
 *
 * RETURNED:    	TimeType.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 03/21/03		Initial revision.
 *
 ***********************************************************************/
TimeType CnvDateTime2Time(DateTimeType* dateTimeP)
{
	TimeType result;

	result.minutes = (uint8_t) dateTimeP->minute;
	result.hours = (uint8_t) dateTimeP->hour;

	return result;
}


/***********************************************************************
 *
 * FUNCTION:    	CnvDate2DateTime
 *
 * DESCRIPTION: 	Conversion from DateType / TimeType to DateTimeType
 *
 * PARAMETERS:  ->	inDateP:     	date
 *				->	inTimeP:		time
 *				<->	outDateTimeP:	dateTime
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			outDateTimeP is set on exit.
 *
 * REVISION HISTORY:
 *	PLE 03/21/03		Initial revision.
 *
 ***********************************************************************/
void CnvDate2DateTime(DateType* inDateP, TimeType* inTimeP, DateTimeType* outDateTimeP)
{
	// Using inTimeP
	if (inTimeP != NULL)
	{
		outDateTimeP->minute = inTimeP->minutes;
		outDateTimeP->hour = inTimeP->hours;
	}
	else
	{
		outDateTimeP->minute = 0;
		outDateTimeP->hour = 0;
	}

	// Using inDateP
	if (inDateP != NULL)
	{
		outDateTimeP->day = inDateP->day;
		outDateTimeP->month = inDateP->month;
		outDateTimeP->year = inDateP->year + firstYear;
	}
	else
	{
		outDateTimeP->day = 0;
		outDateTimeP->month = 0;
		outDateTimeP->year = 0;
	}

	// Assigned to 0
	outDateTimeP->second = 0;
	outDateTimeP->weekDay = 0;
}



/***********************************************************************
 *
 * FUNCTION:    	GetTime
 *
 * DESCRIPTION: 	This routine selects the start and end time of
 *					an event.
 *
 * PARAMETERS:  	startP:
 *						in:   current start time.
 *                      out: selected start time.
 *
 *              	endtP:
 *						in:   current end time.
 *                      out: selected end time.
 *
 *					wrappingEvtP:
 *						in:   	current midnight wrapping state.
 *                      out:	selected midnight wrapping state.
 *
 *              	titleID:	Resource id of the title to display
 *								in the time picker dialog.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	10/24/95	Initial revision.
 *
 ***********************************************************************/
Boolean GetTime (TimePtr startP, TimePtr endP, DatePtr startDateP, Boolean* wrappingEvtP,
	TimeSelectionType * selectTypeP, uint16_t titleStrID)
{
	MemHandle titleH;
	char* title;
	Boolean selected;
	DateTimeType startDateTime, endDateTime, todayDateTime;
	int16_t	firstDisplayHour;

	CnvDate2DateTime(startDateP, startP, &startDateTime);
	CnvDate2DateTime(startDateP, endP, &endDateTime);

	// Get today's date, are we displaying today?
	TimSecondsToDateTime (TimGetSeconds (), &todayDateTime);
	if (startDateTime.year == todayDateTime.year &&
		startDateTime.month == todayDateTime.month &&
		startDateTime.day == todayDateTime.day)
	{
		firstDisplayHour = todayDateTime.hour+1;
	}
	else // Not today
	{
		firstDisplayHour = DayStartHour;
	}

	// Is the event untimed ?
	if (*selectTypeP != tSelectNoTime)
	{
		if (*wrappingEvtP)
		{
			// If the event is wrapped around midnight, end date
			// is equal to start day + 1
			TimAdjust(&endDateTime, daysInSeconds);
		}
	}

	titleH = DmGetResource(gApplicationDbP, strRsc, titleStrID);
	title = MemHandleLock(titleH);

	selected = SelectTimeV6(title, &startDateTime, &endDateTime, selectTypeP,
		DayStartHour, DayEndHour, false);

	MemHandleUnlock(titleH);
	DmReleaseResource(titleH);

	*startDateP = CnvDateTime2Date(&startDateTime);
	*startP = CnvDateTime2Time(&startDateTime);
	*endP = CnvDateTime2Time(&endDateTime);
	if (startDateTime.day != endDateTime.day)
		*wrappingEvtP = true;
	else
		*wrappingEvtP = false;

	return (selected);
}


/***********************************************************************
 *
 * FUNCTION:    	DateCompare
 *
 * DESCRIPTION: 	This routine compares two dates.
 *
 * PARAMETERS:  ->	d1: a date.
 *              ->	d2:	a date.
 *
 * RETURNED:    	if d1 > d2  returns a positive int.
 *              	if d1 < d2  returns a negative int.
 *              	if d1 = d2  returns zero.
 *
 * NOTE: 			This routine treats the DateType structure like an
 *					unsigned int, it depends on the fact the the members
 *					of the structure are ordered year, month, day form
 *					high bit to low low bit.
 *
 * REVISION HISTORY:
 *	ART	6/12/95		Initial Revision.
 *
 ***********************************************************************/
int16_t DateCompare (DateType d1, DateType d2)
{
	uint16_t int1, int2;

	int1 = DateToInt(d1);
	int2 = DateToInt(d2);

	if (int1 > int2)
		return (1);
	else if (int1 < int2)
		return (-1);

	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    	TimeCompare
 *
 * DESCRIPTION: 	This routine compares two times.  "No time" is
 *					represented by minus one, and is considered less
 *					than all times.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   	 	if t1 > t2  returns a positive int
 *              	if t1 < t2  returns a negative int
 *              	if t1 = t2  returns zero
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/12/95		Initial revision.
 *
 ***********************************************************************/
int16_t TimeCompare (TimeType t1, TimeType t2)
{
	int16_t int1, int2;

	int1 = TimeToInt(t1);
	int2 = TimeToInt(t2);

	if (int1 > int2)
		return (1);
	else if (int1 < int2)
		return (-1);

	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    	RepeatsOnDate
 *
 * DESCRIPTION: 	This routine returns true if a repeating appointment
 *              	occurs on the specified date.
 *
 * PARAMETERS:  ->	repeatInfoP:	repeatInfo struct.
 *				->	startDate:		begining of repeat period.
 *              ->	date:			date to check.
 *
 * RETURNED:    	true if the appointment occurs on the date specified.
 *
 * REVISION HISTORY:
 *	ART	6/14/95		Initial Revision
 *	TES	06/07/02	Moved from DateDB.c
 *
 ***********************************************************************/
Boolean RepeatsOnDate(RepeatInfoType *repeatInfoP, DateType startDate, DateType date)
{
	uint16_t 	freq;
	int32_t 	weeksDiff;
	uint16_t	dayInMonth;
	uint16_t 	dayOfWeek;
	uint16_t 	dayOfMonth;
	uint16_t 	firstDayOfWeek;
	int32_t 	dateInDays;
	int32_t 	startInDays;
	Boolean 	onDate = false;

	// Is the date passed before the start date of the appointment?
	if (DateCompare (date, startDate) < 0)
		return (false);

	// Is the date passed after the end date of the appointment?
	if (DateCompare (date, repeatInfoP->repeatEndDate) > 0)
		return (false);

	// Get the frequency of occurrecne (ex: every 2nd day, every 3rd month, etc.).
	freq = repeatInfoP->repeatFrequency;

	switch (repeatInfoP->repeatType)
	{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (startDate);
			onDate = (Boolean) (((dateInDays - startInDays) % freq) == 0);
			break;


		// Weekly repeating appointment (ex: every Monday and Friday).
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			// Are we on a day of the week that the appointment repeats on.
			dayOfWeek = DayOfWeek (date.month, date.day, (int16_t)(date.year + firstYear));
			onDate = (Boolean) ((1 << dayOfWeek) & repeatInfoP->repeatOn);

			if (! onDate)
				break;

			// Are we in a week in which the appointment occurrs, if not
			// move to that start of the next week in which the appointment
			// does occur.
			dateInDays = DateToDays (date);
			startInDays = DateToDays (startDate);

			firstDayOfWeek = (DayOfWeek (1, 1, firstYear) -
				repeatInfoP->repeatStartOfWeek + daysInWeek) % daysInWeek;

			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) -
							 ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;
			onDate = (Boolean) (weeksDiff == 0);
			break;


//			// Compute the first occurrence of the appointment that occurs
//			// on the same day of the week as the date passed.
//			startDayOfWeek = DayOfWeek (startDate.month, startDate.day,
//				startDate.year+firstYear);
//			startInDays = DateToDays (startDate);
//			if (startDayOfWeek < dayOfWeek)
//				startInDays += dayOfWeek - startDayOfWeek;
//			else if (startDayOfWeek > dayOfWeek)
//				startInDays += dayOfWeek+ (daysInWeek *freq) - startDayOfWeek;
//
//			// Are we in a week in which the appointment repeats.
//			dateInDays = DateToDays (date);
//			onDate = (Boolean) ((((dateInDays - startInDays) / daysInWeek) % freq) == 0);
//			break;


		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Are we in a month in which the appointment repeats.
			onDate = (Boolean)  (((((date.year - startDate.year) * monthsInYear) +
						   (date.month - startDate.month)) % freq) == 0);
			if (! onDate)
				break;

			// Do the days of the month match (ex: 3rd Friday)
			dayOfMonth = DayOfMonth (date.month, date.day, (int16_t)(date.year + firstYear));
			onDate = (Boolean) (dayOfMonth == repeatInfoP->repeatOn);

			if (onDate)
				break;

			// If the appointment repeats on one of the last days of the month,
			// check if the date passed is also one of the last days of the
			// month.  By last days of the month we mean: last sunday,
			// last monday, etc.
			if ((repeatInfoP->repeatOn >= domLastSun)
				&& (dayOfMonth >= dom4thSun))
			{
				dayOfWeek = DayOfWeek (date.month, date.day, (int16_t)(date.year + firstYear));
				dayInMonth = DaysInMonth (date.month, (int16_t)(date.year + firstYear));
				onDate = (Boolean) (((date.day + daysInWeek) > dayInMonth) &&
							 (dayOfWeek == (repeatInfoP->repeatOn % daysInWeek)));
			}
			break;


		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Are we in a month in which the appointment repeats.
			onDate = (Boolean) (((((date.year - startDate.year) * monthsInYear) +
						   (date.month - startDate.month)) % freq) == 0);
			if (! onDate)
				break;

			// Are we on the same day of the month as the start date.
			onDate = (Boolean) (date.day == startDate.day);

			if (onDate)
				break;

			// If the staring day of the appointment is greater then the
			// number of day in the month passed, and the day passed is the
			// last day of the month, then the appointment repeats on the day.
			dayInMonth = DaysInMonth (date.month, (int16_t)(date.year + firstYear));
			onDate = (Boolean) ((startDate.day > dayInMonth) && (date.day == dayInMonth));
			break;


		// Yearly repeating appointment.
		case repeatYearly:
			// Are we in a year in which the appointment repeats.
			onDate = (Boolean) (((date.year - startDate.year) % freq) == 0);

			if (! onDate)
				break;

			// Are we on the month and day that the appointment repeats.
			onDate = (Boolean)  ((date.month == startDate.month) &&
				      (date.day == startDate.day));

			if (onDate)
				break;

			// Specal leap day processing.
			if ( (startDate.month == february)
				&& (startDate.day == 29)
				 && (date.month == february)
 				 && (date.day == DaysInMonth (date.month, (int16_t)(date.year + firstYear))))
			{
				onDate = true;
			}
			break;
	}

	return onDate;
}



/***********************************************************************
 *
 * FUNCTION:    	CnvTm2DateTime
 *
 * DESCRIPTION: 	Converts Tm time structure to DateTime.
 *
 * PARAMETERS:  ->	mytm:
 *				->	date:
 *
 * RETURNED:    	true if the appointment occurs on the date specified.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvTm2DateTime(struct tm* mytm, DateTimeType* date)
{
	date->second = mytm->tm_sec;
	date->minute = mytm->tm_min;
	date->hour = mytm->tm_hour;
	date->day = mytm->tm_mday;
	date->month = mytm->tm_mon + 1;
	date->year = mytm->tm_year + 1900;
	date->weekDay = mytm->tm_wday;
}


/***********************************************************************
 *
 * FUNCTION:    	CnvDateTime2Tm
 *
 * DESCRIPTION: 	Converts DateTime to Tm time structure.
 *
 * PARAMETERS:  ->	date:
 *				->	mytm:
 *
 * RETURNED:    	true if the appointment occurs on the date specified.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvDateTime2Tm(DateTimeType* date, struct tm* mytm)
{
	mytm->tm_sec = date->second;
	mytm->tm_min = date->minute;
	mytm->tm_hour = date->hour;
	mytm->tm_mday = date->day;
	mytm->tm_mon = date->month - 1;
	mytm->tm_year = date->year - 1900;
	mytm->tm_wday = date->weekDay;
	// -1 is autoset
	mytm->tm_isdst = -1;
	mytm->tm_gmtoff = 0;
	mytm->tm_zone = 0;
}



/***********************************************************************
 *
 * FUNCTION:    	CnvDateTime2Tm
 *
 * DESCRIPTION: 	Converts UTC to TimeZone
 *
 * PARAMETERS:  ->	inUTCTimeP:
 *	 			->	timeZoneName:
 *				<->	outTZTimeP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvUTC2TimeZone(
	time_t * 		inUTCTimeP,
	const char * 	timeZoneName,
	DateTimeType * 	outTZTimeP)
{
	if (strcmp(timeZoneName, UTCTimezone))
	{
		struct tm ExpandedTime;

		localtime_tz(inUTCTimeP, timeZoneName, &ExpandedTime);
		CnvTm2DateTime(&ExpandedTime, outTZTimeP);
	}
	else
	{
		CnvTm2DateTime(gmtime(inUTCTimeP), outTZTimeP);
	}

}
/***********************************************************************
 *
 * FUNCTION:    	CnvTimeZone2UTC
 *
 * DESCRIPTION: 	Converts TimeZone to UTC.
 *
 * PARAMETERS:  ->	inTZTimeP:
 *	 			<->	timeZoneName:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
time_t CnvTimeZone2UTC(DateTimeType * inTZTimeP, const char * timeZoneName)
{
	time_t	libcTZTime;
	struct tm ExpandedTime;

	CnvDateTime2Tm(inTZTimeP, &ExpandedTime);
	libcTZTime = mktime_tz(&ExpandedTime, timeZoneName);
	return libcTZTime;
}


/***********************************************************************
 *
 * FUNCTION:    	CnvUTC2LocalTime
 *
 * DESCRIPTION: 	Convert the time_t UTC value into the broken-down time
 *					No-Time events are timezone agnostic, thus they
 *					are always treated as events using the Etc/GMT timezone.
 *
 * PARAMETERS:  ->	inUTCTimeP:
 *	 			<->	outLocalTimeP:
 *				->	noTimeAppt:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvUTC2LocalTime(time_t * inUTCTimeP, DateTimeType * outLocalTimeP,
	Boolean noTimeAppt)
{
	if (noTimeAppt)
		CnvUTC2TimeZone(inUTCTimeP, UTCTimezone, outLocalTimeP);
	else
		CnvUTC2TimeZone(inUTCTimeP, gDeviceTimeZone, outLocalTimeP);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvLocalTime2UTC
 *
 * DESCRIPTION: 	Convert the local broken-down time into a time_t UTC
 *					value. No-Time events are timezone agnostic, thus they
 *					are always treated as events using the Etc/GMT timezone.
 *
 * PARAMETERS:  ->	inUTCTimeP:
 *	 			<->	outLocalTimeP:
 *				->	noTimeAppt:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvLocalTime2UTC(
	DateTimeType * 	inLocalTimeP,
	time_t * 		outUTCTimeP,
	Boolean 		noTimeAppt)
{
	if (noTimeAppt)
		*outUTCTimeP = CnvTimeZone2UTC(inLocalTimeP, UTCTimezone);
	else
		*outUTCTimeP = CnvTimeZone2UTC(inLocalTimeP, gDeviceTimeZone);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvCnvApptLocalTime
 *
 * DESCRIPTION: 	Convert the local broken-down time into a time_t UTC
 *					value. No-Time events are timezone agnostic, thus they
 *					are always treated as events using the Etc/GMT timezone.
 *
 * PARAMETERS:  ->	inApptP:
 *	 			->	inTZP:
 *				<->	outLocalTZP:
 *				<->	outTZP:
 *
 * RETURNED:    	Boolean : true if the time has been updated.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvCnvApptLocalTime(
	ApptDateTimeType * 	inApptP,
	char * 				inTZP,
	ApptDateTimeType * 	outLocalTZP,
	char * 				outTZP)
{
	DateTimeType 		tmpDateTime;
	time_t 				utcTime;
	DateType			endDate;

	// Test if target TZ is the same as initial one. If the appointment is
	// an untimed one, or at least one of the 2 timezones is not assigned,
	// consider that the target timezone is the local device one and do not
	// translate times.
	StrNCopy(outLocalTZP->timeZoneName, inApptP->timeZoneName, TZNAME_MAX);

	if (! StrNCompareAscii(inTZP, outTZP, TZNAME_MAX)
	|| inApptP->noTimeEvent
	|| (inTZP[0] == chrNull)
	||  (outTZP[0] == chrNull))
	{
		*outLocalTZP = *inApptP;
		return false;
	}
	else
		outLocalTZP->noTimeEvent = false;

	// Convert start time & date
	CnvDate2DateTime(&inApptP->date, &inApptP->startTime, &tmpDateTime);

	utcTime = CnvTimeZone2UTC(&tmpDateTime, inTZP);

	CnvUTC2TimeZone(&utcTime, outTZP, &tmpDateTime);

	outLocalTZP->date = CnvDateTime2Date(&tmpDateTime);
	outLocalTZP->startTime = CnvDateTime2Time(&tmpDateTime);

	// Convert end time
	CnvDate2DateTime(&inApptP->date, &inApptP->endTime, &tmpDateTime);

	if (inApptP->midnightWrapped)
		TimAdjust(&tmpDateTime , daysInSeconds);

	utcTime = CnvTimeZone2UTC(&tmpDateTime, inTZP);

	CnvUTC2TimeZone(&utcTime, outTZP, &tmpDateTime);

	outLocalTZP->endTime = CnvDateTime2Time(&tmpDateTime);

	// Set overlapping state
	endDate = CnvDateTime2Date(&tmpDateTime);

	if (DateToInt(endDate) != DateToInt(outLocalTZP->date))
		outLocalTZP->midnightWrapped = true;
	else
		outLocalTZP->midnightWrapped = false;

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	CnvApptTZDateTime2LocalTime
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inApptP:
 *	 			->	inTZP:
 *				<->	outLocalTZP:
 *				<->	outTZP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvApptTZDateTime2LocalTime(
	DateTimeType * 	inApptTZDateTimeP,
	char * 			inTZP,
	DateTimeType * 	outLocalDateTimeP)
{
	time_t 			utcTime;

	// Convert the input date time to UTC
	utcTime = CnvTimeZone2UTC(inApptTZDateTimeP, inTZP);

	// UTC to device local time
	CnvUTC2TimeZone(&utcTime, gDeviceTimeZone, outLocalDateTimeP);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvUTCDateTime2LocalTime
 *
 * DESCRIPTION: 	Convert an UTC time broken down in DateTimeType in
 *					a broken-down local time, using device local timezone.
 *
 *				  	This routine must work with in-place conversion.
 *
 * PARAMETERS:  ->	inUTCDateTimeP:
 *	 			<->	outLocalDateTimeP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvUTCDateTime2LocalTime(DateTimeType * inUTCDateTimeP, DateTimeType * outLocalDateTimeP)
{
	time_t 	utcTime;

	// Assign the local device TZ
	utcTime = CnvTimeZone2UTC(inUTCDateTimeP, UTCTimezone);
	CnvUTC2TimeZone(&utcTime, gDeviceTimeZone, outLocalDateTimeP);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvTZDateTime2UTCTime
 *
 * DESCRIPTION: 	Convert any local time broken down in DateTimeType
 *					in a broken- down UTC time, using device local
 *					timezone.
 *
 *				  This routine must work with in-place conversion.
 *
 * PARAMETERS:  ->	inLocalDateTimeP:
 *	 			->	inTimeZoneP:
 *				<->	outUTCDateTimeP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvTZDateTime2UTCTime(
	DateTimeType * 	inLocalDateTimeP,
	char * 			inTimeZoneP,
	DateTimeType * 	outUTCDateTimeP)
{
	time_t 	utcTime;

	utcTime = CnvTimeZone2UTC(inLocalDateTimeP, inTimeZoneP);
	CnvUTC2TimeZone(&utcTime, UTCTimezone, outUTCDateTimeP);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvUTCDateTime2TZTime
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inLocalDateTimeP:
 *				<->	outUTCDateTimeP:
 *				<->	outTimeZoneP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvUTCDateTime2TZTime(
	DateTimeType * 	inLocalDateTimeP,
	DateTimeType * 	outUTCDateTimeP,
	char * 			outTimeZoneP)
{
	time_t 			utcTime;

	utcTime = CnvTimeZone2UTC(inLocalDateTimeP, UTCTimezone);
	CnvUTC2TimeZone(&utcTime, outTimeZoneP, outUTCDateTimeP);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvApptTZ2LocalTime
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inApptTZP:
 *				->	inRepeatP:
 *				<->	outApptTZP:
 *				<->	outRepeatP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvApptTZ2LocalTime(
	ApptDateTimeType * 	inApptTZP,
	RepeatInfoType * 	inRepeatP,
	ApptDateTimeType * 	outLocalTZP,
	RepeatInfoType * 	outRepeatP)
{
	DateTimeType 		tmpDateTime;
	time_t 				utcTime;
	uint8_t				utcRepeatOnRule;

	// Convert begin, end & overlapp data
	if (PrvCnvApptLocalTime(inApptTZP, inApptTZP->timeZoneName, outLocalTZP, gDeviceTimeZone) && inRepeatP)
	{
		// Convert repeat data
		if (DateToInt(inRepeatP->repeatEndDate) != apptNoEndDate)
		{
			CnvDate2DateTime(&inRepeatP->repeatEndDate, &inApptTZP->startTime, &tmpDateTime);

			utcTime = CnvTimeZone2UTC(&tmpDateTime, inApptTZP->timeZoneName);

			CnvUTC2TimeZone(&utcTime, gDeviceTimeZone, &tmpDateTime);

			outRepeatP->repeatEndDate = CnvDateTime2Date(&tmpDateTime);
		}
		else
			DateToInt(outRepeatP->repeatEndDate) = apptNoEndDate;
	}
	else if (outRepeatP)
	{
		// The event time has not been updated, do not update repeat data
		*outRepeatP = *inRepeatP;
	}

	// Update repeating rules
	if (StrCompareAscii(inApptTZP->timeZoneName, gDeviceTimeZone) && inRepeatP && outRepeatP)
	{
		// Timezones differs: update rules
		utcRepeatOnRule = ComputeRepeatOnForUTC(
										inRepeatP->repeatOn,
										inApptTZP->timeZoneName,
										inRepeatP->repeatType,
										inApptTZP);

		outRepeatP->repeatOn = ComputeRepeatOnForTimeZone(
										utcRepeatOnRule,
										gDeviceTimeZone,
										outRepeatP->repeatType,
										outLocalTZP);
	}

}


/***********************************************************************
 *
 * FUNCTION:    	CnvLocalTime2ApptTZ
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inLocalTimeP:
 *				->	inRepeatP:
 *				<->	outApptTZP:
 *				<->	outRepeatP:
 *
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
void CnvLocalTime2ApptTZ(
	ApptDateTimeType * 	inLocalTimeP,
	RepeatInfoType * 	inRepeatP,
	ApptDateTimeType * 	outApptTZP,
	RepeatInfoType * 	outRepeatP)
{
	DateTimeType 		tmpDateTime;
	time_t 				utcTime;
	uint8_t				utcRepeatOnRule;

	// Copy repeat info (only the date may be updated later in this routine)
	if (outRepeatP != inRepeatP)
		memmove(outRepeatP, inRepeatP, sizeof(RepeatInfoType));

	// Convert begin, end & overlapp data
	if (PrvCnvApptLocalTime(inLocalTimeP, gDeviceTimeZone, outApptTZP, outApptTZP->timeZoneName))
	{
		// Convert repeat data
		if (DateToInt(inRepeatP->repeatEndDate) != apptNoEndDate)
		{
			CnvDate2DateTime(&inRepeatP->repeatEndDate, &inLocalTimeP->startTime, &tmpDateTime);

			utcTime = CnvTimeZone2UTC(&tmpDateTime, gDeviceTimeZone);

			CnvUTC2TimeZone(&utcTime, outApptTZP->timeZoneName, &tmpDateTime);

			outRepeatP->repeatEndDate = CnvDateTime2Date(&tmpDateTime);
		}
	}

	// Update repeating rules
	if (StrCompareAscii(gDeviceTimeZone, outApptTZP->timeZoneName))
	{
		// Timezones differs: update rules
		utcRepeatOnRule = ComputeRepeatOnForUTC(
										inRepeatP->repeatOn,
										gDeviceTimeZone,
										inRepeatP->repeatType,
										inLocalTimeP);

		outRepeatP->repeatOn = ComputeRepeatOnForTimeZone(
										utcRepeatOnRule,
										outApptTZP->timeZoneName,
										outRepeatP->repeatType,
										outApptTZP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	CnvDateType2TimeT
 *
 * DESCRIPTION: 	convet a given date into a time_t.
 *
 * PARAMETERS:  ->	dateP:			Date to convert.
 *				<-	dateInSecondsP:	Date converted as time_t.
 *				->	noTimeAppt: 	false = UTC
 *									true =  Device time zone.
 *
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PPL	10/01/03	Initial revision.
 *
 ***********************************************************************/
 void CnvDateType2TimeT(
	DateType* 			dateP,
 	time_t* 			dateInSecondsP,
 	Boolean				noTimeAppt)
{
	DateTimeType 		tmpDateTime;

	// Compute the range in seconds according to the input date
	CnvDate2DateTime(dateP, NULL, &tmpDateTime);
	CnvLocalTime2UTC(&tmpDateTime, dateInSecondsP, noTimeAppt);
}


/***********************************************************************
 *
 * FUNCTION:    	CnvTimeT2DateType
 *
 * DESCRIPTION: 	convet a given time_t into date.
 *
 * PARAMETERS:  ->	iDateInSecondsP:	Date converted as time_t.
 *				<-	oDateP:				Date to convert.
 *				->	noTimeAppt: 	false = UTC
 *									true =  Device time zone.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PPL	10/01/03	Initial revision.
 *
 ***********************************************************************/
 void CnvTimeT2DateType(
 	time_t* 			iDateInSecondsP,
 	DateType* 			oDateP,
 	Boolean				noTimeAppt)
{
	DateTimeType		localTimeP;

	CnvUTC2LocalTime(iDateInSecondsP, &localTimeP,  noTimeAppt);
	*oDateP = CnvDateTime2Date(&localTimeP);
}


/***********************************************************************
 *
 * FUNCTION:    	DateUtilsGetTmUTCMaxSecs
 *
 * DESCRIPTION: 	return macSeconds as a time_t
 *
 * PARAMETERS:  	maxSecondsAsTm retr.
 *
 * RETURNED:    	macSeconds as a time_t
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PPL	10/01/03	Initial revision.
 *
 ***********************************************************************/
static void DateUtilsGetTmUTCMaxSecs(time_t* maxSecondsAsTm)
{
	DateType 		maxDate;
	DateTimeType 	dateTimeTemp;

	if (maxSecondsAsTm)
	{
		maxDate.year =  numberOfYears-1;
		maxDate.month = december;
		maxDate.day =  DaysInMonth(maxDate.month, maxDate.year + firstYear);


		// years since 1904 (MAC format)
		// Compute the range in seconds according to the input date
		CnvDate2DateTime(&maxDate, NULL, &dateTimeTemp);
		CnvLocalTime2UTC(&dateTimeTemp, maxSecondsAsTm, false);

		*maxSecondsAsTm += (daysInSeconds-1);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	CalculateStartEndRangeTimes
 *
 * DESCRIPTION: 	Calculates the start and end time as time_t given
 *					the first date of range and the number of days
 *					in the range.
 *
 *					For Database cursor limits.
 *					So Add kMaxRangeShiftingDueToTZ to end
 *					and subtract kMaxRangeShiftingDueToTZ from start.
 *					for overlapping events.
 *
 * PARAMETERS:  ->	startOfRangeP:	Start tdate of range.
 *				->	daysInRange:	Number of days in  range.
 *				<-	rangeStartTimeP: Calculated start time of range.
 *									Must be a valid time_t*.
 *				<-	rangeEndTimeP:	Calculated end time of range.
 *									Must be a valid time_t*.
 *				<-	endOfRangeP:	Computed end date of range (ptr can be
 *									NULL to discard this parameter)
 *
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PPL	07/25/03	Initial revision.
 *
 ***********************************************************************/
 void CalculateStartEndRangeTimes(
	DateType* 			startOfRangeP,
 	int32_t 			daysInRange,
 	time_t* 			rangeStartTimeP,
	time_t* 			rangeEndTimeP,
	DateType* 			endOfRangeP)
{
	DateTimeType 		tmpDateTime;
	DateType			endRangeDate;
	uint32_t			endRangeDateInDays;
	time_t				macSecsTmUTC;

	DateUtilsGetTmUTCMaxSecs(&macSecsTmUTC);

	// Compute the range in seconds according to the input date
	CnvDate2DateTime(startOfRangeP, NULL, &tmpDateTime);
	CnvLocalTime2UTC(&tmpDateTime, rangeStartTimeP, false);

	endRangeDateInDays = min(DateToDays(*startOfRangeP) + daysInRange - 1, maxDays);
	DateDaysToDate (endRangeDateInDays, &endRangeDate);
	CnvDate2DateTime(&endRangeDate, NULL, &tmpDateTime);
	CnvLocalTime2UTC(&tmpDateTime, rangeEndTimeP, false);

	// Do not forget that the range end is at the end of the day, so add daysInSeconds to the end range
	*rangeEndTimeP += daysInSeconds - 1;
	// Untimed events are stored UTC in database, so enlarge the range by 13 hours in both directions
	// (GMT+13 - GMT-13 is the maximum range of timezones shift).
	// Moreover, move the start time one day before because of midnight overlapping events starting one
	// day before.
	*rangeStartTimeP -= (kMaxRangeShiftingDueToTZ + daysInSeconds);
	*rangeEndTimeP += kMaxRangeShiftingDueToTZ;

	// we cannot show anything beyong maxSeconds value
	if (*rangeEndTimeP > macSecsTmUTC)
		*rangeEndTimeP = macSecsTmUTC;

	if (endOfRangeP != NULL)
		*endOfRangeP = endRangeDate;
}


/***********************************************************************
 *
 * FUNCTION:    	TimezoneExistsInDatabase
 *
 * DESCRIPTION: 	Check if the specified timezone exists in system
 *					database.
 *
 *				  	This routine must work with in-place conversion.
 *
 * PARAMETERS:  ->	timezone2test:
 *
 * RETURNED:    	True when the time zone exits false otherwise.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLe	06/07/02	Initial revision.
 *
 ***********************************************************************/
Boolean TimezoneExistsInDatabase(char * timezone2test)
{
	return hastimezone(timezone2test);
}


/***********************************************************************
 *
 * FUNCTION: 		PrvShiftRepeatOnValue
 *
 * DESCRIPTION: 	Shift the RepeatOnValue by the offset.
 *					Shift == 1 makes the monday to become a tuesday.
 *
 * PARAMETERS: 	-> 	shiftValue:
 *				<-> repeatOnP: The repeatOn value to update
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 10/09/02	Initial revision.
 *
 ***********************************************************************/
static void PrvShiftRepeatOnValue(int32_t shiftValue, uint8_t * repeatOnP)
{
	int32_t i;

	// Can only shift by -2, -1, 0, 1 or 2 day.
	DbgOnlyFatalErrorIf(int_abs(shiftValue) > 2,"Shifting repeatOn value, illegal shift value.");

	if (shiftValue > 0)
	{
		// Shift on left to make monday becoming tuesday + circular for last day
		for (i = 0; i < shiftValue; i++)
			*repeatOnP = ((*repeatOnP & 0x3F) << 1) | ((*repeatOnP & 0x40) >> 6);
	}
	else if (shiftValue < 0)
	{
		// Shift on right to make tuesday becoming monday + circular for last day
		for (i = 0; i < (-shiftValue); i++)
			*repeatOnP = ((*repeatOnP & 0x7E) >> 1) | ((*repeatOnP & 0x01) << 6);
	}
}


/***********************************************************************
 *
 * FUNCTION: 		ComputeRepeatOnForTimeZone
 *
 * DESCRIPTION: 	Compute the new repeatOn value based on
 *					event start day shifting due to timezones.
 *
 *					Applied when reading from DateBook database.
 *
 * PARAMETERS: 	-> 	UTCRepeatOnValue: 	The repeat on value applied
 *				   						to the date in UTC timezone.
 *				-> 	timeZoneP: 			Database record.
 *				->	repeatType:
 *				->	apptRecWhenP:
 *
 * RETURNED: 		????
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 10/09/02	Initial revision.
 *
 ***********************************************************************/
uint8_t ComputeRepeatOnForTimeZone(
	uint8_t 			UTCRepeatOnValue,
	char * 				timeZoneP,
	RepeatType 			repeatType,
	ApptDateTimeType * 	apptRecWhenP)
{
	uint8_t 			repeatOnForTZDate = UTCRepeatOnValue;
	uint8_t				newRepeatOnForTZDate;
	DateTimeType		timeZoneDateTime;
	DateTimeType		UTCDateTime;
	int32_t				localToUTCDateShift;

	// If the event is untimed, we don't have to shift the RepeatOn value
	if (apptRecWhenP->noTimeEvent)
		return UTCRepeatOnValue;

	// Compute the local dateTime : apptRecP is already assigned with
	// correct local time values
	CnvDate2DateTime(&apptRecWhenP->date, &apptRecWhenP->startTime, &timeZoneDateTime);

	// Compute the UTC date corresponding to specified TZ
	CnvTZDateTime2UTCTime(&timeZoneDateTime, timeZoneP, &UTCDateTime);

	// When calling this function, apptRecP->repeat.repeatType is already set
	switch (repeatType)
	{
		case repeatWeekly:
			// Compute the possiblly day shift between UTC and local dates
			localToUTCDateShift = (int32_t) DateToDays(CnvDateTime2Date(&timeZoneDateTime)) -
				(int32_t) DateToDays(CnvDateTime2Date(&UTCDateTime));
			// Now, shift accordingly to the computed value
			PrvShiftRepeatOnValue(localToUTCDateShift, &repeatOnForTZDate);
			break;

		case repeatMonthlyByDay:
			// 4th or last day in week switching
			UTCRepeatOnValue = (uint8_t) DayOfMonth(UTCDateTime.month, UTCDateTime.day, UTCDateTime.year);
			newRepeatOnForTZDate = (uint8_t) DayOfMonth(timeZoneDateTime.month, timeZoneDateTime.day, timeZoneDateTime.year);

			if (repeatOnForTZDate == UTCRepeatOnValue) // if (old UTC repeat == computed UTC repeat)
			{
				// 4th day in week
				repeatOnForTZDate = newRepeatOnForTZDate;
			}
			else if (repeatOnForTZDate == UTCRepeatOnValue + daysInWeek)
			{
				// Last day in week
				repeatOnForTZDate = newRepeatOnForTZDate + daysInWeek;
			}
			else
			{
				// Set a valid date to go on for release roms
				repeatOnForTZDate = newRepeatOnForTZDate;
				DbgOnlyFatalError("Error in checking UTC monthly repeating rule.");
			}
			break;
	}

	return repeatOnForTZDate;
}


/***********************************************************************
 *
 * FUNCTION: 		ComputeRepeatOnForUTC
 *
 * DESCRIPTION: 	Compute the new repeatOn value based on
 *					event start day shifting due to timezones.
 *
 * PARAMETERS: 	-> 	UTCRepeatOnValue: 	The repeat on value applied
 *				   						to the date in UTC timezone.
 *				-> 	timeZoneP: 			Database record.
 *				->	repeatType:
 *				->	apptRecWhenP:
 *
 * RETURNED: 		????
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 10/09/02	Initial revision.
 *
 ***********************************************************************/
uint8_t ComputeRepeatOnForUTC(
	uint8_t 			TZRepeatOnValue,
	char * 				timeZoneP,
	RepeatType 			repeatType,
	ApptDateTimeType * 	apptRecWhenP)
{
	uint8_t 			repeatOnForUTCDate = TZRepeatOnValue;
	uint8_t				newRepeatOnForUTCDate;
	DateTimeType		timeZoneDateTime;
	DateTimeType		UTCDateTime;
	int32_t				localToUTCDateShift;

	// If the event is untimed, we don't have to shift the RepeatOn value
	if (apptRecWhenP->noTimeEvent)
		return TZRepeatOnValue;

	// Compute the local dateTime : apptRecP is already assigned with
	// correct local time values
	CnvDate2DateTime(&apptRecWhenP->date, &apptRecWhenP->startTime, &timeZoneDateTime);

	// Compute the corresponding UTC date
	CnvTZDateTime2UTCTime(&timeZoneDateTime, timeZoneP, &UTCDateTime);

	switch (repeatType)
	{
		case repeatWeekly:
			// Compute the possiblly day shift between local and UTC dates
			localToUTCDateShift = (int32_t) DateToDays(CnvDateTime2Date(&UTCDateTime))
									- (int32_t) DateToDays(CnvDateTime2Date(&timeZoneDateTime));

			// Now, shift accordingly to the computed value
			PrvShiftRepeatOnValue(localToUTCDateShift, &repeatOnForUTCDate);
			break;

		case repeatMonthlyByDay:
			// 4th or last day in week switching
			TZRepeatOnValue = (uint8_t) DayOfMonth(timeZoneDateTime.month, timeZoneDateTime.day, timeZoneDateTime.year);
			newRepeatOnForUTCDate = (uint8_t) DayOfMonth(UTCDateTime.month, UTCDateTime.day, UTCDateTime.year);

			if (repeatOnForUTCDate == TZRepeatOnValue)
			{
				// 4th day in week
				repeatOnForUTCDate = newRepeatOnForUTCDate;
			}
			else if (repeatOnForUTCDate == TZRepeatOnValue + daysInWeek)
			{
				// Last day in week
				repeatOnForUTCDate = newRepeatOnForUTCDate + daysInWeek;
			}
			else
			{
				// Set a valid date to go on for release roms
				repeatOnForUTCDate = newRepeatOnForUTCDate;
				DbgOnlyFatalError("Error in checking time zone monthly repeating rule.");
			}
			break;

		// Other repeating rules don't need to be updated.
	}

	return repeatOnForUTCDate;
}


/***********************************************************************
 *
 * FUNCTION: 		ComputeRepeatOnForUTC
 *
 * DESCRIPTION: 	Update the currently displayed category. Since only
 *					one category should be displayed on a day/week/month
 *					/agenda view, select from the proposed list the
 *					correct category to display according to the following
 *					rule:
 *				  		- If only one category is proposed, select this one,
 *				  		- If two or more categories are proposed, select the
 *						one that was previously displayed, otherwise
 *						select the first in list.
 *
 * PARAMETERS: 	-> 	newCatSelectionNum:
 *				-> 	newCatSelectionP:
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 *
 ***********************************************************************/
void ChangeCurrentCategories(uint32_t newCatSelectionNum, CategoryID * newCatSelectionP)
{
	Boolean prevCatIDInNewSelection = false;
	uint32_t i = 0;

	if (newCatSelectionNum == 0)
	{
		DateBkCurrentCategoriesCount = 0;
		MemPtrFree(DateBkCurrentCategoriesP);
		DateBkCurrentCategoriesP = NULL;
	}
	else
	{
		// Check from newCatSelectionP if previous selection already existed
		if (DateBkCurrentCategoriesCount > 0)
		{
			for (i=0; i<newCatSelectionNum; i++)
			{
				if (newCatSelectionP[i] == DateBkCurrentCategoriesP[0])
				{
					prevCatIDInNewSelection = true;
					break;
				}
			}
		}

		// Allocate the right size for current categories if it was wrong.
		if (DateBkCurrentCategoriesCount != 1)
		{
			DateBkCurrentCategoriesCount = 1;

			if (DateBkCurrentCategoriesP)
				MemPtrFree(DateBkCurrentCategoriesP);

			DateBkCurrentCategoriesP = MemPtrNew(sizeof(CategoryID));
		}

		if (prevCatIDInNewSelection)
		{
			DateBkCurrentCategoriesP[0] = newCatSelectionP[i];
		}
		else
		{
			// Pick the first category from selected list
			DateBkCurrentCategoriesP[0] = newCatSelectionP[0];
		}
	}
}


/***********************************************************************
 *
 * FUNCTION: 		CategoriesTestEquality
 *
 * DESCRIPTION: 	Test equality between two category lists (IDs must be
 *					in same order).
 *
 * PARAMETERS: 	-> 	numCategories1:  	Number of categories in first set.
 *	 			->	pCategoryIDs1[]:	First set of categories.
 *	 			->	numCategories2:		Number of categories in second set.
 *				->	pCategoryIDs2[]:	Second set of categories.
 *
 * RETURNED: 		true is the 2 lists are the same.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 *
 ***********************************************************************/
Boolean CategoriesTestEquality(
	uint32_t 		numCategories1,
	CategoryID 		pCategoryIDs1[],
	uint32_t 		numCategories2,
	CategoryID 		pCategoryIDs2[])
{
	uint32_t 		i;

	if (numCategories1 != numCategories2)
		return false;

	for (i=0; i<numCategories1; i++)
	{
		if (pCategoryIDs1[i] != pCategoryIDs2[i])
			return false;
	}

	return true;
}


/***********************************************************************
 *
 * FUNCTION: 		DatebookAllCategorySelected
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	 	None.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 *
 ***********************************************************************/
Boolean DatebookAllCategorySelected(void)
{
	if (DateBkCurrentCategoriesCount == 1)
	{
		DbgOnlyFatalErrorIf(DateBkCurrentCategoriesP == NULL, "Inconsistent selected categories");
		if (DateBkCurrentCategoriesP[0] == catIDAll)
			return true;
	}
	return false;
}


/***********************************************************************
 *
 * FUNCTION: 		CustomAcceptBeamDialog
 *
 * DESCRIPTION: 	This routine asks the user if he wants to accept the
 *					data as well as set the category to put the data in.
 *					By default all data will go to the unfiled category,
 *					but the user can select another one.
 *
 *					We store the selected category index in the appData
 *					field of the exchange socket so we have it at the
 *					when we get the receive data launch code later.
 *
 * PARAMETERS: 	-> 	dbP: 		open database that holds category
 *								information.
 *				-> 	askInfoP: 	structure passed on exchange ask
 *								launchcode.
 *
 * RETURNED: 		Error if any
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * bha	09/07/99	Initial revision.
 * GPE	11/09/99	Rewritten to use new ExgDoDialog function.
 *
 ***********************************************************************/
status_t CustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP)
{
	ExgDialogInfoType	exgInfo;
	status_t			err = errNone;
	Boolean				result = false;

	memset(&exgInfo, 0, sizeof(ExgDialogInfoType));

	if (dbP && !gSecurityPasswordOpened)
	{
		// set default category to unfiled
		exgInfo.categoryIndex = dmUnfiledCategory;
		// Store the database ref into a gadget for use by the event handler
		exgInfo.db = dbP;

		// Let the exchange manager run the dialog for us
		result = ExgDoDialog(askInfoP->socketP, &exgInfo, &err);
	}

	if ((err >= errNone) && result)
	{
		// pretend as if user hit OK, we'll now accept the data
		askInfoP->result = exgAskOk;

		// Stuff the category index into the appData field
		askInfoP->socketP->appData = exgInfo.categoryIndex;
	}
	else
	{
		// pretend as if user hit cancel, we won't accept the data
		askInfoP->result = exgAskCancel;
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION: 		GetBitmapWidth
 *
 * DESCRIPTION: 	Get the with of a bitmap in resources, according
 *					to the current density
 *
 * PARAMETERS: 	-> 	resID: 	The ID of the bitmap resource.
 *
 *
 * RETURNED: 		The bitmap width, in current density.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 * 	PPL	09/11/03	DmGetResouirce now uses the resID !
 *
 ***********************************************************************/
int16_t GetBitmapWidth(DmResourceID resID)
{
	MemHandle	resH;
	BitmapType*	bmpP;
	Coord 		width;
	Coord 		height;

	resH = DmGetResource(gApplicationDbP, bitmapRsc, resID);
	bmpP = (BitmapType*) MemHandleLock(resH);

	WinGetBitmapDimensions(bmpP, &width, &height);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);
	return (int16_t)width;
}


/***********************************************************************
 *
 * FUNCTION: 		RectIntersects
 *
 * DESCRIPTION: 	Check is two rectangle has a non empty intersection.
 *
 * PARAMETERS: 	-> 	newCatSelectionNum:
 *				-> 	newCatSelectionP:
 *
 * RETURNED: 		true when rectangles have a non empty intersection,
 *					false otherwise.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 *
 ***********************************************************************/
Boolean RectIntersects(RectangleType * r1P, RectangleType * r2P)
{
	Coord left, right, top, bottom;

	left = max (r1P->topLeft.x, r2P->topLeft.x);
	top = max (r1P->topLeft.y, r2P->topLeft.y);
	right = min (r1P->topLeft.x + r1P->extent.x, r2P->topLeft.x + r2P->extent.x);
	bottom = min (r1P->topLeft.y + r1P->extent.y, r2P->topLeft.y + r2P->extent.y);

	if ((right < left) || (bottom < top))
		return false;

	return true;
}

/********************************************************************
 *
 * FUNCTION: 		DateUtilsGetUpperChar
 *
 * DESCRIPTION: 	Returns the UpperChar of a given char.
 *
 * PARAMETERS: 	-> 	inChar: char "to upper"
 *
 * RETURNED: 		upper char.
 *
 * NOTE:			uses wchar.
 *
 * REVISION HISTORY:
 *	PLe	01/01/03	Initial revision.
 *
 ********************************************************************/
wchar32_t DateUtilsGetUpperChar(wchar32_t inChar)
{
	if (inChar < 128)
	{
		if ((inChar >= chrSmall_A) && (inChar <= chrSmall_Z))
		{
			return(inChar - (chrSmall_A - chrCapital_A));
		}
		else
		{
			return(inChar);
		}
	}
	return(inChar);
}


/***********************************************************************
 *
 * FUNCTION:    	DateUtilsGetDayOfWeek
 *
 * DESCRIPTION: 	This routine returns the day-of-the-week, adjusted
 *					by the preference setting that specifies the first
 *					day-of-the-week.  If the date passed is a Tuesday
 *					and the start day of week is Monday, this routine
 *					will return a value of one.
 *
 * PARAMETERS:	->	month:	month (1-12.)
 *              ->	day:	day (1-31.)
 *              ->	year:	year (1904-2031.)
 *
 * RETURNED:		Day of the week (0-first, 1-second.)
 *
 * NOTE:		None.
 *
 * REVISION HISTORY:
 *	art	6/27/95		Initial revision.
 *
 ***********************************************************************/
uint16_t DateUtilsGetDayOfWeek (uint16_t month, uint16_t day, uint16_t year)
{
	return ((DayOfWeek (month, day, year) - StartDayOfWeek + daysInWeek)% daysInWeek);
}


/***********************************************************************
 *
 * FUNCTION:    	DateUtilsSecVerifyPW
 *
 * DESCRIPTION: 	Close the database, Reopens it in ReadOnly
 *					Calls the SecVerifyPassword and after
 *					Close and reopens the database in ReadWrite.
 *
 *					Before calling SecVerifyPAssword we cannot open
 *					the database  in readwrite since Security services
 *					that mightreopen the application database on behalf
 *					the datebook when user declare its pasword as lost
 *					to delete any private records.
 *
 *					As Palm OS 6.0 grants user to call the Find services
 *					while any modal dialog is opened (this was not the
 *					case in previous version of Palm OS) and particularly
 *					when the security input password is opened: we have to
 *					maintain the database opened in read only to grant
 *					datebook view updates.
 *
 *					The solution is factorized in this function.
 *
 *					The gSecurityPasswordOpened flag is here to prevent
 *					the GotoItem from closing all forms.
 *
 * PARAMETERS:	->	newSecLevel: security level
 *
 * RETURNED:		Result of SecVerifyPassword.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	29/10/03	Initial revision.
 *
 ***********************************************************************/
Boolean DateUtilsSecVerifyPW(privateRecordViewEnum newSecLevel)
{
	uint16_t mode;
	Boolean verifyPW;

	ApptCloseAndReopenDatabaseAndCursor(false, dmModeReadOnly);

	gSecurityPasswordOpened = true;

	verifyPW = SecVerifyPW(newSecLevel);

	if (verifyPW)
	{
		 mode = (newSecLevel == hidePrivateRecords) ?
							dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
	}
	else
	{
		mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
							dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
	}

	gSecurityPasswordOpened = false;

	ApptCloseAndReopenDatabaseAndCursor(false, mode);

	return verifyPW;
}


/***********************************************************************
 *
 * FUNCTION:    	DateUtilsSecSelectViewStatus
 *
 * DESCRIPTION: 	Close the database, Reopens it in ReadOnly
 *					Calls the SecSelectViewStatus and after
 *					Close and reopens the database in ReadWrite.
 *
 *					Before calling SecSelectViewStatus we cannot open
 *					the database  in readwrite since Security services
 *					that mightreopen the application database on behalf
 *					the datebook when user declare its pasword as lost
 *					to delete any private records.
 *
 *					As Palm OS 6.0 grants user to call the Find services
 *					while any modal dialog is opened (this was not the
 *					case in previous version of Palm OS) and particularly
 *					when the security input password is opened: we have to
 *					maintain the database opened in read only to grant
 *					datebook view updates.
 *
 *					The solution is factorized in this function.
 *
 *					The gSecurityPasswordOpened flag is here to prevent
 *					the GotoItem from closing all forms.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		privateRecordViewEnum as returned per SecSelectViewStatus.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	29/10/03	Initial revision.
 *
 ***********************************************************************/
privateRecordViewEnum DateUtilsSecSelectViewStatus(void)
{
	privateRecordViewEnum	secStatus;

	// Close Month view cursor in case it is opened
	if (ApptDB && gMonthViewCursorID != dbInvalidCursorID)
		ApptCloseCursor(&gMonthViewCursorID);

	// Close and reopens database in read only
	ApptCloseAndReopenDatabaseAndCursor(false, dmModeReadOnly);

	gSecurityPasswordOpened = true;

	// Call the Security dialog that could changed the level of security
	// and eventually destroy private records when the password is lost
	PrivateRecordVisualStatus = CurrentRecordVisualStatus = secStatus
		= SecSelectViewStatus();

	gSecurityPasswordOpened = false;

	ApptCloseAndReopenDatabaseAndCursor(false, 0);

	return secStatus;
}


/***********************************************************************
 *
 * FUNCTION:		DateUtilsStripNonPrintableChar
 *
 * DESCRIPTION:		This routine remove all non printable char
 *					in a string.
 *
 *					It occurs in place. No new string.
 *
 * PARAMETERS:	->	stringToStrip: Inout string to check and update.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 *	TEs	07/01/03	Initial revision.
 *	PPL	09/11/03	from AddressTools.c
 *					ToolsUtilsStripNonPrintableChar().
 *
 ***********************************************************************/
char* DateUtilsStripNonPrintableChar(char* stringToStrip)
{
	size_t		offset, offsetDest;
	wchar32_t	outChar;
	Boolean		previousCharIsSpace;

	// Remove all non printable char from the string before to compute anything.
	offset = 0;
	offsetDest = 0;
	outChar = chrNull;

	while(true)
	{
		previousCharIsSpace = (Boolean) (outChar == chrSpace);
		offset += TxtGetNextChar(stringToStrip, offset, &outChar);

		if (outChar == chrNull)
			break;

		if (TxtCharIsPrint(outChar))
		{
			offsetDest += TxtSetNextChar(stringToStrip, offsetDest, outChar);
		}
		else if (! previousCharIsSpace)
		{
			// Check for next char
			TxtGetNextChar(stringToStrip, offset, &outChar);
			if (outChar != chrSpace)
			{
				// The next char is also not a space, so replace the current not
				// printable char by a space.
				offsetDest += TxtSetNextChar(stringToStrip, offsetDest, chrSpace);
			}
		}
	}

	TxtSetNextChar(stringToStrip, offsetDest, outChar);

	return stringToStrip;
}


/***********************************************************************
 *
 * FUNCTION:		DateUtilsSetCategoryTriggerLabel
 *
 * DESCRIPTION:		.
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 *	LFe	07/01/03	Initial revision.
 *
 ***********************************************************************/
void DateUtilsSetCategoryTriggerLabel(FormType *formP, uint16_t triggerID)
{
	uint16_t		objectCount, i;
	RectangleType	titleBounds, categoryBounds;

	// First get the title bounds.
	objectCount = FrmGetNumberOfObjects(formP);

 	for (i = 0; i < objectCount; i++)
 	{
		if (FrmGetObjectType(formP, i) == frmTitleObj)
		{
			FrmGetObjectBounds(formP, i, &titleBounds);
			FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, triggerID), &categoryBounds);
			categoryBounds.topLeft.x = titleBounds.topLeft.x + titleBounds.extent.x + 3;
			categoryBounds.extent.x = gCurrentWinBounds.extent.x - categoryBounds.topLeft.x - 10;
			CatMgrTruncateName(ApptDB, DateBkCurrentCategoriesP, DateBkCurrentCategoriesCount, categoryBounds.extent.x, DateBkCategoriesName);
			CtlSetLabel(FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, triggerID)), DateBkCategoriesName);
			break;
		}
 	}
}

#if defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF

/***********************************************************************
 *
 * FUNCTION: 		DatebookTraceAppt
 *
 * DESCRIPTION: 	a few debug traces
 *
 * PARAMETERS: 	-> 	apptRecP:
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 *
 ***********************************************************************/
void DebugTraceAppt(ApptDBRecordType * apptRecP)
{
	// Trace the description
	if (apptRecP->description)
		TraceOutput(TL(appErrorClass,"APPT: %s", apptRecP->description));
	else
		TraceOutput(TL(appErrorClass,"APPT: -Not assigned-"));

	// Trace the date
	TraceOutput(TL(appErrorClass, "   Start: %d:%d", apptRecP->when.startTime.hours,
		apptRecP->when.startTime.minutes));
	TraceOutput(TL(appErrorClass, "   End:  %d:%d", apptRecP->when.endTime.hours,
		apptRecP->when.endTime.minutes));
	TraceOutput(TL(appErrorClass, "   Date:  %d/%d/%d", apptRecP->when.date.month,
		apptRecP->when.date.day, apptRecP->when.date.year+firstYear));
	TraceOutput(TL(appErrorClass, "   Timezone: %s", apptRecP->when.timeZoneName));
	TraceOutput(TL(appErrorClass, "   MidnightWrap: %d", apptRecP->when.midnightWrapped));
	TraceOutput(TL(appErrorClass, "   NoTime: %d", apptRecP->when.noTimeEvent));
}



/***********************************************************************
 *
 * FUNCTION: 		DebugTraceEvent
 *
 * DESCRIPTION: 	A few debug traces
 *
 * PARAMETERS: 	-> 	descP:
 *				-> 	eventP:
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE 06/07/02	Initial revision.
 *
 ***********************************************************************/
void DebugTraceEvent(char * descP, EventType* eventP)
{
	switch (eventP->eType)
	{
		case frmOpenEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** frmOpenEvent", descP));
			break;

		case frmUpdateEvent:
		case winUpdateEvent:
			// Skip cursor blinking event
			if (eventP->data.frmUpdate.dirtyRect.extent.x == 2)
				break;

			TraceOutput(TL(appErrorClass,"******** %s **** frm/WinUpdateEvent ClipRect = [%d,%d] - [%d,%d] UpdtCode = %x",
				descP,
				eventP->data.frmUpdate.dirtyRect.topLeft.x,
				eventP->data.frmUpdate.dirtyRect.topLeft.y,
				eventP->data.frmUpdate.dirtyRect.extent.x,
				eventP->data.frmUpdate.dirtyRect.extent.y,
				eventP->data.frmUpdate.updateCode));
			break;

		case winResizedEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** winResizedEvent ClipRect = [%d,%d] - [%d,%d] UpdtCode = %x",
				descP,
				eventP->data.winResized.newBounds.topLeft.x,
				eventP->data.winResized.newBounds.topLeft.y,
				eventP->data.winResized.newBounds.extent.x,
				eventP->data.winResized.newBounds.extent.y,
				eventP->data.frmUpdate.updateCode));
			break;

		case penDownEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** penDownEvent", descP));
			break;

		case penMoveEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** penMoveEvent", descP));
			break;

		case penUpEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** penUpEvent", descP));
			break;

		case tblEnterEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** tblEnterEvent", descP));
			break;

		case tblSelectEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** tblSelectEvent", descP));
			break;

		case tblExitEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** tblExitEvent", descP));
			break;

		case keyDownEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** keyDownEvent", descP));
			break;

		case frmCloseEvent:
			TraceOutput(TL(appErrorClass,"******** %s **** frmCloseEvent", descP));
			break;

		case datebookRefreshDisplay:
			TraceOutput(TL(appErrorClass,"******** %s **** datebookRefreshDisplay", descP));
			break;

		case datebookPrefPlaySound:
			TraceOutput(TL(appErrorClass,"******** %s **** datebookPrefPlaySound", descP));
			break;
	}
}

#endif // defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF
