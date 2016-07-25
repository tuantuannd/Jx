// ***************************************************************************************
// 场景对象树上的树枝
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-19
// ---------------------------------------------------------------------------------------
// ***************************************************************************************

#ifndef KIPOTBRANCH_H_HEADER_INCLUDED_C22510AB
#define KIPOTBRANCH_H_HEADER_INCLUDED_C22510AB

struct KIpotLeaf;
struct KIpotRuntimeObj;
struct KIpotBuildinObj;

//##ModelId=3DD9DE49027E
//##Documentation
//## 描述对象树的树枝的结构。
class KIpotBranch
{
  friend struct KIpotRuntimeObj;
  public:

	//##ModelId=3DDAC4AC0238
	KIpotBranch();

	//##ModelId=3DDAC4AC0242
	~KIpotBranch();
	
	//##ModelId=3DDACD580262
	//##Documentation
	//## 添加树枝。
	void AddBranch(
		//##Documentation
		//## 树枝所关联的对象的指针
		KIpotBuildinObj* pBranchObj);

	//##ModelId=3DDACA8B015D
	//##Documentation
	//## 删除树枝上所有表述场景中运行时对象的树叶。
	void RemoveAllRtoLeafs(
		//##Documentation
		//## 接受摘下来的叶子的树枝。
		KIpotLeaf* pPermanentRto);

	//##ModelId=3DDACA410279
	//##Documentation
	//## 绘制树枝
	void PaintObjectLayer(RECT* pRepresentArea);
	
	//## 枚举包含在树枝中的对象
	void EnumerateObjects(void* p, ObjectsCallbackFn CallbackFn);

	void PaintNoneObjectLayer(RECT* pRepresentArea, int eLayer);

	//##ModelId=3DDC082502F0
	//##Documentation
	//## 清空树枝上所有的内容。
	void Clear();

	//##ModelId=3DDC101202EE
	//##Documentation
	//## 设置树枝对应的对象以及排序用的线段。
	void AddAObject(
		//##Documentation
		//## 此树枝关联的场景内建对象
		KIpotBuildinObj* pBranchObj);

	void SetLine(
		//##Documentation
		//## 线段的其中一点
		POINT* pPoint1,
		//##Documentation
		//## 线段的另外一点
		POINT* pPoint2);

	//##ModelId=3DE2A08602E2
	//##Documentation
	//## 删除全部超出范围的rto叶子。
/*	void RemoveOutsideRtoLeafs(
		//##Documentation
		//## 指定范围
		RECT& rc);
*/
	//##ModelId=3DE3CBBB03E7
	//##Documentation
	//## 添加以线方式排序的内建对象
	void AddLeafLine(
		//##Documentation
		//## 以线方式排序的内建对象
		KIpotBuildinObj* pLeaf);

	//##ModelId=3DE3CC2401EA
	//##Documentation
	//## 添加以点方式排序的内建对象
	void AddLeafPoint(
		//##Documentation
		//## 以点方式排序的内建对象
		KIpotLeaf* pLeaf);

  private:

	//##Documentation
	//## 添加树枝。
	void AddSubBranch(
		int nSubIndex, 
		//##Documentation
		//## 树枝结构的指针
		KIpotBuildinObj* pBranchObj);

	void AddLineLeafToList(KIpotLeaf*& pFirst, KIpotBuildinObj* pLeaf);
	void AddPointLeafToList(KIpotLeaf*& pFirst, KIpotLeaf* pLeaf, KIpotLeaf* pParentLeaf);
	
	void PaintABranchObject(KIpotBuildinObj* pObject, RECT* pRepresentArea) const;

  private:

	//##ModelId=3DDA93F50055
	enum IPOT_BRANCH_FLAG
	{ 
		//##Documentation
		//## KIpotBranch结构实例对象中pLeftBranch成员有意义且非0，pLeftLeafs成员无意义。
		IPOT_BF_HAVE_LEFT_BRANCH = 1, 
		//##Documentation
		//## KIpotBranch结构实例对象中pRightBranch成员有意义且非0，pRightLeafs成员无意义。
		IPOT_BF_HAVE_RIGHT_BRANCH = 2
	};

  private:

	//##ModelId=3DDA74570346
	//##Documentation
	//## 此树枝节点的标记
	unsigned int m_uFlag;

	//##ModelId=3DD9DE7300C6
	//##Documentation
	//## 父结点，树枝连接的大树枝。
	KIpotBranch* m_pParent;

	union
	{
		//##ModelId=3DD9DEAF0307
		//##Documentation
		//## 分叉树枝。
		//## 与pLeafs为union关系，两者取值同一时刻只有一个有意义。
		KIpotBranch* m_pSubBranch[2];

		//##ModelId=3DD9E03503E4
		//##Documentation
		//## 生在树枝的叶子列表。
		//## 与pSubBranch为union关系，两者取值同一时刻只有一个有意义。
		KIpotLeaf* m_pLeafs[2];
	};

	//##ModelId=3DDAB05102A3
	//##Documentation
	//## 树枝的上侧端点的坐标
	POINT m_oHeadPoint;

	//##ModelId=3DDAB10F026A
	//##Documentation
	//## 树枝的下侧端点的坐标
	POINT m_oEndPoint;


	//##ModelId=3DE3CB760294
	//##Documentation
	//## 此树枝关联的场景内建对象
	KIpotBuildinObj*  m_pFirstObject;
	KIpotBuildinObj** m_pObjectList;
	int		m_nNumObjects;
	int		m_nNumObjectSpace;

  private:

	static unsigned int	m_BranchFlag[2];

};



#endif /* KIPOTBRANCH_H_HEADER_INCLUDED_C22510AB */
