#ifndef KSCRIPTLIST_H
#define KSCRIPTLIST_H
#include "KEngine.h"
#include "KLuaScript.h"
#include "KList.h"
#include "KStepLuaScript.h"

class ENGINE_API KScriptNode : public KNode
{
public:
	 KLuaScript * pScript;
	DWORD			 nId;
	~KScriptNode()
	{
		delete pScript;
	}
};



class ENGINE_API KScriptList:public  KList
{
public:
	KLuaScript * GetScript(Lua_State * L);
	void Activate();
	void SendMessage(char * szMessageName, char * szData);
};
#endif
