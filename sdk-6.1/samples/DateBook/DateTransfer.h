/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DateTransfer.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the datebook transfer functions.
 *
 *****************************************************************************/

#ifndef _DATETRANSFERT_H_
#define _DATETRANSFERT_H_

#include <PalmTypes.h>
#include <AppMgr.h>
#include <DataMgr.h>
#include <PdiLib.h>

typedef Boolean ImportVToDoF(DmOpenRef dbP, PdiReaderType* reader, Boolean beginAlreadyRead);

enum vCardSenderKindTag {
	kPalmOS5AndBefore,
	kPalmOS6AndAfter,
	kNonPalmOSDevice
} ;

typedef Enum16 VCardSenderKindType;

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void 		DateSendRecord (DmOpenRef dbP, uint32_t recordNum, const char * const prefix);

status_t 	DateReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP);

void		DateTransferPreview(ExgPreviewInfoType *infoP);

status_t 	DateImportData(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP);

status_t 	DateExportData(DmOpenRef dbP, uint32_t cursorID,  ImportExportRecordParamsPtr impExpObjP);

status_t 	DateDeleteRecord(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP);

#ifdef __cplusplus
}
#endif

#endif //_DATETRANSFERT_H_
