/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Datebook.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This is the Datebook application's main module.  This module
 *   starts the application, dispatches events, and stops
 *   the application.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#include <DebugAppReporting.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>

#include <AppMgr.h>
#include <DataMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <ExgMgr.h>
#include <ExgLocalLib.h>
#include <FeatureMgr.h>
#include <FormLayout.h>
#include <Loader.h>
#include <Menu.h>
#include <NotifyMgr.h>
#include <MemoryMgr.h>
#include <PenInputMgr.h>
#include <Preferences.h>
#include <PrivateRecords.h>
#include <SchemaDatabases.h>
#include <string.h>
#include <StringMgr.h>
#include <SystemMgr.h>
#include <SystemResources.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>

#include "SearchLib.h"

#include "DateGlobals.h"
#include "Datebook.h"
#include "DateAgenda.h"
#include "DateAlarm.h"
#include "DateDay.h"
#include "DateDayDetails.h"
#include "DateDisplay.h"
#include "DateLunar.h"
#include "DateMonth.h"
#include "DatePref.h"
#include "DateRepeat.h"
#include "DateTransfer.h"
#include "DateUtils.h"
#include "DateWeek.h"



#include "DatebookRsc.h"



/***********************************************************************
 *
 *	Local constants
 *
 ***********************************************************************/
#define kMaxSearchRepeatDescWidth	100
#define kMaxSearchDateWidth			42
#define kMaxSearchTimeWidth			39


/***********************************************************************
 *
 *	Local declarations
 *
 ***********************************************************************/
// exg socket structure for appLaunchCmdExgGetFullLaunch
typedef struct {
	ExgSocketType socket;
	char name[exgMaxTypeLength+1];
	char description[exgMaxTypeLength+1];
	char type[exgMaxTypeLength+1];
} ExgPassableSocketType;

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void PrvEventLoop (void);
static void PrvLoadCurrentCategories(uint32_t *numCategoriesP, CategoryID **categoriesPP);
static void PrvSaveCurrentCategories(uint32_t numCategories, CategoryID *categoriesP);


/***********************************************************************
 *
 * FUNCTION:		PrvRegisterData
 *
 * DESCRIPTION:	Register with the Exchange Manager to receive .vcs and
 *						text/x-vCalendar.
 *
 * PARAMETERS:		none
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			dje		7/28/00	Initial Revision
 *
 ***********************************************************************/
static void PrvRegisterData(void)
{
	MemHandle resH = DmGetResource (gApplicationDbP, strRsc, exgDescriptionStrID);
	char * descP = MemHandleLock(resH);

	ExgRegisterDatatype(sysFileCDatebook, exgRegExtensionID, dateExtension, descP, 0);
	ExgRegisterDatatype(sysFileCDatebook, exgRegTypeID, dateMIMEType, descP, 0);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);

	// Get application (icon name)
	resH = DmGetResource(gApplicationDbP, ainRsc, ainID );
	descP = MemHandleLock(resH);

	// Register for the attach feature
	ExgRegisterDatatype(sysFileCDatebook, exgRegSchemeID, exgGetScheme, descP, 0);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);
}

/***********************************************************************
 *
 * FUNCTION:	PrvRegisterNotifications
 *
 * DESCRIPTION:	Register notifications to be raised by the notification
 *				manager.
 *
 * PARAMETERS:		none
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			PLe			04/20/03	Initial Revision
 *
 ***********************************************************************/
static void PrvRegisterNotifications(void)
{
	status_t 			err;

	err = SysNotifyRegister(gApplicationDbID, sysNotifyTimeChangeEvent, NULL, sysNotifyNormalPriority, NULL, 0 /*datasize*/ );
	ErrNonFatalDisplayIf((err < errNone) && (err != sysNotifyErrDuplicateEntry),
			"Can't register for sysNotifyTimeChangeEvent");
}

/***********************************************************************
 *
 * FUNCTION:     PrvStartApplication
 *
 * DESCRIPTION:  This routine opens the application's database, loads the
 *               saved-state information and initializes global variables.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art			6/12/95		Initial Revision
 *			PLe			01/01/03	New OS6 revision
 *			PPL			07/15/03	cursors !
 *
 ***********************************************************************/
static status_t PrvStartApplication(void)
{
	status_t 					err;
	uint16_t 					mode;
	DateTimeType 				dateTime;
	DatebookPreferenceType 		prefs;
	int16_t 					prefsVersion;
	time_t 						rangeStartTime;
	time_t 						rangeEndTime;


	// Load the ToDo application as a shared lib, Doing this, the ToDo
	// globals will be kept all along the Datebook execution
	err = SysLoadModule(sysFileTApplication, sysFileCToDo, 0, 0, &ToDoRefNum);
	ErrNonFatalDisplayIf(err < errNone, "Unable to load the ToDo application as a shared library");

	// Determime if secret record should be shown.
	PrivateRecordVisualStatus = CurrentRecordVisualStatus =
		(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);

	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
			dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	// Get the time formats from the system preferences.
	TimeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	// Get the date formats from the system preferences.
	LongDateFormat = (DateFormatType)PrefGetPreference(prefLongDateFormat);
	ShortDateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);

	// Get the starting day of the week from the system preferences.
	StartDayOfWeek = (uint16_t) PrefGetPreference(prefWeekStartDay);

	// Get today's date.
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	Date.year = dateTime.year - firstYear;
	Date.month = dateTime.month;
	Date.day = dateTime.day;


	// Find the application's data file.  If it don't exist create it.
	err = DateDBOpenDatabase (&ApptDB, mode);
	if (err < errNone)
		return err;

#if defined(USE_WHERE_CLAUSE)
	// Create initial cursor based on current date and a 1-day range (day / agenda view)
	CalculateStartEndRangeTimes(&Date, 1, &rangeStartTime, &rangeEndTime, NULL);
	err =  ApptDBOpenOrRequeryWithNewRange(ApptDB, &gApptCursorID, rangeStartTime, rangeEndTime, true);
#else
	err =  ApptOpenCursor(ApptDB, DatebookGlobalRequest, 0 /*flags???*/, &gApptCursorID);
