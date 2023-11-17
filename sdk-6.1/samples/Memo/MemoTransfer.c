/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoTransfer.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      Memo Book routines to transfer records.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <StringMgr.h>
#include <TextMgr.h>
#include <UIResources.h>

#include <Form.h>
#include <ErrorMgr.h>
#include <TraceMgr.h>
#include <CatMgr.h>
#include <string.h>

#include <ExgLocalLib.h>

#include "MemoDB.h"
#include "MemoMain.h"
#include "MemoRsc.h"
#include "MemoTransfer.h"

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define identifierLengthMax			40
#define mimeVersionString			"MIME-Version: 1.0\015\012"
#define mimeMultipartString 		"Content-type: multipart/mixed;"
#define mimeBoundaryString 			"boundary="
#define memoSuffix					("." memoExtension)
#define simpleBoundary				"simple boundary"
#define delimiter					"--" simpleBoundary
#define crlf						"\015\012"
#define truncCategoryMaxWidth		640

#define importBufferMaxLength		 80

#define stringZLen 					-1 	// pass to WriteFunc to calculate strlen


// Stream interface to exgsockets to optimize performance
#define maxStreamBuf 				512  // MUST BE LARGER than importBufferMaxLength

// Export feature: If the allocated memory is too short, extend it
// by this value.
#define kExportMemAllocationInc		512

enum StreamTag {
	exgStream,
	importExportStream
} ;

typedef Enum8 StreamType ;

typedef struct GenericStreamTag {
	StreamType 	streamType;
	uint8_t		reserved1 ;
	uint16_t		reserved2 ;
} GenericStreamType;

typedef struct ExgStreamTag {
	StreamType streamType;
	uint8_t		reserved1 ;
	uint16_t		reserved2 ;
	ExgSocketPtr socket;
	uint16_t pos;
	uint16_t len;
	uint16_t bufSize;
	uint16_t		reserved3 ;
	char   buf[maxStreamBuf];
} ExgStreamType;

typedef struct ImportExportStreamTag {
	StreamType streamType;
	uint8_t		reserved1 ;
	uint16_t		reserved2 ;
	MemHandle ieStreamH;
	uint32_t	posInStream;
} ImportExportExgStreamType;

/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/

// last record created from MemoImportMime
uint32_t						gRecIDNew = dbMaxRowIndex;

// For attach sublaunch, declarded in MemoMain.c.
extern Boolean					gAttachRequest;
extern uint32_t					gListViewCursorID;


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void			PrvMemoImportMimeCleanup(DmOpenRef dbP, uint32_t firstRecordID, void* inputStream, uint16_t numRecordsReceived, uint16_t* numRecordsReceivedP);
static void			PrvStreamInit(ExgStreamType *streamP, ExgSocketPtr exgSocketP);
static void			PrvStreamFlush(ExgStreamType *streamP);
static uint32_t		PrvStreamWrite(ExgStreamType *streamP, const char * stringP, int32_t length, status_t *errP);
static uint32_t		PrvStreamRead(ExgStreamType * streamP, char *bufP, uint32_t length, status_t *errP);
static ExgSocketPtr PrvStreamSocket(void *streamP);
static uint32_t		PrvReadFunction(void * stream, char * bufferP, uint32_t length, status_t * error);
static uint32_t		PrvWriteFunction(void * stream, const char * const bufferP, int32_t length, status_t * error);
static void			PrvTransferCleanFileName(char* ioFileName);
static void			PrvSetDescriptionAndFilename(char * textP, char **descriptionPP, MemHandle *descriptionHP, char **filenamePP, MemHandle *filenameHP, const char * const prefix);
static status_t		PrvMemoSendRecordTryCatch (DmOpenRef dbP, char* memoP, ExgSocketPtr exgSocketP);
static status_t		PrvMemoSendCategoryTryCatch (DmOpenRef dbP, uint32_t numCategories, CategoryID *categoriesP, ExgSocketPtr exgSocketP);
static void			PrvMemoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, uint32_t uniqueID);
static status_t		PrvReadThroughCRLF(ReadFunctionF inputFunc, void * inputStreamP, char * bufferP, uint16_t * bufferLengthP);
static void			PrvMemoImportFinishRecord(DmOpenRef dbP, uint32_t gRecIDNew);

/************************************************************
 *
 * FUNCTION: MemoImportMime
 *
 * DESCRIPTION: Import a Mime record.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			obeyUniqueIDs - true to obey any unique ids if possible
 *			beginAlreadyRead - whether the begin statement has been read
 *			numRecordsRecievedP - number of records received
 *
 * RETURNS: true if the input was read
 *
 *	REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			rsf		4/24/97			Initial Revision
 *			bob		01/26/98		re-wrote MIME parser part to get delimiters right
 *			grant	6/25/99			Return count of records received in numRecordsReceivedP.
 *			kwk		06/25/99		Moved return out of ErrTry block.
 *			FPa		11/22/00		Fixed ErrTry/Catch/Throw problems
 *
 *************************************************************/
