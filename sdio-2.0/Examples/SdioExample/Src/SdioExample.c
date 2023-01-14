/******************************************************************************
 *
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: SdioExample.c
 *
 * Release: palmOne SDIO SDK 2.0
 *
 * Description:
 *
 *To start this App looking for a SD I/O card, just start it.
 * (pressing reset or deleteing this app removes all its hooks)
 *Then exit this App.
 *Insert palmOne test SDIO card, and this app will auto start.
 *You can control the LED color, and press the interrupt button to alternate
 *	the color of the LED.
 *The interrupt button on the SDIO card toggles the LED green/red.
 *leave power on to the card, & put handheld to sleep to demostrate
 *	the SDIO interrupt wake up the handheld
 *
 *What this Does:
 * - detects a specific SDIO, via the autoRun notification and starts this application
 *	(you must first start this app so that it will hook into the autorun notification, then quit this app)
 * - Makes SDIO API calls
 * - uses callbacks to monitor the cards power state
 * - uses the SDIO interrupt to toggle the LED color.
 * - gives feed back to a UI
 * - deals with an interrupt routine and its limited function calls
 *
 *What this does not do:
 * - add it hooks after being loaded onto the handheld (ie hotsync, beaming, card from card,...)
 * - re-install itself after a soft reset
 *		Launch command: sysAppLaunchCmdSystemReset can be used to do this
 *		Use PrefGetAppPreferences() & PrefSetAppPreferences() to save information that can survive a reset
 * - Once you run this application it is locked & you cannot replace it via hotsync
 *		Notifications sysNotifySyncStartEvent & sysNotifySyncFinishEvent can be use to do this
 * - Once you run this application it is locked & you cannot replace it via beaming or by a copy from a card
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <SdIO.h>
#include <ExpansionMgr.h>	// Expansion manager public definitions
#include <VFSMgr.h>
#include <FeatureMgr.h>

#include "SdioExampleRsc.h"
#include "SdioExample.h"

/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/
//(intentionally none), see workspaceCreate() and workspaceGet()

#pragma mark ----
#pragma mark (Work Space Functions)----
/***********************************************************************
 *
 * FUNCTIONS:    workspaceGet, workspaceCreate, workspaceDestroy
 *
 * DESCRIPTION: Basic functions of a real global workspace that can exist
 *			when an application has stopped.
 *
 *	Note, If a soft reset occures, the memory manager re-initializes memory
 *			and your global workspace is gone.
 *  Note, Feature manager entries also get delete upon a soft (or hard) reset
 *			This makes feature manager ideal for memory pointers
 *
 *  Note, these are NOT interrupt safe!
 *
 ***********************************************************************/
Err workspaceGet( WorkSpaceType **workspacePP)
{
Err err;
FtrWorkSpaceType *ftrWorkSpaceP;

	*workspacePP = NULL;
	err = FtrGet( appFileCreator, SdioExFeatureWorkSpace ,(UInt32 *)&ftrWorkSpaceP );
	if ( !err )
	{
		*workspacePP = ftrWorkSpaceP->workspaceP;
		if ( *workspacePP == NULL )
			err = memErrNotEnoughSpace;
	}
	return( err );
}
//
//
//
Err workspaceCreate( WorkSpaceType **workspacePP)
{
Err err;
FtrWorkSpaceType *ftrWorkSpaceP;
FtrWorkSpaceType ftrWorkSpace;

	*workspacePP = NULL;
	ftrWorkSpace.workspaceP = MemPtrNew( sizeof( WorkSpaceType ) );
	MemSet( ftrWorkSpace.workspaceP, sizeof(*ftrWorkSpace.workspaceP), 0 );
	MemPtrSetOwner(ftrWorkSpace.workspaceP, 0);
	
	if ( ftrWorkSpace.workspaceP == NULL )
	{
		err = memErrNotEnoughSpace;
		goto Error;
	}
	err = FtrPtrNew( appFileCreator, SdioExFeatureWorkSpace, sizeof(*ftrWorkSpaceP), (void **)&ftrWorkSpaceP );
	if ( err )
	{
		goto Error;
	}
	err = DmWrite( ftrWorkSpaceP, 0, &ftrWorkSpace, sizeof(ftrWorkSpace) );
	if ( err )
	{
		goto Error;
	}
	*workspacePP = ftrWorkSpace.workspaceP;
Error:
	if ( err )
	{
		if ( ftrWorkSpace.workspaceP )
			MemPtrFree( ftrWorkSpace.workspaceP );
		FtrUnregister( appFileCreator, SdioExFeatureWorkSpace );
	}
	return( err );
}
//
//
//
Err workspaceDestroy( void )
{
Err err;
WorkSpaceType *workspaceP;
	
	err = workspaceGet( &workspaceP );
	if ( !err && workspaceP )
		MemPtrFree( workspaceP );
	FtrUnregister( appFileCreator, SdioExFeatureWorkSpace );
	return( errNone );
}

#pragma mark ----
#pragma mark (SDIO Function & UI Routines)----

/***********************************************************************
 *
 * FUNCTIONS:    FunctionStateUpdate
 *
 * DESCRIPTION: An Interrupt safe (NO UI! and NO application global variables!)
 *		routine to talk to the SD I/O card.
 *
 * Note: this is called by an interrupt routine,
 *			only interrupt safe routines can be called!
 *
 ***********************************************************************/
