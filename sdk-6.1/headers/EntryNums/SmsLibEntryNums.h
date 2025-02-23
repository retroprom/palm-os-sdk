/******************************************************************************
 *
 *	Copyright (c) 2004 PalmSource, Inc. or its subsidiaries.
 *	All rights reserved.
 *
 *	File: SmsLibEntryNums.h
 *
 *	Description:
 *		This file is automatically generated by the
 *		PalmSource Shared Library Generator.
 *
 *	History:
 *		Generated on: Tue Aug 31 14:15:34 2004
 *****************************************************************************/
#ifndef __SMSLIBENTRYNUMS_H__	// Avoid multiple inclusion
#define __SMSLIBENTRYNUMS_H__

// System header file
#include <PalmTypes.h>

enum smslibEntryNumTag {
	smslibEntryNumExgSmsLibOpenEnum = 0,
	smslibEntryNumExgSmsLibCloseEnum,
	smslibEntryNumExgSmsLibSleepEnum,
	smslibEntryNumExgSmsLibWakeEnum,
	smslibEntryNumExgSmsLibHandleEventEnum,
	smslibEntryNumExgSmsLibAcceptEnum,
	smslibEntryNumExgSmsLibConnectEnum,
	smslibEntryNumExgSmsLibDisconnectEnum,
	smslibEntryNumExgSmsLibPutEnum,
	smslibEntryNumExgSmsLibGetEnum,
	smslibEntryNumExgSmsLibSendEnum,
	smslibEntryNumExgSmsLibReceiveEnum,
	smslibEntryNumExgSmsLibControlEnum,
	smslibEntryNumExgSmsLibRequestEnum,
	smslibEntryNumSmsLib68kReadExgSocketSmsParamsFrom68KMemoryEnum,
	smslibEntryNumSmsLib68kWriteExgSocketSmsParamsTo68KMemoryEnum,
};

#define smslibEntryNumExgSmsLibOpen                                	((uint32_t)smslibEntryNumExgSmsLibOpenEnum)
#define smslibEntryNumExgSmsLibClose                               	((uint32_t)smslibEntryNumExgSmsLibCloseEnum)
#define smslibEntryNumExgSmsLibSleep                               	((uint32_t)smslibEntryNumExgSmsLibSleepEnum)
#define smslibEntryNumExgSmsLibWake                                	((uint32_t)smslibEntryNumExgSmsLibWakeEnum)
#define smslibEntryNumExgSmsLibHandleEvent                         	((uint32_t)smslibEntryNumExgSmsLibHandleEventEnum)
#define smslibEntryNumExgSmsLibAccept                              	((uint32_t)smslibEntryNumExgSmsLibAcceptEnum)
#define smslibEntryNumExgSmsLibConnect                             	((uint32_t)smslibEntryNumExgSmsLibConnectEnum)
#define smslibEntryNumExgSmsLibDisconnect                          	((uint32_t)smslibEntryNumExgSmsLibDisconnectEnum)
#define smslibEntryNumExgSmsLibPut                                 	((uint32_t)smslibEntryNumExgSmsLibPutEnum)
#define smslibEntryNumExgSmsLibGet                                 	((uint32_t)smslibEntryNumExgSmsLibGetEnum)
#define smslibEntryNumExgSmsLibSend                                	((uint32_t)smslibEntryNumExgSmsLibSendEnum)
#define smslibEntryNumExgSmsLibReceive                             	((uint32_t)smslibEntryNumExgSmsLibReceiveEnum)
#define smslibEntryNumExgSmsLibControl                             	((uint32_t)smslibEntryNumExgSmsLibControlEnum)
#define smslibEntryNumExgSmsLibRequest                             	((uint32_t)smslibEntryNumExgSmsLibRequestEnum)
#define smslibEntryNumSmsLib68kReadExgSocketSmsParamsFrom68KMemory 	((uint32_t)smslibEntryNumSmsLib68kReadExgSocketSmsParamsFrom68KMemoryEnum)
#define smslibEntryNumSmsLib68kWriteExgSocketSmsParamsTo68KMemory  	((uint32_t)smslibEntryNumSmsLib68kWriteExgSocketSmsParamsTo68KMemoryEnum)
#endif // __SMSLIBENTRYNUMS_H__
