/******************************************************************************
 *
 * Copyright (c) 1904-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDB.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		DateBook database access routines.
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
#include <MemoryMgrCompatibility.h>
#include <SchemaDatabases.h>
#include <string.h>
#include <StringMgr.h>
#include <SystemResources.h>
#include <SysUtils.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>

#include "DateGlobals.h"

#include "Datebook.h"
#include "DateAlarm.h"
#include "DateDB.h"
#include "DateUtils.h"

#include "DatebookRsc.h"


/************************************************************************
 *
 *	Internal constants and structures
 *
 ***********************************************************************/
#define kSqlQueryMaxSize	256

typedef struct {
	DmOpenRef				dbP;
	DatabaseID				dbID;
	DatebookPreferenceType	prefs;
} AlarmPostingData;

typedef AlarmPostingData * AlarmPostingDataPtr;

/************************************************************************
 *
 *	Internal statics
 *
 ***********************************************************************/
static Boolean 		sDateDBupdated = false;				// Flags any DB newly added row
static char 		sDateDBsqlQuery[kSqlQueryMaxSize];
static time_t		sSqlQueryRangeStartDate = INT_MIN;
static time_t		sSqlQueryRangeEndDate = INT_MIN;
static Boolean		sSqlQueryAllCategories = false;
static uint32_t		sSqlQueryCategoriesCount = 0;
static CategoryID 	sSqlQueryCategory = catIDUnfiled;	// currently displayed categories
static uint32_t		sCursorFlags = dbCursorEnableCaching;

// Tracing stuff - To remove when DateBook will be bug-free !!!!
#if ((TRACE_OUTPUT == TRACE_OUTPUT_ON) && (!defined(PIM_APPS_PROFILING)))
void ApptListTrace (int32_t days, MemHandle apptLists [], int32_t counts []);
void ApptCursorTrace(DmOpenRef dbP, uint32_t cursorID);
#endif

/***********************************************************************
 *
 *	Internal macros
 *
 ***********************************************************************/
#define DbgOnlyCheckIndexAndColumnID(columnValuesArray, columnIndex, columnId, columnDescString) \
	DbgOnlyFatalErrorIf(columnValuesArray[columnIndex].columnID != columnId, \
		"Retrieving record: wrong " columnDescString " column ID;"); \
	DbgOnlyFatalErrorIf(columnValuesArray[columnIndex].errCode != errNone, \
		"Retrieving record: error getting " columnDescString " column ID data.")

#define DATEBOOK_NON_REQUIRED_PALM_OS_5_COLUMNS { 	\
		EVENT_ALARM_ADVANCE_COL_ID,			\
		EVENT_ALARM_ADVANCE_UNIT_COL_ID,	\
		REPEAT_EVENT_TYPE_COL_ID,			\
		REPEAT_EVENT_END_DATE_COL_ID,		\
		REPEAT_EVENT_REPEAT_FREQ_COL_ID,	\
		REPEAT_EVENT_REPEAT_ON_ID,			\
		REPEAT_EVENT_START_OF_WEEK_ID,		\
		EVENT_EXCEPTIONS_DATES_ID,			\
		EVENT_DESCRIPTION_ID,				\
		EVENT_NOTE_ID}

#define DATEBOOK_ADDITIONNAL_PALM_OS_6_COLUMNS { 	\
		EVENT_LOCATION_ID}

/************************************************************************
 *
 * FUNCTION:     	DateDBSetDBBackupBit
 *
 * DESCRIPTION:  	This routine sets the backup bit on the given database.
 *					This is to aid syncs with non Palm software.
 *					If no DB is given, open the app's default database and set
 *					the backup bit on it.
 *
 * PARAMETERS:   ->	dbP:	the database to set backup bit,
 *							can be NULL to indicate app's default database
 *
 * RETURNED:     	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GGL	4/1/99		Initial Revision.
 *	PPL	07/10/03	Cursors !
 *
 ***********************************************************************/
void DateDBSetDBBackupBit(DmOpenRef dbP)
{
	DmOpenRef 			localDBP;
	uint16_t 			attributes;
	DmDatabaseInfoType 	datebookDbInfo;
	DatabaseID 			dbID;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
	{
		localDBP = DbOpenDatabaseByName(sysFileCDatebook, DatebookDbName, dmModeReadWrite, dbShareRead);

		if (localDBP == NULL)
			return;
	}
	else
	{
		localDBP = dbP;
	}

	// now set the backup bit on localDBP
	memset(&datebookDbInfo, 0x00, sizeof(DmDatabaseInfoType));
	datebookDbInfo.pAttributes = &attributes;

	dbID = DmFindDatabase(DatebookDbName, sysFileCDatebook, dmFindSchemaDB, &datebookDbInfo);

	DmDatabaseInfo(dbID, &datebookDbInfo);
	attributes |= dmHdrAttrBackup;
	DmSetDatabaseInfo(dbID, &datebookDbInfo);

	// close database if necessary
   if (dbP == NULL)
		DbCloseDatabase (localDBP);
}


/************************************************************************
 *
 * FUNCTION:		DateDBCreateDefaultDatabase
 *
 * DESCRIPTION:		Create the DatebookDB from resource image.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		errNone if no error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	10/31/02	Initial revision.
 *
 ***********************************************************************/
status_t DateDBCreateDefaultDatabase(void)
{
	status_t		err;
	MemHandle		defaultDBH;
	void *			defaultDBP;
	DmOpenRef 		dbP ;
	DatabaseID		dbID;

	if ((defaultDBH = DmGetResource(gApplicationDbP, sysResTDefaultSchemaDB, DatebookDefaultDB)) == NULL)
		return DmGetLastErr();

	if ((defaultDBP = MemHandleLock(defaultDBH)) == NULL)
	{
		DmReleaseResource(defaultDBH);
		return memErrNotEnoughSpace;
	}

	err = DmCreateDatabaseFromImage(defaultDBP, &dbID);

	// Always unlock/release, even if there's an error.
	MemHandleUnlock(defaultDBH);
	DmReleaseResource(defaultDBH);

	if (err < errNone)
	{
		// Database can already exist if user is changing the system locale,
		// and they'd previously booted the device.
		DbgOnlyFatalErrorIf(err != dmErrAlreadyExists, "Failed creating default database");
		goto Exit;
	}

	dbP = DbOpenDatabaseByName(sysFileCDatebook, DatebookDbName, dmModeReadWrite, dbShareRead);

	// Set the backup bit.  This is to aid syncs with non Palm software.
	DateDBSetDBBackupBit(dbP);

	// Sort by START DATE ONLY
	err = DbAddSortIndex(dbP, DatebookApptTable DatebookOrderByStartDate);
	DbgOnlyFatalErrorIf(err< errNone, "DatebookDB: Couldn't set the newly added sortID");

	// Sort by REPEAT AND START DATE (Import / Export launch codes)
	err = DbAddSortIndex(dbP, DatebookApptTable DatebookOrderByRepeatThenStartDate);
	DbgOnlyFatalErrorIf(err < errNone, "DatebookDB: Couldn't set the newly added sortID");

	// Close DB
	DbCloseDatabase (dbP);

Exit:
	return err;
}


/************************************************************************
 *
 * FUNCTION:    	DateDBOpenDatabase
 *
 * DESCRIPTION: 	Get the application's database.
 *					Open the database if it exists, create it if neccessary.
 *
 * PARAMETERS:  ->	dbPP:	Pointer to a database ref (DmOpenRef)
 *							to be set.
 *				->	mode:	How to open the database (dmModeReadWrite.)
 *
 * RETURNED:    	Zero if no error, else the error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	10/31/02	Initial Revision.
 *	PPL	07/10/03	Cursors !
 *
 ***********************************************************************/
status_t DateDBOpenDatabase(DmOpenRef *dbPP, uint16_t mode)
{
	DmOpenRef 			dbRef;
	uint16_t 			shareMode;
	status_t 			error = errNone;

	*dbPP = 0;

	// Set the share mode: If we open with write access, we must set the share mode
	// to dbShareRead to let the DB be open by other threads in readOnly.
	// If we open with read access only, we must set the shared mode to dbShareReadWrite
	// because the DB may already be open by the application with dmReadWrite access.
	if (mode & dmModeWrite)
		shareMode = dbShareRead;
	else if (mode & dmModeReadOnly)
		shareMode = dbShareReadWrite;
	else
		shareMode = dbShareNone;


	dbRef = DbOpenDatabaseByName(sysFileCDatebook, DatebookDbName, mode, shareMode);

	// If it doesn't exist, create from resources
	if (! dbRef)
	{
		if (mode & dmModeWrite)
		{
			// We have right access, let's try to create database from default
			error = DateDBCreateDefaultDatabase();
			if (error)
				return error;

			dbRef = DbOpenDatabaseByName(sysFileCDatebook, DatebookDbName, mode, shareMode);

		}
		if (! dbRef)
		{
			DbgOnlyFatalError("DatebookDB: Unable to open or create DB.");
			return DmGetLastErr();
		}
	}

	DbgOnlyFatalErrorIf(!DbHasSortIndex(dbRef, DatebookApptTable DatebookOrderByStartDate),
		"DatebookDB: Default sortID not set, not enough access rights");

	DbgOnlyFatalErrorIf(!DbHasSortIndex(dbRef, DatebookApptTable DatebookOrderByRepeatThenStartDate),
		"DatebookDB: Import / Export sortID not set, not enough access rights");

	if (mode & dmModeWrite)
	{
		error = DbEnableSorting(dbRef, true);
		DbgOnlyFatalErrorIf(error < errNone, "DatebookDB: Couldn't enable sorting");
	}

	*dbPP = dbRef;
	return error;
}


/************************************************************************
 *
 * FUNCTION:    	DateCloseDatabase
 *
 * DESCRIPTION: 	Closes the database if it is opened.
 *					Set global database openref to NULL.
 *
 * PARAMETERS: 		None.
 *
 * RETURNED:    	Zero if no error, else the error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/28/03	Initial Revision.
 *
 ***********************************************************************/
status_t DateCloseDatabase(void)
{
	status_t err = errNone;

	if (ApptDB)
	{
		// Close the application's data file.
		err = DbCloseDatabase (ApptDB);
		ApptDB = NULL;
	}

	return err;
}


/************************************************************************
 *
 * FUNCTION:    	DateReOpenDatabase
 *
 * DESCRIPTION: 	Opens or re-opens the database as required
 *					using the current security mode.
 *
 * PARAMETERS: 		None.
 *
 * RETURNED:    	Zero if no error, else the error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/28/03	Initial Revision.
 *
 ***********************************************************************/
