/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: CmnLaunchCodes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Launch codes are now shared by DAL and Palm OS.
 *	Moved from SystemMgr.h to here.
 *	Changed the custom launch code sample to respect fixed enum types size,
 *	for compiler portability concerns.
 *
 *****************************************************************************/

#ifndef _CMNLAUNCHCODES_H_
#define _CMNLAUNCHCODES_H_


/************************************************************
 * Action Codes
 *
 * IMPORTANT ACTION CODE CONSIDERATIONS:
 *
 * Many action codes are "sent" to apps via a direct function call into the app's
 * PilotMain() function without launching the app.  For these action codes, the
 * application's global and static variables are *not* available, unless the
 * application is already running. Some action codes are synchronized with the
 * currently running UI applcation via the event manager (alarm action codes,
 * for example), while others, such as HotSync action codes, are sent from a
 * background thread. To find out if your app is running (is the current UI
 * app) when an action code is received, test the sysAppLaunchFlagSubCall flag
 * (defined in SystemMgr.h) which is passed to your PilotMain in the
 * launchFlags parameter (the third PilotMain parameter). If it is non-zero,
 * you may assume that your app is currently running and the global variables
 * are accessible. This information is useful if your app maintains an open
 * data database (or another similar resource) when it is running. If the app
 * receives an action code and the sysAppLaunchFlagSubCall is set in
 * launchFlags, the handler may access global variables and use the open
 * database handle while handling the call. On the other hand, if the
 * sysAppLaunchFlagSubCall flag is not set (ie., zero), the handler will need
 * to open and close the database itself and is not allowed to access global
 * or static variables.
 *
 *************************************************************/

// NOTE: for defining custom action codes, see sysAppLaunchCmdCustomBase below.

// System SysAppLaunch Commands
#define	sysAppLaunchCmdNormalLaunch				0			// Normal Launch

#define	sysAppLaunchCmdFind						1			// Find string

#define	sysAppLaunchCmdGoTo						2			// Launch and go to a particular record

#define	sysAppLaunchCmdSyncNotify				3  			// Sent to apps whose databases changed during
															// HotSync after the sync has been completed,
															// including when the app itself has been installed
															// by HotSync. The data database(s) must have the
															// same creator ID as the application for this
															// mechanism to function correctly. This is a
															// good opportunity to update/initialize/validate
															// the app's data, such as resorting records,
															// setting alarms, etc.
															//
															// Parameter block: None.
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a
															//		direct function call into the app's
															//		PilotMain function from the background
															//		thread of the HotSync application.


#define	sysAppLaunchCmdTimeChange				4  			// Sent to all applications and preference
															// panels when the system time is changed.
															// This notification is the right place to
															// update alarms and other time-related
															// activities and resources.
															//
															// Parameter block: None.
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdSystemReset				5  			// Sent to all applications and preference
															// panels when the system is either soft-reset
															// or hard-reset.  This notification is the
															// right place to initialize and/or validate
															// your application's preferences/features/
															// database(s) as well as to update alarms and
															// other time-related activities and resources.
															//
															// Parameter block: SysAppLaunchCmdSystemResetType
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdAlarmTriggered			6  			// Sent to an application at the time its
															// alarm time expires (even when another app
															// is already displaying its alarm dialog box).
															// This call is intended to allow the app to
															// perform some very quick activity, such as
															// scheduling the next alarm or performing a
															// quick maintenance task.  The handler for
															// sysAppLaunchCmdAlarmTriggered must take as
															// little time as possible and is *not* allowed
															// to block (this would delay notification for
															// alarms set by other applications).
															//
															// Parameter block: SysAlarmTriggeredParamType
															//		(defined in AlarmMgr.h)
															// Restrictions: No accessing of global or
															//		static variables unless sysAppLaunchFlagSubCall
															//		flag is set, as discussed above.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdDisplayAlarm				7  			// Sent to an application when it is time
															// to display the alarm UI. The application
															// is responsible for making any alarm sounds
															// and for displaying the alarm UI.
															// sysAppLaunchCmdDisplayAlarm calls are ordered
															// chronoligically and are not overlapped.
															// This means that your app will receive
															// sysAppLaunchCmdDisplayAlarm only after
															// all earlier alarms have been displayed.
															//
															// Parameter block: SysDisplayAlarmParamType
															//		(defined in AlarmMgr.h)
															// Restrictions: No accessing of global or
															//		static variables unless sysAppLaunchFlagSubCall
															//		flag is set, as discussed above.  UI calls are
															//		allowed to display the app's alarm dialog.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdCountryChange			8  			// The country has changed

