/******************************************************************************
 *
 * Copyright (c) 1998-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateTransfer.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      DateBook routines to transfer records.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <CatMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <Preferences.h>
#include <SchemaDatabases.h>
#include <string.h>
#include <StringMgr.h>
#include <Table.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <ExgLocalLib.h>
#include <UDAMgr.h>
#include <UIResources.h>

#include "DateGlobals.h"
#include "DateAgenda.h"
#include "DateTransfer.h"
#include "DateUtils.h"

#include "DatebookRsc.h"

/***********************************************************************
 *
 *	Local definitions
 *
 ***********************************************************************/

// Use this to determine whether the vCalendar 1.0 format used for beaming events uses
// universal time (GMT) instead of local time. Both formats are supported by the standard,
// but Palm OS 3.5 and older devices have always used local time. Using universal time
// causes problems when beaming to these older Palm devices, since they don't know how to
// convert from GMT to local time. Turn this on for testing the ability of this version
// of Datebook to handle incoming beams from devices which use universal time, such as a
// Windows laptop beaming a vCalendar exported from Outlook (or from Palm desktop).
#define GENERATE_UNIVERSAL_TIME_VCALENDARS	0

// vObject increment size (used to increase the initial size)
#define ALLOCATE_VOBJECT_INC_SIZE			512

// Rough size estimate of a common vCard (doesn't include note, location & description sizes)
#define VCARD_LENGTH_ROUGH_ESTIMATE			256

// Local Constants
#define kIdentifierLengthMax				40
#define kTempStringLengthMax				24			// AALARM is longest
#define kRRULEStringLengthMax				48			// Used to construct the whole RRULE value
#define kDateDBType							'DATA'
#define kDateSuffix							("." dateExtension)
#define kVObjectPalmOSV6Version				"6.1"

// Internal temporary file used to store vCals sent from exchange manager
#define kDateBookvCalTmpFileName			"DateBkTmpFile"
#define VCAL_TMP_BUFFER_SIZE				256

/***********************************************************************
 *
 *	Local macros definitions
 *
 ***********************************************************************/
#define BitAtPosition(pos)              	((uint32_t)1 << (pos))
#define GetBitMacro(bitfield, index)		((bitfield) & BitAtPosition(index))
#define SetBitMacro(bitfield, index)		((bitfield) |= BitAtPosition(index))



/***********************************************************************
 *
 * FUNCTION:		PrvTransferCleanFileName
 *
 * DESCRIPTION:		Remove dot characters in file name but not the least.
 *
 * PARAMETERS:	->	ioFileName: a pointer to a string.
 *
 * RETURNED:		String parameter doesn't contains superfluous dot
 *					characters.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ABa	7/28/00		Initial revision.
 *
 ***********************************************************************/
static void PrvTransferCleanFileName(char* ioFileName)
{
	char* 	mayBeLastDotP;
	char*  	lastDotP;
	uint32_t	chrFullStopSize = TxtCharSize(chrFullStop);

	// prevent NULL & empty string
	if (ioFileName == NULL || *ioFileName == 0)
		return;

	// remove dot but not the last one
	mayBeLastDotP = StrChr(ioFileName, 	chrFullStop);
	while ( (lastDotP = StrChr(mayBeLastDotP + chrFullStopSize, chrFullStop) )  != 0 )
	{
		// remove the dot
		strcpy(mayBeLastDotP, mayBeLastDotP + chrFullStopSize);
		mayBeLastDotP = lastDotP - chrFullStopSize;
	}
}

/***********************************************************************
 *
 * FUNCTION:    	ApptGetAlarmTimeVCalForm
 *
 * DESCRIPTION: 	This routine determines the date and time of the
 *					next alarm for the appointment passed.
 *
 * PARAMETERS:  ->	apptRec:	Pointer to an appointment record.
 *
 * RETURNED:    	Date and time of the alarm, in seconds, or zero if
 *					there is no alarm.
 *
 * NOTE:			The only differences between this function and the
 *             		function ApptGetAlarmTime in the App is that
 *			 		this function does not return 0 if the alarm time
 *					has passed and it returns the first event Date as
 *					the alarm date for reapeating events.
 *
 * REVISION HISTORY:
 *	ART	6/20/95		Initial revision.
 *	DJK	7/28/97		Modified for vCal.
 *
 ***********************************************************************/
static uint32_t ApptGetAlarmTimeVCalForm (ApptDBRecordPtr apptRec)
{
    uint32_t 	advance;
    uint32_t 	alarmTime;
    DateTimeType apptDateTime;


    // An alarm on an untimed event triggers at midnight.
    if (apptRec->when.noTimeEvent)
    	CnvDate2DateTime(&(apptRec->when.date), NULL, &apptDateTime);
    else
    	CnvDate2DateTime(&(apptRec->when.date), &(apptRec->when.startTime), &apptDateTime);

    // Compute the time of the alarm by adjusting the date and time
    // of the appointment by the length of the advance notice.
    advance = apptRec->alarm.advance;
    switch (apptRec->alarm.advanceUnit)
    {
	    case aauMinutes:
	        advance *= minutesInSeconds;
	        break;
	    case aauHours:
	        advance *= hoursInSeconds;
	        break;
	    case aauDays:
	        advance *= daysInSeconds;
	        break;
    }

    alarmTime = TimDateTimeToSeconds (&apptDateTime) - advance;

    return alarmTime;
}


/***********************************************************************
 *
 * FUNCTION: 		TranslateAlarm
 *
 * DESCRIPTION: 	Translate an alarm in seconds to a
 *					DateTimeType.
 *
 *					Broken out of DateImportVEvent for linking
 *					on the device purposes.
 *
 * PARAMETERS:	->	newDateRecordP:	The new record.
 *				->	alarmDTinSec:	Date and time of the alarm
 *									in seconds
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RFL	9/19/97		Initial revision.
 *
 ***********************************************************************/
static void TranslateAlarm(ApptDBRecordType *newDateRecordP, uint32_t alarmDTinSec)
{

    DateTimeType eventDT;
    uint32_t alarmAdvanceSeconds;
    uint32_t alarmAdvance;

    // Assign new AlarmInfoType
    eventDT.year  = newDateRecordP->when.date.year + firstYear;
    eventDT.month  = newDateRecordP->when.date.month;
    eventDT.day  = newDateRecordP->when.date.day;

    if (newDateRecordP->when.noTimeEvent)
	    CnvDate2DateTime(&(newDateRecordP->when.date), NULL, &eventDT);
    else
	    CnvDate2DateTime(&(newDateRecordP->when.date), &(newDateRecordP->when.startTime), &eventDT);

	// Check if the alarm is after the event.
	// In this case set it to the same time since we don't handle post event alarm
	if (TimDateTimeToSeconds(&eventDT) >= alarmDTinSec)
	    alarmAdvanceSeconds = TimDateTimeToSeconds(&eventDT) - alarmDTinSec;
    else
		alarmAdvanceSeconds = 0;

	// convert to minutes
    alarmAdvance = alarmAdvanceSeconds / minutesInSeconds;

    if (alarmAdvance < 100 && alarmAdvance != hoursInMinutes)
    {
        newDateRecordP->alarm.advanceUnit = aauMinutes;
        newDateRecordP->alarm.advance = (int8_t) alarmAdvance;
    }
    else
    {
        // convert to hours
        alarmAdvance = alarmAdvance / hoursInMinutes;

        if (alarmAdvance < 100 && (alarmAdvance % hoursPerDay))
        {
            newDateRecordP->alarm.advanceUnit = aauHours;
            newDateRecordP->alarm.advance = (int8_t) alarmAdvance;
        }
        else
        {
            // convert to days
            alarmAdvance = alarmAdvance / hoursPerDay;

            // set to the lesser of 99 and alarmAdvance
            newDateRecordP->alarm.advanceUnit = aauDays;

            if (alarmAdvance < 99)
                newDateRecordP->alarm.advance = (int8_t) alarmAdvance;
            else
                newDateRecordP->alarm.advance = 99;
        }
    }
}



/***********************************************************************
 *
 * FUNCTION: 		GetToken
 *
 * DESCRIPTION: 	Extracts first available token from given
 *					string. Tokens are assumed to be separated
 *					by "white space", as used by the IsSpace()
 *					function.
 *
 * PARAMETERS:	->	startP:	String ptr from which to extract.
 *				->	tokenP:	String ptr where to store found
 *							token.
 *
 * RETURNED: 		String ptr of where to start next token,
 *					or null if end of string is reached.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	FRI	12/3/97		Stolen from rest of code & modified.
 *
 ***********************************************************************/
static char* GetToken(char* startP, char* tokenP)
{
    char c;

    // Skip leading "blank space"
    while (TxtCharIsSpace(*startP))
        startP += TxtNextCharSize(startP, 0);

    // Get first char
    c = *startP;
    // While char is not terminator, nor is it "blank space"
    while (c != '\0' && !TxtCharIsSpace(c))
    {
        // Copy char to token
        *tokenP++ = c;
        // Advance to next char
        c = *(++startP);
    }

    // Terminate token
    *tokenP = '\0';

    // Skip trailing "blank space"
    if (c != '\0')
        while (TxtCharIsSpace(*startP))
            ++startP;

    // Return next token ptr
    return ((*startP == '\0') ? NULL : startP);
}


/***********************************************************************
 *
 * FUNCTION: 		MatchDateTimeToken
 *
 * DESCRIPTION: 	Extract date and time from the given
 *					string, converting from GMT to local time
 *					if necessary.
 *
 * PARAMETERS:	->	tokenP:	String ptr from which to extract.
 *				->	dateP:	Ptr where to store date (optional.)
 *				->	timeP:	Ptr where to store time (optional.)
 *
 * RETURNED: 		true if the date was specified in UTC time.
 *
 * REVISION HISTORY:
 *	FRI	12/3/97		Stolen from rest of code & modified
 *	PEP	3/29/00		Add support for universal time format,
 *					converting to local time based on current
 *					time zone settings.
 *
 ***********************************************************************/
static Boolean MatchDateTimeToken(
	const char*		tokenP,
	DateType*		dateP,
	TimeType*		timeP,
	Boolean*		noTimeEventP)
{
    char			identifier[kIdentifierLengthMax];
    int				nv;
    DateType		date;
    TimeType		time;
    Boolean			noTimeEvent = false;
    DateTimeType	dateTime;
	Boolean			dateInUTC = false;

    // Use identifier[] as a temp buffer to copy parts of the vCal DateTime
    // so we can convert them to correct form.  This date portion
    // is 4 chars (date) + 2 chars (month) + 2 chars (day) = 8 chars long.
    // Optional Z suffix for universal time yields total of 9 chars long.

    // Get the date whether desired by caller or not. It must precede the time.
    // Read the Year.
    StrNCopy(identifier, tokenP, 4);
    identifier[4] = nullChr;
    nv = StrAToI(identifier);

    // Validate the number and use it.
    if (nv < firstYear || lastYear < nv)
        nv = firstYear;

    date.year = nv - firstYear;
    tokenP += strlen(identifier) * sizeof(char);

    // Read the Month.
    StrNCopy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || 12 < nv)
        nv = 1;

    date.month = nv;
    tokenP += strlen(identifier) * sizeof(char);

    // Read the Day.
    StrNCopy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);

    // Validate the number and use it.
    if (nv < 1 || 31 < nv)
        nv = 1;

    date.day = nv;
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
        StrNCopy(identifier, tokenP, 2);
        identifier[2] = nullChr;
        nv = StrAToI(identifier);

        // Validate the number and use it.
        if (nv < 0 || 24 <= nv)
            nv = 0;

        time.hours = nv;
        tokenP += strlen(identifier) * sizeof(char);

        // Read in Minutes
        StrNCopy(identifier, tokenP, 2);
        identifier[2] = nullChr;
        nv = StrAToI(identifier);

        // Validate the number and use it.
        if (nv < 0 || 59 < nv)
            nv = 1;

        time.minutes = nv;
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

            dateInUTC = true;
        }
    }

    // Give the date and/or time to the caller.
    if (dateP != NULL)
        memmove(dateP, &date, sizeof(date));

    if (timeP != NULL)
        memmove(timeP, &time, sizeof(time));

    if (noTimeEventP != NULL)
    	*noTimeEventP = noTimeEvent;

    return dateInUTC;
}


/***********************************************************************
 *
 * FUNCTION: 		GenerateDateTimeToken
 *
 * DESCRIPTION: 	Print a date and time into a given string in
 *					the vCalendar 1.0 format.
 *
 * PARAMETERS:	->	outputString:	String ptr to write the output.
 *				->	dateP:			Ptr to date to print (required.)
 *				->	timeP:			Ptr to time to print (optional.)
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PEP	3/30/00		Initial revision.
 *
 ***********************************************************************/
