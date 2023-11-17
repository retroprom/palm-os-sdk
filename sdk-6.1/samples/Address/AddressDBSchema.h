/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressDBSchema.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSDBSCHEMA_H_
#define _ADDRESSDBSCHEMA_H_

#include <LocaleMgr.h>
#include <DataMgr.h>

#include "AddressTab.h"

/***********************************************************************
 *   Defines
 ***********************************************************************/

#define kAddrDBType								'DATA'
#define kAddrDBName								"RAM_AddressDBS"
#define kAddrPreviewDBName						"RAM_AddressPreviewDBS"

#define kFirstRowIndex							((uint32_t) 0x01)

// Schemas info
#define kAddressDefaultTableName				"RAM_AddressBookDBSchema"
#define kAddressAppInfoTableName				"AddressBookAppInfo"

#define kAddressAppInfoColumnIDCountry			((uint32_t) 100)

// Properties definition
#define kAddrColumnPropertyTabId				((DbSchemaColumnProperty) 64)	// uint16_t
#define kAddrColumnPropertyTabName				((DbSchemaColumnProperty) 65)	// char*
#define kAddrColumnPropertyTabIconResId			((DbSchemaColumnProperty) 66)	// uint16_t
#define kAddrColumnPropertyTabSelectedIconResId	((DbSchemaColumnProperty) 67)	// uint16_t
#define kAddrColumnPropertyTabSkip				((DbSchemaColumnProperty) 68)	// uint8_t
#define kAddrColumnPropertyTabDefaultView		((DbSchemaColumnProperty) 69)	// uint8_t (Boolean)

#define kAddrColumnPropertyName					((DbSchemaColumnProperty) 80)	// char*
#define kAddrColumnPropertyRenameAllowed		((DbSchemaColumnProperty) 81)	// uint8_t
#define kAddrColumnPropertyNewName				((DbSchemaColumnProperty) 82)	// char*
#define kAddrColumnPropertyNoLabel				((DbSchemaColumnProperty) 83)	// uint8_t
#define kAddrColumnPropertyBlankLineAfter		((DbSchemaColumnProperty) 84)	// uint8_t
#define kAddrColumnPropertyDisplayIndex			((DbSchemaColumnProperty) 85)	// uint8_t
#define kAddrColumnPropertyEditFirstField		((DbSchemaColumnProperty) 86)	// uint8_t (Boolean)
#define kAddrColumnPropertyHiddenOnRecordView	((DbSchemaColumnProperty) 87)	// uint8_t (Boolean)
#define kAddrColumnPropertyAddressGroupID		((DbSchemaColumnProperty) 88)	// uint8_t (Boolean)

#define kAddrColumnPropertyFieldInfo			((DbSchemaColumnProperty) 96)	// uint32_t
#define kAddrColumnPropertyAutoFillDbType		((DbSchemaColumnProperty) 97)	// uint32_t (Db type)
#define kAddrColumnPropertyAutoFillInitStr		((DbSchemaColumnProperty) 98)	// ResID/uint16_t (tSTL)
#define kAddrColumnPropertyNotTransferable		((DbSchemaColumnProperty) 99)	// uint8_t (Boolean)


// Column ID definition in Schema
// Personal & Home information
#define kAddrColumnIDTitle						((uint32_t) 100)
#define kAddrColumnIDLastName					((uint32_t) 200)
#define kAddrColumnIDMiddleName					((uint32_t) 300)
#define kAddrColumnIDFirstName					((uint32_t) 400)
#define kAddrColumnIDSuffix						((uint32_t) 500)
#define kAddrColumnIDNickName					((uint32_t) 600)
#define kAddrColumnIDProfession					((uint32_t) 700)
#define kAddrColumnIDHomePhone					((uint32_t) 800)
#define kAddrColumnIDHomeMobile					((uint32_t) 900)
#define kAddrColumnIDHomeFax					((uint32_t) 1000)
#define kAddrColumnIDHomeEmail1					((uint32_t) 1100)
#define kAddrColumnIDHomeEmail2					((uint32_t) 1200)
#define kAddrColumnIDHomeEmail3					((uint32_t) 1300)
#define kAddrColumnIDHomeStreet					((uint32_t) 1400)
#define kAddrColumnIDHomeCity					((uint32_t) 1500)
#define kAddrColumnIDHomeState					((uint32_t) 1600)
#define kAddrColumnIDHomeZipCode				((uint32_t) 1700)
#define kAddrColumnIDHomeCountry				((uint32_t) 1800)
#define kAddrColumnIDAnniversary				((uint32_t) 1900)
#define kAddrColumnIDBirthday					((uint32_t) 2000)

