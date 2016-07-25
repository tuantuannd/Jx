/*****************************************************************************************
//	外界访问Core server 接口方法
//	Copyright : Kingsoft 2002
//	Author	:   Wooy (Wu yue)
//	CreateTime:	2002-12-20
------------------------------------------------------------------------------------------
*****************************************************************************************/
#include "KCore.h"
#include "CoreServerShell.h"
#include "KThread.h"
#include "KPlayer.h"
#include "KItemList.h"
#include "KSubWorldSet.h"
#include "KProtocolProcess.h"
#include "KNewProtocolProcess.h"
#include "KPlayerSet.h"
#include "KLadder.h"

#ifdef _STANDALONE
#include "KLadder.cpp"
#endif

//#include "KNetServer.h"
//#include "../MultiServer/Heaven/Interface/iServer.h"
#ifdef _STANDALONE
#include "IClient.h"
#else
#include "../../lib/S3DBInterface.h"
#include "../../Headers/IClient.h"
#include "../../../Headers/IClient.h"
#include "../../../Headers/KGmProtocol.h"
#endif

#include "LuaFuns.h"
#include "KSortScript.h"
#include "KSubWorld.h"


#include "malloc.h"

class CoreServerShell : public iCoreServerShell
{
public:
	int  GetLoopRate();
	void GetGuid(int nIndex, void* pGuid);
	DWORD GetExchangeMap(int nIndex);
	bool IsPlayerLoginTimeOut(int nIndex);
	void RemovePlayerLoginTimeOut(int nIndex);
	bool IsPlayerExchangingServer(int nIndex);
	void ProcessClientMessage(int nIndex, const char* pChar, int nSize);
	void ProcessNewClientMessage(IClient*, DWORD, DWORD, int nIndex, const char* pChar, int nSize);
	void SendNetMsgToTransfer(IClient* pClient);
	void SendNetMsgToChat(IClient* pClient);
	void SendNetMsgToTong(IClient* pClient);
	void ProcessBroadcastMessage(const char* pChar, int nSize);
	void ProcessExecuteMessage(const char* pChar, int nSize);
	void ClientDisconnect(int nIndex);
	void RemoveQuitingPlayer(int nIndex);
	void* SavePlayerDataAtOnce(int nIndex);
	bool IsCharacterQuiting(int nIndex);
	bool CheckProtocolSize(const char* pChar, int nSize);
	bool PlayerDbLoading(int nPlayerIndex, int bSyncEnd, int& nStep, unsigned int& nParam);
	int  AttachPlayer(const unsigned long lnID, GUID* pGuid);
	void GetPlayerIndexByGuid(GUID* pGuid, int* pnIndex, int* plnID);
	void AddPlayerToWorld(int nIndex);
	void* PreparePlayerForExchange(int nIndex);
	void PreparePlayerForLoginFailed(int nIndex);
	void RemovePlayerForExchange(int nIndex);
	void RecoverPlayerExchange(int nIndex);
	int  AddCharacter(int nExtPoint, int nChangeExtPoint, void* pBuffer, GUID* pGuid);
	int	 AddTempTaskValue(int nIndex, const char* pData);
	//向游戏发送操作
	int	 OperationRequest(unsigned int uOper, unsigned int uParam, int nParam);
	//获取连接状况
	int	 GetConnectInfo(KCoreConnectInfo* pInfo);
	//BOOL ValidPingTime(int nIndex);
	//从游戏世界获取数据
	int	 GetGameData(unsigned int uDataId, unsigned int uParam, int nParam);
	//日常活动，core如果要寿终正寝则返回0，否则返回非0值
	int  Breathe();
	//释放接口对象
	void Release();
	void SetSaveStatus(int nIndex, UINT uStatus);
	UINT GetSaveStatus(int nIndex);

	BOOL GroupChat(IClient* pClient, DWORD FromIP, unsigned long FromRelayID, DWORD channid, BYTE tgtcls, DWORD tgtid, const void* pData, size_t size);
	void SetLadder(void* pData, size_t uSize);
	BOOL PayForSpeech(int nIndex, int nType);
private:
	int	 OnLunch(LPVOID pServer);
	int	 OnShutdown();
};

static CoreServerShell	g_CoreServerShell;

CORE_API void g_InitCore();

#ifndef _STANDALONE
extern "C" __declspec(dllexport)
#endif
iCoreServerShell* CoreGetServerShell()
{
	g_InitCore();
	return &g_CoreServerShell;
}

void CoreServerShell::Release()
{
	g_ReleaseCore();
}

int CoreServerShell::GetLoopRate()
{
	return g_SubWorldSet.m_nLoopRate;
}

	//获取连接状况
int	 CoreServerShell::GetConnectInfo(KCoreConnectInfo* pInfo)
{
	if (pInfo)
		pInfo->nNumPlayer = PlayerSet.GetPlayerNumber();
	return 1;
}


int CoreServerShell::AddCharacter(int nExtPoint, int nChangeExtPoint, void* pBuffer, GUID* pGuid)
{
	int nIdx = 0;
	const TRoleData* pData = (const TRoleData*)pBuffer;

	if (pData && pData->BaseInfo.szName[0])
	{
		nIdx = PlayerSet.Add((char*)pData->BaseInfo.szName, pGuid);
		if (nIdx <= 0 || nIdx >= MAX_PLAYER)
			return 0;
		strcpy(Player[nIdx].m_AccoutName, pData->BaseInfo.caccname);
		strcpy(Player[nIdx].m_PlayerName, pData->BaseInfo.szName);
		DWORD	dwLen = pData->dwDataLen;
//		_ASSERT(dwLen < 64 * 1024);
		ZeroMemory(Player[nIdx].m_SaveBuffer, sizeof(Player[nIdx].m_SaveBuffer));
		memcpy(Player[nIdx].m_SaveBuffer, pBuffer, dwLen);

		Player[nIdx].m_pStatusLoadPlayerInfo = Player[nIdx].m_SaveBuffer;
		// 扩展点，用于活动
		Player[nIdx].SetExtPoint(nExtPoint, nChangeExtPoint);
		return nIdx;
	}
	return 0;
}

