/******************************************************************************
 *
 * Copyright (c) 2001-2004 PalmSource, Inc. All rights reserved.
 *
 * File: TelephonyServer.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef TelephonyServer_h
#define	TelephonyServer_h


/******************************************************************************
 *	Includes
 ******************************************************************************/

#include <TelephonyLib.h>
#include <sys/ioccom.h>


/******************************************************************************
 *	Defines
 ******************************************************************************/

// Telephony server driver name
#define kTelSrvDriverName				"TelSrv"

#define kCncPhonePluginName				"Phone"
#define kCncDataCallPluginName			"DataCall"
#define kCncDataCallChooser				"{DataCallChooser}"

// Parameter for Phone profile
#define kTelSrvParamDriver				'Drvr'
// - Internal parameters
#define kTelSrvParamConnection			'Cnc_'		
#define kTelSrvParamTelType				'TTyp'
#define kTelSrvParamDriverParams		'Drvp'

// Parameter for Data call profile
#define	kTelSrvParamDataServiceType		'Type'

#define	kTelSrvParamDtcDialNumber		'Dial'

#define	kTelSrvParamCsdDialNumber		kTelSrvParamDtcDialNumber
#define	kTelSrvParamCsdBearerSpeed		'Bspd'
#define	kTelSrvParamCsdBearerCncElement	'Bcel'
#define	kTelSrvParamCsdBearerService	'Bsrv'

#define	kTelSrvParamGprsApn				'Apna'
#define	kTelSrvParamGprsDataComp		'Datc'
#define	kTelSrvParamGprsHeaderComp		'Hdrc'
#define	kTelSrvParamGprsQosPrecedence	'Prec'
#define	kTelSrvParamGprsQosDelay		'Dela'
#define	kTelSrvParamGprsQosReliability	'Reli'
#define	kTelSrvParamGprsQosPeak			'Peak'
#define	kTelSrvParamGprsQosMean			'Mean'

//#define	kTelSrvParamContext				'Ctxt'

// IOCTL type
#define IOC_TELEPHONY					'T'
#define	telSrvIoctlTelephonyMessage		_IO(IOC_TELEPHONY, 0)
#define	telSrvIoctlTelephonyMessage68k	_IO(IOC_TELEPHONY, 1)
#define	telSrvIoctlCancelMessage		_IO(IOC_TELEPHONY, 2)
#define	telSrvIoctlGetDriverCreator		_IO(IOC_TELEPHONY, 3)
#define	telSrvIoctlConnect				_IO(IOC_TELEPHONY, 4)
#define	telSrvIoctlDisconnect			_IO(IOC_TELEPHONY, 5)
#define	telSrvIoctlGetConnectionStatus	_IO(IOC_TELEPHONY, 6)
#define	telSrvIoctlRegisterProfile		_IO(IOC_TELEPHONY, 7)
#define	telSrvIoctlUnregisterProfile	_IO(IOC_TELEPHONY, 8)
#define	telSrvIoctlGetAuthentication	_IO(IOC_TELEPHONY, 9)
#define	telSrvIoctlAuthenticationDone	_IO(IOC_TELEPHONY, 10)
#define	telSrvIoctlSleep				_IO(IOC_TELEPHONY, 11)
#define	telSrvIoctlWake					_IO(IOC_TELEPHONY, 12)
#define	telSrvIoctlBatteryChange		_IO(IOC_TELEPHONY, 13)

// Connection type: to indicate to the Telephony plugin what it had to do.
#define	kTelSrvConnectionTypeTransport	0
#define	kTelSrvConnectionTypeTelephony	1

/******************************************************************************
 *	Structures
 ******************************************************************************/

typedef struct _TelMessageType
{
	status_t	returnCode;			// function return code, errNone if ok, else an error
	uint16_t	transId;			// transId returned on asynchronous function call return
	uint16_t	functionId;			// ID of the message associated to the asynchronous function call
	MemPtr		paramP;
	size_t		paramSize;
} TelMessageType, *TelMessagePtr;

typedef struct _TelMessage68kType
{
	MemPtr		paramP;
	uint16_t	functionId;
	uint16_t	padding;
} TelMessage68kType, *TelMessage68kPtr;

typedef struct _TelConnectionInfo
{
	char*		profileStrP;
	size_t		profileStrSize;
	uint32_t	profileId;			// Profile id of the transport driver
	uint32_t	driverCreator;		// Creator of the driver
	uint8_t*	driverParamsP;		// Driver specific parameters
	size_t		driverParamsSize;	// Driver specific parameters size
	uint8_t		cncPurpose;			// Purpose of connection (transport/telephony)
	uint8_t		cncType;			// Type of connection (command, modem, ...)
	uint8_t		padding[2];
} TelConnectionInfoType, *TelConnectionInfoPtr;


#endif // TelephonyServer_h
