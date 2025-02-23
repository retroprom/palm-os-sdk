/******************************************************************************
 *
 * Copyright (c) 1998-2004 PalmSource, Inc. All rights reserved.
 *
 * File: HostControl.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	This file is part of the Palm OS Emulator.
 *
 *****************************************************************************/

#ifndef _HOSTCONTROL_H_
#define _HOSTCONTROL_H_

#include <PalmTypes.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
	Set the values for selectors. Note that these values MUST be
	two bytes long and have the high byte be non-zero. The reason
	for this has to do with the way SysGremlins was originally
	declared. It took a GremlinsSelector enumerated value. Originally,
	there was only one value, and it was zero. The way the 68K compiler
	works, it decides that GremlinsSelectors are only one byte long,
	so a call to SysGremlins would push one byte onto the stack. Because
	all values on the stack need to be word-aligned, the processor
	subtracts 1 from the stack before pushing on the byte. Therefore,
	the stack looks like:

			previous contents
			garbage byte
			selector
			return address

	When the two middle bytes are read together as a word, they appear
	as: <selector> * 256 + <garbage byte>.

	With this setup, we have two choices: leave the selector size at
	one byte and limit ourselves to 256 functions, or define the selector
	to be a two byte value, with the first 256 values (all those with 0x00
	in the upper byte) to be GremlinIsOn. The latter sounds preferable, so
	we start the new selectors at 0x0100.
*/


#define hostSelectorBase					0x0100

	// Host information selectors

#define hostSelectorGetHostVersion			0x0100
#define hostSelectorGetHostID				0x0101
#define hostSelectorGetHostPlatform			0x0102
#define hostSelectorIsSelectorImplemented	0x0103
#define hostSelectorGestalt					0x0104
#define hostSelectorIsCallingTrap			0x0105


	// Profiler selectors

#define hostSelectorProfileInit				0x0200
#define hostSelectorProfileStart			0x0201
#define hostSelectorProfileStop				0x0202
#define hostSelectorProfileDump				0x0203
#define hostSelectorProfileCleanup			0x0204
#define hostSelectorProfileDetailFn			0x0205


	// Std C Library wrapper selectors

#define hostSelectorErrNo					0x0300

#define hostSelectorFClose					0x0301
#define hostSelectorFEOF					0x0302
#define hostSelectorFError					0x0303
#define hostSelectorFFlush					0x0304
#define hostSelectorFGetC					0x0305
#define hostSelectorFGetPos					0x0306
#define hostSelectorFGetS					0x0307
#define hostSelectorFOpen					0x0308
#define hostSelectorFPrintF					0x0309		/* Floating point not yet supported in Poser */
#define hostSelectorFPutC					0x030A
#define hostSelectorFPutS					0x030B
#define hostSelectorFRead					0x030C
#define hostSelectorRemove					0x030D
#define hostSelectorRename					0x030E
#define hostSelectorFReopen					0x030F		/* Not yet implemented in Poser */
#define hostSelectorFScanF					0x0310		/* Not yet implemented */
#define hostSelectorFSeek					0x0311
#define hostSelectorFSetPos					0x0312
#define hostSelectorFTell					0x0313
#define hostSelectorFWrite					0x0314
#define hostSelectorTmpFile					0x0315
#define hostSelectorTmpNam					0x0316

#define hostSelectorGetEnv					0x0317

#define hostSelectorMalloc					0x0318
#define hostSelectorRealloc					0x0319
#define hostSelectorFree					0x031A

#define hostSelectorVFPrintF				0x031B
#define hostSelectorVFScanF					0x031C

	// time.h wrappers
#define hostSelectorAscTime					0x0370
#define hostSelectorClock					0x0371
#define hostSelectorCTime					0x0372
// #define hostSelectorDiffTime				0x0373
#define hostSelectorGMTime					0x0374
#define hostSelectorLocalTime				0x0375
#define hostSelectorMkTime					0x0376
#define hostSelectorStrFTime				0x0377
#define hostSelectorTime					0x0378

	// dirent.h wrappers
