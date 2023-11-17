/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: List.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines list routines.
 *
 *****************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#include <PalmTypes.h>

#include <Event.h>

#define noListSelection    		((int16_t)(-1))	// 16-bit 'invalid' selection

// Draw list item callback routine prototype
typedef void ListDrawDataFuncType (int16_t itemNum, RectangleType *bounds, char **itemsText, struct ListType *listP);
typedef ListDrawDataFuncType *ListDrawDataFuncPtr;

typedef struct ListType ListType;
typedef ListType *ListPtr;


//-------------------------------------------------------------------
// List routines
//-------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern void		LstDrawList (ListType *listP);

extern void		LstEraseList (ListType *listP);

extern int16_t	LstGetSelection (const ListType *listP);

extern char *	LstGetSelectionText (const ListType *listP, const int16_t itemNum);

extern Boolean	LstHandleEvent (ListType *listP, const EventType *eventP);

extern void		LstSetHeight (ListType *listP, int16_t visibleItems);

extern void		LstSetPosition (ListType *listP, const Coord x, const Coord y);

extern void		LstSetSelection (ListType *listP, int16_t itemNum);

extern void		LstSetListChoices (ListType *listP, char **itemsText, int16_t numItems);

extern void		LstSetDrawFunction (ListType *listP, ListDrawDataFuncPtr func);

extern void		LstSetTopItem (ListType *listP, const int16_t itemNum);

extern void		LstMakeItemVisible (ListType *listP, const int16_t itemNum);

extern int16_t	LstGetNumberOfItems (const ListType *listP);

extern int16_t	LstPopupList (ListType *listP);

extern Boolean	LstScrollList(ListType *listP, WinDirectionType direction, int16_t itemCount);

extern int16_t	LstGetVisibleItems (const ListType *listP);

extern status_t	LstNewList (void **formPP, uint16_t id, 
					Coord x, Coord y, Coord width, Coord height, 
					FontID font, int16_t visibleItems, int16_t triggerId);

extern int16_t	LstGetTopItem (const ListType *listP);

extern FontID	LstGetFont (const ListType *listP);

extern void		LstSetFont (ListType *listP, FontID fontID);

extern char**	LstGetItemsText (const ListType *listP);

extern void		LstSetIncrementalSearch (ListType *listP, Boolean incrementalSearch);

extern void		LstSetScrollArrows (ListType *listP, Boolean visible);

#ifdef __cplusplus 
}
#endif

#endif // __LIST_H__
