/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDB.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header for the Appointment Manager
 *
 *****************************************************************************/

#ifndef _DATEDB_H_
#define _DATEDB_H_

#include <DatebookSchemaDefs.h>
#include <DateTime.h>
#include <SchemaDatabases.h>
#include <sys/time.h>

#include "DateTransfer.h"

/************************************************************
 *
 * COMMENT BELOW TO SWITCH FROM NEW WHERE CLAUSE TO OLD BEHAVIOR
 *
 *************************************************************/
#define USE_WHERE_CLAUSE


/************************************************************
 * Appointments Date & Time informations
 *************************************************************/
typedef struct {
	TimeType			startTime;				// Time the appointment starts
	TimeType			endTime;				// Time the appointment ends
	DateType			date;					// date of appointment
	Boolean				midnightWrapped;		// true if the event is wrapped around midnight
	Boolean				noTimeEvent;			// true if the event is an untimed one
	char				timeZoneName[TZNAME_MAX];
												// Appointment Time Zone
} ApptDateTimeType;


/************************************************************
 * Alarm definitions
 *************************************************************/
enum alarmTypes {
	aauMinutes,
	aauHours,
	aauDays
} ;
typedef Enum8 AlarmUnitType;

typedef struct {
	int8_t				advance;				// Alarm advance (-1 = no alarm)
	AlarmUnitType		advanceUnit;			// minutes, hours, days
} AlarmInfoType;


/************************************************************
 * Repeat definitions (used to specify the frequency of
 * repeating appointments).
 *************************************************************/
enum RepeatTag {
	repeatNone,
	repeatDaily,
	repeatWeekly,
	repeatMonthlyByDay,
	repeatMonthlyByDate,
	repeatYearly
};
typedef Enum8  RepeatType;


/************************************************************
 * RepeatInfoType
 * This structure contains information about repeat appointments.  The
 * repeatOn member is only used by weelky and monthly-by-day repeating
 * appointments.  For weekly the byte is a bit field that contains the
 * days of the week the appointments occurs on (bit: 0-sun, 1-mon,
 * 2-tue, etc.).  For monthly-by-day the byte contains the day the
 * appointments occurs, (ex: the 3rd friday), the byte is of type
 * DayOfMonthType.
 *************************************************************/
typedef struct {
	RepeatType	repeatType;			// repeatNone, daily, weekly, monthlyByDay, etc.
	uint8_t		repeatFrequency;	// i.e. every 2 days if repeatType daily
	DateType	repeatEndDate;		// minus one if forever
	uint8_t		repeatOn;			// monthlyByDay and repeatWeekly only
	uint8_t  	repeatStartOfWeek;	// repeatWeekly only
} RepeatInfoType;

typedef RepeatInfoType * RepeatInfoPtr;


/************************************************************
 * ExceptionsListType
 * This is the structure for a repeating appointment's exceptions
 * list. The exceptions list is a variable length list of dates
 * that the repeating appointment should not appear on.  The list
 * is in chronological order.
 *************************************************************/
typedef struct {
	uint32_t    numExceptions;
	DateType *	exceptionsDatesP;
} ExceptionsListType;

typedef ExceptionsListType * ExceptionsListPtr;


/************************************************************
 * Appointments Full informations
 *************************************************************/
typedef struct {
	ApptDateTimeType		when;
	AlarmInfoType 			alarm;
	RepeatInfoType 			repeat;
	ExceptionsListType 		exceptions;
	char *					description;
	char *					location;
	char *					note;
} ApptDBRecordType;

typedef ApptDBRecordType * ApptDBRecordPtr;


/************************************************************
 * Appointment midnight overlapping definitions
 *************************************************************/
enum MidnightOverlapStateTag {
	overlapNone,
	overlapFirstPart,
	overlapFirstPartCommonRow,
	overlapScndPart
};
typedef Enum8  MidnightOverlapStateType;


#define kEventViewStandardDraw 	0
#define kEventViewHighlight 		1

/************************************************************
 * Appointment lists definitions (Returned by ApptGetAppointments)
 *************************************************************/
