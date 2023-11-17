/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Crc.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		This is the header file for the CRC calculation routines for Pilot.
 *
 *****************************************************************************/

#ifndef _CRC_H_
#define _CRC_H_


// Include elementary types
#include <PalmTypes.h>


/********************************************************************
 * CRC Calculation Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


	

//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------


// Crc16CalcBlock()
//
// Calculate the 16-bit CRC of a data block using the table lookup method.
//
uint16_t		Crc16CalcBlock(const void *bufP, uint16_t count, uint16_t crc);

uint16_t		Crc16CalcBigBlock(void *bufP, uint32_t count, uint16_t crc);

uint32_t		Crc32CalcBlock(const void * bufP, uint16_t count, uint32_t crc);


#ifdef __cplusplus
}
#endif



#endif  // _CRC_H_
