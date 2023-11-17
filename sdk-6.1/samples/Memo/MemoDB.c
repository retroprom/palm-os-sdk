/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoDB.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <SchemaDatabases.h>
#include <StringMgr.h>
#include <SystemResources.h>
#include <ErrorMgr.h>
#include <CatMgr.h>
#include <string.h>

#include "MemoDB.h"
#include "MemoMain.h"
#include "MemoRsc.h"

/***********************************************************************
 * Local functions
 ***********************************************************************/
static status_t		MemoDBCreateDatabase(void);

/***********************************************************************
 *
 * FUNCTION:	MemoDBOpenDB
 *
 * DESCRIPTION:	Opens the MemoPad database. Creates it if it doesn't exist
 *
 * PARAMETERS:	None
 *
 * RETURNED:	DmOpenRef of the MemoPad database
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
DmOpenRef MemoDBOpenDatabase(uint16_t mode)
{
	DmOpenRef	dbP;
	status_t			err;

	// Open it
	dbP = DbOpenDatabaseByName(sysFileCMemo, kMemoDBName, mode, dbShareNone);

	// Try to create from defaultDB resource if open failed
	if (dbP == NULL && MemoDBCreateDefaultDatabase() >= errNone)
	{
		dbP = DbOpenDatabaseByName(sysFileCMemo, kMemoDBName, mode, dbShareNone);
	}

	// Create a brand new DB if open fails again
	if (dbP == NULL)
	{
		// Create it
		err = MemoDBCreateDatabase();
		ErrFatalDisplayIf(err < errNone, "Can't create MemoPad database");

		// Same player try again
		dbP = DbOpenDatabaseByName(sysFileCMemo, kMemoDBName, mode, dbShareNone);
		ErrFatalDisplayIf(dbP == NULL, "Can't open gMemoDB database");

	}

	// Really failed
	ErrFatalDisplayIf(dbP == NULL, "Can't open MemoPad database");

	return dbP;
}


/***********************************************************************
 *
 * FUNCTION:	MemoDBOpenIOCursor
 *
 * DESCRIPTION:	Opens a cursor on ToDoDB with catIDAll and no sort order
 *
 * PARAMETERS:	<-> dbPP
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
status_t MemoDBOpenIOCursor(DmOpenRef * dbPP, uint32_t * cursorIDP)
{
	CategoryID catID = catIDAll;
	return MemoDBOpenCursor(dbPP, cursorIDP, &catID, 1, false);
}


/***********************************************************************
 *
 * FUNCTION:	MemoDBOpenCursor
 *
 * DESCRIPTION:	Opens a cursor on MemoDB
 *
 * PARAMETERS:	<-> dbPP
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
status_t MemoDBOpenCursor(DmOpenRef	* dbPP, uint32_t * cursorIDP, CategoryID * catIDsP, uint32_t nCatIDs, Boolean alphabeticSort)
{
	status_t err = errNone ;
	uint32_t flags = dbCursorEnableCaching;

	// Reopen the DB with the default settings
	if (! * dbPP )
		* dbPP = MemoDBOpenDatabase((gPrivateRecordVisualStatus == hidePrivateRecords) ?
									 dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret));

	// Close the last cursor if there was one
	if (* cursorIDP != dbInvalidCursorID)
	{
		DbCursorClose(* cursorIDP);
	}

	// Open the cursor with the first category only if multiple categories are selected
	if (catIDsP == NULL || *catIDsP == catIDUnfiled)
		nCatIDs = 0 ;
	else if (nCatIDs > 1)
		nCatIDs = 1 ;

	// Open the cursor
	if (alphabeticSort)
		err = DbCursorOpenWithCategory(* dbPP, kMemoDBSchemaName kMemoDBAlphabeticSortString, flags, nCatIDs, catIDsP, DbMatchAny, cursorIDP);
	else
		err = DbCursorOpenWithCategory(* dbPP, kMemoDBSchemaName, flags, nCatIDs, catIDsP, DbMatchAny, cursorIDP);

	// The current category might have been deleted at HotSync
	if (err == dmErrInvalidCategory)
	{
		CategoryID all = catIDAll ;
		nCatIDs = 1 ;
		if (alphabeticSort)
			err = DbCursorOpenWithCategory(* dbPP, kMemoDBSchemaName kMemoDBAlphabeticSortString, flags, nCatIDs, &all, DbMatchAny, cursorIDP);
		else
			err = DbCursorOpenWithCategory(* dbPP, kMemoDBSchemaName, flags, nCatIDs, &all, DbMatchAny, cursorIDP);
		ChangeCategory(&all, nCatIDs);
	}


	return err ;
}


/************************************************************
 *
 *  FUNCTION: MemoGetSchemaColumnMaxSize
 *
 *  DESCRIPTION: Retrieve from the schema the maximum
 *				size for specified column
 *
 *  PARAMETERS: -> columnID: the column ID
 *
 *  RETURNS: Maximum size for this column
 *
 *  CREATED: 10/09/02
 *
 *  BY: PLe
 *
 *************************************************************/
