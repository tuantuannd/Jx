// Global.h: interface for the CGlobal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBAL_H__96341752_5601_43CD_8D7B_F05C99266EA2__INCLUDED_)
#define AFX_GLOBAL_H__96341752_5601_43CD_8D7B_F05C99266EA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////


#include <vector>


/////////////////////////////////////////////


#define DUMMY_AUTOLOCKREAD(lock)			0
#define DUMMY_AUTOLOCKREAD_(name, lock)		0

#define DUMMY_AUTOLOCKWRITE(lock)			0
#define DUMMY_AUTOLOCKWRITE_(name, lock)	0


/////////////////////////////////////////////





#include "../../../headers/ServerPort.h"

#include "../../../headers/KProtocol.h"
#include "../../../headers/KProtocolDef.h"
#include "../../../headers/KGmProtocol.h"
#include "../../../headers/KRelayProtocol.h"
#include "../../../headers/KTongProtocol.h"


#include "Memory.h"

#include "HeavenLib.h"
#include "RainbowLib.h"

#include "SockThread.h"

#include "RootCenter.h"
#include "RootClient.h"
#include "RelayCenter.h"
#include "RelayServer.h"
#include "GatewayCenter.h"
#include "GatewayClient.h"
#include "HostServer.h"
#include "TongServer.h"
#include "ChatServer.h"
#include "ChannelMgr.h"
#include "FriendMgr.h"
#include "DealRelay.h"
#include "DBConnect.h"
#include "KTongSet.h"
#include "TongDB.h"




/////////////////////////////////////////////////


const size_t max_packagesize = 1024 - 32;
const DWORD breathe_interval = 1;


const int local_adapt = 0;
const int global_adapt = 1;





/////////////////////////////////////////////////

extern CMemory g_memory;

extern CHeavenLib g_libHeaven;
extern CRainbowLib g_libRainbow;


extern CSockThread g_threadSock;


extern CRelayCenter g_RelayCenter;
extern CRelayServer g_RelayServer;

extern CRootCenter g_RootCenter;
extern CRootClient g_RootClient;

extern CGatewayCenter g_GatewayCenter;
extern CGatewayClient g_GatewayClient;

extern CHostServer g_HostServer;
extern CTongServer g_TongServer;
extern CChatServer g_ChatServer;

extern CChannelMgr g_ChannelMgr;
extern CFriendMgr g_FriendMgr;

extern CDBCenter g_DBCenter;
extern CDBClient g_DBClient;

extern CTongSet g_cTongSet;

extern CTongDB		g_cTongDB;

/////////////////////////////////////////////////


#ifndef MAX_PLAYER_PER_ACCOUNT
#define MAX_PLAYER_PER_ACCOUNT 3
#endif

/////////////////////////////////////////////////

std::_tstring gGetLocateDirectory();

std::_tstring gGetPrivateProfileStringEx(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpFileName, LPCSTR lpDefault = _T(""));
INT gGetPrivateProfileIntEx(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpFileName, INT nDefault = -1);
TCHAR gGetPrivateProfileCharEx(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpFileName, TCHAR chDefault = 0);
const std::vector<std::_tstring>& gGetPrivateProfileSectionEx(LPCTSTR lpAppName, LPCTSTR lpFileName);
const std::vector<std::_tstring>& gGetPrivateProfileSectionNamesEx(LPCTSTR lpFileName);


BOOL gInitialize();
BOOL gUninitialize();

BOOL gIsLocalIP(DWORD ip);

const UCHAR* gGetHostMac(int adapt, size_t* pSize = NULL);
DWORD gGetHostIP(int adapt);

BOOL gConnectToSibling(const char* addr);

size_t gFillClntwayPckg(DWORD nameid, unsigned long param, 
						const RELAY_ASKWAY_DATA* pAskWay, size_t sizeAskWay, RELAY_ASKWAY_DATA* pClntWay, size_t sizeClntWay);
size_t gFillLosewayPckg(DWORD nToIP, DWORD nToRelayID, 
					   const void* pLosePckg, size_t sizeLosePckg, void* pLoseway, size_t sizeLoseway);

BOOL gIsLegalString(LPCTSTR sz, size_t lenlolmt, size_t lenhilmt);

extern DWORD g_String2Id(LPCSTR pString);




enum ONECLIENT {oneclient_root, oneclient_gateway, oneclient_db};

BOOL gOnOneClientCreate(ONECLIENT ct);
BOOL gOnOneClientClose(ONECLIENT ct);
BOOL gReOneClient(ONECLIENT ct);


BOOL gConsignClientSD(IClient* pClient);


BOOL gStartFriendTimingUpdateDB(UINT elapse);
BOOL gStopFriendTimingUpdateDB();
BOOL gFriendTimingUpdateDB();


#endif // !defined(AFX_GLOBAL_H__96341752_5601_43CD_8D7B_F05C99266EA2__INCLUDED_)
