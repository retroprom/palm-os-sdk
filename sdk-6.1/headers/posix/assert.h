/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: assert.h
 *
 * Release: Palm OS 6.0
 *
 *****************************************************************************/

/* Explicitly not guarded against multiple inclusion, as required by the Standard */

#undef _DIAGASSERT
#if !defined(_DIAGNOSTIC)
# if !defined(lint)
#  define _DIAGASSERT(e) (__static_cast(void,0))
# else /* !lint */
#  define _DIAGASSERT(e)
# endif /* lint */
#else /* _DIAGNOSTIC */
# define _DIAGASSERT(e) assert(e)
#endif /* _DIAGNOSTIC */

#undef assert
#ifdef NDEBUG

#define assert(condition) ((void) 0)

#else

/* <sys/cdefs.h> defines __func__ and __BEGIN_DECLS */
#include <sys/cdefs.h>

__BEGIN_DECLS

void _assert_fail(const char* c, const char* f, int l, const char* fc);

#define assert(condition) \
	((condition) ? ((void)0) : _assert_fail( #condition , __FILE__ , __LINE__ , __func__ ))

__END_DECLS

#endif /* def NDEBUG */
