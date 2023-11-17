/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: CmnErrors.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Pre-defined error codes shared by DAL and Palm OS
 *
 *****************************************************************************/

#ifndef _CMNERRORS_H_
#define _CMNERRORS_H_

#include <sys/types.h>
#include <errno.h>

// Error reporting options
#define	errReportStripContext	0x00000001	// Originating file and line not available

// Possible responses
#define errResponseTryHighLevel	0x00000001	// Low level code recommends trying high level UI first
#define errResponseBreakNative	0x00000002	// Break in the native code debugger
#define errResponseBreak68K		0x00000004	// Break in the 68000 code debugger
#define errResponseBreakBoth	(errResponseBreakNative + errResponseBreak68K)
#define	errResponseIgnore		0x00000008	// Resume offending code
#define errResponseKillProcess	0x00000010	// Kill the process hosting the offending code
#define errResponseSoftReset	0x00000020	// Soft reset the Palm OS
#define errResponseShutdown		0x00000040	// Shutdown whole system (for the simulation this means: exit it)

#if BUILD_TYPE == BUILD_TYPE_DEBUG
#	define errResponseDefaultSet	(errResponseBreakNative | errResponseKillProcess |\
										errResponseSoftReset | errResponseShutdown | errResponseIgnore)
#else
#	define errResponseDefaultSet	(errResponseBreakNative | errResponseKillProcess |\
										errResponseSoftReset | errResponseShutdown )
#endif

#define errorClassMask        	0xFFFFFF00	// Mask for extracting class code

/************************************************************
 * Error Classes for each manager
 *************************************************************/
#define	errNone					0x00000000	// No error

