/******************************************************************************
 *
 * Copyright (c) 2003-2004 PalmSource, Inc. All rights reserved.
 *
 * File: SchemaDatabases.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header for the Data Manager schema databases
 *
 *****************************************************************************/

#ifndef _SCHEMA_DATABASES_H_
#define _SCHEMA_DATABASES_H_

#include <PalmTypes.h>
#include <MemoryMgr.h>
#include <azm.h>
#include <LocaleMgrTypes.h>
#include <DataMgr.h>

/******************************************************************************
 * Row Attributes for schema databases
 *****************************************************************************/
#define dbRecAttrArchive			0x01
#define dbRecAttrReadOnly			0x02
#define	dbRecAttrSecret				0x10
#define	dbRecAttrDelete				0x80

#define	dbAllRecAttrs				(dbRecAttrDelete 	| \
									 dbRecAttrSecret	| \
									 dbRecAttrArchive	| \
									 dbRecAttrReadOnly)

// These are the attributes that can be seen with DbGetRowAttr()
// but cannot be set directly through a call to DbSetRowAttr()
#define	dbSysOnlyRecAttrs			(dbRecAttrDelete	| \
									 dbRecAttrArchive)


/******************************************************************************
 * Mode flags used when opening a database
 *****************************************************************************/

typedef uint16_t			DbShareModeType;
#define dbShareNone 		((DbShareModeType)0x0000)
#define dbShareRead 		((DbShareModeType)0x0001)
#define dbShareReadWrite	((DbShareModeType)0x0002)



/******************************************************************************
 * Action Types for use with access rules on secure databases
 *****************************************************************************/

#define dbActionRead		((AzmActionType)0x00000001)
#define dbActionWrite		((AzmActionType)0x00000002)
#define dbActionDelete		((AzmActionType)0x00000004)
#define dbActionBackup		((AzmActionType)0x00000008)
#define dbActionRestore		((AzmActionType)0x00000010)
#define dbActionEditSchema	((AzmActionType)0x00000020)


/******************************************************************************
 * Schema related constants
 *****************************************************************************/

#define dbDBNameLength				32
#define dbMaxRowIndex				0x00FFFFFEL

#define dbSchemaColDynamic			0x01
#define dbSchemaColNonSyncable		0x02
#define dbSchemaColWritable			0x04

#define dbAllSchemaColAttrs			(dbSchemaColDynamic | \
									 dbSchemaColNonSyncable | \
									 dbSchemaColWritable)

// Schema column types
typedef uint8_t						DbSchemaColumnType;

#define	dbVector					((DbSchemaColumnType)0x80)
#define	dbStringVector				((DbSchemaColumnType)0xC0)

#define	dbUInt8						((DbSchemaColumnType)0x01)
#define	dbUInt16					((DbSchemaColumnType)0x02)
#define	dbUInt32					((DbSchemaColumnType)0x03)
#define	dbUInt64					((DbSchemaColumnType)0x04)
#define	dbInt8						((DbSchemaColumnType)0x05)
#define	dbInt16						((DbSchemaColumnType)0x06)
#define	dbInt32						((DbSchemaColumnType)0x07)
#define	dbInt64						((DbSchemaColumnType)0x08)
#define	dbFloat						((DbSchemaColumnType)0x09)
#define	dbDouble					((DbSchemaColumnType)0x0A)
#define	dbBoolean					((DbSchemaColumnType)0x0B)
#define	dbDateTime					((DbSchemaColumnType)0x0C)
#define	dbDate						((DbSchemaColumnType)0x0D)
#define	dbTime						((DbSchemaColumnType)0x0E)
#define	dbChar						((DbSchemaColumnType)0x0F)
#define	dbVarChar					((DbSchemaColumnType)0x10)
#define	dbBlob						((DbSchemaColumnType)0x11)
#define	dbDateTimeSecs				((DbSchemaColumnType)0x12)

// Schema column (built-in) properties
typedef uint8_t						DbSchemaColumnProperty;
#define	dbColumnNameProperty		((DbSchemaColumnProperty)0x01)
#define	dbColumnDatatypeProperty	((DbSchemaColumnProperty)0x02)
#define	dbColumnSizeProperty		((DbSchemaColumnProperty)0x03)
#define	dbColumnAttribProperty		((DbSchemaColumnProperty)0x04)

