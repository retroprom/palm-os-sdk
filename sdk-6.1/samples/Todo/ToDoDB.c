/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoDB.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		To Do Manager routines
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

// Set this to get to private database defines
#define __TODOMGR_PRIVATE__

#include <CatMgr.h>
#include <DataMgr.h>
#include <DateTime.h>
#include <ErrorMgr.h>
#include <TraceMgr.h>
#include <string.h>


#include <MemoryMgr.h>
#include <MemoryMgrCompatibility.h>

#include <StringMgr.h>
#include <SystemResources.h>

#include "ToDo.h"
#include "ToDoDB.h"


/************************************************************
 *	Global variables
 ************************************************************/
extern DmOpenRef ToDoDB ;
extern CategoryID CurrentCategory ;
extern uint32_t gListViewCursorID ;
extern uint8_t CurrentSortId ;

char * SortOrderStr[] = {
	" ORDER BY Priority ASCENDING CASELESS, DueDate ASCENDING CASELESS",	// priorityDueDate
	" ORDER BY DueDate ASCENDING CASELESS, Priority ASCENDING CASELESS"	// dueDatePriority
} ;

/************************************************************
 * Private routines used only in this module
 *************************************************************/

/************************************************************
 *
 *  FUNCTION: ToDoItemToColumnValues
 *
 *  DESCRIPTION:  Fills a record from a ToDo item.
 *		Values are referenced, not copied.
 *
 *  PARAMETERS:
 *		<- colVals : array of gNCols columns.
 *		-> item
 *		<- nCols : number of columns actually filled.
 *
 *  RETURNS: void
 *
 *  CREATED: 10/10/02
 *
 *  BY: LYr
 *
 *************************************************************/
void ToDoItemToColumnValues(DbSchemaColumnValueType * colVals, ToDoItemType * item, uint32_t * nCols)
{
	*nCols = 0 ;
	colVals[*nCols].columnID = todoDueDateColId ;
	colVals[*nCols].dataSize = sizeof(DateType) ;
	colVals[*nCols].data = (DbSchemaColumnData*) &(item->dueDate) ;

	(*nCols)++ ;
	colVals[*nCols].columnID = todoPriorityColId ;
	colVals[*nCols].dataSize = sizeof(uint8_t) ;
	colVals[*nCols].data = (DbSchemaColumnData*) &(item->priority) ;

	(*nCols)++ ;
	colVals[*nCols].columnID = todoCompletedColId ;
	colVals[*nCols].dataSize = sizeof(Boolean) ;
	colVals[*nCols].data = (DbSchemaColumnData*) &(item->completed) ;

	if (item->description)
	{
		(*nCols)++ ;
		colVals[*nCols].columnID = todoDescriptionColId ;
		colVals[*nCols].dataSize = 1 + (uint32_t) strlen(item->description);
		colVals[*nCols].data = (DbSchemaColumnData*) item->description ;
	}

	if (item->note)
	{
		(*nCols)++ ;
		colVals[*nCols].columnID = todoNoteColId ;
		colVals[*nCols].dataSize = 1 + (uint32_t) strlen(item->note);
		colVals[*nCols].data = (DbSchemaColumnData*) item->note ;
	}

	(*nCols)++ ;
}

/************************************************************
 *
 *  FUNCTION: ToDoColumnValuesToItem
 *
 *  DESCRIPTION:  Fills a ToDo item from a record.
 *		String values are referenced, not copied.
 *
 *  PARAMETERS:
 *		-> colVals : array of gNCols columns.
 *		<- item
 *		-> nCols : number of columns in the record.
 *
 *  RETURNS: void
 *
 *  CREATED: 10/10/02
 *
 *  BY: LYr
 *
 *************************************************************/
void ToDoColumnValuesToItem(DbSchemaColumnValueType * colVals, ToDoItemType * item, uint32_t nCols)
{
	uint32_t i ;

	MemSet(item, sizeof(ToDoItemType), 0) ;
	item->priority = defaultPriority;
	*((uint16_t *) &(item->dueDate)) = toDoNoDueDate;

	if (! colVals)
		return ;

	for (i=0; i<nCols; i++)
	{
		if (colVals[i].data)
		{
			switch (colVals[i].columnID)
			{
				case todoDueDateColId :
					item->dueDate = * (DateType*) colVals[i].data ;
				break;

				case todoPriorityColId :
					item->priority = * (uint8_t*) colVals[i].data ;
				break;

				case todoDescriptionColId :
					item->description = (char*) colVals[i].data ;
				break;

				case todoNoteColId :
					item->note = (char*) colVals[i].data ;
				break;

				case todoCompletedColId :
					item->completed = * (Boolean*) colVals[i].data ;
				break;
			}
		}
	}
}

