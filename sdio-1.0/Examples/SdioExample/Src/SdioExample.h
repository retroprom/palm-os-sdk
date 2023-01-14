/******************************************************************************
 *
 * Copyright (c) 2001 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: SdioExample.h
 *
 * Release: Palm SDIO SDK 1.0 (79749)
 *
 *****************************************************************************/

/***********************************************************************
 *
 *   SDIO I/O ports
 *
 ***********************************************************************/
#define sdiofunc0CCCRAddressIOE	0x02 //CCCR, function 0 IOEx function enable address
#define sdiofunc0CCCRDataIOE1	0x02 //CCCR IOE1, 1=function 1 enabled
//#define sdiofunc0CCCRAddressIOR	0x03 //CCCR, function 0 IORx address
//#define sdiofunc0CCCRDataIOR1	0x02 //CCCR, IOR1, 1=function 1 is ready (not supported in this test card)
#define sdiofunc0CCCRAddressIEN	0x04 //CCCR, function 0 IENx address
#define sdiofunc0CCCRDataIENM	0x01 //CCCR, IENM, 1= master interrupt enable
#define sdiofunc0CCCRDataIEN1	0x02 //CCCR, IEN1, 1= function 1 interrupt enable

//
// Function 1 of Palms Test SDIO card.......
//	Note: this is a write only port!
//
#define PalmTestSDIOCardFunc1	1
#define PalmTestSDIOCardFunc1Addr0	0				//Note, this is a write only port, (cheap hardware!)
#define PalmTestSDIOCardFunc1Addr0LedOn		(1<<0)	//Set to 1=turn on the LED(see color below), 0=turn off LED
#define PalmTestSDIOCardFunc1Addr0LedRed	(1<<1)	//Set to 1=Red LED, 0=Green LED (If LED is On)
#define PalmTestSDIOCardFunc1Addr0LedGreen	(0<<1)	//Set to 1=Red LED, 0=Green LED (If LED is On)
#define PalmTestSDIOCardFunc1Addr0NIntLevel	(1<<2)	//Set to 1=generate an interrupt, 0=reset an interrupt (assuming button has been released!)

/***********************************************************************
 *
 *   Internal Constants
 *
 ***********************************************************************/
#define appFileCreator			  'SiEx'				//The Creator ID of this Application
#define appFileCreatorType		sysFileTApplication		//The Creator type of this Application
#define appFileCreatorName		"SdioExample-SiEx"		//The Creator Name of this Application
#define appVersionNum              0x01
#define appPrefID                  0x00
#define appPrefVersionNum          0x01

// Define the minimum OS version we support (4.0 for now).
//#define ourMinVersion	sysMakeROMVersion(4,0,0,sysROMStageRelease,0)
#define ourMinVersion	sysMakeROMVersion(4,0,0,sysROMStageDevelopment,0)

//
// Notification Piroritys
//
#define appNotifySiExRefresEvent appFileCreator	//My Apps event
#define PrvNotifySiExRefreshEventPrio 0
#define PrvNotifyDeleteProtectedEventPrio 0
#define PrvNotifyDeleteProtectedEventPrio 0
#define PrvNotifyDriverSearchEventPrio 0
#define PrvNotifyCardRemovedEventPrio +50	//Well after expansion manager has done it duties

//The SDIO cards Manufacturer & ID numbers
#define SdioCardsOemManufacturer	0x00005672L	//Palm test card
#define SdioCardsOemID				0x00004673L	//Palm test card
#define SdioCardsOemFunctionNum 1				//This palm test card(s) comes in many flavors, 1-7 functions, but we only talk to function 1

//RSC file Push Button Group numbers (Constructor has a strange way of assigning group names to group numbers) 
#define MyMainGroupIDSleepWithCard 1
#define MyMainGroupIDLEDColor 2


/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/
//typedef struct 
//{
//	UInt8 replaceme;
//}StarterPreferenceType;
//
//typedef struct 
//{
//	UInt8 replaceme;
//}StarterAppInfoType;

//
// My global workspace
//
typedef enum {
LED_UNKNOWN,
LED_OFF,
LED_RED,
LED_GREEN
}LedType;

