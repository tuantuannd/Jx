/*****************************************************************************************
//	外界访问服务版Core的接口方法定义
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-12-20
------------------------------------------------------------------------------------------
	外界（如界面系统）通过此接口从Core获取游戏世界数据。
*****************************************************************************************/

#ifndef CORESERVERSHELL_H
#define CORESERVERSHELL_H

#include "CoreServerDataDef.h"


//=========================================================
// Core外部客户对core的操作请求的索引定义
//=========================================================
enum SERVER_SHELL_OPERATION_INDEX
{
	SSOI_LAUNCH = 1,			//启动服务
	SSOI_SHUTDOWN,				//关闭服务
	SSOI_BROADCASTING,			//boardcasting
	//uParam = (char*)pMessage
	//nParam = (int)nMsgLen

	SSOI_RELOAD_WELCOME_MSG,	//reload the server's welcome msg

	SSOI_TONG_CREATE,			// relay 帮会创建成功，通知 core 进行相应的处理
	SSOI_TONG_REFUSE_ADD,		// 拒绝帮会加入申请
	SSOI_TONG_ADD,				// relay 帮会成员添加成功，通知 core 进行相应的处理
};

//=========================================================
// Core外部客户向core获取游戏数据的数据项内容索引定义
//=========================================================
//各数据项索引的相关参数uParam与nParam如果在注释中未提及，则传递定值0。
//如果特别指明返回值含义，则成功获取数据返回1，未成功返回0。
enum GAMEDATA_INDEX
{
	SGDI_CHARACTER_NAME,
	//uParam = (char*) buffer to retrive character name
	//nParam = Client index
	//Return = bIsExistCharacter
	SGDI_LOADEDMAP_ID,
	SGDI_CHARACTER_ACCOUNT,
	SGDI_CHARACTER_ID,
	SGDI_CHARACTER_NETID,
	SGDI_CHARACTER_EXTPOINT,
	SGDI_CHARACTER_EXTPOINTCHANGED,
	//uParam = (char*) id buffer 

	// 传入帮会建立参数，返回条件是否成立
	// uParam : struct STONG_SERVER_TO_CORE_APPLY_CREATE point
	// return : 条件是否成立
	SGDI_TONG_APPLY_CREATE,

	// 申请加入帮会
	// uParam : struct STONG_SERVER_TO_CORE_APPLY_ADD point
	SGDI_TONG_APPLY_ADD,

	// 判断加入帮会条件是否合适
	// uParam : 传入的 char point ，用于接收帮会名称
	// nParam : struct STONG_SERVER_TO_CORE_CHECK_ADD_CONDITION point
	SGDI_TONG_CHECK_ADD_CONDITION,

	// 获得帮会信息
	// uParam : 传入的 STONG_SERVER_TO_CORE_GET_INFO point
	SGDI_TONG_GET_INFO,

	// 判断是否有任命权利
	// uParam : 传入的 TONG_APPLY_INSTATE_COMMAND point
	// nParam : PlayerIndex
	SGDI_TONG_INSTATE_POWER,

	// 被任命，帮会数据变化
	// uParam : 传入的 STONG_SERVER_TO_CORE_BE_INSTATED point
	SGDI_TONG_BE_INSTATED,

	// 判断是否有踢人权利
	// uParam : 传入的 TONG_APPLY_KICK_COMMAND point
	// nParam : PlayerIndex
	SGDI_TONG_KICK_POWER,

	// 被踢出帮会
	// uParam : 传入的 STONG_SERVER_TO_CORE_BE_KICKED point
	SGDI_TONG_BE_KICKED,

	// 离开帮会判断
	// uParam : 传入的 TONG_APPLY_LEAVE_COMMAND point
	// nParam : PlayerIndex
	SGDI_TONG_LEAVE_POWER,

	// 离开帮会
	// uParam : 传入的 STONG_SERVER_TO_CORE_LEAVE point
	SGDI_TONG_LEAVE,

	// 能否传位判断
	// uParam : 传入的 TONG_APPLY_CHANGE_MASTER_COMMAND point
	// nParam : PlayerIndex
	SGDI_TONG_CHANGE_MASTER_POWER,

	// 能否接受传位判断
	// uParam : 传入的 STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER point
	SGDI_TONG_GET_MASTER_POWER,

	// 传位导致身份改变
	// uParam : 传入的 STONG_SERVER_TO_CORE_CHANGE_AS point
	SGDI_TONG_CHANGE_AS,

