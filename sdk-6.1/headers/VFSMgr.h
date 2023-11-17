/******************************************************************************
 *
 * Copyright (c) 2000-2004 PalmSource, Inc. All rights reserved.
 *
 * File: VFSMgr.h
 *
 * Release: Palm OS 6.1
 *
 * Description:
 *		Header file for VFS Manager.
 *
 *****************************************************************************/

#ifndef __VFSMGR_H__
#define __VFSMGR_H__

#include <PalmTypes.h>
#include <DataMgr.h>
#include <CmnErrors.h>
#include <SystemResources.h>

#include <ExpansionMgr.h>


#define vfsFtrIDVersion		(0)	// ID of feature containing version of VFSMgr.
								// Check existence of this feature to see if VFSMgr is installed.

#define vfsFtrIDDefaultFS	(1)	// ID of feature containing the creator ID of the default filesystem library
								// this is the default choice when choosing a library for formatting/mounting

#define vfsMgrVersionNum	((uint16_t)300)	// version of the VFSMgr, obtained from the feature


// MountClass constants:
#define vfsMountClass_SlotDriver	sysFileTSlotDriver   
#define vfsMountClass_POSE          'pose'
#define vfsMountClass_SlotDriver_BE	'sbil'	//DO NOT USE, for PACE compatibility only
#define vfsMountClass_POSE_BE       'esop'	//DO NOT USE, for PACE compatibility only



// Base MountParamType; others such as SlotMountParamType are extensions of this base type,
// switched on value of "mountClass" parameter.  It will make more sense someday when there
// are other kinds of FileSystems...
typedef struct VFSAnyMountParamTag 
{
	uint16_t volRefNum;			// The volRefNum of the volume.
	uint16_t size;				// Undefined for OS 5 & earlier, may be ignored for vfsMountClass_SlotDriver
								//		and vfsMountClass_POSE which have a known size
	uint32_t mountClass;			// 'libs' for slotDriver-based filesystems
	
	// Other fields here, depending on value of 'mountClass'
	
} VFSAnyMountParamType;
#define SIZEOF_VFSAnyMountParamType		(8)
#define SIZEOF_LargestVFSMountParamType	(128)	// The largest size MountParamType PACE will handle

typedef VFSAnyMountParamType *VFSAnyMountParamPtr ;

typedef struct VFSSlotMountParamTag 
{
	VFSAnyMountParamType	vfsMountParam;		// mountClass = VFSMountClass_SlotDriver = 'libs'
	uint16_t				slotLibRefNum;		// obsolete
	uint16_t				slotRefNum;
} VFSSlotMountParamType;
#define SIZEOF_VFSSlotMountParamType	(SIZEOF_VFSAnyMountParamType + 4)

typedef struct VFSPOSEMountParamTag
{
	VFSAnyMountParamType vfsMountParam;    // mountClass = VFSMountClass_POSE = 'pose'
	uint8_t               poseSlotNum;
	uint8_t				reserved;
	uint16_t				reserved2;
} VFSPOSEMountParamType;
#define SIZEOF_VFSPOSEMountParamType	(SIZEOF_VFSAnyMountParamType + 4)

/* For Example...
typedef struct VFSOtherMountParamTag {
	VFSAnyMountParamType	vfsMountParam;		// mountClass = 'othr' (for example)
	uint16_t				otherValue;
} VFSOtherMountParamType;
*/

typedef struct FileInfoTag
{
	uint32_t	attributes;
	char		*nameP;				// buffer to receive full name; pass NULL to avoid getting name
	uint16_t	nameBufLen; 		// size of nameP buffer, in bytes
	uint16_t  reserved;
} FileInfoType, *FileInfoPtr;



