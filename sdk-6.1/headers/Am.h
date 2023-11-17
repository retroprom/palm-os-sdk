/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Am.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Am library API definitions.
 *
 *****************************************************************************/

#ifndef _AM_H_
#define _AM_H_

#include <PalmTypes.h>
#include <CmnErrors.h>

/************************************************************
 * AM Library result codes
 *************************************************************/

#define amErrNotImplemented			(amErrorClass | 0x01)	// Function not implemented
#define amErrInvalidParam			(amErrorClass | 0x02)	// invalid parameter
#define amErrLibNotOpen	    		(amErrorClass | 0x03)	// library is not open
#define amErrLibStillOpen			(amErrorClass | 0x04)	// returned from SampleLibClose() if
														// the library is still open by others
#define amErrOutOfMemory			(amErrorClass | 0x05)	// out of memory error occurred
#define amErrMemory					(amErrorClass | 0x06)	// memory access problem
#define amErrUnsupportedTokenType	(amErrorClass | 0x07)	// Token type is not supported
#define amErrMaxTokens				(amErrorClass | 0x08)	// Out of system space for tokens
#define amErrInvalidToken			(amErrorClass | 0x09)	// Token reference is invalid
#define amErrMaxPlugins				(amErrorClass | 0x0A) // Out of systemspace for plugins
#define amErrInvalidPlugin			(amErrorClass | 0x0B)	// Plugin reference is invalid
#define amErrNotFound				(amErrorClass | 0x0C)	// whatever you're looking for was not found
#define amErrAlreadyRegistered		(amErrorClass | 0x0D)	// the plugin has already been regged.
#define amErrResourceNotFound		(amErrorClass | 0x0E)	// A resource (AM) or (PLUGIN) is missing
#define amErrUserCancel				(amErrorClass | 0x0F)	// the user cancelled the action
#define amErrAuthenticationFailed	(amErrorClass | 0x10)	// the user/am did not authenticatie properly
#define amErrNoPluginsAllowed		(amErrorClass | 0x11)	// security set to high -- no plugins allowed
#define amErrTokenDestroyed			(amErrorClass | 0x12)	// The token we're authenticating has
														// been destroyed
#define amErrTokenExists			(amErrorClass | 0x13)	// This token already exists
#define amErrBufferTooSmall			(amErrorClass | 0x14)	// passed in buffer is too small
#define amErrInvalidImportBuffer	(amErrorClass | 0x15)	// The buffer contains an invalid envelope
#define amErrActionNotSupported		(amErrorClass | 0x16) // the action required is not suopported for this token
#define amErrBackupInProgress		(amErrorClass | 0x17) // a backup of the AM storage space is in progress


/************************************************************
 * System Tokens
 *************************************************************/
#define SysUserToken                "SysUserToken"
#define SysEmptyToken               "SysEmptyToken"
#define SysLockOutToken				"SysLockOutToken"
#define SysAdminToken				"SysAdminToken"



/************************************************************
 * Public Structures
 *************************************************************/
#define amTokenTypeIdentifierLength     8
#define amTokenSystemIdLength           20
#define amTokenFriendlyNameLength       36
#define amInvalidToken                  0xFFFFFFFF

#define amPluginFriendlyNameLength      48
#define amPluginVendorLength            48

/******************************************************************************
 * AmTokenType
 *
 * Token reference
 ******************************************************************************/
typedef uint32_t AmTokenType, *AmTokenPtr;

/******************************************************************************
 * The AmPluginType is a reference to a plugin
 * An array of these can be passed in to the get references call
 * the size of the array that needs to be passed in can be gotten from GetPluginCount
 ******************************************************************************/
typedef uint32_t AmPluginType;

/*******************************************************************************
 * Definition of authentication type enum
 *
 * This type is an enumaration of the different types of authentication
 * situations.  The caller of AmAuthenticate can pass a situation, so the
 * plugin will have an idea of the type of UI to put up for the user
 *
 ******************************************************************************/
