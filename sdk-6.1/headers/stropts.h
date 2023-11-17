/** Copyright (c) 1991-1999  Mentat Inc.
 ** stropts.h 43.1, last change 05/18/01
 **/

#ifndef _MPS_STROPTS_
#define _MPS_STROPTS_

#ifdef MI_H_ID_STRINGS
static	char	stropts_h_sccsid[] = "@(#)stropts.h	43.1";
#endif

/*
 * Important: MIOC_CMD and MIOC_xxx values must be compatible with values
 * defined in miioccom.h and other (port-specific) ioctl() command codes.
 */

#ifndef MIOC_CMD
#include <miioccom.h>
#endif

/* Return the number of bytes in 1st msg */
#define	I_NREAD		MIOC_CMD(MIOC_STREAMIO,1)

/* Push module just below Stream head */
#define	I_PUSH		MIOC_CMD(MIOC_STREAMIO,2)

/* Pop module below Stream head */
#define	I_POP		MIOC_CMD(MIOC_STREAMIO,3)

/* Retrieve name of first stream module */
#define	I_LOOK		MIOC_CMD(MIOC_STREAMIO,4)

/* Flush all input and/or output queues */
#define	I_FLUSH		MIOC_CMD(MIOC_STREAMIO,5)

/* Set the read mode */
#define	I_SRDOPT	MIOC_CMD(MIOC_STREAMIO,6)

/* Get the current read mode */
#define	I_GRDOPT	MIOC_CMD(MIOC_STREAMIO,7)

/* Create an internal ioctl message */
#define	I_STR		MIOC_CMD(MIOC_STREAMIO,8)

/* Request SIGPOLL signal on events */
#define	I_SETSIG	MIOC_CMD(MIOC_STREAMIO,9)

/* Query the registered events */
#define	I_GETSIG	MIOC_CMD(MIOC_STREAMIO,10)

/* Check for module in stream */
#define	I_FIND		MIOC_CMD(MIOC_STREAMIO,11)

/* connect stream under mux fd */
#define	I_LINK		MIOC_CMD(MIOC_STREAMIO,12)

/* disconnect two streams */
#define	I_UNLINK	MIOC_CMD(MIOC_STREAMIO,13)

/* peek at data on read queue */
#define	I_PEEK		MIOC_CMD(MIOC_STREAMIO,15)

/* create a message and send downstream */
#define	I_FDINSERT	MIOC_CMD(MIOC_STREAMIO,16)

/* send an fd to a connected pipe stream */
#define	I_SENDFD	MIOC_CMD(MIOC_STREAMIO,17)

/* retrieve a file descriptor */
#define	I_RECVFD	MIOC_CMD(MIOC_STREAMIO,18)

/* flush a particular input and/or output band */
#define	I_FLUSHBAND	MIOC_CMD(MIOC_STREAMIO,19)

/* set the write mode */
#define	I_SWROPT	MIOC_CMD(MIOC_STREAMIO,20)

/* get the current write mode */
#define	I_GWROPT	MIOC_CMD(MIOC_STREAMIO,21)

/* get a list of all modules on a stream */
#define	I_LIST		MIOC_CMD(MIOC_STREAMIO,22)

/* check to see if the next message is "marked" */
#define	I_ATMARK	MIOC_CMD(MIOC_STREAMIO,23)

/* check for a message of a particular band */
#define	I_CKBAND	MIOC_CMD(MIOC_STREAMIO,24)

/* get the band of the next message to be read */
#define	I_GETBAND	MIOC_CMD(MIOC_STREAMIO,25)

/* check to see if a message may be passed on a stream */
#define	I_CANPUT	MIOC_CMD(MIOC_STREAMIO,26)

/* set the close timeout wait */
#define	I_SETCLTIME	MIOC_CMD(MIOC_STREAMIO,27)

/* get the current close timeout wait */
#define	I_GETCLTIME	MIOC_CMD(MIOC_STREAMIO,28)

/* permanently connect a stream under a mux */
#define	I_PLINK		MIOC_CMD(MIOC_STREAMIO,29)

/* disconnect a permanent link */
#define	I_PUNLINK	MIOC_CMD(MIOC_STREAMIO,30)

