/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AddressAutoFill.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _ADDRESSAUTOFILL_H_
#define _ADDRESSAUTOFILL_H_


/***********************************************************************
 *	Defines
 ***********************************************************************/

#define kMaxLookupEntries				100		// max number of entries per lookup database
#define kEntryMaxSize					256

#define kAddrAutoFillDBNamePrefix		"AddrAutoFillDB_"
#define kAddrAutoFillDBNamePrefixLen	(15)

/***********************************************************************
 *	Private structures
 ***********************************************************************/

typedef struct {
	uint32_t	time;		// time the entry was last accessed
	uint16_t	padding1;
	uint8_t		padding2;
	char		text;		// null-terminated string
} LookupRecordType;

typedef LookupRecordType *LookupRecordPtr;

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

status_t	AutoFillInitDB(uint32_t dbType, uint16_t initRscID);

Boolean		AutoFillLookupStringInDatabase(DmOpenRef dbP, char *keyP, uint16_t *indexP);

void		AutoFillLookupSave(uint32_t dbType, char *strP);

#ifdef __cplusplus
}
#endif

#endif // _ADDRESSAUTOFILL_H_
