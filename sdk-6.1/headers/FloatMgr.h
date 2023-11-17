/******************************************************************************
 *
 * Copyright (c) 1996-2004 PalmSource, Inc. All rights reserved.
 *
 * File: FloatMgr.h
 *
 * Release: Palm OS 6.0.1
 *
 * Description:
 *		New Floating point routines, provided by new IEEE arithmetic
 *		68K software floating point emulator (sfpe) code.
 *
 *****************************************************************************/

#ifndef _FLOATMGR_H_
#define _FLOATMGR_H_

#include <PalmTypes.h>
#include <CmnErrors.h>
#include <stdio.h>

/************************************************************************
 * New Floating point manager constants
 ***********************************************************************/

#define flpVersion		0x05000000	// first version of NewFloatMgr (PalmOS 5.0)
/************************************************************************
 * Differences between FloatMgr (PalmOS v2.0 and beyond) and (this) NewFloatMgr
 ***********************************************************************/
//
// FloatMgr (PalmOS v2.0+)		NewFloatMgr
// ----------------------		---------------------------------------------
// FloatType (64-bits)			use FlpFloat (32-bits) or FlpDouble (64-bits)
//
// fplErrOutOfRange				use _fp_get_fpscr() to retrieve errors
//
//
#ifndef flpErrOutOfRange
#define flpErrOutOfRange (flpErrorClass | 1)
#endif

#define FlpGetExponent(x)		(((_FLP_HI32(x) & 0x7ff00000) >> 20) - 1023)



	
	
/************************************************************
 * New Floating point manager routines
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
 * New Floating point emulator functions
 *************************************************************/


double	FlpCorrectedAdd(double firstOperand, double secondOperand, int16_t howAccurate);
double	FlpCorrectedSub(double firstOperand, double secondOperand, int16_t howAccurate);
status_t 	FlpBase10Info(double a, uint32_t *mantissaP, int16_t *exponentP, int16_t *signP);
status_t FlpFToA(double , char* );

/* general IEEE 754 arithmetic */

float		FlpUInt32ToFloat(uint32_t );
float		FlpInt32ToFloat(int32_t );
float		FlpULongLongToFloat(uint64_t);
float		FlpLongLongToFloat(int64_t );
double		FlpUInt32ToDouble(uint32_t );
double		FlpInt32ToDouble(int32_t );
double		FlpULongLongToDouble(uint64_t );
double		FlpLongLongToDouble(int64_t );
double		FlpFloatToDouble(float );
float		FlpDoubleToFloat(double );
long double FlpFloatToLongDouble(float );
float		FlpLongDoubleToFloat(long double );
long double FlpDoubleToLongDouble(double );
double		FlpLongDoubleToDouble(long double );
uint32_t	FlpFloatToUInt32(float );
int32_t		FlpFloatToInt32(float );
uint64_t	FlpFloatToULongLong(float );
int64_t		FlpFloatToLongLong(float );
uint32_t	FlpDoubleToUInt32(double );
int32_t		FlpDoubleToInt32(double );
uint64_t	FlpDoubleToULongLong(double );
int64_t		FlpDoubleToLongLong(double );
float		FlpNegFloat(float );
float		FlpAddFloat(float, float);
float		FlpMulFloat(float , float );
float		FlpSubFloat(float , float );
float		FlpDivFloat(float , float );
double		FlpNegDouble(double );
double		FlpAddDouble(double , double );
double		FlpMulDouble(double , double );
double		FlpSubDouble(double , double );
double		FlpDivDouble(double , double );
Boolean		FlpCompareFloatEqual(float , float );
Boolean		FlpCompareFloatLessThan(float , float );
Boolean		FlpCompareFloatLessThanOrEqual(float , float );
Boolean		FlpCompareDoubleEqual(double , double );
Boolean		FlpCompareDoubleLessThan(double , double );
Boolean		FlpCompareDoubleLessThanOrEqual(double , double );



#ifdef __cplusplus
}
#endif


#endif
