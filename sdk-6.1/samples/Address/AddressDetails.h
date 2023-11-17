/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDetails.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSDETAILS_H_
#define _ADDRESSDETAILS_H_

#include <Event.h>

/************************************************************
 * Internal constants
 *************************************************************/

#define kMaxItemNumberInShowList	25

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean DetailsHandleEvent (EventType * event);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSDETAILS_H_
