//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KPolygon.cpp
// Date:	2002.01.10
// Code:	Spe
// Desc:	Polygon Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KPolygon.h"
#include <string.h>
//////////////////////////////
// Construction/Destruction	//
//////////////////////////////

KPolygon::KPolygon()
{
	m_Polygon.nVertex = 0;
	ZeroMemory(m_Polygon.Pos, sizeof(POINT) * defMaxVertex);
}

BOOL KPolygon::IsPointInPolygon(POINT pos)
{
	static POINT	pPos[defMaxVertex+1];
	BOOL			bRet = TRUE;
	int				nFlag[2];
	int				nResult;
	register int	i;

	for (i = 0; i < m_Polygon.nVertex; i++)
	{
		pPos[i].x = m_Polygon.Pos[i].x - pos.x;
		pPos[i].y = m_Polygon.Pos[i].y - pos.y;
	}
	pPos[m_Polygon.nVertex] = pPos[0];
	nFlag[1] = 0;
	for (i = 0; i < m_Polygon.nVertex; i++)
	{
		nResult = pPos[i].x * pPos[i + 1].y - pPos[i + 1].x * pPos[i].y;
		nFlag[0]=(nResult>0)?1:((nResult<0)?-1:0);
		if (nFlag[0])
		{
			if (!nFlag[1])
			{
				nFlag[1] = nFlag[0];
			}
			if (nFlag[0] != nFlag[1])
			{
				bRet = FALSE;
				break;
			}
		}
	}
	return bRet;
}

BOOL KPolygon::IsPointInPolygon(int x, int y)
{
	static POINT	pPos[defMaxVertex+1];
	BOOL			bRet = TRUE;
	int				nFlag[2];
	int				nResult;
	register int	i;
	
	for (i = 0; i < m_Polygon.nVertex; i++)
	{
		pPos[i].x = m_Polygon.Pos[i].x - x;
		pPos[i].y = m_Polygon.Pos[i].y - y;
	}
	pPos[m_Polygon.nVertex] = pPos[0];
	nFlag[1] = 0;
	for (i = 0; i < m_Polygon.nVertex; i++)
	{
		nResult = pPos[i].x * pPos[i + 1].y - pPos[i + 1].x * pPos[i].y;
		nFlag[0]=(nResult>0)?1:((nResult<0)?-1:0);
		if (nFlag[0])
		{
			if (!nFlag[1])
			{
				nFlag[1] = nFlag[0];
			}
			if (nFlag[0] != nFlag[1])
			{
				bRet = FALSE;
				break;
			}
		}
	}
	return bRet;
}


int KPolygon::GetNearVertex(POINT pos)
{
	int i;
	int	nMin = 0x7fffffff;
	int	nIndex;

	for (i = 0; i < m_Polygon.nVertex; i++)
	{
		if ((m_Polygon.Pos[i].x - pos.x) * (m_Polygon.Pos[i].x - pos.x) + (m_Polygon.Pos[i].y - pos.y) * (m_Polygon.Pos[i].y - pos.y) < nMin)
		{
			nIndex = i;
			nMin = (m_Polygon.Pos[i].x - pos.x) * (m_Polygon.Pos[i].x - pos.x) + (m_Polygon.Pos[i].y - pos.y) * (m_Polygon.Pos[i].y - pos.y);
		}
	}
	return nIndex;
}

BOOL KPolygon::GetIndexVertex(int i, POINT* vertex)
{
	if (i >= m_Polygon.nVertex)
		return FALSE;
	vertex->x = m_Polygon.Pos[i].x;
	vertex->y = m_Polygon.Pos[i].y;
	return TRUE;
}

BOOL KPolygon::RemoveIndexVertex(int index)
{
	if (m_Polygon.nVertex < 4)
		return FALSE;

	m_Polygon.nVertex --;
	for (int i = index; i < defMaxVertex - 1; i++)
	{
		m_Polygon.Pos[i] = m_Polygon.Pos[i+1];
	}
	return TRUE;
}

BOOL KPolygon::AddPointToVertex(POINT pos)
{
	int nPrevIndex, nNextIndex;

	if (m_Polygon.nVertex < 3)
	{
		m_Polygon.Pos[m_Polygon.nVertex] = pos;
		m_Polygon.nVertex++;
		return TRUE;
	}

	if (m_Polygon.nVertex >= defMaxVertex)
		return FALSE;
//-------------------------------------------------------------------
	// Add Point to Polygon Last point First
	POINT	pVector[2];
	int		nResult, nFlag[2];

	m_Polygon.Pos[m_Polygon.nVertex] = pos;
	nFlag[1] = 0;
	for (int i = 0; i < m_Polygon.nVertex + 1; i++)
	{
		nPrevIndex = i - 1;
		if (nPrevIndex < 0)
			nPrevIndex = m_Polygon.nVertex;
		nNextIndex = i + 1;
		if (nNextIndex > m_Polygon.nVertex)
			nNextIndex = 0;
		// Get Vector of point to next point
		pVector[0].x = m_Polygon.Pos[nNextIndex].x - m_Polygon.Pos[i].x;
		pVector[0].y = m_Polygon.Pos[nNextIndex].y - m_Polygon.Pos[i].y;
		// Get Vector of point to previous point
		pVector[1].x = m_Polygon.Pos[nPrevIndex].x - m_Polygon.Pos[i].x;
		pVector[1].y = m_Polygon.Pos[nPrevIndex].y - m_Polygon.Pos[i].y;

		nResult = pVector[0].x * pVector[1].y - pVector[1].x * pVector[0].y;
		nFlag[0] = (nResult > 0)?1:((nResult < 0)?-1:0);
		if (nFlag[0])
		{
			if (!nFlag[1])
			{
				nFlag[1] = nFlag[0];
			}
			if (nFlag[0] != nFlag[1])
			{
				m_Polygon.Pos[m_Polygon.nVertex].x = 0;
				m_Polygon.Pos[m_Polygon.nVertex].y = 0;
				return FALSE;
			}
		}
	}
//-------------------------------------------------------------------
	m_Polygon.nVertex++;
	return TRUE;
}

