/******************************************************************************
 *
 * Copyright (c) 1994-2004 PalmSource, Inc. All rights reserved.
 *
 * File: DataMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header for the Data Manager
 *
 *****************************************************************************/

#ifndef _DATAMGR_H_
#define _DATAMGR_H_


// Include elementary types
#include <PalmTypes.h>					// Basic types

// Other headers we depend on
#include <MemoryMgr.h>
#include <azm.h>
#include <LocaleMgrTypes.h>


#define appInfoStringsRsc					'tAIS'

// Resources types definitions
typedef uint32_t	DmResourceType;
typedef uint16_t	DmResourceID;

/************************************************************
 * Category equates
 *************************************************************/
#define	dmRecAttrCategoryMask		( (uint8_t) 0x0F )	// mask for category #
#define	dmRecNumCategories			16					// number of categories
#define	dmCategoryLength			16					// 15 chars + 1 null terminator

#define dmAllCategories				( (uint8_t) 0xFF )
#define dmUnfiledCategory			0

// Max. record index is one less than the maximum number of
// records within a database, which can be at most 0xFFFF.
#define	dmMaxRecordIndex			( (uint16_t) 0xFFFE )
#define dmInvalidRecIndex			( (uint16_t) -1 )		// 0xFFFF
/************************************************************
 * Database Header equates
 *************************************************************/
#define	dmDBNameLength				32			// 31 chars + 1 null terminator

// Attributes of a Database
//
// *** IMPORTANT:
// ***
// *** Any changes to database attributes must be reflected in dmAllHdrAttrs and dmSysOnlyHdrAttrs ***
// ***
#define	dmHdrAttrResDB				0x0001	// Resource database
#define	dmHdrAttrReadOnly			0x0002	// Read Only database
#define	dmHdrAttrAppInfoDirty		0x0004	// Set if Application Info block is dirty
															// Optionally supported by an App's conduit
#define	dmHdrAttrBackup				0x0008	//	Set if database should be backed up to PC if
															//	no app-specific synchronization conduit has
															//	been supplied.
#define	dmHdrAttrOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
															//  for it to install a newer version of this database
															//  with a different name if the current database is
															//  open. This mechanism is used to update the
															//  Graffiti Shortcuts database, for example.
#define	dmHdrAttrResetAfterInstall	0x0020 	// Device requires a reset after this database is
															// installed.
#define	dmHdrAttrCopyPrevention		0x0040	// This database should not be copied to

#define	dmHdrAttrStream				0x0080	// This database is used for file stream implementation.
#define	dmHdrAttrHidden				0x0100	// This database should generally be hidden from view
															//  used to hide some apps from the main view of the
															//  launcher for example.
															// For data (non-resource) databases, this hides the record
															//	 count within the launcher info screen.
#define	dmHdrAttrLaunchableData		0x0200	// This data database (not applicable for executables)
															//  can be "launched" by passing it's name to it's owner
															//  app ('appl' database with same creator) using
															//  the sysAppLaunchCmdOpenNamedDB action code.

#define	dmHdrAttrRecyclable			0x0400	// This database (resource or record) is recyclable:
															//  it will be deleted Real Soon Now, generally the next
															//  time the database is closed.

#define	dmHdrAttrBundle				0x0800	// This database (resource or record) is associated with
															// the application with the same creator. It will be beamed
															// and copied along with the application.

#define	dmHdrAttrOpen				0x8000	// Database not closed properly
#define	dmHdrAttrSchema				0x1000	// Set if database is a schema database (i.e. a DB database)
#define	dmHdrAttrSecure				0x2000	// Set if database is a secure database

// overloaded header attributes...

// Set if database is a non-schema rec/res db as opposed to a classic (used/created by a 68K
// emulated app) database. This attribute is otherwise only ever used for schema databases.
#define	dmHdrAttrExtendedDB		dmHdrAttrSecure


