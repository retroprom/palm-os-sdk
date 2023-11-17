/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialLinkMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Source for Serial Link Routines on Pilot
 *
 *****************************************************************************/

#ifndef __SERIAL_LINK_H
#define __SERIAL_LINK_H



// Pilot common definitions
#include <PalmTypes.h>
//#include <ErrorBase.h> 
#include <CmnErrors.h> //slkErrorClass


//*************************************************************************
//   Pre-defined, fixxed  Socket ID's
//*************************************************************************
#define		slkSocketDebugger			0			// Debugger Socket
#define		slkSocketConsole			1			// Console Socket
#define		slkSocketRemoteUI			2			// Remote UI Socket
#define		slkSocketDLP				3			// Desktop Link Socket
#define		slkSocketADM6			4			// ARM Debug Manager
#define		slkSocketFirstDynamic	5			// first dynamic socket ID


//*************************************************************************
//  Packet Types
//*************************************************************************
#define		slkPktTypeSystem			0			// System packets
#define		slkPktTypeUnused1			1			// used to be: Connection Manager packets
#define		slkPktTypePAD				2			// PAD Protocol packets
#define		slkPktTypeLoopBackTest	3			// Loop-back test packets



//*************************************************************************
//
// Packet structure:
//		header
//		body (0-dbgMaxPacketBodyLength bytes of data)
//		footer
//
//*************************************************************************

//----------------------------------------------------------------------
// packet header
// Fields marked with -> must be filled in by caller
// Fields marked with X  will be filled in by SlkSendPacket.
//----------------------------------------------------------------------

typedef	uint8_t	SlkPktHeaderChecksum;

typedef struct SlkPktHeaderType {
	uint16_t					signature1;				// X  first 2 bytes of signature
	uint8_t						signature2;				// X  3 and final byte of signature
	uint8_t						dest;						// -> destination socket Id
	uint8_t						src;						// -> src socket Id
	uint8_t						type;						// -> packet type
	uint16_t					bodySize;				// X  size of body
	uint8_t						transId;					// -> transaction Id
															//    if 0 specified, it will be replaced 
	SlkPktHeaderChecksum	checksum;				// X  check sum of header
	} SlkPktHeaderType;

typedef SlkPktHeaderType*	SlkPktHeaderPtr;

#define	slkPktHeaderSignature1	0xBEEF
#define	slkPktHeaderSignature2	0xED

#define	slkPktHeaderSigFirst		0xBE			// First byte
#define	slkPktHeaderSigSecond	0xEF			// second byte
#define	slkPktHeaderSigThird		0xED			// third byte

//----------------------------------------------------------------------
// packet footer
//----------------------------------------------------------------------
typedef struct SlkPktFooterType {
	uint16_t		crc16;				// header and body crc
	} SlkPktFooterType;

typedef SlkPktFooterType*	SlkPktFooterPtr;


//*************************************************************************
//
// Write Data Structure passed to SlkSendPacket. This structure 
//  Tells SlkSendPacket where each of the chunks that comprise the body are
//  and the size of each. SlkSendPacket accepts a pointer to an array
//  of SlkWriteDataTypes, the last one has a size field of 0.
//
//*************************************************************************
typedef struct SlkWriteDataType {
	uint16_t	size;					// last one has size of 0
	uint16_t	padding;				// alignment padding
	void*	dataP;					// pointer to data
	} SlkWriteDataType;
typedef SlkWriteDataType*	SlkWriteDataPtr;




//*************************************************************************
//
// CPU-dependent macros for getting/setting values from/to packets
//
//*************************************************************************

//--------------------------------------------------------------------
// macros to get packet values
//--------------------------------------------------------------------

#define	slkGetPacketByteVal(srcP)	(*(uint8_t *)(srcP))


#if (CPU_ENDIAN == CPU_ENDIAN_LITTLE)
#define	slkGetPacketWordVal(srcP)								\
	(	(uint16_t)															\
		(																	\
		((uint16_t)((uint8_t *)(srcP))[0] << 8) |					\
		((uint16_t)((uint8_t *)(srcP))[1])							\
		)																	\
	)
#else
#define	slkGetPacketWordVal(srcP)								\
	( *((uint16_t *)(srcP)) )
#endif	//  (CPU_ENDIAN == CPU_ENDIAN_LITTLE)


#if (CPU_ENDIAN == CPU_ENDIAN_LITTLE)
#define	slkGetPacketDWordVal(srcP)								\
	(	(uint32_t)															\
		(																	\
		((uint32_t)((uint8_t *)(srcP))[0] << 24) |					\
		((uint32_t)((uint8_t *)(srcP))[1] << 16) |					\
		((uint32_t)((uint8_t *)(srcP))[2] << 8) |					\
		((uint32_t)((uint8_t *)(srcP))[3])							\
		)																	\
	)
