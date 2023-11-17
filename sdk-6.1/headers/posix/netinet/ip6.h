#ifndef _IP6_
#define	_IP6_

#include <stdint.h>
#include <netinet/in.h>

#ifdef MI_H_ID_STRINGS
static	char	ip6_h_sccsid[] = "@(#)ip6.h	43.2";
#endif

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1982, 1986, 1993
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
 * Definitions for internet protocol version 6.
 */
#define	IP6VERSION	6

/*
 * Structure of an internet version 6 header
 */
struct ip6_hdr {
	union {
		struct ip6_hdrctl {
			uint32_t ip6_un1_flow;
			uint16_t ip6_un1_plen;
			uint8_t	ip6_un1_nxt;
			uint8_t	ip6_un1_hlim;
		} ip6_un1;
		uint8_t	ip6_un2_vfc;
	} ip6_ctlun;
	struct	in6_addr ip6_src;
	struct	in6_addr ip6_dst;
};
#define	ip6_vfc		ip6_ctlun.ip6_un2_vfc
#define	ip6_flow	ip6_ctlun.ip6_un1.ip6_un1_flow
#define	ip6_plen	ip6_ctlun.ip6_un1.ip6_un1_plen
#define	ip6_nxt		ip6_ctlun.ip6_un1.ip6_un1_nxt
#define	ip6_hlim	ip6_ctlun.ip6_un1.ip6_un1_hlim
#define	ip6_hops	ip6_ctlun.ip6_un1.ip6_un1_hlim

/*
 * Internet version 6 implementation parameters.
 */
#define	MAXHOPS		255		/* maximum hop limit */

#define	IP6_MSS		1280		/* default minimum segment size */

/*
 * Structure of an internet version 6 hop-by-hop options header
 */
struct ip6_hbh {
	uint8_t	ip6h_nxt;	/* next header */
	uint8_t	ip6h_len;	/* length in units of 8 octets */
	/* variable length options */
};

/*
 * Structure of an internet version 6 destination options header
 */
struct ip6_dest {
	uint8_t	ip6d_nxt;	/* next header */
	uint8_t	ip6d_len;	/* length in units of 8 octets */
	/* variable length options */
};

/*
 * Structure of internet version 6 generic routing header
 */
struct ip6_rthdr {
	uint8_t	ip6r_nxt;	/* next header */
	uint8_t	ip6r_len;	/* length in units of 8 octets */
	uint8_t	ip6r_type;	/* routing type */
	uint8_t	ip6r_segleft;	/* segments left */
	/* type specific variable length segments */
};

/*
 * Structure of internet version 6 type 0 routing header
 */
struct ip6_rthdr0 {
	uint8_t	ip6r0_nxt;	/* next header */
	uint8_t	ip6r0_len;	/* length in units of 8 octets */
	uint8_t	ip6r0_type;	/* always zero */
	uint8_t	ip6r0_segleft;	/* segments left */
	uint32_t ip6r0_reserved; /* reserved field */
	struct	in6_addr ip6r0_addr[1];	/* up to 127 addresses */
};

/*
 * Structure of internet version 6 fragmentation header
 */
struct ip6_frag {
	uint8_t	ip6f_nxt;	/* next header */
	uint8_t	ip6f_reserverd;	/* reserved field */
	uint16_t ip6f_offlg;	/* offset, reserved, and flag */
	uint32_t ip6f_ident;	/* identification */
};
#if BYTE_ORDER == BIG_ENDIAN
#define	IP6F_OFF_MASK		0xfff8
#define	IP6F_RESERVED_MASK	0x0006
#define	IP6F_MORE_FRAG		0x0001
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define	IP6F_OFF_MASK		0xf8ff
#define	IP6F_RESERVED_MASK	0x0600
#define	IP6F_MORE_FRAG		0x0100
#endif

#define	IP6OPT_TYPE(o)		((0) & 0xc0)
#define	IP6OPT_TYPE_SKIP	0x00
#define	IP6OPT_TYPE_DISCARD	0x40
#define	IP6OPT_TYPE_FORCEICMP	0x80
#define	IP6OPT_TYPE_ICMP	0xc0

#define	IP6OPT_PAD1		0x00
#define	IP6OPT_PADN		0x01
#define	IP6OPT_JUMBO		0xc2
#define	IP6OPT_NSAP_ADDR	0xc3
#define	IP6OPT_TUNNEL_LIMIT	0x04
#define	IP6OPT_BINDING_UPDATE	0xc6
#define	IP6OPT_BINDING_ACK	0x07
#define	IP6OPT_BINDING_REQ	0x08
#define	IP6OPT_HOME_ADDRESS	0xc9
#define	IP6OPT_EID		0x8a

/* Basic option header, data follows it. */
struct ip6_opt {
	uint8_t	ip6o_type;
	uint8_t	ip6o_len;
};

/* Jumbo payload option */
struct ip6_opt_jumbo {
	uint8_t	ip6oj_type;
	uint8_t	ip6oj_len;
	uint8_t	ip6oj_jumbo_len[4];
};
#define	IP6OPT_JUMBO_LEN	6

