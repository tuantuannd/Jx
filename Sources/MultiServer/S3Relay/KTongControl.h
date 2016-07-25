// KTongControl.h: interface for the CTongControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KTONGCONTROL_H__62D04F9A_67CD_419B_B475_BF0F8727A91E__INCLUDED_)
#define AFX_KTONGCONTROL_H__62D04F9A_67CD_419B_B475_BF0F8727A91E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


struct STONG_MEMBER
{
	DWORD	m_dwNameID;
	char	m_szName[defTONG_STR_LENGTH];
};

typedef struct
{
	char		szName[defTONG_STR_LENGTH];			// 帮会名
	int			MemberCount;						//一般帮众数
}TTongList;	//用作帮会列表

class CTongControl
{
	friend class CTongSet;
	friend class CTongDB;
public:
	// 用于创建帮会
	CTongControl(int nCamp, char *lpszPlayerName, char *lpszTongName);
	// 用于数据库载入帮会数据
	CTongControl(TTongList sList);
	virtual ~CTongControl();

private:

	int			m_nCamp;								// 帮会阵营
	DWORD		m_dwMoney;								// 帮会资金
	int			m_nCredit;								// 帮会声望
	int			m_nLevel;								// 帮会等级
	int			m_nDirectorNum;							// 长老人数
	int			m_nManagerNum;							// 队长人数
	int			m_nMemberNum;							// 帮众人数

	DWORD		m_dwNameID;								// 帮会名ID
	char		m_szName[defTONG_STR_LENGTH];			// 帮会名
	char		m_szPassword[defTONG_STR_LENGTH];		// 帮会密码

	char		m_szMasterTitle[defTONG_STR_LENGTH];	// 帮主称号
	char		m_szDirectorTitle[defTONG_MAX_DIRECTOR][defTONG_STR_LENGTH];// 各长老称号
	char		m_szManagerTitle[defTONG_MAX_MANAGER][defTONG_STR_LENGTH];	// 各队长称号
	char		m_szNormalTitle[defTONG_STR_LENGTH];	// 帮会帮众称号

	DWORD		m_dwMasterID;							// 帮主名ID
	char		m_szMasterName[defTONG_STR_LENGTH];		// 帮主名

	DWORD		m_dwDirectorID[defTONG_MAX_DIRECTOR];	// 各长老名ID
	char		m_szDirectorName[defTONG_MAX_DIRECTOR][defTONG_STR_LENGTH];	// 各长老名

	DWORD		m_dwManagerID[defTONG_MAX_MANAGER];		// 各队长名ID
	char		m_szManagerName[defTONG_MAX_MANAGER][defTONG_STR_LENGTH];	// 各队长名

	STONG_MEMBER	*m_psMember;
	int				m_nMemberPointSize;

public:
	BOOL		SearchOne(char *lpszName, STONG_ONE_LEADER_INFO *pInfo);

	BOOL		AddMember(char *lpszPlayerName);		// 添加一个成员

	BOOL		ChangePassword(char *lpOld, char *lpNew);	// 改变帮会密码

	BOOL		GetTongHeadInfo(STONG_HEAD_INFO_SYNC *pInfo);	// 帮会信息

	BOOL		GetTongManagerInfo(STONG_GET_MANAGER_INFO_COMMAND *pApply, STONG_MANAGER_INFO_SYNC *pInfo);

	BOOL		GetTongMemberInfo(STONG_GET_MEMBER_INFO_COMMAND *pApply, STONG_MEMBER_INFO_SYNC *pInfo);

	BOOL		Instate(STONG_INSTATE_COMMAND *pInstate, STONG_INSTATE_SYNC *pSync);

	BOOL		Kick(STONG_KICK_COMMAND *pKick, STONG_KICK_SYNC *pSync);

	BOOL		Leave(STONG_LEAVE_COMMAND *pLeave, STONG_LEAVE_SYNC *pSync);

	BOOL		AcceptMaster(STONG_ACCEPT_MASTER_COMMAND *pAccept);

	BOOL		GetLoginData(STONG_GET_LOGIN_DATA_COMMAND *pLogin, STONG_LOGIN_DATA_SYNC *pSync);
};

#endif // !defined(AFX_KTONGCONTROL_H__62D04F9A_67CD_419B_B475_BF0F8727A91E__INCLUDED_)
