/******************************************************************************
 *                                                                            *
 *                 (C) Copyright 2000, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyHwrOEMIDs.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       GHwrOEMIDs (CompanyID, HALID, DeviceID) definitions                  *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/12/02                                              *
 *       Last Modified: $Date: 01/07/17 19:01 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYHWROEMIDS_H__
#define __SONYHWROEMIDS_H__

/******************************************************************************
 *    Include                                                                 *
 ******************************************************************************/
#include <HwrMiscFlags.h>


/******************************************************************************
 *    Definitions                                                             *
 ******************************************************************************/

/*** GHwrOEMCompanyID ***/
	/* All sony-based model has this ID. other one is not permitted */
	/* can be obtained by FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &value) */
	/* hwrOEMCompanyIDSony may be defined in HwrMiscFlags.h someday :-) */
#define sonyHwrOEMCompanyID_Sony		'sony'	/* CAN'T be changed!! */

/*** GHwrOEMHALID ***/
	/* defined for each HAL source code, same codes have the same ID */
	/* can be obtained by FtrGet(sysFtrCreator, sysFtrNumOEMHALID, &value) */
#define sonyHwrOEMHALIDPda1Mono		hwrOEMHALIDEZRef			/* 'eref' */
#define sonyHwrOEMHALIDPda1Color		hwrOEMHALIDEZRefColor	/* 'cref' */
#define sonyHwrOEMHALIDYosemite		'ysmt'	/* 3.5 */
#define sonyHwrOEMHALIDNasca			'nsca'	/* 4.0 */
#define sonyHwrOEMHALIDYellowstone	'ystn'	/* 4.0 */
#define sonyHwrOEMHALIDYosemite2		sonyHwrOEMHALIDYosemite	/* 4.0 */

/*** GHwrOEMDeviceID ***/
	/* defined for each model. more than one models have the same HALID, but
	     not DeviceID */
	/* can be obtained by FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &value) */
#define sonyHwrOEMDeviceIDPda1Mono			(0x00010001)
#define sonyHwrOEMDeviceIDPda1Color			(0x00010002)
#define sonyHwrOEMDeviceIDYosemite			'ysmt'
#define sonyHwrOEMDeviceIDNasca				'nsca'
#define sonyHwrOEMDeviceIDYellowstone		'ystn'
#define sonyHwrOEMDeviceIDYosemite2			'ysm2'

/******************************************************************************
 *    References                                                              *
 ******************************************************************************/
/* CAUTIONS: This information is provided just for your information, and not
     guaranteed to be correct all the time */

/* PEG-S300 */
#define sonyHwrOEMHALID_S300			sonyHwrOEMHALIDPda1Mono
#define sonyHwrOEMDeviceID_S300		sonyHwrOEMDeviceIDPda1Mono

/* PEG-S500C(J) */
#define sonyHwrOEMHALID_S500C			sonyHwrOEMHALIDPda1Color
#define sonyHwrOEMDeviceID_S500C		sonyHwrOEMDeviceIDPda1Color

/* PEG-N700C(J),N710C(US) */
#define sonyHwrOEMHALID_N700C			sonyHwrOEMHALIDYosemite
#define sonyHwrOEMHALID_N710C			sonyHwrOEMHALIDYosemite
#define sonyHwrOEMDeviceID_N700C		sonyHwrOEMDeviceIDYosemite
#define sonyHwrOEMDeviceID_N710C		sonyHwrOEMDeviceIDYosemite

/* PEG-S320(US) */
#define sonyHwrOEMHALID_S320			sonyHwrOEMHALIDNasca
#define sonyHwrOEMDeviceID_S320		sonyHwrOEMDeviceIDNasca

/* PEG-N600C(J),N610C(US) */
#define sonyHwrOEMHALID_N600C			sonyHwrOEMHALIDYellowstone
#define sonyHwrOEMHALID_N610C			sonyHwrOEMHALIDYellowstone
#define sonyHwrOEMDeviceID_N600C		sonyHwrOEMDeviceIDYellowstone
#define sonyHwrOEMDeviceID_N610C		sonyHwrOEMDeviceIDYellowstone

/* PEG-N750C(J),N760C(US),N760C/G(GVD),N770C/U(UK),N770C/E(EFG) */
#define sonyHwrOEMHALID_N750C			sonyHwrOEMHALIDYosemite2
#define sonyHwrOEMHALID_N760C			sonyHwrOEMHALIDYosemite2
#define sonyHwrOEMHALID_N770C			sonyHwrOEMHALIDYosemite2
#define sonyHwrOEMDeviceID_N750C		sonyHwrOEMDeviceIDYosemite2
#define sonyHwrOEMDeviceID_N760C		sonyHwrOEMDeviceIDYosemite2
#define sonyHwrOEMDeviceID_N770C		sonyHwrOEMDeviceIDYosemite2


#endif	// __SONYHWROEMIDS_H__

