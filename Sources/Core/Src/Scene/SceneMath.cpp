// ***************************************************************************************
// 场景模块的一些数学计算
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-20
// ***************************************************************************************
#include "KCore.h"
#include "Windows.h"
#include "SceneMath.h"
#include <crtdbg.h>
#include <math.h>
#include "..\Engine\Src\KTimer.h"

#define TOGGLE_VALUE_0_1		10

#define TOGGLE_VALUE_0_1_SQRT   3

int	SM_IsPointOnLine(POINT& LP1, POINT& LP2, POINT point)
{
	int nRet = 0;
	if (LP1.x == LP2.x)
	{
		if (point.x = LP1.x)
			nRet = (LP1.y != LP2.y) || (point.y == LP1.y);
	}
	else
	{
		int y = LP1.y + (LP2.y - LP1.y) * (point.x - LP1.x) / (LP2.x - LP1.x);
		nRet = ((y - point.y) * (y - point.y) <= 4);
	}
	return nRet;
}

//点到直线的距离
int	SM_Distance_PointLine(const POINT& Point, const POINT& LP1, const POINT& LP2)
{
	int		x1, y1,x2,y2;
	x1 = LP1.x - Point.x;
	y1 = LP1.y - Point.y;
	x2 = LP2.x - Point.x;
	y2 = LP2.y - Point.y;

//  直线方程：
//	y(x2-x1) - x(y2-y1) + (x1y2 - x2y1) = 0 
//	int d = abs(pt.y*(x2-x1) - pt.x*(y2-y1) + (x1*y2 - x2*y1) ) / sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

	int d = (int)(abs(x1*y2 - x2*y1) / sqrt(double(x2-x1)*(x2-x1) + (y2-y1)*(y2-y1))); // Fixed By MrChuCong@gmail.com
	return d;
}

//点和直线的位置关系
RELATION_ENUM	SM_Relation_PointLine(const POINT& Point, const POINT& LP1, const POINT& LP2)
{
//	_ASSERT(LP1.x <= LP2.x);
	POINT	p1, p2;
	p1.x = LP1.x - Point.x;
	p1.y = LP1.y - Point.y;

	if (p1.x * p1.x <= TOGGLE_VALUE_0_1 && p1.y * p1.y <= TOGGLE_VALUE_0_1)	//fabs(p1.x) < 3.2 && fabs(p1.y) < 3.2
		return RELATION_ON;

	p2.x = LP2.x - Point.x;
	p2.y = LP2.y - Point.y;
	if (p2.x * p2.x <= TOGGLE_VALUE_0_1 && p2.y * p2.y <= TOGGLE_VALUE_0_1)
		return RELATION_ON;

	if (p1.x > p2.x)
	{
		POINT p = p1;
		p1 = p2;
		p2 = p;
	}

//	POINT pt;
//	pt.x = 0;
//	pt.y = 0;
	int /*x0,y0,*/x1,y1,x2,y2;
//	x0=pt.x; y0=pt.y;
	x1=p1.x; y1=p1.y; x2=p2.x; y2=p2.y;
//  直线方程：
//	y(x2-x1) - x(y2-y1) + (x1y2 - x2y1) = 0 
	int f = /*y0 * (x2 - x1) - x0 * (y2 - y1) +*/ (x1*y2 - x2*y1);
	if (f <= TOGGLE_VALUE_0_1 && f >= -TOGGLE_VALUE_0_1)
	{
		return RELATION_ON;
	}
	else if (f < 0)
		return RELATION_UP;
	else
		return RELATION_DOWN;
}

//点和直线的位置关系
RELATION_ENUM	SM_Relation_PointLine_New(const POINT& Point, const POINT& LP1, const POINT& LP2)
{
	POINT p1 = {
        LP1.x - Point.x,  
        LP1.y - Point.y
    };

	if (
        (abs(p1.x) <= TOGGLE_VALUE_0_1_SQRT) && 
        (abs(p1.y) <= TOGGLE_VALUE_0_1_SQRT)
    )
		return RELATION_ON;

	POINT p2 = {
        LP2.x - Point.x,
	    LP2.y - Point.y
    };

	if (
        (abs(p2.x) <= TOGGLE_VALUE_0_1_SQRT) && 
        (abs(p2.y) <= TOGGLE_VALUE_0_1_SQRT)
    )
		return RELATION_ON;

	if (p1.x > p2.x)
	{
		POINT p = p1;
		p1 = p2;
		p2 = p;
	}

	long f = (p1.x * p2.y - p2.x * p1.y);

	if (abs(f) <= TOGGLE_VALUE_0_1)
		return RELATION_ON;

	if (f < 0)
		return RELATION_UP;
    
    return RELATION_DOWN;
}

static KTimer gs_Timer;
static int gs_nTimerStartFlag = false;
static unsigned uOldFastCount = 0;
static unsigned uNewFastCount = 0;

