/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IrDA.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef IRDA_H
#define IRDA_H

#include <PalmTypes.h>
#include <sys/socket.h>


/* IrDA protocols */

#define IRPROTO_LAP		1	/* IrLAP */
#define IRPROTO_LMP		2	/* IrLMP */
#define IRPROTO_TTP		3	/* TinyTP */


/* IrDA protocol address types and constants */

// IrDA device address
typedef uint32_t IrLapDeviceAddrType;

// IrLMP/TinyTP SAP selector
typedef uint8_t IrLmpSAPType;

// format of IrLMP address, as used with sockets API
struct sockaddr_irda {
	sa_family_t			sir_family;
	char				sir_name[25];
	IrLmpSAPType		sir_lsap;
	IrLapDeviceAddrType	sir_addr;
};

// special "anybody" device address
#define IRADDR_ANY			((IrLapDeviceAddrType)0x00000000)

// IrDA device broadcast address
#define IRADDR_BROADCAST	((IrLapDeviceAddrType)0xffffffff)


/* IrLMP-specific constants */

// LM_Status.indication link states
typedef enum {
	irLapLinkDown,
	irLapSearching,
	irLapCalling,
	irLapLinkUp,
	irLapLinkFailing,
} IrLmpLinkStatusType;

// values for 'IrLmpDeviceInfoType::method'
typedef enum {
	irLmpSniffing,
	irLmpActiveDiscovery,
	irLmpPassiveDiscovery,
} IrLmpDiscoveryMethodType;

// special IrLMP LSAP values
#define irLsapIAS		((IrLmpSAPType)0x00)
#define irLsapUnitdata	((IrLmpSAPType)0x70)
#define irLsapAny		((IrLmpSAPType)0xff)


/* structures returned by IrLMP discovery */

// maximum length of discovery device info
#define kMaxDeviceInfoBytes		32

typedef struct IrLmpDeviceInfoTag {
	IrLapDeviceAddrType	deviceAddr;
	uint8_t				method;			// see IrLmpDiscoveryMethodType
	uint8_t				__pad0[2];
	uint8_t				infoLen;
	uint8_t				deviceInfo[kMaxDeviceInfoBytes];	// only first 'infoLen' bytes valid
} IrLmpDeviceInfoType;


/* IrDA-specific setsockopt() commands */

// invoke LM_Idle.Request
#define SO_IRIDLE			0x1101

// invoke LM_AccessMode.Request
#define SO_IREXCLUSIVE		0x1102

// retrieve connection MTU
#define SO_IRMTU			0x1103

// retrieve connection MRU
#define SO_IRMRU			0x1104


#endif	// IRDA_H
