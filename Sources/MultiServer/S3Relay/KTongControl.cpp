// KTongControl.cpp: implementation of the CTongControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Global.h"
#include "KTongControl.h"

#define		defTONG_INIT_MEMBER_SIZE		100		// 成员内存初始化时的大小
#define		defTONG_MEMBER_SIZE_ADD			100		// 成员内存每次增加的大小

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTongControl::CTongControl(int nCamp, char *lpszPlayerName, char *lpszTongName)
{
	m_nCamp			= 0;
	m_dwMoney		= 0;
	m_nCredit		= 0;
	m_nLevel		= 0;
	m_nDirectorNum	= 0;
	m_nManagerNum	= 0;
	m_nMemberNum	= 0;

	m_dwNameID		= 0;
	m_szName[0]		= 0;
	m_szPassword[0]	= 0;

	m_szMasterTitle[0] = 0;
	memset(m_szDirectorTitle, 0, sizeof(m_szDirectorTitle));
	memset(m_szManagerTitle, 0, sizeof(m_szManagerTitle));
	m_szNormalTitle[0] = 0;

	m_dwMasterID = 0;
	m_szMasterName[0] = 0;

	memset(m_dwDirectorID, 0, sizeof(m_dwDirectorID));
	memset(m_szDirectorName, 0, sizeof(m_szDirectorName));

	memset(m_dwManagerID, 0, sizeof(m_dwManagerID));
	memset(m_szManagerName, 0, sizeof(m_szManagerName));

	m_psMember = (STONG_MEMBER*)new STONG_MEMBER[defTONG_INIT_MEMBER_SIZE];
	m_nMemberPointSize = defTONG_INIT_MEMBER_SIZE;
	memset(m_psMember, 0, sizeof(STONG_MEMBER) * m_nMemberPointSize);

	if (nCamp != camp_justice && nCamp != camp_evil && nCamp != camp_balance)
		return;
	if (!lpszPlayerName || !lpszPlayerName[0] || strlen(lpszPlayerName) >= defTONG_STR_LENGTH)
		return;
	if (!lpszTongName || !lpszTongName[0] || strlen(lpszTongName) >= defTONG_STR_LENGTH)
		return;
	m_nCamp = nCamp;
	strcpy(m_szMasterName, lpszPlayerName);
	m_dwMasterID = g_String2Id(m_szMasterName);
	strcpy(m_szName, lpszTongName);
	m_dwNameID = g_String2Id(m_szName);
}

CTongControl::CTongControl(TTongList sList)
{
	m_nCamp			= 0;
	m_dwMoney		= 0;
	m_nCredit		= 0;
	m_nLevel		= 0;
	m_nDirectorNum	= 0;
	m_nManagerNum	= 0;

	strcpy(m_szName, sList.szName);
	m_dwNameID		= g_String2Id(m_szName);
	m_szPassword[0]	= 0;

	m_szMasterTitle[0] = 0;
	memset(m_szDirectorTitle, 0, sizeof(m_szDirectorTitle));
	memset(m_szManagerTitle, 0, sizeof(m_szManagerTitle));
	m_szNormalTitle[0] = 0;

	m_dwMasterID = 0;
	m_szMasterName[0] = 0;

	memset(m_dwDirectorID, 0, sizeof(m_dwDirectorID));
	memset(m_szDirectorName, 0, sizeof(m_szDirectorName));

	memset(m_dwManagerID, 0, sizeof(m_dwManagerID));
	memset(m_szManagerName, 0, sizeof(m_szManagerName));

	m_nMemberNum = sList.MemberCount;
	m_nMemberPointSize = m_nMemberNum + defTONG_INIT_MEMBER_SIZE;
	m_psMember = (STONG_MEMBER*)new STONG_MEMBER[m_nMemberPointSize];
	memset(m_psMember, 0, sizeof(STONG_MEMBER) * m_nMemberPointSize);
}

CTongControl::~CTongControl()
{
	if (m_psMember)
		delete []m_psMember;
}

BOOL	CTongControl::AddMember(char *lpszPlayerName)
{
	// 这种情况不应该发生
	if (m_nMemberPointSize <= 0 || !m_psMember)
		return FALSE;
	if (!lpszPlayerName || strlen(lpszPlayerName) >= defTONG_STR_LENGTH)
		return FALSE;

	// 寻找空位
	for (int i = 0; i < m_nMemberPointSize; i++)
	{
		if (m_psMember[i].m_dwNameID == 0)
			break;
	}
	// 没找着，满了，开辟一块更大的内存

int i;
	if (i >= m_nMemberPointSize)
	{
		i = m_nMemberPointSize;
		// 暂时备份一下
		STONG_MEMBER	*pTemp;
		pTemp = (STONG_MEMBER*)new STONG_MEMBER[m_nMemberPointSize];
		memcpy(pTemp, m_psMember, sizeof(STONG_MEMBER) * m_nMemberPointSize);

		// 分配更大的内存
		delete []m_psMember;
		m_psMember = (STONG_MEMBER*)new STONG_MEMBER[m_nMemberPointSize + defTONG_MEMBER_SIZE_ADD];
		memcpy(m_psMember, pTemp, sizeof(STONG_MEMBER) * m_nMemberPointSize);
		memset(&m_psMember[m_nMemberPointSize], 0, sizeof(STONG_MEMBER) * defTONG_MEMBER_SIZE_ADD);
		m_nMemberPointSize += defTONG_MEMBER_SIZE_ADD;

		delete []pTemp;
	}

	// 增加一个成员
	strcpy(m_psMember[i].m_szName, lpszPlayerName);
	m_psMember[i].m_dwNameID = g_String2Id(lpszPlayerName);
	m_nMemberNum++;

	return TRUE;
}

BOOL	CTongControl::ChangePassword(char *lpOld, char *lpNew)
{
	if (!lpOld || !lpNew)
		return FALSE;
	if (strlen(lpNew) >= defTONG_STR_LENGTH || strcmp(lpOld, m_szPassword) != 0)
		return FALSE;

	strcpy(m_szPassword, lpNew);

	return TRUE;
}

