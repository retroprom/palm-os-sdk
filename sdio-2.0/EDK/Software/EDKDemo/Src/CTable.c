
/***********************************************************************
 *
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *
 ***********************************************************************/


/***********************************************************************
 *
 * EDK Demo CTable.c Src File
 *
 ***********************************************************************/


/***********************************************************************
 *
 * Description:
 *		These routines implement a "print" facility to an on screen text
 *		region that can be scrolled using the scrollbar or the hard scroll
 *		buttons. The maximum number of lines that can be "printed" is
 *		determined at compile time by kCardTableMaxLines. If the application
 *		"prints" more text lines than this to the static buffer, the
 *		additional text lines are discarded. The static buffer used for
 *		storing the text lines does not wrap around. Also, text lines are
 *		truncated to kCardTableMaxLineLen in order to fit into the static
 *		buffer and possibly truncated again to fit within the on-screen
 *		table cell boundaries.
 *
 *		To print, call either of these two macros, PR() or PRB().
 *		PR() prints using standard font, PRB() prints using bold font.
 *		Note that the text lines are not displayed at the time PR() or
 *		PRB() are called. The text lines are only displayed by CTableRedraw().
 *
 * EXAMPLE:
 *		CTableInit();
 *		PR(  "This line is standard font." );
 *		PRB( "However, this will be bold." );
 *	    CTableLoad( 0 );  // specify array index in static buffer of the
 *						  //  top item in scrollable text region
 *   	CTableRedraw();   // display table on screen
 *		PR(  "Append this line to the other two." );
 *	    CTableLoad( 0 );  // specify array index in static buffer of the
 *						  //  top item in scrollable text region
 *   	CTableRedraw();   // display table on screen again this time
 *						  //  with the new line appended.
 *
 * FUNCTIONS DEFINED:
 * 1. CTableInit()		- Initialize table used to control "printing"
 *						  to scrollable text region on screen.
 * 2. CTableLoad()		- Load Palm OS table with indexes into static buffer.
 * 3. CTableStoreText() - This is the "print" routine for storing text lines
 *						  into the static buffer for later display on screen in a
 *						  scrollable text area.
 * 4. CTableRedraw()	- Draw text lines on screen from static text buffer.
 * 5. CTableDrawProc()	- Callback procedure to display text lines from
 *						  static buffer using Palm OS table.
 *
 ***********************************************************************/

/***********************************************************************
 *		Include Files
 ***********************************************************************/

#include <PalmOS.h>
#include "EDKDemoRsc.h"
#include "CTable.h"


/***********************************************************************
 *		Prototypes
 ***********************************************************************/

void *GetObjectPtr( UInt16 objectID );


/***********************************************************************
 *		External References
 ***********************************************************************/

extern Char gTbuf[ 300 ];	// temp buffer

/***********************************************************************
 *		Global variables
 ***********************************************************************/

// 80 chars for the table line length is not overkill here. For example,
// 71 chars (all i's) fit within the 145 pixel width of CardContentTable.
#define kCardTableMaxLines			( 250 )	// Maximum no. of text lines
#define kCardTableMaxLineLen		( 80 )	// Maximum no. of chars per line
#define kCardTableMaxLineSize		( 1 + kCardTableMaxLineLen )


UInt16	gCardTableCnt = 0;

static	Boolean	gBoldLine[  kCardTableMaxLines ];
static	Char	gCardTable[ kCardTableMaxLines ][ kCardTableMaxLineSize ];

/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void * GetObjectPtr(UInt16 objectID)
{
	FormPtr frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}


/***********************************************************************
 *
 * FUNCTION:    CTableInit
 *
 * DESCRIPTION: Initialize table used to control "printing" to scrollable
 *				text region on screen. Initialize text buffer, line
 *				attributes, set the attributes of the table rows: usable,
 *				ptr, int, style, associate a callback custom draw procedure
 *				for drawing table rows, and mark the columns as usable.
 *
 * INPUT:		none
 *
 * OUTPUT:		Globals:
 *				- gBoldLine[]  - initialized
 *				- gCardTable[] - initialized
 *				- CardContentsTable Palm OS Table - set attributes
 *				  for all the table rows
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void CTableInit( void )
{
	TableType   *tableP;
	UInt16	     rowNdx;
	UInt16	     numRows;


	// Initialize globals
	for( rowNdx=0  ;  rowNdx < kCardTableMaxLines  ;  ++rowNdx )
	{
		gBoldLine[  rowNdx ] = false;
		gCardTable[ rowNdx ][ 0 ] = 0;
	}

	gCardTableCnt = 0;

	// Set up the Table attributes
	tableP  = GetObjectPtr( MainCardContentsTable );

	numRows = (UInt16)TblGetNumberOfRows( tableP );

	for( rowNdx=0  ;  rowNdx < numRows  ;  ++rowNdx )
	{
		TblSetRowUsable( tableP, (Int16)rowNdx, false );
		TblSetItemPtr(   tableP, (Int16)rowNdx, 0 /*col*/, 0 /*ptr*/ );
		TblSetItemInt(   tableP, (Int16)rowNdx, 0 /*col*/, 0 /*value*/ );
		TblSetItemStyle( tableP, (Int16)rowNdx, 0 /*col*/, customTableItem );
	}

	// Associate CTableDrawProc() with this table as a
	//  callback custom draw procedure for this table.
	TblSetCustomDrawProcedure( tableP, 0 /*col*/,  CTableDrawProc );

	TblSetColumnUsable( tableP, 0 /*col*/, true );
}


