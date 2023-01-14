/////////////////////////////////////////////////////////////////////////////
//
// FAT Flash FATFlashDoc.h Header File
//
// Copyright (c) 2001 Palm Inc. or its subsidiaries.
// All rights reserved.  This software may be copied and used solely for 
// developing products for the communicating with the Palm Inc. platform
// and for archival and backup purposes.  Except for the foregoing, no part 
// of this software may be reproduced or transmitted in any form or by any 
// means or used to make any derivative work (such as translation, 
// transformation or adaptation) without express written consent from Palm Inc.
//
// Palm Inc. reserves the right to revise this software and to make changes in 
// content from time to time without obligation on the part of Palm Inc. to 
// provide notification of such revision or changes.  PALM INC. MAKES NO 
// REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF ERRORS OR 
// THAT THE SOFTWARE IS SUITABLE FOR YOUR USE.  THE SOFTWARE IS PROVIDED ON
// AN "AS IS" BASIS.  PALM INC. MAKES NO WARRANTIES, TERMS OR CONDITIONS, EXPRESS
// OR IMPLIED, EITHER IN FACT OR BY OPERATION OF LAW, STATUTORY OR 
// OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.
//
// TO THE FULL EXTENT ALLOWED BY LAW, PALM INC. ALSO EXCLUDES FOR ITSELF AND ITS
// SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING 
// NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
// PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
// BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING 
// OUT OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF PALM INC. HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES.
//
// HotSync, Palm Inc., and Graffiti are registered trademarks, 
// and Palm III and Palm OS are trademarks of Palm Inc. or its 
// subsidiaries.
//
// IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK, THE OTHER SOFTWARE AND 
// DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT 
// ACCOMPANYING THE COMPACT DISK.
// 
/////////////////////////////////////////////////////////////////////////////
//
// Revision History:
//
// Date         Name        Description
// ----------------------------------------------------------
//  12/20/01    GAR         initial release
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_FATFLASHDOC_H__85397FCE_BF29_11D5_B90D_00A0CC64D527__INCLUDED_)
#define AFX_FATFLASHDOC_H__85397FCE_BF29_11D5_B90D_00A0CC64D527__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	PATH_SDIO_EDK           "C:\\Palm_SDIO_EDK"
#define PATH_SDIO_EDK_FILENAME  "C:\\Palm_SDIO_EDK\\IMAGE.BIN"
#define PATH_SDIO_EDK_FILENAME_PDB  "C:\\Palm_SDIO_EDK\\IMAGE.PDB"
#define	PATH_SDIO_EDK_CARD      "C:\\Palm_SDIO_EDK\\Card"

#define	EDK_FILESTREAM_NAME		"EDKFileStream-EDK1"
#define	EDK_FILESTREAM_CREATOR	'EDK1'
#define	EDK_FILESTREAM_TYPE		'STRM'
#define	EDK_FILESTREAM_ATTRIBS	0x0000
#define	EDK_FILESTREAM_VERSION	0x00000000L

#define	EDK_FILESTREAM_CREATEDATE	0x3BF072AEL
#define	EDK_FILESTREAM_MODDATE	0x3BF072AEL
#define	EDK_FILESTREAM_BACKUPDATE	0x3BF072AEL
#define	EDK_FILESTREAM_MODNUMBER	0x00000000L

#define	BLANK_BYTE	0xff	// blank byte programmed into flash
#define	ZERO_BYTE	0x00	// zero byte programmed into flash

#define	DOC_OVERWRITES_VIEW	0L	// parameter for UpdateAllViews()
#define	VIEW_OVERWRITES_DOC	1L	// parameter for UpdateAllViews()


#pragma pack(push,1)

class CPbr			// Partition Boot Record
{
public:
	unsigned char	fingerprint[3];
	unsigned char	oemName[8];
	__int16			bytesPerSector;
	__int8			sectorsPerCluster;
	__int16			reservedSectors;
	__int8			numberOfFats;
	__int16			numberOfRootDirEntries;
	__int16			numberOfSectors;
	unsigned char	mediaDescriptor;
	__int16			sectorsPerFat;
	__int16			sectorsPerHead;
	__int16			headsPerCylinder;
	__int32			numberOfHiddenSectors;
	__int32			numberOfSectors2;
	unsigned char	driveNumber;
	unsigned char	reserved;
	unsigned char	extendedSignature;
	__int32			volumeId;
	char			volumeName[11];
	char			fatType[8];
	unsigned char	bootProgram[448];
	unsigned char	signature[2];
};

