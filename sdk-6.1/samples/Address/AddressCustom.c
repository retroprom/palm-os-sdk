/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AddressCustom.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the Address Book "Rename Custom Fields" screen
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <Form.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <PenInputMgr.h>
#include <ErrorMgr.h>
#include <TraceMgr.h>
#include <Table.h>
#include <ScrollBar.h>
#include <UIResources.h>
#include <string.h>

#include "AddressCustom.h"
#include "AddressRsc.h"
#include "AddressTools.h"
#include "Address.h"
#include "AddressDBSchema.h"

/***********************************************************************
 *
 *	Internal globals
 *
 ***********************************************************************/
static CustomItemType	sCustomItems[kMaxNumItems];
static int32_t			sTableTopItemIndex;
static uint16_t			sNumItems;

/***********************************************************************
 *
 * FUNCTION:		PrvCustomEditTableLoadItem
 *
 * DESCRIPTION:		Callback called by the table code
 *
 * PARAMETERS:		A lot (See TableLoadDataFuncType documentation)
 *
 * RETURNED:		errNone
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		TEs		11/22/02	Initial revision
 *
 ***********************************************************************/
status_t PrvCustomEditTableLoadItem(TableType *tableP, int16_t row, int16_t column, Boolean editable, MemHandle *dataH, int16_t *dataOffset, int16_t *dataSize, FieldType *fieldP)
{
	uint32_t	index;

	// Init values
	*dataH		= NULL;
	*dataSize	= 0;
	*dataOffset	= 0;

	if (fieldP && editable)
	{
		FieldAttrType	attributes;

		FldGetAttributes(fieldP, &attributes);
		attributes.dynamicSize = 0;
		FldSetAttributes(fieldP, &attributes);

		FldSetMaxChars(fieldP, kCustomFieldMaxChars);
	}

	index = TblGetRowData(tableP, row);

	if (! sCustomItems[index].textP[0])
		return errNone;

	*dataH = MemHandleNew(kCustomFieldMaxChars + 1);
	ErrFatalDisplayIf(*dataH == NULL, "Out of memory!");
	strcpy(MemHandleLock(*dataH), sCustomItems[index].textP);
	MemHandleUnlock(*dataH);
	*dataSize = kCustomFieldMaxChars + 1;

	return errNone;
}

/***********************************************************************
 *
 * FUNCTION:		PrvCustomEditTableSaveItem
 *
 * DESCRIPTION:		Called by the table code when a cell losts the focus
 *
 * PARAMETERS:		See PrvCustomEditTableLoadItem documentation
 *
 * RETURNED:		true if the table must redrawn, false otherwise
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		TEs		11/22/02	Initial revision
 *
 ***********************************************************************/
