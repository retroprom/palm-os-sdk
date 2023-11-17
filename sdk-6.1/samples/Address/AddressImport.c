/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressImport.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  Address Book vCard import tools
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <UIResources.h>
#include <ErrorMgr.h>
#include <TraceMgr.h>
#include <string.h>

#include "AddressImport.h"
#include "Address.h"

/***********************************************************************
 * Local definitions
 ***********************************************************************/

#define	kInitialPropertiesCount			32
#define	kIncrementPropertiesCount		32

typedef struct AddImportPropertyFieldTag
{
	char *		propFieldPtr;
	uint32_t	customSlotIndex;
}
AddImportPropertyFieldType;

	
typedef struct AddImportPropertyTag
{
	uint32_t					fieldType;
	uint32_t					familyOrKind;
	int32_t						propFieldsNumber;
	Boolean						preferredProperty;
	AddImportPropertyFieldType*	fieldsP;
}
AddImportPropertyType;


/***********************************************************************
 * Local static variables
 ***********************************************************************/

static MemHandle	sPropertiesArrayHdle = NULL;
static int32_t		sPropertiesCount = 0;
static int32_t		sAddPropertiesCursor = -1;
static int32_t		sGetPropertiesCursor = -1;

/***********************************************************************
 *
 * FUNCTION:    ImportPropertiesInit
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
void ImportPropertiesInit(void)
{
	void * propPtr;
	
	sPropertiesCount = kInitialPropertiesCount;
	sAddPropertiesCursor = -1;
	sGetPropertiesCursor = -1;
	sPropertiesArrayHdle = MemHandleNew(sizeof(AddImportPropertyType) * sPropertiesCount);
	propPtr = MemHandleLock(sPropertiesArrayHdle);
	memset(propPtr, 0, sizeof(sizeof(AddImportPropertyType) * sPropertiesCount));
	MemHandleUnlock(sPropertiesArrayHdle);
}

/***********************************************************************
 *
 * FUNCTION:    ImportAddNewProperty
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
void ImportAddNewProperty(uint32_t fieldType, uint32_t familyOrKind, uint32_t propFieldsNumber,	Boolean preferredProperty)
{
	status_t				err;
	char * 					propArrayP;
	AddImportPropertyType *	currentSlotP;

	ErrNonFatalDisplayIf(propFieldsNumber == 0, "wrong fields number: at least one");
	sAddPropertiesCursor++;
	
	// Check if there is enough slots
	if (sAddPropertiesCursor >= sPropertiesCount)
	{
		err = MemHandleResize(sPropertiesArrayHdle, sizeof(AddImportPropertyType) * (sPropertiesCount + kIncrementPropertiesCount));
		ErrNonFatalDisplayIf(err < errNone, "Unable to allocate memory.");

		propArrayP = MemHandleLock(sPropertiesArrayHdle);
		propArrayP += sizeof(AddImportPropertyType) * sPropertiesCount;
		memset(propArrayP, 0, sizeof(AddImportPropertyType) * kIncrementPropertiesCount);
		sPropertiesCount += kIncrementPropertiesCount;
		MemHandleUnlock(sPropertiesArrayHdle);
	}

	// Add new property now
	propArrayP = MemHandleLock(sPropertiesArrayHdle);
	currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sAddPropertiesCursor));
	currentSlotP->fieldType = fieldType;
	currentSlotP->familyOrKind = familyOrKind;
	currentSlotP->propFieldsNumber = (int32_t) propFieldsNumber;
	currentSlotP->preferredProperty = preferredProperty;
	currentSlotP->fieldsP = (AddImportPropertyFieldType *) MemPtrNew(propFieldsNumber * sizeof(AddImportPropertyFieldType));
	memset(currentSlotP->fieldsP, 0, propFieldsNumber * sizeof(AddImportPropertyFieldType));
	MemHandleUnlock(sPropertiesArrayHdle);
}


/***********************************************************************
 *
 * FUNCTION:    ImportAddAssignPropertyField
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
void ImportAddAssignPropertyField(uint32_t propFieldIndex, char * propFieldPtr, uint32_t customSlotIndex)
{
	char * 						propArrayP;
	AddImportPropertyType *		currentSlotP;
	AddImportPropertyFieldType*	currentFieldArrayP;
		
	ErrNonFatalDisplayIf((sAddPropertiesCursor >= sPropertiesCount) || (sAddPropertiesCursor < 0), "wrong cursor");

	propArrayP = MemHandleLock(sPropertiesArrayHdle);
	currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sAddPropertiesCursor));

	ErrNonFatalDisplayIf((int32_t)propFieldIndex >= currentSlotP->propFieldsNumber, "wrong field index");

	currentFieldArrayP = &(currentSlotP->fieldsP[propFieldIndex]);
	currentFieldArrayP->propFieldPtr = propFieldPtr;
	currentFieldArrayP->customSlotIndex = customSlotIndex;
	MemHandleUnlock(sPropertiesArrayHdle);
}

	
/***********************************************************************
 *
 * FUNCTION:    ImportGetFirstProperty
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
Boolean	ImportGetFirstProperty(uint32_t inFieldType, uint32_t * outFamilyP, uint32_t * outPropFieldsNumber)
{
	char * 					propArrayP;
	AddImportPropertyType *	currentSlotP;
	Boolean					foundProperty = false;

	sGetPropertiesCursor = 0;
	propArrayP = MemHandleLock(sPropertiesArrayHdle);

	do
	{
		currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sGetPropertiesCursor));

		if (currentSlotP->fieldType == inFieldType)
		{
			foundProperty = true;
			*outFamilyP = currentSlotP->familyOrKind;
			*outPropFieldsNumber = currentSlotP->propFieldsNumber;
			break;
		}
		
		sGetPropertiesCursor++;
	}
	while (sGetPropertiesCursor <= sAddPropertiesCursor);

	MemHandleUnlock(sPropertiesArrayHdle);

	return foundProperty;
}

/***********************************************************************
 *
 * FUNCTION:    ImportGetNextProperty
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
Boolean	ImportGetNextProperty(uint32_t inFieldType, uint32_t * outFamilyP, uint32_t * outPropFieldsNumber)
{
	char * 					propArrayP;
	AddImportPropertyType *	currentSlotP;
	Boolean					foundProperty = false;

	if (sGetPropertiesCursor >= sAddPropertiesCursor)
		return false;

	propArrayP = MemHandleLock(sPropertiesArrayHdle);
	sGetPropertiesCursor++;
	
	do
	{
		currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sGetPropertiesCursor));

		if (currentSlotP->fieldType == inFieldType)
		{
			foundProperty = true;
			*outFamilyP = currentSlotP->familyOrKind;
			*outPropFieldsNumber = currentSlotP->propFieldsNumber;
			break;
		}
		
		sGetPropertiesCursor++;
	}
	while (sGetPropertiesCursor <= sAddPropertiesCursor);

	MemHandleUnlock(sPropertiesArrayHdle);

	return foundProperty;
}


/***********************************************************************
 *
 * FUNCTION:    ImportIsCurrentPropertyPreferred
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    true if the current property has the preferred flag.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
Boolean	ImportIsCurrentPropertyPreferred(void)
{
	char * 					propArrayP;
	AddImportPropertyType *	currentSlotP;
	Boolean					preferredState;

	ErrNonFatalDisplayIf(sGetPropertiesCursor > sAddPropertiesCursor, "wrong field index");

	propArrayP = MemHandleLock(sPropertiesArrayHdle);
	currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sGetPropertiesCursor));
	preferredState = currentSlotP->preferredProperty;
	MemHandleUnlock(sPropertiesArrayHdle);

	return preferredState;
}


/***********************************************************************
 *
 * FUNCTION:    ImportGetPropertyField
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
char * ImportGetPropertyField(uint32_t propFieldIndex, int32_t * slotIndexP)
{
	char * 					propArrayP;
	AddImportPropertyType *	currentSlotP;
	char *					propFieldP;

	ErrNonFatalDisplayIf(sGetPropertiesCursor > sAddPropertiesCursor,"wrong cursor");

	propArrayP = MemHandleLock(sPropertiesArrayHdle);
	currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sGetPropertiesCursor));

	ErrNonFatalDisplayIf((int32_t)propFieldIndex >= currentSlotP->propFieldsNumber, "wrong field index");
	
	propFieldP = currentSlotP->fieldsP[propFieldIndex].propFieldPtr;

	if (slotIndexP)
		*slotIndexP = currentSlotP->fieldsP[propFieldIndex].customSlotIndex;
		
	MemHandleUnlock(sPropertiesArrayHdle);

	return propFieldP;
}

/***********************************************************************
 *
 * FUNCTION:    ImportUpdatePropertyField
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
void ImportUpdatePropertyField(uint32_t propFieldIndex, char * propFieldPtr, 
	uint32_t customSlotIndex)
{
	char * 						propArrayP;
	AddImportPropertyType *		currentSlotP;
	AddImportPropertyFieldType*	currentFieldArrayP;

	ErrNonFatalDisplayIf(sGetPropertiesCursor > sAddPropertiesCursor, "wrong cursor");

	propArrayP = MemHandleLock(sPropertiesArrayHdle);
	currentSlotP = (AddImportPropertyType *) (propArrayP + (sizeof(AddImportPropertyType) * sGetPropertiesCursor));

	ErrNonFatalDisplayIf((int32_t)propFieldIndex >= currentSlotP->propFieldsNumber, "wrong field index");

	currentFieldArrayP = &(currentSlotP->fieldsP[propFieldIndex]);
	currentFieldArrayP->propFieldPtr = propFieldPtr;
	currentFieldArrayP->customSlotIndex = customSlotIndex;
	
	MemHandleUnlock(sPropertiesArrayHdle);
}


/***********************************************************************
 *
 * FUNCTION:    ImportFreeAllPropertiesAndFields
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  None.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 * 	Name   	Date      	Description
 * 	----   	----      	-----------
 * 	PLe		05/22/03	Initial revision
 *
 ***********************************************************************/
