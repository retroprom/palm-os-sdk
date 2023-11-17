/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PhoneBookIOLibPrv.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Private declarations for the Phone Book Import / Export exchange library.
 *
 *****************************************************************************/

#ifndef __PHONE_BOOK_IO_PRV_H__
#define __PHONE_BOOK_IO_PRV_H__

// System includes
#include <Progress.h>
#include <ExgMgr.h>
#include <FileStream.h>

// Public PhoneBookIOLib include file
#include "PhoneBookIOLib.h"

// *******************************************************************************
//	Defines
// *******************************************************************************

#define phoneBkIOErrorClass 				appErrorClass
#define phoneBkIOErrEmptyRec 				(phoneBkIOErrorClass | 1)
#define phoneBkIOErrNotConnected 			(phoneBkIOErrorClass | 2)
#define phoneBkIOErrTelephonyASync 			(phoneBkIOErrorClass | 3)
#define phoneBkIOPhoneBookEmpty 			(phoneBkIOErrorClass | 4)

#define kPhoneBookIOFtrStorageState				0x0001
#define kPhoneBookIOFtrConnectionState			0x0002
#define kPhoneBookIOFtrSelectedPhoneBook		0x0003

#define kPhoneBookIOStorageReady			1
#define kPhoneBookIOStorageNotReady			0

#define kPhoneBookIOConnectionOn			1
#define kPhoneBookIOConnectionOff			0

#define kPhoneBookIOPhbSIM					0
#define kPhoneBookIOPhbME					1

#define kTelPrvPhoneBookReadyEvent			firstUserEvent + 0x1000
#define kTelPrvPhoneConnectionOnEvent		firstUserEvent + 0x1001
#define kTelPrvAuthenticationCancelledEvent	firstUserEvent + 0x1002


// *******************************************************************************
// Types definition
// *******************************************************************************

// Stages of a connection. The order of following constants must match
// the string list in resources.
#define kPhoneBkIOConStageInvalid				((uint16_t)0)	// invalid stage 
#define kPhoneBkIOConStageConnectingPhone		((uint16_t)1)	// Connecting to the phone
#define kPhoneBkIOConStageWaitingPhoneBookReady	((uint16_t)2)	// waiting for PhoneBookReady
#define kPhoneBkIOConStageSending				((uint16_t)3)	// sending data
#define kPhoneBkIOConStageSendvCard				((uint16_t)4)	// sending the vCard
#define kPhoneBkIOConStageReceiving				((uint16_t)5)	// receiving data
#define kPhoneBkIOConStageRecvCard				((uint16_t)6)	// receiving the vCard
#define kPhoneBkIOConStageDeleting				((uint16_t)7)	// receiving data
#define kPhoneBkIOConStageDisconnecting			((uint16_t)8)	// disconnecting
#define kPhoneBkIOConStageCanceling				((uint16_t)9)	// canceling

#define kPhoneBkIOStrIndexInvalid				((uint16_t)0)	// invalid stage 
#define kPhoneBkIOStrIndexConnectingPhone		((uint16_t)1)	// Connecting to the phone
#define kPhoneBkIOStrIndexSending				((uint16_t)2)	// sending data
#define kPhoneBkIOStrIndexSendvCard				((uint16_t)3)	// sending the vCard
#define kPhoneBkIOStrIndexReceiving				((uint16_t)4)	// receiving data
#define kPhoneBkIOStrIndexRecvCard				((uint16_t)5)	// receiving the vCard
#define kPhoneBkIOStrIndexDeleting				((uint16_t)6)	// receiving data
#define kPhoneBkIOStrIndexDisconnecting			((uint16_t)7)	// disconnecting
#define kPhoneBkIOStrIndexCanceling				((uint16_t)8)	// canceling


// Some errors are caught to display customized error messages, following
// is the list errors matching the error string list in resources.
#define errMessageTimeOut	((uint16_t)0)


// Enum for operations in progress
#define kPhoneBkIOOpNone	((uint8_t)0)
#define kPhoneBkIOOpPut		((uint8_t)1)
#define kPhoneBkIOOpGet		((uint8_t)2)


// Structure pointed to by ExgSocketType.socketRef
typedef struct PhoneBkIOSocketInfoTag
{
	uint8_t				op;					// operation in progress, if any
    Boolean 			freeOnDisconnect;	// whether to free this struct when the operation completes
	uint16_t			padding;
	status_t 			err;				// error code to return to outer caller of ExgDisconnect
	FileHand 			tempFileH;			// temp buffer
} PhoneBkIOSocketInfoType;	


// Global data structure for this library
typedef struct PhoneBkIOLibGlobalsTag
{
	int16_t 					openCount;				// how many times I've been opened	
	uint8_t						padding1;		// SIM, Internal...
	uint8_t						padding2;
	ProgressType *				prgP;					// progress dialog globals
	DmOpenRef 					resourceDBP;			// this library's resources
	uint16_t			 		curStage;				// current stage
	uint16_t					padding3;
} PhoneBkIOLibGlobalsType;	


// *******************************************************************************
// Exported functions
// *******************************************************************************
status_t 	ExgPhoneBookIOLibOpen(void);
status_t 	ExgPhoneBookIOLibClose(void);
status_t 	ExgPhoneBookIOLibWake(void);
status_t 	ExgPhoneBookIOLibSleep(void);
status_t 	ExgPhoneBookIOLibRequest(ExgSocketType *socketP);
Boolean		ExgPhoneBookIOLibHandleEvent(void *eventP);
status_t 	ExgPhoneBookIOLibConnect(ExgSocketType *socketP);
status_t 	ExgPhoneBookIOLibGet(ExgSocketType *socketP);
status_t 	ExgPhoneBookIOLibPut(ExgSocketType *socketP);
status_t 	ExgPhoneBookIOLibAccept(ExgSocketType *socketP);
status_t 	ExgPhoneBookIOLibDisconnect(ExgSocketType *socketP, status_t appError);
uint32_t 	ExgPhoneBookIOLibSend(ExgSocketType* socketP, void* bufP, const uint32_t bufferSize, status_t* errP);
uint32_t 	ExgPhoneBookIOLibReceive(ExgSocketType* socketP, void* bufP, const uint32_t bufSize, status_t* errP);
status_t 	ExgPhoneBookIOLibControl(uint16_t op, void *valueP, uint16_t *valueLenP);

#endif // __PHONE_BOOK_IO_PRV_H__
