/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: GcFonts.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "AppResources.h"

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

// Holds current transform values for the three types we support in the app.
typedef struct TransformType {
	fcoord_t 		minimum;
	fcoord_t 		maximum;
	fcoord_t 		value;
	fcoord_t		step;
} TransformType;

// Transformation types
enum {
	kTransTypeSize = 0,
	kTransTypeShear,
	kTransTypeRotate
};

// Flag to redraw entire form instead of single object
#define	kDrawFormID		(-1)

// Structure to hold our font family names (or styles)
typedef struct ListItemsType {
	uint16_t	uwNumItems;	// number of items in array
	FontFamily	*pItems;	// array of FontFamily/FontStyle items
							// (essentially char[64])
} ListItemsType;


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/

// Holds current value and range for three transformation properties
TransformType	gTransType[3] = {
	// min,		max,		value,	step
	{ 3.0f,		100.0f,		42.0f,	1.0f },		// Size
	{ -10.0f,	10.0f,		0.0f,	0.05f },	// Shear (x)
	{ 0.0f,		6.283185f,	0.0f,	0.05f },	// Rotate (radians)
};


DmOpenRef		gAppResDB;		// Pointer to database holding our form and other resources.

ListItemsType	gFontNames;
ListItemsType	gFontStyles;

GcFontHandle	gFontHandle;

uint16_t		gCurrentNameIndex;
uint16_t		gCurrentStyleIndex;


// Enables automatic form object repositioning when the DIA changes
FormLayoutType gMainFormLayout[] = {
	{ sizeof(FormLayoutType), MainCanvasGadget, 0, frmFollowAllSides },
	{ sizeof(FormLayoutType), MainTextLabel, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainSampleField, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), frmLayoutGraffitiShiftID, 0, frmFollowBottom },
	{ 0,0,0,0 }	// Last item must be all zeros
};



/***********************************************************************
 *	Internal Functions
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:    PrvGetObjectPtr
 *
 * DESCRIPTION: Returns a pointer to an object's data on the specified form.
 *
 * PARAMETERS:   -> pForm: Pointer to form
 *				 -> uwObjectID: ID of desired object
 *
 * RETURNED:    Pointer to object, or NULL if no form specified.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
***********************************************************************/
static void * PrvGetObjectPtr(FormType *pForm, uint16_t uwObjectID)
{
	uint16_t	uwObjIndex;

	if (pForm == NULL)
		return NULL;

	uwObjIndex = FrmGetObjectIndex(pForm, uwObjectID);

	return FrmGetObjectPtr(pForm, uwObjIndex);
}


/******************************************************************************
 *
 * FUNCTION:		PrvInvalidateObject
 *
 * DESCRIPTION:		Invalidates window or form object rectangle to trigger
 *						an update event by the system.
 *
 * PARAMETERS:		 -> uwFormID: The ID of the form
 *					 -> wObjectID: The ID of the object to invalidate. If
 *							wObjectID is kDrawFormID, invalidate whole form.
 *
 * RETURNED:    	Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ******************************************************************************/
static void PrvInvalidateObject(uint16_t uwFormID, int16_t wObjectID)
{
	WinHandle	hWindow;
	FormType *	pForm;

	if ((pForm = FrmGetFormPtr(uwFormID)) == NULL)
		return;

	if ((hWindow = FrmGetWindowHandle(pForm)) == NULL)
		return;

	if (wObjectID == kDrawFormID)
		// Force update of entire window.
		WinInvalidateWindow(hWindow);
	else {
		RectangleType	rect;
		FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, (uint16_t)wObjectID), &rect);
		WinInvalidateRect(FrmGetWindowHandle(pForm), &rect);
	}
}


