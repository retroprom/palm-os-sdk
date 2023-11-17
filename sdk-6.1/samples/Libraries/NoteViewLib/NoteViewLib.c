/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: NoteViewLib.c
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This is the PIM applications Note View handling shared library.
 *
 *	This lib is not Thread Safe since this would be useless (only one PIM can
 *	edit a note at any given time).
 *
 *	Most of this code was previously located in AddressBook.
 *
 *****************************************************************************/

#include <SystemResources.h>
#include <SystemMgr.h>
#include <DataMgr.h>
#include <CatMgr.h>
#include <UIResources.h>
#include <StringMgr.h>
#include <PhoneLookup.h>
#include <TraceMgr.h>
#include <AttentionMgr.h>
#include <TextMgr.h>
#include <ScrollBar.h>
#include <PenInputMgr.h>
#include <ErrorMgr.h>
#include <MemoryMgr.h>
#include <FontSelect.h>
#include <Font.h>
#include "NoteViewLib.h"
#include <UIColor.h>
#include <stdio.h>
#include <FeatureMgr.h>
#include <Preferences.h>
#include <FormLayout.h>
#include <string.h>
#include <SchemaDatabases.h>

#include "NoteViewLibRsc.h"

/***********************************************************************
 *
 *	Constants and macros
 *
 ***********************************************************************/
#define kNotePrefVersionNum				0x01
#define kNotePrefID						0x00

#define kTitleWidthMargin				10
#define kTitleHeight 					15
#define kTitleCornerDiameter2x			7
#define kTitleCornerDiameterQx			4
#define kSpaceBetweenFieldAndDoneButton	3
#define kEndOfFieldCursorPosition		0xFFFFFFFF

/***********************************************************************
 *
 *	Local statics
 *
 ***********************************************************************/
DmOpenRef 		gdbLibRef,	 		// shared library opened database ref for quick DB access purpose.
		  		gNoteDbRef; 		// note opened database ref
uint32_t		gNoteRecID,			// record ID or cursor ID that contain he note.
				gNoteRecPos,		// cursor position if gNoteRecID is a cursor ID, kEndOfFieldCursorPostion otherwise
				gNoteColumnId;		// note column ID.

FormType		*gNoteFrmP;

FontID			gNoteFont;			// font Id loaded from prefs in PrvPrefsLoad.
char			*gCustomTitleP = NULL;	// note view custom title (NULL if form default title is used)

Boolean			gSelectMode = false, // lib was called to edit note with a visible selection set.
				gModifiedNote = false,
				gEmptyNote;			 // initialised upon note loading (in PrvNoteViewLoadColumn)

uint16_t		gSelectionPos, gSelectionLen;

RectangleType 	gCurrentWinBounds;	// current window bounds.

WinHandle		gWinHdl;			// note view window handle, to check win handle upon winResizedEvent receipt.

DeleteNoteCallbackPtr gDeleteCallbackP = NULL;
static Boolean	sDeletingByCallback;

/***********************************************************************
 *   Internal Structures
 ***********************************************************************/

typedef struct NotePreferenceTag
{
	FontID noteFont;
} NotePreferenceType;


/***********************************************************************
 *
 * FUNCTION:    PrvGetGraffitiObjectIndex
 *
 * DESCRIPTION: This routine returns the Object Indew of the
 *				GSI(Graffiti shift indicator) in a Form
 *
 *				needed oto move GSI for Active Input Area Support
 *
 *
 * PARAMETERS: 	 frm FormPtr
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----		-----------
 *			ppl		02/01/02	Initial Revision
 *
 ***********************************************************************/
static uint16_t PrvNoteViewGetGraffitiObjectIndex(void)
{
	uint16_t i, howMany;

	howMany = FrmGetNumberOfObjects(gNoteFrmP);

	for( i = 0 ; i < howMany ; i ++)
	{
		if(FrmGetObjectType(gNoteFrmP, i) == frmGraffitiStateObj)
			return i;
	}

	return(uint16_t)-1;
}

/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewAdjustFieldAndScrollBarHeight
 *
 * DESCRIPTION:
 * PARAMETERS:
 *
 * RETURNED:
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ppl		01/21/02	Initial Revision
 *
 ***********************************************************************/
