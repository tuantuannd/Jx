//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KScriptEngine.cpp
// Date:	2001-9-11 10:33:35
// Code:	Romandou
// Desc:	脚本集合控制基类
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KScriptSet.h"
//---------------------------------------------------------------------------
// 函数:	KScriptSet::KScriptSet
// 功能:	
// 返回:	
//---------------------------------------------------------------------------
KScriptSet::KScriptSet()
{
	m_nKeyStyle = KEY_STRING;
	m_pRootList = NULL;
	m_pLestRecent = NULL;
	m_pMostRecent = NULL;
}

KScriptSet::KScriptSet(int Key_Style)
{
	if (Key_Style == KEY_NUMBER || Key_Style == KEY_STRING)
	m_nKeyStyle = Key_Style;
	else 
	{
		m_nKeyStyle = KEY_STRING;
		printf("UnKnown ScriptSet KeyStyle:[%d]. So Set Key As Default KeyStyle KEY_STRING.\n", Key_Style);
	}
	m_pRootList = NULL;
	m_pLestRecent = NULL;
	m_pMostRecent = NULL;
}


//---------------------------------------------------------------------------
// 函数:	KScriptSet::~KScriptSet
// 功能:	
// 返回:	
//---------------------------------------------------------------------------
KScriptSet::~KScriptSet()
{
	TScript * pTScript = NULL;
	TScript * pDelTScript = NULL;
	pTScript = m_pLestRecent;
	
	while(pTScript)
	{
		pDelTScript =  pTScript;
		pTScript = pTScript->pMoreRecent;
		DeleteScript(pDelTScript->szKey);
	}
	
}

//---------------------------------------------------------------------------
// 函数:	KScriptSet::GetScript
// 功能:	
// 参数:	char * szKey     //该脚本的关键字
// 参数:	BOOL nNeedCreate //当未在链表中找到时是否需要直接新建
// 参数:	BOOL nNeedUpdateRecent //是否需要更新Recent表，重新排序
// 返回:	KScript * 
//---------------------------------------------------------------------------
KScript * KScriptSet::GetScript(char * szKey, BOOL nNeedCreate = 1, BOOL nNeedUpdateRecent = 1)
{
	if (szKey == NULL ) return NULL;
	KScript * pScript = NULL;
	TScript * pTScript = NULL;
	TScript * pGetTScript = NULL;
	

	BOOL nResult = 0;

	if (m_nKeyStyle != KEY_STRING)
	{
		printf("The ScriptSet's Key Is Not a String. So Can Not Find A CorrRespond Script.\n");
		return NULL;
	}
	
	
	
	if (m_pRootList == NULL) 
	{
		if ((pScript = CreateScript(szKey, GetInitStackSize(szKey))) == NULL) 
			return NULL;
		
		pGetTScript = new TScript;
		if (pGetTScript == NULL) 
			return NULL;
		
		//		初始化Recent表
		pGetTScript->pLessRecent = NULL;
		pGetTScript->pMoreRecent = NULL;
		pGetTScript->pLeftChild = NULL;
		pGetTScript->pRightChild = NULL;
		pGetTScript->pScript = pScript;
		pGetTScript->pParent = NULL;
		
		strcpy(pGetTScript->szKey, szKey);
		m_pRootList = pGetTScript;
		m_pLestRecent = pGetTScript;
		m_pMostRecent = pGetTScript;
		
		return pScript;
		
	}
	
	pTScript = BTSearch(m_pRootList, m_pRootList, szKey, &nResult);
	
	if (nResult == 0)
	{
		if (nNeedCreate)//当查找不到时，是否需要建立一个
		{
			KScript * pNewScript;
			if ( (pNewScript = CreateScript(szKey, GetInitStackSize(szKey)) ) == NULL )
			{
				printf("Can not Create Script Instance ,Mayby No Memorys.\n");
				return NULL;
				
			}
			pGetTScript = BTInsert(pTScript, szKey);
			if (pGetTScript == NULL) return NULL;
			pGetTScript->pScript = pNewScript;
		}
		else 
			return NULL;//不需要建立的话，就返回空值
		
		
	}
	else
		pGetTScript = pTScript;
	
	if (nNeedUpdateRecent)	//如果需要更新Recent表，则更新
		UpdateRecent(nResult, pGetTScript);	
	
	return pGetTScript->pScript;
}

