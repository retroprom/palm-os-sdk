/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateAgenda.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  Display and support code for Datebook's Agenda view.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <PalmTypesCompatibility.h>


#include <CatMgr.h>
#include <Chars.h>
#include <Control.h>
#include <DateTime.h>
#include <DataMgr.h>
#include <ErrorMgr.h>
#include <Font.h>
#include <Form.h>
#include <MemoryMgr.h>
#include <PenInputMgr.h>
#include <PrivateRecords.h>
#include <SelDay.h>
#include <ScrollBar.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <SystemResources.h>
#include <Table.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <UIColor.h>
#include <Window.h>
#include <string.h>

#include "DateGlobals.h"

#include "DateAgenda.h"
#include "DateAgendaToDo.h"
#include "DateDay.h"
#include "DateUtils.h"
#include "GUIUtilities.h"


// Lunar Calendar
#include "DateLunar.h"
#include "DateMonth.h"

#include "DateBackground.h"


#include "DatebookRsc.h"


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define minDatebookRows				1
#define minToDoRows					0

#define adjustSeparatorTop			2
#define adjustCategoryTop			2

#define maxDateTemplateLen			31


/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
static MemHandle					sAgendaApptsH = NULL;
static int32_t						sAgendaApptCount = 0;
static int32_t						sAgendaToDoCount = 0;
static int32_t						sAgendaTodoLineCount = 0;
static int32_t						sAgendaApptLineCount = 0;
static uint16_t						sAgendaApptLineHeight = 0;
static uint32_t						sAgendaTimeSeconds = 0;
static DateType						sToday;
static DateGadgetCallbackDataType	sGadgetCallbackData;
static RGBColorType					sBackgroundHighlight;
static uint16_t						sBackgroundMonth = 0xFFFF;

static uint16_t						sCurrentNavObjectID;
static Boolean						sOneHandedRecordSelected;
static Boolean						sOneHandedTableHasFocus;
static Boolean						sOneHandedCheckBoxSelected;
static int16_t						sOneHandedLastSelectedRow;

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static MemHandle	AgendaLoadAppointments (DateType inDate );
static void			AgendaSetAppointments (const MemHandle inAppointmentsH, int32_t inAppointmentCount );
static void			AgendaApptDrawTime (void* inTableP, int16_t inRow, int16_t inColumn, RectangleType* inBoundsP );
static void			AgendaApptDrawDesc (void* inTableP, int16_t inRow, int16_t inColumn, RectangleType* inBoundsP );
static void			AgendaViewChangeDate (FormType* inFormP, DateType inDate, Boolean redraw);
static void			AgendaViewInitDatebook (FormPtr	frm);
static Boolean		AgendaDividerDraw (FormGadgetTypeInCallback* inGadgetP, uint16_t inCommand, void* inParamP );
static void			AgendaViewDrawDate (FormType* inFormP );
static void			AgendaViewDrawTime (FormType* inFormP );
static void			AgendaViewFillAppointments (FormType* inFormP );
static void			AgendaDateToDOWDM (DateType inDate, char* outAscii );
static uint16_t		AgendaViewGetSeparatorHeight (FormType* inFormP );
static uint16_t		AgendaViewGetDatebookDefaultHeight (FormType* inFormP );
static void 		AgendaViewCountDisplayLines (FormType* inFormP, int32_t inApptCount, int32_t inToDoCount, int32_t* outApptLinesP, int32_t* outToDoLinesP );
static void			AgendaViewLayoutTables (FormType* inFormP, int32_t inApptLineCount, int32_t inToDoLineCount );
static void 		AgendaViewSetTopAppointment (void);
static void			AgendaViewLayoutAppointments (FormType* inFormP, RectangleType* ioBoundsP, int32_t inApptLineCount );
static void			AgendaViewLayoutSeparator (FormType* inFormP, RectangleType* ioBoundsP );
static void 		AgendaViewLayout (FormType* inFormP, int32_t inApptCount, int32_t inToDoCount);
static uint16_t		AgendaViewObjectSetTop (FormType* inFormP, uint16_t inObjectID, uint16_t inTop );
static void 		AgendaViewUpdateScroller (FormType* inFormP, uint16_t inObjectID, uint16_t inAssocObjectID, int32_t inRecordCount, int32_t inVisibleCount);

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewSelectCategory
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	 	None.
 *
 * RETURNED:    	Nothing.
 *
 * REVISION HISTORY:
 *	PLe	07/29/02	Initial revision, added category support.
 *
 ***********************************************************************/
static void AgendaViewSelectCategory(void)
{
	FormType* 		frmP;
	Boolean 		categoryEdited;
	CategoryID * 	newCatSelectionP = NULL;
	uint32_t		newCatSelectionNum = 0;
	status_t		err;

	frmP = FrmGetFormPtr(AgendaView);

	// Close all opened cursors (main cusor)
	// we have to close cursors as Datebook is using cahed cursors
	// unfortunaltely when a record is in such loaded in such a cache
	// when CatMgrSelectFilter is called, it cannot update records.
	err = ApptCloseCursor(&gApptCursorID);

	categoryEdited = CatMgrSelectFilter(
						ApptDB, frmP, AgendaCategoryTrigger,
						DateBkCategoriesName, AgendaCategoryList, DateBkCurrentCategoriesP,
						DateBkCurrentCategoriesCount, &newCatSelectionP, &newCatSelectionNum,
						true, NULL);


	if (categoryEdited)
	{
		// Update current categories
		ChangeCurrentCategories(newCatSelectionNum, newCatSelectionP);

		// Free list allocated by CatMgrSelectFilter
		CatMgrFreeSelectedCategories(ApptDB, &newCatSelectionP);

		// Display the new category.
		AgendaViewChangeDate (frmP, Date, true);
	}
	else
		AgendaLoadAppointments (Date);
}

/***********************************************************************
*
* FUNCTION: 	   AgendaViewInitLunarCalendar
*
* DESCRIPTION:	   This routine initializes the "Agenda View" Lunar of the
*				   Datebook application.
*
* PARAMETERS:  ->  formP:    pointer to the Agenda view form.
*
* RETURNED: 	   Nothing.
*
* NOTE: 		   None.
*
* HISTORY:
*  PPL 03/10/04    from PalmOS 5.4
*
***********************************************************************/
void AgendaViewInitLunarCalendar(FormPtr formP)
{
	// Display the Lunar View push button if we've got support for it.
	if (DateSupportsLunarCalendar())
		FrmShowObject (formP, FrmGetObjectIndex(formP, AgendaMonthLunarViewButton));
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewInitDatebook
 *
 * DESCRIPTION: 	Initialize the Datebook portion of the Agenda view.
 *
 * PARAMETERS:  ->	formP:	pointer to the agenda view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewInitDatebook (FormPtr formP)
{
	uint16_t 	row;
	uint16_t 	rowsInTable;
	TablePtr 	table;

	// Initialize the table used to display the day's agenda.
	table = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, AgendaDatebookTable));

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle (table, row, agendaApptTimeColumn, customTableItem);
		TblSetItemStyle (table, row, agendaApptDescColumn, customTableItem);
	}

	TblSetColumnUsable (table, agendaApptTimeColumn, true);
	TblSetColumnUsable (table, agendaApptDescColumn, true);
	TblSetColumnMasked (table, agendaApptDescColumn, true);

	TblSetColumnSpacing (table, agendaApptTimeColumn, agendaApptTimeColumnSpacing);

	// Set the callback routine that will load the description field.
	TblSetCustomDrawProcedure (table, agendaApptDescColumn, AgendaApptDrawDesc);

	// Set the callback routine that draws the time field.
	TblSetCustomDrawProcedure (table, agendaApptTimeColumn, AgendaApptDrawTime);
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewInit
 *
 * DESCRIPTION: 	Initialize the Agenda view of the Datebook application.
 *
 * PARAMETERS:  ->	formP:	pointer to the day view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	RBB	5/28/99		Initial revision.
 *	CS	11/14/00	Use PrefGetPreference instead of PrefGetPreferences.
 *	PPL	01/31/02	Add some active graffiti area support.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *
 ***********************************************************************/