/* NSAP address option */
struct ip6_opt_nsap {
	uint8_t	ip6on_type;
	uint8_t	ip6on_len;
	uint8_t	ip6on_src_nsap_len;
	uint8_t	ip6on_dst_nsap_len;
	/* Followed by source NSAP */
	/* Followed by destination NSAP */
};

struct ip6_opt_tunnel {
	uint8_t	ip6ot_type;
	uint8_t	ip6ot_len;
	uint8_t	ip6ot_encap_limit;
};

struct ip6_opt_router {
	uint8_t	ip6or_type;
	uint8_t	ip6or_len;
	uint8_t	ip6or_value[2];
};

#if BYTE_ORDER == BIG_ENDIAN
#define	IP6_ALERT_MLD	0x0000
#define	IP6_ALERT_RSVP	0x0001
#define	IP6_ALERT_AN	0x0002
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define	IP6_ALERT_MLD	0x0000
#define	IP6_ALERT_RSVP	0x0100
#define	IP6_ALERT_AN	0x0200
#endif

/*
 * The mobility-related options that follow are coded to the 
 * draft-ietf-mobileip-ipv6-15.txt version of MIPv6.  This may well
 * change by the time RFC status is reached....
 */

/* Binding update option - aligned 4n+2 */
struct ip6_opt_binding_update {
	uint8_t	ip6ou_type;
	uint8_t	ip6ou_len;
	uint8_t	ip6ou_flags;
	uint8_t	ip6ou_reserved[2];
	uint8_t	ip6ou_seqno;
	uint8_t	ip6ou_lifetime[4];
	/* Followed by sub-options */
};

/* Binding Update Flag values */
#define	IP6_BUF_ACK	0x80	/* Request Binding Acknowledgement */
#define	IP6_BUF_HOME	0x40	/* This is a Home Registration */
#define	IP6_BUF_SINGLE	0x20	/* Only update Home Address option binding */
#define	IP6_BUF_DAD	0x10	/* Perform duplicate address detection */

/* Binding update option - aligned 4n+3 */
struct ip6_opt_binding_ack {
	uint8_t	ip6oa_type;
	uint8_t	ip6oa_len;
	uint8_t	ip6oa_status;
	uint8_t	ip6oa_reserved;
	uint8_t	ip6oa_seqno;
	uint8_t	ip6oa_lifetime[4];
	uint8_t	ip6oa_refresh[4];
	/* Followed by sub-options */
};

/* Binding Ack ip6oa_status values. */
#define	IP6_BA_UNSPECIFIED	128	/* Reason unspecified */
#define	IP6_BA_ADMIN_PROHIBITED	130	/* Administratively prohibited */
#define	IP6_BA_RESOURCES	131	/* Insufficient resources */
#define	IP6_BA_NO_HOME_REG	132	/* Home registration not supported */
#define	IP6_BA_NOT_HOME_SUBNET	133	/* Not home subnet */
#define	IP6_BA_NOT_HOME_AGENT	137	/* Not home agent for this MN */
#define	IP6_BA_DAD_FAILURE	138	/* Duplicate Address Detection failed */
#define	IP6_BA_NO_SA		139	/* No security association */
#define	IP6_BA_SEQUENCE_SMALL	141	/* Sequence number too small */

/* Binding request option */
struct ip6_opt_binding_request {
	uint8_t	ip6or_type;
	uint8_t	ip6or_len;
	/* Followed by sub-options */
};

/* Home address option - aligned 8n+6 */
struct ip6_opt_home_address {
	uint8_t	ip6oha_type;
	uint8_t	ip6oha_len;
	uint8_t	ip6oha_addr[16];
	/* Followed by sub-options */
};

/* Mobility-related sub-options */
#define	IP6SOPT_PAD1		0x00	/* One byte of pad */
#define	IP6SOPT_PADN		0x01	/* N bytes of pad */
#define	IP6SOPT_UNIQUE_ID	0x02	/* Unique identifier for binding */
#define	IP6SOPT_ACOA		0x03	/* Alternate care-of-address */
#define	IP6SOPT_AUTH_DATA	0x04	/* Authenication Data */

/* Unique Identifier sub-option - aligned 2n */
struct ip6_sub_opt_uid {
	uint8_t	ip6sou_type;
	uint8_t	ip6sou_len;
	uint8_t ip6sou_id[2];
};

/* Alternate care-of-address sub-option - aligned 8n+6 */
struct ip6_sub_opt_acoa {
	uint8_t	ip6soa_type;
	uint8_t	ip6soa_len;
	uint8_t	ip6soa_addr[16];
};

/* Authentication Data sub-option - aligned 8n+6 */
struct ip6_sub_opt_auth {
	uint8_t	ip6soau_type;
	uint8_t	ip6soau_len;
	uint8_t	ip6soau_spi[4];
	/* Authentication data, a variable amount, follows. */
};


#ifdef __cplusplus
}
#endif

#endif	/* _IP6_ */
