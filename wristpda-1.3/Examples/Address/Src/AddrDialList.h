/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrDialList.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRDIALLIST_H
#define ADDRDIALLIST_H

#include <Event.h>

#include "AddressDB.h"


/************************************************************
 * Function Prototypes
 *************************************************************/

Boolean DialListShowDialog( UInt16 recordIndex, UInt16 phoneIndex, UInt16 lineIndex );
Boolean DialListHandleEvent( EventType * event );


#endif // ADDRDIALLIST_H
