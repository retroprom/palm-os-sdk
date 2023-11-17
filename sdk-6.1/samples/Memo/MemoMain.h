/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoMain.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Include file the Memo application
 *
 *****************************************************************************/

#ifndef __MEMOMAIN_H__
#define	__MEMOMAIN_H__

#include <Menu.h>
#include <Field.h>

#include "MemoDB.h"

#include <PalmTypes.h>
#include <TextMgr.h>
#include <ExgMgr.h>
#include <SystemMgr.h>
#include <AppMgr.h>
#include <SchemaDatabases.h>
#include <AboutBox.h>
#include <PrivateRecords.h>

#define memoExtension					"txt"
#define memoMIMEType					"text/plain"

#define memoVersionNum					3

#define scrollTimeOutWhileDragging		(SysTimeInMilliSecs(10))

#define noRecordSelectedID				-1
#define noRecordSelected				0xffffffff

#define attachTransportPrefixMaxSize	16

typedef enum 
{
	appLaunchCmdExgGetFullLaunch = sysAppLaunchCmdCustomBase
} 
MemoCustomLaunchCodes;

/************************************************************
 * Global variables
 *************************************************************/
extern char						gCategoryName[catCategoryNameLength];

extern privateRecordViewEnum	gPrivateRecordVisualStatus;
extern MenuBarPtr				gCurrentMenu;

extern uint32_t					gTopVisibleRecord;
extern uint32_t					gCurrentRecord;
extern uint16_t					gCurrentView;

extern CategoryID *				gCurrentCategoriesP;
extern uint32_t					gCurrentNumCategories;

extern FontID					gListFont;
extern FontID					gEditFont;
extern uint32_t					gEditScrollPosition;
extern Boolean					gSaveBackup;
extern Boolean					gSortAlphabetically;


// Attachments
extern Boolean					gAttachRequest;				
extern char						gTransportPrefix[attachTransportPrefixMaxSize+1];

// Application dbRef
extern DmOpenRef				gApplicationDbP;
// Memo dbRef
extern DmOpenRef				gMemoDB;
// Memo max length
extern int32_t 					gEventLoopWaitTime;

extern Boolean 					gForceReload;

// current window bounds.
extern RectangleType 			gCurrentWinBounds;

// structure for use in caching the contents of the list view
typedef struct _memolist_t {
	uint16_t		attr;
	uint32_t		cursorPos;
	char *			title;
	int16_t			titleAlloc;
	int16_t			indent;
} memolist_t;

#define							kNumPagesInCache	4 // number of pages cached for scrolling

// bufferize chars from list view to edit view
#define							kNewCharBufSize 8
extern Boolean					gEnqueueNewChar;
extern EventType				gNewCharBuffer[];
extern size_t					gNewCharBufferCount;


extern RGBColorType		gMemoListDarkenedListEntry;

extern RGBColorType	   gEditBackgroundColor;
extern RGBColorType		gEditSideLineColor;
extern RGBColorType		gEditFieldLineColor;

/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void		DrawMemoTitle(char * memo, int16_t x, int16_t y, int16_t width);

void *		GetObjectPtr(uint16_t objectID, uint16_t formID);

FieldType *	GetFocusObjectPtr(uint16_t formID);

void		ChangeCategory(CategoryID *categoriesP, uint32_t numCategories);

Boolean		SeekRecord(uint32_t * indexP, int16_t offset, int16_t direction);

void		RegisterData(void);

void		UtilsFrmInvalidateWindow(uint16_t formID);

#ifdef __cplusplus 
}
#endif

#endif	//	__MEMOMAIN_H__