BOOL	CTongControl::GetTongHeadInfo(STONG_HEAD_INFO_SYNC *pInfo)
{
	if (!pInfo)
		return FALSE;

	int		i, j;

	pInfo->ProtocolFamily = pf_tong;
	pInfo->ProtocolID = enumS2C_TONG_HEAD_INFO;
	pInfo->m_dwMoney = m_dwMoney;
	pInfo->m_nCredit = m_nCredit;
	pInfo->m_btCamp = m_nCamp;
	pInfo->m_btLevel = m_nLevel;
	pInfo->m_btManagerNum = m_nManagerNum;
	pInfo->m_dwMemberNum = m_nMemberNum;
	strcpy(pInfo->m_szTongName, m_szName);

	pInfo->m_sMember[0].m_btFigure = enumTONG_FIGURE_MASTER;
	pInfo->m_sMember[0].m_btPos = 0;
	strcpy(pInfo->m_sMember[0].m_szTitle, this->m_szMasterTitle);
	strcpy(pInfo->m_sMember[0].m_szName, this->m_szMasterName);

	pInfo->m_btDirectorNum = 0;
	for (i = 0, j = 1; i < defTONG_MAX_DIRECTOR; i++)
	{
		if (!m_szDirectorName[i][0])
			continue;
		pInfo->m_sMember[j].m_btFigure = enumTONG_FIGURE_DIRECTOR;
		pInfo->m_sMember[j].m_btPos = i;
		strcpy(pInfo->m_sMember[j].m_szTitle, this->m_szDirectorTitle[i]);
		strcpy(pInfo->m_sMember[j].m_szName, this->m_szDirectorName[i]);
		j++;
		pInfo->m_btDirectorNum++;
	}

	pInfo->m_wLength = sizeof(STONG_HEAD_INFO_SYNC) - sizeof(pInfo->m_sMember) + sizeof(STONG_ONE_LEADER_INFO) * (1 + pInfo->m_btDirectorNum);

	return TRUE;
}

BOOL	CTongControl::GetTongManagerInfo(
			STONG_GET_MANAGER_INFO_COMMAND *pApply,
			STONG_MANAGER_INFO_SYNC *pInfo)
{
	if (!pApply || !pInfo)
		return FALSE;
	if (pApply->m_nParam3 <= 0)
		return FALSE;

	pInfo->ProtocolFamily	= pf_tong;
	pInfo->ProtocolID		= enumS2C_TONG_MANAGER_INFO;
	pInfo->m_dwParam		= pApply->m_dwParam;
	pInfo->m_dwMoney		= m_dwMoney;
	pInfo->m_nCredit		= m_nCredit;
	pInfo->m_btCamp			= m_nCamp;
	pInfo->m_btLevel		= m_nLevel;
	pInfo->m_btDirectorNum	= m_nDirectorNum;
	pInfo->m_btManagerNum	= m_nManagerNum;
	pInfo->m_dwMemberNum	= m_nMemberNum;
	pInfo->m_btStartNo		= pApply->m_nParam2;
	strcpy(pInfo->m_szTongName, m_szName);

	int		nNeedNum, nStartNum, i, j;

	nStartNum = pApply->m_nParam2;
	nNeedNum = pApply->m_nParam3;
	if (nNeedNum > defTONG_ONE_PAGE_MAX_NUM)
		nNeedNum = defTONG_ONE_PAGE_MAX_NUM;

	pInfo->m_btCurNum = 0;
	// 找开始位置
	for (i = 0, j = 0; i < defTONG_MAX_MANAGER; i++)
	{
		if (j >= nStartNum)
			break;
		if (m_szManagerName[i][0])
			j++;
	}
	if (i >= defTONG_MAX_MANAGER || j < nStartNum)
		return FALSE;

	// 开始记录数据
	for (; i < defTONG_MAX_MANAGER; i++)
	{
		if (pInfo->m_btCurNum >= nNeedNum)
			break;
		if (!m_szManagerName[i][0])
			continue;

		pInfo->m_sMember[pInfo->m_btCurNum].m_btFigure = enumTONG_FIGURE_MANAGER;
		pInfo->m_sMember[pInfo->m_btCurNum].m_btPos = i;
		strcpy(pInfo->m_sMember[pInfo->m_btCurNum].m_szTitle, m_szManagerTitle[i]);
		strcpy(pInfo->m_sMember[pInfo->m_btCurNum].m_szName, m_szManagerName[i]);

		pInfo->m_btCurNum++;
	}
	if (pInfo->m_btCurNum == 0)
		return FALSE;

	pInfo->m_wLength = sizeof(STONG_MANAGER_INFO_SYNC) - sizeof(STONG_ONE_LEADER_INFO) * (defTONG_ONE_PAGE_MAX_NUM - pInfo->m_btCurNum);

	return TRUE;
}

BOOL	CTongControl::GetTongMemberInfo(
			STONG_GET_MEMBER_INFO_COMMAND *pApply,
			STONG_MEMBER_INFO_SYNC *pInfo)
{
	if (!pApply || !pInfo)
		return FALSE;
	if (pApply->m_nParam3 <= 0)
		return FALSE;
	if (!m_psMember || m_nMemberPointSize <= 0)
		return FALSE;

	pInfo->ProtocolFamily	= pf_tong;
	pInfo->ProtocolID		= enumS2C_TONG_MEMBER_INFO;
	pInfo->m_dwParam		= pApply->m_dwParam;
	pInfo->m_dwMoney		= m_dwMoney;
	pInfo->m_nCredit		= m_nCredit;
	pInfo->m_btCamp			= m_nCamp;
	pInfo->m_btLevel		= m_nLevel;
	pInfo->m_btDirectorNum	= m_nDirectorNum;
	pInfo->m_btManagerNum	= m_nManagerNum;
	pInfo->m_dwMemberNum	= m_nMemberNum;
	pInfo->m_btStartNo		= pApply->m_nParam2;
	strcpy(pInfo->m_szTitle, m_szNormalTitle);
	strcpy(pInfo->m_szTongName, m_szName);

	int		nNeedNum, nStartNum, i, j;

	nStartNum = pApply->m_nParam2;
	nNeedNum = pApply->m_nParam3;
	if (nNeedNum > defTONG_ONE_PAGE_MAX_NUM)
		nNeedNum = defTONG_ONE_PAGE_MAX_NUM;

	pInfo->m_btCurNum = 0;
	// 找开始位置
	for (i = 0, j = 0; i < m_nMemberPointSize; i++)
	{
		if (j >= nStartNum)
			break;
		if (m_psMember[i].m_szName[0])
			j++;
	}
	if (i >= m_nMemberPointSize || j < nStartNum)
		return FALSE;

	// 开始记录数据
	for (; i < m_nMemberPointSize; i++)
	{
		if (pInfo->m_btCurNum >= nNeedNum)
			break;
		if (!m_psMember[i].m_szName[0])
			continue;

		strcpy(pInfo->m_sMember[pInfo->m_btCurNum].m_szName, m_psMember[i].m_szName);

		pInfo->m_btCurNum++;
	}
	if (pInfo->m_btCurNum == 0)
		return FALSE;

	pInfo->m_wLength = sizeof(STONG_MEMBER_INFO_SYNC) - sizeof(STONG_ONE_MEMBER_INFO) * (defTONG_ONE_PAGE_MAX_NUM - pInfo->m_btCurNum);

	return TRUE;
}

