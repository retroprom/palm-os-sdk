/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressLookup.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSLOOKUP_H_
#define _ADDRESSLOOKUP_H_

#include <AppMgr.h>
#include <Form.h>
#include <PdiLib.h>
#include <PhoneLookup.h>

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void		Lookup(AddrLookupParamsType *params);

MemHandle	LookupCreateResultString(DmOpenRef searchDB, char * formatStringP, uint32_t rowID);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSLOOKUP_H_
