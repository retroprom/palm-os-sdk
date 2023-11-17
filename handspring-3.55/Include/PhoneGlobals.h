/*******************************************************************
 *
 * Copyright (c) Handspring, Inc 2000-2002 -- All Rights Reserved
 *
 * Project:
 *	  Handspring Springboard Phone
 *
 * FileName:
 *	  PhoneGlobals.h
 *
 * Description:
 *	  Public header file for globals shared by Phone module applications.
 *
 ***************************************************************/

#ifndef   __PHONEGLOBALS_H__
#define   __PHONEGLOBALS_H__

#include	<PhoneLib.h>

#define phoneGlobalsFeature		0x10

//**********************************************************************
//  Phone state globals structure.
//
//    The structure contains phone state infomation that is shared 
//    between all phone application.  A pointer to this structure 
//    is saved to a system feature.  The feature has a creator of:
//    hsFileTCardSetup and a feature number of: phoneGlobalsFeature
//**********************************************************************
typedef struct
  {
	Boolean			syncing;		// true if hotsync is running
	Boolean			activeCalls;	// number of active voice call
  }
PhoneGlobalsType, * PhoneGlobalsPtr;


#endif	// __PHONEGLOBALS_H__