#define	sysAppLaunchCmdSyncRequestLocal			9  			// Sent to the HotSync application to request a
															// local HotSync.  ("HotSync" button was pressed.)

#define	sysAppLaunchCmdSyncRequest			sysAppLaunchCmdSyncRequestLocal	// for backward compatibility

#define	sysAppLaunchCmdSaveData			  		10 			// Sent to running app before sysAppLaunchCmdFind
															// or other action codes that will cause data
															// searches or manipulation.

#define	sysAppLaunchCmdInitDatabase	  			11			// Sent to an application when a database with
															// a matching Creator ID is created during
															// HotSync (in response to a "create db"
															// request). This allows the application to
															// initialize a newly-created database during
															// HotSync.  This might include creating some
															// default records, setting up the database's
															// application and sort info blocks, etc.
															//
															// Parameter block: SysAppLaunchCmdInitDatabaseType
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a
															//		direct function call into the app's
															//		PilotMain function from the background
															//		thread of the HotSync application.

#define	sysAppLaunchCmdSyncCallApplicationV10	12 			// Used by DesktopLink Server command "call application";
															// Pilot v1.0 only!!!

//------------------------------------------------------------------------
// New launch codes defined for PalmOS 2.0
//------------------------------------------------------------------------

#define	sysAppLaunchCmdPanelCalledFromApp		13 			// The panel should display a done
															// button instead of the pick list.
															// The Done button will return the user
															// to the last app.

#define	sysAppLaunchCmdReturnFromPanel			14			// A panel returned to this app

#define	sysAppLaunchCmdLookup				  	15			// Lookup info managed by an app

#define	sysAppLaunchCmdSystemLock			  	16			// Lock the system until a password is entered.

#define	sysAppLaunchCmdSyncRequestRemote		17			// Sent to the HotSync application to request
															// a remote HotSync.  ("Remote HotSync" button
															// was pressed.)

#define	sysAppLaunchCmdHandleSyncCallApp		18 			// Pilot v2.0 and greater.  Sent by DesktopLink Server to an application to handle
															// the "call application" command; use DlkControl with
															// control code dlkCtlSendCallAppReply to send the reply(see DLServer.h).
															// This action code replaces the v1.0 code sysAppLaunchCmdSyncCallApplication.
															// vmk 11/26/96

#define sysAppLaunchCmdAddRecord				19			// Add a record to an applications's database.


//------------------------------------------------------------------------
// Standard Service Panel launch codes (used by network panel, dialer panel, etc.)
//------------------------------------------------------------------------
#define	sysSvcLaunchCmdSetServiceID				20
#define	sysSvcLaunchCmdGetServiceID				21
#define	sysSvcLaunchCmdGetServiceList			22
#define	sysSvcLaunchCmdGetServiceInfo			23


#define sysAppLaunchCmdFailedAppNotify			24			// An app just switched to failed.
#define sysAppLaunchCmdEventHook				25			// Application event hook callback
#define sysAppLaunchCmdExgReceiveData			26			// Exg command for app to receive data.
#define sysAppLaunchCmdExgAskUser 				27			// Exg command sent before asking user.


//------------------------------------------------------------------------
// Standard Dialer Service launch codes (30 - 39 reserved)
//------------------------------------------------------------------------

// sysDialLaunchCmdDial: dials the modem(optionally displays dial progress UI), given service id
// and serial library reference number
#define sysDialLaunchCmdDial					30
// sysDialLaunchCmdHangUp: hangs up the modem(optionally displays disconnect progress UI), given service id
// and serial library reference number
#define sysDialLaunchCmdHangUp					31
#define sysDialLaunchCmdLast					39


