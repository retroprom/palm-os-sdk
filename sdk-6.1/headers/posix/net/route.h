/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: route.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ROUTE_
#define	_ROUTE_


#include <sys/types.h>
#include <sys/socket.h>

#ifdef MI_H_ID_STRINGS
static	char	route_h_sccsid[] = "@(#)route.h	43.3";
#endif

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1980, 1986, 1993
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
 *	@(#)route.h	8.3 (Berkeley) 4/19/94
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
 * Kernel resident routing tables.
 *
 * The routing tables are initialized when interface addresses
 * are set by making entries for all directly connected interfaces.
 */

/*
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route {
	struct	rtentry *ro_rt;
	struct	sockaddr ro_dst;
};

/*
 * These numbers are used by reliable protocols for determining
 * retransmission behavior and are included in the routing structure.
 */
struct rt_metrics {
	uint32_t rmx_locks;	/* Kernel must leave these values alone */
	uint32_t rmx_mtu;	/* MTU for this path */
	uint32_t rmx_hopcount;	/* max hops expected */
	uint32_t rmx_expire;	/* lifetime for route, e.g. redirect */
	uint32_t rmx_recvpipe;	/* inbound delay-bandwith product */
	uint32_t rmx_sendpipe;	/* outbound delay-bandwith product */
	uint32_t rmx_ssthresh;	/* outbound gateway buffer limit */
	uint32_t rmx_rtt;	/* estimated round trip time */
	uint32_t rmx_rttvar;	/* estimated rtt variance */
	uint32_t rmx_pksent;	/* packets sent using this route */
};

/*
 * Structure used for SIOC[ADD|DEL]RT ioctls.
 */
struct rtentry {
	uint32_t rt_hash;		/* to speed lookups */
	struct	sockaddr rt_dst;	/* key */
	struct	sockaddr rt_gateway;	/* value */
	int16_t	rt_flags;		/* up/down?, host/net */
	int16_t	rt_refcnt;		/* # held references */
	uint32_t rt_use;		/* raw # packets forwarded */
	struct	ifnet *rt_ifp;		/* the answer: interface to use */
	struct	sockaddr rt_subnetmask;	/* subnetmask */
	uint32_t rt_pmtu;		/* path mtu value */
};

/*
 * Structure used for SIOC[ADD|DEL]RTL ioctls.
 */
struct rtlentry {
	uint32_t rtl_hash;		/* to speed lookups */
	struct	sockaddr_storage rtl_dst; /* key */
	struct	sockaddr_storage rtl_gateway; /* value */
	int16_t	rtl_flags;		/* up/down?, host/net */
	int16_t	rtl_refcnt;		/* # held references */
	uint32_t rtl_use;		/* raw # packets forwarded */
	struct	ifnet *rtl_ifp;		/* the answer: interface to use */
	struct	sockaddr_storage rtl_mask; /* mask */
	struct	sockaddr_storage rtl_src; /* src */
	uint32_t rtl_pmtu;		/* path mtu value */
};

#define	RTF_UP		0x1		/* route usable */
#define	RTF_GATEWAY	0x2		/* destination is a gateway */
#define	RTF_HOST	0x4		/* host entry (net otherwise) */
#define	RTF_REJECT	0x8		/* host or net unreachable */
#define	RTF_DYNAMIC	0x10		/* created dynamically (by redirect) */
#define	RTF_MODIFIED	0x20		/* modified dynamically (by redirect) */
#define RTF_DONE	0x40		/* message confirmed */
#define RTF_MASK	0x80		/* subnet mask present */
#define RTF_CLONING	0x100		/* generate new routes on use */
#define RTF_XRESOLVE	0x200		/* external daemon resolves name */
#define RTF_LLINFO	0x400		/* generated by ARP or ESIS */
#define RTF_STATIC	0x800		/* manually added */
#define RTF_BLACKHOLE	0x1000		/* just discard pkts (during updates) */
#define	RTF_TUNNEL	0x2000		/* tunnel route */
#define RTF_PROTO2	0x4000		/* protocol specific routing flag */
#define RTF_PROTO1	0x8000		/* protocol specific routing flag */
#define RTF_PERM_PMTU	0x10000

/* Some overloads for routing sockets */
#define RTF_MTU		RTF_DONE

/*
 * Structures for routing messages.
 */
struct rt_msghdr {
	uint16_t rtm_msglen;	/* to skip over non-understood messages */
	uint8_t	rtm_version;	/* future binary compatibility */
	uint8_t	rtm_type;	/* message type */
	uint16_t rtm_index;	/* index for associated ifp */
	uint16_t padding;
	int32_t	rtm_flags;	/* flags, incl. kern & message, e.g. DONE */
	int32_t	rtm_addrs;	/* bitmask identifying sockaddrs in msg */
	pid_t	rtm_pid;	/* identify sender */
	int32_t	rtm_seq;	/* for sender to identify action */
	int32_t	rtm_errno;	/* why failed */
	int32_t	rtm_use;	/* from rtentry */
	uint32_t rtm_inits;	/* which metrics we are initializing */
	struct	rt_metrics rtm_rmx; /* metrics themselves */
};

#define RTM_VERSION	3	/* Up the ante and ignore older versions */

#define RTM_ADD		0x1	/* Add Route */
#define RTM_DELETE	0x2	/* Delete Route */
#define RTM_CHANGE	0x3	/* Change Metrics or flags */
#define RTM_GET		0x4	/* Report Metrics */
#define RTM_LOSING	0x5	/* Kernel Suspects Partitioning */
#define RTM_REDIRECT	0x6	/* Told to use different route */
#define RTM_MISS	0x7	/* Lookup failed on this address */
#define RTM_LOCK	0x8	/* fix specified metrics */
#define RTM_OLDADD	0x9	/* caused by SIOCADDRT */
#define RTM_OLDDEL	0xa	/* caused by SIOCDELRT */
#define RTM_RESOLVE	0xb	/* req to resolve dst to LL addr */
#define RTM_NEWADDR	0xc	/* address being added to iface */
#define RTM_DELADDR	0xd	/* address being removed from iface */
#define RTM_IFINFO	0xe	/* iface going up/down etc. */

#define RTV_MTU		0x1	/* init or lock _mtu */
#define RTV_HOPCOUNT	0x2	/* init or lock _hopcount */
#define RTV_EXPIRE	0x4	/* init or lock _hopcount */
#define RTV_RPIPE	0x8	/* init or lock _recvpipe */
#define RTV_SPIPE	0x10	/* init or lock _sendpipe */
#define RTV_SSTHRESH	0x20	/* init or lock _ssthresh */
#define RTV_RTT		0x40	/* init or lock _rtt */
#define RTV_RTTVAR	0x80	/* init or lock _rttvar */

/*
 * Bitmask values for rtm_addr.
 */
#define RTA_DST		0x1	/* destination sockaddr present */
#define RTA_GATEWAY	0x2	/* gateway sockaddr present */
#define RTA_NETMASK	0x4	/* netmask sockaddr present */
#define RTA_GENMASK	0x8	/* cloning mask sockaddr present */
#define RTA_IFP		0x10	/* interface name sockaddr present */
#define RTA_IFA		0x20	/* interface addr sockaddr present */
#define RTA_AUTHOR	0x40	/* sockaddr for author of redirect */
#define RTA_BRD		0x80	/* for NEWADDR, broadcast or p-p dest addr */

/*
 * Index offsets for sockaddr array for alternate internal encoding.
 */
#define RTAX_DST	0	/* destination sockaddr present */
#define RTAX_GATEWAY	1	/* gateway sockaddr present */
#define RTAX_NETMASK	2	/* netmask sockaddr present */
#define RTAX_GENMASK	3	/* cloning mask sockaddr present */
#define RTAX_IFP	4	/* interface name sockaddr present */
#define RTAX_IFA	5	/* interface addr sockaddr present */
#define RTAX_AUTHOR	6	/* sockaddr for author of redirect */
#define RTAX_BRD	7	/* for NEWADDR, broadcast or p-p dest addr */
#define RTAX_MAX	8	/* size of array to allocate */

struct rt_addrinfo {
	int32_t	rti_addrs;
	struct	sockaddr *rti_info[RTAX_MAX];
	int32_t	rti_flags;
	struct	ifaddr *rti_ifa;
	struct	ifnet *rti_ifp;
};

/*
 * Structure used for SIOCGRTENTRY ioctl.
 */
#include <net/if.h>

struct rtreq {
	uint32_t rtr_destaddr;
	uint32_t rtr_gwayaddr;
	ushort rtr_flags;
	ushort rtr_refcnt;	/* unused */
	uint rtr_use;		/* unused */
	int rtr_proto;		/* unused */
	time_t rtr_upd;		/* unused */
	int rtr_pmtu_timer;	/* unused */
	ushort rtr_pmtu;
	ushort rtr_ifmtu;
	char rtr_ifname[IFNAMSIZ];
	uint rtr_subnetmask;
};

struct rtlreq {
	uint32_t rtlr_destaddr[4];
	uint32_t rtlr_gwayaddr[4];
	ushort rtlr_flags;
	ushort rtlr_pmtu;
	ushort rtlr_ifmtu;
	ushort padding;
	char rtlr_ifname[IFNAMSIZ];
	uint32_t rtlr_subnetmask[4];
	uint32_t rtlr_srcaddr[4];
};

#ifdef __cplusplus
}
#endif

#endif	/* _ROUTE_ */
