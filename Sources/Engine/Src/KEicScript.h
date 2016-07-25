//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
// 
// File:	KEicScript.h
// Date:	2001-11-6
// Code:	Daphnis
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KEicScript_H
#define KEicScript_H
//---------------------------------------------------------------------------
#include "KScript.h"
//---------------------------------------------------------------------------
class ENGINE_API KEicScript : public KScript
{
private:
	char	m_FileName[80];
public:
	virtual BOOL Init();
	virtual void Exit();
	virtual BOOL Load(char* FileName);
	virtual	BOOL Compile(char* FileName);
	virtual	BOOL Execute(int argc=0, char** argv=0);
	virtual BOOL RegisterFunction(LPSTR FuncName, void* Func);
};
//---------------------------------------------------------------------------
#endif // KEicScript_H
