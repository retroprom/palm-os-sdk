/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DateU16List.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Loading / Accessing / Saving 'u16L' resources types functions.
 *
 *****************************************************************************/

 #ifndef _U16LIST_H_
 #define  _U16LIST_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include <UIResources.h>


#define kU16ListResType				 'u16L'
 

MemHandle  	U16ListGetList(DmOpenRef resourceDBRef, DmResourceID ulistId);
uint16_t  	U16ListGetItem(DmOpenRef resourceDBRef, DmResourceID ulistId, MemHandle uListH, uint32_t index);
uint32_t  	U16ListGetItemNum(DmOpenRef resourceDBRef, DmResourceID ulistId, MemHandle uListH);
void 		U16ListReleaseList(MemHandle uListH);


 #endif