typedef enum {
	AmAuthenticationNone = 0,			/* Private -- used by AM */
	AmAuthenticationOther,				/* Other authentication */
	AmAuthenticationDataAccess,			/* DB Access */
	AmAuthenticationDeviceUnlock,		/* device is being unlocked */
	AmAuthenticationTokenModify			/* token is being modified -- ie pwd change */
} AmAuthenticationEnum;

/*******************************************************************************
 * Definition of token type enum.
 *
 * This type is a enumeration of the different types of tokens that can be
 * requested from the system
 *
 ******************************************************************************/
typedef enum {
    AmTokenUnknown = 0,
    AmTokenCustom,
	AmTokenPassword,
	AmTokenSignedCode,
	AmTokenCodeFingerprint
} AmTokenEnum;

/*******************************************************************************
 * Definition of token strength
 *
 * Token strengths are defined by this parameter, this is minimum level of
 * strength that the plugin supports for token creation
 *
 * Low - Lowes level, no requirements for token creation
 * Medium - some measures are taken to reject weak tokens
 * High - generated token should be guaranteed to not be a weak token.
 ******************************************************************************/
typedef enum {
	AmTokenStrengthLow,
	AmTokenStrengthMedium,
	AmTokenStrengthHigh
} AmTokenStrength;

/*******************************************************************************
 * Definition of token attributes
 *
 * destroy		- the token may be destroyed
 * modify		- the token may be modified (replaced)
 * internactive - the token is user interactive
 * empty		- the token is empty
 * system		- the token is a system token
 *
 ******************************************************************************/
typedef struct {
	int			destroy:1;			// set by plugin
	int			modify:1;			// set by plugin
	int			interactive:1;		// set by plugin
	int			empty:1;			// Set by plugin
	int			system:1;			// Set by AM

	int			reserved:11;
	uint16_t      padding;

	
} AmTokenAttributesType, *AmTokenAttributesPtr;

/*******************************************************************************
 * Token cache settings
 *
 * This enumeration is the different policies that can be applied to a
 * token in the system.  The app creating the token defines the cache settings.
 */
typedef enum {
	AmTokenCacheNever,
	AmTokenCacheSystem
} AmTokenCacheSettings;


/*******************************************************************************
 * Definition of token properties
 *
 * This is the structure that defines the type of token requested.  The AM
 * will find the plug-in that best matches these requirements.
 *
 *      type            - The type of token this is
 *      strength        - the strength of the token
 *      cacheSettings   - the cache settings that will govern this token.
 *      identifier      - if specified, it contains the identifier for the plug-in
 *                        that will service this token * optional * - set to 0 if not set
 *
 ******************************************************************************/
typedef struct {
	uint32_t			        identifier;
	
	AmTokenEnum			    type;           // size is uint8_t
	AmTokenStrength		    strength;       // size is uint8_t
	AmTokenCacheSettings	cacheSettings;  // size is uint8_t
    uint8_t                   rfu;
	
} AmTokenPropertiesType, *AmTokenPropertiesPtr;

/*******************************************************************************
 * Definition of token info
 *
 * So far all of this structure is in place (contigous), if that should change
 * be careful with the import and export routines, since they do not try
 * to do deep copying at this time
 *
 *	ref - the reference to this token (filled in by AM)
 *	type - the type of token this is.  (filled in by AM)
 *	attributes - flags about this token. (filled by plug-in)
 *	strength - strength of token.  (filled in by AM - copied from plug-in characteristics)
 *	cacheSettings - the token cache parameters for this token. (Set by the app at token-create time)
 *	system_id - system id of token (filled in AM)
 *	friendly_name - a string that can be used for display purposes
 *	
 *	identifier - plug-in identifier is set here (filled in by AM)
 *
 ******************************************************************************/