#endif
	if (err < errNone)
		return err;



	PIMAppProfilingBegin("PrvStartApplication, TimeZoneToAscii")

	// Get the devivce localized time zone name
	TimeZoneToAscii(gDeviceTimeZone, gLocalizedTimeZomeName);

	PIMAppProfilingEnd();


	// Read the preferences / saved-state information
	prefsVersion = DatebookLoadPrefs (&prefs);
	DayStartHour = prefs.dayStartHour;
	DayEndHour = prefs.dayEndHour;
	AlarmPreset = prefs.alarmPreset;
	SaveBackup = prefs.saveBackup;
	ShowTimeBars = prefs.showTimeBars;
	CompressDayView = prefs.compressDayView;
	ShowTimedAppts = prefs.showTimedAppts;
	ShowUntimedAppts = prefs.showUntimedAppts;
	ShowDailyRepeatingAppts = prefs.showDailyRepeatingAppts;
	AlarmSoundRepeatCount = prefs.alarmSoundRepeatCount;
	AlarmSoundRepeatInterval = prefs.alarmSoundRepeatInterval;
	AlarmSoundUniqueRecID = prefs.alarmSoundUniqueRecID;
	ApptDescFont = prefs.apptDescFont;
	AlarmSnooze = prefs.alarmSnooze;

	// Get the previous current category
	PrvLoadCurrentCategories(&DateBkCurrentCategoriesCount, &DateBkCurrentCategoriesP);

	// Reset selection
	TopVisibleAppt = 0;

	// Set initial active tab for the details dialog in day view
	DetailsSetDefaultEventDetailsTab(DetailsBookDefaultTabId);

	// LunarCalendar
	LunarCalendarInit(gApplicationDbP);

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    PrvStopApplication
 *
 * DESCRIPTION: This routine closes the application's database
 *              and saves the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art			6/12/95		Initial Revision
 *			PLe			01/01/03	New OS6 revision
  *			PPL			07/15/03	cursors !
 *
 ***********************************************************************/
static void PrvStopApplication (void)
{
	// Save the preferences
	DatebookSavePrefs();

	// Save current categories
	PrvSaveCurrentCategories(DateBkCurrentCategoriesCount, DateBkCurrentCategoriesP);

	// Send a frmSave event to all the open forms.
	FrmSaveAllForms ();

	// Close all the open forms.
	FrmCloseAllForms ();

	// Close the application's cursor
	ApptCloseCursor(&gApptCursorID);

	// Close the application's data file.
	DbCloseDatabase (ApptDB);
	ApptDB = NULL;

	// Unload the ToDo application loaded as a shared library
	if (ToDoRefNum != kRALInvalidRefNum)
		SysUnloadModule(ToDoRefNum);
}


/***********************************************************************
 *
 * FUNCTION:	PrvLoadCurrentCategories
 *
 * DESCRIPTION:	Loads the categories from preferences
 *
 * PARAMETERS:
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			TEs			11/13/02	Initial version
 *
 ***********************************************************************/
static void PrvLoadCurrentCategories(uint32_t *numCategoriesP, CategoryID **categoriesPP)
{
	uint32_t			size;
	int16_t			prefsVersion;

	// If no preferences saved. Set current category to catIDAll
	prefsVersion = PrefGetAppPreferences(sysFileCDatebook, datebookPrefID, NULL, &size, true);
	if (prefsVersion == noPreferenceFound)
		goto ExitNoPrefs;

	size = 0;
	prefsVersion = PrefGetAppPreferences(sysFileCDatebook, datebookCurCategoriesID, NULL, &size, true);
	if (prefsVersion == noPreferenceFound)
		goto ExitNoPrefs;

	*categoriesPP = MemPtrNew(size);
	PrefGetAppPreferences(sysFileCDatebook, datebookCurCategoriesID, *categoriesPP, &size, true);
	*numCategoriesP = size / sizeof(CategoryID);

	return;

ExitNoPrefs:
	*categoriesPP = MemPtrNew(sizeof(CategoryID));
	**categoriesPP = catIDAll;
	*numCategoriesP = 1;
}


/***********************************************************************
 *
 * FUNCTION:	PrvSaveCurrentCategories
 *
 * DESCRIPTION:	Saves the categories from preferences
 *
 * PARAMETERS:
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			TEs			11/13/02	Initial version
 *
 ***********************************************************************/
static void PrvSaveCurrentCategories(uint32_t numCategories, CategoryID *categoriesP)
{
	if (numCategories == 0)
	{
		// Delete the preference
		PrefSetAppPreferences (sysFileCDatebook, datebookCurCategoriesID, datebookPrefsVersionNum,
			0, 0, true);
	}
	else
	{
		// Save the categories
		PrefSetAppPreferences (sysFileCDatebook, datebookCurCategoriesID, datebookPrefsVersionNum,
			categoriesP, numCategories * sizeof(CategoryID), true);

		// Free allocated memory
		MemPtrFree(categoriesP);
	}
}

/***********************************************************************
 *
 * FUNCTION:		DatebookLoadPrefs
 *
 * DESCRIPTION:		Loads app's preferences.
 *
 * PARAMETERS:		prefsP	-- ptr to preferences structure to fill in.
 *
 * RETURNED:		the version of preferences from which values were read
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk			12/9/97		Initial version
 *			PLe			01/01/03	New OS6 revision
 *
 ***********************************************************************/
int16_t DatebookLoadPrefs (DatebookPreferenceType* prefsP)
{
	uint32_t				prefsSize;
	int16_t					prefsVersion = noPreferenceFound;
	uint32_t 				defaultFont = stdFont;

	DbgOnlyFatalErrorIf(!prefsP, "null prefP arg");

	// Read the preferences / saved-state information.
	prefsSize = (uint32_t)sizeof (DatebookPreferenceType);
	prefsVersion = PrefGetAppPreferences (sysFileCDatebook, datebookPrefID, prefsP, &prefsSize, true);

	// If the preferences version is from a future release (as can happen when going back
	// and syncing to an older version of the device), treat it the same as "not found" because
	// it could be significantly different
	if ( prefsVersion > datebookPrefsVersionNum )
		prefsVersion = noPreferenceFound;

	if ( prefsVersion == noPreferenceFound )
	{
		// Version 1 and 2 preferences
		prefsP->dayStartHour = defaultDayStartHour;
		prefsP->dayEndHour = defaultDayEndHour;
		prefsP->alarmPreset.advance = defaultAlarmPresetAdvance;
		prefsP->alarmPreset.advanceUnit = defaultAlarmPresetUnit;
		prefsP->saveBackup = defaultSaveBackup;
		prefsP->showTimeBars = defaultShowTimeBars;
		prefsP->compressDayView = defaultCompressDayView;
		prefsP->showTimedAppts = defaultShowTimedAppts;
		prefsP->showUntimedAppts = defaultShowUntimedAppts;
		prefsP->showDailyRepeatingAppts = defaultShowDailyRepeatingAppts;
		// Version 3 preferences
		prefsP->alarmSoundRepeatCount = defaultAlarmSoundRepeatCount;
		prefsP->alarmSoundRepeatInterval = defaultAlarmSoundRepeatInterval;
		prefsP->alarmSoundUniqueRecID = defaultAlarmSoundUniqueRecID;

		// We need to set up the note font with a default value for the system.
		prefsP->apptDescFont = (FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont) == 0
								? (FontID) defaultFont : stdFont);

		// Version 4 preferences
		prefsP->alarmSnooze = defaultAlarmSnooze;
	}

	return prefsVersion;
}