BOOL	CTongControl::Instate(STONG_INSTATE_COMMAND *pInstate, STONG_INSTATE_SYNC *pSync)
{
	if (!pInstate || !pSync)
		return FALSE;
	if (pInstate->m_btCurFigure == pInstate->m_btNewFigure && pInstate->m_btCurPos == pInstate->m_btNewPos)
		return FALSE;

	int		i, nOldPos, nNewPos;
	DWORD	dwNameID;
	char	szName[32];

	memcpy(szName, pInstate->m_szName, sizeof(pInstate->m_szName));
	szName[31] = 0;
	dwNameID = g_String2Id(szName);
	if (dwNameID == 0)
		return FALSE;

	pSync->ProtocolFamily	= pf_tong;
	pSync->ProtocolID		= enumS2C_TONG_INSTATE;
	pSync->m_btOldFigure	= pInstate->m_btCurFigure;
	pSync->m_btOldPos		= 0;
	pSync->m_btNewFigure	= pInstate->m_btNewFigure;
	pSync->m_btNewPos		= 0;
	pSync->m_dwParam		= pInstate->m_dwParam;
	pSync->m_btSuccessFlag	= 0;
	pSync->m_dwTongNameID	= pInstate->m_dwTongNameID;
	pSync->m_szTitle[0]		= 0;
	strcpy(pSync->m_szName, szName);

	if (pInstate->m_btCurFigure == enumTONG_FIGURE_DIRECTOR)
	{
		for (i = 0; i < defTONG_MAX_DIRECTOR; i++)
		{
			if (m_dwDirectorID[i] == dwNameID && strcmp(szName, m_szDirectorName[i]) == 0)
				break;
		}
		if (i >= defTONG_MAX_DIRECTOR)
			return FALSE;
		nOldPos = i;

		switch (pInstate->m_btNewFigure)
		{
		case enumTONG_FIGURE_MANAGER:
			for (i = 0; i < defTONG_MAX_MANAGER; i++)
			{
				if (m_dwManagerID[i] == 0 && !m_szManagerName[i][0])
					break;
			}
			if (i >= defTONG_MAX_MANAGER)
				return FALSE;
			nNewPos = i;

			strcpy(m_szManagerName[nNewPos], szName);
			m_dwManagerID[nNewPos] = dwNameID;
			m_szDirectorName[nOldPos][0] = 0;
			m_dwDirectorID[nOldPos] = 0;
			m_nDirectorNum--;
			m_nManagerNum++;
			strcpy(pSync->m_szTitle, m_szManagerTitle[nNewPos]);
			break;
		case enumTONG_FIGURE_MEMBER:
			if (!AddMember(szName))
				return FALSE;
			m_szDirectorName[nOldPos][0] = 0;
			m_dwDirectorID[nOldPos] = 0;
			m_nDirectorNum--;
			strcpy(pSync->m_szTitle, m_szNormalTitle);
			break;
		default:
			return FALSE;
		}
	}
	else if (pInstate->m_btCurFigure == enumTONG_FIGURE_MANAGER)
	{
		for (i = 0; i < defTONG_MAX_MANAGER; i++)
		{
			if (m_dwManagerID[i] == dwNameID && strcmp(szName, m_szManagerName[i]) == 0)
				break;
		}
		if (i >= defTONG_MAX_MANAGER)
			return FALSE;
		nOldPos = i;

		switch (pInstate->m_btNewFigure)
		{
		case enumTONG_FIGURE_DIRECTOR:
			for (i = 0; i < defTONG_MAX_DIRECTOR; i++)
			{
				if (m_dwDirectorID[i] == 0 && !m_szDirectorName[i][0])
					break;
			}
			if (i >= defTONG_MAX_DIRECTOR)
				return FALSE;
			nNewPos = i;

			strcpy(m_szDirectorName[nNewPos], szName);
			m_dwDirectorID[nNewPos] = dwNameID;
			m_szManagerName[nOldPos][0] = 0;
			m_dwManagerID[nOldPos] = 0;
			m_nDirectorNum++;
			m_nManagerNum--;
			strcpy(pSync->m_szTitle, m_szDirectorTitle[nNewPos]);
			break;
		case enumTONG_FIGURE_MEMBER:
			if (!AddMember(szName))
				return FALSE;
			m_szManagerName[nOldPos][0] = 0;
			m_dwManagerID[nOldPos] = 0;
			m_nManagerNum--;
			strcpy(pSync->m_szTitle, m_szNormalTitle);
			break;
		default:
			return FALSE;
		}
	}
	else if (pInstate->m_btCurFigure == enumTONG_FIGURE_MEMBER)
	{
		if (!m_psMember)
			return FALSE;
		int		nPos = -1;

		for (int i = 0; i < m_nMemberPointSize; i++)
		{
			if (m_psMember[i].m_dwNameID != dwNameID)
				continue;
			nPos = i;
			break;
		}
		if (nPos < 0)
			return FALSE;
		nOldPos = 0;

		switch (pInstate->m_btNewFigure)
		{
		case enumTONG_FIGURE_DIRECTOR:
			for (i = 0; i < defTONG_MAX_DIRECTOR; i++)
			{
				if (m_dwDirectorID[i] == 0 && !m_szDirectorName[i][0])
					break;
			}
			if (i >= defTONG_MAX_DIRECTOR)
				return FALSE;
			nNewPos = i;

			strcpy(m_szDirectorName[nNewPos], szName);
			m_dwDirectorID[nNewPos] = dwNameID;
			this->m_psMember[nPos].m_szName[0] = 0;
			this->m_psMember[nPos].m_dwNameID = 0;
			m_nDirectorNum++;
			m_nMemberNum--;
			strcpy(pSync->m_szTitle, m_szDirectorTitle[nNewPos]);
			break;
		case enumTONG_FIGURE_MANAGER:
			for (i = 0; i < defTONG_MAX_MANAGER; i++)
			{
				if (m_dwManagerID[i] == 0 && !m_szManagerName[i][0])
					break;
			}
			if (i >= defTONG_MAX_MANAGER)
				return FALSE;
			nNewPos = i;

			strcpy(m_szManagerName[nNewPos], szName);
			m_dwManagerID[nNewPos] = dwNameID;
			this->m_psMember[nPos].m_szName[0] = 0;
			this->m_psMember[nPos].m_dwNameID = 0;
			m_nManagerNum++;
			m_nMemberNum--;
			strcpy(pSync->m_szTitle, m_szManagerTitle[nNewPos]);
			break;
		default:
			return FALSE;
		}
	}
	pSync->m_btSuccessFlag = 1;
	pSync->m_btOldPos = nOldPos;
	pSync->m_btNewPos = nNewPos;

	{{
		CNetConnectDup conndup;
		DWORD nameid = 0;
		unsigned long param = 0;
		if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(szName), &conndup, NULL, &nameid, &param))
		{
			CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup.GetIP());
			if (tongconndup.IsValid())
			{
				STONG_BE_INSTATED_SYNC	sSync;

				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_BE_INSTATED;
				sSync.m_btFigure = pInstate->m_btNewFigure;
				sSync.m_btPos = nNewPos;
				sSync.m_dwParam = param;
				strcpy(sSync.m_szName, szName);
				switch (pInstate->m_btNewFigure)
				{
				case enumTONG_FIGURE_DIRECTOR:
					strcpy(sSync.m_szTitle, m_szDirectorTitle[nNewPos]);
					break;
				case enumTONG_FIGURE_MANAGER:
					strcpy(sSync.m_szTitle, m_szManagerTitle[nNewPos]);
					break;
				case enumTONG_FIGURE_MEMBER:
					strcpy(sSync.m_szTitle, this->m_szNormalTitle);
					break;
				}

				tongconndup.SendPackage((const void *)&sSync, sizeof(sSync));
			}
			else
			{
				//tong is not connect
			}

		}
		else
		{
			//not find
		}
	}}

	// 向帮会频道发消息，某人被任命
	char	szMsg[96];
	sprintf(szMsg, "\\O%u", m_dwNameID);

	DWORD channid = g_ChannelMgr.GetChannelID(szMsg, 0);
	if (channid != -1)
	{
		if (pSync->m_szTitle[0])
			sprintf(szMsg, "%s被任命为%s！", szName, pSync->m_szTitle);
		else
		{
			switch (pSync->m_btNewFigure)
			{
			case enumTONG_FIGURE_DIRECTOR:
				sprintf(szMsg, "%s被任命为长老！", szName);
				break;
			case enumTONG_FIGURE_MANAGER:
				sprintf(szMsg, "%s被任命为队长！", szName);
				break;
			case enumTONG_FIGURE_MEMBER:
				sprintf(szMsg, "%s被任命为普通帮众！", szName);
				break;
			}
		}
		g_ChannelMgr.SayOnChannel(channid, TRUE, std::string(), std::string("公告"), std::string(szMsg));
	}

	// 存盘，数据保存至数据库
	TMemberStruct	sMember;
	sMember.MemberClass = (TONG_MEMBER_FIGURE)pInstate->m_btNewFigure;
	sMember.nTitleIndex = nNewPos;
	strcpy(sMember.szTong, this->m_szName);
	strcpy(sMember.szName, szName);
	g_cTongDB.ChangeMember(sMember);

	return TRUE;
}

