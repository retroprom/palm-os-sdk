/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateRepeat.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Used to manage repeat info for appt and quiet mode
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <Control.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <List.h>
#include <PenInputMgr.h>
#include <SelDay.h>
#include <SoundMgr.h>
#include <string.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <Table.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <UIResources.h>

#include "DateGlobals.h"
#include "DateDayDetails.h"
#include "DateRepeat.h"
#include "DateUtils.h"
#include "Datebook.h"
#include "DatebookRsc.h"

/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
// 5 repeat types: None/Day/Week/Month/Year
#define kMaxRepeatInfo		5

static	RepeatInfoType		sRepeatInfo[kMaxRepeatInfo];
static	DateType			sDate;
static	uint16_t			sRepeatStartOfWeek;
static	char				sRepeatDateTriggerLabel[dowDateStringLength];
static	FormType*			sFormP 		= NULL;
static	uint16_t			sCurrentRepeatIndex;
static	uint16_t			sCurrentNavObjectID;


/***********************************************************************
 *
 * FUNCTION:    SubstituteStr
 *
 * DESCRIPTION: This routine substitutes the occurrence a token, within
 *              a string, with another string.
 *
 * PARAMETERS:  str    - string containing token string
 *              token  - the string to be replaced
 *              sub    - the string to substitute for the token
 *              subLen - length of the substitute string.
 *
 * RETURNED:    pointer to the string
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/6/95	Initial Revision
 *
 ***********************************************************************/
static char* SubstituteStr (char* str, const char* token, char* sub, uint16_t subLen)
{
	int16_t charsToMove;
	uint16_t tokenLen;
	uint16_t strLen;
	uint32_t blockSize;
	char* ptr;
	MemHandle strH;

	// Find the start of the token string, if it doesn't exist, exit.
	ptr = StrStr(str, token);
	if (ptr == NULL)
		return (str);

	tokenLen = strlen (token);
	charsToMove = subLen - tokenLen;


	// Resize the string if necessary.
	strH = MemPtrRecoverHandle (str);

	strLen = strlen (str);
	blockSize = MemHandleSize (strH);

	if (strLen + charsToMove + 1 >= (uint16_t) blockSize)
	{
		MemHandleUnlock (strH);

		MemHandleResize (strH, (uint32_t)(strLen + charsToMove + 1));

		str = MemHandleLock (strH);

		ptr = StrStr (str, token);

		ErrNonFatalDisplayIf(ptr == NULL, "Msg missing token");
	}

	// Make room for the substitute string.
	if (charsToMove)
		memmove (ptr + subLen, ptr + tokenLen, (int32_t)(strlen(ptr + tokenLen) + 1));

	// Replace the token with the substitute string.
	memmove (ptr, sub, subLen);

	return (str);
}

/***********************************************************************
 *
 * FUNCTION:    RepeatSetDateTrigger
 *
 * DESCRIPTION: This routine sets the label of the trigger that displays
 *              the end date of a repeating appointment.
 *
 * PARAMETERS:  endDate	- date or -1 if no end date
 *
 * RETURNED:    nothing
 *
 * NOTES:
 *      This routine assumes that the memory allocated for the label of
 *      the due date trigger is large enough to hold the lagest posible
 *      label.  This label's memory is reserved by initializing the label
 *      in the resource file.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/16/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatSetDateTrigger (DateType endDate)
{
	ListPtr 	lst;
	ControlPtr 	ctl;

	lst = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatEndOnList));
	ctl = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatEndOnTrigger));

	if (DateToInt (endDate) == apptNoEndDate)
	{
		strcpy (sRepeatDateTriggerLabel, LstGetSelectionText (lst, repeatNoEndDateItem));
		LstSetSelection (lst, noEndDateItem);
	}
	else
	{
		// Format the end date into a string.
		DateToDOWDMFormat (	(uint8_t) endDate.month,
						 	(uint8_t) endDate.day,
						 	(uint16_t)(endDate.year + firstYear),
						 	ShortDateFormat,
						 	sRepeatDateTriggerLabel);

		LstSetSelection (lst, repeatChooseDateItem);
	}

	CtlSetLabel (ctl, sRepeatDateTriggerLabel);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatGetUIValues
 *
 * DESCRIPTION: This routine gets the current repeat settings of the
 *              ui gadgets in the repeat dialog box
 *
 *
 * PARAMETERS:  repeatP - RepeatInfoType structure (fill-in by this routine)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/10/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatGetUIValues (RepeatInfoPtr repeatP)
{
	uint16_t	freq;
	uint16_t	i, id;
	uint16_t	index;
	char*		str;

	if (repeatP->repeatType == repeatNone)
		return;

	if (repeatP->repeatType == repeatMonthlyByDay || repeatP->repeatType == repeatMonthlyByDate)
	{
		index = FrmGetControlGroupSelection (sFormP, RepeatByGroup);
		id = FrmGetObjectId (sFormP, index);

		if (id == RepeatByDayPushButon)
			repeatP->repeatType = repeatMonthlyByDay;
		else
			repeatP->repeatType = repeatMonthlyByDate;
	}
	// Get the repeat frequency.
	str = FldGetTextPtr (FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatFrequenceField)));
	if (str)
		freq = (uint16_t) StrAToI (str);
	else
		freq = 0;

	if (freq)
		repeatP->repeatFrequency = (uint8_t) freq;
	else
		repeatP->repeatFrequency = 1;


	// Get the start day of week.  If the original repeat type, that is the
	// repeat type when the dialog was first displayed, is weekly then
	// use the start date in the repeat info (the unedit data), otherwise
	// use the current start of week.
	if (repeatP->repeatType == repeatWeekly)
		repeatP->repeatStartOfWeek = (uint8_t) StartDayOfWeek;
	else
		repeatP->repeatStartOfWeek = 0; 	// For all other repeat types, the repeatStartOfWeek field is meaningless.


	// If the repeat type is weekly, get the day of the week the event
	// repeats on.
	if (repeatP->repeatType == repeatWeekly)
	{
		repeatP->repeatOn = 0;
		index = FrmGetObjectIndex (sFormP, RepeatDayOfWeek1PushButton);
		for (i = 0; i < daysInWeek ; i++)
		{
			if (FrmGetControlValue(sFormP, (uint16_t)(index + ((i - sRepeatStartOfWeek + daysInWeek) % daysInWeek))))
				repeatP->repeatOn |= (1 << i);
		}
	}

	// If the repeat type is monthly by day, get the day of the month (ex:
	// fisrt Friday) of the start date of the event.
	else if (repeatP->repeatType == repeatMonthlyByDay)
	{
			repeatP->repeatOn = (uint8_t) DayOfMonth(sDate.month, sDate.day, (int16_t)(sDate.year + firstYear));
	}

	// For all other repeat types, the repeatOn field is meaningless.
	else
	{
		repeatP->repeatOn = 0;
	}
}


/***********************************************************************
 *
 * FUNCTION:    RepeatSetUIValues
 *
 * DESCRIPTION: This routine sets the current repeat settings of the
 *              ui gadgets in the repeat dialog box
 *
 * PARAMETERS:  repeatP - pointer to a RepeatInfoType structure.
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		07/10/95	art	Created by Art Lamb
 *		08/03/99	kwk	Copy label ptrs when shifting day-of-week pushbutton titles,
 *							not just the first byte of each label.
 *
 ***********************************************************************/
