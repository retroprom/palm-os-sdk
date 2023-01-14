/////////////////////////////////////////////////////////////////////////////
//
// FAT Flash FATFlashDoc.cpp Src File
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

#include "stdafx.h"
#include "math.h"
#include "FATFlash.h"

#include "FATFlashDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFATFlashDoc

IMPLEMENT_DYNCREATE(CFATFlashDoc, CDocument)

BEGIN_MESSAGE_MAP(CFATFlashDoc, CDocument)
	//{{AFX_MSG_MAP(CFATFlashDoc)
	ON_COMMAND(ID_FILE_SETROOTDIR, OnFileSetrootdir)
	ON_COMMAND(ID_EXPORT_FILESYSTEM, OnExportFilesystem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFATFlashDoc construction/destruction

CFATFlashDoc::CFATFlashDoc()
{
	// Initialize the default PBR values
	InitializePBR();

	// Allocate FAT space and Initialize
	m_fat = NULL;
	InitializeFAT();

	// Initialize default Root Directory
	m_csRootDirectory = PATH_SDIO_EDK_CARD ;

	// Allocate a blank cluster
	m_pBlankCluster = (LPBYTE)malloc(m_pbr.sectorsPerCluster*m_pbr.bytesPerSector);
	if(m_pBlankCluster == NULL)
	{
		// Display an error dialog
		AfxMessageBox( _T("Memory allocation error."), MB_OK | MB_ICONERROR );
		exit(-1);
	}
	memset(m_pBlankCluster, ZERO_BYTE, (m_pbr.sectorsPerCluster*m_pbr.bytesPerSector));
}

CFATFlashDoc::~CFATFlashDoc()
{
	// Free any allocated memory
	if( m_fat != NULL )
		free(m_fat);
	if( m_pBlankCluster != NULL )
		free(m_pBlankCluster);
}

BOOL CFATFlashDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Initilize the PBR
	InitializePBR();

	// Initialize default Root Directory
	m_csRootDirectory = PATH_SDIO_EDK_CARD ;

	// Allocate FAT space and Initialize
	InitializeFAT();

	// Update all views
	UpdateAllViews(NULL, DOC_OVERWRITES_VIEW);
	
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CFATFlashDoc serialization

void CFATFlashDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// Save all local classes and variables
		ar<<m_csRootDirectory;
		ar.Write(m_pbr.fingerprint, 512);
	}
	else
	{
		// Read all local classes and variables
		ar >> m_csRootDirectory;
		ar.Read(m_pbr.fingerprint, 512);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFATFlashDoc diagnostics

#ifdef _DEBUG
void CFATFlashDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFATFlashDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFATFlashDoc commands

void CFATFlashDoc::InitializePBR()
{
	memset( &m_pbr, 0, sizeof(m_pbr) );
	m_pbr.fingerprint[0] = 0xeb;
	m_pbr.fingerprint[1] = 0x3c;
	m_pbr.fingerprint[2] = 0x90;
	m_pbr.oemName[0] = 'M';
	m_pbr.oemName[1] = 'S';
	m_pbr.oemName[2] = 'D';
	m_pbr.oemName[3] = 'O';
	m_pbr.oemName[4] = 'S';
	m_pbr.oemName[5] = '5';
	m_pbr.oemName[6] = '.';
	m_pbr.oemName[7] = '0';
	m_pbr.bytesPerSector = 512;
	m_pbr.sectorsPerCluster = 1;
	m_pbr.reservedSectors = 1;
	m_pbr.numberOfFats = 2;
	m_pbr.numberOfRootDirEntries = 512;
	m_pbr.numberOfSectors = 0x0100;
	m_pbr.mediaDescriptor = 0xf8;
	m_pbr.sectorsPerFat = 1;
	m_pbr.sectorsPerHead = 16;
	m_pbr.headsPerCylinder = 2;
	m_pbr.numberOfHiddenSectors = 0;
	m_pbr.numberOfSectors2 = 0;
	m_pbr.driveNumber = 0;
	m_pbr.reserved = 0;
	m_pbr.extendedSignature = 0x29;
	m_pbr.volumeId = 0;
	memset( &m_pbr.volumeName, ' ', sizeof(m_pbr.volumeName) );
	m_pbr.volumeName[0] = 'S';
	m_pbr.volumeName[1] = 'D';
	m_pbr.volumeName[2] = 'I';
	m_pbr.volumeName[3] = 'O';
	m_pbr.volumeName[5] = 'C';
	m_pbr.volumeName[6] = 'a';
	m_pbr.volumeName[7] = 'r';
	m_pbr.volumeName[8] = 'd';
	strcpy( m_pbr.fatType, "FAT12   " );
	m_pbr.signature[0] = 0x55;
	m_pbr.signature[1] = 0xAA;
}

BOOL CFATFlashDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here
	
	return TRUE;
}

void CFATFlashDoc::OnFileSetrootdir() 
{
	// TODO: Add a directory selection dialog
	m_csRootDirectory = PATH_SDIO_EDK_CARD ;

}

void CFATFlashDoc::OnExportFilesystem() 
{
	long	retval = 0;
	CString	csFileSpec = "\\*.*";
	BOOL bMoreFiles;
	CFileFind finder;
	TCHAR	szFilters[] = _T ("Palm OS File Stream(*.pdb)|*.pdb|Binary Image (*.bin)|*.bin||");
	CFileDialog	dlg (FALSE, _T ("pdb"), _T ("*.pdb"), OFN_HIDEREADONLY, szFilters);
			
	// Ensure the root directory of the card exists
	csFileSpec = m_csRootDirectory + csFileSpec;
	bMoreFiles = finder.FindFile(LPCTSTR(csFileSpec));
	if (!bMoreFiles)
	{
		// Inform the user that the root directory cannot be found
		CString	csMessage = "Unable to find Root Directory of card: ";
		csMessage += (LPCTSTR)m_csRootDirectory;
		AfxMessageBox( _T((LPCTSTR)csMessage), MB_OK | MB_ICONERROR );
		goto Exit;
	}
	
	// Pop-up file selection dialog to select the output file
	if( dlg.DoModal() == IDOK ) {
		m_csExportFilePath = dlg.GetPathName();
		CFileException fileException;
		
		if ( !(m_rImageFile.Open( (LPCTSTR)m_csExportFilePath, CFile::modeCreate |   
			CFile::modeReadWrite )) )
		{
			// Inform the user that the export file cannot be opened
			CString	csMessage = "Unable to open/create file: ";
			csMessage += (LPCTSTR)m_csExportFilePath;
			AfxMessageBox( _T((LPCTSTR)csMessage), MB_OK | MB_ICONERROR );
			goto Exit;
		}
		
		// Close the export file
		m_rImageFile.Close();
		
		// Determine what type of file to output
		CString	csMyExportFileExt;
		m_csExportFileExt = csMyExportFileExt = dlg.GetFileExt();
		CString	csPdbFileExt = "pdb";
		csMyExportFileExt.MakeLower();
		if( csMyExportFileExt == csPdbFileExt )
		{
			retval = ExportFilesystemPdb();
		}
		else
		{
			retval = ExportFilesystemImage();
		}

		if( retval == 0 )
		{
			// Inform the user that a file has been written
			CString	csMessage = "File system image written to: ";
			csMessage += (LPCTSTR)m_csExportFilePath;
			AfxMessageBox( _T((LPCTSTR)csMessage), MB_OK );
		}
		else
		{
			// Inform the user that an error occurred
			CString	csMessage = "Error writing: ";
			csMessage += (LPCTSTR)m_csExportFilePath;
			AfxMessageBox( _T((LPCTSTR)csMessage), MB_OK );
		}
	}
Exit:
	;
}

