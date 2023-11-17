/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PhoneBookIOLib.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             	Main source file for PhoneBookIOLib : a library for Import 
 *				/ Export of the address book with a phone.
 *
 *****************************************************************************/

// System includes
#include <SystemMgr.h>
#include <ErrorMgr.h>
#include <ExgMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <TextMgr.h>
#include <PdiLib.h>
#include <TelephonyLib.h>
#include <Control.h>
#include <List.h>
#include <Loader.h>
#include <FeatureMgr.h>
#include <NotifyMgr.h>

#include <stdio.h>
#include <string.h>

// PhoneBoookIO includes
#include "PhoneBookIOLibPrv.h"
#include "PhoneBookIOLibRes.h"
#include "PBkIOTransferPrv.h"
#include "PBkIOProgressPrv.h"

// *******************************************************************************
//	Defines
// *******************************************************************************

// Internal temporary file used to store vCards sent from Address Book.
// Also used to store vCards created locally before sending them to the address book.
#define kPhoneBookIOTmpFileName					"ExgPhoneBkFile"
// Max size of progress messages added to the progress dialog
#define kProgressMessageSize					10
// UDA temporary buffer size (used to store vCards before flushing in file stream)
#define kUDABufferSize							512

#define kVObjectPalmOSV6Version					"6.1"
#define kVObjectvCardVersion					"2.1"

// *******************************************************************************
//	Libray globals
// *******************************************************************************

PhoneBkIOLibGlobalsType gPhoneBookIOGbl;
DmOpenRef				gApplicationDbP = 0;

 /************************************************************
 *
 * Function:	PrvExgPhoneBookIOLibInstall
 *
 * Description: Performs some initialization like registering
 *				our schemes with the exchange manager.
 *
 * Parameters:	None
 *
 * Returned:	0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static status_t PrvExgPhoneBookIOLibInstall(void)
{
	char 		title[exgTitleBufferSize + 1];
	status_t 	err = errNone;
	MemHandle 	resourceH;
	char 		*resourceP;
	SysModuleInfoType	moduleInfo;

	if ((err = SysGetModuleInfo(sysFileTLibrary, sysFileCTelMgrLib, 0, &moduleInfo)) < errNone)
		return err;

	resourceH = DmGetResource(gApplicationDbP, strRsc, PhoneBkIOlTitleString);
	resourceP = MemHandleLock(resourceH);
	strncpy(title, resourceP, exgTitleBufferSize);
	title[exgTitleBufferSize] = chrNull;
	MemHandleUnlock(resourceH);

	return ExgRegisterDatatype(sysFileCPhoneBookIOLib, exgRegSchemeID, exgPhoneBookIOScheme, title, 0 /*flags*/);
}	



