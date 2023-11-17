/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TraceMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *             	Tracing API
 *
 *****************************************************************************/

#ifndef __TRACEMGR_H__
#define __TRACEMGR_H__

/* ------------------------------------------------------------------- */
#include <PalmTypes.h>
#include <CmnErrors.h>
#include <stdarg.h>

/* ------------------------------------------------------------------- 

Expected syntax:
	TraceOutput(T   (errorClass,"format",...)     )
	TraceOutput(TL  (errorClass,"format",...)     )
	TraceOutput(B   (errorClass,addr,count)       )
	TraceOutput(VT  (errorClass,"format",va_list) )
	TraceOutput(VTL (errorClass,"format",va_list) )
or (shorter):
	TM(T  (errorClass,"format",...)     )
	TM(TL (errorClass,"format",...)     )
	TM(B  (errorClass,addr,count)       )
	TM(VT (errorClass,"format",va_list) )
	TM(VTL(errorClass,"format",va_list) )

Error classes are listed in SDK\Int\Core\Common\6.0\Main. 
Applications should use appErrorClass.

Format string: % flags width type

Supported flags: 
- 		Left justified display (default is right justified)
+		Always displays the sign symbol (default: display only '-')
space	Displays a space instead of a '+' symbol
 
Supported types:
ld		int32_t
lu		uint32_t
lx,lX	uint32_t in hexadecimal
hd		int16_t
hu		uint16_t
hx,hX	uint16_t in hexadecimal
s		0 terminated string
c		character
%		the % character
 
---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

void TmOutputVT	(status_t traceModule, const char* aFormatString, va_list arglist);
void TmOutputVTL(status_t traceModule, const char* aFormatString, va_list arglist);
void TmOutputT	(status_t traceModule, const char* aFormatString, ...);
void TmOutputTL	(status_t traceModule, const char* aFormatString, ...);
void TmOutputB	(status_t traceModule, const void* aBuffer, long aBufferLen);

/* ------------------------------------------------------------------- */

#if defined(TRACE_OUTPUT) && TRACE_OUTPUT != TRACE_OUTPUT_OFF
#	define	TraceOutput(X)	TmOutput##X
// Even shorter
#	define	TM(X)			TmOutput##X
#else
#	define	TraceOutput(X)		
#	define	TM(X)
#endif

/* ------------------------------------------------------------------- */

#define TraceDefine(x,y) (x+y)	// Used for custom error classes

#ifdef __cplusplus
}
#endif

#endif	/* __TRACEMGR_H__ */