Boolean PrvCustomEditTableSaveItem(TableType *tableP, int16_t row, int16_t column)
{
	FieldType *	fieldP;
	MemHandle	fieldTextH;
	uint32_t	index;

	if ((fieldP = TblGetCurrentField(tableP)) == NULL)
		return false;

	if ((fieldTextH = FldGetTextHandle(fieldP)) != NULL)
	{
		index = TblGetRowData(tableP, row);
		strcpy(sCustomItems[index].textP, MemHandleLock(fieldTextH));
		MemHandleUnlock(fieldTextH);
	}

	return false;
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditLoadTable
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		TEs		11/22/02	Initial revision
 *
 ***********************************************************************/
static void PrvCustomEditLoadTable(FormType *formP)
{
	TableType *	tableP;
	uint16_t	numRows;
	uint16_t	row;

	tableP = ToolsGetFrmObjectPtr(formP, CustomEditTable);
    numRows = TblGetNumberOfRows(tableP);

	row = 0;
	while (row < sNumItems && row < numRows)
	{
		TblSetRowData(tableP, row, (uint32_t)(row + sTableTopItemIndex));
		TblSetItemPtr(tableP, row, kLabelTableColumn, sCustomItems[row + sTableTopItemIndex].labelP);
		TblSetRowUsable(tableP, row, true);
		row++;
	}

	while (row < numRows)
	{
		TblSetRowUsable(tableP, row, false);
		row++;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditUpdateScrollbar
 *
 * DESCRIPTION:
 *
 * PARAMETERS:  formP
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		TEs		11/25/02	Initial Revision
 *
 ***********************************************************************/
static void PrvCustomEditUpdateScrollbar(FormType *formP)
{
	ScrollBarType *	scrollP;
	int16_t			max;
	uint16_t		tableRows;

	scrollP = ToolsGetFrmObjectPtr(formP, CustomEditScrollbar);

	tableRows = TblGetNumberOfRows(ToolsGetFrmObjectPtr(formP, CustomEditTable));
	max = (sNumItems < tableRows) ? 0 : sNumItems - tableRows;
	SclSetScrollBar(scrollP, sTableTopItemIndex, 0, max, 1);
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditSave
 *
 * DESCRIPTION: Write the renamed field labels
 *
 * PARAMETERS:  frm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   2/23/95   Initial Revision
 *
 ***********************************************************************/
void PrvCustomEditSave(void)
{
	uint16_t	row;
	char *		textP;
	uint16_t	tabIndex;
	uint16_t	tabID;
	status_t	err = errNone;

	// Save tab info
	tabIndex = gBookInfo.currentTabIndex;
	tabID = gBookInfo.currentTabId;

	// Release schema properties so DbSetColumnPropertyValue won't failed
	AddressTabReleaseSchemaInfoFromDB(&gBookInfo, gAddrDB);

	for (row = 0; row < sNumItems; row++)
	{
		textP = sCustomItems[row].textP;

		if (textP && *textP)
		{
			err = DbSetColumnPropertyValue(gAddrDB,
									kAddressDefaultTableName,
									sCustomItems[row].columnID,
									kAddrColumnPropertyNewName,
									(uint32_t)(strlen(textP) + 1),
									textP);
		}
		// Delete property only if it exists
		else if (AddressDBColumnPropertyExists(gAddrDB,
												kAddressDefaultTableName,
												sCustomItems[row].columnID,
												kAddrColumnPropertyNewName))
		{

			err = DbRemoveColumnProperty(gAddrDB,
									kAddressDefaultTableName,
									sCustomItems[row].columnID,
									kAddrColumnPropertyNewName);
		}
		else
			err = errNone;

		ErrNonFatalDisplayIf(err < errNone, "Can't set column property");
	}

	// Restore book info
	AddressTabInitSchemaInfoFromDB(&gBookInfo, gAddrDB);

	// Restore tab info
	gBookInfo.currentTabIndex = tabIndex;
	gBookInfo.currentTabId = tabID;
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditInit
 *
 * DESCRIPTION: Load field labels for editing.
 *
 * PARAMETERS:  frm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         	Name   	Date      		Description
 *         	----   	----      		-----------
 *         	roger   	2/23/95   	Initial Revision
 *		ppl		02/06/02	Active input area support
 *
 ***********************************************************************/
static void PrvCustomEditInit(void)
{
	TableType *						tableP;
	int16_t							numRows;
	int16_t							row;
	uint16_t						tab;
	Coord							columnWidth = 0;
	Coord							width;
	RectangleType					bounds;
	uint16_t						customLabelMaxWidth;
	MemHandle						colonResourceH;
	char*							colonStrP;
	FormType*						formP;
	AddressTabsInfoType*			tabsP;
	AddressTabColumnPropertiesType*	propertiesP;

	formP = FrmGetFormPtr(CustomEditDialog);

	tableP = ToolsGetFrmObjectPtr(formP, CustomEditTable);

	numRows = TblGetNumberOfRows(tableP);

	for (row = 0; row < numRows; row++)
	{
		TblSetItemStyle(tableP, row, kLabelTableColumn, labelTableItem);
		TblSetItemStyle(tableP, row, kFieldTableColumn, textTableItem);
		TblSetRowUsable(tableP, row, false);
	}

	TblSetColumnUsable(tableP, kLabelTableColumn, true);
	TblSetColumnUsable(tableP, kFieldTableColumn, true);

	TblSetLoadDataProcedure(tableP, kFieldTableColumn, (TableLoadDataFuncType*)PrvCustomEditTableLoadItem);
	TblSetSaveDataProcedure(tableP, kFieldTableColumn, (TableSaveDataFuncType*)PrvCustomEditTableSaveItem);

	TblGetBounds(tableP, &bounds);

	customLabelMaxWidth = bounds.extent.x >> 1;

	// Load renable fields from DB
	memset(sCustomItems, 0, sizeof(CustomItemType) * kMaxNumItems);
	sNumItems = 0;

	for (tab = 0; tab < gBookInfo.numTabs; tab++)
	{
		tabsP = gBookInfo.tabs;

		for (row = 0; row < tabsP[tab].numElement; row++)
		{
			if (sNumItems >= kMaxNumItems)
			{
				ErrNonFatalDisplay("Not enough room to handle all renamable fields");
				return;
			}

			propertiesP = &tabsP[tab].columnsPropertiesP[row];

			if (propertiesP->renameAllowed)
			{
				char *	renamedLabelP;

				renamedLabelP = sCustomItems[sNumItems].textP;
				*renamedLabelP = 0;

				if (propertiesP->renamedLabelP)
					StrLCopy(renamedLabelP, propertiesP->renamedLabelP, kCustomFieldMaxChars + 1);

				sCustomItems[sNumItems].labelP = MemPtrNew((uint32_t)strlen(propertiesP->labelP) + 1);
				ErrFatalDisplayIf(!sCustomItems[sNumItems].labelP, "Out of memory!");

				FntTruncateString(sCustomItems[sNumItems].labelP, propertiesP->labelP, stdFont, customLabelMaxWidth, true);
				width = FntCharsWidth(sCustomItems[sNumItems].labelP, strlen(sCustomItems[sNumItems].labelP));

				if (width > columnWidth)
					columnWidth = width;

				sCustomItems[sNumItems].columnID = propertiesP->columnId;
				sNumItems++;
			}
		}
	}

	// NOTE: Not really usefull here because the table concatenate always ':' to the label whatever the locale.
	// But code is ready if table is updated
	if ((colonResourceH = DmGetResource(gApplicationDbP, strRsc, LabelSuffixColonStr)) != NULL)
		colonStrP = DmHandleLock(colonResourceH);
	else colonStrP = ":";

	columnWidth += FntCharsWidth(colonStrP, strlen(colonStrP));

	if (colonResourceH)
	{
		DmHandleUnlock(colonResourceH);
		DmReleaseResource(colonResourceH);
	}

	TblSetColumnWidth(tableP, kLabelTableColumn, columnWidth);
	TblSetColumnWidth(tableP, kFieldTableColumn, bounds.extent.x - columnWidth - 1);

	sTableTopItemIndex = 0;
	PrvCustomEditLoadTable(formP);
	PrvCustomEditUpdateScrollbar(formP);

	// Give focus to the first table item
	FrmSetFocus(formP, FrmGetObjectIndex(formP, CustomEditTable));
	TblGrabFocus(tableP, 0, kFieldTableColumn);
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditClose
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		02/28/03	Initial revision
 *
 ***********************************************************************/
static void PrvCustomEditClose(void)
{
	// Free allocated memory
	while (sNumItems--)
	{
		if (sCustomItems[sNumItems].labelP)
			MemPtrFree(sCustomItems[sNumItems].labelP);
	}

	// Switch to Interaction navigation mode
	FrmSetNavState(FrmGetFormPtr(CustomEditDialog), kFrmNavStateFlagsInteractionMode);
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditDrawTblItemFocusRing
 *
 * DESCRIPTION:	Draw the focus ring around the passed row
 *
 * PARAMETERS:	->	tableP
 *				->	row
 *
 * RETURNED:	Nothing
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		07/23/04	Initial revision
 *
 ***********************************************************************/
static void PrvCustomEditDrawTblItemFocusRing(TableType *tableP, int16_t row)
{
	RectangleType	bounds;

	TblGetItemBounds(tableP, row, kFieldTableColumn, &bounds);
	FrmSetFocusHighlight(FrmGetWindowHandle(FrmGetFormPtr(CustomEditDialog)), &bounds, 0);
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditTableNavigate
 *
 * DESCRIPTION:	Moves the table field focus up or down
 *
 * PARAMETERS:	->	formP
 *				->	direction: winUp or winDown
 *
 * RETURNED:	true if the focus has moved, false otherwise
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		07/23/04	Initial revision
 *
 ***********************************************************************/
static Boolean PrvCustomEditTableNavigate(FormType *formP, WinDirectionType direction)
{
	TableType *	tableP;
	int16_t		row = 0, col = kFieldTableColumn;
	int16_t		lastRowIndex;
	Boolean		redraw = false;
	Boolean		moveFocusRing = false;
    Boolean		setFocus = true;
	int32_t		savedTopItem;

	tableP = ToolsGetFrmObjectPtr(formP, CustomEditTable);

	if (!tableP)
		return false;

	lastRowIndex = TblGetLastUsableRow(tableP);

	// Move focus one line up or down
	if (TblEditing(tableP))
	{
		TblGetSelection(tableP, &row, &col);

		if (direction == winUp)
		{
			if (row > 0)
			{
				row--;
				moveFocusRing = true;
			}
			else if (sTableTopItemIndex > 0)
			{
				sTableTopItemIndex--;
				redraw = true;
			}
			else
				return false;
		}
		else // winDown
		{
			if (row < lastRowIndex)
			{
				row++;
				moveFocusRing = true;
			}
			else if (sTableTopItemIndex + lastRowIndex + 1 < sNumItems)
			{
				sTableTopItemIndex++;
				redraw = true;
			}
			else
				return false;
		}
	}

	// The whole table is selected. Scroll page by page
	else
	{
		// Not enough item in the table. Exits without scrolling
		if (sNumItems <= lastRowIndex + 1)
			return false;

		setFocus = false;
		redraw = true;

		savedTopItem = sTableTopItemIndex;
		
		if (direction == winUp)
		{
			if (sTableTopItemIndex - lastRowIndex >= 0)
				sTableTopItemIndex -= lastRowIndex;
			else
				sTableTopItemIndex = 0;
		}
		else // winDown
		{
			if (sTableTopItemIndex + lastRowIndex < (int32_t)sNumItems - lastRowIndex)
				sTableTopItemIndex += lastRowIndex;
			else
				sTableTopItemIndex = (int32_t)sNumItems - lastRowIndex - 1;
		}

		if (sTableTopItemIndex == savedTopItem)
			return false;
	}

	if (setFocus)
		TblReleaseFocus(tableP);

	if (redraw)
	{
		PrvCustomEditLoadTable(formP);
		PrvCustomEditUpdateScrollbar(formP);
		ToolsFrmInvalidateWindow(CustomEditDialog);
	}

	if (setFocus)
		TblGrabFocus(tableP, row, col);

	if (moveFocusRing)
		PrvCustomEditDrawTblItemFocusRing(tableP, row);

	return true;
}

/***********************************************************************
 *
 * FUNCTION:    PrvCustomEditHandleOnehandedNavigation
 *
 * DESCRIPTION:	Manage Onehanded key down events
 *
 * PARAMETERS:	->	eventP
 *
 * RETURNED:	true if handled, false otherwise
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	TEs		07/23/04	Initial revision
 *
 ***********************************************************************/
static Boolean PrvCustomEditHandleOnehandedNavigation(EventType *eventP)
{
	Boolean					handled = false;
	FormType *				formP;
	TableType *				tableP;
	FieldType *				fieldP;
	FrmNavStateFlagsType	navState;
	int16_t					row, col;
	Boolean					objectFocusMode;

	formP = FrmGetFormPtr(CustomEditDialog);

	objectFocusMode = FrmGetNavState(formP, &navState) >= errNone &&
					(navState & kFrmNavStateFlagsObjectFocusMode) != 0;

	switch (eventP->data.keyDown.chr)
	{
	case vchrRockerUp:
	case vchrRockerDown:
		if (objectFocusMode &&
			FrmGetFocus(formP) == FrmGetObjectIndex(formP, CustomEditTable))
		{
			handled = PrvCustomEditTableNavigate(formP, (eventP->data.keyDown.chr == vchrRockerUp) ? winUp : winDown) ||
				(eventP->data.keyDown.modifiers & autoRepeatKeyMask) != 0;
		}
		break;

	case vchrRockerLeft:
		tableP = ToolsGetFrmObjectPtr(formP, CustomEditTable);

		if (objectFocusMode &&
			FrmGetFocus(formP) == FrmGetObjectIndex(formP, CustomEditTable) &&
			(fieldP = TblGetCurrentField(tableP)) != NULL &&
			FldGetInsPtPosition(fieldP) == 0 &&
			(eventP->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
		{
			FrmNavObjectTakeFocus(formP, CustomEditTable);
			handled = true;
		}
		break;

	case vchrRockerCenter:
		tableP = ToolsGetFrmObjectPtr(formP, CustomEditTable);

		if (!objectFocusMode)
		{
			FrmNavObjectTakeFocus(formP, CustomEditOkButton);
		}
		else if (FrmGetFocus(formP) == FrmGetObjectIndex(formP, CustomEditTable))
		{
			if (!TblEditing(tableP))
			{
				TblGetSelection(tableP, &row, &col);
				TblGrabFocus(tableP, row, kFieldTableColumn);
				PrvCustomEditDrawTblItemFocusRing(tableP, row);
			}

			handled = true;
		}

		break;
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    CustomEditHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Edit Custom
 *              Fields" of the Address application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         	Name   	Date      		Description
 *         	----   	----      		-----------
 *         	roger   	06/23/95   	Initial Revision
 *        	FPa     	11/28/00  	Sends an event when OK button pressed
 *		ppl		02/06/02	Active input area support
 *
 ***********************************************************************/
Boolean CustomEditHandleEvent(EventType * eventP)
{
	Boolean			handled = false;
	FormType *		formP;
	TableType *		tableP;
	uint16_t		objIndex;
	RectangleType	bounds;
	
	switch (eventP->eType)
	{
		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case CustomEditOkButton:
					PrvCustomEditSave();
					PrvCustomEditClose();

					FrmReturnToForm(0);

					// We send this event because View screen needs to recalculate its display when Custom fields are renamed
					PrvPostUserEvent(kFrmCustomUpdateEvent);
					
					handled = true;
					break;

				case CustomEditCancelButton:
					PrvCustomEditClose();
					FrmReturnToForm(0);

					handled = true;
					break;
			}
			break;

		case frmOpenEvent:
			PrvCustomEditInit();
			handled = true;
			break;

		case frmCloseEvent:
			PrvCustomEditClose();
			break;

		case sclRepeatEvent:
			if (eventP->data.sclRepeat.value == eventP->data.sclRepeat.newValue)
				break;

			sTableTopItemIndex = eventP->data.sclRepeat.newValue;
			PrvCustomEditLoadTable(FrmGetFormPtr(CustomEditDialog));
			ToolsFrmInvalidateWindow(CustomEditDialog);
			break;

		case tblSelectEvent:
			// If the label is selected, give the focus to the corresponding field
			if (eventP->data.tblSelect.column == kLabelTableColumn)
				TblGrabFocus(eventP->data.tblSelect.pTable, eventP->data.tblSelect.row, kFieldTableColumn);
			break;

		case keyDownEvent:
			handled = PrvCustomEditHandleOnehandedNavigation(eventP);
			break;

		case frmObjectFocusTakeEvent:
			if (eventP->data.frmObjectFocusTake.objectID == CustomEditTable)
			{
				formP = FrmGetFormPtr(CustomEditDialog);
				objIndex = FrmGetObjectIndex(formP, CustomEditTable);
				tableP = FrmGetObjectPtr(formP, objIndex);
				FrmGetObjectBounds(formP, objIndex, &bounds);

				TblUnhighlightSelection(tableP);
				TblReleaseFocus(tableP);

				FrmSetFocus(formP, objIndex);
				FrmSetFocusHighlight(FrmGetWindowHandle(formP), &bounds, 0);
                
				handled = true;
			}
			break;
		
		default:
			break;
	}

	return (handled);
}
