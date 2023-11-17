/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoDB.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header for the Memo Schema
 *
 *****************************************************************************/

#ifndef _MEMODB_H_
#define _MEMODB_H_


#define kMemoDBName				"RAM_MemoDBS"
#define kMemoDBType				'DATA'

#define kMemoDBSchemaID			1000
#define kMemoDBSchemaName		"RAM_MemoDBSchema"
#define kMemoDBSchemaNumCols	1

#define kMemoDBColumnID			100
#define kMemoDBColumnMaxSize	65535
#define kMemoDBColumnType		dbVarChar
#define kMemoDBColumnName		"MemoText"

#define kMemoDBNumSchemas		1

// Sort orders
#define kMemoDBAlphabeticSortString		" ORDER BY " kMemoDBColumnName " ASCENDING CASED"


#ifdef __cplusplus
extern "C" {
#endif

DmOpenRef	MemoDBOpenDatabase(uint16_t mode);
status_t	MemoDBOpenIOCursor(DmOpenRef * dbPP, uint32_t * cursorIDP);
status_t	MemoDBOpenCursor(DmOpenRef * dbPP, uint32_t * cursorIDP, CategoryID * catIDsP, uint32_t nCatIDs, Boolean alphabeticSort);
status_t	MemoDBCreateDefaultDatabase(void);

uint32_t 	MemoGetSchemaColumnMaxSize(DmOpenRef dbP, uint32_t columnID);

status_t	MemoDBCreateRecord(DmOpenRef dbP, uint32_t cursorID, uint32_t *recordID, uint32_t numCategories, CategoryID *categoriesP);

status_t	MemoDBLoadRecord(DmOpenRef dbP, uint32_t recordID, void **dataPP, uint32_t *dataSizeP);
status_t	MemoDBLoadRecordToHandle(DmOpenRef dbP, uint32_t recordID, MemHandle *textH);

status_t	MemoDBAppendRecord(DmOpenRef dbP, uint32_t *recordID, void *dataP, uint32_t dataSize);
status_t	MemoDBAppendRecordFromHandle(DmOpenRef dbP, uint32_t *recordID, MemHandle textH);

uint32_t	MemoDBNumVisibleRecordsInCategory(uint32_t cursorID);

status_t	MemoDBChangeSortOrder(DmOpenRef dbP, uint32_t * cursorIDP, Boolean alphabeticSort);

void		MemoDBSetBackupBit(DmOpenRef dbP);

#ifdef __cplusplus
}
#endif

#endif /* __MEMODB_H__ */
