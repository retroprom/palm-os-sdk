/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialMgrLib.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef _SERIALMGRLIB_H_
#define _SERIALMGRLIB_H_

#include <BtLibTypes.h>

// New Serial manager feature numbers

#define sysFtrNewSerialPresent     1
#define sysFtrNewSerialVersion     2

#define serMgrVersion              3

/********************************************************************
 * Serial Manager Errors
 * the constant serErrorClass is defined in SystemMgr.h
 ********************************************************************/

#define	serErrBadParam				(serErrorClass | 1)
#define	serErrBadPort				(serErrorClass | 2)
#define	serErrNoMem					(serErrorClass | 3)
#define	serErrBadConnID				(serErrorClass | 4)
#define	serErrTimeOut				(serErrorClass | 5)
#define	serErrLineErr				(serErrorClass | 6)
#define	serErrAlreadyOpen			(serErrorClass | 7)
#define serErrStillOpen				(serErrorClass | 8)
#define	serErrNotOpen				(serErrorClass | 9)
#define	serErrNotSupported			(serErrorClass | 10)	// functionality not supported
#define	serErrNoDevicesAvail		(serErrorClass | 11)	// No serial devices were loaded or are available.
#define	serErrConfigurationFailed	(serErrorClass | 12)

// mask values for the lineErrors  from SerGetStatus

#define	serLineErrorParity		0x0001			// parity error
#define	serLineErrorHWOverrun	0x0002			// HW overrun
#define	serLineErrorFraming		0x0004			// framing error
#define	serLineErrorBreak		0x0008			// break signal asserted
#define	serLineErrorHShake		0x0010			// line hand-shake error
#define	serLineErrorSWOverrun	0x0020			// HW overrun
#define	serLineErrorCarrierLost	0x0040			// CD dropped

/********************************************************************
 * Serial Port Definitions
 ********************************************************************/

#define serPortLogicalPortMask	0x8000		// portId is logical if these bits are set
#define serPortLocalHotSync		0x8000		// Use physical HotSync port
#define serPortCradlePort		0x8000		// Cradle Port (Auto detect cradle type)
#define serPortIrPort			0x8001		// Use available IR port.
#define serPortConsolePort		0x8002		// Console port
#define serPortCradleRS232Port	0x8003		// Cradle RS232 Port
#define serPortCradleUSBPort	0x8004		// Cradle USB Port
#define serPortConsoleRS232Port	0x8005		// Console RS232 Port
#define serPortConsoleUSBPort	0x8006		// Console USB Port

// This constant is used by the Serial Link Mgr only
#define serPortIDMask			0xC000

/********************************************************************
 * Serial Settings Descriptor
 ********************************************************************/
	
#define	srmSettingsFlagStopBitsM			0x00000001		// mask for stop bits field
#define	srmSettingsFlagStopBits1			0x00000000		//  1 stop bits	
#define	srmSettingsFlagStopBits2			0x00000001		//  2 stop bits	
#define	srmSettingsFlagParityOnM			0x00000002		// mask for parity on
#define	srmSettingsFlagParityEvenM			0x00000004		// mask for parity even
#define	srmSettingsFlagXonXoffM				0x00000008		// (NOT IMPLEMENTED) mask for Xon/Xoff flow control
#define	srmSettingsFlagRTSAutoM				0x00000010		// mask to prevent UART input overflow using RTS (NOTE: this flag 
															// alone does not prevent software overruns from the serial input buffer)
#define	srmSettingsFlagCTSAutoM				0x00000020		// mask for CTS xmit flow control (see srmSettingsFlagFlowControlIn below)
#define	srmSettingsFlagBitsPerCharM			0x000000C0		// mask for bits/char
#define	srmSettingsFlagBitsPerChar5			0x00000000		//  5 bits/char	
#define	srmSettingsFlagBitsPerChar6			0x00000040		//  6 bits/char	
#define	srmSettingsFlagBitsPerChar7			0x00000080		//  7 bits/char	
#define	srmSettingsFlagBitsPerChar8			0x000000C0		//  8 bits/char
#define	srmSettingsFlagFlowControlIn		0x00000100		// mask to prevent the serial input buffer overflow, using RTS. Use in
															// conjunction with srmSettingsFlagRTSAutoM for a fully flow controlled input.
