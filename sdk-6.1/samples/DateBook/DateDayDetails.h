/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDayDetails.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  	This file defines the Datebook's DateEvent dialogs's functions
 *		and globals.
 *
 *****************************************************************************/

#ifndef _DATEDETAILS_H_
#define _DATEDETAILS_H_

#include <Event.h>

#include "DateDB.h"

#define DetailsDialogHelpIconWidth	24
#define DetailsTimeItemsSpacing 	4
#define DeleteApptMaxQuestionLength	40

// The following structure is used by the details dialog to hold
// changes made to an appointment record.
typedef struct {
	ApptDateTimeType	when;
	AlarmInfoType 		alarm;
	RepeatInfoType 		repeat;
	MemHandle			locationH;
	uint32_t			numCategories;
	CategoryID *		pCategoryIDs;
	char				categoriesName[catCategoryNameLength];
	uint32_t			savedNumCategories;
	CategoryID *		savedCategoryIDs;
	Boolean				secret;
	Boolean 			categoryChanged;
	uint8_t				padding[2];
} DetailsType;

typedef DetailsType * DetailsPtr;

typedef enum {
	dateRangeNone,
	dateRangeCurrent,
	dateRangeCurrentAndFuture,
	dateRangeAll
} DateRangeType; // for DeleteRecord()


extern uint32_t		gDetailsPosition;
extern uint32_t		gDetailsSelectionLength;
extern int32_t		gLocationFieldMaxLength;


#ifdef __cplusplus
extern "C" {
#endif


status_t 	SplitRepeatingEvent (uint32_t cursorID);

Boolean 	DeleteRecord (uint32_t rowID);

status_t 	CreateExceptionAndMove (
	ApptDBRecordPtr newRec,
	uint32_t 		cursorID,
	RecordFilteringInfoType unwantedFilter, 
	uint32_t 		numCategories,
	CategoryID 		pCategoryIDs[]);

Boolean 	CreateNote (Boolean prompt);

Boolean 	DeleteNote (Boolean exception, Boolean splitEvent);

Boolean 	NoteViewDeleteNote (DmOpenRef dbRef, uint32_t rowID, uint32_t pos, uint32_t columnID);

void 		DetailsEditNote (uint32_t rowID);

void 		DetailsSetRepeatingTrigger(DetailsPtr details);

void 		DetailsSetDefaultEventDetailsTab (uint16_t tabId);

Boolean 	DetailsHandleEvent (EventType* event);

void 		DoEventDetailsDialog(void);


#ifdef __cplusplus
}
#endif

#endif
