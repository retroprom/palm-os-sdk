/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressTransfer.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *      Address Book routines to transfer records.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h> // for min(a,b)
#include <DateTime.h>
#include <BuildDefaults.h>
#include <SystemResources.h>
#include <UIResources.h>
#include <ExgMgr.h>
#include <DataMgr.h>
#include <AppMgr.h>
#include <string.h>
#include <posix/limits.h>		// SIZE_MAX

#include <UDAMgr.h>
#include <TraceMgr.h>
#include <ErrorMgr.h>
#include <StringMgr.h>
#include <CatMgr.h>
#include <PhoneBookIOLib.h>
#include <ExgLocalLib.h>
#include <Table.h>
#include <TelephonyLib.h>

#include "Address.h"

#include "AddressFreeFormName.h"
#include "AddressDBSchema.h"
#include "AddressTab.h"
#include "AddressTransfer.h"
#include "AddressPrefs.h"
#include "AddressTools.h"
#include "AddressPhoneIO.h"
#include "AddressImport.h"
#include "AddressU32List.h"

#include "AddressRsc.h"

/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

#define kTmpBufSize 					128

#define kAddrFilenameExtension			"vcf"
#define kAddrMIMEType					"text/x-vCard"

// Aba: internal version of vCalendar. Must be updated
// the export side of the vCalendar code evoluate

#define kVObjectPalmOSV6Version			"6.1"
#define kVObjectvCardVersion			"2.1"

#ifndef ALLOCATE_VOBJECT_INC_SIZE
#define ALLOCATE_VOBJECT_INC_SIZE		512
#endif

#define kExgSocketDefaultLength			1024

// Column ID vCard field mappings
uint32_t const nameColIDs[nNameFields] =
{
	kAddrColumnIDLastName,
	kAddrColumnIDFirstName,
	kAddrColumnIDMiddleName,
	kAddrColumnIDTitle,
	kAddrColumnIDSuffix
};

uint32_t const addressFieldKinds[nAddressFields] =
{
	0,								// iPOBoxAddress
	0, 								// iXAddress
	kFieldKind_StreetAddress,		// iStreetAddress
	kFieldKind_CityAddress,			// iCityAddress
	kFieldKind_StateAddress,		// iStateAddress
	kFieldKind_ZipCodeAddress,		// iZipCodeAddress
	kFieldKind_CountryAddress		// iCountryAddress
};

int32_t gAddressSlotsAvailable[nFamilyNumber];
int32_t gPhoneSlotsAvailable[nPhoneNumber][nFamilyNumber];

int32_t gInternetSlotsAvailable[nFamilyNumber][nInternetNumber];

int32_t gIMSlotsAvailable[nInstantMessagingNumber][nFamilyNumber];
int32_t gCustomSlotsAvailable[nFamilyNumber];

extern void GenerateDateTimeToken(char* outputString, DateType*	dateP, TimeType* timeP);
extern void MatchDateTimeToken(const char* tokenP, DateType* dateP, TimeType* timeP, Boolean* noTimeEventP);

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
	uint16_t		param16;

	param16 = (uint16_t) (ResLoadConstant(gApplicationDbP, TransfertPDIParameterID) & 0x0000FFFF);
	if (!param16)
		param16 = kPdiEnableQuotedPrintable | kPdiBypassLocaleCharEncoding;

	return param16;
}

/***********************************************************************
 *
 * FUNCTION:    PrvTransferSendRecord
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				recordNum - the record number to send
 * 				recordP - pointer to the record to send
 * 				outputStream - place to send the data
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	12/11/97	Initial Revision
 * ABa		04/10/00	Load Pdi library
 *
 ***********************************************************************/
