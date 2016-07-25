// UpdateMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileUpdateAttribute.h"
#include "ShellFileOp.h"

#include "UpdateMaker.h"
#include "UpdateMakerDlg.h"
#include "FileAttrChange.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_FILE_LIST_COUNT 100
#define ServerUpdateFolder    "."
FileAttribute FileList[MAX_FILE_LIST_COUNT];	//文件列表
int FileListCount=0;

/////////////////////////////////////////////////////////////////////////////
// CUpdateMakerDlg dialog

CUpdateMakerDlg::CUpdateMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateMakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateMakerDlg)
	m_SaveFileDir = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUpdateMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateMakerDlg)
	DDX_Control(pDX, IDC_LISTUPFILE, m_UpFile);
	DDX_Text(pDX, IDC_TXTSAVEDIR, m_SaveFileDir);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUpdateMakerDlg, CDialog)
	//{{AFX_MSG_MAP(CUpdateMakerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNUPFILE, OnBtnUpFile)
	ON_BN_CLICKED(IDC_BTNSAVEFILE, OnBtnSaveFile)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTUPFILE, OnListUpFileDblclk)
	ON_BN_CLICKED(IDOK, OnRun)
	ON_BN_CLICKED(IDRUN, OnRun)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateMakerDlg message handlers