bool CoreServerShell::PlayerDbLoading(int nPlayerIndex, int bSyncEnd, int& nStep, unsigned int& nParam)
{
	TRoleData* pData = (TRoleData *)Player[nPlayerIndex].m_pStatusLoadPlayerInfo;
	
	if (bSyncEnd)
	{
		Player[nPlayerIndex].m_pStatusLoadPlayerInfo = NULL;
		nStep = 0;
		nParam = 0;

		return true;
	}
	else if (pData)	
	{
//		if (0 == Player[nPlayerIndex].LoadDBPlayerInfo((BYTE *)pData, nStep, nParam))
//		{
//			// 把玩家的登入状态设置为未登入，等待时延自动清除
//			Player[nPlayerIndex].m_nNetConnectIdx = -1;
//			Player[nPlayerIndex].m_dwLoginTime = -1;
//			return false;
//		}
//		else
//			return true;
		return Player[nPlayerIndex].LoadDBPlayerInfo((BYTE *)pData, nStep, nParam);
	}
	return false;
}

void CoreServerShell::AddPlayerToWorld(int nIndex)
{
//	int nIndex = PlayerSet.FindClient(lnID);
	Player[nIndex].LaunchPlayer();
}

void CoreServerShell::ProcessClientMessage(int nIndex, const char* pChar, int nSize)
{
	PlayerSet.ProcessClientMessage(nIndex, pChar, nSize);
}

void CoreServerShell::ProcessNewClientMessage(IClient* pTransfer,
									   DWORD dwFromIP, DWORD dwFromRelayID,
									   int nPlayerIndex,
									   const char* pChar, int nSize)
{
	g_NewProtocolProcess.ProcessNetMsg(pTransfer, dwFromIP, dwFromRelayID,
										nPlayerIndex, (BYTE*)pChar, nSize);

}

void CoreServerShell::SendNetMsgToTransfer(IClient* pClient)
{
	g_NewProtocolProcess.SendNetMsgToTransfer(pClient);
}

void CoreServerShell::SendNetMsgToChat(IClient* pClient)
{
	g_NewProtocolProcess.SendNetMsgToChat(pClient);
}

void CoreServerShell::SendNetMsgToTong(IClient* pClient)
{
	g_NewProtocolProcess.SendNetMsgToTong(pClient);
}

void CoreServerShell::ProcessBroadcastMessage(const char* pChar, int nSize)
{
	g_NewProtocolProcess.BroadcastLocalServer(pChar, nSize);
}

void CoreServerShell::ProcessExecuteMessage(const char* pChar, int nSize)
{
	g_NewProtocolProcess.ExecuteLocalServer(pChar, nSize);
}

void CoreServerShell::ClientDisconnect(int nIndex)
{
//	PlayerSet.Remove(nClient);
	PlayerSet.PrepareRemove(nIndex);
}

