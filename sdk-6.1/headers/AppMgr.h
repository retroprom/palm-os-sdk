/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AppMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Declarations for the PalmOS Application Manager
 *
 *****************************************************************************/

#ifndef _APPMGR_H_
#define _APPMGR_H_

#include <PalmTypes.h>	// Include elementary types
#include <DataMgr.h>		// for DmOpenRef, DatabaseID
#include <CmnErrors.h>  // for appMgrErrorClass

/************************************************************
 * Rules for creating and using the Command Parameter Block
 * passed to SysUIAppSwitch
 *************************************************************/

// A parameter block containing application-specific information may be passed
// to an application when launching it via SysUIAppSwitch.  To create the
// parameter block, you allocate a memory block using MemPtrNew and then you must
// call MemPtrSetOwner to set the block's owner ID to 0.  This assigns the block's
// ownership to the system so that it will not be automatically freed by the system
// when the calling app exits. The command block must be self contained. It must not
// have pointers to anything on the stack or in memory blocks owned by an application.
// The launching and launched applications do not need to worry about freeing the
// command block since the system will do this after the launched application exits.
// If no parameter block is being passed, this parameter must be NULL.

//-------------------------------------------------------------------
// Parameter blocks for action codes
// NOTE: The parameter block for the  sysAppLaunchCmdFind  and sysAppLaunchCmdGoTo
//  action codes are defined in "Find.h";
//---------------------------------------------------------------------------

// For sysAppLaunchCmdSaveData
typedef struct {
	Boolean		uiComing;							// true if system dialog will be put up
															// before coming action code arrives.
	uint8_t			reserved1;
	} SysAppLaunchCmdSaveDataType;
	
// For sysAppLaunchCmdSystemReset
typedef struct {
	Boolean		hardReset;							// true if system was hardReset, false if soft-reset.
	Boolean		createDefaultDB;					// true if app should create default database.
	} SysAppLaunchCmdSystemResetType;
	

// For sysAppLaunchCmdInitDatabase
typedef struct SysAppLaunchCmdInitDatabaseType {
	DmOpenRef	dbP;									// Handle of the newly-created database,
															//		already open for read/write access.
															//		IMPORTANT: The handler *MUST* leave
															//		this database handle open on return.
	uint32_t		creator;								//	Creator ID of the newly-created database
	uint32_t		type;									// Type ID of the newly-created database
	uint16_t		version;								// Version number of the newly-created database
	uint16_t		padding;
	} SysAppLaunchCmdInitDatabaseType;


// For sysAppLaunchCmdSyncCallApplicationV10
// This structure used on Pilot v1.0 only.  See sysAppLaunchCmdHandleSyncCallApp
// for later platforms.
typedef struct SysAppLaunchCmdSyncCallApplicationTypeV10 {
	uint16_t		action;					// call action id (app-specific)
	uint16_t		paramSize;				// parameter size
	void *		paramP;					// ptr to parameter
	uint8_t			remoteSocket;			// remote socket id
	uint8_t			tid;						// command transaction id
	Boolean		handled;					// if handled, MUST be set true by the app
	uint8_t			reserved1;
	} SysAppLaunchCmdSyncCallApplicationTypeV10;


// For sysAppLaunchCmdHandleSyncCallApp (Pilot v2.0 and greater).
// This structure replaces SysAppLaunchCmdSyncCallApplicationType
// which was used in Pilot v1.0
typedef struct SysAppLaunchCmdHandleSyncCallAppType {
	uint16_t		pbSize;					// this parameter block size (set to sizeof SysAppLaunchCmdHandleSyncCallAppType)
	uint16_t		action;					// call action id (app-specific)
	void *		paramP;					// ptr to parameter
	uint32_t		dwParamSize;			// parameter size
	void *		dlRefP;					// DesktopLink reference pointer for passing
												// to DlkControl()'s dlkCtlSendCallAppReply code
												
	Boolean		handled;					// initialized to FALSE by DLServer; if
												// handled, MUST be set TRUE by the app(the
												// handler MUST call DlkControl with
												// control code dlkCtlSendCallAppReply);
												// if the handler is not going to send a reply,
												// it should leave this field set to FALSE, in which
												// case DesktopLink Server will send the default
												// "unknown request" reply.
	
	uint8_t		_reserved1;
	uint16_t		_reserved2;
											
	status_t			replyErr;				// error from dlkCtlSendCallAppReply
	
	// RESERVED FOR FUTURE EXTENSIONS				
	uint32_t		dwReserved1;			// RESERVED -- set to null!!!
	uint32_t		dwReserved2;			// RESERVED -- set to null!!!

	} SysAppLaunchCmdHandleSyncCallAppType;