KScript * KScriptSet::GetScript(DWORD nKey, BOOL nNeedCreate = 1, BOOL nNeedUpdateRecent = 1)
{
	
	KScript * pScript = NULL;
	TScript * pTScript = NULL;
	TScript * pGetTScript = NULL;
	
	BOOL nResult = 0;
	
	if (m_nKeyStyle != KEY_NUMBER)
	{
		printf("The ScriptSet's Key Is Not a Number. So Can Not Find A CorrRespond Script.\n");
		return NULL;
	}

	if (m_pRootList == NULL) 
	{
		if ((pScript = CreateScript(nKey, GetInitStackSize(nKey))) == NULL) 
			return NULL;
		
		pGetTScript = new TScript;
		if (pGetTScript == NULL) 
			return NULL;
		
		//		初始化Recent表
		pGetTScript->pLessRecent = NULL;
		pGetTScript->pMoreRecent = NULL;
		pGetTScript->pLeftChild = NULL;
		pGetTScript->pRightChild = NULL;
		pGetTScript->pScript = pScript;
		pGetTScript->pParent = NULL;
		
		strcpy(pGetTScript->szKey, "");
		pGetTScript->nKey = nKey;

		m_pRootList = pGetTScript;
		m_pLestRecent = pGetTScript;
		m_pMostRecent = pGetTScript;
		
		return pScript;
		
	}
	
	pTScript = BTSearch(m_pRootList, m_pRootList, nKey, &nResult);
	
	if (nResult == 0)
	{
		if (nNeedCreate)//当查找不到时，是否需要建立一个
		{
			KScript * pNewScript;
			if ( (pNewScript = CreateScript(nKey, GetInitStackSize(nKey)) ) == NULL )
			{
				printf("Can not Create Script Instance ,Mayby No Memorys.\n");
				return NULL;
				
			}
			pGetTScript = BTInsert(pTScript, nKey);
			if (pGetTScript == NULL) return NULL;
			pGetTScript->pScript = pNewScript;
		}
		else 
			return NULL;//不需要建立的话，就返回空值
		
		
	}
	else
		pGetTScript = pTScript;
	
	if (nNeedUpdateRecent)	//如果需要更新Recent表，则更新
		UpdateRecent(nResult, pGetTScript);	
	
	return pGetTScript->pScript;
}

//---------------------------------------------------------------------------
// 函数:	KScriptSet::UpdateRecent
// 功能:	更新Recent表
// 参数:	BOOL bExistedScirpt  是否pGetTScript是个早已存在的脚本对象
// 参数:	TScript pGetTScript  最新使用的脚本对象
// 返回:	void 
//---------------------------------------------------------------------------
void KScriptSet::UpdateRecent(BOOL bExistedScript, TScript *pGetTScript)
{
	BOOL nResult  = bExistedScript;
	
	if (nResult == 0)//新生成的脚本
	{
		if (m_pRootList == NULL)//第一个结点
		{
			pGetTScript->pLessRecent = NULL;
			pGetTScript->pMoreRecent = NULL;
			m_pMostRecent = pGetTScript;
			m_pLestRecent = pGetTScript;
		}
		else
		{
			pGetTScript->pLessRecent = m_pMostRecent;
			pGetTScript->pMoreRecent = NULL;
			m_pMostRecent->pMoreRecent = pGetTScript;
			
		}
		
	}
	else //使用的是已存在的脚本
	{
		
		if (pGetTScript->pLessRecent == NULL && pGetTScript->pMoreRecent == NULL)
		{
			return ;
		}
		
		if (pGetTScript == m_pLestRecent)//该结点在链表的末尾
		{
			if (pGetTScript->pMoreRecent)//是否不只一个结点
			{
				
				m_pLestRecent = pGetTScript->pMoreRecent;//设置m_pLestRecent
				m_pLestRecent->pLessRecent = NULL;
				
				
				pGetTScript->pMoreRecent = NULL;
				pGetTScript->pLessRecent = m_pMostRecent;
				m_pMostRecent->pMoreRecent = pGetTScript;
				
			}
			
			
		}
		else if (pGetTScript == m_pMostRecent)//在头结点上
		{
			
		}
		else
		{
			
			pGetTScript->pMoreRecent->pLessRecent = pGetTScript->pLessRecent;
			pGetTScript->pLessRecent->pMoreRecent = pGetTScript->pMoreRecent;
			
			pGetTScript->pLessRecent = m_pMostRecent;
			pGetTScript->pMoreRecent = NULL;
			m_pMostRecent->pMoreRecent = pGetTScript;
			
		}
	}
	
	
	m_pMostRecent = pGetTScript;//设置m_pMostRecent
}




