// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTSOCKET_H__C524C6B2_0AA8_4E6C_9FA0_09F316FCA61E__INCLUDED_)
#define AFX_CLIENTSOCKET_H__C524C6B2_0AA8_4E6C_9FA0_09F316FCA61E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <winsock2.h>
#include "ESClient\Exception.h"
#include "ESClient\Utils.h"
#include "ESClient\ManualResetEvent.h"
#include "GameClient.h"
//[ Include in .\..\Protocol
#include "Protocol\Protocol.h"

class S3PDBSocketParser;
class CClientSocket  
{
public:
	typedef struct tag_SERVERPARAM
	{
		SOCKET s;
		CGameClient* pS;
	}_SERVERPARAM, *_PSERVERPARAM;
	typedef struct tag_RECVPARAM
	{
		SOCKET s;
		CGameClient* pS;
		IBYTE* pRecvBuf;
		int iRecvBufSize;
		struct sockaddr_in from;
		int fromlen;
		int iResult;
	}_RECVPARAM, *_PRECVPARAM;
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
	enum { DBLOGINSTRUCT_SIZE = KLOGINSTRUCTHEADSIZE + 32 };
	typedef DWORD ( CALLBACK *LOGINCALLBACK )( DWORD dwLoginResult, DWORD dwCustomParameter );
	static CClientSocket* Instance( short* psiPort = NULL );
	static CClientSocket* GetInstance();
	static void ReleaseInstance();
	static BOOL IncludeSomeBitValue( DWORD dwMatrix, DWORD dwSomeBitValue );
protected:
	static CClientSocket* m_pInstance;

public:
	virtual ~CClientSocket();

	BOOL Start();
	BOOL Stop();
	BOOL SetServerAddr( LPCTSTR lpszIPAddr, short siPort );
	BOOL SetServerAddr( DWORD dwIPAddr, short siPort );
	BOOL RemoteQuery( _PDBLOGINSTRUCT pParam,
		CClientSocket::LOGINCALLBACK LoginCallbackFun,
		DWORD dwCustomParam,
		DWORD dwMilliseconds );
	BOOL Connect( LPCTSTR lpszAddr, short siPort );
protected:
	CClientSocket();

	BOOL InitWinSocket();
	BOOL UninitWinSocket();
	BOOL HasValidSocket();
	BOOL CreateSocket( short siPort );
	BOOL ReleaseSocket();

	DWORD Recvfrom( char FAR* buf, int len, DWORD dwMilliseconds );

	SOCKET m_s;	// Not used
	CGameClient* m_pS;
	_SERVERPARAM m_serverParam;
	_RECVPARAM m_recvParam;
	HANDLE m_hServer;
	struct sockaddr_in m_serverAddr;
};

#endif // !defined(AFX_CLIENTSOCKET_H__C524C6B2_0AA8_4E6C_9FA0_09F316FCA61E__INCLUDED_)