status_t DateReOpenDatabase(Boolean reloadPrefs, uint16_t eMode)
{
	status_t 	err = errNone;
	uint16_t	mode;

	// check if database is not opened, and then re-opens it
	if (!ApptDB)
	{
		// database is not opened, reopens it
		// occurs when the security dialog is up
		// as we closed cursors and Database before to open it

		if (eMode)
		{
			mode = eMode;
		}
		else
		{
			if (reloadPrefs)
			{
				PrivateRecordVisualStatus =
				(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
			}

			mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
							dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
		}

		err = DateDBOpenDatabase(&ApptDB, mode);
	}

	return err;
}

/************************************************************************
 *
 * FUNCTION:    	ApptCloseAndReopenDatabaseAndCursor
 *
 * DESCRIPTION: 	Close and reopens with the mode given in parameter
 *					database and global cursor.
 *
 * PARAMETERS: 		None.
 *
 * RETURNED:    	Zero if no error, else the error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/29/03	Initial Revision.
 *
 ***********************************************************************/
status_t ApptCloseAndReopenDatabaseAndCursor(Boolean reloadPrefs, uint16_t eMode)
{
	uint32_t  	previousRowID = dbInvalidRowID;
	status_t 	err;

	if (gApptCursorID != dbInvalidCursorID)
	{
		err = DbCursorGetCurrentRowID(gApptCursorID, &previousRowID);

		err = ApptCloseCursor(&gApptCursorID);
	}

	err = DateCloseDatabase();


	err = DateReOpenDatabase(reloadPrefs, eMode);


	err = ApptDBOpenOrRequeryWithNewRange(
			ApptDB,
			&gApptCursorID,
			sSqlQueryRangeStartDate,
			sSqlQueryRangeEndDate,
			sSqlQueryAllCategories);

	if ( gApptCursorID != dbInvalidCursorID && previousRowID != dbInvalidRowID)
		err = DbCursorMoveToRowID(gApptCursorID, previousRowID);

	return err;
}


/************************************************************************
 *
 * FUNCTION: 		ApptBuildSQLRequestWithRange
 *
 * DESCRIPTION: 	Create the SQL query, it includes:
 *					- Non repeated events
 *					- Repeated events that have repeat end time
 *					before alarm start.
 *
 *					We can ignore non-repeated events starting before
 *					dbStartRange because alarms always occur
 *					on or before the start of the event.
 *
 * PARAMETERS: ->	sqlStorage: 	sql String container.
 *				-> 	rangeStartDate: start time for date ranges.
 *				->	rangeEndDate: 	End time for date ranges.
 *
 *
 * RETURNED: 		sql request string storage passed as argument.
 *
 * NOTE: 			Do not specify the range in hexadecimal in sql query,
 *					it can be negative !!!
 *
 *					Dates are negatives when before 1st january 1970
 *					the date have to be specified in decimal since
 *					hexadecimal values cannot be signed when converted
 *					leading to wrong values for dates in the sql request.
 *
 *
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision. (Factorization.)
 *
 ***********************************************************************/
static char* ApptBuildSQLRequestWithRange(char* sqlStorage, time_t rangeStartDate, time_t rangeEndDate)
{
	// NB
	//	Do not specify the range in hexadecimal in sql query, it can be negative !!!
	//
	// 	Dates are negatives when before 1st january 1970
	// 	the date have to be specified in decimal
	// 	since hexadecimal values cannot be signed when converted
	// 	leading to wrong values for dates in the sql request

	sprintf(sqlStorage,
		DatebookApptTable kSQLWhereClause \
		"%s = 0" kSQLAndClause "%s > %ld" kSQLAndClause "%s < %ld" \
		kSQLOrClause \
		"%s = 1" kSQLAndClause "%s < %ld" kSQLAndClause "%s > %ld" \
		DatebookOrderByStartDate,
		REPEATING_EVENT_COL_NAME, END_EVENT_DATE_TIME_COL_NAME, rangeStartDate, START_EVENT_DATE_TIME_COL_NAME, rangeEndDate,
		REPEATING_EVENT_COL_NAME, START_EVENT_DATE_TIME_COL_NAME, rangeEndDate, REPEAT_EVENT_END_DATE_COL_NAME, rangeStartDate);

	return sqlStorage;
}

/************************************************************************
 *
 * FUNCTION: 		ApptBuildSQLRequestForAlarmMunger
 *
 * DESCRIPTION: 	Create the SQL query, it includes:
 *					- Non repeated events
 *					- Repeated events that have repeat end time
 *					before alarm start.
 *
 *					We can ignore non-repeated events starting before
 *					dbStartRange because alarms always occur
 *					on or before the start of the event.
 *
 * PARAMETERS: 	->	sqlStorage: sql String container.
 *				-> 	rangeStart: start time for date ranges to find alarms.
 *
 * RETURNED: 		sql request string storage passed as argument.
 *
 * NOTE: 			Do not specify the range in hexadecimal in sql query,
 *					it can be negative !!!
 *
 *					Dates are negatives when before 1st january 1970
 *					the date have to be specified in decimal since
 *					hexadecimal values cannot be signed when converted
 *					leading to wrong values for dates in the sql request.
 *
 *
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision. (Factorization.)
 *
 ***********************************************************************/
static char* ApptBuildSQLRequestForAlarmMunger(char* sqlStorage, time_t rangeStart)
{
	// NB
	//	Do not specify the range in hexadecimal in sql query, it can be negative !!!
	//
	// 	Dates are negatives when before 1st january 1970
	// 	the date have to be specified in decimal
	// 	since hexadecimal values cannot be signed when converted
	// 	leading to wrong values for dates in the sql request

	sprintf(sqlStorage,
		DatebookApptTable kSQLWhereClause \
			"%s = 0" kSQLAndClause "%s >= %ld" \
			kSQLOrClause \
			"%s = 1" kSQLAndClause "%s >= %ld" \
			DatebookOrderByStartDate,
		REPEATING_EVENT_COL_NAME, START_EVENT_DATE_TIME_COL_NAME, rangeStart,
		REPEATING_EVENT_COL_NAME, REPEAT_EVENT_END_DATE_COL_NAME, rangeStart);

	return sqlStorage;
}


/************************************************************************
 *
 * FUNCTION:    	ApptOpenCursor
 *
 * DESCRIPTION:		Opens cursor on database without categories.
 *
 * PARAMETERS:	->	dbP:
 *				->	sql:
 *				->	flags:
 *				<->	cursorIDP:
 *
 * RETURNED:    	zero if no error, else the error
 *
 * NOTE:			flags are masked with sCursorFlags.
 *					so we can re-inforce cursor flags.
 *					e.g dbCursorEnableCaching.
 *
 * REVISION HISTORY:
 *	PPL	07/10/03	Initial Revision. cursors !
 *
 ***********************************************************************/
status_t ApptOpenCursor(DmOpenRef dbP, const char* sql, uint32_t flags, uint32_t* cursorIDP)
{
	uint32_t 	cursorID;
	status_t	err;

	*cursorIDP = dbInvalidCursorID;;

	err = DbCursorOpen(dbP, sql , (flags | sCursorFlags), &cursorID);

	if (err >= errNone)
		*cursorIDP = cursorID;

	return err;
}


/************************************************************************
 *
 * FUNCTION:    	ApptCloseCursor
 *
 * DESCRIPTION:		Closes the cursor.
 *
 * PARAMETERS:  ->	cursorIDP:  cursor Id of the cursor to close.
 *
 * RETURNED:    	Zero if no error, else the error
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	07/10/03	Initial Revision. cursors !
 *
 ***********************************************************************/
status_t ApptCloseCursor(uint32_t* cursorIDP)
{
	status_t err = errNone;

	if (*cursorIDP != dbInvalidCursorID)
	{
		err = DbCursorClose(*cursorIDP);
		*cursorIDP = dbInvalidCursorID;
	}

	return err;
}


/************************************************************************
 *
 * FUNCTION:    	ApptOpenCursorWithCategory
 *
 * DESCRIPTION:		Opens cursor on database with categories.
 *
 * PARAMETERS:	->	dbP:
 *				->	sql:
 *				->	flags:
 *				->	numCategories:
 *				->	pCategoryIDs:
 *				->	matchMode:
 *				<->	cursorIDP:
 *
 * RETURNED:    	Zero if no error, else the error
 *
 * NOTE:			flags are masked with sCursorFlags.
 *					so we can re-inforce cursor flags.
 *					e.g dbCursorEnableCaching.
 *
 * REVISION HISTORY:
 *	PPL	07/10/03	Initial Revision. Cursors !
 *
 ***********************************************************************/
status_t ApptOpenCursorWithCategory(
	DmOpenRef 			dbP,
	const char* 		sql,
	uint32_t			flags,
	uint32_t			numCategories,
	CategoryID 			pCategoryIDs[],
	DbMatchModeType 	matchMode,
	uint32_t * 			cursorIDP)
{
	uint32_t 			cursorID;
	status_t			err;

	*cursorIDP = dbInvalidCursorID;


	err =  DbCursorOpenWithCategory(
				dbP,
				sql,
				(flags | sCursorFlags),
				numCategories,
	 			pCategoryIDs,
	 			matchMode,
				&cursorID);

	if (err >= errNone)
		*cursorIDP = cursorID;

	return err;
}


/************************************************************************
 *
 * FUNCTION:    	ApptFindFirstNonRepeating
 *
 * DESCRIPTION: 	This routine finds the first appointment on the
 *					specified day.
 *
 * PARAMETERS:  ->	dbP:		Pointer to the database.
 *				->	cursorID:	The cursor.
 *              ->	date:		Date to search for.
 *              ->	rowIDP:		Pointer to the rowId of the first record on the
 *                       		specified day (returned value.)
 *
 * RETURNED:    	true if a record has found.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 *	ART	6/15/95		Initial revision.
 *
 ***********************************************************************/
Boolean ApptFindFirstNonRepeating (DmOpenRef dbP, uint32_t cursorID, DateType date)
{
	status_t 					err;
	uint32_t 					numOfRecords;
	int32_t 					kmin, probe, i;			// all positions in the database.
	int16_t 					result = 0;				// result of comparing two records
	uint32_t 					index;
	Boolean 					found = false;
	uint32_t 					columnIDsArray[] = {	REPEATING_EVENT_COL_ID,
													UNTIMED_EVENT_COL_ID,
													START_EVENT_DATE_TIME_COL_ID};
	uint32_t 					numColumns = sizeof(columnIDsArray)/sizeof(uint32_t);
	DbSchemaColumnValueType* 	columnValuesArray;
	DateTimeType				tmpDateTime;

	// Maybe we should use a cursor with cache disabled
	kmin = probe = 0;
	numOfRecords = DbCursorGetRowCount(cursorID);

	while (numOfRecords > 0)
	{
		i = numOfRecords >> 1;
		probe = kmin + i;
		index = probe;

		// Cursor position is 1-based
		err = DbCursorSetAbsolutePosition(cursorID, index+1);
		if (err < errNone )
			goto ExitNotFound;

		// Get the column values for repeating and start time - Do not check categories here.
		err = DbGetColumnValues(dbP, cursorID, numColumns, columnIDsArray, &columnValuesArray);

		// Unlike the PalmOS 4.0 DB, the schema-based DB is always sorted, index are ranged from 0 to numOfRecords
		ErrNonFatalDisplayIf(err < errNone, "Find failed");

		if (*((Boolean*)columnValuesArray[0].data))
		{
			result = 1;
		}
		else
		{
			// Convert in device time zone the UTC time stored in database
			CnvUTC2LocalTime((time_t*)columnValuesArray[2].data, &tmpDateTime,
				*((Boolean*)columnValuesArray[1].data));
			result = DateCompare (date, CnvDateTime2Date(&tmpDateTime));
		}

		// DB values no more used: release the memory used to access DB columns
		DbReleaseStorage(dbP, columnValuesArray);

		// If the date passed is less than the probe's date, keep searching.
		if (result < 0)
		{
			numOfRecords = i;
		}

		// If the date passed is greater than the probe's date, keep searching.
		else if (result > 0)
		{
			kmin = probe + 1;
			numOfRecords = numOfRecords - i - 1;
		}

		// If the records are equal find the first record on the day.
		else
		{
			found = true;

			while (true)
			{
				// Test the previous position, if failed we were on the first position, just exit
				err = DbCursorMovePrev(cursorID);
				if (err < errNone)
				{
					if (err == dmErrCursorBOF)
						DbCursorMoveFirst(cursorID);
					break;
				}

				// Update index
				DbCursorGetCurrentPosition(cursorID, &index);
				index--; // Cursor is 1-based / index 0-based.

				// Get the column values for repeating and start time
				err = DbGetColumnValues(dbP, cursorID, numColumns, columnIDsArray, &columnValuesArray);

				// Unlike the PalmOS 4.0 DB, the schema-based DB is always sorted, index are sorted from 0 to numOfRecords-1
				ErrNonFatalDisplayIf(err < errNone, "Find failed");

				if (*((Boolean*)columnValuesArray[0].data))
				{
					result = 1;
				}
				else
				{
					// Convert in device time zone the UTC time stored in database
					CnvUTC2LocalTime((time_t*)columnValuesArray[2].data, &tmpDateTime,
						*((Boolean*)columnValuesArray[1].data));
					result = DateCompare (date, CnvDateTime2Date(&tmpDateTime));
				}

				// DB values no more used: release the memory used to access DB columns
				DbReleaseStorage(dbP, columnValuesArray);

				if (result != 0)
				{
					// We moved one event more than needed
					DbCursorMoveNext(cursorID);
					break;
				}
			}
			break;
		}
	}

	// If there was no appointments on the specified day, return the
	// index of the next appointment (on a future day).
	if (! found)
	{
		if (result < 0)
		{
			DbCursorSetAbsolutePosition(cursorID, probe+1);
		}
		else if (DbCursorGetRowCount(cursorID) == 0)
		{
			DbCursorMoveFirst(cursorID);
		}
		else
		{
			DbCursorSetAbsolutePosition(cursorID, probe+2);
		}
	}

	return (found);

ExitNotFound:
	return false;
}


/************************************************************************
 *
 * FUNCTION:    	ApptAddException
 *
 * DESCRIPTION: 	This routine adds an entry to the exceptions list of
 *					the specified record.
 *
 * PARAMETERS: ->	dbP:	database pointer.
 *				->	rowID:	record row id or cursorID.
 *				->	date:	exception date.
 *
 * RETURNED:		error code.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	7/26/95		Initial revision.
 *	PPL	07/10/03 	Cursors!
 *
 ***********************************************************************/
status_t ApptAddException (DmOpenRef dbP, uint32_t rowID, DateType date)
{
	status_t 			error;
	uint32_t 			size;
	ApptDBRecordType 	r;
	DatePtr 			exceptionsP;

	error = ApptGetRecord(dbP, rowID, &r, DBK_SELECT_EXCEPTIONS);
	if (error)
		return error;

	// If the record already has an expections list, add an entry to
	// the list.
	if (r.exceptions.numExceptions > 0)
	{
		// Copy the previous list
		size = (sizeof (DateType) * (r.exceptions.numExceptions+1));

		exceptionsP = MemPtrNew (size);
		ErrFatalErrorIf ((!exceptionsP), "Out of memory");

		memmove(exceptionsP, r.exceptions.exceptionsDatesP, size);
		exceptionsP[r.exceptions.numExceptions] = date;
		r.exceptions.numExceptions++;

		// and free the previous list
		MemPtrFree(r.exceptions.exceptionsDatesP);
	}
	else
	{
		// Create an expections list.
		r.exceptions.numExceptions = 1;

		exceptionsP =  MemPtrNew (sizeof (DateType));
		ErrFatalDisplayIf ((!exceptionsP), "Out of memory");

		exceptionsP[0] = date;
	}

	// Update the record, ApptChangeRecord will free the newly allocated exceptions list
	r.exceptions.exceptionsDatesP = exceptionsP;
	error = ApptChangeRecord (dbP, rowID, &r, DBK_SELECT_EXCEPTIONS);

	return (error);
}


/************************************************************************
 *
 * FUNCTION:    	ApptRemoveExceptions
 *
 * DESCRIPTION: 	This routine removed the exceptions list of the
 *              	specified record.
 *
 * PARAMETERS: 	->	apptRec:	a pointer to an appointment record.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	01/13/03	Initial revision.
 *
 ***********************************************************************/
void ApptRemoveExceptions(ApptDBRecordPtr apptRec)
{
	if (apptRec->exceptions.exceptionsDatesP != NULL)
	{
		MemPtrFree(apptRec->exceptions.exceptionsDatesP);
		apptRec->exceptions.exceptionsDatesP = NULL;
	}
	apptRec->exceptions.numExceptions = 0;
}


/************************************************************************
 *
 * FUNCTION:    	ApptRepeatsOnDate
 *
 * DESCRIPTION: 	This routine returns true if a repeating appointment
 *              	occurrs on the specified date.
 *
 * PARAMETERS:  ->	apptRec:	a pointer to an appointment record.
 *              ->	date:		date to check.
 *
 * RETURNED:    	true if the appointment occurs on the date specified
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/14/95		Initial Revision.
 *
 ***********************************************************************/
Boolean ApptRepeatsOnDate (ApptDBRecordPtr apptRec, DateType date)
{
	uint32_t  i;
	Boolean onDate;
	DatePtr localTimeExceptionsP;

	onDate = RepeatsOnDate(&(apptRec->repeat), apptRec->when.date, date);

	// Check for an exception.
	if (onDate)
	{
		localTimeExceptionsP = apptRec->exceptions.exceptionsDatesP;
		for (i = 0; i < apptRec->exceptions.numExceptions; i++)
		{
			if (DateCompare (date, localTimeExceptionsP[i]) == 0)
			{
				onDate = false;
				break;
			}
		}
	}

	return (onDate);
}


/************************************************************************
 *
 * FUNCTION:    	FindNextRepeat
 *
 * DESCRIPTION: 	This routine computes the date of the next
 *              	occurrence of a repeating appointment.
 *
 * PARAMETERS:  ->	apptRec:		a pointer to an appointment record.
 *
 *              ->	date:
 *						passed:   	date to start from.
 *                      returned: 	date of next occurrence.
 *
 *             	-> 	searchForward:	search for the next occurrence before
 *									or after the specified date
 *
 * RETURNED:    	true if an occurrence was found.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/14/95		Initial Revision.
 *
 ***********************************************************************/
Boolean FindNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward)
{
	int16_t  i;
	int32_t  adjust;
	int32_t  daysTilNext;
	int32_t  monthsTilNext;

	uint32_t day;
	uint32_t freq;
	uint32_t year;
	uint32_t weeksDiff;
	uint32_t monthsDiff;
	uint32_t daysInMonth;
	uint32_t dayOfWeek;
	uint32_t apptWeekDay;
	uint32_t firstDayOfWeek;

	uint32_t startInDays;
	uint32_t dateInDays;
	DateType start;
	DateType date;
	DateType next = {0, 0, 0};


	date = *dateP;

	if (searchForward)
	{
		// Is the date passed after the end date of the appointment?
		if (DateCompare (date, apptRec->repeat.repeatEndDate) > 0)
			return (false);

		// Is the date passed before the start date of the appointment?
		if (DateCompare (date, apptRec->when.date) < 0)
			date = apptRec->when.date;
	}
	else
	{
		// Is the date passed is before the start date of the appointment?
		// return false now
		if (DateCompare (date, apptRec->when.date) < 0)
			return (false);

		// Is the date passed after the end date of the appointment?
		// search backwards from repeat end date for first valid occurrence.
		if (DateCompare (date, apptRec->repeat.repeatEndDate) > 0)
			date = apptRec->repeat.repeatEndDate;
	}

	// apptRec->repeat.repeatEndDate can be passed into this routine
	// or be set in the else case above.  Since apptNoEndDate is not a
	// valid date (month is 15) set it must be set to the last date
	// support by the current OS  12/31/31
	if ( DateToInt(date) == apptNoEndDate)
		date.month = 12;

	// Get the frequency on occurrecne (ex: every 2nd day, every 3rd month, etc).
	freq = apptRec->repeat.repeatFrequency;

	// Get the date of the first occurrence of the appointment.
	start = apptRec->when.date;

	switch (apptRec->repeat.repeatType)
	{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);

			if (searchForward)
				daysTilNext = (dateInDays - startInDays + freq - 1) / freq * freq;
			else
				daysTilNext = (dateInDays - startInDays) / freq * freq;

			if (startInDays + daysTilNext > (uint32_t) maxDays)
				return (false);

			DateDaysToDate (startInDays + daysTilNext, &next);
			break;



		// Weekly repeating appointment (ex: every Monday and Friday).
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
		{
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);

			firstDayOfWeek = (uint32_t)((DayOfWeek (1, 1, firstYear)
							- apptRec->repeat.repeatStartOfWeek + daysInWeek) % daysInWeek);

			dayOfWeek = DayOfWeek (date.month, date.day, (int16_t)(date.year + firstYear));
			apptWeekDay = (dayOfWeek - apptRec->repeat.repeatStartOfWeek
							+ daysInWeek) % daysInWeek;

			// Are we in a week in which the appointment occurrs, if not
			// move to that start of the next week in which the appointment
			// does occur.
			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) - ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;

			if (weeksDiff)
			{
				if (searchForward)
				{
					adjust = ((freq - weeksDiff) * daysInWeek) - apptWeekDay;
					apptWeekDay = 0;
					dayOfWeek = (dayOfWeek + adjust) % daysInWeek;
				}
				else
				{
					adjust = (weeksDiff * daysInWeek) - ( (daysInWeek-1) - apptWeekDay);
					apptWeekDay = 6;
					dayOfWeek = (dayOfWeek - (adjust % daysInWeek) + daysInWeek) % daysInWeek;
				}
			}
			else
			{
				adjust = 0;
			}

			// Find the next day on which the appointment repeats.
			if (searchForward)
			{
				for (i = 0; i < daysInWeek; i++)
				{
					if (apptRec->repeat.repeatOn & (1 << dayOfWeek))
					{
						break;
					}

					adjust++;

					if (++dayOfWeek == daysInWeek)
					{
						dayOfWeek = 0;
					}

					if (++apptWeekDay == daysInWeek)
					{
						adjust += (freq - 1) * daysInWeek;
					}
				}


				if (dateInDays + adjust > (uint32_t) maxDays)
				{
					return (false);
				}
				DateDaysToDate (dateInDays + adjust, &next);
			}
			else
			{
				for (i = 0; i < daysInWeek; i++)
				{
					if (apptRec->repeat.repeatOn & (1 << dayOfWeek))
					{
						break;
					}
					adjust++;

					if (dayOfWeek == 0)
					{
						dayOfWeek = daysInWeek-1;
					}
					else
					{
						dayOfWeek--;
					}

					if (apptWeekDay == 0)
					{
						adjust += (freq - 1) * daysInWeek;
						apptWeekDay = daysInWeek-1;
					}
					else
					{
						apptWeekDay --;
					}
				}

				// determine if date goes past first day (unsigned int wraps around)
				if (dateInDays - adjust > dateInDays)
				{
					return (false);
				}

				DateDaysToDate (dateInDays - adjust, &next);
			}

			break;
		}


		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Compute the number of month until the appointment repeats again.
			if (searchForward)
				monthsTilNext = ((uint32_t)(((date.year - start.year) * monthsInYear) + (date.month - start.month)) + freq - 1) /freq * freq;
			else
				monthsTilNext = (uint32_t)(((date.year - start.year) * monthsInYear) + (date.month - start.month)) /freq * freq;

			while (true)
			{
				year = start.year + ((uint32_t)(start.month - 1) + monthsTilNext) / monthsInYear;
				if (year >= numberOfYears)
					return (false);

				next.year = (uint16_t) year;
				next.month = (uint16_t)(((uint32_t)(start.month - 1) + monthsTilNext) % monthsInYear + 1);

				dayOfWeek = DayOfWeek (next.month, 1, (int16_t)(next.year + firstYear));
				if ((apptRec->repeat.repeatOn % daysInWeek) >= (uint16_t) dayOfWeek)
					day = apptRec->repeat.repeatOn - dayOfWeek + 1;
				else
					day = (uint32_t)(apptRec->repeat.repeatOn + daysInWeek) - dayOfWeek + 1;

				//  If repeat-on day is between the last sunday and the last
				// saturday, make sure we're not passed the end of the month.
				if ((apptRec->repeat.repeatOn >= domLastSun)
  					&& ((uint16_t) day > DaysInMonth (next.month, (uint16_t) (next.year+firstYear))))
				{
					day -= daysInWeek;
				}

				next.day = (uint16_t) day;

				// Its posible that "next date" calculated above is
				// before the date passed.  If so, move forward
				// by the length of the repeat freguency and perform
				// the calculation again.
				if (searchForward)
				{
					if ( DateToInt(date) > DateToInt (next))
						monthsTilNext += freq;
					else
						break;
				}
				else
				{
					if ( DateToInt(date) < DateToInt (next))
						monthsTilNext -= freq;
					else
						break;
				}
			}
			break;



		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Compute the number of month until the appointment repeats again.
			monthsDiff = (uint32_t)(((date.year - start.year) * monthsInYear) + (date.month - start.month));
			if (searchForward)
			{
				monthsTilNext = (monthsDiff + freq - 1) / freq * freq;
				if ((date.day > start.day) && (!(monthsDiff % freq)))
					monthsTilNext += freq;
			}
			else
			{
				monthsTilNext = monthsDiff / freq * freq;
				if ((date.day < start.day) && (!(monthsDiff % freq)))
					monthsTilNext -= freq;
			}

			year = start.year + ((uint32_t)(start.month - 1) + monthsTilNext) / monthsInYear;
			if (year >= numberOfYears)
				return (false);

			next.year = (uint16_t) year;
			next.month = (uint16_t) (((uint32_t)(start.month - 1) + monthsTilNext) % monthsInYear + 1);
			next.day = start.day;

			// Make sure we're not passed the last day of the month.
			daysInMonth = DaysInMonth (next.month, (int16_t)(next.year + firstYear));
			if (next.day > ((uint16_t) daysInMonth))
				next.day = (uint16_t) daysInMonth;
			break;


		// Yearly repeating appointment.
		case repeatYearly:
			next.day = start.day;
			next.month = start.month;

			if (searchForward)
			{
				year = start.year + (((uint32_t)(date.year - start.year) + freq - 1) / freq * freq);
				if (	(date.month > start.month)
					|| ((date.month == start.month) && (date.day > start.day)) )
					year += freq;
			}
			else
			{
				year = start.year + ((uint32_t)(date.year - start.year) / freq * freq);
				if (	(date.month < start.month)
					|| ((date.month == start.month) && (date.day < start.day)) )
					year -= freq;
			}


			// Specal leap day processing.
			if ( (next.month == february) && (next.day == 29)
				&& (next.day > DaysInMonth (next.month, (uint16_t) (year+firstYear))))
			{
				next.day = DaysInMonth (next.month,(uint16_t) ( year+firstYear));
			}
			if (year >= numberOfYears)
				return (false);

			next.year = (uint16_t) year;
			break;
		}

	if (searchForward)
	{
		// Is the next occurrence after the end date of the appointment?
		if (DateCompare (next, apptRec->repeat.repeatEndDate) > 0)
			return (false);

		ErrFatalDisplayIf ((DateToInt (next) < DateToInt (*dateP)),
			"Calculation error");
	}
	else
	{
		// Is the next occurrence before the start date of the appointment?
		if (DateCompare (next, apptRec->when.date) < 0)
			return (false);
	}

	*dateP = next;
	return (true);
}


/************************************************************************
 *
 * FUNCTION:    	IsException
 *
 * DESCRIPTION: 	This routine returns true the date passed is in a
 *              	repeating appointment's exception list.
 *
 * PARAMETERS:  ->	apptRec:	a pointer to an appointment record.
 *              ->	date:		date to check.
 *
 * RETURNED:    	true if the date is an exception date.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/14/95		Initial revision.
 *
 ***********************************************************************/
static Boolean IsException (ApptDBRecordPtr apptRec, DateType date)
{
	uint32_t i;
	DatePtr localTimeExceptionsP;

	localTimeExceptionsP = apptRec->exceptions.exceptionsDatesP;
	for (i = 0; i < apptRec->exceptions.numExceptions; i++)
	{
		if (DateCompare (date, localTimeExceptionsP[i]) == 0)
		return (true);
	}

	return (false);
}


/************************************************************************
 *
 * FUNCTION:    	ApptNextRepeat
 *
 * DESCRIPTION:		This routine computes the next occurrence of a
 *              	repeating appointment.
 *
 * PARAMETERS:	->	apptRec:		a pointer to an appointment record.
 *
 *              ->	dateP:
 *						passed:   	date to start from
 *                    	returned: 	date of next occurrence .
 *
 *				->	searchForward:	true if searching for next occurrence.
 *									false if searching for most recent.
 *
 * RETURNED:		true if there is an occurrence of the appointment
 *					between the date passed and the appointment's end date
 *					(if searching forward) or start date (if searching
 *					backwards.)
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/20/95		Initial revision.
 *	GAP	9/25/00		Add capability to search backwards for the most
 *					recent occurrence of the event (needed for attention
 *					manager support)
 *
 ***********************************************************************/
Boolean ApptNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward)
{
	DateType date;

	date = *dateP;

	while (true)
	{
		// Compute the next time the appointment repeats.
		if (! FindNextRepeat (apptRec, &date, searchForward))
			return (false);

		// Check if the date computed is in the exceptions list.
		if (! IsException (apptRec, date))
		{
			*dateP = date;
			return (true);
		}

		DateAdjust (&date, (searchForward) ? 1 : -1);
	}
}


/************************************************************************
 *
 * FUNCTION:    	UnDayOfMonth
 *
 * DESCRIPTION: 	Inverse of DayOfMonth routine.
 *					Takes a month and year and a dayOfMonth value
 *					(e.g., dom1stSun, domLastFri, etc.) and computes
 *					what date that day is for that month.
 *
 * PARAMETERS:	->	month:		a month.
 *				->	year:		a year (1904, etc.)
 *				->	dayOfMonth:	a dayOfMonth, like those returned from
 *								DayOfMonth
 *
 * RETURNED:    	date in month.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GLO	3/1/99		Initial revision.
 *
 ***********************************************************************/
static uint16_t UnDayOfMonth(uint16_t month, uint16_t year, uint16_t dayOfMonth)
{
	int16_t dayOfWeek;
	int16_t firstDayOfWeek;
	int16_t week;
	int16_t day;

	dayOfWeek = dayOfMonth % daysInWeek;
	week = dayOfMonth / daysInWeek;

	firstDayOfWeek = DayOfWeek(month, 1, year);
	day = (dayOfWeek - firstDayOfWeek + daysInWeek) % daysInWeek + 1 + week * daysInWeek;

	// adjust for last-fooday in months with only 4 foodays
	while (day > DaysInMonth(month, year))
	{
		day -= daysInWeek;
	}

	return day;
}


/************************************************************************
 *
 * FUNCTION:    	CountTotalRepeats
 *
 * DESCRIPTION: 	Counts the total number of times a repeating event
 *					occurs.
 *
 *					Returns apptNoEndDate (-1) if the event has no end
 *					date (repeats forever).
 *
 *					The default value returned is 1, since an appointment
 *					that repeats on 0 days is not allowed.
 *
 * PARAMETERS:	->	apptRecP:	pointer to an appointment record
 *
 * RETURNED:		number of times the appointment repeats.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GLO	3/1/99		Initial revision.
 *
 ***********************************************************************/
