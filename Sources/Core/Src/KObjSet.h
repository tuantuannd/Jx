//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KObjSet.h
// Date:	2002.01.06
// Code:	边城浪子
// Desc:	Obj Class
//---------------------------------------------------------------------------
#ifndef	KObjSetH
#define	KObjSetH

#include "KObj.h"
#include "KLinkArray.h"

#define		MAX_OBJ_NAME_COLOR		16

typedef struct
{
	char	m_szCopyRight[31];
	BYTE	m_btVersion;
	DWORD	m_dwNum;
} KObjMapDataHead;

typedef struct
{
	int		m_nTemplateID;
	int		m_nState;
	int		m_nDir;
	int		m_nPos[3];
	DWORD	m_dwScriptLength;
	char	m_szScript[80];
} KObjMapData;

typedef struct
{
	int		m_nItemID;
	int		m_nMoneyNum;
	int		m_nItemWidth;
	int		m_nItemHeight;
	int		m_nColorID;
	int		m_nMovieFlag;
	int		m_nSoundFlag;
	char	m_szName[FILE_NAME_LENGTH];
} KObjItemInfo;

class KObjSet
{
public:
	KTabFile	m_cTabFile;
	KTabFile	m_cMoneyFile;
	int			m_nObjID;
#ifndef _SERVER
	DWORD		m_dwNameColor[MAX_OBJ_NAME_COLOR];
	int			m_nShowNameFlag;		// 是否全部显示 item 和 money 类的 object 的名字在头顶上
#endif
private:
	KLinkArray	m_UseIdx;
	KLinkArray	m_FreeIdx;
public:
	KObjSet();
	~KObjSet();
	BOOL	Init();
	int		GetID();
	void	Remove(int nIdx);
#ifdef _SERVER
	// 添加一个obj，返回在obj数组中的位置编号（如果 < 0 ，失败）
	int		Add(int nDataID, KMapPos MapPos, KObjItemInfo sItemInfo);

	// 从obj数据文件中载入相应数据
	int		AddData(int nDataID, KMapPos MapPos, int nMoneyNum = 0, int nItemID = 0, int nItemWidth = 0, int nItemHeight = 0);

	int		AddMoneyObj(KMapPos MapPos, int nMoneyNum);

	// 服务器端载入一个Region的所有Obj
	BOOL	ServerLoadRegionObj(char *lpszMapPath, int nRegionX, int nRegionY, int nSubWorld);

	// 服务器端载入一个Region的所有Obj
	BOOL	ServerLoadRegionObj(int nSubWorld, KPakFile *pFile, DWORD dwDataSize);

	// 设定obj所带的物件信息，包括物件id，物件在装备栏中的长、宽
	BOOL	SetObjItem(int nObjIndex, int nItemID, int nItemWidth, int nItemHeight);

	BOOL	SyncAdd(int nID, int nClient);
#endif

#ifndef _SERVER
	int		AddData(int nDataID, int nSubWorld, int nRegion, int nMapX, int nMapY, int nOffX, int nOffY);
	int		ClientAdd(int nID, int nDataID, int nState, int nDir, int nCurFrame, int nXpos, int nYpos, KObjItemInfo sInfo);
	BOOL	ClientLoadRegionObj(char *lpszMapPath, int nRegionX, int nRegionY, int nSubWorld, int nRegion);
	BOOL	ClientLoadRegionObj(KPakFile *pFile, DWORD dwDataSize);
	void	RemoveIfClientOnly(int nIdx);
	int		SearchObjAt(int nX, int nY, int nRange);
	DWORD	GetNameColor(int nColorID);
	void	SetShowNameFlag(BOOL bFlag);
	BOOL	CheckShowName();
#endif
	int		FindID(int nID);
	int		FindName(char *lpszObjName);
private:
	int		FindFree();
	int		CheckClientKind(int nKind);
	int		GetDataIDKind(int nDataID);
#ifdef _SERVER
	int		GetMoneyDataId(int nMoney);
#endif
};

extern	KObjSet	ObjSet;
extern	char	g_szObjKind[Obj_Kind_Num][32];

#endif

/*
当服务器接收到需要产生一个新的物件的要求后，例如在地板上放置一件装备，为了确定
这件装备，需要一些信息：
    1、位置信息：这是最基本的必需的信息
    2、基本数据的信息（DataID），这些信息包括：物件的名称、类型、障碍信息等同一
       种DataID共通的信息，这些信息存放在 ObjData.txt 文件里面，通过读取这些信
       息可以初步确定出这个物件
    3、具体的每个实例之间不同的信息，比如：当前的状态、当前的方向（有些类型的物
       件可能需要处理方向信息）、当前的生命时间（有些类型的物件有生命时间及相应
       的处理）、当前所带脚本文件名称、当前的装备数据ID（装备类）、服务器端用于
       动画控制的各种信息（地图机关类等）、当前障碍信息（门类可能有打开、关闭状
       态等）
当确定有足够的信息后，可以调用具体的方法在服务器端生成一个新的物件，具体到使用
上，首先调用 AddData() 方法，把第 1 、第 2 项信息传入，这个方法会自动从 Object
数组中寻找一个空项，加载入相应的基本数据，产生一个新的物件，返回新物件在数组中
的位置xxx；然后给新物件安一个世界唯一ID，通知客户端产生一个对应的新物件；然后就
可以通知相应SubWorld的相应Region开始管理这个新物件：
    SubWorld[nSubWorld].m_WorldMessage.Send(GWM_OBJ_ADD, nRegion, xxx);
然后设定此物件的一些个体信息，即设定第 3 项信息，可能需要调用的方法有：
    Object[xxx].SetLifeTime()
    Object[xxx].SetState()
    Object[xxx].SetDir()
    Object[xxx].SetItemDataID()
    Object[xxx].SetScriptFile() 等；
至此，一个新的物件在服务器端已经产生好了；
而客户端当接收到服务器端的产生新物件的命令后，调用 ClientAdd() 方法在客户端产生
一个对应的新物件，具体产生方法为：首先自动从 Object 数组中寻找一个空项，加载入
相应的基本数据，产生一个新的物件，返回新物件在数组中的位置xxx；把接收到的世界唯
一 ID 给它安上，然后设定各种接收到的参数，最后通知相应SubWorld的相应Region开始
管理这个新物件；至此，新物件已经产生出来了，但是它的各项参数还没有最终设定完成，
需要以后服务器端发过来的同步信息才能最终得到正确的表现。
*/