/************************************************************
 *
 *  FUNCTION: PrvCleanUp
 *
 *  DESCRIPTION: Cleans up after exg transactions. Closes file.
 *		This should be called whenever an exg transaction is complete.
 *
 *  PARAMETERS:
 *				-> gP - pointer library globals
 *				-> socketP - pointer to the exgSocket
 *
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static status_t PrvCleanUp(PhoneBkIOLibGlobalsType *gP, ExgSocketType *socketP)
{
	status_t err = errNone;
	
	if (socketP && socketP->socketRef)
	{
		PhoneBkIOSocketInfoType *socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;
		
		switch (socketInfoP->op)
		{
			case kPhoneBkIOOpNone:
				// No operation in progress.
				break;
			
			case kPhoneBkIOOpPut:
			case kPhoneBkIOOpGet:
				if (socketInfoP->tempFileH)
					err = FileClose(socketInfoP->tempFileH);
				if (err >= errNone)
				{
					socketInfoP->tempFileH = NULL;
					// Sender deletes the file.
					err = FileDelete(kPhoneBookIOTmpFileName, sysFileCPhoneBookIOLib);
				}
				break;
		}
		
		// Don't free until until the end of the outer call to ExgDisconnect
		if (! socketP->preview)
		{
			MemPtrFree(socketInfoP);
			socketP->socketRef = 0;
		}
	}
	
	// Close progress dialog.
	PBkIOProgressStopDialog(gP);

	return err;
}



/************************************************************
 *
 * FUNCTION:	PrvClearPhoneRecord
 *
 * DESCRIPTION:	Clear all fields of a record before assigning
 *				a new one.
 *
 * PARAMETERS:	
 *			-> phoneRecordP : pointer on the record to clear
 * 
 *  RETURNS: Nothing.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static void PrvClearPhoneRecord(AddressPhoneRecordType * phoneRecordP)
{
	memset(phoneRecordP, 0, sizeof(AddressPhoneRecordType));
}


/************************************************************
 *
 * FUNCTION:	PrvTransferDataToPhone
 *
 * DESCRIPTION:	Utility routine sending all received vCards to
 *				the phone.
 *
 * PARAMETERS:	-> socketP	- the socket pointer
 * 
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static status_t PrvTransferDataToPhone(ExgSocketType *socketP)
{
	status_t					err = errNone;
	PhoneBkIOLibGlobalsType *	gP = &gPhoneBookIOGbl;
	PhoneBkIOSocketInfoType *	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;
	PdiReaderType*				readerP = NULL;
	UDAReaderType*				streamP = NULL;
	Boolean						nextObject;
	AddressPhoneRecordType		phoneRecord;
	uint16_t					recPhoneNbIndex, phoneIndex;
	char *						lastNameP = phoneRecord.lastNameP;
	char *						firstNameP = phoneRecord.firstNameP;
	char *						companyP = phoneRecord.companyP;
	char *						titleP = phoneRecord.titleP;
	char *						phoneNbP;
	AddressPhoneBookInfoType	phoneBookInfo;
	Boolean						allPhoneEntriesHandled;
	char						prgMessage[kProgressMessageSize];
	
	// Initialize the telephony
	if ((err = PBkIOTransferTelInitialize(gP, socketP)) < errNone)
		goto Exit;

	// Update the progress
	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Progress to sending..."));
	gP->curStage = kPhoneBkIOConStageSending;
	PBkIOProgressUpdateDialog(gP, err, socketP->description);

	// Get the phoneBook info
	if ((err = PBkIOTransferGetPhoneBookInfo(gP, &phoneBookInfo)) < errNone)
		goto Exit;
	
	// Extract vCard information
	if ((err = PdiLibOpen()) < errNone)
		goto Exit;

	// Rewind the file
	FileRewind(socketInfoP->tempFileH);
	streamP = UDAFileStreamReaderNew(socketInfoP->tempFileH);
	readerP = PdiReaderNew(streamP, kPdiOpenParser);

	if (readerP == NULL)
		goto Exit;

	phoneIndex = 0;

	// Update the stage
	gP->curStage = kPhoneBkIOConStageSendvCard;

	// Read first property
	PdiReadProperty(readerP);

	do 
	{
		if  (readerP->property != kPdiPRN_BEGIN_VCARD) 
		{
			TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: TransferDataToPhone : PARSE Error"));
			goto Exit;
		}

		PdiEnterObject(readerP); 

		nextObject = false;
		// Clear the record
		PrvClearPhoneRecord(&phoneRecord);		
		recPhoneNbIndex = 0;
		
		while (!nextObject && (PdiReadProperty(readerP) == 0))
		{
			switch (readerP->property)
			{
				case kPdiPRN_BEGIN_VCARD:
					nextObject = true;
					break;

				case kPdiPRN_N:
					PdiReadPropertyField(readerP, &lastNameP, kTelPhoneNameMaxLen, kPdiDefaultFields);
					PdiReadPropertyField(readerP, &firstNameP, kTelPhoneNameMaxLen, kPdiDefaultFields);
					break;

				case kPdiPRN_ORG:
					PdiReadPropertyField(readerP, &companyP, kTelPhoneNameMaxLen, kPdiConvertSemicolon);
					break;

				case kPdiPRN_TITLE:
					PdiReadPropertyField(readerP, &titleP, kTelPhoneNameMaxLen, kPdiConvertSemicolon);
					break;

				case kPdiPRN_TEL:
					if (recPhoneNbIndex >= kPhoneBookIOMaxPhoneNumber)
						break;

					phoneNbP = phoneRecord.phoneNbP[recPhoneNbIndex];
					PdiReadPropertyField(readerP, &phoneNbP, kTelPhoneNumberMaxLen, kPdiNoFields);
					recPhoneNbIndex++;
					break;
					
				case kPdiPRN_END_VCARD:
					// Add this entry to the phone
					do 
					{
						sprintf(prgMessage,"%d", phoneIndex + 1); // PhoneIndex is zero based !
						PBkIOProgressUpdateDialog(gP, err, prgMessage);

						err = PBkIOTransferAddRecordToPhone(gP, &phoneRecord, phoneIndex, &allPhoneEntriesHandled);
						if (err == phoneBkIOErrEmptyRec) 
							// Just skipp this record
						err = errNone;
						else
							phoneIndex++;
					}
					while ((!(err < errNone)) && (!allPhoneEntriesHandled));
					break;

				// ignore other properties
			}
		}

		if ( (phoneBookInfo.phoneBkTotalSlots != 0)
			 && (phoneIndex > (uint16_t)(phoneBookInfo.phoneBkTotalSlots)) )
			 break;
	}
	while ( (err >= errNone) && (! (readerP->events & kPdiEOFEventMask)) );

	// If the total phone book slots have not been reached, erase remaining slots
	if (!(err < errNone))
	{
		uint16_t i;
		
		gP->curStage = kPhoneBkIOConStageDeleting;
		PBkIOProgressUpdateDialog(gP, err, NULL);

		// Update phone book information
		if ((err = PBkIOTransferGetPhoneBookInfo(gP, &phoneBookInfo)) < errNone)
			goto Exit;

		// Try to avoid the delete loop by computing the number of assigned slots
		if ((uint16_t)(phoneBookInfo.phoneBkUsedSlots) > phoneIndex)
		{
			for (i = phoneIndex; i < (uint16_t)(phoneBookInfo.phoneBkTotalSlots); i++) 
			{
				if ((err = PBkIOProgressProcessEvents(gP, NULL)) < errNone)
					goto Exit;
			
				PBkIOTransferDeletePhoneRecord(gP, i);
			}
		}
	}

Exit:
	// clean up
	PrvClearPhoneRecord(&phoneRecord);

	if (readerP)
		PdiReaderDelete(&readerP);

	if (streamP)
		UDADelete(streamP);
	
	PdiLibClose();

	// Close the telephony
	PBkIOTransferTelFinalize(gP, socketP);
	
	return err;
}


/************************************************************
 *
 * FUNCTION:	PrvGetDataFromPhone
 *
 * DESCRIPTION:	Utility routine; It gets data from the phone
 *				and creates vCard with it.
 *
 * PARAMETERS:	-> socketP	- the socket
 * 
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static status_t PrvGetDataFromPhone(ExgSocketType *socketP)
{
	status_t 					err = errNone;
	PhoneBkIOLibGlobalsType *	gP = &gPhoneBookIOGbl;
	PhoneBkIOSocketInfoType *	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;
	FileHand					fileHandle = socketInfoP->tempFileH;
	PdiWriterType* 				writerP = NULL;
	UDAWriterType* 				mediaP = NULL;
	char* 						fields[1];		// First name only
	AddressPhoneRecordType 		phoneRecord;
	uint16_t					recordIndex;
	uint16_t 					readRecord = 0;
	char						prgMessage[kProgressMessageSize];
	AddressPhoneBookInfoType	phoneBookInfo;
	
	// Create UDA Object
	mediaP = UDAFileStreamWriterNew(fileHandle, kUDABufferSize);
	if (! mediaP) {
		err = exgMemError;
		goto Exit;
	}

	if ((err = PdiLibOpen()) < errNone)
		goto Exit;

	writerP = PdiWriterNew(mediaP, kPdiPalmCompatibility);

	// Update the progress
	gP->curStage = kPhoneBkIOConStageReceiving;
	PBkIOProgressUpdateDialog(gP, err, socketP->description);

	// Get the phoneBook info
	if ((err = PBkIOTransferGetPhoneBookInfo(gP, &phoneBookInfo)) < errNone)
		goto Exit;

	if (phoneBookInfo.phoneBkUsedSlots == 0)
	{
		// The phone book is empty, warn the user about it after closing the progress
		PBkIOProgressStopDialog(gP);
		err = phoneBkIOPhoneBookEmpty;
		goto Exit;
	}

	// Update the progress stage
	gP->curStage = kPhoneBkIOConStageRecvCard;
	sprintf(prgMessage,"0 on %d", phoneBookInfo.phoneBkUsedSlots);
	PBkIOProgressUpdateDialog(gP, err, prgMessage);
	
	// Loop on receiving phone entries
	for (recordIndex = 0; recordIndex < (uint16_t)phoneBookInfo.phoneBkTotalSlots; recordIndex++)
	{
		PrvClearPhoneRecord(&phoneRecord);

		err = PBkIOTransferReadRecordFromPhone(gP, &phoneRecord, recordIndex);
		if (err == telErrEntryNotFound)
		{
			// The record is empty, just skipp it !
			err = errNone;
			continue;
		}
		
		if (err < errNone)
			goto Exit;

		// Pre-increment to display 1 on 1st record
		sprintf(prgMessage,"%d on %d", ++readRecord , phoneBookInfo.phoneBkUsedSlots);
		PBkIOProgressUpdateDialog(gP, err, prgMessage);

		// Add Phone fields
		PdiWriteBeginObject(writerP, kPdiPRN_BEGIN_VCARD);
		PdiWriteProperty(writerP, kPdiPRN_VERSION);
		PdiWritePropertyValue(writerP, kVObjectvCardVersion, kPdiWriteData);

		// Emit PalmOS version
		PdiWriteProperty(writerP, kPdiPRN_X_PALM);
		PdiWritePropertyValue(writerP, kVObjectPalmOSV6Version, kPdiWriteData);

		// Add the name : only lastNameP is assigned
		PdiWriteProperty(writerP, kPdiPRN_N);
		fields[0] = phoneRecord.lastNameP;
		PdiWritePropertyFields(writerP, fields, 1, kPdiWriteText);
		
		// The organization & title are not set
		// Add the phone number - Only phoneRecord.phoneNbP[0] is assigned
		PdiWriteProperty(writerP, kPdiPRN_TEL);
		PdiWriteParameter(writerP, kPdiPAV_TYPE_PREF, false);
		PdiWriteParameter(writerP, kPdiPAV_TYPE_HOME, false);
		PdiWriteParameter(writerP, kPdiPAV_TYPE_VOICE, false);
		PdiWritePropertyValue(writerP, phoneRecord.phoneNbP[0], kPdiWriteText);						

		// Finish entry
		PdiWriteEndObject(writerP, kPdiPRN_END_VCARD);

		// Check if all records have been received
		if (readRecord == (uint16_t)phoneBookInfo.phoneBkUsedSlots) 
			break;		
	}
	
	// Now, add this vCard to the mediaP
	err = UDAWriterFlush(mediaP);

Exit:
	if (writerP)
		PdiWriterDelete(&writerP);

	PdiLibClose();

	if (mediaP)
		UDADelete(mediaP);

	return err;
}

/***********************************************************************
 *
 * Function:	ExgPhoneBookIOLibMain
 *
 * Description:	This is the main entry point for the plugin
 *
 * Parameters:	-> cmd			- word value specifying the launch code. 
 *              -> cmdPB		- pointer to a structure that is associated with the launch code. 
 *              -> launchFlags	-  word value providing extra information about the launch.
 *
 * Returned:    Result of launch
 *
 ***********************************************************************/