static void GenerateDateTimeToken(char*	outputString, DateType*	dateP, TimeType* timeP)
{
#if GENERATE_UNIVERSAL_TIME_VCALENDARS
    DateTimeType 	dateTime;
	time_t 			utcTime;
	struct tm 		ExpandedTime;

	CnvDate2DateTime(dateP, timeP, &dateTime);

	CnvDateTime2Tm(&dateTime, &ExpandedTime);

	utcTime = timetz(&ExpandedTime, gDeviceTimeZone);

	tztime(&utcTime, "UTC", &ExpandedTime);

	CnvTm2DateTime(&ExpandedTime, &dateTime);

    sprintf(outputString, "%d%02d%02dT%02d%02d00Z", dateTime.year, dateTime.month,
              dateTime.day, dateTime.hour, dateTime.minute);
#else
    if (timeP == NULL)
    {
        TraceOutput(TL(appErrorClass, "GenerateDateTimeToken: (if) %d%02d%02dT000000", firstYear + dateP->year, dateP->month, dateP->day));
        sprintf(outputString, "%d%02d%02dT000000",
                  firstYear + dateP->year, dateP->month, dateP->day);
    }
    else
    {
        TraceOutput(TL(appErrorClass, "GenerateDateTimeToken: (else) %d%02d%02dT%02d%02d00", firstYear + dateP->year, dateP->month, dateP->day, timeP->hours, timeP->minutes));
        sprintf(outputString, "%d%02d%02dT%02d%02d00",
                  firstYear + dateP->year, dateP->month, dateP->day,
                  timeP->hours, timeP->minutes);
    }
#endif
}


/***********************************************************************
 *
 * FUNCTION: 		GenerateDateTimeTokenForSeconds
 *
 * DESCRIPTION: 	Print a date and time into a given string in
 *					the vCalendar 1.0 format.
 *
 * PARAMETERS:	->	outputString:	String ptr to write the output.
 *				->	seconds:		Second count to print.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PEP	3/30/00		Initial Revision.
 *
 ***********************************************************************/
static void GenerateDateTimeTokenForSeconds(
                                            char*		outputString,
                                            uint32_t	seconds)
{
    DateTimeType dateTime;

#if GENERATE_UNIVERSAL_TIME_VCALENDARS
    DateTimeType 	dateTime;
	time_t 			utcTime;
	struct tm 		ExpandedTime;

    TimSecondsToDateTime(seconds, &dateTime);
	CnvDateTime2Tm(&dateTime, &ExpandedTime);
	utcTime = timetz(&ExpandedTime, gDeviceTimeZone);
	tztime(&utcTime, "UTC", &ExpandedTime);
	CnvTm2DateTime(&ExpandedTime, &dateTime);

    sprintf(outputString, "%d%02d%02dT%02d%02d00Z", dateTime.year, dateTime.month,
              dateTime.day, dateTime.hour, dateTime.minute);
#else
    TimSecondsToDateTime(seconds, &dateTime);
    sprintf(outputString, "%d%02d%02dT%02d%02d00", dateTime.year, dateTime.month,
              dateTime.day, dateTime.hour, dateTime.minute);
#endif
}


/***********************************************************************
 *
 * FUNCTION: 		MatchWeekDayToken
 *
 * DESCRIPTION:		Matches the given string to a week day value.
 *
 *					THE TOKEN STRING MUST BE CONVERTED TO LOWERCASE
 *					BEFORE IT IS SENT TO THIS FUNCTION.
 *
 * PARAMETERS:	->	tokenP:	String ptr from which to extract weekday.
 *
 * RETURNED: 		The week day value (sunday -> saturday) or 255
 *					if token didnt match.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	FRI	12/3/97		Initial revision.
 *
 ***********************************************************************/
static uint8_t MatchWeekDayToken(const char* tokenP)
{
    // Token must already be converted to lower-case string

    // Get 2-char token
    uint16_t weekDay = *((uint16_t*)tokenP);

	// This is endianness-dependant, on little-endian platform, we
	// must byte-swap it.
	weekDay = EndianSwap16(weekDay);

    // Find it
    switch (weekDay)
    {
    case 'su':
        return sunday;
        break;
    case 'mo':
        return monday;
        break;
    case 'tu':
        return tuesday;
        break;
    case 'we':
        return wednesday;
        break;
    case 'th':
        return thursday;
        break;
    case 'fr':
        return friday;
        break;
    case 'sa':
        return saturday;
        break;
    default:
        // Bad weekday token
        ErrNonFatalDisplay("Bad weekday");
        return 255;
        break;
    }
}


/***********************************************************************
 *
 * FUNCTION: 		MatchDayOrPositionToken
 *
 * DESCRIPTION:		Extracts a day or position value and its sign from
 *					the given token string.
 *
 * PARAMETERS:	->	tokenP:		String ptr from which to extract.
 *				->	valueP:		Ptr of where to store day/position value.
 *				->	negativeP:	True if valueP is negative.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	FRI	12/3/97		Initial revision.
 *
 ***********************************************************************/
static void MatchDayPositionToken(
  const char*	tokenP,
  uint32_t*		valueP,
  Boolean*		negativeP)
{
    // Get token length
    uint16_t len = strlen(tokenP);
    // Determine sign from last char if present
    *negativeP = (tokenP[len - 1] == '-');
    // Convert string value to integer. Any non-numeric chars
    // after the digits will be ignored
    *valueP = StrAToI(tokenP);
    // Return sign
}


/***********************************************************************
 *
 * FUNCTION: 		DateImportRepeatingRule
 *
 * DESCRIPTION: 	Import a VCal record of type vEvent
 *
 * PARAMETERS:	->	ruleTextP:pointer to the imported rule string
 *				->	repeatInfoP:pointer to the resulting RepeatInfoType
 *
 * RETURNED: 		True if import succeeded.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	DJK	8/9/97		Initial revision.
 *	ART	10/17/97	Added parameter to return unique id.
 *	ART	2/2/98		Handle yearly events better (but not great).
 *	TLW	2/9/98		Use uint16_t for yearly (monthly repeatFrequency) so
 *					its not truncated.
 *	RFL	8/4/98		Broke out of DateImportVEvent.
 *	GGL	8/31/99		Return NULL if the repeat rule is invalid.
 *	JMP	10/21/99	Eliminated "incomplete notes" warning.
 *
 ***********************************************************************/
static Boolean DateImportRepeatingRule(
	char * 			ruleTextP,
	RepeatInfoPtr	repeatInfoP,
	Boolean * 		endDateInUTCP)
{
	Boolean			importSucceeded = true;
    char * 			fieldP;
    char 			identifier[kIdentifierLengthMax];

    uint32_t 		duration;

    uint8_t			weekDay;

    uint16_t 		repeatFrequency;

    uint32_t		position;
    Boolean			fromEndOfMonth;


	// Default : end date in absolute local time
	*endDateInUTCP = false;

    // If some rule was read
    if (ruleTextP != NULL)
    {
        // Initialize all fields
        repeatInfoP->repeatType = repeatNone;

        DateToInt(repeatInfoP->repeatEndDate) = (uint16_t) (defaultRepeatEndDate);

        repeatInfoP->repeatFrequency = 0;
        repeatInfoP->repeatOn = 0;
        repeatInfoP->repeatStartOfWeek = 0;

        // Convert entire field to lower-case
        StrToLower(ruleTextP, ruleTextP);

        // Get the rule type and interval token (required)
        fieldP = GetToken(ruleTextP, identifier);

        // Determine rule type from first char
        switch (identifier[0])
        {
        case 'd':
            {
                // Daily
                repeatInfoP->repeatType = repeatDaily;

                // Convert interval string to integer
                repeatInfoP->repeatFrequency = (uint8_t) StrAToI(&identifier[1]);

                // If there's more to read (optional)
                if (fieldP != NULL)
                {
                    // Read duration or end-date
                    fieldP = GetToken(fieldP, identifier);

                    // Is literal a duration or end-date?
                    if (identifier[0] == '#')
                    {
                        // It's a duration. Extract it.
                       duration = StrAToI(&identifier[1]);
                    }
                    else
                    {
                        // It's an end-date. Read & convert to Palm OS date.
                        *endDateInUTCP = MatchDateTimeToken(identifier, &repeatInfoP->repeatEndDate, NULL, NULL);
                    }
                }
            }
            break;
        case 'w':
            {
                // Weekly
                repeatInfoP->repeatType = repeatWeekly;

                // Convert interval string to integer
                repeatInfoP->repeatFrequency = (uint8_t) StrAToI(&identifier[1]);

                // Read remaining tokens: weekdays, occurrences, duration, end date
                while (fieldP != NULL)
                {
                    // Read a token
                    fieldP = GetToken(fieldP, identifier);

                    // Determine token type
                    if (identifier[0] == '#')
                    {
                        // It's a duration. Extract it.
                        duration = StrAToI(&identifier[1]);
                    }
                    else if (TxtCharIsDigit(identifier[0]))
                    {
                        // It's an end-date. Read & convert to Palm OS date.
                        *endDateInUTCP = MatchDateTimeToken(identifier, &repeatInfoP->repeatEndDate, NULL, NULL);
                    }
                    else
                    {
                        // Try to match weekday token
                        weekDay = MatchWeekDayToken(identifier);
                        if (weekDay != 255)
                        {
                            // Set the bit for this day
                            SetBitMacro(repeatInfoP->repeatOn, weekDay);
                            // We found at least one weekday
                            //	foundWeekday = true;
                        }
                    }
                }
            }
            break;

        case 'm':
            {
                // Monthly
                // Convert interval string to integer
                repeatFrequency = (uint8_t) StrAToI(&identifier[2]);
                // Determine if monthly by day or by position
                switch (identifier[1])
                {
                case 'p':
                    {
                        // Monthly by position
                       position = 0;
                       fromEndOfMonth = false;

                        repeatInfoP->repeatType = repeatMonthlyByDay;
                        repeatInfoP->repeatFrequency = (uint8_t) repeatFrequency;

                        // Read remaining tokens: weekdays, occurrences, duration, end date
                        while (fieldP != NULL)
                        {
                            // Read a token
                            fieldP = GetToken(fieldP, identifier);

                            // Determine token type
                            if (identifier[0] == '#')
                            {
                                // It's a duration. Extract it.
                                duration = StrAToI(&identifier[1]);
                           	}
                            else if (TxtCharIsDigit(identifier[0]))
                            {
                                // It's an occurrence or an end-date. Judge by length
                                if (strlen(identifier) > 2)
                                {
                                    // It should be an end-date
                                    *endDateInUTCP = MatchDateTimeToken(identifier, &repeatInfoP->repeatEndDate, NULL, NULL);
                                }
                                else
                                {
                                    // It should be an occurrence
                                    // Extract day/position and sign
                                    MatchDayPositionToken(identifier, &position, &fromEndOfMonth);
                                    // Validate position
                                    if (position < 1)
                                        position = 1;
                                    else if (position > 5)
                                        position = 5;
                                }
                            }
                            else
                            {
                                // It should be a weekday

                                // Try to match weekday token
                                weekDay = MatchWeekDayToken(identifier);

                                if (weekDay != 255)
                                {
                                    // Calc day of week to repeat. Note that an
                                    // occurrence should already have been found
                                    if (fromEndOfMonth)
                                    {
                                        // assume the position is 1, since datebook doesn't handle
                                        // things like 2nd-to-the-last Monday...
                                        repeatInfoP->repeatOn = ((uint8_t) (domLastSun + weekDay));
                                    }
                                    else
                                    {
                                        repeatInfoP->repeatOn = ((uint8_t) (dom1stSun + ((position - 1) * daysInWeek) + weekDay));
                                    }
                                }
                            }
                        }
                    }
                    break;
                case 'd':
                    {
                        // Monthly By day or Yearly
                        //
                        // Yearly repeating events are passed a monthly-by-date repeating
                        // event with the frequency in months instead of years.  This is due
                        // to the fact that vCal's years rule uses days by number, which creates
                        // problems in leap years.
                        if (repeatFrequency % monthsInYear)
                        {
                            repeatInfoP->repeatType = repeatMonthlyByDate;
                            repeatInfoP->repeatFrequency = (uint8_t) repeatFrequency;
                        }
                        else
                        {
                            repeatInfoP->repeatType = repeatYearly;
                            repeatInfoP->repeatFrequency = ((uint8_t) repeatFrequency / monthsInYear);

                            // Has no meaning for this case
                            repeatInfoP->repeatOn = 0;
                        }

                        // Read remaining tokens: occurrences, duration, end date
                        while (fieldP != NULL)
                        {
                            // Read a token
                            fieldP = GetToken(fieldP, identifier);

                            // Determine token type
                            if (identifier[0] == '#')
                            {
                                // It's a duration. Extract it.
                                duration = StrAToI(&identifier[1]);
                            }
                            else if (TxtCharIsAlNum(identifier[0]))
                            {
                                // It's an occurrence or an end-date. Judge by length
                                if (strlen(identifier) > 3)
                                {
                                    // It should be an end-date
                                    *endDateInUTCP = MatchDateTimeToken(identifier, &repeatInfoP->repeatEndDate, NULL, NULL);
                                }
                                else
                                {
                                    // Datebook doesnt support repeating on a day which isnt the
                                    // same as the start day. Thus, occurrences are not used and
                                    // this value should be zero
                                    repeatInfoP->repeatOn = 0;
                                }
                            }
                        }
                    }
                    break;
                default:
                    // Bad monthly sub-type
                    ErrNonFatalDisplay("Bad monthly rule");
			    	importSucceeded = false;
                }
            }
            break;
        case 'y':
            {
                // Yearly
                repeatInfoP->repeatType = repeatYearly;

                // Has no meaning for this case
                repeatInfoP->repeatFrequency = (uint8_t) StrAToI(&identifier[2]);

                // Determine if yearly by day or by month
                switch (identifier[1])
                {
                case 'm':
                    {
                        // By month
                        // Read remaining tokens: months, duration, end date
                        while (fieldP != NULL)
                        {
                            // Read a token
                            fieldP = GetToken(fieldP, identifier);

                            // Determine token type
                            if (identifier[0] == '#')
                            {
                                // It's a duration. Extract it.
                                duration = StrAToI(&identifier[1]);
                            }
                            else if (TxtCharIsDigit(identifier[0]))
                            {
                                // It's a month occurrence or an end-date. Judge by length
                                if (strlen(identifier) > 2)
                                {
                                    // It should be an end-date
                                    *endDateInUTCP = MatchDateTimeToken(identifier, &repeatInfoP->repeatEndDate, NULL, NULL);
                                }
                                else
                                {
                                    // Datebook doesnt support monthly repeats on a date which isnt
                                    // the same as the start day. Thus, occurrences are not used and
                                    // this value should be zero
                                    repeatInfoP->repeatOn = 0;
                                }
                            }
                        }
                    }
                    break;
                case 'd':
                    {
                        // By day

                        // Read remaining tokens: days, duration, end date
                        while (fieldP != NULL)
                        {
                            // Read a token
                            fieldP = GetToken(fieldP, identifier);
                            // Determine token type
                            if (identifier[0] == '#')
                            {
                                // It's a duration. Extract it.
                                duration = StrAToI(&identifier[1]);
                            }
                            else if (TxtCharIsDigit(identifier[0]))
                            {
                                // It's a day occurrence or an end-date. Judge by length
                                if (strlen(identifier) > 3)
                                {
                                    // It should be an end-date
                                    *endDateInUTCP = MatchDateTimeToken(identifier, &repeatInfoP->repeatEndDate, NULL, NULL);
                                }
                                else
                                {
                                    // Datebook doesnt support daily repeats on a days which arent
                                    // the same as the start day. Thus, occurrences are not used and
                                    // this value should be zero
                                    repeatInfoP->repeatOn = 0;
                                }
                            }
                        }
                    }
                    break;
                default:
                    // Bad yearly sub-type
                    ErrNonFatalDisplay("Bad yearly rule");
			    	importSucceeded = false;
                    break;
                }
            }
            break;
        default:
            // Unknown rule
            ErrNonFatalDisplay("Bad repeat rule");
	    	importSucceeded = false;
            break;
        }
    }
    else
    	importSucceeded = false;

   	return importSucceeded;
}