#else
#define	slkGetPacketDWordVal(srcP)								\
	( *((uint32_t *)(srcP)) )
#endif	//  (CPU_ENDIAN == CPU_ENDIAN_LITTLE)


#define	slkGetPacketSignature1(sigP)							\
	slkGetPacketWordVal(sigP)

#define	slkGetPacketSignature2(sigP)							\
	slkGetPacketByteVal(sigP)


#define	slkGetPacketDest(addressP)								\
	slkGetPacketByteVal(addressP)
	
#define	slkGetPacketSrc(addressP)								\
	slkGetPacketByteVal(addressP)
	
#define	slkGetPacketType(commandP)								\
	slkGetPacketByteVal(commandP)
	

#define	slkGetPacketBodySize(lengthP)							\
	slkGetPacketWordVal(lengthP)

#define	slkGetPacketTransId(transIDP)							\
	slkGetPacketByteVal(transIDP)

#define	slkGetPacketHdrChecksum(checksumP)					\
	slkGetPacketByteVal(checksumP)


#define	slkGetPacketTotalChecksum(checksumP)				\
	slkGetPacketWordVal(checksumP)






//--------------------------------------------------------------------
// macros to set packet values
//--------------------------------------------------------------------


#define	slkSetPacketByteVal(srcByteVal, destP)					\
	( *(uint8_t *)(destP) = (uint8_t)(srcByteVal) )

#if (CPU_ENDIAN == CPU_ENDIAN_LITTLE)
#define	slkSetPacketWordVal(srcWordVal, destP)					\
																				\
	do {																		\
		uint16_t	___srcVal;												\
		uint8_t *	___srcValP;												\
																				\
		___srcVal = (uint16_t)(srcWordVal);							\
		___srcValP = (uint8_t *)(&___srcVal);							\
																				\
		((uint8_t *)(destP))[0] = ___srcValP[1];						\
		((uint8_t *)(destP))[1] = ___srcValP[0];						\
	} while( false )
#else
#define	slkSetPacketWordVal(srcWordVal, destP)					\
	( *((uint16_t *)(destP)) = (uint16_t)(srcWordVal) )
#endif	// (CPU_ENDIAN == CPU_ENDIAN_LITTLE)


#if (CPU_ENDIAN == CPU_ENDIAN_LITTLE)
#define	slkSetPacketDWordVal(srcDWordVal, destP)				\
	do {																		\
		uint32_t	___srcVal;												\
		uint8_t *	___srcValP;												\
																				\
		___srcVal = (uint32_t)(srcDWordVal);							\
		___srcValP = (uint8_t *)(&___srcVal);							\
																				\
		((uint8_t *)(destP))[0] = ___srcValP[3];						\
		((uint8_t *)(destP))[1] = ___srcValP[2];						\
		((uint8_t *)(destP))[2] = ___srcValP[1];						\
		((uint8_t *)(destP))[3] = ___srcValP[0];						\
	} while( false )
#else
#define	slkSetPacketDWordVal(srcDWordVal, destP)				\
	( *((uint32_t *)(destP)) = (uint32_t)(srcDWordVal) )
#endif	// (CPU_ENDIAN == CPU_ENDIAN_LITTLE)


#define slkSetPacketSignature1(magic, destP)						\
	slkSetPacketWordVal(magic, destP)

#define slkSetPacketSignature2(magic, destP)						\
	slkSetPacketByteVal(magic, destP)


#define slkSetPacketDest(dest, destP)								\
	slkSetPacketByteVal(dest, destP)

#define slkSetPacketSrc(src, destP)									\
	slkSetPacketByteVal(src, destP)


#define slkSetPacketType(type, destP)								\
	slkSetPacketByteVal(type, destP)


#define slkSetPacketBodySize(numBytes, destP)					\
	slkSetPacketWordVal(numBytes, destP)


#define slkSetPacketTransId(transID, destP)						\
	slkSetPacketByteVal(transID, destP)

#define slkSetPacketHdrChecksum(checksum, destP)				\
	slkSetPacketByteVal(checksum, destP)

#define slkSetPacketTotalChecksum(checksum, destP)				\
	slkSetPacketWordVal(checksum, destP)






/*******************************************************************
 * Serial Link Manager Errors
 * the constant slkErrorClass is defined in SystemMgr.h
 *******************************************************************/
