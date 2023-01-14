/////////////////////////////////////////////////////////////////////////////
//
// FAT Flash FATFlashView.h Header File
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

#if !defined(AFX_FATFLASHVIEW_H__85397FD0_BF29_11D5_B90D_00A0CC64D527__INCLUDED_)
#define AFX_FATFLASHVIEW_H__85397FD0_BF29_11D5_B90D_00A0CC64D527__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFATFlashView : public CFormView
{
protected: // create from serialization only
	CFATFlashView();
	DECLARE_DYNCREATE(CFATFlashView)

public:
	//{{AFX_DATA(CFATFlashView)
	enum { IDD = IDD_FATFLASH_FORM };
	CString	m_csNumberOfFats;
	CString	m_csBytesPerSector;
	CString	m_csSectorsPerCluster;
	CString	m_csNumberOfHiddenSectors;
	CString	m_csNumberOfRootDirEntries;
	CString	m_csNumberOfSectors;
	CString	m_csReservedSectors;
	CString	m_csVolumeId;
	CString	m_csVolumeName;
	CString	m_csRootDirectory;
	//}}AFX_DATA

// Attributes
public:
	CFATFlashDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFATFlashView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFATFlashView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CFATFlashView)
	afx_msg void OnKillfocusBytespersector();
	afx_msg void OnKillfocusSectorspercluster();
	afx_msg void OnKillfocusReservedsectors();
	afx_msg void OnKillfocusNumberoffats();
	afx_msg void OnKillfocusNumberofrootdirentries();
	afx_msg void OnKillfocusNumberofsectors();
	afx_msg void OnKillfocusNumberofhiddensectors();
	afx_msg void OnKillfocusVolumeid();
	afx_msg void OnKillfocusVolumename();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKillfocusRootdirectory();
	afx_msg void OnButtonbrowse();
	afx_msg void OnFileSetrootdir();
	afx_msg void OnChangeRootdirectory();
	afx_msg void OnChangeVolumename();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FATFlashView.cpp
inline CFATFlashDoc* CFATFlashView::GetDocument()
   { return (CFATFlashDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FATFLASHVIEW_H__85397FD0_BF29_11D5_B90D_00A0CC64D527__INCLUDED_)
