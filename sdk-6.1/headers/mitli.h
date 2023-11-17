/** Copyright (c) 1996-2002  Mentat Inc.
 ** mitli.h 43.2, last change 07/12/02
 **/



#ifndef _MITLI_
#define _MITLI_

#include <streams/types.h>

#ifdef MI_H_ID_STRINGS
static char	mitli_h_sccsid[] = "@(#)mitli.h	43.2";
#endif

/*
 * Remainder based on xti.h from X/Open Transport Interface (XTI) 1992,
 * Appendix F.
 */

/* Error values */
#define	TBADADDR	1
#define	TBADOPT		2
#define	TACCES		3
#define	TBADF		4
#define	TNOADDR		5
#define	TOUTSTATE	6
#define	TBADSEQ		7
#define TSYSERR		8
#define	TLOOK		9
#define	TBADDATA	10
#define	TBUFOVFLW	11
#define	TFLOW		12
#define	TNODATA		13
#define	TNODIS		14
#define	TNOUDERR	15
#define	TBADFLAG	16
#define	TNOREL		17
#define	TNOTSUPPORT	18
#define	TSTATECHNG	19

/* additional XTI errors */
#define TNOSTRUCTYPE	20
#define TBADNAME	21
#define	TBADQLEN	22
#define	TADDRBUSY	23
#define TINDOUT		24
#define TPROVMISMATCH	25
#define TRESQLEN	26
#define TRESADDR	27
#define TQFULL		28
#define TPROTO		29

/* t_look events */
#define	T_LISTEN	0x0001
#define	T_CONNECT	0x0002
#define	T_DATA		0x0004
#define	T_EXDATA	0x0008
#define	T_DISCONNECT	0x0010
#define T_ERROR		0x0020		/* obsolete TLI fatal error */
#define	T_UDERR		0x0040
#define	T_ORDREL	0x0080
#define	T_GODATA	0x0100
#define	T_GOEXDATA	0x0200
#define	T_EVENTS	0x00FF	/* obsolete TLI event mask--value per SVR4 */

/* Flag definitions */
#define	T_MORE		0x001
#define	T_EXPEDITED	0x002
#define	T_NEGOTIATE	0x004
#define	T_CHECK		0x008
#define	T_DEFAULT	0x010
#define T_SUCCESS	0x020
#define	T_FAILURE	0x040
#define T_CURRENT	0x080
#define T_PARTSUCCESS	0x100
#define T_READONLY	0x200
#define T_NOTSUPPORT	0x400

struct t_info {
	long	addr;
	long	options;
	long	tsdu;
	long	etsdu;
	long	connect;
	long	discon;
	long	servtype;
#ifdef TLI_XPG4_TINFO
	long	flags;		/* XTI version of TPI PROVIDER_flags field */
#define	T_SENDZERO	0x001	/* supports 0-length TSDU's */
#endif
};

/* Service types */
#ifndef	T_COTS
#define T_COTS		01	/* Connection-mode service */
#endif
#ifndef	T_COTS_ORD
#define	T_COTS_ORD	02	/* Connection service with orderly release */
#endif
#ifndef	T_CLTS
#define	T_CLTS		03	/* Connectionless-mode service */
#endif

struct netbuf {
	unsigned int	maxlen;
	unsigned int	len;
	char		* buf;
};

struct t_opthdr {
	unsigned long	len;	/* total length of option */
				/* = sizeof(struct t_opthdr) + length	*/
				/*   of option value in bytes		*/
	unsigned long	level;	/* protocol affected */
	unsigned long	name;	/* option name */
	unsigned long	status;	/* status value */
	/* followed by the option value */
};

struct t_bind {
	struct netbuf	addr;
	unsigned	qlen;
};

struct t_optmgmt {
	struct netbuf	opt;
	long		flags;
};

struct t_discon {
	struct netbuf	udata;
	int		reason;
	int		sequence;
};

struct t_call {
	struct netbuf	addr;
	struct netbuf	opt;
	struct netbuf	udata;
	int		sequence;
};

