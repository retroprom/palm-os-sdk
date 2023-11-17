/******************************************************************************
 *
 * Copyright (c) 2003-2004 PalmSource, Inc. All rights reserved.
 *
 * File: PalmMath.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *    Usefull non C-standard math definitions.
 *
 *****************************************************************************/

#ifndef _PALM_MATH_H
#define _PALM_MATH_H
#include <BuildDefaults.h>		/* for INLINE_FNC */

#if TARGET_HOST==TARGET_HOST_PALMOS
#include <inttypes.h>
#endif

#include <math.h>


/* ############################################ */
/* min_c() and max_c() macros, non-template, usable from C */
#define min_c(a,b) ((a)>(b)?(b):(a))
#define max_c(a,b) ((a)>(b)?(a):(b))


#ifdef __cplusplus
extern "C" {
#endif



/************************************************/
#if (CPU_TYPE == CPU_ARM)
/************************************************/

int32_t lceilf(float);
int32_t lfloorf(float);
void  sincosf(float, float*,float*);

#else

#define lceilf(x)	((int32_t)ceil(x))
#define lfloorf(x)		((int32_t)floor(x))
#if defined( __GNUC__) && !defined( __cplusplus)
#define sincosf( _x,  _y, _z) do {    \
        *_y = (float)sin((double)_x) ; \
        *_z = (float)cos((double)_x) ;  \
} while (0)
#else
INLINE_FNC void sincosf(float _x, float* _y,float* _z) {
	*_y=(float)sin(_x); *_z=(float)cos(_x);
	}
#endif	
	


#endif

/* ############################################ */

#ifdef __cplusplus
}
#endif

/* 
   Single precision Math constants.  These constants are
   not reliable in double precision expressions since they
   will be compiled as floats with a 24 bit significand.
   The equivalent 53 bit double precision constants can be
   attained by removing the "f" suffix from each of the decimal
   strings below.
*/
#ifndef M_E
#define M_E         2.7182818284590452354f
#endif
#ifndef M_LOG2E
#define M_LOG2E     1.4426950408889634074f
#endif
#ifndef M_LOG10E
#define M_LOG10E    0.43429448190325182765f
#endif
#ifndef M_LN2
#define M_LN2       0.69314718055994530942f
#endif
#ifndef M_LN10
#define M_LN10      2.30258509299404568402f
#endif
#ifndef M_PI
#define M_PI        3.14159265358979323846f  /* PI */
#endif
#ifndef M_PI_2
#define M_PI_2      1.57079632679489661923f  /* PI * .5 */
#endif
#ifndef M_1_PI
#define M_1_PI      0.31830988618379067154f  /* 1 / PI */
#endif
#ifndef M_PI_4
#define M_PI_4      0.78539816339744830962f  /* PI / 4 */
#endif
#ifndef M_2_PI
#define M_2_PI      0.63661977236758134308f  /* 2 / PI */
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI  1.12837916709551257390f  /* 2 / sqrt(PI) */
#endif
#ifndef M_SQRT2
#define M_SQRT2     1.41421356237309504880f  /* sqrt(2) */
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.70710678118654752440f  /* 1 / sqrt(2) */
#endif
#ifndef PI
#define PI  M_PI
#endif
#ifndef PI2
#define PI2  M_PI_2
#endif

/*! A few additional, useful angles for Arc(), use them for the arclen whenever possible,
	arcs with these arclen are computed at no cost.
*/
#ifndef M_PI_3
#define M_PI_3		1.047197551196597746154f		/* !<	   pi/3		 60 deg */
#endif

#ifndef M_3_PI_4
#define M_3_PI_4	2.356194490192344928846f		/*!<	 3*pi/4		135 deg */
#endif

#ifndef M_5_PI_4
#define M_5_PI_4	3.926990816987241548076f		/*!<	 5*pi/4		225 deg */
#endif

#ifndef M_3_PI_2
#define M_3_PI_2	4.71238898038468985769f			/*!<	 3*pi/2		270 deg */
#endif

#ifndef  M_7_PI_4
#define M_7_PI_4	5.497787143782138167306f		/*!<	 7*pi/4		315 deg */
#endif

#endif /* _PALM_MATH_H */