////点和直线的位置关系
//RELATION_ENUM	SM_Relation_PointLine(const POINT& Point, const POINT& LP1, const POINT& LP2)
//{
//    //g_pRepresent->OutputText(12, Name, KRF_ZERO_END, nMpsX - 12 * g_StrLen(Name) / 4, nMpsY, dwColor, 0, m_Height + nHeightOff);
//    if (!gs_nTimerStartFlag)
//    {   
//        gs_Timer.Start();
//        gs_nTimerStartFlag = true;
//    }
//
//    DWORD dwTimeCount1 = gs_Timer.GetElapseFrequency();
//
//    RELATION_ENUM Result_old = SM_Relation_PointLine_Old(Point, LP1, LP2);
//
//    DWORD dwTimeCount2 = gs_Timer.GetElapseFrequency();
//
//    RELATION_ENUM Result_new = SM_Relation_PointLine_New(Point, LP1, LP2);
//
//    DWORD dwTimeCount3 = gs_Timer.GetElapseFrequency();
//    
//    if (Result_old != Result_new)
//    {
//        _ASSERT(false);
//        DebugBreak();
//    }
//
//    if ((dwTimeCount2 - dwTimeCount1) > (dwTimeCount3 - dwTimeCount2))
//    {
//        uNewFastCount++;
//    }
//    else
//    {
//        uOldFastCount++;
//    }
//
//    return Result_new;
//}


RELATION_ENUM SM_Relation_LineLine_CheckCut(const POINT& LP11, const POINT& LP12, const POINT& LP21, const POINT& LP22, POINT& POI)
{
	RELATION_ENUM eRelate;
	RELATION_ENUM nRPL1 = SM_Relation_PointLine(LP11, LP21, LP22);
	RELATION_ENUM nRPL2 = SM_Relation_PointLine(LP12, LP21, LP22);
	if (nRPL1 == nRPL2)
		eRelate = nRPL1;
	else
	{
		if (nRPL1 == RELATION_ON)
			eRelate = nRPL2;
		else if (nRPL2 == RELATION_ON)
			eRelate = nRPL1;
		else
		{
			eRelate = RELATION_CROSS;
			int x1,y1,x2,y2,x3,y3,x4,y4;
			POINT	offset = LP11;
			x1 = 0; y1 = 0;
			x2 = LP12.x - offset.x;
			y2 = LP12.y - offset.y;
			x3 = LP21.x - offset.x;
			y3 = LP21.y - offset.y;
			x4 = LP22.x - offset.x;
			y4 = LP22.y - offset.y;

			//	y(x2-x1) - x(y2-y1) + (x1y2 - x2y1) = 0 
			//	y(x4-x3) - x(y4-y3) + (x3y4 - x4y3) = 0 
			//	y = -( (x1*y2 - x2*y1)(y4 - y3) - (x3*y4 - x4*y3)(y2 - y1) ) /  ( (x2 - x1) * (y4 - y3) - (x4 - x3)(y2 - y1) );

			if (x2 == x1 && x4 == x3)
			{
				POI.x = x1;
				POI.y = LP12.y;
			}
			else if (x2 == x1)
			{
				POI.x = x1;
				POI.y = (y3*x4 - y4*x3) / (x4 - x3);
			}
			else if (x4 == x3)
			{
				POI.x = x3;
				POI.y = (x3 * y2) / x2;
			}
			else if (y2 == y1 && y4 == y3)
			{
				POI.y = y1;
				POI.x = LP12.x;
			}
			else if (y2 == y1)
			{
				POI.y = y1;
				POI.x = (x3*y4 - x4*y3) / (y4 - y3);
			}
			else if (y4 == y3)
			{
				POI.y = y3;
				POI.x = (y3 * x2) / y2;
			}
			else
			{
				POI.y = (- (x3*y4 - x4*y3) * y2)  /  ( y2 * (x4 - x3) - (y4 - y3) * x2 );
				POI.x = (- (x3*y4 - x4*y3) * x2)  /  ( y2 * (x4 - x3) - (y4 - y3) * x2 );
			}

			POI.x += offset.x;
			POI.y += offset.y;
		}
	}
	return eRelate;
}


void SM_GetPointFromFloatSting32(char* pString, float& x, float& y, float& z)
{
	x = 0;
	y = 0;
	z = 0;
	if (pString == NULL)
		return;

	int  i = 0;
	while (pString[i] != ',' && pString[i] != 0)
		i++;
	if (pString[i] == 0)
	{
		x = (float)(atof(pString) * 32);
		return;
	}
	pString[i] = 0;
	x = (float)(atof(pString));
	x *= 32;

	pString = pString + i + 1;
	i = 0;
	while (pString[i] != ',' && pString[i] != 0)
		i++;
	if (pString[i] == 0)
	{
		y = (float)(atof(pString) * 32);
		return;
	}
	pString[i] = 0;
	y = (float)(atof(pString) * 32);
	
	z = (float)(atof(pString + i + 1) * 32);
}

void ChangeFileName(char* pPathFile, char* pNewFileName)
{
	if (pPathFile && pNewFileName)
	{
		int i = strlen(pPathFile);
		while(i > 0 && pPathFile[i] != '\\')
			i--;
		if (pPathFile[i] == '\\')
			strcpy(pPathFile + i + 1, pNewFileName);
		else
			strcpy(pPathFile, pNewFileName);
	}
}