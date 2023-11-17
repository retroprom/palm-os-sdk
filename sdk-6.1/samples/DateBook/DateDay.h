/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDay.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This file defines the Datebook's Main modual's functions anf globals.
 *
 *****************************************************************************/

#ifndef _DATEDAY_H_
#define _DATEDAY_H_

#include <Event.h>
#include <DateTime.h>

#include "BooksTypes.h"

#include "DateGlobals.h"


#define DetailsDialogHelpIconWidth	24

#define DetailsTimeItemsSpacing 	4
#define DeleteApptMaxQuestionLength	40

#define DayBookMinTabWidth 			11
#define DayBookMaxTabWidth 			100
#define DayBookLabelCharsNum 		32

#define DayBookDOW1TabId 			3000
#define DayBookDOW2TabId 			3001
#define DayBookDOW3TabId 			3002
#define DayBookDOW4TabId 			3003
#define DayBookDOW5TabId 			3004
#define DayBookDOW6TabId 			3005
#define DayBookDOW7TabId 			3006

typedef struct TabInfoTag {
	char 		name[DayBookLabelCharsNum];
	uint16_t	id;
	uint16_t	padding;
} TabInfoType;


#ifdef __cplusplus
extern "C" {
#endif

// Event handling
Boolean 	DayViewHandleEvent (EventType* event);


Boolean 	DayViewClearEditState (void);

void 		DayViewRefreshDisplay (Boolean reloadFromDB, Boolean invalidateDisplay);

void 		DayViewDrawDate (DateType date, Boolean redraw);

void 		DayViewLoadApptsFromDB (void);

// Time drawing callback shared by day view & agenda view
void 		DrawTime (
	TimeType					inTime,	
	MidnightOverlapStateType 	overlapState, 
	Boolean						noTimeEvent,
	Boolean 					conflictingAppt, 
	TimeFormatType				inFormat, 
	FontID 						inFont, 
	JustificationType 			inJustification,
	RectangleType* 				inBoundsP);

// Move appointments
status_t 	MoveEvent (
	uint32_t				cursorID,
	TimeType 				newStartTime, 
	TimeType 				newEndTime,
 	DateType 				newDate, 
 	Boolean 				midnightWrapped, 
 	Boolean 				noTimeEvent,
 	Boolean 				timeChangeOnly, 
 	Boolean* 				movedP);

void DayViewFocusSetRowToSelect(int16_t row);
void DayViewFocusHowToSelect(int16_t state);

#ifdef __cplusplus
}
#endif

#endif //_DATEDAY_H_
