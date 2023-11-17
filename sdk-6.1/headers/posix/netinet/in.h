#ifndef _IN_
#define	_IN_

#ifdef MI_H_ID_STRINGS
static	char	in_h_sccsid[] = "@(#)in.h	43.4";
#endif

#include <sys/ansi.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/endian.h>
#include <sys/socket.h>

/* These had to go here because of a declaration issue in generated stubs. */
__BEGIN_DECLS
/* Ensure that we don't overwrite MS's prototypes if compiling PalmSim's Win32 layer */
#if !defined (NTSIMWIN32GLUE_WINSOCK)
uint32_t	htonl __P((uint32_t)) __attribute__((__const__));
uint16_t	htons __P((uint16_t)) __attribute__((__const__));
uint32_t	ntohl __P((uint32_t)) __attribute__((__const__));
uint16_t	ntohs __P((uint16_t)) __attribute__((__const__));
#endif
__END_DECLS

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)in.h	8.3 (Berkeley) 1/3/94
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
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, and numerous additions.
 */

/*
 * Protocols
 */
#define	IPPROTO_IP		0	/* dummy for IP */
#define	IPPROTO_HOPOPTS		0	/* IPv6 hop-by-hop options header */
#define	IPPROTO_ICMP		1	/* control message protocol */
#define	IPPROTO_IGMP		2	/* group mgmt protocol */
#define	IPPROTO_GGP		3	/* gateway^2 (deprecated) */
#define	IPPROTO_IPIP		4	/* IP inside IP */
#define	IPPROTO_TCP		6	/* tcp */
#define	IPPROTO_EGP		8	/* exterior gateway protocol */
#define	IPPROTO_PUP		12	/* pup */
#define	IPPROTO_UDP		17	/* user datagram protocol */
#define	IPPROTO_IDP		22	/* xns idp */
#define	IPPROTO_TP		29 	/* tp-4 w/ class negotiation */
#define	IPPROTO_XTP		36	/* xtp */
#define	IPPROTO_IPV6		41	/* IPv6 in IP[v6] encapsulation */
#define	IPPROTO_ROUTING		43	/* IPv6 source routing header */
#define	IPPROTO_FRAGMENT	44	/* IPv6 fragmentation header */
#define	IPPROTO_ESP		50	/* encapsulating security protocol */
#define	IPPROTO_AH		51	/* authentication header */
#define	IPPROTO_ICMPV6		58	/* control message protocol version 6 */
#define	IPPROTO_NONE		59	/* IPv6 no next header */
#define	IPPROTO_DSTOPTS		60	/* IPv6 destination options header */
#define	IPPROTO_EON		80	/* ISO cnlp */
#define	IPPROTO_ENCAP		98	/* encapsulation header */
#define	IPPROTO_IPPCP		108	/* IP Payload Compression protocol */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256


/*
 * Local port number conventions:
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

#ifndef in_port_t
typedef	uint16_t in_port_t;
#endif
#ifndef in_addr_t
typedef	uint32_t in_addr_t;
#endif

/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
	in_addr_t s_addr;
};

/* Maximum length of dotted decimal string (000.000.000.000) . */
#define	INET_ADDRSTRLEN		16

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((uint32_t)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((uint32_t)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((uint32_t)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((uint32_t)(i) & 0xf0000000) == 0xe0000000)
#define	IN_CLASSD_NET		0xf0000000	/* These ones aren't really */
#define	IN_CLASSD_NSHIFT	28		/* net and host fields, but */
#define	IN_CLASSD_HOST		0x0fffffff	/* routing needn't know.    */
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((uint32_t)(i) & 0xf0000000) == 0xf0000000)
#define	IN_BADCLASS(i)		(((uint32_t)(i) & 0xf0000000) == 0xf0000000)

#define	INADDR_ANY		(uint32_t)0x00000000
#define	INADDR_BROADCAST	(uint32_t)0xffffffff	/* must be masked */
#define	INADDR_LOOPBACK		(uint32_t)0x7f000001
#ifndef _KERNEL
#define	INADDR_NONE		0xffffffff		/* -1 return */
#endif

#define	INADDR_LINKLOCAL	(uint32_t)0xa9fe0000	/* 169.254.0.0 */
#define	INADDR_LINKLOCAL_NET	(uint32_t)0xffff0000	/* Class B net */

#define	INADDR_UNSPEC_GROUP	(uint32_t)0xe0000000	/* 224.0.0.0 */
#define	INADDR_ALLHOSTS_GROUP	(uint32_t)0xe0000001	/* 224.0.0.1 */
#define	INADDR_ALLRTRS_GROUP	(uint32_t)0xe0000002	/* 224.0.0.2 */
#define	INADDR_MAX_LOCAL_GROUP	(uint32_t)0xe00000ff	/* 224.0.0.255 */

#define	IN_LOOPBACKNET		127			/* official! */

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	sa_family_t	sin_family;
	in_port_t	sin_port;
	struct	in_addr sin_addr;
	uint8_t		sin_zero[8];
};

