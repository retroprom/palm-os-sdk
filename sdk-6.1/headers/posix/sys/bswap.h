/*      $NetBSD: bswap.h,v 1.2 2000/02/03 02:03:12 cgd Exp $      */

/* Written by Manuel Bouyer. Public domain */

 /*
 * Copyright (c) 2003, PalmSource Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *		Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer.
 *
 *		Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution.
 *
 *		Neither the name of the PalmSource nor the names of its contributors may
 *		be used to endorse or promote products derived from this software without
 *		specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* completely different header for PalmOS, because BSD's does wacky things for GCC */

#ifndef _SYS_BSWAP_H_
#define _SYS_BSWAP_H_

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

uint16_t       bswap16 __P((uint16_t));
uint32_t       bswap32 __P((uint32_t));
uint64_t       bswap64 __P((uint64_t));

__END_DECLS

#endif /* _SYS_BSWAP_H_ */