extern Boolean
MemoImportMime(DmOpenRef dbR, void * inputStream, ReadFunctionF inputFunc,
			   Boolean obeyUniqueIDs, Boolean beginAlreadyRead, uint16_t *numRecordsReceivedP, char* descriptionP, uint16_t descriptionSize
			   , status_t * error)
{
	char *		c;
	char		boundaryString[69+2] = "";
	uint32_t	boundaryStrLen = 0;
	DmOpenRef	dbP = dbR;
	char		buffer[importBufferMaxLength + 1];
	uint16_t	bufferLength = 0;
	uint16_t	charsRead;
	uint16_t	charsToWrite;
	uint16_t	newRecordSize = 0;
	char *		nextCrChr;
	int32_t		memoMaxLength, addCr;
	char *		boundaryP;
	char *		boundaryEndP;
	uint16_t	numRecordsReceived = 0;
	uint32_t	firstRecordID = 0;
	Boolean		recordCreated = false;
	CategoryID	categoryID = catIDUnfiled;

	*error = errNone;
	if (obeyUniqueIDs)
	{
		recordCreated = true;
		// gRecIDNew set by caller
	}
	else
	{
		gRecIDNew = dbMaxRowIndex;
	}

	// Get the max memo size for schema
	memoMaxLength = (int32_t) MemoGetSchemaColumnMaxSize(dbR, kMemoDBColumnID) - 1;

	// Keep the buffer always null terminated so we can use string functions on it.
	buffer[importBufferMaxLength] = nullChr;

	// Read chars into the buffer
	charsRead = (uint16_t)inputFunc( inputStream, buffer, importBufferMaxLength - (uint32_t) bufferLength, error);
	bufferLength += charsRead;
	buffer[bufferLength] = nullChr;

	if (charsRead == 0)
	{
		*numRecordsReceivedP = 0;
		return false;
	}

	// An error happens usually due to no memory.  It's easier just to
	// catch the error.  If an error happens, we remove the last record.
	// Then we throw a second time so the caller receives it and displays a message.

	// MIME start, find MIME ID and version
	if (StrNCompare(buffer, mimeVersionString, strlen(mimeVersionString)) == 0)
	{
		// Remove the MIME header
		memmove(buffer, &buffer[strlen(mimeVersionString)], (uint32_t) bufferLength - strlen(mimeVersionString));
		bufferLength -= strlen(mimeVersionString);

		// Read chars into the buffer
		charsRead = (uint16_t)inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - (uint32_t) bufferLength, error);
		bufferLength += charsRead;
		buffer[bufferLength] = nullChr;

		// scan header for a multi-part identifier
		// skip anything else until we get an entirely blank line
		do {
			if (StrNCompare(buffer, mimeMultipartString, strlen(mimeMultipartString)) == 0)
			{
				// found a multi-part header, parse out the boundary string

				// PREVIEW Aba: Here we know that the memo is multipart => several memos
				if (descriptionP)
				{
					MemHandle headerStringH;
					char* 	  headerStringP;

					headerStringH = DmGetResource(gApplicationDbP, strRsc, FindMemoHeaderStr);
					headerStringP = MemHandleLock(headerStringH);
					strcpy(descriptionP, headerStringP);
					MemHandleUnlock(headerStringH);
					DmReleaseResource(headerStringH);

					return true;
				}

				boundaryP = StrStr(buffer, mimeBoundaryString);
				boundaryP += strlen(mimeBoundaryString);

				// Remove the boundary stuff so we can read in more into the buffer
				memmove(buffer, boundaryP, &buffer[bufferLength] - boundaryP);
				bufferLength = (&buffer[bufferLength] - boundaryP);

				// Read chars into the buffer
				charsRead = (uint16_t)inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - (uint32_t) bufferLength, error);
				bufferLength += charsRead;
				buffer[bufferLength] = nullChr;

				boundaryP = buffer;
				if (*boundaryP == '"')
				{
					boundaryP++;
					boundaryEndP = StrChr(boundaryP, '"');
				}
				else
				{
					boundaryEndP = StrChr(boundaryP, crChr);
				}
				if (boundaryEndP == NULL)
				{
					*error=exgErrBadData;
					goto errCatch;
				}
				boundaryString[0] = '-';
				boundaryString[1] = '-';
				memmove(&boundaryString[2], boundaryP, boundaryEndP - boundaryP);
				boundaryString[boundaryEndP - boundaryP + 2] = nullChr;
				boundaryStrLen = strlen(boundaryString);

				c = StrChr(boundaryEndP, crChr);
				if (c == NULL)
				{
					*error=exgErrBadData;
					goto errCatch;
				}
				c += sizeOf7BitChar(crChr) + sizeOf7BitChar(linefeedChr);

				// Remove the boundary stuff so we can read in more into the buffer
				memmove(buffer, c, &buffer[bufferLength] - c);
				bufferLength = (&buffer[bufferLength] - c);
			}
			else
			{
				// just an ordinary header line, skip it
				*error = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
				if (*error < errNone)
					goto errCatch;
			}

			// Read chars into the buffer
			charsRead = (uint16_t)inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - (uint32_t) bufferLength, error);
			bufferLength += charsRead;
			buffer[bufferLength] = nullChr;

			// stop at blank line by itself or EOF
		} while (buffer[0] != crChr && buffer[0] != nullChr);

		// We've now parsed the MIME header.  Preamble, segments, and postamble below.
	} // end of MIME parser

	do {
		// find the boundary and remove it, along with any header info in the body part
		if (*boundaryString != nullChr)
		{
			// Keep reading until we find a boundary
			while (buffer[0] != nullChr && StrNCompare(buffer, boundaryString, boundaryStrLen) != 0)
			{
				*error = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
				if (*error < errNone)
					goto errCatch;
			}

			// Remove the boundary by removing all text until the end of the line.
			*error = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
			if (*error < errNone)
				goto errCatch;

			while (buffer[0] != nullChr && buffer[0] != crChr)
			{
				*error = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
				if (*error < errNone)
					goto errCatch;
			}
			*error = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
			if (*error < errNone)
				goto errCatch;
		}

		// could be that everything was header, and we're out of data.
		// weird error, but MemHandle it.
		if (bufferLength == 0)
		{
			*error=exgErrBadData;
			goto errCatch;
		}


		addCr = 0;
		while (bufferLength > 0 &&
				(*boundaryString == nullChr || StrNCompare(buffer, boundaryString, boundaryStrLen) != 0))
		{
			// find CR or end of buffer
			nextCrChr = StrChr(buffer, crChr);
			if (nextCrChr != NULL)
				charsToWrite = nextCrChr - buffer;
			else
				charsToWrite = bufferLength;

			// PREVIEW Aba: Here we have the first line and we can exit
			if (descriptionP)
			{
				if (charsToWrite >= descriptionSize)
					charsToWrite = descriptionSize - 1;

				StrNCopy(descriptionP, buffer, charsToWrite);
				descriptionP[charsToWrite] = '\0';
				return true;
			}

			// if we're going to overflow record, close it out (leave room for terminating null)
			if ((int32_t) newRecordSize + (int32_t) charsToWrite + addCr > memoMaxLength)
			{
				// since we try to stop parsing at each CR, and most records from other sources (MIME)
				// should have a CR at least every 76 characters, we probably don't have to worry about
				// word wrap.  Still, beaming a lot of just plain text could break records on random
				// boundaries...
				PrvMemoImportFinishRecord(dbP, gRecIDNew);
				addCr = 0;
				numRecordsReceived++;

				// Set 'recordCreated' to false.  Now that we've filled up our
				// current record, we need to create a new one.
				recordCreated = false;
			}

			// Make a record if we need one
			if (! recordCreated)
			{
				MemoDBCreateRecord(dbP, gListViewCursorID, &gRecIDNew, 0, NULL);

				newRecordSize = 0;
				recordCreated = true;
			}

			// Write the buffer out to the record
			if (addCr != 0)
			{
				MemoDBAppendRecord(dbP, &gRecIDNew, "\n", 1);
				newRecordSize++;
			}
			MemoDBAppendRecord(dbP, &gRecIDNew, buffer, charsToWrite);
			newRecordSize += charsToWrite;

			// Remove the chars written so we can read more into the buffer
			if (nextCrChr != NULL)
			{
				if (charsToWrite < importBufferMaxLength-1)
				{
					memmove(buffer, nextCrChr+2, (int32_t) bufferLength-( (int32_t) charsToWrite+2));	// delete LF
					bufferLength -= charsToWrite+2;
				}
				else
					// CR/LF was split by end of buffer, so DON'T delete the CR, catch it next time 'round
				{
					memmove(buffer, nextCrChr, (int32_t) bufferLength-(charsToWrite));		// don't delete CR or LF
					bufferLength -= charsToWrite;
					nextCrChr = NULL;
				}
			}
			else
				buffer[bufferLength = 0] = nullChr;

			// Now read more
			charsRead = (uint16_t)inputFunc( inputStream, &buffer[bufferLength], (int32_t) importBufferMaxLength - bufferLength, error);
			bufferLength += charsRead;
			buffer[bufferLength] = nullChr;

			if (nextCrChr != NULL)
				addCr = 1;
			else
				addCr = 0;
		}	// end of segment parser

		// Set the category for the record
		if ((PrvStreamSocket(inputStream) != NULL) && (PrvStreamSocket(inputStream)->appData))
		{
			categoryID = (CategoryID)PrvStreamSocket(inputStream)->appData;
			DbSetCategory(dbP, gRecIDNew, 1, &categoryID);
		}
		else
			DbSetCategory(dbP, gRecIDNew, 0, NULL);


		PrvMemoImportFinishRecord(dbP, gRecIDNew);
		numRecordsReceived++;
		// A new record will be created if more data is left in the buffer
		recordCreated = false ;

		// save the uniqueID of the first record we loaded
		// we will goto this record when we are done (after sorting)
		if (!firstRecordID)
		{
			// Store the information necessary to navigate to the record inserted.
			firstRecordID = gRecIDNew ;
		}

		// Now that the record is imported check if we need to import any more

		// Stop if there isn't any more input
		if (bufferLength == 0)
			break;

		// Stop if the boundary is followed by "--"
		if ((*boundaryString != nullChr)
			&& bufferLength >= boundaryStrLen + 2
			&& StrNCompare(&buffer[boundaryStrLen], "--", 2) == 0)
			break;

	} while (true);	// end of segment parser

