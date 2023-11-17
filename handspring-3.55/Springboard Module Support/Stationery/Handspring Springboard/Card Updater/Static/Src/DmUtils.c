/***********************************************************************
 *
 *	Copyright (c) Handspring 1999 -- All Rights Reserved
 *
 * PROJECT:  Utilities
 * FILE:     DmUtils.c
 * AUTHOR:	 Douglas Anderson: Oct 11, 1999
 *
 * DESCRIPTION:
 *	  Utilities for database manager stuff.
 *
 *	  11-Apr-2000	vmk	  Updated for new PalmOS SDK (PalmOS.h
 *						   instead of Pilot.h, etc.).
 *
 **********************************************************************/


//#include <Pilot.h>
#include <PalmOS.h>				// all the system toolbox headers
#include <CoreCompatibility.h>

#include "DmUtils.h"


/***************************************************************
 *  Function:    HsUtilDmFindDatabaseMulticard 
 * 
 *  Summary:     
 *    A version of DmFindDatabase that is multicard.  Finds the
 *    database with the given name that has the greatest version,
 *    searching all cards.  If there's a tie, the lower numbered
 *    card wins.
 * 
 *  Parameters:
 *    nameP   IN  The name of the database to search for.
 *    cardNoP OUT The card number that the database was found on.
 *                Untouched upon err.
 *    dbIdP   OUT The local id of the found database.  Untouched
 *                upon err.
 *     
 *  Returns: 
 *    dmErrCantFind if no matches were found; 0 upon success.
 *  
 *  Called By:  
 *    client
 * 
 *  History: 
 *    08-oct-99 dia Created...
 ****************************************************************/

Err 
HsUtilDmFindDatabaseMulticard (CharPtr nameP, UInt16* cardNoP, LocalID* dbIdP)
{
  UInt16 bestCardNo = -1;
  LocalID bestDbId = 0;
  UInt16 bestVersion = 0;
  UInt16 currCardNo;
  const UInt16 numCards = MemNumCards();
  
  // Search over all memory cards...
  for (currCardNo = 0; currCardNo < numCards; currCardNo++)
    {
      LocalID currDbId;

      currDbId = DmFindDatabase (currCardNo, nameP);
      if (currDbId != 0)
        {
          UInt16 currVersion;
      
          DmDatabaseInfo (currCardNo, currDbId, NULL, NULL, &currVersion, 
                          NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

          // We have a new winner if it's got a higher version.  Since we start on lower
          // cards (RAM), those will win if there is a tie.
          if ((bestDbId == 0) || (currVersion > bestVersion))
            {
              bestCardNo = currCardNo;
              bestDbId = currDbId;
              bestVersion = currVersion;
            }
        }
    }

  if (bestDbId == 0)
    {
      return dmErrCantFind;
    }
  else
    {
      *cardNoP = bestCardNo;
      *dbIdP = bestDbId;
      return 0;
    }
}


