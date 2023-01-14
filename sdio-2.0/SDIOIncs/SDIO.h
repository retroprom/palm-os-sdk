/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/

/**
 * @defgroup SDIO SDIO Library
 *
 * @brief SDIO Slot Driver API (Custom Calls)
 *
 *
 * <hr>
 * @{
 * @}
 */

/**
 *@ingroup SDIO
 *
 */

/**
 * @file 	SDIO.h
 * @brief   SDIO Slot Driver API (Custom Calls)
 * @version palmOne SDIO SDK 2.0
 * @date 	04/29/2005
 */


#ifndef __SDIO_h_
#define __SDIO_h_

#include <PalmTypes.h>
#include <SystemResources.h>
#include <NotifyMgr.h>
#include <SystemMgr.h>
#include "SlotDrvrLib.h"
#include "AutoRun.h"


#define sysFileApiCreatorSDIO 'sdio'    /**< Creator code for the SDIO slot driver. */

/**
 * @name SDIO function ID
 */

/*@{*/
/**
 * @brief The ID of the SDIO function passed to the Slot Driver
 */
typedef UInt16 SDIOCustomType;
#define sdioAPIVersion             0  	/**< valueP is a SDIOAPIVersionType */
#define sdioGetPower               1  	/**< valueP is a SDIOPowerType */
#define sdioSetPower               2  	/**< valueP is a SDIOPowerType */
#define sdioGetAutoPowerOff        3  	/**< valueP is a SDIOAutoPowerOffType */
#define sdioSetAutoPowerOff        4  	/**< valueP is a SDIOAutoPowerOffType */
#define sdioGetCurrentLimit        5  	/**< valueP is a SDIOCurrentLimitType */
#define sdioSetCurrentLimit        6  	/**< valueP is a SDIOCurrentLimitType */
#define sdioRemainingCurrentLimit  7  	/**< valueP is a SDIOCurrentLimitType */
#define sdioGetSlotInfo            8  	/**< valueP is a SDIOSlotInfoType */
#define sdioGetInfo                9  	/**< valueP is a SDIOCardInfoType */
#define sdioSetBitMode             10 	/**< valueP is a SDIOSDBitModeType, Note: this can be refused by expErrUnsupportedOperation */
#define sdioGetCallback            11 	/**< valueP is a SDIOCallbackType */
#define sdioSetCallback            12 	/**< valueP is a SDIOCallbackType */
#define sdioGetAutoRun             13 	/**< valueP is a AutoRunInfoType */
#define sdioRWDirect               14 	/**< valueP is a SDIORWDirectType */
#define sdioRWExtendedByte         15 	/**< valueP is a SDIORWExtendedByteType */
#define sdioRWExtendedBlock        16 	/**< valueP is a SDIORWExtendedBlockType */
#define sdioTupleWalk              17 	/**< valueP is a SDIOTupleType */
#define sdioAccessDelay            18 	/**< valueP is a SDIOAccessDelayType */
#define sdioDebugOptions           19 	/**< valueP is a SDIODebugOptionType */
#define sdioDisableMasterInterrupt 20 	/**< valueP is a NULL */
#define sdioEnableMasterInterrupt  21 	/**< valueP is a NULL */
/*@}*/

/**
 * @name SDIO Function Type
 */

/**
 * @brief Used with the SDIORWDirect, SDIORWExtendedByte, and SDIORWExtendedBlock functions to specify
 * the number of the SDIO card function area to be read or written, or with the SDIOTupleWalk
 * function to specify the number of the function to be searched.
 */

/*@{*/
typedef UInt16 SDIOFuncType;
#define sdioFunc0    	0   /**< This is for the SDIO function 0 area (CIA Common I/O Area). */
#define sdioFunc1    	1   /**< This is for the SDIO function 1 area */
#define sdioFunc2    	2   /**< This is for the SDIO function 2 area */
#define sdioFunc3    	3   /**< This is for the SDIO function 3 area */
#define sdioFunc4    	4   /**< This is for the SDIO function 4 area */
#define sdioFunc5    	5   /**< This is for the SDIO function 5 area */
#define sdioFunc6    	6   /**< This is for the SDIO function 6 area */
#define sdioFunc7    	7   /**< This is for the SDIO function 7 area */
#define sdioFuncEntries 8	/**< The total number of SDIO function areas */
/*@}*/

/**
 * @name SDIO Slot Type
 */

/**
 * @brief Used with a number of types and functions to identify a specific function slot driver within
 * an SDIO card.
 */

/*@{*/
typedef UInt16 SDIOSlotType;
#define sdioSlotSDMem    0 /**< This is for the SD Memory card slot (for regular memory cards or SD combo cards) */
#define sdioSlotFunc1    1 /**< This is for the SDIO function 1 Slot for SDIO cards */
#define sdioSlotFunc2    2 /**< This is for the SDIO function 2 Slot for SDIO cards */
#define sdioSlotFunc3    3 /**< This is for the SDIO function 3 Slot for SDIO cards */
#define sdioSlotFunc4    4 /**< This is for the SDIO function 4 Slot for SDIO cards */
#define sdioSlotFunc5    5 /**< This is for the SDIO function 5 Slot for SDIO cards */
#define sdioSlotFunc6    6 /**< This is for the SDIO function 6 Slot for SDIO cards */
#define sdioSlotFunc7    7 /**< This is for the SDIO function 7 Slot for SDIO cards */
/*@}*/


typedef UInt32 SDIOAPIVersionType;    /**< Used to hold the version number of the SDIO slot driver */

/**
 * @name SDIO Card Power Type
 */

/**
 * @brief Used with SDIOPowerType and SDIOAutoPowerOffType to specify whether the SDIO card's power
 * and data signals should be turned on or off, whether the SD Memory section of a combo card
 * should be reset, or whether to wait for the SDIO portion of an SD card to be ready.
 */

/*@{*/
typedef UInt16 SDIOCardPowerType;
#define sdioCardPowerOff    0    /**< Turn off the card, put the data signals in a low power state  */
#define sdioCardPowerOn     1    /**< Power on and initialize the card */
#define sdioCardResetSDMem  2    /**< This will force the SDMem portion of a SD combo card to be software reset by a CMD0, the function will return after the card is initialized. This value cannot be used with SDIOSetAutoPowerOff. */
#define sdioCardWaitSDIO    3    /**< Wait for the I/O portion of an SDIO card to be ready (after IO_SEND_OP_COND CMD5). Use this after resetting 1 or more function(s). This value cannot be used with SDIOSetAutoPowerOff. */
/*@}*/

/**
 * @brief Used by SDIOGetPower and SDIOSetPower to get and set an SD card function's power setting.
 */

typedef struct
{
	SDIOSlotType 		sdioSlotNum;  /**< Identifies a specific SDIO card function's slot driver. */
	SDIOCardPowerType	powerOnCard;  /**< An SDIOCardPowerType that specifies whether power should be applied to or removed from the SDIO card function, or that indicates whether power is or is not currently being applied to the function. */
} SDIOPowerType;

/**
 * @brief Used with SDIOGetCurrentLimit, SDIOSetCurrentLimit, and SDIORemainingCurrentLimit to specify
 * an SDIO card function and the maximum current that can be required by that function.
 */

typedef struct
{
	SDIOSlotType	sdioSlotNum;	/**< Identifies a specific function slot driver withan an SDIO card. */
	UInt32			uaMaximum;		/**< The specified function's maximum peak current in mico-amps (when used with SDIOGetCurrentLimit or SDIOSetCurrentLimit), or the remaining maximum current for the entire card in micro-amps (when used with SDIORemainingCurrentLimit). */
} SDIOCurrentLimitType;

/**
 *
 * @brief Used with SDIOGetAutoPowerOff and SDIOSetAutoPowerOff to specify auto-power-off parameters
 * for a specific SDIO card function.
 *
 */

typedef struct
{
	SDIOSlotType		sdioSlotNum;	/**< Identifies a specific SDIO card function's slot driver. */
	UInt16 				ticksTillOff;	/**< The amount of time, in system ticks, before power to the SDIO card function is automatically turned off. A value of zero disables the auto-power-off function. */
	SDIOCardPowerType	sleepPower;		/**< Specifies whether the SDIO card function's power and data signals should be turned on or off, whether the SD Memory card portion of a combo card should be reset, or whether to wait for the SDIO portion of an SD card to be ready. */
} SDIOAutoPowerOffType;

