/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateAlarm.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This module contains the routines that handle alarms.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>

#include <AppMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <FeatureMgr.h>
#include <Loader.h>
#include <Preferences.h>
#include <PrivateRecords.h>
#include <SchemaDatabases.h>
#include <SoundMgr.h>
#include <string.h>
#include <StringMgr.h>
#include <SystemResources.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>

#include "DateGlobals.h"
#include "Datebook.h"
#include "DateAlarm.h"

#include "DatebookRsc.h"



/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define kAlarmDescSeparatorY			9		// whitespace between title and description
#define kAlarmDetailDescHTextOffset		37
#define kAlarmDetailDescYTextOffset		4
#define kAlarmDetailDescMaxLine			4
#define kAlarmListDescMaxLine			1
#define kAlarmPaddingSeconds			3		// alarms will never trigger more than 3 seconds early


/***********************************************************************
 *
 * FUNCTION:		DrawTimeWithReducedSpace
 *
 * DESCRIPTION:		Reduces the space char width displayed in the alarm list
 *					(not detailed one). It is needed because some dates
 *					(for instance 11:55 pm - 12:55 am 12/24/03 were too long
 *					to fit in initial area.
 *
 * PARAMETERS:		timeStr - The time to display
 *					xP - Pointer on the horizontal position
 *					y - Vertical position
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
  *			Ple		11/07/03	Initial Revision
 *
 ***********************************************************************/
static void DrawTimeWithReducedSpace (char * timeStr, Coord * xP, Coord y)
{
	char*	tmpPtr;

	// Check is there is space char
	tmpPtr = timeStr;
	while (*tmpPtr != chrNull)
	{
		if (*tmpPtr == chrSpace)
		{
			// Draw first part
			*tmpPtr = chrNull;
			WinDrawChars (timeStr, strlen (timeStr), *xP, y);
			*xP += FntCharsWidth (timeStr, strlen (timeStr)) + (FntCharWidth(spaceChr) / 2);

			// Draw second part
			tmpPtr++;
			WinDrawChars (tmpPtr, strlen (tmpPtr), *xP, y);
			*xP += FntCharsWidth (tmpPtr, strlen (tmpPtr));
			*timeStr = chrNull;
			break;
		}
		tmpPtr++;
	}

	// No space, write full time string
	if (*timeStr != chrNull)
	{
		WinDrawChars (timeStr, strlen (timeStr), *xP, y);
		*xP += FntCharsWidth (timeStr, strlen (timeStr));
	}

}

/***********************************************************************
 *
 * FUNCTION:		DrawListAlarm
 *
 * DESCRIPTION:	Draws the alarm info in attention manager list view.
 *
 * PARAMETERS:		eventTime	- pointer to time of event
 *					duration	- pointer to duration of event
 *					untimed		- specifies if this is an untimed event
 *					description	- pointer to the text to be displayed to describe event
 *					paramsPtr	- pointer to attention manager structure containing info
 *										  as to where to draw
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/04/00	Initial Revision
 *			CS		11/14/00	Use PrefGetPreference instead of PrefGetPreferences.
 *
 ***********************************************************************/