static void RepeatSetUIValues (RepeatInfoPtr repeatP)
{
	uint16_t		i;
	uint16_t		id;
	uint16_t		oldFreq;
	MemHandle		freqH;
	char*			freqP;
	Boolean			on;
	FieldPtr		fld;
	RectangleType	fieldBounds;

	// Set the selection of the "repeat type" push button group.
	id = repeatP->repeatType + RepeatNone;

	if (repeatP->repeatType > repeatMonthlyByDay)
		id--;

	FrmSetControlGroupSelection (sFormP, RepeatTypeGroup, id);

	// Set the frequency field
	if (repeatP->repeatType != repeatNone)
	{
		fld = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatFrequenceField));
		freqH = FldGetTextHandle (fld);

		if (freqH)
		{
			freqP = MemHandleLock (freqH);
			oldFreq = (uint16_t) StrAToI (freqP);
		}
		else
		{
			freqH = MemHandleNew (maxFrequenceFieldLen);
			freqP = MemHandleLock (freqH);
			oldFreq = 0;
		}

		if (oldFreq != repeatP->repeatFrequency)
		{
			StrIToA (freqP, repeatP->repeatFrequency);
			FldSetTextHandle (fld, freqH);

			if (FrmVisible (sFormP))
			{
				FldGetBounds (fld, &fieldBounds);
				WinInvalidateRect(FrmGetWindowHandle(sFormP), &fieldBounds);
			}
		}
		MemHandleUnlock (freqH);
	}


	// Set the selection of the "repeat on" push button groups.
	if (repeatP->repeatType == repeatWeekly)
	{
		// If the appointment has a different start-day-of-week than
		// the dialog-box's current start-day-of-week, rearrange the
		//	labels on the days-of-week push buttons.
		//	Note that this will only handle button-label shifts of one day.
		if (StartDayOfWeek != sRepeatStartOfWeek)
		{
			const char* sundayLabel = CtlGetLabel (FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatDayOfWeek1PushButton)));

			for (id = RepeatDayOfWeek1PushButton; id < RepeatDayOfWeek7PushButton; id++)
			{
				CtlSetLabel(FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, id)),
					CtlGetLabel(FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, (uint16_t)(id + 1)))));
			}

			CtlSetLabel(FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatDayOfWeek7PushButton)), sundayLabel);
			sRepeatStartOfWeek = StartDayOfWeek;
		}

		// Turn on the push buttons for the days the appointment repeats on.
		for (i = 0; i < daysInWeek; i++)
		{
			on = ((repeatP->repeatOn & (1 << i) ) != 0);
			id = RepeatDayOfWeek1PushButton + ((i - sRepeatStartOfWeek + daysInWeek) % daysInWeek);
			CtlSetValue (FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, id)), on);
		}
	}


	// Set the selection of the "repeat by" push button groups.
	FrmSetControlGroupSelection (sFormP, RepeatByGroup, (repeatP->repeatType == repeatMonthlyByDate) ? RepeatByDatePushButon : RepeatByDayPushButon);

	// Set the "end on" trigger label and popup list selection.
	if (repeatP->repeatType != repeatNone)
		RepeatSetDateTrigger (repeatP->repeatEndDate);
}

/***********************************************************************
 *
 * FUNCTION:    	RepeatDrawDescCallBack
 *
 * DESCRIPTION: 	Callback for WinRectangleInvalidateFunc().
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	06/16/03	Initial revision.
 *
 ***********************************************************************/
static Boolean RepeatDrawDescCallBack (int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	FormPtr		formP = (FormType *) state;
	FieldPtr 	fldP;

	if (cmd == winInvalidateDestroy)
		return true;

	fldP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, RepeatDescField));
	FldDrawField (fldP);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    RepeatUpdateDescription
 *
 * DESCRIPTION: This routine draws the text description of the current
 *              repeat type and frequency.
 *
 *              The description is created from a template string.  The
 *              repeat type and frequency determines which template is
 *              used.  The template may contain one or more of the
 *              following token:
 *                   ^d - day name (ex: Monday)
 *							^f - frequency
 *                   ^x - day of the month ordinal number (ex: 1st - 31th)
 *                   ^m - month name (ex: July)
 *                   ^w - week ordinal number (1st, 2nd, 3rd, 4th, or last)
 *
 * PARAMETERS:  None
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		07/06/95	art	Created by Art Lam
 *		03/02/99	grant	Only do week ordinal substitution in the repeatMonthlyByDay case
 *		08/04/99	kwk	Use explicit string resources for days/months/years versus
 *							borrowing from system, so we can lo
 calize properly.
 *		10/21/99	gap	Remove rectangle drawing from proc.  Now a gadget in form.
 *		11/05/99	gap	Always use full day of week name when repeat.repeatType is repeatMonthlyByDay.
 *
 ***********************************************************************/
