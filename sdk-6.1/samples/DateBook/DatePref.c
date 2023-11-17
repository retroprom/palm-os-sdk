/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DatePref.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This module contains the routines that handle the Datebook 
 *   applications's preferences.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <CatMgr.h>
#include <CmnRectTypes.h>
#include <Control.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <FormLayout.h>
#include <List.h>
#include <PenInputMgr.h>
#include <Preferences.h>
#include <SoundMgr.h>
#include <string.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <UIResources.h>

#include "DateGlobals.h"
#include "DateAlarm.h"
#include "DatePref.h"
#include "DateUtils.h"
#include "Datebook.h"
#include "DatebookRsc.h"

#define numRepeats 						5
#define numPlayEverys 					4
#define defaultRepeatsLevel				3				
#define defaultPlayEveryLevel			2	

// Max len of sound trigger label placeholder
#define soundTriggerLabelLen			32


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void SetSoundLabel(FormPtr formP, const char* labelP);
static void FreeAll(void);


/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
// Number of times to remind the person
static uint16_t RepeatCountMappings [numRepeats] =
{	
	1, 2, 3, 5, 10
};

// How many seconds between repeats
static uint16_t RepeatIntervalMappings [numPlayEverys] =
{	
	1, 5, 10,  30
};

// Placeholder for sound trigger label
static char * soundTriggerLabelP;

// handle to the list containing names and DB info of MIDI tracks.
// Each entry is of type SndMidiListItemType.
static MemHandle	gMidiListH;
// number of entries in the MIDI list
static uint16_t	gMidiCount;

// The following global variable are only valid while editng the datebook's
// preferences.
static uint16_t	PrefDayStartHour;
static uint16_t	PrefDayEndHour;

// The following globals are for the repeat rates of the alarms preferences.
static uint16_t	PrefSoundRepeatCount;					// number of times to repeat alarm sound 
static uint16_t	PrefSoundRepeatInterval;				// interval between repeat sounds, in seconds

// The following globals are for the repeat rates of the alarms preferences.
static uint32_t	PrefSoundUniqueRecID;					// Alarm sound MIDI file unique ID record identifier


/***********************************************************************
 *
 * FUNCTION:    MidiPickListDrawItem
 *
 * DESCRIPTION: Draw a midi list item.
 *
 * PARAMETERS:  itemNum - which shortcut to draw
 *					 bounds - bounds in which to draw the text
 *					 unusedP - pointer to data (not used)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vnk		8/8/97	Initial version
 *			trev		8/14/97	Ported to dateBook App
 *			frigino	8/18/97	Modified to truncate items in list with ...
 *
 ***********************************************************************/
static void MidiPickListDrawItem (int16_t itemNum, RectangleType *bounds, char **itemsText, ListType *aList)
{
	char *				itemTextP;
	size_t				itemTextLen;
	SndMidiListItemType*	listP;
	
	ErrNonFatalDisplayIf(itemNum >= gMidiCount, "index out of bounds");
	
	// Bail out if MIDI sound list is empty
	if (gMidiListH == NULL)
		return;
	
	listP = MemHandleLock(gMidiListH);

	itemTextP = listP[itemNum].name;

	// Truncate the item with an ellipsis if it doesnt fit in the list width.
	// Get the item text length
	itemTextLen = strlen(itemTextP);
	WinDrawTruncChars(itemTextP, itemTextLen, bounds->topLeft.x, bounds->topLeft.y, bounds->extent.x);

	// Unlock list items
	MemPtrUnlock(listP);
}


/***********************************************************************
 *
 * FUNCTION:    CreateMidiPickList
 *
 * DESCRIPTION: Create a list of midi sounds available.
 *
 * PARAMETERS:	formP	-- the form that owns the list to contain the panel list
 *					objIndex -- the index of the list within the form
 *					funcP	-- item draw function
 *
 * RETURNED:    panelCount and panelIDsP are set
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk		8/8/97	Initial version
 *			trev		8/14/97	Ported to dateBook App
 *			frigino	8/20/97	Added maximum widening of MIDI sound list
 *
 ***********************************************************************/
