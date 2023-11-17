/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DateU16List.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *			Set of Functions to handle u16L resources
 *			List of Unsigned integer whithout counter
 *			format = [uInt16]n
 *
 *			Can be defined in raw data resource (do not forget little endian !)
 *
 * 			<RAW_RESOURCE RESOURCE_ID="n">
 *				<RES_TYPE> 'u16L' </RES_TYPE>
 *				<RES_DATA>	
 *						64 00 
 *						C8 00 
 *						2C 01 
 *						90 01 
 *						F4 01 
 *  				</RES_DATA>
 *			</RAW_RESOURCE>
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <MemoryMgr.h>

#include <TextMgr.h>

#include <SysUtils.h>
#include <StringMgr.h>

#include "DateU16List.h"
 
/***********************************************************************
 *
 * FUNCTION:    	U16ListGetList
 *
 * DESCRIPTION: 	This routine loads  an u16L resources, 
 * 					given the resource database open reference, 
 *					the resource ID and returns a resource Handle.
 *			
 *					The returned handle is not locked.
 *
 *
 * PARAMETERS:  ->	resourceDBRef: 	resource database db ref.
 *				->	ulistId: 		u16L resource ID.
 *
 * NOTE:			None.
 *
 * RETURNED:    	Column ids List Resource Handle.
 *
 * REVISION HISTORY:
 * ppl	12/16/02	Initial revision.
 *
 ***********************************************************************/
MemHandle U16ListGetList(DmOpenRef resourceDBRef, DmResourceID ulistId)
 {
 	return DmGetResource(resourceDBRef, kU16ListResType, ulistId);
 }


/***********************************************************************
 *
 * FUNCTION:    	U16ListGetItem
 *
 * DESCRIPTION: 	This routine retrieves an integer from an u16L 
 *					resources, given the resource database open reference,
 *					the resource ID and the item index inside the resource.
 *
 * PARAMETERS: 	-> 	resourceDBRef: 	resource database db ref.
 *				->	ulistId: 		u16L resource ID.
 *				->	index:			item rank to retrieve.
 *
 *
 * RETURNED:    	ID as  UInt16.
 *
 * REVISION HISTORY:
 * ppl	12/16/02	Initial revision.
 *
 ***********************************************************************/
uint16_t  U16ListGetItem(DmOpenRef resourceDBRef, DmResourceID ulistId, MemHandle  uListH,  uint32_t index)
 {
 	MemHandle 		resH;
 	MemPtr			resP;
 	uint16_t*		uShortP;
 	uint16_t		uShort = 0;

 	if (uListH)
 		resH = uListH;
 	else
		 resH = U16ListGetList( resourceDBRef,  ulistId);

	 if (resH)
	 {
	 	resP = MemHandleLock(resH);
		uShortP = (uint16_t *) resP;
		
		uShort = uShortP[index];
		
	 	MemHandleUnlock(resH);

	 	if (uListH == NULL)
	 		U16ListReleaseList(resH);
	 }

	 return uShort;
 }



/***********************************************************************
 *
 * FUNCTION:    	U16ListGetItemNum
 *
 * DESCRIPTION: 	This routine retrieves the number of unsigned long 
 *					integer of an u16L resource, given the resource 
 *					database open reference and its resource ID.
 *
 *
 * PARAMETERS:  ->	resourceDBRef: 	Resource database db ref.
 *				->	ulistId: 		u16L resource ID.
 *				->	MemHandle:		If not NULL this handle is used in 
 *									place of reloadingthe resource. 
 *									Must be an U16L resource handle.
 *
 *
 * RETURNED:    	How many unsigned longs inside as UInt32.
 *
 * REVISION HISTORY:
 * ppl	12/16/02	Initial revision.
 *
 ***********************************************************************/
uint32_t  U16ListGetItemNum(DmOpenRef resourceDBRef, DmResourceID ulistId, MemHandle  uListH)
 {
 	MemHandle 	resH;
 	uint32_t	size;
 	uint32_t	howMany;
 	
	if (uListH)
 		resH = uListH;
 	else
		resH = U16ListGetList( resourceDBRef,  ulistId);
	 
	size = MemHandleSize(resH);
	
	howMany = size/ sizeof(uint16_t);
	
	return howMany;
 }


/***********************************************************************
 *
 * FUNCTION:    	U16ListReleaseList
 *
 * DESCRIPTION: 	a	This routine frees unsigned long integer list 
 *					resource. (u16L), given its resource Handle.
 *
 *
 * PARAMETERS:  ->	columnListH: 	u16L list handle.
 *
 * RETURNED:    	None.
 *
 * REVISION HISTORY:
 * ppl	12/16/02	Initial revision.
 *
 ***********************************************************************/
void U16ListReleaseList(MemHandle  uListH)
{
	if (uListH)
	{
		DmReleaseResource(uListH);
	}
}
