/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDialList.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSDIALLIST_H_
#define _ADDRESSDIALLIST_H_

#include <Event.h>

//#include "AddressDB.h"

/***********************************************************************
 *
 *	Internal constants
 *
 ***********************************************************************/

#define kSpaceBetweenLabelAndNumber 6
#define kLeftAndRightSpace			2
#define	kMaxPhonesCount				28
#define kMaxCharsLabelLen			27

/***********************************************************************
 *
 *	Internal types
 *
 ***********************************************************************/

typedef struct DialListPhoneTag
{
	char		label[kMaxCharsLabelLen + 1];
	char*		number;
	uint32_t	numberLen;
	uint32_t	phoneWidth;
	uint32_t	labelWidth;
} DialListPhoneType;


typedef struct DialListDataTag
{
	uint32_t			rowID;
	
	// Record description - allocated
	char*				displayName;

	// list info
	int16_t				topIndex;
	int16_t				selectedIndex;

	// Array of data
	DialListPhoneType	phones[kMaxPhonesCount];
	uint16_t			phonesCount;
	
	uint16_t			reserved;
} DialListDataType;

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean DialListShowDialog(uint32_t rowID, uint32_t columnID);
Boolean DialListHandleEvent(EventType *event);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSDIALLIST_H_