static status_t PrvTransferSendRecord(DmOpenRef dbP, uint32_t recordID, UDAWriterType* media, uint32_t transfertMask)
{
	status_t		error = errNone;
	PdiWriterType *	writer;

	writer = PdiWriterNew(media, PrvTransfertGetPdiParameters());

	if (writer)
	{
		error = TransferExportVCard(dbP, recordID, writer, transfertMask);
		if (error >= errNone)
			error = UDAWriterFlush(media);

		PdiWriterDelete(&writer);
	}
	else
	{
		error = exgMemError;
	}

	return error;
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
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ABa		07/28/00	Created
 *
 ***********************************************************************/

static void PrvTransferCleanFileName(char* ioFileName)
{
	char* 		mayBeLastDotP;
	char*  		lastDotP;
	uint32_t	chrFullStopSize = TxtCharSize(chrFullStop);

	// prevent NULL & empty string
	if (!ioFileName || !*ioFileName)
		return;

	// remove dot but not the last one
	mayBeLastDotP = strchr(ioFileName, 	chrFullStop);
	lastDotP = strchr(mayBeLastDotP + chrFullStopSize, chrFullStop);

	while (lastDotP)
	{
		// remove the dot
		strcpy(mayBeLastDotP, mayBeLastDotP + chrFullStopSize);
		mayBeLastDotP = lastDotP - chrFullStopSize;

		lastDotP = strchr(mayBeLastDotP + chrFullStopSize, chrFullStop);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvTransferSendCategory
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				categoryNum - the category of records to send
 * 				exgSocketP - the exchange socket used to send
 * 				index - the record number of the first record in the category to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * -----	--------	-----------
 * roger	05/09/97	Initial Revision
 * ABa		06/20/00	Integrate Pdi library
 *
 ***********************************************************************/

static status_t PrvTransferSendCategory(DmOpenRef dbP, UDAWriterType*  media, uint32_t cursorID)
{
	status_t			error = errNone;
	PdiWriterType *		writer;

	writer = PdiWriterNew(media, PrvTransfertGetPdiParameters());
	if (!writer)
		return exgMemError;

	while (!DbCursorIsEOF(cursorID))
	{
		error = TransferExportVCard(dbP, cursorID, writer, kFieldFamilyMask | kFieldType_Binary | kFieldKind_Note);
		if (error < errNone)
			break;

		DbCursorMoveNext(cursorID);
	}

	if (error >= errNone)
		error = UDAWriterFlush(media);

	return error;
}


/***********************************************************************
 *
 * FUNCTION:		TransferRegisterData
 *
 * DESCRIPTION:		Register with the exchange manager to receive data
 *					with a certain name extension.
 *
 * PARAMETERS:		appDbRef	 - AddressBook application db
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * rsf		12/02/97	Created
 * LFe		10/23/02	Use new database API. Add parameter.
 *
 ***********************************************************************/
void TransferRegisterData (DmOpenRef appDbRef)
{
	MemHandle	resH = DmGetResource(appDbRef, strRsc, ExgDescriptionStr);
	char *		descP = DmHandleLock(resH);

	ExgRegisterDatatype(sysFileCAddress, exgRegExtensionID, kAddrFilenameExtension, descP, 0);
	ExgRegisterDatatype(sysFileCAddress, exgRegTypeID, kAddrMIMEType, descP, 0);

	DmHandleUnlock(resH);
	DmReleaseResource(resH);

	// Get application (icon name)
	resH = DmGetResource(gApplicationDbP, ainRsc, ainID );
	descP = DmHandleLock(resH);

	// Register for the attach feature
	ExgRegisterDatatype(sysFileCAddress, exgRegSchemeID, exgGetScheme, descP, 0);

	DmHandleUnlock(resH);
	DmReleaseResource(resH);
}

/***********************************************************************
 *
 * FUNCTION:    TransferSendRecord
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				recordNum - the record to send
 *				prefix - the scheme with ":" suffix and optional "?" prefix
 *				noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *         dje     4/24/00  Don't specify target creator ID
 *         ABa     6/20/00  Integrate Pdi library
 *
 ***********************************************************************/
void TransferSendRecord (DmOpenRef dbP, uint32_t recordID, const char * const prefix, uint16_t noDataAlertID, uint32_t transfertMask)
{
	MemHandle		descriptionH = NULL;
	size_t			descriptionSize = 0;
	Coord			descriptionWidth = 0;
	Coord			descriptionHeight = 0;
	Boolean			descriptionFit = false;
	size_t			newDescriptionSize = 0;
	MemHandle		nameH = NULL;
	status_t		error = errNone;
	ExgSocketType	exgSocket;
	UDAWriterType*	media = NULL;
	uint8_t			schemeLength = 0;

	// Must be specified. If 0, that means an error occurs.
	if (!transfertMask)
		return;

	// important to init structure to zeros...
	memset(&exgSocket, 0, sizeof(exgSocket));

	// Don't send the record if empty
	if (!AddressDBRecordContainsData(recordID))
	{
		FrmUIAlert(noDataAlertID);
		return;
	}

	// Form a description of what's being sent.  This will be displayed
	// by the system send dialog on the sending and receiving devices.
	if ((descriptionH = AddrFreeFormNameBuild(dbP, recordID)) != NULL)
	{
		exgSocket.description = MemHandleLock(descriptionH);
		descriptionSize = strlen(exgSocket.description) + sizeOf7BitChar(chrNull);
	}

	// Truncate the description if too long
	if (descriptionSize > 0)
	{
		// Make sure the description isn't too long.
		newDescriptionSize = descriptionSize;
		WinGetDisplayExtent(&descriptionWidth, &descriptionHeight);
		FntCharsInWidth (exgSocket.description, &descriptionWidth, &newDescriptionSize, &descriptionFit);

		if (newDescriptionSize > 0)
		{
			if (newDescriptionSize != descriptionSize)
			{
				exgSocket.description[newDescriptionSize] = nullChr;
				MemHandleUnlock(descriptionH);
				MemHandleResize(descriptionH, (uint32_t)(newDescriptionSize + sizeOf7BitChar(chrNull)));
				exgSocket.description = MemHandleLock(descriptionH);
			}
		}
		else
		{
			MemHandleFree(descriptionH);
			exgSocket.description = NULL;
		}

		descriptionSize = newDescriptionSize;
	}

	// Make a filename
	schemeLength = (uint8_t)strlen(prefix);

	if (descriptionSize > 0)
	{
		size_t	size;

		// Now make a filename from the description
		size = strlen(prefix) + descriptionSize + sizeOf7BitChar(chrFullStop) + strlen(kAddrFilenameExtension) + 1;
		nameH = MemHandleNew(size);
		exgSocket.name = MemHandleLock(nameH);
		strcpy(exgSocket.name, prefix);
		strcat(exgSocket.name, exgSocket.description);
		strcat(exgSocket.name, ".");
		strcat(exgSocket.name, kAddrFilenameExtension);
	}

	//ABa: remove superfluous '.' characters
	PrvTransferCleanFileName(exgSocket.name);
	exgSocket.length = kExgSocketDefaultLength;
	exgSocket.type = (char *)kAddrMIMEType;

	error = ExgPut(&exgSocket);   // put data to destination

	TraceOutput(TL(appErrorClass, "TransferSendRecord: description = %s, name = %s", exgSocket.description, exgSocket.name));
	// ABa: Changes to use new streaming mechanism
	media = UDAExchangeWriterNew(&exgSocket,  ALLOCATE_VOBJECT_INC_SIZE);

	if (error >= errNone)
	{
		if (media)
			error = PrvTransferSendRecord(dbP, recordID, media, transfertMask);
		else
			error = exgMemError;

		ExgDisconnect(&exgSocket, error);
	}

	if (media)
		UDADelete(media);

	// Clean up
	if (descriptionH)
	{
		MemHandleUnlock (descriptionH);

		if (MemHandleDataStorage (descriptionH))
			DmReleaseResource(descriptionH);
		else
			MemHandleFree(descriptionH);
	}
	if (nameH)
	{
		MemHandleUnlock (nameH);

		if (MemHandleDataStorage (nameH))
			DmReleaseResource(nameH);
		else
			MemHandleFree(nameH);
	}
}



/***********************************************************************
 *
 * FUNCTION:    TransferSendCategory
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
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
 *         dje     4/24/00  Don't specify target creator ID
 *
 ***********************************************************************/
void TransferSendCategory(DmOpenRef dbP, uint32_t numCategories, CategoryID *categoriesP, const char * const prefix, uint16_t noDataAlertID)
{
	status_t				error;
	char					description[kCategoryLabelLength];
	ExgSocketType			exgSocket;
	UDAWriterType*			media;
	uint32_t				cursorID = dbInvalidCursorID;
	char					sql[kMaxOrderBySQLStrSize];
	DmOpenRef				addrDbP = NULL;

	// Re-open the DB if it is opened with private records visible
	if (gPrivateRecordVisualStatus != hidePrivateRecords)
	{
        error = AddressDBOpenDatabase(gApplicationDbP, dmModeReadOnly, &addrDbP);
		ErrNonFatalDisplayIf(error < errNone, "Can't open database!");
	}
	else
	{
		addrDbP = dbP;
	}

	// Create a new cursor
	sprintf(sql, "%s %s", kAddressDefaultTableName, gCurrentOrderByStr);
	error = DbCursorOpenWithCategory(addrDbP, sql, 0, numCategories, categoriesP, DbMatchAny, &cursorID);

	if (error < errNone || !DbCursorGetRowCount(cursorID))
	{
		FrmUIAlert(noDataAlertID);
		goto Exit;
	}

	// important to init structure to zeros...
	memset(&exgSocket, 0, sizeof(exgSocket));

	// Form a description of what's being sent.  This will be displayed
	// by the system send dialog on the sending and receiving devices.
	// As CatMgrGetName doesn't work for catIDAll and catIDUnfiled, use
	// CatMgrGetUnfiledName instead because it works wharever the category ID.
	CatMgrTruncateName(addrDbP, categoriesP, numCategories, kCategoryLabelLength, description);

// #### LFe.... To be removed ?
/*
	if (!categoriesP || *categoriesP == catIDAll)
	{
		uint32_t numCat = min(numCategories, 1);

		// ####TEs_25, No CatMgrGetUnfiledName (nor catIDAll)
		// Use CatMgrTruncateName instead
	}
	else
	{
		CatMgrGetName(gAddrDB, *categoriesP, description);
	}
*/
	exgSocket.description = description;

	// Now form a file name
	exgSocket.name = MemPtrNew((uint32_t)(strlen(prefix) + strlen(description) + sizeOf7BitChar(chrFullStop) + strlen(kAddrFilenameExtension) + sizeOf7BitChar(chrNull)));

	if (exgSocket.name)
	{
		strcpy(exgSocket.name, prefix);
		strcat(exgSocket.name, description);
		strcat(exgSocket.name, ".");
		strcat(exgSocket.name, kAddrFilenameExtension);
	}

	// ABa: remove superfluous '.' chars
	PrvTransferCleanFileName(exgSocket.name);
	exgSocket.length = 0;		// rough guess
	exgSocket.type = (char *)kAddrMIMEType;

	error = ExgPut(&exgSocket);   // put data to destination
	media = UDAExchangeWriterNew(&exgSocket, ALLOCATE_VOBJECT_INC_SIZE);
	if (error >= errNone)
	{
		if (media)
			error = PrvTransferSendCategory(addrDbP, media, cursorID);
		else
			error = exgMemError;

		ExgDisconnect(&exgSocket, error);
	}

	// Release file name
	if (exgSocket.name)
		MemPtrFree(exgSocket.name);

	if (media)
		UDADelete(media);

Exit:
	if (cursorID != dbInvalidCursorID)
		DbCursorClose(cursorID);

	if (gPrivateRecordVisualStatus != hidePrivateRecords)
		DbCloseDatabase(addrDbP);
}


/***********************************************************************
 *
 * FUNCTION:	TransferGetPhoneCategory
 *
 * DESCRIPTION:	Get from the connected phone its phone book and
 *				put it in the Phone Category. If this category does not
 *				exist, create it. Once completed, switch to the phone
 *				category.
 *
 * PARAMETERS:
 *
 *
 * RETURNED:	error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         PLe    04/19/00  Created
 *
 ***********************************************************************/
status_t TransferGetPhoneCategory(DmOpenRef dbP, CategoryID *phoneCategoryP)
{
	status_t 		err = errNone;
	ExgSocketType 	exgSocket;
	CategoryID *	categoriesP;
	uint32_t		numCategories;
	CategoryID		phoneCategory;
	uint16_t		len;
	uint8_t			phoneDirectory;
	char 			catName[kCategoryLabelLength];
	uint32_t		cursorID = dbInvalidCursorID;
	char			sql[kMaxOrderBySQLStrSize];

	// Initialize the socket fields
	memset(&exgSocket, 0, sizeof(ExgSocketType));
	exgSocket.name = MemPtrNew((uint32_t)(strlen(exgPhoneBookIOPrefix) + 1));
	strcpy(exgSocket.name, exgPhoneBookIOPrefix);

	// Get Information about phone directory and category
	if (! AddressPhoneIOGetUserImportSettings(&phoneDirectory, &phoneCategory))
	{
		// Cancel the export
		err = telErrResultUserCancel;
		goto Exit;
	}

	categoriesP = (phoneCategory == catIDUnfiled) ? NULL : &phoneCategory;
	numCategories = (phoneCategory == catIDUnfiled) ? 0 : 1;

	// Set the recipient phone book to put entries to
	len = sizeof(uint8_t);
	err = ExgControl(&exgSocket, phoneBkIOLibCtlSetPhoneRecipient, &phoneDirectory, &len);
	if (err < errNone)
		goto Exit;

	// Get the data from the phone
	err = ExgGet(&exgSocket);
	if (err < errNone)
		goto Exit;

	if (gCursorID != dbInvalidCursorID)
	{
		DbCursorClose(gCursorID);
		gCursorID = dbInvalidCursorID;
	}

	sprintf(sql, "%s %s", kAddressDefaultTableName, gCurrentOrderByStr);
	DbCursorOpenWithCategory(dbP, sql, 0, numCategories, categoriesP, DbMatchExact, &cursorID);
	// If there is records to delete, ask the user to confirm and delete records
	if (cursorID != dbInvalidCursorID)
	{
		if (DbCursorGetRowCount(cursorID) > 0)
		{
			CatMgrTruncateName(dbP, categoriesP, numCategories, kCategoryLabelLength, catName);
			if (FrmCustomAlert(gApplicationDbP, PhoneIOImportAlert, catName, NULL, NULL))
			{
				// Cancel button pressed
				err = telErrResultUserCancel;
			}
			else
			{
				// Erase all records in SELECTED category
				err = DbCursorDeleteAllRows(cursorID);
				ErrNonFatalDisplayIf(err < errNone, "Can't remove rows");
			}
		}

		DbCursorClose(cursorID);
		cursorID = dbInvalidCursorID;

		if (err < errNone)
		{
			AddressDBOpenCursor(); // Re-open global cursor before exiting
			goto DisconnectAndExit;
		}
	}

	// Remove imported category membership to existing records
	DbCursorOpenWithCategory(dbP, sql, 0, numCategories, categoriesP, DbMatchAny, &cursorID);
	if (cursorID != dbInvalidCursorID)
	{
		if (DbCursorGetRowCount(cursorID) > 0)
		{
			err = DbRemoveCategoryAllRows(dbP, numCategories, categoriesP, DbMatchAny);
			ErrNonFatalDisplayIf(err < errNone, "Can't remove rows");
		}

		DbCursorClose(cursorID);
		cursorID = dbInvalidCursorID;
	}

	// Set the socket category ID, this will allow incoming vCards to be stored in Phone category
	exgSocket.appData = categoriesP ? *categoriesP : catIDUnfiled;

	// Re-open the cursor
	AddressDBOpenCursor();

	// No error : Connection is ok, start receiving data.
	// vCards will be put in phone category
	// Do not accept, connection is already ok
	err = TransferReceiveData(dbP, gCursorID, &exgSocket, false, true, NULL);

	*phoneCategoryP = phoneCategory;

DisconnectAndExit:
	ExgDisconnect(&exgSocket, err);

Exit:
	MemPtrFree(exgSocket.name);
	return err;
}



/***********************************************************************
 *
 * FUNCTION:		TransferSendPhoneCategory
 *
 * DESCRIPTION:		Sends the complete phone category to the
 *				phone using the PhoneBookIO exchange library.
 *
 * PARAMETERS:
 *
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         PLe    04/19/00  Created
 *
 ***********************************************************************/
status_t TransferSendPhoneCategory(DmOpenRef dbP)
{
	status_t 		err = errNone;
	ExgSocketType 	exgSocket;
	CategoryID		phoneCategory;
	CategoryID *	categoriesP;
	uint32_t		numCategories;
	uint16_t		len;
	uint8_t			phoneDirectory;

	// Initialize the socket fields
	// This socket is only used to address the correct exgl lib, in order to retrieve the phone
	// category name.
	memset(&exgSocket, 0, sizeof(ExgSocketType));
	exgSocket.name = MemPtrNew((uint32_t)(strlen(exgPhoneBookIOPrefix) + 1));
	strcpy(exgSocket.name, exgPhoneBookIOPrefix);

	// Get Information about phone directory and category
	if (! AddressPhoneIOGetUserExportSettings(&phoneDirectory, &phoneCategory))
	{
		// Cancel the export
		err = exgErrUserCancel;
		goto Exit;
	}

	categoriesP = (phoneCategory == catIDUnfiled) ? NULL : &phoneCategory;
	numCategories = (phoneCategory == catIDUnfiled) ? 0 : 1;

	// Ask for recipient phone book to put entries to
	len = sizeof(uint8_t);
	err = ExgControl(&exgSocket, phoneBkIOLibCtlSetPhoneRecipient, &phoneDirectory, &len);
	if (err < errNone)
		goto Exit;

	TransferSendCategory(gAddrDB, numCategories, categoriesP, exgPhoneBookIOPrefix, NoDataToSendAlert);

	ExgDisconnect(&exgSocket, errNone);

Exit:
	MemPtrFree(exgSocket.name);
	return err;
}

/***********************************************************************
 *
 * FUNCTION:	TransferReceiveData
 *
 * DESCRIPTION:	Receives data into the output field using the Exg API
 *
 * PARAMETERS:	exgSocketP, socket from the app code
 *				 sysAppLaunchCmdExgReceiveData
 *
 * RETURNED:	error code or zero for no error.
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * ABa	06/20/00	Integrate Pdi library
 *
 ***********************************************************************/
status_t TransferReceiveData(DmOpenRef dbP, uint32_t cursorID, ExgSocketPtr exgSocketP, Boolean acceptBefore, Boolean setGotoParams, char** ffnPP)
{
	status_t		err;
	PdiReaderType*	reader = NULL;
	UDAReaderType*	stream = NULL;

	// accept will open a progress dialog and wait for your receive commands
	if (acceptBefore)
	{
		if ((err = ExgAccept(exgSocketP)) < errNone)
			return err;
	}

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
	while(TransferImportVCard(dbP, cursorID, reader, setGotoParams, NULL, &err, ffnPP)) { /* Do nothing */ }

	// Aba: A record has been added in the Database if the GoTo uniqueID parameter != 0.
	// In the case no record is added, return an error
	if (err >= errNone && cursorID != dbInvalidCursorID && exgSocketP->goToParams.uniqueID == 0)
		err = exgErrBadData;

errorDisconnect:
	if (reader)
		PdiReaderDelete(&reader);

	if (stream)
		UDADelete(stream);

	ExgDisconnect(exgSocketP, err); // closes transfer dialog
	err = errNone;	// error was reported, so don't return it

	return err;
}


/***********************************************************************
 *
 * FUNCTION:	TransferImportData
 *
 * DESCRIPTION:	Receives data from an imported vObject
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 *				impExpObjP - pointer on the record param Ptr
 *
 * RETURNED:	error code or zero for no error.
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * PLe  05/27/02	Added Import / Export feature
 *
 ***********************************************************************/
status_t TransferImportData(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP)
{
	status_t		err;
	PdiReaderType*	reader = NULL;
	UDAReaderType*	stream = NULL;

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
	TransferImportVCard(dbP, cursorID, reader, true, &(impExpObjP->uniqueID), &err, NULL);
	PdiReaderDelete(&reader);

	if (err >= errNone && impExpObjP->uniqueID != dbInvalidRowID)
	{
		DbCursorRequery(cursorID);
		DbCursorGetPositionForRowID(cursorID, impExpObjP->uniqueID, &impExpObjP->index);
		impExpObjP->index--;
	}
	else
	{
		impExpObjP->uniqueID = dbInvalidRowID;
		impExpObjP->index = ImpExpInvalidRecIndex;
	}

Exit:
	if (reader)
		PdiReaderDelete(&reader);

	if (stream)
		UDADelete(stream);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:	TransferExportData
 *
 * DESCRIPTION:	Export a specified record as vObject
 *
 * PARAMETERS:	dbP - pointer to the database to get the record from
 *				impExpObjP - pointer on the record param Ptr
 *
 * RETURNED:	error code or zero for no error.
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * PLe  05/27/02	Added Import / Export feature
 *
 ***********************************************************************/
status_t TransferExportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t		err = errNone;
	PdiWriterType*	writer = NULL;
	UDAWriterType*	stream = NULL;

	if (impExpObjP->index != ImpExpInvalidRecIndex)
	{
		DbCursorGetRowIDForPosition(gCursorID, impExpObjP->index + 1, &impExpObjP->uniqueID);
	}
	else if (impExpObjP->uniqueID != ImpExpInvalidUniqueID)
	{
		DbCursorGetPositionForRowID(gCursorID, impExpObjP->uniqueID, &impExpObjP->index);
		--impExpObjP->index;
	}
	else
	{
		return dmErrInvalidParam;
	}

	// The app just looks for rowID from index (or conversely). Return errNone
	if (impExpObjP->vObjectH == NULL)
		return errNone;

	// Create an PDI writer
	stream = UDAMemHandleWriterNew(impExpObjP->vObjectH,  ALLOCATE_VOBJECT_INC_SIZE);
	writer = PdiWriterNew(stream, PrvTransfertGetPdiParameters());

	err = TransferExportVCard(dbP, impExpObjP->uniqueID, writer, kFieldFamilyMask | kFieldType_Binary | kFieldKind_Note);

	PdiWriterDelete(&writer);

	UDAWriterFlush(stream);

	if (writer)
		PdiWriterDelete(&writer);

	if (stream)
		UDADelete(stream);

	return err;
}

/***********************************************************************
 *
 * FUNCTION:	TransfertDeleteData
 *
 * DESCRIPTION:	Delete the specified record
 *
 * PARAMETERS:	dbP - pointer to the database to get the record from
 *				impExpObjP - pointer on the record param Ptr
 *
 * RETURNED:	error code or zero for no error.
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * PLe  05/27/02	Added Import / Export feature
 *
 ***********************************************************************/
status_t TransfertDeleteData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	uint32_t	index = impExpObjP->index + 1;
	status_t	error;

	if (impExpObjP->index != ImpExpInvalidRecIndex)
		error = DbCursorGetRowIDForPosition(gCursorID, index, &impExpObjP->uniqueID);
	else if (impExpObjP->uniqueID != dbInvalidRowID)
		error = DbCursorGetPositionForRowID(gCursorID, impExpObjP->uniqueID, &index);
	else
		error = dmErrInvalidParam;

	if (error >= errNone)
	{
		DbCursorSetAbsolutePosition(gCursorID, index);
		error = DbDeleteRow(gAddrDB, gCursorID);
		ErrNonFatalDisplayIf(error < errNone, "Can't delete row");
		impExpObjP->index = index - 1;
	}
	else
	{
		impExpObjP->index = ImpExpInvalidRecIndex;
		impExpObjP->uniqueID = dbInvalidRowID;
	}

	return error;
}

/***********************************************************************
 *
 * FUNCTION:	TransfertMoveData
 *
 * DESCRIPTION:	Move a record to the specified location
 *
 * PARAMETERS:	dbP - pointer to the database to get the record from
 *				impExpObjP - pointer on the record param Ptr
 *
 * RETURNED:	error code or zero for no error.
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 * PLe  05/27/02	Added Import / Export feature
 *
 ***********************************************************************/
status_t TransfertMoveData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP)
{
	status_t	error;

	if (impExpObjP->index == ImpExpInvalidRecIndex || impExpObjP->destIndex == ImpExpInvalidRecIndex)
		return dmErrInvalidParam;

	error = DbCursorRelocateRow(gCursorID, impExpObjP->index, impExpObjP->destIndex);
	if (error >= errNone)
		error = DbCursorGetRowIDForPosition(gCursorID, impExpObjP->destIndex, &impExpObjP->uniqueID);

	return error;
}

/************************************************************
 *
 * FUNCTION: PrvVCardSenderType
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	--------	-----------
 *
 *************************************************************/
static VCardSenderKindType PrvVCardSenderType(char * palmOSVersion)
{
	if (palmOSVersion && strcmp(palmOSVersion, kVObjectPalmOSV6Version) >= 0)
	{
		// Import PALM OS vCard newer or equal to v6.0
		return kPalmOS6AndAfter;
	}
	else if (palmOSVersion)
	{
		// Import older PALM OS vCard
		return kPalmOS5AndBefore;
	}

	return kNonPalmOSDevice;
}

/************************************************************
 *
 * FUNCTION: PrvTransferImportName
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: 	true if the vCard is a business one.
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static Boolean PrvTransferImportName(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, char * companyName, char * freeFormName)
{
	uint32_t 	familyOrKind, propFieldNumber = 0, field;
	char * 		propFieldStr;
	char *		lastNameStr = NULL;
	uint32_t	assignedNameFields = 0;
	Boolean		consideredAsBusinessCard = false;

	if (ImportGetFirstProperty(kFieldType_Name, &familyOrKind, &propFieldNumber))
	{
		// Checkings and initializations...
		ErrNonFatalDisplayIf(propFieldNumber != nNameFields, "Invalid Name property field number");

		for (field = 0; field < propFieldNumber; field++)
		{
			if (ImportGetPropertyField(field, NULL))
				assignedNameFields++;
		}

		// Compare last name and company if the last name is the only field assigned
		lastNameStr = ImportGetPropertyField(iLastName, NULL);
		if (lastNameStr && (assignedNameFields == 1) && companyName && !strcmp(lastNameStr, companyName))
		{
			// Last name and company are equal, remove lastname...
			lastNameStr = NULL;
			consideredAsBusinessCard = true;
		}

		// Remove last name if it is "Unnamed"
		if (lastNameStr && (assignedNameFields == 1))
		{
			if (gUnnamedRecordStringP && !strcmp(gUnnamedRecordStringP, lastNameStr))
				lastNameStr = NULL;
		}
	}

	// If no name defined but a company name, the card is a business card
	if ((assignedNameFields == 0) && companyName)
		consideredAsBusinessCard = true;

	// Only one propery for the names
	for (field = 0; field < propFieldNumber; field++)
	{
		if (field == iLastName)
		{
			if (!lastNameStr)
				continue;

			propFieldStr = lastNameStr;
		}
		else
			propFieldStr = ImportGetPropertyField(field, NULL);

		if (propFieldStr && (*colIndexP < colsToWrite))
		{
			colVals[*colIndexP].columnID = nameColIDs[field];
			colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
			colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
			(*colIndexP)++;
			assignedNameFields++;
		}
	}

	// Take care of FN (free form name) if no name at all
	if (freeFormName && (assignedNameFields == 0) && (*colIndexP < colsToWrite))
	{
		colVals[*colIndexP].columnID = nameColIDs[iLastName];
		colVals[*colIndexP].data = (DbSchemaColumnData*) freeFormName;
		colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(freeFormName);
		(*colIndexP)++;
	}

	return consideredAsBusinessCard;
}

/************************************************************
 *
 * FUNCTION: PrvAddFieldInNotes
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: none
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static void PrvAddFieldInNotes(const char * descP, char * propFieldStr, char ** notePP)
{
	size_t noteLengthToAdd, noteInitialSize;
	char * newNoteP;

	if (! propFieldStr)
		return;

	// If we don't find a description in the schema for a field, set it to ?
	if (! descP)
		descP = "?";

	noteLengthToAdd = strlen(descP) + strlen(propFieldStr);

	if (*notePP == NULL)
	{
		*notePP = MemPtrNew((uint32_t)(noteLengthToAdd + 3));
		strcpy(*notePP, descP);
		strcat(*notePP, ": ");
		strcat(*notePP, propFieldStr);
	}
	else
	{
		noteInitialSize = strlen(*notePP);
		newNoteP = MemPtrNew((uint32_t)(noteInitialSize + noteLengthToAdd + 4));
		strcpy(newNoteP, *notePP);
		strcat(newNoteP, "\n");
		strcat(newNoteP, descP);
		strcat(newNoteP, ": ");
		strcat(newNoteP, propFieldStr);
		MemPtrFree(*notePP);
		*notePP = newNoteP;
	}
}


/************************************************************
 *
 * FUNCTION: PrvTransferGetNoteDescriptionFromColID
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: The label
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static char * PrvTransferGetNoteDescriptionFromColID(uint32_t columnID)
{
	AddressTabColumnPropertiesType*	propP;
	char*							descP;

	if ((propP = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, columnID)) != NULL)
		descP = propP->labelP;
	else
		descP = NULL;

	return descP;
}


/************************************************************
 *
 * FUNCTION: PrvTransferAddOrphanFieldInNotes
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: none
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static void PrvTransferAddOrphanFieldInNotes(uint32_t fieldInfo, char * propFieldStr, char ** notePP)
{
	char*		descP;
	uint32_t	colID = kInvalidColumnID;

	switch(fieldInfo & kFieldTypeMask)
	{
		case kFieldType_Address:
			fieldInfo = kFieldFamily_Home | kFieldType_Address | (fieldInfo & kFieldKindMask);
			colID = AddressTabFindColIdByFieldInfo(fieldInfo);
			break;

		case kFieldType_Phone:
			switch(fieldInfo & kFieldKindMask)
			{
				case kFieldKind_VoicePhone:		colID = kAddrColumnIDHomePhone;				break;
				case kFieldKind_MobilePhone:	colID = kAddrColumnIDHomeMobile;			break;
				case kFieldKind_FaxPhone:		colID = kAddrColumnIDHomeFax;				break;
				case kFieldKind_PagerPhone:		colID = kAddrColumnIDOtherPager;			break;
				case kFieldKind_AssistantPhone:	colID = kAddrColumnIDWorkAssistantPhone;	break;
			}
			break;

		case kFieldType_Internet:
			switch(fieldInfo & kFieldKindMask)
			{
				case kFieldKind_EmailInternet:	colID = kAddrColumnIDHomeEmail1;	break;
				case kFieldKind_URLInternet:	colID = kAddrColumnIDOtherURL;		break;
			}
			break;

		case kFieldType_Extended | kFieldType_InstantMessaging:
			switch(fieldInfo & kFieldKindMask)
			{
				case kFieldKind_ICQ_IM:		colID = kAddrColumnIDOtherICQ;		break;
				case kFieldKind_AIM_IM:		colID = kAddrColumnIDOtherAIM;		break;
				case kFieldKind_Yahoo_IM:	colID = kAddrColumnIDOtherYahoo;	break;
				case kFieldKind_MSN_IM:		colID = kAddrColumnIDOtherMSN;		break;
				case kFieldKind_Jabber_IM:	colID = kAddrColumnIDOtherJabber;	break;
			}
			break;

		case kFieldType_Extended:
			switch(fieldInfo & kFieldKindMask)
			{
				case kFieldKind_CustomExt:	colID = kAddrColumnIDOtherCustom1;	break;
			}
			break;
	}

	if (colID != kInvalidColumnID)
	{
		descP = PrvTransferGetNoteDescriptionFromColID(colID);
		PrvAddFieldInNotes(descP, propFieldStr, notePP);
	}
}

/************************************************************
 *
 * FUNCTION: PrvTransferImportAddresses
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: none
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static void PrvTransferImportAddresses(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, Boolean consideredAsBusinessCard, VCardSenderKindType vCardSender,
	char ** notePP)
{
	uint32_t 	family, propFieldNumber, field, fieldInfo, fieldIndex, familyIndex;
	uint32_t	columnID;
	size_t		initialStreetLength, newStreetLength;
	char * 		propFieldStr;
	char * 		newStreetFieldStr;
	char * 		streetFieldStr;
	int32_t 	addrSlotsAssigned[nFamilyNumber];
	Boolean		putFieldInNotes;
	Boolean		canChangeFamily = false;
	char 		spaceStr[] = " ";

	if (! ImportGetFirstProperty(kFieldType_Address, &family, &propFieldNumber))
		return;

	// Checkings and initializations...
	ErrNonFatalDisplayIf(propFieldNumber != nAddressFields, "Invalid Address property field number");
	memset(addrSlotsAssigned, 0, sizeof(addrSlotsAssigned));

	do
	{
		// Here, family can be kFieldFamily_Other / kFieldFamily_Home / kFieldFamily_Corp
		switch (vCardSender)
		{
			case kPalmOS5AndBefore:
				canChangeFamily = true ;

				if ((family == kFieldFamily_Corp) && (!consideredAsBusinessCard))
					family = kFieldFamily_Home;

				break;

			case kNonPalmOSDevice:
				// Non Palm device: not specifying the family will depend on company name
				canChangeFamily = true ;

				if (family == kFieldFamily_Other)
				{
					if (consideredAsBusinessCard)
						family = kFieldFamily_Corp;
					else
						family = kFieldFamily_Home;
				}

				break;
		}

		familyIndex = familyIndexFromFieldInfo(family);

		// Switch to another slot if not available
		if (addrSlotsAssigned[familyIndex] == gAddressSlotsAvailable[familyIndex] && canChangeFamily)
		{
			if (!(family & kFieldFamily_Other))
			{
				// Current Family full : try Other
				if (addrSlotsAssigned[familyIndexFromFieldInfo(kFieldFamily_Other)] != gAddressSlotsAvailable[familyIndexFromFieldInfo(kFieldFamily_Other)])
				{
					family = (family & (~kFieldFamilyMask)) | kFieldFamily_Other;
					familyIndex = familyIndexFromFieldInfo(family);
				}
			}
		}

		if (addrSlotsAssigned[familyIndex] == gAddressSlotsAvailable[familyIndex])
		{
			// NO MORE SLOTS !!! Put the address in notes
			putFieldInNotes = true;
			fieldIndex = 0;
		}
		else
		{
			putFieldInNotes = false;
			// Set the field index (number of addresses in this family)
			fieldIndex = addrSlotsAssigned[familyIndex];
			addrSlotsAssigned[familyIndex]++;
		}

		// Now, put address in available slot
		fieldInfo = kFieldType_Address | family | fieldIndex;

		// Concatenates iStreetAddress with iXAddress and iPOBoxAddress
		initialStreetLength = 0;

		if ((streetFieldStr = ImportGetPropertyField(iStreetAddress, NULL)) != NULL)
			initialStreetLength = strlen(streetFieldStr);

		newStreetLength = initialStreetLength;

		if ((propFieldStr = ImportGetPropertyField(iXAddress, NULL)) != NULL)
			newStreetLength += (strlen(propFieldStr) + 1);

		if ((propFieldStr = ImportGetPropertyField(iPOBoxAddress, NULL)) != NULL)
			newStreetLength += (strlen(propFieldStr) + 1);

		// There's some concatenations to perform...
		if (newStreetLength != initialStreetLength)
		{
			newStreetFieldStr = MemPtrNew((uint32_t) (newStreetLength + 1));
			newStreetFieldStr[0] = chrNull;

			if (streetFieldStr)
				strcpy(newStreetFieldStr, streetFieldStr);

			if ((propFieldStr = ImportGetPropertyField(iXAddress, NULL)) != NULL)
			{
				strcat(newStreetFieldStr, spaceStr);
				strcat(newStreetFieldStr, propFieldStr);
			}

			if ((propFieldStr = ImportGetPropertyField(iPOBoxAddress, NULL)) != NULL)
			{
				strcat(newStreetFieldStr, spaceStr);
				strcat(newStreetFieldStr, propFieldStr);
			}

			propFieldStr = newStreetFieldStr;

			// Free old street allocated by PDI Lib and assign to street
			// to get it deleted after record creation
			MemPtrFree(streetFieldStr);
			ImportUpdatePropertyField(iStreetAddress, newStreetFieldStr, 0);
		}
		else
			propFieldStr = streetFieldStr;

		// Store the street
		if (propFieldStr)
		{
			fieldInfo = (fieldInfo & (~kFieldKindMask)) | kFieldKind_StreetAddress;
			columnID = AddressTabFindColIdByFieldInfo(fieldInfo);

			if (putFieldInNotes || (*colIndexP >= colsToWrite))
				PrvTransferAddOrphanFieldInNotes(fieldInfo, propFieldStr, notePP);
			else if (columnID != kInvalidColumnID)
			{
				colVals[*colIndexP].columnID = columnID;
				colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
				colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
				(*colIndexP)++;
			}
		}

		for (field = iCityAddress; field <= iCountryAddress; field++)
		{
			if ((propFieldStr = ImportGetPropertyField(field, NULL)) != NULL)
			{
				fieldInfo = (fieldInfo & (~kFieldKindMask)) | addressFieldKinds[field];
				columnID = AddressTabFindColIdByFieldInfo(fieldInfo);

				ErrNonFatalDisplayIf(columnID == kInvalidColumnID, "Invalid column ID");

				if (putFieldInNotes || (*colIndexP >= colsToWrite))
					PrvTransferAddOrphanFieldInNotes(fieldInfo, propFieldStr, notePP);
				else if (columnID != kInvalidColumnID)
				{
					colVals[*colIndexP].columnID = columnID;
					colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
					colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
					(*colIndexP)++;
				}
			}
		}
	}
	while (ImportGetNextProperty(kFieldType_Address, &family, &propFieldNumber));
}

/************************************************************
 *
 * FUNCTION: PrvTransferImportPhoneNumbers
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: ColumnID of the preferred phone
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static uint32_t PrvTransferImportPhoneNumbers(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, Boolean consideredAsBusinessCard, VCardSenderKindType vCardSender,
	char ** notePP)
{
	uint32_t 	familyOrKind, propFieldNumber, fieldInfo, familyIndex, fieldIndex, phoneIndex;
	char * 		propFieldStr;
	uint32_t	columnID;
	int32_t 	phoneSlotsAssigned[nPhoneNumber][nFamilyNumber];
	uint32_t	prefColID = kInvalidColumnID;
	Boolean		putFieldInNotes;
	Boolean		canChangeFamily = false;

	if (! ImportGetFirstProperty(kFieldType_Phone, &familyOrKind, &propFieldNumber))
		return prefColID;

	// Checkings and initializations...
	ErrNonFatalDisplayIf(propFieldNumber != nPhoneFields, "Invalid Phone property field number");
	memset(phoneSlotsAssigned, 0, sizeof(phoneSlotsAssigned)) ;

	do
	{
		// Here, familyOrKind contains Family AND Kind information - Device by device checks...
		switch (vCardSender)
		{
			case kPalmOS5AndBefore:
				// Parameter HOME or WORK is actually only used for VOICE phones
				if (! (familyOrKind & kFieldKind_VoicePhone) )
					canChangeFamily = true ;

				// FALLBACK: DO NOT BREAK!!

			case kNonPalmOSDevice:
				// If not 6.0 device, other becomes corp or home...
				if (familyOrKind & kFieldFamily_Other)
				{
					canChangeFamily = true;

					if (consideredAsBusinessCard)
						familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Corp;
					else
						familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Home;
				}
				break;
		}

		familyIndex = familyIndexFromFieldInfo(familyOrKind);
		phoneIndex = phoneIndexFromFieldInfo(familyOrKind);

		// Switch to another slot if not available
		if (phoneSlotsAssigned[phoneIndex][familyIndex] == gPhoneSlotsAvailable[phoneIndex][familyIndex] && canChangeFamily)
		{
			if (!(familyOrKind & kFieldFamily_Other))
			{
				// Current Family full : try Other
				if (phoneSlotsAssigned[phoneIndex][familyIndexFromFieldInfo(kFieldFamily_Other)] != gPhoneSlotsAvailable[phoneIndex][familyIndexFromFieldInfo(kFieldFamily_Other)])
				{
					familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Other;
					familyIndex = familyIndexFromFieldInfo(familyOrKind);
				}
			}
		}

		if (phoneSlotsAssigned[phoneIndex][familyIndex] == gPhoneSlotsAvailable[phoneIndex][familyIndex])
		{
			// NO MORE SLOTS !!! Put the address in notes
			putFieldInNotes = true;
			fieldIndex = 0;
		}
		else
		{
			putFieldInNotes = false;
			// Set the field index (number of phones by kind and family)
			fieldIndex = phoneSlotsAssigned[phoneIndex][familyIndex];
			phoneSlotsAssigned[phoneIndex][familyIndex]++;
		}

		fieldInfo = familyOrKind | fieldIndex;

		// Get the phone number now
		if ((propFieldStr = ImportGetPropertyField(0, NULL)) != NULL)
		{
			columnID = AddressTabFindColIdByFieldInfo(fieldInfo);
			ErrNonFatalDisplayIf(columnID == kInvalidColumnID, "Invalid column ID");

			if (putFieldInNotes || (*colIndexP >= colsToWrite))
				PrvTransferAddOrphanFieldInNotes(fieldInfo, propFieldStr, notePP);
			else if (columnID != kInvalidColumnID)
			{
				colVals[*colIndexP].columnID = columnID;
				colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
				colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
				(*colIndexP)++;
			}

			// Set column IDs for displayed phones or emails in list view
			if (columnID != kInvalidColumnID)
			{
				if (ImportIsCurrentPropertyPreferred() && (prefColID == kInvalidColumnID))
					prefColID = columnID;
			}
		}
	}
	while (ImportGetNextProperty(kFieldType_Phone, &familyOrKind, &propFieldNumber));

	return prefColID;
}

/************************************************************
 *
 * FUNCTION: PrvTransferImportInternet
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: none
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static uint32_t PrvTransferImportInternet(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, Boolean consideredAsBusinessCard, VCardSenderKindType vCardSender,
	char ** notePP)
{
	uint32_t 	familyOrKind, propFieldNumber, fieldInfo, familyIndex, internetIndex;
	int32_t		fieldIndex;
	char * 		propFieldStr;
	uint32_t	columnID;
	int32_t 	internetSlotsAssigned[nFamilyNumber][nInternetNumber];
	uint32_t	prefColID = kInvalidColumnID;
	Boolean		putFieldInNotes;
	Boolean		canChangeFamily = false;

	Boolean	*	usedSlotPerFamilyP[nFamilyNumber][nInternetNumber];
	Boolean		foundMatchingSlot = false;

	if (! ImportGetFirstProperty(kFieldType_Internet, &familyOrKind, &propFieldNumber))
		return prefColID;

	// Checkings and initializations...
	// nEmailFields == nURLFields, so only one check
	ErrNonFatalDisplayIf(propFieldNumber != nEmailFields, "Invalid Email property field number");
	memset(internetSlotsAssigned, 0, sizeof(internetSlotsAssigned)) ;

	// Initializations...
	for (familyIndex = kHomeFamilyIndex; familyIndex <= kOtherFamilyIndex; familyIndex++)
	{
		for (internetIndex = kEmailInternetIndex; internetIndex <= kURLInternetIndex; internetIndex++)
		{
			if (gInternetSlotsAvailable[familyIndex][internetIndex] > 0)
			{
				usedSlotPerFamilyP[familyIndex][internetIndex] = MemPtrNew(gInternetSlotsAvailable[familyIndex][internetIndex] * sizeof(Boolean));
				memset(usedSlotPerFamilyP[familyIndex][internetIndex], 0, gInternetSlotsAvailable[familyIndex][internetIndex] * sizeof(Boolean)) ;
			}
			else
				usedSlotPerFamilyP[familyIndex][internetIndex] = NULL;
		}
	}


	// Get emails fields...
	do
	{
		// Here, familyOrKind contains Family AND Kind information - Device by device checks...
		switch (vCardSender)
		{
			case kNonPalmOSDevice:
			case kPalmOS5AndBefore:
				canChangeFamily = true;

				// If not 6.0 device, other becomes corp or home...
				if ((familyOrKind & kFieldFamily_Other) && (familyOrKind & kFieldKind_EmailInternet))
				{
					// If this is a business card, assign first work slots, then home slots
					if ((consideredAsBusinessCard) &&
						(internetSlotsAssigned[kCorpFamilyIndex][kEmailInternetIndex] <
						gInternetSlotsAvailable[kCorpFamilyIndex][kEmailInternetIndex]))
						// Assign : work email
						familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Corp;
					else if (!consideredAsBusinessCard && internetSlotsAssigned[kHomeFamilyIndex][kEmailInternetIndex] <
						gInternetSlotsAvailable[kHomeFamilyIndex][kEmailInternetIndex])
						// Assign : perso email
						familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Home;
				}
				break;
		}

		familyIndex = familyIndexFromFieldInfo(familyOrKind);
		internetIndex = internetIndexFromFieldInfo(familyOrKind);

		if ((propFieldStr = ImportGetPropertyField(0, (int32_t*)&fieldIndex)) == NULL)
			continue;

		if (fieldIndex)
			fieldIndex--;

		foundMatchingSlot = (Boolean)	(usedSlotPerFamilyP[familyIndex][internetIndex] &&
										fieldIndex < gInternetSlotsAvailable[familyIndex][internetIndex] &&
										!usedSlotPerFamilyP[familyIndex][internetIndex][fieldIndex]);

		if (!foundMatchingSlot && internetSlotsAssigned[familyIndex][internetIndex] < gInternetSlotsAvailable[familyIndex][internetIndex])
		{
			// Get other slots
			for (fieldIndex = 0; fieldIndex < gInternetSlotsAvailable[familyIndex][internetIndex]; fieldIndex++)
			{
				if (usedSlotPerFamilyP[familyIndex][internetIndex][fieldIndex] == false)
				{
					foundMatchingSlot = true;
					break;
				}
			}
		}

		if (internetSlotsAssigned[familyIndex][internetIndex] == gInternetSlotsAvailable[familyIndex][internetIndex])
		{
			// NO MORE SLOTS !!! Put the address in notes
			putFieldInNotes = true;
			fieldIndex = 0;
		}
		else
		{
			putFieldInNotes = false;
			// Set the field index (number of phones by kind and family)
			//fieldIndex = internetSlotsAssigned[internetIndex][familyIndex];
			internetSlotsAssigned[familyIndex][internetIndex]++;
			usedSlotPerFamilyP[familyIndex][internetIndex][fieldIndex] = true;
		}

		// ### LFe: Do it right. It's just a temporary change
		if (gInternetSlotsAvailable[familyIndex][internetIndex] > 1)
			fieldIndex++; // 1 based

		fieldInfo = familyOrKind | fieldIndex;

		// Get the phone number now
		columnID = AddressTabFindColIdByFieldInfo(fieldInfo);

		if (putFieldInNotes || (*colIndexP >= colsToWrite))
			PrvTransferAddOrphanFieldInNotes(fieldInfo, propFieldStr, notePP);
		else if (columnID != kInvalidColumnID)
		{
			colVals[*colIndexP].columnID = columnID;
			colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
			colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
			(*colIndexP)++;
		}

		if (ImportIsCurrentPropertyPreferred() && (prefColID == kInvalidColumnID))
			prefColID = columnID;

		// Set column IDs for displayed phones or emails in list view
		if (columnID != kInvalidColumnID)
		{
			if (ImportIsCurrentPropertyPreferred() && (prefColID == kInvalidColumnID))
				prefColID = columnID;
		}
	}
	while (ImportGetNextProperty(kFieldType_Internet, &familyOrKind, &propFieldNumber));

	for (familyIndex = kHomeFamilyIndex; familyIndex <= kOtherFamilyIndex; familyIndex++)
	{
		for (internetIndex = kEmailInternetIndex; internetIndex <= kURLInternetIndex; internetIndex++)
		{
			if (usedSlotPerFamilyP[familyIndex][internetIndex])
				MemPtrFree(usedSlotPerFamilyP[familyIndex][internetIndex]);
		}
	}

	return prefColID;
}

/************************************************************
 *
 * FUNCTION: PrvTransferImportInstantMessaging
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: none
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static uint32_t PrvTransferImportInstantMessaging(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, Boolean consideredAsBusinessCard, VCardSenderKindType vCardSender,
	char ** notePP)
{
	uint32_t 	familyOrKind, propFieldNumber, fieldInfo, familyIndex, fieldIndex, phoneIndex;
	char * 		propFieldStr;
	uint32_t	columnID;
	int32_t 	imSlotsAssigned[nInstantMessagingNumber][nFamilyNumber];
	uint32_t	prefColID = kInvalidColumnID;
	Boolean		putFieldInNotes;
	Boolean		canChangeFamily = false;

	if (! ImportGetFirstProperty(kFieldType_InstantMessaging, &familyOrKind, &propFieldNumber))
		return prefColID;

	// Checkings and initializations...
	// nEmailFields == nURLFields, so only one check
	ErrNonFatalDisplayIf(propFieldNumber != nIMFields, "Invalid IM property field number");
	memset(imSlotsAssigned, 0, sizeof(imSlotsAssigned)) ;

	// Get emails fields...
	do
	{
		// Here, familyOrKind contains Family AND Kind information - Device by device checks...
		switch (vCardSender)
		{
			case kNonPalmOSDevice:
				canChangeFamily = true ;

				// If not 6.0 device, other becomes corp or home...
				if (familyOrKind & kFieldFamily_Other)
				{
					if (consideredAsBusinessCard)
						familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Corp;
					else
						familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Home;
				}
				break;
		}

		familyIndex = familyIndexFromFieldInfo(familyOrKind);
		phoneIndex = imIndexFromFieldInfo(familyOrKind);

		// Switch to another slot if not available
		if (imSlotsAssigned[phoneIndex][familyIndex] == gIMSlotsAvailable[phoneIndex][familyIndex] && canChangeFamily)
		{
			if (!(familyOrKind & kFieldFamily_Other))
			{
				// Current Family full : try Other
				if (imSlotsAssigned[phoneIndex][familyIndexFromFieldInfo(kFieldFamily_Other)] != gIMSlotsAvailable[phoneIndex][familyIndexFromFieldInfo(kFieldFamily_Other)])
				{
					familyOrKind = (familyOrKind & (~kFieldFamilyMask)) | kFieldFamily_Other;
					familyIndex = familyIndexFromFieldInfo(familyOrKind);
				}
			}
		}

		if (imSlotsAssigned[phoneIndex][familyIndex] == gIMSlotsAvailable[phoneIndex][familyIndex])
		{
			// NO MORE SLOTS !!! Put the address in notes
			putFieldInNotes = true;
			fieldIndex = 0;
		}
		else
		{
			putFieldInNotes = false;
			// Set the field index (number of phones by kind and family)
			fieldIndex = imSlotsAssigned[phoneIndex][familyIndex];
			imSlotsAssigned[phoneIndex][familyIndex]++;
		}

		fieldInfo = familyOrKind | fieldIndex;

		// Get the phone number now
		if ((propFieldStr = ImportGetPropertyField(0, NULL)) != NULL)
		{
			columnID = AddressTabFindColIdByFieldInfo(fieldInfo);
			ErrNonFatalDisplayIf(columnID == kInvalidColumnID, "Invalid column ID");

			if (putFieldInNotes || (*colIndexP >= colsToWrite))
				PrvTransferAddOrphanFieldInNotes(fieldInfo, propFieldStr, notePP);
			else if (columnID != kInvalidColumnID)
			{
				colVals[*colIndexP].columnID = columnID;
				colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
				colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
				(*colIndexP)++;
			}

			// Set column IDs for displayed phones or emails in list view
			if (columnID != kInvalidColumnID)
			{
				if (ImportIsCurrentPropertyPreferred() && (prefColID == kInvalidColumnID))
					prefColID = columnID;
			}
		}
	}
	while (ImportGetNextProperty(kFieldType_InstantMessaging, &familyOrKind, &propFieldNumber));

	return prefColID;
}

/************************************************************
 *
 * FUNCTION: PrvTransferImportCustomFields
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: none
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static void PrvTransferImportCustomFields(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, VCardSenderKindType vCardSender, char ** notePP)
{
	uint32_t 	kind, propFieldNumber, fieldInfo;
	uint32_t	familyIndex;
	int32_t		customIndex;
	char * 		propFieldStr;
	uint32_t	columnID;
	Boolean		foundMatchingSlot = false;
	Boolean	*	usedSlotPerFamilyP[nFamilyNumber];
	Boolean		putFieldInNotes;

	if (! ImportGetFirstProperty(kFieldType_Extended, &kind, &propFieldNumber))
		return;

	// Checkings
	ErrNonFatalDisplayIf(propFieldNumber != nCustomFields, "Invalid Custom property field number");

	// Initializations...
	for (familyIndex = kHomeFamilyIndex; familyIndex <= kOtherFamilyIndex; familyIndex++)
	{
		if (gCustomSlotsAvailable[familyIndex] > 0)
		{
			usedSlotPerFamilyP[familyIndex] = MemPtrNew(gCustomSlotsAvailable[familyIndex] * sizeof(Boolean));
			memset(usedSlotPerFamilyP[familyIndex], 0, gCustomSlotsAvailable[familyIndex] * sizeof(Boolean)) ;
		}
		else
			usedSlotPerFamilyP[familyIndex] = NULL;
	}

	// Import custom fields
	do
	{
		// Only handle custom fields...
		if (kind != kFieldKind_CustomExt)
			continue;

		// Get the custom property with its index
		if ((propFieldStr = ImportGetPropertyField(0, &customIndex)) == NULL)
			continue;

		// Default Palm custom handling : custom are "other"
		// Reverse family search from Other to Personal
		familyIndex = kOtherFamilyIndex;

		foundMatchingSlot = ! usedSlotPerFamilyP[familyIndex][customIndex];

		if (!foundMatchingSlot)
		{	// Get other slots
			do
			{
				for (customIndex = 0; customIndex < gCustomSlotsAvailable[familyIndex]; customIndex++)
				{
					if (usedSlotPerFamilyP[familyIndex][customIndex] == false)
					{
						foundMatchingSlot = true;
						break;
					}
				}

				if (foundMatchingSlot)
					break;

				familyIndex--;
			}
			while (familyIndex != kHomeFamilyIndex);
		}

		// Switch to another slot if not available
		// NO MORE SLOTS !!! Put the address in notes
		putFieldInNotes = !foundMatchingSlot;

		// Set the field index (number of phones by kind and family)
		usedSlotPerFamilyP[familyIndex][customIndex] = true;

		// Set column
		fieldInfo = kFieldType_Extended | familyFieldsTable[familyIndex] | kFieldKind_CustomExt | ( customIndex + 1 ); // 1-based
		columnID = AddressTabFindColIdByFieldInfo(fieldInfo);

		ErrNonFatalDisplayIf(columnID == kInvalidColumnID, "Invalid column ID");

		if (putFieldInNotes || (*colIndexP >= colsToWrite))
			PrvTransferAddOrphanFieldInNotes(fieldInfo, propFieldStr, notePP);
		else if (columnID != kInvalidColumnID)
		{
			colVals[*colIndexP].columnID = columnID;
			colVals[*colIndexP].data = (DbSchemaColumnData*) propFieldStr;
			colVals[*colIndexP].dataSize = 1 + (uint32_t) strlen(propFieldStr);
			(*colIndexP)++;
		}
	}
	while (ImportGetNextProperty(kFieldType_Extended, &kind, &propFieldNumber));

	// Free allocated mem
	for (familyIndex = kHomeFamilyIndex; familyIndex <= kOtherFamilyIndex; familyIndex++)
	{
		if (usedSlotPerFamilyP[familyIndex])
			MemPtrFree(usedSlotPerFamilyP[familyIndex]);
	}
}

/************************************************************
 *
 * FUNCTION: PrvTransferSetPhonePrefField
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS: true if a preferred phone or email was found.
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 *************************************************************/
static Boolean PrvTransferSetPrefField(DbSchemaColumnValueType * colVals, uint32_t colsToWrite,
	uint32_t * colIndexP, uint32_t prefPhoneColID, uint32_t prefEMailColID, uint32_t prefIMColID, uint32_t * prefColIDP)
{
	*prefColIDP = kInvalidColumnID;

	// Test phone prefs first
	if (prefPhoneColID != kInvalidColumnID)
		*prefColIDP = prefPhoneColID;
	else if (prefEMailColID != kInvalidColumnID)
		*prefColIDP = prefEMailColID;
	else if (prefIMColID != kInvalidColumnID)
		*prefColIDP = prefIMColID;

	if ((*prefColIDP != kInvalidColumnID) && (*colIndexP < colsToWrite))
	{
		colVals[*colIndexP].columnID = kAddrColumnIDDisplayedPhone;
		colVals[*colIndexP].data = (DbSchemaColumnData*) prefColIDP;
		colVals[*colIndexP].dataSize = sizeof(uint32_t);
		(*colIndexP)++;
		return true;
	}

	return false;
}

/************************************************************
 *
 * FUNCTION: PrvTransferGetParameterIndex
 *
 * DESCRIPTION: Check for a field index in the vCard
 *
 * PARAMETERS:	reader	- pointer to the PDI reader addressing the data
 *
 * RETURNS: the field index (1-16). 0 = no index
 *
 *	REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *  LFe		09/02/03	Initial revision
 *
 *************************************************************/
static uint32_t PrvTransferGetParameterIndex(PdiReaderType* reader)
{
	// IMPORTANT
	// Even if this is not obvious, this is compiled optimized code.
	// The PdiParameterPairTest is a macro that perform bit tests.
	// Perform on a constant, the evaluation is done at compile time
	// Don't replace this code with a loop, otherwise the macro will
	// test a variable and the evaluation will be performed at run time.

	if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_1))
		return 1;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_2))
		return 2;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_3))
		return 3;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_4))
		return 4;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_5))
		return 5;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_6))
		return 6;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_7))
		return 7;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_8))
		return 8;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_9))
		return 9;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_10))
		return 10;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_11))
		return 11;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_12))
		return 12;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_13))
		return 13;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_14))
		return 14;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_15))
		return 15;
	else if (PdiParameterPairTest(reader, kPdiPAV_X_PALM_INDEX_X_16))
		return 16;

	return 0;
}

