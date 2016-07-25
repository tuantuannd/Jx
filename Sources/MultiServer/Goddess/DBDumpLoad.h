// DBDumpLoad.h: interface for the CDBDump class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBDUMPLOAD_H__956A6AC3_33CD_44D2_B4B2_771DE290B32F__INCLUDED_)
#define AFX_DBDUMPLOAD_H__956A6AC3_33CD_44D2_B4B2_771DE290B32F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDBDump  
{
private:
	char m_FilePath[MAX_PATH];							//文件名
	FILE* m_output;
	bool m_IsOpened;
	//unsigned long offset;
public:
	CDBDump();
	virtual ~CDBDump(){};
	bool Open(char* aFilePath);	//打开包文件
	bool Close();								//保存包文件
	bool AddData(char* key, const size_t keysize, char *aData, const size_t size);		//添加数据

};

class CDBLoad
{
private:
	char m_FilePath[MAX_PATH];							//文件名
	FILE* m_output;
	bool m_IsOpened;
	//unsigned long offset;
public:
	CDBLoad();
	virtual ~CDBLoad(){};
	bool Open(char* aFilePath);	//打开包文件
	bool Close();								//关闭包文件
	bool ReadData(char* key, size_t& keysize, char *aData, size_t& size);		//读取数据
	void GotoHead();					//把文件指针移动到文件头
	bool SearchData(char* key, char *aData, size_t& size);		//搜索数据
};
#endif // !defined(AFX_DBDUMPLOAD_H__956A6AC3_33CD_44D2_B4B2_771DE290B32F__INCLUDED_)