void FunctionStateUpdate( WorkSpaceType *workspaceP, UInt16 forceUpdate )
{
SDIORWDirectType direct;
Err err;
Err errIsr;

	if ( !workspaceP->boardState.correctCardInserted )
		return;

	//
	// Disable SDIO interrupts, avoid All re-entrancy problems within this routine..
	// Warning, if you change this code, beware of workspace re-enterance issues
	//		ie, the workspaceP->boardState.ledColorUpdate is changed by interrupt
	//		and Non-interrupt routines. In this case the non-interrupt routine
	//		does an "=" or a single move instruction making it safe from the
	//		interrupt routine.
	//
	errIsr = SDIODisableHandheldInterrupt( workspaceP->slotDriver.slotLibRefNum );
	//
	// From here on the only "unexpected" callback is "power on"
	//

	//
	// update the sleep with power on/off & auto off timer disable/enable
	//
	if ( forceUpdate || workspaceP->boardState.sleepWithCardOnUpdate )
	{
	SDIOAutoPowerOffType autoPower;
	
		workspaceP->boardState.sleepWithCardOnUpdate = 0;
		workspaceP->uiState.sleepWithCardOnUpdate = 1;
		
		if ( workspaceP->boardState.sleepWithCardOn && !workspaceP->boardState.functionPowered )
		{	//Power on the card if it is off and sleep with card on is being set
		SDIOPowerType power;
		
			workspaceP->uiState.functionPoweredUpdate = 1;

			power.sdioSlotNum = sdioSlotFunc1;
			power.powerOnCard = sdioCardPowerOn;
			err = SDIOSetPower(  workspaceP->slotDriver.slotLibRefNum, &power );
			if ( err )
			{
				workspaceP->boardState.functionPoweredUpdate = 1;
				goto Error;
			}
		}
		
		autoPower.sdioSlotNum = sdioSlotFunc1;
		if ( workspaceP->boardState.sleepWithCardOn )
		{
			autoPower.ticksTillOff = 0;		//No auto-off time
			autoPower.sleepPower = true;	//Sleep with SD I/O card on		
		}
		else
		{
			autoPower.ticksTillOff = SysTicksPerSecond()*15;	//restore auto off-time
			autoPower.sleepPower = false;						//sleep with SD I/O card off
		}
		err = SDIOSetAutoPowerOff( workspaceP->slotDriver.slotLibRefNum, &autoPower );
		if ( err )
		{
			workspaceP->boardState.sleepWithCardOnUpdate = 1;
			goto Error;
		}
	}

	//
	// Enable the function (IOE1 in the CCCR)
	//	Note: this card does not use the I/O ready bit IOR1. So it is ignored.
	//		Other cards might have to wait until the card finishes its reset procedures..
	//
	if ( forceUpdate || workspaceP->boardState.ioe1Update )
	{
		workspaceP->uiState.sleepWithCardOnUpdate = 1;
		workspaceP->boardState.ioe1Update = 0;
			
		direct.requestingFunc = sdioSlotFunc1;
		direct.mode = sdioRWModeRead;
		direct.funcNum = sdioFunc0;
		direct.byteAddress = sdiofunc0CCCRAddressIOE;	
		err = SDIORWDirect( workspaceP->slotDriver.slotLibRefNum, &direct );
		if ( err )
		{
			workspaceP->boardState.ioe1Update = 1;
			goto Error;
		}
		if ( workspaceP->boardState.ioe1 )
		{
			direct.byteData |= (sdiofunc0CCCRAddressIOE | 0x01); //The 0x01 is a glitch from the past....
		}
		else
		{
			direct.byteData &= ~(sdiofunc0CCCRAddressIOE | 0x01);
		}
		direct.mode = sdioRWModeWrite;
		err = SDIORWDirect( workspaceP->slotDriver.slotLibRefNum, &direct );
		if ( err )
		{
			workspaceP->boardState.ioe1Update = 1;
			goto Error;
		}
		workspaceP->boardState.functionPowered = 1;		//This operation turns on a card
		workspaceP->boardState.functionPoweredUpdate = 1;
		workspaceP->uiState.functionPoweredUpdate = 1;
		if ( workspaceP->boardState.ioe1 )
		{
			workspaceP->boardState.ledColorUpdate = 1;
			workspaceP->uiState.ledColorUpdate = 1;
		}
	}

	//
	// Enable the function 1 interrupt (IEN1 in the CCCR) and the cards master
	// interrupt IENM.
	//  Note, interrupt callbacks are already assigned
	//
	if ( forceUpdate || workspaceP->boardState.ien1Update )
	{
		workspaceP->boardState.ien1Update = 0;
		workspaceP->uiState.ien1Update = 1;
		
		direct.requestingFunc = sdioSlotFunc1;
		direct.mode = sdioRWModeRead;
		direct.funcNum = sdioFunc0;
		direct.byteAddress = sdiofunc0CCCRAddressIEN;	
		err = SDIORWDirect( workspaceP->slotDriver.slotLibRefNum, &direct );
		if ( err )
		{
			workspaceP->boardState.ien1Update = 1;
			goto Error;
		}
		if ( workspaceP->boardState.ien1 )
		{
			direct.byteData |= (sdiofunc0CCCRDataIENM | sdiofunc0CCCRDataIEN1 );
		}
		else
		{
			direct.byteData &= ~(sdiofunc0CCCRDataIENM | sdiofunc0CCCRDataIEN1 );
		}
		direct.mode = sdioRWModeWrite;
		err = SDIORWDirect( workspaceP->slotDriver.slotLibRefNum, &direct );
		if ( err )
		{
			workspaceP->boardState.ien1Update = 1;
			goto Error;
		}
		workspaceP->boardState.functionPowered = 1;		//This operation turns on a card
		workspaceP->boardState.functionPoweredUpdate = 1;
		workspaceP->uiState.functionPoweredUpdate = 1;
	}
	
	if ( workspaceP->boardState.functionPoweredUpdate && workspaceP->boardState.functionPowered )
		workspaceP->boardState.ledColorUpdate = 1;

	//
	// Now here is an interesting twist...
	//If you turn on the card by the below SDIORWDirect() call, the "power on" callback
	//will be called, and IT also calls this function (Be sure this function is re-enterant!).
	//BUT it enables the function and interrupts first before turning on the led to the
	//workspaces color. Then after "power on" is done, it returns here to turn on the LED (again).
	//Perhaps a better method is to always turn on the card before setting the led!
	//
	//Note to see this, attach the handheld to a serial port and trun on the 'command' debug trace.
	//
	if ( forceUpdate || workspaceP->boardState.ledColorUpdate )
	{
	
		workspaceP->boardState.ledColorUpdate = 0;
		workspaceP->uiState.ledColorUpdate = 1;
		workspaceP->boardState.functionPoweredUpdate = 1;
		workspaceP->uiState.functionPoweredUpdate = 1;
	
	
		//Function 1 is a write only port, so build a copy of the function 1 byte in direct.byteData
		switch( workspaceP->boardState.ledColor )
		{
		case LED_RED:
			direct.byteData =  palmOneTestSDIOCardFunc1Addr0LedOn|palmOneTestSDIOCardFunc1Addr0LedRed;
			break;
			
		case LED_GREEN:
			direct.byteData =  palmOneTestSDIOCardFunc1Addr0LedOn|palmOneTestSDIOCardFunc1Addr0LedGreen;
			break;
		default: 	//LED_OFF
			direct.byteData =  0;
			workspaceP->boardState.ledColor = LED_OFF;	//Force a valid choice
			break;
		}
		direct.requestingFunc = sdioSlotFunc1;
		direct.mode = sdioRWModeWrite;
		direct.funcNum = sdioFunc1;
		direct.byteAddress = palmOneTestSDIOCardFunc1Addr0;
		err = SDIORWDirect( workspaceP->slotDriver.slotLibRefNum, &direct );
		if ( err )
		{
			workspaceP->boardState.ledColorUpdate = 1;
			goto Error;
		}
	}

	//
	// This is to turn off a function (not necessarily the card!).
	//Note: Usally when a card is inserted, the functions CSA (file system) timer is triggered.
	//	This means that the card is powered for 15 seconds after insertion. Even if this is a single
	//	function card, without a file system, and the function is turned off! This allows
	//	programs/drivers like this to access the card without re-powering "on" the card.
	//
	if ( forceUpdate || workspaceP->boardState.functionPoweredUpdate )
	{
		workspaceP->boardState.functionPoweredUpdate = 0;
		workspaceP->uiState.functionPoweredUpdate = 1;
		if ( !workspaceP->boardState.functionPowered )
		{
		SDIOPowerType power;
		
			power.sdioSlotNum = sdioSlotFunc1;
			power.powerOnCard = sdioCardPowerOff;
			err = SDIOSetPower(  workspaceP->slotDriver.slotLibRefNum, &power );
			if ( err )
			{
				workspaceP->boardState.functionPoweredUpdate = 1;
				goto Error;
			}
		}	
	}
Error:
	//
	// re-enable SDIO interrupts
	//
	if ( errIsr == errNone )
		SDIOEnableHandheldInterrupt( workspaceP->slotDriver.slotLibRefNum );

	return;
}	

