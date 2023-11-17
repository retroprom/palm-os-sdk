/******************************************************************************
 *
 * Copyright (c) 1999-2004 PalmSource, Inc. All rights reserved.
 *
 * File: GUIUtilities.c
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifdef DEBUGAPPLIB_ENABLED
#include <DebugAppLib.h>
#endif

#include <PalmTypes.h>

#include <Bitmap.h>
#include <Chars.h>
#include <Control.h>
#include <DataMgr.h>
#include <ErrorMgr.h>
#include <Form.h>
#include <Field.h>
#include <List.h>
#include <Loader.h>
#include <MemoryMgr.h>
#include <NotifyMgr.h>
#include <Preferences.h>
#include <string.h>
#include <StringMgr.h>
#include <SystemResources.h>
#include <TextMgr.h>
#include <TraceMgr.h>
#include <UIResources.h>

#include "GUIUtilities.h"


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesMapToPositionByte
 *
 * DESCRIPTION:		Map a value to it's position in an array.  If the passed
 *					value is not found in the mappings array, a default
 *					mappings item will be returned.
 *
 * PARAMETERS:  ->	mappingArray:	The mappings array.
 *				->	value:			Value to look for
 *				-> 	mappings:		Number of item in the mappings array.
 *				->	defaultItem:	value to be returned when nothing is found.
 *
 * RETURNED:    	Position of value found.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	KCR	9/13/95		Initial revision.
 *
 ******************************************************************************/
uint16_t UtilitiesMapToPositionByte (uint8_t *mappingArray, uint8_t value, uint16_t mappings, uint16_t defaultItem)
{
	uint16_t i;
	
	i = 0;
	while (mappingArray[i] != value && i < mappings)
		i++;
		
	if (i >= mappings)
		return defaultItem;

	return i;
}	//	end of MapToPositionByte


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGetValueFromByteListRsc
 *
 * DESCRIPTION:		Given an index into a byte list resource, return the value.
 *
 * PARAMETERS:	->	dbRef:		Resource database reference.
 *				->	listRscID:	Resource id of the list.
 *				->	selection:	Index into the byte list, or noListSelection 
 *								for default.
 *
 * RETURNED:    	Byte value for the given position.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	VMK	8/7/96		Initial revision.
 *
 ******************************************************************************/