/* ioctl values needed on non-SYS V systems */

/* getmsg() system call */
#define	I_GETMSG	MIOC_CMD(MIOC_STREAMIO,40)

/* putmsg() system call */
#define	I_PUTMSG	MIOC_CMD(MIOC_STREAMIO,41)

/* poll() system call */
#define	I_POLL		MIOC_CMD(MIOC_STREAMIO,42)

/* set blocking status */
#define	I_SETDELAY	MIOC_CMD(MIOC_STREAMIO,43)

/* get blocking status */
#define	I_GETDELAY	MIOC_CMD(MIOC_STREAMIO,44)

/* sacrifice for the greater good */
#define	I_RUN_QUEUES	MIOC_CMD(MIOC_STREAMIO,45)

/* getpmsg() system call */
#define	I_GETPMSG	MIOC_CMD(MIOC_STREAMIO,46)

/* putpmsg() system call */
#define	I_PUTPMSG	MIOC_CMD(MIOC_STREAMIO,47)

/* for pipe library call */
#define	I_PIPE		MIOC_CMD(MIOC_STREAMIO,49)

/* for fifo library call */
#define	I_FIFO		MIOC_CMD(MIOC_STREAMIO,51)

#ifdef AOSVS
/* AOS/VS signal redirection */
#define I_SIGTASK	MIOC_CMD(MIOC_STREAMIO,57)
#endif

#ifdef NETWARE
/* get the stream handle for a CLIB fd */
#define	I_GETSTREAMID	MIOC_CMD(MIOC_STREAMIO,57)
#endif

#ifdef	VMS
/* permanently map a user region */
#define	I_MAPREGION	MIOC_CMD(MIOC_STREAMIO,57)

/* unmap previously set region */
#define	I_UNMAP		MIOC_CMD(MIOC_STREAMIO,58)
#endif

/*
 * MPS heap ioctls.  Privilege is required for all commands except
 * I_HEAP_READ_REPORT.
 */
#define I_HEAP_CHECK_ON		MIOC_CMD(MIOC_STREAMIO, 60)
#define I_HEAP_CHECK_OFF	MIOC_CMD(MIOC_STREAMIO, 61)
#define I_HEAP_MARK		MIOC_CMD(MIOC_STREAMIO, 62)
#define I_HEAP_LEAK_REPORT	MIOC_CMD(MIOC_STREAMIO, 63)
#define I_HEAP_RECLAIM		MIOC_CMD(MIOC_STREAMIO, 64)
#define I_HEAP_REPORT		MIOC_CMD(MIOC_STREAMIO, 65)
#define I_HEAP_READ_REPORT	MIOC_CMD(MIOC_STREAMIO, 66)

/* Get MPS statistics. */
#define	I_MPS_GET_STAT		MIOC_CMD(MIOC_STREAMIO, 67)

/* priority message request on putmsg() or strpeek */
#define	RS_HIPRI	0x1

/* flags for getpmsg and putpmsg */
#define	MSG_HIPRI	RS_HIPRI
#define	MSG_BAND	0x2	/* Retrieve a message from a particular band */
#define	MSG_ANY		0x4	/* Retrieve a message from any band */

/* return values from getmsg(), 0 indicates all ok */
#define	MORECTL		0x1	/* more control info available */
#define	MOREDATA	0x2	/* more data available */

#ifndef FMNAMESZ
#define FMNAMESZ	31	/* maximum length of a module or device name */
#endif

/* Infinite poll wait time */
#define	INFTIM		-1

/* flush requests */
#define	FLUSHR		0x1		/* Flush the read queue */
#define	FLUSHW		0x2		/* Flush the write queue */
#define	FLUSHRW		(FLUSHW|FLUSHR)	/* Flush both */
#define	FLUSHBAND	0x40		/* Flush a particular band */

/* I_FLUSHBAND */
struct bandinfo {
	unsigned char	bi_pri;	/* Band to flush */
	unsigned char	pad[3];	/* Keep the compiler happy -awk */
	int		bi_flag;/* One of the above flush requests */
};

/* flags for I_ATMARK */
#define	ANYMARK		0x1	/* Check if message is marked */
#define	LASTMARK	0x2	/* Check if this is the only message marked */

