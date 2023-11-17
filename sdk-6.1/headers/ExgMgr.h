/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ExgMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Include file for Exg system functions
 *
 *****************************************************************************/

#ifndef __EXGMGR_H__
#define __EXGMGR_H__

#include <PalmTypes.h>

//#include <ErrorBase.h>
#include <CmnErrors.h>
#include <DataMgr.h>
#include <SchemaDatabases.h>
#include <Rect.h>

#define exgMemError	 			(exgErrorClass | 1)
#define exgErrStackInit 		(exgErrorClass | 2)  // stack could not initialize
#define exgErrUserCancel 		(exgErrorClass | 3)
#define exgErrNoReceiver 		(exgErrorClass | 4)	// receiver device not found
#define exgErrNoKnownTarget		(exgErrorClass | 5)	// can't find a target app
#define exgErrTargetMissing		(exgErrorClass | 6)  // target app is known but missing
#define exgErrNotAllowed		(exgErrorClass | 7)  // operation not allowed
#define exgErrBadData			(exgErrorClass | 8)  // internal data was not valid
#define exgErrAppError			(exgErrorClass | 9)  // generic application error
#define exgErrUnknown			(exgErrorClass | 10) // unknown general error
#define exgErrDeviceFull		(exgErrorClass | 11) // device is full
#define exgErrDisconnected		(exgErrorClass | 12) // link disconnected
#define exgErrNotFound			(exgErrorClass | 13) // requested object not found
#define exgErrBadParam			(exgErrorClass | 14) // bad parameter to call
#define exgErrNotSupported		(exgErrorClass | 15) // operation not supported by this library
#define exgErrDeviceBusy		(exgErrorClass | 16) // device is busy
#define exgErrBadLibrary		(exgErrorClass | 17) // bad or missing ExgLibrary
#define exgErrNotEnoughPower	(exgErrorClass | 18) // Device has not enough power to perform the requested operation
#define exgErrNoHardware		(exgErrorClass | 19) // Device does has not have corresponding hardware
#define exgErrAuthRequired		(exgErrorClass | 20) // Need authentication - username/passwd
#define exgErrRedirect			(exgErrorClass | 21) // Redirected url
#define exgErrLibError			(exgErrorClass | 22) // Library specific error - usually the library can provide more details
#define exgErrConnection		(exgErrorClass | 23) // There was an error in making the connection
#define exgErrInvalidURL		(exgErrorClass | 24) // Bad URL
#define exgErrInvalidScheme		(exgErrorClass | 25) // Bad scheme in URL

#define exgSeparatorChar		'\t'				// char used to separate multiple registry entries

// Exchange Registry ID codes

#define exgRegCreatorID			0xfffb				// creator ID registry
#define exgRegSchemeID			0xfffc				// URL scheme registry
#define exgRegExtensionID		0xfffd				// filename extension registry
#define exgRegTypeID			0xfffe				// MIME type registry

// send data directly to target application (instead of inbox) replaces the UnWrap flag.
#define exgRegDirectCreatorID	0xffeb				// creator ID registry
#define exgRegDirectExtensionID	0xffed				// filename extension registry
#define exgRegDirectTypeID		0xffee				// MIME type registry

// used for viewer applications
#define exgRegViewCreatorID		0xff8b				// creator ID registry
#define exgRegViewExtensionID	0xff8d				// filename extension registry
#define exgRegViewTypeID		0xff8e				// MIME type registry

// used for printing services
#define exgRegPrintCreatorID	0xff9b				// creator ID registry
#define exgRegPrintExtensionID	0xff9d				// filename extension registry
#define exgRegPrintTypeID		0xff9e				// MIME type registry


#define exgDataPrefVersion		0					// all isDefault bits clear
#define exgTitleBufferSize		20					// buffer size for title from exgLibCtlGetTitle, including null terminator
#define exgMaxTitleLen			exgTitleBufferSize	// deprecated
#define exgMaxTypeLength		80					// max length of extensions, MIME types, and URL schemes in registry, excluding null terminator
#define exgMaxDescriptionLength	80					// max length of descriptions in registry, excluding null terminator