void CoreServerShell::RemoveQuitingPlayer(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;

	if (Player[nIndex].IsWaitingRemove())
	{
		PlayerSet.RemoveQuiting(nIndex);
	}
}
//--------------------------------------------------------------------------
//	功能：从游戏世界获取数据
//	参数：unsigned int uDataId --> 表示获取游戏数据的数据项内容索引，其值为梅举类型
//							GAMEDATA_INDEX的取值之一。
//		  unsigned int uParam  --> 依据uDataId的取值情况而定
//		  int nParam --> 依据uDataId的取值情况而定
//	返回：依据uDataId的取值情况而定。
//--------------------------------------------------------------------------
int	CoreServerShell::GetGameData(unsigned int uDataId, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uDataId)
	{
	case SGDI_CHARACTER_ACCOUNT:
		if (uParam)
		{
			nRet = PlayerSet.GetPlayerAccount(nParam, (char *)uParam);
			if (nRet == FALSE)
				((char *)uParam)[0] = 0;
		}
		break;
	case SGDI_CHARACTER_NAME:
		if (uParam)
		{
			nRet = PlayerSet.GetPlayerName(nParam, (char*)uParam);
			if (nRet == FALSE)
				((char *)uParam)[0] = 0;
		}
		break;
	case SGDI_CHARACTER_EXTPOINTCHANGED:
		if (uParam)
		{
			if (uParam >= MAX_PLAYER)
			{
				nRet = 0;
				break;
			}
			nRet = Player[uParam].GetExtPointChanged();
		}
		break;
	case SGDI_CHARACTER_EXTPOINT:
		if (uParam)
		{
			if (uParam >= MAX_PLAYER)
			{
				nRet = 0;
				break;
			}
			nRet = Player[uParam].GetExtPoint();
		}
		break;
	case SGDI_LOADEDMAP_ID:
		if (uParam)
		{
			int i;
			int nMax = nParam;
			if(nMax < MAX_SUBWORLD) nMax = MAX_SUBWORLD;
			for (i = 0; i < nMax; i++)
			{
				if (SubWorld[i].m_SubWorldID != -1)
				{
					((char *)uParam)[i] = (char)SubWorld[i].m_SubWorldID;
				}
				else
				{
					nRet = i;
					break;
				}
			}
		}
		break;
	case SGDI_CHARACTER_ID:
		if (uParam)
		{
			if (uParam >= MAX_PLAYER)
			{
				nRet = 0;
				break;
			}

			nRet = Player[uParam].m_dwID;
			break;
		}
		break;
	case SGDI_CHARACTER_NETID:
		if (uParam)
		{
			if (uParam >= MAX_PLAYER)
			{
				nRet = -1;
				break;
			}
			nRet = Player[uParam].m_nNetConnectIdx;
		}
		break;

	// 传入帮会建立参数，返回条件是否成立
	// uParam : struct STONG_SERVER_TO_CORE_APPLY_CREATE point
	// return : 条件是否成立
	case SGDI_TONG_APPLY_CREATE:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_APPLY_CREATE	*pApply = (STONG_SERVER_TO_CORE_APPLY_CREATE*)uParam;

			int		nPlayerIdx;
			int		nCamp;
			char	szTongName[defTONG_NAME_MAX_LENGTH + 2];

			szTongName[sizeof(szTongName) - 1] = 0;
			memcpy(szTongName, pApply->m_szTongName, sizeof(szTongName));
			nPlayerIdx = pApply->m_nPlayerIdx;
			nCamp = pApply->m_nCamp;

			if (nPlayerIdx <= 0 || nPlayerIdx >= MAX_PLAYER)
			{
				nRet = FALSE;
				break;
			}
			nRet = Player[nPlayerIdx].m_cTong.CheckCreateCondition(nCamp, szTongName);
		}
		break;

	// 申请加入帮会
	// uParam : struct STONG_SERVER_TO_CORE_APPLY_ADD point
	case SGDI_TONG_APPLY_ADD:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_APPLY_ADD	*pAdd = (STONG_SERVER_TO_CORE_APPLY_ADD*)uParam;
			if (pAdd->m_nPlayerIdx <= 0 || pAdd->m_nPlayerIdx >= MAX_PLAYER)
				break;
			Player[pAdd->m_nPlayerIdx].m_cTong.TransferAddApply(pAdd->m_dwNpcID);
		}
		break;

	// 判断加入帮会条件是否合适
	// uParam : 传入得 char point ，用于接收帮会名称
	// nParam : struct STONG_SERVER_TO_CORE_CHECK_ADD_CONDITION point
	case SGDI_TONG_CHECK_ADD_CONDITION:
		{
			nRet = 0;
			STONG_SERVER_TO_CORE_CHECK_ADD_CONDITION	*pAdd;
			pAdd = (STONG_SERVER_TO_CORE_CHECK_ADD_CONDITION*)nParam;
			if (pAdd->m_nSelfIdx <= 0 || pAdd->m_nSelfIdx >= MAX_PLAYER)
				break;
			if (pAdd->m_nTargetIdx <= 0 || pAdd->m_nTargetIdx >= MAX_PLAYER || g_FileName2Id(Npc[Player[pAdd->m_nTargetIdx].m_nIndex].Name) != pAdd->m_dwNameID)
				break;
			if (Player[pAdd->m_nSelfIdx].m_cTong.CheckAddCondition(pAdd->m_nTargetIdx))
			{
				Player[pAdd->m_nSelfIdx].m_cTong.GetTongName((char*)uParam);
				nRet = 1;
			}
		}
		break;

	// 获得帮会信息
	// uParam : 传入的 STONG_SERVER_TO_CORE_GET_INFO point
	case SGDI_TONG_GET_INFO:
		{
			STONG_SERVER_TO_CORE_GET_INFO	*pInfo = (STONG_SERVER_TO_CORE_GET_INFO*)uParam;
			switch (pInfo->m_nInfoID)
			{
			case enumTONG_APPLY_INFO_ID_SELF:
				{
					if (pInfo->m_nSelfIdx <= 0 || pInfo->m_nSelfIdx >= MAX_PLAYER)
						break;
					Player[pInfo->m_nSelfIdx].m_cTong.SendSelfInfo();
				}
				break;
			case enumTONG_APPLY_INFO_ID_MASTER:
				break;
			case enumTONG_APPLY_INFO_ID_DIRECTOR:
				break;
			case enumTONG_APPLY_INFO_ID_MANAGER:
				{
					nRet = 0;
					if (pInfo->m_nSelfIdx <= 0 || pInfo->m_nSelfIdx >= MAX_PLAYER)
						break;
					if (Player[pInfo->m_nSelfIdx].m_cTong.CanGetManagerInfo((DWORD)pInfo->m_nParam1))
						nRet = 1;
				}
				break;
			case enumTONG_APPLY_INFO_ID_MEMBER:
				{
					nRet = 0;
					if (pInfo->m_nSelfIdx <= 0 || pInfo->m_nSelfIdx >= MAX_PLAYER)
						break;
					if (Player[pInfo->m_nSelfIdx].m_cTong.CanGetMemberInfo((DWORD)pInfo->m_nParam1))
						nRet = 1;
				}
				break;
			case enumTONG_APPLY_INFO_ID_ONE:
				break;

			case enumTONG_APPLY_INFO_ID_TONG_HEAD:
				{
					nRet = 0;
					if (pInfo->m_nSelfIdx <= 0 || pInfo->m_nSelfIdx >= MAX_PLAYER)
						break;
					int		nPlayer;
					if (Npc[Player[pInfo->m_nSelfIdx].m_nIndex].m_dwID == (DWORD)pInfo->m_nParam1)
						nPlayer = pInfo->m_nSelfIdx;
					else
						nPlayer = Player[pInfo->m_nSelfIdx].FindAroundPlayer((DWORD)pInfo->m_nParam1);
					if (nPlayer == -1)
						break;
					nRet = Player[nPlayer].m_cTong.GetTongNameID();
				}
				break;
			}
		}
		break;

	// 判断是否有任命权利
	// uParam : 传入的 TONG_APPLY_INSTATE_COMMAND point
	// nParam : PlayerIndex
	case SGDI_TONG_INSTATE_POWER:
		if (uParam)
		{
			nRet = 0;
			TONG_APPLY_INSTATE_COMMAND	*pApply = (TONG_APPLY_INSTATE_COMMAND*)uParam;
			if (nParam <= 0 || nParam >= MAX_PLAYER)
				break;
			if (Player[nParam].m_nIndex <= 0)
				break;
			nRet = Player[nParam].m_cTong.CheckInstatePower(pApply);
		}
		break;

	// 被任命，帮会数据变化
	// uParam : 传入的 STONG_SERVER_TO_CORE_BE_INSTATED point
	case SGDI_TONG_BE_INSTATED:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_BE_INSTATED	*pInstated = (STONG_SERVER_TO_CORE_BE_INSTATED*)uParam;
			if (pInstated->m_nPlayerIdx <= 0 || pInstated->m_nPlayerIdx >= MAX_PLAYER)
				break;
			if (Player[pInstated->m_nPlayerIdx].m_nIndex <= 0)
				break;
			Player[pInstated->m_nPlayerIdx].m_cTong.BeInstated(pInstated);
		}
		break;

	// 判断是否有踢人权利
	// uParam : 传入的 TONG_APPLY_KICK_COMMAND point
	// nParam : PlayerIndex
	case SGDI_TONG_KICK_POWER:
		if (uParam)
		{
			nRet = 0;
			TONG_APPLY_KICK_COMMAND	*pKick = (TONG_APPLY_KICK_COMMAND*)uParam;
			if (nParam <= 0 || nParam >= MAX_PLAYER)
				break;
			if (Player[nParam].m_nIndex <= 0)
				break;
			nRet = Player[nParam].m_cTong.CheckKickPower(pKick);
		}
		break;

	// 被踢出帮会
	// uParam : 传入的 STONG_SERVER_TO_CORE_BE_KICKED point
	case SGDI_TONG_BE_KICKED:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_BE_KICKED	*pKicked = (STONG_SERVER_TO_CORE_BE_KICKED*)uParam;
			if (pKicked->m_nPlayerIdx <= 0 || pKicked->m_nPlayerIdx >= MAX_PLAYER)
				break;
			if (Player[pKicked->m_nPlayerIdx].m_nIndex <= 0)
				break;
			Player[pKicked->m_nPlayerIdx].m_cTong.BeKicked(pKicked);
		}
		break;

	// 离开帮会
	// uParam : 传入的 TONG_APPLY_LEAVE_COMMAND point
	// nParam : PlayerIndex
	case SGDI_TONG_LEAVE_POWER:
		if (uParam)
		{
			nRet = 0;
			TONG_APPLY_LEAVE_COMMAND	*pLeave = (TONG_APPLY_LEAVE_COMMAND*)uParam;
			if (nParam <= 0 || nParam >= MAX_PLAYER)
				break;
			if (Player[nParam].m_nIndex <= 0)
				break;
			nRet = Player[nParam].m_cTong.CheckLeavePower(pLeave);
		}
		break;

	// 离开帮会
	// uParam : 传入的 STONG_SERVER_TO_CORE_LEAVE point
	case SGDI_TONG_LEAVE:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_LEAVE	*pLeave = (STONG_SERVER_TO_CORE_LEAVE*)uParam;
			if (pLeave->m_nPlayerIdx <= 0 || pLeave->m_nPlayerIdx >= MAX_PLAYER)
				break;
			if (Player[pLeave->m_nPlayerIdx].m_nIndex <= 0)
				break;
			Player[pLeave->m_nPlayerIdx].m_cTong.Leave(pLeave);
		}
		break;

	// 离开帮会判断
	// uParam : 传入的 TONG_APPLY_CHANGE_MASTER_COMMAND point
	// nParam : PlayerIndex
	case SGDI_TONG_CHANGE_MASTER_POWER:
		if (uParam)
		{
			nRet = 0;
			TONG_APPLY_CHANGE_MASTER_COMMAND	*pChange = (TONG_APPLY_CHANGE_MASTER_COMMAND*)uParam;
			if (nParam <= 0 || nParam >= MAX_PLAYER)
				break;
			if (Player[nParam].m_nIndex <= 0)
				break;
			nRet = Player[nParam].m_cTong.CheckChangeMasterPower(pChange);
		}
		break;

	// 能否接受传位判断
	// uParam : 传入的 STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER point
	case SGDI_TONG_GET_MASTER_POWER:
		if (uParam)
		{
			nRet = 0;
			STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER	*pCheck = (STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER*)uParam;
			if (pCheck->m_nPlayerIdx <= 0 || pCheck->m_nPlayerIdx >= MAX_PLAYER)
				break;
			if (Player[pCheck->m_nPlayerIdx].m_nIndex <= 0)
				break;
			nRet = Player[pCheck->m_nPlayerIdx].m_cTong.CheckGetMasterPower(pCheck);
		}
		break;

	// 传位导致身份改变
	// uParam : 传入的 STONG_SERVER_TO_CORE_CHANGE_AS point
	case SGDI_TONG_CHANGE_AS:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_CHANGE_AS	*pAs = (STONG_SERVER_TO_CORE_CHANGE_AS*)uParam;
			if (pAs->m_nPlayerIdx <= 0 || pAs->m_nPlayerIdx >= MAX_PLAYER)
				break;
			if (Player[pAs->m_nPlayerIdx].m_nIndex <= 0)
				break;
			Player[pAs->m_nPlayerIdx].m_cTong.ChangeAs(pAs);
		}
		break;

	// 帮主换了
	// uParam : 传入的 STONG_SERVER_TO_CORE_CHANGE_MASTER point
	case SGDI_TONG_CHANGE_MASTER:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_CHANGE_MASTER	*pChange = (STONG_SERVER_TO_CORE_CHANGE_MASTER*)uParam;
			int nIdx;
			nIdx = PlayerSet.GetFirstPlayer();
			while (nIdx)
			{
				if (Player[nIdx].m_cTong.GetTongNameID() == pChange->m_dwTongNameID)
				{
					Player[nIdx].m_cTong.ChangeMaster(pChange->m_szName);
				}
				nIdx = PlayerSet.GetNextPlayer();
			}
		}
		break;

	// 获得帮会名字符串转换成的 dword
	// nParam : PlayerIndex
	case SGDI_TONG_GET_TONG_NAMEID:
		{
			if (nParam <= 0 || nParam >= MAX_PLAYER)
				break;
			if (Player[nParam].m_nIndex <= 0)
				break;
			nRet = Player[nParam].m_cTong.GetTongNameID();
		}
		break;

	// 登陆时候获得帮会信息
	// uParam : 传入的 STONG_SERVER_TO_CORE_LOGIN point
	case SGDI_TONG_LOGIN:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_LOGIN	*pLogin = (STONG_SERVER_TO_CORE_LOGIN*)uParam;
			if (pLogin->m_dwParam <= 0 || pLogin->m_dwParam >= MAX_PLAYER)
				break;
			if (Player[pLogin->m_dwParam].m_nIndex <= 0)
				break;
			Player[pLogin->m_dwParam].m_cTong.Login(pLogin);
		}
		break;
		
	// 通知core发送某玩家的帮会信息
	// nParam : player index
	case SGDI_TONG_SEND_SELF_INFO:
		{
			if (nParam <= 0 || nParam >= MAX_PLAYER)
				break;
			if (Player[nParam].m_nIndex <= 0)
				break;
			Player[nParam].m_cTong.SendSelfInfo();
		}
		break;


	default:
		break;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：向游戏发送操作