//------------------------------------------------------------------------
// Additional standard Service Panel launch codes (used by network panel, dialer panel, etc)
// (40-49 reserved)
//------------------------------------------------------------------------

#define sysSvcLaunchCmdGetQuickEditLabel		40			// SvcQuickEditLabelInfoType
#define sysSvcLaunchCmdLast						49


//------------------------------------------------------------------------
// New launch codes defined for PalmOS 3.x where x >= 1
//------------------------------------------------------------------------

#define sysAppLaunchCmdURLParams				50			// Sent from the Web Clipper application.
															// This launch code gets used to satisfy
															// URLs like the following:
															//    palm:memo.appl?param1=value1&param2=value2
															// Everything in the URL past the '?' is passed
															// to the app as the cmdPBP parameter of PilotMain().

#define sysAppLaunchCmdNotify					51			// This is a NotifyMgr notification sent
															// via SysNotifyBroadcast.  The cmdPBP parameter
															// points to a SysNotifyParamType structure
															// containing more specific information
															// about the notification (e.g., what it's for).
																	
#define sysAppLaunchCmdOpenDB					52			// Sent to switch to an application and have it
															// "open" up the given data file. The cmdPBP
															// pointer is a pointer to a SysAppLaunchCmdOpenDBType
															// structure that has the cardNo and localID of the database
															// to open. This action code is used by the Launcher
															// to launch data files, like Eleven PQA files that
															// have the dmHdrAttrLaunchableData bit set in their
															// database attributes.

#define sysAppLaunchCmdAntennaUp				53			// Sent to switch only to the launcher when
															// the antenna is raised and the launcher
															// is the application in the buttons preferences
															// that is to be run when the antenna is raised is
															// the launcher.

#define sysAppLaunchCmdGoToURL					54			// Sent to Clipper to have it launch and display
															// a given URL.  cmdPBP points to the URL string.

//------------------------------------------------------------------------
// New launch codes defined for Network panel plug-in
//------------------------------------------------------------------------
		
#define sysAppLaunchNppiNoUI					55			// Sent to network panel plug-in ("nppi") to have it launch
															// without UI and load to netlib
															
#define sysAppLaunchNppiUI						56			// Sent to network panel plug-in ("nppi") to have it launch
															// with UI

//------------------------------------------------------------------------
// New launch codes defined for PalmOS 4.x where x >= 0
//------------------------------------------------------------------------
															
#define sysAppLaunchCmdExgPreview				57			// Sent to an application by the Exchange Manager when the
															// application needs to produce a preview.


#define sysAppLaunchCmdCardLaunch				58			// Sent to an application by the Launcher when the
															// application is being run from a card.

#define sysAppLaunchCmdExgGetData				59			// Exg command for app to send data requested by an ExgGet

																	
																	
#define sysAppLaunchCmdAttention				60			// sent to an application by the attention manager
															// when the application needs to take action on an entry
															// that has been submitted to the attention manager queue.

#define sysAppLaunchPnpsPreLaunch				61			//pre-launch code for Pnps devices,
															//cmdPBP points to SysAppLaunchCmdPnpsType


#define sysAppLaunchPreDelete					62			// Sent to palm-created apps before they're deleted.
															// Only sent to Palm apps, and stripped from public
															// headers because this will probably go away in the next
															// release when we have a better solution for the problem.
															// This is used by the new Clipper app which installs a
															// library contained in it, so that it can uninstall the
															// library & unprotect itself before it is deleted.
															// cmdPBP is NULL


//------------------------------------------------------------------------
// New launch codes defined for Palm OS version >= 5.0
//------------------------------------------------------------------------

#define sysAppLaunchCmdMultimediaEvent			63			// launch code for multimedia session event notification

// Palm OS 5.3 launch codes.
#define sysAppLaunchCmdFepPanelAddWord			87			// launch code for adding a word to the FEP's user dictionary.
#define sysAppLaunchCmdLookupWord				88			// launch code for looking up a word in the Word Lookup app.

//------------------------------------------------------------------------
// New launch codes defined for Palm OS version >= 6.0
//------------------------------------------------------------------------

