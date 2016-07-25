#ifdef USEOLD
#include "KWin32.h"
#include "KEngine.h"
#include "KDebug.h"
#include "KStepLuaScript.h"
#include "LuaLib.h"
#include "KScriptList.h"
#include "string.h"
#include "LuaFuns.h"
#include "KCore.h"
#include "KNpc.h"
#include "KSubWorld.h"
//#include "KNetClient.h"
#include "../MultiServer/Rainbow/Interface/iClient.h"
#include "KScriptValueSet.h"
#include "KNpcSet.h"
#include "KPlayer.h"

KScriptList g_StoryScriptList;
KStepLuaScript * LuaGetScript(Lua_State * L);

int LuaAddNpcFromTemplate(Lua_State * L);
int LuaMessageBox(Lua_State * L);
int LuaSendScriptMessage(Lua_State * L);
int LuaWaitForEvent(Lua_State * L);
int LuaSendMessage(Lua_State * L);
int LuaSendScriptMessage(Lua_State * L);
int LuaGetScriptState(Lua_State * L);
int LuaWait(Lua_State * L);
int LuaEndWait(Lua_State * L);
int LuaNewScript(Lua_State * L);
int LuaGotoLabel(Lua_State * L);
int LuaNpcGoto(Lua_State * L);
int LuaNpcTalk(Lua_State * L);
int LuaNpcGotoEx(Lua_State * L);
int LuaGetNpcPos(Lua_State * L);
int LuaGetValue(Lua_State * L);
int LuaSetValue(Lua_State * L);
int LuaReturn(Lua_State * L);
int LuaAddNpcClient(Lua_State * L);
int LuaSetPlayer(Lua_State * L);
int LuaInitStandAloneGame(Lua_State * L);
int LuaCastSkill(Lua_State *L);
int LuaGetNpcTalk(Lua_State * L);
int LuaResume(Lua_State * L);
int LuaLabel(Lua_State * L);
int LuaGetTempTaskValue(Lua_State * L);
int LuaSetTempTaskValue(Lua_State * L);

#ifdef _SERVER
int LuaTaskSay(Lua_State * L);
#endif

CORE_API TLua_Funcs SysFuns[] = 
{
{"Wait", LuaWait},
{"EndWait", LuaEndWait},
{"GetScriptState", LuaGetScriptState},
{"NewScript", LuaNewScript},
{"Goto", LuaGotoLabel},
{"Label",LuaLabel},
{"MessageBox", LuaMessageBox},
{"SendScriptMessage", LuaSendScriptMessage},
{"WaitForEvent", LuaWaitForEvent},
{"SendMessage",LuaSendMessage},
{"NpcGoto", LuaNpcGoto},
{"Talk", LuaNpcTalk},
{"NpcGotoEx",LuaNpcGotoEx},
{"GetNpcPos", LuaGetNpcPos},
{"SetValue",LuaSetValue},
{"GetValue", LuaGetValue},
{"Exit",LuaReturn},
{"AddNpc",LuaAddNpcClient},
{"SetPlayer",LuaSetPlayer},
{"InitGameWorld",LuaInitStandAloneGame},
{"CastSkill", LuaCastSkill},
{"GetNpcTalk",LuaGetNpcTalk},
{"Resume", LuaResume},
{"InsertNpc", LuaAddNpcFromTemplate},
#ifdef _SERVER
{"TaskSay", LuaTaskSay},
#endif
};

CORE_API int g_GetLuaFunsCount()
{
	return sizeof(SysFuns) / sizeof(SysFuns[0]);
}

int LuaLabel(Lua_State * L)
{
	return 0;
}

int LuaGotoLabel(Lua_State * L)
{
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	const char * szLabelName;
	szLabelName = lua_tostring(L, 1);
	pScript->GotoLabel((char *)szLabelName);
	return 0;

}

KStepLuaScript * LuaGetScript(Lua_State * L)
{
	KStepLuaScript * pOrScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	unsigned int Addr = (unsigned int )lua_tonumber(pOrScript->m_LuaState, 1);
	Lua_State * pResultState = (Lua_State * )Addr;
	return  (KStepLuaScript *) g_StoryScriptList.GetScript(pResultState);
}

