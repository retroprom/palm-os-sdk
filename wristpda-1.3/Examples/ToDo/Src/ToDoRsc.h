/******************************************************************************
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: ToDoRsc.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

// Note View

#if WRISTPDA

#ifdef NewNoteView
#undef NewNoteView
#endif

#define NewNoteView						10950

#ifdef NoteField
#undef NoteField
#endif

#define NoteField						10951

#ifdef NoteDoneButton
#undef NoteDoneButton
#endif

#define NoteDoneButton 					10952

#ifdef NoteDeleteButton
#undef NoteDeleteButton
#endif

#define NoteDeleteButton 				10953

#ifdef NoteScrollBar
#undef NoteScrollBar
#endif

#if WRISTPDA
#define NotePageUp						10954
#define NotePageDown					10955
#endif

#endif

// List View
#define ListView						1000
#define ListCategoryTrigger				1003
#define ListCategoryList				1004
#define ListNewToDoButton				1005
#define ListDetailsButton				1006
#define ListShowButton					1007
#define ListUpButton					1008
#define ListDownButton					1009
#define ListTable						1010
#define ListPriorityList				1011
#define ListDueDateList					1012
#define ListItemsCategoryList			1013

#if WRISTPDA
#define ListPageUp						1014
#define ListPageDown					1015
#endif

// Details Dialog Box
#define DetailsDialog					1200
#define DetailsPriority1Trigger			1203
#define DetailsCategoryTrigger			1210
#define DetailsCategoryList				1211
#define DetailsDueDateTrigger			1214
#define DetailsDueDateList				1215
#define DetailsSecretCheckbox			1217
#define DetailsOkButton					1218
#define DetailsCancelButton				1219
#define DetailsDeleteButton				1220
#define DetailsNoteButton				1221
#define DetailsHelpString				1222
#define DetailsPrioritiesGroup			1

// Options Dialog
#ifdef WRISTPDA

#define OptionsDialog1					1400
#define OptionsSortByTrigger			1404
#define OptionsSortByList				1405
#define OptionsShowCompleted			1406
#define OptionsShowDueItems				1407
#define OptionsOkButton1				1413
#define OptionsCancelButton1			1414
#define OptionsNextButton				1415

#define OptionsDialog2					1450
#define OptionsChangeDueDate			1454
#define OptionsShowDueDates				1459
#define OptionsShowPriorities			1461
#define OptionsShowCategories			1462
#define OptionsOkButton2				1463
#define OptionsCancelButton2			1464
#define OptionsPrevButton				1465

#else
#define OptionsDialog					1400
#define OptionsSortByTrigger			1404
#define OptionsSortByList				1405
#define OptionsShowCompleted			1406
#define OptionsShowDueItems				1407
#define OptionsChangeDueDate			1408
#define OptionsShowDueDates				1409
#define OptionsShowPriorities			1411
#define OptionsShowCategories			1412
#define OptionsOkButton					1413
#define OptionsCancelButton				1414
#endif

// Detele Completed Dialog
#define DeleteCompletedDialog			1500
#define DeleteCompletedSaveBackup		1504
#define DeleteCompletedOk				1506
#define DeleteCompletedCancel			1507

// Delete To Do Dialog
#define DeleteToDoDialog				1600
#define DeleteToDoSaveBackup			1604
#define DeleteToDoOk					1606
#define DeleteToDoCancel				1607

// Delete Note Alert
#define DeleteNoteAlert					2001
#define DeleteNoteYes					0
#define DeleteNoteNo             		1

// Select An Item Alert
#define SelectItemAlert					2002

// Menus
#define ListViewMenu					1000
#define NoteViewMenu					1001

// Menu commands
#define DeleteCmd						100
#define CreateNoteCmd					101
#define DeleteNoteCmd					102
#define DeleteCompletedCmd				103
#define BeamRecordCmd					105
#if WRISTPDA
#define SendRecordCmd					107
#define BeamCategoryCmd					106
#define SendCategoryCmd					108
#else
#define SendRecordCmd					106
#define BeamCategoryCmd					107
#define SendCategoryCmd					108
#endif
#define UndoCmd							200
#define CutCmd							201
#define CopyCmd							202
#define PasteCmd						203
#define SelectAllCmd					204
#define EditSeparator					205
#define KeyboardCmd						206
#define FontCmd							300
#define PhoneLookupCmd					301
#define SecurityCmd						302
#if WRISTPDA
#define ShowCmd							303
#define AboutCmd						304
#else
#define AboutCmd						303
#endif

//Command bars
#define GeneralMenuCtl					100 //hasCCP, has extras

// Strings
#define FindToDoHeaderStr				100
#define DueDateTitleStr					101
#define BeamDescriptionStr				1000
#define BeamFilenameStr					1001
