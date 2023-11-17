/**
 * \file PhoneLibTraps.h
 *
 * Handspring Phone Library
 *
 * Public Interface for the Phone Library
 *
 * \license
 *
 * Copyright (c) 2002 Handspring Inc. & Option International
 * All Rights Reserved
 *
 * $Id: $
 *
 ***************************************************************/

#ifdef __MWERKS__
#pragma once
#endif

#ifndef _PHONELIBTRAPS_H_
#define _PHONELIBTRAPS_H_

// Trap IDs for the GSM library's public functions.
#define PhnLibTrapGetLibAPIVersion          sysLibTrapCustom
#define PhnLibTrapUninstall                 (sysLibTrapCustom + 1)
#define PhnLibTrapRegister                  (sysLibTrapCustom + 2)
#define PhnLibTrapFirstAppForService        (sysLibTrapCustom + 3)
#define PhnLibTrapNewAddress                (sysLibTrapCustom + 4)
#define PhnLibTrapGetField                  (sysLibTrapCustom + 5)
#define PhnLibTrapGetID                     (sysLibTrapCustom + 6)
#define PhnLibTrapSetField                  (sysLibTrapCustom + 7)
#define PhnLibTrapSetNumber                 (sysLibTrapCustom + 8)
#define PhnLibTrapSetID                     (sysLibTrapCustom + 9)
#define PhnLibTrapAddressToText             (sysLibTrapCustom + 10)
#define PhnLibTrapHasName                   (sysLibTrapCustom + 11)
#define PhnLibTrapEqual                     (sysLibTrapCustom + 12)
#define PhnLibTrapSendDTMF                  (sysLibTrapCustom + 14)
#define PhnLibTrapGetCLIP                   (sysLibTrapCustom + 29)
#define PhnLibTrapSetOperatorLock           (sysLibTrapCustom + 30)
#define PhnLibTrapGetOperatorLock           (sysLibTrapCustom + 31)
#define PhnLibTrapCurrentOperator           (sysLibTrapCustom + 34)
#define PhnLibTrapCurrentProvider           (sysLibTrapCustom + 35)
#define PhnLibTrapGetOperatorList           (sysLibTrapCustom + 36)
#define PhnLibTrapSetOperator               (sysLibTrapCustom + 37)
#define PhnLibTrapGetOwnNumbers             (sysLibTrapCustom + 38)
#define PhnLibTrapGetRingingInfo            (sysLibTrapCustom + 40)
#define PhnLibTrapSetRingingInfo            (sysLibTrapCustom + 41)
#define PhnLibTrapGetToneIDs                (sysLibTrapCustom + 42)
#define PhnLibTrapGetToneName               (sysLibTrapCustom + 43)
#define PhnLibTrapPlayTone                  (sysLibTrapCustom + 44)
#define PhnLibTrapStopTone                  (sysLibTrapCustom + 45)
#define PhnLibTrapFindEntry                 (sysLibTrapCustom + 46)
#define PhnLibTrapSelectAddress             (sysLibTrapCustom + 47)
#define PhnLibTrapGetMicrophone             (sysLibTrapCustom + 48)
#define PhnLibTrapSetMicrophone             (sysLibTrapCustom + 49)
#define PhnLibTrapGetVolume                 (sysLibTrapCustom + 50)
#define PhnLibTrapSetVolume                 (sysLibTrapCustom + 51)
#define PhnLibTrapRegistered                (sysLibTrapCustom + 52)
#define PhnLibTrapRoaming                   (sysLibTrapCustom + 53)
#define PhnLibTrapSignalQuality             (sysLibTrapCustom + 54)
#define PhnLibTrapErrorRate                 (sysLibTrapCustom + 55)
#define PhnLibTrapBattery                   (sysLibTrapCustom + 56)
#define PhnLibTrapCardInfo                  (sysLibTrapCustom + 57)
#define PhnLibTrapSIMInfo                   (sysLibTrapCustom + 58)
#define PhnLibTrapGetSIMStatus              (sysLibTrapCustom + 59)
#define PhnLibTrapParamText                 (sysLibTrapCustom + 60)
#define PhnLibTrapGetErrorText              (sysLibTrapCustom + 61)
#define PhnLibTrapGetDBRef                  (sysLibTrapCustom + 62)
#define PhnLibTrapReleaseDBRef              (sysLibTrapCustom + 63)
#define PhnLibTrapNewMessage                (sysLibTrapCustom + 64)
#define PhnLibTrapDeleteMessage             (sysLibTrapCustom + 65)
#define PhnLibTrapSendMessage               (sysLibTrapCustom + 66)
#define PhnLibTrapSetText                   (sysLibTrapCustom + 67)
#define PhnLibTrapSetDate                   (sysLibTrapCustom + 68)
#define PhnLibTrapSetOptions                (sysLibTrapCustom + 69)
#define PhnLibTrapSetAddresses              (sysLibTrapCustom + 70)
#define PhnLibTrapSetStatus                 (sysLibTrapCustom + 71)
#define PhnLibTrapSetFlags                  (sysLibTrapCustom + 72)
#define PhnLibTrapSetOwner                  (sysLibTrapCustom + 73)
#define PhnLibTrapGetText                   (sysLibTrapCustom + 74)
#define PhnLibTrapGetDate                   (sysLibTrapCustom + 75)
#define PhnLibTrapGetOptions                (sysLibTrapCustom + 76)
#define PhnLibTrapGetAddresses              (sysLibTrapCustom + 77)
#define PhnLibTrapGetStatus                 (sysLibTrapCustom + 78)
#define PhnLibTrapGetFlags                  (sysLibTrapCustom + 79)
#define PhnLibTrapGetOwner                  (sysLibTrapCustom + 80)
#define PhnLibTrapGetType                   (sysLibTrapCustom + 81)
#define PhnLibTrapIsLegalCharacter          (sysLibTrapCustom + 82)
#define PhnLibTrapMapCharacter              (sysLibTrapCustom + 83)
#define PhnLibTrapSetServiceCentreAddress   (sysLibTrapCustom + 84)
#define PhnLibTrapGetServiceCentreAddress   (sysLibTrapCustom + 85)
#define PhnLibTrapLength                    (sysLibTrapCustom + 86)
#define PhnLibTrapGetSubstitution           (sysLibTrapCustom + 87)
#define PhnLibTrapNewAddressList            (sysLibTrapCustom + 88)
#define PhnLibTrapAddAddress                (sysLibTrapCustom + 90)
#define PhnLibTrapGetNth                    (sysLibTrapCustom + 91)
#define PhnLibTrapCount                     (sysLibTrapCustom + 93)
#define PhnLibTrapSetModulePower            (sysLibTrapCustom + 94)
#define PhnLibTrapModulePowered             (sysLibTrapCustom + 95)
#define PhnLibTrapModuleButtonDown          (sysLibTrapCustom + 96)
#define PhnLibTrapBoxInformation            (sysLibTrapCustom + 97)
#define PhnLibTrapGetBoxNumber              (sysLibTrapCustom + 98)
#define PhnLibTrapGetDataApplication        (sysLibTrapCustom + 99)
#define PhnLibTrapSetDataApplication        (sysLibTrapCustom + 100)
#define PhnLibTrapDebug                     (sysLibTrapCustom + 101)
#define PhnLibTrapGetPhoneBook				(sysLibTrapCustom + 102)
#define PhnLibTrapSetPhoneBook				(sysLibTrapCustom + 103)
#define PhnLibTrapGetPhoneBookIndex			(sysLibTrapCustom + 105)
#define PhnLibTrapBatteryCharge				(sysLibTrapCustom + 106)
#define PhnLibTrapGetEchoCancellation		(sysLibTrapCustom + 107)
#define PhnLibTrapSetEchoCancellation		(sysLibTrapCustom + 108)
#define PhnLibTrapGetEquipmentMode			(sysLibTrapCustom + 109)
#define PhnLibTrapSetEquipmentMode			(sysLibTrapCustom + 110)
#define PhnLibTrapGetSMSRingInfo			(sysLibTrapCustom + 111)
#define PhnLibTrapSetSMSRingInfo			(sysLibTrapCustom + 112)
#define PhnLibTrapPlayDTMF                  (sysLibTrapCustom + 113)
#define PhnLibTrapStartVibrate              (sysLibTrapCustom + 114)
#define PhnLibTrapStopVibrate               (sysLibTrapCustom + 115)
#define PhnLibTrapHomeOperatorID			(sysLibTrapCustom + 120)
#define PhnLibTrapCurrentOperatorID			(sysLibTrapCustom + 121)
#define PhnLibTrapUsableSignalStrengthThreshold	(sysLibTrapCustom + 122)
#define PhnLibTrapConnectionAvailable		(sysLibTrapCustom + 123)
#define PhnLibTrapGetPhoneCallStatus		(sysLibTrapCustom + 124)
#define PhnLibTrapSendSilentDTMF			(sysLibTrapCustom + 125)
#define PhnLibTrapGetCLIR					(sysLibTrapCustom + 127)
#define PhnLibTrapGetSMSGateway				(sysLibTrapCustom + 130)
#define PhnLibTrapGPRSAttached              (sysLibTrapCustom + 132)
#define PhnLibTrapGPRSPDPContextDefine      (sysLibTrapCustom + 133)
#define PhnLibTrapGPRSPDPContextListConstruct (sysLibTrapCustom + 134)
#define PhnLibTrapGPRSPDPContextListDestruct  (sysLibTrapCustom + 135)
#define PhnLibTrapGPRSQualityOfServiceGet	(sysLibTrapCustom + 137)
#define PhnLibTrapGetBinary                 (sysLibTrapCustom + 140)
#define PhnLibTrapNetworkAvailable          (sysLibTrapCustom + 141)
#define PhnLibTrapLast                      (sysLibTrapCustom + 142)

#endif