/************************************************************
 *
 *  FUNCTION: ToDoNewRecord
 *
 *  DESCRIPTION: Create a new record in sorted position
 *
 *  PARAMETERS: database pointer
 *				database record
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
status_t ToDoNewRecord(DmOpenRef dbP, uint32_t * cursorID, ToDoItemType * item, uint32_t *index, CategoryID catID)
{
	status_t 	err;
	uint32_t	rowID;

	DbSchemaColumnValueType colVals[gNCols];
	uint32_t nCols;

	ToDoItemToColumnValues(colVals, item, &nCols);

	*index = 0;
	if ((err = DbInsertRow(dbP, kToDoDBSchemaName, nCols, colVals, &rowID)) < errNone)
		goto Exit;

	// Set the category.
	if (catID != catIDUnfiled && catID != catIDAll)
	{
		if ((err = DbSetCategory(dbP, rowID, 1, &catID)) < errNone)
		{
			goto Exit ;
		}
	}

	ToDoDbCursorRequery(cursorID);
	if ((err = DbCursorMoveToRowID(*cursorID, rowID)) < errNone)
		goto Exit;

	// Return an index
	err = DbCursorGetCurrentPosition(*cursorID, index);
	TraceOutput(TL(appErrorClass, "ToDoNewRecord created ID: %d index: %d", rowID, *index));

Exit:
	return err;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoInsertNewRecord
 *
 * DESCRIPTION: This routine creates a new record and inserts it after
 *              the specified position.  The new record is assigned the
 *              same priority and due date as the record it is
 *              inserted after.
 *
 * PARAMETERS:  database pointer
 *              database record
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
status_t ToDoInsertNewRecord (DmOpenRef dbP, uint32_t * cursorID, uint32_t * index, CategoryID * catIDsP, uint32_t nCatIDs)
{
	ToDoItemType 		item;
	ToDoItemType	 	newItem;
	status_t			err;
	uint32_t			nCols;
	DbSchemaColumnValueType * colVals ;
	uint32_t			rowID;

	// Get the record that the new record will be inserted after.
	DbCursorSetAbsolutePosition(*cursorID, *index);
	err = DbGetColumnValues(dbP, *cursorID, gNCols, (uint32_t*)gColIDs, &colVals);
	ErrFatalDisplayIf (dbGetDataFailed(err), "Error inserting new to do record");

	ToDoColumnValuesToItem(colVals, &item, gNCols);

	// Set the priority and the due date of the new record to the same
	// values as the record we're inserting after.  This will insure
	// that the records are in the proper sort order.
	MemSet(&newItem, sizeof(ToDoItemType), 0) ;
	newItem.dueDate = item.dueDate ;
	newItem.priority = item.priority;

	ToDoItemToColumnValues(colVals, &newItem, &nCols);

	if ((err = DbInsertRow(dbP, kToDoDBSchemaName, nCols, colVals, &rowID)) < errNone)
		goto Exit ;

	DbReleaseStorage(dbP, colVals);

	// Set the category.
	if (nCatIDs && catIDsP && * catIDsP != catIDUnfiled && * catIDsP != catIDAll)
	{
		if ((err = DbSetCategory(dbP, rowID, nCatIDs, catIDsP)) < errNone)
		{
			goto Exit ;
		}
	}

	ToDoDbCursorRequery(cursorID);
	if ((err = DbCursorMoveToRowID(*cursorID, rowID)) < errNone)
		goto Exit;

	// Return an index
	err = DbCursorGetCurrentPosition(*cursorID, index);
	TraceOutput(TL(appErrorClass, "ToDoInsertNewRecord created ID: %d index: %d", rowID, *index));

Exit:
	return err;
}


/************************************************************
 *
 *  FUNCTION: ToDoChangeRecord
 *
 *  DESCRIPTION: Change a record in the ToDo Database
 *
 *  PARAMETERS: database pointer
 *					 database index
 *					 database record
 *					 changed fields
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/14/95
 *
 *  BY: Roger Flores
 *
 *	COMMENTS: LYr 2002 - Updated for new data manager.
 *
 *
 *************************************************************/
