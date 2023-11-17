/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: GraffitiReference.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the Graffiti Reference routines.
 *
 *****************************************************************************/

#ifndef __GRAFFITIREFERENCE_H__
#define __GRAFFITIREFERENCE_H__

#include <PalmTypes.h>

enum ReferenceTag
	{
	referenceDefault = 0xff		// based on graffiti mode
	} ;
typedef Enum8 ReferenceType;

/************************************************************
 * Graffiti Reference procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


extern void SysGraffitiReferenceDialog (ReferenceType referenceType);


#ifdef __cplusplus
}
#endif

#endif // __GRAFFITIREFERENCE_H__
