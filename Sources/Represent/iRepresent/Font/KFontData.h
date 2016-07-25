/*******************************************************************************
// FileName			:	KFontData.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2002-9-4
// FileDescription	:	字体图形数据类头文件
// Revision Count	:	
*******************************************************************************/
#pragma once

//#define KFR_MAX_NUM_CHAR	23940		// For GBK (0xFE-0x80)*(0xFE-0x3F-1)=23940
/*#ifdef CHARASET_GB2132
	#define KFR_MAX_NUM_CHAR	94 * 94		// for GB2132 = 35 KB
#endif
#ifdef CHARASET_BIG5
	#define KFR_MAX_NUM_CHAR	94 * 190	// for BIG5 = 70 KB
#endif*/

//字库文件文件头结构
typedef struct _KFontHead
{
	char			Id[4];		// 标识
	unsigned int	Size;		// 大小
	unsigned int	Count;		// 数量
	unsigned short	Width;		// 宽度
	unsigned short	Height;		// 高度
}KFontHead;

//字体图形资源类
class KFontData
{
public:
	//载入字库文件
	bool			Load(const char* pszFontFile);
	//结束，清除操作
	void			Terminate();
	//得到字体的宽度高度
	void			GetInfo(int& nWidth, int& nHeight) const;
	//取得单个字符的数据区
	unsigned char*	GetCharacterData(unsigned char cFirst, unsigned char cNext) const;
	
	KFontData();
	~KFontData();

protected:
	unsigned int*	m_pdwOffs;					//偏移表
	unsigned char*	m_pFontData;				//字符点阵信息数据
	unsigned int	m_dwDataSize;				//字符点阵信息数据存储的大小
	unsigned int	m_uCharCount;				//现有字符个数
	int				m_nFontW;					//字符宽
	int				m_nFontH;					//字符高
};

