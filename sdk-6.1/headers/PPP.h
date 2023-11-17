/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PPP.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _PPP_h_
#define _PPP_h_

#include <CmnErrors.h>

#define		pppErrUser				(pppErrorClass | 1)		/* User requested termination. */
#define		pppErrRemote			(pppErrorClass | 2)		/* Remote requested termination. */
#define		pppErrAuthFailed		(pppErrorClass | 3)		/* Authentication failed. */
#define		pppErrAuthScheme		(pppErrorClass | 4)		/* No authentication scheme available. */
#define		pppErrTimeout			(pppErrorClass | 5)		/* A timeout or retry count exceeded occured. */
#define		pppErrConfig			(pppErrorClass | 6)		/* An error occured while retrieving the configuration (Script, User, Password, Auth). */
#define		pppErrOther				(pppErrorClass | 7)		/* An error occured that is none of the previous. */
#define		pppErrOptions			(pppErrorClass | 8)		/* PPP (LCP, IPCP, ...) negociation failed. */
#define 	pppErrLinkLoopbacked	(pppErrorClass | 9)		/* Magic said, we receive what we send. The link is loopbacked */
#define		pppErrLinkLost			(pppErrorClass | 10)	/* Link error */

#endif
