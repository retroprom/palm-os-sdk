/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDay.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  DateBook Day view source code.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>

#include <AboutBox.h>
#include <AppMgr.h>
#include <CatMgr.h>
#include <Control.h>
#include <ClipBoard.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <Event.h>
#include <FeatureMgr.h>
#include <FontSelect.h>
#include <FormLayout.h>
#include <List.h>
#include <Loader.h>
#include <Menu.h>
#include <PenInputMgr.h>
#include <PrivateRecords.h>
#include <Preferences.h>
#include <SelDay.h>
#include <SelTimeZone.h>
#include <SchemaDatabases.h>
#include <ScrollBar.h>
#include <SoundMgr.h>
#include <string.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <SysEvtMgr.h>
#include <SystemResources.h>
#include <Table.h>
#include <TimeMgr.h>
#include <TextMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <UIColor.h>


#include "Books.h"
#include "NoteViewLib.h"
#include "PhoneLookup.h"
#include "GUIUtilities.h"

// Lunar Calendar
#include "DateLunar.h"
#include "DateMonth.h"


#include "DateGlobals.h"
#include "DateAlarm.h"
#include "DateDB.h"
#include "DateDay.h"
#include "DateDayDetails.h"
#include "DateTransfer.h"
#include "DateUtils.h"
#include "DateU16List.h"
#include "Datebook.h"
#include "DateBackground.h"
#include "DatebookRsc.h"



/***********************************************************************
 *
 *	Local definitions
 *
 ***********************************************************************/
#define	kDayViewNoIcon					0
#define kDayViewNoteIcon				1
#define kDayViewAlarmIcon 				2
#define kDayViewRepeatIcon 				4
#define kDayViewTimeZoneIcon 			8

// focus states
#define kFocusDayNoFocus				((int16_t) noFocus)
#define kFocusDayTableIsFocused			1
#define kFocusDayTableRowIsFocused		2
#define kFocusDayDetailsOpened			3

//row directions
#define kFocusPreviousEventSlot			-1
#define kFocusSelectionMode 			0
#define kFocusNextEventSlot				1

// selection modes
#define kFocusLastSelectionMode			2
#define kFocusCurrentTimeEventSlotMode	3
#define kFocusFirstTimeMode				4
#define kFocusLastTimeMode				5
#define kFocusTableSelectionMode		6








/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
static MemHandle					sApptsH 		= NULL;
static uint16_t						sNumAppts 		= 0;
static uint16_t						sMaxNumAppts 	= 0; 			// number of available rows in the table

static MemHandle					sApptsOnlyH 	= NULL;
static int32_t						sNumApptsOnly 	= 0;

static uint16_t						sTimeBarColumns = 0;			// Number of time bar columns
static BookType*					sWeekBook 		= NULL;

static TabInfoType   				sWeekTabs[daysInWeek] =
{
	{ "Sun", 	DayBookDOW1TabId},
	{ "Mon", 	DayBookDOW2TabId},
	{ "Tue", 	DayBookDOW3TabId},
	{ "Wed", 	DayBookDOW4TabId},
	{ "Thu", 	DayBookDOW5TabId},
	{ "Fri",  	DayBookDOW6TabId},
	{ "Sat", 	DayBookDOW7TabId}
};

static int32_t						sDescriptionFieldMaxLength 	= 0;
static MemHandle					sDescriptionEditedHolderH 	 = NULL;
static MemHandle					sDescriptionNotEditedHolderH = NULL;
static int32_t						sCurrentFieldOffset = 0;
static RectangleType 				sTitleBounds = {0,0,0,0};

// icons tracking global static variables
static Boolean						sIconTracking = false;
static TableType* 					sTableP = NULL;
static uint16_t 					sRow = 0xFFFF;
static RectangleType 				sIconBounds = {0, 0, 0, 0};
static uint16_t						sIconHit = kDayViewNoIcon;
static Boolean 						sIconSelected = false;
static Boolean 						sDayViewscrollableUp  = false;
static Boolean 						sDayViewscrollableDown = false;

static uint32_t						sApptRowID = dbInvalidRowID;
static RGBColorType					sObjectSelectedInkColor;
static RGBColorType					sObjectSelectedFillColor;
static RGBColorType					sObjectInkColor;
static RGBColorType					sObjectFillColor;

static int16_t						sDayTableFocusState = kFocusDayNoFocus;
static int16_t						sDayViewLastRowFocused = tblUnusableRow;
static int16_t						sDayViewSelectionMode = kFocusCurrentTimeEventSlotMode;
static uint16_t						sDayViewFocusedItemID = frmInvalidObjectId;

static DateGadgetCallbackDataType	sGadgetCallbackData;
static RGBColorType					sBackgroundHighlight;
static uint16_t						sBackgroundMonth = 0xffff;

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void 	DayViewInit (FormPtr frmP, Boolean loadDBandDefineLayout);
static void 	DayViewDrawTimeBars (FormPtr formP);
static void 	DayViewDrawOverlappingBars (FormPtr formP);
static void 	DayViewDrawInvalidate(RectangleType* dirtyRect);
static void 	DayViewDrawVerticalBar(TablePtr tableP);
static void 	DayViewDrawVerticalBarAdapter(FormPtr formP);
static uint16_t DayViewFocusTableRegainFocus(void);
static void 	DayViewFocusSetNavState(void);
static Boolean 	DayViewTableValidRow(TablePtr tableP, int16_t row);
static Boolean 	DayViewFocusSelectRowID(uint32_t rowID);





/***********************************************************************
 *
 * FUNCTION:    	DrawUntimedEventsIcon
 *
 * DESCRIPTION: 	Draws the given time. Used by the custom draw
 *					routines for the date columns of the agenda and
 *					day views.
 *
 * PARAMETERS:  ->	row:
 *              ->	RectangleType:
 *              ->	normalIconID:
 *              ->	highlightedNormalID:
 *              ->	disabledIconID:
 *              ->	highlightedDisabledIconID:
 *              ->	belongingToCurrCat:
 *              ->	overState:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL		07/01/04	Initial Revision.
 *
 ***********************************************************************/
static void DrawUntimedEventsIcon(
	int16_t						row,
	RectangleType*				boundsP,
	uint16_t 					normalIconID,
	uint16_t 					highlightedNormalID,
	uint16_t 					disabledIconID,
	uint16_t 					highlightedDisabledIconID,
	Boolean 					conflictingAppt,
	MidnightOverlapStateType 	overState)
{
	MemHandle					resH;
	BitmapType*					wrappingIconBmpP;
	Coord						bmpHeight;
	Coord						bmpWidth;
	Coord						x;
	Coord						y;
	Coord						dy;
	uint16_t					iconID;
	uint16_t					descLineHeight;
	FontID						curFont;


	curFont	= FntSetFont (ApptDescFont);
	descLineHeight = FntLineHeight();
	FntSetFont(curFont);

	// does the event conflicting with an even tin another category or not
	// or is an overlappinfg event (event spanning on two days)
	if (conflictingAppt || overState == overlapScndPart)
	{

		// the row does not belong to the current Category of dayview
		// we use the shaded time zone icon.

		// normal: 			ShadedTimeZoneIconBitmap
		// hightlighted: 	HighlightedShadedTimeZoneIconBitmap
		iconID = disabledIconID;
	}
	else
	{
		// the row  belongs to the current Category of dayview
		// we use the time zone icon.

		// normal: 			TimeZoneIconBitmap
		// hightlighted: 	HigthlightedTimeZoneBitmap
		iconID = normalIconID;
	}

	// Load  bitmap
	resH = DmGetResource(gApplicationDbP, bitmapRsc, iconID);

	wrappingIconBmpP = (BitmapType*) MemHandleLock(resH);

	BmpGetDimensions (wrappingIconBmpP, &bmpWidth, &bmpHeight, NULL);

	// the icon is assumed smaller than time column !!
	// we center the icon.
	x = boundsP->topLeft.x + ((boundsP->extent.x - bmpWidth) >> 1);

	// we center the icon taking into account the height of the current day view table font.
	dy = descLineHeight - bmpHeight;

	if (dy < 0)
		dy = 0;

	y = boundsP->topLeft.y + (dy >>1);

	WinDrawBitmap(wrappingIconBmpP, x, y);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);

}


/***********************************************************************
 *
 * FUNCTION:    	DrawTime
 *
 * DESCRIPTION: 	Draws the given time. Used by the custom draw
 *					routines for the date columns of the agenda and
 *					day views.
 *
 * PARAMETERS:  ->	inTime:				the time.
 *				->	overlapState:
 *				->	noTimeEvent:
 *				->	conflictingAppt:
 *              ->	inFormat			time-to-ascii conversion format.
 *				->	inFont:
 *				->	inJustification:
 *				->	inBoundsP			drawing area.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb		6/4/99	Initial revision.
 *
 ***********************************************************************/
void DrawTime (
	TimeType					inTime,
	MidnightOverlapStateType 	overlapState,
	Boolean						noTimeEvent,
	Boolean 					conflictingAppt,
	TimeFormatType				inFormat,
	FontID 						inFont,
	JustificationType 			inJustification,
	RectangleType* 				inBoundsP)
{
	FontID						curFont;
	char						timeStr [timeStringLength];
	TimeFormatType				format;
	size_t						len;
	uint16_t					width;
	int16_t						x;
	RGBColorType				prevTextColor;
	WinDrawOperation			oldDraw;


	// No-time appointment?
	if (noTimeEvent)
		// should be drawn calling DrawNoTimeEventIcon
		return;


	// Draw icon if this is the second part of an overlapping appointment
	if ((overlapState == overlapScndPart)
		|| conflictingAppt
		||(overlapState == overlapFirstPartCommonRow))
		WinSetTextColorRGB(&MediumGreyColor, &prevTextColor);



	if (inFormat == tfColonAMPM)
		format = tfColon;
	 else if (inFormat == tfDotAMPM)
	 	format = tfDot;
	 else
		format = inFormat;

	TimeToAscii ((uint8_t) (inTime.hours % 24), (uint8_t)inTime.minutes, format, timeStr);


	// Use the string width and alignment to compute its starting point
	len = strlen (timeStr);
	curFont = FntSetFont (inFont);
	width = FntCharsWidth (timeStr, len);
	x = inBoundsP->topLeft.x;
	switch (inJustification)
	{
		case rightAlign:
			x += inBoundsP->extent.x - width;
			break;

		case centerAlign:
			x += (inBoundsP->extent.x - width) / 2;
			break;
	}

	x = max (inBoundsP->topLeft.x, x);

	oldDraw = WinSetDrawMode(winOverlay);
	// Draw the time
	WinPaintChars (timeStr, (int16_t) len, x, inBoundsP->topLeft.y);
	WinSetDrawMode(oldDraw);

	// Restore initial font
	FntSetFont (curFont);

	// Restore initial color
	if ((overlapState == overlapScndPart)
		|| conflictingAppt
		||(overlapState == overlapFirstPartCommonRow))
		WinSetTextColorRGB(&prevTextColor, NULL);
}


/***********************************************************************
 *
 * FUNCTION:    SetDateToNextOccurrence
 *
 * DESCRIPTION: 	This routine set the "current date" global variable
 *					to the date that the specified record occurs on.
 *					If the record is a repeating appointmnet, we set
 *					the date to the next occurrence of the appointment.
 *					If we are beyond the end date of the repeating
 *					appointment, we set the date to the last occurrence
 *					of the event.
 *
 * PARAMETERS:  ->	rowID:	row ID / cursor ID of the appointment record.
 *
 * RETURNED:    	true if successful, false if not.  It's posible that
 *              	a repeating event may have no displayable occurrences.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	8/24/95		Initial revision.
 *
 ***********************************************************************/
static Boolean SetDateToNextOccurrence (uint32_t rowID)
{
	Boolean 			dateSet = true;
	DateType 			today;
	DateTimeType 		dateTime;
	ApptDBRecordType 	apptRec;
	Boolean 			inCategory;
	CategoryID 			allCategoryID[] = {catIDAll};

	ApptGetRecord (ApptDB, rowID, &apptRec,
		DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);

	if (apptRec.repeat.repeatType == repeatNone)
		Date = apptRec.when.date;

	// If the appointment is a repeating event,  go to the date of the
	// next occurrence of the appointment.
	else
	{
		// Get today's date.
		TimSecondsToDateTime (TimGetSeconds (), &dateTime);
		today.year = dateTime.year - firstYear;
		today.month = dateTime.month;
		today.day = dateTime.day;

		Date = today;
		if ( ! ApptNextRepeat (&apptRec, &Date, true))
		{
			// If we are beyond the end date of the repeating event, display
			// the last occurrence of the event.
			Date = apptRec.repeat.repeatEndDate;

			if ( ! ApptNextRepeat (&apptRec, &Date, false))
			{
				// It possible that there are no occurences that are displayable
				// (ex: an expections is created for each occurrences),  if so
				// just go to today.
				ErrDisplay ("No displayable occurrences of event");
				Date = today;
				dateSet = false;
			}
		}
	}

	ApptFreeRecordMemory (&apptRec);

	// Check that the record is in current category - If not, switch to all
	if (DbIsRowInCategory(ApptDB, rowID, DateBkCurrentCategoriesCount,
		DateBkCurrentCategoriesP, DbMatchAny, &inCategory) < errNone)
	{
		dateSet = false;
	}
	else
	{
		// The record exists but is not in current category, switch to "All" category
		ChangeCurrentCategories(1, allCategoryID);
	}

	return (dateSet);
}


/***********************************************************************
 *
 * FUNCTION:    	MoveEvent
 *
 * DESCRIPTION:  	This routine changes the date and / or time of the
 *               	specified event.
 *
 *					Current row ID in the cursor can change.
 *					Original rowID is the current cursor position.
 *
 *					When the a new record is added,as a result of moving,
 *					the event:
 *					- Cursor is requried,
 *					- the current row ID is set to the new record
 *					- moved is set to true;
 *
 *
 * PARAMETERS:  ->	cursorID:			cursorID (notRowID, only cursor.
 *              -> 	newStartTime:		Updated start time.
 *              -> 	newEndTime:			Updated end time.
 *				-> 	newDate:			Date on which the move is applied.
 *				-> 	midnightWrapped:	Is the event overlapping ?
 *				-> 	noTimeEvent:		Is the event untimed ?
 *				-> 	timeChangeOnly:		If true, update only the time,
 *										not the beginning date.
 *				<- 	moved:				true, if the event actually moved.
 *
 * RETURNED:    	errCode.
 *
 * NOTE:
 *
 * REVISION HISTORY:
 *	art	4/01/96		Initial revision.
 *	gap	9/08/99		Add support for moving only current, current & future,
 *									or all occurrences of the event.
 *	gap	9/15/99		Clean up Current/Future/All handling for weekview.
 *	PPL	07/10/03	Cursors !
 * 	PPL 10/15/03	add cursorID parameter for Cusor Caching.
 *
 ***********************************************************************/
status_t MoveEvent (
	uint32_t				cursorID,
	TimeType 				newStartTime,
	TimeType 				newEndTime,
 	DateType 				newDate,
 	Boolean 				midnightWrapped,
 	Boolean 				noTimeEvent,
 	Boolean 				timeChangeOnly,
 	Boolean* 				movedP)
{
	status_t				err;
	uint16_t				id;
	uint16_t				dayOfWeek;
	uint16_t				alertButton;
	int32_t					adjust;
	FormPtr					alert;
	Boolean					exception 	= false;
	Boolean					splitEvent 	= false;
	Boolean					applyToAll 	= false;
	ApptDBRecordType		newRec;
	ApptDBRecordType		apptRec;
	ApptDateTimeType		when;
	RecordFilteringInfoType	changedFields;
	Boolean					hasAlarm, isRepeating;
	uint32_t 				numCategories;
	uint32_t				rowID;
	CategoryID *			pCategoryIDs;

	// uses cursorID current position
	ApptGetRecord (ApptDB, cursorID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT);

	hasAlarm = (Boolean) (apptRec.alarm.advance != apptNoAlarm);
	isRepeating = (Boolean) (apptRec.repeat.repeatType != repeatNone);

	*movedP = false;

	// If we're changing a repeating appointmemt, check if the changes will
	// be applied to:
	//  - all occurrences of the event
	//  - the current and all future occurrences of the event
	//  - only the current event  (create an expection)
	if (isRepeating)
	{
		alert = FrmInitForm(gApplicationDbP, RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);

		// If the alert was canceled don't apply any of the changes.
		if (alertButton == RangeCancelButton)
		{
			ApptFreeRecordMemory (&apptRec);
			return (0);
		}

		// If the change is to be applied to the selected occurrence only
		// an exception will be created and the specified date and time
		// will be applied.
		else if (alertButton == RangeCurrentButton)
			exception = true;

		// If the change is to be applied to the current and future
		// events, the current record will need to be split, and the
		// specified date and time will be applied.
		else if (alertButton == RangeFutureButton)
			splitEvent = true;

		// If the change is to be applied to the all occurrences of the event
		// the code below will do one of two things.  If the user only changed
		// the time of the event, the original start date will be maintained and only
		// the time change will be applied.  If the user changes both the time and date,
		// the specified date and time will be applied.
		else if (alertButton == RangeAllButton)
		{
			applyToAll = true;

			if (timeChangeOnly)
				newDate = apptRec.when.date;
		}
		else {
			ApptFreeRecordMemory (&apptRec);
			return (0);
		}

	}

	// if there is an alarm associated with the item, remove it from
	// the attention manager queue before changing the event
	if (apptRec.alarm.advance != apptNoAlarm)
	{
		hasAlarm = true;
		DeleteAlarmIfPosted(cursorID);
	}

	ApptFreeRecordMemory (&apptRec);

	// Add an exception to the current record,  and create a new record
	// at the new time.
	if (exception)
	{
		err = ApptAddException (ApptDB, cursorID, newDate);
		if (err < errNone)
			goto Exit;

		ApptGetRecord (ApptDB, cursorID, &apptRec, DBK_SELECT_ALL_FIELDS);
		DbGetCategory (ApptDB, cursorID, &numCategories, &pCategoryIDs);
		DbCursorGetCurrentRowID(cursorID, &rowID);

		when.startTime = newStartTime;
		when.endTime = newEndTime;
		when.date = newDate;
		when.midnightWrapped = midnightWrapped;
		when.noTimeEvent = noTimeEvent;
		strncpy(when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);

		memset (&newRec, 0x00, sizeof (newRec));
		newRec.description = apptRec.description;
		newRec.note = apptRec.note;
		newRec.when = when;
		newRec.alarm = apptRec.alarm;
		newRec.location = apptRec.location;

		err = CreateExceptionAndMove (&newRec, cursorID, NULL, numCategories, pCategoryIDs);

		if (ApptCountMultipleOccurences(&apptRec) == 1)
		{
			if (apptRec.alarm.advance != apptNoAlarm)
				DeleteAlarmIfPosted(rowID);

			err = ApptDeleteRecord (ApptDB, rowID, false);
		}

		ApptFreeRecordMemory (&apptRec);
		DbReleaseStorage(ApptDB, pCategoryIDs);

		if (err < errNone)
			goto Exit;
	}

	// Change the time of the current record.
	else
	{
		changedFields = 0;
		memset (&newRec, 0x00, sizeof (newRec));

		if (isRepeating)
		{
			if (splitEvent)
			{
				err = SplitRepeatingEvent (cursorID);
				if (err < errNone)
					goto Exit;
			}

			ApptGetRecord (ApptDB, cursorID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_REPEAT);

			// When changing the entire range of event, reset the start date by calculating
			// the number of days the user moved the event and apply the delta to the event's
			// start date, maintain the duration and clear the exceptions list.
			newRec.repeat = apptRec.repeat;

			// Maintain the duration of the event.
			if ((DateToInt (apptRec.repeat.repeatEndDate) != apptNoEndDate) && !(splitEvent))
			{
				adjust = (int32_t) DateToDays (newDate) -
							(int32_t) DateToDays (apptRec.when.date);

				DateAdjust (&newRec.repeat.repeatEndDate, adjust);

				changedFields |= DBK_SELECT_REPEAT;
			}

			// If the repeat type is weekly and the start date of the event
			// has been changed,  update the 'repeat on' field, which contains
			// the days of the week the event repeats on, such that the
			// event occurs on the start date.
			if (apptRec.repeat.repeatType == repeatWeekly)
			{
				dayOfWeek = DayOfWeek (apptRec.when.date.month,
					apptRec.when.date.day,
					(int16_t)(apptRec.when.date.year + firstYear));
				newRec.repeat.repeatOn &= ~(1 << dayOfWeek);

				dayOfWeek = DayOfWeek (newDate.month,
					newDate.day,
					(int16_t)(newDate.year + firstYear));
				newRec.repeat.repeatOn |= (1 << dayOfWeek);

				changedFields |= DBK_SELECT_REPEAT;
			}

			// If the repeat type is monthly by day, get the day of the month (ex:
			// first Friday) of the start date of the event.
			else if (apptRec.repeat.repeatType == repeatMonthlyByDay)
			{
				newRec.repeat.repeatOn = (uint8_t) DayOfMonth (newDate.month, newDate.day, (int16_t)(newDate.year + firstYear));
				changedFields |= DBK_SELECT_REPEAT;

				// If we're in the fourth week, and the fourth week is also the last
				// week,  ask the user which week the event repeats in (fourth or last).
				if ( ((newRec.repeat.repeatOn / daysInWeek) == 3) &&
						(newDate.day + daysInWeek > DaysInMonth (newDate.month, (int16_t)(newDate.year + firstYear))))
				{
					alert = FrmInitForm(gApplicationDbP, MonthlyRepeatDialog);

					// Set the 4th / last push button.
					if (apptRec.repeat.repeatOn > dom4thSat)
						id = MonthlyRepeatLastButton;
					else
						id = MonthlyRepeatFourthButton;
					FrmSetControlGroupSelection (alert, MonthlyRepeatWeekGroup,id);

					alertButton = FrmDoDialog (alert);

					if (FrmGetObjectIndex (alert, MonthlyRepeatLastButton) ==
						 FrmGetControlGroupSelection (alert, MonthlyRepeatWeekGroup))
						newRec.repeat.repeatOn += daysInWeek;

					FrmDeleteForm (alert);

					if (alertButton == MonthlyRepeatCancel)
					{
						ApptFreeRecordMemory (&apptRec);
						return errNone;
					}
				}
			}

			// If the record is a repeating appointment, and the start date has changed,
			// then clear the exceptions list.
			if ( !(applyToAll && timeChangeOnly) )
			{
				ApptRemoveExceptions(&newRec);
				changedFields |= DBK_SELECT_EXCEPTIONS;
			}
		}
		else
		{
			ApptGetRecord (ApptDB, cursorID, &apptRec, DBK_SELECT_TIME);
		}

		when.startTime = newStartTime;
		when.endTime = newEndTime;
		when.date = newDate;
		when.midnightWrapped = midnightWrapped;
		when.noTimeEvent = noTimeEvent;
		strncpy(when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);
		newRec.when = when;
		changedFields |= DBK_SELECT_TIME;

		// ApptFreeRecordMemory is performed by ApptChangeRecord
		err = ApptChangeRecord (ApptDB, cursorID, &newRec, changedFields);
		if (err < errNone)
			goto Exit;
	}

	// If the appointment has an alarm, reschedule the next alarm.
	if (hasAlarm)
		RescheduleAlarmsAllRows();

	*movedP = true;
	return errNone;

Exit:
	FrmUIAlert(DeviceFullAlert);
	return (err);
}


/***********************************************************************
 *
 * FUNCTION:    	PurgeRecords
 *
 * DESCRIPTION: 	This routine deletes appointments that are before the
 *              	user specified date.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	True if the current view may need to be redrawn.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	8/1/95	Initial Revision
 *	gap	8/1/00	add attention manager support
 *
 ***********************************************************************/
