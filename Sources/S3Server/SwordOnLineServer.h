



#pragma once
#include "Winsock2.h"
#include "KLinkArray.h"
#include "CoreServerDataDef.h"
#include "../MultiServer/Heaven/interface/iServer.h"

struct ClientConnectInfo
{
	char	AddrInfo[32];
	char	Character[32];
};

enum NetStatus
{
	NET_UNCONNECT = 0,
	NET_CONNECTED,
	NET_DISCONNECT,
};
class KSwordOnLineSever
{
public:
	KSwordOnLineSever();
	~KSwordOnLineSever();
	bool		Init();
	bool		Launch();
	bool		Breathe();	//返回值表示状态是否更新
	bool		ShutDown();

	int			OperationRequest(unsigned int uOper, unsigned int uParam, int nParam);
	int			GetClientConnectInfo(ClientConnectInfo* pInfo, int& nPos);
	void		GetStatus(char* pszStatusString, char* pszInformation);
	void		SetUpdateInterval(int nInterval);
	int			IsRuning() { return m_bRuning; }
	void		SetNetStatus(const unsigned long lnID, NetStatus nStatus);
	int			GetNetStatus(const unsigned long lnID);
	void		DeleteTempValue(void * pValue){if (pValue) delete pValue; pValue = NULL;};
	const int	GetMaxPlayer() { return m_nMaxPlayer; };
private:
	void		MessageLoop();
	void		MessageProcess(const unsigned long lnID, const char* pChar, size_t nSize);
	void		MainLoop();
	BOOL		ProcessLoginProtocol(int nClient, const char* pChar, int nSize);
	BOOL		ProcessSyncReplyProtocol(int nClient, const char* pChar, int nSize);
	void		SendPlayerListToClient(int nClient);
	int			RecvClientPlayerSelect(int nClient, const char* pChar, int nSize, int *pnSel);
	int			RecvClientPlayerNew(int nClient, const char* pChar, int nSize, int *pnRole, int *pnSeries, char *lpszName);
private:
	enum	{ DEFUALT_UPDATE_INTERFAL = 100 };
	IServer*			m_pServer;
	HMODULE				m_hModule;
	int					m_bRuning;
	int					m_nUpdateInterval;
	int					m_nLoopElapse;
	int					m_nMaxPlayer;
	char				m_szStatusString[128];
	char				m_szInformation[128];
	int*				m_pnPlayerIndex;
	LoginData*			m_pPlayerData;
	GameStatus*			m_pPlayerGameStatus;
	ConnectStatus*		m_pNetConnectStatus;
	DWORD*				m_pnPlayerAccountKey;
	DWORD				m_GameLoop;
	KTimer				m_Timer;
	struct iCoreServerShell*	m_pCoreServerShell;
};
