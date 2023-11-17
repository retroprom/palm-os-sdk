/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateLunarFonts.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	This file declares the (temporary) lunar calendar support for the
 *	Datebook application.  These declarations will be part of Time Manager 6.0.
 *
 *****************************************************************************/

#ifndef __DATELUNARFONTS_H__
#define __DATELUNARFONTS_H__

#include <PalmTypes.h>

// LunarCalendar Resources (should be spun off)
// These custom fonts are used by the Lunar View to draw date numbers
#define arabicNumeralFontID				128
#define chineseNumeralFontID			129

void LoadLunarViewFonts(DmOpenRef iResourceDbP);
void ReleaseLunarViewFonts(void);


#endif
