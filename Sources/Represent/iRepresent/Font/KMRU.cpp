/*******************************************************************************
// FileName			:	KMRU.cpp
// FileAuthor		:	Wooy
// FileCreateDate	:	2001-9-12 17:43:32
// FileDescription	:	最近使用表类
// Revision Count	:
*******************************************************************************/
#include "KMRU.h"
#include "memory.h"
#include "malloc.h"

/*!**************************************************************************
// Purpose		: 构造函数
*****************************************************************************/
KMRU::KMRU()
{
	m_nCountInTurn = 0;
	m_nItemCount = 0;
	m_nMaxItemNum = 0;
	m_pItemTable = 0;
}

/*!**************************************************************************
// Purpose		: 析构函数
*****************************************************************************/
KMRU::~KMRU()
{
	Terminate();
}

/*!**************************************************************************
// Function		: KMRU::Init
// Purpose		: 初始化
// Return		: bool 成功否
// Argumant		: int nMaxItemNum 表中最多可容纳的项的数目
*****************************************************************************/
bool KMRU::Init(int nMaxItemNum)
{
	Terminate();
	if ((m_nMaxItemNum = nMaxItemNum) <= 0)
		return false;
	m_pItemTable = (KMruItem*)malloc(sizeof(KMruItem) *m_nMaxItemNum);
	memset(m_pItemTable, 0, sizeof(KMruItem) * m_nMaxItemNum);
	for( int i = 0; i < m_nMaxItemNum; i++)
	{
		m_pItemTable[i].Data = i;
	}
	return true;
}

/*!**************************************************************************
// Function		: KMRU::Terminate
// Purpose		: 结束，清除操作
*****************************************************************************/
void KMRU::Terminate()
{
	m_nCountInTurn = 0;
	m_nItemCount = 0;
	m_nMaxItemNum = 0;
	if (m_pItemTable)
	{
		free(m_pItemTable);
		m_pItemTable = 0;
	}
}

/*!**************************************************************************
// Function		: KMRU::GetItemData
// Purpose		: 获得表中指定项关联的数据
// Return		: unsigned short 表中指定项关联的数据，如果指定的项不存在返回0
// Argumant		: int nIndex 	 指定项的索引
*****************************************************************************/
unsigned short KMRU::GetItemData(int nIndex)
{
	if (nIndex >= m_nItemCount || nIndex < 0)
		return 0;
	return m_pItemTable[nIndex].Data;
}

/*!***************************************************************************
// Function		: KMRU::Update
// Purpose		: 执行一轮结束时MRU表的信息更新。
*****************************************************************************/
void KMRU::Update()
{
	int		i;
	for(i = 0; i < m_nItemCount; i++)
	{
		if (m_pItemTable[i].MRURecord != 0xff)
			m_pItemTable[i].MRURecord ++;
	}
}

/*!***************************************************************************
// Function		: KMRU::CommitText
// Purpose		: 提交字符串，得到串中全部字符在贴图中的位置
// Argumant		: unsigned char  *pszString  提交的字符串
// Argumant		: int nCount	             提交字符的个数
// Argumant		: unsigned short *pPositions 指向用于存储各字符在贴图中的位置的缓冲区
// Argumant		: unsigned short *pNewItems  指向用于存储新进入表中的项的缓冲区
// Argumant		: int& nNumNewItem			 新新进入表中的项的数目
*****************************************************************************/
void KMRU::Commit(unsigned short *pString, int nCount, unsigned short *pPositions,
					  unsigned short *pNewItems, int& nNumNewItem)
{
	if (pString == 0 || pPositions == 0 || pNewItems == 0 || nCount <= 0)
		return;

	unsigned short	nCode, i;
	unsigned int	nInsertSpace, nDueSpace;

	nNumNewItem = 0;

	//检查是否开始新的一轮？
	if (m_nCountInTurn + nCount > m_nMaxItemNum)
	{
		m_nCountInTurn = 0;
		Update();
//		if (nCount > m_nMaxItemNum)
//			nCount = m_nMaxItemNum;
	}
	m_nCountInTurn += nCount;

	//逐个处理串中的字符
	for (i = 0; i < nCount; i++)
	{
		//取得字符的编码
		nCode = *pString++;
		if (!GetCharPos(nCode, 0, m_nItemCount, nDueSpace))
		{	//字符没有在表中
			if (m_nItemCount < m_nMaxItemNum)	//看表中是否还有空的项
				nInsertSpace = m_nItemCount++;
			else
				nInsertSpace = GetMostLongestUnusedItem();

			pNewItems[nNumNewItem * 2] = pPositions[i] = m_pItemTable[nInsertSpace].Data;
			pNewItems[nNumNewItem * 2 + 1] = m_pItemTable[nInsertSpace].Id = nCode;
			m_pItemTable[nInsertSpace].MRURecord = 0;

			//调整新加入项在表中的位置
			if (nInsertSpace != nDueSpace)
				AdjustCharPos(nInsertSpace, nDueSpace);
			
			nNumNewItem ++;
		}
		else
		{
			pPositions[i] = m_pItemTable[nDueSpace].Data;
			m_pItemTable[nDueSpace].MRURecord = 0;
		}
	}	
}


