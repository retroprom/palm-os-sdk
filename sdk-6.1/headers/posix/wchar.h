/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: wchar.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

/*	$NetBSD: wchar.h,v 1.12 2002/03/14 21:22:28 yamt Exp $	*/

/*-
 * Copyright (c)1999 Citrus Project,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1999, 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Julian Coleman.
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

#ifndef _WCHAR_H_
#define _WCHAR_H_

#include <sys/cdefs.h>
#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/ansi.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/ansi.h>
#else
#error Building on unsupported platform
#endif
#include <sys/null.h>

#include <stdio.h> /* for FILE* */

/* Windows wchar_t demands are odd, so we do some tweaking for PalmSim */
#if defined(_MSC_VER) && (_MSC_VER>= 600)
typedef unsigned short wchar_t;
#elif defined(_PACC_VER) && defined(__cplusplus)
/* wchar_t is an intrinsic type in our C++ compiler */
#elif defined(__GNUC__) && defined(__cplusplus)
/* same for GCC */
#else
#ifdef	_BSD_WCHAR_T_
typedef	_BSD_WCHAR_T_	wchar_t;
#undef	_BSD_WCHAR_T_
#endif
#endif

#ifdef	_BSD_MBSTATE_T_
typedef	_BSD_MBSTATE_T_	mbstate_t;
#undef	_BSD_MBSTATE_T_
#endif

#ifdef	_BSD_WINT_T_
typedef	_BSD_WINT_T_	wint_t;
#undef	_BSD_WINT_T_
#endif

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

#ifndef WEOF
#define	WEOF 	((wint_t)-1)
#endif

#define getwc(f) fgetwc(f)
#define getwchar() getwc(stdin)
#define putwc(wc, f) fputwc((wc), (f))
#define putwchar(wc) putwc((wc), stdout)

__BEGIN_DECLS
size_t	mbrlen __P((const char * __restrict, size_t, mbstate_t * __restrict));
size_t	mbrtowc __P((wchar_t * __restrict, const char * __restrict, size_t,
	    mbstate_t * __restrict));
int	mbsinit __P((const mbstate_t *));
size_t	mbsrtowcs __P((wchar_t * __restrict, const char ** __restrict, size_t,
	    mbstate_t * __restrict));
size_t	wcrtomb __P((char * __restrict, wchar_t, mbstate_t * __restrict));
wchar_t	*wcscat __P((wchar_t * __restrict, const wchar_t * __restrict));
wchar_t	*wcschr __P((const wchar_t *, wchar_t));
int	wcscmp __P((const wchar_t *, const wchar_t *));
wchar_t	*wcscpy __P((wchar_t * __restrict, const wchar_t * __restrict));
size_t	wcscspn __P((const wchar_t *, const wchar_t *));
size_t	wcslen __P((const wchar_t *));
wchar_t	*wcsncat __P((wchar_t * __restrict, const wchar_t * __restrict,
	    size_t));
int	wcsncmp __P((const wchar_t *, const wchar_t *, size_t));
wchar_t	*wcsncpy __P((wchar_t * __restrict , const wchar_t * __restrict,
	    size_t));
wchar_t	*wcspbrk __P((const wchar_t *, const wchar_t *));
wchar_t	*wcsrchr __P((const wchar_t *, wchar_t));
size_t	wcsrtombs __P((char * __restrict, const wchar_t ** __restrict, size_t,
	    mbstate_t * __restrict));
size_t	wcsspn __P((const wchar_t *, const wchar_t *));
wchar_t	*wcsstr __P((const wchar_t *, const wchar_t *));
wchar_t	*wmemchr __P((const wchar_t *, wchar_t, size_t));
int	wmemcmp __P((const wchar_t *, const wchar_t *, size_t));
wchar_t	*wmemcpy __P((wchar_t * __restrict, const wchar_t * __restrict,
	    size_t));
wchar_t	*wmemmove __P((wchar_t *, const wchar_t *, size_t));
wchar_t	*wmemset __P((wchar_t *, wchar_t, size_t));

size_t	wcslcat __P((wchar_t *, const wchar_t *, size_t));
size_t	wcslcpy __P((wchar_t *, const wchar_t *, size_t));
int	wcswidth __P((const wchar_t *, size_t));
int	wcwidth __P((wchar_t));

unsigned long int wcstoul __P((const wchar_t * __restrict, wchar_t ** __restrict,
		int base));
long int wcstol __P((const wchar_t * __restrict, wchar_t ** __restrict, int base));
double wcstod __P((const wchar_t * __restrict, wchar_t ** __restrict));

wint_t ungetwc __P((wint_t, FILE *));
wint_t fgetwc __P((FILE *));
wint_t getwc __P((FILE *));
wint_t getwchar __P((void));
wint_t fputwc __P((wchar_t, FILE *));
wint_t putwc __P((wchar_t, FILE *));
wint_t putwchar __P((wchar_t));

int fwide __P((FILE *, int));
__END_DECLS

#endif /* !_WCHAR_H_ */
