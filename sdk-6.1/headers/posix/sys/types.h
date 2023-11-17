/*	$NetBSD: types.h,v 1.51 2002/03/09 23:57:25 chs Exp $	*/

/*-
 * Copyright (c) 1982, 1986, 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)types.h	8.4 (Berkeley) 1/21/94
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

#ifndef _SYS_TYPES_H_
#define	_SYS_TYPES_H_

/* Machine type dependent parameters. */
#include <BuildDefaults.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/types.h>
#include <machine/arm/ansi.h>
#include <machine/arm/int_types.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/types.h>
#include <machine/x86/ansi.h>
#include <machine/x86/int_types.h>
#else
#error Building on unsupported platform
#endif

#include <sys/ansi.h>

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

typedef	uint8_t		u_int8_t;
typedef	uint16_t	u_int16_t;
typedef	uint32_t	u_int32_t;
typedef	uint64_t	u_int64_t;

#include <sys/endian.h>

#if !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

typedef unsigned char	unchar;		/* Sys V compatibility */
typedef	unsigned short	ushort;		/* Sys V compatibility */
typedef	unsigned int	uint;		/* Sys V compatibility */
typedef unsigned long	ulong;		/* Sys V compatibility */

typedef	u_long		cpuid_t;
#endif

typedef	u_int64_t	u_quad_t;	/* quads */
typedef	int64_t		quad_t;
typedef	quad_t *	qaddr_t;

/*
 * The types longlong_t and u_longlong_t exist for use with the
 * Sun-derived XDR routines involving these types, and their usage
 * in other contexts is discouraged.  Further note that these types
 * may not be equivalent to "long long" and "unsigned long long",
 * they are only guaranteed to be signed and unsigned 64-bit types
 * respectively.  Portable programs that need 64-bit types should use
 * the C99 types int64_t and uint64_t instead.
 */

typedef	quad_t		longlong_t;	/* for XDR */
typedef	u_quad_t	u_longlong_t;	/* for XDR */

typedef	int64_t		blkcnt_t;	/* fs block count */
typedef	u_int32_t	blksize_t;	/* fs optimal block size */

#ifndef	caddr_t
typedef	__caddr_t	caddr_t;	/* core address */
//#define	caddr_t		__caddr_t
#endif

typedef	int32_t		daddr_t;	/* disk address */
typedef	u_int32_t	fixpt_t;	/* fixed point number */

/* make sure this agrees with the Windows version if we're building PalmSim */
#if defined(_MSC_VER)
typedef	unsigned int	dev_t;
#else
typedef	u_int32_t	dev_t;		/* device number */
#endif

typedef u_int16_t	major_t;	/* major device number */
typedef u_int16_t	minor_t;	/* minor device number */

#ifndef	gid_t
typedef	__gid_t		gid_t;		/* group id */
//#define	gid_t		__gid_t
#endif

typedef	u_int32_t	id_t;		/* group id, process id or user id */
#ifndef _INO_T_DEFINED
#if 0
typedef	u_int32_t	ino_t;		/* inode number */
#else
typedef unsigned short ino_t;	/* match MSVC 7 */
#endif
#define _INO_T_DEFINED
#endif
typedef	long		key_t;		/* IPC key (for Sys V IPC) */

#ifndef	mode_t
typedef	__mode_t	mode_t;		/* permissions */
//#define	mode_t		__mode_t
#endif

typedef	u_int32_t	nlink_t;	/* link count */

/* change off_t to match MSVC 7 when building code that interacts with Windows */
#if !defined(NTSIMWIN32GLUE_WINSOCK)
#ifndef	__off_t_defined
typedef	__off_t		off_t;		/* file offset */
#define	off_t		__off_t
#define __off_t_defined 1
#endif
#else	/* #if !defined(NTSIMWIN32GLUE_WINSOCK) */
#ifndef _OFF_T_DEFINED
typedef long off_t;
#define _OFF_T_DEFINED
#endif
#endif	/* #if !defined(NTSIMWIN32GLUE_WINSOCK) */

#ifndef	pid_t
typedef	__pid_t		pid_t;		/* process id */
//#define	pid_t		__pid_t
#endif

typedef quad_t		rlim_t;		/* resource limit */
typedef	int32_t		segsz_t;	/* segment size */
typedef	int32_t		swblk_t;	/* swap offset */

#ifndef	uid_t
typedef	__uid_t		uid_t;		/* user id */
//#define	uid_t		__uid_t
#endif

typedef	int32_t		dtime_t;	/* on-disk time_t */

#if defined(_KERNEL) || defined(_LIBC)
/*
 * semctl(2)'s argument structure.  This is here for the benefit of
 * <sys/syscallargs.h>.  It is not in the user's namespace in SUSv2.
 * The SUSv2 semctl(2) takes variable arguments.
 */
union __semun {
	int		val;		/* value for SETVAL */
	struct semid_ds	*buf;		/* buffer for IPC_STAT & IPC_SET */
	unsigned short	*array;		/* array for GETALL & SETALL */
};
#endif /* _KERNEL || _LIBC */

/*
 * These belong in unistd.h, but are placed here too to ensure that
 * long arguments will be promoted to off_t if the program fails to
 * include that header or explicitly cast them to off_t.
 */
#if !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
#ifndef __OFF_T_SYSCALLS_DECLARED
#define __OFF_T_SYSCALLS_DECLARED
#ifndef _KERNEL
#include <sys/cdefs.h>
__BEGIN_DECLS