static int32_t CountTotalRepeats(ApptDBRecordPtr apptRecP)
{
	DateType start;
	DateType end;

	uint32_t daysTotal;
	uint32_t freq;
	uint32_t weeks;
	uint32_t months;
	uint32_t years;
	uint32_t dayOfWeek;
	uint32_t startDOW;
	uint32_t endDOW;
	uint32_t daycount;
	uint32_t repeatOnDay;

	uint32_t startInDays;
	uint32_t endInDays;
	uint32_t firstSunday;
	uint32_t lastSaturday;

	ErrFatalErrorIf(apptRecP == NULL, "no appointment");
	ErrFatalErrorIf(apptRecP->repeat.repeatType == repeatNone, "appointment does not repeat");
	ErrFatalErrorIf(apptRecP->repeat.repeatFrequency == 0, "zero repeat frequency");

	// Get the frequency of occurrence (ex: every 2nd day, every 3rd month, etc).
	freq = apptRecP->repeat.repeatFrequency;

	// Get the date of the first occurrence of the appointment.
	start = apptRecP->when.date;

	// Get the date of the last occurrence of the appointment.
	end = apptRecP->repeat.repeatEndDate;

	// Does the appointment repeat forever?
	if (DateToInt(end) == apptNoEndDate)
		return (apptNoEndDate);

	// if the end date is somehow before the start date, just return 1
	if (DateCompare(end, start) < 0)
		return 1;

	daysTotal = 0;

	switch (apptRecP->repeat.repeatType)
	{
		// Daily repeating appointment.
		case repeatDaily:
			startInDays = DateToDays(start);
			endInDays = DateToDays(end);
			daysTotal = ((endInDays - startInDays) / freq) + 1;
			break;

		// Weekly repeating appointment (ex: every Monday and Friday).
		// The strategy is to break the time period into 3 fragments that
		// are easily dealt with - days before the first sunday, whole weeks
		// from the first sunday to the last saturday, and days after the
		// last saturday.
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			startInDays = DateToDays(start);
			endInDays = DateToDays(end);
			startDOW = DayOfWeek(start.month, start.day, (int16_t)(start.year + firstYear));
			endDOW = DayOfWeek(end.month, end.day, (int16_t)(end.year + firstYear));

			// find firstSunday and lastSaturday
			if (startDOW != sunday)
			{
				firstSunday = startInDays - startDOW + daysInWeek;
			}
			else
			{
				firstSunday = startInDays;
			}

			if (endDOW != saturday)
			{
				lastSaturday = endInDays - endDOW - 1;
			}
			else
			{
				lastSaturday = endInDays;
			}

			// compute number of full sunday-to-saturday weeks
			weeks = (lastSaturday - firstSunday + 1) / daysInWeek;

			// count number of times appt repeats in a full week
			daycount = 0;
			for (dayOfWeek = sunday; dayOfWeek < daysInWeek; dayOfWeek++)
			{
				//if repeat on dayOfWeek, daycount++
				if (RepeatOnDOW(&apptRecP->repeat, dayOfWeek))
					daycount++;
			}

			// Now we are ready to total the repetitions.
			daysTotal = 0;
			// fragment 1 - before firstSunday
			if (startDOW != sunday)
			{
				for (dayOfWeek = startDOW; dayOfWeek < daysInWeek; dayOfWeek++)
				{
					// if repeat on dayOfWeek, daysTotal++
					if (RepeatOnDOW(&apptRecP->repeat, dayOfWeek))
						daysTotal++;
				}
			}

			// fragment 2 - full weeks from firstSunday to lastSaturday
			daysTotal += (daycount * (weeks / freq));

			// fragment 3 - after lastSaturday
			if (endDOW != saturday)
			{
				for (dayOfWeek = sunday; dayOfWeek <= endDOW; dayOfWeek++)
				{
					// if repeat of dayOfWeek, daysTotal++
					if (RepeatOnDOW(&apptRecP->repeat, dayOfWeek))
						daysTotal++;
				}
			}
			break;


		// Monthly-by-day repeating appointment
		case repeatMonthlyByDay:
			// Compute the number of months
			months = (uint32_t)(((end.year - start.year) * monthsInYear) + (end.month - start.month));

			// if the end day is too early in the last month, don't include that month
			repeatOnDay = UnDayOfMonth(end.month, (uint16_t)(end.year + firstYear),
												apptRecP->repeat.repeatOn);
			if (end.day < repeatOnDay)
				months--;

			daysTotal = months / freq + 1;	// repeats once every freq months
			break;


		// Monthly-by-date repeating appointment
		case repeatMonthlyByDate:
			// Compute the number of months
			months = (uint32_t)(((end.year - start.year) * monthsInYear) + (end.month - start.month));

			// if the end day is too early in the last month, don't include that month
			if (end.day < start.day)
				months--;

			daysTotal = months / freq + 1;	// repeats once every freq months
			break;


		// Yearly repeating appointment.
		case repeatYearly:
			years = (uint32_t)(end.year - start.year);

			// if the end day is too early in the last year, don't include that year
			if (end.month < start.month
			|| (end.month == start.month && end.day < start.day))
				years--;

			daysTotal = years / freq + 1;		// repeats once every freq years
			break;

		default:
			daysTotal = 1;
			break;
		}

	DbgOnlyFatalErrorIf(daysTotal == 0, "event repeats on 0 days");
	DbgOnlyFatalErrorIf( ((int32_t) daysTotal) < 0, "event repeats on negative days");

	if ( ((int32_t) (daysTotal)) <= 0)
		daysTotal = 1;

	return (daysTotal);
}

/************************************************************************
 *
 * FUNCTION:    	ApptCountMultipleOccurences
 *
 * DESCRIPTION: 	Return the number of evenment not in the exception list
 *					for a record.
 *
 * PARAMETERS: 	 -> apptRecP:	the appointment to examine.
 *
 * RETURNED:    	0: It's not a repeating event
 *					n: number of repeating event not in the execption list
 *					-1: repeating event with no end date.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe	7/6/04		Initial revision.
 ***********************************************************************/
int32_t ApptCountMultipleOccurences(ApptDBRecordPtr apptRecP)
{
	int32_t totalRepeats;
	int32_t numExceptions;

	ErrFatalErrorIf(apptRecP == NULL, "no appointment");

	// if the appointment does not repeat, then it can't occur more than once
	if (apptRecP->repeat.repeatType == repeatNone)
		return 0;

	totalRepeats = CountTotalRepeats(apptRecP);
	numExceptions = apptRecP->exceptions.numExceptions;

	if (totalRepeats == apptNoEndDate)
		return apptNoEndDate;

	return totalRepeats - numExceptions;
}


/************************************************************************
 *
 * FUNCTION:    	ApptHasMultipleOccurences
 *
 * DESCRIPTION: 	Does the given appointment occur more than once.
 *
 *					This function compares the repeat info and the
 *					exception list for an appointment to determine
 *					if it has more than one visible (non-excepted)
 *					occurence.
 *					The decision is based solely on the number of times
 *					the appointmentrepeats versus the number of exceptions.
 *
 * PARAMETERS: 	 -> apptRecP:	the appointment to examine.
 *
 * RETURNED:    	true if the appointment occurs more than once
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GLO	3/2/99		Initial revision.
 *	PPL 06/24/04	A repeating event still repeats until the diff
 *					(totalRepeats - numExceptions) is  superior
 *					or equal to 1. Adds the "equal" in the test.
 ***********************************************************************/
Boolean ApptHasMultipleOccurences(ApptDBRecordPtr apptRecP)
{
/*	int32_t totalRepeats;
	int32_t numExceptions;

	ErrFatalErrorIf(apptRecP == NULL, "no appointment");

	// if the appointment does not repeat, then it can't occur more than once
	if (apptRecP->repeat.repeatType == repeatNone)
		return false;

	totalRepeats = CountTotalRepeats(apptRecP);
	numExceptions = apptRecP->exceptions.numExceptions;

	if (totalRepeats == apptNoEndDate)
		return true;

	if ((totalRepeats - numExceptions) >= 1)
		return true;

	return false;
*/
	// 0 => It's not a repeating event or event is composed only with exception;
	return (ApptCountMultipleOccurences(apptRecP) != 0);
}


/************************************************************************
 *
 * FUNCTION:   		ApptGetAlarmTime
 *
 * DESCRIPTION: 	This routine determines the date and time of an alarm
 *					for the event passed.  Depending on the search direction
 *					specified, it will return either the time of the next
 *					occurrence of the alarm to fire, or the time of the most
 *					recently triggered alarm.
 *
 * PARAMETERS:  ->	apptRec:		pointer to an appointment record
 *              ->	currentTime:	current date and time in seconds
 *				->	searchForward:	designates whether to find the next
 *									(true) or most recent (false) occurrence
 *									of an event.
 *
 * RETURNED:    	date and time of the alarm, in seconds,
 *					or zero if there is no alarm
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/20/95		Initial revision.
 *	GAP	9/25/00		Add capability to search backwards for the most
 *					recent occurrence of the event (needed for attention
 *					manager support.)
 *	GAP	10/17/00	Small optimization - removed advance calculation
 *					out of while loop as we only need to do this once.
 *					also, add advance to current time in order to
 *					correctly position the start of a backward search in
 *					order to take into account the advance time.
 *
  ***********************************************************************/
uint32_t ApptGetAlarmTime (ApptDBRecordPtr apptRec, uint32_t currentTime, Boolean searchForward)
{
	uint32_t		advance = 0;
	uint32_t		alarmTime, apptTime;
	DateType		repeatDate;
	DateTimeType	curDateTime;
	DateTimeType	apptDateTime;

	if (apptRec->alarm.advance == apptNoAlarm)
		return 0;

	// Non-repeating appointment?
	if (apptRec->repeat.repeatType == repeatNone)
	{
		// An alarm on an untimed event triggers at midnight.
		if (apptRec->when.noTimeEvent)
			CnvDate2DateTime(&apptRec->when.date, NULL, &apptDateTime);
		else
			CnvDate2DateTime(&apptRec->when.date, &apptRec->when.startTime, &apptDateTime);

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

		apptTime = TimDateTimeToSeconds (&apptDateTime);
		alarmTime = apptTime - advance;

		if (searchForward)
		{
			if (alarmTime >= currentTime)
				return (alarmTime);
			else
				return (0);
		}
		else
		{
			if (alarmTime <= currentTime)
				return (alarmTime);
			else
				return (0);
		}
	}


	// Repeating appointment.

	// calculate the appointment alarm advance time.
	switch (apptRec->alarm.advanceUnit)
	{
		case aauMinutes:
			advance = (uint32_t) apptRec->alarm.advance * minutesInSeconds;
			break;
		case aauHours:
			advance = (uint32_t) apptRec->alarm.advance * hoursInSeconds;
			break;
		case aauDays:
			advance = (uint32_t) apptRec->alarm.advance * daysInSeconds;
			break;
	}

	// if searchin backwards, adjust the start point of
	// the search to account for the alarm advance time.
	if (!searchForward)
		TimSecondsToDateTime (currentTime+advance, &curDateTime);
	else
		TimSecondsToDateTime (currentTime, &curDateTime);

	repeatDate = CnvDateTime2Date(&curDateTime);
	while (ApptNextRepeat (apptRec, &repeatDate, searchForward))
	{
		// An alarm on an untimed event triggers at midnight.

		if (apptRec->when.noTimeEvent)
			CnvDate2DateTime(&repeatDate, NULL, &apptDateTime);
		else
			CnvDate2DateTime(&repeatDate, &apptRec->when.startTime, &apptDateTime);

		// Compute the time of the alarm by adjusting the date and time
		// of the appointment by the length of the advance notice.
		alarmTime = TimDateTimeToSeconds (&apptDateTime) - advance;

		if (searchForward)
		{
			if (alarmTime >= currentTime)
				return (alarmTime);

			DateAdjust (&repeatDate, 1);
		}
		else
		{
			if (alarmTime <= currentTime)
				return (alarmTime);

			DateAdjust (&repeatDate, -1);
		}
	}

	return (0);
}


/************************************************************************
 *
 * FUNCTION:    	ApptAlarmMunge
 *
 * DESCRIPTION: 	Helper routine for ApptAlarmMunger.
 *					Process one appointment.
 *
 * PARAMETERS:  ->	apptRecP:				The record to analyse.
 *				-> 	inRowID:				rowID or cursor ID.
 *				-> 	seekAlarmStart:
 *				->	postAlarmTime:
 *				->	inPostingAlarmData:
 *				<->	inOutEarliestAlarmP:
 *				->	rowIDofEarliestAlarmP:	rowID of the alarm matching
 *											to *inOutEarliestAlarmP.
 *
 *				Needs:
 *					DBK_SELECT_TIME
 *					| DBK_SELECT_ALARM
 *					| DBK_SELECT_REPEAT
 *					| DBK_SELECT_EXCEPTIONS
 *
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:

 *	PES	3/22/00		Initial revision.
 *					Based on existing code from ApptAlarmMunger.
 *	GAP	06/06/00	Only increment the alarm counter variable (numAlarms)
 *					when a new alarm is added to the alarms as opposed to
 *					every time an alarm is processed.  Can result in a
 *					Fatal Alert "Error Querying Record" if count does not
 *					match the number of alarms in list and just the right
 *					value happens to reside in the memory following the
 *					actual list contents.
 *	TES	06/06/02	Added QuietMode support.
 *	PPL	07/11/03	Cursors !
 *
 ***********************************************************************/
static void ApptAlarmMunge (
	ApptDBRecordType * 		apptRecP,
	uint32_t	 			inRowID,
	uint32_t 				seekAlarmStart,
	uint32_t 				postAlarmTime,
	AlarmPostingDataPtr 	inPostingAlarmData,
	uint32_t *				inOutEarliestAlarmP,
	uint32_t * 				rowIDofEarliestAlarmP)
{
	uint32_t				alarmTime;
	AttnLevelType 			attnLevel;
	AttnFlagsType 			attnFlags;

	if (inPostingAlarmData)
	{
		// Get the first alarm on or after postAlarmTime
		alarmTime = ApptGetAlarmTime (apptRecP, postAlarmTime, true);

		// If in range, add the alarm to the output
		if (alarmTime && (alarmTime == postAlarmTime))
		{
			// remove it from the attention manager list in the event if it is already posted
			AttnForgetIt(inPostingAlarmData->dbID, inRowID);

			// determine if it is an untimed event. untimed events will be subtle
			if (apptRecP->when.noTimeEvent)
			{
				attnLevel = kAttnLevelSubtle;
				attnFlags = kAttnFlagsNothing;
			}
			else
			{
				attnLevel = kAttnLevelInsistent;
				attnFlags = kAttnFlagsUseUserSettings;	// Use all effects (sound, LED & vibrate) user has enabled via general prefs
			}

			// post the note's alarm to the attention manager queue
			AttnGetAttention(inPostingAlarmData->dbID,
								inRowID,
								attnLevel,
								attnFlags,
								inPostingAlarmData->prefs.alarmSoundRepeatInterval,
								inPostingAlarmData->prefs.alarmSoundRepeatCount);
		}
	}

	// Get the first alarm on or after postAlarmTime
	alarmTime = ApptGetAlarmTime (apptRecP, seekAlarmStart, true);

	// If in range, add the alarm to the output
	if (alarmTime && (alarmTime >= seekAlarmStart))
	{
		// Remember the earliest in-range alarm for our return value
		if ( (alarmTime < *inOutEarliestAlarmP) || (*inOutEarliestAlarmP == 0) )
		{
			*inOutEarliestAlarmP = alarmTime;
			if (rowIDofEarliestAlarmP != NULL)
				*rowIDofEarliestAlarmP = inRowID;
		}
	}
}


/************************************************************************
 *
 * FUNCTION:    	ApptAlarmMunger
 *
 * DESCRIPTION: 	Finds all appointments with alarms within a given range
 *					of time.
 *
 * PARAMETERS:  ->	dbR:					reference to open database
 *				->	seekAlarmStart:			opened cursor
 *				->	postAlarmTime:			first valid alarm time
 *				->	postAlarmsInRange:		last valid alarm time
 *				->	rowIDofEarliestAlarmP:
 *
 * RETURNED:    	time of the first alarm within range
 *
 * NOTE:	 		To find all alarms at a specific time, set alarmStart
 *				 	and alarmStop to the same time.
 *
 *					To find the just the time of the next alarm, set
 *					alarmStart to now+1, alarmStop to max, and the three
 *					output parameters to nil.
 *
 * REVISION HISTORY:
 *	RBB	4/20/99		Initial Revision. Based on existing code from
 *					ApptGetTimeOfNextAlarm and ApptGetAlarmsList.
 *	PES	3/22/00		Pulled out guts into ApptAlarmMunge and replaced
 *					simple loop with optimized pair of loops.
 *
 ***********************************************************************/
static uint32_t ApptAlarmMunger (
	DmOpenRef 				inDbR,
	uint32_t 				seekAlarmStart,
	uint32_t 				postAlarmTime,
	Boolean 				postAlarmsInRange,
	uint32_t* 				rowIDofEarliestAlarmP)
{
	status_t				err;
	uint32_t				earliestAlarm = 0;
	ApptDBRecordType		apptRec;
	AlarmPostingData		postingData;
	AlarmPostingDataPtr		postingDataPtr;
	uint32_t				alarmCursorID;
	uint32_t				currentRowID;
	char 					alarmDBsqlQuery[kSqlQueryMaxSize];
	time_t					dbStartRange;
	DateTimeType 			tmpDateTime;


	// If alarms found in range are to be posted to the attention manager cache all
	// the information that is the same for all alarms to be posted.
	if (postAlarmsInRange)
	{
		postingData.dbP = inDbR;

		// Load Date Book's prefs so we can get the user-specified alarm sound and nag information
		DatebookLoadPrefs(&(postingData.prefs));

		// get the card number & dataBase ID for the app
		postingData.dbID = gApplicationDbID;
		postingDataPtr = &postingData;
	}
	else
		postingDataPtr = NULL;

	// Get the UTC range matching inAlarmStart / inAlarmStop for the WHERE clause
	TimSecondsToDateTime(seekAlarmStart, &tmpDateTime);
	CnvLocalTime2UTC(&tmpDateTime, &dbStartRange, false);

	// Untimed events are stored UTC in database, so enlarge the range by 13 hours
	// (GMT+13 - GMT-13 is the maximum range of timezones shift).
	dbStartRange -= kMaxRangeShiftingDueToTZ;

	// build Sql Request for Alarms Munger
	ApptBuildSQLRequestForAlarmMunger(alarmDBsqlQuery, dbStartRange);

	// Create a cursor having only repeated events
	err = ApptOpenCursor(inDbR, alarmDBsqlQuery, sCursorFlags, &alarmCursorID);
	ErrNonFatalDisplayIf(err < errNone, "Alarm scheduling: error in cursor creation.");

	// Iterate on events
	for (DbCursorMoveFirst(alarmCursorID) ; !DbCursorIsEOF(alarmCursorID) ; DbCursorMoveNext(alarmCursorID) )
	{
		// Skip if alarm not defined
		if (!ApptEventAlarmIsSet(inDbR, alarmCursorID))
			continue;

		// Select columns to read
		if (ApptEventRepeatIsSet(inDbR, alarmCursorID))
			ApptGetRecord(inDbR, alarmCursorID, &apptRec,
					DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);
		else
			ApptGetRecord(inDbR, alarmCursorID, &apptRec,
					DBK_SELECT_TIME | DBK_SELECT_ALARM);

		DbCursorGetCurrentRowID(alarmCursorID, &currentRowID);
		ApptAlarmMunge (&apptRec, currentRowID, seekAlarmStart, postAlarmTime, postingDataPtr, &earliestAlarm, rowIDofEarliestAlarmP);
		ApptFreeRecordMemory (&apptRec);
	}

	ApptCloseCursor(&alarmCursorID);

	return earliestAlarm;
}


/************************************************************************
 * FUNCTION:    	ApptGetTimeOfNextAlarm
 *
 * DESCRIPTION: 	This routine determines the time of the next scheduled
 *              	alarm.
 *
 * PARAMETERS:  ->	dbP:			Pointer to the database.
 *				->	cursorID:		Opened cursor.
 *              ->	timeInSeconds:	Time to search forward from.
 *
 * RETURNED:    	time of the next alarm, in seconds from 1/1/1904, or
 *              	zero if there are no alarms scheduled.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/20/95		Initial revision.
 *	RBB	4/20/99		Rewritten to use new ApptAlarmMunger.
 *	GAP	9/28/00		Rewritten to remove snooze & alarm internals
 *					replaced by attention manager.
 *
 ***********************************************************************/
uint32_t ApptGetTimeOfNextAlarm (DmOpenRef inDbR, uint32_t inAlarmStart, uint32_t * rowIDofEarliestAlarmP)
{
	uint32_t	roundedStart;
	uint32_t	nextAlarm;

	roundedStart = inAlarmStart - (inAlarmStart % minutesInSeconds);
	if (roundedStart != inAlarmStart)
	{
		roundedStart += minutesInSeconds;
	}

	nextAlarm = ApptAlarmMunger (inDbR, roundedStart, 0, false, rowIDofEarliestAlarmP);

	return nextAlarm;
}


/************************************************************************
 * FUNCTION:    	ApptPostTriggeredAlarmsAndGetNext
 *
 * DESCRIPTION: 	This routine posts all events that have an alarm
 *					matching the postAlarmTime time to the attention
 *					manager. Moreover, it returns the next alarm following
 *					getNextStartTime.
 *
 * PARAMETERS:  ->	inDbR:					Pointer to the database.
 *				->	postAlarmTime:			Time of alarm that triggered.
 *				->	getNextStartTime:
 *            	->	rowIDofEarliestAlarmP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GAP	9/28/00		Initial revision.
 *
 ***********************************************************************/
uint32_t ApptPostTriggeredAlarmsAndGetNext (
	DmOpenRef 	inDbR,
	uint32_t 	postAlarmTime,
	uint32_t 	getNextStartTime,
	uint32_t* 	rowIDofEarliestAlarmP)
{
	uint32_t	roundedGetNextStart;
	uint32_t	nextAlarm;

	roundedGetNextStart = getNextStartTime - (getNextStartTime % minutesInSeconds);
	if (roundedGetNextStart != getNextStartTime)
	{
		roundedGetNextStart += minutesInSeconds;
	}

	nextAlarm = ApptAlarmMunger (inDbR, roundedGetNextStart, postAlarmTime, true, rowIDofEarliestAlarmP);

	return nextAlarm;
}


/************************************************************************
 * FUNCTION:    	AddAppointmentToList
 *
 * DESCRIPTION: 	This routine adds an appointment to a list of
 *					appointments.
 *
 * PARAMETERS:  ->	apptListH:			Pointer to list to add appointment
 *										to.
 *				->	count:				Number of appointments on the
 *										specified day.
 *				->	startTime:			Start time of appointment.
 *				->	startTimeInPrevDay: when in next day.
 *				->	endTime:			End time of appointment.
 *				->	endTimeInNextDay:	when in next day.
 *				->	overlapState:		Does the event overlaps over midnight?
 *				->	noTimeEvent:		is it an untimed event record?
 *				->	cursorID:			cursor ID.
 *
 * RETURNED:    	True if successful, false if error.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/17/96		Initial revision.
 *	RBB	4/27/99		Patched error detection when allocating apptListH.
 *
 ***********************************************************************/