long CFATFlashDoc::ExportFilesystemImage( BOOL bImageFileOpen /*=FALSE*/ ) 
{
	long	retval = 0;
	int		i=0;
	int		nReservedSectors;
	LPBYTE	lpBlankSector;
	
	if( bImageFileOpen == FALSE )
	{
		// Open the Image file for output
		if ( !(m_rImageFile.Open( (LPCTSTR)m_csExportFilePath, CFile::modeCreate |   
			CFile::modeReadWrite )) )
		{
			retval = -1;
			goto Exit;
		}
	}
	
	// Initialize all filesystem structures
	InitializeFAT();
	
	// Allocate a blank sector
	lpBlankSector = (LPBYTE)malloc(m_pbr.bytesPerSector);
	if(lpBlankSector == NULL)
	{
		// Display an error dialog
		AfxMessageBox( _T("Memory allocation error."), MB_OK | MB_ICONERROR );
		retval = -1;
		goto CloseAndExit;
	}
	memset(lpBlankSector, BLANK_BYTE, m_pbr.bytesPerSector);
	
	// Keep track of how many sectors we have written
	m_nSectorsWritten = 0;
	
	// First, write the Partition Boot Record (PBR)
	m_rImageFile.Write( &m_pbr.fingerprint, sizeof(m_pbr) );
	m_nSectorsWritten += (long)(ceil(sizeof(m_pbr)/m_pbr.bytesPerSector));
	
	// Write any additional reserved sectors
	nReservedSectors = (int)ceil(((m_pbr.reservedSectors*m_pbr.bytesPerSector)-512)/m_pbr.bytesPerSector);
	for(i=0; i<nReservedSectors; i++)
	{
		m_rImageFile.Write( lpBlankSector, m_pbr.bytesPerSector );
		m_nSectorsWritten++;
	}
	
	// Remember where the start of FAT's are
	m_nStartOfFats = m_nSectorsWritten*m_pbr.bytesPerSector;
	
	// Write initial FAT's
	for( i=0; i<m_pbr.numberOfFats; i++ )
	{
		m_rImageFile.Write( m_fat, m_pbr.sectorsPerFat*m_pbr.bytesPerSector );
		m_nSectorsWritten += m_pbr.sectorsPerFat;
	}
	
	// Write the root directory space
	WriteRootDirectorySectors();
	m_nSectorsWritten += (long)(ceil(m_pbr.numberOfRootDirEntries * 32 / m_pbr.bytesPerSector));
	
	// Remember where the start of available sectors is
	m_nStartOfAvailableClusters = m_nSectorsWritten*m_pbr.bytesPerSector;
	
	// Write the space for files
	for( i=m_nSectorsWritten; i<m_pbr.numberOfSectors; i++ )
	{
		m_rImageFile.Write( lpBlankSector, m_pbr.bytesPerSector );
	}
	m_nSectorsWritten = i;
	
	// Write the actual filesystem data starting at the root directory
	WriteFilesystem();
	
	// Re-write the updated FAT's
	m_rImageFile.Seek(m_nStartOfFats, CFile::begin);
	for( i=0; i<m_pbr.numberOfFats; i++ )
	{
		m_rImageFile.Write( m_fat, m_pbr.sectorsPerFat*m_pbr.bytesPerSector );
	}
	
CloseAndExit:
	// Close the export file
	if( bImageFileOpen == FALSE )
		m_rImageFile.Close();
	
Exit:
	// Free any allocated memory
	if(lpBlankSector)
		free(lpBlankSector);
	
	return(retval);
}

void CFATFlashDoc::InitializeFAT()
{
	if( m_fat != NULL )
		free(m_fat);
	
	// Allocate memory for FAT
	m_fat = (LPBYTE)malloc(m_pbr.sectorsPerFat*m_pbr.bytesPerSector);
	
	// First entry of FAT is sign-extended media descriptor followed
	// by end-of-file indicator
	if( m_fat != NULL )
	{
		memset( m_fat, 0, m_pbr.sectorsPerFat*m_pbr.bytesPerSector);
		m_fat[0] = m_pbr.mediaDescriptor;
		m_fat[1] = 0xff;
		m_fat[2] = 0xff;
	}
}


void CFATFlashDoc::WriteRootDirectorySectors()
{
	CDirEntry	rDirEntry;
	long		i;
	CTime		t = CTime::GetCurrentTime();
	
	// Store the location of the start of the Root Directory
	m_nRootDirectoryStart = (m_pbr.sectorsPerFat*m_pbr.bytesPerSector)*m_pbr.numberOfFats + (m_pbr.reservedSectors*m_pbr.bytesPerSector);
		
	// Keep track of how many root directory entries are used
	m_nRootDirectoryEntriesUsed = 0;
	
	// The first entry in the root directory is the volume name
	memset(&rDirEntry, 0, sizeof(rDirEntry));
	memset(&rDirEntry, ' ', (8+3));
	memcpy(&rDirEntry.name, &m_pbr.volumeName, sizeof(m_pbr.volumeName));
	rDirEntry.attribute = FILE_ATTR_VOLUME;
	rDirEntry.date = GetDateStamp(t);
	rDirEntry.time = GetTimeStamp(t);
	m_rImageFile.Write(&rDirEntry, sizeof(rDirEntry));
	m_nRootDirectoryEntriesUsed++;

	// Write remaining directory entries
	memset(&rDirEntry, 0, sizeof(rDirEntry));
	for(i=m_nRootDirectoryEntriesUsed; i<m_pbr.numberOfRootDirEntries; i++)
	{
		m_rImageFile.Write(&rDirEntry, sizeof(rDirEntry));
	}
}

WORD CFATFlashDoc::GetDateStamp(const CTime& t)
{
	WORD	wResult = 0;
	
	wResult = t.GetYear() - 1980;
	wResult = wResult << 4;
	wResult |= t.GetMonth();
	wResult = wResult << 5;
	wResult |= t.GetDay();
	
	return(wResult);
}

WORD CFATFlashDoc::GetTimeStamp(const CTime& t)
{
	WORD	wResult = 0;
	
	wResult = t.GetHour();
	wResult = wResult << 6;
	wResult |= t.GetMinute();
	wResult = wResult << 5;
	wResult |= (t.GetSecond()/2);
	
	return(wResult);
}

