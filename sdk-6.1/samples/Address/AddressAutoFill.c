/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressAutoFill.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This module contains routines that support the auto-fill feature
 *   of the address application. (Started from ExpLookup.c from Expense)
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <ErrorMgr.h>
#include <TimeMgr.h>
#include <UIResources.h>
#include <SystemResources.h>
#include <SysUtils.h>
#include <string.h>

#include "Address.h"
#include "AddressTools.h"
//#include "AddressDefines.h"
#include "AddressAutoFill.h"
#include "AddressDBSchema.h"

/************************************************************
 *
 *  FUNCTION: PrvAutoFillPartialCaselessCompare
 *
 *  DESCRIPTION: Compares two strings with case and accent insensitivity.
 *               If all of s1 matches all or the start of s2 then
 *               there is a match
 *
 *  PARAMETERS: 2 string pointers
 *
 *  RETURNS: 0 if they match, non-zero if not
 *
 *  REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/4/96	Initial Revision
 *			kwk	05/16/99	Use StrNCaselessCompare.
 *
 *************************************************************/
static int16_t PrvAutoFillPartialCaselessCompare (char * s1, char * s2)
{
	size_t	length;

	// Check for err
	ErrFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	length = strlen (s1);
	return TxtCaselessCompare(s1, length, NULL, s2, length, NULL);
}


/***********************************************************************
 *
 * FUNCTION:     PrvAutoFillLookupCompareRecords
 *
 * DESCRIPTION:  Compare two lookup records.  This is a callback
 *               called by DmGetRecordSortPosition.
 *
 * PARAMETERS:   database record 1
 *               database record 2
 *
 *  RETURNS:     -n if record one is less (n != 0)
 *			         n if record two is less
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/16/96	Initial Revision
 *
 ***********************************************************************/
static int16_t PrvAutoFillLookupCompareRecords (char * strP, LookupRecordPtr r2, int16_t dummy, DmSortRecordInfoPtr info1, DmSortRecordInfoPtr info2, MemHandle appInfoH)
{
	return TxtCaselessCompare(strP, SIZE_MAX, NULL, &r2->text, SIZE_MAX, NULL);
}

/***********************************************************************
 *
 * FUNCTION:     PrvAutoFillSortCompareRecords
 *
 * DESCRIPTION:  Compare two sort records.
 *               This is a callback called by DmQuickSort.
 *
 * PARAMETERS:   database record 1
 *               database record 2
 *
 *  RETURNS:     -n if record one is less (n != 0)
 *			      n if record two is less
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			bhall	11/8/99		Initial Revision
 *
 ***********************************************************************/
static int16_t PrvAutoFillSortCompareRecords (LookupRecordPtr r1, LookupRecordPtr r2, int16_t dummy, DmSortRecordInfoPtr info1, DmSortRecordInfoPtr info2, MemHandle appInfoH)
{
	return TxtCaselessCompare(&r1->text, SIZE_MAX, NULL, &r2->text, SIZE_MAX, NULL);
}

/***********************************************************************
 *
 * FUNCTION:     AutoFillInitDB
 *
 * DESCRIPTION:  This routine initializes a lookup database from a
 *               resource.  Each string in the resource becomes a record
 *               in the database.  The strings in the resouce are delimited
 *               by the '|' character.
 *
 * PARAMETERS:   dbType      - database type
 *               initRscID - reosource used to initialize the database
 *
 * RETURNED:     error code, 0 if successful
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		1/12/96		Initial Revision
 *			grant	4/7/99		Set backup bit on new databases.
 *			bhall	11/1/99		now handles case where init string not present
 * 								also sorts the db in case initial values out of order
 *			bhall	11/8/99		Fixed prob where records not properly sorted
 *								Fixed prob where last record of init string skipped
 *			TEs		01/24/03	Modified for schema driven AddressBook
 *
 ***********************************************************************/