#define dbColumnPropertyUpperBound	((DbSchemaColumnProperty)0x0A)

/******************************************************************************
 * Public schema database structures
 *****************************************************************************/

// schema column definitions
typedef struct
{
	uint32_t 				id;
	uint32_t				maxSize;
	char					name[dbDBNameLength];
	DbSchemaColumnType		type;
	uint8_t					attrib;
	uint16_t				reserved;
	status_t				errCode;
} DbSchemaColumnDefnType, * DbSchemaColumnDefnPtr;

// Schema definition struct
typedef struct
{
	char					name[dbDBNameLength];
	uint32_t				numColumns;
	DbSchemaColumnDefnType	* columnListP;
} DbTableDefinitionType;

// schema column data generic type
typedef void	DbSchemaColumnData;

typedef struct
{
	DbSchemaColumnData	* data;
	uint32_t			dataSize;
	uint32_t			columnID;
	uint32_t			columnIndex;
	status_t			errCode;
	uint32_t			reserved;
} DbSchemaColumnValueType, * DbSchemaColumnValuePtr;

// DbColumnPropertyValueTag: structure used to get/set a specific property.
typedef struct
{
	uint32_t				columnID;
	uint32_t				dataSize;
	void					* data ;
	status_t				errCode;
	DbSchemaColumnProperty	propertyID;
	uint8_t					padding[3];
} DbColumnPropertyValueType, * DbColumnPropertyValuePtr;

// DbColumnPropertySpecTag: structure used to specify properties for
// selective value retrieval.
typedef struct
{
	uint32_t 				columnID;
	DbSchemaColumnProperty	propertyID;
	uint8_t					padding[3];
} DbColumnPropertySpecType, * DbColumnPropertySpecPtr;



/******************************************************************************
 * Schema Databases Category Structures
 *****************************************************************************/
#define DbMaxRecordCategories	255

// Type for defining searches on category sets
// Note: All these modes are exclusive
typedef uint32_t	DbMatchModeType;

#define DbMatchAny 				((DbMatchModeType)1)
#define DbMatchAll 				((DbMatchModeType)2)
#define DbMatchExact			((DbMatchModeType)3)


/******************************************************************************
 ******************************************************************************
 * Function prototypes
 ******************************************************************************
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 * Creating databases
 *****************************************************************************/

status_t DbCreateDatabase(const char					* name,
					 	  uint32_t						creator,
					 	  uint32_t						type,
					 	  uint32_t						numTables,
					 	  const DbTableDefinitionType	schemaListP[],
					 	  DatabaseID					* dbIDP);


/******************************************************************************
 * Opening & closing databses
 *****************************************************************************/

DmOpenRef DbOpenDatabase(DatabaseID			dbID,
						 DmOpenModeType		mode,
						 DbShareModeType	share);

DmOpenRef DbOpenDatabaseByName(uint32_t			creator,
							   const char		* name,
							   DmOpenModeType	mode,
							   DbShareModeType	share);

status_t DbCloseDatabase(DmOpenRef dbRef);


/******************************************************************************
 * Cursors
 *****************************************************************************/

#define dbCursorIncludeDeleted	0x00000001		// Includes deleted and archived rows in the cursor
#define dbCursorOnlyDeleted		0x00000002		// The cursor will ONLY include deleted and archived rows
#define dbCursorOnlySecret		0x00000004		// The cursor will ONLY include secret rows
#define dbCursorEnableCaching	0x00010000		// Enables caching of row data locally in the cursor
#define dbCursorSortByCategory	0x10000000		// Sort by category mode -- rwos with multiple categories show up multiple times


#define dbInvalidCursorID		0x0
#define dbInvalidRowID			dbInvalidCursorID
#define dbCursorEOFPos			0xFFFFFFFE
#define dbCursorBOFPos			0xFFFFFFFF

// Fetch types, used with DbCursorMove
typedef enum {
	dbFetchRelative,
	dbFetchAbsolute,
	dbFetchNext,
	dbFetchPrior,
	dbFetchFirst,
	dbFetchLast,
	dbFetchRowID,
} DbFetchType;

// Cursor creation
/* This function opens a cursor containing all rows in the table that match the
 * given select clause with no filtering performed on categories */
status_t DbCursorOpen(DmOpenRef			dbRef,
					  const char		* sql,
					  uint32_t			flags,
					  uint32_t			* cursorID);

