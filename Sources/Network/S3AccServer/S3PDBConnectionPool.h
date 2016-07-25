//-----------------------------------------//
//                                         //
//  File		: S3PDBConnectionPool.h    //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBCONNECTIONPOOL_H_
#define _S3PDBCONNECTIONPOOL_H_

#include "KStdAfx.h"
#include <list>
#include <vector>
#include "KThread.h"
#include "S3PDBConVBC.h"

class S3PDBConnectionPool : public K_CS
{
public:
	static S3PDBConnectionPool* Instance();
	static void ReleaseInstance();

	BOOL Init(const std::string &strINIPath, const std::string &strSection, DWORD dwConLimits);
	BOOL RemoveDBCon(S3PDBConVBC** ppInfo);
	BOOL ReturnDBCon(S3PDBConVBC* pInfo);

protected:
	static S3PDBConnectionPool* m_pInstance;

	DWORD m_dwConLimits;

	std::vector<S3PDBConVBC*> m_ConVBCPool;
	
	BOOL CreateConnection(_LPDATABASEINFO pDatabase, S3PDBConVBC** ppInfo);

protected:
	S3PDBConnectionPool();
	virtual ~S3PDBConnectionPool();
};

#endif	// _S3PDBCONNECTIONPOOL_H_