static void AddAppointmentToList (
	MemHandle* 					apptListH,
	int32_t *					count,
	TimeType					startTime,
	TimeType 					startTimeInPrevDay,
	TimeType 					endTime,
	TimeType 					endTimeInNextDay,
	MidnightOverlapStateType 	overlapState,
	Boolean 					noTimeEvent,
	uint32_t 					cursorID)
{
	status_t 					err;
	int32_t 					newSize;
	ApptInfoPtr 				apptList;
	uint32_t 					rowID;

	if (*count == 0)
	{
		// Allocated a block to hold the appointment list.
		*apptListH = MemHandleNew (sizeof (ApptInfoType) * (apptMaxPerDay / 10));
		ErrFatalErrorIf(!*apptListH, "Out of memory");
		apptList = MemHandleLock (*apptListH);
	}

	// Resize the list to hold more more appointments.
	else if ((*count % (apptMaxPerDay / 10)) == 0)
	{
		if (*count + (apptMaxPerDay / 10) > apptMaxPerDay)
			return;

		newSize = sizeof (ApptInfoType) * (*count + (apptMaxPerDay / 10));
		err = MemHandleResize (*apptListH, newSize);
		apptList = MemHandleLock (*apptListH);
		ErrFatalErrorIf(err < errNone, "Out of memory");
	}

	else
	{
		apptList = MemHandleLock(*apptListH);
	}

	// Get rowID instead of cursorID
	DbCursorGetCurrentRowID(cursorID, &rowID);

	apptList[*count].startTime = startTime;
	apptList[*count].startTimeInPrevDay = startTimeInPrevDay;
	apptList[*count].endTime = endTime;
	apptList[*count].latestEndTimeInNextDay = endTimeInNextDay;
	apptList[*count].rowID = rowID;
	apptList[*count].overlapState = overlapState;
	apptList[*count].noTimeEvent = noTimeEvent;
	apptList[*count].belongingToCurrCat = true;

	MemHandleUnlock(*apptListH);

	*count+= 1;
}


/************************************************************************
 *
 * FUNCTION:    	ApptListCompare
 *
 * DESCRIPTION: 	This routine compares two entries in the appointment
 *					list, it's called by ApptGetAppointments via the quick
 *					sort routine.
 *
 * PARAMETERS:  ->	a:		A pointer to an entry in the appointment list
 *				->	b:		Pointer to an entry in the appointment list
 *              ->	extra:	Extra data passed to quick sort - not used
 *
 * RETURNED:   	 	if a1 > a2  returns a positive int
 *              	if a1 < a2  returns a negative int
 *              	if a1 = a2  returns zero
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/15/95		Initial Revision.
 *
 ***********************************************************************/
static int16_t ApptListCompare (ApptInfoPtr a1, ApptInfoPtr  a2, int32_t extra)
{
	int16_t result;

	if (a1->noTimeEvent)
	{
		if (a2->noTimeEvent)
			return 0;
		else if (a2->overlapState == overlapScndPart)
			return +1;
		else
			return -1;
	}

	if (a2->noTimeEvent)
	{
		if (a1->overlapState == overlapScndPart)
			return -1;
		else
			return +1;
	}

	result = TimeCompare (a1->startTime, a2->startTime);
	if (result == 0)
	{
		// They may be overlapping events (their start time is 0)
		if ((a1->overlapState == overlapScndPart) && (a2->overlapState == overlapScndPart))
			result = TimeCompare (a1->startTimeInPrevDay, a2->startTimeInPrevDay);
		else
			result = TimeCompare (a1->endTime, a2->endTime);
	}
	return result;
}


/************************************************************************
 *
 * FUNCTION:    	PrvRemoveNonConflictingAppointmentsInDay
 *
 * DESCRIPTION: 	This routine removes the appointments in current
 *					category that do not conflict with appointments from
 *					other categories.
 *
 * PARAMETERS:  -> 	dbP:		Pointer to the database.
 *				<-> apptListP: 	Pointer on the appointment list.
 *				-> 	count : 	Input appointment list.
 *
 * RETURNED:    	The new size of the list, as a number of appointments.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	02/20/03	Initial revision.
 *
 ***********************************************************************/
static int32_t PrvRemoveNonConflictingAppointmentsInDay (DmOpenRef dbP, ApptInfoPtr apptListP, int32_t count)
{
	int32_t 	i, j;
	Boolean 	inCategory;
	status_t	error;
	Boolean		currentApptIsConflicting;

	// Check belonging with current category
	for (i = 0; i < count; i++)
	{
		error = DbIsRowInCategory(dbP, apptListP[i].rowID, DateBkCurrentCategoriesCount,
			DateBkCurrentCategoriesP, DbMatchAny, &inCategory);

		if ((error >= errNone) && inCategory)
		{
			apptListP[i].belongingToCurrCat = true;
		}
		else
		{
			apptListP[i].belongingToCurrCat = false;
		}

		apptListP[i].toBeRemoved = false;
	}

	// Scan each non-belonging appt to check if it conflicts with belonging ones
	for (i = 0; i < count; i++)
	{
		if (! apptListP[i].belongingToCurrCat)
		{
			currentApptIsConflicting = false;

			// Non-timed events can't conflict
			if (apptListP[i].noTimeEvent)
			{
				currentApptIsConflicting = true;
				continue;
			}

			// Check previous events if they conflict
			if (!currentApptIsConflicting)
				for (j = 0; j < i; j++)
				{
					if (apptListP[j].belongingToCurrCat)
					{
						if (TimeToInt(apptListP[j].endTime) > TimeToInt(apptListP[i].startTime))
						{
							currentApptIsConflicting =  true;
							break;
						}
					}
				}

			// Check following events if they conflict
			if (!currentApptIsConflicting)
				for (j = i+1; j < count; j++)
				{
					if (apptListP[j].belongingToCurrCat)
					{
						if (TimeToInt(apptListP[j].startTime) < TimeToInt(apptListP[i].endTime))
						{
							currentApptIsConflicting =  true;
							break;
						}
					}
				}

			// Remove from list if it is not conflicting
			if (! currentApptIsConflicting)
				apptListP[i].toBeRemoved = true;
		}
	}

	// Compacting the table
	i = (int32_t) (count - 1);
	while (i >= 0)
	{
		if (apptListP[i].toBeRemoved)
		{
			count--;
			if ((count - i) > 0)
				memmove(&apptListP[i], &apptListP[i+1], (count - i) * sizeof(ApptInfoType));
			else
				i--;
		}
		else
			i--;
	}

	return count;
}


/************************************************************************
 *
 * FUNCTION:    	PrvSplitEventIfOverlapping
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	apptRecP,
 *				->	startTimeP:
 *				->	endTimeP:
 *				->	noTimeEventP
 *	 			->	overlapStateP:
 *				->	goToNextEventP:
 *				->	occurDateP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	04/16/03	Initial revision.
 *
 ***********************************************************************/