/**
 * @brief This structure is used with SDIOGetSlotlnfo to obtain information about a specific
 * SDIO function's slot driver.
 */

typedef struct
{
	SDIOSlotType	sdioSlotNum;   /**< The ID of the SDIO card function in the current slot driver about which slot library information is desired. */
	UInt16    		volRefNum;     /**< The volume reference number for the mounted file system, if there is one, or vfsInvalidVolRef */
				                   /**< if there is no mounted file system for the specified SDIO card function.*/
	UInt16    		slotLibRefNum; /**< The slot library reference number for the specified SDIO card function. */
	UInt16    		slotRefNum;    /**< The slot reference number for the specified SDIO card function, if there is one, or expInvalidSlotRefNum if there isn't. */
} SDIOSlotInfoType;

/**
 * @name SDIO Bits Of File System Type
 */

/**
 * @brief Returned as part of an SDIOCardInfoType structure, each of the bits that makes
 * up SDIOBitsOfFileSystemType indicates whether the corresponding function
 * has a standard SDIO file system. Note that this file system may or may not be
 * mounted.
 */

/*@{*/
typedef UInt16 SDIOBitsOfFileSystemType;
#define  sdioBitsOfFileSystemMemory     0x0001    /**< This card has a standard SDIO file system in the SD Memory section. */
#define  sdioBitsOfFileSystemFunction1  0x0002    /**< This card has a standard SDIO file system in function 1 */
#define  sdioBitsOfFileSystemFunction2  0x0004    /**< This card has a standard SDIO file system in function 2 */
#define  sdioBitsOfFileSystemFunction3  0x0008    /**< This card has a standard SDIO file system in function 3 */
#define  sdioBitsOfFileSystemFunction4  0x0010    /**< This card has a standard SDIO file system in function 4 */
#define  sdioBitsOfFileSystemFunction5  0x0020    /**< This card has a standard SDIO file system in function 5 */
#define  sdioBitsOfFileSystemFunction6  0x0040    /**< This card has a standard SDIO file system in function 6 */
#define  sdioBitsOfFileSystemFunction7  0x0080    /**< This card has a standard SDIO file system in function 7 */
/*@}*/

/**
 * @name SDIO Bits Of Status Type
 */

/**
 * @brief Returned as part of an SDIOCardInfoType structure, each of the bits that makes
 * up SDIOBitsOfStatusType indicates various status information about the SDIO
 * card.
 */

/*@{*/
typedef UInt16 SDIOBitsOfStatusType;
#define  sdioBitsOfStatusDriverHandledMemory 0x0001 /**< This card has an Auto Run function driver in the SD Memory section */
#define  sdioBitsOfStatusDriverHandledFunc1  0x0002 /**< This card has an Auto Run function driver in function 1 */
#define  sdioBitsOfStatusDriverHandledFunc2  0x0004 /**< This card has an Auto Run function driver in function 2 */
#define  sdioBitsOfStatusDriverHandledFunc3  0x0008 /**< This card has an Auto Run function driver in function 3 */
#define  sdioBitsOfStatusDriverHandledFunc4  0x0010 /**< This card has an Auto Run function driver in function 4 */
#define  sdioBitsOfStatusDriverHandledFunc5  0x0020 /**< This card has an Auto Run function driver in function 5 */
#define  sdioBitsOfStatusDriverHandledFunc6  0x0040 /**< This card has an Auto Run function driver in function 6 */
#define  sdioBitsOfStatusDriverHandledFunc7  0x0080 /**< This card has an Auto Run function driver in function 7 */
#define sdioBitsOfStatusWriteProtectTab      0x0100 /**< The write protect tab on the card indicates that this card is write protected */
/*@}*/

/**
 * @name SDIO SD Bit Mode Type
 */

/**
 * @brief Used in the SDIOCardInfoType and SDIOSDBitModeRequestType structures to indicate which SDIO bit
 * mode is to be used when interacting with a particular SDIO card function.
 */

/*@{*/
typedef UInt16 SDIOSDBitModeType;
#define    sdioSD1BitMode    1 /**< SDIO bit mode 1 (SD or SPI mode) */
#define    sdioSD4BitMode    4 /**< SDIO bit mode 4 (SD mode only) */
/*@}*/

/**
 * @brief This structure is passed to the SDIOSetBitMode function and both specifies the
 * function making the request and the SDIO bit mode that is to be set.
 */

typedef struct
{
	SDIOSlotType		requestingFunc; /**< The number of the function making this request. */
	SDIOSDBitModeType	bitMode;        /**< The requested SDIO bit mode. */
} SDIOSDBitModeRequestType;

/**
 * @brief This structure is used in conjunction with the SDIOGetCardlnfo function to return
 * information about the SDIO card.
 */
typedef struct
{
	UInt16                      numberOfFunctions;  /**< Number of SDIO functions on the card. This field's values range from 0 (no functions) to 7. */
	SDIOSDBitModeType           bitMode;            /**< The card's current SDIO bit mode. */
	SDIOBitsOfFileSystemType    bitsOfFileSystem;   /**< Each bit in this field indicates whether the corresponding function has a standard SDIO file system.  */
						                            /**< Note that just because a function has a file system, it does not mean that the file system is mounted.  */
	SDIOBitsOfStatusType        bitsOfStatus;       /**< Various status bits, as defined under SDIOBitsOfStatusType. */
} SDIOCardInfoType;


/**
 *
 *  @brief SDIOCallbackPtr - Perform driver-specific processing when one of the following occurs:
 *         - the SDIO interrupt is received
 *         - the handheld is going to sleep or has just awakened
 *         - the SDIO card was just turned off or on
 *         - the SDIO card was just reset
 *         - the bus width changed
 *
 *         Which of the above events causes a given callback function to be called, if any, depends
 *         on what was passed to SDIOSetCallback..
 *
 *  @param sdioSlotNum: IN: The ID of the SDIO card function in the current slot driver that generated the
 *                          callback.
 *  @param userDataP:	IN: Pointer to a block of user data specified when the callback was set up using
 *                          SDIOSetCallback.
 *
 *  @retval  errNone  -- if the callback function executed properly, or any other value if an
 *                       error occurred during the processing of the callback..
 *
 *  @note  In your callback function you may not have control of the user interface or access
 *         to your variables. Any necessary data should be made available to your callback
 *         function through the use of userDataP. Be sure to keep memory for your callback
 *         functions and variables locked from the time you set up the callback function to the
 *         time when it can no longer be called.
 *
 *         Callback function information is automatically erased after a card is inserted or
 *         removed (before the card removal event). To detect card removal, use the
 *         notification manager and register for sysNotifyCardRemovedEvent.
 *
 *         Your callback functions can be called from within an interrupt service routine, and
 *         interrupts can occur at any time. The interrupt can even generate a wakeup event
 *         if the handheld is asleep and power to the card is still on. As with an interrupt
 *         service routine, your callback functions can only call a limited set of system
 *         functions (those that are interrupt-safe) and must execute quickly. Your callback
 *         functions do have access to any slot driver functions accessible through the
 *         SlotCustomControl function.
 *
 */
typedef Err (*SDIOCallbackPtr)( SDIOSlotType sdioSlotNum, void *userDataP );


/**
 * @name SDIO Callback Select Type
 */

/**
 * @brief Values of type SDIOCallbackSelectType are used in an SDIOCallbackType structure to identify
 * which of an SDIO card function's callbacks is needed or is to be set.
 */

/*@{*/
typedef UInt16 SDIOCallbackSelectType;

/**
 * Warning, the first 5 callbacks can be called from an interrupt and a non-interrupt routine!
 */

#define sdioCallbackSelectInterruptSDCard    0    /**< Callback that occurs when an SDIO card interrupts the handheld (Note: This callback is made during the processing of an interrupt.) */
#define sdioCallbackSelectSleep              1    /**< Callback that occurs when the handheld wants to go to sleep. */
#define sdioCallbackSelectAwake              2    /**< Callback that occurs when the handheld wants to wake up. */
#define sdioCallbackSelectPowerOn            3    /**< Callback that occurs when the SDIO card power is turned on. */
#define sdioCallbackSelectPowerOff           4    /**< Callback that occurs when the SDIO card power is turned off. */