typedef struct {
	TimeType					startTime;					// Start time in day view
	TimeType					startTimeInPrevDay;		// Start time in previous day for overlapping events
	TimeType					endTime;					// End time in day view
	TimeType					latestEndTimeInNextDay;	// The latest end time in day view, next day
	MidnightOverlapStateType	overlapState;				// Day overlapping state
	Boolean						noTimeEvent;				// lag => true for untimed events
	Boolean						belongingToCurrCat;		// true if the appointment belongs to current category
															// If false, it is a conflicting appt from other category
	Boolean						toBeRemoved;				// Internal flag for conflicts computations
	uint32_t					rowID;						// The current record index in DB
	uint32_t					drawingFlags;				// Event drawing information in views
} ApptInfoType;

typedef ApptInfoType * ApptInfoPtr;


/************************************************************
 * This checks a weekly appointment's repeat info and returns
 * true if the appointment occurs on only one day per week.
 * The form (x & (x - 1)) masks out the lowest order bit in x.
 * (K&R, p. 51)
 * If only one bit is set, which occurs iff the appointment is only
 * once per week, then (x & (x - 1)) == 0.
 *************************************************************/
#define OnlyRepeatsOncePerWeek(R)  (((R).repeatOn & ((R).repeatOn - 1)) == 0)
// RepeatOnDOW - true if repeat info R repeats on day of week D
#define RepeatOnDOW(R, D)   ((1 << (D)) & ((R)->repeatOn))


/************************************************************
 * Filtering selectors (Used with ApptGetRecord for instance)
 *************************************************************/
#define DBK_SELECT_TIME					0x00000001
#define DBK_SELECT_ALARM				0x00000002
#define DBK_SELECT_REPEAT				0x00000004
#define DBK_SELECT_EXCEPTIONS			0x00000008
#define DBK_SELECT_DESCRIPTION			0x00000010
#define DBK_SELECT_LOCATION				0x00000020
#define DBK_SELECT_NOTE					0x00000040
#define DBK_SELECT_ALL_FIELDS			0xFFFFFFFF

typedef uint32_t  RecordFilteringInfoType;

/************************************************************
 *
 * Datebook Appointment constants.
 *
 *************************************************************/
#define apptMaxPerDay 					100		// max appointments displayable on a day.
#define apptMaxDisplayableAlarms 		10
#define apptMaxEndTime					0x1737	// 11:55 pm, hours in high byte, minutes in low byte

#define kMaxRangeShiftingDueToTZ		(13 * hoursInSeconds)	// Untimed events are UTC encoded so
																// enlarge the scanning range by this value.

/************************************************************
 *
 * Cursors Related.
 *
 *************************************************************/
#define kSQLWhereClause							" WHERE "
#define kSQLAndClause							" AND "
#define kSQLOrClause							" OR "

#ifndef USE_WHERE_CLAUSE
#undef DatebookGlobalRequest
#undef DatebookMonthRequest
#define DatebookGlobalRequest					DatebookApptTable DatebookOrderByRepeatThenStartDate
#define DatebookMonthRequest					DatebookApptTable DatebookOrderByRepeatThenStartDate
#endif


