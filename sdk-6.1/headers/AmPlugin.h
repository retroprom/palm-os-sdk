/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AmPlugin.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *	Types for AmPlugins
 *
 *****************************************************************************/

#ifndef _AM_PLUGIN_H_
#define _AM_PLUGIN_H_

#include <Am.h>

/*******************************************************************************
 * Defintion of AmCallMode
 *
 * Used when calling the entry points, to distinguish between calls made when
 * generating new credentials, verifying credentials
 *
 * verification - this call mode is used when we're verifying credentials.
 * replacement - this mode is used when a credential is being modified
 *				  there are 2 stages.  (start) is used first, and in this stage
 *				  we must authenticate whoever (user) or (app) is tring to modify
 *				  the token
 *				  (end) is used last, to create the new replacement credentials.
 * Enrollment	- this call mode is used to create new credentials to be stored
 *				  for comparison later.
 * Destruction	- token is lost and will be destroyed.  (All data protected by
 *				  this token will be removed if successful.
 *
 *
 ******************************************************************************/
typedef enum {
	AmEnrollment = 0,		// Used when creating new credentials
	AmVerification, 		// Used during verification of credentials
	AmReplacementStart,		// 1st phase of replacing a token
	AmReplacementEnd,		// 2nd phase of replacing a token
} AmCallMode;

/*
 * Dynamic Data Cell
 */
typedef struct _AmTokenDataTag
{
	uint32_t	recordId;

	struct _AmTokenDataTag	*next;
} AmTokenDataType, *AmTokenDataPtr;

/*******************************************************************************
 * Definition of authentication tokens
 *
 * An authentication token is an opaque data structure that holds information
 * regarding the credentials that must be matched for a valid authentication
 * to occur.
 * The creation and management of these tokens is left up to the
 * authentication plugin.
 *
 * header			= public info common to tall tokens
 * pluginCreatorId	= the creator id of the plugin that created this token
 * tokenRef			= this token's reference id
 * dataPtr			= Ptr to the data portion for this token
 * data_lenghth		= length of data
 ******************************************************************************/
typedef struct {
	AmTokenInfoType			header;

	uint32_t				pluginCreatorId;
	uint32_t				tokenRecId;

	MemPtr					dataPtr;
	uint32_t				dataLength;

	AmTokenDataPtr			dynamicData;
} AmTokenPrivType, *AmTokenPrivPtr;



/*
 * Definition of plugin functions
 */
typedef struct {

    status_t (*pluginCaptureFtn)(AmCallMode, AmApplicationCtxType *, AmTokenPrivType *, AmAuthenticationEnum, char *, char *);
    status_t (*pluginMatchFtn)(AmApplicationCtxType *, AmTokenPrivType *, AmTokenPrivType *);
    status_t (*pluginDestroyNotifyFtn)(AmTokenPrivType *);
	status_t (*pluginGetTokenExtendedInfoFtn)(AmTokenPrivType *, uint8_t *, uint32_t);
    status_t (*pluginImportTokenFtn)(AmTokenPrivType *, uint8_t *, uint32_t);
    status_t (*pluginExportTokenFtn)(AmTokenPrivType *, uint8_t *, uint32_t *);
    status_t (*pluginGetDerivedData)(AmTokenPrivType *, uint8_t *, uint32_t *);
    status_t (*pluginAdminFtn)(AmPluginType *);

} AmPluginFunctionsType;


/* Plugin Structure */
typedef struct  {
	AmPluginType      		pluginRef;
	uint32_t				recordId;

	AmPluginInfoType		info;
	uint32_t				tokenDataLength;
	uint32_t				tokenExtendedInfoLength;

	AmPluginFunctionsType	ftn;

} AmPluginPrivType, *AmPluginPrivPtr;


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions that can be invoked by the plugins
 */

/*
 * UI Setup and teardown
 *
 * The AM Needs to keep track of each plugin that is using the UI framework
 * each plugin MUST calls these functions before it displays a form
 * and after it completes that display
 *
 * A call into initializeUIContext may block if another plugin is currently
 * using the UI framework.
 *
 * AM plugins can only do UI one-at-a-time
 *
 */


status_t AmInitializeUIContext(void);
status_t AmReleaseUIContext(void);

/*
 * Memory Proxying Functions by the AM
 *
 * All dynamic memory a plugin uses, that is associated with a token
 * should be done through these methods
 */
typedef uint32_t AmMemHandle;

AmMemHandle AmMemHandleNew(AmTokenPrivType *pPrivToken, uint32_t size);
void AmMemHandleFree(AmTokenPrivType *pPrivToken, AmMemHandle hMem);

MemPtr AmMemHandleLock(AmMemHandle hMem);
void AmMemHandleUnlock(AmMemHandle hMem, MemPtr pMem);

#ifdef __cplusplus
}
#endif

#endif /* _AM_PLUGIN_H_ */