/*!**************************************************************************
// Function		: KMRU::AdjustCharPos
// Purpose		: 调整新加入项在表中的位置，使表中的项保持以Id排序。
// Argumant		: unsigned int nIndex    要调整的项的当前位置
// Argumant		: unsigned int nDueIndex 要调整的项的目的位置
*****************************************************************************/
void KMRU::AdjustCharPos(unsigned int nIndex, unsigned int nDueIndex)
{
	KMruItem		Temp;

	Temp = m_pItemTable[nIndex];
	if (nDueIndex < nIndex)
	{	//应该把项往前移
		while(nIndex > nDueIndex)
		{
			m_pItemTable[nIndex] = m_pItemTable[nIndex - 1];
			nIndex --;
		}
		m_pItemTable[nDueIndex] = Temp;
	}
	else if (nDueIndex > nIndex + 1)
	{	//应该把项往后移
		nDueIndex --;
		while (nIndex  < nDueIndex)
		{
			m_pItemTable[nIndex] = m_pItemTable[nIndex + 1];
			nIndex ++;
		}
		m_pItemTable[nDueIndex] = Temp;
	}
}

/*!**************************************************************************
// Function		: KMRU::GetCharPos
// Purpose		: 在给范围内定位一个字符（没找到，则给出如需插入应放置的位置）
// Return		: bool 是否找到
// Argumant		: unsigned short Id	       要查找的项的Id(字符的编码)
// Argumant		: int nFrom	               范围上限
// Argumant		: int nTo		           入范围下限
// Argumant		: unsigned int& nPosition  位置
*****************************************************************************/
bool KMRU::GetCharPos(unsigned short Id, int nFrom, int nCount, unsigned int& nPosition)
{
	unsigned int	nMiddle;
	int		nTo = nFrom + nCount - 1;
	//要求nFrom一定要小于等于nTo，且都必须小于m_nItemCount
	//因为此函数是接口的内部函数，省去对非法参数的判断
	
	if (!nCount)
	{
		nPosition = nFrom;
		return false;
	}
	//以二分法查找
	while(nFrom < nTo)
	{
		nMiddle = (nFrom + nTo) >> 1;   // div 2
		if (Id < m_pItemTable[nMiddle].Id)
		{
			nTo = nMiddle - 1;
			continue;
		}
		if (Id > m_pItemTable[nMiddle].Id)
		{
			nFrom = nMiddle + 1;
			continue;
		}
		nPosition = nMiddle;
		return true;
	};
	if (Id == m_pItemTable[nFrom].Id)
	{
		nPosition = nFrom;
		return true;
	}
	if (Id < m_pItemTable[nFrom].Id)
		nPosition = nFrom;
	else
		nPosition = nTo + 1;
	return false;
}

/*!*****************************************************************************
// Function		: KMRU::GetMostEarlyUsedItem
// Purpose		: 在表中找最久没有被使用项
// Return		: unsigned int 找到的项的索引
*****************************************************************************/
unsigned int KMRU::GetMostLongestUnusedItem()
{
	int	i, nMEU;
	nMEU = 0;
	for (i = 1; i < m_nItemCount; i++)
	{
		if (m_pItemTable[i].MRURecord > m_pItemTable[nMEU].MRURecord)
			nMEU = i;
	}
	return nMEU;
}
