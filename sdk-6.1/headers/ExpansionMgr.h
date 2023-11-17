/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: ExpansionMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header file for Expansion Manager.
 *
 *****************************************************************************/

#ifndef __EXPANSIONMGR_H__
#define __EXPANSIONMGR_H__

#include <PalmTypes.h>
//#include <SystemMgr.h>
#include <CmnErrors.h>

/************************************************************
 * Constants
 *************************************************************/
#define expFtrIDVersion				(0)	// ID of feature containing version of ExpansionMgr.
										// Check existence of this feature to see if ExpMgr is installed.

#define expMgrVersionNum			((uint16_t)300)	// version of the ExpansionMgr, obtained from the feature

#define expInvalidSlotRefNum		(0)

/************************************************************
 * Capabilities of the hardware device for ExpCardInfoType.capabilityFlags
 *************************************************************/
#define	expCapabilityHasStorage		(0x00000001)	// card supports reading (& maybe writing) sectors
#define	expCapabilityReadOnly		(0x00000002)	// card is read only
#define	expCapabilitySerial			(0x00000004)	// card supports dumb serial interface
//reserved (0x00000008)
#define	expCapabilityHidden			(0x00000010)	// card and volumes on it should be hidden to user
#define	expCapabilityPrivate		(0x00000020)	// card and volumes on it are not accessible by user processes
#define	expCapabilitySystemBackup	(0x00000040)	// card is used for system backup

#define expCardInfoStringMaxLen		(31)

typedef struct ExpCardInfoTag
{
	uint32_t	capabilityFlags;	// bits for different stuff the card supports
	char		manufacturerStr[expCardInfoStringMaxLen+1];	// Manufacturer, e.g., "Palm", "Motorola", etc...
	char		productStr[expCardInfoStringMaxLen+1];			// Name of product, e.g., "SafeBackup 32MB"
	char		deviceClassStr[expCardInfoStringMaxLen+1];	// Type of product, e.g., "Backup", "Ethernet", etc.
	char		deviceUniqueIDStr[expCardInfoStringMaxLen+1];// Unique identifier for product, e.g., a serial number.  Set to "" if no such identifier exists.
}	ExpCardInfoType, *ExpCardInfoPtr;


/********************************************************************
 * Card Metrics
 * These structures contains all of the information about the physical
 * structure of the card that may be needed by a filesystem in order
 * to format volumes on the card.
 ********************************************************************/
#define slotDrvrPartitionTypeFAT12				(0x01)
#define slotDrvrPartitionTypeFAT16Under32MB		(0x04)
#define slotDrvrPartitionTypeFAT16Over32MB		(0x06)
#define slotDrvrPartitionTypeFAT32				(0x0b)
#define slotDrvrBootablePartition				(0x80)
#define slotDrvrNonBootablePartition			(0x00)

typedef struct CardMetricsTag {
	uint32_t	totalSectors;			// The total number of sectors accessable via SlotCardSector[Read/Write]
									//  	(some media may contain extra sectors in case one goes bad,
									//   	 or for storing configuration information, but they are handled
									//  	 internally to the slot driver, and not accessable) 
	uint16_t	bytesPerSector;			// The number of bytes in one sector.
									//  	currently for Palm, this must be the standard 512
	uint16_t	sectorsPerHead;			// The number of Sectors per Head
									//  	as given by guidelines in the specification for this media type
									//  	even though all of our disks accesses are LBA, 
									//  	this is for compatibility when filling out MBRs and PBRs
									// 		if the media guidelines don't care, this value is set to 0
	uint16_t	headsPerCylinder;		// The number of Heads per Cylinder
									//  	as given by guidelines in the specification for this media type
									//  	even though all of our disks accesses are LBA, 
									//  	this is for compatibility when filling out MBRs and PBRs
									// 		if the media guidelines don't care, this value is set to 0
	uint16_t	reserved1;				// Reserved
	uint8_t	sectorsPerBlock;		// A suggested number of Sectors per Block (Cluster)
									//  	as given by guidelines in the specification for this media type
									//		if the media guidelines don't care, this value will be set to 0
	uint8_t	partitionType;			// The suggested partition type (System ID) of the first partition
									//  	as given by guidelines in the specification for this media type
									//		if the media guidelines don't care, this value will be set to 0
	uint8_t	bootIndicator;			// The suggested bootability of the first partition
									//  	as given by guidelines in the specification for this media type
									//  	(generally, 0x80=bootable, default boot partition 0x00=not-bootable)
									//		if the media guidelines don't care, this value will be set to 0xFF
	uint8_t	reserved2;				// Reserved
	uint32_t	partitionStart;			// The suggested starting sector of the first partition
									//  	as given by guidelines in the specification for this media type
									//		if this value is set to zero, and the partitionSize value is non-zero
									//		 the media guidelines suggest to not use an MBR, and only use a PBR at sector 0
									//  	if the media guidelines don't care, the partitionSize value will be set to 0
	uint32_t	partitionSize;			// The suggested size of the first partition
									//  	as given by guidelines in the specification for this media type
									// 		if the media guidelines don't care, this value will be set to 0, and 
									//  	 the partitionStart parameter is also ignored
} CardMetricsType, *CardMetricsPtr;

