/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SelectTime.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  		New time selector allowing appointments spanning on midnight
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <Control.h>
#include <Chars.h>
#include <DateTime.h>
#include <Event.h>
#include <Form.h>
#include <List.h>
#include <Preferences.h>
#include <PenInputMgr.h>
#include <Rect.h>
#include <string.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <TimeMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>

#include "DateGlobals.h"
#include "DateUtils.h"
#include "GUIUtilities.h"

#include "SelectTime.h"
#include "SelectTimeRsc.h"



//*********************************************************
// Macro for detecting if character is a rocker character
//*********************************************************

// <c> is a rocker key if the event modifier <m> has the command bit set
// and <c> is in the proper range
#define	TxtCharIsWheelKey(m, c)		((((m) & commandKeyMask) != 0) && \
									((((c) >= vchrThumbWheelUp) && ((c) <= vchrThumbWheelBack))))



/***********************************************************************
 *
 *	Local definitions
 *
 ***********************************************************************/
#define 	hoursInHalfDay				12
#define 	minutesIncrement			5
#define 	maxEventDurationInSecs		(daysInSeconds - 5 * minutesInSeconds)
#define 	dayNameStringLength			10
#define 	spaceAfterTimeItemInList 	1
#define		gadgetLineHeight			4
#define		gadgetShadeWidthPad			4


enum TimeDisplayTag
{
	tampmDisplay,
	t24hDisplay
};

typedef Enum8 TimeDisplayType ;



/***********************************************************************
 *
 *	Local static variables
 *
 ***********************************************************************/
static TimeDisplayType 			sDisplaySetting;
static TimeFormatType 			sTimeFormat;
static TimeSelectionType		sSelectionSetting;
static DateTimeType 			sStartDate;
static DateTimeType 			sEndDate;
static int16_t 					sStartOfDay;
static int16_t 					sEndOfDay;
static Boolean					sLimitedToSameDay;
static char						sGraffitiStr[12];
static FormType * 				sFormP;
static int16_t					gadgetOffsetWidth = 7;
static char 					sStartTimeLabel[timeStringLength*2]= {0}; // timeStringLength = 15
static char 					sEndTimeLabel[timeStringLength*2]= {0};

static RGBColorType				bottomBackColor = { 0, 233, 233, 233 };
static RGBColorType				bottomForeColor = { 0, 140, 140, 140 };
static RGBColorType				rightBackColor = { 0, 140, 140, 140 };

/***********************************************************************
 *
 *	Local Functions
 *
 ***********************************************************************/
static void PrvSwitchAMPM(int16_t gap);
static void PrvDrawDatesAndTimes(Boolean invalidate);
static void PrvUpdateBarsSelection(void);
static void PrvUpdateTriggers(void);
static void PrvCheckDatesConsistency(void);
static void PrvAssignListDrawingCallbacks(void);
static void PrvUpdateAllDayWorkHourField(
	FormType*			formP,				
	int16_t 			startOfDay, 
	int16_t 			endOfDay,
	Boolean 			invalidate);

Boolean PrvSelectTimeBottomGadgetHandler(struct FormGadgetTypeInCallback *gadgetP,
										 uint16_t cmd, void *paramP)
{
	RGBColorType	oldBackColor;
	RGBColorType	oldForeColor;

	if(cmd == formGadgetDrawCmd)
	{
		GcGradientType	grad;
		GcPointType		pnt[2];
		GcColorType		col[2];
		GcHandle		gc;
		RectanglePtr	bounds;

		bounds = &gadgetP->rect;

		pnt[0].x = (fcoord_t)(bounds->topLeft.x);
		pnt[0].y = (fcoord_t)(bounds->topLeft.y);
		pnt[1].x = (fcoord_t)(bounds->topLeft.x + bounds->extent.x);
		pnt[1].y = pnt[0].y;

		col[0].red = col[1].red = 180;
		col[0].green = col[1].green = 180;
		col[0].blue = col[1].blue = 180;
		col[0].alpha = 255;
		col[1].alpha = 0;

		gc = GcGetCurrentContext();

		GcInitGradient(&grad, pnt, col, 2);
		GcSetGradient(gc, &grad);

		GcSetDithering(gc, true);

		GcRect(gc, pnt[0].x, pnt[0].y, pnt[1].x, pnt[1].y + bounds->extent.y);
		GcPaint(gc);

		GcReleaseContext(gc);
		
		WinSetBackColorRGB(&bottomBackColor, &oldBackColor);
		WinSetForeColorRGB(&bottomForeColor, &oldForeColor);
		//WinEraseRectangle(&gadgetP->rect, 0);
		WinDrawLine(gadgetP->rect.topLeft.x, gadgetP->rect.topLeft.y,
					gadgetP->rect.topLeft.x + gadgetP->rect.extent.x,
					gadgetP->rect.topLeft.y);
		WinSetBackColorRGB(&oldBackColor, NULL);
		WinSetForeColorRGB(&oldForeColor, NULL);

		return true;
	}
	return false;
}

Boolean PrvSelectTimeRightGadgetHandler(struct FormGadgetTypeInCallback *gadgetP,
										uint16_t cmd, void *paramP)
{
	FormType		*formP;
	RectangleType	bounds;
	RectangleType	triggerBounds;
	RGBColorType	oldBackColor;
	uint16_t		itemId = 0;

	if(cmd == formGadgetDrawCmd)
	{
		formP = FrmGetActiveForm();
		if(CtlGetValue(UtilitiesGetFormObjectPtr(formP, SelectTimeStartTimeTrigger)))
			itemId = SelectTimeStartTimeTrigger;
		else if(CtlGetValue(UtilitiesGetFormObjectPtr(formP, SelectTimeEndTimeTrigger)))
			itemId = SelectTimeEndTimeTrigger;

		WinSetBackColorRGB(&rightBackColor, &oldBackColor);
		bounds = gadgetP->rect;
		bounds.topLeft.x += gadgetOffsetWidth;
		bounds.extent.x -= gadgetOffsetWidth;
		WinEraseRectangle(&bounds, 2);
		if(itemId)
		{
			FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, itemId), &triggerBounds);
			bounds = gadgetP->rect;
			bounds.extent.x = gadgetOffsetWidth;
			bounds.extent.y = gadgetLineHeight;
			bounds.topLeft.y = triggerBounds.topLeft.y +
				(triggerBounds.extent.y - gadgetLineHeight) / 2;
			WinEraseRectangle(&bounds, 0);
		}
		WinSetBackColorRGB(&oldBackColor, NULL);
		return true;
	}
	return false;
}

static void PrvAssignGadgetDrawingCallbacksAndColors(FormType *formP)
{
	RectangleType gadgetBounds, hoursBounds;

	// Make the buttons transparent so that they don't erase a square
	// of the gray background
	FrmSetTransparentObjects(formP, true);

	FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, SelectTimeBottomGadget),
						PrvSelectTimeBottomGadgetHandler);
						
	FrmSetGadgetHandler(formP, FrmGetObjectIndex(formP, SelectTimeRightGadget),
						PrvSelectTimeRightGadgetHandler);

	FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, SelectTimeRightGadget), &gadgetBounds);

	switch(sDisplaySetting)
	{
		case tampmDisplay:
			FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, SelectTimeHoursListAmPm), &hoursBounds);
			break;
		default:
			FrmGetObjectBounds(formP, FrmGetObjectIndex(formP, SelectTimeHoursList_1_24Hrs), &hoursBounds);
	}

	gadgetOffsetWidth = hoursBounds.topLeft.x - gadgetBounds.topLeft.x - gadgetShadeWidthPad;
}

