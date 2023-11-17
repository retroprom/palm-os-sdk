/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CncMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Client header file for the connection manager
 *
 *****************************************************************************/

#ifndef _CNCMGR_H_
#define _CNCMGR_H_

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <Form.h>

// Error codes (they dont clash with old cncErrXXXX errors)
#define cncErrInvalidParam				((status_t) (cncErrorClass | 0x20))
#define cncErrOpenFailed				((status_t) (cncErrorClass | 0x21))
#define cncErrObjectTableFull			((status_t) (cncErrorClass | 0x22))
#define cncErrInvalidPluginModule		((status_t) (cncErrorClass | 0x23))
#define cncErrMemory					((status_t) (cncErrorClass | 0x24))
#define cncErrNotImplemented			((status_t) (cncErrorClass | 0x25))
#define cncErrObjectNotFound			((status_t) (cncErrorClass | 0x26))
#define cncErrLockedCount				((status_t) (cncErrorClass | 0x27))
#define cncErrCannotAllocateObject		((status_t) (cncErrorClass | 0x28))
#define cncErrObjectFull				((status_t) (cncErrorClass | 0x29))
#define cncErrIndexOutOfRange			((status_t) (cncErrorClass | 0x2a))
#define cncErrDatabase					((status_t) (cncErrorClass | 0x2b))
#define cncErrCommunication				((status_t) (cncErrorClass | 0x2c))
#define cncErrPluginModuleInitFailed	((status_t) (cncErrorClass | 0x2d))
#define cncErrInvalidObject				((status_t) (cncErrorClass | 0x2e))
#define cncErrObjectAlreadyExists		((status_t) (cncErrorClass | 0x2f))
#define cncErrMandatoryParameterNotFound ((status_t) (cncErrorClass | 0x30))
#define cncErrModuleAlreadyLoaded		((status_t) (cncErrorClass | 0x31))
#define cncErrNoPluginForm				((status_t) (cncErrorClass | 0x32))
#define cncErrSessionTableFull			((status_t) (cncErrorClass | 0x33))
#define cncErrExclusiveObject			((status_t) (cncErrorClass | 0x34))
#define cncErrObjectInUse				((status_t) (cncErrorClass | 0x35))
#define cncErrAlreadyDisconnecting		((status_t) (cncErrorClass | 0x36))
#define cncErrUndeletableObject			((status_t) (cncErrorClass | 0x37))
#define cncErrReadOnlyObject			((status_t) (cncErrorClass | 0x38))

// object types
#define kCncAllObjects					((uint8_t) 0xFF)
#define kCncPluginObject				((uint8_t) 0x01)
#define kCncInterfaceObject				((uint8_t) 0x02)
#define kCncProfileObject				((uint8_t) 0x04)
#define kCncTemplateObject				((uint8_t) 0x08)
#define kCncLinkObject					((uint8_t) 0x10)

// parameter value types
// kCncUndefinedParameterType must be 0x00
#define kCncUndefinedParameterType		((uint8_t) 0)
#define kCncIntegerParameterType		((uint8_t) 1)
#define kCncStringParameterType			((uint8_t) 2)
#define kCncBinaryParameterType			((uint8_t) 3)

// reserved/predefined parameter names. Reserved range must include 0
#define kCncParameterFirstReserved		((uint32_t) 0x00000000)
#define kCncParameterLastReverved		((uint32_t) 0x0000001F)
// the following constant must be 0
#define kCncParameterTableEnd			kCncParameterFirstReserved
#define kCncParameterUndefined			(kCncParameterFirstReserved + 1)

// The following value can be used as an index in CncProfileInsertItem
// to insert an item at the end of the profile
#define kCncInsertAtEndIndex			((int16_t)	0x7FFF)

// CncProfileGetParameters method constants. method can also be
// in the -63..+62 range to directly specify an count of items in one
// or the other direction.
#define kCncGetParametersWholeProfile	63
#define kCncGetParametersItemOnly		0
#define kCncGetParametersInherited		-64
#define kCncGetParametersActive			128

// max length of an object name
#define kCncMaxNameLength				31
#define kCncMaxFriendlyNameLength		191

// Here are the two constants with their original (incorrect) spelling:
#define kCncMaxNameLenght				kCncMaxNameLength
#define kCncMaxFriendlyNameLenght		kCncMaxFriendlyNameLength

