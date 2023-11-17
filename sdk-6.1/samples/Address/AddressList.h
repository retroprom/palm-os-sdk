/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressList.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSLIST_H_
#define _ADDRESSLIST_H_

#include <Event.h>

/***********************************************************************
 *   Defines
 ***********************************************************************/

// Address list table columns
#define kNameAndNumColumn					0
#define kNoteColumn							1

// Scroll rate values
#define kScrollDelay						2
#define kScrollAcceleration					2
#define kScrollSpeedLimit					5

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean	ListHandleEvent(EventType *eventP);

void	ListViewUpdateDisplay(Boolean reloadTable);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSLIST_H_