#define FILE_ATTR_READONLY         0x01
#define FILE_ATTR_HIDDEN           0x02
#define FILE_ATTR_SYSTEM           0x04
#define FILE_ATTR_VOLUME           0x08
#define FILE_ATTR_DIRECTORY        0x10
#define FILE_ATTR_ARCHIVE          0x20

class CDirEntry		// 8.3 directory entry
{
public:
	char			name[8];
	char			extension[3];
	BYTE			attribute;	// (00ARSHDV)
	BYTE			reservedNT;	// always 0
	BYTE			createTimeMs;	// creation time (ms)
	WORD			createTime;		// creation time (hr and min)
	WORD			createDate;		// creation date
	WORD			accessDate;		// last accessed date
	WORD			extendedAttr;	// reserved for OS/2 (always 0)
	WORD			time;
	WORD			date;
	WORD			cluster;
	DWORD			fileSize;
};

class CLFNDirEntry	// long filename directory entry
{
public:
	BYTE			ordinal;
	WORD			name1[5];	// characters 1-5 of filename
	BYTE			attribute;	// (00ARSHDV)
	BYTE			type;		// reserved (always 0)
	BYTE			checksum;	
	WORD			name2[6];	// characters 6-11 of filename
	WORD			cluster;
	WORD			name3[2];	// characters 12-13 of filename
};


//
//  Remap Palm OS types to PC types
//
#ifndef LocalID
#define LocalID DWORD
#endif

#ifndef Byte
#define Byte  BYTE
#endif

#ifndef Word
#define Word unsigned short
#endif

#ifndef DWord
#define DWord DWORD
#endif

#ifndef ULong
#define ULong unsigned long
#endif


#define	DB_NAMELEN					32		// Max. 32 characters in DB name
#define	MAX_RECORD_SIZE				65536	// Max. PDB record size

// Attributes of a Database
#define	dmHdrAttrResDB				0x0001	// Resource database
#define dmHdrAttrReadOnly			0x0002	// Read Only database
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
#define dmHdrAttrFileStream			0x0080	// File Stream database
#define	dmHdrAttrOpen				0x8000	// Database not closed properly


/************************************************************
 * Structure of a Record entry
 *************************************************************/
typedef struct {
	LocalID		localChunkID;				// local chunkID of a record
	Byte		attributes;					// record attributes;
	Byte		uniqueID[3];				// unique ID of record
} RecordEntryType;
typedef RecordEntryType*	RecordEntryPtr;



/************************************************************
 * Structure of a Resource entry
 *************************************************************/
typedef struct {
	DWord		type;						// resource type
	Word		id;							// resource id
	LocalID		localChunkID;				// resource data chunk ID
} RsrcEntryType;
typedef RsrcEntryType*	RsrcEntryPtr;

// Attributes field
#define	dmRsrcAttrUnused		0x0000	// to be defined...



/************************************************************
 * Structure of a record list extension. This is used if all
 *  the database record/resource entries of a database can't fit into
 *  the database header.
 *************************************************************/
typedef struct {
	LocalID		nextRecordListID;			// local chunkID of next list
	Word		numRecords;					// number of records in this list
	Word		firstEntry;					// array of Record/Rsrc entries 
											// starts here
} RecordListType;
typedef RecordListType*	RecordListPtr;




/************************************************************
 * Structure of a Database Header
 *************************************************************/
typedef struct {
	Byte		name[DB_NAMELEN];	    // name of database
	Word		attributes;				// database attributes
	Word		version;				// version of database

	DWord		creationDate;			// creation date of database
	DWord		modificationDate;		// latest modification date
	DWord		lastBackupDate;			// latest backup date
	DWord		modificationNumber;		// modification number of database

	LocalID		appInfoID;				// application specific info
	LocalID		sortInfoID;				// app specific sorting info

	DWord		type;					// database type
	DWord		creator;				// database creator 
	
	DWord		uniqueIDSeed;			// used to generate unique IDs

	RecordListType	recordList;			// first record list
} DatabaseHdrType;		

