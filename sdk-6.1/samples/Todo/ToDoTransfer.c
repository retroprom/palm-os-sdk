/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoTransfer.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      To Do routines to transfer records.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <CatMgr.h>
#include <Chars.h>
#include <DataMgr.h>
#include <ErrorMgr.h>
#include <ExgMgr.h>
#include <ExgLocalLib.h>
#include <string.h>

#include <Font.h>
#include <Form.h>
#include <MemoryMgr.h>

#include <PdiLib.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <TraceMgr.h>

#include <UDAMgr.h>

#include <SystemResources.h>
#include <UIResources.h>

#include "ToDo.h"
#include "ToDoTransfer.h"
#include "ToDoRsc.h"

#define identifierLengthMax			80
#define tempStringLengthMax			512
#define defaultPriority				1
#define todoSuffix					".vcs"
#define todoMIMEType				"text/x-vCalendar"

// vObject increment size (used to increase the initial size)
#define ALLOCATE_VOBJECT_INC_SIZE		512

// Aba: internal version of vCalendar. Must be updated
// the export side of the vCalendar code evoluate

#define kVObjectVersion				"6.1"

/************************************************************
 *	Global variables
 ************************************************************/
extern uint32_t			gListViewCursorID;
extern CategoryID		CurrentCategory;


/***********************************************************************
 *
 * FUNCTION:		PrvPdiLibLoad
 *
 * DESCRIPTION:		Load Pdi library
 * PARAMETERS:		a pointer to an integer: the refNum of the libary
 *						return whether the library had to be loaded (and therefore
 *								needs to be unloaded)
 *
 * RETURNED:		An error if library is not found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		4/10/00		Created
 *
 ***********************************************************************/
static status_t PrvPdiLibLoad()
{
	return PdiLibOpen();
}

/***********************************************************************
 *
 * FUNCTION:		PrvPdiLibUnload
 *
 * DESCRIPTION:		Unload Pdi library
 * PARAMETERS:		The refnum of the pdi library
 *						Whether the library was loaded (and therefore needs to be unloaded)
 *
 * RETURNED:		NONE
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		4/10/00		Created
 *
 ***********************************************************************/
static void PrvPdiLibUnload()
{
	PdiLibClose();
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
	lastDotP = StrChr(mayBeLastDotP + chrFullStopSize, chrFullStop);
	while (lastDotP != NULL)
	{
		// remove the dot
		strcpy(mayBeLastDotP, mayBeLastDotP + chrFullStopSize);
		mayBeLastDotP = lastDotP - chrFullStopSize;
		lastDotP = StrChr(mayBeLastDotP + chrFullStopSize, chrFullStop);
	}
}

/************************************************************
 *
 * FUNCTION: MatchDateToken
 *
 * DESCRIPTION: Extract date from the given string
 *
 * PARAMETERS:
 *		tokenP	-	string ptr from which to extract
 *		dateP		-	ptr where to store date (optional)
 *
 * RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			djk		2/2/98	Copied code from Date Book
 *
 *************************************************************/
static void MatchDateToken (const char* tokenP,	DateType * dateP)
{
    char		identifier[identifierLengthMax];
    int			nv;

    // Use identifier[] as a temp buffer to copy parts of the vCal DateTime
    // so we can convert them to correct form.  This date portion
    // is 4 chars (date) + 2 chars (month) + 2 chars (day) = 8 chars long

    // Read the Year
    StrNCopy(identifier, tokenP, 4);
    identifier[4] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < firstYear || lastYear < nv)
        nv = firstYear;
    dateP->year = nv - firstYear;
    tokenP += strlen(identifier) * sizeof(char);

    // Read the Month
    StrNCopy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || monthsInYear < nv)
        nv = 1;
    dateP->month = nv;
    tokenP += strlen(identifier) * sizeof(char);

    // Read the Day
    StrNCopy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || 31 < nv)
        nv = 1;
    dateP->day = nv;
    tokenP += strlen(identifier) * sizeof(char);
}


/***********************************************************************
 *
 * FUNCTION:    SetDescriptionAndFilename
 *
 * DESCRIPTION: Derive and allocate a decription and filename from some text.
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
static void SetDescriptionAndFilename(char * textP, char **descriptionPP,
                                      MemHandle *descriptionHP, char **filenamePP, MemHandle *filenameHP,
                                      const char * const prefix)
{
    char * descriptionP = NULL ;
    size_t descriptionSize = 0;
    Coord descriptionWidth = 0;
    Boolean descriptionFit = false;
    char * spaceP = NULL ;
    char * filenameP = NULL ;
	MemHandle resourceH;
	char * resourceP = NULL ;
    uint8_t filenameLength = 0;
	uint8_t schemeLength = 0;
    Coord unused;


	if (textP)
	{
		descriptionSize = strlen(textP);
		WinGetDisplayExtent(&descriptionWidth, &unused);
		FntCharsInWidth (textP, &descriptionWidth, &descriptionSize, &descriptionFit);
	}
    if (descriptionSize > 0)
    {
        *descriptionHP = MemHandleNew((uint32_t)(descriptionSize + sizeOf7BitChar('\0')));
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
        DmReleaseResource(*descriptionHP);
        *descriptionHP = NULL;			// so the resource isn't freed
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
        *filenameHP = MemHandleNew((uint32_t)(schemeLength + filenameLength + strlen(todoSuffix) + sizeOf7BitChar('\0')));
        filenameP = MemHandleLock(*filenameHP);
        if (filenameP)
        {
			strcpy(filenameP, prefix);
            memmove(&filenameP[schemeLength], descriptionP, filenameLength);
            memmove(&filenameP[schemeLength + filenameLength], todoSuffix,
                    (int32_t)(strlen(todoSuffix) + sizeOf7BitChar('\0')));
        }
    }
    else
    {
		resourceH = DmGetResource(gApplicationDbP, strRsc, BeamFilenameStr);
		resourceP = MemHandleLock(resourceH);

		// Allocate space and form the filename
		filenameLength = (uint8_t)strlen(resourceP);
		schemeLength = (uint8_t)strlen(prefix);
		*filenameHP = MemHandleNew((uint32_t)(schemeLength + filenameLength + sizeOf7BitChar('\0')));
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
 * FUNCTION:    PrvTransfertGetPdiParameters
 *
 * DESCRIPTION: Get the localized PDI parameter flags.
 *
 * PARAMETERS:	None.
 *
 * RETURNED:    PDI parameters flags
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * LFe		06/08/04	Initial Revision
 *
 ***********************************************************************/
