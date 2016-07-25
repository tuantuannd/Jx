// TongConnect.cpp: implementation of the CTongConnect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TongConnect.h"
#include "TongServer.h"
#include "S3Relay.h"
#include "Global.h"
#include "malloc.h"


//-------------------------- tong struct size ---------------------------
int	g_nTongPSSize[defTONG_PROTOCOL_SERVER_NUM] = 
{
	-1,											// enumC2S_TONG_CREATE
	-1,											// enumC2S_TONG_ADD_MEMBER
	sizeof(STONG_GET_TONG_HEAD_INFO_COMMAND),	// enumC2S_TONG_GET_HEAD_INFO
	sizeof(STONG_GET_MANAGER_INFO_COMMAND),		// enumC2S_TONG_GET_MANAGER_INFO
	sizeof(STONG_GET_MEMBER_INFO_COMMAND),		// enumC2S_TONG_GET_MEMBER_INFO
	sizeof(STONG_INSTATE_COMMAND),				// enumC2S_TONG_INSTATE
	sizeof(STONG_KICK_COMMAND),					// enumC2S_TONG_KICK
	sizeof(STONG_LEAVE_COMMAND),				// enumC2S_TONG_LEAVE
	sizeof(STONG_CHANGE_MASTER_COMMAND),		// enumC2S_TONG_CHANGE_MASTER
	sizeof(STONG_ACCEPT_MASTER_COMMAND),		// enumC2S_TONG_ACCEPT_MASTER
	sizeof(STONG_GET_LOGIN_DATA_COMMAND),		// enumC2S_TONG_GET_LOGIN_DATA
};

//------------------------- tong struct size end ---------------------------


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTongConnect::CTongConnect(CTongServer* pTongServer, unsigned long id)
	: CNetConnect(pTongServer, id)
{

}

CTongConnect::~CTongConnect()
{

}


void CTongConnect::OnClientConnectCreate()
{
	rTRACE("tong connect create: %s", _ip2a(GetIP()));
}

void CTongConnect::OnClientConnectClose()
{
	rTRACE("tong connect close: %s", _ip2a(GetIP()));
}


void CTongConnect::RecvPackage(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolFamily == pf_tong)
	{
		Proc0_Tong(pData, size);
	}
	else if (pHeader->ProtocolFamily == pf_friend)
	{
		Proc0_Friend(pData, size);
	}
}


BOOL CTongConnect::PassToSomeone(DWORD ip, unsigned long id, DWORD nameid, const void* pData, size_t size)
{
	CNetConnectDup conndup;
	if (GetIP() == ip)
		conndup = *this;
	else
	{
		conndup = g_TongServer.FindTongConnectByIP(ip);
		if (!conndup.IsValid())
			return FALSE;
	}

	size_t pckgsize = sizeof(EXTEND_PASSTOSOMEONE) + size;
	EXTEND_PASSTOSOMEONE* pEps = (EXTEND_PASSTOSOMEONE*)_alloca(pckgsize);
	pEps->ProtocolFamily = pf_extend;
	pEps->ProtocolID = extend_s2c_passtosomeone;
	pEps->nameid = nameid;
	pEps->lnID = id;
	pEps->datasize = size;
	memcpy(pEps + 1, pData, size);

	conndup.SendPackage(pEps, pckgsize);

	return TRUE;
}