static void DrawListAlarm (uint32_t eventTime, int32_t duration, char* description, Boolean untimed, AttnCommandArgsType *paramsPtr)
{
	char 					dateStr[longDateStrLength + 1];
	char					timeStr [timeStringLength + 1];
 	uint16_t				lineCount = 0;
 	size_t					length;
 	size_t					descLen;
	Coord					descFitWidth;
	size_t					descFitLen;
	Coord					maxWidth;
 	Coord					x, y;
	FontID					curFont;
	MemHandle				resH;
	char*					resP;
	char*					ptr = NULL;
	DateTimeType 			startDateTime, endDateTime, today;
	DateFormatType			dateFormat;
	TimeFormatType			timeFormat;
	Boolean					fit;
 	char					chr;
	MemHandle				smallIconH;
	BitmapPtr				smallIconP;

	// Get the date and time formats.
	dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);
	timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	// Compute the maximum width of a line of the description.
	maxWidth = paramsPtr->drawList.bounds.extent.x;

	// Set the font used to draw the alarm info
	curFont = FntSetFont (stdFont);

	y = paramsPtr->drawList.bounds.topLeft.y;
	x = paramsPtr->drawList.bounds.topLeft.x;

	//draw the application's small icon
	smallIconH = DmGetResource(gApplicationDbP, iconType, 1001);
	if (smallIconH)
	{
		smallIconP = (BitmapPtr)(MemHandleLock(smallIconH));
		WinDrawBitmap(smallIconP, x, y);

		// Release the icon resource
		MemHandleUnlock (smallIconH);
		DmReleaseResource(smallIconH);
	}

	x += kAttnListTextOffset;

	TimSecondsToDateTime (eventTime, &startDateTime);

	// draw the time information for the event if the event has a time and the duration is > 0.
	if (!untimed)
	{
		// Draw the event's start time
		TimeToAscii ((uint8_t) startDateTime.hour, (uint8_t) startDateTime.minute, timeFormat, timeStr);
		DrawTimeWithReducedSpace(timeStr, &x, y);

		// draw the event's end time if its duration is > 0
		if (duration > 0)
		{
			x += (FntCharWidth(spaceChr) / 2);
			chr = '-';
			WinDrawChars (&chr, 1, x, y);
			x += FntCharWidth (chr) + (FntCharWidth(spaceChr) / 2);

			TimSecondsToDateTime (eventTime + (uint32_t)(duration * minutesInSeconds), &endDateTime);
			TimeToAscii ((uint8_t) endDateTime.hour, (uint8_t) endDateTime.minute, timeFormat, timeStr);
			DrawTimeWithReducedSpace(timeStr, &x, y);
		}

		x += FntCharWidth (spaceChr);
	}


	// Draw the event's date
	// If the event occurs today, draw the
	TimSecondsToDateTime (TimGetSeconds(), &today);

	if ( (today.day == startDateTime.day) && (today.month == startDateTime.month) && (today.year == startDateTime.year))
	{
		resH = DmGetResource (gApplicationDbP, strRsc, alarmTodayStrID);
		resP = MemHandleLock(resH);
		WinDrawChars (resP, strlen (resP), x, y);
		MemPtrUnlock (resP);
		DmReleaseResource(resH);
	}
	else
	{
		DateToAscii((uint8_t) startDateTime.month, (uint8_t) startDateTime.day, startDateTime.year, dateFormat, dateStr);
		WinDrawChars (dateStr, strlen (dateStr), x, y);
	}

	// Draw the event's description.
	x = paramsPtr->drawList.bounds.topLeft.x + kAttnListTextOffset;
	y += FntLineHeight();
	maxWidth = paramsPtr->drawList.bounds.extent.x - kAttnListTextOffset;

	if (description)
	{
		ptr = description;
		descLen = strlen(ptr);
	}
	else descLen = 0;

	while(descLen)
	{
		descFitWidth = maxWidth;
		descFitLen = descLen;

		// Calculate how many characters will fit in the window bounds
		FntCharsInWidth (ptr, &descFitWidth, &descFitLen, &fit);
		if (!descFitLen)
		{
			break;
		}

		// Calculate the number of characters in full words that will fit in the bounds
		length = FldWordWrap(ptr, maxWidth);

		// Need to display the minimum of the two as FldWordWrap includes carriage returns, tabs, etc.
		descFitLen = min(descFitLen, length);

		if (++lineCount >= kAlarmListDescMaxLine)
		{
			if (descLen != descFitLen)
			{
				descFitLen = descLen;
			}

			if (descFitWidth < maxWidth)
			{
				descFitWidth += FntCharWidth(chrEllipsis);
			}

			descFitWidth = min(descFitWidth, maxWidth);

			WinDrawTruncChars(ptr, descFitLen, x, y, descFitWidth);
			break;
		}
		else
		{
			WinDrawTruncChars(ptr, descFitLen, x, y, maxWidth);
		}

		descLen -= length;
		ptr += length;

		y += FntLineHeight();
	}

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:		DrawDetailAlarm
 *
 * DESCRIPTION:	Draws the alarm info in attention manager detail view.
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		eventTime	- pointer to time of event
 *					duration	- pointer to duration of event
 *					untimed		- specifies if this is an untimed event
 *					description	- pointer to the text to be displayed to describe event
 *					paramsPtr	- pointer to attention manager structure containing info
 *										  as to where to draw
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/04/00	Initial Revision
 *			CS		11/14/00	Use PrefGetPreference instead of PrefGetPreferences.
 *
 ***********************************************************************/
