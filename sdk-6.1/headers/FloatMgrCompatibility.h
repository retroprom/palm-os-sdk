/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: FloatMgrCompatibility.h
 *
 * Release: Palm OS 6.0.1
 *
 *****************************************************************************/

#ifndef __FLOATMGRCOMPATIBILITY__
#define __FLOATMGRCOMPATIBILITY__
/*  The intent of this header is to map the old palmOS floating point
	environment interface to that of the 1999 ANSI C standard.  This is
	supported by the AXD toolsuite.  This header should and can be generalized to
	support any toolsuite that complies with section xxx of the 1999 ANSI C standard.
	Every compiler emits a predefined macro that distinguishes itself from others.
	For instance, AXD uses "__arm".  add the predefined macro for the appropriate
	toolset to floatmgr.h to enable the toolset to use this.  Note this header is depracated
	and may go away in future releases.  DO NOT include this directly.
*/
#include <stdlib.h> /* for strtod */
#include <stdio.h>  /* for sprintf */
#include <PalmTypes.h>


#ifdef _MSC_VER
#define FE_INVALID            0x81
#define FE_DENORMAL           0x82
#define FE_ZERODIVIDE         0x83
#define FE_OVERFLOW           0x84
#define FE_UNDERFLOW          0x85
#define FE_INEXACT            0x86
/* different rounding modes not supported in OS 6 */
#define FE_TONEAREST
#define FE_TOWARDZERO
#define FE_UPWARD
#define FE_DOWNWARD
#elif defined(__ARMCC_VERSION)
#include <fenv.h>
#else
/* do nothing */
#define FE_INVALID           
#define FE_DENORMAL          
#define FE_ZERODIVIDE        
#define FE_OVERFLOW          
#define FE_UNDERFLOW         
#define FE_INEXACT   
/* different rounding modes not supported in OS 6 */
#define FE_TONEAREST
#define FE_TOWARDZERO
#define FE_UPWARD
#define FE_DOWNWARD        
#endif
#include <errno.h>

/* 	 Note: FlpBase10Info returns the actual sign bit in *signP (1 if negative)
	 Note: FlpBase10Info reports that zero is "negative".
     A workaround is to check (*signP && *mantissaP) instead of just *signP.
*/
#if CPU_ENDIAN == CPU_ENDIAN_LITTLE
#define __HIX 1
#define __LOX 0
#else
#define __HIX 0
#define __LOX 1
#endif
typedef struct {
	int32_t high;
	int32_t low;
} _sfpe_64_bits;											// for internal use only

typedef _sfpe_64_bits sfpe_long_long;				// for internal use only
typedef _sfpe_64_bits sfpe_unsigned_long_long;	// for internal use only


/************************************************************************
 * Floating point manager types (public)
 ***********************************************************************/
typedef int32_t FlpFloat;
typedef _sfpe_64_bits FlpDouble;
typedef _sfpe_64_bits FlpLongDouble;

/*
* A double value comprises the fields:
*		0x80000000 0x00000000 -- sign bit (1 for negative)
*		0x7ff00000 0x00000000 -- exponent, biased by 0x3ff == 1023
*		0x000fffff 0xffffffff -- significand == the fraction after an implicit "1."
* So a double has the mathematical form:
*		(-1)^sign_bit * 2^(exponent - bias) * 1.significand
* What follows are some structures (and macros) useful for decomposing numbers.
*/

typedef struct {
	uint32_t	sign : 1;
	int32_t		exp  : 11;
	uint32_t	manH : 20;
	uint32_t	manL;
} FlpDoubleBits;						// for accessing specific fields

typedef union {
        double				d;			// for easy assignment of values
        FlpDouble			fd;		// for calling New Floating point manager routines
        uint32_t				ul[2];	// for accessing upper and lower longs
        FlpDoubleBits	fdb;		// for accessing specific fields
} FlpCompDouble;

typedef union {
        float				f;			// for easy assignment of values
        FlpFloat			ff;		// for calling New Floating point manager routines
        uint32_t				ul;		// for accessing bits of the float
} FlpCompFloat;

#define     FlpAToF(x) 		strtod(x,NULL);

//#define		FlpFToA(x, s) 	sprintf((char*)s, "%g",*(double*)&x)



#define FlpNegate(x)			(((FlpCompDouble *)&x)->ul[__HIX] ^= 0x80000000)
#define FlpSetNegative(x)		(((FlpCompDouble *)&x)->ul[__HIX] |= 0x80000000)
#define FlpSetPositive(x)		(((FlpCompDouble *)&x)->ul[__HIX] &= ~0x80000000)

#ifndef flpErrOutOfRange
#define	flpErrOutOfRange	 ERANGE
#endif

/*
 * These constants are passed to and received from the _fp_round routine.
 */
/*
 * Exception flags.
 */
#define kMaxSignificantDigits 17
#define flpInvalid		FE_INVALID
#define flpOverflow		FE_OVERFLOW
#define flpUnderflow	FE_UNDERFLOW
#define flpDivByZero	FE_DIVBYZERO
#define flpInexact		FE_INEXACT
/*
 * Rounding modes.
 */

#define flpToNearest	FE_TONEAREST
#define flpTowardZero	FE_TOWARDZERO
#define flpUpward		FE_UPWARD
#define flpDownward		FE_DOWNWARD
#define flpModeMask		0	/* 0x00000030 */
#define flpModeShift	0   /*	4 		  */

/*
 * These masks define the fpscr bits supported by the sfpe (software floating point emulator).
 * These constants are used with the _fp_get_fpscr and _fp_set_fpscr routines.
 */



/*
 * These constants are returned by _d_cmp, _d_cmpe, _f_cmp, and _f_cmpe:
 */
/*
	#define flpEqual			0
	#define flpLess			1
	#define flpGreater		2
	#define flpUnordered		3
*/
/*
 * These three functions define the interface to the (software) fpscr
 * of the sfpe. _fp_round not only sets the rounding mode according
 * the low two bits of its argument, but it also returns those masked
 * two bits. This provides some hope of compatibility with less capable
 * emulators, which support only rounding to nearest. A programmer
 * concerned about getting the rounding mode requested can test the
 * return value from _fp_round; it will indicate what the current mode is.
 *
 * Constants passed to and received from _fp_round are:
 *		flpToNearest, flpTowardZero, flpUpward, or flpDownward 
 */


#endif
