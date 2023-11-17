/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressTransfer.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _ADDRESSTRANSFER_H_
#define _ADDRESSTRANSFER_H_

#include <PalmTypes.h>
#include <UDAMgr.h>
#include <AppMgr.h>
#include <PdiLib.h>

// constants for vCard Import / Export using 6.0 schema-based database
enum vCardNameFieldsIndexTag {
	iLastName ,
	iFirstName ,
	iMiddleName ,
	iTitleName ,
	iSuffixName ,
	nNameFields		// 5 vCard Name fields
} ;

enum vCardAddressFieldsIndexTag {
	iPOBoxAddress ,
	iXAddress ,
	iStreetAddress ,
	iCityAddress ,
	iStateAddress ,
	iZipCodeAddress ,
	iCountryAddress ,
	nAddressFields	// 7 vCard Address fields
} ;


enum vCardSenderKindTag {
	kPalmOS5AndBefore,
	kPalmOS6AndAfter,
	kNonPalmOSDevice
} ;

typedef Enum16 VCardSenderKindType;

#define nPhoneFields 			1
#define nEmailFields 			1
#define nCustomFields 			1
#define nURLFields 				1
#define nIMFields 				1

enum FamilyIndexTag
{
	kHomeFamilyIndex,
	kCorpFamilyIndex,
	kOtherFamilyIndex,
	nFamilyNumber
} ;

enum PhoneIndexTag
{
	kVoicePhoneIndex,
	kMobilePhoneIndex,
	kFaxPhoneIndex,
	kPagerPhoneIndex,
	kAssistantPhoneIndex,
	nPhoneNumber
} ;

enum InternetIndexTag
{
	kEmailInternetIndex,
	kURLInternetIndex,
	nInternetNumber
} ;

enum InstantMessagingIndexTag
{
	kICQ_IMIndex,
	kAIM_IMIndex,
	kYahoo_IMIndex,
	kMSN_IMIndex,
	kJabber_IMIndex,
	nInstantMessagingNumber
} ;

// Yomi (Japanese) specific
enum YomiFieldsIndexTag {
	kYomiLast,
	kYomiFirst,
	kYomiCompany,
	nYomiNames
} ;

static const uint8_t familyIndexTable[5] = { 0, kHomeFamilyIndex, kCorpFamilyIndex, 0, kOtherFamilyIndex};

#define familyIndexFromFieldInfo(w32)	familyIndexTable[((w32)& kFieldFamilyMask) >> 28]

static const uint8_t kindIndexTable[17] = { 0, 0, 1, 0, 2,	0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4};

#define phoneIndexFromFieldInfo(w32)	kindIndexTable[((w32)& kFieldKindMask) >> 4]
#define internetIndexFromFieldInfo(w32)	kindIndexTable[((w32)& kFieldKindMask) >> 4]
#define imIndexFromFieldInfo(w32)		kindIndexTable[((w32)& kFieldKindMask) >> 4]

static const uint32_t familyFieldsTable[3] = {kFieldFamily_Home, kFieldFamily_Corp, kFieldFamily_Other};

typedef struct TransferAddressStructTag
{
	Boolean		addressAssigned;
	uint32_t	family;
	char *		addressFields[nAddressFields];
} TransferAddressStructType;


/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void		TransferRegisterData(DmOpenRef appDbRef);
void		TransferSendRecord(DmOpenRef dbP, uint32_t recordID, const char * const prefix, uint16_t noDataAlertID, uint32_t transfertMask);
void		TransferSendCategory(DmOpenRef dbP, uint32_t numCategories, CategoryID *categoryiesP, const char *const prefix, uint16_t noDataAlertID);
status_t 	TransferSendPhoneCategory(DmOpenRef dbP);
status_t 	TransferGetPhoneCategory(DmOpenRef dbP, CategoryID *phoneCategoryP);
status_t 	TransferImportData(DmOpenRef dbP, uint32_t cursorID, ImportExportRecordParamsPtr impExpObjP);
status_t 	TransferExportData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);
status_t	TransferReceiveData(DmOpenRef dbP, uint32_t cursorID, ExgSocketPtr obxSocketP, Boolean acceptBefore, Boolean setGotoParams, char** ffnPP);
Boolean		TransferImportVCard(DmOpenRef dbP, uint32_t cursorID, PdiReaderType* reader, Boolean setGotoParams, uint32_t *rowIDP, status_t *errorP, char** ffnPP);
status_t	TransferExportVCard(DmOpenRef dbP, uint32_t recordID, PdiWriterType* writer, uint32_t familiesMask);
status_t	TransfertDeleteData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);
status_t	TransfertMoveData(DmOpenRef dbP, ImportExportRecordParamsPtr impExpObjP);
void		TransferPreview(ExgPreviewInfoType *infoP);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSTRANSFER_H_
