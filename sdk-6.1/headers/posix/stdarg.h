/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: stdarg.h 
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _PALMOS_STDARG_H_
#define _PALMOS_STDARG_H_

/*
	This header file is built from the definitions used by ADS and MSVC 7,
	for ARM and WinNT respectively.
*/

#include <sys/cdefs.h>

#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/ansi.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/ansi.h>
#else
#error Building on unsupported platform
#endif

__BEGIN_DECLS

/* Needed for ADS 1.2, which will replace this (noop) definition with its own */
#define va_arg_8(ap,t) va_arg(ap, t)

#if defined (__INTEL_COMPILER)

/* intel compiler  */
#ifndef _VA_LIST
typedef char *va_list;
#define _VA_LIST
#endif
#define va_start(ap,v) 	__builtin_stdarg_start(&(ap),v)
#define va_arg(ap,t) 	__builtin_va_arg(*(ap),t)
#define va_copy(dest, src) ((void)(*(dest) = *(src)))
#define va_end(ap) ((void)(*(ap) = 0))

#elif defined(__ARMCC_VERSION)

/* Currently ADS 1.2 ABI */

typedef _BSD_VA_LIST_ va_list;

#define va_start(ap,v)			(void)((ap).__ap = __va_start(v))
#define va_arg(ap,t)			__va_arg((ap).__ap, t)
#undef va_arg_8

#define va_arg_8(ap,t)		((ap).__ap = (void*)(((unsigned long)((ap).__ap)) + 7 & ~7), __va_arg((ap).__ap, t))
#define va_end(ap)			((void)((ap).__ap = 0))
#define va_copy(dest,src)	((void)((dest).__ap = (src).__ap))

/*
typedef int *va_list[1];

#define va_start(ap, parmN) (void)(*(ap) = __va_start(parmN))
#define va_arg(ap, type) __va_arg(*(ap), type)
#define va_copy(dest, src) ((void)(*(dest) = *(src)))
#define va_end(ap) ((void)(*(ap) = 0))
*/

#elif defined (_MSC_VER)

/* MSVC 7.0 ABI */

typedef char *  va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#ifdef  __cplusplus
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#else
#define _ADDRESSOF(v)   ( &(v) )
#endif

#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#elif defined(_PACC_VER)

#ifdef __cplusplus
extern "C" {
#endif
extern char *__va_funcx(int, void*, ...); /* expanded by compiler */
#ifdef __cplusplus
}
#endif

typedef _BSD_VA_LIST_ va_list;

#define va_start(ap,N) (void)((ap).__ap = (int*)__va_funcx(2, (char *) &(N)))
#define va_arg(ap,T) (*((T*) __va_funcx(3, &((ap).__ap), ((T*)0))))
#define va_end(ap) ((void) 0)
#define va_copy(dst,src) ((dst).__ap = (src).__ap)

#elif defined (__GNUC__)

#include_next <stdarg.h>

#endif

__END_DECLS

#endif /* !_PALMOS_STDARG_H_ */
