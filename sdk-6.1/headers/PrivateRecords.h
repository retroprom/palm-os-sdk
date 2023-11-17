/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PrivateRecords.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This header file defines a generic private record maintainance dialogs, etc.
 *
 *****************************************************************************/

#ifndef	__PRIVATERECORDS_H__
#define	__PRIVATERECORDS_H__

#include <PalmTypes.h>

// Defines needed for hidden record visual determination.
enum privateRecordViewTag {
showPrivateRecords = 0x00,
maskPrivateRecords,
hidePrivateRecords
};
typedef Enum8 privateRecordViewEnum;

//-----------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

privateRecordViewEnum SecSelectViewStatus (void);

Boolean SecVerifyPW (privateRecordViewEnum newSecLevel);

#ifdef __cplusplus 
}
#endif

#endif //__PRIVATERECORDS_H__
