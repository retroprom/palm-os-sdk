/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BooksRendering.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _BOOKSRENDERING_H_
#define _BOOKSRENDERING_H_

#include <PalmTypes.h>
#include <DataMgr.h>

#include "BooksRenderingTypes.h"


/*	------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

void 			RenderCopyRGB2GcColor(RGBColorType* source, uint8_t alpha, GcColorType* cgColor);

void 			RenderCopyGcColor2RGB(GcColorType* source, uint8_t * alpha, RGBColorType* rgb);

void 			RenderTextLabelWithWinAPI(char* textLabel,int16_t textLen, Coord x, Coord y, RGBColorType* bkRgb, RGBColorType* textRgb);

void 			RenderGetTextLabelRectangle(TabFrameType* frameInfo, char* textLabel, uint32_t textLen, Coord x, Coord y, RectangleType* textZoneP);

void  			RenderPrepareContext(TabFrameType* frameInfo);

void 			RenderReleaseContext(TabFrameType* frameInfo);

void 			RenderBook(TabFrameType* frameInfo, RectangleType* activeTab, RectangleType* headerTab, RectangleType*  bodyTab);

void 			RenderEraseRect(TabFrameType* frameInfo, RectangleType* toErase, GcColorType* withColor);

Boolean 		RenderCreateFont(TabFrameType* frameInfo);

void 			RenderReleaseFont(TabFrameType* frameInfo);

void 			RenderDefineRect(GcHandle gc, RectangleType* highlightRect);

void 			RenderDefineRoundRect(GcHandle gc, RectangleType* highlightRect, fcoord_t cornerRadius);

Coord 			RenderGetTextLabelWidth(TabFrameType* frameInfo, char* textLabel, uint32_t textLen);

Coord			RenderGetTextHeight( TabFrameType* frameInfo, uint8_t what);

void 			RenderBooksTextLabel(TabFrameType* frameInfo, char* textLabel, uint32_t textLen, Coord x, Coord y, Boolean activeTab, Boolean highlight, RectangleType * textZoneP);

void 			RenderBooksItemBackground(TabFrameType* frameInfo, void* widget);

void 			RenderDrawBitmap(DmOpenRef dbRef, uint16_t bitmapId, Coord x, Coord y);

void 			RenderPaintBitmapLabel(TabFrameType* frameInfo, GcBitmapHandle bitmapH, RectangleType* bitmapZone);

void 			RenderRawBitmapLabel(TabFrameType* frameInfo, DmOpenRef dbRef, DmResourceID bitmapId, Coord x, Coord y);

void 			RenderBitmapLabel(TabFrameType* frameInfo, GcBitmapHandle bitmapH, Coord x, Coord y);

GcBitmapHandle 	RenderCreateBitmap(DmOpenRef dbRef, DmResourceID bitmapId);

void 			RenderReleaseBitmap(GcBitmapHandle gcBitmapH);

void 			RenderLeftArrow(TabFrameType* frameInfo, void* widget, RectangleType* arrowZone, Boolean highlight);

void 			RenderRightArrow(TabFrameType* frameInfo, void* widget, RectangleType* arrowZone, Boolean highlight);


#ifdef __cplusplus
}
#endif

/*	------------------------------------------------------------------- */

#endif
