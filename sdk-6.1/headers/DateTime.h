/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DateTime.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Date and Time calculations
 *
 *****************************************************************************/

#ifndef _DATETIME_H_
#define	_DATETIME_H_

#include <LocaleMgr.h>		// LmLocaleType

enum TimeFormatTag
	{
	tfColon,				// 1:00
	tfColonAMPM,			// 1:00 pm
	tfColon24h,				// 13:00
	tfDot,					// 1.00
	tfDotAMPM,				// 1.00 pm
	tfDot24h,				// 13.00
	tfHoursAMPM,			// 1 pm
	tfHours24h,				// 13
	tfComma24h				// 13,00
	} ;
typedef Enum8 TimeFormatType ;


enum DaylightSavingsTag
	{
	dsNone,					// Daylight Savings Time not observed
	dsUSA,					// United States Daylight Savings Time
	dsAustralia,			// Australian Daylight Savings Time
	dsWesternEuropean,		// Western European Daylight Savings Time
	dsMiddleEuropean,		// Middle European Daylight Savings Time
	dsEasternEuropean,		// Eastern European Daylight Savings Time
	dsGreatBritain,			// Great Britain and Eire Daylight Savings Time
	dsRumania,				// Rumanian Daylight Savings Time
	dsTurkey,				// Turkish Daylight Savings Time
	dsAustraliaShifted		// Australian Daylight Savings Time with shift in 1986
	} ;
typedef Enum8 DaylightSavingsTypes ;


enum DateFormatTag {
	dfMDYWithSlashes,		// 12/31/95
	dfDMYWithSlashes,		// 31/12/95
	dfDMYWithDots,			// 31.12.95
	dfDMYWithDashes,		// 31-12-95
	dfYMDWithSlashes,		// 95/12/31
	dfYMDWithDots,			// 95.12.31
	dfYMDWithDashes,		// 95-12-31

	dfMDYLongWithComma,		// Dec 31, 1995
	dfDMYLong,				// 31 Dec 1995
	dfDMYLongWithDot,		// 31. Dec 1995
	dfDMYLongNoDay,			// Dec 1995
	dfDMYLongWithComma,		//	31 Dec, 1995
	dfYMDLongWithDot,		//	1995.12.31
	dfYMDLongWithSpace,		//	1995 Dec 31

	dfMYMed,				//	Dec '95
	dfMYMedNoPost,			//	Dec 95		(added for French 2.0 ROM)
	dfMDYWithDashes			// 12-31-95		(added for 4.0 ROM)
	} ;
typedef Enum8 DateFormatType ;

typedef struct {
	int16_t second;
	int16_t minute;
	int16_t hour;
	int16_t day;
	int16_t month;
	int16_t year;
	int16_t weekDay;			// Days since Sunday (0 to 6)
	} DateTimeType;
	
typedef DateTimeType *DateTimePtr;


// This is the time format.  Times are treated as words so don't
// change the order of the members in this structure.  Time
// comparisons are made treating the value as a uint16_t so the
// order must be different between big and little endian devices.
//
typedef struct {
#if CPU_ENDIAN == CPU_ENDIAN_BIG
	uint16_t        hours	:8;
	uint16_t        minutes	:8;
#elif CPU_ENDIAN == CPU_ENDIAN_LITTLE
	uint16_t        minutes	:8;
	uint16_t        hours	:8;
#endif
} TimeType;

typedef TimeType *TimePtr;

#define noTime	-1		// The entire TimeType is -1 if there isn't a time.


// This is the date format.  Dates are treated as words so don't
// change the order of the members in this structure.
//

#if (BITFIELD_LAYOUT == MSB_TO_LSB)

	typedef struct {
		uint16_t year  :7;                   // years since 1904 (MAC format)
		uint16_t month :4;
		uint16_t day   :5;
	} DateType;


#elif (BITFIELD_LAYOUT == LSB_TO_MSB)

	typedef struct {
		uint16_t day   :5;
		uint16_t month :4;
		uint16_t year  :7;                   // years since 1904 (MAC format)
	} DateType;

#endif


typedef DateType *DatePtr;


/************************************************************
 * Date Time Constants
 *************************************************************/

// Maximum lengths of strings return by the date and time formating
// routine DateToAscii and TimeToAscii.
#define timeStringLength		15
#define dateStringLength		15
#define longDateStrLength		31
#define dowDateStringLength		31
#define dowLongDateStrLength	47
#define timeZoneStringLength	50


#define firstYear				1904
#define numberOfYears			128
#define lastYear				(firstYear + numberOfYears - 1)



// Constants for time calculations
// Could change these from xIny to yPerX
#define secondsInSeconds	1
#define minutesInSeconds	60
#define hoursInMinutes		60
#define hoursInSeconds		(hoursInMinutes * minutesInSeconds)
#define hoursPerDay			24
//#define daysInSeconds		((int32_t)(hoursPerDay) * ((int32_t)hoursInSeconds))
#define daysInSeconds		(0x15180)	// cc bug

