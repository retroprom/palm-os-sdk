/******************************************************************************
 *
 * File: Starter.c
 *
 * Project: Example X
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "StarterRsc.h"
#include "starter.rh"     //MainForm definition

#include "Trg.h"
#include "Vga.h"
#include "Silk.h"

/***********************************************************************
 *
 *   Entry Points
 *
 ***********************************************************************/


/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/
typedef struct 
{
    UInt8 replaceme;
} StarterPreferenceType;

typedef struct 
{
    UInt8 replaceme;
} StarterAppInfoType;

typedef StarterAppInfoType* StarterAppInfoPtr;

FontID  currentFont   = stdFont;
Boolean vgaPresent    = false;
Boolean screenChanged = false;

/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/


/***********************************************************************
 *
 *   Internal Constants
 *
 ***********************************************************************/
#define appFileCreator            'Ex_X'
#define appVersionNum              0x01
#define appPrefID                  0x00
#define appPrefVersionNum          0x01

// Define the minimum OS version we support (2.0 for now).
#define ourMinVersion   sysMakeROMVersion(2,0,0,sysROMStageRelease,0)

/***********************************************************************
 *
 *   Internal Functions
 *
 ***********************************************************************/
static void DrawHelloWorld(void);


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
 
/*------------------------------------------------------------------------
 * See if we're on in minimum required version of the ROM or later.
 *----------------------------------------------------------------------*/
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion)
    {
        if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
           (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            FrmAlert (RomIncompatibleAlert);
        
/*------------------------------------------------------------------------
 * Palm OS 1.0 will continuously relaunch this app unless we switch to 
 * another safe one.
 *----------------------------------------------------------------------*/
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
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void * GetObjectPtr(UInt16 objectID)
{
    FormPtr frmP;
 
    frmP = FrmGetActiveForm();
    return(FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID)));
}

/***********************************************************************
 *
 * FUNCTION:    MainFormResize
 *
 * DESCRIPTION: 
 *
 * PARAMETERS: 
 *
 * RETURNED:    
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void MainFormResize(FormPtr frmP, Boolean draw)
{
    Coord         x,y;
    RectangleType r;

    WinGetDisplayExtent(&x, &y);
    RctSetRectangle(&r, 0, 0, x, y);

    frmP = FrmGetActiveForm();
    WinSetWindowBounds(FrmGetWindowHandle(frmP), &r);
    screenChanged = false;

    if (draw)
        DrawHelloWorld();
}

/***********************************************************************
 *
 * FUNCTION:    DrawHelloWorld
 *
 * DESCRIPTION: This routine prints "Hello World" to the center of the 
 *              window.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void DrawHelloWorld(void)
{
    MemHandle       resH;
    BitmapPtr       resP;
    char           *str = "Hello World";
    FontID          savedFont;
    Coord           x, y, winWidth, winHeight;
    FormPtr         formP;
    UInt8          *ptr;

    formP = FrmGetActiveForm();

/*------------------------------------------------------------------------
 * Lock the screen to remove flicker
 *----------------------------------------------------------------------*/
    ptr = WinScreenLock(winLockCopy);

    FrmEraseForm(formP);
    FrmDrawForm(formP);

    if (vgaPresent)
        resH = DmGetResource(bitmapRsc, VGAGlobeBitmap);
    else    
        resH = DmGetResource(bitmapRsc, GlobeBitmap);
    ErrFatalDisplayIf(!resH, "Missing bitmap");
    resP = MemHandleLock(resH);

    WinGetDisplayExtent(&winWidth, &winHeight);
    savedFont = FntSetFont(currentFont);

/*------------------------------------------------------------------------
 * Draw the globe graphic
 *----------------------------------------------------------------------*/
    x = (winWidth/2) - (resP->width/2);
    y = (winHeight/2) - (resP->height/2);
    WinDrawBitmap(resP, x, y);