typedef struct VolumeInfoTag
{
	uint32_t	attributes;			// read-only etc.
	uint32_t	fsType;				// Filesystem type for this volume (defined below)
	uint32_t	fsCreator;			// Creator code of filesystem driver for this volume.  For use with VFSCustomControl().
	uint32_t	mountClass;			// mount class that mounted this volume
	
	// For slot based filesystems: (mountClass = vfsMountClass_SlotDriver)
	uint16_t	slotLibRefNum;		// Library on which the volume is mounted
	uint16_t	slotRefNum;			// ExpMgr slot number of card containing volume
	uint32_t	mediaType;			// Type of card media (mediaMemoryStick, mediaCompactFlash, etc...)
	uint32_t	reserved;			// reserved for future use (other mountclasses may need more space)
} VolumeInfoType, *VolumeInfoPtr;


typedef uint32_t FileRef;

#define	vfsInvalidVolRef		(0)		// constant for an invalid volume reference, guaranteed not to represent a valid one.  Use it like you would use NULL for a FILE*.
#define	vfsInvalidFileRef		(0L)	// constant for an invalid file reference, guaranteed not to represent a valid one.  Use it like you would use NULL for a FILE*.


/************************************************************
 * File Origin constants: (for the origins of relative offsets passed to 'seek' type routines).
 *************************************************************/
#define vfsOriginBeginning		(0)	// from the beginning (first data byte of file)
#define vfsOriginCurrent		(1)	// from the current position
#define vfsOriginEnd			(2)	// from the end of file (one position beyond last data byte, only negative offsets are legal)

typedef uint16_t	FileOrigin;


/************************************************************
 * openMode flags passed to VFSFileOpen
 *************************************************************/
#define vfsModeExclusive			(0x0001U)		// don't let anyone else open it
#define vfsModeRead					(0x0002U)		// open for read access
#define vfsModeWrite				(0x0004U | vfsModeExclusive)		// open for write access, implies exclusive
#define vfsModeCreate				(0x0008U)		// create the file if it doesn't already exist.  Implemented in VFS layer, no FS lib call will ever have to handle this.
#define vfsModeTruncate				(0x0010U)		// Truncate file to 0 bytes after opening, removing all existing data.  Implemented in VFS layer, no FS lib call will ever have to handle this.
#define vfsModeReadWrite			(vfsModeWrite | vfsModeRead)		// open for read/write access
#define vfsModeLeaveOpen			(0x0020U)		// Leave the file open even if when the foreground task closes


		// Combination flag constants, for error checking purposes:
#define vfsModeAll				(vfsModeExclusive | vfsModeRead | vfsModeWrite | vfsModeCreate | vfsModeTruncate | vfsModeReadWrite | vfsModeLeaveOpen)
#define vfsModeVFSLayerOnly		(vfsModeCreate | vfsModeTruncate)		// flags only used apps & the VFS layer, FS libraries will never see these.


/************************************************************
 * File Attributes
 *************************************************************/
#define vfsFileAttrReadOnly		(0x00000001UL)
#define vfsFileAttrHidden		(0x00000002UL)
#define vfsFileAttrSystem		(0x00000004UL)
#define vfsFileAttrVolumeLabel	(0x00000008UL)
#define vfsFileAttrDirectory	(0x00000010UL)
#define vfsFileAttrArchive		(0x00000020UL)
#define vfsFileAttrLink			(0x00000040UL)

#define vfsFileAttrAll			(0x0000007fUL)


/************************************************************
 * Volume Attributes
 *************************************************************/
#define vfsVolumeAttrSlotBased      (0x00000001UL)  // reserved
#define vfsVolumeAttrReadOnly       (0x00000002UL)  // volume is read only
#define vfsVolumeAttrHidden         (0x00000004UL)  // volume should not be user-visible.
// reserved (0x00000008UL)
#define vfsVolumeAttrPrivate        (0x00000010UL)  // volume is not accessible by user processes
#define vfsVolumeAttrSystemBackup   (0x00000020UL)  // volume is used by system backup

/************************************************************
 * Date constants (for use with VFSFileGet/SetDate)
 *************************************************************/