/************************************************************
 *
 * FUNCTION: PrvTransferGetIndex
 *
 * DESCRIPTION: Check for a field index in the vCard
 *
 * PARAMETERS:	reader	- pointer to the PDI reader addressing the data
 *
 * RETURNS: the field index (1-16). 0 = no index
 *
 *	REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *  LFe		09/02/03	Initial revision
 *
 *************************************************************/
static uint32_t PrvTransferGetParameterFamily(PdiReaderType* reader)
{
	// Read family
	if (PdiParameterPairTest(reader, kPdiPAV_TYPE_HOME))
		return kFieldFamily_Home;
	else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_WORK))
		return kFieldFamily_Corp;
	else
		return kFieldFamily_Other;
}

/***********************************************************************
 *
 * FUNCTION:		PrvTransferSetGoToParams
 *
 * DESCRIPTION:	Store the information necessary to navigate to the
 *                record inserted into the launch code's parameter block.
 *
 * PARAMETERS:		dbP        - pointer to the database to add the record to
 *					exgSocketP - parameter block passed with the launch code
 *					uniqueID   - unique id of the record inserted
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		10/17/97	Created
 *
 ***********************************************************************/
void PrvTransferSetGoToParams (DmOpenRef dbP, uint32_t cursorID, ExgSocketPtr exgSocketP, uint32_t uniqueID)
{
	uint32_t	recordNum;
	LocalID 	dbID;

	if (uniqueID == dbInvalidRowID)
		return;

	DmGetOpenInfo (dbP, &dbID, NULL, NULL, NULL);

	// The this the the first record inserted, save the information
	// necessary to navigate to the record.
	if (! exgSocketP->goToParams.uniqueID)
	{
		if (cursorID != dbInvalidCursorID)
			DbCursorGetPositionForRowID(cursorID, uniqueID, &recordNum);
		else
			recordNum = 0;

		exgSocketP->goToCreator = sysFileCAddress;
		exgSocketP->goToParams.uniqueID = uniqueID;
		exgSocketP->goToParams.dbID = dbID;
		exgSocketP->goToParams.recordNum = recordNum;
	}

	// If we already have a record then make sure the record index
	// is still correct.  Don't update the index if the record is not
	// in your the app's database.
	else if (dbID == exgSocketP->goToParams.dbID)
	{
		if (cursorID != dbInvalidCursorID)
			DbCursorGetPositionForRowID(cursorID, exgSocketP->goToParams.uniqueID, &recordNum);
		else
			recordNum = 0;

		exgSocketP->goToParams.recordNum = recordNum;
	}
}

