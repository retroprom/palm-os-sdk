/******************************************************************************
 *
 * Copyright (c) 1998-2004 PalmSource, Inc. All rights reserved.
 *
 * File: NotifyMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Include file for Notification Manager
 *
 *****************************************************************************/

#ifndef	__NOTIFY_H__
#define	__NOTIFY_H__

#include <PalmTypes.h>
#include <DataMgr.h>
#include <SystemResources.h>

typedef struct SysNotifyParamType
{
	uint32_t		notifyType;		// What type of event occurred?
	uint32_t		broadcaster;	// normally creator code of broadcasting app
	void *		notifyDetailsP;	// ptr to notification-specific data, if any
	void *		userDataP;		// user specified ptr passed back with notification
	Boolean		handled;		// true if event is handled yet
	uint8_t		reserved2;
	uint16_t		padding;		// pad struct to 4-byte multiple
} SysNotifyParamType;


typedef status_t (*SysNotifyProcPtr)(SysNotifyParamType *notifyParamsP);

#define sysNotifyNormalPriority		0	// clients should use this priority


#include <LocaleMgr.h>
#include <DataMgr.h>
#include <SysEventCompatibility.h>

#define sysNotifyBroadcasterCode	sysFileCSystem // broadcaster code for events broadcast by the system

#define sysNotifyNoDatabaseH		((DatabaseID) 0xFFFFFFFF)

#define sysNotifyVersionNum			1	// version of the NotifyMgr, obtained from the feature

// The queue is much smaller in debug builds so that we aren't tempted to overflow it 
// (and larger for release builds so that when we do, it will work).

// DOLATER: implement a variable size queue
#define sysNotifyDefaultQueueSize	100 

/*
	Notify Manager Events:
	
	Note that in general, for notifications regarding the creation of information
	(e.g. sysNotifyDBAddedEvent) the broadcast goes out AFTER the fact (e.g.,
	after the database has been created) and that notifications regarding
	the deletion of information are broadcast BEFORE the fact.  A notable and
	unfortunate exception to this rule is the database deleted notification, which
	is sent out after the fact.

	The sleep & wake notifications require a little special attention.  These
	notifications are _not_ guaranteed to always be broadcast, and hence are unsuitable
	for applications where external hardware must be shut off to conserve power when
	we go to sleep.  For example, the sleep notification will not be broadcast when 
	we do an emergency  shutdown due to low battery conditions.  Also note that any 
	sort of prolonged activity (especially displaying UI sich as a dialog) in response 
	to these noticfications can complicate things immensely.  UI is very tricky because 
	these notifications are broadcast from SysHandleEvent.  The result is that you may 
	receive more than one sleep notification if one of the notification clients puts up 
	a dialog or something, and it remains there long enough for another AutoOff event 
	to come through.  There are also possible problems with reentrancy and stack space 
	usage since the notification is broadcast from SysHandleEvent inside an application's 
	event loop.
*/

#define sysNotifySyncStartEvent			'hots'	// Sent at the start of a hotsync.
												// Always sent from UI thread.
												// param: none
											
#define sysNotifySyncFinishEvent		'sync'	// Sent at the end of a hotsync.
												// Always sent from UI thread.
												// param: none

#define sysNotifyAntennaRaisedEvent		'tena'	// Sent when the antenna is raised on a
												// Palm VII series device.
												// Always sent from UI thread.
												// param: none

#define sysNotifyResetFinishedEvent		'rstf'	// Broadcast after all app launchcodes
												// are sent after a reset.
												// Always sent from UI thread.
												// param: none

#define sysNotifyForgotPasswordEvent	'bozo'	// Broadcast when the user presses the 
												// 'Forgotten Password' button in the
												// security app, just before every database's
												// private records are deleted.

#define sysNotifySecuritySettingEvent	'ssch'	// Broadcast when the security setting
												// is changed either by the user in the Security application or 
												// programmatically using SecSvcsSetDeviceSetting.
											
#define sysNotifyTimeChangeEvent		'time'	// Broadcast when the time is changed by the user.
												// param: int32_t*, ptr to time change delta in 
												// seconds (e.g., a value of 100 means that the 
												// user set the clock ahead 100 seconds).

