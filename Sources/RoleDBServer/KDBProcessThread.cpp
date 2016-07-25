#include "stdafx.h"
#include <objbase.h>
#include <initguid.h>
#include "KRoleInfomation.h"
#include "KRoleDBServer.h"
#include "KException.h"
#include "S3DBINTERFACE.H"
#include "KDBProcessThread.h"
#include "KROleDBHeader.h"
#include "resource.h"
#include "MainFrm.h"
#include "RoleDBServerDoc.h"
#include "RoleDBServerView.h"




#pragma warning(disable:4786)
#include <crtdbg.h>
#include <list>
#include "stdio.h"
using namespace std;

static const int g_snMaxPlayerCount = 5;
static const int g_snPrecision = 1;
static const int g_snMaxBuffer = 15;
static const int g_snBufferSize = 1024;

IServer *g_pNetServer = NULL;
static HMODULE hDllModule;

DWORD g_dwPerSaveTime;//每次存档的时间
DWORD g_dwMaxUnModifyTime;
list<TCmd*> g_MainThreadCmdList;
list<TGetRoleInfoFromDB*> g_DBLoadThreadCmdList;
list<TCmd*> g_NetServiceThreadCmdList;
extern BOOL InitNetEngine();
extern RoleDBServer::KNetServer g_NetServer;

CRITICAL_SECTION g_RoleInfoMutex;
CRITICAL_SECTION g_RoleInfoListMutex;
CRITICAL_SECTION g_RoleInfoMapMutex;
CRITICAL_SECTION g_MainMsgListMutex;
CRITICAL_SECTION g_NetMsgListMutex;
CRITICAL_SECTION g_GetRoleInfoMutex;
HANDLE			 g_NetEventMutex = 0;


TThreadData g_ThreadData[4];

char * GetRoleNameFromRoleBuffer(char * pRoleBuffer)
{
	if (pRoleBuffer == NULL)
		return NULL;
	try
	{
		TRoleData *pRoleData = (TRoleData*) pRoleBuffer;
		return pRoleData->BaseInfo.szName;
	}
	catch(...)
	{
		throw KException("GetRoleNameFromRoleBuffer", "内存有问题");
	}
}

int InitServer()
{
	int nReturn;
	S3DBI_InitDBInterface();
	nReturn = InitNetEngine();
	InitializeCriticalSection(&g_RoleInfoMutex);
	InitializeCriticalSection(&g_MainMsgListMutex);
	InitializeCriticalSection(&g_NetMsgListMutex);
	InitializeCriticalSection(&g_GetRoleInfoMutex);
	InitializeCriticalSection(&g_RoleInfoListMutex);
	InitializeCriticalSection(&g_RoleInfoMapMutex);
	g_NetEventMutex = CreateMutex(NULL, FALSE, NULL);
	if (!g_NetEventMutex) return 0;
	return nReturn;
}

DWORD ReleaseServer()
{

	for(int i = 0; i < 4; i ++)
		TerminateThread(g_ThreadData[i].hThread,0);

	DeleteCriticalSection(&g_RoleInfoMutex);
	DeleteCriticalSection(&g_MainMsgListMutex);
	DeleteCriticalSection(&g_NetMsgListMutex);
	DeleteCriticalSection(&g_GetRoleInfoMutex);
	DeleteCriticalSection(&g_RoleInfoListMutex);
	DeleteCriticalSection(&g_RoleInfoMapMutex);
	CloseHandle(g_NetEventMutex);

	g_pNetServer->CloseService();
	g_pNetServer->Cleanup();
	g_pNetServer->Release();
	::FreeLibrary( hDllModule );

	S3DBI_ReleaseDBInterface();
	return 1;
}