/************************************************************
 *
 * FUNCTION: PrvTransfertImportUnknownData
 *
 * DESCRIPTION: Import unknown column if exist.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			reader	- pointer to the PDI reader addressing the data
 			rowIDP ID of the record created or to used to save the data
 *
 * RETURNS: error ONLY on database write function.
 *
 * REVISION HISTORY:
 *
 * Name	Date		Description
 * ----	----------	---------------
 * LFe	02/19/2004	Initial release
 *
 *************************************************************/
status_t PrvTransfertImportUnknownData(DmOpenRef dbP, PdiReaderType* reader, uint32_t *rowIDP)
{
	status_t				err = errNone;
	uint32_t				columnID = kInvalidColumnID;
	char* 					tmpStr = NULL;
	DbSchemaColumnType		columnType = (DbSchemaColumnType)0xFF;
	DbSchemaColumnType		dbColumnType;
	DbSchemaColumnDefnType*	columnDefnsP = NULL;
	DbSchemaColumnValueType	colVals;

	// Get the Column ID & Column Type information
	do
	{
		switch (reader->parameter)
		{
			case kPdiPAN_X_PALM_COLID:		columnID = StrAToI(reader->parameterValue);							break;
			case kPdiPAN_X_PALM_COLTYPE:	columnType = (DbSchemaColumnType) StrAToI(reader->parameterValue);	break;
		}
	}
	while (PdiReadParameter(reader) >= errNone);

	// If parameters are missing, return.
	if ((columnID == kInvalidColumnID) || (columnType == (DbSchemaColumnType)0xFF))
		return errNone;

	// Check if the column exists and get its definition
	// If the column doesn't exist, return.
  	if (DbGetColumnDefinitions(dbP, kAddressDefaultTableName, 1, &columnID, &columnDefnsP) < errNone)
		return errNone;

	dbColumnType = columnDefnsP->type;
	DbReleaseStorage(dbP, columnDefnsP);

	// Check if the column type are identical. If not, return.
	if (dbColumnType != columnType)
		return errNone;

	// Read the property value
	PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);

	memset(&colVals, 0, sizeof(DbSchemaColumnValueType));
	colVals.columnID = columnID;
	colVals.data = tmpStr;
	colVals.dataSize = reader->written;

	// The PDI library add an extra 0 for data type.
	if (((columnType != dbVarChar) && (columnType != dbChar)) || (columnType & dbVector))
		colVals.dataSize--;

	// rowID passed. Write data at that place.
	if (*rowIDP != dbInvalidRowID && gCursorID != dbInvalidCursorID && DbCursorMoveToRowID(gCursorID, *rowIDP) >= errNone)
	{
		err = DbWriteColumnValues(dbP, gCursorID, 1, &colVals);
	}
	else // insert the new record
	{
		if ((err = DbInsertRow(dbP, kAddressDefaultTableName, 1, &colVals, rowIDP)) >= errNone)
			err = AddressDBOpenCursor();
	}

	if (tmpStr)
		MemPtrFree(tmpStr);

	return err;
}