/* This function opens a cursor containing all rows in the table that
 * match the given select clause and categories. */
status_t DbCursorOpenWithCategory(DmOpenRef			dbRef,
								  const char		* sql,
								  uint32_t			flags,
								  uint32_t			numCategories,
								  const CategoryID	categoryIDs[],
								  DbMatchModeType	matchMode,
								  uint32_t			* cursorID);

status_t DbCursorFlushCache(uint32_t cursorID);

// ODBC-like functions
status_t DbCursorClose(uint32_t cursorID);

uint32_t DbCursorGetRowCount(uint32_t cursorID);

/* Cursor positions are 1-based */
status_t DbCursorMove(uint32_t cursorID, int32_t offset, DbFetchType fetchType);

/* Utility "functions" */
#define DbCursorMoveNext(i)					DbCursorMove(i, 0x0, dbFetchNext)
#define DbCursorMovePrev(i)					DbCursorMove(i, 0x0, dbFetchPrior)
#define DbCursorMoveFirst(i)				DbCursorMove(i, 0x0, dbFetchFirst)
#define DbCursorMoveLast(i)					DbCursorMove(i, 0x0, dbFetchLast)
#define DbCursorMoveToRowID(i, r)			DbCursorMove(i, r, dbFetchRowID)
#define DbCursorSetAbsolutePosition(i, o)	DbCursorMove(i, o, dbFetchAbsolute)

Boolean DbCursorIsEOF(uint32_t cursorID);
Boolean DbCursorIsBOF(uint32_t cursorID);
Boolean DbCursorIsDeleted(uint32_t cursorID);

/* Refresh the cursor contents. If they change the cursor is positioned at
 * the first entry */
status_t DbCursorRequery(uint32_t cursorID);

/* Commit bound data buffers to the database */
status_t DbCursorUpdate(uint32_t cursorID);

// Palm OS specific functions
/* Each time the cursor is scrolled all "bound" variables are updated */
status_t DbCursorBindData(uint32_t		cursorID,
						  uint32_t 		columnID,
						  void			* dataBufferP,
						  uint32_t		dataBufferLength,
						  uint32_t		* dataSizeP,
						  status_t		* errCodeP);

status_t DbCursorBindDataWithOffset(uint32_t 	cursorID,
			    					uint32_t 	columnID,
			    					void		* dataBufferP,
			    					uint32_t	dataBufferLength,
									uint32_t	* dataSizeP,
									uint32_t	fieldDataOffset,
			    					status_t	* errCodeP);

status_t DbCursorGetCurrentPosition(uint32_t cursorID, uint32_t * position);
status_t DbCursorGetCurrentRowID(uint32_t cursorID, uint32_t * rowIDP);

status_t DbCursorGetRowIDForPosition(uint32_t	cursorID,
									 uint32_t	position,
									 uint32_t	* rowIDP);

status_t DbCursorGetPositionForRowID(uint32_t	cursorID,
									 uint32_t	rowID,
									 uint32_t	* positionP);

/* Retreive the schema that describes the cursor, with any projections taken into account */
status_t DbCursorGetSchema(uint32_t					cursorID,
						   DbTableDefinitionType	** schemaPP);

/* If the cursor is open on the default sort index, move a row in it */
status_t DbCursorRelocateRow(uint32_t cursorID, uint32_t from, uint32_t to);

// Row destruction
/* Delete all rows contianed in the cursor */
status_t DbCursorDeleteAllRows(uint32_t cursorID);

/* Remove all rows contained in the cursor */
status_t DbCursorRemoveAllRows(uint32_t cursorID);

/* Archive all rows contained in the cursor */
status_t DbCursorArchiveAllRows(uint32_t cursorID);


/******************************************************************************
 * Row management
 *****************************************************************************/

status_t DbInsertRow(DmOpenRef					dbRef,
					 const char					* table,
					 uint32_t					numColumnValues,
					 DbSchemaColumnValueType	* columnValuesP,
					 uint32_t					* rowIDP);

status_t DbRemoveRow(DmOpenRef dbRef, uint32_t rowID);

status_t DbDeleteRow(DmOpenRef dbRef, uint32_t rowID);

status_t DbArchiveRow(DmOpenRef dbRef, uint32_t rowID);

status_t DbRemoveSecretRows(DmOpenRef dbRef);