#define vfsFileDateCreated		(1)
#define vfsFileDateModified		(2)
#define vfsFileDateAccessed		(3)

/************************************************************
 * Iterator start and stop constants.
 * Used by VFSVolumeEnumerate, VFSDirEntryEnumerate, VFSDirEntryEnumerate
 *************************************************************/
#define vfsIteratorStart		(0L)
#define vfsIteratorStop			(0xffffffffL)


/************************************************************
 * 'handled' field bit constants 
 * (for use with Volume Mounted/Unmounted notifications)
 *************************************************************/
#define vfsHandledUIAppSwitch	(0x01)	// Any UI app switching has already been handled.  
												// The VFSMgr will not UIAppSwitch to the start.prc app 
												// (but it will loaded & sent the AutoStart launchcode), 
												// and the Launcher will not switch to itself.
#define vfsHandledStartPrc		(0x02)	// And automatic running of start.prc has already been handled.
												// VFSMgr will not load it, send it the AutoStart launchcode,
												// or UIAppSwitch to it.

/************************************************************
 * Format/Mount flags (for use with VFSVolumeFormat/Mount)
 *************************************************************/
#define vfsMountFlagsUseThisFileSystem		(0x01)	// Mount/Format the volume with the filesystem specified
//#define vfsMountFlagsPrivate1				(0x02)	// for system use only
//#define vfsMountFlagsPrivate2				(0x04)	// for system use only
#define vfsMountFlagsReserved1				(0x08)	// reserved
#define vfsMountFlagsReserved2				(0x10)	// reserved
#define vfsMountFlagsReserved3				(0x20)	// reserved
#define vfsMountFlagsReserved4				(0x40)	// reserved
#define vfsMountFlagsReserved5				(0x80)	// reserved


/************************************************************
 * Common filesystem types.  Used by FSFilesystemType and SlotCardIsFilesystemSupported.
 *************************************************************/
#define vfsFilesystemType_VFAT		'vfat'		// FAT12 and FAT16 extended to handle long file names
#define vfsFilesystemType_FAT		'fats'		// FAT12 and FAT16 which only handles 8.3 file names
#define vfsFilesystemType_NTFS		'ntfs'		// Windows NT filesystem
#define vfsFilesystemType_HFSPlus	'hfse'		// The Macintosh extended hierarchical filesystem
#define vfsFilesystemType_HFS		'hfss'		// The Macintosh standard hierarchical filesystem
#define vfsFilesystemType_MFS		'mfso'		// The Macintosh original filesystem
#define vfsFilesystemType_EXT2		'ext2'		// Linux filesystem
#define vfsFilesystemType_FFS		'ffsb'		// Unix Berkeley block based filesystem
#define vfsFilesystemType_NFS		'nfsu'		// Unix Networked filesystem
#define vfsFilesystemType_AFS		'afsu'		// Unix Andrew filesystem
#define vfsFilesystemType_Novell	'novl'		// Novell filesystem
#define vfsFilesystemType_HPFS		'hpfs'		// OS2 High Performance filesystem


/************************************************************
 * Error codes
 *************************************************************/
