/*****************************************************************************************
//	界面--聊天窗口
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-27
------------------------------------------------------------------------------------------
*****************************************************************************************/
#pragma once

#include "../Elem/WndImage.h"
#include "../Elem/WndEdit.h"
#include "../Elem/WndButton.h"
#include "../Elem/WndScrollBar.h"
#include "../Elem/WndMessageListBox.h"
#include "../../../core/src/gamedatadef.h"
#include "../../../Represent/iRepresent/KRepresentUnit.h"

struct KUiPlayerItem;
struct KUiChatMessage;

class KUiFriendInterview : protected KWndImage
{
public:
	//----界面面板统一的接口函数----
	static KUiFriendInterview*	OpenWindow(KUiPlayerItem* pFriend);		//打开窗口，返回唯一的一个类对象实例
	static int					DoesHaveVisibleWnd();
	static void					CloseWindow();							//关闭窗口，同时可以选则是否删除对象实例
	static void					LoadScheme(const char* pScheme);		//载入界面方案
private:
	KUiFriendInterview();
	~KUiFriendInterview();
	int		Initialize();								//初始化
	void	CloseSelf();
	void	CloneTheme(KUiFriendInterview* pCopy);
	int		WndProc(unsigned int uMsg, unsigned int uParam, int nParam);//窗口函数
	void	PaintWindow();
	int		IsRecyclable();
	void	OnSend();
	void	PopupFaceMenu();
	void	SetFriendData(KUiPlayerItem* pFriend);
	static KUiFriendInterview*	FindWindow(KUiPlayerItem* pFriend, bool bCreate);
	static void					SetSelfName(const char* pszSelfName);
private:
	static KUiFriendInterview* m_pFirstSelf;
	KUiFriendInterview*		m_pNextSelf;
private:
	KWndMessageListBox	m_MsgList;
	KWndScrollBar		m_MsgScroll;
	KWndEdit512			m_InputWnd;
	KWndText32			m_FriendName;
	KWndButton			m_SendBtn, m_CloseBtn, m_ColorBtn, m_FaceBtn;

	KRColor				m_SelfTextColor;	//此窗口的自己说的话的文本颜色
	KUiPlayerItem		m_FriendData;		//好友的标记数据

	static char			m_szSelfName[32];	//自己的名字
	static int			m_nSelfNameLen;		//自己名字的长度
	static unsigned int	m_uLastTextColor;	//最后选择的文本颜色
};
