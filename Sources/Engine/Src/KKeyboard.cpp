//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KKeyboard.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Keyboard Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KDInput.h"
#include "KMessage.h"
#include "KKeyboard.h"
//---------------------------------------------------------------------------
ENGINE_API KKeyboard* g_pKeyboard = NULL;
//---------------------------------------------------------------------------
// 函数:	KKeyboard
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KKeyboard::KKeyboard()
{
	g_pKeyboard = this;
	g_MemZero(m_NewBuf, sizeof(m_NewBuf));
	g_MemZero(m_OldBuf, sizeof(m_OldBuf));
}
//---------------------------------------------------------------------------
// 函数:	UpdateState
// 功能:	刷新键盘状态
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KKeyboard::UpdateState()
{
	int nKey;

	if(g_pDirectInput->GetKeyboardState(m_NewBuf)==FALSE)
	{
//		memset(m_NewBuf,0,256);
		return;
    }
	MakeFuncKey();

	for (nKey = 0x01; nKey <= 0x58; nKey++)
		MakeMessage(nKey);

	for (nKey = 0x9C; nKey <= 0xDC; nKey++)
		MakeMessage(nKey);
}
//---------------------------------------------------------------------------
// 函数:	IsDown
// 功能:	判断一个建是否被按下
// 参数:	byKey	键码
// 返回:	TRUE－按下 FALSE－抬起
//---------------------------------------------------------------------------
BOOL KKeyboard::IsDown(BYTE byKey)
{
	if(m_NewBuf[byKey]) 
     return TRUE;
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	MakeFuncKey
// 功能:	刷新功能键状态
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KKeyboard::MakeFuncKey()
{
	m_FuncKey = 0;

	if (m_NewBuf[DIK_LSHIFT] | m_NewBuf[DIK_RSHIFT])
	{
		m_FuncKey |= KEY_SHIFT;
	}

	if (m_NewBuf[DIK_LCONTROL] | m_NewBuf[DIK_RCONTROL])
	{
		m_FuncKey |= KEY_CTRL;
	}

	if (m_NewBuf[DIK_LALT] | m_NewBuf[DIK_RALT])
	{
		m_FuncKey |= KEY_ALT;
	}
}
//---------------------------------------------------------------------------
// 函数:	MakeMessage
// 功能:	发送键盘消息
// 参数:	nKey	键码
// 返回:	void
//---------------------------------------------------------------------------
void KKeyboard::MakeMessage(int nKey)
{
	if (m_OldBuf[nKey] != m_NewBuf[nKey])
	{
		if (m_NewBuf[nKey] & 0x80)
		{
			g_SendMessage(TM_KEY_DOWN, nKey, m_FuncKey);
		}
		else
		{
			g_SendMessage(TM_KEY_UP, nKey, m_FuncKey);
		}
		m_OldBuf[nKey] = m_NewBuf[nKey];
	}
	else
	{
		if (m_NewBuf[nKey] & 0x80)
		{
			g_SendMessage(TM_KEY_HOLD, nKey, m_FuncKey);
		}
	}
}
//---------------------------------------------------------------------------