#define sysAppLaunchCmdExportRecordGetCount		64			// cmdPBP points to an uint32_t that will receive the total
															// number of records stored in the PIM database.
															
#define sysAppLaunchCmdExportRecord				65			// cmd launch code for PIM-exporting data. The index OR uniqueID
															// of the record to export is stored in the index / uniqueID field
															// of the ImportExportRecordParamsType struct. The index field is
															// checked first, if its value is ImpExpInvalidRecIndex (0xFFFFFFFF), the
															// uniqueID field is then evaluated. The data is addressed by the
															// vObjectH handle. If vObjectH is NULL, the index / uniqueID will
															// be updated according to the rules above without exporting data.
															
#define sysAppLaunchCmdImportRecord				66			// cmd launch code for PIM-importing data. The index field
															// of the ImportExportRecordParamsType struct is ignored.
															// If the uniqueID field value is different from ImpExpInvalidUniqueID (0x0),
															// the PIM DB record attached to this uniqueID (if it exists) will be
															// replaced, otherwise it will be added to the PIM database.
															// The future record position will depend on PIM sort rule if existing, or
															// added to the end of database.
															
#define sysAppLaunchCmdDeleteRecord				67			// Removes the record - Same search rules as export command.				

#define sysAppLaunchCmdMoveRecord				68			// If the application supports it, the record will be moved from the
															// specified initial position (index) to the destination position (destIndex),
															// Both are 0-based.
															// If the application doesn't support is, dmErrInvalidParam is returned.
															// If this launch code succeeds (errNone), uniqueID field is updated.

#define sysLaunchCmdBoot                  		70 			// Sent to v6.0 OS initialization procedures.
															// The procedure runs in the System Process and
															// does whatever is necessary to initialize its component.

#define sysPackageLaunchAttachImage				71			// Sent to packages when they are loaded.
															// Supplies an image context they use to determine
															// when the package should be unloaded.

#define sysPackageLaunchGetInstantiate			72			// Sent to packages when they are loaded.
															// Returns a function used to instantiate components
															// in the package.

#define	sysAppLaunchCmdBackground				73			// Launch app as a background thread, running its own
															// event loop.  This is sent by EvtCreateBackgroundThread().
															
#define sysIOSDriverInstall						74			// Sent to a code module in the I/O Process when it is installed.

#define sysIOSDriverRemove						75			// Sent to a code module in the I/O Process when it is removed.

#define sysBtLaunchCmdPrepareService			76			// Sent to bluetooth service apps: create listener socket and service record

#define sysBtLaunchCmdExecuteService			77			// Sent to bluetooth service apps: here's an inbound-connected data socket

#define sysLaunchCmdGraphicsAccelInit			78			// Sent to graphics accelerant by mini-gl to request
															// it initialize itself.  cmdPBP contains the mini-gl
															// context that it is running in.

#define	sysCncPluginLaunchCmdRegister			79			// Sent to CM plugins modules to do init stuff after plugins registration

#define	sysCncPluginLaunchCmdUnregister			80			// Sent to CM plugins modules when removing plugins

#define	sysCncPluginLaunchCmdGetPlugins			81			// Sent to CM plugins modules to get plugins descritions

#define sysAppLaunchCmdSlipLaunch				82			// Sent to any application that is launched within a Slip.
															// The application can only draw within update events.

#define sysAppLaunchCmdPinletLaunch				83			// Sent to any application that is launched as a pinlet.
															// The application can only draw within update events.

#define sysCncWizardLaunchCmdEdit				84			// sent to CM configuration application to edit a profile

#define sysPinletLaunchCmdLoadProcPtrs			85			// Sent to a prc-style pinlet before the pinlet is displayed on the screen.
															// These functions are used by the pen input mgr to interact with this pinlet.

#define sysBtLaunchCmdDescribeService			86			// Sent to bluetooth service apps: describe yourself

// WARNING!!! Check 5.x section above for 87/88 launch codes.

#define sysBtLaunchCmdDoServiceUI				89			// Sent to bluetooth service apps: do your custom UI thing in the bt panel

