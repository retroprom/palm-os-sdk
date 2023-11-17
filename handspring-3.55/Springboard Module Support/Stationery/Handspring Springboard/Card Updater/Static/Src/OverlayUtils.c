/***********************************************************************
 *
 *	Copyright (c) Handspring 1999 -- All Rights Reserved
 *
 * PROJECT:  Utilities
 * FILE:     OverlayUtils.c
 * AUTHOR:	 Douglas Anderson: Oct 11, 1999
 *
 * DESCRIPTION:
 *	  Utilities for doing pseudo-overlays until OS 3.5 becomes
 *    commonplace and we can use its facilities.
 *
 *	  11-Apr-2000	vmk	  Updated for new PalmOS SDK (PalmOS.h
 *						   instead of Pilot.h, etc.).
 *
 **********************************************************************/


//#include <Pilot.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>

#include <DmUtils.h>
#include "OverlayUtils.h"


// The overlay name is not specified in a resource.  It is not user-visible
// and can't be localized anyway.
#define overlaySuffixNameStr "Rsc_%d_%d"


/***************************************************************
 *  Function:    PrvOverlayName 
 * 
 *  Summary:      
 *    Generates the name of an overlay file given the name of 
 *    the application, the country, and the language.  The format
 *    of the overlay name is $(nameP)_$(country)_$(language)
 *    where country and language are numeric values.  See
 *    preferences.h (for country) and buildrules.h (for language)
 * 
 *  Parameters:
 *    overlayNameP  OUT Buffer to store result name in.  Must be
 *                      at least 32 bytes bit (31 chars + null)
 *    nameP         IN  Base name for the overlay (app's name).
 *    nameLen       IN  Length of nameP
 *    country       IN  Country to use in name generation.
 *    language      IN  Language to use in name generation.
 *     
 *  Returns: 
 *    nothing
 *  
 *  Called By:  
 *    HsUtilOverlayInitialize
 * 
 *  Notes:
 *    + The string in this function (overlay template) need not
 *      be localized, since it's not user-visible.
 *    + This function _is_ multibyte friendly (I hope)
 *
 *  History: 
 *    08-oct-99 dia Created...
 ****************************************************************/

static void 
PrvOverlayName (CharPtr overlayNameP, CharPtr nameP, Int16 nameLen,
                CountryType country, Int16 language)
{
  Char suffixP[32];
  UInt16 suffixLen;

  suffixLen = StrPrintF (suffixP, overlaySuffixNameStr, (Int16) country, (Int16) language);

  if (nameLen + suffixLen > (dmDBNameLength-1))
    {
      nameLen = (dmDBNameLength-1) - suffixLen;
      nameLen = TxtGetTruncationOffset (nameP, nameLen);
    }
  StrNCopy (overlayNameP, nameP, nameLen);
  StrCopy (overlayNameP + nameLen, suffixP);
}


/***************************************************************
 *  Function:    HsUtilOverlayInitialize
 * 
 *  Summary:
 *    Tries to open an overlay resource based on the language 
 *    and country that the ROM was compiled for.  If no overlay
 *    exists, doesn't open anything and just uses built-in
 *    resources.
 *
 *    The format of the overlay name is 
 *    $(nameP)_$(country)_$(language) where country and 
 *    language are numeric values.  See preferences.h (for 
 *    country) and buildrules.h (for language)
 *
 *    ...looks on all cards for the newest version, starting
 *    in RAM.
 * 
 *    ...if one is not found, it keys off of the language and
 *    tries to look for an overlay for the default country
 *    for that language (the logic specifying the default country
 *    is internal to this function).  
 *
 *    ...if no overlay is found, no overlay is opened.
 *  
 *  Parameters:
 *    baseStrP  IN  The base name to use when looking for
 *                  overlays.
 *     
 *  Returns: 
 *    The open ref of the overlay opened, or NULL.
 *  
 *  Called By:  
 *    client
 * 
 *  History: 
 *    08-oct-99 dia Created...
 *    04-aug-00 dia Changed so that if we don't have an exact
 *                  language/country match for english or spanish
 *                  we now check for both US and UK for english
 *                  and Spain and Mexico for spanish.  Really should
 *                  just be Spain and US, but an earlier version used
 *                  Mexico and UK, so we should be compatible w/ both.
 *
 ****************************************************************/