/************************************************************
 *
 * FUNCTION: PrvVCardSenderType
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static VCardSenderKindType PrvVCardSenderType(char * palmOSVersion)
{
	if (palmOSVersion && strcmp(palmOSVersion, kVObjectPalmOSV6Version) >= 0)
	{
		// Import PALM OS vCard newer or equal to v6.0
		return kPalmOS6AndAfter;
	}
	else if (palmOSVersion)
	{
		// Import older PALM OS vCard
		return kPalmOS5AndBefore;
	}

	return kNonPalmOSDevice;
}

static status_t PrvDateImportUnknownData(DmOpenRef dbP, PdiReaderType* reader, uint32_t* rowIDP)
{
	status_t				err = errNone;
	uint32_t				columnID = 0xFFFFFFFF;
	char* 					tmpStr = NULL;
	DbSchemaColumnType		columnType = (DbSchemaColumnType)0xFF;
	DbSchemaColumnType		dbColumnType;
	DbSchemaColumnDefnType*	columnDefnsP = NULL;
	DbSchemaColumnValueType	colVals;

	// Get the Column ID & Column Type information
	do
	{
		switch (reader->parameter)
		{
			case kPdiPAN_X_PALM_COLID:		columnID = StrAToI(reader->parameterValue);							break;
			case kPdiPAN_X_PALM_COLTYPE:	columnType = (DbSchemaColumnType) StrAToI(reader->parameterValue);	break;
		}
	}
	while (PdiReadParameter(reader) >= errNone);

	// If parameters are missing, return.
	if ((columnID == 0xFFFFFFFF) || (columnType == (DbSchemaColumnType)0xFF))
		return errNone;

	// Check if the column exists and get its definition
	// If the column doesn't exist, return.
  	if (DbGetColumnDefinitions(dbP, DatebookApptTable, 1, &columnID, &columnDefnsP) < errNone)
		return errNone;

	dbColumnType = columnDefnsP->type;
	DbReleaseStorage(dbP, columnDefnsP);

	// Check if the column type are identical. If not, return.
	if (dbColumnType != columnType)
		return errNone;

	// Read the property value
	PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);

	memset(&colVals, 0, sizeof(DbSchemaColumnValueType));
	colVals.columnID = columnID;
	colVals.data = tmpStr;
	colVals.dataSize = reader->written;

	// The PDI library add an extra 0 for data type.
	if (((columnType != dbVarChar) && (columnType != dbChar)) || (columnType & dbVector))
		colVals.dataSize--;

	// rowID passed. Write data at that place.
	if (*rowIDP != dbInvalidRowID)
		err = DbWriteColumnValues(dbP, *rowIDP, 1, &colVals);
	else // insert the new record
		err = DbInsertRow(dbP, DatebookApptTable, 1, &colVals, rowIDP);

	if (tmpStr)
		MemPtrFree(tmpStr);

	return err;
}

/***********************************************************************
 *
 * FUNCTION: 		DateImportVEvent
 *
 * DESCRIPTION: 	Import a VCal record of type vEvent.
 *
 * PARAMETERS:
 *				->	dbP:
 *				->	reader:
 *				->	beginAlreadyRead:
 *				->	rowIDP:
 *				->	errorP:
 *
 * RETURNED: 		True if the input was read.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	DJK	8/9/97		Inital revision.
 *	ART	10/17/97	Added parameter to return unique id.
 *	ART	2/2/98		Handle yearly events better (but not great).
 *	TLW	2/9/98		Use uint16_t for yearly (monthly repeatFrequency)
 *					so its not truncated.
 *	PES	3/29/00		Deal with events whose start and end times fall on
 *					different days, as occurs when events expressed in
 *					universal time are shifted.
 *	PPL	07/17/03	cursors !
 *
 ***********************************************************************/