/***********************************************************************
 *
 * FUNCTIONS:    UIStateUpdate
 *
 * DESCRIPTION: Update the UI of the main form within this application.
 *		This assumes that the main form is visible to the user....
 *
 ***********************************************************************/
void UIStateUpdate( WorkSpaceType *workspaceP, UInt16 forceUpdate )
{
FormPtr frmP = FrmGetActiveForm();

	if ( workspaceP->uiAvailable != 1 )
		return;	//Only refresh the screen when the window is being displayed!

	workspaceP->uiRefreshEventsEnqueued = 0;	//UI is being refershed
	if ( forceUpdate || workspaceP->uiState.sleepWithCardOnUpdate )
	{
		if ( workspaceP->boardState.sleepWithCardOn )
		{
			FrmSetControlGroupSelection( frmP, MyMainGroupIDSleepWithCard, MainSleepWithCardOnPushButton );
		}
		else
		{
			FrmSetControlGroupSelection( frmP, MyMainGroupIDSleepWithCard, MainSleepWithCardOffPushButton );
		}
		workspaceP->uiState.sleepWithCardOnUpdate = 0;
	}
	if ( forceUpdate || workspaceP->uiState.correctCardInsertedUpdate || workspaceP->uiState.cardInsertedUpdate )
	{
		workspaceP->uiState.correctCardInsertedUpdate = 0;
		workspaceP->uiState.cardInsertedUpdate = 0;
		
		if ( workspaceP->boardState.correctCardInserted )
		{
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainInsertTestCardLabel ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainWrongCardLabel ) );
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainLEDGreenPushButton ) );				
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainLEDRedPushButton ) );
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainLEDOffPushButton ) );				
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainFunc1OFFPushButton ) );				
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainSleepWithCardOffPushButton ) );				
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainSleepWithCardOnPushButton ) );
			FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainAutoOffLabel ) );
			workspaceP->uiState.ledColorUpdate = 1;
		}
		else
		{
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainLEDGreenPushButton ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainLEDRedPushButton ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainLEDOffPushButton ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainFunc1OFFPushButton ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainSleepWithCardOffPushButton ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainSleepWithCardOnPushButton ) );
			FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainAutoOffLabel ) );
			if ( workspaceP->boardState.cardInserted )
			{
				FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainInsertTestCardLabel ) );	
				FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainWrongCardLabel ) );
			}
			else
			{
				FrmHideObject( frmP, FrmGetObjectIndex (frmP, MainWrongCardLabel ) );	
				FrmShowObject( frmP, FrmGetObjectIndex (frmP, MainInsertTestCardLabel ) );				
			}
		}		
	}
	if ( forceUpdate || workspaceP->uiState.ledColorUpdate  || workspaceP->uiState.functionPoweredUpdate)
	{
		workspaceP->uiState.ledColorUpdate = 0;
		workspaceP->uiState.functionPoweredUpdate = 0;
		
		if (  workspaceP->boardState.correctCardInserted )
		{
		UInt16 controlID = MainFunc1OFFPushButton;

			if ( workspaceP->boardState.functionPowered )
			{
				switch(  workspaceP->boardState.ledColor )
				{
				case LED_GREEN:
					controlID = MainLEDGreenPushButton;
					break;
					
				case LED_RED:
					controlID = MainLEDRedPushButton;
					break;
					
				default:	//case LED_OFF
					controlID = MainLEDOffPushButton;
					break;
				}	
			}
			FrmSetControlGroupSelection( frmP, MyMainGroupIDLEDColor, controlID );
		}
	}

	return;
}	


#pragma mark ----
#pragma mark (SDIO I/O Callbacks)----
/***********************************************************************
 *
 * FUNCTIONS:    SD I/O Callbacks
 *
 * DESCRIPTION: These cannot use a UI, but they can use the SD I/O routines
 *
 ***********************************************************************/
Err MySdioCallBackSelectSleep( SDIOSlotType sdioSlotNum, void *userDataP )
{
#pragma unused (sdioSlotNum )
WorkSpaceType *workspaceP = (WorkSpaceType *)userDataP;

	workspaceP->handheldAsleep = 1;		//Handheld is going asleep, NO UI available!
	return( errNone );
}

Err MySdioCallBackSelectAwake( SDIOSlotType sdioSlotNum, void *userDataP )
{	//Turn card back on when handheld wakes up,
	//a better (less power) solution is to access the card (which turns it on) when needed
WorkSpaceType *workspaceP = (WorkSpaceType *)userDataP;

	workspaceP->handheldAsleep = 0;		//Handheld is no longer asleep
	MySdioCallBackSelectPowerOn( sdioSlotNum, userDataP );
	return( errNone );
}

Err MySdioCallBackSelectPowerOn( SDIOSlotType sdioSlotNum, void *userDataP )
{
#pragma unused (sdioSlotNum )
WorkSpaceType *workspaceP = (WorkSpaceType *)userDataP;

	if ( !workspaceP->boardState.functionPowered )
	{
		MyCardEnable( workspaceP, true );
		workspaceP->boardState.functionPowered = 1;
		workspaceP->uiState.functionPoweredUpdate = 1;
		workspaceP->boardState.functionPoweredUpdate = 1;
		FunctionStateUpdate( workspaceP, false );
		MyRefreshAppInterrupt( workspaceP ); //Interrupt safe way to queue up an Update of the UI, (if needed)	
	}
	return(errNone);
}


