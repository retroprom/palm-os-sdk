/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateUtils.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This file defines some utilities functions.
 *
 *****************************************************************************/

#ifndef _DATEUTILS_H_
#define _DATEUTILS_H_


#include <DateTime.h>
#include <ExgMgr.h>
#include <PrivateRecords.h>
#include <TimeMgr.h>

#include <sys/time.h>

#include "DateDB.h"
#include "SelectTime.h"


#ifdef __cplusplus
extern "C" {
#endif


// New events
#define datebookRefreshDisplay		firstUserEvent
#define datebookEditSelectedNote	(firstUserEvent + 1)
#define datebookPrefPlaySound		(firstUserEvent + 2)
#define datebookRestoreSelection	(firstUserEvent + 3)


// Enqueue datebook specific events
void 		DateBkEnqueueEvent(eventsEnum eventTypeVal);

// UI Tools
void		HideObject (FormPtr frm, uint16_t objectID);

void		ShowObject (FormPtr frm, uint16_t objectID);

void 		GetTitleBounds(FormPtr formP, RectanglePtr titleBoundsP);

// UTC timezone tag
#define 	UTCTimezone			"UTC"

int16_t 	GetBitmapWidth(DmResourceID resID);

Boolean 	RectIntersects(RectangleType * Rect1P, RectangleType * Rect2P);

// Time comparaison
int16_t		DateCompare (DateType d1, DateType d2);

int16_t		TimeCompare (TimeType t1, TimeType t2);

// Time conversions between PalmOS base types and libc standard types
DateType 	CnvDateTime2Date(DateTimeType* dateTimeP);

TimeType 	CnvDateTime2Time(DateTimeType* dateTimeP);

void 		CnvDate2DateTime(DateType* inDateP, TimeType* inTimeP, DateTimeType* outDateTimeP);

// Misc
Boolean		RepeatsOnDate(RepeatInfoType *repeatInfoP, DateType startDate, DateType date);

// Call to time picker dialog
Boolean 	GetTime (TimePtr startP, TimePtr endP, DatePtr startDateP, Boolean* wrappingEvtP,
						TimeSelectionType * selectTypeP, uint16_t titleStrID);

// Conversion tools UTC <-> local time
void 		CnvUTC2TimeZone(time_t * inUTCTimeP, const char * timeZoneName, DateTimeType * outTZTimeP);

time_t 		CnvTimeZone2UTC(DateTimeType * inTZTimeP, const char * timeZoneName);

void 		CnvLocalTime2UTC(DateTimeType * inLocalTimeP, time_t * outUTCTimeP, Boolean noTimeAppt);

void 		CnvUTC2LocalTime(time_t * inUTCTimeP, DateTimeType * outLocalTimeP, Boolean noTimeAppt);

void 		CnvApptTZ2LocalTime(ApptDateTimeType * inApptTZP, RepeatInfoType * inRepeatP,
										ApptDateTimeType * outLocalTZP,	RepeatInfoType * outRepeatP);

void 		CnvLocalTime2ApptTZ(ApptDateTimeType * inLocalTimeP, RepeatInfoType * inRepeatP,
										ApptDateTimeType * outApptTZP, RepeatInfoType * outRepeatP);

void 		CnvDateType2TimeT(
				DateType* 			dateP,
 				time_t* 			dateInSecondsP,
 				Boolean			   	noTimeAppt);


void 		CnvTimeT2DateType(
   				time_t*			   iDateInSecondsP,
   				DateType*		   oDateP,
   				Boolean			   noTimeAppt);


void 		CalculateStartEndRangeTimes(
				DateType*			startOfRange,
				int32_t 			daysInRange,
				time_t* 			rangeStartTime,
				time_t* 			rangeEndTime,
				DateType* 			endOfRangeP);


Boolean 	TimezoneExistsInDatabase(char * timezone2test);


void 		CnvUTCDateTime2LocalTime(DateTimeType * inUTCDateTimeP, DateTimeType * outLocalDateTimeP);


void 		CnvTZDateTime2UTCTime(
				DateTimeType * 		inLocalDateTimeP,
				char * 				inTimeZoneP,
				DateTimeType * 		outUTCDateTimeP);


void 		CnvUTCDateTime2TZTime(
				DateTimeType * 		inLocalDateTimeP,
				DateTimeType * 		outUTCDateTimeP,
				char * 				outTimeZoneP);

void 		CnvApptTZDateTime2LocalTime(
				DateTimeType * 		inApptTZDateTimeP,
				char * 				inTZP,
				DateTimeType * 		outLocalDateTimeP);


// Convertion on repeating rules
uint8_t 	ComputeRepeatOnForTimeZone(
				uint8_t 			UTCRepeatOnValue,
				char * 				timeZoneP,
				RepeatType 			repeatType,
				ApptDateTimeType * 	apptRecWhenP);

uint8_t 	ComputeRepeatOnForUTC(
				uint8_t 			TZRepeatOnValue,
				char * 				timeZoneP,
				RepeatType 			repeatType,
				ApptDateTimeType * 	apptRecWhenP);

// Categories handling
void 		ChangeCurrentCategories(uint32_t	newCatSelectionNum, CategoryID * newCatSelectionP);


Boolean 	DatebookAllCategorySelected(void);


Boolean 	CategoriesTestEquality(
				uint32_t 			numCategories1,
				CategoryID 			pCategoryIDs1[],
				uint32_t 			numCategories2,
				CategoryID 			pCategoryIDs2[]);

// Exchange manager accepting routine
status_t 	CustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP);


// other utility functions
wchar32_t 	DateUtilsGetUpperChar(wchar32_t inChar);

uint16_t 	DateUtilsGetDayOfWeek (uint16_t month, uint16_t day, uint16_t year);

Boolean 	DateUtilsSecVerifyPW(privateRecordViewEnum newSecLevel);

privateRecordViewEnum
			DateUtilsSecSelectViewStatus(void);


char* 		DateUtilsStripNonPrintableChar(char* stringToStrip);

void		DateUtilsSetCategoryTriggerLabel(FormType *formP, uint16_t triggerID);

// Trace stuff
#if defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF
	void 	DebugTraceAppt(ApptDBRecordType * apptRecP);

	void 	DebugTraceEvent(char * descP, EventType* eventP);

	void 	DebugTraceDateTime(DateTimeType * tmpDateTimeP);

	#define DBGTraceAppt(theAppointment)	DebugTraceAppt(theAppointment)
	#define DBGTraceEvent(desc, theEvent)	DebugTraceEvent(desc, theEvent)
	#define DBGTraceDateTime(theDate)		DebugTraceDateTime(theDate)

#else

	#define DBGTraceAppt(theAppointment)
	#define DBGTraceEvent(desc, theEvent)
	#define DBGTraceDateTime(theDate)

#endif

#ifdef __cpluscplus
}
#endif

#endif /* _DATEUTILS_H_ */
