//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KLuaScript.cpp
// Date:	2001-9-13 10:33:29
// Code:	Romandou
// Desc:	
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KPakFile.h"
#include "KLuaScript.h"
#include "LuaLib.h"
#include "KMemClass.h"
#include "KScript.h"
//---------------------------------------------------------------------------
// 函数:	KLuaScript::KLuaScript
// 功能:	
// 参数:	void
// 返回:	
//---------------------------------------------------------------------------
KLuaScript::KLuaScript(void)
{
	m_LuaState					= lua_open(100);

	if (m_LuaState == NULL)
	{
		ScriptError(LUA_CREATE_ERROR);
		m_IsRuning			= FALSE;
		return ;
	}

	m_IsRuning				= TRUE;
	m_szScriptName[0]		= '\0';
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::KLuaScript
// 功能:	
// 参数:	int StackSize
// 返回:	
//---------------------------------------------------------------------------
KLuaScript::KLuaScript(int StackSize )
{
	m_LuaState				= Lua_Create(StackSize);

	if (m_LuaState == NULL )
	{
		ScriptError(LUA_CREATE_ERROR);
		m_IsRuning = FALSE;
		return ;
	}
	m_IsRuning				= TRUE;
	m_szScriptName[0]		= '\0';
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::~KLuaScript
// 功能:	
// 参数:	void
// 返回:	
//---------------------------------------------------------------------------
KLuaScript::~KLuaScript(void)
{
	Exit();
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::LoadBuffer()
// 功能:	
// 参数:	PBYTE pBuffer
// 参数:	DWORD dwLen
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::LoadBuffer(PBYTE pBuffer, DWORD dwLen )
{
	if (dwLen < 0)	
	{
		ScriptError(LUA_SCRIPT_LEN_ERROR);
		return FALSE;
	}
	
	if (Lua_CompileBuffer(m_LuaState, (char *) pBuffer, dwLen, NULL) != 0)
	{
		ScriptError(LUA_SCRIPT_COMPILE_ERROR);
		return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	KLuaScript::Load
// 功能:	
// 参数:	LPSTR Filename
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::Load(char * Filename)
{
	KPakFile	File;
	DWORD		Size;

	
	// open file
	if (!File.Open(Filename))	return FALSE;
	
	// get file size
	Size = File.Size();
	
	KMemClass Memory;
	// alloc memory
	if (! Memory.Alloc(Size + 4))
		return FALSE;
	
	// read file
	if (File.Read(Memory.GetMemPtr(), Size) != Size)
		return FALSE;
	char * pszMem = (char *)Memory.GetMemPtr();
	pszMem[Size + 1] = 0;
	
	File.Close();
	try
	{
		if (!LoadBuffer((PBYTE)Memory.GetMemPtr(), Size ))
		{
			ScriptError(LUA_SCRIPT_COMPILE_ERROR);
			return FALSE;
		}
	}
	catch(...)
	{
		printf("Load Script %s 出现异常，请检查!!\n", Filename);
		return FALSE;
	}
		
	if (!ExecuteCode()) return FALSE;

	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::Execute
// 功能:	
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::Execute()
{
	if (m_IsRuning && m_LuaState)
	return CallFunction(MAINFUNCTIONNAME,0,"");
	
	return FALSE;
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::ExecuteCode
// 功能:	
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::ExecuteCode()
{
	if (!(m_IsRuning && m_LuaState))
	{
		ScriptError(LUA_SCRIPT_EXECUTE_ERROR);
		//if (!ExecuteCode()) return FALSE; ZHANGPENG 发现这里可能有错
		return FALSE;
	}
	
	int state;
	if (state = Lua_Execute(m_LuaState) != 0)
	{
		ScriptError(LUA_SCRIPT_EXECUTE_ERROR, state);
		return FALSE;
	}
	
	return	TRUE;
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::CallFunction
// 功能:	调用Lua脚本内的函数
// 参数:	LPSTR cFuncName
// 参数:	int nResults
// 参数:	LPSTR cFormat  调用时所传参数的类型 
//			n:数字型(double) d:整形(int) s:字符串型 f:C函数型  n:Nil v:Value p:Point
//        v形为Lua支持的，参数为整形的数index，指明将index所指堆栈的变量作为
//			 该函数的调用参数。
//	注意：由于该函数有不定参数…,对于数字，系统并不确定数是以double还是以int
//  存在，两种保存形式是不同的。因此需要注意当传入的数是整形时，格式符应用d
//  而不能用n,或者强行改变为double形。否则会出现计算的错误。
//   
// 参数:	...
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::CallFunction(LPSTR cFuncName, int nResults, LPSTR cFormat, va_list vlist)
{
	
	double nNumber;
	char * cString	= NULL;
	void * pPoint	= NULL;
	Lua_CFunction CFunc;
	int i=0;
	int nArgnum = 0;
	int nIndex = 0;
	int nRetcode;		//调用脚本函数后的返回码

	if (! (m_IsRuning && m_LuaState))
	{
		ScriptError(LUA_SCRIPT_STATES_IS_NULL);
		return FALSE;
	}
	
	{
		Lua_GetGlobal(m_LuaState, cFuncName); //在堆栈中加入需要调用的函数名

		while (cFormat[i] != '\0')
		{
			switch(cFormat[i])
			{
			case 'n'://输入的数据是double形 NUMBER，Lua来说是Double型
				{ 
					nNumber = va_arg(vlist, double );
					Lua_PushNumber(m_LuaState, nNumber);
					nArgnum ++;							

				}
				break;
			
			case 'd'://输入的数据为整形
				{
					nNumber = (double)(va_arg(vlist, int));
					Lua_PushNumber(m_LuaState, (double) nNumber);
					nArgnum ++;
				}
				break;
				
			case 's'://字符串型
				{
					cString = va_arg(vlist, char *);
					Lua_PushString(m_LuaState, cString);
					nArgnum ++;							
				}
				break;
			case 'N'://NULL
				{
					Lua_PushNil(m_LuaState);
					nArgnum ++;
				}
				break;
			
			case 'f'://输入的是CFun形，即内部函数形
				{
					CFunc = va_arg(vlist, Lua_CFunction);
					Lua_PushCFunction(m_LuaState, CFunc) ;
					nArgnum ++;
				}
				break;
			
			case 'v'://输入的是堆栈中Index为nIndex的数据类型
				{
					nNumber = va_arg(vlist, int);
					int nIndex1 = (int) nNumber;
					Lua_PushValue(m_LuaState, nIndex1);
					nArgnum ++;
				}
				break;
			case 't'://输入为一Table类型
				{
					
					

				}
				break;
			
			case 'p':
				{
					pPoint = va_arg(vlist, void *);

					Lua_PushUserTag(m_LuaState, pPoint,m_UserTag);
					nArgnum ++;
				}
				break;
			}
				
			i++;	
		}
		
	}  
    		
	nRetcode = Lua_Call(m_LuaState, nArgnum, nResults);
	
	if (nRetcode != 0)
	{
		ScriptError(LUA_SCRIPT_EXECUTE_ERROR, nRetcode);
		return FALSE;
	}
	

	return	TRUE;
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::CallFunction
// 功能:	
// 参数:	LPSTR cFuncName
// 参数:	int nResults
// 参数:	LPSTR cFormat
// 参数:	...
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::CallFunction(LPSTR cFuncName, int nResults, LPSTR cFormat, ...)
{
	BOOL bResult  = FALSE;
	va_list vlist;
	va_start(vlist, cFormat);
	bResult = CallFunction(cFuncName, nResults, cFormat, vlist);
	va_end(vlist);
	return bResult;
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::GetValuesFromStack
// 功能:	从堆栈中获得变量
// 参数:	char * cFormat
// 参数:	...
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::GetValuesFromStack(char * cFormat, ...)	
{
	va_list vlist;
	double* pNumber = NULL;
	const char **   pString ;
	int * pInt = NULL;
	int i = 0;
	int nTopIndex = 0;
	int nIndex = 0;
	int nValueNum = 0;//cFormat的字符长度，表示需要取的参数数量

	if (! m_LuaState)
		return FALSE;

	nTopIndex = Lua_GetTopIndex(m_LuaState);	
	nValueNum = strlen(cFormat);
	
	if (nTopIndex == 0 || nValueNum == 0)//当堆栈中无数据或不取参数是返回FALSE
		return FALSE;

	if (nTopIndex < nValueNum)
		return FALSE;

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
						return FALSE;

					if (Lua_IsNumber(m_LuaState, nIndex ))
					{
						* pNumber = Lua_ValueToNumber(m_LuaState, nIndex ++ );
												
					}
					else
					{
						ScriptError(LUA_SCRIPT_NOT_NUMBER_ERROR);
						return FALSE;
					}
					
					
				}
				break;
			case 'd':
				{
					pInt = va_arg(vlist, int *);
					if (pInt == NULL)
						return FALSE;
					if ( Lua_IsNumber(m_LuaState, nIndex))
					{
						* pInt = (int ) Lua_ValueToNumber(m_LuaState, nIndex ++);
					}
					else
					{
						ScriptError(LUA_SCRIPT_NOT_NUMBER_ERROR);
						return FALSE;
					}

				}
				break;
			case 's'://字符串形
				{
					pString = va_arg(vlist, const char **);
					
					if (pString == NULL)
						return FALSE;
					
					if (Lua_IsString(m_LuaState, nIndex))
					{
						(*pString) = (const char *)Lua_ValueToString(m_LuaState, nIndex++);
						
					}
					else
					{
						ScriptError(LUA_SCRIPT_NOT_STRING_ERROR);
						return FALSE;
					}
				}
				break;
			
			}
			
			
		i ++;	
		}
		va_end(vlist);     		/* Reset variable arguments.      */
		
	}
	return	TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::Init
// 功能:	初始化脚本对象，注册系统标准函数库
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::Init()
{
	if (! m_LuaState)
	{
		m_LuaState				= Lua_Create(0);
		
		if (m_LuaState == NULL)
		{
			ScriptError(LUA_CREATE_ERROR);
			m_IsRuning			= FALSE;
			return FALSE;
		}
		
		m_IsRuning				= TRUE;
		m_szScriptName[0]		= '\0';
		m_UserTag = lua_newtag(m_LuaState)	;
	}
	
	RegisterStandardFunctions();
	return	TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::RegisterFunction
// 功能:	注册某内部C函数至脚本中
// 参数:	LPSTR FuncName  在脚本中使用的函数名
// 参数:	void* Func    实际相应的C函数指针
// 参数:	int Args = 0 //与KScript接口相容，无用
// 参数:	int Flag = 0 //与KScript接口相容, 无用
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::RegisterFunction(LPSTR FuncName , void* Func)
{
	if (! m_LuaState)
		return FALSE;
	Lua_Register(m_LuaState, FuncName, (Lua_CFunction)Func);
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::Compile
// 功能:	
// 参数:	char *
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::Compile(char *)
{
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::RegisterFunctions
// 功能:	批量注册Lua的内部C函数，各个函数的信息保存在TLua_Funcs的数据中
// 参数:	TLua_Funcs *Funcs 数组的指针
// 参数:	int n 函数数量。可以为零，由系统计算得到。
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::RegisterFunctions(TLua_Funcs Funcs[], int n)
{
	if (! m_LuaState)	return FALSE;
	if (n == 0)	n = sizeof(Funcs) / sizeof(Funcs[0]);
	for (int i = 0; i < n; i ++)	Lua_Register(m_LuaState, Funcs[i].name, Funcs[i].func);
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::RegisterStandardFunctions
// 功能:	注册Lua系统标准的函数库
// 返回:	void 
//---------------------------------------------------------------------------
void KLuaScript::RegisterStandardFunctions()
{
	if (! m_LuaState)		return ;
	Lua_OpenBaseLib(m_LuaState);//Lua基本库
	Lua_OpenIOLib(m_LuaState);//输入输出库
	Lua_OpenStrLib(m_LuaState);//字符串处理库
	Lua_OpenMathLib(m_LuaState);//数值运算库
	//Lua_OpenDBLib(m_LuaState);//调试库
	return;	
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::ReleaseScript
// 功能:	释放该脚本资源。
// 返回:	BOOL 
//---------------------------------------------------------------------------
void KLuaScript::Exit()
{
	
	if (! m_LuaState)		return ;
	Lua_Release(m_LuaState);
	m_LuaState = NULL;
	m_IsRuning = FALSE;
	
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::ScriptError
// 功能:	
// 参数:	int Error
// 返回:	void 
//---------------------------------------------------------------------------
void KLuaScript::ScriptError(int Error)
{
	char lszErrMsg[200];
	sprintf(lszErrMsg, "ScriptError %d. (%s) \n", Error, m_szScriptName);
	lua_outerrmsg(lszErrMsg);
	return;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::ScriptError
// 功能:	
// 参数:	int Error1
// 参数:	int Error2
// 返回:	void 
//---------------------------------------------------------------------------
void KLuaScript::ScriptError(int Error1 ,int Error2)
{
	char lszErrMsg[200];
	sprintf(lszErrMsg, "ScriptError %d:[%d] (%s) \n", Error1, Error2, m_szScriptName);
	lua_outerrmsg(lszErrMsg);
	return;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::SafeCallBegin
// 功能:	
// 参数:	int * pIndex
// 返回:	void 
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// SafeCallBegin与SafeCallEnd两函数应搭配使用，以防止在调用Lua的外部函数之后，
//有多余数据在堆栈中未被清除。达到调用前与调用后堆栈的占用大小不变。
//上述情况只需用在调用外部函数时，内部函数不需如此处理。
//																	Romandou
//---------------------------------------------------------------------------
void KLuaScript::SafeCallBegin(int * pIndex)
{
	if (! m_LuaState)		return ;
	Lua_SafeBegin(m_LuaState, pIndex);
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::SafeCallEnd
// 功能:	
// 参数:	int nIndex
// 返回:	void 
//---------------------------------------------------------------------------
void KLuaScript::SafeCallEnd(int nIndex)
{
	if (! m_LuaState)	return;
	Lua_SafeEnd(m_LuaState, nIndex);
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::StopScript
// 功能:	中止脚本
// 参数:	void
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::Stop(void)
{
	if (! m_IsRuning)		return TRUE;
	if (! m_LuaState)		return FALSE;
	m_IsRuning =  FALSE;
	return TRUE;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::ResumeScript
// 功能:	恢复已中止的脚本
// 参数:	void
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL KLuaScript::Resume(void)
{
	if ((! m_IsRuning) && (m_LuaState))
	{
		m_IsRuning = TRUE;
		return TRUE;
	}
	return FALSE;
}


//---------------------------------------------------------------------------
// 函数:	KLuaScript::CreateTable
// 功能:	建立一个Lua的Table，在调用该函数并设置Table各个成员之后，必须调用
//			SetGlobalName()来给这个Table指定一个名字。
// 返回:	DWORD 
//---------------------------------------------------------------------------
DWORD KLuaScript::CreateTable()
{
	 int nIndex = 0;
	
	nIndex = Lua_GetTopIndex(m_LuaState) ;
	Lua_NewTable(m_LuaState);
	if (Lua_GetTopIndex(m_LuaState) != ++nIndex ) 
		return -1;

	return nIndex;
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::SetGlobalName
// 功能:	设置Lua堆栈顶部的数据一个名字
// 参数:	LPSTR szName
// 返回:	void 
//---------------------------------------------------------------------------
void KLuaScript::SetGlobalName(LPSTR szName)
{
	if (!szName) return ;
	Lua_SetGlobal(m_LuaState, szName);
}

//---------------------------------------------------------------------------
// 函数:	KLuaScript::ModifyTable
// 功能:	将指定名称的LuaTable置堆栈顶端，并返回顶端Index
// 参数:	LPSTR szTableName
// 返回:	DWORD 若Lua中不存在该Table则返回-1
//---------------------------------------------------------------------------
DWORD KLuaScript::ModifyTable(LPSTR szTableName) 
{
	if (! szTableName[0])		return -1;
	
	int nIndex = Lua_GetTopIndex(m_LuaState);
	
	Lua_GetGlobal(m_LuaState, szTableName);

	if (Lua_GetTopIndex(m_LuaState) != ++nIndex)		return -1;
	
	return nIndex;
}
