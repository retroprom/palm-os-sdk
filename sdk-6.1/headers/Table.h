/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Table.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	  This file defines table structures and routines.
 *
 *****************************************************************************/

#ifndef __TABLE_H__
#define __TABLE_H__

#include <PalmTypes.h>

#include <Field.h>

//-------------------------------------------------------------------
// Table structures
//-------------------------------------------------------------------

#define tableDefaultColumnSpacing 		1
#define tableNoteIndicatorWidth			7
#define tableNoteIndicatorHeight		11
#define tableMaxTextItemSize 			255	// does not incude terminating null

// Row type is always int16_t in all table APIs
#define tblUnusableRow					((int16_t)-1)

// Display style of a table item
//
enum tableItemStyles { checkboxTableItem, 
                       customTableItem, 
                       dateTableItem, 
                       labelTableItem,
                       numericTableItem,
                       popupTriggerTableItem,
                       textTableItem,
                       textWithNoteTableItem,
                       timeTableItem,
                       narrowTextTableItem,
                       tallCustomTableItem,
                       labelNoColonTableItem,
                       popupTriggerNoColonTableItem
                       };
typedef Enum8 TableItemStyleType;

// Draw item callback routine prototype, used only by customTableItem.
typedef void TableDrawItemFuncType  
		(void *tableP, int16_t row, int16_t column, RectangleType *bounds);

typedef TableDrawItemFuncType *TableDrawItemFuncPtr;

// Load data callback routine prototype
typedef status_t TableLoadDataFuncType 
		(void *tableP, int16_t row, int16_t column, Boolean editable, 
		MemHandle * dataH, int16_t *dataOffset, int16_t *dataSize, FieldPtr fld);

typedef TableLoadDataFuncType *TableLoadDataFuncPtr;


// Save data callback routine prototype
typedef	Boolean TableSaveDataFuncType
		(void *tableP, int16_t row, int16_t column);

typedef TableSaveDataFuncType *TableSaveDataFuncPtr;

typedef struct TableType TableType;

typedef TableType *TablePtr;


//-------------------------------------------------------------------
// Table routines
//-------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern void TblInvalidate(TableType *tblP);

extern void TblDrawTable (TableType *tableP);

extern void TblRedrawTable (TableType *tableP);

extern void TblEraseTable (TableType *tableP);

extern Boolean TblHandleEvent (TableType *tableP, EventType *event);

extern void TblGetItemBounds (const TableType *tableP, int16_t row, int16_t column, RectangleType *rP);

extern void TblSelectItem (TableType *tableP, int16_t row, int16_t column);

extern int16_t TblGetItemInt (const TableType *tableP, int16_t row, int16_t column);

extern void TblSetItemInt (TableType *tableP, int16_t row, int16_t column, int16_t value);

extern void TblSetItemPtr (TableType *tableP, int16_t row, int16_t column, void *value);

extern void TblSetItemStyle (TableType *tableP, int16_t row, int16_t column, TableItemStyleType type);

extern void TblUnhighlightSelection (TableType *tableP);

extern Boolean TblRowUsable  (const TableType *tableP, int16_t row);

extern void TblSetRowUsable (TableType *tableP, int16_t row, Boolean usable);

extern int16_t TblGetLastUsableRow (const TableType *tableP);

extern void TblSetColumnUsable (TableType *tableP, int16_t column, Boolean usable);

extern Boolean TblColumnUsable (const TableType *tableP, int16_t column);

extern void TblSetRowSelectable (TableType *tableP, int16_t row, Boolean selectable);

extern Boolean TblRowSelectable (const TableType *tableP, int16_t row);

extern int16_t TblGetNumberOfRows (const TableType *tableP);

extern void TblSetCustomDrawProcedure (TableType *tableP, int16_t column,
	TableDrawItemFuncPtr drawCallback);

extern void TblSetLoadDataProcedure (TableType *tableP, int16_t column,
	TableLoadDataFuncPtr loadDataCallback);

extern void TblSetSaveDataProcedure (TableType *tableP, int16_t column,
	TableSaveDataFuncPtr saveDataCallback);

extern void TblGetBounds (const TableType *tableP, RectangleType *rP);

extern void TblSetBounds (TableType *tableP, const RectangleType *rP);

extern Coord TblGetRowHeight (const TableType *tableP, int16_t row);

extern void TblSetRowHeight (TableType *tableP, int16_t row, Coord height);

extern Coord TblGetColumnWidth (const TableType *tableP, int16_t column);

extern void TblSetColumnWidth (TableType *tableP, int16_t column, Coord width);

extern Coord TblGetColumnSpacing (const TableType *tableP, int16_t column);

extern void TblSetColumnSpacing (TableType *tableP, int16_t column, Coord spacing);

extern Boolean TblFindRowID (const TableType *tableP, uint16_t id, int16_t *rowP);

extern Boolean TblFindRowData (const TableType *tableP, uint32_t data, int16_t *rowP);

extern uint16_t TblGetRowID (const TableType *tableP, int16_t row);

extern void TblSetRowID (TableType *tableP, int16_t row, uint16_t id);

extern uint32_t TblGetRowData (const TableType *tableP, int16_t row);

extern void TblSetRowData (TableType *tableP, int16_t row, uint32_t data);

extern Boolean TblRowInvalid (const TableType *tableP, int16_t row);

extern void TblMarkRowInvalid (TableType *tableP, int16_t row);

extern void TblMarkTableInvalid (TableType *tableP);

extern Boolean TblGetSelection (const TableType *tableP, int16_t *rowP, int16_t *columnP);

extern void TblInsertRow (TableType *tableP, int16_t row);
							
extern void TblRemoveRow (TableType *tableP, int16_t row);

extern void TblReleaseFocus (TableType *tableP);
							
extern Boolean TblEditing (const TableType *tableP);

extern FieldPtr TblGetCurrentField (const TableType *tableP);
							
extern void TblGrabFocus (TableType *tableP, int16_t row, int16_t column);

extern void TblSetColumnEditIndicator (TableType *tableP, int16_t column, Boolean editIndicator);

extern void TblSetRowStaticHeight (TableType *tableP, int16_t row, Boolean staticHeight);

extern void TblHasScrollBar (TableType *tableP, Boolean hasScrollBar);

extern FontID TblGetItemFont (const TableType *tableP, int16_t row, int16_t column);

extern  void TblSetItemFont (TableType *tableP, int16_t row, int16_t column, FontID fontID);

extern void *TblGetItemPtr (const TableType *tableP, int16_t row, int16_t column);

extern Boolean TblRowMasked  (const TableType *tableP, int16_t row);
							
extern void TblSetRowMasked  (TableType *tableP, int16_t row, Boolean masked);
							
extern void TblSetColumnMasked  (TableType *tableP, int16_t column, Boolean masked);

extern int16_t TblGetNumberOfColumns (const TableType *tableP);

extern int16_t TblGetTopRow (const TableType *tableP);

extern void TblSetSelection (TableType *tableP, int16_t row, int16_t column);

extern Boolean TblGetColumnMasked (const TableType *tableP, int16_t column);

extern TableItemStyleType TblGetItemStyle (const TableType *tableP, int16_t row, int16_t column);

#ifdef __cplusplus 
}
#endif

#endif //__TABLE_H__