#define sdioCallbackSelectReset              5    /**< Callback that occurs when the SDIO section of the card is reset. */
#define sdioCallbackSelectBitMode            6	  /**< Callback that occurs when the bus width of the card is changed. */
#define sdioCallbackSelectEntries            7    /**< The number of possible callbacks for a given SDIO card function. */
/*@}*/

/**
 *
 *  @brief Used in conjunction with the SDIOGetCallback and SDIOSetCallback functions, this structure
 *  associates a C function with a particular SDIO function callback type.
 *
 */

typedef struct
{
	SDIOSlotType 			sdioSlotNum;     /**< Identifies the SDIO card function whose callback is needed or is to be set. */
	SDIOCallbackSelectType	callbackSelect;  /**< Identifies the particular callback that is needed or is to be set. */
	SDIOCallbackPtr			callBackP;       /**< Pointer to callback function. */
	MemPtr					userDataP;       /**< Pointer to a block of user data that is passed to the callback function when the function is called. */
} SDIOCallbackType;

/**
 * @brief This structure is passed to the SDIOGetAutoRun function and is used to specify the
 * function making the request and the SDIO bit mode that is to be set.
 */

typedef struct
{
	SDIOSlotType 	sdioSlotNum; /**< The ID of the function in the current slot driver about which slot library information is desired.  */
				                 /**< Note that sdioFunc0 is reserved for the SD Memory card slot driver and function 0. */
	AutoRunInfoType autoRun;     /**< Contains a description of the SD/MMC memory card or SDIO card that is currently inserted. */
} SDIOAutoRunInfoType;

/**
 * @name SDIO RW Mode Type
 */

/**
 * @brief Specifies the particular operation to be performed when using SDIORWDirect, SDIORWExtendedBlock
 * , and SDIORWExtendedByte.
 */

/*@{*/
typedef UInt16 SDIORWModeType;
#define sdioRWModeWrite            0x0001    /**< Write data from the specified buffer to the card. */
#define sdioRWModeRead             0x0002    /**< Read data from the card and place it in the specified buffer. */
#define sdioRWModeWriteRead        (sdioRWModeWrite | sdioRWModeRead)    /**< Write data from the specified buffer to the card, then read the data from the card and place it back into the buffer. */
#define sdioRWModeFixedAddress     0x0004    /**< Used in combination with sdioRWModeWrite, sdioRWModeRead, or sdioRWModeWriteRead to perform a multi-byte read or write to a single register address. Useful when transferring data using a FIFO inside the I/O device. */
#define sdioRWModeForceBlockSize   0x0008    /**< Used in combination with sdioRWModeWrite, sdioRWModeRead, or sdioRWModeWriteRead to cause SDIORWExtendedBlock to always set the block size. This is useful if the driver resets and I/O only card or the I/O portion of a combo card, or if it alters the I/O block length in the FBR (Function Basic Registers). */
/*@}*/

/**
 * @brief This structure is used with SDIORWDirect and encapsulates the read or write
 * operation.
 */
typedef struct
{
	SDIOSlotType	requestingFunc;  /**< The number of the SDIO card function making the read or write request. This is */
                                     /**( the function that will be turned on. */
	SDIORWModeType	mode;            /**< The operation to be performed: write, read, or write followed by read. */
	SDIOFuncType	funcNum;         /**< The number of the function within the I/O card to be read or written. Function 0 selects the common I/O area (CIA) */
	UInt32			byteAddress;     /**< The address of the byte inside of the selected SDIO card function's register space that will be read or written. */
				                     /**< There are 17 bits of address available, so the byte must be located within the first 128K addresses of that function. */
	UInt8			byteData;        /**< For a direct write command, the byte that will be written. For a direct read command, the byte read is stored here. */
} SDIORWDirectType;

/**
 * @brief This structure is used with SDIORWExtendedByte and encapsulates the read or write
 * operation.
 */

typedef struct
{
	SDIOSlotType	requestingFunc; /**< The number of the SDIO card function making the read or write request. This is the function that will be turned on.*/
	SDIORWModeType	mode;           /**< The operation to be performed. */
	SDIOFuncType	funcNum;        /**< The number of the function within the I/O card to be read or written. Function 0 selects the common I/O area (CIA). */
	UInt32			byteAddress;    /**< The address of the first byte inside of the selected SDIO card function's register space that will be read or written. */
				                    /**< There are 17 bits of address available, so the byte must be located within the first 128K addresses of that function. */
	MemPtr			bufferP;        /**< For an extended write command, a pointer to the data that will be written. For an extended read command, the data read is stored in the indicated buffer.  */
	UInt16			numBytes;       /**< The number of bytes to transfer, up to 512. A numBytes value of either 512 or 0 indicates that 512 bytes are to be transferred. */
} SDIORWExtendedByteType;

/**
 * @brief This structure is used with SDIORWExtendedBlock and encapsulates the read or
 * write operation.
 */

typedef struct
{
	SDIOSlotType	requestingFunc; /**< The number of the SDIO card function making the read or write request. This is */
                                    /**( the function that will be turned on. */
	SDIORWModeType 	mode;           /**< The operation to be performed. */
	SDIOFuncType 	funcNum;        /**< The number of the function within the I/O card to be read or written. Function 0 selects the common I/O area (CIA) */
	UInt32			byteAddress;    /**< The address of the first byte inside of the selected SDIO card function's register space that will be read or written. */
				                    /**< There are 17 bits of address available, so the byte must be located within the first 128K addresses of that function. */
	MemPtr			bufferP;        /**< For an extended write command, a pointer to the data that will be written. For an extended read command, the data read is stored in the indicated buffer.*/
	UInt16			numBlocks;      /**< The number of blocks to transfer, up to 511. A value of 0 indicates that the block transfer should go on until */
				                    /**< explicitly stopped, but that mode is not supported by this SDIO slot driver. */
	UInt16			ioBlockSize;    /**< The size of each block to be transferred. This value should range from 1 to 2048; all other values are illegal. */
} SDIORWExtendedBlockType;

/**
 * @brief This structure is used with SDIOTupleWalk and both identifies the data block (tuple)
 * to be located and indicates where the search results should be placed.
 */

typedef struct
{
	SDIOSlotType	requestingFunc; /**< The number of the SDIO card function making the read or write request. This is the function that will be turned on. */
	SDIOFuncType	funcNum;        /**< The number of the function within the I/O card to be searched. Function 0 selects the common I/O area (CIA). */
	UInt8			tupleToFind;	/**< The tuple code that identifies the desired data block. */
	MemPtr			bufferP;        /**< Pointer to a buffer into which the contents of the tuple are placed. */
	UInt16			bufferSizeOf;   /**< The size of the supplied buffer. This buffer should be large enough to contain the entire tuple (including bytes for the tuple  */
				                    /**< code and the tuple body size). According to the SDIO specification, a tuple is never larger than 257 bytes */
} SDIOTupleType;

/**
 *	@brief The maximum timeout, in milliseconds, for reads and writes using the IO_RW_DIRECT and IO_RW_EXTENDED commands.
 */

typedef UInt16 SDIOAccessDelayType; /**< The maximum timeout, in milliseconds, for reads and writes using the IO_RW_DIRECT and IO_RW_EXTENDED commands. */


/**
 * @name SDIO Debug Option Type
 */

/**
 * @brief This is used with SDIODebugOptions and identifies which debug messages, if any, are to be sent to
 * the serial or USB port. These messages are sent on debug ROMs and debug RAM patches only.
 */

/*@{*/
typedef UInt16 SDIODebugOptionType;
#define sdioDebugOptionTraceCmds      ((SDIODebugOptionType)0x0001) /**< Sends all commands that are issued to the card */
#define sdioDebugOptionTraceRejection ((SDIODebugOptionType)0x0002) /**< Sends rejection reasons */
#define sdioDebugOptionTraceCmdData   ((SDIODebugOptionType)0x0004) /**< Sends the data from commands that have command/response/data, warning, This is alot of data! */
#define sdioDebugOptionTraceContents  ((SDIODebugOptionType)0x0008) /**< Sends the contents of Tuples and/or parts of the CSD (Card Specific Data register) when they are accessed just after card insertion */
#define sdioDebugOptionTraceProgress  ((SDIODebugOptionType)0x0010) /**< Sends the progress of the tests that are performed upon card insertion  */
#define sdioDebugOptionTraceISR		  ((SDIODebugOptionType)0x0020) /**< Allows debug messages to be sent from within interrupt routines, Warning: keep stack small to avoid overflows */