static Boolean PurgeRecords (void)
{
	status_t			err;
	int32_t 			adjust;
	uint16_t 			ctlIndex;
	uint16_t 			buttonHit;
	uint16_t 			rangeItem;
	ListPtr 			lst;
	FormPtr 			alert;
	Boolean 			purge;
	Boolean 			archive = false;
	DateType 			purgeDate;
	DateTimeType 		todayDate;
	ApptDBRecordType 	apptRec;
	Boolean  			hasAlarm;
	Boolean				reOpenDatabase = false;
	uint32_t			purgeCursorID;

	// Display an alert to comfirm the operation.
	alert = FrmInitForm(gApplicationDbP, PurgeDialog);

	ctlIndex = FrmGetObjectIndex (alert, PurgeSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);

	buttonHit = FrmDoDialog (alert);

	archive = (Boolean) FrmGetControlValue (alert, ctlIndex);

	lst = FrmGetObjectPtr (alert, FrmGetObjectIndex (alert, PurgeRangeList));
	rangeItem = LstGetSelection (lst);

	FrmDeleteForm (alert);

	if (buttonHit == PurgeCancel)
		return (false);

	SaveBackup = archive;

	// Compute the purge date.
	TimSecondsToDateTime (TimGetSeconds (), &todayDate);

	// One, two, or three weeks
	if (rangeItem < 3)
		adjust = (int32_t)((rangeItem + 1) * daysInWeek);

	// One month
	else if (todayDate.month > january)
		adjust = DaysInMonth ((int16_t)(todayDate.month - 1), todayDate.year);
	else
		adjust = DaysInMonth (december, (int16_t)(todayDate.year - 1));

	purgeDate = CnvDateTime2Date(&todayDate);
	DateAdjust (&purgeDate, -adjust);

	// Close the main cursor, or we may not be able to delete currently
	// viewed records - it will be re-opned later
	err = ApptCloseCursor(&gApptCursorID);

	// close and reopen database when user asked to hide private reccords
	if (PrivateRecordVisualStatus == hidePrivateRecords)
	{
		err = DateCloseDatabase();
		err = DateReOpenDatabase(false, (dmModeReadWrite | dmModeShowSecret));
		reOpenDatabase = true;
	}

	// Create new global cursor
	err = ApptOpenCursor(ApptDB, DatebookGlobalRequest, dbCursorEnableCaching, &purgeCursorID);

	// Delete records.
	if (! DbCursorGetRowCount(purgeCursorID))
		return (false);


	for (DbCursorMoveFirst(purgeCursorID) ; !DbCursorIsEOF(purgeCursorID) ; DbCursorMoveNext(purgeCursorID))
	{
		ApptGetRecord (ApptDB, purgeCursorID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT);

		if (apptRec.repeat.repeatType != repeatNone)
			purge = (Boolean) (DateToInt(apptRec.repeat.repeatEndDate) <= DateToInt(purgeDate));
		else
			purge = (Boolean) (DateToInt(apptRec.when.date) <= DateToInt(purgeDate));

		hasAlarm = (Boolean) (apptRec.alarm.advance != apptNoAlarm);
		ApptFreeRecordMemory (&apptRec);

		if (purge)
		{
			// if the event to be delete had an alarm, be sure to remove it
			// from the posted alarm queue before the event is deleted.
			if (hasAlarm)
				DeleteAlarmIfPosted(purgeCursorID);

			if (archive)
				err = DbArchiveRow (ApptDB, purgeCursorID);
			else
				err = DbDeleteRow (ApptDB, purgeCursorID);

			// Deleted record will be automatically resorted
			DbgOnlyFatalErrorIf(err < errNone, "Could not delete record");
		}
	}

	// Close the purge cursor
	ApptCloseCursor(&purgeCursorID);

	if (reOpenDatabase)
	{
		err = DateCloseDatabase();
		err = DateReOpenDatabase(true, 0);
	}

	// This will open the gApptCursorID back up
	DayViewLoadApptsFromDB();

	// Update the global cursor
	ApptDBRequery(ApptDB, gApptCursorID, true);

	// Schedule the next alarm.
	RescheduleAlarmsAllRows();

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    	SelectFont
 *
 * DESCRIPTION: 	This routine handles selection of a font in the List
 *              	View.
 *
 * PARAMETERS:  ->	currFontID:	id of current font.
 *
 * RETURNED:    	id of new font
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:

 *	ART	9/10/97		Initial revision.
 *
 ***********************************************************************/
static FontID SelectFont (FontID currFontID)
{
	uint16_t 	formID;
	FontID 		fontID;

	formID = (FrmGetFormId (FrmGetActiveForm ()));

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (currFontID);

	// If the font changed, refresh the view without reloading
	if (fontID != currFontID)
		DayViewRefreshDisplay(false, true);

	return (fontID);
}


/***********************************************************************
 *
 * FUNCTION:    	DetailsEditSelectNote
 *
 * DESCRIPTION: 	This routine calls the shared lib NoteViewLib exported
 *					routine called EditNote.
 *
 * PARAMETERS:  -> 	rowID:
 *				->	eventP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PLE	01/01/03	Initial revision.
 *
 ***********************************************************************/
 static void DetailsEditSelectNote (uint32_t rowID, EventType * eventP)
 {
	ApptDBRecordType	apptRec;
	char * 				descP;
	uint32_t			pos = 0;

	if (DbIsCursorID(rowID))
		DbCursorGetCurrentPosition(rowID, &pos);

	// Get current record and related info.
	ApptGetRecord (ApptDB, rowID, &apptRec, DBK_SELECT_DESCRIPTION);
	descP = apptRec.description;

	EditNoteSelection(descP, ApptDB, rowID, pos, EVENT_NOTE_ID,
		(uint16_t) eventP->data.frmGoto.matchPos, (uint16_t) eventP->data.frmGoto.matchLen,
		NoteViewDeleteNote);

	// Free the appointment record links
	ApptFreeRecordMemory (&apptRec);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewRestoreEditStateFullfill
 *
 * DESCRIPTION: 	The function finishes edit state restoration.
 *					outside an udpate event...
 *
 *					Event is posted ONLY by DayViewRestoreEditState.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	true if the edit state was restored, false if it was
 *					already set or the record was not displayable.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/13/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewRestoreEditStateFullfill(void)
{
	FormPtr 					frmP;
	TablePtr					tableP;
	FieldPtr					fldP;
	ApptInfoPtr					apptsP;
	uint32_t					currentRowIDInCursor;
	int16_t 					row;
	int16_t 					tableIndex;
	uint16_t					apptIndex;
	MidnightOverlapStateType	overlapState;

	// Don't do anything if nothing selected
	if ( ! gItemSelected)
		return false;

	frmP = FrmGetFormPtr (DayView);
	tableIndex = FrmGetObjectIndex (frmP, DayTable);
	tableP = FrmGetObjectPtr (frmP, tableIndex);

	DbCursorGetCurrentRowID(gApptCursorID, &currentRowIDInCursor);

	if (TblFindRowData (tableP, currentRowIDInCursor, &row))
	{
		apptIndex = TblGetRowID(tableP, row);
		apptsP = MemHandleLock(sApptsH);

		overlapState = apptsP[apptIndex].overlapState;

		MemHandleUnlock(sApptsH);

		if (!(overlapState == overlapFirstPartCommonRow))
		{

			TblSelectItem(tableP, row, timeColumn);
			FrmSetFocus (frmP, tableIndex);

			if (TblGetItemStyle(tableP, row, descColumn) != customTableItem)
			{
				TblGrabFocus(tableP, row, descColumn);

				fldP = TblGetCurrentField(tableP);

				if (fldP)
				{
					if (DayEditSelectionLength)
						FldSetSelection(fldP, DayEditPosition, DayEditPosition + DayEditSelectionLength);
					else
						FldSetInsertionPoint(fldP, DayEditPosition);

					FldGrabFocus(fldP);
				}
			}
		}

		DayViewFocusTableRegainFocus();
	}

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewRestoreEditState
 *
 * DESCRIPTION: 	This routine restores the edit state of the day view,
 *              	if the view is in edit mode. This routine is
 *              	called when the time of an appointment is changed,
 *					or when returning from the details dialog or note view.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	true if the edit state was restored, false if it was
 *					already set or the record was not displayable.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	7/28/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *	PPL 10/13/03	Fixes for being update based:
 *					Enqueue a datebookRestoreSelection event.
 *
 ***********************************************************************/
static Boolean DayViewRestoreEditState (void)
{
	FormPtr 		frmP;
	TablePtr 		tableP;
	uint32_t 		currentRowIDInCursor;
	int16_t 		row;
	int16_t 		tableIndex;
	Boolean			verifyPW;

	if ( ! gItemSelected)
		return false;

	frmP = FrmGetFormPtr (DayView);
	tableIndex = FrmGetObjectIndex (frmP, DayTable);
	tableP = FrmGetObjectPtr (frmP, tableIndex);

	// Check that the form is not already in an edit state : this is
	// important because this routine is called after each update event
	// sent to refresh the view.
	if(FrmGetFocus(frmP) != noFocus && TblGetCurrentField(tableP) != NULL)
	{
		// Row already selected, skip following code
		return false;
	}

	// Find the row that the current record is in.  It's possible
	// that the current record is no longer displayable (ex: the record
	// was marked private).
	if ((DbCursorGetCurrentRowID(gApptCursorID, &currentRowIDInCursor) < errNone) ||
		(! TblFindRowData (tableP, currentRowIDInCursor, &row)))
	{
		ClearEditState ();
		return false;
	}

	// Update highlighting and force focus on the row containing the
	// current record to edit
	if (TblRowMasked(tableP,row))
	{
		// The only time an item might be the current selection but still masked
		// is if the user is "going to" the event from the attention manager.  In
		// that case we need to query for the password then unmask the event.
		verifyPW = DateUtilsSecVerifyPW(showPrivateRecords);

		// Load appointments, this will reopen the cursor
		DayViewLoadApptsFromDB();

		// Set the current position lost because of cursor deletion
		if (DbCursorMoveToRowID(gApptCursorID, currentRowIDInCursor) < errNone)
		{
			ClearEditState();
			DayViewRefreshDisplay(false, true);
			return true;
		}

		DayViewRefreshDisplay(false, false);

		if (verifyPW)
		{
			// We only want to unmask this one record, so restore the preference.
			// to its previous state (not maskPrivateRecords as we can have masked records
			// whithout having set the security level)
			PrefSetPreference (prefShowPrivateRecords, PrivateRecordVisualStatus);


			// Unmask just the current row.
			TblSetRowMasked(tableP, row, false);

			// Draw the row unmasked.
			TblMarkRowInvalid (tableP, row);

			TblRedrawTable(tableP);

			//DayViewDrawTimeBars(frmP);

			// Only change the visual status of this record, leaving all others masked.
			CurrentRecordVisualStatus = showPrivateRecords;
		}
		else
		{
			// Remove the edit state: the user did not enter a valid password
			ClearEditState ();
			return true;
		}
	}

	// post a datebookRestoreSelectionto finish edit field state restauration outside
	// an update event. usefull when we are update based
	DayViewRestoreEditStateFullfill();
	//DateBkEnqueueEvent(datebookRestoreSelection);
	// when we are backbuffered windows

	return true;
}




/***********************************************************************
 *
 * FUNCTION:    	DayViewClearEditState
 *
 * DESCRIPTION: 	This routine clears the edit state of the day view.
 *              	It is called whenever a table item is selected.
 *
 *              	If the new item selected is in a different row than
 *              	the current record the edit state is cleared,  and if
 *              	current record is empty it is deleted.
 *
 * PARAMETERS:  ->	newRow:	row number of newly table item
 *
 * RETURNED:    	True if the current record is deleted and items
 *              	of the table have been move around on the display
 *              	as a result.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	7/28/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
Boolean DayViewClearEditState (void)
{
	int16_t 		row = 0;
	int16_t 		selRow;
	int16_t 		selColumn;
	uint32_t 		currentRowIDInCursor;
	TablePtr 		tableP;
	FormPtr 		formP;
	FieldPtr		fieldP;
	uint16_t 		attr;
	uint16_t		focusedItem = noFocus;
	Boolean 		found = false;
	Boolean 		needsRedrawingTable = false;
	Boolean			itemDeleted = false;


	if (!gItemSelected)
		return (false);

	formP = FrmGetFormPtr (DayView);
	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));

	if (DbCursorGetCurrentRowID(gApptCursorID, &currentRowIDInCursor) >= errNone)
		found = TblFindRowData (tableP, currentRowIDInCursor, &row);

	// If the time column item was selected, unselect it
	if (found && TblGetSelection(tableP, &selRow, &selColumn))
		TblUnhighlightSelection(tableP);

	// get current focus
	focusedItem = FrmGetFocus(formP);

	if (focusedItem != noFocus)
	{
		// Release the focus
		FrmSetFocus (formP, noFocus);
		TblReleaseFocus(tableP);
		fieldP = TblGetCurrentField(tableP);
		if (fieldP)
			FldReleaseFocus(fieldP);
	}


	// We're leaving a record. If it's secret and we're masking secret records
	// (but unmasked just this one), then re-mask it now.
	if (found && (CurrentRecordVisualStatus != PrivateRecordVisualStatus))
	{
		CurrentRecordVisualStatus = PrivateRecordVisualStatus;

		// Is the record still secret? It may have been changed from the
		// details dialog.
		DbGetRowAttr (ApptDB, gApptCursorID, &attr);

		if (attr & dmRecAttrSecret)
		{
			// Re-mask the current row.
			TblSetRowMasked(tableP, row, true);

			// Draw the row masked.
			TblMarkRowInvalid (tableP, row);
			needsRedrawingTable = true;
		}
	}

	// If a different row has been selected, clear the edit state, this
	// will delete the current record if its empty.
	itemDeleted = ClearEditState ();
	if (itemDeleted)
	{
		//numAppts = sNumAppts;

		// Layout the day again, reload from DB and refresh display.
		DayViewRefreshDisplay(true, true);
		needsRedrawingTable = false;

		// Return the deleted state that is if a row was actually removed
		// from the table based on the numAppts before and after the
		// DayViewRefreshDisplay()
		//itemDeleted = (numAppts != sNumAppts) ? true : false;
	}

	if (needsRedrawingTable)
		TblInvalidate(tableP);


	return itemDeleted;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewFindAppointment
 *
 * DESCRIPTION: 	Given the database index of a record, this routine
 *              	finds the record in the appointment list and returns
 *             		the index of the appointment
 *
 * PARAMETERS:  ->	cursorID: 	The cursor ID pointing on selected
 *								appointment.
 *
 * RETURNED:   		Appointment list index.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	7/27/95		Initial revision.
 *  PPL				cursors !
 *
 ***********************************************************************/
static uint16_t DayViewFindAppointment (uint32_t cursorID, ApptInfoPtr apptsP)
{
	uint32_t 	currentRowIDInCursor;
	uint16_t 	i;

	if (DbCursorGetCurrentRowID(cursorID, &currentRowIDInCursor) < errNone)
	{
		// Couldn't find the appointment - Clear the edit state
		ClearEditState ();
		return (0);
	}

	for (i = 0; i < sNumAppts; i++)
	{
		if (apptsP[i].rowID == currentRowIDInCursor)
			return (i);
	}

	// If we're beyond the maximun number of appointment that can be
	// shown on a day, then the record passed may not be in the list.
	if (sNumApptsOnly >= apptMaxPerDay)
		ClearEditState ();

	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawTimeUniqueBar
 *
 * DESCRIPTION: 	This routine draw one time bar the indicate the
 *					durations of apointments.
 *
 *					Only called from DayViewDrawTimeBarsCallback.
 *
 * PARAMETERS:  ->	apptIndex:	index of the appointment to draw
 *				->	appts:		table of day appointments
 *				->	tableRP:	Rectangle of the drawing table
 *				->	endTime:	table of maximal time by column
 *				->	table:		table of appointments information
 *				->	endPoint:	table of minimal position by column
 *								(needed to erase bottom of bars)
 *				->	numColumnsP:Pointer on the number of columns appearing
 *								on the left of time.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/3/96		Initial revision.
 *	PLE	8/9/09		Added support for events wrapping around midnight.
 *
 ***********************************************************************/
void DayViewDrawTimeUniqueBar(
	int16_t 		apptIndex,
	ApptInfoPtr 	appts,
	RectangleType* 	tableRP,
	TimePtr 		endTime,
	TablePtr 		table,
	int16_t* 		endPoint,
	int16_t * 		numColumnsP)
{
	Boolean 		drawTheBar = false;
	int16_t 		i, j;
	Boolean			drawTop = false;
	Boolean			drawBottom = false;
	int16_t			x;
	int16_t			y1 = 0;
	int16_t			y2 = 0;
	int16_t			row;
	RectangleType	r;
	int16_t			lineHeight;
	RectangleType	eraseR;

	lineHeight = FntLineHeight ();

	for (i = 0; i < maxTimeBarColumns; i++)
	{
		if (i == 0)
			WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
		else
			WinSetForeColor(UIColorGetTableEntryIndex(UIWarning));

		if (TimeToInt (appts[apptIndex].startTime) >= TimeToInt (endTime[i]))
		{
			endTime[i] = appts[apptIndex].endTime;

			// Find the row that hold the appointment, it may not be
			// visible.
			if (TblFindRowID (table, apptIndex, &row))
			{
				TblGetItemBounds (table, row, descColumn, &r);
				y1 = r.topLeft.y + (lineHeight >> 1);
				drawTop = true;
			}

			// Is the appointment off the top of the display.
			else if (apptIndex < TopVisibleAppt)
			{
				y1 = tableRP->topLeft.y;
				drawTop = false;
			}

			// If the appointment is below the bottom of the display we
			// don't draw anything.
			else
				break;

			// If the start time matches the end time we don't draw anything
			if (TimeToInt (appts[apptIndex].startTime) ==
				TimeToInt (appts[apptIndex].endTime))
				break;

			// Find the row that contains the end time of the appointment.
			for (j = apptIndex + 1; j < sNumAppts; j++)
			{
				// There may be more the one time slot with the time
				// we're searching for, get the last one.
				if (TimeToInt (appts[apptIndex].endTime) <= TimeToInt (appts[j].startTime))
				{
					break;
				}
			}

			// Is the end-time visible.
			if (TblFindRowID (table, j, &row))
			{
				TblGetItemBounds (table, row, descColumn, &r);
				y2 = r.topLeft.y + (lineHeight >> 1);
				drawBottom = true;
			}

			// Is the end of the appointment off the top of the display, if so
			// don't draw anything.
			else if (j < TopVisibleAppt)
			{
				break;
			}
			else
			{
				y2 = tableRP->topLeft.y + tableRP->extent.y - 1;
				drawBottom = false;
			}

			drawTheBar = true;
		}

		if (drawTheBar)
		{
			x = tableRP->topLeft.x + (i * timeBarWidth);

			// Erase the region between the top of the time bar we're
			// about to draw and the bottom of the previous time bar.
			if (y1 > endPoint[i])
			{
				eraseR.topLeft.x = x;
				eraseR.topLeft.y = endPoint[i];
				eraseR.extent.x = timeBarWidth;
				eraseR.extent.y = y1 - endPoint[i];
				//WinEraseRectangle (&eraseR, 0);
			}
			endPoint[i] = y2 + 1;


			// Draw the time bar.
			// Transparency
			//WinEraseLine((Coord)(x + 1), (Coord)(y1 + 1),
			//			(Coord)(x + 1), (Coord)(y2 - 2));

			if (drawTop) y1++;
			if (drawBottom) y2--;

			WinDrawLine (x, y1, x, y2);

			if (drawTop)
				WinDrawLine (x, y1, (Coord)(x + timeBarWidth - 1), y1);

			if (drawBottom)
				WinDrawLine (x, y2, (Coord)(x + timeBarWidth - 1), y2);

			if (i+1 > *numColumnsP)
				*numColumnsP = i+1;

			break;
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawTimeBarsSubCallback
 *
 * DESCRIPTION: 	This routine draw the time bars the indicate the durations
 *              	of apointments.
 *
 * PARAMETERS:  	tableP: the table to draw time bars on.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/3/96		Initial revision.
 *	PLE	8/7/02		Splitted in two routines: created DayViewDrawTimeUniqueBar
 *					that draws a single bar. Added support to event wrapping
 *					around midnight.
 ***********************************************************************/
static void DayViewDrawTimeBarsSubCallback (TablePtr tableP)
{
	int16_t				i;
	int16_t				apptIndex;
	int16_t				numColumns;
	int16_t				endPoint [maxTimeBarColumns];
	TimeType			endTime [maxTimeBarColumns];
	ApptInfoPtr			appts;
	RectangleType		tableR;
	//RectangleType		eraseR;
	IndexedColorType 	initialColor;

	if (! sTimeBarColumns)
		return;

	memset (endTime, 0x00, sizeof (endTime));
	numColumns = 1;

	TblGetBounds (tableP, &tableR);

	for (i = 0; i < maxTimeBarColumns; i++)
	{
		endPoint[i] = tableR.topLeft.y;
	}

	initialColor = WinSetForeColor(0);
	appts = MemHandleLock (sApptsH);

	// First parse events wrapping around midnight
	for (apptIndex = 0; apptIndex < sNumAppts; apptIndex++)
	{
		if (appts[apptIndex].rowID == dbInvalidRowID)
		{
			continue;
		}
		else if (appts[apptIndex].noTimeEvent)
		{
			continue;
		}

		DayViewDrawTimeUniqueBar(apptIndex, appts, &tableR, endTime,
			tableP, endPoint, &numColumns);
	}
	/*
	// Transparency
	// Erase the regions between the botton of the last time bar in
	// each column and the bottom of the table.
	for (i = 0; i < numColumns; i++)
	{
		if (tableR.topLeft.y + tableR.extent.y > endPoint[i])
		{
			eraseR.topLeft.x = tableR.topLeft.x + (i * timeBarWidth);
			eraseR.topLeft.y = endPoint[i];
			eraseR.extent.x = timeBarWidth;
			eraseR.extent.y = tableR.topLeft.y + tableR.extent.y - endPoint[i];
			//WinEraseRectangle (&eraseR, 0);
		}
	}
	*/

	MemHandleUnlock (sApptsH);
	WinSetForeColor(initialColor);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawOverlappingBarsSubCallback()
 *
 * DESCRIPTION: 	This routine draw the horizontal bar needed to
 *					separate overlapping events from other ones.
 *
 * PARAMETERS:  	tableP: the table to draw time bars on.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	12/11/02	Initial revision.
 ***********************************************************************/
static void DayViewDrawOverlappingBarsSubCallback (TablePtr tableP)
{
	ApptInfoPtr		appts;
	uint16_t		apptIndex = 0;
	int16_t			row;
	RectangleType	timeR;
	RectangleType	descR;
	RectangleType	tableR;
	RGBColorType	prevForeColor;

	appts = MemHandleLock (sApptsH);

	TblGetBounds (tableP, &tableR);

	// Save painting color
	WinSetForeColorRGB(&DarkGreyColor, &prevForeColor);

	// Skip untimed events
	while ((apptIndex < sNumAppts) && (appts[apptIndex].noTimeEvent == true))
		apptIndex++;

	// Check if there is some events that began on previous day
	if (apptIndex < sNumAppts && appts[apptIndex].overlapState == overlapScndPart)
	{
		// Skip other overlapping events
		while((apptIndex < sNumAppts) && (appts[apptIndex].overlapState == overlapScndPart))
			apptIndex++;

		// Go back to last displayed events
		apptIndex--;

		// Find the row that holds the appointment, it may not be visible.
		if (TblFindRowID (tableP, apptIndex, &row))
		{
			// Get the coordinates of the bar:
			TblGetItemBounds (tableP, row, timeColumn, &timeR);
			TblGetItemBounds (tableP, row, descColumn, &descR);
			// Draw the bar
			WinDrawLine (timeR.topLeft.x, timeR.topLeft.y + descR.extent.y -1,
				timeR.topLeft.x + descR.extent.x + timeR.extent.x,
				timeR.topLeft.y + descR.extent.y -1);
		}
	}

	// Check for events spanning on next day
	while ((apptIndex < sNumAppts) && (appts[apptIndex].overlapState != overlapFirstPartCommonRow))
		apptIndex++;

	if (apptIndex < sNumAppts)
	{
		// We found the row for events spanning on next day, get the row just before
		if (TblFindRowID (tableP, apptIndex-1, &row))
		{
			// Get the coordinates of the bar:
			TblGetItemBounds (tableP, row, timeColumn, &timeR);
			TblGetItemBounds (tableP, row, descColumn, &descR);
			// Draw the bar
			WinDrawLine (timeR.topLeft.x, timeR.topLeft.y + descR.extent.y -1,
				timeR.topLeft.x + tableR.extent.x + timeR.extent.x,
				timeR.topLeft.y + descR.extent.y -1);
		}
	}

	// Restore painting state
	WinSetForeColorRGB(&prevForeColor, NULL);

	MemHandleUnlock(sApptsH);
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawBarsCallback
 *
 * DESCRIPTION: 	This routine draws vertical bars.
 *
 *					Table Manager Draw callback.
 *
 * PARAMETERS:  -> 	table:	pointer to the memo Day table.
 *              -> 	row:	row of the table to draw.
 *              -> 	column:	column of the table to draw.
 *              -> 	bounds:	region to draw in.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewDrawBarsCallback(
	void * 					table,
	int16_t 				row,
	int16_t 				column,
	RectanglePtr 			bounds)
{
	if (ApptDB && sApptsH)
	{
		DayViewDrawTimeBarsSubCallback(table);
		DayViewDrawOverlappingBarsSubCallback(table);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawTimeCallback
 *
 * DESCRIPTION: 	This routine draws the start time of an appointment.
 *					This routine is called by the table object as a
 *					callback routine.
 *
 *					Table Manager Draw callback.
 *
 * PARAMETERS:  -> 	table:	pointer to the memo Day table.
 *              -> 	row:	row of the table to draw.
 *              -> 	column:	column of the table to draw.
 *              -> 	bounds:	region to draw in.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewDrawTimeCallback(
	void * 						table,
	int16_t 					row,
	int16_t 					column,
	RectanglePtr 				boundsP)
{
	uint16_t 					apptIndex;
	TimeType 					startTime;
	ApptInfoPtr 				appts;
	Boolean						conflictingAppt;
	Boolean						noTimeAppt;
	MidnightOverlapStateType 	overlapState;


	if (ApptDB && sApptsH)
	{
		// Get the appointment index that corresponds to the table item.
		// The index of the appointment in the appointment list, is stored
		// as the row id.
		apptIndex = TblGetRowID (table, row);

		// Get the start time of the appointment.
		appts = MemHandleLock (sApptsH);

		overlapState = appts[apptIndex].overlapState;
		conflictingAppt = ! appts[apptIndex].belongingToCurrCat;
		noTimeAppt = appts[apptIndex].noTimeEvent;

		if (appts[apptIndex].overlapState == overlapScndPart)
			startTime = appts[apptIndex].startTimeInPrevDay;
		else if (appts[apptIndex].overlapState == overlapFirstPartCommonRow)
			startTime = appts[apptIndex].latestEndTimeInNextDay;
		else
			startTime = appts[apptIndex].startTime;

		MemHandleUnlock (sApptsH);

		if ( noTimeAppt)
		{
			DrawUntimedEventsIcon(
				row,
				boundsP,
				NoTimeDiamondBitmap,
				HighlightedNoTimeDiamondBitmap,
				DisabledNoTimeDiamondBitmap,
				HighlightDisabledNoTimeDiamondBitmap,
				conflictingAppt,
				overlapState);
		}
		else
		{
			DrawTime (startTime, overlapState, noTimeAppt, conflictingAppt,
				TimeFormat, apptTimeFont, rightAlign, boundsP);
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawEventIcon
 *
 * DESCRIPTION: 	This routine draw the event details icon asked in its
 *					current state.
 *
 *					called by DayViewDrawIconsAndOverlappedEvtsCallback
 *					a Table Manager Callback.
 *
 * PARAMETERS:  ->	row:						row index in table
 *				->	descLineHeight:				line height
 *				->	x:							Horizontal coordinate
 *				->	boundsP:					rectangle of cell
 *				->	iconRank:					icon rank
 *				->	normalIconID:				normal state
 *				->	highlightedNormalID:		normal highligthed state
 *				->	disabledIconID:				disbaled state
 *				->	highlightedDisabledIconID: 	disbaled highligthed state
 *				->	belongingToCurrCat:			current categoy
 *				->	overState:					overlap event
 *
 * RETURNED:    	The size to move horizontal coordinates.
 *
 * NOTE:			Graphics state and color are set by
 *					DayViewDrawIconsAndOverlappedEvtsCallback.
 *
 * REVISION HISTORY:
 * 	PPL	07/06/04	Initial Revision.
 * 	PPL	07/06/04	Code Factorization
 *					from DayViewDrawIconsAndOverlappedEvtsCallback.
 *
 ***********************************************************************/
static int16_t DayViewDrawEventIcon(
	int16_t						row,
	int16_t						descLineHeight,
	Coord						x,
	RectangleType*				boundsP,
	uint16_t					iconRank,
	uint16_t 					normalIconID,
	uint16_t 					highlightedNormalID,
	uint16_t 					disabledIconID,
	uint16_t 					highlightedDisabledIconID,
	Boolean 					belongingToCurrCat,
	MidnightOverlapStateType 	overState)
{
	MemHandle					resH;
	BitmapType*					wrappingIconBmpP;
	Coord						bmpHeight;
	Coord						bmpWidth;
	Coord						y;
	Coord						dy;
	uint16_t					iconID;


	if (!belongingToCurrCat || overState == overlapScndPart)
	{
		// the row does not belong to the current Category of dayview
		// we use the shaded time zone icon.

		// normal: 			ShadedTimeZoneIconBitmap
		// hightlighted: 	HighlightedShadedTimeZoneIconBitmap
		iconID = (row == sRow && sIconSelected && sIconHit == iconRank)
					?highlightedDisabledIconID : disabledIconID;
	}
	else
	{
		// the row  belongs to the current Category of dayview
		// we use the time zone icon.

		// normal: 			TimeZoneIconBitmap
		// hightlighted: 	HigthlightedTimeZoneBitmap
		iconID = (row == sRow && sIconSelected && sIconHit == iconRank)
					? highlightedNormalID: normalIconID ;
	}

	// Load standard bitmap
	resH = DmGetResource(gApplicationDbP, bitmapRsc, iconID);

	wrappingIconBmpP = (BitmapType*) MemHandleLock(resH);

	BmpGetDimensions (wrappingIconBmpP, &bmpWidth, &bmpHeight, NULL);

	dy = descLineHeight - bmpHeight;

	if (dy < 0)
		dy = 0;

	y = boundsP->topLeft.y + dy;

	WinDrawBitmap(wrappingIconBmpP, x, y);

	MemHandleUnlock(resH);
	DmReleaseResource(resH);

	return ((int16_t)GetBitmapWidth(iconID));
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawIconsAndOverlappedEvtsCallback
 *
 * DESCRIPTION: 	This routine draws the note, alarm and repeat icons.
 *              	It is called by the table object as a callback
 *              	routine.
 *
 *					Table Manager Draw callback.
 *
 * PARAMETERS:  ->	table:	pointer to the memo Day table.
 *              ->	row:	row of the table to draw.
 *              ->	column:	column of the table to draw.
 *              ->	bounds:	region to draw in.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/16/96		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 * 	PPL 06/03/04	Bitmap icons are replacing symbol chars.
 *
 ***********************************************************************/
static void DayViewDrawIconsAndOverlappedEvtsCallback (
	void * 						tableP,
	int16_t 					row,
	int16_t 					column,
	RectangleType*				boundsP)
{
	uint16_t					apptIndex;
	uint32_t 					rowID;
	Coord						x;
	FontID						curFont = stdFont;
	ApptInfoPtr 				appts;
	MidnightOverlapStateType 	overState;
	RGBColorType				prevTextColor;
	RGBColorType				prevBackColor;
	RGBColorType*				textColorToUseP = NULL;
	RGBColorType*				backColorToUseP = NULL;
	RGBColorType*				textHighlightColorToUseP = NULL;
	RGBColorType*				backHighlightColorToUseP = NULL;
	Coord						iconsWidth;
	MemHandle					overlapStrH;
	char *						overlapStrP;
	MemHandle					descH;
	char *						descP;
	Boolean 					drawIcons = false;
	Boolean 					belongingToCurrCat;
	Coord						descLineHeight;
	WinDrawOperation			curDrawMode;

	if (ApptDB && sApptsH)
	{
		WinPushDrawState();

		// Get the appointment index that corresponds to the table item.
		// The index of the appointment in the appointment list, is stored
		// as the row id.
		apptIndex = TblGetRowID (tableP, row);

		// Get the start time of the appointment.
		appts = MemHandleLock (sApptsH);
		rowID = appts[apptIndex].rowID;
		overState = appts[apptIndex].overlapState;
		belongingToCurrCat = appts[apptIndex].belongingToCurrCat;
		MemHandleUnlock (sApptsH);

		curFont	= FntSetFont (ApptDescFont);
		descLineHeight = FntLineHeight();

		// NB: Erase rectangle (paint with sObjectFillColor)
		// this is done, as now, table is erasing after having set
		// the colors after the selected state.
		// this is to avoid to have the background paint with
		// the selection color (sObjectSelectedFillColor)

		WinSetTextColorRGB(NULL, &prevTextColor);
		WinSetBackColorRGB(NULL, &prevBackColor);

		if (!belongingToCurrCat || overState == overlapScndPart)
		{
			// Save painting color and font
			WinSetTextColorRGB(&MediumGreyColor, NULL);

			// Get the text to display in row
			descH = MemHandleNew(32);  // 32 is an initial guess, it will be resized if needed
			ApptGetDescription(ApptDB, rowID, descH);
			descP = MemHandleLock(descH);
			DateUtilsStripNonPrintableChar(descP);

			curDrawMode =  WinSetDrawMode(winOverlay);

			WinPaintTruncChars (
				descP,
				(int16_t) strlen(descP),
				boundsP->topLeft.x,
				boundsP->topLeft.y,
				boundsP->extent.x - TblGetItemInt (tableP, row, column));

			//WinSetDrawMode(curDrawMode);

			MemHandleUnlock(descH);
			MemHandleFree(descH);
			descH = NULL;

			// Draw the icons after the description
			drawIcons = true;

			// prepare colors (highlighted or not)
			textColorToUseP 			= &MediumGreyColor;
			backColorToUseP 			= &prevBackColor;
			textHighlightColorToUseP 	= &MediumGreyColor;
			backHighlightColorToUseP 	= &sObjectSelectedFillColor;

		}
		else if (overState == overlapFirstPartCommonRow)
		{
			// Get the localized text to print
			overlapStrH = DmGetResource (gApplicationDbP, strRsc, dayViewOverlapStrID);
			overlapStrP = MemHandleLock(overlapStrH);

			// Save painting color and font
			WinSetTextColorRGB(&MediumGreyColor, NULL);

			curDrawMode =  WinSetDrawMode(winOverlay);

			WinPaintTruncChars (
				overlapStrP,
				(int16_t) strlen(overlapStrP),
				boundsP->topLeft.x,
				boundsP->topLeft.y,
				boundsP->extent.x - TblGetItemInt (tableP, row, column));

			//WinSetDrawMode(curDrawMode);

			// Release the resources
			MemHandleUnlock(overlapStrH);
			DmReleaseResource(overlapStrH);
		}
		else
		{
			// Draw the icons after the description
			drawIcons = true;

			// prepare colors (highlighted or not)
			textColorToUseP 			= &sObjectInkColor;


			backColorToUseP 			= &sBackgroundHighlight;

			textHighlightColorToUseP 	= &sObjectSelectedInkColor;


			backHighlightColorToUseP	= &sBackgroundHighlight;
		}

		if (drawIcons)
		{
			iconsWidth = TblGetItemInt (tableP, row, column);

			x = boundsP->topLeft.x + boundsP->extent.x - iconsWidth;

			WinSetTextColorRGB(&sObjectInkColor, NULL);

			if (ApptEventHasNote(ApptDB, rowID))
			{
				x +=  DayViewDrawEventIcon(
						row,
						descLineHeight,
						x,
						boundsP,
						kDayViewNoteIcon,
						NoteIconBitmap,
						HighlightedNoteIconBitmap,
						DisabledNoteIconBitmap,
						HighlightDisabledNoteIconBitmap,
						belongingToCurrCat,
						overState);
			}

			if (ApptEventAlarmIsSet(ApptDB, rowID))
			{
				x +=  DayViewDrawEventIcon(
						row,
						descLineHeight,
						x,
						boundsP,
						kDayViewAlarmIcon,
						AlarmIconBitmap,
						HighlightedAlarmIconBitmap,
						DisabledAlarmIconBitmap,
						HighlightDisabledAlarmIconBitmap,
						belongingToCurrCat,
						overState);
			}

			if (ApptEventRepeatIsSet(ApptDB, rowID))
			{
				x +=  DayViewDrawEventIcon(
						row,
						descLineHeight,
						x,
						boundsP,
						kDayViewRepeatIcon,
						RepeatIconBitmap,
						HighlightedRepeatIconBitmap,
						DisabledRepeatIconBitmap,
						HighlightDisabledRepeatIconBitmap,
						belongingToCurrCat,
						overState);
			}

			if (ApptEventTZDiffersFromSystemTZ(ApptDB, rowID))
			{
				x +=  DayViewDrawEventIcon(
						row,
						descLineHeight,
						x,
						boundsP,
						kDayViewTimeZoneIcon,
						TimeZoneIconBitmap,
						HighlightedTimeZoneIconBitmap,
						DisabledTimeZoneIconBitmap,
						HighlightDisabledTimeZoneIconBitmap,
						belongingToCurrCat,
						overState);
			}
		}

		// Restore  color & font
		WinSetTextColorRGB(&prevTextColor, NULL);
		WinSetBackColorRGB(&prevBackColor, NULL);

		FntSetFont (curFont);

		WinPopDrawState();

		// Draw the vertical bar after everything else.
		DayViewDrawVerticalBar(tableP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewCheckForConflicts
 *
 * DESCRIPTION: 	This routine check the apointment list for conflicts
 *              	(overlapping appointmens).
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	The number of columns of time bars necessary to display
 *             	 	the conflicts.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/3/96		Initial revision.
 *
 ***********************************************************************/
static uint16_t DayViewCheckForConflicts (void)
{
	uint16_t			i;
	uint16_t			numColumns;
	uint16_t			apptIndex;
	uint16_t			width;
	TablePtr			table;
	TimeType			endTime [maxTimeBarColumns];
	ApptInfoPtr			appts;
	RectangleType		tableR;
	TimeType			apptStartTime;
	TimeType			apptEndTime;
	FormPtr 			frmP;
	Boolean				existingOverlappingAppt = false;

	memset (endTime, 0x00, sizeof (endTime));

	numColumns = 1;

	appts = MemHandleLock (sApptsH);
	for (apptIndex = 0; apptIndex < sNumAppts; apptIndex++)
	{
		apptStartTime = appts[apptIndex].startTime;
		apptEndTime = appts[apptIndex].endTime;

		if (appts[apptIndex].rowID == dbInvalidRowID)
			continue;

		else if (appts[apptIndex].noTimeEvent)
			continue;

		if (appts[apptIndex].overlapState == overlapFirstPart)
		{
			TimeToInt(apptEndTime) = overlapEndTime;
			existingOverlappingAppt = true;
		}
		else if (appts[apptIndex].overlapState == overlapScndPart)
		{
			TimeToInt(apptStartTime) = overlapStartTime;
			existingOverlappingAppt = true;
		}

		for (i = 0; i < maxTimeBarColumns; i++)
		{
			if (TimeToInt (apptStartTime) >= TimeToInt (endTime[i]))
			{
				endTime[i] = apptEndTime;
				if (i+1 > numColumns)
					numColumns = i+1;
				break;
			}
		}
	}

	MemPtrUnlock (appts);


	// Reserve space for the time bars.  We will show time bars if the user
	// has requested them, if there are conflicting appointments or appts
	// overlapping on next day.
	if (numColumns == 1 && (! ShowTimeBars) && (!existingOverlappingAppt))
		numColumns = 0;

	if (sTimeBarColumns != numColumns)
	{
		frmP = FrmGetFormPtr (DayView);

		sTimeBarColumns = numColumns;
		table = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));

		// Set the width of the time bar table column.
		TblSetColumnWidth (table, timeBarColumn, (Coord)(numColumns * timeBarWidth));

		// Adjust the width of the description column.
		TblGetBounds (table, &tableR);
		width = tableR.extent.x -
				  (numColumns * timeBarWidth) -
 				  TblGetColumnWidth (table, timeColumn) -
 				  TblGetColumnSpacing (table, timeColumn);
 		TblSetColumnWidth (table, descColumn, width);
	}

	return (numColumns);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawTimeBars
 *
 * DESCRIPTION: 	This routine draw the time bars the indicate the durations
 *              	of apointments.
 *
 * PARAMETERS:  	formP: form where to get the table to draw time bars on.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/3/96		Initial revision.
 *	PLE	8/7/02		Splitted in two routines: created DayViewDrawTimeUniqueBar
 *					that draws a single bar. Added support to event wrapping
 *					around midnight.
 ***********************************************************************/
static void DayViewDrawTimeBars (FormPtr formP)
{
	TablePtr	tableP;


	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));

	DayViewDrawTimeBarsSubCallback(tableP);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawVerticalBarAdapter()
 *
 * DESCRIPTION: 	Used to adapt the DayViewUpdateDisplay expected api.
 *
 * PARAMETERS:  	formP: form to get the table to draw the bar..
 *
 * RETURNED:    	Nothing.
 *
 ***********************************************************************/
static void DayViewDrawVerticalBarAdapter(FormPtr formP)
{
	TablePtr tableP;
	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));
	DayViewDrawVerticalBar(tableP);
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawVerticalBar()
 *
 * DESCRIPTION: 	This routine draws the vertical bar at the left
 *					left margin of the appointment field..
 *
 * PARAMETERS:  	tableP: table in which the line is drawn.
 *
 * RETURNED:    	Nothing.
 *
 ***********************************************************************/
static void DayViewDrawVerticalBar(TablePtr tableP)
{
	RectangleType rect;
	int16_t lastRow;
	PointType top;
	PointType bottom;
	RGBColorType oldColor;

	// Set the line height
	//TblGetBounds(tableP, &rect);
	TblGetItemBounds(tableP, 0, descColumn, &rect);
	top.y = rect.topLeft.y;
	lastRow = TblGetLastUsableRow(tableP);
	TblGetItemBounds(tableP, lastRow, descColumn, &rect);
	bottom.y = rect.topLeft.y + rect.extent.y;

	// Set the leftmost position of the column. We shift it slightly below.
	top.x = rect.topLeft.x;
	bottom.x = top.x;

	// Now switch to native coordinates to draw the single pixel line.
	WinSetCoordinateSystem(kCoordinatesNative);
	WinScalePoint(&top, false);
	WinScalePoint(&bottom, false);

	// The line shift left must occur here, as opposed to before the pts are scaled
	// in order to avoid the  turncation that would cause the result to line up with standard
	// coord again.
	top.x -= 1;
	bottom.x = top.x;

	WinSetForeColorRGB(&sBackgroundHighlight, &oldColor);
	WinDrawLine(top.x, top.y, bottom.x, bottom.y);
	WinSetForeColorRGB(&oldColor, NULL);
	WinSetCoordinateSystem(kCoordinatesStandard);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawOverlappingBars()
 *
 * DESCRIPTION: 	This routine draw the horizontal bar needed to
 *					separate overlapping events from other ones.
 *
 * PARAMETERS:  	formP: form where to get the table to draw time bars on.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	12/11/02	Initial revision.
 ***********************************************************************/
static void DayViewDrawOverlappingBars (FormPtr formP)
{
	TablePtr tableP;

	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));

	DayViewDrawOverlappingBarsSubCallback(tableP);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewInsertAppointment
 *
 * DESCRIPTION: 	This routine inserts the record index of a new record
 *              	into the structure that keeps track of the appointment
 *              	on the current day.
 *
 * PARAMETERS:  ->	apptIndex:
 *              ->	recordNum:	appointment record index.
 *              ->	row:		row in the table object of the new record.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/28/95		Initial revision.
 *
 ***********************************************************************/
static void DayViewInsertAppointment (uint16_t apptIndex, uint32_t apptRowID, uint16_t row)
{
	FormPtr 	frmP;
	TablePtr 	tableP;
	ApptInfoPtr appts;

	// Be sure that it is a rowID and not a cursorID
	ErrNonFatalDisplayIf(! DbIsRowID(apptRowID), "Insert only row IDs in table !");

	appts = MemHandleLock (sApptsH);
	appts[apptIndex].rowID = apptRowID;
	MemHandleUnlock (sApptsH);

	// Store the unique id of the record in the row.
	frmP = FrmGetFormPtr (DayView);
	tableP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));
	TblSetRowData (tableP, row, apptRowID);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewGetDescriptionCallback
 *
 * DESCRIPTION: 	This routine returns a pointer to the description field
 *              	of a appointment record.  This routine is called by
 *              	the table object as a callback routine when it wants to
 *              	display or edit the appointment's description.
 *
 *					Table Manager load Callback.
 *
 * PARAMETERS:  ->	table:			Pointer to the Day View table.
 *              ->	row:			Row in the table.
 *              ->	column:			Column in the table.
 *              ->	editable:		True if the field will be edited
 *									by the table.
 *              ->	textOffset:		Offset within the record of the desc
 *									field (returned.)
 *              ->	textAllocSize:	Allocated size the the description
 *									field (returned.)
 *
 * RETURNED:    	Handle of the appointment record.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static status_t DayViewGetDescriptionCallback (
	void * 				table,
	int16_t 			row,
	int16_t 			column,
	Boolean 			editable,
	MemHandle * 		textH,
	int16_t * 			textOffsetP,
	int16_t * 			textAllocSizeP,
	FieldPtr 			fldP)
{
	status_t 			error = errNone;
	uint16_t 			apptIndex;
	uint32_t			apptRowID;
	FieldAttrType 		attributes;
	ApptInfoPtr 		appts;
	ApptDBRecordType 	apptRec;
	ApptDateTimeType 	when;
	MemHandle			descH;

	if (ApptDB && sApptsH)
	{
		// Initializations
		*textOffsetP = 0;
		*textAllocSizeP = 0;
		*textH = NULL;

		// Get the appointment that corresponds to the table item.
		apptRowID = TblGetRowData (table, row);

		if (apptRowID == dbInvalidRowID)
		{
			// Get index in appt table
			apptIndex = TblGetRowID (table, row);

			// If we're drawing the description, return a null MemHandle.
			if  (! editable)
				return errNone;

			// If we have reached the maximum number of displayable event, then
			// exit returning an error.
			if (sNumApptsOnly >= apptMaxPerDay)
				return ((status_t) (-1));

			// If we're editing the description, create a new record.
			memset (&apptRec, 0x00, sizeof (apptRec));

			appts = MemHandleLock (sApptsH);
			when.startTime = appts[apptIndex].startTime;

			// If the start time is before 11:00 pm, the end time is one hour
			// after the start time.
			if (when.startTime.hours < maxHours)
			{
				when.endTime.hours = when.startTime.hours + 1;
				when.endTime.minutes = when.startTime.minutes;
			}

			// If the start time is 11:00 pm or later, the end time is 11:55 pm.
			else
			{
				when.endTime.hours = maxHours;
				when.endTime.minutes = maxMinutes;
			}

				// Don't let the new appointment overlap the next appointment.
			if (((apptIndex+1) < sNumAppts) &&
				 (TimeToInt(when.endTime) > TimeToInt(appts[apptIndex+1].startTime)))
			{
				when.endTime = appts[apptIndex+1].startTime;
				appts[apptIndex].endTime = when.endTime;
			}


			MemHandleUnlock (sApptsH);
			// Start & end date set to current
			when.date = Date;
			when.midnightWrapped = false;
			when.noTimeEvent = false;
			strncpy(when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);
			apptRec.when = when;

			// Make sure the record has a description field so that we have
			// something to edit - Dynamically allocate due to ApptFreeRecordMemory()
			apptRec.description = MemPtrNew(1);
			apptRec.description[0] = chrNull;

			if (AlarmPreset.advance != apptNoAlarm)
				apptRec.alarm = AlarmPreset;
			else
				apptRec.alarm.advance = apptNoAlarm;

			error = ApptNewRecord (ApptDB, &apptRec, &apptRowID);

			if (error < errNone)
			{
				FrmUIAlert(DeviceFullAlert);
				ApptFreeRecordMemory (&apptRec);
				return (error);
			}

			// Rebuilds the cursor (needed for alarms)
			ApptDBRequery(ApptDB, gApptCursorID, false);

			// Assign the current category to the event, if the current
			// category displayed is all, let the new record as unfiled (default value)
			if (!DatebookAllCategorySelected())
				DbSetCategory(ApptDB, apptRowID, DateBkCurrentCategoriesCount, DateBkCurrentCategoriesP);

			DayViewInsertAppointment (apptIndex, apptRowID, row);

			// If the alarm preset preference is set we needed to reschedule the alarms
			// We don't redraw the row here, we'll do that on the tblSelect event.
			if (AlarmPreset.advance != apptNoAlarm)
			{
				uint32_t ref;
				uint32_t trigger;
				uint32_t newAlarm;

				// If the new event's alarm will sound between now and the currently
				// registered alarm, the new one must be registered
				trigger = AlmGetAlarm (gApplicationDbID, &ref);
				newAlarm = ApptGetAlarmTime (&apptRec, TimGetSeconds (), true);

				if (newAlarm && ((newAlarm < trigger) || (trigger == 0)))
					RescheduleAlarmsAllRows();
			}

			// Release the record
			ApptFreeRecordMemory (&apptRec);
		}

		if (editable)
			descH = sDescriptionEditedHolderH;
		else
			descH = sDescriptionNotEditedHolderH;

		// set the Shift field of the field

		// Update the field
		ApptGetDescription(ApptDB, apptRowID, descH);

		// Set the maximum length
		FldSetMaxChars(fldP, sDescriptionFieldMaxLength);

		// Set the field to support auto-shift.
		FldGetAttributes (fldP, &attributes);
		attributes.autoShift = true;
		FldSetAttributes (fldP, &attributes);

		*textAllocSizeP =(int16_t) (MemHandleSize(descH) - sCurrentFieldOffset);
		*textOffsetP = (int16_t) sCurrentFieldOffset;
		*textH = descH;
	}

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewSaveDescriptionCallback
 *
 * DESCRIPTION: 	This routine saves the description field of a
 *					appointment to its db record.  This routine is called
 *					by the table object, as a callback routine, when it
 *					wants to save the description.
 *
 *					Table Manager Save callback.
 *
 * PARAMETERS:  ->	table:	pointer to the memo Day table.
 *             	-> 	row:	row of the table to draw.
 *              ->	column:	column of the table to draw.
 *
 * RETURNED:    	True if the table needs to be redrawn.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL 10/15/03	for Cursor Cahing uses gApptCursorID.
 *					save restore current rowID when needed.
 *					(it is required as this function is a callback
 *					given to the table manager.)
 *
 ***********************************************************************/
static Boolean DayViewSaveDescriptionCallback (void * table, int16_t row, int16_t column)
{
	uint32_t 	scrollPos;
	FieldPtr 	fldP;
	uint32_t 	apptRowID = dbInvalidRowID;
	uint32_t	currentRowID = dbInvalidRowID;
	status_t	err;

	if (ApptDB && sApptsH)
	{
	 	// If the description has been modified mark the record dirty
		fldP = TblGetCurrentField (table);

		// Check if the top of the description is scroll off the top of the
		// field, if it is then redraw the field.
		scrollPos = FldGetScrollPosition (fldP);
		if (scrollPos)
			FldSetScrollPosition (fldP, 0);

		// Set position and selection to get it restored by DayViewRestoreEditState
		FldGetSelection(fldP, (size_t*)&DayEditPosition, (size_t*)&DayEditSelectionLength);
		DayEditSelectionLength -= DayEditPosition;

		RecordDirty = FldDirty (fldP);
		if (RecordDirty)
		{
			// Save current cursor Row
			err = DbCursorGetCurrentRowID(gApptCursorID, &currentRowID);

			// Get the appointment that corresponds to the table item.
			apptRowID = TblGetRowData (table, row);

			// Exit on invalid handle
			if (apptRowID == dbInvalidRowID)
				return (false);

			if (currentRowID != apptRowID)
				err = DbCursorMoveToRowID(gApptCursorID, apptRowID);

			// Update DB
			ApptChangeDescription(ApptDB, gApptCursorID, sDescriptionEditedHolderH);

			// restore row
			if (currentRowID != dbInvalidRowID && currentRowID != apptRowID)
				DbCursorMoveToRowID(gApptCursorID, currentRowID);
		}

	 }

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewGetDescriptionHeight
 *
 * DESCRIPTION: 	This routine returns the height, in pixels, of the
 *              	description field of an appointment record.
 *
 * PARAMETERS:  ->	apptIndex:		index in appointment list.
 *              ->	width:			width of the description column.
 * 				->	maxHeight:		the maximum height of the field.
 *              ->	iconsWidthP:	space ot reserve for note, alarm and
 *                            		repeat icons.
 *              ->	fontIdP:		font id to draw the text with.
 *
 * RETURNED:    	Height in pixels.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 * 	PPL 06/03/04	Bitmap icons are replacing symbol chars.
 *
 ***********************************************************************/
static int32_t DayViewGetDescriptionHeight (
	uint16_t 					apptIndex,
	int32_t 					width,
	int32_t 					maxHeight,
	int32_t * 					iconsWidthP,
	FontID * 					fontIdP)
{
	int32_t 					height;
	int32_t 					iconsWidth;
	uint32_t 					apptRowID;
	int32_t 					lineHeight;
	FontID 						curFont;
	ApptInfoPtr					appts;
	ApptDBRecordType 			apptRec;
	MidnightOverlapStateType 	overlapState;
	Boolean 					conflictingEvt;

	// Get the record index.
	appts = MemHandleLock (sApptsH);
	apptRowID = appts[apptIndex].rowID;
	overlapState = appts[apptIndex].overlapState;
	conflictingEvt = (Boolean) !appts[apptIndex].belongingToCurrCat;
	MemPtrUnlock (appts);

	iconsWidth = 0;

	// Empty time slot?
	if (overlapState == overlapFirstPartCommonRow)
	{
		curFont = FntSetFont (ApptDescFont);
		height = FntLineHeight ();
		FntSetFont (curFont);
		*fontIdP = ApptDescFont;
	}
	else if (apptRowID == dbInvalidRowID)
	{
		curFont = FntSetFont (apptTimeFont);
		height = FntLineHeight ();
		FntSetFont (curFont);
		*fontIdP = apptTimeFont;

		/*
		curFont = FntSetFont (ApptDescFont);
		height = FntLineHeight ();
		FntSetFont (curFont);
		*fontIdP = ApptDescFont;
		*/
	}
	else
	{
		// Compute the width needed to draw the note, alarm and repeat icons.
		//curFont = FntSetFont (symbolFont);

		if (ApptEventHasNote(ApptDB, apptRowID))
		{
			iconsWidth += GetBitmapWidth(NoteIconBitmap);
		}

		if (ApptEventAlarmIsSet(ApptDB, apptRowID))
		{
			iconsWidth += GetBitmapWidth(AlarmIconBitmap);
		}

		if (ApptEventRepeatIsSet(ApptDB, apptRowID))
		{
			iconsWidth += GetBitmapWidth(RepeatIconBitmap);
		}

		if (ApptEventTZDiffersFromSystemTZ(ApptDB, apptRowID))
		{
			iconsWidth += GetBitmapWidth(TimeZoneIconBitmap);
		}

		// Keep 1 pixel from the right border.
		if (iconsWidth)
			iconsWidth += 1;

		// Get the appointment record.
		ApptGetRecord (ApptDB, apptRowID, &apptRec, DBK_SELECT_DESCRIPTION);

		// Compute the height of the appointment description.
		curFont = FntSetFont (ApptDescFont);

		lineHeight = FntLineHeight ();
		if ((overlapState == overlapScndPart) || conflictingEvt)
			height = lineHeight;
		else
		{
			height = FldCalcFieldHeight (apptRec.description, (Coord)(width - iconsWidth));
			height = min (height, (maxHeight / lineHeight));
			height *= lineHeight;
		}
		*fontIdP = ApptDescFont;

		FntSetFont (curFont);

		ApptFreeRecordMemory (&apptRec);
	}

	*iconsWidthP = iconsWidth;

	return (height);
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewCanScrollScrollers
 *
 * DESCRIPTION: 	this routine returns booleans to know if we can
 *					scroll up or down.
 *
 * PARAMETERS:  	Nothing.
 *
 * RETURNED:    	Nothing.
 *
 * Note:			None.
 *
 * REVISION HISTORY:
 *	PPL	08/10/04	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewCanScrollScrollers (WinDirectionType direction)
{
	switch(direction)
	{
		case winUp:
			return sDayViewscrollableUp;

		case winDown:
			return sDayViewscrollableDown;
	}
	return false;
}



/***********************************************************************
 *
 * FUNCTION:    	DayViewUpdateScrollers
 *
 * DESCRIPTION: 	This routine draws or erases the Day View scroll
 *					arrow buttons.
 *
 * PARAMETERS:  ->	frm:				pointer to the to do Day form.
 *              ->	bottomAppt:			record index of the last visible
 *										record.
 *              ->	lastItemClipped:	true if the list is partially
 *										off the display.
 *
 * RETURNED:    	Nothing.
 *
 * Note:			None.
 *
 * REVISION HISTORY:
 *	ART	7/28/95		Initial revision.
 *
 ***********************************************************************/
static void DayViewUpdateScrollers (
	FormPtr 	frm,
	int32_t 	bottomAppt,
	Boolean 	lastItemClipped)
{
	uint16_t 	upIndex;
	uint16_t 	downIndex;

	// If the first appointment displayed is not the fist appointment
	// of the day, enable the up scroller.
	sDayViewscrollableUp = (Boolean) ((TopVisibleAppt > 0) || (sCurrentFieldOffset > 0));

	// If the last appointment displayed is not the last appointment
	// of the day or if it partially clipped, enable the down scroller.
	sDayViewscrollableDown = (Boolean) ( lastItemClipped || (bottomAppt+1 < sNumAppts) );

	// Update the scroll button.
	upIndex = FrmGetObjectIndex (frm, DayUpButton);
	downIndex = FrmGetObjectIndex (frm, DayDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, sDayViewscrollableUp, sDayViewscrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewInitRow
 *
 * DESCRIPTION: 	This routine initializes a row in the Day View table.
 *
 * PARAMETERS:  ->	table:		pointer to the table of appointments
 *              ->	row:		row number (first row is zero)
 *              ->	apptIndex:	index in appointment list
 *              ->	rowHeight:	height of the row in pixels
 *              ->	uniqueID:	unique ID of appointment record
 *              ->	iconsWidth:	space to reserve for note, alarm and repeat
 *                           	and time zone icons.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewInitRow (
	TablePtr 					table,
	int16_t 					row,
	uint16_t 					apptIndex,
	int32_t 					rowHeight,
	uint32_t 					apptRowID,
	int32_t 					iconsWidth,
	FontID 						fontID)
{
	uint16_t 					time;
	ApptInfoPtr					appts;
	MidnightOverlapStateType 	overState;
	Boolean 					belongingToCurrCat;

	// Make the row usable.
	TblSetRowUsable (table, row, true);

	// Set the height of the row to the height of the description.
	TblSetRowHeight (table, row, (Coord) rowHeight);

	// Store the record number as the row id.
	TblSetRowID (table, row, apptIndex);

	// Store the start time of the appointment in the table.
	appts = MemHandleLock(sApptsH);

	time = TimeToInt(appts[apptIndex].startTime);
	overState = appts[apptIndex].overlapState;
	belongingToCurrCat = appts[apptIndex].belongingToCurrCat;

	MemHandleUnlock(sApptsH);

	TblSetItemInt (table, row, timeColumn, time);

	// Store the unique id of the record in the row, excepted for
	// second part of overlapping events which can't have focus
	if (overState != overlapScndPart)
		TblSetRowData (table, row, apptRowID);
	else
		TblSetRowData (table, row, 0);

	// Set the table item type for the description,  it will differ depending
	// on the presence of a note.
	if (!belongingToCurrCat || overState == overlapFirstPartCommonRow || overState == overlapScndPart)
	{
		TblSetItemStyle (table, row, descColumn, customTableItem);
		TblSetItemInt (table, row, descColumn, (int16_t) iconsWidth);
	}
	else if (! iconsWidth)
	{
		TblSetItemStyle (table, row, descColumn, textTableItem);
		TblSetItemInt (table, row, descColumn, 0);
	}
	else
	{
		TblSetItemStyle (table, row, descColumn, narrowTextTableItem);
		TblSetItemInt (table, row, descColumn, (int16_t) iconsWidth);
	}

	// Set the font used to draw the text of the row.
	TblSetItemFont (table, row, descColumn, fontID);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewLoadTable
 *
 * DESCRIPTION: 	This routine reloads appointment database records
 *					into the Day view.
 *
 *					This routine is called when:
 *              		o A new item is inserted
 *              		o An item is deleted
 *              		o The time of an items is changed
 *
 * PARAMETERS:  ->	frm: day view form.
 *
 * RETURNED:    	Nothing.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewLoadTable (FormPtr frm)
{
	uint16_t 				apptIndex;
	uint16_t 				lastAppt;
	int16_t 				row;
	int32_t 				numRows;
	uint32_t 				apptRowID;
	uint32_t 				currentRowIDInCursor = dbInvalidRowID;
	int32_t 				iconsWidth;
	int32_t 				lineHeight;
	int32_t					dataHeight;
	int32_t 				tableHeight;
	int32_t 				columnWidth;
	int32_t 				pos;
	int32_t					oldPos;
	int32_t 				height;
	int32_t					oldHeight;
	FontID 					fontID;
	FontID 					currFont;
	TablePtr 				table;
	Boolean 				rowUsable;
	Boolean 				lastItemClipped;
	ApptInfoPtr				appts;
	RectangleType 			r;
	uint16_t 				attr;
	Boolean 				masked;
	privateRecordViewEnum 	visualStatus;

	appts = MemHandleLock (sApptsH);

	// Get the height of the table and the width of the description
	// column.
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DayTable));
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (table, descColumn);

	// If we currently have a selected record, make sure that it is not
	// above the first visible record.
	if (gItemSelected)
	{
		apptIndex = DayViewFindAppointment (gApptCursorID, appts);
		if (apptIndex < TopVisibleAppt)
			TopVisibleAppt = apptIndex;
	}

	if (TopVisibleAppt >= sNumAppts)
		TopVisibleAppt = sNumAppts - 1;

	apptIndex = TopVisibleAppt;
	lastAppt = apptIndex;

	// Load records into the table.
	row = 0;
	dataHeight = 0;
	oldPos = pos = 0;

	while (apptIndex < sNumAppts)
	{
		// Compute the height of the appointment's description.
		height = DayViewGetDescriptionHeight (apptIndex, columnWidth, tableHeight, &iconsWidth, &fontID);

		// Is there enought room for at least one line of the the decription.
		currFont = FntSetFont (fontID);

		lineHeight = FntLineHeight ();

		FntSetFont (currFont);


		if (tableHeight >= dataHeight + lineHeight)
		{
			// Get the height of the current row.
			rowUsable = TblRowUsable (table, row);

			if (rowUsable)
				oldHeight = TblGetRowHeight (table, row);
			else
				oldHeight = 0;

			masked = false;
			apptRowID = appts[apptIndex].rowID;
			if (apptRowID != dbInvalidRowID)	// empty time slot?
			{
				// Mask if appropriate
				DbCursorGetCurrentRowID(gApptCursorID, &currentRowIDInCursor);

				visualStatus = apptRowID == currentRowIDInCursor
					? CurrentRecordVisualStatus : PrivateRecordVisualStatus;

				DbGetRowAttr (ApptDB, apptRowID, &attr);
		   		masked = (Boolean) (((attr & dmRecAttrSecret) && visualStatus == maskPrivateRecords));
			}

			TblSetRowMasked(table,row,masked);

			// Init the row
			DayViewInitRow (table, row, apptIndex, height, apptRowID, iconsWidth, fontID);

			pos += height;
			oldPos += oldHeight;

			lastAppt = apptIndex;
			apptIndex++;
			row++;
		}

		dataHeight += height;

		// Is the table full?
		if (dataHeight >= tableHeight)
		{
			// If we have a currently selected record, make sure that it is
			// not below the last visible record.  If the currently selected
			// record is the last visible record, make sure the whole description
			// is visible.
			if (! gItemSelected)
				break;

			apptIndex = DayViewFindAppointment (gApptCursorID, appts);
			if (apptIndex < lastAppt)
				 break;

			// Last visible?
			else if (apptIndex == lastAppt)
			{
				if ((apptIndex == TopVisibleAppt) || (dataHeight == tableHeight))
					break;

				// Remove the top item from the table and reload the table again.
				TopVisibleAppt++;
				apptIndex = TopVisibleAppt;
			}
			// Below last visible.
			else
				TopVisibleAppt = apptIndex;

			row = 0;
			dataHeight = 0;
			oldPos = pos = 0;
		}
	}

	// Hide the items that don't have any data.
	numRows = sMaxNumAppts;
	while (row < numRows)
	{
		TblSetRowUsable (table, row, false);
		row++;
	}

	// If the table is not full and the first visible record is
	// not the first record	in the database, displays enough records
	// to fill out the table.
	while (dataHeight < tableHeight)
	{
		apptIndex = TopVisibleAppt;
		if (apptIndex == 0)
			break;

		apptIndex--;

		height = DayViewGetDescriptionHeight (apptIndex, columnWidth, tableHeight, &iconsWidth, &fontID);

		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;

		// Insert a row before the first row.
		TblInsertRow (table, 0);

		apptRowID = appts[apptIndex].rowID;
		masked = false;
		if (apptRowID != dbInvalidCursorID)	// empty time slot?
		{
			//mask if appropriate
			DbGetRowAttr (ApptDB, apptRowID, &attr);
	   		masked = (Boolean) (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));
		}
		else
			apptRowID = dbInvalidCursorID;

		TblSetRowMasked(table, 0, masked);
		DayViewInitRow (table, 0, apptIndex, height, apptRowID, iconsWidth, fontID);

		TopVisibleAppt = apptIndex;

		dataHeight += height;
	}

	// If the height of the data in the table is greater than the height
	// of the table, then the bottom of the last row is clip and the
	// table is scrollable.
	lastItemClipped = (Boolean) (dataHeight > tableHeight);

	// Update the scroll arrows.
	DayViewUpdateScrollers (frm, lastAppt, lastItemClipped);

	MemPtrUnlock (appts);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewLayoutDay
 *
 * DESCRIPTION: 	This routine builds a list of: untimed appointment,
 *              	timed appointment, and empty time slots, for the
 *              	current day (the date store in the global variable
 *					Date).
 *
 * PARAMETERS:  -> 	frmP: 	pointer on day view form.
 *				-> 	tableP: pointer on appts table.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			The global variables sApptsH and sNumAppts are set by
 *             		this routine.
 *
 * REVISION HISTORY:
 *	ART	5/1/96		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL	12/18/03	fix sNumAppts init when no appointement
 *					to avoid scroll arrows flickering.
 *
 ***********************************************************************/
static void DayViewLayoutDay (FormPtr frmP, TablePtr tableP)
{
	uint16_t		i;
	uint16_t		j;
	uint16_t		k;
	uint16_t		index;
	int32_t			numRows;
	int32_t			height;
	int32_t			lineHeight;
	int32_t			iconsWidth;
	int32_t			tableHeight;
	int32_t			columnWidth;
	FontID			fontID;
	FontID			currFont;
	TimeType		next;
	TimeType		nextTime;
	TimeType		endTime;
	Boolean			replace;
	Boolean			addEndTime;
	ApptInfoPtr		appts;
	ApptInfoPtr		apptsOnly;
	RectangleType 	r;
	Boolean			addedEventWrappingOnNextDay = false;

	// Free the existing list.
	if (sApptsH)
	{
		MemHandleFree (sApptsH);
		sApptsH = NULL;
	}

	// Initializations
	TblGetBounds (tableP, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (tableP, descColumn);
	currFont = FntSetFont (apptEmptyDescFont);
	lineHeight = FntLineHeight ();
	FntSetFont (currFont);

	// If there are no appointments on the day, fill in the appointment list
	// with empty time slots.
	if (! sApptsOnlyH)
	{

		
		sNumAppts = (!CompressDayView) ? 
				(DayViewEndHour - DayStartHour + 1) 
				: min((DayViewEndHour - DayStartHour + 1) , ((uint16_t) tableHeight/ (
				uint16_t)lineHeight)) ;

			
		if (sNumAppts >= sMaxNumAppts)
			sNumAppts = sMaxNumAppts;


		sApptsH = MemHandleNew ((uint32_t)(sNumAppts * sizeof (ApptInfoType)));
		appts = MemHandleLock (sApptsH);

		for (i = 0; i < sNumAppts; i++)
		{
			appts[i].startTime.hours = DayStartHour + i;
			appts[i].startTime.minutes = 0;
			if (appts[i].startTime.hours >= maxHours)
			{
				appts[i].endTime.hours = maxHours;
				appts[i].endTime.minutes = maxMinutes;
			}
			else
			{
				appts[i].endTime.hours = DayStartHour + i + 1;
				appts[i].endTime.minutes = 0;
			}
			appts[i].rowID = dbInvalidCursorID;
			appts[i].overlapState = overlapNone;
			appts[i].noTimeEvent = false;
			appts[i].belongingToCurrCat	= true;
			TimeToInt(appts[i].startTimeInPrevDay) = 0;
			TimeToInt(appts[i].latestEndTimeInNextDay) = 0;
		}

		// Reserve space on left for time bars
		DayViewCheckForConflicts ();

		MemHandleUnlock (sApptsH);
		return;
	}

	// Merge empty time slots into the appointment list.
	//
	// Allocate space for the maximun number of empty time slots that
	// we may need to add.
	sApptsH = MemHandleNew ((uint32_t)((sNumApptsOnly + (hoursPerDay * 2)) * sizeof(ApptInfoType)));
	appts = MemHandleLock (sApptsH);
	sNumAppts = 0;
	index = 0;

	// Add the untimed events, the timed events, and a blank time slot for
	// the end-time of each timed event.
	apptsOnly = MemHandleLock (sApptsOnlyH);
	for (i = 0; i < sNumApptsOnly; i++)
	{
		// Find the correct position at which to insert the current event.
		replace = false;
		for (j = index; j < sNumAppts; j++)
		{
			if (appts[j].rowID == dbInvalidCursorID)
			{
				// If an empty time slot with the same start-time already exist, then
				// replace it.
				if (TimeToInt (appts[j].startTime) == TimeToInt (apptsOnly[i].startTime))
				{
					replace = true;
					break;
				}

				// If we find an empty time slot that has an start-time before
				// the start-time of the current event and an end-time after the
				// the start-time of the current event, adjust the end-time of the
				// empty time such that it is equal to the start of the current event.
			 	if (TimeToInt (appts[j].startTime) < TimeToInt (apptsOnly[i].startTime) &&
			 		 TimeToInt (appts[j].endTime)   > TimeToInt (apptsOnly[i].startTime))
			 	{
		 			appts[j].endTime = apptsOnly[i].startTime;
			 	}
			 }

			if (TimeToInt (appts[j].startTime) > TimeToInt (apptsOnly[i].startTime))
			{
				// Make room for the empty time slot we're about to add.
				memmove (&appts[j+1], &appts[j], (int32_t)((sNumAppts - j) * sizeof(ApptInfoType)));
				break;
			}
		}

		// Add the event to the list.
		appts[j] = apptsOnly[i];
		index = j + 1;

		if (! replace)
			sNumAppts++;

		// If the event has no duration, skip it to avoid displaying the same time twice.
		if ( ((apptsOnly[i].noTimeEvent) && (DayStartHour == 0)) ||
			(TimeToInt (apptsOnly[i].startTime) != TimeToInt (apptsOnly[i].endTime)) )
		{
			// Find the correct position at which to insert the end-time time slot.
			addEndTime = true;

			for (j = index; j < sNumAppts; j++)
			{
				// If an event already exist that has a start-time equal to the
				// end-time of the current event then we don't need to add an
				//  end-time time slot.
				if (TimeToInt (appts[j].startTime) == TimeToInt (apptsOnly[i].endTime))
				{
					addEndTime = false;
					break;
				}

				// We've found the position to insert the empty time slot when we find
				// an appointment with a start-time greater than the end-time of the
				// current event.
				if (TimeToInt (appts[j].startTime) > TimeToInt (apptsOnly[i].endTime))
				{
					// Make room for the empty time slot we're about to add.
					memmove (&appts[j+1], &appts[j], (int32_t)((sNumAppts - j) * sizeof(ApptInfoType)));
					break;
				}
			}

			// Update end time if needed in previously added overlapping column
			if (addedEventWrappingOnNextDay && apptsOnly[i].overlapState == overlapFirstPart)
			{
				for (k=0; k<sNumAppts; k++)
				{
					if (appts[k].overlapState == overlapFirstPartCommonRow)
						break;
				}
				TimeToInt(appts[k].latestEndTimeInNextDay) =
					max(TimeToInt(appts[k].latestEndTimeInNextDay), TimeToInt(appts[i].latestEndTimeInNextDay));
				continue;
			}

			if (addEndTime)
			{
				// Add the ending overlapping event if it doesn't already exist
				if (apptsOnly[i].overlapState == overlapFirstPart)
				{
					if (!addedEventWrappingOnNextDay)
					{
						appts[j].rowID = dbInvalidCursorID;
						TimeToInt(appts[j].startTime) = overlapEndTime;
						TimeToInt(appts[j].endTime) = overlapEndTime;
						appts[j].startTimeInPrevDay = apptsOnly[i].startTimeInPrevDay;
						appts[j].latestEndTimeInNextDay = apptsOnly[i].latestEndTimeInNextDay;
						appts[j].overlapState = overlapFirstPartCommonRow;
						appts[j].noTimeEvent = false;
						appts[j].belongingToCurrCat	= true;

						sNumAppts++;
						addedEventWrappingOnNextDay = true;
					}
				}
				else
				{
					// The end time of the empty time slot is the earlier of:
					//		o the start time plus one hour
					//		o 11:55 pm
					//		o the start time of the next event.
					if (apptsOnly[i].endTime.hours < 23)
					{
						endTime.hours = apptsOnly[i].endTime.hours + 1;
						endTime.minutes = 0;
					}
					else
					{
						endTime.hours = 23;		// max end time is 11:55 pm
						endTime.minutes = 55;
					}

					if (j < sNumAppts && TimeToInt(endTime) > TimeToInt(appts[j+1].startTime))
						endTime = appts[j+1].startTime;

					appts[j].rowID = dbInvalidCursorID;
					appts[j].startTime = apptsOnly[i].endTime;
					appts[j].endTime = endTime;
					appts[j].overlapState = overlapNone;
					appts[j].noTimeEvent = false;
					appts[j].belongingToCurrCat	= true;
					TimeToInt(appts[j].startTimeInPrevDay) = 0;
					TimeToInt(appts[j].latestEndTimeInNextDay) = 0;

					sNumAppts++;
				}
			}
		}
	}


	// Reserve spase for the time bars.  We will show time bars if the user
	// has requested them or if there are overlapping appointments.
	DayViewCheckForConflicts ();


	// Determine if there is space to add empty time slot, these time slot are in
	// addition to the empty time slot that represent end times of events.
	numRows = sMaxNumAppts;


	if (( ! CompressDayView) || (numRows > sNumAppts))
	{
		height = 0;

		if (CompressDayView)
		{
			for (j = 0; j < sNumAppts; j++)
			{
				height += DayViewGetDescriptionHeight (j, columnWidth, tableHeight, &iconsWidth, &fontID);
				if (height >= tableHeight)
					break;
			}
		}

		// Add empty time slots to the list of appointment until the table is full.
		next.hours = (uint8_t) DayStartHour;
		next.minutes = 0;
		i = 0;

		while (( ! CompressDayView) || (height + lineHeight <= tableHeight))
		{
			if ((i < sNumAppts) &&
				 (TimeToInt (next) >= TimeToInt (appts[i].startTime)))
			{
				if (TimeToInt (next) <= TimeToInt (appts[i].endTime) &&
					(appts[i].endTime.hours >= DayStartHour))
				{
					next = appts[i].endTime;
					if (next.minutes || (TimeToInt (appts[i].startTime)
													== TimeToInt (appts[i].endTime)))
					{
						next.hours++;
						next.minutes = 0;
					}
				}
				i++;
			}

			// Insert an empty time slot if we're not passed the end of the
			// day.
			else if ((next.hours < DayViewEndHour) ||
						((next.hours == DayViewEndHour) && next.minutes == 0))
			{
				memmove (&appts[i+1], &appts[i], (int32_t)((sNumAppts - i) * sizeof(ApptInfoType)));
				sNumAppts++;

				appts[i].startTime = next;
				appts[i].rowID = dbInvalidCursorID;
				appts[i].overlapState = overlapNone;
				appts[i].noTimeEvent = false;
				appts[i].belongingToCurrCat	= true;
				TimeToInt(appts[i].startTimeInPrevDay) = 0;
				TimeToInt(appts[i].latestEndTimeInNextDay) = 0;

				// The end time is the beginning of the next hour or the
				// start time of the next appointment, which ever is earliest.
				next.hours++;
				next.minutes = 0;
				if ( (i+1 < sNumAppts)
					&& (TimeToInt (next) > TimeToInt (appts[i+1].startTime)))
				{
					next = appts[i+1].startTime;
				}

				appts[i].endTime = next;

				height += DayViewGetDescriptionHeight (i, columnWidth, tableHeight, &iconsWidth, &fontID);
				i++;
			}

			else if (i < sNumAppts)
			{
				next.hours++;
				next.minutes = 0;
			}

			else
				break;
		}
	}

	// Add events inside a wrapping event if there is space available
	if (addedEventWrappingOnNextDay && (numRows > sNumAppts))
	{
		height = 0;

		for (j = 0; j < sNumAppts; j++)
		{
			height += DayViewGetDescriptionHeight (j, columnWidth, tableHeight, &iconsWidth, &fontID);
			if (height >= tableHeight)
				break;
		}

		// Free space available, we can add some empty rows
		i = 0;

		// Get the first overlapping event
		while(i < sNumAppts)
		{
			if (appts[i].overlapState == overlapFirstPart)
				break;
			i++;
		}

		while (height + lineHeight <= tableHeight)
		{
			if (i == sNumAppts)
				break;

			// Check the time of the following event if it exists
			if (i < sNumAppts-1)
				nextTime = appts[i+1].startTime;
			else
				TimeToInt(nextTime) = overlapEndTime;

			while (height + lineHeight <= tableHeight)
			{
				// Add empty slots until the next event begin date is reached
				next = appts[i].startTime;
				next.hours++;
				next.minutes = 0;
				if (TimeToInt(next) < TimeToInt(nextTime))
				{
					// Increment i to point on the event to add
					i++;

					// Move the events following the one we're going to add
					memmove (&appts[i+1], &appts[i], (int32_t)((sNumAppts - i) * sizeof(ApptInfoType)));

					appts[i].startTime = next;
					next.hours++;
					appts[i].endTime = next;
					appts[i].rowID = dbInvalidCursorID;
					appts[i].overlapState = overlapNone;
					appts[i].noTimeEvent = false;
					appts[i].belongingToCurrCat	= true;
					TimeToInt(appts[i].startTimeInPrevDay) = 0;
					TimeToInt(appts[i].latestEndTimeInNextDay) = 0;

					// Update the height due to the row added
					height += DayViewGetDescriptionHeight (i, columnWidth, tableHeight, &iconsWidth, &fontID);
					sNumAppts++;
				}
				else
					break;
			}

			// Go to next appointment
			i++;
		}
	}

	MemHandleUnlock (sApptsOnlyH);

	// Release any unused space in the appointment list;
	MemHandleUnlock (sApptsH);
	MemHandleResize (sApptsH, (uint32_t)(sNumAppts * sizeof (ApptInfoType)));
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewLoadApptsFromDB
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 		None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	01/20/02	Initial Revision.
 *
 ***********************************************************************/
void DayViewLoadApptsFromDB (void)
{
	if (sApptsOnlyH)
	{
		MemHandleFree (sApptsOnlyH);
		sApptsOnlyH = NULL;
	}

	ApptGetAppointments (ApptDB, &gApptCursorID, Date, 1, &sApptsOnlyH, &sNumApptsOnly, true);
}


/***********************************************************************
 *
 * FUNCTION:    	DayRefreshDisplay
 *
 * DESCRIPTION: 	Redefines from scratch the table layout.
 *
 * PARAMETERS:  ->	reloadFromDB:		true if the list must reloaded
 *										from datebook DB
 *				->	invalidateDisplay:	true to generate an update
 *										code that will draw the new table.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			The global variables sApptsH and sNumAppts are set by
 *             		this routine.
 *
 * REVISION HISTORY:
 *	PLE	01/20/02	Initial revision.
 *
 ***********************************************************************/
void DayViewRefreshDisplay (Boolean reloadFromDB, Boolean invalidateDisplay)
{
	FormPtr 		frmP;
	TablePtr	 	tableP;

	frmP = FrmGetFormPtr (DayView);
	tableP = UtilitiesGetFormObjectPtr(frmP, DayTable);

	// Reload from scratch if requested
	if (reloadFromDB)
		DayViewLoadApptsFromDB();

	// Get correct coloring for the month
	if(sBackgroundMonth != Date.month)
	{
		DateBackgroundGetHighlightColor(&sBackgroundHighlight, Date.month);
		DateBackgroundGetBitmapID(&sGadgetCallbackData.bitmapID, Date.month);
		sGadgetCallbackData.color = sBackgroundHighlight;
		sBackgroundMonth = Date.month;

		// Set the color of the lines in the text fields
		UIColorSetTableEntry(UIFieldTextLines, &sBackgroundHighlight);

		DayViewDrawInvalidate(NULL);
	}

	// Define the layout of the table
	DayViewLayoutDay(frmP, tableP);

	// Initialize the table using new layout
	DayViewLoadTable(frmP);

	// Invalidate the table if requested, so that it will be redrawn
	if (invalidateDisplay)
		TblInvalidate(tableP);

}


/***********************************************************************
 *
 * FUNCTION:    	DayViewNewAppointment
 *
 * DESCRIPTION: 	This routine adds a new untimed appointment to the
 *					top of current day.
 *
 * PARAMETERS:  -> 	event : pointer to the keyDown event, or NULL
 *				-> 	switchOnNewEvent : if true, the table is invalidated.
 *
 * RETURNED:    	true if the key has handle by this routine.
 *
 * NOTE:			Note.
 *
 * REVISION HISTORY:
 *	ART	7/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static uint32_t DayViewNewAppointment (EventType* eventP, Boolean switchOnNewEvent)
{
	status_t 			error;
	uint32_t			newRecRowID = dbInvalidRowID;
	char 				desc[4] = "";
	TimeType 			startTime;
	TimeType 			endTime;
	ApptDBRecordType 	newAppt;
	ApptDateTimeType	 when;
	Boolean 			eventIsWrappedAroundMidnight = false;
	TimeSelectionType 	selectType;
	Boolean				noTimeEvent;

	TimeToInt (startTime) = noTimeStartEndTime;
	TimeToInt (endTime) = noTimeStartEndTime;
	noTimeEvent = true;

	// We'll have an event if the appointment is being created as the result
	// writing a character when no appointment is selected.
	if (eventP)
	{
		if (TxtCharIsDigit (eventP->data.keyDown.chr))
		{
			startTime.hours = (uint8_t)(eventP->data.keyDown.chr - '0');
			startTime.minutes = 0;
			endTime.hours = startTime.hours + 1;
			endTime.minutes = 0;
			eventIsWrappedAroundMidnight = false;
			selectType = tSelectStart;
			if (!GetTime (&startTime, &endTime, &Date, &eventIsWrappedAroundMidnight,
				&selectType, setTimeTitleStrID))
				return dbInvalidRowID;

			noTimeEvent = (Boolean) (selectType == tSelectNoTime);
		}
	}


	// Limit the number of appointments that can be enter on a day.
	if (sNumApptsOnly >= apptMaxPerDay)
		return dbInvalidRowID;

	// Create a untimed appointment on the current day.
	memset (&newAppt, 0x00, sizeof (newAppt));
	when.startTime = startTime;
	when.endTime = endTime;
	when.date = Date;
	when.noTimeEvent = noTimeEvent;
	when.midnightWrapped = eventIsWrappedAroundMidnight;
	strncpy(when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);

	newAppt.when = when;
	newAppt.description = desc;

	if (AlarmPreset.advance != apptNoAlarm)
		newAppt.alarm = AlarmPreset;
	else
		newAppt.alarm.advance = apptNoAlarm;

	error = ApptNewRecord (ApptDB, &newAppt, &newRecRowID);

	// If necessary display an alert that indicates that the new record could
	// not be created.
	if (error < errNone)
	{
		FrmUIAlert(DeviceFullAlert);
		return dbInvalidRowID;
	}

	// Assign the current category to the event, if the current
	// category displayed is all, let the new record as unfiled (default value)
	if (! DatebookAllCategorySelected())
		DbSetCategory(ApptDB, newRecRowID, DateBkCurrentCategoriesCount, DateBkCurrentCategoriesP);

	// Move the cursor to the newly added record
	ApptDBRequery(ApptDB, gApptCursorID, false);

	if (switchOnNewEvent)
	{
		DbCursorMoveToRowID(gApptCursorID, newRecRowID);
		gItemSelected = true;

		// Reload the appointments from DB and redefine the layout if we have to switch
		DayViewRefreshDisplay(true, true);

		if (TblFindRowData (UtilitiesGetObjectPtr(DayView, DayTable), newRecRowID, &gSelectedRow))
		{
			DayViewFocusSetRowToSelect(gSelectedRow);
			DayViewFocusHowToSelect(kFocusLastSelectionMode);
			gSelectedColumn = descColumn;
			DayViewFocusSelectRowID(newRecRowID);
		}
	}
	else
	{
		// Reload the appointments from DB to reload the newly inserted event
		DayViewLoadApptsFromDB();
	}

	return newRecRowID;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDeleteAppointment
 *
 * DESCRIPTION: 	This routine deletes the selected appointment.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewDeleteAppointment (void)
{
	TablePtr 	table;
	FormPtr 	frmP;

	frmP = FrmGetFormPtr (DayView);
	table = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));
	TblReleaseFocus (table);

	// The global cursor points on current record.
	// Clear the edit state, this will delete the current record if it is blank.
	if (! DayViewClearEditState ())
	{
		// Delete the record,  this routine will display an appropriate
		// dialog to confirm the action.  If the dialog is canceled
		// don't update the display.
		if (! DeleteRecord (gApptCursorID))
			return;
	}

	// Reload from DB, layout the day again, and display it
	DayViewRefreshDisplay (true, true);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDeleteNote
 *
 * DESCRIPTION: 	This routine deletes the note attached to the selected
 *              	appointment.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/1/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewDeleteNote (void)
{
	FormPtr 	alert;
	uint16_t 	alertButton;
	Boolean 	exception = false;
	Boolean 	splitEvent = false;

	// Check if we are editing an item.
	if (! ApptEventHasNote(ApptDB, gApptCursorID))
		return;

	// If we're changing a repeating appointmemt, check if all occurrences
	// are being changed, or if and exception is being created.
	if (ApptEventRepeatIsSet(ApptDB, gApptCursorID))
	{
		alert = FrmInitForm(gApplicationDbP, RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);
		switch(alertButton)
		{
			case RangeCancelButton:
				return;

			case RangeCurrentButton:
				exception = true;
				break;

			case RangeFutureButton:
				splitEvent = true;
				break;

			default:;
		}
	}
	else if (FrmAlert(gApplicationDbP, DeleteNoteAlert) != DeleteNoteYes)
	{
		return;
	}

	// Remove the note field from the database record.
	if (! DeleteNote (exception, splitEvent))
	{
		return;
	}

	// The note has been deleted, clear the edit state mode and redraw the table
	DayViewClearEditState();
	DayViewRefreshDisplay (true, true);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewSelectTime
 *
 * DESCRIPTION: 	This routine is called when a time item in the day view
 *              	table is selected.  The time picker is displayed, if
 *              	the start or end time of the appointment is changed,
 *              	the day's appointments are resorted and the appointment
 *              	table is redrawn.
 *
 *					the record handled is the one given by the current
 *					position in the cursor.
 *
 * PARAMETERS:  -> 	rowID : rowID / cursorID of the appointment to be selected.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/12/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL 10/15/03	for Cursor Caching need cursorID only.
 *
 ***********************************************************************/
static void DayViewSelectTime (uint32_t rowID)
{
	Boolean 			userConfirmed;
	Boolean 			moved = false;
	TimeType 			startTime;
	TimeType 			endTime;
	DateType			startDate;
	Boolean 			eventWrappedAroundMidnight;
	ApptDBRecordType 	apptRec;
	TimeSelectionType 	selectType;
	Boolean				noTimeAppt;
	FormPtr				frmP;
	status_t			err;
	TablePtr			tableP;
	int16_t				currentRowIDInTable = tblUnusableRow;


	// Initial checkins
	if (DbIsRowID(rowID))
	{
		TraceOutput(TL(appErrorClass,"DayViewSelectTime : rowID  is a row = %lx", rowID));
		// This is a rowID, make the global cursor to point on it
		DbCursorMoveToRowID(gApptCursorID, rowID);
	}
	else if (DbIsCursorID(rowID))
	{
		TraceOutput(TL(appErrorClass,"DayViewSelectTime : rowID  is a cursor = %lx", rowID));
	}
	else
	{
		TraceOutput(TL(appErrorClass,"parameter is an invalid row or cursor"));

		// Check the cursor
		goto TimeSelectFailed;
	}

	// Load appointment data
	err = ApptGetRecord (ApptDB, gApptCursorID, &apptRec, DBK_SELECT_TIME);
	if (err < errNone)
		goto TimeSelectFailed;


	startTime = apptRec.when.startTime;
	endTime = apptRec.when.endTime;
	eventWrappedAroundMidnight = apptRec.when.midnightWrapped;
	ApptFreeRecordMemory (&apptRec);

	// Display the current date in time selector (for repeating appts)
	startDate = Date;

	// Display the time picker
	selectType = (TimeSelectionType) ((apptRec.when.noTimeEvent) ? tSelectNoTime : tSelectStart);
	userConfirmed = GetTime (&startTime, &endTime, &startDate, &eventWrappedAroundMidnight,
						&selectType, setTimeTitleStrID);

	frmP = FrmGetFormPtr (DayView);
	tableP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));

	// if the user has confirmed the dialog pass the information to MoveEvent
	if (userConfirmed)
	{
		noTimeAppt = (Boolean) (selectType == tSelectNoTime);
		if (MoveEvent (gApptCursorID, startTime, endTime, startDate, eventWrappedAroundMidnight,
			noTimeAppt, true, &moved) < errNone)
		{
			goto TimeSelectFailed;
		}


	}
	else
	{
		// The user tapped cancel
		TblUnhighlightSelection(tableP);
		if (ApptDeleteRecordIfEmpty(ApptDB, gApptCursorID))
		{
			// Recompute the layout
			DayViewRefreshDisplay(true, false);
		}

		// Refresh to redraw the time bar
		TblInvalidate(tableP);

		goto TimeSelectFailed;
	}

	// Activate edition, the global cursor already point on current event
	gItemSelected = true;


	// If the appointment moved, update the cursor to include the new record
	// before setting the cursor on this new record.
	// The display needs to be updated when an event is actually changed because
	// just tapping on the time of an empty slot causes a new "empty" event to be
	// created and the view needs to be updated to take care of time bars & alarm
	// icon (when the preset is set)
	DayViewRefreshDisplay(moved, false);

	// Utile ?
	TblInvalidate(tableP);
	return;

TimeSelectFailed:
	DayViewClearEditState();
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewIconTrackingFind
 *
 * DESCRIPTION: 	This function find the icon hit.
 *
 *
 * PARAMETERS: 	->	tableP:		day View table.
 *				->	row:		row
 *				->	x:			x coordinate.
 *				->	y:			y coordinate.
 *				<-	apptRowIDP:	RowId of related DB record.
 *				<-	iconBounds:	Bounds of found icon.
 *
 * RETURNED:    	return icon identifier.
 *						kDayViewNoteIcon = 1
 *						kDayViewAlarmIcon = 2,
 *						kDayViewRepeatIcon = 4,
 *						kDayViewTimeZoneIcon = 8;
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	09/08/03	Initial revision.
 * 	PPL 06/03/04	Bitmap icons are replacing symbol chars.
 *
 ***********************************************************************/
static uint16_t  DayViewIconTrackingFind(TableType* tableP, uint16_t row, Coord x, Coord y, uint32_t* apptRowIDP, RectangleType* iconBoundsP)
{
	uint32_t		apptRowID = dbInvalidRowID;
	uint16_t		iconsWidth;
	FontID			curFont;
	Boolean			inIcon = false;
	RectangleType	itemR;
	RectangleType	iconBounds;
	uint16_t		iconHit = kDayViewNoIcon;

	// Check if there are any icons.

	if (TblRowMasked(tableP,row))
		return (iconHit);

	iconsWidth = TblGetItemInt (tableP, row, descColumn);
	if (! iconsWidth)
		return iconHit;

	// Check if the pen is within the bounds of the icons.
	TblGetItemBounds (tableP, row, descColumn, &itemR);

	iconBounds.topLeft.x = itemR.topLeft.x + itemR.extent.x - iconsWidth;
	iconBounds.topLeft.y = itemR.topLeft.y;
	iconBounds.extent.x = iconsWidth;
	curFont = FntSetFont (ApptDescFont);
	iconBounds.extent.y = FntLineHeight ();

	FntSetFont (curFont);

	if (RctPtInRectangle (x, y, &iconBounds))
	{
		//curFont = FntSetFont (symbolFont);

		// Get the record index of the selected appointment.
		apptRowID = TblGetRowData (tableP, row);

		// Check if the pen is on the note icon.
		if (ApptEventHasNote(ApptDB, apptRowID))
		{
			iconBounds.extent.x  = GetBitmapWidth(NoteIconBitmap);
			inIcon = RctPtInRectangle (x, y, &iconBounds);
			if (inIcon)
			{
				iconHit = kDayViewNoteIcon;
				goto ExitDayViewFindIcon;
			}

			iconBounds.topLeft.x += iconBounds.extent.x;
		}

		// Check if the pen is on the alarm icon.
		if (ApptEventAlarmIsSet(ApptDB, apptRowID))
		{
			iconBounds.extent.x  = GetBitmapWidth(AlarmIconBitmap);
			inIcon = RctPtInRectangle (x, y, &iconBounds);
			if (inIcon)
			{
				iconHit = kDayViewAlarmIcon;
				goto ExitDayViewFindIcon;
			}

			iconBounds.topLeft.x += iconBounds.extent.x;
		}

		// Check if the pen is on the repeat icon.
		if (ApptEventRepeatIsSet(ApptDB, apptRowID))
		{
			iconBounds.extent.x  = GetBitmapWidth(RepeatIconBitmap);
			inIcon = RctPtInRectangle (x, y, &iconBounds);
			if (inIcon)
			{
				iconHit = kDayViewRepeatIcon;
				goto ExitDayViewFindIcon;
			}

			iconBounds.topLeft.x += iconBounds.extent.x;
		}

		// Check if the pen is on the timezone icon.
		if (ApptEventTZDiffersFromSystemTZ(ApptDB, apptRowID))
		{
			iconHit = kDayViewTimeZoneIcon;
			iconBounds.extent.x = GetBitmapWidth(TimeZoneIconBitmap);
		}
	}

ExitDayViewFindIcon:

	if (iconBoundsP)
	{
		if (iconHit == kDayViewNoIcon)
			RctSetRectangle(iconBoundsP, 0,0,0,0);
		else
			RctCopyRectangle(&iconBounds, iconBoundsP);
	}

	if (apptRowIDP)
		*apptRowIDP = apptRowID;

	FntSetFont (curFont);

	return iconHit;
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewIconTrackingDoAction
 *
 * DESCRIPTION: 	This function find the icon hit.
 *
 * PARAMETERS:  ->	tableP:		table.
 *				->	row:		row.
 *				->	x, y:		x, y coordinate.
 *				->	apptRowID:	record row ID.
 *
 * RETURNED:    	return icon identifier.
 *					eDayViewNoteIcon = 1
 *					eDayViewAlarmIcon = 2,
 *					eDayViewRepeatIcon = 4,
 *					eDayViewTimeZoneIcon = 8;
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	09/08/03	Initial revision.
 *
 ***********************************************************************/
static void  DayViewIconTrackingDoAction(TableType* tableP, uint16_t row, Coord x, Coord y, uint32_t apptRowID)
{
	if (DbCursorMoveToRowID(gApptCursorID, apptRowID) < errNone)
		return;

	switch(sIconHit)
	{
		case kDayViewNoteIcon:
			DayViewClearEditState();
			DetailsEditNote(gApptCursorID);

			// If the note has been deleted, build a new layout
			if (! ApptEventHasNote(ApptDB, gApptCursorID))
				DayViewRefreshDisplay(true, true);

			// we do not want to open Details dialog
			return;;

		case kDayViewAlarmIcon:
		case kDayViewRepeatIcon:
			// Alarm or Repeat icon: force option information tab
			DetailsSetDefaultEventDetailsTab(DetailsBookOptionsTabId);

			// want details dialog to popup
			break;

		case kDayViewTimeZoneIcon:
			// Timezone icon: force event information tab
			DetailsSetDefaultEventDetailsTab(DetailsBookEventInformationTabId);

			// want details dialog to popup
			break;

		case kDayViewNoIcon:
		default:
			// we do not want to open Details dialog
			return;
	}

	// details dialog pops up !
	FrmPopupForm(gApplicationDbP, DetailsDialog);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewIconTrackingEnd
 *
 * DESCRIPTION: 	This routine handles the selection of Day view Icons:
 *					(note, alarm repeat and time zone icons.)
 *
 *					It finds which icon was pressed in and tracks the pen
 *					until it is released.
 *
 *					If the pen is released within the bounds the icon
 *					we nagivate to appropriate form:
 *
 *					Note: 			Note View
 *					Repeat, Alarm:	Details dialog set on Options tab.
 *					Time zone: 		Details dialog set on Information tab.
 *
 * PARAMETERS:  ->	eventP: 	pointer to a tblEnter event.
 *
 * RETURNED:    	True the event was handled .
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 09/09/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewIconTrackingEnd(EventType* eventP)
{
	if (sIconTracking)
	{
		// reset tracking data
		sIconTracking = false;

		if (sIconHit != kDayViewNoIcon && sIconSelected)
		{
			sIconSelected = false;
			DayViewDrawInvalidate(&sIconBounds);

			DayViewIconTrackingDoAction(sTableP, sRow, eventP->screenX, eventP->screenY, sApptRowID);
		}

		sIconHit = kDayViewNoIcon;
		RctSetRectangle (&sIconBounds, 0, 0, 0, 0);

		return true;
	}
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewIconTrackingTrack
 *
 * DESCRIPTION: 	This function find the icon hit.
 *
 * PARAMETERS:  ->	eventP: 	event to handle.
 *
 * RETURNED:    	True the event was handled .
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	09/08/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewIconTrackingTrack(EventPtr eventP)
{
	if (sIconTracking)
	{
		if (RctPtInRectangle (eventP->screenX, eventP->screenY, &sIconBounds))
		{
			if (!sIconSelected)
			{
				// Highlight the icon if not already
				sIconSelected = true;
				DayViewDrawInvalidate(&sIconBounds);
			}
		}
		else if (sIconSelected)
		{
			// Unhighlight the icon if not already
			sIconSelected = false;
			DayViewDrawInvalidate(&sIconBounds);
		}
		return true;
	}
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewIconTrackingStart
 *
 * DESCRIPTION: 	This routine handles the selection of Day view Icons:
 *					(note, alarm repeat and time zone icons.)
 *
 *					It finds which icon was pressed in and tracks the pen
 *					until it is released.
 *
 *					If the pen is released within the bounds the icon
 *					we nagivate to appropriate form:
 *
 *					Note: 			Note View
 *					Repeat, Alarm:	Details dialog set on Options tab.
 *					Time zone: 		Details dialog set on Information tab.
 *
 * PARAMETERS:  ->	eventP: 	pointer to a tblEnter event.
 *
 * RETURNED:    	True the event was handled .
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/16/96		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static Boolean DayViewIconTrackingStart(TablePtr tableP, int16_t enterRow, Coord x, Coord y, int16_t oldRow)
{
	Boolean			penDown;
	RectangleType	dirtyRect;

	// Check if there are any icons.
	sTableP = tableP;
	sRow = enterRow;

	sIconHit = DayViewIconTrackingFind(sTableP, sRow, x, y, &sApptRowID, &sIconBounds);
	if (sIconHit != kDayViewNoIcon)
	{
		// If an other row was highlighted, get it unhighlighted and refresh time bar
		if (oldRow != -1)
		{
			// Force current cell to time column, to get it unhighlighted
			TblSetSelection(sTableP, oldRow, timeColumn);
			TblUnhighlightSelection(sTableP);
			TblGetItemBounds(sTableP, oldRow, timeColumn, &dirtyRect);

			DayViewDrawInvalidate(&dirtyRect);
		}

		// the pen is down
		penDown = true;
		sIconSelected = true;
		sIconTracking = true;

		// Highlight the icon.
		DayViewDrawInvalidate(&sIconBounds);

		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewCheckForNonEditableRows
 *
 * DESCRIPTION: 	Handle specific rows.
 *
 * PARAMETERS:  ->	event:	a pointer to an EventType structure.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	02/26/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewCheckForNonEditableRows(TableType* table, int16_t row)
{
	ApptInfoPtr 				appts;
	MidnightOverlapStateType 	overState;
	Boolean 					belongingToCurrCat;
	uint16_t 					apptIndex;
	uint32_t 					numCategories;
	CategoryID *				pCategoryIDs;
	uint32_t 					recRowID;

	apptIndex = TblGetRowID (table, row);

	appts = MemHandleLock (sApptsH);
	overState = appts[apptIndex].overlapState;
	belongingToCurrCat = appts[apptIndex].belongingToCurrCat;

	// Don't take the rowID from TblGetRowData (table, row) because
	// second parts of overlapping events have an invalid rowID (0)
	recRowID = appts[apptIndex].rowID;
	MemHandleUnlock (sApptsH);

	// Skip if the record is in the wrong category
	if (!belongingToCurrCat)
	{
		// Clear the edit state
		DayViewClearEditState ();

		// Get the category of the selected event
		DbGetCategory (ApptDB, recRowID, &numCategories, &pCategoryIDs);

		// Update current category
		ChangeCurrentCategories(numCategories, pCategoryIDs);
		DbReleaseStorage(ApptDB, pCategoryIDs);

		// Init selection
		DbCursorMoveToRowID(gApptCursorID, recRowID);
		gItemSelected = true;
		DayEditPosition = mostRightFieldPosition;

		// Reload data and refresh
		DayViewDrawDate(Date, true);
		return true;
	}

	if (overState == overlapFirstPartCommonRow)
	{
		// Leave the edit mode
		DayViewClearEditState ();

		// Go back to previous day
		DateAdjust(&Date, +1);

		// Refresh the view
		DayViewDrawDate (Date, true);
		return true;
	}
	else if (overState == overlapScndPart)
	{
		// Leave the edit mode
		DayViewClearEditState ();

		// Go back to previous day
		DateAdjust(&Date, -1);

		// Get the record index
		DbCursorMoveToRowID(gApptCursorID, recRowID);
		gItemSelected = true;

		// Put the cursor a the end of the field
		DayEditPosition = mostRightFieldPosition;

		// Refresh the view
		DayViewDrawDate (Date, true);
		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewSelectItem

 * DESCRIPTION: 	This routine is called when an item in Day View table
 *              	is selected.
 *
 * PARAMETERS:  ->	event:	a pointer to an EventType structure.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/9/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL 10/13/03	Fixes for being update based:
 *					- DayViewRestoreEditState is replaced by
 *					DayViewDrawInvalidate() which is posting winUpdates.
 *	PPL 10/15/03	for Cursor Cahing uses gApptCursorID.
 *
 *
 ***********************************************************************/
static void DayViewSelectItem (TablePtr tableP, int16_t row, int16_t column)
{
	status_t 			error;
	uint16_t 			apptIndex;
	int16_t				updatedRow;
	uint32_t 			recRowID;
	ApptInfoPtr 		appts;
	ApptDBRecordType 	newAppt;
	ApptDateTimeType 	when;
	FormPtr				frmP;
	Boolean				verifyPW;


	apptIndex = TblGetRowID (tableP, row);
	recRowID = TblGetRowData (tableP, row);

	if (recRowID != dbInvalidRowID)
		DbCursorMoveToRowID(gApptCursorID, recRowID);

	if (TblRowMasked(tableP,row))
	{
		frmP = FrmGetFormPtr (DayView);

		// Clear edit state if it was already active, it will be restored below
		if (ClearEditState())
		{
			// an event was deleted so the row could be wrong in the event
			if (TblFindRowData (tableP, recRowID, &updatedRow))
			{
				apptIndex = TblGetRowID (tableP, updatedRow);
				row = updatedRow;
			}
		}

		verifyPW = DateUtilsSecVerifyPW(showPrivateRecords);

		if (verifyPW)
		{
			// We only want to unmask this one record, so restore the preference.
			// to its previous state (not maskPrivateRecords as we can have masked records
			// whithout having set the security level)
			PrefSetPreference (prefShowPrivateRecords, PrivateRecordVisualStatus);

			// Unmask just the current row.
			TblSetRowMasked(tableP, row, false);

			// Only change the visual status of this record, leaving all others masked.
			CurrentRecordVisualStatus = showPrivateRecords;
			// Leave PrivateRecordVisualStatus set to maskPrivateRecords

			// Set selected state
			if (recRowID != dbInvalidRowID)
			{
				DbCursorMoveToRowID(gApptCursorID, recRowID);
				DayEditPosition = mostRightFieldPosition;
				gItemSelected = true;
				gSelectedRow = row;
				gSelectedColumn = column;
				DayViewFocusSetRowToSelect(gSelectedRow);
			}
			// Rebuild the layout and update display
			DayViewRefreshDisplay(true, true);
		}
		else
		{
			// The user did not enter a valid password.
			//Refresh Data and Redraw the table.
			DayViewRefreshDisplay(true, true);
		}
		return;
	}

	// Was a time item selected?
	if (column == timeColumn)
	{
		// If an empty time slot has been selected then create a new appointment.
		if (recRowID == dbInvalidRowID)
		{
			// If we have reached the maximum number of displayable event, then
			// exit.
			if (sNumApptsOnly >= apptMaxPerDay)
				return;

			// Get the start time, and end time of the row selected.
			appts = MemHandleLock (sApptsH);

			// Create a untimed appointment on the current day.
			memset (&newAppt, 0x00, sizeof (newAppt));
			when.startTime = appts[apptIndex].startTime;
			when.endTime = appts[apptIndex].endTime;
			when.date = Date;
			when.midnightWrapped = false;
			when.noTimeEvent = false;
			strncpy(when.timeZoneName, gDeviceTimeZone, TZNAME_MAX);
			newAppt.when = when;
			newAppt.description = (char *)"";

			if (AlarmPreset.advance != apptNoAlarm)
				newAppt.alarm = AlarmPreset;
			else
				newAppt.alarm.advance = apptNoAlarm;

			error = ApptNewRecord (ApptDB, &newAppt, &recRowID);

			if (error >= errNone)
			{
				// Assign the current category to the event, if the current
				// category displayed is all, let the new record as unfiled (default value)
				if (!DatebookAllCategorySelected())
					DbSetCategory(ApptDB, recRowID, DateBkCurrentCategoriesCount, DateBkCurrentCategoriesP);

				appts[apptIndex].rowID = recRowID;

				// Store the unique id of the record in the row.
				TblSetRowData (tableP, row, recRowID);

				// Set current selection
				ApptDBRequery(ApptDB, gApptCursorID, false);

				// move to new recRowID
				DbCursorMoveToRowID(gApptCursorID, recRowID);

				// Reload the appointments from DB to reload the newly inserted event
				DayViewLoadApptsFromDB();
			}

			MemHandleUnlock (sApptsH);

			// Display an alert that indicates that the new record could
			// not be created.
			if (error < errNone)
			{
				FrmUIAlert(DeviceFullAlert);
				return;
			}
		}

		DayViewSelectTime(gApptCursorID);
	}

	// Was a description selected?
	else if (column == descColumn)
	{
		// If the table is in edit mode then the description field
		// was selected, otherwise the note indicator must have
		// been selected.
		if (TblEditing (tableP))
		{
			TblSelectItem(tableP,  row,  column);

			if (!gItemSelected)
			{
				gItemSelected = true;
				gSelectedRow = row;
				gSelectedColumn = column;

				DayViewFocusSetRowToSelect(gSelectedRow);


				// Recompute the layout in case some rows have shifted
				// Reload from DB because conflicts may be found with other categories
				// Before redefining the layout, release the focus and selection
				// to avoid the table pointing on invalid rows due to insertion at top of table.
				//TblReleaseFocus(tableP);
				// we cannot release the focus anymore here : it prevents the fields from handle the tap and drag
				// to select text when another row was focused before
				TblUnhighlightSelection(tableP);
				DayViewLoadApptsFromDB();

				// Move the Cursor to recRowID
				DbCursorMoveToRowID(gApptCursorID, recRowID);

				// Rebuild the layout
				DayViewRefreshDisplay(false, false);

				// Get the new row after new layout computation
				if (TblFindRowData (tableP, recRowID, &updatedRow))
				{
					if (row != updatedRow)
					{
						// Update the row for later use
						row = updatedRow;

						// Force selection on the row containing the current record to edit
						TblSetSelection(tableP, row, descColumn);
					}
				}

				DayViewFocusSetNavState();
				DayViewFocusTableRegainFocus();

				TblInvalidate(tableP);
			}
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandleTableEnterEvent
 *
 * DESCRIPTION: 	This routine is called when we tap in an editable
 *					table row.
 *
 * PARAMETERS:  ->	eventP:		a pointer to an EventType structure.
 *
 * RETURNED:    	True if the event was handled.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	01/01/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewHandleTableEnterEvent (TablePtr tableP, int16_t enterRow, int16_t enterColumn, Coord x, Coord y)
{
	FieldType*		fieldP;
	uint32_t 		recRowID;
	int16_t 		row = -1;
	Boolean			handled = false;


	if (enterColumn == timeBarColumn)
	{
		DayViewClearEditState();
		return true;
	}

	if (DbCursorGetCurrentRowID(gApptCursorID, &recRowID) >= errNone)
	{
		if (TblFindRowData (tableP, recRowID, &row))
		{
			if (enterRow != row)
			{
				//handled = DayViewClearEditState();

				// Switching the new row: clear the edit state but do
				// not call DayViewClearEditState() because the table focus
				// musn't be lost, otherwise the position in new text field
				// will also be lost.
				if (ClearEditState())
				{
					DayViewRefreshDisplay(true, true);

					// Check if the row we tapped on has been removed. If so,
					// release the focus and the edit state.
					if (!TblRowUsable(tableP, enterRow))
					{
						TblReleaseFocus(tableP);
						fieldP = TblGetCurrentField(tableP);
						if (fieldP)
							FldReleaseFocus (fieldP) ;

						TblInvalidate(tableP);

						handled = true;
					}
				}
			}
			else // Same row: just clear edit state but do not remove record
			{
				if (enterColumn == timeColumn)
				{
					FrmSetFocus(FrmGetFormPtr(DayView), noFocus);

					fieldP = TblGetCurrentField(tableP);
					if (fieldP)
						FldReleaseFocus (fieldP) ;

					gItemSelected = false;
				}

				// Set row to -1: we don't need to refresh row in DayViewIconTrackingStart
				// because we didn't switch from an old one.
				row = -1;
			}
		}
 	}

	// Set handled if the user selected the time column or an icon in the description
	if (!handled)
	{
		if (enterColumn == timeColumn)
			return (false);

		handled = DayViewIconTrackingStart(tableP, enterRow, x, y, row);
	}

	// If the user didn't selected time column or icons, handled must be let
	// to false because the cursor must be positionned by system table handling.
 	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewResizeDescription
 *
 * DESCRIPTION: 	This routine is called when the height of
 *					appointment's description is changed as a result of
 *					user input.
 *              	If the new height of the field is shorter,  more
 *					items may need to be added to the bottom of the Day.
 *
 * PARAMETERS:  ->	event:	a pointer to an EventType structure.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	5/18/95		Initial revision.
 *	PPL 10/13/03	Fixes for being update based:
 *					- DayViewRestoreEditState is replaced by
 *					DayViewDrawInvalidate() which is posting winUpdates.
 *
 ***********************************************************************/
static void DayViewResizeDescription (EventType* event)
{
	TablePtr 		tableP;
	FieldPtr 		fieldP;
	RectangleType 	bounds;


	// Get the current height of the field;
	fieldP = event->data.fldHeightChanged.pField;
	FldGetBounds (fieldP, &bounds);

	// Have the table object resize the field and move the items below
	// the field up or down.
	//frmP = FrmGetFormPtr (DayView);
	//tableP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));
	tableP = UtilitiesGetObjectPtr(DayView, DayTable);
	TblHandleEvent (tableP, event);

	// If the field's height has been expanded , and we don't have items scrolled
	// off the top of the table, just update the scrollers.

	if (event->data.fldHeightChanged.newHeight >= bounds.extent.y)
	{
		TopVisibleAppt = TblGetRowID (tableP, 0);
	}

	// Release focus so that DayViewRestoreEditState() will focus
	// on the correct row after scroll
	TblReleaseFocus(tableP);
	DayViewRefreshDisplay(false, true);

}


/***********************************************************************
 *
 * FUNCTION:    	DayViewSetTitle
 *
 * DESCRIPTION: 	Set the Day View form's title, based on the Day and
 *					LongDateFormat global variables.
 *
 * PARAMETERS:  ->	frmP:	Pointer to Day View form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KKR	5/9/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewSetTitle(FormType* frmP)
{
	char 		title[longDateStrLength + 1];
	uint16_t 	dateFormatIndex;
	MemHandle 	templateH;
	char *		templateP;

	// We can't use the long date format to guess at the short date
	// format, since there's not a one-to-one mapping set up in the
	// Formats panel. We'll directly use the short date format.

	// We need to derive the appropriate date template string based on
	// the ShortDateFormat global, which is loaded from sys prefs (and
	// thus is set by the user in the Formats panel).
	if (ShortDateFormat == dfMDYWithDashes)
	{
		dateFormatIndex = DayViewMDYWithDashesTitleStemplateStrID - DayViewFirstTitleTemplateStrID;
	}
	else if (ShortDateFormat > dfYMDWithDashes)
	{
		ErrNonFatalDisplay("Unknown short date format");

		// Default to the dfMDYWithSlashes format.
		dateFormatIndex = (uint16_t)dfMDYWithSlashes;
	}
	else
	{
		dateFormatIndex = (uint16_t) ShortDateFormat;
	}

	templateH = DmGetResource (gApplicationDbP, strRsc, (uint16_t)(DayViewFirstTitleTemplateStrID + dateFormatIndex));
	templateP = (char*)MemHandleLock(templateH);
	DateTemplateToAscii(templateP, (uint8_t) Date.month, (uint8_t) Date.day, (uint16_t)(Date.year + firstYear), title, sizeof(title) - 1);
	MemHandleUnlock(templateH);
   	DmReleaseResource(templateH);

	// Set the title
	FrmCopyTitle (frmP, title);

	// Set the label of the current category
	DateUtilsSetCategoryTriggerLabel(frmP, DayCategoryTrigger);

} // DayViewSetTitle


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawTitle
 *
 * DESCRIPTION: 	This routine draws the day view title and highlights
 *              	the current day's the day-of-week push button.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	8/1/95		Initial revision.
 *
 ***********************************************************************/
static void DayViewDrawTitle (void)
{
	DayViewSetTitle(FrmGetFormPtr (DayView));
	TimeDisplayed = false;
}


/***********************************************************************
 *
 * FUNCTION:    DayViewShowTime
 *
 * DESCRIPTION: 	This routine display the current time in the title
 *					of the day view.
 *
 * PARAMETERS:  ->	frm:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:        	The global variables TimeDisplayed and TimeDisplayTick
 *					are set by this routine.
 *
 * REVISION HISTORY:
 *	ART	5/14/96		Initial revision.
 *	GLO 2/2/99		update TimeDisplayTick (now matches WeekViewShowTime
 *					and MonthViewShowTime.)
 *
 ***********************************************************************/
static void DayViewShowTime (void)
{
	char			title[timeStringLength];
	DateTimeType 	dateTime;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	TimeToAscii ((uint8_t) dateTime.hour, (uint8_t) dateTime.minute, TimeFormat, title);

	FrmCopyTitle (FrmGetFormPtr(DayView), title);

	TimeDisplayed = true;
	TimeDisplayTick = TimGetTicks() + timeDisplayTime;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHideTime
 *
 * DESCRIPTION: 	If the title of the Day View is displaying the current
 *              	time, this routine will change the title to the standard
 *					title (the current date).
 *
 * PARAMETERS:  	None.
 *
 * PARAMETERS:  ->	hide:	True to always hide, false hide only if
 *                     		to time has been display for the require
 *                      	length of time.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 * 	ART	5/14/96		Initial revision.
 *
 ***********************************************************************/
static void DayViewHideTime (Boolean hide)
{
	if (TimeDisplayed)
	{
		if (hide || TimeToWait() == 0)
		{
			DayViewDrawTitle ();
		}
	}

	TimeComputeWaitTime = false;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHighlightSelection
 *
 * DESCRIPTION: 	This routine highlights text in current record.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	04/26/03	Initial revision.
 *
 ***********************************************************************/
static void DayViewHighlightSelection (void)
{
	FormPtr 		frmP;
	TablePtr 		tableP;
	FieldPtr 		fldP;
	uint32_t 		recRowID;
	int16_t 		row;
	int16_t 		tableIndex;

	// Highlight selection if requested
	if (DayEditSelectionLength)
	{
		frmP = FrmGetFormPtr (DayView);
		tableIndex = FrmGetObjectIndex (frmP, DayTable);
		tableP = FrmGetObjectPtr (frmP, tableIndex);
 		DbCursorGetCurrentRowID(gApptCursorID, &recRowID);
		if (TblFindRowData (tableP, recRowID, &row))
		{
			FrmSetFocus (frmP, tableIndex);
			if (TblGetItemStyle(tableP, row, descColumn) != customTableItem)
			{
				TblGrabFocus(tableP, row, descColumn);
				fldP = TblGetCurrentField(tableP);
				if (fldP)
				{
					FldSetSelection(fldP, DayEditPosition, DayEditPosition + DayEditSelectionLength);
					FldGrabFocus(fldP);
				}
			}
			DayViewFocusSetNavState();
			DayViewFocusTableRegainFocus();
		}
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawDate
 *
 * DESCRIPTION: 	This routine display the date passed.
 *
 * PARAMETERS:  ->	date:	date to display.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/29/95		Initial revision.
 *	PPL	02/15/02	Add Active Area support.
 *
 ***********************************************************************/
void DayViewDrawDate (DateType date, Boolean redraw)
{
	uint16_t	dayOfWeek;

	// Adjust the current date and update title
	Date = date;
	sCurrentFieldOffset = 0;
	DayViewDrawTitle();

	// Get all the appointments and empty time slots on the new day.
	DayViewRefreshDisplay (true, false);

	// Update the day-of-week push button to highlight the correct day of
	// the week.
	dayOfWeek = DateUtilsGetDayOfWeek (Date.month, Date.day, (uint16_t)(Date.year + firstYear));
	BookSetActiveTabId(sWeekBook, DayBookDOW1TabId + dayOfWeek, redraw);

	// Highlights the text if requested (find result for instance)
	DayViewHighlightSelection();
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDayOfWeekSelected
 *
 * DESCRIPTION: 	This routine is called when of the day-of-week push
 *					buttons is pressed.
 *
 * PARAMETERS:  ->	updateCode: TabId for one week day.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/27/95		Initial revision.
 *	ppl	12/17/02	rewritten for Tabs support.
 *
 ***********************************************************************/
static void DayViewDayOfWeekSelected (uint16_t updateCode)
{
	int16_t		adjust;
	uint16_t	dayOfWeek;
	uint16_t	newDayOfWeek;

	// Adjust the current date.
	dayOfWeek = DateUtilsGetDayOfWeek (Date.month, Date.day, (uint16_t)(Date.year + firstYear));
	newDayOfWeek = updateCode - DayBookDOW1TabId;

	if (dayOfWeek == newDayOfWeek)
		return;

	adjust = newDayOfWeek - dayOfWeek;
	DateAdjust (&Date, adjust);

	DayViewDrawDate (Date, false);

	DayViewDrawInvalidate(NULL);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewGoToDate
 *
 * DESCRIPTION: 	This routine displays the date picker so that the
 *              	user can select a date to navigate to.  If the date
 *             	 	picker is confirmed, the date selected is displayed.
 *
 *              	This routine is called when a "go to" button is
 *					pressed.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewGoToDate (void)
{
	char* 		title;
	MemHandle 	titleH;
	int16_t 	month;
	int16_t		day;
	int16_t		year;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (gApplicationDbP, strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	if (SelectDay (selectDayByDay, &month, &day, &year, title))
	{
		Date.day = day;
		Date.month = month;
		Date.year = year - firstYear;
	}

	DayViewDrawDate (Date, true);

	MemHandleUnlock (titleH);
   	DmReleaseResource(titleH);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewGotoAppointment
 *
 * DESCRIPTION: 	This routine sets gloabal variables such the Day View
 *              	will display the text found by the text search
 *             	 	command.
 *
 *					frmGotoEvent is sent after a frmLoadEvt
 *					frmGotoEvent replaces the frmOpenForm
 *					see Datebook.c, PrvGoToItem and PrvGoToAlarmItem
 *					That the reason for we call DayViewInit().
 *
 * PARAMETERS:  ->	event: frmGotoEvent.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	8/24/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewGotoAppointment (EventType* event)
{
	FormPtr 			frmP;
	DateTimeType 		dateTime;
	ApptDBRecordType 	apptRec;
	status_t			err;

	TopVisibleAppt = 0;

	frmP = FrmGetFormPtr(DayView);
	DayViewInit (frmP, true);

	// Check that the requested recordnum exists
	if (event->data.frmGoto.recordID != dbInvalidRowID)
	{
		// Update the cursor to include
		ApptGetRecord (ApptDB, event->data.frmGoto.recordID, &apptRec, DBK_SELECT_TIME);
		ApptDBRequeryOnDate(ApptDB, &gApptCursorID, apptRec.when.date);
		ApptFreeRecordMemory (&apptRec);

		// Ok, the record requested exists in database
		err = DbCursorMoveToRowID(gApptCursorID, event->data.frmGoto.recordID);
		ErrNonFatalDisplayIf(err < errNone, "Unable to move on imported record.");
		gItemSelected = true;

		if (event->data.frmGoto.matchFieldNum == EVENT_DESCRIPTION_ID)
		{
			DayEditPosition = event->data.frmGoto.matchPos;
			DayEditSelectionLength = event->data.frmGoto.matchLen;
		}
		else
		{
			// The matching string is not in the description and thus
			// not selectable.
			DayEditPosition = 0;
			DayEditSelectionLength = 0;
		}

		if (event->data.frmGoto.matchFieldNum == EVENT_NOTE_ID)
		{
			EventType editNoteEvent;

			// Copy matching position
			memmove (&editNoteEvent, event, sizeof(EventType));
			editNoteEvent.eType = datebookEditSelectedNote;

			// Push the event
			EvtAddEventToQueue (&editNoteEvent);
		}
		else if (event->data.frmGoto.matchFieldNum == EVENT_LOCATION_ID)
		{
			// Initialize location selection constants
			gDetailsPosition = event->data.frmGoto.matchPos;
			gDetailsSelectionLength = event->data.frmGoto.matchLen;

			// Display the details dialog on location tab
			DetailsSetDefaultEventDetailsTab(DetailsBookEventInformationTabId);
			FrmPopupForm(gApplicationDbP, DetailsDialog);
		}

		// We are entering text for new event
		if (! SetDateToNextOccurrence (gApptCursorID))
			goto SelectionFailed;
	}
	else
		goto SelectionFailed;

	DayViewDrawDate(Date, false);
	return;

SelectionFailed:
	// Requested record does not exist, set date to today
	gItemSelected = false;
	DayEditPosition = 0;
	DayEditSelectionLength = 0;
	// Get today's date
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	Date = CnvDateTime2Date(&dateTime);

	DayViewDrawDate(Date, false);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewScrollDownCurrentRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	tableP:
 *				->	row:
 *				->	tableHeight:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 *	PLe	09/05/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewScrollDownCurrentRow(TablePtr tableP, int16_t row, int32_t tableHeight)
{
	char *			txtP;
	char *			tmpP;
	size_t			textSize;
	size_t			offset;
	int32_t 		descLineHeight;
	int32_t 		linesToDisplayFromCurrToEnd;
	int32_t 		nbLinesDisplayed;
	int32_t 		nbLinesToScroll;
	FontID			fontID;
	Coord			columnWidth;
	RectangleType	r;
	Boolean			scrolledInsideRow = false;

	if (row == 0)
	{
		textSize = 0;
		columnWidth = TblGetColumnWidth (tableP, descColumn);
		txtP = (char *) MemHandleLock(sDescriptionNotEditedHolderH) + sCurrentFieldOffset;

		fontID	= FntSetFont (ApptDescFont);
		descLineHeight = FntLineHeight();
		FntSetFont(fontID);
		nbLinesDisplayed = TblGetRowHeight (tableP, 0) / descLineHeight;

		linesToDisplayFromCurrToEnd = 0;
		tmpP = txtP;
		do
		{
			offset = FntWordWrap(tmpP, columnWidth);
			tmpP += offset;
			linesToDisplayFromCurrToEnd++;
		}
		while (offset > 0);

		// Compute the number of lines to scroll
		nbLinesToScroll = min (linesToDisplayFromCurrToEnd - nbLinesDisplayed, nbLinesDisplayed) - 1;

		if (nbLinesToScroll > 0)
		{
			// Scroll by nbLinesToScroll : compute the new offset to apply
			while (nbLinesToScroll)
			{
				offset = FntWordWrap(txtP, columnWidth);
				sCurrentFieldOffset += offset;
				txtP += offset;
				nbLinesToScroll--;
			}
			scrolledInsideRow = true;
		}

		MemHandleUnlock(sDescriptionNotEditedHolderH);
	}
	else
	{
		TblGetItemBounds (tableP, row, descColumn, &r);
		if ((int32_t) (r.topLeft.y + TblGetRowHeight(tableP, row)) > tableHeight)
		{
			// Do not switch to next row because it wasn't fully displayed, however
			// reset the offset to start at the top of the table
			scrolledInsideRow = true;
			sCurrentFieldOffset = 0;
		}
	}

	if (!scrolledInsideRow)
		sCurrentFieldOffset = 0;

	return scrolledInsideRow;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewScrollUpCurrentRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	tableP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	09/05/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewScrollUpCurrentRow(TablePtr tableP)
{
	char *		txtP;
	char *		tmpP;
	int32_t 	descLineHeight;
	int32_t 	nbLinesDisplayed;
	int32_t 	nbLinesToScroll;
	FontID		fontID;
	Coord	 	columnWidth;
	int32_t		offset;
	int32_t		linesToDisplayFromStartToCurr;
	Boolean		scrolledInsideRow = false;

	if (sCurrentFieldOffset > 0)
	{
		columnWidth = TblGetColumnWidth (tableP, descColumn);
		txtP = (char *) MemHandleLock(sDescriptionNotEditedHolderH);

		fontID	= FntSetFont (ApptDescFont);
		descLineHeight = FntLineHeight();
		FntSetFont(fontID);
		nbLinesDisplayed = TblGetRowHeight (tableP, 0) / descLineHeight;

		// Get the number of rows between offset 0 and offset sCurrentFieldOffset
		offset = 0;
		linesToDisplayFromStartToCurr = 0;
		while (offset < sCurrentFieldOffset)
		{
			tmpP = txtP + offset;
			offset += FntWordWrap(tmpP, columnWidth);
			linesToDisplayFromStartToCurr++;
		}

		// Compute the number of lines to scroll from the start
		nbLinesToScroll = max (0, (linesToDisplayFromStartToCurr - nbLinesDisplayed + 1));

		// Scroll by nbLinesToScroll : compute the new offset to apply
		offset = 0;
		while (nbLinesToScroll)
		{
			tmpP = txtP + offset;
			offset += FntWordWrap(txtP, columnWidth);
			nbLinesToScroll--;
		}
		scrolledInsideRow = true;
		sCurrentFieldOffset = offset;

		MemHandleUnlock(sDescriptionNotEditedHolderH);
	}

	if (!scrolledInsideRow)
		sCurrentFieldOffset = 0;

	return scrolledInsideRow;
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewScroll
 *
 * DESCRIPTION: 	This routine scrolls the day of of appointments
 *              	in the direction specified.
 *
 * PARAMETERS:  ->	direction:	up or dowm.
 *              ->	wrap:     	if true the day is wrap to the first
 *								appointment,
 *                          	if we're at the end of the day.
 *
 * RETURNED:    	True or false to tell caller whether or not
 *					view was scrolled into direction.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	02/21/95	Initial revision.
 *	PPL 07/02/04	Add  Boolean result code that tells if view
 *					was actually scrolled.
 *
 ***********************************************************************/
static Boolean DayViewScroll (WinDirectionType direction)
{
	int16_t			row;
	int32_t			height;
	uint16_t		apptIndex;
	int32_t			iconsWidth;
	int32_t 		columnWidth;
	int32_t 		tableHeight;
	TablePtr		tableP;
	RectangleType	r;
	FormPtr 		frmP;
	FontID			fontID;


	frmP = FrmGetFormPtr (DayView);
	tableP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));

	// Get the height of the tableP and the width of the description
	// column.
	TblGetBounds (tableP, &r);
	tableHeight = r.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth (tableP, descColumn);

	apptIndex = TopVisibleAppt;

	// Scroll the tableP down.
	if (direction == winDown)
	{
		row = TblGetLastUsableRow (tableP);

		if (TopVisibleAppt+row+1 > sNumAppts)
			return false;

		apptIndex = TblGetRowID (tableP, row);

		// If there is only one appointment visible, this is the case
		// when a appointment occupies the whole screeen. Then scroll
		// the field only by increasing the loading offset. Otherwise,
		// go to next appointment.
		if (! DayViewScrollDownCurrentRow(tableP, row, tableHeight))
		{
			// Switch to next appointment
			apptIndex++;
		}
	}


	// Scroll the table up.
	else
	{
		if ((TopVisibleAppt == 0) && (sCurrentFieldOffset == 0))
			return false;

		if (! DayViewScrollUpCurrentRow(tableP))
		{
			// Scan the records starting with the first visible record to
			// determine how many record we need to scroll.  Since the
			// heights of the records vary,  we sum the heights of the
			// records until we get a screen full.
			height = TblGetRowHeight (tableP, 0);
			if (height >= tableHeight)
			{
				height = 0;
			}

			while ( (height < tableHeight) && (apptIndex > 0) )
			{
				height += DayViewGetDescriptionHeight (apptIndex - 1, columnWidth, tableHeight, &iconsWidth, &fontID);
				if ((height <= tableHeight) || (apptIndex == TopVisibleAppt))
				{
					apptIndex--;
				}
			}
		}
	}

	TopVisibleAppt = apptIndex;
	TblUnhighlightSelection (tableP);
	DayViewRefreshDisplay(false, true);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewPurge
 *
 * DESCRIPTION: 	This routine deletes appointment that are before the
 *              	user specified date.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	8/1/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewPurge (void)
{
	// If the current date is before the purge date redraw the day, we may
	// have deleted some of the appointments being displayed.
	if (PurgeRecords ())
		DayViewRefreshDisplay(true, true);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDoCommand
 *
 * DESCRIPTION: 	This routine preforms the menu command specified.
 *
 * PARAMETERS:  ->	command: menu item id.
 *
 * RETURNED:    	true if.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	art	8/1/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *	PPL 10/13/03	Fixes for being update based:
 *					- DayViewRestoreEditState is replaced by
 *					DayViewDrawInvalidate() which is posting winUpdates.
 *	PPL 08/11/03	Fixes in order to grant user to act on the last
 *					selected row in table using one-handed navigation
 *
 ***********************************************************************/
static Boolean DayViewDoCommand (uint16_t command)
{
	size_t	 	pasteLen;
	MemHandle 	pasteCharsH;
	FontID		fontID;
	FieldPtr 	fieldP;
	TablePtr 	tableP;
	Boolean 	handled = true;
	EventType	newEvent;
	FormPtr 	formP;
	uint32_t 	newRecRowID;

	formP = FrmGetFormPtr (DayView);
	tableP = UtilitiesGetFormObjectPtr(formP, DayTable);

	switch (command)
	{
		// New
		case NewItemCmd:
			MenuEraseStatus(0);

			TblReleaseFocus (tableP);
			fieldP = TblGetCurrentField(tableP);
			if (fieldP)
				FldReleaseFocus(fieldP);

			DayViewClearEditState ();
			newRecRowID = DayViewNewAppointment (NULL, true);

			//DayViewSelectTime (newRecRowID);
			break;

		// Delete
		case DeleteCmd:
			MenuEraseStatus(0);
			if (gItemSelected 
			|| (gItemWasSelected && DayViewTableValidRow(tableP, sDayViewLastRowFocused)))
			{
				DayViewDeleteAppointment ();
				gItemWasSelected = false;
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		// Create note
		case CreateNoteCmd:
			MenuEraseStatus(0);
			if (gItemSelected 
			|| (gItemWasSelected && DayViewTableValidRow(tableP, sDayViewLastRowFocused)))
			{
				TblReleaseFocus (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable)));
				if (CreateNote (true))
				{
					DetailsEditNote(gApptCursorID);
					DayViewRefreshDisplay(true, true);
				}
				else
				{
					DayViewDrawInvalidate(NULL);
				}
				gItemWasSelected = false;
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		// Delete nore
		case DeleteNoteCmd:
			MenuEraseStatus(0);
			if (gItemSelected 
			|| (gItemWasSelected && DayViewTableValidRow(tableP, sDayViewLastRowFocused)))
			{
				DayViewDeleteNote ();
				gItemWasSelected = false;
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		// Purge
		case PurgeCmd:
			MenuEraseStatus(0);
			TblReleaseFocus (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable)));
			DayViewClearEditState ();
			DayViewPurge ();
			break;

		// Beam Record
		case BeamRecordCmd:
			MenuEraseStatus(0);
			if (gItemSelected 
			|| (gItemWasSelected && DayViewTableValidRow(tableP, sDayViewLastRowFocused)))
			{
				TblReleaseFocus (tableP);
				DateSendRecord(ApptDB, gApptCursorID, exgBeamPrefix);
				DayViewDrawInvalidate(NULL);
				gItemWasSelected = false;
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		// Send Record
		case SendRecordCmd:
			MenuEraseStatus(0);
			if (gItemSelected 
			|| (gItemWasSelected && DayViewTableValidRow(tableP, sDayViewLastRowFocused)))
			{
				TblReleaseFocus (tableP);
				DateSendRecord(ApptDB, gApptCursorID, exgSendPrefix);
				DayViewDrawInvalidate(NULL);
				gItemWasSelected = false;
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		// Attach Record
		case AttachRecordCmd:
			MenuEraseStatus(0);
			if (gItemSelected 
			|| (gItemWasSelected && DayViewTableValidRow(tableP, sDayViewLastRowFocused)))
			{
				TblReleaseFocus (tableP);
				DateSendRecord(ApptDB, gApptCursorID, gTransportPrefix);
				DayViewDrawInvalidate(NULL);

				memset(&newEvent, 0x00, sizeof(newEvent));
				newEvent.eType = appStopEvent;
				EvtAddEventToQueue(&newEvent);
				gItemWasSelected = false;
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);
			break;

		// Font Selector
		case DayFontsCmd:
			MenuEraseStatus(0);
			fontID = SelectFont (ApptDescFont);
			if (fontID != ApptDescFont)
			{
				ApptDescFont = fontID;
				DayViewClearEditState ();
				DayViewRefreshDisplay (false, true);
			}
			else
			{
				DayViewDrawInvalidate(NULL);
			}
			break;

		// Preferences
		case DayPreferencesCmd:
			MenuEraseStatus(0);
			DayViewClearEditState ();
			FrmPopupForm(gApplicationDbP, PreferencesDialog);
			break;

		// Display Options
		case DayDisplayOptionsCmd:
			MenuEraseStatus(0);
			DayViewClearEditState ();
			FrmPopupForm(gApplicationDbP, DisplayDialog);
			break;

		// Phone Lookup
		case DayPhoneLookup:
			MenuEraseStatus(0);

			fieldP = TblGetCurrentField (tableP);
			if (fieldP)
			{
				PhoneNumberLookup (fieldP);

				if (FldDirty(fieldP))
				{
				// Save the row in DB
					ApptChangeDescription(ApptDB, gApptCursorID, FldGetTextHandle(fieldP));
				}
			}
			else
				FrmAlert(gApplicationDbP, SelectItemAlert);

			break;

		// Security
		case DaySecurityCmd:
			MenuEraseStatus(0);
			DayViewClearEditState();
			DoSecurity();
			break;

		// About
		case DayGetInfoCmd:
			MenuEraseStatus(0);
			DayViewClearEditState ();
			AbtShowAbout (sysFileCDatebook);
			break;

		// Paste
		case sysEditMenuPasteCmd:
			MenuEraseStatus(0);
			fieldP = FrmGetActiveField (NULL);
			if (! fieldP)
			{
				pasteCharsH = ClipboardGetItem (clipboardText, &pasteLen);
				if (pasteCharsH && pasteLen)
					DayViewNewAppointment (NULL, true);
			}
			handled = false;
			break;

		// Some the the edit menu commands are handled by FrmHandleEvent.
		default:
			handled = false;
			break;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewResizeForm
 *
 * DESCRIPTION: 	This routine resize a given form and its form items
 *
 * PARAMETERS:  ->	eventP
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	01/28/02	Add AIA support in DayView form.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *	PPL 04/30/04	Add Landscape support.
 *
 ***********************************************************************/
static void DayViewResizeForm(EventType* eventP)
{
	RectangleType 	objBounds;
	FormType* 		frmP;
	TableType*		tableP;
	FieldPtr		fieldP;
	uint16_t		objIndex;
	Coord			offsetX;
	Coord			offsetY;
	Coord			x;
	Coord			y;
	Coord			width;
	Coord			height;
	size_t			editPosition = DayEditPosition;
	size_t			editSelectionLength = DayEditSelectionLength;
	Boolean			restoreEditState = false;

	width = eventP->data.winResized.newBounds.extent.x;
	height = eventP->data.winResized.newBounds.extent.y;
	frmP = FrmGetFormPtr(DayView);

	offsetY = height - gCurrentWinBounds.extent.y;
	offsetX = width  - gCurrentWinBounds.extent.x;

	if (!offsetY && !offsetX)
	{
		// if the window has the height of the screen
		// then the windows is already at the right size
		goto justDefineLayout;
	}

	// Resize the book, Do not Move, need to be resized both x and y
	BookResizeBody(sWeekBook, offsetX, offsetY);

	// DayTable
	objIndex =  FrmGetObjectIndex (frmP, DayTable);
	tableP = (TableType*) FrmGetObjectPtr( frmP, objIndex);


	// If the table was edited, store settings and clear edit state
	if (gItemSelected)
	{
		// Save table current position and selection
		restoreEditState = true;
		fieldP = TblGetCurrentField(tableP);
		if (fieldP)
		{
			FldGetSelection(fieldP, &editPosition, &editSelectionLength);
			editSelectionLength -= editPosition;

			FldReleaseFocus(fieldP);
		}
		// Release the focus: do not call DayViewClearEditState because
		// it will remove empty records
		TblUnhighlightSelection(tableP);

		// focus has to be set to noFocus
		// else when the we will restore the focus on an item
		// that has moved as table size changed the table will keep
		// its internal field data position set on previous position
		// that is a bad position.
		FrmSetFocus(frmP, noFocus);
	}

	// Resize table, do not move, need to be resized both x and y
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.y +=  offsetY;
	objBounds.extent.x +=  offsetX;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	// BackgroundGadget
	objIndex =  FrmGetObjectIndex (frmP, DayBackgroundGadget);
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x +=   offsetX;
	objBounds.extent.y +=   offsetY;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);
	sGadgetCallbackData.whiteRect.extent.x += offsetX;
	sGadgetCallbackData.whiteRect.extent.y += offsetY;

	if (offsetX)
	{
		// set size of description column for table , X only
		TblSetColumnWidth(tableP, descColumn, TblGetColumnWidth(tableP, descColumn) + offsetX);

		// DayNextWeekButton, move on	X only
		objIndex =	FrmGetObjectIndex (frmP, DayNextWeekButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// DayCategoryTrigger, move on  X only
		objIndex =  FrmGetObjectIndex (frmP, DayCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// DayCategoryList, move on  X only
		objIndex =  FrmGetObjectIndex (frmP, DayCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	if (offsetY)
	{
		// DayDayViewButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayDayViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// DayWeekViewButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayWeekViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// DayMonthLunarViewButton for Lunar Calendar
		if (DateSupportsLunarCalendar())
		{
			objIndex =	FrmGetObjectIndex (frmP, DayMonthLunarViewButton);
			FrmGetObjectPosition (frmP, objIndex, &x, &y);
			y +=   offsetY;
			FrmSetObjectPosition (frmP, objIndex, x, y);
		}

		// DayMonthViewButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayMonthViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// DayAgendaViewButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayAgendaViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// DayNewButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayNewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// DayDetailsButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayDetailsButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// DayGoToButton, move on Y
		objIndex =  FrmGetObjectIndex (frmP, DayGoToButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	// DayUpButton, move on Y and X, not resized
	objIndex =  FrmGetObjectIndex (frmP, DayUpButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y += offsetY;
	x += offsetX;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// DayDownButton,  move on Y and X, not resized
	objIndex =  FrmGetObjectIndex (frmP, DayDownButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y += offsetY;
	x += offsetX;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// The Graffiti Shift Indicator,  move on Y and X, not resized
	objIndex = FrmGetNumberOfObjects(frmP);
	while (objIndex--)
	{
		if (FrmGetObjectType(frmP, objIndex) == frmGraffitiStateObj)
		{
			FrmGetObjectBounds(frmP, objIndex, &objBounds);
			objBounds.topLeft.y += offsetY;
			objBounds.topLeft.x += offsetX;
			FrmSetObjectBounds(frmP, objIndex, &objBounds);
			break;
		}
	}

	// keep the window bounding rect.
	gCurrentWinBounds = eventP->data.winResized.newBounds;

justDefineLayout:

	// Build the layout of the day, do not generate update event
	DayViewRefreshDisplay(false, false);

	if (restoreEditState)
	{
		DayEditPosition = editPosition;
		DayEditSelectionLength = editSelectionLength;

		// If there was a selection, get it back !
		DayViewHighlightSelection();
	}
}

/***********************************************************************
 *
 * FUNCTION:    	DayViewUpdateDisplay
 *
 * DESCRIPTION: 	This routine updates the display of the day View.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/12/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *
 ***********************************************************************/
static void DayViewUpdateDisplay (void)
{
	FormType*	activeFormP;

	PIMAppProfilingBegin("DayView Drawings");

	activeFormP = FrmGetFormPtr (DayView);
	FrmDrawForm (activeFormP);

	DayViewDrawTimeBars (activeFormP);
	DayViewDrawOverlappingBars (activeFormP);
	DayViewDrawVerticalBarAdapter(activeFormP);

	DayViewFocusTableRegainFocus();


	PIMAppProfilingEnd();
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawCallBack
 *
 * DESCRIPTION: 	Callback for WinRectangleInvalidateFunc().
 *					(see Window.h)
 *
 * PARAMETERS:  ->	cmd:
 *				->	window:
 *				->	diryRect:
 *				->	state:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	06/16/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewDrawCallBack (int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	if (cmd == winInvalidateDestroy)
		return true;

	if (ApptDB && sApptsH)
	{
		DayViewUpdateDisplay ();
		DayViewRestoreEditState ();
	}

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewDrawInvalidate
 *
 * DESCRIPTION: 	Direct Draw Invalidated Zone.
 *
 * PARAMETERS:  ->	dirtyRect: zone to redraw.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	06/16/03	Initial revision.
 *
 ***********************************************************************/
static void DayViewDrawInvalidate(RectangleType* dirtyRectP)
{
	RectangleType dirtyRect;

	if (dirtyRectP == NULL)
	{
		WinGetBounds(gWinHdl, &dirtyRect);
	}
	else
		dirtyRect = *dirtyRectP;

	// Transparency tips:
	// due to transparency we must not use drawing callback but post update event
	// invalidating rectangles
	//WinInvalidateRectFunc(gWinHdl, &dirtyRect, DayViewDrawCallBack, NULL);

	WinInvalidateRect(gWinHdl, &dirtyRect);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewSelectCategory
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	07/29/02	Initial revision, added category support.
 *	PPL 10/13/03	Fixes for being update based:
 *					- DayViewRestoreEditState is replaced by
 *					DayViewDrawInvalidate() which is posting winUpdates.
 *
 ***********************************************************************/
static void DayViewSelectCategory(void)
{
	FormType* 		frmP;
	CategoryID* 	newCatSelectionP = NULL;
	uint32_t		newCatSelectionNum = 0;
	uint32_t		currentRowID = dbInvalidRowID;
	Boolean 		categoryEdited;

	// Close all opened cursors (main cusor)

	// we have to close cursors as Datebook is using cahed cursors
	// unfortunaltely when a record is in such loaded in such a cache
	// when CatMgrSelectFilter is called, it cannot update records.

	// For DayView since we have the dayView Table we also need to save
	// and restore the current rowID of the Cursor and to release the focus
	// on the currently selected table row and then reatore it after the
	// cursor is reopened calling DayViewLoadApptsFromDB().
	if (gApptCursorID != dbInvalidCursorID)
		DbCursorGetCurrentRowID(gApptCursorID, &currentRowID);

	frmP = FrmGetFormPtr(DayView);


	// Clear edit state if it was already active, it will be restored below
	ClearEditState();
	//FrmSetFocus (frmP, noFocus); Test

	// close cusor...
	ApptCloseCursor(&gApptCursorID);

	categoryEdited = CatMgrSelectFilter(ApptDB, frmP, DayCategoryTrigger,
		DateBkCategoriesName, DayCategoryList, DateBkCurrentCategoriesP,
		DateBkCurrentCategoriesCount, &newCatSelectionP, &newCatSelectionNum,
		true, NULL);

	// re-opens the global cursor
	DayViewLoadApptsFromDB();

	// Restore previously selected table row
	if (currentRowID != dbInvalidRowID)
	{
		DbCursorMoveToRowID(gApptCursorID, currentRowID);
		DayEditPosition = mostRightFieldPosition;
		gItemSelected = true;
	}

	if (categoryEdited)
	{
		// Exit from edit state
		DayViewClearEditState ();

		// Update current categories
		ChangeCurrentCategories(newCatSelectionNum, newCatSelectionP);

		// Free list allocated by CatMgrSelectFilter
		CatMgrFreeSelectedCategories(ApptDB, &newCatSelectionP);

		// To reopen the cursor and refresh the view
		DayViewRefreshDisplay(true, true);

		// Draw the new category, same day
		DayViewDrawDate (Date, true);
	}
	else
	{
		DayViewRestoreEditState();
		DayViewDrawInvalidate(NULL);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDayViewArrangeDayTabLabels
 *
 * DESCRIPTION: 	This routine initializes the Book's Gadget
 *
 * PARAMETERS:  ->	frm:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	11/15/02	Initial revision.
 *
 ***********************************************************************/
static void PrvDayViewArrangeDayTabLabels(void)
{
	char		label[DayBookLabelCharsNum] = "";
	uint16_t	listId = DayViewSundayWeekId;
	uint32_t	index;

	if (StartDayOfWeek == monday)
	{
		// first week day is Monday
		listId = DayViewMondayWeekId;
	}

	for (index = 0 ; index < daysInWeek ; ++index)
	{
		SysStringByIndex(gApplicationDbP, listId,  (uint16_t) index, label, DayBookLabelCharsNum);
		strcpy( sWeekTabs[index].name, label );
	}
}


/***********************************************************************
 *
 * FUNCTION:   		DayViewDeactivateBookTab
 *
 * DESCRIPTION: 	This routine is a callback for BooksLib Module.
 *					Called when a tab is sent in bakground.
 *					The Tab is deactivated.
 *
 * PARAMETERS:  ->	bookP:
 *				->	tabId:
 *				->	tabIndex:
 *				->	userData:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 *	HISTORY:
 *	 PPL 01/08/03	creation.
 *
 ***********************************************************************/
static Boolean DayViewDeactivateBookTab(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData)
{
	DayViewClearEditState();

	// reset last selected navigation row.
	sDayViewLastRowFocused = tblUnusableRow;

	return true;
}


 /***********************************************************************
 *
 * FUNCTION:   		DayViewActivateBookTab
 *
 * DESCRIPTION: 	This routine is a callback for BooksLib Module.
 *					Called when a tab is sent in foreground.
 *					The Tab is activated.
 *
 * PARAMETERS:  ->	frm - pointer to the day view form.
 *
 * RETURNED:    	nothing.
 *
 * HISTORY:
 *	PPL	01/08/03	Initial revision.
 *
 ***********************************************************************/
static void DayViewActivateBookTab(struct BookTag* bookP, uint16_t tabId, uint16_t tabIndex, uint32_t userData)
{
	DayViewDayOfWeekSelected (tabId);
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewInitBook
 *
 * DESCRIPTION: 	This routine initializes the Book's Gadget.
 *
 * PARAMETERS:  ->	frm - pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * Note:			None.
 *
 * HISTORY:
 *	PPL	11/15/02 	Initial revision.
 *
 ***********************************************************************/
 void DayViewInitBook(FormPtr frmP, uint16_t dayOfWeek)
 {
 	uint16_t	index;
 	status_t	err;

	// If the start-day-of-week is monday rearrange the labels on the
	// days-of-week push buttons.
	PrvDayViewArrangeDayTabLabels();

 	sWeekBook = BookCreate(frmP, DayBookTabsGadget, DayBookBodyGadget , DayBookMinTabWidth, DayBookMaxTabWidth, daysInWeek, kBookStandardLookAndFeelNoScroll);

	BookSetEnableUpdate(sWeekBook, false);

	// BookSetTabsParametrics sends invalidates -
	BookSetTabsParametrics(sWeekBook, 0, 3, 1, 2);

	for (index = 0 ;  index < daysInWeek ; index++)
		err  = BookAddTab(sWeekBook,  sWeekTabs[index].name, NULL, 0, 0,  sWeekTabs[index].id , kBookTabMaxIndex, 0, DayViewActivateBookTab, DayViewDeactivateBookTab, DayViewDrawCallBack);

	BookSetActiveTabId(sWeekBook, DayBookDOW1TabId + dayOfWeek, false);

	BookSetEnableUpdate(sWeekBook, true);
 }

 /***********************************************************************
 *
 * FUNCTION:    	DayViewInitLunarCalendar
 *
 * DESCRIPTION: 	This routine initializes the "Day View" Lunar of the
 *              	Datebook application.
 *
 * PARAMETERS:  ->	frm:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	03/10/04	from PalmOS 5.4
 *
 ***********************************************************************/
void DayViewInitLunarCalendar(FormPtr formP)
{
 	// Display the Lunar View push button if we've got support for it.
	 if (DateSupportsLunarCalendar())
	 {
		 FrmShowObject (formP, FrmGetObjectIndex(formP, DayMonthLunarViewButton));
	 }
 }

/***********************************************************************
 *
 * FUNCTION:    	DayViewInit
 *
 * DESCRIPTION: 	This routine initializes the "Day View" of the
 *              	Datebook application.
 *
 * PARAMETERS:  ->	frm:					pointer to the day view form.
 *				->	loadDBandDefineLayout:	should we load backgrounds DB
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	art	6/12/95		Initial revision.
 *	PLe	01/01/03	New OS6 revision.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *
 ***********************************************************************/
static void DayViewInit (FormPtr frmP, Boolean loadDBandDefineLayout)
{
	uint16_t month;
	uint16_t day;
	uint16_t year;
	uint16_t dayOfWeek;
	uint16_t row;
	uint16_t rowsInTable;
	uint16_t gadgetIndex;
	DmResourceID bitmapID = 0;
	TablePtr table;

	// Since we have a background image, we need to have transparent widgets
	FrmSetTransparentObjects(frmP, true);

	DateBackgroundGetHighlightColor(&sBackgroundHighlight, Date.month);

	DateBackgroundGetBitmapID(&bitmapID, Date.month);

	sBackgroundMonth = Date.month;

	sGadgetCallbackData.color = sBackgroundHighlight;
	sGadgetCallbackData.formP = frmP;
	sGadgetCallbackData.bitmapID = bitmapID;
	sGadgetCallbackData.whiteRect.topLeft.x =  0;
	sGadgetCallbackData.whiteRect.topLeft.y =  30;
	sGadgetCallbackData.whiteRect.extent.x =  160;
	sGadgetCallbackData.whiteRect.extent.y =  112;

	gadgetIndex = FrmGetObjectIndex(frmP, DayBackgroundGadget);

	// Save some info we will need in our callback
	FrmSetGadgetData(frmP, gadgetIndex, &sGadgetCallbackData);

	// Set up the callback for the title bar graphic
	FrmSetGadgetHandler(frmP, gadgetIndex, DateBackgroundGadgetHandler);

	// reset current visual status globale
	CurrentRecordVisualStatus = PrivateRecordVisualStatus;

	// retrieve some palette colors, at least for Day View Icon drawing.
	UIColorGetTableEntryRGB(UIObjectForeground,&sObjectInkColor);
	UIColorGetTableEntryRGB(UIFieldBackground, &sObjectFillColor);
	UIColorGetTableEntryRGB(UIObjectSelectedForeground,&sObjectSelectedInkColor);
	UIColorGetTableEntryRGB(UIObjectSelectedFill, &sObjectSelectedFillColor);

	// Set the color of the lines in the text fields
 	UIColorSetTableEntry(UIFieldTextLines, &sBackgroundHighlight);

	// Get the location field max size (Doing this here avoids to make
	// it in DetailsInit() which is slow enough. Remove 1 to handle the
	// ending '\0' character.
	gLocationFieldMaxLength = (int32_t) ApptGetSchemaColumnMaxSize(ApptDB, EVENT_LOCATION_ID) - 1;

	// Same for description field
	sDescriptionFieldMaxLength = (int32_t) ApptGetSchemaColumnMaxSize(ApptDB, EVENT_DESCRIPTION_ID) - 1;

	// Allocate the description holder
	sDescriptionNotEditedHolderH = MemHandleNew(sDescriptionFieldMaxLength+1);
	sDescriptionEditedHolderH = MemHandleNew(sDescriptionFieldMaxLength+1);

	// Get the day we're displaying.
	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	// get the correct day-of-week rank.
	dayOfWeek = DateUtilsGetDayOfWeek (month, day, year);

	// Init Day View Tabs and its Internal DataStructure
 	DayViewInitBook (frmP, dayOfWeek);

	// Lunar Calendar
	DayViewInitLunarCalendar(frmP);

	// Highlight the Day View push button.
	FrmSetControlGroupSelection (frmP, DayViewGroup, DayDayViewButton);

	// Initialize the table used to display the day's appointments.
	table = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));

	rowsInTable = sMaxNumAppts = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle (table, row, timeBarColumn, customTableItem);
		TblSetItemStyle (table, row, timeColumn, customTableItem);
		TblSetItemStyle (table, row, descColumn, textTableItem);
		TblSetRowUsable (table, row, false);
	}

	TblSetColumnUsable (table, timeBarColumn, true);
	TblSetColumnUsable (table, timeColumn, true);
	TblSetColumnUsable (table, descColumn, true);

	TblSetColumnMasked (table, descColumn, true);

	TblSetColumnSpacing (table, timeBarColumn, spaceAfterTimeBarColumn);
	TblSetColumnSpacing (table, timeColumn, spaceAfterTimeColumn);

	TblSetColumnEditIndicator (table, timeBarColumn, false);

	// Set the callback routines that will load and save the
	// description field.
	TblSetLoadDataProcedure (table, descColumn, DayViewGetDescriptionCallback);
	TblSetSaveDataProcedure (table, descColumn, DayViewSaveDescriptionCallback);


	// Set the callback routine that draws the timebars field.
	TblSetCustomDrawProcedure (table, timeBarColumn, DayViewDrawBarsCallback);

	// Set the callback routine that draws the time field.
	TblSetCustomDrawProcedure (table, timeColumn, DayViewDrawTimeCallback);

	// Set the callback routine that draws the note, alarm, and repeat icons.
	TblSetCustomDrawProcedure (table, descColumn, DayViewDrawIconsAndOverlappedEvtsCallback);

	// By default the list view assume no time bar are displayed.
	sTimeBarColumns = 0;

	// Set title to avoid default title displaying from resources
	DayViewSetTitle(frmP);

	// Initial miscellaneous global variables.
	TimeDisplayed = false;
	sCurrentFieldOffset = 0;

	// Build the layout of the day, do not generate update event
	if (loadDBandDefineLayout)
		DayViewRefreshDisplay(true, false);

	DayViewRestoreEditState ();

	// reset last selected navigation row.
	DayViewFocusSetRowToSelect(tblUnusableRow);
	DayViewFocusHowToSelect(kFocusLastSelectionMode);

	// reset last focused object
	sDayViewFocusedItemID = frmInvalidObjectId;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandleTblExit
 *
 * DESCRIPTION: 	Called when the user moved the pen from a row to
 *					another - Force editing on the row selected
 *					on pen down.
 *
 * PARAMETERS:  -> 	eventP : The event pointer got from event handler.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PLe	10/10/02	Initial revision.
 *
 ***********************************************************************/
static void DayViewHandleTblExit(EventType * eventP)
{
	uint32_t 	recRowID;

	recRowID = TblGetRowData (eventP->data.tblExit.pTable, eventP->data.tblExit.row);

	DayViewRefreshDisplay(true, true);

	if (recRowID != dbInvalidRowID)
	{
		DbCursorMoveToRowID(gApptCursorID, recRowID);
		gItemSelected = true;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewFinalize
 *
 * DESCRIPTION: 	This routine finalizes before exit the "Day View"
 *					of the Datebook application.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PLe	10/10/02	Initial revision.
 *
 ***********************************************************************/
static void DayViewFinalize(FormType* formP)
{
	TablePtr	tableP;

	if (formP)
	{
		// Reset Table Callback
		tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));

		// If necessary, release the focus before freeing sApptsH.  Releasing the focus causes some
		// data to be saved, and that action depends on sApptsH.  If the data isn't
		// saved now, trying to save it later will access a NULL sApptsH.
		TblReleaseFocus(tableP);

		//if (frmP)
		//	FrmSetFocus(frmP, noFocus);

		TblSetLoadDataProcedure (tableP, descColumn, NULL);
		TblSetSaveDataProcedure (tableP, descColumn, NULL);

		TblSetCustomDrawProcedure (tableP, timeBarColumn, NULL);
		TblSetCustomDrawProcedure (tableP, timeColumn, NULL);
		TblSetCustomDrawProcedure (tableP, descColumn, NULL);
	}

	// free allocated memory
	MemHandleFree (sDescriptionNotEditedHolderH);
	sDescriptionNotEditedHolderH = NULL;

	MemHandleFree (sDescriptionEditedHolderH);
	sDescriptionEditedHolderH = NULL;

	MemHandleFree (sApptsH);
	sApptsH = NULL;

	// Also free sApptsOnlyH if necessary (was allocated in last call to DayViewLoadApptsFromDB)
	if (sApptsOnlyH != NULL)
	{
		MemHandleFree(sApptsOnlyH);
		sApptsOnlyH = NULL;
	}
}


/***********************************************************************
 *
 * FUNCTION:		DayViewTableValidRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	row:		row index to select
 *				->	focusing:	should we focus
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFE 03/29/04	Initial Revision.
 *	PPL 06/16/04	Add PalmSOurce One-Handed specification.
 *	PPL	08/26/04	fix the test to be >= rather than just > to 0
 *					for the row.
 *
 ***********************************************************************/
static Boolean DayViewTableValidRow(TablePtr tableP, int16_t row)
{
	int16_t	lastNum;

	lastNum = TblGetLastUsableRow(tableP);

	return (Boolean) ((row >= 0) && (row <lastNum));
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSetRingOnEntireTable
 *
 * DESCRIPTION: 	Set the focus ring on entire DayView table
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 07/13/04	Creation.
 *
 ***********************************************************************/
static void DayViewFocusSetRingOnEntireTable(void)
{
	RectangleType	bounds;

	TblGetBounds(UtilitiesGetObjectPtr(DayView,DayTable), &bounds) ;
	FrmSetFocusHighlight(gWinHdl, &bounds, 0);
}



/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSetRingTableRow
 *
 * DESCRIPTION: 	Set the focus ring on entire DayView table
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	 PPL 07/13/04	 Creation.
 *
 ***********************************************************************/
static void DayViewFocusSetRingTableRow(void)
{
	RectangleType	bounds;
	RectangleType 	ItemBounds;
	TableType*		tableP;

	tableP = UtilitiesGetObjectPtr(DayView, DayTable);

	TblGetBounds(tableP, &bounds );

	if (gItemSelected)
	{
		TblGetItemBounds(tableP, gSelectedRow, descColumn, &ItemBounds );

		bounds.topLeft.y = ItemBounds.topLeft.y;
		bounds.extent.y = ItemBounds.extent.y;
	}


	FrmSetFocusHighlight(gWinHdl, &bounds, 0);
}


/***********************************************************************
 *
 * FUNCTION:		DayFocusClearTableRing
 *
 * DESCRIPTION: 	Set the focus ring on the table or the row
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	 PPL 07/13/04	 Creation.
 *
 ***********************************************************************/
static void DayFocusClearTableRing(void)
{
	FrmClearFocusHighlight();
}

/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSetNavState
 *
 * DESCRIPTION: 	Set navigation status depending on the internal state.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 07/21/04	Creation.
 * 	PPL 07/21/04	Add PalmSource OneHanded Specifications.
 *
 ***********************************************************************/
static void DayViewFocusSetNavState(void)
{
	FormType*				formP;
	FrmNavStateFlagsType 	navStateFlags;
	status_t				err;


	formP = FrmGetFormPtr(DayView);

	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			err = FrmSetNavState (formP, 0);
			break;

		case kFocusDayTableIsFocused:
			navStateFlags = kFrmNavStateFlagsObjectFocusMode;
			err = FrmSetNavState (formP, navStateFlags);
			break;

		case kFocusDayTableRowIsFocused:
			navStateFlags = kFrmNavStateFlagsObjectFocusMode | kFrmNavStateFlagsInteractionMode;
			err = FrmSetNavState (formP, navStateFlags);
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFovusTableRegainFocus
 *
 * DESCRIPTION: 	Regain focus on Table.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Index of focused widget.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 07/16/04	Creation.
 * 	PPL 07/16/04	Add PalmSource OneHanded Specifications.
 *
 ***********************************************************************/
static uint16_t DayViewFocusTableRegainFocus(void)
{
	FormType*				formP;
	TableType*				tableP;
	FrmNavStateFlagsType 	navStateFlags;
	uint16_t				focus	= noFocus;
	uint16_t				tableIndex = noFocus;
	int16_t					selectedRow;
	int16_t					selectedCol;
	status_t				err;

	formP = FrmGetFormPtr(DayView);
	tableIndex  = FrmGetObjectIndex(formP, DayTable);
	focus = FrmGetFocus(formP);
	err = FrmGetNavState (formP, &navStateFlags);

	if (gItemWasSelected)
	{

		// details dialog was opened
		// give focus back to the row that was selected
		// given its row id as its position might have change
		// on the same day
		// the Database currsor will have it scurrent item on this item
	}

	if (focus == noFocus
		&& sDayTableFocusState != (int16_t) noFocus)
	{
		navStateFlags = kFrmNavStateFlagsObjectFocusMode;
		FrmSetNavState (formP, navStateFlags);
		FrmSetFocus(formP, tableIndex);
		focus = FrmGetFocus(formP);
	}


	if ((navStateFlags & kFrmNavStateFlagsObjectFocusMode)
		&& focus != noFocus)
	{
		if (focus == tableIndex)
		{
			switch(sDayTableFocusState)
			{
				case kFocusDayNoFocus:
					tableP = FrmGetObjectPtr(formP, tableIndex);

					TblGetSelection(tableP, &selectedRow, &selectedCol);

					if (selectedRow != (int16_t) noFocus)
					{
						DayViewFocusSetRowToSelect(selectedRow);

						navStateFlags = kFrmNavStateFlagsObjectFocusMode | kFrmNavStateFlagsInteractionMode;
						FrmSetNavState (formP, navStateFlags);

						sDayTableFocusState = kFocusDayTableRowIsFocused;
						gItemWasSelected = true;
						DayViewFocusSetRingTableRow();
					}
					else
					{
						navStateFlags = kFrmNavStateFlagsObjectFocusMode;
						FrmSetNavState (formP, navStateFlags);

						sDayTableFocusState = kFocusDayTableIsFocused;
						DayViewFocusSetRingOnEntireTable();
					}
					break;


				case kFocusDayTableIsFocused:
					navStateFlags = kFrmNavStateFlagsObjectFocusMode;
					FrmSetNavState (formP, navStateFlags);

					DayViewFocusSetRingOnEntireTable();
					break;


				case kFocusDayTableRowIsFocused:
					navStateFlags = kFrmNavStateFlagsObjectFocusMode | kFrmNavStateFlagsInteractionMode;
					FrmSetNavState (formP, navStateFlags);

					DayViewFocusSetRingTableRow();
					break;
			}
		}
	}

	return focus;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewGetCurrentTimeRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:		Nothing.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLL 08/09/04	Initial Revision.
 *
 ***********************************************************************/
int16_t DayViewGetCurrentTimeRow(void)
{
	FormPtr						frmP;
	TablePtr					tableP;
	int16_t						lastUsableRow;
	ApptInfoPtr 				appts;
	int16_t 					apptIndex;
	int16_t						rowToSelect = tblUnusableRow;
	DateTimeType				dateTime;
	TimeType					currentTime;
	TimeType					startTime;

	frmP = FrmGetFormPtr(DayView);
	tableP = UtilitiesGetFormObjectPtr (frmP,  DayTable);

	lastUsableRow = TblGetLastUsableRow(tableP);

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);

	//just want the hour for current time, the search will be tuned later
	currentTime.hours =  (uint16_t) dateTime.hour;
	currentTime.minutes = (uint16_t) 0;

	appts = MemHandleLock (sApptsH);

	for (apptIndex = 0;  apptIndex <= sNumAppts; apptIndex += 1)
	{
		startTime = appts[apptIndex].startTime;

		if (TimeToInt(startTime) >= TimeToInt(currentTime) )
		{
			rowToSelect = apptIndex;
			break;
		}
	}

	MemHandleUnlock (sApptsH);

	return rowToSelect;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSelectGetNextUsableRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	rowDirection: 	direction of move.
 *				-> 	modifiers:		event modifiers.
 *				-> 	lastRow:		row index for last reow selected.
 *				->	newRowID:		row ID of record to select
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFE 03/29/04	Initial Revision.
 *	PPL 06/16/04	Add PalmSOurce One-Handed specification.
 *
 ***********************************************************************/
static int16_t DayViewFocusSelectGetNextUsableRow(int16_t rowDirection, uint16_t modifiers, int16_t lastRowSelected, uint32_t* newRowID)
{
	FormPtr						frmP;
	TablePtr					tableP;
	int16_t						lastUsableRow;
	int16_t						row;
	int16_t						step = 1;
	int16_t						startRow = 0;
	int16_t						startRowInc = 0;
	int16_t						column;
	ApptInfoPtr 				appts;
	MidnightOverlapStateType 	overState;
	Boolean 					belongingToCurrCat;
	int16_t 					apptIndex;
	int16_t						rowToSelect = tblUnusableRow;
	uint32_t					recRowID;

	frmP = FrmGetFormPtr(DayView);
	tableP = UtilitiesGetFormObjectPtr (frmP,  DayTable);

	lastUsableRow = TblGetLastUsableRow(tableP);

	// No usable row, return
	if (lastUsableRow == tblUnusableRow)
		return tblUnusableRow;

	switch (rowDirection)
	{
		case kFocusPreviousEventSlot:

			if (DayViewTableValidRow(tableP, lastRowSelected))
				startRow = lastRowSelected;
			else startRow = 0;

			startRowInc = -1;
			step = -1;
			break;

		case kFocusNextEventSlot:
			if (DayViewTableValidRow(tableP, lastRowSelected))
				startRow = lastRowSelected;
			else startRow = 0;

			startRowInc = +1;
			step = +1;
			break;

		case kFocusSelectionMode:

			switch (sDayViewSelectionMode)
			{
				case kFocusLastSelectionMode:
					// actually I should restore the focus on the
					// current record in the meening of database cursor
					if (DayViewTableValidRow(tableP, lastRowSelected))
						startRow = lastRowSelected;
					else startRow = 0;

					rowDirection = 0;
					startRowInc = 0;
					step = 1;
					break;

				case kFocusCurrentTimeEventSlotMode:
					startRow =  DayViewGetCurrentTimeRow();
					if (startRow != tblUnusableRow)
					{
						rowDirection = kFocusNextEventSlot;
						startRowInc = 0;
						step = 1;
						break; // this is not an error
					}

					//else we treat this as kFocusFirstTime
					//	case kFocusFirstTime: must follow

				case kFocusFirstTimeMode:
					// first visible row in table
					startRow = 0;
					rowDirection = kFocusNextEventSlot;
					startRowInc = 0;
					step = 1;
					break;

				case kFocusLastTimeMode:
					// last visible row in table
					startRow = lastUsableRow;
					rowDirection = kFocusPreviousEventSlot;
					startRowInc = 0;
					step = -1;
					break;

				case kFocusTableSelectionMode:
					TblGetSelection(tableP, &startRow, &column);
					rowDirection = 0;
					startRowInc = 0;
					step = 1;
					break;
			}
			break;

		default:
			startRow = tblUnusableRow;
	}

	if (startRow == tblUnusableRow
	|| (!startRow && (rowDirection < 0))
	|| ((startRow == lastUsableRow) && (rowDirection > 0)) )
		return tblUnusableRow;

	appts = MemHandleLock (sApptsH);

	for (row = startRow + startRowInc;  (row >= 0) && (row <= lastUsableRow) ; row += step)
	{
		apptIndex = TblGetRowID (tableP, row);

		// Don't take the rowID from TblGetRowData (table, row) because
		// second parts of overlapping events have an invalid rowID (0)
		// see DayViewCheckForNonEditableRows().
		recRowID = appts[apptIndex].rowID;

		if (recRowID == dbInvalidRowID)
			continue;

		overState = appts[apptIndex].overlapState;
		belongingToCurrCat = appts[apptIndex].belongingToCurrCat;

		if (belongingToCurrCat && (overState != overlapFirstPartCommonRow) && (overState != overlapScndPart))
		{
			rowToSelect = row;

			if (newRowID)
				*newRowID = recRowID;

			break;
		}
	}

	MemHandleUnlock(sApptsH);

	return rowToSelect;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSetRowToSelect
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	row: row that should be focused next time
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 07/21/04	Add PalmSOurce One-Handed specification.
 *
 ***********************************************************************/
void DayViewFocusHowToSelect(int16_t state)
{
	sDayViewSelectionMode = state;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSetRowToSelect
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	row: row that should be focused next time
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 07/21/04	Add PalmSOurce One-Handed specification.
 *
 ***********************************************************************/
void DayViewFocusSetRowToSelect(int16_t row)
{
	sDayViewLastRowFocused = row;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSelectRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	rowDirection: move direction
 *					-1: up cell, scroll up , up widget
 *					 0: first selection
 *					+1: down cell, scroll down, down  widget
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFE 03/29/04	Initial Revision.
 *	PPL 06/16/04	Add PalmSOurce One-Handed specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusSelectRow(int16_t rowDirection, uint16_t modifiers)
{
	FormPtr 			formP;
	TablePtr 			tableP;
	FieldPtr			fieldP;
	uint32_t			rowID;
	WinDirectionType	direction;
	Boolean				result = true;

	if (!rowDirection && gItemSelected)
		goto ExitDayViewFocusSelectRow;

	// Select next usable row.((existing event only)
	sDayViewLastRowFocused = DayViewFocusSelectGetNextUsableRow(rowDirection, modifiers, sDayViewLastRowFocused, &rowID);

	if (sDayViewLastRowFocused == (int16_t) tblUnusableRow)
	{
		result = false;

		if (rowDirection)
		{
			direction = rowDirection > 0 ? winDown : winUp;

			if (DayViewCanScrollScrollers(direction))
			{
				DayViewScroll (direction);
				result = true;
			}
		}

		goto ExitDayViewFocusSelectRow;
	}

	formP = FrmGetFormPtr(DayView);
	tableP = UtilitiesGetFormObjectPtr (formP, DayTable);

	if (DayViewCheckForNonEditableRows(tableP, sDayViewLastRowFocused))
	{
		result = true;
		goto ExitDayViewFocusSelectRow;
	}

	DayViewClearEditState();


	FrmSetFocus(formP, FrmGetObjectIndex (formP, DayTable));
	TblSetSelection(tableP, sDayViewLastRowFocused, descColumn);

	if (!TblRowMasked(tableP,sDayViewLastRowFocused)
	&& TblGetItemStyle(tableP, sDayViewLastRowFocused, descColumn) != customTableItem)
	{
		TblGrabFocus(tableP, sDayViewLastRowFocused, descColumn);
		fieldP = TblGetCurrentField(tableP);

		if (fieldP)
			FldGrabFocus(fieldP);
	}

	DayViewSelectItem (tableP, sDayViewLastRowFocused, descColumn);
	gItemWasSelected = true;

ExitDayViewFocusSelectRow:
	return result;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusSelectDatabaseRowID
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	rowID: database row ID to select.
 *
 * RETURNED:		a Boolean that tell whether or not the record was
 *					actually selected in the table.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 08/09/04	Initial Revision.
 *
 ***********************************************************************/
static Boolean DayViewFocusSelectRowID(uint32_t rowID)
{
	FormType*	formP;
	TableType* 	tableP;
	uint32_t	currentRowID;
	int16_t		row = tblUnusableRow;
	Boolean		found = false;

	formP = FrmGetFormPtr (DayView);
	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));

	if (DbCursorGetCurrentRowID(gApptCursorID, &currentRowID) >= errNone)
	{
		found = TblFindRowData (tableP, currentRowID, &row);

		if (found)
		{
			DayViewFocusSetRowToSelect( row);
			DayViewFocusHowToSelect(kFocusLastSelectionMode);

	 		DayViewFocusSetNavState();
	 		DayViewFocusTableRegainFocus();
	 	}
	}

	return found;
}


/***********************************************************************
 *
 * FUNCTION:		DayFocusActivate
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *
 ***********************************************************************/
static void DayViewFocusReset(void)
{
	sDayTableFocusState = kFocusDayNoFocus;
}


/***********************************************************************
 *
 * FUNCTION:		DayFocusLost
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *
 ***********************************************************************/
static void DayViewFocusLost(void)
{
	// here we can't do anything that could give the focus back to the table
	DayViewFocusReset();
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusTake
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *
 ***********************************************************************/
static void DayViewFocusTake(void)
{
	FormType*	formP;



	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			DayViewClearEditState();
			sDayTableFocusState = kFocusDayTableIsFocused;
			break;

		case kFocusDayTableIsFocused:
			break;

		case kFocusDayTableRowIsFocused:
			break;

		case kFocusDayDetailsOpened:
			DayViewClearEditState();
			sDayTableFocusState = kFocusDayTableRowIsFocused;
			gItemWasSelected = true;
			break;

	}

	formP = FrmGetFormPtr(DayView);
	FrmSetFocus(formP, FrmGetObjectIndex(formP, DayTable));

	DayViewFocusSetNavState();
	DayViewFocusTableRegainFocus();
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusFirst
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	modifiers: event modifiers.
 *
 * RETURNED:		a row was focused when true.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusFirst(uint16_t modifiers)
{
	return DayViewFocusSelectRow(kFocusSelectionMode, modifiers);
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusUpDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->  focus:  	Current form focused widget index.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusUpDayCell(uint16_t focus, uint16_t modifiers)
{
	Boolean 	handled = false;


	if (focus == noFocus)
	{
		// Scroll Up first
		if (DayViewCanScrollScrollers (winUp))
		{
			DayViewScroll(winUp);

			return true;
		}
		// Focus Category trigger
		// on left key press when focusing is not enabled
		// focus Goto button
		DayViewFocusLost();
		FrmNavObjectTakeFocus(FrmGetFormPtr(DayView), DayCategoryTrigger);
		handled = true;
		return handled;
	}
	else
	{
		switch ( sDayViewFocusedItemID)
		{
			// if any of item below the table is owning the focus
			// we prepare to focus table form the bottom
			case DayDayViewButton:
			case DayWeekViewButton:
			case DayMonthViewButton:
			case DayMonthLunarViewButton:
			case DayAgendaViewButton:
			case DayNewButton:
			case DayDetailsButton:
			case DayGoToButton:
			case DayUpButton:
			case DayDownButton:
				DayViewFocusHowToSelect(kFocusLastTimeMode);
		}
	}


	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			break;


		case kFocusDayTableIsFocused:
			// Scroll Up first
			if (DayViewCanScrollScrollers (winUp))
			{
				DayViewScroll(winUp);

				handled = true;
				break;
			}

			//we are exiting table
			DayViewClearEditState();

			// goto to New Button
			DayViewFocusLost();
			FrmNavObjectTakeFocus(FrmGetFormPtr(DayView), DayNextWeekButton);
			handled = true;
			break;


		case kFocusDayTableRowIsFocused:
			// try to select previous (above) existing event
			handled = DayViewFocusSelectRow(kFocusPreviousEventSlot, modifiers);
			if (!handled)
			{
				//we are exiting table
				DayViewClearEditState();

				// goto to New Button
				DayViewFocusLost();
				FrmNavObjectTakeFocus(FrmGetFormPtr(DayView), DayNextWeekButton);
				handled = true;
			}
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusDownDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->  focus:  	Current form focused widget index.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusDownDayCell(uint16_t focus, uint16_t modifiers)
{
	Boolean handled = false;


	if (focus == noFocus)
	{
		if (DayViewCanScrollScrollers (winDown))
		{
			DayViewScroll(winDown);

			return true;
		}

		// goto to New Button
		DayViewFocusLost();
		FrmNavObjectTakeFocus(FrmGetFormPtr(DayView), DayNewButton);
		handled = true;
		return handled;
	}
	else
	{
		switch ( sDayViewFocusedItemID)
		{
			// if any of item below the table is owning the focus
			// we prepare to focus table form the bottom
			case DayPrevWeekButton:
			case DayNextWeekButton:
			case DayBookTabsGadget:
				DayViewFocusHowToSelect(kFocusFirstTimeMode);
		}
	}


	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			break;


		case kFocusDayTableIsFocused:
			// Scroll down first
			if (DayViewCanScrollScrollers (winDown))
			{
				DayViewScroll(winDown);

				handled = true;
				break;
			}

			// if we did not scroll then go to to New Button
			DayViewFocusLost();
			FrmNavObjectTakeFocus(FrmGetFormPtr(DayView), DayNewButton);
			handled = true;
			break;


		case kFocusDayTableRowIsFocused:
			handled = DayViewFocusSelectRow(kFocusNextEventSlot, modifiers);

			if (!handled)
			{
				//we are exiting table
				DayViewClearEditState();

				// goto to New Button
				DayViewFocusLost();
				FrmNavObjectTakeFocus(FrmGetFormPtr(DayView), DayNewButton);
				handled = true;
			}

			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusRightDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->  focus:  	Current form focused widget index.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusRightDayCell(uint16_t focus, uint16_t modifiers)
{
	FormType*	formP;
	Boolean 	handled = false;


	if (focus == noFocus)
	{
		// goto to next day
		DayViewClearEditState ();
		DateAdjust (&Date, 1);
		DayViewDrawDate (Date, true);
		handled = true;
		return handled;
	}

	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			break;


		case kFocusDayTableIsFocused:
			// goto to next day
			DayViewClearEditState ();

			formP = FrmGetFormPtr(DayView);
			FrmSetFocus(formP, FrmGetObjectIndex(formP, DayTable));
			DayViewFocusSetNavState();

			DateAdjust (&Date, 1);
			DayViewDrawDate (Date, true);

			handled = true;
			break;


		case kFocusDayTableRowIsFocused:
			// removing focus from selected item
			DayViewClearEditState();

			// we are leaving the table
			DayViewFocusLost();

			// for Goto button
			formP = FrmGetFormPtr(DayView);
			FrmNavObjectTakeFocus(formP, DayGoToButton);
			handled = true;
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewFocusLeftDayCell
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->  focus:  	Current form focused widget index.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusLeftDayCell(uint16_t focus, uint16_t modifiers)
{
	FormType*	formP;
	Boolean 	handled = false;

	if (focus == noFocus)
	{
		// goto to previous day
		DayViewClearEditState ();
		DateAdjust (&Date, -1);
		DayViewDrawDate (Date, true);
		handled = true;
		return handled;
	}

	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			break;


		case kFocusDayTableIsFocused:
			// goto to previous day
			DayViewClearEditState ();

			formP = FrmGetFormPtr(DayView);
			FrmSetFocus(formP, FrmGetObjectIndex(formP, DayTable));
			DayViewFocusSetNavState();

			DateAdjust (&Date, -1);
			DayViewDrawDate (Date, true);
			handled = true;
			break;


		case kFocusDayTableRowIsFocused:
			// deselect selected item and gives focus back to the table
			DayViewClearEditState();

			formP = FrmGetFormPtr(DayView);
			FrmSetFocus(formP, FrmGetObjectIndex(formP, DayTable));
			DayViewFocusSetNavState();

			// day table is focused
			sDayTableFocusState  = kFocusDayTableIsFocused;
			gItemWasSelected = true;

			DayViewFocusSetNavState();
			DayViewFocusTableRegainFocus();

			handled = true;
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:		DayViewFocusNewCurrentTimeEvent
 *
 * DESCRIPTION: 	create a new event row at current time
 *					when table is empty and user press enter.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFE ????????	Creation.
 *	PPL 26/08/04	Add header.
 *
 ***********************************************************************/
static void DayViewFocusNewCurrentTimeEvent(void)
{
	uint32_t			newRowID;
	ApptDBRecordType 	newAppt;
	DateTimeType 		dateTime;
	status_t			err;

	if ((newRowID = DayViewNewAppointment (NULL, false)) == dbInvalidRowID)
		return;

	DbCursorMoveToRowID(gApptCursorID, newRowID);

	if (ApptGetRecord (ApptDB, gApptCursorID, &newAppt, DBK_SELECT_TIME) < errNone)
		return;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	newAppt.when.startTime.hours = dateTime.hour;
	newAppt.when.startTime.minutes = 0;
	TimSecondsToDateTime (TimGetSeconds () + hoursInSeconds, &dateTime);

	if (newAppt.when.date.day == dateTime.day)
	{
		newAppt.when.endTime.hours = dateTime.hour;
		newAppt.when.endTime.minutes = 0;
	}
	else
	{
		newAppt.when.endTime.hours = newAppt.when.startTime.hours;
		newAppt.when.endTime.minutes = 55;
	}

	newAppt.when.noTimeEvent = false;
	err = ApptChangeRecord (ApptDB, gApptCursorID, &newAppt, DBK_SELECT_TIME);
	ErrNonFatalDisplayIf(err < errNone, "Can't change record");
	ApptFreeRecordMemory (&newAppt);

	gItemSelected = true;
	
	// Reload the appointments from DB and redefine the layout if we have to switch
	DayViewRefreshDisplay(true, true);

	if (TblFindRowData (UtilitiesGetObjectPtr(DayView, DayTable), newRowID, &gSelectedRow))
	{
		DayViewFocusSetRowToSelect(gSelectedRow);
		DayViewFocusHowToSelect(kFocusLastSelectionMode);
		gSelectedColumn = descColumn;
	}

	sDayTableFocusState = kFocusDayTableRowIsFocused;
	gItemWasSelected = true;

	DayViewFocusSetNavState();
	DayViewFocusTableRegainFocus();
}

/***********************************************************************
 *
 * FUNCTION:		DayViewFocusEnter
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:		focus: curent form's widget focused.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 *	PPL 06/16/04	Add PalmSource OneHanded Specification.
 *
 ***********************************************************************/
static Boolean DayViewFocusEnter(uint16_t focus, uint16_t modifiers)
{
	FormType*				formP;
	FrmNavStateFlagsType 	navStateFlags = 0;
	Boolean 				handled = false;
	Boolean 				exitView = false;
	status_t				err;


	formP = FrmGetFormPtr(DayView);
	err = FrmGetNavState (formP, &navStateFlags);

	if (focus == noFocus && !navStateFlags)
	{
	 	if (DayViewFocusFirst(modifiers))
		{
			sDayTableFocusState = kFocusDayTableRowIsFocused;
			gItemWasSelected = true;

			DayViewFocusSetNavState();
			DayViewFocusTableRegainFocus();
	 	}
		else
		{
			DayViewFocusNewCurrentTimeEvent();
		}
		return true;
	}

	if (navStateFlags & kFrmNavStateFlagsObjectFocusMode)
	{
		if (focus == FrmGetObjectIndex(formP, DayDayViewButton))
		{
			FrmNavObjectTakeFocus(formP, DayTable);
			return true;
		}
	}

	switch(sDayTableFocusState)
	{
		case kFocusDayNoFocus:
			break;


		case kFocusDayTableIsFocused:
			// is there events on that day?
			// yes: then we select the first event
			// no: does nothing. no state change.

			// we should never give focus to an empty entry (cell)

			// first make attemps to change state to "row selected"

		 	if (DayViewFocusFirst(modifiers))
			{
				sDayTableFocusState = kFocusDayTableRowIsFocused;
				gItemWasSelected = true;

				DayViewFocusSetNavState();
				DayViewFocusTableRegainFocus();
		 	}
			else
			{
				DayViewFocusNewCurrentTimeEvent();
			}

		 	handled = true;
			break;


		 case kFocusDayTableRowIsFocused:

			gItemWasSelected = gItemSelected;

			sDayTableFocusState = kFocusDayDetailsOpened;

			DayViewFocusSetRowToSelect(gSelectedRow);
			DayViewFocusHowToSelect(kFocusLastSelectionMode);


			// release focus from currently selected item
//		 	DayViewClearEditState();

			// give up focus
			formP = FrmGetFormPtr(DayView);
			FrmSetFocus(formP, noFocus);

			DayViewFocusLost();

		 	// Opens Details dialog
			DoEventDetailsDialog();

			// focus will be given back later in kFocusDayTableRowIsFocused

		 	handled = true;
		 	break;

		  case kFocusDayDetailsOpened:
		  	sDayTableFocusState = kFocusDayTableRowIsFocused;

			DayViewFocusHowToSelect(kFocusLastSelectionMode);

			DayViewFocusSetNavState();
			DayViewFocusTableRegainFocus();
			handled = true;
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		DayViewHandleNavigation
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	modifiers:		Command mofifiers.
 *				-> 	chr:			Command code.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 * 	PPL 06/16/04	Add PalmSource OneHanded Specifications.
 *
 ***********************************************************************/
Boolean DayViewHandleNavigation( uint16_t	modifiers, wchar32_t chr )
{
	uint16_t	focus	= noFocus;
	Boolean 	handled 	= false;

	TraceOutput(TL(appErrorClass, "DayViewHandleNavigation start"));

	focus = DayViewFocusTableRegainFocus();

	if (focus == noFocus)
	{
		// reset focus when there' still no focused item after
		// calling DayViewFocusTableRegainFocus()
		sDayViewFocusedItemID = frmInvalidObjectId;
	}

	switch(chr)
	{
		case vchrRockerUp:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerUp"));
			handled = DayViewFocusUpDayCell(focus, modifiers);
			break;

		case vchrRockerDown:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerDown"));
			handled = DayViewFocusDownDayCell(focus, modifiers);
			break;

		case vchrThumbWheelBack:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelBack, wheel back not handled"));
			break;

		case vchrThumbWheelDown:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelDown as vchrRockerLeft"));

		case vchrRockerLeft:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerLeft"));
			// 5-way rocker left
			// move focused tab to left
			handled = DayViewFocusLeftDayCell(focus, modifiers);
			break;

		case vchrThumbWheelUp:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelUp as vchrRockerRight"));

		case vchrRockerRight:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerRight"));
			// 5-way rocker right
			// move focused tab to right
			handled = DayViewFocusRightDayCell(focus, modifiers);
			break;

		case vchrThumbWheelPush:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelPush as vchrRockerCenter"));

		case vchrRockerCenter:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerCenter"));
			// 5-way rocker center/press
			// active focused tab
			handled = DayViewFocusEnter(focus, modifiers);
			break;

		default:
			TraceOutput(TL( appErrorClass, "-------------> Not a 5-way or Wheel Navigation vchr"));
			break;
	}

	TraceOutput(TL(appErrorClass, "DayViewHandleNavigation exit"));
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandleKeyDownEvent
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	eventP:	a pointer to an EventType structure
 *
 * RETURNED:    	True if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PLe	01/01/03	Initial revision.
 *	PPL 10/13/03	Fixes for being update based:
 *					- DayViewRestoreEditState is replaced by
 *					DayViewDrawInvalidate() which is posting winUpdates.
 *	PPL 22/10/03	fixes to handle the first chat that is activating a field
 *					in the table when there's no active field.
 * 	PPL 06/16/04	Add PalmSource OneHanded Specifications.
 *
 ***********************************************************************/
static Boolean DayViewHandleKeyDownEvent (EventType* eventP)
{
	Boolean 		handled = false;
	DateType 		date;
	FormPtr 		formP;
	TablePtr 		tableP;
	FieldPtr		fieldP;
	uint32_t		newRowID;
	int16_t			row;
	wchar32_t		chr;
	uint16_t		modifiers;
	wchar32_t		upperChar = 0;
	size_t			length;


	chr 		= eventP->data.keyDown.chr;
	modifiers	= eventP->data.keyDown.modifiers;

	// Datebook key pressed?
	if (TxtCharIsHardKey(modifiers, chr))
	{
		DayViewClearEditState ();
		DateSecondsToDate (TimGetSeconds (), &date);

		if (DateToInt (date) != DateToInt (Date)
			|| ((modifiers & poweredOnKeyMask)))
		{
			DayViewDrawDate (date, true);
		}
		else
		{
			FrmGotoForm(gApplicationDbP, WeekView);
		}
		handled = true;
	}
	else if (TxtCharIsRockerKey(modifiers, chr) || TxtCharIsWheelKey(modifiers, chr))
	{
		handled = DayViewHandleNavigation(modifiers, chr);
	}
	else if (EvtKeydownIsVirtual(eventP))
	{
		switch (chr)
		{
			// Scroll up key pressed?
			case vchrPageUp:
			{
				// keep it commented out
				// inconsistent with 5 way

				//DayViewClearEditState ();
				//DateAdjust (&Date, -1);
				//DayViewDrawDate (Date, true);
				//handled = true;
				break;
			}

			// Scroll down key pressed?
			case vchrPageDown:
			{
				// keep it commented out
				// inconsistent with 5 way

				//DayViewScroll (winDown);
				//DayViewClearEditState ();
				//DateAdjust (&Date, 1);
				//DayViewDrawDate (Date, true);
				//handled = true;
				break;
			}

			// Send data key pressed?
			case vchrSendData:
			{
				if (gItemSelected)
				{
					formP = FrmGetFormPtr(DayView);
					tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));
					//TblReleaseFocus (tableP);
					DateSendRecord(ApptDB, gApptCursorID, exgBeamPrefix);
					DayViewDrawInvalidate(NULL);
				}
				else
				{
					FrmAlert(gApplicationDbP, SelectItemAlert);
				}

				handled = true;
				break;
			}

			// Confirm key pressed?
			case vchrConfirm:
			{
				gItemSelected = false;
				// Leave handled false so table releases focus.
				break;
			}

			default:
			{
				handled = false;
				break;
			}
		}
	}
	else if (TxtCharIsPrint (chr))
	{
		formP = FrmGetFormPtr(DayView);
		tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, DayTable));

		if ((! TblEditing(tableP)) && (!gItemSelected))
		{
			// If no item is selected and the character is display,
			// create a new appointment item.
			if (TxtCharIsDigit (chr))
			{
				DayViewNewAppointment (eventP, true);
				handled = true;
			}
			else if ((newRowID = DayViewNewAppointment (NULL, true)) != dbInvalidRowID)
			{
                // Give the focus to the new item
				TblFindRowData (tableP, newRowID, &row);
				TblGrabFocus(tableP, row, descColumn);

				DayViewFocusTableRegainFocus();

                // Change the char to uppercase
				length = TxtSetNextChar((char*)&upperChar, 0, chr);
				if (! TxtTransliterate((char*)&upperChar, length, (char*)&upperChar, &length, translitOpUpperCase))
					TxtGetNextChar((char*)&upperChar, 0, &eventP->data.keyDown.chr);

				fieldP = TblGetCurrentField(tableP);
				if (fieldP)
				{
					handled = FldHandleEvent(fieldP, eventP);
				}
			}
		}
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandlePenMoveEvent
 *
 * DESCRIPTION: 	Handles Pen Down events.
 *					Handles PenTracking for Day View Event icons.
 *
 * PARAMETERS:  ->	eventP:	a pointer to an EventType structure.
 *
 * RETURNED:   	 	True if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PLE	01/01/03	Initial revision.
 *	PPL 09/08/03	pen tracking support for Day view Icons.
 *
 ***********************************************************************/
static Boolean DayViewHandlePenDownEvent (EventType* eventP)
{
	FormPtr			frmP;
	TablePtr		tableP;

	// If the pen is not in any of the object of the view, take the
	// view out of edit mode.

	// Release edit state if the event couldn't be handled by the form
	if (! FrmHandleEvent (FrmGetActiveForm (), eventP))
	{
		frmP = FrmGetFormPtr(DayView);
		tableP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable));

		if (TblEditing(tableP))
		{
			DayViewClearEditState ();
			TblInvalidate(tableP);
			DayViewDrawInvalidate(NULL);
		}
	}

	// Don't let FrmHandleEvent get this event again.
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandlePenMoveEvent
 *
 * DESCRIPTION: 	Handle Pen Move Event.
 *					Handles PenTracking foe Day View Event icons.
 *
 * PARAMETERS:  ->	eventP:	a pointer to an EventType structure.
 *
 * RETURNED:   	 	True if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	09/08/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewHandlePenMoveEvent(EventType* eventP)
{
	Boolean handled = false;

	handled =  DayViewIconTrackingTrack(eventP);

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandlePenUpEvent
 *
 * DESCRIPTION: 	Handle Pen Up Event.
 *					Handles PenTracking foe Day View Event icons.
 *
 * PARAMETERS:  ->	eventP:	a pointer to an EventType structure.
 *
 * RETURNED:   	 	True if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPL	09/08/03	Initial revision.
 *
 ***********************************************************************/
static Boolean DayViewHandlePenUpEvent(EventType* eventP)
{
	Boolean handled = false;

	// Check if the time was displayed and the pen in the initial title rectangle
	if (TimeDisplayed
	&& RctPtInRectangle(eventP->screenX, eventP->screenY, &sTitleBounds))
		DayViewHideTime(true);

	// Check for pending Day View Icons tracking
	handled =   DayViewIconTrackingEnd(eventP);

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	DayViewHandleEvent
 *
 * DESCRIPTION: 	This routine is the event handler for the Day View
 *              	of the Datebook application.
 *
 * PARAMETERS:  ->	eventP: a pointer to an EventType structure.
 *
 * RETURNED:    	True if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	ART	6/12/95		Initial revision.
 *	PLE	01/01/03	New OS6 revision.
 *	PPL 10/13/03	Fixes for being update based:
 *					- Add handling for datebookRestoreSelection.
 *					- DayViewRestoreEditState is replaced by
 *					DayViewDrawInvalidate() which is posting winUpdates.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *
 ***********************************************************************/
Boolean DayViewHandleEvent (EventType* eventP)
{
	FormPtr 		frmP;
	FieldType* 		fldP;
	size_t 			startPos;
	size_t			endPos;
	uint32_t 		numLibs;
	Boolean 		handled = false;
	Boolean 		penDown;
	Coord 			x;
	Coord			y;
	uint32_t 		newRecRowID;
	DmOpenRef		uilibdbP = NULL;
	uint32_t		uiLibRefNum = 0;
	status_t		err;


	// Trace the event in debug mode
	DBGTraceEvent("DayView", eventP);

	switch (eventP->eType)
	{
		case nilEvent:
			// Check the draw window then redraw the title.
			if (WinGetDrawWindow() != gWinHdl)
				break;

			EvtGetPen(&x, &y, &penDown);

			if (penDown)
				DayViewShowTime ();
			else
				DayViewHideTime (false);

			break;


		case penDownEvent:
			handled = DayViewHandlePenDownEvent(eventP);
			break;


		case penMoveEvent:
			handled = DayViewHandlePenMoveEvent(eventP);
			break;


		case penUpEvent:
			handled = DayViewHandlePenUpEvent(eventP);
			break;


		case keyDownEvent:
			handled = DayViewHandleKeyDownEvent(eventP);
			break;


		case ctlEnterEvent:
			// Check for buttons that take us out of edit mode.

			switch (eventP->data.ctlEnter.controlID)
			{
				case DayNextWeekButton:
				case DayPrevWeekButton:
				case DayDayViewButton:
				case DayWeekViewButton:
				case DayMonthViewButton:
				case DayAgendaViewButton:
				case DayNewButton:
				case DayGoToButton:
				case DayUpButton:
				case DayDownButton:
					DayViewClearEditState ();
					break;
			}
			break;


		case ctlSelectEvent:
			// Handle the button that was selected.
			switch (eventP->data.ctlSelect.controlID)
			{
				case DayCategoryTrigger:
					DayViewSelectCategory ();
					handled = true;
					break;

				case DayWeekViewButton:
					FrmGotoForm(gApplicationDbP, WeekView);
					handled = true;
					break;

				case DayMonthLunarViewButton:
					MonthViewSetLunarCalendar(MonthMonthLunarViewButton);
					FrmGotoForm (gApplicationDbP, MonthView);
					handled = true;
					break;

				case DayMonthViewButton:
					MonthViewSetLunarCalendar(MonthMonthViewButton);
					FrmGotoForm(gApplicationDbP, MonthView);
					handled = true;
					break;

				case DayAgendaViewButton:
					FrmGotoForm(gApplicationDbP, AgendaView);
					handled = true;
					break;

				case DayDetailsButton:
					DoEventDetailsDialog();
					handled = true;
					break;

				case DayNewButton:
					newRecRowID = DayViewNewAppointment (NULL, false);
					DayViewSelectTime (newRecRowID);
					break;

				case DayGoToButton:
					DayViewGoToDate ();
					handled = true;
					break;
			}
			break;


		case ctlExitEvent:
			// If the pen when down in the details button but did not go up in it,
			// restore the edit state.

			if (eventP->data.ctlExit.controlID == DayDetailsButton)
				DayViewDrawInvalidate(NULL);

			break;


		case ctlRepeatEvent:
			// Handle the scrolling controls.

			switch (eventP->data.ctlRepeat.controlID)
			{
				case DayPrevWeekButton:
					DateAdjust (&Date, -daysInWeek);
					DayViewDrawDate (Date, true);
					break;

				case DayNextWeekButton:
					DateAdjust (&Date, daysInWeek);
					DayViewDrawDate (Date, true);
					break;

				case DayUpButton:
					DayViewScroll (winUp);
					break;

				case DayDownButton:
					DayViewScroll (winDown);
					break;
			}
			break;


		case tblEnterEvent:
			// Check if we've changed row in the day view table, if so
			// clear the edit state.
			handled = DayViewCheckForNonEditableRows(eventP->data.tblSelect.pTable, eventP->data.tblSelect.row);
			if (! handled)
				handled = DayViewHandleTableEnterEvent(eventP->data.tblEnter.pTable, eventP->data.tblSelect.row, eventP->data.tblSelect.column, eventP->screenX, eventP->screenY);
			break;


		case tblSelectEvent:
			// An item in the table has been selected.
			DayViewSelectItem (eventP->data.tblEnter.pTable, eventP->data.tblSelect.row, eventP->data.tblSelect.column);
			handled = true;
			break;


		case tblExitEvent:
			// The user released the pen outside the initial row
			DayViewHandleTblExit(eventP);
			break;


		case fldHeightChangedEvent:
			// Expand or compress the height of the appointments description.
			DayViewResizeDescription (eventP);
			handled = true;
			break;


		// Add the buttons that we want available on the command bar, based on the current context
		case menuCmdBarOpenEvent:
			err = SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
			if (err >= errNone)
				err = SysGetModuleDatabase(uiLibRefNum, NULL, &uilibdbP);

			ErrNonFatalDisplayIf(err < errNone, "Can't get UILibRefNum");

			eventP->data.menuCmdBarOpen.preventFieldButtons = false;

			if (gItemSelected)
			{
				frmP = FrmGetFormPtr(DayView);
				fldP = TblGetCurrentField(FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DayTable)));
				FldGetSelection(fldP, &startPos, &endPos);

				if (startPos == endPos)  // there's no highlighted text, but an item is chosen
				{
					// Call directly Field event handler so that system edit buttons are added if applicable
					FldHandleEvent(fldP, eventP);

					// Left: Beam & Secure
					MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarSecureBitmap, menuCmdBarResultMenuItem, DaySecurityCmd, 0);
					MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarBeamBitmap, menuCmdBarResultMenuItem, BeamRecordCmd, 0);

					// Right: Delete
					MenuCmdBarAddButton(menuCmdBarOnRight, uilibdbP, BarDeleteBitmap, menuCmdBarResultMenuItem, DeleteCmd, 0);

					// Prevent the field package to add edit buttons again
					eventP->data.menuCmdBarOpen.preventFieldButtons = true;
				}
			}
			else	// no item is chosen
			{
				MenuCmdBarAddButton(menuCmdBarOnLeft, uilibdbP, BarSecureBitmap, menuCmdBarResultMenuItem, DaySecurityCmd, 0);
				// allow the field package to automatically add cut, copy, paste, and undo buttons as applicable
			}

			if (err >= errNone)
				SysUnloadModule(uiLibRefNum);

			// don't set handled to true; this event must fall through to the system.
			break;


		case menuOpenEvent:
			if (! gAttachRequest)
			{
				if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
					MenuHideItem(SendRecordCmd);
				else
					MenuShowItem(SendRecordCmd);
			}
			else
			{
				// Hide send & beam items
				MenuHideItem(BeamRecordCmd);
				MenuHideItem(SendRecordCmd);

				// Show the attach menu item
				MenuShowItem(AttachRecordCmd);
			}
			// don't set handled = true
			break;


		case menuEvent:
			handled = DayViewDoCommand (eventP->data.menu.itemID);
			break;


		case winUpdateEvent:
			if (gWinHdl != eventP->data.winUpdate.window)
				break;

			if (ApptDB)
			{
				DayViewUpdateDisplay ();
				DayViewRestoreEditState ();
			}

			handled = true;
			break;


		case winResizedEvent:
			// Active Input Area handling
			if (gWinHdl != eventP->data.winResized.window)
				break;

			DayViewResizeForm(eventP);
			handled = true;
			break;


		case frmOpenEvent:
			frmP = FrmGetFormPtr(DayView);
			DayViewInit (frmP, true);
			handled = true;
			break;


		case frmCloseEvent:
			frmP = FrmGetFormPtr(DayView);
			DayViewFinalize(frmP);
			break;

		case frmGotoEvent:
			DayViewGotoAppointment (eventP);
			break;


		case frmSaveEvent:
			DayViewClearEditState ();
			// This deletes empty events. It can do this because we don't do a FrmSaveAllForms
			// on a sysAppLaunchCmdSaveData launch.
			break;


		case frmTitleEnterEvent:
			// Generate a nilEvent within 100 miliseconds
			frmP = FrmGetFormPtr(DayView);

			GetTitleBounds(frmP, &sTitleBounds);

			TimeComputeWaitTime = true;
			TimeDisplayTick = TimGetTicks() + timeDisplayWaitTime;
			break;
	

		case frmObjectFocusTakeEvent:
			TraceOutput(TL(appErrorClass, "DayView, frmObjectFocusTakeEvent fid=%hu obj=%hu",
						eventP->data.frmObjectFocusTake.formID,
						eventP->data.frmObjectFocusTake.objectID));


			if (eventP->data.frmObjectFocusTake.formID == DayView)
			{
				// remember last focused object id
				sDayViewFocusedItemID = eventP->data.frmObjectFocusTake.objectID;

				switch(eventP->data.frmObjectFocusTake.objectID)
				{
					case DayTable:
						if ((gItemSelected)
							&& (DbCursorGetCurrentRowID(gApptCursorID, &newRecRowID) >= errNone)
							&& (TblFindRowData (UtilitiesGetObjectPtr(DayView, DayTable), newRecRowID, &gSelectedRow)))
						{
							DayViewFocusTableRegainFocus();
						}
						else
							DayViewFocusTake();
						handled = true;
						break;

					case DayDayViewButton:
						// when window is regaining focus (windowfocus)
						// is is also refocusing ititial focused item
						// we want to avoid this when we are back from dtails
						// dialog

						//if (sDayTableFocusState == kFocusDayDetailsOpened)
						//{
						//	DayViewFocusTake();
						//	handled = true;
						//}
						break;
				}
			}

			// the event has to be handle by standard navigation handler

			break;


		case frmObjectFocusLostEvent:
			TraceOutput(TL(appErrorClass, "DayView, frmObjectFocusLostEvent fid=%hu obj=%hu",
						eventP->data.frmObjectFocusLost.formID,
						eventP->data.frmObjectFocusLost.objectID));

 			if (eventP->data.frmObjectFocusLost.formID == DayView)
			{
 				switch(eventP->data.frmObjectFocusTake.objectID)
				{
					case DayTable:
						DayViewFocusLost();
						break;
				}
			}

			// the event has to be handle by standard navigation handler

			break;


		case datebookRefreshDisplay:

			frmP = FrmGetFormPtr(DayView);

			// Re-build the layout and send a winUpdateEvent on the table
			DayViewClearEditState ();

			// Do not use drawing callback to factorize drawings with system invalidations
			DayViewRefreshDisplay(true, false);

			// Utile ?
			DayViewDrawInvalidate(NULL);
			handled = true;
			break;


		case datebookRestoreSelection:
			handled =  DayViewRestoreEditStateFullfill();
			break;


		case datebookEditSelectedNote:
			DetailsEditSelectNote(gApptCursorID, eventP);
			handled = true;
			break;
	}

	return (handled);
}