/************************************************************
 *
 * FUNCTION: TransferImportVCard
 *
 * DESCRIPTION: Import a VCard record.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			reader	- pointer to the PDI reader addressing the data
 *
 * RETURNS: true if the input was read
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rsf		4/24/97		Initial Revision
 *			bhall	8/12/99		moved category beaming code from gromit codeline
 *          ABa     6/20/00  	Integrate Pdi library
 *
 *************************************************************/
Boolean TransferImportVCard(DmOpenRef dbP, uint32_t cursorID, PdiReaderType* reader, Boolean setGotoParams, uint32_t *rowIDP, status_t *errorP, char** ffnPP)
{
	Boolean		inputSuccessFullyRead = false;
	uint16_t 	property = 0;
	uint32_t 	family, field, colsToWrite, colIndex, fieldInfo, parameterIndex;
	uint32_t	prefPhoneColID, prefEMailColID, prefIMColID, prefColID;
	char * 		tmpStr = NULL;
	char * 		assistantName = NULL;
	char * 		freeFormName = NULL;
	char * 		nickName = NULL;
	char *		englishName = NULL;
	char * 		companyName = NULL;
	char * 		companyTitle = NULL;
	char * 		companyProfession = NULL;
	char * 		note = NULL;
	char * 		anniv = NULL;
	char * 		palmOSVersionStr = NULL;
	char * 		bDay = NULL;
	char * 		yomiCompany = NULL;
	char * 		yomiFirstName = NULL;
	char * 		yomiLastName = NULL;
	DateType 	annivDate;
	DateType 	bDayDate;
	VCardSenderKindType vCardSender;
	DbSchemaColumnValueType * colVals = NULL;
	Boolean 	preferredProp, businessCard;
	Boolean		prefPhoneOrEMailAssigned;
	status_t	err;
	uint32_t	rowID = dbInvalidRowID;
	Boolean		forceFamilyCorp = false; // used for kPdiPRN_X_MICROSOFT_ASST_TEL

	if (rowIDP)
		rowID = *rowIDP;

	// Initializations
	colsToWrite = 0;
	*errorP = errNone;

	ImportPropertiesInit();

	PdiReadProperty(reader);

	// if not "BEGIN:VCARD"
	if (reader->property != kPdiPRN_BEGIN_VCARD)
	{
		if (reader->property != kPdiPRN_END_VCARD)
			*errorP = exgErrBadData;
		goto FreeAndExit;
	}

	PdiEnterObject(reader);

	while (PdiReadPropertyName(reader) == 0 && (property = reader->property) != kPdiPRN_END_VCARD)
	{
		fieldInfo = 0 ;

		if (property == kPdiPRN_X_PALM_UKNCOL)
		{
			if ((*errorP = PrvTransfertImportUnknownData(dbP, reader, &rowID)) < errNone)
				break;
			else continue;
		}

		while (PdiReadParameter(reader) >= errNone)
			;

		parameterIndex = PrvTransferGetParameterIndex(reader);

		switch(property)
		{
			case kPdiPRN_N:
				// Create new property to store the name - No family here..
				ImportAddNewProperty(kFieldType_Name, 0, nNameFields, false);
				// read all Name fields
				for (field = 0; field < nNameFields; field++)
				{
					tmpStr = NULL;
					PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
					ImportAddAssignPropertyField(field, tmpStr, 0);
				}
				colsToWrite += nNameFields;
				break;

			case kPdiPRN_ADR:
				// Read family
				family = PrvTransferGetParameterFamily(reader);

				// Create new property to store the address
				ImportAddNewProperty(kFieldType_Address, family, nAddressFields, false);

				// read all Address fields
				for (field = 0; field < nAddressFields; field++)
				{
					tmpStr = NULL;
					PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
					ImportAddAssignPropertyField(field, tmpStr, 0);
				}
				colsToWrite += nAddressFields;
				break;

			case kPdiPRN_FN:
				PdiReadPropertyField(reader, &freeFormName, kPdiResizableBuffer, kPdiDefaultFields);

				// DOLATER: Temporary evil hack
				// FFN is not saved, nowehre, so it's not possible to use it after the vCard has been decoded.
				// Will see later how to save this information properly.
				if (ffnPP && !*ffnPP)
					*ffnPP = ToolsStrDup(freeFormName);
				break;

			case kPdiPRN_NICKNAME:
				PdiReadPropertyField(reader, &nickName, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_ORG:
				PdiReadPropertyField(reader, &companyName, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_TITLE:
				PdiReadPropertyField(reader, &companyTitle, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_ROLE:
				PdiReadPropertyField(reader, &companyProfession, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_NOTE:
                PdiDefineResizing( reader, kPdiDefaultBufferDeltaSize, noteViewMaxLength);
				PdiReadPropertyField(reader, &note, kPdiResizableBuffer, kPdiDefaultFields);
				PdiDefineResizing(reader, kPdiDefaultBufferDeltaSize, tableMaxTextItemSize);
				colsToWrite++;
				break;

			case kPdiPRN_X_MICROSOFT_ASSISTANT:
			case kPdiPRN_X_PALM_ASSISTANT:
				PdiReadPropertyField(reader, &assistantName, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_X_MICROSOFT_ASST_TEL:
				forceFamilyCorp = true;
			case kPdiPRN_X_PALM_ASST_TEL:
				fieldInfo |= kFieldType_Extended | kFieldKind_AssistantPhone ;
			case kPdiPRN_TEL:
				fieldInfo |= kFieldType_Phone ;

				// Read family
				fieldInfo |= PrvTransferGetParameterFamily(reader);
				if (forceFamilyCorp)
				{
					fieldInfo &= ~kFieldFamilyMask;
					fieldInfo |= kFieldFamily_Corp;
					forceFamilyCorp = false;
				}

				// read phone kind, set field info mask. Default value
				// is voice phone.
				if (! (fieldInfo & kFieldKind_AssistantPhone))
				{
					if (PdiParameterPairTest(reader, kPdiPAV_TYPE_CELL))
						fieldInfo |= kFieldKind_MobilePhone;
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_FAX))
						fieldInfo |= kFieldKind_FaxPhone;
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_PAGER))
						fieldInfo |= kFieldKind_PagerPhone;
					else if (! (fieldInfo & kFieldKindMask))
						fieldInfo |= kFieldKind_VoicePhone;
				}

				// if this is the "preferred" phone, save the pointer
				preferredProp = (Boolean) PdiParameterPairTest(reader, kPdiPAV_TYPE_PREF);

				// Create new property to store the address
				ImportAddNewProperty(kFieldType_Phone, fieldInfo, nPhoneFields, preferredProp);

				// read phone number
				tmpStr = NULL;
				PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
				ImportAddAssignPropertyField(0, tmpStr, 0);
				colsToWrite++;
				break;

			case kPdiPRN_EMAIL:
				fieldInfo |= kFieldType_Internet | kFieldKind_EmailInternet;

				// Read family
				fieldInfo |= PrvTransferGetParameterFamily(reader);

				// if this is the "preferred" phone, save the pointer
				preferredProp = (Boolean) PdiParameterPairTest(reader, kPdiPAV_TYPE_PREF);

				// Create new property to store the address
				ImportAddNewProperty(kFieldType_Internet, fieldInfo, nEmailFields, preferredProp);

				// read email address
				tmpStr = NULL;
				PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
				ImportAddAssignPropertyField(0, tmpStr, parameterIndex);
				colsToWrite++;
				break;

			case kPdiPRN_URL:
				fieldInfo |= kFieldType_Internet | kFieldKind_URLInternet;

				// Read family
				fieldInfo |= PrvTransferGetParameterFamily(reader);

				// Create new property to store the address
				ImportAddNewProperty(kFieldType_Internet, fieldInfo, nURLFields, false);

				// read URL
				tmpStr = NULL;
				PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
				ImportAddAssignPropertyField(0, tmpStr, 0);
				colsToWrite++;
				break;

			case kPdiPRN_X_PALM_CUSTOM:
				// Create new property to store the address
				ImportAddNewProperty(kFieldType_Extended, kFieldKind_CustomExt, nCustomFields, false);

				// read phone number
				tmpStr = NULL;
				PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
				ImportAddAssignPropertyField(0, tmpStr, reader->customFieldNumber);
				colsToWrite++;
				break;

			case kPdiPRN_BDAY:
				PdiReadPropertyField(reader, &bDay, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_ANNIVERSARY:
				PdiReadPropertyField(reader, &anniv, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			case kPdiPRN_SOUND:
				// Palm has overloaded the SOUND property to use it for 'yomi' field
				if (PdiParameterPairTest(reader, kPdiPAV_X_X_IRMC_ORG) || PdiParameterPairTest(reader, kPdiPAV_X_X_PALM_ORG))
				{	// yomi Company
					PdiReadPropertyField(reader, &yomiCompany, kPdiResizableBuffer, kPdiDefaultFields);
					colsToWrite++;
				}
				else if (PdiParameterPairTest(reader, kPdiPAV_X_X_PALM_N) || PdiParameterPairTest(reader, kPdiPAV_X_X_IRMC_N))
				{	// yomi Name
					PdiReadPropertyField(reader, &yomiLastName, kPdiResizableBuffer, kPdiSemicolonFields);
					if (yomiLastName)
						colsToWrite++;
					PdiReadPropertyField(reader, &yomiFirstName, kPdiResizableBuffer, kPdiDefaultFields);
					if (yomiFirstName)
						colsToWrite++;
				}
				break;

			case kPdiPRN_X_ICQ:
			case kPdiPRN_X_AIM:
			case kPdiPRN_X_MSN:
			case kPdiPRN_X_YAHOO:
			case kPdiPRN_X_JABBER:

				fieldInfo = kFieldType_Extended | kFieldType_InstantMessaging;

				switch(property)
				{
					case kPdiPRN_X_ICQ:		fieldInfo |= kFieldKind_ICQ_IM;		break;
					case kPdiPRN_X_AIM:		fieldInfo |= kFieldKind_AIM_IM;		break;
					case kPdiPRN_X_MSN:		fieldInfo |= kFieldKind_MSN_IM;		break;
					case kPdiPRN_X_YAHOO:	fieldInfo |= kFieldKind_Yahoo_IM;	break;
					case kPdiPRN_X_JABBER:	fieldInfo |= kFieldKind_Jabber_IM;	break;
					default: continue;
				}

				// Read family
				fieldInfo |= PrvTransferGetParameterFamily(reader);

				// if this is the "preferred" phone, save the pointer
				preferredProp = (Boolean) PdiParameterPairTest(reader, kPdiPAV_TYPE_PREF);

				// Create new property to store the address
				ImportAddNewProperty(kFieldType_InstantMessaging, fieldInfo, nIMFields, preferredProp);

				// read IM
				tmpStr = NULL;
				PdiReadPropertyField(reader, &tmpStr, kPdiResizableBuffer, kPdiDefaultFields);
				ImportAddAssignPropertyField(0, tmpStr, 0);
				colsToWrite++;
				break;

			case kPdiPRN_X_PALM_ENGLISH_N:
				PdiReadPropertyField(reader, &englishName, kPdiResizableBuffer, kPdiDefaultFields);
				colsToWrite++;
				break;

			// Unused TAG - for instance
			case kPdiPRN_X_PALM_CATEGORY:
			case kPdiPRN_X_PALM_RENAME:
			case kPdiPRN_LOGO:
			case kPdiPRN_PHOTO:
			case kPdiPRN_KEY:
				break;

			case kPdiPRN_X_PALM:
				// Read the emiting PalmOS device
				PdiReadPropertyField(reader, &palmOSVersionStr, kPdiResizableBuffer, kPdiNoFields);
				break;
		}
	} // end of while

	// Error. We must stop the import
	if (*errorP < errNone)
		goto FreeAndExit;

	// If the last property wasn't kPdiPRN_END_VCARD, we didn't get a whole
	// vcard and we should discard it.
	if( property != kPdiPRN_END_VCARD )
		goto FreeAndExit;

	// Compute the sender terminal type
	vCardSender = PrvVCardSenderType(palmOSVersionStr);

	// now create a new record
	// alloc colsToWrite + 1 (for "preferred" display phone) column values
	++colsToWrite ;
	colVals = (DbSchemaColumnValueType*) MemPtrNew(colsToWrite * sizeof(DbSchemaColumnValueType));
	memset(colVals, 0, colsToWrite * sizeof(DbSchemaColumnValueType));

	// Add the columns for name, address, phones, emails & custom
	colIndex = 0;

	// Save all name in record
	businessCard = PrvTransferImportName(colVals, colsToWrite, &colIndex, companyName, freeFormName);

	// Save all Addresses in record or Note
	PrvTransferImportAddresses(colVals, colsToWrite, &colIndex, businessCard, vCardSender, &note);

	// Save all Phones in record or Note
	prefPhoneColID = PrvTransferImportPhoneNumbers(colVals, colsToWrite, &colIndex, businessCard,  vCardSender, &note);

	// Save all Internet data in record or Note
	prefEMailColID = PrvTransferImportInternet(colVals, colsToWrite, &colIndex, businessCard, vCardSender, &note);

	// Save all Instant Messagerie data in record or Note
	prefIMColID = PrvTransferImportInstantMessaging(colVals, colsToWrite, &colIndex, businessCard, vCardSender, &note);

	// Save all Customs in record or Note
	PrvTransferImportCustomFields(colVals, colsToWrite, &colIndex, vCardSender, &note);

	// Save all Internet data in record or Note
	prefPhoneOrEMailAssigned = PrvTransferSetPrefField(colVals, colsToWrite, &colIndex, prefPhoneColID, prefEMailColID, prefIMColID, &prefColID);

	// other (non-indexed) fields
	if (assistantName && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDWorkAssistantName;
		colVals[colIndex].data = (DbSchemaColumnData*) assistantName;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(assistantName);
		colIndex++;
	}

	if (nickName && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDNickName;
		colVals[colIndex].data = (DbSchemaColumnData*) nickName;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(nickName);
		colIndex++;
	}

	if (englishName && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDEnglishName;
		colVals[colIndex].data = (DbSchemaColumnData*) englishName;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(englishName);
		colIndex++;
	}

	if (companyName && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDWorkCompany;
		colVals[colIndex].data = (DbSchemaColumnData*) companyName;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(companyName);
		colIndex++;
	}

	if (companyTitle && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDWorkTitle;
		colVals[colIndex].data = (DbSchemaColumnData*) companyTitle;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(companyTitle);
		colIndex++;
	}

	if (companyProfession && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDProfession;
		colVals[colIndex].data = (DbSchemaColumnData*) companyProfession;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(companyProfession);
		colIndex++;
	}

	if (bDay && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDBirthday;
		MatchDateTimeToken(bDay, &bDayDate, NULL, NULL);
		colVals[colIndex].data = (DbSchemaColumnData*) &bDayDate;
		colVals[colIndex].dataSize = sizeof(DateType);
		colIndex++;
	}

	if (anniv && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDAnniversary;
		MatchDateTimeToken(anniv, &annivDate, NULL, NULL);
		colVals[colIndex].data = (DbSchemaColumnData*) &annivDate;
		colVals[colIndex].dataSize = sizeof(DateType);
		colIndex++;
	}

	if (yomiCompany && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDYomiCompanyName;
		colVals[colIndex].data = (DbSchemaColumnData*) yomiCompany;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(yomiCompany);
		colIndex++;
	}

	if (yomiFirstName && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDYomiFirstName;
		colVals[colIndex].data = (DbSchemaColumnData*) yomiFirstName;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(yomiFirstName);
		colIndex++;
	}

	if (yomiLastName && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDYomiLastName;
		colVals[colIndex].data = (DbSchemaColumnData*) yomiLastName;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(yomiLastName);
		colIndex++;
	}

	if (note && (colIndex < colsToWrite))
	{
		colVals[colIndex].columnID = kAddrColumnIDNote;
		colVals[colIndex].data = (DbSchemaColumnData*) note;
		colVals[colIndex].dataSize = 1 + (uint32_t) strlen(note);
		colIndex++;
	}

	// Check the number of allocated columns is enough (it should always be !!!)
	// colIndex reflects the number of written columns, so (colIndex == colsToWrite) is ok
	ErrNonFatalDisplayIf(colIndex > colsToWrite, "Address Import, Not enough columns allocated.");

	// We dont find any data to save. Exit without creating an empty record.
	if (!colIndex)
	{
		// We can have read successfully the vCard, but don't find any data for us. So let's go.
		inputSuccessFullyRead = (Boolean)((reader->events & kPdiEOFEventMask) == 0);
		// return an error for instance.
		*errorP = exgErrBadData;
		goto FreeAndExit;
	}

	// rowID passed. Write data at that place.
	if (rowID != dbInvalidRowID && cursorID != dbInvalidCursorID && DbCursorMoveToRowID(cursorID, rowID) >= errNone)
	{
		*errorP = DbWriteColumnValues(dbP, cursorID, colIndex, colVals);
	}
	else // insert the new record
	{
		*errorP = DbInsertRow(dbP, kAddressDefaultTableName, colIndex, colVals, &rowID);
	}

	if (*errorP < errNone)
		goto FreeAndExit;

	if (reader && (ExgSocketPtr)(reader->appData))
	{
		// Set it's category.
		// If the category is All then set the category to unfiled.
		CategoryID	catID = 0;

		catID = ((ExgSocketPtr)(reader->appData))->appData;
		if (catID)
		{
			err = DbSetCategory(dbP, rowID, 1, &catID);
			ErrNonFatalDisplayIf(err < errNone, "Can't set row categories");
		}

		if (setGotoParams)
			PrvTransferSetGoToParams(dbP, cursorID, (ExgSocketPtr)(reader->appData), rowID);

		inputSuccessFullyRead = (Boolean)((reader->events & kPdiEOFEventMask) == 0);
	}

	if (!prefPhoneOrEMailAssigned)
		AddressDBCheckAndSetDisplayedPhoneColumn(dbP, rowID);

	if (rowIDP)
		*rowIDP = rowID;

FreeAndExit:

	// Free dynamic memory allocated by the PDI library
	if (freeFormName)
		MemPtrFree(freeFormName);
	if (nickName)
		MemPtrFree(nickName);
	if (englishName)
		MemPtrFree(englishName);
	if (companyName)
		MemPtrFree(companyName);
	if (companyTitle)
		MemPtrFree(companyTitle);
	if (companyProfession)
		MemPtrFree(companyProfession);
	if (note)
		MemPtrFree(note);
	if (assistantName)
		MemPtrFree(assistantName);
	if (bDay)
		MemPtrFree(bDay);
	if (anniv)
		MemPtrFree(anniv);
	if (palmOSVersionStr)
		MemPtrFree(palmOSVersionStr);

	ImportFreeAllPropertiesAndFields();

	// Free the columns
	if (colVals)
		MemPtrFree(colVals);

	// if no error we must inform caller to continue iff not EOF
	return inputSuccessFullyRead;
}


/************************************************************
 *
 * FUNCTION: TransferExportVCard
 *
 * DESCRIPTION: Export a record as a Imc VCard record
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
 *	HISTORY:
 *		08/06/97	rsf	Created by Roger Flores
 *		06/09/99	grant	Ensure that phone numbers labeled "other" aren't
 *							tagged as ";WORK" or ";HOME".
 *		08/12/99	bhall	moved category beaming code from gromit codeline
 *		10/30/99	kwk	Use TxtGetChar before calling TxtCharIsDigit.
 *         ABa     6/20/00  Integrate Pdi library
 *
 *		11/29/02	LYr	Rewrite to use new Schema-based DataMgr
 *
 *************************************************************/
status_t TransferExportVCard(DmOpenRef dbP, uint32_t recordID, PdiWriterType* writer, uint32_t transfertMask)
{
	int32_t							i, j, fieldIndex, index;
	uint32_t						nCols, familyIndex, prefColID, colSize;
	MemHandle						tmpHandle;
	char *							companyName = NULL;
	status_t						err = errNone;
	DbSchemaColumnValueType *		colVals = NULL;
	AddressTabColumnPropertiesType *colProps = NULL;
	char							tmpBuf[kTmpBufSize]; // for numbers and date-time tokens
	char * 							nameFields[nNameFields];
	char * 							yomiNames[nYomiNames];
	TransferAddressStructType **	addressesByFamilyP[nFamilyNumber];
	TransferAddressStructType *		currentAddressP;
	uint16_t						pdiProp;
	uint16_t						pdiParam;


	// Reset structures
	memset(nameFields, 0, sizeof(nameFields));
	memset(yomiNames, 0, sizeof(yomiNames));
	memset(addressesByFamilyP, 0, sizeof(addressesByFamilyP));

	// Get current record values
	err = DbGetAllColumnValues(dbP, recordID, &nCols, &colVals);
	if (err < errNone)
		goto FreeAndExit;

	// Get default phone column ID
	colSize = sizeof(uint32_t);
	err = DbCopyColumnValue(dbP, recordID, kAddrColumnIDDisplayedPhone, 0, &prefColID, &colSize);

	if (err < errNone || colSize == 0)
		prefColID = kInvalidColumnID;

	// Allocate slots for addresses
	for (i = kHomeFamilyIndex; i <= kOtherFamilyIndex; i++)
	{
		if (gAddressSlotsAvailable[i] > 0)
		{
			addressesByFamilyP[i] = (TransferAddressStructType **) MemPtrNew(gAddressSlotsAvailable[i] * sizeof(TransferAddressStructType *));

			for (j = 0; j < gAddressSlotsAvailable[i]; j++)
			{
				addressesByFamilyP[i][j] = (TransferAddressStructType *) MemPtrNew(sizeof(TransferAddressStructType));
				memset(addressesByFamilyP[i][j], 0, sizeof(TransferAddressStructType));
			}
		}
		else
			addressesByFamilyP[i] = NULL;
	}

	// Emit vCard initialization
	PdiWriteBeginObject(writer, kPdiPRN_BEGIN_VCARD);
	PdiWriteProperty(writer, kPdiPRN_VERSION);
	PdiWritePropertyValue(writer, kVObjectvCardVersion, kPdiWriteData);

	// Emit PalmOS version
	PdiWriteProperty(writer, kPdiPRN_X_PALM);
	PdiWritePropertyValue(writer, kVObjectPalmOSV6Version, kPdiWriteData);

	// Loop through record columns
	for (i = 0; i < (int32_t)nCols; i++)
	{
		// skip emission of any erroneous field
		if (	(colVals[i].errCode < errNone) ||
				((colProps = AddressTabFindPropertiesByColId(gBookInfo.tabs, gBookInfo.numTabs, colVals[i].columnID)) == NULL))
			continue;

		// Skip non tranferable columns
		if (colProps->notTransferable)
			continue;

		// emit note data identified by column ID if beaming ALL families
		if ((transfertMask & kFieldType_Binary) && (transfertMask & kFieldKind_Note) &&
			colVals[i].columnID == kAddrColumnIDNote)
		{
			PdiWriteProperty(writer, kPdiPRN_NOTE);
			PdiWritePropertyValue(writer, (char*) colVals[i].data, kPdiWriteMultiline);
			continue;
		}

		// If the family mask is not ALL and the Type of the field is not NAME (Name is mandatory)
		// and the family mask match the field family skip the field.
		// Yomi are part of the name, so send also Yomi
		if (((transfertMask & kFieldFamilyMask) != kFieldFamilyMask) &&
			!(colProps->fieldInfo & (transfertMask & kFieldFamilyMask)) &&
			!((colProps->fieldInfo & kFieldTypeMask & ~kFieldType_Yomi) == kFieldType_Name))
			continue;

		// emit data identified by column properties
		switch (colProps->fieldInfo & kFieldTypeMask)
		{
			case kFieldType_Name :
				// setup 5 Name fields for differed emission
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_LastName:	index = iLastName;		break;
					case kFieldKind_FirstName:	index = iFirstName;		break;
					case kFieldKind_MiddleName:	index = iMiddleName;	break;
					case kFieldKind_TitleName:	index = iTitleName;		break;
					case kFieldKind_SuffixName:	index = iSuffixName;	break;
					default: continue;	// Skip unknown name
				}

				nameFields[index] = (char*) colVals[i].data;
				// differ family and property value emission
				continue;

			case kFieldType_Address :
				familyIndex = familyIndexFromFieldInfo(colProps->fieldInfo);
				fieldIndex = colProps->fieldInfo & kFieldIndexMask;

				if (fieldIndex >= gAddressSlotsAvailable[familyIndex])
				{
					ErrNonFatalDisplay("invalid field index.");
					continue;
				}

				currentAddressP = addressesByFamilyP[familyIndex][fieldIndex];

				// At least, one field exists, notify it.
				if (colProps->fieldInfo & kFieldKindMask)
				{
					currentAddressP->addressAssigned = true;
					currentAddressP->family = colProps->fieldInfo & kFieldFamilyMask;
				}

				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_StreetAddress:	index = iStreetAddress;		break;
					case kFieldKind_CityAddress:	index = iCityAddress;		break;
					case kFieldKind_StateAddress:	index = iStateAddress;		break;
					case kFieldKind_ZipCodeAddress:	index = iZipCodeAddress;	break;
					case kFieldKind_CountryAddress:	index = iCountryAddress;	break;
					default: continue;	// Skip unknown address
				}

				currentAddressP->addressFields[index]= (char*) colVals[i].data;

				// differ family and property value emission
				continue;

			case kFieldType_Extended | kFieldType_Name :
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_NickName:		pdiProp = kPdiPRN_NICKNAME;			break;
					case kFieldKind_AssitantName:	pdiProp = kPdiPRN_X_PALM_ASSISTANT;	break;
					case kFieldKind_EnglishName:	pdiProp = kPdiPRN_X_PALM_ENGLISH_N;	break;
					default:	continue;	// skip unknown extended name type
				}

				PdiWriteProperty(writer, pdiProp);
				break;

			case kFieldType_Extended | kFieldType_Phone :
				if (colProps->fieldInfo & kFieldKind_AssistantPhone)
					// emit assistant phone property
					PdiWriteProperty(writer, kPdiPRN_X_PALM_ASST_TEL);
				else
					continue ; // skip unknown extended phone type
				break;

			case kFieldType_Phone :
				// emit Phone field
				// first handle phone fields that have a property of their own
				// emit standard phone (TEL) property
				PdiWriteProperty(writer, kPdiPRN_TEL);

				// emit Type parameter
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_VoicePhone:		pdiParam = kPdiPAV_TYPE_VOICE;	break;
					case kFieldKind_MobilePhone:	pdiParam = kPdiPAV_TYPE_CELL;	break;
					case kFieldKind_FaxPhone:		pdiParam = kPdiPAV_TYPE_FAX;	break;
					case kFieldKind_PagerPhone:		pdiParam = kPdiPAV_TYPE_PAGER;	break;
					default:						pdiParam = 0;					break;
				}

				if (pdiParam)
					PdiWriteParameter(writer, pdiParam, false);

				break;

			case kFieldType_Internet :
				// emit Mail address or URL
				pdiParam = 0;

				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_EmailInternet:
						pdiProp = kPdiPRN_EMAIL;
						pdiParam = kPdiPAV_TYPE_INTERNET;
						break;

					case kFieldKind_URLInternet:
						pdiProp = kPdiPRN_URL;
						break;

					default:	continue;	// Skip unknwon internet field
				}

				PdiWriteProperty(writer, pdiProp);

				if (pdiParam)
					// internet mail
					PdiWriteParameter(writer, pdiParam, false);

				break;

			case kFieldType_Extended | kFieldType_InstantMessaging :
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_ICQ_IM:		pdiProp = kPdiPRN_X_ICQ;	break;
					case kFieldKind_AIM_IM:		pdiProp = kPdiPRN_X_AIM;	break;
					case kFieldKind_Yahoo_IM:	pdiProp = kPdiPRN_X_YAHOO;	break;
					case kFieldKind_MSN_IM:		pdiProp = kPdiPRN_X_MSN;	break;
					case kFieldKind_Jabber_IM:	pdiProp = kPdiPRN_X_JABBER;	break;
					default:	continue;	// Skip unknown Instant Messaging
				}

				PdiWriteProperty(writer, pdiProp);
				break;

			case kFieldType_Company :
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_NameCompany:
						pdiProp = kPdiPRN_ORG;
						// save company name in case we have no other name to display
						if (!companyName)
							companyName = (char*) colVals[i].data;
					break;
					case kFieldKind_TitleCompany:		pdiProp = kPdiPRN_TITLE;	break;
					case kFieldKind_ProfessionCompany:	pdiProp = kPdiPRN_ROLE;		break;
					default:	continue;	// Skip unknown Company
				}

				PdiWriteProperty(writer, pdiProp);
				break;

			case kFieldType_Date :
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_BirthdayDate:		pdiProp = kPdiPRN_BDAY;			break;
					case kFieldKind_AnniversaryDate:	pdiProp = kPdiPRN_ANNIVERSARY;	break;
					default: continue;	// unknown date, skip it
				}

				PdiWriteProperty(writer, pdiProp);
				GenerateDateTimeToken(tmpBuf, (DateType*) colVals[i].data, NULL);
				PdiWritePropertyValue(writer, tmpBuf, kPdiWriteData );
				// property value already written, go to next field
				continue;

			case kFieldType_Binary :
				// ####LYr_21 - 2002-11-29 - add binary fields in vCard