/***********************************************************************
 *
 * FUNCTION:    	PrvDialogInit
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	formP:
 *				->	startDateTimeP:
 *				->	endDateTimeP:
 *				->	initialSelection:
 *				->	limitedToTheSameDay:
 *				->	startOfDay:
 *				->	endOfDay:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvDialogInit(
	FormType* 				formP, 
	DateTimeType* 			startDateTimeP, 
	DateTimeType* 			endDateTimeP,	
	TimeSelectionType 		initialSelection,
	Boolean 				limitedToTheSameDay, 
	int16_t 				startOfDay, 
	int16_t 				endOfDay)
{ 
	DateTimeType			todayDateTime;
	DateType				startDate, todayDate;
	TimeType				startDayTime = {0, 0};
	TimeType				endDayTime = {0, 0}; 

	
	// Dialog defaults
	sLimitedToSameDay = limitedToTheSameDay;
	sSelectionSetting = initialSelection;
	sStartOfDay = startOfDay;
	sEndOfDay = endOfDay;
	*sGraffitiStr = '\0';

	switch(sSelectionSetting)
	{
		case tSelectStart:
			sStartDate = *startDateTimeP;
			sEndDate = *endDateTimeP;
			// Check consistency according to the dialog behavior
			PrvCheckDatesConsistency();
			break;
			
		case tSelectEnd:
			sStartDate = *startDateTimeP;
			sEndDate = *endDateTimeP;
			// Check consistency according to the dialog behavior
			PrvCheckDatesConsistency();
			break;
						
		case tSelectNoTime:
			// If we're today, the time is next hour
			TimSecondsToDateTime(TimGetSeconds (), &todayDateTime);
			todayDate = CnvDateTime2Date(&todayDateTime);
			startDate = CnvDateTime2Date(startDateTimeP);
			if (DateToInt(startDate) == DateToInt(todayDate))
			{
				// It's today !
				startDayTime.hours = (uint8_t) todayDateTime.hour;
				if ((todayDateTime.minute > 0) && (startDayTime.hours < (hoursPerDay-1)))
					startDayTime.hours++;
			}
			else
			{
				// Another day : use sStartOfDay
				startDayTime.hours = (uint8_t) sStartOfDay;
			}
			CnvDate2DateTime(&startDate, &startDayTime, &sStartDate);
			sEndDate = sStartDate;
			// Increase the end date by one hour
			TimAdjust(&sEndDate, hoursInSeconds);
			break;
			
		case tSelectAllDay:
			startDate = CnvDateTime2Date(startDateTimeP);
			startDayTime.hours = (uint8_t)sStartOfDay;
			endDayTime.hours = (uint8_t)sEndOfDay;
			CnvDate2DateTime(&startDate, &startDayTime, &sStartDate);
			CnvDate2DateTime(&startDate, &endDayTime, &sEndDate);
			break;
	}
	
	// Get the time format from the system preferences;
	sTimeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);	
	switch (sTimeFormat)
	{
		case tfColon:
		case tfColonAMPM:
		case tfDot:
		case tfDotAMPM:
		case tfHoursAMPM:
			sDisplaySetting = tampmDisplay;
			break;
			
		case tfColon24h:
		case tfDot24h:
		case tfHours24h:
		case tfComma24h:
			sDisplaySetting = t24hDisplay;
			break;

		default:
			sDisplaySetting = tampmDisplay;
			break;
	}

	// Now, display the correct time columns
	switch (sDisplaySetting)
	{
		case tampmDisplay:
			FrmShowObject(formP, FrmGetObjectIndex(formP, SelectTimeHoursListAmPm));
			FrmShowObject(formP, FrmGetObjectIndex(formP, SelectTimeMinutesListAmPm));
			FrmShowObject(formP, FrmGetObjectIndex(formP, SelectTimeMiddleDayAmPm));
			break;
		case t24hDisplay:
			FrmShowObject(formP, FrmGetObjectIndex(formP, SelectTimeHoursList_1_24Hrs));
			FrmShowObject(formP, FrmGetObjectIndex(formP, SelectTimeHoursList_2_24Hrs));
			FrmShowObject(formP, FrmGetObjectIndex(formP, SelectTimeMinutesList24Hrs));
			break;
	}

	// Set the gadget drawing callbacks and set background colors for controls
	// on top of the backgrounds
	PrvAssignGadgetDrawingCallbacksAndColors(formP);

	// Set the drawing callbacks
	PrvAssignListDrawingCallbacks();
	
	// Init Fields
	FldSetTextHandle(UtilitiesGetFormObjectPtr(formP, SelectTimeStartTimeDateField), NULL );		
	FldSetTextHandle(UtilitiesGetFormObjectPtr(formP, SelectTimeEndTimeDateField), NULL );	
	FldSetTextHandle(UtilitiesGetFormObjectPtr(formP, SelectTimeAllDayWorkHoursField), NULL );		

	// Set the AllDay button field
	PrvUpdateAllDayWorkHourField(formP, startOfDay, endOfDay, false);
	
	// Update the time and date in fields
	PrvDrawDatesAndTimes(false);

	// Set selection in triggers and bars
	PrvUpdateBarsSelection();
	PrvUpdateTriggers();
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDrawDateCallBack
 *
 * DESCRIPTION: 	Callback for WinRectangleInvalidateFunc().
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	06/16/03	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDrawFieldCallBack (int32_t cmd, WinHandle window, const RectangleType *diryRect, void *state)
{
	FieldPtr 	fldP;
	uint16_t 	dateFieldID = *((uint16_t*) &state);
	
	if (cmd == winInvalidateDestroy)
		return true;

	fldP = FrmGetObjectPtr (sFormP, FrmGetObjectIndex (sFormP, dateFieldID));
	FldDrawField (fldP);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDrawTime
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	dateTimeP:
 *				->	timeControlID:
 *				->	dateFieldID:
 *				->	timeFormat:
 *				->	dateFormat:
 *				->	redraw:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 * 	PPL	23/10/03	Fix International Date+DayName String:
 *					Use DateToDOWDMFormat rather than DateToAscii
 *					to generate a correct day+date string in EFIGS 
 *					and Japanese. (external parenthesis are under discussion 
 *					as in Japanese we have <date><space><(><dayName><)>.)
 * PPL	02/27/04	Add laber buffer in parameter list.
 *
 ***********************************************************************/
static void PrvDrawTime(
	DateTimeType* 	dateTimeP, 
	uint16_t 		timeControlID, 
	uint16_t 		dateFieldID, 
	TimeFormatType 	timeFormat, 
	DateFormatType 	dateFormat,
	Boolean 		invalidate,
	char*			labelText)
{
	char 			fullDateStr[dowLongDateStrLength+3]; // Add 3 for parenthesis
	ControlType* 	timeControlP;
	FieldType* 		dateFieldP;
	MemHandle		noTimeStrH;
	char *			noTimeStrP;
	RectangleType 	fieldRect;


	timeControlP = UtilitiesGetFormObjectPtr (sFormP, timeControlID);
	dateFieldP = UtilitiesGetFormObjectPtr (sFormP, dateFieldID);

	// Display the time, or "No Time" if notime selected
	if (sSelectionSetting == tSelectNoTime)
	{
		// Get the localized text to print
		noTimeStrH = DmGetResource (gApplicationDbP, strRsc, selectTimeNoTimeStrID);
		noTimeStrP = MemHandleLock(noTimeStrH);
		
		strcpy(labelText, noTimeStrP);
		CtlSetLabel(timeControlP, labelText);
		
		// Release the resources
		MemHandleUnlock(noTimeStrH);
		DmReleaseResource(noTimeStrH);
	}
	else
	{
		TimeToAscii( (uint8_t) dateTimeP->hour, (uint8_t) dateTimeP->minute, timeFormat, labelText);
		CtlSetLabel(timeControlP, labelText);
	}

	DateToDOWDMFormat (
		(uint8_t) dateTimeP->month, 
		(uint8_t) dateTimeP->day, 
		(uint16_t) (dateTimeP->year), 
		dateFormat, 
		fullDateStr);
	
	// Make full string add surrounding parenthesis
	// Note: the parenthesis are not good for localization
	// sprintf(fullDateStr, "(%s)", dateStr );
	
	// Display full date string
	UtilitiesSetFieldPtrText(dateFieldP, fullDateStr, false);

	// Update date if needed
	if (invalidate)
	{
		FrmGetObjectBounds(sFormP, FrmGetObjectIndex (sFormP, dateFieldID), &fieldRect);
		
		// Transparency
		//WinInvalidateRectFunc(FrmGetWindowHandle(sFormP), &fieldRect, PrvDrawFieldCallBack, (void *)dateFieldID);
		WinInvalidateRect(FrmGetWindowHandle(sFormP), &fieldRect);
	}
}


 /***********************************************************************
 *
 * FUNCTION:    	PrvDrawDatesAndTimes
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	drawNow:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *	PPL	02/27/2004	Add buffer to PrvDrawTime calls.
 *
 ***********************************************************************/