static uint16_t PrvTransfertGetPdiParameters(void)
{
	uint16_t	param16;

	if (!(param16 = (uint16_t) (ResLoadConstant(gApplicationDbP, TransfertPDIParameterID) & 0x0000FFFF)))
		param16 = kPdiEnableQuotedPrintable | kPdiBypassLocaleCharEncoding;

	return param16;
}

/***********************************************************************
 *
 * FUNCTION:    ToDoSendRecordTryCatch
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
static status_t ToDoSendRecordTryCatch (DmOpenRef dbP, uint32_t recordNum,
      ToDoItemType * recordP, UDAWriterType* media)
{
    status_t error = 0;
    PdiWriterType* writer;

    error = PrvPdiLibLoad();
    if (error)
        return error;

    writer = PdiWriterNew(media, PrvTransfertGetPdiParameters());
    if (writer)
    {

		error = PdiWriteBeginObject(writer, kPdiPRN_BEGIN_VCALENDAR);
		if (error < errNone) goto doneWriting;
		error = PdiWriteProperty(writer, kPdiPRN_VERSION);
		if (error < errNone) goto doneWriting;
		error = PdiWritePropertyValue(writer, (char*)"1.0", kPdiWriteData);
		if (error < errNone) goto doneWriting;

		error = PdiWriteProperty(writer, kPdiPRN_X_PALM);
		if (error < errNone) goto doneWriting;
		error = PdiWritePropertyValue(writer, (char*) kVObjectVersion, kPdiWriteData);
		if (error < errNone) goto doneWriting;

        ToDoExportVCal(dbP, gListViewCursorID, recordNum, recordP, writer, true, &error);
		if (error < errNone) goto doneWriting;
		PdiWriteEndObject(writer, kPdiPRN_END_VCALENDAR);
		error = UDAWriterFlush(media);

doneWriting:
        PdiWriterDelete(&writer);
    }
    else
    {
    	error = exgMemError;
    }

	PrvPdiLibUnload();

    return error;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoSendRecord
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	 dbP - pointer to the database
 * 				 recordNum - the record to send
 *				 prefix - the scheme with ":" suffix and optional "?" prefix
 *				 noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  5/9/97    Initial Revision
 *
 ***********************************************************************/
