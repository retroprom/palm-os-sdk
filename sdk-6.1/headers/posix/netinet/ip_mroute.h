/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: ip_mroute.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _IP_MROUTE_
#define	_IP_MROUTE_

#include <stdint.h>
#include <netinet/in.h>

#ifdef MI_H_ID_STRINGS
static	char	ip_mroute_h_sccsid[] = "@(#)ip_mroute.h	43.1";
#endif

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1989 Stephen Deering.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Stephen Deering of Stanford University.
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
 *	@(#)ip_mroute.h	8.1 (Berkeley) 6/10/93
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
 * Definitions for the kernel part of DVMRP,
 * a Distance-Vector Multicast Routing Protocol.
 * (See RFC-1075.)
 *
 * Written by David Waitzman, BBN Labs, August 1988.
 * Modified by Steve Deering, Stanford, February 1989.
 *
 * MROUTING 1.0
 */


/*
 * Multicast Routing set/getsockopt commands.
 */
#define	MRT_INIT	100	/* initialize forwarder */
#define	MRT_DONE	101	/* shut down forwarder */
#define	MRT_ADD_VIF	102	/* create virtual interface */
#define	MRT_DEL_VIF	103	/* delete virtual interface */
#define	MRT_ADD_MFC	104	/* insert forwarding cache entry */
#define	MRT_DEL_MFC	105	/* delete forwarding cache entry */
#define	MRT_VERSION	106	/* get kernel version number */
#define	MRT_ASSERT	107	/* enable PIM assert processing */


/*
 * Types and macros for handling bitmaps with one bit per virtual interface.
 */
#define	MAXVIFS 32
typedef uint32_t vifbitmap_t;
typedef uint16_t vifi_t;		/* type of a vif index */
#define	ALL_VIFS	(vifi_t)-1

#define	VIFM_SET(n, m)		((m) |= (1 << (n)))
#define	VIFM_CLR(n, m)		((m) &= ~(1 << (n)))
#define	VIFM_ISSET(n, m)	((m) & (1 << (n)))
#define	VIFM_CLRALL(m)		((m) = 0x00000000)
#define	VIFM_COPY(mfrom, mto)	((mto) = (mfrom))
#define	VIFM_SAME(m1, m2)	((m1) == (m2))

/*
 * Agument structure for MRT_ADD_VIF.
 * (MRT_DEL_VIF takes a single vifi_t argument.)
 */
struct vifctl {
	vifi_t		vifc_vifi;    	/* the index of the vif to be added */
	uint8_t		vifc_flags;    	/* VIFF_ flags defined below */
	uint8_t		vifc_threshold;	/* min ttl required to forward on vif */
	uint32_t	vifc_rate_limit;/* max rate */
	struct in_addr	vifc_lcl_addr;	/* local interface address */
	struct in_addr	vifc_rmt_addr;	/* remote address (tunnels only) */
};

#define	VIFF_TUNNEL	0x1		/* vif represents a tunnel end-point */
#define	VIFF_SRCRT	0x2		/* tunnel uses IP src routing */


/*
 * Argument structure for MRT_ADD_MFC and MRT_DEL_MFC.
 * (mfcc_tos to be added at a future point)
 */
struct mfcctl {
	struct in_addr	mfcc_origin;		/* ip origin of mcasts */
	struct in_addr	mfcc_mcastgrp;		/* multicast group associated */
	vifi_t		mfcc_parent;		/* incoming vifs */
	uint16_t	__pad;
	uint8_t		mfcc_ttls[MAXVIFS];	/* forwarding ttls on vifs */
};

/*
 * Argument structure used by mrouted to get src-grp pkt counts
 */
struct sioc_sg_req {
	struct in_addr src;
	struct in_addr grp;
	uint32_t pktcnt;
	uint32_t bytecnt;
	uint32_t wrong_if;
};

/*
 * Argument structure used by mrouted to get vif pkt counts
 */
struct sioc_vif_req {
	vifi_t vifi;			/* vif number */
	uint16_t __pad;
	uint32_t icount;		/* Input packet count on vif */
	uint32_t ocount;		/* Output packet count on vif */
	uint32_t ibytes;		/* Input byte count on vif */
	uint32_t obytes;		/* Output byte count on vif */
};

/*
 * Struct used to communicate from kernel to multicast router.
 * Note the convenient similarity to an IP packet.
 */
struct igmpmsg {
        uint32_t unused1;
        uint32_t unused2;
        uint8_t	im_msgtype;	/* what type of message */
#define IGMPMSG_NOCACHE		1
#define IGMPMSG_WRONGVIF	2
        uint8_t	im_mbz;		/* must be zero */
        uint8_t	im_vif;		/* vif rec'd on */
        uint8_t	unused3;
        struct in_addr im_src;
        struct in_addr im_dst;
};

/* Multicast route hash information. */
#define	MFCTBLSIZ	256
#if (MFCTBSIZ & (MFCTBLSIZ - 1)) == 0
#define	MFCHASHMOD(h)	((h) & (MFCTBLSIZ - 1))
#else
#define	MFCHASHMOD(h)	((h) % (MFCTBLSIZ))
#endif
#define	MFCHASH(a, g) \
	MFCHASHMOD((((uint32_t)(a)) >> 20) ^ (((uint32_t)(a)) >> 10) \
	^ ((uint32_t)(a)) ^ (((uint32_t)(g)) >> 20) ^ (((uint32_t)(g)) \
	>> 10) ^ ((uint32_t)(g)))

#ifdef __cplusplus
}
#endif

#endif	/* _IP_MROUTE_ */