static Boolean DateImportVEvent(
	DmOpenRef 			dbP,
	PdiReaderType* 		reader,
	uint32_t* 			rowIDP,
	VCardSenderKindType vCardSender,
	status_t* 			errorP)
{
    ApptDBRecordType 	newDateRecord;
    char * 				newAttachment = NULL;
    uint32_t 			alarmDTinSec = 0; // ASSUMPTION: Alarm is not set for 1/1/1904 at 00:00:00
    char *				tempP = NULL;
    status_t 			error = errNone;
    Boolean 			datedEvent = false;	// Aba: true if a date was found in the event
    char				localTimezone[TZNAME_MAX];
    Boolean 			beginEndDateInUTC = false; // The begin & end event dates have been read in UTC ?
    Boolean 			endRepeatDateInUTC = false; // The repeat end event date has been read in UTC ?
    Boolean 			exceptionDatesInUTC = false; // The exception dates have been read in UTC ?
	CategoryID			categoryToAssign = catIDUnfiled;

	DateTimeType		alarmDT;
	DateType			alarmDate;
	TimeType			alarmTime;

	DateType 			endDate = {0,0};

	MemHandle			exceptionListH;
	DateType*			exceptionListP;
    DateType*			exceptionP;
    uint32_t			exceptionCount;
    status_t			err;

    char* 				localTimezoneP;
    char* 				apptTimezoneP;

    DateTimeType 		tmpDateTime;

	uint32_t 			i;

	*rowIDP = dbInvalidRowID;

    PdiEnterObject( reader);

    // Initialize the record to NULL
    memset(&newDateRecord, 0x00, sizeof(ApptDBRecordType));
    newDateRecord.alarm.advance = apptNoAlarm;
	localTimezone[0] = chrNull;

    while (PdiReadPropertyName( reader) == 0 && reader->property != kPdiPRN_END_VEVENT)
    {
		if (reader->property == kPdiPRN_X_PALM_UKNCOL)
		{
			if ((error = PrvDateImportUnknownData(dbP, reader, rowIDP)) < errNone)
				goto Exit;
			else continue;
		}

		while (PdiReadParameter(reader) >= errNone)
			;

		switch(reader->property)
		{
			case kPdiPRN_DTSTART:
	            // Handle Start tag
	            // ASSUMPTION: we will use the date from the start time, not the end time
	            // NOTE: This function will break if the DTSTART value is truncated
	            // beyond the day
                PdiReadPropertyField(reader, &tempP, kPdiResizableBuffer, kPdiDefaultFields);
                if (tempP != NULL)
                {
                    // Extract start date & time
                    beginEndDateInUTC = beginEndDateInUTC ||
                    					MatchDateTimeToken(
                    									tempP,
                    									&newDateRecord.when.date,
                        								&newDateRecord.when.startTime,
                        								&newDateRecord.when.noTimeEvent);

                    // Assume end time same as start time for now...
                    newDateRecord.when.endTime = newDateRecord.when.startTime;
	                datedEvent = true;
                }
                break;

			case kPdiPRN_DTEND:
	            // Read in the end time
	            // ASSUMPTION: It is possible to send an end date that is different from
	            // the start date, this would cause a problem for us, since we
	            // assume that the end time has the same date as the start time. This can
	            // happen even if the event starts and ends on the same day if the sending
	            // device uses universal time format and is in a different time zone than
	            // the receiving device.
                PdiReadPropertyField( reader, &tempP, kPdiResizableBuffer, kPdiDefaultFields);
                if (tempP != NULL)
                {
                    // Extract end time : evaluate endDate first, otherwise MatchDateTimeToken()
					// may never be avaluated.
                    beginEndDateInUTC = (MatchDateTimeToken(
                    									tempP,
                    									&endDate,
                    									&newDateRecord.when.endTime,
                        								&newDateRecord.when.noTimeEvent)
                        				|| beginEndDateInUTC);

                    if ((DateToInt(endDate) != DateToInt(newDateRecord.when.date))
	                && (!newDateRecord.when.noTimeEvent))
                    {
                    	if (DateToDays(newDateRecord.when.date) == (DateToDays(endDate)-1))
                    	{
	                    	newDateRecord.when.midnightWrapped = true;
                    	}
                    	else
                		{
#ifndef PIM_APPS_PROFILING
	                        TraceOutput(TL(appErrorClass, "Importing an event overlapping on more than 1 day"));
#endif
	                    	newDateRecord.when.midnightWrapped = false;
                            newDateRecord.when.endTime.hours = hoursPerDay - 1;
                            newDateRecord.when.endTime.minutes = hoursInMinutes - 5;
                		}
                    }
                    else
                    {
                    	newDateRecord.when.midnightWrapped = false;
                    }
                }
	            break;

            // Read Repeat info
            case kPdiPRN_RRULE:
                PdiReadPropertyField( reader, &tempP, kPdiResizableBuffer, kPdiDefaultFields);

                if (! DateImportRepeatingRule(tempP, &newDateRecord.repeat, &endRepeatDateInUTC))
					newDateRecord.repeat.repeatType = repeatNone;

				// Handle the case where no days of week is set for weekly event.
				// Assume the starting day of the event is the repeating day
				if ((newDateRecord.repeat.repeatType == repeatWeekly)
				&& (newDateRecord.repeat.repeatOn == 0))
				{
					// Assume that start date is known and set it as the repeat day
					SetBitMacro(newDateRecord.repeat.repeatOn,
						DayOfWeek(newDateRecord.when.date.month, newDateRecord.when.date.day, (int16_t)(newDateRecord.when.date.year + firstYear)));
				}
				break;

            // Read Repeat exceptions
			case kPdiPRN_EXDATE:
				exceptionCount = 0;

                // Allocate exception list handle to hold at least the first exception
                exceptionListH = MemHandleNew(sizeof(DateType));
                // Debug check
                ErrFatalDisplayIf(exceptionListH == NULL, "Memory full");

                // Read each exception
                while (PdiReadPropertyField(reader, &tempP, kPdiResizableBuffer, kPdiSemicolonFields) == 0)
                {
                    // Resize handle to hold exception
                    err = MemHandleResize(exceptionListH, (uint32_t)(sizeof(DateType) * (exceptionCount + 1)));
                    ErrFatalDisplayIf(err != 0, "Memory full");

                    // Lock exception handle
                    exceptionListP = MemHandleLock(exceptionListH);
                   	exceptionP = &(exceptionListP[exceptionCount]);

                    // Store exception into exception handle
                    exceptionDatesInUTC = exceptionDatesInUTC
                    					|| MatchDateTimeToken(tempP, exceptionP, NULL, NULL);

                    // Increase exception count
                    exceptionCount++;
                    // Unlock exceptions list handle
                    MemHandleUnlock(exceptionListH);
                }

                // Lock exception handle
                exceptionListP = MemHandleLock(exceptionListH);

                // Store final exception count
                newDateRecord.exceptions.numExceptions = exceptionCount;

                // Save exception list into datebook record
                newDateRecord.exceptions.exceptionsDatesP = MemPtrNew(sizeof(DateType) * exceptionCount);
                memmove(newDateRecord.exceptions.exceptionsDatesP, exceptionListP, sizeof(DateType) * exceptionCount);
                MemHandleUnlock(exceptionListH);
                MemHandleFree(exceptionListH);
            	break;

            // 	Read in Alarm info
            case kPdiPRN_AALARM:
            case kPdiPRN_DALARM:
                PdiReadPropertyField(reader, &tempP, kPdiResizableBuffer, kPdiDefaultFields);

                if (tempP != NULL)
                {
                    // Extract alarm date & time
                    MatchDateTimeToken(tempP, &alarmDate, &alarmTime, NULL);
                    // Copy values to DateTimeType struct
                    CnvDate2DateTime(&alarmDate, &alarmTime, &alarmDT);
                    alarmDTinSec = TimDateTimeToSeconds(&alarmDT);
                }
	            break;

            // Read in Summary
            case kPdiPRN_SUMMARY:
                PdiDefineResizing(reader, kPdiDefaultBufferDeltaSize, noteViewMaxLength);
                PdiReadPropertyField(reader, (char**) &newDateRecord.description, kPdiResizableBuffer, kPdiDefaultFields);
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, kPdiDefaultBufferMaxSize);
                break;

            // Read in Description
            case kPdiPRN_DESCRIPTION:
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, noteViewMaxLength);
                PdiReadPropertyField(reader, (char**) &newDateRecord.note, kPdiResizableBuffer, kPdiDefaultFields);
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, kPdiDefaultBufferMaxSize);
                break;

			// Read the location
            case kPdiPRN_LOCATION:
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, tableMaxTextItemSize);
                PdiReadPropertyField(reader, (char**) &newDateRecord.location, kPdiResizableBuffer, kPdiDefaultFields);
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, kPdiDefaultBufferMaxSize);
                break;

            // Read in attachments.  At the end we place the attachment into the record.
			case kPdiPRN_ATTACH:
                // Note: vCal permits attachments of types other than text, specifically
                // URLs and Content ID's.  At the moment, wee will just treat both of these
                // as text strings
                PdiDefineResizing(reader, kPdiDefaultBufferDeltaSize, noteViewMaxLength);
                PdiReadPropertyField( reader, (char**) &newAttachment, kPdiResizableBuffer, kPdiDefaultFields);
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, kPdiDefaultBufferMaxSize);
                break;

            // read in the unique identifier
            case kPdiPRN_UID:
				// Removed code that was reading the unique ID : There is no meaning in loading
				// the unique ID since it can only be assigned by the data manager.
	            break;

            // read in the timezone information : local (device) timezone
            case kPdiPRN_X_PALM_LOCAL_TZ:
	           localTimezoneP = localTimezone;
               PdiReadPropertyField(reader, (char**) &localTimezoneP, TZNAME_MAX, kPdiDefaultFields);
            	break;

            // read in the timezone information : appointment timezone
            case kPdiPRN_X_PALM_APPT_TZ:
	            apptTimezoneP = newDateRecord.when.timeZoneName;
	            PdiReadPropertyField(reader, (char**) &apptTimezoneP, TZNAME_MAX, kPdiDefaultFields);
	            break;
        }
    }

    // If we go out the loop and last property is not an END:VEVENT => bad data
    if (reader->property != kPdiPRN_END_VEVENT)
	{
		error = exgErrBadData;
		goto Exit;
	}

	// If no date: no syntax error but data can't be accepted
	if (! datedEvent)
	{
		error = errNone;
		goto Exit;
	}

    // if an alarm was read in, translate it appropriately
    // ASSUMPTION: alarm is before start time
    if (alarmDTinSec)
        TranslateAlarm((ApptDBRecordType*)&newDateRecord, alarmDTinSec);

    // PalmIII stored descriptions as DESCRIPTION and notes as ATTACH.
    // vCal spec stores considers them to be SUMMARY and DESCRIPTION
    // PalmOS 6.x fullfills vCal specs but here we handle both for older devices
    // or other brand devices.
    if ((vCardSender != kPalmOS6AndAfter) && (newDateRecord.description == NULL))
    {
        newDateRecord.description = newDateRecord.note;
        newDateRecord.note = newAttachment;
        newAttachment = NULL;
    }

    // Some vCal implementations send duplicate SUMMARY and DESCRIPTION fields.
    if (newDateRecord.description != NULL && newDateRecord.note != NULL &&
        StrCompare(newDateRecord.description, newDateRecord.note) == 0)
    {
        // Delete the duplicate note.
        MemPtrFree(newDateRecord.note);
        newDateRecord.note = newAttachment;
        newAttachment = NULL;
    }

	if ((localTimezone[0] != chrNull) && StrNCompare(gDeviceTimeZone, localTimezone, TZNAME_MAX)
		&& (!newDateRecord.when.noTimeEvent))
	{
		ApptDateTimeType 	initialEvent = newDateRecord.when;
		DateType 			initialDate = initialEvent.date;
		uint8_t				UTCRepeatOnValue;

		if (! beginEndDateInUTC)
		{
			// The begin & end event dates have NOT been read in UTC : translate them
			// First, begin date
			CnvDate2DateTime(&initialDate, &newDateRecord.when.startTime, &tmpDateTime);
			CnvApptTZDateTime2LocalTime(&tmpDateTime, localTimezone, &tmpDateTime);

			newDateRecord.when.date = CnvDateTime2Date(&tmpDateTime);
			newDateRecord.when.startTime = CnvDateTime2Time(&tmpDateTime);

			// Then, end date
			if (newDateRecord.when.midnightWrapped)
				DateAdjust(&initialDate, +1);

			CnvDate2DateTime(&initialDate, &newDateRecord.when.endTime, &tmpDateTime);
			CnvApptTZDateTime2LocalTime(&tmpDateTime, localTimezone, &tmpDateTime);

			newDateRecord.when.endTime = CnvDateTime2Time(&tmpDateTime);
			endDate = CnvDateTime2Date(&tmpDateTime);

			// Adjust the overlapping state
			if (DateToInt(endDate) != DateToInt(newDateRecord.when.date))
			{
				ErrNonFatalDisplayIf(DateToDays(newDateRecord.when.date) != DateToDays(endDate)-1,
					"Wrong end date in vCal import.");

				newDateRecord.when.midnightWrapped = true;
			}
			else
				newDateRecord.when.midnightWrapped = false;
		}

		if ((!endRepeatDateInUTC) && (newDateRecord.repeat.repeatType != repeatNone))
		{
			// The repeatOn value must be updated
			UTCRepeatOnValue = ComputeRepeatOnForUTC(newDateRecord.repeat.repeatOn, localTimezone,
				newDateRecord.repeat.repeatType, &initialEvent);

			newDateRecord.repeat.repeatOn = ComputeRepeatOnForTimeZone(UTCRepeatOnValue, gDeviceTimeZone,
				newDateRecord.repeat.repeatType, &(newDateRecord.when));
		}

		if ((!endRepeatDateInUTC) && (DateToInt(newDateRecord.repeat.repeatEndDate) != apptNoEndDate))
		{
			// The repeat end event date has NOT been read in UTC and is not infinite: translate it.
			CnvDate2DateTime(&newDateRecord.repeat.repeatEndDate, &newDateRecord.when.startTime, &tmpDateTime);
			CnvApptTZDateTime2LocalTime(&tmpDateTime, localTimezone, &tmpDateTime);

			newDateRecord.repeat.repeatEndDate = CnvDateTime2Date(&tmpDateTime);
		}

		if (! exceptionDatesInUTC)
		{
			exceptionListP = newDateRecord.exceptions.exceptionsDatesP;

			// The exception dates have NOT been read in UTC : translate them.
			for (i = 0; i < newDateRecord.exceptions.numExceptions; i++)
			{
				CnvDate2DateTime(&(exceptionListP[i]), &newDateRecord.when.startTime, &tmpDateTime);
				CnvApptTZDateTime2LocalTime(&tmpDateTime, localTimezone, &tmpDateTime);

				exceptionListP[i] = CnvDateTime2Date(&tmpDateTime);
			}
		}
	}

#ifndef PIM_APPS_PROFILING
    TraceOutput(TL(appErrorClass, "Sender Local TZ = %s", localTimezone));
    TraceOutput(TL(appErrorClass, "Initial sender Appt TZ = %s", newDateRecord.when.timeZoneName));
    TraceOutput(TL(appErrorClass, "Device TZ = %s", gDeviceTimeZone));
#endif

	// Assign the appointment timezone
	if ((!newDateRecord.when.noTimeEvent) && (newDateRecord.when.timeZoneName[0] == chrNull))
	{
		if (localTimezone[0] != chrNull)
			// If a PalmOS6-compatible device did not sent appointment timezone, this is because
			// it the same as the sending local device timezone
			StrNCopy(newDateRecord.when.timeZoneName, localTimezone, TZNAME_MAX);
		else
			// Non PalmOS6-compatible device: set appointment timezone to device TZ
			StrNCopy(newDateRecord.when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);
	}

#ifndef PIM_APPS_PROFILING
    TraceOutput(TL(appErrorClass, "Fixed sender Appt TZ = %s", newDateRecord.when.timeZoneName));
#endif

	// If the description is still not assigned, assign it an empty string
	if (newDateRecord.description == NULL)
	{
		newDateRecord.description = MemPtrNew(1);
		newDateRecord.description[0] = chrNull;
	}

	if (*rowIDP == dbInvalidRowID)
	{
 		   TraceOutput(TL(appErrorClass + 5, "Create new record"));
		// Create this record now...
	    if (ApptNewRecord(dbP, (ApptDBRecordType*)&newDateRecord, rowIDP) < errNone)
	    {
	        error = exgMemError;
	        goto Exit;
	    }
	}
	else
	{
 		   TraceOutput(TL(appErrorClass + 5, "Update new record"));
		// Update the record
		if (ApptChangeRecord (dbP, *rowIDP, (ApptDBRecordType*)&newDateRecord, DBK_SELECT_ALL_FIELDS) < errNone)
	    {
	        error = exgMemError;
	        goto Exit;
	    }
	}

	// Check if appData field exists (may not exist when the vCal is imported, not beamed)
	if (reader->appData != NULL)
 		categoryToAssign = ((ExgSocketType*)(reader->appData))->appData;

	// Assign the category set by the user in accept dialog
	if (categoryToAssign != catIDUnfiled)
		DbSetCategory(dbP, *rowIDP, 1, &categoryToAssign);

Exit:
    // Free any temporary buffers used to store the incoming data.
    if (newAttachment)
        MemPtrFree(newAttachment);

	if (tempP)
        MemPtrFree(tempP);

    ApptFreeRecordMemory(&newDateRecord);

    *errorP = error;

    return true;
}

