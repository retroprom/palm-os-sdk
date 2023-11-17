/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DateAlarm.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the alarm functions.
 *
 *****************************************************************************/

#ifndef _DATEALARM_H_
#define _DATEALARM_H_

#include <AlarmMgr.h>
#include <AttentionMgr.h>

// There are several occasions where the app will need to use
// the attention manager iteration routine to update or validate
// the information that exists in the attention manager queue.
// The following constants designate the type of iteration requested.

#define	SoundRepeatChanged			1
#define	PostHotsyncVerification		2
#define	DeviceTimeChanged			3


void 	AlarmReset (Boolean newerOnly);

void 	RescheduleAlarms (DmOpenRef dbP);

void 	RescheduleAlarmsAllRows (void);

void 	AlarmTriggered (SysAlarmTriggeredParamType * cmdPBP);

void 	PlayAlarmSound(uint32_t uniqueRecID);

Boolean DeleteAlarmIfPosted (uint32_t cursorID);

Boolean	AttentionBottleNeckProc(AttnLaunchCodeArgsType * paramP);

#endif //_DATEALARM_H_
