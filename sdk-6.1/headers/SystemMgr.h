/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SystemMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Pilot system equates
 *
 *****************************************************************************/

#ifndef __SYSTEMMGR_H__
#define __SYSTEMMGR_H__

#include <PalmTypes.h>
#include <CmnBatteryTypes.h>
#include <CmnLaunchCodes.h>
#include <Event.h>
#include <SystemResources.h>		// Resource definitions.


/************************************************************
 * System Constants
 *************************************************************/

// Power Manager error codes - soe, srj 9/19/00
#define	pwrErrNone								(pwrErrorClass | 0)
#define	pwrErrBacklight							(pwrErrorClass | 1)
#define	pwrErrRadio								(pwrErrorClass | 2)
#define	pwrErrBeam								(pwrErrorClass | 3)
#define pwrErrGeneric							(pwrErrorClass | 4)

// Orientation states
#define sysOrientationUser             			0
#define sysOrientationPortrait         			1
#define sysOrientationLandscape        			2
#define sysOrientationReversePortrait  			3
#define sysOrientationReverseLandscape 			4

// Orientation trigger states
#define sysOrientationTriggerDisabled  			0
#define sysOrientationTriggerEnabled   			1


/************************************************************
 * System Features
 *************************************************************/
#define	sysFtrCreator			sysFileCSystem		// Feature Creator

#define	sysFtrNumROMVersion				1			// ROM Version
			// 0xMMmfsbbb, where MM is major version, m is minor version
			// f is bug fix, s is stage: 3-release,2-beta,1-alpha,0-development,
			// bbb is build number for non-releases 
			// V1.12b3   would be: 0x01122003
			// V2.00a2   would be: 0x02001002
			// V1.01     would be: 0x01013000

#define	sysFtrNumProcessorID			2			// Product id
			// 0xMMMMRRRR, where MMMM is the processor model and RRRR is the revision.
#define	sysFtrNumProcessorMask		0xFFFF0000		// Mask to obtain processor model
#define	sysFtrNumProcessor328		0x00010000		// Motorola 68328		(Dragonball)
#define	sysFtrNumProcessorEZ		0x00020000		// Motorola 68EZ328	(Dragonball EZ)
#define	sysFtrNumProcessorVZ		0x00030000		// Motorola 68VZ328	(Dragonball VZ)
#define	sysFtrNumProcessorSuperVZ	0x00040000		// Motorola 68SZ328	(Dragonball SuperVZ)
#define	sysFtrNumProcessorARM720T	0x00100000		// ARM 720T
#define sysFtrNumProcessorARM7TDMI	0x00110000		// ARM7TDMI
#define sysFtrNumProcessorARM920T	0x00120000		// ARM920T
#define sysFtrNumProcessorARM922T	0x00130000		// ARM922T
#define sysFtrNumProcessorARM925	0x00140000		// ARM925
#define sysFtrNumProcessorStrongARM	0x00150000		// StrongARM
#define sysFtrNumProcessorXscale	0x00160000		// Xscale
#define sysFtrNumProcessorARM710A	0x00170000		// ARM710A
#define	sysFtrNumProcessorx86		0x01000000		// Intel CPU		(Palm Simulator)
// The following sysFtrNumProcessorIs68K(x) and sysFtrNumProcessorIsARM(x)
// macros are intended to be used to test the value returned from a call to
//    FtrGet(sysFtrCreator, sysFtrNumProcessorID, &value);
// in order to determine if the code being executed is running on a 68K or ARM processor.
 
#define sysFtrNumProcessor68KIfZero    0xFFF00000   // 68K if zero; not 68K if non-zero
#define sysFtrNumProcessorIs68K(x)     (((x&sysFtrNumProcessor68KIfZero)==0)? true : false)
#define sysFtrNumProcessorARMIfNotZero 0x00F00000   // ARM if non-zero
#define sysFtrNumProcessorIsARM(x)     (((x&sysFtrNumProcessorARMIfNotZero)!=0)? true : false)

#define	sysFtrNumProductID	sysFtrNumProcessorID	// old (obsolete) define

#define	sysFtrNumBacklight				3			// Backlight
			// bit 0:	1 if present. 0 if Feature does not exist or backlight is not present

#define	sysFtrNumEncryption				4			// Which encryption schemes are present
#define	sysFtrNumEncryptionMaskDES		0x00000001	// bit 0:	1 if DES is present

