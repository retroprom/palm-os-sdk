/******************************************************************************
 *
 * Copyright (c) 2003-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SamplePinlet.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#include <FeatureMgr.h>
#include <Event.h>
#include <Form.h>
#include <Field.h>
#include <CmnLaunchCodes.h>
#include <Menu.h>
#include <MemoryMgr.h>
#include <Table.h>
#include <Window.h>
#include <Control.h>
#include <Loader.h>
#include <SystemMgr.h>
#include <StringMgr.h>
#include <UIResources.h>
#include <UIColor.h>
#include <Window.h>
#include <posix/time.h>
#include <posix/sys/time.h>
#include <PenInputMgr.h>
#include <Pinlet.h>



#	define B_MAKE_INT64(_x)		_x##L
#define B_ONE_NANOSECOND		(B_MAKE_INT64(1))
#define B_ONE_MICROSECOND		(B_ONE_NANOSECOND*1000)
#define B_ONE_MILLISECOND		(B_ONE_MICROSECOND*1000)
#define B_ONE_SECOND			(B_ONE_MILLISECOND*1000)

#define B_NANOSECONDS_TO_MILLISECONDS(ns)	((nsecs_t)(ns) / B_ONE_MILLISECOND)

#define MAIN_FORM				1000
#define SPEED_PUSH_GROUP		1
#define SPEED_5_PUSH_BUTTON		4001
#define SPEED_10_PUSH_BUTTON	4002
#define SPEED_20_PUSH_BUTTON	4003

// found this value online somewhere
#define kFIVEWORDSPERMINUTEDIT 240
#define kTENWORDSPERMINUTEDIT 120
#define kTWENTYWORDSPERMINUTEDIT 60
#define kINTERWORD             7
#define kINTERCHARACTER        3
#define kDAT                   3

time_t gDit;

char* gTreeArray;
DmOpenRef gFormDatabase = NULL;
Coord	gScreenX;
Coord	gScreenY;
uint8_t	gCurrentChar;
nsecs_t	gTim;
uint16_t gMode;
Boolean	gNewWord; // sitting idle causes a bunch of spaces to be pushed onto the buffer.  We'll use this boolean to limit it to one space per word.
Boolean	gPenStartedInZone;

void clearState(void);
uint16_t getInputMode(void);
void setInputMode(uint16_t mode);
void showReferenceDialog(void);

void nextChild(Boolean moveRight)
{
	// the way the tree is set up is that a dash moves you to the right
	// child (2*P+2) and a dot moves you to the left child (2*P+1)
	gCurrentChar *= 2;
	// as long as they don't move more than 2 pixels away from where they
	// placed the pen we'll consider it a dot
	if(moveRight)
		gCurrentChar += 2;
	else
		gCurrentChar += 1;
}

static Boolean SamplePinletFormHandleEvent (EventType * event)
{
	status_t	err = errNone;
	Boolean		handled	= false;
	FormType	*frmP	= FrmGetActiveForm();
	WinHandle	window	= FrmGetWindowHandle(frmP);
	nsecs_t		numMS = B_NANOSECONDS_TO_MILLISECONDS(system_time()) - gTim;
	uint16_t	numDits = (uint16_t)((B_NANOSECONDS_TO_MILLISECONDS(system_time()) - gTim) / gDit);

	switch(event->eType)
	{
		case frmOpenEvent:
			FrmSetControlGroupSelection(frmP, SPEED_PUSH_GROUP, SPEED_5_PUSH_BUTTON);
			handled = true;
			break;
		case frmUpdateEvent:
			{
				// *Note* I only draw during frmUpdate events
				RectangleType	rect;
				uint16_t		resourceIndex;

				FrmDrawForm(frmP);

				// Throw in a rounded rectangle to give the user a target
				FrmGetFormBounds(frmP, &rect);

				rect.topLeft.x += 5;
				rect.topLeft.y += 5;
				rect.extent.x -= 45;
				rect.extent.y -= 10;

				WinPaintRoundedRectangleFrame(&rect, 1, 4, 0);
			}
			handled = true;
			break;
		case ctlSelectEvent:
			switch(event->data.ctlSelect.controlID)
			{
				// we'll give the user a choice of three speeds.  Having a
				// slider here instead would be cool, especially if we could
				// mark the approximate areas for certain speeds.
				case SPEED_5_PUSH_BUTTON:
					gDit = (time_t)kFIVEWORDSPERMINUTEDIT;
					break;
				case SPEED_10_PUSH_BUTTON:
					gDit = (time_t)kTENWORDSPERMINUTEDIT;
					break;
				case SPEED_20_PUSH_BUTTON:
					gDit = (time_t)kTWENTYWORDSPERMINUTEDIT;
					break;
			}
			handled = true;
			break;
		case nilEvent:
			// nil events indicate that enough time has passed to determine
			// the current letter.
			if((gTreeArray[gCurrentChar] != ' ') && (gScreenX == -1))
			{
				wchar32_t newChar = (wchar32_t)gTreeArray[gCurrentChar];
				PINFeedChar(newChar, 0);
			}
			if(gNewWord)
			{
				PINFeedChar((wchar32_t)' ', 0);
				gNewWord = false;
			}
			gCurrentChar = 0;
			gTim = B_NANOSECONDS_TO_MILLISECONDS(system_time());

			handled = true;
			break;
		case penDownEvent:
			// I don't want to steal penDown events from the controls I have
			// in the pinlet.
			if(event->screenX<116)
			{
				// check to see how much time has passed, push the character if its been
				// long enough
				if(numDits > kINTERCHARACTER)
				{
					wchar32_t newChar = (wchar32_t)gTreeArray[gCurrentChar];
					PINFeedChar(newChar, 0);
					gCurrentChar = 0;
				}
				gTim = B_NANOSECONDS_TO_MILLISECONDS(system_time());
				gScreenX = event->screenX;
				gScreenY = event->screenY;
				gNewWord = true;
				gPenStartedInZone = true;

				handled = true;
			}
			else
				gPenStartedInZone = false;
			break;
		case penUpEvent:
			// Just making sure that the penDown that's paired with this penUp
			// was accepted
			if(gPenStartedInZone && event->screenX<116)
			{
				nextChild((numDits >= kDAT) ? true : false);
				gTim = B_NANOSECONDS_TO_MILLISECONDS(system_time());
				gScreenX = (Coord)-1;
				gScreenY = (Coord)-1;
				handled = true;
			}
			break;
	}

	return handled;
}

static Boolean ApplicationHandleEvent(EventType *event)
{
	if (event->eType == frmLoadEvent)
	{
		// Load the form resource.
		gFormDatabase = event->data.frmLoad.formDatabase;
		{
			uint16_t formID = event->data.frmLoad.formID;
			FormType *frm = FrmInitForm(gFormDatabase, formID);
			WinHandle handle = FrmGetWindowHandle(frm);
			WinSetDrawWindow(handle);
			FrmSetActiveForm(frm);

			FrmSetEventHandler(frm, &SamplePinletFormHandleEvent);
		}

		return (true);
	}
	return (false);
}

static void EventLoop (void)
{
	status_t error;
	EventType event;

	do {
		EvtGetEvent (&event, (int32_t)(kINTERWORD * gDit));

		if (!SysHandleEvent(&event)) {
			if (!MenuHandleEvent(0, &event, &error)) {
				if (!ApplicationHandleEvent(&event)) {
					FrmDispatchEvent(&event);
				}
			}
		}
	} while (event.eType != appStopEvent);
}

uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	MemHandle	resourceH;
	uint16_t	resourceIndex;

	if (cmd == sysAppLaunchCmdPinletLaunch)
	{
		DmOpenRef appdbP;
		RGBColorType color;
		SysGetModuleDatabase(SysGetRefNum(), NULL, &appdbP);

		resourceIndex = DmFindResource(appdbP, strRsc, 1020, NULL);
		resourceH = DmGetResourceByIndex(appdbP, resourceIndex);
		gTreeArray = (char*)DmHandleLock(resourceH);
		gCurrentChar = 0;
		gTim = 0;
		gScreenX = (Coord)-1;
		gScreenY = (Coord)-1;
		gDit = (time_t)kFIVEWORDSPERMINUTEDIT;
		gNewWord = false;
		gPenStartedInZone = false;

		// XXX - I don't know how I missed this, but pinlets like this are
		// using the system default colours for apps (white and black) instead
		// of the system colours we used for our other pinlets.  Hopefully I can
		// sneak in a fix for this soon, until then use the code below to
		// make your pinlet have the same colour schemes as the ones included
		// with the system.  Unless you want your pinlet to be white with
		// black...in that case you probably want to force it to white and
		// black so that when I do fix this you won't be affected!

		// set the object color
		color.r = 0xDC;
		color.b = 0xDC;
		color.g = 0xDC;
		(void)UIColorSetTableEntry(UIObjectFrame, &color);
		// set the object foreground color
		(void)UIColorSetTableEntry(UIObjectForeground, &color);
		// set the object selected fill color
		(void)UIColorSetTableEntry(UIObjectSelectedFill, &color);
		// set the text color
		color.r = 0x3C;
		color.b = 0x3C;
		color.g = 0x3C;
		(void)UIColorSetTableEntry(UIObjectFill, &color);
		// set the object selected foreground color
		(void)UIColorSetTableEntry(UIObjectSelectedForeground, &color);
		// set the background color
		(void)UIColorSetTableEntry(UIFormFill, &color);

		FrmGotoForm(appdbP, MAIN_FORM);

		EventLoop();

		DmHandleUnlock(resourceH);
		DmReleaseResource(resourceH);

		FrmCloseAllForms();
	}
	// check out the documentation for details about these function pointers
	// being loaded
	else if (cmd == sysPinletLaunchCmdLoadProcPtrs)
	{
		PinletAPIType* pinletAPIProcPtrs = (PinletAPIType*)cmdPBP;

		pinletAPIProcPtrs->pinletClearState = clearState;
		pinletAPIProcPtrs->pinletGetInputMode = getInputMode;
		pinletAPIProcPtrs->pinletSetInputMode = setInputMode;
		pinletAPIProcPtrs->pinletShowReferenceDialog = showReferenceDialog;
	}

	return 0;
}

// Right now I don't keep any state that's worth clearing (caps lock, etc)
void clearState(void)
{
}

uint16_t getInputMode(void)
{
	return 0;
}

void setInputMode(uint16_t mode)
{
	gMode = mode;
}

// I have a little pop-up for this.
void showReferenceDialog(void)
{
	uint16_t hit;
	DmOpenRef appdbP;
	FormType * frmP;

	SysGetModuleDatabase(SysGetRefNum(), NULL, &appdbP);
	frmP = FrmInitForm(appdbP, 1001);
	if(frmP != NULL)
	{
		hit = FrmDoDialog(frmP);
		FrmDeleteForm(frmP);
	}
}
