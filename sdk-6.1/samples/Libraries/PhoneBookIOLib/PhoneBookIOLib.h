/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PhoneBookIOLib.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Public declarations for the Phone Book Import / Export exchange library.
 *  If this exchange library is loaded, two new menu items are added in the
 *  address book list view menu: Import Phone Category & Export Phone Category.
 *
 *      This library can be used by any application which wants to import/export
 *	with a phone. The format used to support names and phone numbers is the vCard
 *	format. Use the PDI lib if you want to create your own vCards and send them
 *	to this library.
 *
 *		PhoneBookIOLib supports the following calls for exporting:
 *			- ExgPut()
 *			- ExgSend() [Do as many ExgSend() as needed to send all data]
 *			- ExgDisconnect()
 *
 *		PhoneBookIOLib supports the following calls for importing:
 *			- ExgGet()
 *			- ExgReceive() [Do as many ExgReceive() as needed to receive all data]
 *			- ExgDisconnect()
 *
 *		PhoneBookIOLib preferences can be set by the ExgControl() call.
 *
 *****************************************************************************/

#ifndef __PHONE_BOOK_IO_LIB_H__
#define __PHONE_BOOK_IO_LIB_H__

// *******************************************************************************
//	Defines
// *******************************************************************************

// Specific scheme for PhoneBookIO exg lib
#define exgPhoneBookIOScheme				"_PhoneBookIO"
#define exgPhoneBookIOPrefix				(exgPhoneBookIOScheme ":")

// Constants for phone book selection
#define kPhoneBkSelectSIM					((uint8_t)0)	// kTelPhbSIM
#define kPhoneBkSelectME					((uint8_t)1)	// kTelPhbME

//typedef uint8_t PBkIOSelectType;

// Control opCodes to be used with ExgControl
//
//   - phoneBkIOLibCtlSetPhoneRecipient
//      Set the phone book to import/export, to choose between phoneBkSelectSIM,
//		phoneBkSelectME... valueP is a pointer on a PBkIOSelectType variable.
//
//   - phoneBkIOLibCtlAskPhoneRecipient
//      Displays a dialog for PhoneBookIO preferences, setting the
//    phone book to set/get (SIM, Internal, ...)
//
#define phoneBkIOLibCtlSetPhoneRecipient	exgLibCtlSpecificOp
#define phoneBkIOLibCtlAskPhoneRecipient	(exgLibCtlSpecificOp + 1)

#endif // __PHONE_BOOK_IO_LIB_H__