/*				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_PhotoBin :
					break;
					case kFieldKind_LogoBin :
					break;
					case kFieldKind_SoundBin :
					break;
					case kFieldKind_KeyBin :
					break;
				}
*/
				continue;

			case kFieldType_Extended :
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_CustomExt:	pdiProp = kPdiPRN_X_PALM_CUSTOM;	break;
					default:	continue;	// unknown extended kind
				}

				PdiWriteProperty(writer, pdiProp);
				break;

			case kFieldType_Yomi | kFieldType_Name:
			case kFieldType_Yomi | kFieldType_Company:
				/* WARNING */
				// The code below works because the 3 Kind const are differents
				// #### To be fixed
				switch (colProps->fieldInfo & kFieldKindMask)
				{
					case kFieldKind_FirstName:		index = kYomiFirst;		break;
					case kFieldKind_LastName:		index = kYomiLast;		break;
					case kFieldKind_NameCompany:	index = kYomiCompany;	break;
					default: continue;	// Skip unknown Yomi fields
				}

				yomiNames[index] = (char*) colVals[i].data ;
				// differ yomi names emission
				continue;

			default:
				// Send the data, depending of the column type
				PdiWriteProperty(writer, kPdiPRN_X_PALM_UKNCOL);

				// Send unknown column with the unknown tag
				StrIToA(tmpBuf, colProps->columnId);
				PdiWriteParameterStr(writer , "X-PALM-COLID", tmpBuf);
				StrIToA(tmpBuf, (int32_t)colProps->columnDataType);
				PdiWriteParameterStr(writer , "X-PALM-COLTYPE", tmpBuf);

				if (((colProps->columnDataType == dbVarChar) || (colProps->columnDataType == dbChar)) && !(colProps->columnDataType & dbVector))
					PdiWritePropertyValue(writer, (char*) colVals[i].data, kPdiWriteText);
				else
				{
					uint16_t	oldEncoding = writer->encoding;

					PdiSetEncoding(writer, kPdiB64Encoding);
					PdiWritePropertyBinaryValue(writer, colVals[i].data, (uint16_t)colVals[i].dataSize, kPdiWriteData);
					PdiSetEncoding(writer, oldEncoding);
				}

				continue;
		}

		// Emit the index
		// 2 cases:
		// a - It's a Custom field - Only send the index as parameter ;1
		// b - It's not a custom field - Send the index as a parameter extension: X-1...X-16
		if (colProps->fieldInfo & kFieldIndexMask)
		{
			uint16_t	pos = 0;

			if ((colProps->fieldInfo & (kFieldTypeMask | kFieldKindMask)) != (kFieldType_Extended | kFieldKind_CustomExt))
			{
				strcpy(tmpBuf,"X-");
				pos += 2;
			}

			StrIToA(tmpBuf + pos, (colProps->fieldInfo & kFieldIndexMask));
			PdiWriteParameterStr(writer, "", tmpBuf);
		}

		// if this is the "display phone" column ID, add the "PREF" parameter
		if (colVals[i].columnID == prefColID)
			PdiWriteParameter(writer, kPdiPAV_TYPE_PREF, false);

		// Emit family parameter only if it's either HOME or WORK
		switch (colProps->fieldInfo & kFieldFamilyMask)
		{
			case kFieldFamily_Home:
				PdiWriteParameter(writer, kPdiPAV_TYPE_HOME, false);
				break;
			case kFieldFamily_Corp:
				PdiWriteParameter(writer, kPdiPAV_TYPE_WORK, false);
				break;
		}

		// Emit property value string
		TraceOutput(TL(appErrorClass, "VCard prop value : %s\n", (char*) colVals[i].data));
		PdiWritePropertyValue(writer, (char*) colVals[i].data, kPdiWriteText );
	}

	// Now emit name
	PdiWriteProperty(writer, kPdiPRN_N);

	// If there is a name field, send it
	for (i = 0; i < nNameFields; ++i)
		if(nameFields[i])
			break;

	if (i < nNameFields)
		// emit all 5 name fields
		PdiWritePropertyFields(writer, nameFields, nNameFields, kPdiWriteText);
	// otherwise try company name
	else if (companyName)
		PdiWritePropertyValue(writer, companyName, kPdiWriteText );
	// if it all failed, get the unnamed string resource
	else
	{
		tmpHandle = DmGetResource(gApplicationDbP, strRsc, UnnamedRecordStr);
		PdiWritePropertyValue(writer, (char *) DmHandleLock(tmpHandle), kPdiWriteText);
		DmHandleUnlock(tmpHandle);
		DmReleaseResource(tmpHandle);
	}

	// Emit freeFormName if available
	if ((tmpHandle = AddrFreeFormNameBuild(dbP, recordID)) != NULL)
	{
		PdiWriteProperty(writer, kPdiPRN_FN);
		PdiWritePropertyValue(writer, (char *) MemHandleLock(tmpHandle), kPdiWriteText);
		MemHandleUnlock(tmpHandle);
		MemHandleFree (tmpHandle);
	}

	// Emit yomi names if available
	if (yomiNames[kYomiFirst] || yomiNames[kYomiLast])
	{
		// write first name, last name as 2 semi-colon delimited fields for pre 6.0 compatibility
	   	PdiWritePropertyStr(writer, "SOUND", kPdiSemicolonFields, 2);
//		PdiWriteProperty(writer, kPdiPRN_SOUND);
		PdiWriteParameter(writer, kPdiPAV_X_X_IRMC_N, false);
		PdiWriteParameter(writer, kPdiPAV_X_X_PALM_N, false);
		PdiWritePropertyFields(writer, yomiNames, 2, kPdiWriteText);
	}

	if (yomiNames[kYomiCompany])
	{
		PdiWriteProperty(writer, kPdiPRN_SOUND);
		PdiWriteParameter(writer, kPdiPAV_X_X_IRMC_ORG, false);
		PdiWriteParameter(writer, kPdiPAV_X_X_PALM_ORG, false);
		PdiWritePropertyValue(writer, (char*) yomiNames[kYomiCompany], kPdiWriteText );
	}

	// Emit addresses
	for (i = kOtherFamilyIndex; i >= kHomeFamilyIndex; i--)
	{
		for(j = 0; j < (int32_t)gAddressSlotsAvailable[i]; j++)
		{
			currentAddressP = addressesByFamilyP[i][j];

			if (currentAddressP->addressAssigned)
			{
				PdiWriteProperty(writer, kPdiPRN_ADR);
				// Emit family parameter only if it's either HOME or WORK
				switch (currentAddressP->family)
				{
					case kFieldFamily_Home:	PdiWriteParameter(writer, kPdiPAV_TYPE_HOME, false);	break;
					case kFieldFamily_Corp:	PdiWriteParameter(writer, kPdiPAV_TYPE_WORK, false);	break;
				}

				// Emit DOM parameter if country is not set
				if (!currentAddressP->addressFields[iCountryAddress])
					PdiWriteParameter(writer, kPdiPAV_TYPE_DOM, false);

				// Emit all address fields
				PdiWritePropertyFields(writer, currentAddressP->addressFields, nAddressFields, kPdiWriteText);
			}
		}
	}

	// Emit vCard end
	PdiWriteEndObject(writer, kPdiPRN_END_VCARD);

	// Set error value
	err = (writer->errorLowWord == errNone) ? errNone : ((status_t) writer->errorLowWord | pdiErrorClass);

