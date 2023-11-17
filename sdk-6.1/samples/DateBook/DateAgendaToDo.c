/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateAgendaToDo.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  Manage the ToDo records in agenda view.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <DateTime.h>
#include <ErrorMgr.h>
#include <Loader.h>
#include <ScrollBar.h>
#include <SystemMgr.h>
#include <SystemResources.h>
#include <Table.h>
#include <TimeMgr.h>
#include <Find.h>

#include "ToDoExports.h"

#include "DateGlobals.h"

#include "DateAgenda.h"
#include "DateAgendaToDo.h"

#include "Datebook.h"
#include "DatebookRsc.h"





/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
ToDoSharedDataType ToDoSharedData = {
	AgendaView,
	AgendaToDoTable,
	AgendaToDoCategoryList,
	AgendaToDoCategoryTrigger,
	{0, 0, 0},  /* today */
	0,	/* list font */
	0	/* padding */
};


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewInitToDo
 *
 * DESCRIPTION: 	Initialize the To Do portion of the Agenda view.
 *
 * PARAMETERS:  ->	frm:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
void ToDoViewInit (FormPtr frm )
{

    DateToDoInitializeFunc *	toDoInitializeFuncP = NULL;
    status_t 					err;

	if (ToDoRefNum == kRALInvalidRefNum)
		return ;

    err = SysGetEntryAddresses(ToDoRefNum, DateToDoInitializeId, 1, (void**)&toDoInitializeFuncP);
	ErrNonFatalDisplayIf(err < errNone, "Unable to load DateToDoLoadPrefsFunc");

	// Call the ToDo function
	ToDoSharedData.listFont = ApptDescFont;
	ToDoSharedData.today = Date;
	(*toDoInitializeFuncP)(&ToDoSharedData);
}


/***********************************************************************
 *
 * FUNCTION:    	ToDoViewFinalize
 *
 * DESCRIPTION: 	Finalize the Agenda view of the Datebook application.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	10/10/02	Initial revision.
 *
 ***********************************************************************/