//
DWORD WINAPI DatabaseSaveThreadFun(void * pParam)
{
/*
while(1)
{
list<KRoleInfomation*>::iterator I = g_RoleInfoList.begin();
while(!I->end())
{
if (I->m_bModify)
{
if (I->CheckRoleInfoValid())
{
S3DBI_SaveRoleInfo()
(*I)->m_nSaveDBCount ++;
(*I)->m_nUnModifyTime = 0;
}
else
{
ASSERT(0);
}

  }
  else
  {
  //长时间未存则从存档列表中删除
  if (I->m_nUnModifyTime >= g_dwMaxUnModifyTime)
  {
  //Question			
  I->erase();
  }
  else
  I->m_nLastSaveTime ++;
  
	}
	I ++;
	}
	
	  Sleep(g_dwPerSaveTime);
	  }
	*/
	return 1;
}

//循环处理向数据库请求获得玩家信息的操作 
DWORD WINAPI DatabaseLoadThreadFun(void * pParam)
{
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();	CRoleDBServerView *pView = (CRoleDBServerView*)pFrame->GetActiveView();
	while(1)
	{
		list<TGetRoleInfoFromDB *>::iterator I = g_DBLoadThreadCmdList.begin();
		pView->UpdateDBServiceList();
		while(I != g_DBLoadThreadCmdList.end())
		{
			static unsigned char szRoleInfo[MAX_ROLEINFORMATION_SIZE];
			int nBufferSize = 0;
			char * szName1 =  (*I)->szRoleName;
			if (!S3DBI_GetRoleInfo(szRoleInfo, (*I)->szRoleName, nBufferSize))	continue;
			
			static int ii = 0;
			char FileName[100] ;
			sprintf(FileName, "d:\\LoadRoleout_%d.txt", ii++);
			TextOutRoleInfo(szRoleInfo ,FileName );
			
			
			//增加角色数据和向两个列表增加数据
			
			EnterCriticalSection(&g_RoleInfoMutex);//同时只能有一个线程修改角色数据
			
			char * szName = (char *)((*I)->szRoleName);
			if (szName)
			{
				KRoleInfomation * pRoleInfomation = 	 g_RoleInfoMap[szName] ;
				
				if (!pRoleInfomation)//新加角色！需要立即存档
				{
					KRoleInfomation * pNewRoleInfomation = new KRoleInfomation;
					
					pNewRoleInfomation->SetRoleInfoBuffer(szRoleInfo, nBufferSize);
					LeaveCriticalSection(&g_RoleInfoMutex);
					
					EnterCriticalSection(&g_RoleInfoMapMutex);
					
					g_RoleInfoMap[szName] = pNewRoleInfomation;
					
					EnterCriticalSection(&g_RoleInfoListMutex);
					g_RoleInfoList.push_back(pNewRoleInfomation);
					
					LeaveCriticalSection(&g_RoleInfoMapMutex);
					LeaveCriticalSection(&g_RoleInfoListMutex);
					
				}
				
				
				TCmd * pNewCmd		= (TCmd *)new char [sizeof(TCmd)  + strlen(szName)];
				pNewCmd->ulNetId		= (*I)->nNetId;
				pNewCmd->nProtoId	= PROTOL_LOADROLE;
				pNewCmd->nDataLen	= strlen(szName) + 1;
				strcpy((char *)&pNewCmd->pDataBuffer[0], szName);
				
				EnterCriticalSection(&g_MainMsgListMutex);
				g_MainThreadCmdList.push_back(pNewCmd);
				LeaveCriticalSection(&g_MainMsgListMutex);
				
				/*
				//要求服务列表增加一个读档服务
				if (I->nNetId != 0)
				{
				unsigned char * pBuffer = new unsigned char [nBufferSize + sizeof(TCmd) - 1];
				TCmd * pCmd = (TCmd *)pBuffer;
				pCmd->nNetId = I->nNetId ;
				pCmd->Size = nBufferSize;
				pCmd->Cmd = PROTOL_LOADROLE;
				memcpy(&pCmd->pDataBuffer[0], szRoleInfo, )
				}
				WaitForSingleObject();//同时只能有一个线程修改网络服务列表的数据
				*/
				
			}
			else
				LeaveCriticalSection(&g_RoleInfoMutex);
			
			//	delete *I;
			g_DBLoadThreadCmdList.erase(I);
			
			I = g_DBLoadThreadCmdList.begin();
			pView->UpdateDBServiceList();
		}
		Sleep(10);
	}
	return 1;
}

