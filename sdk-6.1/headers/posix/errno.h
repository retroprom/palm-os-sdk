/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: errno.h
 *
 *****************************************************************************/

#ifndef _ERRNO_H_
#define _ERRNO_H_

#include <stdint.h>

#if !defined(SINGLE_THREADED_ERRNO)

__BEGIN_DECLS
extern int *_errnop(void);
extern int32_t *_ioserrnop(void);
__END_DECLS

#define	errno (*(_errnop()))
#define	__set_errno(val) (*_errnop ()) = (val)

#define ioserrno (*(_ioserrnop()))

#else
extern int errno;
#endif

//
// WARNING - do not change any of these values without adjusting the the
//  errMap array in IOSStreamsErrors.c

/* General errors */
#define	EACCES		1
#define	EBADF		2
#define	EBADMSG		3
#define	EINTR		4
#define	ECANCEL		5
#define	EEXIST		6
#define	ENXIO		7
#define	ENOENT		8
#define	ENOMEM		9
#define	EINPROGRESS	10
#define	EPIPE		11
#define	ERANGE		34
#define	ENOSR		13
#define	EFAULT		14
#define	ETIME		15
#define	ETIMEDOUT	ETIME
#define	ENOSTR		17
#define	EPERM		18
#define	ENOEXEC		19
#define	EFBIG		20
#define	EPROTO		21
#define	EINVAL		22
#define	EMSGSIZE	23
#define	EAGAIN		24
#define	EALREADY	25

#define ENODEV		26	

#define EIO			27	
#define ENODATA		28	
#define	ESRCH		29
#define E2BIG		30
#define ENOSPC		31

#define	EADDRINUSE	32
#define	EADDRNOTAVAIL	33
#define	EAFNOSUPPORT	12
#define	EBADADDR	35
#define	EBUSY		36
#define	ECONNREFUSED	37
#define	ECONNRESET	38
#define	EHOSTUNREACH	39
#define	EISCONN		40
#define	ENETUNREACH	41
#define	ENOBUFS		42
#define	ENOPROTOOPT	43
#define	ENOTCONN	44
#define	ENOTSUP		45
#define	EOPNOTSUPP	46
#define	EPROTONOSUPPORT	47
#define	ESOCKTNOSUPPORT	48
#define EPROTOTYPE ESOCKTNOSUPPORT
#define	EWOULDBLOCK	EAGAIN
#define ENOLINK		49
#define ENOMSG		50
#define ENOTTY		51
#define ENOTSOCK	52
#define EPFNOSUPPORT 53

/* BSD errors added for new libc support */
#define	EFTYPE		54	/* operation on wrong type of file */
#define	EILSEQ		55	/* illegal byte sequence (wide char strings &c) */
#define	ENFILE		56		/* no available file descriptors on system */
#define	EMFILE		57	/* no available file descriptors in process */
#define	ESPIPE		58	/* attempt to seek() a pipe or FIFO */
#define	ENOTDIR	59	/* not a valid directory -- !!! mktemp() no longer allowed to fail? */

//#if TARGET_PLATFORM	!= TARGET_PLATFORM_PALMSIM_WIN32
/* This is defined via Windows headers in the PalmSim build */
#define EDOM		60  /* C99 <errno.h> requirement */
//#endif

#endif /* _ERRNO_H_ */
