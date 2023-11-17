/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressPrefs.h
 *
 * Release: Palm OS 6.0.1
 *
 *****************************************************************************/

#ifndef _ADDRESSPREFS_H_
#define _ADDRESSPREFS_H_

#include <Event.h>
#include <DataMgr.h>

/***********************************************************************
 *	Defines
 ***********************************************************************/

#define kAddrPrefVersionNum				0x06
#define kAddrPrefID						0x00
#define kAddrCurrentCategoriesPrefID	0x01
#define kAddrSystemLocalePrefID			0x02

#define kPrefTransfertAllRecord			0x00
#define kPrefTransfertCurrentTabRecord	0x01
#define kPrefTransfertAskRecord			0x02

/***********************************************************************
 *   Internal Structures
 ***********************************************************************/

typedef struct AddrPreferenceTag
{
	Boolean			saveBackup;
	Boolean			rememberLastCategory;

	FontID			addrListFont;
	FontID			addrRecordFont;
	FontID			addrEditFont;
	FontID			noteFont;

	Boolean			enableTapDialing;

	uint8_t			transfertMode;
	Boolean			transfertNote[4];		// One for each family + 1 for "All"
	
	uint32_t		businessCardRowID;
	int16_t			orderByIndex;

} AddrPreferenceType;

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void	PrefsLoad(DmOpenRef appDbRef);
void	PrefsSave(void);
void	PrefsLoadCurrentCategories(uint32_t *numCategoriesP, CategoryID **categoriesPP);
void	PrefsSaveCurrentCategories(uint32_t numCategories, CategoryID *categoriesP);

Boolean	PrefsHandleEvent (EventType * event);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSPREFS_H_
