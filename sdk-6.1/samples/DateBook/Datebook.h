/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Datebook.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	  This file defines the Datebook's Main modual's functions anf globals.
 *
 *****************************************************************************/

#ifndef _DATEBOOK_H_
#define _DATEBOOK_H_

#include <CmnLaunchCodes.h>
#include <Find.h>
#include <ExgMgr.h>
#include <CatMgr.h>
#include <AlarmMgr.h>
#include <AttentionMgr.h>
#include <Form.h>
#include <PrivateRecords.h>

#include "DateDB.h" // for AlarmInfoType

/***********************************************************************
 *
 *	Datebook prefs structure
 *
 ***********************************************************************/

// This is the structure of the data that's saved to the state file.
typedef struct {
	uint16_t			dayStartHour;
	uint16_t			dayEndHour;
	AlarmInfoType		alarmPreset;
	Boolean				saveBackup;
	Boolean				showTimeBars;
	Boolean				compressDayView;
	Boolean				showTimedAppts;
	Boolean				showUntimedAppts;
	Boolean				showDailyRepeatingAppts;
	
	// Version 3 preferences
	uint16_t 			alarmSoundRepeatCount;
	uint16_t 			alarmSoundRepeatInterval;
	uint32_t			alarmSoundUniqueRecID;
	FontID				apptDescFont;
	uint8_t				padding;
	
	// Version 4 preferences
	uint16_t				alarmSnooze;
} DatebookPreferenceType;


typedef enum {
	appLaunchCmdAlarmEventGoto = sysAppLaunchCmdCustomBase,
	appLaunchCmdExgGetFullLaunch
} DateBookCustomLaunchCodes;


/***********************************************************************
 *
 *	Functions
 *
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


int16_t DatebookLoadPrefs (DatebookPreferenceType* prefsP);
void 	DatebookSavePrefs (void);

int32_t TimeToWait (void);

void 	DoSecurity (void);

Boolean ClearEditState (void);

status_t AppSwitchByCreator(uint32_t creator, uint16_t cmd, MemPtr cmdPBP, uint32_t cmdPBSize) ;

#ifdef __cplusplus
}
#endif

#endif //_DATEBOOK_H_