#define sdioDebugOptionTraceMost       (sdioDebugOptionTraceCmds | sdioDebugOptionTraceRejection | sdioDebugOptionTraceContents | sdioDebugOptionTraceProgress | sdioDebugOptionTraceISR)
#define sdioDebugOptionTraceAll        ((UInt16)-1) 				/**< Enable all options. */
#define sdioDebugOptionTraceNone       (0)  						/**< Disable all options. */
/*@}*/


/**
 *
 *  @brief  SDIOAPIVersion - This call is used to detect if the Slot Driver is SDIO aware and returns the SDIO slot driver
 * 		    version number.
 *          If it returns "errNone" the Slot driver is SDIO aware and the version number
 *          of the SDIO part of the slot driver is in the SDIOAPIVersionType.
 *
 *  @param slotLibRefNum:	IN:  SlotDriver library reference number
 *  @param versionP:		OUT: Pointer to hold the version number of this SDIO aware slot driver
 *
 *  @retval errNone                                  -- The specified slot driver is SDIO aware,
 *                                                      and the version number was successfully set.
 *          sysErrParamErr                           -- versionP is invalid.
 *          expErrUnimplemented (or any other error) -- The specified slot driver does not support SDIO.
 *
 *  @note  This function can safely be called from within an interrupt service routine.
 *         It does not require a SDIO card in the slot to work.
 */
inline Err SDIOAPIVersion( UInt16 slotLibRefNum, SDIOAPIVersionType *versionP );
inline Err SDIOAPIVersion( UInt16 slotLibRefNum, SDIOAPIVersionType *versionP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioAPIVersion, versionP, NULL ) );
}

/**
 *
 *  @brief SDIOGetPower - Determine whether an SD card function is currently powered on or off.
 *
 *  @param slotLibRefNum: IN:      SlotDriver library reference number
 *  @param powerP:		  IN/OUT:  Pointer to an SDIOPowerType structure. Before calling SDIOGetPower set this
 *                                 structure's sdioSlotNum field to the SDIO card function, and upon return the value
 *                                 of this structure's powerOnCard field indicates whether or not the SD card function
 *                                 is turned on.
 *
 *  @retval errNone              -- No error.
 *          sysErrParamErr       -- powerP is invalid.
 *          expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *          expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *
 *  @note  This function does not check the battery level, since turning on the SDIO card might lockout the handheld.
 *         It also does not check SDIO card function current limits. A card must be present
 *         in the SD slot in order to use this function, however.
 *
 *         This function can safely be called from within an interrupt service routine.
 *
 */
inline Err SDIOGetPower( UInt16 slotLibRefNum, SDIOPowerType *powerP );
inline Err SDIOGetPower( UInt16 slotLibRefNum, SDIOPowerType *powerP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetPower, powerP, NULL ) );
}
/**
 *
 *  @brief SDIOSetPower - Turns an SDIO card function on or off.
 *
 *  @param slotLibRefNum: IN:  SlotDriver library reference number
 *  @param powerP:		  IN:  Pointer to an SDIOPowerType structure. Before calling SDIOSetPower set this
 *                             structure's sdioSlotNum field to indicate the SDIO card function to be turned on or
 *                             off, and set the powerOnCard field to one of the values defined for
 *                             SDIOCardPowerType.
 *
 *  @retval errNone              -- No error.
 *          sysErrParamErr       -- powerP is invalid.
 *          expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *	        expErrCardBadSector  -- The card could not be initialized.
 *          expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *
 *  @note  When used to turn an SDIO card function on, SDIOSetPower if necessary, sets the power and bus signals such that the entire
 *         slot is turned on: power is applied to the card and the data bus is ready to transmit or receive commands.
 *         When used to turn an SDIO card function off, power is removed from the card and the data bus is set to a low power state
 *         if no other functions are on. Note that when the card is turned off, SDIO interrupts cannot occur.
 *
 *         SDIOSetPower requires that an SDIO card be present in the slot. Turning on power causes
 *         the card to be accessed, for initialization.
 *
 *         This function can safely be called from within an interrupt service routine.
 *         However, you should not call SDIOSetPower from your sdioCallbackSelectPowerOn or
 *         sdioCallbackSelectPowerOff callback functions.
 *
 */
inline Err SDIOSetPower( UInt16 slotLibRefNum, SDIOPowerType *powerP );
inline Err SDIOSetPower( UInt16 slotLibRefNum, SDIOPowerType *powerP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioSetPower, powerP, NULL ) );
}


/**
 *
 *  @brief SDIOGetAutoPowerOff - Get the current auto-power-off settings for the SD slot.
 *         Every time a card is inserted the card will default to:
 *              "ticksTillOff" = about 15 seconds
 *              "sleepPower" = sdioCardPowerOff
 *              Another default behavior of the card is when the handheld wakes up, it does not turn on the card.
 *              Instead it waits until there is a request to use the card. Then the card is turned on.
 *
 *  @param slotLibRefNum: IN:      SlotDriver library reference number
 *  @param autoP:		  IN/OUT:  Pointer to an SDIOAutoPowerOffType structure that indicates the current auto
 *                                 power-off settings for the SD slot. Before calling SDIOGetAutoPowerOff, set this
 *                                 structure's sdioSlotNum field to indicate the current slot driver function number.
 *                                 Upon return, the ticksTillOff field indicates the number of system ticks until the
 *                                 slot is turned off. A ticksTillOff value of zero indicates that auto-off is disabled..
 *
 *  @retval errNone                                  -- No error.
 *          sysErrParamErr                           -- autoP is invalid.
 *          expErrCardNotPresent                     -- There isn't a card in the slot associated with the specified slot driver.
 *          expErrUnimplemented (or any other error) -- The specified slot driver does not support SDIO.
 *
 *
 *  @note  This function requires that an SDIO card be present in the slot, since these settings are erased when a card is removed.
 *         Note that every time a card is inserted into the SD slot, the auto-power-off time is set to 15 seconds.
 *         When the handheld awakes from sleep mode, it doesn't turn the card on. Only when there is a request to access the card does it turn the card on.
 *         This function only works with SDIO cards; it cannot be used when a memory card is in the slot.
 *         This function can safely be called from within an interrupt service routine.These routines are safe for interrupts.
 */
inline Err SDIOGetAutoPowerOff( UInt16 slotLibRefNum, SDIOAutoPowerOffType *autoP );
inline Err SDIOGetAutoPowerOff( UInt16 slotLibRefNum, SDIOAutoPowerOffType *autoP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetAutoPowerOff, autoP, NULL ) );
}
/**
 *
 *  @brief SDIOSetAutoPowerOff - Alter the auto-power-off settings for the SD slot.
 *
 *  @param slotLibRefNum: IN:  SlotDriver library reference number
 *  @param autoP:		  IN:  Pointer to an SDIOAutoPowerOffType structure which specifies the auto-power-off
 *                             settings for the SD slot. Before calling SDIOSetAutoPowerOff, set this structure's
 *                             sdioSlotNum field to indicate the current slot driver function number, and set the
 *                             ticksTillOff field to the desired number of system ticks until the slot is turned off
 *                             (a value of zero disables auto-off). Set the sleepPower field to sdioCardPowerOff to
 *                             turn the slot off after the specified period of time.
 *
 *  @retval errNone                                  -- No error.
 *          sysErrParamErr                           -- autoP is invalid, or one of the fields of autoP is invalid.
 *          expErrCardNotPresent                     -- There isn't a card in the slot associated with the specified slot driver.
 *          expErrUnimplemented (or any other error) -- The specified slot driver does not support SDIO.
 *
 *
 *  @note  This function requires that an SDIO card be present in the slot, since these settings are erased when an SDIO card is removed.
 *         Note that every time a card is inserted into the SD slot, the auto-power-off time is set to 15 seconds.
 * 	
 *         When the handheld awakes from sleep mode, it doesn't turn the card on. Only when there is a request to access the card does it turn the card on.
 *	
 *         This function only works with SDIO cards; it cannot be used when a memory card is in the slot.
 *	
 *         This function can safely be called from within an interrupt service routine.
 */
