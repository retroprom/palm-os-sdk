/***************************************************************
 *
 * Project:
 *    Ken Expansion Card
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 1999.
 *
 * FileName:
 *    FlashDriverCommon.h
 *
 * Description:
 *    This file contains common private definitaions shared by Flash
 *    Drivers.
 *
 * ToDo:
 *
 * History:
 *    08-Apr-1999 - Created by Vitaly Kruglikov (vkruglikov@handspring.com)
 *	  25-Aug-1999	vmk	  Rewrote
 *	  18-Apr-2000	vmk	  Added support for AMD Am29LV160BT and
 *						   Fujitsu MBM29LV160TE
 *	  03-Oct-2000	jmp	  Added support for Intel E28F640J3
 *	  25-Oct-2000	jmp	  Added support for Hyundai HY29LV160BT
 *	  17-Apr-2001	vmk	  Added support for SST32HF162/164 and SST32HF802
 *
 ****************************************************************/

// Include only once
#ifndef _FLASH_DRIVER_COMMON_H_
#define _FLASH_DRIVER_COMMON_H_



// ============================================================================
// Flash manufacturer IDs and device IDs
// ============================================================================


#define flashManuAMD		0x01
  #define flashDevAMD8MbBB		  0x5B	// AMD Am29LV800B
  #define flashDevAMD16MbBB		  0x49	// AMD Am29LV160BB
  #define flashDevAMD16MbBT		  0xC4	// AMD Am29LV160BT (top boot block 32-8-8-16)

#define flashManuFujitsu	0x04
  #define flashDevFuj16MbBB		  0x49	// MBM29LV160B (Am29LV160BB-compatible)
  #define flashDevFuj16MbBT		  0xC4	// MBM29LV160TE (Am29LV160BT-compatible)*

#define flashManuToshiba	0x98
  #define flashDevTosh16MbBB	  0x43	// TC58FYB160 (Am29LV160BB-compatible)

#define flashManuIntel		0x89		// Intel
  #define flashDevIntel32Mb		  0x16	// INTEL E28F320J3
  #define flashDevIntel64Mb		  0x17	// INTEL E28F640J3
  #define flashDevIntel128Mb	  0x18	// INTEL E28F128J3

#define flashManuHyundai	0xAD		// Hyundai
  #define flashDevHY16MbBB		  0x49	// Hyundai HY29LV160BT (the T in BT is meant for the packaging,
						// this is a Bottom Boot device Am29LV160BB-compatible)


#define flashManuSST		0xBF
  #define flashDevSST32HF802	  0x81	// SST32HF802 1MB, 4KB sectors (Device id : 2781 hex)
  #define flashDevSST32HF162_164  0x82	// SST32HF162/164 2MB, 4KB sectors (Device id : 2782 hex)


#endif	// _FLASH_DRIVER_COMMON_H_ -- include only once