#define hostSelectorOpenDir					0x0380
#define hostSelectorReadDir					0x0381
// #define hostSelectorRewindDir				0x0382
#define hostSelectorCloseDir				0x0383
// #define hostSelectorTellDir					0x0384
// #define hostSelectorSeekDir					0x0385
// #define hostSelectorScanDir					0x0386

	// fcntl.h wrappers
// #define hostSelectorOpen					0x0386
// #define hostSelectorCreat					0x0388
// #define hostSelectorFcntl					0x0389

	// unistd.h wrappers
// #define hostSelectorAccess					0x038A
// #define hostSelectorChDir					0x038B
// #define hostSelectorClose					0x038C
// #define hostSelectorDup						0x038D
// #define hostSelectorDup2					0x038E
// #define hostSelectorGetCwd					0x038F
// #define hostSelectorIsATTY					0x0390
// #define hostSelectorLink					0x0391
// #define hostSelectorLSeek					0x0392
// #define hostSelectorRead					0x0393
#define hostSelectorRmDir					0x0394
// #define hostSelectorTTYName					0x0395
// #define hostSelectorUnlink					0x0396
// #define hostSelectorWrite					0x0397

// #define hostSelectorFChDir					0x0398
// #define hostSelectorFChMod					0x0399
// #define hostSelectorFileNo					0X039A
// #define hostSelectorFSync					0x039B
// #define hostSelectorFTruncate				0x039C
// #define hostSelectorGetHostName				0x039D
// #define hostSelectorGetWD					0x039E
// #define hostSelectorMkSTemp					0x039F
// #define hostSelectorMkTemp					0x03A0
// #define hostSelectorRe_Comp					0x03A1
// #define hostSelectorRe_Exec					0x03A2
// #define hostSelectorReadLink				0x03A3
// #define hostSelectorSetHostName				0x03A4
// #define hostSelectorSymLink					0x03A5
// #define hostSelectorSync					0x03A6
#define hostSelectorTruncate				0x03A7

	// sys/stat.h wrappers
// #define hostSelectorChMod					0x03A8
// #define hostSelectorFStat					0x03A9
#define hostSelectorMkDir					0x03AA
#define hostSelectorStat					0x03AB
// #define hostSelectorLStat					0x03AC

	// sys/time.h wrappers
// #define hostSelectorGetTimeOfDay			0x03AD
#define hostSelectorUTime					0x03AE

	// DOS attr 
#define hostSelectorGetFileAttr				0x03AF
#define hostSelectorSetFileAttr				0x03B0

	// Gremlin selectors

#define hostSelectorGremlinIsRunning		0x0400
//#define hostSelectorGremlinNumber			0x0401
//#define hostSelectorGremlinCounter			0x0402
//#define hostSelectorGremlinLimit			0x0403
//#define hostSelectorGremlinNew				0x0404

	// Database selectors

#define hostSelectorImportFile				0x0500
#define hostSelectorExportFile				0x0501
#define hostSelectorSaveScreen				0x0502

#define hostSelectorExgLibOpen				0x0580
#define hostSelectorExgLibClose				0x0581
#define hostSelectorExgLibSleep				0x0582
#define hostSelectorExgLibWake				0x0583
#define hostSelectorExgLibHandleEvent		0x0584
#define hostSelectorExgLibConnect			0x0585
#define hostSelectorExgLibAccept			0x0586
#define hostSelectorExgLibDisconnect		0x0587
#define hostSelectorExgLibPut				0x0588
#define hostSelectorExgLibGet				0x0589
#define hostSelectorExgLibSend				0x058A
#define hostSelectorExgLibReceive			0x058B
#define hostSelectorExgLibControl			0x058C
#define hostSelectorExgLibRequest			0x058D


	// Preferences selectors

#define hostSelectorGetPreference			0x0600
#define hostSelectorSetPreference			0x0601


	// Logging selectors

#define hostSelectorLogFile					0x0700
#define hostSelectorSetLogFileSize			0x0701


	// RPC selectors

#define hostSelectorSessionCreate			0x0800		/* Not yet implemented in Poser */
#define hostSelectorSessionOpen				0x0801		/* Not yet implemented in Poser */
#define hostSelectorSessionClose			0x0802
#define hostSelectorSessionQuit				0x0803
#define hostSelectorSignalSend				0x0804
#define hostSelectorSignalWait				0x0805
#define hostSelectorSignalResume			0x0806


	// External tracing tool support