inline Err SDIOSetAutoPowerOff( UInt16 slotLibRefNum, SDIOAutoPowerOffType *autoP );
inline Err SDIOSetAutoPowerOff( UInt16 slotLibRefNum, SDIOAutoPowerOffType *autoP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioSetAutoPowerOff, autoP, NULL ) );
}

/**
 *
 *  @brief SDIOGetCurrentLimit - Get the maximum peak current allotted to one of the SDIO card's functions.
 *
 *  @param slotLibRefNum: IN:     SlotDriver library reference number
 *  @param currentLimitP: IN/OUT: Pointer to an SDIOCurrentLimitType structure. Before calling SDIOGetCurrentLimit,
 *                                set this structure's slotFuncNum field to a valid slot driver function number. Upon
 *                                return, the uaMaximum field contains the specified function's maximum peak current
 *                                in micro amps.
 *
 *  @retval  errNone              -- No error.
 *           sysErrParamErr       -- currentLimitP is invalid, or one of currentLimitP's fields is invalid.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *
 *  @note You use SDIOGetCurrentLimit, SDIOSetCurrentLimit, and
 *        SDIORemainingCurrentLimit to ensure that the total of all function hardware that is
 *        active never exceeds the SDIO specification maximum of 200ma. These three
 *        functions do not detect or limit current draw, check the battery level, or reflect how
 *        much energy the battery has left; you simply use them to manage the current limit
 *        values supplied using SDIOSetCurrentLimit. It is up to the writer of the SDIO slot
 *        driver to both supply the proper current limit values and to use
 *        SDIOGetCurrentLimit and SDIORemainingCurrentLimit appropriately so that the
 *        total active SDIO card functions do not draw more current than the handheld's
 *        power source can provide.
 *
 *        When a card is removed, all allocations of current are set to zero. Because of this,
 *        in order to operate properly this function requires an SDIO card in the slot.
 *
 *        This function can safely be called from within an interrupt service routine.
 */
inline Err SDIOGetCurrentLimit( UInt16 slotLibRefNum, SDIOCurrentLimitType *currentLimitP );
inline Err SDIOGetCurrentLimit( UInt16 slotLibRefNum, SDIOCurrentLimitType *currentLimitP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetCurrentLimit, currentLimitP, NULL ) );
}

/**
 *
 *  @brief SDIOSetCurrentLimit - Set the maximum peak current needed by one of the SDIO card's functions
 *
 *  @param slotLibRefNum:	IN: SlotDriver library reference number
 *  @param currentLimitP:	IN: Pointer to an SDIOCurrentLimitType structure. Before calling SDIOSetCurrentLimit,
 *                              set this structure's slotFuncNum field to a valid slot driver function number, and set
 *                              the uaMaximum field to the maximum peak current, in micro-amps, required by the
 *                              function indicated by slotFuncNum.
 *
 *  @retval  errNone              -- No error.
 *           sysErrParamErr       -- currentLimitP is invalid, or one of currentLimitP's fields is invalid.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *
 *
 *  @note  You use SDIOGetCurrentLimit, SDIOSetCurrentLimit, and SDIORemainingCurrentLimit to ensure that
 *         the total of all function hardware that is active never exceeds the SDIO specification
 *         maximum of 200ma. These three functions do not detect or limit current draw,
 * 	       check the battery level, or reflect how much energy the battery has left; you simply use
 *         them to manage the current limit values supplied using SDIOSetCurrentLimit. It is up to
 *         the writer of the SDIO slot driver to both supply the proper current limit values and
 * 	       to use SDIOGetCurrentLimit and SDIORemainingCurrentLimit appropriately so that the total
 *         active SDIO card functions do not draw more current than the handheld's power source can provide.
 *
 *         Note that this function doesn't write the supplied peak current value to the card;
 *         it only sets the value in RAM.
 *
 *         When a card is removed, all allocations of current are set to zero. Because of this, in
 *         order to operate properly this function requires an SDIO card in the slot.
 *
 *         This function can safely be called from within an interrupt service routine.
 */
inline Err SDIOSetCurrentLimit( UInt16 slotLibRefNum, SDIOCurrentLimitType *currentLimitP );
inline Err SDIOSetCurrentLimit( UInt16 slotLibRefNum, SDIOCurrentLimitType *currentLimitP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioSetCurrentLimit, currentLimitP, NULL ) );
}

/**
 *
 *  @brief SDIORemainingCurrentLimit - Get the remaining current for the entire SDIO card.
 *
 *  @param slotLibRefNum:	IN:  SlotDriver library reference number
 *  @param currentLimitP:	OUT: Pointer to an SDIOCurrentLimitType structure. Upon return, the uaMaximum field
 *                               indicates how much current, in micro-amps, remains unallocated by the SDIO
 *                               card's functions. Note that the slotFuncNum field isn't used when calling this
 *                               function.
 *
 *  @retval  errNone              -- No error.
 *           sysErrParamErr       -- currentLimitP is invalid, or one of currentLimitP's fields is invalid.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *
 *
 *  @note  You use SDIOGetCurrentLimit, SDIOSetCurrentLimit, and SDIORemainingCurrentLimit to ensure that the total of all function hardware
 *         that is active never exceeds the SDIO specification maximum of 200ma. These three functions do not detect or limit current draw,
 *         check the battery level, or reflect how much energy the battery has left; you simply use them to manage the current limit values
 *         supplied using SDIOSetCurrentLimit. It is up to the writer of the SDIO slot driver to both supply the proper current limit values
 *         and to use SDIOGetCurrentLimit and SDIORemainingCurrentLimit appropriately so that the total active SDIO card functions do not
 *         draw more current than the handheld's power source can provide.
 *
 *         When a card is removed, all allocations of current are set to zero. Because of this, in order to operate properly this function requires an SDIO card in the slot.
 *
 *         This function can safely be called from within an interrupt service routine.
 */
inline Err SDIORemainingCurrentLimit ( UInt16 slotLibRefNum, SDIOCurrentLimitType *currentLimitP );
inline Err SDIORemainingCurrentLimit ( UInt16 slotLibRefNum, SDIOCurrentLimitType *currentLimitP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioRemainingCurrentLimit, currentLimitP, NULL ) );
}

/**
 *
 *  @brief SDIOGetSlotInfo - Obtain the slot driver reference number, the slot driver library reference number,
 *         and the volume reference number for the associated file system, if any, for one of the SDIO functions or for the SD Memory card slot driver.
 *
 *  @param slotLibRefNum: IN:      SlotDriver library reference number
 *  @param slotInfoP:	  IN/OUT:  Pointer to an SDIOSlotInfoType structure. Before calling this function, set the
 *                                 sdioSlotNum field to indicate the function for which the slot driver information is
 *                                 needed. Upon return, the slotLibRefNum, slotRefNum, and volRefNum fields are set
 *
 *  @retval  errNone             -- No error.
 *           sysErrParamErr      -- slotInfoP is invalid.
 *           expErrUnimplemented -- The specified slot driver does not support SDIO.
 *
 *  @note  This function does not require that an SDIO card be present in the slot.
 *         This function can safely be called from within an interrupt service routine.
 */
inline Err SDIOGetSlotInfo( UInt16 slotLibRefNum, SDIOSlotInfoType *slotInfoP );
inline Err SDIOGetSlotInfo( UInt16 slotLibRefNum, SDIOSlotInfoType *slotInfoP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetSlotInfo, slotInfoP, NULL ) );
}

/**
 *
 *  @brief SDIOGetCardInfo - Return information about the SDIO card obtained with the SDIO IO_QUERY command (ACMD57).
 *         All other information on the card can be obtained by the normal SDIO read & write calls.
 *
 *  @param slotLibRefNum:	IN:  SlotDriver library reference number
 *  @param cardInfoP:		OUT: Pointer to an SDIOCardInfoType structure into which the SDIO card information is
 *                               placed.
 *
 *  @retval  errNone              -- No error.
 *           sysErrParamErr       -- cardInfoP is invalid.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *
 *  @note  Information about the SDIO card other than what is returned by this function can be obtained
 *         through the use of the normal SDIO read and write calls.
 *	
 *         This function can safely be called from within an interrupt service routine.
 *	
 *         This function requires that an SDIO card be present in the slot. Note, however,
 *         that information is cached in RAM.
 */
