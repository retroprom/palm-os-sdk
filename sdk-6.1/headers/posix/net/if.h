/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: if.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/
#ifndef _IF_
#define	_IF_

#ifdef MI_H_ID_STRINGS
static	char	if_h_sccsid = "@(#)if.h	43.7";
#endif

#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1982, 1986, 1989, 1993
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
 *	@(#)if.h	8.1 (Berkeley) 6/10/93
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
 * Structures defining a network interface, providing a packet
 * transport mechanism (ala level 0 of the PUP protocols).
 *
 * Each interface accepts output datagrams of a specified maximum
 * length, and provides higher level routines with input datagrams
 * received from its medium.
 *
 * Output occurs when the routine if_output is called, with three parameters:
 *      (*ifp->if_output)(ifp, m, dst, rt)
 * Here m is the mbuf chain to be sent and dst is the destination address.
 * The output routine encapsulates the supplied datagram if necessary,
 * and then transmits it on its medium.
 *
 * On input, each interface unwraps the data received by it, and either
 * places it on the input queue of a internetwork datagram routine
 * and posts the associated software interrupt, or passes the datagram to a raw
 * packet input routine.
 *
 * Routines exist for locating interfaces by their addresses
 * or for locating a interface on a certain network, as well as more general
 * routing and gateway routines maintaining information used to locate
 * interfaces.  These routines live in the files if.c and route.c
 */

/*
 * Structure describing information about an interface
 * which may be of interest to management entities.
 */
/*
 * Structure defining a queue for a network interface.
 *
 * (Would like to call this struct ``if'', but C isn't PL/1.)
 */

struct if_data {
	/* generic interface information */
	uint8_t	ifi_type;		/* ethernet, tokenring, etc */
	uint8_t	ifi_physical;		/* e.g., AUI, Thinnet, 10base-T, etc */
	uint8_t	ifi_addrlen;		/* media address length */
	uint8_t	ifi_hdrlen;		/* media header length */
	uint8_t	ifi_recvquota;		/* polling quota for receive intrs */
	uint8_t	ifi_xmitquota;		/* polling quota for xmit intrs */
	uint16_t padding;
	uint32_t ifi_mtu;		/* maximum transmission unit */
	uint32_t ifi_metric;		/* routing metric (external only) */
	uint32_t ifi_baudrate;		/* linespeed */
	/* volatile statistics */
	uint32_t ifi_ipackets;		/* packets received on interface */
	uint32_t ifi_ierrors;		/* input errors on interface */
	uint32_t ifi_opackets;		/* packets sent on interface */
	uint32_t ifi_oerrors;		/* output errors on interface */
	uint32_t ifi_collisions;		/* collisions on csma interfaces */
	uint32_t ifi_ibytes;		/* total number of octets received */
	uint32_t ifi_obytes;		/* total number of octets sent */
	uint32_t ifi_imcasts;		/* packets received via multicast */
	uint32_t ifi_omcasts;		/* packets sent via multicast */
	uint32_t ifi_iqdrops;		/* dropped on input, this interface */
	uint32_t ifi_noproto;		/* destined for unsupported protocol */
	uint32_t ifi_hwassist;		/* HW offload capabilities */
	uint32_t ifi_unused;		/* XXX was ifi_xmittiming */
	struct	timeval ifi_lastchange;	/* time of last administrative change */
};
/* Capabilities that interfaces can advertise. */
#define IFCAP_RXCSUM		0x0001  /* can offload checksum on RX */
#define IFCAP_TXCSUM		0x0002  /* can offload checksum on TX */
#define IFCAP_NETCONS		0x0004  /* can be a network console */

#define IFCAP_HWCSUM		(IFCAP_RXCSUM | IFCAP_TXCSUM)

struct ifnet {
	char	* if_name;		/* name, e.g. ``en'' or ``lo'' */
	struct	ifnet * if_next;	/* all struct ifnets are chained */
	struct	ifaddr * if_addrlist;	/* linked list of addresses per if */
	int	if_pcount;		/* number of promiscuous listeners */
	char	* if_bpf;		/* packet filter structure */
	uint16_t if_index;		/* numeric abbreviation for this if  */
	int16_t	if_unit;		/* sub-unit for lower level driver */
	int16_t	if_timer;		/* time 'til if_watchdog called */
	int16_t	if_flags;		/* up/down, broadcast, etc. */
	struct	if_data ifdata;		/* generic interface information. */
/* procedure handles */
	int	(*if_init)();		/* init routine */
	int	(*if_output)();		/* output routine (enqueue) */
	int	(*if_start)();		/* initiate output routine */
	int	(*if_done)();		/* output complete routine */
	int	(*if_ioctl)();		/* ioctl routine */
	int	(*if_reset)();
	int	(*if_watchdog)();	/* timer routine */
	struct	ifqueue {
		struct	mbuf * ifq_head;
		struct	mbuf * ifq_tail;
		int32_t	ifq_len;
		int32_t	ifq_maxlen;
		int32_t	ifq_drops;
	} ifsnd;			/* output queue */
};
#define	if_mtu		if_data.ifi_mtu
#define	if_type		if_data.ifi_type
#define	if_addrlen	if_data.ifi_addrlen
#define	if_hdrlen	if_data.ifi_hdrlen
#define	if_metric	if_data.ifi_metric
#define	if_baudrate	if_data.ifi_baudrate
#define	if_ipackets	if_data.ifi_ipackets
#define	if_ierrors	if_data.ifi_ierrors
#define	if_opackets	if_data.ifi_opackets
#define	if_oerrors	if_data.ifi_oerrors
#define	if_collisions	if_data.ifi_collisions
#define	if_ibytes	if_data.ifi_ibytes
#define	if_obytes	if_data.ifi_obytes
#define	if_imcasts	if_data.ifi_imcasts
#define	if_omcasts	if_data.ifi_omcasts
#define	if_iqdrops	if_data.ifi_iqdrops
#define	if_noproto	if_data.ifi_noproto
#define	if_lastchange	if_data.ifi_lastchange

#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* is a loopback net */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers */
#define	IFF_RUNNING	0x40		/* resources allocated */
#define	IFF_NOARP	0x80		/* no address resolution protocol */
#define	IFF_PROMISC	0x100		/* receive all packets */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets */
#define	IFF_MULTICAST	0x400		/* supports multicast */
#define	IFF_MOBILITY	0x800		/* allocated by mobility */
#define	IFF_UNNUMBERED	0x1000		/* non-unique local address */
#define	IFF_PRIVATE	0x2000		/* do not advertise */
#define	IFF_TUNNEL	0x4000		/* tunnel */
#define	IFF_ANYCAST	0x8000		/* anycast address */
#define	IFF_AUTO	0x10000		/* auto configured address */
#define	IFF_ONLINK	0x20000		/* onlink address */
#define	IFF_TENTATIVE	0x40000		/* tentative address */
#define	IFF_DEPRECATED	0x80000		/* deprecated address */

/* flags set internally only: */
#define	IFF_CANTCHANGE \
	(IFF_POINTOPOINT | IFF_RUNNING | IFF_ALLMULTI | IFF_TUNNEL | IFF_AUTO \
	| IFF_TENTATIVE | IFF_DEPRECATED)

/*
 * Output queues (ifp->if_snd) and internetwork datagram level (pup level 1)
 * input routines have queues of messages stored on ifqueue structures
 * (defined above).  Entries are added to and deleted from these structures
 * by these macros, which should be called with ipl raised to splimp().
 */
#define	IF_QFULL(ifq)		((ifq)->ifq_len >= (ifq)->ifq_maxlen)
#define	IF_DROP(ifq)		((ifq)->ifq_drops++)
#define	IF_ENQUEUE(ifq, m) { \
	(m)->m_nextpkt = 0; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_head = m; \
	else \
		(ifq)->ifq_tail->m_nextpkt = m; \
	(ifq)->ifq_tail = m; \
	(ifq)->ifq_len++; \
}
#define IF_PREPEND(ifq, m) { \
	(m)->m_nextpkt = (ifq)->ifq_head; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_tail = (m); \
	(ifq)->ifq_head = (m); \
	(ifq)->ifq_len++; \
}
#define IF_DEQUEUE(ifq, m) { \
	(m) = (ifq)->ifq_head; \
	if (m) { \
	if (((ifq)->ifq_head = (m)->m_nextpkt) == 0) \
			(ifq)->ifq_tail = 0; \
		(m)->m_nextpkt = 0; \
		(ifq)->ifq_len--; \
	} \
}

