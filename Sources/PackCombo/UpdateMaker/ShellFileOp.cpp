// ShellFileOp.cpp: implementation of the CShellFileOp class.
//	
//	Tim Johnson		timj@progsoc.uts.edu.au
//	Written : July 1998
//
//	Copyright 1998
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellFileOp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//
// CShellFileOp ()
//
// Basic constructor
//
CShellFileOp::CShellFileOp()
{
	m_lMaxSrcCount = 0;
	m_lMaxDestCount = 0;
	m_lCurSrcCount = 0;
	m_lCurDestCount = 0;
	m_pSrc = NULL;
	m_pDest = NULL;
	m_pTo = NULL;
	m_pFrom = NULL;
	m_pTitle = NULL;

	m_FileOp.hwnd = NULL; 
    m_FileOp.wFunc = 0; 
    m_FileOp.pFrom = NULL; 
    m_FileOp.pTo = NULL; 
    m_FileOp.fFlags = 0; 
    m_FileOp.fAnyOperationsAborted = FALSE; 
    m_FileOp.hNameMappings = NULL; 
    m_FileOp.lpszProgressTitle = NULL; 
}

//
// CShellFileOp
//
// Complex constructor - performs the operation straight away
//
CShellFileOp::CShellFileOp( const HWND phWnd, 
						   const UINT wFunc,
						   const CString sFrom,
						   const CString sTo,
						   const FILEOP_FLAGS fFlags,
						   const CString sProgressTitle)
{
	m_lMaxSrcCount = 0;
	m_lMaxDestCount = 0;
	m_lCurSrcCount = 0;
	m_lCurDestCount = 0;
	m_pSrc = NULL;
	m_pDest = NULL;
	m_pTo = NULL;
	m_pFrom = NULL;
	m_pTitle = NULL;

	m_FileOp.hwnd = NULL; 
    m_FileOp.wFunc = 0; 
    m_FileOp.pFrom = NULL; 
    m_FileOp.pTo = NULL; 
    m_FileOp.fFlags = 0; 
    m_FileOp.fAnyOperationsAborted = FALSE; 
    m_FileOp.hNameMappings = NULL; 
    m_FileOp.lpszProgressTitle = NULL; 

	SetParent(phWnd);
	AddFile(SH_SRC_FILE, sFrom);
	AddFile(SH_DEST_FILE, sTo);
	SetFlags(fFlags);
	SetTitle(sProgressTitle);

	switch (wFunc)
	{
	case FO_COPY:
		CopyFiles();
		break;
	case FO_DELETE:
		DeleteFiles();
		break;
	case FO_MOVE:
		MoveFiles();
		break;
	case FO_RENAME:
		RenameFiles();
		break;
	default:
		break;
	}
}

//
// ~CShellFileOp()
//
// deconstructor - kill all memory before object destruction
//
CShellFileOp::~CShellFileOp()
{
	//free all malloc'd blocks
    if (m_pTo) 
		free ((void *)m_pTo );
    if (m_pFrom) 
		free ((void *)m_pFrom);
    if (m_pTitle) 
		free ((void *)m_pTitle);
}

//
// long CopyFiles ( )
//
// Function to perform copy operation
//
long CShellFileOp::CopyFiles()
{
	m_FileOp.fAnyOperationsAborted = FALSE; 
	m_FileOp.wFunc = FO_COPY;
	return SHFileOperation(&m_FileOp);
}

//
// long DeleteFiles ( )
//
// Function to perform delete operation
//
long CShellFileOp::DeleteFiles()
{
	m_FileOp.fAnyOperationsAborted = FALSE; 
	m_FileOp.wFunc = FO_DELETE;
	return SHFileOperation(&m_FileOp);
}

//
// long MoveFiles ( )
//
// Function to perform move operation
//
long CShellFileOp::MoveFiles()
{
	m_FileOp.fAnyOperationsAborted = FALSE; 
	m_FileOp.wFunc = FO_MOVE;
	return SHFileOperation(&m_FileOp);
}

//
// long RenameFiles ( )
//
// Function to perform rename operation
//
long CShellFileOp::RenameFiles()
{
	m_FileOp.fAnyOperationsAborted = FALSE; 
	m_FileOp.wFunc = FO_RENAME;
	return SHFileOperation(&m_FileOp);
}

//
// void ClearFiles ( const int iSrcDest )
//
// Clears a list of files
//
void CShellFileOp::ClearFiles(const int iSrcDest)
{
	if (iSrcDest == SH_SRC_FILE)
	{
		if (m_pFrom) 
		{
			free ((void *)m_pFrom);
			m_pFrom = NULL;
			m_FileOp.pFrom = m_pFrom;
			m_lMaxSrcCount = 0;
			m_lCurSrcCount = 0;
		}	
	}
	else
	{
		if (m_pTo) 
		{
			free ((void *)m_pTo );
			m_pTo = NULL;
			m_FileOp.pTo = m_pTo;
			m_lMaxDestCount = 0;
			m_lCurDestCount = 0;
		}
	}
}

