#include "KCore.h"
#include "KItem.h"
#include "MyAssert.H"
#include "KInventory.h"

KInventory::KInventory()
{
	m_pArray = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
}

KInventory::~KInventory()
{
    Release();
}

BOOL KInventory::Init(int nWidth, int nHeight)
{
	if (m_pArray)
	{
		delete [] m_pArray;
		m_pArray = NULL;
	}
	m_pArray = new int[nWidth * nHeight];

	if (!m_pArray)
		return FALSE;

	ZeroMemory(m_pArray, sizeof(int) * nWidth * nHeight);
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	return TRUE;
}

void KInventory::Release()
{
	if (m_pArray)
	{
		delete []m_pArray;
	}
	m_pArray = NULL;
	this->m_nWidth = 0;
	this->m_nHeight = 0;
	this->m_nMoney = 0;
}

void KInventory::Clear()
{
	if (m_pArray)
		memset(m_pArray, 0, sizeof(int) * m_nWidth * m_nHeight);
	m_nMoney = 0;
}

#pragma optimize( "y", off)

BOOL KInventory::PlaceItem(int nX, int nY, int nIdx, int nWidth, int nHeight)
{
	if (!m_pArray)
		return FALSE;
	if (nX < 0 || nY < 0 || nWidth < 1 || nHeight < 1 || nX + nWidth > m_nWidth || nY + nHeight > m_nHeight)
		return FALSE;
	if (nIdx <= 0)
	{
		_ASSERT(0);
		return FALSE;
	}

	int i, j;
	int nOldIdx = 0;

	for (i = nX; i < nX + nWidth; i++)
	{
		for (j = nY; j < nY + nHeight; j++)
		{
			if (!nOldIdx)
				nOldIdx = m_pArray[j * m_nWidth + i];
			// 位置上有东西
			if (nOldIdx)
			{
				// 位置上有多个东西，无法放置
				if (nOldIdx != m_pArray[j * m_nWidth + i])
					return FALSE;
/*#pragma	message(ATTENTION("需要在道具系统完成后去检查是否可叠放"))
				// 检查是否为可叠放物品
				if (Item[nIdx].CanStack(nOldIdx) && i == nX && j == nY)
				{
					return Item[nOldIdx].Stack(nIdx);
				} // 检查是否是容器
				else if (Item[nOldIdx].IsReceptacle())
				{
					return Item[nOldIdx].HoldItem(nIdx);
				}
				*/
				return FALSE;
			}
		}
	}
	// Set Item Idx to Inventory
	for (i = nX; i < nX + nWidth; i++)
	{
		for (j = nY; j < nY + nHeight; j++)
		{
			m_pArray[j * m_nWidth + i] = nIdx;
		}
	}
#ifdef _DEBUG
//#define	_DEBUG_ITEM	1
#ifdef _DEBUG_ITEM
	for (int kk = 0; kk < 10; kk++)
	{
		g_DebugLog("%d%d%d%d%d%d", 
			m_pArray[kk * m_nWidth],
			m_pArray[kk * m_nWidth + 1],
			m_pArray[kk * m_nWidth + 2],
			m_pArray[kk * m_nWidth + 3],
			m_pArray[kk * m_nWidth + 4],
			m_pArray[kk * m_nWidth + 5]);
	}
#endif
#endif
	return TRUE;
}

#pragma optimize( "", on)