#define sysBtLaunchCmdAbortService				90			// Sent to bluetooth service apps: service has been aborted; clean up any
															// extra resources allocated in the "preparation" entry point

#define sysPatchLaunchCmdClearInfo				0x7ff3		// Sent to patches to tell that a target shared library has been unloaded

#define sysLaunchCmdProcessDestroyed	0x7ff4
#define sysLaunchCmdGetModuleID				0x7ff5
#define sysLaunchCmdInitRuntime				0x7ff6

#define sysAppLaunchCmdFinalizeUI			0x7ff7		// Sent to root application of the UI process only. Startup code
																		// of the app will de-initialize UI for the process.

#define sysAppLaunchCmdInitializeUI			0x7ff8		// Sent to root application of the UI process only. Startup code
																		// of the app will initialize UI for the process.

#define sysLaunchCmdAppExited				0x7ff9		// Sent to all loaded modules after the app exited from its PilotMain

#define sysLaunchCmdGetGlobals				0x7ffa		// Sent to any module to retrieve address of its global structure

#define sysPatchLaunchCmdSetInfo				0x7ffb		// Sent to patches to set initial information
															// cmdPBP points to SysPatchInfoType
// Action codes sent to PACE
#define sysAppLaunchCmdRun68KApp				0x7ffc		// Sent to PACE to run a 68K app

// Action code sent to PACE
#define sysLibLaunchCmdGet68KSupportEntry		0x7ffd		// Sent to shared library to query if it can be called from 68K apps

// Action codes sent to all kinds of executable modules
#define sysLaunchCmdInitialize					0x7ffe		// Sent right after a module gets loaded
#define sysLaunchCmdFinalize					0x7fff		// Sent right before a module gets unloaded



//------------------------------------------------------------------------
// Custom action code base (custom action codes begin at this value)
//------------------------------------------------------------------------
#define	sysAppLaunchCmdCustomBase				0x8000

// Your custom launch codes can be defined like this:
//
//	enum MyAppCustomActionTag {
//		myAppCmdDoSomething = sysAppLaunchCmdCustomBase,
//		myAppCmdDoSomethingElse,
//		myAppCmdEtcetera
//
//	} ;
//	typedef uint32_t MyAppCustomActionCodes ;


//------------------------------------------------------------------------
// SysAppLaunch flags (passed to PilotMain)
//------------------------------------------------------------------------

#define	sysAppLaunchFlagNewThread					0x01	// create a new thread for application
															//  - implies sysAppLaunchFlagNewStack
#define	sysAppLaunchFlagNewStack					0x02	// create separate stack for application
#define	sysAppLaunchFlagNewGlobals					0x04	// create new globals world for application
															//  - implies new owner ID for Memory chunks
#define	sysAppLaunchFlagUIApp						0x08	// notifies launch routine that this is a UI app being
															//  launched.
#define	sysAppLaunchFlagSubCall						0x10	// notifies launch routine that the app is calling it's
															//  entry point as a subroutine call. This tells the launch
															//  code that it's OK to keep the A5 (globals) pointer valid
															//  through the call.
															// IMPORTANT: This flag is for internal use by
															//  SysAppLaunch only!!! It should NEVER be set
															//  by the caller.
#define	sysAppLaunchFlagGlobalsAvailable			0x20	// New launch code in 5.0. Notifies the application
															// that it can access globals. This flag will be set whenever
															// sysAppLaunchFlagNewGlobals is set, or when the app has
															// an unique runtime ID.
															// IMPORTANT: This flag is for internal use by
															//  SysAppLaunch only!!! It should NEVER be set
															//  by the caller.
#define sysAppLaunchFlagDataRelocated				0x80	// global data (static ptrs) have been "relocated"
															//  by either SysAppStartup or StartupCode.c
															// IMPORTANT: This flag is for internal use by
															//  SysAppLaunch only!!! It should NEVER be set
															//  by the caller.

// The set of private, internal flags that should never be set by the caller
#define sysAppLaunchFlagPrivateSet		(sysAppLaunchFlagSubCall | sysAppLaunchFlagDataRelocated | sysAppLaunchFlagGlobalsAvailable)


#endif // _CMNLAUNCHCODES_H_