static void PrvDateExportUnknownCol(PdiWriterType* writer, DmOpenRef dbP, int32_t rowID)
{
	uint32_t					i, j;
	uint32_t					numColumns;
	uint32_t					numNewColumns = 0;
	uint32_t*					newColumnIDArray = NULL;
	DbSchemaColumnType*			newColumnTypeArray = NULL;
	DbSchemaColumnValueType*	columnValuesP = NULL;
	DbSchemaColumnDefnType*		columnDefnsP = NULL;
	char						tmpBuf[24]; // for numbers
	status_t					error = errNone;

	if (DbGetAllColumnDefinitions(dbP, DatebookApptTable, &numColumns, &columnDefnsP) < errNone)
		return;

	if ((newColumnIDArray = MemPtrNew(numColumns * sizeof(uint32_t))) == NULL)
		goto Exit;

	if ((newColumnTypeArray = MemPtrNew(numColumns * sizeof(DbSchemaColumnType))) == NULL)
		goto Exit;

	memset(newColumnIDArray, 0, numColumns * sizeof(uint32_t));
	memset(newColumnTypeArray, 0, numColumns * sizeof(DbSchemaColumnType));

	for (i = 0; i < numColumns; i++)
	{
		switch (columnDefnsP[i].id)
		{
			case REPEATING_EVENT_COL_ID:
			case UNTIMED_EVENT_COL_ID:
			case START_EVENT_DATE_TIME_COL_ID:
			case END_EVENT_DATE_TIME_COL_ID:
			case EVENT_TIMEZONE_COL_ID:
			case EVENT_ALARM_ADVANCE_COL_ID:
			case EVENT_ALARM_ADVANCE_UNIT_COL_ID:
			case REPEAT_EVENT_TYPE_COL_ID:
			case REPEAT_EVENT_END_DATE_COL_ID:
			case REPEAT_EVENT_REPEAT_FREQ_COL_ID:
			case REPEAT_EVENT_REPEAT_ON_ID:
			case REPEAT_EVENT_START_OF_WEEK_ID:
			case EVENT_EXCEPTIONS_DATES_ID:
			case EVENT_DESCRIPTION_ID:
			case EVENT_LOCATION_ID:
			case EVENT_NOTE_ID:
					continue;

			default:
				newColumnIDArray[numNewColumns] = columnDefnsP[i].id;
				newColumnTypeArray[numNewColumns] = columnDefnsP[i].type;
				numNewColumns++;
				break;
		}
	}

	if (!numNewColumns)
		goto Exit;

	error = DbGetColumnValues(dbP, rowID, numNewColumns, newColumnIDArray, &columnValuesP);

	if ((error < errNone) && (error != dmErrOneOrMoreFailed))
		goto Exit;

	// Neested loop. We can't rely on the column order, return by the DbGetColumnValues
	// We have 2 solutions.
	// - Neested loop:
	//		All is already loaded into memory. Decrease the number of call to the Database Manager (IPC)
	//		The number of extra columns should not be very high => The loop should be fast.
	// - Single loop:
	//		For each extra column, a call to get its data or info.
	//		Faster than the previous solution about the extra data parsing
	//		Slower than the previous version because we need to do a extra data manager call by column
	//
	// So I choose the neested loop solution.
	for (i = 0; i < numNewColumns; i++)
	{
		// Skip column with error.
		if (columnValuesP[i].errCode != errNone)
			continue;

		for (j = 0; j < numNewColumns; j++)
		{
			if (columnValuesP[i].columnID == newColumnIDArray[j])
			{
				// Send the data, depending of the column type
				PdiWriteProperty(writer, kPdiPRN_X_PALM_UKNCOL);

				// Send unknown column with the unknown tag
				StrIToA(tmpBuf, newColumnIDArray[j]);
				PdiWriteParameterStr(writer , "X-PALM-COLID", tmpBuf);
				StrIToA(tmpBuf, (int32_t)newColumnTypeArray[j]);
				PdiWriteParameterStr(writer , "X-PALM-COLTYPE", tmpBuf);

				if (((newColumnTypeArray[j] == dbVarChar) || (newColumnTypeArray[j] == dbChar)) && !(newColumnTypeArray[j] & dbVector))
					PdiWritePropertyValue(writer, (char*) columnValuesP[i].data, kPdiWriteText);
				else
				{
					uint16_t	oldEncoding = writer->encoding;

					PdiSetEncoding(writer, kPdiB64Encoding);
					PdiWritePropertyBinaryValue(writer, columnValuesP[i].data, (uint16_t)columnValuesP[i].dataSize, kPdiWriteData);
					PdiSetEncoding(writer, oldEncoding);
				}
			}
		}
	}

Exit:

	if (columnDefnsP)
		DbReleaseStorage(dbP, columnDefnsP);

	if (columnValuesP)
		DbReleaseStorage(dbP, columnValuesP);

	if (newColumnIDArray)
		MemPtrFree(newColumnIDArray);

	if (newColumnTypeArray)
		MemPtrFree(newColumnTypeArray);
}


/***********************************************************************
 *
 * FUNCTION: 		DateExportVCal
 *
 * DESCRIPTION: 	Export a VCALENDAR record.
 *
 * PARAMETERS:
 *				->	dbP:			Pointer to the database to export
 *									the records from.
 *				->	index:			The record rowID or CursorID Current
 *									item to export.
 *				->	recordP:		Whether the begin statement has been
 *									read.
 *				->	outputStream:	Pointer to where to export the record to.
 *				->	outputFunc:		Function to send output to the stream.
 *				->	writeUniqueIDs True to write the record's unique id.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	djk	8/9/97		Initial revision.
 *	CS	05/16/00	Use DayOfMonthType instead of DayOfWeekType.
 *
 ***********************************************************************/
void DateExportVCal(
	DmOpenRef 		dbP,
	int32_t	 		rowID,
	ApptDBRecordPtr recordP,
	PdiWriterType* 	writer)
{
    char 			tempString[kRRULEStringLengthMax];
	DateType		endDate;
    int 			dayIndex;
    char* 			writePtr;
    char* 			prepareString;
    DatePtr 		exArrayP;
    char** 			fields;
    uint16_t 		i;

    PdiWriteBeginObject( writer, kPdiPRN_BEGIN_VEVENT);

    // Handle When
    // ASSUMPTION: To represent non-timed events, we will use a
    // DTSTART propert with a date value (instead of a date/time value)
    // and _no_ DTEND proprty.  This is permitted by ISO 8601.

    PdiWriteProperty( writer, kPdiPRN_DTSTART);

    if (! recordP->when.noTimeEvent)
    {
        GenerateDateTimeToken(tempString, &recordP->when.date, &recordP->when.startTime);
		PdiWritePropertyValue( writer, tempString, kPdiWriteData);

        // End Time/Date
        endDate = recordP->when.date;

        if (recordP->when.midnightWrapped)
			DateAdjust(&endDate, +1);

        PdiWriteProperty( writer, kPdiPRN_DTEND);
        GenerateDateTimeToken(tempString, &endDate, &recordP->when.endTime);
    }
    else
        // Handle a non-timed event -- see note above for convention
    {
        sprintf(tempString, "%d%02d%02d", firstYear + recordP->when.date.year,
                  recordP->when.date.month, recordP->when.date.day);
    }

	PdiWritePropertyValue( writer, tempString, kPdiWriteData);

    // Handle Alarm
    if (recordP->alarm.advance != apptNoAlarm)
	{
	    PdiWriteProperty( writer, kPdiPRN_AALARM);
        GenerateDateTimeTokenForSeconds(tempString, ApptGetAlarmTimeVCalForm(recordP));
		PdiWritePropertyValue( writer, tempString, kPdiWriteData);
    }

    // Handle Repeating Events
    if (recordP->repeat.repeatType != repeatNone)
    {
    	writePtr = tempString;
    	prepareString = NULL;

	    PdiWriteProperty( writer, kPdiPRN_RRULE);

        // Set the rule type
        switch(recordP->repeat.repeatType)
        {
	        case repeatDaily:
	        	prepareString = (char*) "D";
	            break;

	        case repeatWeekly:
	        	prepareString = (char*) "W";
	            break;

	            // One small oddity is that our repeatMonthlyByDay is equivelent
	            // to vCal's by-postion and our repeatMonthlyByDate is equivelent
	            // to vCal's by-Day rule
	        case repeatMonthlyByDay:
	        	prepareString = (char*) "MP";
	            break;

	        case repeatMonthlyByDate:
	        case repeatYearly:
	            // vCal's years rule uses days by number, which creates problems in leap years
	            // so instead we will use the month by-Day rule (equiv to our month by date rule)
	            // and multiply by monthsInYear
	        	prepareString = (char*) "MD";
	            break;
        }

        // Set the freqency
        if (recordP->repeat.repeatType == repeatYearly)
            writePtr += sprintf(writePtr, "%s%d ", prepareString, monthsInYear * (uint16_t) recordP->repeat.repeatFrequency);
        else
            writePtr += sprintf(writePtr, "%s%d ", prepareString, (uint16_t) recordP->repeat.repeatFrequency);

        // if the repeat type is repeatWeekly, emit which days the event is on
        if (recordP->repeat.repeatType == repeatWeekly)
        {
            for (dayIndex = 0; dayIndex < daysInWeek; dayIndex++)
            {
                if (GetBitMacro(recordP->repeat.repeatOn, dayIndex))
                {
                    switch(dayIndex)
                    {
                    case sunday:
                    	prepareString = (char*) "SU";
                        break;
                    case monday:
                    	prepareString = (char*) "MO";
                        break;
                    case tuesday:
                    	prepareString = (char*) "TU";
                        break;
                    case wednesday:
                    	prepareString = (char*) "WE";
                        break;
                    case thursday:
                    	prepareString = (char*) "TH";
                        break;
                    case friday:
                    	prepareString = (char*) "FR";
                        break;
                    case saturday:
                    	prepareString = (char*) "SA";
                        break;
                    }
                    writePtr += sprintf(writePtr, "%s ", prepareString);
                }
            }
        }

        // If the repeat type is a pilot monthly repeat by day (as opposed to by date),
        // emit the repetition rule
        if (recordP->repeat.repeatType == repeatMonthlyByDay)
        {
            // Deal with the case that are not the domLast___ cases
            if (((DayOfMonthType) recordP->repeat.repeatOn) < domLastSun)
            {
                // Figure out which week were in and emit it
                writePtr += sprintf(writePtr, "%d+ ", (recordP->repeat.repeatOn / daysInWeek) + 1);
            }
            else
            {
                // domLast___ are all in week -1
                writePtr += sprintf(writePtr, "1- ");
            }

            //	Figure out what the day of the week is and emit it
            if ((recordP->repeat.repeatOn == dom1stSun) || (recordP->repeat.repeatOn == dom2ndSun) ||
                (recordP->repeat.repeatOn == dom3rdSun) || (recordP->repeat.repeatOn == dom4thSun) ||
                (recordP->repeat.repeatOn == domLastSun))
                prepareString = (char*) "SU ";

            if ((recordP->repeat.repeatOn == dom1stMon) || (recordP->repeat.repeatOn == dom2ndMon) ||
                (recordP->repeat.repeatOn == dom3rdMon) || (recordP->repeat.repeatOn == dom4thMon) ||
                (recordP->repeat.repeatOn == domLastMon))
                prepareString = (char*) "MO ";

            if ((recordP->repeat.repeatOn == dom1stTue) || (recordP->repeat.repeatOn == dom2ndTue) ||
                (recordP->repeat.repeatOn == dom3rdTue) || (recordP->repeat.repeatOn == dom4thTue) ||
                (recordP->repeat.repeatOn == domLastTue))
                prepareString = (char*) "TU ";

            if ((recordP->repeat.repeatOn == dom1stWen) || (recordP->repeat.repeatOn == dom2ndWen) ||
                (recordP->repeat.repeatOn == dom3rdWen) || (recordP->repeat.repeatOn == dom4thWen) ||
                (recordP->repeat.repeatOn == domLastWen))
                prepareString = (char*) "WE ";

            if ((recordP->repeat.repeatOn == dom1stThu) || (recordP->repeat.repeatOn == dom2ndThu) ||
                (recordP->repeat.repeatOn == dom3rdThu) || (recordP->repeat.repeatOn == dom4thThu) ||
                (recordP->repeat.repeatOn == domLastThu))
                prepareString = (char*) "TH ";

            if ((recordP->repeat.repeatOn == dom1stFri) || (recordP->repeat.repeatOn == dom2ndFri) ||
                (recordP->repeat.repeatOn == dom3rdFri) || (recordP->repeat.repeatOn == dom4thFri) ||
                (recordP->repeat.repeatOn == domLastFri))
                prepareString = (char*) "FR ";

            if ((recordP->repeat.repeatOn == dom1stSat) || (recordP->repeat.repeatOn == dom2ndSat) ||
                (recordP->repeat.repeatOn == dom3rdSat) || (recordP->repeat.repeatOn == dom4thSat) ||
                (recordP->repeat.repeatOn == domLastSat))
                prepareString = (char*) "SA ";

            writePtr += sprintf(writePtr, prepareString);
        }

        // If the record is repeatMonthlyByDate, put out the # of the day
        if (recordP->repeat.repeatType == repeatMonthlyByDate)
        {
            writePtr += sprintf(writePtr, "%d ", recordP->when.date.day);
        }

        // Emit the end date
        if (DateToInt(recordP->repeat.repeatEndDate) == apptNoEndDate)
        {
	        writePtr += sprintf(writePtr, "#0");
        }
        else
        {
            // NOTE: The vCalendar 1.0 specification says that the repeat end date is a date/time
            // between the start time of the last occurrence and the start time of the next occurrence
            // (inclusing of start, exclusive of end). Previous versions of this code, as found in Palm
            // OS 3.0 to 3.5, emitted the start of the day specified as the repeat end date. That did
            // not conform to the specification. Since the old versions of this code ignored the time
            // specified, we can correct this while maintaining compatibility. We now emit the date and
            // time of the start of the last occurrence.
            char dateTimeString[kTempStringLengthMax];
            GenerateDateTimeToken(dateTimeString, &recordP->repeat.repeatEndDate, &recordP->when.startTime);
	        writePtr += sprintf(writePtr, dateTimeString);
		}
        PdiWritePropertyValue( writer, tempString, kPdiWriteData);
    }


    // Handle exceptions to repeating
    if (recordP->exceptions.numExceptions > 0)
    {
        exArrayP = recordP->exceptions.exceptionsDatesP;

	    PdiWriteProperty( writer, kPdiPRN_EXDATE);
	    fields = MemPtrNew(sizeof(char*) * recordP->exceptions.numExceptions);

        for (i = 0; i < recordP->exceptions.numExceptions; i++)
        {
            // ASSUMPTION:  EXDATE has a date/time property, although the ISO 8601 standard allows
            // us to truncate this to a date (which is what we keep), we will make the reasonable
            // assumption that the time of an exception is the same as the time of the event it is
            // an exception for.  This does not affect communication with another pilot, but will
            // require less robustness for other devices we communicate with
            fields[i] = MemPtrNew(kTempStringLengthMax);
            GenerateDateTimeToken(fields[i], &(exArrayP[i]), &recordP->when.startTime);
        }

        PdiWritePropertyFields( writer, fields, (uint16_t)(recordP->exceptions.numExceptions), kPdiWriteData);
        for (i = 0; i < recordP->exceptions.numExceptions; i++)
        {
        	MemPtrFree(fields[i]);
        }
        MemPtrFree(fields);
    }


    // Handle description, store it as summary (Outlook compliant)
    if (recordP->description != NULL)
    {
	    PdiWriteProperty( writer, kPdiPRN_SUMMARY);
        PdiWritePropertyValue(writer, recordP->description, kPdiWriteText);
    }

    // Handle note, store it as description (Outlook compliant)
    if (recordP->note != NULL)
    {
		PdiWriteProperty( writer, kPdiPRN_DESCRIPTION);
        PdiWritePropertyValue(writer, recordP->note, kPdiWriteText);
    }

	// Handle location
    if (recordP->location != NULL)
    {
	    PdiWriteProperty( writer, kPdiPRN_LOCATION);
        PdiWritePropertyValue(writer, recordP->location, kPdiWriteText);
    }

	// Handle local (device) timezone
	if (TimezoneExistsInDatabase(gDeviceTimeZone))
	{
		PdiWriteProperty( writer, kPdiPRN_X_PALM_LOCAL_TZ);
    	PdiWritePropertyValue(writer, gDeviceTimeZone, kPdiWriteText);

		// Handle appointment timezone : set this property only if this is an
		// untimed appointment or if it is different from local timezone.
	    if ((! recordP->when.noTimeEvent) && (recordP->when.timeZoneName[0] != chrNull) &&
	    	(StrNCompare(gDeviceTimeZone, recordP->when.timeZoneName, TZNAME_MAX)))
	    {
			PdiWriteProperty( writer, kPdiPRN_X_PALM_APPT_TZ);
	        PdiWritePropertyValue(writer, recordP->when.timeZoneName, kPdiWriteText);
	    }
	}

	PrvDateExportUnknownCol(writer, dbP, rowID);

    PdiWriteEndObject(writer, kPdiPRN_END_VEVENT);
}


