/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ErrTryCatch.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Include file for Error Management 
 *
 *****************************************************************************/

#ifndef __ERRTRYCATCH_H__
#define __ERRTRYCATCH_H__

/********************************************************************
 * Try / Catch / Throw support
 *
 * ---------------------------------------------------------------------
 * Exception Handler structure
 *  
 *    An ErrExceptionType object is created for each ErrTry & ErrCatch block.
 *    At any point in the program, there is a linked list of
 *    ErrExceptionType objects. GErrFirstException points to the
 *    most recently entered block. A ErrExceptionType blocks stores
 *    information about the state of the machine (register values)
 *    at the start of the Try block
 ********************************************************************/

#include <PalmTypes.h>

#if TARGET_PLATFORM == TARGET_PLATFORM_PALMSIM_WIN32

/* Replicate the Windows setjmp prototypes and environment */
#ifdef  _MSC_VER
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* _CRTIMP */

#ifndef _INC_SETJMPEX
#define setjmp  _setjmp
#endif

#define _JBLEN  16
#define _JBTYPE int

typedef struct __JUMP_BUFFER {
    unsigned long Ebp;
    unsigned long Ebx;
    unsigned long Edi;
    unsigned long Esi;
    unsigned long Esp;
    unsigned long Eip;
    unsigned long Registration;
    unsigned long TryLevel;
    unsigned long Cookie;
    unsigned long UnwindFunc;
    unsigned long UnwindData[6];
} _JUMP_BUFFER;

/* Define the buffer type for holding the state information */

#ifndef _JMP_BUF_DEFINED
typedef _JBTYPE jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif

int __cdecl setjmp(jmp_buf);

#if     _MSC_VER >= 1200
_CRTIMP __declspec(noreturn) void __cdecl longjmp(jmp_buf, int);
#else
_CRTIMP void __cdecl longjmp(jmp_buf, int);
#endif

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#	define ErrJumpBuf jmp_buf
#elif TARGET_PLATFORM == TARGET_PLATFORM_DEVICE_ARM
	typedef long* ErrJumpBuf[16];		// r4-r11, lr, sp, space for 24 more bytes
#else
#	error Impossible to define ErrJumpBuf
#endif
	
// Structure used to store Try state.
typedef struct ErrExceptionType {
  struct ErrExceptionType*	nextP;	    // next exception type
  ErrJumpBuf			state;	    // setjmp/longjmp storage
  int32_t				err;	    // Error code
  VAddr                         errVAddr;   // Error address
} ErrExceptionType;
typedef ErrExceptionType *ErrExceptionPtr;


// Try & Catch macros
#define	ErrTry                                                \
  {                                                           \
    ErrExceptionType  _TryObject;                             \
    _TryObject.err = 0;                                       \
    ErrExceptionListAppend(&_TryObject);                      \
    if (ErrSetJump(_TryObject.state) == 0) {

    
// NOTE: All variables referenced in and after the ErrCatch must 
// be declared volatile.  Here's how for variables and pointers:
// volatile uint32_t          oldMode;
//  ShlDBHdrTablePtr volatile hdrTabP = nil;
// If you have many local variables after the ErrCatch you may
// opt to put the ErrTry and ErrCatch in a separate enclosing function.


// old macro, for backwards compatibility
#define  ErrCatch(inErr)                                      \
      ErrExceptionListRemove(&_TryObject);                    \
    } else {                                                  \
      int32_t  inErr = _TryObject.err;                          \
      ErrExceptionListRemove(&_TryObject);                    

// new macro that gives the address reference that caused fault if available
#define  ErrCatchWithAddress(inErr, inErrVAddr)               \
      ErrExceptionListRemove(&_TryObject);                    \
    } else {                                                  \
      int32_t  inErr = _TryObject.err;                          \
      VAddr  inErrVAddr = _TryObject.errVAddr;                \
      ErrExceptionListRemove(&_TryObject);                          
      
#define  ErrEndCatch                                          \
    }                                                         \
  }


/********************************************************************
 * Error Manager Routines
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_PLATFORM == TARGET_PLATFORM_PALMSIM_WIN32
#define ErrSetJump setjmp
#define ErrLongJump longjmp
#else
int32_t    ErrSetJump(ErrJumpBuf buf);
void     ErrLongJump(ErrJumpBuf buf, int32_t result);
#endif
void     ErrExceptionListAppend(ErrExceptionType* errExceptionTypeP);
void     ErrExceptionListRemove(ErrExceptionType* errExceptionTypeP);

  // Throw an error to the first handler on the Exception list for this Thread.
  // New Throw with an errVAddr that can be reported to the ErrCatchWithAddress
void     ErrThrowWithAddress(int32_t err, VAddr errVAddr);

  // For backwards compatiblity
#define  ErrThrow(err) ErrThrowWithAddress(err, 0xffffffff)

// The next two functions are designed to be used by a Keeper Thread.
// This first function gets the head of the Exception list.  It doesn't
//   change the list at all though.  We considered setting it to NULL
//   and having the recovery code restore the next in the list, because we
//   thought this might be a way to prevent infinite loops where if Thread
//   faults again before it can remove the previously registered handler,
//   but that design is hard to do without a stack using recovery handler.
//   The recovery handler can't use the stack though because it might be
//   recoverying from the a stack fault, and so must do just the ErrJump.
ErrExceptionType* ErrExceptionListGetByThreadID(SysHandle iThreadID);

// This second function is what the Keeper Thread should make the faulted 
//   Thread run when it resumes.  This function will do the longjmp to
//   make the faulted Thread clear it's stack and jump to it's ErrCatch
//   handler. 
void ErrThrowWithHandler(int32_t err, VAddr errVAddr, ErrExceptionType *tryP);

// test the err code before throw...
#define ErrThrowIf(err) do{if (errNone != err) ErrThrow(err)}while(0);
              
#ifdef __cplusplus 
}
#endif




#endif // __ERRTRYCATCH_H__
