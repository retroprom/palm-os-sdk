/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FixedMath.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 * Types and defines shared by Palm OS and HAL drawing functions
 *
 * Name Date Description
 * ---- ---- -----------
 * bob 03/04/02 Split out from CmnDrawingTypes.h
 *
 *****************************************************************************/

#ifndef __FIXEDMATH_H__
#define __FIXEDMATH_H__

#include <stdint.h>

//----------------------------------------------------
// 32 bit (unsigned 16.16) Fixed Point math definitions
//----------------------------------------------------
typedef uint32_t Fixed32;

#if defined(_MSC_VER) // MS VC++ Compiler
typedef unsigned __int64 Fixed32Intermediate;
#else
typedef unsigned long long Fixed32Intermediate;
#endif

#define kFixed32Bias			(16) // Use 16 bits for integer, 16 bits for fraction
#define kFixed32FractionMask	(0x0000FFFF) // low 16 bits turned on

#define Fixed32Fraction(x)		((x) & kFixed32FractionMask)


#define Fixed32FromInteger(n)	((Fixed32)((uint32_t)(n) << kFixed32Bias))
#define Fixed32ToInteger(n)		((Fixed32)((n) >> kFixed32Bias))


#define Fixed32Mul(lhs, rhs)	((Fixed32)((((Fixed32Intermediate) (lhs) * (rhs))) >> kFixed32Bias))
#define Fixed32Div(lhs, rhs)	((Fixed32)((((Fixed32Intermediate) (lhs)) << kFixed32Bias) / ((Fixed32Intermediate) (rhs))))

//-----------------------------------------------
// 32 bit (signed 16.16) Fixed Point math definitions
//-----------------------------------------------

typedef int32_t 				Fixed;
typedef Fixed32Intermediate	FixedIntermediate;

#define kFixedBias			kFixed32Bias
#define kFixedFractionMask	kFixed32FractionMask

#define kFixedOne			0x00010000
#define kFixedOneHalf		0x00008000
#define kFixedTwo			0x00020000
#define kFixedOneAndOneHalf	0x00018000
#define kFixedTwoThirds		0x0000AAAB

// convert integer to Fixed
#define FixedFromInteger(x)	((x) << (kFixedBias))
// convert Fixed to integer
#define FixedToInteger(x)		((Fixed) (x) < 0) ? -(-((Fixed) (x)) >> (kFixedBias)) : ((x) >> (kFixedBias))

#define FixedFraction(x)	((x) & kFixedFractionMask)

#define FixedAdd(lhs, rhs)	((lhs) + (rhs))
#define FixedSub(lhs, rhs)	((lhs) - (rhs))

// if lhs and rhs are fixed, product fixed
// if lhs or rhs are int, product int
#define FixedMul(lhs, rhs)	((((FixedIntermediate) (lhs) * (rhs))) >> kFixedBias)

#define FixedMulByFixed(lhs, rhs) ((Fixed)((((FixedIntermediate) (lhs) * (rhs))) >> kFixedBias))
#define FixedMulByInt16(lhs, rhs) ((int16_t)((((FixedIntermediate) (lhs) * (rhs))) >> kFixedBias))
#define FixedMulByInt32(lhs, rhs) ((int32_t)((((FixedIntermediate) (lhs) * (rhs))) >> kFixedBias))


// if lhs and rhs are fixed, quotient fixed
// if lhs and rhs are int, quotient fixed
#define FixedDiv(lhs, rhs) ((((lhs) == 72) && ((rhs) == 108)) ? kFixedTwoThirds : ((Fixed) (((((FixedIntermediate) (lhs)) << kFixedBias)) / ((FixedIntermediate) (rhs)))))
// lhs is int, rhs is fixed, quotient int
#define DivIntByFixedResultInt(lhs, rhs) ((((FixedIntermediate) (lhs)) << kFixedBias) / ((FixedIntermediate) (rhs)))

// optimization
#define FixedPower2Mul(x, power)	((x) << (power))
#define FixedPower2Div(x, power)	((x) >> (power))

#endif // __FIXEDMATH_H__