/*
BOOL	CTongControl::Instate(STONG_INSTATE_COMMAND *pInstate, STONG_INSTATE_SYNC *pSync)
{
	if (!pInstate || !pSync)
		return FALSE;
	if (pInstate->m_btCurFigure == pInstate->m_btNewFigure && pInstate->m_btCurPos == pInstate->m_btNewPos)
		return FALSE;

	char	szName[32];

	memcpy(szName, pInstate->m_szName, sizeof(pInstate->m_szName));
	szName[31] = 0;

	pSync->ProtocolFamily	= pf_tong;
	pSync->ProtocolID		= enumS2C_TONG_INSTATE;
	pSync->m_btOldFigure	= pInstate->m_btCurFigure;
	pSync->m_btOldPos		= pInstate->m_btCurPos;
	pSync->m_btNewFigure	= pInstate->m_btNewFigure;
	pSync->m_btNewPos		= pInstate->m_btNewPos;
	pSync->m_dwParam		= pInstate->m_dwParam;
	pSync->m_btSuccessFlag	= 0;
	pSync->m_dwTongNameID	= pInstate->m_dwTongNameID;
	pSync->m_szTitle[0]		= 0;
	strcpy(pSync->m_szName, szName);

	if (pInstate->m_btCurFigure == enumTONG_FIGURE_DIRECTOR)
	{
		if (pInstate->m_btCurPos >= defTONG_MAX_DIRECTOR)
			return FALSE;
		if (!m_szDirectorName[pInstate->m_btCurPos][0] ||
			strcmp(szName, m_szDirectorName[pInstate->m_btCurPos]) != 0)
			return FALSE;
		switch (pInstate->m_btNewFigure)
		{
		case enumTONG_FIGURE_DIRECTOR:
			if (pInstate->m_btNewPos >= defTONG_MAX_DIRECTOR)
				return FALSE;
			if (m_szDirectorName[pInstate->m_btNewPos][0])
				return FALSE;
			strcpy(m_szDirectorName[pInstate->m_btNewPos], szName);
			m_dwDirectorID[pInstate->m_btNewPos] = g_String2Id(szName);
			m_szDirectorName[pInstate->m_btCurPos][0] = 0;
			m_dwDirectorID[pInstate->m_btCurPos] = 0;
			strcpy(pSync->m_szTitle, m_szDirectorTitle[pInstate->m_btNewPos]);
			break;
		case enumTONG_FIGURE_MANAGER:
			if (pInstate->m_btNewPos >= defTONG_MAX_MANAGER)
				return FALSE;
			if (m_szManagerName[pInstate->m_btNewPos][0])
				return FALSE;
			strcpy(m_szManagerName[pInstate->m_btNewPos], szName);
			m_dwManagerID[pInstate->m_btNewPos] = g_String2Id(szName);
			m_szDirectorName[pInstate->m_btCurPos][0] = 0;
			m_dwDirectorID[pInstate->m_btCurPos] = 0;
			m_nDirectorNum--;
			m_nManagerNum++;
			strcpy(pSync->m_szTitle, m_szManagerTitle[pInstate->m_btNewPos]);
			break;
		case enumTONG_FIGURE_MEMBER:
			if (!AddMember(szName))
				return FALSE;
			m_szDirectorName[pInstate->m_btCurPos][0] = 0;
			m_dwDirectorID[pInstate->m_btCurPos] = 0;
			m_nDirectorNum--;
			strcpy(pSync->m_szTitle, m_szNormalTitle);
			break;
		default:
			return FALSE;
		}
	}
	else if (pInstate->m_btCurFigure == enumTONG_FIGURE_MANAGER)
	{
		if (pInstate->m_btCurPos >= defTONG_MAX_MANAGER)
			return FALSE;
		if (!m_szManagerName[pInstate->m_btCurPos][0] ||
			strcmp(szName, m_szManagerName[pInstate->m_btCurPos]) != 0)
			return FALSE;
		switch (pInstate->m_btNewFigure)
		{
		case enumTONG_FIGURE_DIRECTOR:
			if (pInstate->m_btNewPos >= defTONG_MAX_DIRECTOR)
				return FALSE;
			if (m_szDirectorName[pInstate->m_btNewPos][0])
				return FALSE;
			strcpy(m_szDirectorName[pInstate->m_btNewPos], szName);
			m_dwDirectorID[pInstate->m_btNewPos] = g_String2Id(szName);
			m_szManagerName[pInstate->m_btCurPos][0] = 0;
			m_dwManagerID[pInstate->m_btCurPos] = 0;
			m_nDirectorNum++;
			m_nManagerNum--;
			strcpy(pSync->m_szTitle, m_szDirectorTitle[pInstate->m_btNewPos]);
			break;
		case enumTONG_FIGURE_MANAGER:
			if (pInstate->m_btNewPos >= defTONG_MAX_MANAGER)
				return FALSE;
			if (m_szManagerName[pInstate->m_btNewPos][0])
				return FALSE;
			strcpy(m_szManagerName[pInstate->m_btNewPos], szName);
			m_dwManagerID[pInstate->m_btNewPos] = g_String2Id(szName);
			m_szManagerName[pInstate->m_btCurPos][0] = 0;
			m_dwManagerID[pInstate->m_btCurPos] = 0;
			strcpy(pSync->m_szTitle, m_szManagerTitle[pInstate->m_btNewPos]);
			break;
		case enumTONG_FIGURE_MEMBER:
			if (!AddMember(szName))
				return FALSE;
			m_szManagerName[pInstate->m_btCurPos][0] = 0;
			m_dwManagerID[pInstate->m_btCurPos] = 0;
			m_nManagerNum--;
			strcpy(pSync->m_szTitle, m_szNormalTitle);
			break;
		default:
			return FALSE;
		}
	}
	else if (pInstate->m_btCurFigure == enumTONG_FIGURE_MEMBER)
	{
		if (!m_psMember)
			return FALSE;
		int		nPos = -1;
		DWORD	dwNameID = g_String2Id(szName);
		if (dwNameID == 0)
			return FALSE;
		for (int i = 0; i < m_nMemberPointSize; i++)
		{
			if (m_psMember[i].m_dwNameID != dwNameID)
				continue;
			nPos = i;
			break;
		}
		if (nPos < 0)
			return FALSE;

		switch (pInstate->m_btNewFigure)
		{
		case enumTONG_FIGURE_DIRECTOR:
			if (pInstate->m_btNewPos >= defTONG_MAX_DIRECTOR)
				return FALSE;
			if (m_szDirectorName[pInstate->m_btNewPos][0])
				return FALSE;
			strcpy(m_szDirectorName[pInstate->m_btNewPos], szName);
			m_dwDirectorID[pInstate->m_btNewPos] = g_String2Id(szName);
			this->m_psMember[nPos].m_szName[0] = 0;
			this->m_psMember[nPos].m_dwNameID = 0;
			m_nDirectorNum++;
			m_nMemberNum--;
			strcpy(pSync->m_szTitle, m_szDirectorTitle[pInstate->m_btNewPos]);
			break;
		case enumTONG_FIGURE_MANAGER:
			if (pInstate->m_btNewPos >= defTONG_MAX_MANAGER)
				return FALSE;
			if (m_szManagerName[pInstate->m_btNewPos][0])
				return FALSE;
			strcpy(m_szManagerName[pInstate->m_btNewPos], szName);
			m_dwManagerID[pInstate->m_btNewPos] = g_String2Id(szName);
			this->m_psMember[nPos].m_szName[0] = 0;
			this->m_psMember[nPos].m_dwNameID = 0;
			m_nManagerNum++;
			m_nMemberNum--;
			strcpy(pSync->m_szTitle, m_szManagerTitle[pInstate->m_btNewPos]);
			break;
		default:
			return FALSE;
		}
	}
	pSync->m_btSuccessFlag = 1;

	{{
		CNetConnectDup conndup;
		DWORD nameid = 0;
		unsigned long param = 0;
		if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(szName), &conndup, NULL, &nameid, &param))
		{
			CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup.GetIP());
			if (tongconndup.IsValid())
			{
				STONG_BE_INSTATED_SYNC	sSync;

				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_BE_INSTATED;
				sSync.m_btFigure = pInstate->m_btNewFigure;
				sSync.m_btPos = pInstate->m_btNewPos;
				sSync.m_dwParam = param;
				strcpy(sSync.m_szName, szName);
				switch (pInstate->m_btNewFigure)
				{
				case enumTONG_FIGURE_DIRECTOR:
					strcpy(sSync.m_szTitle, m_szDirectorTitle[pInstate->m_btNewPos]);
					break;
				case enumTONG_FIGURE_MANAGER:
					strcpy(sSync.m_szTitle, m_szManagerTitle[pInstate->m_btNewPos]);
					break;
				case enumTONG_FIGURE_MEMBER:
					strcpy(sSync.m_szTitle, this->m_szNormalTitle);
					break;
				}

				tongconndup.SendPackage((const void *)&sSync, sizeof(sSync));
			}
			else
			{
				//tong is not connect
			}

		}
		else
		{
			//not find
		}
	}}

	// 向帮会频道发消息，某人被任命
	char	szMsg[96];
	sprintf(szMsg, "\\O%u", m_dwNameID);

	DWORD channid = g_ChannelMgr.GetChannelID(szMsg, 0);
	if (channid != -1)
	{
		if (pSync->m_szTitle[0])
			sprintf(szMsg, "%s被任命为%s！", szName, pSync->m_szTitle);
		else
		{
			switch (pSync->m_btNewFigure)
			{
			case enumTONG_FIGURE_DIRECTOR:
				sprintf(szMsg, "%s被任命为长老！", szName);
				break;
			case enumTONG_FIGURE_MANAGER:
				sprintf(szMsg, "%s被任命为队长！", szName);
				break;
			case enumTONG_FIGURE_MEMBER:
				sprintf(szMsg, "%s被任命为普通帮众！", szName);
				break;
			}
		}
		g_ChannelMgr.SayOnChannel(channid, TRUE, std::string(), std::string("公告"), std::string(szMsg));
	}


	// 存盘，数据保存至数据库
	TMemberStruct	sMember;
	sMember.MemberClass = (TONG_MEMBER_FIGURE)pInstate->m_btNewFigure;
	sMember.nTitleIndex = pInstate->m_btNewPos;
	strcpy(sMember.szTong, this->m_szName);
	strcpy(sMember.szName, szName);
	g_cTongDB.ChangeMember(sMember);

	return TRUE;
}
*/

