/******************************************************************************
 *
 * Copyright (c) 2003-2004 PalmSource, Inc. All rights reserved.
 *
 * File: GcDraw.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#include <PalmOS.h>

#include "GcDrawResources.h"

DmOpenRef	gAppDB;

// Enables automatic form object repositioning when the DIA changes
FormLayoutType gMainFormLayout[] = {
	{ sizeof(FormLayoutType), MainFormLogoButton, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), MainFormRotateButton, 0, frmFollowBottom },
	{ sizeof(FormLayoutType), frmLayoutGraffitiShiftID, 0, frmFollowBottom },
	{ 0,0,0,0 }	// Last item must be all zeros
};

/***********************************************************************
 *
 * FUNCTION:    DisplayAboutDialog
 *
 * DESCRIPTION: This routine performs the menu command specified.b
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
static void DisplayAboutDialog(void)
{
	FormType	*pForm;

	// OS6NEW: FrmInitForm() needs app db ptr as first arg now
	pForm = FrmInitForm(gAppDB, AboutDialog);
	FrmDrawForm(pForm);
	FrmDoDialog(pForm);					
	FrmDeleteForm(pForm);
	
}



/******************************************************************************
 *
 * Function		: StartApplication
 * Returns		: status_t value 0 if nothing went wrong
 * Parameters	: none
 * Description	: Function called before starting the application.
 *				  Here it does not do anything, add your own code if you want to
 *				  set user preferences.
 *
 *****************************************************************************/
static status_t StartApplication()
{
	return errNone;;
}

/******************************************************************************
 *
 * Function		: StopApplication
 * Returns		: nothing
 * Parameters	: none
 * Description	: This function does not do anything now except closing all forms
 *				  it can be used to save the current state of the application.
 *
 *****************************************************************************/
static void StopApplication()
{
	FrmCloseAllForms();
}

/******************************************************************************
 *
 * Function		: RotateObject
 * Returns		: nothing
 * Parameters	: none
 * Description	: This function is used to demonstrate the rotation transformation
 *				  using the GcXXXX functions. The idea here is to simulate
 *				  a simplistic sun-earth-moon rotation on their respective orbits.
 *
 *****************************************************************************/

