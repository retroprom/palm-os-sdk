/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CmnDrawingTypes.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Types and defines shared by Palm OS and HAL drawing functions
 *
 *    Name	Date		Description
 *    ----	----		-----------
 *    PPo   04/19/00 Initial Revision. Moved structures here from
 *							Incs\Core\System\Window.h
 *
 *****************************************************************************/

#ifndef _CMNDRAWINGTYPES_H_
#define _CMNDRAWINGTYPES_H_

#include <CmnBitmapTypes.h>

// transfer modes for color drawing
enum WinDrawOperationTag
{
	winPaint,
	winErase,
	winMask,
	winInvert,
	winOverlay,
	winPaintInverse,
	winSwap
};
typedef Enum8 WinDrawOperation;


enum PatternTag
{
	blackPattern,
	whitePattern,
	grayPattern,
	customPattern,
	lightGrayPattern,
	darkGrayPattern
};
typedef Enum8 PatternType;

#define noPattern				blackPattern
#define grayHLinePattern		0xAA
#define grayHLinePatternOdd		0x55


// grayUnderline means dotted current foreground color
// solidUnderline means solid current foreground color
// colorUnderline redundant, use solidUnderline instead
// hairlineUnderline uses a single-pixel line, half-way
// between the current foreground and background colors
enum UnderlineModeTag
{
	noUnderline,
	grayUnderline,
	solidUnderline,
	colorUnderline,
	thinUnderline
};
typedef Enum8 UnderlineModeType;

typedef uint8_t IndexedColorType;			// 1, 2, 4, or 8 bit index

typedef uint8_t CustomPatternType[8];		// 8x8 1-bit deep pattern

// for WinPalette startIndex value, respect indexes in passed table
#define WinUseTableIndexes -1


#endif //_CMNDRAWINGTYPES_H_
