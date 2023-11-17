/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressNote.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSNOTE_H_
#define _ADDRESSNOTE_H_

#include <Event.h>

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean 	AddressExistingNote(uint32_t rowID);
Boolean 	AddressEditNote(uint32_t rowID);
void 		AddressDeleteNote(uint32_t rowID);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSNOTE_H_
