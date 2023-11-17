/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: if_nd.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _IF_ND_
#define	_IF_ND_

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#ifdef MI_H_ID_STRINGS
static	char	if_nd_h_sccsid[] = "@(#)if_nd.h	43.3";
#endif

#ifdef __cplusplus
extern	"C" {
#endif

/*
 * Copyright (c) 1986, 1993
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
 * ND ioctl request
 */
struct ndreq {
	struct	sockaddr_storage nd_pa;	/* protocol address */
	struct	sockaddr_storage nd_ha;	/* hardware address */
					/* hardware address padding */
	uint8_t	nd_ha_pad[256 - (sizeof(struct sockaddr_storage) \
		- offsetof(struct sockaddr, sa_data))];
	uint8_t	nd_ha_len;		/* hardware address length */
	uint8_t	__pad;
	char	nd_name[IFNAMSIZ];	/* interface */
	int32_t	nd_flags;		/* flags */
};
/* nd_flags field values */
#define	NTF_INUSE	0x01	/* entry in use */
#define	NTF_COM		0x02	/* entry complete (enaddr valid) */
#define	NTF_PERM	0x04	/* permanent entry */
#define	NTF_PUBL	0x08	/* publish entry (respond for other host) */
#define	NTF_USETRAILERS	0x10	/* has requested trailers */
#define	NTF_LOCAL	0x20	/* local entry */
#define NTF_INCOMPLETE	0x40	/* incomplete entry */
#define	NTF_REACHABLE	0x80	/* reachable entry */
#define	NTF_STALE	0x100	/* stale entry */
#define	NTF_PROBE	0x200	/* probe entry */
#define	NTF_DELAY	0x400	/* delay entry */
#define	NTF_NODAD	0x800	/* do not do DAD for published entry */

#ifdef __cplusplus
}
#endif

#endif	/* _IF_ND_ */
