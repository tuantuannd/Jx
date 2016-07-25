// RoleDBServerDoc.cpp : implementation of the CRoleDBServerDoc class
//

#include "stdafx.h"
#include "RoleDBServer.h"

#include "RoleDBServerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerDoc

IMPLEMENT_DYNCREATE(CRoleDBServerDoc, CDocument)

BEGIN_MESSAGE_MAP(CRoleDBServerDoc, CDocument)
	//{{AFX_MSG_MAP(CRoleDBServerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerDoc construction/destruction

CRoleDBServerDoc::CRoleDBServerDoc()
{
	// TODO: add one-time construction code here

}

CRoleDBServerDoc::~CRoleDBServerDoc()
{
}

BOOL CRoleDBServerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerDoc serialization

void CRoleDBServerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerDoc diagnostics

#ifdef _DEBUG
void CRoleDBServerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CRoleDBServerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerDoc commands
