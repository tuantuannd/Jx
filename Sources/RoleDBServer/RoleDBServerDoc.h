// RoleDBServerDoc.h : interface of the CRoleDBServerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROLEDBSERVERDOC_H__96A9C3CD_F8DD_4629_9031_8093110E8349__INCLUDED_)
#define AFX_ROLEDBSERVERDOC_H__96A9C3CD_F8DD_4629_9031_8093110E8349__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CRoleDBServerDoc : public CDocument
{
protected: // create from serialization only
	CRoleDBServerDoc();
	DECLARE_DYNCREATE(CRoleDBServerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoleDBServerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRoleDBServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CRoleDBServerDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLEDBSERVERDOC_H__96A9C3CD_F8DD_4629_9031_8093110E8349__INCLUDED_)
