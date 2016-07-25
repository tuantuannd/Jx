// ShellFileOp.h: interface for the CShellFileOp class.
//	
//	Tim Johnson		timj@progsoc.uts.edu.au
//	Written : July 1998
//
//	Copyright 1998
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHELLFILEOP_H__DA3A3661_1EF3_11D2_9E56_444553540000__INCLUDED_)
#define AFX_SHELLFILEOP_H__DA3A3661_1EF3_11D2_9E56_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


//constats for use in iSrcDest arguements
const int SH_SRC_FILE = 0;
const int SH_DEST_FILE = 1;

class CShellFileOp  
{
public:

	//
	//simple constructor
	//
	CShellFileOp();
	
	//
	//complex constructor - performs the operation straight away
	//
	CShellFileOp( const HWND phWnd, 
				  const UINT wFunc,
				  const CString sFrom,
				  const CString sTo,
				  const FILEOP_FLAGS fFlags,
				  const CString sProgressTitle);

	//
	//destructor
	//
	virtual ~CShellFileOp();

public: //operations

	//
	// Function to perform copy operation
	//
	long CopyFiles();
	
	//
	// Function to perform delete operation
	//
	long DeleteFiles();
	
	//
	// Function to perform move operation
	//
	long MoveFiles();
	
	//
	// Function to perform rename operation
	//
	long RenameFiles();
	
	//
	// Adds another filename to the end of the current string
	//
	void AddFile(const int iSrcDest, const CString sFile);
	
	//
	// Clears a list of files
	//
	void ClearFiles(const int iSrcDest);
	
	//
	// Function to pre-allocate string memory to prevent 
	//  lots of re-allocations
	//
	void SetMaxCount(const int iSrcDest, const long lMax);
	
	//
	// Function to set the dialog title
	//  sTitle is a string to be used
	//  nTitle is a resource ID to get the string from
	void SetTitle(CString sTitle);
	void SetTitle(const int nTitle);
	
	//
	// Function to set the parent HWND of the dialog
	//
	void SetParent(const HWND phWnd);
	
	//
	// Function to get Abort flag
	//
	BOOL AnyOperationsAborted() const;
	
	//
	// Functions to get/set the operation flags
	//
	//	Flags that control the file operation. This member can be a combination of the 
	//	following values: 
	//
	//	FOF_ALLOWUNDO			Preserves undo information, if possible. 
	//
	//	FOF_CONFIRMMOUSE		Not implemented. 
	//
	//	FOF_FILESONLY			Performs the operation only on files if a wildcard 
	//							filename (*.*) is specified. 
	//
	//	FOF_MULTIDESTFILES		Indicates that the pTo member specifies multiple destination 
	//							files (one for each source file) rather than one directory 
	//							where all source files are to be deposited. 
	//
	//	FOF_NOCONFIRMATION		Responds with "yes to all" for any dialog box that is 
	//							displayed. 
	//
	//	FOF_NOCONFIRMMKDIR		Does not confirm the creation of a new directory if the 
	//							operation requires one to be created. 
	//
	//	FOF_RENAMEONCOLLISION	Gives the file being operated on a new name (such as 
	//							"Copy #1 of...") in a move, copy, or rename operation if a 
	//							file of the target name already exists. 
	//
	//	FOF_SILENT				Does not display a progress dialog box. 
	//
	//	FOF_SIMPLEPROGRESS		Displays a progress dialog box, but does not show the 
	//							filenames. 
	FILEOP_FLAGS GetFlags() const;
	void SetFlags(const FILEOP_FLAGS fNewFlags);

private:	//operations
	
	// function to grab some string space memory
	void GrabMem(const int iSrcDest, const long lNum);

private:	//attributes
	
	//max no. char in source string
	long m_lMaxSrcCount;
	
	//max no. char in dest string
	long m_lMaxDestCount;
	
	//current no. char in source string
	long m_lCurSrcCount;
	
	//current no. char in dest string
	long m_lCurDestCount;
	
	//structure for shell call
	SHFILEOPSTRUCT m_FileOp;
	
	//pointer to start of source string
	char * m_pTo;
	
	//pointer to start of dest string
	char * m_pFrom;
	
	//current pointer in source string
	char * m_pSrc;
	
	//current pointer in dest string
	char * m_pDest;	
	
	//title to be used on dialog
	char * m_pTitle;
};

#endif // !defined(AFX_SHELLFILEOP_H__DA3A3661_1EF3_11D2_9E56_444553540000__INCLUDED_)
