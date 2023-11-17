/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ExgMgrCompatibility.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Compatibility include file for old Exg system functions
 *
 *****************************************************************************/

#ifndef __EXGMGRCOMPATIBILITY_H__
#define __EXGMGRCOMPATIBILITY_H__

#include <PalmTypes.h>

#include <CmnErrors.h>
#include <DataMgr.h>
#include <Rect.h>
#include <ExgMgr.h>

//
// Compatibility Routines
//
#ifdef __cplusplus
extern "C" {
#endif

// This function was documented as System Use Only in 3.5, so no third party
// code should have been calling it.  So the addition of "V35" to the name
// should not affect anyone.
status_t ExgNotifyReceiveV35(ExgSocketType *socketP);

status_t ExgDBReadV40(ExgDBReadProcPtr readProcP, ExgDBDeleteProcPtr deleteProcP, void* userDataP, LocalID* dbIDP, uint16_t cardNo, Boolean* needResetP, Boolean keepDates);

status_t ExgDBWriteV40(ExgDBWriteProcPtr writeProcP, void* userDataP, const char* nameP, LocalID dbID, uint16_t cardNo);

#ifdef __cplusplus 
}
#endif

#endif  // __EXGMGRCOMPATIBILITY_H__