#define	memErrorClass			0x80000100	// Memory Manager
#define	dmErrorClass			0x80000200	// Data Manager
#define	serErrorClass			0x80000300	// Serial Manager
#define	slkErrorClass			0x80000400	// Serial Link Manager
#define	sysErrorClass			0x80000500	// System Manager
#define	fplErrorClass			0x80000600	// Floating Point Library
#define	flpErrorClass			0x80000680	// New Floating Point Library
#define	evtErrorClass			0x80000700  // System Event Manager
#define	sndErrorClass			0x80000800  // Sound Manager
#define	almErrorClass			0x80000900  // Alarm Manager
#define	timErrorClass			0x80000A00  // Time Manager
#define	penErrorClass			0x80000B00  // Pen Manager
#define	ftrErrorClass			0x80000C00  // Feature Manager
#define	cmpErrorClass			0x80000D00  // Connection Manager (HotSync)
#define	dlkErrorClass			0x80000E00	// Desktop Link Manager
#define	padErrorClass			0x80000F00	// PAD Manager
#define	grfErrorClass			0x80001000	// Graffiti Manager
#define	mdmErrorClass			0x80001100	// Modem Manager
#define	netErrorClass			0x80001200	// Net Library
#define	htalErrorClass			0x80001300	// HTAL Library
#define	inetErrorClass			0x80001400	// INet Library
#define	exgErrorClass			0x80001500	// Exg Manager
#define	fileErrorClass			0x80001600	// File Stream Manager
#define	rfutErrorClass			0x80001700	// RFUT Library
#define	txtErrorClass			0x80001800	// Text Manager
#define	tsmErrorClass			0x80001900	// Text Services Library
#define	webErrorClass			0x80001A00	// Web Library
#define	secErrorClass			0x80001B00	// Security Library
#define	emuErrorClass			0x80001C00	// Emulator Control Manager
#define	flshErrorClass			0x80001D00	// Flash Manager
#define	pwrErrorClass			0x80001E00	// Power Manager
#define	cncErrorClass			0x80001F00	// Connection Manager (Serial Communication)
#define	actvErrorClass			0x80002000	// Activation application
#define	radioErrorClass		   	0x80002100	// Radio Manager (Library)
#define	dispErrorClass			0x80002200	// Display Driver Errors.
#define	bltErrorClass			0x80002300	// Blitter Driver Errors.
#define	winErrorClass			0x80002400	// Window manager.
#define	omErrorClass			0x80002500	// Overlay Manager
#define	menuErrorClass			0x80002600	// Menu Manager
#define	lz77ErrorClass			0x80002700	// Lz77 Library
#define	smsErrorClass			0x80002800	// Sms Library
#define	expErrorClass			0x80002900	// Expansion Manager and Slot Driver Library
#define	vfsErrorClass			0x80002A00	// Virtual Filesystem Manager and Filesystem library
#define	lmErrorClass			0x80002B00	// Locale Manager
#define	intlErrorClass			0x80002C00	// International Manager
#define pdiErrorClass			0x80002D00	// PDI Library
#define	attnErrorClass			0x80002E00	// Attention Manager
#define	telErrorClass			0x80002F00	// Telephony Manager
#define hwrErrorClass			0x80003000	// Hardware Manager (HAL)
#define	blthErrorClass			0x80003100	// Bluetooth Library Error Class
#define	udaErrorClass			0x80003200	// UDA Manager Error Class
#define	tlsErrorClass			0x80003300	// Thread Local Storage
#define em68kErrorClass			0x80003400	// 68K appl emulator
#define grmlErrorClass			0x80003500	// Gremlins
#define IrErrorClass			0x80003600	// IR Library
#define IrCommErrorClass		0x80003700	// IRComm Serial Mgr Plugin
#define cpmErrorClass			0x80003800  // Crypto Manager
#define sslErrorClass			0x80003900  // SSL (from RSA)
#define kalErrorClass			0x80003A00  // KAL errors
#define halErrorClass			0x80003B00  // HAL errors
#define azmErrorClass			0x80003C00  // Authorization Manager (AZM)
#define amErrorClass			0x80003D00  // Authentication Manager (AM)
#define dirErrorClass			0x80003E00  // Directory
#define svcErrorClass			0x80003F00  // Service Manager
#define appMgrErrorClass		0x80004000  // Application Manager
#define ralErrorClass			0x80004100  // RAL errors
#define iosErrorClass			0x80004200  // IOS errors
#define signErrorClass			0x80004300  // Digital Signature Verification shared library
#define perfErrorClass			0x80004400  // Performance Manager
#define drvrErrorClass			0x80004500  // Hardware Driver errors
#define mediaErrorClass			0x80004600  // Multimedia Error Class
#define catmErrorClass			0x80004700  // Category Mgr Errors
#define certErrorClass			0x80004800	// Certificate Manager Error Class
#define secSvcsErrorClass		0x80004900  // Security Services Errors
#define bndErrorClass			0x80004A00  // Binder Error Class
#define syncMgrErrorClass		0x80004B00	// Sync Manager Errors
#define HttpErrorClass			0x80004C00	// Http Lib errors
#define xSyncErrorClass 		0x80004D00	// Exchange Sync Library Errors
#define hsExgErrorClass			0x80004E00	// HotSync Exchange library errors
#define pppErrorClass			0x80004F00	// PPP lib errors
#define pinsErrorClass			0x80005000	// Pen Input Services errors
#define statErrorClass			0x80005100	// Status Bar Service errors
#define regexpErrorClass		0x80005200	// Regular Expression errors
#define posixErrorClass			0x80005300	// All those nice POSIX errors
#define uilibErrorClass			0x80005400	// UI Library (Forms, Controls, etc)

#define	oemErrorClass			0x80007000	// OEM/Licensee errors (0x80007000-0x80007EFF shared among ALL partners)
#define errInfoClass			0x80007F00	// special class shows information w/o error code
#define	appErrorClass			0x80008000	// Application-defined errors

#define	dalErrorClass			0x8000FF00	// DAL error class

/*******************************************************************
 *	Error Codes
 *******************************************************************/

#define	kDALError				((status_t)(dalErrorClass | 0x00FF))
#define	kDALTimeout				((status_t)(sysErrorClass | 1))	// compatible with sysErrTimeout

/************************************************************
 * System Errors
 *************************************************************/

#define sysErrTimeout					((status_t)(sysErrorClass | 1))
#define sysErrParamErr					((status_t)(sysErrorClass | 2))
#define sysErrNoFreeResource			((status_t)(sysErrorClass | 3))
#define sysErrNoFreeRAM					((status_t)(sysErrorClass | 4))
#define sysErrNotAllowed				((status_t)(sysErrorClass | 5))
#define	sysErrOutOfOwnerIDs				((status_t)(sysErrorClass | 8))
#define	sysErrNoFreeLibSlots			((status_t)(sysErrorClass | 9))
#define	sysErrLibNotFound				((status_t)(sysErrorClass | 10))
#define sysErrModuleNotFound			sysErrLibNotFound
#define	sysErrRomIncompatible			((status_t)(sysErrorClass | 12))
#define sysErrBufTooSmall				((status_t)(sysErrorClass | 13))
#define	sysErrPrefNotFound				((status_t)(sysErrorClass | 14))