struct t_unitdata {
	struct netbuf	addr;
	struct netbuf	opt;
	struct netbuf	udata;
};

struct t_uderr {
	struct netbuf	addr;
	struct netbuf	opt;
	long		error;
};

/* t_alloc structure types */
#define	T_BIND		1
#define T_OPTMGMT	2
#define	T_CALL		3
#define	T_DIS		4
#define	T_UNITDATA	5
#define	T_UDERROR	6
#define	T_INFO		7

/* t_alloc field identifiers */
#define	T_ADDR		0x01
#define	T_OPT		0x02
#define	T_UDATA		0x04
#ifdef USE_XTIMODE
#define	T_ALL		0xffff	/* it's what XTI sez */
#else
#define	T_ALL		0x07	/* it's what SVR4 sez */
#endif

/* State values */
#define T_UNINIT	0	/* addition to standard xti.h */
#define	T_UNBND		1	/* unbound */
#define	T_IDLE		2	/* idle */
#define	T_OUTCON	3	/* outgoing connection pending */
#define	T_INCON		4	/* incoming connection pending */
#define	T_DATAXFER	5	/* data transfer */
#define	T_OUTREL	6	/* outgoing orderly release */
#define	T_INREL		7	/* incoming orderly release */

/* general purpose defines */
#define	T_YES		1
#define	T_NO		0
#define	T_UNUSED	-1
#define	T_NULL		0
#define	T_ABSREQ	0x8000
#define T_INFINITE	-1	/* for t_info req/ack */
#define T_INVALID	-2	/* ditto */

/* for options management */
#ifndef	T_UNSPEC
#define T_UNSPEC	(~0 - 2)
#endif
#define T_ALLOPT	0
#define T_ALIGN(p)	(((unsigned long)(p)+(sizeof(long)-1)) & ~(sizeof(long)-1))
#define OPT_NEXTHDR(pbuf, buflen, popt)	\
		(((char *)(popt)+T_ALIGN((popt)->len) < (char *)(pbuf)+buflen)	   \
		? (struct t_opthdr *)((char *)(popt)+T_ALIGN((popt)->len))  \
		: (struct t_opthdr *)NULL)

/* Options on XTI level */
#define XTI_GENERIC	0xffff

#define XTI_DEBUG		0x0001
#define XTI_LINGER		0x0080
#define XTI_RCVBUF		0x1002
#define XTI_RCVLOWAT		0x1004
#define XTI_SNDBUF		0x1001
#define XTI_SNDLOWAT		0x1003

/* Structure used with linger option */
struct t_linger {
	long	l_onoff;	/* option on/off */
	long	l_linger;	/* linger time */
};

/* ISO definitions */
#define	T_CLASS0	0
#define	T_CLASS1	1
#define	T_CLASS2	2
#define	T_CLASS3	3
#define	T_CLASS4	4

/* priorities */
#define	T_PRITOP	0
#define	T_PRIHIGH	1
#define	T_PRIMID	2
#define	T_PRILOW	3
#define	T_PRIDFLT	4

/* protection levels */
#define	T_NOPROTECT		1
#define	T_PASSIVEPROTECT	2
#define	T_ACTIVEPROTECT		4

/* default value for the length of TPDU's */
#define	T_LTPDUDFLT	128

/* rate structure */
struct rate {
	long	targetvalue;
	long	minacceptvalue;
};

/* reqvalue structure */
struct reqvalue {
	struct rate	called;
	struct rate	calling;
};

/* throughput structure */
struct thrpt {
	struct reqvalue	maxthrpt;
	struct reqvalue	avgthrpt;
};

/* transit delay structure */
struct transdel {
	struct reqvalue	maxdel;
	struct reqvalue	avgdel;
};

/*
 * Protocol Levels
 */

/*
 * ISO QOS definitions 
 */
#define TCO_THROUGHPUT		0x0001
#define TCO_TRANSDEL		0x0002
#define TCO_RESERRORRATE	0x0003
#define TCO_TRANSFFAILPROB	0x0004
#define TCO_ESTFAILPROB		0x0005
#define TCO_RELFAILPROB		0x0006
#define TCO_ESTDELAY		0x0007
#define TCO_RELDELAY		0x0008
#define TCO_CONNRESIL		0x0009
#define TCO_PROTECTION		0x000a
#define TCO_PRIORITY		0x000b
#define TCO_EXPD		0x000c

