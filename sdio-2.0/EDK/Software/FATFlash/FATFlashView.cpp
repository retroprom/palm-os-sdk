/////////////////////////////////////////////////////////////////////////////
//
// FAT Flash FATFlashView.cpp Src File
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
#include "FATFlash.h"

#include "FATFlashDoc.h"
#include "FATFlashView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFATFlashView

IMPLEMENT_DYNCREATE(CFATFlashView, CFormView)

BEGIN_MESSAGE_MAP(CFATFlashView, CFormView)
	//{{AFX_MSG_MAP(CFATFlashView)
	ON_EN_KILLFOCUS(IDC_BYTESPERSECTOR, OnKillfocusBytespersector)
	ON_EN_KILLFOCUS(IDC_SECTORSPERCLUSTER, OnKillfocusSectorspercluster)
	ON_EN_KILLFOCUS(IDC_RESERVEDSECTORS, OnKillfocusReservedsectors)
	ON_EN_KILLFOCUS(IDC_NUMBEROFFATS, OnKillfocusNumberoffats)
	ON_EN_KILLFOCUS(IDC_NUMBEROFROOTDIRENTRIES, OnKillfocusNumberofrootdirentries)
	ON_EN_KILLFOCUS(IDC_NUMBEROFSECTORS, OnKillfocusNumberofsectors)
	ON_EN_KILLFOCUS(IDC_NUMBEROFHIDDENSECTORS, OnKillfocusNumberofhiddensectors)
	ON_EN_KILLFOCUS(IDC_VOLUMEID, OnKillfocusVolumeid)
	ON_EN_KILLFOCUS(IDC_VOLUMENAME, OnKillfocusVolumename)
	ON_WM_KILLFOCUS()
	ON_EN_KILLFOCUS(IDC_ROOTDIRECTORY, OnKillfocusRootdirectory)
	ON_BN_CLICKED(IDC_BUTTONBROWSE, OnButtonbrowse)
	ON_COMMAND(ID_FILE_SETROOTDIR, OnFileSetrootdir)
	ON_EN_CHANGE(IDC_ROOTDIRECTORY, OnChangeRootdirectory)
	ON_EN_CHANGE(IDC_VOLUMENAME, OnChangeVolumename)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFATFlashView construction/destruction

CFATFlashView::CFATFlashView()
	: CFormView(CFATFlashView::IDD)
{
	//{{AFX_DATA_INIT(CFATFlashView)
	m_csNumberOfFats = _T("");
	m_csBytesPerSector = _T("");
	m_csSectorsPerCluster = _T("");
	m_csNumberOfHiddenSectors = _T("");
	m_csNumberOfRootDirEntries = _T("");
	m_csNumberOfSectors = _T("");
	m_csReservedSectors = _T("");
	m_csVolumeId = _T("");
	m_csVolumeName = _T("");
	m_csRootDirectory = _T("");
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

CFATFlashView::~CFATFlashView()
{
}

void CFATFlashView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFATFlashView)
	DDX_Text(pDX, IDC_NUMBEROFFATS, m_csNumberOfFats);
	DDV_MaxChars(pDX, m_csNumberOfFats, 3);
	DDX_Text(pDX, IDC_BYTESPERSECTOR, m_csBytesPerSector);
	DDV_MaxChars(pDX, m_csBytesPerSector, 5);
	DDX_Text(pDX, IDC_SECTORSPERCLUSTER, m_csSectorsPerCluster);
	DDV_MaxChars(pDX, m_csSectorsPerCluster, 3);
	DDX_Text(pDX, IDC_NUMBEROFHIDDENSECTORS, m_csNumberOfHiddenSectors);
	DDV_MaxChars(pDX, m_csNumberOfHiddenSectors, 10);
	DDX_Text(pDX, IDC_NUMBEROFROOTDIRENTRIES, m_csNumberOfRootDirEntries);
	DDV_MaxChars(pDX, m_csNumberOfRootDirEntries, 5);
	DDX_Text(pDX, IDC_NUMBEROFSECTORS, m_csNumberOfSectors);
	DDV_MaxChars(pDX, m_csNumberOfSectors, 5);
	DDX_Text(pDX, IDC_RESERVEDSECTORS, m_csReservedSectors);
	DDV_MaxChars(pDX, m_csReservedSectors, 5);
	DDX_Text(pDX, IDC_VOLUMEID, m_csVolumeId);
	DDV_MaxChars(pDX, m_csVolumeId, 10);
	DDX_Text(pDX, IDC_VOLUMENAME, m_csVolumeName);
	DDV_MaxChars(pDX, m_csVolumeName, 11);
	DDX_Text(pDX, IDC_ROOTDIRECTORY, m_csRootDirectory);
	DDV_MaxChars(pDX, m_csRootDirectory, 255);
	//}}AFX_DATA_MAP
}

BOOL CFATFlashView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CFATFlashView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}

/////////////////////////////////////////////////////////////////////////////
// CFATFlashView diagnostics

