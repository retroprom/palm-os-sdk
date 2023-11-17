/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDateTime.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <sys/time.h>
#include <CmnErrors.h>
#include <Preferences.h>
#include <DateTime.h>
#include <TimeMgr.h>
#include <Chars.h>
#include <MemoryMgr.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <TraceMgr.h>
#include <string.h>


// Use this to determine whether the vCalendar 1.0 format used for beaming events uses
// universal time (GMT) instead of local time. Both formats are supported by the standard,
// but Palm OS 3.5 and older devices have always used local time. Using universal time
// causes problems when beaming to these older Palm devices, since they don't know how to
// convert from GMT to local time. Turn this on for testing the ability of this version
// of Datebook to handle incoming beams from devices which use universal time, such as a
// Windows laptop beaming a vCalendar exported from Outlook (or from Palm desktop).
#define GENERATE_UNIVERSAL_TIME_VCALENDARS	0
#define identifierLengthMax					40
#define UTCTimezone							"UTC"

/***********************************************************************
 *
 * FUNCTION:    CnvDateTime2Date
 *
 * DESCRIPTION: Conversion from DateTimeType to DateType
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static DateType CnvDateTime2Date(DateTimeType* dateTimeP)
{
	DateType result;

	result.day = (uint8_t) dateTimeP->day;
	result.month = (uint8_t) dateTimeP->month;
	result.year = (uint8_t) dateTimeP->year - firstYear;

	return result;
}

/***********************************************************************
 *
 * FUNCTION:    CnvDateTime2Time
 *
 * DESCRIPTION: Conversion from DateTimeType to TimeType
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static TimeType CnvDateTime2Time(DateTimeType* dateTimeP)
{
	TimeType result;

	result.minutes = (uint8_t) dateTimeP->minute;
	result.hours = (uint8_t) dateTimeP->hour;

	return result;
}

/***********************************************************************
 *
 * FUNCTION:    CnvDate2DateTime
 *
 * DESCRIPTION: Conversion from DateType / TimeType to DateTimeType
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void CnvDate2DateTime(DateType* inDateP, TimeType* inTimeP, DateTimeType* outDateTimeP)
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
 * FUNCTION:    CnvTm2DateTime
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *			PLe		06/07/02	Initial revision
 *
 ***********************************************************************/
static void CnvTm2DateTime(struct tm* mytm, DateTimeType* date)
{
	date->second	= (int16_t)mytm->tm_sec;
	date->minute	= (int16_t)mytm->tm_min;
	date->hour		= (int16_t)mytm->tm_hour;
	date->day		= (int16_t)mytm->tm_mday;
	date->month		= (int16_t)mytm->tm_mon + 1;
	date->year		= (int16_t)mytm->tm_year + 1900;
	date->weekDay	= (int16_t)mytm->tm_wday;
}


/***********************************************************************
 *
 * FUNCTION:    CnvDateTime2Tm
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *			PLe		06/07/02	Initial revision
 *
 ***********************************************************************/
static void CnvDateTime2Tm(DateTimeType* date, struct tm* mytm)
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
 * FUNCTION:    CnvTimeZone2UTC
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *			PLe		06/07/02	Initial revision
 *
 ***********************************************************************/
static time_t CnvTimeZone2UTC(DateTimeType * inTZTimeP, const char * timeZoneName)
{
	time_t	libcTZTime;
	struct tm ExpandedTime;

	CnvDateTime2Tm(inTZTimeP, &ExpandedTime);
	libcTZTime = mktime_tz(&ExpandedTime, timeZoneName);
	return libcTZTime;
}

/***********************************************************************
 *
 * FUNCTION:    PrvUTCToTimeZone
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *			PLe		06/07/02	Initial revision
 *
 ***********************************************************************/
static void CnvUTC2TimeZone(time_t * inUTCTimeP, const char * timeZoneName,
	DateTimeType * outTZTimeP)
{
	struct tm ExpandedTime;

	localtime_tz(inUTCTimeP, timeZoneName, &ExpandedTime);
	CnvTm2DateTime(&ExpandedTime, outTZTimeP);
}

/***********************************************************************
 *
 * FUNCTION:    CnvUTCDateTime2LocalTime
 *
 * DESCRIPTION: Convert an UTC time broken down in DateTimeType in a broken-
 *				down local time, using device local timezone.
 *				  This routine must work with in-place conversion.
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * HISTORY:
 *			PLe		06/07/02	Initial revision
 *
 ***********************************************************************/
static void CnvUTCDateTime2LocalTime(DateTimeType * inUTCDateTimeP, DateTimeType * outLocalDateTimeP)
{
	char 	deviceTimeZone[TZNAME_MAX];
	time_t 	utcTime;

	// Assign the local device TZ
	gettimezone(deviceTimeZone, TZNAME_MAX);
	utcTime = CnvTimeZone2UTC(inUTCDateTimeP, UTCTimezone);
	CnvUTC2TimeZone(&utcTime, deviceTimeZone, outLocalDateTimeP);
}

/************************************************************
 *
 * FUNCTION: MatchDateTimeToken
 *
 * DESCRIPTION: Extract date and time from the given string,
 *					 converting from GMT to local time if necessary.
 *
 * PARAMETERS:
 *		tokenP		-	string ptr from which to extract
 *		dateP		-	ptr where to store date (optional)
 *		timeP		-	ptr where to store time (optional)
 *		noTimeEventP - ptr where to store noTime flag (optional)
 *
 * RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	12/3/97	Stolen from rest of code & modified
 *
 *************************************************************/
