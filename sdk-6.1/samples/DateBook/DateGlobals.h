/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateGlobals.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             	This file holds global variables definition.
 *
 *****************************************************************************/

#ifndef _DATEGLOBALS_H_
#define	_DATEGLOBALS_H_


#include <PalmTypes.h>

#include <Bitmap.h>
#include <CatMgr.h>
#include <DataMgr.h>
#include <DateTime.h>
#include <PrivateRecords.h>
#include <Rect.h>
#include <TimeMgr.h>
#include <Window.h>

#include "DateDB.h"


#ifdef PIM_APPS_PROFILING
#include "PIMAppsProfiling.h"
#else
#define PIMAppProfilingBegin(name)
#define PIMAppProfilingEnd()
#define PIMAppProfilingReturnVoid()	return
#define PIMAppProfilingReturnValue(val)	(return (val))
#endif


#ifndef TxtCharIsWheelKey
//*********************************************************
// Macro for detecting if character is a rocker character
//*********************************************************

// <c> is a rocker key if the event modifier <m> has the command bit set
// and <c> is in the proper range
#define	TxtCharIsWheelKey(m, c)		((((m) & commandKeyMask) != 0) && \
									((((c) >= vchrThumbWheelUp) && ((c) <= vchrThumbWheelBack))))

#endif


/***********************************************************************
 *
 *	Constants
 *
 ***********************************************************************/
#define maxDescSize							256

// Length of verious
#define maxFrequenceFieldLen				3
#define defaultRepeatDecsLen				32
#define maxAdvanceFieldLen					3

#define datebookDBType						'DATA'

// Error conditions
#define datebookErrDuplicateAlarm			1		// Used only with AddPendingAlarm
#define datebookErrAlarmListFull			2		// Used only with AddPendingAlarm

// Feature numbers used by the datebook
#define alarmPendingListFeature				0		// List of pending alarms
#define alarmSnoozeTimeFeature				1
#define alarmPreviouslyDismissedFeature		2
#define recentFormFeature					3

#define datebookVersionNum					3
#define datebookPrefsVersionNum				5
#define datebookPrefID						0x00
#define datebookCurCategoriesID				0x01
#define datebookUnsavedPrefsVersionNum		2		// version 2 represents reformatted structure for attn mgr support
#define datebookUnsavedPrefID				0x00

#define dateExtension						"vcs"
#define dateMIMEType						"text/x-vCalendar"

// Column in the to do table on the day view.
#define timeBarColumn						0
#define timeColumn							1
#define descColumn							2

#define spaceAfterTimeBarColumn				0
#define spaceAfterTimeColumn				2

#define newEventSize  						16
#define maxNoteTitleLen						40

// Details timeZone display: space to right border
#define detailsTriggerSelectorsSpaceOnRight	10

// Fonts used by application
#define apptTimeFont 						stdFont
#define apptEmptyDescFont					stdFont
#define noteTitleFont						boldFont

// Token strings in repeating event description template.
#define frequenceToken						"^f"
#define dayOrdinalToken						"^x"
#define weekOrdinalToken					"^w"
#define monthNameToken						"^m"
#define dayNameToken						"^d"

// Default setting for the Details Dialog.
#define defaultAlarmAdvance					5
#define defaultAdvanceUnit					aauMinutes
	
// Default setting for the Repeating Events Dialog.
#define defaultRepeatFrequency				1
#define defaultRepeatEndDate				-1

// Repeat dialog "end on" popup list items.
#define repeatNoEndDateItem					0
#define repeatChooseDateItem				1

// End Date popup list chooses
#define noEndDateItem						0
#define selectEndDateItem					1

// Preference Dialog.
#define dayRangeTimeWidth					50
#define dayRangeTimeHeight					13

// Latest time
#define maxHours							23
#define maxMinutes							55

// Time bars
#define maxTimeBarColumns					5
#define timeBarWidth						2

// The duration to display the current time when the title of the
// Day View is pressed.
#define timeDisplayTime 					(SysTimeInMilliSecs(1500))			// 1 1/2 seconds
#define timeDisplayWaitTime 				(SysTimeInMilliSecs(100))			// 1/10 second

// noTime position in day
#define noTimeStartEndTime					(0x0000)

// Appointments overlapping midnight: begin and end times
#define overlapStartTime					(0x0000)		// 0:0		
#define overlapEndTime						(0x173B)		// 23:59		

// The day view limits
#define DayViewStartHour					DayStartHour	// Mapped on the worked day starting
#define DayViewEndHour						(23)			// Fixed to 11 pm

// Default databook app preference values
#define defaultDayStartHour					(8)
#define defaultDayEndHour					(18)
#define defaultAlarmPresetAdvance			(apptNoAlarm)
#define defaultAlarmPresetUnit				(aauMinutes)
#define defaultSaveBackup					(true)
#define defaultShowTimeBars					(true)
#define defaultCompressDayView				(true)
#define defaultShowTimedAppts				(true)
#define defaultShowUntimedAppts				(false)
#define defaultShowDailyRepeatingAppts		(false)
#define defaultAlarmSoundRepeatCount		(3)
#define defaultAlarmSoundRepeatInterval		(300)
#define defaultAlarmSoundUniqueRecID		(dmUnusedRecordID)
#define defaultAlarmSnooze					(300)
#define defaultRecentForm					(DayView)