//主循环，负责处理网络端发来的完整的请求数据
DWORD 	WINAPI RoleServerMainThreadFun(void * pParam)
{
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();	CRoleDBServerView *pView = (CRoleDBServerView*)pFrame->GetActiveView();
	
	while(1)
	{
		pView->UpdateMainLoopList();
		list<TCmd *>::iterator I = g_MainThreadCmdList.begin();
		
		while (I != g_MainThreadCmdList.end())
		{
			switch((*I)->nProtoId)
			{
				//Save
			case PROTOL_ONLYSAVEROLE://游戏服务器向数据库服务器要求保存数据
			case PROTOL_SAVEORCREATEROLE:
				{
					EnterCriticalSection(&g_RoleInfoMutex);
					const char * szName = GetRoleNameFromRoleBuffer((char *)&(*I)->pDataBuffer[0]);
					
					static ii = 0;
					char FileName[100] ;
					sprintf(FileName, "d:\\SaveRoleout_%d.txt", ii++);
					TextOutRoleInfo((unsigned char *)&(*I)->pDataBuffer[0],FileName );
					
					if (szName)
					{
						map<std::string , KRoleInfomation * >::iterator II;
						II =  g_RoleInfoMap.find((const char *)szName) ;
						KRoleInfomation * pRoleInfomation = II->second;
						
						if (!pRoleInfomation)//新加角色！需要立即存档
						{
							KRoleInfomation * pNewRoleInfomation = new KRoleInfomation;
							
							pRoleInfomation->SetRoleInfoBuffer((unsigned char *)&((*I)->pDataBuffer[0]), (*I)->nDataLen);
							
							EnterCriticalSection(&g_RoleInfoMapMutex);
							
							g_RoleInfoMap[szName] = pNewRoleInfomation;
							
							EnterCriticalSection(&g_RoleInfoListMutex);
							g_RoleInfoList.push_back(pNewRoleInfomation);
							
							LeaveCriticalSection(&g_RoleInfoMapMutex);
							LeaveCriticalSection(&g_RoleInfoListMutex);
							
						}
						else
						{
							pRoleInfomation->SetRoleInfoBuffer((unsigned char *)&((*I)->pDataBuffer), (*I)->nDataLen);
						}
					}
					LeaveCriticalSection(&g_RoleInfoMutex);
				}
				break;
				
				//Load
			case PROTOL_LOADROLE://获得角色数据
				{
					char szName[100] ;
					memcpy(szName, (*I)->pDataBuffer, (*I)->nDataLen);// GetRoleNameFromRoleBuffer((char *)&(*I)->pDataBuffer);
					szName[(*I)->nDataLen] = 0;
					if (szName)
					{
						KRoleInfomation * pRoleInfomation = g_RoleInfoMap[szName];
						
						/*map<const char * , KRoleInfomation * >::iterator II;
						II =  g_RoleInfoMap.find((const char *)szName) ;
						KRoleInfomation * pRoleInfomation = II->second;
						*/
						if (pRoleInfomation )
						{
							size_t BufferSize = 0;
							BYTE * pBuffer = pRoleInfomation->GetRoleInfoBuffer(BufferSize);
							_ASSERT(BufferSize && pBuffer);
							BYTE * pNewBuffer = new BYTE[sizeof(TCmd) + BufferSize - 1];
							TCmd * pNewCmd = (TCmd*)pNewBuffer;
							pNewCmd->nProtoId = PROTOL_ROLEINFO;
							pNewCmd->nDataLen = BufferSize;
							pNewCmd->ulNetId = (*I)->ulNetId;
							memcpy(pNewCmd->pDataBuffer, pBuffer, BufferSize);
							
							EnterCriticalSection(&g_NetMsgListMutex);
							g_NetServiceThreadCmdList.push_back(pNewCmd);
							LeaveCriticalSection(&g_NetMsgListMutex);
						}
						else//没有则需要实际的取数据，需要向LoadThread发送消息
						{
							TGetRoleInfoFromDB * pNewCmd = new TGetRoleInfoFromDB;
							pNewCmd->nNetId = (*I)->ulNetId;
							strcpy(pNewCmd->szRoleName, szName);
							EnterCriticalSection(&g_GetRoleInfoMutex);
							g_DBLoadThreadCmdList.push_back(pNewCmd);
							LeaveCriticalSection(&g_GetRoleInfoMutex);
						}
					}
					
				}break;
				
				//LoadRoleList
			case PROTOL_GETROLELIST://获得某个帐号的角色列表
				{
					unsigned char  MaxRoleCount = (int)(*I)->pDataBuffer[0];
					unsigned char * pNewBuffer = new  unsigned char [sizeof(S3DBI_RoleBaseInfo) * MaxRoleCount + 1];//列表和1个
					TCmd * pCmd = (TCmd*)pNewBuffer;
					pCmd->ulNetId = (*I)->ulNetId;
					pCmd->nProtoId	=  (*I)->nProtoId;
					pCmd->nDataLen  =  sizeof(S3DBI_RoleBaseInfo) * MaxRoleCount + 1;
					
					S3DBI_RoleBaseInfo *pRoleBaseInfoList  =(S3DBI_RoleBaseInfo*) ((char*)&pCmd->pDataBuffer[0] + 1);
					char  szAccountName[32];
					memcpy(szAccountName, &(*I)->pDataBuffer[0] + 1, (*I)->nDataLen - 1 );
					szAccountName[(*I)->nDataLen - 1] = 0;

					pCmd->pDataBuffer[0]  = (unsigned char )S3DBI_GetRoleListOfAccount(szAccountName, pRoleBaseInfoList, MaxRoleCount);

					for (int i  = 0; i < pCmd->pDataBuffer[0]; i ++)
					{
						KRoleInfomation * pRoleInfo = g_RoleInfoMap[pRoleBaseInfoList->szRoleName];
						if (pRoleInfo)
						{
							size_t nSize = 0;
							TRoleData * pRoleData = (TRoleData*)pRoleInfo->GetRoleInfoBuffer(nSize);
							if (pRoleData && nSize)
							{
								pRoleBaseInfoList->nArmorType = pRoleData->BaseInfo.iarmorres;
								pRoleBaseInfoList->nHelmType  = pRoleData->BaseInfo.ihelmres;
								pRoleBaseInfoList->nLevel     = pRoleData->BaseInfo.ifightlevel;
								pRoleBaseInfoList->nSex		  = pRoleData->BaseInfo.bSex;
								pRoleBaseInfoList->nWeaponType = pRoleData->BaseInfo.iweaponres;
							}
						}
					}

					EnterCriticalSection(&g_NetMsgListMutex);
					g_NetServiceThreadCmdList.push_back(pCmd);
					LeaveCriticalSection(&g_NetMsgListMutex);
				}
				break;
			case PROTOL_DELETEROLE:
				{
					
				}break;
				//Exit
				
				
			}
			
			g_MainThreadCmdList.erase(I);
			I = g_MainThreadCmdList.begin();
			pView->UpdateMainLoopList();
		}
		Sleep(20);
		
	}
	return 1;
}


