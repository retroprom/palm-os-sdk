
#ifndef _SOCKET_
#define _SOCKET_

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1982, 1985, 1986, 1988, 1993, 1994
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
 *	@(#)socket.h	8.4 (Berkeley) 2/21/94
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

/*
 * Definitions related to sockets: types, address families, options.
 */

typedef unsigned int socklen_t;

/*
 * Types
 */
#define	SOCK_STREAM	1			/* stream socket */
#define	SOCK_DGRAM	2			/* datagram socket */
#define	SOCK_RAW	3			/* raw-protocol interface */
#define	SOCK_RDM	4			/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001			/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080			/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */
#define	SO_REUSEPORT	0x0200		/* allow local address & port reuse */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF	0x1001			/* send buffer size */
#define SO_RCVBUF	0x1002			/* receive buffer size */
#define SO_SNDLOWAT	0x1003			/* send low-water mark */
#define SO_RCVLOWAT	0x1004			/* receive low-water mark */
#define SO_SNDTIMEO	0x1005			/* send timeout */
#define SO_RCVTIMEO	0x1006			/* receive timeout */
#define	SO_ERROR	0x1007			/* get error status and clear */
#define	SO_TYPE		0x1008			/* get socket type */
#define	SO_PROTOTYPE	0x1009		/* get/set protocol type */
#define	SO_SND_COPYAVOID	0x100a	/* avoid copy on send */
#define	SO_RCV_COPYAVOID	0x100b	/* avoid copy on recv */

/* Option structure. */
struct opthdr {
	int32_t	level;
	int32_t	name;
	int32_t	len;
};
#define OPTLEN(opt)	((((opt) + sizeof(int32_t) - 1) \
			/ sizeof(int32_t)) * sizeof(int32_t))
#define	OPTVAL(opt)	((char *)(opt + 1))

/*
 * Structure used for manipulating linger option.
 */
struct	linger {
	int		l_onoff;		/* option on/off */
	int		l_linger;		/* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Address families.
 */
#define	AF_UNSPEC		0		/* unspecified */
#define	AF_LOCAL		1		/* local to host (pipes, portals) */
#define	AF_UNIX		AF_LOCAL	/* backward compatibility */
#define	AF_INET			2		/* internetwork: UDP, TCP, etc. */
#define	AF_IMPLINK		3		/* arpanet imp addresses */
#define	AF_PUP			4		/* pup protocols: e.g. BSP */
#define	AF_CHAOS		5		/* mit CHAOS protocols */
#define	AF_NS			6		/* XEROX NS protocols */
#define	AF_ISO			7		/* ISO protocols */
#define	AF_OSI			AF_ISO
#define	AF_ECMA			8		/* european computer manufacturers */
#define	AF_DATAKIT		9		/* datakit protocols */
#define	AF_CCITT		10		/* CCITT protocols, X.25 etc */
#define	AF_SNA			11		/* IBM SNA */
#define AF_DECnet		12		/* DECnet */
#define AF_DLI			13		/* DEC Direct data link interface */
#define AF_LAT			14		/* LAT */
#define	AF_HYLINK		15		/* NSC Hyperchannel */
#define	AF_APPLETALK	16		/* Apple Talk */
#define	AF_ROUTE		17		/* Internal Routing Protocol */
#define	AF_LINK			18		/* Link layer interface */
#define	pseudo_AF_XTP	19		/* eXpress Transfer Protocol (no AF) */
#define	AF_COIP			20		/* connection-oriented IP, aka ST II */
#define	AF_CNT			21		/* Computer Network Technology */
#define pseudo_AF_RTIP	22		/* Help Identify RTIP packets */
#define	AF_IPX			23		/* Novell Internet Protocol */
#define	AF_SIP			24		/* Simple Internet Protocol */
#define pseudo_AF_PIP	25		/* Help Identify PIP packets */
#define	AF_INET6		26		/* internetwork v6: UDP, TCP, etc. */
#define AF_IRDA			27		/* Infrared Data Association protocols */
#define AF_BTH			28		/* Bluetooth */

#define	AF_MAX			29

typedef	uint16_t sa_family_t;

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	sa_family_t	sa_family;		/* address family */
	char		sa_data[14];	/* address value */
};

#define	__ss_aligntype		uint64_t
#define	_SS_SIZE			128
#define	_SS_PAD1SIZE		(sizeof(__ss_aligntype) - sizeof(sa_family_t))
#define	_SS_PAD2SIZE		(_SS_SIZE - (sizeof(sa_family_t) \
							+ _SS_PAD1SIZE + sizeof(__ss_aligntype)))

