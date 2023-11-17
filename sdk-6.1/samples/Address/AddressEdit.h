/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressEdit.h
 *
 * Release: Palm OS 6.0.1
 *
 *****************************************************************************/

#ifndef _ADDRESSEDIT_H_
#define _ADDRESSEDIT_H_

#include <Event.h>

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean	EditHandleEvent (EventType * event);
void	EditNewRecord (void);
void	EditViewSetFocusable(Boolean focusable);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSEDIT_H_