static void RotateObject()
{
	// get a pointer to the currently active form
	FormType	*form = FrmGetActiveForm();
	//initialize the event to 0;
	EventType	event = {0};

	// gc is a Graphics Context Handle varialble
	// it keeps track of the current rendering state.
	// The Graphics Context is where the new API draws to,
	// instead of the window directly.
	GcHandle gc;

	// fcoord_t is of type float for Gc APIs
	fcoord_t counter = 1;

	// The radians by which the rotation will take place
	// pi radians = 22/7 = 3.14
	// pi radians = 180 degrees
	fcoord_t rot=0.0;

	// Creates the graphics context for the current draw window
	gc = GcGetCurrentContext();
	// Checks if a NULL was returned above. Returns if the gc is not created
	if (!gc)
		return;

	// Set the gc to use Antialiasing.
	GcSetAntialiasing(gc, 1);

	while (true)
	{
		EvtGetEvent(&event, 30);
		// Redraw the form each time we enter the loop to erase the drawing done in the
		// previous iteration.
		FrmDrawForm(form);
		// Increase the degree of rotation by 9 degrees(=0.15714 radians) in each iteration
		rot = rot + (fcoord_t)0.15714;
		// Save the current state of the Graphics Context
		// Using this function we can isolate the transformations done after this function call
		// right upto the matching PopState() from the current Graphics Context
		GcPushState(gc);
			// Set Background Color as Black
			GcSetColor(gc,0,0,0,255);
			// Set the coordinate system to use standard coordinates 
			GcSetCoordinateSystem(gc,kCoordinatesStandard);
			// Make a rectangular black background, with top left coordinates
			// as (0,20) and bottom right corner at (160,140)
			GcRect(gc,0,20,160,140);
			// Paint the background
			GcPaint(gc);
			// Translate the center of the coordinate system to 80,80
			GcTranslate(gc,80,80);
			// Change color to yellow to paint the Sun, as the loop iterates(value of counter increases), the transparency decreases
			GcSetColor(gc,248,251,123,(uint8_t)155 + counter);
			// Draw a circle, with the center as 0,0
			// This is done by drawing an arc with horizontal and vertical
			// radii set to 12
			GcArc(gc,(fcoord_t)0,(fcoord_t)0,(fcoord_t)12,(fcoord_t)12,0,(float)6.29,false); // ArcTo function to draw Sun
			//Paint the sun
			GcPaint(gc);
			// Change color to Red to paint the orbit of the planet revolving around the sun
			GcSetColor(gc,255,0,0,(uint8_t)185);
			// Draw a circle to display the orbit, with the center as 0,0
			// This is done by drawing an arc with horizontal and vertical
			// radii set to 40
			GcArc(gc,(fcoord_t)0,(fcoord_t)0,(fcoord_t)40,(fcoord_t)40,0,(float)6.29,false); // ArcTo function to draw Orbit
			// Set the thickness of the orbit
			GcSetPenSize(gc,2);
			// This function causes only the circumference of the circle to be drawn
			GcStroke(gc);
			//Paint the orbit
			GcPaint(gc);
			// Rotates the entire coordinate system
			// by the rot radians to show the revolution of the planet around the sun
			GcRotate(gc,(fcoord_t)rot);
			// Save the current state of the graphics context by pushing it to the stack
			GcPushState(gc);
				// set the color of planet, with an alpha value of 255 
				// vs a value of 0 which makes it invisible
				GcSetColor(gc,0,0,255,(uint8_t)255);
				// Translate the center of the co-ordinate system to (0,40)
				GcTranslate(gc,0,40);
				// Draw a circle, with the center as 0,0
				// This is done by drawing an arc with horizontal and vertical
				// radii set to 8
				GcArc(gc,(fcoord_t)0,(fcoord_t)0,(fcoord_t)8,(fcoord_t)8,0,(float)6.29,false);
				// Paint the planet
				GcPaint(gc);
				// set the color of the orbit of the satellite, with an alpha
				// value of 185 a value of 0 which makes it invisible
				GcSetColor(gc,235,0,0,(uint8_t)185);
				// Draw a circle to display the orbit, with the center as 0,0
				// This is done by drawing an arc with horizontal and vertical
				// radii set to 15
				GcArc(gc,(fcoord_t)0,(fcoord_t)0,(fcoord_t)15,(fcoord_t)15,0,(float)6.29,false);
				// set the thickness of the orbit
				GcSetPenSize(gc,1);
				// This function causes only the circumference of the circle to be drawn
				GcStroke(gc);
				// Paint the orbit
				GcPaint(gc);
				// Rotates the entire coordinate system 
				// by the rot radians to show the revolution of the satellite around the planet
				GcRotate(gc,(fcoord_t)rot);
				// Save the current state of the Graphics Context
				GcPushState(gc);
					// set the color of satellite, with an alpha value of 255 which makes it opaque
					// vs a value of 0 which makes it invisible
					GcSetColor(gc,255,255,255,(uint8_t)255);
					// Draw a circle to display the satellite, with the center as 0,0
					// This is done by drawing an arc with horizontal and vertical
					// radii set to 4
					GcArc(gc,(fcoord_t)0,(fcoord_t)15,(fcoord_t)4,(fcoord_t)4,0,(float)6.29,false);
					// Paint the satellite
					GcPaint(gc);
				// Restore the previous Graphics Context
				GcPopState(gc);
			// Restore the previous Graphics Context
			GcPopState(gc);
		// Restore the previous Graphics Context
		GcPopState(gc);

		// Delay by 20 to show animation
		SysTaskDelay(25);
		if(counter++>80)
			break;
	}

	// Release the current graphics context
	GcReleaseContext(gc);
}

/******************************************************************************
 *
 * Function		: LaunchLogo
 * Returns		: nothing
 * Parameters	: none
 * Description	: This is the function that is called for displaying the PalmSource Logo
 *
 *****************************************************************************/
