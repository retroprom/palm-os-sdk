/******************************************************************************
 *
 * Copyright (c) 2004 PalmSource, Inc. All rights reserved.
 *
 * File: math.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Palm OS 6 library for math functions
 *		Based on the Freely-Available math library (fdlibm) at 
 *      http://www.netlib.org/fdlibm
 *
 *****************************************************************************/

/*
	Differences from C99 math.h specification:

	Argument types: parallel sets of functions for float and long double
		arguments defined only for 1989 ANSI C functions.

	All functions covered prior to C99 are provided here, as well as a 
	majority of C99.
	
	Functions not in SystemLib: listed under __USE_C99_EXTENSIONS__ below. 
	
	
	We assume sizeof(double)=sizeof(long double) = 8 
	
	We do not set floating point exceptions in exceptional cases, but 
	correctly set errno and return the appropriate value (e.g. a nan or
	infinity).
*/

#ifndef __INC_MATH__
#define __INC_MATH__

#include <PalmTypes.h>     /* need _FLP_HI32, _FLP_LO32 macros */
#include <BuildDefaults.h> /* for INLINE_FNC */
#include <inttypes.h>
#include <limits.h>
#if CPU_TYPE == CPU_ARM
#include <machine/arm/ansi.h>
#elif CPU_TYPE == CPU_x86
#include <machine/x86/ansi.h>
#else
#error Building on unsupported platform
#endif
#if defined( __GNUC__) 
#ifndef __cplusplus
#define __NO_MATH_INLINES__ 1
#endif
#endif
/* 7.12.1 Treatment of error conditions */
#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2
#define math_errhandling MATH_ERRNO

#define FLT_EVAL 	 0  /* 
							float_t and double_t should be IEEE 754 single (4 byte float) 
							and IEEE 754 double (8 byte double) 
						*/

/* 7.12.3  Classification macros */
#define FP_NAN       1  /*   quiet NaN            */
#define FP_INFINITE  2  /*   +/- infinity         */
#define FP_ZERO      3  /*   +/- zero             */
#define FP_NORMAL    4  /*   all normal numbers   */
#define FP_SUBNORMAL 5  /*   tiny numbers         */

#ifndef __NO_MATH_INLINES__ 
INLINE_FNC int  __signbitf(float _x)
{
	return (int)((*(uint32_t*)&_x)>>31);
}

INLINE_FNC int  __signbitd(double _x)
{
	return (int)(_FLP_HI32(_x)>>31);
}
/* inline versions of classification macros optimized for floats */
INLINE_FNC int __isnormalf(float _x) 
{
	const uint32_t _f =  (*(uint32_t*)&_x)&0x7f800000;
	return (int) ((_f < 0x7f800000) && _f );
	
}
INLINE_FNC int __isnanf(float _x) 
{
	const uint32_t _f =  (*(uint32_t*)&_x)&0x7fffffff;
	return (int) ( _f > 0x7f800000 ) ;
}
INLINE_FNC int __isinf(float _x) 
{
	const uint32_t _f =  (*(uint32_t*)&_x)&0x7fffffff;
	return (int) (_f == 0x7f800000) ;
}

INLINE_FNC int __isfinitef(float _x) 
{
	const uint32_t _f =  (*(uint32_t*)&_x)&0x7f800000;
	return (int) (_f < 0x7f800000) ;
}

INLINE_FNC int __fpclassifyf(float _x)
{
	 const uint32_t _f =  (*(uint32_t*)&_x)&0x7f800000;
	 if(_f== 0x7f800000)
	 {
	    if((*(uint32_t*)&_x)&0x007fffff) 
	    	return FP_NAN;
	    else
	    	return FP_INFINITE;
	 }
	 else if(!_f)
	 {
	    if((*(uint32_t*)&_x)&0x007fffff) 
	    	return FP_SUBNORMAL;
	    else 
	    	return FP_ZERO;  
	 }  
	 return FP_NORMAL;
}  



INLINE_FNC int  __fpclassifyd(double _x) 
{
  const uint32_t _f = _FLP_HI32(_x)&0x7ff00000 ;
  if(_f == 0x7ff00000)
  {
    if((_FLP_HI32(_x)&0x000fffff) || (_FLP_LO32(_x)&0xffffffff))
     return FP_NAN;
    else
     return FP_INFINITE;  
  }
  else if(!_f)
  {
    if((_FLP_HI32(_x)&0x000fffff) || (_FLP_LO32(_x)&0xffffffff)) 
    	return FP_SUBNORMAL;
    else 
    	return FP_ZERO; 

  }

  return FP_NORMAL;
} 