uint8_t UtilitiesGetValueFromByteListRsc(DmOpenRef dbRef, int16_t listRscID, int16_t selection)
{
	uint8_t				value = 0;
	ByteListRscType*	listP;
	MemHandle			listH;
	
	
	listH = DmGetResource(dbRef, byteListRscType, listRscID);
	
	ErrNonFatalDisplayIf((listH == NULL), "resource not found");

	if (listH)
	{
		listP = (ByteListRscType*) MemHandleLock(listH);
		
		if ( selection == noListSelection )
			selection = listP->defItem - 1;

	
		ErrNonFatalDisplayIf((selection >= listP->count), "bad index");
	
		value = listP->item[selection];
		
		MemHandleUnlock(listH);
		
		DmReleaseResource(listH);
	}

	return( value );
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesMapValueInByteListRsc
 *
 * DESCRIPTION:		Given a byte value, return the index into the byte list 
 *					resource.
 *
 * PARAMETERS:	->	dbRef:		Resource database reference.
 *				->	listRscID:	Resource id of the list.
 *				->	value:		Value to map.
 *
 * RETURNED:    	Position of the given value.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	VMK	8/5/96		Initial revision.
 *
 ******************************************************************************/
uint16_t UtilitiesMapValueInByteListRsc(DmOpenRef dbRef, int16_t listRscID, uint8_t value)
{
	uint16_t			position = 0;
	ByteListRscType*	listP;
	MemHandle			listH;
	
	
	listH = DmGetResource( dbRef, byteListRscType, listRscID );
	
	ErrNonFatalDisplayIf((listH == NULL), "byteListRscType not found");

	if (listH)
	{
		listP = (ByteListRscType*) MemHandleLock(listH);
	
		position = UtilitiesMapToPositionByte(listP->item, value, listP->count, (uint16_t) (listP->defItem - 1));
	
		MemHandleUnlock(listH);
		DmReleaseResource(listH);
	}

	return( position );
}


/******************************************************************************
 *
 * FUNCTION: 		UtilitiesSkipLeadingSpace
 *
 * DESCRIPTION:		Utility routine for stipping leading white space from 
 *					a string.
 *
 * PARAMETERS:	->	strP:	Pointer to string.
 *
 * RETURNED:		Pointer to the rest of the string after leading spaces has 
 *					been stripped.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	VMK	8/13/96		Initial Version.
 *	KWK	05/19/99	Modify for int'l.
 *
 ******************************************************************************/
char * UtilitiesSkipLeadingSpace(char * strP)
{
	wchar32_t curChar = 0;
	uint16_t charSize = 0;

	// Skip leading space characters
	
	while ( *strP )
	{
		charSize = TxtGetNextChar(strP, 0, &curChar);
		
		if (! TxtCharIsSpace(curChar) )
			break;
		
		strP += charSize;
	}
	
	return ( strP );
}


/******************************************************************************
 *
 * FUNCTION: 		UtilitiesRemoveControlChars
 *
 * DESCRIPTION:		Utility routine for removing any Control Characters
 *					from the StrP string.
 *
 * PARAMETERS:		strP:	ptr to string
 *
 * RETURNED:		Pointer to the rest of the string after which space has 
 *					been stripped.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ppl	09/22/00	Initial revision.
 *			
 ******************************************************************************/
char * UtilitiesRemoveControlChars(char * strP)
{
	char* 		stringP;
	char* 		sourceP;
	char*		destinationP;
	uint16_t 	stringSize;
	uint16_t	charSize;
	wchar32_t	curChar;

	stringSize = strlen(strP);
	stringP = (char*) MemPtrNew((uint32_t)(stringSize + 1));
	
	if (stringP)
	{
		sourceP = strP;
		destinationP = stringP;
		
		while ( *sourceP )
		{
			charSize = TxtGetNextChar(sourceP, 0, &curChar);
			
			if (!TxtCharIsCntrl(curChar))
			{
				memmove(destinationP, sourceP, charSize);
				destinationP += charSize;
			}
			sourceP += charSize;
		}
		
		TxtSetNextChar(destinationP, 0, chrNull);
		
		strcpy(strP, stringP);
		
		MemPtrFree(stringP);
	}
	
	return strP;

}





/******************************************************************************
 *
 * FUNCTION:    	UtilitiesGetFormObjectPtr
 *
 * DESCRIPTION: 	This routine returns a pointer to an object in the current
 *              	form.
 *
 * PARAMETERS:  ->	formP:		The form's pointer.
 *				->	objectID:	ID of the object.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		6/5/95	Initial revision.
 *
 ******************************************************************************/
void* 	UtilitiesGetFormObjectPtr (FormType* formP, uint16_t objectID)
{
	return (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, objectID)));
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesGetObjectPtr
 *
 * DESCRIPTION: 	This routine returns a pointer to an object in the current
 *              	form.
 *
 * PARAMETERS:  ->	formId:		id of the form to display.
 *				->	objectId: 	id of object to retriev a pointer on.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART	6/5/95		Initial revision.
 *	PPL	06/29/04 	Add formId.
 *
 ******************************************************************************/
