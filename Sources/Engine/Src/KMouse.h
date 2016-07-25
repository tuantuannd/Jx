//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMouse.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMouse_H
#define KMouse_H
//---------------------------------------------------------------------------
#include "KTimer.h"
//---------------------------------------------------------------------------
class ENGINE_API KMouse
{
protected:
	RECT		m_WorkingRect;
	BYTE		m_WorkingMode;
	POINT		m_Cursor;
	BYTE		m_LButton;
	BYTE		m_RButton;
	BYTE		m_KeyState;
	KTimer		m_Timer;
	BOOL		m_bTimerStart;
	void		MakeMessage(int dx, int dy, BYTE lb, BYTE rb);
public:
	KMouse();
	void		UpdateState();
	void		SetPos(POINT* pt);
	void		GetPos(POINT* pt);
	void		SetWorkingRect(LPRECT pRect);
	void		SetWorkingMode(BYTE byMode){m_WorkingMode = byMode;};
	void		SetKeyState(BYTE KeyState){m_KeyState = KeyState;};
};
extern ENGINE_API KMouse* g_pMouse;
//---------------------------------------------------------------------------
#endif
