/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: StatusBar.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _STATUSBAR_H_
#define _STATUSBAR_H_

#include <CmnErrors.h>

// StatusBar errors
#define statErrInvalidLocation	(statErrorClass | 1)
#define statErrInvalidName		(statErrorClass | 2)
#define statErrInputWindowOpen	(statErrorClass | 3)

// selectors for StatGetAttribute
typedef enum {
	statAttrBarVisible,
	statAttrDimension
} StatAttrType;

#ifdef __cplusplus
extern "C" {
#endif

status_t StatGetAttribute(StatAttrType selector, uint32_t* dataP);
status_t StatHide(void);
status_t StatShow(void);

#ifdef __cplusplus
}
#endif

#endif //_STATUSBAR_H_