	// 帮主换了
	// uParam : 传入的 STONG_SERVER_TO_CORE_CHANGE_MASTER point
	SGDI_TONG_CHANGE_MASTER,

	// 获得帮会名字符串转换成的 dword
	// nParam : PlayerIndex
	SGDI_TONG_GET_TONG_NAMEID,

	// 登陆时候获得帮会信息
	// uParam : 传入的 STONG_SERVER_TO_CORE_LOGIN point
	SGDI_TONG_LOGIN,

	// 通知core发送某玩家的帮会信息
	// nParam : player index
	SGDI_TONG_SEND_SELF_INFO,
};

#ifdef _STANDALONE
class IClient;
#else
struct IClient;
#endif

#ifndef _STANDALONE
struct _declspec (novtable) iCoreServerShell
#else
struct iCoreServerShell
#endif
{
	virtual int  GetLoopRate() = 0;
	virtual void GetGuid(int nIndex, void* pGuid) = 0;
	virtual DWORD GetExchangeMap(int nIndex) = 0;
	virtual bool IsPlayerLoginTimeOut(int nIndex) = 0;
	virtual void RemovePlayerLoginTimeOut(int nIndex) = 0;
	virtual bool IsPlayerExchangingServer(int nIndex) = 0;
	virtual void ProcessClientMessage(int nIndex, const char* pChar, int nSize) = 0;
	virtual void ProcessNewClientMessage(IClient* pTransfer, DWORD dwFromIP, DWORD dwFromRelayID, int nPlayerIndex, const char* pChar, int nSize) = 0;
	virtual void SendNetMsgToTransfer(IClient* pClient) = 0;
	virtual void SendNetMsgToChat(IClient* pClient) = 0;
	virtual void SendNetMsgToTong(IClient* pClient) = 0;
	virtual void ProcessBroadcastMessage(const char* pChar, int nSize) = 0;
	virtual void ProcessExecuteMessage(const char* pChar, int nSize) = 0;
	virtual void ClientDisconnect(int nIndex) = 0;
	virtual void RemoveQuitingPlayer(int nIndex) = 0;
	virtual void* SavePlayerDataAtOnce(int nIndex) = 0;
	virtual bool IsCharacterQuiting(int nIndex) = 0;
	virtual bool CheckProtocolSize(const char* pChar, int nSize) = 0;
	virtual bool PlayerDbLoading(int nPlayerIndex, int bSyncEnd, int& nStep, unsigned int& nParam) = 0;
	virtual int  AttachPlayer(const unsigned long lnID, GUID* pGuid) = 0;
	virtual void GetPlayerIndexByGuid(GUID* pGuid, int* pnIndex, int* plnID) = 0;
	virtual void AddPlayerToWorld(int nIndex) = 0;
	virtual void* PreparePlayerForExchange(int nIndex) = 0;
	virtual void PreparePlayerForLoginFailed(int nIndex) = 0;
	virtual void RemovePlayerForExchange(int nIndex) = 0;
	virtual void RecoverPlayerExchange(int nIndex) = 0;
	virtual int  AddCharacter(int nExtPoint, int nChangeExtPoint, void* pBuffer, GUID* pGuid) = 0;
	virtual int	 AddTempTaskValue(int nIndex, const char* pData) = 0;
	//向游戏发送操作
	virtual int	 OperationRequest(unsigned int uOper, unsigned int uParam, int nParam) = 0;
	//获取连接状况
	virtual int	 GetConnectInfo(KCoreConnectInfo* pInfo) = 0;
//	virtual	BOOL ValidPingTime(int nIndex) = 0;
	//从游戏世界获取数据
	virtual int	 GetGameData(unsigned int uDataId, unsigned int uParam, int nParam) = 0;
	//日常活动，core如果要寿终正寝则返回0，否则返回非0值
	virtual int  Breathe() = 0;
	//释放接口对象
	virtual void Release() = 0;
	virtual void SetSaveStatus(int nIndex, UINT uStatus) = 0;
	virtual UINT GetSaveStatus(int nIndex) = 0;
	virtual BOOL GroupChat(IClient* pClient, DWORD FromIP, unsigned long FromRelayID, DWORD channid, BYTE tgtcls, DWORD tgtid, const void* pData, size_t size) = 0;
	virtual	void SetLadder(void* pData, size_t uSize) = 0;
	virtual BOOL PayForSpeech(int nIndex, int nType) = 0;
};

#ifndef CORE_EXPORTS

	//获取iCoreShell接口实例的指针
#ifndef __linxu
	extern "C" 
#endif
	iCoreServerShell* CoreGetServerShell();

#endif
#endif
