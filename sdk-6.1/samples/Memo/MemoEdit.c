/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MemoEdit.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#ifdef PIM_APPS_PROFILING
#include <PIMAppsProfiling.h>
#else
#define PIMAppProfilingBegin(name)
#define PIMAppProfilingEnd()
#define PIMAppProfilingReturnVoid()	return
#define PIMAppProfilingReturnValue(val)	(return (val))
#endif

#include <PalmTypes.h>
#include <Event.h>
#include <Form.h>
#include <UIResources.h>
#include <FontSelect.h>
#include <ExgLocalLib.h>
#include <Table.h>
#include <CmnRectTypes.h>
#include <ErrorMgr.h>
#include <StringMgr.h>
#include <Control.h>
#include <ScrollBar.h>
#include <CatMgr.h>
#include <UIColor.h>
#include <PenInputMgr.h>
#include <Preferences.h>
#include <PhoneLookup.h>
#include <SoundMgr.h>
#include <FormLayout.h>
#include <Loader.h>
#include <TraceMgr.h>

#include "MemoRsc.h"
#include "MemoMain.h"
#include "MemoPrefs.h"
#include "MemoDB.h"
#include "MemoTransfer.h"

/************************************************************
 *	Global variables
 ************************************************************/
extern uint32_t			gListViewCursorID;

/***********************************************************************
 *	Static local variables
 ***********************************************************************/
static uint32_t			gTitlePos;

static CategoryID *		sDetailsCategoriesP = NULL;
static uint32_t			sDetailsNumCategories = 0;
static char				sDetailsCategoryName[catCategoryNameLength+1];

static CategoryID *		sDetailsSavedCategoriesP = NULL;
static uint32_t			sDetailsSavedNumCategories = 0;
static uint32_t			sEditPosition = 0;
static uint32_t			sEditSelectionLength = 0;

static Boolean			sInitialResizeEvent = false;
static Boolean			gEditNavFocus = false;
static Boolean			gEditing = false;

																						  

/***********************************************************************
 *	Internal Functions
 ***********************************************************************/
static void		EditViewDrawCounter(uint32_t pos, uint32_t total);
static void		EditViewResize(FormType *formP, RectangleType *newBoundsP);
static Boolean	EditViewDeleteRecord(void);
static void 	EditViewResizeFieldAndScrollBar(FormType *formP);
static void		EditViewRestoreEditState(void);
static int32_t		EditViewUpdateScrollBar(void);
static Boolean PrvBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP);

/***********************************************************************
 *	Internal Macros
 ***********************************************************************/

#define scrollFieldNone						0	// No scroll bar
#define scrollFieldTop						1
#define scrollFieldMiddle					2
#define scrollFieldBottom					3

#define kCategoriesChangedEvent				((uint16_t)(firstUserEvent + 1))
#define kCatPopupIndicatorGapAndWidth		12
#define kCatPopupSpaceOnRight				5
#define kEndOfFieldCursorPostion			0xFFFFFFFF
#define kSpaceBetweenFieldAndDoneButton		3
#define detailsTriggerSelectorsSpaceOnRight	10

#define kSmallCounterTemplate				"^0/^1"

static const struct FormLayoutType formLayout[] = {
	{ sizeof(FormLayoutType), EditViewBackground, 0, frmFollowAllSides },
	{ 0, 0 ,0, 0 }
};


static RGBColorType gOriginalFieldBackgroundColor;

/***********************************************************************
 *
 * FUNCTION:    MemoEditSetCategory
 *
 * DESCRIPTION: Releases edit field storage, changes category, 
 *	and returns storage
 *
 * PARAMETERS:  DbSetCategory parameters
 *
 * RETURNED:    DbSetCategory result
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *		LYr			9/17/03		created
 *
 ***********************************************************************/
static status_t MemoEditSetCategory(DmOpenRef dbRef, uint32_t rowID, uint32_t numToSet, const CategoryID categoryIDs[])
{
	FormType * formP;
	FieldType * fieldP;
	status_t err ;
	CategoryID all = catIDAll ;

	TraceOutput(TL(appErrorClass, "MemoEditSet %d Categories on rowID %d", numToSet, rowID));
	// Release the storage from the field so we can change the category
	formP = FrmGetFormPtr(EditView);
	fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, EditMemoField));
	FldReleaseStorage(fieldP);

	err = DbSetCategory(dbRef, rowID, numToSet, categoryIDs);
	if(err < errNone)
	{
		err = DbSetCategory(dbRef, rowID, 0, NULL);
		ChangeCategory(&all, 1);
	}


	// Return the storage to the field
	FldReturnStorage(fieldP);

	return err ;
}

/***********************************************************************
 *
 * FUNCTION:    DetailsSetCatTrigger
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
static void DetailsSetCatTrigger(void)
{
	FormPtr			formP;
	uint16_t		maxCatWidth, catTriggerIndex;
	Coord			catPosx, catPosy;
	RectangleType	formRect;

	formP = FrmGetFormPtr(DetailsDialog);

	// Set the label of the category trigger.
	catTriggerIndex = FrmGetObjectIndex (formP, DetailsCategoryTrigger);
	FrmGetObjectPosition(formP, catTriggerIndex, &catPosx, &catPosy);
	FrmGetFormInitialBounds(formP, &formRect);
	maxCatWidth = formRect.extent.x - catPosx - detailsTriggerSelectorsSpaceOnRight ;

	CatMgrTruncateName(gMemoDB, sDetailsCategoriesP, sDetailsNumCategories, maxCatWidth, sDetailsCategoryName);
	CtlSetLabel (FrmGetObjectPtr (formP, catTriggerIndex), sDetailsCategoryName);
}

/***********************************************************************
 *
 * FUNCTION:    DetailsSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    index of the selected category.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static Boolean DetailsSelectCategory(void)
{
	Boolean			categoriesChanged;
	CategoryID *	selectedCategoriesP = NULL;
	uint32_t		selectedNumCategories = 0;
	status_t		err;

	// Close our cursor in case current category was deleted
	DbCursorClose(gListViewCursorID);
	gListViewCursorID = dbInvalidCursorID;

	categoriesChanged = CatMgrSelectEdit(gMemoDB, FrmGetFormPtr(DetailsDialog),
									DetailsCategoryTrigger, sDetailsCategoryName, DetailsCategoryList, true,
									sDetailsCategoriesP, sDetailsNumCategories,
									&selectedCategoriesP, &selectedNumCategories,
									true, NULL);

	// Reopen it. In case of failure, it will switch to catIDAll
	MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
	
	TraceOutput(TL(appErrorClass, "Memo Details : CatMgrSelectEdit %d (0x%x) => %d (0x%x) : %s", 
		sDetailsNumCategories, sDetailsCategoriesP, selectedNumCategories, selectedCategoriesP,
		(categoriesChanged ? "true" : "false")));

	if (categoriesChanged)
	{
		if (sDetailsCategoriesP)
			DbReleaseStorage(gMemoDB, sDetailsCategoriesP);

		MemoEditSetCategory(gMemoDB, gCurrentRecord, selectedNumCategories, selectedCategoriesP);
		err = DbGetCategory(gMemoDB, gCurrentRecord, &sDetailsNumCategories, &sDetailsCategoriesP);
		ErrNonFatalDisplayIf(err < errNone, "Can't read category");

		selectedCategoriesP = NULL;
		DetailsSetCatTrigger();
	}
	
	if (selectedCategoriesP)
		CatMgrFreeSelectedCategories(gMemoDB, &selectedCategoriesP);
	
	return categoriesChanged;
}


/***********************************************************************
 *
 * FUNCTION:    DetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  category - new catagory
 *
 * RETURNED:    code which indicates how the memo was changed,  this
 *              code is sent as the update code, in the frmUpdate event.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *			kcr	10/9/95	added 'private records' alert.
 *
 ***********************************************************************/
