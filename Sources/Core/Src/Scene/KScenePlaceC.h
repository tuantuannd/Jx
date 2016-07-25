// ***************************************************************************************
// 场景地图（客户端版）
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-11
// ---------------------------------------------------------------------------------------
// ***************************************************************************************

#ifndef KSCENEPLACEC_H_HEADER_INCLUDED_C22D00B5
#define KSCENEPLACEC_H_HEADER_INCLUDED_C22D00B5
#include "../../Represent/iRepresent/KRepresentUnit.h"
#include "KScenePlaceRegionC.h"
#include "SceneDataDef.h"
#include "KIpoTree.h"
#include "ScenePlaceMapC.h"

// 环境光数组索引
/*
子：（23：00－1：00）RGB1
丑：（1：00－3：00）RGB1－RGB2
寅：（3：00－5：00）RGB2
卯：（5：00－7：00）RGB2－RGB3
辰：（7：00－9：00）RGB3－RGB4
巳：（9：00－11：00）RGB4－RGB5
午：（11：00－13：00）RGB5
未：（13：00－15：00）RGB5－RGB4
申：（15：00－17：00）RGB4－RGB6
酉：（17：00－19：00）RGB6－RGB7
戌：（19：00－21：00）RGB7－RGB1
亥：（21：00－23：00）RGB1
*/
#define ENVLIGHT_MIDNIGHT	0	//RGB1：午夜的黑暗
#define ENVLIGHT_DAWN		1	//RGB2：天亮前有些发蓝的黑暗
#define ENVLIGHT_MORNING	2	//RGB3：太阳刚出来时的亮度
#define ENVLIGHT_FORENOON	3	//RGB4：地图的正常亮度
#define ENVLIGHT_NOON		4	//RGB5：正午的高亮
#define ENVLIGHT_DUSK		5	//RGB6：太阳落山的晚霞亮度
#define ENVLIGHT_EVENING	6	//RGB7：傍晚灰蒙蒙的亮度

class EnvironmentLight
{
private:
	KLColor m_cLight[7];
public:
	EnvironmentLight();
	// 设置第nIdx个颜色
	void SetLight(const KLColor &cLight, int nIdx);
	// 设置第nIdx个颜色
	void SetLight(BYTE r, BYTE g, BYTE b, int nIdx);
	// 设置所有7个颜色
	void SetLight(KLColor *pLight);
	// 取得距一天开始nMinutes分钟时的环境光颜色
	DWORD GetEnvironmentLight(int nMinutes);
};

//================================================================
//	场景中一个区域被加载完毕后的回调函数的函数原型
//================================================================
//	参数: nRegionH, nRegionV 分别为加载完毕的区域的横向与纵向索引
typedef void (*funScenePlaceRegionLoadedCallback)(int nRegionH, int nRegionV);

class KWeather;


#define KMAX_PREV_LOAD_POS_NUM  5
typedef struct _KPrevLoadPosItem
{
    int     m_nNum;
    POINT   m_Pos[KMAX_PREV_LOAD_POS_NUM];

} KPrevLoadPosItem;


#define MAX_PREV_LOAD_FILE_COUNT    1024
#define PREV_LOAD_FILE_MIN_SIZE     (32 * 1024)

typedef struct _KPrevLoadFileNameAndFrameAndFrame
{
    char szName[MAX_RESOURCE_FILE_NAME_LEN];
    int  nFrame;
} KPrevLoadFileNameAndFrame;



//typedef struct _KPrevLoadItem
//{
//    POINT
//
//} KPrevLoadItem;


//##ModelId=3DB8F8B40128
//##Documentation
//## 场景的一个地图。
class KScenePlaceC
{
  public:
	//##ModelId=3DBE3B53008C
	//##Documentation
	//## 构造函数。
	//## 函数体一定包含此语句m_szPlaceRootPath[0] = 0
	KScenePlaceC();

	//##ModelId=3DD17A770383
	//##Documentation
	//## call Terminate
	~KScenePlaceC();

