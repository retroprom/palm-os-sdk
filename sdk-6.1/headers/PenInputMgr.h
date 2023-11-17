/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PenInputMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *  This header file describes the Pen Input Manager,
 *  part of Pen Input Services.
 *
 *****************************************************************************/

#ifndef __PENINPUTMGR_H__
#define __PENINPUTMGR_H__

#include <PalmTypes.h>
#include <SystemResources.h>


// Pen Input Manager Features
#define pinCreator           'pins'
#define pinFtrAPIVersion     1

// PINS API version numbers
// Preliminary 1.0 release from Garmin
#define pinAPIVersion1_0            0x01000000
// Version number 1.1 (for OS 5.3)
#define pinAPIVersion1_1            0x01103000
// Version number 2.0 (for OS 6.0)
#define pinAPIVersion2_0            0x02003000
// Version number 2.0.1 (for 6.1 -- added constants for Onscreen Input Areas)
#define pinAPIVersion2_0_1          0x02013000

// This header is for version 2.0.1
#define pinAPIVersion               pinAPIVersion2_0_1


// Pinlet settings Features
#define pinletFtrCreator            sysFileTPinletApp
#define pinletFtrNumFadeDelay       1


// Flags for the input area feature (sysFtrCreator/sysFtrNumInputAreaFlags)
#define grfFtrInputAreaFlagDynamic              0x00000001
#define grfFtrInputAreaFlagLiveInk              0x00000002
#define grfFtrInputAreaFlagCollapsible          0x00000004
#define grfFtrInputAreaFlagLandscape            0x00000008
#define grfFtrInputAreaFlagReversePortrait      0x00000010
#define grfFtrInputAreaFlagReverseLandscape     0x00000020
#define grfFtrInputAreaFlagLefthanded           0x00000040
#define grfFtrInputAreaFlagOnscreen             0x00000080


// Pen Input Manager errors
#define pinErrNoSoftInputArea (pinsErrorClass | 0x00)
#define pinErrInvalidParam    (pinsErrorClass | 0x01)
#define pinErrPinletNotFound  (pinsErrorClass | 0x02)


// Input area states
#define pinInputAreaOpen         0
#define pinInputAreaClosed       1
#define pinInputAreaNone         2
#define pinInputAreaOverlay      3
#define pinInputAreaStick        4
#define pinInputAreaUnstick      5


// Input modes
#define pinInputModeNormal       0
#define pinInputModeShift        1
#define pinInputModeCapsLock     2
#define pinInputModePunctuation  3
#define pinInputModeNumeric      4
#define pinInputModeExtended     5
#define pinInputModeHiragana     6
#define pinInputModeKatakana     7
// Pinlets may use custom input modes in the following range
#define pinInputModeCustomBase   128
#define pinInputModeCustomMax    255
// The following modes have special meanings.
#define pinInputModeUnShift      256


// Pinlet info selectors
#define pinPinletInfoName          0
#define pinPinletInfoStyle         1
#define pinPinletInfoFEPAssoc      2
#define pinPinletInfoIcon          3
#define pinPinletInfoComponentName 4


// Pinlet styles
#define pinPinletStyleHandwriting 0
#define pinPinletStyleKeyboard    1
#define pinPinletStyleOther       2
#define pinPinletStyleOverlay     3

// Default Pinlet names (for PINSwitchToPinlet)
#define pinDefaultHWR             "default:hwr"
#define pinDefaultKeyboard        "default:keyboard"

// Default Pinlet codes (for PINGet/SetDefaultPinlet)
#define pinDefaultPinletHWR       0
#define pinDefaultPinletKeyboard  1

// Character flags
#define pinCharFlagVirtual        0x00000001


#ifdef __cplusplus
extern "C" {
#endif


// Input area control
uint16_t PINGetInputAreaState(void);
status_t PINSetInputAreaState(uint16_t state);

// Alternative input systems
Boolean PINAltInputSystemEnabled(void);

// Pinlets
const char *PINGetCurrentPinletName(void);
status_t PINSwitchToPinlet(const char *pinletName, uint16_t initialInputMode);

// Pinlet info
uint16_t PINCountPinlets(void);
status_t PINGetPinletInfo(uint16_t index, uint16_t infoSelector, uint32_t *info);
uint16_t PINGetDefaultPinlet(uint16_t defaultCode);
status_t PINSetDefaultPinlet(uint16_t defaultCode, uint16_t index);

// Input mode
void PINSetInputMode(uint16_t inputMode);
uint16_t PINGetInputMode(void);

// Reset
void PINClearPinletState(void);

// Reference/help dialog
void PINShowReferenceDialog(void);


#ifdef __cplusplus
}
#endif


/* SYSTEM USE ONLY -- PLEASE IGNORE */

// Input area types
#define  kHardInputArea   0
#define  kSoftInputArea   1
#define  kNoInputArea     2


#endif /* __PENINPUTMGR_H__ */
