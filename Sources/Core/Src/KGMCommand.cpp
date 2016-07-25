#include "KCore.h"
#include "KPlayer.h"
#include "LuaFuns.h"
#include "KGMCommand.h"
#include "KSortScript.h"
#define GMCMD_GENRE_PREFIX_LEN 3
#define GM_CTRL_CMD_PREFIX '?'

static TGameMaster_Command GM_Command[]=
{
	{"DoSct",			GMDoScriptAction},				//DoSct Say("abc");
	{"ds",				GMDoScriptAction},				//DS Say("abc");

	{"dw",				GMDoWorldScriptAction},			//dw AddNews("abc");

	{"RunSctFile",		GMRunScriptFile},				//RunSctFile /Scripts/Abc.lua main 10
	{"RSF",				GMRunScriptFile},				//RSF /Scripts/Abc.lua main 10

	{"ReLoadSct",		GMReloadScriptFile},			//ReloadSct  /Scripts/Abc.lua
	{"RLS",				GMReloadScriptFile},
	
	{"ReLoadAllSct",	GMReloadAllScriptFile},			//ReloadAllSct
	{"RLAS",			GMReloadAllScriptFile},
	
};
#ifdef _DEBUG
CORE_API BOOL TextGMFilter(int nPlayerIdx, const char* pText, int nLen)
#else
BOOL TextGMFilter(int nPlayerIdx, const char* pText, int nLen)
#endif
{
	try
	{
		
		if (!pText)	return FALSE;
		bool	bHandled = false;
		if (nLen >= GMCMD_GENRE_PREFIX_LEN + 1 && pText[0] == GM_CTRL_CMD_PREFIX)
		{
			if ((*(unsigned int*)pText) == 0x206D673F || (*(unsigned int*)pText) == 0x204D473F)	// 0x2067642F = "/gm " "/GM "
			{
				bHandled = (bool)(0 != TextMsgProcessGMCmd(nPlayerIdx, pText + GMCMD_GENRE_PREFIX_LEN + 1,
					nLen - GMCMD_GENRE_PREFIX_LEN - 1));
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		return FALSE;
	}
	catch(...)
	{
		printf("执行GM指令,发生异常!\n");
		return FALSE;
	}

	
}

BOOL TextMsgProcessGMCmd(int nPlayerIdx, const char * pGMCmd, int nLen)
{
	if (nLen <= 0 || !pGMCmd) return FALSE;
	char szCmd[20];
	const char * pStart = strstr(pGMCmd," ");// Fixed By MrChuCong@gmail.com
	int nTempLen = nLen;
	
	if (NULL == pStart)
	{
		memcpy(szCmd, pGMCmd, nTempLen);
		szCmd[nLen] = 0;
	}
	else
	{	nTempLen = pStart - pGMCmd;
		memcpy(szCmd, pGMCmd, nTempLen);
		szCmd[nTempLen] = 0;
	}
	for(int i  = 0; i < sizeof(GM_Command) / sizeof(TGameMaster_Command); i ++)
	{
		if (strcmp(GM_Command[i].Command, szCmd) == 0)
			return ProcessGMCommand(nPlayerIdx, GM_Command[i].eCommandId, pGMCmd + nTempLen + 1, nLen - nTempLen - 1);
	}
		
	return FALSE;
}

BOOL  ProcessGMCommand(int nPlayerIdx, EGameMasterCommand eCommand, const char * pParam, int nLen)
{
	
	switch(eCommand)
	{
	
	case GMDoWorldScriptAction:
		{
			if (nLen <=0 || nLen >= 300)
				return FALSE;
			char szScriptAction[300];
			memcpy(szScriptAction, pParam, nLen);
			szScriptAction[nLen] = 0;
			BOOL bResult = FALSE;
			KLuaScript WorldScript;
			WorldScript.Init();
			WorldScript.RegisterFunctions(WorldScriptFuns, g_GetWorldScriptFunNum());
			
			if (WorldScript.LoadBuffer((PBYTE)szScriptAction, nLen))
			{
				bResult = WorldScript.ExecuteCode();	
			}
			return bResult;

		}break;

	case GMDoScriptAction:
		{
			if (nLen <= 0 || nLen >= 300) 
				return FALSE;
			//检查nPlayerIdx的权限
			if (nPlayerIdx < 0 || Player[nPlayerIdx].m_dwID <= 0)
				return FALSE;
			char szScriptAction[300];
			memcpy(szScriptAction, pParam, nLen);
			szScriptAction[nLen] = 0;
			Player[nPlayerIdx].DoScript(szScriptAction);
			return TRUE;

		}break;

	case GMRunScriptFile:
		{
			if (nPlayerIdx < 0 || Player[nPlayerIdx].m_dwID <= 0)
				return FALSE;
			char szScriptFile[200];
			char szScriptFun[100];
			char szScriptParam[100];
			int nBufLen = GetNextUnit(pParam, ' ', nLen, szScriptFile);
			if (szScriptFile[0] == 0) return FALSE;

			int nBufLen1 = GetNextUnit(pParam + nBufLen, ' ', nLen - nBufLen, szScriptFun);
			if (szScriptFun[0] == 0) return FALSE;
			
			GetNextUnit(pParam + nBufLen + nBufLen1, ' ', nLen - nBufLen - nBufLen1, szScriptParam);
			return Player[nPlayerIdx].ExecuteScript(szScriptFile, szScriptFun, szScriptParam);
			
		}break;
	case GMReloadScriptFile:
		{
			if (nPlayerIdx < 0 || Player[nPlayerIdx].m_dwID <= 0)	return FALSE;
			char szScriptFile[200];
			GetNextUnit(pParam, ' ', nLen, szScriptFile);
			if (szScriptFile[0] == 0) return FALSE;
			ReLoadScript(szScriptFile);
			return TRUE;

		}break;
	case GMReloadAllScriptFile:
		{
			return ReLoadAllScript();
		}break;
	}

	return FALSE;
}

int GetNextUnit(const char * szString , const char cDiv, int nLen, char * szResult)
{
	szResult[0] = 0;
	if (nLen <= 0 || szString == NULL)
		return FALSE;
	
	char * pChar = (char *)szString;
	int i = 0;
	int j = 0;
	BOOL bFind = FALSE;

	while (*(pChar + i) == cDiv && i < nLen) i++;
	
	while(i  < nLen && *(pChar + i) != '\0')
	{
		if (*(pChar + i) == cDiv) 
		{
			szResult[j] = 0;
			return i;
		}
		else
			szResult[j++] = *(pChar + i);
		i ++;
	}

	szResult[j] = 0;
	return i;	
}
