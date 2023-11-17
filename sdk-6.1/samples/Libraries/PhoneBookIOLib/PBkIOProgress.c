/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PBkIOProgress.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *             	Handling of progress dialog for the PhoneBookIO exchange library
 *
 *****************************************************************************/

// System includes
#include <PalmTypes.h>
#include <SysEvtMgr.h>
#include <SystemMgr.h>
#include <ErrorMgr.h>
#include <Progress.h>
#include <TextMgr.h>
#include <StringMgr.h>
#include <TraceMgr.h>
#include <TimeMgr.h>
#include <TelephonyLib.h>
#include <SysUtils.h>

// PhoneBoookIO includes
#include "PBkIOProgressPrv.h"
#include "PhoneBookIOLibRes.h"


// *******************************************************************************
//	Defines
// *******************************************************************************

#define kUpdateProgressTimeoutTicks (SysTimeInMilliSecs(500))  // update animation every second.

/************************************************************
 *
 * FUNCTION:		PrvSwitchBitmap
 *
 *
 *************************************************************/
 static uint16_t PrvSwitchBitmap(uint16_t prevId, uint16_t choice1, uint16_t choice2, uint16_t choice3)
 {
 	uint16_t idRes;

	if (choice3 != 0)
	{
		if (prevId == choice1)
			idRes = choice2;
		else if (prevId == choice2)
			idRes = choice3;
		else
			idRes = choice1;
	}
	else
	{
		if (prevId == choice1)
			idRes = choice2;
		else 
			idRes = choice1;
	}

	return idRes;
 }

 /************************************************************
 *
 * FUNCTION:		PrvGetStrIdxByStage
 *
 *
 *************************************************************/