BOOL	CTongControl::Kick(STONG_KICK_COMMAND *pKick, STONG_KICK_SYNC *pSync)
{
	if (!pKick || !pSync)
		return FALSE;

	char	szName[32];
	DWORD	dwNameID;
	int		nKickPos;
	memcpy(szName, pKick->m_szName, sizeof(pKick->m_szName));
	szName[31] = 0;
	dwNameID = g_String2Id(szName);
	if (dwNameID == 0)
		return FALSE;

	pSync->ProtocolFamily	= pf_tong;
	pSync->ProtocolID		= enumS2C_TONG_KICK;
	pSync->m_btFigure		= pKick->m_btFigure;
	pSync->m_btPos			= pKick->m_btPos;
	pSync->m_dwParam		= pKick->m_dwParam;
	pSync->m_dwTongNameID	= pKick->m_dwTongNameID;
	pSync->m_btSuccessFlag	= 0;
	strcpy(pSync->m_szName, szName);

	if (pKick->m_btFigure == enumTONG_FIGURE_MANAGER)
	{
		if (pKick->m_btPos >= defTONG_MAX_MANAGER)
			return FALSE;
		for (int i = 0; i < defTONG_MAX_MANAGER; i++)
		{
			if (m_dwManagerID[i] == dwNameID && strcmp(m_szManagerName[i], szName) == 0)
				break;
		}
int i;
		if (i >= defTONG_MAX_MANAGER)
			return FALSE;
		nKickPos = i;
		this->m_szManagerName[i][0] = 0;
		this->m_dwManagerID[i] = 0;
		this->m_nManagerNum--;
	}
	else if (pKick->m_btFigure == enumTONG_FIGURE_MEMBER)
	{
		if (!m_psMember)
			return FALSE;

		int		nPos = -1;
		DWORD	dwNameID = g_String2Id(szName);
		if (dwNameID == 0)
			return FALSE;

		for (int i = 0; i < m_nMemberPointSize; i++)
		{
			if (m_psMember[i].m_dwNameID != dwNameID)
				continue;
			nPos = i;
			break;
		}
		if (nPos < 0)
			return FALSE;

		if (!m_psMember[nPos].m_szName[0] ||
			strcmp(m_psMember[nPos].m_szName, szName) != 0)
			return FALSE;
		m_psMember[nPos].m_szName[0] = 0;
		m_psMember[nPos].m_dwNameID = 0;
		m_nMemberNum--;
		nKickPos = 0;
	}
	else
	{
		return FALSE;
	}
	pSync->m_btSuccessFlag = 1;
	pSync->m_btPos = nKickPos;


	{{
		CNetConnectDup conndup;
		DWORD nameid = 0;
		unsigned long param = 0;
		if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(szName), &conndup, NULL, &nameid, &param))
		{
			CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup.GetIP());
			if (tongconndup.IsValid())
			{
				STONG_BE_KICKED_SYNC	sSync;

				sSync.ProtocolFamily = pf_tong;
				sSync.ProtocolID = enumS2C_TONG_BE_KICKED;
				sSync.m_btFigure = pKick->m_btFigure;
				sSync.m_btPos = nKickPos;
				sSync.m_dwParam = param;
				strcpy(sSync.m_szName, szName);

				tongconndup.SendPackage((const void *)&sSync, sizeof(sSync));
			}
			else
			{
				//tong is not connect
			}

		}
		else
		{
			//not find
		}
	}}

	// 存盘
	g_cTongDB.DelMember(szName);

	// 向帮会频道发消息，某人被踢出帮会
	char	szMsg[96];
	sprintf(szMsg, "\\O%u", m_dwNameID);

	DWORD channid = g_ChannelMgr.GetChannelID(szMsg, 0);
	if (channid != -1)
	{
		sprintf(szMsg, "%s被踢出帮会！", szName);
		g_ChannelMgr.SayOnChannel(channid, TRUE, std::string(), std::string("公告"), std::string(szMsg));
	}

	return TRUE;
}

