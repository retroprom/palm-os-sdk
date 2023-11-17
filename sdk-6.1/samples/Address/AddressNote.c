/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressNote.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the Address Book Note screen
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <CatMgr.h>
#include <UIResources.h>
#include <StringMgr.h>
#include <PenInputMgr.h>
#include <DataMgr.h>
#include <ErrorMgr.h>
#include "NoteViewLib.h"
#include <TraceMgr.h>

#include "Address.h"
#include "AddressFreeFormName.h"
#include "AddressNote.h"
#include "AddressDBSchema.h"
#include "AddressRsc.h"
#include "AddressTools.h"

/***********************************************************************
 *
 * FUNCTION:    AddressExistingNote
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *         	Name   	Date      		Description
 *        	----   	----      		-----------
 *			PLe		11/29/02		Initial revision
 *
 ***********************************************************************/
Boolean AddressExistingNote(uint32_t rowID)
{
	uint32_t	valueSize;

	if (rowID == dbInvalidRowID)
		return false;

	if (DbCopyColumnValue(gAddrDB, rowID, kAddrColumnIDNote, 0, NULL, &valueSize) < errNone)
		return false;

	return (Boolean) (valueSize > 0);
}

/***********************************************************************
 *
 * FUNCTION:    AddressEditNote
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  -> rowID: a DB row ID or cursor ID positioned
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *         	Name   	Date      		Description
 *        	----   	----      		-----------
 *			PLe		11/29/02		Initial revision
 *
 ***********************************************************************/
 Boolean AddressEditNote(uint32_t rowID)
 {
	char*	nameP;
	Boolean	noteModified = false;
	Coord	extentX, extentY;
	uint32_t pos = 0;

	if (rowID == dbInvalidRowID)
		return false;

	if (DbIsCursorID(rowID))
		DbCursorGetCurrentPosition(rowID, &pos);

	WinGetWindowExtent(&extentX, &extentY);
	// WinGetWindowExtent sometime returns -1 as width and height.
	if (extentX < 0) extentX = 0;
	nameP = ToolsGetRecordNameStr(gAddrDB, rowID, boldFont, extentX);

	if (nameP)
	{
		noteModified = EditNote(nameP, gAddrDB, rowID, pos, kAddrColumnIDNote, NULL);
		MemPtrFree(nameP);
	}

	return noteModified;
}


/***********************************************************************
 *
 * FUNCTION:    AddressDeleteNote
 *
 * DESCRIPTION: Deletes the note field from the current record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/21/95   Initial Revision
 *
 ***********************************************************************/
void AddressDeleteNote(uint32_t rowID)
{
	status_t	err;

	if (rowID == dbInvalidRowID)
		return;

	err = DbWriteColumnValue(gAddrDB, rowID, kAddrColumnIDNote, 0 /* Offset */, -1, NULL, 0);
	ErrNonFatalDisplayIf(err < errNone, "Can't write column");
}
