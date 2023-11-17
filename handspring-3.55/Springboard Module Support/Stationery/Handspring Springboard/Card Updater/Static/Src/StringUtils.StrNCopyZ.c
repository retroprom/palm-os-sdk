// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    StrNCopyZ
 *
 *  Summary:      
 *    StrNCopy() the way it should be: it always terminates
 *    the string for you.  That is, it copies at most n-1
 *    characters and _always_ terminates.  The string passed in
 *    as the destination should be n bytes big.
 *
 *  Parameters:
 *    dstP        OUT   The buffer to copy into.
 *    srcP        IN    The string to copy.
 *    n           IN    The size of the destination buffer.
 *    
 *  Returns:
 *    dstP
 *  
 *  Notes:
 *    * Multi-byte clean assuming that terminating null is
 *      one byte big.
 *
 *  History:
 *    07-sep-1999 dia Created
 ****************************************************************/

Char* StrNCopyZ (Char* dstP, const Char* srcP, UInt16 n)
{
  dstP[n-1] = '\0';
  return StrNCopy (dstP, srcP, n-1);
}