inline Err SDIOGetCardInfo( UInt16 slotLibRefNum, SDIOCardInfoType *cardInfoP );
inline Err SDIOGetCardInfo( UInt16 slotLibRefNum, SDIOCardInfoType *cardInfoP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetInfo, cardInfoP, NULL ) );
}

/**
 *
 *  @brief SDIOSetBitMode - Change the bus width.
 *
 *  @param slotLibRefNum:	IN:     SlotDriver library reference number
 *  @param bitModeRequestP:	IN/OUT: Pointer to an SDIOSDBitModeRequestType structure that indicates which function is
 *                                  making the request, and which bit mode to set.
 *
 *  @retval  errNone                    -- No error.
 *           sysErrParamErr             -- bitModeRequestP is invalid.
 *           expErrUnsupportedOperation -- The SDIO card does not support the requested bit mode.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *
 *  @note  Set the bit mode after the card has been inserted but before setting any callbacks. Be sure to check the value returned
 *         from this function: due to hardware constraints, this command can be rejected, returning expErrUnsupportedOperation.
 *
 *         The current bit mode can be obtained with a call to SDIOGetCardlnfo.
 *
 *         This function requires that an SDIO card be present in the slot, and it may turn on and access the card.
 *
 *         This function can safely be called from within an interrupt service routine.
 *
 *
 */
inline Err SDIOSetBitMode( UInt16 slotLibRefNum, SDIOSDBitModeRequestType *bitModeRequestP );
inline Err SDIOSetBitMode( UInt16 slotLibRefNum, SDIOSDBitModeRequestType *bitModeRequestP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioSetBitMode, bitModeRequestP, NULL ) );
}

/**
 *
 *  @brief SDIOGetCallback - Obtain pointers to an SDIO card function's callback routine and associated data.
 *
 *  @param slotLibRefNum: IN:     slotDriver library reference number
 *  @param callBackP:	  IN/OUT: Pointer to an SDIOCallbackType structure. Before calling this function, set the
 *                                sdioSlotNum and callbackSelect fields. Upon return, the callBackP and userDataP
 *                                fields point to the callback function and any associated user data. Either or both of
 *                                these pointers can be NULL to indicate that there is no associated callback function
 *                                or that there is no user data block.
 *
 *  @retval  errNone              -- No error.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *           sysErrParamErr       -- callBackP is invalid, or one of its fields is invalid.
 *
 *  @note  Each callback selector for each card function slot, as well as the SD Memory card slot, may have been assigned a separate callback function.
 *   	   The following list of callback selectors details those situations in which a particular callback function is called:
 *
 *	   - sdioCallbackSelectInterruptSdCard: the corresponding callback function is executed whenever the SD card interrupts the handheld.
 *	   The handheld enables the SDIO interrupt whenever a valid sdioCallbackSelectInterruptSdCard callback is set.
 *	   The callback function needs to reset the interrupt source to prevent the interrupt callback from being called again.
 *
 *	   - sdioCallbackSelectSleep and sdioCallbackSelectAwake: the corresponding callback functions are executed whenever the handheld
 *	   is about to be put to sleep or just after it wakes. These callback functions are always called with interrupts disabled, and should be as fast as possible.
 *
 *	   - sdioCallbackSelectPowerOn and sdioCallbackSelectPowerOff: the corresponding callback functions are executed when the SDIO card power
 *  	   is turned on or is about to be turned off. Never call SDIOSetPower while processing these functions in order to turn an SDIO card function on or off.
 *
 *	   - sdioCallbackSelectReset: the corresponding callback function is executed whenever SDIOSetPower is called with the powerP structure's
 *	   powerOnCard field set to sdioCardWaitSdio. sdioCardWaitSdio is typically used after the SDIO section has been reset by setting the
 *         RES (I/O Card Reset) bit in the CCCR (Card Common Control Registers).
 *
 *	   - sdioCallbackSelectBitMode: the corresponding callback function is executed whenever the bus width is successfully changed with SDIOSetBitMode.
 *   	   Note that in version 1.0 of the SDIO slot driver, this callback is never executed because the bus is always one bit wide.
 *
 *	   When a situation arises that causes one of the above callback functions to be called, the corresponding callback for the SD Memory card slot
 *	   is generally the first one called, followed by the corresponding callback functions for SDIO functions 1 through 7. Because the SD Memory
 *	   card slot and each SDIO card function slot can have a separate callback function for each callback selector, each callback function can
 *         limit itself to dealing with a single selector and a single SDIO card function.
 *
 *	   Callback function information is automatically erased after a card is inserted or removed (before the card removal event).
 *	   Because of this, SDIOGetCallback can only be used when a card is in the SD slot. To detect card removal, use the notification manager
 *	   and register for sysNotifyCardRemovedEvent.
 * 	   SDIOGetCallback can safely be called from within an interrupt service routine.
 *
 *
 *  Warning:
 *    If you use any of these callbacks, you may not have control of the user interface or access to your variables.
 *    Always Set/Use the userDataP to access your user variables! Also, remember to lock memory for
 *    your callback routines and variables.
 */
inline Err SDIOGetCallback( UInt16 slotLibRefNum, SDIOCallbackType *callBackP );
inline Err SDIOGetCallback( UInt16 slotLibRefNum, SDIOCallbackType *callBackP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetCallback, callBackP, NULL ) );
}
/**
 *
 *  @brief SDIOSetCallback - Set pointers to an SDIO card function's callback routine and associated data.
 *
 *  @param slotLibRefNum:	IN: SlotDriver library reference number
 *  @param callBackP:		IN: Pointer to an SDIOCallbackType structure. Before calling this function, set each of
 *                              this structure's fields.
 *
 *  @retval  errNone              -- No error.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *           sysErrParamErr       -- callBackP is invalid, or one of its fields is invalid.
 *
 *  @note  You can assign a separate callback function to each callback selector for each card function slot as well as the SD Memory card slot.
 *	   The following list of callback selectors details those situations in which a particular callback function is called:
 *
 *	   - sdioCallbackSelectInterruptSdCard: the corresponding callback function is executed whenever the SD card interrupts the handheld.
 *	   The handheld enables the SDIO interrupt whenever a valid sdioCallbackSelectInterruptSdCard callback is set.
 *	   The callback function needs to reset the interrupt source to prevent the interrupt callback from being called again.
 *
 *	   - sdioCallbackSelectSleep and sdioCallbackSelectAwake: the corresponding callback functions are executed whenever the handheld is
 *	   about to be put to sleep or just after it wakes. These callback functions are always called with interrupts disabled, and should be as fast as possible.
 *	
 *	   - sdioCallbackSelectPowerOn and sdioCallbackSelectPowerOff: the corresponding callback functions are executed when the SDIO card power
 *	   is turned on or is about to be turned off. Never call SDIOSetPower while processing these functions in order to turn an SDIO card function on or off.
 *
 *	   - sdioCallbackSelectReset: the corresponding callback function is executed whenever SDIOSetPower is called with the powerP structure's
 *	   powerOnCard field set to sdioCardWaitSdio. sdioCardWaitSdio is typically used after the SDIO section has been reset by setting the
 *	   RES (1/O Card Reset) bit in the CCCR (Card Common Control Registers).
 *
 *	   - sdioCallbackSelectBitMode: the corresponding callback function is executed whenever the bus width is successfully changed with
 *	   SDIOSetBitMode. Note that in version 1.0 of the SDIO slot driver, this callback is never executed because the bus is always one bit wide.
 *
 *	   When a situation arises that causes one of the above callback functions to be called, the corresponding callback for the SD Memory
 *	   card slot is generally the first one called, followed by the corresponding callback functions for SDIO functions 1 through 7.
 *	   Because the SD Memory card slot and each SDIO card function slot can have a separate callback function for each callback selector,
 *	   each callback function can limit itself to dealing with a single selector and a single SDIO card function.
 *
 * 	   If you use any of these callbacks, you may not have control of the user interface or access to your variables. Make any necessary data
 *	   available to your callback function through the use of userDataP. Be sure to lock memory for your callback functions and variables.
 *
 *	   Callback functions must be interrupt-safe: they should only call interrupt-safe functions. Your callback functions can be called from
 *	   within an interrupt service routine, and interrupts can occur at any time. The interrupt can even generate a wakeup event if the handheld
 *	   is asleep and power to the card is still on. As with an interrupt service routine, your callback functions can only call a limited set
 *	   of system functions and must execute quickly. Your callback functions do have access to any slot driver functions accessible through the
 *	   slotCustomControl function.
 *	
 *	   Callback function information is automatically erased after a card is inserted or removed (before the card removal event).
 *	   Because of this, SDIOSetCallback can only be used when a card is in the SD slot. To detect card removal, use the notification manager
 *	   and register for sysNotifyCardRemovedEvent.
 *	
 * 	   SDIOSetCallback can safely be called from within an interrupt service routine.
 *
 *
 *     Warning:
 *     If you use any of these callbacks, you may not have control of the user interface or access to your variables.
 *     Always Set/Use the userDataP to access your user variables! Also, remember to lock memory for
 *     your callback routines and variables.
 */
