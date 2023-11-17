/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: inet.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ARPA_INET_H_
#define	_ARPA_INET_H_

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Standard network functions */
in_addr_t		inet_addr(const char *);
const char	*	inet_ntoa(struct in_addr);
const char *	inet_ntop(int, const void *, char *, size_t);
int				inet_pton(int, const char *, void *);

/* Non-standard network functions */
int				inet_aton(const char *, struct in_addr *);
in_addr_t		inet_lnaof(struct in_addr);
struct in_addr	inet_makeaddr(int, int);
in_addr_t		inet_netof(struct in_addr);
in_addr_t		inet_network(const char *);

#ifdef __cplusplus
}
#endif

#endif /* !_ARPA_INET_H_ */
