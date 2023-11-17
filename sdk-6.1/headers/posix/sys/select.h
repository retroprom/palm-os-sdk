/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: select.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef H_SELECT
#define H_SELECT

#include <sys/time.h>
#include <IOS.h>

__BEGIN_DECLS

#ifndef FD_SET
/* 
 * Select uses bit masks of file descriptors in uint32_t's.  These macros 
 * manipulate such bit fields (the filesystem macros use chars). 
 * FD_SETSIZE may be defined by the user, but the default here should 
 * be enough for most uses. 
 */ 
#ifndef        FD_SETSIZE 
#define        FD_SETSIZE      MAX_FILDES		/* IOS limit */
#endif 

typedef unsigned long   fd_mask; 
#define NFDBITS        (sizeof(fd_mask) * 8)       /* bits per mask */ 

#ifndef howmany 
#define        howmany(x, y)  (((x) + ((y) - 1)) / (y)) 
#endif 

typedef        struct fd_set { 
        fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)]; 
} fd_set; 

#define        FD_SET(n, p)  ((p)->fds_bits[(n)/NFDBITS] |= (((fd_mask)1) << ((n) % NFDBITS))) 
#define        FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(((fd_mask)1) << ((n) % NFDBITS))) 
#define        FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (((fd_mask)1) << ((n) % NFDBITS))) 
#define        FD_COPY(f, t)   memcpy(t, f, sizeof(*(f))) 
#define        FD_ZERO(p)      memset(p, 0, sizeof(*(p))) 

#endif /*FD_SET*/

int select(int fd, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout);

__END_DECLS

#endif /* H_SELECT */
