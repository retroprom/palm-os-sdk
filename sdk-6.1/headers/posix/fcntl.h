/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: fcntl.h
 *
 *****************************************************************************/

#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/types.h>  /* for mode_t */
#include <BuildDefaults.h>		/* for TARGET_HOST */

/* commands that can be passed to fcntl */
#define	F_DUPFD			0x0001
#define	F_GETFD			0x0002
#define	F_SETFD			0x0004
#define	F_GETFL			0x0008
#define	F_SETFL			0x0010
#define F_GETLK         0x0020
#define F_RDLCK         0x0040
#define F_SETLK         0x0080
#define F_SETLKW        0x0100
#define F_UNLCK         0x0200
#define F_WRLCK         0x0400

#if defined(__GNUC__)
#define	FD_CLOEXEC	1	/* Close on exec.  */
#else
#define FD_CLOEXEC      0x0800
#endif

/* flags for open() */
#if TARGET_HOST == TARGET_HOST_WIN32

#define O_RDONLY       0x0000  /* open for reading only */
#define O_WRONLY       0x0001  /* open for writing only */
#define O_RDWR         0x0002  /* open for reading and writing */
#define O_RWMASK       0x0003  /* Mask to get open mode */

#define O_CLOEXEC      0x0080  /* child process doesn't inherit file */
#define O_APPEND       0x0008  /* writes done at eof */
#define O_CREAT        0x0100  /* create and open file */
#define O_TRUNC        0x0200  /* open and truncate */
#define O_EXCL         0x0400  /* open only if file doesn't already exist */
#define O_TEXT         0x4000  /* file mode is text (translated) */
#define O_BINARY       0x8000  /* file mode is binary (untranslated) */

#elif TARGET_HOST == TARGET_HOST_PALMOS

#define O_RDONLY       0x0000  /* open for reading only */
#define O_WRONLY       0x0001  /* open for writing only */
#define O_RDWR         0x0002  /* open for reading and writing */
#define O_RWMASK       0x0003  /* Mask to get open mode */
#define O_ACCMODE		O_RWMASK

#define O_NDELAY       0x0004  /* System V non-blocking semantics */
#define O_NONBLOCK		O_NDELAY /* !!! BSD-ish name for the same thing? */

#define O_APPEND       0x0008

#define O_CREAT        0x0100  /* create and open file */
#define O_TRUNC        0x0200  /* open and truncate */
#define O_EXCL         0x0400  /* open only if file doesn't already exist */

/* note that IOS reserves values 0x0010 and above */

#else

#define O_RDONLY		0	/* read only */
#define O_WRONLY		1	/* write only */
#define O_RDWR			2	/* read and write */
#define O_RWMASK		3	/* Mask to get open mode */

#define O_CLOEXEC		0x0040	/* close fd on exec */
#define	O_NONBLOCK		0x0080	/* non blocking io */
#define	O_EXCL			0x0100	/* exclusive creat */
#define O_CREAT			0x0200	/* create and open file */
#define O_TRUNC			0x0400	/* open with truncation */
#define O_APPEND		0x0800	/* to end of file */
#define O_NOCTTY    	0x1000  /* currently unsupported */
#define	O_NOTRAVERSE	0x2000	/* do not traverse leaf link */
#define O_ACCMODE   	0x0003  /* currently unsupported */
#define O_TEXT			0x4000	/* CR-LF translation	*/
#define O_BINARY		0x8000	/* no translation	*/

#endif

/* #define O_DSYNC XXXdbg */
/* #define O_RSYNC XXXdbg */
/* #define O_SYNC  XXXdbg */

struct flock {
	off_t l_start;
	off_t l_len;
	pid_t l_pid;
	short l_type;
	short l_whence;
};


__BEGIN_DECLS
/*"creat" function is not supported by Palm OS Cobalt yet*/
extern int	creat(const char *path, mode_t mode);
extern int	fcntl(int fd, int op, ...);
extern int	open(const char *pathname, int oflags, ...);

__END_DECLS

#endif /* _FCNTL_H */