status_t DbGetRowAttr(DmOpenRef dbRef, uint32_t rowID, uint16_t * attrP);

status_t DbSetRowAttr(DmOpenRef dbRef, uint32_t rowID, uint16_t * attrP);

status_t DbGetTableForRow(DmOpenRef dbRef, uint32_t rowID, char * buf, size_t bufSize);

Boolean DbIsRowID(uint32_t uniqueID);

Boolean DbIsCursorID(uint32_t uniqueID);

/******************************************************************************
 * Reading & writing data
 *****************************************************************************/

// Write data to a row
status_t DbWriteColumnValues(DmOpenRef					dbRef,
							 uint32_t					rowID,
							 uint32_t					numColumnValues,
							 DbSchemaColumnValueType	* columnValuesP);

status_t DbWriteColumnValue(DmOpenRef			dbRef,
							uint32_t			rowID,
							uint32_t 			columnID,
							uint32_t 			offset,
							int32_t				bytesToReplace,
							const void			* srcP,
							uint32_t			srcBytes);

// Copy the data to a provided buffer owned by the caller
status_t DbCopyColumnValue(DmOpenRef	dbRef,
						   uint32_t		rowID,
						   uint32_t		columnID,
						   uint32_t		offset,
						   void			* valueP,
						   uint32_t		* valueSizeP);

status_t DbCopyColumnValues(DmOpenRef  				dbRef,
							uint32_t				rowID,
							uint32_t			 	numColumns,
							DbSchemaColumnValueType	* columnValuesP);

// Get a pointer to a server owned buffer containing the data
status_t DbGetColumnValue(DmOpenRef		dbRef,
						  uint32_t		rowID,
						  uint32_t		columnID,
						  uint32_t		offset,
						  void			** valuePP,
						  uint32_t		* valueSizeP);

status_t DbGetColumnValues(DmOpenRef				dbRef,
						   uint32_t			 		rowID,
						   uint32_t			 		numColumns,
						   const uint32_t			columnIDs[],
						   DbSchemaColumnValueType	** columnValuesPP);

status_t DbGetAllColumnValues(DmOpenRef					dbRef,
							  uint32_t					rowID,
							  uint32_t					* numColumnsP,
							  DbSchemaColumnValueType	** columnValuesPP);

status_t DbReleaseStorage(DmOpenRef dbRef, void * ptr);


/******************************************************************************
 * Table management
 *****************************************************************************/

// Tables
status_t DbGetTableSchema(DmOpenRef					dbRef,
						  const char				* table,
						  DbTableDefinitionType		** schemaPP);

status_t DbAddTable(DmOpenRef dbRef, const DbTableDefinitionType * schemaP);

status_t DbRemoveTable(DmOpenRef dbRef, const char * table);

status_t DbNumTables(DmOpenRef dbRef, uint32_t * tableCountP);

status_t DbGetTableName(DmOpenRef dbRef, uint32_t index, char * table);

Boolean DbHasTable(DmOpenRef dbRef, const char * table);

// Columns
status_t DbAddColumn(DmOpenRef						dbRef,
					 const char 					* table,
					 const DbSchemaColumnDefnType	* addColumnP);

status_t DbRemoveColumn(DmOpenRef 	dbRef,
						const char	* table,
						uint32_t	columnID);

status_t DbNumColumns(DmOpenRef		dbRef,
					  const char	* table,
					  uint32_t		* columnCountP);

status_t DbGetColumnID(DmOpenRef	dbRef,
					   const char	* table,
					   uint32_t		columnIndex,
					   uint32_t		* columnIDP);

status_t DbGetColumnDefinitions(DmOpenRef				dbRef,
								const char				* table,
								uint32_t				numColumns,
								const uint32_t			columnIDs[],
								DbSchemaColumnDefnType	** columnDefnsPP);

status_t DbGetAllColumnDefinitions(DmOpenRef				dbRef,
								   const char				* table,
								   uint32_t					* numColumnsP,
								   DbSchemaColumnDefnType	** columnDefnsPP);


/******************************************************************************
 * Custom column properties
 *****************************************************************************/

status_t DbGetColumnPropertyValue(DmOpenRef					dbRef,
								  const char				* table,
								  uint32_t					columnID,
								  DbSchemaColumnProperty	propID,
								  uint32_t					* numBytesP,
								  void						** propValuePP);