static void RepeatUpdateDescription (void)
{
	uint8_t 		repeatOn;
	uint16_t 		i;
	uint16_t 		len;
	uint16_t 		freq;
	uint16_t 		dayOfWeek;
	uint16_t 		templateId = 0;
	uint16_t 		repeatOnCount = 0;
	char* 			descP;
	char* 			resP;
	char* 			saveResP;
	MemHandle 		descH;
	MemHandle 		resH;
	FieldPtr 		fldP;
	RectangleType 	fieldRect;
	uint16_t		fieldIndex;
//	RepeatInfoType 	repeat;

	// Get the current setting of the repeat ui gadgets.
	RepeatGetUIValues (&sRepeatInfo[sCurrentRepeatIndex]);

	// Determine which template string to use.  The template string is
	// used to format the description string. Note that we could add
	// a soft constant which tells us whether we need to use different
	// strings for freq == 1 case (depends on language), thus saving space.
	freq = sRepeatInfo[sCurrentRepeatIndex].repeatFrequency;
	switch (sRepeatInfo[sCurrentRepeatIndex].repeatType)
	{
		case repeatNone:
			templateId = repeatNoneStrID;
			break;

		case repeatDaily:
			if (freq == 1)
				// "Every day"
				templateId = everyDayRepeatStrID;
			else
				// "Every [other | 2nd | 3rd...] day"
				templateId = dailyRepeatStrID;
			break;

		case repeatWeekly:
			if (freq == 1)
				// "Every week on [days of week]"
				templateId = everyWeekRepeat1DayStrID;
			else
				templateId = weeklyRepeat1DayStrID;

			// Generate offset to appropriate string id,
			// based on # of days that we need to append.
			for (i = 0; i < daysInWeek; i++)
			{
				if (sRepeatInfo[sCurrentRepeatIndex].repeatOn & (1 << i) ) repeatOnCount++;
			}
			templateId += (repeatOnCount - 1);
			break;

		case repeatMonthlyByDate:
			if (freq == 1)
				// "The ^w ^d of every month"
				templateId = everyMonthByDateRepeatStrID;
			else
				templateId = monthtlyByDateRepeatStrID;
			break;

		case repeatMonthlyByDay:
			if (freq == 1)
				templateId = everyMonthByDayRepeatStrID;
			else
				templateId = monthtlyByDayRepeatStrID;
			break;

		case repeatYearly:
			if (freq == 1)
				templateId = everyYearRepeatStrID;
			else
				templateId = yearlyRepeatStrID;
			break;

		default:
			ErrNonFatalDisplay("Unknown repeat type");
			break;
	}

	// Allocate a block to hold the description and copy the template
	// string into it.
	resH = DmGetResource (gApplicationDbP, strRsc, templateId);
	resP = MemHandleLock (resH);
	descH = MemHandleNew (MemPtrSize(resP));
	descP = MemHandleLock (descH);
	strcpy (descP, resP);
	MemHandleUnlock (resH);
	DmReleaseResource (resH);

	// Substitute the month name string for the month name token.
	resH = DmGetResource (gApplicationDbP, strRsc, repeatMonthNamesStrID);
	resP = MemHandleLock (resH);
	for (i = 1; i < sDate.month; i++)
		resP = StrChr (resP, spaceChr) + 1;

	len = (uint16_t) (StrChr (resP, spaceChr) - resP);
	descP = SubstituteStr (descP, monthNameToken, resP, len);
	MemHandleUnlock (resH);
	DmReleaseResource (resH);

	// Substitute the day name string for the day name token.
	if ( (repeatOnCount == 1)  || (sRepeatInfo[sCurrentRepeatIndex].repeatType == repeatMonthlyByDay) )
		templateId = repeatFullDOWNamesStrID;
	else
		templateId = repeatShortDOWNamesStrID;

	resH = DmGetResource (gApplicationDbP, strRsc, templateId);
	resP = MemHandleLock (resH);
	if (sRepeatInfo[sCurrentRepeatIndex].repeatType == repeatWeekly)
	{
		dayOfWeek = sRepeatInfo[sCurrentRepeatIndex].repeatStartOfWeek;
		repeatOn = sRepeatInfo[sCurrentRepeatIndex].repeatOn;
		saveResP = resP;
		while (StrStr (descP, dayNameToken))
		{
			for (i = 0; i < daysInWeek; i++)
			{
				if (repeatOn & (1 << dayOfWeek) )
				{
					repeatOn &= ~(1 << dayOfWeek);
					break;
				}
				dayOfWeek = (dayOfWeek + 1 + daysInWeek) % daysInWeek;
			}
			resP = saveResP;
			for (i = 0; i < dayOfWeek; i++)
				resP = StrChr (resP, spaceChr) + 1;

			len = (uint16_t) (StrChr (resP, spaceChr) - resP);
			descP = SubstituteStr (descP, dayNameToken, resP, len);
		}
	}
	else
	{
		dayOfWeek = DayOfWeek(sDate.month, sDate.day, (int16_t)(sDate.year + firstYear));
		for (i = 0; i < dayOfWeek; i++)
			resP = StrChr (resP, spaceChr) + 1;

		len = (uint16_t) (StrChr (resP, spaceChr) - resP);
		descP = SubstituteStr (descP, dayNameToken, resP, len);
	}
	MemHandleUnlock (resH);
	DmReleaseResource (resH);

	// Substitute the repeat frequency string for the frequency token. Note that
	// we do something special for 2nd (other), since the gender of 'other' changes
	// for some languages, depending on whether the next word is day, month, week,
	// or year.
	if (freq == 2)
	{
		char otherFreqName[16];
		//uint16_t index = (uint16_t)sRepeatInfo.repeatType - (uint16_t)repeatNone;

		SysStringByIndex(gApplicationDbP, freqOrdinal2ndStrlID, sCurrentRepeatIndex, otherFreqName, sizeof(otherFreqName));
		descP = SubstituteStr (descP, frequenceToken, otherFreqName, strlen(otherFreqName));
	}
	else
	{
		resH = DmGetResource (gApplicationDbP, strRsc, freqOrdinalsStrID);
		resP = MemHandleLock (resH);
		for (i = 1; i < freq; i++)
			resP = StrChr (resP, spaceChr) + 1;

		len = (uint16_t) (StrChr (resP, spaceChr) - resP);
		descP = SubstituteStr (descP, frequenceToken, resP, len);
		MemHandleUnlock (resH);
		DmReleaseResource (resH);
	}


	// Substitute the repeat week string (1st, 2nd, 3rd, 4th, or last)
	// for the week ordinal token.
	if (sRepeatInfo[sCurrentRepeatIndex].repeatType == repeatMonthlyByDay)
	{
		resH = DmGetResource (gApplicationDbP, strRsc, weekOrdinalsStrID);
		resP = MemHandleLock (resH);
		for (i = 0; i < sRepeatInfo[sCurrentRepeatIndex].repeatOn / daysInWeek; i++)
			resP = StrChr (resP, spaceChr) + 1;

		len = (uint16_t) (StrChr (resP, spaceChr) - resP);
		descP = SubstituteStr (descP, weekOrdinalToken, resP, len);
		MemHandleUnlock (resH);
		DmReleaseResource (resH);
	}
	else
	{
		// make sure the week ordinal token really doesn't appear
		ErrNonFatalDisplayIf(StrStr(descP, weekOrdinalToken) != NULL, "week ordinal not substituted");
	}

	// Substitute the repeat date string (1st, 2nd, ..., 31th) for the
	// day ordinal token.
	resH = DmGetResource (gApplicationDbP, strRsc, dayOrdinalsStrID);
	resP = MemHandleLock (resH);
	for (i = 1; i < sDate.day; i++)
		resP = StrChr (resP, spaceChr) + 1;

	len = (uint16_t) (StrChr (resP, spaceChr) - resP);
	descP = SubstituteStr (descP, dayOrdinalToken, resP, len);
	MemHandleUnlock (resH);
	DmReleaseResource (resH);
	MemHandleUnlock (descH);

	// Invalidate the description field
	fieldIndex = FrmGetObjectIndex (sFormP, RepeatDescField);
	fldP = FrmGetObjectPtr (sFormP, fieldIndex);
	FldFreeMemory (fldP);
	FldSetTextHandle (fldP, descH);
	FrmGetObjectBounds(sFormP, fieldIndex, &fieldRect);
	//Transparency
	//WinInvalidateRectFunc(FrmGetWindowHandle(sFormP), &fieldRect, RepeatDrawDescCallBack, sFormP);
	WinInvalidateRect(FrmGetWindowHandle(sFormP), &fieldRect);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatSelectEndDate
 *
 * DESCRIPTION: This routine selects the end date of a repeating event.
 *
 * PARAMETERS:  event - pointer to a popup select event
 *
 * RETURNED:    nothing
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/16/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatSelectEndDate (EventType* event)
{
	int16_t month, day, year;
	char* titleP;
	MemHandle titleH;

	// "No due date" items selected?
	if (event->data.popSelect.selection == repeatNoEndDateItem)
	{
		DateToInt (sRepeatInfo[sCurrentRepeatIndex].repeatEndDate) = apptNoEndDate;
	}

	// "Select date" item selected?
	else if (event->data.popSelect.selection == repeatChooseDateItem)
	{
		if ( DateToInt (sRepeatInfo[sCurrentRepeatIndex].repeatEndDate) == apptNoEndDate)
		{
			year = sDate.year + firstYear;
			month = sDate.month;
			day = sDate.day;
		}
		else
		{
			year = sRepeatInfo[sCurrentRepeatIndex].repeatEndDate.year + firstYear;
			month = sRepeatInfo[sCurrentRepeatIndex].repeatEndDate.month;
			day = sRepeatInfo[sCurrentRepeatIndex].repeatEndDate.day;
		}

		titleH = DmGetResource (gApplicationDbP, strRsc, endDateTitleStrID);
		titleP = (char*) MemHandleLock (titleH);

		if (SelectDay (selectDayByDay, &month, &day, &year, titleP))
		{
			sRepeatInfo[sCurrentRepeatIndex].repeatEndDate.day = day;
			sRepeatInfo[sCurrentRepeatIndex].repeatEndDate.month = month;
			sRepeatInfo[sCurrentRepeatIndex].repeatEndDate.year = year - firstYear;

			// Make sure the end date is not before the start date.
			if (DateToInt(sRepeatInfo[sCurrentRepeatIndex].repeatEndDate) < DateToInt (sDate))
			{
				SndPlaySystemSound (sndError);
				DateToInt (sRepeatInfo[sCurrentRepeatIndex].repeatEndDate) = apptNoEndDate;
			}
		}

		MemHandleUnlock (titleH);
	   	DmReleaseResource (titleH);
	}

	RepeatSetDateTrigger (sRepeatInfo[sCurrentRepeatIndex].repeatEndDate);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatChangeRepeatOn
 *
 * DESCRIPTION: This routine is called when one of the weekly "repeat on"
 *              push button is pushed.  This routine checks
 *              if all the buttons has been turned off,  if so the day
 *              of the week of the appointment's start date is turn on.
 *
 * PARAMETERS:  event - pointer to and event
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/7/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatChangeRepeatOn (EventType* event)
{
	uint16_t i;
	uint16_t id;
	uint16_t index;
	uint16_t dayOfWeek;
	Boolean on;

	// Check if any of the buttons are on.
	index = FrmGetObjectIndex (sFormP, RepeatDayOfWeek1PushButton);
	on = false;
	for (i = 0; i < daysInWeek; i++)
	{
		if (FrmGetControlValue(sFormP, (uint16_t)(index + i)) != 0)
		{
			on = true;
			break;
		}
	}

	// If all the buttons are off, turn on the start date's button.
	if (! on)
	{
		dayOfWeek = DayOfWeek (sDate.month, sDate.day, (int16_t)(sDate.year + firstYear));
		dayOfWeek = (dayOfWeek - StartDayOfWeek + daysInWeek) % daysInWeek;

		id = RepeatDayOfWeek1PushButton + dayOfWeek;
		CtlSetValue (FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, id)), true);
	}

	// Update the display of the repeat descrition.
	RepeatUpdateDescription ();
}