//
// Warning Code Warrior breakpoints do not seem to work when going to sleep and powering off.
// (they work if you are awake, and turning off the card...)
// Use DbgMessage() for debugging.
// Note: Since this can be used when going to sleep, this rotuine should be FAST!
//
Err MySdioCallBackSelectPowerOff( SDIOSlotType sdioSlotNum, void *userDataP )
{
#pragma unused (sdioSlotNum )
WorkSpaceType *workspaceP = (WorkSpaceType *)userDataP;

	//DbgMessage("Power Off");
	MyCardPowerOff( workspaceP, false );	//Modify my internal workspace to match tha card state
	//Although the card is still powered and ready to accept commands,
	//it is faster to not send commands to the card. (since it will be turned off)
	if ( workspaceP->handheldAsleep )
	{	
		; //Warning, do nothing if powering off and going to sleep... there is NO UI at this time...
	}
	else
	{
		MyRefreshAppInterrupt( workspaceP ); //Interrupt safe way to queue up an Update of the UI, (if needed)
	}
	return(errNone);
}


Err MySdioCallBackSelectReset( SDIOSlotType sdioSlotNum, void *userDataP )
{
#pragma unused (sdioSlotNum )
WorkSpaceType *workspaceP = (WorkSpaceType *)userDataP;

	MyCardSdioReset( workspaceP, false );
	MyCardEnable( workspaceP, true );	
	FunctionStateUpdate( workspaceP, false );	//restore the state of the function before the reset
	return(errNone);
}
//**********************************************************************************************************
//Warning, this is an interrupt routine,
//The amount callable interrupt safe routines is extremely limited.
//Only use an OS function if it says (in print) that it is interrupt safe!
//
//Well designed SD I/O cards should seperate the I/O ports for interrupt routines & non-interrupt routines.
//If this is not possible, you must deal with the re-entrancy issues (ie a non-interrupt routine reading
//a register, modifying it contents, the writting the register. If an interrupt routine occures before the
//write and it writes something else to the same register, the non-interrupt write will overwrite the
//interrupt write)
//
//	You might want to impliment a lock count. The function FunctionStateUpdate() would increment it
//	at the start of the function, and decrement it at the end of the function. if the decrement is zero
//	check for a "delayed interrupt flag" that indicates that the interrupt occured and if so, service it
//	(repeating the inc/dec procedures).
//	Then the interrupt routine can monitor this lock count and if non-zero clear the interrupt and set the
//	"delayed interrupt flag".
//
//
Err MySdioCallBackSelectInterruptSdCard( SDIOSlotType sdioSlotNum, void *userDataP )
{
#pragma unused (sdioSlotNum )
WorkSpaceType *workspaceP = (WorkSpaceType *)userDataP;

	switch( workspaceP->boardState.ledColor )
	{
		case LED_RED:
			workspaceP->boardState.ledColor = LED_GREEN;
			break;

		default:	//LED_GREEN, LED_OFF, LED_UNKNOWN
			workspaceP->boardState.ledColor = LED_RED;
			break;
	}
	workspaceP->boardState.ledColorUpdate = 1;
	workspaceP->uiState.ledColorUpdate = 1;
	FunctionStateUpdate( workspaceP, false );	//A new LED state also resets the interrupt
	MyRefreshAppInterrupt( workspaceP ); //Interrupt safe way to queue up an Update of the UI, (if needed)
	return(errNone);
}
//
// These Slot Driver call backs can only be installed after a card is inserted
//	and are automatically erased when a card is removed.
//
Err MyAddCallBacks( WorkSpaceType *workspaceP, UInt16 install )
{
Err err = errNone;
SDIOCallbackType callBack;

	if ( !workspaceP->boardState.correctCardInserted )
	{
		workspaceP->slotDriverCallBacksInstalled = 0;
		goto Done;
	}
	if ( install )
	{
		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectInterruptSDCard;
		callBack.callBackP = MySdioCallBackSelectInterruptSdCard;
		callBack.userDataP = workspaceP;
		err = SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		if ( err )
			goto Done;
		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectPowerOn;
		callBack.callBackP = MySdioCallBackSelectPowerOn;
		callBack.userDataP = workspaceP;
		err = SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		if ( err )
			goto Done;
		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectPowerOff;
		callBack.callBackP = MySdioCallBackSelectPowerOff;
		callBack.userDataP = workspaceP;
		err = SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		if ( err )
			goto Done;
		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectAwake;
		callBack.callBackP = MySdioCallBackSelectAwake;
		callBack.userDataP = workspaceP;
		err = SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		if ( err )
			goto Done;

		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectSleep;
		callBack.callBackP = MySdioCallBackSelectSleep;
		callBack.userDataP = workspaceP;
		err = SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		if ( err )
			goto Done;

		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectReset;
		callBack.callBackP = MySdioCallBackSelectReset;
		callBack.userDataP = workspaceP;
		err = SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		if ( err )
			goto Done;
		workspaceP->slotDriverCallBacksInstalled = 1;
	}
	else
	{
		callBack.sdioSlotNum = sdioSlotFunc1;
		callBack.callbackSelect = sdioCallbackSelectInterruptSDCard;
		callBack.callBackP = NULL;
		callBack.userDataP = NULL;
		SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		callBack.callbackSelect = sdioCallbackSelectPowerOn;
		SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		callBack.callbackSelect = sdioCallbackSelectPowerOff;
		SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		callBack.callbackSelect = sdioCallbackSelectAwake;
		SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		callBack.callbackSelect = sdioCallbackSelectSleep;
		SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		callBack.callbackSelect = sdioCallbackSelectReset;
		SDIOSetCallback( workspaceP->slotDriver.slotLibRefNum, &callBack );
		workspaceP->slotDriverCallBacksInstalled = 0;
	}
Done:
	return ( err );
}

#pragma mark ----
#pragma mark (Notifications)----
/***********************************************************************
 *
 * FUNCTIONS:    Notifications
 *
 * DESCRIPTION: These are not interrupt routines. And they can occure even when
 *		the application has stopped (so the UI from SdioExample.prc might not be running!)
 *
 *Note: Not all notifications should have the "handled" flag set.
 *		some just monitor the events as they pass.....
 ***********************************************************************/