inline Err SDIOSetCallback( UInt16 slotLibRefNum, SDIOCallbackType *callBackP );
inline Err SDIOSetCallback( UInt16 slotLibRefNum, SDIOCallbackType *callBackP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioSetCallback, callBackP, NULL ) );
}

/**
 *
 *  @brief SDIOGetAutoRun - Provide a description of the SD/MMC memory card or SDIO card that is currently inserted.
 * 	       This fills in information for a Auto Run  of a RAM based Application.
 *         Generally this is not for the Function Driver (since it is already executing).
 *
 *  @param slotLibRefNum: IN:     SlotDriver library reference number
 *  @param AutoRunP:	  IN/OUT: Pointer to an SDIOAutoRunInfoType structure, which describes the SD/MMC
 *                                memory card or SDIO card that is currently inserted. Before calling SDIOGetAutoRun
 *                                set this structure's sdioSlotNum field to the card function you are using (a value of
 *                                1-7 indicates one of the SDIO functions, while a value of 0 indicates the SD memory
 *                                card slot driver).
 *
 *  @retval  errNone                    -- No error.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *           expErrUnsupportedOperation -- The SDIO card does not support the specified SDIO card function.
 *           sysErrParamErr             -- autoRunP is invalid, or one of its fields is invalid.
 *
 *
 *  @note  Information provided by this function is only maintained while a card is in the SD slot, and is erased when the card is removed.
 *	       Because of this, this function requires that an SDIO card be present in the slot.
 *	
 *	       This structure is provided to SDIO device drivers as part of the sysNotifyDriverSearch notification that is broadcast by the
 *         SDIO slot driver in an attempt to locate a device driver for an SDIO card.
 *
 *	       This function can safely be called from within an interrupt service routine.
 *
 */
inline Err SDIOGetAutoRun( UInt16 slotLibRefNum, SDIOAutoRunInfoType *AutoRunP );
inline Err SDIOGetAutoRun( UInt16 slotLibRefNum, SDIOAutoRunInfoType *AutoRunP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioGetAutoRun, AutoRunP, NULL ) );
}

/**
 *
 *  @brief  SDIORWDirect - Read or write a single byte to any I/O function, including the common
 *                         I/O area (CIA), at any address using CMD52 (IO_RW_DIRECT).
 *
 *  @param slotLibRefNum: IN:     Slot Driver library reference number
 *  @param directP:		  IN/OUT: Pointer to an SDIORWDirectType structure which describes the read or write operation.
 *
 *  @retval  errNone                    -- No error.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrCardBadSector        -- The SDIO memory could not be read or written.
 *           expErrUnsupportedOperation -- The SDIO card does not support the specified SDIO card function.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *           sysErrParamErr             -- directP is invalid, or one of its fields is invalid.
 *
 *  @note  This function is commonly used to initialize registers or monitor status values for I/O functions.
 *    	   This function requires that an SDIO card be present in the slot. The card will be turned on and accessed.
 *	       See the SDIO specification for the SDIO registers that can be read or written.
 *
 *	       The write protect tab on the SD card is ignored by the SDIO slot driver. Issuing a write request with this function
 *	       always causes the write command to be sent to the card.
 *
 * 	       This function can safely be called from within an interrupt service routine.
 *
 */
inline Err SDIORWDirect( UInt16 slotLibRefNum, SDIORWDirectType *directP );
inline Err SDIORWDirect( UInt16 slotLibRefNum, SDIORWDirectType *directP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioRWDirect, directP, NULL ) );
}
/**
 *
 *  @brief  SDIORWExtendedByte - Read or write multiple bytes to any I/O function, including the
 *          common I/O area (CIA), at any address using the byte mode of CMD53 (IO_RW_EXTENDED).
 *
 *  @param slotLibRefNum: IN:     SlotDriver library reference number
 *  @param extendedByteP: IN/OUT: Pointer to an SDIORWExtendedByteType structure which describes the read or write operation.
 *
 *  @retval  errNone                    -- No error.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrCardBadSector        -- The SDIO memory could not be read or written.
 *           expErrUnsupportedOperation -- The SDIO card does not support the specified SDIO card function.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *           sysErrParamErr             -- extendedByteP is invalid, or one of its fields is invalid.
 *
 *  @note  This function is commonly used to initialize registers or monitor status values for I/O functions.
 *    	   This function requires that an SDIO card be present in the slot. The card will be turned on and accessed.
 *	       See the SDIO specification for the SDIO registers that can be read or written.
 *
 *	       The write protect tab on the SD card is ignored by the SDIO slot driver. Issuing a write
 *         request with this function always causes the write command to be sent to the card.
 *
 * 	       This function can safely be called from within an interrupt service routine.
 */
inline Err SDIORWExtendedByte( UInt16 slotLibRefNum, SDIORWExtendedByteType *extendedByteP );
inline Err SDIORWExtendedByte( UInt16 slotLibRefNum, SDIORWExtendedByteType *extendedByteP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioRWExtendedByte, extendedByteP, NULL ) );
}
/**
 *
 *  @brief  SDIORWExtendedBlock - Read or write multiple blocks of a specified size to any I/O function,
 *          including the common I/O area (CIA), at any address using the optional block mode of
 *          CMD53 (IO_RW_EXTENDED).
 *
 *  @param slotLibRefNum:	IN:     SlotDriver library reference number
 *  @param extendedBlockP:	IN/OUT: Pointer to an SDIORWExtendedBIockType structure which describes
 *                                  the read or write operation.
 *
 *  @retval  errNone                     -- No error.
 *           expErrCardNotPresent        -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrCardBadSector         -- The SDIO memory could not be read or written.
 *           expErrUnsupportedOperation  -- The SDIO card does not support the specified SDIO card function.
 *           expErrUnimplemented         -- The specified slot driver does not support SDIO.
 *           sysErrParamErr              -- extendedBlockP is invalid, or one of its fields is invalid.
 *
 *  @note A given SDIO card may or may not support SDIORWExtendedBlock; the SDIO specification doesn't require it.
 *	      Verify that the card supports block operations by checking the SMB (Card Supports MBIO) bit in the CCCR (Card Common Control Registers).
 *	      This SDIO slot driver does not support the “infinite” mode (which is normally indicated by setting the block count to zero).
 *	      See the SDIO specification for the SDIO registers that can be read or written.
 *
 *	      This function is commonly used to initialize registers or monitor status values for I/O functions. This function requires
 *	      that an SDIO card be present in the slot. The card will be turned on and accessed.
 *
 *	      The write protect tab on the SD card is ignored by the SDIO slot driver. Issuing a write request with this function always
 *	      causes the write command to be sent to the card.
 *
 * 	      This function can safely be called from within an interrupt service routine.
 */
inline Err SDIORWExtendedBlock( UInt16 slotLibRefNum, SDIORWExtendedBlockType *extendedBlockP );
inline Err SDIORWExtendedBlock( UInt16 slotLibRefNum, SDIORWExtendedBlockType *extendedBlockP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioRWExtendedBlock, extendedBlockP, NULL ) );
}

