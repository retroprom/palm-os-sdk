/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Keyboard.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *   This file defines the keyboard's structures
 *   and routines.
 *
 *****************************************************************************/

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <PalmTypes.h>

enum KeyboardTag
	{
	kbdAlpha = 0,
	kbdNumbersAndPunc = 1,
	kbdAccent = 2,
	kbdDefault = 0xff       // based on graffiti mode (usually alphaKeyboard)
	} ;
typedef Enum8 KeyboardType;


/************************************************************
 * Keyboard procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// At some point the Graffiti code will need access to the
// shift and caps lock info.  Either export the structures
// or provide calls to the info.

extern void SysKeyboardDialog (KeyboardType kbd);

#ifdef __cplusplus
}
#endif

#endif // __KEYBOARD_H__
