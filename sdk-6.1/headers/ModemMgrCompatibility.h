/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ModemMgrCompatibility.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Include file for Modem Manager
 *
 *****************************************************************************/

#ifndef __MODEM_MGR_H__
#define __MODEM_MGR_H__


// Include elementary types
#include <PalmTypes.h>
#include <CmnErrors.h>

/************************************************************
 * Modem Manager constants
 *************************************************************/

#define mdmMaxStringSize	40

#define mdmCmdBufSize		81		// command buffer capacity (including null)
#define mdmRespBufSize		81		// reply buffer capacity (including null)
#define mdmCmdSize			8		// max storage needed for smartmodem command

#define mdmDefCmdTimeOut	500000L		// in micro-seconds

#define mdmDefDTWaitSec		4
#define mdmDefDCDWaitSec	70
#define mdmDefSpeakerVolume	1

#define mdmResetStrInCmdBuf	0x01

// Speaker volume settings
enum {
	mdmVolumeOff = 0,
	mdmVolumeLow = 1,
	mdmVolumeMed = 2,
	mdmVolumeHigh = 3
	};


// Modem connection stages (NEW for Pilot 2.0)
typedef enum {
	mdmStageInvalid = 0,					// invalid state
	mdmStageReserved = 1,				// reserved for 1.0 compatibility
	mdmStageFindingModem,				// checking if modem is present
	mdmStageInitializing,				// initializing the modem
	mdmStageDialing,						// dialing the modem
	mdmStageWaitingForCarrier,			// waiting for carrier detect
	mdmStageHangingUp						// hanging up the modem
	} MdmStageEnum;

/************************************************************
 * Modem Manager data structures
 *************************************************************/
// Prototype for the "user cancel" check callback function
typedef int16_t (*MdmUserCanProcPtr)(uint32_t userRef);

typedef struct MdmInfoType {
	uint32_t	initialBaud;				// initial baud rate to use
	uint32_t	cmdTimeOut;					// number of micro-sec to wait after a cmd
	uint16_t	portID;						// serial port ID number.	[NewSerialMgr; replaces serRefNum]
	int16_t	dtWaitSec;					// dialtone wait (sec) (-1 for modem's default)
	int16_t	dcdWaitSec;					// dcd timeout wait (sec) (-1 for modem's default)
	int16_t	volume;						// speaker volume(see mdmVolume... constants)
	Boolean	pulse;						// pulse or tone dialing
	Boolean	hwHShake;					// enable cts/rts handshaking
	Boolean	autoBaud;					// enable/disable auto-baud to connected baud rate
	uint8_t	telConnection;				// Boolean true if connecting to a mobile phone
										// false otherwise.
	MdmUserCanProcPtr	canProcP;		// ptr to user-cancel function
	uint32_t	userRef;					// parameter for canProcP()
	char	cmdBuf[mdmCmdBufSize];		// build all commands here
	char	respBuf[mdmRespBufSize];	// response buffer
	uint8_t	curStage;					// set by ModemMgr to report current MdmStageEnum
	uint8_t	strInCmdBuf;				// Set to mdmResetStrInCmdBuf if the reset string is 
										// stored in the command buffer cmdBuf.  This is to 
										// get around a compatibility problem with not being 
										// able pass in a reset string.  The reset string 
										// must be prefixed with AT.  Set to zero otherwise
	uint32_t	connectBaud;				// baud at which connection was established
										// (0 = unknown)
	} MdmInfoType;

typedef MdmInfoType*		MdmInfoPtr;

/************************************************************
 * Modem Manager result codes
 * (mdmErrorClass is defined in ErrorBase.h)
 *************************************************************/
#if 0
#pragma mark -Error Codes-
#endif

#define	mdmErrNoTone			(mdmErrorClass | 1)		// no dial tone
#define	mdmErrNoDCD				(mdmErrorClass | 2)		// no carrier / timeout
#define	mdmErrBusy				(mdmErrorClass | 3)		// busy signal heard
#define	mdmErrUserCan			(mdmErrorClass | 4)		// cancelled by user
#define	mdmErrCmdError			(mdmErrorClass | 5)		// command error
#define	mdmErrNoModem			(mdmErrorClass | 6)		// no modem detected
#define	mdmErrMemory			(mdmErrorClass | 7)		// not enough memory
#define	mdmErrPrefs				(mdmErrorClass | 8)		// modem preferences have not been
																		// setup - (app should take user to modem prefs panel)
#define	mdmErrDial				(mdmErrorClass | 9)		// dial command error - most likely the dial
																		// string is too long for the modem's buffer or
																		// contains invalid characters
// <chg 3-7-98 RM> New error code for empty phone number which is only invalid if
//  the modem type is not a "Direct Connect" modem
#define	mdmErrNoPhoneNum		(mdmErrorClass | 10)		// No phone number and not "Direct Connect"


/********************************************************************
 * Modem Manager Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/
#if 0
#pragma mark -API Routines-
#endif

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

status_t MdmDialV50(MdmInfoPtr modemP, char *okDialP, char *userInitP, char *phoneNumP);

status_t MdmHangUpV50(MdmInfoPtr modemP);


#ifdef __cplusplus 
}
#endif



/************************************************************
 * Modem Manager Macros
 *************************************************************/

#endif  // __MODEM_MGR_H__