//	参数：unsigned int uDataId --> Core外部客户对core的操作请求的索引定义
//							其值为梅举类型GAMEOPERATION_INDEX的取值之一。
//		  unsigned int uParam  --> 依据uOperId的取值情况而定
//		  int nParam --> 依据uOperId的取值情况而定
//	返回：如果成功发送操作请求，函数返回非0值，否则返回0值。
//--------------------------------------------------------------------------
int	CoreServerShell::OperationRequest(unsigned int uOper, unsigned int uParam, int nParam)
{
	int nRet = 1;
	switch(uOper)
	{		
	case SSOI_BROADCASTING:
		nRet = PlayerSet.Broadcasting((char*)uParam, nParam);
		break;
	case SSOI_LAUNCH:	//启动服务
		nRet = OnLunch((LPVOID)uParam);
		break;
	case SSOI_SHUTDOWN:	//关闭服务
		nRet = OnShutdown();
		break;
	case SSOI_RELOAD_WELCOME_MSG:
		PlayerSet.ReloadWelcomeMsg();
		break;

	// relay 帮会创建成功，通知 core 进行相应的处理
	case SSOI_TONG_CREATE:
		{
			STONG_SERVER_TO_CORE_CREATE_SUCCESS	*pCreate = (STONG_SERVER_TO_CORE_CREATE_SUCCESS*)uParam;
			if (pCreate->m_nPlayerIdx <= 0 || pCreate->m_nPlayerIdx >= MAX_PLAYER)
			{
				nRet = 0;
				break;
			}
			if (Player[pCreate->m_nPlayerIdx].m_nIndex)
			{
				DWORD	dwID = g_FileName2Id(Npc[Player[pCreate->m_nPlayerIdx].m_nIndex].Name);
				if (dwID != pCreate->m_dwPlayerNameID)
				{
					nRet = 0;
					break;
				}
			}
			else
			{
				nRet = 0;
				break;
			}
			nRet = Player[pCreate->m_nPlayerIdx].CreateTong(pCreate->m_nCamp, pCreate->m_szTongName);
		}
		break;

	case SSOI_TONG_REFUSE_ADD:
		if (uParam)
		{
			STONG_SERVER_TO_CORE_REFUSE_ADD	*pRefuse = (STONG_SERVER_TO_CORE_REFUSE_ADD*)uParam;
			if (pRefuse->m_nSelfIdx > 0 && pRefuse->m_nSelfIdx <= MAX_PLAYER)
			{
				Player[pRefuse->m_nSelfIdx].m_cTong.SendRefuseMessage(pRefuse->m_nTargetIdx, pRefuse->m_dwNameID);
			}
		}
		break;

	case SSOI_TONG_ADD:
		if (uParam)
		{
			nRet = 0;
			STONG_SERVER_TO_CORE_ADD_SUCCESS	*pAdd = (STONG_SERVER_TO_CORE_ADD_SUCCESS*)uParam;
			if (pAdd->m_nPlayerIdx <= 0 || pAdd->m_nPlayerIdx >= MAX_PLAYER)
				break;
			if (Player[pAdd->m_nPlayerIdx].m_nIndex <= 0)
				break;
			if (g_FileName2Id(Npc[Player[pAdd->m_nPlayerIdx].m_nIndex].Name) != pAdd->m_dwPlayerNameID)
				break;
			Player[pAdd->m_nPlayerIdx].m_cTong.AddTong(
				pAdd->m_nCamp,
				pAdd->m_szTongName,
				pAdd->m_szMasterName,
				pAdd->m_szTitleName);
		}
		break;

	default:
		nRet = 0;
		break;
	}	
	return nRet;
}

