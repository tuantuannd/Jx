//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerChat.h
// Date:	2002.10.05
// Code:	边城浪子
// Desc:	PlayerChat Class
//---------------------------------------------------------------------------

#ifndef KPLAYERCHAT_H
#define KPLAYERCHAT_H

#include	"GameDataDef.h"
#ifndef _SERVER
#include	"KNode.h"
#include	"KList.h"
#endif

#ifdef _SERVER
#define		MAX_FRIEND_TEAM				1	// 最大好友分组数
#define		CHAT_SPEED					20	// 玩家多长时间能发一句话
#else
#define		MAX_FRIEND_TEAM				10	// 最大好友分组数
#endif
#define		CHAT_APPLY_ADD_FRIEND_LIST	10

enum	// 当前发送频道
{
	CHAT_CUR_CHANNEL_NONE = 0,				// 当前没有发送频道
	CHAT_CUR_CHANNEL_ALL,					// 所有同一服务器
	CHAT_CUR_CHANNEL_SCREEN,				// 当前九屏区域内
	CHAT_CUR_CHANNEL_SINGLE,				// 单聊
	CHAT_CUR_CHANNEL_TEAM,					// 所有对友
	CHAT_CUR_CHANNEL_FACTION,				// 同门派
	CHAT_CUR_CHANNEL_TONG,					// 同帮派
	CHAT_CUR_CHANNEL_SCREENSINGLE,			// 九屏区域内单聊
	CHAT_CUR_CHANNEL_SYSTEM,				// 系统频道，专用于系统给玩家发消息，必须订阅
	CHAT_CUR_CHANNEL_NUM,	// 频道数量
};

extern	DWORD	g_dwChannelAnd[CHAT_CUR_CHANNEL_NUM];
extern	DWORD	g_dwChannelOr[CHAT_CUR_CHANNEL_NUM];

#ifndef _SERVER
class KCHAT_RECORDER
{
private:
	char	m_szDay[11];
	char	m_szTime[9];

private:
	void		SetTime();

public:
	void		SaveSentenceSingle(char *lpszSelfName, char *lpszTarget, char *lpszTalkerName, char *lpszSentence);
	void		SaveSentenceChannel(char *lpszSelfName, char *lpszTalkerName, char *lpszSentence);

};
#endif

// 每个聊天好友的数据
class CChatFriend : public KNode
{
public:
	DWORD			m_dwID;					// 好友ID
	int				m_nPlayerIdx;			// 好友在 player 数组中的位置
	char			m_szName[32];			// 好友名字

public:
	CChatFriend() {Release();};
	void			Release() {m_dwID = 0; memset(m_szName, 0, 32);};
};

// 好友分组数据
class CChatFriendTeam
{
public:
	int				m_nFriendNo;			// 好友数量
	char			m_szTeamName[32];		// 组名
	KList			m_cEveryOne;			// 本组每一个好友的数据
public:
	CChatFriendTeam();
	void			Release();
#ifdef _SERVER
	void			DataBaseAddOne(int nSelfIdx, DWORD dwID, char *lpszName);		// 玩家登陆的时候从数据库获得数据时专用
#endif
};

#ifndef _SERVER
class CChatApplyListNode : public KNode
{
public:
	int		m_nPlayerIdx;
	int		m_nAccessFlag;
	char	m_szName[32];
	char	m_szInfo[MAX_SENTENCE_LENGTH];

public:
	CChatApplyListNode() { m_nPlayerIdx = 0; m_nAccessFlag = 0; m_szName[0] = 0; m_szInfo[0] = 0; };
};
#endif

#ifdef _SERVER
#include	<list>
#endif

class KPlayerChat
{
public:

#ifdef _SERVER
	int				m_nAddFriendList[CHAT_APPLY_ADD_FRIEND_LIST];	// 当前申请添加的好友 npc id 列表
	int				m_nListPos;										// m_nAddFriendList 的当前位置
	int				m_nSyncState;									// 玩家登陆时候发送好友数据的过程
	CChatFriend		*m_pStateSendPos;								// 玩家登陆时候发送好友数据时位置指针
	int				m_nTimer;										// 控制聊天刷屏速度
#endif

#ifndef _SERVER
	KList			m_cApplyAddList;					// 客户端申请人列表(链表)
	int				m_nCurChannel;						// 当前聊天频道
	BOOL			m_btIniLoadFlag;					// m_cTeamInfo 是否载入
	KIniFile		m_cTeamInfo;						// 客户端好友分组文件
	int				m_nLoginGetFirstOneFriendFlag;		// 玩家登录时接收到第一个好友信息
	KCHAT_RECORDER	m_cRecorder;
#endif