#define hostSelectorTraceInit				0x0900
#define hostSelectorTraceClose				0x0901
#define hostSelectorTraceOutputT			0x0902
#define hostSelectorTraceOutputTL			0x0903
#define hostSelectorTraceOutputVT			0x0904
#define hostSelectorTraceOutputVTL			0x0905
#define hostSelectorTraceOutputB			0x0906


	// Slot support

#define hostSelectorSlotMax					0x0A00
#define hostSelectorSlotRoot				0x0A01
#define hostSelectorSlotHasCard				0x0A02




	// File choosing support

#define hostSelectorGetFile					0x0B00
#define hostSelectorPutFile					0x0B01
#define hostSelectorGetDirectory			0x0B02


// PalmOS 5.0 support
#define hostSelectorEnteringApp				0x0C03
#define hostSelectorExitedApp				0x0C04

#define hostSelectorGet68KDebuggerPort		0x0C05	// Obsolete

#define hostSelectorSetErrorLevel			0x0C06
#define hostSelectorLogEvent				0x0C07

#define hostSelectorHostControl68K			0x0C08

// PalmOS 6.0 Support
#define hostSelectorGetChar					0x0C40
#define hostSelectorPrintF					0x0C41
#define hostSelectorVPrintF					0x0C42
#define hostSelectorGetFirstApp				0x0C43



#define hostSelectorLastTrapNumber			0x0CFF


	// * Types

typedef uint16_t	HostControlSelectorType;
typedef long	HostBoolType;
typedef long	HostClockType;
typedef long	HostErrType;
typedef long	HostIDType;
typedef long	HostPlatformType;
typedef long	HostSignalType;
typedef long	HostSizeType;
typedef long	HostTimeType;


	// * HostDIRType

struct HostDIRType
{
	long	_field;
};

typedef struct HostDIRType HostDIRType;


	// * HostDirEntType

#define HOST_NAME_MAX	255

struct HostDirEntType
{
	char	d_name[HOST_NAME_MAX + 1];
};

typedef struct HostDirEntType HostDirEntType;


	// * HostFILEType

struct HostFILEType
{
	long	_field;
};

typedef struct HostFILEType HostFILEType;

	// * HostStatType
	//		Note that the field names here have an underscore appended to
	//		them in order to differentiate them from "compatibility macros"
	//		under Solaris.

struct HostStatType
{
	unsigned long	st_dev_;
	unsigned long	st_ino_;
	unsigned long	st_mode_;
	unsigned long	st_nlink_;
	unsigned long	st_uid_;
	unsigned long	st_gid_;
	unsigned long	st_rdev_;
	HostTimeType	st_atime_;
	HostTimeType	st_mtime_;
	HostTimeType	st_ctime_;
	unsigned long	st_size_;
	unsigned long	st_blksize_;
	unsigned long	st_blocks_;
	unsigned long	st_flags_;
};

typedef struct HostStatType HostStatType;


	// * HostTmType
	//		Note that the field names here have an underscore appended to
	//		them for consistancy with HostStatType.

struct HostTmType
{
	long	tm_sec_;	/* seconds after the minute - [0,59] */
	long	tm_min_;	/* minutes after the hour - [0,59] */
	long	tm_hour_;	/* hours since midnight - [0,23] */
	long	tm_mday_;	/* day of the month - [1,31] */
	long	tm_mon_;	/* months since January - [0,11] */
	long	tm_year_;	/* years since 1900 */
	long	tm_wday_;	/* days since Sunday - [0,6] */
	long	tm_yday_;	/* days since January 1 - [0,365] */
	long	tm_isdst_;	/* daylight savings time flag */
};

typedef struct HostTmType HostTmType;


	// * HostUTimeType
	//		Note that the field names here have an underscore appended to
	//		them for consistancy with HostStatType.

struct HostUTimeType
{
	HostTimeType	crtime_;	/* creation time */
	HostTimeType	actime_;	/* access time */
	HostTimeType	modtime_;	/* modification time */
};

