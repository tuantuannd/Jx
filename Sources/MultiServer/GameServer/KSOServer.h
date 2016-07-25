#ifndef KSOServerH
#define	KSOServerH

#include <map>
#include "KEngine.h"
#include "IClient.h"
#ifndef _STANDALONE
#include "RainbowInterface.h"
#include "HeavenInterface.h"
#include "..\..\Core\src\CoreServerShell.h"
#include "UsesWinsock.h"
#include "Buffer.h"
using OnlineGameLib::Win32::CPackager;

#else
#include "CoreServerShell.h"
#endif

#include "IServer.h"
#include "KTimer.h"
#include "KTransferUnit.h"


enum NetStatus
{
	enumNetUnconnect = 0,
	enumNetConnected,
};

struct GameStatus 
{
	GameStatus() 
	{
		nGameStatus = 0; 
		nNetStatus = 0;
		nExchangeStatus = 0;
		nPlayerIndex = 0;
		nSendPingTime = 0;
		nReplyPingTime = 0;
	};
	int					nPlayerIndex;
	int					nGameStatus;
	int					nNetStatus;
	int					nExchangeStatus;
	int					nSendPingTime;
	int					nReplyPingTime;
} ;

class KSwordOnLineSever
{
private:
#ifdef _STANDALONE
	ZBuffer *net_buffer;
#endif
	int					m_nMaxPlayerCount;
	int					m_nPrecision;
	static const int	m_snMaxBuffer;
	static const int	m_snBufferSize;
	int					m_nMaxPlayer;
	int					m_nGameLoop;
	int					m_nServerPort;
	int					m_nGatewayPort;
	int					m_nDatabasePort;
	int					m_nTransferPort;
	int					m_nChatPort;
	int					m_nTongPort;
	DWORD				m_dwIntranetIp;
	DWORD				m_dwInternetIp;
	char				m_szGatewayIP[16];
	char				m_szDatabaseIP[16];
	char				m_szTransferIP[16];
	char				m_szChatIP[16];
	char				m_szTongIP[16];
	BOOL				m_bIsRunning;
//	BOOL				m_bSaveFlag;
//	int					m_nSaveCount;
	IServer*			m_pServer;
	IClient*			m_pGatewayClient;
	IClient*			m_pDatabaseClient;
	IClient*			m_pTransferClient;
	IClient*			m_pChatClient;
	IClient*			m_pTongClient;
	GameStatus*			m_pGameStatus;
	struct iCoreServerShell*	m_pCoreServerShell;
	KTimer				m_Timer;
	typedef std::map<DWORD, KTransferUnit*>	IP2CONNECTUNIT;
	IP2CONNECTUNIT		m_mapIp2TransferUnit;
public:
	KSwordOnLineSever();
	~KSwordOnLineSever();
	BOOL				Init();
	BOOL				Breathe();
	int					GetNetStatus(const unsigned long lnID);
	void				SetNetStatus(const unsigned long lnID, NetStatus nStatus);
	void				SetRunningStatus(BOOL bStatus);
	void				Release();
private:
	void				MessageLoop();
	void				GatewayMessageProcess(const char* pChar, size_t nSize);
	void				DatabaseMessageProcess(const char* pChar, size_t nSize);
	void				DatabaseLargePackProcess(const char* pChar, size_t nSize);
	void				TransferMessageProcess(const char* pChar, size_t nSize);
	void				PlayerMessageProcess(const unsigned long lnID, const char* pChar, size_t nSize);
	void				GatewayLargePackProcess(const void *pData, size_t dataLength);
	void				GatewaySmallPackProcess(const void *pData, size_t dataLength);
	void				GatewayBoardCastProcess(const char* pData, size_t dataLength);
	void				TransferLargePackProcess(const void *pData, size_t dataLength, KTransferUnit* pUnit);
	void				TransferSmallPackProcess(const void *pData, size_t dataLength, KTransferUnit* pUnit);
	void				TransferAskWayMessageProcess(const char *pData, size_t dataLength);
	void				TransferLoseWayMessageProcess(const char *pData, size_t dataLength);
	void				ChatMessageProcess(const char *pChar, size_t nSize);
	void				ChatGroupMan(const void *pData, size_t dataLength);
	void				ChatSpecMan(const void *pData, size_t dataLength);

	BOOL				ConformAskWay(const void* pData, int nSize, DWORD *pdwID);
	void				MainLoop();
	int					ProcessLoginProtocol(const unsigned long lnID, const char* pData, size_t dataLength);
	BOOL				ProcessSyncReplyProtocol(const unsigned long lnID, const char* pData, size_t dataLength);
	void				PingClient(const unsigned long lnID);
	void				ProcessPingReply(const unsigned long lnID, const char* pData, size_t dataLength);

	BOOL				SendGameDataToClient(const unsigned long lnID, const int nPlayerIndex);
	void				SavePlayerData();
	BOOL				SavePlayerData(int nIndex, bool bUnLock);
	void				ExitAllPlayer();
	void				PlayerLogoutGateway();
	void				PlayerExchangeServer();
	BOOL				GetLocalIpAddress(DWORD *pIntranet, DWORD *pInternet);

	// 帮会协议处理( relay 发过来的协议)
	void				TongMessageProcess(const char *pChar, size_t nSize);
	// 帮会协议处理(客户端通过帮会扩展协议发过来的协议)
	void				ProcessPlayerTongMsg(const unsigned long nPlayerIdx, const char* pData, size_t dataLength);

	BOOL				CheckPlayerID(unsigned long netidx, DWORD nameid);
};

#endif
