/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SysUtils.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *	  These are miscellaneous routines.
 *
 *****************************************************************************/

#ifndef _SYSUTILS_H_
#define _SYSUTILS_H_

// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CmnBitmapTypes.h>
#include <DataMgr.h>

#ifndef SysBatteryKind_defined
typedef Enum8 SysBatteryKind;
#define SysBatteryKind_defined 1
#endif

typedef int16_t _comparF (void *, void *, int32_t other);
typedef _comparF * CmpFuncPtr;

typedef int16_t _searchF (void const *searchData, void const *arrayData, int32_t other);
typedef _searchF * SearchFuncPtr;

typedef struct {
	char			name[dmDBNameLength];
	uint32_t		creator;
	uint32_t		type;
	uint16_t		version;
	uint16_t		padding;
	LocalID			dbID;
	BitmapPtr		iconP;
} SysDBListItemType;


/************************************************************
 * Constants
 *************************************************************/
#define	sysRandomMax		0x7FFF			// Max value returned from SysRandom()


/************************************************************
 * Macros
 *************************************************************/
#define Abs(a) (((a) >= 0) ? (a) : -(a))

/************************************************************
 * procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Boolean	SysBinarySearch (void const *baseP, const uint16_t numOfElements, const int16_t width, 
			SearchFuncPtr searchF, void const *searchData, 
			const int32_t other, int32_t *position, const Boolean findFirst);

void 	SysInsertionSort (void *baseP, uint16_t numOfElements, int16_t width, 
			const CmpFuncPtr comparF, const int32_t other);

void 	SysQSort (void *baseP, uint16_t numOfElements, int16_t width, 
			const CmpFuncPtr comparF, const int32_t other);

void	SysCopyStringResource (char *string, DmOpenRef dbRef, DmResourceID resID);

MemHandle SysFormPointerArrayToStrings(char *c, int16_t stringCount);
						
						
// Return a random number ranging from 0 to sysRandomMax.
// Normally, 0 is passed unless you want to start with a new seed.
int16_t	SysRandom(int32_t newSeed);

char *	SysStringByIndex(DmOpenRef dbRef, DmResourceID resID, uint16_t index, char *strP, uint16_t maxLen);

char *	SysErrString(status_t err, char *strP, uint16_t maxLen);

char *	SysErrStringTextOnly(status_t err, char *strP, uint16_t maxLen);

char *	SysGetOSVersionString(void);

Boolean SysCreateDataBaseList(uint32_t type, uint32_t creator, uint16_t *dbCount, MemHandle *dbIDs, Boolean lookupName, DmFindType findType);

Boolean SysCreatePanelList(uint16_t * panelCount, MemHandle *panelIDs);

// Returns TRUE if this is the main UI thread.
Boolean SysAreWeUIThread(void);

// Returns TRUE if this thread is running a UI context.  This is true
// for the main UI thread (above) and any thread that is currently
// inside a UI context from WinStartThreadUI().
Boolean SysAreWeRunningUI(void);

// Deprecated APIs...
// *************************************************************************

void	SysCopyStringResourceV50 (char *string, DmResourceID resID);

char *	SysStringByIndexV50(DmResourceID resID, uint16_t index, char *strP, uint16_t maxLen);

status_t SysGetROMTokenV40(uint16_t cardNo, uint32_t token, uint8_t **dataP, uint16_t *sizeP);

uint16_t SysBatteryInfoV40(Boolean set, uint16_t *warnThresholdP, uint16_t *criticalThresholdP, uint32_t *maxTimeP, SysBatteryKind* kindP, Boolean *pluggedIn, uint8_t *percentP);

Boolean SysCreateDataBaseListV50(uint32_t type, uint32_t creator, uint16_t* count, MemHandle* dbIDs, Boolean lookupName);

#ifdef __cplusplus
}
#endif

#endif // _SYSUTILS_H_