#define sysNotifySleepRequestEvent		'slpq'	// A "Sleep Request Event" is broadcast when the device
												// is about to go to sleep, and is a chance for clients
												// to do perform an action or even delay going to sleep
												// for a little while.  This may be broadcast multiple 
												// times if one or more clients defer the sleep. Parameter 
												// is a pointer to a SleepEventParamType.  See below 
												// details on this structure. Note that this event is 
												// NOT guaranteed to be broadcast.  See the big comment 
												// at the top of this list for important detials & warnings.
												// Always sent from UI thread.

#define sysNotifySleepNotifyEvent		'slp!'	// A "Sleep Notify Event" is broadcast when the device is 
												// definitely going to sleep as soon as the broadcast 
												// is finished.  Parameter is unused.
												// Note that this event is NOT guaranteed to be broadcast.  
												// See the big comment at the top of this list for 
												// important detials & warnings.
												// Always sent from UI thread.


#define sysNotifyEarlyWakeupEvent		'worm'	// ...the early bird gets the worm...
												// Broadcast just after the device wakes up
												// at the early stage where the screen may 
												// still be turned off and we may quickly go 
												// back to sleep after handling an alarm or 
												// charger event.  
												// Always sent from UI thread.


#define sysNotifyLateWakeupEvent		'lazy'	// Broadcast after the device wakes up
												// at the later stage of the wakeup 
												// process after we turn the screen on, 
												// broadcast from EvtResetAutoOffEvent 
												// the first time it is called implying
												// that the device will remain awake 
												// for at least a little while.
												// Always sent from UI thread.


#define sysNotifyDisplayChangeEvent		'scrd'	// Sent when the display depth is
												// changed, notifyDetailsP has old/new depth
												// see SysNotifyDisplayChangeDetailsType
											

#define sysNotifyMenuCmdBarOpenEvent	'cbar'	// Sent by FormHandleEvent when a menuCmdBarOpenEvent
												// passes through. The system will not open the toolbar 
												// if the 'handled' field is set in the parameter block, 
												// so most clients should not set it.  The notification
												// is provided so that Hack-like entities can catch
												// it and add their own buttons to the bar. These
												// buttons will show up to the left of cut/copy/paste/undo and
												// to the right of everything else. Without this
												// notification, people would hack SysHandleEvent... ick.
												// Note that this is only sent when FormHandleEvent is called from
												// the UI thread.  When invoked from mother threads, the notification
												// is not sent.


#define cncNotifyConnectionStateEvent	'cncc'	// sent by the Connection Manager when a persistent profile
												// came connected or disconnected.

#define	sysExternalConnectorAttachEvent	'ecna'	// Broadcast when anything is attached to 
												// the external connector.

#define	sysExternalConnectorDetachEvent	'ecnd'	// Broadcast when anything is detached from 
												// the external connector.

#define sysNotifyCardInsertedEvent		'crdi'	// Broadcast when an ExpansionMgr card is 
												// inserted into a slot, and the slot driver 
												// calls ExpCardInserted.  Always broadcast
												// from UI task.
												// ExpansionMgr will play a sound & attempt to
												// mount a volume unless the corresponding 
												// bits in the 'handled' field are set by a 
												// notification handler (see ExpansionMgr.h).
												// PARAMETER: slot number cast as void*
															
#define sysNotifyCardRemovedEvent		'crdo'	// Broadcast when an ExpansionMgr card is 
												// removed from a slot, and the slot driver 
												// calls ExpCardRemoved.  Always broadcast
												// from UI task.
												// ExpansionMgr will play a sound & attempt to
												// unmount a volume unless the corresponding 
												// bits in the 'handled' field are set by a 
												// notification handler (see ExpansionMgr.h).
												// PARAMETER: slot number cast as void*

#define sysNotifyVolumeMountedEvent		'volm'	// Broadcast when a VFSMgr volume is 
												// mounted, Always broadcast from UI task.
												// VFSMgr will run start.prc (if present),
												// and SysUIAppSwitch  to it or the Launcher
												// unless the appropriate bits in the 'handled'
												// field are set by a notification handler.
												// PARAMETER: VFSAnyMountParamPtr cast as void*

