/*******************************************************************************
// FileName			:	KMRU.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2001-9-12 17:42:49
// FileDescription	:	最近使用表类
// Revision Count	:
*******************************************************************************/
#pragma once

//MRU表的项
#pragma pack(1)
typedef struct _KMruItem
{
	unsigned short	Id;			//项id
	unsigned short	Data;		//项数据
	unsigned char	MRURecord;	//使用纪录
	unsigned char	Reserved;	//保留，未用到
    unsigned short  Reserved1;
}KMruItem;
#pragma pack()

class KMRU  
{
public:
	//提交字符串，得到串中全部字符在贴图中的位置
	void Commit(unsigned short* pString, int nCount, unsigned short* pPositions, unsigned short* pNewItems, int& nNumNewItem);
	void Terminate();				//结束，清除操作
	bool Init(int nMaxItemNum);		//初始化
	
	KMRU();
	virtual ~KMRU();

private:
	//调整新加入项在表中的位置，使表中的项保持以Id排序。
	void AdjustCharPos(unsigned int nIndex,  unsigned int nDueIndex);
	void Update();								//执行一轮结束时MRU表的信息更新
	unsigned int GetMostLongestUnusedItem();	//在表中找最久没有被使用项
	unsigned short GetItemData(int nIndex);		//获得表中指定项关联的数据
	//在给范围内定位一个字符（没找到，则给出如需插入应放置的位置）
	bool GetCharPos(unsigned short Id, int nFrom, int nCount, unsigned int& nPosition);
private:
	KMruItem*	m_pItemTable;		//项表
	int			m_nMaxItemNum;		//表中最多可容纳的项的数目
	int			m_nItemCount;		//当前表中包含的项的数目
	int			m_nCountInTurn;		//此轮已引用的不同项的数目
};
