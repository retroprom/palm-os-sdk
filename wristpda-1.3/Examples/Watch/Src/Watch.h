/*
 * Watch.h
 *
 * header file for Watch
 *
 * This wizard-generated code is based on code adapted from the
 * stationery files distributed as part of the Palm OS SDK 4.0.
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 */
 
#ifndef WATCH_H_
#define WATCH_H_

/*********************************************************************
 * Internal Structures
 *********************************************************************/

typedef struct WatchPreferenceType
{
	UInt16 WatchFaceIndex;
} WatchPreferenceType;

typedef enum { Analog, Digital } FaceType;

typedef enum { Hour12, Hour24 } HourType;

typedef enum { DayDate1, DayDate2, DayDate3, DayDate4, DayDate5, 
			   DayDate6, DayDate7, DayDate8, DayDate9, DayDate10,
			   DayDateNo } DayDateType;

typedef enum { Logo1, Logo2, Logo3, Logo4, LogoNo } LogoType;

typedef enum { InvertedYes, InvertedNo } InvertedType;

typedef struct AnalogFaceType
{
	UInt16	Type;
	UInt16	DayDate;
	UInt16	FaceResId;
	UInt16	BaseWidth;
	UInt16	HourLen;
	UInt16	MinuteLen;
	UInt16	HandOffsetX;
	Int16	HandOffsetY;
	Int16	FontIndex;
	UInt16	Inverted;
	UInt16	ReservedPadding1;
	UInt16	ReservedPadding2;
	UInt16	ReservedPadding3;
	UInt16	ReservedPadding4;
	UInt16	ReservedPadding5;
} AnalogFaceType;

typedef struct DigitalFaceType
{
	UInt16	Type;
	UInt16	Hour;
	UInt16	DayDate;
	UInt16	FaceResId;
	UInt16	NumberResIdBase;
	UInt16	NumberResIdInc;
	UInt16	SeparatorResId;
	UInt16	ExtraSpace;
	Int16	OffsetX;
	Int16	OffsetY;
	UInt16	FontIndex;
	UInt16	LogoResId;
	UInt16	LogoStyle;
	UInt16	LogoYCoord;
	UInt16	Inverted;
} DigitalFaceType;

typedef union WatchFaceType
{
	AnalogFaceType  Analog;
	DigitalFaceType Digital;
} WatchFaceType;

/*********************************************************************
 * Global variables
 *********************************************************************/

extern WatchPreferenceType g_prefs;

/*********************************************************************
 * Internal Constants
 *********************************************************************/

#define appName					"Watch"
#define appVersionNum			0x01
#define appPrefVersionNum		0x01

#define	AlarmResId				100
#define	BatteryLowResId			101
#define	BatteryVeryLowResId		102
#define	BatteryEmptyResId		103

#endif /* WATCH_H_ */