	//##ModelId=3DCAA6A703DB
	//##Documentation
	//## 初始化
	bool Initialize();

	//##ModelId=3DCD58AC00BC
	//##Documentation
	//## 结束对象功能。释放对象的全部数据与动态构造的资源。
	void Terminate();

	//##ModelId=3DCAA64C01DA
	//##Documentation
	//## 更换/加载指定的场景地图。
	bool OpenPlace(
		//##Documentation
		//## 场景地图索引
		int nPlaceIndex);

	//##ModelId=3DCAAE3703A6
	//##Documentation
	//## 关闭场景地图。
	void ClosePlace();

	//##ModelId=3DCD7F0A0071
	//##Documentation
	//## 绘制场景
	void Paint();

	//##ModelId=3DBCE7B70358
	//##Documentation
	//## 设置场景地图的焦点
	void SetFocusPosition(
		//##Documentation
		//## 场景焦点坐标的x分量
		int nX, 
		//##Documentation
		//## 场景焦点坐标的y分量
		int nY, 
		//##Documentation
		//## 场景焦点坐标的z分量
		int nZ);

	void GetFocusPosition(
		//##Documentation
		//## 场景焦点坐标的x分量
		int& nX, 
		//##Documentation
		//## 场景焦点坐标的y分量
		int& nY, 
		//##Documentation
		//## 场景焦点坐标的z分量
		int& nZ);

//----关于不隶属于场景地图的对象与场景关系的操作----

	//##ModelId=3DCAA6B90196
	//##Documentation
	//## 添加运行时对象(存在于场景中但是不隶属于场景地图的对象)到场景中。
	//## 返回添加对象添加到场景中获得的标记。如果返回0值表示添加对象失败。或者对象没有不在场景落在场景当前的处理区域中，添加操作被忽略。
	unsigned int AddObject(
		//##Documentation
		//## 要加入的对象类属。
		unsigned int uGenre, 
		//##Documentation
		//## 要加入的对象的id
		int nId, 
		//##Documentation
		//## 加入的位置坐标。
		int x, int y, int z,
		int eLayerParam = IPOT_RL_OBJECT);

	//##ModelId=3DCAA7000085
	//##Documentation
	//## 移动存在于场景中但是不隶属于场景地图的对象在场景中的位置。
	//## 如果对象移动成功则返回对象在场景地图中的标记数值，为非0值；如果失败则返回0；如果对象移出了场景当前处理的区域也返回0。
	unsigned int MoveObject(
		//##Documentation
		//## 要移动的对象类属。
		unsigned int uGenre, 
		//##Documentation
		//## 要移动的对象的id
		int nId, 
		//##Documentation
		//## 对象的目的坐标。
		int x, int y, int z, 		 
		//##Documentation
		//## 移动前对象在场景中的标记数值。如果对象是新加入，则传入0值。
		unsigned int& uRtoid,
		int eLayerParam = IPOT_RL_OBJECT);

	//##ModelId=3DCAA70603E3
	//##Documentation
	//## 去除存在于场景中但是不隶属于场景地图的对象
	void RemoveObject(
		//##Documentation
		//## 要去除的对象类属。
		unsigned int uGenre, 
		//##Documentation
		//## 要去除的对象的id
		int nId, 
		//##Documentation
		//## 对象被清除前在场景中的标记数值。
		unsigned int& uRtoid);

	void Breathe();
	
	void SetRepresentAreaSize(int nWidth, int nHeight);

	void ProjectDistToSpaceDist(int& nXDistance, int& nYDistance);

	//视图/绘图设备坐标 转化为空间坐标
	void ViewPortCoordToSpaceCoord(
		int& nX,	//传入：视图/绘图设备坐标的x量，传出：空间坐标的x量
		int& nY,	//传入：视图/绘图设备坐标的y量，传出：空间坐标的y量
		int  nZ		//（期望）得到的空间坐标的z量
		);
	
	void GetRegionLeftTopPos(int nRegionX, int nRegionY, int& nLeft, int& nTop);
	
