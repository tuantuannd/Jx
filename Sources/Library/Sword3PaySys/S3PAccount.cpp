//-----------------------------------------//
//                                         //
//  File		: S3PAccount.cpp		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/29/2002                //
//                                         //
//-----------------------------------------//

#include "S3PAccount.h"

S3PAccount::S3PAccount():
m_bLocked( TRUE )
{
}

S3PAccount::~S3PAccount()
{
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: ActivateAccount
//	Return	: BOOL
//			  = TRUE	---- Successful;
//			  = FALSE	---- Failed to access database.
//	Description	: Activate a account.
//----------------------------------------------------------
BOOL S3PAccount::ActivateAccount( const std::string strAccName,
								 const std::string strPassword,
								 const DWORD dwGameID )
{
	BOOL bRet = FALSE;

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );

	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		char szBuffer[20];
		_ltoa( dwGameID, szBuffer, 10 );
		cav["igameid"] = szBuffer;

		ColumnAndValue where;
		where["caccname"] = strAccName;
		where["strpassword"] = strPassword;
		where["igameid"] = szBuffer;
		
		
		S3PRow row( account.GetTableName(), &cav, pAccountCon );
		S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );

		if ( account.Update( &row, &rowWhere ) <= 0 )
		{
			bRet = FALSE;
			assert( false );
		}
		else
		{
			bRet = TRUE;
		}
		pAccountCon->Close();
	}
	
	return bRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: AddAccount
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strAccName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Parameter "strRealName" is null;
//			  = -4	---- The account has existed;
//			  = others	---- Not defined.
//	Description	: Add a new account.
//----------------------------------------------------------
int S3PAccount::AddAccount( const std::string strAccName,
							const std::string strPassword,
							const std::string strRealName,
							const std::string strIDNum,
							const std::string strBirthDay,
							const std::string strArea,
							const std::string strPhone )
{
	int iRet = 1;

	if ( strAccName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	if ( strRealName.empty() )
	{
		iRet = -3;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		cav["caccname"] = strAccName;
		cav["cpassword"] = strPassword;
		cav["crealname"] = strRealName;
		if ( !( strIDNum.empty() ) )
		{
			cav["cidnum"] = strIDNum;
		}
		if ( !( strBirthDay.empty() ) )
		{
			cav["dbirthday"] = strBirthDay;
		}
		if ( !( strArea.empty() ) )
		{
			cav["carea"] = strArea;
		}
		if ( !( strPhone.empty() ) )
		{
			cav["cphone"] = strPhone;
		}
		cav["dregdate"] = pAccountCon->GetDate();

		ColumnAndValue where;
		where["caccname"] = strAccName;
		
		S3PRow rowCav( account.GetTableName(), &cav, pAccountCon );
		S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );
		if ( false == account.HasItem( &rowWhere ) )
		{
			int iResult = account.Add( &rowCav );
			if ( iResult <= 0 )
			{
				iRet = 0;
			}
			else
			{
				iRet = 1;
			}
		}
		else
		{
			iRet = -4;
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}

	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: Login
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strAccName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- The same user has logged in from
//					     other host machine;
//			  = -4	---- The user has not registered;
//			  = others	---- Not defined.
//	Description	: login from game.
//----------------------------------------------------------
int S3PAccount::Login( const std::string strAccName, const std::string strPassword, int iGameId )
{
	int iRet = 1;
	if ( strAccName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		char szGameID[11];	// The maxima number is 4294967296
		memset( szGameID, 0, 11 );
		_itoa( iGameId, szGameID, 10 );
		cav["igameid"] = szGameID;
		cav["itimecount"] = "1";
		
		ColumnAndValue where;
		where["caccname"] = strAccName;
		where["cpassword"] = strPassword;
		S3PRow rowWhere0( account.GetTableName(), &where, pAccountCon );
		if ( account.HasItem( &rowWhere0 ) )
		{
			where["igameid"] = "0";
			S3PRow rowWhere1( account.GetTableName(), &where, pAccountCon );
			if ( account.HasItem( &rowWhere1 ) )
			{
				if ( 0 != iGameId )
				{
					S3PRow rowCav( account.GetTableName(), &cav, pAccountCon );
					S3PRow rowWhere2( account.GetTableName(), &where, pAccountCon );
					if ( account.Update( &rowCav, &rowWhere2 ) <= 0 )
					{
						iRet = 0;
					}
					else
					{
						iRet = 1;
					}
				}
				else
				{
					iRet = 1;
				}
			}
			else
			{
				iRet = -3;
			}
		}
		else
		{
			iRet = -4;
		}
		
		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}
	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: Logout
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strAccName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = others	---- Not defined.
//	Description	: logout from game.
//----------------------------------------------------------
int S3PAccount::Logout( const std::string strAccName, const std::string strPassword, int iGameId )
{
	int iRet = 1;
	if ( strAccName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		char szGameId[11];	// The maxima number is 4294967296
		memset( szGameId, 0, 11 );
		_itoa( iGameId, szGameId, 10 );
		cav["igameid"] = "0";
		cav["itimecount"] = "0";
		
		ColumnAndValue where;
		where["caccname"] = strAccName;
		where["cpassword"] = strPassword;
		where["igameid"] = szGameId;

		S3PRow rowCav( account.GetTableName(), &cav, pAccountCon );
		S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );

		
		if ( account.Update( &rowCav, &rowWhere ) <= 0 )
		{
			iRet = 0;
		}
		else
		{
			iRet = 1;
		}
		
		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}

	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: Report
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strAccName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Has been disconnected;
//			  = -4	---- User has not registered;
//			  = others	---- Not defined.
//	Description	: 与服务器握手,用于检查游戏是否在线.
//----------------------------------------------------------
int S3PAccount::Report( const std::string strAccName, const std::string strPassword, int iGameId )
{
	int iRet = 1;
	if ( strAccName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		ColumnAndValue cav;
		char szGameId[11];	// The maxima number is 4294967296
		memset( szGameId, 0, 11 );
		_itoa( iGameId, szGameId, 10 );
		cav["itimecount"] = "1";
		ColumnAndValue where0;
		where0["caccname"] = strAccName;
		where0["cpassword"] = strPassword;
		S3PRow rowWhere0( account.GetTableName(), &where0, pAccountCon );
		if ( account.HasItem( &rowWhere0 ) )
		{
			ColumnAndValue where;
			where["caccname"] = strAccName;
			where["cpassword"] = strPassword;
			where["igameid"] = "0";
			
			S3PRow rowCav( account.GetTableName(), &cav, pAccountCon );
			S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );
			
			if ( !( account.HasItem( &rowWhere ) ) )
			{
				where["igameid"] = szGameId;
				where["itimecount"] = "0";
				S3PRow rowWhere1( account.GetTableName(), &where, pAccountCon );
				if ( account.Update( &rowCav, &rowWhere1 ) <= 0 )
				{
					iRet = 0;
				}
				else
				{
					iRet = 1;
				}
			}
			else
			{
				iRet = -3;
			}
		}
		else
		{
			iRet = -4;
		}
		
		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}

	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: QueryGameserverList
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strAccName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- i)The account doesn't exist,
//					     ii)account or password is wrong;
//			  = -4	---- The size of "pServers"("dwSize") is
//					     smaller than the reality;
//			  = others	---- Not defined.
//	Description	: Query game server list.
//----------------------------------------------------------
int S3PAccount::QueryGameserverList( const std::string strAccName,
									const std::string strPassword,
									IBYTE* pServers, DWORD& dwSize )
{
	int iRet = 1;
	if ( strAccName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountInfoDAO account( pAccountCon );
		S3PServerListDAO serverList( pAccountCon );

		ColumnAndValue where;
		where["caccname"] = strAccName;
		where["cpassword"] = strPassword;

		S3PRow rowWhere( account.GetTableName(), &where, pAccountCon );
		if ( account.HasItem( &rowWhere ) )
		{
			std::string strTableName = serverList.GetTableName();
			std::string strSql = "select * from ";
			strSql += strTableName;
			S3PResult res;
			serverList.Query( strSql, res );

			int iRowNum = res.num_rows();
			DWORD dwSizeTemp = iRowNum * sizeof( KLoginGameServer );
			if ( dwSizeTemp > dwSize )
			{
				iRet = -4;
			}
			else
			{
				if ( NULL != pServers )
				{
					memset( pServers, 0, dwSizeTemp );
					res.data_seek( 0 );
					for ( int i = 0; i < iRowNum; i++ )
					{
						ColumnAndValue cav = res[i];
						std::string strName = cav["cservername"];
						std::string strIP = cav["dwip"];
						std::string strPort = cav["iport"];
						std::string strID = cav["iid"];
						DWORD dwIP = atol( strIP.c_str() );
						int iPort = atol( strPort.c_str() );
						DWORD dwID = atol( strID.c_str() );

						int iLength = strName.length();
						int offset = 0;
						memcpy( &( pServers[i*sizeof( KLoginGameServer )+offset] ),
							strName.c_str(),
							GAMESERVERNAME_MAX_LEN );
						offset += GAMESERVERNAME_MAX_LEN + 2;
						memcpy( &( pServers[i*sizeof( KLoginGameServer )+offset] ),
							&dwIP, sizeof( DWORD ) );
						offset += sizeof( DWORD );
						memcpy( &( pServers[i*sizeof( KLoginGameServer )+offset] ),
							&iPort, sizeof( short ) );
						offset += sizeof( short );
						memcpy( &( pServers[i*sizeof( KLoginGameServer )+offset] ),
							&dwID, sizeof( DWORD ) );
					}
					iRet = 1;
				}
			}
			dwSize = dwSizeTemp;
		}
		else
		{
			iRet = -3;	// 未注册用户或者用户名、密码错误
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}
	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBLogin
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Parameter "strHostAddr" is null;
//			  = -4	---- User has logged in.
//			  = others	---- Not defined.
//	Description	: log in account database as database
//				  remote user.
//----------------------------------------------------------
int S3PAccount::DBLogin( const std::string strUserName,
						const std::string strPassword,
						const std::string strHostAddr )
{
	int iRet = 1;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	if ( strHostAddr.empty() )
	{
		iRet = -3;
		return iRet;
	}
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );
			
		ColumnAndValue where0;
		where0["szusername"] = strUserName;
		where0["szpassword"] = strPassword;
		where0["eloggedin"] = "N";
		
		S3PRow rowWhere0( user.GetTableName(), &where0, pAccountCon );
		if ( user.HasItem( &rowWhere0 ) )
		{
			ColumnAndValue cav;
			cav["eloggedin"] = "Y";
			cav["szhostaddr"] = strHostAddr;
			cav["dlastlogintime"] = pAccountCon->GetDateTime();

			ColumnAndValue where1;
			where1["szusername"] = strUserName;
			where1["szpassword"] = strPassword;

			S3PRow rowCav( user.GetTableName(), &cav, pAccountCon );
			S3PRow rowWhere1( user.GetTableName(), &where1, pAccountCon );
			if ( user.Update( &rowCav, &rowWhere1 ) <= 0 )
			{
				iRet = 0;
			}
			else
			{
				iRet = 1;
			}
		}
		else
		{
			where0["eloggedin"] = "Y";
			S3PRow rowWhereTemp( user.GetTableName(), &where0, pAccountCon );
			if ( user.HasItem( &rowWhereTemp ) )
				iRet = -4; // User has logged in.
			else
				iRet = 0;
		}
		
		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}
	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBLogout
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = others	---- Not defined.
//	Description	: log out from account database as database
//				  remote user.
//----------------------------------------------------------
int S3PAccount::DBLogout( const std::string strUserName,
						 const std::string strPassword )
{
	int iRet = 1;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );

		ColumnAndValue cav;
		cav["eloggedin"] = "N";
		cav["dlastlogouttime"] = pAccountCon->GetDateTime();
		
		ColumnAndValue where;
		where["szusername"] = strUserName;
		where["szpassword"] = strPassword;
		
		S3PRow rowCav( user.GetTableName(), &cav, pAccountCon );
		S3PRow rowWhere( user.GetTableName(), &where, pAccountCon );
		if ( user.Update( &rowCav, &rowWhere ) <= 0 )
		{
			iRet = 0;
		}
		else
		{
			iRet = 1;
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}

	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBQueryUserList
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Has no enough priority;
//			  = -4	---- Has no such user or password is wrong
//					     or the user has not logged in;
//			  = -5	---- User list is empty;
//			  = -6	---- The size of pUser buffer is not enough big;
//			  = others	---- Not defined.
//	Description	: Query user list from account database
//				  as database remote user.
//----------------------------------------------------------
int S3PAccount::DBQueryUserList( const std::string strUserName,
								const std::string strPassword,
								IBYTE* pUsers, DWORD& dwSize )
{
	int iRet = 1;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );

		std::string strTableName = user.GetTableName();
		std::string strSql = "select * from ";
		strSql += strTableName;
		strSql += " where szUserName=\'";
		strSql += strUserName;
		strSql += "\' and szPassword=\'";
		strSql += strPassword;
		strSql += "\' and eLoggedin=\'Y\'";
		S3PResult res;
		user.Query( strSql, res );
		
		int iRowNum = res.num_rows();
		res.data_seek( 0 );
		if ( 1 == iRowNum )
		{
			ColumnAndValue cav = res[0];
			std::string strPriority = cav["epriority"];
			BOOL bHasPriority = FALSE;
			if ( 0 == stricmp( "Read", strPriority.c_str() ) )
			{
				bHasPriority = TRUE;
			}
			else if ( 0 == stricmp( "Read-Write", strPriority.c_str() ) )
			{
				bHasPriority = TRUE;
			}
			else if ( 0 == stricmp( "Administrator", strPriority.c_str() ) )
			{
				bHasPriority = TRUE;
			}
			if ( TRUE == bHasPriority )
			{
				strSql = "select * from ";
				strSql += strTableName;
				S3PResult queryUserListRes;
				user.Query( strSql, queryUserListRes );
				iRowNum = queryUserListRes.num_rows();
				queryUserListRes.data_seek( 0 );
				if ( 0 < iRowNum )
				{
					DWORD dwSizeTemp = iRowNum * sizeof( _USERINFO );
					if ( NULL != pUsers )
					{
						if ( dwSizeTemp > dwSize )
						{
							iRet = -6;
						}
						else
						{
							memset( pUsers, 0, dwSize );
							for ( int i = 0; i < iRowNum; i++ )
							{
								int offset = 0;
								ColumnAndValue userinfo = queryUserListRes[i];
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset], 
									userinfo["szhostaddr"].c_str(), 16 );
								offset += 16;
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset],
									userinfo["szusername"].c_str(),
									userinfo["szusername"].size() );
								offset += def_DBUSERNAME_MAX_LEN + 1;
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset],
									userinfo["szpassword"].c_str(),
									userinfo["szpassword"].size() );
								offset += def_DBPASSWORD_MAX_LEN + 1;
								short siPriority;
								std::string strPriority = userinfo["epriority"];
								if ( 0 == stricmp( strPriority.c_str(), "Read" ) )
								{
									siPriority = 0;
								}
								else if ( 0 == stricmp( strPriority.c_str(), "Write" ) )
								{
									siPriority = 1;
								}
								else if ( 0 == stricmp( strPriority.c_str(), "Read-Write" ) )
								{
									siPriority = 2;
								}
								else if ( 0 == stricmp( strPriority.c_str(), "Administrator" ) )
								{
									siPriority = 3;
								}
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset],
									&siPriority, sizeof( short ) );
								offset += sizeof( short );
								BOOL bLoggedin;
								std::string strLoggedin = userinfo["eloggedin"];
								if ( 0 == stricmp( strLoggedin.c_str(), "N" ) )
								{
									bLoggedin = FALSE;
								}
								else if ( 0 == stricmp( strLoggedin.c_str(), "Y" ) )
								{
									bLoggedin = TRUE;
								}
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset],
									&bLoggedin, sizeof( BOOL ) );
								offset += sizeof( BOOL );
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset],
									userinfo["dlastlogintime"].c_str(),
									userinfo["dlastlogintime"].size() );
								offset += 20;
								memcpy( &pUsers[i*sizeof( _USERINFO )+offset],
									userinfo["dlastlogouttime"].c_str(),
									userinfo["dlastlogouttime"].size() );
								offset += 20;
							}
							iRet = 1;
						}
					}
					else
					{
						iRet = 1;
					}
					dwSize = dwSizeTemp;
				}
				else
				{
					iRet = -5;
					// or iRet = 0;
				}
			}
			else
			{
				iRet = -3;	// Has no enough priority.
			}
		}
		else
		{
			iRet = -4;
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}

	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBLock
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = others	---- Not defined.
//	Description	: Check whether the specified user who carry out
//			      locking account database operation is connected.
//----------------------------------------------------------
int S3PAccount::DBLock( const std::string strUserName, const std::string strPassword )
{
	int iRet = 1;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );
		
		ColumnAndValue where;
		where["szusername"] = strUserName;
		where["szpassword"] = strPassword;
		where["eloggedin"] = "Y";
		
		S3PRow rowWhere( user.GetTableName(), &where, pAccountCon );
		if ( user.HasItem( &rowWhere ) )
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}
	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBLock
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = others	---- Not defined.
//	Description	: Check whether the specified user who carry out
//			      activating account database operation is connected.
//----------------------------------------------------------
int S3PAccount::DBActivate( const std::string strUserName, const std::string strPassword )
{
	int iRet = 1;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}

	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );
		
		ColumnAndValue where;
		where["szusername"] = strUserName;
		where["szpassword"] = strPassword;
		where["eloggedin"] = "Y";
		
		S3PRow rowWhere( user.GetTableName(), &where, pAccountCon );
		if ( user.HasItem( &rowWhere ) )
		{
			iRet = 1;
		}
		else
		{
			iRet = 0;
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}
	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBAddUser
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Parameter "strNewUserName" is null;
//			  = -4	---- Parameter "strNewUserPassword" is null;
//			  = -5	---- Parameter "siNewUserPriority" is invalid;
//			  = -6	---- Has no enough priority;
//			  = -7	---- The user has existed;
//			  = others	---- Not defined.
//	Description	: Add new administrator of account database.
//----------------------------------------------------------
int S3PAccount::DBAddUser( const std::string strUserName, const std::string strPassword,
						  const std::string strNewUserName, const std::string strNewUserPassword,
						  short int siNewUserPriority )
{
	int iRet = 0;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	if ( strNewUserName.empty() )
	{
		iRet = -3;
		return iRet;
	}
	if ( strNewUserPassword.empty() )
	{
		iRet = -4;
		return iRet;
	}
	if ( ( 0 > siNewUserPriority ) || ( 3 < siNewUserPriority ) )
	{
		iRet = -5;
		return iRet;
	}
	
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );

		std::string strTableName = user.GetTableName();
		std::string strSql = "select * from ";
		strSql += strTableName;
		strSql += " where szUserName=\'";
		strSql += strUserName;
		strSql += "\' and szPassword=\'";
		strSql += strPassword;
		strSql += "\' and eLoggedin=\'Y\'";
		S3PResult res;
		user.Query( strSql, res );
		
		int iRowNum = res.num_rows();
		res.data_seek( 0 );
		if ( 1 == iRowNum )
		{
			ColumnAndValue cav = res[0];
			std::string strPriority = cav["epriority"];
			BOOL bHasPriority = FALSE;
			if ( 0 == stricmp( "Write", strPriority.c_str() ) )
			{
				if ( ( 0 < siNewUserPriority )
					&& ( 1 >= siNewUserPriority ) )
				{
					bHasPriority = TRUE;
				}
			}
			else if ( 0 == stricmp( "Read-Write", strPriority.c_str() ) )
			{
				if ( ( 0 < siNewUserPriority )
					&& ( 2 >= siNewUserPriority ) )
				{
					bHasPriority = TRUE;
				}
			}
			else if ( 0 == stricmp( "Administrator", strPriority.c_str() ) )
			{
				bHasPriority = TRUE;
			}
			if ( TRUE == bHasPriority )
			{
				ColumnAndValue where;
				where["szusername"] = strNewUserName;
				S3PRow rowWhere( user.GetTableName(), &where, pAccountCon );
				if ( false == user.HasItem( &rowWhere ) )
				{
					ColumnAndValue newCav;
					newCav["szusername"] = strNewUserName;
					newCav["szpassword"] = strNewUserPassword;
					newCav["eloggedin"] = "N";
					//newCav["szhostaddr"] = "0.0.0.0";
					//newCav["dlastlogintime"] = "1978-2-21 12:00:00";
					//newCav["dlastlogouttime"] = "1978-2-21 12:00:00";
					switch ( siNewUserPriority )
					{
					case 0:
						newCav["epriority"] = "Read";
						break;
					case 1:
						newCav["epriority"] = "Write";
						break;
					case 2:
						newCav["epriority"] = "Read-Write";
						break;
					case 3:
						newCav["epriority"] = "Administrator";
						break;
					}
					S3PRow rowCav( user.GetTableName(), &newCav, pAccountCon );
					int iResult = user.Add( &rowCav );
					if ( iResult <= 0 )
					{
						iRet = 0;
					}
					else
					{
						iRet = 1;
					}
				}
				else
				{
					iRet = -7;
				}
			}
			else
			{
				iRet = -6;
			}
		}
		else
		{
			iRet = 0;
		}

		pAccountCon->Close();
	}
	else
	{
		iRet = 0;
	}

	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBDeleteUser
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Parameter "strSelUserName" is null;
//			  = -4	---- Has no enough priority or is disconnected;
//			  = others	---- Not defined.
//	Description	: Delete specified administrator of account database.
//----------------------------------------------------------
int S3PAccount::DBDeleteUser( const std::string strUserName,
							 const std::string strPassword,
							 const std::string strSelUserName )
{
	int iRet = 0;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	if ( strSelUserName.empty() )
	{
		iRet = -3;
		return iRet;
	}
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );
		ColumnAndValue where1;
		where1["szusername"] = strUserName;
		where1["szpassword"] = strPassword;
		where1["eloggedin"] = "Y";
		where1["epriority"] = "Administrator";
		S3PRow rowWhere1( user.GetTableName(), &where1, pAccountCon );
		if ( user.HasItem( &rowWhere1 ) )
		{
			ColumnAndValue where2;
			where2["szusername"] = strSelUserName;
			S3PRow rowWhere2( user.GetTableName(), &where2, pAccountCon );
			if ( 1 == user.Delete( &rowWhere2 ) )
			{
				iRet = 1;
			}
			else
			{
				iRet = 0;
			}
		}
		else
		{
			iRet = -4;
		}
		pAccountCon->Close();
	}
	return iRet;
}