typedef struct {
	AmTokenType			    ref;				                    // set by AM - 32 bit
	AmTokenEnum			    type;				                    // set by AM - 8bit
	AmTokenCacheSettings	cacheSettings;		                    // Set by AM - 8bit	
	
	AmTokenStrength		    strength;			                    // set by AM - 8bit	
	uint8_t                   rfu;
	AmTokenAttributesType	attributes;			                    // set by PLUGIN - 32bit
	
	uint8_t	                systemId[amTokenSystemIdLength];		// set by AM
	char	                friendlyName[amTokenFriendlyNameLength];// set by AM

} AmTokenInfoType, *AmTokenInfoPtr;


/****************** APPLICATION CONTEXT ******************/
/*******************************************************************************
 * Definition of application context
 *
 * An application context is a data structure that is prepared by the
 * caller of of the authentication manager, it holds information about
 * the application that needs to be authentication, and other private
 * data that the plugin may be able to interpret, it is also the generic
 * method by which plugins may return data back to the caller.
 *
 *	PasswordCtxType
 *	    Password		- the clear-text password (used to verify or set - depending on call)
 *  	PasswordLength	- length of the password buffer
 *  	Hint			- hint to save in new password token
 *  	HintLength		- length of hint buffer
 *
 *	SignatureCtxType		- used when creating a token of this type
 *  	CertificateId		- ID of certificate (used during create)
 *  	CertificateidLength	- length of certificate ID buffer
 *
 * 	AppFingerprintCtxType	- used when creating a token  of this type
 *  	CreatorID		- the creatorID for the app's resource DB
 *  	TypeID			- the typeID for the app's resource DB
 *
 * 	CustomCtxType		- used for creation and verification
 *  	DataPtr			- data buffer passed into plug-in
 *  	DataLength		- length of data buffer
 *
 *
 ******************************************************************************/
typedef struct {
	
	uint32_t		processIDKey;	// Process ID key for the process being authenticated
									// If you have access to Kernel.h -- this is a placeholder
									// for a KeyID type
	
	AmTokenEnum		dataType;	    // What kind of data is being passed
	uint8_t           padding1;
	uint16_t          padding2;

	union {
		struct {
			char*	password;	    // A clear-text password
			uint32_t 	passwordLength;	// length of password buffer (must include the NULL)
			char*   hint;		    // A hint
			uint32_t  hintLength;		// length of hint buffer
		} passwordCtxType;
		
		struct {
			uint8_t*	certificateId;	    // ID of the certificate
			uint32_t	certificateIdLength;// length of the certificate id
		} signatureCtxType;

        struct {
        	uint32_t	type;			// DB Type
        	uint32_t	creator;		// DB creator
        	
        	char*	dbname;			// DB name
        	uint32_t	dbnameLength;	// Length of DB name
        } appFingerprintCtxType;
	
		struct {
			uint8_t*	dataPtr;        // custom data ptr
			uint32_t 	dataLength;     // custom data length
		} customCtxType;		
	} data;						    // data field is a union for all plug-in types
	
} AmApplicationCtxType, *AmApplicationCtxPtr;


/****************** PLUGIN INFO  ******************/
/*******************************************************************************
 * Definition of plugin info structure
 *
 * this structure is used when iterating through all plugins
 *
 ******************************************************************************/
typedef struct {
	char					friendlyName[amPluginFriendlyNameLength];
	char					vendor[amPluginVendorLength];
	uint32_t					version;
	AmTokenPropertiesType	tokenProperties;		
} AmPluginInfoType, *AmPluginInfoPtr;


/* The name under which the Authorization Manager is
   registered with the Service Manager. */
#define AmServiceName "psysAuthenticationMgr"

/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*
 * AmCreateToken
 *
 * Description:
 *      Will create a new token.  The token property parameter is used by
 *      the caller to describe the desired properties of the new token.
 *      The system will find the plug-in that best meets these requirements,
 *      but does not guarantee that all (or any) requirements will be met.
 *      Once a token has been created, the caller may invoke AmGetTokenInfo to
 *      get the properties of the created token.
 *
 * Parameters:
 *      pToken                  - pointer to a token reference, will be returned here
 *      pSystemId               - pointer to a system id buffer
 *      pFriendlyName           - pointer to a friendly name buffer
 *      pProperties             - pointer to a properties structure
 *      pAppCtx                 - pointer to a app context structure
 *		systemToken				- make this a system token (true if system token)
 *
 * Return:
 *
 *
 * Notes:
 *
 */