//
// This is an "Interrupt safe" way to enqueue a UI refresh event (step 2 of 2).
// OR method of converting an Interrupt routine to a non-interrupt routine so that
// more OS function calls are available, in his case a form update.
//
//see MyRefreshAppInterrupt() for step 1 of 2
//
Err PrvNotifySiExRefresEventProc(SysNotifyParamType *notifyParamsP)
{
WorkSpaceType *workspaceP = (WorkSpaceType *)notifyParamsP->userDataP;

	if ( workspaceP->mainFormFormID && (workspaceP->uiAvailable == 1) )
		FrmUpdateForm( workspaceP->mainFormFormID,-1);	//send a UI frmUpdateEvent!
	return( errNone );
}

//
// epansion manager's card removal
//
Err PrvNotifyCardRemovedEventProc(SysNotifyParamType *notifyParamsP)
{
UInt16 slotRefNum = (UInt16)notifyParamsP->notifyDetailsP;	//Convert to reality
WorkSpaceType *workspaceP = (WorkSpaceType *)notifyParamsP->userDataP;

	if ( !workspaceP->boardState.correctCardInserted )
		goto Done;	//This was never inserted, so it cannot be removed...
	if ( slotRefNum != workspaceP->slotDriver.slotRefNum )
		goto Done;	//Another Slot got removed
	workspaceP->boardState.correctCardInserted = 0;
	workspaceP->boardState.correctCardInsertedUpdate = 1;
	workspaceP->uiState.correctCardInsertedUpdate = 1;
	workspaceP->boardState.cardInserted = 0;
	workspaceP->boardState.cardInsertedUpdate = 1;
	workspaceP->uiState.cardInsertedUpdate = 1;
	workspaceP->slotDriverCallBacksInstalled = 0;	//ALL callbacks are instantly removed, when a card is removed

	MyCardPowerOff( workspaceP, false );		//Mimic the card being powered off.
	MyRefreshAppInterrupt( workspaceP );

Done:
	return(errNone);
}
//
// AutoRun event
//
Err PrvNotifyDriverSearchEventProc(SysNotifyParamType *notifyParamsP)
{
AutoRunInfoType *autoRunP = (AutoRunInfoType *)notifyParamsP->notifyDetailsP;	//Convert to reality
WorkSpaceType *workspaceP = (WorkSpaceType *)notifyParamsP->userDataP;

	if ( notifyParamsP->handled )
		goto Skip;	//Too late, Already handled by someone else.....

	if ( MyAppAutoRunCk( workspaceP, autoRunP ) )
	{
		if ( !LaunchApp( appFileCreator ) )	//start the App
		{	//Could not launch App, it might be running already, so try to refresh the UI!
			MyRefreshAppInterrupt( workspaceP );
		}
		notifyParamsP->handled = true;
	}
Skip:	
	return( errNone );
}

//
// The Launcher's Delete an Application Notification (undo the hooks, then allow to be deleted)
//
//	Warning: when debugging with code warrior, it locks this app and prevents
//		it from being deleted and does not call this Notification
//		until you quit the debugger AND the handheld resets!
//		You can only test this Notification with the debugger and DbgBreak(); or DbgMessage();
//
Err PrvNotifyDeleteProtectedEventProc(SysNotifyParamType *notifyParamsP)
{
SysNotifyDBInfoType *dbInfoP = (SysNotifyDBInfoType *)notifyParamsP->notifyDetailsP;	//Convert to reality
WorkSpaceType *workspaceP = (WorkSpaceType *)notifyParamsP->userDataP;

	//DbgBreak();	//See debug warning above
	if ( (dbInfoP->creator == appFileCreator) && (dbInfoP->type == appFileCreatorType) )
	{	//This is me being deleted, release resources & unlock DB's
		MyAddCallBacks( workspaceP, false );	//remove card hooks
		MyAddHooks( workspaceP, false );		//remove system hooks
		workspaceDestroy();						//remove my workspace
	}
	return( errNone );
}

//
// These "Hooks" are installed and left installed even when the App has stopped
//	and even when a card is removed.
//Note: These hooks can be created by this Application, but destroyed by the launcher APP when this APP is deleted!
//
Err MyAddHooks( WorkSpaceType *workspaceP, UInt16 install )
{
Err err;
LocalID dbID;

	dbID = DmFindDatabase(0, appFileCreatorName);	//Find DB ID of this Application!
	if( dbID == NULL)
	{
		err = DmGetLastErr();
		if ( err==errNone )
			err = memErrNotEnoughSpace;
		ErrFatalDisplay("can't get dbID for Notifys");
	}
	if ( install )
	{
		//Register for My App's UI update
		if ( !workspaceP->notifySiExRefresEventRegistered )
		{
			err = SysNotifyRegister(0, dbID, appNotifySiExRefresEvent, PrvNotifySiExRefresEventProc, PrvNotifySiExRefreshEventPrio, workspaceP );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyRegister() for appNotifySiExRefresEvent");		
			}
			else
			{
				workspaceP->notifySiExRefresEventRegistered = 1;
			}
		}
		//Register for Card removals notifications
		if ( !workspaceP->notifyCardRemovedEventRegistered )
		{
			err = SysNotifyRegister(0, dbID, sysNotifyCardRemovedEvent, PrvNotifyCardRemovedEventProc, PrvNotifyCardRemovedEventPrio, workspaceP );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyRegister() for sysNotifyCardRemovedEvent");		
			}
			else
			{
				workspaceP->notifyCardRemovedEventRegistered = 1;
			}
		}
		//Register for the launchers pre-delete application notifications
		if ( !workspaceP->notifyDeleteProtectedEventRegistered )
		{
			err = SysNotifyRegister(0, dbID, sysNotifyDeleteProtectedEvent, PrvNotifyDeleteProtectedEventProc, PrvNotifyDeleteProtectedEventPrio, workspaceP );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyRegister() for sysNotifyDeleteProtectedEvent");		
			}
			else
			{
				workspaceP->notifyDeleteProtectedEventRegistered = 1;
			}
		}
		//Register for AutoRun notifications
		if ( !workspaceP->notifyDriverSearchEventRegistered )
		{
			err = SysNotifyRegister(0, dbID, sysNotifyDriverSearch, PrvNotifyDriverSearchEventProc, PrvNotifyDriverSearchEventPrio, workspaceP );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyRegister() for sysNotifyDriverSearch event");		
			}
			else
			{
				workspaceP->notifyDriverSearchEventRegistered = 1;
			}
		}
		//Protect this 	app (activate sysNotifyDeleteProtectedEvent),
		//& prevent memory manager from moving it since we use absolute notification & callback addresses
		if ( !workspaceP->appDmDatabaseProtect )
		{
			// Protect this database from being deleted
			err = DmDatabaseProtect(0, dbID, true);
			if ( err == errNone )
			{
			MemHandle h;
			
				// and lock down this APP code resource from being moved by memory manager
				h = DmGet1Resource(sysResTAppCode, 0);
				MemHandleLock(h);
				workspaceP->appDmDatabaseProtect = 1;
			}
		}
	}
	else
	{
		if ( workspaceP->notifySiExRefresEventRegistered )
		{
			err = SysNotifyUnregister(0, dbID, appNotifySiExRefresEvent, PrvNotifySiExRefreshEventPrio );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyUnregister() for appNotifySiExRefresEvent");		
			}
			else
			{
				workspaceP->notifySiExRefresEventRegistered = 0;
			}
		}
		if ( workspaceP->notifyCardRemovedEventRegistered )
		{
			err = SysNotifyUnregister(0, dbID, sysNotifyCardRemovedEvent, PrvNotifyCardRemovedEventPrio );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyUnregister() for sysNotifyCardRemovedEvent");		
			}
			else
			{
				workspaceP->notifyCardRemovedEventRegistered = 0;
			}
		}
		if ( workspaceP->notifyDeleteProtectedEventRegistered )
		{
			err = SysNotifyUnregister(0, dbID, sysNotifyDeleteProtectedEvent, PrvNotifyDeleteProtectedEventPrio );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyUnregister() for sysNotifyDeleteProtectedEvent");		
			}
			else
			{
				workspaceP->notifyDeleteProtectedEventRegistered = 0;
			}
		}	
		if ( workspaceP->notifyDriverSearchEventRegistered )
		{
			err = SysNotifyUnregister(0, dbID, sysNotifyDriverSearch, PrvNotifyDriverSearchEventPrio );
			if ( err )
			{
				ErrFatalDisplay("can't SysNotifyUnregister() for sysNotifyDriverSearch event");		
			}
			else
			{
				workspaceP->notifyDriverSearchEventRegistered = 0;
			}
		}	
		if ( workspaceP->appDmDatabaseProtect )
		{
			// Protect this database from being deleted
			err = DmDatabaseProtect(0, dbID, false);
			if ( err == errNone )
			{
			MemHandle h;
			
				// and lock down this APP code resource
				h = DmGet1Resource(sysResTAppCode, 0);
				MemHandleUnlock(h);
			}
			workspaceP->appDmDatabaseProtect = 0;
		}
	}
	return( errNone );
}
#pragma mark ----
#pragma mark (My Utilities)----

