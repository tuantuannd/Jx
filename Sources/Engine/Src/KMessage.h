//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMessage.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMessage_H
#define KMessage_H
//---------------------------------------------------------------------------
typedef struct {
	int	nType;		// 消息类型
	int	nParam[3];	// 消息参数
} KMessage;
//---------------------------------------------------------------------------
ENGINE_API void	g_SendMessage(int nType, int nP0=0, int nP1=0, int nP2=0);
ENGINE_API BOOL	g_PeekMessage(KMessage* pMsg);
ENGINE_API BOOL	g_GetMessage(KMessage* pMsg);
ENGINE_API void	g_ClearMessage();
//---------------------------------------------------------------------------
#define TM_NONE					0
//---------------------------------------------------------------------------
// Key Message
#define TM_KEY_BEGIN			10
#define TM_KEY_UP				TM_KEY_BEGIN + 1
#define TM_KEY_DOWN				TM_KEY_BEGIN + 2
#define TM_KEY_HOLD				TM_KEY_BEGIN + 3
#define TM_KEY_END				TM_KEY_BEGIN + 4
//---------------------------------------------------------------------------
// Mouse Message
#define TM_MOUSE_BEGIN			20
#define TM_MOUSE_LBUP			TM_MOUSE_BEGIN + 1
#define TM_MOUSE_LBDOWN			TM_MOUSE_BEGIN + 2
#define TM_MOUSE_LBMOVE			TM_MOUSE_BEGIN + 3
#define TM_MOUSE_RBUP			TM_MOUSE_BEGIN + 4
#define TM_MOUSE_RBDOWN			TM_MOUSE_BEGIN + 5
#define TM_MOUSE_RBMOVE			TM_MOUSE_BEGIN + 6
#define TM_MOUSE_MOVE			TM_MOUSE_BEGIN + 7
#define TM_MOUSE_STOP			TM_MOUSE_BEGIN + 8
#define TM_MOUSE_END			TM_MOUSE_BEGIN + 9

#endif