/*  special number macros */
INLINE_FNC  float __huge_valf(void)
{
	uint32_t _f = 0x7f800000ul;
	return *(float*)&_f;
}

INLINE_FNC  double __huge_val(void)
{
#if  defined (_MSC_VER)
	const uint64_t _f = 0x7ff0000000000000;
#else
	const uint64_t _f = 0x7ff0000000000000ull;
#endif
	return *(double*)&_f;
}

INLINE_FNC  float __nan(void)
{
	const uint32_t _f = 0x7fc00000ul;
	return *(float *)&_f;
}
#if !defined(__ARMCC_VERSION)

INLINE_FNC float _fabsf(float _x)
{
	const uint32_t _f =  (*(uint32_t*)&_x)&0x7fffffff;
	return *(float *)&_f;
}
#endif /*  !defined(__ARMCC_VERSION) */

#else /* __NO_MATH_INLINES__ */
/*  special number macros */

extern const float __huge_valf(void);
extern const double __huge_val(void);
extern const float __nan(void);
extern int  __fpclassifyd(double _x) ;
extern int  __fpclassifyf(float _x) ;
extern int  __signbitd(double _x) ;
extern int  __signbitf(float _x) ;
#if defined( __GNUC__) 
extern long double __gcc_modfl_12byteld(long double , long double* );
extern long double __gcc_modfl_16byteld(long double , long double* );
#define modfl(_x,_y) ( sizeof(long double) == 12 ? __gcc_modfl_12byteld(_x,_y) : \
                                                   __gcc_modfl_16byteld(_x,_y))
#else                                                   
extern long double modfl(long double , long double *);
#endif
extern float modff(float , float *);
#endif /* __NO_MATH_INLINES__  */

#define fpclassify(_x)  \
	 	((sizeof(_x) == sizeof(float)) ? __fpclassifyf((float)_x) : \
	 	 (__fpclassifyd((double)_x)) )
	 	 
#if defined( __GNUC__)  || defined( _PACC_VER)/* use floating point hexadecimal constants */
#define HUGE_VAL  0x1.0p2047
#define HUGE_VALF 0x1.0p255f
#define NAN 	  (0x1.1p255f - 0x1.1p255f)
#else
#define HUGE_VAL  (__huge_val())  
#define NAN       ( __nan())	/* quiet Nan of type float -- can be used to manipulate nans without generating exceptions */
#define HUGE_VALF ( __huge_valf())
#endif

#if defined( __GNUC__) 
#define isnormal(_x) (fpclassify(_x) == FP_NORMAL)
#define isnan(_x)    (fpclassify(_x) == FP_NAN)
#define isinf(_x)    (fpclassify(_x) == FP_INFINITE)
#define isfinite(_x) ((fpclassify(_x) > FP_INFINITE))
#else
#define isnormal(_x) ((sizeof(_x) == sizeof(float)) ? __isnormalf((float)_x) : (__fpclassifyd((double)_x) == FP_NORMAL))
#define isnan(_x)    ((sizeof(_x) == sizeof(float)) ? __isnanf((float)_x) : (__fpclassifyd((double)_x) == FP_NAN))
#define isinf(_x)    ((sizeof(_x) == sizeof(float)) ? __isinf((float)_x) : (__fpclassifyd((double)_x) == FP_INFINITE))
#define isfinite(_x) ((sizeof(_x) == sizeof(float)) ? __isfinitef((float)_x) : (__fpclassifyd((double)_x) > FP_INFINITE))
#endif

#define HUGE_VALL  HUGE_VAL
#define INFINITY  HUGE_VALF



#define FP_ILOGBNAN  INT_MAX	
#define FP_ILOGB0	 INT_MIN




#define signbit(_x)((sizeof(_x) == 4) ? __signbitf((float)_x) : __signbitd((double)_x))

/* ANSI/POSIX function prototypes */