//
// Set SDIO debug trace flags at any time!
//
static Err SdioDebugOptionsSet( SDIODebugOptionType debugOption )
{
Err err = errNone;
UInt32 slotIterator;
UInt16 slotRefNum;
UInt16 slotLibRefNum;
UInt32 mediaType;
UInt16 count=0;

	slotIterator = expIteratorStart;
	while( slotIterator != expIteratorStop )
	{
		err =  ExpSlotEnumerate( &slotRefNum, &slotIterator );
		if ( err )
		{
			break;
		}
		err = ExpSlotLibFind(slotRefNum,  &slotLibRefNum);
		if ( !err )
		{
			err = SlotMediaType( slotLibRefNum, slotRefNum, &mediaType );
			if ( !err )
			{
				if ( mediaType == expMediaType_SecureDigital )
				{
					err = SDIODebugOptions( slotLibRefNum, &debugOption );
					if ( err == errNone )
						count++;
				}
			}
		}
	}
	if ( count == 0 )
		err = expErrNotOpen;
	return( err );
}

/***********************************************************************
 *
 * FUNCTION:    LaunchApp
 *
 * DESCRIPTION: 	Launch this app... (if it is not running)
 *
 * PARAMETERS:  creator - creator ID of the APP to launch
 *
 * RETURNED:    false if App is not launched (for any reason)
 *
 ***********************************************************************/
UInt16 LaunchApp(UInt32 creator)
{
UInt16					cardNo;
LocalID					dbID;
UInt32					curCreator;	
Err						err;
DmSearchStateType		searchState;									

	// Get the creator type of the currently running app to see if the
	//  User hit the same button
	curCreator = 0;
	err = SysCurAppDatabase(&cardNo, &dbID);
	if (!err)
		err = DmDatabaseInfo(cardNo, dbID, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &curCreator);
						
	if (curCreator == creator)
	{	//App is currently running, so send it a refresh instead
	  	return ( false );
	}

	err = DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, creator, true, &cardNo, &dbID);
	if (!err && dbID)
	{
		SysUIAppSwitch(cardNo, dbID, sysAppLaunchCmdNormalLaunch, (MemPtr)0);
		return( true );
	}
	return( false );
}

/***********************************************************************
 *
 * FUNCTION:    MyRefreshAppInterrupt
 *
 * DESCRIPTION: This is an "Interrupt safe" way to enqueue a UI refresh event (step 1 of 2).
 *			OR method of converting an Interrupt routine to a non-interrupt routine so that
 *			more OS function calls are available, in his case a form update.
 *			(another way it to use a key and interrupt safe routine EvtEnqueueKey() )
 *
 *			See PrvNotifySiExRefresEventProc() for step 2 of 2
 *
 * PARAMETERS:  workspaceP - pointer to my workspace
 *
 ***********************************************************************/
void MyRefreshAppInterrupt( WorkSpaceType *workspaceP )
{
Err err;

	if ( workspaceP->uiAvailable!=1 )
		goto Done;		//My app only has the UI on the main form
	if ( workspaceP->uiRefreshEventsEnqueued )
		goto Done;		//Already enqueued a refresh key
	
	err = SysNotifyBroadcastFromInterrupt(appNotifySiExRefresEvent, appNotifySiExRefresEvent, workspaceP );
	if ( err )
		goto Done;	
	workspaceP->uiRefreshEventsEnqueued=1;	//Remember that this was enqueued (do not overflow the queue! it is very small!)
Done:
	return;
}