void CFATFlashDoc::WriteFilesystem()
{
	CFileFind finder;
	BOOL bMoreFiles;
	CString	csFileSpec = "\\*.*";
	CString	csFilename;
	CString	csShortFilename;
	CString	csAlias;
	CString	csCurrentDir = ".";
	CString	csParentDir = "..";
	long	myEntryOffset;
	WORD	wCluster;
	int		nNumLFNDirEntries;
	
	// Start at the root directory of the card
	csFileSpec = m_csRootDirectory + csFileSpec;
	bMoreFiles = finder.FindFile(LPCTSTR(csFileSpec));
	while (bMoreFiles)
   {
      // Find the first file
	  bMoreFiles = finder.FindNextFile();
	  if( finder.IsDots() )
	  {
		  // In the root directory, we ignore the "."
		  // and ".." directory entries.
		  continue;
	  }

	  // Is this a directory or a file?
	  if( finder.IsDirectory() )
	  {
		  // Add a directory entry
		  if( IsShortFilename(finder) )
		  {
			  // This is a short filename.
			  // Remember the start of my directory entry
			  myEntryOffset = (m_nRootDirectoryStart+m_nRootDirectoryEntriesUsed*32);
			  
			  // Short entries use only one root directory entry
			  m_nRootDirectoryEntriesUsed++;
			  
			  // Allocate a cluster for this directory entry
			  wCluster = AllocateCluster();
			  if( wCluster <= 1 )
			  {
				  AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
				  exit(-1);
			  }
			  
			  // Actually write the entry
			  WriteShortDirectoryEntry(myEntryOffset, finder.GetFileName(), finder, wCluster);
			  
			  // Add the sub-directory
			  AddSubdirectory(finder.GetFilePath(), wCluster, 0x0000);
		  }
		  else
		  {
			  // This is a long filename directory
			  // Determine how many directory entries this filename will take.
			  // Each LFN directory entry holds 13 characters.
			  nNumLFNDirEntries = (int)(((int)((finder.GetFileName()).GetLength()-1) / 13) + 1);

			  for( int i=nNumLFNDirEntries; i>=1; i-- )
			  {
				  // Remember the start of my directory entry
				  myEntryOffset = (m_nRootDirectoryStart+m_nRootDirectoryEntriesUsed*32);
				  
				  // The cluster is only associated with the 8.3 directory entry
				  wCluster = 0;
				  
				  // Actually write the entry
				  WriteLFNDirectoryEntry(i, myEntryOffset, finder, wCluster, ((i==nNumLFNDirEntries)?TRUE:FALSE));
				  
				  // Short entries use only one root directory entry
				  m_nRootDirectoryEntriesUsed++;
			  }
			  
			  // Determine the short filename
			  GetShortFilename(finder, csShortFilename, csAlias);
			  
			  // Remember the start of my directory entry
			  myEntryOffset = (m_nRootDirectoryStart+m_nRootDirectoryEntriesUsed*32);
			  
			  // Allocate a cluster for this directory entry
			  wCluster = AllocateCluster();
			  if( wCluster <= 1 )
			  {
				  AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
				  exit(-1);
			  }
			  
			  // Now, write the short filename entry
			  WriteShortDirectoryEntry(myEntryOffset, csShortFilename, finder, wCluster);
			  
			  // Short entries use only one root directory entry
			  m_nRootDirectoryEntriesUsed++;

			  // Add the sub-directory
			  AddSubdirectory(finder.GetFilePath(), wCluster, 0x0000);
		  }
	  }
	  else
	  {
		  // Add a file entry
		  if( IsShortFilename(finder) )
		  {
			  // This is a short filename.
			  // Remember the start of my directory entry
			  myEntryOffset = (m_nRootDirectoryStart+m_nRootDirectoryEntriesUsed*32);
			  
			  // Short entries use only one root directory entry
			  m_nRootDirectoryEntriesUsed++;
			  
			  // Allocate a cluster for this file entry
			  wCluster = AllocateCluster();
			  if( wCluster <= 1 )
			  {
				  AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
				  exit(-1);
			  }
			  
			  // Actually write the directory entry
			  WriteShortFileEntry(myEntryOffset, finder.GetFileName(), finder, wCluster);
			  
			  // Add the file to the available cluster space
			  WriteFile(finder, wCluster);
		  }
		  else
		  {
			  // This is a long filename
			  // Determine how many directory entries this filename will take.
			  // Each LFN directory entry holds 13 characters.
			  nNumLFNDirEntries = (int)(((int)((finder.GetFileName()).GetLength()-1) / 13) + 1);

			  for( int i=nNumLFNDirEntries; i>=1; i-- )
			  {
				  // Remember the start of my directory entry
				  myEntryOffset = (m_nRootDirectoryStart+m_nRootDirectoryEntriesUsed*32);
				  
				  // The cluster is only associated with the 8.3 directory entry
				  wCluster = 0;
				  
				  // Actually write the entry
				  WriteLFNDirectoryEntry(i, myEntryOffset, finder, wCluster, ((i==nNumLFNDirEntries)?TRUE:FALSE));
				  
				  // Short entries use only one root directory entry
				  m_nRootDirectoryEntriesUsed++;
			  }
			  
			  // Determine the short filename
			  GetShortFilename(finder, csShortFilename, csAlias);
			  
			  // Remember the start of my directory entry
			  myEntryOffset = (m_nRootDirectoryStart+m_nRootDirectoryEntriesUsed*32);
			  
			  // Allocate a cluster for this directory entry
			  wCluster = AllocateCluster();
			  if( wCluster <= 1 )
			  {
				  AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
				  exit(-1);
			  }
			  
			  // Actually write the directory entry
			  WriteShortFileEntry(myEntryOffset, finder.GetFileName(), finder, wCluster);
			  
			  // Short entries use only one root directory entry
			  m_nRootDirectoryEntriesUsed++;

			  // Add the file to the available cluster space
			  WriteFile(finder, wCluster);
		  }
	  }
   }
   
}

WORD CFATFlashDoc::AllocateCluster()
{
	WORD	i, wFatEntry;
	BOOLEAN	bEntryFound;
	WORD	*pWord;
	
	// Search the FAT for first free cluster
	// We always start with cluster number 2
	bEntryFound = FALSE;
	for( i=2; i<(m_pbr.numberOfSectors/m_pbr.sectorsPerCluster); i++)
	{
		pWord = (WORD*)&m_fat[i*3/2];
		wFatEntry = (WORD)*pWord;
		if( (i%2) == 0 )
		{
			// cluster is an even cluster
			wFatEntry &= 0x0fff;
		}
		else
		{
			// cluster is an odd cluster
			wFatEntry = wFatEntry >> 4;
		}

		if( wFatEntry == 0 )
		{
			bEntryFound = TRUE;
			break;
		}
	}

	if( bEntryFound )
	{
		// Mark the sector as used with eof (0xfff)
		pWord = (WORD*)&m_fat[i*3/2];
		wFatEntry = (WORD)*pWord;
		if( (i%2) == 0 )
		{
			// cluster is an even cluster
			wFatEntry &= 0xf000;
			*pWord = (WORD)(wFatEntry | 0x0fff);
		}
		else
		{
			// cluster is an odd cluster
			wFatEntry &= 0x000f;
			*pWord = (WORD)(wFatEntry | 0xfff0);
		}
	}
	
	if( !bEntryFound )
		i = 0;

	return(i);
}