//#define	srmSettingsFlagRTSInactive			0x00000200		// if set and srmSettingsFlagRTSAutoM==0, RTS is held in the inactive (flow off) state forever.

// Default settings

#define		srmDefaultSettings			(srmSettingsFlagBitsPerChar8 | srmSettingsFlagStopBits1 | \
										srmSettingsFlagRTSAutoM | srmSettingsFlagFlowControlIn | srmSettingsFlagCTSAutoM)
#define		srmDefaultCTSTimeoutV4		500	// ticks
#define		srmDefaultCTSTimeout		(srmDefaultCTSTimeoutV4*10)		// milliseconds

// Status bitfield constants

#define srmStatusCtsOn				0x00000001
#define srmStatusRtsOn				0x00000002
#define srmStatusDsrOn				0x00000004
#define srmStatusBreakSigOn			0x00000008
#define srmStatusDtrOn				0x00000010
#define srmStatusDcdOn				0x00000020
#define srmStatusRingOn				0x00000040

// Info fields describing serial HW capabilities.

#define serDevCradlePort			0x00000001		// Serial HW controls RS-232 serial from cradle connector of handheld.
#define serDevRS232Serial			0x00000002		// Serial HW has RS-232 line drivers
#define serDevIRDACapable			0x00000004		// Serial Device has IR line drivers and generates IRDA mode serial.	
//#define serDevModemPort				0x00000008		// Serial deivce drives modem connection.
//#define serDevCncMgrVisible			0x00000010		// Serial device port name string to be displayed in Connection Mgr panel.
#define serDevConsolePort			0x00000020		// Serial device is the default console port.
#define serDevUSBCapable   			0x00000040		// USB driver for USB hardware connected to the cradle connector of the handheld.
#define serDevHotsyncCapable        0x00000080      // this port can be used for Hotsync


typedef struct DeviceInfoType {
	uint32_t serDevCreator;								// Four Character creator type for serial driver ('sdrv')
	uint32_t serDevFtrInfo;								// Flags defining features of this serial hardware.
	uint32_t serDevMaxBaudRate;							// Maximum baud rate for this device.
	uint32_t serDevHandshakeBaud;						// HW Handshaking is reccomended for baud rates over this
	char *serDevPortInfoStr;							// Description of serial HW device or virtual device.			
	uint8_t reserved[8];									// Reserved.
} DeviceInfoType;

typedef DeviceInfoType *DeviceInfoPtr;

// Function IDs
//
// Standard set of function ids for the SrmOpen.  Out of convenience, function ids
// use the same namespace as creator ids.  Custom functions can be defined by
// using your app's creator id.  The driver must have knowledge of that creator
// id for it to be of any use.  A driver should handle an unknown function id
// gracefully, either use default functionality or return a serErrBadParam error.
// 
#define serFncUndefined 	0L						// Undefined function
#define serFncPPPSession	netIFCreatorPPP			// NetLib PPP Interface
#define serFncSLIPSession  	netIFCreatorSLIP		// NetLib SLIP Interface
#define serFncDebugger		sysFileCSystem			// PalmOS Debugger
#define serFncHotSync		sysFileCSync			// HotSync function
#define serFncConsole		sysFileCSystem			// PalmOS Console
#define serFncTelephony   	sysFileCTelMgrLib		// Telephony Library

