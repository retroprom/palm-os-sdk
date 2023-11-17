/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: Serial.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *	Serial header for non-posix features of the serial interface.
 *
 *****************************************************************************/

#ifndef __SERIAL_H__
#define	__SERIAL_H__

#include <sys/ttycom.h>
#include <PalmTypes.h>
 
// Serial Port Attributes.
// Attributes are used by the SerialMgr to determine the use and 
// capabilities of each port.
#define	SERIAL_PORT_ATTR_DEFAULT_PORT	0x00000001
#define SERIAL_PORT_ATTR_SIR		 	0x00000002
#define SERIAL_PORT_ATTR_RS232			0x00000004
#define SERIAL_PORT_ATTR_USB			0x00000008
#define SERIAL_PORT_ATTR_HOTSYNC		0x00000010
#define SERIAL_PORT_ATTR_SERIALMGR		0x00000020
#define SERIAL_PORT_ATTR_68K_CONSOLE	0x00000040
#define SERIAL_PORT_ATTR_BLUETOOTH		0x00000080
#define SERIAL_PORT_ATTR_BLUETOOTH_DEFAULT	0x00000100


typedef struct SerialAttributesTag {
	uint32_t portFlags;
} SerialAttributesType;

// Non standard ioctls for serial devices
#define SYSTEM_RANGE_START	0x0000
#define THIRD_PARTY_RANGE_START 0x8000

// IOCTL command used to configure USB connections
#define	SERIAL_SET_USB_FUNCTION_ID 	_IOW('u', SYSTEM_RANGE_START + 1, uint32_t)
#define	SERIAL_START_USB_ENUMERATION _IO('u', SYSTEM_RANGE_START + 2)

// IOCTL commands used to control how long the serial driver should wait 
// before issuing an M_HANGUP when the DCD or DSR line drops.
// Timeouts are in milliseconds.
#define SERIAL_TIMEOUT_NEVER		0xffffffff

#define SERIAL_GET_DSR_TIMEOUT 		_IOR('u', SYSTEM_RANGE_START + 3, uint32_t)
#define SERIAL_SET_DSR_TIMEOUT 		_IOW('u', SYSTEM_RANGE_START + 4, uint32_t)
#define SERIAL_GET_DCD_TIMEOUT 		_IOR('u', SYSTEM_RANGE_START + 5, uint32_t)
#define SERIAL_SET_DCD_TIMEOUT 		_IOW('u', SYSTEM_RANGE_START + 6, uint32_t)

#endif // __SERIAL_H__
