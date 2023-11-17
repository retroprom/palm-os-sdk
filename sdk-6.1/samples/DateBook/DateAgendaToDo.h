/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateAgendaToDo.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  Manage the ToDo records in agenda view
 *
 *****************************************************************************/

#ifndef _DATEAGENDATODO_H_
#define _DATEAGENDATODO_H_

#include <PalmTypes.h>

#include <Form.h>


void 		ToDoViewInit (FormPtr frm );

void 		ToDoViewFinalize (void);

void 		AgendaViewInitToDo (FormPtr frm );

uint16_t 	ToDoCountRecordsOnCurrentDate (void);

void 		AgendaViewFillTasks (void);

void 		ToDoListViewDrawTable (void);

Boolean 	ToDoListViewSelectCategory (void);

Boolean		ToDoListViewScroll (int16_t delta);

void 		ToDoListViewChangeCompleteStatus (int16_t row, uint16_t complete);

void 		ToDoListViewResizeTableColumns(TableType* tableP, Coord offsetX);

void 		LaunchToDoWithRecord (uint32_t inRowID);


#endif