void CFATFlashDoc::AddSubdirectory(const CString& myDir, WORD myCluster, WORD myParentCluster)
{
    CFileFind finder;
    BOOL bMoreFiles;
    CString	csFileSpec = "\\*.*";
    CString	csFilename;
	CString	csShortFilename;
	CString	csAlias;
    CString	csCurrentDir = ".";
    CString	csParentDir = "..";
    long		nDirectoryEntriesUsed=0;
    long		nNextFreeEntryOffset = m_nStartOfAvailableClusters + ((myCluster-2)*m_pbr.sectorsPerCluster*m_pbr.bytesPerSector);
    WORD	wCluster;
	int		nNumLFNDirEntries;
    
    // Initialize the cluster to all 0's
    m_rImageFile.Seek(nNextFreeEntryOffset, CFile::begin);
    m_rImageFile.Write(m_pBlankCluster, (m_pbr.sectorsPerCluster*m_pbr.bytesPerSector));
    
    // Start at the directory passed in
    csFileSpec = myDir + csFileSpec;
    bMoreFiles = finder.FindFile(LPCTSTR(csFileSpec));
    while (bMoreFiles)
    {
		// Find the first file
		bMoreFiles = finder.FindNextFile();
		if( finder.IsDots() )
		{
			// The "." and ".." directories have to be treated special because
			// they don't require allocation of a new FAT entry.
			csFilename = finder.GetFileName();
			if( csFilename == csCurrentDir )
				WriteShortDirectoryEntry(nNextFreeEntryOffset, finder.GetFileName(), finder, myCluster);
			else
				WriteShortDirectoryEntry(nNextFreeEntryOffset, finder.GetFileName(), finder, myParentCluster);
			nNextFreeEntryOffset += 32;
			nDirectoryEntriesUsed++;
		}
		else if( finder.IsDirectory() )
		{
			// Add a sub-directory entry
			if( IsShortFilename(finder) )
			{
				// This is a short filename.
				// Allocate a cluster for this directory entry
				wCluster = AllocateCluster();
				if( wCluster <= 1 )
				{
					AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
					exit(-1);
				}
				
				// Actually write the entry
				WriteShortDirectoryEntry(nNextFreeEntryOffset, finder.GetFileName(), finder, wCluster);
				nNextFreeEntryOffset += 32;
				nDirectoryEntriesUsed++;
				
				// Add the sub-directory
				AddSubdirectory(finder.GetFilePath(), wCluster, myCluster);
			}
			else
			{
				// This is a long filename directory
				// Determine how many directory entries this filename will take.
				// Each LFN directory entry holds 13 characters.
				nNumLFNDirEntries = (int)(((int)((finder.GetFileName()).GetLength()-1) / 13) + 1);
				
				for( int i=nNumLFNDirEntries; i>=1; i-- )
				{
					// The cluster is only associated with the 8.3 directory entry
					wCluster = 0;
					
					// Actually write the entry
					WriteLFNDirectoryEntry(i, nNextFreeEntryOffset, finder, wCluster, ((i==nNumLFNDirEntries)?TRUE:FALSE));
					nNextFreeEntryOffset += 32;
					nDirectoryEntriesUsed++;
				}
				
				// Determine the short filename
				GetShortFilename(finder, csShortFilename, csAlias);
				
				// Allocate a cluster for this directory entry
				wCluster = AllocateCluster();
				if( wCluster <= 1 )
				{
					AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
					exit(-1);
				}
				
				// Now, write the short filename entry
				WriteShortDirectoryEntry(nNextFreeEntryOffset, csShortFilename, finder, wCluster);
				
				// Short entries use only one root directory entry
				nNextFreeEntryOffset += 32;
				nDirectoryEntriesUsed++;
				
				// Add the sub-directory
				AddSubdirectory(finder.GetFilePath(), wCluster, myCluster);
			}
		}
		else
		{
			// Add a file entry
			if( IsShortFilename(finder) )
			{
				// This is a short filename.
				// Allocate a cluster for this file entry
				wCluster = AllocateCluster();
				if( wCluster <= 1 )
				{
					AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
					exit(-1);
				}
				
				// Actually write the directory entry
				WriteShortFileEntry(nNextFreeEntryOffset, finder.GetFileName(), finder, wCluster);
				
				// Short entries use only one root directory entry
				nDirectoryEntriesUsed++;
				nNextFreeEntryOffset += 32;
				
				// Add the file to the available cluster space
				WriteFile(finder, wCluster);
			}
			else
			{
				// This is a long filename
				// Determine how many directory entries this filename will take.
				// Each LFN directory entry holds 13 characters.
				nNumLFNDirEntries = (int)(((int)((finder.GetFileName()).GetLength()-1) / 13) + 1);
				
				for( int i=nNumLFNDirEntries; i>=1; i-- )
				{
					// The cluster is only associated with the 8.3 directory entry
					wCluster = 0;
					
					// Actually write the entry
					WriteLFNDirectoryEntry(i, nNextFreeEntryOffset, finder, wCluster, ((i==nNumLFNDirEntries)?TRUE:FALSE));
					nNextFreeEntryOffset += 32;
					nDirectoryEntriesUsed++;
				}
				
				// Determine the short filename
				GetShortFilename(finder, csShortFilename, csAlias);
				
				// Allocate a cluster for this directory entry
				wCluster = AllocateCluster();
				if( wCluster <= 1 )
				{
					AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
					exit(-1);
				}
				
				// Actually write the directory entry
				WriteShortFileEntry(nNextFreeEntryOffset, finder.GetFileName(), finder, wCluster);
				
				// Short entries use only one root directory entry
				nDirectoryEntriesUsed++;
				nNextFreeEntryOffset += 32;
				
				// Add the file to the available cluster space
				WriteFile(finder, wCluster);
			}
		}
    }
	
}

void CFATFlashDoc::WriteShortDirectoryEntry(const long myOffset, const CString csFileName, const CFileFind &finder, const WORD wMyCluster)
{
	CString	csFileTitle;
	CString	csFileExt;
	CString	csSubDirPath;
	CDirEntry	rDirEntry;
	CTime		t;
	CString	csCurrentDir = ".";
	CString	csParentDir = "..";
	CString	csFilename;
	CString	csAlias;

	// Get the name, title and extension of the file
	if( csFileName == csCurrentDir )
	{
		csFileTitle = csCurrentDir;
		csFileExt = "";
	}
	else if( csFileName == csParentDir )
	{
		csFileTitle = csParentDir;
		csFileExt = "";
	}
	else
	{
		csFileTitle = finder.GetFileTitle();
		if( csFileName.GetLength() > csFileTitle.GetLength() )
			csFileExt = csFileName.Right(csFileName.GetLength()-csFileTitle.GetLength()-1);
		else
			csFileExt = "";
	}
	
	// Determine the short filename
	GetShortFilename(finder, csFilename, csAlias);

	// Initialize the directory entry
	memset(&rDirEntry, 0, sizeof(rDirEntry));
	memset(&rDirEntry, ' ', (8+3));
	memcpy(&rDirEntry.name, csAlias.GetBuffer(0), 11);
	
	// Set the file attributes
	rDirEntry.attribute |= (finder.IsReadOnly()) ? FILE_ATTR_READONLY : 0x00;
	rDirEntry.attribute |= (finder.IsHidden()) ? FILE_ATTR_HIDDEN : 0x00;
	rDirEntry.attribute |= (finder.IsSystem()) ? FILE_ATTR_SYSTEM : 0x00;
	rDirEntry.attribute |= (finder.IsDirectory()) ? FILE_ATTR_DIRECTORY : 0x00;
	rDirEntry.attribute |= (finder.IsArchived()) ? FILE_ATTR_ARCHIVE : 0x00;
	
	// Set the last accessed date and time
	t = CTime::GetCurrentTime();
	rDirEntry.accessDate = GetDateStamp(t);
	rDirEntry.date = GetDateStamp(t);
	rDirEntry.time = GetTimeStamp(t);
	
	// Set the creation dates and times
	finder.GetCreationTime(t);
	rDirEntry.createDate = GetDateStamp(t);
	rDirEntry.createTime = GetTimeStamp(t);
	
	rDirEntry.cluster = wMyCluster;
	
	// Seek to my entry in the root directory
	m_rImageFile.Seek(myOffset, CFile::begin);
	
	// Write the entry
	m_rImageFile.Write(&rDirEntry, 32);
}

