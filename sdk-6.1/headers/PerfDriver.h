/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PerfDriver.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *       Public header for the Performance Manager Driver
 *
 *****************************************************************************/

#ifndef __PerfDriver_H__
#define __PerfDriver_H__

// Include elementary types
#include <PalmTypes.h>		// Basic types

// Other headers we depend on
#include <CmnErrors.h>

// Error Codes definition
#define perfErrNone  errNone
#define perfErrInvalidParams      (perfErrorClass | 1)
#define perfErrLimitReached       (perfErrorClass | 2)
#define perfErrNotImplemented     (perfErrorClass | 3)
#define perfErrBufferTooSmall     (perfErrorClass | 4)
#define perfErrDeniedPowerLow     (perfErrorClass | 5)

// Fast Ioctl codes
#define kGetCPUClockInfo          (perfErrorClass | 1)
#define kGetCPUClockRateArray     (perfErrorClass | 2)
#define kSetDefaultCPUClockRate   (perfErrorClass | 3)
#define kCreatePerfRequest        (perfErrorClass | 4)
#define kCancelPerfRequest        (perfErrorClass | 5)

// Constants and type declarations for kGetCPUClockInfo call
#define kCPUClockInfoVersion_0         0
#define kCurrentCPUClockInfoVersion    kCPUClockInfoVersion_0

// Version kCPUClockInfoVersion_0 of CPU clock info type
typedef struct PerfGenCPUClockInfoType
{
    uint32_t  minClock;       // minimum CPU clock (KHz)
    uint32_t  maxClock;       // maximum CPU clock (KHz)
    uint32_t  defClock;       // default CPU clock (KHz)
    uint32_t  curClock;       // current CPU clock (KHz)
    uint32_t  numClockModes;  // number of CPU clock modes
}   PerfGenCPUClockInfoType, * PerfGenCPUClockInfoPtr;

// Constants and types for kCreatePerfRequest call
#define kPerfClockValueMax    0xFFFFFFFF  // Maximize CPU speed 
#define kPerfClockValueDelta  0x80000000  // Delta

// Performance request reference number
typedef uint32_t PerfRefNumType;

// Constants and types for kCancelPerfRequest call
#define kPerfRequestAny       0x00000000  // Represents any request

// Is used to return result of some Fast Ioctl calls
typedef struct PerfResultType
{   
    uint32_t     clockVal ;  // new CPU closk speed after request beeing executed
    uint32_t     extraData;  // request specific data.
                           // kCreatePerfRequest     - request reference number
                           // kCancelPerfRequest      - number of canceled requests 
                           // kSetDefaultCPUClockRate - unused
}   PerfResultType, * PerfResultPtr;


#endif // __PerfDriver_H__
