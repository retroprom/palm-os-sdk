// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    StrDup 
 *
 *  Summary:      
 *    Implementation of ANSI strdup() function, which
 *    allocates memory in the heap and the copies the given
 *    string into it.
 *
 *  Parameters:
 *    src         IN    The string to copy.
 *    
 *  Returns:
 *    The copy, allocated with MemPtrNew
 *  
 *  Notes:
 *    * As far as I know, this function should be multi-byte clean.
 *
 *  History:
 *    07-sep-1999 dia Created
 ****************************************************************/

Char* StrDup (Char* src)
{
  Char* dst;
  UInt16 len;

  len = StrLen (src);
  dst = (char*)MemPtrNew ((len + 1) * sizeof (Char));
  MemMove (dst, src, len + 1);

  return dst;
}
