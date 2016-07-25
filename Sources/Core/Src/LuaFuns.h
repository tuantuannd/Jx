#ifndef LUAFUNS_H
#define LUAFUNS_H
#include "KCore.h"
#include "KScriptList.h"
#include "KScriptCache.h"

extern CORE_API TLua_Funcs GameScriptFuns[];
extern CORE_API int	g_GetGameScriptFunNum();

extern CORE_API TLua_Funcs WorldScriptFuns[] ;
extern CORE_API int	g_GetWorldScriptFunNum();

//脚本列表实例
//Questions:单机版剧情脚本列表在网络版暂用.
extern  KScriptList		g_StoryScriptList;	//单机版剧情脚本列表
#endif