#ifdef __cplusplus
extern "C" {
#endif


#if defined(__ARMCC_VERSION)
float (_fabsf)(float);
#endif
/* 7.12.4 Trigonometric functions */
double cos(double);
double sin(double);
double tan(double);
double acos(double);
double asin(double);
double atan(double);
double atan2(double, double);

/* 7.12.5 Hyperbolic functions */
double cosh(double);
double sinh(double);
double tanh(double);
double acosh(double);
double asinh(double);
double atanh(double);


/* 7.12.6 Exponential and logarithmic functions */
double exp(double);
double expm1(double);
extern int ilogb(double);
double log(double);
double log10(double);
double logb(double);
double frexp(double, int *);
double ldexp(double, int);
double modf(double, double *);
double scalbn(double, int);
double log1p(double);

/* 7.12.7 Power and absolute-value functions */
double cbrt(double);
double hypot(double, double);
double pow(double, double);
double sqrt(double);
double fabs(double);


/* 7.12.9 Nearest integer functions */
double ceil(double);
double floor(double);
double rint(double);
/* 7.12.10 Remainder functions */
double fmod(double, double);
double remainder(double, double);
/* 7.12.11 Manipulation functions */
double copysign(double, double);
double nextafter(double, double);

/*
  The __USE_C99_EXTENSIONS__ switch can be turned on when one of the lesser
  used math.h extensions is needed.  The functions within this area are NOT 
  included with the shared math library in ROM and not supported by Palmsource.
  These prototypes are here for convenience, but also to clearly indicate what is
  not in ROM.  Some of the sources for these functions may be available at:
  http://www.netlib.org/fdlibm 
*/
  
#ifdef __USE_C99_EXTENSIONS__


/* 7.12.6 Exponential and logarithmic functions */
double exp2(double);
double log2(double);
double scalbln(double, long int);

/* 7.12.8 Error and gamma functions */
double erf(double);
double erfc(double);
double lgamma(double);
double tgamma(double);
/* 7.12.9 Nearest integer functions */
double nearbyint(double );
long int lrint(double );
long long int llrint(double );
double round(double );
long int lround(double );
long long int llround(double );
double trunc(double );
/* 7.12.10 Remainder functions */
double remquo(double , double , int*);
/* 7.12.11 Manipulation functions */
double nan(const char*);
double nexttoward(double , long double );
/* 7.12.12 Maximum, minimum, and positive difference functions */
double fdim(double, double);
double fmax(double, double);
double fmin(double, double);
/* 7.12.13 Floating multiply-add */
double fma(double, double, double);
/* 7.12.14 Comparison macros */
#define isgreater(_x,_y)
#define isless(_x,_y) 
#define islessequal(_x,_y) 
#define islessgreater(_x,_y) 
#define isgreaterequal(_x,_y)
#define isunordered(_x,_y) 
#endif /* __USE_C99_EXTENSIONS__ */



#if CPU_TYPE == CPU_x86                /* there are ARM specific versions of these in SystemLib */
#if !defined(__NO_MATH_INLINES__)

INLINE_FNC float sqrtf(float _x)
        {return ((float)sqrt((double)_x)); }
INLINE_FNC float ceilf(float _x)
        {return ((float)ceil((double)_x)); } 
INLINE_FNC float floorf(float _x)
        {return ((float)floor((double)_x)); } 
INLINE_FNC float hypotf(float _x, float _y)
        {return ((float)hypot((double)_x,(double)_y)); }  
#endif        
#else
float ceilf(float);
float floorf(float);
float hypotf(float,float);
float sqrtf(float);
#endif             
        
#ifdef __cplusplus 
}
#endif

/* float and long double overloads + abs overloads required by C++ standard 
   we only supply these overloads for the 1989 ANSI C standard 
*/
#ifdef __cplusplus 
INLINE_FNC double  abs(double _x)   // need this to satisfy section 26.5.6 of standard
        {return (fabs(_x)); }
INLINE_FNC float  abs(float _x)   // need this to satisfy section 26.5.6 of standard
        {return (_fabsf(_x)); } 
INLINE_FNC long double  abs(long double _x)   // need this to satisfy section 26.5.6 of standard
        {return ((long double)fabs((double)_x)); } 
#endif /* __cplusplus */

#if  !defined(__NO_MATH_INLINES__)
INLINE_FNC long double acosl(long double _x)
        {return (acos((double)_x)); }
INLINE_FNC long double asinl(long double _x)
        {return (asin((double)_x)); }
INLINE_FNC long double atanl(long double _x)
        {return (atan((double)_x)); }
INLINE_FNC long double atan2l(long double _x, long double _y)
        {return (atan2((double)_x, (double)_y)); }
INLINE_FNC long double ceill(long double _x)
        {return (ceil((double)_x)); }
INLINE_FNC long double cosl(long double _x)
        {return (cos((double)_x)); }
INLINE_FNC long double coshl(long double _x)
        {return (cosh((double)_x)); }
INLINE_FNC long double expl(long double _x)
        {return (exp((double)_x)); }
INLINE_FNC long double fabsl(long double _x)
        {return (fabs((double)_x)); }
INLINE_FNC long double floorl(long double _x)
        {return (floor((double)_x)); }
