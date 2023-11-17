#ifndef _ICMP6_
#define	_ICMP6_

#include <stdint.h>
#include <netinet/in.h>

#ifdef MI_H_ID_STRINGS
static	char	icmp6_h_sccsid[] = "@(#)icmp6.h	43.3";
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
 * Interface Control Message Protocol Version 6 Definitions.
 */

/*
 * Structure of an icmpv6 header.
 */
struct icmp6_hdr {
	uint8_t	icmp6_type;		/* type of message, see below */
	uint8_t	icmp6_code;		/* type sub code */
	uint16_t icmp6_cksum;		/* ones complement cksum of struct */
	union {
		uint32_t icmp6_un_data32[1];	/* type-specific field */
		uint16_t icmp6_un_data16[2];	/* type-specific field */
		uint8_t	icmp6_un_data8[4];	/* type-specific field */
	} icmp6_dataun;
#define	icmp6_data32	icmp6_dataun.icmp6_un_data32
#define	icmp6_data16	icmp6_dataun.icmp6_un_data16
#define	icmp6_data8	icmp6_dataun.icmp6_un_data8
#define	icmp6_pptr	icmp6_data32[0]
#define	icmp6_mtu	icmp6_data32[0]
#define	icmp6_id	icmp6_data16[0]
#define	icmp6_seq	icmp6_data16[1]
#define	icmp6_maxdelay	icmp6_data16[0]
};

/*
 * Definition of type and code field values.
 */
#define	ICMP6_DST_UNREACH	1
#define	ICMP6_PACKET_TO_BIG	2
#define	ICMP6_TIME_EXCEEDED	3
#define	ICMP6_PARAM_PROB	4

#define	ICMP6_INFOMSG_MASK	0x80	/* all informational message */

#define	ICMP6_ECHO_REQUEST		128
#define	ICMP6_ECHO_REPLY		129
#define	ICMP6_MEMBERSHIP_QUERY		130
#define	ICMP6_MEMBERSHIP_REPORT		131
#define	ICMP6_MEMBERSHIP_REDUCTION	132

#define	ICMP6_ROUTER_SOLICITATION	133
#define	ICMP6_ROUTER_ADVERTISEMENT	134
#define	ICMP6_NEIGHBOR_SOLICITATION	135
#define	ICMP6_NEIGHBOR_ADVERTISEMENT	136
#define	ICMP6_ROUTER_REDIRECT		137

#define	ICMP6_HA_ADDR_DISC_REQUEST	150
#define	ICMP6_HA_ADDR_DISC_REPLY	151
#define	ICMP6_MOBILE_PREFIX_SOLICIT	152
#define	ICMP6_MOBILE_PREFIX_ADVERT	153


#define	ICMP6_DST_UNREACH_NOROUTE	0
#define	ICMP6_DST_UNREACH_ADMIN		1
#define	ICMP6_DST_UNREACH_BEYONDSCOPE	2
#define	ICMP6_DST_UNREACH_ADDR		3
#define	ICMP6_DST_UNREACH_NOPORT	4

#define	ICMP6_TIME_EXCEED_TRANSIT	0
#define	ICMP6_TIME_EXCEED_REASSEMBLY	1

#define	ICMP6_PARAMPROB_HEADER		0
#define	ICMP6_PARAMPROB_NEXTHEADER	1
#define	ICMP6_PARAMPROB_OPTION		2


#define	ND_ROUTER_SOLICIT		133
#define	ND_ROUTER_ADVERT		134
#define	ND_NEIGHBOR_SOLICIT		135
#define	ND_NEIGHBOR_ADVERT		136
#define	ND_REDIRECT			137

/*
 * Structure of icmpv6 router solicitation header
 */
struct nd_router_solicit {
	struct	icmp6_hdr nd_rs_hdr;
	/* variable length extensions */
};
#define	nd_rs_type	nd_rs_hdr.icmp6_type
#define	nd_rs_code	nd_rs_hdr.icmp6_code
#define	nd_rs_cksum	nd_rs_hdr.icmp6_cksum
#define	nd_rs_reserved	nd_rs_hdr.icmp6_data32[0]

/*
 * Structure of icmpv6 router advertisement header
 */
struct nd_router_advert {
	struct	icmp6_hdr nd_ra_hdr;
	uint32_t	nd_ra_router_reachable;
	uint32_t	nd_ra_router_retrans;
	
	/* variable length extensions */
};
#define	nd_ra_type		nd_ra_hdr.icmp6_type
#define	nd_ra_code		nd_ra_hdr.icmp6_code
#define	nd_ra_cksum		nd_ra_hdr.icmp6_cksum
#define	nd_ra_currhoplimit	nd_ra_hdr.icmp6_data8[0]
#define	nd_ra_flags_reserved	nd_ra_hdr.icmp6_data8[1]
#define	ND_RA_FLAG_MANAGED	0x80
#define	ND_RA_FLAG_OTHER	0x40
#define	ND_RA_FLAG_HOMEAGENT	0x20
#define	nd_ra_router_lifetime	nd_ra_hdr.icmp6_data16[1]

