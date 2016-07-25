// ED1EditorView.h : interface of the CED1EditorView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ED1EDITORVIEW_H__24D08F8D_EE70_42A7_A726_FA8951EC4E6D__INCLUDED_)
#define AFX_ED1EDITORVIEW_H__24D08F8D_EE70_42A7_A726_FA8951EC4E6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CED1EditorView : public CEditView
{
protected: // create from serialization only
	CED1EditorView();
	DECLARE_DYNCREATE(CED1EditorView)

// Attributes
public:
	CED1EditorDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CED1EditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CED1EditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CED1EditorView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ED1EditorView.cpp
inline CED1EditorDoc* CED1EditorView::GetDocument()
   { return (CED1EditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ED1EDITORVIEW_H__24D08F8D_EE70_42A7_A726_FA8951EC4E6D__INCLUDED_)