status_t DbGetColumnPropertyValues(DmOpenRef						dbRef,
								   const char						* table,
								   uint32_t							numProps,
								   const DbColumnPropertySpecType	propSpecs[],
								   DbColumnPropertyValueType		** propValuesPP);

status_t DbGetAllColumnPropertyValues(DmOpenRef					dbRef,
									  const char				* table,
									  Boolean					customPropsOnly,
									  uint32_t					* numPropsP,
									  DbColumnPropertyValueType	** propValuesPP);

status_t DbSetColumnPropertyValue(DmOpenRef					dbRef,
								  const char				* table,
								  uint32_t					columnID,
								  DbSchemaColumnProperty	propID,
								  uint32_t					numBytes,
								  const void				* propValueP);

status_t DbSetColumnPropertyValues(DmOpenRef						dbRef,
								   const char						* table,
								   uint32_t							numProps,
								   const DbColumnPropertyValueType	propValues[]);

status_t DbRemoveColumnProperty(DmOpenRef				dbRef,
								const char				* table,
								uint32_t				columnID,
								DbSchemaColumnProperty	propID);


/******************************************************************************
 * Security
 *****************************************************************************/

// Secure Database APIs
status_t DbCreateSecureDatabase(const char 					* name,
								uint32_t					creator,
								uint32_t					type,
								uint32_t					numSchemas,
								const DbTableDefinitionType	schemaList[],
								AzmRuleSetType				* ruleset,
								DatabaseID					* id);

status_t DbGetRuleSet(DatabaseID dbID, AzmRuleSetType * ruleset);


/******************************************************************************
 * Category membership
 *****************************************************************************/

// Row based
status_t DbAddCategory(DmOpenRef		dbRef,
				  	   uint32_t			rowID,
				  	   uint32_t			numToAdd,
				  	   const CategoryID	categoryIDs[]);

status_t DbGetCategory(DmOpenRef		dbRef,
				  	   uint32_t			rowID,
				  	   uint32_t			* pNumCategories,
				  	   CategoryID		* pCategoryIDs[]);

status_t DbIsRowInCategory(DmOpenRef		dbRef,
						   uint32_t			rowID,
					 	   uint32_t			numCategories,
					 	   const CategoryID	categoryIDs[],
					 	   DbMatchModeType	matchMode,
					 	   Boolean			* pIsInCategory);

status_t DbNumCategory(DmOpenRef	dbRef,
					   uint32_t		rowID,
					   uint32_t		* pNumCategories);

status_t DbSetCategory(DmOpenRef		dbRef,
					   uint32_t			rowID,
					   uint32_t			numToSet,
					   const CategoryID	categoryIDs[]);

status_t DbRemoveCategory(DmOpenRef			dbRef,
						  uint32_t			rowID,
						  uint32_t			numToRemove,
						  const CategoryID	categoryIDs[]);

// Global
status_t DbMoveCategory(DmOpenRef			dbRef,
						CategoryID			toCategory,
						uint32_t			numFromCategories,
						const CategoryID	fromCategoryIDs[],
						DbMatchModeType		matchMode);

status_t DbRemoveCategoryAllRows(DmOpenRef			dbRef,
						 		 uint32_t	 		numCategories,
						 		 const CategoryID	categoryIDs[],
						 		 DbMatchModeType	matchMode);


/******************************************************************************
 * Restore
 *****************************************************************************/

status_t DbCreateSecureDatabaseFromImage(const void		* bufferP,
										 DatabaseID		* pDbID,
										 AzmRuleSetType	* pRuleSet);


/******************************************************************************
 * Sort indices
 *****************************************************************************/

status_t DbAddSortIndex(DmOpenRef dbRef, const char * orderBy);

status_t DbGetSortDefinition(DmOpenRef dbRef, uint32_t sortIndex, char** orderByPP);

status_t DbNumSortIndexes(DmOpenRef dbRef, uint32_t *countP);

status_t DbRemoveSortIndex(DmOpenRef dbRef, const char* orderBy);

Boolean DbHasSortIndex(DmOpenRef dbRef, const char * orderBy);

status_t DbEnableSorting(DmOpenRef dbRef, Boolean enable);

status_t DbIsSortingEnabled(DmOpenRef dbP, Boolean * enableP);


/*****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif // _SCHEMA_DATABASES_H_