static void DetailsApply(Boolean categoriesChanged)
{
	uint16_t	attr;
	Boolean		secret;

	// Get the category and secret attribute of the current record.
	DbGetRowAttr(gMemoDB, gCurrentRecord, &attr);

	// Get the current setting of the secret checkbox.
	secret = (CtlGetValue (GetObjectPtr (DetailsSecretCheckbox, DetailsDialog)) != 0);

	// Has the secret attribute was been changed?
	if (((attr & dbRecAttrSecret) == dbRecAttrSecret) != secret)
	{
		if (secret)
		{
			attr |= dbRecAttrSecret;
			if (gPrivateRecordVisualStatus == showPrivateRecords)
				FrmUIAlert(privateRecordInfoAlert);
		}
		else
		{
			attr &= ~dbRecAttrSecret;
		}

		DbSetRowAttr(gMemoDB, gCurrentRecord, &attr);
	}

	// Has the category been changed?
	if (categoriesChanged)
	{
		if (!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll)
			ChangeCategory(sDetailsCategoriesP, sDetailsNumCategories);
	}
}


/***********************************************************************
 *
 * FUNCTION:    DetailsInit
 *
 * DESCRIPTION: This routine initializes the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
static void DetailsInit(void)
{
	uint16_t		attr;
	Boolean 		secret;
	ControlPtr		ctlP;
	status_t		err;

	// Get the secret attribute of the current record.
	DbGetRowAttr(gMemoDB, gCurrentRecord, &attr);
	secret = attr & dbRecAttrSecret;
	
	// If the record is marked secret, turn on the secret checkbox.
	ctlP = GetObjectPtr (DetailsSecretCheckbox, DetailsDialog);
	CtlSetValue (ctlP, secret);

	// Get record categories
	err = DbGetCategory(gMemoDB, gCurrentRecord, &sDetailsNumCategories, &sDetailsCategoriesP);
	ErrNonFatalDisplayIf(err < errNone, "Can't read category");
	err = DbGetCategory(gMemoDB, gCurrentRecord, &sDetailsSavedNumCategories, &sDetailsSavedCategoriesP);
	ErrNonFatalDisplayIf(err < errNone, "Can't read category");


	// Update category trigger
	DetailsSetCatTrigger();
}

/***********************************************************************
 *
 * FUNCTION:    DetailsFree
 *
 * DESCRIPTION: This routine frees the category list dynamically allocated
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		10/28/02	Initial Revision
 *
 ***********************************************************************/
static void DetailsFree(void)
{
	if (sDetailsCategoriesP)
	{
		DbReleaseStorage(gMemoDB, sDetailsCategoriesP);
		sDetailsCategoriesP = NULL;
		sDetailsNumCategories = 0;
	}

	if (sDetailsSavedCategoriesP)
	{	
		DbReleaseStorage(gMemoDB, sDetailsSavedCategoriesP);
		sDetailsSavedCategoriesP = NULL;
		sDetailsSavedNumCategories = 0;
	}

}

/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
Boolean DetailsHandleEvent (EventType * event)
{
	static Boolean	categoriesChanged = false;
	static uint16_t	objectFocused = 0xffff;
	FormType *	formP;
	Boolean		handled = false;
	EventType	evt;

	switch (event->eType)
	{
		case frmObjectFocusTakeEvent:
			if (event->data.frmObjectFocusTake.formID == DetailsDialog)
				objectFocused = event->data.frmObjectFocusTake.objectID;
			break;

		case keyDownEvent:
			if (event->data.keyDown.chr == vchrRockerRight
			&&	objectFocused == DetailsCategoryTrigger)
			{
				formP = FrmGetFormPtr(DetailsDialog);
				FrmNavObjectTakeFocus(formP, DetailsOkButton);
				handled = true;
			}
			break;

		case ctlSelectEvent:
		switch (event->data.ctlSelect.controlID)
		{
			case DetailsOkButton:
				DetailsApply(categoriesChanged);
				DetailsFree();
				
				FrmReturnToForm(EditView);
				
				if (categoriesChanged)
				{
					evt.eType = kCategoriesChangedEvent;
					EvtAddEventToQueue(&evt);
				}
				
				handled = true;
				break;

			case DetailsCancelButton:
				if (categoriesChanged)
				{
					// user cancelled so we restore previous category choice
					MemoEditSetCategory(gMemoDB, gCurrentRecord, sDetailsSavedNumCategories, sDetailsSavedCategoriesP);
					// but send an event anyway in case the previous category was destroyed
					evt.eType = kCategoriesChangedEvent;
					EvtAddEventToQueue(&evt);
				}

				FrmReturnToForm(EditView);

				DetailsFree();

				handled = true;
				break;

			case DetailsDeleteButton:
				if (EditViewDeleteRecord())
				{
					if (categoriesChanged && (!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll))
						ChangeCategory(sDetailsCategoriesP, sDetailsNumCategories);

					formP = FrmGetFormPtr(DetailsDialog);
					FrmEraseForm(formP);
					FrmDeleteForm(formP);
					FrmCloseAllForms();
					FrmGotoForm (gApplicationDbP, ListView);

					DetailsFree();
				}

				handled = true;
				break;

			case DetailsCategoryTrigger:
				categoriesChanged = DetailsSelectCategory() || categoriesChanged;
				handled = true;
				break;
		}
		break;
	
	case frmOpenEvent:
		DetailsInit();
		categoriesChanged = false;
		objectFocused = 0xffff;
		handled = true;
		break;

	case frmCloseEvent:
		DetailsFree();
		break;

	case winUpdateEvent:
		formP = FrmGetFormPtr(DetailsDialog);
		if (event->data.winUpdate.window != FrmGetWindowHandle(formP))
			break;
		FrmDrawForm(formP);
		handled = true;
		break;

	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    EditViewSetCounter
 *
 * DESCRIPTION: This routine formats and sets the title of the Edit View.
 *              If the Edit View is visible, the new title is drawn.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *	02/21/95	art	Created by Art Labm.
 *	08/31/00	kwk	Re-wrote to use TxtParamString, versus hard-coding
 *					order of elements using '#' in template string.
 *	11/22/00	FPa	Added DmReleaseResource()
 *	11/28/00	CS	Use FrmSetTitle instead of FrmCopyTitle and leave it
 *					allocated until EditViewExit.  This removes the
 *					dependency that the form title in the resource has to
 *					be wide enough to accomodate the longest possible title.
 *	11/30/00	FPa	Fixed bug #46014
 *	07/12/04	LYr splitted EditViewSetCounter with EditViewDrawCounter
 *
 ***********************************************************************/
static void EditViewSetCounter(void)
{
	uint32_t		size;
	uint32_t		memosInCategory = 0;
	status_t		err;
		
	MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;
	// Format as strings, the memo's postion within its category, and
	// the total number of memos in the category.
	gTitlePos = size = 0 ;
	err = DbCursorMoveToRowID(gListViewCursorID, gCurrentRecord);
	DbCursorGetCurrentPosition(gListViewCursorID, &gTitlePos);
	memosInCategory = MemoDBNumVisibleRecordsInCategory(gListViewCursorID);
	// marked record private and changed category with 'hidePrivateRecords' policy
	if (err < errNone && gPrivateRecordVisualStatus == hidePrivateRecords)
		gTitlePos = ++memosInCategory; // then pretend it's the last record, this one added
	if (gTitlePos == 1)
	{	// new empty record created ? 
		DbCopyColumnValue(gMemoDB, gCurrentRecord, kMemoDBColumnID, 0, NULL, &size);
		if (size == 0) // then pretend it's the last record
			gTitlePos = memosInCategory ; 		
	}

	EditViewDrawCounter(gTitlePos, memosInCategory);
}

/***********************************************************************
 *
 * FUNCTION:    EditViewDrawCounter
 *
 * DESCRIPTION: This routine formats and sets the title of the Edit View.
 *              If the Edit View is visible, the new title is drawn.
 *				No DB access is done in this routine.
 *
 * PARAMETERS:  pos / total
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *	07/12/04	LYr splitted EditViewSetCounter with EditViewDrawCounter
 *
 ***********************************************************************/
static void EditViewDrawCounter(uint32_t pos, uint32_t total)
{
	static Coord	maxWidth = 0;
	static Coord	labelMinX = 0;
	static RectangleType labelRect;
	MemHandle		largeCounterTemplateH;
	char *			largeCounterTemplateP;
	char *			counterP;
	char			posStr[maxStrIToALen + 1];
	char			totalStr[maxStrIToALen + 1];
	FormType *		formP;
	RectangleType	bounds;
	Coord			width;
	uint16_t		objIndex;
	FontID			savedFont;
	WinHandle		winH;
	
	formP = FrmGetFormPtr(EditView);
	winH = FrmGetWindowHandle(formP);
	
	StrIToA(posStr, (int32_t) pos);
	StrIToA(totalStr, total);
	
	// Calculate label max width (done only once: maxWidth is static)
	if (maxWidth == 0)
	{
		FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditRightButton), &bounds);
		maxWidth = bounds.topLeft.x;
		FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditLeftButton), &bounds);
		labelMinX = bounds.topLeft.x + bounds.extent.x + 1;
		maxWidth -= labelMinX + 1;
		labelRect.topLeft.x = labelMinX;
		labelRect.topLeft.y = bounds.topLeft.y;
		labelRect.extent.x = maxWidth;
		labelRect.extent.y = bounds.extent.y;
	}

	savedFont = FntSetFont(stdFont);

	// Load the large counter template "XXX of XXX"
	largeCounterTemplateH = DmGetResource(gApplicationDbP, strRsc, EditLargeCounterString);
	largeCounterTemplateP = MemHandleLock(largeCounterTemplateH);
	counterP = TxtParamString(largeCounterTemplateP, posStr, totalStr, NULL, NULL);	// title needs to be freed
	width = FntCharsWidth(counterP, StrLen(counterP));
	// If it fits within maxWidth, use it
	if (width <= maxWidth)
		goto SetLabel;

    // Or try with the small template "XXX/XXX"	
	MemPtrFree(counterP);
	counterP = TxtParamString(kSmallCounterTemplate, posStr, totalStr, NULL, NULL);
	width = FntCharsWidth(counterP, StrLen(counterP));
	if (width <= maxWidth)
		goto SetLabel;

	// Last chance: Use only posStr
	MemPtrFree(counterP);
	counterP = NULL;
	width = FntCharsWidth(posStr, StrLen(posStr));
	
