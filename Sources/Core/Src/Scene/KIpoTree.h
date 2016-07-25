// ***************************************************************************************
// 场景地图中对象集合树
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-19
// ---------------------------------------------------------------------------------------
// ***************************************************************************************

#ifndef KIPOTREE_H_HEADER_INCLUDED_C22523F9
#define KIPOTREE_H_HEADER_INCLUDED_C22523F9

#include "SceneDataDef.h"
#include "KIpotLeaf.h"
#include "KIpotBranch.h"
#include "..\GameDataDef.h"
#include "../../Represent/iRepresent/iRepresentshell.h"

#pragma warning(disable : 4786)
#include <list>
#ifndef __CODEWARRIOR
using namespace std;
#endif

struct KIpotLeaf;
struct KIpotRuntimeObj;
class  KIpotBranch;

// 用于存储遮挡信息的结构
struct KVector2
{
	float fX;
	float fY;
};

struct KLObstacle
{
	int nObstacle;						// 遮挡ID
	KVector2 vDir;						// 障碍方向
    unsigned nReserveForAlign;
};

class KLightBase
{
public:
	KIpotLeaf* m_pParent;				// 光源所属对象指针
	KPosition3 m_oPosition;				// 光源位置
	DWORD m_dwColor;					// 光源颜色及亮度
	long  m_nRadius;					// 作用半径
public:
	virtual void Breath(){}
};

class KSelfBreathLight : public KLightBase
{
public:
	float	fMinRange;					// 最小半径
	float	fMaxRange;					// 最大半径
	float	fRadius;					// 以浮点数表示的光源半径
	float	fCycle;						// 循环周期,毫秒
	int		nDir;						// 半径变化方向
	DWORD	dwLastBreathTime;			// 上次呼吸时间
public:
	KSelfBreathLight();
	virtual void Breath();
};

//##ModelId=3DD95FC001F8
//##Documentation
//## 场景中的对象根据空间相互位置，排列为特殊的树形结构的集合。
class KIpoTree
{
  public:

  	//##ModelId=3DDA92EE0346
	KIpoTree();

	//##ModelId=3DDA92EE0350
	~KIpoTree();

	//##ModelId=3DD9ECFD00E6
	//##Documentation
	//## 绘制这棵对象集合树
	void Paint(RECT* pRepresentArea, enum IPOT_RENDER_LAYER eLayer);

	//##ModelId=3DDA96360240
	//##Documentation
	//## 清空树中所有的内容。包括树枝等在内。仍被外部引用而不能删除的叶子，则转到m_PermanetBranch枝上。
	void Clear();

	//##ModelId=3DE1D38A0173
	//##Documentation
	//## 清空树中所有的内容。包括树枝等在内。但是树上的KIpotRuntimeObj类型的叶子全部摘下,转到m_PermanetBranch枝上。
	void Fell();

	//m_PermanentLeaf上连接的KIpotRuntimeObj类型的叶子，属于KeepRtoArea范围的全部移动到树枝上，其它的都抛弃。
	void StrewRtoLeafs(RECT& KeepRtoArea);

	//##ModelId=3DDAB2E20352
	//##Documentation
	//## 添加树枝。
	//## 成功返回非0值，否则返回0值。
	void AddBranch(
		//##Documentation
		//## 树枝所关联的对象的指针
		KIpotBuildinObj* pBranchObj);

	//##ModelId=3DDA96780082
	//##Documentation
	//## 删除树中所有表述场景中运行时对象的树叶。
//	void RemoveAllRtoLeafs();

	//##ModelId=3DE2A05601A3
	//##Documentation
	//## 删除全部超出范围的rto叶子。
/*	void RemoveOutsideRtoLeafs(
		//##Documentation
		//## 指定的范围
		RECT& rc);
*/
	//##ModelId=3DE3D4AB036F
	//##Documentation
	//## 添加以线方式排序的内建对象叶子
	void AddLeafLine(
		//##Documentation
		//## 以线方式排序的内建对象
		KIpotBuildinObj* pLeaf);