void CFATFlashDoc::WriteShortFileEntry(const long myOffset, const CString& csFileName, const CFileFind &finder, WORD wMyCluster)
{
	CString	csFileTitle;
	CString	csFileExt;
	CString	csSubDirPath;
	CDirEntry	rDirEntry;
	CTime		t;
	CString	csCurrentDir = ".";
	CString	csParentDir = "..";
	CString	csFilename;
	CString	csAlias;

	// Get the name, title and extension of the file
	if( csFileName == csCurrentDir )
	{
		csFileTitle = csCurrentDir;
		csFileExt = "";
	}
	else if( csFileName == csParentDir )
	{
		csFileTitle = csParentDir;
		csFileExt = "";
	}
	else
	{
		csFileTitle = finder.GetFileTitle();
		if( csFileName.GetLength() > csFileTitle.GetLength() )
			csFileExt = csFileName.Right(csFileName.GetLength()-csFileTitle.GetLength()-1);
		else
			csFileExt = "";
	}
	
	// Determine the short filename
	GetShortFilename(finder, csFilename, csAlias);

	// Initialize the directory entry
	memset(&rDirEntry, 0, sizeof(rDirEntry));
	memset(&rDirEntry, ' ', (8+3));
	memcpy(&rDirEntry.name, csAlias.GetBuffer(0), 11);

	// Set the file attributes
	rDirEntry.attribute |= (finder.IsReadOnly()) ? FILE_ATTR_READONLY : 0x00;
	rDirEntry.attribute |= (finder.IsHidden()) ? FILE_ATTR_HIDDEN : 0x00;
	rDirEntry.attribute |= (finder.IsSystem()) ? FILE_ATTR_SYSTEM : 0x00;
	rDirEntry.attribute |= (finder.IsDirectory()) ? FILE_ATTR_DIRECTORY : 0x00;
	rDirEntry.attribute |= (finder.IsArchived()) ? FILE_ATTR_ARCHIVE : 0x00;
	
	// Set the last accessed date and time
	t = CTime::GetCurrentTime();
	rDirEntry.accessDate = GetDateStamp(t);
	rDirEntry.date = GetDateStamp(t);
	rDirEntry.time = GetTimeStamp(t);
	
	// Set the creation dates and times
	finder.GetCreationTime(t);
	rDirEntry.createDate = GetDateStamp(t);
	rDirEntry.createTime = GetTimeStamp(t);
	
	// Set the starting cluster and the file size
	rDirEntry.cluster = wMyCluster;
	rDirEntry.fileSize = finder.GetLength();
	
	// Seek to my entry in the root directory
	m_rImageFile.Seek(myOffset, CFile::begin);
	
	// Write the entry
	m_rImageFile.Write(&rDirEntry, 32);
}

BOOLEAN CFATFlashDoc::IsShortFilename(const CFileFind &finder)
{
	CString	csFileName;
	CString	csFileNameUpperCase;
	CString	csFileTitle;
	CString	csFileExt;
	
	// Get the name, title and extension of the file
	csFileNameUpperCase = csFileName = finder.GetFileName();
	csFileNameUpperCase.MakeUpper();
	csFileTitle = finder.GetFileTitle();
	if( csFileName.GetLength() > csFileTitle.GetLength() )
		csFileExt = csFileName.Right(csFileName.GetLength()-csFileTitle.GetLength()-1);
	else
		csFileExt = "";
	
	return( (csFileTitle.GetLength() <= 8)
		&& (csFileNameUpperCase == csFileName) );
}

void CFATFlashDoc::WriteFile(const CFileFind &finder, WORD wStartingCluster)
{
	CFile	rInputFile;
	CFileException fileException;
	WORD	wCurrentCluster, wNewCluster;
	DWORD	dwBytesToRead=0;
	DWORD	dwBytesRead=0;
	LPBYTE	lpBuf=NULL;
	DWORD	dwOffset;

	// Allocate memory for the buffer
	lpBuf = (LPBYTE)malloc(m_pbr.sectorsPerCluster*m_pbr.bytesPerSector);
	if( !lpBuf )
	{
		AfxMessageBox( _T("Insufficient memory."), MB_OK | MB_ICONERROR );
		exit(-1);
	}
	
	// Open the file to be copied
	CString	csFilePath = finder.GetFilePath();
	
	if ( !rInputFile.Open( (LPCTSTR)csFilePath, CFile::modeRead) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			(LPCTSTR)csFilePath, fileException.m_cause );
	}

	// Read the file a cluster at a time and copy to
	// available clusters.
	wCurrentCluster = wStartingCluster;
	dwBytesToRead = finder.GetLength();
	while( dwBytesToRead > 0 )
	{
		dwBytesRead = rInputFile.Read(lpBuf, (m_pbr.sectorsPerCluster*m_pbr.bytesPerSector) );
		dwBytesToRead -= dwBytesRead;
		
		// Seek to the current cluster
		dwOffset = m_nStartOfAvailableClusters+(wCurrentCluster-2)*(m_pbr.sectorsPerCluster*m_pbr.bytesPerSector);
		m_rImageFile.Seek(dwOffset, CFile::begin);

		// Write the data to the available cluster
		m_rImageFile.Write(lpBuf, dwBytesRead);

		// If there are more bytes in the file to be copied, then
		// we must adjust allocate a new cluster for the additional 
		// bytes.
		if( dwBytesToRead > 0 )
		{
			wNewCluster = AllocateCluster();
			if( wNewCluster <= 1 )
			{
				AfxMessageBox( _T("Insufficient space on destination filesystem."), MB_OK | MB_ICONERROR );
				exit(-1);
			}

			// Adjust the FAT entry to point to the next cluster
			WriteFATEntry(wCurrentCluster, wNewCluster);

			// Set the current cluster
			wCurrentCluster = wNewCluster;
		}
	}
	
	// Close the file that was copied
	rInputFile.Close();
	
	// Free any memory used
	if(lpBuf)
		free(lpBuf);
}

void CFATFlashDoc::WriteFATEntry(WORD wEntry, WORD wValue)
{
	WORD	wFatEntry;
	WORD	*pWord;
	
	// Read the existing data
	pWord = (WORD*)&m_fat[wEntry*3/2];
	wFatEntry = (WORD)*pWord;
	if( (wEntry%2) == 0 )
	{
		// cluster is an even cluster
		wFatEntry &= 0xf000;
		*pWord = (WORD)(wFatEntry | wValue);
	}
	else
	{
		// cluster is an odd cluster
		wFatEntry &= 0x000f;
		*pWord = (WORD)(wFatEntry | (wValue<<4));
	}
}