SetLabel:

	// Force redraw of label rectangle
	WinInvalidateRect(winH, &labelRect);

	// Center the label between the 2 arrows
	objIndex = FrmGetObjectIndex(formP, EditCounterLabel);
	FrmGetObjectPosition(formP, objIndex, &bounds.topLeft.x, &bounds.topLeft.y);
	bounds.topLeft.x = labelMinX + ((maxWidth >> 1) - (width >> 1));
	FrmSetObjectPosition(formP, objIndex, bounds.topLeft.x, bounds.topLeft.y);

	// Copy text
	FrmCopyLabel(formP, EditCounterLabel, counterP ? counterP : posStr);

	// Cleanup
	if (counterP)
		MemPtrFree(counterP);
	MemHandleUnlock(largeCounterTemplateH);
	DmReleaseResource(largeCounterTemplateH);

	FntSetFont(savedFont);
}

/***********************************************************************
 *
 * FUNCTION:    PrvEditSetCategoryTriggerLabel
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 *
 *
 ***********************************************************************/
static void EditViewSetCategoryTriggerLabel(FormType *formP)
{
	Coord			displayWidth;
	Coord			labelWidth;
	RectangleType	bounds;
	ControlType *	triggerP;
	uint32_t		numCategories = 0;
	CategoryID *	categoriesP = NULL;
    status_t		err;
	FontID			currentFont;

	err = DbGetCategory(gMemoDB, gCurrentRecord, &numCategories, &categoriesP);
	if (err < errNone)
		return;

	WinGetDisplayExtent(&displayWidth, NULL);

	// Compute maximum label width
	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, EditRightButton), &bounds);
	labelWidth = displayWidth - (bounds.topLeft.x + bounds.extent.x) - 2;
	
	// Get truncated category for maximum width
	CatMgrTruncateName(gMemoDB, categoriesP, numCategories, labelWidth, gCategoryName);

	// Get real width
	currentFont = FntSetFont(stdFont);
	labelWidth = FntCharsWidth(gCategoryName, StrLen(gCategoryName));
	FntSetFont(currentFont);

	// Set trigger
	triggerP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, EditCategoryTrigger));
	CtlSetLabel(triggerP, gCategoryName);

	// Cleanup
	if (categoriesP)
		DbReleaseStorage(gMemoDB, categoriesP);
}

/***********************************************************************
 *
 * FUNCTION:    EditViewSelectCategory
 *
 * DESCRIPTION: This routine  recategorizes a memo in the "Edit View".
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/03/95	Initial Revision
 *			grant	86/29/99	Adjust MemosInCategory when the record is private
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static void EditViewSelectCategory (void)
{
	WinHandle		winHndH;
	FormType *		formP;
	FieldType *		fieldP;
	Boolean			categoryChanged;
	CategoryID *	currentCategoriesP = NULL;
	uint32_t		currentNumCategories = 0;
	CategoryID *	selectedCategoriesP = NULL;
	uint32_t		selectedNumCategories = 0;
	size_t			startPosition, endPosition, scrollPosition;
	status_t		err;

	// Get the current category.
	err = DbGetCategory(gMemoDB, gCurrentRecord, &currentNumCategories, &currentCategoriesP);
	ErrNonFatalDisplayIf(err < errNone, "Can't read category");

	// Process the category popup list.
	formP = FrmGetFormPtr(EditView);

	// Close our cursor in case current category was deleted
	DbCursorClose(gListViewCursorID);
	gListViewCursorID = dbInvalidCursorID;

	// Release field storage
	fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex (formP, EditMemoField));
	scrollPosition = FldGetScrollPosition(fieldP);
	FldGetSelection(fieldP, &startPosition, &endPosition);
	sEditPosition = startPosition;
	sEditSelectionLength = (uint32_t)(endPosition - startPosition);
	FrmSetFocus(formP, noFocus);
	FldFreeMemory(fieldP);
	FldSetTextHandle(fieldP, NULL);

	categoryChanged = CatMgrSelectEdit(gMemoDB, formP, EditCategoryTrigger, gCategoryName, EditCategoryList,
					true, currentCategoriesP, currentNumCategories, &selectedCategoriesP, &selectedNumCategories, true, NULL);

	// Return field storage
	FldSetTextColumn(fieldP, gMemoDB, gCurrentRecord, 0, kMemoDBColumnID, 0);

	// Reopen cursor. In case of failure, it will switch to catIDAll
	MemoDBOpenCursor(&gMemoDB, &gListViewCursorID, gCurrentCategoriesP, gCurrentNumCategories, gSortAlphabetically) ;

	TraceOutput(TL(appErrorClass, "Memo Edit : CatMgrSelectEdit %d (0x%x) => %d (0x%x) : %s", 
		currentNumCategories, currentCategoriesP, selectedNumCategories, selectedCategoriesP,
		(categoryChanged ? "true" : "false")));

	if (currentCategoriesP)
		DbReleaseStorage(gMemoDB, currentCategoriesP);

	// If the current category was changed or the name of the category
	// was edited,  draw the title.
	if (categoryChanged)
	{
		MemoEditSetCategory(gMemoDB, gCurrentRecord, selectedNumCategories, selectedCategoriesP);

		EditViewSetCategoryTriggerLabel(formP);

		if (!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll)
			ChangeCategory(selectedCategoriesP, selectedNumCategories);
		EditViewSetCounter();
	}

	if (selectedCategoriesP)
		CatMgrFreeSelectedCategories(gMemoDB, &selectedCategoriesP);

	// Restore edit state, scroll always
	FrmSetFocus(formP, FrmGetObjectIndex(formP, EditMemoField)); 
	// Scroll to correct position
	FldSetScrollPosition(fieldP, scrollPosition);
	// Update scroll bar to the correct position
	EditViewUpdateScrollBar();
	if (sEditSelectionLength > 0)
		FldSetSelection(fieldP, sEditPosition, sEditPosition + sEditSelectionLength);	
	else
		FldSetInsertionPoint(fieldP, sEditPosition);				

	winHndH = FrmGetWindowHandle(formP);
	WinInvalidateWindow(winHndH);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewUpdateScrollBar
 *
 * DESCRIPTION: This routine update the scroll bar.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/01/96	Initial Revision
 *			gap	11/02/96	Fix case where field and scroll bars get out of sync
 *			lyr	08/12/04	Return field scroll state (Top, Middle, Bottom) 
 *
 ***********************************************************************/
