/*****************************************************************************************
//	游戏的登陆连接功能逻辑模块
//	Copyright : Kingsoft 2002-2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-8-13
------------------------------------------------------------------------------------------
	状态机
    参看KLoginDef.h文件获得相关一些登陆相关信息。
*****************************************************************************************/
#pragma once
#include "LoginDef.h"
#include "../NetConnect/NetMsgTargetObject.h"

struct TProcessData;
struct tagNewDelRoleResponse;
struct tagNotifyPlayerLogin;

enum LOGIN_LOGIC_STATUS
{
	LL_S_IDLE = 0,					//空闲
	LL_S_WAIT_INPUT_ACCOUNT,		//等待传账号密码
	LL_S_ACCOUNT_CONFIRMING,		//等待账号密码验证
	LL_S_WAIT_ROLE_LIST,			//等待接收角色列表数据
	LL_S_ROLE_LIST_READY,			//角色列表就绪
	LL_S_CREATING_ROLE,				//正在新建角色
	LL_S_DELETING_ROLE,				//正在删除角色
	LL_S_WAIT_TO_LOGIN_GAMESERVER,	//等待登陆游戏服务器
	LL_S_ENTERING_GAME,				//正在进入游戏
	LL_S_IN_GAME,					//游戏运行时
};

enum LOGIN_LOGIC_RESULT_INFO
{
	LL_R_NOTHING,					//无结果信息
	LL_R_CONNECT_FAILED,			//连接失败
	LL_R_CONNECT_SERV_BUSY,			//服务器忙
	LL_R_CONNECT_TIMEOUT,			//连接超时未获得响应
	LL_R_ACCOUNT_PWD_ERROR,			//账号/密码错误
	LL_R_ACCOUNT_FREEZE,			//账号冻结
	LL_R_ACCOUNT_LOCKED,			//账号被锁定
	LL_R_INVALID_ROLENAME,			//(新建)角色的名字不合法
	LL_R_SERVER_SHUTDOWN,			//游戏服务器已满或正在维护中
	LL_R_INVALID_PROTOCOLVERSION,	//版本号较旧，需要升级到新的客户端	
	LL_R_INVALID_PASSWORD,			//（删除角色时）提供的密码错误

	LL_R_ACCOUNT_CONFIRM_SUCCESS,	//账号验证成功
	LL_R_CREATE_ROLE_SUCCESS,		//创建角色成功
	LL_R_LOGIN_TO_GAMESERVER,		//开始了与游戏世界服务器的连接
	LL_R_ACCOUNT_NOT_ENOUGH_POINT,	//账号点数不足

};

#ifndef MAX_PLAYER_PER_ACCOUNT
	#define	MAX_PLAYER_PER_ACCOUNT	3
#endif

//=====================================
//	新建角色时需要的角色相关信息结构
//=====================================
struct KRoleChiefInfo
{
	char				Name[32];		//姓名
	unsigned	char	Gender;			//性别	be one of the SSC_CHARACTER_GENDER value
	unsigned	char	Attribute;		//五行属性
	union
	{
		unsigned short	NativePlaceId;	//出生地ID
		short			nLevel;			//等级
	};
};

struct KLoginServer
{
	char			Title[32];		//服务器文字说明
	unsigned char	Address[4];		//服务器ip地址
};

class KLogin : public iKNetMsgTargetObject
{
public:
	KLogin();								//构造函数
	~KLogin();								//析构函数

	//====iKNetMsgTargetObject接口函数====
	void	AcceptNetMsg(void* pMsgData);	//接受网络消息

	//====操作函数，它们一般引发状态跃迁====
	//与（账号）服务器建立连接
	int	CreateConnection(const unsigned char* pAddress);
	//传入帐号密码，账号登陆
	int	AccountLogin(const char* pAccount, const KSG_PASSWORD& crPassword, bool bOrignPassword = true);
	//选中游戏角色
	int	SelectRole(int nIndex);
	//请求新建游戏角色
	int	CreateRole(KRoleChiefInfo* pCreateInfo);
	//请求删除游戏角色
	int	DeleteRole(int nIndex, const KSG_PASSWORD &crSupperPassword);
	//通知等待返回结果超时了
	void NotifyTimeout();
	//通知要开始游戏了
	void NotifyToStartGame();
	//通知网络连接（意外）断开了
	void NotifyDisconnect();

