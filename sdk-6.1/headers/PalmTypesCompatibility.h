/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmTypesCompatibility.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		This file contains deprecated types from PalmTypes.h 
 *
 *****************************************************************************/

#ifndef _PALM_OS__PALM_TYPES_COMPATIBILITY_H
#define _PALM_OS__PALM_TYPES_COMPATIBILITY_H

#include <sys/types.h>
#include <stdint.h>

#ifdef _PALM_OS_SYS_TYPES_H
// In some cases we include the Windows sys/types.h
typedef status_t				Err;
#else
typedef int32_t					Err;
#endif

typedef char					Char;

typedef	int8_t					int8;
typedef uint8_t					uint8;

typedef	int16_t					int16;
typedef uint16_t				uint16;

typedef	int32_t					int32;
typedef uint32_t				uint32;

typedef	int64_t					int64;
typedef uint64_t				uint64;

typedef int8_t					Int8;
typedef int16_t					Int16;	
typedef int32_t					Int32;
typedef int64_t					Int64;

typedef uint8_t					UInt8;
typedef uint16_t				UInt16;
typedef uint32_t				UInt32;
typedef uint64_t				UInt64;

// WChar is now deprecated. Use wchar32_t, or wchar16_t if you need
// an explicit 16-bit value for UTF-16/UCS-2 Unicode support.
typedef uint16_t				WChar;

#ifndef __cplusplus
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#endif

#ifndef OffsetOf
#define OffsetOf(type, member)	((uint32_t) &(((type *) 0)->member))
#endif

#endif
