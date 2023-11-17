/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressImport.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSIMPORT_H_
#define _ADDRESSIMPORT_H_

#include <PalmTypes.h>

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------------------
// Initializations
// --------------------------------------------------------------------------------------

void 		ImportPropertiesInit(void);

// --------------------------------------------------------------------------------------
// Create and assign properties
// Supported field types and corresponding familyOrKind
//
//		kFieldType_Name		<=>	kFieldKind_TitleName / kFieldKind_LastName / kFieldKind_MiddleName...
//		kFieldType_Address	<=>	kFieldFamily_Home / kFieldFamily_Corp / kFieldFamily_Other
//		kFieldType_Phone	<=>	kFieldFamily_Home / kFieldFamily_Corp / kFieldFamily_Other
//		kFieldType_Internet	<=>	kFieldFamily_Home / kFieldFamily_Corp / kFieldFamily_Other
//		kFieldType_Extended	<=>	kFieldKind_CustomExt
//
//  These routines handle an "Add" cursor to keep track of the current position to add field
//	properties.
//
// --------------------------------------------------------------------------------------

void 		ImportAddNewProperty(uint32_t fieldType, uint32_t familyOrKind, uint32_t propFieldsNumber, Boolean preferredProperty);
void 		ImportAddAssignPropertyField(uint32_t propFieldIndex, char * propFieldPtr, uint32_t customSlotIndex);

// --------------------------------------------------------------------------------------
// Parse and Read properties
//
//  These routines handle a "Get" cursor to keep track of the current position to read
// 	or update the field properties.
//
// --------------------------------------------------------------------------------------
Boolean		ImportGetFirstProperty(uint32_t inFieldType, uint32_t * outFamilyOrKindP, uint32_t * outPropFieldsNumber);
Boolean		ImportGetNextProperty(uint32_t inFieldType, uint32_t * outFamilyOrKindP, uint32_t * outPropFieldsNumber);
Boolean		ImportIsCurrentPropertyPreferred(void);

// Get the property fields (0 <= propFieldIndex < propFieldsNumber)
char *		ImportGetPropertyField(uint32_t propFieldIndex, int32_t * slotIndexP);

// Update the property fields (0 <= propFieldIndex < propFieldsNumber)
void 		ImportUpdatePropertyField(uint32_t propFieldIndex, char * propFieldPtr, uint32_t customSlotIndex);

// --------------------------------------------------------------------------------------
// Free properties and content pointed by fields
// --------------------------------------------------------------------------------------

void		ImportFreeAllPropertiesAndFields(void);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSIMPORT_H_