/************************************************************
 *
 * Appointment database routines.
 *
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

status_t 	DateDBCreateDefaultDatabase(void);

void 		DateDBSetDBBackupBit(DmOpenRef dbP);

status_t 	DateDBOpenDatabase(DmOpenRef *dbPP, uint16_t mode);

status_t 	DateCloseDatabase(void);

status_t 	DateReOpenDatabase(Boolean reloadPrefs, uint16_t eMode);

status_t 	ApptCloseAndReopenDatabaseAndCursor(Boolean reloadPrefs, uint16_t eMode);

status_t 	ApptOpenCursor(DmOpenRef dbP, const char* sql, uint32_t	flags, uint32_t * cursorIDP);

status_t 	ApptOpenCursorWithCategory(DmOpenRef dbP, const char* sql, uint32_t	flags, uint32_t	numCategories, CategoryID 	pCategoryIDs[], DbMatchModeType	matchMode, uint32_t * cursorIDP);

status_t 	ApptCloseCursor(uint32_t * cursorIDP);

status_t 	ApptGetRecord (DmOpenRef dbP, uint32_t rowID, ApptDBRecordType * apptRecP,
								RecordFilteringInfoType loadingFilter);

void 		ApptFreeRecordMemory (ApptDBRecordType * apptRecP);

status_t 	ApptDBOpenOrRequeryWithNewRange(DmOpenRef dbP, uint32_t * cursorIDP, time_t rangeStartDate, time_t rangeEndDate, Boolean queryAllCategories);

status_t 	ApptDBReOpenCursor(DmOpenRef dbP, uint32_t * cursorIDP);

void 		ApptDBRequery(DmOpenRef dbP, uint32_t cursorID, Boolean forceRequery);

status_t 	ApptDBRequeryOnDate(DmOpenRef dbP, uint32_t * cursorIDP, DateType theDate);

status_t 	ApptDBRequeryOnCategories(DmOpenRef dbP, uint32_t * cursorIDP);

status_t 	ApptNewRecord (DmOpenRef dbP, ApptDBRecordType * apptRecP, uint32_t *index);

status_t 	ApptDuplicateRecord(DmOpenRef dbP, uint32_t sourceRowId, RecordFilteringInfoType changedFields, RecordFilteringInfoType unwantedFilter, ApptDBRecordType * apptRecP, uint32_t *newRowIDP);

Boolean 	ApptEventHasDescription (DmOpenRef dbP, uint32_t rowID);

void 		ApptGetDescription (DmOpenRef dbP, uint32_t rowID, MemHandle descH);

void 		ApptChangeDescription (DmOpenRef dbP, uint32_t rowID, MemHandle descH);

void 		ApptChangeLocation (ApptDBRecordType * rectP, char* newLocationP,
										RecordFilteringInfoType * changedFieldsP);

Boolean 	ApptEventRepeatIsSet (DmOpenRef dbP, uint32_t rowID);

Boolean 	ApptEventAlarmIsSet (DmOpenRef dbP, uint32_t rowID);

Boolean 	ApptEventHasNote (DmOpenRef dbP, uint32_t rowID);

Boolean 	ApptRepeatEventIsFinished (DmOpenRef dbP, uint32_t rowID, time_t endCheckDateInUTCSecs);

void 		ApptDeleteNote (DmOpenRef dbP, uint32_t rowID);

status_t 	ApptDeleteRecord (DmOpenRef dbP, uint32_t rowID, Boolean archive);

status_t 	ApptUpdateRecordWithNewColumns (DmOpenRef dbP, uint32_t cursorID, VCardSenderKindType receivedVCardType,
					uint32_t fromRowID, uint32_t toRowID);

Boolean 	ApptIsRecordEmpty(DmOpenRef dbP, uint32_t cursorID);

Boolean 	ApptDeleteRecordIfEmpty (DmOpenRef dbP, uint32_t cursorID);

Boolean 	ApptEventTZDiffersFromSystemTZ (DmOpenRef dbP, uint32_t rowID);

status_t 	ApptChangeRecord (DmOpenRef dbP, uint32_t rowId, ApptDBRecordType * apptRecP,
										RecordFilteringInfoType changedFields);

status_t 	ApptAddException (DmOpenRef dbP, uint32_t rowID, DateType date);

void 		ApptRemoveExceptions(ApptDBRecordPtr apptRec);

Boolean 	ApptFindFirstNonRepeating (DmOpenRef dbP, uint32_t cursorID, DateType date);

Boolean		ApptRepeatsOnDate  (ApptDBRecordType * apptRecP, DateType date);

Boolean		ApptNextRepeat (ApptDBRecordType * apptRecP, DatePtr dateP, Boolean searchForward);

Boolean		ApptHasMultipleOccurences (ApptDBRecordType * apptRecP);

int32_t		ApptCountMultipleOccurences(ApptDBRecordPtr apptRecP);

uint32_t	ApptGetAlarmTime (ApptDBRecordType * apptRecP, uint32_t currentTime, Boolean searchForward);

uint32_t 	ApptGetTimeOfNextAlarm (DmOpenRef inDbR, uint32_t inAlarmStart, uint32_t * rowIDofEarliestAlarmP);

uint32_t 	ApptPostTriggeredAlarmsAndGetNext (DmOpenRef inDbR, uint32_t postAlarmTime,
				uint32_t getNextStartTime, uint32_t * rowIDofEarliestAlarmP);

void 		ApptGetAppointments (DmOpenRef dbP, uint32_t * cursorIDP, DateType date, int32_t days,
										MemHandle apptLists [], int32_t counts [], Boolean addConflicts);

uint32_t 	ApptGetSchemaColumnMaxSize(DmOpenRef dbP, uint32_t columnID);


#ifdef __cplusplus
}
#endif

#endif //_DATEDB_H_
