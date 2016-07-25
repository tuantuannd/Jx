// -------------------------------------------------------------------------
//	文件名		：	DoScript.cpp
//	创建者		：	万里
//	创建时间	：	2003-9-16 21:07:01
//	功能描述	：	
//
// -------------------------------------------------------------------------

#include "stdafx.h"
#include "DoScript.h"

#include "Global.h"
#include "S3Relay.h"

DWORD g_ExecuteIP = 0;
DWORD g_ExecuteRelayID = 0;

void BroadGlobal(const void* pData, size_t size, DWORD nFromIP, DWORD nFromRelayID)
{
	if (pData == NULL || size == 0)
		return;

	RELAY_DATA data;
	data.ProtocolFamily = pf_relay;
	data.ProtocolID = relay_c2c_data;
	data.nToIP = 0;
	data.nToRelayID = 0;
	data.nFromIP = nFromIP;
	data.nFromRelayID = nFromRelayID;
	data.routeDateLength = 1 + size;

	size_t pckgsize = sizeof(RELAY_DATA) + data.routeDateLength;
	void* pRelayData = _alloca(pckgsize);
	
	char* pOData = (char*)pRelayData;

	memcpy(pOData, &data, sizeof(RELAY_DATA));
	pOData += sizeof(RELAY_DATA);

	*pOData = s2s_execute;
	pOData++;

	memcpy(pOData, pData, size);
	pOData += size;

	g_HostServer.BroadPackage(pRelayData, pckgsize);
}

void BroadGame(DWORD nToIP, const void* pData, size_t size, DWORD nFromIP, DWORD nFromRelayID)
{
	if (pData == NULL || size == 0 || nToIP == 0)
		return;

	CNetSockDupEx sockdup = dealrelay::FindRelaySockByIP(nToIP);
	if (!sockdup.IsValid())
		return;

	RELAY_DATA data;
	data.ProtocolFamily = pf_relay;
	data.ProtocolID = relay_c2c_data;
	data.nToIP = 0;
	data.nToRelayID = 0;
	data.nFromIP = nFromIP;
	data.nFromRelayID = nFromRelayID;
	data.routeDateLength = 1 + size;

	size_t pckgsize = sizeof(RELAY_DATA) + data.routeDateLength;
	void* pRelayData = _alloca(pckgsize);
	
	char* pOData = (char*)pRelayData;

	memcpy(pOData, &data, sizeof(RELAY_DATA));
	pOData += sizeof(RELAY_DATA);

	*pOData = s2s_execute;
	pOData++;

	memcpy(pOData, pData, size);
	pOData += size;

	sockdup.SendPackage(pRelayData, pckgsize);
}

void RootSend(DWORD nToIP, DWORD nToRelayID, const void* pData, size_t size)
{
	if (pData == NULL || size == 0)
		return;

	RELAY_DATA data;
	data.ProtocolFamily = pf_relay;
	data.ProtocolID = relay_c2c_data;
	data.nToIP = nToIP;
	data.nToRelayID = nToRelayID;
	data.nFromIP = 0;
	data.nFromRelayID = 0;
	data.routeDateLength = size;

	size_t pckgsize = sizeof(RELAY_DATA) + data.routeDateLength;
	void* pRelayData = _alloca(pckgsize);
	
	char* pOData = (char*)pRelayData;

	memcpy(pOData, &data, sizeof(RELAY_DATA));
	pOData += sizeof(RELAY_DATA);

	memcpy(pOData, pData, size);
	pOData += size;

	g_RootClient.SendPackage(pRelayData, pckgsize);
}


KLuaScript gs_Script;

int LuaGlobalExecute(Lua_State * L)
{
	if (Lua_GetTopIndex(L) != 1)
		return 0;
	
	const char * szMsg = Lua_ValueToString(L, 1);
	if (szMsg && szMsg[0])
	{
		int nszLen = strlen(szMsg);

		GM_EXECUTE_COMMAND exe;
		exe.ProtocolFamily = pf_gamemaster;
		exe.ProtocolType = gm_c2s_execute;
		strncpy(exe.AccountName , "GM", 32);
		exe.wLength = nszLen;
		exe.wExecuteID = 1;

		size_t pckgsize = sizeof(GM_EXECUTE_COMMAND) + exe.wLength;
		void* pRelayData = _alloca(pckgsize);

		char* pOData = (char*)pRelayData;

		memcpy(pOData, &exe, sizeof(GM_EXECUTE_COMMAND));
		pOData += sizeof(GM_EXECUTE_COMMAND);

		memcpy(pOData, szMsg, nszLen);
		pOData += nszLen;

		BroadGlobal(pRelayData, pckgsize, g_ExecuteIP, g_ExecuteRelayID);
	}

	return 0;
}

