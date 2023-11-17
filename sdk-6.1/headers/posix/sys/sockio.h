
#ifndef _SOCKIO_
#define _SOCKIO_

#ifdef MI_H_ID_STRINGS
static	char	sockio_h_sccsid[] = "@(#)sockio.h	42.4";
#endif

#ifdef __cplusplus
extern	"C" {
#endif

/*-
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
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
 *	@(#)sockio.h	8.1 (Berkeley) 3/28/94
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


#include <sys/ioccom.h>

#define SIOCSHIWAT	_IOW('s',  0, int)		/* set high watermark */
#define SIOCGHIWAT	_IOR('s',  1, int)		/* get high watermark */
#define SIOCSLOWAT	_IOW('s',  2, int)		/* set low watermark */
#define SIOCGLOWAT	_IOR('s',  3, int)		/* get low watermark */
#define SIOCATMARK	_IOR('s',  7, int)		/* at oob mark? */
#define SIOCSPGRP	_IOW('s',  8, int)		/* set process group */
#define SIOCGPGRP	_IOR('s',  9, int)		/* get process group */

#define SIOCADDRT	_IOW('r', 10, struct rtentry)	/* add route */
#define SIOCDELRT	_IOW('r', 11, struct rtentry)	/* delete route */
#define SIOCFLUSHRT	_IO('r', 55)			/* flush all routes */
#define SIOCGRTENTRY	_IOWR('r', 58, struct rtreq)	/* query route */

/* IPv6 routing ioctl's. */
#define SIOCADDRTL	_IOW('r', 10, struct rtlentry)	/* add route */
#define SIOCDELRTL	_IOW('r', 11, struct rtlentry)	/* add route */
#define SIOCFLUSHRTL	_IO('r', 56)			/* flush routes */
#define SIOCGRTLENTRY	_IOWR('r', 59, struct rtlreq)	/* query route */

#define SIOCSIFADDR	_IOW('i', 12, struct ifreq)	/* set ifnet address */
#define SIOCGIFADDR	_IOWR('i',13, struct ifreq)	/* get ifnet address */
#define SIOCSIFDSTADDR	_IOW('i', 14, struct ifreq)	/* set p-p address */
#define SIOCGIFDSTADDR	_IOWR('i',15, struct ifreq)	/* get p-p address */
#define SIOCSIFFLAGS	_IOW('i', 16, struct ifreq)	/* set ifnet flags */
#define SIOCGIFFLAGS	_IOWR('i',17, struct ifreq)	/* get ifnet flags */
#define SIOCSIFMEM	_IOW('i', 18, struct ifreq)	/* set interface mem */
#define SIOCGIFMEM	_IOWR('i',19, struct ifreq)	/* get interface mem */
#define SIOCGIFCONF	_IOWR('i',20, struct ifconf)	/* get ifnet list */
#define SIOCSIFMTU	_IOW('i', 21, struct ifreq)	/* set if_mtu */
#define SIOCGIFMTU	_IOWR('i',22, struct ifreq)	/* get if_mtu */
#define SIOCGIFBRDADDR	_IOWR('i',23, struct ifreq)	/* get broadcast addr */
#define SIOCSIFBRDADDR	_IOW('i', 24, struct ifreq)	/* set broadcast addr */
#define SIOCGIFNETMASK	_IOWR('i',25, struct ifreq)	/* get net addr mask */
#define SIOCSIFNETMASK	_IOW('i', 26, struct ifreq)	/* set net addr mask */
#define SIOCGIFMETRIC	_IOWR('i',27, struct ifreq)	/* get IF metric */
#define SIOCSIFMETRIC	_IOW('i', 28, struct ifreq)	/* set IF metric */
#define SIOCGIFINDEX	_IOWR('i',29, struct ifreq)	/* get IF index */
#define SIOCSIFINDEX	_IOW('i', 30, struct ifreq)	/* set IF index */
#define SIOCGIFNAME	_IOWR('i',31, struct ifreq)	/* get IF name */
#define SIOCSIFNAME	_IOW('i', 32, struct ifreq)	/* set IF name */
#define SIOCAIFADDR	_IOWR('i', 33, struct ifaliasreq)	/* add IF alias */
#define SIOCDIFADDR	_IOW('i', 34, struct ifreq)	/* delete IF alias */
#define SIOCGIFNUM	_IOWR('i', 35, int)		/* get num of IFs */

#define SIOCGIFTUNPARMS	_IOWR('i', 39, struct iftun_parms) /* get tunnel parms */
#define SIOCSIFTUNPARMS	_IOW('i', 40, struct iftun_parms)	/* set tunnel parms */
#define SIOCSIFXFLAGS	_IOW('i', 41, struct ifreq)	/* set extended flags */
#define SIOCGIFXFLAGS	_IOWR('i',42, struct ifreq)	/* get extended flags */

/* IPv6 interface ioctl's. */
#define SIOCSIFLADDR	_IOW('i', 12, struct iflreq)	/* set ifnet addr */
#define SIOCGIFLADDR	_IOWR('i',13, struct iflreq)	/* get ifnet addr */
#define SIOCSIFLDSTADDR	_IOW('i', 14, struct iflreq)	/* set p-p address */
#define SIOCGIFLDSTADDR	_IOWR('i',15, struct iflreq)	/* get p-p address */
#define SIOCSIFLFLAGS	_IOW('i', 16, struct iflreq)	/* set ifnet flags */
#define SIOCGIFLFLAGS	_IOWR('i',17, struct iflreq)	/* get ifnet flags */
#define SIOCSIFLMEM	_IOW('i', 18, struct iflreq)	/* set if mem */
#define SIOCGIFLMEM	_IOWR('i',19, struct iflreq)	/* get if mem */
#define SIOCGIFLCONF	_IOWR('i',20, struct iflconf)	/* get ifnet list */
#define SIOCSIFLMTU	_IOW('i', 21, struct iflreq)	/* set if_mtu */
#define SIOCGIFLMTU	_IOWR('i',22, struct iflreq)	/* get if_mtu */
#define SIOCGIFLNETMASK	_IOWR('i',23, struct iflreq)	/* get ifnet mask */
#define SIOCSIFLNETMASK	_IOW('i', 24, struct iflreq)	/* set ifnet mask */
#define SIOCGIFLMETRIC	_IOWR('i',25, struct iflreq)	/* get IF metric */
#define SIOCSIFLMETRIC	_IOW('i', 26, struct iflreq)	/* set IF metric */
#define SIOCGIFLINDEX	_IOWR('i',27, struct iflreq)	/* get IF index */
#define SIOCSIFLINDEX	_IOW('i', 28, struct iflreq)	/* set IF index */
#define SIOCGIFLNAME	_IOWR('i',29, struct iflreq)	/* get IF name */
#define SIOCSIFLNAME	_IOW('i', 30, struct iflreq)	/* set IF name */
#define SIOCAIFLADDR	_IOWR('i', 31, struct iflaliasreq) /* add IF alias */
#define SIOCDIFLADDR	_IOW('i', 32, struct iflreq)	/* delete IF alias */
#define SIOCGIFLNUM	_IOWR('i', 33, int)		/* get num of IFs */

#define SIOCSARP	_IOW('i', 36, struct arpreq)	/* set arp entry */
#define SIOCGARP	_IOWR('i',37, struct arpreq)	/* get arp entry */
#define SIOCDARP	_IOW('i', 38, struct arpreq)	/* delete arp entry */

#define SIOCSND		_IOW('i', 34, struct ndreq)	/* set arp entry */
#define SIOCGND		_IOWR('i',35, struct ndreq)	/* get arp entry */
#define SIOCDND		_IOW('i', 36, struct ndreq)	/* delete arp entry */

#define SIOCSIFLXFLAGS	_IOW('i', 47, struct iflreq)	/* set extended flags */
#define SIOCGIFLXFLAGS	_IOWR('i',48, struct iflreq)	/* get extended flags */

#define SIOCADDMULTI	_IOW('i', 49, struct ifreq)	/* set m/c address */
#define SIOCDELMULTI	_IOW('i', 50, struct ifreq)	/* clr m/c address */

#define IF_UNITSEL	_IOW('i', 54, int)		/* set unit number */
#define IF_INDEXSEL	_IOW('i', 57, int)		/* set index number */

#define SIOCGETVIFCNT	_IOWR('i',60, struct sioc_vif_req)/* Get VIF Count */
#define SIOCGETSGCNT	_IOWR('i',61, struct sioc_sg_req)	/* Get SG Count */

#define	SIOCADDBINDL	_IOWR('i', 62, struct bindlentry)	/* add/mod v6 mobile binding */
#define	SIOCDELBINDL	_IOWR('i', 63, struct bindlentry)	/* delete v6 mobile binding */
#define	SIOCGETBINDL	_IOWR('i', 64, struct bindlentry)	/* get v6 mobile binding */

#define	SIOCGIFLLIFETIME	_IOWR('i', 65, struct iflreq) /* get IF lifetimes */
#define	SIOCSIFLLIFETIME	_IOWR('i', 66, struct iflreq) /* set IF lifetimes */
#define	SIOCGIFLID			_IOWR('i', 67, struct iflreq) /* get alias IF name */
#define	SIOCGIFID			_IOWR('i', 68, struct iflreq) /* get alias IF name */

#define SIOCGPTLNUM	_IOWR('a', 69, int)		/* num of PT entries */
#define SIOCGPTL	_IOWR('a', 70, struct ptlentry)	/* get policy table */
#define SIOCADDPTL	_IOW('a', 71, struct ptlentry)	/* add PT entry */
#define SIOCFLUSHPTL	_IO('a',  72)			/* flush PT */

#define SIOCGCSTLSIZE	_IOWR('a', 73, int)		/* size of CST */
#define SIOCGCSTL	_IOWR('a', 74, struct cstlentry)/* get CS table */
#define SIOCADDCSTL	_IOW('a', 75, struct cstlentry)	/* add CST entry */
#define SIOCFLUSHCSTL	_IO('a', 76)			/* flush CST */

#define SIOCGDASPREF	_IOWR('a', 77, struct daspref)	/* get DAS pref. */
#define SIOCSDASPREF	_IOW('a', 78, struct daspref)	/* set DAS pref. */

#define SIOCGDEFLSRC	_IOWR('a', 79, struct deflsrc)	/* get DAS result. */

#ifdef __cplusplus
}
#endif

#endif	/* _SOCKIO_ */
