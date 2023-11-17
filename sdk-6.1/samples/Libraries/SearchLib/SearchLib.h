/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SearchLib.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef __SEARCHLIB_H__
#define __SEARCHLIB_H__

#include <SchemaDatabases.h>
#include <CatMgr.h>

/************************************************************
 * Public constants
 *************************************************************/

// Applied to row and column of a row. This allow to search from a direction in the cursor (from first to last or last to first row)
// and to search from another direction inside a row (first to last or last to first column)
typedef uint8_t					SearchDirectionType;

#define kSearchMoveForward		((SearchDirectionType) 0x01)	// If the search failed in the current row/column, go to the next row/column in the cursor
#define kSearchMoveBackward		((SearchDirectionType) 0x02)	// If the search failed in the current row/column, go to the previous row/column in the cursor
#define kSearchMoveLocked		((SearchDirectionType) 0x03)	// If the search failed in the current row/column, stop the search.

// After a search succeed, this option determine where to continue the search. In the next row, next column of the same row,
// or in the same column where the previous find succeed.
typedef uint8_t					SearchResumeType;

#define kSearchResumeChangeRecord	((SearchResumeType) 0x01)
#define kSearchResumeChangeColumn	((SearchResumeType) 0x02)
#define kSearchResumeContinue		((SearchResumeType) 0x03)

/************************************************************
 * Public structure
 *************************************************************/

typedef struct SearchLibTag
{
	// Input definition

	// Search parameters
	uint32_t					cursorID;			// ID of the cursor used for the seach. If != dbInvalidCursorID use it for the search otherwise create a new one
	DmOpenRef					searchDB;			// Database to search in
	char*						schemaName;			// Schema to use to get records
	char*	 					orderBy;			// Order by to use for the search.
	uint32_t					cursorFlag;			// Falg to use to create the cursor (dbCursorIncludeSecret, ...)
	uint32_t					startRowIndex;		// Start from this row index for the first search (1 =  first / dbMaxRowIndex = last)
	uint32_t					startColumnIndex;	// Start from this record index for the first search (0 =  first / dbMaxRowIndex = last)
	uint32_t					numCols;			// number of columns IDs to search in
	uint32_t*					colIDsP;			// array of column IDs to search in
	uint32_t					numCategories;		// number of category to search in
	CategoryID*					catIDsP;			// array of category to search in
	DbMatchModeType				matchMode;			// match mode to apply to the catgory search
	char*						origTextToSearch;	// Text to search (unnormalized)
	char*						textToSearch;		// Text to search (normalized)

	// Input options

	// Move option
	SearchDirectionType			recordDirection;	// Search direction in the cursor(next/previous row)
	SearchDirectionType			columnDirection;	// Search direction inside a row (next/previous column)
	SearchResumeType			resumePolicy;		// Determine where the next search will start (Next/Previous row/Column, ...)
	uint8_t						padding1;

	// filter option
	uint16_t					charFilter;			// If != 0, only keep chars that match this char Attrib (charAttr_XD, charAttrAlNum, ...)
	Boolean						reverseString;		// if true, reverse string before comparing.
	Boolean						useCompare;			// If true, Compare String function will be used instead of Find
	Boolean						caseCompare;		// If true, use case compare function instead of case less compare.
	Boolean						hidePrivate;		// If true, record marked as private are not foundable
	uint8_t						padding2[2];
	uint32_t					minCompareMatch;	// if != 0, stop searching when having a least this number of matching chars.

	// Timer option
	Boolean						indirect;			// If true, assume that columns listed in colIDsP contains columnID as data.
	Boolean						interruptible;		// If true, the search could be interrupted
	Boolean						interrupted;		// If true, the search has been interrupted.
	uint32_t					interruptCheck;		// Check for an event every interruptCheck milliseconds.


	// Output parameters
	status_t					err;				// if pattern not found, could be != errNone
	uint32_t					foundRowID;			// RowID that contains an occurence
	uint32_t					foundRowIndex;		// RowIndex that contains an occurence
	uint32_t					foundColID;			// ColumnID that contains an occurence
	uint32_t					matchPos;			// Position of the occurence in the column
	uint32_t					matchLength;		// Length of the matching string
	uint32_t					strDestLength;		// Length of the string that contains the pattern

	// Internal data
	Boolean						cursorCreated;
	Boolean						firstCall;
	Boolean						freeColIDs;			// True if the colIDsP array has been initialized by the Search lib
	uint8_t						padding4;
	uint32_t					timer;
	uint32_t					foundColIndex;		// Index of the column in the initial array;
	DbSchemaColumnValueType*	columnData;			// Data of the current column ID list of the record.
	DbSchemaColumnDefnType*		columnDefs;

} SearchLibType, *SearchLibPtr;
/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

status_t	SearchLibInit(SearchLibType* search);
status_t	SearchLibRelease(SearchLibType* search);

Boolean		SearchLibSearch(SearchLibType* search);
status_t	SearchLibReset(SearchLibType* search);

status_t	SearchLibSetStartIndexesById(SearchLibType* search, uint32_t recordID, uint32_t columnID);

char*		SearchLibConvertString(char* origString, uint16_t charFilter, Boolean reverse, uint32_t* newSize);

#ifdef __cplusplus
}
#endif

#endif // __SEARCHLIB_H__
