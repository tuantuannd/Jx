// Global.cpp: implementation of the CGlobal class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "S3Relay.h"

#include <map>

#include "../../../Headers/inoutmac.h"

#include "DoScript.h"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////



const int _localmask = 0x0000FFFF;
const int _localtag = 0x0000A8C0;


const DWORD delay_reclient = 30000;


////////////
//variants


CMemory g_memory;

CHeavenLib g_libHeaven;
CRainbowLib g_libRainbow;


CSockThread g_threadSock;


CRelayCenter g_RelayCenter;
CRelayServer g_RelayServer;

CRootCenter g_RootCenter;
CRootClient g_RootClient(&g_RootCenter, FALSE);

CDBCenter g_DBCenter;
CDBClient g_DBClient(&g_DBCenter, FALSE);

CGatewayCenter g_GatewayCenter;
CGatewayClient g_GatewayClient(&g_GatewayCenter, FALSE);

CHostServer g_HostServer;
CTongServer g_TongServer;
CChatServer g_ChatServer;

CChannelMgr g_ChannelMgr;
CFriendMgr g_FriendMgr;

CTongSet g_cTongSet;



//////////////////////////////////////////////////


const char file_relaycfg[] = "relay_config.ini";


const char sec_gateway[] = "gateway";
const char sec_relay[] = "relay";
const char sec_root[] = "root";
const char sec_host[] = "host";
const char sec_tong[] = "tong";
const char sec_chat[] = "chat";
const char sec_dbrole[] = "dbrole";


const char key_ip[] = "ip";
const char key_address[] = "address";
const char key_playercnt[] = "playercnt";
const char key_precision[] = "precision";
const char key_freebuffer[] = "freebuffer";
const char key_buffersize[] = "buffersize";
const char key_account[] = "account";
const char key_password[] = "password";
const char key_retryinterval[] = "retryinterval";



/////////////////////////////////////////////////
static UINT gs_reclient_enable = FALSE;
//static gs_reclient_enable = FALSE;

static UINT gs_timerRoot = 0;
static UINT gs_timerGateway = 0;
static UINT gs_timerDB = 0;
static UINT gs_timerFriendUdtDB = 0;



/////////////////////////////////////////////////


HWND gGetMainWnd();
HWND gGetListBox();


/////////////////////////////////////////////////
// functions

std::_tstring gGetLocateDirectory()
{
	static struct _LOCALDIR
	{
		std::_tstring path;

		_LOCALDIR()
		{
			TCHAR buff[_MAX_PATH] = {0};
			::GetModuleFileName(NULL, buff, _MAX_PATH);
			TCHAR* pCh = _tcsrchr(buff, _T('\\'));
			assert(pCh);
			*(pCh + 1) = 0;
			path = buff;

			assert(!path.empty());
		}
	} s_locdir;

	return s_locdir.path;
}


std::_tstring gGetPrivateProfileStringEx(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpFileName, LPCSTR lpDefault)
{
	std::_tstring path = gGetLocateDirectory() + lpFileName;

	TCHAR buffer[_MAX_PATH] = {0};
	::GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, buffer, _MAX_PATH, path.c_str());
	return buffer;
}
INT gGetPrivateProfileIntEx(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpFileName, INT nDefault)
{
	std::_tstring path = gGetLocateDirectory() + lpFileName;

	return ::GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, path.c_str());
}

TCHAR gGetPrivateProfileCharEx(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpFileName, TCHAR chDefault)
{
	std::_tstring theStr = ::gGetPrivateProfileStringEx(lpAppName, lpKeyName, lpFileName);

	return theStr.empty() ? chDefault : theStr[0];
}

const std::vector<std::_tstring>& gGetPrivateProfileSectionEx(LPCTSTR lpAppName, LPCTSTR lpFileName)
{
	std::_tstring path = gGetLocateDirectory() + lpFileName;

	
	static std::vector<std::_tstring> s_vecstr;
	s_vecstr.clear();

	const DWORD MAX_BUFFSIZE = 4096;
	char buff[MAX_BUFFSIZE];
	DWORD cch = ::GetPrivateProfileSection(lpAppName, buff, MAX_BUFFSIZE, path.c_str());

	for (char* pc = buff; *pc != 0; pc += strlen(pc))
		s_vecstr.push_back(pc);

	return s_vecstr;
}

