/*	$NetBSD: string.h,v 1.23 2001/03/22 07:37:04 kleink Exp $	*/

/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)string.h	8.1 (Berkeley) 6/2/93
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

#ifndef _STRING_H_
#define	_STRING_H_
#include <sys/cdefs.h>

#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/ansi.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/ansi.h>
#else
#error Building on unsupported platform
#endif

/* remap memcpy to our own */
#if CPU_TYPE == CPU_ARM
#define		memcpy		_PalmOS_memcpy
#endif

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

#include <sys/null.h>

#include <sys/featuretest.h>

__BEGIN_DECLS
void	*memchr __P((const void *, int, size_t));
int	 memcmp __P((const void *, const void *, size_t));
void	*memcpy __P((void * __restrict, const void * __restrict, size_t));
void	*memmove __P((void *, const void *, size_t));
void	*memset __P((void *, int, size_t));
char	*strcat __P((char * __restrict, const char * __restrict));
char	*strchr __P((const char *, int));
int	 strcmp __P((const char *, const char *));
int	 strcoll __P((const char *, const char *));
char	*strcpy __P((char * __restrict, const char * __restrict));
size_t	 strcspn __P((const char *, const char *));
__aconst char *strerror __P((int));
__aconst char *strerror_r __P((int, char *buf, size_t buflen));
size_t	 strlen __P((const char *));
char	*strncat __P((char * __restrict, const char * __restrict, size_t));
int	 strncmp __P((const char *, const char *, size_t));
char	*strncpy __P((char * __restrict, const char * __restrict, size_t));
char	*strpbrk __P((const char *, const char *));
char	*strrchr __P((const char *, int));
size_t	 strspn __P((const char *, const char *));
char	*strstr __P((const char *, const char *));
char	*strtok __P((char * __restrict, const char * __restrict));
#if (!defined(_ANSI_SOURCE) && !defined(_POSIX_C_SOURCE) && \
     !defined(_XOPEN_SOURCE)) || defined(_REENTRANT) || \
    (_POSIX_C_SOURCE - 0 >= 199506L) || (_XOPEN_SOURCE - 0 >= 500)
char	*strtok_r __P((char *, const char *, char **));
#endif /* !defined(_ANSI_SOURCE) || defined(_REENTRANT) || ... */
size_t	 strxfrm __P((char * __restrict, const char * __restrict, size_t));

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_C_SOURCE) || \
    defined(_XOPEN_SOURCE)
void	*memccpy __P((void *, const void *, int, size_t));
char	*strdup __P((const char *));
#endif /* !defined(_ANSI_SOURCE) && !defined(_POSIX_C_SOURCE) */

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_C_SOURCE) && \
    !defined(_XOPEN_SOURCE)
#include <strings.h>		/* for backwards-compatibilty */
size_t	 strlcat __P((char *, const char *, size_t));
size_t	 strlcpy __P((char *, const char *, size_t));
char	*strsep __P((char **, const char *));
#endif /* !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE) && ... */
__END_DECLS

#endif /* !defined(_STRING_H_) */
