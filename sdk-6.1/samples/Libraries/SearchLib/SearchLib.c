/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SearchLib.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the generic search engine for the Address Book Schema
 *
 *****************************************************************************/

#include <StringMgr.h>
#include <TextMgr.h>
#include <SysEventCompatibility.h>
#include <SystemMgr.h>
#include <TimeMgr.h>
#include <ErrorMgr.h>
#include <PrivateRecords.h>
#include <string.h>

#include "SearchLib.h"

#define kSqlRequestStr	"SELECT * FROM"


/******************************************************************************
 *
 * FUNCTION: SearchLibMain
 *
 * DESCRIPTION:
 *	Main entry point of the Books library shared library.
 *
 * CALLED BY:
 *	Low-level process startup code.
 *
 * PARAMETERS:
 *	cmd			 -> launch command, can be one of the following:
 *					sysAppLaunchCmdNormalLaunch - during process startup.
 *					sysLaunchCmdInitialize - right after this module is loaded
 *					sysLaunchCmdFinalize - right after this module is unloaded
 *	cmdPBP		 -> when cmd is sysAppLaunchCmdNormalLaunch, this
 *					parameter is the start code specified by ApplicationMgr
 *					when it started this thread.
 * launchFlags	 ->
 *
 * RETURNS:		errNone, or system error code.
 *
 *****************************************************************************/
uint32_t SearchLibMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:    PrvSearchLibResetPosition
 *
 * DESCRIPTION: Reset the view to its original state.
 *
 * PARAMETERS:  search	- Search structure. Contains all parameters
 *						to perform the search.
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		12/16/02	Initial revision
 *
 ***********************************************************************/
