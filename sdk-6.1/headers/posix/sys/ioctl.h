/*	$NetBSD: ioctl.h,v 1.30 2001/06/11 01:50:57 wiz Exp $	*/

/*-
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
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
 *	@(#)ioctl.h	8.6 (Berkeley) 3/28/94
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
#ifndef	_SYS_IOCTL_H_
#define	_SYS_IOCTL_H_

#include <sys/ttycom.h>

/*
 * Pun for SunOS prior to 3.2.  SunOS 3.2 and later support TIOCGWINSZ
 * and TIOCSWINSZ (yes, even 3.2-3.5, the fact that it wasn't documented
 * nonwithstanding).
 */
struct ttysize {
	unsigned short	ts_lines;
	unsigned short	ts_cols;
	unsigned short	ts_xxx;
	unsigned short	ts_yyy;
};
#define	TIOCGSIZE	TIOCGWINSZ
#define	TIOCSSIZE	TIOCSWINSZ

#include <sys/ioccom.h>

#include <sys/dkio.h>
#include <sys/filio.h>
#include <sys/sockio.h>

/*
 * Passthrough ioctl commands. These are passed through to devices
 * as they are, it is expected that the device (an LKM, for example),
 * will know how to deal with them. One for each emulation, so that
 * no namespace clashes will occur between them, for devices that
 * may be dealing with specific ioctls for multiple emulations.
 */

struct ioctl_pt {
	unsigned long com;
	void *data;
};

#define PTIOCNETBSD	_IOW('Z', 0, struct ioctl_pt)
#define PTIOCSUNOS	_IOW('Z', 1, struct ioctl_pt)
#define PTIOCSVR4	_IOW('Z', 2, struct ioctl_pt)
#define PTIOCLINUX	_IOW('Z', 3, struct ioctl_pt)
#define PTIOCFREEBSD	_IOW('Z', 4, struct ioctl_pt)
#define PTIOCOSF1	_IOW('Z', 5, struct ioctl_pt)
#define PTIOCULTRIX	_IOW('Z', 6, struct ioctl_pt)
#define PTIOCWIN32	_IOW('Z', 7, struct ioctl_pt)

#ifndef _KERNEL

#include <sys/cdefs.h>

__BEGIN_DECLS
int	ioctl __P((int, unsigned long, ...));
__END_DECLS
#endif /* !_KERNEL */
#endif /* !_SYS_IOCTL_H_ */