/* MSVC headers require a certain linkage spec for lseek(), so PalmSim builds need it */

#if defined(_MSC_VER)
#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* _CRTIMP */
#else /* _MSC_VER */
#define _CRTIMP
#endif /* _MSC_VER */

_CRTIMP off_t	 lseek __P((int, off_t, int));
int	 ftruncate __P((int, off_t));
int	 truncate __P((const char *, off_t));
__END_DECLS
#endif /* !_KERNEL */
#endif /* __OFF_T_SYSCALLS_DECLARED */
#endif /* !defined(_POSIX_SOURCE) ... */

#if !defined(__PALMOS_KERNEL__)
#if !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
/* Major, minor numbers, dev_t's. */
#define	major(x)	((int32_t)((((x) & 0x000fff00) >>  8)))
#define	minor(x)	((int32_t)((((x) & 0xfff00000) >> 12) | \
				   (((x) & 0x000000ff) >>  0)))
#define	makedev(x,y)	((dev_t)((((x) <<  8) & 0x000fff00) | \
				 (((y) << 12) & 0xfff00000) | \
				 (((y) <<  0) & 0x000000ff)))
#endif
#endif /* !defined(__PALMOS_KERNEL__) */

#ifdef	_BSD_CLOCK_T_
typedef	_BSD_CLOCK_T_		clock_t;
#undef	_BSD_CLOCK_T_
#endif

#ifdef	_BSD_SIZE_T_
typedef	_BSD_SIZE_T_		size_t;
#define _SIZE_T
#undef	_BSD_SIZE_T_
#endif

#ifdef	_BSD_SSIZE_T_
typedef	_BSD_SSIZE_T_		ssize_t;
#undef	_BSD_SSIZE_T_
#endif

#ifdef	_BSD_TIME_T_
typedef	_BSD_TIME_T_		time_t;
#undef	_BSD_TIME_T_
#endif

#ifdef	_BSD_CLOCKID_T_
typedef	_BSD_CLOCKID_T_		clockid_t;
#undef	_BSD_CLOCKID_T_
#endif

#ifdef	_BSD_TIMER_T_
typedef	_BSD_TIMER_T_		timer_t;
#undef	_BSD_TIMER_T_
#endif

#ifdef	_BSD_SUSECONDS_T_
typedef	_BSD_SUSECONDS_T_	suseconds_t;
#undef	_BSD_SUSECONDS_T_
#endif

#ifdef	_BSD_USECONDS_T_
typedef	_BSD_USECONDS_T_	useconds_t;
#undef	_BSD_USECONDS_T_
#endif

#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) || \
    (defined(_XOPEN_SOURCE) && defined(_XOPEN_SOURCE_EXTENDED)) || \
    (_XOPEN_SOURCE - 0) >= 500

/*
 * Implementation dependent defines, hidden from user space. X/Open does not
 * specify them.
 */
#define	__NBBY	8		/* number of bits in a byte */
typedef int32_t	__fd_mask;
#define __NFDBITS	(sizeof(__fd_mask) * __NBBY)	/* bits per mask */

#ifndef howmany
#define	__howmany(x, y)	(((x) + ((y) - 1)) / (y))
#else
#define __howmany(x, y) howmany(x, y)
#endif

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

/* the PalmSim build blows up if we redefine these in WinSock-using code */
#if defined(NTSIMWIN32GLUE_WINSOCK)

/* Do not declare fd_set etc, because we'll get those from <WinSock2.h> */

#else /* ARM device */

typedef	struct fd_set {
	__fd_mask	fds_bits[__howmany(FD_SETSIZE, __NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	\
    ((p)->fds_bits[(n)/__NFDBITS] |= (1 << ((n) % __NFDBITS)))
#define	FD_CLR(n, p)	\
    ((p)->fds_bits[(n)/__NFDBITS] &= ~(1 << ((n) % __NFDBITS)))
#define	FD_ISSET(n, p)	\
    ((p)->fds_bits[(n)/__NFDBITS] & (1 << ((n) % __NFDBITS)))
#define	FD_ZERO(p)	(void)memset((p), 0, sizeof(*(p)))

#endif	/* _PALMOS_NEED_WINDOWS_FDS */

/*
 * Expose our internals if we are not required to hide them.
 */
#ifndef _XOPEN_SOURCE

#define NBBY __NBBY
#define fd_mask __fd_mask
#define NFDBITS __NFDBITS
#ifndef howmany
#define howmany(a, b) __howmany(a, b)
#endif

#define	FD_COPY(f, t)	(void)memcpy((t), (f), sizeof(*(f)))

#endif

#if defined(__STDC__) && defined(_KERNEL)
/*
 * Forward structure declarations for function prototypes.  We include the
 * common structures that cross subsystem boundaries here; others are mostly
 * used in the same place that the structure is defined.
 */
struct	proc;
struct	pgrp;
struct	ucred;
struct	rusage;
struct	file;
struct	buf;
struct	tty;
struct	uio;
#endif

#endif /* !defined(_POSIX_SOURCE) ... */

/*-------------------------------------------------------------*/
/* PalmOS types and definitions */
/*-------------------------------------------------------------*/

/* Descriptive formats */
typedef int32_t					status_t;
typedef int64_t					nsecs_t;
typedef uint32_t				type_code;
typedef uint32_t				perform_code;

/* IOS */
typedef int		FDCELL;
typedef void*		PCELL;
typedef int32_t	pl_t;

#endif /* !_SYS_TYPES_H_ */
