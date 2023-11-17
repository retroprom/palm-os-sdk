/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoTransfer.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Header for To Do.
 *
 *****************************************************************************/

#ifndef _TODOTRANSFER_H_
#define _TODOTRANSFER_H_

#include <PalmTypes.h>
//#include <IMCUtils.h>
#include <ExgMgr.h>

#include <AppMgr.h>
#include <PdiLib.h>

#include "ToDoDB.h"


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/




/************************************************************
 * Application specific launch codes
 * ABa 04/07/00
 *************************************************************/
#define todoLaunchCmdImportVObject				sysAppLaunchCmdCustomBase


/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
	
extern Boolean ToDoImportVToDo(DmOpenRef dbP, PdiReaderType* reader, uint32_t * uniqueIDP, status_t * error);

extern void ToDoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, uint32_t uniqueID);

extern void ToDoSendRecord (DmOpenRef dbP, uint32_t recordNum,
	const char * const prefix, uint16_t noDataAlertID);
	
extern void ToDoSendCategory (DmOpenRef dbP, CategoryID categoryNum,
	const char * const prefix, uint16_t noDataAlertID);
	
extern status_t ToDoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP);

extern Boolean ToDoImportVCal(DmOpenRef dbP, PdiReaderType* reader, status_t * error, uint32_t * uniqueIDP);

extern void ToDoExportVCal(DmOpenRef dbP, uint32_t cursorID, uint32_t index, ToDoItemType * recordP, 
	PdiWriterType* writer, Boolean writeUniqueIDs, status_t * error);

extern status_t ToDoImportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);

extern status_t ToDoExportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);

extern status_t ToDoDeleteRecord(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);

extern status_t ToDoMoveRecord(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);

#ifdef __cplusplus 
}
#endif

#endif /* _TODOTRANFER_H_ */
