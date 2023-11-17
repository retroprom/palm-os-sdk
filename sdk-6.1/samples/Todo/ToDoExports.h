/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoExports.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  ToDo Export definitions - Used in Datebook to display the Agenda view
 *
 *****************************************************************************/

#ifndef __TODOEXPORTS_H__
#define	__TODOEXPORTS_H__

#include <PalmTypes.h>
#include <Form.h>
#include <Table.h>


// Sort orders
#define soPriorityDueDate		0
#define soDueDatePriority		1
#define soCategoryPriority		2
#define soCategoryDueDate		3

#define defaultSortId 			soPriorityDueDate

// Schema macros
#define todoDefaultDbSchemaId	0
#define todoDueDateColId		100
#define todoPriorityColId		200
#define todoDescriptionColId	300
#define todoNoteColId			400
#define todoCompletedColId		500

// Default DB
#define ToDoDefaultDBResId		1000


// Id shared with DateBook
#define DateToDoInitializeId						0
#define DateToDoFinalizeId							1
#define DateToDoListViewInitId						2
#define DateToDoCountRecordsWithNewDateId			3
#define DateToDoFillViewId							4
#define DateToDoListViewSelectCategoryFuncId		5
#define DateToDoUpdateCompleteStatusFuncId			6
#define DateToDoScrollRecordsId						7
#define DateToDoShowCategoriesStatusId				8
#define DateToDoListViewResizeTableColumnsFuncId	9


// This structures lists all the fields, table, control IDs needed
// to display records in the main form
typedef struct {
	uint16_t		listViewId;
	uint16_t		listTableId;
	uint16_t		ListCategoryListId;
	uint16_t		ListCategoryTriggerId;
	DateType		today;
	FontID			listFont;
	uint8_t			reserved;
} ToDoSharedDataType;


typedef void 		DateToDoInitializeFunc(ToDoSharedDataType * sharedIdsP);
typedef void 		DateToDoFinalizeFunc(void);
typedef void 		DateToDoListViewInitFunc (FormPtr frm);
typedef	uint16_t 	DateToDoCountRecordsWithNewDateFunc (DateType newDate);
typedef	void 		DateToDoFillViewFunc(void);
typedef	Boolean		ListViewSelectCategoryFunc (void);
typedef	void 		ListViewChangeCompleteStatusFunc (int16_t row, uint16_t complete);
typedef	Boolean		DateToDoScrollRecordsFunc (uint32_t count, int32_t seekDirection);
typedef	Boolean		DateToDoShowCategoriesStatusFunc (void);
typedef	void 		DateToDoListViewResizeTableColumnsFunc(TableType* tableP, Coord offsetX);


#endif // __TODOEXPORTS_H__