// All database atributes (for error-checking)
#define	dmAllHdrAttrs				(	dmHdrAttrResDB |				\
										dmHdrAttrReadOnly |				\
										dmHdrAttrAppInfoDirty |			\
										dmHdrAttrBackup |				\
										dmHdrAttrOKToInstallNewer |		\
										dmHdrAttrResetAfterInstall |	\
										dmHdrAttrCopyPrevention |		\
										dmHdrAttrStream |				\
										dmHdrAttrHidden |				\
										dmHdrAttrLaunchableData |		\
										dmHdrAttrRecyclable |			\
										dmHdrAttrBundle |				\
										dmHdrAttrSchema |				\
										dmHdrAttrSecure |				\
										dmHdrAttrOpen	)

// Database attributes which only the system is allowed to change (for error-checking)
#define	dmSysOnlyHdrAttrs			(	dmHdrAttrResDB |		\
										dmHdrAttrSchema |		\
										dmHdrAttrSecure |		\
										dmHdrAttrOpen	)

// Record Attributes for record/resource databases
//
// *** IMPORTANT:
// ***
// *** Any changes to record attributes must be reflected in dmAllRecAttrs and dmSysOnlyRecAttrs ***
// ***
// *** Only one nibble is available for record attributes
//
// *** ANY CHANGES MADE TO THESE ATTRIBUTES MUST BE REFLECTED IN DESKTOP LINK
// *** SERVER CODE (DLCommon.h, DLServer.c)
#define	dmRecAttrDelete				0x80	// delete this record next sync
#define	dmRecAttrDirty				0x40	// archive this record next sync
#define	dmRecAttrBusy				0x20	// record currently in use
#define	dmRecAttrSecret				0x10	// "secret" record - password protected

// All record atributes (for error-checking)
#define	dmAllRecAttrs				( dmRecAttrDelete 	|	\
									  dmRecAttrDirty 	|	\
									  dmRecAttrBusy 	|	\
									  dmRecAttrSecret )

#define	dmSysOnlyRecAttrs			( dmRecAttrBusy )


/************************************************************
 * Unique ID equates
 *************************************************************/
#define	dmRecordIDReservedRange		1			// The range of upper bits in the database's
															// uniqueIDSeed from 0 to this number are
															// reserved and not randomly picked when a
															// database is created.
#define	dmDefaultRecordsID			0			// Records in a default database are copied
															// with their uniqueIDSeeds set in this range.
#define	dmUnusedRecordID			0			// Record ID not allowed on the device


/************************************************************
 * Mode flags
 *************************************************************/
typedef uint16_t					DmOpenModeType;
#define	dmModeReadOnly				((DmOpenModeType)0x0001)		// read  access
#define	dmModeWrite					((DmOpenModeType)0x0002)		// write access
#define	dmModeReadWrite				((DmOpenModeType)0x0003)		// read & write access
#define	dmModeReserved1				((DmOpenModeType)0x0004)		// not currently used
#define	dmModeExclusive				((DmOpenModeType)0x0008)		// don't let anyone else open it
#define	dmModeShowSecret			((DmOpenModeType)0x0010)		// force show of secret records


/************************************************************
 * Database manager error codes
 * the constant dmErrorClass is defined in ErrorBase.h
 *************************************************************/