	//##ModelId=3DE3D4AB0371
	//##Documentation
	//## 添加以点方式排序的内建对象叶子
	void AddLeafPoint(
		//##Documentation
		//## 以点方式排序的内建对象
		KIpotLeaf* pLeaf);

	//设置永久树枝的位置
	void SetPermanentBranchPos(int nLeftX, int nRightX, int y);

	// 删除一个KIpotRuntimeObj，同时删除附带的光源
	void PluckRto(KIpotRuntimeObj* pLeaf);

	// 设置九个焦点区域左上角坐标
	void SetLightenAreaLeftTopPos(int nX, int nY)
	{
		m_nLeftTopX = nX;
		m_nLeftTopY = nY;
	}

	void Breathe();

	void AddBuildinLight(KBuildInLightInfo* pLights, int nCount);

	// 设置当前时间，nCurrentTime为从凌晨开始的分钟数
	void SetCurrentTime(int nCurrentTime){m_nCurrentTime = nCurrentTime;}
	// 设置是否是室内
	void SetIsIndoor(bool bIsIndoor){m_bIsIndoor = bIsIndoor;}
	
	// 设置环境光的颜色和亮度
	void SetAmbient(DWORD dwAmbient){m_dwAmbient = dwAmbient;}

	// 渲染光照图
	void RenderLightMap();

	// 设置是否处理地图内建光源，用于表现白天和晚上
	void EnableBioLights(bool bEnable);
	// 设置是否处理动态光照
	void EnableDynamicLights(bool bEnable);

  private:

	//删除与永远的树叶一起的rto叶子
	void RemoveRtoGroupWithPermanentLeaf();

  private:

	//##ModelId=3DDABBB80020
	enum PARAM
	{
		//##Documentation
		//## 叶子标记数值中叶子半指针数据的mask
		LEAF_IDVALUE_HALF_POINTER_MASK = 0xFFFF, 
		//##Documentation
		//## 叶子标记数值中所在树枝值的mask
		LEAF_IDVALUE_BRANCH_VALUE_MASK = 0xFF0000
	};

 private:
	
	//##ModelId=3DDA93C50327
	//##Documentation
	//## 指向主干树枝结构。
	KIpotBranch* m_pMainBranch;

	//##ModelId=3DE1D266030D
	//##Documentation
	//## 永远的树枝。树在枝在，树亡枝亡。
	KIpotBranch m_DefaultBranch;

	//永远的rto树叶，用于组织没有加入树枝的rto叶子
	KIpotLeaf	m_PermanentLeaf;

	// 九个焦点区域左上角X坐标
	int m_nLeftTopX;
	// 九个焦点区域左上角Y坐标
	int m_nLeftTopY;
	
	int m_nCurrentTime;
	bool m_bIsIndoor;

	// 是否处理地图内建光源
	bool m_bProcessBioLights;
	// 是否处理动态光
	bool m_bDynamicLighting;

	// 环境光的颜色和强度
	DWORD m_dwAmbient;
	KLObstacle pObstacle[LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT];	// 挡光信息数组
	//KLColor pLColor[LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT];		// 保存光照计算的中间结果
	KLColor *pLColor;                                           		// 保存光照计算的中间结果
    unsigned char *m_pbyLColorBase;
	KLColor *ptp;                                                       // 保存经过平滑过渡的光照计算的中间结果
	unsigned char *m_pby_ptpBase;


	DWORD pLightingArray[LIGHTING_GRID_WIDTH * LIGHTING_GRID_HEIGHT];	// 光照图信息数组
	list<KLightBase*> m_LightList;

	// 计算光照阻挡的回调函数
	static void ObjectsCallback(void* p, KIpotLeaf* pLeaf);
	void FillLineObstacle(int nX1, int nY1, int nX2, int nY2, int nObstacleID);
	bool CanLighting(int x, int y, int nLightPosX, int nLightPosY);
};

#endif /* KIPOTREE_H_HEADER_INCLUDED_C22523F9 */
