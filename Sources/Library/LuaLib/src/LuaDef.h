//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	LuaDef.h
// Date:	2001-9-27 11:40:28
// Code:	Romandou
// Desc:	
//---------------------------------------------------------------------------
#ifndef LUADEF_H
#define LUADEF_H
//---------------------------------------------------------------------------
#include "Lua.h"
#include "Lualib.h"

#define LUA_CREATE_ERROR 1
#define LUA_SCRIPT_LEN_ERROR 2
#define LUA_SCRIPT_COMPILE_ERROR 3
#define LUA_SCRIPT_EXECUTE_ERROR 4


#define LUA_SCRIPT_NOT_NUMBER_ERROR 10
#define LUA_SCRIPT_NOT_STRING_ERROR 11
#define LUA_SCRIPT_NOT_TABLE_ERROR 12

#define LUA_SCRIPT_STATES_IS_NULL 20



#define Lua_CFunction					lua_CFunction
#define Lua_State						lua_State
#define Lua_Create(nSize)				lua_open(nSize)
#define Lua_Release(L)					lua_close(L)
#define Lua_GetTopIndex(L)				lua_gettop(L)
#define Lua_SetTopIndex(L,nIndex)		lua_settop(L,nIndex)
#define Lua_PushValue(L,nIndex)			lua_pushvalue(L,nIndex)
#define Lua_RemoveValue(L,nIndex)		lua_remove(L,nIndex)
#define Lua_InsertValue(L,nIndex)		lua_insert(L,nIndex)
#define Lua_GetStackSpace(L)			lua_stackspace(L)
#define Lua_GetValueType(L,nIndex)		lua_type(L,nIndex)
#define Lua_GetTypeName(L,Tag)			lua_typename(L,Tag)
#define Lua_IsNumber(L,nIndex)			lua_isnumber(L,nIndex)
#define Lua_IsString(L,nIndex)			lua_isstring(L,nIndex)
#define Lua_IsCFunction(L,nIndex)		lua_iscfunction(L,nIndex)
#define Lua_IsTable(L,nIndex)			lua_istable(L,nIndex)
#define Lua_GetValueTag(L,nIndex)		lua_tag(L,nIndex)
#define Lua_IsEqual(L,index1,index2)	lua_equal(L,index1,index2)
#define Lua_IsLessThan(L,index1,index2)	lua_lessthan(L,index1,index2)
#define Lua_ValueToNumber(L,nIndex)		lua_tonumber(L,nIndex)
#define Lua_ValueToString(L,nIndex)		lua_tostring(L,nIndex)
#define Lua_GetStrLen(L,nIndex)			lua_strlen(L,nIndex)
#define Lua_ValueToCFunction(L,nIndex)	lua_tocfunction(L,nIndex)
#define Lua_ValueToUserData(L,nIndex)	lua_touserdata(L,nIndex)
#define Lua_ValueToPoint(L,nIndex)		lua_topointer(L,nIndex)

/*
** push functions (C -> stack)
*/

#define Lua_PushNil(L)					lua_pushnil(L)
#define Lua_PushNumber(L,Number)		lua_pushnumber(L,Number)
#define Lua_PushLString(L,LString,Len)	lua_pushlstring(L,LString,Len)
#define Lua_PushString(L,String)		lua_pushstring(L,String)
#define Lua_PushCClosure(L,Fun,N)		lua_pushcclosure(L,Fun,N)
#define Lua_PushUserTag(L,PVoid,Tag)	lua_pushusertag(L,PVoid,Tag)

#define Lua_GetGlobal(L,Valuename)		lua_getglobal(L,Valuename)
#define Lua_GetTable(L,nIndex)			lua_gettable(L,nIndex)
#define Lua_RawGet(L,nIndex)			lua_rawget(L,nIndex)
#define Lua_RawGetI(L,nIndex,n)			lua_rawgeti(L,nIndex,n)
#define Lua_GetGlobals(L)				lua_getglobals(L)
#define Lua_GetTagMethod(L,iTag,cEvent)	lua_gettagmethod(L,iTag,cEvent)
#define Lua_GetRef(L,iRef)				lua_getref(L,iRef)
#define Lua_NewTable(L)					lua_newtable(L)


