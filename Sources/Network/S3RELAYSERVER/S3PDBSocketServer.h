//-----------------------------------------//
//                                         //
//  File		: S3PDBSocketServer.h	   //
//	Author		: Yang Xiaodong            //
//	Modified	: 8/26/2002                //
//                                         //
//-----------------------------------------//
#ifndef _S3PDBSOCKETSERVER_H_
#define _S3PDBSOCKETSERVER_H_

#include "KStdAfx.h"
#include "S3PDBSocketPool.h"

typedef struct tag_DBSOCKETSERVERPARAM
{
	SOCKET serverSocket;
	int* piRunSignal;
	S3PDBSocketPool* pSocketPool;
}_DBSOCKETSERVERPARAM, *_LPDBSOCKETSERVERPARAM;

class S3PDBSocketServer  
{
public:
	static SOCKET CreateSocket( int iPort );
	static BOOL SendUDP( SOCKET s,
		DWORD dwTargetIP,
		int iTargetPort,
		IBYTE buf[def_UDPSIZE],
		DWORD dwSize );
public:
	S3PDBSocketServer( SOCKET s, S3PDBSocketPool* pPool = NULL );
	S3PDBSocketServer( int iPort, S3PDBSocketPool* pPool = NULL );
	virtual ~S3PDBSocketServer();

	virtual HANDLE Start();
	virtual BOOL Stop();

protected:
	static DWORD WINAPI StartServer( LPVOID lpParam );
	virtual void CreateEnablePoolEvent();
	virtual BOOL ReleaseSocket();
	virtual BOOL CreateSocket();

	int* m_piRunSignal;
	int m_iPort;
	SOCKET m_Socket;
	HANDLE m_hServer;
	DWORD m_dwServerThreadId;
	S3PDBSocketPool* m_pSocketPool;
	_DBSOCKETSERVERPARAM m_ServerParam;
	HANDLE m_hEnablePool;
};

#endif	// _S3PDBSOCKETSERVER_H_