//
// void AddFile ( const int iSrcDest, CString sFile )
//
// Adds another filename to the end of the current string
//
void CShellFileOp::AddFile(const int iSrcDest, const CString sFile)
{
	int iLength = sFile.GetLength() + 1;	//+1 for the null

	if (iSrcDest == SH_SRC_FILE)
	{
		//check enough allocated space...
		if ((m_lCurSrcCount + iLength + 1)> m_lMaxSrcCount)	//+1 for the double null termination...
		{
			//have to get more mem.
			GrabMem(iSrcDest, (m_lCurSrcCount + iLength + 1));
		}
		//now theres enough memory!  yay.
		//now copy the filename in
		strcpy(m_pSrc, (LPCTSTR)sFile);

		//go to end of this files null term.
		m_pSrc += iLength;
		m_lCurSrcCount += iLength;

		//always keep it double null terminated, but don't
		// increment past it incase we want to add more files
		m_pSrc[0] = 0;
	}
	else
	{
		//check enough allocated space...
		if ((m_lCurDestCount + iLength + 1)> m_lMaxDestCount)	//+1 for the double null termination...
		{
			//have to get more mem.
			GrabMem(iSrcDest, (m_lCurDestCount + iLength + 1));
		}
		//now theres enough memory!  yay.
		//now copy the filename in
		strcpy(m_pDest, (LPCTSTR)sFile);

		//go to end of this files null term.
		m_pDest += iLength;
		m_lCurDestCount += iLength;
		//always keep it double null terminated, but don't
		// increment past it incase we want to add more files
		m_pDest[0] = 0;
	}
}

//
// SetMaxCount( const int iSrcDest, long lMax )
//
// Function to pre-allocate string memory to prevent 
//  lots of re-allocations
//
void CShellFileOp::SetMaxCount(const int iSrcDest, const long lMax)
{
	if (iSrcDest == SH_SRC_FILE)
	{
		m_lMaxSrcCount = lMax + 1;	//+1 for double null term.
		GrabMem(iSrcDest, m_lMaxSrcCount);
	} else {
		m_lMaxDestCount = lMax + 1;	//+1 for double null term.
		GrabMem(iSrcDest, m_lMaxDestCount);
	}
}

//
// SetTitle ( CString sTitle )
//
// Function to set the dialog title from a string
//
void CShellFileOp::SetTitle(CString sTitle)
{
	int iLength;
	char * pBuf;

	//free mem of current title
	if (m_pTitle)
	{
		free ((void *)m_pTitle);
		m_pTitle = NULL;
		m_FileOp.lpszProgressTitle = NULL;
	}

	iLength = sTitle.GetLength()+1;

	if (iLength > 1)
	{
		//grab more mem
		m_pTitle = (char *)malloc(iLength);
		//copy the title
		pBuf = sTitle.GetBuffer(iLength);
		strcpy(m_pTitle, pBuf);
		//now point the struct to the title
		m_FileOp.lpszProgressTitle = m_pTitle;
	}
}

//
// SetTitle ( const int nTitle )
//
// Function to set the dialog title from a resource identifier
//
void CShellFileOp::SetTitle( const int nTitle )
{
	CString sTitle;
	sTitle.LoadString(nTitle);
	SetTitle(sTitle);
}

//
// SetParent ( const HWND phWnd )
//
// Function to set the parent dialog
//
void CShellFileOp::SetParent(const HWND phWnd)
{
	m_FileOp.hwnd = phWnd;
}

//
// BOOL AnyOperationsAborted ( ) const
//
// Function to get Abort flag
//
BOOL CShellFileOp::AnyOperationsAborted() const
{
	return m_FileOp.fAnyOperationsAborted;
}

//
// FILEOP_FLAGS GetFlags() const
//
// Function to return the operation flags
//
FILEOP_FLAGS CShellFileOp::GetFlags() const
{
	return m_FileOp.fFlags;
}

//
// SetFlags ( const FILEOP_FLAGS fNewFlags )
//
// Function to set the operation flags
//
void CShellFileOp::SetFlags(const FILEOP_FLAGS fNewFlags)
{
	m_FileOp.fFlags = fNewFlags;
}

//
// GrabMem( const int iSrcDest, const long lNum )
//
// function to grab some string space memory
//
void CShellFileOp::GrabMem(const int iSrcDest, const long lNum)
{
	char * pMem;
	long lOffset;

	//get current ptr
	if (iSrcDest == SH_SRC_FILE) {
		pMem = m_pFrom;
	} else {
		pMem = m_pTo;
	}

	if (pMem)	//some mem is already allocated!
	{
		//have to make sure our offset ptrs dont get screwed up...
		if (iSrcDest == SH_SRC_FILE) {
			lOffset = (m_pSrc - pMem);
		} else {
			lOffset = (m_pDest - pMem);
		}

		//get more!
		pMem = (char *)realloc((void *)pMem, lNum);

		//reassign offset ptr, and max counts
		if (iSrcDest == SH_SRC_FILE) {
			m_pSrc = pMem + lOffset;
			m_lMaxSrcCount = lNum;
		} else {
			m_pDest = pMem + lOffset;
			m_lMaxDestCount = lNum;
		}
	}
	else
	{
		//get first block
		pMem = (char *)malloc(lNum);

		//assign offset ptr to start of block, and max counts
		if (iSrcDest == SH_SRC_FILE) {
			m_pSrc = pMem;
			m_lMaxSrcCount = lNum;
		} else {
			m_pDest = pMem;
			m_lMaxDestCount = lNum;
		}
		pMem[0] = 0;	//ensure null terminated
	}
	//assign ptrs in sh structure.
	if (iSrcDest == SH_SRC_FILE) {
		m_pFrom = pMem;
		m_FileOp.pFrom = m_pFrom;

	} else {
		m_pTo = pMem;
		m_FileOp.pTo = m_pTo;
	}
	
}