goto errOk;
errCatch:
	// PREVIEW Aba
	if (descriptionP)
		return false;

	// Remove the incomplete record
	if (*error < errNone)
		DbRemoveRow(dbP, gRecIDNew);

	// if we got at least one record, set goto parameters...
	if (firstRecordID)
	{
		if (PrvStreamSocket(inputStream) != NULL)
			PrvMemoSetGoToParams (dbP, PrvStreamSocket(inputStream), firstRecordID);
	}

	// return number of records received
	*numRecordsReceivedP = numRecordsReceived;

	if ( *error != exgMemError )
		PrvMemoImportMimeCleanup(dbP, firstRecordID, inputStream, numRecordsReceived, numRecordsReceivedP);
	return true;

errOk:
	if ( *error >= errNone)
		PrvMemoImportMimeCleanup(dbP, firstRecordID, inputStream, numRecordsReceived, numRecordsReceivedP);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    MemoSendRecord
 *
 * DESCRIPTION: Beam or send a record.
 *
 * PARAMETERS:		dbP - pointer to the database to add the record to
 * 					recordNum - the record to send
 *					prefix - the scheme with ":" suffix and optional "?" prefix
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *		   dje     4/21/00	Add Send support
 *         dje     4/24/00  Don't specify target creator ID
 *
 ***********************************************************************/
extern void MemoSendRecord (DmOpenRef dbP, uint32_t recordNum, const char * const prefix)
{
	char *			memoP;
	MemHandle		memoH;
	MemHandle		descriptionH;
	status_t		error;
	MemHandle		nameH;
	ExgSocketType	exgSocket;

	// important to init structure to zeros...
	memset(&exgSocket, 0x0, sizeof(exgSocket));

	// Form a description of what's being sent.  This will be displayed
	// by the system send dialog on the sending and receiving devices.
	if (MemoDBLoadRecordToHandle(dbP, recordNum, &memoH) != errNone)
		return;

	memoP = (char*)MemHandleLock(memoH);

	// Set the description to be the beginning of the memo
	descriptionH = NULL;
	exgSocket.description = NULL;

	// Set the exg description to the record's description.
	PrvSetDescriptionAndFilename(memoP, &exgSocket.description,
							  &descriptionH, &exgSocket.name, &nameH, prefix);

	// ABa: Clean superfluous '.' characters
	PrvTransferCleanFileName(exgSocket.name);

	exgSocket.length = MemHandleSize(memoH);		// rough guess
	exgSocket.type = (char *)memoMIMEType;

	error = ExgPut(&exgSocket);   // put data to destination
	if (!error)
	{
		error = PrvMemoSendRecordTryCatch(dbP, memoP, &exgSocket);

		ExgDisconnect(&exgSocket, error);
	}


	// Clean up
	if (descriptionH)
	{
		MemHandleUnlock (descriptionH);
		if (MemHandleDataStorage (descriptionH))
			DmReleaseResource(descriptionH);	// DOLATER dje - this shouldn't be possible any more
		else
			MemHandleFree(descriptionH);
	}
	if (nameH)
	{
		MemHandleUnlock (nameH);
		if (MemHandleDataStorage (nameH))
			DmReleaseResource(nameH);		// DOLATER dje - this shouldn't be possible any more
		else
			MemHandleFree(nameH);
	}
	MemHandleUnlock(memoH);
	MemHandleFree(memoH);

	return;
}


/***********************************************************************
 *
 * FUNCTION:    MemoSendCategory
 *
 * DESCRIPTION: Beam or send all visible records in a category.
 *
 * PARAMETERS:		 dbP - pointer to the database to add the record to
 * 					 categoryID - the category of records to send
 *					 prefix - the scheme with ":" suffix and optional "?" prefix
 *					 noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if any records are found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *		   dje     4/21/00	Add Send support
 *         dje     4/24/00  Don't specify target creator ID
 *
 ***********************************************************************/
extern void MemoSendCategory(DmOpenRef dbP, uint32_t numCategories, CategoryID *categoriesP, const char * const prefix, uint16_t noDataAlertID)
{
	status_t		error;
	char			description[catCategoryNameLength];
 	Boolean			foundAtLeastOneRecord;
	ExgSocketType	exgSocket;
	uint16_t		attr;

	// important to init structure to zeros...
	memset(&exgSocket, 0x0, sizeof(exgSocket));

	// Make sure there is at least one record in the category.
 	foundAtLeastOneRecord = false;

	for(DbCursorMoveFirst(gListViewCursorID); !DbCursorIsEOF(gListViewCursorID); DbCursorMoveNext(gListViewCursorID))
	{
		DbGetRowAttr(dbP, gListViewCursorID, &attr);
		if (!(attr & dbRecAttrSecret))
		{
			// The record is not secret. The idea is that secret records are not sent when a
			// category is sent. They must be explicitly sent one by one.
			DbIsRowInCategory(dbP, gListViewCursorID, numCategories, categoriesP, DbMatchAny, &foundAtLeastOneRecord);
			if (foundAtLeastOneRecord)
				break;
		}
 	}

	if (!foundAtLeastOneRecord)
	{
		// There's no record to send.
		FrmUIAlert(noDataAlertID);
		return;
	}

	// Form a description of what's being sent.  This will be displayed
	// by the system send dialog on the sending and receiving devices.
	if (!categoriesP || *categoriesP == catIDAll)
		CatMgrTruncateName(dbP, categoriesP, categoriesP ? 1 : 0, truncCategoryMaxWidth, description);
	else
		CatMgrGetName(dbP, *categoriesP, description);

	exgSocket.description = description;

	// Now form a file name
	exgSocket.name = MemPtrNew((uint32_t) strlen(prefix) + strlen(description) + strlen(memoSuffix) + sizeOf7BitChar('\0'));
	if (exgSocket.name)
	{
		strcpy(exgSocket.name, prefix);
		strcat(exgSocket.name, description);
		strcat(exgSocket.name, memoSuffix);
	}

	// ABa: Clean superfluous '.' characters
	PrvTransferCleanFileName(exgSocket.name);

	exgSocket.length = 0;		// rough guess
	exgSocket.type = (char*)memoMIMEType;

	error = ExgPut(&exgSocket);   // put data to destination

	if (!error)
	{
		error = PrvMemoSendCategoryTryCatch(dbP, numCategories, categoriesP, &exgSocket);
		ExgDisconnect(&exgSocket, error);
	}

	// Clean up
	if (exgSocket.name)
		MemPtrFree(exgSocket.name);
}