#define daysInWeek			7
#define daysInYear			365
#define daysInLeapYear		366
#define daysInFourYears		(daysInLeapYear + 3 * daysInYear)

#define monthsInYear			12

#define maxDays				((uint32_t) numberOfYears / 4 * daysInFourYears - 1)
#define maxSeconds			((uint32_t) (maxDays + 1) * daysInSeconds - 1)

// Values returned by DayOfWeek routine.
#define sunday					0
#define monday					1
#define tuesday					2
#define wednesday				3
#define thursday				4
#define friday					5
#define saturday				6

// Months of the year
#define january					1
#define february				2
#define march					3
#define april					4
#define may						5
#define june					6
#define july					7
#define august					8
#define september				9
#define october					10
#define november				11
#define december				12

// It would have been cool to have a real DayOfWeekType, but we #define the
// following for compatibility with existing code.  Please use the new name
// (DayOfMonthType).
#define DayOfWeekType DayOfMonthType

// Values returned by DoyOfMonth routine.
enum DayOfWeekTag {
	dom1stSun, dom1stMon, dom1stTue, dom1stWen, dom1stThu, dom1stFri, dom1stSat,
	dom2ndSun, dom2ndMon, dom2ndTue, dom2ndWen, dom2ndThu, dom2ndFri, dom2ndSat,
	dom3rdSun, dom3rdMon, dom3rdTue, dom3rdWen, dom3rdThu, dom3rdFri, dom3rdSat,
	dom4thSun, dom4thMon, dom4thTue, dom4thWen, dom4thThu, dom4thFri, dom4thSat,
	domLastSun, domLastMon, domLastTue, domLastWen, domLastThu, domLastFri,
	domLastSat
	} ;
typedef Enum8 DayOfWeekType ;

// Values used by DateTemplateToAscii routine.
#define dateTemplateChar					chrCircumflexAccent

enum {
	dateTemplateDayNum = '0',
	dateTemplateDOWName,
	dateTemplateMonthName,
	dateTemplateMonthNum,
	dateTemplateYearNum
};

#define	dateTemplateShortModifier		's'
#define	dateTemplateRegularModifier		'r'
#define	dateTemplateLongModifier		'l'
#define	dateTemplateLeadZeroModifier	'z'

//************************************************************
//* Date and Time macros
//***********************************************************

// Convert a date in a DateType structure to an uint16_t.
 #define DateToInt(date) (*(uint16_t *) &date)


// Convert a date in a DateType structure to a signed int.
 #define TimeToInt(time) (*(int16_t *) &time)



//************************************************************
//* Date Time procedures
//************************************************************
#ifdef __cplusplus
extern "C" {
#endif

void TimSecondsToDateTime(uint32_t seconds, DateTimePtr dateTimeP);

uint32_t TimDateTimeToSeconds(DateTimePtr dateTimeP);

void TimAdjust(DateTimePtr dateTimeP, int32_t adjustment);

uint32_t TimTimeZoneToUTC (uint32_t seconds, int16_t timeZone,
				int16_t daylightSavingAdjustment);

uint32_t TimUTCToTimeZone (uint32_t seconds, int16_t timeZone,
				int16_t daylightSavingAdjustment);

void TimeToAscii(uint8_t hours, uint8_t minutes, TimeFormatType timeFormat,
				char *pString);

// Replaces Use24HourFormat macro.
Boolean TimeIs24HourFormat(TimeFormatType timeFormat);

// Replaces TimeSeparator macro.
wchar32_t	TimeGetFormatSeparator(TimeFormatType timeFormat);

Boolean TimeGetFormatSuffix(TimeFormatType timeFormat, uint8_t hours, char* suffixStr);

/*
 * New API, please use.
 */
void TimeZoneToAscii(const char* timeZoneID, char* string);

/*
 * Deprecated API.  Formerly known as TimeZoneToAscii
 */
void TimeZoneToAsciiV50(int16_t timeZone, const LmLocaleType* localeP, char* string);

int16_t DaysInMonth(int16_t month, int16_t year);

int16_t DayOfWeek (int16_t month, int16_t day, int16_t year);

int16_t DayOfMonth (int16_t month, int16_t day, int16_t year);



// Date routines.
void DateSecondsToDate (uint32_t seconds, DatePtr date);

void DateDaysToDate (uint32_t days, DatePtr date);

uint32_t DateToDays (DateType date);

void DateAdjust (DatePtr dateP, int32_t adjustment);

void DateToAscii(uint8_t months, uint8_t days, uint16_t years,
				DateFormatType dateFormat, char *pString);

void DateToDOWDMFormat(uint8_t months, uint8_t days,  uint16_t years,
				DateFormatType dateFormat, char *pString);

uint16_t DateTemplateToAscii(const char *templateP, uint8_t months,
				uint8_t days, uint16_t years, char *stringP, int16_t stringLen);

#ifdef __cplusplus
}
#endif


#endif //_DATETIME_H_