#define	sysFtrNumCountry68K				5			// International ROM identifier
			// This was sysFtrNumCountry - you should now call LmGetSystemLocale to determine
			// the base language & country being used by the OS, or DmGetOverlayLocale to
			// determine which overlays will be automatically opened by the Data Mgr. Note
			// that the feature value is still used by 68K apps, and it will contain the
			// old country values (enums from 0...n) versus two letter ISO country names.
			// Result is of type CountryType as defined in LocaleMgr.h.
			// Result is essentially the "default" country for this ROM.
			// Assume cUnitedStates if sysFtrNumROMVersion >= 02000000
			// and feature does not exist. Result is in low sixteen bits.
			
#define	sysFtrNumLanguage68K			6			// Language identifier
			// This was sysFtrNumLanguage - you should now call LmGetSystemLocale to determine
			// the base language & country being used by the OS, or DmGetOverlayLocale to
			// determine which overlays will be automatically opened by the Data Mgr. Note
			// that the feature value is still used by 68K apps, and it will contain the
			// old language values (enums from 0...n) versus two letter ISO language names.
			// Result is of type LanguageType as defined in LocaleMgr.h
			// Result is essentially the "default" language for this ROM.
			// This is new for the WorkPad (v2.0.2) and did NOT exist for any of the
			// following: GermanPersonal, GermanPro, FrenchPersonal, FrenchPro
			// Thus we can't really assume anything if the feature doesn't exist,
			// though the actual language MAY be determined from sysFtrNumCountry,
			// above. Result is in low sixteen bits.

#define	sysFtrNumDisplayDepth			7			// Display depth
			// Result is the "default" display depth for the screen.				(PalmOS 3.0)
			// This value is used by ScrDisplayMode when setting the default display depth.
			
#define	sysFtrNumHwrMiscFlags			8			// GHwrMiscFlags value			(PalmOS 3.1)
#define	sysFtrNumHwrMiscFlagsExt		9			// GHwrMiscFlagsExt value		(PalmOS 3.1)
			
#define	sysFtrNumTextMgrFlags			10
			// This was sysFtrNumIntlMgr, but the Int'l manager no longer exists in 6.0.
			// Result is a set of flags that define functionality supported by the
			// Text Manager.														(PalmOS 3.1)

#define	sysFtrNumEncoding68K			11
			// This was sysFtrNumEncoding - you should now call LmGetSystemLocale() to
			// get the character encoding. 68K apps will still use the feature, though.
			// Result is the character encoding (defined in TextMgr.h) supported
			// by this ROM. If this feature doesn't exist then the assumed encoding
			// is PalmLatin (superset of Windows code page 1252)					(PalmOS 3.1)
			
#define	sysFtrDefaultFont				12
			// Default font ID used for displaying text.							(PalmOS 3.1)

#define	sysFtrDefaultBoldFont			13
			// Default font ID used for displaying bold text.						(PalmOS 3.1)

#define	sysFtrNumVendor					15
			// Result is the vendor id, in the low sixteen bits.					(PalmOS 3.3)

			// DOLATER: Need more information about what to expect in this feature.


#define	sysFtrNumCharEncodingFlags68K	16
			// This was sysFtrNumCharEncodingFlags - you should now call TxtGetEncodingFlags,
			// passing in the encoding returned by LmGetSystemLocale(NULL). The flags are
			// defined in Textmgr.h													(PalmOS 3.5)
			
#define	sysFtrNumNotifyMgrVersion		17 			// version of NotifyMgr, if any	(PalmOS 3.5)
			
#define	sysFtrNumOEMROMVersion			18			// Supplemental ROM version, provided by OEM
			// This value may be present in OEM devices, and is in the same format
			// as sysFtrNumROMVersion.												(PalmOS 3.5)

#define	sysFtrNumROMBuildType			19			// ROM build setting of BUILD_TYPE
			// May be set to BUILD_TYPE_DEBUG or BUILD_TYPE_RELEASE
			// as defined in <BuildDefines.h>.										(PalmOS 3.5)
			// NOTE: Renamed from the old sysFtrNumErrorCheckLevel in PalmOS 6.0.  
			// The values are compatible (1=release, 2=debug).

#define	sysFtrNumOEMCompanyID			20			// GHwrOEMCompanyID value		(PalmOS 3.5)
#define	sysFtrNumOEMDeviceID			21			// GHwrOEMDeviceID value		(PalmOS 3.5)
#define	sysFtrNumOEMHALID				22			// GHwrOEMHALID value			(PalmOS 3.5)
#define	sysFtrNumDefaultCompression		23			// Default Clipper's compression (Palmos 3.5)