/*
 * Structure used to describe IP options.
 * Used to store options internally , to pass them to a process,
 * or to restore options retrieved earlier.
 * The ip_dst is used for the first_hop gateway when using a source route
 * (This gets put into the header proper).
 */
#ifdef NEVER
struct ip_opts {
	struct	in_addr ip_dst;		/* first hop, 0 w/o src rt */
	uint8_t	ip_opts[40];		/* actually variable size */
};
#endif

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#ifndef IP_OPTIONS
#define	IP_OPTIONS		1    /* buf/ip_opts; set/get IP options */
#endif
#ifndef IP_TOS
#define	IP_TOS			2    /* int; IP type of service and preced. */
#endif
#ifndef IP_TTL
#define	IP_TTL			3    /* int; IP time to live */
#endif
#define	IP_HDRINCL		202  /* int; header is included with data */
#define	IP_RECVOPTS		205  /* bool; receive all IP opts w/dgram */
#define	IP_RECVRETOPTS		206  /* bool; receive IP opts for response */
#define	IP_RECVDSTADDR		207  /* bool; receive IP dst addr w/dgram */
#define	IP_RETOPTS		208  /* ip_opts; set/get IP options */
#define	IP_MULTICAST_IF		209  /* uchar; set/get IP multicast i/f  */
#define	IP_MULTICAST_TTL	210  /* uchar; set/get IP multicast ttl */
#define	IP_MULTICAST_LOOP	211  /* uchar; set/get IP multicast loopback */
#define	IP_ADD_MEMBERSHIP	212  /* ip_mreq; add an IP group membership */
#define	IP_DROP_MEMBERSHIP	213  /* ip_mreq; drop an IP group membership */
#define	IP_BROADCAST_IF		214  /* set/get broadcast interface */
#define	IP_BROADCAST_IFNAME	215  /* set/get broadcast interface name */
#define	IP_LINK_STATUS		216  /* bool; receive IP i/f status w/dgram */
#define	IP_RECVIFADDR		217  /* i32; receive IP i/f addr w/dgram */
#define	IP_RECVLINKSRC		218  /* i32; recv source link-layer addr */
#define	IP_RECVTTL		219  /* i32; recv TTL of inbound packet */
#define	IP_TUNNEL_NOTIFIES	220  /* i32; be notified of tunnel events */

/*
 * Defaults and limits for options
 */
#define	IP_DEFAULT_MULTICAST_TTL  1	/* normally limit m'casts to 1 hop  */
#define	IP_DEFAULT_MULTICAST_LOOP 1	/* normally hear sends if a member  */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct ip_mreq {
	struct	in_addr imr_multiaddr;	/* IP multicast address of group */
	struct	in_addr imr_interface;	/* local IP address of interface */
};

/*
 * Internet version 6 address (a structure for historical reasons)
 */
struct in6_addr {
	uint8_t	s6_addr[16];
};

/*
 * Maximum length of colon seperated hex string
 * (0000:0000:0000:0000:0000:0000:000.000.000.000)
 */
#define	INET6_ADDRSTRLEN	46

#define	IN6ADDR_ANY_INIT	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define	IN6ADDR_LOOPBACK_INIT	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}

extern struct in6_addr in6addr_any;
extern struct in6_addr in6addr_loopback;