int CoreServerShell::OnLunch(LPVOID pServer)
{
	g_SetServer(pServer);

//	g_SetFilePath("\\script");
	KLuaScript * pStartScript =(KLuaScript*) g_GetScript("\\script\\ServerScript.lua");
	int i = 0;
	
	if (!pStartScript)
		g_DebugLog("Load ServerScript failed!");
	else
	{	
		pStartScript->CallFunction("StartGame",0,"");
	}

	PlayerSet.ReloadWelcomeMsg();

	return true;
}

int CoreServerShell::OnShutdown()
{
	return true;
}

//日常活动，core如果要寿终正寝则返回0，否则返回非0值
int CoreServerShell::Breathe()
{
	g_SubWorldSet.MessageLoop();
	g_SubWorldSet.MainLoop();
	return true;
}

bool CoreServerShell::CheckProtocolSize(const char* pChar, int nSize)
{
	WORD wCheckSize;
	BYTE nProtocol = (BYTE)pChar[0];

	if (nProtocol >= c2s_end || nProtocol <= c2s_gameserverbegin)
	{
		g_DebugLog("[error]NetServer:Invalid Protocol!");
		return false;
	}

	if (g_nProtocolSize[nProtocol - c2s_gameserverbegin - 1] == -1)
	{
		wCheckSize = *(WORD*)&pChar[1] + PROTOCOL_MSG_SIZE;
	}
	else
	{
		wCheckSize = g_nProtocolSize[nProtocol - c2s_gameserverbegin - 1];
	}
	if (wCheckSize != nSize)
	{
		g_DebugLog("[error]网络接收协议大小不匹配");
#ifndef _WIN32
		printf("[error]网络接收协议大小不匹配<%d>, should %d, but %d\n", nProtocol, wCheckSize, nSize);
#endif
		return false;
	}
	return true;
}


