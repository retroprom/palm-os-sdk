/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SelectTime.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  		New time selector allowing appointments spanning on midnight
 *
 *****************************************************************************/

#ifndef _SELECT_TIME_H_
#define _SELECT_TIME_H_

#include <PalmTypes.h>
#include <DateTime.h>

enum TimeSelectionTag {
	tSelectStart, 
	tSelectEnd,
	tSelectNoTime,
	tSelectAllDay
} ;

typedef Enum8 TimeSelectionType;

Boolean SelectTimeV6(
	char * 				titleP, 
	DateTimeType* 		startDateTimeP, 
	DateTimeType* 		endDateTimeP,
	TimeSelectionType * inOutSelectionP, 
	int16_t 			startOfDay, 
	int16_t 			endOfDay,
	Boolean 			limitedToTheSameDay);


#endif
