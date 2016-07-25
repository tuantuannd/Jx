//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPolyRelation.cpp
// Date:	2002.03.11
// Code:	Spe
// Desc:	Header File
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KEngine.h"
#include "KPolygon.h"
#include "KPolyRelation.h"

BOOL g_ColliSion(KPolygon Poly1, KPolygon Poly2)
{
	POINT	posTest;

	for (int i = 0; i < Poly1.GetVertexNumber(); i++)
	{
		Poly1.GetIndexVertex(i, &posTest);
		if (Poly2.IsPointInPolygon(posTest))
			return TRUE;
	}
	return FALSE;
}

int	g_SpaceRelation(KPolygon Poly1, KPolygon Poly2)
{
	int		nLeft, nRight;
	POINT	posLeft[2], posRight[2];

	nLeft = Poly1.GetLeftVertex();
	nRight = Poly1.GetRightVertex();
	Poly1.GetIndexVertex(nLeft, &posLeft[0]);
	Poly1.GetIndexVertex(nRight, &posRight[0]);
	
	nLeft = Poly2.GetLeftVertex();
	nRight = Poly2.GetRightVertex();
	Poly2.GetIndexVertex(nLeft, &posLeft[1]);
	Poly2.GetIndexVertex(nRight, &posRight[1]);

	//	以下情况没有遮挡关系
	if (posLeft[0].x > posRight[1].x		//	说明第一个多边形完全在第二个的右边
		|| posLeft[1].x > posRight[0].x)	//	说明第二个多边形完全在第一个的右边
		return 0;							//	返回两个多边形无关

	int y, i, j;
	POINT	p1, p2;

	if (posLeft[0].x < posLeft[1].x)		//	第一个多边形最左点比第二个更左
	{
		for (i = 0; i < Poly1.GetVertexNumber(); i++)
		{
			j = i + 1;
			if (j == Poly1.GetVertexNumber())
				j = 0;
			Poly1.GetIndexVertex(i, &p1);
			Poly1.GetIndexVertex(j, &p2);
			if (IntersectionX2Y(p1, p2, posLeft[1].x, &y))
			{
				if (y > posLeft[1].y)
					return 1;
				else
					return -1;
			}
		}
	}
	else
	{
		for (i = 0; i < Poly2.GetVertexNumber(); i++)
		{
			j = i + 1;
			if (j == Poly2.GetVertexNumber())
				j = 0;
			Poly2.GetIndexVertex(i, &p1);
			Poly2.GetIndexVertex(j, &p2);
			if (IntersectionX2Y(p1, p2, posLeft[0].x, &y))
			{
				if (y > posLeft[0].y)
					return -1;
				else
					return 1;
			}
		}
	}
	return 0;
}

BOOL IntersectionX2Y(POINT p1, POINT p2, int x, int *y)
{
	if ((p1.x < x && p2.x < x)
		|| (p1.x > x && p2.x > x))
		return FALSE;

	*y = p1.y + (p2.y - p1.y) * (x - p1.x) / (p2.x - p1.x);
	return TRUE;
}