static void PrvSplitEventIfOverlapping(
	ApptDBRecordType * 			apptRecP,
	TimeType * 					startTimeP,
	TimeType * 					endTimeP,
	Boolean * 					noTimeEventP,
	MidnightOverlapStateType * 	overlapStateP,
	Boolean * 					goToNextEventP,
	DateType * 					occurDateP)
{
	*startTimeP = apptRecP->when.startTime;
	*endTimeP = apptRecP->when.endTime;
	*noTimeEventP = apptRecP->when.noTimeEvent;

	if (*noTimeEventP)
	{
		// Display no time events on top (they may have been updated due
		// to the time zone shifting
		TimeToInt(*startTimeP) = noTimeStartEndTime;
		TimeToInt(*endTimeP) = noTimeStartEndTime;
	}

	if (apptRecP->when.midnightWrapped)
	{
		// The event overlapps midnight, first add the first day part
		if (*goToNextEventP)
		{
			// Set the end date to overlapEndTime
			TimeToInt(*endTimeP) = overlapEndTime;
			*goToNextEventP = false;
			*overlapStateP = overlapFirstPart;
		}
		else
		// Then, add the second part
		{
			// Set the start date to overlapStartTime, increase the date by one
			TimeToInt(*startTimeP) = overlapStartTime;
			if (occurDateP)
				DateAdjust(occurDateP, +1);
			*goToNextEventP = true;
			*overlapStateP = overlapScndPart;
		}
	}
	else
	{
		*overlapStateP = overlapNone;
		*goToNextEventP = true;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvApplyDSTRulesToThisDate
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	apptRecP:
 *				->	dateP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	04/16/03	Initial revision.
 *
 ***********************************************************************/
static void PrvApplyDSTRulesToThisDate(ApptDBRecordType * apptRecP, DateType * dateP)
{
	char *			apptTimeZone = apptRecP->when.timeZoneName;
	DateTimeType 	localBeginDateTime;
	DateTimeType	localDateToDisplay;
	DateTimeType 	localEndDateTime;
	DateTimeType 	apptTZBeginDateTime;
	DateTimeType	apptTZDateToDisplay;
	DateTimeType 	apptTZEndDateTime;
	time_t			utcBeginTime;
	time_t			utcEndTime;
	time_t			utcDateToDisplay;
	time_t 			utcBeginInDateToDisplay;
	time_t			utcEndInDateToDisplay;
	DateTimeType	apptTZBeginWithDisplayDate;
	DateTimeType	localBeginWithDisplayDate;
	DateTimeType	apptTZEndWithDisplayDate;
	DateTimeType	localEndWithDisplayDate;
	DateType		tmpDate;
	DateType		beginDateApptTZ;
	DateType		endDateApptTZ;
	DateType		beginDateLocal;
	DateType		endDateLocal;
	TimeType		tmpTime;
	Boolean			overlapInApptTZ;
	int32_t			offset;

	// Check if it not an untimed event
	if (apptRecP->when.noTimeEvent)
		return;

	// Check if device TZ and Appointment TZ are equal or not assigned
	if ((! StrCompare(gDeviceTimeZone, apptTimeZone)) || (apptTimeZone[0] == chrNull))
		return;

	// They are different

	// Step 1 : Get begin/end date & time in local (device) TZ
	tmpDate = apptRecP->when.date;
	CnvDate2DateTime(&tmpDate, &(apptRecP->when.startTime), &localBeginDateTime);
	if (apptRecP->when.midnightWrapped)
		DateAdjust(&tmpDate, +1);

	CnvDate2DateTime(&tmpDate, &(apptRecP->when.endTime), &localEndDateTime);

	/* Steps 2 to 3 : convert local date & time to APPT TZ date & time */

	// Step 2 : Convert local appt begin date & time TO TZ appt begin date & time
	utcBeginTime = CnvTimeZone2UTC(&localBeginDateTime, gDeviceTimeZone);
	utcEndTime = CnvTimeZone2UTC(&localEndDateTime, gDeviceTimeZone);

	// Step 3 : Convert utc begin date & time to appt TZ
	CnvUTC2TimeZone(&utcBeginTime, apptTimeZone, &apptTZBeginDateTime);
	CnvUTC2TimeZone(&utcEndTime, apptTimeZone, &apptTZEndDateTime);

	// Does it overlap in appointment TZ ?
	beginDateApptTZ = CnvDateTime2Date(&apptTZBeginDateTime);
	endDateApptTZ = CnvDateTime2Date(&apptTZEndDateTime);
	if (DateToInt(beginDateApptTZ) != DateToInt(endDateApptTZ))
	{
		ErrNonFatalDisplayIf(DateToDays(endDateApptTZ) != DateToDays(beginDateApptTZ)+1,
			"Repeating events: Error in computing timezone shift.");
		overlapInApptTZ = true;
	}
	else
		overlapInApptTZ = false;

	// Step 4 : Convert the local date to display to appt TZ
	// Must be linked with the appointment local start time
	CnvDate2DateTime(dateP, &(apptRecP->when.startTime), &localDateToDisplay);
	utcDateToDisplay = CnvTimeZone2UTC(&localDateToDisplay, gDeviceTimeZone);
	CnvUTC2TimeZone(&utcDateToDisplay, apptTimeZone, &apptTZDateToDisplay);

	// Step 5 : Make the appointment in display date + Begin appt time in appt TZ
	tmpDate = CnvDateTime2Date(&apptTZDateToDisplay);
	tmpTime = CnvDateTime2Time(&apptTZBeginDateTime);
	CnvDate2DateTime(&tmpDate, &tmpTime, &apptTZBeginWithDisplayDate);

	// Step 6 : Convert the display date + appt time from appt TZ to device TZ
	utcBeginInDateToDisplay = CnvTimeZone2UTC(&apptTZBeginWithDisplayDate, apptTimeZone);
	CnvUTC2TimeZone(&utcBeginInDateToDisplay, gDeviceTimeZone, &localBeginWithDisplayDate);

	// Step 7 : Make the appointment in display date + End appt time in appt TZ
	// If the appointment overlaps in the appt TZ, increase the date by one
	if (overlapInApptTZ)
		DateAdjust(&tmpDate, +1);

	tmpTime = CnvDateTime2Time(&apptTZEndDateTime);
	CnvDate2DateTime(&tmpDate, &tmpTime, &apptTZEndWithDisplayDate);

	// Step 8 : Convert the display date + appt time from appt TZ to device TZ
	utcEndInDateToDisplay = CnvTimeZone2UTC(&apptTZEndWithDisplayDate, apptTimeZone);
	CnvUTC2TimeZone(&utcEndInDateToDisplay, gDeviceTimeZone, &localEndWithDisplayDate);

	// Step 9 : Update event information
	beginDateLocal = CnvDateTime2Date(&localBeginWithDisplayDate);
	endDateLocal = CnvDateTime2Date(&localEndWithDisplayDate);

	apptRecP->when.startTime = CnvDateTime2Time(&localBeginWithDisplayDate);
	apptRecP->when.endTime = CnvDateTime2Time(&localEndWithDisplayDate);

	if (DateToInt(beginDateLocal) != DateToInt(endDateLocal))
	{
		ErrNonFatalDisplayIf(DateToDays(endDateLocal) != DateToDays(beginDateLocal)+1,
			"Repeating events: Error in computing timezone shift.");
		apptRecP->when.midnightWrapped = true;
	}
	else
		apptRecP->when.midnightWrapped = false;

	// Update repeat end & begin date if needed
	if (DateToInt(beginDateLocal) != DateToInt(*dateP))
	{
		offset = DateToDays(beginDateLocal) - DateToDays(*dateP);
		DateAdjust(&apptRecP->when.date, offset);

		if (DateToInt(apptRecP->repeat.repeatEndDate) != apptNoEndDate)
			DateAdjust(&apptRecP->repeat.repeatEndDate, offset);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	ApptAddNonRepeatingAppt.
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	cursorID:
 *				->	apptRecP:
 *				->	startRangeDate:
 *				->	endRangeDate:
 *				->	theDayBeforeStartRange:
 *				->	apptLists:
 *				->	counts:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	08/23/03	Initial revision.
 *
 ***********************************************************************/
void ApptAddNonRepeatingAppt(
	uint32_t 					cursorID,
	ApptDBRecordType * 			apptRecP,
	DateType 					startRangeDate,
	DateType 					endRangeDate,
	DateType 					theDayBeforeStartRange,
	MemHandle 					apptLists [],
	int32_t 					counts [])
{
	ApptDateTimeType *			apptWhenP = &(apptRecP->when);
	DateType 					apptDate = apptWhenP->date;
	TimeType					startTimeInPrevDay = apptWhenP->startTime;
	TimeType					endTimeInNextDay = apptWhenP->endTime;
	TimeType					tempTime;
	MidnightOverlapStateType 	overlapState = overlapNone;
	uint32_t					index;

	// Due to timezones offsets, we may have here unrelevant appointments
	if ((DateToInt (apptWhenP->date) < DateToInt(theDayBeforeStartRange)) ||
		(DateToInt (apptWhenP->date) > DateToInt (endRangeDate)))
		return;

	// Skip the appointment in the day before if it does not overlapp midnight
	// Take care of the day 1/1/1904 for which startRangeDate == theDayBeforeStartRange
	if ((DateToInt (apptWhenP->date) == DateToInt(theDayBeforeStartRange)) &&
		(!apptWhenP->midnightWrapped) &&
		DateToInt(startRangeDate) != DateToInt(theDayBeforeStartRange))
		return;

	if (apptWhenP->noTimeEvent)
	{
		// Display no time events on top (they may have been updated due
		// to the time zone shifting
		TimeToInt(apptWhenP->startTime) = noTimeStartEndTime;
		TimeToInt(apptWhenP->endTime) = noTimeStartEndTime;
	}

	if (apptWhenP->midnightWrapped)
	{
		if (DateToInt (apptDate) == DateToInt (endRangeDate))
		{
			// Only the beginning part of the appointment must be added
			TimeToInt(apptWhenP->endTime) = overlapEndTime;
			index = DateToDays (apptDate) - DateToDays (startRangeDate);
			overlapState = overlapFirstPart;
		}
		else if (DateToInt (apptDate) == DateToInt (theDayBeforeStartRange))
		{
			// Only the ending part of the appointment must be added
			index = 0;
			TimeToInt(apptWhenP->startTime) = overlapStartTime;
			overlapState = overlapScndPart;
		}
		else
		{
			// Two appointments must be created: create here for the day after
			index = DateToDays (apptDate) - DateToDays (startRangeDate) + 1;
			TimeToInt(tempTime) = overlapStartTime;
			overlapState = overlapScndPart;
			AddAppointmentToList (&apptLists[index], &counts[index], tempTime,
				startTimeInPrevDay, apptWhenP->endTime, endTimeInNextDay, overlapState,
				apptWhenP->noTimeEvent, cursorID);

			// Now add the first part of the event
			index--;
			TimeToInt(apptWhenP->endTime) = overlapEndTime;
			overlapState = overlapFirstPart;
		}
	}
	else
	{
		index = (uint32_t) (DateToDays (apptDate) - DateToDays (startRangeDate));
	}

	// Add the record to the appointment list.
	AddAppointmentToList (&apptLists[index], &counts[index], apptWhenP->startTime,
		startTimeInPrevDay, apptWhenP->endTime, endTimeInNextDay, overlapState,
		apptWhenP->noTimeEvent, cursorID);
}


/***********************************************************************
 *
 * FUNCTION:    	ApptAddRepeatingAppt
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	cursorID:
 *				->	apptRecP:
 *				->	startRangeDate:
 *				->	endRangeDate:
 *				->	theDayBeforeStartRange:
 *				->	apptLists:
 *				->	counts:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	08/23/03	Initial revision.
 *
 ***********************************************************************/
void ApptAddRepeatingAppt(
	uint32_t 					cursorID,
	ApptDBRecordType * 			apptRecP,
	DateType 					startRangeDate,
	DateType 					endRangeDate,
	DateType 					theDayBeforeStartRange,
	MemHandle 					apptLists [],
	int32_t 					counts [])
{
	ApptDateTimeType *			apptWhenP = &(apptRecP->when);
	ApptDateTimeType 			apptDateInDeviceTZ;
	RepeatInfoType 				apptRepeatInDeviceTZ;
	Boolean 					goToNextEvent;
	TimeType 					startTime;
	TimeType 					endTime;
	Boolean						noTimeEvent;
	MidnightOverlapStateType 	overlapState;
	DateType 					repeatDate;
	DateType 					occurDate;
	DateType 					repeatStartDate;
	DateType					tempDate;
	uint32_t					dateInDays;
	uint32_t					currentDateInDays;
	uint32_t					index;


	// Get only one day
	if (DateToInt(startRangeDate) == DateToInt(endRangeDate))
	{
		// Save initial values (must be restored before DST adjustment)
		apptDateInDeviceTZ = *apptWhenP;
		apptRepeatInDeviceTZ = apptRecP->repeat;

		// Iterate on current day and the day before because some events
		// may have started the day before
		PrvApplyDSTRulesToThisDate(apptRecP, &theDayBeforeStartRange);

		// An event starting the day before overlapps...
		if(apptRecP->when.midnightWrapped)
		{
			if (ApptRepeatsOnDate (apptRecP, theDayBeforeStartRange))
			{
				goToNextEvent = false;
				// Below assigns startTime, endTime, noTimeEvent, overlapState
				PrvSplitEventIfOverlapping(apptRecP, &startTime, &endTime,
					&noTimeEvent, &overlapState, &goToNextEvent, NULL);

				AddAppointmentToList (apptLists, counts, startTime, apptWhenP->startTime, endTime,
							apptWhenP->endTime, overlapState, noTimeEvent, cursorID);
			}
		}

		// Add today event: Restore initial values for next day
		apptRecP->when = apptDateInDeviceTZ;
		apptRecP->repeat = apptRepeatInDeviceTZ;
		PrvApplyDSTRulesToThisDate(apptRecP, &startRangeDate);
		if (ApptRepeatsOnDate (apptRecP, startRangeDate))
		{
			goToNextEvent = true;
			// Assign local values
			PrvSplitEventIfOverlapping(apptRecP, &startTime, &endTime, &noTimeEvent,
											&overlapState, &goToNextEvent, NULL);

			AddAppointmentToList (apptLists, counts, startTime, apptWhenP->startTime, endTime,
									apptWhenP->endTime, overlapState, noTimeEvent, cursorID);
		}
	}
	else // Get a whole week
	{
		// Init
		dateInDays = DateToDays (startRangeDate);

		// Set initial date to scan (will be increased to parse the whole week)
		repeatDate = theDayBeforeStartRange;
		// Set event initial date
		repeatStartDate = apptWhenP->date;

		// Save initial values (must be restored before DST adjustment)
		apptDateInDeviceTZ = *apptWhenP;
		apptRepeatInDeviceTZ = apptRecP->repeat;

	 	while(true) // Loop for days in week
		{
			// Check that the event is into the defined interval
			if (!ApptNextRepeat (apptRecP, &repeatDate, true))
				break;

			// Translate event only once (daylight saving change on <repeatDate> :
			// apptRec DST rules are linked with appointment starting date.
			PrvApplyDSTRulesToThisDate(apptRecP, &repeatDate);

			// goToNextEvent is used to reiterate on same event if it overlaps
			goToNextEvent = true;

			do // Loop for overlapping events
			{
				// Assign local values or skip
				occurDate = repeatDate;
				PrvSplitEventIfOverlapping(apptRecP, &startTime, &endTime, &noTimeEvent,
												&overlapState, &goToNextEvent, &occurDate);

				if (DateToInt (occurDate) > DateToInt (endRangeDate))
					break;

				// Add the record to the appointment list.
				currentDateInDays = DateToDays (occurDate);
				if (currentDateInDays < dateInDays)
					continue;

				index = currentDateInDays - dateInDays;

				AddAppointmentToList (&apptLists[index], &counts[index], startTime, apptWhenP->startTime,
									endTime, apptWhenP->endTime, overlapState, noTimeEvent, cursorID);
			}
			while (! goToNextEvent); // End of loop on overlapping events

			// When the date reach the limits, DateAdjust doesn't change the value.
			tempDate = repeatDate;
			DateAdjust (&repeatDate, 1);

			if (DateToInt (tempDate) == DateToInt (repeatDate))
				break;

			if (DateToInt (repeatDate) > DateToInt (endRangeDate))
				break;

			// Restore initial values for next day
			*apptWhenP = apptDateInDeviceTZ;
			apptRecP->repeat = apptRepeatInDeviceTZ;
		} // End for Loop on days in week
	}
}


/************************************************************************
 *
 * FUNCTION:    	ApptGetAppointmentsFullCursor
 *
 * DESCRIPTION: 	This routine returns a list of appointments that are
 *					in the range of dates specified - Uses a full cursor.
 *
 * PARAMETERS:  ->	dbP:			pointer to the database
 *				->	cursorID:		cursor to update
 *              ->	date :			start date to search from
 *              ->	days:			number a days in search range
 *              ->	apptLists:		returned: array of handle of the
 *									appointment list (ApptInfoType)
 *             ->	counts:			returned: returned: array of counts of the
 *                          		number of appointments in each list.
 *				->	addConflicts:	if true, conflicting appointments belonging
 *									to other categories (not current one)
 *									will be added.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/7/96		Initial revision.
 *	PPL	07/10/03	Cursors !
 *
 ***********************************************************************/
void ApptGetAppointmentsFullCursor (
	DmOpenRef 					dbP,
	uint32_t				 	cursorID,
	DateType 					startRangeDate,
	int32_t 					days,
	MemHandle 					apptLists [],
	int32_t 					counts [],
	Boolean 					addConflicts)
{
	int32_t						index;
	uint32_t					rowsCount;
	int32_t						dateInDays;
	DateType					endRangeDate;
	DateType					theDayBefore;
	ApptInfoPtr 				apptList;
	ApptDBRecordType 			apptRec;
	uint16_t 					attr;
	time_t 						checkDateInUTCSecs;
	status_t					err;
	Boolean						isInCurrentCategory;
	uint32_t					initialCurrentRowID = dbInvalidRowID;
	DateTimeType				tmpDateTime;

	// Initial checkings & profiling
	ErrNonFatalDisplayIf(! DbIsCursorID(cursorID), "CursorID required");
	PIMAppProfilingBegin("Load Appointments in DB, Full DB based");

	// Save the initial position
	DbCursorGetCurrentRowID(cursorID, &initialCurrentRowID);

	// Initializations
	memset(apptLists, 0x00, days * sizeof (MemHandle));
	memset(counts, 0x00, days * sizeof (int32_t));

	// We may need to requery if there was newly inserted appointments
	ApptDBRequery(dbP, cursorID, false);

	// Compute date-1
	theDayBefore = startRangeDate;
	DateAdjust (&theDayBefore,-1);

	//startDate = DateToInt (date);
	endRangeDate = startRangeDate;
	DateAdjust (&endRangeDate, days - 1);

	rowsCount = DbCursorGetRowCount (cursorID);

	// Find the first non-repeating appointment of the day.
	ApptFindFirstNonRepeating (dbP, cursorID, theDayBefore);
	while (!DbCursorIsEOF(cursorID))
	{
		// Check the attributes if current security status is Hiding
		if (CurrentRecordVisualStatus == hidePrivateRecords)
		{
			DbGetRowAttr(dbP, cursorID, &attr);
			if (attr & dbRecAttrSecret)
			{
				// Skip the appointment : it shouldn't be displayed since it is marked hidden
				DbCursorMoveNext(cursorID);
				continue;
			}
		}

		// If we don't add conflicting appointments, check categories belonging here
		if (! addConflicts)
		{
			err = DbIsRowInCategory(dbP, cursorID, DateBkCurrentCategoriesCount,
				DateBkCurrentCategoriesP, DbMatchAny, &isInCurrentCategory);
			ErrNonFatalDisplayIf(err < errNone, "Error in category test");

			if (!isInCurrentCategory)
			{
				DbCursorMoveNext(cursorID);
				continue;
			}
		}

		// Check if the appointment is on the date passed. If it is,
		// add it to the appointment list.
		ApptGetRecord(dbP, cursorID, &apptRec, DBK_SELECT_TIME);

		ApptAddNonRepeatingAppt(cursorID, &apptRec, startRangeDate, endRangeDate, theDayBefore, apptLists, counts);

		ApptFreeRecordMemory (&apptRec);
		DbCursorMoveNext(cursorID);
	}


	// Add the repeating appointments to the list.  Repeating appointments
	// are stored at the beginning of the database.
	dateInDays = DateToDays (startRangeDate);
	CnvDate2DateTime(&startRangeDate, NULL, &tmpDateTime);
	CnvLocalTime2UTC(&tmpDateTime, &checkDateInUTCSecs, true);
	// Substract daysInSeconds because of 1-day overlapping events
	checkDateInUTCSecs -= daysInSeconds;
	DbCursorMoveFirst(cursorID);

	while (!DbCursorIsEOF(cursorID))
	{
		if (ApptRepeatEventIsFinished(dbP, cursorID, checkDateInUTCSecs))
		{
			// Skip the appointment : the ending date is before "date".
			DbCursorMoveNext(cursorID);
			continue;
		}

		// Check the attributes if current security status is Hiding
		if (CurrentRecordVisualStatus == hidePrivateRecords)
		{
			DbGetRowAttr(dbP, cursorID, &attr);
			if (attr & dbRecAttrSecret)
			{
				// Skip the appointment : it shouldn't be displayed since it is marked hidden
				DbCursorMoveNext(cursorID);
				continue;
			}
		}
		// If we don't add conflicting appointments, check categories belonging here
		if (! addConflicts)
		{
			err = DbIsRowInCategory(dbP, cursorID, DateBkCurrentCategoriesCount,
				DateBkCurrentCategoriesP, DbMatchAny, &isInCurrentCategory);
			ErrNonFatalDisplayIf(err < errNone, "Error in category test");

			if (!isInCurrentCategory)
			{
				DbCursorMoveNext(cursorID);
				continue;
			}
		}

		ApptGetRecord(dbP, cursorID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);

		// If the record has no repeating info we've reached the end of the
		// repeating appointments.
		if (apptRec.repeat.repeatType == repeatNone)
		{
			ApptFreeRecordMemory (&apptRec);
			break;
		}

		ApptAddRepeatingAppt(cursorID, &apptRec, startRangeDate, endRangeDate, theDayBefore, apptLists, counts);

		ApptFreeRecordMemory (&apptRec);
		DbCursorMoveNext(cursorID);
	}


	// Sort the list by start time.
	for (index = 0; index < days; index ++)
	{
		if (apptLists[index])
		{
			apptList = MemHandleLock(apptLists[index]);
			SysInsertionSort (apptList, (uint16_t) counts[index], sizeof (ApptInfoType), (_comparF *)ApptListCompare, 0L);

			MemHandleUnlock (apptLists[index]);
			MemHandleResize (apptLists[index], (uint32_t)(counts[index] * sizeof (ApptInfoType)));
		}
	}

	// Remove from list appointments that don't conflict with current category
	if (addConflicts && ((DateBkCurrentCategoriesP == NULL) || (*DateBkCurrentCategoriesP != catIDAll)))
	{
		for (index = 0; index < days; index ++)
		{
			if (apptLists[index])
			{
				apptList = MemHandleLock(apptLists[index]);

				counts[index] = PrvRemoveNonConflictingAppointmentsInDay(dbP, apptList, counts[index]);

				MemHandleUnlock (apptLists[index]);
				MemHandleResize (apptLists[index], (uint32_t)(counts[index] * sizeof (ApptInfoType)));
			}
		}
	}

	// Restore initial position
	if (initialCurrentRowID != dbInvalidRowID)
		DbCursorMoveToRowID(cursorID, initialCurrentRowID);

	PIMAppProfilingEnd();
}


/************************************************************************
 *
 * FUNCTION:    	ApptGetAppointmentsWhereClause
 *
 * DESCRIPTION: 	This routine returns a list of appointments that are
 *					in the range of dates specified - Uses the DB Mgr
 *					WHERE clause.
 *
 * PARAMETERS:  ->	dbP:			pointer to the database
 *				->	cursorID:		cursor to update
 *             	->	date:			start date to search from
 *             	->	days:			number a days in search range
 *              ->	apptLists:		returned: array of handle of the
 *									appointment list (ApptInfoType)
 *              ->	counts:			returned: returned: array of counts of the
 *                          		number of appointments in each list.
 *				->	addConflicts:	if true, conflicting appointments belonging
 *									to other categories (not current one)
 *									will be added.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	4/7/96		Initial revision.
 *	ppl	07/10/03	Cursors !
 *
 ***********************************************************************/
void ApptGetAppointmentsWhereClause (
	DmOpenRef 					dbP,
	uint32_t * 					cursorIDP,
	DateType 					startRangeDate,
	int32_t 					days,
	MemHandle 					apptLists [],
	int32_t 					counts [],
	Boolean 					addConflicts)
{
	DateType					endRangeDate;
	DateType					theDayBeforeStartRange;
	time_t						rangeStartDateSecs;
	time_t						rangeEndDateSecs;
	ApptDBRecordType 			apptRec;
	ApptInfoPtr 				apptList;
	int32_t						index;
	uint32_t					initialCurrentRowID = dbInvalidRowID;

	// Initial checkings & profiling
	PIMAppProfilingBegin("Load Appointments in DB, Where clause");

	// Initializations
	memset(apptLists, 0x00, days * sizeof (MemHandle));
	memset(counts, 0x00, days * sizeof (int32_t));

	// Save the initial position
	if (DbCursorGetCurrentRowID(*cursorIDP, &initialCurrentRowID) < errNone)
		initialCurrentRowID = dbInvalidRowID;

	// Compute the range in seconds according to the input date
	CalculateStartEndRangeTimes(&startRangeDate, days, &rangeStartDateSecs,
		&rangeEndDateSecs, &endRangeDate);

	// Compute the day before the start range day
	theDayBeforeStartRange = startRangeDate;
	DateAdjust (&theDayBeforeStartRange,-1);

	// We may need to requery if there was newly inserted appointments
	ApptDBOpenOrRequeryWithNewRange(dbP, cursorIDP, rangeStartDateSecs, rangeEndDateSecs, addConflicts);
	if (*cursorIDP == dbInvalidCursorID)
		PIMAppProfilingReturnVoid();

	// Now, filter the appointments from cursor
	for (DbCursorMoveFirst(*cursorIDP) ; !DbCursorIsEOF(*cursorIDP) ; DbCursorMoveNext(*cursorIDP) )
	{
		if (ApptEventRepeatIsSet(dbP, *cursorIDP))
		{
			// Load the repeating appointment
			ApptGetRecord(dbP, *cursorIDP, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);

			// Add appointment in list
			ApptAddRepeatingAppt(*cursorIDP, &apptRec, startRangeDate, endRangeDate, theDayBeforeStartRange, apptLists, counts);
		}
		else
		{
			// Load the non-repeating appointment
			ApptGetRecord(dbP, *cursorIDP, &apptRec, DBK_SELECT_TIME);

			// Add appointment in list
			ApptAddNonRepeatingAppt(*cursorIDP, &apptRec, startRangeDate, endRangeDate, theDayBeforeStartRange, apptLists, counts);
		}

		// Free the appointment struct
		ApptFreeRecordMemory (&apptRec);
	}

	// Sort the list by start time.
	for (index = 0; index < days; index ++)
	{
		if (apptLists[index])
		{
			apptList = MemHandleLock(apptLists[index]);
			SysInsertionSort (apptList, (uint16_t) counts[index], sizeof (ApptInfoType), (_comparF *)ApptListCompare, 0L);

			MemHandleUnlock (apptLists[index]);
			MemHandleResize (apptLists[index], (uint32_t)(counts[index] * sizeof (ApptInfoType)));
		}
	}

	// Remove from list appointments that don't conflict with current category
	if (addConflicts && ((DateBkCurrentCategoriesP == NULL) || (*DateBkCurrentCategoriesP != catIDAll)))
	{
		for (index = 0; index < days; index ++)
		{
			if (apptLists[index])
			{
				apptList = MemHandleLock(apptLists[index]);

				counts[index] = PrvRemoveNonConflictingAppointmentsInDay(dbP, apptList, counts[index]);

				MemHandleUnlock (apptLists[index]);
				MemHandleResize (apptLists[index], (uint32_t)(counts[index] * sizeof (ApptInfoType)));
			}
		}
	}

	// Restore initial position
	if (initialCurrentRowID != dbInvalidRowID)
		DbCursorMoveToRowID(*cursorIDP, initialCurrentRowID);

	PIMAppProfilingEnd();
}


/************************************************************************
 *
 * FUNCTION:   		ApptGetAppointments
 *
 * DESCRIPTION: 	This routine returns a list of appointments that are in
 *              	the range of dates specified
 *
 * PARAMETERS:  ->	dbP:			pointer to the database
 *				->	cursorID:		opened cursor
 *              ->	date:			start date to search from
 *              ->	days:			number a days in search range
 *              ->	apptLists:		returned: array of handle of the
 *									appointment list (ApptInfoType)
 *              ->	counts:			returned: returned: array of counts of the
 *                          		number of appointments in each list.
 *				->	addConflicts:	if true, conflicting appointments belonging
 *									to other categories (not current one)
 *									will be added.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	4/7/96		Initial revision.
 *	PPL	07/10/03	Cursors !
 *
 ***********************************************************************/
void ApptGetAppointments (
	DmOpenRef 	dbP,
	uint32_t * 	cursorIDP,
	DateType 	date,
	int32_t 	days,
	MemHandle 	apptLists [],
	int32_t 	counts [],
	Boolean 	addConflicts)
{
#if defined(USE_WHERE_CLAUSE)
	// Get the appointments using where clause
	ApptGetAppointmentsWhereClause(dbP, cursorIDP, date, days, apptLists, counts, addConflicts);

	#if ((TRACE_OUTPUT == TRACE_OUTPUT_ON) && (!defined(PIM_APPS_PROFILING)))
		// Tracing filtered cursor
		//ApptCursorTrace(dbP, *cursorIDP);

		// Tracing loaded appointments
		TraceOutput(TL(appErrorClass,"------- TRACE APPTS WITH WHERE CLAUSE ----------"));
		ApptListTrace(days, apptLists, counts);
	#endif
#else
	// Get the appointments - Old style, without new where clause
	ApptGetAppointmentsFullCursor(dbP, *cursorIDP, date, days, apptLists, counts, addConflicts);

	#if ((TRACE_OUTPUT == TRACE_OUTPUT_ON) && (!defined(PIM_APPS_PROFILING)))
		// Tracing full cursor
		//ApptCursorTrace(dbP, *cursorIDP);

		// Tracing loaded appointments
		TraceOutput(TL(appErrorClass,"------- TRACE APPTS WITHOUT WHERE ----------"));
		ApptListTrace(days, apptLists, counts);
	#endif
#endif
}


/***********************************************************************
 *
 * FUNCTION: 		PrvComputeRepeatOnForLocalDate
 *
 * DESCRIPTION: 	Compute the new repeatOn value based on
 *					event start day shifting due to timezones.
 *
 *				 	Applied when reading from DateBook database.
 *
 * PARAMETERS: -> 	UTCRepeatOnValue: 	The repeat on value applied
 *				   						to the date in UTC timezone.
 *				-> 	apptRecP: 			Database record.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision.
 *
 ***********************************************************************/
static uint8_t PrvComputeRepeatOnForLocalDate(uint8_t UTCRepeatOnValue, ApptDBRecordType * apptRecP)
{
	return ComputeRepeatOnForTimeZone(UTCRepeatOnValue, gDeviceTimeZone, apptRecP->repeat.repeatType, &apptRecP->when);
}


/***********************************************************************
 *
 * FUNCTION: 		PrvComputeRepeatOnForUTCDate
 *
 * DESCRIPTION: 	Compute the new repeatOn value based on
 *					event start day shifting due to timezones.
 *
 * PARAMETERS: -> 	apptRecP	: database record.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	10/09/02	Initial revision.
 *
 *
 ***********************************************************************/
static uint8_t PrvComputeRepeatOnForUTCDate(ApptDBRecordType * apptRecP)
{
	return ComputeRepeatOnForUTC(apptRecP->repeat.repeatOn, gDeviceTimeZone, apptRecP->repeat.repeatType, &apptRecP->when);
}


/************************************************************************
 *
 * FUNCTION: 		ApptFillRecord
 *
 * DESCRIPTION: 	Get a read-only record from the Appointment Database.
 *
 * PARAMETERS: 	-> 	dbP: 	database handle.
 *				-> 	rowID: 	record row ID or Cursor ID.
 *				<- 	rectP: 	read-only database record.
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
static status_t PrvApptFillRecord (DmOpenRef dbP, uint32_t rowID, ApptDBRecordType * rectP, RecordFilteringInfoType loadingFilter)
{
	status_t 					err = errNone;
	uint32_t 					timeColumnIDsArray[] = DATEBOOK_TIME_INFORMATION;
	uint32_t 					timeNumColumns = sizeof(timeColumnIDsArray)/sizeof(uint32_t);
	uint32_t 					alarmColumnIDsArray[] = DATEBOOK_ALARM_INFORMATION;
	uint32_t 					alarmNumColumns = sizeof(alarmColumnIDsArray)/sizeof(uint32_t);
	uint32_t 					repeatColumnIDsArray[] = DATEBOOK_REPEAT_INFORMATION;
	uint32_t 					repeatNumColumns = sizeof(repeatColumnIDsArray)/sizeof(uint32_t);
	time_t						UTCstartTimeInSecs;
	time_t						UTCendTimeInSecs;
	time_t						UTCrepeatEndTimeInSecs;
	uint32_t					i;
	uint32_t					exceptionsSize;
	uint32_t					noteSize;
	uint32_t					locationSize;
	uint32_t					descSize;
	DateTimeType 				tmpDateTime;
	time_t*						exceptionDataP = NULL;
	DbSchemaColumnValueType* 	columnValuesArray;
	Boolean						repeatSet;
	Boolean						timezoneNotAssignedInDB = false;

	// ***************************************** DBK_SELECT_TIME ******************************************
	if (loadingFilter & DBK_SELECT_TIME)
	{
		// Get time column values
		err = DbGetColumnValues(dbP, rowID, timeNumColumns, timeColumnIDsArray, &columnValuesArray);

		// The time information must ALWAYS be set, excepted the timezone
		if (err == dmErrOneOrMoreFailed)
		{
			// Check that needed fields are assigned
			if ((columnValuesArray[DBK_WHEN_UNTIMED_INDEX].errCode != errNone) ||
				(columnValuesArray[DBK_WHEN_START_INDEX].errCode != errNone) ||
				(columnValuesArray[DBK_WHEN_END_INDEX].errCode != errNone))
			{
				DbReleaseStorage(dbP, columnValuesArray);
				goto Exit;
			}
			// Only the timezone was not assigned
			rectP->when.timeZoneName[0] = chrNull;
			timezoneNotAssignedInDB = true;
			// Reset the error code since the timezone missing is allowed
			err = errNone;
		}
		else if (err < errNone)
		{
			// For all other errors, abort.
			DbReleaseStorage(dbP, columnValuesArray);
			goto Exit;
		}

		// Extract the no-time information
		DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_WHEN_UNTIMED_INDEX, UNTIMED_EVENT_COL_ID, "untimed");
		rectP->when.noTimeEvent = *((Boolean*)columnValuesArray[DBK_WHEN_UNTIMED_INDEX].data);

		// Extract START date & time
		DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_WHEN_START_INDEX, START_EVENT_DATE_TIME_COL_ID, "start time");
		UTCstartTimeInSecs = *((time_t*)columnValuesArray[DBK_WHEN_START_INDEX].data);
		CnvUTC2LocalTime(&UTCstartTimeInSecs, &tmpDateTime, rectP->when.noTimeEvent);

		// Now assign start date & time
		rectP->when.startTime = CnvDateTime2Time(&tmpDateTime);
		rectP->when.date = CnvDateTime2Date(&tmpDateTime);

		// Extract END date & time
		DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_WHEN_END_INDEX, END_EVENT_DATE_TIME_COL_ID, "end time");
		UTCendTimeInSecs = *((time_t*)columnValuesArray[DBK_WHEN_END_INDEX].data);

		// Test the event validity: it must NOT exceed 24 hours.
		if ((UTCendTimeInSecs < UTCstartTimeInSecs) || (UTCendTimeInSecs - UTCstartTimeInSecs > daysInSeconds))
		{
			// set the maximum to daysInSeconds
			UTCendTimeInSecs = UTCstartTimeInSecs + daysInSeconds;
		}
		CnvUTC2LocalTime(&UTCendTimeInSecs, &tmpDateTime, rectP->when.noTimeEvent);

		// Now assign end time, & midnight wrapping
		rectP->when.endTime = CnvDateTime2Time(&tmpDateTime);
		if (rectP->when.date.day != (uint8_t)tmpDateTime.day)
			rectP->when.midnightWrapped = true;
		else
			rectP->when.midnightWrapped = false;

		// Extract the timezone information if existing
		if (! timezoneNotAssignedInDB)
		{
			DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_WHEN_TIMEZONE_INDEX, EVENT_TIMEZONE_COL_ID, "timezone");
			StrNCopy(rectP->when.timeZoneName, (char*)columnValuesArray[DBK_WHEN_TIMEZONE_INDEX].data, TZNAME_MAX);
			rectP->when.timeZoneName[TZNAME_MAX-1] = chrNull;
		}

		// Release the database storage
		DbReleaseStorage(dbP, columnValuesArray);
	} // End of SELECT_TIME info retrieval...

	// ***************************************** DBK_SELECT_ALARM ******************************************
	if (loadingFilter & DBK_SELECT_ALARM)
	{
		// Get alarm column values
		err = DbGetColumnValues(dbP, rowID, alarmNumColumns, alarmColumnIDsArray, &columnValuesArray);
		if (err < errNone)
		{
			DbReleaseStorage(dbP, columnValuesArray);
			if ((err == dmErrOneOrMoreFailed) || (err == dmErrNoColumnData))
			{
				// The information does not exist on DB, set as unset.
				rectP->alarm.advance = apptNoAlarm;
				rectP->alarm.advanceUnit = 0;
				err = errNone;
			}
			else
				goto Exit;
		}
		else
		{
			// Extract the alarm information
			DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_ALARM_ADVANCE_INDEX, EVENT_ALARM_ADVANCE_COL_ID, "time advance");
			rectP->alarm.advance = *((int8_t*)columnValuesArray[DBK_ALARM_ADVANCE_INDEX].data);
			if (rectP->alarm.advance != apptNoAlarm)
			{
				DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_ALARM_ADV_UNIT_INDEX, EVENT_ALARM_ADVANCE_UNIT_COL_ID, "advance unit");
				rectP->alarm.advanceUnit = *((AlarmUnitType*)columnValuesArray[DBK_ALARM_ADV_UNIT_INDEX].data);
			}

			// Release the database storage
			DbReleaseStorage(dbP, columnValuesArray);
		}
	} // End of SELECT_ALARM info retrieval...

	// ***************************************** DBK_SELECT_REPEAT ******************************************
	if (loadingFilter & DBK_SELECT_REPEAT)
	{
		// Get repeat column values
		err = DbGetColumnValues(dbP, rowID, repeatNumColumns, repeatColumnIDsArray, &columnValuesArray);
		if (err < errNone)
		{
			DbReleaseStorage(dbP, columnValuesArray);
			if ((err == dmErrOneOrMoreFailed) || (err == dmErrNoColumnData))
			{
				// The information does not exist on DB, set as unset.
				memset(&rectP->repeat, 0x00, sizeof(RepeatInfoType));
				err = errNone;
			}
			else
				goto Exit;
		}
		else
		{
			// Extract the repeat information
			repeatSet = *((Boolean*)columnValuesArray[DBK_REPEATING_EVENT_INDEX].data);
			DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_REPEAT_TYPE_INDEX, REPEAT_EVENT_TYPE_COL_ID, "repeat type");
			rectP->repeat.repeatType = *((RepeatType*)columnValuesArray[DBK_REPEAT_TYPE_INDEX].data);
			if ((rectP->repeat.repeatType == repeatNone) && !repeatSet)
			{
				// Nothing to do here
			}
			else if ((rectP->repeat.repeatType != repeatNone) && repeatSet)
			{
				DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_REPEAT_END_DATE_INDEX, REPEAT_EVENT_END_DATE_COL_ID, "repeat end time");
				UTCrepeatEndTimeInSecs = *((time_t*)columnValuesArray[DBK_REPEAT_END_DATE_INDEX].data);
				if (UTCrepeatEndTimeInSecs == apptNoEndTimeValueInDB)
					DateToInt(rectP->repeat.repeatEndDate) = apptNoEndDate;
				else
				{
					CnvUTC2LocalTime(&UTCrepeatEndTimeInSecs, &tmpDateTime, rectP->when.noTimeEvent);
					// Check if the date is valid
					if ((tmpDateTime.year > lastYear) ||
						(tmpDateTime.year == lastYear && tmpDateTime.month >= monthsInYear && tmpDateTime.day >= 31))
					{
						DateToInt(rectP->repeat.repeatEndDate) = apptNoEndDate;
					}
					else
					{
						rectP->repeat.repeatEndDate = CnvDateTime2Date(&tmpDateTime);
						if (DateToInt(rectP->repeat.repeatEndDate) < DateToInt(rectP->when.date))
							rectP->repeat.repeatEndDate = rectP->when.date;
					}
				}

				DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_REPEAT_FREQ_INDEX, REPEAT_EVENT_REPEAT_FREQ_COL_ID, "repeat frequency");
				rectP->repeat.repeatFrequency = *((uint8_t*)columnValuesArray[DBK_REPEAT_FREQ_INDEX].data);
				DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_REPEAT_ON_INDEX, REPEAT_EVENT_REPEAT_ON_ID, "repeat On");
				rectP->repeat.repeatOn = PrvComputeRepeatOnForLocalDate(
					*((uint8_t*)columnValuesArray[DBK_REPEAT_ON_INDEX].data), rectP);
				DbgOnlyCheckIndexAndColumnID(columnValuesArray, DBK_REPEAT_START_OF_WEEK_INDEX, REPEAT_EVENT_START_OF_WEEK_ID, "repeat start of week");
				rectP->repeat.repeatStartOfWeek = *((uint8_t*)columnValuesArray[DBK_REPEAT_START_OF_WEEK_INDEX].data);
			}
			else
				DbgOnlyFatalError("Retrieving record: Repeat info inconsistency");

			// Release the database storage
			DbReleaseStorage(dbP, columnValuesArray);
		}
	}

	// ***************************************** DBK_SELECT_EXCEPTIONS ******************************************
	if (loadingFilter & DBK_SELECT_EXCEPTIONS)
	{
		// Get exceptions column values
		err = DbGetColumnValue(dbP, rowID, EVENT_EXCEPTIONS_DATES_ID, 0 /* offset */, (void**)&exceptionDataP, &exceptionsSize);
		if (err < errNone)
		{
			DbReleaseStorage(dbP, exceptionDataP);
			if (err == dmErrNoColumnData)
			{
				// The information does not exist on DB, set as unset.
				memset(&rectP->exceptions, 0x00, sizeof(ExceptionsListType));
				err = errNone;
			}
			else
				goto Exit;
		}
		else
		{
			rectP->exceptions.numExceptions = exceptionsSize / sizeof(uint32_t);
			if (rectP->exceptions.numExceptions > 0)
			{
				rectP->exceptions.exceptionsDatesP =
					MemPtrNew(rectP->exceptions.numExceptions * sizeof(DateType));
				for (i=0; i<rectP->exceptions.numExceptions; i++)
				{
					CnvUTC2LocalTime(&(exceptionDataP[i]), &tmpDateTime, rectP->when.noTimeEvent);
					rectP->exceptions.exceptionsDatesP[i] = CnvDateTime2Date(&tmpDateTime);
				}
			}

			// Release the database storage
			DbReleaseStorage(dbP, exceptionDataP);
		}
	}

	// ***************************************** DBK_SELECT_DESCRIPTION ******************************************
	if (loadingFilter & DBK_SELECT_DESCRIPTION)
	{
		// Get description column values
		err = DbCopyColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0 /* offset */, NULL, &descSize);
		if (err < errNone)
		{
			if (err == dmErrNoColumnData)
			{
				// Let rectP->description to NULL
				err = errNone;
			}
			else
				goto Exit;
		}
		else
		{
			rectP->description = MemPtrNew(descSize);
			DbCopyColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0 /* offset */, rectP->description, &descSize);
			// Force the ending '\0': data may have been entered by a third party app that forgot it
			rectP->description[descSize-1] = chrNull;
		}
	}

	// ***************************************** DBK_SELECT_LOCATION ******************************************
	if (loadingFilter & DBK_SELECT_LOCATION)
	{
		// Get description column values
		err = DbCopyColumnValue(dbP, rowID, EVENT_LOCATION_ID, 0 /* offset */, NULL, &locationSize);
		if (err < errNone)
		{
			if (err == dmErrNoColumnData)
			{
				// Let rectP->location to NULL
				err = errNone;
			}
			else
				goto Exit;
		}
		else
		{
			rectP->location = MemPtrNew(locationSize);
			DbCopyColumnValue(dbP, rowID, EVENT_LOCATION_ID, 0 /* offset */, rectP->location, &locationSize);
			// Force the ending '\0': data may have been entered by a third party app that forgot it
			rectP->location[locationSize-1] = chrNull;
		}
	}


	// ***************************************** DBK_SELECT_NOTE ******************************************
	if (loadingFilter & DBK_SELECT_NOTE)
	{
		// Get note column values
		err = DbCopyColumnValue(dbP, rowID, EVENT_NOTE_ID, 0 /* offset */, NULL, &noteSize);
		if (err < errNone)
		{
			if (err == dmErrNoColumnData)
			{
				// Let rectP->note to NULL
				err = errNone;
			}
			else
				goto Exit;
		}
		else
		{
			rectP->note = MemPtrNew(noteSize);
			DbCopyColumnValue(dbP, rowID, EVENT_NOTE_ID, 0 /* offset */, rectP->note, &noteSize);
			// Force the ending '\0': data may have been entered by a third party app that forgot it
			rectP->note[noteSize-1] = chrNull;
		}
	}