// For sysAppLaunchCmdFailedAppNotify
typedef struct
	{
	uint32_t		creator;
	uint32_t		type;
	status_t			result;
	} SysAppLaunchCmdFailedAppNotifyType;
	
	
// For sysAppLaunchCmdOpenDB
typedef struct
	{
	MemHandle	dbH;
	} SysAppLaunchCmdOpenDBType;
	

// For sysAppLaunchCmdCardLaunch
typedef struct
	{
	status_t			err;
	uint16_t		volRefNum;
	uint16_t		_reserved1;
	const char	*path;
	uint16_t		startFlags;			// See vfsStartFlagXXX constants below
	uint16_t		padding;
	} SysAppLaunchCmdCardType;

#define sysAppLaunchStartFlagAutoStart		0x0001	// this bit in the 'startFlags' field is set for an app which is run automatically on card insertion
#define sysAppLaunchStartFlagNoUISwitch		0x0002	// set this bit in the 'startFlags' field to prevent a UI switch to the start.prc app
#define sysAppLaunchStartFlagNoAutoDelete	0x0004	// set this bit in the 'startFlags' field to prevent VFSMgr from deleting start.prc app on volume unmount


//for launch code sysAppLaunchPnpsPreLaunch
typedef struct {
	status_t		error;			//an error code from the pre-launch application, set to errNone to prevent normal launching
	uint16_t	volRefNum;		//Non-zero if an optional file system was mounted
	uint16_t	slotLibRefNum;	//always valid for a slot driver call
 	uint16_t	slotRefNum;		//always valid for a slot driver call
	uint16_t  _reserved1;
 }SysAppLaunchCmdPnpsType;


#define ImpExpInvalidUniqueID		dmUnusedRecordID
#define ImpExpInvalidRecIndex		0xFFFFFFFF

// Param Block passed with the sysAppLaunchCmdImportRecord, sysAppLaunchCmdExportRecord and
// SysAppLaunchCmdDeleteRecord launch codes.
typedef struct
{
	uint32_t 		index;
		// Index in Database
		//  - when importing : discarded.
		//  - when exporting : indicates the record to export.
		// This value is updated after sublaunch.

	uint32_t 		destIndex;
		// New position in Database, only used with sysAppLaunchCmdMoveRecord.
		// The sysAppLaunchCmdMoveRecord launch code may not be supported by applications
		// (dmErrInvalidParam is returned in such case).

	uint32_t 		uniqueID;
		// Unique ID for the imported/exported record
		//  - when importing : if set to a valid record unique ID,
		//	  (different from ImpExpInvalidUniqueID == 0) it replaces
		//	  the whole record information by imported record.
		//	- when exporting : if index equals ImpExpInvalidRecIndex (0xFFFFFFFF),
		//	  the record that matches with this uniqueID is exported. If the
		//	  index value is valid, this uniqueID is discarded.
		// This value is updated after sublaunch.
		
	MemHandle	vObjectH;
		// Allocated memHandle containing the record to export
		// or accepting the record to import.
		//   This field can be NULL with sysAppLaunchCmdExportRecord
		// launch code. If so, the matching uniqueID will be returned
		// if index is valid. If index is set ImpExpInvalidRecIndex and
		// uniqueID is valid, then the matching index will be returned.
} ImportExportRecordParamsType;

