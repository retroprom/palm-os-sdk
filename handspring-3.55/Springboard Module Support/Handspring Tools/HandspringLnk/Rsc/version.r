/*
 *	File Name:			Version.r
 *	File Purpose:		Version resource definition for MacOS rez compiler ...
 *
 *	Author:				Berardino E. Baratta
 *	Last Revision:		11/11/97
 *
 *  Copyright © 1997 metrowerks inc.  All rights reserved.
 */

#include "Types.r"

#ifndef BUILD_FRENCH
#define BUILD_FRENCH						0
#endif

#ifndef BUILD_JAPANESE
#define BUILD_JAPANESE						0
#endif


#define MAJORREVISION		1
#define MINORREVISION		0
#define MINORFIX			0
#define RELEASETYPE			release
#define RELEASEVERSION		8

#define VERSION_STRING		"1.0.0 Build 0008"

#define PRODUCT_ID			"CodeWarrior for Palm OS Platform 8.0"
#define COPYRIGHT_STRING	"Copyright © 2001 Handspring, Inc."

/*
 *	do not touch what comes after this comment
 */

#define ID_1				MAJORREVISION
#define ID_2				MINORREVISION * 16 + MINORFIX

resource 'vers' (1) {
	ID_1,
	ID_2,
	RELEASETYPE,
	RELEASEVERSION,
	
/* Setting country code */	
#if BUILD_JAPANESE	
	14,
#else
	0,
#endif	

	VERSION_STRING,
	"v" VERSION_STRING ", a Metrowerks CodeWarriorª Component"
};

resource 'vers' (2) {
	ID_1,
	ID_2,
	RELEASETYPE,
	RELEASEVERSION,
	
/* Setting country code */	
#if BUILD_JAPANESE	
	verJapan,
#else
	verUS,
#endif	

	VERSION_STRING,
	COPYRIGHT_STRING
};