#define	IN6_ARE_ADDR_EQUAL(i1, i2) \
	((((i1)->s6_addr[0] ^ (i2)->s6_addr[0]) \
	| ((i1)->s6_addr[1] ^ (i2)->s6_addr[1]) \
	| ((i1)->s6_addr[2] ^ (i2)->s6_addr[2]) \
	| ((i1)->s6_addr[3] ^ (i2)->s6_addr[3]) \
	| ((i1)->s6_addr[4] ^ (i2)->s6_addr[4]) \
	| ((i1)->s6_addr[5] ^ (i2)->s6_addr[5]) \
	| ((i1)->s6_addr[6] ^ (i2)->s6_addr[6]) \
	| ((i1)->s6_addr[7] ^ (i2)->s6_addr[7]) \
	| ((i1)->s6_addr[8] ^ (i2)->s6_addr[8]) \
	| ((i1)->s6_addr[9] ^ (i2)->s6_addr[9]) \
	| ((i1)->s6_addr[10] ^ (i2)->s6_addr[10]) \
	| ((i1)->s6_addr[11] ^ (i2)->s6_addr[11]) \
	| ((i1)->s6_addr[12] ^ (i2)->s6_addr[12]) \
	| ((i1)->s6_addr[13] ^ (i2)->s6_addr[13]) \
	| ((i1)->s6_addr[14] ^ (i2)->s6_addr[14]) \
	| ((i1)->s6_addr[15] ^ (i2)->s6_addr[15])) \
	== 0)

#define	IN6_IS_ADDR_UNSPECIFIED(i) \
	(((i)->s6_addr[0] | (i)->s6_addr[1] | (i)->s6_addr[2] \
	| (i)->s6_addr[3] | (i)->s6_addr[4] | (i)->s6_addr[5] \
	| (i)->s6_addr[6] | (i)->s6_addr[7] | (i)->s6_addr[8] \
	| (i)->s6_addr[9] | (i)->s6_addr[10] | (i)->s6_addr[11] \
	| (i)->s6_addr[12] | (i)->s6_addr[13] | (i)->s6_addr[14] \
	| (i)->s6_addr[15]) == 0)

#define	IN6_IS_ADDR_LOOPBACK(i) \
	(((i)->s6_addr[0] | (i)->s6_addr[1] | (i)->s6_addr[2] \
	| (i)->s6_addr[3] | (i)->s6_addr[4] | (i)->s6_addr[5] \
	| (i)->s6_addr[6] | (i)->s6_addr[7] | (i)->s6_addr[8] \
	| (i)->s6_addr[9] | (i)->s6_addr[10] | (i)->s6_addr[11] \
	| (i)->s6_addr[12] | (i)->s6_addr[13] | (i)->s6_addr[14] \
	| ((i)->s6_addr[15] ^ 0x1)) == 0)

#define	IN6_IS_ADDR_MULTICAST(i) \
	((((i)->s6_addr[0] ^ 0xff) | ((i)->s6_addr[1] & 0xe0)) == 0)

#define	IN6_IS_ADDR_LINKLOCAL(i) \
	((((i)->s6_addr[0] ^ 0xfe) | (((i)->s6_addr[1] & 0xc0) ^ 0x80)) == 0)

#define	IN6_IS_ADDR_SITELOCAL(i) \
	((((i)->s6_addr[0] ^ 0xfe) | (((i)->s6_addr[1] & 0xc0) ^ 0xc0)) == 0)

#define	IN6_IS_ADDR_V4MAPPED(i) \
	(((i)->s6_addr[0] | (i)->s6_addr[1] | (i)->s6_addr[2] \
	| (i)->s6_addr[3] | (i)->s6_addr[4] | (i)->s6_addr[5] \
	| (i)->s6_addr[6] | (i)->s6_addr[7] | (i)->s6_addr[8] \
	| (i)->s6_addr[9] | ((i)->s6_addr[10] ^ 0xff) \
	| ((i)->s6_addr[11] ^ 0xff)) == 0)

#define	IN6_IS_ADDR_V4COMPAT(i) \
	(((i)->s6_addr[0] | (i)->s6_addr[1] | (i)->s6_addr[2] \
	| (i)->s6_addr[3] | (i)->s6_addr[4] | (i)->s6_addr[5] \
	| (i)->s6_addr[6] | (i)->s6_addr[7] | (i)->s6_addr[8] \
	| (i)->s6_addr[9] | (i)->s6_addr[10] | (i)->s6_addr[11]) == 0)

