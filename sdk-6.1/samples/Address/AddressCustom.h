/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressCustom.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSCUSTOM_H_
#define _ADDRESSCUSTOM_H_

#include <Event.h>

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

#define kLabelTableColumn		0
#define kFieldTableColumn		1
#define kMaxNumItems			20

#define kCustomFieldMaxChars	15

/***********************************************************************
 *
 *	Internal structures
 *
 ***********************************************************************/
typedef struct CustomItemTag
{
	uint32_t	columnID;
	char *		labelP;
	char		textP[kCustomFieldMaxChars + 1];
} CustomItemType;

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean CustomEditHandleEvent (EventType * event);

#ifdef __cplusplus
}
#endif

#endif // _ADDRCUSTOM_H_
