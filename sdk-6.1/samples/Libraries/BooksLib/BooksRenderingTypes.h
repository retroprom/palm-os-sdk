/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: BooksRenderingTypes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              Contains constants and structures required by AddressBook sample.
 *
 *****************************************************************************/

#ifndef _BOOKSRENDERINGTYPE_H_
#define _BOOKSRENDERINGTYPE_H_

#include <PalmTypes.h>
#include <GcRender.h>
#include <GcFont.h>
#include <Font.h>

#define kStandardArcRendering			1
#define kStandardBezierRendering		2
#define kSlopedBezierRendering			3

#define kRoundCornerYOffset 			(8.0f)
#define kRoundCornerXOffset 			(8.0f)

#define kArcCornerYOffset 				(3.0f)
#define kArcCornerXOffset 				(3.0f)

#define kArrowCornerYOffset 			(3.0f)
#define kArrowCornerXOffset 			(3.0f)

#define kTabSlope 						(2.0f)

#define kSloppedCornerYOffset 			(4.0f)
#define kSloppedCornerXOffset 			(4.0f)

#define kSloppedCornerYControlOffset	(0.5f)
#define kSloppedCornerXControlOffset 	(0.5f)

#define kPenWidth						(1.0f)
#define kBookDefaultFontLeading			(2.0f)

#define kBookFontAscent					0x01
#define kBookFontDescent				0x02
#define kBookFontLeading				0x04

#define kBookDefaultFontSpec			"palmos-plain"
#define kBookDefaulfFontSize			(19.0f)

typedef struct TabFrameTag {
	uint16_t 				varCode;
	uint8_t					padding1;		//arm padding
	Boolean					fill;
	Boolean					frame;
	Boolean					transparent;
	Boolean					usePalmOSFont;
	FontID					fontId;
	FontID					savFontId;
	fcoord_t 				slopeWidth;
	fcoord_t 				xSlopeCornerOffset;
	fcoord_t 				ySlopeCornerOffset;
	fcoord_t 				xCornerOffset;
	fcoord_t 				yCornerOffset;
	fcoord_t 				xArcCornerOffset;
	fcoord_t 				yArcCornerOffset;
	fcoord_t 				xArrowCornerOffset;
	fcoord_t 				yArrowCornerOffset;
	fcoord_t				penSize;
	GcColorType				white;
	GcColorType				fillColor;
	GcColorType				frameColor;
	GcColorType				textActiveColor;
	GcColorType				textInactiveColor;
	GcColorType				textFgHighlightColor;
	GcColorType				textBgHighlightColor;
	GcColorType				arrowsColor;
	GcColorType				arrowsFgHighlightColor;
	GcColorType				arrowsBgHighlightColor;
	GcColorType				formBackgroundColor;
	RGBColorType			fillColorRGB;
	RGBColorType			textActiveColorRGB;
	RGBColorType			textInactiveColorRGB;
	RGBColorType			textFgHighlightColorRGB;
	RGBColorType			textBgHighlightColorRGB;
	RGBColorType			formBackgroundColorRGB;
	FontFamily				fontSpec;
	fcoord_t				fontSize;
	uint16_t				fontHeightA;
	uint16_t				fontHeightAD;
	GcFontHandle			gcFont;
	GcHandle				gcContext;
	void*					plugInfoP;
} TabFrameType, *TabFramePtr;

#endif