// Business information
#define kAddrColumnIDWorkCompany				((uint32_t) 3000)
#define kAddrColumnIDWorkTitle					((uint32_t) 3100)
#define kAddrColumnIDWorkStreet					((uint32_t) 3200)
#define kAddrColumnIDWorkCity					((uint32_t) 3300)
#define kAddrColumnIDWorkState					((uint32_t) 3400)
#define kAddrColumnIDWorkZipCode				((uint32_t) 3500)
#define kAddrColumnIDWorkCountry				((uint32_t) 3600)
#define kAddrColumnIDWorkPhone					((uint32_t) 3700)
#define kAddrColumnIDWorkMobile					((uint32_t) 3800)
#define kAddrColumnIDWorkFax					((uint32_t) 3900)
#define kAddrColumnIDWorkEmail					((uint32_t) 4000)
#define kAddrColumnIDWorkAssistantName			((uint32_t) 4100)
#define kAddrColumnIDWorkAssistantPhone			((uint32_t) 4200)

// Other information
#define kAddrColumnIDOtherStreet				((uint32_t) 6000)
#define kAddrColumnIDOtherCity					((uint32_t) 6100)
#define kAddrColumnIDOtherState					((uint32_t) 6200)
#define kAddrColumnIDOtherZipCode				((uint32_t) 6300)
#define kAddrColumnIDOtherCountry				((uint32_t) 6400)
#define kAddrColumnIDOtherPhone					((uint32_t) 6500)
#define kAddrColumnIDOtherMobile				((uint32_t) 6600)
#define kAddrColumnIDOtherFax					((uint32_t) 6700)
#define kAddrColumnIDOtherPager					((uint32_t) 6800)
#define kAddrColumnIDOtherCustom1				((uint32_t) 6900)
#define kAddrColumnIDOtherCustom2				((uint32_t) 7000)
#define kAddrColumnIDOtherCustom3				((uint32_t) 7100)
#define kAddrColumnIDOtherCustom4				((uint32_t) 7200)
//...
#define kAddrColumnIDOtherICQ					((uint32_t) 7500)
#define kAddrColumnIDOtherAIM					((uint32_t) 7600)
#define kAddrColumnIDOtherYahoo					((uint32_t) 7700)
#define kAddrColumnIDOtherMSN					((uint32_t) 7800)
#define kAddrColumnIDOtherJabber				((uint32_t) 7900)
//...
#define kAddrColumnIDOtherURL					((uint32_t) 8500)

// Common information
#define kAddrColumnIDNote						((uint32_t) 9000)
#define kAddrColumnIDDisplayedPhone				((uint32_t) 9100)
#define kAddrColumnIDLastVisibleTabID			((uint32_t) 9200)

// Specific Japanese information
#define kAddrColumnIDYomiLastName				((uint32_t) 10000)
#define kAddrColumnIDYomiFirstName				((uint32_t) 10100)
#define kAddrColumnIDYomiCompanyName			((uint32_t) 10200)

// Specific Chinese information
#define kAddrColumnIDEnglishName				((uint32_t) 11000)

// AppInfo schema definition
#define kAddrAppInfoCountryColID				((uint32_t) 100)

// Missing Invalid definition in Database Manager
#define kInvalidColumnID						((uint32_t) 0xFFFFFFFF)
#define kInvalidRowIndex						((uint32_t) 0xFFFFFFFF)

// Base ID for resources used to rename schema's fields depending
// of the country code (got from local manager)
// The country code will be converted using an ISO 3166 function
// to a int (value between 1101 to 1876)
// This will reserve a range of u32l and strl from 1101 to 1876
#define kAddrRenameFieldStrListBaseID			0
#define kAddrRenameFieldFieldInfoU32LBaseID		0

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

status_t	AddressDBOpenDatabase (DmOpenRef appDbRef, uint16_t mode, DmOpenRef *dbRef);

void		AddressDBCloseDatabase(void);

status_t	AddressDBOpenCursor(void);

status_t	AddressDBNewRecord(DmOpenRef dbRef, uint32_t *rowIDP);

status_t	AddressDBCheckCountry(DmOpenRef dbRef);

uint32_t	AddressDBCheckAndSetDisplayedPhoneColumn(DmOpenRef dbRef, uint32_t rowID);

Boolean		AddressDBColumnsContainData(uint32_t rowID, uint32_t numColumns, uint32_t *columnsP);

Boolean		AddressDBRecordContainsData(uint32_t rowID);

Boolean		AddressDBColumnPropertyExists(DmOpenRef dbP, char *tableNameP, uint32_t columnID, DbSchemaColumnProperty propertyID);

void		AddressDBSetDatabaseAttributes(DatabaseID dbID, uint16_t attrBits);

void		AddressDBDeleteRecord(Boolean archive);

void		AddressDBDuplicateRecord(uint32_t rowID, uint32_t *newRowIDP, uint16_t *numCharsToHiliteP, uint32_t *columnIDToHiliteP);

Boolean		AddressDBVerifyPassword(void);

status_t	AddressDBCreateDefaultDatabase(DmOpenRef appDbP);

Boolean		AddressDBSaveOverlayLocale(LmLocaleType *ovlyLocaleP);

Boolean		AddressDBGetSavedOverlayLocale(LmLocaleType *localeTypeP);

status_t	AddressDBOpenPreviewDatabase (DmOpenRef adbDbRef, DmOpenRef *previewDbRefP);

status_t	AddressDBDeletePreviewDatabase(void);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSDBSCHEMA_H_
