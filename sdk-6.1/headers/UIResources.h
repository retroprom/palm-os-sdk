/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: UIResources.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines UI resource types & ids.
 *
 *****************************************************************************/

#ifndef _UIRESOURCES_H_
#define _UIRESOURCES_H_

#include <DataMgr.h>
#include <Form.h>
#include <Menu.h>

// System Default app icon (for apps missing a tAIB)
#define defaultAppIconBitmap				10000
#define defaultAppSmallIconBitmap			10001

//	System version string ID.
#define systemVersionID						10000


//------------------------------------------------------------
// Resource Type Constants 
//------------------------------------------------------------

// Resources that are byte oriented so they are the same
// regardless of endianness or alignment of the target device.
#define ainRsc		 						'tAIN'
#define strRsc								'tSTR'
#define verRsc								'tver'
#define appInfoStringsRsc					'tAIS'
#define defaultCategoryRscType				'taic'
#define strListRscType						'tSTL'
#define midiRsc								'MIDI'
#define binaryGeneralRscType				'tbin'

// These resource were left in their 68K format
#define MenuRscType							'MBAR'
#define alertRscType	   					'Talt'
#define bitmapRsc	 						'Tbmp'
#define bsBitmapRsc		 					'Tbsb'
#define iconType							'tAIB'
#define formRscType							'tFRM'
#define constraintsRscType					'frmb'
#define colorTableRsc						'tclt'
#define constantRscType						'tint'
#define fontRscType							'NFNT'
#define fontExtRscType						'nfnt'
#define fontIndexType						'fnti'

#ifdef INCLUDE_ALIGN_16_STRUCT
#define kbdRscTypeBE16						'tkbd'
#define byteListRscTypeBE16					'BLST'
#define wrdListRscTypeBE16					'wrdl'
#define dwordListRscTypeBE16				'DLST'
#endif

#define kbdRscType							'akbd'
#define byteListRscType						'abyt'
#define wrdListRscType						'awrd'
#define dwordListRscType					'adwd'
#define formNavRscType						'fnav'

//------------------------------------------------------------
// Basic list resource types
//------------------------------------------------------------
// uint8_t List Resource Structure
typedef struct ByteListRscType {
	uint16_t			defItem;							// default item (1-based)
	uint16_t			count;								// number of items
	uint8_t			item[1];							// variable number of items
	uint8_t			reserved;							//Should this even be there??? ADH
	} ByteListRscType;
typedef ByteListRscType*	ByteListRscPtr;

// uint16_t List Resource Structure - this type is older and
// often not used.  The resource is often just locked down and
// cast as an array of UInt16s.  It doesn't have a defItem field
// like the other two list types.
typedef struct WordListRscType {
	uint16_t			count;								// number of items
	uint16_t			item[1];							// variable number of items
	} WordListRscType;
typedef WordListRscType*	WordListRscPtr;

// uint32_t List Resource Structure
typedef struct DWordListRscType {
	uint16_t			defItem;							// default item (1-based)
	uint16_t			count;								// number of items
	uint32_t			item[1];							// variable number of items
	} DWordListRscType;
typedef DWordListRscType*	DWordListRscPtr;

//------------------------------------------------------------
// App Version Constants 
//------------------------------------------------------------

#define appVersionID						1			// our apps use tver 1 resource
#define appVersionAlternateID				1000		// CW Constructor uses tver 1000 resource
														// so we'll look for ours first, then try theirs
#define ainID								1000

#define oemVersionID						10001		// Per-DB version provided by OEMs


// DOLATER kwk - should resource ids >= 10000 be in a private header file, so that
// developers know they're not guaranteed to be around (or in the same format)?


//------------------------------------------------------------
// System Information Constants 
//------------------------------------------------------------

#define	maxCategoryWidthID					10001		// Max pixel width for category trigger.
#define  exchangeLibraryInterfaceID			10002

//------------------------------------------------------------
// System Alerts 
//------------------------------------------------------------

#define SelectACategoryAlert				10000

// This alert broke 1.0 applications and is now disabled until later.
// It is redefined below (10015).
//#define RemoveCategoryAlert				10001
//#define RemoveCategoryRecordsButton		0
//#define RemoveCategoryNameButton			1
//#define RemoveCategoryCancelButton		2

#define LowBatteryAlert						10002
#define VeryLowBatteryAlert					10003
#define UndoAlert							10004
#define UndoCancelButton					1

#define MergeCategoryAlert					10005
#define MergeCategoryYes					0
#define MergeCategoryNo						1

#define privateRecordInfoAlert				10006

#define ClipboardLimitAlert					10007