// Open Configuration Structure
//
typedef struct SrmOpenConfigType {
	uint32_t baud;				// Baud rate that the connection is to be opened at.
								// Applications that use drivers that do not require
								// baud rates can set this to zero or any other value.
								// Drivers that do not require a baud rate should 
								// ignore this field
	uint32_t function;			// Designates the function of the connection. A value
								// of zero indictates default behavior for the protocol.
								// Drivers that do not support multiple functions should
								// ignore this field.	
	MemPtr drvrDataP;			// Pointer to driver specific data.
	uint16_t drvrDataSize;		// Size of the driver specific data block.	
	uint16_t sysReserved0;		// Alignment padding
	uint32_t sysReserved1;   		// System Reserved
	uint32_t sysReserved2;    	// System Reserved 
} SrmOpenConfigType,*SrmOpenConfigPtr;

// drvrData structures for well known ports (a pointer to this structure must be passed in
// ConfigType.drvrDataP.

typedef struct SrmRfcommOpenParamsType {		// Rfcomm port (creator = 'rfcm')
	BtLibDeviceAddressType btAddr;
	uint16_t sysReserved0;		// Alignment padding
	char *serviceClassIDName;	// this is the string to put in CncMgr BT profile parameter BLUETOOTH_CNCMGR_PARAMNAME_ServiceClassId ('sci ')
} SrmRfcommOpenParamsType;

/********************************************************************
 * Type of a wakeup handler procedure which can be installed through the
 *   SerSetWakeupHandler() call.
 ********************************************************************/
typedef void (*WakeupHandlerProcPtr)(uint32_t refCon);

/********************************************************************
 * Serial Library Control Enumerations
 ********************************************************************/

#define srmCtlSystemStart		0x7000	// Start point for system op codes.
#define srmCtlCustomStart		0x8000	// Start point for custom op codes.

typedef enum SrmCtlEnum {
	srmCtlFirstReserved = 0,		// RESERVE 0
	
	srmCtlSetBaudRate,				// Sets the current baud rate for the HW.
									// valueP = pointer to int32_t, valueLenP = pointer to sizeof(int32_t)
											
	srmCtlGetBaudRate,				// Gets the current baud rate for the HW.
											
	srmCtlSetFlags,					// Sets the current flag settings for the serial HW.
	
	srmCtlGetFlags,					// Gets the current flag settings the serial HW.
	
	srmCtlSetCtsTimeout,			// Sets the current Cts timeout value.
	
	srmCtlGetCtsTimeout,			// Gets the current Cts timeout value.
	
	srmCtlStartBreak,				// turn RS232 break signal on:
									// users are responsible for ensuring that the break is set
									// long enough to genearate a valid BREAK!
									// valueP = 0, valueLenP = 0
											
	srmCtlStopBreak,				// turn RS232 break signal off:
									// valueP = 0, valueLenP = 0

	srmCtlStartLocalLoopback,		// Start local loopback test
									// valueP = 0, valueLenP = 0
											
	srmCtlStopLocalLoopback,		// Stop local loopback test
									// valueP = 0, valueLenP = 0


	srmCtlIrDAEnable,				// Enable IrDA connection on this serial port
									// valueP = 0, valueLenP = 0

	srmCtlIrDADisable,				// Disable IrDA connection on this serial port
									// valueP = 0, valueLenP = 0

	srmCtlRxEnable,					// enable receiver  ( for IrDA )
	
	srmCtlRxDisable,				// disable receiver ( for IrDA )

	obsolete1,						// was srmCtlEmuSetBlockingHook									

	srmCtlUserDef,					// Specifying this opCode passes through a user-defined
									//  function to the DrvControl function. This is for use
									//  specifically by serial driver developers who need info
									//  from the serial driver that may not be available through the
									//  standard SrmMgr interface.
											
	srmCtlGetOptimalTransmitSize,	// This function will ask the port for the most efficient buffer size
									// for transmitting data packets.  This opCode returns serErrNotSupported
									// if the physical or virtual device does not support this feature.
									// The device can return a transmit size of 0, if send buffering is
									// requested, but the actual size is up to the caller to choose.
									// valueP = pointer to uint32_t --> return optimal buf size
									// ValueLenP = sizeof(uint32_t)
	
	srmCtlSetDTRAsserted,			// Enable or disable DTR.
	
	srmCtlGetDTRAsserted,			// Determine if DTR is enabled or disabled.
	
	srmCtlSetYieldPortCallback,  	// Set the yield port callback
	
	srmCtlSetYieldPortRefCon,     	// Set the yield port refNum
	
						// ***** ADD NEW ENTRIES BEFORE THIS ONE
	
	srmCtlSystemReserved = srmCtlSystemStart, 	// Reserve control op code space for system use.
	
	srmCtlCustom = srmCtlCustomStart,     		// Reserve control op code space for licensee use.
	
	srmCtlLAST						
	
} SrmCtlEnum;

