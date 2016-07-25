// KTongSet.cpp: implementation of the CTongSet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "TongDB.h"
#include "KTongSet.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTongSet::CTongSet()
{
	m_pcTong = NULL;
	m_nTongPointSize = 0;
	Init();
}

CTongSet::~CTongSet()
{
	DeleteAll();
}

void	CTongSet::Init()
{
	DeleteAll();

	m_pcTong = (CTongControl**)new LPVOID[defTONG_SET_INIT_POINT_NUM];
	m_nTongPointSize = defTONG_SET_INIT_POINT_NUM;
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		m_pcTong[i] = NULL;
	}
}

void	CTongSet::DeleteAll()
{
	if (m_pcTong)
	{
		for (int i = 0; i < m_nTongPointSize; i++)
		{
			if (m_pcTong[i])
			{
				delete m_pcTong[i];
				m_pcTong[i] = NULL;
			}
		}
		delete []m_pcTong;
		m_pcTong = NULL;
	}
	m_nTongPointSize = 0;
}

int		CTongSet::Create(int nCamp, char *lpszPlayerName, char *lpszTongName)
{
	// 帮会模块出错
	if (!m_pcTong || m_nTongPointSize <= 0)
		return enumTONG_CREATE_ERROR_ID10;
	// 名字字符串出错
	if (!lpszPlayerName || !lpszTongName)
		return enumTONG_CREATE_ERROR_ID11;
	// 名字字符串过长
	if (strlen(lpszTongName) >= defTONG_STR_LENGTH ||
		strlen(lpszPlayerName) >= defTONG_STR_LENGTH)
		return enumTONG_CREATE_ERROR_ID12;

	int		i, nPos;
	DWORD	dwTongNameID, dwPlayerNameID;

	dwTongNameID = g_String2Id(lpszTongName);
	dwPlayerNameID = g_String2Id(lpszPlayerName);

	// 名称检测，是否有同名的帮主或者同名的帮会
	for (i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && (m_pcTong[i]->m_dwNameID == dwTongNameID || m_pcTong[i]->m_dwMasterID == dwPlayerNameID))
			break;
	}
	// 帮会同名错误
	if (i < m_nTongPointSize)
		return enumTONG_CREATE_ERROR_ID13;

	// 查找空位
	nPos = -1;
	for (i = 0; i < m_nTongPointSize; i++)
	{
		if (!m_pcTong[i])
		{
			nPos = i;
			break;
		}
	}
	// 现有指针空间已满，分配新的更大的指针空间
	if (nPos < 0)
	{
		// 临时存储旧的指针
		CTongControl**	pTemp;
		pTemp = (CTongControl**)new LPVOID[m_nTongPointSize];
		for (i = 0; i < m_nTongPointSize; i++)
			pTemp[i] = m_pcTong[i];

		// 分配新的更大的指针空间，大小是原来的两倍
		delete []m_pcTong;
		m_pcTong = NULL;
		m_pcTong = (CTongControl**)new LPVOID[m_nTongPointSize * 2];
		for (i = 0; i < m_nTongPointSize; i++)
			m_pcTong[i] = pTemp[i];
		delete []pTemp;
		m_nTongPointSize *= 2;
		for (i = m_nTongPointSize / 2; i < m_nTongPointSize; i++)
			m_pcTong[i] = NULL;
		nPos = m_nTongPointSize / 2;
	}

	// 产生一个新的帮会
	m_pcTong[nPos] = new CTongControl(nCamp, lpszPlayerName, lpszTongName);
	// 产生失败
	if (m_pcTong[nPos]->m_dwNameID == 0)
	{
		delete m_pcTong[nPos];
		m_pcTong[nPos] = NULL;
		return enumTONG_CREATE_ERROR_ID14;
	}

	// 存盘，数据保存至数据库
	TMemberStruct	sMember;
	sMember.MemberClass = enumTONG_FIGURE_MASTER;
	sMember.nTitleIndex = 0;
	strcpy(sMember.szTong, m_pcTong[nPos]->m_szName);
	strcpy(sMember.szName, m_pcTong[nPos]->m_szMasterName);

	g_cTongDB.ChangeTong(*m_pcTong[nPos]);
	g_cTongDB.ChangeMember(sMember);

	return 0;
}