#define CategoryExistsAlert					10012

#define DeviceFullAlert						10013

#define categoryAllUsedAlert				10014

#define RemoveCategoryAlert					10015		// See alert 10001
#define RemoveCategoryYes					0
#define RemoveCategoryNo					1

#define DemoUnitAlert						10016

// The "no data to send" message is a shared error message that is displayed
// when no data is selected when a beam or send command is issued.
#define NoDataToBeamAlert					10017
#define NoDataToSendAlert					10017

// New for PalmOS 3.1
#define LowCradleChargedBatteryAlert		10018		// (Not present in Palm VII)
#define VeryLowCradleChargedBatteryAlert	10019		// (Not present in Palm VII)

// New for PalmOS 3.1 (Instant Karma only)
#define CategoryTooLongAlert				10020		// (Not present in Palm VII)

// New for PalmOS 3.2 - Alerts used by the ErrAlertCustom()  call.
#define ErrOKAlert							10021		// Error Alert with just an OK button
#define ErrOKCancelAlert					10022		// Error Alert with an OK & Cancel button
#define ErrCancelAlert						10023		// Error Alert with just Cancel button.  Special case for antenna down alert.
#define InfoOKAlert							10024		// Info alert with just an OK button
#define InfoOKCancelAlert					10025		// Info alert with an OK & Cancel button
#define InfoCancelAlert						10026		// Info alert with just a Cancel button
#define PrivacyWarningAlert					10027    	// Privacy warning for weblib
#define ConfirmationOKAlert					10028		// Confirmation alert with just an OK button
#define ConfirmationOKCancelAlert			10029		// Confirmation alert with an OK & Cancel button
#define ConfirmationCancelAlert				10030		// Confirmation alert with just a Cancel button
#define WarningOKAlert						10031		// Warning Alert with just an OK button
#define WarningOKCancelAlert				10032		// Warning Alert with an OK & Cancel button
#define WarningCancelAlert					10033		// Warning Alert with just Cancel button.  Special case for antenna down alert.

// New for PalmOS 3.5 - Fatal Alert template
#define sysFatalAlert						10100		// Template for fatal alert

// New for PalmOS 3.5 - Alerts used by new security traps
#define secInvalidPasswordAlert				13250
#define secGotoInvalidRecordAlert			13251
#define secShowPrivatePermanentPassEntryAlert		13261
#define secShowMaskedPrivatePermanentPassEntryAlert	13265
#define secHideRecordsAlert					13268
#define secMaskRecordsAlert					13269
#define secHideMaskRecordsOK				0
#define secHideMaskRecordsCancel			1

// New for PalmOS 4.0 -  General purpose password prompt alert
#define secEnterPasswordAlert				13300
#define secEnterPasswordOK					0
#define secEnterPasswordCancel				1

// New for PalmOS 6.0 -  Invalid system date
#define InvalidSystemDateAlert				13400


// command-bar bitmaps
#define BarCutBitmap						10030
#define BarCopyBitmap						10031
#define BarPasteBitmap						10032
#define BarUndoBitmap						10033
#define BarBeamBitmap						10034
#define BarSecureBitmap						10035
#define BarDeleteBitmap						10036
#define BarInfoBitmap						10037

//Masking bitmaps
#define SecLockBitmap						10050
#define SecLockWidth						6
#define SecLockHeight						8

// System Menu Bar and Menus
#define sysEditMenuID						10000
#define sysEditMenuUndoCmd					10000
#define sysEditMenuCutCmd					10001
#define sysEditMenuCopyCmd					10002
#define sysEditMenuPasteCmd					10003
#define sysEditMenuSelectAllCmd				10004
#define sysEditMenuSeparator				10005
#define sysEditMenuKeyboardCmd				10006
#define sysEditMenuGraffitiCmd				10007

#define sysNetworkProgress01Bitmap			10020
#define sysNetworkProgress02Bitmap			10021
#define sysNetworkProgress03Bitmap			10022
#define sysNetworkProgress04Bitmap			10023
#define sysNetworkProgress05Bitmap			10024
#define sysNetworkProgress06Bitmap			10025

#define sysErrorBitmap						10046		// resource ID of /!\ error picture

// Dynamically added to System Edit menu at runtime
#define sysEditMenuAddFepWord				10100		// was sysEditMenuJapAddWord
#define sysEditMenuLookupWord				10101		// was sysEditMenuJapLookupWord