void ImportFreeAllPropertiesAndFields(void)
{
	char * 						propArrayP;
	AddImportPropertyType *		currentSlotP;
	int32_t						i, j;
	AddImportPropertyFieldType*	currentFieldArrayP;

	propArrayP = MemHandleLock(sPropertiesArrayHdle);

	for (i = 0; i < sAddPropertiesCursor; i++)
	{
		currentSlotP = (AddImportPropertyType *)(propArrayP + (sizeof(AddImportPropertyType) * i));

		ErrNonFatalDisplayIf(currentSlotP->propFieldsNumber <= 0, "wrong fields number: at least one");

		for (j = 0; j < currentSlotP->propFieldsNumber; j++)
		{
			currentFieldArrayP = &(currentSlotP->fieldsP[j]);
			// Delete field property (allocated by the PDI library)
			if (currentFieldArrayP->propFieldPtr)
				MemPtrFree(currentFieldArrayP->propFieldPtr);
		}

		// Free the property content
		MemPtrFree(currentSlotP->fieldsP);
	}

	MemHandleUnlock(sPropertiesArrayHdle);
	MemHandleFree(sPropertiesArrayHdle);

	sPropertiesArrayHdle = NULL;
	sPropertiesCount = 0;
	sAddPropertiesCursor = -1;
	sGetPropertiesCursor = -1;
}
