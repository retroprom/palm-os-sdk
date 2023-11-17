/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Address.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESS_H_
#define _ADDRESS_H_

#include <PrivateRecords.h>
#include <CatMgr.h>
#include <DataMgr.h>
#include <LocaleMgrTypes.h>

#include "AddressIdLookup.h"

/************************************************************
 * Constants
 *************************************************************/

#define kAttachTransportPrefixMaxSize		16
#define kMaxOrderBySQLStrSize				256

#define kOrderByNameType					10
#define kOrderByCompanyType					20

#define kDateTimeSeparatorMaxLength			5
#define kMaxDateTimeStringSize				(timeStringLength + longDateStrLength + kDateTimeSeparatorMaxLength + 1)

#define kCategoryLabelLength				(catCategoryNameLength * 2)

#define kSpaceBetweenNamesAndPhoneNumbers			4
#define kSpaceBetweenPhoneNumbersAndAbbreviation	1

#define kNamePopupTimeout					4000 // ms
#define kNamePopupEventTimeOut				100

// #### LFe: could be remove ?
#define kStringMaxSize						256

#define kTblInvalidRowIndex					((int16_t)-1)

/************************************************************
 * Additionnal Event codes
 *************************************************************/

#define kFrmCustomUpdateEvent				((uint16_t)(firstUserEvent + 1))
#define kFrmDetailsUpdateEvent				((uint16_t)(firstUserEvent + 2))
#define kListViewReloadTableEvent			((uint16_t)(firstUserEvent + 3))
#define kEditViewRestoreEditStateEvent		((uint16_t)(firstUserEvent + 4))
#define kEditViewReleaseFocusEvent			((uint16_t)(firstUserEvent + 5))

#define PrvPostUserEvent(evtType)		\
	do {								\
		EventType	evt;				\
		memset(&evt, 0, sizeof(evt));	\
		evt.eType = evtType;			\
		EvtAddEventToQueue(&evt);		\
	} while (0)

/************************************************************
 * Additionnal launch codes
 *************************************************************/

#define appLaunchCmdExgGetFullLaunch		(AddressBookIdLookupLaunch + 1)

/************************************************************
 * Global Variables
 *************************************************************/

extern uint32_t					gListViewSelectThisRowID;
extern Boolean					gEnableTapDialing;
extern uint8_t					gTransfertMode;
extern Boolean					gTransfertNote[4];
extern uint32_t					gCurrentNumCategories;
extern CategoryID *				gCurrentCategoriesP;
extern char						gCategoryName[kCategoryLabelLength + 1];
extern DmOpenRef				gAddrDB;
extern DmOpenRef				gApplicationDbP;
extern uint32_t					gCursorID;
extern DatabaseID				gAddrDatabaseID;
extern uint32_t       			gTopVisibleRowIndex;
extern FontID					gAddrListFont;
extern uint32_t          		gCurrentRowID;

extern LmLocaleType				gFormatLocale;
extern LmLocaleType				gSystemLocale;

extern privateRecordViewEnum	gPrivateRecordVisualStatus;

extern int16_t					gOrderByIndex;
extern int16_t					gOrderByCount;
extern char						gCurrentOrderByStr[kMaxOrderBySQLStrSize];
extern uint32_t					gCurrentOrderByType;

extern char *					gUnnamedRecordStringP;

// These are used for controlling the display of the duplicated address records.
extern uint16_t					gNumCharsToHilite;

// List view : phone label width. This value is computed at application start to
// increase list displaying.
extern int16_t	 				gPhoneLabelWidth;

// The following global variable are saved to a state file.
extern Boolean					gSaveBackup;
extern Boolean					gRememberLastCategory;
extern FontID					gNoteFont;
extern FontID					gAddrRecordFont;
extern FontID					gAddrEditFont;
extern uint32_t					gBusinessCardRowID;

extern Boolean					gDialerPresentChecked;
extern Boolean					gDialerPresent;

// Active Input Area support
extern RectangleType			gCurrentWinBounds;		// Current window bounds. Set at frmLoadEvent

// If true, the ADB has been sublaunched to display the main view and should return to the calling application.
extern Boolean					gCalledFromApp;
extern LocalID					gCallerAppDbID;

// If true, the ADB has been sublaunched to select an attachment
// The cmdPBP argument contains the application creator ID that expects the data.
extern Boolean					gAttachRequest;
extern char						gTransportPrefix[kAttachTransportPrefixMaxSize];

// Set by RecordView when the user taps within the display area
// to edit. It's used by the EditView to give the focus to the
// corresponding field.
extern uint32_t					gTappedColumnID;

// EvtGetEvent timeout param
extern int32_t					gEvtGetEventTimeout;

// Device has FEP installed
extern Boolean					gDeviceHasFEP;

// Template globals. To avoid to recompute the resource ID each time.
extern DmResourceID				gFFNLayoutResID;
extern DmResourceID				gDisplayNameLayoutResID;
extern DmResourceID				gAddressLayoutResID;
extern DmResourceID				gFFNColumnListResID;
extern DmResourceID				gDisplayNameColumnListResID;

#endif // _ADDRESS_H_