static void DrawDetailAlarm (uint32_t eventTime, int32_t duration, char* description, Boolean untimed, AttnCommandArgsType *paramsPtr)
{
	FontID			curFont;
 	Coord			x, y;
	DateTimeType	dateTime;
 	size_t			length;
	Coord			maxWidth;
 	size_t			descLen;
	Coord			descFitWidth;
	size_t			descFitLen;
	Boolean			fit;
 	uint16_t		lineCount = 0;
	char			dowNameStr[dowDateStringLength + 1];
	char 			dateStr[longDateStrLength + 1];
	char			timeStr [timeStringLength + 1];
	MemHandle		resH;
	char*			resP;
	char*			ptr;
 	char			chr;
	DateFormatType	dateFormat;
	TimeFormatType	timeFormat;

	// Get the date and time formats.
	dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);
	timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	// Draw the alarm clock icon
	resH = DmGetResource (gApplicationDbP, bitmapRsc, AlarmClockIcon);
	WinDrawBitmap(MemHandleLock(resH), paramsPtr->drawDetail.bounds.topLeft.x, (Coord)(paramsPtr->drawDetail.bounds.topLeft.y + kAlarmDetailDescYTextOffset));
	MemHandleUnlock(resH);
	DmReleaseResource(resH);

	// Set the font used to draw the alarm info
	curFont = FntSetFont (largeBoldFont);

	// Draw the date - for English this will be "Monday, <date>".
	x = paramsPtr->drawDetail.bounds.topLeft.x + kAlarmDetailDescHTextOffset;
	y = paramsPtr->drawDetail.bounds.topLeft.y + kAlarmDetailDescYTextOffset;

	TimSecondsToDateTime (eventTime, &dateTime);

	// Get the day-of-week name and the system formatted date
	DateTemplateToAscii("^1l", (uint8_t) dateTime.month, (uint8_t) dateTime.day, dateTime.year, dowNameStr, sizeof(dowNameStr));
	DateToAscii((uint8_t) dateTime.month, (uint8_t) dateTime.day, dateTime.year, dateFormat, dateStr);

	resH = DmGetResource (gApplicationDbP, strRsc, drawAlarmDateTemplateStrID);
	resP = MemHandleLock (resH);
	ptr = TxtParamString(resP, dowNameStr, dateStr, NULL, NULL);
	MemPtrUnlock(resP);
	DmReleaseResource(resH);

	WinDrawChars(ptr, strlen(ptr), x, y);
	MemPtrFree((MemPtr)ptr);

	// the time of the event if the event has a time.
	if (!untimed)
	{
		// Draw the event's time and duration.
		TimeToAscii ((uint8_t) dateTime.hour, (uint8_t) dateTime.minute, timeFormat, timeStr);

		y += FntLineHeight();
		WinDrawChars (timeStr, strlen (timeStr), x, y);

		if (duration > 0)
		{
			x += FntCharsWidth (timeStr, strlen (timeStr)) + FntCharWidth (spaceChr);
			chr = '-';
			WinDrawChars (&chr, 1, x, y);
			TimSecondsToDateTime (eventTime + (uint32_t)(duration * minutesInSeconds), &dateTime);
			TimeToAscii ((uint8_t) dateTime.hour, (uint8_t) dateTime.minute, timeFormat, timeStr);
			x += FntCharWidth (chr) + FntCharWidth (spaceChr);
			WinDrawChars (timeStr, strlen (timeStr), x, y);
		}
	}


	// Draw the event's description.
	y += kAlarmDescSeparatorY;
	x = paramsPtr->drawDetail.bounds.topLeft.x + kAlarmDetailDescHTextOffset;
	maxWidth = paramsPtr->drawDetail.bounds.extent.x - x;

	if (description)
	{
		ptr = description;
		descLen = strlen(ptr);
	}
	else descLen = 0;

	while(descLen)
	{
		descFitWidth = maxWidth;
		descFitLen = descLen;

		// Calculate how many characters will fit in the window bounds
		FntCharsInWidth (ptr, &descFitWidth, &descFitLen, &fit);
		if (!descFitLen)
		{
			break;
		}

		// Calculate the number of characters in full words that will fit in the bounds
		length = FldWordWrap  (ptr, maxWidth);

		// Need to display the minimum of the two as FldWordWrap includes carriage returns, tabs, etc.
		descFitLen = min(descFitLen, length);

		y += FntLineHeight();


		if (++lineCount >= kAlarmDetailDescMaxLine)
		{
			if (descLen != descFitLen)
			{
				descFitLen = descLen;
			}

			if (descFitWidth < maxWidth)
			{
				descFitWidth += FntCharWidth(chrEllipsis);
			}


			descFitWidth = min(descFitWidth, maxWidth);

			WinDrawTruncChars(ptr, descFitLen, x, y, descFitWidth);
			break;
		}
		else
		{
			WinDrawTruncChars(ptr, descFitLen, x, y, maxWidth);
		}

		descLen -= length;
		ptr += length;
	}

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:		DrawAlarm
 *
 * DESCRIPTION:	Does the initial validation an setup for alarm drawing then
 *						calls the appropriate routine for either list or detailed
 *						display.
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		uniqueID	- the uniqueID of the event
 *					paramsPtr	- info provided by attention manager
 *					drawDetail	- boolean specifying Detail or list view
 *
 * RETURNED:		false if something went wrong, true otherwise.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap		10/04/00	Initial Revision
 *			peter	01/22/01	Add kAlarmPaddingSeconds to work around Alarm Mgr flaw
 *			ppl		07/10/03	cursor !
 *
 ***********************************************************************/
