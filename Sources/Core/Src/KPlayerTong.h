//---------------------------------------------------------------------------
// Sword3 Engine (c) 2003 by Kingsoft
//
// File:	KPlayerTong.h
// Date:	2003.08.12
// Code:	边城浪子
// Desc:	KPlayerTong Class
//---------------------------------------------------------------------------

#ifndef KPLAYERTONG_H
#define KPLAYERTONG_H

class KPlayerTong
{
	friend class KPlayer;
private:
	int			m_nPlayerIndex;		// 指向对应的player数组的位置
	int			m_nFlag;			// 是否加入帮会
	int			m_nFigure;			// 帮会中的身份
	int			m_nCamp;			// 帮会阵营
	DWORD		m_dwTongNameID;		// 帮会名字符串转换成的 dword
	char		m_szName[32];		// 帮会名称
	char		m_szTitle[32];		// 当前称号
	int			m_nApplyTo;			// 如果已经向某个帮会申请加入，帮主的 playerindex

public:
	char		m_szMasterName[32];	// 帮主名字

private:
	BOOL		CheckAcceptAddApplyCondition();	// 判断是否可以转发别人的加入帮会申请

public:
	// 初始化
	void		Init(int nPlayerIdx);
	void		Clear();
	void		GetTongName(char *lpszGetName);
	DWORD		GetTongNameID();
	void		SetTongNameID(DWORD dwID) { m_dwTongNameID = dwID; };
	int			CheckIn() {return m_nFlag;};
	int			GetCamp() {return m_nCamp;};
	int			GetFigure() {return m_nFigure;};

	// 是否有权力查询队长信息
	BOOL		CanGetManagerInfo(DWORD dwTongNameID);
	// 是否有权力查询帮众信息
	BOOL		CanGetMemberInfo(DWORD dwTongNameID);

#ifndef _SERVER
	// 申请创建帮会
	BOOL		ApplyCreateTong(int nCamp, char *lpszTongName);
	// 得到服务器通知创建帮会
	void		Create(TONG_CREATE_SYNC *psCreate);
	// 申请加入帮会
	BOOL		ApplyAddTong(DWORD dwNpcID);
	// 是否接受成员 bFlag == TRUE 接受 == FALSE 不接受
	void		AcceptMember(int nPlayerIdx, DWORD dwNameID, BOOL bFlag);
	// 加入帮会，成为普通帮众
	BOOL		AddTong(int nCamp, char *lpszTongName, char *lpszTitle, char *lpszMaster);
	// 申请任命
	BOOL		ApplyInstate(int nCurFigure, int nCurPos, int nNewFigure, int nNewPos, char *lpszName);
	// 申请踢人
	BOOL		ApplyKick(int nCurFigure, int nCurPos, char *lpszName);
	// 申请传位
	BOOL		ApplyChangeMaster(int nCurFigure, int nPos, char *lpszName);
	// 申请离开帮会
	BOOL		ApplyLeave();

	/* 申请获得帮会信息
	if nInfoID == enumTONG_APPLY_INFO_ID_SELF 不用参数
	if nInfoID == enumTONG_APPLY_INFO_ID_MASTER nParam1 帮会名id
	if nInfoID == enumTONG_APPLY_INFO_ID_DIRECTOR nParam1 帮会名id
	if nInfoID == enumTONG_APPLY_INFO_ID_MANAGER nParam1 帮会名id nParam2 从开始有效第几个 nParam3 连续几个
	if nInfoID == enumTONG_APPLY_INFO_ID_MEMBER nParam1 帮会名id nParam2 从开始有效第几个 nParam3 连续几个
	if nInfoID == enumTONG_APPLY_INFO_ID_ONE lpszName 人名
	if nInfoID == enumTONG_APPLY_INFO_ID_TONG_HEAD nParam1 NpcIndex
	*/
	BOOL		ApplyInfo(int nInfoID, int nParam1, int nParam2, int nParam3, char *lpszName = NULL);
	// 设定自身帮会信息
	void		SetSelfInfo(TONG_SELF_INFO_SYNC *pInfo);
	
	void        OpenCreateInterface();
#endif

#ifdef _SERVER
	// 判断创建帮会条件是否成立 if 成功 return == 0 else return error id
	int			CheckCreateCondition(int nCamp, char *lpszTongName);
	// 得到relay通知，帮会创建成功，处理相应数据
	BOOL		Create(int nCamp, char *lpszTongName);
	// 头上是否需要顶找人标志
	BOOL		GetOpenFlag();
	// 转发加入帮会申请给对方客户端
	BOOL		TransferAddApply(DWORD dwNpcID);
	// 发消息通知拒绝某人申请
	void		SendRefuseMessage(int nPlayerIdx, DWORD dwNameID);
	// 判断别人加入自己帮会条件是否成立
	BOOL		CheckAddCondition(int nPlayerIdx);
	// 加入帮会，成为普通帮众
	BOOL		AddTong(int nCamp, char *lpszTongName, char *lpszMasterName, char *lpszTitleName);
	// 给客户端发送自己在帮会中的信息
	void		SendSelfInfo();
	// 检测是否有任命权利
	BOOL		CheckInstatePower(TONG_APPLY_INSTATE_COMMAND *pApply);
	// 被任命
	void		BeInstated(STONG_SERVER_TO_CORE_BE_INSTATED *pSync);
	// 检测是否有踢人权利
	BOOL		CheckKickPower(TONG_APPLY_KICK_COMMAND *pKick);
	// 被踢出帮会
	void		BeKicked(STONG_SERVER_TO_CORE_BE_KICKED *pSync);
	// 检测是否有离开权利
	BOOL		CheckLeavePower(TONG_APPLY_LEAVE_COMMAND *pLeave);
	// 离开帮会
	void		Leave(STONG_SERVER_TO_CORE_LEAVE *pLeave);
	// 检测是否有权利换帮主
	BOOL		CheckChangeMasterPower(TONG_APPLY_CHANGE_MASTER_COMMAND *pChange);
	// 检测是否有能力接受传位
	BOOL		CheckGetMasterPower(STONG_SERVER_TO_CORE_CHECK_GET_MASTER_POWER *pCheck);
	// 传位导致身份改变
	void		ChangeAs(STONG_SERVER_TO_CORE_CHANGE_AS *pAs);
	// 换帮主
	void		ChangeMaster(char *lpszMaster);
	// 登陆时候获得帮会信息
	void		Login(STONG_SERVER_TO_CORE_LOGIN *pLogin);

	void		DBSetTongNameID(DWORD dwID) { m_dwTongNameID = dwID; if (dwID) m_nFlag = 1;};

#endif
};
#endif
