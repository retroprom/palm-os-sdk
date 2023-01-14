/******************************************************************************
 *                                                                            *
 *                 (C) Copyright 2000, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonySystemResources.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       CreatorID/Type of DB, ID/Type of Resoueces for Sony System           *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/11/28                                              *
 *       Last Modified: $Date: 01/08/31 13:11 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYSYSTEMRESOURCES_H__
#define __SONYSYSTEMRESOURCES_H__

/******************************************************************************
 *    Includes                                                                *
 ******************************************************************************/
#include <SystemResources.h>
#include <UIResources.h>

/******************************************************************************
 *    CreatorID and Type for Databases                                        *
 ******************************************************************************/

/*** Sony oveall ***/
#define sonySysFileCSony			'SoNy'	/* Sony overall */
#define	sonySysVfsCstmApiCreator	sonySysFileCSony	/* VFS Custom Control */

#define sonySysFileCSystem			'SsYs'	/* Sony overall System */

/*** Application ***/


/*** Extension ***/


/*** Library ***/
/* HR-Lib */
#define sonySysFileCHRLib			'SlHr'	/* High Resolution */
#define sonySysFileTHRLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameHR			"Sony HR Library"

/* Msa-Lib */
#define sonySysFileCMsaLib			'SlMa'	/* MS Audio */
#define sonySysFileTMsaLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameMsa			"Sony Msa Library"

/* Rmc-Lib */
#define sonySysFileCRmcLib			'SlRm'	/* Remote Control */
#define sonySysFileTRmcLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameRmc			"Sony Rmc Library"

/* MsScsi-Lib */	/* not officially supported (as of 2001/4) */
#define sonySysFileCMsScsiLib		'SlSc'	/* MS SCSI */
#define sonySysFileTMsScsiLib		sysFileTLibrary		/* 'libr' */
#define sonySysLibNameMsScsi		"Sony MsScsi Library"

/* Irc-Lib */
#define sonySysFileCIrcLib			'SlIr'	/* Infrared Controller */
#define sonySysFileTIrcLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameIrc			"Sony Irc Library"

/* SonySoundLib */
#define sonySysFileTPcmLib			'pcmR'

#define sonySysFileCSoundLib		'SlSd'	/* Sony Sound Lib  */
#define sonySysFileTSoundLib		sysFileTLibrary		/* 'libr' */
#define sonySysLibNameSound		"Sony Sound Library"

/*** System Application ***/
/* JogPanel Preference */
#define sonySysFileCJogPanel		'SnJp'	/* Jog Panel */
#define sonySysFileTJogPanel		sysFileTPanel		/* 'panl' */


/******************************************************************************
 *    Misc CreatorID                                                          *
 ******************************************************************************/


#endif	// __SONYSYSTEMRESOURCES_H__