extern void ToDoSendRecord (DmOpenRef dbP, uint32_t recordNum, const char * const prefix, uint16_t noDataAlertID)
{
    MemHandle descriptionH = NULL;
    status_t error;
    ExgSocketType exgSocket;
    MemHandle nameH = NULL;
    Boolean empty;
    UDAWriterType* media;
    ToDoItemType record;
	DbSchemaColumnValueType * colVals = NULL ;
	uint32_t uniqueID;

    // important to init structure to zeros...
    MemSet(&exgSocket, sizeof(exgSocket), 0);

    // Form a description of what's being sent.  This will be displayed
    // by the system send dialog on the sending and receiving devices.

	DbCursorGetRowIDForPosition(gListViewCursorID, recordNum, &uniqueID);
	DbGetColumnValues(dbP, uniqueID, gNCols, (uint32_t*)gColIDs, &colVals);
	ToDoColumnValuesToItem(colVals, &record, gNCols);

    // If the description field is empty and the note field is empty,
    // consider the record empty.
	empty = itemIsEmpty(record);

    if (!empty)
    {
        // Set the exg description to the record's description.
        SetDescriptionAndFilename(record.description, &exgSocket.description,
                                  &descriptionH, &exgSocket.name, &nameH, prefix);

		//ABa: remove superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);
		TraceOutput(TL(appErrorClass, "ToDoSendRecord: description = %s, name = %s", exgSocket.description, exgSocket.name));

        exgSocket.length = 120 ;		// rough guess
		if (itemHasDescription(record))
			exgSocket.length += strlen(record.description) ;
		if (itemHasNote(record))
			exgSocket.length += strlen(record.note) ;

        // Note, a change in exgmgr will remap this to datebook

        // NOTE: ABa To test that the DateBook sublaunches the ToDo app
        // when parsing VTODO, just comment out the next line
        // And do the same in ToDoSendCategory
        exgSocket.target = sysFileCToDo68K;	// include target since todo and date book share an extension
        exgSocket.type = (char *)todoMIMEType;

        error = ExgPut(&exgSocket);   // put data to destination
		// ABa: Changes to use new streaming mechanism
        media = UDAExchangeWriterNew(&exgSocket,  512);
        if (!error)
        {
        	if (media)
	            error = ToDoSendRecordTryCatch(dbP, recordNum, &record, media);
	        else
	        	error = exgMemError;

            ExgDisconnect(&exgSocket, error);
        }

        if (media)
	        UDADelete(media);

	    // Clean up
	    if (descriptionH)
	        MemHandleFree(descriptionH);
	    if (nameH)
	        MemHandleFree(nameH);
   }
    else
        FrmUIAlert( noDataAlertID);

	DbReleaseStorage(dbP, colVals);
	colVals = NULL;

    return;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoSendCategoryTryCatch
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:		dbP - pointer to the database to add the record to
 * 					categoryNum - the category of records to send
 * 					exgSocketP - the exchange socket used to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/11/97  Initial Revision
 *
 ***********************************************************************/
static status_t ToDoSendCategoryTryCatch (DmOpenRef dbP, CategoryID categoryNum,
    UDAWriterType*  media)
{

    status_t error = 0;
    ToDoItemType outRecord;
	DbSchemaColumnValueType * colVals = NULL ;
	uint32_t recordNum = 1 ;	// Start from first record in category
	uint32_t cursorID = dbInvalidCursorID;

    PdiWriterType* writer;

    error = PrvPdiLibLoad();
    if (error)
        return error;

    writer = PdiWriterNew(media, PrvTransfertGetPdiParameters());
    if (writer)
    {

		error = PdiWriteBeginObject(writer, kPdiPRN_BEGIN_VCALENDAR);
		if (error < errNone) goto doneWriting;
	    error = PdiWriteProperty(writer, kPdiPRN_VERSION);
		if (error < errNone) goto doneWriting;
	    error = PdiWritePropertyValue(writer, (char*)"1.0", kPdiWriteData);
		if (error < errNone) goto doneWriting;

        // Loop through all records in the category.

		ToDoOpenCursor(dbP, &cursorID, &categoryNum, 1, priorityDueDateItem) ;

		for(DbCursorMoveFirst(cursorID); !DbCursorIsEOF(cursorID); DbCursorMoveNext(cursorID))
        {
			colVals = NULL ;
            // Emit the record.  If the record is private do not emit it.
			error = DbGetColumnValues(dbP, cursorID, gNCols, (uint32_t*)gColIDs, &colVals);

            if (!dbGetDataFailed(error))
            {
				ToDoColumnValuesToItem(colVals, &outRecord, gNCols);
                ToDoExportVCal(dbP, cursorID, recordNum, &outRecord, writer, true, &error);
				DbReleaseStorage(dbP, colVals);
				colVals = NULL ;
				if (error < errNone) goto doneWriting;
            }

            recordNum++;
        }
	    PdiWriteEndObject(writer, kPdiPRN_END_VCALENDAR);
		error = UDAWriterFlush(media);

doneWriting:
		if (colVals)
			DbReleaseStorage(dbP, colVals);

		ToDoCloseCursor(&cursorID);
        PdiWriterDelete(&writer);
    }
    else
    {
    	error = exgMemError;
    }
	PrvPdiLibUnload();


    return error;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoSendCategory
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:  dbP - pointer to the database
 * 				categoryNum - the category of records to send
 *				prefix - the scheme with ":" suffix and optional "?" prefix
 *				noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if any records are found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *
 ***********************************************************************/
extern void ToDoSendCategory (DmOpenRef dbP, CategoryID categoryNum, const char * const prefix, uint16_t noDataAlertID)
{
    status_t error;
    static char description[maxCategoryNameLength];
    Boolean foundAtLeastOneRecord;
    ExgSocketType exgSocket;
    uint16_t mode;
    DatabaseID dbID = 0;
	uint32_t cursorID = dbInvalidCursorID;

    Boolean databaseReopened;
    UDAWriterType* media;

    // If the database was opened to show secret records, reopen it to not see
    // secret records.  The idea is that secret records are not sent when a
    // category is sent.  They must be explicitly sent one by one.
	DmGetOpenInfo(dbP, &dbID, NULL, &mode, NULL) ;

    if (mode & dmModeShowSecret)
    {
		dbP = DbOpenDatabase(dbID, dmModeReadOnly, dbShareReadWrite);
        databaseReopened = true;
    }
    else
        databaseReopened = false;


    // important to init structure to zeros...
    MemSet(&exgSocket, sizeof(exgSocket), 0);
    MemSet(description, catCategoryNameLength * sizeof(char), 0);


    // Make sure there is at least one record in the category.
	ToDoOpenCursor(dbP, &cursorID, &categoryNum, 1, priorityDueDateItem);
    foundAtLeastOneRecord = (DbCursorMoveFirst(cursorID) >= errNone) ;
	ToDoCloseCursor(&cursorID);

	// We should send the category because there's at least one record to send.
    if (foundAtLeastOneRecord)
    {
        // Form a description of what's being sent.  This will be displayed
        // by the system send dialog on the sending and receiving devices.
        CatMgrTruncateName (dbP, categoryNum ? &categoryNum : NULL, categoryNum ? 1 : 0, truncCategoryMaxWidth, description);
        exgSocket.description = description;

        // Now form a file name
        exgSocket.name = MemPtrNew((uint32_t)(strlen(prefix) + strlen(description) + strlen(todoSuffix) + sizeOf7BitChar('\0')));
        if (exgSocket.name)
        {
			strcpy(exgSocket.name, prefix);
            strcat(exgSocket.name, description);
            strcat(exgSocket.name, todoSuffix);
        }


		//ABa: remove superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);

        exgSocket.length = 0;		// rough guess
        // Note, a change in exgmgr will remap this to datebook
        // NOTE: ABa To test that the DateBook sublaunches the ToDo app
        // when parsing VTODO, just comment out the next line
        // And do the same in ToDoSendRecord
        exgSocket.target = sysFileCToDo68K;	// include target since todo and date book share an extension
        exgSocket.type = (char *)todoMIMEType;

        error = ExgPut(&exgSocket);   // put data to destination

        media = UDAExchangeWriterNew(&exgSocket, 512);
        if (!error)
        {
        	if (media)
	            error = ToDoSendCategoryTryCatch (dbP, categoryNum, media);
	        else
	        	error = exgMemError;
            ExgDisconnect(&exgSocket, error);
        }

        // Release file name
        if (exgSocket.name)
            MemPtrFree(exgSocket.name);

		if (media)
	        UDADelete(media);
    }
    else
        FrmUIAlert(noDataAlertID);

    if (databaseReopened)
        DbCloseDatabase(dbP);

    return;
}


/***********************************************************************
 *
 * FUNCTION:		ToDoSetGoToParams
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
 *			ABa	08/18/00 	make it public for use in ToDo.c
 *
 ***********************************************************************/
extern void ToDoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, uint32_t uniqueID)
{
    status_t		err;
    DatabaseID		dbID;
	CategoryID 		dbSelectAllCategoriesID[] = {catIDAll};
	Boolean			recordIsInCategory = false;

    if (uniqueID == dbInvalidRowID)
    	return;

	DmGetOpenInfo(dbP, &dbID, NULL, NULL, NULL);

    // If we already have a record then make sure the record index
    // is still correct. Don't update the index if the record is not
    // in your the app's database.
    if ((exgSocketP->goToParams.uniqueID != dbInvalidRowID) && (dbID == exgSocketP->goToParams.dbID))
    {
		err = DbIsRowInCategory(dbP, exgSocketP->goToParams.uniqueID, 1, dbSelectAllCategoriesID,
			DbMatchAny, &recordIsInCategory);

		// If the record can't be find, assign the new one later below
		if ((err < errNone) || (!recordIsInCategory))
			exgSocketP->goToParams.uniqueID = dbInvalidRowID;
	}

    // Save the information necessary to navigate to the record.
    if (exgSocketP->goToParams.uniqueID == dbInvalidRowID)
    {
        exgSocketP->goToCreator = sysFileCToDo;
        exgSocketP->goToParams.uniqueID = uniqueID;
        exgSocketP->goToParams.dbID =  dbID;
        exgSocketP->goToParams.recordNum = dmInvalidRecIndex;
    }
}


/***********************************************************************
 *
 * FUNCTION:		ReceiveData
 *
 * DESCRIPTION:	Receives data into the output field using the Exg API
 *
 * PARAMETERS:		exgSocketP, socket from the app code
 *						 sysAppLaunchCmdExgReceiveData
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			???	???		Created
 *			art	10/17/97	Added "go to" record logic
 *			bhall	09/27/99	Now always read until no more data
 *								(previously datebook called us once per event)
 *
 ***********************************************************************/
extern status_t ToDoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP)
{
    status_t err = errNone;
    PdiReaderType* reader = NULL ;
    UDAReaderType* stream = NULL ;
    Boolean pdiLibLoaded = false;

    // accept will open a progress dialog and wait for your receive commands
    if ((err = ExgAccept(exgSocketP)) < errNone)
    	return err;

    if ((err = PrvPdiLibLoad()) < errNone)
	{
		goto errorDisconnect;
	}
	pdiLibLoaded = true;

    if ((stream = UDAExchangeReaderNew(exgSocketP)) == NULL)
	{
		err = exgMemError;
		goto errorDisconnect;
	}

    if ((reader = PdiReaderNew(stream, kPdiOpenParser)) == NULL)
	{
		err = exgMemError;
		goto errorDisconnect;
	}

    reader->appData = exgSocketP;

    // Keep importing records until it can't
    while(ToDoImportVCal(dbP, reader, &err, NULL))
	{
		if (err < errNone)
			goto errorDisconnect ;
	}

	// Aba: A record has been added in the Database iff the GoTo
	// uniqueID parameter != 0.
	// In the case no record is added, return an error
	if (err >= errNone && exgSocketP->goToParams.uniqueID == 0)
		err = exgErrBadData;

errorDisconnect:
	if (reader)
		PdiReaderDelete(&reader);

	if (stream)
		UDADelete(stream);

	if (pdiLibLoaded)
		PrvPdiLibUnload();

	ExgDisconnect(exgSocketP, err); // closes transfer dialog
	err = errNone;	// error was reported, so don't return it

    return err;
}


