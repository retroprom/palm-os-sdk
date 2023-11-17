/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateGlobals.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  Because of a bug in Metrowerks, we must compile the globals separately with
 *		PC-relative strings turned off. Otherwise, we get linker errors with
 *		pre-initialized structures.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <TimeMgr.h>
#include <DateTime.h>
#include <Table.h>

#include "DateGlobals.h"

/***********************************************************************
 *
 *	Shared globals
 *
 ***********************************************************************/

DmOpenRef				gApplicationDbP = NULL;							// Application DB handle
DatabaseID				gApplicationDbID = 0;							// Application DB ID
DmOpenRef				ApptDB = NULL;									// datebook database
DateType				Date;											// date currently displayed
TimeFormatType			TimeFormat;
DateFormatType			LongDateFormat;
DateFormatType			ShortDateFormat;
uint16_t				TopVisibleAppt;
privateRecordViewEnum	CurrentRecordVisualStatus;						// applies to current record
privateRecordViewEnum	PrivateRecordVisualStatus;						// applies to all other records

char 					gDeviceTimeZone[TZNAME_MAX];
char					gLocalizedTimeZomeName[TZNAME_MAX];	

Boolean					gSecurityPasswordOpened = false;				// Is the Security Pasword doalog is currently opened

// Dynamic Input Area handling
uint16_t				gCurrentFormID	 = frmInvalidObjectId;
FormPtr					gCurrentFormPtr = NULL;
WinHandle				gWinHdl = NULL; 								// view window handle, to check win handle upon winResizedEvent receipt.
RectangleType 			gCurrentWinBounds;								// current window bounds.


// Categories handling
uint32_t				DateBkCurrentCategoriesCount = 0;
CategoryID *			DateBkCurrentCategoriesP = NULL;				// currently displayed categories
char					DateBkCategoriesName[catCategoryNameLength];	// name of the current categories

// The following global variables are used to keep track of the edit
// state of the application.
uint32_t				DayEditPosition = 0;							// position of the insertion point in the desc field
uint32_t				DayEditSelectionLength = 0;						// length of the current selection.
uint32_t				gApptCursorID = dbInvalidCursorID;				// Cursor ID pointing on currently edited row
uint32_t				gMonthViewCursorID = dbInvalidCursorID;			// cursor ID to be used whithin Month View
int16_t					gSelectedRow = tblUnusableRow;					// Selected row - valid when gItemSelected is true
int16_t					gSelectedColumn = tblUnusableRow;				// Selected column - valid when gItemSelected is true
Boolean					gItemSelected = false;							// true if a day view item is selected
Boolean					gItemWasSelected = false;						// true if a day view item was selected for details


Boolean					RecordDirty = false;							// true if a record has been modified
	
// The following global variables are only valid while editing the repeat information of an appointment
void *					ApptDetailsP = NULL;

// The following global variable are saved to a state file.
uint16_t				DayStartHour = defaultDayStartHour;							// start of the day 8:00am
uint16_t				DayEndHour = defaultDayEndHour;								// End of the day 18:00am
uint16_t				StartDayOfWeek = sunday;
AlarmInfoType			AlarmPreset = { defaultAlarmPresetAdvance, defaultAlarmPresetUnit };
Boolean					SaveBackup = defaultSaveBackup;								// default setting "Backup to PC" checkbox
Boolean					ShowTimeBars = defaultShowTimeBars;							// show time bars in the day view
Boolean					CompressDayView = defaultCompressDayView;					// remove empty time slot to prevent scrolling
Boolean					ShowTimedAppts = defaultShowTimedAppts;						// show timed appointments in month view
Boolean					ShowUntimedAppts = defaultShowUntimedAppts;					// show untimed appointments in month view
Boolean					ShowDailyRepeatingAppts = defaultShowDailyRepeatingAppts;	// show daily repeating appointments in month view

// The following global variable is used to control the behavior Datebook
// Hard Button when pressed from the week or month views.  If no pen or key event 
// when occurred since enter the Week View then pressing the Datebook button
// will nagivate to the Month View, otherwise we go the the Day View of Today.
// Likewise, pressing the Datebook Hard Button will navigate from the Month View
// to either the Agenda View or the Day View, depending upon whether or not there
// were any user actions.
Boolean					EventInCurrentView;

// The following global variable is used to control the displaying of the
// current time in the title of a view.
Boolean					TimeDisplayed = false;							// True if time in been displayed
Boolean					TimeComputeWaitTime = false;					// True to take into account TimeDisplayTick
uint64_t				TimeDisplayTick;								// Tick count when we stop showing time

// The following globals are for the repeat rates of the alarms.
// number of times to repeat alarm sound 
uint16_t				AlarmSoundRepeatCount = defaultAlarmSoundRepeatCount;
																	
// interval between repeat sounds, in seconds
uint16_t				AlarmSoundRepeatInterval = defaultAlarmSoundRepeatInterval;

// Alarm sound MIDI file unique ID record identifier
uint32_t				AlarmSoundUniqueRecID = defaultAlarmSoundUniqueRecID;

FontID					ApptDescFont;									// font for drawing event description.
																			
uint16_t				AlarmSnooze = defaultAlarmSnooze;				// snooze delay, in seconds

// For attach sublaunch
Boolean					gAttachRequest = false;
char					gTransportPrefix[attachTransportPrefixMaxSize+1];

// The ToDo RefNum, used to access ToDo exported routines
uint32_t				ToDoRefNum = kRALInvalidRefNum;

RGBColorType 			colorLine = {0x00, 0x00, 0x00, 0x00};			// like 0x88 but draws as black in 1bpp mode

// Colors for shaded effects
// Note: index field have to be 0xff (-1)
RGBColorType			MediumGreyColor = { 0xFF, 140, 140, 140};
RGBColorType			LightGreyColor 	= { 0xFF, 222, 222, 222};
RGBColorType			DarkGreyColor 	= { 0xFF, 117, 117, 117};
RGBColorType			DarkRedColor 	= { 0xFF, 168, 0, 0 };
RGBColorType			WhiteColor 		= { 0xFF, 255, 255, 255 };
RGBColorType			BlackColor 		= { 0xFF, 0, 0, 0 };