#define	IFQ_MAXLEN	50
#define	IFNET_SLOWHZ	1	/* granularity is 1 second */

/*
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address families,
 * are allocated and attached when an address is set, and are linked
 * together so all addresses for an interface can be located.
 */
struct ifaddr {
	struct	sockaddr * ifa_addr;	/* address of interface */
	struct	sockaddr * ifa_dstaddr;	/* other end of p-to-p link */
#define	ifa_broadaddr	ifa_dstaddr	/* broadcast address interface */
	struct	sockaddr * ifa_netmask;	/* used to determine subnet */
	struct	ifnet * ifa_ifp;	/* back-pointer to interface */
	struct	ifaddr * ifa_next;	/* next address for interface */
	void	(*ifa_rtrequest)();	/* check or clean routes (+ or -)'d */
	uint16_t ifa_flags;		/* mostly rt_flags for cloning */
	int16_t	ifa_refcnt;		/* extra to malloc for link info */
	int32_t	ifa_metric;		/* cost of going out this interface */
};
#define	IFA_ROUTE	RTF_UP          /* route installed */

/*
 * Message format for use in obtaining information about interfaces
 * from getkerninfo and the routing socket
 */
struct if_msghdr {
	uint16_t ifm_msglen;	/* to skip over non-understood messages */
	uint8_t	ifm_version;	/* future binary compatability */
	uint8_t	ifm_type;	/* message type */
	int32_t	ifm_addrs;	/* like rtm_addrs */
	int32_t	ifm_flags;	/* value of if_flags */
	uint16_t ifm_index;	/* index for associated ifp */
	uint16_t padding;
	struct	if_data ifm_data;/* statistics and other data about if */
};