/***********************************************************************
 *
 * FUNCTION:		DateSetGoToParams
 *
 * DESCRIPTION:		Store the information necessary to navigate to
 *					the record inserted into the launch code's parameter
 *					block.
 *
 * PARAMETERS:	->	dbP:		Pointer to the database to add the record to
 *				->	exgSocketP:	Parameter block passed with the launch code
 *				->	rowID:		Unique id of the record inserted
 *								(not a cursor ID.)
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	10/17/97	Initial revision.
 *
 ***********************************************************************/
static void DateSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, uint32_t rowID)
{
    DatabaseID 		databaseID;
    status_t		err;
	CategoryID 		dbSelectAllCategoriesID[] = {catIDAll};
	Boolean			recordIsInCategory = false;

	// Make sure we have a valid rowID
	ErrNonFatalDisplayIf((!DbIsRowID(rowID)) || (rowID == dbInvalidRowID), "Setting GoTo params: Invalid rowID.");

	databaseID =  DmFindDatabase(DatebookDbName, sysFileCDatebook, dmFindSchemaDB, NULL);

    // The this the the first record inserted, save the information
    // necessary to navigate to the record.
    if (!exgSocketP->goToParams.uniqueID)
    {
		err = DbIsRowInCategory(dbP, rowID, 1, dbSelectAllCategoriesID, DbMatchAny, &recordIsInCategory);
		if ((err < errNone) || (!recordIsInCategory))
			goto ExitOnFailure;

		exgSocketP->goToCreator = sysFileCDatebook;
		exgSocketP->goToParams.uniqueID = rowID;
		exgSocketP->goToParams.dbID = databaseID;
		// recordNum is discarded when we receive the GoTo event.
		exgSocketP->goToParams.recordNum = dmInvalidRecIndex;
    }

	return;

ExitOnFailure:
	exgSocketP->goToCreator = 0;
	exgSocketP->goToParams.uniqueID = dbInvalidRowID;
	exgSocketP->goToParams.dbID = 0;
	exgSocketP->goToParams.recordNum = 0;
}


/***********************************************************************
 *
 * FUNCTION: 		DateImportVCal
 *
 * DESCRIPTION: 	Import a VCal record of type vEvent and vToDo.
 *
 * 					The Datebook handles vCalendar records.
 *					Any vToDo records are sent to the ToDo app for
 *					importing.
 *
 * PARAMETERS:	->	dbP:				Pointer to the database to add
 *										the record to.
 *
 *				->	inputStream:		Pointer to where to import the
 *										record from.
 *
 *				->	inputFunc:			Function to get input from the
 *										stream.
 *
 * RETURNED: 		True if the input was read.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	DJK	8/9/97		Inital revision.
 *	RFL	9/12/97		Modified to work on the device.
 *	ART	10/17/97	Return unique id of first record inserted.
 *	ABA	06/21/00	Integrate Pdi library.
 *
 ***********************************************************************/
Boolean DateImportVCal(
	DmOpenRef 				dbP,
	PdiReaderType* 			reader,
	Boolean *				existingVToDoDataP,
	uint32_t * 				uniqueIDP,
	VCardSenderKindType * 	vCardSenderP,
	status_t * 				errorP)
{
    uint32_t 	uniqueID = dbInvalidRowID;
	char * 		palmOSVersionStr = NULL;
	VCardSenderKindType vCardSender = kNonPalmOSDevice;

	// Check vCalendar beginning
	*errorP = errNone;
	PdiReadProperty( reader);
	if (reader->property != kPdiPRN_BEGIN_VCALENDAR)
        return false;

    // Read in the vcard entry
    PdiEnterObject( reader);

    while (PdiReadProperty( reader) == 0 && reader->property != kPdiPRN_END_VCALENDAR)
    {
		switch(reader->property)
		{
			case kPdiPRN_X_PALM:
				// Read the emiting PalmOS device
				PdiReadPropertyField(reader, &palmOSVersionStr, kPdiResizableBuffer, kPdiNoFields);
				// Compute the sender terminal type
				vCardSender = PrvVCardSenderType(palmOSVersionStr);
				break;

			case kPdiPRN_BEGIN_VEVENT:
    			DateImportVEvent(dbP, reader, &uniqueID, vCardSender, errorP);
	            if (*errorP < errNone)
	            	goto Exit;

				// Test if reader->appData is equal to NULL: if we are sublaunched
				// for importing a vCal, there is not exchange socket valid.
				if (reader->appData != NULL)
					DateSetGoToParams (dbP, reader->appData, uniqueID);
				break;

			case kPdiPRN_BEGIN_VTODO:
	        	if (existingVToDoDataP)
	        		*existingVToDoDataP = true;
	        	break;
        }
    }

Exit:
	if (palmOSVersionStr)
		MemPtrFree(palmOSVersionStr);

    if (uniqueIDP)
    	*uniqueIDP = uniqueID;

    if (vCardSenderP)
    	vCardSender = *vCardSenderP;

    return ((reader->events & kPdiEOFEventMask) == 0);
}

/***********************************************************************
 *
 * FUNCTION:    	SetDescriptionAndFilename
 *
 * DESCRIPTION: 	Derive and allocate a decription and filename from
 *					some text.
 *
 * PARAMETERS:  ->	textP:			The text string to derive the names
 *									from.
 *				->	descriptionPP:	Pointer to set to the allocated
 *									description.
 *				->	descriptionHP:	Handle to set to the allocated
 *									description.
 *				->	filenamePP:		Pointer to set to the allocated
 *									filename.
 *				->	filenameHP:		Handle to set to the allocated
 *									description.
 *				->	prefix:			The scheme with ":" suffix and
 *									optional "?" prefix
 *
 * RETURNED:    	A description and filename are allocated and the
 *					pointers are set.
 *
 * REVISION HISTORY:
 *  RFL  11/4/97   Initial revision.
 *
 ***********************************************************************/
static void SetDescriptionAndFilename(
	char * 				textP,
	char **				descriptionPP,
	MemHandle *			descriptionHP,
	char **				filenamePP,
	MemHandle *			filenameHP,
	const char * const 	prefix)
{
    char * 				descriptionP;
    size_t	 			descriptionSize;
    Coord	 			descriptionWidth;
    Boolean 			descriptionFit;
    char * 				spaceP;
    char * 				filenameP;
    MemHandle 			resourceH;
    char * 				resourceP;
    uint8_t 			filenameLength;
    uint8_t 			schemeLength;


    descriptionSize = strlen(textP);
    WinGetDisplayExtent(&descriptionWidth, NULL);
    FntCharsInWidth (textP, &descriptionWidth, &descriptionSize, &descriptionFit);

    if (descriptionSize > 0)
    {
        *descriptionHP = MemHandleNew((uint32_t)(descriptionSize + sizeOf7BitChar('\0')));
        descriptionP = MemHandleLock(*descriptionHP);
        memmove(descriptionP, textP, descriptionSize);
        descriptionP[descriptionSize] = nullChr;
    }
    else
    {
        *descriptionHP = DmGetResource(gApplicationDbP, strRsc, beamDescriptionStrID);
        descriptionP = MemHandleLock(*descriptionHP);
    }


    if (descriptionSize > 0)
    {
        // Now form a file name.  Use only the first word or two.
        spaceP = StrChr(descriptionP, spaceChr);

        if (spaceP)
            // Check for a second space
            spaceP = StrChr(spaceP + sizeOf7BitChar(spaceChr), spaceChr);

        // If at least two spaces were found then use only that much of the description.
        // If less than two spaces were found then use all of the description.
        if (spaceP)
            filenameLength = spaceP - descriptionP;
        else
            filenameLength = (uint8_t) strlen(descriptionP);

        // Allocate space and form the filename
        schemeLength =(uint8_t) strlen(prefix);
        *filenameHP = MemHandleNew((uint32_t)(schemeLength + filenameLength + strlen(kDateSuffix) + sizeOf7BitChar('\0')));
        filenameP = MemHandleLock(*filenameHP);

        if (filenameP)
        {
            strcpy(filenameP, prefix);
            memmove(&filenameP[schemeLength], descriptionP, filenameLength);
            memmove(&filenameP[schemeLength + filenameLength], kDateSuffix, (int32_t)(strlen(kDateSuffix) + sizeOf7BitChar('\0')));
        }
    }
    else
    {
        resourceH = DmGetResource(gApplicationDbP, strRsc, beamFilenameStrID);
        resourceP = MemHandleLock(resourceH);

        // Allocate space and form the filename
        filenameLength = (uint8_t) strlen(resourceP);
        schemeLength = (uint8_t) strlen(prefix);

        *filenameHP = MemHandleNew((uint32_t)(schemeLength + filenameLength + 4 + sizeOf7BitChar('\0')));
        filenameP = MemHandleLock(*filenameHP);

        if (filenameP)
        {
            strcpy(filenameP, prefix);
            strcat(filenameP, resourceP);
            strcat(filenameP, kDateSuffix);
        }

        MemHandleUnlock(resourceH);
        DmReleaseResource(resourceH);
    }


    *descriptionPP = descriptionP;
    *filenamePP = filenameP;
}

