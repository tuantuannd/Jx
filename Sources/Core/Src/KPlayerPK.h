//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KPlayerPK.h
// Date:	2003.07.15
// Code:	边城浪子
// Desc:	PlayerPK Class
//---------------------------------------------------------------------------

#ifndef KPLAYERPK_H
#define KPLAYERPK_H

#define		PK_ANMITY_TIME					200	
#define		PK_ANMITY_CLOSE_TIME			10800

enum
{
	enumPK_ENMITY_STATE_CLOSE = 0,
	enumPK_ENMITY_STATE_TIME,
	enumPK_ENMITY_STATE_PKING,
	enumPK_ENMITY_STATE_NUM,
};

#ifdef _SERVER

class KPlayerPK
{
	friend class KPlayer;
private:
	int		m_nPlayerIndex;					// 指向对应的player数组的位置
	int		m_nNormalPKFlag;				// 正常PK开关 TRUE 打开，可以砍人  FALSE 关闭，不可以砍人
	int		m_nEnmityPKState;				// 仇杀PK状态 0 关闭 1 正在计时 2 仇杀正在进行，对应 enumPK_ANMITY_STATE
	int		m_nEnmityPKAim;					// 仇杀PK目标，player idx
	int		m_nEnmityPKTime;				// 仇杀PK状态为1正在计时，倒计时计数
	int		m_nEnmityPKLaunchFlag;			// 是否仇杀发起人
	int		m_nExercisePKFlag;				// 切磋模式PK开关 TRUE 打开，可以砍人  FALSE 关闭，不可以砍人
	int		m_nExercisePKAim;				// 切磋模式PK目标，player idx
	int		m_nPKValue;						// PK值
public:
	KPlayerPK();							// 构造函数
	void	Init(int nPlayerIdx);			// 初始化
	void	SetNormalPKState(BOOL bFlag);	// 设定正常PK状态
	BOOL	GetNormalPKState();				// 获得正常PK状态
	void	EnmityPKClose();				// 关闭仇杀PK
	BOOL	EnmityPKOpen(int nAim);			// 仇杀开始
	int		GetEnmityPKState();				// 获得仇杀状态
	int		GetEnmityPKAim();				// 获得仇杀目标
	BOOL	IsEnmityPKLauncher() {return m_nEnmityPKLaunchFlag;};
	void	ExercisePKClose();				// 关闭切磋PK
	BOOL	ExercisePKOpen(int nAim);		// 打开切磋PK
	int		GetExercisePKAim();				// 获得切磋目标
	int		GetExercisePKState() {return m_nExercisePKFlag;};
	void	SetPKValue(int nValue);			// 设定PK值
	int		GetPKValue();					// 获得PK值
	void	AddPKValue(int nAdd);			// 增加(或减少)PK值，PK值最多减少到0
	void	CloseAll();						// 关闭仇杀PK和切磋PK
	void	EnmityPKCountDown();			// 仇杀倒计时

	void	Active();
};

#endif

#ifndef _SERVER
class KPlayerPK
{
	friend class KPlayer;
private:
	int		m_nNormalPKFlag;				// 正常PK开关 TRUE 打开，可以砍人  FALSE 关闭，不可以砍人
	int		m_nEnmityPKState;				// 仇杀PK状态 0 关闭 1 正在计时 2 仇杀正在进行，对应 enumPK_ANMITY_STATE
	int		m_nEnmityPKAim;					// 仇杀PK目标，npc id
	int		m_nEnmityPKTime;				// 仇杀PK状态为1正在计时，倒计时计数
	char	m_szEnmityAimName[32];			// 仇杀目标名字
	int		m_nExercisePKFlag;				// 切磋模式PK开关 TRUE 打开，可以砍人  FALSE 关闭，不可以砍人
	int		m_nExercisePKAim;				// 切磋模式PK目标，npc id
	char	m_szExerciseAimName[32];		// 切磋目标名字
	int		m_nPKValue;						// PK值

public:
	void	Init();							// 初始化
	void	ApplySetNormalPKState(BOOL bFlag);// 向服务器申请打开、关闭正常PK状态
	void	SetNormalPKState(BOOL bFlag, BOOL bShowMsg = TRUE);	// 设定正常PK状态
	BOOL	GetNormalPKState();				// 获得正常PK状态
	void	ApplyEnmityPK(char *lpszName);	// 向服务器申请仇杀某人
	void	ApplyEnmityPK(int nNpcID);		// 向服务器申请仇杀某人
	void	SetEnmityPKState(int nState, int nNpcID = 0, char *lpszName = NULL);	// 设定仇杀PK状态
	int		GetEnmityPKState()	{return m_nEnmityPKState;}		// 获得仇杀PK状态
	int		GetEnmityPKAimNpcID()	{return m_nEnmityPKAim;}	// 获得仇杀PK目标npc id
	int		GetEnmityPKTime()	{return m_nEnmityPKTime;}		// 获得仇杀PK准备时间
	char*	GetEnmityPKAimName()	{return m_szEnmityAimName;}	// 获得仇杀PK目标名字
	void	EnmityPKCountDown();			// 仇杀倒计时
	void	SetExercisePKState(int nState, int nNpcID = 0, char *lpszName = NULL);	// 设定切磋状态
	int		GetExercisePKState()	{return m_nExercisePKFlag;}	// 获得切磋状态
	int		GetExercisePKAim()	{return m_nExercisePKAim;}		// 获得切磋目标npc id
	char*	GetExercisePKName()	{return m_szExerciseAimName;}	// 获得切磋目标名字
	void	SetPKValue(int nValue);			// 设定PK值
	int		GetPKValue()	{return m_nPKValue;}	// 获得PK值

	void	Active();
};
#endif

#endif