typedef struct {
	UInt16 ioe1Update:1;
	UInt16 ien1Update:1;
	UInt16 sleepWithCardOnUpdate:1;
	UInt16 cardInsertedUpdate:1;
	UInt16 correctCardInsertedUpdate:1;
	UInt16 functionPoweredUpdate:1;
	UInt16 ledColorUpdate:1;

	UInt16 ioe1:1;		//if==1 function is enabled
	UInt16 ien1:1;		//if==1 function interrupts are enabled 
	UInt16 sleepWithCardOn:1;
	UInt16 cardInserted:1;
	UInt16 correctCardInserted:1;
	UInt16 functionPowered:1;
	LedType ledColor;
}BoardStateType;

typedef struct {
	AutoRunSlotDriverType slotDriver;
	WinHandle mainFormWinHandle;
	UInt16  mainFormFormID;
	UInt16  uiAvailable;					//if==1 this applications UI is available
	UInt16  uiRefreshEventsEnqueued:1;
	UInt16  handheldAsleep:1;		//if==1 the handheld is asleep or is going to sleep
	UInt16  notifySiExRefresEventRegistered:1;
	UInt16  notifyCardRemovedEventRegistered:1;
	UInt16  notifyDeleteProtectedEventRegistered:1;
	UInt16  notifyDriverSearchEventRegistered:1;
	UInt16  appDmDatabaseProtect:1;
	UInt16  slotDriverCallBacksInstalled:1;
	BoardStateType boardState;
	BoardStateType uiState;
}WorkSpaceType;

//
//Feature Manager 
//Warning, Feature Manager items get deleted when a soft or hard reset occures
//
#define SdioExFeatureWorkSpace 1	//A place to keep my pointer to my global workspace

typedef struct {
WorkSpaceType *workspaceP;
}FtrWorkSpaceType;



/***********************************************************************
 *
 *   Internal Functions
 *
 ***********************************************************************/
Err MyAddHooks( WorkSpaceType *workspaceP, UInt16 install );
Err PrvNotifyDeleteProtectedEventProc(SysNotifyParamType *notifyParamsP);
Err PrvNotifyDriverSearchEventProc(SysNotifyParamType *notifyParamsP);
Err PrvNotifyCardRemovedEventProc(SysNotifyParamType *notifyParamsP);
Err PrvNotifySiExRefresEventProc(SysNotifyParamType *notifyParamsP);

Err MyAddCallBacks( WorkSpaceType *workspaceP, UInt16 install );
Err MySdioCallBackSelectPowerOn( SDIOSlotType sdioSlotNum, void *userDataP );
Err MySdioCallBackSelectPowerOff( SDIOSlotType sdioSlotNum, void *userDataP );
Err MySdioCallBackSelectAwake( SDIOSlotType sdioSlotNum, void *userDataP );
Err MySdioCallBackSelectSleep( SDIOSlotType sdioSlotNum, void *userDataP );
Err MySdioCallBackSelectReset( SDIOSlotType sdioSlotNum, void *userDataP );
Err MySdioCallBackSelectInterruptSdCard( SDIOSlotType sdioSlotNum, void *userDataP );

void UIStateUpdate( WorkSpaceType *workspaceP, UInt16 forceUpdate );
void FunctionStateUpdate( WorkSpaceType *workspaceP, UInt16 forceUpdate );

Err workspaceGet( WorkSpaceType **workspacePP);
Err workspaceCreate( WorkSpaceType **workspacePP);
Err workspaceDestroy( void );

Err MyCardPowerOff( WorkSpaceType *workspaceP, Boolean functionUpdate );
Err MyCardSdioReset( WorkSpaceType *workspaceP, Boolean functionUpdate );
Err MyCardEnable( WorkSpaceType *workspaceP, Boolean functionUpdate );

UInt16 LaunchApp(UInt32 creator);
void MyRefreshAppInterrupt( WorkSpaceType *workspaceP );
UInt16 MyAppAutoRunCk( WorkSpaceType *workspaceP, AutoRunInfoType *autoRunP );
void MyCardAlreadyInserted( WorkSpaceType *workspaceP );

//
//EOF
//
