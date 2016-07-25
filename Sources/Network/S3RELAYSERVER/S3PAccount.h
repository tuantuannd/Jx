//-----------------------------------------//
//                                         //
//  File		: S3PAccount.h			   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/29/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_)
#define AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_

#include "GlobalDTD.h"
#include <string>
#include <list>
#include "../S3AccServer/AccountLoginDef.h"

class S3PDBConVBC;

using namespace std;
typedef std::list<KAccountUserTimeInfo> AccountTimeList;

class S3PAccount  
{
public:
	S3PAccount();
	virtual ~S3PAccount();

	static int ServerLogin(S3PDBConVBC* pConn, const char* strAccName, const char* strPassword, const DWORD Address, const short Port, const BYTE Mac[6], DWORD& nGameID);
	static int ServerLogout(S3PDBConVBC* pConn, DWORD ClientID, BOOL bElapse);
	static int CheckAddress(S3PDBConVBC* pConn, const DWORD Address, const short Port);
	static int GetAccountCount(S3PDBConVBC* pConn, DWORD nGameID, BOOL bOnline, DWORD& dwCount);
	static int GetAccountGameID(S3PDBConVBC* pConn, const char* strAccName, DWORD& ClientID);
	static int UnlockAccount(S3PDBConVBC* pConn, const char* strAccName);
	static int UnlockServer(S3PDBConVBC* pConn, unsigned long nGameID);
	static int GetAccountsTime(S3PDBConVBC* pConn, DWORD ClientID, DWORD dwMinSecond, AccountTimeList& List);
	static int GetServerID(S3PDBConVBC* pConn, const char* strAccName, unsigned long& nGameID);
	static int FreezeAccount(S3PDBConVBC* pConn, const char* strAccName);

protected:
};

#endif // !defined(AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_)
