//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include<FSTREAM>


#include "FileUpdateAttribute.h"
#include "unitfrmAttrChange.h"
#include "unitfrmMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#define ServerUpdateFolder    "."

TfrmMain *frmMain;
//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnCloseClick(TObject *Sender)
{
  Close();        
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnOpenSaveFileClick(TObject *Sender)
{
  string FileListTemp[100];
  if(!dlgOpen->Execute()) return;
  int FileListTempCount=0;
  int i,j;
  for(i=0;i<dlgOpen->Files->Count;++i)
  {
    FileListTemp[FileListTempCount] = dlgOpen->Files->Strings[i].c_str();
    ++FileListTempCount;
  }
  
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
			FileList[FileListCount].FileName = ExtractFileName(FileListTemp[i].c_str()).c_str();
			FileList[FileListCount].Belong = "-1";
			FileList[FileListCount].CheckVersionMethod = "5";
			FileList[FileListCount].UpdateFileFlag = "1";
			FileList[FileListCount].LocalPath = "%PROGRAMDIR%";
			FileList[FileListCount].UpdateFileMethod = "copy";
			FileList[FileListCount].RemotePath = "";
			FileList[FileListCount].UpdateRelative = "0";

      //个别文件判断
      string aFileName = FileList[FileListCount].FileName;
      char aFileChar[20] = {0};
      memcpy(aFileChar,aFileName.c_str(),6);
      char *ptr = aFileChar;
	    while(*ptr)
      {
		    if(*ptr >= 'A' && *ptr <= 'Z') *ptr += 'a' - 'A';
		    ptr++;
      }
      if((aFileName == "AutoUpdateRes.dll") ||
          (aFileName == "Engine.dll") ||
          (aFileName == "JXOnline.exe") ||
          (aFileName == "Represent2.dll") ||
          (aFileName == "Represent3.dll") ||
          (aFileName == "UpdateDLL.dll"))
      {
        FileList[FileListCount].UpdateRelative = "1";
      }
      else if(aFileName == "ChatSent.flt")
      {
        FileList[FileListCount].LocalPath = "%PROGRAMDIR\\Settings\\";
      }
      else if(strcmp(aFileChar,"update") == 0)
      {
        FileList[FileListCount].LocalPath = "%PROGRAMDIR\\data\\";
        FileList[FileListCount].UpdateFileMethod = "Package(Update.pak)";
        char* aBelongChar;
        char aBelongStr[5] = {0};
        aBelongChar = aFileName.begin();
        aBelongChar += 6;
        int strIndex = 0;
        while((*aBelongChar != '-')&&(*aBelongChar != '.'))
        {
          memcpy(&aBelongStr[strIndex], aBelongChar, 1);
          aBelongChar++;
          strIndex++;
        }
        FileList[FileListCount].Belong = aBelongStr;
      }
			++FileListCount;
		}
	}

	lvSaveFile->Items->Clear();
	for(i=0;i<FileListCount;++i)
	{
    TListItem* aListItem = lvSaveFile->Items->Add();
    aListItem->Caption = FileList[i].FileName.c_str();
    aListItem->SubItems->Add(FileList[i].FilePath.c_str());
    aListItem->SubItems->Add(FileList[i].Belong.c_str());
    aListItem->SubItems->Add(FileList[i].CheckVersionMethod.c_str());
    aListItem->SubItems->Add(FileList[i].UpdateFileFlag.c_str());
    aListItem->SubItems->Add(FileList[i].LocalPath.c_str());
    aListItem->SubItems->Add(FileList[i].UpdateFileMethod.c_str());
    aListItem->SubItems->Add(FileList[i].RemotePath.c_str());
    aListItem->SubItems->Add(FileList[i].UpdateRelative.c_str());
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnSaveDirClick(TObject *Sender)
{
  AnsiString aDir;
  if(!SelectDirectory("浏览文件夹","",aDir)) exit;
  txtSaveDir->Text = aDir;
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::lvSaveFileDblClick(TObject *Sender)
{
  if(lvSaveFile->Selected == NULL) exit;
  int index = lvSaveFile->Selected->Index;

  TfrmAttrChange* frmAttrChange = new TfrmAttrChange(this);
  frmAttrChange->txtFileName->Text = FileList[index].FileName.c_str();
  frmAttrChange->txtFilePath->Text = FileList[index].FilePath.c_str();
  frmAttrChange->txtBelong->Text = FileList[index].Belong.c_str();
  frmAttrChange->txtCheckVersionMethod->Text = FileList[index].CheckVersionMethod.c_str();
  frmAttrChange->txtUpdateFileFlag->Text = FileList[index].UpdateFileFlag.c_str();
  frmAttrChange->txtLocalPath->Text = FileList[index].LocalPath.c_str();
  frmAttrChange->txtUpdateFileMethod->Text = FileList[index].UpdateFileMethod.c_str();
  frmAttrChange->txtRemotePath->Text = FileList[index].RemotePath.c_str();
  frmAttrChange->txtUpdateRelative->Text = FileList[index].UpdateRelative.c_str();

  frmAttrChange->ShowModal();
  if(frmAttrChange->ModalResult != 1) return;

  FileList[index].FileName = frmAttrChange->txtFileName->Text.c_str();
	FileList[index].FilePath = frmAttrChange->txtFilePath->Text.c_str();
	FileList[index].Belong = frmAttrChange->txtBelong->Text.c_str();
	FileList[index].CheckVersionMethod = frmAttrChange->txtCheckVersionMethod->Text.c_str();
	FileList[index].UpdateFileFlag = frmAttrChange->txtUpdateFileFlag->Text.c_str();
	FileList[index].LocalPath = frmAttrChange->txtLocalPath->Text.c_str();
	FileList[index].UpdateFileMethod = frmAttrChange->txtUpdateFileMethod->Text.c_str();
	FileList[index].RemotePath = frmAttrChange->txtRemotePath->Text.c_str();
	FileList[index].UpdateRelative = frmAttrChange->txtUpdateRelative->Text.c_str();

  TListItem* aListItem = lvSaveFile->Items->operator [](index);
  aListItem->Caption = FileList[index].FileName.c_str();
  aListItem->SubItems->Strings[0] = FileList[index].FilePath.c_str();
  aListItem->SubItems->Strings[1] = FileList[index].Belong.c_str();
  aListItem->SubItems->Strings[2] = FileList[index].CheckVersionMethod.c_str();
  aListItem->SubItems->Strings[3] = FileList[index].UpdateFileFlag.c_str();
  aListItem->SubItems->Strings[4] = FileList[index].LocalPath.c_str();
  aListItem->SubItems->Strings[5] = FileList[index].UpdateFileMethod.c_str();
  aListItem->SubItems->Strings[6] = FileList[index].RemotePath.c_str();
  aListItem->SubItems->Strings[7] = FileList[index].UpdateRelative.c_str();
  frmAttrChange->Free();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnRunClick(TObject *Sender)
{
  using namespace std;
  
  if(!DirectoryExists(txtSaveDir->Text))
      ForceDirectories(txtSaveDir->Text);

  int i;
  for(i=0;i<FileListCount;++i)
  {
    char aFilePath[260] = {0};
    strcpy(aFilePath,(char*)(txtSaveDir->Text.c_str()));
    strcat(aFilePath,"\\");
    strcat(aFilePath,(char*)(FileList[i].FileName.c_str()));
    if(!CopyFile(FileList[i].FilePath.c_str(),aFilePath,false))
    {
      string aMsg = "复制文件 ";
      aMsg += FileList[i].FilePath.c_str();
      aMsg += " 到 ";
      aMsg += aFilePath;
      aMsg += "时出错！";
      MessageBox(this->Handle, aMsg.c_str() ,"错误",MB_OK | MB_ICONEXCLAMATION );;
    }
  }

  char aAppPath[MAX_PATH+100];
	GetModuleFileName(NULL,aAppPath,MAX_PATH+100);//获取应用程序的全路径
	aAppPath[strlen(aAppPath)-15]='\0';		//去掉应用程序的全名（15为应用程序文件全名的长度）
	string strAppPath = aAppPath;
	string strDataPath = txtSaveDir->Text.c_str();
	string IndexPath = strAppPath + "\\FileComment.ini";

  //写version.cfg
	string FileVersionPath = txtSaveDir->Text.c_str();
	FileVersionPath += "\\version.cfg";
	fstream FileVersion( FileVersionPath.c_str() ,ios::out);
	FileVersion<<"[Version]"<<endl;
	FileVersion<<"Version = "<<txtSubVer->Text.c_str()<<endl;
	FileVersion<<"MajorVersion = "<<txtMainVer->Text.c_str()<<endl;
	FileVersion.close();

  //写commonindex.ini
	FileVersionPath = strAppPath;
	FileVersionPath += "\\commonindex.ini";
	fstream FileCommonIndex( FileVersionPath.c_str() ,ios::out);
	FileCommonIndex<<"[Version]"<<endl;
	FileCommonIndex<<"Version = "<<txtSubVer->Text.c_str()<<endl;
	FileCommonIndex<<"MajorVersion = "<<txtMainVer->Text.c_str()<<endl;
	FileCommonIndex<<"WebDown = "<<txtWebDown->Text.c_str()<<endl<<endl;
	FileCommonIndex.close();

  	fstream FileCommondFile(IndexPath.c_str(),ios::out);
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
	//把version.cfg写入index.dat文件
	string aStr;
	aStr = "Version.cfg";
	FileUpdateAttribute IndexFileData(aStr);
	aStr = "-1";
	IndexFileData.SetBelong(aStr);
	aStr = "5";
	IndexFileData.SetCheckVersionMethod(aStr);
	aStr = "1";
	IndexFileData.SetUpdateFileFlag(aStr);
	aStr = "%PROGRAMDIR%";
	IndexFileData.SetLocalPath(aStr);
	aStr = "Last;Copy";
	IndexFileData.SetUpdateFileMethod(aStr);
	aStr = "";
	IndexFileData.SetRemotePath(aStr);
	aStr = "0";
	IndexFileData.SetUpdateRelative(aStr);
	IndexFileData.DumpTostream(FileCommondFile);
	FileCommondFile.close();

  RunAutoBat(strDataPath,strAppPath,strAppPath,strDataPath);

	string aMsg;
	aMsg = "完成！\n请在";
  aMsg += txtWebDown->Text.c_str();
  aMsg += "目录查看相应文件和index.dat文件。";
	MessageBox(this->Handle, aMsg.c_str(),"完成",MB_OK | MB_ICONASTERISK );
}
//---------------------------------------------------------------------------

void TfrmMain::RunAutoBat(const string &DataPath, const string &ToolsPath, const string &InfoPath, const string &IndexPath)
{
	string  FilesPath = DataPath + " ";
	string  OutPath   = InfoPath + "\\index.txt ";
	string  CommonIndexPath = InfoPath + "\\CommonIndex.ini ";
	string  FilesCommentPath = InfoPath + "\\" + "FileComment.ini" + " " ;
	string  infoPath         = InfoPath + "\\info.txt ";
	string  IndexGenExeFile  = ToolsPath + "\\IndexGen ";
	string  Command = IndexGenExeFile +   FilesPath + OutPath + " 1  "+ ServerUpdateFolder + " " +  CommonIndexPath +  FilesCommentPath + infoPath;
	RunProcess((char *)Command.c_str());
	
	string  CompressDllExeFile = ToolsPath + "\\CompressDll ";
		
	Command  = CompressDllExeFile + "-l " + OutPath + IndexPath+"\\index.dat";	
	RunProcess((char *)Command.c_str());
}

//---------------------------------------------------------------------------
void TfrmMain::RunProcess(char *CommandLine)
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
    
		throw exception();
    }
	WaitForSingleObject( pi.hProcess, INFINITE );


}
void __fastcall TfrmMain::txtSaveDirChange(TObject *Sender)
{
  btnRun->Enabled = ((!txtSaveDir->Text.IsEmpty()) || (lvSaveFile->Items->Count != 0));  
}
//---------------------------------------------------------------------------
