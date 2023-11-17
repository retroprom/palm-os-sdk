/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressRsc.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef ADDRESSRSC_H
#define ADDRESSRSC_H

// List View
#define ListView								1000
#define ListCategoryTrigger						1003
#define ListCategoryList						1004
#define ListTable								1005
#define ListLookUpLabel							1006
#define ListLookupField							1007
#define ListNewButton							1008
#define ListUpButton							1009
#define ListDownButton							1010
#define ListDoneButton							1011
#define ListAttachButton						1012

// Details Dialog Box
#define DetailsDialog							1200
#define DetailsCategoryTrigger					1204
#define DetailsCategoryList						1205
#define DetailsSecretCheckbox					1207
#define DetailsOkButton							1208
#define DetailsCancelButton						1209
#define DetailsDeleteButton						1210
#define DetailsNoteButton						1211
#define DetailsPhoneList						1213
#define DetailsPhoneTrigger						1214

// Options Dialog
#define OptionsDialog							1400
#define OptionsSortByPriority					1403
#define OptionsSortByDueDate					1404
#define OptionsShowCompleted					1405
#define OptionsShowDueItems						1407
#define OptionsShowDueDates						1409
#define OptionsShowPriorities					1411
#define OptionsOkButton							1413
#define OptionsCancelButton						1414

// Detele Completed Dialog
#define DeleteCompletedDialog					1500
#define DeleteCompletedSaveBackup				1504
#define DeleteCompletedOk						1506
#define DeleteCompletedCancel					1507

// Delete Addr Dialog
#define DeleteAddrDialog						1600
#define DeleteAddrSaveBackup					1604
#define DeleteAddrOk							1606
#define DeleteAddrCancel						1607

// Address Record View
#define RecordView								1700
#define RecordCategoryLabel						1702
#define RecordDoneButton						1704
#define RecordEditButton						1705
#define RecordNewButton							1706
#define RecordUpButton							1707
#define RecordDownButton						1708
#define RecordViewDisplay						1709
#define RecordAttachButton						1711
#define RecordBook								1712
#define RecordViewName							1713
#define RecordBookBody							1714
#define RecordBackgroundGadget					1715


// Edit Address View
#define EditView								1800
#define EditCategoryTrigger						1803
#define EditCategoryList						1804
#define EditTable								1805
#define EditDoneButton							1806
#define EditDetailsButton						1807
#define EditUpButton							1808
#define EditDownButton							1809
#define EditNoteButton							1812
#define EditBackgroundGadget					1813
#define EditViewEditBook						1814
#define EditViewName							1815
#define EditViewEditBookBody					1816

// BusinessCard bitmap
#define AddressBusinessCardBmp					1000

#define DeleteAlternateToTextDataIcon			1700
#define DeleteAlternateToTextDataInvertedIcon	1710

// Custom Edit
#define CustomEditDialog						1900
#define CustomEditTable							1903
#define CustomEditScrollbar						1904
#define CustomEditOkButton						1907
#define CustomEditCancelButton					1908

// Preferences
#define PreferencesDialog						2000
#define PreferencesRememberCategoryCheckbox		2001
#define PreferencesEnableTapDialingCheckbox		2002
#define PreferencesOrderByTrigger				2003
#define PreferencesOrderByList					2004
#define PreferencesOkButton						2005
#define PreferencesCancelButton					2006
#define PreferencesListByLabel					2008
#define PreferencesExportRecordLabel			2009
#define PreferencesExportRecordList				2010
#define PreferencesExportRecordTrigger			2011

// Lookup View
#define LookupView								2100
#define LookupTitle								2101
#define LookupTable								2102
#define LookupLookupLabel						2103
#define LookupLookupField						2104
#define LookupPasteButton						2105
#define LookupCancelButton						2106
#define LookupUpButton							2107
#define LookupDownButton						2108

// Addr Dial list
#define DialListDialog                    		2300
#define DialListDialButton                 		2301
#define DialListCancelButton               		2302
#define DialListNumberField                 	2305
#define DialListDescriptionGadget          		2304
#define DialListNumberToDialLabel           	2306
#define DialListList                        	2303

// ExportAsk Dialog
#define ExportAskDialog							2500
#define ExportAskOkButton						2501
#define ExportAskCancelButton					2502
#define ExportAskLabel							2503
#define ExportAskList							2504
#define ExportAskPopTrigger						2505
#define ExportAskAddNoteCheckbox				2506
#define ExportAskHelpString						2500