//---------------------------------------------------------------------------
// 函数:	KScriptSet::SearchScript
// 功能:	
// 参数:	char * szKey
// 返回:	BOOL 
//---------------------------------------------------------------------------
TScript *  KScriptSet::SearchScript(char * szKey, BOOL * pnResult)
{
	TScript * pTScript;
	if (m_pRootList == NULL)
		return NULL;
	
	pTScript = BTSearch(m_pRootList, m_pRootList, szKey, pnResult 	);
	return pTScript;
}

//---------------------------------------------------------------------------
// 函数:	 KScriptSet::SearchScript
// 功能:	
// 参数:	DWORD nKey
// 参数:	BOOL * pnResult
// 返回:	TScript * 
//---------------------------------------------------------------------------
TScript *  KScriptSet::SearchScript(DWORD nKey, BOOL * pnResult)
{
	TScript * pTScript;
	if (m_pRootList == NULL)
		return NULL;
	
	pTScript = BTSearch(m_pRootList, m_pRootList, nKey, pnResult);
	return pTScript;
}


//---------------------------------------------------------------------------
// 函数:	 KScriptSet::CreateScript
// 功能:	新建脚本实例，为虚函数，由派生类负责建立
// 参数:	char * szKey
// 参数:	int StackSize
// 返回:	KScript * 
//---------------------------------------------------------------------------
KScript *  KScriptSet::CreateScript(char * szKey , int StackSize)
{
	return NULL;
}

//---------------------------------------------------------------------------
// 函数:	 KScriptSet::CreateScript
// 功能:	
// 参数:	DWORD nKey
// 参数:	int StackSize
// 返回:	KScript * 
//---------------------------------------------------------------------------
KScript *  KScriptSet::CreateScript(DWORD nKey , int StackSize)
{
	return NULL;
}