BOOL CUpdateMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_UpFile.InsertColumn(0,"文件名",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(1,"路径",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(3,"Belong",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(4,"CheckVersionMethod",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(5,"UpdateFileFlag",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(6,"LocalPath",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(7,"UpdateFileMethod",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(8,"RemotePath",LVCFMT_LEFT,80);
	m_UpFile.InsertColumn(9,"UpdateRelative",LVCFMT_LEFT,80);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CUpdateMakerDlg::OnPaint() 
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
HCURSOR CUpdateMakerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CUpdateMakerDlg::OnBtnUpFile() 
{
	CFileDialog DlgOpenFile(true,NULL,NULL,OFN_HIDEREADONLY,"所有文件(*.*)|*.*",
		this->GetOwner());
	DlgOpenFile.m_ofn.Flags |= OFN_ALLOWMULTISELECT;
	if(DlgOpenFile.DoModal() != IDOK)return;
	CString PathName;
	POSITION aPosition = DlgOpenFile.GetStartPosition();
	
	string FileListTemp[100];
	int FileListTempCount=0;
	
	while(aPosition != NULL)	//把路径存到临时列表中
	{ 
		FileListTemp[FileListTempCount] = DlgOpenFile.GetNextPathName(aPosition);
		++FileListTempCount;
		//MessageBox(PathName);
	}
	int i,j;
	for(i=0;i<FileListTempCount;++i)//对比已有列表，如果不重复的就复制到正是列表中
	{
		bool isSame = false;
		for(j=0;j<FileListCount;++j)
		{
			if(FileList[j].FilePath == FileListTemp[i])
			{
				isSame = true;
				break;
			}
		}
		if(!isSame)
		{
			FileList[FileListCount].FilePath = FileListTemp[i];
			CFile aFile(FileListTemp[i].data(),CFile::modeRead);
			FileList[FileListCount].FileName = aFile.GetFileName().GetBuffer(100);
			aFile.Close();
			FileList[FileListCount].Belong = "-1";
			FileList[FileListCount].CheckVersionMethod = "5";
			FileList[FileListCount].UpdateFileFlag = "1";
			FileList[FileListCount].LocalPath = "%PROGRAMDIR%";
			FileList[FileListCount].UpdateFileMethod = "";
			FileList[FileListCount].RemotePath = "";
			FileList[FileListCount].UpdateRelative = "";

			++FileListCount;
		}
	}
	
	m_UpFile.DeleteAllItems();
	for(i=0;i<FileListCount;++i)
	{
		m_UpFile.InsertItem(i,FileList[i].FileName.data());
		m_UpFile.SetItemText(i,1,FileList[i].FilePath.data());
		m_UpFile.SetItemText(i,2,FileList[i].Belong.data());
		m_UpFile.SetItemText(i,3,FileList[i].CheckVersionMethod.data());
		m_UpFile.SetItemText(i,4,FileList[i].UpdateFileFlag.data());
		m_UpFile.SetItemText(i,5,FileList[i].LocalPath.data());
		m_UpFile.SetItemText(i,6,FileList[i].UpdateFileMethod.data());
		m_UpFile.SetItemText(i,7,FileList[i].RemotePath.data());
		m_UpFile.SetItemText(i,8,FileList[i].UpdateRelative.data());
	}
}

void CUpdateMakerDlg::OnBtnSaveFile() 
{
	LPITEMIDLIST pidlRoot=NULL; 
	SHGetSpecialFolderLocation(m_hWnd,CSIDL_DESKTOP,&pidlRoot); 

	BROWSEINFO bi;   //必须传入的参数,下面就是这个结构的参数的初始化 
	//CString strDisplayName;   //用来得到,你选择的活页夹路径,相当于提供一个缓冲区 
	bi.hwndOwner=GetSafeHwnd();   //得到父窗口Handle值 
	bi.pidlRoot=pidlRoot;   //这个变量就是我们在上面得到的. 
	bi.pszDisplayName=NULL;//strDisplayName.GetBuffer(MAX_PATH+1);   //得到缓冲区指针, 
	bi.lpszTitle="浏览文件夹";   //设置标题 
	bi.ulFlags=0;   //设置标志 
	bi.lpfn=NULL; 
	bi.lParam=0; 
	bi.iImage=0;   //上面这个是一些无关的参数的设置,最好设置起来, 
	LPITEMIDLIST pIIL =SHBrowseForFolder(&bi);   //打开对话框 
	
	if(pIIL == NULL)return;
	//strDisplayName.ReleaseBuffer();   //和上面的GetBuffer()相对应 

	TCHAR szInitialDir[MAX_PATH];
	BOOL bRet = ::SHGetPathFromIDList(pIIL, (char*)&szInitialDir);

	m_SaveFileDir = szInitialDir;
	UpdateData(false);

}

void CUpdateMakerDlg::OnListUpFileDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	if(phdn->iItem == -1) return;
	int index = phdn->iItem;
	CFileAttrChange frmAttrChange;
	frmAttrChange.m_strFileName = FileList[index].FileName.data();
	frmAttrChange.m_strFilePath = FileList[index].FilePath.data();
	frmAttrChange.m_strBelong = FileList[index].Belong.data();
	frmAttrChange.m_strCheckVersionMethod = FileList[index].CheckVersionMethod.data();
	frmAttrChange.m_strUpdateFileFlag = FileList[index].UpdateFileFlag.data();
	frmAttrChange.m_strLocalPath = FileList[index].LocalPath.data();
	frmAttrChange.m_strUpdateFileMethod = FileList[index].UpdateFileMethod.data();
	frmAttrChange.m_strRemotePath = FileList[index].RemotePath.data();
	frmAttrChange.m_strUpdateRelative = FileList[index].UpdateRelative.data();
	
	if(frmAttrChange.DoModal() != IDOK) return;
	

	FileList[index].FileName = frmAttrChange.m_strFileName.GetBuffer(100);
	FileList[index].FilePath = frmAttrChange.m_strFilePath.GetBuffer(100);
	FileList[index].Belong = frmAttrChange.m_strBelong.GetBuffer(100);
	FileList[index].CheckVersionMethod = frmAttrChange.m_strCheckVersionMethod.GetBuffer(100);
	FileList[index].UpdateFileFlag = frmAttrChange.m_strUpdateFileFlag.GetBuffer(100);
	FileList[index].LocalPath = frmAttrChange.m_strLocalPath.GetBuffer(100);
	FileList[index].UpdateFileMethod = frmAttrChange.m_strUpdateFileMethod.GetBuffer(100);
	FileList[index].RemotePath = frmAttrChange.m_strRemotePath.GetBuffer(100);
	FileList[index].UpdateRelative = frmAttrChange.m_strUpdateRelative.GetBuffer(100);
	
	m_UpFile.SetItemText(index,1,FileList[index].FileName.data());
	m_UpFile.SetItemText(index,1,FileList[index].FilePath.data());
	m_UpFile.SetItemText(index,2,FileList[index].Belong.data());
	m_UpFile.SetItemText(index,3,FileList[index].CheckVersionMethod.data());
	m_UpFile.SetItemText(index,4,FileList[index].UpdateFileFlag.data());
	m_UpFile.SetItemText(index,5,FileList[index].LocalPath.data());
	m_UpFile.SetItemText(index,6,FileList[index].UpdateFileMethod.data());
	m_UpFile.SetItemText(index,7,FileList[index].RemotePath.data());
	m_UpFile.SetItemText(index,8,FileList[index].UpdateRelative.data());
	UpdateData(false);
	*pResult = 0;
}

void CUpdateMakerDlg::OnRun() 
{
	int i;
	using namespace std;
	UpdateData(true);
	
	CString sDestDir, sSrcDir;
	CString sCache;
	CShellFileOp FileOp;
	FileOp.SetParent(m_hWnd);
	FileOp.SetFlags(FOF_FILESONLY | FOF_NOCONFIRMMKDIR | FOF_MULTIDESTFILES);
	for(i=0;i<FileListCount;++i)
	{
		CString aFilePath(FileList[i].FilePath.data());
		FileOp.AddFile(SH_SRC_FILE,aFilePath);
	}
	FileOp.AddFile(SH_DEST_FILE,m_SaveFileDir + "\\");
	if(FileOp.CopyFiles())
	{
		MessageBox("拷贝文件失败，请重试！","错误",MB_OK | MB_ICONHAND);
		return;
	}

	char aAppPath[MAX_PATH+100];
	GetModuleFileName(NULL,aAppPath,MAX_PATH+100);//获取应用程序的全路径
	aAppPath[strlen(aAppPath)-15]='\0';		//去掉应用程序的全名（15为应用程序文件全名的长度）
	string strAppPath = aAppPath;
	string strDataPath = m_SaveFileDir.GetBuffer(100);
	string IndexPath = strAppPath + "\\FileComment.ini";
	
	fstream FileCommondFile(IndexPath.data(),ios::out);
		
	for(i=0;i<FileListCount;++i)
	{	
		//写入index.dat文件
		FileUpdateAttribute IndexFileData(FileList[i].FileName);
		IndexFileData.SetBelong(FileList[i].Belong);
		IndexFileData.SetCheckVersionMethod(FileList[i].CheckVersionMethod);
		IndexFileData.SetUpdateFileFlag(FileList[i].UpdateFileFlag);
		IndexFileData.SetLocalPath(FileList[i].LocalPath);
		IndexFileData.SetUpdateFileMethod(FileList[i].UpdateFileMethod);
		IndexFileData.SetRemotePath(FileList[i].RemotePath);
		IndexFileData.SetUpdateRelative(FileList[i].UpdateRelative);
		IndexFileData.DumpTostream(FileCommondFile);
	}
	FileCommondFile.close();

	
/*
	strcat(strCompressDLL,"Auto.bat ");
	strcat(strCompressDLL,m_SaveFileDir.GetBuffer(100));
	MessageBox(strCompressDLL);
	WinExec(strCompressDLL,SW_SHOW);
*/
	
	RunAutoBat(strDataPath,strAppPath,strAppPath,strDataPath);

	string aMsg;
	aMsg = "完成！\n请在" + m_SaveFileDir + "目录查看相应文件和index.dat文件。";
	MessageBox(aMsg.data(),"完成",MB_OK | MB_ICONASTERISK );
}

void CUpdateMakerDlg::RunAutoBat(const string &DataPath, const string &ToolsPath, const string &InfoPath, const string &IndexPath)
{
	string  FilesPath = DataPath + " ";
	string  OutPath   = InfoPath + "\\index.txt ";
	string  CommonIndexPath = InfoPath + "\\CommonIndex.ini ";
	string  FilesCommentPath = InfoPath + "\\" + "FileComment.ini" + " " ;
	string  infoPath         = InfoPath + "\\info.txt ";
	string  IndexGenExeFile  = ToolsPath + "\\IndexGen ";
	string  Command = IndexGenExeFile +   FilesPath + OutPath + " 1  "+ ServerUpdateFolder + " " +  CommonIndexPath +  FilesCommentPath + infoPath;
	MessageBox((char *)Command.c_str());
	RunProcess((char *)Command.c_str());
	
	string  CompressDllExeFile = ToolsPath + "\\CompressDll ";
		
	Command  = CompressDllExeFile + "-l " + OutPath + IndexPath+"\\index.dat";
	MessageBox((char *)Command.c_str());
	
	RunProcess((char *)Command.c_str());
}

void CUpdateMakerDlg::RunProcess(char *CommandLine)
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
	
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	
    // Start the child process. 
    if( !CreateProcess( NULL, // No module name (use command line). 
        CommandLine, // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
    {
		string ErrorInfo = string("CreateProcess(" )+ string(CommandLine);
        
		throw exception("ERROR when CreateProcess ");
    }
	WaitForSingleObject( pi.hProcess, INFINITE );


}