static uint16_t PrvGetStrIdxByStage(const uint16_t stage)
{
	switch (stage)
	{
		case kPhoneBkIOConStageConnectingPhone:
		case kPhoneBkIOConStageWaitingPhoneBookReady:
			return kPhoneBkIOStrIndexConnectingPhone;

		case kPhoneBkIOConStageSending:
			return kPhoneBkIOStrIndexSending;

		case kPhoneBkIOConStageSendvCard:
			return kPhoneBkIOStrIndexSendvCard;

		case kPhoneBkIOConStageReceiving:
			return kPhoneBkIOStrIndexReceiving;

		case kPhoneBkIOConStageRecvCard:
			return kPhoneBkIOStrIndexRecvCard;

		case kPhoneBkIOConStageDeleting:
			return kPhoneBkIOStrIndexDeleting;

		case kPhoneBkIOConStageDisconnecting:
			return kPhoneBkIOStrIndexDisconnecting;

		case kPhoneBkIOConStageCanceling:
			return kPhoneBkIOStrIndexCanceling;
	} // switch (stage)

	return kPhoneBkIOStrIndexInvalid;
}

 /************************************************************
 *
 * FUNCTION:		PrvPBkIOProgressCallback
 *
 * DESCRIPTION:	Set the appropriate text and icons for the
 *						current stage of the connection. This is called by the progress
 *						manager periodically to update the progress dialogs. We pass a pointer
 *						to this callback when we start the progress manager.
 *
 * PARAMETERS:		cbP - pointer to callback data
 *
 * CALLED BY:		Progress dialog code.
 *
 * RETURNS:			true if able to generate valid output
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static Boolean PrvPBkIOProgressCallback(PrgCallbackData *cbP)
{
	char*	message = NULL;
	Boolean	handled = false;
	char	ellipsisString[maxCharBytes + 1];

	TraceOutput(TL(phoneBkIOErrorClass,"PhoneBookIO: PrvTextCallback, timeOut = %d", cbP->timedOut));
	
	// bitmaps are located in the PhoneBookIOLib database.
	cbP->bitmapDatabase = gApplicationDbP;
	
	// Check for animation timeout and update icon if necessary.
	if ((cbP->timedOut) && (cbP->bitmapId != PBkIOPrgIdle))
	{
		switch (cbP->bitmapId)
		{
			case PBkIOPrgImport1:
			case PBkIOPrgImport2:
			case PBkIOPrgImport3:
				cbP->bitmapId = PrvSwitchBitmap(cbP->bitmapId, PBkIOPrgImport1, PBkIOPrgImport2, PBkIOPrgImport3);
				break;
			
			case PBkIOPrgExport1:
			case PBkIOPrgExport2:
			case PBkIOPrgExport3:
				cbP->bitmapId = PrvSwitchBitmap(cbP->bitmapId, PBkIOPrgExport1, PBkIOPrgExport2, PBkIOPrgExport3);
				break;
			
			case PBkIOPrgConnecting1:
			case PBkIOPrgConnecting2:
				cbP->bitmapId = PrvSwitchBitmap(cbP->bitmapId, PBkIOPrgConnecting1, PBkIOPrgConnecting2, 0);
				break;
		}

		cbP->textChanged = false;	// do not update text
		cbP->timeout = (uint32_t)(TimGetTicks() + kUpdateProgressTimeoutTicks);	// if we're way behind, skip frames
		
		return true;
	}

	// Check for known error codes
	switch (cbP->error) 
	{
		case exgErrUserCancel:		
			// If the current error is userCancel, clear it and put dialog into
			// canceling mode (no Cancel button).
			cbP->error = errNone;
			cbP->canceled = true;
			break;

		case telErrResultTimeOut:
			// Get the text from resource
			SysStringByIndex(gApplicationDbP, PBkIOErrMessagesStringList, errMessageTimeOut, cbP->textP, (uint16_t)cbP->textLen);
			cbP->textP[cbP->textLen - 1] = chrNull;			
			handled = true;
			break;
	}
	
	// If the protocol task hasn't recognized the cancel yet, the boolean
	// will still be set with no error yet.
	if (cbP->canceled && !(cbP->error < errNone))
	{
		handled = true;
		SysStringByIndex(gApplicationDbP, PBkIOProgressStringList, kPhoneBkIOStrIndexCanceling, cbP->textP, (uint16_t)cbP->textLen);
		cbP->textP[cbP->textLen - 1] = chrNull;
		cbP->bitmapId = PBkIOPrgIdle;
	}
		
	// Let the default handlers take care of errors and the canceling status
	if (!cbP->canceled && !(cbP->error < errNone))
	{
		handled = true;
		message = NULL;

		switch (cbP->stage)
		{
			
			case kPhoneBkIOConStageConnectingPhone:
			case kPhoneBkIOConStageDisconnecting:
			case kPhoneBkIOConStageWaitingPhoneBookReady:
				cbP->bitmapId = PrvSwitchBitmap(cbP->bitmapId, PBkIOPrgConnecting1, PBkIOPrgConnecting2, 0);
				cbP->timeout = kUpdateProgressTimeoutTicks;  // start animation
				break;

			case kPhoneBkIOConStageSendvCard:
				message = cbP->message;
				// Don't break here

			case kPhoneBkIOConStageDeleting:
				cbP->bitmapId = PrvSwitchBitmap(cbP->bitmapId, PBkIOPrgImport1, PBkIOPrgImport2, PBkIOPrgImport3);
				cbP->timeout = kUpdateProgressTimeoutTicks;  // start animation
				break;

			case kPhoneBkIOConStageRecvCard:
				message = cbP->message;
				cbP->bitmapId = PrvSwitchBitmap(cbP->bitmapId, PBkIOPrgExport1, PBkIOPrgExport2, PBkIOPrgExport3);
				cbP->timeout = kUpdateProgressTimeoutTicks;  // start animation
				break;
				
			case kPhoneBkIOConStageSending:
			case kPhoneBkIOConStageReceiving:
				cbP->bitmapId = PBkIOPrgTransfer;
				break;
				
			case kPhoneBkIOConStageInvalid:
				cbP->bitmapId = PBkIOPrgIdle;
				cbP->canceled = true; 		// put dialog in cancel mode (no OK button)
				cbP->timeout = 0;	// always clear timeout when updating status
				break;
			
			default:
				ErrNonFatalDisplay("Unknown Stage");
				break;
		} // end of switch
		
		// Look up our action string
       	SysStringByIndex(gApplicationDbP, PBkIOProgressStringList, PrvGetStrIdxByStage(cbP->stage), cbP->textP, (uint16_t)cbP->textLen);
		
		// Add extra message text to dialog if it exists...
		if (message)
		{
			StrNCat(cbP->textP, ": ", (uint16_t)cbP->textLen);
			StrNCat(cbP->textP, message, (uint16_t)cbP->textLen);
		}
		
		// add Elipsis
		ellipsisString[TxtSetNextChar(ellipsisString, 0, chrEllipsis)] = chrNull;
		StrNCat(cbP->textP, ellipsisString, (uint16_t)cbP->textLen);
	}
	
	// Adjust timeout to beyond current time.
	if (cbP->timeout)
		cbP->timeout =(uint32_t)(cbP->timeout + TimGetTicks());
	
	return handled;
}

/************************************************************
 *
 *  FUNCTION: PBkIOProgressStartDialog
 *
 *  DESCRIPTION: Starts the progress dialog if necessary.
 *
 *  PARAMETERS:
 *		gP		 	- library globals
 *		socketStatusDisplayOff	- socket noStatus flag
 *
 *	CALLED BY:
 *		Whoever wants to start the progress dialog
 *
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
void PBkIOProgressStartDialog(PhoneBkIOLibGlobalsType *gP, Boolean socketStatusDisplayOff)
{
	if (! socketStatusDisplayOff && ! gP->prgP)
	{
		uint16_t	bufferSize = exgTitleBufferSize;
		char		title[exgTitleBufferSize];
		status_t 	err = errNone;
		
		// Get the title of the library
		err = ExgPhoneBookIOLibControl(exgLibCtlGetTitle, &title, &bufferSize);
		ErrNonFatalDisplayIf(err, "Error getting title");
		
		// Start progress dialogs giving title and progress callback function
		gP->prgP = PrgStartDialog(title, PrvPBkIOProgressCallback, NULL);
		
		TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: Start the dialog, gP->prgP= %lx", gP->prgP));
	}
}


/************************************************************
 *
 *  FUNCTION: PBkIOProgressStopDialog
 *
 *  DESCRIPTION: Stops the progress dialog
 *
 *  PARAMETERS:
 *		gP		 	- library globals
 *
 *	CALLED BY:
 *		Whoever wants to stop the progress dialog
 *
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
void PBkIOProgressStopDialog(PhoneBkIOLibGlobalsType *gP)
{
	if (gP->prgP)
	{
		TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressStopDialog: Stop the dialog, gP->prgP= %lx", gP->prgP));
		
		PrgStopDialog(gP->prgP, false);
		gP->curStage = kPhoneBkIOConStageDisconnecting;
		gP->prgP = NULL;
	}
}


/************************************************************
 *
 *  FUNCTION: PhoneIODisplayTelephonyEvent
 *
 *  DESCRIPTION: 
 *
 *  PARAMETERS:
 *
 * Returned:	true to exit the event process loop
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
static void PhoneIODisplayTelephonyEvent(TelEventType *eventP)
{
	switch (eventP->functionId)
	{
		case kTelCncOpenMessage:
			TraceOutput(TL(phoneBkIOErrorClass,"ProcessTelEvent : kTelCncOpenMessage"));
			break;

		case kTelCncCloseMessage:
			TraceOutput(TL(phoneBkIOErrorClass,"ProcessTelEvent : kTelCncCloseMessage"));
			break;

		default:
			TraceOutput(TL(phoneBkIOErrorClass,"ProcessTelEvent : message with functionId = %x", eventP->functionId));
			break;
	}
}


/************************************************************
 *
 *  FUNCTION: PBkIOProgressProcessEvents
 *
 *  DESCRIPTION: Utility routine handles events during exchange.
 *		We take over the UI from the current app with our progress
 *		dialogs; this handles events for progress.
 *	
 *			This call is not blocking when called with telASyncInfoP set
 *		to NULL, this allow to update the progress display and then to go on.
 *		When telASyncInfoP is not NULL, the call has been done to wait
 *		for a telephony asynchronous call. In this case, this routine blocks
 *		until the telephony result OR a tap on the Cancel button.
 *
 *  PARAMETERS:
 *				gp - pointer to library globals
 *				telASyncInfoP - tel info for asynchronous calls.
 *
 * Returned:	0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
status_t PBkIOProgressProcessEvents(PhoneBkIOLibGlobalsType *gP, TelASyncEventInfoType * telASyncInfoP)
{
	EventType	event;
	TelEventPtr	telEventP;
	status_t 	err = errNone;
	Boolean 	nowExit = false;

	TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: Enter"));

	do
	{
		if (telASyncInfoP)
			TelEvtGetEvent(telASyncInfoP->telAppId, &event, kUpdateProgressTimeoutTicks);
		else
			EvtGetEvent(&event, evtNoWait);

		// Was it a valid event ( != nilEvent)
		if (event.eType == nilEvent) 
		{
			// nil Event => the timeOut elapsed, update progress without event (for animation)
			TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: RCV nil event"));

			if (gP->prgP)
				PrgHandleEvent(gP->prgP, NULL); 

			if ((!telASyncInfoP) && (gP->curStage != kPhoneBkIOConStageWaitingPhoneBookReady))
				nowExit = true;
		}
		else if (event.eType == kTelTelephonyEvent)
		{
			telEventP = (TelEventPtr) &event;

			// Telephony event, check the transId we were looking for
			TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: RCV Tel event: TransID: %hu, error: 0x%08lX.", telEventP->transId, telEventP->returnCode));
			
			if (gP->prgP)
				PrgHandleEvent(gP->prgP, NULL);

			// Following routine is for debugging only - can be commented
			PhoneIODisplayTelephonyEvent(telEventP);
			
			if (telEventP->transId == telASyncInfoP->transId)
				// The transaction IDs match, get the result
				err = telEventP->returnCode;
			else
				// The transaction IDs do not match => BIG telephony problem !!!!
				err = phoneBkIOErrTelephonyASync;

			nowExit = true;
		}
		else if ((event.eType == kTelPrvPhoneBookReadyEvent) && (gP->curStage == kPhoneBkIOConStageWaitingPhoneBookReady))
		{
			TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: RCV Prv PhoneBookReady event"));

			if (gP->prgP)
				PrgHandleEvent(gP->prgP, NULL);

			nowExit = true;
		}
		else if (event.eType == kTelPrvAuthenticationCancelledEvent)
		{
			TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: RCV Prv kTelPrvAuthenticationCancelledEvent event"));
			err = telErrResultUserCancel;
			nowExit = true;
		}

		else
		{
			// Other event, check the user cancel
			TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: RCV Other event"));

			if (!ExgPhoneBookIOLibHandleEvent(&event))
			{
				if (! PrgHandleEvent(gP->prgP, &event)) 
				{	
					// Let the system handle this event not handled by the progress
					SysHandleEvent(&event);
					
					TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: Test user cancel"));

					if (gP->prgP && PrgUserCancel(gP->prgP)) 
					{
						if (telASyncInfoP)
							// The user canceled a telephony ASync command, cancel it.
							TelCancel(telASyncInfoP->telAppId, telASyncInfoP->transId, NULL);

						err = exgErrUserCancel;
						nowExit = true;
					}
				}
			}

			if ((!telASyncInfoP) && (gP->curStage != kPhoneBkIOConStageWaitingPhoneBookReady))
				nowExit = true;

			// If we were launched to wait telephony events, discard all other events.
		}
	}
	while (! nowExit);
	
	TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: exit with err = %x", err));
	return err;
}


/************************************************************
 *
 *  FUNCTION: PBkIOProgressUpdateDialog
 *
 *  DESCRIPTION: Updates the progress dialog
 *
 *  PARAMETERS:
 *		gP		 	- library globals
 *
 *	CALLED BY:
 *		Whoever wants to update the progress dialog
 *
 *  RETURNS: 0 if no error
 *
 * History:
 *	04/04/2002	PLe		Created
 *
 *************************************************************/
void PBkIOProgressUpdateDialog(PhoneBkIOLibGlobalsType *gP, status_t err, const char* messageP)
{
	if (err < errNone)
	{
		TraceOutput(TL(phoneBkIOErrorClass, "PBkIOProgressUpdateDialog:  err: 0x%08lX.", err));				
		
		if ((err != telErrResultUserCancel) && (err != exgErrUserCancel))
		{
			// Close the PhoneBookIO lib progress to let the telephony handle its own error
			PBkIOProgressStopDialog(gP);
			TelUiManageError(err, NULL);
		}
		else
			PBkIOProgressStopDialog(gP);
	}

	if  (gP->prgP)
	{
		TraceOutput(TL(phoneBkIOErrorClass,"PBkIOProgressProcessEvents: Update the dialog, gP->prgP= %lx, err = %lx", gP->prgP, err));
		PrgUpdateDialog(gP->prgP, err, (uint16_t) gP->curStage, messageP, true /* Update now */);
	}
}
