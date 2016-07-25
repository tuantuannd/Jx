#include "KWin32.h"
#include "KScriptList.h"

KLuaScript* KScriptList::GetScript(Lua_State * L)
{
	if (IsEmpty() || L == NULL)	return NULL;

	KNode * pNode = GetHead();
	while (pNode)
	{
		KScriptNode * pScriptNode;
		pScriptNode = (KScriptNode *)pNode;
		if (pScriptNode->pScript->m_LuaState == L)		return pScriptNode->pScript;
		
		pNode = pNode->GetNext();
	}
	return NULL;
}


void KScriptList::Activate()
{
	if (GetNodeCount() == 0)	return;
	KNode * pNode = NULL;
	
	pNode = GetHead();
	
	while (pNode!=NULL)
	{
		KStepLuaScript * pScript;
		KScriptNode * pScriptNode;
		pScriptNode = (KScriptNode *)pNode;
		pScript = (KStepLuaScript * )pScriptNode->pScript;
	
		if (pScript->Active())
		{
			KScriptNode *pDelNode = (KScriptNode*)pNode;
			pNode = pNode->GetNext();
			pDelNode->Remove();
			delete pDelNode;
			continue;
		}
		int count = lua_getgccount(pScript->m_LuaState);
		lua_setgcthreshold(pScript->m_LuaState, 200);
		pNode = pNode->GetNext();
	}

}
//SendMessage  向所有的脚本对象发送消息
void KScriptList::SendMessage(char * szMessageName, char * szData)
{
	KNode * pNode;
	if (this->GetNodeCount()== 0)
		return;
	pNode = this->GetHead();
	
	while (pNode!=NULL)
	{
		KStepLuaScript * pScript;
		KScriptNode * pScriptNode;
		pScriptNode = (KScriptNode *)pNode;
		pScript = (KStepLuaScript * )pScriptNode->pScript;
	//全局发送时，对象为0
		pScript->AddMessage(0,szMessageName, szData);
		pNode = pNode->GetNext();
	}
}
