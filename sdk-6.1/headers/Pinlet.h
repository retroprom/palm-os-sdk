/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Pinlet.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *              Pinlet plugin API.
 *
 *****************************************************************************/

#ifndef __PINLET_H__
#define __PINLET_H__

#include <PalmTypes.h>


// Functions that Pinlets must implement.

// Reset the internal state of the Pinlet.
typedef void (*PinletClearStateProcPtr)(void);

// Input Mode control (shift, caps lock, ...)
typedef uint16_t (*PinletGetInputModeProcPtr)(void);
typedef void (*PinletSetInputModeProcPtr)(uint16_t mode);

// Overlay pinlet state control.
typedef uint16_t (*PinletGetInputAreaStateProcPtr)(void);
typedef void (*PinletSetInputAreaStateProcPtr)(uint16_t state);

// Reference/help dialog
typedef void (*PinletShowReferenceDialogProcPtr)(void);


typedef struct
{
	PinletClearStateProcPtr          pinletClearState;
	PinletGetInputModeProcPtr        pinletGetInputMode;
	PinletSetInputModeProcPtr        pinletSetInputMode;
	PinletShowReferenceDialogProcPtr pinletShowReferenceDialog;

	// Added at PINS API version 2.0.1
	PinletGetInputAreaStateProcPtr   pinletGetInputAreaState;
	PinletSetInputAreaStateProcPtr   pinletSetInputAreaState;

} PinletAPIType;


#ifdef __cplusplus
extern "C" {
#endif


// Send a character to the main event queue.
void PINFeedChar(wchar32_t chr, uint32_t flags);

// Send a string to the main events queue.
void PINFeedString(const char *str);


#ifdef __cplusplus
}
#endif

#endif /* __PINLET_H__ */
