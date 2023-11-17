/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SoundMgr.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _SOUNDMGR_H_ARM_ // there's a separate file for 68K land
#define _SOUNDMGR_H_ARM_


#include <AudioTypes.h>
#include <PalmTypes.h>
#include <Preferences.h>
#include <DataMgr.h>

#define sndFtrIDVersion		0		// ID of feature containing (new) sound manager version
									// Check for existence of this feature to determine if 
									//  the Sound streams APIs are available
#define sndMgrVersionNum	(100)   // The current version (1.00) 

typedef uint32_t SndStreamRef;

enum SndStreamModeTag {
	sndInput,
	sndOutput
};

typedef Enum8 SndStreamMode;

enum SndStreamWidthTag {
	sndMono,
	sndStereo
};
typedef Enum8 SndStreamWidth;

typedef status_t (*SndStreamVariableBufferCallback)(void *userdata,
		SndStreamRef channel, void *buffer, uint32_t *bufferSizeP);

enum {
	sndFormatPCM		= 0,			// Raw PCM (Pulse Code Modulation), (attributes defined by type parameter)
	sndFormatIMA_ADPCM	= 'APCM',		// IMA ADPCM (Interactive Multimedia Association Adaptive Differential PCM)
	sndFormatDVI_ADPCM	= 'DPCM',		// Intel/MS/DVI ADPCM
	sndFormatMP3		= 'MPG3',		// Moving Picture Experts Group, Audio Layer III (MPEG-1 and MPEG-2 depending on bit-rate)
	sndFormatAAC		= 'DAAC',		// Dolby Advanced Audio Coding
	sndFormatOGG		= 'OGGV'		// Ogg Vorbis
};

typedef uint32_t SndFormatType;


/*
 * special 'cookie' values that can be used instead of a real volume setting
 * */
enum
{
	sndSystemVolume = -1,
	sndGameVolume = -2,
	sndAlarmVolume = -3
};

// Use real typedef'd enum here. For 68K apps, PACE will convert
typedef audio_type_t SndSampleType;


// Flags to be used with SndPlayResource
#define sndFlagSync			0x00000000
#define sndFlagAsync		0x00000001
#define sndFlagInvalidMask	0xfffffffe
#define sndFlagNormal		sndFlagSync

// Special Panning values
#define sndPanCenter		(0)
#define sndPanFullLeft		(-1024)
#define sndPanFullRight		(1024)





typedef void *SndPtr;

typedef status_t (*SndStreamBufferCallback)(void *userdata,
		SndStreamRef channel, void *buffer, uint32_t numberofframes);