//	
DWORD WINAPI 	RoleNetWorkServiceThreadFun(void * pParam)
{
	g_NetServer.Init();
	//g_NetServer.RegisterClient(1);
	while(1)
	{
		g_NetServer.Receive();
		g_NetServer.Send();
		Sleep(10);
	}
}
TRoleNetMsg  * pTestCmd = NULL;
void * EnumSendMsg(unsigned long nId, size_t& nDataLen)
{
	if (pTestCmd)
	{
		nDataLen = pTestCmd->nDataLen;
		return  pTestCmd;
	}
	else 
	{
		nDataLen = 0;
		return NULL;
	}
	
}

FILE *streamout;
FILE *streamin;

void SendToClient(unsigned long nId, TRoleNetMsg * pMsg, size_t nMsgLen)
{
	static i = 0;
	char FileName[100] ;
	sprintf(FileName, "d:\\testout_%d.txt", i++);
	if( (streamout = fopen( FileName, "wb" )) != NULL )
	{
		fwrite(pMsg, 1, nMsgLen, streamout);
	}
	fclose(streamout);
}

void TextOutRoleInfo(BYTE * pDataBuffer, char * FileName)
{
	if (!pDataBuffer) return ;
	char showString[50000];
	S3DBI_ShowDebugInfo(pDataBuffer, showString);
	FILE * streamout;
	if( (streamout = fopen( FileName, "wb" )) != NULL )
	{
		fwrite(showString, 1, strlen(showString), streamout);
	}
	fclose(streamout);
}