INLINE_FNC long double fmodl(long double _x, long double _y)
        {return (fmod((double)_x, (double)_y)); }
INLINE_FNC long double frexpl(long double _x, int *_y)
        {return (frexp((double)_x, _y)); }
INLINE_FNC long double ldexpl(long double _x, int _y)
        {return (ldexp((double)_x, _y)); }
INLINE_FNC long double logl(long double _x)
        {return (log((double)_x)); }
INLINE_FNC long double log10l(long double _x)
        {return (log10((double)_x)); }
INLINE_FNC long double modfl(long double _x, long double *_y)
        {double _Di, _Df = modf((double)_x, &_Di);
        *_y = (long double)_Di;
        return (_Df); }
INLINE_FNC long double powl(long double _x, long double _y)
        {return (pow((double)_x, (double)_y)); }
INLINE_FNC long double sinl(long double _x)
        {return (sin((double)_x)); }
INLINE_FNC long double sinhl(long double _x)
        {return (sinh((double)_x)); }
INLINE_FNC long double sqrtl(long double _x)
        {return (sqrt((double)_x)); }
INLINE_FNC long double tanl(long double _x)
        {return (tan((double)_x)); }
INLINE_FNC long double tanhl(long double _x)
        {return (tanh((double)_x)); }

INLINE_FNC long double hypotl(long double _x, long double _y)
        {return (hypot((double)_x, (double)_y)); }
        
/* float overloads for 1989 standard functions only */
INLINE_FNC float acosf(float _x)
        {return ((float)acos((double)_x)); }
INLINE_FNC float asinf(float _x)
        {return ((float)asin((double)_x)); }
INLINE_FNC float atanf(float _x)
        {return ((float)atan((double)_x)); }
INLINE_FNC float atan2f(float _x, float _y)
        {return ((float)atan2((double)_x, (double)_y)); }

INLINE_FNC float cosf(float _x)
        {return ((float)cos((double)_x)); }
INLINE_FNC float sinf(float _x)
        {return ((float)sin((double)_x)); }
INLINE_FNC float tanf(float _x)
        {return ((float)tan((double)_x)); }
                        
INLINE_FNC float coshf(float _x)
        {return ((float)cosh((double)_x)); }
INLINE_FNC float sinhf(float _x)
        {return ((float)sinh((double)_x)); }

INLINE_FNC float tanhf(float _x)
        {return ((float)tanh((double)_x)); } 

                
INLINE_FNC float expf(float _x)
        {return ((float)exp((double)_x)); }
INLINE_FNC float fabsf(float _x)
        {return _fabsf(_x); }
INLINE_FNC float frexpf(float _x, int *_y)
        {return ((float)frexp((double)_x, _y)); }
INLINE_FNC float ldexpf(float _x, int _y)
        {return ((float)ldexp((double)_x, _y)); }        

INLINE_FNC float fmodf(float _x, float _y)
        {return ((float)fmod((double)_x, (double)_y)); }
INLINE_FNC float logf(float _x)
        {return ((float)log((double)_x)); }
INLINE_FNC float log10f(float _x)
        {return ((float)log10((double)_x)); }
        
INLINE_FNC float modff(float _x, float *_y)
        { double _Di, _Df = modf((double)_x, &_Di);
        *_y = (float)_Di;
        return ((float)_Df); }
INLINE_FNC float powf(float _x, float _y)
        {return ((float)pow((double)_x, (double)_y)); }

#else  

#define acosl acos
#define asinl asin
#define atanl atan
#define atan2l atan2
#define ceill ceil
#define cosl cos
#define coshl cosh
#define expl exp
#define fabsl fabs
#define floorl floor
#define fmodl fmod
#define frexpl frexp
#define ldexpl ldexp
#define logl log
#define log10l log10
#define powl pow
#define sinl sin
#define sinhl sinh
#define sqrtl sqrt
#define tanl tan
#define tanhl tanh
#define hypotl hypot
       
/* float overloads for 1989 standard functions only */
#define acosf acos
#define asinf asin
#define atanf atan
#define atan2f atan2
#define cosf cos
#define sinf sin
#define tanf tan
#define coshf cosh
#define sinhf sinh
#define tanhf tanh 
#define expf exp
#define fabsf fabs
#define frexpf frexp
#define ldexpf ldexp
#define fmodf fmod
#define logf log
#define log10f log10      
#define powf pow
#define ceilf ceil
#define floorf floor
#define hypotf hypot
#define sqrtf sqrt
#endif /* __cplusplus  || !defined(__NO_MATH_INLINES__) */    
#endif		/* __INC_MATH__ */