static void PrvDrawDatesAndTimes(Boolean invalidate)
{
	DateFormatType dateFormat;

	dateFormat = (TimeFormatType)PrefGetPreference(prefDateFormat);	
	
	PrvDrawTime(
			&sStartDate, 
			SelectTimeStartTimeTrigger, 
			SelectTimeStartTimeDateField,
			sTimeFormat, 
			dateFormat, 
			invalidate,
			sStartTimeLabel);
		
	PrvDrawTime(
			&sEndDate, 
			SelectTimeEndTimeTrigger, 
			SelectTimeEndTimeDateField,
			sTimeFormat, 
			dateFormat, 
			invalidate,
			sEndTimeLabel);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvAssignListPtrs
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  <->	listHoursBar1PP:
 *				<->	listHoursBar2PP:
 *				<->	listMinBarPP:
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvAssignListPtrs(
	ListType** listHoursBar1PP, 
	ListType** listHoursBar2PP,
	ListType** listMinBarPP)
{
	switch (sDisplaySetting)
	{
		case tampmDisplay:
			*listHoursBar1PP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeHoursListAmPm);
			*listHoursBar2PP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeMiddleDayAmPm);
			*listMinBarPP 		= UtilitiesGetFormObjectPtr(sFormP, SelectTimeMinutesListAmPm);
			break;
			
		case t24hDisplay:
			*listHoursBar1PP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeHoursList_1_24Hrs);
			*listHoursBar2PP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeHoursList_2_24Hrs);
			*listMinBarPP 		= UtilitiesGetFormObjectPtr(sFormP, SelectTimeMinutesList24Hrs);
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvMinBarDrawListProc
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	itemNum:
 *				->	boundsP:
 *				->	itemsText:
 *				->	listP:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvMinBarDrawListProc (
	int16_t 		itemNum, 
	RectangleType *	boundsP, 
	char **			itemsText, 
	ListType *		listP)
{
	FontID 			oldFont;
	Coord 			x;

	// Draw in bold each 3 item
	if (itemNum % 3 == 0)
		oldFont = FntSetFont(boldFont);
	else
		oldFont = FntGetFont();
	
	x = boundsP->topLeft.x + boundsP->extent.x - spaceAfterTimeItemInList 
		- FntCharsWidth(itemsText[itemNum], strlen(itemsText[itemNum]));

	WinDrawChars(itemsText[itemNum], strlen(itemsText[itemNum]), x, boundsP->topLeft.y);

	FntSetFont(oldFont);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvHourBarDrawBoldListProc
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	itemNum:
 *				->	boundsP:
 *				->	itemsText:
 *				->	listP:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvHourBarDrawBoldListProc (
	int16_t 		itemNum, 
	RectangleType *	boundsP, 
	char **			itemsText, 
	ListType *		listP)
{
	FontID 			oldFont;
	Coord 			x;

	// Draw in bold each 3 item
	if (itemNum % 3 == 0)
		oldFont = FntSetFont(boldFont);
	else
		oldFont = FntGetFont();

	x = boundsP->topLeft.x + boundsP->extent.x - spaceAfterTimeItemInList 
		- FntCharsWidth(itemsText[itemNum], strlen(itemsText[itemNum]));

	WinDrawChars(itemsText[itemNum], strlen(itemsText[itemNum]), x, boundsP->topLeft.y);

	FntSetFont(oldFont);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvHourBarDrawListProc
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	itemNum:
 *				->	boundsP:
 *				->	itemsText:
 *				->	listP:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvHourBarDrawListProc (
	int16_t 		itemNum, 
	RectangleType*	boundsP, 
	char**			itemsText, 
	ListType*		listP)
{
	Coord 			x;

	x = boundsP->topLeft.x + boundsP->extent.x - spaceAfterTimeItemInList 
		- FntCharsWidth(itemsText[itemNum], strlen(itemsText[itemNum]));

	WinDrawChars(itemsText[itemNum], strlen(itemsText[itemNum]), x, boundsP->topLeft.y);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvAssignListDrawingCallbacks
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvAssignListDrawingCallbacks(void)
{
	ListType* listHoursBar1P = NULL;
	ListType* listHoursBar2P = NULL;
	ListType* listMinBarP = NULL;

	// Get the lists pointers
	PrvAssignListPtrs(&listHoursBar1P, &listHoursBar2P, &listMinBarP);

	// And assign the callbacks
	LstSetDrawFunction(listHoursBar1P, PrvHourBarDrawBoldListProc);
	
	if (sDisplaySetting == tampmDisplay)
		LstSetDrawFunction(listHoursBar2P, PrvHourBarDrawListProc);
	else
		LstSetDrawFunction(listHoursBar2P, PrvHourBarDrawBoldListProc);

	LstSetDrawFunction(listMinBarP, PrvMinBarDrawListProc);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvUpdateBarsSelection
 *
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	dateTimeP:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvUpdateBarSel(DateTimeType* dateTimeP)
{
	ListType* 	listHoursBar1P = NULL;
	ListType* 	listHoursBar2P = NULL;
	ListType* 	listMinBarP = NULL;
	int16_t 	itemHoursBar1 = 0;
	int16_t 	itemHoursBar2 = 0;
	int16_t 	itemMinBar = 0;

	PrvAssignListPtrs(&listHoursBar1P, &listHoursBar2P, &listMinBarP);

	switch(sSelectionSetting)
	{
		case tSelectNoTime:
		case tSelectAllDay:
			itemHoursBar1 = noListSelection;
			itemHoursBar2 = noListSelection;
			itemMinBar = noListSelection;
			break;

		case tSelectStart:
		case tSelectEnd:
			switch (sDisplaySetting)
			{
				case tampmDisplay:
					itemHoursBar1 = dateTimeP->hour % hoursInHalfDay;
					itemHoursBar2 = dateTimeP->hour / hoursInHalfDay;
					break;
					
				case t24hDisplay:
					itemHoursBar1 = (dateTimeP->hour < hoursInHalfDay) ? dateTimeP->hour : noListSelection ;
					itemHoursBar2 = (dateTimeP->hour >= hoursInHalfDay) ? 
						dateTimeP->hour - hoursInHalfDay : noListSelection;
					break;
			}		
			itemMinBar = dateTimeP->minute / minutesIncrement;
			break;
	}

	LstSetSelection(listHoursBar1P, itemHoursBar1);
	LstSetSelection(listHoursBar2P, itemHoursBar2);
	LstSetSelection(listMinBarP, itemMinBar);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvUpdateAllDayWorkHourField
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	09/15/2003	Initial revision.
 *
 ***********************************************************************/
static void PrvUpdateAllDayWorkHourField(
	FormType*			formP,				
	int16_t 			startOfDay, 
	int16_t 			endOfDay,
	Boolean 			invalidate)
{
	char 				startOfDayStr[timeStringLength];
	char 				endOfDayStr[timeStringLength];
	char 				fullAllDayStr[timeStringLength*2+2];
	FieldType*			workHoursFieldP;
	ControlType*		allDayButtonP;
	Coord				buttonWidth;
	Coord				stringWidth;
	Coord				fieldMaxWidth;
	RectangleType		buttonBounds;
	RectangleType		fieldBounds;
	Coord				maxOffset;
	Coord				offset;
	RectangleType 		fieldRect;
	
	TimeToAscii( (uint8_t) startOfDay, 0, sTimeFormat, startOfDayStr);
	TimeToAscii( (uint8_t) endOfDay, 0, sTimeFormat, endOfDayStr);
	sprintf(fullAllDayStr, "%s-%s", startOfDayStr, endOfDayStr);
	stringWidth = FntCharsWidth(fullAllDayStr,strlen(fullAllDayStr));
	
	allDayButtonP = UtilitiesGetFormObjectPtr(formP, SelectTimeAllDayButton);
	UtilitiesGetFormObjectBound (formP, SelectTimeAllDayButton, &buttonBounds);
	buttonWidth = buttonBounds.extent.x;

	workHoursFieldP = UtilitiesGetFormObjectPtr(formP, SelectTimeAllDayWorkHoursField);
	UtilitiesGetFormObjectBound (formP, SelectTimeAllDayWorkHoursField, &fieldBounds);
	fieldMaxWidth = fieldBounds.extent.x;

	// 	NB: 
	//	Original resource width of all day button and work hours field
	//	are  used to calculate the maximum possible offset of which work hour field
	//	can move: doing so, the UI design is reinforced, and we are not going to have 
	//	the field move to overlap other UI items whithin the form.

	maxOffset = fieldMaxWidth - buttonWidth;
	if (maxOffset)
		maxOffset = (maxOffset >> 1) + (maxOffset % 2);
	
	offset = buttonWidth - stringWidth;
	if (offset)
		offset = (offset >> 1) + (offset % 2);

	if (offset != 0 && (fieldBounds.topLeft.x + stringWidth) < (fieldBounds.topLeft.x + fieldMaxWidth))
	{
		fieldBounds.topLeft.x = buttonBounds.topLeft.x + offset;
		fieldBounds.extent.x = stringWidth;
		UtilitiesSetFormObjectBound (formP, SelectTimeAllDayWorkHoursField, &fieldBounds);
	}
	
	UtilitiesSetFieldPtrText(UtilitiesGetFormObjectPtr(formP, SelectTimeAllDayWorkHoursField), fullAllDayStr, false);

	// Update field if needed
	if (invalidate)
	{
		FrmGetObjectBounds(formP, FrmGetObjectIndex (formP, SelectTimeAllDayWorkHoursField), &fieldRect);

		//Transparency
		//WinInvalidateRectFunc(FrmGetWindowHandle(sFormP), &fieldRect, PrvDrawFieldCallBack, (void *)SelectTimeAllDayWorkHoursField);
		WinInvalidateRect(FrmGetWindowHandle(sFormP), &fieldRect);
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvUpdateBarsSelection
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvUpdateBarsSelection(void)
{
	// Get the active trigger selection 
	switch (sSelectionSetting)
	{
		case tSelectStart:
			PrvUpdateBarSel(&sStartDate);
			break;

		case tSelectEnd:
			PrvUpdateBarSel(&sEndDate);
			break;

		case tSelectAllDay:
		case tSelectNoTime:
			PrvUpdateBarSel(NULL);
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvUpdateTriggers
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvUpdateTriggers(void)
{
	uint16_t				afterObjID;
	uint16_t				aboveObjID;
	uint16_t				belowObjID;
	FrmNavObjectFlagsType	objFlags;
	RectangleType			startBounds;
	RectangleType			endBounds;
	RectangleType			bounds;
	

	ControlType* startTriggerP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeStartTimeTrigger);
	ControlType* endTriggerP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeEndTimeTrigger);
	ControlType* NoTimeButtonP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeNoTimeButton);
	ControlType* allDayButtonP 	= UtilitiesGetFormObjectPtr(sFormP, SelectTimeAllDayButton);

	// First, set all to false
	CtlSetValue(startTriggerP, false);
	CtlSetValue(endTriggerP, false);
	CtlSetValue(NoTimeButtonP, false);
	CtlSetValue(allDayButtonP, false);

	if (sSelectionSetting == tSelectNoTime)
	{
		if (FrmGetNavEntry (sFormP, SelectTimeEndTimeTrigger, 
				&afterObjID, &aboveObjID,&belowObjID, &objFlags) == errNone) 
		{
			
			FrmSetNavEntry (sFormP, SelectTimeEndTimeTrigger, 
				afterObjID, aboveObjID, belowObjID, objFlags |= kFrmNavObjectFlagsSkip);
		}
	}
	else
	{
		if (FrmGetNavEntry (sFormP, SelectTimeEndTimeTrigger, 
				&afterObjID, &aboveObjID,&belowObjID, &objFlags) == errNone) 
		{
			
			FrmSetNavEntry (sFormP, SelectTimeEndTimeTrigger, 
				afterObjID, aboveObjID, belowObjID, objFlags &= ~kFrmNavObjectFlagsSkip);
		}
	}

	FrmGetObjectBounds(sFormP, FrmGetObjectIndex(sFormP, SelectTimeStartTimeTrigger), &startBounds);
	FrmGetObjectBounds(sFormP, FrmGetObjectIndex(sFormP, SelectTimeEndTimeTrigger), &endBounds);

	// Invalidate the region of the gadget that connects to the start/end
	// time so that it redraws to the correct setting
	bounds.topLeft.x = startBounds.topLeft.x + startBounds.extent.x;
	bounds.topLeft.y = startBounds.topLeft.y;
	bounds.extent.x = gadgetOffsetWidth;
	bounds.extent.y = endBounds.topLeft.y + endBounds.extent.y - startBounds.topLeft.y;
	WinInvalidateRect(FrmGetWindowHandle(sFormP), &bounds);

	switch (sSelectionSetting)
	{
		case tSelectStart:
			CtlSetValue(startTriggerP, true);
			FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, SelectTimeStartTimeTrigger));
			break;
			
		case tSelectEnd:
			CtlSetValue(endTriggerP, true);
			FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, SelectTimeEndTimeTrigger));
			break;

		case tSelectNoTime:
			CtlSetValue(NoTimeButtonP, true);
			FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, SelectTimeNoTimeButton));
			// Take the end button on the nav order when we're in no-time mode
			break;

		case tSelectAllDay:
			CtlSetValue(allDayButtonP, true);
			FrmSetFocus(sFormP, FrmGetObjectIndex(sFormP, SelectTimeAllDayButton));
			break;
	}
}


 /***********************************************************************
 *
 * FUNCTION:    	PrvComputeTimeFromBars
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  ->	dateTimeP:
 *				->	barSelectedID:
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvComputeTimeFromBars(DateTimeType* dateTimeP, uint16_t barSelectedID)
{
	ListType* 	listHoursBar1P = NULL;
	ListType* 	listHoursBar2P = NULL;
	ListType* 	listMinBarP = NULL;
	int16_t 	itemHoursBar1 = 0;
	int16_t 	itemHoursBar2 = 0;
	int16_t 	itemMinBar = 0;
	
	PrvAssignListPtrs(&listHoursBar1P, &listHoursBar2P, &listMinBarP);

	// Read the selection
	itemHoursBar1 	= LstGetSelection(listHoursBar1P);
	itemHoursBar2 	= LstGetSelection(listHoursBar2P);
	itemMinBar 		= LstGetSelection(listMinBarP);

	// Compute time
	switch (sDisplaySetting)
	{
		case tampmDisplay:
			if (itemHoursBar1 == noListSelection)
			{
				itemHoursBar1 = 0;

				if (itemHoursBar2 == noListSelection)
					itemHoursBar2 = 0;

				dateTimeP->hour= (sStartDate.hour % hoursInHalfDay) + (itemHoursBar2 * hoursInHalfDay);
				break;
			}
				
			if (itemHoursBar2 == noListSelection)
				itemHoursBar2 = 0;
				
			dateTimeP->hour = itemHoursBar1 + itemHoursBar2 * hoursInHalfDay;
			break;

		case t24hDisplay:
			switch (barSelectedID)
			{
				case SelectTimeHoursList_1_24Hrs:
					itemHoursBar2 = noListSelection;
					break;
					
				case SelectTimeHoursList_2_24Hrs:
					itemHoursBar1 = noListSelection;
					break;
			}
			
			if (itemHoursBar1 != noListSelection)
				dateTimeP->hour = itemHoursBar1;
			else if (itemHoursBar2 != noListSelection)
				dateTimeP->hour = hoursInHalfDay + itemHoursBar2;
			else
				dateTimeP->hour = 0;
			break;
	}

	if (itemMinBar == noListSelection)
		dateTimeP->minute = 0;
	else
		dateTimeP->minute = itemMinBar * minutesIncrement;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvTranslate24HourTime
 *
 * DESCRIPTION: 	This routine translated a series of character into a 
 *              	24 hour times.
 *
 * PARAMETERS:  ->	chr:		character written.
 *              <->	timeStr:	
 *						in: 	previously written character.
 *                      out: 	written character, including char passed.
 *				<->	 time:
 *						in: 	previously translated time.
 *                      out:	returned: the translated time.
 *
 * RETURNED:	 	true if sucessful
 *
 * REVISION HISTORY:
 *	art	4/2/96		Initial Revision.
 *
 ***********************************************************************/
