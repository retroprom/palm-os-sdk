/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: netdb.h
 *
 *****************************************************************************/

#ifndef _NETDB_H_
#define _NETDB_H_

#include <sys/types.h>

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in an unsigned long -- probably a poor one.
 */
struct	netent {
	char		*n_name;	/* official name of net */
	char		**n_aliases;	/* alias list */
	int		n_addrtype;	/* net address type */
	unsigned long	n_net;		/* network # */
};

struct	servent {
	char	*s_name;	/* official service name */
	char	**s_aliases;	/* alias list */
	int	s_port;		/* port # */
	char	*s_proto;	/* protocol to use */
};

struct	protoent {
	char	*p_name;	/* official protocol name */
	char	**p_aliases;	/* alias list */
	int	p_proto;	/* protocol # */
};

struct	addrinfo {
	int		ai_flags;	/* AI_PASSIVE, AI_CANONNAME */
	int		ai_family;	/* PF_xxx */
	int		ai_socktype;	/* SOCK_xxx */
	int		ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
	size_t		ai_addrlen;	/* length of ai_addr */
	char		*ai_canonname;	/* canonical name for hostname */
	struct sockaddr	*ai_addr; 	/* binary address */
	struct addrinfo	*ai_next; 	/* next structure in linked list */
};

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	NETDB_INTERNAL	-1	/* see errno */
#define	NETDB_SUCCESS	0	/* no problem */
#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN	2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY	3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA		4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS	NO_DATA		/* no address, look for MX record */

/*
 * Error return codes from getaddrinfo()
 */

#define	EAI_ADDRFAMILY	 1	/* address family for hostname not supported */
#define	EAI_AGAIN	 2	/* temporary failure in name resolution */
#define	EAI_BADFLAGS	 3	/* invalid value for ai_flags */
#define	EAI_FAIL	 4	/* non-recoverable failure in name resolution */
#define	EAI_FAMILY	 5	/* ai_family not supported */
#define	EAI_MEMORY	 6	/* memory allocation failure */
#define	EAI_NODATA	 7	/* no address associated with hostname */
#define	EAI_NONAME	 8	/* hostname nor servname provided, or not known */
#define	EAI_SERVICE	 9	/* servname not supported for ai_socktype */
#define	EAI_SOCKTYPE	10	/* ai_socktype not supported */
#define	EAI_SYSTEM	11	/* system error returned in errno */
#define EAI_BADHINTS	12
#define EAI_PROTOCOL	13
#define EAI_MAX		14

/*
 * Flag values for getaddrinfo()
 */
#define	AI_PASSIVE	0x00000001
#define	AI_CANONNAME	0x00000002
#define AI_NUMERICHOST	0x00000004
#define AI_PREFER_SRC_HOME	0x00000008	/* not implemented */
#define AI_PREFER_SRC_COA	0x00000010	/* not implemented */
#define AI_PREFER_SRC_TMP	0x00000020	/* not implemented */
#define AI_PREFER_SRC_PUBLIC	0x00000040	/* not implemented */
#define AI_SELCOMPAT	0xf8000000
#define AI_SELENHANCED(cat)						\
	((6 * (cat)[0] +						\
	  2 * ((cat)[0] > (cat)[1]? (cat)[1] : ((cat)[1] - 1)) +	\
	  1 * ((cat)[2] < (cat)[3]? 0 : 1) + 1)				\
	  << 27)
/* 5 bits from MSB. Values 1,2,..,23,24,31 and 0 are used. */
#define AI_SELMASK	0xf8000000
#define	AI_MASK		(0x0000007f|AI_SELMASK)

/*
 * Flag values for getipnodebyname()
 */
#define AI_V4MAPPED	0x00000008
#define AI_ALL		0x00000010
#define AI_ADDRCONFIG	0x00000020
#define AI_DEFAULT	(AI_V4MAPPED|AI_ADDRCONFIG)

/*
 * Constants for getnameinfo()
 */
#define	NI_MAXHOST	1025
#define	NI_MAXSERV	32

/*
 * Flag values for getnameinfo()
 */
#define	NI_NOFQDN	0x00000001
#define	NI_NUMERICHOST	0x00000002
#define	NI_NAMEREQD	0x00000004
#define	NI_NUMERICSERV	0x00000008
#define	NI_DGRAM	0x00000010
#define	NI_WITHSCOPEID	0x00000020
#define NI_NUMERICSCOPE	0x00000040

/*
 * Scope delimit character
 */
#define SCOPE_DELIMITER	'%'

#ifdef __cplusplus
extern "C" {
#endif

extern int		*__h_errno(void);
#define	h_errno (*__h_errno())

struct hostent	*gethostent(void);
struct hostent	*gethostbyaddr(const char *, int, int);
struct hostent	*gethostbyname(const char *);
struct hostent	*gethostbyname2(const char *, int);
void			sethostent(int); 
void			endhostent(void);

struct netent	*getnetent(void);
struct netent	*getnetbyaddr(unsigned long, int);
struct netent	*getnetbyname(const char *);
void			setnetent(int);
void			endnetent(void);

struct protoent	*getprotoent(void);
struct protoent	*getprotobyname(const char *);
struct protoent	*getprotobynumber(int);
void			setprotoent(int);
void			endprotoent(void);

struct servent	*getservent(void);
struct servent	*getservbyname(const char *, const char *);
struct servent	*getservbyport(int, const char *);
void			setservent(int);
void			endservent(void);

const char		*hstrerror(int);

int				getaddrinfo(const char *, const char *, const struct addrinfo *, struct addrinfo **);
int				getnameinfo(const struct sockaddr *, size_t, char *, size_t, char *, size_t, int);
void			freeaddrinfo(struct addrinfo *);
const char		*gai_strerror(int);

struct hostent *getipnodebyname(const char *name, int af, int flags, int *error_num);
struct hostent *getipnodebyaddr(const void *src, size_t len, int af, int *error_num);
void			freehostent(struct hostent *);

#ifdef __cplusplus
}
#endif

#endif /* !_NETDB_H_ */