typedef ImportExportRecordParamsType *ImportExportRecordParamsPtr;


/************************************************************
 * Declaration of the ARM Launch Preference resource
 * (sysResTAppLaunchPrefs#sysResIDAppLaunchPrefs)
 *************************************************************/

typedef struct ARMAppLaunchPrefsType {
	uint32_t version;         // Version of this structure
	uint32_t reserved1;  // Not used. Used to be version of PalmOS that was targeted
		// when the application was built.
	uint32_t reserved2;	// Not used. Used to be minimum required version of PalmOS
		// minOSVersion and buildOSVersion are in the same
		// format as the system version feature sysFtrNumROMVersion
	uint32_t stackSize;		// required stack space in bytes. 0 => use default.
	uint32_t flags;			// See flags defined below
} ARMAppLaunchPrefsType;

#define ARMAppLaunchPrefsTypeVersion60 1	// first version in 6.0
#define ARMAppLaunchPrefsTypeVersionCurrent ARMAppLaunchPrefsTypeVersion60

// Bits in flags:
#define ARMAppLaunchPrefsResetNotification	    0x01
		// If set, app will be
		// sent sysAppLaunchCmdSystemReset action code
#define ARMAppLaunchPrefsTimeChangeNotification 0x04
		// If set, app will be
		// sent sysAppLaunchCmdTimeChange action code
#define ARMAppLaunchPrefsFindNotification       0x02
		// If set, app will be
		// sent sysAppLaunchCmdFind action code
#define ARMAppLaunchPrefsNoOverlay              0x20
		// Used by the Overlay Manager (formerly in 'xprf' resource)

#if 0  // These bits are not used
#define ARMAppLaunchPrefsInstallNotification    0x08
		// If set, app will be
		// sent sysAppLaunchCmdSyncNotify action code when it is
		// installed (by HotSync or ExgMgr)
#define ARMAppLaunchPrefsDBChangeNotification   0x10
		// If set, app will be
		// sent sysAppLaunchCmdDBChange action code when databases
		// that have the same creator ID as the application have
		// been modified (by HotSync or ExgMgr)
#define ARMAppLaunchPrefsReserved         0xffffffc0 // Must be zero
#endif
#define ARMAppLaunchPrefsReserved         0xffffffd8 // Must be zero


/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (PilotMainType)(uint16_t cmd, void * cmdPBP, uint16_t launchFlags);

// Prototype for Pilot applications entry point
uint32_t PilotMain(uint16_t cmd, void * cmdPBP, uint16_t launchFlags);

							
status_t SysBroadcastActionCode(uint16_t cmd, void * cmdPBP);

status_t SysUIAppSwitch(DatabaseID dbID, uint16_t cmd, void *cmdPBP, uint32_t cmdPBSize);
status_t SysUIAppSwitchV40(uint16_t cardNo, LocalID dbID, uint16_t cmd, MemPtr cmdPBP);
							
status_t SysCurAppDatabase(DatabaseID * dbIDP);
status_t SysCurAppDatabaseV40(uint16_t *cardNoP, LocalID *dbIDP);
							
status_t SysAppLaunch (DatabaseID dbID, uint16_t cmd, void *cmdPBP, uint32_t* resultP);
status_t SysAppLaunchRemote (DatabaseID dbID, uint16_t cmd, void *cmdPBP,
                                       uint32_t cmdPBSize, uint32_t* resultP);
status_t SysAppLaunchLocal (DatabaseID dbID, uint16_t cmd, void *cmdPBP,
                                       uint32_t cmdPBSize, uint32_t* resultP);
status_t SysAppLaunchV40(uint16_t cardNo, LocalID dbID, uint16_t launchFlags,
                 uint16_t cmd, MemPtr cmdPBP, uint32_t *resultP);
Boolean SysGetStackInfo(void **startPP, void **endPP);

void SysReset(void);

#ifdef __cplusplus
}
#endif

#endif  //_APPMGR_H_
