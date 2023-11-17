// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    GetStringResource 
 *
 *  Summary:      
 *    Gets a string out of a resource.  If the given buffer is not
 *    NULL, copies the string into there.  If the given buffer is
 *    NULl, allocates just enough space for the string in the heap.
 *
 *  Parameters:
 *    id          IN    The ID of the string resource.
 *    buffer      OUT   The place to put the string--if NULL, allocate
 *                      memory on the heap.
 *    bufferSize  IN    The size of the buffer in bytes.
 *    
 *  Returns:
 *    A pointer to the string--if buffer was not NULL, this will be
 *    equal to buffer; otherwise it will point to heap memory.
 *    ...returns NULL if the resource doesn't exist.  
 *  
 *  History:
 *    26-Jul-1999 dia Created
 *    03-sep-1999 dia Fixed major bugs with passing NULL buffer (oops)
 ****************************************************************/

Char* GetStringResource (UInt16 id, Char* buffer, UInt16 bufferSize)
{
  MemHandle strH;
  Char* strP;

  strH = DmGetResource (strRsc, id);
  if (strH == 0)
    return NULL;

  strP = (char*)MemHandleLock (strH);
  if (buffer == NULL)
    {
      buffer = StrDup (strP);
      ErrNonFatalDisplayIf (buffer == NULL, "No mem");
    }
  else
    {
      StrNCopy (buffer, strP, bufferSize-1);
      buffer[bufferSize-1] = '\0';
    }
  
  MemHandleUnlock (strH);
  DmReleaseResource (strH);
  return buffer;
}