#define exgLibCtlGetTitle		1					// get title for Exg dialogs
#define exgLibCtlGetVersion		2					// get version of exg lib API
#define exgLibCtlGetPreview		3					// find out if library supports preview
#define exgLibCtlGetURL			4					// get current URL
#define exgLibCtlSocketRefIn    5					// convert SocketRef from 68K
#define exgLibCtlSocketRefOut	6					// Convert socketRef to 68K

#define exgLibCtlSpecificOp		0x8000				// start of range for library specific control codes

#define exgLibAPIVersion		0					// current version of exg lib API

// Pre-defined URL schemes
#define exgBeamScheme			"_beam"		// general scheme for Beam commands
#define exgSendScheme			"_send"		// general scheme for Send commands
#define exgGetScheme			"_get"      // special scheme for supporting get

// Pre-defined URL prefixes
#define exgBeamPrefix			exgBeamScheme ":"
#define exgSendPrefix			"?" exgSendScheme ":"
#define exgSendBeamPrefix		"?" exgSendScheme ";" exgBeamScheme ":"
#define exgLocalPrefix			exgLocalScheme ":"
#define exgGetPrefix			exgGetScheme ":"

// A flag used for attachments.
#define exgUnwrap				0x0001 
// A flag used for skipping the sysAppLaunchCmdExgAskUser sublaunch and the subsequent
// call to ExgDoDialog.
#define exgNoAsk				0x0002
#define exgGet					0x0004 
//--------------------------------------------------------------------------------------

// Enum for preview operations. Also used as masks for the query operation.
#define exgPreviewQuery			((uint16_t)0x0000)
#define exgPreviewShortString	((uint16_t)0x0001)
#define exgPreviewLongString	((uint16_t)0x0002)
#define exgPreviewDraw			((uint16_t)0x0004)
#define exgPreviewDialog		((uint16_t)0x0008)
#define exgPreviewFirstUser		((uint16_t)0x0400)		// used for app-specific operations
#define exgPreviewLastUser		((uint16_t)0x8000)		//


// special exchange mgr event key
#define exgIntDataChr			0x01ff

// refNum values above this number are stored in the exchange mgr plug-in table
// Values below this number are 68K library ref numbers
// In reality the 68K lib refNum range will probably be something like 0 to 20 (or so)
#define maxExg68KLibRefNum		0x7FFF

// The offset number used to convert exchange mgr plug-in table indices to 'fake' lib ref nums
// Not using 0xFFFF since it is reserved for invalidRefNum
#define maxExgNativeLibRefNum	0xFFFE


/******************************************************************************
 *  Exg Plugin API
 ******************************************************************************/

typedef struct ExgSocketTag * ExgSocketPtr;

// Plugin entry functions prototypes
typedef status_t (*ExgPluginOpenProcPtr)();
typedef status_t (*ExgPluginCloseProcPtr)();
typedef status_t (*ExgPluginSleepProcPtr)();
typedef status_t (*ExgPluginWakeProcPtr)();

typedef Boolean (*ExgPluginHandleEventProcPtr)(void *);
typedef status_t (*ExgPluginAcceptProcPtr)(ExgSocketPtr );
typedef status_t (*ExgPluginConnectProcPtr)(ExgSocketPtr );
typedef status_t (*ExgPluginDisconnectProcPtr)(ExgSocketPtr , status_t );
typedef status_t (*ExgPluginPutProcPtr)(ExgSocketPtr );
typedef status_t (*ExgPluginGetProcPtr)(ExgSocketPtr );
typedef uint32_t (*ExgPluginSendProcPtr)(ExgSocketPtr , 
						const void * const , uint32_t , status_t *);
typedef uint32_t (*ExgPluginReceiveProcPtr)(ExgSocketPtr , void *, uint32_t , status_t *);
// This entry accepts data with native ordering
typedef status_t (*ExgPluginControlProcPtr)(uint16_t , void *, uint16_t *);
typedef status_t (*ExgPluginRequestProcPtr)(ExgSocketPtr);
// This entry accepts data with 68K ordering and alignment
typedef status_t (*ExgPluginControl68KProcPtr)(uint16_t , void *, uint16_t *);