/***********************************************************************
 *
 * FUNCTION:    MyAppAutoRunCk
 *
 * DESCRIPTION: This checks an "autorun" parameters to see if this card is for this program.
 *			This is used for the autorun event AND for detecting the initial state of an already inserted card.
 *
 * PARAMETERS:  workspaceP - pointer to my workspace
 *				autoRunP - autorun workspace
 *
 * RETURNED:    true = this is the corect card for this program.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
UInt16 MyAppAutoRunCk( WorkSpaceType *workspaceP, AutoRunInfoType *autoRunP )
{
UInt16 result = false;

	if ( autoRunP->media != autoRunMediaSDIO )
		goto Skip;
	
	workspaceP->boardState.cardInserted = 1;
	workspaceP->boardState.cardInsertedUpdate = 1;

	//Start filtering out the mismatches
	if ( autoRunP->oemManufacturer != SdioCardsOemManufacturer )
		goto Skip;
	if ( autoRunP->oemID != SdioCardsOemID )
		goto Skip;	
	if ( autoRunP->oemFunctionStandard != autoRunFunctionStandardSDIOCustom ) //This is a SDIO custom device
		goto Skip;
	if ( autoRunP->oemFunctionNum != SdioCardsOemFunctionNum)
		goto Skip;	//We are only hooking into function 1
	if ( autoRunP->sourceStruct != autoRunSourceSlotDriverType )
		goto Skip;	//We are only Demo'ing Slot driver
	
	//
	// This is the SDIO card this app reconizes
	//	
	workspaceP->slotDriver = autoRunP->source.slotDriver;	//Save Slot Driver info for later use
	workspaceP->boardState.correctCardInserted = 1;
	workspaceP->boardState.correctCardInsertedUpdate = 1;
	workspaceP->uiState.correctCardInsertedUpdate = 1;
	workspaceP->boardState.cardInserted = 1;
	workspaceP->boardState.cardInsertedUpdate = 1;
	workspaceP->uiState.cardInsertedUpdate = 1;
	workspaceP->boardState.functionPowered = 1;
	workspaceP->boardState.functionPoweredUpdate = 1;
	workspaceP->boardState.functionPoweredUpdate = 1;
	MyAddCallBacks( workspaceP, true );		//install callback's first
	MyCardEnable( workspaceP, true );		//Then enable interrupts
	FunctionStateUpdate( workspaceP, false );
	result = true;
	
Skip:
	return( result );
}

/***********************************************************************
 *
 * FUNCTION:    MyCardAlreadyInserted
 *
 * DESCRIPTION: for detecting the initial autorun state of an already inserted card.
 *
 * PARAMETERS:  workspaceP - pointer to my workspace
 *
 * RETURNED:    workspace modified to reflect proper function & UI states
 *
 ***********************************************************************/
void MyCardAlreadyInserted( WorkSpaceType *workspaceP )
{
Err err = errNone;
UInt32 slotIterator;
UInt16 slotRefNum;
UInt16 slotLibRefNum;
UInt32 mediaType;
SDIOAutoRunInfoType autoRun;

	slotIterator = expIteratorStart;
	while( slotIterator != expIteratorStop )
	{
		err =  ExpSlotEnumerate( &slotRefNum, &slotIterator );
		if ( err )
		{
			break;
		}
		err = ExpSlotLibFind(slotRefNum,  &slotLibRefNum);
		if ( !err )
		{
			err = SlotMediaType( slotLibRefNum, slotRefNum, &mediaType );
			if ( !err )
			{
				if ( mediaType == expMediaType_SecureDigital )
				{
					autoRun.sdioSlotNum = sdioSlotFunc1; //only look in function 1
					err = SDIOGetAutoRun( slotLibRefNum, &autoRun );
					if ( err == errNone )
					{
						if ( MyAppAutoRunCk( workspaceP, &autoRun.autoRun ) )
							return;		//I found it!
					}
				}
			}
		}
	}
	return;	
}

/***********************************************************************
 *
 * FUNCTIONS:    MyCardPowerOff, MyCardSdioReset, MyCardEnable
 *
 *Parameters
 *	workspaceP, the workspace
 *	functionUpdate,	if==true, the hardware should be updated, (user was to change the state)
 *					if==false, the hardware is or will be updated, keep a record of what is happening
 *
 * DESCRIPTION: Miminic what happens or directly set SD I/O card registers when certain
 *		events occure.
 *
 ***********************************************************************/
//
// This is what happens when the SD I/O card is turned off
//
Err MyCardPowerOff( WorkSpaceType *workspaceP, Boolean functionUpdate )
{	
	workspaceP->boardState.functionPowered = 0;
	if ( functionUpdate )
	{	
		workspaceP->boardState.functionPoweredUpdate = 1;
	}
	workspaceP->uiState.functionPoweredUpdate = 1;
	MyCardSdioReset( workspaceP, functionUpdate );
	return( errNone );
}
//
// This is what happens when a SD I/O reset (all functions) occures
//
Err MyCardSdioReset( WorkSpaceType *workspaceP, Boolean functionUpdate  )
{	
	workspaceP->boardState.ioe1 = 0;
	workspaceP->boardState.ien1 = 0;
	if ( functionUpdate )
	{
		workspaceP->boardState.ioe1Update = 1;
		workspaceP->boardState.ien1Update = 1;
	}
	workspaceP->uiState.ioe1Update = 1;
	workspaceP->uiState.ien1Update = 1;
	return( errNone );
}
//
// This is what it takes to enable the function & interrupts
//
Err MyCardEnable( WorkSpaceType *workspaceP, Boolean functionUpdate  )
{	
	workspaceP->boardState.ioe1 = 1;
	workspaceP->boardState.ien1 = 1;
	if ( functionUpdate )
	{
		workspaceP->boardState.ioe1Update = 1;
		workspaceP->boardState.ien1Update = 1;
	}
	workspaceP->uiState.ioe1Update = 1;
	workspaceP->uiState.ien1Update = 1;

	return( errNone );
}