/*
 * Structure of icmpv6 neighbor solicitation header
 */
struct nd_neighbor_solicit {
	struct	icmp6_hdr nd_ns_hdr;
	struct	in6_addr nd_ns_target;
	/* variable length extentions */
};
#define	nd_ns_type	nd_ns_hdr.icmp6_type
#define	nd_ns_code	nd_ns_hdr.icmp6_code
#define	nd_ns_cksum	nd_ns_hdr.icmp6_cksum
#define	nd_ns_reserved	nd_ns_hdr.icmp6_reserved

/*
 * Structure of icmpv6 neighbor advertisement header
 */
struct nd_neighbor_advert {
	struct	icmp6_hdr nd_na_hdr;
	struct	in6_addr nd_na_target;
	/* variable length extentions */
};
#define	nd_na_type	nd_na_hdr.icmp6_type
#define	nd_na_code	nd_na_hdr.icmp6_code
#define	nd_na_cksum	nd_na_hdr.icmp6_cksum
#define	nd_na_flags_reserved	nd_na_hdr.icmp6_data8[0]
#define	ND_NA_FLAG_ROUTER	0x80
#define	ND_NA_FLAG_SOLICITED	0x40
#define	ND_NA_FLAG_OVERRIDE	0x20

/*
 * Structure of icmpv6 redirect header
 */
struct nd_redirect {
	struct	icmp6_hdr nd_rd_hdr;
	struct	in6_addr nd_rd_target;
	struct	in6_addr nd_rd_dst;
	/* variable length extensions */
};
#define	nd_rd_type	nd_rd_hdr.icmp6_type
#define	nd_rd_code	nd_rd_hdr.icmp6_code
#define	nd_rd_cksum	nd_rd_hdr.icmp6_cksum
#define	nd_rd_reserved	nd_rd_hdr.icmp6_data32[0]

/*
 * Struct of icmpv6 generic extension header
 */
struct nd_opt_hdr {
	uint8_t	nd_opt_type;
	uint8_t	nd_opt_len;
	/* variable length option specific data */
};

#define	ND_OPT_SOURCE_LINKADDR		1
#define	ND_OPT_TARGET_LINKADDR		2
#define	ND_OPT_PREFIX_INFORMATION	3
#define	ND_OPT_REDIRECTED_HEADER	4
#define	ND_OPT_MTU			5
#define	ND_OPT_INTERVAL			7
#define	ND_OPT_HA_INFO			8

/*
 * Structure of icmpv6 prefix information extension header
 */
struct nd_opt_prefix_info {
	uint8_t	nd_opt_pi_type;
	uint8_t	nd_opt_pi_len;
	uint8_t	nd_opt_pi_prefix_len;
	uint8_t	nd_opt_pi_flags_reserved;
	uint32_t nd_opt_pi_valid_time;
	uint32_t nd_opt_pi_preferred_time;
	uint32_t nd_opt_pi_reserved2;
	uint32_t nd_opt_prefix[4];
};

#define	ND_OPT_PI_FLAG_ONLINK	0x80
#define	ND_OPT_PI_FLAG_AUTO	0x40
#define	ND_OPT_PI_FLAG_ROUTER	0x20

/*
 * Structure of icmpv6 ND Home Agent Information option on router advertisments
 */
struct nd_opt_ha_info {
	uint8_t	nd_opt_hi_type;
	uint8_t	nd_opt_hi_len;
	uint8_t	nd_opt_hi_reserved[2];
	uint16_t nd_opt_hi_preference;
	uint16_t nd_opt_hi_lifetime;
};

/*
 * Structure of icmpv6 ND Router Advertisement Interval option
 */
struct nd_opt_advert_interval {
	uint8_t	nd_opt_ai_type;
	uint8_t	nd_opt_ai_len;
	uint8_t	nd_opt_ai_reserved[2];
	uint32_t nd_opt_ai_interval;
};

/*
 * Structure of icmpv6 redirected header extension header
 */
struct nd_opt_rd_hdr {
	uint8_t	nd_opt_rh_type;
	uint8_t	nd_opt_rh_len;
	uint16_t nd_opt_rh_reserved1;
	uint16_t nd_opt_rh_reserved2;
	/* variable length IPv6 header and data */
};

/*
 * Structure of icmpv6 MTU extension header
 */
struct nd_opt_mtu {
	uint8_t	nd_opt_mtu_type;
	uint8_t	nd_opt_mtu_len;
	uint8_t	nd_opt_mtu_reserved;
	uint8_t	nd_opt_mtu_mtu;
};

/*
 * Structure of icmpv6 multicast listener discovery header
 */
struct mld_hdr {
	struct	icmp6_hdr mld_icmp6_hdr;
	struct	in6_addr mld_addr;
};
#define	mld_type	mld_icmp6_hdr.icmp6_type
#define	mld_code	mld_icmp6_hdr.icmp6_code
#define	mld_cksum	mld_icmp6_hdr.icmp6_cksum
#define	mld_maxdelay	mld_icmp6_hdr.icmp6_data16[0]
#define	mld_reserved	mld_icmp6_hdr.icmp6_data16[1]

