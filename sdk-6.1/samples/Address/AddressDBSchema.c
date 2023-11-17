/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDBSchema.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      Address Manager routines
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <BuildDefaults.h>
#include <SystemResources.h>
#include <SysUtils.h>
#include <UIResources.h>
#include <CatMgr.h>
#include <ErrorMgr.h>
#include <StringMgr.h>
#include <TraceMgr.h>
#include <string.h>
#include <Preferences.h>

// for min/max Macros
#include <PalmTypesCompatibility.h>

#include "AddressDBSchema.h"
#include "Address.h"
#include "AddressTools.h"
#include "AddressRsc.h"
#include "AddressU32List.h"
#include "AddressPrefs.h"

/***********************************************************************
 *
 * FUNCTION:	AddressDBCreateDefaultDatabase
 *
 * DESCRIPTION:	This routine creates the default database from the
 *				saved image in a resource in the application.
 *
 * PARAMETERS:	dbP	- Address Book database reference
 *
 * RETURNED:	0 - if no error otherwise appropriate error value
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * vivek	08/17/00	Initial Revision
 * LFe		10/21/02	Use new Database Manager. - Add parameter
 *						Don't create anymore an empty database
 *
 ***********************************************************************/
status_t AddressDBCreateDefaultDatabase(DmOpenRef appDbP)
{
	MemHandle	resH;
	status_t	error;
	DatabaseID	dbID;

	// Attempt to get our default data image and create our database.
	resH = DmGetResource(appDbP, sysResTDefaultSchemaDB, sysResIDDefaultDB);

	if (resH)
	{
		error = DmCreateDatabaseFromImage(DmHandleLock(resH), &dbID);

		// Set the backup bit on the new database.
		if (!error)
			AddressDBSetDatabaseAttributes(dbID, dmHdrAttrBackup);

		DmHandleUnlock(resH);
		DmReleaseResource(resH);
	}
	else error = DmGetLastErr();

	return error;
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBNewRecord
 *
 * DESCRIPTION:	Create a new empty record.
 *
 * PARAMETERS:	->	dbRef	- AddressBook database pointer
 *				<-	rowIDP	- Pointer to new record ID
 *
 * RETURNED:	status_t		- >=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		10/24/02	Initial revision
 *
 ***********************************************************************/
status_t AddressDBNewRecord(DmOpenRef dbRef, uint32_t *rowIDP)
{
	return DbInsertRow(dbRef, kAddressDefaultTableName, 0, NULL, rowIDP);
}


 /***********************************************************************
 *
 * FUNCTION:	PrvAddressDBCheckAndSetSortIndexes
 *
 * DESCRIPTION:	Check if the 2 default sort indexes exist. if not
 *				add them in the database.
 *
 * PARAMETERS:	dbRef	- AddressBook database pointer
 *
 * RETURNED:	status_t		- >=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/23/02	Initial revision
 *	LFe	11/14/02	Add the check.
 *
 ***********************************************************************/
status_t PrvAddressDBCheckAndSetSortIndexes(DmOpenRef dbRef)
{
	status_t	err = errNone;
	uint16_t	index = 0;
	char		orderBy[kMaxOrderBySQLStrSize];
	char		sqlStr[kMaxOrderBySQLStrSize];

	gOrderByCount = 0;

	do
	{
		SysStringByIndex(gApplicationDbP, OrderBySQLQueriesStrList, index++, orderBy, kMaxOrderBySQLStrSize);

		if (orderBy[0])
		{
			// + 2 = <space> + nullChr
			DbgOnlyFatalErrorIf(strlen(kAddressDefaultTableName) + strlen(orderBy) + 2 > kMaxOrderBySQLStrSize, "buffer too small");
			sprintf(sqlStr, "%s %s", kAddressDefaultTableName, orderBy);

			if(!DbHasSortIndex(dbRef, sqlStr))
				err = DbAddSortIndex(dbRef, sqlStr);

			gOrderByCount++;
		}

	} while (orderBy[0] && (err >= errNone));

	if (!gOrderByCount && (err >= errNone))
		err = dmErrInvalidSortIndex;

	return err;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressDBCheckAndSetAppInfo
 *
 * DESCRIPTION:	Check if the AppInfo schema has been set
 *				correctly. If not, remove the old one (if any)
 *				set the schema for the App Info block and create
 *				an empty record.
 *
 * PARAMETERS:	dbRef		- Application data database reference
 *
 * RETURNED:	status_t			- >=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	11/05/02	Initial revision
 *
 ***********************************************************************/
static status_t PrvAddressDBCheckAndSetAppInfo(DmOpenRef dbRef, uint32_t *rowIDP)
{
	status_t				err = errNone;
	uint32_t				cursorID;
	uint32_t				rowID = dbInvalidRowID;
	uint32_t				numRows;
	DbSchemaColumnValueType	colValue;

	if ((err = DbCursorOpen(dbRef, kAddressAppInfoTableName, 0, &cursorID)) < errNone)
		goto Exit;

	if ((numRows = DbCursorGetRowCount(cursorID)) > 0)
		err = DbCursorGetRowIDForPosition(cursorID, kFirstRowIndex, &rowID);

	if (!numRows || err < errNone)
	{
		memset(&colValue, 0, sizeof(DbSchemaColumnValueType));
		colValue.data = (DbSchemaColumnData*)&gFormatLocale.country;
		colValue.dataSize = sizeof(gFormatLocale.country);
		colValue.columnID = kAddressAppInfoColumnIDCountry;
		err = DbInsertRow(dbRef, kAddressAppInfoTableName, 1, &colValue, &rowID);
	}

	err = DbCursorClose(cursorID);

Exit:
	if (rowIDP)
		*rowIDP = rowID;

	return err;
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBOpenDatabase
 *
 * DESCRIPTION:	Open the Address Book database and create a new one if
 *				necessary. Sort order not specified.
 *
 * PARAMETERS:	appDbRef	- Application database reference
 *				dbRef		- Application data database reference
 *				openMode	- how to open the database
 *
 * RETURNED:	status_t			- >=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	10/11/02	Initial revision
 *
 ***********************************************************************/
status_t AddressDBOpenDatabase (DmOpenRef appDbRef, uint16_t openMode, DmOpenRef *dbRef)
{
	status_t		error = errNone;
	DmOpenRef		dbP;
	DbShareModeType	shareMode = dbShareRead;

	*dbRef = NULL;

	// The DB is opened in read only mode only by the find mechanism. We need to specify
	// dbShareModeReadWrite only if the address book is already sub-launched (by the phone
	// pad for example)
	if ((openMode & dmModeReadWrite) == dmModeReadOnly)
		shareMode = dbShareReadWrite;

	// Find the application's data file.  If it doesn't exist create it.
	dbP = DbOpenDatabaseByName(sysFileCAddress, kAddrDBName, openMode, shareMode);

	if (!dbP)
	{
		error = AddressDBCreateDefaultDatabase(appDbRef);

		if (error >= errNone)
		{
			dbP = DbOpenDatabaseByName(sysFileCAddress, kAddrDBName, openMode, shareMode);

			if (!dbP)
				error = DmGetLastErr();
		}
	}

	// Check the App Info block and the default sort info keys
	if (error >= errNone)
		error = PrvAddressDBCheckAndSetAppInfo(dbP, NULL);

	if (error >= errNone)
		error = PrvAddressDBCheckAndSetSortIndexes(dbP);

	*dbRef = dbP;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBOpenCursor
 *
 * DESCRIPTION:	Open the address book cursor with the current categories.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * JHa		07/12/03	Initial revision
 *
 ***********************************************************************/
status_t AddressDBOpenCursor(void)
{
	status_t err;
	char	sql[kMaxOrderBySQLStrSize];

	if (gCursorID != dbInvalidCursorID)
	{
		err = DbCursorClose(gCursorID);
		DbgOnlyFatalErrorIf(err, "Failed to close existing cursor");
	}

	// Verify gCurrentCategoriesP before re-opeing the cursor.
	if (gCurrentNumCategories > 0 && *gCurrentCategoriesP != catIDAll)
	{
		// Check if the global category still exists cause it may have been deleted
		ToolsCheckCategories(gCurrentCategoriesP, &gCurrentNumCategories);
		
		// Free memory if there are no more valid category IDs
		if (gCurrentNumCategories == 0)
		{
			gCurrentNumCategories = 1;
			*gCurrentCategoriesP = catIDAll;
		}
	}

	/* Future requests
	Sort by name request:
	"WHERE LastName PS_LIKE '%s' OR FirstName PS_LIKE '%s' OR (WorkCompany PS_LIKE '%s' AND LastName IS NULL AND FirstName IS NULL)"

	Sort by company request:
	"WHERE WorkCompany PS_LIKE '%s' OR LastName PS_LIKE '%s' OR (FirstName PS_LIKE '%s' AND (LastName IS NULL OR WorkCompany IS NULL))"
	*/

	// Create the SQL request and open the cursor
	sprintf(sql, "%s %s", kAddressDefaultTableName, gCurrentOrderByStr);
	err = DbCursorOpenWithCategory(gAddrDB, sql, dbCursorEnableCaching,
								   gCurrentNumCategories, gCurrentCategoriesP,
								   DbMatchAny, &gCursorID);

	DbgOnlyFatalErrorIf(err, "Failed to open cursor");

	// Restore cursor position in case of reopen.
	if (err >= errNone && gCurrentRowID != dbInvalidRowID)
		DbCursorMoveToRowID(gCursorID, gCurrentRowID);

	return err;
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBColumnsContainData
 *
 * DESCRIPTION:	Checks for data in passed columns list
 *
 * PARAMETERS:	->	numColumns: number of columns to check
 *				->	columnsP: and their column IDs
 *
 * RETURNED:	true if at least one of the passed columns contains data,
 *				false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		10/20/2003	Initial draft
 *
 ***********************************************************************/
Boolean AddressDBColumnsContainData(uint32_t rowID, uint32_t numColumns, uint32_t *columnsP)
{
	status_t					err;
	DbSchemaColumnValueType	*	columnValuesP = NULL;

	if (!numColumns)
		return false;

	err = DbGetColumnValues(gAddrDB, rowID, numColumns, columnsP, &columnValuesP);

	if (columnValuesP)
		DbReleaseStorage(gAddrDB, columnValuesP);

	return (Boolean)(err >= errNone || err == dmErrOneOrMoreFailed);
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBRecordContainsData
 *
 * DESCRIPTION:	Check if one of the visible column contains data
 *
 * PARAMETERS:	rowID	- The record ID to check
 *
 * RETURNED:	true if record contains data, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		02/10/03	Initial revision
 *
 ***********************************************************************/
Boolean AddressDBRecordContainsData(uint32_t rowID)
{
	uint32_t *	columnIDsP = NULL;
	uint32_t	numCols;
	Boolean		containsData;
	uint16_t	tabIndex;
	uint16_t	numElem;

	// Build the columnID list
	if (DbNumColumns(gAddrDB, kAddressDefaultTableName, &numCols) < errNone)
		return true; // Let say that this record contains data

	columnIDsP = MemPtrNew(sizeof(uint32_t) * numCols);
	ErrFatalDisplayIf(!columnIDsP, "Out of memory!");

	numCols = 0;
	for (tabIndex = 0; tabIndex < gBookInfo.numTabs; tabIndex++)
	{
		if (!TabIsVisible(gBookInfo.tabs[tabIndex].tabInfo.tabId))
			continue;

		for (numElem = 0; numElem < gBookInfo.tabs[tabIndex].numElement; numElem++)
		{
			//if (gBookInfo.tabs[tabIndex].columnsPropertiesP[numElem].
			columnIDsP[numCols++] = gBookInfo.tabs[tabIndex].columnsPropertiesP[numElem].columnId;
		}
	}

	// Note column
	columnIDsP[numCols++] = kAddrColumnIDNote;

	containsData = AddressDBColumnsContainData(rowID, numCols, columnIDsP);

	if (columnIDsP)
		MemPtrFree(columnIDsP);

	return containsData;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressDBGetDBLocaleCountry
 *
 * DESCRIPTION:	Check the database country versus the system & rom
 *				country. If the database country is different from
 *				system country, the function will check if resources
 *				exist to rename some fields. If resources doesn't exist
 *				for this country, ROM locale (default) will be used.
 *
 * PARAMETERS:	dbRef:		AddressBook database pointer
 *
 * RETURNED:	status_t:	>=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		09/05/03	Initial revision
 *
 ***********************************************************************/
static status_t PrvAddressDBGetDBLocaleCountry(DmOpenRef dbRef, uint32_t cursorID, LmCountryType* dbCountry)
{
	status_t	err;
	uint32_t	size;

	*dbCountry = lmAnyCountry;

	if ((err = DbCursorMoveFirst(cursorID)) < errNone)
		return err;

	size = sizeof(LmCountryType);

	return DbCopyColumnValue(dbRef, cursorID, kAddressAppInfoColumnIDCountry, 0, (void*)dbCountry, &size);
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBCheckCountry
 *
 * DESCRIPTION:	Check the database country versus the system & rom
 *				country. If the database country is different from
 *				system country, the function will check if resources
 *				exist to rename some fields. If resources doesn't exist
 *				for this country, ROM locale (default) will be used.
 *
 * PARAMETERS:	dbRef:		AddressBook database pointer
 *
 * RETURNED:	status_t:	>=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		09/05/03	Initial revision
 *
 ***********************************************************************/
status_t AddressDBCheckCountry(DmOpenRef dbRef)
{
	LmCountryType	dbCountry;
	DmResourceID	dbCountryResId, newCountryResId, newStrListId = 0;
	MemHandle		dbFieldInfoResH = NULL, dbStrListResH = NULL;
	MemHandle		newFieldInfoResH = NULL, newStrListResH = NULL;
	uint16_t		dbFieldInfoItemNum = 0, dbStrListItemNum = 0;
	uint16_t		newFieldInfoItemNum = 0, newStrListItemNum = 0;
	uint16_t		columnIndex, listIdx;
	status_t		err = errNone;
	uint32_t		cursorID;
	uint32_t		rowID;
	uint32_t*		newFiedldInfoP = NULL;
	uint32_t*		dbFieldInfoP = NULL;
	uint32_t		fieldInfo, *fieldInfoP = NULL;
	uint32_t		columnCount, columnID;
	uint32_t		numBytes;
	char*			strListP;
	char			fieldName[dbDBNameLength + 1];

	// Open a cursor on the AppInfo block schema
	if ((err = DbCursorOpen(dbRef, kAddressAppInfoTableName, 0, &cursorID)) < errNone)
		return err;

	// Get the database country. Should exist. Has been initialized at the database creation.
	PrvAddressDBGetDBLocaleCountry(dbRef, cursorID, &dbCountry);

	// If the Database country is different from the system coutry, check and update it.
	if (dbCountry == gFormatLocale.country)
		goto Exit;

	// First, check if a resource exist for the system country or ROM country.
	// ROM country resources dosn't exist. It's the default schema.
	if ((newCountryResId = ToolsCountryToResourceID(gFormatLocale.country)) != 0)
		newFieldInfoResH = U32ListGetList(gApplicationDbP, kAddrRenameFieldFieldInfoU32LBaseID + newCountryResId);

	// Then check if a resource exist for the database country.
	if ((dbCountryResId = ToolsCountryToResourceID(dbCountry)) != 0)
		dbFieldInfoResH = U32ListGetList(gApplicationDbP, kAddrRenameFieldFieldInfoU32LBaseID + dbCountryResId);

	// If no resources exist for the country saved in the database and for the system country
	// we don't have to rename fields, only to save the system country as the database country.
	// That means the database will/still display the default ROM database field.
	// This could occurs when a user switch between countries that don't have 'rename resources'.
	if (!newFieldInfoResH && !dbFieldInfoResH)
		goto Save;

	// If newFieldInfoResH == NULL that means we are going to use the ROM locale
	// There is no resource for the ROM locale as the default schema definition is the ROM locale.

	// Get the String list that belong to the FieldInfo list, if exist
	if (newFieldInfoResH)
	{
		newStrListId = kAddrRenameFieldStrListBaseID + newCountryResId;
		newStrListResH = DmGetResource(gApplicationDbP, strListRscType, newStrListId);

		// Get the 2 list items number and check integrity. Both have to have the same number of items.
		newFieldInfoItemNum = (uint16_t)U32ListGetItemNum(NULL, NULL, newFieldInfoResH);

		// Get the number of items in the string list.
		strListP = (char*)DmHandleLock(newStrListResH);
		// Skip over prefix string, then get the entry count.
		strListP += strlen(strListP) + 1;
		newStrListItemNum = *strListP++;
		newStrListItemNum = (newStrListItemNum << 8);
		newStrListItemNum = newStrListItemNum +  *(uint8_t *)strListP++;
		DmHandleUnlock(newStrListResH);

		if (newFieldInfoItemNum != newStrListItemNum)
		{
			ErrNonFatalDisplay("resource don't match");
			goto Release;
		}

		newFiedldInfoP = DmHandleLock(newFieldInfoResH);
	}

	// Get the String list that belong to the FieldInfo list, if exist
	if (dbFieldInfoResH)
	{
		dbStrListResH = DmGetResource(gApplicationDbP, strListRscType, kAddrRenameFieldStrListBaseID + dbCountryResId);

		// Get the 2 list items number and check integrity. Both have to have the same number of items.
		dbFieldInfoItemNum = (uint16_t)U32ListGetItemNum(NULL, NULL, dbFieldInfoResH);

		// Get the number of items in the string list.
		strListP = (char*)DmHandleLock(dbStrListResH);
		// Skip over prefix string, then get the entry count.
		strListP += strlen(strListP) + 1;
		dbStrListItemNum = *strListP++;
		dbStrListItemNum = (dbStrListItemNum << 8);
		dbStrListItemNum = dbStrListItemNum +  *(uint8_t *)strListP++;
		DmHandleUnlock(dbStrListResH);

		if (dbFieldInfoItemNum != dbStrListItemNum)
		{
			ErrNonFatalDisplay("resource don't match");
			goto Release;
		}

		dbFieldInfoP = DmHandleLock(dbFieldInfoResH);
	}

	// Now, check all the fields in the database schema.
	// If a field is already renamed for the current db, remove it, then
	// if the field exist in the new list, add it.

	// Get the number of column in the database schema
	err = DbNumColumns(dbRef, kAddressDefaultTableName, &columnCount);

	for (columnIndex = 0; columnIndex < columnCount; columnIndex++)
	{
		// Get the column ID first, then the column FieldInfo. Should not exist. In this case, loop without error
		if ((err = DbGetColumnID(dbRef, kAddressDefaultTableName, columnIndex, &columnID)) >= errNone)
			err = DbGetColumnPropertyValue(dbRef, kAddressDefaultTableName, columnID, kAddrColumnPropertyFieldInfo, &numBytes, (void**)&fieldInfoP);

		if (err >= errNone)
			fieldInfo = *fieldInfoP;
		else if (err == dmErrInvalidPropID)
			continue;
		else goto Release;

		if (fieldInfoP)
			DbReleaseStorage(dbRef, fieldInfoP);

		// Reset 'kAddrColumnPropertyNewName' column Property matching with the FldInfoList
		// corresponding to the current country stored in schema Db.
		// If the current DB FldInfoList doesn't exist, dbFieldInfoItemNum = 0
		for (listIdx = 0; listIdx < dbFieldInfoItemNum; listIdx++)
		{
			// If the Tab field is found in the current rename db field list, reset it.
			if ((fieldInfo & dbFieldInfoP[listIdx]) == dbFieldInfoP[listIdx])
			{
				if (AddressDBColumnPropertyExists(dbRef, kAddressDefaultTableName, columnID, kAddrColumnPropertyNewName))
				{
					if ((err = DbRemoveColumnProperty(dbRef, kAddressDefaultTableName, columnID, kAddrColumnPropertyNewName)) < errNone)
					{
						ErrNonFatalDisplay("Can't remove column property");
						goto Release;
					}
				}

				// Doesn't need to check the other field info. we find it. Exit from the loop
				break;
			}
		} // for dbFieldInfoItemNum

		// If the new pref FieldInfo List exists, rename the fields.
		// if the list doesn't exist, newFieldInfoItemNum = 0
		for (listIdx = 0; listIdx < newFieldInfoItemNum; listIdx++)
		{
			if ((fieldInfo & newFiedldInfoP[listIdx]) == newFiedldInfoP[listIdx])
			{
				SysStringByIndex(gApplicationDbP, newStrListId, listIdx, fieldName, dbDBNameLength);
				numBytes = (uint32_t)(strlen(fieldName) + 1);

				if ((err = DbSetColumnPropertyValue(dbRef, kAddressDefaultTableName, columnID, kAddrColumnPropertyNewName, numBytes, &fieldName)) < errNone)
				{
					ErrNonFatalDisplay("Can't set column property");
					goto Release;
				}

				// Doesn't need to check the other field info. we find it. Exit from the loop
				break;
			}
		} // for newFieldInfoItemNum
	} // for columnIndex

Save:

	dbCountry = gFormatLocale.country;

	// Be sure the App Info record exist and get it's rowID
	PrvAddressDBCheckAndSetAppInfo(dbRef, &rowID);

	// Save the preference country in the schema Db.
	if ((err = DbWriteColumnValue(dbRef, rowID, kAddressAppInfoColumnIDCountry, 0, -1, (void*)&dbCountry, sizeof(dbCountry))) < errNone)
		ErrNonFatalDisplay("Can't save country");

Release:

	if (newFiedldInfoP)
		DmHandleUnlock(newFieldInfoResH);

	if (dbFieldInfoP)
		DmHandleUnlock(dbFieldInfoResH);

	if(newFieldInfoResH)
		U32ListReleaseList(newFieldInfoResH);

	if(dbFieldInfoResH)
		U32ListReleaseList(dbFieldInfoResH);

	if (newStrListResH)
		DmReleaseResource(newStrListResH);

	if (dbStrListResH)
		DmReleaseResource(dbStrListResH);

Exit:

	DbCursorClose(cursorID);

	return err;
}

/***********************************************************************
 *
 *  FUNCTION:	AddressDBCheckAndSetDisplayedPhoneColumn
 *
 *  DESCRIPTION: Check and set the 'DisplayedPhone' Column used in list
 *				view.
 *
 *  PARAMETERS: dbRef		-> Application data database reference
 *				bookInfo	-> Ptr to the Schema database
 *				sortID		-> current Sort ID
 *				rowID		-> record ID to be checked
 *
 * RETURNED:	the columnID of the default phone number
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		11/19/02	Initial revision
 *
 ***********************************************************************/
uint32_t AddressDBCheckAndSetDisplayedPhoneColumn(DmOpenRef dbRef, uint32_t rowID)
{
	status_t	err = errNone;
	uint16_t	listID = 0;
	MemHandle	listH = NULL;
	uint32_t *	listP = NULL;
	uint16_t	listCount;
	uint16_t	tabIdx, colPropIdx, listIdx;
	uint32_t	size = sizeof(uint32_t);
	uint32_t	phoneColID = 0;

	// check the record Index validity.
	if (rowID == dbInvalidRowID)
		return kInvalidColumnID;

	// check if there is already a column value for the 'DisplayedPhone' column for the given record.
	if ((err = DbCopyColumnValue(dbRef,
								rowID,
								kAddrColumnIDDisplayedPhone,
								0,
								&phoneColID,
								&size)) >= errNone)
	{
		// the column exits, check if a value exist for the column referenced by its content.
		size = 0;
		if ((err = DbCopyColumnValue(dbRef, rowID, phoneColID, 0, NULL, &size)) >= errNone &&
			size > 0)
			goto exit; // a value already exist for this column
	}

	// the column doesn't exit or has an empty or invalid value we need to create/update it.
	// get the right field info list ID according to the sortID.

	// Load the u32l resource list that contain the ListPhone u32l resouce ID to use
	// The index in this u32l have to match the index of the sort index.
	listID = (uint16_t)U32ListGetItem(gApplicationDbP, OrderByListPhoneResIDUIn32List, NULL, gOrderByIndex);
	if (!listID)
		goto exit;

	// Load the resource list.
	if ((listH = U32ListGetList(gApplicationDbP, listID)) == NULL)
		goto exit;

	// Get the list items number.
	listCount = (uint16_t)U32ListGetItemNum(NULL, 0, listH);
	listP = (uint32_t*)DmHandleLock(listH);

	// Search for the coldID to set according to the field info list order.

	// For each column properties of each tab, Check if the 'kAddrColumnPropertyFieldInfo' matches
	// with the element of the fieldinfo list resource.
	for (listIdx = 0; listIdx < listCount; listIdx++)
	{
		for (tabIdx=0; tabIdx < gBookInfo.numTabs; tabIdx++)
		{
			for (colPropIdx=0; colPropIdx < gBookInfo.tabs[tabIdx].numElement; colPropIdx++)
			{
				uint32_t fieldInfo = gBookInfo.tabs[tabIdx].columnsPropertiesP[colPropIdx].fieldInfo;

				if ((fieldInfo & listP[listIdx]) == listP[listIdx])
				{
					uint32_t columnID = gBookInfo.tabs[tabIdx].columnsPropertiesP[colPropIdx].columnId;

					// check if a value exist for this colID in the current record.
					size = 0;
					if ((err = DbCopyColumnValue(dbRef, rowID, columnID, 0, NULL, &size)) >= errNone &&
						size > 0)
					{
						// the column exists so a value is set.
						// record the column ID, and stop the search.
						size = sizeof(uint32_t);
						err = DbWriteColumnValue(dbRef,
													rowID,
													kAddrColumnIDDisplayedPhone,
													0,
													-1,
													&columnID,
													size);
						ErrNonFatalDisplayIf(err < errNone, "Can't write column");

						phoneColID = columnID;
						goto exit;
					}
				}
			} // for colPropIdx
		} // for tabIdx
	} // for listIdx

exit:
	if (listP)
		DmHandleUnlock(listH);

	// Release handles
	U32ListReleaseList(listH);

	return phoneColID;
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBColumnPropertyExists
 *
 * DESCRIPTION:	Check if a property exists for the given column
 *
 * PARAMETERS:	->	dbP - Address Book database reference
 *				->	tableNameP - The table name (yeah! true!)
 *				->	columnID
 *				->	propertyID
 *
 * RETURNED:	true if the property exists, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		07/31/03	Initial revision
 *
 ***********************************************************************/
Boolean AddressDBColumnPropertyExists(DmOpenRef dbP, char *tableNameP, uint32_t columnID, DbSchemaColumnProperty propertyID)
{
	void *		dataP = NULL;
	uint32_t	size = 0;
	status_t	err;

	err = DbGetColumnPropertyValue(dbP, tableNameP, columnID, propertyID, &size, &dataP);

	if (err < errNone)
		return false;

	if (dataP)
		DbReleaseStorage(dbP, dataP);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:     AddressDBSetDatabaseAttributes
 *
 * DESCRIPTION:  This routine sets the backup bit on the given database.
 *					  This is to aid syncs with non Palm software.
 *					  If no DB is given, open the app's default database
 *					  and set the backup bit on it.
 *
 * PARAMETERS:   dbP -	the database to set backup bit,
 *						can be NULL to indicate app's default database
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * grant	04/01/99	Initial Revision
 * bhall	07/09/99	made non-static for access in AddressAutoFill.c
 * LFe		10/21/02	Use new Database Manager.
 *
 ***********************************************************************/
void AddressDBSetDatabaseAttributes(DatabaseID dbID, uint16_t attrBits)
{
	DmDatabaseInfoType	databaseInfo;
	uint16_t			attributes;

	memset(&databaseInfo, 0, sizeof(DmDatabaseInfoType));
	databaseInfo.size = sizeof(DmDatabaseInfoType);
	databaseInfo.pAttributes = &attributes;

	// now set the backup bit on localDBP
	if (DmDatabaseInfo(dbID, &databaseInfo) == errNone)
	{
		attributes |= attrBits;
		DmSetDatabaseInfo(dbID, &databaseInfo);
	}
}

/***********************************************************************
 *
 * FUNCTION:    AddressDBDeleteRecord
 *
 * DESCRIPTION: Deletes an address record.
 *
 * PARAMETERS:  archive: if true, mark the record as to be archived
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	06/13/95	Initial Revision
 * grant	06/21/99	Unlock the record if it is being archived.
 * jwm		10/08/99	Swap forward/backward so users can work
 *						through deleting records from top to bottom.
 * LFe		10/22/02	Use new Database Manager. Remove the move of
 *						the deleted record to the end of the database.
 *						It's now automatic.
 *
 ***********************************************************************/
void AddressDBDeleteRecord(Boolean archive)
{
	uint32_t	newRowID = dbInvalidRowID;
	uint32_t	currentRowIndex;
	status_t	err;
	uint32_t	rowCount;
	uint32_t	uniqueID;

	// Show the following record. Since the database is automatically
	// sorted after a delete or archive operation, keep the index
	// value unmodified, unless the record selected was the last one
	// or there's no more records in database.

	// Reset businessCardRecID
	if (gCurrentRowID == gBusinessCardRowID)
		gBusinessCardRowID = dbInvalidRowID;

	// First check for a valid cursor
	if (gCursorID != dbInvalidCursorID)
	{
		err = DbCursorGetCurrentPosition(gCursorID, &currentRowIndex);
		ErrNonFatalDisplayIf(err < errNone, "Can't get current row index");
		rowCount = DbCursorGetRowCount(gCursorID);

		// Get position of the futur current record
		if (currentRowIndex < rowCount)
			DbCursorGetRowIDForPosition(gCursorID, currentRowIndex + 1, &newRowID);
		else if (currentRowIndex > kFirstRowIndex)
			DbCursorGetRowIDForPosition(gCursorID, currentRowIndex - 1, &newRowID);

		uniqueID = gCursorID;
	}
	// No valid cursor. Check for the current record ID
	else if (gCurrentRowID != dbInvalidRowID)
		uniqueID = gCurrentRowID;
	else
		goto Exit;

	// Delete or archive the record.
	if (archive)
		err = DbArchiveRow(gAddrDB, uniqueID);
	else
		err = DbDeleteRow(gAddrDB, uniqueID);

	ErrNonFatalDisplayIf(err < errNone, "Can't delete row");

Exit:
	// Assign to gListViewSelectThisRowID
	gListViewSelectThisRowID = gCurrentRowID = newRowID;

	// Reopen cursor
	AddressDBOpenCursor();
}

/***********************************************************************
 *
 * FUNCTION:    AddressDBDuplicateRecord
 *
 * DESCRIPTION: Duplicates a new record from the current record.
 *
 * PARAMETERS:  numCharsToHilite (Output):  The number of characters added to the
 *				first name field to indicate that it was a duplicated record.
 *
 * RETURNED:    The ID of the new duplicated record.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         css	  6/13/99   Initial Revision
 *
 ***********************************************************************/
void AddressDBDuplicateRecord(uint32_t rowID, uint32_t *newRowIDP, uint16_t *numCharsToHiliteP, uint32_t *columnIDToHiliteP)
{
	uint32_t					numColumns = 0;
	MemHandle					columnH = NULL;
	uint32_t*			 		columnIDP;
	uint32_t					colIndex;
	uint32_t					size;
	status_t					err = errNone;
	DbSchemaColumnValueType		*columnValuesP = NULL;
	char*						duplicatedRecordIndicatorP = NULL;
	char*						newColDataP = NULL;
	Boolean						allocated = false;
	DbSchemaColumnDefnType *	columnDefnP = NULL;
	uint32_t					maxSize = 0;
	CategoryID *				categoriesP = NULL;
	uint32_t					numCategories = 0;
	uint16_t					recordAttr = 0;

	*newRowIDP = dbInvalidRowID;
	*numCharsToHiliteP = 0;

	// Get all the columns values for the record
	err = DbGetAllColumnValues(gAddrDB, rowID, &numColumns, &columnValuesP);
	if (err < errNone)
	{
		err = errNone;
		goto Exit;
	}

	// Duplicate it.
	if ((err = DbInsertRow(gAddrDB, kAddressDefaultTableName, numColumns, columnValuesP, newRowIDP)) < errNone)
		goto Exit;

	// Release orig record data
	DbReleaseStorage(gAddrDB, columnValuesP);
	columnValuesP = NULL;

	// Get orig record categories
	if ((err = DbGetCategory(gAddrDB, rowID, &numCategories, &categoriesP)) >= errNone)
	{
		// Set new record categories
		err = DbSetCategory(gAddrDB, *newRowIDP, numCategories, categoriesP);

		// Release categories list
		if (categoriesP)
			DbReleaseStorage(gAddrDB, categoriesP);
	}

	ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");

	// Restore private flag.
	DbGetRowAttr(gAddrDB, rowID, &recordAttr);
	if ((recordAttr & dbRecAttrSecret) == dbRecAttrSecret)
	{
		DbGetRowAttr(gAddrDB, *newRowIDP, &recordAttr);
		recordAttr |= dbRecAttrSecret;
		DbSetRowAttr(gAddrDB, *newRowIDP, &recordAttr);
	}

	// Now we must add the "duplicated indicator" to the end of the First Name so that people
	// know that this was the duplicated record.
	duplicatedRecordIndicatorP = ToolsCopyStringResource (DuplicatedRecordIndicatorStr);

	// Get the list of column ID that could bu used to concatenate the duplicate record indicator string.
	columnH = U32ListGetList(gApplicationDbP, DuplicateNameColumnIDU32LList);
	if (!columnH)
		goto Exit;

	numColumns = U32ListGetItemNum(NULL, 0, columnH);
	columnIDP = DmHandleLock(columnH);

	columnValuesP = NULL;
	err = DbGetColumnValues(gAddrDB, *newRowIDP, numColumns, columnIDP, &columnValuesP);

	if (err < errNone && err != dmErrOneOrMoreFailed && err != dmErrNoColumnData)
		goto Exit;


	// Find the first non empty field
	colIndex = 0;

	if (err != dmErrNoColumnData && columnValuesP)
	{
		while (colIndex < numColumns && columnValuesP[colIndex].errCode < errNone)
			colIndex++;
	}

	if (err == dmErrNoColumnData || colIndex > numColumns - 1)
	{
		// All column are empty.
		newColDataP = duplicatedRecordIndicatorP;
		colIndex = 0;
		*numCharsToHiliteP = (uint16_t)strlen (duplicatedRecordIndicatorP);
	}
	else
	{
		// First get the max size of the column. If for any reason we could access this info, set it to the current column data size.
		if (DbGetColumnDefinitions(gAddrDB, kAddressDefaultTableName, 1, &columnIDP[colIndex], &columnDefnP) >= errNone && columnDefnP)
			maxSize = columnDefnP->maxSize;
		else maxSize = columnValuesP[colIndex].dataSize;

		// Get enough space for current string, one blank and duplicated record indicator string.
		// No need to add a null chr, already computed in columnValuesP.dataSize
		size = columnValuesP[colIndex].dataSize + TxtCharSize(spaceChr) + strlen (duplicatedRecordIndicatorP);
		// Take the smaller size between the max column size and the real size needed.
		size = min(size, maxSize);

		// Number of char to highlight, depending of the number of char concatenated
		(*numCharsToHiliteP) = (uint16_t) (size - columnValuesP[colIndex].dataSize);

		// Allocate the buffer.
		newColDataP = MemPtrNew(size);
		ErrFatalDisplayIf(!newColDataP, "Out of memory");
		allocated = true;

		// make the new first name string with what was already there followed by
		// a space and the duplicate record indicator string.
		if (StrLCopy(newColDataP, (char*)columnValuesP[colIndex].data, size) < size)
			if (StrLCat(newColDataP, " ", size) < size)
				StrLCat(newColDataP, duplicatedRecordIndicatorP, size);

		if (columnDefnP)
			DbReleaseStorage(gAddrDB, columnDefnP);
	}

	// Release storage at that point to unlock the row
	if (columnValuesP != NULL)
	{
		DbReleaseStorage(gAddrDB, columnValuesP);
		columnValuesP = NULL;
	}

	if ((err = DbWriteColumnValue(gAddrDB, *newRowIDP, columnIDP[colIndex], 0, -1, newColDataP, (uint32_t)(strlen(newColDataP) + 1))) < errNone)
	{
		ErrNonFatalDisplay("Can't write column");
		goto Exit;
	}

	*columnIDToHiliteP = columnIDP[colIndex];

Exit:
	if (columnH)
	{
		DmHandleUnlock(columnH);
		U32ListReleaseList(columnH);
	}

	if (newColDataP && allocated)
		MemPtrFree (newColDataP);

	if (columnValuesP != NULL)
		DbReleaseStorage(gAddrDB, columnValuesP);

	if (duplicatedRecordIndicatorP)
		MemPtrFree(duplicatedRecordIndicatorP);

	if (err < errNone)
	{
		if (*newRowIDP != dbInvalidRowID)
		{
			DbDeleteRow(gAddrDB, *newRowIDP);
			*newRowIDP = dbInvalidRowID;
		}

		FrmUIAlert(DeviceFullAlert);
	}
}

/***********************************************************************
 *
 * FUNCTION:    AddressDBVerifyPassword
 *
 * DESCRIPTION: Verify a password for masked record.
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    true if the password is correct.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		10/01/03	Initial revision
 *
 ***********************************************************************/
Boolean AddressDBVerifyPassword(void)
{
	Boolean		verifyPW;
	uint16_t	mode;

	// Because Address is the running applications, we must close the DB
	// in case the user taps on 'Lost password'. That way, the system will
	// be able to open the AddressDB for write operation and delete the
	// records marked as private.
	AddressTabReleaseSchemaInfoFromDB(&gBookInfo, gAddrDB);
	AddressDBCloseDatabase();

	verifyPW = SecVerifyPW(showPrivateRecords);

	// Re-open DB and cursor
	mode = (gPrivateRecordVisualStatus == hidePrivateRecords) ? dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
	AddressDBOpenDatabase(gApplicationDbP, mode, &gAddrDB);
	ErrFatalDisplayIf(!gAddrDB, "Can't reopen DB");
	AddressDBOpenCursor();
	AddressTabInitSchemaInfoFromDB(&gBookInfo, gAddrDB);

	// We only want to unmask this one record, so restore the preference only if it has changed.
	if (verifyPW)
		PrefSetPreference(prefShowPrivateRecords, gPrivateRecordVisualStatus);

	return verifyPW;
}

/***********************************************************************
 *
 * FUNCTION:    AddressDBCloseDatabase
 *
 * DESCRIPTION: Closes cursor and database
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nohting
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * TEs		12/08/2003	Initial revision
 *
 ***********************************************************************/
void AddressDBCloseDatabase(void)
{
	if (gAddrDB == NULL)
		return;

	if (gCursorID != dbInvalidCursorID)
	{
		DbCursorClose(gCursorID);
		gCursorID = dbInvalidCursorID;
	}

	DbCloseDatabase(gAddrDB);
	gAddrDB = NULL;
}

/***********************************************************************
 *
 * FUNCTION:    AddressDBSaveOverlayLocale
 *
 * DESCRIPTION: Saved the passed locale
 *
 * PARAMETERS:  -> localeTypeP: the locale struct to be saved
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		02/19/2003	Initial revision
 *
 ***********************************************************************/
Boolean AddressDBSaveOverlayLocale(LmLocaleType *ovlyLocaleP)
{
	char				ovlyDbName[dmDBNameLength] = { 0 };
	DmDatabaseInfoType	dbInfo;
	Boolean				result = true;

	TraceOutput(TL(appErrorClass, "AddressDBSaveOverlayLocale"));

	memset(&dbInfo, 0, sizeof(dbInfo));
	dbInfo.size = sizeof(dbInfo);
	dbInfo.pName = ovlyDbName;
	DmDatabaseInfo(gAddrDatabaseID, &dbInfo);

	DmGetOverlayDatabaseName(ovlyDbName, ovlyLocaleP, ovlyDbName);
	if (DmFindDatabase(ovlyDbName, sysFileCAddress, dmFindExtendedDB, NULL) == 0)
	{
		// We can't find the overlay database for the overlay locale. Store the default one
		DmGetFallbackOverlayLocale(ovlyLocaleP);
		result = false;
	}

	PrefSetAppPreferences(sysFileCAddress, kAddrSystemLocalePrefID, kAddrPrefVersionNum, ovlyLocaleP, sizeof(LmLocaleType), false);

	return result;
}

/***********************************************************************
 *
 * FUNCTION:    AddressDBGetSavedOverlayLocale
 *
 * DESCRIPTION: Retreives the saved locale
 *
 * PARAMETERS:  <- localeTypeP: the current system locale
 *
 * RETURNED:    true if a saved locale has been found, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * TEs		02/19/2003	Initial revision
 *
 ***********************************************************************/
Boolean AddressDBGetSavedOverlayLocale(LmLocaleType *localeTypeP)
{
	int16_t		prefsVersion;
	uint32_t	prefsSize;

	TraceOutput(TL(appErrorClass, "AddressDBGetSavedOverlayLocale"));

	prefsSize = (uint32_t)sizeof(LmLocaleType);
	prefsVersion = PrefGetAppPreferences(sysFileCAddress, kAddrSystemLocalePrefID, localeTypeP, &prefsSize, false);
	return (Boolean)(prefsVersion != noPreferenceFound);
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBOpenPreviewDatabase
 *
 * DESCRIPTION:	Open the Address Book Preview database and create a new
 *				one if necessary. This database is only used to save
 *				vCard and to apply preview function on the saved record.
 *
 * PARAMETERS:	dbRef		- Application data database reference
 *
 * RETURNED:	status_t	- >=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	06/14/04	Initial revision
 *
 ***********************************************************************/
status_t AddressDBOpenPreviewDatabase (DmOpenRef adbDbRef, DmOpenRef *previewDbRefP)
{
	status_t	error = errNone;
	DmOpenRef	dbP;
	DatabaseID	dbID;
	DbTableDefinitionType*	schemaP = NULL;

	TraceOutput(TL(appErrorClass, "AddressDBOpenPreviewDatabase"));

	*previewDbRefP = NULL;

	// Find the Preview database. If it doesn't exist create it.
	dbP = DbOpenDatabaseByName(sysFileCAddress, kAddrPreviewDBName, dmModeReadWrite, dbShareRead);

	if (!dbP)
	{
		// Get the same schema as the 'standard' address database
		if ((error = DbGetTableSchema(adbDbRef, kAddressDefaultTableName, &schemaP)) < errNone)
			goto Exit;

		// Create the preview database with this schema
		if ((error = DbCreateDatabase(kAddrPreviewDBName, sysFileCAddress, kAddrDBType, 1, schemaP, &dbID)) < errNone)
			goto Exit;

		if (error >= errNone)
		{
			dbP = DbOpenDatabaseByName(sysFileCAddress, kAddrPreviewDBName, dmModeReadWrite, dbShareRead);

			if (!dbP)
				error = DmGetLastErr();
		}
	}

Exit:

	if (schemaP)
		DbReleaseStorage(adbDbRef, schemaP);

	*previewDbRefP = dbP;

	return error;
}

/***********************************************************************
 *
 * FUNCTION:	AddressDBDeletePreviewDatabase
 *
 * DESCRIPTION:	Delete the preview database.
 *
 * PARAMETERS:	None
 *
 * RETURNED:	status_t	- >=0 if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	06/15/04	Initial revision
 *
 ***********************************************************************/
status_t AddressDBDeletePreviewDatabase(void)
{
	DatabaseID	previewDBID;

	if ((previewDBID = DmFindDatabase(kAddrPreviewDBName, sysFileCAddress, dmFindSchemaDB, NULL)) != 0)
		return DmDeleteDatabase(previewDBID);

	return DmGetLastErr();
}
