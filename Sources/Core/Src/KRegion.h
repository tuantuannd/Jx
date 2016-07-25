#ifndef	KRegionH
#define	KRegionH

//-----------------------------------------------------------------------
#include "KEngine.h"
#include "KWorldMsg.h"
//-----------------------------------------------------------------------
#ifdef _SERVER
#define	MAX_BROADCAST_COUNT		100
#else
#define	MAX_REGION		9
#endif

#define	REGION_GRID_WIDTH	16
#define	REGION_GRID_HEIGHT	32

enum MOVE_OBJ_KIND
{
	obj_npc,
	obj_object,
	obj_missle,
};
#ifndef TOOLVERSION
class KRegion
#else
class CORE_API KRegion
#endif
{
	friend class	KSubWorld;
public:
	int			m_nIndex;							// 地图索引
	int			m_RegionID;
	KList		m_NpcList;							// 人物列表
	KList		m_ObjList;							// 物件列表
	KList		m_MissleList;						// 子弹列表
	KList		m_PlayerList;						// 玩家列表
	int			m_nConnectRegion[8];				// 相邻的地图索引
	int			m_nConRegionID[8];					// 相邻的地图ID
	int			m_nRegionX;							// 在世界中的位置X（象素点）
	int			m_nRegionY;							// 在世界中的位置Y（象素点）
	int			m_nWidth;
	int			m_nHeight;
private:
#ifdef _SERVER
	long		m_Obstacle[REGION_GRID_WIDTH][REGION_GRID_HEIGHT];	// 地图障碍信息表
	DWORD		m_dwTrap[REGION_GRID_WIDTH][REGION_GRID_HEIGHT];	// 地图trap信息表
#endif
	int			m_nNpcSyncCounter;					// 同步计数器
	int			m_nObjSyncCounter;
	int			m_nActive;							// 是否激活（是否有玩家在附近）
	BYTE*		m_pNpcRef;							// 每个格子上的NPC数目
	BYTE*		m_pObjRef;							// 每个格子上的OBJ数目
	BYTE*		m_pMslRef;							// 每个格子上的MISSLE数目
public:
	KRegion();
	~KRegion();
	BOOL		Init(int nWidth, int nHeight);
	BOOL		Load(int nX, int nY);
#ifdef _SERVER
	// 载入服务器端地图上本region 的 object数据（包括npc、trap、box等）
	BOOL		LoadObject(int nSubWorld, int nX, int nY);
	// 载入服务器端地图上本 region 的障碍数据
	BOOL		LoadServerObstacle(KPakFile *pFile, DWORD dwDataSize);
	// 载入服务器端地图上本 region 的 trap 数据
	BOOL		LoadServerTrap(KPakFile *pFile, DWORD dwDataSize);
	// 载入服务器端地图上本 region 的 npc 数据
	BOOL		LoadServerNpc(int nSubWorld, KPakFile *pFile, DWORD dwDataSize);
	// 载入服务器端地图上本 region 的 obj 数据
	BOOL		LoadServerObj(int nSubWorld, KPakFile *pFile, DWORD dwDataSize);
#endif

#ifndef _SERVER
	// 载入客户端地图上本region 的 object数据（包括npc、box等）
	// 如果 bLoadNpcFlag == TRUE 需要载入 clientonly npc else 不载入
	BOOL		LoadObject(int nSubWorld, int nX, int nY, char *lpszPath);
	// 载入客户端地图上本 region 的 clientonlynpc 数据
	BOOL		LoadClientNpc(KPakFile *pFile, DWORD dwDataSize);
	// 载入客户端地图上本 region 的 clientonlyobj 数据
	BOOL		LoadClientObj(KPakFile *pFile, DWORD dwDataSize);
	// 载入障碍数据给小地图
	static void		LoadLittleMapData(int nX, int nY, char *lpszPath, BYTE *lpbtObstacle);
#endif
	void		Close();
	void		Activate();
	BYTE		GetBarrier(int MapX, int MapY, int nDx, int nDy);	//	地图高度

	// 按 像素点坐标 * 1024 的精度判断某个位置是否障碍
	// 参数：nGridX nGirdY ：本region格子坐标
	// 参数：nOffX nOffY ：格子内的偏移量(像素点 * 1024 精度)
	// 参数：bCheckNpc ：是否判断npc形成的障碍
	// 返回值：障碍类型(if 类型 == Obstacle_NULL 无障碍)
	BYTE		GetBarrierMin(int nGridX, int nGridY, int nOffX, int nOffY, BOOL bCheckNpc);

	DWORD		GetTrap(int MapX, int MapY);						//	得到Trap编号
	inline BOOL		IsActive() 
	{
#ifdef _SERVER
		return m_nActive; 
#else
		return TRUE;
#endif
	};
	int			GetRef(int nMapX, int nMapY, MOVE_OBJ_KIND nType);
	BOOL		AddRef(int nMapX, int nMapY, MOVE_OBJ_KIND nType);
	BOOL		DecRef(int nMapX, int nMapY, MOVE_OBJ_KIND nType);
	int			FindNpc(int nMapX, int nMapY, int nNpcIdx, int nRelation);
	int			FindEquip(int nMapX, int nMapY);
	int			FindObject(int nMapX, int nMapY);
	int			FindObject(int nObjID);
	void*		GetObjNode(int nIdx);
	int			SearchNpc(DWORD dwNpcID);		// 寻找本区域内是否有某个指定 id 的 npc (zroc add)
#ifdef _SERVER
	void		SendSyncData(int nClient);
	void		BroadCast(void *pBuffer, DWORD dwSize, int &nMaxCount, int nX, int nY);
	int			FindPlayer(DWORD dwId);
	BOOL		CheckPlayerIn(int nPlayerIdx);
#endif

#ifndef _SERVER
	void		Paint();
#endif
	void		AddNpc(int nIdx);
	void		RemoveNpc(int nIdx);
	void		AddMissle(int nIdx);
	void		RemoveMissle(int nIdx);
	void		AddObj(int nIdx);
	void		RemoveObj(int nIdx);
	BOOL		AddPlayer(int nIdx);
	BOOL		RemovePlayer(int nIdx);
};

//--------------------------------------------------------------------------
//	Find Npc
//--------------------------------------------------------------------------
inline int KRegion::FindNpc(int nMapX, int nMapY, int nNpcIdx, int nRelation)
{
	if (m_pNpcRef[nMapY * m_nWidth + nMapX] == 0)
		return 0;

	KIndexNode *pNode = NULL;
	
	pNode = (KIndexNode *)m_NpcList.GetHead();
	
	while(pNode)
	{
		if (Npc[pNode->m_nIndex].m_MapX == nMapX && Npc[pNode->m_nIndex].m_MapY == nMapY)
		{
			if (NpcSet.GetRelation(nNpcIdx, pNode->m_nIndex) & nRelation)
			{
				return pNode->m_nIndex;
			}
		}
		pNode = (KIndexNode *)pNode->GetNext();
	}	
	return 0;
}
#endif