static void CreateMidiPickList(FormType *formP, uint16_t objIndex, ListDrawDataFuncPtr funcP)
{
	SndMidiListItemType*	midiListP;
	uint16_t				i;
	uint16_t				listWidth;
	uint16_t				maxListWidth;
	Boolean				bSuccess;
	RectangleType 		formBounds, listBounds;
	ListPtr				listP;
		
	// Load list of midi record entries
	bSuccess = SndCreateMidiList(sysFileCSystem, false, &gMidiCount, &gMidiListH);
	if ( !bSuccess )
	{
		gMidiListH = 0;
		gMidiCount = 0;
		return;
	}
	
	listP = FrmGetObjectPtr(formP, objIndex);
		
	// Now set the list to hold the number of sounds found.  There
	// is no array of text to use.
	LstSetListChoices(listP, NULL, gMidiCount);
	
	// Now resize the list to the number of panel found
	LstSetHeight (listP, gMidiCount);

	// Because there is no array of text to use, we need a function
	// to interpret the panelIDsP list and draw the list items.
	LstSetDrawFunction(listP, funcP);

	// Make the list as wide as possible to display the full sound names
	// when it is popped winUp.

	// Lock MIDI sound list
	midiListP = MemHandleLock(gMidiListH);
	
	// Initialize max width
	maxListWidth = 0;
	
	// Iterate through each item and get its width
	for (i = 0; i < gMidiCount; i++)
	{
		// Get the width of this item
		listWidth = FntCharsWidth(midiListP[i].name, strlen(midiListP[i].name));
		
		// If item width is greater that max, swap it
		if (listWidth > maxListWidth)
		{
			maxListWidth = listWidth;
		}
	}
	// Unlock MIDI sound list
	MemPtrUnlock(midiListP);

	// Set list width to max width + left margin
	FrmGetObjectBounds(formP, objIndex, &listBounds);
	listBounds.extent.x = maxListWidth + 2;

	// Get pref dialog window extent
	FrmGetFormInitialBounds(FrmGetActiveForm(), &formBounds);
	
	// Make sure width is not more than window extent
	if (listBounds.extent.x > formBounds.extent.x)
	{
		listBounds.extent.x = formBounds.extent.x;
	}
	// Move list left if it doesnt fit in window
	if (listBounds.topLeft.x + listBounds.extent.x > formBounds.extent.x)
	{
		listBounds.topLeft.x = formBounds.extent.x - listBounds.extent.x;
	}
		
	FrmSetObjectBounds(formP, objIndex, &listBounds);
}

/***********************************************************************
 *
 * FUNCTION:    FreeMidiPickList
 *
 * DESCRIPTION: Free the list of midi sounds available.
 *
 * PARAMETERS:	none
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk	8/11/97	Initial version
 *			trev	08/14/97	Ported to dateBook App
 *
 ***********************************************************************/
static void FreeMidiPickList(void)
{
	if ( gMidiListH )
	{
		MemHandleFree(gMidiListH);
		gMidiListH = 0;
		gMidiCount = 0;
	}
}


/***********************************************************************
 *
 * FUNCTION:    MapToPosition
 *
 * DESCRIPTION:	Map a value to it's position in an array.  If the passed
 *						value is not found in the mappings array, a default
 *						mappings item will be returned.
 *
 * PARAMETERS:  value	- value to look for
 *
 * RETURNED:    position value found in
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			kcr		9/13/95	Initial Revision
 *			frigino	8/21/97	Converted all params to uint16_t
 *
 ***********************************************************************/
static uint16_t MapToPosition (uint16_t* mappingArray, uint16_t value,
									uint16_t mappings, uint16_t defaultItem)
{
	uint16_t i;
	
	i = 0;
	while (mappingArray[i] != value && i < mappings)
	{
		i++;
	}
	
	if (i >= mappings)
		return defaultItem;

	return i;
}	//	end of MapToPosition


