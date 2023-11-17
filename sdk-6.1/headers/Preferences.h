/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Preferences.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Public API for system preferences
 *
 *****************************************************************************/

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <PalmTypes.h>


/***********************************************************************
 *	Constants
 ***********************************************************************/

#define noPreferenceFound				-1

enum MeasurementSystemTag
	{
	unitsEnglish = 0,		// Feet, yards, miles, gallons, pounds, slugs, etc.
	unitsMetric				//	Meters, liters, grams, newtons, etc.
	};
typedef Enum8 MeasurementSystemType;


//	These sound levels must correspond to positions in the popup lists
//	used by the preferences app.  These are made obsolete after V20.  The
// loudness of the sound is now represented as a number from 0 to sndMaxAmp.
enum SoundLevelTypeV20Tag {
	slOn = 0,
	slOff = 1
	} ;
typedef Enum8 SoundLevelTypeV20;


enum SystemPreferencesChoiceTag	{
	prefVersion,
	prefCountry68K,
	prefDateFormat,
	prefLongDateFormat,
	prefWeekStartDay,
	prefTimeFormat,
	prefNumberFormat,
	prefAutoOffDuration,					// prefAutoOffDurationSecs is now preferred (prefAutoOffDuration is in minutes)
	prefSysSoundLevelV20,				// slOn or slOff - error beeps and other non-alarm/game sounds
	prefGameSoundLevelV20,				// slOn or slOff - game sound effects 
	prefAlarmSoundLevelV20,				// slOn or slOff - alarm sound effects 
	prefHidePrivateRecordsV33,
	prefDeviceLocked,
	prefLocalSyncRequiresPassword,	// no longer used by OS
	prefRemoteSyncRequiresPassword,
	prefSysBatteryKind,
	prefAllowEasterEggs,
	prefMinutesWestOfGMT,				// deprecated old unsigned minutes EAST of GMT
	prefDaylightSavings,					// deprecated old daylight saving time rule
	prefRonamaticChar,
	prefHard1CharAppCreator,			// App creator for hard key #1
	prefHard2CharAppCreator,			// App creator for hard key #2
	prefHard3CharAppCreator,			// App creator for hard key #3
	prefHard4CharAppCreator,			// App creator for hard key #4
	prefCalcCharAppCreator,				// App creator for calculator soft key
	prefHardCradleCharAppCreator,		// App creator for hard cradle key
	prefLauncherAppCreator,				// App creator for launcher soft key
	prefSysPrefFlags,		
	prefHardCradle2CharAppCreator,	// App creator for 2nd hard cradle key
	prefAnimationLevel,					// no longer used by OS

	// Additions for PalmOS 3.0:
	prefSysSoundVolume,					// actual amplitude - error beeps and other non-alarm/game sounds
	prefGameSoundVolume,					// actual amplitude - game sound effects
	prefAlarmSoundVolume,				// actual amplitude - alarm sound effects
	prefBeamReceive,						// not used - use ExgLibControl with ir(Get/Set)ScanningMode instead
	prefCalibrateDigitizerAtReset,	// no longer used by OS
	prefSystemKeyboardID,				// no longer used by OS
	prefDefSerialPlugIn,					// no longer used by OS

	// Additions for PalmOS 3.1:
	prefStayOnWhenPluggedIn,			// don't sleep after timeout when using line current
	prefStayLitWhenPluggedIn,			// keep backlight on when not sleeping on line current

	// Additions for PalmOS 3.2:
	prefAntennaCharAppCreator,			// App creator for antenna key

	// Additions for PalmOS 3.3:
	prefMeasurementSystem,				// English, Metric, etc.
	
	// Additions for PalmOS 3.5:
	prefShowPrivateRecords,				// returns privateRecordViewEnum
	prefAutoOffDurationSecs,			// auto-off duration in seconds
	
	// Additions for PalmOS 4.0:
	prefTimeZone,							// GMT offset in minutes, + for east of GMT, - for west
	prefDaylightSavingAdjustment,		// current DST adjustment in minutes (typically 0 or 60)

	prefAutoLockType,						// no longer used by OS
	prefAutoLockTime,						// no longer used by OS
	prefAutoLockTimeFlag,    			// no longer used by OS

	prefLanguage68K,						// no longer used by OS; use LmGetFormatsLocale instead.
	prefFormatsLocale68K,				// Locale for country selected via Setup app/Formats panel 
													// Use LmGetFormatsLocale/LmSetFormatsLocale instead.

	prefTimeZoneCountry,					// Country used to specify time zone.
	
	prefAttentionFlags,					// User prefs for getting user's attention

	prefDefaultAppCreator,				// no longer used by OS

	// Additions for PalmOS 5.0:
	prefDefFepPlugInCreator,			// creator ID of the default Fep plug-in

	// Additions for PalmOS 5.1:
	prefColorThemeID,						// Resource ID of the color theme.  Unused in 6.0.

	prefHandednessChoice,					// for landscape orientation's input area
	// reserved for 5.3's prefHWRCreator, for handwriting recognition library creator ID
	reservedPrefs2						

};
typedef Enum8 SystemPreferencesChoice;


/************************************************************
 * Param Block passsed with the sysAppLaunchCmdSetActivePanel Command
 *************************************************************/
 
#define prefAppLaunchCmdSetActivePanel		(sysAppLaunchCmdCustomBase + 1)	

// Record this panel so switching to the Prefs app causes this panel to execute.																
typedef struct 
	{
	uint32_t activePanel;
		// The creator ID of a panel.  Usually sent by a panel so the prefs 
		// apps will switch to it.  This allows the last used panel to appear
		// when switching to the Prefs app.
	} PrefActivePanelParamsType;

typedef PrefActivePanelParamsType *PrefActivePanelParamsPtr;


//-------------------------------------------------------------------
// Preferences routines
//-------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


extern uint32_t PrefGetPreference(SystemPreferencesChoice choice);

extern void PrefSetPreference(SystemPreferencesChoice choice, uint32_t value);

extern int16_t PrefGetAppPreferences (uint32_t creator, uint16_t id, void *prefs, 
	uint32_t *prefsSize, Boolean saved);

extern void PrefSetAppPreferences (uint32_t creator, uint16_t id, int16_t version, 
	const void *prefs, uint32_t prefsSize, Boolean saved);


#ifdef __cplusplus 
}
#endif

#endif	// __PREFERENCESBASE_H__