/***********************************************************************
 *
 * FUNCTION:    RepeatChangeType
 *
 * DESCRIPTION: This routine changes the ui gadgets in the repeat dialog
 *              such that they match the newly selected repeat type.  The
 *              routine is called when one of the repeat type push buttons
 *              are pushed.
 *
 * PARAMETERS:  controlID - Id of the selected control
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/07/95	Initial Revision
 *			gap	10/28/99	End on value maintained last selected date
 *								after switching repeat type instead of returning to
 *								"no end date" also reset sRepeatInfo.repeatEndDate global such that
 * 							if end date is "no end date" and "choose" is selected from
 *								popup - calendar always defaults to current day.
 *
 ***********************************************************************/
static void RepeatChangeType (uint16_t	controlID)
{
	uint16_t 		id;
	RepeatType 		oldType;
	RepeatType 		newType;
	RectangleType 	infoBox;

	// If the type if monthly default to monthly-by-date.
	newType = (RepeatType) (controlID - RepeatNone);

	oldType = sRepeatInfo[sCurrentRepeatIndex].repeatType;

	if (oldType > repeatMonthlyByDay)
		oldType--;

	if (oldType == newType)
		return;

	// Save old values.
	RepeatGetUIValues(&sRepeatInfo[sCurrentRepeatIndex]);

	// Hide the ui gadgets that are unique to the repeat type we are no longer editing.
	switch (sRepeatInfo[sCurrentRepeatIndex].repeatType)
	{
		case repeatNone:
			HideObject (sFormP, RepeatNoRepeatLabel);
			break;

		case repeatDaily:
			HideObject (sFormP, RepeatDaysLabel);
			break;

		case repeatWeekly:
			HideObject (sFormP, RepeatWeeksLabel);
			for (id = RepeatRepeatOnLabel; id <= RepeatDayOfWeek7PushButton; id++)
				HideObject (sFormP, id);
			break;

		case repeatMonthlyByDay:
		case repeatMonthlyByDate:
			HideObject (sFormP, RepeatMonthsLabel);
			for (id = RepeatByLabel; id <= RepeatByDatePushButon; id++)
				HideObject (sFormP, id);
			break;

		case repeatYearly:
			HideObject (sFormP, RepeatYearsLabel);
			break;
	}


	// Handle switching to or from "no" repeat.
	if (sRepeatInfo[sCurrentRepeatIndex].repeatType == repeatNone)
	{
		ShowObject (sFormP, RepeatEveryLabel);
		ShowObject (sFormP, RepeatFrequenceField);
		ShowObject (sFormP, RepeatEndOnLabel);
		ShowObject (sFormP, RepeatEndOnTrigger);
	}
	else if (newType == repeatNone)
	{
		HideObject (sFormP, RepeatEveryLabel);
		HideObject (sFormP, RepeatFrequenceField);
		HideObject (sFormP, RepeatEndOnLabel);
		HideObject (sFormP, RepeatEndOnTrigger);
	}


	// Set the current group values.
	sCurrentRepeatIndex = newType;
	RepeatSetUIValues (&sRepeatInfo[sCurrentRepeatIndex]);

	// Show the ui object that are appropriate for the new repeat type.
	switch (sRepeatInfo[sCurrentRepeatIndex].repeatType)
	{
		case repeatNone:
			ShowObject (sFormP, RepeatNoRepeatLabel);
			break;

		case repeatDaily:
			ShowObject (sFormP, RepeatDaysLabel);
			break;

		case repeatWeekly:
			ShowObject (sFormP, RepeatWeeksLabel);
			ShowObject (sFormP, RepeatRepeatOnLabel);
			for (id = RepeatRepeatOnLabel; id <= RepeatDayOfWeek7PushButton; id++)
			{
				ShowObject (sFormP, id);
			}
			break;

		case repeatMonthlyByDay:
		case repeatMonthlyByDate:
			ShowObject (sFormP, RepeatMonthsLabel);
			ShowObject (sFormP, RepeatByLabel);
			ShowObject (sFormP, RepeatByDayPushButon);
			ShowObject (sFormP, RepeatByDatePushButon);
			break;

		case repeatYearly:
			ShowObject (sFormP, RepeatYearsLabel);
			break;
	}

	// Update the display of the repeat descrition.
	RepeatUpdateDescription ();

	// Invalidate the information box
	FrmGetObjectBounds(sFormP, FrmGetObjectIndex (sFormP, RepeatInfoRectGadget), &infoBox);
	WinInvalidateRect(FrmGetWindowHandle(sFormP), &infoBox);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatApply
 *
 * DESCRIPTION: This routine applies the changes made in the Repeat Dialog.
 *              The changes or copied to a block intiialize by the
 *              Details Dialog,  they a not written to the database until
 *              the Details Dialog is applied.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/24/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatApply (void)
{
	RepeatGetUIValues (&sRepeatInfo[sCurrentRepeatIndex]);

	ErrNonFatalDisplayIf(!ApptDetailsP, "Can't apply repeat info");

	// If RepeatDetialsP is not null, we came from the details dialog.
	// So, we update the repeat info struct for the current event details
	((DetailsType*)ApptDetailsP)->repeat = sRepeatInfo[sCurrentRepeatIndex];
}

/***********************************************************************
 *
 * FUNCTION:    RepeatDescRectHandler
 *
 * DESCRIPTION: This routine is the event handler for rectangle gadget
 *					 surrounding the repeat description in the "Repeat
 *              Dialog Box".
 *
 *					 Instead of drawing a static rectangle the size of the
 *					 gadget bounds, I have sized the gadget to be the full area
 *					 of the repeat dialog the description field will still fit
 *					 should the field have to be slightly resized for any reason.
 *					 The bounding rect drawn is calculated from the field's
 *					 bounding rect.
 *
 * PARAMETERS:  gadgetP	- pointer to the gadget
 *					 cmd		- the event type to be handled
 *					 paramp	- any additional data that is passed to the gadget
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/21/99	Initial Revision
 *
 ***********************************************************************/
static Boolean RepeatDescRectHandler(FormGadgetType *gadgetP, uint16_t cmd, void *paramP)
{
	FieldPtr 		fld;
  	RectangleType	r;

	switch(cmd)
	{
		case formGadgetEraseCmd:
		case formGadgetDrawCmd:

			// get the repeat description field and calculate a bounding box
			fld = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatDescField));
			FldGetBounds (fld, &r);
			RctInsetRectangle (&r, -4);

			if (cmd == formGadgetDrawCmd)
				WinDrawRectangleFrame (simpleFrame, &r);
			else
				WinEraseRectangle (&r, 0);
			break;

		case formGadgetHandleEventCmd:
			return false;
			break;

		default:
			break;
	}

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatInitInfo
 *
 * DESCRIPTION: Initialize the repeat structure for each repeat type.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/10/04	Initial Revision
 *
 ***********************************************************************/