void ToDoViewFinalize (void)
{
    DateToDoFinalizeFunc *toDoFinalizeFuncP = NULL;

	if (ToDoRefNum == kRALInvalidRefNum)
		return ;

    SysGetEntryAddresses(ToDoRefNum, DateToDoFinalizeId, 1, (void**)&toDoFinalizeFuncP);

	// Call the ToDo function
	(*toDoFinalizeFuncP)();
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewInitToDo
 *
 * DESCRIPTION: 	Initialize the To Do portion of the Agenda view.
 *
 * PARAMETERS:  ->	frm - pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
void AgendaViewInitToDo (FormPtr frm )
{
    DateToDoListViewInitFunc *toDoListViewInitFuncP = NULL;

	if (ToDoRefNum == kRALInvalidRefNum)
		return ;

	SysGetEntryAddresses(ToDoRefNum, DateToDoListViewInitId, 1, (void**)&toDoListViewInitFuncP);

	// Call the ToDo function
	(*toDoListViewInitFuncP)(frm);
}


/***********************************************************************
 *
 * FUNCTION:    	ToDoCountRecordsOnCurrentDate
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
uint16_t ToDoCountRecordsOnCurrentDate (void)
{
    DateToDoCountRecordsWithNewDateFunc *toDoCountRecordsWithNewDateFuncP = NULL;

	if (ToDoRefNum == kRALInvalidRefNum)
		return 0;

    SysGetEntryAddresses(ToDoRefNum, DateToDoCountRecordsWithNewDateId, 1,
    	(void**)&toDoCountRecordsWithNewDateFuncP);

	// Call the ToDo function
	return (*toDoCountRecordsWithNewDateFuncP)(Date);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaLoadTasks
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
void AgendaViewFillTasks (void)
{
    DateToDoFillViewFunc *toDoFillViewFuncP = NULL;

	if (ToDoRefNum == kRALInvalidRefNum)
		return ;

    SysGetEntryAddresses(ToDoRefNum, DateToDoFillViewId, 1, (void**)&toDoFillViewFuncP);

	// Call the ToDo function
	(*toDoFillViewFuncP)();
}


/***********************************************************************
 *
 * FUNCTION:    	ToDoListViewDrawTable
 *
 * DESCRIPTION: 	Updates the entire list view, such as when changing categories
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	4/14/99		Initial revision.
 *	ppl 10/07/03	use invalidate mechanism.
 *					AgendaViewDisplayInvalidate().
 *
 ***********************************************************************/
void ToDoListViewDrawTable (void)
{
	FormType*		formP;

	formP = FrmGetFormPtr (AgendaView);

	AgendaUpdateToDoCount ();
	AgendaViewRefreshLayoutTasks (formP);

	AgendaViewDisplayInvalidate( formP, NULL);

/*
	TblRedrawTable (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, AgendaDatebookTable)));
	AgendaViewDrawSeparator (formP);
	TblRedrawTable (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, AgendaToDoTable)));
*/
}


/***********************************************************************
 *
 * FUNCTION:    	ToDoListViewSelectCategory
 *
 * DESCRIPTION: 	This routine handles selection, creation and
 *					deletion of categories in the List View.
 *
 * PARAMETERS:  ->	reOpenCursor: do we re open the cursor.
 *
 * RETURNED:    	The index of the new category.
 *
 *              	The following global variables are modified:
 *							CurrentCategory
 *							ShowAllCategories
 *							CategoryName
 *
 * NOTE:			Closing the cursor can be required when we changed
 *					the categories but not the date.
 *
 *					When we change categories we have to reopen the
 *					cursor currently we do not have an api to explicitly
 *					requery with categories.
 *
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
Boolean ToDoListViewSelectCategory ()
{
	Boolean 								categoriesUpdated;
	FormType*								formP;
    DateToDoListViewInitFunc *				toDoListViewInitFuncP = NULL;
    ListViewSelectCategoryFunc *			toDoListViewSelectCategoryFuncP = NULL;
	DateToDoShowCategoriesStatusFunc * 		toDoListShowCategoriesStatusFuncP = NULL;

	if (ToDoRefNum == kRALInvalidRefNum)
		return 0;

    SysGetEntryAddresses(ToDoRefNum, DateToDoListViewSelectCategoryFuncId,
    	1, (void**)&toDoListViewSelectCategoryFuncP);

	SysGetEntryAddresses(ToDoRefNum, DateToDoListViewInitId, 1, (void**)&toDoListViewInitFuncP);

	SysGetEntryAddresses(ToDoRefNum, DateToDoShowCategoriesStatusId, 1,
		(void**)&toDoListShowCategoriesStatusFuncP);

	// Call the ToDo function
	categoriesUpdated = (*toDoListViewSelectCategoryFuncP)();

	// If the categories selection have been updated, resize and redraw
	if (categoriesUpdated)
	{
		formP = FrmGetFormPtr (AgendaView);

		// Call the ToDo initialize function to update the category column
		// if categories were displayed
		if ((*toDoListShowCategoriesStatusFuncP)())
			(*toDoListViewInitFuncP)(formP);

		AgendaViewSetScrollThumb (formP, AgendaToDoScroller, 0);
		ToDoListViewDrawTable ();
	}

	return categoriesUpdated;
}


/***********************************************************************
 *
 * FUNCTION:    	ToDoListViewScroll
 *
 * DESCRIPTION: 	This routine scrolls the list of ToDo items
 *             	 	in the direction specified.
 *
 * PARAMETERS:  ->	direction:	winUp or dowm
 *              ->	oneLine:	if true the list is scrolled by a single line,
 *                          	if false the list is scrolled by a full screen.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	2/21/95		Initial revision.
 *
 ***********************************************************************/
Boolean ToDoListViewScroll (int16_t delta)
{
	FormType*			formP;
	TablePtr 			table;
	int32_t				seekDirection;
	uint32_t				count;
    DateToDoScrollRecordsFunc *toDoScrollRecordsFuncP = NULL;
	Boolean				scrolled;

	if ((delta == 0) || (ToDoRefNum == kRALInvalidRefNum))
		return false;

	formP = FrmGetActiveForm ();
	table = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, AgendaToDoTable));
	TblReleaseFocus (table);

	seekDirection = ( delta > 0 ? dmSeekForward : dmSeekBackward );
	count = ( delta > 0 ? delta : ((uint32_t) -delta) );

	// Find the new top visible record
    SysGetEntryAddresses(ToDoRefNum, DateToDoScrollRecordsId, 1, (void**)&toDoScrollRecordsFuncP);

	// Call the ToDo scroll function
	scrolled = (*toDoScrollRecordsFuncP)(count, seekDirection);

	if (scrolled)
		ToDoListViewDrawTable();

	return scrolled;
}


