/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressU32List.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *			Set of Functions to handle u32L resources
 *			List of Unsigned integer whithout counter
 *			format = [uInt32]n
  *
 *			Can be defined in raw data resource (do not forget little endian !)
 *
 * 			<RAW_RESOURCE RESOURCE_ID="2800">
 *				<RES_TYPE> 'u32L' </RES_TYPE>
 *				<RES_DATA>
 *						64 00 00 00
 *						C8 00 00 00
 *						2C 01 00 00
 *						90 01 00 00
 *						F4 01 00 00
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

 #include "AddressU32List.h"

/***********************************************************************
 *
 * FUNCTION:    U32ListGetList
 *
 * DESCRIPTION:	This routine loads an u32L resources, given the resource
 *				database open reference, the resource ID and returns a
 *				resource Handle.
 *
 *				The returned handle is not locked.
 *
 * PARAMETERS:	resourceDBRef:	resource database db ref.
 *				u32listId:		u32L resource ID.

 *
 * RETURNED:    uint32_t List Handle.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		11/19/02	Initial Revision
 *
 ***********************************************************************/
MemHandle U32ListGetList(DmOpenRef resourceDBRef, DmResourceID u32listId)
{
	return DmGetResource(resourceDBRef, kU32ListResType, u32listId);
}


/***********************************************************************
 *
 * FUNCTION:    U32ListGetItem
 *
 * DESCRIPTION:	This routine retrieves an uint32_t at a specific index,
 *				using a list already loaded into memory or directly
 * 				in a resource database.
 *
 * PARAMETERS:	resourceDBRef: 	resource database db ref
 *				u32listId: 		u32L resource ID
 *				u32ListH:		u32L already loaded Resource (could be NULL)
 *				index:			item rank to retrieve
 *
 *
 * RETURNED:    ID, Uint32
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		11/19/02	Initial Revision
 *
 ***********************************************************************/
uint32_t U32ListGetItem(DmOpenRef resourceDBRef, DmResourceID u32listId, MemHandle u32ListH, uint32_t index)
{
	MemHandle 	resH;
	uint32_t*	uLongP;
	uint32_t	uLong = 0;

	if (u32ListH)
		resH = u32ListH;
	else
	 resH = U32ListGetList( resourceDBRef,  u32listId);

	if (resH)
	{
		uLongP = (uint32_t*) DmHandleLock(resH);

		uLong = uLongP[index];

		DmHandleUnlock(resH);
	}

	if (!u32ListH)
		U32ListReleaseList(resH);

	return uLong;
}


/***********************************************************************
 *
 * FUNCTION:    U32ListGetItemNum
 *
 * DESCRIPTION:	This routine retrieves the number of unsigned long integer
 *				of an u32L resource, given the resource database open
 *				reference and its resource ID or the resource handle.
 *
 * PARAMETERS:	resourceDBRef: 	resource database db ref
 *				u32listId: 		u32L resource ID
 *				MemHandle:		if not NULL this handle is used in place of
 *								reloading the resource.
 *
 * RETURNED:    How many unsigned longs inside
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		11/19/02	Initial Revision
 *
 ***********************************************************************/
uint32_t U32ListGetItemNum(DmOpenRef resourceDBRef, DmResourceID u32listId, MemHandle u32ListH)
{
 	MemHandle 	resH;

	 if (u32ListH)
 		resH = u32ListH;
 	else
		resH = U32ListGetList(resourceDBRef, u32listId);

	return DmHandleSize(resH) / sizeof(uint32_t);
}


/***********************************************************************
 *
 * FUNCTION:    U32ListReleaseList
 *
 * DESCRIPTION:	This routine release the u32l resource.
 *
 * PARAMETERS:	columnListH:	u32L list handle.
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * ppl		11/19/02	Initial Revision
 *
 ***********************************************************************/
void U32ListReleaseList(MemHandle  u32ListH)
{
	if (u32ListH)
		DmReleaseResource(u32ListH);
}

/***********************************************************************
 *
 * FUNCTION:    U32ListContainsItem
 *
 * DESCRIPTION:	This find an item inside a list.
 *
 *
 * PARAMETERS:	resourceDBRef: 	resource database db ref
 *				u32listId: 		u32L resource ID
 *				u32ListH:		u32L already loaded Resource (could be NULL)
 *				itemToFind:		item to find inside the list
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 * Name		Date		Description
 * ----		--------	-----------
 * LFe		07/10/03	Initial Revision
 *
 ***********************************************************************/
Boolean U32ListContainsItem(DmOpenRef resourceDBRef, DmResourceID u32listId, MemHandle u32ListH, uint32_t itemToFind)
{
 	MemHandle 	resH;
 	uint32_t*	uLongP;
 	uint32_t	howMany;
	uint16_t	i;
	Boolean		found = false;

	if (u32ListH)
 		resH = u32ListH;
 	else
		resH = U32ListGetList( resourceDBRef,  u32listId);

	if (resH)
	{
		howMany = DmHandleSize(resH) / sizeof(uint32_t);
		uLongP = (uint32_t*) DmHandleLock(resH);

		for (i = 0; i < howMany && !found; i++)
			 found = (Boolean)(uLongP[i] == itemToFind);

		DmHandleUnlock(resH);
	}

	return found;
}
