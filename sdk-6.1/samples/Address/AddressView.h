/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressView.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSVIEW_H_
#define _ADDRESSVIEW_H_

#include <Event.h>

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean ViewHandleEvent(EventType *eventP);

void	RecordViewGotoItem(GoToParamsType *goToParamsP);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSVIEW_H_