#define	IN6_IS_ADDR_MC_NODELOCAL(i) \
	((((i)->s6_addr[0] ^ 0xff) | (((i)->s6_addr[1] & 0x0f) ^ 0x1) \
	| ((i)->s6_addr[1] & 0xe0)) == 0)

#define	IN6_IS_ADDR_MC_LINKLOCAL(i) \
	((((i)->s6_addr[0] ^ 0xff) | (((i)->s6_addr[1] & 0x0f) ^ 0x2) \
	| ((i)->s6_addr[1] & 0xe0)) == 0)

#define	IN6_IS_ADDR_MC_SITELOCAL(i) \
	((((i)->s6_addr[0] ^ 0xff) | (((i)->s6_addr[1] & 0x0f) ^ 0x5) \
	| ((i)->s6_addr[1] & 0xe0)) == 0)

#define	IN6_IS_ADDR_MC_ORGLOCAL(i) \
	((((i)->s6_addr[0] ^ 0xff) | (((i)->s6_addr[1] & 0x0f) ^ 0x8) \
	| ((i)->s6_addr[1] & 0xe0)) == 0)

#define	IN6_IS_ADDR_MC_GLOBAL(i) \
	((((i)->s6_addr[0] ^ 0xff) | (((i)->s6_addr[1] & 0x0f) ^ 0x0e) \
	| ((i)->s6_addr[1] & 0xe0)) == 0)

/*
 * Socket address, IPv6 style.
 */
struct sockaddr_in6 {
	sa_family_t	sin6_family;
	in_port_t	sin6_port;
	uint32_t	sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t	sin6_scope_id;
};

/*
 * Structure used by IPV6_PKTINFO socket option.
 */
struct in6_pktinfo {
	struct in6_addr ipi6_addr;
	uint32_t	ipi6_ifindex;
};

/*
 * Options for use with [gs]etsockopt, sendmsg and recvmsg at the
 * IPv6 level.  First word of comment is data type; bool is stored
 * in int.
 */
#define	IPV6_UNICAST_HOPS	1    /* int; set/get IPv6 unicast hop limit */
#define	IPV6_MULTICAST_IF	2    /* int; set/get IPv6 multicast i/f */
#define	IPV6_MULTICAST_HOPS	3    /* int; set/get IPv6 multicast hop limit */
#define	IPV6_MULTICAST_LOOP	4    /* bool; set/get IPv6 multicast loop */
#define	IPV6_JOIN_GROUP		5    /* ipv6_mreq; add IPv6 group membership */
#define	IPV6_LEAVE_GROUP	6    /* ipv6_mreq; drop IPv6 group membership */
#define	IPV6_CHECKSUM		7    /* int; set/get offset for checksum */
#define	IPV6_NEXTHOP		9    /* sockaddr_in6; set/get IPv6 next hop */
#define	IPV6_PKTINFO		10   /* in6_pktinfo; set/get IPv6 packet info */
#define	IPV6_HOPLIMIT		11   /* int; set/get IPv6 hop limit */
#define	IPV6_HOPOPTS		12   /* buf; set/get IPv6 hop-by-hop options */
#define	IPV6_DSTOPTS		13   /* buf; set/get IPv6 destination options */
#define	IPV6_RTHDR		14   /* buf; set/get IPv6 routing header */
#define	IPV6_RTHDRDSTOPTS	15   /* buf; set/get IPv6 destination options */
#define	IPV6_PATHMTU		16   /* int; path MTU */
#define	IPV6_REACHCONF		17   /* void; reachability confirmation */
#define	IPV6_RECVPKTINFO	18   /* bool; receive IPv6 packet information */
#define	IPV6_RECVHOPLIMIT	19   /* bool; receive IPv6 hop limit */
#define	IPV6_RECVHOPOPTS	20   /* bool; receive IPv6 hop-by-hop options */
#define	IPV6_RECVDSTOPTS	21   /* bool; receive IPv6 dest. options */
#define	IPV6_RECVRTHDR		22   /* bool; receive IPv6 routing header */
#define	IPV6_RECVRTHDRDSTOPTS	23   /* bool; receive IPv6 dest. options */
#define	IPV6_RECVPATHMTU	24   /* bool; receive path MTU */
#define	IPV6_LINK_STATUS	25   /* bool; receive IPv6 i/f status w/dgram */
#define IPV6_V6ONLY		26   /* bool; IPv6 communications only */
#define IPV6_USE_MIN_MTU	27   /* bool; use the minimum IPV6 MTU */
#define IPV6_BIND_NOTIFIES	28   /* bool; receive binding notifications */
#define IPV6_BINDING_BYPASS	29   /* bool; kernel binding ignored on sends */
#define IPV6_SRC_PREFERENCES	30   /* int; set/get address prefs */