int CoreServerShell::AttachPlayer(const unsigned long lnID, GUID* pGuid)
{
	return PlayerSet.AttachPlayer(lnID, pGuid);
}

void* CoreServerShell::SavePlayerDataAtOnce(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
	{
		return NULL;
	}

	if (Player[nIndex].Save())
	{
		Player[nIndex].m_uMustSave = SAVE_REQUEST;
		return &Player[nIndex].m_SaveBuffer;
	}
	else
	{
		return NULL;
	}
}

bool CoreServerShell::IsCharacterQuiting(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
	{
		return FALSE;
	}	
	return Player[nIndex].IsWaitingRemove();
}

bool CoreServerShell::IsPlayerLoginTimeOut(int nIndex)
{
	return Player[nIndex].IsLoginTimeOut();
}

void CoreServerShell::RemovePlayerLoginTimeOut(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;
	if (Player[nIndex].IsLoginTimeOut())
	{
		PlayerSet.RemoveLoginTimeOut(nIndex);
	}
}


int CoreServerShell::AddTempTaskValue(int nIndex, const char* pData)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return 0;

	return Player[nIndex].AddTempTaskValue((void *)pData);
}

void CoreServerShell::GetPlayerIndexByGuid(GUID* pGuid, int* pnIndex, int* plnID)
{
	*pnIndex = PlayerSet.GetPlayerIndexByGuid(pGuid);
	if (*pnIndex)
	{
		*plnID = Player[*pnIndex].m_nNetConnectIdx;
	}
	else
	{
		*plnID = -1;
	}

	if (*plnID == -1)
	{
		*pnIndex = 0;
	}
}

void* CoreServerShell::PreparePlayerForExchange(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return NULL;
	PlayerSet.PrepareExchange(nIndex);
	return &Player[nIndex].m_SaveBuffer;
}

bool CoreServerShell::IsPlayerExchangingServer(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return false;

	return Player[nIndex].IsExchangingServer();
}

void CoreServerShell::RemovePlayerForExchange(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;

	PlayerSet.RemoveExchanging(nIndex);
}

void CoreServerShell::GetGuid(int nIndex, void* pGuid)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;
	memcpy(pGuid, &Player[nIndex].m_Guid, sizeof(GUID));
}

DWORD CoreServerShell::GetExchangeMap(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return -1;

	return Player[nIndex].m_sExchangePos.m_dwMapID;
}

void CoreServerShell::RecoverPlayerExchange(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;

	Player[nIndex].m_bExchangeServer = FALSE;
	if (Player[nIndex].m_nIndex > 0)
	{
		KNpc* pNpc = &Npc[Player[nIndex].m_nIndex];
		pNpc->m_bExchangeServer = FALSE;
		pNpc->m_FightMode = pNpc->m_OldFightMode;
	}
	Player[nIndex].Earn(Player[nIndex].m_nPrePayMoney);
	Player[nIndex].m_nPrePayMoney = 0;
}