/***********************************************************************
 *
 * FUNCTION:		MemoReceiveData
 *
 * DESCRIPTION:		Receives data into the output field using the Exg API
 *
 * PARAMETERS:		dbP - database to put received memos in
 *						exgSocketP - socket from the app code sysAppLaunchCmdExgReceiveData
 *						numRecordsReceivedP - number of records received is returned here
 *
 * RETURNED:		error code or zero for no error.
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	6/25/99	Keep count of received records and return in numRecordsReceivedP
 *
 ***********************************************************************/
extern status_t MemoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP, uint16_t *numRecordsReceivedP)
{
	status_t err;
	uint16_t numRecordsReceived;
	ExgStreamType stream;

	// initialize new record count
	ErrNonFatalDisplayIf(numRecordsReceivedP == NULL, "NULL numRecordsReceivedP");
	*numRecordsReceivedP = 0;

	PrvStreamInit( &stream, exgSocketP);

	// accept will open a progress dialog and wait for your receive commands
	err = ExgAccept(exgSocketP);

	if (err >= errNone)
	{
		// Catch errors receiving records.  The import routine will clean up the
		// incomplete record.  This routine displays an error message.
		// Keep importing records until it can't
		while (MemoImportMime(dbP, &stream, PrvReadFunction, false, false, &numRecordsReceived, NULL, 0, &err))
		{
			if (err < errNone)
				goto errCatch;
			*numRecordsReceivedP += numRecordsReceived;
		};

// catch the records from the final MemoImportMime
		*numRecordsReceivedP += numRecordsReceived;

errCatch:

		// Aba: A record has been added in the Database iff the GoTo
		// uniqueID parameter != 0.
		// In the case no record is added, return an error
		if (err >= errNone && exgSocketP->goToParams.uniqueID == 0)
			err = exgErrBadData;

		ExgDisconnect(exgSocketP, err); // closes transfer dialog
		err = errNone;	// error was reported, so don't return it
	}

	return err;
}


/************************************************************
 *
 * FUNCTION: MemoExportMime
 *
 * DESCRIPTION: Export a record as a Imc Mime record
 *
 * PARAMETERS:
 *			dbP - pointer to the database to export the records from
 *			index - the record number to export
 *			recordP - whether the begin statement has been read
 *			outputStream - pointer to where to export the record to
 *			outputFunc - function to send output to the stream
 *			writeUniqueIDs - true to write the record's unique id
 *
 * RETURNS: nothing
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rsf	8/6/97	Initial Revision
 *
 *************************************************************/

void MemoExportMime(DmOpenRef dbP, char* memoP,
					void * outputStream, WriteFunctionF outputFunc, Boolean writeUniqueIDs,
					Boolean outputMimeInfo, status_t * error)
{
	char * c;
	char * eolP;
	uint32_t len;
	CharEncodingType charEncoding;


	// Write out all of the memo.  All linefeeds must be replaced with CRLF combos.
	c = memoP;

	if (outputMimeInfo)
	{
		if ((charEncoding = TxtStrEncoding(c)) != charEncodingAscii)
		{
			outputFunc(outputStream, "Content-Type: Text/plain; charset=" , stringZLen, error);
			outputFunc(outputStream, TxtEncodingName(charEncoding), stringZLen, error);
			outputFunc(outputStream, crlf, stringZLen, error);
		}

		outputFunc(outputStream, crlf, stringZLen, error);
	}

	while (*c != '\0')
	{
		eolP = StrChr(c, linefeedChr);
		if (eolP)
		{
			len = outputFunc( outputStream, c, eolP - c, error);

			outputFunc(outputStream, crlf, stringZLen, error);
			c = eolP + sizeOf7BitChar(linefeedChr);
		}
		else if (*c != '\0')
		{
			eolP = StrChr(c, '\0');
			len = outputFunc( outputStream, c, eolP - c, error);

			c = eolP;
		}
	}
	outputFunc(outputStream, crlf, stringZLen, error);	// always end with an extra crlf
}

/***********************************************************************
 *
 * FUNCTION:		MemoTransferPreview
 *
 * DESCRIPTION:	Create a short string preview of the data coming in.
 *
 * PARAMETERS:		infoP - the preview info from the command parameter block
 *						        of the sysAppLaunchCmdExgPreview launch
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         ABa    11/10/00   Created
 *
 ***********************************************************************/
void MemoTransferPreview(ExgPreviewInfoType *infoP)
{
	status_t 	err;
	uint16_t 			numRecordsReceived;
	ExgStreamType 		stream;

	if (infoP->op == exgPreviewQuery)
	{
		infoP->types = exgPreviewShortString;
		return;
	}
	if (infoP->op != exgPreviewShortString)
	{
		infoP->error = exgErrNotSupported;
		return;
	}

	// if we have a description we don't have to parse the vObject
	if (infoP->socketP->description && *infoP->socketP->description)
	{
		StrNCopy(infoP->string, infoP->socketP->description, (int16_t)(infoP->size - 1));
		infoP->string[infoP->size - 1] = 0;
		infoP->error = errNone;
		return;
	}

	PrvStreamInit(&stream, infoP->socketP);

	err = ExgAccept(infoP->socketP);

	if (err >= errNone)
	{
		MemoImportMime((DmOpenRef) NULL, &stream, PrvReadFunction, false, false, &numRecordsReceived, infoP->string, (uint16_t)infoP->size, &err);
		ExgDisconnect(infoP->socketP, err); // closes transfer dialog
	}

	infoP->error = err;
}


/***********************************************************************
 *
 * FUNCTION:    PrvImportReadFunction
 *
 * DESCRIPTION: Function to get a character from the import stream
 *
 * PARAMETERS:  stream - the stream that handles data
 *				bufferP - buffer accepting data
 *				length - length of available buffer space
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/15/97   Initial Revision
 *         PLe     05/29/02  Added support for export feature
 *
 ***********************************************************************/
