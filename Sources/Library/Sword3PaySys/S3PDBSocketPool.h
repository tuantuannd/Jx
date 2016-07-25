//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketPool.h		   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBSOCKETPOOL_H_
#define _S3PDBSOCKETPOOL_H_

#include "KStdAfx.h"
#include "LoginDef.h"
#include "S3PAccount.h"
#include "S3PList.h"
#include "S3PDBSocketParser.h"
#include <list>
using namespace std;

class S3PDBSocketPool  
{
// Static members
//------>BEGIN
public:
	typedef struct tag_DBSOCKETPOOLDATAPACKET
	{
		DWORD dwFromIP;
		int iFromPort;
		DWORD dwSize;
		IBYTE dataBuf[def_UDPSIZE];
	}_DBSOCKETPOOLDATAPACKET, *_LPDBSOCKETPOOLDATAPACKET;
	typedef list<_DBSOCKETPOOLDATAPACKET> DBSPOOL;

	typedef struct tag_DBSOCKETPOOLPARAM
	{
		int* piRunSignal;
		HANDLE hEnableEvent;
		S3PDBSocketPool* pPool;
	}_DBSOCKETPOOLPARAM, *_LPDBSOCKETPOOLPARAM;
	S3PDBSocketPool();
	virtual ~S3PDBSocketPool();

	virtual BOOL Stop();
	virtual HANDLE Start();
	virtual void Init( SOCKET s, HANDLE hEnableEvent );

	virtual void PushDataPacket( _LPDBSOCKETPOOLDATAPACKET lpData );
	virtual BOOL PopDataPacket( _DBSOCKETPOOLDATAPACKET& data );
	virtual BOOL Process( _LPDBSOCKETPOOLDATAPACKET lpData );

protected:
	static BOOL m_bAccDBIsLocked;
	S3PList m_UDPs;
	HANDLE m_hEnable;
	SOCKET m_Socket;
	int* m_piRunSignal;
	HANDLE m_hProcessor;
	DWORD m_dwProcessorThreadId;
	_DBSOCKETPOOLPARAM m_PoolParam;

	virtual BOOL SendUDP( DWORD dwTargetIP,
		int iTargetPort,
		IBYTE* pBuf,
		DWORD dwSize );
	virtual BOOL SendError( DWORD dwTargetIP, int iTargetPort );
	virtual BOOL Distribute( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProMsgWhenLocked( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProAddAccount( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProLoginFromGame( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProLogoutFromGame( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProReportFromGame( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProQueryGameserverList( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProBeginGame( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBLogin( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBLogout( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBQueryUserList( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBLock( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBActivate( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBAddUser( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBDeleteUser( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
	virtual BOOL ProDBCreateAccount( DWORD dwTargetIP,
		int iTargetPort,
		_LPCPARSEDDATAPACKET lpcParsedData );
};

#endif	// _S3PDBSOCKETPOOL_H_