BOOL KInventory::HoldItem(int nIdx, int nWidth, int nHeight)
{
	int i, j;
	for (i = 0; i < m_nWidth - nWidth + 1; i++)
	{
		for (j = 0; j < m_nHeight - nHeight + 1; j++)
		{
			if (PlaceItem(i, j, nIdx, nWidth, nHeight))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL	KInventory::PickUpItem(int nIdx, int nX, int nY, int nWidth, int nHeight)
{
	if (nX < 0 || nY < 0 || nWidth < 1  || nHeight < 1 || nX + nWidth > this->m_nWidth || nY + nHeight > this->m_nHeight)
		return FALSE;

	int		i;
	for (i = nX; i < nX + nWidth; i++)
	{
		for (int j = nY; j < nY + nHeight; j++)
		{
			if (m_pArray[j * m_nWidth + i] != nIdx)
			{
				_ASSERT(0);
				return FALSE;
			}
		}
	}

	for (i = nX; i < nX + nWidth; i++)
	{
		for (int j = nY; j < nY + nHeight; j++)
		{
			m_pArray[j * m_nWidth + i] = 0;
		}
	}

	return TRUE;
}

int		KInventory::FindItem(int nX, int nY)
{
	if (!m_pArray)
		return -1;
	if (nX < 0 || nX >= this->m_nWidth || nY < 0 || nY >= this->m_nHeight)
		return -1;

	int		nPos = nY * m_nWidth + nX;
	int		*pArray = &m_pArray[nPos];
	if (*pArray <= 0)
		return 0;
	int		nIdx = *pArray;

	if ((nPos > 0 && *(pArray - 1) == nIdx) || (nPos >= m_nWidth && *(pArray - m_nWidth) == nIdx))
		return -1;

	return nIdx;

/*
	for (int i = 0; i < nY * m_nWidth + nX; i++)
	{
		if (m_pArray[i] == nIdx)
			return -1;
	}

	return nIdx;
*/
}

BOOL KInventory::FindRoom(int nWidth, int nHeight, POINT* pPos)
{
	if (!pPos)
		return FALSE;
	if (nWidth < 1 || nWidth > m_nWidth || nHeight < 1 || nHeight > m_nHeight)
		return FALSE;

	int i, j;
	for (i = 0; i < m_nWidth - nWidth + 1; i++)
	{
		for (j = 0; j < m_nHeight - nHeight + 1; j++)
		{
			if (CheckRoom(i, j, nWidth, nHeight))
			{
				pPos->x = i;
				pPos->y = j;
				return TRUE;
			}
		}
	}
	pPos->x = 0;
	pPos->y = 0;
	return FALSE;
}

BOOL KInventory::CheckRoom(int nX, int nY, int nWidth, int nHeight)
{
	if (!m_pArray)
	{
		_ASSERT(0);
		return FALSE;
	}

	if (nX < 0 || nY < 0 || nWidth < 1 || nHeight < 1 || nX + nWidth > m_nWidth || nY + nHeight > m_nHeight)
		return FALSE;

	int i, j;
	int nOldIdx = 0;

	for (i = nX; i < nX + nWidth; i++)
	{
		for (j = nY; j < nY + nHeight; j++)
		{
			if (m_pArray[j * m_nWidth + i])
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL KInventory::AddMoney(int nMoney)
{
	if (m_nMoney + nMoney < 0)
		return FALSE;
	m_nMoney += nMoney;
	return TRUE;
}

int		KInventory::GetNextItem(int nStartIdx, int nXpos, int nYpos, int *pX, int *pY)
{
	if (!m_pArray)
		return 0;
	if (nXpos < 0 || nYpos < 0 || nXpos >= m_nWidth || nYpos >= m_nHeight || !pX || !pY)
		return 0;
	int		nSize = m_nWidth * m_nHeight;
	int		i = nYpos * m_nWidth + nXpos;
	int		*pArray = &m_pArray[i];
	for ( ; i < nSize; i++, pArray++)
	{
		if (*pArray <= 0)
			continue;
		if (*pArray == nStartIdx)
			continue;
		if (i < m_nWidth || pArray[-m_nWidth] != *pArray)
		{
			*pX = i % m_nWidth;
			*pY = i / m_nWidth;
			return *pArray;
		}
	}
	return 0;
}

int		KInventory::CalcSameDetailType(int nGenre, int nDetail)
{
	if (!m_pArray)
		return 0;
	int		nNum = 0;
	int		nCurIdx = 0;
	int		nSize = m_nWidth * m_nHeight;
	int		*pArray = m_pArray;
	for (int i = 0; i < nSize; i++)
	{
		if (*pArray <= 0)
		{
			pArray++;
			continue;
		}
		if (nCurIdx == *pArray)
		{
			pArray++;
			continue;
		}
		if (i < m_nWidth || pArray[-m_nWidth] != *pArray)
		{
			nCurIdx = *pArray;
			if (Item[nCurIdx].GetGenre() == nGenre && Item[nCurIdx].GetDetailType() == nDetail)
				nNum++;
		}
		pArray++;
	}

	return nNum;
}

BOOL	KInventory::FindEmptyPlace(int nWidth, int nHeight, POINT *pPos)
{
	if (!m_pArray)
		return FALSE;
	if (!pPos || nWidth <= 0 || nHeight <= 0 || nWidth > m_nWidth || nHeight > m_nHeight)
		return FALSE;

	int		i, j, nIdx, a, b, nFind;
	for (i = 0; i < m_nHeight - nHeight + 1; i++)
	{
		for (j = 0; j < m_nWidth - nWidth + 1; )
		{
			nIdx = m_pArray[i * m_nWidth + j];
			if (nIdx)
			{
				_ASSERT(Item[nIdx].GetWidth() > 0);
				j += Item[nIdx].GetWidth();
			}
			else
			{
				nFind = 1;
				for (a = i; a < i + nHeight; a++)
				{
					for (b = j; b < j + nWidth; b++)
					{
						if (m_pArray[a * m_nWidth + b])
						{
							nFind = 0;
							break;
						}
					}
					if (nFind == 0)
						break;
				}
				if (nFind)
				{
					pPos->x = a;
					pPos->y = b;
					return TRUE;
				}
				j++;
			}
		}
	}

	return FALSE;
}
//---------------------------------------------------------------------------------
// 功能：输入物品类型和具体类型，察看Inventory里面有没有相同的物品，输出位置和编号
//---------------------------------------------------------------------------------
BOOL	KInventory::FindSameDetailType(int nGenre, int nDetail, int *pnIdx, int *pnX, int *pnY)
{
	if (!m_pArray)
		return FALSE;
	if (!pnIdx || !pnX || !pnY)
		return FALSE;

	int		*pArray = m_pArray;
	int		i, nSize = m_nWidth * m_nHeight;

	for (i = 0; i < nSize; i++, pArray++)
	{
		if (*pArray <= 0)
			continue;
		if (Item[*pArray].GetGenre() == nGenre && Item[*pArray].GetDetailType() == nDetail)
		{
			*pnIdx = *pArray;
			*pnX = i % m_nWidth;
			*pnY = i / m_nWidth;
			return TRUE;
		}
	}

	return FALSE;

/*
	int j;

	for (i = 0; i < this->m_nHeight; i++)
	{
		for (j = 0; j < this->m_nWidth; j++)
		{
			if (!m_pArray[i * m_nWidth + j])
				continue;
			if (Item[m_pArray[i * m_nWidth + j]].GetGenre() != nGenre)
				continue;
			if (Item[m_pArray[i * m_nWidth + j]].GetDetailType() != nDetail)
				continue;
			*pnIdx = m_pArray[i * m_nWidth + j];
			*pnX = j;
			*pnY = i;
			return TRUE;
		}
	}

	return FALSE;
*/
}

//---------------------------------------------------------------------------------
// 功能：输入物品类型和具体类型，察看Inventory里面有没有相同的物品
//---------------------------------------------------------------------------------
BOOL	KInventory::CheckSameDetailType(int nGenre, int nDetail)
{
	if (!m_pArray)
		return FALSE;

	int		*pArray = m_pArray;
	int		i, nSize = m_nWidth * m_nHeight;

	for (i = 0; i < nSize; i++, pArray++)
	{
		if (*pArray <= 0)
			continue;
		if (Item[*pArray].GetGenre() == nGenre && Item[*pArray].GetDetailType() == nDetail)
			return TRUE;
	}

	return FALSE;
}