#define TCL_TRANSDEL		0x000d
#define TCL_RESERRORRATE	TCO_RESERRORRATE
#define TCL_PROTECTION		TCO_PROTECTION
#define TCL_PRIORITY		TCO_PRIORITY

#define TCO_LTPDU		0x0100
#define TCO_ACKTIME		0x0200
#define TCO_REASTIME		0x0300
#define TCO_EXTFORM		0x0400
#define TCO_FLOWCTRL		0x0500
#define TCO_CHECKSUM		0x0600
#define TCO_NETEXP		0x0700
#define TCO_NETRECPTCF		0x0800
#define TCO_PREFCLASS		0x0900
#define TCO_ALTCLASS1		0x0a00
#define TCO_ALTCLASS2		0x0b00
#define TCO_ALTCLASS3		0x0c00
#define TCO_ALTCLASS4		0x0d00

#define TCL_CHECKSUM		TCO_CHECKSUM

/*
 * TCP Level
 */
#define INET_TCP	0x06
 
/* TCP-level Options */
#ifndef TCP_NODELAY
#define TCP_NODELAY	0x01
#endif
#ifndef TCP_MAXSEG
#define TCP_MAXSEG	0x02
#endif
#ifndef TCP_KEEPALIVE
#define TCP_KEEPALIVE	0x08
#endif

struct t_kpalive {
	long	kp_onoff;
	long	kp_timeout;
};
#define	T_GARBAGE	0x02

/*
 * UDP Level
 */
#define INET_UDP	0x11

/* UDP-level Options */
#ifndef UDP_CHECKSUM
#define UDP_CHECKSUM	TCO_CHECKSUM
#endif
/*
 * IP Level
 */
#define INET_IP	0x0

/* IP-level Options */
#ifndef IP_OPTIONS
#define	IP_OPTIONS	0x01
#endif
#ifndef IP_TOS
#define IP_TOS		0x02
#endif
#ifndef IP_TTL
#define	IP_TTL		0x03
#endif
#ifndef	IP_REUSEADDR
#define IP_REUSEADDR	0x04
#endif
#ifndef	IP_DONTROUTE
#define	IP_DONTROUTE	0x10
#endif
#ifndef	IP_BROADCAST
#define IP_BROADCAST	0x20
#endif

/* IP_TOS precdence levels */
#define T_ROUTINE	0
#define	T_PRIORITY	1
#define T_IMMEDIATE	2
#define T_FLASH		3
#define T_OVERRIDEFLASH	4
#define T_CRITIC_ECP	5
#define	T_INETCONTROL	6
#define T_NETCONTROL	7

/* IP_TOS type of service */
#define T_NOTOS		0
#define T_LDELAY	(1<<4)
#define T_HITHRPT	(1<<3)
#define T_HIREL		(1<<2)

#define	SET_TOS(prec,tos)	(((0x7 & (prec)) << 5) | (0x1c & (tos)))

#ifdef OBSOLETE_XTI_STUFF
/* management structure */
struct management {
	short	dflt;		/* T_YES to use default values or T_NO to use values in structure */
	int	ltpdu;		/* maximum length of TPDU */
	short	reastime;	/* reassignment time (in seconds) */
	char	class;		/* preferred class */
	char	altclass;	/* alternative class */
	char	extform;	/* extended format: T_YES or T_NO */
	char	flowctrl;	/* flow control: T_YES or T_NO */
	char	checksum;
	char	netexp;		/* network expedited data */
	char	netrecptcf;	/* receipt confirmation */
};

