//-----------------------------------------//
//                                         //
//  File		: S3PDBConnectionPool.cpp  //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#include "GlobalFun.h"

#include "S3PDBConnectionPool.h"

#include "S3PDB_MSSQLServer_Connection.h"
#include "S3PAccount.h"

S3PDBConnectionPool* S3PDBConnectionPool::m_pInstance = NULL;

static DWORD g_nGMID = 0;

DWORD GetGMID()
{
	return g_nGMID;
}

S3PDBConnectionPool::S3PDBConnectionPool()
{
	m_dwConLimits = 0;
}

S3PDBConnectionPool::~S3PDBConnectionPool()
{
	for (int i = 0; i < m_ConVBCPool.size(); i++)
	{
		if (m_ConVBCPool[i])
		{
			m_ConVBCPool[i]->CloseConnect();
			delete m_ConVBCPool[i];
		}
	}
}

BOOL S3PDBConnectionPool::Init(const std::string &strINIPath, const std::string &strSection, DWORD dwConLimits)
{
	m_dwConLimits = dwConLimits;

	if (!(strINIPath.empty()))
	{
		TCHAR szServer[def_KEYNAMELEN];
		TCHAR szDataBase[def_KEYNAMELEN];
		TCHAR szUser[def_KEYNAMELEN];
		TCHAR szPassword[def_KEYNAMELEN];
		
		DWORD dwLen =
			KPIGetPrivateProfileString(strSection.c_str(),
			def_SERVERKEYNAME,
			"",
			szServer,
			def_KEYNAMELEN,
			strINIPath.c_str() );
		dwLen =
			KPIGetPrivateProfileString( strSection.c_str(),
			def_DATABASEKEYNAME,
			"",
			szDataBase,
			def_KEYNAMELEN,
			strINIPath.c_str() );
		dwLen =
			KPIGetPrivateProfileString( strSection.c_str(),
			def_USERKEYNAME,
			"",
			szUser,
			def_KEYNAMELEN,
			strINIPath.c_str() );
		dwLen =
			KPIGetPrivateProfileString( strSection.c_str(),
			def_PASSWORDKEYNAME,
			"",
			szPassword,
			def_KEYNAMELEN,
			strINIPath.c_str() );

		_DATABASEINFO DBInfo;
		DBInfo.strServer = szServer;
		DBInfo.strDataBase = szDataBase;
		DBInfo.strUser = szUser;
		DBInfo.strPassword = szPassword;

		S3PDBConVBC* pVBC = NULL;
		for (int i = 0; i < m_dwConLimits; i++)
		{
			if (CreateConnection(&DBInfo, &pVBC))
			{
				m_ConVBCPool.push_back(pVBC);
			}
			else
				break;
		}
	}

	if (m_ConVBCPool.size() > 0 && m_ConVBCPool[0])
	{
		S3PAccount::GetServerID(m_ConVBCPool[0], "gm-kingsoft", g_nGMID);
	}

	return m_ConVBCPool.size() == m_dwConLimits;
}

BOOL S3PDBConnectionPool::RemoveDBCon(S3PDBConVBC** ppInfo)
{
	while (IsLocked())
		Sleep(1);
	Lock();
	BOOL b = FALSE;
	for (int nIndex = 0; nIndex < m_ConVBCPool.size(); nIndex++)
	{
		if (m_ConVBCPool[nIndex] && !m_ConVBCPool[nIndex]->IsLocked())
		{
			*ppInfo = m_ConVBCPool[nIndex];
			m_ConVBCPool[nIndex]->Lock();
			b = TRUE;
			gTrace("DB Connect %d is busy !", nIndex);
			break;
		}
	}
	Unlock();

	return b;
}

BOOL S3PDBConnectionPool::ReturnDBCon(S3PDBConVBC* pInfo)
{
	while (IsLocked())
		Sleep(1);
	Lock();
	BOOL b = FALSE;
	for (int nIndex = 0; nIndex < m_ConVBCPool.size(); nIndex++)
	{
		if (m_ConVBCPool[nIndex] == pInfo && m_ConVBCPool[nIndex]->IsLocked())
		{
			m_ConVBCPool[nIndex]->Unlock();
			b = TRUE;
			gTrace("DB Connect %d is free !", nIndex);
			break;
		}
	}
	Unlock();

	return b;
}

S3PDBConnectionPool* S3PDBConnectionPool::Instance()
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new S3PDBConnectionPool;
	}
	return m_pInstance;
}

void S3PDBConnectionPool::ReleaseInstance()
{
	if ( NULL != m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

BOOL S3PDBConnectionPool::CreateConnection(_LPDATABASEINFO pDatabase, S3PDBConVBC** ppInfo)
{
	S3PDBConVBC* pCon = new S3PDB_MSSQLServer_Connection();
	if (pCon->OpenConnect(pDatabase))
	{
		*ppInfo = pCon;
		return TRUE;
	}
	delete pCon;
	pCon = NULL;
	return FALSE;
}