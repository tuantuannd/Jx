//-----------------------------------------//
//                                         //
//  File		: S3PAccount.h			   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/29/2002                //
//                                         //
//-----------------------------------------//

#if !defined(AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_)
#define AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_

#include "AccountLogindef.h"
#include "GlobalDTD.h"
#include <string>
#include <list>

class S3PDBConVBC;

using namespace std;
typedef std::list<KAccountUserTimeInfo> AccountTimeList;

class S3PAccount  
{
public:
	S3PAccount();
	virtual ~S3PAccount();

	static int Login(S3PDBConVBC* pConn, const char* strAccName, const char* strPassword, DWORD ClientID, WORD& nExtPoint, DWORD& nLeftTime);
	static int LoginGame(S3PDBConVBC* pConn, DWORD ClientID, const char* strAccName);
	static int Logout(S3PDBConVBC* pConn, DWORD ClientID, const char* strAccName, WORD nExtPoint);
	static int ElapseTime(S3PDBConVBC* pConn, DWORD ClientID, const char* strAccName, const DWORD dwDecSecond);
	static int QueryTime(S3PDBConVBC* pConn, DWORD ClientID, const char* strAccName, DWORD& dwSecond);
	static int ServerLogin(S3PDBConVBC* pConn, const char* strAccName, const char* strPassword, const DWORD Address, const short Port, const BYTE Mac[6], unsigned long& nGameID);
	static int UnlockAll(S3PDBConVBC* pConn, DWORD ClientID);
	static int ElapseAll(S3PDBConVBC* pConn, DWORD ClientID);
	static int CheckAddress(S3PDBConVBC* pConn, const DWORD Address, const short Port);
	static int GetAccountGameID(S3PDBConVBC* pConn, const char* strAccName, DWORD& ClientID);
	static int VerifyUserModifyPassword(S3PDBConVBC* pConn, DWORD ClientID, const char* strAccName, const char* strPassword);
	static int GetServerID(S3PDBConVBC* pConn, const char* strAccName, unsigned long& nGameID);

protected:
	static int GetLeftSecondsOfDeposit(S3PDBConVBC* pConn, const char* strAccName, long& liLeft, long& liExp);
};

#endif // !defined(AFX_S3PACCOUNT_H__7222E304_3E04_44D2_A3D1_E277BEF07235__INCLUDED_)
