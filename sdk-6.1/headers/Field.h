/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Field.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines field structures and routines.
 *
 *****************************************************************************/

#ifndef _FIELD_H_
#define _FIELD_H_

#include <PalmTypes.h>

#include <Font.h>
#include <Event.h>
#include <Window.h>
#include <limits.h>

#define maxFieldTextLen	SIZE_MAX

// default maximun number of line the a dynamicly sizing field will expand to.
// Can be changed with FldSetMaxVisibleLines
#define  maxFieldLines 	11


// Note: This is also used in TextServices/JapaneseFep/TsmFEP.h and
//       TextServices/SampleFep/SampleFepPrv.h, which aren't in the regular build!
#define undoBufferSize 100


// kind alignment values
enum justifications { leftAlign, centerAlign, rightAlign };
typedef Enum8 JustificationType;


typedef struct FieldAttrTag
{
	uint16_t usable			:1;	// Read-only, true if object should be drawn
	uint16_t visible		:1;	// Read-only, true if drawn
	uint16_t editable		:1;	// Read-only, true if editable
	uint16_t singleLine		:1;	// Read-only, true if only a single line is displayed
	uint16_t hasFocus      	:1; // Read-only, true if the field has the focus
	uint16_t dynamicSize	:1; // Read-write, true if height expands as text is entered
	uint16_t insPtVisible	:1;	// Read-only, true if the ins pt is scrolled into view
	uint16_t dirty			:1;	// Read-only (set with FldSetDirty()), true if user modified
	uint16_t underlined		:2;	// Read-write, but won't be redrawn automatically; text underlined mode (UnderlineModeType)
	uint16_t justification	:2;	// Read-write, but won't be redrawn automatically; text alignment
	uint16_t autoShift		:1;	// Read-write, true if auto case shift
	uint16_t hasScrollBar	:1;	// Read-write, true if the field has a scroll bar
	uint16_t numeric		:1;	// Read-write, true if numeric, digits, and decimal separator only
	uint16_t unused			:1; // padding
} FieldAttrType;

typedef FieldAttrType *FieldAttrPtr;

typedef struct FieldType FieldType;

typedef FieldType *FieldPtr;					// deprecated, use FieldType *


//---------------------------------------------------------------------
//	Field Functions
//---------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

void FldCopy (const FieldType *fldP);

void FldCut (FieldType *fldP);

void FldDrawField (FieldType *fldP);

void FldEraseField (FieldType *fldP);
 
void FldFreeMemory (FieldType *fldP);

void FldGetBounds (const FieldType *fldP, RectanglePtr rect);

FontID FldGetFont (const FieldType *fldP);

void FldGetSelection (const FieldType *fldP, size_t *startPosition, size_t *endPosition);

MemHandle FldGetTextHandle (const FieldType *fldP);

char * FldGetTextPtr (const FieldType *fldP);

Boolean FldHandleEvent (FieldType *fldP, EventType *eventP);

void FldPaste (FieldType *fldP);

void FldRecalculateField (FieldType *fldP, Boolean redraw);

void FldSetBounds (FieldType *fldP, const RectangleType *rP);

void FldSetFont (FieldType *fldP, FontID fontID);

void FldSetText (FieldType *fldP, MemHandle textHandle, size_t offset, size_t size);

void FldSetTextHandle (FieldType *fldP, MemHandle textHandle);

void FldSetTextColumn (FieldType *fldP, DmOpenRef database, uint32_t cursor, uint32_t position, uint32_t columnID, size_t offset);

void FldGetTextColumn (const FieldType *fldP, DmOpenRef *databaseP, uint32_t *cursorP, uint32_t *positionP, uint32_t *columnIDP, size_t *offsetP);

void FldSetTextPtr (FieldType *fldP, char *textP);

void FldSetUsable (FieldType *fldP, Boolean usable);

void FldSetSelection (FieldType *fldP, size_t startPosition, size_t endPosition);

void FldGrabFocus (FieldType *fldP);

void FldReleaseFocus (FieldType *fldP);

size_t FldGetInsPtPosition (const FieldType *fldP);

void FldSetInsPtPosition (FieldType *fldP, size_t pos);

void FldSetInsertionPoint (FieldType *fldP, size_t pos);

size_t FldGetScrollPosition (const FieldType *fldP);

void FldSetScrollPosition (FieldType *fldP, size_t pos);
							
void FldGetScrollValues (const FieldType *fldP, uint32_t *scrollLineP,
	uint32_t *textHeightP, uint32_t *fieldHeightP);

size_t FldGetTextLength (const FieldType *fldP);

void FldScrollField (FieldType *fldP, uint32_t linesToScroll, WinDirectionType direction);
							
Boolean FldScrollable (const FieldType *fldP,  WinDirectionType direction);

uint32_t FldGetVisibleLines (const FieldType *fldP);

Coord FldGetTextHeight (const FieldType *fldP);

uint32_t FldCalcFieldHeight (const char *chars, Coord maxWidth);

size_t FldWordWrap (const char *chars, Coord maxWidth);

void FldCompactText (FieldType *fldP);

Boolean FldDirty (const FieldType *fldP);

void FldSetDirty (FieldType *fldP, Boolean dirty);

size_t FldGetMaxChars (const FieldType *fldP);

void FldSetMaxChars (FieldType *fldP, size_t maxChars);

Boolean FldInsert (FieldType *fldP, const char *insertChars, size_t insertLen);

void FldDelete (FieldType *fldP, size_t start, size_t end);

void FldUndo (FieldType *fldP);

size_t FldGetTextAllocatedSize (const FieldType *fldP);

void FldSetTextAllocatedSize (FieldType *fldP, size_t allocatedSize);

void FldGetAttributes (const FieldType *fldP, FieldAttrType *attrP);

void FldSetAttributes (FieldType *fldP, const FieldAttrType *attrP);

void FldSendChangeNotification (const FieldType *fldP);

void FldSendHeightChangeNotification (const FieldType *fldP, size_t pos, int32_t numLines);

Boolean FldMakeFullyVisible (FieldType *fldP);

uint32_t FldGetNumberOfBlankLines (const FieldType *fldP);

FieldType *FldNewField (void **formPP, uint16_t id, 
	Coord x, Coord y, Coord width, Coord height, 
	FontID font, size_t maxChars, Boolean editable, Boolean underlined, 
	Boolean singleLine, Boolean dynamicSize, JustificationType justification, 
	Boolean autoShift, Boolean hasScrollBar, Boolean numeric);

// added in 4.0
void FldSetMaxVisibleLines (FieldType *fldP, uint8_t maxLines);

Boolean FldGetLineInfo(const FieldType *fldP, uint32_t lineNum, size_t *startP, size_t *lengthP);

status_t FldReleaseStorage(FieldType *fldP);

status_t FldReturnStorage(FieldType *fldP);

// added in 6.0
void FldReplaceText(FieldType *fldP, const char *textP);

#ifdef __cplusplus 
}
#endif

#endif // __FIELD_H__
