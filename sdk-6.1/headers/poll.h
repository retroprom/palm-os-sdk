/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Poll.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _POLL_
#define _POLL_

#ifdef MI_H_ID_STRINGS
static char	poll_h_sccsid[] = "@(#)poll.h	43.1";
#endif

#define _MIPOLL_		/* Use by miverify.h for assertion checking */

/* Poll masks */
#define	POLLIN		0x001	/* A non-priority message is available */
#define	POLLPRI		0x002	/* A high priority message is available */
#define	POLLOUT		0x004	/* The stream is writable for non-priority msgs*/
#define	POLLERR		0x008	/* A error message has arrived */
#define	POLLHUP		0x010	/* A hangup has occurred */
#define	POLLNVAL	0x020	/* This fd is bogus */
#define	POLLRDNORM	0x040	/* A non-priority message is available */
#define	POLLRDBAND	0x080	/* A priority (band > 0) msg is available */
#define	POLLWRNORM	0x100	/* Same as POLLOUT */
#define	POLLWRBAND	0x200	/* A priority band exists and is writable */
#define	POLLMSG		0x400	/* A signal message has reached the front */
				/* of the queue */

/* array of streams to poll */
struct pollfd {
	int	fd;
	short	events;
	short	revents;
};

/* I_POLL structure for ioctl on non-5.3 systems */
struct strpoll {
	unsigned long	nfds;
	struct pollfd	* pollfdp;
	int		timeout;
};

#ifdef __cplusplus
extern "C" {
#endif

int poll(struct pollfd *fds, unsigned int nfds, int timeout);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif	/* _POLL_ */