// Note View Menu Bar and Menus
#define noteMenuID							10200		// Old NoteView MenuBar
#define noteUndoCmd							sysEditMenuUndoCmd	
#define noteCutCmd							sysEditMenuCutCmd
#define noteCopyCmd							sysEditMenuCopyCmd
#define notePasteCmd						sysEditMenuPasteCmd
#define noteSelectAllCmd					sysEditMenuSelectAllCmd
#define noteSeparator						sysEditMenuSeparator
#define noteKeyboardCmd						sysEditMenuKeyboardCmd
#define noteGraffitiCmd						sysEditMenuGraffitiCmd

#define noteFontCmd							10200		// These are here for backwards
#define noteTopOfPageCmd					10201		// compatibility.  The built-in
#define noteBottomOfPageCmd					10202		// apps no longer use them.
#define notePhoneLookupCmd					10203

#define newNoteMenuID						10300		// The Edit Menu for the new NoteView.
#define newNoteFontCmd						10300		// MenuBar is the same as it is for
#define newNotePhoneLookupCmd				10301		// the old NoteView MenuBar.

// Note View (used by Datebook, To Do, Address, and Expense apps)
#define NoteView							10900		// The new NoteView is "new" as of Palm OS 3.5.
#define NewNoteView							10950		// Same as old NoteView, but points to newNoteMenuID and doesn't ref UI objects listed below.
#define NoteField							10901
#define NoteDoneButton						10902
#define NoteSmallFontButton					10903		// Not in NewNoteView, use FontCmd instead.
#define NoteLargeFontButton					10904		// Not in NewNoteView, use FontCmd instead.
#define NoteDeleteButton					10905
#define NoteUpButton						10906		// Not in NewNoteView, use scrollbars now.
#define NoteDownButton						10907		// Not in NewNoteView, use scrollbars now.
#define NoteScrollBar						10908
#define NoteFontGroup						1
#define noteViewMaxLength					0xffff		// not including null, tied to tFLD rsrc 10901

//	About Box - used by Datebook, Memo, Address, To Do, & others
#define aboutDialog							11000
#define aboutNameLabel						11001
#define aboutVersionLabel					11002
#define aboutErrorStr						11003

// Category New Name Dialog (used for new and renamed categories)
#define categoryNewNameDialog				11100
#define categoryNewNameField				11103
#define categoryNewNameOKButton				11104

// Categories Edit Dialog
#define CategoriesEditForm					10000
#define CategoriesEditList					10002
#define CategoriesEditOKButton				10003
#define CategoriesEditNewButton				10004
#define CategoriesEditRenameButton			10005
#define CategoriesEditDeleteButton			10006
#define CatMgrSelectMultipleForm			10007
#define CatMgrSelectMultipleTable			10008
#define CatMgrSelectMultipleOKButton		10009
#define CatMgrSelectMultipleCancelButton	10010
#define CatMgrSelectMultipleScrollBar		10011


// Graffiti Reference Dialog
#define graffitiReferenceDialog				11200
#define graffitiReferenceDoneButton			11202
#define graffitiReferenceUpButton			11203
#define graffitiReferenceDownButton			11204
#define graffitiReferenceFirstBitmap		11205

// System string resources
#define categoryAllStrID					10004
#define categoryEditStrID  					10005
#define menuCommandStrID					10006
#define launcherBatteryStrID				10007
#define phoneLookupTitleStrID				10009
#define phoneLookupAddStrID					10010
#define phoneLookupFormatStrID				10011
#define categoryRecCountSuffixStrID  		10012
#define categoryMultipleStrID  				10013


// DOLATER kwk - other strings are defined in UIResourcesPrv.h.
// We should have one header file with all of the system.prc resources,
// or explicit ranges for public/private & UI/system.


//------------------------------------------------------------
// Misc. resource routines 
//------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

FormType*		ResLoadForm(DmOpenRef database, DmResourceID rscID);

// Like above, but override any associated constraints' window flags.
FormType*		ResLoadFormWithFlags(DmOpenRef database, DmResourceID rscID, WinFlagsType windowFlags);

MenuBarType*	ResLoadMenu(DmOpenRef database, DmResourceID rscID);

uint32_t		ResLoadConstant(DmOpenRef database, DmResourceID rscID);

// Load a string resource w/ID = <rscID> into <strP>. Limit the length
// of the string to be <maxLen> bytes.
char*			ResLoadString(DmOpenRef database, DmResourceID rscID, char* strP, size_t maxLen);

// Compatibility routines.
void *	ResLoadFormV50(DmResourceID rscID);
void *	ResLoadMenuV50(DmResourceID rscID);
uint32_t	ResLoadConstantV50(DmResourceID rscID);

#ifdef __cplusplus 
}
#endif

#endif // _UIRESOURCES_H_
