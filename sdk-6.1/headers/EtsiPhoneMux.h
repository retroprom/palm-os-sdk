/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: EtsiPhoneMux.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

#ifndef EtsiPhoneMux_h
#define	EtsiPhoneMux_h


/******************************************************************************
 *	Includes
 ******************************************************************************/
#include <sys/ioccom.h>

/******************************************************************************
 *	Defines
 ******************************************************************************/

// Etsi Phone Multiplexer driver name
#define kEtsiPMuxDriverName				"EtsiPMux"

// IOCTL type
#define IOC_ETSI_PHONE_MUX				'2'
#define	EtsiPMuxIOCChannel				_IO(IOC_ETSI_PHONE_MUX, 0)
#define	EtsiPMuxIOCMuxMode				_IO(IOC_ETSI_PHONE_MUX, 1)


#endif // EtsiPhoneMux_h