Exit:
	return err;
}


/************************************************************************
 *
 * FUNCTION: 		ApptGetRecord
 *
 * DESCRIPTION: 	Get a read-only record from the Appointment Database
 *
 * PARAMETERS: 	-> 	dbP: 			database handle.
 *				-> 	rowID: 			record rowID or CursorID.
 *				<- 	rectP: 			database record to fill.
 *				-> 	loadingFilter:	 filter on data to load.
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	10/09/02	Initial revision.
 *	PPL 7/10/03		cursors !
 *
 ***********************************************************************/
status_t ApptGetRecord (DmOpenRef dbP, uint32_t rowID, ApptDBRecordType * rectP, RecordFilteringInfoType loadingFilter)
{
	status_t 	err;

	PIMAppProfilingBegin("Load Record in DB");

	// Initializations...
	memset(rectP, 0x00, sizeof(ApptDBRecordType));

	// When asked for repeat or exceptions, we need to know if the appointement is a no-time
	// appointment, thus we have also to load the time information.
	if (loadingFilter & DBK_SELECT_REPEAT)
		loadingFilter |= DBK_SELECT_TIME;

	if (loadingFilter & DBK_SELECT_EXCEPTIONS)
		loadingFilter |= DBK_SELECT_TIME;

	// Fill the record
	err = PrvApptFillRecord(dbP, rowID, rectP, loadingFilter);

	// Check the timezone value with the system database
	if (!TimezoneExistsInDatabase(rectP->when.timeZoneName))
	{
		// The timezone is not known by the system - Set it to Null, it will be considered
		// as the device local timezone
		rectP->when.timeZoneName[0] = chrNull;
	}

	//DbgOnlyFatalErrorIf(err < errNone,"ApptGetRecord returned error.");

	PIMAppProfilingEnd();

	return err;
}


/************************************************************************
 *
 * FUNCTION: 		ApptFreeRecordMemory
 *
 * DESCRIPTION: 	Free dynamically allocated data
 *
 * PARAMETERS: -> 	rectP: the record.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	10/09/02	Initial revision.
 *
 ***********************************************************************/
void ApptFreeRecordMemory (ApptDBRecordType * rectP)
{
	if (rectP->exceptions.exceptionsDatesP != NULL)
	{
		MemPtrFree(rectP->exceptions.exceptionsDatesP);
		rectP->exceptions.exceptionsDatesP = NULL;
		rectP->exceptions.numExceptions = 0;
	}

	if (rectP->description != NULL)
	{
		MemPtrFree(rectP->description);
		rectP->description = NULL;
	}

	if (rectP->location != NULL)
	{
		MemPtrFree(rectP->location);
		rectP->location = NULL;
	}

	if (rectP->note != NULL)
	{
		MemPtrFree(rectP->note);
		rectP->note = NULL;
	}
}


/************************************************************************
 *
 * FUNCTION: 		ApptGetDescription
 *
 * DESCRIPTION: 	Get the description of the record from DB
 *
 * PARAMETERS: 	-> 	rectP: the record
 *				->	rowID:
 *				-> 	descH:
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLE	10/09/02	Initial revision.
 *	PPL 7/10/03		cursors !
 *
 ***********************************************************************/
void ApptGetDescription (DmOpenRef dbP, uint32_t rowID, MemHandle descH)
{
	status_t	err;
	char * 		descP;
	uint32_t	descSize = 0;

	// Get the size of column
	err = DbCopyColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0, NULL, &descSize);

	// If no description...
	if ((err < errNone) || (descSize == 0))
	{
		// Force descSize to 1 to assign '\0'
		descSize = 1;
	}

   //MemHandleLockCount() is a PDK-only API. Since this is only for debug purpose,
   // I'm just commenting it out
	// Display the lock count !!!
	//TraceOutput(TL(appErrorClass, "Get description: rowID = %lx, Handle = %lx, LockCount = %d",
	//	rowID, descH, MemHandleLockCount(descH)));

	// Resize if needed
	if (descSize > MemHandleSize(descH))
		MemHandleResize(descH, descSize);

	// Assign value or nullChr
	descP = MemHandleLock(descH);

	if (descSize > 1)
		DbCopyColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0, descP, &descSize);
	else
		descP[0] = '\0';

	MemHandleUnlock(descH);
}


/************************************************************************
 *
 * FUNCTION: 		ApptChangeDescription
 *
 * DESCRIPTION: 	Updates the description of the record directly in DB.
 *
 * PARAMETERS: 	-> 	rectP: the record
 *				->	rowID:
 *				-> 	descH:
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 * 	PPL	10/09/02	Initial revision.
 *	PPL 7/10/03		cursors !
 *	PPL	08/26/04	change the ErrNonFatalAlertIf() for  TraceOutput().
 *
 ***********************************************************************/
void ApptChangeDescription (DmOpenRef dbP, uint32_t rowID, MemHandle descH)
{
	status_t	err;
	char * 		descP;
	uint32_t	descSize;


	descP = MemHandleLock(descH);
	descSize = (uint32_t) (strlen(descP) + 1);

	err = DbWriteColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0 /* Offset */,
			-1 /* => replace entire column */, descP, descSize);

	ErrNonFatalDisplayIf(err < errNone, "Failed in updating description");
	TraceOutput(TL(appErrorClass,"Change description: rowID = %lx, Handle = %lx, error = %lx", rowID, descH, err));

	MemHandleUnlock(descH);
}


/************************************************************************
 *
 * FUNCTION: 		ApptChangeLocation
 *
 * DESCRIPTION: 	Updates the location of the record. Allocate
 *					a new buffer and assign it to the location field.
 *
 * PARAMETERS:
 *				-> 	rectP: 			the record
 *				-> 	newLocationP:
 *				->	changedFieldsP:
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	PLE	10/09/02	Initial revision.
 *
 ***********************************************************************/
void ApptChangeLocation (ApptDBRecordType * rectP, char* newLocationP, RecordFilteringInfoType * changedFieldsP)
{
	// Free the previous description is it existed
	if (rectP->location != NULL)
	{
		MemPtrFree(rectP->location);
	}

	// Assign new description
	if (newLocationP != NULL)
	{
		rectP->location = MemPtrNew ((uint32_t)(strlen(newLocationP) + 1));
		strcpy(rectP->location, newLocationP);
	}
	else
		rectP->location = NULL;

	*changedFieldsP |= DBK_SELECT_LOCATION;
}


/************************************************************************
 *
 * FUNCTION: 		ApptRepeatEventIsFinished
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	-> 	dbP: pointer on the DB
 *				-> 	rowID: record rowID or CursorID
 *				-> 	checkDate: compare this date to end repeating date
 *
 * RETURNED: 		True if the event occurs only in the past.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
Boolean ApptRepeatEventIsFinished (DmOpenRef dbP, uint32_t rowID, time_t endCheckDateInUTCSecs)
{
	time_t 		repeatEndDate;
	uint32_t 	repeatEndDateSize = sizeof(uint32_t);

	if (DbCopyColumnValue(dbP, rowID, REPEAT_EVENT_END_DATE_COL_ID, 0, (void*) &repeatEndDate, &repeatEndDateSize) < errNone)
		return false;

	if (repeatEndDate >= endCheckDateInUTCSecs)
		return false;

	return true;
}


/************************************************************************
 *
 * FUNCTION: 		ApptEventRepeatIsSet
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	dbP:	Opened Database Ref.
 *				-> 	rowID: 	Cursor rowID or CursorID.
 *
 * RETURNED: 		True if the event repeat is set.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
Boolean ApptEventRepeatIsSet (DmOpenRef dbP, uint32_t rowID)
{
	Boolean repeatSet;
	uint32_t repeatSize = sizeof(Boolean);

	if (DbCopyColumnValue(dbP, rowID, REPEATING_EVENT_COL_ID, 0 /* offset */, (void*) &repeatSet, &repeatSize) < errNone)
		return false;

	return repeatSet;
}


/************************************************************************
 *
 * FUNCTION: 		ApptEventTZDiffersFromSystemTZ
 *
 * DESCRIPTION:
 *
 * PARAMETERS: ->	dbP:	Opened Database Ref.
 *				-> 	rowID: 	The record row ID of a cursor ID
 *							(current row id)
 *
 * RETURNED: 		True if the event time zone is not the system one.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
Boolean ApptEventTZDiffersFromSystemTZ (DmOpenRef dbP, uint32_t rowID)
{
	Boolean			eventTZDiffersFromSystem = false;
	char			eventTZName[TZNAME_MAX];
	uint32_t		eventTZSize = TZNAME_MAX;
	Boolean			noTimeEvent;
	uint32_t		noTimeEventSize = sizeof(Boolean);
	status_t		err;

	// Check that it's not a no-time event
	err = DbCopyColumnValue(dbP, rowID, UNTIMED_EVENT_COL_ID, 0 /* offset */, (void*)(&noTimeEvent), &noTimeEventSize);
	if ((err < errNone) || noTimeEvent)
	{
		// Unable to check the no-time status, or no-time event, consider it as device timezone.
		return false;
	}

	// Get the timezone value
	err = DbCopyColumnValue(dbP, rowID, EVENT_TIMEZONE_COL_ID, 0 /* offset */,
		(void*) eventTZName, &eventTZSize);

	// If the timezone information is not set - Consider it is device timezone
	if (err < errNone)
		return false;

	// If the appointment timezone is not known by the system, Consider also that
	// this timezone is the device one.
	if (!TimezoneExistsInDatabase(eventTZName))
		return false;

	// Compare values with system ones
	if (StrNCompareAscii(eventTZName, gDeviceTimeZone, TZNAME_MAX))
		eventTZDiffersFromSystem = true;

	return eventTZDiffersFromSystem;
}


/************************************************************************
 *
 * FUNCTION: 		ApptEventHasDescription
 *
 * DESCRIPTION: 	Get the description of the record from DB
 *
 * PARAMETERS: 	->	dbP:	Opened Database Ref.
 *				-> 	rectP: 	the record.
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLE 10/09/02	Initial revision.
 *
 ***********************************************************************/
Boolean ApptEventHasDescription (DmOpenRef dbP, uint32_t rowID)
{
	uint32_t	descSize = 0;

	// Get the size of column
	if (DbCopyColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0, NULL, &descSize) < errNone)
		return false;

	// If the size is 0 or 1 (ending '\0'), the description is empty
	if (descSize <= 1)
		return false;

	return true;
}


/************************************************************************
 *
 * FUNCTION: ApptEventAlarmIsSet
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	dbP:	Opened Database Ref.
 *				-> 	rowID: 	the record row Id of a cursor Id
 *							(current row id.)
 *
 * RETURNED: 		True if the event alarm is set.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
Boolean ApptEventAlarmIsSet (DmOpenRef dbP, uint32_t rowID)
{
	int8_t 		alarmAdvance;
	uint32_t 	alarmSize = sizeof(int8_t);

	if (DbCopyColumnValue(dbP, rowID, EVENT_ALARM_ADVANCE_COL_ID, 0 /* offset */, (void*) &alarmAdvance, &alarmSize) < errNone)
		return false;

	if (alarmAdvance == apptNoAlarm)
		return false;

	return true;
}


/************************************************************************
 *
 * FUNCTION: 		ApptEventHasNote
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	dbP:	Opened Database Ref.
 *				-> 	rowID: 	The record row Id of a cursor Id
 *							(current row id.)
 *
 * RETURNED: 		True if the record note exists.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
Boolean ApptEventHasNote (DmOpenRef dbP, uint32_t rowID)
{
	uint32_t noteSize;

	if (DbCopyColumnValue(dbP, rowID, EVENT_NOTE_ID, 0 /* offset */, NULL, &noteSize) < errNone)
		return false;

	return (Boolean) (noteSize > 0);
}


/************************************************************************
 *
 * FUNCTION: 		ApptDeleteNote
 *
 * DESCRIPTION:
 *
 * PARAMETERS: ->	dbP:	Opened Database Ref.
 *				->	rowID: 	Record row ID of a cursor ID (current row id.)
 *
 *
 * RETURNED: 		Nothing.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
void ApptDeleteNote (DmOpenRef dbP, uint32_t rowID)
{
	DbWriteColumnValue(dbP, rowID, EVENT_NOTE_ID, 0 /* Offset */, -1 /* => delete entire column */, NULL, 0);
}