#define vfsErrBufferOverflow		(vfsErrorClass | 1)	// passed in buffer is too small
#define vfsErrFileGeneric			(vfsErrorClass | 2)	// Generic file error.
#define vfsErrFileBadRef			(vfsErrorClass | 3)	// the fileref is invalid (has been closed, or was not obtained from VFSFileOpen())
#define vfsErrFileStillOpen			(vfsErrorClass | 4)	// returned from FSFileDelete if the file is still open
#define vfsErrFilePermissionDenied	(vfsErrorClass | 5)	// The file is read only
#define vfsErrFileAlreadyExists		(vfsErrorClass | 6)	// a file of this name exists already in this location
#define vfsErrFileEOF				(vfsErrorClass | 7)	// file pointer is at end of file
#define vfsErrFileNotFound			(vfsErrorClass | 8)	// file was not found at the path specified
#define vfsErrVolumeBadRef			(vfsErrorClass | 9)	// the volume refnum is invalid.
#define vfsErrVolumeStillMounted	(vfsErrorClass | 10)	// returned from FSVolumeFormat if the volume is still mounted
#define vfsErrNoFileSystem			(vfsErrorClass | 11)	// no installed filesystem supports this operation
#define vfsErrBadData				(vfsErrorClass | 12)	// operation could not be completed because of invalid data (i.e., import DB from .PRC file)
#define vfsErrDirNotEmpty			(vfsErrorClass | 13)	// can't delete a non-empty directory
#define vfsErrBadName				(vfsErrorClass | 14)	// invalid filename, or path, or volume label or something...
#define vfsErrVolumeFull			(vfsErrorClass | 15)	// not enough space left on volume
#define vfsErrUnimplemented			(vfsErrorClass | 16)	// this call is not implemented
#define vfsErrNotADirectory			(vfsErrorClass | 17)	// This operation requires a directory
#define vfsErrIsADirectory          (vfsErrorClass | 18) // This operation requires a regular file, not a directory
#define vfsErrDirectoryNotFound		(vfsErrorClass | 19) // Returned from VFSFileCreate when the path leading up to the new file does not exist
#define vfsErrNameShortened			(vfsErrorClass | 20) // A volume name or filename was automatically shortened to conform to filesystem spec