static void AgendaViewInit (FormPtr formP)
{
	FontID		curFont;
	uint16_t	gadgetIndex;

	TopVisibleAppt = 0;
	sAgendaApptLineCount = 0;

	// Since we have a background image, we need to have transparent widgets
	FrmSetTransparentObjects(formP, true);

	curFont = FntSetFont (ApptDescFont);
	sAgendaApptLineHeight = FntLineHeight ();
	FntSetFont (curFont);

	DateBackgroundGetHighlightColor(&sBackgroundHighlight, Date.month);
	sBackgroundMonth = Date.month;

	memset(&sGadgetCallbackData, 0, sizeof(sGadgetCallbackData));
	sGadgetCallbackData.color = sBackgroundHighlight;

	gadgetIndex = FrmGetObjectIndex(formP, AgendaBackgroundGadget);

	// Save some info we will need in our callback
	FrmSetGadgetData(formP, gadgetIndex, &sGadgetCallbackData);

	// Set up the callback for the title bar graphic
	FrmSetGadgetHandler(formP, gadgetIndex, DateBackgroundGadgetHandler);

	// Get today
	DateSecondsToDate (TimGetSeconds (), &sToday);

	// Lunar Calendar
	AgendaViewInitLunarCalendar(formP);

	// Init views
	ToDoViewInit(formP);
	AgendaViewInitDatebook (formP);
	AgendaViewInitToDo (formP);

	// Divider
	// Save some info we will need in our callback
	gadgetIndex = FrmGetObjectIndex(formP, AgendaDivider);
	FrmSetGadgetData(formP, gadgetIndex, &sGadgetCallbackData);

	// Set up the callback for the divider line
	FrmSetGadgetHandler (formP, gadgetIndex, (FormGadgetHandlerType*) AgendaDividerDraw);

	// Set the label of the current category
	CatMgrSetTriggerLabel(ApptDB, DateBkCurrentCategoriesP, DateBkCurrentCategoriesCount,
	FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, AgendaCategoryTrigger)), DateBkCategoriesName);

	sCurrentNavObjectID = frmInvalidObjectId;
	sOneHandedRecordSelected = false;
	sOneHandedTableHasFocus = false;
	sOneHandedCheckBoxSelected = false;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaUpdateToDoCount
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
void AgendaUpdateToDoCount(void)
{
	sAgendaToDoCount = ToDoCountRecordsOnCurrentDate();
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewLayout
 *
 * DESCRIPTION: 	this routine displays the date passed.
 *
 * PARAMETERS:  ->	inFormP:
 *				->	inApptCount:
 *				->	inToDoCount:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	9/13/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewLayout (FormType* inFormP, int32_t inApptCount, int32_t inToDoCount)
{
	int32_t		apptLineCount;
	int32_t		todoLineCount;

	AgendaViewCountDisplayLines (inFormP, inApptCount, inToDoCount,
		&apptLineCount, &todoLineCount);

	AgendaViewLayoutTables (inFormP, apptLineCount, todoLineCount);
	AgendaViewUpdateScroller (inFormP, AgendaDatebookScroller,AgendaDatebookTable,
										inApptCount, apptLineCount);
	AgendaViewUpdateScroller (inFormP, AgendaToDoScroller, AgendaToDoTable,
										inToDoCount, todoLineCount);

	sAgendaApptLineCount = apptLineCount;
	sAgendaTodoLineCount = todoLineCount;
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewResize
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	eventP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/11/02	Initial revision.
 *	PPL 10/07/03	Change Name from AgendaViewToDoResize.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *	PPL 04/30/04	Add Landscape support.
 *
 ***********************************************************************/
static void AgendaViewResize (EventPtr eventP)
{
	RectangleType 	objBounds;
	uint16_t		objIndex;
	Coord			x;
	Coord			y;
	int16_t			offsetX;
	int16_t			offsetY;
	TableType*		tableP;
	Coord			width;
	Coord			height;
	FormType* 		frmP;

	width = eventP->data.winResized.newBounds.extent.x;
	height = eventP->data.winResized.newBounds.extent.y;
	frmP = FrmGetActiveForm();

	offsetY = height - gCurrentWinBounds.extent.y;
	offsetX = width  - gCurrentWinBounds.extent.x;

	if (!offsetX && !offsetY)
	{
		// if the window has the height of the screen
		// then the windows is already at the right size
		goto justSet;
	}

	// NB:
	// AgendaDivider and AgendaSeparatorDefaultArea are bboth involved in view split:
	// AgendaSeparatorDefaultArea is the separator that have to be move here
	// AgendaDivider draws the separtor line and is mmoved later as a result.
	// the y-axis position of AgendaSeparatorDefaultArea is used to fine tune
	// y-axis AgendaDivider. taking into account fonr size real rnumber of items
	// and so on.
	// see AgendaViewRefreshLayoutTasks()
	// 		that calls AgendaDayViewLayout()
	//  		that calls AgendaViewLayoutTables()
	// 				that calls AgendaViewLayoutSeparator()



	// AgendaScrollableArea (gadget)-  resized on both x and y
	objIndex =  FrmGetObjectIndex (frmP, AgendaScrollableArea);
	tableP = (TableType*) FrmGetObjectPtr( frmP, objIndex);

	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.y +=   offsetY;
	objBounds.extent.x +=   offsetX;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	// BackgroundGadget
	objIndex =  FrmGetObjectIndex (frmP, AgendaBackgroundGadget);
	FrmGetObjectBounds(frmP, objIndex, &objBounds);
	objBounds.extent.x +=   offsetX;
	objBounds.extent.y +=   offsetY;
	FrmSetObjectBounds(frmP, objIndex, &objBounds);

	if (offsetX)
	{
		// AgendaDatebookTable, set size of description column, X only
		objIndex =  FrmGetObjectIndex (frmP, AgendaDatebookTable);
		tableP = (TableType*) FrmGetObjectPtr( frmP, objIndex);

		FrmGetObjectBounds(frmP, objIndex, &objBounds);
		objBounds.extent.x +=   offsetX;
		FrmSetObjectBounds(frmP, objIndex, &objBounds);

		TblSetColumnWidth(tableP, agendaApptDescColumn, TblGetColumnWidth(tableP, agendaApptDescColumn) + offsetX);

		// AgendaDatebookScroller, move only on x
		objIndex =  FrmGetObjectIndex (frmP, AgendaDatebookScroller);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaDatebookScrAgendaToDoScrolleroller, move only on x
		objIndex =	FrmGetObjectIndex (frmP, AgendaToDoScroller);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);

		// AgendaToDoTable, set size of description column, X only
		objIndex =  FrmGetObjectIndex (frmP, AgendaToDoTable);
		tableP = (TableType*) FrmGetObjectPtr( frmP, objIndex);

		FrmGetObjectBounds(frmP, objIndex, &objBounds);
		objBounds.extent.x +=   offsetX;
		FrmSetObjectBounds(frmP, objIndex, &objBounds);
		ToDoListViewResizeTableColumns(tableP, offsetX);


		// AgendaSeparatorDefaultArea
		// we are dividing the offset by two as AgendaSeparatorDefaultArea
		// is here to split the screen space for date book appoitments and todo tasks
		//objIndex =  FrmGetObjectIndex (frmP, AgendaSeparatorDefaultArea);
		//FrmGetObjectPosition (frmP, objIndex, &x, &y);
		//y +=   (offsetY >> 1);
		//FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaCategoryTrigger, move only on x
		objIndex =  FrmGetObjectIndex (frmP, AgendaCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaCategoryList, move only on x
		objIndex =  FrmGetObjectIndex (frmP, AgendaCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaToDoCategoryTrigger, move only on x
		objIndex =  FrmGetObjectIndex (frmP, AgendaToDoCategoryTrigger);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaToDoCategoryList, move only on x
		objIndex =  FrmGetObjectIndex (frmP, AgendaToDoCategoryList);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		x +=   offsetX;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}

	if (offsetY)
	{
		// AgendaDayViewButton , move only on y
		objIndex =  FrmGetObjectIndex (frmP, AgendaDayViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaWeekViewButton, move only on y
		objIndex =  FrmGetObjectIndex (frmP, AgendaWeekViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaMonthLunarViewButton for Lunar Calendar
		if (DateSupportsLunarCalendar())
		{
			objIndex =	FrmGetObjectIndex (frmP, AgendaMonthLunarViewButton);
			FrmGetObjectPosition (frmP, objIndex, &x, &y);
			y +=   offsetY;
			FrmSetObjectPosition (frmP, objIndex, x, y);
		}

		// AgendaMonthViewButton, move only on y
		objIndex =  FrmGetObjectIndex (frmP, AgendaMonthViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaAgendaViewButton, move only on y
		objIndex =  FrmGetObjectIndex (frmP, AgendaAgendaViewButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);


		// AgendaGoToButton, move only on y
		objIndex =  FrmGetObjectIndex (frmP, AgendaGoToButton);
		FrmGetObjectPosition (frmP, objIndex, &x, &y);
		y +=   offsetY;
		FrmSetObjectPosition (frmP, objIndex, x, y);
	}


	// AgendaPreviousDayButton, move only both on x and y
	objIndex =  FrmGetObjectIndex (frmP, AgendaPreviousDayButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y +=   offsetY;
	x +=   offsetX;
	FrmSetObjectPosition (frmP, objIndex, x, y);


	// AgendaNextDayButton, move only both on x and y
	objIndex =  FrmGetObjectIndex (frmP, AgendaNextDayButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y +=   offsetY;
	x +=   offsetX;
	FrmSetObjectPosition (frmP, objIndex, x, y);


	// AgendaCurrentDayButton, move only both on x and y
	objIndex =  FrmGetObjectIndex (frmP, AgendaCurrentDayButton);
	FrmGetObjectPosition (frmP, objIndex, &x, &y);
	y +=   offsetY;
	x +=   offsetX;
	FrmSetObjectPosition (frmP, objIndex, x, y);

	// keep the window bounding rect.
	gCurrentWinBounds = eventP->data.winResized.newBounds;

justSet:
	AgendaViewRefreshLayoutTasks (frmP);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewCountDisplayLines
 *
 * DESCRIPTION: 	Calculates how many lines of Day Appointement
 *					versus how many lines of Task we show.
 *
 *
 * PARAMETERS:  ->	inFormP:
 *				->	inApptCount:
 *				->	inToDoCount:
 *				->	outApptLinesP:
 *				->	outToDoLinesP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	9/13/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewCountDisplayLines (FormType* inFormP, int32_t inApptCount, int32_t inToDoCount, int32_t* outApptLinesP, int32_t* outToDoLinesP )
{
	int32_t							apptCount;
	int32_t							todoCount;
	RectangleType					agendaBounds;
	int32_t							agendaHeight;
	int32_t							separatorHeight;
	int32_t							apptSectionHeight;
	int32_t							todoSectionHeight;
	int32_t							apptDefaultCount;
	int32_t							todoDefaultCount;
	int32_t							apptDisplayCount;
	int32_t							todoDisplayCount;
	int32_t							apptDisplayHeight;
	int32_t							todoDisplayHeight;
	int32_t							remainingDisplayHeight;
	int32_t							remainingDisplayCount;

	// Compensate for empty sections, which will show something like,
	// "No Appointments Today"
	apptCount = max (minDatebookRows, inApptCount);
	todoCount = max (minToDoRows, inToDoCount);

	// Get the default sizes for each of the view sections
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaScrollableArea), &agendaBounds);

	agendaHeight = agendaBounds.extent.y;
	separatorHeight = AgendaViewGetSeparatorHeight (inFormP);
	apptSectionHeight = AgendaViewGetDatebookDefaultHeight (inFormP);
	todoSectionHeight = agendaHeight - separatorHeight - apptSectionHeight;

	// Convert those sizes into line counts
	apptDefaultCount = apptSectionHeight / sAgendaApptLineHeight;
	todoDefaultCount = todoSectionHeight / sAgendaApptLineHeight;

	// Adjust the layout for optimum viewing
	if (apptCount <= apptDefaultCount)
	{
		// All appointments fit within the standard area
		apptDisplayCount = apptCount;
		apptDisplayHeight = apptDisplayCount * sAgendaApptLineHeight;

		if (todoCount <= todoDefaultCount)
		{
			// All tasks fit within the standard area
			todoDisplayCount = todoCount;
		}
		else
		{
			// Extra tasks can grow into the unused appointment area
			remainingDisplayHeight = agendaHeight - separatorHeight - apptDisplayHeight;
			remainingDisplayCount = remainingDisplayHeight / sAgendaApptLineHeight;
			todoDisplayCount = min (todoCount, remainingDisplayCount);
		}
	}
	else if (todoCount <= todoDefaultCount)
	{
		// All tasks fit within the standard area
		todoDisplayCount = todoCount;
		todoDisplayHeight = todoDisplayCount * sAgendaApptLineHeight;

		// Extra appointments can grow into the unused task area
		remainingDisplayHeight = agendaHeight - separatorHeight - todoDisplayHeight;
		remainingDisplayCount = remainingDisplayHeight / sAgendaApptLineHeight;
		apptDisplayCount = min (apptCount, remainingDisplayCount);
	}
	else
	{
		// Both the appointment and task sections are full, so use the default layout
		apptDisplayCount = apptDefaultCount;
		todoDisplayCount = todoDefaultCount;
	}

	*outApptLinesP = apptDisplayCount;
	*outToDoLinesP = todoDisplayCount;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewSetTopAppointment
 *
 * DESCRIPTION: 	This routine determines the first appointment that should
 *             	 	be visible on the current day.  For all dates other than
 *              	today the fisrt time slot of the appointment list
 *              	is the first visible appointment.  For today the time
 *              	slot that stats before to the current time should be the top
 *              	visible time slot.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewSetTopAppointment (void)
{
	uint16_t 		i;
	TimeType 		time;
	DateTimeType 	dateTime;
	ApptInfoPtr 	apptsP;

	TopVisibleAppt = 0;

	// If the current date is not today, then the first appointment
	// is the first one visible.
	if (DateToInt(sToday) != DateToInt(Date))
		return;

	// If the current date is today, then the top visible appointment is
	// the appointment with the greatest end time that is before the
	// current time.
	TimSecondsToDateTime (sAgendaTimeSeconds, &dateTime);
	time.hours 	= (uint8_t) dateTime.hour;
	time.minutes 	= (uint8_t) dateTime.minute;

	if (sAgendaApptsH)
	{
		apptsP = MemHandleLock (sAgendaApptsH);
		for (i = 0; i < sAgendaApptCount; i++)
		{
			if (TimeToInt (apptsP[i].endTime) < TimeToInt (time))
				TopVisibleAppt = i;
		}

		MemPtrUnlock (apptsP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewLayoutTasks
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inFormP:
 *				->	ioBoundsP:
 *				->	inToDoLineCount
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial Revision
 *
 ***********************************************************************/
static void AgendaViewLayoutTasks (FormType* inFormP, RectangleType* ioBoundsP, int32_t inToDoLineCount)
{
	TableType*			tableP;
	int16_t				row;
	int16_t				rowCount;
	int16_t				tableIndex;
	RectangleType		bounds;


	tableIndex = FrmGetObjectIndex (inFormP, AgendaToDoTable);
	tableP = FrmGetObjectPtr (inFormP, tableIndex);

	rowCount = TblGetNumberOfRows (tableP);
	TblGetBounds (tableP, &bounds);

	// Set the usability for each row in the table. If there are fewer tasks
	// than rows, the remaining rows will be unusable
	for (row = 0; row < rowCount; row++)
	{
		if ((row < inToDoLineCount)
			&& (!TblRowUsable (tableP, row)
				|| (ioBoundsP->topLeft.y != bounds.topLeft.y)
				|| (TblGetRowHeight (tableP, row) != sAgendaApptLineHeight)) )
		{
			TblMarkRowInvalid (tableP, row);
		}

		TblSetRowUsable (tableP, row, (Boolean)(row < inToDoLineCount));
		TblSetRowHeight (tableP, row, sAgendaApptLineHeight);
	}

	// For drawing purposes, the table should extend to the bottom of the
	// displayable area, regardless of the number of displayable lines.
	TblSetBounds (tableP, ioBoundsP);

	// about focusing
	//if (sAgendaFocusedTableId == AgendaToDoTable)
	//	FrmSetFocus(inFormP, tableIndex);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewLayoutTables
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	inFormP:
 *				->	inApptLineCount:
 *				->	inToDoLineCount:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial Revision.
 *
 ***********************************************************************/
static void AgendaViewLayoutTables (FormType* inFormP, int32_t inApptLineCount, int32_t inToDoLineCount)
{
	RectangleType		scrollableRect;
	RectangleType		apptRect;
	RectangleType		separatorRect;
	RectangleType		todoRect;

	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaScrollableArea), &scrollableRect);

	apptRect = scrollableRect;

	AgendaViewLayoutAppointments (inFormP, &apptRect, inApptLineCount);

	separatorRect.topLeft.x = scrollableRect.topLeft.x;
	separatorRect.topLeft.y = apptRect.topLeft.y + apptRect.extent.y;
	separatorRect.extent.x = scrollableRect.extent.x;
	separatorRect.extent.y = scrollableRect.extent.y - apptRect.extent.y;

	AgendaViewLayoutSeparator (inFormP, &separatorRect);

	todoRect.topLeft.x = scrollableRect.topLeft.x;
	todoRect.topLeft.y = separatorRect.topLeft.y + separatorRect.extent.y;
	todoRect.extent.x = scrollableRect.extent.x;
	todoRect.extent.y = scrollableRect.extent.y - apptRect.extent.y - separatorRect.extent.y;

	AgendaViewLayoutTasks (inFormP, &todoRect, inToDoLineCount);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewLayoutAppointments
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	inFormP:
 *				->	ioBoundsP:
 *				->	inApptLineCount:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewLayoutAppointments (
	FormType* 		inFormP,
	RectangleType* 	ioBoundsP,
	int32_t 		inApptLineCount)
{
	TableType*		tableP;
	int16_t		row;
	int16_t		rowCount;
	int16_t			tableIndex;
	RectangleType	bounds;

	tableIndex = FrmGetObjectIndex (inFormP, AgendaDatebookTable);
	tableP = FrmGetObjectPtr (inFormP, tableIndex);

	rowCount = TblGetNumberOfRows (tableP);

	// Set the usability for each row in the table. If there are fewer appointments
	// than rows, the remaining rows will be unusable
	for (row = 0; row < rowCount; row++)
	{
		if ((row < inApptLineCount)
				&& (!TblRowUsable (tableP, row)
						|| (TblGetRowHeight (tableP, row) != sAgendaApptLineHeight)) )
		{
			TblMarkRowInvalid (tableP, row);
		}

		TblSetRowUsable (tableP, row, (Boolean)(row < inApptLineCount));
		TblSetRowHeight (tableP, row, sAgendaApptLineHeight);
	}

	// Adjust the table size so that it doesn't overlap the To Do table
	TblGetBounds (tableP, &bounds);
	bounds.extent.y = (Coord) (inApptLineCount * sAgendaApptLineHeight);
	TblSetBounds (tableP, &bounds);
	ioBoundsP->extent.y = (Coord) (inApptLineCount * sAgendaApptLineHeight);

	// about focusing
	//if (sAgendaFocusedTableId == AgendaDatebookTable)
	//	FrmSetFocus(inFormP, tableIndex);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewLayoutSeparator
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	inFormP:
 *				->	ioBoundsP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewLayoutSeparator (
	FormType*			inFormP,
	RectangleType*		ioBoundsP )
{
	RectangleType		dividerBounds;
	uint16_t			top = ioBoundsP->topLeft.y;

	AgendaViewObjectSetTop (inFormP, AgendaDivider, top);
	AgendaViewObjectSetTop (inFormP, AgendaToDoCategoryList, (uint16_t)(top + adjustCategoryTop));
	AgendaViewObjectSetTop (inFormP, AgendaToDoCategoryTrigger, (uint16_t)(top + adjustCategoryTop));

	FrmGetObjectBounds (inFormP, FrmGetObjectIndex (inFormP, AgendaDivider), &dividerBounds);
	ioBoundsP->topLeft.y = dividerBounds.topLeft.y;
	ioBoundsP->extent.y = dividerBounds.extent.y;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewObjectSetTop
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inFormP:
 *				->	inObjectID:
 *				->	inTop:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
static uint16_t AgendaViewObjectSetTop (
	FormType* 		inFormP,
	uint16_t 		inObjectID,
	uint16_t 		inTop)
{
	RectangleType	bounds;
	uint16_t		index;

	index = FrmGetObjectIndex (inFormP, inObjectID);
	FrmGetObjectBounds (inFormP, index, &bounds);
	bounds.topLeft.y = inTop;
	FrmSetObjectBounds (inFormP, index, &bounds);

	return bounds.topLeft.y + bounds.extent.y;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewUpdateScroller
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inFormP:
 *				->	inObjectID:
 *				->	inAssocObjectID:
 *				->	inRecordCount:
 *				->	inVisibleCount:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial Revision
 *
 ***********************************************************************/
static void AgendaViewUpdateScroller (
	FormType* 		inFormP,
	uint16_t 		inObjectID,
	uint16_t 		inAssocObjectID,
	int32_t 		inRecordCount,
	int32_t 		inVisibleCount)
{
	RectangleType	assocBounds;
	RectangleType	scrollBounds;
	uint16_t		scrollIndex;
	int32_t			currentValue;
	int32_t			currentMin;
	int32_t			currentMax;
	int32_t			currentPageSize;
	int32_t			scrollMax;

	FrmGetObjectBounds (inFormP, FrmGetObjectIndex (inFormP, inAssocObjectID), &assocBounds);

	scrollMax = max (0, inRecordCount - inVisibleCount);	// without the typecast, max ignores the 0!!!

	// Resize the scroll bar
	scrollIndex = FrmGetObjectIndex (inFormP, inObjectID);
	FrmGetObjectBounds (inFormP, scrollIndex, &scrollBounds);

	scrollBounds.topLeft.y = assocBounds.topLeft.y;
	scrollBounds.extent.y = assocBounds.extent.y;

	FrmSetObjectBounds (inFormP, scrollIndex, &scrollBounds);

	SclGetScrollBar (
		FrmGetObjectPtr (inFormP, FrmGetObjectIndex (inFormP, inObjectID)),
		&currentValue,
		&currentMin,
		&currentMax,
		&currentPageSize);

	currentValue = min (currentValue, scrollMax);

	SclSetScrollBar (FrmGetObjectPtr (inFormP, FrmGetObjectIndex (inFormP, inObjectID)),
		currentValue, 0, scrollMax, inVisibleCount);
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewSetScrollThumb
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inFormP:
 *				->	inObjectID:
 *				->	inValue:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
void AgendaViewSetScrollThumb (FormType* inFormP, uint16_t inObjectID, int32_t inValue)
{
	ScrollBarType*	scrollBarP;
	int32_t 		scrollValue;
	int32_t			scrollMin;
	int32_t			scrollMax;
	int32_t			scrollPage;

	scrollBarP = FrmGetObjectPtr (inFormP, FrmGetObjectIndex (inFormP, inObjectID));

	SclGetScrollBar (scrollBarP, &scrollValue, &scrollMin, &scrollMax, &scrollPage);
	SclSetScrollBar (scrollBarP, inValue, scrollMin, scrollMax, scrollPage);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewRefreshLayoutTasks
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  -> 	inFormP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
void AgendaViewRefreshLayoutTasks (FormType* inFormP )
{
	int32_t	oldApptLineCount;

	// This routine is called when the To Do section of the agenda has been altered.
	// Even though only the To Do section has changed, the heuristic divider may
	// need to be repositioned if the Datebook section overflows its default size.

	oldApptLineCount = sAgendaApptLineCount;

	AgendaViewLayout (inFormP, sAgendaApptCount, sAgendaToDoCount);

	if (sAgendaApptLineCount != oldApptLineCount)
	{
		AgendaViewFillAppointments (inFormP);
		AgendaViewSetScrollThumb (inFormP, AgendaDatebookScroller, TopVisibleAppt);
	}

	AgendaViewFillTasks ();
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaApptDrawTime
 *
 * DESCRIPTION: 	Draw the starting time of an appointment.  This
 *              	routine is called by the table object as a callback
 *             		routine.
 *
 * PARAMETERS:  ->	table:		pointer to the memo Day table (TablePtr)
 *              ->	row:		row of the table to draw
 *              ->	column:		column of the table to draw
 *              ->	bounds:		region to draw in
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	6/4/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaApptDrawTime (
	void*						inTableP,
	int16_t						inRow,
	int16_t						inColumn,
	RectangleType*				inBoundsP )
{
	uint16_t					apptIndex;
	ApptInfoPtr					appts;
	TimeType					startTime;
	Boolean						conflictingAppt;
	Boolean						noTimeAppt;
	MidnightOverlapStateType 	overlapState;

	// Get the appointment that corresponds to the table item.
	// The index of the appointment in the appointment list is stored
	// as the row id.
	apptIndex = TblGetRowID (inTableP, inRow);

	// When there are no appointments, a special index is stored. If this is the case, don't
	// try to draw the time; the description column will show "No Appointments"
	if (apptIndex != noAppointments)
	{
		// Get the record index of the next appointment
		appts = MemHandleLock (sAgendaApptsH);
		overlapState = appts[apptIndex].overlapState;
		conflictingAppt = ! appts[apptIndex].belongingToCurrCat;
		noTimeAppt = appts[apptIndex].noTimeEvent;

		if (appts[apptIndex].overlapState == overlapScndPart)
			startTime = appts[apptIndex].startTimeInPrevDay;
		else
			startTime = appts[apptIndex].startTime;

		MemPtrUnlock (appts);

		DrawTime (startTime, overlapState, noTimeAppt, conflictingAppt,
			TimeFormat, apptTimeFont, rightAlign, inBoundsP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaApptDrawDesc
 *
 * DESCRIPTION: 	Draw the description of an appointment.  This
 *              	routine is called by the table object as a callback
 *              	routine.
 *
 *					Table Draw Callback.
 *
 * PARAMETERS:  ->	inTableP:	pointer to the memo Day table (TablePtr)
 *              ->	inRow:		row of the table to draw
 *              ->	inColumn:	column of the table to draw
 *              ->	inBoundsP:	region to draw in
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	8/30/99		Initial revision.
 *	ppl	07/15/03	cursors !
 *
 ***********************************************************************/
static void AgendaApptDrawDesc (
	void*						inTableP,
	int16_t						inRow,
	int16_t						inColumn,
	RectangleType*				inBoundsP )
{
	status_t					err = errNone;
	uint16_t					apptIndex;
	ApptInfoPtr					apptsP;
	uint32_t					rowID;
	ApptDBRecordType			apptRec;
	MemHandle					textH = NULL;
	char*						textP = NULL;
	FontID						curFont;
	char*						tempP;
	uint16_t					tempCount = 0;
	MidnightOverlapStateType 	overState = overlapNone;
	RGBColorType				prevTextColor;

	// Get the appointment that corresponds to the table item.
	// The index of the appointment in the appointment list is stored
	// as the row id.
	apptIndex = TblGetRowID (inTableP, inRow);

	// When there are no appointments, a special index is stored. If this is the case, don't
	// try to draw the time; the description column will show "No Appointments"
	if (apptIndex != noAppointments)
	{
		// Get the record index of the next appointment
		apptsP = MemHandleLock (sAgendaApptsH);
		rowID = apptsP[apptIndex].rowID;
		overState = apptsP[apptIndex].overlapState;
		MemPtrUnlock (apptsP);

		// Get the offset and length of the description field
		err = ApptGetRecord (ApptDB, rowID, &apptRec, DBK_SELECT_DESCRIPTION);

		if (err >= errNone)
			textP = apptRec.description;
	}
	else
	{
		// Check if the day displayed is Today or not
		if (DateToInt(sToday) == DateToInt(Date))
			textH = DmGetResource (gApplicationDbP, strRsc, agendaNoAppointmentsTodayStrID);
		else
			textH = DmGetResource (gApplicationDbP, strRsc, agendaNoAppointmentsThisDayStrID);

		textP = MemHandleLock (textH);
	}

	if (textP)
	{
		WinDrawOperation oldDraw;
		curFont = FntSetFont (ApptDescFont);

		tempP = textP;
		while (*tempP && *tempP != linefeedChr)
		{
			++tempCount;
			++tempP;
		}

		// Set new and save current color
		if (overState == overlapScndPart)
			WinSetTextColorRGB(&MediumGreyColor, &prevTextColor);
		else
			WinSetTextColorRGB(NULL, &prevTextColor);

		oldDraw = WinSetDrawMode(winOverlay);
		WinPaintTruncChars (
				textP,
				tempCount, inBoundsP->topLeft.x, inBoundsP->topLeft.y,
				inBoundsP->extent.x);
		WinSetDrawMode(oldDraw);

		// Restore color
		WinSetTextColorRGB(&prevTextColor, NULL);
		FntSetFont (curFont);
	}

	if (apptIndex != noAppointments)
		ApptFreeRecordMemory (&apptRec);

	if (textH)
	{
		MemHandleUnlock (textH);
		DmReleaseResource (textH);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaDividerDraw
 *
 * DESCRIPTION: 	Gadget Callback.
 *
 * PARAMETERS:  ->	inGadgetP:
 *				->	inCommand:
 * 				->	inParamP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
static Boolean AgendaDividerDraw (FormGadgetTypeInCallback* inGadgetP, uint16_t inCommand, void* inParamP)
{
	EventType * 		eventP;
	EventType 			newEvent;
	ControlType * 		categoryTriggerP;
	FormType *			gadgetFormP;
	uint16_t 			gadgetIndex;
	RectangleType 		gadgetBounds;
	DateGadgetCallbackDataType* cbdP;
	RGBColorType		oldColor;

	// Get the bounds of the gadget from the form
	gadgetFormP = FrmGetActiveForm();
	if (gadgetFormP == NULL)
		return false;

	gadgetIndex = FrmGetObjectIndexFromPtr (gadgetFormP, inGadgetP);
	FrmGetObjectBounds (gadgetFormP, gadgetIndex, &gadgetBounds);

	switch (inCommand)
	{
		case formGadgetDrawCmd:
			cbdP = (DateGadgetCallbackDataType *)inGadgetP->data;
			WinSetForeColorRGB(&(cbdP->color), &oldColor);
			WinDrawLine (	gadgetBounds.topLeft.x,
							(Coord)(gadgetBounds.topLeft.y + adjustSeparatorTop),
							(Coord)(gadgetBounds.topLeft.x + gadgetBounds.extent.x - 1),
							(Coord)(gadgetBounds.topLeft.y + adjustSeparatorTop));
			WinSetForeColorRGB(&oldColor, NULL);
			return true;

		case formGadgetEraseCmd:
			WinEraseLine (	gadgetBounds.topLeft.x,
							(Coord)(gadgetBounds.topLeft.y + adjustSeparatorTop),
							(Coord)(gadgetBounds.topLeft.x + gadgetBounds.extent.x - 1),
							(Coord)(gadgetBounds.topLeft.y + adjustSeparatorTop));
			return true;

		case formGadgetHandleEventCmd:
			// Since the gadget can overlap the popup trigger for the to do category,
			// we have to take the gadget enter event generated for a pen down event
			// and convert it back to a pen down event to pass on to the control. If
			// the point is in the bounds of the popup trigger, it will then convert
			// it into a control enter event.
			eventP = (EventType *)inParamP;
			categoryTriggerP = FrmGetObjectPtr (
								gadgetFormP,
								FrmGetObjectIndex (gadgetFormP, AgendaToDoCategoryTrigger));

			if (eventP->eType == frmGadgetEnterEvent)
			{
				newEvent = *eventP;
				newEvent.eType = penDownEvent;

				return CtlHandleEvent(categoryTriggerP, &newEvent);
			}
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewDrawDate
 *
 * DESCRIPTION: 	Draw the current date
 *
 * PARAMETERS:  ->	inFormP:	pointer to the agenda view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	6/4/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewDrawDate (FormType* inFormP )
{
	static char		titleP [dowLongDateStrLength];
	ControlType*	controlP;

	AgendaDateToDOWDM (Date, titleP);

	controlP = (ControlType*) FrmGetObjectPtr (inFormP, FrmGetObjectIndex (inFormP, AgendaCurrentDayButton));

	CtlSetLabel (controlP, titleP);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewDrawTime
 *
 * DESCRIPTION: 	Draw the current time.
 *
 * PARAMETERS:  ->	inFormP:	pointer to the agenda view form.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	6/4/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewDrawTime (FormType* inFormP )
{
	static char		timeStr [timeStringLength];
	DateTimeType 	dateTime;

	TimSecondsToDateTime (sAgendaTimeSeconds, &dateTime);
	TimeToAscii ((uint8_t) dateTime.hour, (uint8_t) dateTime.minute, TimeFormat, timeStr);

	FrmCopyTitle (inFormP, timeStr);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaDateToDOWDM
 *
 * DESCRIPTION: 	Similar to DateToAscii, but accepts DateType and masks
 *					out century
 *
 * PARAMETERS:  ->	inDate:
 *				->	outAscii:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	06/28/99	Initial revision.
 *	gap	10/10/00 	Added a format string list so that the format string
 *					in the agenda view title will respect user's format
 *					setting specified in formats panel.
 *
 ***********************************************************************/
static void AgendaDateToDOWDM (DateType inDate, char*	outAscii )
{
	uint16_t	dateFormatIndex;
	char 		templateBuffer[maxDateTemplateLen + 1];


	// Convert the current short day of week format selector into an index into the
	// agenda title date formats list.
	if (ShortDateFormat > dfYMDWithDashes)
	{
		// Default to the dfMDYWithSlashes format.
		dateFormatIndex = (uint16_t)dfMDYWithSlashes;
	}
	else
		dateFormatIndex = (uint16_t) ShortDateFormat;

	SysStringByIndex(gApplicationDbP, agendaTitleDateFormatsListID, (uint16_t)dateFormatIndex,
		templateBuffer, sizeof(templateBuffer) - 1);

	DateTemplateToAscii(templateBuffer, (uint8_t) inDate.month, (uint8_t) inDate.day, (uint16_t)(inDate.year + firstYear),
								outAscii, dowLongDateStrLength);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewGoToDate
 *
 * DESCRIPTION: 	This routine displays the date picker so that the
 *              	user can select a date to navigate to.  If the date
 *              	picker is confirmed, the date selected is displayed.
 *
 *              	This routine is called when a "go to" button is pressed.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewGoToDate ( void )
{
	char* 		title;
	MemHandle 	titleH;
	int16_t 	month;
	int16_t		day;
	int16_t		year;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (gApplicationDbP, strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	// Display the date picker.
	if (SelectDay (selectDayByDay, &month, &day, &year, title))
	{
		Date.day = day;
		Date.month = month;
		Date.year = year - firstYear;

		AgendaViewChangeDate (FrmGetActiveForm(), Date, true);
	}

	MemHandleUnlock (titleH);
	DmReleaseResource(titleH);
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewFillAppointments
 *
 * DESCRIPTION:
 *
 * PARAMETERS: 	->	inFormP:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaViewFillAppointments (FormType*	inFormP )
{
	TableType*		tableP;
	RectangleType	tableBounds;
	uint16_t		row;
	uint16_t		lastRow;
	FontID			oldFont;
	uint16_t		apptIndex;
	uint16_t		rowHeight = 0;
	uint16_t		displayableHeight = 0;
	uint16_t 		attr;
	Boolean 		masked;
	int32_t			maxIndex;
	ApptInfoType*	apptsP;

	// Get the height of the table and the width of the description column.
	tableP = FrmGetObjectPtr (inFormP, FrmGetObjectIndex (inFormP, AgendaDatebookTable));
	TblGetBounds (tableP, &tableBounds);

	row = 0;
	lastRow = TblGetLastUsableRow (tableP);

	oldFont = FntSetFont (ApptDescFont);
	rowHeight = FntLineHeight ();
	FntSetFont (oldFont);

	maxIndex = max (0, sAgendaApptCount - lastRow - 1);
	apptIndex = (uint16_t) min (TopVisibleAppt, maxIndex);
	TopVisibleAppt = apptIndex;

	// Associate each row with the appropriate appointment
	while ( (apptIndex < sAgendaApptCount) && (row <= lastRow) )
	{
		// Also ensure that we're within our drawing area
		if ( displayableHeight + rowHeight <= tableBounds.extent.y )
		{
			TblSetRowID (tableP, row, (uint16_t) apptIndex);

			// Mask if appropriate
			if (sAgendaApptsH)
			{
				apptsP = MemHandleLock (sAgendaApptsH);

				DbGetRowAttr (ApptDB, apptsP[apptIndex].rowID, &attr);

			   	masked = (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));

				TblSetRowMasked(tableP,row,masked);

				MemPtrUnlock (apptsP);
			}

			apptIndex++;
			row++;
			displayableHeight += rowHeight;
		}
		else
		{
			break;
		}
	}

	if (sAgendaApptCount == 0)
	{
		// Set the ID to a special value to trigger the display of the "no appointments" text
		TblSetRowID (tableP, 0, 0xffff);
		TblSetRowMasked (tableP, row, false);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaLoadAppointments
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inDate:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
static MemHandle AgendaLoadAppointments (DateType	inDate )
{
	MemHandle	appointmentsH;
	int32_t		count;

	// Load the day's appointments from the database, do not add conflicting
	// appointments from other categories.
	ApptGetAppointments (ApptDB, &gApptCursorID, inDate, 1, &appointmentsH, &count, false);

	// Replace the current appointments with our new data (frees any old data)
	AgendaSetAppointments (appointmentsH, count);

	return appointmentsH;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaFreeAppointments
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   	 	Nothing.
 *
 * NOTE:
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision.
 *
 ***********************************************************************/
static void AgendaFreeAppointments (void )
{
	if ( sAgendaApptsH != NULL )
	{
		MemHandleFree (sAgendaApptsH);
	}

	sAgendaApptsH = NULL;
	sAgendaApptCount = 0;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaSetAppointments
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  ->	inAppointmentsH:
 *				->	inAppointmentCount:
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	5/28/99		Initial revision
 *
 ***********************************************************************/
static void AgendaSetAppointments (const MemHandle inAppointmentsH, int32_t inAppointmentCount )
{
	AgendaFreeAppointments();

	sAgendaApptsH = inAppointmentsH;
	sAgendaApptCount = inAppointmentCount;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewGetSeparatorHeight
 *
 * DESCRIPTION: 	Returns the height of the area between the datebook and
 *					to do sections of the agenda view, into which the category
 *					popup is drawn
 *
 * PARAMETERS:  	Nothing
 *
 * RETURNED:    	outBoundsP:	Screen area for agenda contents.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	6/14/99		Initial revision.
 *
 ***********************************************************************/
static uint16_t AgendaViewGetSeparatorHeight (FormType* inFormP )
{
	RectangleType	bounds;

	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaSeparatorDefaultArea), &bounds);

	return bounds.extent.y;
}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewGetDatebookDefaultHeight
 *
 * DESCRIPTION: 	Returns the height of the area between the datebook and
 *					to do sections of the agenda view, into which the category
 *					popup is drawn
 *
 * PARAMETERS:  ->	inFormP:
 *
 * RETURNED:    	outBoundsP:	 Screen area for agenda contents.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	rbb	6/14/99		Initial Revision.
 *
 ***********************************************************************/
static uint16_t AgendaViewGetDatebookDefaultHeight (FormType* inFormP )
{
	RectangleType	agendaBounds;
	RectangleType	separatorBounds;

	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaScrollableArea), &agendaBounds);
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaSeparatorDefaultArea), &separatorBounds);

	return separatorBounds.topLeft.y - agendaBounds.topLeft.y;
}


/***********************************************************************
 *
 * FUNCTION:    	GoToAppointment
 *
 * DESCRIPTION: 	Switch to the To Do application and display the given
 *					record On the day view.
 *
 *					If there are any problems retrieving the record,
 *					launch the app without it.
 *
 *					When record is private ans security is set at some levels
 *					(password assigned) calls the security password tyo improve
 *					responsiveness otherwise user see a white screen for a few
 *					seconds while day view is parsing the db, setting up the view
 *					and bringing the seurity password.
 *
 * PARAMETERS:  ->	inDB:		a pointer to an EventType structure
 *				->	rowID:
 *				->	overlapState:
 *
 * RETURNED:    	true if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
static void GoToAppointment (
	DmOpenRef 					inDB,
	uint32_t 					rowID,
	MidnightOverlapStateType 	overlapState)
{
	uint16		rowAttributes;
	status_t 	err;

	// Set the record to be edited in the day view
	if (rowID != dbInvalidRowID)
	{
		err = DbCursorMove(gApptCursorID, rowID, dbFetchRowID);

		if (err >= errNone)
		{
			FrmSetFocus(FrmGetFormPtr(AgendaView), noFocus);
			DayViewFocusSetRowToSelect(tblUnusableRow);

			// Set up Day View item selection
			gItemSelected = true;

			// Set the cursor at the end of the line in Day View
			DayEditPosition = mostRightFieldPosition;

			// Mask if appropriate
			err = DbGetRowAttr(ApptDB, rowID, &rowAttributes);

	   		if (((rowAttributes & dmRecAttrSecret) && CurrentRecordVisualStatus == maskPrivateRecords))
			{
				if (DateUtilsSecVerifyPW(showPrivateRecords))
					CurrentRecordVisualStatus = showPrivateRecords;
				else
				{
					//	exit whithout going to day view
					// but we update the agenda view
					// as private events might have disappear (lost password)
					AgendaViewChangeDate (gCurrentFormPtr, Date, true);

					// Reset Day View item selection
					gItemSelected = false;
					return;
				}
			}
		}
	}

	// Switch to the day view
	if (overlapState == overlapScndPart)
		DateAdjust (&Date, -1);

	FrmGotoForm(gApplicationDbP, DayView);
}


/***********************************************************************
 *
 * FUNCTION:		AgendaViewTableValidRow
 *
 * DESCRIPTION:
 *
 * PARAMETERS:	->	row:		row index to select
 *				->	focusing:	should we focus
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFE 03/29/04	Initial Revision.
 *	PPL 06/16/04	Add PalmSOurce One-Handed specification.
 *
 ***********************************************************************/
static Boolean AgendaViewTableValidRow(TablePtr tableP, int16_t row)
{
	int16_t	lastNum;

	lastNum = TblGetLastUsableRow(tableP);

	return (Boolean) ((row > 0) && (row <lastNum));
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaDayViewScroll
 *
 * DESCRIPTION: 	This routine scrolls the list of ToDo items
 *              	in the direction specified.
 *
 * PARAMETERS:  ->	direction:	inUp or dowm
 *              ->	oneLine:	if true the list is scrolled by a single line,
 *                          	if false the list is scrolled by a full screen.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	art	2/21/95		Initial revision.
 *	PPL 10/07/03	Change Name to
 *	PPL 10/07/03	USe WinUpdateCallback rather than calling TblRedraw.
 *
 ***********************************************************************/
static Boolean AgendaViewScroll (int16_t delta)
{
	RectangleType	dirtyRect;
	TableType* 		tableP;
	FormType* 		frmP;
	uint16_t 		apptIndex;
	uint16_t		rowCount;
	uint16_t		newIndex;
	Boolean			scrolled = false;

	if (delta == 0)
		return false;

	frmP = FrmGetActiveForm();
	tableP = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, AgendaDatebookTable));
	TblReleaseFocus (tableP);

	apptIndex = TopVisibleAppt;
	rowCount = TblGetLastUsableRow (tableP) + 1;

	ErrFatalDisplayIf (apptIndex == noAppointments, "AgendaViewScroll called with no appointments");

	newIndex = max (0, (uint16_t) apptIndex + delta);
	newIndex = (uint16_t) min (newIndex, sAgendaApptCount - rowCount);

	if (newIndex != TopVisibleAppt)
	{
		TopVisibleAppt = newIndex;

		AgendaViewFillAppointments (FrmGetActiveForm());

		TblGetBounds(tableP, &dirtyRect);
		AgendaViewDisplayInvalidate(frmP, &dirtyRect);
		scrolled = true;
	}

	return scrolled;
}


/***********************************************************************
 *
 * FUNCTION:		AgendaViewDrawTableItemFocusRing
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe 081104	Creation.
 *
 ***********************************************************************/
static void AgendaViewDrawTableItemFocusRing(FormType* formP, TableType *tableP, int16_t tableRowIndex, int16_t column)
{
	RectangleType	bounds, itemBounds;

	if (!sOneHandedTableHasFocus || !sOneHandedRecordSelected)
		return;

	if (column == -1)
	{
		TblGetBounds(tableP, &bounds );
		TblGetItemBounds(tableP, tableRowIndex, 0, &itemBounds );
		bounds.topLeft.y = itemBounds.topLeft.y;
		bounds.extent.y = itemBounds.extent.y;
	}
	else TblGetItemBounds(tableP, tableRowIndex, column, &bounds );

	FrmClearFocusHighlight();
	FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
}

/***********************************************************************
 *
 * FUNCTION:		AgendaViewHandleNavigationUp
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe 081104	Creation.
 *
 ***********************************************************************/
static Boolean AgendaViewHandleNavigationUp(FormType* formP, Boolean objectFocusMode, Boolean repeat)
{
	int16_t		delta = 0;
	Boolean		scrolled = false;
	Boolean		handled = false;
	uint32_t	rowID = dbInvalidRowID;
	uint16_t	rowIndex = noAppointments;
	TablePtr	tableP;
	int16_t		column;
	int16_t		tableLineCount;
	int32_t		itemCount;
	uint16_t	scrollID;
	int32_t		topItem;

	// ToDo index is 1 based....

	if (objectFocusMode)
	{
		if (sOneHandedTableHasFocus)
		{
			tableLineCount = (int16_t)((sCurrentNavObjectID == AgendaDatebookTable) ? sAgendaApptLineCount: sAgendaTodoLineCount);
			itemCount = (sCurrentNavObjectID == AgendaDatebookTable) ? sAgendaApptCount: sAgendaToDoCount + 1;
			scrollID = (sCurrentNavObjectID == AgendaDatebookTable) ? AgendaDatebookScroller : AgendaToDoScroller;
			tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, sCurrentNavObjectID));
			topItem = TblGetRowID (tableP, 0) - ((sCurrentNavObjectID == AgendaDatebookTable) ? 0 : 1);

			if (sOneHandedRecordSelected)
			{
				delta = 0;
				scrolled = false;
				rowIndex = TblGetRowID (tableP, sOneHandedLastSelectedRow);

				// Is it possible to scroll ? Are we on the first item ?
				if (rowIndex)
				{
					rowIndex--;
					sOneHandedLastSelectedRow--;

					// Are we on the latest line of the table ?
					if (sOneHandedLastSelectedRow < 0)
						delta = -1;
					else scrolled = true;
				}
			}
			else if (itemCount && topItem)
					delta = -tableLineCount + 1;

			if (delta)
			{
				if (sCurrentNavObjectID == AgendaDatebookTable)
				{
					if ((scrolled = AgendaViewScroll (delta)) == true)
						AgendaViewSetScrollThumb (formP, scrollID, TopVisibleAppt);
				}
				else
				{
					if ((scrolled = ToDoListViewScroll (delta)) == true)
						AgendaViewSetScrollThumb (formP, scrollID, TblGetRowID (tableP, 0) - 1);
				}

				if (sOneHandedRecordSelected && !TblFindRowID(tableP, rowIndex, &sOneHandedLastSelectedRow))
					sOneHandedLastSelectedRow = 0;
			}

			if (sOneHandedRecordSelected)
			{
				column = (sCurrentNavObjectID == AgendaDatebookTable) ? -1 : sOneHandedCheckBoxSelected ? completedColumn : completedColumn + 2;
				AgendaViewDrawTableItemFocusRing(formP, tableP, sOneHandedLastSelectedRow, column);
				column = (sCurrentNavObjectID == AgendaDatebookTable) ?  agendaApptDescColumn : sOneHandedCheckBoxSelected ? completedColumn : completedColumn + 2;
				TblSelectItem (tableP, sOneHandedLastSelectedRow, column);
			}

			handled = scrolled || repeat;
		}
	}
	else
	{
		FrmSetFocus(formP, FrmGetObjectIndex(formP, AgendaCategoryTrigger));
		FrmNavObjectTakeFocus(formP, AgendaCategoryTrigger);
		handled = true;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:		AgendaViewHandleNavigationDown
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe 081104	Creation.
 *
 ***********************************************************************/
static Boolean AgendaViewHandleNavigationDown(FormType* formP, Boolean objectFocusMode, Boolean repeat)
{
	int16_t		delta = 0;
	Boolean		scrolled = false;
	Boolean		handled = false;
	uint32_t	rowID = dbInvalidRowID;
	uint16_t	rowIndex = noAppointments;
	TablePtr	tableP;
	int16_t		column;
	int16_t		tableLineCount;
	int32_t		itemCount;
	int32_t		topItem;
	uint16_t	scrollID;

	// ToDo index is 1 based....

	if (objectFocusMode)
	{
		if (sOneHandedTableHasFocus)
		{
			tableLineCount = (int16_t)((sCurrentNavObjectID == AgendaDatebookTable) ? sAgendaApptLineCount: sAgendaTodoLineCount);
			itemCount = (sCurrentNavObjectID == AgendaDatebookTable) ? sAgendaApptCount: sAgendaToDoCount + 1;
			scrollID = (sCurrentNavObjectID == AgendaDatebookTable) ? AgendaDatebookScroller : AgendaToDoScroller;
			tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, sCurrentNavObjectID));
			topItem = TblGetRowID (tableP, 0);

			if (sOneHandedRecordSelected)
			{
				delta = 0;
				scrolled = false;
				rowIndex = TblGetRowID (tableP, sOneHandedLastSelectedRow);

				// Is it possible to scroll ? Are we on the latest item ?
				if (rowIndex + 1 < itemCount)
				{
					rowIndex++;
					sOneHandedLastSelectedRow++;

					// Are we on the latest line of the table ?
					if (sOneHandedLastSelectedRow >= tableLineCount)
						delta = 1;
					else scrolled = true;
				}
			}
			else if (itemCount && (topItem + tableLineCount) < itemCount)
					delta = tableLineCount - 1;

			if (delta)
			{
				if (sCurrentNavObjectID == AgendaDatebookTable)
				{
					if ((scrolled = AgendaViewScroll (delta)) == true)
						AgendaViewSetScrollThumb (formP, scrollID, TopVisibleAppt);
				}
				else
				{
					if ((scrolled = ToDoListViewScroll (delta)) == true)
						AgendaViewSetScrollThumb (formP, scrollID, TblGetRowID (tableP, 0) - 1);
				}

				if (sOneHandedRecordSelected && !TblFindRowID(tableP, rowIndex, &sOneHandedLastSelectedRow))
					sOneHandedLastSelectedRow = 0;
			}

			if (sOneHandedRecordSelected)
			{
				column = (sCurrentNavObjectID == AgendaDatebookTable) ? -1 : sOneHandedCheckBoxSelected ? completedColumn : completedColumn + 2;
				AgendaViewDrawTableItemFocusRing(formP, tableP, sOneHandedLastSelectedRow, column);
				column = (sCurrentNavObjectID == AgendaDatebookTable) ?  agendaApptDescColumn : sOneHandedCheckBoxSelected ? completedColumn : completedColumn + 2;
				TblSelectItem (tableP, sOneHandedLastSelectedRow, column);
			}

			handled = scrolled || repeat;
		}
	}
	else
	{
		FrmSetFocus(formP, FrmGetObjectIndex(formP, AgendaToDoCategoryTrigger));
		FrmNavObjectTakeFocus(formP, AgendaToDoCategoryTrigger);
		handled = true;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:		AgendaViewHandleNavigationRight
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe 081104	Creation.
 *
 ***********************************************************************/
static Boolean AgendaViewHandleNavigationRight(FormType* formP, Boolean objectFocusMode, Boolean repeat)
{
	if (sOneHandedTableHasFocus && sOneHandedRecordSelected)
	{
		if (sCurrentNavObjectID == AgendaToDoTable)
		{
			if (sOneHandedCheckBoxSelected)
			{
				TablePtr	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, AgendaToDoTable));

				sOneHandedCheckBoxSelected = false;
				AgendaViewDrawTableItemFocusRing(formP, tableP, sOneHandedLastSelectedRow, completedColumn + 2);
				TblSelectItem (tableP, sOneHandedLastSelectedRow, completedColumn + 2);
				return true;
			}
			else
			{
				sOneHandedRecordSelected = false;

				FrmSetFocus(formP, FrmGetObjectIndex(formP, AgendaGoToButton));
				FrmNavObjectTakeFocus(formP, AgendaGoToButton);
				return true;
			}
		}
	}
	else
	{
		if (!objectFocusMode || sOneHandedTableHasFocus)
		{
			DateAdjust (&Date, 1);
			AgendaViewChangeDate (formP, Date, true);
			return true;
		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:		AgendaViewHandleNavigationLeft
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe 081104	Creation.
 *
 ***********************************************************************/
static Boolean AgendaViewHandleNavigationLeft(FormType* formP, Boolean objectFocusMode, Boolean repeat)
{
	if (sOneHandedTableHasFocus && sOneHandedRecordSelected)
	{

		if ((sCurrentNavObjectID == AgendaDatebookTable) || ((sCurrentNavObjectID == AgendaToDoTable) && sOneHandedCheckBoxSelected))
		{
			sOneHandedRecordSelected = false;
			// Give the focus back to the whole table.
			FrmSetFocus(formP, FrmGetObjectIndex(formP, sCurrentNavObjectID));
			FrmNavObjectTakeFocus(formP, sCurrentNavObjectID);
			return true;
		}
		else
		{
			if (!sOneHandedCheckBoxSelected)
			{
				TablePtr	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, AgendaToDoTable));

				sOneHandedCheckBoxSelected = true;
				AgendaViewDrawTableItemFocusRing(formP, tableP, sOneHandedLastSelectedRow, completedColumn);
				TblSelectItem (tableP, sOneHandedLastSelectedRow, completedColumn);
				return true;
			}
		}
	}
	else
	{
		if (!objectFocusMode || sOneHandedTableHasFocus)
		{
			DateAdjust (&Date, -1);
			AgendaViewChangeDate (formP, Date, true);
			return true;
		}
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:		AgendaViewHandleNavigationCenter
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	LFe 081104	Creation.
 *
 ***********************************************************************/
static Boolean AgendaViewHandleNavigationCenter(FormType* formP, Boolean objectFocusMode, Boolean repeat)
{
	TablePtr		tableP;
	uint32_t		rowID = dbInvalidRowID;
	uint16_t		apptIndex = noAppointments;
	int16_t 		on;
	uint16_t		rowIndex;
	Boolean			overlapState = false;
	ApptInfoType* 	apptsP;

	if (repeat)
		return false;

	if (!objectFocusMode)
		return false;

	if (objectFocusMode && (sCurrentNavObjectID == AgendaAgendaViewButton))
	{
		// force interaction mode to the 'Agenda Event Table' button.
		FrmSetFocus(formP, FrmGetObjectIndex(formP, AgendaDatebookTable));
		FrmNavObjectTakeFocus(formP, AgendaDatebookTable);
		return true;
	}

	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, sCurrentNavObjectID));

	if (sOneHandedTableHasFocus)
	{
		if (!sOneHandedRecordSelected)
		{
			sOneHandedLastSelectedRow = 0;

			if (((sCurrentNavObjectID == AgendaDatebookTable) && sAgendaApptCount) ||
				((sCurrentNavObjectID == AgendaToDoTable) && sAgendaToDoCount))
			{
				int16_t	column1;
				int16_t	column2;

				column1 = (sCurrentNavObjectID == AgendaDatebookTable) ?  agendaApptDescColumn : completedColumn + 2;
				column2 = (sCurrentNavObjectID == AgendaDatebookTable) ? -1 : completedColumn + 2;

				sOneHandedRecordSelected = true;
				sOneHandedCheckBoxSelected = false;
				AgendaViewDrawTableItemFocusRing(formP, tableP, sOneHandedLastSelectedRow, column2);
				TblSelectItem (tableP, sOneHandedLastSelectedRow, column1);
				return true;
			}
		}

		switch (sCurrentNavObjectID)
		{
			case AgendaDatebookTable:
				apptIndex = TblGetRowID (tableP, sOneHandedLastSelectedRow);

				if (apptIndex != noAppointments)
				{
					// Get the record index of the next appointment
					if (sAgendaApptsH)
					{
						apptsP = MemHandleLock (sAgendaApptsH);

						rowID = apptsP[apptIndex].rowID;
						overlapState = apptsP[apptIndex].overlapState;

						MemPtrUnlock (apptsP);
					}
				}

				GoToAppointment (ApptDB, rowID, overlapState);
				break;

			case AgendaToDoTable:
				if (sOneHandedCheckBoxSelected)
				{
					RectangleType	cellRect;
					int32_t			oldCount = sAgendaToDoCount;

					on = TblGetItemInt (tableP, sOneHandedLastSelectedRow, completedColumn);
					TblGetItemBounds(tableP, sOneHandedLastSelectedRow, completedColumn, &cellRect);
					rowIndex = TblGetRowID (tableP, sOneHandedLastSelectedRow);

					TblMarkRowInvalid(tableP, sOneHandedLastSelectedRow);

					ToDoListViewChangeCompleteStatus (sOneHandedLastSelectedRow, !on);

					if (oldCount != sAgendaToDoCount)
					{
						if (sAgendaToDoCount)
						{
							if (sOneHandedLastSelectedRow >= sAgendaTodoLineCount)
								sOneHandedLastSelectedRow--;
						}
						else
						{
							sOneHandedCheckBoxSelected = false;
							sOneHandedRecordSelected = false;

							// force interaction mode to the 'ToDo Event Table' button.
							FrmSetFocus(formP, FrmGetObjectIndex(formP, AgendaToDoTable));
							FrmNavObjectTakeFocus(formP, AgendaToDoTable);
						}
					}
					else AgendaViewDisplayInvalidate(formP, &cellRect);
				}
				else
				{
					rowID = sAgendaToDoCount ? TblGetRowData (tableP, sOneHandedLastSelectedRow) : dbInvalidRowID;
					LaunchToDoWithRecord (rowID);
				}
				break;
		}

		return true;
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:		AgendaViewHandleNavigation
 *
 * DESCRIPTION: 	Handle Virtual keys events.
 *					Particularly navigation events.
 *					(up and down keys and 5way events)
 *
 * PARAMETERS:	->	modifiers:		Command mofifiers.
 *				-> 	chr:			Command code.
 *
 * RETURNED:		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 06/16/04	Creation.
 * 	PPL 06/16/04	Add PalmSource OneHanded Specifications.
 *
 ***********************************************************************/
Boolean AgendaViewHandleNavigation( uint16_t modifiers, wchar32_t chr)
{
	FormPtr		formP = FrmGetFormPtr(AgendaView);
	Boolean 	handled = false;
	Boolean		objectFocusMode;
	Boolean		repeat;
	FrmNavStateFlagsType	navState;

	TraceOutput(TL(appErrorClass, "AgendaViewHandleNavigation start"));

	objectFocusMode = (Boolean) (FrmGetNavState(formP, &navState) >= errNone && (navState & kFrmNavStateFlagsObjectFocusMode) != 0);
	repeat = modifiers & autoRepeatKeyMask;

	switch(chr)
	{
		case vchrRockerUp:
			handled = AgendaViewHandleNavigationUp(formP, objectFocusMode, repeat);
			break;

		case vchrRockerDown:
			handled = AgendaViewHandleNavigationDown(formP, objectFocusMode, repeat);
			break;

		case vchrThumbWheelDown:
		case vchrRockerLeft:
			handled = AgendaViewHandleNavigationLeft(formP, objectFocusMode, repeat);
			break;

		case vchrThumbWheelUp:
		case vchrRockerRight:
			handled = AgendaViewHandleNavigationRight(formP, objectFocusMode, repeat);
			break;

		case vchrThumbWheelPush:
		case vchrRockerCenter:
			handled = AgendaViewHandleNavigationCenter(formP, objectFocusMode, repeat);
			break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewUpdateDisplay
 *
 * DESCRIPTION: 	Draw form in response to an update event.
 *
 * PARAMETERS:  ->	formP: form pointer;
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLE	12/11/02	Initial Revision.
 *	PPL	10/07/03	Change name; Avoid drawing separation line twice
 *					(It is part of the gadget draw callback.)
 ***********************************************************************/
static void AgendaViewUpdateDisplay (FormPtr formP)
{
	PIMAppProfilingBegin("AgendaViewUpdateDisplay");

	FrmDrawForm (formP);

	if (sOneHandedTableHasFocus)
	{
		TablePtr	tableP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, sCurrentNavObjectID));
		int16_t		column2 = (sCurrentNavObjectID == AgendaDatebookTable) ? -1 : sOneHandedCheckBoxSelected ? completedColumn : completedColumn + 2;

		AgendaViewDrawTableItemFocusRing(formP, tableP, sOneHandedLastSelectedRow, column2);
	}

	PIMAppProfilingEnd();
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewUpdateDisplayInvalidateCallback
 *
 * DESCRIPTION: 	Callback for WinInvalidateRectFunc().
 *
 * PARAMETERS:  ->	cmd: 			form pointer.
 *				->	window:			window handle.
 *				->	dirtyRect:		zone to invalidate.
 *				->	state:			a user parameter

 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/07/03	Initial revision.
 *
 ***********************************************************************/
static Boolean AgendaViewUpdateDisplayCallback (int32_t cmd, WinHandle window, const RectangleType *dirtyRect, void *state)
{
	if (cmd == winInvalidateDestroy)
		return true;

	AgendaViewUpdateDisplay (*((FormType**) &state));

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewDisplayInvalidate
 *
 * DESCRIPTION: 	Invalids the given dirty rectangle for the given form.
 *					the form have to be Aganda View...
 *					(will see how to get rid of it)
 *
 * PARAMETERS:  ->	formP: 		form pointer;
 *				->	dirtyRect:	rectangle to invalidate;
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/07/03	Initial revision.
 *
 ***********************************************************************/
void AgendaViewDisplayInvalidate(FormPtr formP, RectangleType* dirtyRect)
{
	RectangleType 	fullWindowBounds;
	RectangleType* 	dirtyP;

	if (dirtyRect == NULL)
	{
		WinGetBounds(gWinHdl, &fullWindowBounds);
		dirtyP = &fullWindowBounds;
	}
	else
		dirtyP = dirtyRect;

	// transparency
	//WinInvalidateRectFunc(gWinHdl, dirtyP, AgendaViewUpdateDisplayCallback, (void*) formP);
	WinInvalidateRect(gWinHdl, dirtyP);

}


/***********************************************************************
 *
 * FUNCTION:    	AgendaViewChangeDate
 *
 * DESCRIPTION: 	This routine displays the date passed.
 *
 * PARAMETERS:  ->	inFormP:		form
 *				->	inDate:			date to display
 *				->	closeCursor: 	do we close the cusor
 *				->	redraw:			do we redraw
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			Closing the cursor is required when we change the category
 *					whithout changing the date. the ApptGetAppointments update
 *					the cursor when the date changed or when the cursor was
 *					closed and has to be re-opened.
 *
 * REVISION HISTORY:
 *	art	6/29/95		Initial Revision.
 *
 ***********************************************************************/
static void AgendaViewChangeDate (FormType* inFormP, DateType inDate, Boolean redraw)
{
	uint16_t	oldMonth;

	oldMonth = Date.month;

	// Keep the new date for future reference
	Date = inDate;
	sAgendaTimeSeconds = TimGetSeconds ();

	// Get correct coloring for the month
	if(sBackgroundMonth != Date.month)
	{
		WinHandle		winH;
		RectangleType	rect;
		DateBackgroundGetHighlightColor(&sBackgroundHighlight, Date.month);
		sGadgetCallbackData.color = sBackgroundHighlight;
		winH = FrmGetWindowHandle(inFormP);
		WinGetBounds(winH, &rect);
		WinInvalidateRect(winH, &rect);
		sBackgroundMonth = Date.month;
	}

	// Find out how many appointments and tasks to display
	AgendaLoadAppointments (inDate);		// sets sAgendaApptCount

	// Get the number of displayable tasks
	AgendaUpdateToDoCount();

	AgendaViewDrawDate (inFormP);
	AgendaViewDrawTime (inFormP);

	// Get all the appointments and empty time slots on the new day.
	//AgendaViewEraseObject (inFormP, AgendaDatebookTable);
	//AgendaViewEraseObject (inFormP, AgendaToDoTable);
	AgendaViewLayout (inFormP, sAgendaApptCount, sAgendaToDoCount);

	// Determine the top items to display
	AgendaViewSetTopAppointment ();

	// Fill the display tables
	AgendaViewFillAppointments (inFormP);
	AgendaViewFillTasks ();

	// Update the scroll arrows
	AgendaViewSetScrollThumb (inFormP, AgendaDatebookScroller, TopVisibleAppt);
	AgendaViewSetScrollThumb (inFormP, AgendaToDoScroller, 0);

	// Draw the new day's events
	FrmSetControlGroupSelection (inFormP, AgendaViewGroup, AgendaAgendaViewButton);

	if (redraw)
		AgendaViewDisplayInvalidate(inFormP, NULL);
}

/***********************************************************************
 *
 * FUNCTION:    	AgendaViewHandleEvent
 *
 * DESCRIPTION: 	This routine is the event handler for the Agenda View
 *              	of the Datebook application.
 *
 * PARAMETERS:  ->	eventP:	a pointer to an EventType structure.
 *
 * RETURNED:    	true if the event was handled and should not be passed
 *              	to a higher level handler.
 *
 * NOTE:
 *
 * REVISION HISTORY:
 *	ART	4/8/96		Initial revision.
 *	RBB	6/2/99		Added button for Agenda view.
 *	RBB	11/15/99 	Refresh clock, as needed, on nil events.
 *	RBB	11/15/99	Enhanced usage of the Datebook hard key.
 *	PPL 10/07/03 	Ehanced variable naming.
 *	PPL 03/10/04	Lunar Calendar from Palm OS 5.4
 *
 ***********************************************************************/
Boolean AgendaViewHandleEvent (EventType*	 eventP)
{
	FormType*					formP;
	ApptInfoType* 				apptsP;
	uint32_t 					rowID = dbInvalidRowID;
	uint32_t 					now;
	uint16_t 					apptIndex;
	Boolean 					handled = false;
	int16_t 					delta;
	int16_t 					on;
	MidnightOverlapStateType 	overlapState = overlapNone;
	wchar32_t					chr;
	uint16_t					modifiers;


	// Trace the event in debug mode
	DBGTraceEvent("AgendaViewHandleEvent", eventP);

	switch (eventP->eType)
	{
		case nilEvent:
			// Refresh the clock, as needed
			now = TimGetSeconds ();
			if ((now / minutesInSeconds) != (sAgendaTimeSeconds / minutesInSeconds))
			{
				sAgendaTimeSeconds = now;
				AgendaViewDrawTime (FrmGetActiveForm ());
			}
			break;


		case keyDownEvent:

			chr 		= eventP->data.keyDown.chr;
			modifiers	= eventP->data.keyDown.modifiers;

			if (EvtKeydownIsVirtual(eventP))
			{
				if (TxtCharIsRockerKey(modifiers, chr) || TxtCharIsWheelKey(modifiers, chr))
				{
					handled = AgendaViewHandleNavigation(modifiers, chr);
				}
				else if (TxtCharIsHardKey(modifiers, chr))
				{
					// Datebook key pressed? Only the Datebook button will reach this point.
					// If a date other than today is being viewed, or if the user powered up
					// by pressing the Datebook hard key, show today in the agenda view
					if (DateToInt (sToday) != DateToInt (Date)
					||((eventP->data.keyDown.modifiers & poweredOnKeyMask)))
						AgendaViewChangeDate (FrmGetActiveForm (), sToday, true);

					else
						// otherwise, cycle through to the day view
						FrmGotoForm(gApplicationDbP, DayView);

					handled = true;
				}
/*				else
				{
					switch (chr)
					{
						case vchrPageUp:
							//formP = FrmGetActiveForm ();
							//DateAdjust (&Date, -1);
							//AgendaViewChangeDate (formP, Date, true);
							//handled = true;
							break;

						case vchrPageDown:
							//formP = FrmGetActiveForm ();
							//DateAdjust (&Date, +1);
							//AgendaViewChangeDate (formP, Date, true);
							//handled = true;
							break;
					}
				}
*/
			}
			break;


		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case AgendaCategoryTrigger:
					AgendaViewSelectCategory ();
					handled = true;
					break;

				case AgendaGoToButton:
				case AgendaCurrentDayButton:
					AgendaViewGoToDate ();
					handled = true;
					break;

				case AgendaDayViewButton:
					FrmGotoForm(gApplicationDbP, DayView);
					handled = true;
					break;

				case AgendaWeekViewButton:
					FrmGotoForm(gApplicationDbP, WeekView);
					handled = true;
					break;

				case AgendaMonthLunarViewButton:
					// Lunar Calendar
					MonthViewSetLunarCalendar(MonthMonthLunarViewButton);

					FrmGotoForm (gApplicationDbP, MonthView);
					handled = true;
					break;

				case AgendaMonthViewButton:
					// Lunar Calendar
					MonthViewSetLunarCalendar(MonthMonthViewButton);

					FrmGotoForm(gApplicationDbP, MonthView);
					handled = true;
					break;

				case AgendaToDoCategoryTrigger:
					ToDoListViewSelectCategory ();
					handled = true;
					break;

				default:;
			}
			break;


		case ctlRepeatEvent:
			switch (eventP->data.ctlRepeat.controlID)
			{
				case AgendaPreviousDayButton:
					formP = FrmGetActiveForm ();
					DateAdjust (&Date, -1);
					AgendaViewChangeDate (formP, Date, true);
					break;

				case AgendaNextDayButton:
					formP = FrmGetActiveForm ();
					DateAdjust (&Date, +1);
					AgendaViewChangeDate (formP, Date, true);
					break;

				default:;
			}
			break;


		case sclRepeatEvent:
			delta = (int16_t) (eventP->data.sclRepeat.newValue - eventP->data.sclRepeat.value);
			if (delta != 0)
			{
				switch (eventP->data.sclRepeat.scrollBarID)
				{
					case AgendaToDoScroller:
						ToDoListViewScroll (delta);
						break;
					case  AgendaDatebookScroller:
						AgendaViewScroll (delta);
						break;
				}
			}
			break;


		case tblSelectEvent:
			// An item in the table has been selected.
			switch (eventP->data.tblEnter.tableID)
 			{
 				case AgendaDatebookTable:
					apptIndex = TblGetRowID (eventP->data.tblEnter.pTable, eventP->data.tblEnter.row);

					if (apptIndex != noAppointments)
					{
						// Get the record index of the next appointment
						if (sAgendaApptsH)
						{
							apptsP = MemHandleLock (sAgendaApptsH);

							rowID = apptsP[apptIndex].rowID;
							overlapState = apptsP[apptIndex].overlapState;

							MemPtrUnlock (apptsP);
						}
					}

					GoToAppointment (ApptDB, rowID, overlapState);
					break;

 				case AgendaToDoTable:
					if (eventP->data.tblEnter.column == completedColumn)
					{
						on = TblGetItemInt (	eventP->data.tblEnter.pTable,
												eventP->data.tblEnter.row,
												eventP->data.tblEnter.column);

						ToDoListViewChangeCompleteStatus (eventP->data.tblEnter.row, on);
					}
					else
					{
						rowID = TblGetRowData (eventP->data.tblEnter.pTable, eventP->data.tblEnter.row);
						LaunchToDoWithRecord (rowID);
					}
					break;
			}
			break;

		case winUpdateEvent:
			if (gWinHdl != eventP->data.winUpdate.window)
				break;

			formP = FrmGetActiveForm ();
			AgendaViewUpdateDisplay(formP);
			handled = true;
			break;


		case winResizedEvent:
			// Active Input Area handling
			if (gWinHdl != eventP->data.winResized.window)
				break;

			AgendaViewResize(eventP);
			handled = true;
			break;


		case frmOpenEvent:
			formP = FrmGetActiveForm ();
			AgendaViewInit (formP);
			AgendaViewChangeDate (formP, Date, false);
			handled = true;
			break;


		case  frmCloseEvent:
			AgendaFreeAppointments ();
			ToDoViewFinalize ();
			break;


		case frmObjectFocusTakeEvent:
			formP = FrmGetActiveForm ();
			sCurrentNavObjectID = eventP->data.frmObjectFocusTake.objectID;

			if ((eventP->data.frmObjectFocusTake.objectID == AgendaDatebookTable) ||
				(eventP->data.frmObjectFocusTake.objectID == AgendaToDoTable))
			{
				RectangleType	bounds;
				uint16_t		objIndex;

				sOneHandedTableHasFocus = true;

				if (!sOneHandedRecordSelected)
				{
					formP = FrmGetFormPtr(AgendaView);
					objIndex = FrmGetObjectIndex(formP, sCurrentNavObjectID);

					TblUnhighlightSelection(FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, sCurrentNavObjectID)));

					FrmSetFocus(formP, objIndex);
					FrmGetObjectBounds(formP, objIndex, &bounds);
					FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
				}

				handled = true;
			}
			break;

		case frmObjectFocusLostEvent:
			if ((eventP->data.frmObjectFocusTake.objectID == AgendaDatebookTable) ||
				(eventP->data.frmObjectFocusTake.objectID == AgendaToDoTable))
			{
				//int16_t	tableRowIndex;
				formP = FrmGetFormPtr(AgendaView);

				sOneHandedTableHasFocus = false;
				if (sOneHandedRecordSelected)
				{
					TblUnhighlightSelection(FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, eventP->data.frmObjectFocusTake.objectID)));
					sOneHandedRecordSelected = false;
				}
			}
			break;

		case datebookRefreshDisplay:
			formP = FrmGetActiveForm ();
			AgendaViewChangeDate(formP, Date, true);
			break;

	}

	return (handled);
}