/***********************************************************************
 *
 * FUNCTION:    CTableLoad
 *
 * DESCRIPTION: Load Palm OS table with indexes into text array.
 *				For each table row, set the 'int' fields to the
 *				corresponding elements in the gCardTable[] array. If at
 *				least one 'int' field has changed since last time, mark
 *				the table as invalid to force the whole table to be redrawn
 *				the next time its drawn.
 *
 * INPUT:		topArrayNdx - index into gCardTable[] to display at top of table.
 *
 * OUTPUT:		Globals:
 *				- CardContentsTable Palm OS Table - set the 'int' field of
 *				  each table row to the index of the corresponding entry in
 *				  the gCardTable[] array, then mark each row as usable. If
 *				  the on-screen row is beyond the end of the data in the
 *				  gCardTable[] array, set those rows as unusable.
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void CTableLoad( UInt16 topArrayNdx )
{
	TableType *tableP;
	UInt16	   tblNdx;
	UInt16	   numRows;
	UInt16	   prevArrayNdx, newArrayNdx;
	Boolean	   dirty = false;


	tableP  = GetObjectPtr( MainCardContentsTable );
	numRows = (UInt16)TblGetNumberOfRows( tableP );

	for( tblNdx=0  ;  tblNdx < numRows  ;  ++tblNdx )
	{
		newArrayNdx = tblNdx + topArrayNdx;
		if( tblNdx < gCardTableCnt )
		{
			// Store tblNdx in 'int' field in table
			prevArrayNdx = (UInt16)TblGetItemInt( tableP, (Int16)tblNdx, 0 /*col*/ );
			if( prevArrayNdx != newArrayNdx )
			{
				// Table index value has changed, replace it
				TblSetItemInt( tableP, (Int16)tblNdx, 0 /*col*/, (Int16)newArrayNdx );
				dirty = true;
			}
			// Mark this row as USABLE
			TblSetRowUsable( tableP, (Int16)tblNdx, true );
		}
		else
		{
			// On-screen row is beyond our data, set this row as UNUSABLE
			TblSetRowUsable( tableP, (Int16)tblNdx, false );
		}
	}

	if( dirty )
	{
		// At least one item has changed, so force the whole
		//  table to be redrawn the next time it is drawn.
		TblMarkTableInvalid( tableP );
	}
}