/*
 * Message format for use in obtaining information about interface addresses
 * from getkerninfo and the routing socket
 */
struct ifa_msghdr {
	uint16_t ifam_msglen;	/* to skip over non-understood messages */
	uint8_t	ifam_version;	/* future binary compatability */
	uint8_t	ifam_type;	/* message type */
	int32_t	ifam_addrs;	/* like rtm_addrs */
	int32_t	ifam_flags;	/* value of ifa_flags */
	uint16_t ifam_index;	/* index for associated ifp */
	uint16_t padding;
	int32_t	ifam_metric;	/* value of ifa_metric */
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct ifreq {
#define	IFNAMSIZ	36
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		int16_t	ifru_flags;		/* Should match ifflags_t */
		uint64_t ifru_xflags;		/* Should match ifxflags_t */
		int32_t	ifru_metric;
		char	* ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_xflags	ifr_ifru.ifru_xflags	/* extended flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_mtu		ifr_ifru.ifru_metric	/* mtu */
#define	ifr_index	ifr_ifru.ifru_metric	/* index */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};

/*
 * Common lifetime structure used in (ipv6) interface and alias requests.
 */
struct iflife {
	uint32_t ifl_plife;			/* preferred lifetime */
	uint32_t ifl_vlife;			/* valid lifetime */
};

/*
 * Long interface request structure used for socket
 * ioctl's.  All long interface ioctl's must have parameter
 * definitions which begin with iflr_name.  The
 * remainder may be interface specific.
 */
struct iflreq {
	char	iflr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr_storage iflru_addr;
		struct	sockaddr_storage iflru_dstaddr;
		struct	sockaddr_storage iflru_broadaddr;	/* RESERVED */
		int32_t	iflru_flags;		/* Should match iflflags_t */
		uint64_t iflru_xflags;		/* Should match ifxflags_t */
		int32_t	iflru_metric;
		struct 	iflife	iflru_lifetimes;
		char	* iflru_data;
	} iflr_iflru;
#define	iflr_addr	iflr_iflru.iflru_addr	/* address */
#define	iflr_dstaddr	iflr_iflru.iflru_dstaddr /* other end of p-to-p link */
#define	iflr_broadaddr	iflr_iflru.iflru_broadaddr /* broadcast address */
#define	iflr_flags	iflr_iflru.iflru_flags	/* flags */
#define	iflr_xflags	iflr_iflru.iflru_xflags	/* extended flags */
#define	iflr_lifetimes	iflr_iflru.iflru_lifetimes /* preferred and valid
						   lifetimes */
#define	iflr_metric	iflr_iflru.iflru_metric	/* metric */
#define	iflr_mtu	iflr_iflru.iflru_metric	/* mtu */
#define	iflr_index	iflr_iflru.iflru_metric	/* index */
#define	iflr_prefix	iflr_iflru.iflru_metric	/* prefix */
#define	iflr_data	iflr_iflru.iflru_data	/* for use by interface */
};

struct ifaliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr ifra_addr;
	struct	sockaddr ifra_broadaddr;
	struct	sockaddr ifra_mask;
};

struct iflaliasreq {
	char	iflra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr_storage iflra_addr;
	struct	sockaddr_storage iflra_broadaddr;	/* RESERVED */
	struct	sockaddr_storage iflra_mask;
	struct 	iflife	iflra_lifetimes;
};

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct ifconf {
	int32_t	ifc_len;		/* size of associated buffer */
	union {
		char	* ifcu_buf;
		struct	ifreq * ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};

/*
 * Structure used in SIOCGIFLCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct iflconf {
	int32_t	iflc_len;		/* size of associated buffer */
	union {
		char	* iflcu_buf;
		struct	iflreq * iflcu_req;
	} iflc_iflcu;
#define	iflc_buf	iflc_iflcu.iflcu_buf /* buffer address */
#define	iflc_req	iflc_iflcu.iflcu_req /* array of structures returned */
};