#define	MLD_MEMBERSHIP_QUERY		130
#define	MLD_MEMBERSHIP_REPORT		131
#define	MLD_MEMBERSHIP_REDUCTION	132

/*
 * Structur of icmpv6 router renumbering header
 */
struct icmp6_router_renum {
	struct	icmp6_hdr rr_hdr;
	uint8_t	rr_segnum;
	uint8_t	rr_flags;
	uint8_t	rr_maxdelay;
	uint8_t	rr_reserved;
};
#define	rr_type		rr_hdr.icmp6_type
#define	rr_code		rr_hdr.icmp6_code
#define	rr_cksum	rr_hdr.icmp6_cksum
#define	rr_seqnum	rr_hdr.icmp6_date32[0]

#define	ICMP6_RR_FLAGS_TEST		0x80
#define	ICMP6_RR_FLAGS_REQRESULT	0x40
#define	ICMP6_RR_FLAGS_FORCEAPPLY	0x20
#define	ICMP6_RR_FLAGS_SPECSITE		0x10
#define	ICMP6_RR_FLAGS_PREVDONE		0x08

struct rr_pco_match {
	uint8_t	rpm_code;
	uint8_t	rpm_len;
	uint8_t	rpm_ordinal;
	uint8_t	rpm_matchlen;
	uint8_t	rpm_minlen;
	uint8_t	rpm_maxlen;
	uint8_t	rpm_reserved;
	struct	in6_addr rpm_prefix;
};

#define	RPM_PCO_ADD		1
#define	RPM_PCO_CHANGE		2
#define	RPM_PCO_SETGLOBAL	3

struct rr_pco_use {
	uint8_t	rpu_uselen;
	uint8_t	rpu_keeplen;
	uint8_t	rpu_ramask;
	uint8_t	rpu_raflags;
	uint8_t	rpu_vltime;
	uint8_t	rpu_pltime;
	uint8_t	rpu_flags;
	struct	in6_addr rpu_prefix;
};

#define	ICMP6_RR_PCOUSE_RAFLAGS_ONLINK	0x20
#define	ICMP6_RR_PCOUSE_RAFLAGS_AUTO	0x10

#if BYTE_ORDER == BIG_ENDIAN
#define	ICMP6_RR_RESULT_FLAGS_OOB	0x0002
#define	ICMP6_RR_RESULT_FLAGS_FORBIDDEN	0X0001
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define	ICMP6_RR_RESULT_FLAGS_OOB	0x0200
#define	ICMP6_RR_RESULT_FLAGS_FORBIDDEN	0X0100
#endif

/*
 * Structure used by ICMP6_FILTER, IPPROTO_ICMPV6 level socket option.
 */
struct icmp6_filter {
	uint32_t icmp6_filt[8];
};

#define	ICMP6_FILTER_SETBLOCK(t, f)					\
	((void)((f)->icmp6_filt[(t) >> 5] &= ~(1 << ((t) & 31))))

#define	ICMP6_FILTER_SETBLOCKALL(f)					\
	((void)((f)->icmp6_filt[0] = 0, (f)->icmp6_filt[1] = 0		\
	, (f)->icmp6_filt[2] = 0, (f)->icmp6_filt[3] = 0		\
	, (f)->icmp6_filt[4] = 0, (f)->icmp6_filt[5] = 0		\
	, (f)->icmp6_filt[6] = 0, (f)->icmp6_filt[7] = 0))

#define	ICMP6_FILTER_SETPASS(t, f)					\
	((void)((f)->icmp6_filt[(t) >> 5] |= (1 << ((t) & 31))))

#define	ICMP6_FILTER_SETPASSALL(f)					\
	((void)((f)->icmp6_filt[0] = 0xffffffff				\
	, (f)->icmp6_filt[1] = 0xffffffff, (f)->icmp6_filt[2] = 0xffffffff \
	, (f)->icmp6_filt[3] = 0xffffffff, (f)->icmp6_filt[4] = 0xffffffff \
	, (f)->icmp6_filt[5] = 0xffffffff, (f)->icmp6_filt[6] = 0xffffffff \
	, (f)->icmp6_filt[7] = 0xffffffff))

#define	ICMP6_FILTER_WILLBLOCK(t, f)					\
	(((f)->icmp6_filt[(t) >> 5] & (1 << ((t) & 31))) == 0)

#define	ICMP6_FILTER_WILLPASS(t, f)					\
	(((f)->icmp6_filt[(t) >> 5] & (1 << ((t) & 31))) != 0)

/*
 * Options for use with [gs]etsockopt at the ICMPV6 level.
 * First word of comment is data type; bool is stored in int.
 */
#define	ICMP6_FILTER	1   /* icmp6_filter; set/get icmpv6 type filter */

#ifdef __cplusplus
}
#endif

#endif	/* _ICMP6_ */