// Delete Note Alert
#define DeleteNoteAlert							2001
#define DeleteNoteYes							0
#define DeleteNoteNo           			  		1

#define NameNeededAlert							2003

#define EmptyRecordAlert						2008

// Delete Alternate to text Data
#define DeleteAlternateToTextDataAlert			4000
#define DeleteAlternateToTextDataYes			0
#define DeleteAlternateToTextDataNo           	1

// Select Business Card Alert
#define SelectBusinessCardAlert					2004
#define SelectBusinessCardYes					0
#define SelectBusinessCardNo        			1

// Send Business Card Alert
#define SendBusinessCardAlert					2005

// Send Business Card Alert
#define AttachNoRecordSelectedAlert				2006

// Phone import alert
#define PhoneIOImportAlert						2007

// Phone import dialog
#define PhoneIOImportPhoneDirectoryDialog		2400

// Phone export dialog
#define PhoneIOExportPhoneDirectoryDialog		2401

// Import/Export controls (use the same IDs)
#define PhoneIOPhoneDirectoryTrigger			2404
#define PhoneIOPhoneDirectoryList				2405
#define PhoneIOCategoryTrigger					2407
#define PhoneIOCategoryList						2408
#define PhoneIOPhoneCatOkButton					2409
#define PhoneIOPhoneCatCancelButton				2410

// Menus
#define ListViewMenuBar							1000
#define RecordViewMenuBar						1100
#define EditViewMenuBar							1200

// Menu commands
#define ListRecordDeleteRecordCmd				100
#define ListRecordDuplicateAddressCmd			101
#define ListRecordDialCmd						102
#define ListRecordSeparator1					103
#define ListRecordImportPhoneCatCmd				104
#define ListRecordExportPhoneCatCmd				105
#define ListRecordSeparator2					106
#define ListRecordBeamCategoryCmd				107
#define ListRecordSendCategoryCmd				108
#define ListRecordBeamBusinessCardCmd			109
#define ListRecordAttachRecordCmd				110
#define ListRecordAttachCategoryCmd				111
// Only used in command bar
#define ListRecordBeamRecordCmd					112


#define ListOptionsFontCmd						300
#define ListOptionsListByCmd					301
#define ListOptionsEditCustomFldsCmd			302
#define ListOptionsSecurityCmd					303
#define ListOptionsAboutCmd						304

#define RecordRecordDeleteRecordCmd				100
#define RecordRecordDuplicateAddressCmd			101
#define RecordRecordBeamRecordCmd				102
#define RecordRecordSendRecordCmd				103
#define RecordRecordDialCmd						104
#define RecordRecordSeparator1					105
#define RecordRecordSeparator2					108
#define RecordRecordAttachNoteCmd				106
#define RecordRecordDeleteNoteCmd				107
#define RecordRecordSelectBusinessCardCmd 		109
#define RecordRecordBeamBusinessCardCmd 		110
#define RecordRecordAttachRecordCmd		 		111

#define RecordOptionsFontCmd					200
#define RecordOptionsEditCustomFldsCmd			201
#define RecordOptionsAboutCmd					202

#define EditRecordDeleteRecordCmd				100
#define EditRecordDuplicateAddressCmd			101
#define EditRecordBeamRecordCmd					102
#define EditRecordSendRecordCmd					103
#define EditRecordDialCmd						104
#define EditRecordSeparator1					105
#define EditRecordAttachNoteCmd					106
#define EditRecordDeleteNoteCmd					107
#define EditRecordSeparator2					108
#define EditRecordSelectBusinessCardCmd 		109
#define EditRecordBeamBusinessCardCmd 			110
#define EditRecordAttachRecordCmd 				111

#define EditOptionsFontCmd						300
#define EditOptionsEditCustomFldsCmd			301
#define EditOptionsAboutCmd						302


// Strings
#define FindAddrHeaderStr						100		// "Addresses"
#define UnnamedRecordStr						1000	// "-Unnamed-"
#define BeamDescriptionStr						1001	// "an address"
#define BeamFilenameStr							1002	// "Address.vcf"
#define DuplicatedRecordIndicatorStr			1003	// "Copy"
#define DateAndTimeTemplate						1004	// "^0, ^1"
#define DeleteRecordStr							1005	// "Delete Address..."
#define BeamRecordStr							1006	// "Beam Address"
#define ExgDescriptionStr						1007	// "Business Card"
#define ExgMultipleDescriptionStr				1008	// "Business Cards"
#define SetDateTitleStr							1009	// "Set Date"
#define SetTimeTitleStr							1010	// "Set Time"
#define LabelSuffixColonStr						1011	// ":"
#define ShowInListTemplate						1012	// "^0 ^1" ^0 is the tab name, ^1 is the column name