typedef DatabaseHdrType*	DatabaseHdrPtr;
typedef DatabaseHdrPtr*		DatabaseHdrHand;


#pragma pack(pop)

#define	CATEGORY_UNFILED	0x00
#define	ATTR_MODIFIED		0x0004
#define	UID_NEW_RECORD		0x00000000L
#define	FS_RECORDSIZE		4096

class CFATFlashDoc : public CDocument
{
protected: // create from serialization only
	CFATFlashDoc();
	DECLARE_DYNCREATE(CFATFlashDoc)

// Attributes
public:
	CString m_csRootDirectory;
	CPbr	m_pbr;	// Partition Boot Record (PBR)	
	LPBYTE	m_fat;	// Fila Allocation Table (FAT)

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFATFlashDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
    static unsigned long SwapDWordToIntel(DWORD dwSubj);
    static WORD FlipWord(WORD wSubj);
    static unsigned long SwapDWordToMotor(unsigned long dwSubj);

	void SetRootDirectory( const CString& value );
	void SetVolumeName( const CString& value );
	void SetVolumeId( const __int32 value );
	void SetNumberOfHiddenSectors( const __int32 value );
	void SetNumberOfSectors( const __int16 value );
	void SetNumberOfRootDirEntries( const __int16 value );
	void SetNumberOfFats( const __int8 value );
	void SetReservedSectors( const __int16 value );
	void SetSectorsPerCluster( const __int8 value );
	void SetBytesPerSector( const __int16 value );
	CFile m_rImageFile;
	CFile m_rPdbFile;
	virtual ~CFATFlashDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	WORD GetTimeStamp(const CTime& t);
	WORD GetDateStamp(const CTime& t);
	void WriteRootDirectorySectors();
	void WriteFilesystem();
	long m_nSectorsWritten;

private:
    // Database Header Variables
	DatabaseHdrType     m_dbHeader;	// Temporary buffer for database header data
    BYTE		m_szRawData[MAX_RECORD_SIZE];	// Temporary buffer for record data

// Generated message map functions
protected:
	long StorePdb(void);
	long StoreDbHeader(void);
	long StoreAppInfo(void);
	long StoreSortInfo(void);
	long StoreRecordList(void);
	long StoreRecords(void);
	void ConvertDBHdr(DatabaseHdrPtr pHdr);
	char* m_lpszUsedShortFilenames[1024];
	void GetShortFilename(const CFileFind &finder, CString& csFilename, CString& csAlias, BOOL bSave=FALSE);
	void WriteLFNDirectoryEntry(const int nSequence, const long myOffset, const CFileFind &finder, const WORD wMyCluster, BOOL bLastEntry=FALSE);
	CString m_csExportFilePath;
	CString m_csExportFileExt;
	CString m_csExportPdbFilePath;
	void WriteFATEntry(WORD wEntry, WORD wValue);
	void WriteFile(const CFileFind& finder, WORD nStartingCluster);
	void AddSubdirectory(const CString& myDir, WORD myCluster, WORD myParentCluster);
	BOOLEAN IsShortFilename(const CFileFind& finder);
	LPBYTE m_pBlankCluster;
	long m_nStartOfFats;
	long m_nStartOfAvailableClusters;
	void WriteShortDirectoryEntry(const long myOffset, const CString csFileName, const CFileFind &finder, const WORD wMyCluster);
	void WriteShortFileEntry(const long myOffset, const CString& csFilename, const CFileFind& finder, WORD wMyCluster);
	WORD AllocateCluster();
	long m_nRootDirectoryEntriesUsed;
	long m_nRootDirectoryStart;
	void InitializeFAT();
	void InitializePBR();
	long ExportFilesystemImage( BOOL bImageFileOpen=FALSE );
	long ExportFilesystemPdb();
	//{{AFX_MSG(CFATFlashDoc)
	afx_msg void OnFileSetrootdir();
	afx_msg void OnExportFilesystem();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD m_dwCurrentRecordOffset;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FATFLASHDOC_H__85397FCE_BF29_11D5_B90D_00A0CC64D527__INCLUDED_)