/***********************************************************************
 *
 * FUNCTION:    PreferencesUpdateScrollers
 *
 * DESCRIPTION: This routine updates the day-start and day-end time 
 *              scrollers
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *
 ***********************************************************************/
static void PreferencesUpdateScrollers ()
{
	FormPtr frm;
	uint16_t upIndex;
	uint16_t downIndex;
	
	// Update the start time scrollers.
	frm = FrmGetActiveForm ();
	upIndex = FrmGetObjectIndex (frm, PreferStartUpButton);
	downIndex = FrmGetObjectIndex (frm, PreferStartDownButton);
	FrmUpdateScrollers(	frm, upIndex, downIndex,
						(Boolean)(PrefDayStartHour < 23),
						(Boolean)(PrefDayStartHour > 0));
	// Update the end time scrollers.
	upIndex = FrmGetObjectIndex (frm, PreferEndUpButton);
	downIndex = FrmGetObjectIndex (frm, PreferEndDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, 
						(Boolean)(PrefDayEndHour < 23),
						(Boolean)(PrefDayEndHour > 0));
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesAlarmOnOff
 *
 * DESCRIPTION: This routine shows or hides the alarm preset ui object.
 *              It is call when the alarm preset check box is turn on 
 *              or off.
 *
 * PARAMETERS:  on - true to show alarm preset ui, false to hide.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *
 ***********************************************************************/
 static void PreferencesAlarmPresetOnOff (Boolean on)
 {
 	uint16_t fldIndex;
 	uint16_t ctlIndex;
 	char * textP;
 	char * label;
 	MemHandle textH;
 	FormPtr frm;
 	ListPtr lst;
 	FieldPtr fld;
 	ControlPtr ctl;
 
	frm = FrmGetActiveForm ();
	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, PreferAlarmField));

	fldIndex = FrmGetObjectIndex (frm, PreferAlarmField);
	ctlIndex = FrmGetObjectIndex (frm, PreferAlarmUnitTrigger);
	
 	if (on)
 	{
		// Set the value of the alarm advance field.
		textH = FldGetTextHandle (fld);
		if (textH) MemHandleFree (textH);
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, defaultAlarmAdvance);
		MemPtrUnlock (textP);

		FldSetTextHandle (fld, textH);
		
		// Set the alarm advance unit of measure (minutes, hours, or days).
		lst = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, PreferAlarmList));
		LstSetSelection (lst, defaultAdvanceUnit);
		label = LstGetSelectionText (lst, defaultAdvanceUnit);

		ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, PreferAlarmUnitTrigger));
		CtlSetLabel (ctl, label);

		// Show the alarm advance ui objects. 		
		FrmShowObject (frm, fldIndex);
		FrmShowObject (frm, ctlIndex);

		FrmSetFocus (frm, fldIndex);
 	}
 	else
 	{
		FrmSetFocus (frm, noFocus);

		FldFreeMemory (fld);

		// Hide the alarm advance ui objects. 		
		FrmHideObject (frm, fldIndex);
		FrmHideObject (frm, ctlIndex);
 	}
 }

/***********************************************************************
 *
 * FUNCTION:    PreferencesUpdateTimeDisplay
 *
 * DESCRIPTION: This routine invalidates the specified time and invalidates
 *				the rectangle so that it will be updated at next update event.
 *
 * PARAMETERS:  isStartTime - true if we're drawing the start time
 *
 * RETURNED:    nothing
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			PLe		01/09/03	Initial Revision
 *
 ***********************************************************************/
static void PreferencesUpdateTimeDisplay (Boolean isStartTime)
{
	uint16_t id;
	uint16_t index;
	int16_t x, y;
	FormPtr frm;
	RectangleType r;

	// Compute the drawing bounds.
	if (isStartTime)
		id = PreferStartUpButton;
	else
		id = PreferEndUpButton;

	frm = FrmGetActiveForm ();
	index = FrmGetObjectIndex (frm, id);
	FrmGetObjectPosition (frm, index, &x, &y);
	
	r.topLeft.x = x - dayRangeTimeWidth - 5;
	r.topLeft.y = y + 2;
	r.extent.x = dayRangeTimeWidth;
	r.extent.y = dayRangeTimeHeight;

	WinInvalidateRect(WinGetDrawWindow(), &r);
}