status_t ToDoChangeRecord(DmOpenRef dbP, uint32_t cursorID, uint32_t *index, ToDoRecordFieldType changedField, const void * data)
{
	status_t err = errNone;
	uint32_t uid;

	err = DbCursorSetAbsolutePosition(cursorID, *index);
	DbCursorGetCurrentRowID(cursorID, &uid);

	TraceOutput(TL(appErrorClass, "ToDoChangeRecord ID: %d old index: %d", uid, *index));

	if (changedField == toDoPriority)
	{	// data is a little-endian uint16_t , use the first byte only
		err=DbWriteColumnValue(dbP, cursorID, todoPriorityColId, 0, -1, (uint8_t *) data, sizeof(uint8_t));
		goto exit;
	}

	if (changedField == toDoComplete)
	{	// data is a little-endian uint16_t , use the first byte only
		err=DbWriteColumnValue(dbP, cursorID, todoCompletedColId, 0, -1, (uint8_t *) data, sizeof(uint8_t));
		goto exit;
	}

	if (changedField == toDoDueDate)
	{
		err=DbWriteColumnValue(dbP, cursorID, todoDueDateColId, 0, -1, (void*)data, sizeof(DateType));
		goto exit;
	}

	// Change the description field.
	if (changedField == toDoDescription)
	{
		err=DbWriteColumnValue(dbP, cursorID, todoDescriptionColId, 0, -1, (void*)data, (data ? 1+(uint32_t)strlen(data) : 0));
		goto exit;
	}

	// Change the note field
	if (changedField == toDoNote)
	{
		err=DbWriteColumnValue(dbP, cursorID, todoNoteColId, 0, -1, (void*)data, (data ? 1+(uint32_t)strlen(data) : 0));
		goto exit;
	}

exit:
	err = ToDoDbCursorRequery(&cursorID) ;
	err = DbCursorMoveToRowID(cursorID,uid);
	err = DbCursorGetCurrentPosition(cursorID, index);
	TraceOutput(TL(appErrorClass, "ToDoChangeRecord ID: %d new index: %d, errCode: %d",uid,*index,err));
	return err ;
}


/***********************************************************************
 *
 * FUNCTION:     ToDoSetDBBackupBit
 *
 * DESCRIPTION:  This routine sets the backup bit on the given database.
 *					  This is to aid syncs with non Palm software.
 *					  If no DB is given, open the app's default database and set
 *					  the backup bit on it.
 *
 * PARAMETERS:   dbP -	the database to set backup bit,
 *								can be NULL to indicate app's default database
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	4/1/99	Initial Revision
 *
 ***********************************************************************/
void ToDoSetDBBackupBit(DmOpenRef dbP)
{
	DmOpenRef 		localDBP;
	DatabaseID			dbID;
	DmDatabaseInfoType dbInfo;
	uint16_t 			attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
	{
		localDBP = DbOpenDatabaseByName (sysFileCToDo, kToDoDBName, dmModeReadWrite, dbShareRead);
		if (localDBP == NULL)  return;
	}
	else
	{
		localDBP = dbP;
	}

	// now set the backup bit on localDBP
	DmGetOpenInfo(localDBP, &dbID, NULL, NULL, NULL);

	MemSet(&dbInfo, sizeof(DmDatabaseInfoType), 0);
	dbInfo.pAttributes = &attributes ;
	DmDatabaseInfo(dbID, &dbInfo);
	attributes |= dmHdrAttrBackup;

	DmSetDatabaseInfo(dbID, &dbInfo);

	// close database if necessary
	if (dbP == NULL)
	{
   		DbCloseDatabase(localDBP);
	}
}


/***********************************************************************
 *
 * FUNCTION:     ToDoGetDatabase
 *
 * DESCRIPTION:  Get the application's database.  Open the database if it
 * exists, create it if neccessary.
 *
 * PARAMETERS:   *dbPP - pointer to a database ref (DmOpenRef) to be set
 *					  mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:     status_t - zero if no error, else the error
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			jmp		10/04/99	Initial Revision
 *
 ***********************************************************************/
status_t ToDoGetDatabase (DmOpenRef *dbPP, uint16_t mode)
{
	status_t 		error = 0;
	DmOpenRef 		dbP;
	uint16_t 		shareMode;

	*dbPP = NULL;

  	if (mode & dmModeWrite)
		shareMode = dbShareRead;
	else if (mode & dmModeReadOnly)
		shareMode = dbShareReadWrite;
	else
		shareMode = dbShareNone;


  	// Find the application's data file.  If it doesn't exist create it.
	dbP = DbOpenDatabaseByName (sysFileCToDo, kToDoDBName, mode, shareMode);
	if (!dbP)
	{
		error = CreateDefaultDatabase() ;
		if (error < errNone)
			return error;

		dbP = DbOpenDatabaseByName (sysFileCToDo, kToDoDBName, mode, shareMode);
		if (!dbP)
			return -1;

		// Set the backup bit.  This is to aid syncs with non Palm software.
		ToDoSetDBBackupBit(dbP);
	}

	*dbPP = dbP;
	return 0;
}


/***********************************************************************
 *
 * FUNCTION:	ToDoOpenIOCursor
 *
 * DESCRIPTION:	Opens a cursor on ToDoDB with catIDAll and no sort order
 *
 * PARAMETERS:	->  dbP
 *				<-> cursorIDP
 *
 * RETURNED:	err
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			LYr		10/14/03		Initial Revision
 *
 ***********************************************************************/