/***********************************************************************
 *
 * FUNCTION:    DatebookSavePrefs
 *
 * DESCRIPTION: Saves the current preferences of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * CALLED:	from DatePref.c and Datebook.c
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk			12/9/97		Initial version
 *			rbb			4/23/99		Added alarmSnooze
 *
 ***********************************************************************/
void DatebookSavePrefs (void)
{
	DatebookPreferenceType prefs;

	// Write the preferences / saved-state information.
	prefs.dayStartHour = DayStartHour;
	prefs.dayEndHour = DayEndHour;
	prefs.alarmPreset = AlarmPreset;
	prefs.saveBackup = SaveBackup;
	prefs.showTimeBars = ShowTimeBars;
	prefs.compressDayView = CompressDayView;
	prefs.showTimedAppts = ShowTimedAppts;
	prefs.showUntimedAppts = ShowUntimedAppts;
	prefs.showDailyRepeatingAppts = ShowDailyRepeatingAppts;
	prefs.alarmSoundRepeatCount = AlarmSoundRepeatCount;
	prefs.alarmSoundRepeatInterval = AlarmSoundRepeatInterval;
	prefs.alarmSoundUniqueRecID = AlarmSoundUniqueRecID;
	prefs.apptDescFont = ApptDescFont;
	prefs.alarmSnooze = AlarmSnooze;

	// Write the state information.
	PrefSetAppPreferences (sysFileCDatebook, datebookPrefID, datebookPrefsVersionNum,
		&prefs, sizeof (DatebookPreferenceType), true);
}


/***********************************************************************
 *
 * FUNCTION:    PrvInitDatabase
 *
 * DESCRIPTION: This routine initializes the datebook database.
	*
 * PARAMETERS:	 datebase
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		10/19/95	Initial Revision
 *			grant	6/23/99		Set the backup bit.
 *
 ***********************************************************************/