/************************************************************
 *
 * FUNCTION: ToDoImportVToDo
 *
 * DESCRIPTION: Import a VCal record of type vToDo
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			beginAlreadyRead - whether the begin statement has been read
 *			uniqueIDP - (returned) id of record inserted.
 *
 * RETURNS: true if the input was read
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			djk	8/9/97	Created
 *			art	10/17/97	Added parameter to return unique id
 *
 *************************************************************/

extern Boolean ToDoImportVToDo(DmOpenRef dbP, PdiReaderType* reader,
   uint32_t * uniqueIDP, status_t * error)
{
    uint32_t 	indexNew;
    uint32_t 	uid = 0;
    ToDoItemType newToDo;
    int   		nv;
    char * 		fieldP = NULL;
    char 		categoryName[dmCategoryLength];
    uint32_t	categoryID = catIDUnfiled;
    Boolean 	oneProperty = false;	// Aba: true iff one property was found in the note
	char *		summary = NULL;
	uint32_t	numColumns, i, colID, colIndex;
	DbSchemaColumnDefnType * colDefs;
	DbSchemaColumnValueType * colVals = NULL;
	DbSchemaColumnType colType;
	char * tmpStr = NULL;


    *error = errNone;
    categoryName[0] = 0;

	TraceOutput(TL(appErrorClass, "ToDoImportVToDo: BEGIN"));

	DbGetAllColumnDefinitions(dbP, kToDoDBSchemaName, &numColumns, &colDefs);
	if (numColumns > gNCols)
	{
		colVals = MemPtrNew(numColumns * sizeof(DbSchemaColumnValueType));
		MemSet(colVals, numColumns * sizeof(DbSchemaColumnValueType), 0);
		for (i=0; i<numColumns; i++)
			colVals[i].columnID = colDefs[i].id;
	}

	// Enter the object
   	PdiEnterObject(reader);

    // Initialize the record to default values
    *((uint16_t *) &newToDo.dueDate) = toDoNoDueDate;
    newToDo.priority = defaultPriority;
	newToDo.completed = false;
    newToDo.description = NULL;
    newToDo.note = NULL;

	while (PdiReadPropertyName(reader) == 0 && (reader->property) != kPdiPRN_END_VTODO)
	{
		TraceOutput(TL(appErrorClass, "ToDoImportVToDo: property = %s", reader->propertyName));
		colID = colIndex = 0xFFFFFFFF;
		colType = 0xFF;
		if (reader->property == kPdiPRN_X_PALM_UKNCOL && numColumns > gNCols)
		{
			while (PdiReadParameter(reader) >= errNone)
			{
				switch (reader->parameter)
				{
					case kPdiPAN_X_PALM_COLID:
						colID = StrAToI(reader->parameterValue);
					break;
					case kPdiPAN_X_PALM_COLTYPE:
						colType = (DbSchemaColumnType) StrAToI(reader->parameterValue);
					break;
				}
			}

			for (i=0; i<numColumns; i++)
			{
				if (colID == colDefs[i].id && colType == colDefs[i].type)
				{
					oneProperty = true;
					colIndex = i;
					break;
				}
			}
			if (colIndex != 0xFFFFFFFF)
			{
				PdiReadPropertyField(reader, (char**) &(colVals[colIndex].data), kPdiResizableBuffer, kPdiDefaultFields);
				if (((colType != dbVarChar) && (colType != dbChar)) || (colType & dbVector))
					colVals[colIndex].dataSize = reader->written-1;
				else
					colVals[colIndex].dataSize = reader->written;
			}
		}

		switch(reader->property)
		{
	        // Handle Priority tag
			case kPdiPRN_PRIORITY:
	            *error = PdiReadPropertyField(reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            if (fieldP != NULL)
	            {
	                nv = StrAToI(fieldP);
	                nv = min(nv, toDoMaxPriority);
	                newToDo.priority = nv ;
	            }
	            oneProperty = true;
				break;

	        // Handle the due date.
	        case kPdiPRN_DUE:
	            *error = PdiReadPropertyField(reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            if (fieldP != NULL)
	            {
	                // Extract due date
	                MatchDateToken(fieldP, (DateType*)&newToDo.dueDate);
	            }
	            oneProperty = true;
	            break;


	        // Handle the two cases that indicate completed
	        //the Status:completed property
	        case kPdiPRN_STATUS:
	            *error = PdiReadPropertyField(reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            if (fieldP != NULL)
	            {
	                if (StrCaselessCompare(fieldP, "COMPLETED") == 0)
	                {
						newToDo.completed = true ;
	                }
	            }
	            oneProperty = true;
	            break;

	        // and the date/time completed property
	        case kPdiPRN_COMPLETED:
	            newToDo.completed = true ;
	            oneProperty = true;
	            break;

	        // Handle the description
	        case kPdiPRN_DESCRIPTION:
	            *error = PdiReadPropertyField(reader, (char**) &newToDo.description, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            oneProperty = true;
	        	break;

	        // Handle the description
	        case kPdiPRN_SUMMARY:
	            *error = PdiReadPropertyField(reader, (char**) &summary, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            oneProperty = true;
	        	break;

	        // Treat attachments as notes
	        case kPdiPRN_ATTACH:
	            // Note: vCal permits attachments of types other than text, specifically
	            // URLs and Content ID's.  At the moment, wee will just treat both of these
	            // as text strings
				PdiDefineResizing(reader, kPdiDefaultBufferDeltaSize, noteViewMaxLength);
	            *error = PdiReadPropertyField(reader, (char**) &newToDo.note, kPdiResizableBuffer, kPdiDefaultFields);
				PdiDefineResizing(reader, kPdiDefaultBufferDeltaSize, kPdiDefaultBufferMaxSize);
				if (*error < errNone) goto readDone;
	            oneProperty = true;
	            break;

	        // read in the category
	        case kPdiPRN_CATEGORIES:
	            *error = PdiReadPropertyField(reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            if (fieldP != NULL)
	            {
	                char *spot = fieldP;

	                // If the category was not a predefined vCal catval,
	                // we need to skip the leading special category name mark ("X-")
	            	if (spot[0] == 'X' && spot[1] == '-')
	            	{
	            		spot += 2;
	            	}

	                // Make a copy, leaving room for the terminator, cropping if necessary.
	                StrNCopy(categoryName, spot, dmCategoryLength - 1);

	                // Make sure it is null terminated.
	                categoryName[dmCategoryLength - 1] = 0;
	            }
	            oneProperty = true;
	            break;

	        // read in the unique identifier
	        case kPdiPRN_UID:
	            *error = PdiReadPropertyField(reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
				if (*error < errNone) goto readDone;
	            if (fieldP != NULL)
	            {
	            	uid = StrAToI(fieldP);
	                // Check the uid for reasonableness.
	                if (uid < (dmRecordIDReservedRange << 12))
	                    uid = 0;
	            }
	            oneProperty = true;
	            break;
		}

		if (fieldP != NULL)
		{
			MemPtrFree(fieldP);
			fieldP = NULL;
		}

    }

    // Non syntax error but empty object: don't create it
    if (oneProperty == false)
	{
		*error = errNone;
		goto readDone;
	}

	// If summary was set but not a description, use the summary instead of the
	// description.
	if ((newToDo.description == NULL) && (summary != NULL))
	{
		newToDo.description = summary;
		summary = NULL;
	}

    // Set the category for the record (we saved category ID in appData field)
	// This record may have been imported, in this case reader->appData is NULL and
	// thus does not point on an exchange socket.
    if ((reader->appData != NULL) && ((ExgSocketPtr)(reader->appData))->appData)
    {
        categoryID = ((ExgSocketPtr)(reader->appData))->appData;
    }
    // we really need to recognize the vCal category specified, our category picker needs to somehow
    // know the default category. Without that support, we need to always put things into unfiled by
    // default because that is what we show the user. This logic really needs to run before the ask dialog
    // comes up, but we have not yet parsed the data then... Hmmmm

	if (*uniqueIDP != dmUnusedRecordID
	&& DbCursorMoveToRowID(gListViewCursorID, *uniqueIDP) >= errNone )
	{
		DbSchemaColumnValueType localColVals[gNCols];
		uint32_t nCols;
		// Update an existing record
		TraceOutput(TL(appErrorClass, "Update record %d",*uniqueIDP));
		ToDoItemToColumnValues(localColVals, (ToDoItemType*)&newToDo, &nCols);
		*error = DbWriteColumnValues(dbP, gListViewCursorID, nCols, localColVals);
		ToDoDbCursorRequery(&gListViewCursorID);
	}
	else
	{
		// Write the actual record
		TraceOutput(TL(appErrorClass, "Add the record"));
		*error = ToDoNewRecord(dbP, &gListViewCursorID, (ToDoItemType*)&newToDo, &indexNew, categoryID) ;

		// Return the unique id of the record inserted.
		DbCursorGetRowIDForPosition(gListViewCursorID, indexNew, uniqueIDP) ;
	}

	if (colVals)
	{
		for (i=0; i<numColumns; i++)
		{
			if (colVals[i].data && ! knownColID(colVals[i].columnID))
			{
				TraceOutput(TL(appErrorClass, "Add unknown col ID %lu", colVals[i].columnID));
				*error = DbWriteColumnValue(dbP, gListViewCursorID, colVals[i].columnID, 0, -1, colVals[i].data, colVals[i].dataSize);
				MemPtrFree(colVals[i].data);
			}
		}
	}


readDone:
    // Free any temporary buffers used to store the incoming data.
	DbReleaseStorage(dbP, colDefs);
	if (colVals) MemPtrFree(colVals);
    if (newToDo.note) MemPtrFree(newToDo.note);
    if (newToDo.description) MemPtrFree(newToDo.description);
    if (summary) MemPtrFree(summary);

    return ((reader->events & kPdiEOFEventMask) == 0);
}


/************************************************************
 *
 * FUNCTION: ToDoImportVCal
 *
 * DESCRIPTION: Import a VCal record of type vEvent and vToDo
 *
 * The Datebook handles vCalendar records.  Any vToDo records
 * are sent to the ToDo app for importing.
 *
 * This routine doesn't exist for the device since the Datebook
 * is sent vCal data.  The Datebook will To Do any vToDo records
 * via an action code.  This routine is only for the simulator
 * because the Datebook can't run at the same time.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			beginAlreadyRead - whether the begin statement has been read
 *			vToDoFunc - function to import vToDo records
 *						on the device this is a function to call ToDo to read
 *						for the shell command this is an empty function
 *
 * RETURNS: true if the input was read
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			djk	8/9/97		Created
 *			roger	9/12/97		Modified to work on the device
 *			ABa	06/21/00		Integrate Pdi library
 *
 *************************************************************/
extern Boolean ToDoImportVCal(DmOpenRef dbP, PdiReaderType* reader, status_t * errorP, uint32_t * uniqueIDP)
{
    uint32_t uniqueID = dmUnusedRecordID;

    *errorP = errNone;

	TraceOutput(TL(appErrorClass, "ToDoImportVCal: BEGIN"));

	// Use the provided uniqueID if any
	if (uniqueIDP)
		uniqueID = *uniqueIDP ;

    // Read in the vEvent entry
	PdiReadProperty(reader);
	if (reader->property != kPdiPRN_BEGIN_VCALENDAR)
        return false;

    // Read in the vcard entry
	PdiEnterObject(reader);
    while (PdiReadProperty(reader) == 0 && reader->property != kPdiPRN_END_VCALENDAR)
    {
        // Only process vToDo objects, skipp others
        if (reader->property == kPdiPRN_BEGIN_VTODO)
        {
            ToDoImportVToDo(dbP, reader, &uniqueID, errorP);
			// If the record was imported with sysAppLaunchCmdImportRecord cmd,
			// there is no corresponding exchange socket and reader->appData is NULL.
			if (reader->appData != NULL)
				ToDoSetGoToParams (dbP, reader->appData, uniqueID);

			// Save last uniqueID if needed
			if (uniqueIDP)
				*uniqueIDP = uniqueID;

			// Reset uniqueID so the next vToDo will not override this one
			uniqueID = dmUnusedRecordID;
        }
    }

    return ((reader->events & kPdiEOFEventMask) == 0);
}


/************************************************************
 *
 * FUNCTION: ToDoExportVCal
 *
 * DESCRIPTION: Export a VCALENDAR record.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			beginAlreadyRead - whether the begin statement has been read
 *
 * RETURNS: true if the input was read
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			djk	8/9/97	Created
 *
 *************************************************************/

extern void ToDoExportVCal(DmOpenRef dbP, uint32_t cursorID, uint32_t index, ToDoItemType *  recordP,
	PdiWriterType* writer, Boolean writeUniqueIDs, status_t * error)
{
    uint32_t		uid;
    static char		tempString[tempStringLengthMax];
	CategoryID * categoryIDsP;
	uint32_t		numCategories, numColumns, i;
	DbSchemaColumnValueType*	colVals;
	DbSchemaColumnDefnType*		colDefs;

    PdiWriteBeginObject(writer, kPdiPRN_BEGIN_VTODO);

    // Emit the Category
    PdiWriteProperty(writer, kPdiPRN_CATEGORIES);
	DbCursorSetAbsolutePosition(cursorID, index);
	DbCursorGetCurrentRowID(cursorID, &uid);
	DbGetCategory (dbP, cursorID, &numCategories, &categoryIDsP);
    CatMgrTruncateName (dbP, numCategories ? categoryIDsP : NULL, numCategories, truncCategoryMaxWidth, tempString + 2);

    // Check to see if the category is a predefined vCal catval
    if((StrCaselessCompare(tempString + 2, "Personal") != 0) &&
       (StrCaselessCompare(tempString + 2, "Business") != 0))
    {
    	tempString[0] = 'X';
    	tempString[1] = '-';
    	PdiWritePropertyValue(writer, tempString, kPdiWriteText);
    }
	else
	{
    	PdiWritePropertyValue(writer, tempString + 2, kPdiWriteText);
	}

    // Emit the Due date
    if (DateToInt(recordP->dueDate) != toDoNoDueDate)
    {
	    PdiWriteProperty(writer, kPdiPRN_DUE);
        // NOTE: since we don't keep a time for ToDo due dates,
        // we will truncate the ISO 8601 date/time to an ISO 8601 date
        // as allowed by the standard
        sprintf(tempString, "%d%02d%02d", firstYear + recordP->dueDate.year,
                  recordP->dueDate.month, recordP->dueDate.day);
    	PdiWritePropertyValue(writer, tempString, kPdiWriteData);
    }

    // Emit the completed flag
    PdiWriteProperty(writer, kPdiPRN_STATUS);
	if (recordP->completed)
	{
    	PdiWritePropertyValue(writer, (char*) "COMPLETED", kPdiWriteData);
    }
    else
    {
    	PdiWritePropertyValue(writer, (char*) "NEEDS ACTION", kPdiWriteData);
    }

    // Emit the Priority Level
    if (recordP->priority != 0)
    {
	    PdiWriteProperty(writer, kPdiPRN_PRIORITY);
        sprintf(tempString, "%d", recordP->priority);
    	PdiWritePropertyValue(writer, tempString, kPdiWriteData);
    }

    // Emit the Decsription Text
    if(recordP->description != '\0')
    {
	    PdiWriteProperty(writer, kPdiPRN_DESCRIPTION);
    	PdiWritePropertyValue(writer, recordP->description, kPdiWriteText);
    }

    // Emit the note
    if(itemHasNote(*recordP))
    {
	    PdiWriteProperty(writer, kPdiPRN_ATTACH);
		PdiWritePropertyValue(writer, recordP->note, kPdiWriteText);
    }

    // Emit an unique id
    if (writeUniqueIDs)
    {
	    PdiWriteProperty(writer, kPdiPRN_UID);
        // Get the record's unique id and append to the string.
        StrIToA(tempString, uid);
    	PdiWritePropertyValue(writer, tempString, kPdiWriteData);
    }

	// Are there other (unknown) columns to emit ?
	DbGetAllColumnValues(dbP, uid, &numColumns, &colVals);
	for (i=0; i<numColumns; i++)
	{
		if (!knownColID(colVals[i].columnID))
		{
			TraceOutput(TL(appErrorClass, "ToDoExportVCal index %lu : col %lu unknown", index, colVals[i].columnID));

			DbGetColumnDefinitions(dbP, kToDoDBSchemaName, 1, &(colVals[i].columnID), &colDefs);
			PdiWriteProperty(writer, kPdiPRN_X_PALM_UKNCOL);
			// Send unknown column with the unknown tag
			StrIToA(tempString, colVals[i].columnID);
			PdiWriteParameterStr(writer , "X-PALM-COLID", tempString);
			StrIToA(tempString, (int32_t)colDefs->type);
			PdiWriteParameterStr(writer , "X-PALM-COLTYPE", tempString);
			if (((colDefs->type == dbVarChar) || (colDefs->type == dbChar)) && !(colDefs->type & dbVector))
				PdiWritePropertyValue(writer, (char*) colVals[i].data, kPdiWriteText);
			else
			{
				uint16_t encodingBak = writer->encoding;
				PdiSetEncoding(writer, kPdiB64Encoding);
				PdiWritePropertyBinaryValue(writer, colVals[i].data, (uint16_t)colVals[i].dataSize, kPdiWriteData);
				PdiSetEncoding(writer, encodingBak);
			}
			DbReleaseStorage(dbP, colDefs);
		}
	}
	DbReleaseStorage(dbP, colVals);

    PdiWriteEndObject(writer, kPdiPRN_END_VTODO);

	*error = (writer->errorLowWord == errNone) ? errNone : ((status_t) writer->errorLowWord | pdiErrorClass);
}


/***********************************************************************
 *
 * FUNCTION:		ToDoImportData
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
status_t ToDoImportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t err;
	PdiReaderType* reader = NULL;
	UDAReaderType* stream = NULL;
	Boolean	pdiLibLoaded = false;

	err = PrvPdiLibLoad();
	if (err < errNone)
		goto Exit;
	pdiLibLoaded = true;

	if (impExpObjP->vObjectH == NULL)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	// Create an UDA reader
	if ((stream = UDAMemHandleReaderNew(impExpObjP->vObjectH)) == NULL)
	{
		err = memErrNotEnoughSpace;
		goto Exit;
	}

	reader = PdiReaderNew(stream, kPdiOpenParser);
	if (! ToDoImportVCal(dbP, reader, &err, &(impExpObjP->uniqueID)))
		err = exgErrBadData;
	PdiReaderDelete(&reader);

	// return the index and id of the last memo created
	if (err >= errNone)
	{
		DbCursorGetPositionForRowID(gListViewCursorID, impExpObjP->uniqueID, &(impExpObjP->index));
		(impExpObjP->index)--;
	}


Exit:
	if (stream)
		UDADelete(stream);

	if (pdiLibLoaded)
		PrvPdiLibUnload();

	return err;
}

/***********************************************************************
 *
 * FUNCTION:		ToDoExportData
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
status_t ToDoExportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t err;
	UDAWriterType* stream = NULL;
    ToDoItemType record;
	DbSchemaColumnValueType * colVals = NULL ;
	uint32_t nRecs, uniqueID;
	CategoryID catID = catIDAll;

	if (impExpObjP->vObjectH == NULL)
	{
		err = dmErrInvalidParam;
		goto Exit;
	}

	// indexes are based on All categories
	if (CurrentCategory != catIDAll)
		ToDoOpenCursor(dbP, &gListViewCursorID, &catID, 1, priorityDueDateItem);

	nRecs = DbCursorGetRowCount(gListViewCursorID) ;

	// Check the validity of the record index
	if (impExpObjP->index == ImpExpInvalidRecIndex)
	{
		uniqueID = impExpObjP->uniqueID;
		DbCursorGetPositionForRowID(gListViewCursorID, impExpObjP->uniqueID, &impExpObjP->index);
		impExpObjP->index--;
	}
	else
	{
		if (impExpObjP->index >= nRecs)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
		else
		{
			DbCursorGetRowIDForPosition(gListViewCursorID, impExpObjP->index + 1, &uniqueID);
			impExpObjP->uniqueID = uniqueID;
		}
	}

	// Create an PDI writer
	stream = UDAMemHandleWriterNew(impExpObjP->vObjectH,  ALLOCATE_VOBJECT_INC_SIZE);
	DbCursorSetAbsolutePosition(gListViewCursorID, impExpObjP->index + 1);
	DbCursorGetCurrentRowID(gListViewCursorID, &uniqueID);
	err = DbGetColumnValues(dbP, gListViewCursorID, gNCols, (uint32_t*)gColIDs, &colVals);
    if (!dbGetDataFailed(err))
    {
		ToDoColumnValuesToItem(colVals, &record, gNCols);
		err = ToDoSendRecordTryCatch(dbP, impExpObjP->index + 1, &record, stream);
		DbReleaseStorage(dbP, colVals);
    }

	UDAWriterFlush(stream);

Exit:
	// indexes are based on All categories
	if (CurrentCategory != catIDAll)
		ToDoOpenCursor(dbP, &gListViewCursorID, &CurrentCategory, 1, priorityDueDateItem);

	if (stream)
		UDADelete(stream);

	return err;
}

/***********************************************************************
 *
 * FUNCTION:		ToDoDeleteRecord
 *
 * DESCRIPTION:		Deletes a record from DB
 *
 * PARAMETERS:		dbP - pointer to the database
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
status_t ToDoDeleteRecord(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	uint32_t 	numRecords;
	status_t	err = errNone;
	uint32_t	index = impExpObjP->index + 1;

	numRecords = DbCursorGetRowCount(gListViewCursorID);

	if ((impExpObjP->index != ImpExpInvalidRecIndex) && (impExpObjP->index < numRecords))
	{
		// Check the validity of the record index
		if (DbCursorGetRowIDForPosition(gListViewCursorID, index, &impExpObjP->uniqueID) < errNone)
		{
			err = dmErrInvalidParam;
			goto Exit;
		}
	} else if (impExpObjP->uniqueID != ImpExpInvalidUniqueID)
	{
		// Get the index from the unique ID
		if (DbCursorGetPositionForRowID(gListViewCursorID, impExpObjP->uniqueID, &index) < errNone)
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
	err = DbCursorMoveToRowID(gListViewCursorID, impExpObjP->uniqueID);
	if (err >= errNone)
		err = DbDeleteRow(dbP, gListViewCursorID);

Exit:
	if (err != errNone)
	{
		impExpObjP->index = ImpExpInvalidRecIndex;
		impExpObjP->uniqueID = ImpExpInvalidUniqueID;
	}

	return err;
}


/***********************************************************************
 *
 * FUNCTION:		ToDoMoveRecord
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
status_t ToDoMoveRecord(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t	err = errNone;

	if (impExpObjP->index == ImpExpInvalidRecIndex || impExpObjP->destIndex == ImpExpInvalidRecIndex)
		return dmErrInvalidParam;

	err = DbCursorRelocateRow(gListViewCursorID, impExpObjP->index, impExpObjP->destIndex);
	if (err >= errNone)
		err = DbCursorGetRowIDForPosition(gListViewCursorID, impExpObjP->destIndex, &impExpObjP->uniqueID);

	return err;
}