/************************************************************************
 *
 * FUNCTION: 		ApptDeleteRecord
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	-> 	dbP:	database open ref.
 *				-> 	rowID: 	cursor Id / row id.
 *
 * RETURNED: 		errNone if success.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
status_t ApptDeleteRecord (DmOpenRef dbP, uint32_t rowID, Boolean archive)
{
	status_t err;

	if (archive)
		err = DbArchiveRow (dbP, rowID);
	else
		err = DbDeleteRow (dbP, rowID);

	// Mark the database as dirty so we will requery the cursor before using it again
	sDateDBupdated = true;

	return err;
}

/************************************************************************
 *
 * FUNCTION: 		ApptUpdateRecordWithNewColumns
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	-> 	dbP:	database open ref.
 *				-> 	rowID: 	cursor Id / row id.
 *
 * RETURNED: 		errNone if success.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
status_t ApptUpdateRecordWithNewColumns (DmOpenRef dbP, uint32_t cursorID, VCardSenderKindType receivedVCardType,
		uint32_t fromRowID, uint32_t toRowID)
{
	status_t 					err = errNone;
	uint32_t 					allOS5ColumnIDsArray[] = DATEBOOK_NON_REQUIRED_PALM_OS_5_COLUMNS;
	uint32_t 					allOS5NumColumns = sizeof(allOS5ColumnIDsArray)/sizeof(uint32_t);
	uint32_t 					addOS6ColumnIDsArray[] = DATEBOOK_ADDITIONNAL_PALM_OS_6_COLUMNS;
	uint32_t 					addOS6NumColumns = sizeof(addOS6ColumnIDsArray)/sizeof(uint32_t);
	uint32_t					newCols;
	DbSchemaColumnValueType * 	newColValsP = NULL;
	uint32_t					i;

	// First, delete Palm OS 5 columns from destination record
	err = DbCursorMoveToRowID(cursorID, toRowID);
	if (err < errNone)
		goto Exit;

	for (i = 0; i < allOS5NumColumns; i++)
	{
		err = DbWriteColumnValue(dbP, cursorID, allOS5ColumnIDsArray[i], 0 /* Offset */, -1 /* => delete entire column */, NULL, 0);
		if (err < errNone)
			goto Exit;
	}

	// If we imported from PalmOS 6, also remove new columns
	if (receivedVCardType == kPalmOS6AndAfter)
	{
		for (i = 0; i < addOS6NumColumns; i++)
		{
			err = DbWriteColumnValue(dbP, cursorID, addOS6ColumnIDsArray[i], 0 /* Offset */, -1 /* => delete entire column */, NULL, 0);
			if (err < errNone)
				goto Exit;
		}
	}

	// Then copy from initial record to destination one
	DbGetAllColumnValues(dbP, fromRowID, &newCols, &newColValsP);
	if (err < errNone)
		goto Exit;

	// Cursor is still on toRowID row
	err = DbWriteColumnValues(dbP, cursorID, newCols, newColValsP);

	sDateDBupdated = true;

Exit:
	// Free column data
	if (newColValsP)
		DbReleaseStorage(dbP, newColValsP);

	return err;
}

/************************************************************************
 *
 * FUNCTION: 		ApptIsRecordEmpty
 *
 * DESCRIPTION:		Tells if a record is considered empty.
 *					(From an application stand point.)
 *
 * PARAMETERS: 	-> 	dbP:		Database open ref.
 *				-> 	cursorID: 	Cursor Id (current row id.)
 *
 * RETURNED: 		true is the record was empty and has been deleted.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PPL	07/06/04	Initial revision.
 *					Splitted from ApptDeleteRecordIfEmpty.
 *
 ***********************************************************************/
Boolean ApptIsRecordEmpty(DmOpenRef dbP, uint32_t cursorID)
{
	Boolean empty = true;
	// If the description field is empty there is no note, delete
	// the record.
	if (ApptEventHasDescription(dbP, cursorID))
		empty = false;
	else if (ApptEventHasNote(dbP, cursorID))
		empty = false;
	else if ( ApptEventRepeatIsSet (dbP, cursorID))
		empty = false;

	return empty;
}


/************************************************************************
 *
 * FUNCTION: 		ApptDeleteRecordIfEmpty
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	-> 	dbP:		Database open ref.
 *				-> 	cursorID: 	Cursor Id (current row id.)
 *
 * RETURNED: 		true is the record was empty and has been deleted.
 *
 * NOTE:		 	None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
  *	PPL	07/06/04	Extract ApptIsRecordEmpty.
 *
 ***********************************************************************/
Boolean ApptDeleteRecordIfEmpty (DmOpenRef dbP, uint32_t cursorID)
{
	Boolean 			empty = true;
	Boolean 			hasAlarm;
	uint32_t			alarmRef;	// Ignored. Needed for AlmGetAlarm()
	Boolean				result = false;

	// If the description field is empty there is no note, delete
	// the record.
	empty = ApptIsRecordEmpty( dbP,  cursorID);

	if (empty)
	{
		hasAlarm = ApptEventAlarmIsSet(dbP, cursorID);

		// if the event to be delete had an alarm, be sure to remove it
		// from the posted alarm queue before the event is deleted.
		if (hasAlarm)
			DeleteAlarmIfPosted(cursorID);

		// If the record was not modified, and the description and
		// note fields are empty, remove the record from the database.
		// This can occur when a new empty record is deleted.
		if (RecordDirty)
			DbDeleteRow (dbP, cursorID);
		else
			DbRemoveRow (dbP, cursorID);

		// Mark the database as dirty so we will requery the cursor before using it again
		sDateDBupdated = true;

		// If the appointment had the currently scheduled alarm, reschedule the next alarm
		if (hasAlarm)
		{
			uint32_t alarmTrigger = AlmGetAlarm (gApplicationDbID, &alarmRef);
			uint32_t rowID;

			if (DbIsCursorID(cursorID))
				DbCursorGetCurrentRowID(cursorID, &rowID);
			else
				rowID = cursorID;

			if (alarmTrigger && (alarmRef == rowID))
				RescheduleAlarms (dbP);
		}
		result = true;
	}

	return result;
}


/************************************************************************
 *
 * FUNCTION: 		ApptGetSchemaColumnMaxSize.
 *
 * DESCRIPTION: 	Retrieve from the schema the maximum
 *					size for specified column.
 *
 * PARAMETERS: 	->	dbP:		Opened database ref.
 *				-> 	columnID: 	The column ID.
 *
 * RETURNED: 		Maximum size for this column.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *
 ***********************************************************************/
uint32_t ApptGetSchemaColumnMaxSize(DmOpenRef dbP, uint32_t columnID)
{
	DbSchemaColumnDefnType * 	thisColumnDefinitionP = NULL;
	uint32_t 					thisColumnMaxSize = 0;

	if (DbGetColumnDefinitions(dbP, DatebookApptTable, 1, &columnID, &thisColumnDefinitionP) >= errNone)
	{
		DbgOnlyFatalErrorIf(thisColumnDefinitionP[0].id != columnID,
			"Retrieved wrong column ID");
		thisColumnMaxSize = thisColumnDefinitionP[0].maxSize;
		DbReleaseStorage(dbP, thisColumnDefinitionP);
	}

	return thisColumnMaxSize;
}


/************************************************************************
 *
 * FUNCTION: 		PrvApptChangeRecord
 *
 * DESCRIPTION: 	Change a record in the Appointment Database
 *
 * PARAMETERS: 	-> 	dbP : database pointer
 *				-> 	rowID: the record row ID of a cursor ID (current row id)
 *				-> 	apptRecP	: database record
 *				-> 	changedFields : changed fields
 *
 * RETURNED: 		errNone if successful, errorcode if not.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
static status_t PrvApptChangeRecord (
	DmOpenRef 				dbP,
	uint32_t 				rowID,
	ApptDBRecordType * 		apptRecP,
	RecordFilteringInfoType changedFields)
{
	status_t				err = errNone;
	uint32_t				i;
	uint32_t				exceptionsSize;
	uint32_t 				timeColumnIDsArray[] = DATEBOOK_TIME_INFORMATION;
	uint32_t 				timeNumColumns = sizeof(timeColumnIDsArray)/sizeof(uint32_t);
	DbSchemaColumnValueType timeColumnValues[sizeof(timeColumnIDsArray)/sizeof(uint32_t)];
	uint32_t 				alarmColumnIDsArray[] = DATEBOOK_ALARM_INFORMATION;
	uint32_t 				alarmNumColumns = sizeof(alarmColumnIDsArray)/sizeof(uint32_t);
	DbSchemaColumnValueType alarmColumnValues[sizeof(timeColumnIDsArray)/sizeof(uint32_t)];
	uint32_t 				repeatColumnIDsArray[] = DATEBOOK_REPEAT_INFORMATION;
	uint32_t 				repeatNumColumns = sizeof(repeatColumnIDsArray)/sizeof(uint32_t);
	DbSchemaColumnValueType repeatColumnValues[sizeof(repeatColumnIDsArray)/sizeof(uint32_t)];
	DateTimeType 			tmpDateTime;
	time_t *				exceptionsP;
	time_t					tmpStartDateTime;
	time_t					tmpEndDateTime;
	Boolean					repeatSet;
	uint8_t 				repeatOn;


	// ***************************************** DBK_SELECT_TIME ******************************************
	if (changedFields & DBK_SELECT_TIME)
	{
		// Assign column IDs
		for (i=0; i<timeNumColumns; i++)
			timeColumnValues[i].columnID = timeColumnIDsArray[i];

		// Untimed flag
		timeColumnValues[DBK_WHEN_UNTIMED_INDEX].dataSize = sizeof(uint8_t);
		timeColumnValues[DBK_WHEN_UNTIMED_INDEX].data = (DbSchemaColumnData*) (&apptRecP->when.noTimeEvent);

		// Start date & time
		CnvDate2DateTime(&apptRecP->when.date, &apptRecP->when.startTime, &tmpDateTime);
		CnvLocalTime2UTC(&tmpDateTime, &tmpStartDateTime, apptRecP->when.noTimeEvent);
		timeColumnValues[DBK_WHEN_START_INDEX].dataSize = sizeof(time_t);
		timeColumnValues[DBK_WHEN_START_INDEX].data = (DbSchemaColumnData*) &tmpStartDateTime;

#ifndef PIM_APPS_PROFILING
		TraceOutput(TL(appErrorClass,"Writing Record: rowID = %lx, startTime = %lx", rowID, tmpStartDateTime));
#endif

		// End date & time
		CnvDate2DateTime(&apptRecP->when.date, &apptRecP->when.endTime, &tmpDateTime);
		if (apptRecP->when.midnightWrapped)
			TimAdjust(&tmpDateTime, daysInSeconds);
		CnvLocalTime2UTC(&tmpDateTime, &tmpEndDateTime, apptRecP->when.noTimeEvent);
		timeColumnValues[DBK_WHEN_END_INDEX].dataSize = sizeof(time_t);
		timeColumnValues[DBK_WHEN_END_INDEX].data = (DbSchemaColumnData*) &tmpEndDateTime;

		// Timezone name: fill column only if this is a timed event
		if (apptRecP->when.noTimeEvent)
		{
			timeColumnValues[DBK_WHEN_TIMEZONE_INDEX].data = NULL;
		}
		else
		{
			timeColumnValues[DBK_WHEN_TIMEZONE_INDEX].dataSize = (uint32_t) (strlen(apptRecP->when.timeZoneName)+1);
			timeColumnValues[DBK_WHEN_TIMEZONE_INDEX].data = (DbSchemaColumnData*) apptRecP->when.timeZoneName;
		}

		// Set time column values
		err = DbWriteColumnValues(dbP, rowID, timeNumColumns, timeColumnValues);

		if (err < errNone)
			goto Exit;
	}

	// ***************************************** DBK_SELECT_ALARM ******************************************
	if (changedFields & DBK_SELECT_ALARM)
	{
		// Assign column IDs
		alarmColumnValues[DBK_ALARM_ADVANCE_INDEX].columnID = EVENT_ALARM_ADVANCE_COL_ID;
		alarmColumnValues[DBK_ALARM_ADV_UNIT_INDEX].columnID = EVENT_ALARM_ADVANCE_UNIT_COL_ID;

		// Check if the alarm is set - otherwise delete the column info
		if (apptRecP->alarm.advance != apptNoAlarm)
		{
			// Set advance
			alarmColumnValues[DBK_ALARM_ADVANCE_INDEX].dataSize = sizeof(int8_t);
			alarmColumnValues[DBK_ALARM_ADVANCE_INDEX].data = (DbSchemaColumnData*) (&apptRecP->alarm.advance);
			alarmColumnValues[DBK_ALARM_ADV_UNIT_INDEX].dataSize = sizeof(uint8_t);
			alarmColumnValues[DBK_ALARM_ADV_UNIT_INDEX].data = (DbSchemaColumnData*) (&apptRecP->alarm.advanceUnit);

			// Set alarm column values
			err = DbWriteColumnValues(dbP, rowID, alarmNumColumns, alarmColumnValues);
			if (err < errNone)
				goto Exit;
		}
		else
		{
			// Delete the columns - Discard return value (the values may not previously exist)
			alarmColumnValues[DBK_ALARM_ADVANCE_INDEX].data = NULL;
			alarmColumnValues[DBK_ALARM_ADV_UNIT_INDEX].data = NULL;
			DbWriteColumnValues(dbP, rowID, alarmNumColumns, alarmColumnValues);
		}
	}

	// ***************************************** DBK_SELECT_REPEAT ******************************************
	if (changedFields & DBK_SELECT_REPEAT)
	{
		// Assign column IDs
		for (i=0; i<repeatNumColumns; i++)
			repeatColumnValues[i].columnID = repeatColumnIDsArray[i];

		repeatSet = (Boolean) (apptRecP->repeat.repeatType != repeatNone);

		// Check if the repeat is set - otherwise delete the column info
		if (repeatSet)
		{
			repeatColumnValues[DBK_REPEATING_EVENT_INDEX].dataSize = sizeof(Boolean);
			repeatColumnValues[DBK_REPEATING_EVENT_INDEX].data = (DbSchemaColumnData*) (&repeatSet);

			repeatColumnValues[DBK_REPEAT_TYPE_INDEX].dataSize = sizeof(uint8_t);
			repeatColumnValues[DBK_REPEAT_TYPE_INDEX].data = (DbSchemaColumnData*) (&apptRecP->repeat.repeatType);

			repeatColumnValues[DBK_REPEAT_END_DATE_INDEX].dataSize = sizeof(uint32_t);

			if (DateToInt(apptRecP->repeat.repeatEndDate) == apptNoEndDate)
			{
				tmpStartDateTime = apptNoEndTimeValueInDB;
			}
			else
			{
				// Link the end date with the event start time
				CnvDate2DateTime(&apptRecP->repeat.repeatEndDate, &apptRecP->when.startTime, &tmpDateTime);
				CnvLocalTime2UTC(&tmpDateTime, &tmpStartDateTime, apptRecP->when.noTimeEvent);
			}
			repeatColumnValues[DBK_REPEAT_END_DATE_INDEX].data = (DbSchemaColumnData*) &tmpStartDateTime;

			repeatColumnValues[DBK_REPEAT_FREQ_INDEX].dataSize = sizeof(uint8_t);
			repeatColumnValues[DBK_REPEAT_FREQ_INDEX].data = (DbSchemaColumnData*) (&apptRecP->repeat.repeatFrequency);

			repeatColumnValues[DBK_REPEAT_ON_INDEX].dataSize = sizeof(uint8_t);
			repeatOn = PrvComputeRepeatOnForUTCDate(apptRecP);
			repeatColumnValues[DBK_REPEAT_ON_INDEX].data = (DbSchemaColumnData*) &repeatOn;


			repeatColumnValues[DBK_REPEAT_START_OF_WEEK_INDEX].dataSize = (uint32_t) sizeof(uint8_t);
			repeatColumnValues[DBK_REPEAT_START_OF_WEEK_INDEX].data =
				(DbSchemaColumnData*) (&apptRecP->repeat.repeatStartOfWeek);
		}
		else
		{
			// Delete the columns - Discard return value (the values may not previously exist)
			for (i=0; i<repeatNumColumns; i++)
				repeatColumnValues[i].data = NULL;
			// Set the REPEATING_EVENT_COL_ID column to false
			repeatColumnValues[DBK_REPEATING_EVENT_INDEX].dataSize = sizeof(Boolean);
			repeatColumnValues[DBK_REPEATING_EVENT_INDEX].data = (DbSchemaColumnData*) &repeatSet;
		}

		// Set repeat column values
		err = DbWriteColumnValues(dbP, rowID, repeatNumColumns, repeatColumnValues);

		if (err < errNone)
			goto Exit;
	}

	// ***************************************** DBK_SELECT_EXCEPTIONS ******************************************
	if (changedFields & DBK_SELECT_EXCEPTIONS)
	{
		// Check if some exceptions are set - otherwise delete the column info
		if (apptRecP->exceptions.numExceptions > 0)
		{
			exceptionsSize = apptRecP->exceptions.numExceptions * sizeof(time_t);
			exceptionsP = MemPtrNew(exceptionsSize);
			if (exceptionsP == NULL)
			{
				err = memErrNotEnoughSpace;
				goto Exit;
			}
			for (i=0; i<apptRecP->exceptions.numExceptions; i++)
			{
				CnvDate2DateTime(&(apptRecP->exceptions.exceptionsDatesP[i]), &apptRecP->when.startTime,
					&tmpDateTime);
				CnvLocalTime2UTC(&tmpDateTime, &(exceptionsP[i]), apptRecP->when.noTimeEvent);
			}

			// Set exceptions column values
			err = DbWriteColumnValue(dbP, rowID, EVENT_EXCEPTIONS_DATES_ID, 0 /* Offset */,
				-1 /* => replace entire column */, exceptionsP, exceptionsSize);

			MemPtrFree(exceptionsP);
			if (err < errNone)
				goto Exit;
		}
		else
		{
			// Delete the columns - Discard return value (the values may not previously exist)
			DbWriteColumnValue(dbP, rowID, EVENT_EXCEPTIONS_DATES_ID, 0 /* Offset */,
				-1 /* => delete entire column */, NULL, 0);
		}
	}

	// ***************************************** DBK_SELECT_DESCRIPTION ******************************************
	if (changedFields & DBK_SELECT_DESCRIPTION)
	{
		if (apptRecP->description != NULL)
		{
			// Set description column values
			err = DbWriteColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0 /* Offset */,
				-1 /* => replace entire column */, apptRecP->description, (uint32_t)(strlen(apptRecP->description)+1));
			if (err < errNone)
				goto Exit;
		}
		else
		{
			// Delete the column - Discard return value (the values may not previously exist)
			DbWriteColumnValue(dbP, rowID, EVENT_DESCRIPTION_ID, 0 /* Offset */, -1, NULL, 0);
		}
	}

	// ***************************************** DBK_SELECT_LOCATION ******************************************
	if (changedFields & DBK_SELECT_LOCATION)
	{
		if (apptRecP->location != NULL)
		{
			// Set location column values
			err = DbWriteColumnValue(dbP, rowID, EVENT_LOCATION_ID, 0 /* Offset */,
				-1 /* => replace entire column */, apptRecP->location, (uint32_t)(strlen(apptRecP->location)+1));
			if (err < errNone)
				goto Exit;
		}
		else
		{
			// Delete the column - Discard return value (the values may not previously exist)
			DbWriteColumnValue(dbP, rowID, EVENT_LOCATION_ID, 0 /* Offset */, -1, NULL, 0);
		}
	}

	// ***************************************** DBK_SELECT_NOTE ******************************************
	if (changedFields & DBK_SELECT_NOTE)
	{
		if (apptRecP->note != NULL)
		{
			// Set location column values
			err = DbWriteColumnValue(dbP, rowID, EVENT_NOTE_ID, 0 /* Offset */,
				-1 /* => replace entire column */, apptRecP->note, (uint32_t)(strlen(apptRecP->note)+1));
		}
		else
		{
			// Delete the column - Discard return value (the values may not previously exist)
			DbWriteColumnValue(dbP, rowID, EVENT_NOTE_ID, 0 /* Offset */, -1, NULL, 0);
		}
	}

Exit:
	return err;
}


/************************************************************************
 *
 * FUNCTION: 		ApptChangeRecord
 *
 * DESCRIPTION: 	Change a record in the Appointment Database
 *
 * PARAMETERS: 	-> 	dbP: 			database pointer
 *				-> 	rowID: 			record row Id or Cursor Id
 *				-> 	apptRecP: 		database record
 *				-> 	changedFields: 	changed fields
 *
 * RETURNED: 		errNone if successful, errorcode if not
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
status_t ApptChangeRecord (
	DmOpenRef 					dbP,
	uint32_t 					rowID,
	ApptDBRecordType * 			apptRecP,
	RecordFilteringInfoType 	changedFields)
{
	status_t					err = errNone;
	uint32_t					initialChangedFields = changedFields;

	if (changedFields == 0)
		goto Exit;

	// Fullfill the record if needed.
	if (changedFields & DBK_SELECT_TIME)
	{
		if (apptRecP->repeat.repeatType != repeatNone)
			changedFields |= DBK_SELECT_REPEAT;

		if (apptRecP->exceptions.numExceptions > 0)
			changedFields |= DBK_SELECT_EXCEPTIONS;
	}
	else
	{
		if (changedFields & DBK_SELECT_REPEAT)
			changedFields |= DBK_SELECT_TIME;

		if (changedFields & DBK_SELECT_EXCEPTIONS)
			changedFields |= DBK_SELECT_TIME;
	}

	// If some information was missing, load it now without overwritting updated one
	if (initialChangedFields != changedFields)
	{
		err = PrvApptFillRecord(dbP, rowID, apptRecP, changedFields ^ initialChangedFields);
		if (err < errNone)
			goto Exit;
	}

	// Update columns we asked for AND linked columns
	err = PrvApptChangeRecord(dbP, rowID, apptRecP, changedFields);

	// Release allocated resources
	ApptFreeRecordMemory(apptRecP);

	// Clean the record to avoid using it after ChangeRecord: times have been moved to UTC
	memset(apptRecP, 0x00, sizeof(ApptDBRecordType));

	// If the repeat or time information have changed, requery to get the cursor resorted
	// as they are keys in our sort id (order by)
	if (changedFields & (DBK_SELECT_TIME | DBK_SELECT_REPEAT))
		sDateDBupdated = true;

#ifndef PIM_APPS_PROFILING
	TraceOutput(TL(appErrorClass,"End of ApptChangeRecord for record = %d", rowID));
#endif

Exit:
	DbgOnlyFatalErrorIf(err < errNone,"PrvApptChangeRecord returned error.");
	return err;
}


/************************************************************************
 *
 * FUNCTION: 		ApptListTrace
 *
 * DESCRIPTION: 	Traces all the record whithin the appointement list.
 *
 * PARAMETERS: 	-> 	days: 		Number of Days.
 *				-> 	apptLists: 	Appointements'list.
 *				-> 	counts:
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			Available Only when Debugging and PIMS
 *					profilling traces are enabled.
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision.
 *
 ***********************************************************************/