//---------------------------------------------------------------------------
// 函数:	KScriptSet::DeleteScript
// 功能:	
// 参数:	char * szKey
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KScriptSet::DeleteScript(char * szKey)
{
	BOOL nResult = FALSE;//查找的结果
	TScript * pTScript;//查找返回的指针
	pTScript = SearchScript(szKey, &nResult);
	
	if (nResult && pTScript) //在链表中找到了该关键字
	{
		if (pTScript->pScript)
			pTScript->pScript->Exit();
		
		if (pTScript->pLessRecent == NULL  && pTScript->pMoreRecent == NULL)//全部为空，即只有一个结点
		{
			m_pMostRecent = NULL;
			m_pLestRecent = NULL;
			//if (m_pRootList == pTScript) 
			//	m_pRootList = NULL;
			return DeleteScript(pTScript);
		}
		
		else if (!(pTScript->pLessRecent && pTScript->pMoreRecent))//有一个为空
		{
			
			if (pTScript->pLessRecent)
			{
				pTScript->pLessRecent->pMoreRecent = NULL;
				
				if (m_pMostRecent == pTScript)
					m_pMostRecent = pTScript->pLessRecent;
				
				
			}
			else
			{
				pTScript->pMoreRecent->pLessRecent = NULL;
				
				if (m_pLestRecent == pTScript)
					m_pLestRecent = pTScript->pMoreRecent;
				
				
			}
			
			
			return DeleteScript(pTScript);
		}
		else//全实
		{
			pTScript->pLessRecent->pMoreRecent = pTScript->pMoreRecent;
			pTScript->pMoreRecent->pLessRecent = pTScript->pLessRecent;
			
			if (m_pMostRecent == pTScript)
				m_pMostRecent = pTScript->pLessRecent;
			
			if (m_pLestRecent == pTScript)
				m_pLestRecent = pTScript->pMoreRecent;
			
			return DeleteScript(pTScript);
		}
	}
	else
		return FALSE;
	
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KScriptSet::DeleteScript
// 功能:	
// 参数:	DWORD nKey
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KScriptSet::DeleteScript(DWORD nKey)
{
	BOOL nResult = FALSE;//查找的结果
	TScript * pTScript;//查找返回的指针
	pTScript = SearchScript(nKey, &nResult);
	
	if (nResult && pTScript) //在链表中找到了该关键字
	{
		if (pTScript->pScript)
			pTScript->pScript->Exit();
		
		if (pTScript->pLessRecent == NULL  && pTScript->pMoreRecent == NULL)//全部为空，即只有一个结点
		{
			m_pMostRecent = NULL;
			m_pLestRecent = NULL;
			
			return DeleteScript(pTScript);
		}
		
		else if (!(pTScript->pLessRecent && pTScript->pMoreRecent))//有一个为空
		{
			
			if (pTScript->pLessRecent)
			{
				pTScript->pLessRecent->pMoreRecent = NULL;
				
				if (m_pMostRecent == pTScript)
					m_pMostRecent = pTScript->pLessRecent;
				
				
			}
			else
			{
				pTScript->pMoreRecent->pLessRecent = NULL;
				
				if (m_pLestRecent == pTScript)
					m_pLestRecent = pTScript->pMoreRecent;
				
				
			}
			
			
			return DeleteScript(pTScript);
		}
		else//全实
		{
			pTScript->pLessRecent->pMoreRecent = pTScript->pMoreRecent;
			pTScript->pMoreRecent->pLessRecent = pTScript->pLessRecent;
			
			if (m_pMostRecent == pTScript)
				m_pMostRecent = pTScript->pLessRecent;
			
			if (m_pLestRecent == pTScript)
				m_pLestRecent = pTScript->pMoreRecent;
			
			return DeleteScript(pTScript);
		}
	}
	else
		return FALSE;
	
	return TRUE;
}



//---------------------------------------------------------------------------
// 函数:	KScriptSet::DeleteScript
// 功能:	
// 参数:	TScript * pTScript
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KScriptSet::DeleteScript(TScript * pTScript)
{
	
	if (pTScript == NULL)
		return FALSE;
	TScript * pDelTScript;
	
	pDelTScript = BTDelete(pTScript, &m_pRootList, m_nKeyStyle);
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	KScriptSet::InsertScript
// 功能:	
// 参数:	char * szKey
// 参数:	KScript * pScript
// 返回:	BOOL 
//---------------------------------------------------------------------------
TScript*  KScriptSet::InsertScript(char * szKey)
{
	BOOL nResult = 0;
	TScript * pTScript = NULL;
	//TScript * pNewTScript = NULL;
	pTScript = BTSearch(m_pRootList, m_pRootList, szKey, &nResult);
	
	if (nResult == 1)
	{
		printf("Have Exist This Key In TScript Sets\n");
		return NULL;
		
	}
	else if (nResult == 0)
		return BTInsert(pTScript, szKey);
	else return NULL;
}

//---------------------------------------------------------------------------
// 函数:	KScriptSet::InsertScript
// 功能:	
// 参数:	DWORD nKey
// 返回:	TScript*  
//---------------------------------------------------------------------------
TScript*  KScriptSet::InsertScript(DWORD nKey)
{
	BOOL nResult = 0;
	TScript * pTScript = NULL;
	//TScript * pNewTScript = NULL;
	pTScript = BTSearch(m_pRootList, m_pRootList, nKey, &nResult);
	
	if (nResult == 1)
	{
		printf("Have Exist This Key In TScript Sets\n");
		return NULL;
		
	}
	else if (nResult == 0)
		return BTInsert(pTScript, nKey);
	else return NULL;
}



//---------------------------------------------------------------------------
// 函数:	KScriptSet::GetScriptStackSize
// 功能:	
// 参数:	char * szKey
// 返回:	int 
//---------------------------------------------------------------------------
int KScriptSet::GetInitStackSize(char * szKey)
{
	return 0;
}

//---------------------------------------------------------------------------
// 函数:	KScriptSet::GetScriptStackSize
// 功能:	
// 参数:	DWORD nKey
// 返回:	int 
//---------------------------------------------------------------------------
int KScriptSet::GetInitStackSize(DWORD nKey)
{
	return 0;
}


//---------------------------------------------------------------------------
// 函数:	KScriptSet::ListScriptsKey
// 功能:	
// 返回:	void 
//---------------------------------------------------------------------------
DWORD  KScriptSet::ListScriptsKey()
{
	return BTPreorder(m_pRootList);
}

DWORD  KScriptSet::ListRecent(int order = 0)
{
	TScript * pTScript;
	DWORD nCount = 0;
	
	if (order == 0)
	{
		pTScript = m_pMostRecent;
		
		while(pTScript)
		{
			//	printf("%s\n",pTScript->szKey);
			pTScript = pTScript->pLessRecent;
			nCount ++;
		}
		return nCount;
		
	}
	else
	{
		pTScript = m_pLestRecent;
		
		while(pTScript)
		{
			//	printf("%s\n",pTScript->szKey);
			pTScript = pTScript->pMoreRecent;
			nCount ++;
		}
		return nCount;
		
	}
	
}


BOOL KScriptSet::Run(char * szKey)
{		
	return TRUE;
}

BOOL KScriptSet::Run(DWORD nKey)
{		
	return TRUE;
}


BOOL KScriptSet::RunFunction(char * szKey, char * szFuncName, char * szFormat, ...)
{
	return TRUE;
}


BOOL KScriptSet::RunFunction(DWORD nKey, char * szFuncName, char * szFormat, ...)
{
	return TRUE;
}


//---------------------------------------------------------------------------
// 函数:	BTSearch
// 功能:	查找是否存在指定的关键字的二叉树结点，如果存在则*pResult = 1,返回相应指针；
//			否则的话*pResult = 0,返回最接近的指针（该指针对今后插入有用）
// 参数:	TScript * pParentTScript 为递归调用传入父结点
// 参数:	TScript * pTScript
// 参数:	char * szKey	关键点
// 参数:	BOOL * pResult 指向是否找到该结点的标志
// 返回:	TScript * 
//---------------------------------------------------------------------------
TScript * BTSearch(TScript * pParentTScript, TScript * pTScript, char * szKey, BOOL * pResult)
{
	
	if (pTScript == NULL || pParentTScript == NULL)
	{
		*pResult = FALSE;
		return pParentTScript;
	}
	
	int nResult = strcmp(szKey, pTScript->szKey);
	
	if (nResult == 0)
	{
		*pResult = TRUE;
		return pTScript;
	}
	else if (nResult > 0)
		return BTSearch(pTScript, pTScript->pRightChild, szKey, pResult);
	else
		return BTSearch(pTScript, pTScript->pLeftChild, szKey, pResult);
	
}


//---------------------------------------------------------------------------
// 函数:	BTSearch
// 功能:	
// 参数:	TScript * pParentTScript
// 参数:	TScript * pTScript
// 参数:	DWORD nKey
// 参数:	BOOL * pResult
// 返回:	TScript * 
//---------------------------------------------------------------------------
TScript * BTSearch(TScript * pParentTScript, TScript * pTScript, DWORD nKey, BOOL * pResult)
{
	
	if (pTScript == NULL || pParentTScript == NULL)
	{
		*pResult = FALSE;
		return pParentTScript;
	}
	
		
	if (nKey == pTScript->nKey)
	{
		*pResult = TRUE;
		return pTScript;
	}
	else if (nKey > pTScript->nKey)
		return BTSearch(pTScript, pTScript->pRightChild, nKey, pResult);
	else
		return BTSearch(pTScript, pTScript->pLeftChild, nKey, pResult);
	
}



//---------------------------------------------------------------------------
// 函数:	BTInsert
// 功能:	以pTScrpt为父，插入以szKey为关键点的结点
// 参数:	TScript *pTScript 
// 参数:	char * szKey
// 返回:	TScript * 返回插入的结点
//---------------------------------------------------------------------------
TScript * BTInsert(TScript *pTScript, char * szKey)
{
	TScript * pNewTScript = new TScript;
	int nResult = 0;
	if (pNewTScript == NULL)
		return NULL;
	
	pNewTScript->pLeftChild = NULL;
	pNewTScript->pParent = NULL;
	pNewTScript->pRightChild = NULL;
	strcpy(pNewTScript->szKey,szKey);
	
	nResult = strcmp(szKey, pTScript->szKey);
	
	if (nResult == 0)
		return NULL;
	//根据大小确定左子还是右子
	if (nResult > 0)
	{
		pTScript->pRightChild = pNewTScript;
		pNewTScript->pParent = pTScript;
	}
	else 
	{
		pTScript->pLeftChild = pNewTScript;
		pNewTScript->pParent = pTScript;
	}
	return pNewTScript;
	
}

//---------------------------------------------------------------------------
// 函数:	BTInsert
// 功能:	
// 参数:	TScript *pTScript
// 参数:	DWORD nKey
// 返回:	TScript * 
//---------------------------------------------------------------------------
TScript * BTInsert(TScript *pTScript, DWORD nKey)
{
	TScript * pNewTScript = new TScript;
	int nResult = 0;
	if (pNewTScript == NULL)
		return NULL;
	
	pNewTScript->pLeftChild = NULL;
	pNewTScript->pParent = NULL;
	pNewTScript->pRightChild = NULL;
	pNewTScript->nKey = nKey;
	
		
	if (nKey == pTScript->nKey)
		return NULL;
	//根据大小确定左子还是右子
	if (nKey > pTScript->nKey)
	{
		pTScript->pRightChild = pNewTScript;
		pNewTScript->pParent = pTScript;
	}
	else 
	{
		pTScript->pLeftChild = pNewTScript;
		pNewTScript->pParent = pTScript;
	}
	return pNewTScript;
	
}




//---------------------------------------------------------------------------
// 函数:	 BTDelete
// 功能:	删除结点
// 参数:	TScript * pTScript
// 参数:	TScript ** ppRootTScript
// 返回:	TScript * 
//---------------------------------------------------------------------------
TScript *  BTDelete(TScript * pTScript, TScript ** ppRootTScript, int nKeyStyle)
{
	TScript * pFindTScript;
	if (pTScript == NULL)
		return NULL;
	
	if (pTScript->pLeftChild == NULL && pTScript->pRightChild == NULL)//该结点没有左右子
	{
		if (*ppRootTScript == pTScript)
		{
			*ppRootTScript = NULL;
		}
		else
		{
			int nResult ;
			
			if (nKeyStyle == KEY_STRING)
			{
				nResult = strcmp(pTScript->szKey, pTScript->pParent->szKey);
			}
			else
			{
				if (pTScript->nKey < pTScript->pParent->nKey)
					nResult = -1;
				else
					nResult = 1;

			}
			

			if (nResult < 0)//小于
				pTScript->pParent->pLeftChild = NULL;
			else
				pTScript->pParent->pRightChild = NULL;
			
		}
		
		delete pTScript;
		return NULL;
	}
	
	else if (!(pTScript->pLeftChild && pTScript->pRightChild))//只有单子时
	{
		if (pTScript == *ppRootTScript)
		{
			if (pTScript->pLeftChild)
			{
				*ppRootTScript = pTScript->pLeftChild;
				pTScript->pLeftChild->pParent = NULL;
			}
			else
			{
				*ppRootTScript = pTScript->pRightChild;
				pTScript->pRightChild->pParent = NULL;
			}
			delete pTScript;
			return	NULL;
		}
		

		int nResult ;
		
		if (nKeyStyle == KEY_STRING)
		{
			nResult = strcmp(pTScript->szKey, pTScript->pParent->szKey);
		}
		else
		{
			if (pTScript->nKey < pTScript->pParent->nKey)
				nResult = -1;
			else
				nResult = 1;
			
		}

		
		if (nResult < 0)//在父的左边
		{
			
			if (pTScript->pLeftChild)//只有左子
			{
				pTScript->pParent->pLeftChild = pTScript->pLeftChild;
				pTScript->pLeftChild->pParent = pTScript->pParent;
			}
			else
			{
				pTScript->pParent->pLeftChild = pTScript->pRightChild;
				pTScript->pRightChild->pParent = pTScript->pParent;
			}
			
			delete pTScript;
			
		}
		else 
		{
			if (pTScript->pLeftChild)//只有左子
			{
				pTScript->pParent->pRightChild = pTScript->pLeftChild;
				pTScript->pLeftChild->pParent = pTScript->pParent;
			}
			else
			{
				pTScript->pParent->pRightChild = pTScript->pRightChild;
				pTScript->pRightChild->pParent = pTScript->pParent;
			}
			delete pTScript;
			
		} 
		
		
		return NULL;
		
	}  
	else//有全子 
	{
		
		//if (strcmp(pTScript, pTScript->pParent) < 0)//在父的左面
		{
			
			pFindTScript = BTFindLess(pTScript->pLeftChild);
			
			if (pFindTScript)
			{
				
				//第一部分：处理该结点有左子时，对左子进行指向的改变
				//该结点仍有左子
				if (pFindTScript->pLeftChild)
				{
					//当发现所将要替代的结点正是它的左子时，将原来的关系不将变化；否则按正常思路改变
					if (pFindTScript != pTScript->pLeftChild)
					{
						pFindTScript->pParent->pRightChild = pFindTScript->pLeftChild;
						pFindTScript->pLeftChild->pParent = pFindTScript->pParent;
					}
				}
				else
				{
					if (pFindTScript != pTScript->pLeftChild)
					{
						pFindTScript->pParent->pRightChild = NULL;
					}
					
				}
				
				//第二部分：改变替换结点链结，实现与原结点相同。
				//处理当该替换的结点为删除结点的左子的特殊情况
				if (pFindTScript == pTScript->pLeftChild)
				{
					//其左子不用交代，保持原状
					pTScript->pRightChild->pParent = pFindTScript;
					
					//其左子不用交代,保持原状	
					pFindTScript->pRightChild = pTScript->pRightChild;
					pFindTScript->pParent = pTScript->pParent;
					
				}
				else
				{
					
					pTScript->pLeftChild->pParent = pFindTScript;
					pTScript->pRightChild->pParent = pFindTScript;
					
					
					pFindTScript->pLeftChild = pTScript->pLeftChild ; 
					pFindTScript->pRightChild = pTScript->pRightChild;
					pFindTScript->pParent = pTScript->pParent;
					
				}
				
				
				//第三部分   删除结点之父结点链结
				
				if (*ppRootTScript == pTScript)
				{
					*ppRootTScript = pFindTScript;
					pFindTScript->pParent = NULL;
				}
				else 
				{

					int nResult ;
					
					if (nKeyStyle == KEY_STRING)
					{
						nResult = strcmp(pTScript->szKey, pTScript->pParent->szKey);
					}
					else
					{
						if (pTScript->nKey, pTScript->pParent->nKey)
							nResult = -1;
						else
							nResult = 1;
						
					}


					if (nResult < 0)//在父的左面
					{
						pTScript->pParent->pLeftChild = pFindTScript;
					}
					else
					{
						pTScript->pParent->pRightChild = pFindTScript;
					}
				}
				
				
				delete pTScript;
				return pFindTScript;
			}
			else
				return NULL;
		}
	
	}	
	
	
}

//---------------------------------------------------------------------------
// 函数:	BTFindLess
// 功能:	
// 参数:	TScript * pTScript
// 返回:	TScript * 
//---------------------------------------------------------------------------
TScript * BTFindLess(TScript * pTScript)
{
	if (pTScript == NULL)
		return NULL;
	
	if (pTScript->pRightChild == NULL )
		return pTScript;
	else
		return BTFindLess(pTScript->pRightChild);
	
	
}

//---------------------------------------------------------------------------
// 函数:	BTPreorder
// 功能:	
// 参数:	TScript * pTScript
// 返回:	DWORD  
//---------------------------------------------------------------------------
DWORD  BTPreorder(TScript * pTScript)//中序遍历
{
	static DWORD nCount = 0;
	if (pTScript != NULL)
	{
		
		BTPreorder(pTScript->pLeftChild);
		//printf("nKey [%d%s\n", pTScript->szKey);
		nCount ++;
		BTPreorder(pTScript->pRightChild);
	}	
	return nCount;
	
}



