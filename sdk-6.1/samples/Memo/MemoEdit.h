/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoEdit.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef __MEMOEDIT_H__
#define __MEMOEDIT_H__

/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Boolean EditViewHandleEvent(EventType *event);

Boolean DetailsHandleEvent(EventType * event);

	void EditViewInitLayout(FormPtr formP);

#ifdef __cplusplus
}
#endif

#endif /* __MEMOEDIT_H__ */
