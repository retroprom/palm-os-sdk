/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressFreeFormName.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *			Formating Free Form name functions based on Free Form Name resource types.
 *			Loading /saving related resources types functions.
 *
 *
 *			Free Form Name Template is built using a tSTL resource that maps one-one
 *			with the u32L resources with the same resource Id.
 *
 *			u32L is a list of UINT32.
 *
 *			Each entry in the Free Form name template resource has the following structure
 * 			<symbols>^0<more symbols> where:
 *				-<Symbols> and more Symbols are any  text ,
 *				punctuation signs and so on...
 *				- ^0 is a parameter string that will be replace with the name
 *				 in the column Id
 *
 *			Final Free Form name string is the concatenation of all [n] strings built using
 * 			these template and the related [n] columns IDs list:
 *				Sigma (<symbols-i><name[ID-i]i><symbols- i>)[n]
 *
 * 			A string is part of the final Free Form name only when the name in the
 *			considered database record is not empty.
 *
 *			Id 2800 is  reserved for Edit/View form Free Form name template.
 *			Id 2805 is  reserved for vCard full name template.
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <MemoryMgr.h>
#include <string.h>

#include <TextMgr.h>

#include <SysUtils.h>
#include <StringMgr.h>

#include "AddressU32List.h"
#include "AddressFreeFormName.h"
#include "AddressDBSchema.h"
#include "AddressDisplayLayouts.h"
#include "AddressRsc.h"
#include "AddressTools.h"
#include "Address.h"

/***********************************************************************
 *
 * FUNCTION:    AddrFreeFormNameBuild
 *
 * DESCRIPTION:
 *			This routine builds a Free Form  Name given the Free Form template,
 * 			for the given record in the schema based address book database.
 *
 *
 * PARAMETERS:
 *			dbP:	AddressBook Database Ref
 *			rowID:	rowID or cursor ID positioned on a valid row
 *
 * RETURNED:    error code
 *
 * REVISION HISTORY:
 *
 * Name		Date			Description
 * ----		--------	-----------
 * ppl		11/19/02	Initial Revision
 * TEs		04/15/03	Rewrote to use the display layouts engine
 *
 ***********************************************************************/
MemHandle AddrFreeFormNameBuild(DmOpenRef dbP, uint32_t rowID)
{
	MemHandle					fullnameH = NULL;
	char *						fullnameP = NULL;
	uint32_t*					columnIDsP = NULL;
	uint32_t					numColumns = 0;
	MemHandle					columnIDResH = NULL;
	ViewDisplayLayoutInfoType	layoutInfo;
	ViewDiplayLayoutRowTextType	rowText;

	if (!gFFNColumnListResID)
		gFFNColumnListResID = ToolsGetCountryBaseResID(kU32ListResType, kAddrFFNTemplateBaseID, kAddrTemplateResIDIncrement);

	if ((columnIDResH = U32ListGetList(gApplicationDbP, gFFNColumnListResID)) != NULL)
	{
		columnIDsP = DmHandleLock(columnIDResH);
		numColumns = U32ListGetItemNum(NULL, 0, columnIDResH);
	}

	if (!gFFNLayoutResID)
		gFFNLayoutResID = ToolsGetCountryBaseResID(kViewBlockLayoutRscType, kAddrFFNTemplateBaseID, kAddrTemplateResIDIncrement);

	layoutInfo.dbP			= dbP;
	layoutInfo.rowID		= rowID;
	layoutInfo.columnIDsP	= columnIDsP;
	layoutInfo.numColumns	= numColumns;
	layoutInfo.layoutRscID	= gFFNLayoutResID;
	layoutInfo.fontID		= stdFont;

	memset(&rowText, 0, sizeof(ViewDiplayLayoutRowTextType));

	if (AddrDisplayLayoutsInit(&layoutInfo))
		AddrDisplayLayoutsGetRowText(&layoutInfo, 0, &rowText);
	else
		AddrDisplayLayoutsGetDefaultText(&layoutInfo, &rowText);

	if (!rowText.numFields)
	{
		if (!rowText.fieldTextPP)
			rowText.fieldTextPP = MemPtrNew(sizeof(char*));

		rowText.fieldTextPP[0] = ToolsStrDup(gUnnamedRecordStringP);
		rowText.numBytes = (uint16_t) (gUnnamedRecordStringP ? strlen(gUnnamedRecordStringP) : 0);
		rowText.textWidth = gUnnamedRecordStringP ? FntCharsWidth(gUnnamedRecordStringP, rowText.numBytes) : 0;
		rowText.numFields = 1;
	}

	if (rowText.numFields && (fullnameH = MemHandleNew((uint32_t)rowText.numBytes + 1)) != NULL)
	{
		uint16_t	i;

		fullnameP = MemHandleLock(fullnameH);
		*fullnameP = 0;

		for (i = 0; i < rowText.numFields; i++)
			strcat(fullnameP, rowText.fieldTextPP[i]);

		MemHandleUnlock(fullnameH);
	}

	AddrDisplayLayoutsFreeRowText(&rowText);
	AddrDisplayLayoutsFree(&layoutInfo);

	if (columnIDResH)
	{
		DmHandleUnlock(columnIDResH);
		U32ListReleaseList(columnIDResH);
	}

	return fullnameH;
}

/***********************************************************************
 *
 * FUNCTION:    AddrFreeFormNameFree
 *
 * DESCRIPTION:
 *			This routine frees Free Form Name text handle returned by AddrFreeFormNameBuild.
 *
 *			see AddrFreeFormNameBuild.
 *
 * PARAMETERS:
 *			fullNameH:	MemHandle.
 *
 *
 * RETURNED:    None.
 *
 * REVISION HISTORY:
 *
 * Name		Date			Description
 * ----		--------	-----------
 * ppl		11/19/02	Initial Revision.
 *
 ***********************************************************************/
void AddrFreeFormNameFree(MemHandle fullNameH)
{
	if (fullNameH)
		MemHandleFree(fullNameH);
}