/**
 *
 *  @brief  SDIOTupleWalk - Search an SDIO card function's Card Information Structure (CIS) for a
 *          particular data block (tuple) and return the contents of the data block.
 *
 *  @param slotLibRefNum: IN: SlotDriver library reference number
 *  @param tupleP:		  IN: Pointer to an SDIOTupleType structure which identifies the tuple to be located and
 *                            indicates where the results should be placed.
 *
 *  @retval  errNone                    -- No error.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnsupportedOperation -- The SDIO card does not support the specified SDIO card function.
 *           expErrCardBadSector        -- The SDIO card's function memory could not be read or the requested tuple was not found
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *           sysErrParamErr             -- tupleP is invalid, or one of its fields is invalid.
 *
 *  @note  This function requires that an SDIO card be present in the slot. The card will be turned on and accessed.
 *	
 *         This function can safely be called from within an interrupt service routine.
 */
inline Err SDIOTupleWalk( UInt16 slotLibRefNum, SDIOTupleType *tupleP );
inline Err SDIOTupleWalk( UInt16 slotLibRefNum, SDIOTupleType *tupleP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioTupleWalk, tupleP, NULL ) );
}
/**
 *
 *  @brief  SDIOAccessDelay - Change the SDIO card access timeout for reads and writes using IO_RW_DIRECT and IO_RW_EXTENDED from the 1 second default.
 *          If a card needs a slower (worst case) delay use this.
 *          Note: basic commands like read CCRCR, FBR's, and tuples MUST be readable within 1 second!
 *          If a file system is available, it will be mounted (accesssed) with this 1 second limitation too!
 *          Drivers only have access to this after their driver is started!
 *
 *  @param slotLibRefNum:	IN:  SlotDriver library reference number
 *  @param delayMSP:		IN:  Pointer to delay in Milliseconds, mimimum is 1,000 (1 second)
 *
 *  @retval  errNone              -- No error.
 *           expErrCardNotPresent -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnimplemented  -- The specified slot driver does not support SDIO.
 *           sysErrParamErr       -- delayMSP is NULL.
 *
 *  @note  The default timeout for any SDIO card is 1 second. Use this function if your card needs a longer worst-case delay.
 *	   Note that this timeout only affects reads and writes using IO_RW_DIRECT and IO_RW_EXTENDED; basic commands such as
 *	   those that read the CCCRs, FBRs, and tuples must execute within 1 second. If a file system is available, it is mounted with this 1 second limitation.
 *
 *	   The SDIO slot driver initializes and identifies the card and mounts files before a custom slot driver gains access to the card.
 *	   The SDIO slot driver assumes that the card functions properly with the 1-second specification time limit.
 *
 *	   This function requires that an SDIO card be present in the slot.
 *
 *	   This function can safely be called from within an interrupt service routine.
 *
 */
inline Err SDIOAccessDelay( UInt16 slotLibRefNum, SDIOAccessDelayType *delayMSP );
inline Err SDIOAccessDelay( UInt16 slotLibRefNum, SDIOAccessDelayType *delayMSP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioAccessDelay, delayMSP, NULL ) );
}

/**
 *
 *  @brief  SDIODebugOptions - Enable or disable the sending of debug messages to the serial or USB port.
 *	        FOR DEBUG ROMS and DEBUG RAM patches only!
 *
 *  @param slotLibRefNum: IN:     SlotDriver library reference number
 *  @param debugOptionsP: IN/OUT: Pointer to SDIODebugOptionType structure which specifies which messages should be sent.
 *
 *  @retval  errNone                    -- No error.
 *           expErrUnsupportedOperation -- This is not a debug ROM or debug RAM patch. No debug features are available.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *
 *  @note  If you use this function to enable debugging, be sure that the handheld is in a cradle and a debugger is running
 *	   on your desktop computer. CodeWarrior can be used, but it adds extra carriage-return/linefeed pairs to the messages.
 *
 *	   WARNING: If this option is activated, and the device is not in a cradle, all debug messages will be routed to the USB cradle
 *	   by default. However, since the device is not in a USB cradle, the software will "lock", forever trying to open a
 *	   non-existent USB port. To recover, either press reset or start a USB debugger on your PC or Mac and then connect the handheld to the cradle.
 *
 *	   To deactivate debugging, perform a soft reset on the handheld or call SDIODebugOptions and specify sdioDebugOptionTraceNone.
 *
 *	   This is not a real time trace: the serial port slows down the card's response. Use a logic analyzer for real time tracing.
 *
 * 	   This function can safely be called from within an interrupt service routine.
 *
 */
inline Err SDIODebugOptions( UInt16 slotLibRefNum, SDIODebugOptionType *debugOptionsP );
inline Err SDIODebugOptions( UInt16 slotLibRefNum, SDIODebugOptionType *debugOptionsP )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioDebugOptions, debugOptionsP, NULL ) );
}

/**
 *
 *  @brief  SDIODisableHandheldInterrupt - Disables the interrupt on the handheld. This function does not turn off interrupts on the card.
 *			This call disables the interrupt on the handheld side
 *			(This does not turn off any interrupt on the card). The disable interrupt
 *			is implemented as an incrementing counter (like locking memory), thus
 *			making it re-entrant. However, for every call of SDIODisableHandheldInterrupt()
 *			there must be an equal number (or more) of SDIOEnableHandheldInterrupt() to
 *			actually re-enable interrupts.
 *
 *  @param slotLibRefNum:	IN:  SlotDriver library reference number
 *
 *  @retval  errNone                    -- No error.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnsupportedOperation -- SDIODisableHandheldInterrupt has been called in excess of 65,535 times.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *
 *  @note  This function is implemented as an incrementing counter, making it re-entrant. For every call to SDIODisableHandheldInterrupt
 *	   there must be an equal number (or more) of calls to SDIOEnableHandheldInterrupt in order to re-enable interrupts.
 *
 *	   This function requires that an SDIO card be present in the slot.
 *
 *	   This function can safely be called from within an interrupt service routine.
 */
inline Err SDIODisableHandheldInterrupt( UInt16 slotLibRefNum );
inline Err SDIODisableHandheldInterrupt( UInt16 slotLibRefNum )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioDisableMasterInterrupt, NULL, NULL ) );
}

/**
 *
 *  @brief  SDIOEnableHandheldInterrupt - Enables the interrupt on the handheld. This function does not affect interrupts on the card.
 *			This call enables the interrupt on the handheld side
 *			For every call of SDIODisableHandheldInterrupt()
 *			there must be an equal number (or more) of SDIOEnableHandheldInterrupt() to
 *			actually re-enable interrupts. By default, when the card is inserted,
 *			this interrupt is enabled by this routine, but disabled internally until
 *			an interrupt callback is set.
 *
 *  @param slotLibRefNum:	IN:  SlotDriver library reference number
 *
 *  @retval  errNone                    -- No error.
 *           expErrCardNotPresent       -- There isn't a card in the slot associated with the specified slot driver.
 *           expErrUnsupportedOperation -- Interrupts are already enabled.
 *           expErrUnimplemented        -- The specified slot driver does not support SDIO.
 *
 *  @note  This function is implemented as a decrementing counter, making it re-entrant. For every call to SDIODisableHandheldInterrupt
 *	   there must be an equal number (or more) of calls to SDIOEnableHandheldInterrupt in order to re-enable interrupts.
 *
 *	   By default, when the card is inserted interrupts on the handheld are enabled by this function, but are disabled internally
 *	   until an interrupt callback is set with SDIOSetCallback. Note that in order to receive the SDIO interrupt, power to the card
 *	   must be on, even if the handheld is asleep.
 *
 *	   This function requires that an SDIO card be present in the slot.
 *	
 *	   This function can safely be called from within an interrupt service routine.
 */
inline Err SDIOEnableHandheldInterrupt( UInt16 slotLibRefNum );
inline Err SDIOEnableHandheldInterrupt( UInt16 slotLibRefNum )
{
    return( SlotCustomControl( slotLibRefNum, sysFileApiCreatorSDIO, sdioEnableMasterInterrupt, NULL, NULL ) );
}

#endif //__SDIO_h_