/* Structure used by if_nameindex and if_freenameindex. */
struct if_nameindex {
	uint32_t if_index;	/* 1, 2, ... */
	char	* if_name;	/* null terminated name: "le0" */
};

/* Tunnel parameters structure used by SIOCG/SIFTUNPARMS */
struct iftun_parms {
	char	iftp_name[IFNAMSIZ];		/* v4tu0:nnn interface name */
	struct sockaddr_storage iftp_tunnel;	/* Outer header src or dst */
	struct sockaddr_storage iftp_node;	/* Node which tunnel is for */
	uint32_t	iftp_ttl;		/* 0 for kernel default */
	uint32_t	iftp_type;		/* Type of tunnel, see below */
	uint32_t	iftp_flags;		/* Misc. flags, see below */
	uint32_t	iftp_pmtu;		/* Soft path MTU, "get" only */
	uint32_t	iftp_status;		/* Soft status, "get" only */
   };

/* iftp_flags values for iftun_parms */
#define	IFTP_F_NOTIFY_FORWARDS	0x01	/* "upcall" on pkt forwarding */
#define	IFTP_F_ANY_INBOUND_OK	0x02	/* Any inbound tunnel OK */
#define	IFTP_F_SOURCE_FORWARDING 0x04	/* Tunnel pkts via src addr */
#define	IFTP_F_XUNICAST_ENCAP	0x08	/* broad/multicast get encap. */
#define	IFTP_F_TUNNEL_ENCAP	0x10	/* Tunnel if rcv'ed encap. */
#define	IFTP_F_INBOUND_ONLY	0x20	/* To authorize inbound pkts */
#define	IFTP_F_FLOAT_OUTERSRC	0x40	/* Null src addr, let routing choose. */
#define	IFTP_F_ANY_NODE_OK	0x80	/* Allow any inner packet src/dst. */

/* iftp_type values for iftun_parms */
#define	IFTP_TUNNEL_IPINIP	1	/* IP-in-IP tunneling */
#define	IFTP_TUNNEL_MINENCAP	2	/* Minimal encapsulation tunnel */
#define	IFTP_TUNNEL_GRE		4	/* GRE */

/* iftp_status values for iftun_parms */
#define	IFTP_F_STATUS_UNREACH	0x01	/* recent "unreachable" status */
#define	IFTP_F_STATUS_HOPLIMIT	0x02	/* recent "hop limit" status */

/*
 * Notification structure for tunnel "upcall" events to application.  This is
 * used in conjunction with the IP_TUNNEL_NOTIFIES socket option along with
 * the above tunnel forwarding flag.
 */
struct iftun_notify {
	struct sockaddr_storage iftn_src;	/* src addr of forwarded pkt */
	struct sockaddr_storage iftn_dst;	/* dst addr of forwarded pkt */
	uint32_t	iftn_event;		/* Event reason */
	char	iftn_name[IFNAMSIZ];		/* v4tu0:nnn interface name */
};
/* iftn_event events - currently only one */
#define	IFTN_EVENT_FORWARD	1

/* Mobility binding interface for IPv6. */

/*
 * Structure used for SIOC[ADD|DEL|GET]BINDL ioctls.
 */
struct bindlentry {
	unsigned int bindl_flags;		/* Flags. */
	struct sockaddr_storage bindl_mn;	/* Mobile Node (key). */
	struct sockaddr_storage bindl_coa;	/* Care Of Address. */
	struct sockaddr_storage bindl_tsrc;	/* Tunnel src. */
};

/* bindl_flags values for bindlentry. */
#define	IP6BE_F_TUNNEL	0x01	/* Tunnel forwarded MN packets to COA. */
#define	IP6BE_F_USE_TSRC 0x02	/* Use tsrc for tunnel addr. */

/*
 * Structure used with the IPV6_BIND_NOTIFY upcall event to inform the
 * requesting application of an incoming mobility binding operation.  This
 * structure is embedded within the IPV6_BIND_NOTIFY option and is
 * accompanied by one or more IPV6_DSTOPTS which contain the mobility binding
 * the home address option.  The packet addresses from the header are prior to
 * any Home Address Option swapping with the src addr but after any routing
 * header update to the dst.
 */
struct bindlnotify {
	unsigned int bn_flags;			/* Flags. */
	struct sockaddr_storage bn_src;		/* Src from packet header */
	struct sockaddr_storage bn_dst;		/* Dst from packet header */
};

/* bn_flags values for upcall. */
#define	IP6BN_F_HAO_PROCESSED	0x01		/* HOA has been processed. */

extern	uint32_t if_nametoindex(const char * );
extern	char	* if_indextoname(uint32_t, char *);
extern	struct if_nameindex * if_nameindex(void);
extern	void	if_freenameindex(struct if_nameindex *);

#ifdef __cplusplus
}
#endif

#endif	/* _IF_ */
