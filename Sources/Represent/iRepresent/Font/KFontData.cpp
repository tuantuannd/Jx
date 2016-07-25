/*******************************************************************************
// FileName			:	KFontData.h
// FileAuthor		:	Wooy
// FileCreateDate	:	2002-9-4
// FileDescription	:	字体图形数据类头文件
// Revision Count	:	
*******************************************************************************/
#include "KFontData.h"
#include "../../../Engine/Src/KWin32.h"
#include "../../../Engine/Src/KPakFile.h"

/*!*****************************************************************************
// Purpose		:  构造函数
*****************************************************************************/
KFontData::KFontData()
{
	m_pdwOffs = NULL;
	m_pFontData = NULL;
	m_dwDataSize = 0;
	m_nFontW = 0;
	m_nFontH = 0;
}

/*!*****************************************************************************
// Purpose		:  析构函数
*****************************************************************************/
KFontData::~KFontData()
{
	Terminate();
}

/*!*****************************************************************************
// Function		: KFontData::Init
// Purpose		: 载入字库文件
// Return		: 是否成功
// Argumant		: const char* pszFontFile      -> 字库文件名
*****************************************************************************/
bool KFontData::Load(const char* pszFontFile)
{
	Terminate();

	bool bOk = false;
	while(pszFontFile)
	{
		KPakFile	File;
		KFontHead	Header;

		//打开字库文件
		if (File.Open((char*)pszFontFile) == FALSE)
			break;
		//读字库文件头结构
		if (File.Read(&Header, sizeof(Header)) != sizeof(Header))
			break;

		//检查字库文件ID
		if (*((int*)(&Header.Id)) != 0x465341 ||	//"ASF"
			Header.Count <= 0 || Header.Size <= 0)
		{
			break;
		}

		//分配字符点阵信息数据的存储空间
		m_pdwOffs = (unsigned int*)malloc(sizeof(unsigned int) * Header.Count);
		m_pFontData = (unsigned char*)malloc(Header.Size);
		if (m_pdwOffs == NULL || m_pFontData == NULL)
			break;

		//得到字体信息
		m_dwDataSize = Header.Size;
		m_uCharCount = Header.Count;
		m_nFontW = Header.Width;
		m_nFontH = Header.Height;

		//读取偏移表
		if (File.Read(m_pdwOffs, m_uCharCount * sizeof(unsigned int)) != m_uCharCount * sizeof(unsigned int))
			break;

		//读取字库点阵数据
		if (File.Read(m_pFontData, m_dwDataSize) == m_dwDataSize)
			bOk = true;

		//关闭字库文件
		File.Close();
		break;
	};


	if (bOk == false)
		Terminate();
	return bOk;
}

/*!*****************************************************************************
// Function		: KFontData::GetInfo
// Purpose		: 得到字体的宽度高度，贴图可以缓存的字符的个数
// Argumant		: int &nWidth	字符宽
// Argumant		: int &nHeight	字符高
*****************************************************************************/
void KFontData::GetInfo(int &nWidth, int &nHeight) const
{
	nWidth  = m_nFontW;
	nHeight = m_nFontH;
}

/*!*****************************************************************************
// Function		: KFontData::Terminate
// Purpose		: 结束，清除操作
*****************************************************************************/
void KFontData::Terminate()
{
	if (m_pFontData)
	{
		free(m_pFontData);
		m_pFontData = NULL;
	}
	if (m_pdwOffs)
	{
		free (m_pdwOffs);
		m_pdwOffs = NULL;
	}
	m_uCharCount = 0;
	m_dwDataSize = 0;
}

/*!*****************************************************************************
// Function		: KFontData::GetCharacterData
// Purpose		: 取得单个字符的数据区
*****************************************************************************/
unsigned char*	KFontData::GetCharacterData(unsigned char cFirst, unsigned char cNext) const
{
	if (m_pFontData)
	{
		//字符在字库里的索引	//GBK编码方式的索引运算
		unsigned int	uCharIndex = (cFirst - 0x81) * 190 + (cNext - 0x40) - (cNext >> 7);
		if (uCharIndex < m_uCharCount && m_pdwOffs[uCharIndex])
			return (m_pFontData + m_pdwOffs[uCharIndex]);
	}
	return NULL;
}