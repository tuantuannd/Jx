//-----------------------------------------//
//                                         //
//  File		: S3PDBConVBC.h			   //
//	Author		: Yang Xiaodong            //
//	Modified	: 3/25/2003                //
//                                         //
//-----------------------------------------//
#if !defined(AFX_S3PDBCONVBC_H__7CEDC5DC_8F90_45DA_B24D_27A082789125__INCLUDED_)
#define AFX_S3PDBCONVBC_H__7CEDC5DC_8F90_45DA_B24D_27A082789125__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include "KThread.h"

typedef struct tag_DATABASEINFO
{
	std::string strServer;
	std::string strDataBase;
	std::string strUser;
	std::string strPassword;
}_DATABASEINFO, *_LPDATABASEINFO;

class S3PResultVBC;

class S3PDBConVBC  : public K_CS
{
public:
	virtual bool OpenConnect(_LPDATABASEINFO) = 0;
	virtual void CloseConnect() = 0;
	virtual bool QuerySql(const char* lpszSql, S3PResultVBC** ppResult) = 0;
	virtual bool Do(const char* lpszSql) = 0;

	~S3PDBConVBC();
	
protected:
	S3PDBConVBC();
};

#endif // !defined(AFX_S3PDBCONVBC_H__7CEDC5DC_8F90_45DA_B24D_27A082789125__INCLUDED_)
