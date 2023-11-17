/******************************************************************************
 *
 * Copyright (c) 1996-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateDisplay.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This module contains the routines that handle the Datebook 
 *   applications's display options.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>
#include <TimeMgr.h>
#include <DateTime.h>
#include <CatMgr.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <DataMgr.h>
#include <ErrorMgr.h>

#include "DateGlobals.h"
#include "DateDisplay.h"
#include "DateUtils.h"
#include "DatebookRsc.h"


/***********************************************************************
 *
 * FUNCTION:    DisplayOptionsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Display
 *              Options Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/95	Initial Revision
 *
 ***********************************************************************/
static Boolean DisplayOptionsApply (void)
{	
	Boolean on;
	Boolean updated = false;
 	FormPtr frmP;

	frmP = FrmGetActiveForm ();
		
	// Get the "Show Time Bars" setting.
	on = (CtlGetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowTimeBarsCheckbox)))!= 0);
	if (on != ShowTimeBars)
	{
		updated = true;
		ShowTimeBars = on;
	}		

	// Get the "Compress Day View" setting.
	on = (CtlGetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayCompressDayViewCheckbox)))!= 0);
	if (on != CompressDayView)
	{
		updated = true;
		CompressDayView = on;
	}		
		
	// Get the "Show Timed Events" setting
	on = (CtlGetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowTimedCheckbox)))!= 0);
	if (on != ShowTimedAppts)
	{
		updated = true;
		ShowTimedAppts = on;
	}		

	// Get the "Show Untimed Events" setting
	on = (CtlGetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowUntimedCheckbox)))!= 0);
	if (on != ShowUntimedAppts)
	{
		updated = true;
		ShowUntimedAppts = on;
	}		

	// Get the "Show Daily repeating Events" setting
	on = (CtlGetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowRepeatingCheckbox)))!= 0);
	if (on != ShowDailyRepeatingAppts)
	{
		updated = true;
		ShowDailyRepeatingAppts = on;
	}		

	return updated;
}


/***********************************************************************
 *
 * FUNCTION:    DisplayOptionsInit
 *
 * DESCRIPTION: This routine initializes the DisplayOptions Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/96	Initial Revision
 *
 ***********************************************************************/
static void DisplayOptionsInit (void)
{
 	FormPtr frmP;

	frmP = FrmGetActiveForm ();

	// Set the "Show Time Bars" setting.
	CtlSetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowTimeBarsCheckbox)),
		ShowTimeBars);

	// Set the "Compress Day View" setting.
	CtlSetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayCompressDayViewCheckbox)),
		CompressDayView);
	
	// Set the "Show Timed Events" setting
	CtlSetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowTimedCheckbox)),
		ShowTimedAppts);
	
	// Set the "Show Untimed Events" setting
	CtlSetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowUntimedCheckbox)),
		ShowUntimedAppts);

	// Set the "Show Daily repeating Events" setting
	CtlSetValue (FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, DisplayShowRepeatingCheckbox)),
		ShowDailyRepeatingAppts);
	}


/***********************************************************************
 *
 * FUNCTION:    DisplayOptionsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Display Options
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/96	Initial Revision
 *
 ***********************************************************************/
Boolean DisplayOptionsHandleEvent (EventType * event)
{
	Boolean updated;
	FormPtr frm;
	Boolean handled = false;

	switch(event->eType)
	{
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case DisplayOkButton:
					updated = DisplayOptionsApply ();
					FrmReturnToForm (0);
					if (updated)
						DateBkEnqueueEvent(datebookRefreshDisplay);
					break;

				case DisplayCancelButton:
					FrmReturnToForm (0);
					break;
			}
			handled = true;
			break;

		case frmOpenEvent:
			frm = FrmGetActiveForm ();
			DisplayOptionsInit ();
			handled = true;
			break;
	}

	return (handled);
}