/* Connection manager SerialMgr profile parameter tags */

#define serMgrCncParamCreator 'crea'
#define serMgrCncParamCreatorStr "crea"
#define serMgrCncParamFeatures 'feat'
#define serMgrCncParamFeaturesStr "feat"
#define serMgrCncParamMaxBaudRate 'mbrt'
#define serMgrCncParamMaxBaudRateStr "mbrt"
#define serMgrCncParamHShakeBaudRate 'hsbr'
#define serMgrCncParamHShakeBaudRateStr "hsbr"
#define serMgrCncParamHumanName 'hnam'
#define serMgrCncParamHumanNameStr "hnam"
#define serMgrCncParamLogicalPortId 'lpid'
#define serMgrCncParamLogicalPortIdStr "lpid"
#define serMgrCncParamFuntionId 'fnid'
#define serMgrCncParamFunctionIdStr "fnid"


#ifdef __cplusplus
extern "C" {
#endif

status_t SrmOpen(uint32_t port, uint32_t baud, uint16_t *newPortIdP);
	
status_t SrmExtOpen(uint32_t port, SrmOpenConfigType* configP, uint16_t configSize, uint16_t *newPortIdP);

status_t SrmExtOpenBackground(uint32_t port, SrmOpenConfigType* configP, uint16_t configSize, uint16_t *newPortIdP);
	
status_t SrmOpenBackground(uint32_t port, uint32_t baud, uint16_t *newPortIdP);

status_t SrmClose(uint16_t portId);
	
status_t SrmSleep(void);
	
status_t SrmWake(void);

status_t SrmGetDeviceCount(uint16_t *numOfDevicesP);

status_t SrmGetDeviceInfo(uint32_t deviceID, DeviceInfoType *deviceInfoP);

status_t SrmGetStatus(uint16_t portId, uint32_t *statusFieldP, uint16_t *lineErrsP);

status_t SrmClearErr (uint16_t portId);

status_t SrmControl(uint16_t portId, uint16_t op, void *valueP, uint16_t *valueLenP);
	
status_t SrmCustomControl(uint16_t portId, uint16_t opCode, uint32_t creator, 
							void* valueP, uint16_t* valueLenP);
	
uint32_t SrmSend (uint16_t portId, const void *bufP, uint32_t count, status_t *errP);

status_t SrmSendWait(uint16_t portId);

status_t SrmSendCheck(uint16_t portId, uint32_t *numBytesP);

status_t SrmSendFlush(uint16_t portId);

uint32_t SrmReceive(uint16_t portId, void *rcvBufP, uint32_t count, int32_t msTimeout, status_t *errP);

status_t SrmReceiveWait(uint16_t portId, uint32_t bytes, int32_t msTimeout);

status_t SrmReceiveCheck(uint16_t portId,  uint32_t *numBytesP);

status_t SrmReceiveFlush(uint16_t portId, int32_t msTimeout);

status_t SrmSetReceiveBuffer(uint16_t portId, void *bufP, uint16_t bufSize);

status_t SrmReceiveWindowOpen(uint16_t portId, uint8_t **bufPP, uint32_t *sizeP);

status_t SrmReceiveWindowClose(uint16_t portId, uint32_t bytesPulled);

status_t SrmSetWakeupHandler(uint16_t portId, WakeupHandlerProcPtr procP, uint32_t refCon);

status_t SrmPrimeWakeupHandler(uint16_t portId, uint16_t minBytes);

#ifdef __cplusplus
}
#endif

#endif
