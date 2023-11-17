/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Azm.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Azm library API definitions.  The Azm library serves as an example
 *		of creating Palm OS shared libraries.
 *
 *****************************************************************************/

#ifndef _AZM_H_
#define _AZM_H_

#include <PalmTypes.h>
#include <Am.h>

/************************************************************
 * Azm Library result codes
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 *************************************************************/

#define azmErrBadParam			    (azmErrorClass | 1)		// invalid parameter
#define azmErrNotImplemented		(azmErrorClass | 2)
#define azmErrNotOpen				(azmErrorClass | 3)		// library is not open
#define azmErrStillOpen				(azmErrorClass | 4)		// returned from SampleLibClose() if
															// the library is still open by others
#define azmErrMaxRuleSets			(azmErrorClass | 5)		// Maximum number of rule-sets in system
#define azmErrMemory				(azmErrorClass | 6)		// memory error occurred
#define azmErrOutOfMemory           (azmErrorClass | 7)     // Out of memory
#define azmErrInvalidReference		(azmErrorClass | 8)		// invalid rule-set reference
#define azmErrAuthorizationFailed	(azmErrorClass | 9)		// Authorization Denied
#define azmErrMgrNotRegistered      (azmErrorClass | 10)    // Caller is not a registered MGR
#define azmErrRestrictedAPI         (azmErrorClass | 11)    // usedgeneral key to invoke a protected API
#define azmErrNotFound              (azmErrorClass | 12)    // The rule-set was not found
#define azmErrMgrAlreadyRegistered  (azmErrorClass | 13)    // This manager has already registered
#define azmErrTooManyTokensInRule	(azmErrorClass | 14)	// too many tokens in rule
#define azmErrInvalidTokenReference (azmErrorClass | 15)	// invalid token ref
#define azmErrInvalidRuleSyntax		(azmErrorClass | 16)	// Invalid rule syntax (addRule)
#define azmErrBackupInProgress		(azmErrorClass | 17)	// a backup is in progress
#define azmErrInvalidParameter		(azmErrorClass | 18)	// invalid parameter passed in
#define azmErrAlreadyExists			(azmErrorClass | 19)	// ruleset already exists

/********************************************************************
 * Public Structures
 ********************************************************************/
#define azmCreator							'azm_'
#define azmRuleSetNameMaxLength    			40
#define azmMaxTokenNodes                    2
#define azmMaxTokensInNode                  8
#define azmMaxTokensInTree                  azmMaxTokenNodes * azmMaxTokensInNode

#define azmInvalidRuleSet					0xFFFFFFFF
#define azmSyncRuleSet                      0x00800000

/** @}
 *
 * Action type
 *
 * This type defines the length of the action bitmap
 *
 */
typedef uint32_t AzmActionType;

/** @typedef AzmRuleSetType
 *
 * An opaque handle to an rule-set container managed by the AZM.
 */
typedef uint32_t AzmRuleSetType;

/* The name under which the Authorization Manager is
   registered with the Service Manager. */
#define AzmServiceName "psysAuthorizationMgr"

/********************************************************************
 * Notification Callbacks
 *
 *
 * When a token is destroyed, the rule-sets may be left in an invalid
 * state.  The authentication manager sends out a general notification
 * of token destruction to the system, this notification is caught by
 * the authentication manager.
 *
 * When this notification is processed, the AZM will examine all its
 * rules-sets for instances of the destroyed token.  Rules that contain
 * the destroyed token will be re-analyzed; if they are invalid, they
 * are removed from the rule-set.  If the rule-set is void of rules,
 * then the rule-set will be destroyed, and a notification is sent to
 * the manager that created the rule-set.  The request key (call-back
 * key) to which that notification is set was registered when the
 * manager registered with the AZM with the call AzmRegisterManager.
 *
 * The callback will be done by calling the routine KALMsgSend, which
 * means that the AZM will not wait for a reply.
 *
 *
 * where:
 *	version		- is set to 1
 *
 *	ruleSetDestroyed	- is the structure used during ruleSetDestroyed
 *                        calls
 *		name		- name of the rule set that was destroyed
 *		length		- length of the name
 *
 * The Messages that will be sent to the callback-key is defined as
 * follows: -- these notifications are sent using the KalMsgSend,
 * no reply is expected.  One call per destroyed rule-set is made.
 *
 * iSendData0		- set to AzmNotificationRuleSetDestroyed
 * iSendData1		- set to 0
 * iSendData2		- set to 0
 *
 * iExtInfoP		-
 *  ->sendKey0	- set to kKALKeyIDNull
 *  ->sendKey1	- set to kKALKeyIDNull
 *  ->sendKey2	- set to kKALKeyIDNull
 *	->bytesToSend	- set to sizeof(AzmNotifyType)
 *	->sendBufferP	- set to a pointer to a AzmNotifyType structure
 *	->sendTimeOut	- some reasonable number (>100ms, but <500ms)
 *
 *********************************************************************/