#define	dmErrMemError				(dmErrorClass | 1)
#define	dmErrIndexOutOfRange		(dmErrorClass | 2)
#define	dmErrInvalidParam			(dmErrorClass | 3)
#define	dmErrReadOnly				(dmErrorClass | 4)
#define	dmErrDatabaseOpen			(dmErrorClass | 5)
#define	dmErrCantOpen				(dmErrorClass | 6)
#define	dmErrCantFind				(dmErrorClass | 7)
#define	dmErrRecordInWrongCard		(dmErrorClass | 8)
#define	dmErrCorruptDatabase		(dmErrorClass | 9)
#define	dmErrRecordDeleted			(dmErrorClass | 10)
#define	dmErrRecordArchived			(dmErrorClass | 11)
#define	dmErrNotRecordDB			(dmErrorClass | 12)
#define	dmErrNotResourceDB			(dmErrorClass | 13)
#define	dmErrROMBased				(dmErrorClass | 14)
#define	dmErrRecordBusy				(dmErrorClass | 15)
#define	dmErrResourceNotFound		(dmErrorClass | 16)
#define	dmErrNoOpenDatabase			(dmErrorClass | 17)
#define	dmErrInvalidCategory		(dmErrorClass | 18)
#define	dmErrNotValidRecord			(dmErrorClass | 19)
#define	dmErrWriteOutOfBounds		(dmErrorClass | 20)
#define	dmErrSeekFailed				(dmErrorClass | 21)
#define	dmErrAlreadyOpenForWrites	(dmErrorClass | 22)
#define	dmErrOpenedByAnotherTask	(dmErrorClass | 23)
#define dmErrUniqueIDNotFound		(dmErrorClass | 24)
#define dmErrAlreadyExists			(dmErrorClass | 25)
#define	dmErrInvalidDatabaseName	(dmErrorClass | 26)
#define	dmErrDatabaseProtected		(dmErrorClass | 27)
#define	dmErrDatabaseNotProtected	(dmErrorClass | 28)
#define	dmErrInvalidIndex			(dmErrorClass | 29)
#define	dmErrInvalidID				(dmErrorClass | 30)
#define	dmErrUnknownLocale			(dmErrorClass | 31)
#define	dmErrBadOverlayDBName		(dmErrorClass | 32)
#define dmErrBaseRequiresOverlay	(dmErrorClass | 33)

#define	dmErrSchemaBase				(dmErrorClass | 34)
#define	dmErrNotSchemaDatabase		(dmErrorClass | 35)
#define dmErrNotSecureDatabase		(dmErrorClass | 36)
#define dmErrAccessDenied			(dmErrorClass | 37)
#define dmErrInvalidSchemaDefn		(dmErrorClass | 38)

#define dmErrInvalidColSpec			(dmErrorClass | 40)
#define dmErrInvalidColType			(dmErrorClass | 41)
#define dmErrBufferNotLargeEnough	(dmErrorClass | 42)
#define dmErrColumnIndexOutOfRange	(dmErrorClass | 43)
#define dmErrInvalidColumnID		(dmErrorClass | 44)

#define dmErrColumnIDAlreadyExists	(dmErrorClass | 46)
#define dmErrSchemaIndexOutOfRange	(dmErrorClass | 47)
#define dmErrNoColumnData			(dmErrorClass | 48)
#define dmErrReadOutOfBounds		(dmErrorClass | 49)
#define dmErrInvalidVectorType		(dmErrorClass | 50)
#define dmErrInvalidSizeSpec		(dmErrorClass | 51)

#define dmErrNoData					(dmErrorClass | 53)
#define dmErrEncryptionFailure		(dmErrorClass | 54)

#define dmErrInvalidPropID			(dmErrorClass | 56)
#define dmErrNoCustomProperties		(dmErrorClass | 57)
#define dmErrBuiltInProperty		(dmErrorClass | 58)
#define dmErrDeviceLocked			(dmErrorClass | 59)
#define dmErrInvalidOperation		(dmErrorClass | 60)
#define dmErrTableNotEmpty			(dmErrorClass | 61)
#define dmErrOneOrMoreFailed		(dmErrorClass | 62)
#define dmErrCursorBOF				(dmErrorClass | 63)
#define dmErrCursorEOF				(dmErrorClass | 64)
#define dmErrInvalidSortIndex		(dmErrorClass | 65)
#define dmErrInvalidPrimaryKey		(dmErrorClass | 66)
#define dmErrSortDisabled			(dmErrorClass | 67)
#define dmErrNoUserPassword			(dmErrorClass | 68)
#define dmErrTableNameAlreadyExists (dmErrorClass | 69)
#define dmErrColumnNameAlreadyExists (dmErrorClass | 70)
#define dmErrInvalidSortDefn		(dmErrorClass | 71)
#define dmErrNoMoreData				(dmErrorClass | 72)
#define dmErrOperationAborted		(dmErrorClass | 73)