// NotifyMgr error codes:
#define	sysNotifyErrEntryNotFound		((status_t)(sysErrorClass | 16)) // could not find registration entry in the list
#define	sysNotifyErrDuplicateEntry		((status_t)(sysErrorClass | 17)) // identical entry already exists
#define	sysNotifyErrBroadcastBusy		((status_t)(sysErrorClass | 19)) // a broadcast is already in progress - try again later.
#define	sysNotifyErrBroadcastCancelled	((status_t)(sysErrorClass | 20)) // a handler cancelled the broadcast
#define sysNotifyErrNoServer			((status_t)(sysErrorClass | 21)) // can't find the notification server

// NotifyMgr Phase #2 Error Codes:
#define	sysNotifyErrQueueFull			((status_t)(sysErrorClass | 27)) // deferred queue is full.
#define	sysNotifyErrQueueEmpty			((status_t)(sysErrorClass | 28)) // deferred queue is empty.
#define	sysNotifyErrNoStackSpace		((status_t)(sysErrorClass | 29)) // not enough stack space for a broadcast
#define	sysErrNotInitialized			((status_t)(sysErrorClass | 30)) // manager is not initialized

// Loader error codes:
#define sysErrModuleInvalid				((status_t)(sysErrorClass | 31))  // following sysErrNotInitialized
#define sysErrModuleIncompatible		((status_t)(sysErrorClass | 32))
#define sysErrModuleFound68KCode		((status_t)(sysErrorClass | 33))  // ARM module not found, but 68K code resource was
#define sysErrModuleRelocationError		((status_t)(sysErrorClass | 34))  // Failed to apply relocation while loading a module
#define sysErrNoGlobalStructure			((status_t)(sysErrorClass | 35))  // Module has no global structure
#define sysErrInvalidSignature			((status_t)(sysErrorClass | 36))  // Module has no valid digital signuture
#define sysErrInternalError				((status_t)(sysErrorClass | 37))    // System encountered unexpected internal error
#define sysErrDynamicLinkerError		((status_t)(sysErrorClass | 38))    // Error occured during dynamic linking
#define sysErrRAMModuleNotAllowed		((status_t)(sysErrorClass | 39))    // RAM-based module cannot be loaded when device is booted into ROM-only mode
#define sysErrCPUArchitecture			((status_t)(sysErrorClass | 40))  // Program needs different architecture of the CPU to run

// More general error codes:
#define sysErrBadIndex					((status_t)(sysErrorClass | 41))  // Out-of-range index supplied to function
#define sysErrBadType					((status_t)(sysErrorClass | 42))  // Bad argument type supplied to function
#define sysErrMismatchedValues			((status_t)(sysErrorClass | 43))  // Unexpected value supplied to function
#define sysErrNameNotFound				((status_t)(sysErrorClass | 44))  // The requested name does not exist
#define sysErrNameInUse					((status_t)(sysErrorClass | 45))  // The requested name already exists
#define sysErrNoInit					((status_t)(sysErrorClass | 46))  // Target not initialized
#define sysErrBadData					((status_t)(sysErrorClass | 47))  // Input data is corrupt
#define sysErrDataTruncated				((status_t)(sysErrorClass | 48))  // Not all of the data got through
#define sysErrIO						((status_t)(sysErrorClass | 49))  // General IO error
#define sysErrBadDesignEncountered		((status_t)(sysErrorClass | 50))  // Will be fixed in the next version

// AppMgr error codes:
#define sysErrProcessFaulted			((status_t)(sysErrorClass | 51))  // Process has faulted during execution

#define sysErrWeakRefGone				((status_t)(sysErrorClass | 52))  // I was holding a weak reference, and that object is gone now

#define sysErrOSVersion					((status_t)(sysErrorClass | 53))  // Program needs higher OS version to run

// Yet more general error codes (post-6.0.x)
#define sysErrEndOfData					((status_t)(sysErrorClass | 54))  // General iteration end "error" code

//We may be building the support kit for windows so make sure some of these are defined
#ifndef ECANCEL
#define	ECANCEL		5
#endif

#ifndef ENOTSUP
#define	ENOTSUP		45
#endif

#ifndef EWOULDBLOCK
#define	EWOULDBLOCK	EAGAIN
#endif