status_t AutoFillInitDB(uint32_t dbType, uint16_t initRscID)
{
	status_t			error = errNone;
	uint16_t			index, i;
	uint32_t			entryLen;
	uint32_t			timeT;
	MemHandle			resH = NULL;
	DmOpenRef			dbP = NULL;
	DatabaseID			dbID = 0;
	MemHandle			recH;
	LookupRecordType *	recP;
	uint32_t			recordSize;
	char				dbName[kAddrAutoFillDBNamePrefixLen + sizeof(dbType) + 1];
	uint32_t			dbTypeBE;
	char*				strListP;
	uint32_t			strListCount;
	char				defaultStr[kEntryMaxSize];


	// Compute the DB name: "AddrAutoFillDB_" . (char*)&dbType
	strcpy(dbName, kAddrAutoFillDBNamePrefix);
	dbTypeBE = EndianSwap32(dbType);
	memmove(&dbName[kAddrAutoFillDBNamePrefixLen], &dbTypeBE, sizeof(dbTypeBE));
	dbName[kAddrAutoFillDBNamePrefixLen + sizeof(dbTypeBE)] = 0;

	// The DB already exists. exit
	if ((dbID = DmFindDatabase(dbName, sysFileCAddress, dmFindExtendedDB, NULL)) != 0)
		return errNone;

	// If the description database does not exist, create it.
	if ((error = DmCreateDatabase(dbName, sysFileCAddress, dbType, false)) < errNone)
		return error;

	// Find the new created database
	if ((dbID = DmFindDatabase(dbName, sysFileCAddress, dmFindExtendedDB, NULL)) == 0)
		return DmGetLastErr();

	// Open it. If failed, delete it.
	if ((dbP = DmOpenDatabase(dbID, dmModeReadWrite)) == NULL)
	{
		error = DmGetLastErr();
		goto exit;
	}

	// Set the backup bit.  This is to aid syncs with non-Palm software.
	// Also set hidden bit so launcher info doesn't give confusing record counts.
	AddressDBSetDatabaseAttributes(dbID, dmHdrAttrBackup | dmHdrAttrHidden);

	// Attempt to initialize the description datebase from a Sring List resource that contains
	// a list of default data to add to the DB.
	if (initRscID && (resH = DmGetResource(gApplicationDbP, strListRscType, initRscID)) != NULL)
	{
		// Get the number of items in the string list.
		strListP = (char*)DmHandleLock(resH);
		// Skip over prefix string, then get the entry count.
		strListP += strlen(strListP) + 1;
		strListCount = *strListP++;
		strListCount = (strListCount << 8);
		strListCount = strListCount +  *(uint8_t *)strListP++;

		// Done with the init string - release it
		DmHandleUnlock(resH);
		DmReleaseResource(resH);

		for (i = 0; (i < strListCount) && (error >= errNone); i++)
		{
			// We will need the time for default records we may create
			timeT = TimGetSeconds();

			SysStringByIndex(gApplicationDbP, initRscID, i, defaultStr, kEntryMaxSize);
			entryLen = strlen(defaultStr);
			recordSize = sizeof(LookupRecordType) + entryLen;

			// If we could not create a new record, exit and delete the database
			index = dmMaxRecordIndex;
			if ((recH = DmNewRecord(dbP, &index, recordSize)) == NULL)
			{
				error = DmGetLastErr();
				break;
			}

			recP = DmHandleLock(recH);

			// Write out the data
			if ((error = DmWrite (recP, offsetof(LookupRecordType, time), &timeT, sizeof (uint32_t))) >= errNone)
				error = DmStrCopy(recP, offsetof(LookupRecordType, text), defaultStr);

			DmHandleUnlock(recH);

			// Release it, dirty
			DmReleaseRecord(dbP, index, true);
		}

		// Now sort the database - required for proper operation
		if (error >= errNone)
			error = DmQuickSort(dbP, (DmCompareFunctionType *)PrvAutoFillSortCompareRecords, 0);
	}

exit:
	// If we had the database open, close it
	if (dbP)
		DmCloseDatabase(dbP);

	if (error < errNone && dbID)
		DmDeleteDatabase(dbID);

	return error;
}


/***********************************************************************
 *
 * FUNCTION:     AutoFillLookupStringInDatabase
 *
 * DESCRIPTION:  This routine seatches a database for a the string passed.
 *
 * PARAMETERS:   dpb       - description database
 *					  key       - string to lookup record with
 *					  indexP    - to contain the record found
 *
 * RETURNED:     true if a unique match was found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		1/4/96		Initial Revision
 *			TEs		01/24/03	Modified for schema driven AddressBook
 *
 ***********************************************************************/