#define dmErrCategoryLimitReached	(dmErrorClass | 74)
#define dmErrColumnPropertiesLocked		(dmErrorClass | 75)
#define dmErrColumnDefinitionsLocked	(dmErrorClass | 76)
#define dmErrSQLParseError			(dmErrorClass | 77)
#define dmErrInvalidTableName		(dmErrorClass | 78)
#define dmErrInvalidColumnName		(dmErrorClass | 79)

#define dmErrInvalidStore			(dmErrorClass | 80)


/************************************************************
 * Values for the direction parameter of DmFindRecordByOffsetInCategory
 *************************************************************/
#define dmSeekForward				 1
#define dmSeekBackward				-1


/********************
Search sorted options
********************/
// Generic type used to represent an open Database
#if !defined(DmOpenRef)
	typedef	struct _opaque*	DmOpenRef;
#endif

typedef int32_t CategoryID;


/************************************************************
 * Structure passed to DmGetNextDatabaseByTypeCreator and used
 *  to cache search information between multiple searches.
 *************************************************************/
typedef struct {
	uint32_t		info[8];
	} DmSearchStateType;
typedef DmSearchStateType*	DmSearchStatePtr;

// Value to pass to DmGetNextDatabaseByTypeCreator to indicate
// wildcard matching for either the creator or the type, or
// both.
#define dmSearchWildcardID	((uint32_t)0)


 /************************************************************
 * Structures used by the sorting routines
 *************************************************************/
typedef struct {
	uint8_t			attributes;				// record attributes;
	uint8_t			uniqueID[3];			// unique ID of record
} DmSortRecordInfoType;
typedef DmSortRecordInfoType *DmSortRecordInfoPtr;

// *****************************************************************************
// *
// * FUNCTION: DmCompareFunctionType
// *
// * DESCRIPTION: Application-defined comparison function.
// *			  Compares two records in a database.
// *
// * PARAMETERS:
// *			> rec1P, rec2P: Pointers to the two records to compare
// * 			> other: Custom information to pass to the comparison function
// *			> rec1SortInfoP, rec2SortInfoP: Pointers to DmSortRecordInfoType
// *			  structures that specify unique sorting information for the
// *			  two records.
// *			> appInfoH: Handle to the database application info block
// *
// * RETURNS:	- 0 if rec1P == res2P
// *			- < 0 if rec1P < res2P
// *			- > 0 if rec1P > res2P
// *
// *****************************************************************************
typedef int16_t DmCompareFunctionType(void*				rec1P,
									void*				rec2P,
									int16_t				other,
									DmSortRecordInfoPtr rec1SortInfoP,
									DmSortRecordInfoPtr rec2SortInfoP,
									MemHandle			appInfoH);


/*************************************************************
 * Serialization APIs Structures (Backup & Restore)
 *************************************************************/
typedef struct DmBackupRestoreStateTag
{
	uint32_t	info[12];
} DmBackupRestoreStateType;

typedef DmBackupRestoreStateType*			DmBackupRestoreStatePtr;

/*************************************************************
 * DatabaseID type
 *************************************************************/
typedef uint32_t	DatabaseID;

typedef struct DmDatabaseInfoTag
{
	uint32_t		size;				// Size of this structure
	char*		pName;
	char*		pDispName;			// Schema databases only
	uint16_t*		pAttributes;
	uint16_t*		pVersion;
	uint32_t*		pType;
	uint32_t*		pCreator;
	uint32_t*		pCrDate;
	uint32_t*		pModDate;
	uint32_t*		pBckpDate;
	uint32_t*		pModNum;
	MemHandle*	pAppInfoHandle; 	// Record/Resource databases only
	MemHandle*	pSortInfoHandle; 	// Record/Resource databases only
	uint16_t*		pEncoding;			// Schema databases only
} DmDatabaseInfoType;

typedef DmDatabaseInfoType*			DmDatabaseInfoPtr;