/************************************************************
 * Iterator start and stop constants.
 * Used by ExpSlotEnumerate
 *************************************************************/
#define expIteratorStart              (0L)
#define expIteratorStop               (0xffffffffL)


/************************************************************
 * Bits in the 'handled' field used in Card Inserted and Removed notifications
 *************************************************************/
#define expHandledVolume		(0x01)	// any volumes associated with the card have been dealt with... the ExpansionMgr will not mount or unmount as appropriate.
#define expHandledSound			(0x02)	// Any pleasing sounds have already been played... the ExpansionMgr will not play a pleasing sound on this insertion/removal.


/************************************************************
 * Error codes
 *************************************************************/
#define expErrUnsupportedOperation			(expErrorClass | 1)		// unsupported or undefined opcode and/or creator
#define expErrNotEnoughPower				(expErrorClass | 2)		// the required power is not available

#define expErrCardNotPresent				(expErrorClass | 3)		// no card is present
#define expErrInvalidSlotRefNum				(expErrorClass | 4)		// slot reference number is bad
#define expErrSlotDeallocated				(expErrorClass | 5)		// slot reference number is within valid range, but has been deallocated.
#define expErrCardNoSectorReadWrite			(expErrorClass | 6)		// the card does not support the 
																						// SlotDriver block read/write API
#define expErrCardReadOnly					(expErrorClass | 7)		// the card does support R/W API
																						// but the card is read only
#define expErrCardBadSector					(expErrorClass | 8)		// the card does support R/W API
																						// but the sector is bad
#define expErrCardProtectedSector			(expErrorClass | 9)		// The card does support R/W API
																						// but the sector is protected
#define expErrNotOpen						(expErrorClass | 10)		// slot driver library has not been opened
#define expErrStillOpen						(expErrorClass | 11)		// slot driver library is still open - maybe it was opened > once
#define expErrUnimplemented					(expErrorClass | 12)		// Call is unimplemented
#define expErrEnumerationEmpty				(expErrorClass | 13)		// No values remaining to enumerate
#define expErrIncompatibleAPIVer			(expErrorClass | 14)		// The API version of this slot driver is not supported by this version of ExpansionMgr.


/************************************************************
 * Common media types.  Used by SlotCardMediaType and SlotMediaType.
 *************************************************************/
#define expMediaType_Any				'wild'	// matches all media types when looking up a default directory
#define expMediaType_MemoryStick		'mstk'
#define expMediaType_CompactFlash		'cfsh'
#define expMediaType_SecureDigital		'sdig'
#define expMediaType_MultiMediaCard		'mmcd'
#define expMediaType_SmartMedia			'smed'
#define expMediaType_RAMDisk			'ramd'	// a RAM disk based media
#define expMediaType_PoserHost			'pose'	// Host filesystem emulated by Poser
#define expMediaType_MacSim				'PSim'	// Host filesystem emulated by Poser
#define expMediaType_SimBlockDevice		'smbd'	// Simulated block device on host file

#ifdef __cplusplus
extern "C" {
#endif

status_t ExpCardPresent(uint16_t slotRefNum);

status_t ExpCardInfo(uint16_t slotRefNum, ExpCardInfoType* infoP);

status_t ExpSlotEnumerate(uint16_t *slotRefNumP, uint32_t *slotIteratorP);
		
// New sector access APIs
status_t ExpCardSectorRead(uint16_t slotRefNum, uint32_t sectorNumber, uint8_t *bufferP, uint32_t *numSectorsP);
status_t ExpCardSectorWrite(uint16_t slotRefNum, uint32_t sectorNumber, uint8_t *bufferP, uint32_t *numSectorsP);

status_t ExpSlotCustomControl(uint16_t slotRefNum, uint32_t apiCreator, uint16_t apiSelector, void *valueP,
		uint16_t *valueLenP);

status_t ExpCardMediaType(uint16_t slotRefNum, uint32_t *mediaTypeP);

status_t ExpSlotMediaType(uint16_t slotRefNum, uint32_t *mediaTypeP);

Boolean ExpCardIsFilesystemSupported(uint16_t slotRefNum, uint32_t filesystemType);

status_t ExpCardMetrics(uint16_t slotRefNum, CardMetricsPtr cardMetricsP);

status_t ExpSlotPowerCheck(uint16_t slotRefNum, uint16_t operationFlags, uint16_t readBlocks, uint16_t writeBlocks);

#ifdef __cplusplus
}
#endif

#endif	// __EXPANSIONMGR_H__