BOOL	CTongControl::Leave(STONG_LEAVE_COMMAND *pLeave, STONG_LEAVE_SYNC *pSync)
{
	if (!pLeave || !pSync)
		return FALSE;

	char	szName[32];
	memcpy(szName, pLeave->m_szName, sizeof(pLeave->m_szName));
	szName[31] = 0;

	pSync->ProtocolFamily	= pf_tong;
	pSync->ProtocolID		= enumS2C_TONG_LEAVE;
	pSync->m_dwParam		= pLeave->m_dwParam;
	pSync->m_btSuccessFlag	= 0;
	strcpy(pSync->m_szName, szName);

	switch (pLeave->m_btFigure)
	{
	case enumTONG_FIGURE_MEMBER:
		{
			if (!m_psMember)
				break;
			int		nPos = -1;
			DWORD	dwNameID = g_String2Id(szName);
			if (dwNameID == 0)
				return FALSE;
			for (int i = 0; i < m_nMemberPointSize; i++)
			{
				if (m_psMember[i].m_dwNameID != dwNameID)
					continue;
				nPos = i;
				break;
			}
			if (nPos < 0)
				return FALSE;
			if (!m_psMember[nPos].m_szName[0] ||
				strcmp(m_psMember[nPos].m_szName, szName) != 0)
				return FALSE;

			m_psMember[nPos].m_szName[0] = 0;
			m_psMember[nPos].m_dwNameID = 0;
			m_nMemberNum--;
		}
		break;
	case enumTONG_FIGURE_MANAGER:
		{
			if (pLeave->m_btPos >= 0 && pLeave->m_btPos < defTONG_MAX_MANAGER)
			{
				if (strcmp(m_szManagerName[pLeave->m_btPos], szName) == 0)
				{
					m_szManagerName[pLeave->m_btPos][0] = 0;
					m_dwManagerID[pLeave->m_btPos] = 0;
					m_nManagerNum--;
					break;
				}
			}

			int		nPos = -1;
			DWORD	dwNameID = g_String2Id(szName);
			if (dwNameID == 0)
				return FALSE;
			for (int i = 0; i < defTONG_MAX_MANAGER; i++)
			{
				if (m_dwManagerID[i] != dwNameID)
					continue;
				nPos = i;
				break;
			}
			if (nPos < 0)
				return FALSE;
			if (!m_szManagerName[nPos][0] ||
				strcmp(szName, m_szManagerName[nPos]) != 0)
				return FALSE;
			m_szManagerName[nPos][0] = 0;
			m_dwManagerID[nPos] = 0;
			m_nManagerNum--;
		}
		break;
	case enumTONG_FIGURE_DIRECTOR:
		return FALSE;
	case enumTONG_FIGURE_MASTER:
		return FALSE;
	}
	pSync->m_btSuccessFlag = 1;

	// 存盘
	g_cTongDB.DelMember(szName);

	// 通过帮会频道发布消息：某某人叛帮
	char	szMsg[96];
	sprintf(szMsg, "\\O%u", m_dwNameID);

	DWORD channid = g_ChannelMgr.GetChannelID(szMsg, 0);
	if (channid != -1)
	{
		sprintf(szMsg, "%s叛出本帮！", szName);
		g_ChannelMgr.SayOnChannel(channid, TRUE, std::string(), std::string("公告"), std::string(szMsg));
	}

	return TRUE;
}

