/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IOS.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *	Header file for all IOS Drivers.
 *
 *****************************************************************************/

#ifndef _IOS_H_
#define _IOS_H_

#include <PalmTypes.h>
#include <CmnErrors.h>

//
// include posix defs
//

#include <stropts.h>
#include <fcntl.h>
#include <poll.h>

//
// Constants
//
#define MAX_FILDES           64  // Maximum number of file descriptors per process.


//
// IOS Error Codes
//

#define iosErrInvalidInstallationData (iosErrorClass | 1) // Invalid Installation Data
#define iosErrInvalidDeviceType		(iosErrorClass | 2)  // The specified device type is invalid for this operation
#define iosErrTooManyNamesPerDriver (iosErrorClass | 3)  // The number of names registered for that driver exceeded the maximum for that class of driver.
#define iosErrNameAlreadyRegistered (iosErrorClass | 4)  // That name has already been registered.
#define iosErrDriverNotFound        (iosErrorClass | 5)  // The driver prc could not be found.
#define iosErrDriverAlreadyInstalled (iosErrorClass | 6) // That driver prc has already been installed.
#define iosErrAuthFailed        	(iosErrorClass | 7)  // Authorization failure
#define iosErrBadMessage            (iosErrorClass | 8)  // Bad message
#define iosErrBadQueueID            (iosErrorClass | 9)  // Bad lwITC queue id
#define iosErrBadSemaphore          (iosErrorClass | 10)
#define iosErrUndefined             (iosErrorClass | 11)
#define iosErrDeviceInUse           (iosErrorClass | 12) // The device with that name is in use.
#define iosErrDeviceNotFound    	(iosErrorClass | 13) // No device with that name can be find
#define iosErrInvalidParam			(iosErrorClass | 14) // One or more of the parameters is invalid.
#define iosErrAccess             	(iosErrorClass | 15) //  Permission denied
#define iosErrIOError               (iosErrorClass | 16)
#define iosErrNoData                (iosErrorClass | 17)
#define iosErrNoFlag                (iosErrorClass | 18)
#define iosErrNoFileDescriptors     (iosErrorClass | 19) // Out of file descriptors
#define iosErrNoKeyVariable         (iosErrorClass | 20) // Out of key variables
#define iosErrBadFD            		(iosErrorClass | 21) //  Invalid fild descriptor
#define iosErrNotAllowed			(iosErrorClass | 22) //  Operation not allowed
#define iosErrNoProcessEntry        (iosErrorClass | 23)
#define iosErrNoQueueID             (iosErrorClass | 24)
#define iosErrNoDHQueueEntry        (iosErrorClass | 25)  // Out of space for messages to the driver.
#define iosErrOutOfRange			(iosErrorClass | 26) //  Value not in valid range
#define iosErrNoSessionEntry        (iosErrorClass | 27)
#define iosErrBadTimerID            (iosErrorClass | 28)
#define iosErrThreadInit			(iosErrorClass | 29)
#define iosErrNotOpened             (iosErrorClass | 30)
#define iosErrNotSupported          (iosErrorClass | 31)
#define iosErrQueueIsFull           (iosErrorClass | 32)
#define iosErrBadQueueName          (iosErrorClass | 33)
#define iosErrInvalidKey			(iosErrorClass | 34)
#define iosErrBadMessageFlag        (iosErrorClass | 35)
#define iosErrUnderlock				(iosErrorClass | 36)
#define iosErrNotEnoughSpace        (iosErrorClass | 37) //  Input Buffer too small for output data
#define iosErrNoMsg					(iosErrorClass | 38) //	 No message of desired type	
#define iosErrOpNotSupported		(iosErrorClass | 39) //  Operation not supported on endpoint
#define iosErrCanceled             	(iosErrorClass | 40) //  Operation Canceled
#define iosErrDevice       			(iosErrorClass | 41) //  Device I/O Error or no such minor num
#define iosErrPipe             		(iosErrorClass | 42) //  Write to pipe with no listener
#define iosErrTooBig				(iosErrorClass | 43) //  Object too big
#define iosErrProtocol         		(iosErrorClass | 44) //  Protocol Error
#define iosErrMessageSize           (iosErrorClass | 45) //  Msg length exceeds protocol limit
#define iosErrAgain            		(iosErrorClass | 46) //  Try again later
#define iosErrDeviceFull			(iosErrorClass | 47) //  No space left on device
#define iosErrAddressInUse          (iosErrorClass | 48) //  Address is in use
#define iosErrAddressNotAvail       (iosErrorClass | 49) //  Address not available
#define iosErrAFNotSupported        (iosErrorClass | 50) //  Address Format not supported
#define iosErrBadAddress            (iosErrorClass | 51) //  Address is invalid
#define iosErrBusy             		(iosErrorClass | 52) //  Device Busy
#define iosErrConnRefused           (iosErrorClass | 53) //  Connection refused
#define iosErrConnReset             (iosErrorClass | 54) //  Connection reset
#define iosErrHostUnreachable       (iosErrorClass | 55) //  Host is unreachable
#define iosErrIsConnected           (iosErrorClass | 56) //  Already connected
#define iosErrNetUnreachable        (iosErrorClass | 57) //  Network is unreachable
#define iosErrNoBufs             	(iosErrorClass | 58) //  No buffer space available
#define iosErrNoProtoOpt            (iosErrorClass | 59) //  Invalid protocol specifed for option
#define iosErrNotConnected          (iosErrorClass | 60) //  Not connected
#define iosErrNoProto       		(iosErrorClass | 61) //  Protocol not supported
#define iosErrSockNotSupp			(iosErrorClass | 62) //  Socked type not supported
#define iosErrNotSock				(iosErrorClass | 63) //  Socket operation on non-socket
#define iosErrNoLink				(iosErrorClass | 64) //  Link has been severed. 
#define iosErrRemovalAuthorized     (iosErrorClass | 65) //  Internal error code.
#define iosErrBadMemoryAddress		(iosErrorClass | 66)
#define iosErrConnectFailed			(iosErrorClass | 67) //	 Unable to connect