#pragma mark ----
#pragma mark (Standard Palm OS Application stuff)----
/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h
 *                                for format)
 *              launchFlags     - flags that indicate if the application
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
		{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			FrmAlert (RomIncompatibleAlert);
		
			// Palm OS 1.0 will continuously relaunch this app unless we switch to
			// another safe one.
			if (romVersion < ourMinVersion)
				{
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
				}
			}
		
		return sysErrRomIncompatible;
		}

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:  frm - pointer to the MainForm form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void MainFormInit(FormPtr frmP )
{
#pragma unused (frmP )
WorkSpaceType *workspaceP;

	if ( workspaceGet( &workspaceP ) )
		return;

	workspaceP->mainFormWinHandle = FrmGetWindowHandle ( frmP );
	workspaceP->mainFormFormID =  FrmGetFormId ( frmP );
	UIStateUpdate(workspaceP, true );
	
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void PrvMenuSdioDebugOptionsSet( SDIODebugOptionType option, const Char *textP )
{
Err err;
	err = SdioDebugOptionsSet( option );	
	if ( err )
	{
		FrmCustomAlert(GenericAlert,"Error Finding SDIO debug Slot Driver","","");				
	}
	else
	{
		FrmCustomAlert(GenericAlert,"Debug Trace:\n",textP,"");
	}
	return;
}

static Boolean MainFormDoCommand(UInt16 command)
{
Boolean handled = false;
FormPtr frmP;
FormPtr curFormP;

	switch (command)
		{
		case MainOptionsAbout:
			curFormP = FrmGetActiveForm();
			frmP = FrmInitForm (AboutForm);
			FrmSetActiveForm( frmP );
			FrmDoDialog (frmP);					// Display the About Box.
			FrmDeleteForm (frmP);
			FrmSetActiveForm (curFormP);
			handled = true;
			break;

		case DbgTraceAll:			
			handled = true;
			PrvMenuSdioDebugOptionsSet( sdioDebugOptionTraceAll, "All" );
			break;
				
		case DbgTraceMost:			
			handled = true;
			PrvMenuSdioDebugOptionsSet( sdioDebugOptionTraceMost, "Most" );
			break;
			
		case DbgTraceCommandsOnly:			
			handled = true;
			PrvMenuSdioDebugOptionsSet( sdioDebugOptionTraceCmds|sdioDebugOptionTraceISR, "Commands" );
			break;
			
		case DbgTraceNone:			
			handled = true;
			PrvMenuSdioDebugOptionsSet( sdioDebugOptionTraceNone, "None" );
			break;			

		}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr eventP)
{
Boolean handled = false;
FormPtr frmP;
WorkSpaceType *workspaceP;

	switch (eventP->eType)
		{
		case menuOpenEvent:
			if ( workspaceGet( &workspaceP ) )
				break;	
			workspaceP->uiAvailable++;
			break;

		case menuCloseEvent:
			if ( workspaceGet( &workspaceP ) )
				break;	
			workspaceP->uiAvailable--;
			UIStateUpdate( workspaceP, false );
			break;

		case menuEvent:
			if ( workspaceGet( &workspaceP ) )
				break;	
			workspaceP->uiAvailable++;
			handled = MainFormDoCommand(eventP->data.menu.itemID);
			workspaceP->uiAvailable--;
			UIStateUpdate( workspaceP, false );
			frmP = FrmGetActiveForm();
			FrmDrawForm( frmP);
			break;

		case frmOpenEvent:
			if ( workspaceGet( &workspaceP ) == errNone )
				workspaceP->uiAvailable = 1;
			frmP = FrmGetActiveForm();
			MainFormInit( frmP);
			FrmDrawForm ( frmP);
			handled = true;
			break;

		case frmCloseEvent:
			frmP = FrmGetActiveForm();
			if ( workspaceGet( &workspaceP ) )
				break;
			if ( eventP->data.frmClose.formID == FrmGetFormId ( frmP ) )
				workspaceP->uiAvailable = 2;
			workspaceP->mainFormWinHandle = NULL;
			workspaceP->mainFormFormID = NULL;
			break;

		case winEnterEvent:
			if ( workspaceGet( &workspaceP ) )
				break;
			if ( eventP->data.winEnter.enterWindow == workspaceP->mainFormWinHandle )
			{
				workspaceP->uiAvailable = 1;
				UIStateUpdate( workspaceP, false );
			}
			break;

		case winExitEvent:
			if ( workspaceGet( &workspaceP ) )
				break;
			if ( eventP->data.winExit.exitWindow == workspaceP->mainFormWinHandle )
				workspaceP->uiAvailable++;
			if ( eventP->data.winExit.enterWindow == workspaceP->mainFormWinHandle )
			{
				workspaceP->uiAvailable = 1;
				UIStateUpdate( workspaceP, false );
			}
			break;			

		case frmUpdateEvent:
				handled = true;
				if ( workspaceGet( &workspaceP ) )
					break;	
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;

		case ctlSelectEvent:
			if ( workspaceGet( &workspaceP ) )
				break;	
			switch (eventP->data.ctlSelect.controlID)
			{
			case MainFunc1OFFPushButton:
				handled = true;
				workspaceP->boardState.functionPowered = 0;
				workspaceP->boardState.functionPoweredUpdate = 1;
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;
			
			case MainLEDOffPushButton:
				handled = true;
				workspaceP->boardState.ledColor = LED_OFF;
				workspaceP->boardState.ledColorUpdate = 1;
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;
			
			case MainLEDRedPushButton:
				handled = true;
				workspaceP->boardState.ledColor = LED_RED;
				workspaceP->boardState.ledColorUpdate = 1;
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;
			
			case MainLEDGreenPushButton:
				handled = true;
				workspaceP->boardState.ledColor = LED_GREEN;
				workspaceP->boardState.ledColorUpdate = 1;
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;
			
			case MainSleepWithCardOffPushButton:
				handled = true;
				workspaceP->boardState.sleepWithCardOn = 0;
				workspaceP->boardState.sleepWithCardOnUpdate = 1;
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;
			
			case MainSleepWithCardOnPushButton:
				handled = true;
				workspaceP->boardState.sleepWithCardOn = 1;
				workspaceP->boardState.sleepWithCardOnUpdate = 1;
				FunctionStateUpdate( workspaceP, false );
				UIStateUpdate( workspaceP, false );
				break;

			default:
				break;
			}
		
		default:
			break;
		
		}
	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			default:
//				ErrFatalDisplay("Invalid Form Load Event");
				break;

			}
		return true;
		}
	
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	UInt16 error;
	EventType event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStart(void)
{
//StarterPreferenceType prefs;
//UInt16 prefsSize;
WorkSpaceType *workspaceP;
Err err;

	// Read the saved preferences / saved-state information.
	//prefsSize = sizeof(StarterPreferenceType);
	//if (PrefGetAppPreferences(appFileCreator, appPrefID, &prefs, &prefsSize, true) !=
	//	noPreferenceFound)
	//	{
	//	}

	if ( workspaceGet( &workspaceP ) )
	{
		err = workspaceCreate( &workspaceP );
		if ( err )
		{
			ErrNonFatalDisplay( "can't create workspace!" );
			return( err );
		}
		MyCardAlreadyInserted( workspaceP );
	}
	MyAddHooks( workspaceP, true );

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppStop(void)
{
//StarterPreferenceType prefs;

	// Write the saved preferences / saved-state information.  This data
	// will be backed up during a HotSync.
	//PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersionNum,
	//	&prefs, sizeof (prefs), true);
		
	// Close all the open forms.
	FrmCloseAllForms ();
}


/***********************************************************************
 *
 * FUNCTION:    StarterPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static UInt32 StarterPalmMain(UInt16 cmd, MemPtr /*cmdPBP*/, UInt16 launchFlags)
{
	Err error;

	error = RomVersionCompatible (ourMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd)
		{
		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error)
				return error;
				
			FrmGotoForm(MainForm);
			AppEventLoop();
			AppStop();
			break;

		default:
			break;

		}
	
	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    return StarterPalmMain(cmd, cmdPBP, launchFlags);
}
