/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DatebookSchemaDefs.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Shared Header for the Datebook and the Sync library
 *
 *****************************************************************************/

#ifndef _DATEBOOK_SCHEMA_DEFS_H_
#define _DATEBOOK_SCHEMA_DEFS_H_


/************************************************************
 * Appointment constants
 *************************************************************/
#define apptNoTime					-1				// start time of an untimed appt.
#define apptNoEndDate				0xffff			// end date of appts that repeat forever
#define apptNoAlarm					-1
#define apptEndOfTime				0xffffffff
#define apptNoEndTimeValueInDB		0x7fffffff		// end date & time of appts that repeat forever in REPEAT_EVENT_END_DATE_COL_ID column

/************************************************************
 *
 * Schema database definitions
 *	Each column definition is followed by the Schema Type & Column Name
 *  set in resources (file DateDefaultDB.xrd)
 *
 *************************************************************/
#define REPEATING_EVENT_COL_ID				10		// dbBoolean - Repeating Event
#define UNTIMED_EVENT_COL_ID				20		// dbBoolean - Untimed Event
#define START_EVENT_DATE_TIME_COL_ID		30		// dbDateTimeSecs - Start Date and Time
#define END_EVENT_DATE_TIME_COL_ID			40		// dbDateTimeSecs - End Date and Time
#define EVENT_TIMEZONE_COL_ID				50		// dbVarChar - Timezone
#define EVENT_ALARM_ADVANCE_COL_ID			100		// dbInt8 - Alarm Advance
#define EVENT_ALARM_ADVANCE_UNIT_COL_ID		110		// dbUInt8 - Alarm Advance Unit
#define REPEAT_EVENT_TYPE_COL_ID			200		// dbUInt8 - Repeat Type
#define REPEAT_EVENT_END_DATE_COL_ID		210		// dbDateTimeSecs - Repeat End Date
#define REPEAT_EVENT_REPEAT_FREQ_COL_ID		220		// dbUInt8 - Repeat Frequency
#define REPEAT_EVENT_REPEAT_ON_ID			230		// dbUInt8 - Repeat On
#define REPEAT_EVENT_START_OF_WEEK_ID		240		// dbUInt8 - Repeat Start of Week
#define EVENT_EXCEPTIONS_DATES_ID			300		// dbVector | dbDateTimeSecs - Exceptions Dates
#define EVENT_DESCRIPTION_ID				400		// dbVarChar - Description
#define EVENT_LOCATION_ID					410		// dbVarChar - Location
#define EVENT_NOTE_ID						420		// dbVarChar - Note

#define REPEATING_EVENT_COL_NAME			"RepeatingEvent"
#define UNTIMED_EVENT_COL_NAME				"UntimedEvent"
#define START_EVENT_DATE_TIME_COL_NAME		"StartDateTime"
#define END_EVENT_DATE_TIME_COL_NAME		"EndDateTime"
#define EVENT_TIMEZONE_COL_NAME				"Timezone"
#define EVENT_ALARM_ADVANCE_COL_NAME		"TimeAdvance"
#define EVENT_ALARM_ADVANCE_UNIT_COL_NAME	"TimeAdvanceUnit"
#define REPEAT_EVENT_TYPE_COL_NAME			"RepeatType"
#define REPEAT_EVENT_END_DATE_COL_NAME		"RepeatEndDate"
#define REPEAT_EVENT_REPEAT_FREQ_COL_NAME	"RepeatFrequency"
#define REPEAT_EVENT_REPEAT_ON_NAME			"RepeatOn"
#define REPEAT_EVENT_START_OF_WEEK_NAME		"RepeatStartOfWeek"
#define EVENT_EXCEPTIONS_DATES_NAME			"ExceptionsDates"
#define EVENT_DESCRIPTION_NAME				"Description"
#define EVENT_LOCATION_NAME					"Location"
#define EVENT_NOTE_NAME						"Note"


// Schema ID set in resources
#define DATEBOOK_DB_SCHEMA_ID				1000

// Column indexes as string for SQL queries definitions
#define REPEATING_EVENT_COL_ID_STR			"10"
#define START_EVENT_DATE_TIME_COL_ID_STR	"30"


/************************************************************
 *
 *  Order definitions : 2 sort orders are defined by default:
 *		- Native one (DatebookOrderByStartDate)
 *		- Compatibility with 68K apps (DatebookOrderByRepeatThenStartDate)
 *
 *************************************************************/
#define DatebookDbName 							"RAM_DateBookDBS"
#define DatebookApptTable 						"RAM_DateBookDBSchema"

// Sort Orders definitions (CASELESS and ASC are default values)
#define DatebookOrderByRepeatThenStartDate 		" ORDER BY " REPEATING_EVENT_COL_NAME " DESC, " START_EVENT_DATE_TIME_COL_NAME
#define DatebookOrderByStartDate 				" ORDER BY " START_EVENT_DATE_TIME_COL_NAME

#define DatebookImportExportRequest				DatebookApptTable DatebookOrderByRepeatThenStartDate
#define DatebookGlobalRequest					DatebookApptTable DatebookOrderByStartDate
#define DatebookMonthRequest					DatebookApptTable DatebookOrderByStartDate


/************************************************************
 *
 *  Following macros are used to access DB data by semantic
 *	groups.
 *
 *************************************************************/
// Access the date and time information, including localization
#define DATEBOOK_TIME_INFORMATION { \
	UNTIMED_EVENT_COL_ID, \
	START_EVENT_DATE_TIME_COL_ID, \
	END_EVENT_DATE_TIME_COL_ID, \
	EVENT_TIMEZONE_COL_ID}
#define DBK_WHEN_UNTIMED_INDEX 						0
#define DBK_WHEN_START_INDEX 						1
#define DBK_WHEN_END_INDEX 							2
#define DBK_WHEN_TIMEZONE_INDEX 					3

// Access the alarm information contained in a record
#define DATEBOOK_ALARM_INFORMATION {\
	EVENT_ALARM_ADVANCE_COL_ID, \
	EVENT_ALARM_ADVANCE_UNIT_COL_ID }
#define DBK_ALARM_ADVANCE_INDEX 					0
#define DBK_ALARM_ADV_UNIT_INDEX 					1

// Access the repeating information contained in a record
#define DATEBOOK_REPEAT_INFORMATION {\
	REPEATING_EVENT_COL_ID, \
	REPEAT_EVENT_TYPE_COL_ID, \
	REPEAT_EVENT_END_DATE_COL_ID, \
	REPEAT_EVENT_REPEAT_FREQ_COL_ID, \
	REPEAT_EVENT_REPEAT_ON_ID, \
	REPEAT_EVENT_START_OF_WEEK_ID }
#define DBK_REPEATING_EVENT_INDEX 					0
#define DBK_REPEAT_TYPE_INDEX 						1
#define DBK_REPEAT_END_DATE_INDEX 					2
#define DBK_REPEAT_FREQ_INDEX 						3
#define DBK_REPEAT_ON_INDEX 						4
#define DBK_REPEAT_START_OF_WEEK_INDEX 				5


#endif  // _DATEBOOK_SCHEMA_DEFS_H_
