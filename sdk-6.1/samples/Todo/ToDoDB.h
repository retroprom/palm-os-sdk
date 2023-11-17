/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoDB.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		Header for the To Do Manager
 *
 *****************************************************************************/

#ifndef _TDTODOMGR_H_
#define _TDTODOMGR_H_

#include <DateTime.h>
#include "ToDoExports.h"

#define LocalizedAppInfoStr		1000

#define todoLabelLength			12
#define todoNumFields			16

#define toDoMaxPriority			5
#define defaultPriority			1

// Dirty flags for to do application info
#define toDoSortByPriorityDirty	0x0001

// ToDoItemType
//
// This is the format of a to do record used by the application.  All 
// pointers are either NULL or point to data within the PackedDB record.
// All strings are null character terminated.
//
typedef struct {
	DateType 			dueDate;
	uint8_t 			priority;
	Boolean				completed;
	char *				description;
	char *				note;
} ToDoItemType;

#define toDoNoDueDate	0xffff

#define itemHasDescription(x) ((x).description && *((x).description))
#define itemHasNote(x) ((x).note && *((x).note))
#define itemIsEmpty(x) ((!itemHasDescription(x)) && (!itemHasNote(x)))

#define dbGetDataFailed(err) ((err) < errNone && (err) != dmErrOneOrMoreFailed && (err) != dmErrNoColumnData)

// Used for ToDoChangeRecord.
enum ToDoRecordFieldTag { 
	toDoPriority,
	toDoComplete,
	toDoDueDate,
	toDoDescription,
	toDoNote
} ;

typedef Enum8 ToDoRecordFieldType ;

// Constants
static const uint32_t gColIDs[] = {todoDueDateColId,todoPriorityColId,todoDescriptionColId,todoNoteColId,todoCompletedColId} ;
#define gNCols (sizeof(gColIDs) / sizeof(uint32_t))
#define knownColID(x) (x==todoDueDateColId || x==todoPriorityColId || x==todoDescriptionColId || x==todoNoteColId || x==todoCompletedColId)

#define toDoDescriptionMaxSize		256
#define toDoNoteMaxSize				65534

#ifdef __cplusplus
extern "C" {
#endif

extern char * SortOrderStr[];

//-------------------------------------------------------------------
// Routines
//-------------------------------------------------------------------
status_t ToDoNewRecord(DmOpenRef dbP, uint32_t * cursorID, ToDoItemType * item, uint32_t * index, CategoryID catID);

status_t ToDoInsertNewRecord (DmOpenRef dbP, uint32_t * cursorID, uint32_t * index, CategoryID * catIDsP, uint32_t nCatIDs);

status_t ToDoChangeRecord(DmOpenRef dbP, uint32_t cursorID, uint32_t * index, 
	ToDoRecordFieldType changedField, const void * data);

uint8_t ToDoGetSortId (DmOpenRef dbP, uint32_t cursorID);

status_t ToDoGetDatabase (DmOpenRef *dbPP, uint16_t mode);

status_t CreateDefaultDatabase(void);

status_t ToDoOpenIOCursor(DmOpenRef dbP, uint32_t * cursorIDP);

status_t ToDoOpenCursor(DmOpenRef dbP, uint32_t * cursorIDP, CategoryID * catIDsP, uint32_t nCatIDs, uint8_t sortOrder);

status_t ToDoCloseCursor(uint32_t * cursorIDP);

status_t ToDoDbCursorRequery(uint32_t * cursorIDP);

void ToDoSetDBBackupBit (DmOpenRef dbP);

void ToDoItemToColumnValues(DbSchemaColumnValueType * colVals, ToDoItemType * item, uint32_t * nCols);

void ToDoColumnValuesToItem(DbSchemaColumnValueType * colVals, ToDoItemType * item, uint32_t nCols);


#ifdef __cplusplus
}
#endif


#endif
