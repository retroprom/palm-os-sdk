/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Find.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines field structures and routines.
 *
 *****************************************************************************/

#ifndef _FIND_H_
#define _FIND_H_

#include <PalmTypes.h>

#include <DataMgr.h>
#include <Rect.h>

#define maxFindStrLen		48				// Was 16 (increased for 16 UTF-8 characters)
#define maxFindStrPrepLen	64				// Enough space for 16 4-byte "find values"
											// (currently uses 2-byte values, though)

typedef struct {
	DatabaseID			appDbID;			// ID of the application
	DatabaseID			dbID;				// ID of the database that the record was found in
	Boolean				foundInCaller;		// true if found in app that called Find
	uint8_t 			reserved;
	uint16_t			reserved2;
	uint32_t 			recordNum;			// index of record that contains the match
	uint32_t			recordID;			// unique ID of record that contains the match
	uint32_t			matchFieldNum;		// field number or column ID (schema DB).
	size_t				matchPos;			// postion in record of the match.
	size_t				matchLen;			// length of match.
	uint32_t			matchCustom;		// app specific data
} FindMatchType;

typedef FindMatchType *FindMatchPtr;

typedef struct {

	// These fields are used by the applications.
	uint32_t			dbAccesMode;		// read mode and maybe show secret
	uint32_t 			recordNum;			// index of last record that contained a match
	uint32_t			recordID;			// unique ID of last record that contained a match
	Boolean				more;				// true if more matches to display
	char				strAsTyped[maxFindStrLen+1];	// search string as entered
	char				strToFind[maxFindStrPrepLen+1];	// search string returned by TxtPrepFindString
	Boolean				continuation;		// true if contining search of same app
	uint16_t			lineNumber;			// next line in the results tabel
	RectangleType		bounds;

	// The lineNumber field can be modified by the app. The continuation field can
	// be tested by the app. All other fields are private to the Find routine and
	// should NOT be accessed by applications.
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FINDPARAMS	// These fields will not be available in the next OS release!
	uint16_t			numMatches;			// # of matches
	Boolean				searchedCaller;		// true after we've searched app that initiated the find
	uint8_t				reserved1;			// alignment padding
	uint8_t 			reserved2;			// alignment padding
	uint8_t 			reserved3;			// alignment padding
	DatabaseID			callerAppDbID;		// ID of app that initiated search
	DatabaseID			appDbID;			// ID of app that we're currently searching
	DmSearchStateType	searchState;		// search state
#else
	uint16_t			noAccessAllowed1;	// numMatches
	Boolean				noAccessAllowed2;	// searchedCaller
	uint8_t				noAccessAllowed3;	// reserved1
	uint8_t 			noAccessAllowed4;	// reserved2
	uint8_t				noAccessAllowed5;	// reserved3
	DatabaseID			noAccessAllowed6;	// callerAppDbID
	DatabaseID			noAccessAllowed7;	// appDbID
	DmSearchStateType	noAccessAllowed8;	// search state
#endif
} FindParamsType;

typedef FindParamsType *FindParamsPtr;


// Param Block passsed with the sysAppLaunchCmdGoto Command
typedef struct {
	DatabaseID			dbID;				// ID of the database
	uint32_t 			recordNum;			// index of record that contains a match
	uint32_t			recordID;			// unique ID of record that contains a match
	size_t				matchPos;			// postion in record of the match.
	size_t				matchLen;			// length of matched text in record.
	uint32_t			matchFieldNum;		// field number string was found int
	size_t				searchStrLen;		// length of search string.
	uint32_t			matchCustom;		// application specific info
	char				string[maxFindStrLen+1];
	uint8_t				reserved1;			// alignment padding
	uint8_t 			reserved2;			// alignment padding
	uint8_t 			reserved3;			// alignment padding
} GoToParamsType;

typedef GoToParamsType *GoToParamsPtr;


//----------------------------------------------------------
//	Find Functions
//----------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

void Find (GoToParamsPtr goToP);

Boolean FindSaveMatch (FindParamsPtr findParams, uint32_t recordNum, uint32_t recordID,
	size_t matchPos, size_t matchLen, uint32_t fieldNum, uint32_t appCustom, DatabaseID dbID);

void FindGetLineBounds (const FindParamsType *findParams, RectanglePtr r);

Boolean FindDrawHeader (FindParamsPtr findParams, char const *title);

// Deprecated - use TxtFindString instead, which returns back the match length,
// and supports a 32-bit match offset.
Boolean FindStrInStrV50 (char const *strToSearch, char const *strToFind, uint16_t *posP);

Boolean	FindSaveMatchV40(FindParamsPtr findParams, uint16_t recordNum, uint16_t pos,
			uint16_t fieldNum, uint32_t appCustom, uint16_t cardNo, LocalID dbID);

#ifdef __cplusplus 
}
#endif

#endif //_FIND_H_