BOOL KPolygon::AddPointToVertex(int x, int y)
{
	int nPrevIndex, nNextIndex;
	
	if (m_Polygon.nVertex < 3)
	{
		m_Polygon.Pos[m_Polygon.nVertex].x = x;
		m_Polygon.Pos[m_Polygon.nVertex].y = y;
		m_Polygon.nVertex++;
		return TRUE;
	}
	
	if (m_Polygon.nVertex >= defMaxVertex)
		return FALSE;
	//-------------------------------------------------------------------
	// Add Point to Polygon Last point First
	POINT	pVector[2];
	int		nResult, nFlag[2];
	
	m_Polygon.Pos[m_Polygon.nVertex].x = x;
	m_Polygon.Pos[m_Polygon.nVertex].y = y;
	nFlag[1] = 0;
	for (int i = 0; i < m_Polygon.nVertex + 1; i++)
	{
		nPrevIndex = i - 1;
		if (nPrevIndex < 0)
			nPrevIndex = m_Polygon.nVertex;
		nNextIndex = i + 1;
		if (nNextIndex > m_Polygon.nVertex)
			nNextIndex = 0;
		// Get Vector of point to next point
		pVector[0].x = m_Polygon.Pos[nNextIndex].x - m_Polygon.Pos[i].x;
		pVector[0].y = m_Polygon.Pos[nNextIndex].y - m_Polygon.Pos[i].y;
		// Get Vector of point to previous point
		pVector[1].x = m_Polygon.Pos[nPrevIndex].x - m_Polygon.Pos[i].x;
		pVector[1].y = m_Polygon.Pos[nPrevIndex].y - m_Polygon.Pos[i].y;
		
		nResult = pVector[0].x * pVector[1].y - pVector[1].x * pVector[0].y;
		nFlag[0] = (nResult > 0)?1:((nResult < 0)?-1:0);
		if (nFlag[0])
		{
			if (!nFlag[1])
			{
				nFlag[1] = nFlag[0];
			}
			if (nFlag[0] != nFlag[1])
			{
				m_Polygon.Pos[m_Polygon.nVertex].x = 0;
				m_Polygon.Pos[m_Polygon.nVertex].y = 0;
				return FALSE;
			}
		}
	}
	//-------------------------------------------------------------------
	m_Polygon.nVertex++;
	return TRUE;
}


void KPolygon::LoopVertex(int nTurn)
{
	if (nTurn > m_Polygon.nVertex)
		return;
	if (nTurn < 0)
		nTurn = m_Polygon.nVertex + nTurn;
	int	nNext;
	POINT	BackPos[defMaxVertex];
	memcpy(BackPos, m_Polygon.Pos, sizeof(POINT) * defMaxVertex);
	for (int j = 0; j < m_Polygon.nVertex; j++)
	{
		nNext = j + nTurn;
		if (nNext >= m_Polygon.nVertex)
			nNext -= m_Polygon.nVertex;
		m_Polygon.Pos[j] = BackPos[nNext];
	}
}

void KPolygon::Clear()
{
	m_Polygon.nVertex = 0;
	ZeroMemory(m_Polygon.Pos, sizeof(POINT) * defMaxVertex);	
}

void KPolygon::GetCenterPos(POINT *pos)
{
	pos->x = 0;
	pos->y = 0;
	for (int i = 0; i < m_Polygon.nVertex; i++)
	{
		pos->x += m_Polygon.Pos[i].x;
		pos->y += m_Polygon.Pos[i].y;
	}
	pos->x /= m_Polygon.nVertex;
	pos->y /= m_Polygon.nVertex;
}

BOOL KPolygon::ShiftVertex(int nDir, int nDistance)
{
	int	i;

	switch(nDir)
	{
	case 0:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].y += nDistance;
		}
		break;
	case 1:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].x -= nDistance;
			m_Polygon.Pos[i].y += nDistance;
		}
		break;
	case 2:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].x -= nDistance;
		}
		break;
	case 3:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].x -= nDistance;
			m_Polygon.Pos[i].y -= nDistance;
		}
		break;
	case 4:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].y -= nDistance;
		}
		break;
	case 5:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].x += nDistance;
			m_Polygon.Pos[i].y -= nDistance;
		}
		break;
	case 6:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].x += nDistance;
		}
		break;
	case 7:
		for (i = 0; i < m_Polygon.nVertex; i++)
		{
			m_Polygon.Pos[i].x += nDistance;
			m_Polygon.Pos[i].y += nDistance;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

int KPolygon::GetLeftVertex()
{
	int		nLeft = m_Polygon.Pos[0].x;
	int		nIdx = 0;

	for (int i = 1; i < m_Polygon.nVertex; i++)
	{
		if (m_Polygon.Pos[i].x < nLeft)
		{
			nLeft = m_Polygon.Pos[i].x;
			nIdx = i;
		}
	}
	return nIdx;
}

int KPolygon::GetRightVertex()
{
	int		nRight = m_Polygon.Pos[0].x;
	int		nIdx = 0;
	
	for (int i = 1; i < m_Polygon.nVertex; i++)
	{
		if (m_Polygon.Pos[i].x > nRight)
		{
			nRight = m_Polygon.Pos[i].x;
			nIdx = i;
		}
	}
	return nIdx;
}
