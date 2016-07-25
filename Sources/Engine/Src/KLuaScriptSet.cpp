//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KLuaScriptSet.cpp
// Date:	2001-10-18 11:11:18
// Code:	Romandou
// Desc:	
//---------------------------------------------------------------------------
#include "KWin32.h"

/*
#include "KDebug.h"
#include "KLuaScriptSet.h"

//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::KLuaScriptSet
// 功能:	
// 返回:	
//---------------------------------------------------------------------------
KLuaScriptSet::KLuaScriptSet()
{
	KScriptSet::KScriptSet();
	m_nMaxScriptNum = MAXLUASCRIPTNUM;
	m_nNumPerClear =  NUMPERCLEAR;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::~KLuaScriptSet
// 功能:	
// 返回:	
//---------------------------------------------------------------------------
KLuaScriptSet::~KLuaScriptSet()
{
	KScriptSet::~KScriptSet();
}

//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::CreateScript
// 功能:	
// 参数:	char * szKey
// 参数:	int StackSize
// 返回:	KScript * 
//---------------------------------------------------------------------------
KScript * KLuaScriptSet::CreateScript(char * szKey, int StackSize)
{
	//清理
	ClearUpSet();	
	if (szKey == NULL && StackSize < 0) return NULL;
	char * szFileName =  GetScriptFileNameFromKey(szKey);
	if (szFileName == NULL ) return NULL;
	
	KLuaScript * pScript = new KLuaScript(StackSize);
	if (! pScript) return NULL;
	
	//脚本对象的脚本名为关键字名
	if (! pScript->Init()) return NULL;
	if (! pScript->Load(szFileName)) return NULL;
	return pScript;
}

KScript * KLuaScriptSet::CreateScript(DWORD nKey, int StackSize)
{
	//清理
	ClearUpSet();	
	if (StackSize < 0) return NULL;
	char * szFileName =  GetScriptFileNameFromKey(nKey);
	if (szFileName == NULL ) return NULL;
	
	KLuaScript * pScript = new KLuaScript(StackSize);
	if (! pScript) return NULL;
	
	//脚本对象的脚本名为关键字名
	if (! pScript->Init()) return NULL;
	if (! pScript->Load(szFileName)) return NULL;
	
	return pScript;
}


//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::ClearUpSet
// 功能:	
// 参数:	void
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScriptSet::ClearUpSet(void)
{
	
	TScript * pTScript = NULL;
	TScript * pDelTScript = NULL;

	if (GetCount() >= m_nMaxScriptNum) 
	{
		pTScript = m_pLestRecent;
//	printf("ClearUpSet\n");
		for (DWORD i = 0; i < m_nNumPerClear;i++)
		{
			pDelTScript = pTScript;
			pTScript = pTScript->pMoreRecent;
			DeleteScript(pDelTScript->szKey);
		}
	
		return TRUE;
	}

	return FALSE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::ClearUpSet
// 功能:	按照最旧使用脚本的顺序删除nCount个脚本实例
// 参数:	DWORD nCount
// 返回:	int 返回实际删除的脚本数量
//---------------------------------------------------------------------------
DWORD KLuaScriptSet::ClearUpSet(DWORD nCount)
{
	
	TScript * pTScript = NULL;
	TScript * pDelTScript = NULL;
	
	DWORD	nUsedCount = GetCount();
	
	if (nUsedCount < nCount)
		nCount = nUsedCount;

		pTScript = m_pLestRecent;
		
		for (DWORD i = 0; i < nCount; i++)
		{
			pDelTScript = pTScript;
			pTScript = pTScript->pMoreRecent;
			DeleteScript(pDelTScript->szKey);
		}
		
	return nCount;

}

//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::SetMaxScriptNum
// 功能:	
// 参数:	DWORD nMaxCount
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScriptSet::SetMaxScriptNum(DWORD nMaxCount )
{
	if (nMaxCount < 0)
		return FALSE;
	
	m_nMaxScriptNum = nMaxCount;
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScriptSet::SetNumPerClear
// 功能:	
// 参数:	DWORD nNumPerClear
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScriptSet::SetNumPerClear(DWORD nNumPerClear)
{
	if (nNumPerClear < 0 )
		return FALSE;
	
	if (m_nMaxScriptNum < nNumPerClear)
		return FALSE;
	
	m_nNumPerClear = nNumPerClear;
	return TRUE;
}


BOOL KLuaScriptSet::Run(DWORD nKey)
{
	KLuaScript *pScript ;
	pScript = (KLuaScript* ) this->GetScript(nKey,1,1);
	if (pScript)
	{
		return pScript->Execute();
	}
	return FALSE;
	
}

BOOL KLuaScriptSet::Run(char * szKey)
{
	KLuaScript *pScript ;
	pScript = (KLuaScript* ) this->GetScript(szKey,1,1);
	if (pScript)
	{
		return pScript->Execute();
	}
	return FALSE;
	
}


BOOL KLuaScriptSet::RunFunction(DWORD nKey, char * szFuncName, int nResults, char * szFormat, ...)
{
	KLuaScript * pScript;
	pScript = (KLuaScript *) this->GetScript(nKey, 1, 1);
	if (pScript)
	{
		va_list va;
		BOOL bResult;
		va_start(va, szFormat);
		bResult = pScript->CallFunction(szFuncName, nResults, szFormat, va);
		va_end(va);
		return bResult;

	}
	return FALSE;

}

BOOL KLuaScriptSet::RunFunction(char * szKey, char * szFuncName, int nResults, char * szFormat, ...)
{
	KLuaScript * pScript;
	pScript = (KLuaScript *) this->GetScript(szKey, 1, 1);
	
	if (pScript)
	{
		va_list va;
		BOOL bResult;
		va_start(va, szFormat);
		bResult = pScript->CallFunction(szFuncName, nResults, szFormat, va);
		va_end(va);
		return bResult;
		
	}
	return FALSE;

}

*/