Boolean AutoFillLookupStringInDatabase(DmOpenRef dbP, char *keyP, uint16_t *indexP)
{
	int16_t 			result;
	int16_t 			numOfRecords;
	uint16_t			kmin, probe, i;
	Boolean				match = false;
	MemHandle 			recH;
	LookupRecordType * 	recP;
	Boolean 			unique;

	unique = false;

	if (!keyP || !(*keyP))
		return false;

	numOfRecords = DmNumRecords(dbP);

	if (!numOfRecords)
		return false;

	result = 0;
	kmin = probe = 0;

	while (numOfRecords > 0)
	{
		i = numOfRecords / 2;
		probe = kmin + i;

		// Compare the two records.  Treat deleted records as greater.
		// If the records are equal look at the following position.
		recH = DmQueryRecord (dbP, probe);
		if (!recH)
		{
			result = -1;		// Delete record is greater
		}
		else
		{
			recP = DmHandleLock(recH);
			result = PrvAutoFillPartialCaselessCompare(keyP, &recP->text);
			DmHandleUnlock(recH);
		}


		// If the date passed is less than the probe's date , keep searching.
		if (result < 0)
			numOfRecords = i;

		// If the date passed is greater than the probe's date, keep searching.
		else if (result > 0)
		{
			kmin = probe + 1;
			numOfRecords = numOfRecords - i - 1;
		}

		// If equal stop here!  Make sure the match is unique by checking
		// the records before and after the probe,  if either matches then
		// we don't have a unique match.
		else
		{
			// Start by assuming we have a unique match.
			match = true;
			unique = true;
			*indexP = probe;

			// If we not have a unique match,  we want to return the
			// index one the first item that matches the key.
			while (probe)
			{
				recH = DmQueryRecord (dbP, probe-1);
				recP = DmHandleLock(recH);
				result = PrvAutoFillPartialCaselessCompare(keyP, &recP->text);
				DmHandleUnlock(recH);

				if (result != 0)
					break;

				unique = false;
				*indexP = probe-1;
				probe--;
			}

			if (! unique) break;


			if (probe + 1 < DmNumRecords(dbP))
			{
				recH = DmQueryRecord (dbP, probe+1);

				if (recH)
				{
					recP = DmHandleLock(recH);
					result = PrvAutoFillPartialCaselessCompare(keyP, &recP->text);
					DmHandleUnlock(recH);

					if (result == 0)
						unique = false;
				}
			}
			break;
		}
	}
	return (match);
}


/***********************************************************************
 *
 * FUNCTION:     AutoFillLookupSave
 *
 * DESCRIPTION:  This routine saves the string passed to the specified
 *               lookup database.
 *
 * PARAMETERS:   dbType      - database type
 *               strP       - string to save to lookup record
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		1/16/96		Initial Revision
 *			TEs		01/24/03	Modified for schema driven AddressBook
 *
 ***********************************************************************/
void AutoFillLookupSave(uint32_t dbType, char * strP)
{
	uint16_t			i;
	uint16_t			index = 0;
	uint16_t			numRecords;
	uint32_t			entryLen;
	uint32_t			time;
	Boolean				insert = true;
	MemHandle			recH;
	DmOpenRef			dbP;
	LookupRecordType *	recP;
	uint32_t			recordSize;

	if ((dbP = DmOpenDatabaseByTypeCreator (dbType, sysFileCAddress, dmModeReadWrite)) == NULL)
		return;

	entryLen = strlen(strP);

	// There is a limit on the number of entries in a lookup database,
	// if we've reached that limit find the oldest entry and delete it.
	numRecords = DmNumRecords (dbP);

	if (numRecords >= kMaxLookupEntries)
	{
		time = 0xFFFFFFFF;

		for (i = 0; i < numRecords; i++)
		{
			recH = DmQueryRecord(dbP, i);
			recP = DmHandleLock(recH);

			if (recP->time < time)
			{
				index = i;
				time = recP->time;
			}

			DmHandleUnlock(recH);
		}

		DmRemoveRecord(dbP, index);
	}

	// Check if the string passed already exist in the database,  if it
	// doesn't insert it.  If it does,  write the passed string to
	// the record, the case of one or more of the character may
	// changed.
	index = DmGetRecordSortPosition(dbP, strP, NULL, (DmCompareFunctionType*)PrvAutoFillLookupCompareRecords, 0);

	if (index)
	{
		recH = DmQueryRecord(dbP, index - 1);

		recP = DmHandleLock(recH);

		if (TxtCaselessCompare(strP, SIZE_MAX, NULL, &recP->text, SIZE_MAX, NULL) == 0)
		{
			insert = false;
			index--;
		}

		DmHandleUnlock(recH);
	}

	// Compute the record size.
	recordSize = sizeof(LookupRecordType) + entryLen;

	if (insert)
	{
		recH = DmNewRecord(dbP, &index, recordSize);

		if (!recH)
			goto exit;

		DmReleaseRecord (dbP, index, true);
	}
	else
	{
		recH = DmResizeRecord(dbP, index, recordSize);

		if (!recH)
			goto exit;
	}

	// Copy the string passed to the record and time stamp the entry with
	// the current time.
	time = TimGetSeconds();
	recH = DmGetRecord(dbP, index);
	recP = DmHandleLock(recH);
	DmWrite(recP, offsetof(LookupRecordType, time), &time, sizeof(uint32_t));
	DmWrite(recP, offsetof(LookupRecordType, text), strP, entryLen + 1);
	DmHandleUnlock(recH);

	DmReleaseRecord (dbP, index, true);

exit:
	DmCloseDatabase (dbP);
}
