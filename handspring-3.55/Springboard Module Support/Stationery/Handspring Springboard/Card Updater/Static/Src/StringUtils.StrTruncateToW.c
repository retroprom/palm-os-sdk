// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    StrTruncateToWidth 
 *
 *  Summary:      
 *    Truncates the given string to the given number of pixels,
 *    adding an ellipsis to the end of it if truncated.
 *
 *  Parameters:
 *    str         IN/OUT  The string to truncate.
 *    width       IN      The number of pixels to truncate to.
 *    font        IN      The font to use in the calculation;
 *                        ...will not change the currently set font.
 *    
 *  Returns:
 *    The copy, allocated with MemPtrNew
 *  
 *  Notes:
 *    * As far as I know, this function should be multi-byte clean.
 *    * Requires OS 3.1, since it uses the new ellipsis characer.
 *    * If the width won't even handle the ellipsis, just makes
 *      str the empty string.
 *    * Assumes that there is space in the given string buffer
 *      to do ellipse truncation.  This should be fine _unless_
 *      the last character in the string is one byte and the ellipse
 *      character is two bytes, which isn't the case anywhere as far
 *      as I know.
 *
 *  DOLATER:
 *    * Make an alternate version of the function that 
 *      takes/returns the length.  Could make this function 
 *      use that one.
 *
 *  History:
 *    07-sep-1999 dia Created
 *    15-oct-1999 dia Made more multi-byte friendly (should be
 *                    completely multi-byte friendly now).
 *    15-oct-1999 dia Returns the width of the final string...
 *        
 ****************************************************************/

Int16 
StrTruncateToWidth (Char* str, Int16 fieldWidth, FontID font)
{
  Int16           oldLen;
  Int16           newLen;
  Int16           strWidth;
  FontID          currFont;
  
  currFont = FntSetFont (font);
  
  oldLen = StrLen (str);
  newLen = FntWidthToOffset (str, oldLen, fieldWidth, NULL, &strWidth);

  if (newLen != oldLen)
    {
      Int16 ellipseWidth;

      ellipseWidth = FntCharWidth (chrEllipsis);
      fieldWidth -= ellipseWidth;

      if (fieldWidth < 0)
        {
          TxtSetNextChar (str, 0, '\0');
          strWidth = 0;
          newLen = 0;
        }
      else
        {
          newLen = FntWidthToOffset (str, oldLen, fieldWidth, NULL, &strWidth);
          newLen += TxtSetNextChar (str, newLen, chrEllipsis);

          TxtSetNextChar (str, newLen, '\0');
          strWidth += ellipseWidth;
        }
    }

  // now, newLen contains the new length of the string, strWidth contains
  // the width of the string
  ErrNonFatalDisplayIf (newLen != StrLen (str), "Length mismatch");
  ErrNonFatalDisplayIf (strWidth != FntCharsWidth (str, newLen), "Width mismatch");

  (void) FntSetFont (currFont);

  return strWidth;
}
