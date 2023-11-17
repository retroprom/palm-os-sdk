/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: IdLookupLib.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#include <StringMgr.h>
#include <TextMgr.h>
#include <SysEventCompatibility.h>
#include <DataMgr.h>
#include <SystemResources.h>
#include <AppMgr.h>
#include <CmnLaunchCodes.h>
#include <SchemaDatabases.h>

#include "IdLookupLib.h"

#include "AddressIdLookup.h"


/***********************************************************************
 *
 * FUNCTION:    prvIdLookupLibSearch
 *
 * DESCRIPTION: Perform the Phone Number search and update the struct.
 *
 * PARAMETERS:  addressIdLookupStructP	- Address Search structure.
 *				 Contains all parameters to perform the Address id Lookup
 *				 search.
 *
 * RETURNED:    errNone, if no error.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		01/20/03	Initial revision
 *
 ***********************************************************************/
static status_t prvIdLookupLibSearch(AddressIdLookupLibSearchType * addressIdLookupStructP)
{
	status_t			err = errNone;
	uint32_t			sublaunchResult = NULL;
	DatabaseID			dbId;
	
	dbId = DmFindDatabaseByTypeCreator(sysFileTApplication, sysFileCAddress, dmFindExtendedDB, NULL);
	if (dbId == NULL)
		err = DmGetLastErr();
	else
		err = SysAppLaunch(dbId, AddressBookIdLookupLaunch, (uint32_t*)addressIdLookupStructP, &sublaunchResult);
				
	return err;
}


/******************************************************************************
 *
 * FUNCTION: IdLookupLibMain
 *
 * DESCRIPTION: 
 *	Main entry point of the IdLookupLib library shared library.
 *
 * CALLED BY:
 *	Low-level process startup code.
 *
 * PARAMETERS: 
 *	cmd			 -> launch command, can be one of the following:
 *					sysAppLaunchCmdNormalLaunch - during process startup.
 *					sysLaunchCmdInitialize - right after this module is loaded
 *					sysLaunchCmdFinalize - right after this module is unloaded
 *	cmdPBP		 -> when cmd is sysAppLaunchCmdNormalLaunch, this
 *					parameter is the start code specified by ApplicationMgr 
 *					when it started this thread.
 * launchFlags	 -> 
 *`
 * RETURNS:
 *	errNone, or system error code.
 *****************************************************************************/
uint32_t IdLookupLibMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    IdLookupLibSearchPhoneNumber
 *
 * DESCRIPTION: Perform the Phone Number search and update the struct.
 *
 * PARAMETERS:  searchPhoneNumberP	- Search structure. Contains all parameters
 *							to perform the Phone Number search.
 *
 * RETURNED:    errNone, if no error.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		01/20/03	Initial revision
 *
 ***********************************************************************/
status_t IdLookupLibSearchPhoneNumber(IdLookupLibSearchPhoneNumberType * searchPhoneNumberP)
{
	status_t						err = errNone;
	AddressIdLookupLibSearchType 	addressIdLookupStruct;

	if (!searchPhoneNumberP)
		return err;
	
	addressIdLookupStruct.addressIdSearchType = kIdLookupLibSearchTypePhoneNumber;
	addressIdLookupStruct.addressIdLookupLibSearchTypeP = searchPhoneNumberP;

	err = prvIdLookupLibSearch(&addressIdLookupStruct);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    IdLookupLibSearchEmail
 *
 * DESCRIPTION: Perform the Email search and update the struct.
 *
 * PARAMETERS:  searchEmailP	- Search structure. Contains all parameters
 *							to perform the Email search.
 *
 * RETURNED:    errNone, if no error.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		01/20/03	Initial revision
 *
 ***********************************************************************/
status_t IdLookupLibSearchEmail(IdLookupLibSearchEmailType * searchEmailP)
{
	status_t						err = errNone;
	AddressIdLookupLibSearchType 	addressIdLookupStruct;

	if (!searchEmailP)
		return err;
	
	addressIdLookupStruct.addressIdSearchType = kIdLookupLibSearchTypeEmail;
	addressIdLookupStruct.addressIdLookupLibSearchTypeP = searchEmailP;

	err = prvIdLookupLibSearch(&addressIdLookupStruct);
	
	return err;
}


/***********************************************************************
 *
 * FUNCTION:    IdLookupLibSearchUrl
 *
 * DESCRIPTION: Perform the url search and update the struct.
 *
 * PARAMETERS:  searchUrlP	- Search structure. Contains all parameters
 *						to perform the url search.
 *
 * RETURNED:    errNone, if no error.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * CGu		01/20/03	Initial revision
 *
 ***********************************************************************/
status_t IdLookupLibSearchUrl(IdLookupLibSearchUrlType * searchUrlP)
{
	status_t						err = errNone;
	AddressIdLookupLibSearchType 	addressIdLookupStruct;

	if (!searchUrlP)
		return err;
	
	addressIdLookupStruct.addressIdSearchType = kIdLookupLibSearchTypeUrl;
	addressIdLookupStruct.addressIdLookupLibSearchTypeP = searchUrlP;

	err = prvIdLookupLibSearch(&addressIdLookupStruct);
	
	return err;
}