	long GetObstacleInfo(int nX, int nY);
	long GetObstacleInfoMin(int nX, int nY, int nOffX, int nOffY);

	void RepresentShellReset();


	//设置场景中一个区域被加载完毕后的回调函数
	void	SetRegionLoadedCallback(funScenePlaceRegionLoadedCallback pfunCallback);
	
	void	SetHightLightSpecialObject(int nRegionX, int nRegionY, int nBioIndex);
	void	UnsetHightLightSpecialObject();

	void	GetSceneNameAndFocus(char* pszName, int& nId, int& nX, int& nY);

	// 设置环境光的颜色和亮度
	void SetAmbient(DWORD dwAmbient);
	void SetCurrentTime(DWORD dwCurrentTime);
	// 设置24小时环境光变化数组的第idx个单元，idx取值0－6
	void SetEnvironmentLight(int idx, BYTE r, BYTE g, BYTE b);

	// 设置是否处理地图内建光源，用于表现白天和晚上
	void EnableBioLights(bool bEnable);
	// 设置是否处理动态光照
	void EnableDynamicLights(bool bEnable);
	
	void ChangeWeather(int nWeatherID);

	//获取场景的小地图信息
	int GetMapInfo(KSceneMapInfo* pInfo);
	//设置地图参数
	void SetMapParam(unsigned int uShowElems, int nSize);
	//设置场景的地图的焦点(单位:场景坐标)
	void SetMapFocusPositionOffset(int nOffsetX, int nOffsetY);
	//小地图绘制
	void  PaintMap(int nX, int nY);
	//设置是否跟随地图的移动而移动
	void FollowMapMove(int nbEnable);

	void EnableWeather(int nbEnable);


private:

	//##ModelId=3DCE68BB0238
	//##Documentation
	//## 改变加载范围。
	void ChangeLoadArea();

	//##ModelId=3DBF946D0053
	//##Documentation
	//## 先对全部离开处理范围的区域对象，调用FreePrerender方法，释放这些区域对象预渲染好的大张地表图，然后设置预处理信号。
	void ChangeProcessArea();

	//##ModelId=3DBFA1460230
	//##Documentation
	//## 场景处理范围内的区域对象的预处理。
	void Preprocess();

	//##ModelId=3DCCBD7B0239
	//##Documentation
	//## 清除/释放预处理产生的数据与信息。
	void ClearPreprocess(int bIncludeRto);

	//##ModelId=3DBDBC7200B4
	//##Documentation
	//## 设置新的要加载的区域。
	void SetRegionsToLoad();

	//##ModelId=3DCB6BC90345
	//##Documentation
	//## 加载区域数据的过程，具体内容见活动图：
	//## Use Case View/场景/客户端/场景地图加载/State/Activity Model/加载区域的机制 中子线程空间的执行内容
	void LoadProcess();

	//##ModelId=3DCCD131018C
	//##Documentation
	//## 加载线程的执行入口函数。
	//## 以pParam作为KScenePlaceC对象的指针，调用它的LoadAndPreprocess方法。
	//## 返回值固定为0。
	static DWORD WINAPI LoadThreadEntrance(void* pParam);

	void PrerenderGround(bool bForce);

	void ARegionLoaded(KScenePlaceRegionC* pRegion);
	
	KRUImage* GetFreeGroundImage();

	KScenePlaceRegionC*	GetLoadedRegion(int h, int v);

	void SetLoadingStatus(bool bLoading);

    void PreLoadProcess();

private:

