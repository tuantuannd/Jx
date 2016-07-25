//---------------------------------------------------------------------------
// Sword3 Engine (c) 2002 by Kingsoft
//
// File:	KPlayerTeam.h
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Team Class
//---------------------------------------------------------------------------

#ifndef KPLAYERTEAM_H
#define KPLAYERTEAM_H

#include	"KPlayerDef.h"

#define		MAX_TEAM			MAX_PLAYER

#ifdef _SERVER
class KPlayerTeam	// 服务器端玩家的组队信息
{
	friend class KPlayer;
public:
	int		m_nFlag;			// 标志是否已经组队
	int		m_nID;				// 如果已经组队，队伍的 id
	int		m_nFigure;			// 如果已经组队，player 的身份：TEAM_CAPTAIN TEAM_MEMBER
	int		m_nApplyCaptainID;	// 申请加入的目标队伍的队长的 npc id
	int		m_nInviteList[MAX_TEAM_MEMBER];	// 邀请玩家加入本队伍列表
	int		m_nListPos;			// 列表当前位置

private:
	BOOL	m_bCanTeamFlag;

public:
	KPlayerTeam() {Release();};
	void	Release();
	BOOL	CreateTeam(int nIdx, PLAYER_APPLY_CREATE_TEAM *pCreateTeam);
	void	InviteAdd(int nIdx, TEAM_INVITE_ADD_COMMAND *pAdd);
	void	GetInviteReply(int nSelfIdx, int nTargetIdx, int nResult);

	void	SetCanTeamFlag(int nSelfIdx, BOOL bFlag);
	BOOL	GetCanTeamFlag() {return m_bCanTeamFlag;};
};
#endif

#ifndef _SERVER
class KTeamApplyList	// 申请加入队伍者列表中的申请人信息
{
public:
	DWORD	m_dwNpcID;			// 申请人 npc id
	DWORD	m_dwTimer;			// 申请时间计数器
	int		m_nLevel;			// 申请人等级
	char	m_szName[32];		// 申请人姓名
public:
	KTeamApplyList() {Release();};
	void	Release() {m_dwNpcID = 0; m_dwTimer = 0; m_nLevel = 0; m_szName[0] = 0;};
};				// 客户端保存在队长 player 身上的队伍申请人列表信息

class KPlayerTeam	// 客户端玩家的组队信息
{
public:
	int		m_nFlag;			// 标志是否已经组队
	int		m_nFigure;			// 如果已经组队，player 的身份：TEAM_CAPTAIN TEAM_MEMBER

	int		m_nApplyCaptainID;	// 申请加入的目标队伍的队长的 npc id
	DWORD	m_dwApplyTimer;		// 申请时间计数器（申请多久了，超过时间取消申请）
	BOOL	m_bAutoRefuseInviteFlag;// 是否自动拒绝别人的加入队伍的邀请 TRUE 自动拒绝   FALSE 手动
	KTeamApplyList	m_sApplyList[MAX_TEAM_APPLY_LIST];	// 如果为队长，队伍的申请人列表
public:
	KPlayerTeam();
	void	Release();
	void	ReleaseList();
	BOOL	ApplyCreate();//char *lpszTeamName);				// 申请创建一支队伍
	void	InviteAdd(DWORD dwNpcID);						// 邀请加入队伍
	void	ReceiveInvite(TEAM_INVITE_ADD_SYNC *pInvite);	// 收到邀请
	void	ReplyInvite(int nIdx, int nResult);				// 回复邀请
	void	SetAutoRefuseInvite(BOOL bFlag);				// 设定是否自动拒绝别人的加入队伍的邀请
	BOOL	GetAutoRefuseState();							// 获得是否自动拒绝别人的加入队伍的邀请状态
	int		GetInfo(KUiPlayerTeam *pTeam);					// 获得自身队伍信息（给界面）
	void	UpdateInterface();								// 更新界面显示
	void	DeleteOneFromApplyList(DWORD dwNpcID);			// 从申请人列表中删除某个申请人
};
#endif


// 此队伍是否为空通过判断队长 id 来决定，当为 -1 时为空（每个队伍必定有一个队长）
class KTeam
{
private:
	int		m_nIndex;									// 本 Team 在 g_Team 中的位置
public:
	int		m_nState;									// 队伍状态：Team_S_Open Team_S_Close
	int		m_nCaptain;									// 队长 id ，服务器端用 player index ，客户端用 npc id ，-1 为空
	int		m_nMember[MAX_TEAM_MEMBER];					// 所有队员 id ，服务器端用 player index ，客户端用 npc id ，-1 为空
	int		m_nMemNum;									// 已有队员数量(不包括队长)
#ifndef _SERVER
	int		m_nMemLevel[MAX_TEAM_MEMBER + 1];			// 队员等级 (第一个队员为队长) (队员升级的时候要通知所有其他队员 not end)
	char	m_szMemName[MAX_TEAM_MEMBER + 1][32];		// 队员名字 (第一个队员为队长)
	DWORD	m_nTeamServerID;								// 队伍在服务器上的唯一标识
#endif

public:
	KTeam();											// 构造函数
	void	Release();									// 清空
	void	SetIndex(int nIndex);						// 设定 Team 在 g_Team 中的位置
	BOOL	SetTeamOpen();								// 设定队伍状态：打开（允许接受新成员）
	BOOL	SetTeamClose();								// 设定队伍状态：关闭（不允许接受新成员）
	int		CalcCaptainPower();							// 计算队长能统帅队员的人数
	int		FindFree();									// 寻找队员空位
	int		FindMemberID(DWORD dwNpcID);				// 寻找具有指定npc id的队员（不包括队长）
#ifdef _SERVER
	BOOL	CreateTeam(int nPlayerIndex);//, char *lpszName);// 创建一支队伍
	BOOL	AddMember(int nPlayerIndex);				// 添加一个队伍成员
	BOOL	DeleteMember(int nPlayerIndex);				// 删除一个队伍成员
	BOOL	CheckFull();								// 判断队伍是否已经满员
	BOOL	IsOpen();									// 判断队伍是否是打开状态
	BOOL	CheckIn(int nPlayerIndex);					// 判断某人是否是队伍成员
#endif
#ifndef _SERVER
	// 客户端创建一支队伍（客户端只可能存在一支队伍，属于本地玩家）
	void	CreateTeam(int nCaptainNpcID, char *lpszCaptainName, int nCaptainLevel, DWORD nTeamServerID);
	BOOL	AddMember(DWORD dwNpcID, int nLevel, char *lpszNpcName);// 添加一个队伍成员
	void	DeleteMember(DWORD dwNpcID);				// 客户端删除一个队伍成员
	int		GetMemberInfo(KUiPlayerItem *pList, int nCount);	// 输出队员信息(按界面的要求，给界面)
#endif

private:
#ifdef _SERVER
	BOOL	CheckAddCondition(int nPlayerIndex);		// 判断加入队伍的属性条件是否合适
#endif

};

extern	KTeam	g_Team[MAX_TEAM];

#ifdef _SERVER
class KTeamSet
{
public:
	void	Init();										// 初始化
	int		CreateTeam(int nPlayerID);//, char *lpszName);	// 创建一支队伍
private:
	int		FindFree();									// 查找 g_Team 中的空位
//	BOOL	CheckName(char *lpszName);					// 判断队名是否可用
};

extern	KTeamSet	g_TeamSet;

#endif


#endif