/***********************************************************************
 *
 * FUNCTION:    PreferencesDrawTime
 *
 * DESCRIPTION: This routine draw the time passed at the location specified.
 *
 * PARAMETERS:  hour        - hour to draw, minutes is assumed to be zero
 *              isStartTime - true if we're drawing the start time
 *
 * RETURNED:    nothing
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/7/95	Initial Revision
 *
 ***********************************************************************/
static void PreferencesDrawTime (uint8_t hour, Boolean isStartTime)
{
	char str[timeStringLength];
	uint16_t id;
	uint16_t len;
	uint16_t index;
	int16_t x, y;
	FontID curFont;
	FormPtr frm;
	RectangleType r;

	// Compute the drawing bounds.
	if (isStartTime)
		id = PreferStartUpButton;
	else
		id = PreferEndUpButton;

	frm = FrmGetActiveForm ();
	index = FrmGetObjectIndex (frm, id);
	FrmGetObjectPosition (frm, index, &x, &y);
	
	r.topLeft.x = x - dayRangeTimeWidth - 5;
	r.topLeft.y = y + 2;
	r.extent.x = dayRangeTimeWidth;
	r.extent.y = dayRangeTimeHeight;
	
	// Format the time into a string.
	TimeToAscii (hour, 0, TimeFormat, str);
	len = strlen (str);
	
	// Draw a frame around the time.
	WinDrawRectangleFrame (simpleFrame, &r);
	
	WinEraseRectangle (&r, 0);
	
	// Draw the time.
	curFont = FntSetFont (boldFont);
	x = r.topLeft.x + (r.extent.x - FntCharsWidth (str, len)) - 3;
	y = r.topLeft.y + ((r.extent.y - FntLineHeight ()) / 2);
	WinDrawChars (str, len, x, y);
	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesApply
 *
 * DESCRIPTION: This routine applies the changes made in the Preferences Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *			vmk	12/9/97	Set alarm sound and repeat/interval to Datebook globals
 *			vmk	12/9/97	Added a call to save app's preferences
 *
 ***********************************************************************/
static Boolean PreferencesApply (uint16_t dayStartHour, uint16_t dayEndHour)
{	
	Boolean		updated = false;
	ListPtr		lst;
	FieldPtr 	fld;
	ControlPtr	ctl;
	Boolean		updateAlarms;
 	FormPtr 	frmP;
		
	if ((dayStartHour != DayStartHour) || (dayEndHour != DayEndHour))
	{
		DayStartHour = dayStartHour;
		DayEndHour = dayEndHour;
		updated = true;
	}

	// Get the alarm preset settings.
	frmP = FrmGetActiveForm ();
	ctl = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, PreferAlarmCheckbox));
	if (CtlGetValue (ctl))
	{
		fld = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, PreferAlarmField));
		
		AlarmPreset.advance = (uint8_t) StrAToI (FldGetTextPtr (fld));

		lst = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, PreferAlarmList));
		AlarmPreset.advanceUnit = (AlarmUnitType) LstGetSelection (lst);
	}
	else
	{
		AlarmPreset.advance = apptNoAlarm;			// no alarm is set
	}

	
	// If the sound, or nag information is changed, all alarms posted
	// to the attention manager will need to be udpated after the prefs
	// are saved.
	updateAlarms = ((AlarmSoundRepeatCount != PrefSoundRepeatCount) ||
		 			(AlarmSoundRepeatInterval != PrefSoundRepeatInterval) ||
		 			(AlarmSoundUniqueRecID != PrefSoundUniqueRecID) );

	// Get the alarm sound and interval settings
	AlarmSoundRepeatCount = PrefSoundRepeatCount;
	AlarmSoundRepeatInterval = PrefSoundRepeatInterval;
	AlarmSoundUniqueRecID = PrefSoundUniqueRecID;
	
	// Save app's preferences -- this is needed in case alarm settings
	// have been changed and an alarm occurs before we leave the app, since
	// the alarm notification handlers cannot rely on globals and always get
	// fresh settings from app's preferences.
	DatebookSavePrefs ();
	
	if (updateAlarms)
		AttnIterate(gApplicationDbID, SoundRepeatChanged);

	return updated;
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesInit
 *
 * DESCRIPTION: This routine initializes the Preferences Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		08/03/95	Initial Revision
 *			trev		08/14/97	Added MIDI support for alarms
 *       frigino	08/20/97	Modified sound name trigger initialization
 *			frigino	08/21/97	Removed typecasts in MapToPosition
 *			vmk		12/09/97	Check gMidiListH for null before locking
 *			kwk		07/07/99	Use default sound name str to find default entry.
 *
 ***********************************************************************/