FreeAndExit:
	// Free column data
	DbReleaseStorage(dbP, colVals);

	// Free allocated memory for the addresses
	for (i = kHomeFamilyIndex; i <= kOtherFamilyIndex; i++)
	{
		if (addressesByFamilyP[i])
		{
			for (j = 0; j<gAddressSlotsAvailable[i]; j++)
				MemPtrFree(addressesByFamilyP[i][j]);

			MemPtrFree(addressesByFamilyP[i]);
		}
	}

	return err;
}

/************************************************************
 *
 * FUNCTION: 	TransferPreview
 *
 * DESCRIPTION: Create a short string preview of the data coming in.
 *
 * PARAMETERS:	infoP:	the preview info from the command parameter block
 *						of the sysAppLaunchCmdExgPreview launch
 *
 * RETURNS:		nothing
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		06/11/04	Initial revision
 *
 *************************************************************/
 void TransferPreview(ExgPreviewInfoType *infoP)
{
	DmOpenRef	previewDbRef = NULL;
	MemHandle	resH;
	char*		descP;
	uint32_t	cursorID = dbInvalidCursorID;
	uint32_t	rowID;
	uint32_t	size;
	void *		colDataP;
	char*		ffnStrP = NULL;


	// RULES:
	//
	// Depending of the opCode, this function will return different results.
	// The results could be one of the following:
	// - Free Form Name
	// - Display Name
	// - NickName
	// - 'Unnamed' localized string
	// - 'Business Cards' localized string
	//
	// Whatever the opCode is, if the socket contains a description, it will be return as result.
	//
	// If the vObject contains more than one vCard, it will return the resource string ExgMultipleDescriptionStr
	// tSTR ID 1008 ('Business Cards')
	//
	// Those results are locale dependent. For example, the English Name field is a part of the FFN and Display Name in zhCN.
	// So to be able to build the Display Name, we need, first, to load the resource that contain the list of column that are
	// part of the Display Name. When the function will decode the vCard, it will save all the parameters that match those
	// column ID.
	//
	// - exgPreviewDraw: Not supported. Will return an error.
	// - exgPreviewDialog: Not supported. Will return an error.
	//
	// - exgPreviewShortString:
	//		The function will return the 'Display Name' if the N parameter exists in the vCard (Mandatory field,
	//		but who know). If the Display Name could not be returned, the function will then return the FFN based
	//		ONLY on the FN parameter. If the N parameters doesn't exist, we won't be able to build a FFN. If the
	//		FN parameter doesn't exist, then the function will return the NickName. It if doesn't exist also, the
	//		function will return the 'Unnamed' string.
	//
	// - exgPreviewLongString:
	//		The function will return the FFN and if it doesn't exist (FN parameter not available in the vCard)
	//		the function will try to build the FFN using the N parameter and the FFN Template Layout.
	//		If it failed, it will try to return the NickName. If the parameter doesn't exist, it will then
	//		return the 'Unnamed' string.

	infoP->error = errNone;

	if (infoP->op == exgPreviewQuery)
	{
		infoP->types = exgPreviewShortString /* | exgPreviewLongString*/ ;
		return;
	}

	if (infoP->op != exgPreviewShortString && infoP->op != exgPreviewLongString)
	{
		infoP->error = exgErrNotSupported;
		return;
	}

	// If no allocated buffer, no need to decode th vCard
	if (!infoP->size)
	{
		infoP->error = exgErrBadParam;
		return;
	}

	infoP->string[0] = chrNull;

	// if we have a description we don't have to parse the vObject
	if (infoP->socketP->description && *infoP->socketP->description)
	{
		StrLCopy(infoP->string, infoP->socketP->description, (int16_t)infoP->size);
		return;
	}

	AddressDBOpenPreviewDatabase(gAddrDB, &previewDbRef);

	if ((infoP->error = DbCursorOpen(previewDbRef, kAddressDefaultTableName, DbMatchAny, &cursorID)) < errNone)
		goto Exit;

	if (DbCursorGetRowCount(cursorID) > 0)
		DbCursorRemoveAllRows(cursorID);

	if ((infoP->error = TransferReceiveData(previewDbRef, dbInvalidCursorID, infoP->socketP, true, false, &ffnStrP)) < errNone)
		goto Exit;

	if ((infoP->error = DbCursorRequery(cursorID)) < errNone)
		goto Exit;

	switch (DbCursorGetRowCount(cursorID))
	{
		case 0:
			// No vCard decoded.
			infoP->error = exgErrBadData;
			break;

		case 1:
			DbCursorMoveFirst(cursorID);
			DbCursorGetCurrentRowID(cursorID, &rowID);

			// 1 vCard: Return the right description
			switch (infoP->op)
			{
				/*
				case exgPreviewShortString:
					if ((descP = ToolsGetRecordNameStr(previewDbRef, rowID, stdFont, 0)) != NULL)
					{
						StrLCopy(infoP->string, descP, (int16_t)infoP->size);
						MemPtrFree(descP);
					}
					break;
*/
				case exgPreviewShortString:
				case exgPreviewLongString:
					// First, check if the FFN property exist in the vCard
					if (ffnStrP && *ffnStrP)
					{
						StrLCopy(infoP->string, ffnStrP, (int16_t)infoP->size);
					}
					else if ((resH = AddrFreeFormNameBuild(previewDbRef, rowID)) != NULL)
					{
						// Try to build the FFN based on the Name.
						descP = MemHandleLock(resH);
						StrLCopy(infoP->string, descP, (int16_t)infoP->size);
						MemHandleUnlock(resH);
						AddrFreeFormNameFree(resH);
					}
					break;
			}

			// Previous function already return the Unnamed string if nothing has been found.
			// So if the record doesn't contains a Nickname, do nothing, otherwise return it instead.
			if (DbGetColumnValue(previewDbRef, rowID, kAddrColumnIDNickName, 0, &colDataP, &size) >= errNone)
			{
				if (gUnnamedRecordStringP && !strcmp(infoP->string, gUnnamedRecordStringP) && size)
					StrLCopy(infoP->string, colDataP, (int16_t)infoP->size);

				DbReleaseStorage(previewDbRef, colDataP);
			}
			break;

		default:

			// More than one vCard. Return the defined resource string.
			resH = DmGetResource(gApplicationDbP, strRsc, ExgMultipleDescriptionStr);
			descP = DmHandleLock(resH);
			StrLCopy(infoP->string, descP, (int16_t)infoP->size);
			DmHandleUnlock(resH);
			DmReleaseResource(resH);
			break;
	}

Exit:

	if (ffnStrP)
		MemPtrFree(ffnStrP);

	// Delete all row
	if (cursorID != dbInvalidCursorID)
	{
		DbCursorRemoveAllRows(cursorID);
		DbCursorClose(cursorID);
	}

	if (previewDbRef)
		DbCloseDatabase(previewDbRef);

	return;
}
