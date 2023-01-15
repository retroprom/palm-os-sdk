/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrNote.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRNOTE_H
#define ADDRNOTE_H

#include <Form.h>
#include <ErrorMgr.h>


/************************************************************
 * Function Prototypes
 *************************************************************/

Boolean	NoteViewHandleEvent (EventType * event);
Boolean	NoteViewCreate (void);
void	NoteViewDelete (void);

#endif // ADDRNOTE_H