uint32_t ExgPhoneBookIOLibMain( uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	status_t	err = errNone;

	switch(cmd) 
	{
		case sysLaunchCmdInitialize:
			/* init is done on lib install. */
			memset(&gPhoneBookIOGbl, 0, sizeof(PhoneBkIOLibGlobalsType));
			SysGetModuleDatabase(SysGetRefNum(), NULL, &gApplicationDbP);
			break;
			
		case sysLaunchCmdBoot:
			{
				DatabaseID	databaseId;
			
				TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: sysLaunchCmdBoot"));


				PrvExgPhoneBookIOLibInstall();
	
				databaseId = DmFindDatabaseByTypeCreator(sysFileTExgLib, sysFileCPhoneBookIOLib, dmFindExtendedDB, NULL);
				if (databaseId == NULL)
				{
					err = DmGetLastErr();
					return err;
				}

				if ((err = SysNotifyRegister(databaseId, kTelTelephonyNotification, NULL, sysNotifyNormalPriority, NULL, 0)) != errNone)
				{
					TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: failed to register for telephony notifications"));
					return err;
				}
			}	
			break;

		case sysLaunchCmdFinalize:
			/* clean up is done on last remove. */
			break;
		
		case sysAppLaunchCmdNotify:
		{
			if (((SysNotifyParamType*)cmdPBP)->notifyType == kTelTelephonyNotification)
			{
				switch (((TelNotificationPtr)((SysNotifyParamType*) cmdPBP)->notifyDetailsP)->id)
				{
					case kTelPowLaunchCmdPhonebookReady:
						{
							EventType	event;

							TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: kTelPowLaunchCmdPhonebookReady"));
							err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrStorageState, kPhoneBookIOStorageReady);
							
							// Enqueue the kTelPrvPhoneBookReadyEvent
							event.eType = kTelPrvPhoneBookReadyEvent;
							EvtAddEventToQueue(&event);
						}	
						break;
					
					case kTelPowLaunchCmdPhonebookNotReady:
						{
							TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: kTelPowLaunchCmdPhonebookNotReady"));
							err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrStorageState, kPhoneBookIOStorageNotReady);
						}
						break;

					case kTelPowLaunchCmdConnectionOff:
						{
							TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: kTelPowLaunchCmdConnectionOff"));
							err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrConnectionState, kPhoneBookIOConnectionOff);
						}
						break;

					case kTelPowLaunchCmdConnectionOn:
						{
							EventType	event;

							TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: kTelPowLaunchCmdConnectionOn"));
							err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrConnectionState, kPhoneBookIOConnectionOn);
							
							// Enqueue the kTelPrvPhoneConnectionOnEvent
							event.eType = kTelPrvPhoneConnectionOnEvent;
							EvtAddEventToQueue(&event);
						}
						break;

					case kTelStyLaunchCmdAuthenticationCanceled:
						{
							EventType	event;

							TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLib: kTelStyLaunchCmdAuthenticationCanceled"));
							
							// Enqueue the kTelPrvAuthenticationCancelledEvent
							event.eType = kTelPrvAuthenticationCancelledEvent;
							EvtAddEventToQueue(&event);
						}
				}
			}
		}
		break;
	}

    return 0;
}

