#ifndef _TCP_
#define	_TCP_

#include <stdint.h>
#include <sys/endian.h>

#ifdef MI_H_ID_STRINGS
static	char	tcp_h_sccsid[] = "@(#)tcp.h	43.1";
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
 *
 *	@(#)tcp.h	8.1 (Berkeley) 6/10/93
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
typedef	uint32_t	tcp_seq;
/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
	uint16_t th_sport;		/* source port */
	uint16_t th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
#if BYTE_ORDER == LITTLE_ENDIAN
	uint8_t	th_x2:4,		/* (unused) */
		th_off:4;		/* data offset */
#endif
#if BYTE_ORDER == BIG_ENDIAN
	uint8_t	th_off:4,		/* data offset */
		th_x2:4;		/* (unused) */
#endif
	uint8_t	th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
	uint16_t th_win;		/* window */
	uint16_t th_sum;		/* checksum */
	uint16_t th_urp;		/* urgent pointer */
};

#define	TCPOPT_EOL		0
#define	TCPOPT_NOP		1
#define	TCPOPT_MAXSEG		2
#define    TCPOLEN_MAXSEG		4
#define TCPOPT_WINDOW		3
#define    TCPOLEN_WINDOW		3
#define TCPOPT_SACK_PERMITTED	4		/* Experimental */
#define    TCPOLEN_SACK_PERMITTED	2
#define TCPOPT_SACK		5		/* Experimental */
#define TCPOPT_TIMESTAMP	8
#define    TCPOLEN_TIMESTAMP		10
#define    TCPOLEN_TSTAMP_APPA		(TCPOLEN_TIMESTAMP+2) /* appendix A */

#define TCPOPT_TSTAMP_HDR	\
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

/*
 * Default maximum segment size for TCP.
 * With an IP MSS of 576, this is 536,
 * but 512 is probably more convenient.
 * This should be defined as MIN(512, IP_MSS - sizeof (struct tcpiphdr)).
 */
#define	TCP_MSS	512

#define	TCP_MAXWIN	65535	/* largest value for (unscaled) window */

#define TCP_MAX_WINSHIFT	14	/* maximum window shift */

/* Structure and defines used by TCP_KEEPALIVE option. */
struct kpalive {
	int32_t	kp_onoff;
#define	KP_OFF		0x0
#define	KP_ON		0x1
#define	KP_GARBAGE	0x2
#define	KP_UNSPEC	(~0 - 2)
	int32_t	kp_timeout;
};

/*
 * User-settable options (used with setsockopt).
 */
#ifndef TCP_NODELAY
#define	TCP_NODELAY		0x01 /* bool; don't delay send */
#endif
#ifndef TCP_MAXSEG
#define	TCP_MAXSEG		0x02 /* int; get max segment size */
#endif
#ifndef	TCP_KEEPALIVE
#define	TCP_KEEPALIVE		0x08 /* kpalive; get/set keepalive time */
#endif
#define	TCP_NOTIFY_THRESHOLD	0x10 /* int; get/set notify time */
#define	TCP_ABORT_THRESHOLD	0x11 /* int; get/set abort time */
#define	TCP_CONN_NOTIFY_THRESHOLD 0x12 /* int; get/set connect notify time */
#define	TCP_CONN_ABORT_THRESHOLD 0x13 /* int; get/set connect abort time */
#define	TCP_OOBINLINE		0x14 /* bool; get/set OOB inline */
#define	TCP_URGENT_PTR_TYPE	0x15 /* bool; get/set URG type 0==new, 1==old */
#define	TCP_CWND_INIT		0x16 /* int; get/set initial congestion wind. */

#ifdef __cplusplus
}
#endif

#endif	/* _TCP_ */