/* signal event masks */
#define	S_INPUT		0x1	/* A non-M_PCPROTO message has arrived */
#define	S_HIPRI		0x2	/* A priority (M_PCPROTO) message is available */
#define	S_OUTPUT	0x4	/* The write queue is no longer full */
#define	S_MSG		0x8	/* A signal message has reached the front of */
				/* read queue */
#define	S_RDNORM	0x10	/* A non-priority message is available */
#define	S_RDBAND	0x20	/* A banded messsage is available */
#define	S_WRNORM	0x40	/* Same as S_OUTPUT */
#define	S_WRBAND	0x80	/* A priority band exists and is writable */
#define	S_ERROR		0x100	/* Error message has arrived */
#define	S_HANGUP	0x200	/* Hangup message has arrived */
#define	S_BANDURG	0x400	/* Use SIGURG instead of SIGPOLL on S_RDBAND */
				/* signals */

/* read mode bits for I_S|GRDOPT; choose one of the following */
#define	RNORM		0x01	/* byte-stream mode, default */
#define	RMSGD		0x02	/* message-discard mode */
#define	RMSGN		0x04	/* message-nondiscard mode */
#define	RFILL		0x08	/* fill read buffer mode (PSE private) */

/* More read modes, these are bitwise or'ed with the modes above */
#define	RPROTNORM	0x10	/* Normal handling of M_PROTO/M_PCPROTO msgs, */
				/* default */
#define	RPROTDIS	0x20	/* Discard M_PROTO/M_PCPROTO message blocks */
#define	RPROTDAT	0x40	/* Convert M_PROTO/M_PCPROTO message blocks */
				/* into M_DATA */

/* write modes for I_S|GWROPT */
#define	SNDZERO		0x1	/* Send a zero-length message downstream on */
				/* a write of zero bytes */

#define	MUXID_ALL	-1	/* Unlink all lower streams for I_UNLINK and */
				/* I_PUNLINK */

struct	strbuf {
	int	maxlen;		/* max buffer length */
	int	len;		/* length of data */
	char	* buf;		/* pointer to buffer */
};

/* structure of ioctl data on I_FDINSERT */
struct	strfdinsert {
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	long		flags;	/* type of message, 0 or RS_HIPRI */
	int		fildes;	/* fd of other stream */
	int		offset;	/* where to put other stream read qp */
};

/* I_LIST structures */
struct str_list {
	int	sl_nmods;	/* number of modules in sl_modlist array */
	struct str_mlist * sl_modlist;
};

struct str_mlist {
	char	l_name[FMNAMESZ + 1];
};

/* I_PEEK structure */
struct	strpeek {
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	long		flags;	/* if RS_HIPRI, get priority messages only */
};

struct	strpmsg {		/* structure for getpmsg and putpmsg */
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	int		band;
	long		flags;
};

/* structure of ioctl data on I_RECVFD */
struct	strrecvfd {
	int		fd;	/* new file descriptor */
	unsigned short	uid;	/* user id of sending stream */
	unsigned short	gid;
	char		fill[8];
};

/* structure of ioctl data on I_STR */
struct	strioctl {
	int	ic_cmd;		/* downstream command */
	int	ic_timout;	/* ACK/NAK timeout */
	int	ic_len;		/* length of data arg */
	char	* ic_dp;	/* ptr to data arg */
};

#ifndef KERNEL

#ifdef __cplusplus
extern "C" {
#endif

extern	int	getmsg(int fd, struct strbuf * ctlbuf
			, struct strbuf * databuf, int * flagsp);
extern	int	putmsg(int fd, const struct strbuf * ctlbuf
			, const struct strbuf * databuf, int flags);
extern	int	getpmsg(int fd, struct strbuf * ctlbuf
			, struct strbuf * databuf, int * bandp, int * flagsp);
extern	int	putpmsg(int fd, const struct strbuf * ctlbuf
			, const struct strbuf * databuf, int band, int flags);

extern	int	fattach(int fd, const char *name);
extern	int	fdetach(const char *name);

#ifdef __cplusplus
};
#endif

#endif /* KERNEL */

#endif /* ifdef _STROPTS_ */

