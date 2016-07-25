// FileUpdateAttribute.h: interface for the FileUpdateAttribute class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEUPDATEATTRIBUTE_H__AAFAA2AA_ADC1_43C1_801C_5AF2E489BA73__INCLUDED_)
#define AFX_FILEUPDATEATTRIBUTE_H__AAFAA2AA_ADC1_43C1_801C_5AF2E489BA73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include<string>
#include<iostream>
using namespace std;

typedef struct
{
	string FileName;
	string UpdateFileMethod;
	string RemotePath;
	string LocalPath;
	string UpdateFileFlag;
	string CheckVersionMethod;
	string Belong;
	string UpdateRelative;

	string FilePath;
} FileAttribute;

class FileUpdateAttribute  
{
public:
	void SetUpdateRelative(const string& Value);
	void DumpTostream(iostream &InfoStream);
	void SetUpdateFileMethod(string& UpdateFileMethodValue);
	void SetUpdateFileFlag(string &UpdateFileFlagValue);
	void SetRemotePath(string& RemotePathValue);
	void SetLocalPath(string& LocalPathValue);
	void SetCheckVersionMethod(string& CheckVersionMethodValue);
	void SetBelong(string& BelongValue);
	
	FileUpdateAttribute(string& File);
	
	
	
	virtual ~FileUpdateAttribute();


private:
	string FileName;
	string UpdateFileMethod;
	string RemotePath;
	string LocalPath;
	string UpdateFileFlag;
	string CheckVersionMethod;
	string Belong;
	string UpdateRelative;
};

#endif // !defined(AFX_FILEUPDATEATTRIBUTE_H__AAFAA2AA_ADC1_43C1_801C_5AF2E489BA73__INCLUDED_)
