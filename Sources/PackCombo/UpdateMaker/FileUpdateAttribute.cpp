// FileUpdateAttribute.cpp: implementation of the FileUpdateAttribute class.
//
//////////////////////////////////////////////////////////////////////

#include "FileUpdateAttribute.h"
#include   "stdafx.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileUpdateAttribute::FileUpdateAttribute(string& File)
:Belong("-1")
,CheckVersionMethod("5")
,UpdateFileFlag("1")
,LocalPath("%PROGRAMDIR%")
,UpdateFileMethod("")
,RemotePath("")
,FileName(File)
,UpdateRelative("")
{
	


}

FileUpdateAttribute::~FileUpdateAttribute()
{

}

void FileUpdateAttribute::SetBelong(string &BelongValue)
{
	Belong = BelongValue;

}

void FileUpdateAttribute::SetCheckVersionMethod(string &CheckVersionMethodValue)
{
	CheckVersionMethod = CheckVersionMethodValue;


}

void FileUpdateAttribute::SetLocalPath(string &LocalPathValue)
{
	LocalPath = LocalPathValue;


}

void FileUpdateAttribute::SetRemotePath(string &RemotePathValue)
{
	RemotePath = RemotePathValue;


}

void FileUpdateAttribute::SetUpdateFileFlag(string &UpdateFileFlagValue)
{
	UpdateFileFlag = UpdateFileFlagValue;


}

void FileUpdateAttribute::SetUpdateFileMethod(string &UpdateFileMethodValue)
{
	UpdateFileMethod = UpdateFileMethodValue;
	

}

void  FileUpdateAttribute::DumpTostream(iostream& InfoStream)
{
	InfoStream << ("[" + FileName + "]").c_str()<<endl;
	
	if(Belong !="" )
	{
		InfoStream<< ( "Belong = " + Belong).c_str()<<endl;


	}
	if(CheckVersionMethod !="" )
	{
		 InfoStream <<("CheckVersionMethod = " + CheckVersionMethod).c_str() <<endl;


	}
	
	if(LocalPath !="" )
	{
		InfoStream  <<("LocalPath = " + LocalPath).c_str()<<endl;


	}
	

	if(RemotePath !="" )
	{
		InfoStream << ("RemotePath = " + RemotePath).c_str() <<endl;


	}
	
	if(UpdateFileFlag !="" )
	{
		InfoStream << ("UpdateFileFlag = " + UpdateFileFlag).c_str()<<endl;


	}
	
	if(UpdateFileMethod !="" )
	{
	   
		InfoStream << "UpdateFileMethod = " + UpdateFileMethod<<endl;


	}
	
	if(UpdateRelative !="" )
	{
	   
		InfoStream << "UpdateRelative = " + UpdateRelative<<endl;


	}

	InfoStream<< endl;



}

void FileUpdateAttribute::SetUpdateRelative(const string &Value)
{
	UpdateRelative = Value;
}