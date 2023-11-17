/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BooksPlug.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _BOOKSPLUG_H_
#define _BOOKSPLUG_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include <Font.h>
#include <Form.h>

#include "BooksTypes.h"



/*	------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

Boolean 	PlugBookFormIsTransparent(BookType* bookP);

void 		PlugGetTabLabelRectangle(BookType* bookP, uint16_t tabIndex, RectangleType* labelZoneP);

void 		PlugGetTabRectangle(BookType* bookP, uint16_t tabIndex, RectangleType* tabZoneP);

Boolean 	PlugGetLeftArrowRectangle(BookType* bookP, RectangleType* arrowZoneP);

Boolean 	PlugGetRightArrowRectangle(BookType* bookP, RectangleType* arrowZoneP);

void 		PlugGetBookTabsRectangle(BookType* bookP,Boolean noScrolls, RectangleType* tabZoneP);

void 		PlugSetUpFrameInfo(BookType* bookP);

void 		PlugUnsetFrameInfo(BookType* bookP);

void 		PrvPlugCreateScrollerDisplays(BookType* bookP);

void 		PrvPlugFreeScrollerDisplays(BookType* bookP);

void 		PrvPlugPrepareScrollerDisplay(BookType* bookP, void* scrollDisplay, RectangleType* arrowZoneP, Boolean enabled, Boolean highlighted);

void 		PrvPlugCreateTabDisplays(BookType* bookP, uint16_t tabIndex);

void 		PrvPlugFreeTabDisplays(BookType* bookP, uint16_t tabIndex);

void 		PrvPlugPrepareTabDisplays(BookType* bookP, uint16_t tabIndex, RectangleType* boundsP);

void 		PlugAdjustScrollsSpace(BookType* bookP, Coord* xOffset, Coord* bookWidth);

void 		PlugCalculateTabIndexWidth(BookType* bookP, uint16_t pageIndex);

//void 		PlugEraseTabIndex(BookType* bookP, uint16_t tabIndex);

void 		PlugDrawTab(BookType* bookP, uint16_t tabIndex, Coord xOffset,  Coord yOffset, Coord tabHeight, Coord bookHeight);

void 		PlugScaleDrawingParams(BookType* bookP);

void 		PlugUnscaleDrawingParams(BookType* bookP);

uint16_t 	PlugTabToBecomeFirstTab(BookType* bookP, uint16_t willBeLastTab);

uint16_t	PlugGetLastVisibleTab(BookType* bookP);

void 		PlugUpdateVisibility(BookType* bookP);

uint16_t 	PlugFindTab(BookType* bookP, RectangleType* gadgetBoundsP, Coord x, Coord y, RectangleType * tabZoneP, RectangleType * labelZoneP, uint16_t * activeTabP);

Boolean 	PlugBookDrawTabsHeader(BookType* bookP, RectangleType* drawingZoneP);

Boolean 	PlugBookDrawTabsBody(BookType* bookP, RectangleType* drawingZoneP);

Boolean 	PlugTabDrawCallback(int32_t cmd, WinHandle window, const RectangleType *dirtyRect, void *state);

Boolean 	PlugTabsHeaderDrawCallback(int32_t cmd, WinHandle window, const RectangleType *dirtyRect, void *state);

Boolean 	PlugTabsBodyDrawCallback(int32_t cmd, WinHandle window, const RectangleType *dirtyRect, void *state);

Boolean 	PlugHandleEvent(BookType* bookP, EventType* eventP);


#ifdef __cplusplus
}
#endif

/*	------------------------------------------------------------------- */

#endif
