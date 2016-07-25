/*****************************************************************************************
//	存档角色选择
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-9-12

//	■■估计此模块代码量600-800行，尚需工时8-12小时。■■
------------------------------------------------------------------------------------------
	此模块用于获取玩家已经拥有的角色的信息，并可以进行添加/删除/载入角色等操作。

	应为此模块将连接服务器执行操作，所以调用方法LoadData、NewCharacter、DeleteCharacter、
SelCharacter，立即返回的都是(整数表示的布尔值)表示发送操作请求是否成功(0值表示失败，非0值
表示成功)，而不是操作的实际结果。通过GetLastActionResult方法可以知道最后一次操作的执行结果。

	关于此模块的状态：
	1 模块初始的时候是'空闲'状态(SSC_S_IDLE)。
	2 调用LoadData方法请求载入存档角色数据，载入角色数据时处于'载入角色数据中'状态(SSC_S_LOADING_DATA)，
	如果成功则切换为'角色已经载入'状态(SSC_S_STANDBY)，如果失败则返回'空闲'状态(SSC_S_IDLE)。
	3 处于SSC_S_STANDBY状态的时候可以执行NewCharacter、DeleteCharacter、SelCharacter操作。
	4 成功执行NewCharacter后转入'正在新建角色'状态(SSC_S_CREATING_CHARACTER)，操作结束后，
	无论成功与否都转入SSC_S_STANDBY状态。操作的执行结果通过GetLastActionResult方法获得。
	5 成功执行DeleteCharacter后转入'正在删除角色'状态(SSC_S_DELETING_CHARACTER)，操作结束后，
	无论成功与否都转入SSC_S_STANDBY状态。操作的执行结果通过GetLastActionResult方法获得。
	6 成功执行SelCharacter后转入'把角色载入游戏中'状态(SSC_S_LOADING_CHARACTER)，操作结束后，
	成功则转入'角色已经成功载入游戏'状态(SSC_S_LOAD_CHARACTER_LOADED)，失败泽转入SSC_S_STANDBY
	状态。
*****************************************************************************************/
#pragma once

#include "../../NetConnect/NetConnectAgent.h"

#ifndef MAX_PLAYER_PER_ACCOUNT
	#define	MAX_PLAYER_PER_ACCOUNT	3
#endif

//=====================================
//	KSelSavedCharacter的状态定义
//=====================================
enum SSC_STATUS
{
	SSC_S_IDLE	= 0,				//空闲状态
	SSC_S_LOADING_DATA,				//载入角色数据中
	SSC_S_STANDBY,					//角色已经载入
	SSC_S_LOADING_CHARACTER,		//把角色载入游戏中
	SSC_S_LOAD_CHARACTER_LOADED,	//角色已经成功载入游戏
	SSC_S_CREATING_CHARACTER,		//正在新建角色
	SSC_S_DELETING_CHARACTER,		//正在删除角色
};

//=====================================
//	KSelSavedCharacter的操作返回结果
//=====================================
enum SSC_RESULT
{
	SSC_R_IN_PROGRESS = 0,	//操作执行中
	SSC_R_NOTHING,			//操作完毕，没有变化
	SSC_R_UPDATE,			//更新
	SSC_R_INVALID_ROLENAME,	//新建的角色的名字不合法或以存在
	SSC_R_CREATE_ROLE_SUCCEED,//创建角色成功
	SSC_R_START_GAME,		//开始游戏
	SSC_R_SVR_DOWN,			//找不到有效的服务器
	SSC_R_FAILED,			//失败
};

//=====================================
//	新建角色时需要的角色相关信息结构
//=====================================
struct KNewCharacterInfo
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

class KSelSavedCharacter
{
public:
	KSelSavedCharacter();				//构造函数
	~KSelSavedCharacter();				//析构函数
	void	AcceptNetMsg(void* pMsgData);//接受网络消息
	int		LoadData();					//开始载入角色数据
	int		GetStatus();				//返回值为梅举SSC_STATUS的取值之一，返回值含义请看相关的值声明
	void	SetLastActionResult(int nResult);		//设置最后一次操作的返回结果
	int		GetLastActionResult();		//得到最后一次操作的返回结果
	int		GetCharacterNum();			//获取角色的数目
	int		GetCharacterInfo(int nIndex, KNewCharacterInfo* pInfo);	//获取某个角色的信息
	int		NewCharacter(KNewCharacterInfo* pData);					//请求新建一个角色
	int		DeleteCharacter(int nIndex, const char* pszPassword);	//请求删除一个角色
	int		SelCharacter(int nIndex);								//选择某个角色
	void	FreeData();												//释放角色数据
	void	SetCharacterBaseInfo(int nNum, const RoleBaseInfo *pInfo);
	void	SetAccountName(const char* pAccount);
private:
	SSC_STATUS			m_Status;
	RoleBaseInfo		m_BaseInfo[MAX_PLAYER_PER_ACCOUNT];
	int					m_nNumCharacter;	//角色的数目
	unsigned int		m_nRequestTime;		//发出情求的时间
	int					m_nLastOperResult;	//上次的操作返回结果
	char				m_AccountName[32];
	char				m_szProcessingRoleName[32];
};
