// SavePack.h: interface for the CSavePack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SAVEPACK_H__2776CBC0_DFB3_4814_A570_D2FC339E0D05__INCLUDED_)
#define AFX_SAVEPACK_H__2776CBC0_DFB3_4814_A570_D2FC339E0D05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZPackFile.h"

#define MAX_FILE					2004800			//最多20万个文件

class CSavePack  
{
private:
	char FileName[256];							//文件名
	index_info* index_list;						//文件索引
	z_pack_header header;						//包文件头部
	FILE* output;
	unsigned long offset;
public:
	CSavePack();
	virtual ~CSavePack(){};
	bool open(char* FileName, index_info* index);	//打开包文件
	bool Close();										//保存包文件
	bool AddData(char* data, index_info* index, char *temp_buffer);		//添加文件
};

#endif // !defined(AFX_SAVEPACK_H__2776CBC0_DFB3_4814_A570_D2FC339E0D05__INCLUDED_)