int LuaGameExecute(Lua_State * L)
{
	if (Lua_GetTopIndex(L) != 2)
		return 0;
	
	const char * szIP = Lua_ValueToString(L, 1);
	const char * szMsg = Lua_ValueToString(L, 2);

	DWORD nIP = _a2ip(szIP);
	if (nIP == 0)
		return 0;
	if (szMsg)
	{
		int nszLen = strlen(szMsg);

		GM_EXECUTE_COMMAND exe;
		exe.ProtocolFamily = pf_gamemaster;
		exe.ProtocolType = gm_c2s_execute;
		strncpy(exe.AccountName , "GM", 32);
		exe.wLength = nszLen;
		exe.wExecuteID = 1;

		size_t pckgsize = sizeof(GM_EXECUTE_COMMAND) + exe.wLength;
		void* pRelayData = _alloca(pckgsize);

		char* pOData = (char*)pRelayData;

		memcpy(pOData, &exe, sizeof(GM_EXECUTE_COMMAND));
		pOData += sizeof(GM_EXECUTE_COMMAND);

		memcpy(pOData, szMsg, nszLen);
		pOData += nszLen;

		BroadGame(nIP, pRelayData, pckgsize, g_ExecuteIP, g_ExecuteRelayID);
	}

	return 0;
}

int LuaMsg2IP(Lua_State * L)
{
	int nParamCount = Lua_GetTopIndex(L);
	if ( nParamCount < 3 ) 
		return 0;
	int nIP = 0;
	const char * szIP = Lua_ValueToString(L, 1) ;
	nIP = _a2ip(szIP);
	if (nIP == 0)
		return 0;

	int nID = (int) Lua_ValueToNumber(L, 2);
	const char * szMsg = Lua_ValueToString(L, 3) ;
	if (!szMsg) 
		return 0;

	int nParamID = 0;
	if (nParamCount < 4)
	{
		nParamID = 0;
	}
	else
	{
		nParamID = (int) Lua_ValueToNumber(L, 4);
	}

	if (szMsg && szMsg[0])
	{
		int nszLen = strlen(szMsg);

		char szID[32];

		itoa(nParamID, szID, 10);

		CHAT_MSG_EX ChatMsgEx;
		ChatMsgEx.ProtocolFamily = pf_playercommunity;
		ChatMsgEx.ProtocolID = playercomm_channelchat;
		strcpy(ChatMsgEx.m_szSourceName, szID);
		strcpy(ChatMsgEx.m_szAccountName, "GM");
		ChatMsgEx.SentenceLength = nszLen;

		size_t pckgsize = sizeof(ChatMsgEx) + ChatMsgEx.SentenceLength;
		void* pRelayData = _alloca(pckgsize);

		char* pOData = (char*)pRelayData;

		memcpy(pOData, &ChatMsgEx, sizeof(CHAT_MSG_EX));
		pOData += sizeof(CHAT_MSG_EX);

		memcpy(pOData, szMsg, nszLen);
		pOData += nszLen;

		RootSend(nIP, nID, pRelayData, pckgsize);
	}

	return 0;
}

int LuaGetIP(Lua_State * L)
{
	int nParamCount = Lua_GetTopIndex(L);
	if ( nParamCount < 1 ) 
		return 0;
	int nParamID = (int) Lua_ValueToNumber(L, 1);

	char szDesMsg[200];
	szDesMsg[0] = 0;
	if (nParamID == 0)
	{
		strcpy(szDesMsg, _ip2a(gGetHostIP(local_adapt)));
	}
	else if (nParamID == 1)
	{
		strcpy(szDesMsg, _ip2a(gGetHostIP(global_adapt)));
	}
	else
	{
		strcpy(szDesMsg, "0.0.0.0");
	}

	Lua_PushString(L, szDesMsg);
	return 1;
}

TLua_Funcs GameScriptFuns[] = 
{
	{"GlobalExecute", LuaGlobalExecute},		//GlobalExecute通知整个服务组执行一个脚本
	{"GameExecute", LuaGameExecute},		//GameExecute通知一个游戏世界执行一个脚本
	{"Msg2IP", LuaMsg2IP},		//Msg2IP返回结果值到发出通知的地方
	{"GetIP", LuaGetIP},		//GetIP(index)
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int g_GetGameScriptFunNum()
{
	return sizeof(GameScriptFuns)  / sizeof(TLua_Funcs);
}

BOOL InitScript()
{
	if (gs_Script.Init() && gs_Script.RegisterFunctions(GameScriptFuns, g_GetGameScriptFunNum()))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL ExcuteScript(DWORD nIP, DWORD nRelayID, const char * ScriptCommand)
{
	if (ScriptCommand && ScriptCommand[0] != 0)
	{
		if (gs_Script.LoadBuffer((PBYTE)ScriptCommand, strlen(ScriptCommand)))
		{
			g_ExecuteIP = nIP;
			g_ExecuteRelayID = nRelayID;
			return gs_Script.ExecuteCode();
		}
	}

	return FALSE;
}

BOOL UninitScript()
{
	return TRUE;
}
