// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    StrCSpn 
 *
 *  Summary:      
 *    Implementation of ANSI strcspn() function, which
 *    returns the length of the prefix in str consisting
 *    of characters NOT in set.
 *
 *  Parameters:
 *    str         IN    The string to search through.
 *    set         IN    The set of characters NOT allowable
 *                      in the prefix.
 *    
 *  Returns:
 *    The length of the prefix.
 *  
 *  Notes:
 *    * As far as I know, this function should be multi-byte clean
 *      (since the functions it is based on are)
 *
 *  History:
 *    07-sep-1999 dia Created
 ****************************************************************/

UInt16 StrCSpn (Char* str, Char* set)
{
  Char* cP = str;
  UInt16 prevSize = 0;
  WChar c;

  do
    {
      cP += prevSize;
      prevSize = TxtGetNextChar(cP, 0, &c);
    }
  while (c != '\0' && StrChr (set, c) == NULL);

  return cP - str;
}
