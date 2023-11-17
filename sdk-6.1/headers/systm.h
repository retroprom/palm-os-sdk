/** Copyright (c) 1996  Mentat Inc.
 ** systm.h 43.1, last change 05/18/01
 **/

#ifndef __SYSTM__
#define __SYSTM__

#ifdef MI_H_ID_STRINGS
static	char	systm_h_sccsid[] = "@(#)systm.h	43.1";
#endif

#include <SysThread.h>

#ifndef HZ
#define HZ		1000	/* 'lbolt' is incremented 1000 times per second */
#endif

#define lbolt	_lbolt_func()

INLINE_FNC time_t
_lbolt_func()
{
	return (time_t)((SysGetRunTime() * HZ) / 1000000000);
}

#endif