#define sysFtrNumWinVersion				24			// Window version				(PalmOS 4.0)

#define	sysFtrNumAccessorTrapPresent	25			// If accessor trap exists		(PalmOS 4.0)

#define sysFtrNumInputAreaFlags			26			// Dynamic Input Area flags		(PalmOS 5.3)

#define sysFtrNumUIHardwareFlags		27			// Additional user input hardware	(PalmOS 5.3)

#define	sysFtrNumResetType				28			// See ResetTypes.h				(PalmOS 6.0)

#define sysFtrNumFastBoot				29			// Enable fast minimal boot		(PalmOS 6.0)
			// When non-zero, various parts of the boot process will be shortened or
			// completely skipped.  This mode is ONLY for development, to reduce
			// the amount of waiting required each time a new build is tested.

#define sysFtrNumDisplayUpdateMode		30			// How is display updated?		(PalmOS 6.0.1)
			// 0 -> Drawing occurs directly to the screen, allowing flicker.
			// 1 -> The display uses double buffering (page flipping) to eliminate
			//		flicker when drawing.
			// Additional values will be defined in the future for other methods
			// of eliminating flicker; a value of 0 will always mean that drawing
			// in update-based windows flickers.

#define sysFtrNumDmAutoBackup			31			// no battery backup 

#define sysFtrNumFiveWayNavVersion		32			// version of the 5-way nav if any (PalmOS 5.4, 6.1)

#define sysFtrNumDisableSortDuringSyncThreshold		33
			//
			// If this feature is not set, then database sorting is always disabled 
			// during HotSync (PalmOS 6.1)
			//
			// If this feature is set, and the value is --
			//  == 0, sorting of database is enabled during sync.
			//  >  0, and if the number of rows to sync using bulk 
			//        APIs is greater than or equal to this value, 
			//        sorting is disabled from this point onwards. 
			//        Otherwise, sorting is enabled during sync.
			//
			//  Licensees are free to change/customize this value
			//	for release on their hardware.
			//

#define sysFtrNumEnableSortAfterSyncThreshold		34
			//
			// If this feature is not set, then database is not sorted 
			// on close during HotSync (PalmOS 6.1)
			//
			// If this feature is set, and the value is --
			//	== 0, database is sorted on close, if it is not already sorted
			//  >  0, and if this value is less than or equal to the number 
			//        of rows in the database, database is sorted on close if
			//        it is not already sorted.
			//
			// Otherwise, database is not sorted on close
			//
			// Licensees are free to change/customize this value
			// for release on their hardware.
			//

#define sysFtrNumSkipCalibration		35

/************************************************************
 * ROM token information (for SysGetROMToken, below)
 *************************************************************/
// Additional tokens and token information is located in <Hardware.h>
#define	sysROMTokenSnum			'snum'	// Memory Card Flash ID (serial number)



/************************************************************
 * Assigned values for sysFtrNumOEMCompanyID (and *HALID, and *DeviceID)
 *************************************************************/

// Values are assigned by the PalmSource Platform Engineering group.
// Note: These values are different from the values that may be found in some
// OEM devices which used HwrROMTokens on versions of Palm OS prior to 3.5.

#define sysOEMCompanyIDUnspecified	0x00000000		// sysOEMCompanyID not specified by HAL
#define sysOEMHALIDUnspecified		0x00000000		// sysOEMHALID not specified by HAL
#define sysOEMDeviceIDUnspecified	0x00000000		// sysOEMDeviceID not specified by HAL

#define sysOEMCompanyIDPalmPlatform	'psys'			// Reference Platforms made by PalmSource
#define sysOEMCompanyIDPalmDevices	'palm'			// Devices made by Palm
#define sysOEMCompanyIDSymbol		'smbl'			// Devices made by Symbol Technologies
#define sysOEMCompanyIDQualcomm		'qcom'			// Devices made by Qualcomm
#define sysOEMCompanyIDTRG			'trgp' 			// Devices made by TRG Products
#define sysOEMCompanyIDHandspring	'hspr'			// Devices made by Handspring



/************************************************************
 * Macros for extracting and combining ROM/OS version components
 *************************************************************/

// ROM/OS stage numbers
#define sysROMStageDevelopment			(0)
#define sysROMStageAlpha				(1)
#define sysROMStageBeta					(2)
#define sysROMStageRelease				(3)


