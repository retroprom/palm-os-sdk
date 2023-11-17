/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PBkIOTransfer.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             	library for Import / Export address book with a phone.
 *
 *****************************************************************************/

// System includes
#include <PalmTypes.h>
#include <TraceMgr.h>
#include <SystemMgr.h>
#include <TelephonyLib.h>
#include <StringMgr.h>
#include <Chars.h>
#include <FeatureMgr.h>

#include <stdio.h>
#include <string.h>

// PhoneBoookIO includes
#include "PhoneBookIOLibPrv.h"
#include "PBkIOTransferPrv.h"
#include "PBkIOProgressPrv.h"
#include "PhoneBookIOLibRes.h"

// *******************************************************************************
//	Static variables
// *******************************************************************************
// Telephony constants : match with resources
static uint16_t sPhoneBooksTelConstants[] = {kTelPhbSIM, kTelPhbME};

// PhoneLib reference number, static phone constants...
static int32_t 				sTelAppId 				= kTelInvalidAppId;
static Boolean 				sConnected 				= false;
static Boolean 				sWasAlreadyConnected	= false;
static TelPhbPhonebookType	sPhoneBkInfo;
static Boolean 				sPhoneBkInfoAvailable 	= false;


/************************************************************
 *
 * FUNCTION:		PBkIOTransferTelInitialize
 *
 * DESCRIPTION:	Initialize the telephony
 *
 * PARAMETERS:	
 *				-> phoneBkGlobalsP : PhoneBookIO globals.
 * 
 *  RETURNS: Error Code, 0 if no error.
 *		Possible error codes : telErrTTaskNotFound, telErrTTaskNotRunning, ...
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOTransferTelInitialize(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, ExgSocketType* socketP)
{
	status_t 					err;
	uint16_t 					ioTransId;
	TelASyncEventInfoType 		telASyncInfo;
	uint8_t						authenticationResult;
	TelStyAuthenticationType	authenticationInfo;
	Boolean 					askPINCode = false;
	uint32_t					storageState;
	
	if ((err = TelOpen(kTelMgrVersion, &sTelAppId)) < errNone)
		goto ExitOnError;

	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Phone lib opened successfully"));
	
	// Check the connection state
	if ((err = TelCncGetStatus(sTelAppId, &sWasAlreadyConnected)) < errNone)
		goto ExitOnError;

	// Set progress state
	phoneBkGlobalsP->curStage = kPhoneBkIOConStageConnectingPhone;

	if (!sWasAlreadyConnected)
	{
		// Perform asynchronous call
		TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Calling TelCncOpen"));

		if ((err = TelCncOpen(sTelAppId, &ioTransId)) < errNone)
        	goto ExitOnError;

		// Open progress dialog to be informed of connect process
		PBkIOProgressStartDialog(phoneBkGlobalsP, (Boolean) socketP->noStatus);
		PBkIOProgressUpdateDialog(phoneBkGlobalsP, err, socketP->description);

		telASyncInfo.telAppId = sTelAppId;
		telASyncInfo.transId = ioTransId;

		if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)
			goto ExitOnError;

		// Wait for the PowerOn notification
		

	}
	else
	{
		// Open progress dialog 
		PBkIOProgressStartDialog(phoneBkGlobalsP, (Boolean) socketP->noStatus);
		PBkIOProgressUpdateDialog(phoneBkGlobalsP, err, socketP->description);
	}
    
	sConnected = true;

	// Check the PIN code
	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: checking for PIN code"));

	if ((err = TelStyGetAuthenticationStatus(sTelAppId, &authenticationResult, &ioTransId)) < errNone)
	{
		TraceOutput(TL(phoneBkIOErrorClass, "PhoneBookIO: ## TelStyGetAuthenticationStatus(...) - failed - error: 0x%08lX.", err));
		/*
		if (err == telErrAlreadyAuthenticating)
		{
			// The PIN dialog is already displayed by other application or sliplet,
			// ask the user to retry or cancel...
			FrmAlert(gApplicationDbP, PBkIOAlreadyAuthenticating);
			// Set sWasAlreadyConnected to true so the connection won't be closed
			// on exit.
			sWasAlreadyConnected = true;
		}
		*/
		goto ExitOnError;
	}
	else
	{
		TraceOutput(TL(phoneBkIOErrorClass, "PhoneBookIO: ## TelStyGetAuthenticationStatus(...) - success - TransID: %hu.", ioTransId));
	}

	// Wait the result of PIN code status
	telASyncInfo.telAppId = sTelAppId;
	telASyncInfo.transId = ioTransId;

	if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)
		goto ExitOnError;

	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Aunthentification status = %d", authenticationResult));
	
	switch (authenticationResult)
	{
		case kTelStyAuthReady:
			// All is fine
			break;
			
		case kTelStyAuthNoSim:
			// No SIM : abort connection
			err = telErrNoSIMInserted;
			PBkIOProgressStopDialog(phoneBkGlobalsP);
			goto ExitOnError;
			
		default:
			// Mobile expects some PIN / PUK... code
			askPINCode = true;
			break;
	}

	if (askPINCode)
	{
		TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Asking for PIN code"));

		// Close the progress and ask the code
		PBkIOProgressStopDialog(phoneBkGlobalsP);
		authenticationInfo.type = authenticationResult;
		err = TelStyEnterAuthentication(sTelAppId, &authenticationInfo, &ioTransId);
		if (err < errNone)
		{
			TraceOutput(TL(phoneBkIOErrorClass, "PhoneBookIO: ## TelStyEnterAuthentication(...) - failed - error: 0x%08lX.", err));

			/*
			if (err == telErrAlreadyAuthenticating)
			{
				// The PIN dialog is already displayed by other application or sliplet,
				// ask the user to retry or cancel...
				FrmAlert(gApplicationDbP, PBkIOAlreadyAuthenticating);
				// Set sWasAlreadyConnected to true so the connection won't be closed
				// on exit.
				sWasAlreadyConnected = true;
			}
			*/
			goto ExitOnError;
		}
		else
		{
			TraceOutput(TL(phoneBkIOErrorClass, "PhoneBookIO: ## TelStyEnterAuthentication(...) - success - TransID: %hu.", ioTransId));
		}

		// Set progress state
		phoneBkGlobalsP->curStage = kPhoneBkIOConStageConnectingPhone;

		// Reopen the progress
		PBkIOProgressStartDialog(phoneBkGlobalsP, (Boolean) socketP->noStatus);
		PBkIOProgressUpdateDialog(phoneBkGlobalsP, err, socketP->description);
		
		// Wait the result of PIN code status
		telASyncInfo.telAppId = sTelAppId;
		telASyncInfo.transId = ioTransId;
		err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo);

		// Check if the user entered a correct PIN
		PBkIOProgressUpdateDialog(phoneBkGlobalsP, err, NULL);
		
		if (err < errNone)
			goto ExitOnError;
	}
	
	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Phone lib connected successfully"));
	
	// Check the storage state, wait for ready state.
	err = FtrGet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrStorageState, &storageState);
	if (err == ftrErrNoSuchFeature)
        storageState = kPhoneBookIOStorageNotReady;
	else if (err < errNone)
		goto ExitOnError;
	
	if (storageState == kPhoneBookIOStorageNotReady)
	{
		// Set progress state
		err = errNone;
		phoneBkGlobalsP->curStage = kPhoneBkIOConStageWaitingPhoneBookReady;
		PBkIOProgressUpdateDialog(phoneBkGlobalsP, err, socketP->description);

		TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Storage is Not Ready"));
		err = PBkIOProgressProcessEvents(phoneBkGlobalsP, NULL);
		if (err < errNone)
			goto ExitOnError;
	}
	else
		TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: Storage is Ready"));

	return err;
	
