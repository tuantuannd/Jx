#ifndef	KWorldH
#define	KWorldH

#ifdef _SERVER
#define	MAX_SUBWORLD	80
#else
#define	MAX_SUBWORLD	1
#endif
#define	VOID_REGION		-2
//-------------------------------------------------------------
#include "KEngine.h"
#include "KRegion.h"
#include "KWeatherMgr.h"

#ifdef _SERVER
#include "KMission.h"
#include "KMissionArray.h"
#define MAX_SUBWORLD_MISSIONCOUNT 10
#define MAX_GLOBAL_MISSIONCOUNT 50
typedef KMissionArray <KMission , MAX_TIMER_PERMISSION> KSubWorldMissionArray;
typedef KMissionArray <KMission , MAX_GLOBAL_MISSIONCOUNT> KGlobalMissionArray;
extern KGlobalMissionArray g_GlobalMissionArray;
#endif

//-------------------------------------------------------------


#ifndef TOOLVERSION
class KSubWorld
#else

class CORE_API KSubWorld
#endif
{
public:	
	int			m_nIndex;
	int			m_SubWorldID;
#ifdef _SERVER
	KSubWorldMissionArray m_MissionArray;
#endif
	KRegion*	m_Region;
#ifndef _SERVER
	int			m_ClientRegionIdx[MAX_REGION];
	char		m_szMapPath[FILE_NAME_LENGTH];
	//KLittleMap	m_cLittleMap;
#endif
	int			m_nWorldRegionWidth;			//	SubWorld里宽几个Region
	int			m_nWorldRegionHeight;			//	SubWorld里高几个Region
	int			m_nTotalRegion;					//	SubWorld里Region个数
	int			m_nRegionWidth;					//	Region的格子宽度
	int			m_nRegionHeight;				//	Region的格子高度
	int			m_nCellWidth;					//	Cell的像素宽度
	int			m_nCellHeight;					//	Cell的像素高度
	int			m_nRegionBeginX;				
	int			m_nRegionBeginY;
	int			m_nWeather;						//	天气变化
	DWORD		m_dwCurrentTime;				//	当前帧
	KWorldMsg	m_WorldMessage;					//	消息
	KList		m_NoneRegionNpcList;			//	不在地图上的NPC

#ifdef _SERVER
	KWeatherMgr *m_pWeatherMgr;
#endif
private:
public:
	KSubWorld();
	~KSubWorld();
	void		Activate();
	void		GetFreeObjPos(POINT& pos);
	BOOL		CanPutObj(POINT pos);
	void		ObjChangeRegion(int nSrcRegionIdx, int nDesRegionIdx, int nObjIdx);
	void		MissleChangeRegion(int nSrcRegionIdx, int nDesRegionIdx, int nObjIdx);
	void		AddPlayer(int nRegion, int nIdx);
	void		RemovePlayer(int nRegion, int nIdx);
	void		Close();
	int			GetDistance(int nRx1, int nRy1, int nRx2, int nRy2);						// 像素级坐标
	void		Map2Mps(int nR, int nX, int nY, int nDx, int nDy, int *nRx, int *nRy);		// 格子坐标转像素坐标
	static void Map2Mps(int nRx, int nRy, int nX, int nY, int nDx, int nDy, int *pnX, int *pnY);		// 格子坐标转像素坐标
	void		Mps2Map(int Rx, int Ry, int * nR, int * nX, int * nY, int *nDx, int * nDy);	// 像素坐标转格子坐标
	void		GetMps(int *nX, int *nY, int nSpeed, int nDir, int nMaxDir = 64);			// 取得某方向某速度下一点的坐标
	BYTE		TestBarrier(int nMpsX, int nMpsY);
	BYTE		TestBarrier(int nRegion, int nMapX, int nMapY, int nDx, int nDy, int nChangeX, int nChangeY);	// 检测下一点是否为障碍
	BYTE		TestBarrierMin(int nRegion, int nMapX, int nMapY, int nDx, int nDy, int nChangeX, int nChangeY);	// 检测下一点是否为障碍
	BYTE		GetBarrier(int nMpsX, int nMpsY);											// 取得某点的障碍信息
	DWORD		GetTrap(int nMpsX, int nMpsY);
	void		MessageLoop();
	int			FindRegion(int RegionID);													// 找到某ID的Region的索引
	int			FindFreeRegion(int nX = 0, int nY = 0);
#ifdef _SERVER
	int			RevivalAllNpc();//将地图上所有的Npc包括已死亡的Npc全部恢复成原始状态
	void		BroadCast(const char* pBuffer, size_t uSize);
	BOOL		LoadMap(int nIdx);
	void		LoadObject(char* szPath, char* szFile);
	void		NpcChangeRegion(int nSrcRegionIdx, int nDesRegionIdx, int nNpcIdx);
	void		PlayerChangeRegion(int nSrcRegionIdx, int nDesRegionIdx, int nObjIdx);
	BOOL		SendSyncData(int nIdx, int nClient);
	int			GetRegionIndex(int nRegionID);
	int			FindNpcFromName(const char * szName);
#endif
#ifndef _SERVER
	BOOL		LoadMap(int nIdx, int nRegion);
	void		NpcChangeRegion(int nSrcRegionIdx, int nDesRegionIdx, int nNpcIdx);
	void		Paint();
	void		Mps2Screen(int *Rx, int *Ry);
	void		Screen2Mps(int *Rx, int *Ry);
#endif
private:
	void		LoadTrap();
	void		ProcessMsg(KWorldMsgNode *pMsg);
#ifndef _SERVER
	void		LoadCell();
#endif
};

#ifndef TOOLVERSION
extern KSubWorld	SubWorld[MAX_SUBWORLD];
#else 
extern CORE_API KSubWorld	SubWorld[MAX_SUBWORLD];
#endif
#endif
