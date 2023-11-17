/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DLServer.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Desktop Link Protocol(DLP) Server Public Interface.
 *
 *****************************************************************************/

#ifndef _DL_SERVER_H_
#define _DL_SERVER_H_

// Pilot common definitions
#include <PalmTypes.h>

/************************************************************
 * DLK result codes
 * (dlkErrorClass is defined in SystemMgr.h)
 *************************************************************/
#if 0
#pragma mark *Error Codes*
#endif

#define dlkErrParam					(dlkErrorClass | 1)	// invalid parameter
#define dlkErrMemory				(dlkErrorClass | 2)	// memory allocation error
#define dlkErrNoSession				(dlkErrorClass | 3)	// could not establish a session	
#define dlkErrSizeErr				(dlkErrorClass | 4)	// reply length was too big
#define dlkErrLostConnection		(dlkErrorClass | 5)	// lost connection
#define dlkErrInterrupted			(dlkErrorClass | 6)	// sync was interrupted (see sync state)
#define dlkErrUserCan				(dlkErrorClass | 7)	// cancelled by user
#define dlkErrIncompatibleProducts	(dlkErrorClass | 8) // incompatible desktop version
#define dlkErrNPOD					(dlkErrorClass | 9) // New Password, Old Desktop


#if 0
#pragma mark *Sync state*
#endif


#define dlkMaxUserNameLength			(40)
#define dlkUserNameBufSize				(dlkMaxUserNameLength + 1)


typedef enum DlkSyncStateType {
	dlkSyncStateNeverSynced = 0,		// never synced
	dlkSyncStateInProgress,				// sync is in progress
	dlkSyncStateLostConnection,			// connection lost during sync
	dlkSyncStateLocalCan,				// cancelled by local user on handheld
	dlkSyncStateRemoteCan,				// cancelled by user from desktop
	dlkSyncStateLowMemoryOnTD,			// sync ended due to low memory on handheld
	dlkSyncStateAborted,				// sync was aborted for some other reason
	dlkSyncStateCompleted,				// sync completed normally
	// Added in PalmOS v3.0:
	dlkSyncStateIncompatibleProducts,	// sync ended because desktop HotSync product
										// is incompatible with this version
										// of the handheld HotSync
	dlkSyncStateNPOD					// New Password, Old Desktop

} DlkSyncStateType;


#if 0
#pragma mark *Function Parameter Structures*
#endif

//
// Parameter passed to DlkControl()
//
enum DlkCtlEnumTag {
	dlkCtlFirst = 0,				// reserve 0
	
	//
	// Pilot v2.0 control codes:
	//
	dlkCtlGetPCHostName,			// param1P = ptr to text buffer; (can be null if *(uint16_t *)param2P is 0)
									// param2P = ptr to buffer size(uint16_t);
									// returns actual length, including null, in *(uint16_t *)param2P which may be bigger than # of bytes copied.
									
	dlkCtlSetPCHostName,			// param1P = ptr to host name(zero-terminated) or NULL if *param2 is 0
									// param2P = ptr to length(uint16_t), including NULL (if length is 0, the current name is deleted)
	
	dlkCtlGetCondFilterTable,		// param1P =	ptr to destination buffer for filter table, or NULL if *param2 is 0
									// param2P =	on entry, ptr to size of buffer(uint16_t) (the size may be 0)
									// 				on return, size, in bytes, of the actual filter table
	
	dlkCtlSetCondFilterTable,		// param1P =	ptr to to conduit filter table, or NULL if *param2 is 0
									// param2P =	ptr to size of filter table(uint16_t) (if size is 0, the current table will be deleted)
	
	dlkCtlGetLANSync,				// param1P =	ptr to store for the LANSync setting(uint16_t): 0 = off, otherwise on
									// param2P =	not used, set to NULL
	
	dlkCtlSetLANSync,				// param1P =	ptr to the LANSync setting(uint16_t): 0 = off, otherwise on
									// param2P =	not used, set to NULL
	
	dlkCtlGetHSTCPPort,				// param1P =	ptr to store for the Desktop HotSync TCP/IP port number(uint32_t) -- zero if not set
									// param2P =	not used, set to NULL
	
	dlkCtlSetHSTCPPort,				// param1P =	ptr to the Desktop HotSync TCP/IP port number(uint32_t)
									// param2P =	not used, set to NULL
	
	dlkCtlSendCallAppReply,			// param1P =	ptr to DlkCallAppReplyParamType structure
									// param2P =	not used, set to NULL
									//
									// RETURNS: send error code; use this error code
									// as return value from the action code handler


	dlkCtlGetPCHostAddr,			// param1P = ptr to text buffer; (can be null if *(uint16_t *)param2P is 0)
									// param2P = ptr to buffer size(uint16_t);
									// returns actual length, including null, in *(uint16_t *)param2P which may be bigger than # of bytes copied.
									
	dlkCtlSetPCHostAddr,			// param1P = ptr to host address string(zero-terminated) or NULL if *param2 is 0
									// param2P = ptr to length(uint16_t), including NULL (if length is 0, the current name is deleted)


	dlkCtlGetPCHostMask,			// param1P = ptr to text buffer; (can be null if *(uint16_t *)param2P is 0)
									// param2P = ptr to buffer size(uint16_t);
									// returns actual length, including null, in *(uint16_t *)param2P which may be bigger than # of bytes copied.
									
	dlkCtlSetPCHostMask,			// param1P = ptr to subnet mask string(zero-terminated) or NULL if *param2 is 0
									// param2P = ptr to length(uint16_t), including NULL (if length is 0, the current name is deleted)
									
	dlkCtlGetPostSyncErr,			// param1P = ptr to status_t to contain any cleanup phase sync errors; 
									// param2P =	not used, set to NULL
	
	dlkCtlSetPostSyncErr,			// param1P = ptr to status_t to store any cleanup phase sync errors; 
									// param2P =	not used, set to NULL
	
	dlkCtlLAST						// *KEEP THIS ENTRY LAST*
	
};
typedef Enum8 DlkCtlEnum;


//
// Parameter passed with DlkControl()'s dlkCtlSendCallAppReply code
//
typedef struct DlkCallAppReplyParamType {
	uint16_t				pbSize;			// size of this parameter block (set to sizeof(DlkCallAppReplyParamType))
	uint16_t				padding;		// added to remove warning
	uint32_t				dwResultCode;	// result code to be returned to remote caller
	const void *		resultP;		// ptr to result data
	uint32_t				dwResultSize;	// size of reply data in number of bytes
	void *				dlRefP;			// DesktopLink reference pointer from
										// SysAppLaunchCmdHandleSyncCallAppType
	uint32_t				dwReserved1;	// RESERVED -- set to null!!!
} DlkCallAppReplyParamType;


/********************************************************************
 * DesktopLink Server Routines
 ********************************************************************/
#if 0
#pragma mark *Function Prototypes*
#endif


#ifdef __cplusplus
extern "C" {
#endif

status_t	
DlkGetSyncInfo(uint32_t* succSyncDateP, uint32_t* lastSyncDateP, DlkSyncStateType* syncStateP,
			   char* nameBufP, char* logBufP, int32_t* logLenP);

extern
void	
DlkSetLogEntry(const char* textP, int16_t textLen, Boolean append);


extern
status_t
DlkControl(DlkCtlEnum op, void* param1P, void* param2P);

#ifdef __cplusplus
}
#endif

#endif //_DL_SERVER_H_
