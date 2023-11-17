/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: clipBoard.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines clipboard routines.
 *
 *****************************************************************************/

#ifndef _CLIPBOARD_H_
#define _CLIPBOARD_H_

#include <PalmTypes.h>

#define numClipboardFormats		3

// Clipboard standard formats
enum clipboardFormats { clipboardText, clipboardInk, clipboardBitmap };

typedef Enum8 ClipboardFormatType ;

typedef struct ClipboardItemTag ClipboardItem;

//----------------------------------------------------------
//	Clipboard Functions
//----------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern void ClipboardAddItem
	(const ClipboardFormatType format, const void *ptr, size_t length);

extern status_t ClipboardAppendItem
	(const ClipboardFormatType format, const void *ptr, size_t length);

extern MemHandle ClipboardGetItem (const ClipboardFormatType format,
	size_t *length);

#ifdef __cplusplus
}
#endif

#endif // _CLIPBOARD_H_