//----------------------------------------------------------
//	< Static Member Function >
//	Name	: DBDeleteUser
//	Return	: int
//			  = 1	---- Successful;
//			  = 0	---- Failed to access database;
//			  = -1	---- Parameter "strUserName" is null;
//			  = -2	---- Parameter "strPassword" is null;
//			  = -3	---- Parameter "strAccRealName" is null;
//			  = -4	---- Parameter "strAccPassword" is null;
//			  = -5	---- Parameter "strAccName" is null;
//			  = -6	---- Has no enough priority or is disconnected;
//			  = -7	---- The account has existed;
//			  = others	---- Not defined.
//	Description	: Create new account.
//----------------------------------------------------------
int S3PAccount::DBCreateAccount( const std::string strUserName,
								const std::string strPassword,
								const std::string strAccRealName,
								const std::string strAccPassword,
								const std::string strAccName )
{
	int iRet = 1;
	if ( strUserName.empty() )
	{
		iRet = -1;
		return iRet;
	}
	if ( strPassword.empty() )
	{
		iRet = -2;
		return iRet;
	}
	if ( strAccRealName.empty() )
	{
		iRet = -3;
		return iRet;
	}
	if ( strAccPassword.empty() )
	{
		iRet = -4;
		return iRet;
	}
	if ( strAccName.empty() )
	{
		iRet = -5;
		return iRet;
	}
	S3PDBConnection* pAccountCon =
		S3PDBConnector::Instance()->ApplyDBConnection( def_ACCOUNTDB );
	if ( NULL != pAccountCon )
	{
		S3PAccountUserDAO user( pAccountCon );
		ColumnAndValue where;
		where["szusername"] = strUserName;
		where["szpassword"] = strPassword;
		where["eloggedin"] = "Y";
		where["epriority"] = "Administrator";
		S3PRow rowWhere( user.GetTableName(), &where, pAccountCon );
		if ( user.HasItem( &rowWhere ) )
		{
			S3PAccountInfoDAO account( pAccountCon );
			
			ColumnAndValue where1;
			where1["caccname"] = strAccName;
			S3PRow rowWhere1( account.GetTableName(), &where1, pAccountCon );
			if ( !( account.HasItem( &rowWhere1 ) ) )
			{
				ColumnAndValue cav;
				cav["caccname"] = strAccName;
				cav["cpassword"] = strAccPassword;
				cav["crealname"] = strAccRealName;
				cav["dregdate"] = pAccountCon->GetDate();
				S3PRow rowCav( account.GetTableName(), &cav, pAccountCon );
				if ( account.Add( &rowCav ) <= 0 )
				{
					iRet = 0;
				}
				else
				{
					iRet = 1;
				}
			}
			else
			{
				iRet = -7;
			}
		}
		else
		{
			iRet = -6;
		}
		pAccountCon->Close();
	}
	return iRet;
}