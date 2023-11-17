/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: types.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		This file contains types that STREAMS expects to be in sys/types.h
 *		that are not part of the standard and therefore not in our copy of
 *		sys/types.h
 *
 *		These types are deprecated and should not be used by developers.  
 *		Please use the C99 types defined in sys/types.h.
 *
 *****************************************************************************/

#ifndef _PALMSOURCE_STREAMS_TYPES_H
#define _PALMSOURCE_STREAMS_TYPES_H

#include <sys/types.h>
#include <stdint.h>

#ifndef _HIDE_OBSOLETE_TYPES_

typedef unsigned char		uchar;

typedef uchar				u8;
typedef ushort				u16;
typedef uint				u32;
typedef uint64_t			u64;

typedef signed char			i8;
typedef signed short		i16;
typedef signed int			i32;
typedef int64_t				i64;

typedef unsigned int		boolean;

typedef int					fd_t;

#endif /* _HIDE_OBSOLETE_TYPES_ */

#endif