extern status_t AmCreateToken(
    AmTokenPtr pToken,
    char *pSystemId,
    char *pFriendlyName,
    AmTokenPropertiesPtr pProperties,
    AmApplicationCtxPtr pAppCtx,
    Boolean systemToken);

/*
 * AmDestroyToken
 *
 * Description:
 *      Will free all resources associated with specified token  This API function
 *      is called when you would like to remove the token from the system, but cannot
 *      authenticate against it prior to its deletion.  (Lost token).The AM will verify
 *      if there are other tokens protecting the destruction of this token, if there are,
 *      those tokens must be authenticated prior to destruction of the specified token.
 *      The AM will verify if the action is allowed with the plug-in, if its allowed then
 *      the token will be removed, along with all the data that it protected.
 *      Care must be taken when a plug-in allows destruction of tokens, as the deletion
 *      of certain data objects may leave the system in a non-useful state.Any application
 *      may call this function when a token is lost, even if they did not create the token.
 *      (All data protected by this token is deleted though, so there is not a security
 *      loophole), The AM will display a dialog informing the user that the destruction
 *      of the token will lead to possible data loss.When the token is destroyed, a
 *      notification is broadcast throughout the system about the token being destroyed.
 *
 * Parameters:
 *       token      - the token reference to destroy
 *       pAppCtx    - pointer to an app context structure
 *
 * Return:
 *
 *
 * Notes:
 *
 */
extern status_t AmDestroyToken(
    AmTokenType token,
    AmApplicationCtxPtr pAppCtx);


/*
 * AmModifyToken
 *
 * Description:
 *      Will modify the token.  This function allows the replacement of a token,
 *      or modification of an existing token, (depending on the plug-in).  This
 *      function is used by applications that wish to replace/modify authentication
 *      tokens.  (Even remove tokens - make tokens empty).Tokens may be protected
 *      from modification by authentication contexts.  With the help of the AZM
 *      the AM will authenticate the user prior to modification of the token.After
 *      authentication for modify, the AM will create a new token, given the
 *      pProperties passed in.  The new token will replace the old token.
 *      Applications wishing to change tokens, (change password, clear password,
 *      etc) use this function.
 *
 * Parameters:
 *      token                   - a token reference (the token to modify)
 *      pProperties             - OPTIONAL - the properties of the new token
 *      pAppCtxOld              - OPTIONAL application context for the OLD token
 *      pAppCtxNew              - OPTIONAL applicaiton context for the NEW token
 *
 * Return:
 *
 *
 * Notes:
 *
 */
extern status_t AmModifyToken(
    AmTokenType token,
    AmTokenPropertiesPtr pProperties,
    AmApplicationCtxPtr pAppCtxOld,
    AmApplicationCtxPtr pAppCtxNew);

/*
 * AmGetTokenBySystemId
 *
 * Description:
 *      Purpose	Finds a token reference given its system id.  This API allows for
 *      the creation of "well known" tokens, such as the system password, and the
 *      admin password on a device.  Applications that wish to protect data with
 *      the system password, can get a reference to it by invoking this function.
 *
 * Parameters:
 *      pToken      - a reference to the token found will be returned here
 *      pSystemId   - the system id of the token we're searching for
 *
 * Return:
 *
 *
 * Notes:
 *      there is a max size for the system id
 *
 */
extern status_t AmGetTokenBySystemId(
    AmTokenPtr pToken,
    char const * pSystemId);

/*
 * AmGetTokenInfo
 *
 * Description:
 *      Get the public info block for the referenced token.This call is used by
 *      an application after creating the token to examine the properties of
 *      the generated token.
 *
 * Parameters:
 *      token           - the token refernece to get an info block for
 *      pInfo           - the info will be returned here
 *
 * Return:
 *
 *
 * Notes:
 *
 */
extern status_t AmGetTokenInfo(
    AmTokenType token,
    AmTokenInfoPtr pInfo);

