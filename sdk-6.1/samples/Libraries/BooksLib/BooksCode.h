/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BooksCode.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		this  File implements Private functions of Books.c
 *		Most of the time they hold implementation.
 *****************************************************************************/

#ifndef _BOOKSCODE_H_
#define _BOOKSCODE_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include <Font.h>
#include <Form.h>

#include "BooksTypes.h"


/*	------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/*	------------------------------------------------------------------- */

uint16_t 	PrvTabIndexIsInValidRange(BookType* bookP, uint16_t tabIndex);

void 		PrvBooksAddTabHeaderInNavigation(BookType* bookP);

void 		PrvBookSetUpFrameInfo(BookType* bookP);

void 		PrvBookUnsetFrameInfo(BookType* bookP);

void 		PrvBookCreateScrollerDisplays(BookType* bookP);

void 		PrvBookFreeScrollerDisplays(BookType* bookP);

void 		PrvBookCreateTabDisplays(BookType* bookP, uint16_t tabIndex);

void 		PrvBookFreeTabDisplays(BookType* bookP, uint16_t tabIndex);

void  		PrvBookFreeOneTab(BookType* bookP, uint16_t tabIndex);

void  		PrvBookFreeTabs(BookType* bookP);

Boolean 	PrvBookDrawTabsHeader(BookType* bookP, RectangleType* drawingZoneP);

Boolean 	PrvBookDrawTabsBody(BookType* bookP, RectangleType* drawingZoneP);

void 		PrvBookInval(BookType* bookP, Boolean bookTabs, Boolean bookBody);

void 		PrvBookInvalTabLabel(BookType* bookP,  uint16_t tabIndex);

void 		PrvBookEraseTabIndex(BookType* bookP, uint16_t tabIndex);

Boolean 	PrvBookTabIsVisibleByIndex(BookType* bookP, uint16_t tabIndex);

Boolean 	PrvBookActiveTabIndex(BookType* bookP, uint16_t tabIndex);

void 		PrvBookSetActiveTabIndex(BookType* bookP, uint16_t tabIndex, Boolean redraw);

uint16_t 	PrvBookGetActiveTabIndex(BookType* bookP);

Boolean 	PrvBookActivePreviousTabIndex(BookType* bookP);

Boolean 	PrvBookActiveNextTabIndex(BookType* bookP);

void 		PrvBookCallTabActivationProc(BookType* bookP, uint16_t tabIndex);

Boolean 	PrvBookCallTabDeactivationProc(BookType* bookP, uint16_t tabIndex);

void 		PrvBookFocusSetRing(BookType* bookP);

void 		PrvBookFocusClearRing(void);

void 		PrvBookSetFocusedTabIndex(BookType* bookP, uint16_t tabIndex, Boolean redraw);

uint16_t 	PrvBookGetFocusedTabIndex(BookType* bookP);

Boolean 	PrvBookFocusPreviousTab(BookType* bookP, uint16_t* modifiers);

Boolean 	PrvBookFocusNextTab(BookType* bookP, uint16_t* modifiers);

void 		PrvBookFocusFirst(BookType* bookP);

Boolean 	PrvBookFocusEnter(BookType* bookP);

void 		PrvBookFocusTake(BookType* bookP);

void 		PrvBookFocusReset(BookType* bookP);

void 		PrvBookFocusLost(BookType* bookP);

void 		PrvBookFocusActivate(BookType* bookP);

void 		PrvBookSetFocusedTab(BookType* bookP, uint16_t tabIndex, Boolean on);

Boolean 	PrvBookGetTab(BookType* bookP, uint16_t tabIndex);

void 		PrvBookSetHighlightTab(BookType* bookP, uint16_t tabIndex, Boolean on);

Boolean 	PrvBookGetHighlightTab(BookType* bookP, uint16_t tabIndex);

void 		PrvBookSetFocusedTab(BookType* bookP, uint16_t tabIndex, Boolean on);

Boolean 	PrvBookGetFocusTab(BookType* bookP, uint16_t tabIndex);

void 		PrvBookUpdateScrollFlag(BookType* bookP);

void 		PrvBookGetTabBounds(BookType* bookP, RectangleType* bounds);

void 		PrvBookSetTabBounds(BookType* bookP, RectangleType* bounds);

void 		PrvBookGetBodyBounds(BookType* bookP, RectangleType* bounds);

void 		PrvBookSetBodyBounds(BookType* bookP, RectangleType* bounds);

void 		PrvBookAddTabWidth(BookType* bookP, uint16_t	tabIndex);

void 		PrvBookSubstractTabWidth(BookType* bookP, uint16_t tabIndex);

void 		PrvBookCalculateTabIndexWidth(BookType* bookP, uint16_t tabIndex);

void 		PrvBookReCalculateTabs(BookType* bookP);

void 		PrvBookMakeTabVisible(BookType* bookP, uint16_t tabIndex);

Boolean 	PrvBookHasRightScroll(BookType* bookP);

Boolean 	PrvBookHasLeftScroll(BookType* bookP);

void 		PrvBookScroll(BookType* bookP, int16_t tabsNum);

Boolean 	PrvBookGadgetHandleVirtualKeys( BookType* bookP, EventType* eventP);

Boolean 	PrvBookGadgetTabsHeaderCallBack( FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);

Boolean 	PrvBookGadgetTabsBodyCallBack( FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);

status_t 	PrvBookResize(BookType* bookP, uint16_t howManyTabs);

Boolean 	PrvBookHandleEvent(BookType* bookP, EventType* eventP);


/*	------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

/*	------------------------------------------------------------------- */

#endif