int LuaNewScript(Lua_State * L)
{
	
	KStepLuaScript * pOrScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	KStepLuaScript * pScript = new KStepLuaScript(0);
	KScriptNode * pNode = new KScriptNode;
	
	const char * szScriptFIle = NULL;
	szScriptFIle = lua_tostring(pOrScript->m_LuaState, 1);
	
	if (strlen(szScriptFIle)<=0)
		return -1;
	
	if (!pScript->Init())
	{
		Lua_PushNil(L);
		return 1;
	}

	g_SetFilePath("\\Script");
	char FilePath[200];
	g_GetFullPath(FilePath, (char *)szScriptFIle);
	if (!pScript->Load(( char *)FilePath))
	{
		Lua_PushNil(L);
		return 1;
	}
	pScript->RegisterFunctions(LuaFuns,g_GetLuaFunsCount());
	pScript->SeekToExeBegin();
	pScript->RunMain();

	pNode->pScript = pScript;
	g_StoryScriptList.AddTail(pNode);
	pScript->RunMain();
	Lua_PushNumber(L, (unsigned int) pScript->m_LuaState);
	return 1;

}

int LuaResume(Lua_State * L)
{
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	if (pScript)
		pScript->RunResume();
	return 0;
}

int LuaWait(Lua_State * L)
{

	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	pScript->PosUp();
	pScript->RunFunc();
	return 0;
}

int LuaEndWait(Lua_State * L)
{
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	pScript->RunMain();
	return 0;
}

int LuaGetScriptState(Lua_State * L)
{
	KStepLuaScript * pScript;

	pScript = LuaGetScript(L);
	
	//不存在
	if(pScript == NULL)
	{
		Lua_PushNumber(L, -1);
		return -1;
	}
	Lua_PushNumber(L, pScript->GetStatus());
	return 1;
}




int LuaSendScriptMessage(Lua_State * L)
{

	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	unsigned int stateid;
	
	stateid =(unsigned int ) lua_tonumber(L, 1);
	const char * szMessage = lua_tostring(L, 2);
	const char * szData	 = lua_tostring(L, 3);
	KStepLuaScript * pSendedScript = LuaGetScript((Lua_State * )stateid);
	
	if (pSendedScript == NULL)
	{
		g_DebugLog("无法找到指定的脚本对象");
		return 0;
	}
	
	pScript->SendMessage(pSendedScript, (char *)szMessage, (char *)szData);
	return 0;
}

int LuaSendMessage(Lua_State * L)
{
	g_StoryScriptList.SendMessage((char *) lua_tostring(L,1), (char*) lua_tostring(L,2));
	return 0;
}

/*!*****************************************************************************
// Function		: LuaWaitForEvent
// Purpose		: 
// Return		: int 
// Argumant		: Lua_State * L
// Comments		:
// Author		: RomanDou
*****************************************************************************/
int LuaWaitForEvent(Lua_State * L)
{
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	strcpy(pScript->GetWaitingMsg()	, lua_tostring(L, 1));
	pScript->RunWaitMsg();
	return 0;
}

int LuaGetNpcPos(Lua_State * L)
{
	char szName[100];
	strcpy(szName, lua_tostring(L,1));
	
	for(int i  = 0 ; i < MAX_NPC; i ++)
	{
		if (Npc[i].m_dwID <= 0)		continue;
		if (!strcmp(Npc[i].Name, szName))	break;
		
	}
	if (i >= MAX_NPC) return 0 ;
	
	int nCurPosX, nCurPosY;
	Npc[i].GetMpsPos(&nCurPosX,&nCurPosY);
	lua_pushnumber(L, nCurPosX);
	lua_pushnumber(L, nCurPosY);
	return 2;
}

int LuaNpcGotoEx(Lua_State * L)
{
	char szName[100];
	strcpy(szName, lua_tostring(L,1));
	
	for(int i  = 0 ; i < MAX_NPC; i ++)
	{
		if (Npc[i].m_dwID <= 0)		continue;
		if (!strcmp(Npc[i].Name, szName))	break;
		
	}
	if (i >= MAX_NPC) return 0 ;
	
	int nDesX = (int)lua_tonumber(L,2);
	int nDesY = (int)lua_tonumber(L,3);
	
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	Npc[i].SendCommand(do_walk, nDesX, nDesY);
	return 0;
}