//--------------------------------------------------------------------
//	功能：帮会协议处理，收到 game server 发来的与帮会有关的协议
//--------------------------------------------------------------------
void CTongConnect::Proc0_Tong(const void* pData, size_t size)
{
	if (!pData)
		return;

	// 协议长度检测
	if (size < sizeof(EXTEND_HEADER))
		return;
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;
	if (pHeader->ProtocolID >= enumC2S_TONG_NUM)
		return;
	if (g_nTongPSSize[pHeader->ProtocolID] < 0)
	{
		if (size <= sizeof(EXTEND_HEADER) + 2)
			return;
		WORD	wLength = *((WORD*)((BYTE*)pData + sizeof(EXTEND_HEADER)));
		if (wLength != size)
			return;
	}
	else if (g_nTongPSSize[pHeader->ProtocolID] != size)
	{
		return;
	}

	switch (pHeader->ProtocolID)
	{
	case enumC2S_TONG_CREATE:
		{
			char	szPlayerName[64], szTongName[64];
			STONG_CREATE_COMMAND	*pTongCreate = (STONG_CREATE_COMMAND*)pData;
			int		nCamp = pTongCreate->m_btCamp;

			memcpy(szTongName, &pTongCreate->m_szBuffer[0], pTongCreate->m_btTongNameLength);
			szTongName[pTongCreate->m_btTongNameLength] = 0;
			memcpy(szPlayerName, &pTongCreate->m_szBuffer[pTongCreate->m_btTongNameLength], pTongCreate->m_btPlayerNameLength);
			szPlayerName[pTongCreate->m_btPlayerNameLength] = 0;

			// 建立帮会
			int nErrorID = g_cTongSet.Create(nCamp, szPlayerName, szTongName);
			if (nErrorID == 0)
			{
				STONG_CREATE_SUCCESS_SYNC	sSync;
				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_CREATE_SUCCESS;
				sSync.m_dwPlayerNameID = pTongCreate->m_dwPlayerNameID;
				sSync.m_dwParam = pTongCreate->m_dwParam;
				sSync.m_btCamp = pTongCreate->m_btCamp;
				sSync.m_btTongNameLength = pTongCreate->m_btTongNameLength;
				memcpy(sSync.m_szTongName, pTongCreate->m_szBuffer, sSync.m_btTongNameLength);
				sSync.m_wLength = sizeof(STONG_CREATE_SUCCESS_SYNC) - sizeof(sSync.m_szTongName) + sSync.m_btTongNameLength;

				this->SendPackage((const void *)&sSync, sSync.m_wLength);
			}
			// 建立失败
			else
			{
				STONG_CREATE_FAIL_SYNC	sSync;
				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_CREATE_FAIL;
				sSync.m_dwPlayerNameID = pTongCreate->m_dwPlayerNameID;
				sSync.m_dwParam = pTongCreate->m_dwParam;
				sSync.m_btFailID = nErrorID;

				this->SendPackage((const void *)&sSync, sizeof(STONG_CREATE_FAIL_SYNC));
			}
		}
		break;

	// 添加成员
	case enumC2S_TONG_ADD_MEMBER:
		{
			char	szPlayerName[64], szTongName[64];
			STONG_ADD_MEMBER_COMMAND	*pAdd = (STONG_ADD_MEMBER_COMMAND*)pData;

			memcpy(szTongName, &pAdd->m_szBuffer[0], pAdd->m_btTongNameLength);
			szTongName[pAdd->m_btTongNameLength] = 0;
			memcpy(szPlayerName, &pAdd->m_szBuffer[pAdd->m_btTongNameLength], pAdd->m_btPlayerNameLength);
			szPlayerName[pAdd->m_btPlayerNameLength] = 0;

			int nRet = g_cTongSet.AddMember(szPlayerName, szTongName);
			// 成员添加成功
			if (nRet >= 0)
			{
				STONG_ADD_MEMBER_SUCCESS_SYNC	sSync;
				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_ADD_MEMBER_SUCCESS;
				sSync.m_dwPlayerNameID = pAdd->m_dwPlayerNameID;
				sSync.m_dwParam = pAdd->m_dwParam;
				sSync.m_btCamp = g_cTongSet.GetTongCamp(nRet);
				g_cTongSet.GetMasterName(nRet, sSync.m_szMasterName);
				g_cTongSet.GetMemberTitle(nRet, sSync.m_szTitleName);
				strcpy(sSync.m_szTongName, szTongName);

				this->SendPackage((const void *)&sSync, sizeof(sSync));
			}
			// 成员添加不成功
			else
			{
				STONG_ADD_MEMBER_FAIL_SYNC	sSync;
				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_ADD_MEMBER_FAIL;
				sSync.m_dwPlayerNameID = pAdd->m_dwPlayerNameID;
				sSync.m_dwParam = pAdd->m_dwParam;
				sSync.m_btFailID = nRet;

				this->SendPackage((const void *)&sSync, sizeof(STONG_ADD_MEMBER_FAIL_SYNC));
			}
		}
		break;
	// 获得帮会信息
	case enumC2S_TONG_GET_HEAD_INFO:
		{
			STONG_GET_TONG_HEAD_INFO_COMMAND	*pGet = (STONG_GET_TONG_HEAD_INFO_COMMAND*)pData;
			STONG_HEAD_INFO_SYNC	sInfo;
			if (g_cTongSet.GetTongHeadInfo(pGet->m_dwTongNameID, &sInfo))
			{
				sInfo.m_dwParam = pGet->m_dwParam;
				sInfo.m_dwNpcID = pGet->m_dwNpcID;
				this->SendPackage((const void *)&sInfo, sInfo.m_wLength);
			}
		}
		break;
	case enumC2S_TONG_GET_MANAGER_INFO:
		{
			STONG_GET_MANAGER_INFO_COMMAND	*pGet = (STONG_GET_MANAGER_INFO_COMMAND*)pData;
			STONG_MANAGER_INFO_SYNC	sInfo;
			if (g_cTongSet.GetTongManagerInfo(pGet, &sInfo))
				this->SendPackage((const void *)&sInfo, sInfo.m_wLength);
		}
		break;
	case enumC2S_TONG_GET_MEMBER_INFO:
		{
			STONG_GET_MEMBER_INFO_COMMAND	*pGet = (STONG_GET_MEMBER_INFO_COMMAND*)pData;
			STONG_MEMBER_INFO_SYNC	sInfo;
			if (g_cTongSet.GetTongMemberInfo(pGet, &sInfo))
				this->SendPackage((const void *)&sInfo, sInfo.m_wLength);
		}
		break;
	case enumC2S_TONG_INSTATE:
		{
			STONG_INSTATE_COMMAND	*pInstate = (STONG_INSTATE_COMMAND*)pData;
			STONG_INSTATE_SYNC	sSync;
			sSync.ProtocolID = 0;
			g_cTongSet.Instate(pInstate, &sSync);
			if (sSync.ProtocolID != 0)
				this->SendPackage((const void *)&sSync, sizeof(sSync));
		}
		break;
	case enumC2S_TONG_KICK:
		{
			STONG_KICK_COMMAND	*pKick = (STONG_KICK_COMMAND*)pData;
			STONG_KICK_SYNC	sKick;
			sKick.ProtocolID = 0;
			g_cTongSet.Kick(pKick, &sKick);
			if (sKick.ProtocolID != 0)
				this->SendPackage((const void *)&sKick, sizeof(sKick));
		}
		break;
	case enumC2S_TONG_LEAVE:
		{
			STONG_LEAVE_COMMAND	*pLeave = (STONG_LEAVE_COMMAND*)pData;
			STONG_LEAVE_SYNC	sLeave;
			sLeave.ProtocolID = 0;
			g_cTongSet.Leave(pLeave, &sLeave);
			if (sLeave.ProtocolID != 0)
				this->SendPackage((const void *)&sLeave, sizeof(sLeave));
		}
		break;
	case enumC2S_TONG_CHANGE_MASTER:
		{
			STONG_CHANGE_MASTER_COMMAND	*pChange = (STONG_CHANGE_MASTER_COMMAND*)pData;

			char	szName[32];
			STONG_ONE_LEADER_INFO	sInfo;
			memcpy(szName, pChange->m_szName, sizeof(pChange->m_szName));
			szName[31] = 0;

			if (!g_cTongSet.SearchOne(pChange->m_dwTongNameID, szName, &sInfo))
			{
				STONG_CHANGE_MASTER_FAIL_SYNC	sFail;
				sFail.ProtocolFamily	= pf_tong;
				sFail.ProtocolID		= enumS2C_TONG_CHANGE_MASTER_FAIL;
				sFail.m_dwParam			= pChange->m_dwParam;
				sFail.m_btFailID		= 2;
				sFail.m_dwTongNameID	= pChange->m_dwTongNameID;
				memcpy(sFail.m_szName, szName, sizeof(szName));
				this->SendPackage((const void *)&sFail, sizeof(sFail));
				break;
			}

			{{
				CNetConnectDup conndup;
				DWORD nameid = 0;
				unsigned long param = 0;
				if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(szName), &conndup, NULL, &nameid, &param))
				{
					CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup.GetIP());
					if (tongconndup.IsValid())
					{
						STONG_CHECK_GET_MASTER_POWER_SYNC	sCheck;
						sCheck.ProtocolFamily	= pf_tong;
						sCheck.ProtocolID		= enumS2C_TONG_CHECK_CHANGE_MASTER_POWER;
						sCheck.m_dwTongNameID	= pChange->m_dwTongNameID;
						sCheck.m_btFigure		= pChange->m_btFigure;
						sCheck.m_btPos			= pChange->m_btPos;
						sCheck.m_dwParam		= param;
						memcpy(sCheck.m_szName, szName, sizeof(szName));
						
						tongconndup.SendPackage((const void *)&sCheck, sizeof(sCheck));
					}
					break;
				}

				// 不在线 not find 或者 tong is not connect 当作不在线
				STONG_CHANGE_MASTER_FAIL_SYNC	sFail;
				sFail.ProtocolFamily	= pf_tong;
				sFail.ProtocolID		= enumS2C_TONG_CHANGE_MASTER_FAIL;
				sFail.m_dwParam			= pChange->m_dwParam;
				sFail.m_btFailID		= 0;
				sFail.m_dwTongNameID	= pChange->m_dwTongNameID;
				memcpy(sFail.m_szName, szName, sizeof(szName));
				this->SendPackage((const void *)&sFail, sizeof(sFail));
			}}

		}
		break;

	case enumC2S_TONG_ACCEPT_MASTER:
		{
			STONG_ACCEPT_MASTER_COMMAND	*pAccept = (STONG_ACCEPT_MASTER_COMMAND*)pData;
			g_cTongSet.AcceptMaster(pAccept);
		}
		break;

	case enumC2S_TONG_GET_LOGIN_DATA:
		{
			STONG_GET_LOGIN_DATA_COMMAND	*pLogin = (STONG_GET_LOGIN_DATA_COMMAND*)pData;
			STONG_LOGIN_DATA_SYNC	sLogin;
			sLogin.m_dwParam = 0;
			g_cTongSet.GetLoginData(pLogin, &sLogin);
			if (sLogin.m_dwParam)
				this->SendPackage((const void *)&sLogin, sizeof(sLogin));
		}
		break;

	default:
		break;
	}
}

