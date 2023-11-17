/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressTab.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSTAB_H_
#define _ADDRESSTAB_H_

#include <PalmTypes.h>
#include <SchemaDatabases.h>
#include <AddressFields.h>

#include "Books.h"

/*-------------------------------------------------------------------
	Public constants
---------------------------------------------------------------------*/

#define kTabAllId				((uint16_t) 0xFFFE)

#define TabIsVisible(tabId)		(tabId != kBookInvalidTabId)

#define kTabMinWidth			((uint16_t) 15)
#define kTabMaxWidth			((uint16_t) 60)

#define kFieldClass_Standard	((uint8_t) 0)
#define kFieldClass_TabName		((uint8_t) 1)
#define kFieldClass_Separator	((uint8_t) 2)
#define kFieldClass_Prefix		((uint8_t) 3)
#define kFieldClass_Suffix		((uint8_t) 4)
#define kFieldClass_BlankLine	((uint8_t) 5)

/*-------------------------------------------------------------------
	Public structures
---------------------------------------------------------------------*/

typedef struct AddressTabGraphicInfoTag
{
	char *			tabNameP;
	uint16_t		tabId;
	DmResourceID	tabIconId;
	DmResourceID	tabSelectedIconId;
	uint16_t		padding;
} AddressTabGraphicInfoType, *AddressTabGraphicInfoPtr;

typedef struct AddressTabColumnPropertiesTag
{
	uint32_t				columnId;					// Id of the column in the database schema
	uint32_t				fieldInfo;					// Fields Info Properties of the columns
	uint32_t				autoFillDbType;				// autoFill DB type used for this column. 0 for none
	uint16_t				autoFillDbInitStr;			// autoFill default init StrList
	uint16_t				displayIndex; 				// Display order properties of the column
	uint32_t				maxDataSize;				// Maximum data size for VarChar Field
	DbSchemaColumnType		columnDataType;				// data type  of the column in the database schema

	uint8_t					noLabel;					// Don't display the label of the column
	uint8_t					blankLineAfter;				// BlankLineAfter properties of the column
	uint8_t					renameAllowed;				// rename allowed properties of the column
	Boolean					visible;					// set to true if the property kAddrColumnPropertyTabSkip is set to 1
	Boolean					hiddenOnRecordView;			// Set to true for the field to be displayed in the RecordView (used for names and yomi)
	uint8_t					addressGroupID;				// All fields of an address block have the same ID
	Boolean					notTransferable;			// If true, the column will not be added in the vCard
	char *					labelP;						// label properties of the column
	char *					renamedLabelP;				// renamed label properties of the column
} AddressTabColumnPropertiesType;

typedef	struct AddressTabsInfoTag
{
	AddressTabGraphicInfoType		tabInfo;			// Informationation on the Tab
	uint16_t						numElement;			// Number of filed to display in the tab
	Boolean							editableTextField;	// True is one of the fields in the tab is editable in table.
	uint8_t							padding;			// arm padding
	AddressTabColumnPropertiesType*	columnsPropertiesP; // array of properties sorted in the display order in the tab
} AddressTabsInfoType, *AddressTabsInfoPtr;

typedef struct AddressColsGroupInfoTag
{
	uint16_t	groupID;
	uint16_t	numColumns;
	uint32_t *	columnsListP;
} AddressColsGroupInfoType;

typedef struct AddressBookInfoTag
{
	uint16_t					currentTabId;			// tab Id of the active tab in the Book
	uint16_t					currentTabIndex;		// tab index of the current tab in the Book

	uint16_t					defaultViewTabId;		// Default tab to activate when displaying a record
	uint16_t					defaultNewTabId;		// Default tab to activate when creating a new record
	uint32_t					defaultNewColumnId;		// Default Column to select when creating a new record

	uint32_t					schemaID;				// ID of the database schema

	DbTableDefinitionType*		schemaP;				// Pointer on Schema definition as loaded from the DB
	DbSchemaColumnDefnType*		colListP;				// column list as loaded from the DB
	DbColumnPropertyValueType*	propListP;				// properties list as loaded from the DB

	uint32_t					numProp;				// number of properties

	uint16_t					numTabs;				// number of tabs
	uint16_t					numYomi;				// Number of active Yomi field = (Nb Element in yomiColIdArray)

	AddressTabsInfoPtr			tabs;					// tabs Array sorted by increasing Tab ID
	uint32_t*					yomiColIdArray;			// 2d array [ColId][YomiColId]

	uint16_t					padding;
	uint16_t					numAddressGroups;
	AddressColsGroupInfoType*	addressColsGroupInfoP;
} AddressBookInfoType, *AddressBookInfoPtr;

/*-------------------------------------------------------------------
	Globals
------------------------------------------------------------------- */

extern AddressBookInfoType		gBookInfo;

/*-------------------------------------------------------------------
	Public functions
------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

status_t						AddressTabInitSchemaInfoFromDB(AddressBookInfoType* bookInfoP, DmOpenRef dbP);
void							AddressTabReleaseSchemaInfoFromDB(AddressBookInfoType* bookInfoP, DmOpenRef dbP);

AddressTabColumnPropertiesType* AddressTabFindPropertiesByColId(AddressTabsInfoType* tabInfoP, uint16_t numTabs, uint32_t columnId);
uint32_t						AddressTabMatchColIds(uint32_t fieldMask, uint32_t ** colIDsP);
uint32_t						AddressTabFindColIdByFieldInfo(uint32_t fieldInfo);
uint32_t						AddressTabFindFamilyOfTabId(uint32_t tabId);
Boolean							AddressTabFindColIDTab(AddressBookInfoType* bookInfoP, uint32_t columnID, uint16_t *tabIndexP, uint16_t *tabIdP);
Boolean							AddressTabGetAddressGroupColumns(AddressBookInfoType* bookInfoP, uint8_t groupID, uint16_t *numColumnsP, uint32_t **columnsListPP);

#ifdef __cplusplus
}
#endif

/*	------------------------------------------------------------------- */

#endif
