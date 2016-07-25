//-----------------------------------------//
//                                         //
//  File		: S3PDBConnectionPool.cpp  //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/24/2002                //
//                                         //
//-----------------------------------------//
#include "S3PDBConnectionPool.h"

S3PDBConnectionPool* S3PDBConnectionPool::m_pInstance = NULL;
std::string S3PDBConnectionPool::m_strINIPath = "";
S3PDBConnectionPool::_DATABASEINFO S3PDBConnectionPool::m_AccountDBInfo;
S3PDBConnectionPool::_DATABASEINFO S3PDBConnectionPool::m_RoleDBInfo;
S3PDBConnectionPool::_DATABASEINFO S3PDBConnectionPool::m_CardDBInfo;

S3PDBConnectionPool::S3PDBConnectionPool()
{

}

S3PDBConnectionPool::~S3PDBConnectionPool()
{
	
}

void S3PDBConnectionPool::InitAccountDBInfo()
{
	TCHAR szServer[def_KEYNAMELEN];
	TCHAR szDataBase[def_KEYNAMELEN];
	TCHAR szUser[def_KEYNAMELEN];
	TCHAR szPassword[def_KEYNAMELEN];
	
	DWORD dwLen =
		KPIGetPrivateProfileString( def_ACCOUNTSECTIONNAME,
		def_SERVERKEYNAME,
		"",
		szServer,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szServer[dwLen] = 0;
	}
	else
	{
		szServer[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_ACCOUNTSECTIONNAME,
		def_DATABASEKEYNAME,
		"",
		szDataBase,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szDataBase[dwLen] = 0;
	}
	else
	{
		szDataBase[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_ACCOUNTSECTIONNAME,
		def_USERKEYNAME,
		"",
		szUser,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szUser[dwLen] = 0;
	}
	else
	{
		szUser[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_ACCOUNTSECTIONNAME,
		def_PASSWORDKEYNAME,
		"",
		szPassword,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szPassword[dwLen] = 0;
	}
	else
	{
		szPassword[0] = 0;
	}
	m_AccountDBInfo.strServer = szServer;
	m_AccountDBInfo.strDataBase = szDataBase;
	m_AccountDBInfo.strUser = szUser;
	m_AccountDBInfo.strPassword = szPassword;
}

void S3PDBConnectionPool::InitRoleDBInfo()
{
	TCHAR szServer[def_KEYNAMELEN];
	TCHAR szDataBase[def_KEYNAMELEN];
	TCHAR szUser[def_KEYNAMELEN];
	TCHAR szPassword[def_KEYNAMELEN];

	DWORD dwLen =
		KPIGetPrivateProfileString( def_ROLESECTIONNAME,
		def_SERVERKEYNAME,
		"",
		szServer,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szServer[dwLen] = 0;
	}
	else
	{
		szServer[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_ROLESECTIONNAME,
		def_DATABASEKEYNAME,
		"",
		szDataBase,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szDataBase[dwLen] = 0;
	}
	else
	{
		szDataBase[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_ROLESECTIONNAME,
		def_USERKEYNAME,
		"",
		szUser,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szUser[dwLen] = 0;
	}
	else
	{
		szUser[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_ROLESECTIONNAME,
		def_PASSWORDKEYNAME,
		"",
		szPassword,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szPassword[dwLen] = 0;
	}
	else
	{
		szPassword[0] = 0;
	}

	m_RoleDBInfo.strServer = szServer;
	m_RoleDBInfo.strDataBase = szDataBase;
	m_RoleDBInfo.strUser = szUser;
	m_RoleDBInfo.strPassword = szPassword;
}

void S3PDBConnectionPool::InitCardDBInfo()
{
	TCHAR szServer[def_KEYNAMELEN];
	TCHAR szDataBase[def_KEYNAMELEN];
	TCHAR szUser[def_KEYNAMELEN];
	TCHAR szPassword[def_KEYNAMELEN];

	DWORD dwLen =
		KPIGetPrivateProfileString( def_CARDSECTIONNAME,
		def_SERVERKEYNAME,
		"",
		szServer,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szServer[dwLen] = 0;
	}
	else
	{
		szServer[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_CARDSECTIONNAME,
		def_DATABASEKEYNAME,
		"",
		szDataBase,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szDataBase[dwLen] = 0;
	}
	else
	{
		szDataBase[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_CARDSECTIONNAME,
		def_USERKEYNAME,
		"",
		szUser,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szUser[dwLen] = 0;
	}
	else
	{
		szUser[0] = 0;
	}
	dwLen =
		KPIGetPrivateProfileString( def_CARDSECTIONNAME,
		def_PASSWORDKEYNAME,
		"",
		szPassword,
		def_KEYNAMELEN,
		m_strINIPath.c_str() );
	if ( dwLen < def_KEYNAMELEN )
	{
		szPassword[dwLen] = 0;
	}
	else
	{
		szPassword[0] = 0;
	}

	m_CardDBInfo.strServer = szServer;
	m_CardDBInfo.strDataBase = szDataBase;
	m_CardDBInfo.strUser = szUser;
	m_CardDBInfo.strPassword = szPassword;
}

void S3PDBConnectionPool::Init( const std::string &strINIPath )
{
	m_strINIPath = strINIPath;

	if ( !( m_strINIPath.empty() ) )
	{
		InitAccountDBInfo();
		InitRoleDBInfo();
		InitCardDBInfo();
	}
}

S3PDBConnectionPool* S3PDBConnectionPool::Instance()
{
	if ( NULL == m_pInstance )
	{
		m_pInstance = new S3PDBConnectionPool;
		//Question no delete,must delete in somewhere romandoufind
	}
	return m_pInstance;
}

void S3PDBConnectionPool::ReleaseInstance()
{
	if ( NULL != m_pInstance )
	{
		m_pInstance->Release();
		m_pInstance = NULL;
	}
}

_CONNECTION_ID S3PDBConnectionPool::ApplyAccountDBCon()
{
	_CONNECTION_ID ret;
	ret.dwID = def_ERRORCONID;
	ret.pCon = NULL;

	DWORD dwNullID = def_ERRORCONID;

	DWORD dwSize = m_AccountMap.size();
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		if ( NULL != m_AccountMap[i].pCon )
		{
			if ( TRUE != m_AccountMap[i].bBusy )
			{
				ret.dwID = i;
				ret.pCon = m_AccountMap[i].pCon;
				break;
			}
		}
		else if ( def_ERRORCONID == dwNullID )
		{
				dwNullID = i;
		}
	}

	if ( NULL == ret.pCon )
	{
		Connection* pCon =
			( Connection* )( malloc( sizeof( Connection ) ) );
		if ( NULL != pCon )
		{
			if ( TRUE == pCon->connect( m_AccountDBInfo.strDataBase.c_str(),
				m_AccountDBInfo.strServer.c_str(),
				m_AccountDBInfo.strUser.c_str(),
				m_AccountDBInfo.strPassword.c_str() ) )
			{
				if ( def_ERRORCONID != dwNullID )
				{
					ret.dwID = dwNullID;
				}
				else
				{
					ret.dwID = dwSize;
				}
				ret.pCon = pCon;
				_CONNECTIONITEM conItem = { TRUE, pCon };
				m_AccountMap[ret.dwID] = conItem;
			}
			else
			{
				free( pCon );
				pCon = NULL;
			}
		}
	}

	return ret;
}

_CONNECTION_ID S3PDBConnectionPool::ApplyRoleDBCon()
{
	_CONNECTION_ID ret;
	ret.dwID = def_ERRORCONID;
	ret.pCon = NULL;

	DWORD dwNullID = def_ERRORCONID;

	DWORD dwSize = m_RoleMap.size();
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		if ( NULL != m_RoleMap[i].pCon )
		{
			if ( TRUE != m_RoleMap[i].bBusy )
			{
				ret.dwID = i;
				ret.pCon = m_RoleMap[i].pCon;
				break;
			}
		}
		else if ( def_ERRORCONID == dwNullID )
		{
			dwNullID = i;
		}
	}
	if ( NULL == ret.pCon )
	{
		Connection* pCon =
			( Connection* )( malloc( sizeof( Connection ) ) );
		//Question up no delete ,must delete
		if ( NULL != pCon )
		{
			if ( TRUE == pCon->connect( m_RoleDBInfo.strDataBase.c_str(),
				m_RoleDBInfo.strServer.c_str(),
				m_RoleDBInfo.strUser.c_str(),
				m_RoleDBInfo.strPassword.c_str() ) )
			{
				if ( def_ERRORCONID != dwNullID )
				{
					ret.dwID = dwNullID;
				}
				else
				{
					ret.dwID = dwSize;
				}
				ret.pCon = pCon;
				_CONNECTIONITEM conItem = { TRUE, pCon };
				m_RoleMap[ret.dwID] = conItem;
			}
			else
			{
				free( pCon );
				pCon = NULL;
			}
		}
	}

	return ret;
}

_CONNECTION_ID S3PDBConnectionPool::ApplyCardDBCon()
{
	_CONNECTION_ID ret;
	ret.dwID = def_ERRORCONID;
	ret.pCon = NULL;
	DWORD dwNullID = def_ERRORCONID;

	DWORD dwSize = m_CardMap.size();
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		if ( NULL != m_CardMap[i].pCon )
		{
			if ( TRUE != m_CardMap[i].bBusy )
			{
				ret.dwID = i;
				ret.pCon = m_CardMap[i].pCon;
				break;
			}
		}
		else if ( def_ERRORCONID == dwNullID )
		{
			dwNullID = i;
		}
	}
	if ( NULL == ret.pCon )
	{
		Connection* pCon =
			( Connection* )( malloc( sizeof( Connection ) ) );
		if ( NULL != pCon )
		{
			if ( TRUE == pCon->connect( m_CardDBInfo.strDataBase.c_str(),
				m_CardDBInfo.strServer.c_str(),
				m_CardDBInfo.strUser.c_str(),
				m_CardDBInfo.strPassword.c_str() ) )
			{
				if ( def_ERRORCONID != dwNullID )
				{
					ret.dwID = dwNullID;
				}
				else
				{
					ret.dwID = dwSize;
				}
				ret.pCon = pCon;
				_CONNECTIONITEM conItem = { TRUE, pCon };
				m_CardMap[ret.dwID] = conItem;
			}
			else
			{
				free( pCon );
				pCon = NULL;
			}
		}
	}

	return ret;
}

_CONNECTION_ID S3PDBConnectionPool::ApplyDBConnection( int iDBIdentifier )
{
	_CONNECTION_ID ret;
	ret.dwID = def_ERRORCONID;
	ret.pCon = NULL;
	switch ( iDBIdentifier )
	{
	case def_ACCOUNTDB:
		ret = ApplyAccountDBCon();
		break;
	case def_ROLEDB:
		ret = ApplyRoleDBCon();
		break;
	case def_CARDDB:
		ret = ApplyCardDBCon();
		break;
	default:
		break;
	}
	return ret;
}

BOOL S3PDBConnectionPool::SetAccountDBConFree( DWORD dwConID )
{
	BOOL bRet = FALSE;
	ConMap::iterator accountIterator = m_AccountMap.find( dwConID );
	if ( m_AccountMap.end() != accountIterator )
	{
		( *accountIterator ).second.bBusy = FALSE;
	}
	return bRet;
}

BOOL S3PDBConnectionPool::SetRoleDBConFree( DWORD dwConID )
{
	BOOL bRet = FALSE;
	ConMap::iterator roleIterator = m_RoleMap.find( dwConID );
	if ( m_RoleMap.end() != roleIterator )
	{
		( *roleIterator ).second.bBusy = FALSE;
	}
	return bRet;
}

BOOL S3PDBConnectionPool::SetCardDBConFree( DWORD dwConID )
{
	BOOL bRet = FALSE;
	ConMap::iterator cardIterator = m_CardMap.find( dwConID );
	if ( m_CardMap.end() != cardIterator )
	{
		( *cardIterator ).second.bBusy = FALSE;
	}
	return bRet;
}

BOOL S3PDBConnectionPool::SetDBConFree( int iDBIdentifier, DWORD dwConID )
{
	BOOL bRet = FALSE;
	switch ( iDBIdentifier )
	{
	case def_ACCOUNTDB:
		bRet = SetAccountDBConFree( dwConID );
		break;
	case def_ROLEDB:
		bRet = SetRoleDBConFree( dwConID );
		break;
	case def_CARDDB:
		bRet = SetCardDBConFree( dwConID );
		break;
	default:
		break;
	}
	return bRet;
}

void S3PDBConnectionPool::ReleaseAccountMap()
{
	DWORD dwSize = m_AccountMap.size();
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		if ( NULL != m_AccountMap[i].pCon )
		{
			m_AccountMap[i].pCon->close();
			free( m_AccountMap[i].pCon );
			m_AccountMap[i].pCon = NULL;
		}
	}
	m_AccountMap.clear();
}

void S3PDBConnectionPool::ReleaseRoleMap()
{
	DWORD dwSize = m_RoleMap.size();
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		if ( NULL != m_RoleMap[i].pCon )
		{
			m_RoleMap[i].pCon->close();
			free( m_RoleMap[i].pCon );
			m_RoleMap[i].pCon = NULL;
		}
	}
	m_RoleMap.clear();
}

void S3PDBConnectionPool::ReleaseCardMap()
{
	DWORD dwSize = m_CardMap.size();
	for ( DWORD i = 0; i < dwSize; i++ )
	{
		if ( NULL != m_CardMap[i].pCon )
		{
			m_CardMap[i].pCon->close();
			free( m_CardMap[i].pCon );
			m_CardMap[i].pCon = NULL;
		}
	}
	m_CardMap.clear();
}

void S3PDBConnectionPool::Release()
{
	ReleaseAccountMap();
	ReleaseRoleMap();
	ReleaseCardMap();
}