#ifdef _DEBUG
void CFATFlashView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFATFlashView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CFATFlashDoc* CFATFlashView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFATFlashDoc)));
	return (CFATFlashDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFATFlashView message handlers

void CFATFlashView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	char	lpszString[32];

	if( lHint == VIEW_OVERWRITES_DOC )
	{
		// Update all the doc data structures with view values
		GetDocument()->SetRootDirectory( m_csRootDirectory );
		
		GetDocument()->SetBytesPerSector( atoi((LPCTSTR)m_csBytesPerSector) );
		
		GetDocument()->SetSectorsPerCluster( atoi((LPCTSTR)m_csSectorsPerCluster) );
		
		GetDocument()->SetReservedSectors( atoi((LPCTSTR)m_csReservedSectors) );
		
		GetDocument()->SetNumberOfFats( atoi((LPCTSTR)m_csNumberOfFats) );
		
		GetDocument()->SetNumberOfRootDirEntries( atoi((LPCTSTR)m_csNumberOfRootDirEntries) );
		
		GetDocument()->SetNumberOfSectors( atoi((LPCTSTR)m_csNumberOfSectors) );
		
		GetDocument()->SetReservedSectors( atoi((LPCTSTR)m_csReservedSectors) );
		
		GetDocument()->SetNumberOfHiddenSectors( atoi((LPCTSTR)m_csNumberOfHiddenSectors) );
		
		GetDocument()->SetVolumeId( atoi((LPCTSTR)m_csVolumeId) );
		
		GetDocument()->SetVolumeName( m_csVolumeName );
	}
	else
	{
		// Update all the view data structures with doc values
		m_csRootDirectory = GetDocument()->m_csRootDirectory;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.bytesPerSector );
		m_csBytesPerSector = lpszString;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.sectorsPerCluster );
		m_csSectorsPerCluster = lpszString;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.reservedSectors );
		m_csReservedSectors = lpszString;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.numberOfFats );
		m_csNumberOfFats = lpszString;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.numberOfRootDirEntries );
		m_csNumberOfRootDirEntries = lpszString;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.numberOfSectors );
		m_csNumberOfSectors = lpszString;
		
		sprintf( lpszString, "%d", GetDocument()->m_pbr.reservedSectors );
		m_csReservedSectors = lpszString;
		
		sprintf( lpszString, "%ld", GetDocument()->m_pbr.numberOfHiddenSectors );
		m_csNumberOfHiddenSectors = lpszString;
		
		sprintf( lpszString, "%ld", GetDocument()->m_pbr.volumeId );
		m_csVolumeId = lpszString;
		
		memset( lpszString, 0, sizeof(lpszString) );
		memcpy( lpszString, GetDocument()->m_pbr.volumeName, sizeof(GetDocument()->m_pbr.volumeName) );
		m_csVolumeName = lpszString;
	}

	// Finally, use DDX to update the data in the dialog
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusRootdirectory() 
{
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	
	// Set the Document data
	GetDocument()->SetRootDirectory(m_csRootDirectory);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnChangeRootdirectory() 
{
	OnKillfocusRootdirectory();
}

void CFATFlashView::OnKillfocusBytespersector() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csBytesPerSector);
	iTemp = 512;	// SD limits bytes per sector to 512
	sprintf( lpszTempString, "%d", iTemp );
	m_csBytesPerSector = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetBytesPerSector(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusSectorspercluster() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csSectorsPerCluster);
	iTemp = 1;	// We limit sectors per cluster to 1
	sprintf( lpszTempString, "%d", iTemp );
	m_csSectorsPerCluster = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetSectorsPerCluster(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusReservedsectors() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csReservedSectors);
	iTemp = (iTemp>0) ? iTemp : 1;	// Must be >=1
	sprintf( lpszTempString, "%d", iTemp );
	m_csReservedSectors = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetReservedSectors(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusNumberoffats() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csNumberOfFats);
	iTemp = (iTemp>2) ? iTemp : 2;	// Must be >=2
	sprintf( lpszTempString, "%d", iTemp );
	m_csNumberOfFats = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetNumberOfFats(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusNumberofrootdirentries() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csNumberOfRootDirEntries);
	iTemp = 512;	// SD specifies must be 512
	sprintf( lpszTempString, "%d", iTemp );
	m_csNumberOfRootDirEntries = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetNumberOfRootDirEntries(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusNumberofsectors() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csNumberOfSectors);
	iTemp = (iTemp>=36) ? iTemp : 0x0100;	// Must be 36<=iTemp<4085
	iTemp = (iTemp<4085) ? iTemp : 4085;	// Must be 36<=iTemp<4085
	sprintf( lpszTempString, "%d", iTemp );
	m_csNumberOfSectors = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetNumberOfSectors(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusNumberofhiddensectors() 
{
	char		lpszTempString[256];
	int			iTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	iTemp = atoi(m_csNumberOfHiddenSectors);
	iTemp = (iTemp>=0) ? iTemp : 0;	// Must be >=0
	sprintf( lpszTempString, "%d", iTemp );
	m_csNumberOfHiddenSectors = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetNumberOfHiddenSectors(iTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusVolumeid() 
{
	char		lpszTempString[256];
	long		lTemp;
	
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Check the data from form
	lTemp = atol(m_csVolumeId);
	lTemp = (lTemp>=0) ? lTemp : 0;	// Must be >=0
	sprintf( lpszTempString, "%ld", lTemp );
	m_csVolumeId = lpszTempString;
	
	// Set the Document data
	GetDocument()->SetVolumeId(lTemp);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}

void CFATFlashView::OnKillfocusVolumename() 
{
	// Read data from view using DataExchange
	UpdateData(TRUE);

	// Set the Document data
	GetDocument()->SetVolumeName(m_csVolumeName);

	// Update the Document data using DataExchange
	UpdateData(FALSE);
}


void CFATFlashView::OnChangeVolumename() 
{
	OnKillfocusVolumename();
}

void CFATFlashView::OnKillFocus(CWnd* pNewWnd) 
{
	CFormView::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	
}


void CFATFlashView::OnButtonbrowse() 
{
	OnFileSetrootdir();
}

void CFATFlashView::OnFileSetrootdir() 
{
	// TODO: Add a root directory selection dialog
	
}

