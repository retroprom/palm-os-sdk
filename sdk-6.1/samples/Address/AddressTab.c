/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressTab.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <MemoryMgr.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <TraceMgr.h>
#include <ErrorMgr.h>
#include <string.h>

#include "AddressTab.h"
#include "AddressRsc.h"
#include "AddressDBSchema.h"
#include "AddressTransfer.h"
#include "AddressAutoFill.h"
#include "AddressTools.h"

/***********************************************************************
 *
 *	Globals
 *
 ***********************************************************************/

AddressBookInfoType	gBookInfo;

extern int32_t gAddressSlotsAvailable[nFamilyNumber];
extern int32_t gPhoneSlotsAvailable[nPhoneNumber][nFamilyNumber];
extern int32_t gInternetSlotsAvailable[nFamilyNumber][nInternetNumber];
extern int32_t gIMSlotsAvailable[nInternetNumber][nFamilyNumber];
extern int32_t gCustomSlotsAvailable[nFamilyNumber];

/**********************************************************************
 *
 * Defines
 *
 **********************************************************************/
#define kAddressesGroupIncrement	5

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabInitvCardInfo
 *
 * DESCRIPTION:	Initialize an array used to import/export vCard.
 *
 * PARAMETERS:	fieldInfo:	Description of the column
 *
 * RETURNED:	Nothing.
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	03/17/03	Initial revision
 *
 ***********************************************************************/