/************************************************************
 *
 * Function:	ExgPhoneBookIOLibOpen
 *
 * Description:	Initializes the Phone Book IO library
 *
 * Parameters:	None
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibOpen(void)
{
	status_t	err = errNone;

	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibOpen"));
	
	if (!gPhoneBookIOGbl.resourceDBP)
	{			
		// Open the library's database, so we can use it's resources.
		// Note that resources used here must not overlap with application resources!
		gPhoneBookIOGbl.resourceDBP = DmOpenDatabaseByTypeCreator(sysFileTExgLib, sysFileCPhoneBookIOLib, dmModeReadOnly);
		//gPhoneBookIOGbl.resourceDBP = DbOpenDatabaseByName(sysFileCPhoneBookIOLib, "PhoneBookIO Library", dmModeReadOnly, dbShareNone);

		if (! gPhoneBookIOGbl.resourceDBP)
		{
			ErrNonFatalDisplay("Error opening resource database");
			err = exgErrStackInit;
		}
	}
	
	gPhoneBookIOGbl.openCount++;
	
	return err;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibClose
 *
 * Description:	Shuts down  the Phone Book IO library
 *
 * Parameters:	None
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibClose(void)
{
	status_t	err = errNone;
	
	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibClose"));

	if (gPhoneBookIOGbl.openCount == 1)
	{
		// Close the application/library's database
		if (gPhoneBookIOGbl.resourceDBP)
		{	
			err = DmCloseDatabase(gPhoneBookIOGbl.resourceDBP);
			gPhoneBookIOGbl.resourceDBP = NULL;
			ErrNonFatalDisplayIf((err < errNone), "Error closing resource database");
		}
	}

	gPhoneBookIOGbl.openCount--;
	
	return err;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibSleep
 *
 * Description:	Called by the system before it goes to sleep
 *
 * Parameters:	nothing
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibSleep(void)
{
	return errNone;
}

/************************************************************
 *
 * Function:	ExgPhoneBookIOLibWake
 *
 * Description:	Called by the system before it wakes up
 *
 * Parameters:  nothing
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibWake(void)
{
	return errNone;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibHandleEvent
 *
 * Description: Called by the SysHandleEvent on event hook keys,
 *				and by the PhoneBookIO progress event handler.
 *
 * Parameters:	eventP	- pointer to the event
 *
 * Returned:	true if event was handled
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
Boolean ExgPhoneBookIOLibHandleEvent(void* eventP)
{
	// For now, skipp the events (do not handle them)
	// Additionnal event can be handled here if needed.
	return false;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibConnect
 *
 * Description:	Not supported by this library
 *
 * Parameters:	-> exgSocketP - pointer to the exgSocket
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibConnect(ExgSocketType* exgSocketP)
{
	return exgErrNotSupported;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibAccept
 *
 * Description:	Not supported by this library
 *
 * Parameters:	-> exgSocketP - pointer to the exgSocket
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibAccept(ExgSocketType* socketP)
{
	return exgErrNotSupported;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibDisconnect
 *
 * Description:	Standard ExgLibDisconnect routine - See PalmOS 
 *				reference document for description.
 * 
 *				Closes an Exchange session releasing all library
 *				memory.
 *
 * Parameters:
 *		-> socketP	- pointer to exg socket data
 *		-> appError - application error code
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibDisconnect(ExgSocketType* socketP, status_t appError)
{
	PhoneBkIOSocketInfoType *socketInfoP;
	status_t				err = appError;
	uint8_t					op = kPhoneBkIOOpNone;
	
	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibDisconnect"));

	ErrNonFatalDisplayIf(!socketP, "NULL socketP");
	
	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;
	
	// Grab the operation before we do anything that might change it
	if (socketInfoP)
		op = socketInfoP->op;
		
	if (!(err < errNone) && socketInfoP && socketInfoP->op == kPhoneBkIOOpPut && ! socketP->preview)
	{
		// Update the progress dialog
		gPhoneBookIOGbl.curStage = kPhoneBkIOConStageConnectingPhone;
		PBkIOProgressUpdateDialog(&gPhoneBookIOGbl, err, socketP->description);
		err = PrvTransferDataToPhone(socketP);
	}
	
	// Show error message in dialog.
	if (err < errNone) 
	{
		gPhoneBookIOGbl.curStage = kPhoneBkIOConStageDisconnecting;
		PBkIOProgressUpdateDialog(&gPhoneBookIOGbl, err, socketP->description);
	}
	
	// Store error from inner call to ExgDisconnect and return it to outer caller
	if (socketInfoP)
	{
		if (op == kPhoneBkIOOpPut && (socketInfoP->err < errNone))
			err = socketInfoP->err;
	}
		
	// Clean things up (not just on error)
	PrvCleanUp(&gPhoneBookIOGbl, socketP);

	ExgPhoneBookIOLibClose();	
	
	return err;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibPut
 *
 * Description:	Standard ExgLibPut routine - See PalmOS 
 *				reference document for description.
 *
 *   		Starts a Put operation. The socket data
 *		contains both the connection information and the info
 *		about the object to be sent. If a connection does not
 *		already exist, then this will establish one.
 *				
 * Parameters:	-> socketP - pointer to exg socket data
 *
 * Returned:	Error code, 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibPut(ExgSocketType* socketP)
{	
	status_t				err = errNone;
	PhoneBkIOLibGlobalsType *gP = &gPhoneBookIOGbl;
	PhoneBkIOSocketInfoType *socketInfoP;
	
	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibPut"));

	ErrNonFatalDisplayIf(!socketP, "NULL socketP");
	
	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;
	
	// This exchange library doesn't support multiple PUT's.
	if (socketInfoP && socketInfoP->tempFileH)
		return exgErrNotSupported;
	
	if (! socketInfoP)
	{
		// Allocate a struct to store the filename and operation.
		socketInfoP = MemPtrNew(sizeof(PhoneBkIOSocketInfoType));
		
		if (socketInfoP)
		{
			memset(socketInfoP, 0, sizeof(PhoneBkIOSocketInfoType));
			MemPtrSetOwner(socketInfoP, 0);
			socketP->socketRef = (uint32_t)socketInfoP;
			socketInfoP->tempFileH = NULL;
		}
		else
			return exgMemError;
	}

	if (! socketInfoP->op)
	{
		// only open for first PUT
		if ((err = ExgPhoneBookIOLibOpen()) < errNone)
			return err;
	}
		
	// Show dialog and setup callback for status text.
	PBkIOProgressStartDialog(gP, (Boolean) socketP->noStatus);

	// Update the progress dialog
	gP->curStage = kPhoneBkIOConStageSending;
	PBkIOProgressUpdateDialog(gP, err, socketP->description);

	// Open the file.
	socketP->localMode = true;
	socketInfoP->op = kPhoneBkIOOpPut;

	if (! socketInfoP->tempFileH)
		socketInfoP->tempFileH = FileOpen(kPhoneBookIOTmpFileName, 0, sysFileCPhoneBookIOLib, fileModeReadWrite, &err);
	
	// Clean up on error.
	if (err < errNone)
		PrvCleanUp(gP, socketP);
	else 
		err = PBkIOProgressProcessEvents(gP, NULL);

	if (err < errNone)
		ExgPhoneBookIOLibClose();
	
	return err;
}
	

/************************************************************
 *
 * Function:	ExgPhoneBookIOLibSend
 *
 * Description:	Standard ExgLibSend routine - See PalmOS 
 *				reference document for description.
 *
 * Parameters:	
 *		-> socketP 		- pointer to exg socket data
 *		-> bufP			- pointer on sending buffer
 *		-> bufferSize	- sent buffer size
 *		<- errP			- ptr on Error code, 0 if no error
 *
 * Returned:	The number of bytes handled by the exchange library.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
uint32_t ExgPhoneBookIOLibSend(ExgSocketType* socketP, void* bufP, const uint32_t bufferSize, 
	status_t* errP)
{
	PhoneBkIOLibGlobalsType *gP = &gPhoneBookIOGbl;
	uint32_t				charsWritten;
	PhoneBkIOSocketInfoType *socketInfoP;

	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibSend"));
	
	*errP = errNone;
	
	ErrNonFatalDisplayIf(!socketP, "NULL socketP");
	ErrNonFatalDisplayIf(!bufP, "NULL bufP");
	ErrNonFatalDisplayIf(!errP, "NULL errP");
	
	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;

	ErrNonFatalDisplayIf(!socketInfoP && socketInfoP->op != kPhoneBkIOOpPut, "Must call ExgPut first");
	ErrNonFatalDisplayIf(!socketInfoP->tempFileH,  "Temp file not open");
	
	if (gP->curStage != kPhoneBkIOConStageSending)	
	{
		gP->curStage = kPhoneBkIOConStageSending;	
		PBkIOProgressUpdateDialog(gP, *errP, socketP->description);
	}
	
	*errP = PBkIOProgressProcessEvents(gP, NULL);

	if (*errP == exgErrUserCancel)
		return 0;
	
	// Write the data to the file (all or nothing).
	charsWritten = FileWrite(socketInfoP->tempFileH, bufP, sizeof(char), bufferSize, errP);

	if (!(*errP < errNone) && charsWritten < bufferSize)
		*errP = exgErrDeviceFull;
	
	return charsWritten;
}


/************************************************************
 *
 * Function:	ExgPhoneBookIOLibGet
 *
 * Description:	Standard ExgLibGet routine - See PalmOS 
 *				reference document for description.
 *
 * Parameters:	
 *		-> socketP 	- pointer to exg socket data
 *
 * Returned:	0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibGet(ExgSocketType* socketP)
{
	PhoneBkIOLibGlobalsType *gP = &gPhoneBookIOGbl;
	status_t				err = errNone;
	PhoneBkIOSocketInfoType *socketInfoP;
	
	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibGet"));

	ErrNonFatalDisplayIf(!socketP, "NULL socketP");
	
	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;
	
	if (! socketInfoP)
	{
		// Allocate a struct to store the filename and operation.
		socketInfoP = MemPtrNew(sizeof(PhoneBkIOSocketInfoType));
		
		if (socketInfoP)
		{
			memset (socketInfoP, 0, sizeof(PhoneBkIOSocketInfoType));
			MemPtrSetOwner(socketInfoP, 0);
			socketP->socketRef = (uint32_t)socketInfoP;
			socketInfoP->tempFileH = NULL;
		}
		else
			return exgMemError;
	}
	
	if (! socketInfoP->op)
	{
		// only open for first GET
		if ((err = ExgPhoneBookIOLibOpen()) < errNone)
			return err;
	}

	// Create the file stream
	socketInfoP->tempFileH = FileOpen(kPhoneBookIOTmpFileName, 0, sysFileCPhoneBookIOLib, fileModeReadWrite, &err);

	if (! socketInfoP->tempFileH)
	{
		err = exgErrNotFound;
		goto ExitWithCleanUp;
	}

	// Set State	
	socketInfoP->op = kPhoneBkIOOpGet; // special Get State while on sending side

	// Initialize the telephony
	if ((err = PBkIOTransferTelInitialize(gP, socketP)) < errNone)
	{	
		if ((err != telErrResultUserCancel) && (err != exgErrUserCancel))
			TelUiManageError(err, NULL);

		goto ExitWithCleanUp;
	}

	// Now, get the data from the phone
	if ((err = PrvGetDataFromPhone(socketP)) < errNone)
	{
		if (err == phoneBkIOPhoneBookEmpty)
			FrmAlert (gApplicationDbP, PBkIOPhoneBookEmptyAlert);
		else if ((err != telErrResultUserCancel) && (err != exgErrUserCancel))
			TelUiManageError(err, NULL);
		goto DisconnectAndExit;
	}

	// Close our progress dialog in order to close the telephony
	PBkIOProgressStopDialog(gP);
	
	// Close the telephony
	if ((err = PBkIOTransferTelFinalize(gP, socketP)) < errNone)
		goto ExitWithCleanUp;
	
	// Rewind the file for next receive commands
	FileRewind(socketInfoP->tempFileH);

	// All is successfull
	return errNone;

DisconnectAndExit:
	PBkIOTransferTelFinalize(gP, socketP);
	
ExitWithCleanUp:	
	PrvCleanUp(gP, socketP);
	ExgPhoneBookIOLibClose();
	return err;
}

	
/************************************************************
 *
 * Function:	ExgPhoneBookIOLibReceive
 *
 * Description:	Standard ExgLibReceive routine - See PalmOS 
 *				reference document for description.
 *
 * Parameters:	
 *		-> socketP 		- pointer to exg socket data
 *		-> bufP			- pointer on receiving buffer
 *		-> bufferSize	- the buffer size
 *		<- errP			- ptr on Error code, 0 if no error
 *
 * Returned:	The number of bytes received, or 0 if the object is complete.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
uint32_t ExgPhoneBookIOLibReceive(ExgSocketType* socketP, void* bufP, const uint32_t bufSize, 
	status_t* errP)
{	
  	PhoneBkIOLibGlobalsType *gP = &gPhoneBookIOGbl;
	uint32_t bytesRead = 0;
	PhoneBkIOSocketInfoType *socketInfoP;

	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibReceive"));
	
	ErrNonFatalDisplayIf(!socketP, "NULL socketP");
	ErrNonFatalDisplayIf(!bufP, "NULL bufP");
	ErrNonFatalDisplayIf(!errP, "NULL errP");
	
	socketInfoP = (PhoneBkIOSocketInfoType *)socketP->socketRef;

	// The progress dialog doesn't exist when exiting ExgGet, reopen it if needed
	PBkIOProgressStartDialog(gP, (Boolean) socketP->noStatus);
	gP->curStage = kPhoneBkIOConStageReceiving;
	PBkIOProgressUpdateDialog(gP, errNone, NULL);
	
 	*errP = PBkIOProgressProcessEvents(gP, NULL);
	if (*errP == exgErrUserCancel)
		return 0;

	if (! (*errP < errNone) )
	{
		// Read the data from the file (allow for partial read).
		bytesRead = FileRead(socketInfoP->tempFileH, bufP, sizeof(char), bufSize, errP);
		if (*errP == fileErrEOF)
			*errP = errNone;
		else if (! (*errP < errNone) && bytesRead == 0)
			*errP = exgErrUnknown;
	}
	
	return bytesRead;
}

	
	
/************************************************************
 *
 * Function:	ExgPhoneBookIOLibControl
 *
 * Description:	Standard ExgLibControl routine - See PalmOS 
 *				reference document for description.
 *
 * Parameters:	-> op			- control operation to perform
 *				<-> valueP		- ptr to value for operation
 *				<-> valueLenP	- ptr to size of value
 *
 * Returned:	0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibControl(uint16_t op, void* valueP, uint16_t* valueLenP)
{	
	status_t 	err = errNone;
	MemHandle 	resourceH;
	char 		*resourceP;
	FormType* 	dialogP;
	uint16_t 	buttonHit;
	
	TraceOutput(TL(phoneBkIOErrorClass,"ExgPhoneBookIOLibControl"));

	// Open the lib or increment ref count if already loaded
	ExgPhoneBookIOLibOpen();

	switch (op)
		{
		case exgLibCtlGetTitle:
			if (valueP && valueLenP) {
				resourceH = DmGetResource(gApplicationDbP, strRsc, PhoneBkIOlTitleString);
				if (resourceH == NULL) {
					err = DmGetLastErr();
					break;
				}
				resourceP = MemHandleLock(resourceH);
				strncpy(valueP, resourceP, *valueLenP);
				MemHandleUnlock(resourceH);
				((char *)valueP)[*valueLenP - 1] = chrNull;	// terminate string if clipped
			}
			else
				err = exgErrBadParam;
			break;
			
		case exgLibCtlGetVersion:
			if (valueP)
				*((uint16_t *)valueP) = exgLibAPIVersion;
			else
				err = exgErrBadParam;
			break;
			
		case exgLibCtlGetPreview:
			// This lib does not support preview
			if (valueP)
				*((Boolean *)valueP) = false;
			else
				err = exgErrBadParam;
			break;

		case phoneBkIOLibCtlSetPhoneRecipient:
			// Assign the phone recipient
			if (valueP && (*valueLenP == sizeof(uint8_t)))
			{
				uint8_t selPhBook = *((uint8_t*) valueP);

				// Store selected the PhoneBook
				if (selPhBook == kPhoneBookIOPhbSIM)
					err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, kPhoneBookIOPhbSIM);
				else if (selPhBook == kPhoneBookIOPhbME)
					err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, kPhoneBookIOPhbME);
				else
					err = exgErrBadParam;
			}
			else
				err = exgErrBadParam;
			break;

		case phoneBkIOLibCtlAskPhoneRecipient:
			if (valueP) 
			{
				ControlType*	popUpControlP;
				ListType*		listP;
				uint32_t		selectedPhoneBook;
				
                err = FtrGet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, &selectedPhoneBook);
				if (err < errNone)
					selectedPhoneBook = kPhoneBookIOPhbSIM;

				dialogP = FrmInitForm (gApplicationDbP, PBkIOPreferencesForm);				
				// Set current selection
				popUpControlP = (ControlType*) FrmGetObjectPtr(dialogP,	FrmGetObjectIndex(dialogP, PBkIOPreferencesRecipientPopTrigger));
				listP = (ListType*) FrmGetObjectPtr(dialogP, FrmGetObjectIndex(dialogP, PBkIOPreferencesList));
												
				LstSetSelection(listP, (int16_t)selectedPhoneBook);
				CtlSetLabel(popUpControlP, LstGetSelectionText(listP, (int16_t)selectedPhoneBook));
				buttonHit = FrmDoDialog (dialogP);

				if (buttonHit == PBkIOPreferencesOkButton)
				{	
					// Update selection
					selectedPhoneBook = (uint32_t) LstGetSelection(listP);
                  	if (selectedPhoneBook == kPhoneBookIOPhbSIM)
						err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, kPhoneBookIOPhbSIM);
					else
						err = FtrSet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, kPhoneBookIOPhbME);

					*((Boolean*)valueP) = true;
				}
				else
					*((Boolean*)valueP) = false;
				
				FrmDeleteForm(dialogP);				
			}
			else
				err = exgErrBadParam;
			break;
			
		default:
			err = exgErrNotSupported;
		}

	// Decrement ref count or close the lib
	ExgPhoneBookIOLibClose();

	return err;
}

	
/************************************************************
 *
 * Function:	ExgPhoneBookIOLibRequest
 *
 * Description:	Not supported by this library
 *
 * Parameters:	-> exgSocketP - socket specifying what to check
 *
 * Returned:	0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t ExgPhoneBookIOLibRequest(ExgSocketType* exgSocketP)
{	
	return exgErrNotSupported;
}