/*------------------------------------------------------------------------
 * Draw String below the globe
 *----------------------------------------------------------------------*/
    x = (winWidth/2) - (FntCharsWidth(str, StrLen(str))/2);
    y = y + resP->height + 5;
    WinDrawChars(str, StrLen(str), x, y); 

    FntSetFont(savedFont);
    MemPtrUnlock(resP);
    DmReleaseResource(resH);

    if (ptr)
        WinScreenUnlock();
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
static void MainFormInit(FormPtr frmP)
{
    currentFont = stdFont;

    if (vgaPresent)
    {
        currentFont = VgaBaseToVgaFont(currentFont);
        MainFormResize(frmP, false);
    }    
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
static Boolean MainFormDoCommand(UInt16 command)
{
    Boolean handled = false;
    UInt16  formID;
    FormPtr frmP;
    FontID  newFont;
    VgaScreenModeType    screenMode;
    VgaRotateModeType    rotateMode;
 
    formID = (FrmGetFormId (FrmGetActiveForm ()));

    switch (command)
    {
        case MainOptionsAboutExampleX:
            MenuEraseStatus(0);              
            VgaGetScreenMode(&screenMode, &rotateMode);
            VgaSetScreenMode(screenModeScaleToFit, rotateMode);
            frmP = FrmInitForm (AboutForm);
            FrmDoDialog (frmP);              // Display the About Box.
            FrmDeleteForm (frmP);
            VgaSetScreenMode(screenMode, rotateMode);
            handled = true;
            break;
           
        case MainOptionsFont :
            newFont = FontSelect(currentFont);
            FrmEraseForm(FrmGetActiveForm ());
            currentFont = newFont;
            handled = true;
            break;
    }

    FrmUpdateForm (formID, frmRedrawUpdateCode);

    return(handled);
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
 
    switch (eventP->eType) 
    {
        case menuEvent:
            return MainFormDoCommand(eventP->data.menu.itemID);
 
        case frmOpenEvent:
            frmP = FrmGetActiveForm();
            MainFormInit( frmP);
            FrmDrawForm ( frmP);
            DrawHelloWorld();
            handled = true;
            break;

        case frmUpdateEvent :
            DrawHelloWorld();
            handled = true;
            break;

        case winEnterEvent:
   	    if (screenChanged && (FrmGetActiveFormID() == MainForm))
   	        MainFormResize(FrmGetActiveForm(), true);
            break;
 
        default:
            break;
    }
    return(handled);
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
    UInt16  formId;
    FormPtr frmP;

    if (eventP->eType == frmLoadEvent)
    {
/*------------------------------------------------------------------------
 * Load the form resource.
  *----------------------------------------------------------------------*/
        formId = eventP->data.frmLoad.formID;
        frmP   = FrmInitForm(formId);
        FrmSetActiveForm(frmP);
/*------------------------------------------------------------------------
 * Set the event handler for the form.  The handler of the currently
 * active form is called by FrmHandleEvent each time is receives an event.
 *----------------------------------------------------------------------*/
        switch (formId)
        {
            case MainForm:
                FrmSetEventHandler(frmP, MainFormHandleEvent);
                break;
            default:
                ErrFatalDisplay("Invalid Form Load Event");
                break;
 
        }
        return(true);
    }
    
    return(false);
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

    do 
    {
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
    StarterPreferenceType prefs;
    UInt16                prefsSize;
    UInt32                version;
    UInt16                cardNo;
    LocalID               appID;

/*------------------------------------------------------------------------
 * Read the saved preferences / saved-state information.
 *----------------------------------------------------------------------*/
    prefsSize = sizeof(StarterPreferenceType);
    if (PrefGetAppPreferences(appFileCreator, appPrefID, &prefs, &prefsSize, true) != 
            noPreferenceFound)
    {
    }

/*------------------------------------------------------------------------
 * Check for the VGA Extension
 *----------------------------------------------------------------------*/
    if (_TRGVGAFeaturePresent(&version))          
        vgaPresent  = true;
    else
    {
        FrmAlert(VgaNotFoundAlert);
        vgaPresent = false;
        return(-1);     //some generic error for now
    }    

/*------------------------------------------------------------------------
 * Register for silk screen change notifications. 
 * Note: You do not need to check for the presense of the silk extension to
 *       register for its notifications.  If the silk extension is not present
 *       you simply will not receive any notifications.
 *----------------------------------------------------------------------*/
    SysCurAppDatabase(&cardNo, &appID);
    SysNotifyRegister(0, appID, trgNotifySilkEvent, NULL, sysNotifyNormalPriority, NULL);
   
    return(errNone);
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
    StarterPreferenceType prefs;
    UInt16                cardNo;
    LocalID               appID;

/*------------------------------------------------------------------------
 * Write the saved preferences / saved-state information.  This data 
 * will be backed up during a HotSync.
 *----------------------------------------------------------------------*/
    PrefSetAppPreferences (appFileCreator, appPrefID, appPrefVersionNum, 
                           &prefs, sizeof (prefs), true);
      
/*------------------------------------------------------------------------
 * Unregister for silk screen change notifications
 *----------------------------------------------------------------------*/
    SysCurAppDatabase(&cardNo, &appID);
    SysNotifyUnregister(cardNo, appID, trgNotifySilkEvent, sysNotifyNormalPriority);

/*------------------------------------------------------------------------
 * Close all the open forms.
 *----------------------------------------------------------------------*/
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
static UInt32 StarterPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
    Err error;
 
    if ((error = RomVersionCompatible (ourMinVersion, launchFlags)) != 0)
        return (error);
 
    switch (cmd)
    {
        case sysAppLaunchCmdNormalLaunch:
            error = AppStart();
            if (error) 
                return error;
              
            if (vgaPresent)
                VgaSetScreenMode(screenMode1To1, rotateModeNone);

            FrmGotoForm(MainForm);
            AppEventLoop();
            AppStop();
            break;

        case sysAppLaunchCmdNotify : // HandEra popup silk screen
            if ( ((SysNotifyParamType *)cmdPBP)->notifyType == trgNotifySilkEvent)
            {
                if ((FrmGetActiveFormID() == MainForm) && FrmVisible(FrmGetActiveForm()))
                    MainFormResize(FrmGetActiveForm(), true);
                else
                    screenChanged = true;
            }    
            break;    
  
        default:
            break;
  
    }
    return(errNone);
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