	DWORD			m_dwTakeChannel;					// 当前订阅频道
	DWORD			m_dwTargetID;						// 当前聊天对象ID 在不同的频道下会有不同的含义
	int				m_nTargetIdx;						// 当前聊天对象在服务器端的index
	CChatFriendTeam	m_cFriendTeam[MAX_FRIEND_TEAM];		// 好友分组列表

public:
	void			Release();
	BOOL			CheckExist(DWORD dwID);
	BOOL			CheckExist(char *lpszName);
	BOOL			GetName(DWORD dwID, char *lpszName);
	CChatFriend*	GetFriendNode(DWORD dwID);
	int				GetTeamNo(DWORD dwID);
	void			SetTakeChannel(DWORD dwChannel);			// 订阅聊天频道
	BOOL			CheckTalkChannel(int nChannel);

#ifdef _SERVER
	void			ServerSendChat(int nPlayerIdx, BYTE* pProtocol);// 服务器收到客户端发来的聊天信息
	void			DataBaseAddOne(int nSelfIdx, DWORD dwID, char *lpszName);// 玩家登陆的时候从数据库获得数据时专用
	void			AddFriendData(int nSelfIdx, int nDestIdx);		// 添加好友数据
	void			ResendOneFriendData(DWORD dwID, int nPlayerIdx);// 重新发送某个好友数据
	BOOL			SyncFriendData(int nPlayerIdx, unsigned int &nParam);// 玩家登录时加载数据库玩家好友数据完成后自动依次发送好友数据给客户端
	void			StartSendFriendData();							// 玩家登录时加载数据库玩家好友数据完成后调用次方法开始发送给客户端好友数据
	void			DeleteFriend(DWORD dwID, DWORD dwSelfID);		// 删除某个好友数据（会通知对方相应处理）
	void			DeleteFriendData(DWORD dwID, int nSelfIndex);	// 删除某个好友数据（会通知自己客户端）
	void			RedeleteFriend(DWORD dwID, int nSelfIndex);		// 再次删除某好友
	void			OffLine(DWORD dwSelfID);						// 本玩家下线
	void			GetMsgOffLine(DWORD dwID, int nSelfIdx);		// 得到通知有好友下线
	void			TimerAdd();										// 聊天时间计数加一
	// 发送系统消息 nType = 0 给全体玩家发送 nType == 1 给某个特定玩家发送
	static	void	SendSystemInfo(int nType, int nTargetIdx, char *lpszSendName, char *lpszSentence, int nSentenceLength);
	static	void	SendGlobalSystemInfo(char *lpszSendName, char *lpszSentence, int nSentenceLength);
	typedef std::list<std::string>	STRINGLIST;
	static	void	MakeBrother(const STRINGLIST& brothers);
	static	void	MakeEnemy(char* szPlayer, char* szEnemy);
	static	void	SendInfoToGM(char *lpszAccName, char *lpszRoleName, char *lpszSentence, int nSentenceLength);
	static	void	SendInfoToIP(DWORD nIP, DWORD nID, char *lpszAccName, char *lpszRoleName, char *lpszSentence, int nSentenceLength);
#endif

#ifndef _SERVER
	void			SetCurChannel(int nChannelNo, DWORD dwID = 0, int nIdx = 0);// 设定当前发送频道
	void			AddChannel(int nChannelNo);						// 增加订阅频道
	void			SubChannel(int nChannelNo);						// 取消订阅频道
	void			SendTakeChannel();								// 客户端发送订阅频道给服务器

	void			SendSentence(KUiMsgParam *pMsg, char *lpszSentence);// 客户端发送聊天语句给服务器
	void			QQSendSentence(DWORD dwID, int nIdx, KUiMsgParam *pMsg, char *lpszSentence);// 客户端发送聊天语句给服务器(QQ专用)

	void			GetChat(PLAYER_SEND_CHAT_SYNC *pChat);			// 客户端收到聊天语句

	void			ApplyAddFriend(DWORD dwNpcID, char *lpszSentence = NULL);// 申请添加聊天好友
	void			ApplyDeleteFriend(DWORD dwID, int nTeamNo, char *lpszSelfName);// 客户端申请删除某好友

	BOOL			GetApplyOneInfo(int *lpnPlayerIdx, char *lpszName, char *lpszInfo);// 界面获取聊天好友添加申请人信息

	void			SaveTeamInfo(char *lpszName);					// 保存所有好友分组信息
	int				CheckTeamNo(char *lpszName, char *lpszGetName);	// 判断某个ID的好友在哪个组里
	void			LoadTeamName(char *lpszPlayerName);				// 载入组名(初始化的时候调用，其他时候不允许调用)

	void			AddOne(DWORD dwID, int nPlayerIdx, char *lpszName, int nTeamNo);// 添加一个成员
	BOOL			CheckIsFriend(int nPlayerIdx);					// 判断某个playeridx是否好友
	int				AddTeam(char *lpszPlayerName, char *lpszTeamName);// 添加组，返回第几组，if < 0 失败
	BOOL			RenameTeam(int nTeamNo, char *lpszTeamName, char *lpszPlayerName);// 组改名
	BOOL			DeleteTeam(int nTeamNo, char *lpszPlayerName);	// 删除组，返回成功与否
	BOOL			OneFriendChangeTeam(DWORD dwID, int nSrcTeam, int nDestTeam);// 把一个好友从一个组移动到另一个组

	void			GetMsgOffLine(DWORD dwID);						// 得到通知有好友下线

	BOOL			CheckTakeChannel(int nChannelNo);				// 判断是否订阅了某个频道
	void			SaveTakeChannelInfo(char *lpszName);			// 保存频道订阅信息
	void			LoadTakeChannelInfo(char *lpszName);			// 载入频道订阅信息

	// 用于界面
	int				SendSelectChannelList(KUiChatChannel *pList, int nCount);	// 发送可选择聊天频道列表给界面
	int				SendTakeChannelList(KUiChatChannel *pList, int nCount);		// 发送可订阅聊天频道列表给界面
	BOOL			GetCurChannel(KUiChatChannel *pList);			// 获得当前使用聊天频道信息
	int				IGetTeamInfo(int nTeamNo, KUiChatGroupInfo *psTeamInfo);	// 获得某个分组信息，返回下一个有效组编号(如果是最后一个有效组，返回-1)
	int				IGetTeamFriendInfo(int nTeamNo, KUiPlayerItem *pList);	// 获得某个分组的所有成员的信息
#endif

};

#endif

