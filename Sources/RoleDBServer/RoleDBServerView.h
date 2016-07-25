// RoleDBServerView.h : interface of the CRoleDBServerView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROLEDBSERVERVIEW_H__43765D79_6E36_4F70_8AA3_5B788EED1952__INCLUDED_)
#define AFX_ROLEDBSERVERVIEW_H__43765D79_6E36_4F70_8AA3_5B788EED1952__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRoleDBServerView : public CFormView
{
protected: // create from serialization only
	CRoleDBServerView();
	DECLARE_DYNCREATE(CRoleDBServerView)

public:
	//{{AFX_DATA(CRoleDBServerView)
	enum { IDD = IDD_ROLEDBSERVER_FORM };
	CListBox	m_DBServiceList;
	CListBox	m_MainLoopList;
	CListBox	m_RoleServerList;
	//}}AFX_DATA

// Attributes
public:
	CRoleDBServerDoc* GetDocument();
	void	UpdateMainLoopList();
	void	UpdateRoleServerList();
	void	UpdateDBServiceList();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoleDBServerView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRoleDBServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CRoleDBServerView)
	afx_msg void OnStartthreads();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in RoleDBServerView.cpp
inline CRoleDBServerDoc* CRoleDBServerView::GetDocument()
   { return (CRoleDBServerDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLEDBSERVERVIEW_H__43765D79_6E36_4F70_8AA3_5B788EED1952__INCLUDED_)