/***********************************************************************
 *
 * FUNCTION:    CTableStoreText
 *
 * DESCRIPTION:	This is the "print" routine for storing text lines into
 *				a static buffer for later display on screen in the
 *				scrollable text area. To call this routine, use one of
 *				the following macros. See CTable.h for more.
 *
 *				- PR() stores the text line in the next available position in
 *				  the static buffer for display later using the standard font.
 *				- PRB() stores the text line for display later using bold font.
 *
 *				This routine copies the input text string to the next
 *				available position in static buffer for displaying later
 *				on screen. If the static buffer is full of text lines,
 *				the input text line is discarded.
 *
 *				Do not append "\n" to your text line. Each call to PR()
 *				or PRB() creates a new text line.
 *
 *				If the input strP will not fit within the pixel boundaries
 *				of the Palm OS table containing these strings, the table's
 *				callback draw function will truncate the string on display
 *				and append an ellipsis.
 *
 *				This routine handles multibyte characters.
 *
 * EXAMPLE:		PR(  "This will be standard font." );
 *				PRB( "This will be bold font." );
 *
 * INPUT:		strP - text string to display on screen at next available
 *					   line position within gCardTable[] array.
 *				bold - true  - display text string in bold,
 *					   false - display using standard font
 *
 * OUTPUT:		Globals:
 *				- gCardTable[] - text string is copied to next available
 *				  position in this array. Input string is truncated to
 *				  fit into static buffer if necessary.
 *				- gCardTableCnt - updated to reflect total number of text
 *				  lines in gCardTable[] array.
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void CTableStoreText( Char *strP, Boolean bold )
{
	if( gCardTableCnt < kCardTableMaxLines )
	{
		// There's room to add another line
		if( bold )
			gBoldLine[ gCardTableCnt ] = true;
		else
			gBoldLine[ gCardTableCnt ] = false;

		// Store string to display in our text-scroll table.
		StrCopy( gCardTable[ gCardTableCnt ],
					  strP );

		++gCardTableCnt;
	}
	else
	{
		// The text array is full, discard the input text line.
	}
}


/***********************************************************************
 *
 * FUNCTION:    CTableRedraw
 *
 * DESCRIPTION:	Draw text lines on screen using Palm OS Table.
 *				Get bounds of table from OS, draw box around the table,
 *				then redraw table contents on screen keeping the same
 *				offset within the gCardTable[] array (that was set by
 *				CTableLoad()). That is, the text line displayed at the top
 *				of the scrollable text area remains unchanged. Draw the box
 *				around the table wide enough (9 more pixels) to include
 *				the scrollbar at the far right edge of the scrollable
 *				text area.
 *
 * INPUT:		none
 *
 * OUTPUT:		none
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void CTableRedraw( void )
{
	RectangleType r;
	TableType    *tableP;

	tableP  = GetObjectPtr( MainCardContentsTable );

	// Find out dimensions of the current table
	TblGetBounds( tableP, &r );

	// Draw box around table with 2 pixel margin all the way around.
	r.topLeft.x -= 2;
	r.topLeft.y -= 2;
	r.extent.x  += 4 + 9;  // Add 9 pixels to put the scrollbar inside the box
	r.extent.y  += 4;

	WinDrawRectangleFrame( simpleFrame, &r );

	TblRedrawTable( tableP );
}


/***********************************************************************
 *
 * FUNCTION:    CTableDrawProc
 *
 * DESCRIPTION: Callback procedure to display text lines from static buffer
 *				using Palm OS table. This callback procedure is associated with
 *				the CardContentsTable by calling TblSetCustomDrawProcedure()
 *				during initialization. See CTableInit() for more.
 *
 *				This routine is for custom drawing of each table row on
 *				screen. The 'int' field is retrieved from the Palm OS table,
 *				CardContentsTable, and used as an index into the gCardTable[]
 *				array to select the text line to display at the specified
 *				row and col on screen. The table cell dimensions are used
 *				to truncate text strings that may not fit within the table
 *				cell boundaries on screen.
 *
 *				If the input string from the table will not fit within the
 *				pixel boundaries of the Palm OS table containing these
 *				strings, this function will truncate the string on display
 *				and append an ellipsis.
 *
 *				This routine handles multibyte characters.
 *
 * INPUT:		- tableP - ptr to table which controls scrollable text area
 *				- row	 - row in table to be displayed
 *				- col	 - col in table to be displayed (only 1 column
 *						   in this case)
 *				- r		 - on-screen dimensions of the Palm OS table cell
 *
 *				Globals:
 *				- gCardTable[] - array of text lines to display in the
 *				  scrollable text area on screen.
 *				- gBoldLine[]  - array of booleans indicating whether the
 *				  corresponding text line in gCardTable[] should be displayed
 *				  in bold or not.
 *
 * OUTPUT:		none
 *
 * FN RETURNS:	none
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

static void CTableDrawProc( void *tableP, Int16 row, Int16 col, RectanglePtr r )
{
	Int16     x, y, maxWid, ndx;
	Char      *strP;
	FontID	  origFont;

	ndx = TblGetItemInt( tableP, row, col );

	x      = r->topLeft.x;
	y      = r->topLeft.y;
	maxWid = r->extent.x;    // Max width in pixels for the text string

	// Note that maxWid is the width in pixels of the CardContentsTable,
	// so when WinDrawTruncChars() below truncates the string, it will
	// be truncated to fit within the table dimensions. Consequently, the
	// text won't overwrite the scroll bar, which is just to the right of
	// the table on screen.

	// Get pointer to text string to display
	strP = gCardTable[ ndx ];

	if( gBoldLine[ ndx ] )
		origFont = FntSetFont( boldFont );
	else
		origFont = FntSetFont( stdFont );

	// WinDrawTruncChars() handles multi-byte chars, and appends
	// an ellipsis "..." if the string is truncated.
	WinDrawTruncChars( strP, StrLen( strP ), x, y, maxWid );

	// Restore orig font
	FntSetFont( origFont );
}

//------------------------------------------------------------------------

