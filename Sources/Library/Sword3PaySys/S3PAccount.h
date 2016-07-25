//-----------------------------------------//
//                                         //
//  File		: S3PAccount.h			   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/29/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_)
#define AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_

#include "KStdAfx.h"
#include "S3PDBConnection.h"
#include "S3PDBConnector.h"
#include "S3PAccountInfoDAO.h"
#include "S3PAccountHabitusDAO.h"
#include "S3PAccCardHistoryDAO.h"
#include "S3PServerListDAO.h"
#include "S3PAccountUser.h"
#include <string>

class S3PAccount  
{
public:
	typedef struct tag_USERINFO
	{
		char szHostAddr[16];
		char szUserName[def_DBUSERNAME_MAX_LEN+1];
		char szPassword[def_DBPASSWORD_MAX_LEN+1];
		short siPriority;
		BOOL bLoggedin;
		char szLastLoginTime[20];
		char szLastLogoutTime[20];
	}_USERINFO, *_PUSERINFO;
	S3PAccount();
	virtual ~S3PAccount();

	static BOOL ActivateAccount( const std::string strAccName,
		const std::string strPassword,
		const DWORD dwGameID );
	static int AddAccount( const std::string strAccName,
		const std::string strPassword,
		const std::string strRealName,
		const std::string strIDNum = "",
		const std::string strBirthDay = "",
		const std::string strArea = "",
		const std::string strPhone = "" );
	static int Login( const std::string strAccName/* Input */,
		const std::string strPassword/* Input */,
		int iGameId = 0 );
	static int Logout( const std::string strAccName, const std::string strPassword, int iGameId );
	static int Report( const std::string strAccName, const std::string strPassword, int iGameId );
	static int QueryGameserverList( const std::string strAccName/* Input */,
		const std::string strPassword/* Input */,
		IBYTE* pServers/* Output */,
		DWORD& dwSize/* Input/Output */ );

	static int DBLogin( const std::string strUserName,
		const std::string strPassword,
		const std::string strHostAddr );
	static int DBLogout( const std::string strUserName,
		const std::string strPassword );
	static int DBQueryUserList( const std::string strUserName,
		const std::string strPassword,
		IBYTE* pUsers/* Input/Output */,
		DWORD& dwSize/* Input/Output */ );
	static int DBLock( const std::string strUserName,
		const std::string strPassword );
	static int DBActivate( const std::string strUserName,
		const std::string strPassword );
	static int DBAddUser( const std::string strUserName,
		const std::string strPassword,
		const std::string strNewUserName,
		const std::string strNewUserPassword,
		short int siNewUserPriority );
	static int DBDeleteUser( const std::string strUserName,
		const std::string strPassword,
		const std::string strSelUserName );
	static int DBCreateAccount( const std::string strUserName,
		const std::string strPassword,
		const std::string strAccRealName,
		const std::string strAccPassword,
		const std::string strAccName );
protected:
	BOOL m_bLocked;	// 用于标识数据库是否被管理员锁住停止对公共用户的访问请求提供服务
};

#endif // !defined(AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_)