// flags to manipulate object information (see CncObjectSetInfo)
#define	kCncNameInfoFlag				((uint32_t) 1)
#define	kCncPriorityInfoFlag			((uint32_t) 2)
#define	kCncAvailabilityInfoFlag		((uint32_t) 4)
#define	kCncOptionsInfoFlag				((uint32_t) 8)
#define	kCncOptionsSetInfoFlag			((uint32_t) 16)
#define	kCncOptionsClearInfoFlag		((uint32_t) 32)
#define	kCncOptionsInvertInfoFlag		((uint32_t) 64)
#define	kCncToggleTemplateInfoFlag		((uint32_t) 128)
#define	kCncManualIdInfoFlag			((uint32_t) 256)
#define kCncToggleLinkInfoFlag			((uint32_t) 512)

#define kCncEdgePriority				((uint8_t)  0)
#define kCncUnknownProfilePriority		((uint8_t)  1)

// invalid object Id
#define kCncInvalidRecordId				((uint32_t) 0)

// plugins root and wizard entry plugins (technologies) interfaces
#define kCncPluginsRoot					"PluginsRoot"
#define kCncTechnologiesRoot			"TechnologiesRoot"
#define kCncUsingMacro					"{USING}"
#define kCncReplaceMacro				"{REPLACE}"

// connect options
#define kCncConnectProgressUI			1
#define kCncConnectChooserUI			2
#define kCncConnectDisableFallback		4
#define kCncConnectDisableReconnection	8
#define kCncConnectAsynchronous			16

// configuration application mode
typedef uint32_t CncEditMode;

// configuration application mode constants
#define kCncTechnologyMode				((CncEditMode) 1)
#define kCncProfileMode					((CncEditMode) 2)
#define kCncNewMode						((CncEditMode) 4)
#define kCncDeleteMode					((CncEditMode) 8)
#define kCncEditMode					((CncEditMode) 16)
#define kCncFullMode					(kCncTechnologyMode | kCncProfileMode | kCncNewMode | kCncDeleteMode | kCncEditMode)
#define kCncPanelMode					(kCncProfileMode | kCncNewMode | kCncDeleteMode | kCncEditMode)
#define kCncAppSwitchMode				((CncEditMode) 0x8000)
#define kCncNoDoneButtonMode			((CncEditMode) 0x4000)

// update code for configuration apps
#define kCncUpdateFormDone				0x7FFF
#define kCncUpdateFormCancel			0x7FFE
#define kCncUpdateFormAction			0x7FFD
#define kCncUpdateFormDelete			0x7FFC
#define	kCncUpdateUser					0x7000

// Options flags for CncInfoType
#define kCncHasUIOption					0x8000
#define kCncHasConnectOption			0x4000
#define kCncHasFriendlyNameOption		0x2000
#define kCncIsConnectedOption			0x1000
#define kCncUsableOption				0x0001
#define kCncVisibleOption				0x0002
#define kCncUnsearchableOption			0x0004
#define kCncReadOnlyOption				0x0008
#define kCncReplaceOnUsingOption		0x0010
#define kCncUndeletableOption			0x0020
#define kCncPersistentOption			0x0040
#define kCncManualModeOption			0x0080
#define kCncUserCanChangeModeOption		0x0100
#define kCncTestableOption				0x0200

// Constants to use with CncObjectMoveItem
#define	kCncMoveUp						-1
#define	kCncMoveDown					+1
#define	kCncMoveAsDefault				-128
#define	kCncMoveLast					127

// constant introducing a special macro profile
#define	kCncMacroSpecialChar			'{'

// connection state constants to use with CncConnectionStateType
#define kCncConnectedState				0
#define kCncConnectingState				1
#define kCncDisconnectedState			2
#define kCncDisconnectingState			3

// kind of disconnection/cancel
#define	kCncNotCancelled				0
#define	kCncUserCancelled				1
#define	kCncSystemCancelled				2
#define kCncUserDisconnected			3
#define	kCncSystemDisconnected			4

// availability constants
#define kCncUnknownAvailability			0
#define kCncAvailable					1
#define kCncPercentBaseAvailability		100
#define kCncNotAvailable				255

// structure to manipulate item parameters
typedef struct CncParameterTag {
	uint32_t		name;					// parameter name
	uint16_t		size;					// size of the value in bytes
	uint8_t			type;					// parameter type (see kCncXxxxParameterType)
	uint8_t			reserved;
	union {
		int32_t		asInteger;
		char*		asString;
		uint8_t*	asBinary;
	} value;							// parameter value
} CncParameterType;