const std::vector<std::_tstring>& gGetPrivateProfileSectionNamesEx(LPCTSTR lpFileName)
{
	std::_tstring path = gGetLocateDirectory() + lpFileName;

	
	static std::vector<std::_tstring> s_vecstr;
	s_vecstr.clear();

	const DWORD MAX_BUFFSIZE = 4096;
	char buff[MAX_BUFFSIZE];
	DWORD cch = ::GetPrivateProfileSectionNames(buff, MAX_BUFFSIZE, path.c_str());

	for (char* pc = buff; *pc != 0; pc += s_vecstr.back().size())
		s_vecstr.push_back(pc);

	return s_vecstr;
}




BOOL gInitialize()
{
	InitScript();
	
	gs_reclient_enable = TRUE;


	//check the netcard, need 1 at least
	if (gGetMacAndIPAddress(NULL, NULL, NULL, NULL, _localtag, 0) <= 0)
		return FALSE;


	if (!g_memory.Initialize())
		return FALSE;


	if (!g_libHeaven.Initialize())
		return FALSE;
	if (!g_libRainbow.Initialize())
		return FALSE;


	if (!g_ChannelMgr.Initialize())
		return FALSE;
	if (!g_FriendMgr.Initialize())
		return FALSE;


	const size_t playercntRelay = gGetPrivateProfileIntEx(sec_relay, key_playercnt, file_relaycfg, 0);
	const size_t precisionRelay = gGetPrivateProfileIntEx(sec_relay, key_precision, file_relaycfg, 0);
	const size_t freebufferRelay = gGetPrivateProfileIntEx(sec_relay, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeRelay = gGetPrivateProfileIntEx(sec_relay, key_buffersize, file_relaycfg, 0);
	const DWORD listenaddrRelay = INADDR_ANY;
	const unsigned short listenportRelay = RELAY_SIBLING_PORT;

	if (!g_RelayCenter.Initialize())
		return FALSE;
	if (!g_RelayServer.Startup(playercntRelay, precisionRelay, freebufferRelay, buffersizeRelay, listenaddrRelay, listenportRelay))
	{
		rTRACE("FAIL: relay server");
		//return FALSE;
	}


	const size_t freebufferRoot = gGetPrivateProfileIntEx(sec_root, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeRoot = gGetPrivateProfileIntEx(sec_root, key_buffersize, file_relaycfg, 0);
	std::_tstring addrRoot = gGetPrivateProfileStringEx(sec_root, key_address, file_relaycfg);
	const unsigned short portRoot = RELAY_ROUTE_PORT;

	if (!g_RootCenter.Initialize())
		return FALSE;
	if (!g_RootClient.Startup(freebufferRoot, buffersizeRoot, addrRoot.c_str(), portRoot))
	{
		rTRACE("FAIL: root client - %s", addrRoot.c_str());
		gOnOneClientClose(oneclient_root);
		//return FALSE;
	}


	const size_t freebufferDB = gGetPrivateProfileIntEx(sec_dbrole, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeDB = gGetPrivateProfileIntEx(sec_dbrole, key_buffersize, file_relaycfg, 0);
	std::_tstring addrDB = gGetPrivateProfileStringEx(sec_dbrole, key_address, file_relaycfg);
	const unsigned short portDB = ROLEDBSVR_ROLEDB_PORT;

	if (!g_DBCenter.Initialize())
		return FALSE;
	if (!g_DBClient.Startup(freebufferDB, buffersizeDB, addrDB.c_str(), portDB))
	{
		rTRACE("FAIL: DBRole client - %s", addrDB.c_str());
		gOnOneClientClose(oneclient_db);
		//return FALSE;
	}

	
	const size_t freebufferGateway = gGetPrivateProfileIntEx(sec_gateway, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeGateway = gGetPrivateProfileIntEx(sec_gateway, key_buffersize, file_relaycfg, 0);
	std::_tstring addrGateway = gGetPrivateProfileStringEx(sec_gateway, key_address, file_relaycfg);
	const unsigned short portGateway = GATEWAY_GAMESVR_SERVICE_PORT;

	if (!g_GatewayCenter.Initialize())
		return FALSE;
	if (!g_GatewayClient.Startup(freebufferGateway, buffersizeGateway, addrGateway.c_str(), portGateway))
	{
		rTRACE("FAIL: gateway client - %s", addrGateway.c_str());
		gOnOneClientClose(oneclient_gateway);
		//return FALSE;
	}


	const size_t playercntHost = gGetPrivateProfileIntEx(sec_host, key_playercnt, file_relaycfg, 0);
	const size_t precisionHost = gGetPrivateProfileIntEx(sec_host, key_precision, file_relaycfg, 0);
	const size_t freebufferHost = gGetPrivateProfileIntEx(sec_host, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeHost = gGetPrivateProfileIntEx(sec_host, key_buffersize, file_relaycfg, 0);
	const DWORD listenaddrHost = INADDR_ANY;
	const unsigned short listenportHost = RELAY_GAMESVR_SERVICE_PORT;

	if (!g_HostServer.Startup(playercntHost, precisionHost, freebufferHost, buffersizeHost, listenaddrHost, listenportHost))
	{
		rTRACE("FAIL: host server");
		//return FALSE;
	}


	const size_t playercntTong = gGetPrivateProfileIntEx(sec_tong, key_playercnt, file_relaycfg, 0);
	const size_t precisionTong = gGetPrivateProfileIntEx(sec_tong, key_precision, file_relaycfg, 0);
	const size_t freebufferTong = gGetPrivateProfileIntEx(sec_tong, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeTong = gGetPrivateProfileIntEx(sec_tong, key_buffersize, file_relaycfg, 0);
	const DWORD listenaddrTong = INADDR_ANY;
	const unsigned short listenportTong = RELAY_GAMESVR_TONGSERVICE_PORT;

	if (!g_TongServer.Startup(playercntTong, precisionTong, freebufferTong, buffersizeTong, listenaddrTong, listenportTong))
	{
		rTRACE("FAIL: tong server");
		//return FALSE;
	}


	const size_t playercntChat = gGetPrivateProfileIntEx(sec_chat, key_playercnt, file_relaycfg, 0);
	const size_t precisionChat = gGetPrivateProfileIntEx(sec_chat, key_precision, file_relaycfg, 0);
	const size_t freebufferChat = gGetPrivateProfileIntEx(sec_chat, key_freebuffer, file_relaycfg, 0);
	const size_t buffersizeChat = gGetPrivateProfileIntEx(sec_chat, key_buffersize, file_relaycfg, 0);
	const DWORD listenaddrChat = INADDR_ANY;
	const unsigned short listenportChat = RELAY_GAMESVR_CHATSERVICE_PORT;

	if (!g_ChatServer.Startup(playercntChat, precisionChat, freebufferChat, buffersizeChat, listenaddrChat, listenportChat))
	{
		rTRACE("FAIL: chat server");
		//return FALSE;
	}

#ifdef _WORKMODE_SINGLETHREAD
	if (!g_threadSock.Start())
		return FALSE;
#endif

	// 帮会数据初始化
	if (g_cTongDB.Open())
	{
		rTRACE("tong database open ok!");
		if (g_cTongSet.InitFromDB())
			rTRACE("tongset init ok !");
		else
			rTRACE("FAIL: tongset init error!!!");
	}
	else
	{
		rTRACE("FAIL: tong database open error!!!");
	}

	return TRUE;
}

BOOL gUninitialize()
{
	UninitScript();

	try
	{
		//disable reclient timer
		gOnOneClientCreate(oneclient_root);
		gOnOneClientCreate(oneclient_gateway);
		gOnOneClientCreate(oneclient_db);


		gs_reclient_enable = FALSE;


#ifdef _WORKMODE_SINGLETHREAD
		g_threadSock.Stop();
#endif


		g_ChatServer.Shutdown();
		g_TongServer.Shutdown();
		g_HostServer.Shutdown();

		g_GatewayClient.Shutdown();
		g_GatewayCenter.Uninitialize();

		g_DBClient.Shutdown();
		g_DBCenter.Uninitialize();

		g_RootClient.Shutdown();
		g_RootCenter.Uninitialize();

		g_RelayServer.Shutdown();
		g_RelayCenter.Uninitialize();



		//delay
		::Sleep(2000);



		g_ChannelMgr.Uninitialize();
		g_FriendMgr.Uninitialize();


		g_libRainbow.Uninitialize();
		g_libHeaven.Uninitialize();

		g_memory.Uninitialize();

		g_cTongDB.Close();

	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL gIsLocalIP(DWORD ip)
{
	return (ip & _localmask) == _localtag;
}


const UCHAR* gGetHostMac(int adapt, size_t* pSize)
{
	static struct _MACSTORE
	{
		enum {MAC_SIZE = 6};

		UCHAR macLocal[MAC_SIZE];
		UCHAR macGlobal[MAC_SIZE];


		_MACSTORE()
		{
			memset(macLocal, 0, sizeof(macLocal));
			memset(macGlobal, 0, sizeof(macGlobal));

			gGetMacAndIPAddress(macLocal, NULL, macGlobal, NULL, _localtag, MAC_SIZE);
		}
	} _macstore;

	if (!pSize == NULL)
		*pSize = _MACSTORE::MAC_SIZE;

	switch (adapt)
	{
	case local_adapt: return _macstore.macLocal;
	case global_adapt: return _macstore.macGlobal;
	}

	if (!pSize == NULL)
		*pSize = 0;

	assert(FALSE);
	return NULL;
}


DWORD gGetHostIP(int adapt)
{
	static struct _IPSTORE
	{
		DWORD ipLocal;
		DWORD ipGlobal;

		_IPSTORE() : ipLocal(0), ipGlobal(0)
		{
			gGetMacAndIPAddress(NULL, &ipLocal, NULL, &ipGlobal, _localtag, 0);
		}
	} _ipstore;

	switch (adapt)
	{
	case local_adapt: return _ipstore.ipLocal;
	case global_adapt: return _ipstore.ipGlobal;
	}

	assert(FALSE);
	return 0;
}



BOOL gConnectToSibling(const char* addr)
{
	const size_t freebuffer = gGetPrivateProfileIntEx(sec_relay, key_freebuffer, file_relaycfg, 0);
	const size_t buffersize = gGetPrivateProfileIntEx(sec_relay, key_buffersize, file_relaycfg, 0);

	CRelayClient* pRelayClient = new CRelayClient(&g_RelayCenter, TRUE);
	if (!pRelayClient->Startup(freebuffer, buffersize, addr, RELAY_SIBLING_PORT, FALSE))
		return FALSE;

	return TRUE;
}


size_t gFillClntwayPckg(DWORD nameid, unsigned long param, 
						const RELAY_ASKWAY_DATA* pAskWay, size_t sizeAskWay, RELAY_ASKWAY_DATA* pClntWay, size_t sizeClntWay)
{
	assert(sizeAskWay >= sizeof(EXTEND_HEADER));
	assert(sizeClntWay >= sizeAskWay + sizeof(DWORD) * 2);

	size_t sizeNeed = sizeAskWay + sizeof(DWORD) * 2;

	if (sizeClntWay < sizeNeed)
		return 0;

	*pClntWay = *pAskWay;
	pClntWay->seekRelayCount ++;

	const void* pMethodData = pAskWay + 1;
	void* pMethodDataDup = pClntWay + 1;
	memcpy(pMethodDataDup, pMethodData, pAskWay->wMethodDataLength);

	DWORD* pp = (DWORD*)((BYTE*)pMethodDataDup + pClntWay->wMethodDataLength);
	pp[0] = nameid; pp[1] = param;
	pClntWay->wMethodDataLength += sizeof(DWORD) * 2;

	const void* pRouteData = (BYTE*)pMethodData + pAskWay->wMethodDataLength;
	void* pRouteDataDup = pp + 2;
	memcpy(pRouteDataDup, pRouteData, pAskWay->routeDateLength);

	return sizeNeed;
}

size_t gFillLosewayPckg(DWORD nToIP, DWORD nToRelayID, 
					   const void* pLosePckg, size_t sizeLosePckg, void* pLoseway, size_t sizeLoseway)
{
	assert(sizeLosePckg >= sizeof(EXTEND_HEADER));
	assert(sizeLoseway >= sizeLosePckg + sizeof(RELAY_DATA));

	size_t sizeNeed = sizeLosePckg + sizeof(RELAY_DATA);

	if (sizeLoseway < sizeNeed)
		return 0;

	EXTEND_HEADER* pSrcHdr = (EXTEND_HEADER*)pLosePckg;

	RELAY_DATA* pLoseData = (RELAY_DATA*)pLoseway;

	pLoseData->ProtocolFamily = pSrcHdr->ProtocolFamily;
	pLoseData->ProtocolID = relay_s2c_loseway;

	pLoseData->nToIP = nToIP;
	pLoseData->nToRelayID = nToRelayID;
	pLoseData->nFromIP = 0;
	pLoseData->nFromRelayID = -1;
	pLoseData->routeDateLength = sizeLosePckg;

	void* pRouteData = pLoseData + 1;
	memcpy(pRouteData, pLosePckg, sizeLosePckg);

	return sizeNeed;
}


BOOL gIsLegalString(LPCTSTR sz, size_t lenlolmt, size_t lenhilmt)
{
	if (sz == NULL || lenhilmt < lenlolmt)
		return FALSE;
	for (size_t pl = 0; pl < lenlolmt; pl++)
		if (sz[pl] == 0)
			return FALSE;
	for (size_t ph = lenlolmt; ph < lenhilmt; ph++)
		if (sz[ph] == 0)
			return TRUE;
	return TRUE;
}

DWORD g_String2Id(LPCSTR pString)
{
	DWORD Id = 0;
	for (int i = 0; pString[i]; i++)
	{
		Id = (Id + (i + 1) * pString[i]) % 0x8000000b * 0xffffffef;
	}
	return Id ^ 0x12345678;
}





BOOL gOnOneClientCreate(ONECLIENT ct)
{
	if (!gs_reclient_enable)
		return TRUE;


	HWND mainwnd = gGetMainWnd();
	if (mainwnd == NULL)
		return FALSE;

	switch (ct)
	{
	case oneclient_root:
		if (gs_timerRoot)
		{
			if (::KillTimer(mainwnd, gs_timerRoot))
			{
				gs_timerRoot = 0;
				rTRACE("disable reclient: root");
			}
		}
		break;
	case oneclient_gateway:
		if (gs_timerGateway)
		{
			if (::KillTimer(mainwnd, gs_timerGateway))
			{
				gs_timerGateway = 0;
				rTRACE("disable reclient: gateway");
			}
		}
		break;
	case oneclient_db:
		if (gs_timerDB)
		{
			if (::KillTimer(mainwnd, gs_timerDB))
			{
				gs_timerDB = 0;
				rTRACE("disable reclient: db");
			}
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL gOnOneClientClose(ONECLIENT ct)
{
	if (!gs_reclient_enable)
		return FALSE;

	HWND mainwnd = gGetMainWnd();
	if (mainwnd == NULL)
		return FALSE;

	switch (ct)
	{
	case oneclient_root:
		if (!gs_timerRoot)
		{
			INT rootinterval = gGetPrivateProfileIntEx(sec_root, key_retryinterval, file_relaycfg, 0);
			if (rootinterval > 0)
			{
				gs_timerRoot = ::SetTimer(mainwnd, timer_rootclient, rootinterval, NULL);
				if (!gs_timerRoot)
				{
					rTRACE("FAIL: enable root reclient");
					return FALSE;
				}
				rTRACE("enable reclient: root");
			}
		}
		break;
	case oneclient_gateway:
		if (!gs_timerGateway)
		{
			INT gatewayinterval = gGetPrivateProfileIntEx(sec_gateway, key_retryinterval, file_relaycfg, 0);
			if (gatewayinterval > 0)
			{
				gs_timerGateway = ::SetTimer(mainwnd, timer_gatewayclient, gatewayinterval, NULL);
				if (!gs_timerGateway)
				{
					rTRACE("FAIL: enable gateway reclient");
					return FALSE;
				}
				rTRACE("enable reclient: gateway");
			}
		}
		break;
	case oneclient_db:
		if (!gs_timerDB)
		{
			INT dbinterval = gGetPrivateProfileIntEx(sec_dbrole, key_retryinterval, file_relaycfg, 0);
			if (dbinterval > 0)
			{
				gs_timerDB = ::SetTimer(mainwnd, timer_dbclient, dbinterval, NULL);
				if (!gs_timerDB)
				{
					rTRACE("FAIL: enable db reclient");
					return FALSE;
				}
				rTRACE("enable reclient: db");
			}
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL gReOneClient(ONECLIENT ct)
{
	if (!gs_reclient_enable)
		return FALSE;

	switch (ct)
	{
	case oneclient_root:
		{{
		if (g_RootClient.IsReady())
			return FALSE;

		const size_t freebufferRoot = gGetPrivateProfileIntEx(sec_root, key_freebuffer, file_relaycfg, 0);
		const size_t buffersizeRoot = gGetPrivateProfileIntEx(sec_root, key_buffersize, file_relaycfg, 0);
		std::_tstring addrRoot = gGetPrivateProfileStringEx(sec_root, key_address, file_relaycfg);
		const unsigned short portRoot = RELAY_ROUTE_PORT;

		rTRACE("try reclient: root - %s", addrRoot.c_str());

		if (!g_RootClient.Startup(freebufferRoot, buffersizeRoot, addrRoot.c_str(), portRoot))
		{
			rTRACE("FAIL: reclient root - %s", addrRoot.c_str());
			//return FALSE;
		}
		}}
		break;

	case oneclient_gateway:
		{{
		if (g_GatewayClient.IsReady())
			return FALSE;

		const size_t freebufferGateway = gGetPrivateProfileIntEx(sec_gateway, key_freebuffer, file_relaycfg, 0);
		const size_t buffersizeGateway = gGetPrivateProfileIntEx(sec_gateway, key_buffersize, file_relaycfg, 0);
		std::_tstring addrGateway = gGetPrivateProfileStringEx(sec_gateway, key_address, file_relaycfg);
		const unsigned short portGateway = GATEWAY_GAMESVR_SERVICE_PORT;

		rTRACE("try reclient: gateway - %s", addrGateway.c_str());

		if (!g_GatewayClient.Startup(freebufferGateway, buffersizeGateway, addrGateway.c_str(), portGateway))
		{
			rTRACE("FAIL: reclient gateway - %s", addrGateway.c_str());
			//return FALSE;
		}
		}}
		break;

	case oneclient_db:
		{{
		if (g_DBClient.IsReady())
			return FALSE;

		const size_t freebufferDB = gGetPrivateProfileIntEx(sec_dbrole, key_freebuffer, file_relaycfg, 0);
		const size_t buffersizeDB = gGetPrivateProfileIntEx(sec_dbrole, key_buffersize, file_relaycfg, 0);
		std::_tstring addrDB = gGetPrivateProfileStringEx(sec_dbrole, key_address, file_relaycfg);
		const unsigned short portDB = ROLEDBSVR_ROLEDB_PORT;

		rTRACE("try reclient: db - %s", addrDB.c_str());

		if (!g_DBClient.Startup(freebufferDB, buffersizeDB, addrDB.c_str(), portDB))
		{
			rTRACE("FAIL: reclient db - %s", addrDB.c_str());
			//return FALSE;
		}
		}}
		break;
	}

	return TRUE;
}


BOOL gConsignClientSD(IClient* pClient)
{
	if (!pClient)
		return TRUE;

	HWND mainwnd = gGetMainWnd();
	if (!mainwnd)
		return FALSE;

	pClient->AddRef();
	::PostMessage(mainwnd, UM_CONSIGNCLIENTSD, (WPARAM)pClient, 0);

	return TRUE;
}


BOOL gStartFriendTimingUpdateDB(UINT elapse)
{
	if (gs_timerFriendUdtDB)
		return TRUE;

	HWND mainwnd = gGetMainWnd();
	if (!mainwnd)
		return FALSE;

	gs_timerFriendUdtDB = ::SetTimer(mainwnd, timer_friendudtdb, elapse, NULL);
	if (!gs_timerFriendUdtDB)
	{
		rTRACE("FAIL: start friend timing update db");
		return FALSE;
	}

	rTRACE("friend timing update db: start");

	return TRUE;
}

BOOL gStopFriendTimingUpdateDB()
{
	if (!gs_timerFriendUdtDB)
		return TRUE;

	HWND mainwnd = gGetMainWnd();
	if (!mainwnd)
		return FALSE;

	::KillTimer(mainwnd, gs_timerFriendUdtDB);
	gs_timerFriendUdtDB = 0;

	rTRACE("friend timing update db: stop");

	return TRUE;
}

BOOL gFriendTimingUpdateDB()
{
	if (!g_FriendMgr.BkgrndUpdateDB())
	{
		rTRACE("warning: update friend DB fail");
		return FALSE;
	}

	rTRACE("update friend DB succeed");
	return TRUE;
}