static void RepeatInitInfo(void)
{
	uint16_t dayOfWeek;
	uint16_t i;

	memmove(&sDate, &((DetailsType*)ApptDetailsP)->when.date, sizeof(sDate));
	memset(&sRepeatInfo, 0, sizeof(RepeatInfoType) * kMaxRepeatInfo);

	for (i = 0; i < kMaxRepeatInfo; i++)
	{
		DateToInt(sRepeatInfo[i].repeatEndDate) = (uint16_t) defaultRepeatEndDate;
		sRepeatInfo[i].repeatFrequency = defaultRepeatFrequency;
		sRepeatInfo[i].repeatStartOfWeek = (uint8_t) StartDayOfWeek;

		// Only 5 slots, but 6 types. Month is divided in MonthByDay and MonthByDate.
		// Default is MonthByDate, so skip the MonthByDay value.
		sRepeatInfo[i].repeatType = repeatNone + i + (i > repeatWeekly ? 1 : 0);

		if (sRepeatInfo[i].repeatType == repeatWeekly)
		{
			dayOfWeek = DayOfWeek (sDate.month, sDate.day, (int16_t)(sDate.year+firstYear));
			sRepeatInfo[i].repeatOn = (1 << dayOfWeek);
		}
		else if (sRepeatInfo[i].repeatType == repeatMonthlyByDate)
		{
			sRepeatInfo[i].repeatOn = (uint8_t)DayOfMonth(sDate.month, sDate.day, (int16_t)(sDate.year + firstYear));
		}
	}

	sCurrentRepeatIndex = ((DetailsType*)ApptDetailsP)->repeat.repeatType;

	if (sCurrentRepeatIndex > repeatMonthlyByDay)
		sCurrentRepeatIndex--;

	sRepeatInfo[sCurrentRepeatIndex] = ((DetailsType*)ApptDetailsP)->repeat;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatInit
 *
 * DESCRIPTION: This routine initializes tthe "Repeat Dialog Box".  All
 *              the object in the dialog are initialize, even if they
 *              are not used given the current repeat type.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/4/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatInit (void)
{
	uint16_t	id;

	sFormP = FrmGetActiveForm ();

	ErrNonFatalDisplayIf(!ApptDetailsP, "Can't init repeat info");

	RepeatInitInfo();

	// Set usable the ui object that are appropriate for the given repeat
	// type.
	switch (sRepeatInfo[sCurrentRepeatIndex].repeatType)
	{
		case repeatNone:
			ShowObject (sFormP, RepeatNoRepeatLabel);
			break;

		case repeatDaily:
			ShowObject (sFormP, RepeatEveryLabel);
			ShowObject (sFormP, RepeatFrequenceField);
			ShowObject (sFormP, RepeatDaysLabel);
			ShowObject (sFormP, RepeatEndOnLabel);
			ShowObject (sFormP, RepeatEndOnTrigger);
			break;

		case repeatWeekly:
			ShowObject (sFormP, RepeatEveryLabel);
			ShowObject (sFormP, RepeatFrequenceField);
			ShowObject (sFormP, RepeatWeeksLabel);
			ShowObject (sFormP, RepeatRepeatOnLabel);
			ShowObject (sFormP, RepeatEndOnLabel);
			ShowObject (sFormP, RepeatEndOnTrigger);
			for (id = RepeatDayOfWeek1PushButton; id <= RepeatDayOfWeek7PushButton; id++)
			{
				ShowObject (sFormP, id);
			}
			break;

		case repeatMonthlyByDay:
		case repeatMonthlyByDate:
			ShowObject (sFormP, RepeatEveryLabel);
			ShowObject (sFormP, RepeatFrequenceField);
			ShowObject (sFormP, RepeatMonthsLabel);
			ShowObject (sFormP, RepeatByLabel);
			ShowObject (sFormP, RepeatByDayPushButon);
			ShowObject (sFormP, RepeatByDatePushButon);
			ShowObject (sFormP, RepeatEndOnLabel);
			ShowObject (sFormP, RepeatEndOnTrigger);
			break;

		case repeatYearly:
			ShowObject (sFormP, RepeatEveryLabel);
			ShowObject (sFormP, RepeatFrequenceField);
			ShowObject (sFormP, RepeatYearsLabel);
			ShowObject (sFormP, RepeatEndOnLabel);
			ShowObject (sFormP, RepeatEndOnTrigger);
			break;

	}

	FrmSetGadgetHandler(sFormP, FrmGetObjectIndex(sFormP, RepeatDescRectGadget), (FormGadgetHandlerType*) &RepeatDescRectHandler);

	sRepeatStartOfWeek = sunday;

	RepeatSetUIValues (&sRepeatInfo[sCurrentRepeatIndex]);

	sCurrentNavObjectID = frmInvalidObjectId;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatHandleNavigationUp
 *
 * DESCRIPTION: Handle UI Navigation for the Up key.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/06/04	Initial Revision
 *
 ***********************************************************************/
static Boolean RepeatHandleNavigationUp(Boolean objectFocusMode, Boolean repeat)
{
	uint16_t		i;
	uint16_t		id = RepeatDayOfWeek1PushButton + ((0 - sRepeatStartOfWeek + daysInWeek) % daysInWeek);
	Boolean			on = false;
	Boolean			handled = false;

	if (objectFocusMode)
	{
			switch (sCurrentNavObjectID)
		{
			case RepeatOkButton:
			case RepeatCancelButton:
			{
				switch(sRepeatInfo[sCurrentRepeatIndex].repeatType)
				{
					case repeatWeekly:
						// Turn on the push buttons for the days the appointment repeats on.
						for (i = 0; (i < daysInWeek) && !on; i++)
						{
							on = ((sRepeatInfo[sCurrentRepeatIndex].repeatOn & (1 << i) ) != 0);

							if (on)
								id = RepeatDayOfWeek1PushButton + ((i - sRepeatStartOfWeek + daysInWeek) % daysInWeek);
						}

						// force interaction mode to the 'New' button.
						FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, id));
						FrmNavObjectTakeFocus(sFormP, id);
						handled = true;
						break;

					case repeatMonthlyByDay:
						// force interaction mode to the 'New' button.
						FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, RepeatByDayPushButon));
						FrmNavObjectTakeFocus(sFormP, RepeatByDayPushButon);
						handled = true;
						break;

					case repeatMonthlyByDate:
						// force interaction mode to the 'New' button.
						FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, RepeatByDatePushButon));
						FrmNavObjectTakeFocus(sFormP, RepeatByDatePushButon);
						handled = true;
						break;
				}
				break;
			}

			case RepeatFrequenceField:
				// Set the selection of the "repeat type" push button group.
				id = sRepeatInfo[sCurrentRepeatIndex].repeatType + RepeatNone;

				if (sRepeatInfo[sCurrentRepeatIndex].repeatType > repeatMonthlyByDay)
					id--;

				// force interaction mode to the 'New' button.
				FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, id));
				FrmNavObjectTakeFocus(sFormP, id);
				handled = true;
				break;
		}
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatHandleNavigationDown
 *
 * DESCRIPTION: Handle UI Navigation for the Down key.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/06/04	Initial Revision
 *
 ***********************************************************************/
