// S3AccClientSampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "S3AccClientSample.h"
#include "S3AccClientSampleDlg.h"
#include "Sample.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CS3AccClientSampleDlg dialog

CS3AccClientSampleDlg::CS3AccClientSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CS3AccClientSampleDlg::IDD, pParent),
	m_pSample( NULL )
{
	//{{AFX_DATA_INIT(CS3AccClientSampleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CS3AccClientSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CS3AccClientSampleDlg)
	DDX_Control(pDX, IDC_PARAM, m_paramCtrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CS3AccClientSampleDlg, CDialog)
	//{{AFX_MSG_MAP(CS3AccClientSampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_EDITCHANGE(IDC_PARAM, OnEditchangeParam)
	ON_BN_CLICKED(IDC_SEND, OnSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CS3AccClientSampleDlg message handlers

BOOL CS3AccClientSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_paramCtrl.SetCurSel( 0 );

	// 初始化客户端套接字
	//------>BEGIN
	m_pSample = CSample::Instance();
	ASSERT(NULL!=m_pSample);
	ASSERT(m_pSample->Connect( "192.168.26.211", 8200 ));
	ASSERT(m_pSample->Start());
	//<------END
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CS3AccClientSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CS3AccClientSampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CS3AccClientSampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CS3AccClientSampleDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CS3AccClientSampleDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CS3AccClientSampleDlg::OnEditchangeParam() 
{
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
}

void CS3AccClientSampleDlg::OnSend() 
{
	// TODO: Add your control notification handler code here
	int iSel = m_paramCtrl.GetCurSel();
	if ( ( iSel < 0 ) || ( iSel > 3 ) )
	{
		return;
	}
	const DWORD params[] =
	{
		LOGIN_A_SERVERLIST,
		LOGIN_A_LOGIN,
		LOGIN_A_DECACCOUNTDEPOSIT,
		LOGIN_A_LOGOUT
	};
	ASSERT(NULL!=m_pSample);
	switch( params[iSel] )
	{
	case LOGIN_A_SERVERLIST:
		m_pSample->QueryGSList();
		break;
	case LOGIN_A_LOGIN:
		m_pSample->Login( "test0", "888888", 78221 );
		break;
	case LOGIN_A_DECACCOUNTDEPOSIT:
		m_pSample->DecAccountDeposit( 78221, 78221, 60 );
		break;
	case LOGIN_A_LOGOUT:
		m_pSample->Logout( 78221, 78221 );
		break;
	default:
		break;
	}
}