typedef struct DmStorageInfoTag
{
	uint32_t		size;				// Size of this structure.
	uint32_t		bytesTotal;			// Total amount of memory available for persistent storage.
	uint32_t		bytesNonSecureUsed;	// Amount of memory used in non-secure storage.
	uint32_t		bytesNonSecureFree;	// Amount of free memory in non-secure storage.
	uint32_t		bytesSecureUsed;	// Amount of memory used in secure storage.
	uint32_t		bytesSecureFree;	// Amount of free memory in secure storage.
	uint32_t		bytesFreePool;		// Amount of memory in the free pool avaialble for both
									//   non-secure storage and secure storage.
} DmStorageInfoType;
typedef DmStorageInfoType*			DmStorageInfoPtr;

/************************************************************
 * Data Manager procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// Directory APIs
// *************************************************************************
typedef uint32_t			DmFindType;

#define dmFindSchemaDB		((DmFindType)0x00000001)
#define dmFindExtendedDB	((DmFindType)0x00000002)
#define dmFindClassicDB		((DmFindType)0x00000004)

#define dmFindAllDB			(dmFindSchemaDB | dmFindExtendedDB | dmFindClassicDB)

// find schema/non-schema dbs (or both) together with info for the found db (optional)...
DatabaseID			DmFindDatabase(const char* nameP,
							uint32_t creator,
							DmFindType find,
							DmDatabaseInfoPtr databaseInfoP);

DatabaseID			DmFindDatabaseByTypeCreator(uint32_t type,
							uint32_t creator,
							DmFindType find,
							DmDatabaseInfoPtr databaseInfoP);

// iterate over schema/non-schema dbs (or both) and (optionally) retrieve the db info...
status_t			DmOpenIteratorByTypeCreator(DmSearchStatePtr stateInfoP,
							uint32_t type,
							uint32_t creator,
							Boolean onlyLatestVers,
							DmFindType find);

status_t			DmGetNextDatabaseByTypeCreator(DmSearchStatePtr stateInfoP,
							DatabaseID*	 dbIDP,
							DmDatabaseInfoPtr databaseInfoP);

status_t			DmCloseIteratorByTypeCreator(DmSearchStatePtr stateInfoP);


uint16_t			DmNumDatabases(void);

status_t			DmDatabaseInfo(DatabaseID dbID, DmDatabaseInfoPtr pDatabaseInfo);

status_t			DmSetDatabaseInfo(DatabaseID dbID, DmDatabaseInfoPtr pDatabaseInfo);

status_t			DmDatabaseSize(DatabaseID dbID,
						   uint32_t* numRecordsP,
						   uint32_t* totalBytesP,
						   uint32_t* dataBytesP);

// Previously known as DmDatabaseProtect
status_t			DmSetDatabaseProtection(DatabaseID dbID, Boolean protect);

status_t			DmGetAppInfo(DmOpenRef dbRef, MemHandle* pAppInfoHandle);

// Previously known as DmOpenDatabaseInfo
status_t			DmGetOpenInfo(DmOpenRef dbRef,
							DatabaseID*	pDbID,
							uint16_t* pOpenCount,
							DmOpenModeType* pOpenMode,
							Boolean* pResDB);

status_t			DmGetStorageInfo(DmStorageInfoPtr pStorageInfo);

status_t			DmCreateDatabase(const char *nameP,
							uint32_t creator,
							uint32_t type,
							Boolean resDB);

status_t			DmDeleteDatabase(DatabaseID dbID);

status_t			DmCreateDatabaseFromImage(MemPtr pImage, DatabaseID* pDbID);


// Dm Database APIs
// *************************************************************************

DmOpenRef DmOpenDatabase(DatabaseID dbID, DmOpenModeType mode);

DmOpenRef DmOpenDBNoOverlay(DatabaseID dbID, DmOpenModeType mode);

DmOpenRef DmOpenDatabaseByTypeCreator(uint32_t type, uint32_t creator, DmOpenModeType mode);

status_t DmCloseDatabase(DmOpenRef dbRef);

status_t DmGetLastErr(void);


// Dm Record APIs
// *************************************************************************

status_t DmAttachRecord(DmOpenRef dbRef, uint16_t* pIndex, MemHandle hNewRecord,
					MemHandle*	hReplacedRecord );

status_t DmDetachRecord(DmOpenRef dbRef, uint16_t index, MemHandle* hDetached);

MemHandle DmNewHandle(DmOpenRef dbRef, uint32_t size);

MemHandle DmNewRecord(DmOpenRef dbRef, uint16_t* atP, uint32_t size);

MemHandle DmResizeRecord(DmOpenRef dbRef, uint16_t index, uint32_t newSize);

uint16_t DmNumRecords(DmOpenRef dbRef);

status_t DmWrite(void *pRecord, uint32_t offset, const void *pSrc, uint32_t bytes);

MemHandle DmQueryRecord(DmOpenRef dbRef, uint16_t index);

MemHandle DmGetRecord(DmOpenRef dbRef, uint16_t index);

status_t DmReleaseRecord(DmOpenRef dbRef, uint16_t index, Boolean fDirty);

status_t DmDeleteRecord(DmOpenRef dbRef, uint16_t index);

status_t DmRemoveRecord(DmOpenRef dbRef, uint16_t index);

status_t DmStrCopy(void *pRecord, uint32_t offset, const void *pSrc);

status_t DmSet(void *pRecord, uint32_t offset, uint32_t bytes, uint8_t value);

status_t DmRemoveSecretRecords(DmOpenRef dbRef);

status_t DmArchiveRecord(DmOpenRef dbRef, uint16_t index);

status_t DmMoveRecord(DmOpenRef dbRef, uint16_t from, uint16_t to);

status_t DmDeleteCategory(DmOpenRef dbRef, uint16_t categoryNum);

status_t DmMoveCategory(DmOpenRef dbRef, uint16_t toCategory,
					uint16_t fromCategory, Boolean fDirty);

MemHandle DmQueryNextInCategory(DmOpenRef dbRef, uint16_t *pIndex, uint16_t category);

uint16_t DmNumRecordsInCategory(DmOpenRef dbRef, uint16_t category);

status_t DmGetRecordAttr(DmOpenRef dbRef, uint16_t index, uint8_t* pAttr);

status_t DmGetRecordCategory(DmOpenRef dbRef, uint16_t index, uint8_t* pCategory);

status_t DmGetRecordID(DmOpenRef dbRef, uint16_t index, uint32_t* pUID);

status_t DmSetRecordAttr(DmOpenRef dbRef, uint16_t index, uint8_t* pAttr);

status_t DmSetRecordCategory(DmOpenRef dbRef, uint16_t index, uint8_t* pCategory);

status_t DmSetRecordID(DmOpenRef dbRef, uint16_t index, uint32_t* pUID);

// Previously known as DmPositionInCategory
uint16_t DmGetPositionInCategory(DmOpenRef dbRef, uint16_t index, uint16_t category);

// Previously known as DmSeekRecordInCategory
status_t DmFindRecordByOffsetInCategory(DmOpenRef		dbRef,
										uint16_t*		pIndex,
										uint16_t		offset,
										int16_t			direction,
										uint16_t		category );

status_t DmFindRecordByID(DmOpenRef dbRef, uint32_t uniqueID, uint16_t* pIndex);

// Previously known as DmSearchRecord
uint16_t DmSearchRecordOpenDatabases(MemHandle hRecord, DmOpenRef* pDbRef);

// Previously known as DmFindSortPosition
uint16_t DmGetRecordSortPosition(DmOpenRef 				dbRef,
								 void*					pNewRecord,
								 DmSortRecordInfoType*	pNewRecordInfo,
								 DmCompareFunctionType* pFuncCompar,
								 int16_t				other);

status_t DmInsertionSort(const DmOpenRef dbR, DmCompareFunctionType *compar, int16_t other);

status_t DmQuickSort(const DmOpenRef dbR, DmCompareFunctionType *compar, int16_t other);

// Resource APIs
// *************************************************************************

status_t DmAttachResource(DmOpenRef	dbRef,
					 MemHandle	hNewRes,
					 DmResourceType	resType,
					 DmResourceID	resID);

status_t DmDetachResource(DmOpenRef	dbRef,
					 uint16_t		index,
					 MemHandle*	hDetached);

MemHandle DmNewResource(DmOpenRef	dbRef,
						DmResourceType	resType,
						DmResourceID		resID,
						uint32_t		size);

status_t DmRemoveResource(DmOpenRef dbRef, uint16_t index);

MemHandle DmResizeResource(MemHandle hResource, uint32_t size);

MemHandle DmGetResource(DmOpenRef dbRef, DmResourceType resType, DmResourceID resID);

// Previously known as DmGetResourceIndex
MemHandle DmGetResourceByIndex(DmOpenRef dbRef, uint16_t index);

status_t DmReleaseResource(MemHandle hResource);

uint16_t DmNumResources(DmOpenRef dbRef);

uint16_t DmFindResource(DmOpenRef	dbRef,
					  DmResourceType	resType,
					  DmResourceID	resID,
					  MemHandle	hResource);

uint16_t DmFindResourceType(	DmOpenRef	dbRef,
							DmResourceType	resType,
							uint16_t		typeIndex );

// Previously known as DmSearchResource
uint16_t DmSearchResourceOpenDatabases(DmResourceType	resType,
									 DmResourceID	resID,
									 MemHandle	hResource,
									 DmOpenRef*	pDbRef);

status_t DmSetResourceInfo(DmOpenRef		dbRef,
					  uint16_t		index,
					  DmResourceType*	pResType,
					  DmResourceID*		pResID);

status_t DmResourceInfo(	DmOpenRef	dbRef,
					uint16_t		index,
					DmResourceType*	pResType,
					DmResourceID*	pResID,
					MemHandle*	pChunkHandle );


DmOpenRef DmNextOpenResDatabase(DmOpenRef dbRef);

DmOpenRef DmNextOpenDatabase(DmOpenRef dbRef);

status_t DmGetDatabaseLockState(DmOpenRef	dbRef,
								uint8_t*	pHighest,
								uint32_t*	pCount,
								uint32_t*	pBusy);

status_t DmResetRecordStates(DmOpenRef dbRef);


// Overlay APIs
// *************************************************************************

status_t DmGetOverlayDatabaseName(const char* baseDBName, const LmLocaleType* targetLocale, char* overlayDBName);

status_t DmGetOverlayDatabaseLocale(const char* overlayDBName, LmLocaleType* overlayLocale);

status_t DmGetOverlayLocale(LmLocaleType* overlayLocale);

status_t DmSetOverlayLocale(const LmLocaleType* overlayLocale);

status_t DmGetFallbackOverlayLocale(LmLocaleType* fallbackLocale);

status_t DmSetFallbackOverlayLocale(const LmLocaleType* fallbackLocale);


// Heap APIs
// *************************************************************************

MemPtr DmHandleLock(MemHandle handle);
status_t DmHandleUnlock(MemHandle handle);
status_t DmHandleFree(MemHandle handle);
uint32_t DmHandleSize(MemHandle handle);
status_t DmHandleResize(MemHandle handle, uint32_t newSize);
MemHandle DmRecoverHandle(MemPtr pChunk);
status_t DmPtrResize(MemPtr p, uint32_t newSize);
status_t DmPtrUnlock(MemPtr p);
uint32_t DmPtrSize(MemPtr p);


// Serialization APIs
// *************************************************************************

status_t DmBackupInitialize(DmBackupRestoreStatePtr	pState,
					   		DatabaseID				dbID);

status_t DmBackupUpdate(DmBackupRestoreStatePtr	pState,
				 		MemPtr					pBuffer,
						uint32_t*				pSize);

status_t DmBackupFinalize(DmBackupRestoreStatePtr	pState,
					 	  Boolean					fAbort);


status_t DmRestoreInitialize(DmBackupRestoreStatePtr pState,
							 DmDatabaseInfoPtr		pDbInfo);

status_t DmRestoreUpdate(DmBackupRestoreStatePtr	pState,
						 MemPtr						pBuffer,
						 uint32_t					size,
						 Boolean					endOfData,
						 Boolean*					pfDbInfoAvailable);

status_t DmRestoreFinalize(DmBackupRestoreStatePtr	pState,
						   Boolean					fAbort,
						   Boolean					fOverwrite,
						   DatabaseID*				pDbID);

status_t DmInitiateAutoBackupOfOpenDatabase(DmOpenRef dbRef);

// Deprecated APIs...
// *************************************************************************

status_t DmDeleteDatabaseV50(uint16_t cardNo, LocalID dbID);

uint16_t DmNumDatabasesV50(uint16_t cardNo);

LocalID	DmGetDatabaseV50(uint16_t cardNo, uint16_t index);

LocalID	DmFindDatabaseV50(uint16_t cardNo, const char *nameP);

MemHandle DmGetResourceV50(DmResourceType resType, DmResourceID resID);

MemHandle DmGet1ResourceV50(DmResourceType resType, DmResourceID resID);

status_t DmGetNextDatabaseByTypeCreatorV50(Boolean newSearch, DmSearchStatePtr stateInfoP,
					uint32_t type, uint32_t creator, Boolean onlyLatestVers,
				 	uint16_t *cardNoP, LocalID *dbIDP);

status_t DmDatabaseInfoV50(uint16_t cardNo, LocalID	dbID, char *nameP,
					uint16_t *attributesP, uint16_t *versionP, uint32_t *crDateP,
					uint32_t *modDateP, uint32_t *bckUpDateP,
					uint32_t *modNumP, LocalID *appInfoIDP,
					LocalID *sortInfoIDP, uint32_t *typeP,
					uint32_t *creatorP);

status_t DmSetDatabaseInfoV50(uint16_t cardNo, LocalID	dbID, const char *nameP,
					uint16_t *attributesP, uint16_t *versionP, uint32_t *crDateP,
					uint32_t *modDateP, uint32_t *bckUpDateP,
					uint32_t *modNumP, LocalID *appInfoIDP,
					LocalID *sortInfoIDP, uint32_t *typeP,
					uint32_t *creatorP);

status_t DmDatabaseSizeV50(uint16_t cardNo, LocalID dbID, uint32_t *numRecordsP,
					uint32_t *	totalBytesP, uint32_t *dataBytesP);

status_t DmDatabaseProtectV50(uint16_t cardNo, LocalID dbID, Boolean protect);

status_t DmCreateDatabaseV50(uint16_t cardNo, const char *nameP,
					uint32_t creator, uint32_t type, Boolean resDB);

DmOpenRef DmOpenDatabaseV50(uint16_t cardNo, LocalID dbID, DmOpenModeType mode);

DmOpenRef DmOpenDBNoOverlayV50(uint16_t cardNo, LocalID dbID, DmOpenModeType mode);

DmOpenRef DmOpenDatabaseByTypeCreatorV50(uint32_t type, uint32_t creator, DmOpenModeType mode);

status_t DmRecordInfoV50(DmOpenRef dbRef, uint16_t index,
					 uint16_t *pAttr, uint32_t *pUID, LocalID *pChunkID);

status_t DmSetRecordInfoV50(DmOpenRef dbRef, uint16_t index, uint16_t *pAttr, uint32_t *pUID);

status_t DmResourceInfoV50(	DmOpenRef	dbRef,
					uint16_t		index,
					DmResourceType*	pResType,
					DmResourceID*	pResID,
					LocalID*	pChunkLocalID );

LocalID DmGetAppInfoIDV50(DmOpenRef dbRef);

status_t DmOpenDatabaseInfoV50(	DmOpenRef	dbRef,
					LocalID*	pDbID,
					uint16_t*		pOpenCount,
					DmOpenModeType*		pMode,
					uint16_t*		pCardNo,
					Boolean*	pResDB );

status_t DmWriteCheckV50(void *pRecord, uint32_t offset, uint32_t bytes);

DmOpenRef DmNextOpenResDatabaseV50(DmOpenRef dbRef);

DmOpenRef DmNextOpenDatabaseV50(DmOpenRef dbRef);

status_t DmCreateDatabaseFromImageV50(MemPtr pImage);


#ifdef __cplusplus
}
#endif

#endif // _DATAMGR_H__
