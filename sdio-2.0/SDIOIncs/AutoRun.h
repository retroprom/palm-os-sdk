/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/

/**
 *
 * @defgroup AutoRun AutoRun Library
 *
 * @brief The AutoRun data structures and constants
 *
 * <hr>
 * @{
 * @}
 */

/**
 *@ingroup AutoRun
 *
 */

/**
 * @file 	AutoRun.h
 * @brief   The AutoRun data structures and constants
 * @version palmOne SDIO SDK 2.0
 * @date 	04/29/2005
 */


#ifndef _AUTORUN_H
#define _AUTORUN_H



#include <NotifyMgr.h>
#include <PalmTypes.h>



/**
 * @name Notification event
 *
 */
/*@{*/
#define sysNotifyDriverSearch	'arun'	/**< Sent after a card has been inserted and the card's information has been identified. */
 								        /**< It allows SDIO drivers already on the handheld to launch themselves. The parameter pointer that accompanies the notification points to an AutoRunInfoType structure. */
/*@}*/


/**
 * AutoRun defines and structs
 */

typedef UInt32 AutoRunMediaType;		/**< This is the SD card type that is requesting to run a program */


/**
 * The next 4 typedefs these depend upon the AutoRunMediaType and are found on the "card" or "hardware"
 */
typedef UInt32 AutoRunOemManufacturerType;	/**< Device manufacturer number. */
typedef UInt32 AutoRunOemProductIDType;		/**< Device manufacturer's product number. */
typedef UInt16 AutoRunFunctionNumType;		/**< Function number from a multi-function card (ranges in value from 1-7). Not used for single-function cards */
typedef UInt16 AutoRunFunctionStandardType;	/**< I/O device (i.e. modem, UART,...) interface code. */

/**
 * @name AutoRun Source
 */

/*@{*/
typedef UInt16 AutoRunSourceType;		/**< Specifies which member of the source union to use, if any. */

#define autoRunSourceNone				((AutoRunSourceType)0)	/**< Driver source is: not used */
#define autoRunSourceSlotDriverType		((AutoRunSourceType)1)	/**< Driver source is: AutoRunSlotDriverType */
/*@}*/




/**
 * @name Media types and expected associated info
 *
 */
/*@{*/

/**
 * When the AutoRunInfoType structure's media field is set to autoRunMediaMMCmem, the oem...
 * fields are defined as:
 * AutoRunOemManufacturerType  = MMC's CID register, MID (8-bit unsigned Manufacturer field)
 * AutoRunOemProductIDType     = MMC's CID Register, OID (16 bit unsigned OEM/Application ID)
 * AutoRunFunctionNumType      = (not used)
 * AutoRunFunctionStandardType = (not used)
 * AutoRunSourceType           = autoRunSourceSlotDriverType
 */
#define	autoRunMediaMMCmem		((AutoRunMediaType)'mcmm')	/**< MMC memory cards */

/**
 * When the AutoRunInfoType structure's media field is set to autoRunMediaMMCrom, the oem...
 * fields are defined as:
 * AutoRunOemManufacturerType  = MMC's CID register, MID (8-bit unsigned Manufacturer field)
 * AutoRunOemProductIDType     = MMC's CID Register, OID (16 bit unsigned OEM/Application ID)
 * AutoRunFunctionNumType      = (not used)
 * AutoRunFunctionStandardType = (not used)
 * AutoRunSourceType           = autoRunSourceSlotDriverType
 */

#define	autoRunMediaMMCrom		((AutoRunMediaType)'mcrm')	/**< MMC ROM cards */

/**
 * When the AutoRunInfoType structure's media field is set to autoRunMediaSDmem, the oem...
 * fields are defined as:
 * AutoRunOemManufacturerType  = SD's CID register, MID (8-bit unsigned Manufacturer field)
 * AutoRunOemProductIDType     = SD's CID Register, OID (16 bit unsigned OEM/Application ID)
 * AutoRunFunctionNumType      = (not used)
 * AutoRunFunctionStandardType = (not used)
 * AutoRunSourceType           = autoRunSourceSlotDriverType
 */

#define	autoRunMediaSDmem		((AutoRunMediaType)'sdmm')	/**< SD memory cards */


/**
 * When the AutoRunInfoType structure's media field is set to autoRunMediaSDrom, the oem...
 * fields are defined as:
 * AutoRunOemManufacturerType  = SD's CID register, MID (8-bit unsigned Manufacturer field)
 * AutoRunOemProductIDType     = SD's CID Register, OID (16 bit unsigned OEM/Application ID)
 * AutoRunFunctionNumType      = (not used)
 * AutoRunFunctionStandardType = (not used)
 * AutoRunSourceType           = autoRunSourceSlotDriverType
 */

#define	autoRunMediaSDrom		((AutoRunMediaType)'sdrm')	/**< SD ROM cards */

