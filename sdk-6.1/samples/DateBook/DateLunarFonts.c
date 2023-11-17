/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateLunarFonts.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif


#include <DataMgr.h>
#include <ErrorMgr.h>
#include <Font.h>
//#include <SystemResources.h>
#include <UIResources.h>



#include "DateLunar.h"
#include "DateLunarFonts.h"



static 	MemHandle	pArabicNumeralFontH = NULL;
static	MemHandle	pChineseNumeralFontH = NULL;		


/***********************************************************************
 *
 * FUNCTION:    	LoadLunarViewFonts
 *
 * DESCRIPTION: 	Load the special Arabic and Chinese numeral fonts
 *					needed for the Lunar View.
 *
 * PARAMETERS:  ->	iResourceDbP: resource database open ref.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	CS 	2002-10-22	New today to support Lunar View.
 *	CS	2002-11-04	Use fontExtRscType, since we now have multi-density
 *					font families.
 *	PPL	03/15/04	Port to PalmOS 6.0
 *
 ***********************************************************************/
void LoadLunarViewFonts(DmOpenRef iResourceDbP)
{
	FontType *fontP = NULL;

	pArabicNumeralFontH = NULL;
	pChineseNumeralFontH = NULL;
	
	if (DateSupportsLunarCalendar())
	{
		pArabicNumeralFontH = DmGetResource(iResourceDbP, fontExtRscType, arabicNumeralFontID);
		ErrNonFatalDisplayIf(!pArabicNumeralFontH,
									"Can't load Arabic Numeral Font");
		if (pArabicNumeralFontH)
		{
			fontP = (FontType *)MemHandleLock(pArabicNumeralFontH);
			FntDefineFont((FontID)arabicNumeralFontID, fontP);
		}
		pChineseNumeralFontH = DmGetResource(iResourceDbP, fontExtRscType, chineseNumeralFontID);
		ErrNonFatalDisplayIf(!pChineseNumeralFontH,
									"Can't load Chinese Numeral Font");
		if (pChineseNumeralFontH)
		{
			fontP = (FontType *)MemHandleLock(pChineseNumeralFontH);
			FntDefineFont((FontID)chineseNumeralFontID, fontP);
		}
	}
} // LoadLunarViewFonts



/***********************************************************************
 *
 * FUNCTION:    	ReleaseLunarViewFonts
 *
 * DESCRIPTION: 	Release the special Arabic and Chinese numeral fonts
 *					needed for the Lunar View.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	CS 	2002-10-22	New today to support Lunar View.
 *	CS	2002-11-04	Use fontExtRscType, since we now have multi-density
 *					font families.
 *	PPL	03/15/04	Port to PalmOS 6.0
 *
 ***********************************************************************/
void ReleaseLunarViewFonts(void)
{
	if (pArabicNumeralFontH)
	{
		MemHandleUnlock(pArabicNumeralFontH);
		DmReleaseResource(pArabicNumeralFontH);
	}
	pArabicNumeralFontH = NULL;
	
	if (pChineseNumeralFontH)
	{
		MemHandleUnlock(pChineseNumeralFontH);
		DmReleaseResource(pChineseNumeralFontH);
	}
	pChineseNumeralFontH = NULL;
	
} // ReleaseLunarViewFonts