/* Structure used store addresses socket addresses of any type. */
struct sockaddr_storage {
	sa_family_t		ss_family;
	char			__ss_pad1[_SS_PAD1SIZE];
	__ss_aligntype	__ss_align;
	char			__ss_pad2[_SS_PAD2SIZE];
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
	sa_family_t		sp_family;		/* address family */
	uint16_t		sp_protocol;		/* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_LOCAL	AF_LOCAL
#define	PF_UNIX		PF_LOCAL	/* backward compatibility */
#define	PF_INET		AF_INET
#define	PF_IMPLINK	AF_IMPLINK
#define	PF_PUP		AF_PUP
#define	PF_CHAOS	AF_CHAOS
#define	PF_NS		AF_NS
#define	PF_ISO		AF_ISO
#define	PF_OSI		AF_ISO
#define	PF_ECMA		AF_ECMA
#define	PF_DATAKIT	AF_DATAKIT
#define	PF_CCITT	AF_CCITT
#define	PF_SNA		AF_SNA
#define PF_DECnet	AF_DECnet
#define PF_DLI		AF_DLI
#define PF_LAT		AF_LAT
#define	PF_HYLINK	AF_HYLINK
#define	PF_APPLETALK	AF_APPLETALK
#define	PF_ROUTE	AF_ROUTE
#define	PF_LINK		AF_LINK
#define	PF_XTP		pseudo_AF_XTP	/* really just proto family, no AF */
#define	PF_COIP		AF_COIP
#define	PF_CNT		AF_CNT
#define	PF_SIP		AF_SIP
#define	PF_IPX		AF_IPX		/* same format as AF_NS */
#define PF_RTIP		pseudo_AF_FTIP	/* same format as AF_INET */
#define PF_PIP		pseudo_AF_PIP
#define	PF_INET6	AF_INET6
#define PF_IRDA		AF_IRDA
#define PF_BTH		AF_BTH

#define	PF_MAX		AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define	SOMAXCONN	5

/*
 * Message header for recvmsg and sendmsg calls.
 * Used value-result for recvmsg, value only for sendmsg.
 */
struct msghdr {
	char *			msg_name;		/* optional address */
	socklen_t		msg_namelen;	/* size of address */
	struct iovec *	msg_iov;		/* scatter/gather array */
	int				msg_iovlen;		/* # elements in msg_iov */
	char	*		msg_control;	/* ancillary data, see below */
	socklen_t		msg_controllen;	/* ancillary data buffer len */
	int				msg_flags;		/* flags on received message */
};

#define msg_accrights		msg_control
#define msg_accrightslen	msg_controllen

#define	MSG_OOB			0x1		/* process out-of-band data */
#define	MSG_PEEK		0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */
#define	MSG_EOR			0x8		/* data completes record */
#define	MSG_TRUNC		0x10		/* data discarded before delivery */
#define	MSG_CTRUNC		0x20		/* control data lost before delivery */
#define	MSG_WAITALL		0x40		/* wait for full request or error */
#define	MSG_DONTWAIT	0x80		/* this message should be nonblocking */

/*
 * Header for ancillary data objects in msg_control buffer.
 * Used for additional information with/about a datagram
 * not expressible by flags.  The format is a sequence
 * of message elements headed by cmsghdr structures.
 */
struct cmsghdr {
	socklen_t	cmsg_len;		/* data byte count, including hdr */
	int			cmsg_level;		/* originating protocol */
	int			cmsg_type;		/* protocol-specific type */
/* followed by	uint8_t cmsg_data[]; */
};

/* given pointer to struct cmsghdr, return pointer to data */
#define	CMSG_DATA(cmsg)		((uint8_t *)((cmsg) + 1))

/* given pointer to struct cmsghdr, return pointer to next cmsghdr */
#define	CMSG_NXTHDR(mhdr, cmsg)	\
		(((caddr_t)(cmsg) + (cmsg)->cmsg_len + sizeof(struct cmsghdr) > \
	    (mhdr)->msg_control + (mhdr)->msg_controllen) ? \
	    (struct cmsghdr *)NULL : \
	    (struct cmsghdr *)((caddr_t)(cmsg) + ALIGN((cmsg)->cmsg_len)))

#define	CMSG_FIRSTHDR(mhdr)	((struct cmsghdr *)(mhdr)->msg_control)

/* "Socket"-level control message types: */
#define	SCM_RIGHTS	0x01		/* access rights (array of int) */

int		accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
int		bind(int sock, const struct sockaddr *addr, socklen_t addrlen);
int		connect(int sock, const struct sockaddr *addr, socklen_t addrlen);
int		getpeername(int sock, struct sockaddr *addr, socklen_t *addrlen);
int		getsockname(int sock, struct sockaddr *addr, socklen_t *addrlen);
int		getsockopt(int sock, int level, int option, void *optval, socklen_t *optlen);
int		listen(int sock, int backlog);
ssize_t	recv(int sock, void *data, size_t datalen, int flags);
ssize_t	recvfrom(int sock, void *data, size_t datalen, int flags, struct sockaddr *addr, socklen_t *addrlen);
ssize_t	recvmsg(int   sd, struct msghdr * msg, int  flags);
ssize_t	send(int sock, const void *data, size_t datalen, int flags);
ssize_t	sendto(int sock, const void *data, size_t datalen, int flags, const struct sockaddr *addr, socklen_t addrlen);
ssize_t	sendmsg(int   sd, const struct msghdr * msg, int  flags);
int		setsockopt(int sock, int level, int option, const void *optval, socklen_t optlen);
int		shutdown(int sock, int direction);
int		socket(int family, int type, int proto);
int		socketpair(int domain, int type, int protocol, int sd[]);

/* shutdown() ways */
#define SHUT_RD		0
#define SHUT_WR		1
#define SHUT_RDWR	2

#ifdef __cplusplus
}
#endif

#endif /* _SOCKET_ */