static void PrvInitDatabase (DmOpenRef dbP)
{
	// Set the backup bit.  This is to aid syncs with non-Palm software.
	DateDBSetDBBackupBit(dbP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvSyncNotification
 *
 * DESCRIPTION: This routine is called when the datebook database is
 *              synchronized.  This routine will sort the database
 *              and schedule the next alarm.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		6/6/95		Initial Revision
 *			ppl		07/10/03	cursor !
 *
 ***********************************************************************/
static void PrvSyncNotification (void)
{
	DmOpenRef 	dbP;

	// Find the application's data file.
	// RescheduleAlram needs dmModeShowSecret
	if (DateDBOpenDatabase(&dbP, dmModeReadWrite | dmModeShowSecret) < errNone)
		return;

	// Remove any alarms from the attention manager queue
	// that might be associated with items deleted during the hotsync.
	AttnIterate(gApplicationDbID, PostHotsyncVerification);

	// Reschedule the next alarm.
	RescheduleAlarms (dbP);

	// Close the appointment database.
	DbCloseDatabase (dbP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvSearchDraw
 *
 * DESCRIPTION: This routine draws the description, date, and time of
 *              an appointment found by the text PrvSearch routine.
 *
 * PARAMETERS:	 apptRecP - pointer to an appointment record.
 *              x        - draw position
 *              y        - draw position
 *              width    - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		6/6/95		Initial Revision
 *			PLe		01/01/03	New OS6 revision
 *
 ***********************************************************************/
static void PrvSearchDraw(ApptDBRecordType *apptRecP, int16_t x, int16_t y, int16_t width)
{
	uint16_t			i;
	char				timeStr [timeStringLength];
	char				dateStr [dateStringLength];
	uint16_t			len;
	uint16_t			maxDescWidth;
	Coord				drawX;
	char *				ptr;
	char *				descP;
	char *				rscP;
	MemHandle			rscH;
	DateFormatType		dateFormat;
	TimeFormatType		timeFormat;

	if (apptRecP->repeat.repeatType != repeatNone)
	{
		maxDescWidth = kMaxSearchRepeatDescWidth;
	}
	else
	{
		maxDescWidth = width - kMaxSearchDateWidth - kMaxSearchTimeWidth;
	}

	// Draw the appointment's desciption.
	if (apptRecP->description)
	{
		descP = apptRecP->description;
		ptr = StrChr(descP, linefeedChr);
		len = (ptr == NULL ? strlen(descP) : (uint16_t)(ptr - descP));
		WinDrawTruncChars (descP, len, x, y, maxDescWidth);
	}

	// If the event is repeating, draw the repeat type.
	if (apptRecP->repeat.repeatType != repeatNone)
	{
		// to have an array of string null terminated in a buffer with the new XML resource compiler
		rscH = DmGetResource (gApplicationDbP, appInfoStringsRsc, repeatTypesStrID);
		rscP = MemHandleLock (rscH);
		for (i = 0; i < apptRecP->repeat.repeatType; i++)
		{
			rscP += strlen(rscP) + 1;
		}
		x += (width - FntCharsWidth (rscP, strlen (rscP)));
		WinDrawChars (rscP, strlen (rscP), x, y);
		MemHandleUnlock (rscH);
		DmReleaseResource(rscH);
	}

	// Draw the appointment's date and time.
	else
	{
		// Get time and date formats from the system preferences.
		dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);
		timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

		if (! apptRecP->when.noTimeEvent)
		{
			TimeToAscii ((uint8_t)apptRecP->when.startTime.hours,
				(uint8_t)apptRecP->when.startTime.minutes, timeFormat, timeStr);

			len = strlen (timeStr);
			drawX = x + (width - FntCharsWidth (timeStr, len));
			WinDrawChars (timeStr, len, drawX, y);
		}

		DateToAscii((uint8_t) apptRecP->when.date.month,
					(uint8_t) apptRecP->when.date.day,
					(uint16_t)(apptRecP->when.date.year + firstYear),
					dateFormat, dateStr);

		len = strlen (dateStr);
		drawX = x + (width - FntCharsWidth (dateStr, len) - kMaxSearchTimeWidth);
		WinDrawChars (dateStr, len, drawX, y);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvSearch
 *
 * DESCRIPTION: This routine searchs the datebook database for records
 *              containing the string passed.
 *
 * PARAMETERS:	 findParams - text PrvSearch parameter block
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		6/6/95		Initial Revision
 *			PLe		01/01/03	New OS6 revision
 *
 ***********************************************************************/
static void PrvSearch (FindParamsPtr findParams)
{
	char *				header;
	Boolean 			done;
	Boolean 			match;
	MemHandle			headerStringH;
	DatabaseID			dbID;
	uint32_t 			recordNum;
	DmOpenRef 			dbP = NULL;
	RectangleType	 	r;
	SearchLibType		search;
	ApptDBRecordType	apptRec;
	CategoryID			cat = catIDAll;
	uint16_t			attr;
	uint32_t			searchableColIDs[] = {	EVENT_DESCRIPTION_ID,
												EVENT_LOCATION_ID,
												EVENT_NOTE_ID };

	PIMAppProfilingBegin("Find");

	findParams->more = false;
 	memset(&search, 0x00, sizeof(SearchLibType));

	if (ApptDB)
	{
		search.searchDB = ApptDB;
	}
	else
	{
		// Open it
		DateDBOpenDatabase(&dbP, (uint16_t)findParams->dbAccesMode);
		search.searchDB	= dbP;

		// Set gAddrDB global used in ToolsDrawRecordNameAndPhoneNumber
		ApptDB = dbP;
		PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
	}

	// No DB
	if (!search.searchDB)
		PIMAppProfilingReturnVoid();

	// Get the DatabaseID
	DmGetOpenInfo(search.searchDB, &dbID, NULL, NULL, NULL);
	search.cursorID			= dbInvalidCursorID; // SearchLib to create the cursor
	search.schemaName		= DatebookApptTable;
#if defined(USE_WHERE_CLAUSE)
	search.orderBy			= DatebookOrderByStartDate;
#else
	search.orderBy			= DatebookOrderByRepeatThenStartDate;
#endif
	search.cursorFlag		= dbCursorEnableCaching;
	search.startRowIndex	= findParams->recordNum + 1;
	search.startColumnIndex	= 0;
	search.numCols			= sizeof(searchableColIDs) / sizeof(searchableColIDs[0]);
	search.colIDsP			= searchableColIDs;
	search.numCategories	= 1;
	search.catIDsP			= &cat;
	search.matchMode		= DbMatchAny;
	search.textToSearch		= findParams->strToFind;
	search.origTextToSearch	= findParams->strAsTyped;
	search.recordDirection	= dmSeekForward;
	search.columnDirection	= dmSeekForward;
	search.interruptible	= true;
	search.interruptCheck	= 500;	// Every 500 ms, check if interrupted

	if (SearchLibInit(&search) < errNone)
		goto Exit;

	// Display the heading line.
	headerStringH = DmGetResource (gApplicationDbP, strRsc, findDatebookHeaderStrID);
	header = MemHandleLock(headerStringH);
	done = FindDrawHeader(findParams, header);
   	MemHandleUnlock(headerStringH);
   	DmReleaseResource(headerStringH);
  	if (done)
   		goto Exit;

	// search the description and note fields for the "find" string.
	recordNum = findParams->recordNum;
	while (true)
	{
		match = SearchLibSearch(&search);

		if (!match)
		{
			findParams->more = search.interrupted;
			break;
		}

		// Verify the secret attribute.
		// This could happen when the application is already launched. That cost less than creating another cursor.
		if ((DbGetRowAttr(search.searchDB, search.foundRowID, &attr) >= errNone) && (attr & dbRecAttrSecret) && (PrivateRecordVisualStatus != showPrivateRecords))
		{
			search.resumePolicy = kSearchResumeChangeRecord;
			continue;
		}

		ApptGetRecord(ApptDB, search.cursorID, &apptRec,
			DBK_SELECT_TIME | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS | DBK_SELECT_DESCRIPTION);

		// If a match occurred in a repeating event,  make sure there is
		// a displayable occurrence of the event.
		if (apptRec.repeat.repeatType != repeatNone)
		{
			DateType	date;

			date = apptRec.when.date;
			match = ApptNextRepeat(&apptRec, &date, true);
		}

		// Add the match to the find paramter block,  if there is no room to
		// display the match the following function will return true.

		done = FindSaveMatch(findParams, search.foundRowIndex, search.foundRowID, search.matchPos, search.matchLength, search.foundColID, 0, dbID);
		if (done)
		{
			findParams->more = true;
			ApptFreeRecordMemory(&apptRec);
			break;
		}

		// Get the bounds of the region where we will draw the results.
		FindGetLineBounds (findParams, &r);

		// Display the appointment info.
		PrvSearchDraw(&apptRec, (int16_t)(r.topLeft.x + 1), r.topLeft.y, (int16_t)(r.extent.x - 2));

		ApptFreeRecordMemory(&apptRec);

		findParams->lineNumber++;
		search.resumePolicy = kSearchResumeChangeRecord;
	}

Exit:
	SearchLibRelease(&search);

	if (dbP)
	{
		DbCloseDatabase(dbP);
		// Reset gAddrDB if we have opened the database
		ApptDB = NULL;
	}

	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:    PrvGoToItem
 *
 * DESCRIPTION: This routine is called as the result of hitting the
 *              "Go to" button in the text PrvSearch dialog.
 *
 * PARAMETERS:	 goToParams   - where to go to.
 *              launchingApp - true if the application is being launched
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		6/6/95		Initial Revision
 *			PLe		01/01/03	New OS6 revision
 *			PPL		07/16/03	cursors !
 *
 ***********************************************************************/
static void PrvGoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	EventType 	event;
	CategoryID 	dmSelectAllCategoriesID[] = {catIDAll};
	uint32_t 	rowID;
	status_t	err;
	Boolean		recordIsInCategory = false;


	rowID = goToParams->recordID;

	err = DbIsRowInCategory(ApptDB, rowID, 1, dmSelectAllCategoriesID, DbMatchAny, &recordIsInCategory);

	if (err < errNone || ! recordIsInCategory)
	{
		FrmUIAlert(secGotoInvalidRecordAlert);
		FrmGotoForm(gApplicationDbP, DayView);
		return;
	}

	// If the application is already running, destroy the UI, this may
	// change the index of record we're going to.
	if ( !launchingApp  && !gSecurityPasswordOpened)
	{
		FrmCloseAllForms ();
		ClearEditState ();
	}

	// Go to day view
	memset (&event, 0x00, sizeof(EventType));

	// Send an event to load the form we want to goto.
	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = DayView;
	EvtAddEventToQueue (&event);

	// Send an event to goto a form and select the matching text.
	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = DayView;
	event.data.frmGoto.recordID = rowID;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchLen;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	EvtAddEventToQueue (&event);
}


/***********************************************************************
 *
 * FUNCTION:    PrvGoToAlarmItem
 *
 * DESCRIPTION: This routine is called as the result of hitting the
 *              "Go to" button in the attention manager dialog.
 *
 * PARAMETERS:	 UniqueID   - uniqueID of the item to go to.
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap		6/6/95		Initial Revision
 *			PLe		01/01/03	New OS6 revision
 *			PPL		07/16/03	cursors !
 *
 ***********************************************************************/
static void PrvGoToAlarmItem (uint32_t uniqueID)
{
	uint16_t	attributes;
	EventType	event;
	Boolean		verifyPW;
	status_t	err;
	CategoryID 	dmSelectAllCategoriesID[] = {catIDAll};
	Boolean		recordIsInCategory = false;

	// remove the item from the attention manager queue
	AttnForgetIt(gApplicationDbID, uniqueID);

	// verify that the specified uniqueID is still in the Date Book's database
	// one reason this needs to be done is to prevent crashing should the alarm
	// have been on an untitled event with no note attached.  In the process of
	// executing the goto, the original record may have been deleted as all
	// untitled events w/o a note are as soon as the item is deselected.
	err = DbIsRowInCategory(ApptDB, uniqueID, 1, dmSelectAllCategoriesID, DbMatchAny, &recordIsInCategory);
	if (err < errNone || ! recordIsInCategory)
	{
		FrmGotoForm(gApplicationDbP, DayView);
		return;
	}

	// Get the attribute information for the record
	DbGetRowAttr (ApptDB, uniqueID, &attributes);

	// if the user does a "go to" from an attention manager dialog while the
	// private items are hidden and the selected item happens to have the
	// private bit set, the user must submit his password so that the
	// security level can be changed from hide to show before the goto to the
	// specified record occurs
	if ( (attributes & dmRecAttrSecret) && (PrivateRecordVisualStatus == hidePrivateRecords) )
	{


		verifyPW = DateUtilsSecVerifyPW(showPrivateRecords);

		if (verifyPW)
		{
			PrivateRecordVisualStatus = CurrentRecordVisualStatus = showPrivateRecords;
		}
		else
		{
			FrmGotoForm(gApplicationDbP, DayView);
			return;
		}
	}

	memset (&event, 0x00, sizeof(EventType));

 	// Send an event to load the form we want to go to.
 	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = DayView;
	EvtAddEventToQueue (&event);


	// Send an event to goto a form and select the matching text.
	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = DayView;
	event.data.frmGoto.recordID = uniqueID;
	event.data.frmGoto.matchPos = mostRightFieldPosition;
	event.data.frmGoto.matchLen = 0;
	event.data.frmGoto.matchFieldNum = EVENT_DESCRIPTION_ID;
	EvtAddEventToQueue (&event);
}


/***********************************************************************
 *
 * FUNCTION:    	DoSecurity
 *
 * DESCRIPTION: 	Bring up security dialog and then reopen database if
 *					necessary.
 *
 * PARAMETERS:  	Nothing.
 *
 * RETURNED:    	Nothing.
 *
 * JAQ	6/12/99		Initial Revision.
 *	PPL	07/16/03	Cursors !
 *					Add closing and reopening cursors.
 *
 ***********************************************************************/
void DoSecurity (void)
{
	DateUtilsSecSelectViewStatus();

	// Send the event to reload from DB and refresh display
	TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : DoSecurity..."));
	DateBkEnqueueEvent(datebookRefreshDisplay);
}


/***********************************************************************
 *
 * FUNCTION:    ClearEditState
 *
 * DESCRIPTION: This routine take the application out of edit mode.
 *              The edit state of the current record is remember until
 *              this routine is called.
 *
 *              If the current record is empty its deleted by this
 *              routine.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true is current record is deleted by this routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		7/28/95		Initial Revision
 *
 ***********************************************************************/
Boolean ClearEditState (void)
{
	Boolean result;

	if ( ! gItemSelected)
	{
		return false;
	}

	result = ApptDeleteRecordIfEmpty(ApptDB, gApptCursorID);

	// Clear the global variables that keep track of the edit state of the
	// current record.

	gItemSelected = false;
	DayEditPosition = 0;
	DayEditSelectionLength = 0;

	return result;
}


/***********************************************************************
 *
 * FUNCTION:    PrvCheckForAttachURL
 *
 * DESCRIPTION: Extracts from the socket URL the attach transport scheme
 *				if the datebook was sublaunched by the exchange manager
 *
 *
 * PARAMETERS:  socketP - exchange socket from exgGet launch code
 *				attachTransportScheme - transport to use for sending data back
 *				targetID - target application to use when transport is _local.
 *
 * RETURNED:    true if we got a scheme
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			PLe		08/13/02	Initial Revision
 *
 ***********************************************************************/
static Boolean PrvCheckForAttachURL(ExgSocketType* socketP, char* attachTransportScheme, uint32_t *targetID)
{
	char *endP;

	if (!socketP || !attachTransportScheme) return false;

	*attachTransportScheme = 0;
	if (socketP->name)
	{
		endP = StrChr(socketP->name, chrColon);
		if (endP)
		{
			// Copy including colon
			StrNCopy(attachTransportScheme,socketP->name, max((endP-socketP->name)+1, attachTransportPrefixMaxSize));
		}
	}

	if (*attachTransportScheme ==0)
		StrCopy(attachTransportScheme,exgLocalPrefix); // assume local by default

	if (targetID)
		*targetID = socketP->goToCreator;

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppSwitchByCreator
 *
 * DESCRIPTION: Switches to an application given a creatorID
 *
 * PARAMETERS:  creator - creatorID of app to switch to
 *				cmd     - launch code
 *				cmdPBP  - command parameters
 *				cmdPBPSize  - size of cmdPBP block
 *
 * RETURNED:    err - zero if none.
 *
 ***********************************************************************/
status_t AppSwitchByCreator(uint32_t creator, uint16_t cmd, MemPtr cmdPBP, uint32_t cmdPBSize)
{
	status_t err;
	DatabaseID appDBID;

	// Find the application
	appDBID = DmFindDatabaseByTypeCreator(sysFileTApplication, creator, dmFindExtendedDB, NULL);
	if (appDBID == NULL)
	{
		err = DmGetLastErr();
		return err;
	}

	err = SysUIAppSwitch(appDBID, cmd, cmdPBP, cmdPBSize );

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    PrvExgMakePassableSocket
 *
 * DESCRIPTION: This creates a duplicate copy of an exchange socket
 *              The new socket memory is self contained, all the associated memory
 *               pointers are in the same allocated memory block
 *				Owner is set to system for launch calls
 *
 * PARAMETERS:  socketP - exchange socket to  clone
 *
 * RETURNED:    new passable exchange socket pointer or NULL.
 *
 ***********************************************************************/
static ExgPassableSocketType * PrvExgMakePassableSocket(ExgSocketType * socketP)
{
	ExgPassableSocketType *newSockP;
	// allocate space for new socket and the three associated strings
	newSockP = (ExgPassableSocketType *)MemPtrNew(sizeof(ExgPassableSocketType));
	if (!newSockP) return NULL;

	memset(newSockP, 0x00, sizeof(ExgPassableSocketType)); // fill  null
	// copy the socket data
	MemMove(&newSockP->socket,socketP,sizeof(ExgSocketType));
	newSockP->socket.socketRef = 0; // can't duplicate socketRef...
	newSockP->socket.componentIndex  = 0; // or this
	// now copy the string data from the source to the buffer after the new socket
	// and make the new socket point to those strings
	if (socketP->name)
		StrNCopy(&newSockP->name[0], socketP->name, exgMaxTypeLength);
	newSockP->socket.name = &newSockP->name[0];
	if (socketP->description)
		StrNCopy(&newSockP->description[0],socketP->description, exgMaxTypeLength);
	newSockP->socket.description = &newSockP->description[0];
	if (socketP->type)
		StrNCopy(&newSockP->type[0],socketP->type, exgMaxTypeLength);
	newSockP->socket.type = &newSockP->type[0];
	MemPtrSetOwner(newSockP,0); // assign to system memory so that we can
								// pass this via an appswitch
	return (newSockP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvExgFixupPassableSocket
 *
 * DESCRIPTION: This repairs pointer references in a passable socket passed through the app manager
 *
 * PARAMETERS:  socketP - passable exchange socket to  clone
 *
 * RETURNED:    repaired exchange socket pointer.
 *
 ***********************************************************************/
static ExgSocketType *PrvExgFixupPassableSocket(ExgPassableSocketType *socketP)
{
    // reconstruct the socket passed by clone - SysUIAppSwitch now makes a copy and munches pointer references.
	socketP->socket.name = socketP->name;
	socketP->socket.description = socketP->description;
	socketP->socket.type = socketP->type;
	return &(socketP->socket);
}

/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the Datebook
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		6/12/95		Initial Revision
 *			PLe		01/01/03	New OS6 revision
 *			PPL		07/15/03	cursors !
 *
 ***********************************************************************/
uint32_t PilotMain (uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	ExgPassableSocketType* 	passableSocketP;
	ExgSocketType* 			socketP;
	DmOpenRef 				dbP;
 	uint32_t				cursorID = dbInvalidCursorID;
	uint32_t 				value;
	uint32_t				defaultForm;
	uint16_t				mode;
	Boolean 				launched;
	status_t 				error = errNone; // the error returned by PilotMain

	#ifdef DEBUGAPPLIB_ENABLED
	App_DebugAppLib_Init( "Datebook", cmd );
	#endif

#ifndef PIM_APPS_PROFILING
	TraceOutput(TL(appErrorClass, "====> Datebook entering PilotMain - cmd=%d",cmd));
#endif

	switch (cmd)
	{
		// Launch code sent by the launcher or the datebook button.
		case sysAppLaunchCmdNormalLaunch:
			error = PrvStartApplication ();
			if (error < errNone)
				goto ExitError;

			// If the user previously left the Datebook while viewing the agenda,
			// return there. Otherwise, go to the day view
			error = FtrGet (sysFileCDatebook, recentFormFeature, &value);
			if (error)
				defaultForm = defaultRecentForm;
			else
				defaultForm = value;

			FrmGotoForm(gApplicationDbP, (uint16_t) defaultForm);

			PrvEventLoop ();
			PrvStopApplication ();
			break;

		case sysAppLaunchCmdExgGetData:
			// Handling get request - since we want to be fully launched, clone the socket and
			// AppSwitch to ourselves, otherwise we could respond here
			passableSocketP = PrvExgMakePassableSocket((ExgSocketPtr) cmdPBP);

			if (passableSocketP)
			{
				// we really only need the name and goToCreatorID fields, but we'll copy the whole struture here.
				error = AppSwitchByCreator(sysFileCDatebook, appLaunchCmdExgGetFullLaunch, (MemPtr) passableSocketP , sizeof(ExgPassableSocketType));
				if (error < errNone)
				{
					error = exgErrUserCancel; // return cancel error to let caller skip reading
					break;
				}
				else
					MemPtrFree(passableSocketP);
			}
			else
				error = exgErrAppError;
			break;

		case appLaunchCmdExgGetFullLaunch:
			// This is our custom full launch code
			// Repair socket from transit
			socketP = PrvExgFixupPassableSocket((ExgPassableSocketType *)cmdPBP);

			// Test if the PIM is called for attach
			gAttachRequest = PrvCheckForAttachURL(socketP, gTransportPrefix, NULL);

			error = PrvStartApplication ();
			if (error < errNone)
				break;

			// Switch to day view (not saved view because the attach can only be performed from this one)
			FrmGotoForm(gApplicationDbP,  (uint16_t) DayView);

			PrvEventLoop ();
			PrvStopApplication ();

	     	ExgNotifyGoto(socketP, 0);
			break;

		case  appLaunchCmdAlarmEventGoto:
			// This action code is a DateBook specific custom launch code.
			// It will always require that the app launches as it is a result
			// of a SysUIAppSwitch call.
			error = PrvStartApplication ();
			if (error < errNone)
				break;

			PrvGoToAlarmItem (*((uint32_t*)cmdPBP));

			PrvEventLoop ();
			PrvStopApplication ();
			break;

		case  sysAppLaunchCmdGoTo:
			// This action code might be sent to the app when it's already running
			// if the use hits the "Go To" button in the Find Results dialog box.
			launched = launchFlags & sysAppLaunchFlagNewGlobals;
			if (launched)
			{
				// New start
				error = PrvStartApplication ();
				if (error < errNone)
					break;

				PrvGoToItem ((GoToParamsPtr) cmdPBP, launched);

				PrvEventLoop ();
				PrvStopApplication ();
			}
			else
			{
				// application was already started
				PrvGoToItem ((GoToParamsPtr) cmdPBP, launched);
			}
			break;

		case sysAppLaunchCmdFind:
			// Launch code sent by text PrvSearch.
			PrvSearch ( (FindParamsPtr) cmdPBP);
			break;

		case sysAppLaunchCmdSyncNotify:
			// Launch code sent by sync application to notify the datebook
			// application that its database was been synced.
			PrvSyncNotification ();
			break;


		case sysAppLaunchCmdNotify:
			if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyTimeChangeEvent)
			{
				// Launch code sent when the system time is changed.
				// reset the trigger for the next alarm to fire
				AlarmReset (false);

				// Remove any "future" alarms from the attention manager queue
				// (ie alarms that will trigger after the new time setting)
				AttnIterate(gApplicationDbID, DeviceTimeChanged);
			}
			break;

		case  sysAppLaunchCmdSystemReset:
			// This action code is sent after the system is reset.
			if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->hardReset)
				TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : sysAppLaunchCmdSystemReset: HARD RESET"));
			else
				TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : sysAppLaunchCmdSystemReset: SOFT RESET"));

			// Register to receive vcs files
			PrvRegisterData();

			// Register to get notification manager events.
			PrvRegisterNotifications();

			// On a soft reset, reschedule the alarms
			if (! ((SysAppLaunchCmdSystemResetType*)cmdPBP)->hardReset)
				AlarmReset (false);		// Find and install next upcoming alarm

			// Create our default database.
			if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
			{
				TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : sysAppLaunchCmdSystemReset: CREATE DEFAULT DB"));

				// Create the default DB from image
				error = DateDBCreateDefaultDatabase();
			}
			break;

		case sysAppLaunchCmdExgAskUser:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
				error = DateDBOpenDatabase(&dbP, dmModeReadWrite);
			else
				dbP = ApptDB;

			CustomAcceptBeamDialog(dbP, (ExgAskParamPtr) cmdPBP);

			if (!(launchFlags & sysAppLaunchFlagSubCall) && dbP)
				DbCloseDatabase(dbP);

			break;

		case sysAppLaunchCmdExgReceiveData:
			// Receive the record.  The app will parse the data and add it to the database.
			// If our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				error = DateDBOpenDatabase(&dbP, dmModeReadWrite);
				if (error < errNone)
					break;
			}

			else
				dbP = ApptDB;

			if (dbP != NULL)
			{
				error = DateReceiveData(dbP, (ExgSocketPtr) cmdPBP);

				// The received event may have an alarm, reschedule the next alarm.
				RescheduleAlarmsAllRows();

				// Refresh display if we were already launched
				if (launchFlags & sysAppLaunchFlagSubCall)
					DateBkEnqueueEvent(datebookRefreshDisplay);
				else
					DbCloseDatabase(dbP);
			}
			else
				error = exgErrAppError;

			// If we can't open our database, return the error since it wasn't passed to ExgDisconnect
			break;

		case sysAppLaunchCmdExgPreview:
			TraceOutput(TL(appErrorClass, ">>>> DATEBOOK : sysAppLaunchCmdExgPreview"));
			DateTransferPreview((ExgPreviewInfoType *)cmdPBP);
			break;

		case sysAppLaunchCmdInitDatabase:
			// This action code is sent by the DesktopLink server when it creates
			// a new database.  We will initialize the new database.
			PrvInitDatabase (((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
			break;


		case sysAppLaunchCmdAlarmTriggered:
			// Launch code sent by Alarm Manager to notify the datebook
			// application that an alarm has triggered.
			AlarmTriggered ((SysAlarmTriggeredParamType *) cmdPBP);
			break;


		case sysAppLaunchCmdAttention:
			// Launch Code sent by Attention Manager tio let Datebook draws
			// alarmed events.
			AttentionBottleNeckProc((AttnLaunchCodeArgsType*)cmdPBP);
			break;


		case sysAppLaunchCmdExportRecordGetCount:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				// Determime if secret record should be shown.
				PrivateRecordVisualStatus = CurrentRecordVisualStatus =
					(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);

				mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	      		error = DateDBOpenDatabase(&dbP, mode);
	      		if (error < errNone)
					break;

				error = ApptOpenCursor(dbP , DatebookImportExportRequest, 0, &cursorID);
	      	}
			else
			{
				dbP = ApptDB;
				cursorID = gApptCursorID;
			}

			if (dbP != NULL)
			{
				// Assign the number of records within the openened cursor
				*((uint32_t*)cmdPBP) = DbCursorGetRowCount(cursorID);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
				{
					ApptCloseCursor(&cursorID);
					DbCloseDatabase(dbP);
				}
			}
			break;

		case sysAppLaunchCmdExportRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				// Determime if secret record should be shown.
				PrivateRecordVisualStatus = CurrentRecordVisualStatus =
					(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);

				mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	      		error = DateDBOpenDatabase(&dbP, mode);
	      		if (error < errNone)
					break;
			}
			else
				dbP = ApptDB;

			// the cursor will include secret records only if the database
			// is open with mode = dmModeReadWrite

			if (dbP != NULL)
			{
				error = ApptOpenCursor(dbP , DatebookImportExportRequest, 0, &cursorID);
	      		if (error < errNone)
					break;

				error = DateExportData(dbP, cursorID, (ImportExportRecordParamsPtr)cmdPBP);

				ApptCloseCursor(&cursorID);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
					DbCloseDatabase(dbP);
			}
			break;

		case sysAppLaunchCmdImportRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				// Determime if secret record should be shown.
				PrivateRecordVisualStatus = CurrentRecordVisualStatus =
					(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);

				mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	      		error = DateDBOpenDatabase(&dbP,  mode);
	      		if (error < errNone)
					break;
	      	}
			else
				dbP = ApptDB;

			if (dbP != NULL)
			{
				error = ApptOpenCursor(dbP , DatebookImportExportRequest, 0, &cursorID);
	      		if (error < errNone)
					break;

				error = DateImportData(dbP, cursorID, (ImportExportRecordParamsPtr)cmdPBP);

				ApptCloseCursor(&cursorID);

				// The received event may have an alarm, reschedule the next alarm.
				RescheduleAlarmsAllRows();

				// Refresh display if we were already launched
				if (launchFlags & sysAppLaunchFlagSubCall)
					DateBkEnqueueEvent(datebookRefreshDisplay);
				else
					DbCloseDatabase(dbP);
			}
			break;

		case sysAppLaunchCmdDeleteRecord:
			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				// Determime if secret record should be shown.
				PrivateRecordVisualStatus = CurrentRecordVisualStatus =
					(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);

				mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	      		error = DateDBOpenDatabase(&dbP, mode);
	      			if (error < errNone)
					break;

				error = ApptOpenCursor(dbP , DatebookImportExportRequest, 0, &cursorID);
			}
			else
				dbP = ApptDB;

			if (dbP != NULL)
			{
				error = ApptOpenCursor(dbP , DatebookImportExportRequest, 0, &cursorID);
	      		if (error < errNone)
					break;

				error = DateDeleteRecord(dbP, cursorID, (ImportExportRecordParamsPtr)cmdPBP);

				ApptCloseCursor(&cursorID);

				if (!(launchFlags & sysAppLaunchFlagSubCall))
					DbCloseDatabase(dbP);
			}
			break;

		case sysAppLaunchCmdMoveRecord:
			((ImportExportRecordParamsPtr)cmdPBP)->index 	= ImpExpInvalidRecIndex;
			((ImportExportRecordParamsPtr)cmdPBP)->destIndex	= ImpExpInvalidRecIndex;
			((ImportExportRecordParamsPtr)cmdPBP)->uniqueID 	= ImpExpInvalidUniqueID;
			error = dmErrInvalidParam;
			break;

		case sysLaunchCmdInitialize:
			// when module loads
			// Get application dbP
			error = SysGetModuleDatabase(SysGetRefNum(), &gApplicationDbID, &gApplicationDbP);

			// Assign the local device TZ
			gettimezone(gDeviceTimeZone, TZNAME_MAX);

			// Check that the UTC timezone exists in DB, for time computation purpose
			DbgOnlyFatalErrorIf(!TimezoneExistsInDatabase(UTCTimezone), "Error: UTC Timezone not in DB.");
			break;

		case sysLaunchCmdFinalize:
			// before  module is unloaded
			LunarCalendarClose(gApplicationDbP);
			break;

		default:
			break;
	}

ExitError:

#ifndef PIM_APPS_PROFILING
	TraceOutput(TL(appErrorClass, "<==== Datebook exiting PilotMain cmd=%d errCode=%d",cmd,error));
#endif

	#ifdef DEBUGAPPLIB_ENABLED
 	App_DebugAppLib_Exit();
 	#endif

	return error;
}



/***********************************************************************
 *
 * FUNCTION:    TimeToWait
 *
 * DESCRIPTION:	This routine calculates the number of ticks until the
 *						time display should be hidden.  If the time is not being
 *						displayed, we return evtWaitForever - a good default for
 *						EvtGetEvent.
 *
 *						If the agenda view is on display, return a delay of one
 *						second, allowing the title bar clock to update.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    number of ticks to wait
 *						== evtWaitForever if we are not showing the time
 *						== 0 if time is up
 *
 *	NOTES:		Uses the global variables TimeDisplayed and TimeDisplayTick.
 *					TimeDisplayed is true when some temporary information is on
 *					display.  In that case, TimeDisplayTick is the time when the
 *					temporary display should go away.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	2/2/99		Initial Revision
 *			rbb		11/15/99	Added timeout for the agenda view
 *
 ***********************************************************************/
int32_t TimeToWait(void)
{
	int64_t timeRemaining;

	if (gCurrentFormID == AgendaView)
	{
		return SysTicksPerSecond();
	}

	if (TimeComputeWaitTime)
	{
		if ((timeRemaining = TimeDisplayTick - TimGetTicks()) < 0)
			timeRemaining = 0;

		return (int32_t) timeRemaining;
	}

	return evtWaitForever;
}

/***********************************************************************
 *
 * FUNCTION:    PrvApplicationHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		9/11/95		Initial Revision
 *			PLe		01/01/03	New OS6 revision
 *
 ***********************************************************************/
static Boolean PrvApplicationHandleEvent (EventType* event)
{
	uint16_t	formID;
	FormPtr 	frm;

	if (event->eType == frmLoadEvent)
	{
		// Load the form resource.
		formID = event->data.frmLoad.formID;
		frm = FrmInitForm(gApplicationDbP, formID);
		FrmSetActiveForm (frm);

		// Set the constraints only for forms, not for dialogs
		switch (formID)
		{
			case DayView:
			case WeekView:
			case MonthView:
			case AgendaView:
				// Get the handle on the window, will be used to identifiy winResizedEvent events
				gWinHdl = FrmGetWindowHandle(frm);

				// keep the window bounding rect.
				FrmGetFormInitialBounds(frm, &gCurrentWinBounds);

				gCurrentFormID	 = event->data.frmLoad.formID;
				gCurrentFormPtr = frm;
				break;

			default:
				gCurrentFormID		= frmInvalidObjectId;
				gCurrentFormPtr 	= NULL;
				break;

		}

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmDispatchEvent each time is receives an
		// event.
		switch (formID)
		{
			case  DayView:
				FrmSetEventHandler (frm, DayViewHandleEvent);
				break;

			case WeekView:
				FrmSetEventHandler (frm, WeekViewHandleEvent);
				break;

			case MonthView:
				FrmSetEventHandler (frm, MonthViewHandleEvent);
				break;

			case AgendaView:
				FrmSetEventHandler (frm, AgendaViewHandleEvent);
				break;

			case DetailsDialog:
				FrmSetEventHandler (frm, DetailsHandleEvent);
				break;

			case RepeatDialog:
				FrmSetEventHandler (frm, RepeatHandleEvent);
				break;

			case PreferencesDialog:
				FrmSetEventHandler (frm, PreferencesHandleEvent);
				break;

			case DisplayDialog:
				FrmSetEventHandler (frm, DisplayOptionsHandleEvent);
				break;

		}

		FtrSet (sysFileCDatebook, recentFormFeature,
					formID == AgendaView ? AgendaView : DayView);

		return (true);
	}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    PrvPreprocessEvent
 *
 * DESCRIPTION: This routine handles special event processing that
 *              needs to occur before the System Event Handler get
 *              and event.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		1/10/97		Initial Revision
 *			CSS		06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *			PPL		10/31/03	comment out the function as it is not called.
 *
 ***********************************************************************/
/*
static Boolean PrvPreprocessEvent (EventType* event)
{
	Boolean handled = false;

	// We want to allow the datebook event handled see the command keys
	// before the any other event handler get them.  Some of the databook
	// views display UI that dispappears automaticial (the time in the
	// title, or an event's descripition in the week view).

	// Initially This UI
	// needs to be dismissed before the System Event Handler or the
	// Menu Event Handler displays any UI objects.
	// But with FEPs (japanese and so on input mode)
	// some events have to be handle FIRST by System Event Handler

	if (event->eType == keyDownEvent)
	{
		if	(	(!TxtCharIsHardKey(	event->data.keyDown.modifiers,
											event->data.keyDown.chr))
			&&	(EvtKeydownIsVirtual(event))
			&&	(event->data.keyDown.chr != vchrPageUp)
			&&	(event->data.keyDown.chr != vchrPageDown)
			&&	(event->data.keyDown.chr != vchrSendData))
		{
			handled = FrmDispatchEvent (event);
		}
	}
	return handled;
}
*/

/***********************************************************************
 *
 * FUNCTION:    PrvEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the Datebook
 *              aplication.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		6/12/95		Initial Revision
 *			grant 	2/2/99		Use TimeToWait() for time to wait for next event
 *
 ***********************************************************************/
static void PrvEventLoop (void)
{
	status_t error;
	EventType event;

	do
	{
		EvtGetEvent (&event, TimeToWait());

		if (! SysHandleEvent (&event))
		{
			//if (!PrvPreprocessEvent (&event))
			{
				if (! MenuHandleEvent (NULL, &event, &error))
				{
					if (! PrvApplicationHandleEvent (&event))
					{
						FrmDispatchEvent (&event);
					}
				}
			}
		}
	}
	while (event.eType != appStopEvent);
}
