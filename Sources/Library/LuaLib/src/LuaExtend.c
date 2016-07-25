//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	LuaExtend.cpp
// Date:	2001-9-27 15:46:40
// Code:	Romandou
// Desc:	扩展的LuaAPI函数
//---------------------------------------------------------------------------
#include "Lua.h"
#include "string.h"
#include <stdio.h>
#include <stdarg.h>

//---------------------------------------------------------------------------
// 函数:	Lua_SetTable
// 功能:	修改Lua table中的某个成员的值
// 参数:	Lua_State * L
// 参数:	int nIndex 数组在堆中的index
// 参数:	const char * szMemberName 以字符串表示成员的名称
// 参数:	double Number 成员的数值
// 返回:	int 
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//                   Lua_SetTableMember
//    该函数体由若干的函数组成，以实现改变Lua Table中的某个变量的值
//    Lua的Table的表现形式有两种，一种是以数字形式表示如 Table[1],而另一
//	  种是以字符串的形式表示，如Table.Member。	
//	  同时，变量值还包括Number、String、CFun等
//	 	  nIndex为数组在堆栈中的位置。
//---------------------------------------------------------------------------
int Lua_SetTable_DoubleFromName(Lua_State * L, int nIndex, const char * szMemberName, double Number)
{
	
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushString(L, szMemberName);
	Lua_PushNumber(L, Number);
	Lua_SetTable(L, nIndex);
	return 1;
	
}


//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_IntFromName
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	const char * szMemberName
// 参数:	int Number
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_IntFromName(Lua_State * L, int nIndex, const char * szMemberName, int Number)
{

	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushString(L, szMemberName);
	Lua_PushNumber(L, (double)Number);
	Lua_SetTable(L, nIndex);
	return 1;
	
}


//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_StringFromName
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	const char * szMemberName
// 参数:	char * szString
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_StringFromName(Lua_State * L ,int nIndex, const char * szMemberName, char * szString)
{
	
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushString(L, szMemberName);
	Lua_PushString(L, szString);
	Lua_SetTable(L, nIndex);
	return 1;
	
}

//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_StringFromId
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	int Id
// 参数:	const char * szString
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_StringFromId(Lua_State * L, int nIndex, int Id, const char * szString)
{
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushNumber(L, (double)Id);
	Lua_PushString(L, szString);
	Lua_SetTable(L, nIndex);
	return 1;
}


//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_DoubleFromId
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	int Id
// 参数:	double nNumber
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_DoubleFromId(Lua_State * L , int nIndex, int Id, double nNumber)
{
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushNumber(L, (double) Id);
	Lua_PushNumber(L, nNumber);
	Lua_SetTable(L, nIndex);
	return 1;
}

//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_IntFromId
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	int Id
// 参数:	int nNumber
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_IntFromId(Lua_State * L, int nIndex, int Id, int nNumber)
{
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushNumber(L, (double) Id);
	Lua_PushNumber(L, (double) nNumber);
	Lua_SetTable(L, nIndex);
	return 1;
}

//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_CFunFromName
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	const char * szMemberName
// 参数:	Lua_CFunction CFun
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_CFunFromName(Lua_State * L, int nIndex, const char * szMemberName, Lua_CFunction CFun)
{
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushString(L, szMemberName);
	Lua_PushCFunction(L, CFun);
	Lua_SetTable(L, nIndex);
	return 1;
}

//---------------------------------------------------------------------------
// 函数:	Lua_SetTable_CFunFromId
// 功能:	
// 参数:	Lua_State * L
// 参数:	int nIndex
// 参数:	int nId
// 参数:	Lua_CFunction CFun
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_SetTable_CFunFromId(Lua_State * L, int nIndex, int nId, Lua_CFunction CFun)
{
	if (! Lua_IsTable(L, nIndex))
		return 0;
	Lua_PushNumber(L, nId);
	Lua_PushCFunction(L, CFun);
	Lua_SetTable(L, nIndex);
	return 1;
}


//---------------------------------------------------------------------------
// 函数:	Lua_GetValuesFromStack
// 功能:	
// 参数:	Lua_State * L
// 参数:	char * cFormat
// 参数:	...
// 返回:	int 
//---------------------------------------------------------------------------
int Lua_GetValuesFromStack(Lua_State * L, char * cFormat, ...)	
{

	va_list vlist;
	double* pNumber;
	 const char  *pString;
	   const char ** pStrPoint;
	
	int i = 0;
	int nTopIndex = 0;
	int nIndex = 0;
	int nValueNum = 0;//cFormat的字符长度，表示需要取的参数数量
	
	
	if (! L)
		return 0;
	
	nTopIndex = Lua_GetTopIndex(L);	
	nValueNum = strlen(cFormat);
	
	if (nTopIndex == 0 || nValueNum == 0)//当堆栈中无数据或不取参数是返回FALSE
		return 0;
	
	if (nTopIndex < nValueNum)
		nIndex = 1 ;
	else
		nIndex = nTopIndex - nValueNum +1;
	
	{
		va_start(vlist, cFormat);     
		
		while (cFormat[i] != '\0')
		{
			
			switch(cFormat[i])
			{
			case 'n'://返回值为数值形,Number,此时Lua只传递double形的值
				{
					pNumber = va_arg(vlist, double *);
					
					if (pNumber == NULL)
						return 0;
					
					if (Lua_IsNumber(L, nIndex ))
					{

						* pNumber = Lua_ValueToNumber(L, nIndex ++ );
						
					}
					else
					{
					//	ScriptError(LUA_SCRIPT_NOT_NUMBER_ERROR);
						return 0;
					}
					
					
				}
				break;
			case 's'://字符串形
				{
					pStrPoint = va_arg(vlist, const char **);
					
					if (pStrPoint == NULL)
						return 0;
					
					if (Lua_IsString(L, nIndex))
					{
						pString = Lua_ValueToString(L, nIndex++);
					
						*pStrPoint = pString;
						
					}
					else
					{
					//	ScriptError(LUA_SCRIPT_NOT_STRING_ERROR);
						return 0;
					}
				}
				break;
				
			}
			
			

			i ++;	
		}
		va_end(vlist);     		/* Reset variable arguments.      */
		
	}
	return	1;
}