int LuaNpcGoto(Lua_State * L)
{
	char szName[100];
	strcpy(szName, lua_tostring(L,1));

	for(int i  = 0 ; i < MAX_NPC; i ++)
	{
		if (Npc[i].m_dwID <= 0)		continue;
		if (!strcmp(Npc[i].Name, szName))	break;
		
	}
	if (i >= MAX_NPC) return 0 ;

	int nDesX = (int)lua_tonumber(L,2);
	int nDesY = (int)lua_tonumber(L,3);
	
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	if(pScript->IsRunMain())
	{
		Npc[i].SendCommand(do_walk, nDesX, nDesY);
		BYTE	NetCommand[9];
		NetCommand[0] = (BYTE)c2s_npcwalk;
		*(int *)&NetCommand[1] = nDesX;
		*(int *)&NetCommand[5] = nDesY;
		SendToServer(NetCommand, sizeof(NetCommand));
		pScript->RunFunc();
	}
	else
	{
		int nSrcX = 0;
		int nSrcY = 0;

		SubWorld[Npc[i].m_SubWorldIndex].Map2Mps(Npc[i].m_RegionIndex, Npc[i].m_MapX, Npc[i].m_MapY, Npc[i].m_OffX, Npc[i].m_OffY, &nSrcX, &nSrcY);
		if ( abs(nSrcX - nDesX) < Npc[i].m_WalkSpeed && abs(nSrcY - nDesY) < Npc[i].m_WalkSpeed)
			pScript->RunMain();
		else
			Npc[i].SendCommand(do_walk, nDesX, nDesY);
	}
	return 0;
}

int LuaNpcTalk(Lua_State * L)
{
	char szName[100];
	strcpy(szName, lua_tostring(L,1));
	if (!szName) return 0;
	for(int i  = 0 ; i < MAX_NPC; i ++)
	{
		if (Npc[i].m_dwID <= 0)		continue;
		if (!strcmp(Npc[i].Name, szName))	break;
	}

	if (i >= MAX_NPC) return 0 ;
	char *szTalk;
	szTalk = (char *)lua_tostring(L, 2);
	
	Npc[i].DoPlayerTalk(szTalk);
	strcpy(Npc[i].m_szChatBuffer, szTalk);
	Npc[i].m_nCurChatTime = NPC_SHOW_CHAT_TIME_LENGTH;
	return 0;
}

int LuaGetValue(Lua_State * L)
{
	int nArg = lua_gettop(L);
	if (nArg == 0) return 0;
	
	char * pValueName = NULL;
	pValueName = (char *)lua_tostring(L, 1);
	
	if (nArg == 1)
	{
		int nValue = 0;
		char  strValue[100];
		if (!g_ScriptValueSet.GetValue(pValueName, &nValue))
		{
			if (!g_ScriptValueSet.GetValue(pValueName, strValue))
			{
				lua_pushnumber(L, 0);
			}
			else 
			{
				lua_pushstring(L, strValue);
			}
		}
		else
		{
			lua_pushnumber(L,nValue);
		}
	}
	
	if (nArg == 2)
	{
		int nType = (int)lua_tonumber(L, 2);
		if (nType = 0)
		{
			char strValue[100];
			if (!g_ScriptValueSet.GetValue(pValueName, strValue))
			{
				strcpy(strValue, "");
			}
			lua_pushstring(L, strValue);
		}
		else
		{
			int nValue = 0 ;
			if (!g_ScriptValueSet.GetValue(pValueName, &nValue))
			nValue = 0;
			lua_pushnumber(L, nValue);
		}
	}

//	int itop = lua_gettop(L);
	return 1;
}

int LuaSetValue(Lua_State * L)
{
	char *szValueName = NULL;
	szValueName = (char *)lua_tostring(L,1) ;
	if (!strcmp(szValueName,"")) return 0;
	
	if (lua_isnumber(L, 2))
	{
		int nValue = (int)lua_tonumber(L, 2);
		g_ScriptValueSet.SetValue(szValueName, nValue);
		
	}
	else
	{
		if (lua_isstring(L,2))
		{
			char * pStr = (char *)lua_tostring(L,2);
			g_ScriptValueSet.SetValue(szValueName, pStr);
		}
	}

	return 0;
}

int LuaReturn(Lua_State *L)
{
	KStepLuaScript * pScript = (KStepLuaScript *)g_StoryScriptList.GetScript(L);
	if (pScript)
	pScript->RunIdle();
	return 0;
}
//AddNpc（"*.ini", Name, map, x, y, direction）
//AddNpcFromTemplate(Name, Level, subworld, x, y) 
int LuaAddNpcFromTemplate(Lua_State * L)
{
	const char * pName = lua_tostring(L,1);	
	int nId = g_NpcSetting.FindRow((char*)pName) - 2;
	if (nId < 0) nId = 0;


	int nLevel = (int)lua_tonumber(L,2);
	if (nLevel >= 128) nLevel = 127;
	if (nLevel < 0 ) nLevel = 1;

	int	nNpcIdxInfo = (nId << 7) + nLevel;
	
	int nNpcIdx = NpcSet.Add(nNpcIdxInfo, (int)lua_tonumber(L, 3), (int)lua_tonumber(L,4), (int)lua_tonumber(L,5));
	g_StrCpy(Npc[nNpcIdx].Name, (char*)pName);
#ifndef _SERVER
	Npc[nNpcIdx].m_dwID = (DWORD)lua_tonumber(L,6);
#endif

	if (nNpcIdx <= 0) return 0;
	return 0;
}