#ifdef __cplusplus
extern "C" {
#endif

status_t SndStreamCreate(SndStreamRef *channel,	/* channel-ID is stored here */
						SndStreamMode mode,			/* input/output, enum */
						uint32_t samplerate,				/* in frames/second, e.g. 44100, 48000 */
						SndSampleType type,				/* enum, e.g. sndInt16 */
						SndStreamWidth width,			/* enum, e.g. sndMono */
						SndStreamBufferCallback func,	/* function that gets called to fill a buffer */
						void *userdata,					/* gets passed in to the above function */
						uint32_t buffsize);				/* preferred buffersize in frames, not guaranteed, use 0 for default */

status_t SndStreamDelete(SndStreamRef channel);
status_t SndStreamStart(SndStreamRef channel);
status_t SndStreamPause(SndStreamRef channel, Boolean pause);
status_t SndStreamStop(SndStreamRef channel);
status_t SndStreamSetVolume(SndStreamRef channel, int32_t volume);
status_t SndStreamGetVolume(SndStreamRef channel, int32_t *volume);
status_t SndStreamSetPan(SndStreamRef channel, int32_t panposition);
status_t SndStreamGetPan(SndStreamRef channel, int32_t *panposition);
status_t SndPlayResource(SndPtr sndP, int32_t volume, uint32_t flags);

status_t SndStreamCreateExtended(
	SndStreamRef *channel,	/* channel-ID is stored here */
	SndStreamMode mode,		/* input/output, enum */
	SndFormatType format,	/* enum, e.g., sndFormatMP3 */
	uint32_t samplerate,		/* in frames/second, e.g. 44100, 48000 */
	SndSampleType type,		/* enum, e.g. sndInt16, if applicable, or 0 otherwise */
	SndStreamWidth width,	/* enum, e.g. sndMono */
	SndStreamVariableBufferCallback func,	/* function that gets called to fill a buffer */
	void *userdata,		/* gets passed in to the above function */
	uint32_t buffsize);		/* preferred buffersize, use 0 for default */

status_t SndStreamDeviceControl(
	SndStreamRef channel,
	int32_t cmd,
	void* param,
	int32_t size);




/************************************************************
 * Sound Manager result codes
 * (sndErrorClass is defined in SystemMgr.h)
 *************************************************************/
#define	sndErrBadParam			(sndErrorClass | 1)
#define	sndErrBadChannel		(sndErrorClass | 2)
#define	sndErrMemory			(sndErrorClass | 3)
#define	sndErrOpen				(sndErrorClass | 4)
#define	sndErrQFull				(sndErrorClass | 5)
#define	sndErrQEmpty			(sndErrorClass | 6)		// internal
#define	sndErrFormat			(sndErrorClass | 7)		// unsupported data format
#define sndErrBadStream			(sndErrorClass | 8)
#define	sndErrInterrupted		(sndErrorClass | 9)		// play was interrupted
#define sndErrNotImpl			(sndErrorClass |10)		// function not implemented
#define sndErrInvalidStream		(sndErrorClass |11)		// invalid stream-identifier



/*
 * The (really) OLD sound manager
 *
 */


/************************************************************
 * Sound Manager constants
 *
 *************************************************************/

// Sound Manager max and default volume levels
#define sndMaxAmp				64		// old API. The stream-API uses 1024 as unity volume.
#define sndDefaultAmp		sndMaxAmp
#define sndMidiNameLength	32			// MIDI track name length *including* NULL terminator


/************************************************************
 * Sound Manager data structures
 *
 *************************************************************/

//
// Command numbers for SndCommandType's cmd field
//
enum SndCmdIDTag {
									
	sndCmdFreqDurationAmp = 1,		// play a sound, blocking for the entire duration (except for zero amplitude)
											// param1 = frequency in Hz
											// param2 = duration in milliseconds
											// param3 = amplitude (0 - sndMaxAmp); if 0, will return immediately
											
	// Commands added in PilotOS v3.0
	// ***IMPORTANT***
	// Please note that SndDoCmd() in PilotOS before v3.0 will Fatal Error on unknown
	// commands (anything other than sndCmdFreqDurationAmp).  For this reason,
	// applications wishing to take advantage of these new commands while staying
	// compatible with the earlier version of the OS, _must_ avoid using these commands
	// when running on OS versions less thatn v3.0 (see sysFtrNumROMVersion in SystemMgr.h).
	// Beginning with v3.0, SndDoCmd has been fixed to return sndErrBadParam when an
	// unknown command is passed.
	//
	sndCmdNoteOn,						// start a sound given its MIDI key index, max duration and velocity;
											// the call will not wait for the sound to complete, returning imeediately;
											// any other sound play request made before this one completes will interrupt it.
											// param1 = MIDI key index (0-127)
											// param2 = maximum duration in milliseconds
											// param3 = velocity (0 - 127) (will be interpolated as amplitude)
	
	sndCmdFrqOn,						// start a sound given its frequency in Hz, max duration and amplitude;
											// the call will not wait for the sound to complete, returning imeediately;
											// any other sound play request made before this one completes will interrupt it.
											// param1 = frequency in Hz
											// param2 = maximum duration in milliseconds 
											// param3 = amplitude (0 - sndMaxAmp)
	
	sndCmdQuiet							// stop current sound
											// param1 = 0
											// param2 = 0
											// param3 = 0

	};

typedef Enum8 SndCmdIDType ;

	

//
// SndCommandType: used by SndDoCmd()
//

typedef struct SndCommandType {
SndCmdIDType	cmd;					// command id
uint8_t 			reserved;
uint16_t			padding;				// alignment padding
int32_t			param1;					// first parameter
uint16_t			param2;					// second parameter
uint16_t			param3;					// third parameter
} SndCommandType;

typedef SndCommandType*		SndCommandPtr;


//
// Beep numbers used by SndSysBeep()
//

enum SndSysBeepTag {
	sndInfo = 1,
	sndWarning,
	sndError,
	sndStartUp,
	sndAlarm,
	sndConfirmation,
	sndClick,
	sndCardInserted,
	sndCardRemoved
	} ;
typedef Enum8 SndSysBeepType ;


/************************************************************
 * Standard MIDI File (SMF) support structures
 *************************************************************/


// Structure of records in the MIDI sound database:
//
// Each MIDI record consists of a record header followed immediately by the
// Standard MIDI File (SMF) data stream.  Only SMF format #0 is presently supported.
// The first byte of the record header is the byte offset from the beginning of the record
// to the SMF data stream.  The name of the record follows the byte offset
// field.  sndMidiNameLength is the limit on name size (including NULL).

#if CPU_ENDIAN == CPU_ENDIAN_BIG
#define sndMidiRecSignature	'PMrc'
#else
#define sndMidiRecSignature	'crMP'		// How the signature looks on a little endian system
#endif

// NOTE: The header is packed (its not a multiple of 4), so do not use
// sizeof(SmdMidiRecHdrType) to get the size of the header.  Instead use this define:
#define sndMidiRecHdrSize	6

typedef struct SndMidiRecHdrType {
	uint32_t		signature;				// set to sndMidiRecSignature
	uint8_t		bDataOffset;			// offset from the beginning of the record
										// to the Standard Midi File data stream
	uint8_t		reserved;				// set to zero
	uint16_t		padding;				// pad out to multiple of 4 bytes
	} SndMidiRecHdrType;

// NOTE: There used to be a struct typedef SndMidiRecType which has been removed.
// The struct was used to access the name field following the header, but this does
// not work on 4-byte aligned systems.  If you need to access the name field then do
// something like this:
//	pName = (char*)hdrP + sndMidiRecHdrSize;


// Midi records found by SndCreateMidiList.
typedef struct SndMidiListItemType
	{
	char		name[sndMidiNameLength];			// including NULL terminator
	uint32_t		uniqueRecID;
	DatabaseID	dbH;
	} SndMidiListItemType;


// Commands for SndPlaySmf
enum SndSmfCmdEnumTag {
	sndSmfCmdPlay = 1,					// play the selection
	sndSmfCmdDuration						// get the duration in milliseconds of the entire track
	} ;
typedef Enum8 SndSmfCmdEnum ;

typedef void SndComplFuncType(void *chanP, uint32_t dwUserData);
typedef SndComplFuncType *SndComplFuncPtr;


// Return true to continue, false to abort
typedef Boolean SndBlockingFuncType(void *chanP, uint32_t dwUserData, int32_t sysTicksAvailable);
typedef SndBlockingFuncType *SndBlockingFuncPtr;

typedef struct SndCallbackInfoType {
	MemPtr		funcP;			// pointer to the callback function (NULL = no function)
	uint32_t	dwUserData;		// value to be passed in the dwUserData parameter of the callback function
	} SndCallbackInfoType;


typedef struct SndSmfCallbacksType {
	SndCallbackInfoType	completion;		// completion callback function (see SndComplFuncType)
	SndCallbackInfoType	blocking;		// blocking hook callback function (see SndBlockingFuncType)
	SndCallbackInfoType	reserved;		// RESERVED -- SET ALL FIELDS TO ZERO BEFORE PASSING
	} SndSmfCallbacksType;


#define sndSmfPlayAllMilliSec		0xFFFFFFFFUL

typedef struct SndSmfOptionsType {
	// dwStartMilliSec and dwEndMilliSec are used as inputs to the function for sndSmfCmdPlay and as
	// outputs for sndSmfCmdDuration
	uint32_t	dwStartMilliSec;				// 0 = "start from the beginning"
	uint32_t	dwEndMilliSec;					// sndSmfPlayAllMilliSec = "play the entire track";
													// the default is "play entire track" if this structure
													// is not passed in
	
	// The amplitude and interruptible fields are used only for sndSmfCmdPlay
	uint16_t	amplitude;						// relative volume: 0 - sndMaxAmp, inclusively;  the default is
													// sndMaxAmp if this structure is not passed in; if 0, the play will
													// be skipped and the call will return immediately
	
	Boolean	interruptible;					// if true, sound play will be interrupted if
													// user interacts with the controls (digitizer, buttons, etc.);
													// if false, the paly will not be interrupted; the default behavior
													// is "interruptible" if this structure is not passed in
													
	uint8_t		reserved1;
	uint32_t	reserved;						// RESERVED! -- MUST SET TO ZERO BEFORE PASSING
	} SndSmfOptionsType;


typedef struct SndSmfChanRangeType {
	uint8_t	bFirstChan;							// first MIDI channel (0-15 decimal)
	uint8_t	bLastChan;							// last MIDI channel (0-15 decimal)
	} SndSmfChanRangeType;



//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

// Sets default sound volume levels
//
// Any parameter may be passed as NULL
extern void			SndSetDefaultVolume(uint16_t *alarmAmpP, uint16_t *sysAmpP, uint16_t *defAmpP);

// Gets default sound volume levels
//
// Any parameter may be passed as NULL
extern void			SndGetDefaultVolume(uint16_t *alarmAmpP, uint16_t *sysAmpP, uint16_t *masterAmpP);

// Executes a sound command on the given sound channel (pass
// channelP = 0 to use the shared channel).
extern status_t			SndDoCmd(void * /*SndChanPtr*/ channelP, SndCommandPtr cmdP, Boolean noWait);

// Plays one of several defined system beeps/sounds (see sndSysBeep...
// constants).
extern void			SndPlaySystemSound(SndSysBeepType beepID);


// NEW FOR v3.0
// Performs an operation on a Standard MIDI File (SMF) Format #0
extern status_t		SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, uint8_t *smfP, SndSmfOptionsType *selP,
						SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP,
						Boolean bNoWait);

// NEW FOR v3.0
// Creates a list of all midi records.  Useful for displaying in lists.
// For creator wildcard, pass creator=0;
extern Boolean		SndCreateMidiList(uint32_t creator, Boolean multipleDBs, uint16_t *wCountP, MemHandle *entHP);
							
// NEW FOR v3.2
// Plays a MIDI sound which is read out of an open resource database
extern status_t 			SndPlaySmfResource(uint32_t resType, DmOpenRef dbRef, int16_t resID, SystemPreferencesChoice volumeSelector);




#ifdef __cplusplus 
}
#endif


#endif
