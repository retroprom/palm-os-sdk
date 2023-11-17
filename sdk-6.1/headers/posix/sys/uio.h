/*	$NetBSD: uio.h,v 1.24 2002/03/31 14:21:56 tron Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1993, 1994
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
 *	@(#)uio.h	8.5 (Berkeley) 2/22/94
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

#ifndef _SYS_UIO_H_
#define	_SYS_UIO_H_

#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/ansi.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/ansi.h>
#else
#error Building on unsupported platform
#endif
#include <sys/featuretest.h>

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

#ifdef	_BSD_SSIZE_T_
typedef	_BSD_SSIZE_T_	ssize_t;
#undef	_BSD_SSIZE_T_
#endif

struct iovec {
	void	*iov_base;	/* Base address. */
	size_t	 iov_len;	/* Length. */
};

#if !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)
#include <sys/ansi.h>

#ifndef	__off_t_defined
typedef	__off_t		off_t;	/* file offset */
#define	off_t		__off_t
#define __off_t_defined 1
#endif

enum	uio_rw { UIO_READ, UIO_WRITE };

/* Segment flag values. */
enum uio_seg {
	UIO_USERSPACE,		/* from user data space */
	UIO_SYSSPACE		/* from system space */
};

/*
 * Limits
 */
/* Deprecated: use IOV_MAX from <limits.h> instead. */
#define UIO_MAXIOV	1024		/* max 1K of iov's */
#endif /* !_POSIX_C_SOURCE && !_XOPEN_SOURCE */

#ifdef _KERNEL
#define UIO_SMALLIOV	8		/* 8 on stack, else malloc */
#endif

#ifndef	_KERNEL
#include <sys/cdefs.h>

__BEGIN_DECLS
#if !defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)
ssize_t preadv __P((int, const struct iovec *, int, off_t));
ssize_t pwritev __P((int, const struct iovec *, int, off_t));
#endif /* !_POSIX_C_SOURCE && !_XOPEN_SOURCE */

/* PalmOS uses size_t for readv/writev, not int */
ssize_t	readv __P((int, const struct iovec *, size_t));
ssize_t	writev __P((int, const struct iovec *, size_t));
__END_DECLS
#endif /* !_KERNEL */

#endif /* !_SYS_UIO_H_ */
