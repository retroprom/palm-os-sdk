/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: IOSAttributes.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	Header file for the driver head.
 *
 *****************************************************************************/

#ifndef __IOSAttributes_H__
#define	__IOSAttributes_H__

 #include "PalmTypes.h"
 
 
// Class IDs should all be registered creator codes
#define iosDriverClassGeneric	'cgen'
#define iosDriverClassSerial	'cser'
#define iosDriverClassEthernet	'ceth'
#define iosDriverClassSlot		'cslt'
#define iosDriverClassVolume	'cvol'
#define iosDriverClassAdmin		'cadm'
#define iosDriverClassWifi		'wifi'
#define iosDriverClassAll		'call'


#ifdef __cplusplus
extern "C" {
#endif

 status_t IOSGetNumDrivers(uint32_t iClassID, int16_t *oCount);
 
 status_t IOSGetDriverAttributesByIndex(uint32_t iClassID, int16_t iIndex, MemPtr ioBuf, uint16_t* ioBufLen);
 
 status_t IOSGetDriverAttributesByName(char const * iIOSName, MemPtr ioBuf, uint16_t* ioBufLen);
 
 status_t IOSGetDriverDescriptionByIndex(uint32_t iClassID, int16_t iIndex, char* ioBuf, uint16_t* ioBufLen);
 
 status_t IOSGetDriverDescriptionByName(char const * iIOSName, char* ioBuf, uint16_t* ioBufLen);
 
 status_t IOSGetDriverNameByIndex(uint32_t iClassID, int16_t iIndex, char* ioBuf, uint16_t* ioBufLen);

#ifdef __cplusplus
}
#endif

 #endif // __IOSAttributes_H__