int LuaAddNpcClient(Lua_State * L)
{
	const char * pIniFile = lua_tostring(L,1);	
	const char * pName = lua_tostring(L,2);
	int	nNpcIdx;
	
	if (pIniFile[0])
	{
		KIniFile IniFile;
		if (IniFile.Load("pIniFile"))
			NpcSet.Add(&IniFile, (char*)pName);
		else 
			return 0;
	}
	else
	{
		nNpcIdx = NpcSet.Add(1, 0, 0,(int)lua_tonumber(L,4), (int)lua_tonumber(L,5));
		g_StrCpy(Npc[nNpcIdx].Name, (char*)pName);
		
#ifdef TOOLVERSION
		//技能编缉器暂时这样用
		//Question
		for (int i  = 0 ; i < 100; i ++)//
			Npc[nNpcIdx].m_SkillList.Add(i+1);
#endif
	}
	
	Npc[nNpcIdx].m_dwID = (DWORD)lua_tonumber(L,6);
	if (nNpcIdx <= 0) return 0;
	return 0;
}


//SetPlayer("Name");
int LuaSetPlayer(Lua_State * L)
{
	for (int i  = 0 ; i < MAX_REGION; i ++)
		SubWorld[0].m_Region[i].Init(400,400);

	int nIdx = NpcSet.SearchName((char *)lua_tostring(L,1));
	if (nIdx == 0)  return 0;
	Player[CLIENT_PLAYER_INDEX].m_nIndex = nIdx;
	Player[CLIENT_PLAYER_INDEX].m_dwID = Npc[nIdx].m_dwID;
	return 0;
}
CORE_API FILE *stream1;
CORE_API FILE* stream;

int LuaInitStandAloneGame(Lua_State * L)
{
#ifndef _SERVER
	/*
	WORLD_SYNC sync;
	sync.ProtocolType = 6;
	sync.SubWorld = 1000;
	sync.Frame = 0;
	sync.Region = 0;
	sync.Weather = 0;
	Player[CLIENT_PLAYER_INDEX].SyncWorld((BYTE *)&sync);
	CURPLAYER_SYNC cursyn;
	cursyn.m_dwID = Player[CLIENT_PLAYER_INDEX].m_dwID;
	cursyn.ProtocolType = 10;
	Player[CLIENT_PLAYER_INDEX].SyncCurPlayer((BYTE*)&cursyn);
//	printf("ok!");
//	fprintf(sout,"Hello World!");
//	fflush(sout);
/*	stream = freopen( "d:\\output.txt", "w", stdout );
	stream1 = freopen( "d:\\error.txt", "w", stderr );
	printf("abc");
	if (stream == NULL )
	{
		printf("error !");
		return 0;
	}
	if (stream1 == NULL )
	{
		printf("error !");
		return 0;
	}
*/	
#endif
	return 0;
}
//CastSkill(SrcNpcName, nSkillId, nParam1, nParam2);
int LuaCastSkill(Lua_State * L)
{
	char * SrcNpcName = (char *)lua_tostring(L,1);
	int nSrc = NpcSet.SearchName(SrcNpcName);
	
	if (nSrc == 0 ) return 0;
	Npc[nSrc].SendCommand(do_skill, (int)lua_tonumber(L,2), (int)lua_tonumber(L,3), (int)lua_tonumber(L,4));
	return 0;
}

int LuaGetNpcTalk(Lua_State * L)
{
	int argnum = lua_gettop(L);
	if (argnum == 0) 
	{
		lua_pushstring(L,""); return 1;
	}

	char  SrcNpcName[100] ;
	strcpy(SrcNpcName,(char *)lua_tostring(L,1));
	int nSrc = NpcSet.SearchName(SrcNpcName);
	
	if (nSrc == 0 ) lua_pushstring(L," ");
	else
	{
		
		char c1 = 19;
		char c2 = 13;
		
		char *strBuf ;
		if (Npc[nSrc].m_szChatBuffer[0] == 19) strBuf = Npc[nSrc].m_szChatBuffer + 2;
		else
		{
			strBuf = Npc[nSrc].m_szChatBuffer;
		}
		

		int len = strlen(strBuf);
		if (strBuf[len - 1] == 13)
			strBuf[len -1] = 0;
		lua_pushstring(L,strBuf);
		
		return 1;
	}
	return 1;
}
#endif