#define TabNameAllStr							1700

// Soft Constant
#define TransfertPDIParameterID					1000	// Depending of the locale, the PDI should use specific parameters.

/***** STR ID FROM 3000 TO 3100 RESERVED FOR SEPARATOR/PREFIX/SUFFIX LAYOUT - Not used directly  *****/
// Define globaly
#define AddressLayoutPrefix_LeftParenthesisStr	3000	// "("
#define AddressLayoutPrefix_RightParenthesisStr	3001	// ")"
#define AddressLayoutSeparator_DefaultStr		3002	// " "
#define AddressLayoutSeparator_CommaStr			3003	// ", " -> with space

// Define localy
#define AddressLayoutPrefix_PostalMarkStr_JP	3004	// "?" -> Japanese character
#define AddressLayoutPrefix_ZipCodeMarkStr_CN	3005	// "?" -> Chine 'Zip Code Str'

#define AlternateDataFieldNoDataStr				4000	// "Tap to enter..."


// Order and country depend resources ID and base ID

// String List

/***** STRL ID FROM 1101 TO 1876 RESERVED FOR SCHEMA FIELD RENAMING DEPENDING OF THE COUNTRY CODE *****/

// String lists
#define FamilySuffixStrList						1850
#define AbbreviationsByKindStrList				1855

// Defined by locale
#define OrderBySQLQueriesStrList				2805
#define OrderNameStrList						2815


// uint32_t lists

/***** U32L ID FROM 1101 TO 1876 RESERVED FOR SCHEMA FIELD RENAMING DEPENDING OF THE COUNTRY CODE *****/

// Not used directly. Are referenced inside another resource (u32L 2805)
// Define globaly
#define ListPhoneNameFieldInfoUIn32List			2000
#define ListPhoneCompanyFieldInfoUIn32List		2005

// Contains as many items as OrderByStrList & OrderNameStrList
// Define globaly
#define OrderByListPhoneResIDUIn32List			2805	// List of u32l resource id (2000&2005) that define the default phone search order
#define OrderByLookupColumnIDResIDUIn32List		2825	// list of u32l resource id
#define OrderByDisplayLayoutResIDUIn32List		2815	// List of Base VBLT resource id (Display Name Layout: 2000 & 3000)
#define OrderByTypeUInt32List					2835

// Not used directly. Are referenced inside another resource (u32L 2825)
// Define localy
#define OrderBySortByNameLookupColumnIDs		2900
#define OrderBySortByCompanyColumnIDs			2910

// Defined only in main resource file.
#define DisplayNameColumnIDU32LList_ALL			3000
#define DisplayNameColumnIDU32LList_CN			3174

// Define in each localized file
#define DuplicateNameColumnIDU32LList			3500

#define FreeFormNameColumnIDU32LList_ALL		5000
#define FreeFormNameColumnIDU32LList_JP			5386
#define FreeFormNameColumnIDU32LList_CN			5174


/***** VBLT ID FROM 3000 TO 7000 RESERVED FOR LAYOUT DEPENDING OF THE COUNTRY CODE *****/

/***** VBLT ID FROM 3000 TO 3999 RESERVED FOR NAME LAYOUT (Sort by Last Name) - Not used directly *****/
#define DisplayNameByLastNameLayoutCountry_ALL	3000
#define DisplayNameByLastNameLayoutCountry_CN	3174

/* As the Layout is the same for all ROM Locale, there is no need for ROM country layout resource definition
   FYI: US=3719 - FR=3268 - ES=3239 - IT=3360 - DE=3195 - JP=3386 - CN=3174 */

/***** VBLT ID FROM 4000 TO 4999 RESERVED FOR NAME LAYOUT (Sort by Company) - Not used directly *****/
#define DisplayNameByCompanyLayoutCountry_ALL	4000
#define DisplayNameByCompanyLayoutCountry_CN	4174

/* As the Layout is the same for all ROM Locale, there is no need for ROM country layout resource definition
   FYI: US=4719 - FR=4268 - ES=4239 - IT=4360 - DE=4195 - JP=4386 - CN=4174 */