typedef struct ExgMgrPluginAPITag {
	ExgPluginOpenProcPtr		exgPluginOpen;
	ExgPluginCloseProcPtr		exgPluginClose;
	ExgPluginSleepProcPtr		exgPluginSleep;
	ExgPluginWakeProcPtr		exgPluginWake;

	ExgPluginHandleEventProcPtr	exgPluginHandleEvent;
	ExgPluginAcceptProcPtr		exgPluginAccept;
	ExgPluginConnectProcPtr		exgPluginConnect;
	ExgPluginDisconnectProcPtr	exgPluginDisconnect;
	ExgPluginPutProcPtr			exgPluginPut;
	ExgPluginGetProcPtr			exgPluginGet;
	ExgPluginSendProcPtr		exgPluginSend;
	ExgPluginReceiveProcPtr		exgPluginReceive;
	ExgPluginControlProcPtr		exgPluginControl;
	ExgPluginRequestProcPtr		exgPluginRequest;
	ExgPluginControl68KProcPtr	exgPluginControl68K;
} ExgMgrPluginAPIType;

typedef ExgMgrPluginAPIType* ExgMgrPluginAPIPtr;

// Plugin entry point function
typedef status_t (*PluginEntryPointProcPtr)( ExgMgrPluginAPIPtr pluginFunctsP );


/******************************************************************************
 *  ExgMgr typedefs
 ******************************************************************************/


typedef struct {
	DatabaseID	dbID;				// databaseID
	uint32_t 	recordNum;			// index of record that contain a match
	uint32_t	uniqueID;			// postion in record of the match.
	uint32_t	matchCustom;		// application specific info
} ExgGoToType;	

typedef ExgGoToType *ExgGoToPtr;


typedef struct ExgSocketTag{
	uint16_t	libraryRef;		// Used to identify the exchange library
	uint16_t	socketRefSize;	// size of data block referenced in socketRef
	uint32_t 	socketRef;		// used by Exg library to identify this connection
	uint32_t 	target;			// Creator ID of application this is sent to
	uint32_t	count;			// # of objects in this connection (usually 1)
	uint32_t	length;			// # total byte count for all objects being sent (optional)
	uint32_t	time;			// last modified time of object (optional)
	uint32_t	appData;		// application specific info
	uint32_t 	goToCreator;	// creator ID of app to launch with goto after receive
	ExgGoToType goToParams;	// If launchCreator then this contains goto find info
	uint16_t	localMode:1;	// Exchange with local machine only mode 
	uint16_t	packetMode:1;	// Use connectionless packet mode (Ultra)
	uint16_t	noGoTo:1;		// Do not go to app (local mode only)
	uint16_t 	noStatus:1;		// Do not display status dialogs
	uint16_t 	preview:1;		// Preview in progress: don't throw away data as it's read
	uint16_t	cnvFrom68KApp:1;// This socket has been originaly created by a 68K application
	uint16_t	acceptedSocket:1;// set to non zero if this socket was passed into ExgAccept
	uint16_t	reserved:9;		// reserved system flags
	uint8_t   componentIndex; // ExgMgr C++ component holding this Socket (if any)
	uint8_t	padding_1;
	char 	*description;	// text description of object (for user)
	char 	*type;			// Mime type of object (optional)
	char 	*name;			// name of object, generally a file name (optional)
} ExgSocketType;


// structures used for sysAppLaunchCmdExgAskUser launch code parameter
// default is exgAskDialog (ask user with dialog...
typedef enum { exgAskDialog,exgAskOk,exgAskCancel } ExgAskResultType;

typedef struct {
	ExgSocketPtr		socketP;
	ExgAskResultType	result;			// what to do with dialog	
	uint8_t 				reserved;
	uint16_t				padding_1;
} ExgAskParamType;	
typedef ExgAskParamType *ExgAskParamPtr;

