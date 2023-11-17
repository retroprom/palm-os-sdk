/** Copyright (c) 1993, 1996  Mentat Inc.
 ** sad.h 43.1, last change 05/18/01
 **/



#ifndef _SAD_
#define _SAD_

#include <sys/types.h>
#include <stropts.h>

#ifdef MI_H_ID_STRINGS
static char	sad_h_sccsid[] = "@(#)sad.h	43.1";
#endif

/*
 * NOTE: add define of MPS_SAD_USES_DEVICE_NAMES here for systems that
 * support dynamically loaded devices.  For these systems, sad ioctls
 * use device names rather than major numbers to identify drivers.
 */
#define MPS_SAD_USES_DEVICE_NAMES	1

#ifndef _MIIOCCOM_
#include <miioccom.h>
#endif

#define	MAXAPUSH	8

#define	SAD_SAP		MIOC_CMD(MIOC_SAD,1)	/* Set autopush information */
#define	SAD_GAP		MIOC_CMD(MIOC_SAD,2)	/* Get autopush information */
#define	SAD_VML		MIOC_CMD(MIOC_SAD,3)	/* Validate a list of modules */
						/* (uses str_list structure) */

/* Ioctl structure used for SAD_SAP and SAD_GAP commands */
struct strapush {
	unsigned int	sap_cmd;
#ifdef	MPS_SAD_USES_DEVICE_NAMES
	char		sap_device_name[FMNAMESZ+1];
#else
	long		sap_major;
#endif
	long		sap_minor;
	long		sap_lastminor;
	long		sap_npush;
	char		sap_list[MAXAPUSH][FMNAMESZ+1];
};

/* Command values for sap_cmd */
#define	SAP_ONE		1	/** Configure a single minor device */
#define	SAP_RANGE	2	/** Configure a range of minor devices */
#define	SAP_ALL		3	/** Configure all minor devices */
#define	SAP_CLEAR	4	/** Clear autopush information */

#endif	/* _SAD_ */