static Boolean PrvTranslate24HourTime (char chr, char * timeStr, TimePtr time)
{
	uint16_t 	i;
	int16_t 	len;
	uint8_t 	num;
	uint8_t 	hours = 0;
	uint8_t 	minutes = 0;
	Boolean 	accept = false;
	
	len = strlen (timeStr);
	
	// Was a digit written?
	if (chr >= '0' && chr <= '9')
	{
		// The first character can be 0 - 9.
		if (len == 0)
			accept = true;

		// The second character can be:
		//		0 - 9  if the first character is a 0.
		//		0 - 9  if the first character is a 1.
		//		0 - 6  if the first character is greater than 1 (first minute 
		//           	digit).
		else if (len == 1 && ((*timeStr <= '1' || chr < '6')))
			accept = true;

		// The third character can be:
		//		0 - 6   if the first two character are between 00 - 23 (first 
		//            		minute digit).
		//		0 or 5  if the first character is greater than 2 (second munute digit). 
		else if (len == 2 && (
			(*timeStr < '2' && chr < '6') ||
			(*timeStr == '2' && timeStr[1] < '4' && chr < '6') ||
			(chr == '0' || chr == '5')))
			accept = true;

		// The forth character can be:
		//		0 or 5  if the first two character are between 00 - 23
		//						(second minute digit).
		else if (len == 3 && 
			(chr == '0' || chr == '5') && 
			(timeStr[0] < '2' || (timeStr[0] == '2' && timeStr[1] < '4')))
			accept = true;
		}
			

	
	// Add the character passed to the time string if it is valid, 
	// otherwise exit.
	if (accept)
	{
		timeStr[len++] = chr;
		timeStr[len] = 0;
	}
	else
		return (false);



	// Translate the written character into a time.
	for (i = 0; i < len; i++)
	{
		num = timeStr[i] - '0';
		
		// The first characters is always an hours digit.
		if (i == 0)
		{
			hours = num;
			minutes = 0;
		}

		// The second characters may be the second hours digit or the
		// first minutes digit.
		else if (i == 1)
		{
			if (hours == 0)
				hours = num;
			else if (hours < 2 || (hours == 2 && (num < 4)))
				hours = (hours * 10) + num;
			else
				minutes = num * 10;
		}

		// The third characters is	may be the first or second minutes digit		
		else if (i == 2)
		{
			if ((hours > 9) || (*timeStr == '0'))
				minutes = num * 10;
			else
				minutes += num;
		}
			
		// The forth characters is always the second minutes digit..
		else if (i == 3)
			minutes += num;
	}		
		
	// Return the translated time.
	time->hours = hours;
	time->minutes = minutes;
	
	return (true);
}