#if ((TRACE_OUTPUT == TRACE_OUTPUT_ON) && (!defined(PIM_APPS_PROFILING)))
void ApptListTrace (int32_t days, MemHandle apptLists [], int32_t counts [])
{
	uint16_t 		index;
	uint16_t		i;
	ApptInfoPtr 	apptList;

	for (index = 0; index < days; index ++)
	{
		if (apptLists[index])
		{
			apptList = MemHandleLock(apptLists[index]);

			for (i = 0; i < counts[index]; i++)
			{
				TraceOutput(TL(appErrorClass,"Day[%d] - index = %d, RowID = %lx",
					index, i, apptList[i].rowID ));
			}

			MemHandleUnlock (apptLists[index]);
			MemHandleResize (apptLists[index], (uint32_t)(counts[index] * sizeof (ApptInfoType)));
		}
	}
}
#endif


/************************************************************************
 *
 * FUNCTION: 		ApptCursorTrace
 *
 * DESCRIPTION: 	Traces all the record whithin the cursor.
 *
 * PARAMETERS: 	->	dbP:		database reference.
 *				-> 	cursorID: 	Cursor to trace.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			Available Only when Debugging and PIMS
 *					profilling traces are enabled.
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision.
 *
 ***********************************************************************/

#if ((TRACE_OUTPUT == TRACE_OUTPUT_ON) && (!defined(PIM_APPS_PROFILING)))
void ApptCursorTrace(DmOpenRef dbP, uint32_t cursorID)
{
	uint32_t 		currSel;
	uint32_t 		testRowID;
	uint32_t 		testPos;
	Boolean 		repeatVal;
	uint32_t 		repeatSize = sizeof(repeatVal);
	uint32_t 		startVal;
	uint32_t 		startSize = sizeof(startVal);
	uint32_t 		endRepeatVal = 0;
	uint32_t 		endRepeatSize = sizeof(endRepeatVal);
	uint32_t 		numCats;
	CategoryID * 	pCategoryIDs = NULL;

	// Save pos...
	if (DbCursorGetCurrentRowID(cursorID, &currSel) < errNone)
		currSel = dbInvalidRowID;

	for (DbCursorMoveFirst(cursorID) ; !DbCursorIsEOF(cursorID) ; DbCursorMoveNext(cursorID) )
	{
		DbCursorGetCurrentRowID(cursorID, &testRowID);
		DbCursorGetCurrentPosition(cursorID, &testPos);
		DbGetCategory(dbP, cursorID, &numCats, &pCategoryIDs);
		DbCopyColumnValue(dbP, cursorID, REPEATING_EVENT_COL_ID, 0 /* offset */, &repeatVal, &repeatSize);
		DbCopyColumnValue(dbP, cursorID, START_EVENT_DATE_TIME_COL_ID, 0 /* offset */, &startVal, &startSize);
		if (DbCopyColumnValue(dbP, cursorID, REPEAT_EVENT_END_DATE_COL_ID, 0 /* offset */, &endRepeatVal, &endRepeatSize) < errNone)
			endRepeatVal = 0;
		TraceOutput(TL(appErrorClass," => Pos = %ld, rowID = %lx, repeat = %d, start = %lx, endRepeat = %lx, numCats = %d, Cat[0] = %d",
			testPos, testRowID, repeatVal, startVal, endRepeatVal, numCats, (pCategoryIDs == NULL) ? 0:pCategoryIDs[0] ));
		DbReleaseStorage(dbP, pCategoryIDs);
	}
	if (currSel != dbInvalidRowID)
		DbCursorMoveToRowID(cursorID, currSel);
}
#endif


/************************************************************************
 *
 * FUNCTION: 		ApptDbCursorRequeryWithCategories
 *
 * DESCRIPTION: 	Close and reopen the cursor with the current set
 *					of categories
 *
 * PARAMETERS: -> 	dbP: 			Database pointer
 *				->	sql: 			Sql request
 *									(where clause based whithin Datebook.)
 *				->	numCategories: 	How many categories.
 *				->	categoryIDs		Categories set.
 *				-> 	cursorID : 		Cursor Id.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			This is a temporary Implementation.
 *					Waiting for the DataMgr to have a
 *
 *					DBCursorRequeryWithCategories(
 *							uint32_t			cursor_id)
 *							const char* 		sql,
 *							uint32_t			numCategories,
 *						 	const CategoryID 	categoryIDs[]);
 *
 *
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision.
 *
 ***********************************************************************/
static status_t ApptDbCursorRequeryWithCategories (
	DmOpenRef			dbP,
	const char* 		sql,
	uint32_t			numCategories,
 	const CategoryID 	categoryIDs[],
	uint32_t* 			cursor_id)
{
	status_t			err = errNone;

	//ErrNonFatalDisplay("DbCursorRequery with categories filtering not yet implemented");

	err = ApptCloseCursor(cursor_id);

	err = DbCursorOpenWithCategory(
						dbP,
						sDateDBsqlQuery,
						sCursorFlags,  // flags
						sSqlQueryCategoriesCount,
						&sSqlQueryCategory,
						DbMatchAny,
						cursor_id);
	return err;

}

/************************************************************************
 *
 * FUNCTION: 		ApptDBOpenOrRequeryWithNewRange
 *
 * DESCRIPTION: 	Rebuilds the cursor to append newly inserted rows.
 *
 * PARAMETERS: 	-> 	dbP: 				Database pointer.
 *				-> 	cursorID: 			Cursor Id.
 *				-> 	rangeStartDate:
 *				->	rangeEndDate:
 *				-> 	queryAllCategories:
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *
 ***********************************************************************/
status_t ApptDBOpenOrRequeryWithNewRange(DmOpenRef dbP, uint32_t * cursorIDP, time_t rangeStartDate, time_t rangeEndDate, Boolean queryAllCategories)
{
	status_t 	err = errNone;
	Boolean 	whereClauseChanged = false;
	Boolean 	categoriesChanged = false;
	uint32_t 	currRowID;

	// If the cursor already existed, get the current rowID
	if ((*cursorIDP == dbInvalidCursorID)
		|| ((err = DbCursorGetCurrentRowID(*cursorIDP, &currRowID)) < errNone))
		currRowID = dbInvalidRowID;

	// Compute the new sql query if it has changed
	if ((rangeStartDate != sSqlQueryRangeStartDate)
		|| (rangeEndDate != sSqlQueryRangeEndDate))
	{
		// Update the where clause range
		sSqlQueryRangeStartDate = rangeStartDate;
		sSqlQueryRangeEndDate = rangeEndDate;

		ApptBuildSQLRequestWithRange(sDateDBsqlQuery, sSqlQueryRangeStartDate, sSqlQueryRangeEndDate);

		whereClauseChanged = true;

		#ifndef PIM_APPS_PROFILING
		TraceOutput(TL(appErrorClass,"NEW SQL = %s Cursor => %lx", sDateDBsqlQuery, *cursorIDP));
		#endif
	}

	// Checked if we asked for all categories like previous query or if the categories have changed
	if ((sSqlQueryAllCategories != queryAllCategories)
		||(!CategoriesTestEquality(
				DateBkCurrentCategoriesCount,
				DateBkCurrentCategoriesP,
				sSqlQueryCategoriesCount,
				&sSqlQueryCategory)))
	{
		sSqlQueryAllCategories = queryAllCategories;
		sSqlQueryCategoriesCount = DateBkCurrentCategoriesCount;
		sSqlQueryCategory = (DateBkCurrentCategoriesCount == 0) ? catIDUnfiled : DateBkCurrentCategoriesP[0];
		categoriesChanged = true;
	}

	// Recreate the cursor now if needed
	if (*cursorIDP == dbInvalidCursorID || whereClauseChanged || categoriesChanged)
	{
		if (*cursorIDP != dbInvalidCursorID)
			DbCursorClose(*cursorIDP);

		if (sSqlQueryAllCategories)
		{
			err = DbCursorOpen(
					dbP,
					sDateDBsqlQuery,
					sCursorFlags, // flags
					cursorIDP);
		}
		else
		{
			err = DbCursorOpenWithCategory(
					dbP,
					sDateDBsqlQuery,
					sCursorFlags, // flags
					sSqlQueryCategoriesCount,
					&sSqlQueryCategory,
					DbMatchAny,
					cursorIDP);
		}

		if (err < errNone)
		{
			ErrNonFatalDisplayIf(err == dmErrInvalidSortIndex, "Cursor opening, invalid sort order.");
			ErrNonFatalDisplay("WHERE - Error in creating the cursor");
			return err;
		}
		sDateDBupdated = false;
	}
	else if (sDateDBupdated)
	{
		if (queryAllCategories)
			err = DbCursorRequery(*cursorIDP);
		else
		{
			ApptDbCursorRequeryWithCategories(
				dbP,
				sDateDBsqlQuery,
				sSqlQueryCategoriesCount,
				&sSqlQueryCategory,
				cursorIDP);
		}
		sDateDBupdated = false;
	}

	// Restore the current rowID if it exists in new cursor (ignore result)
	if (currRowID != dbInvalidRowID)
		DbCursorMoveToRowID(*cursorIDP, currRowID);

	return err;
}

/************************************************************************
 *
 * FUNCTION: 		ApptDBReOpenCursor
 *
 * DESCRIPTION: 	Rebuilds the cursor to append newly inserted rows.
 *
 * PARAMETERS: 	-> 	dbP: 		Database pointer.
 *				<-> cursorID: 	Cursor Id.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *
 ***********************************************************************/
status_t ApptDBReOpenCursor(DmOpenRef dbP, uint32_t * cursorIDP)
{
	return ApptDBOpenOrRequeryWithNewRange(
		dbP,
		cursorIDP,
		sSqlQueryRangeStartDate,
		sSqlQueryRangeEndDate,
		sSqlQueryAllCategories);
}


/************************************************************************
 *
 * FUNCTION: 		ApptDBRequery
 *
 * DESCRIPTION: 	Rebuilds the cursor to append newly inserted rows.
 *
 * PARAMETERS:	 -> dbP: 			Database reference.
 *				-> 	cursorID: 		Cursor Id.
 *				->	forceRequery:	force cursor requery.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *
 ***********************************************************************/
void ApptDBRequery(DmOpenRef dbP, uint32_t cursorID, Boolean forceRequery)
{
	status_t err;

	#ifndef PIM_APPS_PROFILING
	TraceOutput(TL(appErrorClass,"REQUERY, RANGE = [%lx, %lx] => %d",
		sSqlQueryRangeStartDate, sSqlQueryRangeEndDate, sDateDBupdated));
	#endif

	if (sDateDBupdated || forceRequery)
	{
		err = DbCursorRequery(cursorID);
		sDateDBupdated = false;
	}

	// TESTING
	#if ((TRACE_OUTPUT == TRACE_OUTPUT_ON) && (!defined(PIM_APPS_PROFILING)))
	ApptCursorTrace(dbP, cursorID);
	#endif
}


/************************************************************************
 *
 * FUNCTION: 		ApptDBRequeryOnDate
 *
 * DESCRIPTION: 	Rebuilds the cursor for a new range of date that
 *					contain date.
 *
 * PARAMETERS: 	-> 	dbP : 		database pointer
 *				<-> cursorIDP : Cursor Id pointer.
 *				->	theDate: 	when.
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *
 ***********************************************************************/
status_t ApptDBRequeryOnDate(DmOpenRef dbP, uint32_t * cursorIDP, DateType theDate)
{
	time_t rangeStartDateSecs;
	time_t rangeEndDateSecs;

	CalculateStartEndRangeTimes(&theDate, 1, &rangeStartDateSecs, &rangeEndDateSecs, NULL);
	return ApptDBOpenOrRequeryWithNewRange(dbP, cursorIDP, rangeStartDateSecs, rangeEndDateSecs, true);
}


/************************************************************************
 *
 * FUNCTION: 		ApptDBRequeryOnCategories
 *
 * DESCRIPTION: 	Rebuilds the cursor in order to have new categories.
 *
 * PARAMETERS: 	-> 	dbP : database pointer
 *				-> 	cursorID : Cursor Id
 *
 * RETURNED: 		Nothing.
 *
 * NOTE: 			None.
 *
 * REVISION HISTORY:
 *	PPL	10/09/02	Initial revision.
 *					(after ApptDBRequeryOnDate.)
 *
 ***********************************************************************/
status_t ApptDBRequeryOnCategories(DmOpenRef dbP, uint32_t * cursorIDP)
{
	return ApptDBOpenOrRequeryWithNewRange(dbP, cursorIDP, sSqlQueryRangeStartDate, sSqlQueryRangeEndDate, true);
}


/************************************************************************
 *
 * FUNCTION: 		ApptNewRecord
 *
 * DESCRIPTION: 	Creates a new record, with the information contained
 *					in apptRecP structure. It does not update the apptRecP
 *					structure neither desallocates the dynamically allocated
 *					fields pointed by apptRecP (desc, location, note).
 *
 * PARAMETERS: 	-> 	dbP: 		database pointer.
 *				<- 	apptRecP: 	database  rowId returned record.
 *				<- 	rowIDP: 	rowID mandatory - Do not give a CursorID as
 *								argument.
 *
 * RETURNED: 		errNone if successful, error code if not.
 *
 * NOTE:			rowIDP must be valid.
 *
 * REVISION HISTORY:
 *	PLe	10/09/02	Initial revision.
 *	PPL 07/10/03	Cursors !
 *
 ***********************************************************************/
status_t ApptNewRecord (DmOpenRef dbP, ApptDBRecordType * apptRecP, uint32_t *rowIDP)
{
	status_t 					err;
	DbSchemaColumnValueType		initialRecordInfo = {	NULL,
														sizeof(Boolean),
														REPEATING_EVENT_COL_ID, 0, 0, 0};
	RecordFilteringInfoType 	changedFields = DBK_SELECT_TIME;

	ApptDBRecordType			recordInUTCTime = *apptRecP;
	Boolean						initialRepeatSet = false;

	// Creates an empty record. Only one information is set: the repeat
	// flag.
	initialRecordInfo.data = (DbSchemaColumnData*) &initialRepeatSet;
	err = DbInsertRow(dbP, DatebookApptTable, 1, &initialRecordInfo, rowIDP);
	if (err < errNone)
		goto Exit;

	// Set additional changedFields
	if (recordInUTCTime.alarm.advance != apptNoAlarm)
		changedFields |= DBK_SELECT_ALARM;

	if (recordInUTCTime.repeat.repeatType != repeatNone)
		changedFields |= DBK_SELECT_REPEAT;

	if (recordInUTCTime.exceptions.numExceptions > 0)
		changedFields |= DBK_SELECT_EXCEPTIONS;

	if (recordInUTCTime.description != NULL)
		changedFields |= DBK_SELECT_DESCRIPTION;

	if (recordInUTCTime.location != NULL)
		changedFields |= DBK_SELECT_LOCATION;

	if (recordInUTCTime.note != NULL)
		changedFields |= DBK_SELECT_NOTE;

	// Update the record information. Use the local copy because the input
	// apptRecP structure must not be modified.
	err = PrvApptChangeRecord(dbP, *rowIDP, &recordInUTCTime, changedFields);

	// Flag the DB as dirty
	sDateDBupdated = true;

#ifndef PIM_APPS_PROFILING
	TraceOutput(TL(appErrorClass,"New Record: rowID = %lx", *rowIDP));
#endif

Exit:
	DbgOnlyFatalErrorIf(err < errNone,"ApptNewRecord returned error.");
	return err;
}



/***********************************************************************
 *
 * FUNCTION:    	AddressDBDuplicateRecord
 *
 * DESCRIPTION: 	Duplicates a new record from the current record.
 *					and writes modifyed data as required.
 *
 * PARAMETERS:  ->	dbP:				Database OpenRef.
 *				->	sourceRowId:		cursorId or RowId of the record
 *										to clone.
 *				->	changedFields:		filters on field qsthan need to be saved
 *				->						again as they were changed.
 *										(comparing to database record data)
 *				->	unwantedFilter: 	filters on non wanted columns.
 *				->	apptRecP:			New appointement data.
 * 				->	newRowIDP:			Row Id of new created record.
 *
 *
 * RETURNED:    The ID of the new duplicated record.
 *
 * NOTE:		Carefull ! Filtering is enabled for UNWANTED column.
 *				All Columns present in the rrow are going to be in the clone
 *				(as it is a clone) Unwanted column will be removed.
 *
 * REVISION HISTORY:
 * 	ppl	  02/23/04   Initial Revision.
 *
 ***********************************************************************/
status_t ApptDuplicateRecord(DmOpenRef dbP, uint32_t sourceRowId, RecordFilteringInfoType changedFields, RecordFilteringInfoType unwantedFilter, ApptDBRecordType * apptRecP, uint32_t *newRowIDP)
{
	ApptDBRecordType			recordInUTCTime = *apptRecP;
	Boolean 					initialRepeatSet = false;
	uint32_t					numColumns = 0;
	DbSchemaColumnValueType		*columnValuesP = NULL;
	CategoryID *				categoriesP = NULL;
	uint32_t					numCategories = 0;
	uint16_t					recordAttr = 0;
	status_t					err;

	// first set the new Row ID to dbInvalidRowID
	// newRowIDP have to be valid.
	*newRowIDP = dbInvalidRowID;

	// Get all the columns values for the record
	err = DbGetAllColumnValues(dbP, sourceRowId, &numColumns, &columnValuesP);
	if (err < errNone)
	{
		err = errNone;
		goto Exit;
	}

	// Duplicate it.
	if ((err = DbInsertRow(dbP, DatebookApptTable, numColumns, columnValuesP, newRowIDP)) < errNone)
		goto Exit;

	// Get orig record categories
	if ((err = DbGetCategory(dbP, sourceRowId, &numCategories, &categoriesP)) >= errNone)
	{
		// Set new record categories
		err = DbSetCategory(dbP, *newRowIDP, numCategories, categoriesP);

		// Release categories list
		if (categoriesP)
			DbReleaseStorage(dbP, categoriesP);
	}

	ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");


	// in the following we check if a common appoinbtement field was unwanted
	// and then we reset it, or wanted and then we are setting the new value if any

	// Restore private flag.
	DbGetRowAttr(dbP, sourceRowId, &recordAttr);
	if ((recordAttr & dbRecAttrSecret) == dbRecAttrSecret)
	{
		DbGetRowAttr(dbP, *newRowIDP, &recordAttr);
		recordAttr |= dbRecAttrSecret;
		DbSetRowAttr(dbP, *newRowIDP, &recordAttr);
	}


	// Set additional changedFields

	if (unwantedFilter & DBK_SELECT_ALARM)
	{
		recordInUTCTime.alarm.advance = apptNoAlarm;
		changedFields |=  DBK_SELECT_ALARM;
	}


	if (unwantedFilter & DBK_SELECT_REPEAT)
	{
		recordInUTCTime.repeat.repeatType = repeatNone;
		changedFields |=  DBK_SELECT_REPEAT;
	}


	if (unwantedFilter & DBK_SELECT_EXCEPTIONS)
	{
		recordInUTCTime.exceptions.numExceptions = 0;
		changedFields |=  DBK_SELECT_EXCEPTIONS;
	}


	if (unwantedFilter & DBK_SELECT_DESCRIPTION)
	{
		recordInUTCTime.description = NULL;
		changedFields |=  DBK_SELECT_DESCRIPTION;
	}


	if (unwantedFilter & DBK_SELECT_LOCATION)
	{
		recordInUTCTime.location = NULL;
		changedFields |= DBK_SELECT_LOCATION;
	}


	if (unwantedFilter & DBK_SELECT_NOTE)
	{
		recordInUTCTime.note = NULL;
		changedFields |=  DBK_SELECT_NOTE;
	}



	// Update the record information. Use the local copy because the input
	// apptRecP structure must not be modified.
	err = PrvApptChangeRecord(dbP, *newRowIDP, &recordInUTCTime, changedFields);

	// Flag the DB as dirty
	sDateDBupdated = true;


#ifndef PIM_APPS_PROFILING
	TraceOutput(TL(appErrorClass,"Duplicate Record: rowID/cursorId = %lx, newRowID = %lx", sourceRowId, *newRowIDP));
#endif

Exit:
	DbgOnlyFatalErrorIf(err < errNone, "ApptDuplicateRecord returned error.");

	// Release orig record data
	if (columnValuesP)
		DbReleaseStorage(dbP, columnValuesP);

	if (err < errNone)
	{
		if (*newRowIDP != dbInvalidRowID)
		{
			DbDeleteRow(dbP, *newRowIDP);
			*newRowIDP = dbInvalidRowID;
		}

		//FrmUIAlert(DeviceFullAlert);
	}
	return err;
}