// structure to manipulate object information
typedef struct CncInfoTag {
	char			name[kCncMaxNameLength + 1];	// RW
	uint32_t		objectId;						// RO
	uint8_t			version;						// RO
	uint8_t			type;							// RW (profile to/from template)
	uint16_t		count;							// RO
	uint8_t			priority;						// RW
	uint8_t			availability;					// RW
	uint16_t		options;						// RW
	uint32_t		manualId;						// RW

	uint32_t		availModuleCreator;				// RO
	uint32_t		availModuleType;				// RO
	uint16_t		availIconId;					// RO	availabalility icon
	uint16_t		padding0;						// RO
} CncInfoType;

// structure of a connection state transition notification
typedef struct CncConnectionStateTag {
	int32_t			asyncId;				// asynchronous operation id
	uint32_t		profileId;				// connected profile Id
	uint32_t		flags;
	status_t		error;					// error (when disconnected)
	int32_t			fd;						// fd of a connected in the system process
	uint16_t		state;					// connection state
	uint16_t		prgResoureId;			// module to find the progress info in
	uint32_t		prgModuleCreator;		
	uint32_t		prgModuleType;
	uint16_t		prgStringId;			// progress string
	uint16_t		prgIconId;				// progress icon
} CncConnectionStateType;

typedef uint32_t CncFindOptionsType;

typedef union CncEditParametersTag {
	uint32_t		appSelector;
	void*			appData;
} CncEditParametersType;

// asynchronous termination notification 
typedef void (*CncAsyncNotifyFunc)(uint32_t asyncId, MemPtr dataP); 

#define CncFindGetType(X)			((uint8_t) ((X) & 0x000000FF))
#define kCncFindAllObjects			((CncFindOptionsType) kCncAllObjects)
#define kCncFindPluginObjects		((CncFindOptionsType) kCncPluginObject)
#define kCncFindInterfaceObjects	((CncFindOptionsType) kCncInterfaceObject)
#define kCncFindProfileObjects		((CncFindOptionsType) kCncProfileObject)
#define kCncFindTemplateObjects		((CncFindOptionsType) kCncTemplateObject)
#define kCncFindLinkObjects			((CncFindOptionsType) kCncLinkObject)
#define kCncFindAvailableOnly		((CncFindOptionsType) 0x100)
#define kCncFindUsableOnly			((CncFindOptionsType) 0x200)
#define kCncFindInvisible			((CncFindOptionsType) 0x400)

#define kCncFindAllCountMax			128

#define kCncFindDefault				(kCncFindProfileObjects)

// Predefined control requests are in the 0x00000000 to 0x000000FF range
#define kCncControlTest				0x00000080
#define kCncControlUserChange		0x00000081
#define kCncControlAvailability		0x00000082

#define cncNotifyEvent				lastUserEvent+1

// parameter block for the sysCncWizardLaunchCmdEdit launch command
typedef struct CncWizardEditPBTag {
	uint32_t					Id;
	CncEditMode					launchMode;
	CncEditMode					enabledModes;
	CncEditParametersType*		params;
	status_t					error;
} CncWizardEditPBType;

// structure used in CncObjectControl
typedef struct CncControlTag {
	uint16_t		size;			// <- the size of the structure			
	uint16_t		session;		// <- the session to use un CncSrvXXXX functions
	status_t		error;			// -> the error
	uint32_t		Id;				// <- the object Id associated w/ the function
	int16_t			index;			// <- the current plugin index in the profile or -1 for plugins
	int16_t			count;			// <- the items total count in the profile or -1 for plugins
	uint32_t		data;			// <-> function specific data
} CncControlType;

// the availability control type
typedef struct CncControlAvailabilityTag {
	CncControlType	control;
	uint8_t			availability;
	uint8_t			padding1;
	uint16_t		iconId;
	uint32_t		moduleCreator;
	uint32_t		moduleType;
} CncControlAvailabilityType;

