/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PBkIOProgressPrv.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *             	  Private include file, Handling of the progress dialog for the 
 *				PhoneBookIO exchange library
 *
 *****************************************************************************/

#ifndef __PBKIO_PROGRESS_PRV_H__
#define __PBKIO_PROGRESS_PRV_H__

// Private includes
#include "PhoneBookIOLibPrv.h"
#include "PBkIOTransferPrv.h"

extern DmOpenRef	gApplicationDbP;	// Application Database

void 		PBkIOProgressStartDialog(PhoneBkIOLibGlobalsType *gP, Boolean socketStatusDisplayOff);
void 		PBkIOProgressStopDialog(PhoneBkIOLibGlobalsType *gP);
status_t 	PBkIOProgressProcessEvents(PhoneBkIOLibGlobalsType *gP, TelASyncEventInfoType * telASyncInfoP);
void 		PBkIOProgressUpdateDialog(PhoneBkIOLibGlobalsType *gP, status_t err, const char* messageP);

#endif // __PBKIO_PROGRESS_PRV_H__
