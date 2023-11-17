/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SecurityServices.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef __SECURITY_SERVICES_H__
#define __SECURITY_SERVICES_H__

#include <PalmTypes.h>
#include <CmnErrors.h>

#define secSvcsErrNotImplemented     ((status_t)(secSvcsErrorClass | 1))
#define secSvcsErrBufferTooSmall     ((status_t)(secSvcsErrorClass | 2))
#define secSvcsErrNoPolicies         ((status_t)(secSvcsErrorClass | 3))
#define secSvcsErrUnauthorized       ((status_t)(secSvcsErrorClass | 4))
#define secSvcsErrOutOfMemory        ((status_t)(secSvcsErrorClass | 5))
#define secSvcsErrServiceNotStarted  ((status_t)(secSvcsErrorClass | 6))
#define secSvcsErrInvalid            ((status_t)(secSvcsErrorClass | 7))

#define SecSvcsServiceName      "pSysSecSvcs"

typedef enum {
  SecSvcsDeviceSecurityNone = 0,
  SecSvcsDeviceSecurityMedium,
  SecSvcsDeviceSecurityHigh
} SecSvcsDeviceSettingEnum;

typedef enum {
  SecSvcsDeviceLockoutNever = 0,
  SecSvcsDeviceLockoutPowerOff,
  SecSvcsDeviceLockoutAt,
  SecSvcsDeviceLockoutAfter
} SecSvcsDeviceLockoutEnum;

  /******************************************************************
 * Entry numbers of exported functions. Must conform to the MDF
 ******************************************************************/
#define entryNumSecSvcsGetDevicePolicies (0)
#define entryNumSecSvcsGetDeviceSetting  (1)
#define entryNumSecSvcsSetDeviceSetting  (2)
#define entryNumSecSvcsGetDeviceLockout  (3)
#define entryNumSecSvcsSetDeviceLockout  (4)
#define entryNumSecSvcsEncodeLockoutTime (5)
#define entryNumSecSvcsDecodeLockoutTime (6)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SecSvcsGetSecPolicies
 *
 */
status_t SecSvcsGetDevicePolicies(uint32_t creatorID, uint8_t *buffer, uint32_t *buflen);

typedef status_t (*SecSvcsGetDevicePoliciesPtrType)(uint32_t creatorID, uint8_t *buffer, uint32_t *buflen);

/*
 * SecSvcsGetDeviceSetting
 */
status_t SecSvcsGetDeviceSetting(SecSvcsDeviceSettingEnum *level);

typedef status_t (*SecSvcsGetDeviceSettingPtrType)(SecSvcsDeviceSettingEnum *level);

/*
 * SecSvcsSetDeviceSetting
 */
status_t SecSvcsSetDeviceSetting(SecSvcsDeviceSettingEnum level);

typedef status_t (*SecSvcsSetDeviceSettingPtrType)(SecSvcsDeviceSettingEnum level);

/*
 * SecSvcsGetDeviceLockout
 */
status_t SecSvcsGetDeviceLockout(uint32_t *encoded_level);

typedef status_t (*SecSvcsGetDeviceLockoutPtrType)(uint32_t *encoded_level);

/*
 * SecSvcsSetDeviceLockout
 */
status_t SecSvcsSetDeviceLockout(uint32_t encoded_level);

typedef status_t (*SecSvcsSetDeviceLockoutPtrType)(uint32_t encoded_level);

/*
 * SecSvcsEncodeLockoutTime
 */
status_t SecSvcsEncodeLockoutTime(SecSvcsDeviceLockoutEnum lockoutType, uint32_t *encoded_level, uint32_t hours, uint32_t minutes);

typedef status_t (*SecSvcsEncodeLockoutTimePtrType)(SecSvcsDeviceLockoutEnum lockoutType, uint32_t *encoded_level, uint32_t hours, uint32_t minutes);

/*
 * SecSvcsDecodeLockoutTime
 */
status_t SecSvcsDecodeLockoutTime(uint32_t encoded_level, SecSvcsDeviceLockoutEnum *lockoutType, uint32_t *hours, uint32_t *minutes);

typedef status_t (*SecSvcsDecodeLockoutTimePtrType)(uint32_t encoded_level, SecSvcsDeviceLockoutEnum *lockoutType, uint32_t *hours, uint32_t *minutes);

/*
 * SecSvcsIsDeviceLocked
 */
Boolean SecSvcsIsDeviceLocked(void);

typedef Boolean (*SecSvcsIsDeviceLockedPtrType)(void);

/*
 * SecSvcsSetDeviceLocked
 */
status_t SecSvcsSetDeviceLocked(Boolean locked);

typedef status_t (*SecSvcsSetDeviceLockedPtrType)(Boolean locked);

#ifdef __cplusplus
}
#endif

#endif /* __SECURITY_SERVICES_H__ */