static Boolean RepeatHandleNavigationDown(Boolean objectFocusMode, Boolean repeat)
{
	uint16_t		i;
	uint16_t		id = RepeatDayOfWeek1PushButton + ((0 - sRepeatStartOfWeek + daysInWeek) % daysInWeek);
	Boolean			on = false;
	Boolean			handled = false;

	if (objectFocusMode)
	{
		switch (sCurrentNavObjectID)
		{
			case RepeatEndOnTrigger:
			{
				switch(sRepeatInfo[sCurrentRepeatIndex].repeatType)
				{
					case repeatWeekly:
						// Turn on the push buttons for the days the appointment repeats on.
						for (i = 0; (i < daysInWeek) && !on; i++)
						{
							on = ((sRepeatInfo[sCurrentRepeatIndex].repeatOn & (1 << i) ) != 0);

							if (on)
								id = RepeatDayOfWeek1PushButton + ((i - sRepeatStartOfWeek + daysInWeek) % daysInWeek);
						}

						// force interaction mode to the 'New' button.
						FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, id));
						FrmNavObjectTakeFocus(sFormP, id);
						handled = true;
						break;

					case repeatMonthlyByDay:
						// force interaction mode to the 'New' button.
						FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, RepeatByDayPushButon));
						FrmNavObjectTakeFocus(sFormP, RepeatByDayPushButon);
						handled = true;
						break;

					case repeatMonthlyByDate:
						// force interaction mode to the 'New' button.
						FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, RepeatByDatePushButon));
						FrmNavObjectTakeFocus(sFormP, RepeatByDatePushButon);
						handled = true;
						break;
				}
				break;
			}

			case frmNavTitleID:
				// Set the selection of the "repeat type" push button group.
				id = sRepeatInfo[sCurrentRepeatIndex].repeatType + RepeatNone;

				if (sRepeatInfo[sCurrentRepeatIndex].repeatType > repeatMonthlyByDay)
					id--;

				// force interaction mode to the 'New' button.
				FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, id));
				FrmNavObjectTakeFocus(sFormP, id);
				handled = true;
				break;
		}
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatHandleNavigationLeft
 *
 * DESCRIPTION: Handle UI Navigation for the Left key.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/06/04	Initial Revision
 *
 ***********************************************************************/