/***********************************************************************
 *
 * FUNCTION:    PrvTransfertGetPdiParameters
 *
 * DESCRIPTION: Get the localized PDI parameter flags.
 *
 * PARAMETERS:	None.
 *
 * RETURNED:    PDI parameters flags
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		06/08/04	Initial Revision
 *
 ***********************************************************************/
static uint16_t PrvTransfertGetPdiParameters(void)
{
	uint16_t	param16;

	if (!(param16 = (uint16_t) (ResLoadConstant(gApplicationDbP, transfertPDIParameterID) & 0x0000FFFF)))
		param16 = kPdiEnableQuotedPrintable | kPdiBypassLocaleCharEncoding;

	return param16;
}

/***********************************************************************
 *
 * FUNCTION:    	DateSendThisRecord
 *
 * DESCRIPTION: 	Send a record.
 *
 * PARAMETERS:	 ->	dbP:		Pointer to the database to add the record
 *								to.
 * 				 ->	recordNum:	The record rowId or cursorID (currentItem)
 *								to send.
 * 				 ->	recordP:	Pointer to the record to send.
 * 				 ->	exgSocketP:	The exchange socket used to send.
 *
 * RETURNED:    	0 if there's no error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RFL  12/11/97  	Initial revision.
 *	ABA   06/21/00  Integrate Pdi library.
 *	PPL	07/17/03	Cursors !
 *
 ***********************************************************************/
static status_t DateSendThisRecord (
	DmOpenRef 			dbP,
	int32_t 			rowID,
	ApptDBRecordPtr 	recordP,
	UDAWriterType* 		media)
{
    status_t			error = errNone;
    PdiWriterType* 		writer;

	error = PdiLibOpen();

   	if (error)
       	return error;

   	writer = PdiWriterNew( media, PrvTransfertGetPdiParameters());

   	if (writer)
	{
		PdiWriteBeginObject( writer, kPdiPRN_BEGIN_VCALENDAR);
		PdiWriteProperty( writer, kPdiPRN_VERSION);
		PdiWritePropertyValue( writer, (char*)"1.0", kPdiWriteData);

		// Emit PalmOS version
		PdiWriteProperty( writer, kPdiPRN_X_PALM);
		PdiWritePropertyValue(writer, kVObjectPalmOSV6Version, kPdiWriteData);

		DateExportVCal(dbP, rowID, recordP, writer);
		PdiWriteEndObject( writer, kPdiPRN_END_VCALENDAR);

	    if (writer->errorLowWord == errNone)
            UDAWriterFlush(media);
	    else
	    	error = pdiErrorClass | ((status_t) writer->errorLowWord);

        PdiWriterDelete( &writer);
    }
    else
    {
    	error = exgMemError;
    }

	PdiLibClose();

    return error;
}

/***********************************************************************
 *
 * FUNCTION:    	DateSendRecord
 *
 * DESCRIPTION: 	Send a record.
 *
 * PARAMETERS:	->	dbP:	Pointer to the database to add the record to
 * 				->	rowID:	Record row ID or cursorID current item
 *				->	prefix:	The scheme with ":" suffix and optional "?"
 *							prefix.
 *
 * RETURNED:    	True if the record is found and sent.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RFL	5/9/97   	Initial revision.
 *  DJE 4/24/00  	Don't specify target creator ID.
 *	PPL	07/17/03	Cursors !
 *
 ***********************************************************************/
void DateSendRecord (DmOpenRef dbP, uint32_t rowID, const char * const prefix)
{
    ApptDBRecordType 	record;
    MemHandle 			descriptionH = NULL;
    status_t 			error;
    ExgSocketType 		exgSocket;
    MemHandle 			nameH = NULL;
    Boolean 			empty;
    UDAWriterType* 		media;

    // Form a description of what's being sent.  This will be displayed
    // by the system send dialog on the sending and receiving devices.
    error = ApptGetRecord (dbP, rowID, &record, DBK_SELECT_ALL_FIELDS);

    // If the description field is empty and the note field is empty, then
    // consider the record empty and don't beam it.  (Logic from ApptDeleteRecordIfEmpty)
    empty = true;

    if (record.description && *record.description)
        empty = false;
    else if (record.note && *record.note)
        empty = false;

    if (!empty)
    {
      	 // important to init structure to zeros...
        memset(&exgSocket, 0x00, sizeof(exgSocket));

        // Set the exg description to the record's description.
        SetDescriptionAndFilename(	record.description,
        								&exgSocket.description,
                                  		&descriptionH,
                                  		&exgSocket.name,
                                  		&nameH,
                                  		prefix);

		//ABa: remove superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);
		TraceOutput(TL(appErrorClass, "DateSendRecord: description = %s, name = %s", exgSocket.description, exgSocket.name));

		// Compute a rough guess of exgSocket.length
		exgSocket.length = VCARD_LENGTH_ROUGH_ESTIMATE;

		if (record.description != NULL)
			exgSocket.length += strlen(record.description);

		if (record.location != NULL)
			exgSocket.length += strlen(record.location);

		if (record.note != NULL)
			exgSocket.length += strlen(record.note);

		// Set the type
        exgSocket.type = (char *)dateMIMEType;

        error = ExgPut(&exgSocket);   // put data to destination

		// ABa: Changes to use new streaming mechanism
        media = UDAExchangeWriterNew(&exgSocket,  512);

        if (!error)
        {
        	if (media)
	            error = DateSendThisRecord(dbP, rowID, &record, media);
	        else
	        	error = exgMemError;

            ExgDisconnect(&exgSocket, error);
        }

        if (media)
	        UDADelete(media);
    }
    else
        FrmUIAlert(NoDataToBeamAlert);


    // Clean winUp
    //
    // The description may be an allocated memeory block or it may be a
    // locked resource.
    if (descriptionH)
    {
        MemHandleUnlock (descriptionH);

        if (MemHandleDataStorage (descriptionH))
            DmReleaseResource(descriptionH);
        else
            MemHandleFree(descriptionH);
    }

    // The name may be an allocated memeory block or it may be a
    // locked resource.
    if (nameH)
    {
        MemHandleUnlock (nameH);

        if (MemHandleDataStorage (nameH))
            DmReleaseResource(nameH);
        else
            MemHandleFree(nameH);
    }

	ApptFreeRecordMemory (&record);
}


/***********************************************************************
 *
 * FUNCTION:		DateReceiveDateBookData
 *
 * DESCRIPTION:
 *
 * PARAMETERS:		->	exgSocketP:	socket from the app code
 *						 		sysAppLaunchCmdExgReceiveData.
 *
 * RETURNED:		error code or zero for no error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
status_t DateReceiveDateBookData(DmOpenRef dbP, ExgSocketPtr exgSocketP, FileHand vObjCopyH,
	Boolean * existingVToDoDataP)
{
    PdiReaderType* 	reader = NULL;
    UDAReaderType* 	stream = NULL;
    Boolean			pdiLibLoaded = false;
	status_t 		err;

	FileRewind(vObjCopyH);

	err = PdiLibOpen();
    if (err < errNone)
		goto Exit;
    else
		pdiLibLoaded = true;

	stream = UDAFileStreamReaderNew(vObjCopyH);
    if (stream== NULL)
	{
		err = exgMemError;
    	goto Exit;
	}

	reader = PdiReaderNew(stream, kPdiOpenParser);
	if (reader == NULL)
	{
		err = exgMemError;
    	goto Exit;
	}

	// Give a pointer on the socket to initialize goToCreator / goToParams
    reader->appData = exgSocketP;

    // Catch errors receiving records.  The import routine will clean up the
    // incomplete record.  This routine gives the error to ExgDisconnect
    // which displays appropriate ui.

    // Keep importing records until it can't
    while (DateImportVCal(dbP, reader, existingVToDoDataP, NULL, NULL, &err))
	{
		if (err < errNone)
			goto Exit;
    }

	// Aba: A record has been added in the Database iff the GoTo
	// uniqueID parameter != 0.
	// In the case no record is added, return an error
	if ((err == errNone) && (! (*existingVToDoDataP)) && (exgSocketP->goToParams.uniqueID == 0))
		err = exgErrBadData;

Exit:
	if (reader)
		PdiReaderDelete(&reader);

	if (stream)
		UDADelete(stream);

	if (pdiLibLoaded)
		PdiLibClose();

	return err;
}

/***********************************************************************
 *
 * FUNCTION:		DateReceiveData
 *
 * DESCRIPTION:		Receives data into the output field using
 *					the Exg API.
 *
 * PARAMETERS:	->	exgSocketP:	socket from the app code
 *						 		sysAppLaunchCmdExgReceiveData.
 *
 * RETURNED:		error code or zero for no error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	???	???			Initial revision.
 *	ART	10/17/97	Added "go to" record logic.
 *  ABA 6/20/00  	Integrate Pdi library.
 *
 ***********************************************************************/
status_t DateReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP)
{
    status_t 		err;
	FileHand 		vObjCopyH = 0;
	Boolean 		doDisconnect = true;
	uint32_t		rcvdBytes;
	char 			copyBuffer[VCAL_TMP_BUFFER_SIZE];
	Boolean			existingVToDoData = false;
	ExgSocketType	newToDoSocket;

    // Accept will open a progress dialog and wait for your receive commands
    if ((err = ExgAccept(exgSocketP)) < errNone)
    	return err;

	// Make an internal copy in a file stream
	vObjCopyH = FileOpen(kDateBookvCalTmpFileName, 0, sysFileCDatebook,
		fileModeReadWrite | fileModeExclusive | fileModeTemporary, &err);
	if (err < errNone)
		goto Disconnect;

	// Get data from the exchange manager
	do
	{
		rcvdBytes = ExgReceive(exgSocketP, copyBuffer, VCAL_TMP_BUFFER_SIZE, &err);
		if ((err < errNone) || (! rcvdBytes))
			break;
		FileWrite(vObjCopyH, copyBuffer, rcvdBytes, sizeof(char), &err);
	}
	while (err >= errNone);

	if (err < errNone)
		goto Disconnect;

	// Extract data belonging to the datebook
	err = DateReceiveDateBookData(dbP, exgSocketP, vObjCopyH, &existingVToDoData);
	if (err < errNone)
		goto Disconnect;

	// Disconnect properly now
	ExgDisconnect(exgSocketP, err); // closes transfer dialog
	doDisconnect = false;

	// If there was data belonging to the ToDo application, send it now.
	// To do so, we switch the filestream to the destructive mode (this
	// will implicitely rewind it).
	if (existingVToDoData)
	{
		ExgLocalSocketInfoType localInfo; // need this to set noAsk flag.
		char *filename;
		memset(&localInfo, 0, sizeof(ExgLocalSocketInfoType));

		// Rewind and switch to destructive mode
		FileControl(fileOpDestructiveReadMode, vObjCopyH, NULL, NULL);
		memset(&newToDoSocket, 0, sizeof(ExgSocketType));
		// get the filename portion of the source URL (strip off the scheme)
		filename = StrChr(exgSocketP->name, ':') ? StrChr(exgSocketP->name, ':')+1 : exgSocketP->name;
		if (!filename) filename = "";
		newToDoSocket.name = MemPtrNew((uint32_t)(strlen(exgLocalPrefix) + StrLen(filename) + 1));
		strcpy(newToDoSocket.name, exgLocalPrefix);
		StrCat(newToDoSocket.name, filename);
		newToDoSocket.target = sysFileCToDo;
		newToDoSocket.localMode	= 1;
		localInfo.noAsk = true; // set noAsk and set SocketRef.
		newToDoSocket.socketRef = (uint32_t)&localInfo;
		newToDoSocket.socketRefSize = sizeof(ExgLocalSocketInfoType);

		if ((err = ExgConnect(&newToDoSocket)) >= errNone)
		{
			if ((err = ExgPut(&newToDoSocket)) >= errNone)
			{
				do
				{
					rcvdBytes = FileRead(vObjCopyH, copyBuffer, sizeof(char), VCAL_TMP_BUFFER_SIZE, &err);
					if ((err < errNone) && (err != fileErrEOF))
						break;
					if (rcvdBytes)
						ExgSend(&newToDoSocket, copyBuffer, rcvdBytes, &err);
				}
				while (err >= errNone);
			}
			if (err == fileErrEOF)
				err = errNone;
			ExgDisconnect(&newToDoSocket, err);
			MemPtrFree(newToDoSocket.name);
			err = errNone;	// error was reported, so don't return it
		}
	}

Disconnect:
	if (vObjCopyH)
		FileClose(vObjCopyH);

	if (doDisconnect)
	{
		ExgDisconnect(exgSocketP, err); // closes transfer dialog
		err = errNone;	// error was reported, so don't return it
	}

    return err;
}


