// ***************************************************************************************
// 场景中的对象树中的树叶结构定义
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-19
// ---------------------------------------------------------------------------------------
// ***************************************************************************************

#ifndef KIPOTLEAF_H_HEADER_INCLUDED_C2254C34
#define KIPOTLEAF_H_HEADER_INCLUDED_C2254C34

#include "SceneDataDef.h"
#include "../../../Represent/iRepresent/KRepresentUnit.h"
class KIpotBranch;

struct KIpotLeaf;
// 定义枚举回调函数
typedef void (*ObjectsCallbackFn)(void* p, KIpotLeaf* pLeaf);

#define	POINT_LEAF_Y_ADJUST_VALUE	6

//##ModelId=3DD9B1EA02A3
//##Documentation
//## 场景中对象集合树的树叶。
struct KIpotLeaf
{
	//##ModelId=3DD9B259020C
	//##Documentation
	//## 场景中对象集合树的树叶（表示的对象）的类型
	enum IPOTLEAF_TYPE
	{
		//##Documentation
		//## 场景地图的内建对象
		IPOTL_T_BUILDIN_OBJ, 
		//##Documentation
		//## 场景中的发光源
//		IPOTL_T_LIGHT, 
		//##Documentation
		//## 运行时(加入)存在于场景的对象。
		IPOTL_T_RUNTIME_OBJ
	};

	//##ModelId=3DD9B22D006E
	//##Documentation
	//## 对象区分叶子的类型
	IPOTLEAF_TYPE eLeafType;

	//##ModelId=3DD9DD1203A3
	//##Documentation
	//## 指向长在它边上的叶子兄弟。
	KIpotLeaf* pBrother;

	//子叶子
	KIpotLeaf  *pLChild, *pRChild;

	//##ModelId=3DDBF7F8024B
	//##Documentation
	//## 对象的位置坐标。
	POINT oPosition;

};

//## 枚举包含在叶子中的对象
void KIpotLeaf_EnumerateObjects(KIpotLeaf* pLeaf, void* p, ObjectsCallbackFn CallbackFn);
//绘制叶子的对象层
void KIpotLeaf_PaintObjectLayer(KIpotLeaf* pLeaf, RECT* pRepresentArea);
//绘制叶子的对象层之外的其他层
void KIpotLeaf_PaintNoneObjectLayer(KIpotLeaf* pLeaf, RECT* pRepresentArea, int eLayer);
//移除全部的rto叶子
void KIpotLeaf_RemoveAllRtoLeafs(KIpotLeaf* pLeaf, KIpotLeaf* pPermanentRto);
//请除
void KIpotLeaf_Clear(KIpotLeaf* pLeaf);


//##ModelId=3DD9EB4500C5
//##Documentation
//## 处理树中描述运行时对象(存在于场景中但是不隶属于场景地图的对象)的叶子
struct KIpotRuntimeObj : public KIpotLeaf
{
	//##ModelId=3DDBD5DB02DA
	//##Documentation
	//## 如果这片叶子在树枝上，则把它摘下来。
	void Pluck();

	//##ModelId=3DD9EBFE032E
	//##Documentation
	//## 对象类属。
	unsigned int uGenre;

	//##ModelId=3DD9EC170027
	//##Documentation
	//## 对象的id
	int nId;

	int	nPositionZ;

	int eLayerParam;

	//##ModelId=3DDBD19C0345
	//##Documentation
	//## 叶子的所在的树枝的指针
	KIpotBranch* pParentBranch;

	//##Documentation
	//## 叶子的父叶子
	KIpotLeaf*	pParentLeaf;
	//pParentBranch与pParentLeaf同时最多只可有一个为非0。

	//##ModelId=3DDBD1DC03AB
	//##Documentation
	//## 长在同一支树枝上的它前面的叶子兄弟。
	KIpotLeaf* pAheadBrother;
};

//##ModelId=3DD9B745022C
//##Documentation
//## 场景地图上的内建对象，一直存在场景中，永远存在，直到所依附的场景对象寿终正寝。
struct KIpotBuildinObj : public KIpotLeaf
{
	//生成一个克隆体
	KIpotBuildinObj* Clone(POINT* pDivisionPos);
	void PaintABuildinObject(RECT* pRepresentArea);

    //## 图形id
    unsigned int uImage;

	POINT	oEndPos;
	
	float	fAngleXY;
	float	fNodicalY;

    //## 图形原来在iImageStore内的位置。
    short 	nISPosition;
	bool	bClone;
	bool	bImgPart;

	struct KBuildinObj* pBio;
};

void  BuildinObjNextFrame(KBuildinObj* pBio);


#endif /* KIPOTLEAF_H_HEADER_INCLUDED_C2254C34 */