static Boolean RepeatHandleNavigationLeft(Boolean objectFocusMode, Boolean repeat)
{
	uint16_t	id;

	if (objectFocusMode)
	{
		switch(sCurrentNavObjectID)
		{
			case RepeatOkButton:
			case RepeatFrequenceField:
				// Set the selection of the "repeat type" push button group.
				id = sRepeatInfo[sCurrentRepeatIndex].repeatType + RepeatNone;

				if (sRepeatInfo[sCurrentRepeatIndex].repeatType > repeatMonthlyByDay)
					id--;

				// force interaction mode to the 'New' button.
				FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, id));
				FrmNavObjectTakeFocus(sFormP, id);

				return true;
				break;

			case RepeatNone:
			case RepeatDayOfWeek1PushButton:
				if (repeat)
					return true;
				break;
		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatHandleNavigationRight
 *
 * DESCRIPTION: Handle UI Navigation for the Right key.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    true if the event has been handled
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * LFe	08/06/04	Initial Revision
 *
 ***********************************************************************/
static Boolean RepeatHandleNavigationRight(Boolean objectFocusMode, Boolean repeat)
{
	if (objectFocusMode)
	{
		switch(sCurrentNavObjectID)
		{
			case RepeatYearly:
			case RepeatDayOfWeek7PushButton:
				if (repeat)
					return true;

			// FALLBACK: no break

			case RepeatFrequenceField:
			case RepeatEndOnTrigger:
			case RepeatByDatePushButon:
				break;

			default: return false;

		}
	}
	else FrmSetNavState(sFormP, kFrmNavStateFlagsObjectFocusMode);

	// force interaction mode to the 'New' button.
	FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, RepeatOkButton));
	FrmNavObjectTakeFocus(sFormP, RepeatOkButton);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    RepeatHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Repeat
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95	Initial Revision
 *			CSS	06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *			gap	10/15/99	Added added handling of frmUpdateEvent
 *			gap	11/01/00	Update the description if the user changes the
 *								repeat every field via the silkscreen keyboard.
 *			gap	01/09/01 Remove double update caused by edit of a selection
 *								of text in the "Repeat Every" field.
 *
 ***********************************************************************/
