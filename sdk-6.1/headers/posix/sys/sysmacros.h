/** Copyright (c) 1993-2000  Mentat Inc.
 ** sysmacros.h 43.1, last change 05/18/01
 **/

#ifndef __SYSMACROS__
#define __SYSMACROS__

#ifdef MI_H_ID_STRINGS
static	char	sysmacros_h_sccsid[] = "05/18/01"
#endif

/* major part of a device */
#define	getmajor(x)	((int)(((unsigned)(x)>>8)&0377))

/* minor part of a device */
#define	getminor(x)	((int)((x)&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | (y)))

/* compress and expand devs */
#define	cmpdev(d)	(d)
#define	expdev(d)	(d)

#define	getemajor	getmajor
#define	geteminor	getminor

#define	etoimajor(emaj)			(emaj)
#define	itoemajor(imaj, prevemaj)	(imaj)

#define makedevice	makedev

#define	major		getmajor
#define minor		getminor

#endif /* __SYSMACROS__ */