#define sysNotifyVolumeUnmountedEvent	'volu'	// Broadcast AFTER a VFSMgr volume is 
												// unmounted, Always broadcast from UI task.
												// VFSMgr will delete start.prc (if it was loaded).
												// The volume ref number for the unmounted volume is
												// NO LONGER VALID, so don't bother trying to use it.
												// PARAMETER: volume refNum cast as void*

#define sysNotifyHelperEvent			'hlpr'	// Sent by Address Book (or any
												// 3rd party application) to
												// communicate with Phone Apps


#define sysNotifyPOSEMountEvent			'pose'  // Sent by HostFS to communicate with itself,
												// saving on stack depth.
												// For system use only.
											
#define sysNotifyHostFSInitDone			'hfid'  // Sent by HostFS when its Init() hook is about to return,
												// so the AutoMounter can mount volumes on POSE slots.
												// For system use only.
											
#define sysNotifyLocaleChangedEvent		'locc'	// Notify apps/panel that the system locale  
												// has changed.
												// This is broadcast by the language picker after it
												// has changed the locale.
												// Always sent from UI thread.
												// param: ptr to SysNotifyLocaleChangedType



#define sysNotifyEvtGotAttnEvent		'evtg'	// Sent by EvtGetEvent when it returns a
												// virtual keyDownEvent for the attention manager.
												// Used by the attention manager to detect whether it's
												// events are discarded without being passed to SysHandleEvent.
												// Stripped from public headers because this will
												// probably go away in the next release when we have
												// a better solution for the problem, and to avoid
												// potential performance problems. This notification went
												// out for all events in PalmOS 4.0, but is now limited to
												// only attention manager virtual keys to improve performance.
												// For system use only.



#define sysNotifyRetryEnqueueKey		'retk'	// An EvtEnqueueKey failed, so retry.
												// For system use only.

#define sysNotifyGotUsersAttention		'attn'	// Got users attention.

											
#define sysNotifyDBDeletedEvent			'dbs-'	// broadcast AFTER a database is removed from the device.
												// Note that the database ID in the parameter block is
												// NO LONGER VALID, and you WILL CRASH the device if you
												// try to pass it to any DataMgr routines.
												// notifyDetailsP: ptr to SysNotifyDBDeletedType.

#define sysNotifyDeviceUnlocked			'unlk'	// Broadcasted by the Security app After the device is 
												//unlocked.
												//notifyDetailsP: none 

#define sysNotifyPhoneEvent				'fone'	// Sent by third-party applications to communicate
												// with built-in or external phones.  
												// NOTE THAT THIS EVENT IS FOR FUTURE USE

#define sysNotifyDeleteProtectedEvent	'-pdb'	// Broadcast BEFORE a database is removed from the device when a user
												// or installer application wants to remove a protected database.  This
												// will ONLY be issued from the UI task and should be issued before each
												// user driven deletion that targets a protected database that the app
												// running does not own (i.e. needs to request be unprotected).
												// This gives the unlucky client a chance to do any required 
												// cleanup work.  Note that if an application has multiple protected 
												// databases, this notification may be sent out more than once.
												// notifyDetailsP: ptr to SysNotifyDBInfoType.

#define sysNotifyDBCreatedEvent			'dbcr'	// Sent by DataMgr when a database is created (in DmCreateDatabase)
												// notifyDetailsP: ptr to SysNotifyDBCreatedType

#define sysNotifyDBChangedEvent			'dbmn'	// Sent by DataMgr when a database's info is modified (in DmSetDatabaseInfo)
												// notifyDetailsP: ptr to SysNotifyDBChangedType

#define sysNotifyDBDirtyEvent			'dbdr'	// Sent by DataMgr when a database is 'dirtied'
												// notifyDetailsP: ptr to SysNotifyDBDirtyType

#define sysNotifyDBAddedEvent			'dbs+'	// Broadcast by DataMgr after a database is added to the device
												// param: ptr to SysNotifyDBAddedType.

#define sysNotifyAppServicesEvent		'apsv'	// Broadcast when application services are terminated.