#define	slkErrChecksum				(slkErrorClass | 1)
#define	slkErrFormat				(slkErrorClass | 2)
#define	slkErrBuffer				(slkErrorClass | 3)
#define	slkErrTimeOut				(slkErrorClass | 4)
#define	slkErrHandle				(slkErrorClass | 5)
#define	slkErrBodyLimit			(slkErrorClass | 6)
#define	slkErrTransId				(slkErrorClass | 7)
#define	slkErrResponse				(slkErrorClass | 8)
#define	slkErrNoDefaultProc		(slkErrorClass | 9)
#define	slkErrWrongPacketType	(slkErrorClass | 10)
#define 	slkErrBadParam				(slkErrorClass | 11)
#define 	slkErrAlreadyOpen			(slkErrorClass | 12)
#define	slkErrOutOfSockets		(slkErrorClass | 13)
#define	slkErrSocketNotOpen		(slkErrorClass | 14)
#define	slkErrWrongDestSocket	(slkErrorClass | 15)
#define	slkErrWrongPktType		(slkErrorClass | 16)
#define	slkErrBusy					(slkErrorClass | 17)	// called while sending a packet
																		// only returned on single-threaded
																		// emulation implementations 
#define	slkErrNotOpen				(slkErrorClass | 18)



/*******************************************************************
 * Type definition for a Serial Link Socket Listener
 *
 *******************************************************************/
typedef	void (*SlkSocketListenerProcPtr)
			(SlkPktHeaderPtr headerP, void *bodyP);
			
typedef struct SlkSocketListenType {
	SlkSocketListenerProcPtr 	listenerP;
	SlkPktHeaderPtr				headerBufferP;		// App allocated buffer for header
	void*								bodyBufferP;		// App allocated buffer for body
	uint32_t							bodyBufferSize;
	} SlkSocketListenType;
typedef SlkSocketListenType*	SlkSocketListenPtr;



/*******************************************************************
 * Prototypes
 *******************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------
// Initializes the Serial Link Manager
//-------------------------------------------------------------------
status_t			SlkOpen(void);

//-------------------------------------------------------------------
// Close down the Serial Link Manager
//-------------------------------------------------------------------
status_t			SlkClose(void);

//-------------------------------------------------------------------
// Open up another Serial Link socket. The caller must have already
//  opened the comm library and set it to the right settings.
//-------------------------------------------------------------------

status_t			SlkOpenSocket(uint16_t portID, uint16_t *socketP, Boolean staticSocket);

//-------------------------------------------------------------------
// Close up a Serial Link socket. 
//  Warning: This routine is assymetrical with SlkOpenSocket because it
//   WILL CLOSE the library for the caller (unless the refNum is the
//   refNum of the debugger comm library).
//-------------------------------------------------------------------
status_t			SlkCloseSocket(uint16_t socket);

//-------------------------------------------------------------------
// Get the library refNum for a particular Socket
//-------------------------------------------------------------------

status_t			SlkSocketPortID(uint16_t socket, uint16_t *portIDP);

#define SlkSocketRefNum SlkSocketPortID

			
//-------------------------------------------------------------------
// Set the in-packet timeout for a socket
//-------------------------------------------------------------------
status_t			SlkSocketSetTimeout(uint16_t socket, int32_t timeout);

//-------------------------------------------------------------------
// Flush a Socket
//-------------------------------------------------------------------
status_t			SlkFlushSocket(uint16_t socket, int32_t timeout);

//-------------------------------------------------------------------
// Set up a Socket Listener
//-------------------------------------------------------------------
status_t			SlkSetSocketListener(uint16_t socket,  SlkSocketListenPtr socketP);

//-------------------------------------------------------------------
// Sends a packet's header, body, footer.  Stuffs the header's
// magic number and checksum fields.  Expects all other
// header fields to be filled in by caller.
// errors returned: dseHandle, dseLine, dseIO, dseParam, dseBodyLimit,
//					dseOther
//-------------------------------------------------------------------
status_t 		SlkSendPacket(SlkPktHeaderPtr headerP, SlkWriteDataPtr writeList);

//-------------------------------------------------------------------
// Receives and validates an entire packet.
// errors returned: dseHandle, dseParam, dseLine, dseIO, dseFormat,
//					dseChecksum, dseBuffer, dseBodyLimit, dseTimeOut,
//					dseOther
//-------------------------------------------------------------------
status_t			SlkReceivePacket( uint16_t socket, Boolean andOtherSockets, 
						SlkPktHeaderPtr headerP, void *bodyP,  uint16_t bodySize,  
						int32_t timeout);

#ifdef __cplusplus
}
#endif
	
	
#endif	//__SERIAL_LINK_H