static void PrvAddressTabInitvCardInfo(uint32_t fieldInfo)
{
	uint32_t	family;
	uint32_t	index;

	// Get the available slots in schema
	family = familyIndexFromFieldInfo(fieldInfo & kFieldFamilyMask);

	switch (fieldInfo & kFieldTypeMask)
	{
		case kFieldType_Address :
			// Count number of addresses information set by family
			if (fieldInfo & kFieldKind_StreetAddress)
				gAddressSlotsAvailable[family]++;
			break;

		case kFieldType_Extended | kFieldType_Phone :
		case kFieldType_Phone :
			index = phoneIndexFromFieldInfo(fieldInfo);
			gPhoneSlotsAvailable[index][family]++;
			break;

		case kFieldType_Internet :
			index = internetIndexFromFieldInfo(fieldInfo);
			gInternetSlotsAvailable[family][index]++;
			break;

		case kFieldType_Extended :
			if (fieldInfo & kFieldKind_CustomExt)
				gCustomSlotsAvailable[family]++;
			break;

		case kFieldType_Extended | kFieldType_InstantMessaging :
			index = imIndexFromFieldInfo(fieldInfo);
			gIMSlotsAvailable[index][family]++;
			break;
	}
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabInitYomi
 *
 * DESCRIPTION:	Initialize an array used to match Yomi and associated field.
 *
 * PARAMETERS:	bookInfoP:	Books information
 *
 * RETURNED:	errNone an error.
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	03/17/03	Initial revision
 *
 ***********************************************************************/
static status_t PrvAddressTabInitYomi(AddressBookInfoType* bookInfoP)
{
	uint32_t						size;
	uint32_t						i;
	uint32_t						matchNumYomi;
	uint32_t						*colYomiIDsP = NULL;
	uint32_t						mask;
	uint32_t						colId;
	status_t						err = errNone;
	AddressTabColumnPropertiesType	*colYomiProp = NULL;

	// Define a mask to find any field with the yomi bit field set.
	mask = kFieldFamilyMask | kFieldType_Yomi | kFieldKindMask;

	// If no Yomi field found, return.
	if ((matchNumYomi = AddressTabMatchColIds(mask, &colYomiIDsP)) == 0)
		return errNone;

	if (!bookInfoP->yomiColIdArray)
	{
		// We allocate to the maximum possible result, id 2 uint32_t if all Yomi are visible
		// Will be resize at the end of the parse to its real size or release if no Yomi are
		// visible or found.
		size = sizeof(uint32_t) * 2 * matchNumYomi;
		bookInfoP->yomiColIdArray = MemPtrNew(size);

		// If the allocation failed, return.
		if (!bookInfoP->yomiColIdArray)
		{
			err = memErrNotEnoughSpace;
			goto Exit;
		}

		memset(bookInfoP->yomiColIdArray, 0, size);
	}

	// For each yomi field found, try to found a standart field.
	for (i = 0; i < matchNumYomi; i++)
	{
		// Find the column properties for the Yomi column
		if ((colYomiProp = AddressTabFindPropertiesByColId(bookInfoP->tabs, bookInfoP->numTabs, colYomiIDsP[i])) != NULL)
		{
			// If the Yomi is an invisible field, skip it
			if (!colYomiProp->visible)
				continue;

			// Now we are going to find the column that belong to the Yomi column
			// Yomi and associated column have the same fieldInfo, with the kFieldType_Yomi set for the Yomi
			mask = colYomiProp->fieldInfo & ~kFieldType_Yomi;

			// Search column that match the computed mask before.
			// Should find only 1!! If not found, skip it.
			if ((colId = AddressTabFindColIdByFieldInfo(mask)) == kInvalidColumnID)
				continue;

			// Save the Yomi and associated column IDs.
			bookInfoP->yomiColIdArray[bookInfoP->numYomi * 2] = colId;
			bookInfoP->yomiColIdArray[bookInfoP->numYomi * 2 + 1] = colYomiIDsP[i];
			bookInfoP->numYomi++;
		}
	}

	// Resize the pointer to its real size. Previoulsy allocated for max space
	if (bookInfoP->numYomi)
		err = MemPtrResize(bookInfoP->yomiColIdArray, (uint32_t)(sizeof(uint32_t) * 2 * bookInfoP->numYomi));
	else
	{
		// Or release it if no Yomi field found in the schema.
		MemPtrFree(bookInfoP->yomiColIdArray);
		bookInfoP->yomiColIdArray = NULL;
	}

Exit:

	if (colYomiIDsP)
		MemPtrFree(colYomiIDsP);

	return err;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabFindTabInBook
 *
 * DESCRIPTION:	Search for a Tab in the Book on its ID.
 *
 * PARAMETERS:	bookInfoP:	Books information
 *				tabId:		Id of the tab to find in the book.
 *				foundIndex:	Index of the tab. If not found, it's contain
 *							the index where to create the new tab info
 *
 * RETURNED:	True if the Tab has been found in the book, false otherwise.
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	10/16/02	Initial revision
 *
 ***********************************************************************/
static Boolean PrvAddressTabFindTabInBook(AddressBookInfoType* bookInfoP, uint16_t tabId, uint16_t* foundIndex)
{
	uint16_t	i;

	// foundIndex initialized to the next empty slot (0 based).
	*foundIndex = bookInfoP->numTabs;

	// Now, searching in the already identified tab, it it's already exist or if it's a new one
	for (i = 0; i < bookInfoP->numTabs; i++)
	{
		if (bookInfoP->tabs[i].tabInfo.tabId == tabId)
		{
			*foundIndex = i;
			return true;
		}
	}

	TraceOutput(TL(appErrorClass, "Tab width ID %hu not found. Next free index is %hu", tabId, *foundIndex));

	return false;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabSaveTabColInfo
 *
 * DESCRIPTION:	Add the new Tab if not already found in the tab list.
 *				Save the column info associated to the tab.
 *
 * PARAMETERS:	bookInfoP:	Global to initialize with the db schema
 *				tabInfo:	Tab info to save if not already saved.
 *				colPropP:	Column property to save in the tab defined
 *							with the tabInfo parameter.
 *
 * RETURNED:	status_t - zero if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/16/02	Initial revision
 *
 ***********************************************************************/
static status_t PrvAddressTabSaveTabColInfo(AddressBookInfoType* bookInfoP,
											AddressTabGraphicInfoType* tabInfo,
											AddressTabColumnPropertiesType *colPropP,
											Boolean editableTextField,
											Boolean defaultViewTab,
											Boolean defaultNewField)
{
	uint16_t	tabIndex;
	uint32_t	size;

	// the var tabIndex is always initialized to the proper index if the tab already exist,
	// or to the new empty index for a new tab
	if (!PrvAddressTabFindTabInBook(bookInfoP, tabInfo->tabId, &tabIndex))
	{
		// New Tab founded. Create a new entry and create a pointer to save the columnId that belong to it.
		bookInfoP->tabs[bookInfoP->numTabs].tabInfo.tabId = tabInfo->tabId;

		// As we don't know how many column are assigned to a Tab, we allocate the max to avoid to resize
		// the pointer (= all columnn in one tab). The pointer will be resized after the scan complete.
		size = sizeof(AddressTabColumnPropertiesType) * bookInfoP->schemaP->numColumns;
		bookInfoP->tabs[bookInfoP->numTabs].columnsPropertiesP = MemPtrNew(size);

		// All column should be part of one tab. Allocate the max space for a tab.
		if (!bookInfoP->tabs[bookInfoP->numTabs].columnsPropertiesP)
			return memErrNotEnoughSpace;

		memset(bookInfoP->tabs[bookInfoP->numTabs].columnsPropertiesP, 0, size);

		bookInfoP->numTabs++;
	}

	// Save the tabName, IconIds, only if not null.
	if (tabInfo->tabNameP && *tabInfo->tabNameP)
		bookInfoP->tabs[tabIndex].tabInfo.tabNameP = tabInfo->tabNameP;

	if (tabInfo->tabIconId)
		bookInfoP->tabs[tabIndex].tabInfo.tabIconId = tabInfo->tabIconId;

	if (tabInfo->tabSelectedIconId)
		bookInfoP->tabs[tabIndex].tabInfo.tabSelectedIconId = tabInfo->tabSelectedIconId;

	// Add the new columnId in the Tab that belong to it.
	bookInfoP->tabs[tabIndex].columnsPropertiesP[bookInfoP->tabs[tabIndex].numElement] = *colPropP;
	bookInfoP->tabs[tabIndex].numElement++;

	// If only one column in the Tab is editable, the set this field to true.
	bookInfoP->tabs[tabIndex].editableTextField |= editableTextField;

	// If the default View Tab property has been defined, save it.
	// This property should be defined only once. If defined more than once, only display a message in debug
	// and keep the last. Default tab ID should be valid.
	if (defaultViewTab && tabInfo->tabId != kBookInvalidTabId)
	{
		DbgOnlyFatalErrorIf(bookInfoP->defaultViewTabId != kBookInvalidTabId, "Default View TabId should be defined once");
		bookInfoP->defaultViewTabId = tabInfo->tabId;
	}

	// As for the previous field, save the default Tab and field ID for the New form.
	// Should be defined only once. Default tab ID should be valid.
	if (defaultNewField && tabInfo->tabId != kBookInvalidTabId)
	{
		DbgOnlyFatalErrorIf(bookInfoP->defaultNewTabId != kBookInvalidTabId, "Default New FieldId should be defined once");
		bookInfoP->defaultNewTabId = tabInfo->tabId;
		bookInfoP->defaultNewColumnId = colPropP->columnId;
	}

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	AddressTabGetAddressGroupColumns
 *
 * DESCRIPTION:	Returns the list of columnIDs used by an address.
 *				The address columns group is identified by the
 *				passed groupID.
 *
 * PARAMETERS:	->	bookInfoP: Internal book structure
 *				->	groupID: The wanted groupID
 *				<-	numColumnsP: The number of columns in the group is returned here
 *				<-	columnsListPP: The list of columns is returned here. The caller
 *					doesn't have freed this buffer. It points directly to a gBookInfo
 *					parameter and will be freed and frmCloseEvent.
 *
 * RETURNED:	true if group found, false otherwise
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * TEs	10/17/2003	Initial revision
 *
 ***********************************************************************/
Boolean AddressTabGetAddressGroupColumns(AddressBookInfoType* bookInfoP, uint8_t groupID, uint16_t *numColumnsP, uint32_t **columnsListPP)
{
	uint16_t	groupIndex;

	for (groupIndex = 0; groupIndex < bookInfoP->numAddressGroups; groupIndex++)
	{
		if (bookInfoP->addressColsGroupInfoP[groupIndex].groupID == groupID)
		{
			*numColumnsP = bookInfoP->addressColsGroupInfoP[groupIndex].numColumns;
			*columnsListPP = bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP;
			return true;
		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabAddAddressGroup
 *
 * DESCRIPTION:	Add a column of type address to its correponding address
 *				columns group.
 *
 * PARAMETERS:	->	bookInfoP: Internal book struture
 *				->	columnID: Column ID to be added to the group
 *				->	groupID: The columns list groupID
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * TEs	10/17/2003	Initial revision
 *
 ***********************************************************************/
static void PrvAddressTabAddAddressGroup(AddressBookInfoType* bookInfoP, uint32_t columnID, uint8_t groupID)
{
	Boolean		found = false;
	uint32_t *	newColumnsListP;
	size_t		oldSize;
	size_t		newSize;
	uint16_t	groupIndex;
	uint16_t	colIndex;
	AddressColsGroupInfoType *	newGroupsP;

	groupIndex = 0;
	while (groupIndex < bookInfoP->numAddressGroups)
	{
		if (bookInfoP->addressColsGroupInfoP[groupIndex].groupID == groupID)
		{
			found = true;
			break;
		}

		groupIndex++;
	}

	// Create a new address group
	if (!found)
	{
		groupIndex = bookInfoP->numAddressGroups;
		bookInfoP->numAddressGroups++;

		if ((groupIndex % kAddressesGroupIncrement) == 0)
		{
			oldSize = groupIndex * sizeof(AddressColsGroupInfoType);
			newSize = oldSize + (kAddressesGroupIncrement * sizeof(AddressColsGroupInfoType));
			newGroupsP = MemPtrNew(newSize);
			ErrFatalDisplayIf(!newGroupsP, "Out of memory!");
			memset(newGroupsP, 0, newSize);

			if (bookInfoP->addressColsGroupInfoP)
			{
				memmove(newGroupsP, bookInfoP->addressColsGroupInfoP, oldSize);
				MemPtrFree(bookInfoP->addressColsGroupInfoP);
			}

			bookInfoP->addressColsGroupInfoP = newGroupsP;
		}

		bookInfoP->addressColsGroupInfoP[groupIndex].groupID = groupID;
		bookInfoP->addressColsGroupInfoP[groupIndex].numColumns = 0;
		bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP = NULL;
	}

	colIndex = bookInfoP->addressColsGroupInfoP[groupIndex].numColumns;
	bookInfoP->addressColsGroupInfoP[groupIndex].numColumns++;

	if ((colIndex % kAddressesGroupIncrement) == 0)
	{
		oldSize = colIndex * sizeof(uint32_t);
		newSize = oldSize + (kAddressesGroupIncrement * sizeof(uint32_t));
		newColumnsListP = MemPtrNew(newSize);
		ErrFatalDisplayIf(!newColumnsListP, "Out of memory!");
		memset(newColumnsListP, 0, newSize);

		if (bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP)
		{
			memmove(newColumnsListP, bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP, oldSize);
			MemPtrFree(bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP);
		}

		bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP = newColumnsListP;
	}

	bookInfoP->addressColsGroupInfoP[groupIndex].columnsListP[colIndex] = columnID;
}


/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabScanSchema
 *
 * DESCRIPTION:	Scan the Schema and properties to initialize the Tabs
 *				information.
 *
 * PARAMETERS:	bookInfoP:	Global to initialize with the db schema
 *
 * RETURNED:	status_t - zero if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/15/02	Initial revision
 *
 ***********************************************************************/
static status_t PrvAddressTabScanSchema(AddressBookInfoType* bookInfoP)
{
	uint16_t						i, j;
	status_t						err;
	Boolean							editableTextField;
	Boolean							defaultViewTab, defaultNewField;
	AddressTabColumnPropertiesType	colProp;
	AddressTabGraphicInfoType		tabInfo;

	memset(gAddressSlotsAvailable, 0, sizeof(gAddressSlotsAvailable)) ;
	memset(gPhoneSlotsAvailable, 0, sizeof(gPhoneSlotsAvailable)) ;
	memset(gInternetSlotsAvailable, 0, sizeof(gInternetSlotsAvailable)) ;
	memset(gIMSlotsAvailable, 0, sizeof(gIMSlotsAvailable)) ;
	memset(gCustomSlotsAvailable, 0, sizeof(gCustomSlotsAvailable)) ;


	// Order of algorithm is in bookInfoP->schemaP->numColumns * bookInfoP->numProp
	// For each column, we are going to search for their properties (only the one needed for UI).
	for (i = 0; i < bookInfoP->schemaP->numColumns; i++)
	{
		// Reset the properties var.
		memset(&tabInfo, 0, sizeof(AddressTabGraphicInfoType));
		memset(&colProp, 0, sizeof(AddressTabColumnPropertiesType));

		colProp.columnId = bookInfoP->colListP[i].id;
		colProp.columnDataType = bookInfoP->colListP[i].type;
		colProp.maxDataSize = bookInfoP->colListP[i].maxSize;
		colProp.displayIndex = 0xFFFF;
		// by default, a column is visible, unless the kAddrColumnPropertyTabSkip property is set to 1
		colProp.visible = true;

		tabInfo.tabId = kBookInvalidTabId;
		editableTextField = true;
		defaultViewTab = false;
		defaultNewField = false;

		switch( colProp.columnDataType )
		{
			case dbBoolean:
			case dbDateTime:
			case dbDate:
			case dbTime:
			case dbBlob:
				editableTextField = false;
				break;
		}

		for (j = 0; j < bookInfoP->numProp; j++)
		{
			if (bookInfoP->colListP[i].id == bookInfoP->propListP[j].columnID)
			{
				switch(bookInfoP->propListP[j].propertyID)
				{
					case kAddrColumnPropertyTabId:
						tabInfo.tabId = *(uint16_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyTabName:
						tabInfo.tabNameP = (char*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyTabIconResId:
						tabInfo.tabIconId= *(uint16_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyTabSelectedIconResId:
						tabInfo.tabSelectedIconId = *(uint16_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyTabSkip:
						// This column should not be part of a tab. Skip it.
						colProp.visible = !*(Boolean*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyTabDefaultView:
						defaultViewTab = *(Boolean*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyName:
						colProp.labelP = (char*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyRenameAllowed:
						colProp.renameAllowed = *(uint8_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyNewName:
						colProp.renamedLabelP = (char*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyNoLabel:
						colProp.noLabel = *(uint8_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyBlankLineAfter:
						colProp.blankLineAfter = *(uint8_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyDisplayIndex:
						colProp.displayIndex = *(uint16_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyEditFirstField:
						defaultNewField = *(Boolean*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyHiddenOnRecordView:
						colProp.hiddenOnRecordView = *(Boolean*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyAddressGroupID:
						colProp.addressGroupID = *(uint8_t*)bookInfoP->propListP[j].data;
						PrvAddressTabAddAddressGroup(bookInfoP, colProp.columnId, colProp.addressGroupID);
						break;

					case kAddrColumnPropertyFieldInfo:
						colProp.fieldInfo = *(uint32_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyAutoFillDbType:
						colProp.autoFillDbType = *(uint32_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyAutoFillInitStr:
						colProp.autoFillDbInitStr = *(uint16_t*)bookInfoP->propListP[j].data;
						break;

					case kAddrColumnPropertyNotTransferable:
						colProp.notTransferable = *(Boolean*)bookInfoP->propListP[j].data;
						break;

					default:
						// This is a property we don't care about.
						continue;
				} // switch
			} // if
		} // for j


		// Post processing....
		// Once the column has been parsed, applied various function on it.

		// A column could be assigned to a valid Tab, but has the skip/visible property set
		// In this case, we change the column from it's assigned tab ID to kBookInvalidTabId
		if(!colProp.visible)
			tabInfo.tabId = kBookInvalidTabId;

		// Initialized th AutoFillDB for the current column
		if (colProp.autoFillDbType)
			AutoFillInitDB(colProp.autoFillDbType, colProp.autoFillDbInitStr);

		// Save the info for this column. All properties has been checked for it.
		// tabId = kBookInvalidTabId: This Tab will not be part of the UI.
		// This "Tab" contains the DisplayPhone or the Yomi (for non JaJp Rom) column for example.
		if ((err = PrvAddressTabSaveTabColInfo(bookInfoP, &tabInfo, &colProp, editableTextField, defaultViewTab, defaultNewField)) < errNone)
			return err;

		// Set the proper Column ID vCard field mapping
		PrvAddressTabInitvCardInfo(colProp.fieldInfo);

	} // for i

	// Once the schema has been parsed, initialize the Yomi counter information and array.
	if ((err = PrvAddressTabInitYomi(bookInfoP)) < errNone)
		return err;

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabColumnIdCompare
 *
 * DESCRIPTION:	SysQSort callback.
 *				This will routine will sort the fields inside a Tabs
 *				on their display index.
 *				As we don't know in which order they have been defined/loaded,
 *				we need to sort them. This will defined the order of diplay
 *				for the fields for a Tab.
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/16/02	Initial revision
 *
 ***********************************************************************/
static int16_t PrvAddressTabColumnIdCompare(void *elem1, void *elem2, int32_t other)
{
	// Sort the column by their display index property, in ascending order.
	// The display index of the column determine their order of appearence in the Tab

	//return (((AddressTabColumnPropertiesType*)elem1)->displayIndex > ((AddressTabColumnPropertiesType*)elem2)->displayIndex) ? 1 : -1;

	return ((AddressTabColumnPropertiesType*)elem1)->displayIndex - ((AddressTabColumnPropertiesType*)elem2)->displayIndex;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabIdCompare
 *
 * DESCRIPTION:	SysQSort callback.
 *				This will routine will sort the Tabs on their ID.
 *				They are displayed on their ID value from the smallest
 *				to the greater one. As we don't know in which order they
 *				have been defined/loaded, we need to sort them.
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/16/02	Initial revision
 *
 ***********************************************************************/
static int16_t PrvAddressTabIdCompare (void *elem1, void *elem2, int32_t other)
{
	// Sort the tab on their ID, in ascending order
	// The Id of the Tab determine their display order.

	return (((AddressTabsInfoType*)elem1)->tabInfo.tabId> ((AddressTabsInfoType*)elem2)->tabInfo.tabId) ? 1 : -1;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabSortAndResizeBook
 *
 * DESCRIPTION:	Resize all pointer to their real size now we know the
 *				number of tabs and element in tabs.
 *				Also sort the buffer on tabId and display index fields.
 *
 * PARAMETERS:	bookInfoP:	Global to initialize with the db schema
 *
 * RETURNED:	status_t - zero if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/16/02	Initial revision
 *
 ***********************************************************************/
static status_t PrvAddressTabSortAndResizeBook(AddressBookInfoType* bookInfoP)
{
	status_t	err;
	uint16_t	i;

	// Schema and properties has been parsed.
	if (bookInfoP->numTabs)
	{
		// Resize the pointer to its real size. Previoulsy allocated for max space
		if ((err = MemPtrResize(bookInfoP->tabs, (uint32_t)(sizeof(AddressTabsInfoType) * bookInfoP->numTabs))) != errNone)
			return err;

		// Each tab have a pointer that contains columnId that belong to it.
		// Resize this pointer to its real size. Previoulsy allocated for max space
		for (i = 0; i < bookInfoP->numTabs; i++)
		{
			if ((err = MemPtrResize(bookInfoP->tabs[i].columnsPropertiesP, (uint32_t)(sizeof(AddressTabColumnPropertiesType) * bookInfoP->tabs[i].numElement))) != errNone)
				return err;

			// Sort the columnId in ascending order => Order of display in tab
			if (bookInfoP->tabs[i].numElement > 1)
				SysQSort (bookInfoP->tabs[i].columnsPropertiesP, bookInfoP->tabs[i].numElement, sizeof(AddressTabColumnPropertiesType), PrvAddressTabColumnIdCompare, 0);
		}

		// Sort the tab in ascending order. Order of display in the book.
		if (bookInfoP->numTabs > 1)
			SysQSort (bookInfoP->tabs, bookInfoP->numTabs, sizeof(AddressTabsInfoType), PrvAddressTabIdCompare, 0);
	}

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:	PrvAddressTabCheckDefaultValues
 *
 * DESCRIPTION:	Check if the values for the default Tabd Id & Column ID
 *				used in Edit & view form have been initialized by
 *				properties found in the schema. If not, initilize them
 *				to default values.
 *
 * PARAMETERS:	bookInfoP:	Global to initialize with the db schema
 *
 * RETURNED:	Nothing.
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	03/27/03	Initial revision
 *
 ***********************************************************************/
static void PrvAddressTabCheckDefaultValues(AddressBookInfoType* bookInfoP)
{
	uint16_t	i, j;

	// Schema and properties has been parsed.
	if (bookInfoP->numTabs)
	{
		// If the default Tab has not been defined for the View form, initialize it to "All" Tab
		if (bookInfoP->defaultViewTabId == kBookInvalidTabId)
			bookInfoP->defaultViewTabId = kTabAllId;

		// If the default New Tab & ColumnID have not been defined, set it to the first visible (and non Yomi)
		if (bookInfoP->defaultNewTabId == kBookInvalidTabId)
		{
			for (i = 0; i < bookInfoP->numTabs; i++)
			{
				for (j = 0; j < bookInfoP->tabs[i].numElement; j++)
				{
					if (bookInfoP->tabs[i].columnsPropertiesP[j].visible && !ToolsIsYomiFieldByIndex(i, j))
					{
						bookInfoP->defaultNewTabId = bookInfoP->tabs[i].tabInfo.tabId;
						bookInfoP->defaultNewColumnId = bookInfoP->tabs[i].columnsPropertiesP[j].columnId;
						return;
					}
				}
			}
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:	AddressTabInitSchemaInfoFromDB
 *
 * DESCRIPTION:	Load and keep in memory the schema database and properties.
 *				Initialize the information needed for the Tab.
 *
 * PARAMETERS:	bookInfoP:	Global to initialize with the db schema
 *				dbP:		Address Book database ref pointer
 *
 * RETURNED:	status_t - zero if no error, else the error
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/15/02	Initial revision
 *
 ***********************************************************************/
status_t AddressTabInitSchemaInfoFromDB(AddressBookInfoType* bookInfoP, DmOpenRef dbP)
{
	status_t	err;

	// If the bookinfo doesn't exist, return, as we will not be able to present the database.
	if (!bookInfoP)
		return dmErrInvalidParam;

	// Cleanup the struct.
	memset(bookInfoP, 0, sizeof(AddressBookInfoType));

	// Get the default schema from the DB and keep it in memory.
	if ((err = DbGetTableSchema(dbP, kAddressDefaultTableName, &bookInfoP->schemaP)) != errNone)
		goto Error;

	bookInfoP->colListP = bookInfoP->schemaP->columnListP;
	//bookInfoP->schemaID = bookInfoP->schemaP->id; ####TEs: New DBMgr

	// Load all the column properties from the db and keep it in memory.
	if ((err = DbGetAllColumnPropertyValues(dbP, kAddressDefaultTableName, true, &bookInfoP->numProp, &bookInfoP->propListP)) != errNone)
		goto Error;

	bookInfoP->numTabs = 0;

	// We don't know at this point how many Tabs have been set in the schema.
	// The max Tab allowed is potentialy 1 column by Tab. So we allocate the max possible Tabs
	// and we will resize it after the load.
	if ((bookInfoP->tabs = MemPtrNew(sizeof(AddressTabsInfoType) * bookInfoP->schemaP->numColumns)) == NULL)
	{
		err = memErrNotEnoughSpace;
		goto Error;
	}

	// Cleanup the struct.
	memset(bookInfoP->tabs, 0, sizeof(AddressTabsInfoType) * bookInfoP->schemaP->numColumns);

	// Initialize the default Tab & Column ID for New & View form to Invalid values.
	bookInfoP->defaultViewTabId		= kBookInvalidTabId;
	bookInfoP->defaultNewTabId		= kBookInvalidTabId;
	bookInfoP->defaultNewColumnId	= kInvalidColumnID;

	// Scan the schema and properties to initialize the Tabs info structure.
	if ((err = PrvAddressTabScanSchema(bookInfoP)) != errNone)
		goto Error;

	// Resize the pointer and sort the tab & col info.
	if ((err = PrvAddressTabSortAndResizeBook(bookInfoP)) != errNone)
		goto Error;

	// After the scan & sort, verify & initilize the default values
	PrvAddressTabCheckDefaultValues(bookInfoP);

	return errNone;

Error:

	AddressTabReleaseSchemaInfoFromDB(bookInfoP, dbP);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:	AddressTabReleaseSchemaInfoFromDB
 *
 * DESCRIPTION:	Release all information loaded for managing Tabs & Schema.
 *
 * PARAMETERS:	bookInfoP:	Global to initialize with the db schema
 *
 * RETURNED:	None
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/15/02	Initial revision
 *
 ***********************************************************************/
void AddressTabReleaseSchemaInfoFromDB(AddressBookInfoType* bookInfoP, DmOpenRef dbP)
{
	uint16_t	i;

	if (!bookInfoP)
		return;

	if (bookInfoP->tabs)
	{
		for (i = 0; i < bookInfoP->numTabs; i++)
		{
			if (bookInfoP->tabs[i].columnsPropertiesP)
				MemPtrFree(bookInfoP->tabs[i].columnsPropertiesP);
		}

		MemPtrFree(bookInfoP->tabs);
	}

	if (bookInfoP->schemaP)
		DbReleaseStorage(dbP, bookInfoP->schemaP);

	if (bookInfoP->propListP)
		DbReleaseStorage(dbP, bookInfoP->propListP);

	if (bookInfoP->yomiColIdArray)
		MemPtrFree(bookInfoP->yomiColIdArray);

	if (bookInfoP->addressColsGroupInfoP)
	{
		while (bookInfoP->numAddressGroups--)
		{
			if (bookInfoP->addressColsGroupInfoP[bookInfoP->numAddressGroups].columnsListP)
				MemPtrFree(bookInfoP->addressColsGroupInfoP[bookInfoP->numAddressGroups].columnsListP);
		}

		MemPtrFree(bookInfoP->addressColsGroupInfoP);
	}

	memset(bookInfoP, 0, sizeof(AddressBookInfoType));
}

/***********************************************************************
 *
 * FUNCTION:	AddressTabFindColIDTab
 *
 * DESCRIPTION:	Returns the tab index & id of the passed columnID
 *
 * PARAMETERS:	->	bookInfoP:	Global to initialize with the db schema
 *				->	columnID:	columnID to search
 *				<-	tabIndexP:	the corresponding tab index is returned here
 *				<-	tabIdP:		the corresponding tab id is returned here
 *
 * RETURNED:	True if the tab info have been found. False otherwise
 *
 * REVISION HISTORY:
 *
 *	Name	Date		Description
 *	----	--------	-----------
 *	TEs		04/25/03	Initial revision
 *
 ***********************************************************************/
Boolean AddressTabFindColIDTab(AddressBookInfoType* bookInfoP, uint32_t columnID, uint16_t *tabIndexP, uint16_t *tabIdP)
{
	uint16_t	tabIndex;
	uint16_t	fieldIndex;

	for (tabIndex = 0; tabIndex < bookInfoP->numTabs; tabIndex++)
	{
		// skip the "All" Tab
		if (bookInfoP->tabs[tabIndex].tabInfo.tabId == kTabAllId)
			continue;

		for (fieldIndex = 0; fieldIndex < bookInfoP->tabs[tabIndex].numElement; fieldIndex++)
		{
			if (bookInfoP->tabs[tabIndex].columnsPropertiesP[fieldIndex].columnId == columnID)
			{
				if (tabIndexP)
					*tabIndexP = tabIndex;
				if (tabIdP)
					*tabIdP = bookInfoP->tabs[tabIndex].tabInfo.tabId;
				return true;
			}
		}
	}

	if (tabIndexP)
		*tabIndexP = kBookInvalidTabIndex;
	if (tabIdP)
		*tabIdP = kBookInvalidTabId;

	return false;
}

/***********************************************************************
 *
 * FUNCTION:	AddressTabFindPropertiesByColId
 *
 * DESCRIPTION:	Return all properties for a Column.
 *
 * PARAMETERS:	tabInfoP:	Array that contains all properties for all columns
 *				numTabs:	number of tabs in the array
 *				columnId:	Id of the column to find
 *
 * RETURNED:	struct that contains all the properties for the column or NULL
 *				if not found
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 *	LFe	10/15/02	Initial revision
 *
 ***********************************************************************/
AddressTabColumnPropertiesType* AddressTabFindPropertiesByColId(AddressTabsInfoType* tabInfoP, uint16_t numTabs, uint32_t columnId)
{
	uint16_t	tabIndex;
	uint16_t	propIndex;

	for (tabIndex = 0; tabIndex < numTabs; tabIndex++)
	{
		for (propIndex = 0; propIndex < tabInfoP[tabIndex].numElement; propIndex++)
		{
			if (tabInfoP[tabIndex].columnsPropertiesP[propIndex].columnId == columnId)
				return &tabInfoP[tabIndex].columnsPropertiesP[propIndex];
		}
	}

	return NULL;
}


/***********************************************************************
 *
 * FUNCTION:	AddressTabMatchColIds
 *
 * DESCRIPTION:	Search for columns matching a field property bitmask
 *
 * PARAMETERS:	fieldMask:	properties to match
 *				colIDsP:	columns
 *
 * RETURNED:	Number of columns matching the properties
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LYr	12/17/02	Initial revision
 *
 ***********************************************************************/
uint32_t AddressTabMatchColIds(uint32_t fieldMask, uint32_t ** colIDsP)
{
	uint32_t i, j, n;

	n = 0 ;
	// Allocate the buffer to the max number of match = number of columns.
	*colIDsP = (uint32_t *) MemPtrNew(gBookInfo.schemaP->numColumns * sizeof(uint32_t)) ;

	for (i = 0; i < gBookInfo.numTabs; i++)
	{
		for (j = 0; j < gBookInfo.tabs[i].numElement; j++)
		{
			// match column family
			if ( ((gBookInfo.tabs[i].columnsPropertiesP[j].fieldInfo & kFieldFamilyMask) & fieldMask)
			// match column type
			&&	((gBookInfo.tabs[i].columnsPropertiesP[j].fieldInfo & kFieldTypeMask) & fieldMask)
			// match column kind
			&&	((gBookInfo.tabs[i].columnsPropertiesP[j].fieldInfo & kFieldKindMask) & fieldMask)
			// exact the index is correct
			&&	(!(fieldMask & kFieldIndexMask)
				||	((gBookInfo.tabs[i].columnsPropertiesP[j].fieldInfo & kFieldIndexMask) == (fieldMask & kFieldIndexMask))) )
			(*colIDsP)[n++] = gBookInfo.tabs[i].columnsPropertiesP[j].columnId ;
		}
	}

	if (n)
		MemPtrResize(*colIDsP, (uint32_t)(n * sizeof(uint32_t))) ;
	else
	{
		MemPtrFree(*colIDsP) ;
		*colIDsP = NULL ;
	}

	return n;
}

/***********************************************************************
 *
 * FUNCTION:	AddressTabFindColIdByFieldInfo
 *
 * DESCRIPTION:	Search the 'unique' colId that match the 'unique' fieldInfo
 *
 * PARAMETERS:	fieldInfo:	FieldInfo to find. Should be unique in the db.
 *
 * RETURNED:	The ColId that contains the fieldInfo in its properties.
 *				kInvalidColumnID if nothing found.
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	03/19/02	Initial revision
 *
 ***********************************************************************/
uint32_t AddressTabFindColIdByFieldInfo(uint32_t fieldInfo)
{
#if BUILD_TYPE != BUILD_TYPE_RELEASE
	uint32_t	i, j, foundColId = kInvalidColumnID, n = 0;

	for (i = 0; i < gBookInfo.numTabs; i++)
	{
		for (j = 0; j < gBookInfo.tabs[i].numElement; j++)
		{
			if (gBookInfo.tabs[i].columnsPropertiesP[j].fieldInfo == fieldInfo)
			{
				foundColId = gBookInfo.tabs[i].columnsPropertiesP[j].columnId;
				n++;
			}
		}
	}

	ErrNonFatalDisplayIf(n > 1, "There should be only one colId for a fieldInfo");

	return foundColId;

#else
	uint32_t	i, j;

	for (i = 0; i < gBookInfo.numTabs; i++)
	{
		for (j = 0; j < gBookInfo.tabs[i].numElement; j++)
		{
			if (gBookInfo.tabs[i].columnsPropertiesP[j].fieldInfo == fieldInfo)
				return gBookInfo.tabs[i].columnsPropertiesP[j].columnId;
		}
	}

	return kInvalidColumnID;

#endif
}

/***********************************************************************
 *
 * FUNCTION:	AddressTabFindFamilyFromTabId
 *
 * DESCRIPTION:	Return the record family of a Tab
 *
 * PARAMETERS:	tabId:	Tab unique id
 *
 * RETURNED:	The family value (kFieldFamily_Home, ...).
 *				O - if nothing found
 *
 * REVISION HISTORY:
 *
 * Name	  Date		Description
 * ----	--------	-----------
 * LFe	02/17/02	Initial revision
 *
 ***********************************************************************/
uint32_t AddressTabFindFamilyOfTabId(uint32_t tabId)
{
	uint32_t	tabIndex;
	uint32_t	i;

	switch(tabId)
	{
		case kBookInvalidTabId:	return 0;
		case kTabAllId:			return kFieldFamilyMask;
		default:
			for (tabIndex = 0; tabIndex < gBookInfo.numTabs; tabIndex++)
			{
				if (gBookInfo.tabs[tabIndex].tabInfo.tabId == tabId)
				{
					for (i = 0; i < gBookInfo.tabs[tabIndex].numElement; i++)
					{
						if (gBookInfo.tabs[tabIndex].columnsPropertiesP[i].fieldInfo)
							return gBookInfo.tabs[tabIndex].columnsPropertiesP[i].fieldInfo & kFieldFamilyMask;
					}
				}
			}
			break;
	}

	return 0;
}
