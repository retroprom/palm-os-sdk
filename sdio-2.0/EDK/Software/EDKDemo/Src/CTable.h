/***********************************************************************
 *
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *
 ***********************************************************************/

/***********************************************************************
 *
 * EDK Demo CTable.h Header File.
 *
 ***********************************************************************/

/***********************************************************************
 *		Prototypes
 ***********************************************************************/

void CTableInit( void );
void CTableLoad( UInt16 topNdx );
void CTableStoreText( Char *strP, Boolean bold );
void CTableRedraw( void );
void CTableDrawProc( void *tableP, Int16 row, Int16 col, RectanglePtr r );



/***********************************************************************
 *		Global variables
 ***********************************************************************/

// Macros for "printing" text to the scrollable table on screen
// PR() - prints standard font, PRB() - prints bold font
#define PR(x)	CTableStoreText( (Char *)x, false )
#define PRB(x)	CTableStoreText( (Char *)x, true )


//------------------------------------------------------------------------