static void PrvNoteViewAdjustFieldAndScrollBarHeight(void)
{
	RectangleType 	objBounds,
					fieldBounds;
	uint16_t		objIndex;
	FieldType		*fieldP;
	FontID			currFont;
	Coord			fontHeight, noteDoneButtonY, x;

	// Get NoteField size and then adjust scroll bar to the same size
	fieldP =(FieldType*)FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
	FldGetBounds(fieldP, &fieldBounds);

	// Get Done button position
	FrmGetObjectPosition(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteDoneButton), &x, &noteDoneButtonY);

	// Get font height
	currFont = FntSetFont (gNoteFont);
	fontHeight = FntLineHeight();
	FntSetFont (currFont);

	// Remove the last incomplete line
	fieldBounds.extent.y = noteDoneButtonY - fieldBounds.topLeft.y - kSpaceBetweenFieldAndDoneButton;
	fieldBounds.extent.y -= fieldBounds.extent.y % fontHeight;

	// Resize the field
	FldSetBounds(fieldP, &fieldBounds);
	FldRecalculateField(fieldP, false);

	// NoteScrollBar and NoteField must have the same height
	objIndex = FrmGetObjectIndex(gNoteFrmP, NoteScrollBar);
	FrmGetObjectBounds(gNoteFrmP, objIndex, &objBounds);
	objBounds.topLeft.y = fieldBounds.topLeft.y;
	objBounds.extent.y = fieldBounds.extent.y;
	FrmSetObjectBounds(gNoteFrmP, objIndex, &objBounds);
}

/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewUpdateScrollBar
 *
 * DESCRIPTION: This routine update the scroll bar.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/01/96	Initial Revision
 *			gap	11/02/96	Fix case where field and scroll bars get out of sync
 *
 ***********************************************************************/
static void PrvNoteViewUpdateScrollBar(void)
{
	uint32_t		scrollPos,
					textHeight,
					fieldHeight,
					maxValue;
	FieldPtr		fld;
	ScrollBarPtr	bar;

	fld = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
	bar = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteScrollBar));

	FldGetScrollValues(fld, &scrollPos, &textHeight,  &fieldHeight);

	if(textHeight > fieldHeight)
	{
		// On occasion, such as after deleting a multi-line selection of text,
		// the display might be the last few lines of a field followed by some
		// blank lines.  To keep the current position in place and allow the user
		// to "gracefully" scroll out of the blank area, the number of blank lines
		// visible needs to be added to max value.  Otherwise the scroll position
		// may be greater than maxValue, get pinned to maxvalue in SclSetScrollBar
		// resulting in the scroll bar and the display being out of sync.
		maxValue =(textHeight - fieldHeight) + FldGetNumberOfBlankLines(fld);
	}
	else if(scrollPos)
		maxValue = scrollPos;
	else
		maxValue = 0;

	SclSetScrollBar(bar,(int16_t)scrollPos, 0,(int16_t)maxValue,(int16_t)(fieldHeight - 1));
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewFormResize
 *
 * DESCRIPTION: This routine set the form size and moves items
 * PARAMETERS:  none.
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----		-----------
 *			ppl		01/21/02	Initial Revision
 *
 ***********************************************************************/
