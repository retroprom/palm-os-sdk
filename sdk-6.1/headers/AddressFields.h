/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressFields.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

/* --------------------
 *
 *	Field type definition
 *
 * 31                                     0
 * |iiii|iiii iiii iiii iiii|iiii iiii|iiii
 * |----|-------------------|---------|----
 * |    |                   |         |index (4 bits: sequential)
 * |    |                   |
 * |    |                   |Kind (8 bits)
 * |    |Type (16 bits)
 * |
 * |Family (4 bits)
 *
 * -------------------- */

#ifndef _ADDRESSFIELDS_H_
#define _ADDRESSFIELDS_H_

// Family: Mainly to manage the vCard Home&Work&Other parameter and the UI
#define kFieldFamilyMask				0xF0000000
#define kFieldFamily_Home				0x10000000
#define kFieldFamily_Corp				0x20000000
#define kFieldFamily_Other				0x40000000

// Type: Identify a field with a generic Type (7 free slots)
#define kFieldTypeMask					0x0FFFF000
#define kFieldType_Name					0x00001000
#define kFieldType_Address				0x00002000
#define kFieldType_Phone				0x00004000
#define kFieldType_Internet				0x00008000
#define kFieldType_Company				0x00010000
#define kFieldType_Date					0x00020000
#define kFieldType_Binary				0x00040000
#define kFieldType_InstantMessaging		0x00080000
#define kFieldType_Yomi					0x04000000	// Not exclusive
#define kFieldType_Extended				0x08000000	// Not exclusive

// Kind: Specify a field inside a Type
#define kFieldKindMask					0x00000FF0

// Kind for kFieldType_Name
#define kFieldKind_TitleName			0x00000010
#define kFieldKind_LastName				0x00000020
#define kFieldKind_MiddleName			0x00000040
#define kFieldKind_FirstName			0x00000080
#define kFieldKind_SuffixName			0x00000100
#define kFieldKind_NickName				0x00000200
#define kFieldKind_AssitantName			0x00000400
#define kFieldKind_EnglishName			0x00000800

// Kind for kFieldType_Address
#define kFieldKind_StreetAddress		0x00000010
#define kFieldKind_CityAddress			0x00000020
#define kFieldKind_StateAddress			0x00000040
#define kFieldKind_ZipCodeAddress		0x00000080
#define kFieldKind_CountryAddress		0x00000100

// Kind for kFieldType_Phone
#define kFieldKind_VoicePhone			0x00000010
#define kFieldKind_MobilePhone			0x00000020
#define kFieldKind_FaxPhone				0x00000040
#define kFieldKind_PagerPhone			0x00000080
#define kFieldKind_AssistantPhone		0x00000100

// Kind for kFieldType_Internet
#define kFieldKind_EmailInternet		0x00000010
#define kFieldKind_URLInternet			0x00000020

// Kind for kFieldType_InstantMessaging
#define kFieldKind_ICQ_IM				0x00000010
#define kFieldKind_AIM_IM				0x00000020
#define kFieldKind_Yahoo_IM				0x00000040
#define kFieldKind_MSN_IM				0x00000080
#define kFieldKind_Jabber_IM			0x00000100

// Kind for kFieldType_Company
#define kFieldKind_NameCompany			0x00000010
#define kFieldKind_TitleCompany			0x00000020
#define kFieldKind_ProfessionCompany	0x00000040

// Kind for kFieldType_Date
#define kFieldKind_BirthdayDate			0x00000010
#define kFieldKind_AnniversaryDate		0x00000020

// Kind for kFieldType_Binary
#define kFieldKind_PhotoBin				0x00000010
#define kFieldKind_LogoBin				0x00000020
#define kFieldKind_SoundBin				0x00000040
#define kFieldKind_KeyBin				0x00000080
#define kFieldKind_Note					0x00000800		// Very special

// Kind for kFieldType_Extended
#define kFieldKind_CustomExt			0x00000010

// Index: If a field is duplicate, identify them with an index (0-15)
#define kFieldIndexMask					0x0000000F

#endif // _ADDRESSFIELDS_H_
