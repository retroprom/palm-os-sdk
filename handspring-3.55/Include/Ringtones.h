/**
 * \file Treo300_RingTones.h
 *
 * Treo 300 Ring Tones
 *
 * Header file for the Treo 300 Ringtones
 *
 * \license
 *
 * Copyright (c) 2002 Handspring Inc., All Rights Reserved
 *
 * $Id: $
 *
 ***************************************************************/

#ifndef _TREO300_RINGTONES_H_
#define _TREO300_RINGTONES_H_

// Common definitions
#include <PalmOS.h>

// Common definitions
#define PHN_LIB_TRAP(trapNum) SYS_TRAP(trapNum)


// ************************************************************
// Phone library constants
// ************************************************************
#define kPhnSndMaxNumOfRingerID		52

#define defaultRingersMax			24
#define roamingRingerID				defaultRingersMax + 3
#define callerMatchRingerID			defaultRingersMax + 1
#define noCallerMatchRingerID		defaultRingersMax + 2

// Phone slider switch
#define kSliderLow			0
#define kSliderHigh			1
#define kSliderPositions	2

// Maximum length of a ringer name (string resource)
#define kMaxRingName		16


// ************************************************************
// Phone library data structures
// ************************************************************
// Phone slider switch setting
typedef struct
  {
    UInt8            ringerID;
    UInt16            volume;
    Boolean         vibrate;
    Boolean         msgAlert;
    Boolean         svcAlert;
  }
PhnRingingProfileType;

typedef struct
  {
    PhnRingingProfileType profiles[kSliderPositions];
    Boolean         allSoundOff;
    UInt8            switchCurrPos;
  }
PhnRingingInfoType,* PhnRingingInfoPtr;


// ************************************************************
// Phone Library Enums
// ************************************************************
// Volume of phone ringer
typedef enum
  {
    phnRingerLoud = 4,
    phnRingerMedium = 3,
    phnRingerSoft = 2,
    phnRingerOff = 0,
	phnRingerLoudForceAmp = 8
  }
PhnRingerVolumeType;

// Support different audio ringer type, OR combinable!
// Match 1 to 1 to values supported by the modem
typedef enum
{
	phnRingerAudioTypeStandard  = 0x01,
	phnRingerAudioTypeMIDI      = 0x02,
	phnRingerAudioTypeMP3       = 0x04,

 	phnRingerAudioTypeAll       = phnRingerAudioTypeStandard |
                                phnRingerAudioTypeMIDI     |
                                phnRingerAudioTypeMP3
} PhnRingerAudioType;

// Sound Type state
typedef enum
  {
    phnSndTypeBeep,             // SM_SND_TYPE_BEEP
    phnSndTypeEarbeep,          // SM_SND_TYPE_EARBEEP
    phnSndTypeRinger,           // SM_SND_TYPE_ALERT
    phnSndTypeMsg,              // SM_SND_TYPE_MSG
    phnSndTypePwrup,            // SM_SND_TYPE_PWRUP

    // Special sound type must be declared outside of
    // the range used be the modem software.
    // Each sound type will do special thing.
    phnSndTypePwrUpTone = 0xE0,
    phnSndTypePwrDnTone,
    phnSndTypeToneBeep,           // Play beep tone, "bip!!!"
    phnSndTypeVeryLowBattAlert    // Beep for very low alert.
  }
PhnSndType;


// ************************************************************
// Phone Library Traps
// ************************************************************
#define GSMReservedFirst                    (sysLibTrapCustom + 130)
#define GSMReservedLast                     (GSMReservedFirst + 49)
#define CDMAFunctionsFirst                  (GSMReservedLast + 1)

#define PhnLibTrapGetRingingInfo            (sysLibTrapCustom + 40)
#define PhnLibTrapSetRingingInfo            (sysLibTrapCustom + 41)
#define PhnLibTrapGetToneIDs                (sysLibTrapCustom + 42)
#define PhnLibTrapGetToneName               (sysLibTrapCustom + 43)
#define PhnLibTrapPlayTone                  (sysLibTrapCustom + 44)
#define PhnLibTrapStopTone                  (sysLibTrapCustom + 45)
#define PhnLibTrapSndSave               	(CDMAFunctionsFirst + 44)
#define PhnLibTrapSndDelete   				(CDMAFunctionsFirst + 45)
#define PhnLibTrapStartVibrate              (sysLibTrapCustom + 114)
#define PhnLibTrapStopVibrate               (sysLibTrapCustom + 115)


// ************************************************************
// Phone Library Functions
// ************************************************************
extern Err      PhnLibGetRingingInfo (UInt16 refNum, PhnRingingInfoPtr info)
                PHN_LIB_TRAP (PhnLibTrapGetRingingInfo);

extern Err      PhnLibSetRingingInfo (UInt16 refNum, PhnRingingInfoPtr info)
                PHN_LIB_TRAP (PhnLibTrapSetRingingInfo);

extern Err      PhnLibGetToneIDs (UInt16 refNum, PhnRingerAudioType combinedRingerAudioType,
								  PhnRingerPtr* list, UInt16* listLength)
                PHN_LIB_TRAP (PhnLibTrapGetToneIDs);

extern Err      PhnLibGetToneName (UInt16 refNum, UInt16 tone, Char* name, UInt16 maxLength)
                PHN_LIB_TRAP (PhnLibTrapGetToneName);

extern Err      PhnLibPlayTone (UInt16 refNum, int tone, int volume, int vibrateState,
								UInt8 soundType, UInt16 duration, Boolean useSoundOffFlag)
                PHN_LIB_TRAP (PhnLibTrapPlayTone);

extern Err      PhnLibStopTone (UInt16 refNum)
                PHN_LIB_TRAP (PhnLibTrapStopTone);

extern Err      PhnLibSndSave (UInt16 refNum, PhnRingerAudioType theRingerFileType, UInt8 ringerID, UInt16  totalLength, Char*  fileNameP, UInt8*  audioDataP)
                PHN_LIB_TRAP (PhnLibTrapSndSave);

extern Err      PhnLibSndDelete (UInt16 refNum, UInt8	ringerID)
                PHN_LIB_TRAP (PhnLibTrapSndDelete);

extern Err	PhnLibStopVibrate (UInt16 refNum)
				  PHN_LIB_TRAP(PhnLibTrapStopVibrate);             

extern Err	PhnLibStartVibrate (UInt16 refNum, Boolean pulse, Boolean repeat)
				  PHN_LIB_TRAP(PhnLibTrapStartVibrate);             

#endif  // _TREO300_RINGTONES_H_
