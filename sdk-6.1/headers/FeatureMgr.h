/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FeatureMgr.h
 *
 * Release: Palm OS 6.0
 *
 * Description:
 *		Header for the Feature Manager
 *
 *****************************************************************************/

#ifndef __FEATUREMGR_H__
#define __FEATUREMGR_H__

#include <PalmTypes.h>
#include <CmnErrors.h>



/************************************************************
 * Feature manager error codes
 * the constant ftrErrorClass is defined in ErrorBase.h
 *************************************************************/
#define	ftrErrInvalidParam				(ftrErrorClass | 1)
#define	ftrErrNoSuchFeature				(ftrErrorClass | 2)
#define	ftrErrInternalErr				(ftrErrorClass | 5)


/************************************************************
 * Feature Manager procedures
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Get a feature
status_t		FtrGet(uint32_t creator, uint32_t featureNum, uint32_t *valueP);

// Set/Create a feature.
status_t		FtrSet(uint32_t creator, uint32_t featureNum, uint32_t newValue);

// Unregister a feature
status_t		FtrUnregister(uint32_t creator, uint32_t featureNum);

// Get a feature by index
status_t		FtrGetByIndex(uint16_t index, Boolean romTable, 
					  uint32_t *creatorP, uint16_t *numP, uint32_t *valueP);

// Get temporary space from storage heap
status_t		FtrPtrNew(uint32_t creator, uint32_t featureNum, size_t size,
				  void **newPtrP);

// Retrieve previously created feature pointer.
status_t		FtrPtrGet(uint32_t creator, uint32_t featureNum, size_t* sizeP,
				  void **newPtrP);

// Release temporary space to storage heap
status_t		FtrPtrFree(uint32_t creator, uint32_t featureNum);


// Resize block of temporary storage
status_t		FtrPtrResize(uint32_t creator, uint32_t featureNum, size_t newSize,
					 void **newPtrP);


#ifdef __cplusplus
}
#endif

#endif // __FEATUREMGR_H__