// Optional parameter structure used with ExgDoDialog for category control
typedef struct {
	uint16_t			version;		// version of this structure (should be zero)
	uint16_t			padding_1;
	DmOpenRef		db;				// open database ref (for category information)
	CategoryID		categoryIndex;	// index of selected category
} ExgDialogInfoType;

typedef struct {
	uint16_t version;
	uint16_t	padding1;
	ExgSocketType *socketP;
	uint16_t op;
	uint16_t	padding2;
	char *string;
	uint32_t size;
	RectangleType bounds;
	uint16_t types;
	uint16_t padding3;
	status_t error;
} ExgPreviewInfoType;

// used with exgLibCtlGetURL
typedef struct ExgCtlGetURLType {
	ExgSocketType	*socketP;
	char			*URLP;
	uint16_t		URLSize;
	uint16_t		padding;
} ExgCtlGetURLType;

typedef status_t	(*ExgDBReadProcPtr) 
				(void *dataP, uint32_t *sizeP, void *userDataP);

typedef Boolean	(*ExgDBDeleteProcPtr)
				(const char *nameP, uint16_t version, 
				DatabaseID dbID, void *userDataP);

typedef status_t	(*ExgDBWriteProcPtr)
				(const void *dataP, uint32_t *sizeP, void *userDataP);

#ifdef __cplusplus
extern "C" {
#endif

status_t ExgInit(void);

status_t ExgConnect(ExgSocketPtr socketP);

status_t ExgPut(ExgSocketPtr socketP);

status_t ExgGet(ExgSocketPtr socketP);

status_t ExgAccept(ExgSocketPtr socketP);

status_t ExgDisconnect(ExgSocketPtr socketP,status_t error);

uint32_t ExgSend(ExgSocketPtr socketP, const void * const bufP, uint32_t bufLen, status_t *err);

uint32_t ExgReceive(ExgSocketPtr socketP, void *bufP, uint32_t bufLen, status_t *err);

status_t ExgControl(ExgSocketType *socketP, uint16_t op, void *valueP, uint16_t *valueLenP);

status_t ExgRegisterData(uint32_t creatorID, uint16_t id, const char *dataTypesP);

status_t ExgRegisterDatatype(uint32_t creatorID, uint16_t id, const char *dataTypesP,
	const char *descriptionsP, uint16_t flags);

status_t ExgNotifyReceive(ExgSocketType *socketP, uint16_t flags);

status_t ExgNotifyGoto(ExgSocketType *socketP, uint16_t flags);

status_t	ExgDBRead(
		ExgDBReadProcPtr		readProcP,
		ExgDBDeleteProcPtr		deleteProcP,
		void*					userDataP,
		DatabaseID*				dbIDP,
		Boolean*				needResetP,
		Boolean					keepDates);

status_t	ExgDBWrite(
		ExgDBWriteProcPtr		writeProcP,
		void*					userDataP,
		const char*				nameP,
		DatabaseID				dbID);

Boolean ExgDoDialog(ExgSocketType *socketP, ExgDialogInfoType *infoP, status_t *errP);

status_t ExgRequest(ExgSocketType *socketP);

status_t ExgSetDefaultApplication(uint32_t creatorID, uint16_t id, const char *dataTypeP);

status_t ExgGetDefaultApplication(uint32_t *creatorIDP, uint16_t id, const char *dataTypeP);

status_t ExgGetTargetApplication(ExgSocketType *socketP, Boolean unwrap, uint32_t *creatorIDP, char *descriptionP, uint32_t descriptionSize);

status_t ExgGetRegisteredApplications(uint32_t **creatorIDsP, uint32_t *numAppsP, char **namesP, char **descriptionsP, uint16_t id, const char *dataTypeP);

status_t ExgGetRegisteredTypes(char **dataTypesP, uint32_t *sizeP, uint16_t id);

status_t ExgNotifyPreview(ExgPreviewInfoType *infoP);


#ifdef __cplusplus 
}
#endif

#endif  // __EXGMGR_H__