void CFATFlashDoc::SetBytesPerSector(const __int16 value)
{
	if( m_pbr.bytesPerSector != value )
	{
		m_pbr.bytesPerSector = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetSectorsPerCluster(const __int8 value)
{
	if( m_pbr.sectorsPerCluster != value )
	{
		m_pbr.sectorsPerCluster = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetReservedSectors(const __int16 value)
{
	if( m_pbr.reservedSectors != value )
	{
		m_pbr.reservedSectors = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetNumberOfFats(const __int8 value)
{
	if( m_pbr.numberOfFats != value )
	{
		m_pbr.numberOfFats = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetNumberOfRootDirEntries(const __int16 value)
{
	if( m_pbr.numberOfRootDirEntries != value )
	{
		m_pbr.numberOfRootDirEntries = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetNumberOfSectors(const __int16 value)
{
	if( m_pbr.numberOfSectors != value )
	{
		m_pbr.numberOfSectors = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetNumberOfHiddenSectors(const __int32 value)
{
	if( m_pbr.numberOfHiddenSectors != value )
	{
		m_pbr.numberOfHiddenSectors = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetVolumeId(const __int32 value)
{
	if( m_pbr.volumeId != value )
	{
		m_pbr.volumeId = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::SetVolumeName(const CString &value)
{
	// Initialize the value to all spaces
	memset(m_pbr.volumeName, ' ', sizeof(m_pbr.volumeName));
	memcpy(m_pbr.volumeName, (LPCTSTR)value, ((value.GetLength()<=sizeof(m_pbr.volumeName)) ? value.GetLength() : sizeof(m_pbr.volumeName)));
	
	// Indicate that the document has been modified
	SetModifiedFlag(TRUE);
}

void CFATFlashDoc::SetRootDirectory(const CString &value)
{
	if( m_csRootDirectory != value )
	{
		m_csRootDirectory = value;
		
		// Indicate that the document has been modified
		SetModifiedFlag(TRUE);
	}
}

void CFATFlashDoc::WriteLFNDirectoryEntry(const int nSequence, const long myOffset, const CFileFind &finder, const WORD wMyCluster, BOOL bLastEntry /*=FALSE*/)
{
	CLFNDirEntry	rDirEntry;
	char	filename[13];
	char	alias[11];
	CString	csFilename;
	CString	csShortFilename;
	CString	csAlias;
	char	*cPtr;
	__int8	sum;
	int		nMySequence;

	// Initialize the directory entry
	memset(&rDirEntry, 0, sizeof(rDirEntry));
	memset(filename, 0, sizeof(filename));

	// Set the sequence number
	// If this is the last entry, the sequence number is and'ed with 0x40
	nMySequence = ( bLastEntry ) ? (nSequence | 0x40) : nSequence;
	
	rDirEntry.ordinal = nMySequence;
	
	// Set the attributes to read-only, hidden, system, volume label 0x0F
	rDirEntry.attribute = 0x0F;
	
	// Set the cluster to 0
	rDirEntry.cluster = 0x0000;
	
	// Set the filename. Pad extra space with NULL terminator then 0xFF characters.
	csFilename = finder.GetFileName().Mid((nSequence-1)*13);
	csFilename = csFilename.Left(13);
	memcpy(filename, (LPCTSTR)csFilename, csFilename.GetLength());
	if( csFilename.GetLength() < 12 )
	{
		memset(&(filename[csFilename.GetLength()+1]), 0xFF, 13-csFilename.GetLength()-1);
	}

	cPtr = &filename[0];
	for( int i=0; i<=4; i++ )
	{
		if( *cPtr == 0xFF )
			rDirEntry.name1[i] = 0xFFFF;
		else
			rDirEntry.name1[i] = (*cPtr++);
	}

	for( i=0; i<=5; i++ )
	{
		if( *cPtr == 0xFF )
			rDirEntry.name2[i] = 0xFFFF;
		else
			rDirEntry.name2[i] = (*cPtr++);
	}

	for( i=0; i<=1; i++ )
	{
		if( *cPtr == 0xFF )
			rDirEntry.name3[i] = 0xFFFF;
		else
			rDirEntry.name3[i] = (*cPtr++);
	}

	// Compute and set the checksum
	GetShortFilename(finder, csShortFilename, csAlias);
	memcpy(alias, (LPCTSTR)csAlias, csAlias.GetLength());
	for( sum = i = 0; i < 11; i++ )
	{
		sum = (((sum & 0x01) << 7) | ((sum & 0xfe) >> 1)) + alias[i];
	}
	rDirEntry.checksum = sum;

	// Seek to my entry in the root directory
	m_rImageFile.Seek(myOffset, CFile::begin);
	
	// Write the entry
	m_rImageFile.Write(&rDirEntry, sizeof(CLFNDirEntry));
}

void CFATFlashDoc::GetShortFilename(const CFileFind &finder, CString& csFilename, CString& csAlias, BOOL bSave /*=FALSE*/)
{
	CString		csTempString;
	int			nLastPosition, nFirstPosition;
	CString		csFileTitle;
	CString		csFileExt;
	CString		csCurrentDir = ".";
	CString		csParentDir = "..";
	
	csFilename = finder.GetFileName();
	csAlias = csFilename;

	// Treat the files "." and ".." special
	if( csFilename == csCurrentDir )
	{
		csAlias = ".          ";
		return;
	}
	if( csFilename == csParentDir )
	{
		csAlias = "..         ";
		return;
	}
	if( IsShortFilename(finder) )
	{
		csAlias = finder.GetFileTitle();
		// Pad with spaces
		while( csAlias.GetLength() < 8 )
			csAlias = csAlias + " ";

		csFileTitle = finder.GetFileTitle();
		if( csFilename.GetLength() > csFileTitle.GetLength() )
			csFileExt = csFilename.Right(csFilename.GetLength()-csFileTitle.GetLength()-1);
		else
			csFileExt = "";
		csAlias = csAlias + csFileExt;
		// Pad with spaces
		while( csAlias.GetLength() < 11 )
			csAlias = csAlias + " ";
		return;
	}
	
	// 1. Remove all spaces from long filename
	csAlias.Remove(' ');

	// 2. Remove leading periods, trailing periods and
	//    extra periods prior to the last period.
	csTempString = csAlias.SpanIncluding(".");
	csAlias = csAlias.Right(csAlias.GetLength()-csTempString.GetLength());

	// Remove the trailing periods
	while( csAlias.GetAt(csAlias.GetLength()-1) == '.' )
		csAlias = csAlias.Left(csAlias.GetLength()-1);

	nLastPosition = csAlias.ReverseFind('.');
	if( nLastPosition >= 0 )
	{
		while( (nLastPosition >= 0) && ((nFirstPosition=csAlias.Find('.')) != nLastPosition) )
		{
			// Remove all but last period
			csAlias.Delete(nFirstPosition);
			nLastPosition = csAlias.ReverseFind('.');
		}
	}

	// 3. Translate all illegal 8.3 characters to "_" underscore
	csTempString = csAlias.SpanIncluding("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$%-_@~'!(){}^#&.");
	while( csTempString.GetLength() < csAlias.GetLength() )
	{
		csAlias.SetAt(csTempString.GetLength(),'_');
		csTempString = csAlias.SpanIncluding("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$%-_@~'!(){}^#&");
	}

	// Truncate the filename title (no extension) to 6 characters, and the extension to 3 characters
	if( csAlias.ReverseFind('.') >= 0 )
	{
		// We have a title and file extension
		csFileTitle = csAlias.Left(csAlias.ReverseFind('.'));
		if( csFileTitle.GetLength() > 6 )
		{
			if( csFileTitle.GetLength() > 8 )
			{
				csFileTitle = csFileTitle.Left(6);
				csFileTitle = csFileTitle + "~1";
			}
		}
		csFileExt = csAlias.Right(csAlias.GetLength()-csAlias.ReverseFind('.')-1);
		if( csFileExt.GetLength() > 3 )
		{
			csFileExt = csFileExt.Left(3);
		}
		
		csAlias = csFileTitle;
		// Pad with spaces
		while( csAlias.GetLength() < 8 )
			csAlias = csAlias + " ";
		csAlias = csAlias + csFileExt;
		// Pad with spaces
		while( csAlias.GetLength() < 11 )
			csAlias = csAlias + " ";
		
		csFilename = csFileTitle + "." + csFileExt;
	}
	else
	{
		// We have no file extension
		if( csAlias.GetLength() > 6 )
		{
			if( csAlias.GetLength() > 8 )
			{
				csAlias = csAlias.Left(6);
				csAlias = csAlias + "~1";
			}
			csFilename = csAlias;
			// Pad with spaces
			while( csAlias.GetLength() < 11 )
				csAlias = csAlias + " ";
		}
		else
		{
			csFilename = csAlias;
			// Pad with spaces
			while( csAlias.GetLength() < 11 )
				csAlias = csAlias + " ";
		}
	}
	
	csAlias.MakeUpper();
	csFilename.MakeUpper();
}

long CFATFlashDoc::ExportFilesystemPdb() 
{
	long	retval = 0;
	CString	csTempFilePath;
	
	// Open the temporary filesystem image file
	csTempFilePath = m_csExportFilePath + ".tmp000";
	if ( !(m_rImageFile.Open( (LPCTSTR)csTempFilePath, CFile::modeCreate |   
		CFile::modeReadWrite )) )
	{
		// Inform the user that the temporary file cannot be opened
		CString	csMessage = "Unable to open/create temporary file: ";
		csMessage += (LPCTSTR)csTempFilePath;
		AfxMessageBox( _T((LPCTSTR)csMessage), MB_OK | MB_ICONERROR );
		retval = -1;
		goto Exit;
	}
		
	// First, create a normal filesystem image file
	if( (retval = ExportFilesystemImage(TRUE)) == errNone )
	{
		// Open the PDB file for output
		if ( !(m_rPdbFile.Open( (LPCTSTR)m_csExportFilePath, CFile::modeCreate |   
			CFile::modeReadWrite )) )
		{
			retval = -1;
			goto CloseImageAndExit;
		}
		
		// Read data from temporary (image) file and write a pdb file
		retval = StorePdb();
		
		// Close the PDB file
		m_rPdbFile.Close();
	}

CloseImageAndExit:
	// Close and delete the temporary image file
	m_rImageFile.Close();
	
	TRY
	{
		CFile::Remove( (LPCTSTR)csTempFilePath );
	}
	CATCH( CFileException, e )
	{
#ifdef _DEBUG
        afxDump << "File " << (LPCTSTR)csTempFilePath << " cannot be removed\n";
#endif
	}
	END_CATCH

Exit:
	return (retval);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class:   CFATFlashDoc
//
// Method:  StorePDB()
//
// Description: This method is called to store a file stream image to 
//				a PDB file.
//
// Parameter(s): None.
//
// Return Value(s): 0 - no error
//                  <0 - error code.
//
///////////////////////////////////////////////////////////////////////////////
long CFATFlashDoc::StorePdb(void)
{
    long retval = 0;

    // Store the PDB header
    retval = StoreDbHeader();
    if (retval) {
        goto Exit;
    }
	
	// Store the Record Header List
	retval = StoreRecordList();
	if (retval) {
        goto Exit;
	}
	
	// Store the AppInfo Block
	retval = StoreAppInfo();
	if (retval) {
        goto Exit;
	}
	
	// Store the SortInfo Block
	retval = StoreSortInfo();
	if (retval) {
        goto Exit;
	}
	
	
    // Store the Records
    retval = StoreRecords();
	
Exit:
    return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
// Class:   CFATFlashDoc
//
// Method:  StoreDbHeader()
//
// Description: This method stores the database header to the PDB file.
//
// Parameter(s): None.
//
// Return Value(s): 0 - no error
//                  <0 - error code.
//
///////////////////////////////////////////////////////////////////////////////
long CFATFlashDoc::StoreDbHeader(void)
{
    long retval = 0;
	
	memset(&m_dbHeader, 0, sizeof(m_dbHeader));

	// Get the header info from the m_pDBInfo structure in CPcMgr
    strcpy((char*)m_dbHeader.name, EDK_FILESTREAM_NAME);
	m_dbHeader.creator = EDK_FILESTREAM_CREATOR;			// database creator id
	m_dbHeader.type = EDK_FILESTREAM_TYPE;					// database type
	m_dbHeader.attributes = dmHdrAttrFileStream | dmHdrAttrBackup;			// database attributes
	m_dbHeader.version = EDK_FILESTREAM_VERSION;			// version of database
	
	// Note: We store all dates in desktop format, because this is the way
	// the Backup and Install Conduits expect them. However, if you 
	// exported the database using POSE, it would be in Palm date format.
	// Note: the Mac and PC date formats are not the same. A PDB file
	// created on the Mac and the PC are not identical.
	m_dbHeader.creationDate = EDK_FILESTREAM_CREATEDATE;			// creation date of database
	m_dbHeader.modificationDate = EDK_FILESTREAM_MODDATE;			// latest modification date
	m_dbHeader.lastBackupDate = EDK_FILESTREAM_BACKUPDATE;			// latest backup date
	m_dbHeader.modificationNumber = EDK_FILESTREAM_MODNUMBER;		// modification number of database
	
	// Set the unique id seed to 0 because it is done this way on Backup/Restore conduits
	m_dbHeader.uniqueIDSeed = 0x00000000L;			// used to generate unique IDs
	
	// Set the AppInfoID appropriately
	DWORD	dwAppInfoSize = 0;
	m_dbHeader.appInfoID = 0;
	
	// Set the SortInfoID appropriately 
	DWORD	dwSortInfoSize = 0;
	m_dbHeader.sortInfoID = 0;

	// We will only write one record list, so set the nextRecordListID to 0
	m_dbHeader.recordList.nextRecordListID = 0;

	// Determine the number of records
	DWORD	dwRecordCount = m_pbr.bytesPerSector * m_pbr.numberOfSectors / FS_RECORDSIZE;
	m_dbHeader.recordList.numRecords = (unsigned short)dwRecordCount;

    // Convert to Motorola format and write
	ConvertDBHdr(&m_dbHeader);

    // Only write out the first entry bytes if there are no records
	if( dwRecordCount == 0 ) {
		m_rPdbFile.Write((BYTE*)&m_dbHeader, sizeof(m_dbHeader));
	} else {
		m_rPdbFile.Write((BYTE*)&m_dbHeader, sizeof(m_dbHeader) - sizeof(WORD));
	}
	
	// Convert the database header back
	ConvertDBHdr(&m_dbHeader);
	
    // Save an offset position of where to start storing record data
	m_dwCurrentRecordOffset = sizeof(DatabaseHdrType) + dwRecordCount * sizeof(RecordEntryType) + dwAppInfoSize + dwSortInfoSize;

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
// Class:   CFATFlashDoc
//
// Method:  StoreAppInfo()
//
// Description: This method stores the AppInfo Block to the PDB file.
//
// Parameter(s): None.
//
// Return Value(s): 0 - no error
//                  <0 - error code.
//
///////////////////////////////////////////////////////////////////////////////
long CFATFlashDoc::StoreAppInfo(void)
{
    long retval = 0;

	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
// Class:   CFATFlashDoc
//
// Method:  StoreSortInfo()
//
// Description: This method stores the SortInfo Block to the PDB file.
//
// Parameter(s): None.
//
// Return Value(s): 0 - no error
//                  <0 - error code.
//
///////////////////////////////////////////////////////////////////////////////
long CFATFlashDoc::StoreSortInfo(void)
{
    long retval = 0;

	return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
// Class:   CFATFlashDoc
//
// Method:  StoreRecordList()
//
// Description: This method stores the Record Header List to the PDB file.
//
// Parameter(s): None.
//
// Return Value(s): 0 - no error
//                  <0 - error code.
//
///////////////////////////////////////////////////////////////////////////////
long CFATFlashDoc::StoreRecordList(void)
{
    long retval = 0;
    RecordEntryType recType;
    DWORD dwRecID;
	WORD	wDummyValue = 0x0000;
	unsigned char	lpszBuffer[FS_RECORDSIZE];
	int		nByteCount;
	
    // Loop through all records and build the record header list.
	// Each record in a file stream is 4096 bytes, plus a "DBLK" header.
	nByteCount = m_rImageFile.Read(lpszBuffer, FS_RECORDSIZE);
	while( nByteCount > 0 ) {

		// Start building a RecordEntryType structure
		recType.attributes      = 0;
		recType.attributes      |= (CATEGORY_UNFILED & 0x0F); 
		
		// Set the record attributes appropriately
		recType.attributes      |= ((ATTR_MODIFIED << 4) & 0xF0);
		
		// Get the Unique ID
		dwRecID = UID_NEW_RECORD;
		
		// Put 4 bytes into three bytes
		recType.uniqueID[2]  = (Byte)dwRecID & 0xff;
		recType.uniqueID[1]  = (Byte)((dwRecID >> 8) & 0xff);
		recType.uniqueID[0]  = (Byte)((dwRecID >> 16) & 0xff);
		
		// Set the localChunkID and convert to Motorola format.
		recType.localChunkID = m_dwCurrentRecordOffset;
		recType.localChunkID = CFATFlashDoc::SwapDWordToMotor(recType.localChunkID);

		// Each record in a file stream is 4096 bytes, plus a 'DBLK' header plus DWORD record length (Total 8 bytes).
		m_dwCurrentRecordOffset += FS_RECORDSIZE + 8;
		m_rPdbFile.Write((BYTE*)&recType, sizeof(RecordEntryType));
		
		// Get the next record
		nByteCount = m_rImageFile.Read(lpszBuffer, FS_RECORDSIZE);
    }

	if( !retval ) {
		// Write 2 dummy bytes because PDB spec includes
		//	 two extra (unused) bytes
        m_rPdbFile.Write( &wDummyValue, sizeof(WORD));
	}

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
//
// Class:   CFATFlashDoc
//
// Method:  StoreRecords()
//
// Description: This method stores the record data to the PDB file.
//
// Parameter(s): None.
//
// Return Value(s): 0 - no error
//                  <0 - error code.
//
///////////////////////////////////////////////////////////////////////////////
long CFATFlashDoc::StoreRecords(void)
{
    long retval = 0;
	unsigned char	lpszBuffer[FS_RECORDSIZE];
	DWORD		dwByteCount;
	const char	lpszHeader[] = "DBLK";
	
    // Reset to the beginning of the image file
	m_rImageFile.SeekToBegin();
	
	// Loop through all records and write them out
	// Each record in a file stream is 4096 bytes, plus a "DBLK" header.
	memset( lpszBuffer, 0, sizeof(lpszBuffer) );
	dwByteCount = m_rImageFile.Read(lpszBuffer, FS_RECORDSIZE);
	while( dwByteCount > 0 ) {

		// Prepend each record with a "DBLK" header, and write out each record
		m_rPdbFile.Write((BYTE*)&lpszHeader, sizeof(lpszHeader)-1);
		dwByteCount = CFATFlashDoc::SwapDWordToMotor((DWORD)dwByteCount);
		m_rPdbFile.Write((BYTE*)&dwByteCount, sizeof(dwByteCount));
		m_rPdbFile.Write((BYTE*)&lpszBuffer, sizeof(lpszBuffer));
		
		// Clear our buffer each time
		memset( lpszBuffer, 0, sizeof(lpszBuffer) );

		// Get the next record
		dwByteCount = m_rImageFile.Read(lpszBuffer, FS_RECORDSIZE);
    }

	return retval;
}

/////////////////////////////////////////////////////////////////////////
//
// METHOD:		ConvertDBHdr
//
// DESCRIPTION:	Converts an Intel-format database header to Motorola format.
//				This is only necessary on Windows.
//
// PARAMETERS:	DatabaseHdrPtr pHdr		- Pointer to header
//
// RETURNED:	None.
//
/////////////////////////////////////////////////////////////////////////
void CFATFlashDoc::ConvertDBHdr(DatabaseHdrPtr pHdr)
{
    pHdr->attributes =         CFATFlashDoc::FlipWord(pHdr->attributes);
	pHdr->version =            CFATFlashDoc::FlipWord(pHdr->version);

	pHdr->creationDate =       CFATFlashDoc::SwapDWordToIntel(pHdr->creationDate);
	pHdr->modificationDate =   CFATFlashDoc::SwapDWordToIntel(pHdr->modificationDate);
	pHdr->lastBackupDate =	    CFATFlashDoc::SwapDWordToIntel(pHdr->lastBackupDate);
	pHdr->modificationNumber =	CFATFlashDoc::SwapDWordToIntel(pHdr->modificationNumber);

	pHdr->appInfoID =          CFATFlashDoc::SwapDWordToIntel(pHdr->appInfoID);
	pHdr->sortInfoID =	        CFATFlashDoc::SwapDWordToIntel(pHdr->sortInfoID);

	pHdr->type =               CFATFlashDoc::SwapDWordToIntel(pHdr->type);
	pHdr->creator =            CFATFlashDoc::SwapDWordToIntel(pHdr->creator);

	pHdr->uniqueIDSeed =       CFATFlashDoc::SwapDWordToIntel(pHdr->uniqueIDSeed);

	pHdr->recordList.numRecords 	  = CFATFlashDoc::FlipWord((WORD)pHdr->recordList.numRecords);
}

/////////////////////////////////////////////////////////////////////////
//
// METHOD:		FlipWord
//
// DESCRIPTION:	Flips an Intel-format Word to Motorola format.
//				This is only necessary on Windows.
//
// PARAMETERS:	Word wSubj		- Word to flip
//
// RETURNED:	result.
//
/////////////////////////////////////////////////////////////////////////
WORD CFATFlashDoc::FlipWord(WORD wSubj)
{
    WORD wAnswer;

    wAnswer = (WORD)(((wSubj >> 8) & 0xff) | ((wSubj & 0xff) << 8));

    return wAnswer;
}

/////////////////////////////////////////////////////////////////////////////
//
//	Function:		SwapDWordToIntel()                 ** Static **
//
//	Description:	Converts an Motorola 4 byte DWORD to a 4 byte Intel DWORD.
//					Makes the least significant byte of
//					the passed in subject the most significant byte of the
//					returned answer.
//
//	Parameters:		DWORD value to swap
//
//	Returns:		Swapped DWORD
//
/////////////////////////////////////////////////////////////////////////////
unsigned long CFATFlashDoc::SwapDWordToIntel(DWORD dwSubj)
{
    DWORD  dwAnswer = 0;
    BYTE   oneByte;
    int    maxCars = sizeof(DWORD) - 1;    // 3

    for (int i=0; i <= maxCars; i++)
    {
        oneByte = (BYTE)((dwSubj >> ((maxCars - i) * 8)) & 0xFF);
        dwAnswer |= oneByte << (i * 8);
    }
    return(dwAnswer);
}

/////////////////////////////////////////////////////////////////////////////
//
//	Function:		SwapDWordToMotor()                 ** Static **
//
//	Description:	Converts an intel 4 byte DWORD to a 4 byte Motorola DWORD.
//					Makes the least significant byte of
//					the passed in subject the most significant byte of the
//					returned answer.
//
//	Parameters:		DWORD value to swap
//
//	Returns:		Swapped DWORD
//
/////////////////////////////////////////////////////////////////////////////
unsigned long CFATFlashDoc::SwapDWordToMotor(unsigned long dwSubj)
{
	DWORD  dwAnswer = 0;
	BYTE   oneByte;
	int    maxCars = sizeof(DWORD) - 1;    // 3

	for (int i=0; i <= maxCars; i++) {
		oneByte  = (BYTE)((dwSubj >> (i * 8)) & 0xFF);
		dwAnswer |= oneByte << ((maxCars - i) * 8);
	}
	return(dwAnswer);
}