static Boolean DrawAlarm (uint32_t inUniqueID, AttnCommandArgsType *paramsPtr, Boolean drawDetail)
{
	DmOpenRef 				dbP;
	int32_t 				duration = 0;
	int32_t 				adjust;
	uint32_t 				eventTime;
	Boolean					untimed;
	MemHandle				resH = NULL;
	char*					description;
	ApptDBRecordType 		apptRec;
	uint16_t 				attr;
	Boolean					displayPrivate = false;

	// Open the appointment database.
	if (DateDBOpenDatabase(&dbP, dmModeReadOnly | dmModeShowSecret) < errNone)
		return false;

	if (ApptGetRecord(dbP, inUniqueID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_DESCRIPTION | DBK_SELECT_REPEAT) < errNone)
	{
		DbCloseDatabase(dbP);
		return false;
	}

	// If the device is locked, it is still possible for the app to
	// get called by the attention manager in order to display information
	// for  an event whose alarm has just triggered.  In this case
	// DateBook must always display the "Private Record" string regardless
	// of the setting of the event's private bit.  Otherwise,
	// if the user has selected that private records be masked or hidden
	// determine if the private bit has been set for the record, if it is
	// we need to show the string "Private Appointment" in place of the
	// actual appointment title.
	if (PrefGetPreference(prefDeviceLocked))
	{
		displayPrivate = true;
	}
	else
	{
		if ( PrefGetPreference(prefShowPrivateRecords) != showPrivateRecords )
		{
			DbGetRowAttr (dbP, inUniqueID, &attr);
			displayPrivate = (attr & dmRecAttrSecret);
		}
	}

	if (displayPrivate)
	{
		resH = DmGetResource (gApplicationDbP, strRsc, alarmPrivateApptStrID);
		description = MemHandleLock(resH);
	}
	else
	{
		description = apptRec.description;
	}


	// Calculate the event's date and time from the alarm time and the alarm
	// advance.  The date and time stored in the record will not be
	// the correct values to display, if the event is repeating.
	adjust = apptRec.alarm.advance;
	switch (apptRec.alarm.advanceUnit)
	{
		case aauMinutes:
			adjust *= minutesInSeconds;
			break;
		case aauHours:
			adjust *= hoursInSeconds;
			break;
		case aauDays:
			adjust *= daysInSeconds;
			break;
	}

	// The alarm manager may trigger a bit early, so to be safe, search
	// backwards for the appropriate alarm time, not from the current time,
	// but from a bit into the future. Since the fastest repeat rate for
	// alarms is once a day, this won't cause any significant problems.
	// If displaying a repeating event alarm just before the next occurrence
	// is about to go off, you may see the new alarm time, but that's not
	// a serious problem.
	eventTime = ApptGetAlarmTime (&apptRec, TimGetSeconds() + kAlarmPaddingSeconds, false);
	ErrNonFatalDisplayIf(eventTime == 0, "No alarm before now");
	eventTime = eventTime + adjust;

	// Calculate the duration of the event.
	untimed = apptRec.when.noTimeEvent;
	if (!untimed)
	{
		duration = (int32_t)((apptRec.when.endTime.hours * hoursInMinutes + apptRec.when.endTime.minutes) -
					  (apptRec.when.startTime.hours * hoursInMinutes + apptRec.when.startTime.minutes));
		if (apptRec.when.midnightWrapped)
			duration += daysInSeconds;
	}

	if (drawDetail)
	{
		DrawDetailAlarm (eventTime, duration, description, untimed, paramsPtr);
	}
	else
	{
		DrawListAlarm(eventTime, duration, description, untimed, paramsPtr);
	}

	if (displayPrivate)
	{
		MemHandleUnlock (resH);
		DmReleaseResource(resH);
	}

	ApptFreeRecordMemory (&apptRec);

	DbCloseDatabase (dbP);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:		ValidateAlarm
 *
 * DESCRIPTION:	This routine validates the item with the specified
 *						uniqueID to determine if it still exists as well as if
 *						its alarm is still valid.  This routine is called to
 *						validate alarms in the attention manager queue following
 *						a device time change, a hotsync, and an attention manager
 *						"tickle"
 *
 * PARAMETERS:  	uniqueID  - the unique ID of the event
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	08/15/00	Initial Revision
 *
 ***********************************************************************/
static void ValidateAlarm (uint32_t uniqueID)
{
	DmOpenRef			dbP;
	status_t			err;
	Boolean				valid = true;
	ApptDBRecordType	apptRec;
	CategoryID 			dbSelectAllCategoriesID[] = {catIDAll};
	Boolean				recordIsInCategory = false;

	// Open the appointment database with show secret flag
	// as we have to show alarm on "private events"
	// (the cursor that we are going to open will use the same flags)
	if (DateDBOpenDatabase(&dbP, dmModeReadOnly | dmModeShowSecret) < errNone)
		return;

	// Determine if the record still exists
	err = DbIsRowInCategory(dbP, uniqueID, 1, dbSelectAllCategoriesID, DbMatchAny, &recordIsInCategory);
	if ((err< errNone) || (!recordIsInCategory))
		valid = false;

	// Now get the event's info to determine if the alarm time is <= the current time
	// if it is greater than the current time, forget it and let it ring again when
	// the device reaches its specified time.
	if (valid)
	{
		ApptGetRecord (dbP, uniqueID, &apptRec, DBK_SELECT_TIME | DBK_SELECT_ALARM | DBK_SELECT_REPEAT | DBK_SELECT_EXCEPTIONS);
		valid = ( (apptRec.alarm.advance) && ApptGetAlarmTime (&apptRec, TimGetSeconds(), false) );
		ApptFreeRecordMemory (&apptRec);
	}

	// If one of the previous tests failed, remove the alarm from the attention manager queue
	if (!valid)
		AttnForgetIt(gApplicationDbID, uniqueID);

	DbCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:		GotoAlarm
 *
 * DESCRIPTION:	This code handle user specification in an attention
 *						manager dialog (both list view and details view).
 *
 * PARAMETERS:  	uniqueID  - the unique ID of the event
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	09/06/00	Initial Revision
 *
 ***********************************************************************/
static void GotoAlarm (uint32_t uniqueID)
{
	DmOpenRef	dbP;
	uint32_t	*gotoInfoP;
	status_t	err;
	CategoryID 	dbSelectAllCategoriesID[] = {catIDAll};
	Boolean		recordIsInCategory = false;

	// Open the appointment database with show secret flag
	// as we have to show alarm on "private events"
	// (the cursor that we are going to open will use the same flags)
	if (DateDBOpenDatabase(&dbP, dmModeReadOnly | dmModeShowSecret) < errNone)
		return;

	err = DbIsRowInCategory(dbP, uniqueID, 1, dbSelectAllCategoriesID, DbMatchAny, &recordIsInCategory);
	DbCloseDatabase (dbP);
	// if the unique ID no longer exists, just remove the attention from the attention manager queue and return
	if ((err < errNone) || (!recordIsInCategory) )
	{
		AttnForgetIt(gApplicationDbID, uniqueID);
		return;
	}

	// if we received a goto for a valid unique ID
	// create the pointer to contain the goto information
	gotoInfoP = (uint32_t*)MemPtrNew (sizeof(uint32_t));
	ErrFatalDisplayIf ((!gotoInfoP), "Out of memory");
	MemPtrSetOwner(gotoInfoP, 0);

	// initialize the goto params structure so that datebook will open day view
	// with the specified item selected
	*gotoInfoP = uniqueID;

	// Launch DateBook with the corresponding launch code.
	SysUIAppSwitch(gApplicationDbID, appLaunchCmdAlarmEventGoto, gotoInfoP, sizeof(uint32_t));
}


/***********************************************************************
 *
 * FUNCTION:		AttentionBottleNeckProc
 *
 * DESCRIPTION:	Main bottleneck proc whihc processes attention manager
 *						launch codes.
 *
 * PARAMETERS:  	paramP -  the launch code specific information supplied
 *						by the attention manager.
 *
 * RETURNED:    	true if the alarm was found/deleted
 *						false if alarm was not posted
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	07/31/00	Initial Revision
 *
 ***********************************************************************/
Boolean AttentionBottleNeckProc(AttnLaunchCodeArgsType * paramP)
{
	AttnCommandArgsType * 	argsP = paramP->commandArgsP;
	DatebookPreferenceType 	prefs;

	TraceOutput(TL(appErrorClass,">>>> AttentionBottleNeckProc, command = %d", paramP->command));

	switch (paramP->command)
	{
		case kAttnCommandDrawDetail:
			if (!DrawAlarm(paramP->userData, paramP->commandArgsP, true))
				AttnForgetIt(gApplicationDbID, paramP->userData);
			break;

		case kAttnCommandDrawList:
			if (!DrawAlarm(paramP->userData, paramP->commandArgsP, false))
				AttnForgetIt(gApplicationDbID, paramP->userData);
			break;

		case kAttnCommandPlaySound:
			// Load Date Book's prefs so we can get the user-specified alarm sound.
			DatebookLoadPrefs (&prefs);
			PlayAlarmSound(prefs.alarmSoundUniqueRecID);
			break;

		case kAttnCommandGotIt:
			if (argsP->gotIt.dismissedByUser)
				AttnForgetIt(gApplicationDbID, paramP->userData);
			break;

		case kAttnCommandGoThere:
			GotoAlarm(paramP->userData);
			break;

		case kAttnCommandIterate:
			// if the argument is nil, this is a "tickle from the attention manager
			// asking the application to validate the specified entry
			// this may happen at interrupt time - do not use globals here
			if (argsP == NULL)
				ValidateAlarm(paramP->userData);
			else
			{
				// otherwise, this launch code was received from attention manager in
				// response to an AttnIterate made by Date Book to update the posted alarms
				// with respect to one of the following occurrences
				switch (argsP->iterate.iterationData)
				{
					// When the user changes the nag parameters, assign the new value to each alarm currently
					// in the attention manager queue.
					// THIS WILL ONLY OCCUR WHEN APP IS RUNNING SO CACHED GLOBALS ARE USED FOR SPEED
					case SoundRepeatChanged:
						AttnUpdate(gApplicationDbID, paramP->userData, NULL, &AlarmSoundRepeatInterval,
							&AlarmSoundRepeatCount);
						break;

					default:
						ValidateAlarm(paramP->userData);
				}
			}
			break;
	}

	return true;
}



/***********************************************************************
 *
 * FUNCTION:		DeleteAlarmIfPosted
 *
 * DESCRIPTION:	Get the unique ID of the current record number and call
 *						AttnForgetIt to remove it form the attention manager
 *						queue.
 *
 * PARAMETERS:  	rowID / cursorID -  the event row or cursor ID.
 *
 * RETURNED:    	true if the alarm was found/deleted
 *						false if alarm was not posted
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	07/31/00	Initial Revision
 *			gap	12/11/00	Add param check for recordnum to be sure it is not noRecordSelected
 *
 ***********************************************************************/
Boolean DeleteAlarmIfPosted (uint32_t rowID)
{
	uint32_t 	localRowID = rowID;

	// Should always have a valid record number when this routine is called.
	if (rowID == dbInvalidRowID)
	{
		ErrNonFatalDisplay("trying to delete recordNum of noRecordSelected from attn mgr queue ");
		return false;
	}

	// Check if the argument is a cursor, otherwise it must be a rowID
	if (DbIsCursorID(rowID))
	{
		if (DbCursorGetCurrentRowID(rowID, &localRowID) < errNone)
			return false;
	}

	ErrNonFatalDisplayIf(! DbIsRowID(localRowID), "Invalid row ID");

	// remove the alarm from the attention manager queue if it is present
 	return (AttnForgetIt(gApplicationDbID, localRowID));
}


/***********************************************************************
 *
 * FUNCTION:	AlarmTriggered
 *
 * DESCRIPTION:	This routine is called when the alarm manager informs the
 *						datebook application that an alarm has triggered.
 *						THIS IS CALLED AT INTERRUPT LEVEL! DONT USE GLOBALS!!
 *
 * PARAMETERS:	time of the alarm.
 *
 * RETURNED:	time of the next alarm
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		9/20/95	Initial Revision
 *			frigino	9/9/97	Switched use of globals to use of prefs
 *									values instead.
 *			vmk		12/9/97	Call DatebookLoadPrefs() to load/fixup our app prefs
 *			rbb		04/22/99	Added snooze feature
 *			gap		08/08/00	Rewritten for attention manager support
 *			gap		08/08/00	Rewritten again for attention manager support
 *									now post alarms to attention manager during the
 *									triggered launch code in stead of the display
  *		ppl		07/15/03	cursor !
 *
 ***********************************************************************/
void AlarmTriggered (SysAlarmTriggeredParamType * cmdPBP)
{
	DmOpenRef	dbP;
	uint32_t	alarmTime;
	uint32_t	nextAlarmTime;
	uint32_t	rowIDofEarliestAlarm;

	// Get the time of the alarm that just triggered
	alarmTime = cmdPBP->alarmSeconds;

	// all triggered alarms are sent to attention manager for display so there is
	// no need for alarm manager to send the sysAppLaunchCmdDisplayAlarm launchcode
	cmdPBP->purgeAlarm = true;

	// Check if the alarm occurs in past, ignore it if so. We have to check this
	// because alarm triggering is asynchronous and AlmSetAlarm(0,0) doesn't
	// remove the lastly enqueued alarm. Because alarms resolution is 1 mn, we
	// consider that the alarm occurred in the past if 59s elapsed since the
	// alarm programmed time. This lets 59s to the system to trigger it, largely
	// enough !
	if (alarmTime <= TimGetSeconds() - minutesInSeconds)
	{
		TraceOutput(TL(appErrorClass,"Alarm triggered in the past, ignored (alarm = %ld, now = %ld)",
			alarmTime, TimGetSeconds()));
		return;
	}

	// Open the appointment database with show secret flag
	// as we have to show alarm on "private events"
	// (the cursor that we are going to open will use the same flags)
	if (DateDBOpenDatabase(&dbP, dmModeReadOnly | dmModeShowSecret) < errNone)
	{
		TraceOutput(TL(appErrorClass,"AlarmTriggered: DB could not be open"));
		return;
	}

	// Establish the time for which alarms need to be retrieved.
#if defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF
	{
		DateTimeType testTime;
		char strTime[longDateStrLength+1];

		TimSecondsToDateTime(cmdPBP->alarmSeconds, &testTime);
		DateToAscii((uint8_t)testTime.month,(uint8_t)testTime.day, (uint16_t)testTime.year, dfMDYWithSlashes, strTime);
		TraceOutput(TL(appErrorClass,">>>> Received alarm set to: %s (%lx) - rowID = %lx", strTime, cmdPBP->alarmSeconds, cmdPBP->ref));
	}
#endif

	// Post alarms matching alarmTime and get the next alarm to ring.
	nextAlarmTime = ApptPostTriggeredAlarmsAndGetNext(dbP, alarmTime /* post alarm*/, alarmTime + minutesInSeconds /* get next*/,
		&rowIDofEarliestAlarm);

	// Set the next alarm if found one
	if (nextAlarmTime)
	{
		AlmSetAlarm (gApplicationDbID, rowIDofEarliestAlarm, nextAlarmTime, true);

#if defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF
		{
			DateTimeType testTime;
			char strTime[longDateStrLength+1];

			TimSecondsToDateTime(nextAlarmTime, &testTime);
			DateToAscii((uint8_t)testTime.month,(uint8_t)testTime.day, (uint16_t)testTime.year, dfMDYWithSlashes, strTime);
			TraceOutput(TL(appErrorClass,">>>> Set next alarm to: %s (%lx) - rowID = %lx", strTime, nextAlarmTime, rowIDofEarliestAlarm));
		}
#endif
	}

	DbCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:    RescheduleAlarms
 *
 * DESCRIPTION: This routine computes the time of the next alarm and
 *              compares it to the time of the alarm scheduled with
 *              Alarm Manager,  if they are different it reschedules
 *              the next alarm with the Alarm Manager.
 *
 * PARAMETERS:  dbP - the appointment database
 *				cursorID - A cursor on all appointments, current position
 *					is discarded.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		art		9/5/95		Initial Revision
 *		rbb		4/22/99		Snoozing now disables other rescheduling of alarms
 *		gap		9/25/00		removed snoozing & alarm internals
 *		ppl		07/15/03	cursor !
 *
 ***********************************************************************/
void RescheduleAlarms (DmOpenRef dbP)
{
	uint32_t ref;
	uint32_t timeInSeconds;
	uint32_t nextAlarmTime;
	uint32_t scheduledAlarmTime;
	uint32_t rowIDofEarliestAlarm;

	scheduledAlarmTime = AlmGetAlarm (gApplicationDbID, &ref);
	timeInSeconds = TimGetSeconds();
	if ((timeInSeconds < scheduledAlarmTime) || (scheduledAlarmTime == 0))
		scheduledAlarmTime = timeInSeconds;

	// ApptGetTimeOfNextAlarm is calling ApptAlazrmMunger that reopens the database
	// in readwrite and show secret mode.
	nextAlarmTime = ApptGetTimeOfNextAlarm (dbP, scheduledAlarmTime, &rowIDofEarliestAlarm);

	// If the scheduled time of the next alarm is not equal to the
	// calculated time of the next alarm,  reschedule the alarm with
	// the alarm manager.
	if (nextAlarmTime && (scheduledAlarmTime != nextAlarmTime))
	{
		AlmSetAlarm (gApplicationDbID, rowIDofEarliestAlarm, nextAlarmTime, true);
#if defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF
		{
			DateTimeType testTime;
			char strTime[longDateStrLength+1];

			TimSecondsToDateTime(nextAlarmTime, &testTime);
			DateToAscii((uint8_t)testTime.month,(uint8_t)testTime.day, (uint16_t)testTime.year, dfMDYWithSlashes, strTime);
			TraceOutput(TL(appErrorClass,">>>> Reschedule AlmSetAlarm: alarm Set to = %s (%lx) - rowID = %lx", strTime, nextAlarmTime, rowIDofEarliestAlarm));
		}
#endif
	}
}

/***********************************************************************
 *
 * FUNCTION:    RescheduleAlarmsAllRows
 *
 * DESCRIPTION: Calls rescheduleAlarms reopening the db with read write
 *				and show secret flags.
 *
 * PARAMETERS:  dbP - the appointment database
 *				cursorID - A cursor on all appointments, current position
 *				is discarded.
 *
 * RETURNED:    nothing.
 *
 *				Actually Reschedule alarm have to read private record
 *				for being able to reschedule alarm on private records.
 *
 * REVISION HISTORY:
 *		ppl		12/15/03	more cursors !
 *
 ***********************************************************************/

void RescheduleAlarmsAllRows (void)
{
	DmOpenRef dbP = NULL;

	// Open the appointment database with show secret flag
	// as we have to show alarm on "private events"
	// (the cursor that we are going to open will use the same flags)
	if (DateDBOpenDatabase(&dbP, dmModeReadOnly | dmModeShowSecret) < errNone)
		return;

	RescheduleAlarms (dbP);

	DbCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:    AlarmReset
 *
 * DESCRIPTION: This routine is called when the system time is changed
 *              by the Preference application, or when the device is reset.
 *
 * PARAMETERS:  newerOnly - If true, we will not reset the alarm if the
 *              time of the next (as calculated) is greater then the
 *              currently scheduled alarm.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		art		9/21/95		Initial Revision
 *      art		11/18.96	Reseting time will nolong trigger alarms.
 *		rbb		4/22/99		Reset snooze and any pending alarms
 *		ppl		07/15/03	cursor !
 *
 ***********************************************************************/
void AlarmReset (Boolean newerOnly)
{
	DmOpenRef 	dbP;
	uint32_t 	ref;
	uint32_t 	currentTime;
	uint32_t 	alarmTime;

	if (newerOnly)
	{
		alarmTime = AlmGetAlarm (gApplicationDbID, &ref);
		currentTime = TimGetSeconds ();
		if ( alarmTime && (alarmTime <= currentTime))
			return;
	}

	// Clear any pending alarms.
	AlmSetAlarm (gApplicationDbID, 0, 0, true);
	TraceOutput(TL(appErrorClass,">>>> AlmSetAlarm: Reset all alarms"));

	// Open the appointment database with show secret flag
	// as we have to show alarm on "private events"
	// (the cursor that we are going to open will use the same flags)
	if (DateDBOpenDatabase(&dbP, dmModeReadOnly | dmModeShowSecret) < errNone)
		return;

	RescheduleAlarms (dbP);

	DbCloseDatabase (dbP);
}



/***********************************************************************
 *
 * FUNCTION:    PlayAlarmSound
 *
 * DESCRIPTION:	Play a MIDI sound given a unique record ID of the MIDI record in the System
 *						MIDI database.  If the sound is not found, then the default alarm sound will
 *						be played.
 *
 * PARAMETERS:  uniqueRecID	-- unique record ID of the MIDI record in the System
 *										   MIDI database.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		8/10/97		Initial version
 *			PLe		04/24/03	Palm OS6 revision
 *
 ***********************************************************************/
void PlayAlarmSound(uint32_t uniqueRecID)
{
	status_t			err;
	MemHandle			midiH;			// handle of MIDI record
	SndMidiRecHdrType*	midiHdrP;		// pointer to MIDI record header
	uint8_t*			midiStreamP;	// pointer to MIDI stream beginning with the 'MThd'
										// SMF header chunk
	DatabaseID 			dbID;			// Local ID of System MIDI database
	DmOpenRef			dbP = NULL;		// reference to open database
	uint16_t			recIndex = 0;	// record index of the MIDI record to play
	SndSmfOptionsType	smfOpt;			// SMF play options

	// Find the system MIDI database
	dbID = DmFindDatabaseByTypeCreator(sysFileTMidi, sysFileCSystem, dmFindClassicDB, NULL);
	if (dbID == NULL)
	{
		err = DmGetLastErr();
		goto ExitWithError;
	}

	// Open the MIDI database in read-only mode
	dbP = DmOpenDatabase(dbID, dmModeReadOnly);
	if ( !dbP )
		goto ExitWithError;

	// Find the MIDI track record
	if (DmNumRecords(dbP) == 0)
		goto ExitWithError;

	// Keep the default value is the ID is not valid
	if (uniqueRecID != dmUnusedRecordID)
	{
		err = DmFindRecordByID (dbP, uniqueRecID, &recIndex);
		if ( err )
			goto ExitWithError;
	}

	// Find the record handle and lock the record
	midiH = DmQueryRecord(dbP, recIndex);
	midiHdrP = MemHandleLock(midiH);

	// Get a pointer to the SMF stream
	midiStreamP = (uint8_t*) midiHdrP + midiHdrP->bDataOffset;

	// Play the sound (ignore the error code)
	// The sound can be interrupted by a key/digitizer event
	smfOpt.dwStartMilliSec 	= 0;
	smfOpt.dwEndMilliSec 	= sndSmfPlayAllMilliSec;
	smfOpt.amplitude 		= (uint16_t) PrefGetPreference(prefAlarmSoundVolume);
	smfOpt.interruptible 	= true;
	smfOpt.reserved 		= 0;

	err = SndPlaySmf (NULL, sndSmfCmdPlay, midiStreamP, &smfOpt, NULL, NULL, false);

	// Unlock the record
	MemPtrUnlock (midiHdrP);

	// Close the MIDI database
	if ( dbP )
		DmCloseDatabase (dbP);

	// Return with no error
	return;

ExitWithError:

	// Close the MIDI database
	if ( dbP )
		DmCloseDatabase (dbP);

	// If there was an error, play the alarm sound
	SndPlaySystemSound(sndAlarm);
}