#define Lua_SetGlobal(L,cName)			lua_setglobal(L,cName)
#define Lua_SetTable(L,nIndex)			lua_settable(L,nIndex)
#define Lua_RawSet(L,nIndex)			lua_rawset(L,nIndex)
#define Lua_RawSetI(L,nIndex,nNum)		lua_rawseti(L,nIndex,nNum)
#define Lua_SetGlobals(L)				lua_setglobals(L)
#define Lua_SetTagMethod(L,iTag,cEvent)	lua_settagmethod(L,iTag,cEvent)
#define Lua_SetRef(L,nLock)				lua_ref(L,nLock)

#define Lua_Call(L,nArgs,nResults)		lua_call(L,nArgs,nResults)
#define Lua_RawCall(L,nArgs,nResults)	lua_rawcall(L,nArgs,nResults)
#define	Lua_ExecuteFile(L,cFilename)	lua_dofile(L,cFilename)
#define Lua_ExecuteString(L,cString)	lua_dostring(L,cString)
#define Lua_ExecuteBuffer(L,pBuff,iSize,cname)	lua_dobuffer(L,pBuff,iSize,cname)

#define Lua_GetGCThreshold(L)			lua_getgcthreshold(L)
#define Lua_GetGCCount(L)				lua_getgccount(L)
#define Lua_SetGCThreshold(L,nThreshold)lua_setgcthreshold(L,nThreshold)


/*
** miscellaneous functions
*/

#define Lua_NewTag(L)					lua_newtag(L)
#define Lua_CopyTagMethods(L,nTagTo,nTagFrom)	lua_copytagmethods(L,nTagTo,nTagFrom)
#define Lua_SetTag(L,nTag)				lua_settag(L,nTag)
#define	Lua_Error(L,cError)				lua_error(L,cError)
#define Lua_UnRef(L,nRef)				lua_unref(L,nRef)
#define Lua_Next(L,nIndex)				lua_next(L,nIndex)
#define Lua_GetN(L,nIndex)				lua_getn(L,nIndex)
#define Lua_Concat(L,nNum)				lua_concat(L,nNum)
#define Lua_NewUserData(L,nSize)		lua_newuserdata(L,nSize)


#define Lua_OpenBaseLib(L)				 lua_baselibopen(L)
#define Lua_OpenIOLib(L)				 lua_iolibopen(L)
#define Lua_OpenStrLib(L)				 lua_strlibopen(L)
#define Lua_OpenMathLib(L)				 lua_mathlibopen(L)
#define Lua_OpenDBLib(L)				 lua_dblibopen(L)


/* 
** ===============================================================
** some useful macros
** ===============================================================
*/
#define Lua_Pop(L,nIndex)				lua_pop(L,nIndex)
#define Lua_Register(L,cfname,pFun)		lua_register(L,cfname,pFun)
#define Lua_PushUserData(L,UseData)	lua_pushuserdata(L,UseData)
#define Lua_PushCFunction(L,pFun)	lua_pushcfunction(L,pFun)
#define Lua_CloneTag(L,nTag)		lua_clonetag(L,nTag)
#define Lua_GetRegistry(L)			lua_getregistry(L)


#define Lua_CompileBuffer(L,pBUFF,nBuffSize,cBname)	lua_compilebuffer(L,pBUFF,nBuffSize,cBname)
#define Lua_CompileFile(L,cFilename)		lua_compilefile(L,cFilename)
#define Lua_Execute(L)				lua_execute(L)
#define Lua_SafeBegin(L,pIndex)		lua_gettopindex(L,pIndex)
#define Lua_SafeEnd(L,nIndex)		Lua_SetTopIndex(L,nIndex)


int Lua_SetTable_StringFromId(Lua_State * L, int nIndex, int Id, const char * szString);
int Lua_SetTable_DoubleFromId(Lua_State * L, int nIndex, int Id, double nNumber);
int Lua_SetTable_IntFromId(Lua_State * L, int nIndex, int Id, int nNumber);
int Lua_SetTable_CFunFromName(Lua_State * L, int nIndex, const char * szMemberName, Lua_CFunction CFun);
int Lua_SetTable_CFunFromId(Lua_State * L, int nIndex, int nId, Lua_CFunction CFun);
int Lua_SetTable_StringFromName(Lua_State * L, int nIndex, const char * szMemberName, char * szString);
int Lua_SetTable_IntFromName(Lua_State * L, int nIndex, const char * szMemberName, int Number);
int Lua_SetTable_DoubleFromName(Lua_State * L, int nIndex, const char * szMemberName, double Number);

int Lua_GetValuesFromStack(Lua_State * L, char * cFormat , ...);

//---------------------------------------------------------------------------
#endif //LUADEF_H
