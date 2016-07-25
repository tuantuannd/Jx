//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KScript.h
// Date:	2001-9-11 10:33:35
// Code:	Romandou,Daphnis
// Desc:	脚本引擎接口
//---------------------------------------------------------------------------
#ifndef KScript_H
#define KScript_H
//---------------------------------------------------------------------------
class ENGINE_API KScript
{
public:
	KScript();
	virtual ~KScript();
	virtual BOOL Init();
	virtual void Exit();
	virtual BOOL Load(char* FileName);
	virtual	BOOL Compile(char* FileName);
	virtual	BOOL Execute();
	virtual	BOOL CallFunction(LPSTR cFuncName, int nResults, LPSTR cFormat,...);
	virtual BOOL RegisterFunction(LPSTR FuncName, void* Func);
	
};
//---------------------------------------------------------------------------
#endif  //KScript_H