#if 0
  Char nameP[dmDBNameLength];
  DmSearchStateType stateInfo;

  // Construct a name if user didn't supply one...
  if (baseStrP == NULL)
    {
      DmGetNextDatabaseByTypeCreator (true /*newSearch*/, &stateInfo, sysFileTApplication, 
                                      hsFileCBackup, true /*onlyLatest*/, &cardNo, &dbId);
      DmDatabaseInfo (cardNo, dbId, nameP, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      baseStrP = nameP;
    }
#endif // 0

typedef struct
  {
	UInt16 numCountries;
	CountryType* countriesArr;
  }
CountryListType;


DmOpenRef 
HsUtilOverlayInitialize (CharPtr baseStrP)
{
  Err err;
  DWord temp;
  UInt16 cardNo;
  LocalID dbId;
  const UInt16 baseLen = StrLen (baseStrP);
  Char overlayNameP[dmDBNameLength];
  CountryType country = cUnitedStates;
  DWord language = lEnglish;

  // Find the country/language of the ROM...
  // <chg 09-dec-99 dia> Allow overriding using 'LanguageSpoofer'...
  if (FtrGet (hsFtrCreatorOverlayOverrides, hsFtrIDCountryOverride, &temp) != 0)
    {
      FtrGet(sysFtrCreator, sysFtrNumCountry, &temp);
    }
  country = (CountryType) temp;

  if (FtrGet (hsFtrCreatorOverlayOverrides, hsFtrIDLanguageOverride, &language) != 0)
    {
      FtrGet (sysFtrCreator, sysFtrNumLanguage, &language);
    }

  // Look for it...
  PrvOverlayName (overlayNameP, baseStrP, baseLen, country, language);
  err = HsUtilDmFindDatabaseMulticard (overlayNameP, &cardNo, &dbId);

  // ...if there is an error, use a heuristic: look for resources for the main countries
  // of the given resource.
  if (err)
    {
	  // Note that this code _only_ gets executed if we couldn't find a resource file
	  // using both the country and language specified by the ROM (or a spoof).  Now,
	  // we try using just the language to construct a country.  

	  if (language < languageCount && language >= 0)
		{
		  // <chg 04-aug-00 dia> Now allows search for multiple countries for a given
		  // language.  This is because when I first wrote the function, I assumed that
		  // the main country for english was UnitedKingdom (I think this was a paste-o) and
		  // that the main country for spanish was Mexico (this was just a mistake).  
		  // In order to transition to the correct countries but to still maintain full
		  // backward compatibility, I have made it look at both.  This code should 
		  // very rarely get executed in the real world anyway...

		  static const CountryType kEnglishCountries[]  = { cUnitedStates, cUnitedKingdom };
		  static const CountryType kFrenchCountries[]   = { cFrance };
		  static const CountryType kGermanCountries[]   = { cGermany };
		  static const CountryType kItalianCountries[]  = { cItaly };
		  static const CountryType kSpanishCountries[]  = { cSpain, cMexico };
		  static const CountryType kJapaneseCountries[] = { cJapan };
		  static const CountryType kDutchCountries[]    = { cNetherlands };
		  const CountryListType countryListArr[languageCount] = 
			{
			  { sizeof (kEnglishCountries)  / sizeof (kEnglishCountries[0]),  (CountryType*)kEnglishCountries  },
			  { sizeof (kFrenchCountries)   / sizeof (kFrenchCountries[0]),   (CountryType*)kFrenchCountries   },
			  { sizeof (kGermanCountries)   / sizeof (kGermanCountries[0]),   (CountryType*)kGermanCountries   },
			  { sizeof (kItalianCountries)  / sizeof (kItalianCountries[0]),  (CountryType*)kItalianCountries  },
			  { sizeof (kSpanishCountries)  / sizeof (kSpanishCountries[0]),  (CountryType*)kSpanishCountries  },
			  { 0 /*WORKPAD LANGUAGE*/,                                       NULL               },
			  { sizeof (kJapaneseCountries) / sizeof (kJapaneseCountries[0]), (CountryType*)kJapaneseCountries },
			  { sizeof (kDutchCountries)    / sizeof (kDutchCountries[0]),    (CountryType*)kDutchCountries    },
			};
		  const CountryListType* countryListP = &(countryListArr[language]);
		  UInt16 i;
/*
		  #if (languageFirst != 0 || languageLast != (languageCount-1))
			#error "Assumed zero-based enum"
		  #endif
*/
		  for (i = 0; err && i < countryListP->numCountries; i++)
			{
			  country = countryListP->countriesArr[i];

			  PrvOverlayName (overlayNameP, baseStrP, baseLen, country, language);
			  err = HsUtilDmFindDatabaseMulticard (overlayNameP, &cardNo, &dbId);
			}
		}
    }

  return (err) ? NULL : DmOpenDatabase (cardNo, dbId, dmModeReadOnly);
}
  

/***************************************************************
 *  Function:    HsUtilOverlayCleanup
 * 
 *  Summary:
 *    Closes an overlay opened with HsUtilOverlayInitialize()
 * 
 *  Parameters:
 *    openRefP  IN  The DmOpenRef returned from 
 *                  HsUtilOverlayInitialize().  Ok to call with NULL.
 *     
 *  Returns: 
 *    nothing
 *  
 *  Called By:  
 *    client
 * 
 *  History: 
 *    08-oct-99 dia Created...
 ****************************************************************/

void 
HsUtilOverlayCleanup (DmOpenRef openRefP)
{
  if (openRefP != NULL) 
    {
      DmCloseDatabase (openRefP);
    }
}