/***********************************************************************
 *
 * FUNCTION:		DateTransferPreview
 *
 * DESCRIPTION:		Create a short string preview of the data coming in.
 *
 * PARAMETERS:	->	infoP:	the preview info from the command parameter
 *							block of the sysAppLaunchCmdExgPreview launch
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			DateTransferPreview is called in several different
 *					cases:
 *   		 			- Palm & non-Palm to Palm normal beam of a vevent.
 *           			- non Palm to Palm beam of vtodo.
 *           			- Palm & non-Palm to Palm SMS send of vevent.
 *           			- Palm & non-Palm to Palm SMS send of vtodo.
 *
 * REVISION HISTORY:
 *	ABA	10/20/00	Initial revision.
 *
 ***********************************************************************/
void DateTransferPreview(ExgPreviewInfoType *infoP)
{
	status_t 			err;
	PdiReaderType* 		reader;
	UDAReaderType* 		stream;
	uint16_t  			todoObjects = 0;
	uint16_t  			objects = 0;
	char*				descriptionP = NULL;
	DmResourceID		resID;
	MemHandle 			resH;
	void 				*resP;

	if (infoP->op == exgPreviewQuery)
	{
		infoP->types = exgPreviewShortString;
		return;
	}

	if (infoP->op != exgPreviewShortString)
	{
		infoP->error = exgErrNotSupported;
		return;
	}

	// if we have a description we don't have to parse the vObject
	if (infoP->socketP->description && *infoP->socketP->description)
	{
		StrNCopy(infoP->string, infoP->socketP->description, (uint16_t) (infoP->size - 1));
		infoP->string[infoP->size - 1] = 0;
		infoP->error = errNone;
		return;
	}

	err = ExgAccept(infoP->socketP);
	if (!err)
	{
		err = PdiLibOpen();
	}

	if (!err)
	{
		stream = UDAExchangeReaderNew(infoP->socketP);
		reader = PdiReaderNew( stream, kPdiOpenParser);
		reader->appData = infoP->socketP;

		if (reader)
		{
			PdiReadProperty(reader);
			if  (reader->property != kPdiPRN_BEGIN_VCALENDAR)
				goto ParseError;

			PdiEnterObject(reader);
			PdiDefineResizing( reader, 16, tableMaxTextItemSize);

			while (PdiReadProperty( reader) == 0 && objects <= 1)
			{
				switch (reader->property)
				{
					case kPdiPRN_BEGIN_VTODO:
						todoObjects++;

					case kPdiPRN_BEGIN_VEVENT:
						objects++;
						PdiEnterObject( reader);
						break;

					// VEVENT & VTODO description strings
					case kPdiPRN_DESCRIPTION:
					case kPdiPRN_SUMMARY:
						PdiReadPropertyField( reader, &descriptionP, kPdiResizableBuffer, kPdiDefaultFields);
						break;
				}
			}

			if (objects == 0)
			{
			ParseError:
				err = exgErrBadData;
			}
			else
			{
				resID = 0;

				if (objects == 1)
				{
					// a single object
					if (descriptionP == NULL || *descriptionP == 0)
					{
						resID = exgDesciptionUnknownStrID;
					}
				}
				else
				{
					// multiple objects
					if (todoObjects == objects)
					{
						// todo items
						resID = exgDescriptionMultipleToDoStrID;
					}
					else
					{
						// events
						resID = exgDescriptionMultipleStrID;
					}
				}

				if (resID)
				{
					resH = DmGetResource(gApplicationDbP, strRsc, resID);
					resP = MemHandleLock(resH);

					StrNCopy(infoP->string, resP, (uint16_t) infoP->size);
					MemHandleUnlock(resH);
					DmReleaseResource(resH);
				}
				else
				{
					StrNCopy(infoP->string, descriptionP,(int16_t) infoP->size);
				}

				infoP->string[infoP->size - 1] = chrNull;
			}

			if (descriptionP)
			{
				MemPtrFree(descriptionP);
			}

			ExgDisconnect(infoP->socketP, err);
		}

		PdiReaderDelete(&reader);
		UDADelete(stream);
		PdiLibClose();
	}
	infoP->error = err;
}


/***********************************************************************
 *
 * FUNCTION:		DateImportData
 *
 * DESCRIPTION:		Receives data from an imported vObject.
 *
 * PARAMETERS:	->	dbP:		Pointer to the database.
 *				->	impExpObjP:	Pointer on the record param Ptr.
 *
 *					see AppMgr.h ImportExportRecordParamsType
 *
 *					Import/Export are for 68k apps than needs to access
 *					to Arm DateBook records, we need to maintain an
 *					record index , record UniqueID paradigm. To achieve
 *					that we are using a cursor ordered on all records
 *					in the database,
 *					the position of records whinthin the cursor is used
 *					as the index the rowID of records as unique ID.
 *
 *
 * RETURNED:		Error code.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *         PLe    05/27/02  Added Import / Export feature
 *
 ***********************************************************************/
status_t DateImportData(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP)
{
	status_t 		err;
	PdiReaderType* 	reader = NULL;
	UDAReaderType* 	stream = NULL;
	Boolean			pdiLibLoaded = false;
	CategoryID 		dbSelectAllCategoriesID[] = {catIDAll};
	Boolean			recordToReplace = false;
	VCardSenderKindType receivedVCardType;
	uint32_t		newRecUniqueID = dbInvalidRowID;
	uint32_t		position;

	err = PdiLibOpen();
	if (err)
		goto Exit;

	pdiLibLoaded = true;

	if (impExpObjP->vObjectH == NULL)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	// Check if the input uniqueID matches an event in database
	if (impExpObjP->uniqueID != ImpExpInvalidUniqueID)
	{
		err = DbIsRowInCategory(dbP, impExpObjP->uniqueID, 1, dbSelectAllCategoriesID, DbMatchAny, &recordToReplace);
		if (err < errNone)
			recordToReplace = false;
	}

	// Create an UDA reader
	if ((stream = UDAMemHandleReaderNew(impExpObjP->vObjectH)) == NULL)
	{
		err = memErrNotEnoughSpace;
		goto Exit;
	}

	reader = PdiReaderNew(stream, kPdiOpenParser);

	if (! DateImportVCal(dbP, reader, NULL, &newRecUniqueID, &receivedVCardType, &err))
	{
		err = exgErrBadData;
		goto Exit;
	}

	// Replace if required
	if (recordToReplace)
	{
		err = ApptUpdateRecordWithNewColumns(dbP, cursorID, receivedVCardType, newRecUniqueID, impExpObjP->uniqueID);
		if (err >= errNone)
		{
			err = ApptDeleteRecord(dbP, newRecUniqueID, false);
			if (err < errNone)
				goto Exit;
		}
	}
	else
	{
		impExpObjP->uniqueID = newRecUniqueID;
	}

	// Requery to get new indexes
	ApptDBRequery(dbP, cursorID, false);

	err = DbCursorGetPositionForRowID(cursorID, impExpObjP->uniqueID, &position);
	if (err >= errNone)
		impExpObjP->index = position - 1;

Exit:
	if (reader)
		PdiReaderDelete(&reader);

	if (stream)
		UDADelete(stream);

	if (pdiLibLoaded)
		PdiLibClose();

	if (err < errNone)
	{
		impExpObjP->index = ImpExpInvalidRecIndex;
		impExpObjP->uniqueID = ImpExpInvalidUniqueID;
	}

	return err;
}

/***********************************************************************
 *
 * FUNCTION:		DateExportData
 *
 * DESCRIPTION:		Export a specified record as vObject.
 *					The function treats the old record index as a position
 *					whithin the opened cursor in the parameter list.
 *
 *					see AppMgr.h ImportExportRecordParamsType
 *
 *					Import/Export are for 68k apps than needs to access
 *					to Arm DateBook records, we need to maintain an
 *					record index , record UniqueID paradigm.
 *
 *					To achieve that we are using a cursor ordered on
 *					all records inthe database:
 *						- The position of records winthin the cursor
 *						is used as the index.
 *						- The rowID of records as unique ID.
 *
 * PARAMETERS:	->	dbP:		Pointer to the database.
 *				->	cursorID: 	CursorID on all recortds in the databases.
 *				->	impExpObjP:	Pointer on the record param Ptr.
 *
 *
 * RETURNED:		Error code or zero for no error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *  PLe	05/27/02  	Added Import / Export feature.
 *	PPL	07/17/03	cursors !
 *
 ***********************************************************************/
status_t DateExportData(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP)
{

	UDAWriterType* 		stream = NULL;
	ApptDBRecordType 	record;
	uint32_t 			numRecords;
	uint32_t			position;
	uint32_t			rowID;
	status_t 			err = errNone;

	numRecords = DbCursorGetRowCount(cursorID);
	position = impExpObjP->index +1; 	// position in 1 based, idexes were 0 based
	rowID	 = impExpObjP->uniqueID;

	// First: the index is the position whithin the open cursor
	if (impExpObjP->index != ImpExpInvalidRecIndex && impExpObjP->index < numRecords)
	{
		// the position is valid, tries to retrieve a recors RowId
		err = DbCursorGetRowIDForPosition(cursorID, position, &rowID);
		if (err < errNone)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
	}

	// Test if the row exists...
	err = DbCursorMoveToRowID(cursorID, rowID);
	if (err < errNone)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	err = DbCursorGetCurrentPosition(cursorID, &position);
	if (err < errNone)
		goto Exit;

	impExpObjP->index 		= position-1; 	// position in 1 based, idexes were 0 based
	impExpObjP->uniqueID 	= rowID;

	// Test if we only want to return matching rowID / index
	if (impExpObjP->vObjectH == NULL)
		goto Exit;

	// Create an PDI writer
	stream = UDAMemHandleWriterNew(impExpObjP->vObjectH,  ALLOCATE_VOBJECT_INC_SIZE);

    err = ApptGetRecord (dbP, cursorID, &record, DBK_SELECT_ALL_FIELDS);

    if (err >= errNone)
    {
		err = DateSendThisRecord(dbP, impExpObjP->uniqueID, &record, stream);

		ApptFreeRecordMemory (&record);
	}

	UDAWriterFlush(stream);

Exit:
	if (stream)
		UDADelete(stream);

	if (err < errNone)
	{
		impExpObjP->index 		= ImpExpInvalidRecIndex;
		impExpObjP->uniqueID 	= ImpExpInvalidUniqueID;
	}

	return err;

}

/***********************************************************************
 *
 * FUNCTION:		DateDeleteRecord
 *
 * DESCRIPTION:		Deletes a record from DB.
 *					The function treats the old record index as a position
 *					whithin the opened cursor in the parameter list.
 *
 *					see AppMgr.h ImportExportRecordParamsType
 *
 *					Import/Export are for 68k apps than needs to access
 *					to Arm DateBook records, we need to maintain an
 *					record index , record UniqueID paradigm.
 *
 *					To achieve that we are using a cursor ordered on
 *					all records inthe database:
 *						- The position of records winthin the cursor
 *						is used as the index.
 *						- The rowID of records as unique ID.
 *
 * PARAMETERS:	->	dbP:		pointer to the database
 *				->	cursorID: 	cursorID on all recortds in the databases
 *				->	impExpObjP:	pointer on the record param Ptr
 *
 *
 * RETURNED:		error code or zero for no error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	05/27/02  	Added Import / Export feature.
 *	PPL	07/17/03	Cursors !
 *
 ***********************************************************************/
status_t DateDeleteRecord(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP)
{
	uint32_t 	numRecords;
	status_t	err = errNone;
	uint32_t	index = impExpObjP->index + 1;

	numRecords = DbCursorGetRowCount(cursorID);

	if ((impExpObjP->index != ImpExpInvalidRecIndex) && (impExpObjP->index < numRecords))
	{
		// Check the validity of the record index
		if (DbCursorGetRowIDForPosition(cursorID, index, &impExpObjP->uniqueID) < errNone)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
	} else if (impExpObjP->uniqueID != ImpExpInvalidUniqueID)
	{
		// Get the index from the unique ID
		if (DbCursorGetPositionForRowID(cursorID, impExpObjP->uniqueID, &index) < errNone)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
	}
	else
	{
		// No valid index or unique ID
		err = dmErrInvalidParam;
		goto Exit;
	}

	// Assign the result
	impExpObjP->index = index - 1;

	// Now delete the record
	err = DbCursorMoveToRowID(cursorID, impExpObjP->uniqueID);
	if (err >= errNone)
		err = DbDeleteRow(dbP, cursorID);

Exit:
	if (err != errNone)
	{
		impExpObjP->index = ImpExpInvalidRecIndex;
		impExpObjP->uniqueID = ImpExpInvalidUniqueID;
	}

	return err;
}
