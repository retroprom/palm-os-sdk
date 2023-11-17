/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: MemoTransfer.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef __MEMOTRANSFERT_H__
#define __MEMOTRANSFERT_H__

#include "MemoDB.h"

/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t ReadFunctionF (void * stream, char * bufferP, uint32_t length, status_t * error);
typedef uint32_t WriteFunctionF (void * stream, const char * const bufferP, int32_t length, status_t * error);

void	MemoSendRecord (DmOpenRef dbP, uint32_t recordNum, const char * const prefix);

void	MemoSendCategory (DmOpenRef dbP, uint32_t numCategories, CategoryID *categoriesP, const char * const prefix, uint16_t noDataAlertID);

status_t		MemoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP, uint16_t *numRecordsReceived);
void	MemoTransferPreview(ExgPreviewInfoType *infoP);

Boolean	MemoImportMime(DmOpenRef dbR, void * inputStream, ReadFunctionF inputFunc,
					   Boolean obeyUniqueIDs, Boolean beginAlreadyRead, uint16_t *numRecordsReceivedP,
					   char* descriptionP, uint16_t descriptionSize, status_t * error);

void	MemoExportMime(DmOpenRef dbP, char *memoP,
					   void * outputStream, WriteFunctionF outputFunc,
					   Boolean writeUniqueIDs, Boolean outputMimeInfo, status_t * error);
	
status_t		MemoImportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);
status_t		MemoExportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);
status_t		MemoDeleteData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);
status_t		MemoMoveData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);

#ifdef __cplusplus 
}
#endif

#endif /* __MEMOTRANSFERT_H__ */