static void PreferencesInit (void)
{
	FormPtr			formP;
	char *			label;
	char *			textP;
	MemHandle		textH;
	FieldPtr		fld;
	ControlPtr		ctl;
	ListPtr			listP;
	uint16_t			item;
	uint16_t			i;
	uint16_t			fldIndex;
	SndMidiListItemType*	midiListP;
	MemHandle		defaultNameH;
	char *			defaultName;

	formP = FrmGetActiveForm ();

	PrefDayStartHour = DayStartHour;
	PrefDayEndHour = DayEndHour;
	PrefSoundRepeatCount = AlarmSoundRepeatCount;
	PrefSoundRepeatInterval = AlarmSoundRepeatInterval;
	PrefSoundUniqueRecID = AlarmSoundUniqueRecID;

	// Set the alarm preset values.
	if (AlarmPreset.advance != apptNoAlarm)
	{
		// Turn the preset checkbox on.
		CtlSetValue (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferAlarmCheckbox)), true);
		
		// Set the alarm advance value.
		fld = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferAlarmField));
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, AlarmPreset.advance);
		MemPtrUnlock (textP);
		FldSetTextHandle (fld, textH);
		fldIndex = FrmGetObjectIndex (formP, PreferAlarmField);
		FrmShowObject (formP, fldIndex);
		FrmSetFocus (formP, fldIndex);
	
		// Set the alarm advance unit of measure (minutes, hours, or days).
		listP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferAlarmList));
		LstSetSelection (listP, AlarmPreset.advanceUnit);
		label = LstGetSelectionText (listP, AlarmPreset.advanceUnit);

		ctl = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferAlarmUnitTrigger));
		CtlSetLabel (ctl, label);
		FrmShowObject (formP, FrmGetObjectIndex (formP, PreferAlarmUnitTrigger));
	}
	
	// Set the Remind Me trigger and list
	listP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferRemindMeList));

	//	Convert the preference setting to it's UI list position:
	item = MapToPosition (RepeatCountMappings, PrefSoundRepeatCount,
								 numRepeats, defaultRepeatsLevel);
	LstSetSelection (listP, item);
	CtlSetLabel (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferRemindMeTrigger)),
											LstGetSelectionText (listP, item));

	// Set the Play Every trigger and list
	listP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferPlayEveryList));

	//	Convert the preference setting to it's UI list position:
	item = MapToPosition (RepeatIntervalMappings,
								(uint16_t)(PrefSoundRepeatInterval / minutesInSeconds),
								numRepeats, defaultPlayEveryLevel);
	LstSetSelection (listP, item);
	CtlSetLabel (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferPlayEveryTrigger)),
											LstGetSelectionText (listP, item));
	
	listP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, PreferAlarmSoundList));

	CreateMidiPickList(formP, FrmGetObjectIndex(formP, PreferAlarmSoundList), MidiPickListDrawItem);

	// Traverse MIDI pick list and find the item whose unique ID matches our
	// saved unique ID and use its index as our list selection index. If we
	// don't find a match, then we want to use the MIDI sound that corresponds
	// to our default sound name; if that's not found, use item 0.

	// Default to first sound in list
	item = 0;

	// Lock MIDI sound list
	if ( gMidiListH )
	{
		midiListP = MemHandleLock(gMidiListH);
		defaultNameH = DmGetResource (gApplicationDbP, strRsc, defaultAlarmSoundNameID);
		defaultName = MemHandleLock(defaultNameH);

		// Iterate through each item and get its unique ID
		for (i = 0; i < gMidiCount; i++)
		{
			if (midiListP[i].uniqueRecID == PrefSoundUniqueRecID)
			{
				item = i;
				break;		// exit for loop
			}
			else if (StrCompare(midiListP[i].name, defaultName) == 0)
			{
				item = i;
			}
		}
		
		MemHandleUnlock(defaultNameH);
  	 	DmReleaseResource(defaultNameH);   
		
		// Set the list selection
		LstSetSelection (listP, item);

		// Init the sound trigger label
		// Create a new ptr to hold the label
		soundTriggerLabelP = MemPtrNew(soundTriggerLabelLen);
		// Check for mem failure
		ErrFatalDisplayIf(soundTriggerLabelP == NULL, "Out of memory");
		// Set the trigger label
		SetSoundLabel(formP, midiListP[item].name);

		// Unlock MIDI sound list
		MemPtrUnlock(midiListP);
	}

	PreferencesUpdateScrollers ();
}