static void LaunchLogo()
{
	// get a pointer to the currently active form
	FormType	*form = FrmGetActiveForm();

	// Variables required for using the PalmSource logo bitmap
	MemHandle bitmapH;
	BitmapPtr bitmapP;

	// Initialize the event to 0
	EventType	event = {0};

	// gc is a Graphics Context Handle varialble
	// it keeps track of the current rendering state.
	// The Graphics Context is where the new API draws to,
	// instead of the window directly.
	GcHandle gc;
	fcoord_t counter = 0;
	// Variables required to carry out the transformations
	fcoord_t l=-100;
	fcoord_t left=0;

	// loading bitmap resource
	bitmapH = DmGetResource(gAppDB,bitmapRsc, PSLogoBitmap);
	if(!bitmapH) {
		// display error message and return if the bitmap cannot be loaded.
		FrmAlert(gAppDB, NoLogoAlert);
		return;
	}
	bitmapP = MemHandleLock(bitmapH);

	// Creates the graphics context for the current draw window
	gc = GcGetCurrentContext();
	// Checks if a NULL was returned above. Returns if the gc is not created
	if (!gc) {
		// Releases the bitmap resources
		MemPtrUnlock(bitmapP);
		DmReleaseResource( bitmapH );
		return;
	}
	// enable antialiasing
	GcSetAntialiasing(gc,1);
	// Run the event Loop till event type is appStopEvent
 	int trans = 0;
	while (true)
	{
		EvtGetEvent(&event, 30);
		// Redraw the form each time we enter the loop to erase the drawing done in the
		// previous iteration.
		FrmDrawForm(form);
		// variable value incremented to enable translation
		left+=20;
		// Set the coordinate system to use the standard coordinates
		GcSetCoordinateSystem (gc, kCoordinatesStandard);
		// Save the current state of the Graphics Context
		// Using this function we can isolate the transformations done starting after this
		// function call right upto the matching PopState() from the current Graphics Context
		GcPushState(gc);
				// Set Color of the rectangle that appears behind the PalmSource Logo to Blue
				// The Alpha blending(transparency) decreases as the rectangle translates to the right
				GcSetColor(gc,0,0,185,trans*3);
				// Move the Rectangle to the right by translation
				GcTranslate(gc,trans,0);
				// specify the dimensions of the rectangle
				GcRect(gc,0,30,80,130);
				// Paint the rectangle
				GcPaint(gc);
		// Restore the graphics context
		GcPopState(gc);
		// Save the current state of the Graphics Context
		// Using this function we can isolate the transformations done starting after this
		// function call right upto the matching PopState() from the current Graphics Context
		GcPushState(gc);
				// Set Color of the rectangle that appears behind the PalmSource Logo to Black
				// The Alpha blending(transparency) decreases as the rectangle translates to the right
				GcSetColor(gc,0,0,0,trans*3);
				// Move the Rectangle to the left
				GcTranslate(gc,-trans,0);
				// specify the dimensions of the rectangle
				GcRect(gc,80,30,160,130);
				// Paint the rectangle
				GcPaint(gc);
		// Restore the graphics context
		GcPopState(gc);

		// Save the current state of the Graphics Context
		// Using this function we can isolate the transformations done starting after this
		// function call right upto the matching PopState() from the current Graphics Context
		GcPushState(gc);
			// Translates the PalmSource Logo in the South-West direction in each iteration
			GcTranslate(gc,-trans/10,trans/10);
			// Draw the PalmSource Logo bitmap
		WinDrawBitmap (bitmapP, 35, 50);
		GcPopState(gc);
		// Save the current state of the Graphics Context
		// Using this function we can isolate the transformations done starting after this
		// function call right upto the matching PopState() from the current Graphics Context
		GcPushState(gc);
			// Set the color and transparency of the parallelogram above the PalmSource Logo
			GcSetColor(gc,255,255,255,255);
			// Move the drawing location to (35,50)
			GcMoveTo(gc,(fcoord_t)35,(fcoord_t)50);
			GcLineTo(gc,(fcoord_t)131,(fcoord_t)50);
			GcLineTo(gc,(fcoord_t)131-trans/10,(fcoord_t)50+trans/10);
			GcLineTo(gc,(fcoord_t)35-trans/10,(fcoord_t)50+trans/10);
			GcClosePath(gc);
			// Paint the parallelogram, since GcStroke was not called, hence the entire
			// region bounded by the above quadrilateral is painted white
			GcPaint(gc);
			// Set the color and transparency of the parallelogram to the right of PalmSource Logo
			GcSetColor(gc,255,255,255,200);
			// Move the drawing location to (131,50)
			GcMoveTo(gc,(fcoord_t)131,(fcoord_t)50);
			GcLineTo(gc,(fcoord_t)131,(fcoord_t)104);
			GcLineTo(gc,(fcoord_t)131-trans/10,(fcoord_t)104+trans/10);
			GcLineTo(gc,(fcoord_t)131-trans/10,(fcoord_t)50+trans/10);
			GcClosePath(gc);
			// Paint the parallelogram, since GcStroke was not called, hence the entire
			// region bounded by the above quadrilateral is painted white
			GcPaint(gc);
		GcPopState(gc);
		// Adds a delay before the next iteration
		SysTaskDelay (25);
		// execute the loop till the value of trans crosses 80
		if(trans >= 80)
			break;
		trans+=5;
	}
	// Release the graphics context and the bitmap resources
	GcReleaseContext(gc);
	MemPtrUnlock(bitmapP);
	DmReleaseResource( bitmapH );
}

/******************************************************************************
 *
 * Function		: MainMenuHandleEvent
 * Returns		: true if the menu id is handled otherwise false
 * Parameters	: command  - menu item id
 * Description	: This routine performs the menu command specified.
 *
 *****************************************************************************/