// Nice names for some POSIX error codes:
#define sysErrCanceled					((status_t)(posixErrorClass | ECANCEL))
#define sysErrPermissionDenied			((status_t)(posixErrorClass | EPERM))
#define sysErrUnsupported				((status_t)(posixErrorClass | ENOTSUP))
#define sysErrBrokenPipe				((status_t)(posixErrorClass | EPIPE))
#define sysErrEntryNotFound				((status_t)(posixErrorClass | ENOENT))
#define sysErrEntryExists				((status_t)(posixErrorClass | EEXIST))
#define sysErrNameTooLong				((status_t)(posixErrorClass | E2BIG))
#define sysErrWouldBlock				((status_t)(posixErrorClass | EWOULDBLOCK))
#define sysErrBusy						((status_t)(posixErrorClass | EBUSY))
#define sysErrOutOfRange				((status_t)(posixErrorClass | ERANGE))
#define sysErrDontDoThat				((status_t)(posixErrorClass | EACCES))
#define sysErrBadAddress				((status_t)(posixErrorClass | EBADADDR))
#define sysErrInterrupted				((status_t)(posixErrorClass | EINTR))


/************************************************************
 * Binder Errors
 *************************************************************/

#define bndErrMissingArg				((status_t)(bndErrorClass | 1))  // Required argument not supplied
#define bndErrBadType					((status_t)(bndErrorClass | 2))  // Given argument has invalid type
#define bndErrDead						((status_t)(bndErrorClass | 3))  // Target Binder is no longer with us
#define bndErrUnknownTransact			((status_t)(bndErrorClass | 4))  // Don't know the given transaction code
#define bndErrBadTransact				((status_t)(bndErrorClass | 5))  // Transaction data is corrupt
#define bndErrTooManyLoopers			((status_t)(bndErrorClass | 6))  // No more asynchronicity for you!
#define bndErrBadInterface				((status_t)(bndErrorClass | 7))  // Requested interface is not implemented
#define bndErrUnknownMethod				((status_t)(bndErrorClass | 8))  // Binder does not implement requested method
#define bndErrUnknownProperty			((status_t)(bndErrorClass | 9))  // Binder does not implement requested property
#define bndErrOutOfStack				((status_t)(bndErrorClass | 10)) // No more recursion for you!

/************************************************************
 * RegExp Errors
 *************************************************************/

#define regexpErrUnmatchedParenthesis			((status_t)(regexpErrorClass | 1))  // ())
#define regexpErrTooBig							((status_t)(regexpErrorClass | 2))  // Not that we like hard-coded limits
#define regexpErrTooManyParenthesis				((status_t)(regexpErrorClass | 3))  // No, really, we don't
#define regexpErrJunkOnEnd						((status_t)(regexpErrorClass | 4))  // Junk isn't good for you
#define regexpErrStarPlusOneOperandEmpty		((status_t)(regexpErrorClass | 5))  // As it says
#define regexpErrNestedStarQuestionPlus			((status_t)(regexpErrorClass | 6))  // Likewise
#define regexpErrInvalidBracketRange			((status_t)(regexpErrorClass | 8))  // Bad stuff inside []
#define regexpErrUnmatchedBracket				((status_t)(regexpErrorClass | 9))  // []]
#define regexpErrInternalError					((status_t)(regexpErrorClass | 10))  // Uh oh!
#define regexpErrQuestionPlusStarFollowsNothing	((status_t)(regexpErrorClass | 11))  // That's a mouthful
#define regexpErrTrailingBackslash				((status_t)(regexpErrorClass | 12))  // This isn't C
#define regexpErrCorruptedProgram				((status_t)(regexpErrorClass | 13))  // Poor program
#define regexpErrMemoryCorruption				((status_t)(regexpErrorClass | 14))  // Poor memory
#define regexpErrCorruptedPointers				((status_t)(regexpErrorClass | 15))  // Poor pointer
#define regexpErrCorruptedOpcode				((status_t)(regexpErrorClass | 16))  // Poor opcode

/************************************************************
 * Media Errors
 *************************************************************/

#define mediaErrFormatMismatch			((status_t)(mediaErrorClass | 1))
#define mediaErrAlreadyVisited			((status_t)(mediaErrorClass | 2))
#define mediaErrStreamExhausted			((status_t)(mediaErrorClass | 3))
#define mediaErrAlreadyConnected		((status_t)(mediaErrorClass | 4))
#define mediaErrNotConnected			((status_t)(mediaErrorClass | 5))
#define mediaErrNoBufferSource			((status_t)(mediaErrorClass | 6))
#define mediaErrBufferFlowMismatch		((status_t)(mediaErrorClass | 7))

#endif	// _CMNERRORS_H_
