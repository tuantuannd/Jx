//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KStepLuaScript.h
// Date:	2001-9-11 11:11:09
// Code:	Romandou
// Desc:	
//---------------------------------------------------------------------------
#ifndef KStepLuaScript_H
#define KStepLuaScript_H
//---------------------------------------------------------------------------
#include "KScript.h"
#include "KLuaScript.h"
#include "KMemClass.h"
#include "KMemClass1.h"
#include "KLubCmpl_Blocker.h"

#define MainBlockBeginString		"function main()"
#define MainBlockEndString			"end;--main"

struct TScriptMsg
{
	char * szMessage;
	char * szMsgData;
	Lua_State* StateAddr;
	TScriptMsg * NextMsg;
	
};

typedef enum
{
	ssRunIdle,	// 并未加载脚本
	ssRunMain,	// 脚本正在执行
	ssRunFunc,	// 函数正在执行
	ssRunResume, //暂时返回
	ssRunWaitMsg, //等待某个消息发生
}
RunStatus;


//---------------------------------------------------------------------------
class  ENGINE_API KStepLuaScript : public KLuaScript
{

public:
				KStepLuaScript();
	virtual		~KStepLuaScript();
				KStepLuaScript( int StackSize);
	
	int			Active();
	BOOL		GetNextLine(LPBYTE lpByte, char * szLin);
	BOOL		ExeLine(LPSTR szLine);
	BOOL		CheckLine(LPSTR szLine);//检查将执行的Lua语句是否符合条件，如不能有for goto 
	BOOL		GetExeBufferFromFile(char * filename);//获得当前文件中执行段的语句
	BOOL		GetExeBuffer(void *, int len);//获得执行段的语句
	BOOL		Load(LPSTR szFileName);
	void		SeekBegin(){m_CurPos = 0;	};
	void		SeekToExeBegin(){m_CurPos = m_FirstExecuteLine;};

	void		RunMain(){ m_Status = ssRunMain; };
	void		RunFunc(){ m_Status = ssRunFunc; };
	void		RunIdle(){	m_Status = ssRunIdle;};
	void		RunResume(){ m_Status = ssRunResume;};
	void		RunWaitMsg(){ m_Status = ssRunWaitMsg;	};
	BOOL		IsRunMain(){ return m_Status == ssRunMain; };
	BOOL		IsRunFunc(){ return m_Status == ssRunFunc; };
	BOOL		IsRunIdle(){ return m_Status == ssRunIdle;};
	BOOL		IsRunResume(){ return m_Status == ssRunResume;	};
	BOOL		IsRunWaitMsg(){return m_Status == ssRunWaitMsg;};
	
	KMemClass	m_Memory;
	BOOL		SendMessage(KStepLuaScript * pSendedScript, char * szMessageName, char * szData);
	BOOL		AddMessage(Lua_State * L, char * szMessageName, char * szData);
	LPSTR		GetWaitingMsg(){return m_szWaitingMsg;};
	
	void		PosUp();
	void		GotoLabel( LPSTR szLabelName);
	int			GetStatus(){return m_Status;	};

private:

	TScriptMsg  *m_pMsgQueue;
	RunStatus	m_Status;       // 脚本的状态
	BOOL		m_ScriptStyle; //该脚本所属类型是否为主类型   或附属类型
	long		m_CurLine;
	long		m_BufLen;
	long		m_CurPos;
	long		m_FirstExecuteLine;
	long		m_EndExecuteLine;
	char		m_szFilename[32];
	char		m_szWaitingMsg[40];
};
//---------------------------------------------------------------------------
#endif //KStepLuaScript_H