/*
 * Defaults and limits for options
 */
#define	IPV6_DEFAULT_MULTICAST_HOPS	1
#define	IPV6_DEFAULT_MULTICAST_LOOP	1

struct ipv6_mreq {
	struct in6_addr ipv6mr_multiaddr;	/* IPv6 multicast addr */
	uint32_t	ipv6mr_interface;	/* interface index */
};

#define	IPV6_RTHDR_TYPE_0	0

/*
 * Structure used by ioctl(SIOCGPTL) and ioctl(SIOCADDPTL).
 */
struct ptlentry {
	struct sockaddr_in6	ptl_prefix;
	int			ptl_prefix_len;
	int			ptl_precedence;
	int			ptl_label;
};

/*
 * Structure used by ioctl(SIOCGCSTL) and ioctl(SIOCADDCSTL).
 */
#include <net/if.h>

struct cstlentry {
	struct sockaddr_in6	cstl_prefix;
	int			cstl_prefix_len;
	int			cstl_num_ifname;
	char			cstl_ifname[1][IFNAMSIZ];
};

/*
 * Structure used by ioctl(SIOCGDASPREF) and ioctl(SIOCSDASPREF).
 */
struct	daspref {
	int		dp_preference;	/* IPV6_PREFER_SRC_xxx */
	int		dp_enhanced_mode;/* is category ordering effective? */
	int		dp_category[4]; /* category ordering (IP_ACAT_xx) */
};

/* These flags currently have no effect, but are reserved for future use. */
#define IPV6_PREFER_SRC_DEFAULT	0x00000000
#define IPV6_PREFER_SRC_COA	0x00000001	/* not implemented */
#define IPV6_PREFER_SRC_TMP	0x00000002	/* not implemented */
#define IPV6_PREFER_SRC_MASK	0x00000003

#define IP_ACAT_V4_GLOBAL	0	/* global IPv4 address */
#define IP_ACAT_V4_PRIVATE	1	/* private IPv4 address */
#define IP_ACAT_V6_NATIVE	2	/* native IPv6 communication */
#define IP_ACAT_V6_TUNNEL	3	/* tunnel IPv6 communication */

/*
 * Structure used by ioctl(SIOCGDEFLSRC).
 * for address families AF_INET and AF_INET6.
 */
struct deflsrc {
	int			ds_preference;	/* preference */
	struct sockaddr_storage	ds_dstaddr;	/* destination */
	struct sockaddr_storage	ds_srcaddr;	/* returned source address */
	char			ds_ifname[IFNAMSIZ]; /* returned interface */
};

extern	uint8_t	* inet6_option_alloc(struct cmsghdr *, int, int, int);
extern	int	inet6_option_append(struct cmsghdr *, const uint8_t *, int);
extern	int	inet6_option_find(const struct cmsghdr *, uint8_t *, int);
extern	int	inet6_option_init(void *, struct cmsghdr **, int);
extern	int	inet6_option_next(const struct cmsghdr *, uint8_t **);
extern	size_t	inet6_option_space(int);

extern	int	inet6_rthdr_add(struct cmsghdr *, const struct in6_addr *
		, unsigned int);
extern	struct in6_addr inet6_rthdr_getaddr(struct cmsghdr *, int);
extern	struct cmsghdr * inet6_rthdr_init(void *, int);
extern	struct cmsghdr * inet6_rthdr_init(void * , int);
extern	int	inet6_rthdr_lasthop(const struct cmsghdr *, struct cmsghdr *);
extern	int	inet6_rthdr_reverse(const struct cmsghdr *, struct cmsghdr *);
extern	int	inet6_rthdr_segments(const struct cmsghdr *);
extern	size_t	inet6_rthdr_space(int, int);

#ifdef __cplusplus
}
#endif

#endif	/* _IN_ */