void CoreServerShell::SetSaveStatus(int nIndex, UINT uStatus)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;

	Player[nIndex].m_uMustSave = uStatus;
}

UINT CoreServerShell::GetSaveStatus(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return SAVE_IDLE;

	return Player[nIndex].m_uMustSave;
}


void CoreServerShell::PreparePlayerForLoginFailed(int nIndex)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return;

	PlayerSet.PrepareLoginFailed(nIndex);
}
//BOOL CoreServerShell::ValidPingTime(int nIndex)
//{
//	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
//		return FALSE;
//
//	if (Player[nIndex].m_uLastPingTime == -1)
//		return TRUE;
//	
//#define	MAX_PING_TIME	(60 * 20)	//	1min
//	if (g_SubWorldSet.GetGameTime() - Player[nIndex].m_uLastPingTime > MAX_PING_TIME)
//	{
//		return FALSE;
//	}
//	return TRUE;
//}

BOOL CoreServerShell::GroupChat(IClient* pClient, DWORD FromIP, unsigned long FromRelayID, DWORD channid, BYTE tgtcls, DWORD tgtid, const void* pData, size_t size)
{
	switch(tgtcls)
	{

	case tgtcls_team:
		{{
		if (tgtid < 0 || tgtid >= MAX_TEAM)
			return FALSE;

		size_t pckgsize = sizeof(tagExtendProtoHeader) + size;
#ifdef WIN32
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
		pExHeader->ProtocolType = s2c_extendchat;
		pExHeader->wLength = pckgsize - 1;
		memcpy(pExHeader + 1, pData, size);

		int nTargetIdx;
		// 给队长发
		nTargetIdx = g_Team[tgtid].m_nCaptain;
//		if (FromRelayID != Player[nTargetIdx].m_nNetConnectIdx)
			g_pServer->SendData(Player[nTargetIdx].m_nNetConnectIdx, pData, size);
		// 给队员发
		for (int i = 0; i <	MAX_TEAM_MEMBER; i++)
		{
			nTargetIdx = g_Team[tgtid].m_nMember[i];
			if (nTargetIdx < 0)
				continue;

//			if (FromRelayID != Player[nTargetIdx].m_nNetConnectIdx)
				g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pExHeader, pckgsize);
		}
#ifndef WIN32
		delete ((char*)pExHeader);
#endif
		}}
		break;

	case tgtcls_fac:
		{{
		size_t pckgsize = sizeof(tagExtendProtoHeader) + size;
#ifdef WIN32
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
		pExHeader->ProtocolType = s2c_extendchat;
		pExHeader->wLength = pckgsize - 1;
		memcpy(pExHeader + 1, pData, size);

		int nTargetIdx;
		nTargetIdx = PlayerSet.GetFirstPlayer();
		while (nTargetIdx)
		{
			if (Player[nTargetIdx].m_cFaction.m_nCurFaction == tgtid
)//				&& FromRelayID != Player[nTargetIdx].m_nNetConnectIdx)
				g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pExHeader, pckgsize);

			nTargetIdx = PlayerSet.GetNextPlayer();
		}
#ifndef WIN32
		delete ((char*)pExHeader);
#endif
		}}
		break;

	case tgtcls_tong:
		{{
		size_t pckgsize = sizeof(tagExtendProtoHeader) + size;
#ifdef WIN32
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
		pExHeader->ProtocolType = s2c_extendchat;
		pExHeader->wLength = pckgsize - 1;
		memcpy(pExHeader + 1, pData, size);

		int nTargetIdx;
		nTargetIdx = PlayerSet.GetFirstPlayer();
		while (nTargetIdx)
		{
			if (Player[nTargetIdx].m_cTong.GetTongNameID() == tgtid
)//				&& FromRelayID != Player[nTargetIdx].m_nNetConnectIdx)
				g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pExHeader, pckgsize);

			nTargetIdx = PlayerSet.GetNextPlayer();
		}
#ifndef WIN32
		delete ((char*)pExHeader);
#endif
		}}
		break;

	case tgtcls_scrn:
		{{

//		int nMaxRelayPlayer = (1024 - 32 - sizeof(CHAT_GROUPMAN) - size) / sizeof(WORD);
//		if (nMaxRelayPlayer <= 0)
//			return FALSE;


		int idxNPC = Player[tgtid].m_nIndex;
		int idxSubWorld = Npc[idxNPC].m_SubWorldIndex;
		int idxRegion = Npc[idxNPC].m_RegionIndex;
//		_ASSERT(idxSubWorld >= 0 && idxRegion >= 0);
		int nOX = Npc[idxNPC].m_MapX;
		int nOY = Npc[idxNPC].m_MapY;
		int nTX = 0;
		int nTY = 0;
		if (idxSubWorld < 0 || idxRegion < 0)
			return FALSE;


//		size_t basesize = sizeof(CHAT_GROUPMAN) + size;
//		BYTE buffer[1024];
//
//		CHAT_GROUPMAN* pCgc = (CHAT_GROUPMAN*)buffer;
//		pCgc->ProtocolType = chat_groupman;
//		pCgc->wChatLength = size;
//		pCgc->byHasIdentify = false;
//
//		void* pExPckg = pCgc + 1;
//		memcpy(pExPckg, pData, size);
//
//		WORD* pPlayers = (WORD*)((BYTE*)pExPckg + size);
//
//
//		pCgc->wPlayerCount = 0;


		size_t pckgsize = sizeof(tagExtendProtoHeader) + size;
#ifdef WIN32
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
		pExHeader->ProtocolType = s2c_extendchat;
		pExHeader->wLength = pckgsize - 1;
		memcpy(pExHeader + 1, pData, size);

#define	MAX_SYNC_RANGE	23
		static POINT POff[9] = 
		{
			{0, 0},
			{0, 32},
			{-16, 32},
			{-16, 0},
			{-16, -32},
			{0, -32},
			{16, -32},
			{16, 0},
			{16, 32},
		};

		KRegion* pRegionBase = &SubWorld[idxSubWorld].m_Region[idxRegion];

		for (int i = -1; i < 8; i++)
		{
			KRegion* pRegion = NULL;
			if (i < 0)
				pRegion = pRegionBase;
			else
			{
				if (pRegionBase->m_nConnectRegion[i] < 0)
					continue;
				pRegion = &SubWorld[idxSubWorld].m_Region[pRegionBase->m_nConnectRegion[i]];
			}
			if (pRegion == NULL)
				continue;


			KIndexNode *pNode = (KIndexNode *)pRegion->m_PlayerList.GetHead();
			while(pNode)
			{
//				_ASSERT(pNode->m_nIndex > 0 && pNode->m_nIndex < MAX_PLAYER);

				//if (FromRelayID != Player[pNode->m_nIndex].m_nNetConnectIdx)
				{
					int nTargetNpc = Player[pNode->m_nIndex].m_nIndex;
					if (nTargetNpc > 0)
					{
						nTX = Npc[nTargetNpc].m_MapX + POff[i + 1].x;
						nTY = Npc[nTargetNpc].m_MapY + POff[i + 1].y;
						
						if ((nTX - nOX) * (nTX - nOX) + (nTY - nOY) * (nTY - nOY) < MAX_SYNC_RANGE * MAX_SYNC_RANGE)
							g_pServer->PackDataToClient(Player[pNode->m_nIndex].m_nNetConnectIdx, pExHeader, pckgsize);
					}


//					pPlayers[pCgc->wPlayerCount] = (WORD)Player[pNode->m_nIndex].m_nNetConnectIdx;
//					++ pCgc->wPlayerCount;
//
//					if (pCgc->wPlayerCount >= nMaxRelayPlayer)
//					{
//						size_t pckgsize = basesize + sizeof(WORD) * pCgc->wPlayerCount;
//						pCgc->wSize = pckgsize - 1;
//						
//						pClient->SendPackToServer(pCgc, pckgsize);
//
//						pCgc->wPlayerCount = 0;
//					}
				}

				pNode = (KIndexNode *)pNode->GetNext();
			}
		}

//		if (pCgc->wPlayerCount > 0)
//		{
//			size_t pckgsize = basesize + sizeof(WORD) * pCgc->wPlayerCount;
//			pCgc->wSize = pckgsize - 1;
//
//			pClient->SendPackToServer(pCgc, pckgsize);
//		}

#ifndef WIN32
		delete (char*)pExHeader;
#endif
		}}
		break;


	case tgtcls_bc:
		{{
		size_t pckgsize = sizeof(tagExtendProtoHeader) + size;
#ifdef WIN32
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
#else
		tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)(new char[pckgsize]);
#endif
		pExHeader->ProtocolType = s2c_extendchat;
		pExHeader->wLength = pckgsize - 1;
		memcpy(pExHeader + 1, pData, size);

		int nTargetIdx;
		nTargetIdx = PlayerSet.GetFirstPlayer();
		while (nTargetIdx)
		{
			g_pServer->PackDataToClient(Player[nTargetIdx].m_nNetConnectIdx, pExHeader, pckgsize);

			nTargetIdx = PlayerSet.GetNextPlayer();
		}
#ifndef WIN32
		delete ((char*)pExHeader);
#endif
		}}
		break;


	default:
		break;
	}
	return TRUE;
}