// MACRO: sysMakeROMVersion
//
// Builds a ROM version value from the major, minor, fix, stage, and build numbers
//
#define sysMakeROMVersion(major, minor, fix, stage, buildNum)			\
		(																					\
		(((uint32_t)((uint8_t)(major) & 0x0FF)) << 24) |							\
		(((uint32_t)((uint8_t)(minor) & 0x00F)) << 20) |							\
		(((uint32_t)((uint8_t)(fix)   & 0x00F)) << 16) |							\
		(((uint32_t)((uint8_t)(stage) & 0x00F)) << 12) |							\
		(((uint32_t)((uint16_t)(buildNum) & 0x0FFF)))								\
		)


// Macros for parsing the ROM version number
// (the system OS version is obtained by calling
// FtrGet(sysFtrCreator, sysFtrNumROMVersion, dwOSVerP), where dwOSVerP is
// a pointer to to a uint32_t variable that is to receive the OS version number)
#define sysGetROMVerMajor(dwROMVer)		(((uint16_t)((dwROMVer) >> 24)) & 0x00FF)
#define sysGetROMVerMinor(dwROMVer)		(((uint16_t)((dwROMVer) >> 20)) & 0x000F)
#define sysGetROMVerFix(dwROMVer)		(((uint16_t)((dwROMVer) >> 16)) & 0x000F)
#define sysGetROMVerStage(dwROMVer)		(((uint16_t)((dwROMVer) >> 12)) & 0x000F)
#define sysGetROMVerBuild(dwROMVer)		(((uint16_t)(dwROMVer))         & 0x0FFF)




/************************************************************
 * System Types
 *************************************************************/

/************************************************************
 * System Pre-defined "file descriptors"
 * These are used by applications that use the  Net Library's 
 *   NetLibSelect() call 
 *************************************************************/
#define	sysFileDescStdIn						0


/************************************************************
 * Entry number of 'main'
 *************************************************************/
#define sysEntryNumMain							((uint32_t)0xffffffff)


/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

uint16_t	SysBatteryInfo(Boolean set, uint16_t* warnThresholdPercentP, uint16_t* criticalThresholdPercentP, uint16_t *shutdownThresholdPercentP, 
			uint32_t* maxMilliSecsP, SysBatteryKind* kindP, Boolean* pluggedInP, uint8_t * percentP);

status_t	SysGetROMToken(uint32_t token, uint8_t **dataP, uint16_t *sizeP);

Boolean		SysHandleEvent(EventPtr eventP);

uint16_t 	SysSetAutoOffTime(uint16_t seconds);

void 		SysSleep(void);

void 		SysRequestSleep(void);

void		SysWakeupUI(void);

status_t	SysTaskDelay(int32_t delayInMilliSecs);

uint16_t 	SysUIBusy(Boolean set, Boolean value);

uint8_t		SysLCDContrast(Boolean set, uint8_t newContrastLevel);

uint8_t		SysLCDBrightness(Boolean set, uint8_t newBrightnessLevel);

void		SysTurnDeviceOn(void);

uint16_t	SysGetOrientation(void);

status_t	SysSetOrientation(uint16_t orientation);

uint16_t	SysGetOrientationTriggerState(void);

status_t	SysSetOrientationTriggerState(uint16_t triggerState);


/************************************************************
 * Time conversion macros - DOLATER - is this the best place for these?
 *************************************************************/

// macros to generate system time of various flavors, system time
// will always be in milliseconds (1/1000 of a second)

// create a system time (milliSecs) from a value in microseconds
#define SysTimeInMicroSecs(microSecs) ((microSecs) / 1000)
// create a system time (milliSecs) from a value in milliseconds
#define SysTimeInMilliSecs(milliSecs) (milliSecs)
// create a system time (milliSecs) from a value in centiseconds
#define SysTimeInCentiSecs(centiSecs) ((centiSecs) * 10)
// create a system time (milliSecs) from a value in seconds
#define SysTimeInSecs(secs) ((secs) * 1000)
// create a system time (milliSecs) from a value in minutes
#define SysTimeInMins(mins) ((mins) * 60 * 1000)

// convert a system time value to seconds
#define SysTimeToSecs(sysTime) ((sysTime) / 1000)
// convert a system time value to milliseconds
#define SysTimeToMilliSecs(sysTime) (sysTime)
// convert a system time value to microseconds
#define SysTimeToMicroSecs(sysTime) ((sysTime) * 1000)

// Deprecated APIs...
#define SysTicksPerSecond()			1000

#ifdef __cplusplus 
}
#endif

#endif  //__SYSTEMMGR_H__