//----------------------------------------------------------------------
//	功能：添加一个帮会成员，if return == 0 成功 else return error id
//----------------------------------------------------------------------
int		CTongSet::AddMember(char *lpszPlayerName, char *lpszTongName)
{
	if (!m_pcTong || m_nTongPointSize <= 0)
		return -1;
	if (!lpszPlayerName || !lpszPlayerName[0] || !lpszTongName || !lpszTongName[0])
		return -1;
	if (strlen(lpszTongName) >= defTONG_STR_LENGTH ||
		strlen(lpszPlayerName) >= defTONG_STR_LENGTH)
		return -1;

	int		i;
	DWORD	dwTongNameID;

	dwTongNameID = g_String2Id(lpszTongName);

	// 寻找帮会
	for (i = 0; i < m_nTongPointSize; i++)
	{
		// 找到了
		if (m_pcTong[i] && (m_pcTong[i]->m_dwNameID == dwTongNameID))
		{
			if (!m_pcTong[i]->AddMember(lpszPlayerName))
				return -1;
			else
			{
				// 存盘，数据保存至数据库
				TMemberStruct	sMember;
				sMember.MemberClass = enumTONG_FIGURE_MEMBER;
				sMember.nTitleIndex = 0;
				strcpy(sMember.szTong, m_pcTong[i]->m_szName);
				strcpy(sMember.szName, lpszPlayerName);
				g_cTongDB.ChangeMember(sMember);

				// 给帮会频道发消息
				char	szMsg[96];
				sprintf(szMsg, "\\O%u", m_pcTong[i]->m_dwNameID);

				DWORD channid = g_ChannelMgr.GetChannelID(szMsg, 0);
				if (channid != -1)
				{
					sprintf(szMsg, "%s加入本帮！", lpszPlayerName);
					g_ChannelMgr.SayOnChannel(channid, TRUE, std::string(), std::string("公告"), std::string(szMsg));
				}

				return i;
			}
		}
	}

	return -1;
}

//----------------------------------------------------------------------
//	功能：获得帮会阵营
//----------------------------------------------------------------------
int		CTongSet::GetTongCamp(int nTongIdx)
{
	if (!m_pcTong || m_nTongPointSize <= 0)
		return -1;
	if (nTongIdx < 0 || nTongIdx >= m_nTongPointSize)
		return -1;

	if (m_pcTong[nTongIdx])
		return m_pcTong[nTongIdx]->m_nCamp;

	return -1;
}

BOOL	CTongSet::GetMasterName(int nTongIdx, char *lpszName)
{
	if (!lpszName)
		return FALSE;
	if (!m_pcTong || m_nTongPointSize <= 0)
		return FALSE;
	if (nTongIdx < 0 || nTongIdx >= m_nTongPointSize)
		return FALSE;
	if (!m_pcTong[nTongIdx])
		return FALSE;
	strcpy(lpszName, m_pcTong[nTongIdx]->m_szMasterName);
	return TRUE;
}

BOOL	CTongSet::GetMemberTitle(int nTongIdx, char *lpszTitle)
{
	if (!lpszTitle)
		return FALSE;
	if (!m_pcTong || m_nTongPointSize <= 0)
		return FALSE;
	if (nTongIdx < 0 || nTongIdx >= m_nTongPointSize)
		return FALSE;
	if (!m_pcTong[nTongIdx])
		return FALSE;
	strcpy(lpszTitle, m_pcTong[nTongIdx]->m_szNormalTitle);
	return TRUE;
}

BOOL	CTongSet::GetTongHeadInfo(DWORD dwTongNameID, STONG_HEAD_INFO_SYNC *pInfo)
{
	if (!m_pcTong || m_nTongPointSize <= 0 || dwTongNameID == 0)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == dwTongNameID)
		{
			return m_pcTong[i]->GetTongHeadInfo(pInfo);
		}
	}

	return FALSE;
}

BOOL	CTongSet::GetTongManagerInfo(STONG_GET_MANAGER_INFO_COMMAND *pApply, STONG_MANAGER_INFO_SYNC *pInfo)
{
	if (!pApply || !pInfo)
		return FALSE;
	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == (DWORD)pApply->m_nParam1)
		{
			return m_pcTong[i]->GetTongManagerInfo(pApply, pInfo);
		}
	}

	return FALSE;
}

BOOL	CTongSet::GetTongMemberInfo(STONG_GET_MEMBER_INFO_COMMAND *pApply, STONG_MEMBER_INFO_SYNC *pInfo)
{
	if (!pApply || !pInfo)
		return FALSE;
	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == (DWORD)pApply->m_nParam1)
		{
			return m_pcTong[i]->GetTongMemberInfo(pApply, pInfo);
		}
	}

	return FALSE;
}