/* ISO connection-oriented options */
struct isoco_options {
	struct thrpt	throughput;
	struct reqvalue	transdel;	/* transit delay */
	struct rate	reserrorrate;	/* residual error rate */
	struct rate	transffailprob;	/* transfer failure problem */
	struct rate	estfailprob;	/* connection establishment failure problem */
	struct rate	relfailprob;	/* connection release failure problem */
	struct rate	estdelay;	/* connection establishment delay */
	struct rate	reldelay;	/* connection release delay */
	struct netbuf	connresil;	/* connection resilience */
	unsigned short	protection;
	short		priority;
	struct management mngmt;	/* management parameters */
	char		expd;		/* expedited data: T_YES or T_NO */
};

/* ISO connectionless options */
struct isocl_options {
	struct rate	transdel;	/* transit delay */
	struct rate	reserrorrate;	/* residual error rate */
	unsigned short	protection;
	short		priority;
};

/* TCP security options structure */
struct secoptions {
	short	security;
	short	compartment;
	short	handling;	/* handling restrictions */
	long	tcc;		/* transmission control code */
};

/* TCP options */
struct tcp_options {
	short		precedence;
	long		timeout;	/* abort timeout */
	long		max_seg_size;
	struct secoptions secopt;	/* security options */
};

#endif /* OBSOLETE_XTI_STUFF */

#ifndef KERNEL

extern	int	t_errno;

#ifndef USE_XTIMODE		/* not visible with XTI */
extern	char	* t_errlist[];
extern	int	t_nerr;
#endif

extern	int	t_accept(fd_t fd, int resfd, struct t_call * call);
extern	char	* t_alloc(fd_t fd, int struct_type, int fields);
extern	int	t_bind(fd_t fd, struct t_bind * req, struct t_bind * ret);
extern	int	t_blocking(fd_t fd);
extern	int	t_close(fd_t fd);
extern	int	t_connect(fd_t fd, struct t_call * sndcall
		, struct t_call * rcvcall);
extern	int	t_free(char * ptr, int struct_type);
extern	int	t_getinfo(fd_t fd, struct t_info * info);
extern	int	t_getstate(fd_t fd);
extern	int	t_listen(fd_t fd, struct t_call * call);
extern	int	t_look(fd_t fd);
extern	int	t_nonblocking(fd_t fd);
extern	int	t_open(char * path, int	oflag, struct t_info * info);
extern	int	t_optmgmt(fd_t fd, struct t_optmgmt * req
		, struct t_optmgmt * ret);
extern	int	t_rcv(fd_t fd, char * buf, unsigned nbytes, int * flags);
extern	int	t_rcv_with_type(fd_t fd, char * buf, unsigned nbytes
		, int * flags, int * typep);
extern	int	t_rcvconnect(fd_t fd, struct t_call * call);
extern	int	t_rcvdis(fd_t fd, struct t_discon * discon);
extern	int	t_rcvrel(fd_t fd);
extern	int	t_rcvudata(fd_t fd, struct t_unitdata * unitdata, int * flags);
extern	int	t_rcvuderr(fd_t fd, struct t_uderr * uderr);
extern	int	t_snd(fd_t fd, char * buf, unsigned nbytes, int flags);
extern	int	t_snddis(fd_t fd, struct t_call * call);
extern	int	t_sndrel(fd_t fd);
extern	int	t_sndudata(fd_t fd, struct t_unitdata * unitdata);
extern	int	t_sync(fd_t fd);
extern	int	t_unbind(fd_t fd);

/* following XTI only, but put in TLI library for folks who know... */
extern	char	* t_strerror(int errnum);
extern	int	t_getprotaddr(fd_t fd, struct t_bind * boundaddr
		, struct t_bind * peeraddr); 

#ifdef USE_XTIMODE
extern	int	t_error(char * errmsg);
extern	int	tli_open(char * path, int oflag, struct t_info * info);

#else
extern	void	t_error(char * errmsg);
extern	int	xti_open(char * path, int oflag, struct t_info * info);
/* Following is nonstandard TLI version of t_getprotaddr() */
extern	int	t_getname(fd_t fd, struct netbuf * name, int type);
#define	LOCALNAME	0
#define REMOTENAME	1
#endif	/* USE_XTIMODE */

#endif /* KERNEL */

#endif	/* _MITLI_ */
