/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressPhoneIO.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSPHONEIO_H_
#define _ADDRESSPHONEIO_H_

#include <PalmTypes.h>
#include <DataMgr.h>
#include "PhoneBookIOLib.h"

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Boolean AddressPhoneIOGetUserImportSettings(uint8_t *phoneDirectoryP, CategoryID *categoryIDP);
Boolean AddressPhoneIOGetUserExportSettings(uint8_t *phoneDirectoryP, CategoryID *categoryIDP);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSPHONEIO_H_