status_t ToDoOpenIOCursor(DmOpenRef dbP, uint32_t * cursorIDP)
{
	CategoryID catID = catIDAll;
	return ToDoOpenCursor(dbP, cursorIDP, &catID, 1, noSortOrder);
}


/***********************************************************************
 *
 * FUNCTION:	ToDoOpenCursor
 *
 * DESCRIPTION:	Opens a cursor on ToDoDB
 *
 * PARAMETERS:	->  dbP
 *				<-> cursorIDP
 *				-> catIDsP
 *				-> nCatIDs
 *				-> alphabeticSort
 *
 * RETURNED:	err
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			LYr		7/15/03		Initial Revision
 *
 ***********************************************************************/
status_t ToDoOpenCursor(DmOpenRef dbP, uint32_t * cursorIDP, CategoryID * catIDsP, uint32_t nCatIDs, uint8_t sortOrder)
{
	status_t err = errNone ;
	char query [255] ;
	char str1[catCategoryNameLength], str2[catCategoryNameLength] ;
	CategoryID tmpCatID, * allCategories;
	uint32_t i, nAllCats;
	uint32_t flags = dbCursorEnableCaching;
	Boolean	orderChanged;

	// Close the last cursor if there was one
	ToDoCloseCursor(cursorIDP);

	strcpy(query, kToDoDBSchemaName);
	if (sortOrder != noSortOrder)
		strcat(query, SortOrderStr[sortOrder % 2]);

	TraceOutput(TL(appErrorClass, "ToDoOpenCursor sort: %d \"%s\"",sortOrder,query));

	CatMgrNumCategories(dbP, &nAllCats);
	if (nAllCats && sortingByCategories(sortOrder) && catIDsP && * catIDsP == catIDAll)
	{
		flags |= dbCursorSortByCategory;
		allCategories = MemPtrNew((nAllCats) * sizeof(CategoryID *)) ;
		for (i=0; i<nAllCats; i++)
			CatMgrGetID(dbP, i, allCategories + i);
		// Now sort category table by names
		orderChanged = true;
		while (orderChanged)
		{
			orderChanged = false;
			for (i=0; i<nAllCats-1; i++)
			{
				CatMgrGetName(dbP, allCategories[i], str1);
				CatMgrGetName(dbP, allCategories[i+1], str2);
				if (StrCaselessCompare(str1, str2) > 0)
				{
					orderChanged = true;
					tmpCatID = allCategories[i];
					allCategories[i] = allCategories[i+1];
					allCategories[i+1] = tmpCatID;
				}
			}
		}

		err = DbCursorOpenWithCategory(dbP, query, flags, nAllCats, allCategories, DbMatchAny, cursorIDP);
		MemPtrFree(allCategories);
	}
	else
	{
		if (nCatIDs == 0 || catIDsP == NULL || (nCatIDs == 1 && *catIDsP == catIDUnfiled))
			err = DbCursorOpenWithCategory(dbP, query, flags, 0, NULL, DbMatchAny, cursorIDP);
		else
			err = DbCursorOpenWithCategory(dbP, query, flags, nCatIDs, catIDsP, DbMatchAny, cursorIDP);
	}

	return err ;
}

/***********************************************************************
 *
 * FUNCTION:	ToDoCloseCursor
 *
 * DESCRIPTION:	Close a cursor on ToDoDB
 *
 * PARAMETERS:	-> cursorIDP
 *
 * RETURNED:	err
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			LYr		7/15/03		Initial Revision
 *
 ***********************************************************************/
status_t ToDoCloseCursor(uint32_t * cursorIDP)
{
	status_t err = errNone ;

	if (*cursorIDP != dbInvalidCursorID)
	{
		err = DbCursorClose(*cursorIDP);
		*cursorIDP = dbInvalidCursorID;
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:	ToDoDbCursorRequery
 *
 * DESCRIPTION:	Requery a cursor. If it is the main cursor sorting by
 *		categories, reopen it instead to avoid a crash in DbCursorRequery
 *
 * PARAMETERS:	-> cursorIDP
 *
 * RETURNED:	err
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			LYr		12/10/03	Initial Revision
 *
 ***********************************************************************/
status_t ToDoDbCursorRequery(uint32_t * cursorIDP)
{
	TraceOutput(TL(appErrorClass, "ToDoDbCursorRequery %lx", * cursorIDP));
	if (cursorIDP == &gListViewCursorID && sortingByCategories(CurrentSortId))
	{
		TraceOutput(TL(appErrorClass, "ToDoDbCursorRequery reopening %lx", * cursorIDP));
		return ToDoOpenCursor(ToDoDB, cursorIDP, &CurrentCategory, 1, CurrentSortId);
	}
	return DbCursorRequery(* cursorIDP);
}