//
// Alleged type of the IOS keys
//

#define kAllegedTypeIOS (iosErrorClass | 0)


//
// Message opcodes
//

#define kMsgIOS          (iosErrorClass | 0)
#define kMsgIOSFastIoctl (iosErrorClass | 1)
#define kMsgIOSFcntl	 (iosErrorClass | 2)



//
// Structure Definitions
//

#include <sys/uio.h>	// import iovec type
	
//
// IOS Standard I/O Functions
//

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t IOSOpen (const char * iPath, int32_t iFlags, status_t * oErrno);
extern int32_t IOSIoctl (int32_t iFildes, int32_t iRequest, int32_t iArgP, status_t * oErrno);
extern int32_t IOSFastIoctl (int32_t iFildes, int32_t iRequest, int32_t iSendLen, MemPtr iSendP, int32_t iRecvLen, MemPtr iRecvP, status_t * oErrno);
extern int32_t IOSRead (int32_t iFildes, MemPtr iBuf, int32_t iNbytes, status_t * oErrno);
extern int32_t IOSWrite (int32_t iFildes, MemPtr iBuf, int32_t iNbytes, status_t * oErrno);
extern int32_t IOSReadv (int32_t iFildes, const struct iovec * iIovP, int32_t iIovcnt, status_t * oErrno);
extern int32_t IOSWritev (int32_t iFildes, const struct iovec * iIovP, int32_t iIovcnt, status_t * oErrno);
extern int32_t IOSGetmsg (int32_t iFildes, struct strbuf * iCtlptrP, struct strbuf * iDataptrP, int32_t * ioFlags,
						status_t * oErrno);
extern int32_t IOSGetpmsg (int32_t iFildes, struct strbuf * iCtlptrP, struct strbuf * iDataptrP, int32_t * ioBand,
						int32_t * ioFlags, status_t * oErrno);

extern status_t IOSClose (int32_t iFildes);
extern status_t IOSPutmsg (int32_t iFildes, const struct strbuf * iCtlptrP, const struct strbuf * iDataptrP,
						int32_t iFlags);
extern status_t IOSPutpmsg (int32_t iFildes, const struct strbuf * iCtlptrP, const struct strbuf * iDataptrP,
						int32_t iBand, int32_t iFlags);
extern status_t IOSFattach (int32_t iFildes, const char * iPath);
extern status_t IOSFdetach (const char * iPath);
extern status_t IOSPipe (int32_t * oFildes);
extern status_t IOSPoll (struct pollfd iFds[], int32_t iNfds, int32_t iTimeout, int32_t *oNfds);
extern int32_t IOSFcntl(int32_t iFildes, int32_t iRequest, int32_t * iArgP, status_t * oErrno);

#ifdef __cplusplus
}
#endif

#endif /* _IOS_H_ */