	//##ModelId=3DCAC1D103D7
	//##Documentation
	//## KScenePlaceC运行参数
	enum SP_WORKING_PARAM { 
		//##Documentation
		//## (动态)加载范围，纵/横向在中心区域四侧还要延伸出的范围。(单位：区域)
		//## 其取值来自(加载范围纵/横向跨越的区域的数目-1)/2
		SPWP_LOAD_EXTEND_RANGE = 3,
		//##Documentation
		//## 最多加载的区域的数目
		SPWP_MAX_NUM_REGIONS = 49,
		//##Documentation
		//## 最多能够进入处理范围的区域的数目
		SPWP_NUM_REGIONS_IN_PROCESS_AREA = 9,
		//##Documentation
		//## 焦点移动横向或者纵向跨越多少个区域之后重设动态加载范围
		SPWP_TRIGGER_RANGE = 2,
		//## 焦点移动横向或者纵向突然跨越多少个区域会触动进入加载中状态
		SPWP_TRIGGER_LOADING_RANGE = 4,
		//##Documentation
		//## 处理的跨度
		SPWP_PROCESS_RANGE = 3,
		//##Documentation
		//## 表示一个遥远的坐标值，远离有效的坐标范围。
	    SPWP_FARAWAY_COORD = -2147476129,

		SPWP_REPRESENT_RECT_WINDAGE_X = 140,
		SPWP_REPRESENT_RECT_WINDAGE_T = 90,
		SPWP_REPRESENT_RECT_WINDAGE_B = 150,
		//##Documentation
		//##能触发预先处理与预渲染的已加载区域树目
		SPWP_PROCESS_PRERENDER_REGION_COUNTER_TRIGGER = 25,
		//##Documentation
		//##无加量显示的对象的标志
		SPWP_NO_HL_SPECAIL_OBJECT = -1,
		//##Documentation
		//##切换地图加载的超时时限(单位：毫秒)
        SPWP_SWITCH_SCENE_TIMEOUT = 30000,
		//##Documentation
		//##rto对象的大约一半宽/高度
        SPWP_RTO_HALF_RANGE = 80,
		//没有场景
		SPWP_NO_SCENE = -1,
	};

private:

	//场景中一个区域被加载完毕后的回调函数的指针。
	funScenePlaceRegionLoadedCallback m_pfunRegionLoadedCallback;

	//##ModelId=3DD2CA84021C
	//##Documentation
	//## 标志对象是否已经成功执地被初始化了。
	bool	m_bInited;

	//## 正在加载中
	bool	m_bLoading;

	bool	m_bEnableWeather;

	//是否跟随地图的移动而移动
	bool	m_bFollowWithMap;

	//原始的焦点坐标，此值只在m_bFollowWithMap为真值时有效
	POINT	m_OrigFocusPosition;

	//地图焦点坐标的偏移
	POINT	m_MapFocusOffset;

	//##ModelId=3DCE5BF203D6
	//##Documentation
	//## 焦点坐标,此点在z=0平面上。
	POINT m_FocusPosition;

	//##ModelId=3DCD42F60221
	//##Documentation
	//## 焦点所在的区域的区域纵横索引坐标
	POINT m_FocusRegion;

	//##ModelId=3DD3B98002E1
	//##Documentation
	//## 当前的焦点所在的区域的索引与旧的焦点所在的区域的索引值加的差值。
	//## 暨累计焦点移动纵向与横向跨越'区域'的数目。
	SIZE m_FocusMoveOffset;

	//##ModelId=3DCB792F02D7
	//##Documentation
	//## 触发加载区域的信号（简称加载信号）
	HANDLE m_hLoadRegionEvent;

	//## 切换地图加载结束的信号
	HANDLE m_hSwitchLoadFinishedEvent;

	//##ModelId=3DCD478001BA
	//##Documentation
	//## 触发预处理计算的信号(简称预处理信号)
	int	m_bPreprocessEvent;

	//##ModelId=3DCB83F30160
	//##Documentation
	//## 九区域数据处理/欲处理临界区。
	CRITICAL_SECTION m_ProcessCritical;

	//##ModelId=3DCAB77A01E9
	//##Documentation
	//## 加载场景区域修改关键数据的临界区对象。
	CRITICAL_SECTION m_LoadCritical;

	//##ModelId=3DCB84480342
	//##Documentation
	//## 访问m_RegionListAdjustCritical的临界区。
	CRITICAL_SECTION m_RegionListAdjustCritical;


