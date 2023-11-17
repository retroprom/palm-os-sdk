/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoInDatebook.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This is the ToDo application's main module.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <Font.h>
#include <CatMgr.h>
#include <string.h>
#include <PrivateRecords.h>

#include "ToDo.h"
#include "ToDoExports.h"


/***********************************************************************
 *
 *	Globals exported from ToDo.c
 *
 ***********************************************************************/
extern uint32_t					TopVisibleRecord;	// Top visible record in list view
extern DateType					Today;				// Date when the device was powered on.
extern FontID					ListFont;			// Font used to draw to do item
extern Boolean					ShowCategories;		// True if categories are displayed in the list view
extern uint32_t					gListViewCursorID;
extern DmOpenRef				ToDoDB;				// ToDo database
extern privateRecordViewEnum	PrivateRecordVisualStatus;	// applies to all other records
extern CategoryID				CurrentCategory;
extern uint8_t					CurrentSortId;

// Is the ToDo launched in stand-alone mode or as a shared library from the Datebook ?
extern Boolean 					LoadedAsSharedLib;
extern ToDoSharedDataType		ToDoSharedData;


/***********************************************************************
 *
 * FUNCTION:    DateToDoInitialize
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void DateToDoInitialize (ToDoSharedDataType * sharedDataP)
{
	// Let the ToDo know that it is used as a shared library
	LoadedAsSharedLib = true;

	// Copy all the IDs from the Datebook agenda form
	memmove(&ToDoSharedData, sharedDataP, sizeof(ToDoSharedDataType));

	StartApplication();

	// Set the list font as requested by the caller
	ListFont = ToDoSharedData.listFont;
	Today = ToDoSharedData.today;
}

/***********************************************************************
 *
 * FUNCTION:    DateToDoFinalize
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void DateToDoFinalize (void)
{
	StopApplication();
}


/***********************************************************************
 *
 * FUNCTION:    DateToDoListViewInit
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void DateToDoListViewInit (FormPtr frm)
{
	ListViewInit (frm);
}


/***********************************************************************
 *
 * FUNCTION:    DateToDoListViewInit
 *
 * DESCRIPTION: Return the number of records according to the specified
 *				date.
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
uint16_t DateToDoCountRecordsWithNewDate (DateType newDate)
{
	uint32_t		recordNum = 1;
	uint32_t		offset = 0;
	uint16_t		count = 0;

	Today = newDate;

	while (SeekRecord (&recordNum, offset, dmSeekForward))
	{
		++count;
		if (offset == 0)
			offset = 1;
	}

	return count;
}

/***********************************************************************
 *
 * FUNCTION:    DateToDoFillView
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void DateToDoFillView(void)
{
	ListViewLoadTable(true);
}

/***********************************************************************
 *
 * FUNCTION:    DateToDoListViewSelectCategory
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  reOpenCursor : do we need to reopen the cursor
 *
 * RETURNED:    Nothing
 *
 * NOTE:		reopening the cursor is required when categories
 *				are changed.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
Boolean DateToDoListViewSelectCategory(void)
{
	// This routine doesn't redraw the table
	return ListViewSelectCategory();
}

/***********************************************************************
 *
 * FUNCTION:    DateToDoUpdateCompleteStatus
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void DateToDoUpdateCompleteStatus(int16_t row, uint16_t complete)
{
	uint16_t	mode;

	// Close database and reopen it with write access
	ToDoCloseCursor(&gListViewCursorID);
	DbCloseDatabase (ToDoDB);
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
			dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
	ToDoGetDatabase(&ToDoDB, mode);
	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);

	// Update complete status
	ListViewChangeCompleteStatus(row, complete);

	// Reopen database with read only access
	ToDoCloseCursor(&gListViewCursorID);
	DbCloseDatabase (ToDoDB);
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
			dmModeReadOnly : (dmModeReadOnly | dmModeShowSecret);
	ToDoGetDatabase(&ToDoDB, mode);
	ToDoOpenCursor(ToDoDB, &gListViewCursorID, &CurrentCategory, 1, CurrentSortId);
}


/***********************************************************************
 *
 * FUNCTION:    DateToDoScrollRecords
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
Boolean DateToDoScrollRecords(uint32_t count, int32_t seekDirection)
{
	uint32_t	recordNum;
	Boolean		scrolled = false;

	recordNum = TopVisibleRecord;
	while (count)
	{
		if (!SeekRecord (&recordNum, 1, seekDirection))
			break;

		TopVisibleRecord = recordNum;
		--count;
		scrolled = true;
	}

	return scrolled;
}

/***********************************************************************
 *
 * FUNCTION:    DateToDoShowCategoriesStatus
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    The display options "show categories" status.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
Boolean DateToDoShowCategoriesStatus(void)
{
	return ShowCategories;
}


/***********************************************************************
 *
 * FUNCTION:    DateToDoResizeTableColumns
 *
 * DESCRIPTION: resize to do table columns.
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    The display options "show categories" status.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			PPL		05/04/04	Landscape mode.
 *
 ***********************************************************************/
void DateToDoListViewResizeTableColumns(TableType* tableP, Coord offsetX)
{
	TblSetColumnWidth(tableP, descColumn, TblGetColumnWidth(tableP, descColumn) + offsetX);
}
