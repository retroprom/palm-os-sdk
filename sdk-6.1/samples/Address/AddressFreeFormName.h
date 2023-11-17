/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressFreeFormName.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Formating full name functions based on Full Name resource types.
 *	Loading /Saving related resources types functions.
 *
 *****************************************************************************/

#ifndef _ADDRESSFREEFORMNAME_H_
#define _ADDRESSFREEFORMNAME_H_

#include <PalmTypes.h>
#include <DataMgr.h>

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

MemHandle	AddrFreeFormNameBuild(DmOpenRef dbP, uint32_t rowID);

void 		AddrFreeFormNameFree(MemHandle fullNameH);

#ifdef __cplusplus
}
#endif

#endif
