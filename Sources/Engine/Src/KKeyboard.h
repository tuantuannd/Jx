//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KKeyboard.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KKeyboard_H
#define KKeyboard_H
//---------------------------------------------------------------------------
#define KEY_SHIFT			1
#define KEY_CTRL			2
#define KEY_ALT				4
//---------------------------------------------------------------------------
class ENGINE_API KKeyboard
{
protected:
	BYTE		m_NewBuf[256];
	BYTE		m_OldBuf[256];
	BYTE		m_FuncKey;
	void		MakeMessage(int nKey);
	void		MakeFuncKey();
public:
	KKeyboard();
	void		UpdateState();
	BOOL		IsDown(BYTE byKey);
	BYTE		GetFuncKey(){return m_FuncKey;};
};
extern ENGINE_API KKeyboard* g_pKeyboard;
//---------------------------------------------------------------------------
#endif /* keyboard_H */
