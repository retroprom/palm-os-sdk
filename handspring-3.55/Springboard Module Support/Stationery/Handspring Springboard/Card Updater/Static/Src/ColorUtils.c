/***************************************************************
 *
 * Project:
 *    ViewerUtils 
 *
 * Copyright info:
 *    Copyright Handspring, Inc. 2000
 *
 * FileName:
 *    ColorUtils.c
 * 
 * Description:
 *    This file contains utility functions dealing with color.
 *
 * ToDo:
 *
 * History:
 *    23-may-2000 dia  Created by Douglas Anderson (dianders@handspring.com)
 *
 ****************************************************************/

#include <PalmOS.h>				// all the system toolbox headers

#include "ColorUtils.h"


/***************************************************************
 *  Function:    HsUtilSetScreenDepth
 *
 *  Summary:
 *    Sets the screen depth using the table passed in.  
 *    Processes the table starting at element 0 until it finds
 *    an entry where the current depth is in the set of
 *    fromDepths and the toDepth is supported.
 *
 *  Parameters:
 *    numMappings       IN  Num entries in depthMappingsTbl.
 *    depthMappingsTbl  IN  A table of mappings from sets of
 *                          start depths to final depths.
 *    
 *  Returns:
 *    The final depth after the function has run.
 *  
 *  Called By: 
 *    client
 *
 *  History:
 *    23-may-2000 dia Created.
 *
 ****************************************************************/

UInt16
HsUtilSetScreenDepth (UInt16 numMappings, 
                      const HsUtilDepthMappingType* depthMappingsTbl)
{
  UInt32 depth;
  UInt32 supportedDepths;
  UInt16 i;


  // Get current depth...
  WinScreenMode (winScreenModeGet, 
	             NULL /*width*/, NULL /*height*/, 
				 &depth, NULL /*enableColor*/);

  // Get supported depths...
  WinScreenMode (winScreenModeGetSupportedDepths, 
				 NULL /*width*/, NULL /*height*/, 
				 &supportedDepths, NULL /*enableColor*/);

  for (i = 0; i < numMappings; i++)
    {
      const UInt32 fromDepths = depthMappingsTbl->fromDepths;
      const UInt16 toDepth = depthMappingsTbl->toDepth;

      if ((fromDepths      & (1 << (depth   - 1))) &&
          (supportedDepths & (1 << (toDepth - 1)))   )
        {
          depth = toDepth;

		  WinScreenMode (winScreenModeSet, 
						 NULL /*width*/, NULL /*height*/, 
						 &depth, NULL /*enableColor*/);	
          break;
        }
    }

  return depth;
}


/***************************************************************
 *  Function:    HsUtilSetScreenDepthFromRes
 *
 *  Summary:
 *    Calls HsUtilSetScreenDepth(), getting the depth mappings
 *	  from a resource of type 'dMap' with the given resource
 *	  ID.
 *
 *  Parameters:
 *	  resID	  IN  ID of a 'dMap' resource with the depth map
 *				  to use.
 *    
 *  Returns:
 *    The final depth after the function has run.
 *  
 *  Called By: 
 *    client
 *
 *  History:
 *    28-aug-2000 dia Created.
 *
 ****************************************************************/

UInt16
HsUtilSetScreenDepthFromRes (UInt16 resID)
{
  MemHandle resH;
  MemPtr resP = NULL;
  UInt16 numMappings = 0;
  UInt16 depth;

  resH = DmGetResource (hsUtilDepthMapRscType, resID);
  if (resH != 0)
	{
	  resP = MemHandleLock (resH);
	  numMappings = (UInt16) MemHandleSize (resH) / sizeof (HsUtilDepthMappingType);
	}

  depth = HsUtilSetScreenDepth (numMappings, (HsUtilDepthMappingType*)resP);

  if (resH)
	{
	  MemHandleUnlock (resH);
	  DmReleaseResource (resH);
	}

  return depth;
}