void MatchDateTimeToken(const char*	tokenP, DateType* dateP, TimeType* timeP,
	Boolean* noTimeEventP)
{
    char			identifier[identifierLengthMax];
    int32_t			nv;
    DateType		date;
    TimeType		time;
    Boolean			noTimeEvent = false;
    DateTimeType	dateTime;

    // Use identifier[] as a temp buffer to copy parts of the vCal DateTime
    // so we can convert them to correct form.  This date portion
    // is 4 chars (date) + 2 chars (month) + 2 chars (day) = 8 chars long.
    // Optional Z suffix for universal time yields total of 9 chars long.

    // Get the date whether desired by caller or not. It must precede the time.
    // Read the Year.
    strncpy(identifier, tokenP, 4);
    identifier[4] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < firstYear || lastYear < nv)
        nv = firstYear;
    date.year = (int16_t)(nv - firstYear);
    tokenP += strlen(identifier) * sizeof(char);

    // Read the Month.
    strncpy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || 12 < nv)
        nv = 1;
    date.month = (int16_t)nv;
    tokenP += strlen(identifier) * sizeof(char);

    // Read the Day.
    strncpy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || 31 < nv)
        nv = 1;
    date.day = (int16_t)nv;
    tokenP += strlen(identifier) * sizeof(char);

    // Get the time whether desired by caller or not.
    // Check to see if there is a time value, if so read it in,
    // if not assume that the event has no time.
    if (StrNCaselessCompare(tokenP, "T", strlen("T")) != 0)
    {
        noTimeEvent = true;
        time.hours = 0;
        time.minutes = 0;
    }
    else
    {
        // Move over the time/date separator
        tokenP = tokenP + sizeOf7BitChar('T');

        // Read in the Hours
        strncpy(identifier, tokenP, 2);
        identifier[2] = nullChr;
        nv = StrAToI(identifier);
        // Validate the number and use it.
        if (nv < 0 || 24 <= nv)
            nv = 0;
        time.hours = (int16_t)nv;
        tokenP += strlen(identifier) * sizeof(char);

        // Read in Minutes
        strncpy(identifier, tokenP, 2);
        identifier[2] = nullChr;
        nv = StrAToI(identifier);
        // Validate the number and use it.
        if (nv < 0 || 59 < nv)
            nv = 1;
        time.minutes = (int16_t)nv;
        tokenP += strlen(identifier) * sizeof(char);

        // Skip the Seconds
        tokenP += 2 * sizeof(char);

        // Read the universal time indicator if present
        if (StrNCaselessCompare(tokenP, "Z", strlen("Z")) == 0)
        {
            // Convert the time as parsed from GMT to local time.
            CnvDate2DateTime(&date, &time, &dateTime);
            CnvUTCDateTime2LocalTime(&dateTime, &dateTime);
            date = CnvDateTime2Date(&dateTime);
            time = CnvDateTime2Time(&dateTime);
        }
    }

    // Give the date and/or time to the caller.
    if (dateP != NULL)
        memmove(dateP, &date, sizeof(date));

    if (timeP != NULL)
        memmove(timeP, &time, sizeof(time));

    if (noTimeEventP != NULL)
    	*noTimeEventP = noTimeEvent;
}

/************************************************************
 *
 * FUNCTION: GenerateDateTimeToken
 *
 * DESCRIPTION: Print a date and time into a given string in
 *					 the vCalendar 1.0 format.
 *
 * PARAMETERS:
 *		outputString	-	string ptr to write the output
 *		dateP				-	ptr to date to print (required)
 *		timeP				-	ptr to time to print (optional)
 *
 * RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			peter		3/30/00	Initial Revision.
 *
 *************************************************************/
void GenerateDateTimeToken(char* outputString, DateType* dateP, TimeType* timeP)
{
#if GENERATE_UNIVERSAL_TIME_VCALENDARS
	ErrFatalDisplay("UTC for export not handled in AddressBook, get code from DateBook if needed.");
#else
    if (timeP == NULL)
    {
        TraceOutput(TL(appErrorClass, "GenerateDateTimeToken: (if) %d%02d%02dT000000", firstYear + dateP->year, dateP->month, dateP->day));
        sprintf(outputString, "%d%02d%02d", firstYear + dateP->year, dateP->month, dateP->day);
    }
    else
    {
        TraceOutput(TL(appErrorClass, "GenerateDateTimeToken: (else) %d%02d%02dT%02d%02d00", firstYear + dateP->year, dateP->month, dateP->day, timeP->hours, timeP->minutes));
         sprintf(outputString, "%d%02d%02dT%02d%02d00", firstYear + dateP->year, dateP->month, dateP->day, timeP->hours, timeP->minutes);
    }
#endif
}


/************************************************************
 *
 * FUNCTION: GenerateDateTimeTokenForSeconds
 *
 * DESCRIPTION: Print a date and time into a given string in
 *					 the vCalendar 1.0 format.
 *
 * PARAMETERS:
 *		outputString	-	string ptr to write the output
 *		seconds			-	second count to print
 *
 * RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			peter		3/30/00	Initial Revision.
 *
 *************************************************************/
void GenerateDateTimeTokenForSeconds(char* outputString, uint32_t seconds)
{
    DateTimeType dateTime;

#if GENERATE_UNIVERSAL_TIME_VCALENDARS
	ErrFatalDisplay("UTC for export not handled in AddressBook, get code from DateBook if needed.");
#else
    TimSecondsToDateTime(seconds, &dateTime);
    sprintf(outputString, "%d%02d%02dT%02d%02d00", dateTime.year, dateTime.month,
              dateTime.day, dateTime.hour, dateTime.minute);
#endif
}
