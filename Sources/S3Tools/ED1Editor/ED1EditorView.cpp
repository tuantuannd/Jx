// ED1EditorView.cpp : implementation of the CED1EditorView class
//

#include "stdafx.h"
#include "ED1Editor.h"

#include "ED1EditorDoc.h"
#include "ED1EditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CED1EditorView

IMPLEMENT_DYNCREATE(CED1EditorView, CEditView)

BEGIN_MESSAGE_MAP(CED1EditorView, CEditView)
	//{{AFX_MSG_MAP(CED1EditorView)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CED1EditorView construction/destruction

CED1EditorView::CED1EditorView()
{
	// TODO: add construction code here

}

CED1EditorView::~CED1EditorView()
{
}

BOOL CED1EditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	//cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CED1EditorView drawing

void CED1EditorView::OnDraw(CDC* pDC)
{
	CED1EditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CED1EditorView printing

BOOL CED1EditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CEditView preparation
	return CEditView::OnPreparePrinting(pInfo);
}

void CED1EditorView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView begin printing.
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CED1EditorView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CEditView end printing
	CEditView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CED1EditorView diagnostics

#ifdef _DEBUG
void CED1EditorView::AssertValid() const
{
	CEditView::AssertValid();
}

void CED1EditorView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CED1EditorDoc* CED1EditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CED1EditorDoc)));
	return (CED1EditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CED1EditorView message handlers