BOOL	CTongControl::AcceptMaster(STONG_ACCEPT_MASTER_COMMAND *pAccept)
{
	if (!pAccept)
		return FALSE;

	char	szName[32], szOldMaster[32];
	memcpy(szName, pAccept->m_szName, sizeof(pAccept->m_szName));
	szName[31] = 0;
	if (!szName[0])
		return FALSE;
	strcpy(szOldMaster, this->m_szMasterName);

	// 能力不够
	if (pAccept->m_btAcceptFalg == 0)
	{
		CNetConnectDup conndup;
		DWORD nameid = 0;
		unsigned long param = 0;
		if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(m_szMasterName), &conndup, NULL, &nameid, &param))
		{
			CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup.GetIP());
			if (tongconndup.IsValid())
			{
				STONG_CHANGE_MASTER_FAIL_SYNC	sFail;
				sFail.ProtocolFamily	= pf_tong;
				sFail.ProtocolID		= enumS2C_TONG_CHANGE_MASTER_FAIL;
				sFail.m_dwParam			= param;
				sFail.m_btFailID		= 1;
				sFail.m_dwTongNameID	= pAccept->m_dwTongNameID;
				memcpy(sFail.m_szName, szName, sizeof(szName));
				tongconndup.SendPackage((const void *)&sFail, sizeof(sFail));
			}
			else
			{
				//tong is not connect
			}
		}
		else
		{
			//not find
		}

		return FALSE;
	}

	DWORD	dwNameID;
	int		i;

	dwNameID = g_String2Id(szName);
	if (dwNameID == 0)
		return FALSE;
	if (pAccept->m_btFigure == enumTONG_FIGURE_DIRECTOR)
	{
		for (i = 0; i < defTONG_MAX_DIRECTOR; i++)
		{
			if (m_dwDirectorID[i] == dwNameID && strcmp(szName, m_szDirectorName[i]) == 0)
				break;
		}
		if (i >= defTONG_MAX_DIRECTOR)
			return FALSE;
		if (!AddMember(m_szMasterName))
			return FALSE;
		strcpy(this->m_szMasterName, szName);
		this->m_dwMasterID = dwNameID;
		m_szDirectorName[i][0] = 0;
		m_dwDirectorID[i] = 0;
		this->m_nDirectorNum--;
	}
	else if (pAccept->m_btFigure == enumTONG_FIGURE_MANAGER)
	{
		for (i = 0; i < defTONG_MAX_MANAGER; i++)
		{
			if (m_dwManagerID[i] == dwNameID && strcmp(szName, m_szManagerName[i]) == 0)
				break;
		}
		if (i >= defTONG_MAX_MANAGER)
			return FALSE;
		if (!AddMember(m_szMasterName))
			return FALSE;
		strcpy(this->m_szMasterName, szName);
		this->m_dwMasterID = dwNameID;
		m_szManagerName[i][0] = 0;
		m_dwManagerID[i] = 0;
		this->m_nManagerNum--;
	}
	else if (pAccept->m_btFigure == enumTONG_FIGURE_MEMBER)
	{
		if (!m_psMember || m_nMemberPointSize <= 0)
			return FALSE;
		for (i = 0; i < this->m_nMemberPointSize; i++)
		{
			if (m_psMember[i].m_dwNameID == dwNameID &&
				strcmp(szName, m_psMember[i].m_szName) == 0)
				break;
		}
		if (i >= m_nMemberPointSize)
			return FALSE;
		if (!AddMember(m_szMasterName))
			return FALSE;
		strcpy(m_szMasterName, szName);
		m_dwMasterID = dwNameID;
		m_psMember[i].m_szName[0] = 0;
		m_psMember[i].m_dwNameID = 0;
		m_nMemberNum--;
	}
	else
	{
		return FALSE;
	}

	STONG_CHANGE_AS_SYNC	sChange;

	sChange.ProtocolFamily	= pf_tong;
	sChange.ProtocolID		= enumS2C_TONG_CHANGE_AS;
	sChange.m_dwTongNameID	= this->m_dwNameID;

	// 给老帮主发消息
	DWORD nameid = 0;
	unsigned long param = 0;
	CNetConnectDup conndup1;
	if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(szOldMaster), &conndup1, NULL, &nameid, &param))
	{
		CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup1.GetIP());
		if (tongconndup.IsValid())
		{
			sChange.m_dwParam		= param;
			sChange.m_btFigure		= enumTONG_FIGURE_MEMBER;
			sChange.m_btPos			= 0;
			strcpy(sChange.m_szTitle, m_szNormalTitle);
			strcpy(sChange.m_szName, m_szMasterName);

			tongconndup.SendPackage((const void *)&sChange, sizeof(sChange));
		}
	}

	// 给新帮主发消息
	CNetConnectDup conndup2;
	if (g_HostServer.FindPlayerByRole(NULL, std::_tstring(szName), &conndup2, NULL, &nameid, &param))
	{
		CNetConnectDup tongconndup = g_TongServer.FindTongConnectByIP(conndup2.GetIP());
		if (tongconndup.IsValid())
		{
			sChange.m_dwParam		= param;
			sChange.m_btFigure		= enumTONG_FIGURE_MASTER;
			sChange.m_btPos			= 0;
			strcpy(sChange.m_szTitle, this->m_szMasterTitle);
			strcpy(sChange.m_szName, this->m_szMasterName);

			tongconndup.SendPackage((const void *)&sChange, sizeof(sChange));
		}
	}

	STONG_CHANGE_MASTER_SYNC	sMaster;
	sMaster.ProtocolFamily	= pf_tong;
	sMaster.ProtocolID		= enumS2C_TONG_CHANGE_AS;
	sMaster.m_dwTongNameID	= m_dwNameID;
	strcpy(sMaster.m_szName, m_szMasterName);

	g_TongServer.BroadPackage((const void*)&sMaster, sizeof(sMaster));

	// 存盘
	// 新帮主
	TMemberStruct	sMember1;
	sMember1.MemberClass = enumTONG_FIGURE_MASTER;
	sMember1.nTitleIndex = 0;
	strcpy(sMember1.szTong, this->m_szName);
	strcpy(sMember1.szName, szName);
	g_cTongDB.ChangeMember(sMember1);
	// 老帮主
	TMemberStruct	sMember2;
	sMember2.MemberClass = enumTONG_FIGURE_MEMBER;
	sMember2.nTitleIndex = 0;
	strcpy(sMember2.szTong, this->m_szName);
	strcpy(sMember2.szName, szOldMaster);
	g_cTongDB.ChangeMember(sMember2);

	// 聊天频道发消息
	char	szMsg[96];
	sprintf(szMsg, "\\O%u", m_dwNameID);

	DWORD channid = g_ChannelMgr.GetChannelID(szMsg, 0);
	if (channid != -1)
	{
		sprintf(szMsg, "%s把帮主之位传给了%s！", szOldMaster, szName);
		g_ChannelMgr.SayOnChannel(channid, TRUE, std::string(), std::string("公告"), std::string(szMsg));
	}

	return TRUE;
}

