// HostConnect.h: interface for the CHostConnect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HOSTCONNECT_H__42803752_B2ED_4398_9FF8_BE05DC352F41__INCLUDED_)
#define AFX_HOSTCONNECT_H__42803752_B2ED_4398_9FF8_BE05DC352F41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetConnect.h"

class CHostConnect : public CNetConnect  
{
public:
	CHostConnect(class CHostServer* pHostServer, unsigned long id);
	virtual ~CHostConnect();

protected:
	virtual void RecvPackage(const void* pData, size_t size);

private:
	void Proc0_Normal(const void* pData, size_t size);
	void Proc1_Normal_EnterGame(const void* pData, size_t size);
	void Proc1_Normal_LeaveGame(const void* pData, size_t size);

	void Proc0_Relay(const void* pData, size_t size);
	void Proc1_Relay_Data(const void* pData, size_t size);
	void Proc1_Relay_AskWay(const void* pData, size_t size);
	void Proc1_Relay_LoseWay(const void* pData, size_t size);
	void Proc2_Relay_AskWay_AccountRoleID(const void* pData, size_t size, BOOL acc);
	void Proc2_Relay_AskWay_MapID(const void* pData, size_t size);
	void Proc2_Relay_AskWay_GM(const void* pData, size_t size);

	void Proc0_PlayerCommunity(const void* pData, size_t size);
	void Proc1_PlayerCommunity_QueryChannelID(const void* pData, size_t size);
	void Proc1_PlayerCommunity_FreeChannelID(const void* pData, size_t size);
	void Proc1_PlayerCommunity_Subscribe(const void* pData, size_t size);

protected:
	virtual void OnClientConnectCreate();
	virtual void OnClientConnectClose();

	virtual void PrepareRecvs();
	virtual void UnprepareRecvs();

//track player
private:
	struct ACCINFO
	{
		std::_tstring role;
		DWORD nameID;
		unsigned long param;
	};
	typedef std::map<std::_tstring, ACCINFO, _tstring_iless>	ACCMAP;
	ACCMAP m_mapAcc;

	struct ROLEINFO
	{
		std::_tstring acc;
		DWORD nameID;
		unsigned long param;
	};
	typedef std::map<std::_tstring, ROLEINFO, _tstring_less>	ROLEMAP;
	ROLEMAP m_mapRole;

	struct PARAMINFO
	{
		std::_tstring acc;
		std::_tstring role;
		DWORD nameID;
	};
	typedef std::map<unsigned long, PARAMINFO>	PARAMMAP;
	PARAMMAP m_mapParam;

	CLockMRSW m_lockPlayer;

public:
	BOOL FindPlayerByAcc(const std::_tstring& acc, std::_tstring* pRole, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealAcc = NULL);
	BOOL FindPlayerByRole(const std::_tstring& role, std::_tstring* pAcc, DWORD* pNameID, unsigned long* pParam, std::_tstring* pRealRole = NULL);
	BOOL FindPlayerByParam(unsigned long param, std::_tstring* pAcc, std::_tstring* pRole, DWORD* pNameID);

	//BOOL BroadOnPlayer(BOOL byAcc, DWORD fromIP, unsigned long fromRelayID, const void* pData, size_t size);

public:
	size_t GetPlayerCount();
};

#endif // !defined(AFX_HOSTCONNECT_H__42803752_B2ED_4398_9FF8_BE05DC352F41__INCLUDED_)