/***********************************************************************
 *
 * FUNCTION:    	ToDoListViewChangeCompleteStatus
 *
 * DESCRIPTION: 	This routine is called when a ToDo item is marked
 *              	complete.  If completed items are not displayed
 *              	(a preference setting),  this routine will remove
 *					the item from the list.
 *
 * PARAMETERS:  ->	row:		row in the table.
 *              ->	complete:	true if the item is marked complete.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	5/1/95		Initial revision.
 *
 ***********************************************************************/
void ToDoListViewChangeCompleteStatus (int16_t row, uint16_t complete)
{
	FormType*		formP;
    ListViewChangeCompleteStatusFunc *toDoListViewChangeCompleteStatusFuncP = NULL;

	if (ToDoRefNum == kRALInvalidRefNum)
		return ;

    SysGetEntryAddresses(ToDoRefNum, DateToDoUpdateCompleteStatusFuncId,
    	1, (void**)&toDoListViewChangeCompleteStatusFuncP);

	// Call the ToDo function
	(*toDoListViewChangeCompleteStatusFuncP)(row, complete);

	// Update ToDo scrollbar if needed.
	formP = FrmGetFormPtr (AgendaView);
	AgendaUpdateToDoCount ();
	AgendaViewRefreshLayoutTasks (formP);
}

/***********************************************************************
 *
 * FUNCTION:    	ToDoListViewResizeTableColumns
 *
 * DESCRIPTION: 	resizes task table column size.
 *					Need to do knowledge of shown columns.
 *
 * PARAMETERS:  ->	tableP:		the table.
 *              ->	offsetX:	horizontal offset when resized.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	05/03/04	Initial revision.
 *	PPL	05/04/04	Add Lanscape Mode;
 *
 ***********************************************************************/
void ToDoListViewResizeTableColumns(TableType* tableP, Coord offsetX)
{
    DateToDoListViewResizeTableColumnsFunc *toDoListViewResizeTableColumnsFuncP = NULL;

	if (tableP == NULL)
		return;

	// Find the new top visible record
    SysGetEntryAddresses(ToDoRefNum, DateToDoListViewResizeTableColumnsFuncId, 1, (void**)&toDoListViewResizeTableColumnsFuncP);

	// Call the ToDo scroll function
	(*toDoListViewResizeTableColumnsFuncP)(tableP, offsetX);
}


/***********************************************************************
 *
 * FUNCTION:    	LaunchToDoWithRecord
 *
 * DESCRIPTION: 	Switch to the To Do application and display the
 *					given record. If there are any problems retrieving
 *					the record, launch the app without it.
 *
 * PARAMETERS:  ->	inRowID: a pointer to an EventType structure.
 *
 * RETURNED:    	true if the event was handled and should not be
 *					passed to a higher level handler.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ppl	07/15/03	cursors !
 *
 ***********************************************************************/
void LaunchToDoWithRecord (uint32_t inRowID)
{
	GoToParamsType*		gotoParamsP;
	uint16_t			launchCmd 		= sysAppLaunchCmdNormalLaunch;
	MemPtr				launchParamsP = NULL;

	if (inRowID != dbInvalidRowID)
	{
		// Create the param block (system is responsible for disposal)
		gotoParamsP = MemPtrNew(sizeof(GoToParamsType));
		if (gotoParamsP)
		{
			// Fill it in
			MemPtrSetOwner (gotoParamsP, 0);
			gotoParamsP->recordNum		= dmInvalidRecIndex;
			gotoParamsP->recordID		= inRowID;
			gotoParamsP->matchPos		= 0;					// put cursor at end of string
			gotoParamsP->matchLen		= 0;					// length of match (for ToDo, ignored by Datebook)
			gotoParamsP->matchFieldNum	= todoDescriptionColId;	// same for Datebook and ToDo
			gotoParamsP->searchStrLen	= 0;					// length of match (for Datebook, ignored by ToDo)
			gotoParamsP->matchCustom	= 0;
			gotoParamsP->dbID			= 0;

			launchCmd = sysAppLaunchCmdGoTo;
			launchParamsP = gotoParamsP;
		}
		else
			ErrNonFatalDisplay ("Not enough memory to go to requested record");
	}

	// Switch to ToDo app to handle UI
	AppSwitchByCreator(sysFileCToDo, launchCmd, (MemPtr) gotoParamsP , sizeof(GoToParamsType));
}