ExitOnError:	
	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: TelInitialize failed with err = %x", err));
	
	PBkIOTransferTelFinalize(phoneBkGlobalsP, socketP);
	sConnected = false;
	return err;
}


/************************************************************
 *
 * FUNCTION:		PBkIOTransferTelFinalize
 *
 * DESCRIPTION:	Close the phone connection and unloads the 
 *				phone library.
 *
 * PARAMETERS:	
 *				-> phoneBkGlobalsP : PhoneBookIO globals.
 * 
 *  RETURNS: Error Code, 0 if no error.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOTransferTelFinalize(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, ExgSocketType* socketP)
{
	status_t err = errNone;

	TraceOutput(TL(phoneBkIOErrorClass,"PBkIOTransferTelFinalize"));

	if ((sConnected) && (!sWasAlreadyConnected))
	{
		phoneBkGlobalsP->curStage = kPhoneBkIOConStageDisconnecting;
		PBkIOProgressUpdateDialog(phoneBkGlobalsP, err, socketP->description);
		PBkIOProgressProcessEvents(phoneBkGlobalsP, NULL);

		if ((err = TelCncClose(sTelAppId)) >= errNone)
			sConnected = false;
		else
		{
			if ((err != telErrResultUserCancel) && (err != exgErrUserCancel))
				TelUiManageError(err, NULL);
		}
	}
	
	if (sTelAppId != kTelInvalidAppId) 
	{
		err = TelClose(sTelAppId);
		if (err < errNone)
		{
			if ((err != telErrResultUserCancel) && (err != exgErrUserCancel))
				TelUiManageError(err, NULL);
		}

		sTelAppId = kTelInvalidAppId;
	}
		
	sPhoneBkInfoAvailable = false;
	return err;
}


/************************************************************
 *
 * FUNCTION:		PBkIOTransferReadRecordFromPhone
 *
 * DESCRIPTION:	Read the phone entry specified by the slot
 *				index.
 *
 * PARAMETERS:	
 *				-> phoneBkGlobalsP : PhoneBookIO globals.
 *				<- phoneRecordP : the record read from phone.
 *				-> index : the phone slot to get back.
 * 
 *  RETURNS: Error Code, 0 if no error.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOTransferReadRecordFromPhone(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, AddressPhoneRecordType * phoneRecordP, int32_t index)
{
	status_t				err = errNone;
	TelPhbEntryType			phoneBkEntry;
	uint16_t				ioTransId;
	TelASyncEventInfoType	telASyncInfo;

	phoneBkEntry.fullNameP = phoneRecordP->lastNameP;
	phoneBkEntry.fullNameSize = kTelPhoneNameMaxLen;
	phoneBkEntry.dialNumber.numberP = phoneRecordP->phoneNbP[0];
	phoneBkEntry.dialNumber.size = kTelPhoneNumberMaxLen;
	phoneBkEntry.phoneIndex = (uint16_t) index;
	TelPhbGetEntry(sTelAppId, &phoneBkEntry, &ioTransId);
	telASyncInfo.telAppId = sTelAppId;
	telASyncInfo.transId = ioTransId;
	
	err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo);

//	phoneRecordP->phoneLabel[0] = homeLabel;

	return err;
}


/************************************************************
 *
 * FUNCTION:		PBkIOTransferGetPhoneBookInfo
 *
 * DESCRIPTION:	Get come phone book information (used, total
 *				slots) of the phone directory specified by
 *				the selectedPhoneBook field of PhoneBookIO globals.
 *
 * PARAMETERS:	
 *				-> phoneBkGlobalsP : PhoneBookIO globals.
 *				<- phoneBookInfoP : phone book information
 * 
 *  RETURNS: Error Code, 0 if no error.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOTransferGetPhoneBookInfo(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, AddressPhoneBookInfoType * phoneBookInfoP)
{
	status_t				err ;
	uint16_t				ioTransId;
	TelASyncEventInfoType	telASyncInfo;
	uint32_t				selectedPhoneBook;
	
	telASyncInfo.telAppId = sTelAppId;

	if (!sConnected)
		return phoneBkIOErrNotConnected;

	// Select the phone storage
	memset(&sPhoneBkInfo, 0, sizeof(sPhoneBkInfo));
	
	err = FtrGet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, &selectedPhoneBook);
	if (err == ftrErrNoSuchFeature)
        selectedPhoneBook = kPhoneBookIOPhbSIM;
	else if (err < errNone)
		goto Exit;

	sPhoneBkInfo.id = sPhoneBooksTelConstants[selectedPhoneBook];
	TelPhbSetPhonebook(sTelAppId, &sPhoneBkInfo, &ioTransId);
	telASyncInfo.transId = ioTransId;

	if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)
		goto Exit;

	TelPhbGetPhonebook(sTelAppId, &sPhoneBkInfo, &ioTransId);
	telASyncInfo.transId = ioTransId;

	if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)
		goto Exit;

	phoneBookInfoP->phoneBkUsedSlots = sPhoneBkInfo.usedSlot;
	phoneBookInfoP->phoneBkTotalSlots = sPhoneBkInfo.totalSlot;
	sPhoneBkInfoAvailable = true;

Exit:	
	return err;
}


	
/************************************************************
 *
 * FUNCTION:		PBkIOTransferAddRecordToPhone
 *
 * DESCRIPTION:	Transfer the record to the phone. The record may
 *				have several phone numbers: in such case, call 
 *				again this routine until allPhoneEntriesHandledP
 *				returns true.
 *
 * PARAMETERS:	
 *				-> phoneBkGlobalsP : PhoneBookIO globals.
 *				-> phoneRecordP : ptr on the record to add to 
 *					the phone.
 *				-> index : phone slot index to put the record
 *				<- allPhoneEntriesHandledP : returns true if there
 *					is still phone fields in phoneRecordP that have
 *					not been handled, false otherwise.
 * 
 *  RETURNS: Error Code, 0 if no error.
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOTransferAddRecordToPhone(PhoneBkIOLibGlobalsType * phoneBkGlobalsP,
	AddressPhoneRecordType * phoneRecordP, int32_t index,  Boolean * allPhoneEntriesHandledP)
{
	status_t				err ;
	uint16_t				i, srcLen;
	TelPhbEntryType			phoneEntry;
	char					*fullNameP = NULL;
	char					*phoneNbP = NULL;
	Boolean					phoneEntryAdded = false;
	char					*srcP, *destP;
	Boolean					recordIsValid = false;
	uint16_t				ioTransId;
	TelASyncEventInfoType	telASyncInfo;
	uint32_t				selectedPhoneBook;

	telASyncInfo.telAppId = sTelAppId;

	if (!sConnected)
		return phoneBkIOErrNotConnected;

	if (!sPhoneBkInfoAvailable)
	{
		memset(&sPhoneBkInfo, 0, sizeof(sPhoneBkInfo));
		
		err = FtrGet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, &selectedPhoneBook);
		if (err == ftrErrNoSuchFeature)
			selectedPhoneBook = kPhoneBookIOPhbSIM;
		else if (err < errNone)
			goto Exit;
		
		sPhoneBkInfo.id = sPhoneBooksTelConstants[selectedPhoneBook];
		TelPhbSetPhonebook(sTelAppId, &sPhoneBkInfo, &ioTransId);
		telASyncInfo.transId = ioTransId;

		if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)	
			goto Exit;
		
		TelPhbGetPhonebook(sTelAppId, &sPhoneBkInfo, &ioTransId);
		telASyncInfo.transId = ioTransId;

		if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)	
			goto Exit;

		sPhoneBkInfoAvailable = true;
	}

	// Allocate name & phone numbers
	fullNameP = MemPtrNew((size_t)(sPhoneBkInfo.fullNameMaxSize));
	phoneNbP = MemPtrNew((size_t)(sPhoneBkInfo.dialNumberMaxSize));

	// Concatenate the name
	StrNCopy(fullNameP, phoneRecordP->lastNameP, (int16_t)(sPhoneBkInfo.fullNameMaxSize));
	fullNameP[sPhoneBkInfo.fullNameMaxSize] = chrNull;
	if ((phoneRecordP->firstNameP) && (StrCompare((const char*)phoneRecordP->firstNameP, "")))
	{
		StrNCat(fullNameP, " ", (int16_t)(sPhoneBkInfo.fullNameMaxSize));
		StrNCat(fullNameP, phoneRecordP->firstNameP, (int16_t)(sPhoneBkInfo.fullNameMaxSize));
	}

	if ((phoneRecordP->titleP) && (StrCompare((const char*)phoneRecordP->titleP, "")))
	{
		StrNCat(fullNameP, " ", (int16_t)(sPhoneBkInfo.fullNameMaxSize));
		StrNCat(fullNameP, phoneRecordP->titleP, (int16_t)(sPhoneBkInfo.fullNameMaxSize));
	}

	if ((phoneRecordP->companyP) && (StrCompare((const char*)phoneRecordP->companyP, "")))
	{
		StrNCat(fullNameP, " ", (int16_t)(sPhoneBkInfo.fullNameMaxSize));
		StrNCat(fullNameP, phoneRecordP->companyP, (int16_t)(sPhoneBkInfo.fullNameMaxSize));
	}

	// Skipp beginning spaces
	srcP = destP = fullNameP;

	while (*srcP == chrSpace)
		srcP++;

	while (*srcP != chrNull)
		*destP++ = *srcP++;

	*destP = chrNull;

	// Get the phone number: get the first valid label
	*allPhoneEntriesHandledP = true;

	for (i = 0; i < kPhoneBookIOMaxPhoneNumber; i++) 
	{
		if (phoneRecordP->phoneNbP[i][0])
//		if ((phoneRecordP->phoneLabel[i] != emptyLabel) && (phoneRecordP->phoneLabel[i] != emailLabel)) 
		{
			if (phoneEntryAdded) 
			{
				// Multiple phone entries => ask to be called again
				*allPhoneEntriesHandledP = false;
				break;
			}
			// StrNCopy(phoneNbP, phoneRecordP->phoneNbP[i], (int16_t)(sPhoneBkInfo.dialNumberMaxSize));
			srcP = phoneRecordP->phoneNbP[i];
			destP = phoneNbP;
			srcLen = (uint16_t)sPhoneBkInfo.dialNumberMaxSize;
			
			while (*srcP != chrNull && srcLen--)
			{
				if ((*srcP == chrPlusSign) || (*srcP >= chrDigitZero && *srcP <= chrDigitNine))
					*destP++ =  *srcP;

				srcP++;
			}
			
			// Put final chrNull
			*destP = chrNull;
			// Mark as emptyLabel for next pass
//			phoneRecordP->phoneLabel[i] = emptyLabel;
			phoneRecordP->phoneNbP[i][0] = chrNull;

			if ((phoneNbP[0] != chrNull) && (fullNameP[0] != chrNull)) 
			{
				// A number and a name have been retrieved, set this record as valid
				recordIsValid = true;
				phoneEntryAdded = true;
			}
		}
	}
	
	if (recordIsValid) 
	{
		// Add the entry
		phoneEntry.phoneIndex = (uint16_t)index;
		phoneEntry.fullNameP = fullNameP;
		phoneEntry.fullNameSize = strlen(fullNameP) + 1;
		phoneEntry.dialNumber.numberP = phoneNbP;
		phoneEntry.dialNumber.size = strlen(phoneNbP) + 1;
		
		if ((phoneNbP) && (*phoneNbP == chrPlusSign))
			phoneEntry.dialNumber.type = kTelNumberTypeInternational;
		else
			phoneEntry.dialNumber.type = kTelNumberTypeUnknown;

		TelPhbAddEntry(sTelAppId, &phoneEntry, &ioTransId);
		telASyncInfo.transId = ioTransId;
		err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo);
	}
	else
	{		
		err = phoneBkIOErrEmptyRec;
	}

Exit:	
	if (fullNameP)
		MemPtrFree(fullNameP);
	
	if (phoneNbP)
		MemPtrFree(phoneNbP);

	return err;
}


/************************************************************
 *
 * FUNCTION:		PBkIOTransferDeletePhoneRecord
 *
 * DESCRIPTION:	Deletes the phone slot specified by the slot index
 *
 * PARAMETERS:
 *				-> phoneBkGlobalsP : PhoneBookIO globals.
 *				-> index : phone slot index 
 * 
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOTransferDeletePhoneRecord(PhoneBkIOLibGlobalsType * phoneBkGlobalsP, int32_t index)
{
	status_t				err = errNone;
	uint16_t				ioTransId;
	TelASyncEventInfoType	telASyncInfo;
	uint32_t				selectedPhoneBook;

	telASyncInfo.telAppId = sTelAppId;

	if (!sConnected)
		return phoneBkIOErrNotConnected;

	if (!sPhoneBkInfoAvailable)
	{
		memset(&sPhoneBkInfo, 0, sizeof(sPhoneBkInfo));
		
		err = FtrGet(sysFileCPhoneBookIOLib, kPhoneBookIOFtrSelectedPhoneBook, &selectedPhoneBook);
		if (err == ftrErrNoSuchFeature)
			selectedPhoneBook = kPhoneBookIOPhbSIM;
		else if (err < errNone)
			goto Exit;

		sPhoneBkInfo.id = sPhoneBooksTelConstants[selectedPhoneBook];
		TelPhbSetPhonebook(sTelAppId, &sPhoneBkInfo, &ioTransId);
		telASyncInfo.transId = ioTransId;

		if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)	
			goto Exit;

		err = TelPhbGetPhonebook(sTelAppId, &sPhoneBkInfo, &ioTransId);
		telASyncInfo.transId = ioTransId;

		if ((err = PBkIOProgressProcessEvents(phoneBkGlobalsP, &telASyncInfo)) < errNone)
			goto Exit;

		sPhoneBkInfoAvailable = true;
	}

	err = TelPhbDeleteEntry(sTelAppId, (uint16_t)index, NULL);

Exit:
	return err;
}
