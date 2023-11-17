// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    StrVariableSubstitute 
 *
 *  Summary:      
 *    Does variable substitution on the given string, putting the
 *    output in the given buffer.  The variables should be of the
 *    form "$(var)".  Variables are specified using as arguments
 *    (alternate variable name and definition).  If buffer space
 *    runs out, the output string is simply truncated.
 *
 *  Sample call:
 *    char buffer[64];
 *    StrVariableSubstitute (buffer, 64, "Welcome $(user)@$(host)",
 *                           2, "user", "doug", "host", "dilbert");
 *
 *  Parameters:
 *    buffer      OUT   The buffer to "print" to.
 *    bufferSize  IN    The size of the buffer.
 *    format      IN    The format string to replace variable in.
 *    numVars     IN    The number of variables specified in arguments.
 *    ...         IN    Alternating strings of variable and definition.
 *    
 *  Returns:
 *    The final length of the string.
 *
 *  Notes:
 *    * The format string is allowed to have any number of references
 *      to a variable, including 0.
 *    * The format string may refer to variables not passed in--these
 *      will be replaced by the empty string.
 *    * There is currently no way to specify "$(" as a literal.
 *    * Do not use "$$" in strings--it may be used later to quote $.
 *    * You cannot have recursive variables (variables that refer to
 *      themselves).
 *    * As far as I know, this function should be multi-byte clean
 *      (since the functions it is based on are)
 *  
 *  History:
 *    26-Jul-1999 dia Created
 *    03-sep-1999 dia Made return length; made sure there are no
 *                    reads past the end of the format string.
 *    10-oct-1999 dia Made more multi-byte friendly.
 ****************************************************************/

UInt16 StrVariableSubstitute (Char* buffer, UInt16 bufferSize, Char* format,
                            UInt16 numVars, ...)
{
  const UInt16 origBufferSize = bufferSize;
  const Char* varStartStr = "$(";
  const UInt16 varStartStrLen = StrLen (varStartStr);
  const Char* varEndStr = ")";
  const UInt16 varEndStrLen = StrLen (varEndStr);
  va_list ap;
  typedef struct csub { Char* var; UInt16 varLen; Char* val; } csub;
  csub *subs;
  
  UInt16 i;

  // Make it more convenient to access the substitutions--put in a table.
  subs = (csub*)MemPtrNew (numVars * sizeof (subs[0]));
  va_start (ap, numVars);

  for (i = 0; i < numVars; i++)
    {
      subs[i].var = va_arg(ap, Char*);
      subs[i].varLen = StrLen (subs[i].var);
      subs[i].val = va_arg(ap, Char*);
    }
  va_end (ap);

  // StrNCopy doesn't always terminate if you reach the end of the buffer.
  ErrNonFatalDisplayIf (TxtCharSize ('\0') != 1, "Expect zero-termination fits in 1 byte");
  buffer[bufferSize - 1] = '\0';
  bufferSize -= TxtCharSize ('\0');

  // Do the substitution.  Work left-to-right.  Any undefined variables are
  // treated as empty strings.
  while (bufferSize != 0)
    {
      Char* nextVar;
      Char* nextVarEnd;
      UInt16 copyLen;
      UInt16 foundVarLen;

      nextVar = StrStr (format, varStartStr);
      if (nextVar == NULL)
        {
          StrNCopy (buffer, format, bufferSize);
          bufferSize -= StrLen (format);
          break;
        }
  
      copyLen = nextVar - format;
      if (copyLen > bufferSize)
        copyLen = bufferSize;

      StrNCopy (buffer, format, copyLen);
      buffer += copyLen;
      bufferSize -= copyLen;
      format = nextVar + varStartStrLen;

      nextVarEnd = StrStr (format, varEndStr);
      if (nextVarEnd == NULL)
        {
          // Nice to not crash...this is a good backup case...
          ErrNonFatalDisplayIf (true, "Unterminated $(");
          nextVarEnd = format;
        }

      foundVarLen = nextVarEnd - format;
      for (i = 0; i < numVars; i++)
        {
          const Char* var = subs[i].var;
          const Char* val = subs[i].val;
          const UInt16 varLen = subs[i].varLen;

          if (varLen == foundVarLen && StrNCompare (format, var, varLen) == 0)
            {
              const UInt16 valLen = StrLen (val);

              copyLen = valLen;
              if (copyLen > bufferSize)
                copyLen = bufferSize;

              StrNCopy (buffer, subs[i].val, copyLen);
              buffer += copyLen;
              bufferSize -= copyLen;
              break;
            }
        }
      format += foundVarLen + varEndStrLen;
    }

    MemPtrFree (subs);
    return origBufferSize - bufferSize - TxtCharSize ('\0');
}