typedef struct HostUTimeType HostUTimeType;


	// * Backward compatiblity

typedef HostControlSelectorType	HostControlTrapNumber;
typedef HostBoolType			HostBool;
typedef HostErrType				HostErr;
typedef HostIDType				HostID;
typedef HostPlatformType		HostPlatform;
typedef HostSignalType			HostSignal;
typedef HostFILEType			HostFILE;



#ifndef hostErrorClass
	#define	hostErrorClass				0x1C00	// Host Control Manager
#else
	#if hostErrorClass != 0x1C00
		#error "You cannot change hostErrorClass without telling us."
	#endif
#endif

enum	// HostErrType values
{
	hostErrNone = 0,

	hostErrBase = hostErrorClass,

	hostErrUnknownGestaltSelector,
	hostErrDiskError,
	hostErrOutOfMemory,
	hostErrMemReadOutOfRange,
	hostErrMemWriteOutOfRange,
	hostErrMemInvalidPtr,
	hostErrInvalidParameter,
	hostErrTimeout,
	hostErrInvalidDeviceType,
	hostErrInvalidRAMSize,
	hostErrFileNotFound,
	hostErrRPCCall,				// Issued if the following functions are not called remotely:
								//		HostSessionCreate
								//		HostSessionOpen
								//		HostSessionClose
								//		HostSessionQuit
								//		HostSignalWait
								//		HostSignalResume
	hostErrSessionRunning,		// Issued by HostSessionCreate, HostSessionOpen, and
								// HostSessionQuit if a session is running.
	hostErrSessionNotRunning,	// Issued by HostSessionClose if no session is running.
	hostErrNoSignalWaiters,		// Issued by HostSendSignal if no one's waiting for a signal.
	hostErrSessionNotPaused,	// Issued when HostSignalResume, but the session was not
								// halted from a HostSignalSend call.
	
	hostErrPermissions,
	hostErrFileNameTooLong,
	hostErrNotADirectory,
	hostErrTooManyFiles,
	hostErrFileTooBig,
	hostErrReadOnlyFS,
	hostErrIsDirectory,
	hostErrExists,
	hostErrOpNotAvailable,
	hostErrDirNotEmpty,
	hostErrDiskFull,
	hostErrProfilingNotReady,
	hostErrUnknownError
};


enum	// HostIDType values
{
	hostIDPalmOS,			// The plastic thingy
	hostIDPalmOSEmulator,	// The Copilot thingy
	hostIDPalmOSSimulator	// The Mac libraries you link with thingy
};


enum	// HostPlatformType values
{
	hostPlatformPalmOS,
	hostPlatformWindows,
	hostPlatformMacintosh,
	hostPlatformUnix
};

enum	// HostSignalType values
{
	hostSignalReserved,
	hostSignalIdle,
	hostSignalQuit,
	hostSignalUser	= 0x40000000	// User-defined values start here and go up.
};

enum	// HostGet/SetFileAttr flags, matching EmFileAttr flags
{
	hostFileAttrReadOnly = 1,
	hostFileAttrHidden = 2,
	hostFileAttrSystem = 4
};

// Use these to call FtrGet to see if you're running under the
// Palm OS Emulator.  If not, FtrGet will return ftrErrNoSuchFeature.

#define kPalmOSEmulatorFeatureCreator	('pose')
#define kPalmOSEmulatorFeatureNumber	(0)

/* ==================================================================== */
/* Host environment-related calls										*/
/* ==================================================================== */
uint32_t		HostControl(HostControlTrapNumber selector, ...);


/* ==================================================================== */
/* Host environment-related calls										*/
/* ==================================================================== */

/*
When running on a real device, HostControl does nothing except setting R0 to 0.
This allows HosGetHostID to return "PalmOS", meaning that further HostControl functions MUST not be called.
*/
long				HostGetHostVersion(void);

HostIDType			HostGetHostID(void);

HostPlatformType	HostGetHostPlatform(void);

HostBoolType		HostIsSelectorImplemented(long selector);

HostErrType			HostGestalt(long gestSel, long* response);

