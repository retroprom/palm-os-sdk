/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IOSDriver.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Header file for all IOS Drivers.
 *
 *****************************************************************************/

#ifndef __IOSDriver_H__
#define	__IOSDriver_H__

#include <PalmTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Driver Types
//
// An IOS driver may be type iosNonStreamsDriver, iosStreamsModule, or iosStreamsDriver.
// The driver types iosNamedPipe and iosVirtualName are used for
// non-driver names that have been added to the name table by IOS.

typedef enum {
	iosNonStreamsDriver,
	iosStreamsModule,
	iosStreamsDriver,
	iosNamedPipe,
	iosVirtualName,
	iosNonIOSCodeModule
} IOSDriverTypeEnum;

//
// Adminstration Functions and Structures
//
// 

typedef struct IOSDriverInstallTag IOSDriverInstallType;

typedef uint32_t DriverInitParamsType;

typedef void (*DrvrInstallQueryPtr)( IOSDriverInstallType* ioInstallParams);

typedef status_t (*DrvrInitFuncPtr)(DriverInitParamsType* iInitParams);

typedef status_t (*DrvrDeInitFuncPtr)( void );

typedef struct IOSInstallAdminTag {
	DrvrInitFuncPtr			driverInit;
	DrvrDeInitFuncPtr		driverDeInit;
} IOSInstallAdminType;

//
// Driver Attributes Structure
// 

typedef struct IOSDriverAttributesTag {
	const char*	description;
	uint32_t	classID;
	MemPtr	classAttributes;
	uint32_t	classAttribLen;
} IOSDriverAttributesType;

//
// STREAMS Drivers and Modules Configuration Structure
//

typedef struct IOSStreamsConfigType {
	uint32_t stackSize;
	MemPtr streamTab;
	uint32_t MPSFlags;
} IOSStreamsConfigType;


//
// Non-STREAMS Driver Configuration Structure
// 

typedef struct IOSNSDriverConfigTag {
	uint32_t stackSize;
	uint32_t threadTag;
	uint32_t maxRcvBufSize;  // Needed to support fast ioctl.
} IOSNSDriverConfigType;



//
// STREAMS Name Entry Structures
//

typedef struct IOSStreamsNameEntryTag IOSStreamsNameEntryType;

struct IOSStreamsNameEntryTag {
	const char*				name;
	int32_t					minor;
	IOSDriverAttributesType* attributes;
	IOSStreamsNameEntryType* next;
};

//
// IOS Driver Installation structure
//
typedef union IOSConfigTag {
		IOSStreamsConfigType * streams;
		IOSNSDriverConfigType * nsd;
	} IOSConfigType;
	
struct IOSDriverInstallTag {
	IOSInstallAdminType*	admin;
	MemPtr					nameList;
	IOSConfigType           config;
	IOSDriverTypeEnum		driverType;
	uint8_t					numDriver;
	uint16_t					wReserved;
	uint32_t					dwReserved;
};



// Define the minor number for Clone Opens in Streams
#define IOS_CLONE_MINOR (-1)


// Define the default flags for STREAMS Drivers as 
//     (SQLVL_MODULE << 8) + F_MODSW_IS_DEVICE
#define IOS_STREAMS_DRIVER_FLAGS  (0x302)

// Define the default flags for STREAMS Modules as 
//     (SQLVL_MODULE << 8)
#define IOS_STREAMS_MODULE_FLAGS  (0x300)

#ifdef __cplusplus
}
#endif


#endif // __IOSDriver_h__