/***********************************************************************
 *
 * FUNCTION:    DisplayAboutDialog
 *
 * DESCRIPTION: Display the About box dialog.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void DisplayAboutDialog(void)
{
	FormType	*pForm;

	pForm = FrmInitForm(gAppResDB, AboutForm);
	FrmDoDialog(pForm);
	FrmDeleteForm(pForm);
}


/***********************************************************************
 *
 * FUNCTION:    BuildFamilyArray
 *
 * DESCRIPTION: Create an array of font families installed on device.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Index of first family in the list
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static uint16_t BuildFamilyArray(void)
{
	int32_t		numFamilies = GcCountFontFamilies();
	int32_t		i;
	FontFamily	family;
	FontStyle	style;
	status_t	err;
	int16_t		which = 0;

	// Initialize the data structure for our font families.
	gFontNames.uwNumItems = (uint16_t)numFamilies;
	if (numFamilies > 0) {
		gFontNames.pItems = (FontFamily *)MemPtrNew(sizeof(FontFamily) * numFamilies);
		MemSet(gFontNames.pItems, sizeof(FontFamily) * numFamilies, 0);
	}

	for (i = 0; i < numFamilies; i++) {
		// Get the next family name.
		err = GcGetFontFamily(i, &family);
		if (err == errNone)
			StrCopy(gFontNames.pItems[i], family);
	}

	// Set initial font
	if (gFontHandle)
		GcReleaseFont(gFontHandle);
	gFontHandle = GcCreateFont("palmos-plain");

	// Get the family name and find it in the names list.
	GcGetFontFamilyAndStyle(gFontHandle, &family, &style);

	for (i = 0; i < numFamilies; i++) {
		if (StrCompare(gFontNames.pItems[i], family) == 0) {
			which = (int16_t)i;
			break;
		}
	}

	return which;
}


/***********************************************************************
 *
 * FUNCTION:    BuildStyleArray
 *
 * DESCRIPTION: Create an array of styles for the specified font family.
 *
 * PARAMETERS:   -> family: Pointer to name of font family
 *
 * RETURNED:    Index of current style
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static int16_t BuildStyleArray(const char* family)
{
	int32_t		numStyles;
	int32_t		i;
	FontStyle	currentStyle;
	FontFamily	currentFamily;
	int16_t		which = 0;

	// Free existing pointer if it exists.
	if (gFontStyles.uwNumItems > 0 && gFontStyles.pItems)
		MemPtrFree(gFontStyles.pItems);

	numStyles = GcCountFontStyles(family);
	gFontStyles.uwNumItems = (uint16_t)numStyles;

	// Allocate space for an array of styles (strings).
	gFontStyles.pItems = (FontFamily *)MemPtrNew(numStyles * sizeof(FontStyle));

	for (i = 0; i < numStyles; i++) {
		FontStyle style;
		status_t err = GcGetFontStyle(family, i, &style);
		if (err == errNone)
			StrCopy(gFontStyles.pItems[i], style);
	}

	// Get the family name and find it in the names list.
	GcGetFontFamilyAndStyle(gFontHandle, &currentFamily, &currentStyle);

	for (i = 0; i < numStyles; i++) {
		if (StrCompare(gFontStyles.pItems[i], currentStyle) == 0) {
			which = (int16_t)i;
			break;
		}
	}

	return which;
}


/***********************************************************************
 *
 * FUNCTION:    LimitProperty
 *
 * DESCRIPTION: Ensure value is within property min/max limits.
 *
 * PARAMETERS:   -> tt: Index of property to check
 *				 -> val: Value to validate
 *
 * RETURNED:    Value, within property limits
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
***********************************************************************/
static fcoord_t LimitProperty(int32_t tt, fcoord_t val)
{
	if (val < gTransType[tt].minimum)
		return gTransType[tt].minimum;

	if (val > gTransType[tt].maximum)
		return gTransType[tt].maximum;

	return val;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormUpdateStylePopup
 *
 * DESCRIPTION: Update the Style popup trigger and list when user
 *					chooses a different font family.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
***********************************************************************/
static void MainFormUpdateStylePopup(void)
{
	FormType	*pForm;
	ListType	*pList;
	ControlType	*pTrigger;

	pForm = FrmGetActiveForm();
	pList = PrvGetObjectPtr(pForm, MainFaceList);

	LstSetListChoices(pList, NULL, gFontStyles.uwNumItems);
	LstSetHeight(pList, gFontStyles.uwNumItems);
	LstSetSelection(pList, gCurrentStyleIndex);

	pTrigger = PrvGetObjectPtr(pForm, MainFacePopTrigger);
	CtlSetLabel(pTrigger, gFontStyles.pItems[gCurrentStyleIndex]);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDrawFontStyleItem
 *
 * DESCRIPTION: Callback routine to draw items in the font style popup
 *
 * PARAMETERS:	 -> itemNum: Index of list item to draw
 *				 -> pRect: Pointer to bounds where we can draw
 *				 -> pItemsText: unused (will be NULL)
 *				 -> pList: Pointer to ListType structure
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/

static void MainFormDrawFontStyleItem(int16_t itemNum, RectangleType *pRect,
									   char **itemsText, struct ListType *listP)
{
	uint16_t	itemTextLen;
	char		*pStr;

	// Truncate the item with an ellipsis if it doesn't fit in the list width.
	// Get the item text length
	pStr = (char *)gFontStyles.pItems[itemNum];

	itemTextLen = StrLen(pStr);

	WinDrawTruncChars(pStr, itemTextLen, pRect->topLeft.x, pRect->topLeft.y,
					pRect->extent.x);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDrawFontFamilyItem
 *
 * DESCRIPTION: Callback routine to draw items in the font family popup
 *
 * PARAMETERS:	 -> itemNum: Index of list item to draw
 *				 -> pRect: Pointer to bounds where we can draw
 *				 -> pItemsText: unused (will be NULL)
 *				 -> pList: Pointer to ListType structure
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
***********************************************************************/

static void MainFormDrawFontFamilyItem(int16_t itemNum, RectangleType *pRect,
									   char **pItemsText, struct ListType *pList)
{
	uint16_t	itemTextLen;
	char		*pStr;

	// Truncate the item with an ellipsis if it doesn't fit in the list width.
	// Get the item text length
	pStr = (char *)gFontNames.pItems[itemNum];

	itemTextLen = StrLen(pStr);

	WinDrawTruncChars(pStr, itemTextLen, pRect->topLeft.x, pRect->topLeft.y,
					pRect->extent.x);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormSetAmountSlider
 *
 * DESCRIPTION: Called when user chooses a different Transform Type push
 *				button. Set the slider values for the transform.
 *
 * PARAMETERS:   -> pForm: Pointer to main form's data
 *				 -> uwPushBtnID: ID of pushed button :-)
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void MainFormSetAmountSlider(FormType* pForm, uint16_t uwPushBtnID)
{
	ControlType		*pSlider = PrvGetObjectPtr(pForm, MainAmountFeedbackSliderControl);
	uint16_t		tt = uwPushBtnID - MainSizePushButton;		// transform type = 0..2
	fcoord_t		range;
	uint16_t		ctlMin, ctlMax, ctlPage, ctlValue;

	range = gTransType[tt].maximum - gTransType[tt].minimum;

	// The minimum is always 0. Scale the other values accordingly.
	ctlMin = 0;
	ctlMax = (uint16_t)((range / gTransType[tt].step) + 0.5f);
	ctlPage = (uint16_t)((ctlMax * (gTransType[tt].step * 10.0 / range)) + 0.5f);
	ctlValue = (uint16_t)((ctlMax * ((gTransType[tt].value - gTransType[tt].minimum) / range)) + 0.5f);

	// Set the values - the OS will take care of updating the slider's appearance.
	CtlSetSliderValues(pSlider, &ctlMin, &ctlMax, &ctlPage, &ctlValue);
}

/***********************************************************************
 *
 * FUNCTION:    MainFormDrawCanvas
 *
 * DESCRIPTION: Called by gadget handler when gadget needs to be redrawn.
 *
 * PARAMETERS:   ->	pRect: Pointer to gadget's bounding rectangle
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void MainFormDrawCanvas(RectangleType *pRect)
{
	FormType		*pForm = FrmGetActiveForm();
	FieldType		*field = PrvGetObjectPtr(pForm, MainSampleField);
	char			*pText = FldGetTextPtr(field);
	RectangleType	bounds = *pRect;
	GcHandle		gc;
	fcoord_t		y, x;
	FAbsRectType	fontR;

	// Save the draw state (we'll restore it when we are done drawing).
	WinPushDrawState();

	// Set the window's coordinate system to native coordinates. Although you could
	// do the drawing in standard coords, the GcFontSize() call returns
	// native coordinates, so this way we don't have to do very much scaling.
	WinSetCoordinateSystem(kCoordinatesNative);

	// We do need to scale the canvas bounds rectangle to native, however.
	WinScaleRectangle(&bounds);

	// Get the graphics context (don't forget to release when done).
	gc = GcGetCurrentContext();

	// Although we have set the window's coordinate system to native, we
	// still need to set the graphic context coordinates as well.
	GcSetCoordinateSystem(gc, kCoordinatesNative);

	// Set clipping region to the canvas area.
	GcBeginClip(gc, false);
	GcRectI(gc, bounds.topLeft.x, bounds.topLeft.y,
			bounds.topLeft.x + bounds.extent.x, bounds.topLeft.y + bounds.extent.y);
	GcPaint(gc);
	GcEndClip(gc);

	// Clear canvas by painting white.
	GcSetColor(gc, 255, 255, 255, 255);
	GcPaint(gc);

	// Turn on antialiasing to make the text look better
	GcSetAntialiasing(gc, kNormalAntialiasing);

	// Set font size, style, and color
	GcSetFont(gc, gFontHandle);
	GcSetFontSize(gFontHandle, gTransType[kTransTypeSize].value);
	GcSetFontStyle(gFontHandle, gFontStyles.pItems[gCurrentStyleIndex]);
	GcSetColor(gc, 255, 0, 128, 255);	// Reddish - maybe we'll add a picker later

	// Find the center of the canvas and move the origin there. We'll be rotating
	// the canvas around that point.
	y = (fcoord_t)(bounds.topLeft.y + (bounds.extent.y / 2));
	x = (fcoord_t)(bounds.topLeft.x + (bounds.extent.x / 2));
	GcTranslate(gc, x, y);

	// Apply our effects
	GcShear(gc, gTransType[kTransTypeShear].value, 0.0f);
	GcRotate(gc, gTransType[kTransTypeRotate].value);

	// Get the height and width of the font, so we can draw the text
	// centered on the canvas's midpoint.
	GcFontStringBounds(gFontHandle, pText, StrLen(pText), &fontR);

	// Draw the text to complete the path
	GcDrawTextAt(gc, -((fontR.right - fontR.left) / 2), ((fontR.bottom - fontR.top) /2),
						pText, StrLen(pText));

	// Release the context...
	GcReleaseContext(gc);

	// ...and restore our window's draw state.
	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    MainFormGadgetHandler
 *
 * DESCRIPTION: Called by OS to handle events for gadget (update, penDown, etc.).
 *
 * PARAMETERS:   -> Standard gadget handler callback function parameters.
 *
 * RETURNED:    'true' if no additional processing of event is required.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean MainFormGadgetHandler(struct FormGadgetTypeInCallback *gadgetP,
									uint16_t cmd, void *paramP)
{
	// We only care about update events - our signal to redraw the canvas.
	if (cmd == formGadgetDrawCmd && gFontHandle) {
		MainFormDrawCanvas(&gadgetP->rect);
		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormCloseEvent
 *
 * DESCRIPTION: Clean up allocated memory here before we close (we
 *					allocated memory in MainFormOpenEvent().
 *
 * PARAMETERS:  void
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void MainFormCloseEvent(void)
{
	// Free allocated memory
	if (gFontStyles.pItems)
		MemPtrFree(gFontStyles.pItems);

	if (gFontNames.pItems)
		MemPtrFree(gFontNames.pItems);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormOpenEvent
 *
 * DESCRIPTION: Initializes objects on the main form. Do not do any
 *					drawing: when everything is set up, invalidate the
 *					form to force the initial drawing.
 *				NOTE: we allocate two handles here that must be
 *					freed when we leave the form.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void MainFormOpenEvent(void)
{
	FormType	*pForm = FrmGetFormPtr(MainForm);
	ListType	*pList;
	ControlType	*pTrigger, *pPushButton;
	FieldType	*pField;
	const char	*text = "PalmSource";

	// Build array of font names (for popup list) and return selected font.
	gCurrentNameIndex = BuildFamilyArray();

	// Set font family popup list and trigger.
	pList = PrvGetObjectPtr(pForm, MainFamilyList);
	LstSetDrawFunction(pList, MainFormDrawFontFamilyItem);
	LstSetListChoices(pList, NULL, gFontNames.uwNumItems);
	LstSetHeight(pList, gFontNames.uwNumItems);
	LstSetSelection(pList, gCurrentNameIndex);

	pTrigger = PrvGetObjectPtr(pForm, MainFamilyPopTrigger);
	CtlSetLabel(pTrigger, gFontNames.pItems[gCurrentNameIndex]);

	// Set up the font style list and trigger.
	gCurrentStyleIndex = BuildStyleArray(gFontNames.pItems[gCurrentNameIndex]);

	pList = PrvGetObjectPtr(pForm, MainFaceList);
	LstSetDrawFunction(pList, MainFormDrawFontStyleItem);

	MainFormUpdateStylePopup();

	// Set the initial value for the push buttons
	pPushButton = PrvGetObjectPtr(pForm, MainSizePushButton);
	CtlSetValue(pPushButton, true);

	// Set up the slider
	MainFormSetAmountSlider(pForm, MainSizePushButton);

	// Set the handler function for events in our canvas (gadget). The only events
	// we care about are update events, so we can draw the canvas.
	FrmSetGadgetHandler(pForm, FrmGetObjectIndex(pForm, MainCanvasGadget),
						MainFormGadgetHandler);

	// Set the starting text.
	pField = PrvGetObjectPtr(pForm, MainSampleField);
	FldInsert(pField, text, StrLen(text));

	// Put insertion point in text field.
	FrmSetFocus(pForm, FrmGetObjectIndex(pForm, MainSampleField));

	// Invalidate whole window - OS will draw during next update event.
	PrvInvalidateObject(MainForm, kDrawFormID);
}


/***********************************************************************
 *
 * FUNCTION:    MainFormMenuEvent
 *
 * DESCRIPTION: Process user's menu selections.
 *
 * PARAMETERS:   -> uwMenuCmdID: ID of selected menu item
 *
 * RETURNED:    'true' if item is selected
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean MainFormMenuEvent(uint16_t uwMenuCmdID)
{
	Boolean		fHandled = false;

	switch (uwMenuCmdID) {
		case MainOptionsAboutGcFonts:
			DisplayAboutDialog();
			fHandled = true;
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormKeydownEvent
 *
 * DESCRIPTION: Update the canvas when user edits the input text
 *
 * PARAMETERS:   -> pEvent: Pointer to event's data
 *
 * RETURNED:    'true' if we update the display; 'false' otherwise
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean MainFormKeydownEvent(EventType *pEvent)
{
	// Ignore all virtual keys
	if (EvtKeydownIsVirtual(pEvent))
		return false;

	// Force redraw of canvas.
	PrvInvalidateObject(MainForm, MainCanvasGadget);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormCtlSelectEvent
 *
 * DESCRIPTION: If the user taps on the Size, Shear, or Rotate pushbutton,
 *				set the values of the slider and redraw it.
 *
 * PARAMETERS:   -> pEvent: Pointer to event's data
 *
 * RETURNED:    'true' if a button was pushed.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean MainFormCtlSelectEvent(EventType *pEvent)
{
	Boolean		fHandled = false;

	switch (pEvent->data.ctlSelect.controlID ) {
		case MainSizePushButton:
		case MainShearPushButton:
		case MainAnglePushButton:
			MainFormSetAmountSlider(FrmGetActiveForm(), pEvent->data.ctlSelect.controlID);
			fHandled = true;
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormCtlRepeatEvent
 *
 * DESCRIPTION: Process user's adjustment of the value slider control.
 *
 * PARAMETERS:   -> pEvent: Pointer to event's data
 *
 * RETURNED:    Nothing (the caller must return 'false' so sytem can
 *					process the next ctlRepeatEvent
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void MainFormCtlRepeatEvent(EventType *pEvent)
{
	// We only have one slider control
	if (pEvent->data.ctlRepeat.controlID == MainAmountFeedbackSliderControl) {
		uint16_t	value = pEvent->data.ctlRepeat.value;
		uint16_t	pbIndex;
		uint16_t	tt;
		FormType	*pForm = FrmGetActiveForm();
		uint16_t	minVal, maxVal;
		fcoord_t	range;
		ControlType *pSlider;

		pbIndex = FrmGetControlGroupSelection(pForm, MainGroupID);
		tt = FrmGetObjectId(pForm, pbIndex) - MainSizePushButton;

		pSlider = PrvGetObjectPtr(pForm, MainAmountFeedbackSliderControl);
  		CtlGetSliderValues(pSlider, &minVal, &maxVal, NULL, NULL);

		range = gTransType[tt].maximum - gTransType[tt].minimum;
		gTransType[tt].value = LimitProperty(tt, gTransType[tt].minimum
					+ (range * value) / (maxVal - minVal));

		PrvInvalidateObject(MainForm, MainCanvasGadget);
	}
}


/***********************************************************************
 *
 * FUNCTION:    MainFormPopSelectEvent
 *
 * DESCRIPTION: Process user selection of popup list item.
 *
 * PARAMETERS:   -> pEvent: Pointer to event's data
 *
 * RETURNED:    Return 'true' if user chooses a list item, since we
 *					are setting the popup trigger's text ourselves.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean MainFormPopSelectEvent(EventType *pEvent)
{
	Boolean		fHandled = false;
	char		*pSelText;

	// Get the index of the selected item from the event data structure
	switch (pEvent->data.popSelect.controlID ) {
		case MainFamilyPopTrigger:
			// Update the trigger text, since we are overriding the default behavior.
			gCurrentNameIndex = pEvent->data.popSelect.selection;
			pSelText = gFontNames.pItems[gCurrentNameIndex];
			CtlSetLabel(pEvent->data.popSelect.controlP, pSelText);

			// Set new font handle, update the Style popup, and update canvas
			// TODO: factor following code
			if (gFontHandle)
				GcReleaseFont(gFontHandle);
			gFontHandle = GcCreateFont(pSelText);

			GcSetFontSize(gFontHandle, gTransType[kTransTypeSize].value);
			GcSetFontStyle(gFontHandle, gFontStyles.pItems[gCurrentStyleIndex]);

			gCurrentStyleIndex = BuildStyleArray(pSelText);
			MainFormUpdateStylePopup();

			// Force an update of the canvas to reflect user's choice.
			PrvInvalidateObject(MainForm, MainCanvasGadget);

			fHandled = true;
			break;

		case MainFacePopTrigger:
			gCurrentStyleIndex = pEvent->data.popSelect.selection;
			pSelText = gFontStyles.pItems[gCurrentNameIndex];
			CtlSetLabel(pEvent->data.popSelect.controlP, pSelText);
			MainFormUpdateStylePopup();

			GcSetFontStyle(gFontHandle, gFontStyles.pItems[gCurrentStyleIndex]);

			// Force an update of the canvas to reflect user's choice.
			PrvInvalidateObject(MainForm, MainCanvasGadget);

			fHandled = true;
			break;
	}

	return fHandled;
}



/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: Handles events for this form.
 *
 * PARAMETERS:   -> pEvent: Pointer to received EventType structure
 *
 * RETURNED:	Return 'true' to prevent system from calling default
 *					event handler.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventType* pEvent)
{
	Boolean 	fHandled = false;

	switch (pEvent->eType) {
		case frmOpenEvent:
			MainFormOpenEvent();
			fHandled = true;
			break;

		case frmCloseEvent:
			MainFormCloseEvent();
			break;

		case ctlSelectEvent:
			fHandled = MainFormCtlSelectEvent(pEvent);
			break;

		case popSelectEvent:
			fHandled = MainFormPopSelectEvent(pEvent);
			break;

		case ctlRepeatEvent:
			MainFormCtlRepeatEvent(pEvent);
			break;

		case keyDownEvent:
			MainFormKeydownEvent(pEvent);
			break;

		case menuEvent:
			fHandled = MainFormMenuEvent(pEvent->data.menu.itemID);
			break;

		default:
			break;
	}

	return fHandled;
}


/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: Loads form resources, makes form active, and registers
 *					the form's event handler function.
 *
 * PARAMETERS:   -> pEvent: Pointer to EventType structure
 *
 * RETURNED:    Return 'true' if you don't want system to call the default
 *					event handler for the event.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventType* pEvent)
{
	uint16_t 	formId;
	FormType	*pForm;

	if (pEvent->eType == frmLoadEvent) {
		// Load the form resource.
		formId = pEvent->data.frmLoad.formID;

		// OS6NEW: FrmInitForm() needs app res db ptr as first argument
		pForm = FrmInitForm(gAppResDB, formId);
		FrmSetActiveForm(pForm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId) {
			case MainForm:
				FrmSetEventHandler(pForm, MainFormHandleEvent);
				FrmInitLayout(pForm, gMainFormLayout);
				break;

			default:
				ErrFatalDisplay("Invalid Form Load Event");
				break;
		}

		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Set up app's state, init globals, etc.
 *
 * PARAMETERS:   None
 *
 * RETURNED:     0 (errNone) if no problems.
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static status_t AppStart(void)
{

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Clean up and save application state before closing.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void AppStop(void)
{
	// Close all the open forms.
	FrmCloseAllForms();
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: Processes all events for the application.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	status_t	error;
	EventType	event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (SysHandleEvent(&event))
			continue;

		// OS6NEW: error codes are now status_t, not status_t (32 bit)
		if (MenuHandleEvent(0, &event, &error))
			continue;

		if (AppHandleEvent(&event))
			continue;

		FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: Main entry point for the application.
 *
 * PARAMETERS:   -> launchCode: Launch code specifying why app's entry
 *						point was called.
 *              <-> pParams: Pointer to a data structure associated
 *						with the launch code.
 *               -> launchFlags: Provides information about app conditions at launch
 *					(	e.g., is app the active application).
 * RETURNED:    Result of launch. If no errors, return 0
 *
 * REVISION HISTORY:
 *		6/2/03	jbp	Initial version
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t launchCode, MemPtr pParams, uint16_t launchFlags)
{
	// OS6NEW: New error type
	status_t error = errNone;

	// OS6NEW: Get app resource database ref - needed for Form Mgr calls, resources, etc.
	if ((error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gAppResDB)) < errNone)
		return error;

	// Handle launch code
	switch (launchCode) {
		case sysAppLaunchCmdNormalLaunch:
			// Perform app initialization.
			error = AppStart();
			if (error)
				return error;

			// OS6NEW: FrmGotoForm() now requires app db ref argument
			FrmGotoForm(gAppResDB, MainForm);

			// Handle events until user switches to another app.
			AppEventLoop();

			// Clean up before exit.
			AppStop();
			break;

		default:
			break;

	}

	return errNone;
}
