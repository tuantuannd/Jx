// RoleDBServerView.cpp : implementation of the CRoleDBServerView class
//

#include "stdafx.h"
#include "RoleDBServer.h"

#include "RoleDBServerDoc.h"
#include "RoleDBServerView.h"
#include "KDBProcessThread.h"
#include "KRoleInfomation.h"
#include "KRoleDBserver.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerView

IMPLEMENT_DYNCREATE(CRoleDBServerView, CFormView)

BEGIN_MESSAGE_MAP(CRoleDBServerView, CFormView)
	//{{AFX_MSG_MAP(CRoleDBServerView)
	ON_BN_CLICKED(IDC_STARTTHREADS, OnStartthreads)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerView construction/destruction

CRoleDBServerView::CRoleDBServerView()
	: CFormView(CRoleDBServerView::IDD)
{
	//{{AFX_DATA_INIT(CRoleDBServerView)
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

CRoleDBServerView::~CRoleDBServerView()
{
}

void CRoleDBServerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoleDBServerView)
	DDX_Control(pDX, IDC_LIST3, m_DBServiceList);
	DDX_Control(pDX, IDC_LIST2, m_MainLoopList);
	DDX_Control(pDX, IDC_LIST1, m_RoleServerList);
	//}}AFX_DATA_MAP
}

BOOL CRoleDBServerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CRoleDBServerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

}

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerView printing

BOOL CRoleDBServerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CRoleDBServerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CRoleDBServerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CRoleDBServerView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerView diagnostics

#ifdef _DEBUG
void CRoleDBServerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CRoleDBServerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CRoleDBServerDoc* CRoleDBServerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRoleDBServerDoc)));
	return (CRoleDBServerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRoleDBServerView message handlers
extern TThreadData g_ThreadData[4];


void CRoleDBServerView::OnStartthreads() 
{
	
	
	// TODO: Add your control notification handler code here
	if (InitServer() == 0)	return;
	
	CWnd * pButton = this->GetDlgItem(IDC_STARTTHREADS);
	pButton->EnableWindow(FALSE);

    g_ThreadData[0].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        RoleServerMainThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[0].dwThreadId);                // returns the thread identifier 
	
	g_ThreadData[1].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        DatabaseLoadThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[1].dwThreadId);                // returns the thread identifier 
	
	g_ThreadData[2].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        DatabaseSaveThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[2].dwThreadId);                // returns the thread identifier 
	
	g_ThreadData[3].hThread = CreateThread( 
        NULL,                        // no security attributes 
        0,                           // use default stack size  
        RoleNetWorkServiceThreadFun,     // thread function 
		NULL,                // argument to thread function 
        0,                           // use default creation flags 
        &g_ThreadData[3].dwThreadId);                // returns the thread identifier 
	
}

void	CRoleDBServerView::UpdateMainLoopList()
{
	list<TCmd*>::iterator I =  g_MainThreadCmdList.begin();
	m_MainLoopList.ResetContent();
	while(I != g_MainThreadCmdList.end())
	{
		CString strContent;
		strContent.Format("Server%d %d DataLen%d", (*I)->ulNetId, (*I)->nProtoId, (*I)->nDataLen);
		m_MainLoopList.AddString(strContent);
		I ++;
	}
}

void	CRoleDBServerView::UpdateRoleServerList()
{
	RoleDBServer::KClientUserSet::iterator I = g_NetServer.m_ClientUserSet.begin();
	
	m_RoleServerList.ResetContent();
	while(I != g_NetServer.m_ClientUserSet.end())
	{
		CString strContent;
		strContent.Format("Connecting Server%d", I->first);
		m_RoleServerList.AddString(strContent);
		I ++;
	}

}

void	CRoleDBServerView::UpdateDBServiceList()
{
	list<TGetRoleInfoFromDB*>::iterator I =  g_DBLoadThreadCmdList.begin();
	m_DBServiceList.ResetContent();
	while(I != g_DBLoadThreadCmdList.end())
	{
		CString strContent;
		strContent.Format("Server%d LoadRole%s", (*I)->nNetId, (*I)->szRoleName);
		m_DBServiceList.AddString(strContent);
		I ++;
	}
}

extern TRoleNetMsg * pTestCmd;
/*
void CRoleDBServerView::OnTry() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	TRoleNetMsg * pNewCmd = (TRoleNetMsg * )(new char[sizeof(TRoleNetMsg) - 1 + strlen(m_RoleName)]);
	pNewCmd->nDataLen = strlen(m_RoleName);
	pNewCmd->nProtoId = PROTOL_LOADROLE;
	pNewCmd->nOffset  = strlen(m_RoleName);
	pNewCmd->bDataState = 1;
	
	strcpy(pNewCmd->pDataBuffer, m_RoleName);
	pTestCmd = pNewCmd;
}
*/
/*
void CRoleDBServerView::OnChangeRolename() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CRoleDBServerView::OnRecv() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	FILE * pFile;
   if( (pFile = fopen( m_RecvFile, "rb" )) != NULL )
   {
	   fseek(pFile, 0, SEEK_END);
	   long nFileSize = ftell(pFile);
	   pTestCmd = (TRoleNetMsg*)new char[nFileSize];
	   fseek(pFile, 0, SEEK_SET);
	   fread(pTestCmd, 1, nFileSize, pFile);	   
	   fclose(pFile);
   }
	
}
*/