Boolean RepeatHandleEvent (EventType* event)
{
	Boolean 	handled = false;
	Boolean 	noSelection;
	Boolean		objectFocusMode;
	Boolean		repeat;
	FrmNavStateFlagsType	navState;
	size_t 		startPos;
	size_t		endPos;
	FieldPtr 	fld;
	wchar32_t 	chr;

	switch (event->eType)
	{
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case RepeatOkButton:
					RepeatApply ();

					FrmReturnToForm(DetailsDialog);

					DetailsSetRepeatingTrigger(ApptDetailsP);

					handled = true;
					break;

				case RepeatCancelButton:
					// Give control of the details info to the Details form
					FrmReturnToForm(DetailsDialog);
					handled = true;
					break;

				case RepeatNone:
				case RepeatDaily:
				case RepeatWeekly:
				case RepeatMonthly:
				case RepeatYearly:
					RepeatChangeType (event->data.ctlSelect.controlID);
					handled = true;
					break;

				case RepeatDayOfWeek1PushButton:
				case RepeatDayOfWeek2PushButton:
				case RepeatDayOfWeek3PushButton:
				case RepeatDayOfWeek4PushButton:
				case RepeatDayOfWeek5PushButton:
				case RepeatDayOfWeek6PushButton:
				case RepeatDayOfWeek7PushButton:
					RepeatChangeRepeatOn (event);
					handled = true;
					break;

				case RepeatByDayPushButon:
				case RepeatByDatePushButon:
					RepeatUpdateDescription ();
					handled = true;
					break;

				default:;
			}
			break;

		case keyDownEvent:
			if (!TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{
				if (EvtKeydownIsVirtual(event))
				{
					objectFocusMode = (Boolean) (FrmGetNavState(sFormP, &navState) >= errNone && (navState & kFrmNavStateFlagsObjectFocusMode) != 0);
					repeat = event->data.keyDown.modifiers & autoRepeatKeyMask;

					switch(event->data.keyDown.chr)
					{
						case vchrRockerUp:
							handled = RepeatHandleNavigationUp(objectFocusMode, repeat);
							break;

						case vchrRockerDown:
							handled = RepeatHandleNavigationDown(objectFocusMode, repeat);
							break;

						case vchrRockerLeft:
							handled = RepeatHandleNavigationLeft(objectFocusMode, repeat);
							break;

						case vchrRockerRight:
							handled = RepeatHandleNavigationRight(objectFocusMode, repeat);
							break;
					}
				}
				else
				{
					chr = event->data.keyDown.chr;
					if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
					{
						fld = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, RepeatFrequenceField));
						FldGetSelection(fld, &startPos, &endPos);
						noSelection = (startPos == endPos);
						FldHandleEvent (fld, event);

						// There are 3 ways that the repeat every value can be
						// updated
						//		1) by the soft keyboard (on silkscreen)
						//		2) via graffiti entry in the form with a selection range
						// 	3) via graffiti entry in the form with no selection range
						//			(ie just an insertion cursor)
						// Methods 1 & 2 result in a fldChangedEvent being posted so the
						// update will be handled in response to that event.  ONLY when
						// there is no selection range replacement, do we do the update here
						// to avoid a double redraw of the description field.
						if (noSelection)
							RepeatUpdateDescription ();
					}
					handled = true;
				}
			}
			break;

		case fldChangedEvent:
			// when the user changes the "repeat every" value via the
			// soft keyboard or in the form via a selection replacement
			// a fldChangedEvent is posted. Update the description in
			// response
			RepeatUpdateDescription ();
			handled = true;
			break;

		case popSelectEvent:
			if (event->data.popSelect.listID == RepeatEndOnList)
			{
				RepeatSelectEndDate (event);
				handled = true;
			}
			break;

		case frmOpenEvent:
			RepeatInit ();
			RepeatUpdateDescription ();
			//FrmSetFocus (sFormP, FrmGetObjectIndex (sFormP, RepeatFrequenceField));
			handled = true;
			break;

		case winUpdateEvent:
			if (event->data.winUpdate.window != FrmGetWindowHandle(sFormP))
				break;

			FrmDrawForm (sFormP);
			handled = true;
			break;

		case frmCloseEvent:
			handled = true;
			break;

		case frmObjectFocusTakeEvent:
			sCurrentNavObjectID = event->data.frmObjectFocusTake.objectID;
			break;
	}

	return (handled);
}
