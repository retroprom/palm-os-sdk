/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressU32List.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Loading / Accessing / Saving 'u32L' resources types functions.
 *
 *****************************************************************************/

#ifndef _ADDRESSU32LIST_H_
#define _ADDRESSU32LIST_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include <UIResources.h>


/************************************************************
 * Constante
 *************************************************************/

#define kU32ListResType		'u32L'

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

MemHandle  	U32ListGetList(DmOpenRef resourceDBRef, DmResourceID u32listId);
uint32_t  	U32ListGetItem(DmOpenRef resourceDBRef, DmResourceID u32listId, MemHandle u32ListH, uint32_t index);
uint32_t  	U32ListGetItemNum(DmOpenRef resourceDBRef, DmResourceID u32listId, MemHandle u32ListH);
void 		U32ListReleaseList(MemHandle u32ListH);
Boolean		U32ListContainsItem(DmOpenRef resourceDBRef, DmResourceID u32listId, MemHandle u32ListH, uint32_t toFind);

#ifdef __cplusplus
}
#endif


#endif