BOOL	CTongSet::Instate(STONG_INSTATE_COMMAND *pInstate, STONG_INSTATE_SYNC *pSync)
{
	if (!pInstate || !pSync)
		return FALSE;
	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == pInstate->m_dwTongNameID)
		{
			return m_pcTong[i]->Instate(pInstate, pSync);
		}
	}

	return FALSE;
}

BOOL	CTongSet::Kick(STONG_KICK_COMMAND *pKick, STONG_KICK_SYNC *pSync)
{
	if (!pKick || !pSync)
		return FALSE;
	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == pKick->m_dwTongNameID)
		{
			return m_pcTong[i]->Kick(pKick, pSync);
		}
	}

	return FALSE;
}

BOOL	CTongSet::Leave(STONG_LEAVE_COMMAND *pLeave, STONG_LEAVE_SYNC *pSync)
{
	if (!pLeave || !pSync)
		return FALSE;
	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == pLeave->m_dwTongNameID)
		{
			return m_pcTong[i]->Leave(pLeave, pSync);
		}
	}

	return FALSE;
}

BOOL	CTongSet::AcceptMaster(STONG_ACCEPT_MASTER_COMMAND *pAccept)
{
	if (!pAccept)
		return FALSE;
	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == pAccept->m_dwTongNameID)
		{
			return m_pcTong[i]->AcceptMaster(pAccept);
		}
	}

	return FALSE;
}

BOOL	CTongSet::InitFromDB()
{
	int		nTongNum;

	nTongNum = g_cTongDB.GetTongCount();
	if (nTongNum < 0)
		return FALSE;
	if (nTongNum == 0)
		return TRUE;

	if (nTongNum > m_nTongPointSize)
	{
		DeleteAll();
		m_pcTong = (CTongControl**)new LPVOID[nTongNum + defTONG_SET_INIT_POINT_NUM];
		m_nTongPointSize = nTongNum + defTONG_SET_INIT_POINT_NUM;
		for (int i = 0; i < m_nTongPointSize; i++)
		{
			m_pcTong[i] = NULL;
		}
	}

	int		i;
	TTongList	*pList = new TTongList[nTongNum];
	memset(pList, 0, sizeof(TTongList) * nTongNum);

	int nGetNum = g_cTongDB.GetTongList(pList, nTongNum);
	if (nGetNum <= 0)
		return TRUE;

	for (i = 0; i < nGetNum; i++)
	{
		m_pcTong[i] = new CTongControl(pList[i]);
		if (!m_pcTong[i]->m_szName[0])
		{
			delete m_pcTong[i];
			m_pcTong[i] = NULL;
		}
	}

	for (i = 0; i < nGetNum; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_szName[0])
			g_cTongDB.SearchTong(m_pcTong[i]->m_szName, *(m_pcTong[i]));
	}

	if (pList)
	{
		delete []pList;
	}

	return TRUE;
}

BOOL	CTongSet::GetLoginData(STONG_GET_LOGIN_DATA_COMMAND *pLogin, STONG_LOGIN_DATA_SYNC *pSync)
{
	if (!pLogin || !pSync)
		return FALSE;

	memset(pSync, 0, sizeof(STONG_LOGIN_DATA_SYNC));
	pSync->ProtocolFamily	= pf_tong;
	pSync->ProtocolID		= enumS2C_TONG_LOGIN_DATA;
	pSync->m_btFlag			= 0;
	pSync->m_dwParam		= pLogin->m_dwParam;

	if (!m_pcTong)
		return FALSE;

	// 寻找帮会
	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == pLogin->m_dwTongNameID)
		{
			return m_pcTong[i]->GetLoginData(pLogin, pSync);
		}
	}

	return TRUE;
}

BOOL	CTongSet::SearchOne(DWORD dwTongNameID, char *lpszName, STONG_ONE_LEADER_INFO *pInfo)
{
	if (!m_pcTong)
		return FALSE;
	if (dwTongNameID == 0 || !lpszName || !lpszName[0] || !pInfo)
		return FALSE;

	for (int i = 0; i < m_nTongPointSize; i++)
	{
		if (m_pcTong[i] && m_pcTong[i]->m_dwNameID == dwTongNameID)
		{
			return m_pcTong[i]->SearchOne(lpszName, pInfo);
		}
	}

	return FALSE;
}