HostBoolType		HostIsCallingTrap(void);


/* ==================================================================== */
/* Profiling-related calls												*/
/* ==================================================================== */

HostErrType			HostProfileInit(long maxCalls, long maxDepth);

HostErrType			HostProfileDetailFn(void* addr, HostBoolType logDetails);

HostErrType			HostProfileStart(void);

HostErrType			HostProfileStop(void);

HostErrType			HostProfileDump(const char* filename);

HostErrType			HostProfileCleanup(void);


/* ==================================================================== */
/* Std C Library-related calls											*/
/* ==================================================================== */

long				HostErrNo(void);


long				HostFClose(HostFILEType* f);

long				HostFEOF(HostFILEType* f);

long				HostFError(HostFILEType* f);

long				HostFFlush(HostFILEType* f);

long				HostFGetC(HostFILEType* f);

long				HostFGetPos(HostFILEType* f, long* posP);

char*				HostFGetS(char* s, long n, HostFILEType* f);

HostFILEType*		HostFOpen(const char* name, const char* mode);

long				HostFPrintF(HostFILEType* f, const char* fmt, ...);

long				HostVFPrintF(HostFILEType* f, const char* fmt, va_list args);

long				HostFPutC(long c, HostFILEType* f);

long				HostFPutS(const char* s, HostFILEType* f);

long				HostFRead(void* buffer, long size, long count, HostFILEType* f);

long				HostRemove(const char* name);

long				HostRename(const char* oldName, const char* newName);

HostFILEType*		HostFReopen(const char* name, const char* mode, HostFILEType* f);

long				HostFScanF(HostFILEType* f, const char* fmt, ...);

long				HostVFScanF(HostFILEType* f, const char* fmt, va_list args);

long				HostFSeek(HostFILEType* f, long offset, long origin);

long				HostFSetPos(HostFILEType* f, long* pos);

long				HostFTell(HostFILEType* f);

long				HostFWrite(const void* buffer, long size, long count, HostFILEType* f);

HostFILEType*		HostTmpFile(void);

char*				HostTmpNam(char* name);

char*				HostGetEnv(const char*);


void*				HostMalloc(long size);

void*				HostRealloc(void* p, long size);

void				HostFree(void* p);


char*				HostAscTime(const HostTmType*);

char*				HostCTime(const HostTimeType*);

HostClockType		HostClock(void);

//double				HostDiffTime(HostTimeType, HostTimeType);

HostTmType*			HostGMTime(const HostTimeType*);

HostTmType*			HostLocalTime(const HostTimeType*);

HostTimeType		HostMkTime(HostTmType*);

HostSizeType		HostStrFTime(char*, HostSizeType, const char*, const HostTmType*);

HostTimeType		HostTime(HostTimeType*);


long				HostMkDir(const char*);

long				HostRmDir(const char*);

HostDIRType*		HostOpenDir(const char*);

HostDirEntType*		HostReadDir(HostDIRType*);

long				HostCloseDir(HostDIRType*);


long				HostStat(const char*, HostStatType*);

long				HostTruncate(const char*, long);


long				HostUTime (const char*, HostUTimeType*);

long				HostGetFileAttr(const char*, long*);

long				HostSetFileAttr(const char*, long);

long				HostPrintF(const char* fmt, ...);

long				HostVPrintF(const char* fmt, va_list args);

long				HostGetChar(void);


/* ==================================================================== */
/* Gremlin-related calls												*/
/* ==================================================================== */

HostBoolType		HostGremlinIsRunning(void);


/* ==================================================================== */
/* Import/export-related calls											*/
/* ==================================================================== */

HostErrType			HostImportFile(const char* fileName);

HostErrType			HostExportFile(const char* fileName, const char* dbName);

HostErrType			HostSaveScreen(const char* fileName);

	// These are private, internal functions.  Third party applications
	// should not be calling them.

status_t					HostExgLibOpen			(uint16_t libRefNum);

status_t					HostExgLibClose			(uint16_t libRefNum);

status_t					HostExgLibSleep			(uint16_t libRefNum);

status_t					HostExgLibWake			(uint16_t libRefNum);