void * UtilitiesGetObjectPtr (uint16_t formId, uint16_t objectId)
{
	return UtilitiesGetFormObjectPtr(FrmGetFormPtr(formId), objectId);
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesGetFormObjectPosition
 *
 * DESCRIPTION: 	This routine returns the position of a form object.
 *
 * PARAMETERS:  ->	formP: 	The form.
 *				->	objectID:	ID of the object.
 *				<-	x		x topleft coordinate;
 *				<-	y: 		y topleft coordinate;
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		6/5/95	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGetFormObjectPosition (FormType* formP, uint16_t objectID, Coord* x, Coord* y)
{
	FrmGetObjectPosition (formP, FrmGetObjectIndex (formP, objectID), x, y);
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesGetFormObjectBounds
 *
 * DESCRIPTION: 	This routine returns the bounds of a form object.
 *
 * PARAMETERS:  ->	formP: 	The form.
 *				->	objectID:	ID of the object.
 *				->	bounds: The object's bounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		6/5/95	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGetFormObjectBound (FormType* formP, uint16_t objectID, RectangleType* bounds)
{
	FrmGetObjectBounds (formP, FrmGetObjectIndex (formP, objectID), bounds);
}


/******************************************************************************
 *
 * FUNCTION:   		UtilitiesFormHideObjectID
 *
 * DESCRIPTION: 	This routine hides an object in the form given its ID.
 *
 * PARAMETERS: 	->	formP:		 The Form's pointer.
 *				->	objectID:	 ID of the UI Item to hide.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/04/00	Initial revision.
 *
 ******************************************************************************/
void UtilitiesFormHideObjectID (FormType* formP, uint16_t objectID)
{
	FrmHideObject(formP, FrmGetObjectIndex (formP, objectID));
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesFormShowObjectID
 *
 * DESCRIPTION: 	This routine shows an object in the frome given its ID.
 *
 * PARAMETERS: 	->	formP:		 The Form's pointer.
 *				->	objectID:	 ID of the UI Item to show.
 *
 * RETURNED:    	Nothing
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/04/00	Initial revision.
 *
 ******************************************************************************/
void UtilitiesFormShowObjectID (FormType* formP, uint16_t objectID)
{
	FrmShowObject(formP, FrmGetObjectIndex (formP, objectID));
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesSetFormObjectPosition
 *
 * DESCRIPTION: 	This routine sets the position of a form object.
 *
 * PARAMETERS:  ->	formP: 		The form.
 *				->	objectID:	ID of the object.
 *				->	x			x topleft coordinate;
 *				->	y: 			y topleft coordinate;
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		6/5/95	Initial revision.
 *
 ******************************************************************************/
void UtilitiesSetFormObjectPosition (FormType* formP, uint16_t objectID, Coord x, Coord y)
{
	FrmSetObjectPosition (formP, FrmGetObjectIndex (formP, objectID), x, y);
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesSetFormObjectBounds
 *
 * DESCRIPTION: 	This routine sets the bounds of a form object.
 *
 * PARAMETERS:  ->	formP: 	The form.
 *				->	objectID:	ID of the form to display.
 *				->	bounds: The object's bounds.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ART		6/5/95	Initial revision.
 *
 ******************************************************************************/
void UtilitiesSetFormObjectBound (FormType* formP, uint16_t objectID, RectangleType* bounds)
{
	FrmSetObjectBounds (formP, FrmGetObjectIndex (formP, objectID), bounds);
}


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesMoveFormObject
 *
 * DESCRIPTION:		Offset an object within a form.
 *
 * PARAMETERS:  ->	frm:		Pointer to form.
 *				->	objectID:	ID of the object.
 *				->	deltaX: 	X-axis distance to move.
 *				->	deltaY: 	Y-axis distance to move.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	GPE 	1/7/97	Initial revision.
 *
 ******************************************************************************/
void UtilitiesMoveFormObject(
	FormType* 	formP, 
	uint16_t 	objectID, 
	Coord 		deltaX, 
	Coord 		deltaY, 
	Boolean 	redraw)
{
	uint16_t 	index;
	Coord 		x;
	Coord		y;
	
	index = FrmGetObjectIndex(formP, objectID);
	
	if (redraw) 
		FrmHideObject(formP,index);
	
	FrmGetObjectPosition(formP, index, &x, &y);
	
	x += deltaX;
	y += deltaY;
	
	FrmSetObjectPosition(formP, index, x, y);
	
	if (redraw) 
		FrmShowObject(formP, index);
}	


/******************************************************************************
 *
 * FUNCTION:    	UtilitiesFormSetList
 *
 * DESCRIPTION:		Put the List item text inside the Pop Selectector trigger text
 *					works only for self data contained list.
 *
 * PARAMETERS:  ->	listResID:		The object id of the list.
 *				->	triggerResID:	The object id of the trigger.
 *				->	selection:		Index of item to put inside 
 *									the list to the trigger.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL 10/12/00	Initial revision.
 *
 ******************************************************************************/
void UtilitiesFormSetList(uint16_t listResID, uint16_t triggerResID, int16_t selection)
{
	FormType*	formP;
	ListType* 	listP;

	formP = FrmGetActiveForm();
	
	listP = (ListType*) UtilitiesGetFormObjectPtr( formP, listResID );
	
	LstSetSelection(listP, selection);

	CtlSetLabel((ControlType*) UtilitiesGetFormObjectPtr(formP, triggerResID), LstGetSelectionText(listP, selection));
}

/******************************************************************************
 *
 * FUNCTION:		UtilitiesSetModemText
 *
 * DESCRIPTION:		Sets the Modem text for the Modem String Selector
 * 					If the string passed is null then ("Tap to enter string") 
 *					will be displayed in its place.  The string will be 
 *					truncated and three ellipses will be displayed when the 
 *					string length exceeds the display area. 
 *
 *					The string passed into this function is not modified at
 *					any time.
 * 
 * PARAMETERS:	->	string:	String to be copied to the selector label.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	ADH	7/29/98		Adapted from Memo Pad title draw algorithm.
 *
 ******************************************************************************/
char* UtilitiesTruncateStringWithEllipses(const char* stringP, char* outputP, Coord desiredWidth)
{
	Coord			stringWidth;
	size_t			stringLength;
	Boolean 		stringFit; 
	size_t			charsToDraw;	
	wchar32_t		theChar;
	wchar32_t		thePreviousChar;
	size_t			theCharSize;	
	size_t			thePreviousCharSize;		
		
	charsToDraw = strlen(stringP);
		
	stringWidth = desiredWidth;
	stringLength = charsToDraw;
	
	FntCharsInWidth ( stringP, &stringWidth, &stringLength, &stringFit);

	if (stringFit)
	{
		// in this case, the string is left unchanged
		outputP = (char*) stringP;
	}
	else
	{
		// Measuring text character-by-character doesn't work for contextual writing systems.
		desiredWidth -= FntCharWidth(chrEllipsis);
		
		// stringLength could be 0 if we have a string as '\n'AB
		// in such cases FntCharsInWidth returns 0 in stringLength
		if (stringLength > 0)
		{
			thePreviousCharSize = TxtGetPreviousChar(stringP, stringLength, &thePreviousChar);
	
			while ((stringWidth > desiredWidth) 
				|| TxtCharIsSpace(thePreviousChar)  
				|| (thePreviousChar == chrHorizontalTabulation))
			{
				theCharSize = TxtGetPreviousChar(stringP, stringLength, &theChar);
				
				stringLength -= theCharSize;

				stringWidth -= FntCharWidth (theChar);
				
				thePreviousCharSize = TxtGetPreviousChar(stringP, stringLength, &thePreviousChar);
			}
	
			StrNCopy(outputP, stringP, stringLength);
		}

		// Add ellipsis
		outputP[TxtSetNextChar(outputP, stringLength, chrEllipsis)] = chrNull;
	}
	
	return outputP;
}


// Field Utilities

/******************************************************************************
 *
 * FUNCTION:		UtilitiesSetFieldPtrText
 *
 * DESCRIPTION:		Set field object's text MemHandle.  Will reuse an existing
 *					text MemHandle, if any
 *
 * PARAMETERS:	->	fieldP:	Field Ptr.
 *				->	srcP:	Source text pointer.
 *				->	append:	If true, the new text will be appended.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	VMK	10/3/95		Initial revision.
 *
 ******************************************************************************/
void UtilitiesSetFieldPtrText(FieldType* fieldP, char * srcP, Boolean append)
{
	MemHandle 	stringH;
	char * 		stringP;
	uint16_t	oldLength;
	uint16_t	newSize;


	oldLength = 0;
	newSize = strlen(srcP) + 1;							// initialize new size
	

	stringH = FldGetTextHandle( fieldP );				// get the current text MemHandle
	FldSetTextHandle( fieldP, NULL );					// release this MemHandle from field

	// Resize the existing MemHandle, if any
	if ( stringH )
	{
		if ( append )
		{
			stringP = (char*) MemHandleLock( stringH );
			
			oldLength = strlen(stringP);
			newSize += oldLength;
			
			MemHandleUnlock( stringH );
		}
		if ( MemHandleResize(stringH, newSize) )
		{
			goto Exit;
		}
	} // Resize the existing MemHandle, if any
	
	// Otherwise, allocate a new MemHandle
	else
	{
		stringH = MemHandleNew( newSize );		// allocate a new chunk
		if ( !stringH )	
		{
			return;
		}
	}

	if (stringH)
	{
		// Append the new text
		stringP = (char*) MemHandleLock( stringH );
		strcpy( stringP + oldLength, srcP );		// copy the new text
		MemHandleUnlock( stringH );
	}
	
Exit:
	if (stringH)
	{
		FldSetTextHandle( fieldP, stringH );			// set the new text MemHandle
		FldRecalculateField(fieldP, false);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGetFieldPtrText
 *
 * DESCRIPTION:	Copies text from a field to a destination buffer.
 *
 * PARAMETERS:	->	fieldP:		Field Ptr.
 *				->	bufSize:	Buffer size (actually the number of Chars
 *								NULL end not included.)
 *				->	destP:		Dest text buffer pointer.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	VMK		10/3/95	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGetFieldPtrText(FieldType* fieldP, uint16_t bufSize, char * destP)
{
	char * 			stringP;
	
	destP[0] = '\0';
	stringP = (char*) FldGetTextPtr( fieldP);
	
	if ( stringP )
	{
		StrLCopy( destP, stringP, bufSize );
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGetFieldText
 *
 * DESCRIPTION:		Copies text from a field to a destination buffer.
 *
 * PARAMETERS:	->	fldID:		Field object id.
 *				->	bufSize:	Buffer size (actually the number of Chars
 *								NULL end not included.)
 *				->	destP:		Dest text buffer pointer.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/3/20		Initial revision.
 *
 ******************************************************************************/
void UtilitiesGetFieldText(uint16_t fldID, uint16_t bufSize, char * destP)
{
	FormPtr formP = FrmGetActiveForm();
	UtilitiesGetFieldPtrText((FieldType*) UtilitiesGetFormObjectPtr(formP, fldID), bufSize,  destP);
}
 

/******************************************************************************
 *
 * FUNCTION:		UtilitiesResizeEditField
 *
 * DESCRIPTION:		Resizes the visible size of a Dynamic Text Edit Field
 *					for example as a response to a fldHeightChangedEvent.
 *
 * PARAMETERS:	->	fieldP:			Field to resize.
 *				->	maxNumLines:	Maximum number of lines in the edit field.
 *				->	newHeight:		NewHeight for the field.
 *				->	currentPos:		Current insertion point position.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/12/00	Initial revision.
 *	KWK	11/06/00	Call FldSetInsPtPosition to get the blinking
 *					insertion point.
 *
 ******************************************************************************/
void UtilitiesResizeEditField (FieldType* fieldP, uint16_t maxNumLines, uint16_t newHeight, uint16_t currentPos)
{
	RectangleType	fieldR;
	RectangleType	newBounds;
	FontID			currFont;
	uint16_t		maxHeight;
	int16_t			scrollY;
	
	
	// Get the current height of the field;

	FldGetBounds (fieldP, &fieldR);
	RctCopyRectangle (&fieldR, &newBounds);

	// calculate distance to scroll objects (signed value)
	scrollY = newHeight;
	scrollY -= fieldR.extent.y;
	
	if (!scrollY)
	{
		return; // nothing to do;
	}
	
	
	// Is the field's height contracting?
	if (newHeight < fieldR.extent.y)
	{
		
		newBounds.extent.y = newHeight;
		FldEraseField(fieldP);
	}
	else // It is expanding
	{
		currFont = FntSetFont (FldGetFont(fieldP));
		maxHeight =  maxNumLines * FntLineHeight ();
		FntSetFont (currFont);
               
        // do not allow the field to grow beyond maxNetFieldLines 
		if (newHeight > maxHeight)
		{
			newHeight = scrollY = maxHeight;
		}
		
		newBounds.extent.y = newHeight;
	}
	
	// Resize the field
	FldSetBounds (fieldP, &newBounds);

	// For the case where the field is expanding, we need to make
	// sure the x/y position (if visible) is set properly, otherwise
	// the insertion point won't blink. At this point, even if the
	// insertion point is on a visible line, the field object might
	// still have it marked as not visible (probably a FldSetBounds
	// design flaw/bug).
	FldSetInsPtPosition (fieldP, currentPos);
	
	FldDrawField (fieldP);
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesUpdateScrollBar
 *
 * DESCRIPTION:		Update the Scrollbar
 *						
 *
 * PARAMETERS:	->	fieldP:		Field Ptr.
 *				->	scrollP:	Scroll bar ptr.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/06/00	Initial revision.
 *
 ******************************************************************************/
void UtilitiesUpdateScrollBar(FieldType* fieldP, ScrollBarType* scrollP)
{
	uint32_t scrollPos;
	uint32_t textHeight;
	uint32_t fieldHeight;
	uint32_t maxValue;

	TraceOutput(TL(appErrorClass, "UtilitiesUpdateScrollBar: Processing..."));
		
	FldGetScrollValues (fieldP, &scrollPos, &textHeight, &fieldHeight);
	
	if (textHeight > fieldHeight)
	{
	    maxValue = textHeight - fieldHeight;
	}
	else if (scrollPos)
	{
	    maxValue = scrollPos;
	}
	else
	{
	    maxValue = 0;
	}
	
	SclSetScrollBar (scrollP, (int16_t) scrollPos, 0, (int16_t) maxValue, (int16_t) (fieldHeight-1));
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesScrollScrollBar
 *
 * DESCRIPTION:		Handle event related to the hard keys for a field
 *					and its scroll text MemHandle, if any.
 *
 * PARAMETERS:	->	scrollID:			Object ID of the scroll.
 *				->	linesToScroll:		Number of lines to scroll.
 *										(sign for the direction.)
 *				->	updateScrollbar:	If true, need to draw things
 *				->	fldID:				related field Object ID.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/06/00	Initial revision.
 *
 ******************************************************************************/
void UtilitiesScrollScrollBar(uint16_t scrollID, int16_t linesToScroll, Boolean updateScrollbar, uint16_t fldID)
{
	uint32_t			blankLines;
	FormType*			formP;
	FieldType*			fieldP;
	ScrollBarType*   	scrollP;


	formP = FrmGetActiveForm();
	
	scrollP 	= (ScrollBarType *) UtilitiesGetFormObjectPtr (formP, scrollID);
	fieldP 		= (FieldType *) UtilitiesGetFormObjectPtr (formP, fldID);
	
	blankLines 	= FldGetNumberOfBlankLines (fieldP);

	if (linesToScroll < 0)
	{
		FldScrollField (fieldP, (uint32_t) -linesToScroll, winUp);
	}
	else if (linesToScroll > 0)
	{
		FldScrollField (fieldP, linesToScroll, winDown);
	}

	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	if (blankLines || updateScrollbar)
	{
		ErrNonFatalDisplayIf(blankLines && linesToScroll > 0, "blank lines when scrolling winDown");
		
		UtilitiesUpdateScrollBar(fieldP, scrollP);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesHandleTextScrollWithKey
 *
 * DESCRIPTION:		Handle event related to the hard keys for a field
 *					and its scroll text MemHandle, if any.
 *
 * PARAMETERS:	->	event:			event record.
 *				->	textID:			source text pointer.
 *				->	scrollBarID:	if true, the new text will be appended.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	10/06/00	Initial revision.
 *
 ******************************************************************************/
Boolean UtilitiesHandleTextScrollWithKey(EventType* event, uint16_t fieldID, uint16_t scrollID) 
{
	FormType*			formP;
	FieldType*			fieldP;
	ScrollBarType*		scrollP;
	uint32_t			lines;
	Boolean				handled = false;
	WinDirectionType	direction = winUp;

	formP = FrmGetActiveForm();
	
	scrollP 	= (ScrollBarType*) UtilitiesGetFormObjectPtr (formP, scrollID);
	fieldP 		= (FieldType*) UtilitiesGetFormObjectPtr (formP, fieldID);
	lines 		= FldGetVisibleLines(fieldP);

	if (lines > 1)
	{
		lines--;
	}
		
	if (event->data.keyDown.chr == pageUpChr)
	{
		handled = true;
	}

	//	Scroll down key pressed?
	if (event->data.keyDown.chr == pageDownChr)
	{
		direction = winDown;
		handled = true;
	}
	
	if (handled)
	{
		FldScrollField(fieldP, lines, direction);		
		UtilitiesUpdateScrollBar(fieldP, scrollP);
	}
	
	return handled;
}



/******************************************************************************
 *
 * FUNCTION:		UtilitiesSendSimpleEvent
 *
 * DESCRIPTION:		Send a simple event to the application.
 *
 * PARAMETERS:		eType: event type to send.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPL	03/11/02	Initial revision.
 *
 ******************************************************************************/

void UtilitiesSendSimpleEvent(eventsEnum eType)
{
	EventType	newEvent;
	
	// By simply exiting we return to the prior which called us.

	memset (&newEvent, 0x00, sizeof(EventType));
	
	newEvent.eType = eType;
	
	EvtAddEventToQueue(&newEvent);
}


// Graffiti Shift utilities

/***********************************************************************
 *
 * FUNCTION:    	UtilitiesGetGraffitiObjectIndex
 *
 * DESCRIPTION: 	This routine returns the Object Index of the
 *					GSI (Graffiti shift indicator) in the active form.
 *
 *					Needed to move GSI for Active Input Area Support.
 *
 * PARAMETERS: 	->	formId:	the form Id to operate on.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * HISTORY:
 *	PPl	02/01/02	Initial revision.
 *
 ***********************************************************************/
uint16_t UtilitiesGetGraffitiObjectIndex (uint16_t formId)
{
	FormType* 	formP;
	uint16_t 	i;
	uint16_t 	howMany;

	formP = FrmGetFormPtr(formId);

	howMany = FrmGetNumberOfObjects(formP);
		
	for ( i = 0 ; i < howMany ; i ++)
	{
		if (FrmGetObjectType(formP, i) == frmGraffitiStateObj)
		{
			return (i);
		}
	}
	
	return (uint16_t)(-1);
}


// GadgetUtilities

/******************************************************************************
 *
 * FUNCTION:		UtilitiesGadgetFree
 *
 * DESCRIPTION:		Frees  Gadget Bitmap data.
 *				
 * PARAMETERS:	->	formP:		FormId of owner form.
 *				->	gadgetId:	Resource Id of the gadget.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPl	03/27/02	Initial Version.
 *
 ******************************************************************************/
void UtilitiesGadgetFree(uint16_t formId, uint16_t gadgetId)
{
	FormType* 	formP;
	MemHandle	bitmapH;
	uint16_t		objIndex;
	
	formP = FrmGetFormPtr(formId);
	if (formP == NULL)
		return;
	
	objIndex = FrmGetObjectIndex(formP, gadgetId);
	if (objIndex == frmInvalidObjectId)
		return;

	// Free previous bitmap
	bitmapH = (MemHandle) FrmGetGadgetData (formP, objIndex);
	if (bitmapH != NULL)
	{
		DmReleaseResource(bitmapH);
		FrmSetGadgetData (formP, objIndex, NULL);
	}
}	



/******************************************************************************
 *
 * FUNCTION:		UtilitiesGadgetChangeBitmap
 *
 * DESCRIPTION:		Change the bitmap.
 *					Free previous bitmap.
 *					Store the new bitmap handle in Gadget's data.
 *
 *					The gadget must be set up using UtilitiesGadgetSetup().
 *
 * PARAMETERS:	->	formId:		FormId of owner form.
 *				->	gadgetId:	Resource Id of the gadget.
 *				-> 	bitmapId:	Resource Id of the bitmaps to draw.
 *			
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPl	03/28/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGadgetChangeBitmap(DmOpenRef dbRef, uint16_t formId, uint16_t gadgetId, uint16_t bitmapId)
{
	FormType*	formP;
	MemHandle	bitmapH;
	uint16_t		objIndex;

	formP = FrmGetFormPtr(formId);
	if (formP == NULL)
		return;
	
	objIndex = FrmGetObjectIndex(formP, gadgetId);
	if (objIndex == frmInvalidObjectId)
		return;

	// Free previous bitmap
	bitmapH = (MemHandle) FrmGetGadgetData (formP, objIndex);
	if (bitmapH != NULL)
	{
		DmReleaseResource(bitmapH);
		FrmSetGadgetData (formP, objIndex, NULL);
	}
	
	// load Bitmap and put it in the gadget's data
	bitmapH = DmGetResource(dbRef, bitmapRsc, bitmapId);

	// set the gadget data - could be NULL
	FrmSetGadgetData (formP, objIndex, bitmapH);
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGadgetSetup
 *
 * DESCRIPTION:		Set up a Gadget Bitmap data.
 *					Store a bitmap handle in Gadget's data.
 *
 *					Set the default utilities gadget event handler when 
 *					gadgetEventHandler is NULL.
 *
 * PARAMETERS:	->	formId:			FormId of owner form.
 *				->	gadgetId:		Resource Id of the gadget.
 *				-> 	bitmapId:		Resource Id of the bitmaps to draw.
 *				-> 	gadgetCallback: Gadget's Eventhandler callback.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPl	03/28/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGadgetSetup(DmOpenRef dbRef, uint16_t formId, uint16_t gadgetId, uint16_t bitmapId, FormGadgetHandlerType* gadgetCallback)
{
	FormType* 	formP;
	MemHandle	bitmapH;
	uint16_t		objIndex;
	
	formP = FrmGetFormPtr(formId);
	if (formP == NULL)
		return;
		
	// Set the extended gadget handler routine for all the button gadgets.
	objIndex = FrmGetObjectIndex(formP, gadgetId);
	if (objIndex == frmInvalidObjectId)
		return;
		
	FrmSetGadgetHandler(formP, objIndex, gadgetCallback);

	// load Bitmap and put it in the gadget's data
	bitmapH = DmGetResource(dbRef, bitmapRsc, bitmapId);

	// set the gadget data - could be NULL
	FrmSetGadgetData (formP, objIndex, bitmapH);
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGadgetDrawBitmapHandler
 *
 * DESCRIPTION:		Draws a Gadget Bitmap data.
 *					The bitmap handle is stored in the gadget's data.
 *
 *					UtilitiesGadgetSetup can be used to associate the gadget and 
 *					its bitmap.
 *	
 * PARAMETERS:	->	formP:		Form owning the bitmap gadget.
 *				->	gadgetId:	Resource Id of the gadget.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			FormGadgetHandlerType gadget Callbalk.
 *
 * REVISION HISTORY:
 *	PPl	03/27/02	Initial revision.
 *
 ******************************************************************************/
Boolean UtilitiesGadgetDrawBitmapCallback(struct FormGadgetTypeInCallback *gadgetP, uint16_t cmd, void *paramP)
{
	BitmapType*	bitmapP;

	if (gadgetP == NULL) 
		return false;
		
	if (gadgetP->data == NULL)
		return false;
		
	if (cmd != formGadgetDrawCmd)
		return false;
		
	bitmapP = MemHandleLock((MemHandle) gadgetP->data);
	
	WinDrawBitmap(bitmapP, gadgetP->rect.topLeft.x, gadgetP->rect.topLeft.y);

	MemHandleUnlock((MemHandle) gadgetP->data);
	
	return true;
}

// Bitmap Utilities


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGetBitmapPtrBounds
 *
 * DESCRIPTION:		Get the bitamp bounds
 *
 *					Not Yet 	Density Proof
 * PARAMETERS:	->	bitmapP:	Bitmap ptr on the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *		
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGetBitmapPtrBounds(FormType* formP, BitmapType*	bitmapP, RectangleType * bounds)
{
	FormActiveStateType 	stateP;
	Coord 					width;
	Coord 					height;
	uint16_t 				rowBytes;

	if (bitmapP && bounds)
	{
		FrmSaveActiveState(&stateP);

		FrmSetActiveForm(formP);
	
		WinSetDrawWindow(FrmGetWindowHandle(formP));
		
		BmpGetDimensions (bitmapP, &width, &height, &rowBytes);

		RctSetRectangle(bounds, 0, 0, width, height);

		FrmRestoreActiveState(&stateP);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesGetBitmapHandleBounds
 *
 * DESCRIPTION:		Get the bitamp bounds.
 *
 *					Not Yet Density Proof.
 *
 * PARAMETERS:	->	bitmapH:	Bitmap Handle on the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *		
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesGetBitmapHandleBounds(FormType* formP, MemHandle bitmapH, RectangleType * bounds)
{
	BitmapType*	bitmapP;

	if (bitmapH != NULL)
	{
		bitmapP = MemHandleLock(bitmapH);

		UtilitiesGetBitmapPtrBounds(formP, bitmapP, bounds);

		MemHandleUnlock(bitmapH);
	}
}


// Bitmap drawing utilities


/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmapPtr
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 *
 * PARAMETERS:	->	bitmapP:	Bitmap ptr on the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *		
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesDrawBitmapPtr(BitmapType*	bitmapP, RectangleType * bounds)
{
	Coord 		width;
	Coord 		height;
	uint16_t 		rowBytes;
	uint16_t		density;
	Coord		densityDivisor;
	Coord		x;
	Coord		y;

	if (bitmapP != NULL)
	{
		BmpGetDimensions (bitmapP, &width, &height, &rowBytes);

		density	= BmpGetDensity(bitmapP);
		
		switch(density)
		{
			case kDensityDouble:
				densityDivisor = 2;
				break;
				
			case kDensityLow:
			default:
				densityDivisor = 1;
				break;
		}

		x = bounds->topLeft.x +(Coord) ((bounds->extent.x - (width/densityDivisor))  / 2);
		y = bounds->topLeft.y +(Coord) ((bounds->extent.y - (height/densityDivisor)) / 2);

		WinDrawBitmap(bitmapP, x, y);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmapHandle
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 *
 * PARAMETERS:	->	bitmapH:	Bitmap Handle on the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesDrawBitmapHandle(MemHandle bitmapH, RectangleType * bounds)
{
	BitmapType*	bitmapP;

	if (bitmapH != NULL)
	{
		bitmapP = MemHandleLock(bitmapH);

		UtilitiesDrawBitmapPtr(bitmapP, bounds);

		MemHandleUnlock(bitmapH);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmap
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 * PARAMETERS:	->	bitmapId:	Resource Id of the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesDrawBitmap(DmOpenRef dbRef, uint16_t bitmapId, RectangleType * bounds)
{
	MemHandle	bitmapH;

	// load Bitmap and put it in the gadget's data
	bitmapH = DmGetResource(dbRef, bitmapRsc, bitmapId);
	
	UtilitiesDrawBitmapHandle( bitmapH, bounds);
}



/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmapPtrAt
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 * PARAMETERS:	->	bitmapP:	Bitmap ptr on the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesDrawBitmapPtrAt(BitmapType*	bitmapP, Coord* atX, Coord* atY, RectangleType* bounds)
{
	if (bitmapP && atX && atY && bounds)
	{
		WinDrawBitmap(bitmapP, *atX, *atY);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmapHandleAt
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 * PARAMETERS:	->	bitmapH:	Bitmap Handle on the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *		
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesDrawBitmapHandleAt(MemHandle bitmapH, Coord *atX, Coord *atY, RectangleType* bounds)
{
	BitmapType*		bitmapP;

	if (bitmapH != NULL)
	{
		bitmapP = MemHandleLock(bitmapH);

		UtilitiesDrawBitmapPtrAt(bitmapP, atX, atY, bounds);

		MemHandleUnlock(bitmapH);
	}
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmapAt
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 * PARAMETERS:	->	bitmapId:	Resource Id of the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *		
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
void UtilitiesDrawBitmapAt(DmOpenRef dbRef, uint16_t bitmapId, Coord *atX, Coord *atY, RectangleType* bounds)
{
	MemHandle		bitmapH;

	// load Bitmap and put it in the gadget's data
	bitmapH = DmGetResource(dbRef, bitmapRsc, bitmapId);
	
	UtilitiesDrawBitmapHandleAt( bitmapH, atX, atY, bounds);
}


/******************************************************************************
 *
 * FUNCTION:		UtilitiesDrawBitmapAt
 *
 * DESCRIPTION:		Draws Bitmap into a rectangle.
 *
 * PARAMETERS:	->	bitmapId:	Resource Id of the bitmap to draw.
 *				->	bounds:		Drawing zone rectangle.
 *
 * RETURNED:    	Nothing.
 *
 * NOTE:			None.
 *
 * REVISION HISTORY:
 *		
 *	PPl	03/29/02	Initial revision.
 *
 ******************************************************************************/
Boolean UtilitiesRctRectangleEquals(RectangleType* rect1, RectangleType* rect2)
{
	Boolean result;

	result = rect1->topLeft.x == rect2->topLeft.x
			&& rect1->topLeft.y == rect2->topLeft.y
			&& rect1->extent.x == rect2->extent.x
			&& rect1->extent.y == rect2->extent.y;

	return result;
}
