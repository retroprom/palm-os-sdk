/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DataMgrCompatibility.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Data Manager Deprecated APIs (suffix V50) Compatibility Header.
 *
 *****************************************************************************/

#ifndef _DmCompatibility_h__
#define _DmCompatibility_h__
#include <DataMgr.h>

#ifndef _DATAMGR_H_
#	error "You must include DataMgr.h before including DataMgrCompatibility.h"
#else
#	define DmCreateDatabase						DmCreateDatabaseV50
#	define DmCreateDatabaseFromImage			DmCreateDatabaseFromImageV50
#	define DmDatabaseInfo						DmDatabaseInfoV50
#	define DmDatabaseProtect					DmDatabaseProtectV50
#	define DmDatabaseSize						DmDatabaseSizeV50
#	define DmDeleteDatabase						DmDeleteDatabaseV50
#	define DmFindDatabase						DmFindDatabaseV50
#	define DmGet1Resource						DmGet1ResourceV50
#	define DmGetAppInfoID						DmGetAppInfoIDV50
#	define DmGetDatabase						DmGetDatabaseV50
#	define DmGetNextDatabaseByTypeCreator		DmGetNextDatabaseByTypeCreatorV50
#	define DmGetResource						DmGetResourceV50
#	define DmNextOpenDatabase					DmNextOpenDatabaseV50
#	define DmNextOpenResDatabase				DmNextOpenResDatabaseV50
#	define DmNumDatabases						DmNumDatabasesV50
#	define DmOpenDatabase						DmOpenDatabaseV50
#	define DmOpenDatabaseByTypeCreator			DmOpenDatabaseByTypeCreatorV50
#	define DmOpenDatabaseInfo					DmOpenDatabaseInfoV50
#	define DmOpenDBNoOverlay					DmOpenDBNoOverlayV50
#	define DmRecordInfo							DmRecordInfoV50
#	define DmResourceInfo						DmResourceInfoV50
#	define DmSetDatabaseInfo					DmSetDatabaseInfoV50
#	define DmSetRecordInfo						DmSetRecordInfoV50
#	define DmWriteCheck							DmWriteCheckV50

#	define DmFindSortPosition					DmGetRecordSortPosition
#	define DmGetResourceIndex					DmGetResourceByIndex
#	define DmPositionInCategory					DmGetPositionInCategory
#	define DmSearchRecord						DmSearchRecordOpenDatabases
#	define DmSearchResource						DmSearchResourceOpenDatabases
#	define DmSeekRecordInCategory				DmFindRecordByOffsetInCategory

#	define DmComparF							DmCompareFunctionType
#	define DmResID								DmResourceID
#	define DmResType							DmResourceType
#	define SortRecordInfoType					DmSortRecordInfoType

#endif

#endif // _DmCompatibility_h__