// Attach transport scheme max size (will be "_mail", "send"...)
#define attachTransportPrefixMaxSize		16

// Invalid RefNum used for DateBook loading as a shared library
#define kRALInvalidRefNum	            	((uint32_t)0xffffffff)

// Most right position in fields (to be used with FldSetInsertionPoint()
// so the blinking cursor will be set at the end of the text in field.
#define mostRightFieldPosition				0xffffffff

/***********************************************************************
 *
 *	Datebook prefs structure
 *
 ***********************************************************************/


/***********************************************************************
 *
 *	Shared globals
 *
 ***********************************************************************/

extern	DmOpenRef				gApplicationDbP;					// application database
extern 	DatabaseID				gApplicationDbID;					// Application DB ID
extern 	DmOpenRef				ApptDB;								// datebook database

extern	DateType				Date;								// date currently displayed
extern	uint16_t				StartDayOfWeek;

// Dynamic Input Area handling
extern 	uint16_t				gCurrentFormID;						// currrent formID
extern 	FormPtr					gCurrentFormPtr;					// current form Ptr
extern	WinHandle				gWinHdl; 							// View window handle, to check win handle upon winResizedEvent receipt.
extern	RectangleType 			gCurrentWinBounds;					// Current window bounds.

// System preference
extern	TimeFormatType			TimeFormat;
extern	DateFormatType			LongDateFormat;			
extern 	DateFormatType			ShortDateFormat;

// Edition shared parameters
extern 	uint32_t				gApptCursorID;						// Cursor pointing on currently edited row
extern	uint32_t				gMonthViewCursorID;					// Cursor used within Month View
extern 	int16_t					gSelectedRow;						// Selected row - valid when gItemSelected is true
extern	int16_t					gSelectedColumn;					// Selected column - valid when gItemSelected is true
extern	Boolean					gItemSelected;						// true if a day view item is selected
extern 	Boolean 				gItemWasSelected;					// true if a day view item was selected for details
extern	Boolean					RecordDirty;						// true if a record has been modified
extern	uint32_t				DayEditPosition;					// position of the insertion point in the desc field
extern	uint32_t				DayEditSelectionLength;				// length of the current selection.
extern 	RGBColorType	 		colorLine;							// Color to draw week and month view lines in
extern 	uint16_t				TopVisibleAppt;
extern 	privateRecordViewEnum	CurrentRecordVisualStatus;			// applies to current record
extern 	privateRecordViewEnum	PrivateRecordVisualStatus;			// applies to all other records

extern char 					gDeviceTimeZone[TZNAME_MAX];
extern char						gLocalizedTimeZomeName[TZNAME_MAX];	

extern Boolean					gSecurityPasswordOpened;

// The following global variable are saved to a state file.
extern 	uint16_t				DayStartHour;						// start of the day 8:00am
extern 	uint16_t				DayEndHour;							// end of the day 11:00pm
extern  AlarmInfoType			AlarmPreset;						// default alarm settings.
extern	Boolean					SaveBackup;							// default setting "Backup to PC" checkbox
extern	Boolean					ShowTimeBars;						// show time bars in the day view
extern	Boolean					CompressDayView;					// remove empty time slot to prevent scrolling
extern	Boolean					ShowTimedAppts;						// show timed appointments in month view
extern	Boolean					ShowUntimedAppts;					// show untimed appointments in month view
extern	Boolean					ShowDailyRepeatingAppts;			// show daily repeating appointments in month view
extern	Boolean					EventInCurrentView;					// true if pen or key event has occurred in the
																	// current view (used only by Week and Month views)
extern	FontID					ApptDescFont;						// font for drawing event description


// The following global variable is used to control the displaying of the
// current time in the title of a view.
extern	Boolean					TimeDisplayed;							// True if time in been displayed
extern	Boolean					TimeComputeWaitTime;					// True to take into account TimeDisplayTick
extern	uint64_t				TimeDisplayTick;						// Tick count when we stop showing time


// The following globals are for the repeat rates of the alarms.
extern	uint16_t				AlarmSoundRepeatCount;					// number of times to repeat alarm sound
extern	uint16_t				AlarmSoundRepeatInterval;				// interval between repeat sounds, in seconds
extern	uint32_t				AlarmSoundUniqueRecID;					// Unique record ID of desired alarm sound
extern	uint16_t				AlarmSnooze;							// snooze duration, in seconds

// This global variable is only valid while editing the repeat info of an appointment.
extern	void *					ApptDetailsP;

// Categories handling
extern uint32_t					DateBkCurrentCategoriesCount;
extern CategoryID *				DateBkCurrentCategoriesP;						// currently displayed categories
extern char						DateBkCategoriesName[catCategoryNameLength];	// name of the current categories

// For attach sublaunch
extern Boolean					gAttachRequest;
extern char						gTransportPrefix[attachTransportPrefixMaxSize+1];

// The ToDo RefNum, used to access ToDo exported routines
extern uint32_t					ToDoRefNum;

// Colors for shaded effects
extern RGBColorType				MediumGreyColor;
extern RGBColorType				LightGreyColor;
extern RGBColorType				DarkGreyColor;
extern RGBColorType				DarkRedColor;
extern RGBColorType				WhiteColor;
extern RGBColorType				BlackColor;


#endif
