/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ErrorMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *    Include file for Error Management that depends on BUILD_TYPE
 *
 *****************************************************************************/

#ifndef __ERRORMGR_H__
#define __ERRORMGR_H__

#include <PalmTypes.h>
#include <CmnErrors.h>

//	The (DbgOnly|Err)FatalError(If) macros can be used in two ways:
//	as a statement:
//		FatalError(c, m);
//	or as an expression:
//		(FatalError(c, m), value)

//-----------------------------------------------------------------------------
// New model
//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

void ErrErrorInContext		(const char* fileName, uint32_t lineNum, const char* errMsg, uint32_t options, uint32_t responses);	// New in 6.1
void ErrFatalErrorInContext	(const char* fileName, uint32_t lineNum, const char* errMsg);

#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------

#define ErrFatalError(errMsg)						ErrFatalErrorInContext(MODULE_NAME, __LINE__, errMsg)
#define ErrFatalErrorIf(condition, errMsg)			((condition) ? ErrFatalError(errMsg) : (void)0)

#if BUILD_TYPE != BUILD_TYPE_RELEASE

#	define DbgOnlyFatalError(errMsg)				ErrFatalErrorInContext(MODULE_NAME, __LINE__, errMsg)
#	define DbgOnlyFatalErrorIf(condition, errMsg)	((condition) ? DbgOnlyFatalError(errMsg) : (void)0)

#	define DbgOnlyNonFatalError(errMsg)				ErrErrorInContext(MODULE_NAME, __LINE__, errMsg, 0, errResponseDefaultSet | errResponseIgnore)
#	define DbgOnlyNonFatalErrorIf(condition, errMsg)((condition) ? DbgOnlyNonFatalError(errMsg) : (void)0)

#else

#	define DbgOnlyFatalError(errMsg)				/* NOP */
#	define DbgOnlyFatalErrorIf(condition, errMsg)	/* NOP */

#	define DbgOnlyNonFatalError(errMsg)				/* NOP */
#	define DbgOnlyNonFatalErrorIf(condition, errMsg)/* NOP */

#endif

//-----------------------------------------------------------------------------
// Compatibility
//-----------------------------------------------------------------------------

#define ErrFatalDisplay(msg)					ErrFatalError(msg)
#define ErrFatalDisplayIf(condition, msg)		ErrFatalErrorIf((condition), msg)
#define ErrDisplay(msg)							ErrFatalError(msg)
#define ErrDisplayFileLineMsg(a,b,c)			ErrFatalErrorInContext(a,b,c)
#define ErrNonFatalDisplay(msg)					DbgOnlyNonFatalError(msg)
#define ErrNonFatalDisplayIf(condition, msg)	DbgOnlyNonFatalErrorIf((condition), msg)

//-----------------------------------------------------------------------------

#endif // __ERRORMGR_H__