	//##ModelId=3DB907ED02B7
	//##Documentation
	//## （固定）一定数目的Region对象实例。
	KScenePlaceRegionC m_RegionObjs[SPWP_MAX_NUM_REGIONS];

	//##ModelId=3DDBC73803A4
	//##Documentation
	//## 进入处理范围的以加载区域的指针。
	KScenePlaceRegionC* m_pInProcessAreaRegions[SPWP_NUM_REGIONS_IN_PROCESS_AREA];

	KRUImage	m_RegionGroundImages[SPWP_NUM_REGIONS_IN_PROCESS_AREA];
	int			m_nNumGroundImagesAvailable;

	//##ModelId=3DCAB33A01CE
	//##Documentation
	//## 场景地图文件的根目录（暨场景地图根文件所在的目录）。
	//## KScenePlaceC借助m_szPlaceRootPath[0]是否为0来作为是否打开场景地图的判断依据。
	char m_szPlaceRootPath[80];
	char m_szSceneName[32];
	int	 m_nSceneId;
	//##ModelId=3DD2E1410028
	//##Documentation
	//## m_pRegions列表的索引，如果m_nFirstToLoadIndex的值在有效范围内，则表示m_pRegions列表于m_nFirstToLoadIndex索引之前的元素为已经加载了所需数据的区域对象的指针，剩于的元素为待加载数据的区域对象的指针。如果m_nFirstToLoadIndex的值不在有效范围内则表示列表中全部指针所表述的区域对象都已经加载了所需的数据。
	int m_nFirstToLoadIndex;

	//##ModelId=3DD2E1410064
	//##Documentation
	//## 区域对象指针列表。
	//## 列表中的每个元素内容都是一个区域对象的指针，这些指针分别就是指向m_RegionObjs中的一个元素，是一一对应的关系。区域对象指针列表中个元素的顺序会根据运算发生变化。
	KScenePlaceRegionC* m_pRegions[SPWP_MAX_NUM_REGIONS];

	//##ModelId=3DD3A6140179
	//##Documentation
	//## 执行加载区域数据与预处理的线程的句柄。
	HANDLE m_hLoadAndPreprocessThread;

	//##ModelId=3DD960150394
	//##Documentation
	//## 处于处理范围内的部分对象的集合。
	KIpoTree m_ObjectsTree;

	RECT m_RepresentArea;
	SIZE m_RepresentExactHalfSize;
	
	long m_bRenderGround;

	int		m_nHLSpecialObjectRegionX, m_nHLSpecialObjectRegionY;
	int		m_nHLSpecialObjectBioIndex;

	KBuildinObj**	m_pObjsAbove;
	unsigned int	m_nNumObjsAbove;

	// 当前时间，一天开始的分钟数
	int m_nCurrentTime;

	// 处理环境光的对象
	EnvironmentLight m_EnLight;
	// 天气对象指针
	KWeather *m_pWeather;
	//小地图
	KScenePlaceMapC	m_Map;

private:
	//##ModelId=3DDB39BA029B
	//##Documentation
	//## 计算加载区域的优先次序的表
	static int m_PRIIdxTable[SPWP_MAX_NUM_REGIONS];

	//##ModelId=3DDB39150334
	//##Documentation
	//## 计算加载范围内场景区域的索引坐标用的表
	static POINT m_RangePosTable[SPWP_MAX_NUM_REGIONS];

	// 根据玩家前进的方向，需要加载的相对玩家当前Region的偏移
    static const KPrevLoadPosItem m_PrevLoadPosOffset[3][3];

	// 根据玩家前进的方向，需要加载的Region
    KPrevLoadPosItem    m_PreLoadPosItem;

    int                 m_nPrevLoadFileCount;
    KPrevLoadFileNameAndFrame   m_PrevLoadFileNameAndFrames[MAX_PREV_LOAD_FILE_COUNT];

};

extern KScenePlaceC	g_ScenePlace;
#endif /* KSCENEPLACEC_H_HEADER_INCLUDED_C22D00B5 */
