//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMouse.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Mouse Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KDDraw.h"
#include "KDInput.h"
#include "KMessage.h"
#include "KMouse.h"
#include "KWin32Wnd.h"

//---------------------------------------------------------------------------
ENGINE_API KMouse* g_pMouse = NULL;
//---------------------------------------------------------------------------
// 函数:	KMouse
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMouse::KMouse()
{
	g_pMouse = this;
	m_WorkingRect.left = 0;
	m_WorkingRect.top = 0;
	m_WorkingRect.right = WND_INIT_WIDTH;
	m_WorkingRect.bottom = WND_INIT_HEIGHT;
	m_WorkingMode = WINDOWMODE;
	m_Cursor.x = 0;
	m_Cursor.y = 0;
	m_LButton = 0;
	m_RButton = 0;
	m_KeyState = 0;
	m_bTimerStart = FALSE;
}

void KMouse::SetWorkingRect(LPRECT pRect)
{
	m_WorkingRect = *pRect;
}

//---------------------------------------------------------------------------
// 函数:	SetPos
// 功能:	设置鼠标位置
// 参数:	pt		鼠标位置点
// 返回:	void
//---------------------------------------------------------------------------
void KMouse::SetPos(POINT* pt)
{
	m_Cursor.x = pt->x;
	m_Cursor.y = pt->y;
}
//---------------------------------------------------------------------------
// 函数:	GetPos
// 功能:	取得鼠标位置
// 参数:	pt		鼠标位置点
// 返回:	void
//---------------------------------------------------------------------------
void KMouse::GetPos(POINT* pt)
{
	pt->x = m_Cursor.x;
	pt->y = m_Cursor.y;
}
//---------------------------------------------------------------------------
// 函数:	UpdateState
// 功能:	刷新鼠标状态
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMouse::UpdateState()
{
	int		dx = 0;
	int		dy = 0;
	BYTE	lb = 0;
	BYTE	rb = 0;
//	POINT	pt = {0, 0};

	if(g_pDirectInput->GetMouseState(&dx, &dy, &lb, &rb)==FALSE) 
		return;

	if (m_WorkingMode == FULLSCREEN)
	{
		m_Cursor.x = m_Cursor.x + dx * 2;
		m_Cursor.y = m_Cursor.y + dy * 2;
		if (m_Cursor.x < m_WorkingRect.left)
			m_Cursor.x = m_WorkingRect.left;
		if (m_Cursor.x > m_WorkingRect.right)
			m_Cursor.x = m_WorkingRect.right;
		if (m_Cursor.y < m_WorkingRect.top)
			m_Cursor.y = m_WorkingRect.top;
		if (m_Cursor.y > m_WorkingRect.bottom)
			m_Cursor.y = m_WorkingRect.bottom;
	}
	else
	{
		GetCursorPos(&m_Cursor);
		g_ScreenToClient(&m_Cursor);
	}

	// generate mouse message
	MakeMessage(dx, dy, lb, rb);
}
//---------------------------------------------------------------------------
// 函数:	MakeMessage
// 功能:	产生鼠标消息
// 参数:	dx		水平偏移
//			dy		垂直偏移
//			lb		左键状态
//			rb		右键状态
// 返回:	void
//---------------------------------------------------------------------------
void KMouse::MakeMessage(int dx, int dy, BYTE lb, BYTE rb)
{
	if ((dx == 0) && (dy == 0))
	{
		if (!m_bTimerStart)
		{
			m_bTimerStart = TRUE;
			m_Timer.Passed(0);
		}
		else if (m_Timer.Passed(500))
		{
			m_bTimerStart = FALSE;
			g_SendMessage(TM_MOUSE_STOP, m_Cursor.x, m_Cursor.y, m_KeyState);
		}
	}
	else
	{
		m_bTimerStart = FALSE;
		g_SendMessage(TM_MOUSE_MOVE, m_Cursor.x, m_Cursor.y, m_KeyState);
	}

	if (m_LButton != lb)
	{
		if (lb & 0x80)
		{
			g_SendMessage(TM_MOUSE_LBDOWN, m_Cursor.x, m_Cursor.y, m_KeyState);
		}
		else
		{
			g_SendMessage(TM_MOUSE_LBUP, m_Cursor.x, m_Cursor.y, m_KeyState);
		}
		m_LButton = lb;
	}
	else if (m_LButton & 0x80)
	{
		g_SendMessage(TM_MOUSE_LBMOVE, m_Cursor.x, m_Cursor.y, m_KeyState);
	}

	if (m_RButton != rb)
	{
		if (rb & 0x80)
		{
			g_SendMessage(TM_MOUSE_RBDOWN, m_Cursor.x, m_Cursor.y, m_KeyState);
		}
		else
		{
			g_SendMessage(TM_MOUSE_RBUP, m_Cursor.x, m_Cursor.y, m_KeyState);
		}
		m_RButton = rb;
	}
	else if (m_RButton & 0x80)
	{
		g_SendMessage(TM_MOUSE_RBMOVE, m_Cursor.x, m_Cursor.y, m_KeyState);
	}
}
//---------------------------------------------------------------------------