extern status_t AmGetTokenExtendedInfo(
	AmTokenType token,
	uint8_t *pExtInfo,
	uint32_t *pExtInfolen);


/*
 * AmAuthenticateToken
 *
 * Description:
 *      This function will authenticate the token that is referenced.
 *      The AM will invoke the plug-in to gather a new token, and then
 *      call the plug-in to compare the new token with the referenced token.
 *      If the plug-in deems a match, the authentication is successful.
 *
 * Parameters:
 *      token           - the token to authenticate
 *      pAppCtx         - the applicaiton ctx.  Information that can be used to authenticate the token
 *                        may be passed in by an applicaiton
 *
 * Return:
 *      errNone                     - no error authentication was successful
 *      errAmAuthenticationFailed   - error during authentication
 *      errAmInvalidReference       - invalid token reference
 *
 * Notes:
 *
 */
extern status_t AmAuthenticateToken(
    AmTokenType token,
    AmApplicationCtxPtr pAppCtx,
	AmAuthenticationEnum authType,		/* Why the AM is being called ie: Unlock Device */
	char *titleString,					/* optional string to be displayed by the plugin -- on the title of the window */
	char *descriptionString);			/* optional string to be displayed by the plugin */

/*
 * AmGetPluginReferences
 *
 * Description:
 *      This function will authenticate the token that is referenced.
 *      The AM will invoke the plug-in to gather a new token, and then
 *      call the plug-in to compare the new token with the referenced token.
 *      If the plug-in deems a match, the authentication is successful.
 *
 * Parameters:
 *      refList         - a buffer where the list of references is returned (allocated by caller)
 *      pSize           - the size of the buffer being passed in.
 *
 * Return:
 *      errAmBufferTooSmall         - the buffer passed in is too small
 *
 * Notes:
 *      if refList is NULL or if the size of the buffer is too small, errAmBufferTooSmall
 *      is returned, and pSize is set to the required size.
 *
 */
extern status_t AmGetPluginReferences(
    AmPluginType *refList,
    uint16_t *pSize);

/*
 * AmGetPluginInfo
 *
 * Description:
 *      Get the public info block for this plug-in.  This data structure contains
 *      information about the plug-in, such as vendor name,. Friendly name, and
 *      information about the type of tokens that the plug-in can create.
 *
 * Parameters:
 *      plugin      - the plugin reference to get info about
 *      pInfo       - the plugin info structure will be returned here
 *
 * Return:
 *
 * Notes:
 *
 */
extern status_t AmGetPluginInfo(
    AmPluginType plugin,
    AmPluginInfoPtr pInfo);


/*
 * AmRegisterPlugin
 *
 * Description:
 *      Will register the specified plugin in the AM.
 *      An authorization step must be satisfied prior to the registration
 *      completing successfully.  (By default that will be signed by PALM
 *      or user password.
 *
 * Parameters:
 *      creator     - the creator ID of the PRC that contains the plugin
 *      force       - force the registration, even if a plugin of this creator
 *                    has already been loaded.  This is a risky operation.
 *                    the AM will try its best to export and import all
 *                    tokens used by this plugin.
 *
 * Return:
 *      errNone                 - no error plugin loaded successfully
 *      AmErrOutOfMemory        - out of memory
 *      AmErrAlreadyRegistered  - this plugin has already been registered
 *      AmErrNotAuthorized      - authorization failed for this operation
 *
 * Notes:
 *      Will load and open the shared library that is the plugin
 *
 */
extern status_t AmRegisterPlugin(uint32_t creator, Boolean force);

/*
 * AmRemovePlugin
 *
 * Description:
 *      Will remove a previously registered plugin
 *
 * Parameters:
 *      creator     - the creator ID of the PRC that contains the plugin
 *
 * Return:
 *
 * Notes:
 *      No tokens referencing this plugin may exist
 *
 */
extern status_t AmRemovePlugin(uint32_t creator);


#ifdef __cplusplus
}
#endif


#endif // _AM_H_