static int32_t EditViewUpdateScrollBar ()
{
	uint32_t	scrollPos;
	uint32_t	textHeight;
	uint32_t	fieldHeight;
	int32_t	maxValue;
	FieldPtr fieldP;
	ScrollBarPtr bar;

	fieldP = GetObjectPtr (EditMemoField, EditView);
	bar = GetObjectPtr (EditMemoScrollBar, EditView);

	FldGetScrollValues (fieldP, &scrollPos, &textHeight,  &fieldHeight);

	if (textHeight > fieldHeight)
	{
		// On occasion, such as after deleting a multi-line selection of text,
		// the display might be the last few lines of a field followed by some
		// blank lines.  To keep the current position in place and allow the user
		// to "gracefully" scroll out of the blank area, the number of blank lines
		// visible needs to be added to max value.  Otherwise the scroll position
		// may be greater than maxValue, get pinned to maxvalue in SclSetScrollBar
		// resulting in the scroll bar and the display being out of sync.
		maxValue = (textHeight - fieldHeight) + FldGetNumberOfBlankLines (fieldP);
	}
	else if (scrollPos)
		maxValue = scrollPos;
	else
		maxValue = 0;

	SclSetScrollBar (bar, (int16_t)scrollPos, 0, (int16_t)maxValue, (int16_t)(fieldHeight - 1));
	if (fieldHeight >= textHeight)
		return scrollFieldNone ;
	if (scrollPos == 0)
		return scrollFieldTop ;
	if (scrollPos + fieldHeight >= textHeight)
		return scrollFieldBottom ;
	return scrollFieldMiddle ;
}


/***********************************************************************
 *
 * FUNCTION:    EditViewLoadRecord
 *
 * DESCRIPTION: This routine loads a memo record into the Edit View form.
 *
 * PARAMETERS:  formP - pointer to the Edit View form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			roger	6/24/99	Fixup MemosInCategory when the record is private
 *			peter	4/25/00	Add support for un-masking just the selected record.
 *
 ***********************************************************************/
static void EditViewLoadRecord(FormType * formP, Boolean redraw)
{
	FieldType *	fieldP;

	PIMAppProfilingBegin("EditViewLoadRecord");

	// Get a pointer to the memo field.
	fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, EditMemoField));

	// Set the font used by the memo field.
	FldSetFont (fieldP, gEditFont);
	FldSetTextColumn(fieldP, gMemoDB, gCurrentRecord, 0, kMemoDBColumnID, 0);
	FldSetScrollPosition (fieldP, gEditScrollPosition);

	// Set the view's title
	if (redraw)
	{
		EditViewSetCounter();

		// Get record categories
		EditViewSetCategoryTriggerLabel(formP);

		// Set the label that contains the note's category.
		EditViewUpdateScrollBar();
	}

	PIMAppProfilingEnd();
}

/***********************************************************************
 *
 * FUNCTION:    EditViewSaveRecord
 *
 * DESCRIPTION: This routine save a memo record to the memo database or
 *				deletes it if it's empty.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			kcr	11/16/95	use DmReleaseRecord to set dirty attribute
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *
 ***********************************************************************/
static void EditViewSaveRecord (void)
{
	char *		ptr;
	Boolean 	empty;
	Boolean		dirty;
	FieldPtr	fieldP;
	FormPtr		formP;
	MemHandle	fieldH;
	
	// Find out if the field has been modified or if it's empty.
	if ((formP = FrmGetFormPtr(EditView)) == NULL)
		return;
	if ((fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex (formP, EditMemoField))) == NULL)
		return;
	dirty	= FldDirty(fieldP);
	ptr		= FldGetTextPtr(fieldP);
	empty	= (ptr == NULL) || (*ptr == 0);
	
	FldReleaseFocus (fieldP);

	// Release any free space in the memo field.
	FldCompactText (fieldP);

	// Clear the handle value in the field, otherwise the handle
	// will be free when the form is disposed of.
	fieldH = FldGetTextHandle(fieldP);
	FldSetTextHandle (fieldP, 0);
	
	// If there's data in an existing record, mark it dirty if
	// necessary and release it.
	if (! empty)
		FldFreeMemory(fieldP);
	
	// If the record is empty, delete it.
	else
	{
		if (dirty)
		{
			DbDeleteRow(gMemoDB, gCurrentRecord);
		}
		else
		{
			DbRemoveRow(gMemoDB, gCurrentRecord);
		}
		DbCursorRequery(gListViewCursorID);
		gCurrentRecord = noRecordSelected;
	}

	if (fieldH)
		MemHandleFree(fieldH);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewChangeFont
 *
 * DESCRIPTION: This routine redisplay the memo in the font specified.
 *              It is called when one of the font push-buttons is presed.
 *
 * PARAMETERS:  controlID - id to button pressed.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *			kcr	10/20/95		handles two-button font change mechanism
 *
 ***********************************************************************/
static void EditViewChangeFont (void)
{
	FontID	fontID;
	FieldPtr fieldP;

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (gEditFont);

	if (fontID != gEditFont)
	{
		gEditFont = fontID;

		// FldSetFont will redraw the field if it is visible.
		fieldP = GetObjectPtr (EditMemoField, EditView);
		FldSetFont (fieldP, fontID);
	}

	EditViewResizeFieldAndScrollBar(FrmGetFormPtr(EditView));
	EditViewUpdateScrollBar ();
	UtilsFrmInvalidateWindow(EditView);
}



/***********************************************************************
 *
 * FUNCTION:    EditViewDeleteRecord
 *
 * DESCRIPTION: This routine deletes a memo record..
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static Boolean EditViewDeleteRecord (void)
{
	FormType *	formP;
	FieldType *	fieldP;
	char *		ptr;
	uint16_t	ctlIndex;
	uint16_t	buttonHit;
	FormPtr		alert;
	Boolean		empty;
	Boolean		saveBackup;


	formP = FrmGetFormPtr (EditView);
	fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, EditMemoField));

	// Find out if the field is empty.
	ptr = FldGetTextPtr (fieldP);
	empty = (ptr == NULL) || (*ptr == 0);

	// Display an alert to confirm the operation.
	if (!empty)
	{
		alert = FrmInitForm (gApplicationDbP, DeleteMemoDialog);

		ctlIndex = FrmGetObjectIndex (alert, DeleteMemoSaveBackup);
		FrmSetControlValue (alert, ctlIndex, gSaveBackup);
		buttonHit = FrmDoDialog (alert);
		saveBackup = (Boolean)FrmGetControlValue (alert, ctlIndex);;

		FrmDeleteForm (alert);

		if (buttonHit == DeleteMemoCancel)
			return (false);

		gSaveBackup = saveBackup;
	}

	// Clear the handle value in the field, otherwise the handle
	// will be free when the form is disposed of.
	FldSetTextHandle (fieldP, 0);

	// Delete or archive the record.
	if (empty && ! FldDirty(fieldP))
	{
		DbRemoveRow(gMemoDB, gCurrentRecord);
		DbCursorRequery(gListViewCursorID);
	}
	else
	{
		uint32_t	numRecords;

		if (gSaveBackup)
			DbArchiveRow(gMemoDB, gCurrentRecord);
		else
			DbDeleteRow(gMemoDB, gCurrentRecord);

		DbCursorRequery(gListViewCursorID);

		numRecords = DbCursorGetRowCount(gListViewCursorID);
	}

	gCurrentRecord = noRecordSelected;

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:		EditViewDoCommand
 *
 * DESCRIPTION:	This routine performs the menu command specified.
 *
 * PARAMETERS:		command  - menu item id
 *
 * RETURNED:    	True if we handled the command.
 *
 *	HISTORY:
 *		03/29/95	art	Created by Art Lamb.
 *		11/07/95	kcr	converted to common about box
 *		08/21/99	kwk	Deleted page top/bottom commands.
 *		11/04/99	jmp	To prevent other sublaunch issues, remind ourselves
 *							that we've sublaunched already into PhoneNumberLookup().
 *
 ***********************************************************************/
