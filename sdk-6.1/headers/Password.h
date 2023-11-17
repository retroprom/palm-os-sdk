/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Password.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Password include file
 *
 *****************************************************************************/

#ifndef __PASSWORD_H__
#define __PASSWORD_H__

// Include elementary types
#include <PalmTypes.h>					// Basic types

#define pwdLength						32

// #define pwdEncryptionKeyLength			64	// XXX - TO BE REMOVED - XXX

#ifdef __cplusplus
extern "C" {
#endif

Boolean 	PwdExists(void);
				
Boolean 	PwdVerify(char *string);
				
void 		PwdSet(char *oldPassword, char *newPassword);
				
void 		PwdRemove(void);
				
#ifdef __cplusplus 
}
#endif
				
#endif // __PASSWORD_H__
