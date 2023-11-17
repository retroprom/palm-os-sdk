/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DebugMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		DebugMgr API.
 *
 *****************************************************************************/

#ifndef _DEBUGMGR_H_
#define _DEBUGMGR_H_

#include <PalmTypes.h>
#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Sends a null terminated string to current debug device.  The
// string is NOT guaranteed to be sent atomically; concurrent
// DbgMessage() calls may result in interleaved output unless
// some mutual exclusion is provided by the caller.
void DbgMessage(const char* iStringP);

// Varargs version of DbgMessage that builds a string using vsprintf
// to a buffer of max length 200 characters, and then sends it out
// via DbgMessage.  Text beyond this maximum will be truncated.
// Returns the number of bytes written.
long DbgVPrintf(const char* iFormat, va_list arglist);

// Varargs version of DbgMessage that builds a string using vsprintf
// to a buffer of max length 200 characters, and then sends it out
// via DbgMessage.  Text beyond this maximum will be truncated.
// Returns the number of bytes written.
long DbgPrintf(const char* iFormat, ...);

// If the output device is buffered, this call forces the output
// stream to be flushed.  The call does not return until all pending
// output has been written out.  If the output device is not buffered,
// this call is a no-op.
void DbgOutputSync(void);

// Reads a single character from the debug device.  Note that on some
// devices, this may currently block all Threads until the character
// has been read.
char DbgGetChar(void);

// Connects to the external debugger and halts execution.  Currently
// this halts the entire device.  Future implementations may halt only
// the calling Thread.
#if TARGET_PLATFORM == TARGET_PLATFORM_PALMSIM_WIN32
/* This allows to break in the current stack frame - easier debugging */
#define DbgBreak() __asm { int 3 }
#else
void DbgBreak(void);
#endif

#define DbgBreakMessage(message) \
  { DbgMessage(message); DbgBreak(); }

#define DbgBreakMessageIf(condition, message) \
  if (condition) { DbgMessage(message); DbgBreak(); }

// Retrieve a stack crawl for the current thread.  The 'outAddresses' will
// be filled in with the address of each function on the stack, from the
// immediate caller down.  The number of available slots to return is in
// 'maxResults'.  Use 'ignoreDepth' to skip functions at the top of the
// stack.  Returns the number of functions actually found.
// NOTE: ONLY IMPLEMENTED FOR PALMSIM.
int32_t DbgWalkStack(int32_t ignoreDepth, int32_t maxResults, void** outAddresses);

// Given a function address, return the raw symbol name for this
// function.
// NOTE: ONLY IMPLEMENTED FOR PALMSIM.
status_t DbgLookupSymbol(const void* addr, int32_t maxLen, char* outSymbol, void** outAddr);

// Given a symbol name, return the corresponding unmangled name.
// NOTE: ONLY IMPLEMENTED FOR PALMSIM.
status_t DbgUnmangleSymbol(char* symbol, int32_t maxLen, char* outName);

// Clear out all malloc() profiling statistics that have been collected
// so far for the current process.
void DbgRestartMallocProfiling(void);
void DbgSetMallocProfiling(Boolean enabled, int32_t dumpPeriod, int32_t maxItems, int32_t stackDepth);

// Is the debugger connected?
Boolean DbgIsPresent(void);

// Low-level error reporting - this gives a few options to the developer such as ignore, break, etc.
uint32_t DbgFatalErrorInContext(const char* fileName, uint32_t lineNum, const char* errMsg, uint32_t options, uint32_t allowedResponses);

#if 0
// Future ideas for a DebugMgr architecture with pluggable drivers
// that will allow output to go to different targets such as:
//   a) ARM semi-hosting
//   b) serial
//   c) memory log

// Set new driver to be the current one
void DbgSetDriver();

// Get information about current debugging state (current debug driver,
// number of drivers registered, etc).
void DbgGetInfo();

// Get information about specific driver.
void DbgDriverGetInfo();

// Register and unregister a driver plugin.
void DbgDriverRegister();
void DbgDriverUnregister();

#endif

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif  //_DEBUGMGR_H_