/***********************************************************************
 *
 * FUNCTION:    	PrvTranslate12HourTime
 *
 * DESCRIPTION: 	This routine handles key events in the Date Picker.  
 *              	Key events are translated into times.
 *
 * PARAMETERS:  ->	chr:		character written.
 *              ->	firstHour:	first valid hour.
 *              <->	timeStr:
 *						in: 	previously written character.
 *                      out: 	written character, including char passed.
 *				<->	 time:
 * 						in: 	previously translated time
 *                      out: 	the translated time.
 *
 * RETURNED:	 	true if sucessful.
 *
 * REVISION HISTORY:
 *	ART	4/2/96		Initial revision.
 *
 ***********************************************************************/
static Boolean PrvTranslate12HourTime (
	char 		chr, 
	int16_t 	firstHour, 
	char * 		timeStr, 
	TimePtr 	time)
{
	uint16_t 	i;
	int16_t 	len;
	uint8_t 	num;
	uint8_t 	hours = 0;
	uint8_t 	minutes = 0;
	Boolean 	accept = false;	
	
	len = strlen (timeStr);
	
	// Was a digit written?
	if (chr >= '0' && chr <= '9')
	{
		// The first character can be 0 - 9.
		if (len == 0)
			accept = true;

		// The second character can be:
		//		0 - 9  if the first character is a 0.
		//		0 - 6  if the first character is a 1.
		//		0 - 6  if the first character is greater than 1 (first minute 
		//           digit).
		else if (len == 1 && 
			((*timeStr == '0' || chr < '6')))
			accept = true;

		// The third character can be:
		//		0 - 6   if the first character is 0 (first minute digit).
		//		0 - 6   if the first character is 1 and the second character is 
		//            less then or equal to 2 (first minute digit).
		//		0 or 5  if the first character is greater than 1 (second munute digit). 
		else if (len == 2 && (
			(*timeStr == '0' && chr < '6') ||
			(*timeStr == '1' && timeStr[1] <= '2' && chr < '6') ||
			(chr == '0' || chr == '5')))
			accept = true;

		// The forth character can be:
		//		0 or 5  if the if the first character is 0 (second minute digit).
		//		0 or 5  if the first character is 1 and the second character is 
		//            less then or equal to 2 (second minute digit).
		else if (len == 3 && 
			(chr == '0' || chr == '5') && 
			(timeStr[0] == '0' || (timeStr[0] == '1' && timeStr[1] <= '2')))
			accept = true;
		}

	
	// Add the character passed to the time string if it is valid, 
	// otherwise exit.
	if (accept)
	{
		timeStr[len++] = chr;
		timeStr[len] = 0;
	}
	else
		return (false);

	// Translate the written character into a time.
	for (i = 0; i < len; i++)
	{
		// Have we reached the end of the numeric characters?
		if (! (timeStr[i] >= '0' && timeStr[i] <= '9'))
			break;

		num = timeStr[i] - '0';
		
		// The first characters is always an hours digit.
		if (i == 0)
		{
			hours = num;
			if (hours && hours < firstHour)
				hours += 12;
			minutes = 0;
		}

		// The second characters may be the second hours digit or the
		// first minutes digit.
		else if (i == 1)
		{
			if (hours == 0)
			{
				hours = num;
				if (hours && hours < firstHour)
					hours += 12;
			}
			else if ((hours == 1 || hours == 13) && (num <= 2))
			{
				hours = 10 + num;
				if (hours && hours < firstHour)
					hours += 12;
			}
			else
				minutes = num * 10;
		}

		// The third characters is	may be the first or second minutes digit		
		else if (i == 2)
		{
			if ((hours > 9 && hours < 13) || hours > 21)
				minutes = num * 10;
			else if (*timeStr == '0')
				minutes = num * 10;
			else
				minutes += num;
		}
			
		// The forth characters is always the second minutes digit..
		else if (i == 3)
			minutes += num;
	}
		
	// Return the translated time.
	time->hours = hours;	
	time->minutes = minutes;
	
	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    	PrvTranslateTime
 *
 * DESCRIPTION: 	This routine translated a series of character into a 
 *              	times.
 *
 * PARAMETERS:  ->	chr:		character written
 
 *              <->	timeStr:	
 *						in: 	previously written character
 *                      out: 	written character, including char passed
 
 *				<->	time	
 *						in: 	previously translated time
 *                      out: 	the translated time
 *
 * RETURNED:	 	true if sucessful.
 *
 * REVISION HISTORY:
 *	ART	4/2/96		Initial revision.
 *
 ***********************************************************************/
static Boolean PrvTranslateTime (char chr, char * timeStr, TimePtr time)
{
	if (sDisplaySetting == tampmDisplay)
	{
		switch(sSelectionSetting)
		{
			case tSelectEnd:
				return (PrvTranslate12HourTime (chr, sStartDate.hour, timeStr, time));

			default:
				//  0 is the first hour initially displayed 
				return (PrvTranslate12HourTime (chr, 0 , timeStr, time));
		}
	}
	else
		return (PrvTranslate24HourTime (chr, timeStr, time));
}


/***********************************************************************
 *
 * FUNCTION:    	PrvCheckDatesConsistency
 *
 * DESCRIPTION: 	Check the start and end time and update the date 
 *					if needed.
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvCheckDatesConsistency(void)
{
	uint32_t	startDate;
	uint32_t	endDate;
	TimeType	startTime;
	TimeType	endTime;
	Boolean 	sameDay;
	DateType	startDateInDateType;
	DateType	endDateInDateType;
	
	// Convert the date to seconds
	startDate = TimDateTimeToSeconds(&sStartDate);
	endDate = TimDateTimeToSeconds(&sEndDate);

	startTime = CnvDateTime2Time(&sStartDate);
	endTime = CnvDateTime2Time(&sEndDate);
	
	startDateInDateType = CnvDateTime2Date(&sStartDate);
	endDateInDateType = CnvDateTime2Date(&sEndDate);
	
	sameDay = (Boolean) (DateToInt(startDateInDateType) == DateToInt(endDateInDateType));
		
	if (sLimitedToSameDay)
	{
		if (TimeToInt(startTime) > TimeToInt(endTime))
		{
			sEndDate = sStartDate;
		}
	}
	else
	{
		// Check overlapping depending on the time just modified
		switch (sSelectionSetting)
		{
			case tSelectStart:
				if (sameDay)
				{
					if (TimeToInt(startTime) > TimeToInt(endTime))
					{
						sEndDate = sStartDate;
					}
				}
				else
				{
					if (endDate - startDate >= maxEventDurationInSecs)
					{
						endDate = startDate + maxEventDurationInSecs;
						TimSecondsToDateTime(endDate, &sEndDate);
					}
				}
				break;
				
			case tSelectEnd:
				if ((! sameDay) && (TimeToInt(endTime) >= TimeToInt(startTime)))
				{
					CnvDate2DateTime(&startDateInDateType, &endTime, &sEndDate);
					break;
				}

				if (sameDay && (TimeToInt(endTime) < TimeToInt(startTime)))
				{
					endDate += daysInSeconds;
					if (endDate > maxSeconds)
					{
						// Exceeded maximum displayable date - Truncating it...
						endTime.hours = maxHours;
						endTime.minutes = maxMinutes;
						CnvDate2DateTime(&startDateInDateType, &endTime, &sEndDate);
					}
					else
						TimSecondsToDateTime(endDate, &sEndDate);
				}
					
				if (endDate - startDate >= maxEventDurationInSecs)
				{
					startDate = endDate - maxEventDurationInSecs;
					TimSecondsToDateTime(startDate, &sStartDate);
				}
				break;
		}						
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvSwitchToCurrentTimeMode
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvSwitchToCurrentTimeMode(void)
{
	switch(sSelectionSetting)
	{
		case tSelectNoTime:
		case tSelectAllDay:
			sSelectionSetting = tSelectStart;
			PrvUpdateTriggers();
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvSwitchToNoTimeMode
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  	None.
 *
 * RETURNED:   		Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static void PrvSwitchToNoTimeMode(void)
{
	switch(sSelectionSetting)
	{
		case tSelectStart:
		case tSelectEnd:
		case tSelectAllDay:
			sSelectionSetting = tSelectNoTime;
			PrvUpdateBarsSelection();
			PrvUpdateTriggers();
			PrvDrawDatesAndTimes(true);
			break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleTriggerTap
 *
 * DESCRIPTION: 	The triggers event handler for ctlSelectEvent events
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogHandleTriggerTap(EventType *eventP)
{
	Boolean		handled = false;

	TimeSelectionType oldSelectSetting = sSelectionSetting;
			
	switch (eventP->data.ctlSelect.controlID)
	{
		case SelectTimeStartTimeTrigger:
			sSelectionSetting = tSelectStart;
			PrvUpdateBarSel(&sStartDate);
			PrvUpdateTriggers();
			*sGraffitiStr = '\0';
			handled = true;
			break;

		case SelectTimeEndTimeTrigger:
			sSelectionSetting = tSelectEnd;
			PrvUpdateBarSel(&sEndDate);
			PrvUpdateTriggers();
			*sGraffitiStr = '\0';
			handled = true;
			break;
	}

	// If the mode was previously "no time", update the triggers
	if (oldSelectSetting == tSelectNoTime)
		PrvDrawDatesAndTimes(true);

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleTapInList
 *
 * DESCRIPTION: 	The SelectTime event handler for lstEnterEvent events
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogHandleTapInList(EventType *eventP)
{
	Boolean handled = false;
	uint32_t startInSecs, endInSecs, duration;
	
	PrvSwitchToCurrentTimeMode();
	startInSecs = TimDateTimeToSeconds(&sStartDate);
	endInSecs = TimDateTimeToSeconds(&sEndDate);
	if (endInSecs >= startInSecs)
		duration = endInSecs - startInSecs;
	else
		duration = hoursInSeconds;
	
	switch(sSelectionSetting)
	{
		case tSelectStart:
			PrvComputeTimeFromBars(&sStartDate, eventP->data.lstEnter.listID);
			sEndDate = sStartDate;
			TimAdjust(&sEndDate, duration);
			handled = true;
			break;

		case tSelectEnd:
			 PrvComputeTimeFromBars(&sEndDate, eventP->data.lstEnter.listID);
			handled = true;
			break;
	}

	if (handled)
	{
		PrvCheckDatesConsistency();
		PrvUpdateBarsSelection();
		PrvDrawDatesAndTimes(true);
	}

 	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    	PrvDialogRockerLeftKey
 *
 * DESCRIPTION: 	Handles left rocker keys (Wheel and 5Way Rocker)
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/30/2004	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogRockerLeftKey(wchar32_t chr, uint16_t modifiers )
{
	uint16_t	focusedIndex;
	uint16_t	objectId;
	uint16_t	itemId;

	Boolean 	handled = false;

	focusedIndex = FrmGetFocus (sFormP);
	if (focusedIndex != noFocus)
	{
		objectId = FrmGetObjectId(sFormP, focusedIndex);

		switch (objectId)
		{
			case SelectTimeStartTimeTrigger:
				// should blocks
				break;
				
			case SelectTimeEndTimeTrigger:
				// goes on Start time push button
				FrmNavObjectTakeFocus(sFormP, SelectTimeStartTimeTrigger);
				handled = true;
				break;
			

			case SelectTimeHoursListAmPm:	   // first (hours)list, 12 h
			case SelectTimeHoursList_1_24Hrs:	// first (hours [0 - 12[)list, 24 h

				// focus is on the left most list hours (12h) or hours 1 (24h)
				// we do on left so we give the focus back to start or end time
				// push button (SelectTimeStartTimeTrigger or SelectTimeEndTimeTrigger)
				itemId = noFocus;

				// when the hours list (12h)  or the first hours list (24h)
				// we focus the first time selector trigger
				//if (CtlGetValue (UtilitiesGetFormObjectPtr (sFormP, SelectTimeStartTimeTrigger)))
				//	itemId = SelectTimeStartTimeTrigger;
				//else if (CtlGetValue (UtilitiesGetFormObjectPtr (sFormP, SelectTimeEndTimeTrigger)))
				//	itemId = SelectTimeEndTimeTrigger;

				itemId = SelectTimeStartTimeTrigger;
					
				if (itemId != noFocus)
				{
					FrmNavObjectTakeFocus(sFormP, itemId);
					handled = true;
				}
				break;
			   
			case SelectTimeMinutesListAmPm:		// second (minutes) list, 12 h
		   	case SelectTimeHoursList_2_24Hrs:	// second (hours [12 - 24[) list, 24 h
			   break;

		   	case SelectTimeMiddleDayAmPm:	   // third (am/pm) list, 12 h
		   	case SelectTimeMinutesList24Hrs:   // third (minutes) list, 24 h
				break;

			case SelectTimeAllDayButton:
			case SelectTimeNoTimeButton:
				break;
		}
	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogRockerRightKey
 *
 * DESCRIPTION: 	Handles left rocker keys (Wheel and 5Way Rocker)
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	04/06/2004	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogRockerRightKey(wchar32_t chr, uint16_t modifiers )
{
	uint16_t	focusedIndex;
	uint16_t	objectId;
	uint16_t	itemId;
	Boolean 	handled = false;
	
	focusedIndex = FrmGetFocus (sFormP);
	if (focusedIndex != noFocus)
	{
		objectId = FrmGetObjectId(sFormP, focusedIndex);

		switch (objectId)
		{
		   	case SelectTimeStartTimeTrigger:
		   	case SelectTimeEndTimeTrigger:
		   		// focus was on Start time or end time
				// we go on right so focus is given to 
				// the hours list (hours (12h) or hours 1 (24h))
			   itemId = (sDisplaySetting == t24hDisplay) ? SelectTimeHoursList_1_24Hrs : SelectTimeHoursListAmPm;
			   FrmNavObjectTakeFocus(sFormP, itemId);
			   handled = true;
			   break;
			   
		   	case SelectTimeHoursListAmPm:	   	// first (hours)list, 12 h
		   	case SelectTimeHoursList_1_24Hrs:  	// first (hours [0 - 12[)list, 24 h
			   break;
			   
		   	case SelectTimeMinutesListAmPm:		// second (minutes) list, 12 h
		   	case SelectTimeHoursList_2_24Hrs:	// second (hours [12 - 24[) list, 24 h
			   break;

		   	case SelectTimeMiddleDayAmPm:	   	// third (am/pm) list, 12 h
		   	case SelectTimeMinutesList24Hrs:   	// third (minutes) list, 24 h
				// focus was on the right most list
				// we go to the right so the focus again start or end time
				// push button (SelectTimeStartTimeTrigger or SelectTimeEndTimeTrigger)
				
		   		itemId = noFocus;
				
				//if (CtlGetValue (UtilitiesGetFormObjectPtr (sFormP, SelectTimeStartTimeTrigger)))
				//	itemId = SelectTimeStartTimeTrigger;
				//else if (CtlGetValue (UtilitiesGetFormObjectPtr (sFormP, SelectTimeEndTimeTrigger)))
				//itemId = SelectTimeEndTimeTrigger; 
				
				// after focusing the am /pm list (12h)or the minutes list (24h)
				// we go on the end time
				itemId = SelectTimeEndTimeTrigger; 
					
				if (itemId != noFocus)
				{
					FrmNavObjectTakeFocus(sFormP, itemId);
					handled = true;
				}
				break;


		  	case SelectTimeAllDayButton:
		   	case SelectTimeNoTimeButton:
			   break;
	   	}
   	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogRockerCenterKey
 *
 * DESCRIPTION: 	Handles center rocker key(Wheel and 5Way Rocker)
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	04/06/2004	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogRockerCenterKey(wchar32_t chr, uint16_t modifiers )
{
	//ListType*	listP;
	//EventType	newEvent;
	//int16_t 	lstSelection;
	uint16_t	focusedIndex;
	uint16_t	objectId;


	Boolean		handled = false;
	
	// 5-way rocker center/press
	// active focused tab
	focusedIndex = FrmGetFocus (sFormP);

	if (focusedIndex != noFocus)
	{
		 objectId = FrmGetObjectId(sFormP, focusedIndex);

		 switch (objectId)
		 {
			case SelectTimeStartTimeTrigger:
			case SelectTimeEndTimeTrigger:
				break;
				
			case SelectTimeHoursListAmPm:	   // first (hours)list, 12 h
			case SelectTimeHoursList_1_24Hrs:	// first (hours [0 - 12[)list, 24 h
				break;
					   
			case SelectTimeMinutesListAmPm:		// second (minutes) list, 12 h
			case SelectTimeHoursList_2_24Hrs:	// second (hours [12 - 24[) list, 24 h
			   break;
		
			case SelectTimeMiddleDayAmPm:	   // third (am/pm) list, 12 h
			case SelectTimeMinutesList24Hrs:   // third (minutes) list, 24 h
				/*
				listP = (ListType*) FrmGetObjectPtr (sFormP, focusedIndex);
			
				// Select the temporarily selected item
				lstSelection = NavPrvLstGetTempSelection (sFormP, objectId);
				LstSetSelection (listP, lstSelection);

				// Send a lstSelect event for the selected item
				MemSet (&newEvent, sizeof (newEvent), 0);
				newEvent.eType = lstSelectEvent;
				newEvent.data.lstSelect.listID = objectId;
				newEvent.data.lstSelect.pList = listP;
				newEvent.data.lstSelect.selection = lstSelection;

				EvtAddEventToQueue (&newEvent);
				*/
				break;

			case SelectTimeAllDayButton:
			case SelectTimeNoTimeButton:
			// does nothing
				break;
		}
	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleNavigationKeys
 *
 * DESCRIPTION: 	Handles Navigation keys (Wheel and 5Way Rocker)
 *
 * PARAMETERS:  ->	chr:		key event chr.
 *				-> 	modifiers: 	key event modifier.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/30/2004	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogHandleNavigationKeys(wchar32_t	chr, uint16_t	modifiers)
{
	Boolean		handled = false;

	TraceOutput(TL(appErrorClass, "DateBook, SelectTime, PrvDialogHandleNavigationKeys: Start"));

	switch(chr)
	{
		case vchrRockerUp:	
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerUp, before/above item, left not handled"));
			break;

			
		case vchrRockerDown:	
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerDown, next/below item, left not handled"));
			break;


		case vchrThumbWheelBack:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelBack, next/below item, left not handled"));
			break;


		case vchrThumbWheelDown:	
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelDown as vchrRockerLeft"));


		case vchrRockerLeft:	
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerLeft, Previous tab is focused"));
			// 5-way rocker left
			// move focused tab to left
			handled = PrvDialogRockerLeftKey(chr,modifiers);
			break;

		
		case vchrThumbWheelUp:	
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelUp as vchrRockerRight"));


		case vchrRockerRight:
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerRight, Next tab is focused"));
			// 5-way rocker right
			// move focused tab to right
			handled = PrvDialogRockerRightKey(chr,modifiers);
			break;

			
		case vchrThumbWheelPush:
			TraceOutput(TL(appErrorClass, "-------------> vchrThumbWheelPush as vchrRockerCenter"));


		case vchrRockerCenter: 
			TraceOutput(TL(appErrorClass, "-------------> vchrRockerCenter"));	
			handled = PrvDialogRockerCenterKey(chr,modifiers);
			break;
			
		default:
			TraceOutput(TL( appErrorClass, "-------------> Not a 5-way or Wheel Navigation vchr"));
			break;
	}

	TraceOutput(TL(appErrorClass, "DateBook, SelectTime, PrvDialogHandleNavigationKeys: Start"));

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleKeys
 *
 * DESCRIPTION: 	The SelectTime event handler for keyDownEvent events
 *
 * PARAMETERS:  ->	chr:		key event chr.
 *				-> 	modifiers: 	key event modifier.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *	PPL	03/03/04	Add protection to avoid eating 5way vchr
 *	PPL	04/07/04	change names.
 *
 ***********************************************************************/
static Boolean PrvDialogHandleKeys(wchar32_t	chr, uint16_t	modifiers)
{
	Boolean 	handled = false;
	TimeType 	tmpTime;
	DateType	locDate;		

	// If a backspace character was written, change the time picker's
	// current setting to "no-time".
	switch (chr)
	{
		case backspaceChr:
			*sGraffitiStr = '\0';
			PrvSwitchToNoTimeMode();
			handled = true;
			break;

	// linefeedChr and nextFieldChr are not managed by PalmSim: added space to test
		case linefeedChr:
		case nextFieldChr:
		case spaceChr:
			switch(sSelectionSetting)
			{
				case tSelectStart:
					sSelectionSetting = tSelectEnd;
					PrvUpdateBarSel(&sEndDate);
					break;
				case tSelectEnd:
					sSelectionSetting = tSelectStart;
					PrvUpdateBarSel(&sStartDate);
					break;
			}
			PrvUpdateTriggers();
			*sGraffitiStr = '\0';
			handled = true;
			break;
		// a		
		case chrSmall_A:
		case chrCapital_A:
			PrvSwitchAMPM(-12);
			handled = true;
			break;
		// p
		case chrSmall_P:
		case chrCapital_P:
			PrvSwitchAMPM(12);
			handled = true;
			break;
			
		default:
			if (PrvTranslateTime ((char) chr, sGraffitiStr, &tmpTime))
			{
				PrvSwitchToCurrentTimeMode();
				
				switch(sSelectionSetting)
				{
					case tSelectStart:
						locDate = CnvDateTime2Date(&sStartDate);
						CnvDate2DateTime(&locDate, &tmpTime, &sStartDate);
						sEndDate = sStartDate;
						TimAdjust(&sEndDate, hoursInSeconds);
						break;
					case tSelectEnd:
						locDate = CnvDateTime2Date(&sEndDate);
						CnvDate2DateTime(&locDate, &tmpTime, &sEndDate);
						break;
				}
				PrvCheckDatesConsistency();
				PrvDrawDatesAndTimes(true);
				PrvUpdateBarsSelection();
				handled = true;
			}
			break;
	}

	return handled;
}

static void PrvSwitchAMPM(int16_t gap)
{	
	uint32_t startInSecs, endInSecs, duration;	
	PrvSwitchToCurrentTimeMode();
	startInSecs = TimDateTimeToSeconds(&sStartDate);
	endInSecs = TimDateTimeToSeconds(&sEndDate);
	if (endInSecs >= startInSecs)
		duration = endInSecs - startInSecs;
	else
		duration = hoursInSeconds;
	
	switch(sSelectionSetting)
		{
		case tSelectStart:					
			if (sStartDate.hour+gap>=0 && sStartDate.hour+gap<24)
			{
				sStartDate.hour=sStartDate.hour+gap;
				sEndDate = sStartDate;
				TimAdjust(&sEndDate, duration);
			}
			break;
		case tSelectEnd:
			if (sEndDate.hour+gap>=0 && sEndDate.hour+gap<24)
			{
				sEndDate.hour=sEndDate.hour+gap;
			}			
			break;
		}
		
	PrvCheckDatesConsistency();
	PrvDrawDatesAndTimes(true);		
	PrvUpdateBarsSelection();
}

/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleFocusTake
 *
 * DESCRIPTION: 	Handles Focus Take.
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/30/2004	Initial revision.
 *
 ***********************************************************************/
static Boolean	PrvDialogHandleFocusTake(EventType* eventP)
{
	uint16_t objectId = eventP->data.frmObjectFocusTake.objectID;
	Boolean  handled = false;

	switch(objectId)
	{
		// If start time button/trigger just got focus and it is displaying a time,
		// set the hour and minutes list to the time displayed and clear
		// the start time's character buffer.
		case SelectTimeStartTimeTrigger:
			sSelectionSetting = tSelectStart;
			break;
			
		case SelectTimeEndTimeTrigger:
			sSelectionSetting = tSelectEnd;
			break;

		// 12 h lists
		case SelectTimeHoursListAmPm:
		case SelectTimeMiddleDayAmPm:
		case SelectTimeMinutesListAmPm:

		//24 hours list
		case SelectTimeHoursList_1_24Hrs:
		case SelectTimeHoursList_2_24Hrs:
		case SelectTimeMinutesList24Hrs:
			break;
	}
	
	PrvUpdateBarsSelection();
	PrvUpdateTriggers();
	PrvDrawDatesAndTimes(true);


	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleFocusLost
 *
 * DESCRIPTION: 	Handles Focus Lost.
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/30/2004	Initial revision.
 *
 ***********************************************************************/
static Boolean	PrvDialogHandleFocusLost(EventType* eventP)
{
	uint16_t objectId 		= eventP->data.frmObjectFocusLost.objectID;
	uint16_t focusedIndex 	= frmInvalidObjectId;
	Boolean  handled 		= false;

	
	switch(objectId)
	{
		// If start time button/trigger just got focus and it is displaying a time,
		// set the hour and minutes list to the time displayed and clear
		// the start time's character buffer.
		case SelectTimeStartTimeTrigger:
			//sSelectionSetting = tSelectStart;
			break;
			
		case SelectTimeEndTimeTrigger:
			//sSelectionSetting = tSelectEnd;
			break;

		// 12 h lists
		case SelectTimeHoursListAmPm:
		case SelectTimeMiddleDayAmPm:
		case SelectTimeMinutesListAmPm:

		//24 hours list
		case SelectTimeHoursList_1_24Hrs:
		case SelectTimeHoursList_2_24Hrs:
		case SelectTimeMinutesList24Hrs:
			break;
	}
	
	//PrvUpdateBarsSelection();
	//PrvUpdateTriggers();
	//PrvDrawDatesAndTimes(true);

	/*
	focusedIndex = FrmGetFocus (sFormP);

	switch (objectId)
	{
		// If focus has left the minute or hour list and focus is not on the
		// other list now, de-select the start/end time button if it is not
		// the newly focused object.
		case TimeSelectorMinuteList:
		case TimeSelectorHourList:
			if (focusedIndex != hourListIndex) && focusedIndex != minuteListIndex))
			{
				// De-select start time button if it does not currently have the focus
				if (focusedIndex != startTimeIndex)
					CtlSetValue (FrmGetObjectPtr (sFormP startTimeIndex), false);

				// De-select end time button if it does not currently have the focus
				if (focusedIndex != endTimeIndex) {
					CtlSetValue (FrmGetObjectPtr (sFormP, endTimeIndex), false);
			}
			break;
			
		// if focus left the start or end time button and focus is not on
		// the minute or hour list now, de-select the button

		case TimeSelectorStartTimeButton:
		case TimeSelectorEndTimeButton:

			if (focusedIndex != FrmGetObjectIndex (sFormP, TimeSelectorHourList)) 
			&&(focusedIndex != FrmGetObjectIndex (sFormP, TimeSelectorMinuteList)))
			{
				CtlSetValue (FrmGetObjectPtr (sFormP, FrmGetObjectIndex (frmsFormP, objectId)), false);
			}
			break;
	}
		*/
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleKeyEvents
 *
 * DESCRIPTION: 	The SelectTime event handler for keyDownEvent events
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 04/07/04	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvDialogHandleKeyEvents(EventType * eventP)
{
	wchar32_t	chr 			= eventP->data.keyDown.chr;
	uint16_t	modifiers 		= eventP->data.keyDown.modifiers;

	Boolean 	handled = false;

	if (TxtCharIsRockerKey (modifiers, chr) || TxtCharIsWheelKey(modifiers, chr))
	{
		handled = PrvDialogHandleNavigationKeys(chr, modifiers);
	}
	else
	{
		handled = PrvDialogHandleKeys(chr, modifiers);
	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvDialogHandleEvent
 *
 * DESCRIPTION: 	The SelectTime event handler
 *
 * PARAMETERS:  ->	eventP:	pointer on the event.
 *
 * RETURNED:   		handled state.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *	PPL	03/30/2004	Add Focusing event.
 *
 ***********************************************************************/
static Boolean PrvDialogHandleEvent(EventType * eventP)
{
	Boolean	handled = false;

	// Trace the event in debug mode
	DBGTraceEvent("SelectTimeDialog", eventP);
	
	switch(eventP->eType)
	{
		case ctlSelectEvent:
			handled = PrvDialogHandleTriggerTap(eventP);
			break;

		case lstSelectEvent:
			handled = PrvDialogHandleTapInList(eventP);
			break;
			
		case keyDownEvent:
			handled = PrvDialogHandleKeyEvents(eventP);
			break;

		case frmObjectFocusTakeEvent:
			handled = PrvDialogHandleFocusTake(eventP);
			break;
			
		case frmObjectFocusLostEvent:
			handled = PrvDialogHandleFocusLost(eventP);
			break;
	}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    	PrvAssignUpdatedDates
 *
 * DESCRIPTION: 	The new SelectTime call.
 *
 * PARAMETERS: 	->	buttonHit:
 *				->	startDateTimeP:
 * 				->	endDateTimeP:
 * 				<->	inOutSelectionP:
 *
 * RETURNED:   		Returns true if the user selects OK and false otherwise. 
 *					If true is returned, the values in hour and minute have 
 *					probably been changed.
 *	
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *
 ***********************************************************************/
static Boolean PrvAssignUpdatedDates(
	uint16_t 				buttonHit, 
	DateTimeType* 			startDateTimeP, 
	DateTimeType* 			endDateTimeP, 
	TimeSelectionType * 	inOutSelectionP)
{
	Boolean 	wasChanged = false;
	DateType	startDate = CnvDateTime2Date(startDateTimeP);
	TimeType	startDayTime = {0, 0};
	TimeType	endDayTime = {0, 0};
	
	switch (buttonHit)
	{
		case SelectTimeOKButton:
			*startDateTimeP = sStartDate;
			*endDateTimeP = sEndDate;
			*inOutSelectionP = sSelectionSetting;
			wasChanged = true;
			break;
			
		case SelectTimeAllDayButton:
			startDayTime.hours = (uint8_t)sStartOfDay;
			endDayTime.hours = (uint8_t)sEndOfDay;
			CnvDate2DateTime(&startDate, &startDayTime, startDateTimeP);
			CnvDate2DateTime(&startDate, &endDayTime, endDateTimeP);
			*inOutSelectionP = tSelectAllDay;
			wasChanged = true;
			break;
			
		case SelectTimeNoTimeButton:
			*inOutSelectionP = tSelectNoTime;
			wasChanged = true;
			break;

		case SelectTimeCancelButton:
			*inOutSelectionP = sSelectionSetting;
			wasChanged = false;
			break;
	}

	// Reset time if noTime selected
	if (*inOutSelectionP == tSelectNoTime)
	{
		CnvDate2DateTime(&startDate, NULL, startDateTimeP);
		CnvDate2DateTime(&startDate, NULL, endDateTimeP);
	}

	return wasChanged;
}


/***********************************************************************
 *
 * FUNCTION:    	SelectTimeV6
 *
 * DESCRIPTION: 	The new SelectTime call.
 *
 * PARAMETERS: 	->	titleP:
 *				->	startDateTimeP:
 * 				->	endDateTimeP:
 * 				<->	inOutSelectionP:
 * 				->	startOfDay:
 * 				->	endOfDay:
 * 				->	limitedToTheSameDay:
 *
 * RETURNED:   		Returns true if the user selects OK and false otherwise. 
 *					If true is returned, the values in hour and minute have 
 *					probably been changed.
 *	
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PLe	12/18/2002	Initial revision.
 *	PPL	09/15/2003	remove call to PrvCloseDialog() as it became useless.
 *					(Te close was only freeing the all day button label
 *					and we added a feild to hold the work day hours.)
 *
 ***********************************************************************/
Boolean SelectTimeV6(
	char * 				titleP, 
	DateTimeType* 		startDateTimeP, 
	DateTimeType* 		endDateTimeP,
	TimeSelectionType * inOutSelectionP, 
	int16_t				startOfDay, 
	int16_t 			endOfDay,
	Boolean 			limitedToTheSameDay)
{
	Boolean 			wasChanged;
	FormActiveStateType	curState;
	uint16_t			buttonHit;

	FrmSaveActiveState(&curState);	

	// Create the form
	sFormP = FrmInitForm(gApplicationDbP, SelectTimeForm);
	
	if (titleP)
		FrmSetTitle(sFormP, titleP);
		
	FrmSetActiveForm(sFormP);

	// Set the event handler
	FrmSetEventHandler(sFormP, PrvDialogHandleEvent);

	// Initialization stuff
	PrvDialogInit(	sFormP, 
					startDateTimeP, 
					endDateTimeP, 
					*inOutSelectionP,
					limitedToTheSameDay, 
					startOfDay, 
					endOfDay);

	// Activate the dialog as modal
	buttonHit = FrmDoDialog (sFormP);

	// Close the dialog: we have nothing to release

	// Apply dates & times updated and set the returned state
	wasChanged = PrvAssignUpdatedDates(	buttonHit, 
										startDateTimeP, 
										endDateTimeP, 
										inOutSelectionP);

	// Delete the form and restore previous state
	FrmDeleteForm(sFormP);
	FrmRestoreActiveState(&curState);	
	
	return  wasChanged;
}