static Boolean MainMenuHandleEvent (UInt16 menuID)
{
	Boolean handled = false;
	FormType	*form = FrmGetActiveForm();

	switch(menuID)
	{
		case MainOptionsMenuDetails:
				FrmAlert(gAppDB, AboutAlert);
				FrmDrawForm(form);
				handled = true;
				break;

		case MainOptionsMenuAboutGcDraw:
				DisplayAboutDialog();
				FrmDrawForm(form);
				handled = true;
				break;
		default :
				break;
	}
	return handled;
}

/******************************************************************************
 *
 * Function		: MainFormHandleEvent
 * Returns		: true if the event has handle and should not be passed
 *                to a higher level handler.
 * Parameters	: pEvent  - a pointer to an EventType structure
 * Description	: This routine is the event handler for the
 *                "MainForm" of this application.
 *
 *****************************************************************************/
static Boolean MainFormHandleEvent (EventPtr event)
{
	Boolean handled = false;
	FormType *form;
	switch (event->eType)
	{
		case frmOpenEvent:
				{
					form = FrmGetActiveForm();
					FrmDrawForm(form);
					handled = true;
					break;
				}
		case ctlSelectEvent:
				{
					switch(event->data.ctlSelect.controlID)
					{
					case MainFormLogoButton:
							LaunchLogo();
							handled = true;
							break;
					case MainFormRotateButton:
							RotateObject();
							handled = true;
							break;
					default:
							break;
					}
				break;
				}
		case menuEvent:
				handled = MainMenuHandleEvent(event->data.menu.itemID);
				break;
		default:
				break;
	}
	return handled;
}

/******************************************************************************
 *
 * Function		: ApplicationHandleEvent
 * Returns		: true if the event has handle and should not be passed
 *                to a higher level handler.
 * Parameters	: event  - a pointer to an EventType structure
 * Description	: This routine loads form resources and set the event
 *	              handler for the form loaded.
 *
 *****************************************************************************/
static Boolean ApplicationHandleEvent( EventPtr event)
{
	FormType *pForm;
	UInt16	formID;
	Boolean handled = false;

	if(event->eType == frmLoadEvent)
	{
		formID = event->data.frmLoad.formID;
		pForm = FrmInitForm(gAppDB,formID);
		FrmSetActiveForm(pForm);

		switch (formID)
		{
		case MainFormForm:
			FrmSetEventHandler (pForm, MainFormHandleEvent);
			FrmInitLayout(pForm,gMainFormLayout);
			break;
		default:
			break;
		}
		handled = true;
	}
	return handled;
}
/******************************************************************************
 *
 * Function		: EventLoop
 * Returns		: nothing
 * Parameters	: none
 * Description	: This routine is the event loop for the application. Each time
 *				  an event occurs, first it checks to see if the event can be
 *				  handled by the operating system, if not then by the menu,
 *				  finally it is checked by the application. If it is an application
 *				  event, the event is dispatched to the FrmDispatchEvent function
 *				  to be handled. The event loop quits if the type of the event is
 *				  appStopEvent, which terminates the loop and returns from this function.
 *****************************************************************************/
static void EventLoop()
{
	EventType event;
	status_t error;
	do
	{
		EvtGetEvent(&event,evtWaitForever);
		if(!SysHandleEvent(&event))
			if(!MenuHandleEvent(0,&event,&error))
				if(!ApplicationHandleEvent(&event))
					FrmDispatchEvent(&event);
	}
	while(event.eType != appStopEvent);
}

/******************************************************************************
 *
 * Function		: PilotMain
 * Returns		: Result of launch of type UInt32
 * Parameters	: cmd		  - word value specifying the launch code.
 *                cmdPB		  - pointer to a structure that is associated with the launch code.
 *                launchFlags -  word value providing extra information about the launch.
 * Description	: This is the equivalent of a main function of a C program
 *				  The application execution starts here.
 *
 *****************************************************************************/

UInt32 PilotMain(UInt16 launchCode,MemPtr cmdPBP,UInt16 launchFlags)
{
	// OS6NEW: New error type
	status_t error = errNone;

	// OS6NEW: Get app database ref - needed for Form Mgr calls, resources.
	if ((error = SysGetModuleDatabase(SysGetRefNum(), NULL, &gAppDB)) < errNone)
		return error;

	// Handle launch code
	switch(launchCode)
	{

	case sysAppLaunchCmdNormalLaunch:
		// Perform app initialization.
		if((error = StartApplication())==0)
		{
			// OS6NEW: FrmGotoForm() now requires app db ref argument
			FrmGotoForm(gAppDB, MainFormForm);
			// Handle events until user switches to another app.
			EventLoop();
			// Clean up before exit.
			StopApplication();
		}
		break;
	default :
		break;
	}
	return error;
}
