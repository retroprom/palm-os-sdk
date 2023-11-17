/******************************************************************************
 *
 * Copyright (c) 1995-2004 PalmSource, Inc. All rights reserved.
 *
 * File: AlarmMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Include file for Alarm Manager
 *
 *****************************************************************************/

#ifndef _ALARM_MGR_H_
#define _ALARM_MGR_H_


// Include elementary types
#include <PalmTypes.h>
#include <CmnErrors.h>
#include <DataMgr.h>
#include <MemoryMgr.h>

/************************************************************
 * Alarm Manager result codes
 * (almErrorClass is defined in ErrorBase)
 *************************************************************/
#define	almErrMemory			(almErrorClass | 1)	// ran out of memory
#define	almErrFull				(almErrorClass | 2)	// alarm table is full

/********************************************************************
 * Alarm Manager Structures
 ********************************************************************/
typedef struct SysAlarmTriggeredParamType
{
	uint32_t			ref;			// --> alarm reference value passed by caller;
	uint32_t			alarmSeconds;	// --> alarm date/time in seconds since 1/1/1904;

	Boolean			purgeAlarm;		// <-- if set to true on return, this alarm
									// will be removed from the alarm table and the
									// display notification will NOT be generated for it
	uint8_t 			padding;
	uint16_t			padding1;
} SysAlarmTriggeredParamType;


typedef struct SysDisplayAlarmParamType
{
	uint32_t			ref;			// alarm reference value passed by caller;
	uint32_t			alarmSeconds;	// alarm date/time in seconds since 1/1/1904;
	Boolean			soundAlarm;		// non-zero if alarm needs to be sounded;
	uint8_t 			padding;
	uint16_t			padding1;
} SysDisplayAlarmParamType;

enum AlmProcCmdEnumTag
{
	almProcCmdTriggered = 0,		// Alarm triggered
	almProcCmdReschedule,			// Reschedule (usually as a result of time change)
	almProcCmdCustom = 0x8000		// Alarm manager reserves all enums up to almProcCmdCustom
};

typedef Enum16 AlmProcCmdEnum;

#ifdef __cplusplus
extern "C" {
#endif

status_t AlmSetAlarm(DatabaseID dbID, uint32_t ref, uint32_t alarmSeconds, Boolean quiet);
uint32_t AlmGetAlarm(DatabaseID dbID, uint32_t *refP);
void AlmEnableNotification(Boolean enable);

#ifdef __cplusplus
}
#endif

#endif  // _ALARM_MGR_H_