	//回到空闲状态
	void ReturnToIdle();
	//全程自动连接
	void AutoLogin();
	//判断是否可以执行全程自动连接
	int	IsAutoLoginEnable();
	//设置纪录标记
	void SetRememberAccountFlag(bool bEnable);
	//设置纪录标记
	void SetRememberAllFlag(bool bEnable);

	//====数据获取函数====
	//获取登陆逻辑当前的状态
	LOGIN_LOGIC_STATUS GetStatus() { return m_Status; }
	//获取操作的结果信息
	LOGIN_LOGIC_RESULT_INFO GetResult();
	//获取角色的数目
	int		GetRoleCount(int& nAdviceChoice);
	//获取某个角色的信息
	int		GetRoleInfo(int nIndex, KRoleChiefInfo* pInfo);
	//保存登陆选择
	void	SaveLoginChoice();
	//读取以前的的登陆选择
	void	LoadLoginChoice();
	//获取服务器区域的列表
	KLoginServer*	GetServerRegionList(int& nCount, int& nAdviceChoice);
	//登陆服务器列表获取
	KLoginServer*	GetServerList(int nRegion, int& nCount, int& nAdviceChoice);
	//获取建议（旧的）登陆账号
	bool	GetLoginAccount(char* pszAccount);
	//取得当前账号的剩余时间
	DWORD	GetAccountLifeTime() { return m_LeftTime; }
	//判断当前角色是否为新建的角色
	int		IsRoleNewCreated() { return m_Choices.bIsRoleNewCreated; }
    //设置选中的服务器，用来存盘
    int     SetAccountServer(const KLoginServer &rcSelectServer);

private:
	//====各操作的网络响应的返回处理====
	void	ProcessAccountLoginResponse(KLoginStructHead* pResponse);		//处理账号登陆的响应
	void	ProcessRoleListResponse(TProcessData* pResponse);				//角色列表返回
	void	ProcessDeleteRoleResponse(tagNewDelRoleResponse* pResponse);	//删除角色列表的响应
	void	ProcessCreateRoleResponse(tagNewDelRoleResponse* pResponse);	//新建角色列表的响应
	void	ProcessToLoginGameServResponse(tagNotifyPlayerLogin* pResponse);//准备登陆游戏服务器的响应

	//发送消息请求
	int		Request(const char* pszAccount, const KSG_PASSWORD* pcPassword, int nAction);
	//连接账号服务器
	int		ConnectAccountServer(const unsigned char* pIpAddress);

	void	RegistNetAgent();
	void	UnRegistNetAgent();
	void	GetAccountPassword(char* pszAccount, KSG_PASSWORD* pPassword);
	void	SetAccountPassword(const char* pszAccount, const KSG_PASSWORD* pcPassword);
	void	ClearAccountPassword(bool bAccount, bool bPassword);

private:
	LOGIN_LOGIC_STATUS			m_Status;
	LOGIN_LOGIC_RESULT_INFO		m_Result;
	
	bool						m_bInAutoProgress;				//是否正处于自动登陆过程中
	bool						m_bReserved;
	short						m_nNumRole;						//角色的数目
	KRoleChiefInfo				m_RoleList[MAX_PLAYER_PER_ACCOUNT];
	DWORD						m_LeftTime;

	struct	LOGIN_CHOICE
	{
		int						nServerRegionIndex;			//服务器区域索引
		KLoginServer			AccountServer;				//当前使用的服务器
		char					Account[32];				//当前账号
		KSG_PASSWORD    		Password;					//当前账号的密码
		char					szProcessingRoleName[32];	//当前处理的角色的名字
		bool					bRememberAccount;			//是否纪录登陆账号
		bool					bRememberAll;				//是否纪录全部的登陆选择
		bool					bAutoLoginEnable;			//是否允许自动登陆
		bool					bIsRoleNewCreated;			//当前角色是否为新建的角色
		bool					bLoaded;					//是否已加载选择纪录
	}							m_Choices;
};

extern	KLogin		g_LoginLogic;