void CoreServerShell::SetLadder(void* pData, size_t uSize)
{
	Ladder.Init(pData, uSize);
}

BOOL CoreServerShell::PayForSpeech(int nIndex, int nType)
{
	if (nIndex <= 0 || nIndex >= MAX_PLAYER)
		return FALSE;

	int	nMoney = 0;
	int nNpcIdx = Player[nIndex].m_nIndex;
	if (nNpcIdx <= 0)
		return FALSE;
	if (Player[nIndex].m_nForbiddenFlag & KPlayer::FF_CHAT)	//被禁言
		return FALSE;
	int nLevel = Npc[nNpcIdx].m_Level;
	int nMaxMana = Npc[nNpcIdx].m_CurrentManaMax;
	switch (nType)
	{
	case 0:		//免费
		return TRUE;
		break;
	case 1:		//10元每句
		{
			nMoney = 10;
			return Player[nIndex].Pay(nMoney);
		}
		break;
	case 2:		//2: <10Lv ? 不能说 : MaxMana/2/句
		{
			if (nLevel < 10)
				return FALSE;
			return Npc[nNpcIdx].Cost(attrib_mana, nMaxMana / 2);
		}
		break;
	case 3:		//3: MaxMana/10/句
		{
			return Npc[nNpcIdx].Cost(attrib_mana, nMaxMana / 10);
		}
		break;
	case 4:		//4: <20Lv ? 不能说 : MaxMana*4/5/句
		{
			if (nLevel < 20)
				return FALSE;
			return Npc[nNpcIdx].Cost(attrib_mana, nMaxMana * 4 / 5);
		}
		break;
	default:
		return FALSE;	//不认识的类别不发送
	}
}