void CTongConnect::Proc0_Friend(const void* pData, size_t size)
{
	EXTEND_HEADER* pHeader = (EXTEND_HEADER*)pData;

	if (pHeader->ProtocolID == friend_c2c_askaddfriend)
	{
		Proc1_Friend_AskAddFriend(pData, size);
	}
	else if (pHeader->ProtocolID == friend_c2c_repaddfriend)
	{
		Proc1_Friend_RepAddFriend(pData, size);
	}
	else if (pHeader->ProtocolID == friend_c2s_groupfriend)
	{
		Proc1_Friend_GroupFriend(pData, size);
	}
	else if (pHeader->ProtocolID == friend_c2s_erasefriend)
	{
		Proc1_Friend_EraseFriend(pData, size);
	}
	else if (pHeader->ProtocolID == friend_c2s_asksyncfriendlist)
	{
		Proc1_Friend_AskSyncFriendList(pData, size);
	}
	else if (pHeader->ProtocolID == friend_c2s_associate)
	{
		Proc1_Friend_Associate(pData, size);
	}
	else if (pHeader->ProtocolID == friend_c2s_associatebevy)
	{
		Proc1_Friend_AssociateBevy(pData, size);
	}
}

void CTongConnect::Proc1_Friend_AskAddFriend(const void* pData, size_t size)
{
	ASK_ADDFRIEND_CMD* pAafCmd = (ASK_ADDFRIEND_CMD*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	if (!gIsLegalString(pAafCmd->dstrole, 1, _NAME_LEN))
		return;

	std::_tstring srcrole;
	if (!g_HostServer.FindPlayerByIpParam(NULL, GetIP(), pSrcInfo->lnID, NULL, NULL, &srcrole, NULL))
		return;

	std::_tstring dstrole(pAafCmd->dstrole);

	if (_tstring_equal()(srcrole, dstrole))
		return;


	BOOL pass = FALSE;
	BOOL promise = FALSE;

	CNetConnectDup dsthostconndup;
	DWORD dstnameid = 0;
	unsigned long dstparam = 0;
	if (g_HostServer.FindPlayerByRole(NULL, dstrole, &dsthostconndup, NULL, &dstnameid, &dstparam))
	{
		if (g_FriendMgr.PlayerIsFriend(dstrole, srcrole))
		{
			g_FriendMgr.PlayerAddFriend(srcrole, dstrole);

			promise = TRUE;	//默许
		}
		else
		{
			if (g_FriendMgr.TrackAddFriend(srcrole, dstrole, pAafCmd->pckgid))
			{
				ASK_ADDFRIEND_SYNC AafSync;
				AafSync.ProtocolFamily = pf_friend;
				AafSync.ProtocolID = friend_c2c_askaddfriend;
				AafSync.pckgid = pAafCmd->pckgid;
				strcpy(AafSync.srcrole, srcrole.c_str());

				if (PassToSomeone(dsthostconndup.GetIP(), dstparam, dstnameid, &AafSync, sizeof(ASK_ADDFRIEND_SYNC)))
					pass = TRUE;
				else
					g_FriendMgr.ExtractAddFriend(srcrole, dstrole, pAafCmd->pckgid);
			}
		}
	}

	if (!pass)
	{
		REP_ADDFRIEND_SYNC RafSync;
		RafSync.ProtocolFamily = pf_friend;
		RafSync.ProtocolID = friend_c2c_repaddfriend;
		RafSync.pckgid = pAafCmd->pckgid;
		strcpy(RafSync.srcrole, pAafCmd->dstrole);
		RafSync.answer = promise ? answerAgree : answerUnable;

		PassToSomeone(GetIP(), pSrcInfo->lnID, pSrcInfo->nameid, &RafSync, sizeof(REP_ADDFRIEND_SYNC));
	}
}

void CTongConnect::Proc1_Friend_RepAddFriend(const void* pData, size_t size)
{
	REP_ADDFRIEND_CMD* pRafCmd = (REP_ADDFRIEND_CMD*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	if (!gIsLegalString(pRafCmd->dstrole, 1, _NAME_LEN))
		return;

	std::_tstring srcrole;
	if (!g_HostServer.FindPlayerByIpParam(NULL, GetIP(), pSrcInfo->lnID, NULL, NULL, &srcrole, NULL))
		return;

	std::_tstring dstrole(pRafCmd->dstrole);

	if (_tstring_equal()(srcrole, dstrole))
		return;


	if (!g_FriendMgr.ExtractAddFriend(dstrole, srcrole, pRafCmd->pckgid))
		return;


	if (pRafCmd->answer == answerAgree)
		g_FriendMgr.PlayerAddFriend(srcrole, dstrole);


	{{
	CNetConnectDup dsthostconndup;
	DWORD dstnameid = 0;
	unsigned long dstparam = 0;
	if (g_HostServer.FindPlayerByRole(NULL, dstrole, &dsthostconndup, NULL, &dstnameid, &dstparam))
	{
		REP_ADDFRIEND_SYNC RafSync;
		RafSync.ProtocolFamily = pf_friend;
		RafSync.ProtocolID = friend_c2c_repaddfriend;
		RafSync.pckgid = pRafCmd->pckgid;
		strcpy(RafSync.srcrole, srcrole.c_str());
		RafSync.answer = pRafCmd->answer;

		PassToSomeone(dsthostconndup.GetIP(), dstparam, dstnameid, &RafSync, sizeof(REP_ADDFRIEND_SYNC));
	}
	else
	{
		FRIEND_STATE Fs;
		Fs.ProtocolFamily = pf_friend;
		Fs.ProtocolID = friend_s2c_friendstate;
		Fs.state = stateOffline;

		PassToSomeone(GetIP(), pSrcInfo->lnID, pSrcInfo->nameid, &Fs, sizeof(FRIEND_STATE));
	}
	}}
}

void CTongConnect::Proc1_Friend_GroupFriend(const void* pData, size_t size)
{
	GROUP_FRIEND* pGf = (GROUP_FRIEND*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	std::_tstring srcrole;
	if (!g_HostServer.FindPlayerByIpParam(NULL, GetIP(), pSrcInfo->lnID, NULL, NULL, &srcrole, NULL))
		return;

	char* pGroupTag = (char*)(pGf + 1);
	if (*pGroupTag != specGroup)
		return;	//error

	char* pGroup = (char*)(pGroupTag + 1);

on_nextgroup:
	if (!gIsLegalString(pGroup, 0, _GROUP_NAME_LEN))
		return;	//error

	std::_tstring group(pGroup);

	for (char* pRoleTag = pGroup + strlen(pGroup) + 1; ; )
	{
		if (*pRoleTag == specOver)
			return;	//success
		else if (*pRoleTag == specGroup)
		{
			pGroup = pRoleTag + 1;
			goto on_nextgroup;	//next group
		}
		else if (*pRoleTag == specRole)
		{
			char* pRole = pRoleTag + 1;

			if (!gIsLegalString(pRole, 1, _NAME_LEN))
				return;	//error

			std::_tstring role(pRole);
			g_FriendMgr.SetFriendGroup(srcrole, role, group);

			pRoleTag = pRole + role.size() + 1;
			continue; 	//next role
		}
		else
			return;	//error
	}
}

void CTongConnect::Proc1_Friend_EraseFriend(const void* pData, size_t size)
{
	ERASE_FRIEND* pEf = (ERASE_FRIEND*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	if (!gIsLegalString(pEf->friendrole, 1, _NAME_LEN))
		return;

	std::_tstring srcrole;
	if (!g_HostServer.FindPlayerByIpParam(NULL, GetIP(), pSrcInfo->lnID, NULL, NULL, &srcrole, NULL))
		return;

	g_FriendMgr.PlayerDelFriend(srcrole, std::_tstring(pEf->friendrole));
}

void CTongConnect::Proc1_Friend_AskSyncFriendList(const void* pData, size_t size)
{
	ASK_SYNCFRIENDLIST* pAsfl = (ASK_SYNCFRIENDLIST*)pData;
	tagPlusSrcInfo* pSrcInfo = (tagPlusSrcInfo*)((BYTE*)pData + size) - 1;

	std::_tstring role;
	if (!g_HostServer.FindPlayerByIpParam(NULL, GetIP(), pSrcInfo->lnID, NULL, NULL, &role, NULL))
		return;

	g_FriendMgr.SomeoneSyncFriends(role, pSrcInfo->lnID, pSrcInfo->nameid, CNetConnectDup(*this), pAsfl->full, pAsfl->pckgid);
}

void CTongConnect::Proc1_Friend_Associate(const void* pData, size_t size)
{
	FRIEND_ASSOCIATE* pFa = (FRIEND_ASSOCIATE*)pData;

	char* szGroup = (char*)(pFa + 1);
	if (!gIsLegalString(szGroup, 0, _GROUP_NAME_LEN))
		return;

	std::_tstring group(szGroup);

	char* szRole1 = szGroup + group.size() + 1;
	if (!gIsLegalString(szRole1, 1, _NAME_LEN))
		return;

	std::_tstring role1(szRole1);

	char* szRole2 = szRole1 + role1.size() + 1;
	if (!gIsLegalString(szRole2, 1, _NAME_LEN))
		return;

	std::_tstring role2(szRole2);

	if (_tstring_equal()(role1, role2))
		return;


	{{
	g_FriendMgr.PlayerAssociate(role1, role2, group, pFa->bidir);
	}}



	size_t basesize = sizeof(FRIEND_SYNCASSOCIATE) + group.size() + 1;
	size_t role1size = role1.size() + 1;
	size_t role2size = role2.size() + 1;
	size_t maxsize = basesize + (pFa->bidir ? max(role1size, role2size) : role2size) + 1;

	FRIEND_SYNCASSOCIATE* pFsa = (FRIEND_SYNCASSOCIATE*)_alloca(maxsize);
	pFsa->ProtocolFamily = pf_friend;
	pFsa->ProtocolID = friend_s2c_syncassociate;

	char* szGroupSync = (char*)(pFsa + 1);
	strcpy(szGroupSync, group.c_str());

	char* szRoleSync = szGroupSync + group.size() + 1;


	{{
	CNetConnectDup conndup;
	DWORD nameid = 0;
	unsigned long param = 0;
	if (g_HostServer.FindPlayerByRole(NULL, role1, &conndup, NULL, &nameid, &param))
	{
		size_t pckgsize = basesize + role2size + 1;

		strcpy(szRoleSync, role2.c_str());
		szRoleSync[role2size] = 0;

		PassToSomeone(conndup.GetIP(), param, nameid, pFsa, pckgsize);
	}
	}}

	if (pFa->bidir)
	{
		CNetConnectDup conndup2;
		DWORD nameid2 = 0;
		unsigned long param2 = 0;
		if (g_HostServer.FindPlayerByRole(NULL, role2, &conndup2, NULL, &nameid2, &param2))
		{
			size_t pckgsize2 = basesize + role1size + 1;

			strcpy(szRoleSync, role1.c_str());
			szRoleSync[role1size] = 0;

			PassToSomeone(conndup2.GetIP(), param2, nameid2, pFsa, pckgsize2);
		}
	}
}

void CTongConnect::Proc1_Friend_AssociateBevy(const void* pData, size_t size)
{
	FRIEND_ASSOCIATEBEVY* pFab = (FRIEND_ASSOCIATEBEVY*)pData;

	std::_tstring group;
	CFriendMgr::_BEVY bevy;

	size_t rolessize = 0;

	{{
	char* szGroup = (char*)(pFab + 1);
	if (!gIsLegalString(szGroup, 0, _GROUP_NAME_LEN))
		return;

	group.assign(szGroup);

	char* szRoles = szGroup + group.size() + 1;
	while (*szRoles)
	{
		if (!gIsLegalString(szRoles, 1, _NAME_LEN))
			return;

		std::_tstring role(szRoles);
		bevy.push_back(role);

		szRoles += role.size() + 1;
		rolessize += role.size() + 1;
	}

	if (bevy.size() < 2)
		return;
	}}


	g_FriendMgr.PlayerAssociateBevy(bevy, group);


	{{
	size_t basesize = sizeof(FRIEND_SYNCASSOCIATE) + group.size() + 1;
	size_t maxsize = basesize + rolessize + 1;

	FRIEND_SYNCASSOCIATE* pFsa = (FRIEND_SYNCASSOCIATE*)_alloca(maxsize);
	pFsa->ProtocolFamily = pf_friend;
	pFsa->ProtocolID = friend_s2c_syncassociate;

	char* szGroupSync = (char*)(pFsa + 1);
	strcpy(szGroupSync, group.c_str());

	char* szRolesSync = szGroupSync + group.size() + 1;


	for (CFriendMgr::_BEVY::iterator itS = bevy.begin(); itS != bevy.end(); itS++)
	{
		std::_tstring someone = *itS;

		CNetConnectDup conndup;
		DWORD nameid = 0;
		unsigned long param = 0;
		if (!g_HostServer.FindPlayerByRole(NULL, someone, &conndup, NULL, &nameid, &param))
			continue;

		size_t cursor = 0;
		for (CFriendMgr::_BEVY::iterator itD = bevy.begin(); itD != bevy.end(); itD++)
		{
			if (itD == itS)
				continue;

			std::_tstring dst = *itD;

			strcpy(szRolesSync + cursor, dst.c_str());
			cursor += dst.size() + 1;
		}
		szRolesSync[cursor++] = 0;

		PassToSomeone(conndup.GetIP(), param, nameid, pFsa, basesize + cursor);
	}
	}}
}
