/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Books.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _BOOKS_H_
#define _BOOKS_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include <Font.h>
#include <Form.h>
#include <GcRender.h>

#include "BooksTypes.h"
#include "BooksRenderingTypes.h"


/*	------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

void 		BookSetFormId(BookType* bookP, uint16_t formId);

void 		BookSetFormPtr(BookType* bookP, FormPtr formP);

void 		BookSetGadgets(BookType* bookP, DmResourceID bookTabGadgetId, DmResourceID bookBodyGadgetId);

BookType* 	BookCreate(	FormPtr formP, DmResourceID bookTabGadgetId, DmResourceID bookBodyGadgetId,
				uint16_t minTabWidth, uint16_t maxTabWidth, uint16_t preallocatedTabs,
				uint16_t booksFlags);

void 		BookFree(BookType* bookP);

uint16_t 	BookFindTabIndex(BookType* bookP, uint16_t tabId);

status_t 	BookAddTab(	BookType* bookP,
				char* labelNameP,
				DmOpenRef dbRef, DmResourceID normalBitmapId, DmResourceID selectedBitmapId,
				uint16_t tabId, uint16_t tabIndex, uint32_t userData,
				BookActiveCallbackProcType* activationCallback,
				BookDeactiveCallbackProcType* deactivationCallback,
				winInvalidateFunc invalidateBodyProc);

status_t	BookRemoveTab(BookType* bookP, uint16_t tabId, Boolean compact);

void 		BookReActiveCurrentTab(BookType* bookP, Boolean redraw, Boolean makeVisible);
Boolean 	BookActiveTabIndex(BookType* bookP, uint16_t tabIndex);
Boolean 	BookActiveTabId(BookType* bookP, uint16_t tabId);

void 		BookSetActiveTabIndex(BookType* bookP, uint16_t tabIndex, Boolean redraw);
void 		BookSetActiveTabId(BookType* bookP, uint16_t tabId, Boolean redraw);

uint16_t 	BookGetActiveTabIndex(BookType* bookP);
uint16_t 	BookGetActiveTabId(BookType* bookP);

Boolean 	BookFocusPreviousTab(BookType* bookP, uint16_t* modifiers);
Boolean		BookFocusNextTab(BookType* bookP, uint16_t* modifiers);
void 		BookFocusFirst(BookType* bookP);
void 		BookFocusEnter(BookType* bookP);
void 		BookFocusTake(BookType* bookP);
void 		BookFocusReset(BookType* bookP);
void 		BookFocusLost(BookType* bookP);

void 		BookFocusActivate(BookType* bookP);

void 		BookEraseTabIndex(BookType* bookP, uint16_t tabIndex);
void		BookEraseTabId(BookType* bookP, uint16_t tabId);

void 		BookRedraw(BookType* bookP);
void 		BookResizeBody(BookType* bookP, uint16_t hOffset, uint16_t vOffset);

char*		BookGetTabLabel (BookType* bookP, uint16_t tabId);
void		BookSetTabLabel (BookType* bookP, uint16_t tabId, char* newLabel);

void 		BookSetFontSpec(BookType* bookP, char*	fontSpec, fcoord_t fontSize);
Boolean 	BookGetFontSpec (BookType* bookP, char* fontSpec, fcoord_t* fontSize);

void 		BookSetFontId (BookType* bookP, FontID fontId);
Boolean 	BookGetFontId (BookType* bookP, FontID* fontId);

void		BookGetTabGraphics (BookType* bookP, uint16_t tabId,
			DmOpenRef *dbRef, DmResourceID* bitmapId, DmResourceID* selectedBitmapId);
										
void		BookSetTabGraphics (BookType* bookP, uint16_t tabId,
			DmOpenRef dbRef, DmResourceID newBitmapId, DmResourceID newSelectedBitmapId);

uint16_t	BookGetNumberOfTab(BookType* bookP);
uint16_t	BookGetNumberOfAllocatedTab(BookType* bookP);

void 		BookGetVisibleTab(BookType* bookP, uint16_t *firstVisibleTabId,
			uint16_t *lastVisibleTabId, uint16_t *numVisibleTab);

void  		BookMakeTabVisibleByIndex(BookType* bookP, uint16_t tabIndex);
void		BookMakeTabVisible(BookType* bookP, uint16_t tabId);
Boolean		BookIsTabVisible(BookType* bookP, uint16_t tabId);

void 		BookTabHide (BookType* bookP, uint16_t tabId);
void 		BookTabShow (BookType* bookP, uint16_t tabId);

void 		BookGetBounds(BookType* bookP, RectangleType *tabBounds, RectangleType* bodyBounds);
void		BookSetBounds(BookType* bookP, RectangleType *tabBounds, RectangleType* bodyBounds);

void 		BookSetFrameColor (BookType* bookP, GcColorType* frame);
void 		BookGetFrameColor (BookType* bookP, GcColorType* frame);

void 		BookSetFillColor (BookType* bookP, GcColorType* fill);
void 		BookGetFillColor (BookType* bookP, GcColorType* fill);

void 		BookSetBackgroundColor (BookType* bookP, GcColorType* backgroundColor);
void 		BookGetBackgroundColor (BookType* bookP, GcColorType* backgroundColor);

void 		BookSetArrowColors (	BookType* bookP,
					GcColorType* arrow,
					GcColorType* fgArrowHighlight,
					GcColorType* bgArrowHighlight);
										
void 		BookGetArrowColors (	BookType* bookP,
					GcColorType* arrow,
					GcColorType* fgArrowHighlight,
					GcColorType* bgArrowHighlight);

void 		BookSetTextColors (	BookType* bookP,
					GcColorType* textActive,
					GcColorType* textInactive,
					GcColorType* fgTextHighlight,
					GcColorType* bgTextHighlight);
										
void 		BookGetTextColors (	BookType* bookP,
					GcColorType* textActive,
					GcColorType* textInactive,
					GcColorType* fgTextHighlight,
					GcColorType* bgTextHighlight);

void 		BookSetFrameInfo(BookType* bookP, TabFrameType* frameInfo);
void 		BookGetFrameInfo(BookType* bookP, TabFrameType* frameInfo);

void 		BookGetTabsParametrics(BookType* bookP, Coord* tabSpacing, Coord* labelSurround, Coord* labelVQuo, Coord* labelvDiv);
void 		BookSetTabsParametrics(BookType* bookP, Coord tabSpacing, Coord labelSurround, Coord labelVQuo, Coord labelvDiv);

void 		BookTabGetCallbacks(	BookType* bookP, 	uint16_t tabId,
					BookActiveCallbackProcType** activationCallback,
					BookDeactiveCallbackProcType** deactivationCallback,
					winInvalidateFunc* invalidateBodyProc);
					
void 		BookTabSetCallbacks(	BookType* bookP, uint16_t tabId,
			BookActiveCallbackProcType* activationCallback,
			BookDeactiveCallbackProcType* deactivationCallback,
			winInvalidateFunc invalidateBodyProc);

void 		BookTabGetFlags (BookType* bookP, uint16_t tabId, uint16_t* tabFlagsP);
void 		BookTabSetFlags (BookType* bookP, uint16_t tabId, uint16_t tabFlags);

void 		BookGetFlags (BookType* bookP, uint16_t* bookFlagsP);
void 		BookSetFlags (BookType* bookP, uint16_t bookFlags);

Boolean 	BookGetEnableUpdate (BookType* bookP);
void 		BookSetEnableUpdate (BookType* bookP, Boolean  enable);

void 		BookGetUserData (BookType* bookP, uint16_t tabId, uint32_t* userDataP);
void 		BookSetUserData (BookType* bookP, uint16_t tabId, uint32_t userData);

Boolean 	BookHandleEvent(BookType* bookP, EventType* eventP);

#ifdef __cplusplus
}
#endif

/*	------------------------------------------------------------------- */

#endif