typedef HRESULT ( __stdcall * pfnCreateServerInterface )(
			REFIID	riid,
			void	**ppv
		);

void __stdcall ServerEventNotify(
			LPVOID lpParam,
			const unsigned long &ulnID,
			const unsigned long &ulnEventType )
{
	CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();	CRoleDBServerView *pView = (CRoleDBServerView*)pFrame->GetActiveView();
	switch( ulnEventType )
	{
	case enumClientConnectCreate:
		WaitForSingleObject(g_NetEventMutex, INFINITE);
		g_NetServer.RegisterClient(ulnID);
		ReleaseMutex(g_NetEventMutex);
		break;
	case enumClientConnectClose:
		WaitForSingleObject(g_NetEventMutex, INFINITE);
		g_NetServer.DestoryClient(ulnID);
		ReleaseMutex(g_NetEventMutex);
		break;
	}
	pView->UpdateRoleServerList();
	
}

unsigned _stdcall ThreadFunction( void *pParam )
{
	IServer *pServer = reinterpret_cast< IServer * >( pParam );

	while ( pServer )
	{
		Sleep( 1 );
		
		size_t datalength = 0;

		const void *pData = pServer->GetPackFromClient( 4, datalength );

		if ( !pData || 0 == datalength )
		{
			continue;
		}

	}

	if ( pServer )
	{
		pServer->Release();
		pServer = NULL;
	}

	return 0L;
}

BOOL InitNetEngine()
{

	hDllModule = ::LoadLibrary( "heaven.dll" );

	if ( hDllModule )
	{
		pfnCreateServerInterface pFactroyFun = ( pfnCreateServerInterface )GetProcAddress( hDllModule, "CreateInterface" );

		IServerFactory *pServerFactory = NULL;

		if ( SUCCEEDED( pFactroyFun( IID_IServerFactory, reinterpret_cast< void ** >( &pServerFactory ) ) ) )
		{
			pServerFactory->SetEnvironment( g_snMaxPlayerCount, g_snPrecision, g_snMaxBuffer, g_snBufferSize  );

			pServerFactory->CreateServerInterface( IID_IIOCPServer, reinterpret_cast< void ** >( &g_pNetServer ) );

			pServerFactory->Release();
		}
	}
	else
		return FALSE;

	g_pNetServer->Startup();

	g_pNetServer->RegisterMsgFilter( reinterpret_cast< void * >( g_pNetServer ), ServerEventNotify );

	g_pNetServer->OpenService( 0, 5001 );


	//unsigned int threadid = 0;
	//HANDLE handle = (HANDLE)::_beginthreadex( 0, 0, ThreadFunction, reinterpret_cast< void * >( pClonServer ), 0, &threadid );

	//CloseHandle( handle );
	
/*	const size_t len = strlen( "to client" );

	while ( pServer )
	{
		pServer->PreparePackSink();

		pServer->PackDataToClient( 4, "to client", len );

		pServer->SendPackToClient();
		
		Sleep( 100 );
	}
	*/
	return 1;
}