#define sysNotifyAltInputSystemEnabled	'aise'	// Sent by alternative (text) input system, such as external keyboard
#define sysNotifyAltInputSystemDisabled	'aisd'	// driver, when it becomes enabled/disabled.

// These notifications are no longer sent by the OS.
//#define cncNotifyProfileEvent				'cncp'
//#define sysNotifyNetLibIFMediaEvent		'neti'
//#define sysNotifyIrDASniffEvent			'irda'
//#define sysNotifyProcessPenStrokeEvent	'hpps'
//#define sysNotifyVirtualCharHandlingEvent	'hvch'
//#define sysNotifyEventDequeuedEvent		'hede'
//#define sysNotifyIdleTimeEvent			'hidl'
//#define sysNotifyAppLaunchingEvent		'hapl'
//#define sysNotifyAppQuittingEvent			'hapq'
//#define sysNotifyInsPtEnableEvent			'hipe'
//#define sysNotifyKeyboardDialogEvent		'hkbd'

// for sysNotifyAppLaunchingEvent and sysNotifyAppQuittingEvent
typedef struct SysNotifyAppLaunchOrQuitTag {
	uint32_t    version;   // this is version 0
	uint32_t    dbID;      // the application being started or quitting (note: type unsafeness is intentional)
	uint16_t    cardNo;    // the application being started or quitting
	uint16_t    padding;   // pad struct to 4-byte multiple
} SysNotifyAppLaunchOrQuitType;


// for sysNotifyVirtualCharHandlingEvent
typedef struct SysNotifyVirtualCharHandlingTag {
	uint32_t                     version;   // this is version 0
	struct _KeyDownEventType   keyDown;
} SysNotifyVirtualCharHandlingType;


// for sysNotifyProcessPenStrokeEvent
typedef struct SysNotifyPenStrokeTag {
	uint32_t     version;   // this is version 0
	PointType  startPt;
	PointType  endPt;
} SysNotifyPenStrokeType;


// for sysNotifyDisplayChangeEvent
typedef struct SysNotifyDisplayChangeDetailsTag {
	uint32_t oldDepth;
	uint32_t newDepth;
	} SysNotifyDisplayChangeDetailsType;


// For sysNotifyLocaleChangedEvent
typedef struct SysNotifyLocaleChangedTag {
	LmLocaleType 	oldLocale;
	LmLocaleType 	newLocale;
} SysNotifyLocaleChangedType;


// Possible values for the sleep-reason for the sysNotifySleepEvent
#define sysSleepPowerButton		0
#define sysSleepAutoOff			1
#define sysSleepResumed			2
#define sysSleepUnknown			3


// for sysNotifySleepRequestEvent
typedef struct SleepEventParamTag {
	uint16_t reason;		// contains the reason we are going to sleep. See above list for possible values.
	uint16_t deferSleep;	// Only used for the sysNotifySleepRequestEvent, this should be incremented by 
						// clients that want to defer the sleep.  They are then responsible for enqueing
						// a resumeSleepChr in the event queue.
} SleepEventParamType;


// for sysNotifyDeleteProtectedEvent, and possibly others...
typedef struct SysNotifyDBInfoTag {
	MemHandle	dbID;					// database ID of dabatase
	uint16_t		cardNo;					// card number that database resided on
	uint16_t		attributes;				// database attributes
	char		dbName[dmDBNameLength];	// name of database
	uint32_t		creator;				// creator code of dabatase
	uint32_t		type;					// type of dabatase
} SysNotifyDBInfoType;

// for sysNotifyDBDeletedEvent
typedef struct SysNotifyDBDeletedTag {
	DatabaseID	oldDBID;
	char		name[dmDBNameLength];
	uint32_t		creator;
	uint32_t		type;
	uint16_t		attributes;
	uint16_t		reserved;
} SysNotifyDBDeletedType;


// for sysNotifyDBCreatedEvent
typedef struct SysNotifyDBCreatedTag {
	DatabaseID	newDBID;
	char		name[dmDBNameLength];
	uint32_t		creator;
	uint32_t		type;
	uint16_t		attributes;
	uint16_t		reserved;
} SysNotifyDBCreatedType;


