// KTongSet.h: interface for the CTongSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KTONGSET_H__B42782F1_FA08_4D1C_A209_1ED1F5E0BAA3__INCLUDED_)
#define AFX_KTONGSET_H__B42782F1_FA08_4D1C_A209_1ED1F5E0BAA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include	"KTongControl.h"

#define		defTONG_SET_INIT_POINT_NUM		16

class CTongSet
{
public:
	CTongSet();
	virtual ~CTongSet();

private:
	CTongControl**	m_pcTong;
	int				m_nTongPointSize;		// 指针 m_pcTong 当前分配内存的大小(多少个)

public:
	void			Init();
	void			DeleteAll();
	BOOL			InitFromDB();

	BOOL			SearchOne(DWORD dwTongNameID, char *lpszName, STONG_ONE_LEADER_INFO *pInfo);

	// 创建一个帮会
	int				Create(int nCamp, char *lpszPlayerName, char *lpszTongName);
	// 添加一个帮会成员，if return == 0 成功 else return error id
	int				AddMember(char *lpszPlayerName, char *lpszTongName);
	// 获得帮会阵营
	int				GetTongCamp(int nTongIdx);

	BOOL			GetMasterName(int nTongIdx, char *lpszName);

	BOOL			GetMemberTitle(int nTongIdx, char *lpszTitle);

	BOOL			GetTongHeadInfo(DWORD dwTongNameID, STONG_HEAD_INFO_SYNC *pInfo);

	BOOL			GetTongManagerInfo(STONG_GET_MANAGER_INFO_COMMAND *pApply, STONG_MANAGER_INFO_SYNC *pInfo);

	BOOL			GetTongMemberInfo(STONG_GET_MEMBER_INFO_COMMAND *pApply, STONG_MEMBER_INFO_SYNC *pInfo);

	BOOL			Instate(STONG_INSTATE_COMMAND *pInstate, STONG_INSTATE_SYNC *pSync);

	BOOL			Kick(STONG_KICK_COMMAND *pKick, STONG_KICK_SYNC *pSync);

	BOOL			Leave(STONG_LEAVE_COMMAND *pLeave, STONG_LEAVE_SYNC *pSync);

	BOOL			AcceptMaster(STONG_ACCEPT_MASTER_COMMAND *pAccept);

	BOOL			GetLoginData(STONG_GET_LOGIN_DATA_COMMAND *pLogin, STONG_LOGIN_DATA_SYNC *pSync);
public:
};

#endif // !defined(AFX_KTONGSET_H__B42782F1_FA08_4D1C_A209_1ED1F5E0BAA3__INCLUDED_)
