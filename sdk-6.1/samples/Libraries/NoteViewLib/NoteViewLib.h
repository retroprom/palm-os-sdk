/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: NoteViewLib.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the Note View API.
 *
 *****************************************************************************/

#ifndef __NOTE_VIEW_H__
#define __NOTE_VIEW_H__

#include <PalmTypes.h>

//--------------------------------------------------------------------
// Note View Functions
//--------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// If argument rowID is a cursorID, argument rowPos must be a valid index [1..dbMaxRowIndex]

typedef Boolean (*DeleteNoteCallbackPtr)(DmOpenRef dbRef, uint32_t rowID, uint32_t rowPos, uint32_t columnId);

// If you want that your application checks and delete by itself the note, define and assign a
// callback using these API functions. Otherwise, do not define any callback, set the deleteCallbackP
// parameter to NULL and the default behavior will be used.
extern Boolean 	EditNote(const char *title, DmOpenRef dbRef, uint32_t rowID, uint32_t rowPos, uint32_t columnId,
	DeleteNoteCallbackPtr deleteCallbackP);
extern Boolean 	EditNoteSelection(const char *title, DmOpenRef dbRef, uint32_t rowID, uint32_t rowPos, uint32_t columnId, 
	uint16_t selectionPos, uint16_t positionLen, DeleteNoteCallbackPtr deleteCallbackP);

#ifdef __cplusplus
}
#endif

#endif // __NOTE_VIEW_H__
