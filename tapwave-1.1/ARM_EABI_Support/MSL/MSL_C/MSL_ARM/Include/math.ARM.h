
/* Metrowerks Standard Library
 * Copyright � 1995-2003 Metrowerks Corporation.  All rights reserved.
 *
 * $Date: 2003/05/19 21:42:30 $
 * $Revision: 1.9 $
 */

/* $Id: math.ARM.h,v 1.9 2003/05/19 21:42:30 ceciliar Exp $ */

#ifndef _MSL_MATH_ARM_H
#define _MSL_MATH_ARM_H

#ifndef _MSL_CMATH
#error This header may only be included from <cmath>
#endif

#define __SET_ERRNO__
#ifdef __SET_ERRNO__
	#include <cerrno>
#endif

_MSL_BEGIN_NAMESPACE_STD

	typedef float float_t;
	typedef double double_t;

_MSL_END_NAMESPACE_STD

_MSL_BEGIN_EXTERN_C
	/* private functions */
	_MSL_IMP_EXP_C  double      _MSL_MATH_CDECL gamma(double);
	
_MSL_END_EXTERN_C

_MSL_BEGIN_NAMESPACE_STD
_MSL_BEGIN_EXTERN_C

	#if _MSL_USE_INLINE

		_MSL_INLINE float _MSL_MATH_CDECL
			acosf(float x) { return (float)(acos)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			acoshf(float x) { return (float)(acosh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			acoshl(long double x) { return (long double)(acosh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			acosl(long double x) { return (long double)(acos)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			asinf(float x) { return (float)(asin)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			asinhf(float x) { return (float)(asinh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			asinhl(long double x) { return (long double)(asinh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			asinl(long double x) { return (long double)(asin)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			atan2f(float y, float x) { return (float)(atan2)((double)(y), (double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			atan2l(long double y, long double x) { return (long double)(atan2)((double)(y), (double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			atanf(float x) { return (float)(atan)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			atanhf(float x) { return (float)(atanh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			atanhl(long double x) { return (long double)(atanh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			atanl(long double x) { return (long double)(atan)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			cbrtl(long double x) { return (long double)(cbrt)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			ceilf(float x) { return (float)(ceil)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			ceill(long double x) { return (long double)(ceil)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			copysignf(float x, float y) { return (float)(copysign)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			copysignl(long double x, long double y) { return (long double)(copysign)((double)(x), (double)(y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			cosf(float x) { return (float)(cos)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			coshf(float x) { return (float)(cosh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			coshl(long double x) { return (long double)(cosh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			cosl(long double x) { return (long double)(cos)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			expf(float x) { return (float)(exp)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			expl(long double x) { return (long double)(exp)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			expm1f(float x) { return (float)(expm1)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			expm1l(long double x) { return (long double)(expm1)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			fabsf(float x) { return (float)(fabs)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			fabsl(long double x) { return (long double)(fabs)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			fdimf(float x, float y) { return (float)(fdim)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			fdiml(long double x, long double y) { return (long double)(fdim)((double)(x), (double)(y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			floorf(float x) { return (float)(floor)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			floorl(long double x) { return (long double)(floor)((double)(x)); }
		/*
		_MSL_INLINE float _MSL_MATH_CDECL
			fmaf(float x, float y, float z) { return (float)(fma)((double)(x), (double)(y), (double)(z)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			fmal(long double x, long double y, long double z) { return (long double)(fma)((double)(x), (double)(y), (double)(z)); }
		*/
		_MSL_INLINE float _MSL_MATH_CDECL
			fmaxf(float x, float y) { return (float)(fmax)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			fmaxl(long double x, long double y) { return (long double)(fmax)((double)(x), (double)(y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			fminf(float x, float y) { return (float)(fmin)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			fminl(long double x, long double y) { return (long double)(fmin)((double)(x), (double)(y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			fmodf(float x, float y) { return (float)(fmod)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			fmodl(long double x, long double y) { return (long double)(fmod)((double)(x), (double)(y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			frexpf(float x, int* y) { return (float)(frexp)((double)(x), (y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			frexpl(long double x, int* y) { return (long double)(frexp)((double)(x), (y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			hypotf(float x, float y) { return (float)(hypot)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			hypotl(long double x, long double y) { return (long double)(hypot)((double)(x), (double)(y)); }
		#if _MSL_USES_SUN_MATH_LIB
		_MSL_INLINE int _MSL_MATH_CDECL
			ilogbf(float x) { return (ilogb)((double)(x)); }
		_MSL_INLINE int _MSL_MATH_CDECL
			ilogbl(long double x) { return (ilogb)((double)(x)); }
		#endif
		_MSL_INLINE float _MSL_MATH_CDECL
			ldexpf(float x, int y) { return (float)(ldexp)((double)(x), (y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			ldexpl(long double x, int y) { return (long double)(ldexp)((double)(x), (y)); }
		#if _MSL_LONGLONG
		_MSL_INLINE long long _MSL_MATH_CDECL
			llrintf(float x) { return (llrint)((double)(x)); }
		_MSL_INLINE long long _MSL_MATH_CDECL
			llrintl(long double x) { return (llrint)((double)(x)); }
		#if !_MSL_USES_SUN_MATH_LIB			
		_MSL_INLINE long long _MSL_MATH_CDECL
			llroundf(float x) { return (llround)((double)(x)); }
		_MSL_INLINE long long _MSL_MATH_CDECL
			llroundl(long double x) { return (llround)((double)(x)); }
		#endif
		#endif
		_MSL_INLINE float _MSL_MATH_CDECL
			log10f(float x) { return (float)(log10)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			log10l(long double x) { return (long double)(log10)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			log1pf(float x) { return (float)(log1p)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			log1pl(long double x) { return (long double)(log1p)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			log2f(float x) { return (float)(log2)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			log2l(long double x) { return (long double)(log2)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			logbf(float x) { return (float)(logb)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			logbl(long double x) { return (long double)(logb)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			logf(float x) { return (float)(log)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			logl(long double x) { return (long double)(log)((double)(x)); }
		/*
		_MSL_INLINE long _MSL_MATH_CDECL
			lrintf(float x) { return (lrint)((double)(x)); }
		_MSL_INLINE long _MSL_MATH_CDECL
			lrintl(long double x) { return (lrint)((double)(x)); }	
		_MSL_INLINE long _MSL_MATH_CDECL
			lroundf(float x) { return (lround)((double)(x)); }
		_MSL_INLINE long _MSL_MATH_CDECL
			lroundl(long double x) { return (lround)((double)(x)); }
		*/	
		_MSL_INLINE float _MSL_MATH_CDECL
			modff(float x, float* iptr) {
		  double iptrd;
		  float result = (float)modf((double)x, &iptrd);
		  *iptr = (float)iptrd;
		  return result;
		}
		
		_MSL_INLINE long double _MSL_MATH_CDECL
			modfl(long double x, long double* iptr) {
		  double iptrd;
		  long double result = (long double)modf((double)x, &iptrd);
		  *iptr = (long double)iptrd;
		  return result;
		}
		/*
		_MSL_INLINE float _MSL_MATH_CDECL
			nearbyintf(float x) { return (float)(nearbyint)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			nearbyintl(long double x) { return (long double)(nearbyint)((double)(x)); }
		*/
		#if !__INTEL__
		_MSL_INLINE long double _MSL_MATH_CDECL
			nextafterl(long double x, long double y) { return (long double)(nextafter)((double)(x), (double)(y)); }
		#endif
		_MSL_INLINE long double _MSL_MATH_CDECL
			nexttowardl(long double x, long double y) { return (long double)(nexttoward)((double)(x), (y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			powf(float x, float y) { return (float)(pow)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			powl(long double x, long double y) { return (long double)(pow)((double)(x), (double)(y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			remainderf(float x, float y) { return (float)(remainder)((double)(x), (double)(y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			remainderl(long double x, long double y) { return (long double)(remainder)((double)(x), (double)(y)); }
		/*
		_MSL_INLINE float _MSL_MATH_CDECL
			remquof(float x, float y, int* z) { return (float)(remquo)((double)(x), (double)(y), (z)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			remquol(long double x, long double y, int* z) { return (long double)(remquo)((double)(x), (double)(y), (z)); }
		*/
		_MSL_INLINE float _MSL_MATH_CDECL
			rintf(float x) { return (float)(rint)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			rintl(long double x) { return (long double)(rint)((double)(x)); }
		/*	
		_MSL_INLINE float _MSL_MATH_CDECL
			roundf(float x) { return (float)(round)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			roundl(long double x) { return (long double)(round)((double)(x)); }			
		_MSL_INLINE float _MSL_MATH_CDECL
			scalblnf(float x, long int y) { return (float)(scalbln)((double)(x), (y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			scalblnl(long double x, long int y) { return (long double)(scalbln)((double)(x), (y)); }
		*/	
		_MSL_INLINE float _MSL_MATH_CDECL
			scalbnf(float x, int y) { return (float)(scalbn)((double)(x), (y)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			scalbnl(long double x, int y) { return (long double)(scalbn)((double)(x), (y)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			sinf(float x) { return (float)(sin)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			sinhf(float x) { return (float)(sinh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			sinhl(long double x) { return (long double)(sinh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			sinl(long double x) { return (long double)(sin)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			sqrtf(float x) { return (float)(sqrt)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			sqrtl(long double x) { return (long double)(sqrt)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			tanf(float x) { return (float)(tan)((double)(x)); }
		_MSL_INLINE float _MSL_MATH_CDECL
			tanhf(float x) { return (float)(tanh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			tanhl(long double x) { return (long double)(tanh)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			tanl(long double x) { return (long double)(tan)((double)(x)); }
		/*
		_MSL_INLINE float _MSL_MATH_CDECL
			truncf(float x) { return (float)(trunc)((double)(x)); }
		_MSL_INLINE long double _MSL_MATH_CDECL
			truncl(long double x) { return (long double)(trunc)((double)(x)); }
		*/

	#endif /* _MSL_USE_INLINE */

_MSL_END_EXTERN_C
_MSL_END_NAMESPACE_STD

#endif /* _MSL_MATH_ARM_H */

/* Change record: 
 * ejs 020129 This file is generated from a script now.
 * ejs 020130 Added float_t and double_t to namespace std
 * cc  021022 Added ilogb and cbrt to arm
 * JWW 030224 Changed __MSL_LONGLONG_SUPPORT__ flag into the new more configurable _MSL_LONGLONG
 */