Boolean				HostExgLibHandleEvent	(uint16_t libRefNum, void* eventP);

status_t	 				HostExgLibConnect		(uint16_t libRefNum, void* exgSocketP);

status_t					HostExgLibAccept		(uint16_t libRefNum, void* exgSocketP);

status_t					HostExgLibDisconnect	(uint16_t libRefNum, void* exgSocketP,status_t error);

status_t					HostExgLibPut			(uint16_t libRefNum, void* exgSocketP);

status_t					HostExgLibGet			(uint16_t libRefNum, void* exgSocketP);

uint32_t 				HostExgLibSend			(uint16_t libRefNum, void* exgSocketP, const void* const bufP, const uint32_t bufLen, status_t* errP);

uint32_t 				HostExgLibReceive		(uint16_t libRefNum, void* exgSocketP, void* bufP, const uint32_t bufSize, status_t* errP);

status_t 				HostExgLibControl		(uint16_t libRefNum, uint16_t op, void* valueP, uint16_t* valueLenP);

status_t 				HostExgLibRequest		(uint16_t libRefNum, void* exgSocketP);


/* ==================================================================== */
/* Preference-related calls												*/
/* ==================================================================== */

HostBoolType		HostGetPreference(const char*, char*);

void				HostSetPreference(const char*, const char*);


/* ==================================================================== */
/* Logging-related calls												*/
/* ==================================================================== */

HostFILEType*		HostLogFile(void);

void				HostSetLogFileSize(long);


/* ==================================================================== */
/* RPC-related calls													*/
/* ==================================================================== */

HostErrType			HostSessionCreate(const char* device, long ramSize, const char* romPath);

HostErrType			HostSessionOpen(const char* psfFileName);

HostErrType			HostSessionClose(const char* saveFileName);

HostErrType			HostSessionQuit(void);

HostErrType			HostSignalSend(HostSignalType signalNumber);

HostErrType			HostSignalWait(long timeout, HostSignalType* signalNumber);

HostErrType			HostSignalResume(void);


/* ==================================================================== */
/* Tracing calls														*/
/* ==================================================================== */

void				HostTraceInit(void);

void				HostTraceClose(void);

void				HostTraceOutputT(unsigned short, const char*, ...);

void				HostTraceOutputTL(unsigned short, const char*, ...);

void				HostTraceOutputVT(unsigned short, const char*, va_list);

void				HostTraceOutputVTL(unsigned short, const char*, va_list);

void				HostTraceOutputB(unsigned short, const void*, long/*size_t*/);



/* ==================================================================== */
/* Slot related calls													*/
/* ==================================================================== */

long				HostSlotMax(void);

const char*			HostSlotRoot(long slotNo);

HostBoolType		HostSlotHasCard(long slotNo);



/* ==================================================================== */
/* File Choosing support												*/
/* ==================================================================== */

const char*			HostGetFile(const char* prompt, const char* defaultDir);

const char*			HostPutFile(const char* prompt, const char* defaultDir, const char* defaultName);

const char*			HostGetDirectory(const char* prompt, const char* defaultDir);


/* ==================================================================== */
/* UIAppShell callbacks													*/
/* ==================================================================== */

void				HostEnteringApp(char*);

void				HostExitedApp(char*, uint32_t);

/* ==================================================================== */
/* PACE Unsupported 68K HostControl calls								*/
/* ==================================================================== */

uint32_t				HostHostControl68K(HostControlSelectorType selector, void* pceEmulState);

/* ==================================================================== */
/* Auto-harness support													*/
/* ==================================================================== */


#define	kErrorLevelNone		0
#define kErrorLevelWarning	1
#define kErrorLevelFatal	2



void				HostSetErrorLevel(uint32_t, const char* msg);

/* ==================================================================== */
/* Event logging support												*/
/* ==================================================================== */

void				HostLogEvent(void*);

/* ==================================================================== */
/* AutoRun support														*/
/* ==================================================================== */

uint32_t				HostGetFirstApp(void);

/* ==================================================================== */
/* Done																	*/
/* ==================================================================== */



#ifdef __cplusplus 
}
#endif

#endif /* _HOSTCONTROL_H_ */