static uint32_t PrvImportReadFunction(void * stream, char * bufferP, uint32_t length, status_t * error)
{
	ImportExportExgStreamType * inputStreamP = (ImportExportExgStreamType*)stream;
	MemHandle inputBufH = inputStreamP->ieStreamH;
	char* inputBufP = (char*) MemHandleLock(inputBufH);
	uint32_t totalInputBufSize = MemHandleSize(inputBufH);
	uint32_t bytesRead = 0;
	* error = errNone;

	while ((length > 0) && (inputStreamP->posInStream < totalInputBufSize))
	{
		bufferP[bytesRead++] = inputBufP[(inputStreamP->posInStream)++];
		length--;
	}

	MemHandleUnlock(inputBufH);

	return bytesRead;
}


/***********************************************************************
 *
 * FUNCTION:		MemoImportData
 *
 * DESCRIPTION:		Receives data from an imported vObject
 *
 * PARAMETERS:		dbP - pointer to the database to add the record to
 *					impExpObjP - pointer on the record param Ptr
 *
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         PLe    05/27/02  Added Import / Export feature
 *
 ***********************************************************************/
status_t MemoImportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t err = errNone;
	ImportExportExgStreamType importStream;
	uint16_t numRecordsReceived;
	Boolean memoRead = false;
	Boolean obeyUniqueIDs = false;
	uint32_t cursorID;
	CategoryID catID = catIDAll;
	DmOpenRef dbRef = dbP;

	if (impExpObjP->vObjectH == NULL)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	// Initialize ImportExportExgStreamType struct
	importStream.ieStreamH = impExpObjP->vObjectH;
	importStream.posInStream = 0;
	importStream.streamType = importExportStream;

	// Set gRecIDNew from the passed in uniqueID if it is valid
	if (impExpObjP->uniqueID != dmUnusedRecordID)
	{
		gRecIDNew = impExpObjP->uniqueID;
		// Clear the record before reusing it, since MemoImportMime
		// used append and assumes that the record is empty.
		DbWriteColumnValue(dbRef, gRecIDNew, kMemoDBColumnID, 0, -1, "", 1);
		obeyUniqueIDs = true;
	}

	memoRead = MemoImportMime(dbRef, &importStream, PrvImportReadFunction,
			obeyUniqueIDs, false, &numRecordsReceived, NULL, 0, &err);

	// return the index and id of the last memo created
	if (memoRead)
	{
		impExpObjP->uniqueID = gRecIDNew;
		MemoDBOpenCursor(&dbRef, &cursorID, &catID, 1, false);
		DbCursorGetPositionForRowID(cursorID, impExpObjP->uniqueID, &(impExpObjP->index));
		DbCursorClose(cursorID);
		(impExpObjP->index)--;
	}


Exit:
	if (err >= errNone && !memoRead)
		err = exgErrBadData;
	return err;
}


/***********************************************************************
 *
 * FUNCTION:    PrvExportWriteFunction
 *
 * DESCRIPTION: Function to put a string for export transport.
 *
 * PARAMETERS:  stream - the stream that handles data
 *				bufferP - buffer pointing on data to export
 *				length - length of available data
 *
 * RETURNED:    nothing
 *					 If the all the string isn't sent an error is thrown using ErrThrow.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/15/97   Initial Revision
 *         PLe     05/29/02  Added support for export feature
 *
 ***********************************************************************/
static uint32_t PrvExportWriteFunction(void * stream, const char * const bufferP, int32_t length, status_t *error)
{
	ImportExportExgStreamType * outputStreamP = (ImportExportExgStreamType*)stream;
	MemHandle outputBufH = outputStreamP->ieStreamH;
	uint32_t totalOutputBufSize = MemHandleSize(outputBufH);
	uint32_t currentPos = 0;
	char* outputBufP = (char*) MemHandleLock(outputBufH);

	*error = errNone;

	// passing -1 length will assume a null terminated string
	if (length == -1)  length = strlen(bufferP);

	while (length > 0)
	{
		if ((outputStreamP->posInStream + 2) > totalOutputBufSize)
		{
			// The output buffer needs to be resized
			MemHandleUnlock(outputBufH);
			totalOutputBufSize += kExportMemAllocationInc;
			*error = MemHandleResize(outputBufH, totalOutputBufSize);
			if (*error < errNone)
				goto Exit;
			outputBufP = MemHandleLock(outputBufH);
		}

		outputBufP[(outputStreamP->posInStream)++] = bufferP[currentPos++];
		// Put a chrNull (may be overwritten by next call to PrvExportWriteFunction)
		// Note: this seems inefficient (to constantly null terminate the result), but
		// it is just a simple write to a buffer and is simpler than having to check
		// and regrow the buffer outside of this loop.  And, this routine is called
		// at least once (when exporting data), to append a final CRLF, so there will
		// be guaranteeds that the result is null terminated.
		outputBufP[(outputStreamP->posInStream)] = '\0';
		length --;
	}
	MemHandleUnlock(outputBufH);

Exit:
	// If the bytes were not sent throw an error.
	if (*error >= errNone && currentPos == 0 && length > 0)
		*error = exgErrAppError;

	return currentPos;
}



/***********************************************************************
 *
 * FUNCTION:		MemoExportData
 *
 * DESCRIPTION:		Export a specified record as vObject
 *
 * PARAMETERS:		dbP - pointer to the database to get the record from
 *					impExpObjP - pointer on the record param Ptr
 *
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         PLe    05/27/02  Added Import / Export feature
 *
 ***********************************************************************/
status_t MemoExportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t err = errNone;
	char *		memoP = NULL;
	MemHandle	memoH = NULL;
	ImportExportExgStreamType exportStream;
	uint32_t cursorID;
	uint32_t nRows, uniqueID;
	DmOpenRef dbRef = dbP;

	MemoDBOpenIOCursor(&dbRef, &cursorID);
	nRows = DbCursorGetRowCount(cursorID);

	if (impExpObjP->vObjectH == NULL)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	// Check the validity of the record index
	if (impExpObjP->index == ImpExpInvalidRecIndex)
		uniqueID = impExpObjP->uniqueID;
	else
	{
		if (impExpObjP->index >= nRows)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
		else
		{
			DbCursorGetRowIDForPosition(cursorID, impExpObjP->index+1, &uniqueID);
			impExpObjP->uniqueID = uniqueID;
		}
	}

	// Initialize ImportExportExgStreamType struct
	exportStream.ieStreamH = impExpObjP->vObjectH;
	exportStream.posInStream = 0;
	exportStream.streamType = importExportStream;

	MemoDBLoadRecordToHandle(dbRef, uniqueID, &memoH);
    if (memoH)
    {
		memoP = (char*)MemHandleLock(memoH);
		// Note: Do NOT export the mime header.
		MemoExportMime(dbRef, memoP, &exportStream,
			PrvExportWriteFunction, false, false, &err);
    }
    else
    	err = DmGetLastErr();