static void PrvNoteViewFormResize(EventType *eventP)
{
	uint16_t		objIndex;
	Coord			x, y;
	size_t			insertionPoint,
					startPosition,
					endPosition;
	Coord			deltaX;
	Coord			deltaY;
	FieldType		*fieldP;
	RectangleType	bounds;

	// Get the delta between the old window bounding rect and the new display extent.
	deltaX = eventP->data.winResized.newBounds.extent.x - gCurrentWinBounds.extent.x;
	deltaY = eventP->data.winResized.newBounds.extent.y - gCurrentWinBounds.extent.y;

	// if the window has the height of the screen
	// then the windows is already at the right size
	if(!deltaX && !deltaY)
		return;

	if (deltaY)
	{
		// NoteDeleteButton
		objIndex = FrmGetObjectIndex(gNoteFrmP, NoteDeleteButton);
		FrmGetObjectPosition(gNoteFrmP, objIndex, &x, &y);
		y += deltaY;
		FrmSetObjectPosition(gNoteFrmP, objIndex, x, y);

		// NoteDoneButton
		objIndex = FrmGetObjectIndex(gNoteFrmP, NoteDoneButton);
		FrmGetObjectPosition(gNoteFrmP, objIndex, &x, &y);
		y += deltaY;
		FrmSetObjectPosition(gNoteFrmP, objIndex, x, y);

		// GSI
		objIndex = PrvNoteViewGetGraffitiObjectIndex();
		FrmGetObjectPosition(gNoteFrmP, objIndex, &x, &y);
		y += deltaY;
		FrmSetObjectPosition(gNoteFrmP, objIndex, x, y);
	}

	// Notefield
	objIndex = FrmGetObjectIndex(gNoteFrmP, NoteField);
	fieldP =(FieldType*)FrmGetObjectPtr(gNoteFrmP, objIndex);

	// Store initial selection & position
	insertionPoint = FldGetInsPtPosition(fieldP);
	FldGetSelection(fieldP, &startPosition, &endPosition);

	if (deltaX)
	{
		// Resize the field and the scroll bar
		FldGetBounds(fieldP, &bounds);
		bounds.extent.x += deltaX;
		FldSetBounds(fieldP, &bounds);

		objIndex = FrmGetObjectIndex(gNoteFrmP, NoteScrollBar);
		FrmGetObjectBounds(gNoteFrmP, objIndex, &bounds);
		bounds.topLeft.x += deltaX;
		FrmSetObjectBounds(gNoteFrmP, objIndex, &bounds);
	}

	PrvNoteViewAdjustFieldAndScrollBarHeight();

	// Restore selection or position
	if (startPosition != endPosition)
	{
		FldSetScrollPosition(fieldP, startPosition);
		FldSetSelection(fieldP, startPosition, endPosition);
	}
	else
	{
		if (FrmGetFocus (gNoteFrmP) != noFocus)
			FldSetScrollPosition(fieldP, insertionPoint);

		FldSetInsertionPoint(fieldP, insertionPoint);
	}

	PrvNoteViewUpdateScrollBar();

	// keep the window bounding rect.
	gCurrentWinBounds = eventP->data.winResized.newBounds;
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewScroll
 *
 * DESCRIPTION: This routine scrolls the Note View by the specified
 *					 number of lines.
 *
 * PARAMETERS:  linesToScroll - the number of lines to scroll,
 *						positive for down,
 *						negative for up
 *					 updateScrollbar - force a scrollbar update?
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 * 		Name	Date		Description
 * 		----	----		-----------
 * 		art		7/1/96	Initial Revision
 *		grant	2/2/99	Use PrvNoteViewUpdateScrollBar()
 *
 ***********************************************************************/
static void PrvNoteViewScroll(int16_t linesToScroll, Boolean updateScrollbar)
{
	uint32_t	blankLines;
	FieldPtr	fld;

	fld = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
	blankLines = FldGetNumberOfBlankLines(fld);

	if(linesToScroll < 0)
		FldScrollField(fld,(uint16_t)-linesToScroll, winUp);
	else if(linesToScroll > 0)
		FldScrollField(fld, linesToScroll, winDown);

	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	if(blankLines && linesToScroll < 0 || updateScrollbar)
		PrvNoteViewUpdateScrollBar();
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the message a page up or down.
 *
 * PARAMETERS:   direction     up or down
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   7/1/96   	Initial Revision
 *		   grant 2/2/99		Use PrvNoteViewScroll() to do actual scrolling
 *
 ***********************************************************************/
static void PrvNoteViewPageScroll(WinDirectionType direction)
{
	int32_t		linesToScroll;
	FieldPtr	fld;

	fld = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));

	if(FldScrollable(fld, direction))
	{
		linesToScroll = FldGetVisibleLines(fld) - 1;

		if(direction == winUp)
			linesToScroll = -linesToScroll;

		PrvNoteViewScroll((int16_t)linesToScroll, true);
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewSelectFont
 *
 * DESCRIPTION: This routine handles selection of a font
 *
 * PARAMETERS:  currFontID - id of current font
 *
 * RETURNED:    id of new font
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gfa		10/15/02	Initial Revision
 *
 ***********************************************************************/
static void PrvNoteViewSelectFont(FontID currFontID)
{
	FieldPtr	fldP;

	// call the OS font selector to get the id of a font.
	if((gNoteFont = FontSelect(currFontID)) != currFontID)
	{
		fldP = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
		FldSetFont(fldP, gNoteFont);
		PrvNoteViewAdjustFieldAndScrollBarHeight();
		PrvNoteViewUpdateScrollBar();
	}

	// Invalidate the form to get it redrawn
	WinInvalidateWindow(gWinHdl);
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteInitTitle
 *
 * DESCRIPTION: Set the form title
 *
 * PARAMETERS:  titleP : pointer on the form title set by the calling app.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gfa	  10/16/02	Initial Revision.
 *
 ***********************************************************************/
static void PrvNoteInitTitle(const char * titleP)
{
	Coord			titleWidth;
	int32_t 		i;

	// Set custom title only if a title was set in EditNote call
	if (titleP == NULL)
	{
		gCustomTitleP = NULL;
		return;
	}

	// First, make a copy of this title because it may be truncated
	gCustomTitleP = MemPtrNew((uint32_t)(strlen(titleP) + 1));
	strcpy(gCustomTitleP, titleP);

	// Get window width
	titleWidth = gCurrentWinBounds.extent.x - kTitleWidthMargin;

	// Truncate the title if it's too large
	FntTruncateString(gCustomTitleP, gCustomTitleP, boldFont, titleWidth, true);

	// Replace any CR / LF by ellipsis... and tabs by whitespaces
	i = 0;
	while (gCustomTitleP[i] != chrNull)
	{
		if ((gCustomTitleP[i] == chrLineFeed) || (gCustomTitleP[i] == chrCarriageReturn))
		{
			gCustomTitleP[i] = chrEllipsis;
			gCustomTitleP[i+1] = chrNull;
			break;
		}
		if (gCustomTitleP[i] == chrTab)
			gCustomTitleP[i] = chrSpace;
		i++;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvNoteDrawTitle
 *
 * DESCRIPTION: Draw the form title
 *
 * PARAMETERS:  formP : pointer on the note form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gfa	  10/16/02	Initial Revision.
 *
 ***********************************************************************/
static void PrvNoteDrawTitle(FormType * formP)
{
	Coord			x, y;
	int16_t			titleExtent, titleLen, titlePixels;
	Coord			formWidth;
	RectangleType	r;
	FontID			curFont;
	IndexedColorType curBackColor;
	IndexedColorType curTextColor;
	char * 			formTitleP;


	formTitleP = (gCustomTitleP == NULL) ? ((char*) FrmGetTitle(formP)) : gCustomTitleP;

	// Perform initial set up.
	FrmGetFormBounds(formP, &r);
	formWidth = r.extent.x;
	x = 2;
	y = 1;
	titleExtent = formWidth - 4;

	// Save/Set window colors and font.  Do this after FrmDrawForm() is called
	// because FrmDrawForm() leaves the fore/back colors in a state that we
	// don't want here.
	curBackColor = WinSetBackColor(UIColorGetTableEntryIndex(UIFormFill));
	curTextColor = WinSetTextColor(UIColorGetTableEntryIndex(UIFormFrame));
	curFont = FntSetFont(boldFont);

	// draw title...
	titleLen = strlen(formTitleP);
	titlePixels = FntCharsWidth(formTitleP, titleLen);
	x +=(titleExtent - titlePixels) / 2;

	WinSetBackColor(UIColorGetTableEntryIndex(UIFormFrame));
	WinSetTextColor(UIColorGetTableEntryIndex(UIFormFill));

	// Erase rectangle below
	r.topLeft.x = 0;
	r.topLeft.y = 0;
	r.extent.y = kTitleHeight;
	WinSetCoordinateSystem(kCoordinatesNative);
	WinScaleRectangle(&r);
	WinEraseRectangle (&r, (BmpGetDensity(WinGetBitmap(WinGetDrawWindow())) == kDensityOneAndAHalf)
								? kTitleCornerDiameterQx : kTitleCornerDiameter2x);
	WinUnscaleRectangle(&r);
	WinSetCoordinateSystem(kCoordinatesStandard);

	// Draw the title
	WinDrawChars(formTitleP, titleLen, (x < 0) ? 0 : x, y);

	// restore window colors and font.
	WinSetBackColor(curBackColor);
	WinSetTextColor(curTextColor);
	FntSetFont(curFont);
}

/***********************************************************************
 *
 * FUNCTION:    PrvNoteFreeTitle
 *
 * DESCRIPTION: Set the form title
 *
 * PARAMETERS:  None
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gfa	  10/16/02	Initial Revision.
 *
 ***********************************************************************/
static void PrvNoteFreeTitle(void)
{
	if (gCustomTitleP)
	{
		MemPtrFree(gCustomTitleP);
		gCustomTitleP = NULL;
	}
}

static void PrvPrefsLoad(void)
{
	int16_t				prefsVersion;
	uint32_t			prefsSize, dFont;
	NotePreferenceType	prefs;


	// first initialize the font with the system default one.
	gNoteFont = (FontID)((FtrGet(sysFtrCreator, sysFtrDefaultFont, &dFont) >= errNone) ? dFont : stdFont);

	// Read the preferences / saved-state information.
	// New ARM version. Impossible to read previous versions, if any.
	prefsSize = (uint32_t)sizeof(NotePreferenceType);
	MemSet(&prefs, prefsSize, 0);
	prefsVersion = PrefGetAppPreferences(sysFileCNoteViewLib, kNotePrefID, &prefs, &prefsSize, true);

	if (prefsVersion == kNotePrefVersionNum)
		gNoteFont = prefs.noteFont;
}

static void PrvPrefsSave(void)
{
	NotePreferenceType	prefs;

	DbgOnlyFatalErrorIf(gNoteFont > largeBoldFont, "Note font invalid");

	// write the preferences / saved-state information.
	prefs.noteFont				= gNoteFont;

	// write the state information.
	PrefSetAppPreferences(sysFileCNoteViewLib, kNotePrefID, kNotePrefVersionNum, &prefs, sizeof(NotePreferenceType), true);
}

/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *       	art   	6/5/95   	Initial Revision
 *			jmp		9/8/99		Moved the gNoteFontCmd case to where it is in this
 *								routine in all the other NoteView implementations.
 *			jmp		9/17/99		Eliminate the goto top/bottom of page menu items
 *								as NewNoteView no longer supports them.
 *
 ***********************************************************************/
static Boolean PrvNoteViewDoCommand(uint16_t command)
{
	Boolean	handled = false;

	switch(command)
	{
		case newNoteFontCmd:
			PrvNoteViewSelectFont(gNoteFont);
			handled = true;
			break;

		case newNotePhoneLookupCmd:
			PhoneNumberLookup(FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField)));
			handled = true;
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewDeleteNote
 *
 * DESCRIPTION: Deletes the note field from the current record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    note was modified (wasn't empty when loaded).
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         gfa   10/14/02   Initial Revision
 *
 ***********************************************************************/
static Boolean PrvNoteViewDeleteNote(void)
{
	status_t	err;
	uint32_t	size = 0;

	if (DbCopyColumnValue(gNoteDbRef, gNoteRecID, gNoteColumnId, 0, NULL, &size) >= errNone)
	{
		err = DbWriteColumnValue(gNoteDbRef, gNoteRecID, gNoteColumnId, 0, -1, NULL, 0);
		ErrFatalDisplayIf(err < errNone, "Couldn't delete column value");
	}

	return !gEmptyNote;
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewLoadColumn
 *
 * DESCRIPTION: Load the record's note field into the field object
 * for editing.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			gfa			10/14/02	Initial Revision.
 *
 ***********************************************************************/
static void PrvNoteViewLoadColumn(void)
{
	FieldType *			fieldP;
	uint32_t			noteSize;
	status_t			err;

	// get a pointer to the memo field.
	fieldP = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));

	// set the font used in the memo field.
	FldSetFont(fieldP, gNoteFont);

	// get note.
	noteSize = 0;
	gEmptyNote = false;

	FldSetTextColumn (fieldP, gNoteDbRef, gNoteRecID, gNoteRecPos, gNoteColumnId, 0);

	err = DbCopyColumnValue(gNoteDbRef,  gNoteRecID, gNoteColumnId, 0, NULL, &noteSize);
	ErrFatalDisplayIf(err < errNone && err != dmErrNoColumnData, "Couldn't open note");

	if (!noteSize)
		gEmptyNote = true;
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewSaveColumn
 *
 * DESCRIPTION: This routine
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gfa	  10/15/02  initial revision.
 *
 ***********************************************************************/
static Boolean PrvNoteViewSaveColumn(void)
{
	FieldPtr 	fld;
	uint32_t	textLength;
	Boolean		modified = false;

	fld = FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));

	// if the field wasn't modified then don't do anything
	modified = FldDirty(fld);

	textLength = FldGetTextLength(fld);

	FldReleaseStorage(fld);

	// empty fields are not allowed because they cause problems
	if(!textLength)
		return PrvNoteViewDeleteNote();

	return modified;
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewInit
 *
 * DESCRIPTION: This routine initials the Edit View form.
 *
 * PARAMETERS:  frm - pointer to the Active form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *	        	----	----		-----------
 *				art   	6/5/95		Initial Revision
 *				jmp		9/23/99		Eliminate code to hide unused font controls
 *									now that we're using a NoteView form that doesn't
 *									have them anymore.
 *				peter	09/20/00	Disable attention indicator because title is custom.
 *				gfa		10/14/02	updated to create a separate shared lib for note view.
 *
 ***********************************************************************/
static void PrvNoteViewInit(void)
{
	FieldPtr	fld;
	uint16_t	fldIndex;

	gWinHdl = FrmGetWindowHandle(gNoteFrmP);

	// select the font.
	FntSetFont(gNoteFont);

	// keep the window bounding rect.
	FrmGetFormInitialBounds(gNoteFrmP, &gCurrentWinBounds);

	PrvNoteViewLoadColumn();

	// have the field send events to maintain the scroll bar.
	fldIndex = FrmGetObjectIndex(gNoteFrmP, NoteField);
	fld = FrmGetObjectPtr(gNoteFrmP, fldIndex);

	// Adjust the field bounds
	PrvNoteViewAdjustFieldAndScrollBarHeight();

	// if select mode, show selection.
	if (gSelectMode)
	{
		FldSetScrollPosition(fld, gSelectionPos);
		FldSetSelection(fld, gSelectionPos, (uint16_t)(gSelectionPos + gSelectionLen));
	}
	else
	{
		FldSetInsertionPoint(fld, kEndOfFieldCursorPosition);
	}
	sDeletingByCallback = false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the NoteView.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			gfa		10/15/02		huge modification for separate shared lib.
 *
 ***********************************************************************/
static Boolean PrvNoteViewHandleEvent(EventType *eventP)
{
	FieldType * fieldP = NULL;
	Boolean		handled = false;

	switch(eventP->eType)
	{
		case menuEvent:
 			handled = PrvNoteViewDoCommand(eventP->data.menu.itemID);
 			break;

		case winUpdateEvent:
			if (eventP->data.winUpdate.window != gWinHdl)
				break;

			if (sDeletingByCallback)
			{
				fieldP =(FieldType*)FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
				FldSetTextColumn (fieldP, gNoteDbRef, gNoteRecID, gNoteRecPos, gNoteColumnId, 0);
			}

			// Drawings...
			FrmDrawForm(gNoteFrmP);

			if (eventP->data.winUpdate.dirtyRect.topLeft.y <= kTitleHeight)
				PrvNoteDrawTitle(gNoteFrmP);

			if (sDeletingByCallback)
			{
				FldReleaseFocus(fieldP);
				FldReleaseStorage(fieldP);
			}
/*
			// Set the focus
			if (FrmGetFocus(gNoteFrmP) == noFocus)
				FrmSetFocus(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
*/
			handled = true;
			break;

		case keyDownEvent:
			if(EvtKeydownIsVirtual(eventP))
			{
				switch(eventP->data.keyDown.chr)
				{
					case vchrPageUp:
						PrvNoteViewPageScroll(winUp);
						handled = true;
						break;

					case vchrPageDown:
						PrvNoteViewPageScroll(winDown);
						handled = true;
						break;
				}
			}
			break;


		case fldChangedEvent:
			PrvNoteViewUpdateScrollBar();
			handled = true;
			break;

		case sclRepeatEvent:
			PrvNoteViewScroll((int16_t)(eventP->data.sclRepeat.newValue - eventP->data.sclRepeat.value), false);
			break;

		case winResizedEvent:
			if (gWinHdl != eventP->data.winResized.window)
				break;

			if (sDeletingByCallback)
			{
				fieldP =(FieldType*)FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
				FldSetTextColumn (fieldP, gNoteDbRef, gNoteRecID, gNoteRecPos, gNoteColumnId, 0);
			}

			PrvNoteViewFormResize(eventP);

			if (sDeletingByCallback)
			{
				FldReleaseFocus(fieldP);
				FldReleaseStorage(fieldP);
			}

			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case NoteDeleteButton:
					fieldP =(FieldType*)FrmGetObjectPtr(gNoteFrmP, FrmGetObjectIndex(gNoteFrmP, NoteField));
					// setting handled to false when delete is confirmed will cause the dialog to close
					handled = true;
					if (gDeleteCallbackP)
					{
						// Release the focus to unlock the record (because of edit-in-place)
						FldReleaseFocus(fieldP);
						FldReleaseStorage(fieldP);
						sDeletingByCallback = true;

						if ((*gDeleteCallbackP)(gNoteDbRef, gNoteRecID, gNoteRecPos, gNoteColumnId))
						{
							// The user accepted to delete, the callback removed the column by itself, just exit now
							gModifiedNote = true;
							handled = false;
						}
						else
						{
							FldReturnStorage(fieldP);
							// The user cancel the delete, restore the editing state
							FldGrabFocus(fieldP);
						}

						sDeletingByCallback = false;
					}
					else
					{
						if (FrmAlert(gdbLibRef, DeleteNoteAlert) != DeleteNoteYes)
							break;

						FldReleaseStorage(fieldP);
						gModifiedNote = PrvNoteViewDeleteNote();
						handled = false;
					}
					break;
			}
			break;

		default:
			break;
	}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    EditNote, EditNoteSelection
 *
 * DESCRIPTION: This is the API for editing a note/note selection.
 *
 * PARAMETERS:  title: title of the note form
 *				dbRef: database open reference for the note db
 *				recordIndex: index of the record that contains the note
 *				columnId: column ID for the note in the schema
 *				selectionPos (EditNoteSelection only): selection start offset.
 *				selectionLen (EditNoteSelection only): selection len.
 *
 * RETURNED:    true if note was modified, else false.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			gfa		10/15/02		initial revision
 *
 ***********************************************************************/
Boolean EditNoteSelection(const char *title, DmOpenRef dbRef, uint32_t rowID, uint32_t rowPos,
	uint32_t columnId, uint16_t selectionPos, uint16_t selectionLen, DeleteNoteCallbackPtr deleteCallbackP)
{
	Boolean modified = false;

	gSelectMode = true;
	gSelectionPos = selectionPos;
	gSelectionLen = selectionLen;
	modified = EditNote(title, dbRef, rowID, rowPos, columnId, deleteCallbackP);
	gSelectMode = false;

	return modified;
}

Boolean EditNote(const char *title, DmOpenRef dbRef, uint32_t rowID, uint32_t rowPos,
	uint32_t columnId, DeleteNoteCallbackPtr deleteCallbackP)
{

	// keep open db, record index, column ID and initialize note buffer handle and font.
	gNoteDbRef = dbRef;
	gNoteRecID = rowID;
	gNoteColumnId = columnId;
	gNoteFont = stdFont;
	gNoteRecPos = DbIsCursorID(rowID) ? rowPos : 0 ;

	// set delete callback to be used from noteView event handler
	gDeleteCallbackP = deleteCallbackP;

	// load the preferences.
	PrvPrefsLoad();

	// init form and change its title if a valid title was passed.
	gNoteFrmP = FrmInitForm(gdbLibRef, NewNoteView);
	ErrFatalDisplayIf((!gNoteFrmP), "NoteView Form not found");

	// initialize the view, title, and scrollbar.
	PrvNoteViewInit();
	PrvNoteInitTitle(title);
	PrvNoteViewUpdateScrollBar();

	// open the form.
	FrmSetEventHandler(gNoteFrmP, PrvNoteViewHandleEvent);

	// until the done button is hit.
	if(FrmDoDialog(gNoteFrmP) == NoteDoneButton)
		gModifiedNote = PrvNoteViewSaveColumn();

	// we're done with the note.
	PrvNoteFreeTitle();
	FrmDeleteForm(gNoteFrmP);

	// save the preferences.
	PrvPrefsSave();

	return gModifiedNote;
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the library.
 *
 * PARAMETERS:  cmd - word value specifying the launch code.
 *              cmdPB - pointer to a structure that is associated with the launch code.
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			gfa		10/15/02		initial revision
 *
 ***********************************************************************/
uint32_t PilotMain(uint16_t cmd, MemPtr cmdPBP, uint16_t launchFlags)
{
	switch(cmd)
	{
	case sysLaunchCmdInitialize:
		// open 'this' to access 'its' resources
		gdbLibRef = DmOpenDatabaseByTypeCreator(sysFileTLibrary, sysFileCNoteViewLib, dmModeReadOnly);
		ErrFatalDisplayIf(!gdbLibRef, "Unable to open note view database");
		break;

	case sysLaunchCmdFinalize:
		DbCloseDatabase(gdbLibRef);
		break;
	}

	return 0;
}
