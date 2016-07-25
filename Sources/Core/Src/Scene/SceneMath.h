// ***************************************************************************************
// 场景模块的一些数学计算
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-20
// ---------------------------------------------------------------------------------------
// ***************************************************************************************
#ifndef __SCENEMATH__H__
#define __SCENEMATH__H__

#define	PI_PER_ANGLE_DEGREE				0.0174532925f
#define	LINE_LINKABLE_SLOPE_RANGE		0.01f
#define LINE_LINKABLE_NODICALY_RANGE	16.0f

enum RELATION_ENUM
{
	RELATION_UP,
	RELATION_ON,
	RELATION_DOWN,
	RELATION_CROSS
};

//判断对象是否在绘制范围内
#ifdef _EXCLUDE_OUTSIDE_OBJECT
	#define	SM_IsOutsideRepresentArea(_prc, _x, _y, _z, _ex, _ey, _ez)	\
		(	(_ex) < (_prc)->left || (_x) > (_prc)->right ||				\
			(_ey) - (((_ez) * 887) >> 9) < (_prc)->top ||				\
			(_y) - (((_z) * 887) >> 9) > (_prc) ->bottom )
#else
	#define SM_IsOutsideRepresentArea(_prc, _x, _y, _z, _ex, _ey, _ez) false
#endif

//判断是否两个线段可以连接成线
#define	SM_IsLineLinkable(fSlope1, fNodicalY1, fSlope2, fNodicalY2)	\
	(fabs((fSlope1) - (fSlope2)) < LINE_LINKABLE_SLOPE_RANGE &&		\
		fabs((fNodicalY1) - (fNodicalY2)) < LINE_LINKABLE_NODICALY_RANGE)

int		SM_IsPointOnLine(POINT& LP1, POINT& LP2, POINT point);

//点到直线的距离
int				SM_Distance_PointLine(const POINT& Point, const POINT& LP1, const POINT& pLP2);
//点和直线的位置关系
RELATION_ENUM	SM_Relation_PointLine(const POINT& Point, const POINT& LP1, const POINT& pLP2);
//线和线的位置关系
RELATION_ENUM	SM_Relation_LineLine_CheckCut(const POINT& LP11, const POINT& LP12, const POINT& LP21, const POINT& LP22, POINT& POI);

//void	SM_GetLineEndPointAndNodicalX(float x1, float y1, int nAngleXY, int nWidth, float& x2, float& y2, int& nNodicalX);
void	SM_GetPointFromFloatSting32(char* pString, float& x, float& y, float& z);
void	ChangeFileName(char* pPathFile, char* pNewFileName);

inline int	SM_Relation_LineLine(const POINT& LP11, const POINT& pLP12, const POINT& LP21, const POINT& pLP22)
{
	int nRet;
	if (LP11.y < LP21.y)
		nRet = -1;
	else if (LP11.y > LP21.y)
		nRet = 1;
	else
		nRet = 0;
	return nRet;
}



#endif //#ifndef __SCENEMATH__H__