uint32_t MemoGetSchemaColumnMaxSize(DmOpenRef dbP, uint32_t columnID)
{
	DbSchemaColumnDefnType * thisColumnDefinitionP = NULL;
	uint32_t thisColumnMaxSize = 0;

	if (DbGetColumnDefinitions(dbP, kMemoDBSchemaName, 1, &columnID,
		&thisColumnDefinitionP) >= errNone)
	{
		DbgOnlyFatalErrorIf(thisColumnDefinitionP[0].id != columnID,
			"Retrieved wrong column ID");
		thisColumnMaxSize = thisColumnDefinitionP[0].maxSize;
		DbReleaseStorage(dbP, thisColumnDefinitionP);
	}

	return thisColumnMaxSize;
}


/***********************************************************************
 *
 * FUNCTION:	MemoDBCreateRecord
 *
 * DESCRIPTION:	Create a new empty record
 *
 * PARAMETERS:	->	dbP: the memo database ref
 *				<-	recordIndex: the new record index is returned here
 *				->	numCategories: The number of categories
 *				->	categoriesP: the categoryID array to affect to the record
 *
 * RETURNED:	errNone if no error.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
status_t MemoDBCreateRecord(DmOpenRef dbP, uint32_t cursorID, uint32_t *recordID, uint32_t numCategories, CategoryID *categoriesP)
{
	status_t	err;

	if ((err = DbInsertRow(dbP, kMemoDBSchemaName, 0, NULL, recordID)) < errNone)
	{
		*recordID = noRecordSelected;
		goto Exit;
	}

	if (categoriesP && *categoriesP != catIDAll &&
		(err = DbSetCategory(dbP, *recordID, numCategories, categoriesP)) < errNone)
	{
		DbRemoveRow(gMemoDB, *recordID);
		goto Exit;
	}

	err = DbCursorRequery(cursorID);
	if (err)
	{
		DbRemoveRow(gMemoDB, *recordID);
		goto Exit;
	}

Exit:
	return err;
}

/***********************************************************************
 *
 * FUNCTION:	MemoDBLoadRecord
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
status_t MemoDBLoadRecord(DmOpenRef dbP, uint32_t recordID, void **dataPP, uint32_t *dataSizeP)
{
	status_t		err;
	uint32_t	size = 0;
	void *	dataP = NULL;

	err = DbGetColumnValue(dbP, recordID, kMemoDBColumnID, 0, &dataP, &size);
	if (err < errNone)
		return err;

	if (dataPP)
		*dataPP = dataP;

	if (dataSizeP)
		*dataSizeP = size;

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	MemoDBLoadRecordToHandle
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
status_t MemoDBLoadRecordToHandle(DmOpenRef dbP, uint32_t recordID, MemHandle *textH)
{
	status_t		err;
	void *	dataP = NULL;
	uint32_t	dataSize = 0;
	char *	textP;

	err = MemoDBLoadRecord(dbP, recordID, &dataP, &dataSize);
	if (err < errNone)
	{
		*textH = NULL;
		return err;
	}

	*textH = MemHandleNew(dataSize + 1);
	ErrFatalDisplayIf(*textH == NULL, "Out of memory!");

	textP = MemHandleLock(*textH);
	memmove(textP, dataP, dataSize);

	*(textP + dataSize) = nullChr;

	MemHandleUnlock(*textH);

	DbReleaseStorage(dbP, dataP);

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	MemoDBAppendRecord
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
status_t MemoDBAppendRecord(DmOpenRef dbP, uint32_t *recordID, void *dataP, uint32_t dataSize)
{
	uint32_t	offset = 0;
	char *		colDataP = NULL;
	int32_t		bytesToReplace = -1; // replace everything
	status_t	err;
	char *		writeDataP;

	if (!dataP || !dataSize)
		return -1;

	// Get the current data size
	err = DbGetColumnValue(dbP, *recordID, kMemoDBColumnID, 0, (void**)&colDataP, &offset);
	if (err >= errNone)
	{
		if (offset)
		{
			if (*(colDataP + offset - 1) == nullChr)
			{
				offset--;
				bytesToReplace = 1; // Will replace the trailing \0
			}
			else
			{
				bytesToReplace = 0; // Insert
			}
		}

		if (colDataP)
			DbReleaseStorage(dbP, colDataP);
	}

	// Append (add nullChr if necessary)
	if (((char*)dataP)[dataSize-1] != nullChr)
	{
		writeDataP = (char*)MemPtrNew(dataSize+1) ;
		strncpy(writeDataP,(char*)dataP,dataSize);
		writeDataP[dataSize] = nullChr ;
		err = DbWriteColumnValue(dbP, *recordID, kMemoDBColumnID, offset, bytesToReplace, writeDataP, dataSize+1);
		MemPtrFree(writeDataP);
	}
	else
		err = DbWriteColumnValue(dbP, *recordID, kMemoDBColumnID, offset, bytesToReplace, dataP, dataSize);
	return err ;
}

/***********************************************************************
 *
 * FUNCTION:	MemoDBAppendRecordFromHandle
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
status_t MemoDBAppendRecordFromHandle(DmOpenRef dbP, uint32_t *recordID, MemHandle textH)
{
	char *	textP;

	textP = MemHandleLock(textH);
	if (!textP)
		return -1;

	return MemoDBAppendRecord(dbP, recordID, textP, (uint32_t)(strlen(textP) + 1));
}


/***********************************************************************
 *
 * FUNCTION:	MemoDBNumVisibleRecordsInCategory
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		02/12/03	Initial Revision
 *
 ***********************************************************************/