/***** VBLT ID FROM 5000 TO 5999 RESERVED FOR FREE FORM NAME LAYOUT - Not used directly *****/
#define FreeFormNameLayoutCountry_ALL			5000
#define FreeFormNameLayoutCountry_JP			5386
#define FreeFormNameLayoutCountry_CN			5174

/* As the Layout is the same for all ROM Locale except JP, only the JP layout is defined to overwrite the default
   FYI: US=5719 - FR=5268 - ES=5239 - IT=5360 - DE=5195 */

/***** VBLT ID FROM 6000 TO 6999 RESERVED FOR ADDRESS LAYOUT - Not used directly *****/
#define AddressLayoutCountry_US					6719
#define AddressLayoutCountry_FR					6268
#define AddressLayoutCountry_ES					6239
#define AddressLayoutCountry_IT					6360
#define AddressLayoutCountry_DE					6195
#define AddressLayoutCountry_JP					6386
#define AddressLayoutCountry_CN					6174

/* No default ROM country layout definition as each country has its own definition. In the future, if we
   define more country layout and found common one, we should define it as default one and remove the country
   specific one. */


/***** VRLT ID FROM x001 TO x100 (x = 3..6) RESERVED FOR LAYOUT DEPENDING OF THE COUNTRY CODE *****/

/***** VRLT ID FROM 3001 TO 3004 RESERVED FOR DISPLAY NAME LAYOUT - Sort by Last Name - Not used directly *****/
#define DisplayNameLineLayout_LastFirstName		3001
#define DisplayNameLineLayout_FirstName			3002
#define DisplayNameLineLayout_CompanyName		3003

/***** VRLT ID FROM 4001 TO 4004 RESERVED FOR DISPLAY NAME LAYOUT - Sort by Company - Not used directly *****/
#define DisplayNameLineLayout_CompanyLastName	4001
#define DisplayNameLineLayout_CompanyFirst		4002

/***** VRLT ID FROM 5001 TO 5004 RESERVED FOR FREE FORM NAME LAYOUT - Not used directly *****/
#define FreeFormNameLineLayout_LastName			5001	// Prefix Firstname Middlename Lastname(Mandatory) Suffix
#define FreeFormNameLineLayout_FirstName		5002	// Prefix Firstname(Mandatory) Middlename Lastname Suffix
#define FreeFormNameLineLayout_CompanyName		5003	// Company Name
#define FreeFormNameLineLayout_LastName2		5004	// Prefix Lastname(Mandatory) Firstname Suffix: Double byte
#define FreeFormNameLineLayout_FirstName2		5005	// Prefix Last First(Mandatory) Suffix: Double byte
#define FreeFormNameLineLayout_LastName3		5006	// Lastname(Mandatory)Firstname Prefix [(Englishname)] (zhCN specific): Double byte
#define FreeFormNameLineLayout_FirstName3		5007	// Firstname(Mandatory) Prefix [(Englishname)] (zhCN specific): Double byte
#define FreeFormNameLineLayout_EnglishName		5008	// Englishname (zhCN specific)

/***** VRLT ID FROM 6001 TO 6011 RESERVED FOR ADDRESS LAYOUT - Not used directly *****/
#define AddressLineLayout_Street					6001	// Street
#define AddressLineLayout_Country					6002	// Country
#define AddressLineLayout_State						6003	// State
#define AddressLineLayout_CountryState				6004	// Country (State)
#define AddressLineLayout_CityStateZip				6005	// City, State Zip
#define AddressLineLayout_ZipCityState				6006	// Zip City State
#define AddressLineLayout_ZipCityState2				6007	// Zip [City] (State)
#define AddressLineLayout_ZipCityState3				6008	// [Zip] City (State)
#define AddressLineLayout_ZipCity					6009	// Zip City
#define AddressLineLayout_Zip						6010	// Zip
#define AddressLineLayout_StateCityStreet			6011	// StateCityStreet
#define AddressLineLayout_CountryStateCityZipStreet	6012	// Country StateCity Zip Street
#define AddressLineLayout_CountryCityZipStreet		6013	// Country City Zip Street

/***** VRLT ID FROM 9000 TO 9100 RESERVED FOR COMMON LAYOUT - Not used directly *****/
#define AddressLineLayout_BlankLine				9000	// Blank line

#endif // ADDRESSRSC_H