#ifdef __cplusplus
extern "C" {
#endif

extern status_t CncInterfaceNew(const char* nameStr, uint32_t* Id);

extern status_t CncEdgeNew(uint32_t fromId, uint32_t toId);

extern status_t CncEdgeDelete(uint32_t fromId, uint32_t toId);

extern status_t CncObjectDelete(uint32_t nodeId);

extern uint32_t CncObjectGetIndex(const char* nameStr);

extern status_t CncObjectGetInfo(uint32_t recordId, CncInfoType* infoP);

extern status_t CncObjectSetInfo(uint32_t recordId, CncInfoType* infoP, uint32_t flags);

extern status_t CncRegisterNotify(uint32_t Id);

extern status_t CncRegisterPluginModule(uint32_t dbType, uint32_t dbCreator, uint16_t rsrcId);

extern int32_t CncProfileFindConnect(char* profileStr, uint32_t flags, uint32_t* profileIdP, status_t* error);

extern int32_t CncProfileConnect(uint32_t profileId, uint32_t flags, status_t* error);

extern status_t CncConnectReceiveState(CncConnectionStateType* stateP);

extern status_t CncProfileDisconnect(uint32_t profileId, uint8_t kind);

extern status_t CncProfileDeleteItem(uint32_t lockedId, uint32_t itemIndex);

extern status_t CncProfileGetParameters(uint32_t lockedId, int16_t itemIndex, int8_t method, CncParameterType parameters[]);

extern status_t CncProfileSetParameters(uint32_t lockedId, int16_t itemIndex, CncParameterType parameters[]);

extern status_t CncProfileInsertItem(uint32_t lockedId, int16_t* atIndex, uint32_t itemId, CncParameterType parameters[]);

extern uint32_t CncProfileGetLength(uint32_t lockedId);

extern status_t CncProfileGetItemIndex(uint32_t lockedId, int16_t* index, char* nameStr);

extern uint32_t CncProfileGetItemId(uint32_t lockedId, int16_t index);

extern uint32_t CncProfileDecode(const char* profileName, const char* pathStr);

extern char* CncProfileEncode(uint32_t profileId, int16_t index);

extern status_t CncProfileLock(uint32_t Id);

extern status_t CncProfileUnlock(uint32_t lockedId);

extern status_t CncProfileSubmit(uint32_t lockedId);

extern status_t CncProfileAttach(uint32_t fromId, uint32_t toId);

extern status_t CncProfileNew(char* profileName, uint32_t* lockedId);

extern uint32_t CncProfileFindFirst(char* searchStr, CncFindOptionsType options, uint32_t* searchIdP, status_t* errP);

extern uint32_t CncProfileFindNext(uint32_t searchId, status_t* errP);

extern void CncProfileFindClose(uint32_t searchId);

extern status_t CncObjectFindAll(uint32_t objectId, CncFindOptionsType options, int16_t *countP, CncInfoType** infoArrayP);

extern status_t CncObjectGetDependencies(uint32_t objectId, uint32_t** objectsPP, int16_t* countP);

extern status_t CncProfileEdit(uint32_t* Id, CncEditMode launchMode, CncEditMode enabledModes, CncEditParametersType* params);

extern void  CncCloseSession(void);

extern int32_t CncGetOrOpenSession(void);

extern int32_t CncGetSession(void);

extern void CncParametersInit(CncParameterType parameters[], int32_t n);

extern void CncParametersFree(CncParameterType parameters[]);

extern void CncDoUniqueName(char* nameP, uint32_t uid);

extern status_t CncProfileCopy(uint32_t* profileId, int16_t from, int16_t to);

extern status_t CncProfileUngroup(uint32_t profileId, Boolean regroupTags);

extern status_t CncProfileRegroupSubmit(uint32_t lockedId);

extern status_t CncSubProfileAssign(uint32_t lockedId, int16_t itemIndex, uint32_t newRefId);

extern status_t CncObjectMoveItem(uint32_t itemId, int16_t newIndexRelative);

extern status_t CncObjectControl(uint32_t objectId, uint32_t request, CncControlType* controlP);

extern status_t CncPluginRunForm(uint32_t profileId, uint16_t tabIndex);

extern status_t CncPluginRegisterForm(DmOpenRef formDatabase, uint16_t formId, FormEventHandlerPtr formHandlerF); 

extern Boolean CncPluginLoadForm(EventType* eventP);

extern Boolean CncPluginHandleEvent(EventType* eventP);

extern void CncPluginUnloadAllForms(void);

#ifdef __cplusplus 
}
#endif

#endif // _CNCMGR_H_