Exit:
	DbCursorClose(cursorID);
	if (memoH)
	{
        MemHandleUnlock(memoH);
		MemHandleFree(memoH);
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:		MemoDeleteData
 *
 * DESCRIPTION:		Delete specified record
 *
 * PARAMETERS:		dbP - pointer to the database to get the record from
 *					impExpObjP - pointer on the record param Ptr
 *
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         LYr    10/15/03  Initial revision
 *
 ***********************************************************************/
status_t MemoDeleteData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t 	err = errNone;
	uint32_t 	cursorID, nRows;
	DmOpenRef 	dbRef = dbP;
	uint32_t	index = impExpObjP->index + 1;

	MemoDBOpenIOCursor(&dbRef, &cursorID);
	nRows = DbCursorGetRowCount(cursorID);

	if ((impExpObjP->index != ImpExpInvalidRecIndex) && (impExpObjP->index < nRows))
	{
		// Check the validity of the record index
		if (DbCursorGetRowIDForPosition(cursorID, index, &impExpObjP->uniqueID) < errNone)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
	} else if (impExpObjP->uniqueID != ImpExpInvalidUniqueID)
	{
		// Get the index from the unique ID
		if (DbCursorGetPositionForRowID(cursorID, impExpObjP->uniqueID, &index) < errNone)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
	}
	else
	{
		// No valid index or unique ID
		err = dmErrInvalidParam;
		goto Exit;
	}

	// Assign the result
	impExpObjP->index = index - 1;

	// Now delete the record
	err = DbCursorMoveToRowID(cursorID, impExpObjP->uniqueID);
	if (err >= errNone)
		err = DbDeleteRow(dbP, cursorID);

Exit:
	DbCursorClose(cursorID);
	if (err != errNone)
	{
		impExpObjP->index = ImpExpInvalidRecIndex;
		impExpObjP->uniqueID = ImpExpInvalidUniqueID;
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:		MemoMoveData
 *
 * DESCRIPTION:		Move a record to the specified location
 *
 * PARAMETERS:		dbP - pointer to the database to get the record from
 *					impExpObjP - pointer on the record param Ptr
 *
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         LYr	10/29/03	Created
 *
 ***********************************************************************/
status_t MemoMoveData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t err = errNone;
	uint32_t cursorID;
	DmOpenRef dbRef = dbP;

	if (impExpObjP->index == ImpExpInvalidRecIndex || impExpObjP->destIndex == ImpExpInvalidRecIndex)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	err = MemoDBOpenIOCursor(&dbRef, &cursorID);
	if (err < errNone)
		goto Exit;

	err = DbCursorRelocateRow(gListViewCursorID, impExpObjP->index, impExpObjP->destIndex);
	if (err >= errNone)
		err = DbCursorGetRowIDForPosition(gListViewCursorID, impExpObjP->destIndex, &impExpObjP->uniqueID);

	DbCursorClose(cursorID);
	if (err < errNone)
	{
		impExpObjP->index = ImpExpInvalidRecIndex;
		impExpObjP->uniqueID = dmUnusedRecordID;
	}

Exit:
	return err;
}


#if 0
#pragma mark -
#endif
/***********************************************************************
 *
 * FUNCTION:    PrvStreamInit
 *
 * DESCRIPTION: Function to put Initialize a stream socket.
 *
 * PARAMETERS:  streamP - the output stream
 *				exgSocketP - pointer to an intitialized exgSocket
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static void PrvStreamInit(ExgStreamType *streamP, ExgSocketPtr exgSocketP)
{
	streamP->streamType = exgStream;
	streamP->socket = exgSocketP;
	streamP->bufSize = maxStreamBuf;
	streamP->pos = 0;
	streamP->len = 0;
}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamFlush
 *
 * DESCRIPTION: Function to put a string to the exg transport.
 *
 * PARAMETERS:  streamP - the output stream
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static void PrvStreamFlush(ExgStreamType *streamP)
{
	status_t err = 0;
	while (streamP->len && err >= errNone)
		streamP->len -= (uint16_t)ExgSend(streamP->socket,streamP->buf,streamP->len,&err);

}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamWrite
 *
 * DESCRIPTION: Function to put a string to the exg transport.
 *
 * PARAMETERS:  streamP - the output stream
 *				stringP - the string to put
 *
 * RETURNED:    nothing
 *				If the all the string isn't sent an error is thrown using ErrThrow.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static uint32_t PrvStreamWrite(ExgStreamType *streamP, const char * stringP, int32_t length, status_t *errP)
{
	int32_t count = 0;
	*errP = 0;

	while (count < length && !*errP)
	{
		if (streamP->len < streamP->bufSize)
		{
			streamP->buf[streamP->len++] = *stringP++;
			count++;
		}
		else
			streamP->len -= (uint16_t)ExgSend(streamP->socket, streamP->buf, streamP->len, errP);
	}
	return count;
}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamRead
 *
 * DESCRIPTION: Function to get a character from the input stream.
 *
 * PARAMETERS:  streamP - the output stream
 *
 * RETURNED:    a character of EOF if no more data
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static uint32_t PrvStreamRead(ExgStreamType * streamP, char *bufP, uint32_t length, status_t *errP)
{
	uint32_t count = 0;

	*errP = 0;
	while (count < length)
	{
		if (streamP->pos < streamP->len)
			bufP[count++] = streamP->buf[streamP->pos++];
		else
		{	streamP->pos = 0;
			streamP->len = (uint16_t)ExgReceive(streamP->socket, streamP->buf, streamP->bufSize, errP);
		if (!streamP->len || *errP)
			break;
		}
	}
	return count;
}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamSocket
 *
 * DESCRIPTION: returns the socket from a stream.
 *
 * PARAMETERS:  streamP - the output stream
 *
 * RETURNED:    The socket associated with the stream
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static ExgSocketPtr PrvStreamSocket(void *streamP)
{
	if (((GenericStreamType*)streamP)->streamType == exgStream)
		return ((ExgStreamType*)streamP)->socket;
	else return NULL;
}

/***********************************************************************
 *
 * FUNCTION:    GetChar
 *
 * DESCRIPTION: Function to get a character from the exg transport.
 *
 * PARAMETERS:  exgSocketP - the exg connection
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/15/97   Initial Revision
 *
 ***********************************************************************/
static uint32_t PrvReadFunction(void * stream, char * bufferP, uint32_t length, status_t * error)
{
	uint32_t bytesRead;
	bytesRead = PrvStreamRead((ExgStreamType *)stream, bufferP, length, error);
	return bytesRead;
}


/***********************************************************************
 *
 * FUNCTION:    PutString
 *
 * DESCRIPTION: Function to put a string to the exg transport.
 *
 * PARAMETERS:  exgSocketP - the exg connection
 *					 stringP - the string to put
 *
 * RETURNED:    nothing
 *					 If the all the string isn't sent an error is thrown using ErrThrow.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/15/97   Initial Revision
 *
 ***********************************************************************/
static uint32_t PrvWriteFunction(void * stream, const char * const bufferP, int32_t length, status_t * error)
{
	uint32_t len;

	// passing -1 length will assume a null terminated string
	if (length == -1)  length = strlen(bufferP);

	len = PrvStreamWrite( stream, bufferP, length, error);

	// If the bytes were not sent throw an error.
	if (*error >= errNone && len == 0 && length > 0)
		*error = exgErrAppError;

	return len;
}


/***********************************************************************
 *
 * FUNCTION:		PrvTransferCleanFileName
 *
 * DESCRIPTION:		Remove dot characters in file name but not the least
 * PARAMETERS:		a pointer to a string
 *
 * RETURNED:		String parameter doesn't contains superfluous dot characters
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		7/28/00		Created
 *
 ***********************************************************************/
static void PrvTransferCleanFileName(char* ioFileName)
{
	char* 	mayBeLastDotP;
	char*  	lastDotP;
	uint32_t	chrFullStopSize = TxtCharSize(chrFullStop);

	// prevent NULL & empty string
	if (ioFileName == NULL || *ioFileName == 0)
		return;

	// remove dot but not the last one
	mayBeLastDotP = StrChr(ioFileName, 	chrFullStop);
	while ((lastDotP = StrChr(mayBeLastDotP + chrFullStopSize, chrFullStop)) != NULL)
	{
		// remove the dot
		strcpy(mayBeLastDotP, mayBeLastDotP + chrFullStopSize);
		mayBeLastDotP = lastDotP - chrFullStopSize;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvSetDescriptionAndFilename
 *
 * DESCRIPTION: Derive and allocate a decription and filename from some text.
 *				The filename will be a URL which includes the specified scheme.
 *
 * PARAMETERS:  textP - the text string to derive the names from
 *					 descriptionPP - pointer to set to the allocated description
 *					 descriptionHP - MemHandle to set to the allocated description
 *					 filenamePP - pointer to set to the allocated filename
 *					 filenameHP - MemHandle to set to the allocated description
 *					 prefix - the scheme with ":" suffix and optional "?" prefix
 *
 * RETURNED:    a description and filename are allocated and the pointers are set
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   11/4/97   Initial Revision
 *
 ***********************************************************************/
static void PrvSetDescriptionAndFilename(char * textP, char **descriptionPP,
									  MemHandle *descriptionHP, char **filenamePP, MemHandle *filenameHP, const char * const prefix)
{
	char * descriptionP = NULL;
	size_t descriptionSize;
	Coord descriptionWidth;
	Boolean descriptionFit;
	char * spaceP;
	char * filenameP;
	MemHandle resourceH;
	char * resourceP;
	uint8_t filenameLength;
	uint8_t schemeLength;
	Coord unused;


	descriptionSize = strlen(textP);
	WinGetDisplayExtent(&descriptionWidth, &unused);
	FntCharsInWidth (textP, &descriptionWidth, &descriptionSize, &descriptionFit);

	if (descriptionSize > 0)
	{
		*descriptionHP = MemHandleNew((uint32_t)descriptionSize+sizeOf7BitChar('\0'));
		if (*descriptionHP)
		{
			descriptionP = MemHandleLock(*descriptionHP);
			memmove(descriptionP, textP, descriptionSize);
			descriptionP[descriptionSize] = nullChr;
		}
	}
	else
	{
		*descriptionHP = DmGetResource(gApplicationDbP, strRsc, BeamDescriptionStr);
		descriptionP = MemHandleLock(*descriptionHP);
	}


	if (descriptionSize > 0)
	{
		// Now form a file name.  Use only the first word or two.
		spaceP = StrChr(descriptionP, spaceChr);
		if (spaceP)
			// Check for a second space
			spaceP = StrChr(spaceP + sizeOf7BitChar(spaceChr), spaceChr);

		// If at least two spaces were found then use only that much of the description.
		// If less than two spaces were found then use all of the description.
		if (spaceP)
			filenameLength = spaceP - descriptionP;
		else
			filenameLength = (uint8_t)strlen(descriptionP);


		// Allocate space and form the filename
		schemeLength = (uint8_t)strlen(prefix);
		*filenameHP = MemHandleNew((uint32_t) schemeLength + filenameLength + strlen(memoSuffix) + sizeOf7BitChar('\0'));
		filenameP = MemHandleLock(*filenameHP);
		if (filenameP)
		{
			strcpy(filenameP, prefix);
			memmove(&filenameP[schemeLength], descriptionP, filenameLength);
			memmove(&filenameP[schemeLength + filenameLength], memoSuffix,
					(int32_t) strlen(memoSuffix) + sizeOf7BitChar('\0'));
		}
	}
	else
	{
		resourceH = DmGetResource(gApplicationDbP, strRsc, BeamFilenameStr);
		resourceP = MemHandleLock(resourceH);

		// Allocate space and form the filename
		filenameLength = (uint8_t)strlen(resourceP);
		schemeLength = (uint8_t)strlen(prefix);
		*filenameHP = MemHandleNew((uint32_t) schemeLength + filenameLength + sizeOf7BitChar('\0'));
		filenameP = MemHandleLock(*filenameHP);
		if (filenameP)
		{
			strcpy(filenameP, prefix);
			strcat(filenameP, resourceP);
		}

		MemHandleUnlock(resourceH);
		DmReleaseResource(resourceH);
	}


	*descriptionPP = descriptionP;
	*filenamePP = filenameP;
}


/***********************************************************************
 *
 * FUNCTION:    PrvMemoSendRecordTryCatch
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	 dbP - pointer to the database to add the record to
 * 				 recordNum - the record number to send
 * 				 recordP - pointer to the record to send
 * 				 exgSocketP - the exchange socket used to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/11/97  Initial Revision
 *
 ***********************************************************************/
static status_t PrvMemoSendRecordTryCatch (DmOpenRef dbP, char* memoP, ExgSocketPtr exgSocketP)
{
	status_t error = errNone;
	ExgStreamType stream;

	PrvStreamInit(&stream, exgSocketP);

	MemoExportMime(dbP, memoP, &stream, PrvWriteFunction, true, false, &error);

	PrvStreamFlush( &stream);
	return error;
}


/***********************************************************************
 *
 * FUNCTION:    PrvMemoSendCategoryTryCatch
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:		dbP - pointer to the database to add the record to
 * 					categoryID - the category of records to send
 * 					exgSocketP - the exchange socket used to send
 * 					index - the record number of the first record in the category to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/11/97  Initial Revision
 *
 ***********************************************************************/
static status_t PrvMemoSendCategoryTryCatch (DmOpenRef dbP, uint32_t numCategories, CategoryID *categoriesP,
									 ExgSocketPtr exgSocketP)
{
	status_t		error = errNone;
	MemHandle		outMemoH = 0;
	char *			outMemoP;
	ExgStreamType	stream;
	uint32_t		index, numRecs;
	uint16_t		attr;

	PrvStreamInit(&stream, exgSocketP);

	// Write out the beginning of a multipart mime message
	PrvWriteFunction(&stream,
					mimeVersionString
					"Content-type: multipart/mixed; boundary=\"" simpleBoundary "\"" crlf crlf, stringZLen, &error);

	// Loop through all records in the category.
	index = 1;
	numRecs = MemoDBNumVisibleRecordsInCategory(gListViewCursorID);
	while (index <= numRecs)
	{
		DbCursorSetAbsolutePosition(gListViewCursorID, index++);
		DbGetRowAttr(dbP, gListViewCursorID, &attr);
		// Is the record secret? The idea is that secret records are not sent when a
		// category is sent. They must be explicitly sent one by one.
		if (!(attr & dbRecAttrSecret))
		{
			// Emit the record.  If the record is private do not emit it.
			MemoDBLoadRecordToHandle(dbP, gListViewCursorID, &outMemoH);
			if (outMemoH != NULL)
			{
				outMemoP = (char*)MemHandleLock(outMemoH);

				// Emit a mime boundary
				PrvWriteFunction(&stream, delimiter crlf, stringZLen, &error);
				if (error < errNone)
					goto errCatch;

				MemoExportMime(dbP, outMemoP, &stream, PrvWriteFunction, true, true, &error);
				if (error < errNone)
					goto errCatch;

				MemHandleUnlock(outMemoH);
				MemHandleFree(outMemoH);
			}
		}
	}
	outMemoH = NULL;
	dbP = 0;

	// All done.  Write out an epilogue.
	PrvWriteFunction(&stream, delimiter "--" crlf crlf, stringZLen, &error);

errCatch:

	if (outMemoH)
	{
		MemHandleUnlock(outMemoH);
		MemHandleFree(outMemoH);
	}

	PrvStreamFlush(&stream);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:		PrvMemoSetGoToParams
 *
 * DESCRIPTION:	Store the information necessary to navigate to the
 *                record inserted into the launch code's parameter block.
 *
 * PARAMETERS:		 dbP        - pointer to the database to add the record to
 *						 exgSocketP - parameter block passed with the launch code
 *						 uniqueID   - unique id of the record inserted
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/17/97	Created
 *
 ***********************************************************************/
static void PrvMemoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, uint32_t uniqueID)
{
	uint32_t		recordNum;
	DatabaseID	dbID;

	if (! uniqueID) return;

	DmGetOpenInfo(dbP, &dbID, NULL, NULL, NULL);

	// The this the the first record inserted, save the information
	// necessary to navigate to the record.
	if (! exgSocketP->goToParams.uniqueID)
	{
		DbCursorGetRowIDForPosition(gListViewCursorID, uniqueID, &recordNum);

		exgSocketP->goToCreator = sysFileCMemo;
		exgSocketP->goToParams.uniqueID = uniqueID;
		exgSocketP->goToParams.dbID =  dbID;
		exgSocketP->goToParams.recordNum = (uint16_t)recordNum;
	}

	// If we already have a record then make sure the record index
	// is still correct.  Don't update the index if the record is not
	// in your the app's database.
	else if (dbID == exgSocketP->goToParams.dbID)
	{
		DbCursorGetRowIDForPosition(gListViewCursorID, exgSocketP->goToParams.uniqueID, &recordNum);

		exgSocketP->goToParams.recordNum = (uint16_t)recordNum;
	}
}


/************************************************************
 *
 * FUNCTION: PrvReadThroughCRLF
 *
 * DESCRIPTION: Consume data up to and including the next CRLF.
 *
 * PARAMETERS:
 *			inputStreamP	- pointer to where to import from
 *			bufferP - where the input stream is stored
 *			bufferLengthP - the length of bufferP used
 *
 * RETURNED: error code or zero for no error.
 *
 * ASSUMPTIONS:
 *			Buffer is full when called
 *  		Buffer is big enough to hold a full line (including LF)
 *			  ...so CR/LF will never split
 *	END CONDITION:
 *			Buffer is full when routine exits.
 *
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			bob	1/26/98  initial revision
 *
 *************************************************************/
static status_t PrvReadThroughCRLF(ReadFunctionF inputFunc, void * inputStreamP, char * bufferP, uint16_t * bufferLengthP)
{
	char *c;
	uint16_t charsRead;
	status_t err = errNone;
	Boolean gotOne = false;

	while (*bufferLengthP > 0 && !gotOne)
	{
		c = StrChr(bufferP, crChr);
		if (c == NULL)
			c = &bufferP[*bufferLengthP];		// end of the buffer
		else if (c < bufferP + *bufferLengthP - 1)		// guard against buffer splitting cr/lf
		{
			c += sizeOf7BitChar(crChr) + sizeOf7BitChar(linefeedChr);
			gotOne = true;
		}

		// Consume everything up to the CR/NULL
		memmove(bufferP, c, &bufferP[*bufferLengthP] - c);
		*bufferLengthP = &bufferP[*bufferLengthP] - c;

		// Read in more chars
		charsRead = (uint8_t)inputFunc(inputStreamP, bufferP + *bufferLengthP, (uint32_t) importBufferMaxLength - *bufferLengthP, &err);
		*bufferLengthP += charsRead;
		bufferP[*bufferLengthP] = nullChr;
	}

	return err;
}


/************************************************************
 *
 * FUNCTION: PrvMemoImportFinishRecord
 *
 * DESCRIPTION: Make sure record is null terminated, and close it.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			gRecIDNew	- index of new record
 *			newRecordH - MemHandle to new record
 *			newRecordSize - bytes currently in new record
 *
 *	REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			bob	1998-02-05	Moved out of MemoImportMime
 *
 *************************************************************/
static void PrvMemoImportFinishRecord(DmOpenRef dbP, uint32_t gRecIDNew)
{
	char *		dataP = NULL;
	uint32_t	dataSize;

	// Make sure the record is nullChr terminated
	if (MemoDBLoadRecord(dbP, gRecIDNew, (void**)&dataP, &dataSize) < errNone || dataP == NULL)
		return;

	if (*(dataP + dataSize - 1) != nullChr)
		MemoDBAppendRecord(dbP, &gRecIDNew, "", sizeOf7BitChar(nullChr));

	DbReleaseStorage(dbP, dataP);
}


/************************************************************
 *
 * FUNCTION: PrvMemoImportMimeCleanup
 *
 * DESCRIPTION: Cleanup function for MemoImportMime
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			firstRecordID - uniqueID of the first record we loaded
 *			inputStream	- pointer to where to import the record from
 *			numRecordsReceived - number of records received
 *			numRecordsReceivedP - pointer to the number of records received
 *
 * RETURNS: None
 *
 *	REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			FPa		11/22/00		Initial Revision
 *
 *************************************************************/
void PrvMemoImportMimeCleanup(DmOpenRef dbP, uint32_t firstRecordID, void* inputStream, uint16_t numRecordsReceived, uint16_t* numRecordsReceivedP)
{
	// if we got at least one record, sort and set goto parameters...
	if (firstRecordID)
	{
		//MemoSort(dbP);
		if (PrvStreamSocket(inputStream) != NULL)
			PrvMemoSetGoToParams (dbP, PrvStreamSocket(inputStream), firstRecordID);
	}

	// return number of records received
	*numRecordsReceivedP = numRecordsReceived;
}
