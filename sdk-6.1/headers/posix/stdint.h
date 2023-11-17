/*	$NetBSD: stdint.h,v 1.2 2001/07/18 17:29:53 kleink Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Klaus Klein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


 /*
 * Copyright (c) 2003, PalmSource Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *		Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer.
 *
 *		Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution.
 *
 *		Neither the name of the PalmSource nor the names of its contributors may
 *		be used to endorse or promote products derived from this software without
 *		specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS_STDINT_H_
#define _SYS_STDINT_H_

#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/int_types.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/int_types.h>
#else
#error Building on unsupported platform
#endif

#ifndef	__int8_t_defined
typedef	__int8_t	int8_t;
#define __int8_t_defined 1
#endif

#ifndef	__uint8_t_defined
typedef	__uint8_t	uint8_t;
#define __uint8_t_defined 1
#endif

#ifndef	__int16_t_defined
typedef	__int16_t	int16_t;
#define __int16_t_defined 1
#endif

#ifndef	__uint16_t_defined
typedef	__uint16_t	uint16_t;
#define __uint16_t_defined 1
#endif

#ifndef	__int32_t_defined
typedef	__int32_t	int32_t;
#define __int32_t_defined 1
#endif

#ifndef	__uint32_t_defined
typedef	__uint32_t	uint32_t;
#define __uint32_t_defined 1
#endif

#ifndef	__int64_t_defined
typedef	__int64_t	int64_t;
#define __int64_t_defined 1
#endif

#ifndef	__uint64_t_defined
typedef	__uint64_t	uint64_t;
#define __uint64_t_defined 1
#endif

/* these interfere with the PalmSim build and we don't use them */
#if !defined(_MSC_VER)

#ifndef	intptr_t
typedef	__intptr_t	intptr_t;
#if defined(__GNUC__)
#define	intptr_t	__intptr_t
#endif
#endif

#ifndef	uintptr_t
typedef	__uintptr_t	uintptr_t;
//#define	uintptr_t	__uintptr_t
#endif

#endif /* !defined(_MSC_VER) */

#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/int_mwgwtypes.h>
#elif CPU_x86
#include <machine/x86/int_mwgwtypes.h>
#else
#error Building on unsupported platform
#endif

#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/int_limits.h>
#elif CPU_x86
#include <machine/x86/int_limits.h>
#else
#error Building on unsupported platform
#endif

#if !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS)
#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/int_const.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/int_const.h>
#else
#error Building on unsupported platform
#endif
#endif

#endif /* !_SYS_STDINT_H_ */
