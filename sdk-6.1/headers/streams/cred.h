/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: cred.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/
#ifndef _CRED_
#define _CRED_

#ifdef MI_H_ID_STRINGS
static char	cred_h_sccsid[] = "@(#)cred.h	43.1";
#endif

#include <sys/types.h>

typedef struct cred {
	ushort	cr_ref;		/* reference count on processes using cred structures */
	ushort	cr_ngroups;	/* number of groups in cr_groups */
	uid_t	cr_uid;		/* effective user id */
	gid_t	cr_gid;		/* effective group id */
	uid_t	cr_ruid;	/* real user id */
	gid_t	cr_rgid;	/* real group id */
	uid_t	cr_suid;	/* user id saved by exec */
	gid_t	cr_sgid;	/* group id saved by exec */
	gid_t	cr_groups[1];	/* supplementary groups list */
} cred_t;

#ifdef __cplusplus
extern "C" {
#endif

extern	int	drv_priv(cred_t * credp);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif
