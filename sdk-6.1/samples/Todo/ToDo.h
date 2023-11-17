/******************************************************************************
 *
 * Copyright (c) 1997-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ToDo.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header for To Do.
 *
 *****************************************************************************/

#ifndef _TODO_H_
#define _TODO_H_

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>
#include <CmnLaunchCodes.h>
#include <Form.h>
#include <ExgMgr.h>
#include <DataMgr.h>

#include "ToDoDB.h"

/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define toDoVersionNum				3
#define toDoPrefsVersionNum			3
#define todoPrefID					0x00
#define toDoDBType					'DATA'
#define kToDoDBName					"RAM_ToDoDBS"
#define kToDoDBSchemaName			"RAM_ToDoDBSchema"

// If the preferences version found is less than or equal to this
// value, we need to update the font info.
#define toDoPrefsVerUpdateFonts			2

// Fonts used by application
#define noteTitleFont					boldFont

// Columns in the ToDo table of the list view.
#define completedColumn					0
#define priorityColumn					1
#define descColumn						2
#define dueDateColumn					3
#define categoryColumn					4

#define spaceNoPriority					1
#define spaceBeforeDesc					2
#define spaceBeforeCategory				2

#define maxNoteTitleLen					40

// Due Date popup list chooses
#define dueTodayItem					0
#define dueTomorrowItem					1
#define dueOneWeekLaterItem				2
#define noDueDateItem					3
#define selectDateItem					4

// Sort Order popup list choices
#define noSortOrder						0xff
#define priorityDueDateItem				0
#define dueDatePriorityItem				1
#define categoryPriorityItem			2
#define categoryDueDateItem				3
#define sortingByCategories(x) ((x) == categoryPriorityItem || (x) == categoryDueDateItem)

// Update codes, used to determine how the ToDo list should 
// be redrawn.
#define updateRedrawAll					0x00
#define updateItemDelete				0x01
#define updateItemMove					0x02
#define updateItemHide					0x04
#define updateCategoryChanged			0x08
#define updateDisplayOptsChanged		0x10
#define updateGoTo						0x20
#define updateFontChanged				0x40

// Events for UI outside updates
#define	updateCategoryTriggerEvent		firstUserEvent

#define noRecordSelected				0xffffffff

#define truncCategoryMaxWidth			640

// Number of system ticks to display crossed out item before they're erased.
#define crossOutDelay					(SysTimeInMilliSecs(500))

// Buffer limits
#define attachTransportPrefixMaxSize	16
#define maxCategoryNameLength (catCategoryNameLength*16)

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

// This is the structure of the data stored in the state file.
typedef struct {
	uint16_t		currentCategory;
	FontID			v20NoteFont;		// For 2.0 compatibility (BGT)
	Boolean			showAllCategories;	// 4 bytes
	Boolean 		showCompletedItems;
	Boolean 		showOnlyDueItems;
	Boolean			showDueDates;
	Boolean			showPriorities;		// 8 bytes
	Boolean			showCategories;		// added in version 2 preferences
	Boolean			saveBackup;
	Boolean			changeDueDate;		// added in version 2 preferences
	
	// Version 3 preferences
	FontID			listFont;			// 12 bytes
	FontID			noteFont;			// For 3.0 and later units.	(BGT)

	uint8_t			sortId ;			// For 6.0 Data Manager
} ToDoPreferenceType;

typedef struct {
	DmOpenRef		db;
	char *			categoryName;
	uint16_t		categoryIndex;
	uint16_t		padding;
} AcceptBeamType;

/************************************************************
 * Application specific launch codes
 *************************************************************/
typedef enum 
{
	todoLaunchCmdImportVObject = sysAppLaunchCmdCustomBase,
	appLaunchCmdExgGetFullLaunch
} 
ToDoCustomLaunchCodes;

// Application db
extern DmOpenRef						gApplicationDbP;

/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 *
 *	Routines exported from ToDo.c, used in ToDoInDatebook.c
 *
 ***********************************************************************/
status_t 	StartApplication (void);
void 		StopApplication (void);
void 		ListViewInit (FormPtr frm);
Boolean 	SeekRecord (uint32_t* indexP, int32_t offset, int32_t direction);
void 		ListViewResizeTableColumns(TableType* tableP, Coord offsetX, Boolean showCategories);
Boolean		ListViewSelectCategory (void);
void 		ListViewChangeCompleteStatus (int16_t row, uint16_t complete);
void 		ListViewLoadTable (Boolean fillTable);
void 		SetDBBackupBit(DmOpenRef dbP);


#ifdef __cplusplus 
}
#endif

#endif /* __TODO_H__ */
