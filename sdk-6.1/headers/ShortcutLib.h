/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ShortcutLib.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 * 	This header file describes the Shortcuts Library API.
 *
 *****************************************************************************/

#ifndef __SHORTCUTLIB_H__
#define __SHORTCUTLIB_H__

#include <PalmTypes.h>


/********************************************************************
 * Constants
 ********************************************************************/

#define shortcutLibName	"Shortcuts Library"
#define shortcutDBName	"Graffiti ShortCuts"

#define shortcutAPIVersion (sysMakeROMVersion(4, 0, 0, sysROMStageDevelopment, 0))


#define shortcutNone			0xffff			// Index which isn't a shortcut


#define shortcutMaxNameLen		8


// char indicating a seqeunce of characters to expand.
#define	shortcutExpansionSequence	'@'

// Chars indicating what to expand into
#define	expandDateChar				'D'
#define	expandTimeChar				'T'
#define	expandStampChar				'S'	//	This follows 'D' or 'T' for the sake
										//	of the mnemonic name.
#ifndef HasExpansionSequence
#define HasExpansionSequence(s)	(s[0] == shortcutExpansionSequence)
#endif


// Errors
// These shortcut-related errors were defined in Graffiti.h
// with different names.
#define	shortcutErrNotFound				(grfErrorClass | 6)
#define	shortcutErrPtrTooSmall			(grfErrorClass | 8)
#define	shortcutErrNoShortcuts			(grfErrorClass | 9)
#define	shortcutErrIncompleteMatch		(grfErrorClass | 129)

// Future errors should be defined starting from (tsmErrorClass+128)
#define	shortcutErrCustomBase			(tsmErrorClass+128)

// Notifications
#define shortCutNotifyAddDbgMacrosEvent	'SCam'	// Notify Graffiti that it needs to add  
												// the debugging macros to the shortcuts database.
												// This is broadcast by the Shortcuts library after it
												// has re-created the shortcuts database on a locale
												// changed event.


/********************************************************************
 * Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


status_t ShortcutLibGetRaw(const char *nameP, uint8_t *dataP, uint16_t *dataLenP);

status_t ShortcutLibGetExpanded(const char *nameP, uint8_t *dataP, uint16_t *dataLenP);

status_t ShortcutLibGetNameByIndex(uint16_t index, char *nameP);

status_t ShortcutLibDeleteByIndex(uint16_t index);

status_t ShortcutLibAdd(const char *nameP, const uint8_t *dataP, uint16_t dataLen);


#ifdef __cplusplus
}
#endif

#endif 	//__SHORTCUTLIB_H__