uint32_t MemoDBNumVisibleRecordsInCategory(uint32_t cursorID)
{
	return DbCursorGetRowCount(cursorID);
}

/************************************************************
 *
 *  FUNCTION: MemoChangeSortOrder
 *
 *  DESCRIPTION: Change the Memo Database's sort order
 *
 *  PARAMETERS: database pointer
 *				TRUE if sort by company
 *
 *  RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/96	Initial Revision
 *
 *************************************************************/
status_t MemoDBChangeSortOrder(DmOpenRef dbP, uint32_t * cursorIDP, Boolean alphabeticSort)
{
	status_t err;

	if (* cursorIDP != dbInvalidCursorID) {
		DbCursorClose(* cursorIDP);

		if (alphabeticSort) {
			err = DbCursorOpenWithCategory(dbP, kMemoDBSchemaName kMemoDBAlphabeticSortString, 0,
										   gCurrentNumCategories, gCurrentCategoriesP, DbMatchAny, cursorIDP);
		} else {
			err = DbCursorOpenWithCategory(dbP, kMemoDBSchemaName, 0, gCurrentNumCategories,
										   gCurrentCategoriesP, DbMatchAny, cursorIDP);
		}
	} else {
		err = errNone;
	}

	if (!err) {
		gSortAlphabetically = alphabeticSort;
	}

	return err;
}

/***********************************************************************
 *
 * FUNCTION:     MemoSetDBBackupBit
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
void MemoDBSetBackupBit(DmOpenRef dbP)
{
	DmOpenRef			localDBP;
	DatabaseID			dbID;
	DmDatabaseInfoType	databaseInfo;
	uint16_t				attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
	{
		localDBP = DbOpenDatabaseByName(sysFileCMemo, kMemoDBName, dmModeReadWrite, dbShareNone);
		if (localDBP == NULL)
			return;
	}
	else
	{
		localDBP = dbP;
	}

	// now set the backup bit on localDBP
	DmGetOpenInfo(localDBP, &dbID, NULL, NULL, NULL);

	MemSet(&databaseInfo, sizeof(DmDatabaseInfoType), 0);
	databaseInfo.pAttributes = &attributes;
	DmDatabaseInfo(dbID, &databaseInfo);
	attributes |= dmHdrAttrBackup;
	DmSetDatabaseInfo(dbID, &databaseInfo);

	// close database if necessary
	if (dbP == NULL)
	{
		DbCloseDatabase(localDBP);
	}
}

/***********************************************************************
 *
 * FUNCTION:	MemoDBCreateDefaultDatabase
 *
 * DESCRIPTION:	Create the MemoDB from resource image
 *
 * PARAMETERS:	None
 *
 * RETURNED:	errNone if no error
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/31/02	Initial Revision
 *
 ***********************************************************************/
status_t MemoDBCreateDefaultDatabase(void)
{
	MemHandle	defaultDBH;
	void *		defaultDBP;
	DatabaseID	dbID;
	status_t			err = errNone;

	if ((defaultDBH = DmGetResource(gApplicationDbP, 'scdb', MemoDefaultDB)) == NULL)
		return DmGetLastErr();

	if ((defaultDBP = MemHandleLock(defaultDBH)) == NULL)
	{
		DmReleaseResource(defaultDBH);
		return memErrNotEnoughSpace;
	}

	if ((err = DmCreateDatabaseFromImage(defaultDBP, &dbID)) >= errNone)
		MemoDBSetBackupBit(NULL);

	MemHandleUnlock(defaultDBH);
	DmReleaseResource(defaultDBH);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:	MemoDBCreateDatabase
 *
 * DESCRIPTION:	Create the schema based database
 *
 * PARAMETERS:	None
 *
 * RETURNED:	errNone if no error
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/2/02		Initial Revision
 *
 ***********************************************************************/
static status_t MemoDBCreateDatabase(void)
{
	DbTableDefinitionType	dbSchema;
	DbSchemaColumnDefnType	column;
	DatabaseID			dbID;

	// Initialize schema structure
	dbSchema.numColumns		= kMemoDBSchemaNumCols;
	dbSchema.columnListP	= &column;
	strcpy(dbSchema.name, kMemoDBSchemaName);

	// Initialize *the* Memo column
	MemSet(&column, sizeof(DbSchemaColumnDefnType), 0);
	column.id				= kMemoDBColumnID;
	column.maxSize			= kMemoDBColumnMaxSize;
	column.type				= kMemoDBColumnType;
	strlcpy(column.name, kMemoDBColumnName, dmDBNameLength);

	// Create database
	return DbCreateDatabase(kMemoDBName, sysFileCMemo, kMemoDBType, kMemoDBNumSchemas, &dbSchema, &dbID);
}