// for sysNotifyDBChangedEvent
typedef struct SysNotifyDBChangedTag {
	DatabaseID	dbID;
	char     	name[dmDBNameLength];
	uint32_t   	creator;
	uint32_t   	type;
	uint16_t		attributes;

	uint16_t   	version;
	uint32_t   	crDate;
	uint32_t   	modDate;
	uint32_t   	bckUpDate;
	uint32_t   	modNum;
	MemHandle	appInfoH;
	MemHandle	sortInfoH;

	char     	displayName[dmDBNameLength];
	uint16_t		encoding;
	uint16_t   	fields;					// what fields above are set?
		
	// Old field values, prior to the DmSetDatabaseInfo
	char    	oldName[dmDBNameLength];
	uint32_t   	oldCreator;
	uint32_t   	oldType;
	uint16_t   	oldAttributes;

	uint16_t		reserved;
} SysNotifyDBChangedType;

// for sysNotifyDBDirtyEvent
typedef struct SysNotifyDBDirtyTag {
	DatabaseID	dbID;
	char		name[dmDBNameLength];
	uint32_t		creator;
	uint32_t		type;
	uint16_t		attributes;
	uint16_t		reserved;
} SysNotifyDBDirtyType;

// for sysNotifyDBAddedEvent
typedef SysNotifyDBCreatedType	SysNotifyDBAddedType;

// Definitions for changed fields set in fields field of SysNotifyDBChangedType
#define DBChangedFieldSetName             0x1
#define DBChangedFieldSetCreator          0x2
#define DBChangedFieldSetType             0x4
#define DBChangedFieldSetCrDate           0x8
#define DBChangedFieldSetModDate         0x10
#define DBChangedFieldSetBckUpDate       0x20
#define DBChangedFieldSetModNum          0x40
#define DBChangedFieldSetAppInfo         0x80
#define DBChangedFieldSetSortInfo       0x100
#define DBChangedFieldSetAttributes     0x200
#define DBChangedFieldSetVersion        0x400
#define DBChangedFieldSetDisplayName    0x800
#define DBChangedFieldSetEncoding	   0x1000


/*
Possible event types to be added in the future:
NOTE that these are NOT implemented yet!!!!
											
#define sysNotifyInitializeEvent	'helo'	// broadcast to an app after its installed
											// so it can do any necessary initalization
											// This event is always broadcast.
											// param: ptr to SysNotifyDBInfoType.

#define sysNotifyCleanupEvent		'gbye'	// broadcast to an app just before its deleted
											// so it can do any necessary cleanup work.
											// This event is always broadcast.
											// The database's type will have been set to 
											// sysFileTTemp so that if case the handler 
											// crashes, the DB will be deleted on reset.
											// param: ptr to SysNotifyDBInfoType.

#define sysNotifyCardChangedEvent	'card'	// broadcast when the owner's business card
											// has changed, allowing apps to adjust for that.
											// param: ptr to new owner data.

*/



#ifdef __cplusplus
extern "C" {
#endif

status_t SysNotifyRegister(DatabaseID database, uint32_t notifyType, 
						SysNotifyProcPtr callback, int32_t priority,
						void *userData, uint32_t userDataSize);

status_t SysNotifyRegisterV40(uint16_t cardNo, LocalID dbID, uint32_t notifyType, 
						SysNotifyProcPtr callback, int8_t priority, void *userData);

status_t SysNotifyRegisterBackground(DatabaseID database, uint32_t notifyType, 
						int32_t priority, void *userData, uint32_t userDataSize);

status_t SysNotifyUnregister(DatabaseID database, uint32_t notifyType, int32_t priority);

status_t SysNotifyUnregisterV40(uint16_t cardNo, LocalID dbID, uint32_t notifyType, int8_t priority);

/*
 *	WARNING: Never call SysNotifyBroadcast from a background task.
 *			 This requirement is more restrictive than it used to be.
 */
status_t SysNotifyBroadcast(SysNotifyParamType *notify);

status_t SysNotifyBroadcastDeferred(SysNotifyParamType *notify, uint32_t paramSize);


#ifdef __cplusplus 
}
#endif



#endif	// __NOTIFY_H__
