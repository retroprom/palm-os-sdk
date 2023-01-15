/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrPrefs.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRPREFS_H
#define ADDRPREFS_H

#include <Form.h>
#include <Event.h>
#include "AddressDB.h"


/************************************************************
 * Function Prototypes
 *************************************************************/

void	PrefsLoad(AddrAppInfoPtr appInfoPtr);
void	PrefsSave(void);
Boolean	PrefsHandleEvent (EventType * event);

#endif // ADDRPREFS_H
