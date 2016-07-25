// ED1EditorDoc.h : interface of the CED1EditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ED1EDITORDOC_H__4E550F95_580F_4FB4_B37D_4467EF8B5E1A__INCLUDED_)
#define AFX_ED1EDITORDOC_H__4E550F95_580F_4FB4_B37D_4467EF8B5E1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CED1EditorDoc : public CDocument
{
protected: // create from serialization only
	CED1EditorDoc();
	DECLARE_DYNCREATE(CED1EditorDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CED1EditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CED1EditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CED1EditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ED1EDITORDOC_H__4E550F95_580F_4FB4_B37D_4467EF8B5E1A__INCLUDED_)