static Boolean EditViewDoCommand(uint16_t command)
{
	FieldPtr	fieldP;
	FormType *	formP;
	Boolean		handled = true;
	EventType	newEvent;
	uint32_t	numCategories = 0;
	CategoryID *categoriesP = NULL;
	status_t	err;

	switch (command)
	{
		case NewMemoCmd:
			MenuEraseStatus(0);
			EditViewSaveRecord();
			if (gCurrentRecord != noRecordSelected)
			{
				err = DbGetCategory(gMemoDB, gCurrentRecord, &numCategories, &categoriesP);
				ErrNonFatalDisplayIf(err < errNone, "Can't read category");
				MemoDBCreateRecord(gMemoDB, gListViewCursorID, &gCurrentRecord, numCategories, categoriesP);
				if (categoriesP)
					DbReleaseStorage(gMemoDB, categoriesP);
			}
			else
				MemoDBCreateRecord(gMemoDB, gListViewCursorID, &gCurrentRecord, numCategories, categoriesP);
			gEditScrollPosition = 0;
			if (gCurrentRecord != noRecordSelected)
			{
				EditViewLoadRecord(FrmGetFormPtr(EditView),true);	
				UtilsFrmInvalidateWindow(EditView);
				formP = FrmGetFormPtr(EditView);
				FrmSetFocus(formP, FrmGetObjectIndex(formP, EditMemoField));
			}
			else
				FrmGotoForm(gApplicationDbP, ListView);
			break;

		case DeleteMemoCmd:
			MenuEraseStatus(0);
			if (EditViewDeleteRecord())
				FrmGotoForm(gApplicationDbP, ListView);
			break;

		case BeamMemoCmd:
		case SendMemoCmd:
			MenuEraseStatus(0);
			fieldP = GetObjectPtr (EditMemoField, EditView);
			if (FldGetTextLength(fieldP) > 0)
			{
				EditViewSaveRecord();	// Save the record
				// And reload it so it is displayed while beaming
				formP = FrmGetFormPtr(EditView);
				EditViewLoadRecord (formP,true);
				MemoSendRecord(gMemoDB, gCurrentRecord, (command == BeamMemoCmd ? exgBeamPrefix : exgSendPrefix));
			}
			else
				FrmUIAlert(NoDataToBeamAlert);
			break;

		case AttachMemoCmd:
			MenuEraseStatus(0);
			fieldP = GetObjectPtr (EditMemoField, EditView);
			if (FldGetTextLength(fieldP) > 0)
			{
				MemoSendRecord(gMemoDB, gCurrentRecord, gTransportPrefix);
				// Send quit event to quit application
				MemSet(&newEvent, sizeof(newEvent), 0);
				newEvent.eType = appStopEvent;
				EvtAddEventToQueue(&newEvent);
			}
			else
				FrmUIAlert(NoDataToBeamAlert);
			break;
			
		case EditOptionsFontsCmd:
			MenuEraseStatus(0);
			EditViewChangeFont ();
			handled = true;
			break;

		case EditOptionPhoneLookupCmd:
			MenuEraseStatus(0);
			fieldP = GetObjectPtr (EditMemoField, EditView);
			if (fieldP)
				PhoneNumberLookup (fieldP);
			break;

		case EditOptionsAboutCmd:
			MenuEraseStatus(0);
			AbtShowAbout (sysFileCMemo);
			break;

		default:
			handled = false;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewScroll
 *
 * DESCRIPTION: This routine scrolls the memo edit view a page or a
 *              line at a time.
 *
 * PARAMETERS:  linesToScroll - the number of lines to scroll,
 *						positive for winDown,
 *						negative for winUp
 *					 updateScrollbar - force a scrollbar update?
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant 2/4/99	Use EditViewUpdateScrollBar()
 *
 ***********************************************************************/
static void EditViewScroll (int16_t linesToScroll, Boolean updateScrollbar)
{
	uint32_t				blankLines;
	FieldPtr			fieldP;

	fieldP = GetObjectPtr (EditMemoField, EditView);
	blankLines = FldGetNumberOfBlankLines (fieldP);

	if (linesToScroll < 0)
		FldScrollField (fieldP, (uint16_t)(-linesToScroll), winUp);
	else if (linesToScroll > 0)
		FldScrollField (fieldP, linesToScroll, winDown);

	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	if (blankLines || updateScrollbar)
	{
		ErrNonFatalDisplayIf(blankLines && linesToScroll > 0, "blank lines when scrolling winDown");

		EditViewUpdateScrollBar();
	}
}

/***********************************************************************
 *
 * FUNCTION:    EditViewChangeRecord
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
static void EditViewChangeRecord(int16_t seekDirection)
{
	uint32_t	numRecords;
	uint32_t	recordIndex;
	uint32_t	recordNum;
	uint16_t	attr;
	Boolean		checkPrivacy = true;

	DbCursorRequery(gListViewCursorID);
	numRecords = MemoDBNumVisibleRecordsInCategory(gListViewCursorID);
	DbCursorGetPositionForRowID(gListViewCursorID, gCurrentRecord, &recordIndex);

	if (numRecords <= 1)
		return;

	DbCursorMoveToRowID(gListViewCursorID, gCurrentRecord);
	recordNum = recordIndex;

	EditViewSaveRecord();	
	if (gCurrentRecord == noRecordSelected)
	{	// just deleted an empty record, use same index unless it was the last one
		if (gTitlePos == numRecords) // empty record with alphabetic sort has a faux title index
		{
			--numRecords;
			recordNum = recordIndex = (seekDirection == dmSeekBackward) ? numRecords : 1;
		}
		DbCursorSetAbsolutePosition(gListViewCursorID, recordIndex);
		DbCursorGetRowIDForPosition(gListViewCursorID, recordIndex, &gCurrentRecord);
		EditViewLoadRecord(FrmGetFormPtr(EditView),true);
		return;
	}
	
	DbCursorGetPositionForRowID(gListViewCursorID, gCurrentRecord, &recordIndex);
	recordNum = recordIndex;

	if ((seekDirection == dmSeekBackward && recordIndex == 1)
	||	(seekDirection == dmSeekForward && recordIndex == numRecords))
	{
		recordNum = recordIndex = (seekDirection == dmSeekBackward) ? numRecords : 1;		
		DbCursorSetAbsolutePosition(gListViewCursorID, recordIndex);
		DbCursorGetRowIDForPosition(gListViewCursorID, recordIndex, &gCurrentRecord);
		checkPrivacy = (	DbGetRowAttr(gMemoDB, gListViewCursorID, &attr) == errNone 
						&&	(attr & dbRecAttrSecret)
						&&	gPrivateRecordVisualStatus == maskPrivateRecords);
	}
	EditViewLoadRecord(FrmGetFormPtr(EditView),true);

	//while to skip masked records. Even if the body never executes, we'll have done a DbFindRecordByOffsetInCategory
	while (	checkPrivacy &&
			!DbCursorMove(gListViewCursorID, seekDirection == dmSeekForward ? 1 : -1, dbFetchRelative) &&
			DbGetRowAttr(gMemoDB, gListViewCursorID, &attr) == errNone &&
			((attr & dbRecAttrSecret) && gPrivateRecordVisualStatus == maskPrivateRecords))
	{
	}

	DbCursorGetCurrentPosition(gListViewCursorID, &recordIndex);
	if (recordNum == recordIndex)
		return;

	// Don't show first/last record if it's private and we're masking.
	if (!DbGetRowAttr(gMemoDB, gListViewCursorID, &attr) &&
		((attr & dbRecAttrSecret) && gPrivateRecordVisualStatus == maskPrivateRecords))
		return;
	
	SndPlaySystemSound (sndInfo);

	// Saving the current record may cause it to move if the records are
	// sorted alphabeticly.
	DbCursorGetRowIDForPosition(gListViewCursorID, recordIndex, &gCurrentRecord);
	EditViewSaveRecord();	
	EditViewLoadRecord(FrmGetFormPtr(EditView),true);
}

/***********************************************************************
 *
 * FUNCTION:    EditViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the message a page winUp or winDown.
 *					 When the top of a memo is visible, scrolling up will
 *              display the display the botton of the previous memo.
 *              If the bottom of a memo is visible, scrolling down will
 *              display the top of the next memo.
 *
 * PARAMETERS:   direction     winUp or winDown
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant 2/4/99	Use EditViewScroll() to do actual scrolling.
 *
 ***********************************************************************/
static Boolean EditViewPageScroll(WinDirectionType direction)
{
	int16_t		linesToScroll;
	FieldPtr	fieldP;
		
	fieldP = GetObjectPtr (EditMemoField, EditView);
	
	if (FldScrollable (fieldP, direction))
	{
		linesToScroll = (int16_t)(FldGetVisibleLines(fieldP) - 1);

		if (direction == winUp)
			linesToScroll = -linesToScroll;

		EditViewScroll(linesToScroll, true);
		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    EditViewExit
 *
 * DESCRIPTION: This routine is call when the Edit View is exited.  It
 *              releases any memory allocated for the Edit View.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/17/95		Initial Revision
 *
 ***********************************************************************/
static void EditViewExit (void)
{
	UIColorSetTableEntry( UIFieldBackground, &gOriginalFieldBackgroundColor );
}


/***********************************************************************
 *
 * FUNCTION:    EditViewResize
 *
 * DESCRIPTION: This routine resize the list view fomr and its objects
 *				when the input area appears or disappears
 *
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		1/18/02		Initial Revision
 *
 ************************************************************************/
static void EditViewResizeFieldAndScrollBar(FormType *formP)
{	
	RectangleType 	objBounds,
					fieldBounds;
	uint16_t		objIndex;
	FieldType		*fieldP;
	FontID			currFont;
	Coord			fontHeight, noteDoneButtonY, x;

	// Get NoteField size and then adjust scroll bar to the same size
	fieldP =(FieldType*)FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, EditMemoField));
	FldGetBounds(fieldP, &fieldBounds);

	// Get Done button position
	FrmGetObjectPosition(formP, FrmGetObjectIndex(formP, EditDoneButton), 
		&x, &noteDoneButtonY);

	// Get font height
	currFont = FntSetFont (gEditFont);
	fontHeight = FntLineHeight();
	FntSetFont (currFont);

	// Remove the last incomplete line
	fieldBounds.extent.y = noteDoneButtonY - fieldBounds.topLeft.y - kSpaceBetweenFieldAndDoneButton;
	fieldBounds.extent.y -= fieldBounds.extent.y % fontHeight;

	// Resize the field
	FldSetBounds(fieldP, &fieldBounds);
	FldRecalculateField(fieldP, false);
	
	// NoteScrollBar and NoteField must have the same height
	objIndex = FrmGetObjectIndex(formP, EditMemoScrollBar);
	FrmGetObjectBounds(formP, objIndex, &objBounds);
	objBounds.topLeft.y = fieldBounds.topLeft.y;
	objBounds.extent.y = fieldBounds.extent.y;
	FrmSetObjectBounds(formP, objIndex, &objBounds);
}

/***********************************************************************
 *
 * FUNCTION:    EditViewResize
 *
 * DESCRIPTION: This routine resize the list view fomr and its objects
 *				when the input area appears or disappears
 *
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			TEs		1/18/02		Initial Revision.
 *			PPL 	05/04/04	Finalize Landcsape Mode.
 *
 ************************************************************************/
static void EditViewResize(FormType *formP, RectangleType *newBoundsP)
{
	uint16_t		objIndex;
	Coord			offsetX;
	Coord			offsetY;
	Coord			x;
	Coord			y;
	RectangleType	rect;
	Coord			width;
	Coord			height;
	FieldType*		fieldP;
	size_t			startSel;
	size_t			endSel;

	// get the new window extent
	width = newBoundsP->extent.x;
	height = newBoundsP->extent.y;
	
	// get the delta between the old window bounding rect and the new display extent.
	offsetX = width  - gCurrentWinBounds.extent.x;
	offsetY = height - gCurrentWinBounds.extent.y;

	if (!offsetX && !offsetY)
		return ;

	if (offsetY)
	{
		// EditDetailsButton, moves only on Y
		objIndex = FrmGetObjectIndex(formP, EditDetailsButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (formP, objIndex, x, y);


		// EditDoneButton, moves only on Y
		objIndex = FrmGetObjectIndex(formP, EditDoneButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (formP, objIndex, x, y);


		// EditAttachButton, moves only on Y
		objIndex = FrmGetObjectIndex(formP, EditAttachButton);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		y += offsetY;
		FrmSetObjectPosition (formP, objIndex, x, y);
	}


	// EditMemoField
	fieldP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, EditMemoField));
	FldGetSelection(fieldP, &startSel, &endSel);

	
	// EditMemoField and EditMemoScrollBar are both vertically resized (y)
	EditViewResizeFieldAndScrollBar(formP);


	if (offsetX)
	{
		// EditMemoField is resived on X
		FldGetBounds(fieldP, &rect);
		rect.extent.x += offsetX;
		FldSetBounds(fieldP, &rect);


		//  EditMemoScrollBar, moves on X
		objIndex = FrmGetObjectIndex(formP, EditMemoScrollBar);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (formP, objIndex, x, y);


		//  EditCategoryTrigger, moves on X
		objIndex = FrmGetObjectIndex(formP, EditCategoryTrigger);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (formP, objIndex, x, y);
		EditViewSetCategoryTriggerLabel(formP);


		// EditCategoryList, moves on X
		objIndex = FrmGetObjectIndex(formP, EditCategoryList);
		FrmGetObjectPosition (formP, objIndex, &x, &y);
		x += offsetX;
		FrmSetObjectPosition (formP, objIndex, x, y);
	}
	
	// Restore selection unless we just opened the form
	if (FrmGetFocus (formP) != noFocus && !sInitialResizeEvent)
	{
		FldSetScrollPosition(fieldP, startSel);
		FldSetSelection(fieldP, startSel, endSel);
	}


	// Edit scrollbar cursor position	
	EditViewUpdateScrollBar();

	
	// The Graffiti Shift Indicator, moves on both x and y
	objIndex = FrmGetNumberOfObjects(formP);
	while (objIndex--)
	{
		if (FrmGetObjectType(formP, objIndex) == frmGraffitiStateObj)
		{
			FrmGetObjectPosition (formP, objIndex, &x, &y);
			x += offsetX;
			y += offsetY;
			FrmSetObjectPosition (formP, objIndex, x, y);
			break;
		}
	}
	
	// keep the window bounding rect.
	gCurrentWinBounds = *newBoundsP;
} 

/***********************************************************************
 *
 * FUNCTION:    EditViewInit
 *
 * DESCRIPTION: This routine initials the Edit View form.
 *
 * PARAMETERS:  formP - pointer to the Edit View form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		3/31/95		Initial Revision
 *			art		7/1/96		Set field hasScrollBar attribute
 *			FPa		11/30/96	Set title to 0
 *
 ***********************************************************************/
static void EditViewInit(FormType * formP)
{
	FieldType *		fieldP;
	FieldAttrType	attr;
	
	// Make the edit form transparent.
	FrmSetTransparentObjects( formP, TRUE );
	
	FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, EditViewBackground),
							  PrvBackgroundGadgetHandler);

	// Have the field send event to maintain the scroll bar.
	fieldP = GetObjectPtr (EditMemoField, EditView);
	FldGetAttributes (fieldP, &attr);
	attr.hasScrollBar = true;
	FldSetAttributes (fieldP, &attr);

	EditViewResizeFieldAndScrollBar(FrmGetFormPtr(EditView));
	
	EditViewLoadRecord(formP,true);
    
	// Were we requested to add an attachment?
	// if yes, we show the attach button.
	// if not we hide the attach button
	
	if (gAttachRequest)
	{
		// Show Attach Button
		FrmShowObject(formP,FrmGetObjectIndex(formP, EditAttachButton));
	}
	else
	{
		// Hide attach button
		FrmHideObject(formP, FrmGetObjectIndex(formP, EditAttachButton));
	}

	gCurrentView = EditView;

	// Init selection status
	sEditPosition = kEndOfFieldCursorPostion;
	sEditSelectionLength = 0;
	// Don't set selection at the initial winResize
	sInitialResizeEvent = true;

	FrmSetFocus(formP, FrmGetObjectIndex(formP, EditMemoField));
	gEditNavFocus = true;

	FrmSetNavEntry(formP, EditMemoField, EditCategoryTrigger, EditCategoryTrigger, EditDoneButton, 0);

	EditViewSetCategoryTriggerLabel(formP);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewGotoEvent
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  eventP
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			PLe		06/16/03	Initial Revision
 *
 ***********************************************************************/
static void EditViewGotoEvent(EventPtr eventP)
{
	FormPtr	formP;
	FieldPtr fieldP;
	
	formP = FrmGetFormPtr(EditView);
	fieldP = GetObjectPtr (EditMemoField, EditView);
	gCurrentRecord = eventP->data.frmGoto.recordID;
	EditViewInit(formP);
	sEditPosition = eventP->data.frmGoto.matchPos;
	sEditSelectionLength = eventP->data.frmGoto.matchLen;
}


/***********************************************************************
 *
 * FUNCTION:    EditViewRestoreEditState
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  eventP
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			PLe		06/16/03	Initial Revision
 *
 ***********************************************************************/
static void EditViewRestoreEditState(void)
{
	FormPtr	formP;
	FieldPtr fieldP;

	formP = FrmGetFormPtr(EditView);

	fieldP = GetObjectPtr(EditMemoField, EditView);
	FrmSetFocus(formP, FrmGetObjectIndex(formP, EditMemoField)); 
	if (sEditSelectionLength > 0)
	{
		// Scroll to correct position
		FldSetScrollPosition(fieldP, sEditPosition);
		// Update scroll bar to the correct position
		EditViewUpdateScrollBar();
		// Set the selection
		FldSetSelection(fieldP, sEditPosition, sEditPosition + sEditSelectionLength);	
	}
	else
		FldSetInsertionPoint(fieldP, sEditPosition);				
}

	
/***********************************************************************
 *
 * FUNCTION:    EditViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Edit View"
 *              of the Memo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	02/21/95	Initial Revision.
 *			kwk	10/04/98	Explicitly disable Graffiti auto-shift if we get
 *								a printable key-down event.
 *			kwk	11/21/98	Handle cmd keys in separate code block, so TxtCharIsPrint
 *								doesn't get called w/virtual key codes.
 *			gap	10/27/00	change the command bar initialization to allow field
 *								code to add cut, copy, paste, & undo commands as
 *								appropriate rather than adding a fixed set of selections.
 *			CS		11/28/00	Call EditViewExit in response to frmCloseEvent.
 *
 ***********************************************************************/
Boolean EditViewHandleEvent (EventType * event)
{
	static FormType * formP = NULL;
	EventType	newEvent;
	Boolean		handled = false;
	FieldType *	fieldP;
	uint32_t	numLibs;

	switch (event->eType)
	{
		case frmObjectFocusTakeEvent:
			gEditNavFocus = (event->data.frmObjectFocusTake.formID == EditView 
							&& event->data.frmObjectFocusTake.objectID == EditMemoField);
			gEditing = false;
			break;

		case keyDownEvent:
			if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{
				EditViewSaveRecord ();
				FrmGotoForm (gApplicationDbP, ListView);
				handled = true;
			}
			
			else if (EvtKeydownIsVirtual(event))
			{
				switch (event->data.keyDown.chr)
				{
				case vchrRockerUp:
					if (!gEditNavFocus || gEditing)
						break;
				case vchrPageUp:
					if ((handled = EditViewPageScroll(winUp)) != false)
					{
						formP = FrmGetFormPtr(EditView);
						FrmSetFocus(formP, FrmGetObjectIndex(formP, EditMemoField));
					}
					else
						handled = (event->data.keyDown.modifiers & autoRepeatKeyMask) ;
					break;

				case vchrRockerDown:
					if (!gEditNavFocus || gEditing)
						break;
				case vchrPageDown:
					if ((handled = EditViewPageScroll(winDown)) != false)
					{
						formP = FrmGetFormPtr(EditView);
						FrmSetFocus(formP, FrmGetObjectIndex(formP, EditMemoField));
					}
					else
						handled = (event->data.keyDown.modifiers & autoRepeatKeyMask) ;
					break;

				case vchrRockerCenter:
					if (!gEditNavFocus || gEditing)
					{	
						gEditing = false;
						break;
					}
					FrmClearFocusHighlight();
					fieldP = GetObjectPtr(EditMemoField, EditView);
					switch (EditViewUpdateScrollBar())
					{
						case scrollFieldTop:
						case scrollFieldNone:
							FldSetInsertionPoint(fieldP, 0);
						break;

						case scrollFieldBottom:
							FldSetInsertionPoint(fieldP, FldGetTextLength(fieldP));
						break;

						case scrollFieldMiddle:
							FldSetInsertionPoint(fieldP, FldGetScrollPosition(fieldP) + 1);
						break;
					}
					gEditing = true ;
					break;

				// Send data key presed?
				case vchrSendData:
					if (FldGetTextLength(GetObjectPtr (EditMemoField, EditView)) > 0)
					{
						EditViewSaveRecord();
						MemoSendRecord(gMemoDB, gCurrentRecord, exgBeamPrefix);

						// Redisplay the record.  If the IR loopback mechanism sends the
						// record to this app the goto action code closes all forms and
						// send a frmGotoEvent.  Load the record again only if the form
						// still exits.
						if (formP != NULL)
							EditViewLoadRecord(formP,false);
					}
					else
						FrmUIAlert(NoDataToBeamAlert);
					handled = true;
					break;
				}
			}
			break;

		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case EditCategoryTrigger:
					EditViewSelectCategory ();
					handled = true;
					break;

				case EditDoneButton:
					EditViewSaveRecord ();
					FrmGotoForm (gApplicationDbP, ListView);
					handled = true;
					break;

				case EditDetailsButton:
					FrmPopupForm (gApplicationDbP, DetailsDialog);
					handled = true;
					break;
					
				case EditAttachButton:
					if (gAttachRequest)
					{
						// Send record vcard
						MemoSendRecord(gMemoDB, gCurrentRecord, gTransportPrefix);

						// Send quit event to quit application
						MemSet(&newEvent, sizeof(newEvent), 0);
						newEvent.eType = appStopEvent;
						EvtAddEventToQueue(&newEvent);
					}
					handled = true;
					break;
			}
			break;

		case ctlRepeatEvent:
			switch (event->data.ctlRepeat.controlID)
			{
			case EditLeftButton:
				gEditScrollPosition = 0;
				EditViewChangeRecord(dmSeekBackward);
				break;

			case EditRightButton:
				gEditScrollPosition = 0;
				EditViewChangeRecord(dmSeekForward);
				break;
			}
			break;
	
		case menuOpenEvent:
			if (! gAttachRequest)
			{
				if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
					MenuHideItem(SendMemoCmd);
				else
					MenuShowItem(SendMemoCmd);
			}
			else
			{
				// Hide send & Beam commands
				MenuHideItem(SendMemoCmd);
				MenuHideItem(BeamMemoCmd);

				// Show attach command			
				MenuShowItem(AttachMemoCmd);
			}
			// don't set handled = true
			break;

		case menuEvent:
			handled = EditViewDoCommand (event->data.menu.itemID);
			break;

		// Add the buttons that we want available on the command bar, based on the current context
		case menuCmdBarOpenEvent:
			{
				size_t startPos, endPos;

				fieldP = GetObjectPtr (EditMemoField, EditView);
				FldGetSelection(fieldP, &startPos, &endPos);

				if (startPos == endPos)  // there's no highlighted text
				{
					DmOpenRef	uiLibDBP = NULL;
					uint32_t	uiLibRefNum = 0;
                    
					// Load UILIb to get its module DB
					SysLoadModule(sysFileTLibrary, sysFileCUI, 0, 0, &uiLibRefNum);
					SysGetModuleDatabase(uiLibRefNum, NULL, &uiLibDBP);
                    
					// Call directly Field event handler so that System Edit buttons are added
					FldHandleEvent(fieldP, event);

					// Beam on the left
					MenuCmdBarAddButton(menuCmdBarOnLeft, uiLibDBP, BarBeamBitmap, menuCmdBarResultMenuItem, BeamMemoCmd, 0);

					// Delete on the Right
					MenuCmdBarAddButton(menuCmdBarOnRight, uiLibDBP, BarDeleteBitmap, menuCmdBarResultMenuItem, DeleteMemoCmd, 0);

					// Unload UILib
					SysUnloadModule(uiLibRefNum);

					// Prevent the field package to automatically add cut, copy, paste, and undo buttons as applicable
					// since it was done previously
					event->data.menuCmdBarOpen.preventFieldButtons = true;
				}

				// don't set handled to true; this event must fall through to the system.
				break;

		case frmOpenEvent:
			formP = FrmGetFormPtr(EditView);
			EditViewInit(formP);
			EditViewRestoreEditState();
			handled = true;
			break;

		case frmGotoEvent:
			formP = FrmGetFormPtr(EditView);
			EditViewGotoEvent(event);
			EditViewRestoreEditState();
			handled = true;
			break;

		case winFocusGainedEvent:
			// Post a key down event if the memo was created by a keyDown event in the ListView
			if (gEnqueueNewChar && event->data.winFocusGained.window == FrmGetWindowHandle(formP))
			{
				size_t i;
				//TraceOutput(TL(appErrorClass, "Fake evtQueue (EditView) insert \"%s\" into field", gNewCharBuffer));
				for (i = 0; i < gNewCharBufferCount; i++)
				{
					EvtAddEventToQueue(&gNewCharBuffer[i]);
				}
				gEnqueueNewChar = false;
			}
			
			break;

		case winUpdateEvent:
			if (event->data.winUpdate.window != FrmGetWindowHandle(formP))
				break;
			FrmDrawForm(formP);
			handled = true;
			break;

		case winResizedEvent:
			if (event->data.winResized.window != FrmGetWindowHandle(formP))
				break;
			EditViewResize(FrmGetFormPtr(EditView), &event->data.winResized.newBounds);
			sInitialResizeEvent = false;

			// Let the default form handler also handle this event.
			// It needs to move the background gadget.
			//handled = true;
			break;

		case fldChangedEvent:
			EditViewUpdateScrollBar();
			handled = true;
			break;

		case frmSaveEvent:
			EditViewSaveRecord (); // This deletes empty memos.
			break;

		case frmCloseEvent:
			EditViewExit();
			break;

		case sclRepeatEvent:
			EditViewScroll ((int16_t)(event->data.sclRepeat.newValue -
							event->data.sclRepeat.value), false);
			break;

		// Sent by the DetailsDialog when categories have changed
		case kCategoriesChangedEvent:
			{
				EditViewSetCategoryTriggerLabel(FrmGetFormPtr(EditView));

				// Set the title of the edit view, unless current category is 'all'
				if (!gCurrentCategoriesP || *gCurrentCategoriesP != catIDAll)
					EditViewSetCounter();
                
				// Update form
				UtilsFrmInvalidateWindow(EditView);
			}
			break;
		}
		break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    EditViewInitLayout
 *
 * DESCRIPTION: Initialize the form..
 *              
 *
 * PARAMETERS:  formP = pointer to the form.
 *
 * RETURNED:    void
 *
 ***********************************************************************/
void EditViewInitLayout(FormPtr formP)
{
	FrmInitLayout(formP, formLayout);

	// Change the color of the lines drawn by the field.
	UIColorSetTableEntry(UIFieldTextLines, &gEditFieldLineColor );

	// The FEP causes some trouble when used over the yellow background.
	// I want the FEP to draw raw characters with underlines.  In order
	// for tha to happen, the raw text color, foreground & background,
	// needs to match the regular character colors.  The text is still
	// black.  I just changed the background.  I need to change the fep
	// raw text background to match.


	UIColorGetTableEntryRGB( UIFieldBackground, &gOriginalFieldBackgroundColor );
	UIColorSetTableEntry(UIFieldBackground, &gEditBackgroundColor);
	UIColorSetTableEntry(UIFieldFepRawBackground, &gEditBackgroundColor);
}

/***********************************************************************
 *
 * FUNCTION:    PrvBackgroundGadgetHandler
 *
 * DESCRIPTION: The event handler for the gadget handler.  Draws
 * 				 the visual effects that add to the ecroll bar.
 *              
 *
 * PARAMETERS:  gadgetP = pointer to the gadget
 *					 cmd - the gadget handler command.  Indicates what the
 *					 gadget handler must do
 *					 paramP = getget handler parameter
 *
 * RETURNED:    void
 *
 ***********************************************************************/
Boolean PrvBackgroundGadgetHandler(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP)
{
	if( cmd == formGadgetDrawCmd )
	{
		FormPtr frmP = NULL;
		RectangleType fieldRect;
		GcHandle gc = NULL; 
		FieldType *fldP = NULL;
		GcPointType fldTopLeft;
		GcPointType fldExtent;
		Coord startY;
		Coord lineHeight;
		uint32_t numberOfLines;
		
		//
		// Fill the bounds of the gadget with the yellowish paper color
		//

		RGBColorType		prevColor;

		WinSetForeColorRGB( &gEditBackgroundColor, &prevColor );
		WinDrawRectangle( &gadgetP->rect, 0 );
		WinSetForeColorRGB( &prevColor, NULL );


		// Get the bounds of the field
		frmP = FrmGetActiveForm();
		FrmGetObjectBounds( frmP, FrmGetObjectIndex(frmP, EditMemoField), &fieldRect );
		fldP = FrmGetObjectPtr( frmP, FrmGetObjectIndex(frmP, EditMemoField) );
		
		// Get the lineHeight, num lines info before converting to native coordinates.
		lineHeight = fieldRect.extent.y / (Coord)FldGetVisibleLines(fldP);
		numberOfLines = (uint32_t)(fieldRect.extent.y / lineHeight);
		
		// Convert the rect to native coordinates.  All subsequent operations
		// are done in native coordinates.
		WinSetCoordinateSystem(kCoordinatesNative);
		startY = fieldRect.topLeft.y;
		WinScaleRectangle( &fieldRect );
		fldTopLeft.x = (fcoord_t)fieldRect.topLeft.x;
		fldTopLeft.y = (fcoord_t)fieldRect.topLeft.y;
		fldExtent.x = (fcoord_t)fieldRect.extent.x;
		fldExtent.y = (fcoord_t)fieldRect.extent.y;

		gc = GcGetCurrentContext();
		GcPushState(gc);
		
		GcSetCoordinateSystem(gc, kCoordinatesNative);
				
		// Adjust the pen size
		GcSetPenSize(gc,1.0);

		// Make the 'red' color the current color
		// in preparation for drawing the long red vertical
		// lines on the left hand side of the edit screen.
		GcSetColor(
			gc,
			gEditSideLineColor.r,
			gEditSideLineColor.g,
			gEditSideLineColor.b,
			/*alpha*/ 255);
		
		// Draw two long vertical red lines on the left hand side
		// of the form field.  Use the position of the form field
		// to determine the positions of the red lines.
		GcMoveTo(gc, fldTopLeft.x-6.0f, fldTopLeft.y-1.0f );
		GcLineTo(gc, fldTopLeft.x-6.0f, fldTopLeft.y-1.0f + fldExtent.y - 1.0f );
		GcStroke(gc);
		GcMoveTo(gc, fldTopLeft.x-10.0f, fldTopLeft.y-1.0f );
		GcLineTo(gc, fldTopLeft.x-10.0f, fldTopLeft.y-1.0f + fldExtent.y - 1.0f );
		GcStroke(gc);
		GcPaint(gc);
		
		// Draw in extensions for the green lines in the form field.
		// The form field lines terminate at the edge of the form field.  We
		// new to manually extend them to the edge of the screen.
		{
			Coord yPos;
			uint32_t lineIndex;
			
			yPos = lineHeight + startY;

			GcSetColor(
				gc,
				gEditFieldLineColor.r,
				gEditFieldLineColor.g,
				gEditFieldLineColor.b,
				255 /*alpha*/);

			for( lineIndex = 0; lineIndex < numberOfLines; lineIndex++ )
			{
				fcoord_t yPosN = (fcoord_t)WinScaleCoord(yPos, true);
				GcRect(gc, fldTopLeft.x, yPosN - 2, 0.0f, yPosN - 1);
				yPos += lineHeight;
			}
		}
		
		GcPaint(gc);
		
		GcPopState(gc);

		GcReleaseContext(gc);

		WinSetCoordinateSystem(kCoordinatesStandard);
	}
	
	
	return true;
}
