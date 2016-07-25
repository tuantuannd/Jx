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

#include <sqlplus.hh>
#include <custom.hh>

#include <string>
#include <map>

#define def_ERRORCONID			0xffffffff

typedef struct tag_CONNECTION_ID
{
	DWORD dwID;
	Connection* pCon;
}_CONNECTION_ID, * _LPCONNECTION_ID;

class S3PDBConnectionPool
{
	typedef struct tag_DATABASEINFO
	{
		std::string strServer;
		std::string strDataBase;
		std::string strUser;
		std::string strPassword;
	}_DATABASEINFO, *_LPDATABASEINFO;
	typedef struct tag_CONNECTIONITEM
	{
		BOOL bBusy;
		Connection* pCon;
	}_CONNECTIONITEM, *_LPCONNECTIONITEM;
	typedef std::map< DWORD, _CONNECTIONITEM > ConMap;
// Static members
//------>BEGIN
public:
	static void Init( const std::string &strINIPath );
	static S3PDBConnectionPool* Instance();
	static void ReleaseInstance();
protected:
	static void InitAccountDBInfo();
	static void InitRoleDBInfo();
	static void InitCardDBInfo();

	static S3PDBConnectionPool* m_pInstance;
	static std::string m_strINIPath;

	static _DATABASEINFO m_AccountDBInfo;
	static _DATABASEINFO m_RoleDBInfo;
	static _DATABASEINFO m_CardDBInfo;
//<------END

public:
	virtual _CONNECTION_ID ApplyDBConnection( int iDBIdentifier );
	virtual BOOL SetDBConFree( int iDBIdentifier, DWORD dwConID );

protected:
	S3PDBConnectionPool();
	virtual ~S3PDBConnectionPool();

	virtual _CONNECTION_ID ApplyAccountDBCon();
	virtual _CONNECTION_ID ApplyRoleDBCon();
	virtual _CONNECTION_ID ApplyCardDBCon();

	virtual BOOL SetAccountDBConFree( DWORD dwConID );
	virtual BOOL SetRoleDBConFree( DWORD dwConID );
	virtual BOOL SetCardDBConFree( DWORD dwConID );
	
	virtual void ReleaseAccountMap();
	virtual void ReleaseRoleMap();
	virtual void ReleaseCardMap();
	virtual void Release();

protected:
	ConMap m_AccountMap;
	ConMap m_RoleMap;
	ConMap m_CardMap;
};

#endif	// _S3PDBCONNECTIONPOOL_H_