#ifdef __cplusplus
extern "C" {
#endif

typedef status_t	(*VFSImportProcPtr)
				(uint32_t totalBytes, uint32_t offset, void *userDataP);
typedef status_t	(*VFSExportProcPtr)
				(uint32_t totalBytes, uint32_t offset, void *userDataP);
	
// if you pass NULL for fsCreator, VFS will iterate through 
// all installed filesystems until it finds one that does not return an error.
status_t VFSCustomControl(uint32_t fsCreator, uint32_t apiCreator, uint16_t apiSelector, 
							void *valueP, uint16_t *valueLenP);

status_t VFSFileCreate(uint16_t volRefNum, const char *pathNameP);

status_t VFSFileOpen(uint16_t volRefNum, const char *pathNameP, uint16_t openMode, FileRef *fileRefP);

status_t VFSFileOpenFromURL(const char *fileURLP, uint16_t openMode, FileRef *fileRefP, uint16_t *numOccurrencesP);

status_t VFSFileClose(FileRef fileRef);

status_t VFSFileReadData(FileRef fileRef, uint32_t numBytes, void *bufBaseP, uint32_t offset, uint32_t *numBytesReadP);

status_t VFSFileRead(FileRef fileRef, uint32_t numBytes, void *bufP, uint32_t *numBytesReadP);

status_t VFSFileWrite(FileRef fileRef, uint32_t numBytes, const void *dataP, uint32_t *numBytesWrittenP);

// some file routines work on directories
status_t VFSFileDelete(uint16_t volRefNum, const char *pathNameP);

status_t VFSFileRename(uint16_t volRefNum, const char *pathNameP, const char *newNameP);

status_t VFSFileSeek(FileRef fileRef, FileOrigin origin, int32_t offset);

status_t VFSFileEOF(FileRef fileRef);

status_t VFSFileTell(FileRef fileRef, uint32_t *filePosP);

status_t VFSFileSize(FileRef fileRef, uint32_t *fileSizeP);

status_t VFSFileResize(FileRef fileRef, uint32_t newSize);

status_t VFSFileGetAttributes(FileRef fileRef, uint32_t *attributesP);

status_t VFSFileSetAttributes(FileRef fileRef, uint32_t attributes);

status_t VFSFileGetDate(FileRef fileRef, uint16_t whichDate, uint32_t *dateP);

status_t VFSFileSetDate(FileRef fileRef, uint16_t whichDate, uint32_t date);


status_t VFSDirCreate(uint16_t volRefNum, const char *dirNameP);

status_t VFSDirEntryEnumerate(FileRef dirRef, uint32_t *dirEntryIteratorP, FileInfoType *infoP);


status_t VFSGetDefaultDirectory(uint16_t volRefNum, const char *fileTypeStr, char *pathStr, uint16_t *bufSizeP);

status_t VFSRegisterDefaultDirectory(const char *fileTypeStr, uint32_t mediaType, const char *pathStr);

status_t VFSUnregisterDefaultDirectory(const char *fileTypeStr, uint32_t mediaType);



status_t VFSVolumeFormat(uint8_t flags, uint16_t fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP);

status_t VFSVolumeMount(uint8_t flags, uint16_t fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP);

status_t VFSVolumeUnmount(uint16_t volRefNum);

status_t VFSVolumeEnumerateV60(uint16_t *volRefNumP, uint32_t *volIteratorP);

status_t VFSVolumeEnumerate(uint16_t *volRefNumP, uint32_t *volIteratorP);

status_t VFSVolumeInfo(uint16_t volRefNum, VolumeInfoType *volInfoP);

status_t VFSVolumeGetLabel(uint16_t volRefNum, char *labelP, size_t bufSize);

status_t VFSVolumeSetLabel(uint16_t volRefNum, const char *labelP);

status_t VFSVolumeSizeV60(uint16_t volRefNum, uint32_t *volumeUsedP, uint32_t *volumeTotalP);

status_t VFSVolumeSize(uint16_t volRefNum, uint64_t *volumeUsedP, uint64_t *volumeTotalP);

status_t VFSImportDatabaseFromFile(uint16_t volRefNum, const char *pathNameP, DatabaseID *dbIDP);

status_t VFSImportDatabaseFromFileCustom(uint16_t volRefNum, const char *pathNameP, DatabaseID *dbIDP, VFSImportProcPtr importProcP,
							void *userDataP);

status_t VFSExportDatabaseToFile(uint16_t volRefNum, const char *pathNameP, DatabaseID dbID);

status_t VFSExportDatabaseToFileCustom(uint16_t volRefNum, const char *pathNameP, DatabaseID dbID, VFSExportProcPtr exportProcP,
							void *userDataP);

status_t VFSFileDBGetResource(FileRef ref, DmResourceType type, DmResourceID resID, MemHandle *resHP);

status_t VFSFileDBInfo(FileRef ref, char *nameP,
					uint16_t *attributesP, uint16_t *versionP, uint32_t *crDateP,
					uint32_t *modDateP, uint32_t *bckUpDateP,
					uint32_t *modNumP, MemHandle *appInfoHP,
					MemHandle *sortInfoHP, uint32_t *typeP,
					uint32_t *creatorP, uint16_t *numRecordsP);

status_t VFSFileDBGetRecord(FileRef ref, uint16_t recIndex, MemHandle *recHP, uint8_t *recAttrP, uint32_t *uniqueIDP);

// Deprecated APIs ************************************************************

status_t VFSExportDatabaseToFileV40(uint16_t volRefNum, const char *pathNameP, uint16_t cardNo, LocalID dbID);

status_t VFSExportDatabaseToFileCustomV40(uint16_t volRefNum, const char *pathNameP, uint16_t cardNo, LocalID dbID,
	VFSExportProcPtr exportProcP, void *userDataP);

status_t VFSImportDatabaseFromFileV40(uint16_t volRefNum, const char *pathNameP, uint16_t *cardNoP, LocalID *dbIDP);

status_t VFSImportDatabaseFromFileCustomV40(uint16_t volRefNum, const char *pathNameP, uint16_t *cardNoP, LocalID *dbIDP,
	VFSImportProcPtr importProcP, void *userDataP);

#ifdef __cplusplus
}
#endif

#endif	// __VFSMGR_H__
