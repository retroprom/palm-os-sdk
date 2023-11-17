/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DateAgenda.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  Display and support code for Datebook's Agenda view.
 *
 *****************************************************************************/

#ifndef _DATEAGENDA_H_
#define _DATEAGENDA_H_

#define agendaTitleDateColumn				0
#define agendaTitleTimeColumn				1

#define agendaTitleInset					4
#define agendaTitleDateColumnSpacing		0

// Columns in the agenda view's appointment table
#define agendaApptTimeColumn				0
#define agendaApptDescColumn				1

#define agendaApptTimeColumnSpacing			3


#define agendaToDoTitleColumn				0
#define agendaToDoCategoryColumn			1

#define agendaToDoTitleInset				4		// Pixels between title and left margin
#define agendaToDoCategoryInset				4		// Pixels between category menu and right margin

#define agendaToDoTitleColumnSpacing		0		// Space between title and category columns

#define noAppointments						0xffff

#define completedColumn						0

#define toDoNoDueDate						0xffff

/***********************************************************************
 *
 *	Agenda structure
 *
 ***********************************************************************/

typedef struct {
	uint16_t							todoCount;
	uint16_t							todoLineHeight;
	uint16_t							todoLineCount;
} ToDoViewInfoType;


// Used for ToDoChangeRecord.
typedef enum  {
	toDoPriority,
	toDoComplete,
	toDoDueDate,
	toDoDescription,
	toDoNote,
	toDoCategory } ToDoRecordFieldType;

typedef struct {
	DateType dueDate;
	uint8_t priority;		// high bit is complete flag
	char description;
} ToDoDBRecord;

typedef ToDoDBRecord			ToDoDBRecordType;
typedef ToDoDBRecord*			ToDoDBRecordPtr;


/***********************************************************************
 *
 *	Functions
 *
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void 		AgendaUpdateToDoCount(void);

Boolean		AgendaViewHandleEvent (EventType* event);

void		AgendaViewRefreshLayoutTasks (FormType* inFormP );

//void		AgendaViewDrawSeparator (FormType* inFormP );

void 		AgendaViewDisplayInvalidate(FormPtr formP, RectangleType* dirtyRect);							

void		AgendaViewSetScrollThumb (FormType* inFormP, uint16_t inObjectID, int32_t inValue );

#ifdef __cplusplus
}
#endif

#endif // _DATEAGENDA_H_