static status_t PrvSearchLibResetPosition(SearchLibType* search)
{
	status_t	err;

	// Set the DbView to it's starting point.
	switch(search->startRowIndex)
	{
		case 1:
			err = DbCursorMoveFirst(search->cursorID);
			break;

		case dbMaxRowIndex:
			err = DbCursorMoveLast(search->cursorID);
			break;

		default:
			// Check that the start index <= number of row in the cursor
			if (search->startRowIndex && (DbCursorGetRowCount(search->cursorID) >= search->startRowIndex))
				err = DbCursorSetAbsolutePosition(search->cursorID, search->startRowIndex);
			else
			{
				DbgOnlyFatalError("Invalid startRowIndex");
				err = dmErrInvalidParam;
			}
			break;
	}

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    SearchLibInit
 *
 * DESCRIPTION: Create all the depencies to perform the search but
 *				do not perform the first search.
 *
 * PARAMETERS:  search	- Search structure. Contains all parameters
 *						to perform the search.
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		12/16/02	Initial revision
 *
 ***********************************************************************/
status_t SearchLibInit(SearchLibType* searchP)
{
	status_t	err;
	char*		queryStr = NULL, *queryStrP;
	uint32_t	*colP = NULL, i, j;

	if (!searchP)
	{
		DbgOnlyFatalError("SearchLib: No SearchLib structure passed");
		return dmErrInvalidParam;
	}

	if (!searchP->schemaName)
	{
		DbgOnlyFatalError("SearchLib: Schema name not specified");
		return dmErrInvalidParam;
	}

	if (!searchP->orderBy && searchP->cursorID == dbInvalidCursorID)
	{
		DbgOnlyFatalError("SearchLib: No OrderBy specified and invalid cursorID passed");
		return dmErrInvalidParam;
	}

	// DOLATER - struct is supposed to be zeroed before.
	// Check if created is not yet set to true.
	DbgOnlyFatalErrorIf(searchP->cursorCreated, "already created");

	// If the number of columns is not defined, build an array of column to search in.
	// This array will contains only filtered column ID to speed up the search.
	if (!searchP->numCols)
	{
		// Get the columns definition for the whole database
		if ((err = DbGetAllColumnDefinitions(searchP->searchDB, searchP->schemaName, &searchP->numCols, &searchP->columnDefs)) < errNone)
			goto Error;

		// Allocate an array to store only the filtered column ID: max is the full table.
		if ((colP = (uint32_t*) MemPtrNew(sizeof(uint32_t) * searchP->numCols)) == NULL)
		{
			err = dmErrMemError;
			goto Error;
		}

		// Filter the column ID.
		// Keep the column if it's a text column or if we do an indirect search or if we check only if column are not empty
		for (i = 0, j = 0; i < searchP->numCols; i++)
		{
			if (searchP->indirect ||
				!searchP->textToSearch ||
				(searchP->columnDefs[i].type == dbVarChar) ||
				(searchP->columnDefs[i].type == dbChar))
			{
				colP[j++] = searchP->columnDefs[i].id;
			}
		}

		if (j)
		{
			// We found matching column to search in
			// First, resize the pointer to it's real size
			err = MemPtrResize(colP, sizeof(uint32_t) * j);
			ErrFatalDisplayIf(err < errNone, "Resize error");
			searchP->numCols = j;
			searchP->colIDsP = colP;
			searchP->freeColIDs = true;
		}
		else
		{
			// The database doesn't have column to search in. Why not?
			searchP->numCols = 0;
			searchP->colIDsP = NULL;
			searchP->freeColIDs = false;
			MemPtrFree(colP);
			colP = NULL;
		}

		DbReleaseStorage(searchP->searchDB, searchP->columnDefs);
		searchP->columnDefs = NULL;
	}

	// Only load the column definition if we have an array of column ID
	if (searchP->numCols)
		if ((err = DbGetColumnDefinitions(searchP->searchDB, searchP->schemaName, searchP->numCols, searchP->colIDsP, &searchP->columnDefs)) < errNone)
			goto Error;

	// If a cursor has been specified, use it, otherwise create a new one.
	if (searchP->cursorID == dbInvalidCursorID)
	{
		searchP->cursorCreated = false;

		// DON'T BUILD the WHERE clause if
		// - The search Lib have to do TxtCompare instead od TxtFind
		// - The search Lib have to filter the data
		// - The search Lib have to reverse the data
		// - The search Lib have to perform indirect search

		// BUILD the WHERE clause if
		// - Column ID are defined
		// - There is text to search

		if (!searchP->useCompare && !searchP->reverseString && !searchP->charFilter && !searchP->indirect &&
			searchP->numCols && searchP->textToSearch && searchP->origTextToSearch)
		{
			char	*searchStr, *searchStrP, *p, *oldP;
			Boolean first = true;

			// We can use an SQL where clause

			// Escape the search string properly (double any single quotes)
			oldP = searchP->origTextToSearch;

			if ((searchStrP = searchStr = (char*)MemPtrNew(strlen(searchP->origTextToSearch) * 2 + 1)) == NULL)
			{
				err = dmErrMemError;
				goto Error;
			}

			while (p = StrChr(oldP, '\''))
			{
				strncpy(searchStrP, oldP, p-oldP);
				searchStrP += p-oldP;
				*searchStrP++ = '\'';
				*searchStrP++ = '\'';
				oldP = p + 1;
			}

			strcpy(searchStrP, oldP);

			queryStr = (char*)MemPtrNew(strlen(kSqlRequestStr) + strlen(searchP->schemaName) + strlen(searchP->orderBy) + (searchP->numCols * (30 + strlen(searchStr))));

			if (!queryStr)
			{
				MemPtrFree(searchStr);
				err = dmErrMemError;
				goto Error;
			}

			sprintf(queryStr, "%s %s WHERE (", kSqlRequestStr, searchP->schemaName);
			queryStrP = queryStr + strlen(queryStr);

			for (i = 0; i < searchP->numCols; i++)
			{
				if (!first)
				{
					strcpy(queryStrP, " OR ");
					queryStrP += 4;
				}

				sprintf(queryStrP, "(%s PS_LIKE '%s')", searchP->columnDefs[i].name, searchStr);
				queryStrP += strlen(queryStrP);
				first = false;
			}

			*queryStrP++ = ')';
			*queryStrP++ = ' ';
			strcpy(queryStrP,searchP->orderBy);
			MemPtrFree(searchStr);
		}
		else
		{
			// We can't use a where clause, so just open all the rows

			// 3 = 2 spaces between kSqlRequestStr & schemaName & orderBy + \0
			if ((queryStr = MemPtrNew((uint32_t)(strlen(kSqlRequestStr) + strlen(searchP->schemaName) + strlen(searchP->orderBy) + 3))) == NULL)
			{
				err = dmErrMemError;
				goto Error;
			}

			sprintf(queryStr, "%s %s %s", kSqlRequestStr, searchP->schemaName, searchP->orderBy);

			// Create the Db Cursor. It will be used to navigate in the DB based on the search struct criteria.
			if ((err = DbCursorOpenWithCategory(searchP->searchDB, queryStr, searchP->cursorFlag, searchP->numCategories, searchP->catIDsP, searchP->matchMode, &searchP->cursorID)) < errNone)
				goto Error;
		}

		if ((err = DbCursorOpenWithCategory(searchP->searchDB, queryStr, searchP->cursorFlag, searchP->numCategories, searchP->catIDsP, searchP->matchMode, &searchP->cursorID)) < errNone)
			goto Error;

		searchP->cursorCreated = true;
	}

	if ((err = PrvSearchLibResetPosition(searchP)) < errNone)
		goto Error;

	searchP->firstCall = true;

Error:

	if (queryStr)
		MemPtrFree(queryStr);

	if (err < errNone)
	{
		if (searchP->freeColIDs && colP)
		{
			MemPtrFree(colP);
			searchP->colIDsP = NULL;
			searchP->numCols = 0;
		}
	}

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    SearchLibRelease
 *
 * DESCRIPTION: Release all the depencies needed for the search.
 *
 * PARAMETERS:  search	- Search structure. Contains all parameters
 *						to perform the search.
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		12/16/02	Initial revision
 *
 ***********************************************************************/
status_t SearchLibRelease(SearchLibType* search)
{
	if (!search)
		return dmErrInvalidParam;

	if (search->columnData)
	{
		DbReleaseStorage(search->searchDB, search->columnData);
		search->columnData = NULL;
	}

	if (search->columnDefs)
	{
		DbReleaseStorage(search->searchDB, search->columnDefs);
		search->columnDefs = NULL;
	}

	if (search->cursorCreated)
	{
		DbCursorClose(search->cursorID);
		search->cursorCreated = false;
	}

	if (search->freeColIDs)
	{
		MemPtrFree(search->colIDsP);
		search->colIDsP = NULL;
		search->freeColIDs = false;
		search->numCols = 0;
	}

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:    SearchLibSearch
 *
 * DESCRIPTION: Perform the search and update the struct to keep the
 *				stage of the search.
 *
 * PARAMETERS:  search	- Search structure. Contains all parameters
 *						to perform the search.
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		12/16/02	Initial revision
 *
 ***********************************************************************/
Boolean SearchLibSearch(SearchLibType* search)
{
	status_t	err = errNone;
	int32_t		i;
	uint32_t	currentColumnIndex = 0;
	uint32_t	startOffset = 0;
	uint32_t	columnID = 0;
	uint32_t	valueSize;
	uint32_t	newTime;
	int32_t		columnIncrement;
	size_t		matchLen = 0;
	int16_t		compareOrder;
	Boolean		found = false;
	char*		startStr = NULL;
	char*		toBeDeletedStr = NULL;
	char*		toBeReleasedStr = NULL;
	Boolean		initColumnIndex = false;
	Boolean		loadRecord = false;

	/* NOTE
		search->foundColID, search->foundColIndex and search->foundRecordID
		will not be initialized to invalid values.
		This will allow to have the last founded values if the next call
		to this function failed.
	*/

	if (!search)
		return false;

	// Numcols should have been initialized by the Init function or by the parameters
	// If null, no column to search in, return Not Found directly.
	if (!search->numCols)
		return false;

	// set the increment, depending of the search direction.
	columnIncrement = (search->columnDirection != kSearchMoveBackward) ? 1 : -1;

	// Reset some flags.
	search->err = errNone;
	search->interrupted = false;
	search->strDestLength = 0;

	// Init timer interrupt
	search->timer = (uint32_t) SysTimeInMilliSecs(TimGetTicks());

	// If not the first call to the search, go to the next record
	// otherwise we will get the record set during the init phase.
	if (!search->firstCall)
	{
		switch(search->resumePolicy)
		{
			case kSearchResumeChangeRecord:
				// Perform the search on a new record
				// Release the column data if we are going to change record.
				if (search->columnData)
				{
					DbReleaseStorage(search->searchDB, search->columnData);
					search->columnData = NULL;
				}

				// Go forward or backward, depending of the settings.
				switch (search->recordDirection)
				{
					case kSearchMoveForward:	err = DbCursorMoveNext(search->cursorID);	break;
					case kSearchMoveBackward:	err = DbCursorMovePrev(search->cursorID);	break;
					default:					err = dmErrInvalidParam;
												DbgOnlyFatalError("Invalid record move policy");
												break;
				}

				// If an error occurs (BOF or EOF), return.
				if (err < errNone)
					goto Error;

				initColumnIndex = true;
				loadRecord = true;
				break;

			case kSearchResumeChangeColumn:
				// perform the search on the same record, but in the next/previous column ID
				currentColumnIndex = search->foundColIndex + columnIncrement;
				break;

			case kSearchResumeContinue:
				// continue the search in the same record
				// Continue after the previous matching pattern.
				startOffset = search->matchPos + search->matchLength;
				break;

			default:
				DbgOnlyFatalError("Invalid resume policy");
				return false;
		}
	}
	else
	{
		// If this is the first call to search, the startColumnIndex should have been set
		// and we need to take it into account
		// If the search->startColumnIndex = 0, currentColumnIndex will be initialized
		// properly according to the search direction later, once the record loaded.
		currentColumnIndex = (search->startColumnIndex != dbMaxRowIndex) ? search->startColumnIndex : 0;
		initColumnIndex = (search->startColumnIndex == dbMaxRowIndex);
		loadRecord = true;
	}

	// Reset the flag firstCall.
	search->firstCall = false;

	while (!found)
	{
		uint16_t	attr = 0;

		// If the search could be interrupted, check every specified number of record
		// if there is an event in the queue and then stop the search.
		if (search->interruptible)
		{
			newTime = (uint32_t) SysTimeInMilliSecs(TimGetTicks());

			if (newTime - search->timer > search->interruptCheck)
			{
				search->timer = newTime;

				if (EvtSysEventAvail(true))
				{
					search->interrupted = true;
					break;
				}
			}
		}
		else
		{
			// Search is not interruptible, so if interruptCheck field as a value,
			// it must be used as a search timeout value (in milliseconds).
			// if the search is not terminated before that this time out is ellapsed,
			// The search is stopped, and interrupted field is set to true to inform
			// the caller that the search failed due to the time out.
			if (search->interruptCheck)
			{
				newTime = (uint32_t) SysTimeInMilliSecs(TimGetTicks());

				if ((newTime - search->timer) > search->interruptCheck)
				{
					search->interrupted = true;
					break;
				}
			}
		}

        // Skip hidden record if needed
		if (search->hidePrivate && DbGetRowAttr(search->searchDB, search->cursorID, &attr) >= errNone && (attr & dbRecAttrSecret) == dbRecAttrSecret)
			goto Continue;

		// numCols is always specified. Get the data for the specified columns.
		if (loadRecord)
			err = DbGetColumnValues(search->searchDB, search->cursorID, search->numCols, search->colIDsP, &search->columnData);

		if ((err < errNone) && (err != dmErrOneOrMoreFailed) && (err != dmErrNoColumnData))
			goto Error;

		// Set the current depending of the search direction.
		// If currentColumnIndex = 0, that means we need to set it according to columnDirection field value.
		// If searching forward, set it to 0 -> start from the first column in the array.
		if (initColumnIndex)
		{
			switch(search->columnDirection)
			{
				case kSearchMoveForward:	currentColumnIndex = 0;	break;
				case kSearchMoveBackward:	currentColumnIndex = (search->numCols) ? search->numCols - 1 : 0;	break;
				case kSearchMoveLocked:		currentColumnIndex = (search->startColumnIndex != dbMaxRowIndex) ? 0 : ((search->numCols) ? search->numCols - 1 : 0);	break;
				default: DbgOnlyFatalError("invalid columnDirection");
			}
		}

		// if textToSearch parameter is NULL, we only want to  check if there is data in some columns
		if (!search->textToSearch)
		{
			// Skip to next record if err = dmErrNoColumnData
			if ((err >= errNone) || (err == dmErrOneOrMoreFailed))
			{
				valueSize = 0;

				// Search the first column ID that contains any data.
				// It SHOULD NOT failed as at least one column contains data
				// except if the flag "indirect" is set, because we are going to check
				// for another column data.

				// Start from a specific index. In case of a new record, index = 0, otherwise it's the last or next
				// index where we found the previous matching pattern, depending of the settings.
				for (i = currentColumnIndex; (i < (int32_t)search->numCols) && (i >= 0) && !found; i += columnIncrement)
				{
					if (search->columnData[i].errCode == errNone)
					{
						// If the flag indirect is set, this mean the column contains a column ID
						// We need to check the data for this column ID.
						if (search->indirect)
						{
							// Check first if the size of the data is the size of a column id.
							if (search->columnData[i].dataSize == sizeof(uint32_t))
							{
								columnID = *(uint32_t *)search->columnData[i].data;
								err = DbCopyColumnValue(search->searchDB, search->cursorID, columnID, 0, NULL, &valueSize);

								if ((err < errNone) && (err != dmErrNoColumnData))
									goto Error;

								// Check if the column has data. If true, we find a solution.
								found = (Boolean) valueSize > 0;
							}
						}
						else
						{
							// Direct access to the column data. Check if the column has data.
							// Should be true as errCode != dmErrOneOrMoreFailed but in case of
							// another error for this column ID
							columnID = search->columnData[i].columnID;
							valueSize = search->columnData[i].dataSize;
							found =  (Boolean) valueSize > 0;
						}
					}

					// If the search is locked on a column, go up one level and move to another record or leave.
					if (search->columnDirection == kSearchMoveLocked)
						break;
				} // for

				if (found)
				{
					// If found, save the information related to the record
					// #### Test error
					err = DbCursorGetCurrentPosition(search->cursorID, &search->foundRowIndex);
					err = DbCursorGetCurrentRowID(search->cursorID, &search->foundRowID);
					search->err = errNone;
					search->foundColIndex = i;
					search->foundColID = columnID;
					search->strDestLength = valueSize;
					return true;
				}
			}
		}
		else
		{
			// Skip to next record if err = dmErrNoColumnData
			if ((err >= errNone) || (err == dmErrOneOrMoreFailed))
			{
				// Start from a specific index. In case of a new record, index = 0, otherwise it's the last or next
				// index where we found the previous matching pattern, depending of the settings.
				for (i = currentColumnIndex; (i < (int32_t)search->numCols) && (i >= 0) && !found; i += columnIncrement)
				{
					// If the column data exist
					if ((search->columnData[i].errCode >= errNone) && ((search->columnDefs[i].type == dbVarChar) || (search->columnDefs[i].type == dbChar)))
					{
						// If the flag indirect is set, this mean the column contains a column ID
						// We need to check the data for this column ID.
						if (search->indirect)
						{
							startStr = NULL;
							valueSize = 0;

							// Check first if the size of the data is the size of a column id.
							if (search->columnData[i].dataSize == sizeof(uint32_t))
							{
								columnID = *(uint32_t *)search->columnData[i].data;
								err = DbGetColumnValue(search->searchDB, search->cursorID, columnID, 0, (void**)&toBeReleasedStr, &valueSize);

								if ((err < errNone) && (err != dmErrNoColumnData))
									goto Error;

								startStr = toBeReleasedStr;
							}
						}
						else
						{
							// Direct access to column data.
							startStr = (char*)search->columnData[i].data;
							valueSize = search->columnData[i].dataSize;
							columnID = search->columnData[i].columnID;
						}

						// If the string should be reversed or filtered, we create a new string based on the column data
						if (valueSize && (search->charFilter || search->reverseString))
						{
							toBeDeletedStr = SearchLibConvertString(startStr, search->charFilter, search->reverseString, &valueSize);
							startStr = toBeDeletedStr;
						}

						// if startOffset is greater that valueSize, it means we have reach the end of the column data
						// If smaller, we are going to start the search at this offset in the buffer.
						if (valueSize && (startOffset < valueSize))
						{
							startStr = startStr + startOffset;

							if (!search->useCompare)
							{
								size_t matchPos;

								found = TxtFindString(startStr, search->textToSearch, &matchPos, &matchLen);
								search->matchPos = matchPos;
							}
							else
							{
								// Optimization: Call the compare functions only if the database data (valueSize - startOffset) is greater than minCompareMatch
//								if (!search->minCompareMatch || (valueSize - startOffset >= search->minCompareMatch))
								{
									if (!search->caseCompare)
										compareOrder = TxtCaselessCompare(startStr, (uint16_t) (valueSize - startOffset), NULL,	search->textToSearch, strlen(search->textToSearch), &matchLen);
									else
										compareOrder = TxtCompare(startStr, (uint16_t)(valueSize - startOffset), NULL,	search->textToSearch, strlen(search->textToSearch), &matchLen);

									// Found a matching string if the TxtCompare say it (compareOrder == 0)
									// or if the number of matching chars are greater or equal to the minimum asked.
									found = (Boolean) ((!compareOrder) || (search->minCompareMatch && (matchLen >= search->minCompareMatch)));
								}
								//else found = false;
							}
						}

						// In case of indirect search, we load localy the new data. Now, release it
						if (toBeReleasedStr)
						{
							DbReleaseStorage(search->searchDB, toBeReleasedStr);
							toBeReleasedStr = NULL;
						}

						// If filters apply on the data, a new string has been allocated. Release it.
						if (toBeDeletedStr)
						{
							MemPtrFree(toBeDeletedStr);
							toBeReleasedStr = NULL;
						}

						// If the string has been found, save the information and return.
						if (found)
						{
							// #### Test error
							err = DbCursorGetCurrentPosition(search->cursorID, &search->foundRowIndex);
							err = DbCursorGetCurrentRowID(search->cursorID, &search->foundRowID);
							search->err = errNone;
							search->matchLength = matchLen;
							search->foundColID = columnID;
							search->foundColIndex = i;
							search->strDestLength = valueSize - startOffset;
							return true;
						}
					}
					// Change of column. Restart from the begining.
					startOffset = 0;

					// If the search is locked on a column, go up one level and move to another record or leave.
					if (search->columnDirection == kSearchMoveLocked)
						break;
				} // for
			} // if - err = dmErrNoColumnData
		}// else

Continue:
		// Perform the search on a new record
		// Release the column data if we are going to change record.
		if (search->columnData)
		{
			DbReleaseStorage(search->searchDB, search->columnData);
			search->columnData = NULL;
		}


		// Go forward or backward, depending of the settings.
		// If the search is locked on the current record, return the found state.
		switch (search->recordDirection)
		{
			case kSearchMoveForward:	err = DbCursorMoveNext(search->cursorID);	break;
			case kSearchMoveBackward:	err = DbCursorMovePrev(search->cursorID);	break;
			case kSearchMoveLocked:		return found;
			default:					err = dmErrInvalidParam;
										DbgOnlyFatalError("Invalid record move policy");
										break;
		}

		// If an error occurs (BOF or EOF), return.
		if (err < errNone)
			goto Error;

		// Force update of currentColumnIndex, otherwise it will not be set to the right
		// value after loading the new record.
		initColumnIndex = true;
		startOffset = 0;
		loadRecord = true;

	} // while (!found)

	return found;

Error:

	// Return also BOF or EOF errors. Don't reset it to noError
	// This is also information.
	search->err = err;

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    SearchLibReset
 *
 * DESCRIPTION: Reset the search to it's first stage.
 *
 * PARAMETERS:  search	- Search structure. Contains all parameters
 *						to perform the search.
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		12/16/02	Initial revision
 *
 ***********************************************************************/
status_t SearchLibReset(SearchLibType* search)
{
	status_t	err;

	if (search->columnData)
	{
		DbReleaseStorage(search->searchDB, search->columnData);
		search->columnData = NULL;
	}

	err = PrvSearchLibResetPosition(search);

	search->firstCall = true;
	search->err = err;

	return err;
}

/***********************************************************************
 *
 * FUNCTION:    SearchLibConvertString
 *
 * DESCRIPTION: This function will return a new allocated string .
 *
 * PARAMETERS:  origString:
 *				charFilter:
 *				reverse:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		12/16/02	Initial revision
 *
 ***********************************************************************/
char* SearchLibConvertString(char* origString, uint16_t charFilter, Boolean reverse, uint32_t* newSize)
{
	char*		newString;
	wchar32_t	outChar;
	uint32_t	offsetOrig = 0;
	uint32_t	offsetDest = 0;

	// If no input string or no filter defined and no reverse, return null.
	if (!origString || (!charFilter && !reverse))
		return NULL;

	newString = MemPtrNew((uint32_t)(strlen(origString) + 1));

	if (!newString)
		return NULL;

	if (reverse)
	{
		uint32_t	origStringLen;
		uint32_t	dstStringLen = 0;
		uint16_t	chrSize;

		offsetDest = origStringLen = strlen(origString);
		offsetOrig = 0;

		// Parse the orig string
		while (offsetOrig < origStringLen)
		{
			chrSize = TxtGetNextChar(origString, offsetOrig, &outChar);
			offsetOrig += chrSize;

			// Filter unwanted characters and copy to dest string
			if (!charFilter || (TxtCharAttr(outChar) & charFilter) != 0)
			{
				offsetDest -= chrSize;
				dstStringLen += TxtSetNextChar(newString, offsetDest, outChar);
			}
		}

		// Some characters have been filtered so the dest string doesn't start
		// at the begining of its buffer. Move it.
		if (offsetDest)
			memmove(newString, newString + offsetDest, dstStringLen);

		// To set the null terminator
		offsetDest = dstStringLen;
	}
	else
	{
		do
		{
			offsetOrig += TxtGetNextChar(origString, offsetOrig, &outChar);

			if (!charFilter || (TxtCharAttr(outChar) & charFilter) != 0)
				offsetDest += TxtSetNextChar(newString, offsetDest, outChar);

		} while (outChar != chrNull);
	}

	if (newSize)
		*newSize = offsetDest;

	outChar = chrNull;
	TxtSetNextChar(newString, offsetDest, outChar);

	return newString;
}

/***********************************************************************
 *
 * FUNCTION:    SearchLibSetStartRecordIndexById
 *
 * DESCRIPTION: This function will reset the start point to a specific
 *				record and column ID.
 *
 * PARAMETERS:  origString:
 *				charFilter:
 *				reverse:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		01/27/02	Initial revision
 *
 ***********************************************************************/
status_t SearchLibSetStartIndexesById(SearchLibType* search, uint32_t rowID, uint32_t columnID)
{
	uint32_t	i;
	Boolean		found = false;

	if (!search)
		return dmErrInvalidParam;

	if (rowID)
		search->err = DbCursorGetPositionForRowID(search->cursorID, rowID, &search->startRowIndex);

	if (columnID)
	{
		search->startColumnIndex = 0;

		if (!search->numCols)
			DbgOnlyFatalError("search->numCols is zero");

		// Array of column ID specified by the user.
		for (i = 0; i < search->numCols; i++)
		{
			if (search->colIDsP[i] == columnID)
			{
				search->startColumnIndex = i;
				found = true;
				break;
			}
		}

		search->err = (found) ? errNone : dmErrInvalidColumnID;
	}

	return search->err;
}