/**
 * When the AutoRunInfoType structure's media field is set to autoRunMediaSDIO, the oem...
 * fields are defined as:
 * AutoRunOemManufacturerType  = TPLMID_MANF field inside the function's CID CISTPL_MANFID tuple (16-bit Manufacturer field)
 * AutoRunOemProductIDType     = TPLMID_CARD field inside the function's CID CISTPL_MANFID tuple (16 bit OEM/Application ID)
 * AutoRunFunctionNumType      = Function number (1-7).
 * AutoRunFunctionStandardType = I/O device interface code field in the SD card's FBR.
 * AutoRunSourceType           = autoRunSourceSlotDriverType
 */
#define	autoRunMediaSDIO		((AutoRunMediaType)'sdio')	/**< SD I/O cards */

/**
 * When the AutoRunInfoType structure's media field is set to autoRunMediaPnps, the oem...
 * fields are defined as:
 * AutoRunOemManufacturerType  = Vendor ID from the Pnps's Configuration Data Structure (16-bit unsigned field)
 * AutoRunOemProductIDType     = Device ID from the Pnps's Configuration Data Structure (16-bit unsigned field)
 * AutoRunFunctionNumType      = (not used)
 * AutoRunFunctionStandardType = (not used)
 * AutoRunSourceType           = autoRunSourceSlotDriverType
 */

#define	autoRunMediaPnps		((AutoRunMediaType)'pnps')	/**< Plug and Play for a Serial Perpherial */

/*@}*/

/**
 * @name I/O Device Interface Code
 */
/*@{*/
 #define autoRunFunctionStandardSDIOCustom			((AutoRunFunctionStandardType)0)	/**< I/O device interface code for Custom function */
 #define autoRunFunctionStandardSDIOUart			((AutoRunFunctionStandardType)1)	/**< I/O device interface code for SDIO UART */
 #define autoRunFunctionStandardSDIOBlueToothFat	((AutoRunFunctionStandardType)2)	/**< I/O device interface code for SDIO Bluetooth Fat */
 #define autoRunFunctionStandardSDIOBlueToothThin	((AutoRunFunctionStandardType)3)	/**< I/O device interface code for SDIO BlueTooth Thin */
/*@}*/

/**
 * @brief Data Structures that identify the broadcaster (the slot driver for the "source" union)
 * that issued the sysNotifyDriverSearch notification.
 *
 * This structure is a member of the AutoRunInfoType structure's source union.
 */

typedef struct AutoRunSlotDriverType	
{
    UInt16 volRefNum;       /**< The volume reference number for the mounted file system, if there is one, or vfsInvalidVolRef if there is no mounted file system. */
    UInt16 slotLibRefNum;   /**< The slot library reference number for the slot driver that issued the sysNotifyDriverSearch notification. */
    UInt16 slotRefNum;      /**< The slot reference number for the slot driver that issued the sysNotifyDriverSearch notification, or expInvalidSlotRefNum if there is no such slot. */
} AutoRunSlotDriverType;


/**
 * @brief Data Structure to be broadcast to the drivers to identify the card
 * inserted and for each registered driver to examine.
 *
 * When a card is inserted into the SD slot, after it has been initialized the SDIO slot driver broadcasts a series of
 * sysNotifyDriverSearch notifications (one for each function up to 8, on an SDIO card; only one notification is broadcast
 * for an SD or MMC memory card) in an attempt to locate function- or card-specific drivers. The notifyDetailsP field of
 * the SysNotifyParamType structure that accompanies the notification points to an AutoRunInfoType structure. Each driver
 * that has registered for sysNotifyDriverSearch should examine the contents of the AutoRunInfoType structure to determine
 * if it is the driver that should control the inserted card. If so, the driver should then check the SysNotifyParamType
 * structure's handled field.
 *
 * If handled is set to true, another driver has received the broadcast and will control the card.
 * If handled is set to false, the driver should set it to true to indicate that it will control the device.
 *
 * The AutoRunInfoType structure can also be obtained by calling SDIOGetAutoRun.
 */

typedef struct _AutoRunInfoType
{
	AutoRunMediaType    			media;           	/**< Identifies the type of card in the SD slot. */
	AutoRunOemManufacturerType		oemManufacturer;	/**< Device manufacturer number. */
	AutoRunOemProductIDType			oemID;				/**< Device manufacturer's product number. */
	AutoRunFunctionNumType			oemFunctionNum;		/**< Function number, for multi-function cards. Not used for single-function cards. */
	AutoRunFunctionStandardType		oemFunctionStandard;/**< For multi-function cards, I/O device (i.e. modem, UART,...) interface code for the function indicated by oemFunctionNum. Not used for single-function cards. */
	AutoRunSourceType   			sourceStruct;    	/**< Specifies which member of the source union to use, if any. */
	union												/**< The members of this union provides additional information about the slot driver; which member to choose is determined by the value of the sourceStruct field. Currently this union has only one member: a structure that identifies the slot driver. */
	{
	     AutoRunSlotDriverType		slotDriver;
	} source;
} AutoRunInfoType;

typedef AutoRunInfoType *AutoRunInfoP;

#endif //_AUTORUN_H