/* Notification Callback Opcodes */
#define		AzmNotificationRuleSetDestroyed			0x1

/* Notification Callback Type */
typedef struct {
	union {
		struct {
			uint8_t	    name [azmRuleSetNameMaxLength];
			uint32_t		length;
		} ruleSetDestroyed;
	} data;
	
	uint16_t		version;
	uint16_t		padding;
	
} AzmNotificationType;


/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Azm Library
 */


/*
 * AzmAddRule (AzmMgrAddRule)
 *
 * Description:
 *
 *
 * Parameters:
 *
 *
 * Return:
 *
 *
 * Notes:
 *
 *
 */
extern status_t AzmAddRule(
    AzmRuleSetType ruleset,
    AzmActionType action,
    char* rulefmt, ...);


/*
 * AzmCreateRuleSet
 *
 * Description:
 *
 *
 * Parameters:
 *
 *
 * Return:
 *
 */
extern status_t AzmNonInteractiveAuthorize(
    AzmRuleSetType ruleSet, 
    AzmActionType action,
    uint32_t appIdentityKeyId);			/* Placeholder for a KeyID type, defined in Kernel.h */
										/* If you dont have access to kernel.h -- just insert 0 here */


/*
 * AzmSetSyncBypass
 *
 * Description:
 *      Sets the bypass state for a sync operation on the protected object
 *
 * Parameters:
 *      ruleset     - reference to a rule-set
 *      state       - state of sync bypass (true for sync-allow)
 *
 * Return:
 *      errNone                 - no error
 *      AzmErrInvalidReference  - reference in parameters is invalid
 *
 * Notes:
 *
 */
extern status_t AzmSetSyncBypass(
    AzmRuleSetType ruleset,
    AzmActionType action,
    Boolean state);

/*
 * AzmGetSyncBypass
 *
 * Description:
 *      Gets the bypass state for a sync operation on the protected object
 *
 * Parameters:
 *      ruleset     - reference to a rule-set
 *      state       - returns the state of sync bypass (true for sync-allow)
 *
 * Return:
 *      errNone                 - no error
 *      AzmErrInvalidReference  - reference in parameters is invalid
 *
 * Notes:
 *
 */
extern status_t AzmGetSyncBypass(
    AzmRuleSetType ruleset,
    uint32_t *statebitfield);



#ifdef __cplusplus
}
#endif


/********************************************************************
 * Public Macros
 ********************************************************************/
/** @defgroup Actions Predefined Actions
 *
 * Actions, for the most part, are totally opaque to the AZM. The
 * manager protecting the object is the only one aware of what the
 * actions actually mean. To not confuse the developer we manufacture
 * some arbitrary but useful meaning to the first couple of bits.
 *
 * The ACTION_MODIFY action is known explicitly to the AZM and is used
 * to associate an authorization context with the ability to modify
 * the rule-set container. &quot;Modify&quot; implies the ability to add
 * ACE entries and delete the rule-set container.
 *
 * @{
 */

/** @def ACTION_MODIFY
 * Predefined MODIFY action. This action may NOT be redefined.
 *
 * This is an AZM specific action which gates the modification of an
 * rule-set container. A modification of an rule-set container is defined as
 * creation (always allowed), addition or modification of ACE entries,
 * or destruction.
 */
#define azmActionModify       	0x80000000


#endif	// _AZM_H_
