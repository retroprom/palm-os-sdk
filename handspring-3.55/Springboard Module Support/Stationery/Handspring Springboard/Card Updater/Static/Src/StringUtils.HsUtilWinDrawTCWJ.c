// See master "StringUtils.c" file for header comments...

#include <stdarg.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <TextMgr.h>

#include "StringUtils.h"

/***************************************************************
 *  Function:    HsUtilWinDrawTruncCharsWithJust 
 *
 *  Summary:      
 *    A version of WinDrawTruncChars that adds justification.
 *    See WinDrawTruncChars for a full description.
 *
 *  Parameters:
 *    pChars    IN  The string to draw.
 *    length    IN  The length of pChars (in bytes).
 *    x         IN  The left coord to draw at.  Note that depending
 *                  on the width of the string, the maxWidth, and
 *                  the justification, drawing may start to the
 *                  right of this location.
 *    y         IN  The top coord to draw at.
 *    maxWidth  IN  The width of the "field".  The string will be
 *                  justified in this field.
 *    just      IN  The justification (justLeft, justRight, or
 *                  justCenter).
 *    
 *  Returns:
 *    nothing
 *  
 *  Notes:
 *    * As far as I know, this function should be multi-byte clean.
 *    * Requires OS 3.1, since it uses the new ellipsis characer.
 *    * If the width won't even handle the ellipsis, draws nothing.
 *
 *  History:
 *    15-oct-1999 dia Created based on DrawStringWithEllipsis from
 *                    Vitaly's FileMover (except that this one is
 *                    multibyte clean).
 *        
 ****************************************************************/

void
HsUtilWinDrawTruncCharsWithJust (const Char* pChars, UInt16 length, 
                                 Int16 x, Int16 y, Int16 maxWidth, 
                                 JustificationEnum just)
{
  Int16 ellipseWidth = 0;
  Int16 strWidth;
  UInt16 strLen;

  strLen = FntWidthToOffset (pChars, length, maxWidth, NULL, &strWidth);
  
  // If the string doesn't fit, make room for an ellipsis...
  if (strLen != length)
    {
      ellipseWidth = FntCharWidth (chrEllipsis);
      if (ellipseWidth > maxWidth)
        {
          // Nothing fits--not even an ellipse.  Set so nothing draws...
          strLen = 0;
          strWidth = 0;
          ellipseWidth = 0;
        } 
      else
        {
          // Take out the width of an ellipsis...
          strLen = FntWidthToOffset (pChars, length, maxWidth - ellipseWidth, 
                                     NULL, &strWidth);
        }
    }

  // Adjust for alignment.
  if (just == justCenter)
	{
	  x += ((maxWidth - strWidth - ellipseWidth) / 2);
	}
  else if (just == justRight)
	{
	  x += (maxWidth - strWidth - ellipseWidth);
	}

  // Draw the string...
  WinDrawChars (pChars, strLen, x, y);

  // Draw the ellipsis, if necessary...
  if (ellipseWidth != 0)  
    {
      x += strWidth;

      //#define WIN_DRAW_CHAR_WORKS_WITH_ELLIPSIS 
      #ifdef WIN_DRAW_CHAR_WORKS_WITH_ELLIPSIS
        {
          WinDrawChar (chrEllipsis, x, y);
        }
      #else // (not) WIN_DRAW_CHAR_WORKS_WITH_ELLIPSIS
        {
          Char elP[4];
          UInt16 elLen;

          elLen = TxtSetNextChar (elP, 0, chrEllipsis);
          elP[elLen] = '\0';

          WinDrawChars (elP, elLen, x, y);
        }
      #endif // (not) WIN_DRAW_CHAR_WORKS_WITH_ELLIPSIS
    }
}