BOOL	CTongControl::GetLoginData(STONG_GET_LOGIN_DATA_COMMAND *pLogin, STONG_LOGIN_DATA_SYNC *pSync)
{
	if (!pLogin || !pSync)
		return FALSE;

	int		i;
	DWORD	dwNameID = g_String2Id(pLogin->m_szName);
	if (dwNameID == 0)
		return FALSE;

	pSync->m_btCamp		= this->m_nCamp;
	strcpy(pSync->m_szTongName, this->m_szName);
	strcpy(pSync->m_szMaster, this->m_szMasterName);
	strcpy(pSync->m_szName, pLogin->m_szName);

	if (this->m_dwMasterID == dwNameID)
	{
		pSync->m_btFigure	= enumTONG_FIGURE_MASTER;
		pSync->m_btPos		= 0;
		pSync->m_btFlag		= 1;
		strcpy(pSync->m_szTitle, this->m_szMasterTitle);
		return TRUE;
	}

	for (i = 0; i < defTONG_MAX_DIRECTOR; i++)
	{
		if (this->m_dwDirectorID[i] == dwNameID)
		{
			pSync->m_btFigure	= enumTONG_FIGURE_DIRECTOR;
			pSync->m_btPos		= i;
			pSync->m_btFlag		= 1;
			strcpy(pSync->m_szTitle, this->m_szDirectorTitle[i]);
			return TRUE;
		}
	}

	for (i = 0; i < defTONG_MAX_MANAGER; i++)
	{
		if (this->m_dwManagerID[i] == dwNameID)
		{
			pSync->m_btFigure	= enumTONG_FIGURE_MANAGER;
			pSync->m_btPos		= i;
			pSync->m_btFlag		= 1;
			strcpy(pSync->m_szTitle, this->m_szManagerTitle[i]);
			return TRUE;
		}
	}

	if (this->m_psMember)
	{
		for (i = 0; i < this->m_nMemberPointSize; i++)
		{
			if (m_psMember[i].m_dwNameID == dwNameID)
			{
				pSync->m_btFigure	= enumTONG_FIGURE_MEMBER;
				pSync->m_btPos		= 0;
				pSync->m_btFlag		= 1;
				strcpy(pSync->m_szTitle, this->m_szNormalTitle);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL	CTongControl::SearchOne(char *lpszName, STONG_ONE_LEADER_INFO *pInfo)
{
	if (!lpszName || !lpszName[0] || !pInfo)
		return FALSE;

	int		i;
	DWORD	dwNameID = g_String2Id(lpszName);
	if (dwNameID == 0)
		return FALSE;

	if (m_dwMasterID == dwNameID)
	{
		pInfo->m_btFigure	= enumTONG_FIGURE_MASTER;
		pInfo->m_btPos		= 0;
		strcpy(pInfo->m_szTitle, m_szMasterTitle);
		strcpy(pInfo->m_szName, m_szMasterName);
		return TRUE;
	}

	for (i = 0; i < defTONG_MAX_DIRECTOR; i++)
	{
		if (m_dwDirectorID[i] == dwNameID)
		{
			pInfo->m_btFigure	= enumTONG_FIGURE_DIRECTOR;
			pInfo->m_btPos		= i;
			strcpy(pInfo->m_szTitle, m_szDirectorTitle[i]);
			strcpy(pInfo->m_szName, m_szDirectorName[i]);
			return TRUE;
		}
	}

	for (i = 0; i < defTONG_MAX_MANAGER; i++)
	{
		if (m_dwManagerID[i] == dwNameID)
		{
			pInfo->m_btFigure	= enumTONG_FIGURE_MANAGER;
			pInfo->m_btPos		= i;
			strcpy(pInfo->m_szTitle, m_szManagerTitle[i]);
			strcpy(pInfo->m_szName, m_szManagerName[i]);
			return TRUE;
		}
	}

	if (m_psMember)
	{
		for (i = 0; i < m_nMemberPointSize; i++)
		{
			if (m_psMember[i].m_dwNameID == dwNameID)
			{
				pInfo->m_btFigure	= enumTONG_FIGURE_MEMBER;
				pInfo->m_btPos		= 0;
				strcpy(pInfo->m_szTitle, m_szNormalTitle);
				strcpy(pInfo->m_szName, m_psMember[i].m_szName);
				return TRUE;
			}
		}
	}

	return FALSE;
}