/***********************************************************************
 *
 * FUNCTION:    SetSoundLabel
 *
 * DESCRIPTION: Sets the sound trigger label, using truncation
 *
 * PARAMETERS:  formP - the form ptr
 *              labelP - ptr to original label text
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	8/18/97	Initial Revision
 *
 ***********************************************************************/

static void SetSoundLabel(FormPtr formP, const char* labelP)
{
	ControlPtr	triggerP;
	uint16_t		triggerIdx;

	// Copy the label, winUp to the max into the ptr
	StrNCopy(soundTriggerLabelP, labelP, soundTriggerLabelLen);
	
	// Terminate string at max len
	soundTriggerLabelP[soundTriggerLabelLen - 1] = '\0';
	
	// Get trigger idx
	triggerIdx = FrmGetObjectIndex(formP, PreferAlarmSoundTrigger);
	
	// Get trigger control ptr
	triggerP = FrmGetObjectPtr(formP, triggerIdx);
	
	// Use category routines to truncate it
	CategoryTruncateName(soundTriggerLabelP, (uint16_t) ResLoadConstant(gApplicationDbP, soundTriggerLabelWidth));
	
	// Set the label
	CtlSetLabel(triggerP, soundTriggerLabelP);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Preferences
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		8/3/95	Initial Revision
 *			trev		8/14/97	Added MIDI support for alarms
 *			frigino	9/9/97	Added PlayAlarmSound call when selecting new
 *									alarm sound from popup list
 *			vmk		12/9/97	Added initialization of alarm prefs in frmOpenEvent
 *			gap		10/15/99	Added added handling of frmUpdateEvent
 *
 ***********************************************************************/
Boolean PreferencesHandleEvent (EventType * event)
{
	Boolean 		updated;
	FormPtr 		frm = NULL;
	Boolean 		handled = false;
	SndMidiListItemType* listP;
	uint16_t 		item;
	wchar32_t 		chr;

	// Trace the event in debug mode
	DBGTraceEvent("Preferences Dialog", event);

	switch (event->eType)
	{
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case PreferOkButton:
					updated = PreferencesApply (PrefDayStartHour, PrefDayEndHour);
					FrmReturnToForm (0);
					if (updated)
						DateBkEnqueueEvent(datebookRefreshDisplay);
					
					// Free all data before we return to underlying form
					FreeAll();
					handled = true;
					break;

				case PreferCancelButton:
					FrmReturnToForm (0);
					// Free all data before we return to underlying form
					FreeAll();
					handled = true;
					break;
				
				case PreferAlarmCheckbox:
					PreferencesAlarmPresetOnOff (event->data.ctlSelect.on);
					handled = true;
					break;
			}
			break;

		case ctlRepeatEvent:
			switch (event->data.ctlRepeat.controlID)
			{
				case PreferStartDownButton:
					PrefDayStartHour--;
					PreferencesUpdateTimeDisplay (true);
					break;

				case PreferStartUpButton:
					PrefDayStartHour++;
					PreferencesUpdateTimeDisplay (true);
					if (PrefDayEndHour < PrefDayStartHour)
					{
						PrefDayEndHour = PrefDayStartHour;
						PreferencesUpdateTimeDisplay (false);
					}
					break;

				case PreferEndDownButton:
					PrefDayEndHour--;
					if (PrefDayEndHour < PrefDayStartHour)
					{
						PrefDayStartHour = PrefDayEndHour;
						PreferencesUpdateTimeDisplay (true);
					}
					PreferencesUpdateTimeDisplay (false);
					break;

				case PreferEndUpButton:
					PrefDayEndHour++;
					PreferencesUpdateTimeDisplay (false);
					break;
			}
			PreferencesUpdateScrollers ();
			break;

		case keyDownEvent:
			if (EvtKeydownIsVirtual(event))
				break;

			chr = event->data.keyDown.chr;
			if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
			{
				// Redirect numeric input to the Alarm Preset input field
				frm = FrmGetActiveForm();
				FldHandleEvent (FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, PreferAlarmField)), event);
			}
			handled = true;
			break;

		case frmOpenEvent:
			frm = FrmGetActiveForm ();
			PreferencesInit ();
			handled = true;
			break;
		
		case winUpdateEvent:						
			frm = FrmGetActiveForm ();
			if (event->data.winUpdate.window != FrmGetWindowHandle(frm))
				break;
			FrmDrawForm (frm);
			PreferencesDrawTime ((uint8_t) PrefDayStartHour, true);
			PreferencesDrawTime ((uint8_t) PrefDayEndHour, false);
			handled = true;
			break;
		
		case popSelectEvent:
			switch (event->data.popSelect.listID)
			{
				case PreferRemindMeList:
					item = event->data.popSelect.selection;
					PrefSoundRepeatCount = RepeatCountMappings[item];
					break;

				case PreferPlayEveryList:
					item = event->data.popSelect.selection;				
					PrefSoundRepeatInterval = RepeatIntervalMappings[item] * minutesInSeconds;
					break;	
					
				case PreferAlarmSoundList:
					// Get new selected item
					item = event->data.popSelect.selection;
					// Get active form
					frm = FrmGetActiveForm ();
					// Lock MIDI list
					listP = MemHandleLock(gMidiListH);
					// Save alarm sound unique rec ID
					PrefSoundUniqueRecID = listP[item].uniqueRecID;
					// Set new trigger label
					SetSoundLabel(frm, listP[item].name);
					// Unlock MIDI list
					MemPtrUnlock(listP);
					
					// Push datebookPrefPlaySound event
					DateBkEnqueueEvent(datebookPrefPlaySound);
					// Mark event as handled
					handled = true;
					break;
			}
			// Do not set handled = true to let the trigger be updated
			// for the 2 first triggers (PreferRemindMeList & PreferPlayEveryList)
			break;

		case frmCloseEvent:
			// Free all data before we return to underlying form
			FreeAll();
			break;

		case datebookPrefPlaySound:
			// Play new alarm sound
			PlayAlarmSound (PrefSoundUniqueRecID);
			handled = true;
			break;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    FreeAll
 *
 * DESCRIPTION: Frees all data allocated for the duration of the dialog
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	8/20/97	Initial Revision
 *			vmk		12/9/97	Check soundTriggerLabelP for null before deleting
 *
 ***********************************************************************/
static void FreeAll(void)
{
	// Free the MIDI pick list
	FreeMidiPickList();

	// Free the sound trigger label placeholder
	if ( soundTriggerLabelP )
	{
		MemPtrFree(soundTriggerLabelP);
		soundTriggerLabelP = NULL;
	}
}
