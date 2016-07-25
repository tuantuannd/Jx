//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KIniFile.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Ini File Operation Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFile.h"
#include "KFilePath.h"
#include "KPakFile.h"
#ifndef _SERVER
#include "KCodec.h"
#endif
#include "KIniFile.h"
#include <string.h>
#include "KSG_StringProcess.h"

//---------------------------------------------------------------------------
typedef struct {
	DWORD		Id;			// 文件标识 = 0x4b434150 ("PACK")
	DWORD		DataLen;	// 文件原始的长度
	DWORD		PackLen;	// 文件压缩后长度
	DWORD		Method;		// 使用的压缩算法
} TPackHead;
//---------------------------------------------------------------------------
static int  g_nCodec = 0;// codec method = 0,1,2,3,
//---------------------------------------------------------------------------
// 函数:	SetCodec
// 功能:	set codec method
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void g_SetIniCodec(int nCodec)
{
	g_nCodec = nCodec;
}
//---------------------------------------------------------------------------
// 函数:	KIniFile
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KIniFile::KIniFile()
{
	g_MemZero(&m_Header, sizeof(SECNODE));
}
//---------------------------------------------------------------------------
// 函数:	~KIniFile
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KIniFile::~KIniFile()
{
	Clear();
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	加载一个INI文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KIniFile::Load(LPCSTR FileName)
{
	KPakFile	File;
	DWORD		dwSize;
	PVOID		Buffer;
	TPackHead*	pHeader;

	// check file name
	if (FileName[0] == 0)
		return FALSE;

	if (m_Header.pNextNode)
		Clear();

	if (!File.Open((char*)FileName))
	{
		g_DebugLog("Can't open ini file : %s", FileName);
		return FALSE;
	}
#pragma message("KIniFile 当前不支持 KeyName 为中文! by Romandou")

	dwSize = File.Size();

	Buffer = m_MemStack.Push(dwSize + 4);

	File.Read(Buffer, dwSize);

	pHeader = (TPackHead*)Buffer;
	if (pHeader->Id == 0x4b434150) // "PACK"
	{
		File.Close();
		return LoadPack(FileName);
	}

	CreateIniLink(Buffer, dwSize);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	LoadPack
// 功能:	打开一个压缩的INI文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KIniFile::LoadPack(LPCSTR FileName)
{
	KPakFile	File;
	PVOID		PackBuf;
	PVOID		DataBuf;
#ifndef _SERVER
	KCodec*		pCodec;
	TCodeInfo	CodeInfo;
#endif
	TPackHead	Header;

	// check file name
	if (FileName[0] == 0)
		return FALSE;

	// close open file
	if (m_Header.pNextNode)
		Clear();

	// open the pack ini file
	if (!File.Open((char*)FileName))
		return FALSE;

	// read pack file header
	File.Read(&Header, sizeof(Header));

	// if not a pack file read directly
	if (Header.Id != 0x4b434150) // "PACK"
		return FALSE;

	// init codec
#ifndef _SERVER
	pCodec = NULL;
	g_InitCodec(&pCodec, Header.Method);
	if (pCodec == NULL)
		return FALSE;
#endif
	// allocate buffer for pack data
	PackBuf = m_MemStack.Push(Header.PackLen);

	// allocate bufer for real data
	DataBuf = m_MemStack.Push(Header.DataLen + 2);

	// read pack data for file
	File.Read(PackBuf, Header.PackLen);

	// decompress data
#ifndef _SERVER
	CodeInfo.lpPack = (PBYTE)PackBuf;
	CodeInfo.dwPackLen = Header.PackLen;
	CodeInfo.lpData = (PBYTE)DataBuf;
	CodeInfo.dwDataLen = Header.DataLen;
	pCodec->Decode(&CodeInfo);
	g_FreeCodec(&pCodec, Header.Method);
	if (CodeInfo.dwDataLen != Header.DataLen)
	{
		g_DebugLog("Ini file unpack fail : %s", FileName);
		return FALSE;
	}
#endif
	// check data length

	// create ini link in memory
	CreateIniLink(DataBuf, Header.DataLen);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Save
// 功能:	保存当前的INI文件
// 参数:	void
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KIniFile::Save(LPCSTR FileName)
{
	KFile		File;
	LPSTR		DataBuf;
	LPSTR		pBuffer;
	DWORD		dwLen;
	
	if (FileName[0] == 0)
		return FALSE;

	if (g_nCodec)
		return SavePack(FileName);
	
	dwLen = m_MemStack.GetStackSize();

	DataBuf = (LPSTR)m_MemStack.Push(dwLen);
	pBuffer = DataBuf;
	
	SECNODE* SecNode = m_Header.pNextNode;
	KEYNODE* KeyNode = NULL;
	
	// write ini file to memory buffer
	while (SecNode != NULL)
	{
		dwLen = sprintf(pBuffer, "%s\r\n", SecNode->pSection);
		pBuffer += dwLen;
		KeyNode = SecNode->pKeyNode.pNextNode;
		while (KeyNode != NULL)
		{
			dwLen = sprintf(pBuffer, "%s%c%s\r\n", KeyNode->pKey, '=',
				KeyNode->pValue);
			pBuffer += dwLen;
			KeyNode = KeyNode->pNextNode;
		}
		SecNode = SecNode->pNextNode;
		dwLen = sprintf(pBuffer, "\r\n");
		pBuffer += dwLen;
	}

	// get buffer len
	dwLen = pBuffer - DataBuf;
	
	// create ini file
	if (!File.Create((char*)FileName))
		return FALSE;

	// write ini file
	File.Write(DataBuf, dwLen);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	SavePack
// 功能:	保存一个压缩的INI文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KIniFile::SavePack(LPCSTR FileName)
{
	KFile		File;
	PVOID		DataBuf;
	PVOID		PackBuf;
#ifndef _SERVER
	KCodec*		pCodec;
	TCodeInfo	CodeInfo;
#endif
	TPackHead	Header;
	LPSTR		pBuffer;
	DWORD		dwLen;
	
	// check file name
	if (FileName[0] == 0)
		return FALSE;

	dwLen = m_MemStack.GetStackSize();
	
	// allocate realdata buffer
	DataBuf = m_MemStack.Push(dwLen);

	// allocate compress buffer
	PackBuf = m_MemStack.Push(dwLen + 256);

	// print ini file to realdata buffer
	pBuffer = (LPSTR)DataBuf;
	SECNODE* SecNode = m_Header.pNextNode;
	KEYNODE* KeyNode = NULL;

	// write ini file to memory buffer
	while (SecNode != NULL)
	{
		dwLen = sprintf(pBuffer, "%s\r\n", SecNode->pSection);
		pBuffer += dwLen;
		KeyNode = SecNode->pKeyNode.pNextNode;
		while (KeyNode != NULL)
		{
			dwLen = sprintf(pBuffer, "%s%c%s\r\n", KeyNode->pKey, '=',
					KeyNode->pValue);
			pBuffer += dwLen;
			KeyNode = KeyNode->pNextNode;
		}
		SecNode = SecNode->pNextNode;
		dwLen = sprintf(pBuffer, "\r\n");
		pBuffer += dwLen;
	}

	// set header
	Header.Id = 0x4b434150; // "PACK"
	Header.Method = g_nCodec; // CODEC_LZO = 2;

	// init codec
#ifndef _SERVER
	pCodec = NULL;
	g_InitCodec(&pCodec, Header.Method);
	if (pCodec == NULL)
		return FALSE;
#endif

	// compress the data buffer
#ifndef _SERVER
	CodeInfo.lpData = (PBYTE)DataBuf;
	CodeInfo.lpPack = (PBYTE)PackBuf;
	CodeInfo.dwDataLen = pBuffer - (LPCSTR)DataBuf;
	CodeInfo.dwPackLen = 0;
	pCodec->Encode(&CodeInfo);
	g_FreeCodec(&pCodec, Header.Method);
	Header.DataLen = CodeInfo.dwDataLen;
	Header.PackLen = CodeInfo.dwPackLen;
#endif

	// create ini file
	if (!File.Create((char*)FileName))
		return FALSE;

	// write file header
	File.Write(&Header, sizeof(Header));

	// write file data
	File.Write(PackBuf, Header.PackLen);

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Clear
// 功能:	清除INI文件的内容
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::Clear()
{
	ReleaseIniLink();
}
//---------------------------------------------------------------------------
// 函数:	ReadLine
// 功能:	读取INI文件的一行
// 参数:	Buffer	缓存
//			Szie	长度
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
BOOL KIniFile::ReadLine(LPSTR Buffer,LONG Size)
{
	if (m_Offset >= Size)
	{
		return FALSE;
	}
	while (Buffer[m_Offset] != 0x0D && Buffer[m_Offset] != 0x0A)
	{
		m_Offset++;
		if (m_Offset >= Size)
			break;
	}
	Buffer[m_Offset] = 0;
	if (Buffer[m_Offset] == 0x0D && Buffer[m_Offset + 1] == 0x0A)
		m_Offset += 2;
	else
		m_Offset += 1;	//linux [wxb 2003-7-29]
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	IsKeyChar
// 功能:	判断一个字符是否为字母
// 参数:	ch		要判断的字符	
// 返回:	TRUE	是字母
//			FALSE	不是字母
//---------------------------------------------------------------------------
BOOL KIniFile::IsKeyChar(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return TRUE;
	if ((ch >= 'A') && (ch <= 'Z'))
		return TRUE;
	if ((ch >= 'a') && (ch <= 'z'))
		return TRUE;
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	CreateIniLink
// 功能:	创建Ini链表
// 参数:	pBuffer		缓存
//			nBufLen		长度
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::CreateIniLink(LPVOID pBuffer, LONG nBufLen)
{
	LPSTR lpBuffer = (LPSTR)pBuffer;
	LPSTR lpString = NULL;
	LPSTR lpValue  = NULL;
	char  szSection[32] = "[MAIN]";

	m_Offset = 0;
	while (m_Offset < nBufLen)
	{
		lpString = &lpBuffer[m_Offset];
		if (!ReadLine(lpBuffer, nBufLen))
			break;
		if (IsKeyChar(*lpString))
		{
			lpValue = SplitKeyValue(lpString);
			SetKeyValue(szSection, lpString, lpValue);
		}
		if (*lpString == '[')
		{
			g_StrCpyLen(szSection, lpString, sizeof(szSection));
		}
	}
}
//---------------------------------------------------------------------------
// 函数:	ReleaseIniLink()
// 功能:	释放Ini链表
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::ReleaseIniLink()
{
	SECNODE* pThisSec = &m_Header;
	SECNODE* pNextSec = pThisSec->pNextNode;
	KEYNODE* pThisKey = NULL;
	KEYNODE* pNextKey = NULL;

	while (pNextSec != NULL)
	{
		pThisSec = pNextSec->pNextNode;
		pThisKey = &pNextSec->pKeyNode;
		pNextKey = pThisKey->pNextNode;
		while (pNextKey != NULL)
		{
			pThisKey = pNextKey->pNextNode;
			m_MemStack.Free(pNextKey->pKey);
			m_MemStack.Free(pNextKey->pValue);
			m_MemStack.Free(pNextKey);
			pNextKey = pThisKey;
		}
		m_MemStack.Free(pNextSec->pSection);
		m_MemStack.Free(pNextSec);
		pNextSec = pThisSec;
	}
	m_Header.pNextNode = NULL;

	m_MemStack.FreeAllChunks();
}
//---------------------------------------------------------------------------
// 函数:	SplitKeyValue
// 功能:	分割Key和Value
// 参数:	pString		Key=Value
// 返回:	指向Value
//---------------------------------------------------------------------------
LPSTR KIniFile::SplitKeyValue(LPSTR pString)
{
	LPSTR pValue = pString;
	while (*pValue)
	{
		if (*pValue == '=')
			break;
		pValue++;
	}
	*pValue = 0;
	return pValue + 1;
}
//---------------------------------------------------------------------------
// 函数:	String2Id
// 功能:	字符串转成32 bits ID
// 参数:	pString		字符串
// 返回:	32 bits ID
//---------------------------------------------------------------------------
DWORD KIniFile::String2Id(LPCSTR pString)
{
	DWORD Id = 0;
	for (int i=0; pString[i]; i++)
	{
		Id = (Id + (i+1) * pString[i]) % 0x8000000b * 0xffffffef;
	}
	return Id ^ 0x12345678;
}
//---------------------------------------------------------------------------
// 函数:	IsSectionExist
// 功能:	Section是否存在
// 参数:	pSection	节的名字
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KIniFile::IsSectionExist(LPCSTR pSection)
{
	// setup section name
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}
	
	// search for the matched section
	SECNODE* pSecNode = m_Header.pNextNode;
	DWORD dwID = String2Id(szSection);
	while (pSecNode != NULL)
	{
		if (dwID == pSecNode->dwID)
			return TRUE;
		pSecNode = pSecNode->pNextNode;
	}
	
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	EraseSection
// 功能:	清除一个Section的内容
// 参数:	pSection	节的名字
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::EraseSection(LPCSTR pSection)
{
	// setup section name
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}

	// search for the matched section
	SECNODE* pSecNode = m_Header.pNextNode;
	DWORD dwID = String2Id(szSection);
	while (pSecNode != NULL)
	{
		if (dwID == pSecNode->dwID)
		{
			break;
		}
		pSecNode = pSecNode->pNextNode;
	}

	// if no such section found
	if (pSecNode == NULL)
	{
		return;
	}

	// erase all key in the section
	KEYNODE* pThisKey = &pSecNode->pKeyNode;
	KEYNODE* pNextKey = pThisKey->pNextNode;
	while (pNextKey != NULL)
	{
		pThisKey = pNextKey->pNextNode;
		m_MemStack.Free(pNextKey->pKey);
		m_MemStack.Free(pNextKey->pValue);
		m_MemStack.Free(pNextKey);
		pNextKey = pThisKey;
	}
	pSecNode->pKeyNode.pNextNode = NULL;
}

//---------------------------------------------------------------------------
// 函数:	EraseKey
// 功能:	清除Section的下一个Key的内容
// 参数:	pSection	节的名字
// 返回:	void
//---------------------------------------------------------------------------
void	KIniFile::EraseKey(LPCSTR lpSection, LPCSTR lpKey)
{
	// setup section name
	char szSection[32] = "[";
	if (lpSection[0] != '[')
	{
		g_StrCat(szSection, lpSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, lpSection);
	}

	// search for the matched section
	SECNODE* pSecNode = m_Header.pNextNode;
	DWORD dwID = String2Id(szSection);
	while (pSecNode != NULL)
	{
		if (dwID == pSecNode->dwID)
		{
			break;
		}
		pSecNode = pSecNode->pNextNode;
	}

	// if no such section found
	if (pSecNode == NULL)
	{
		return;
	}

	// erase all key in the section
	KEYNODE* pThisKey = &pSecNode->pKeyNode;
	KEYNODE* pNextKey = pThisKey->pNextNode;
	dwID = String2Id(lpKey);
	while (pNextKey != NULL)
	{
		
		if (pNextKey->dwID == dwID)
		{
		pThisKey->pNextNode = pNextKey->pNextNode;
		m_MemStack.Free(pNextKey->pKey);
		m_MemStack.Free(pNextKey->pValue);
		m_MemStack.Free(pNextKey);
		return;
		}
		pThisKey = pNextKey;
		pNextKey = pNextKey->pNextNode;
	}

}



//---------------------------------------------------------------------------
// 函数:	SetKeyValue
// 功能:	设置Key的Value
// 参数:	pSection	节名
//			pKey		建名
//			pValue		建值
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KIniFile::SetKeyValue(
	LPCSTR	pSection,
	LPCSTR	pKey,
	LPCSTR	pValue)
{
	int		nLen;
	DWORD	dwID;

	// setup section name
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}

	// search for the matched section
	SECNODE* pThisSecNode = &m_Header;
	SECNODE* pNextSecNode = pThisSecNode->pNextNode;
	dwID = String2Id(szSection);
	while (pNextSecNode != NULL)
	{
		if (dwID == pNextSecNode->dwID)
		{
			break;
		}
		pThisSecNode = pNextSecNode;
		pNextSecNode = pThisSecNode->pNextNode;
	}

	// if no such section found create a new section
	if (pNextSecNode == NULL)
	{
		nLen = g_StrLen(szSection) + 1;
		pNextSecNode = (SECNODE *)m_MemStack.Push(sizeof(SECNODE));
		pNextSecNode->pSection = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextSecNode->pSection, szSection, nLen);
		pNextSecNode->dwID = dwID;
		pNextSecNode->pKeyNode.pNextNode = NULL;
		pNextSecNode->pNextNode = NULL;
		pThisSecNode->pNextNode = pNextSecNode;
	}

	// search for the same key
	KEYNODE* pThisKeyNode = &pNextSecNode->pKeyNode;
	KEYNODE* pNextKeyNode = pThisKeyNode->pNextNode;
	dwID = String2Id(pKey);
	while (pNextKeyNode != NULL)
	{
		if (dwID == pNextKeyNode->dwID)
		{
			break;
		}
		pThisKeyNode = pNextKeyNode;
		pNextKeyNode = pThisKeyNode->pNextNode;
	}

	// if no such key found create a new key
	if (pNextKeyNode == NULL)
	{
		pNextKeyNode = (KEYNODE *)m_MemStack.Push(sizeof(KEYNODE));

		nLen = g_StrLen(pKey) + 1;
		pNextKeyNode->pKey = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextKeyNode->pKey, (void*)pKey, nLen);

		nLen = g_StrLen(pValue) + 1;
		pNextKeyNode->pValue = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextKeyNode->pValue, (void*)pValue, nLen);

		pNextKeyNode->dwID = dwID;
		pNextKeyNode->pNextNode = NULL;
		pThisKeyNode->pNextNode = pNextKeyNode;
	}
	// replace the old value with new
	else
	{
		m_MemStack.Free(pNextKeyNode->pValue);
		nLen = g_StrLen(pValue) + 1;
		pNextKeyNode->pValue = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextKeyNode->pValue, (void*)pValue, nLen);
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetKeyValue
// 功能:	取得Key的Value
// 参数:	pSection	节名
//			pKey		建名
//			pValue		建值
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KIniFile::GetKeyValue(
	LPCSTR	pSection,
	LPCSTR	pKey,
	LPSTR	pValue,
	DWORD	dwSize)
{
	DWORD	dwID;

	// setup section name
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}

	// search for the matched section
	SECNODE* pSecNode = m_Header.pNextNode;
	dwID = String2Id(szSection);
	while (pSecNode != NULL)
	{
		if (dwID == pSecNode->dwID)
		{
			break;
		}
		pSecNode = pSecNode->pNextNode;
	}

	// if no such section founded
	if (pSecNode == NULL)
	{
		return FALSE;
	}

	// search for the same key
	KEYNODE* pKeyNode = pSecNode->pKeyNode.pNextNode;
	dwID = String2Id(pKey);
	while (pKeyNode != NULL)
	{
		if (dwID == pKeyNode->dwID)
		{
			break;
		}
		pKeyNode = pKeyNode->pNextNode;
	}

	// if no such key found
	if (pKeyNode == NULL)
	{
		return FALSE;
	}

	// copy the value of the key
	g_StrCpyLen(pValue, pKeyNode->pValue, dwSize);
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetString
// 功能:	读取一个字符串
// 参数:	lpSection		节名
//			lpKeyName		建名
//			lpDefault		缺省值
//			lpRString		返回值
//			dwSize			返回字符串的最大长度
// 返回:	void
//---------------------------------------------------------------------------
BOOL KIniFile::GetString(
	LPCSTR lpSection,		// points to section name
	LPCSTR lpKeyName,		// points to key name
	LPCSTR lpDefault,		// points to default string
	LPSTR lpRString,		// points to destination buffer
	DWORD dwSize			// size of string buffer
	)
{
	if (GetKeyValue(lpSection, lpKeyName, lpRString, dwSize))
		return TRUE;
	g_StrCpyLen(lpRString, lpDefault, dwSize);
	return FALSE;
}
//---------------------------------------------------------------------------
// 函数:	GetInteger
// 功能:	读取一个整数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	void
//---------------------------------------------------------------------------
BOOL KIniFile::GetInteger(
	LPCSTR lpSection,		// points to section name
	LPCSTR lpKeyName,		// points to key name
	int   nDefault,			// default value
	int   *pnValue          // points to value
	)
{
	char Buffer[32];
	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		*pnValue = atoi(Buffer);
		return TRUE;
	}
	else
	{
		*pnValue = nDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetInteger2
// 功能:	读取2个整数，整数之间用逗号分割。
// 参数:	lpSection		节名
//			lpKeyName		建名
//			pnValue1		返回值1
//			pnValue2		返回值2
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::GetInteger2(
	LPCSTR lpSection,		// points to section name
	LPCSTR lpKeyName,		// points to key name
	int   *pnValue1,		// value 1
	int   *pnValue2			// value 2
	)
{
	char  Buffer[32];
	char* pChar = Buffer;
	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		while (*pChar)
		{
			if (',' == *pChar)
			{
				*pChar++ = 0;
				break;
			}
			pChar++;
		}
		*pnValue1 = atoi(Buffer);
		*pnValue2 = atoi(pChar);
	}
}
//---------------------------------------------------------------------------
// 函数:	GetFloat
// 功能:	读取1个浮点数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			fDefault		缺省值
//			pfValue			返回值
// 返回:	void
//---------------------------------------------------------------------------
BOOL KIniFile::GetFloat(
	LPCSTR	lpSection,		// points to section name
	LPCSTR	lpKeyName,		// points to key name
	float	fDefault,		// default value
	float	*pfValue        // return value
	)
{
	char Buffer[32];
	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		*pfValue = (float)atof(Buffer);
		return TRUE;
	}
	else
	{
		*pfValue = fDefault;
		return FALSE;
	}
}
//---------------------------------------------------------------------------
// 函数:	GetFloat2
// 功能:	读取2个浮点数，之间用逗号分割。
// 参数:	lpSection		节名
//			lpKeyName		建名
//			pfValue1		返回值1
//			pfValue2		返回值2
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::GetFloat2(
	LPCSTR lpSection,		// points to section name
	LPCSTR lpKeyName,		// points to key name
	float *pfValue1,		// value 1
	float *pfValue2			// value 2
	)
{
	char  Buffer[32];
	char* pChar = Buffer;

	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		while (*pChar)
		{
			if (',' == *pChar)
			{
				*pChar++ = 0;
				break;
			}
			pChar++;
		}
		*pfValue1 = (float)atof(Buffer);
		*pfValue2 = (float)atof(pChar);
	}
}
//---------------------------------------------------------------------------
// 函数:	GetStruct
// 功能:	读取一个结构
// 参数:	lpSection		节名
//			lpKeyName		建名
//			lpStruct		缓存
//			dwSize			缓存大小
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::GetStruct(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	LPVOID	lpStruct,		// pointer to buffer that contains data to add
	DWORD	dwSize			// size, in bytes, of the buffer
	)
{
	char    Buffer[512];
	LPBYTE	lpByte;
	BYTE	ah,al;

	if (!GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
		return;
	lpByte = (LPBYTE)lpStruct;
	int len = g_StrLen(Buffer);
	if (len / 2 != (int)dwSize)
		return;
	for (int i = 0; i < len; i += 2)
	{
		// get byte high
		ah = Buffer[i];
		if ((ah >= 48) && (ah <= 57))
			ah = (BYTE)(ah - 48);
		else
			ah = (BYTE)(ah - 65 + 10);
		// get byte low
		al = Buffer[i+1];
		if ((al >= 48) && (al <= 57))
			al = (BYTE)(al - 48);
		else
			al = (BYTE)(al - 65 + 10);
		// set struct bye
		*lpByte++ = (BYTE)(ah * 16 + al);
	}
}
//---------------------------------------------------------------------------
// 函数:	WriteString
// 功能:	写入一个字符串
// 参数:	lpSection		节名
//			lpKeyName		建名
//			lpString		字符串
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::WriteString(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	LPCSTR	lpString		// pointer to string to add
	)
{
	SetKeyValue(lpSection, lpKeyName, lpString);
}
//---------------------------------------------------------------------------
// 函数:	WriteInteger
// 功能:	写入一个整数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			nValue			整数
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::WriteInteger(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	int 	nValue			// Integer to write
	)
{
	char Buffer[32];

	sprintf(Buffer, "%d", nValue);
	SetKeyValue(lpSection, lpKeyName, Buffer);
}
//---------------------------------------------------------------------------
// 函数:	WriteInteger2
// 功能:	写入2个整数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			nValue1			整数1
//			nValue2			整数2
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::WriteInteger2(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	int 	Value1,			// value 1 to write
	int		Value2			// value 2 to write
	)
{
	char Buffer[32];

	sprintf(Buffer, "%d,%d", Value1, Value2);
	SetKeyValue(lpSection, lpKeyName, Buffer);
}
//---------------------------------------------------------------------------
// 函数:	WriteFloat
// 功能:	写入1个浮点数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			fValue			浮点数
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::WriteFloat(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	float	fValue			// Integer to write
	)
{
	char Buffer[32];

	sprintf(Buffer,"%f",fValue);
	SetKeyValue(lpSection, lpKeyName, Buffer);
}
//---------------------------------------------------------------------------
// 函数:	WriteFloat2
// 功能:	写入2个浮点数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			fValue1			浮点数1
//			fValue2			浮点数2
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::WriteFloat2(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	float 	fValue1,		// value 1 to write
	float	fValue2			// value 2 to write
	)
{
	char Buffer[32];

	sprintf(Buffer, "%f,%f", fValue1, fValue2);
	SetKeyValue(lpSection, lpKeyName, Buffer);
}
//---------------------------------------------------------------------------
// 函数:	WriteFloat2
// 功能:	写入一个结构
// 参数:	lpSection		节名
//			lpKeyName		建名
//			lpStruct		结构
//			dwSize			结构大小
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::WriteStruct(
	LPCSTR	lpSection,		// pointer to section name
	LPCSTR	lpKeyName,		// pointer to key name
	LPVOID	lpStruct,		// pointer to buffer that contains data to add
	DWORD 	dwSize			// size, in bytes, of the buffer
	)
{
	char    Buffer[512];
	LPSTR	lpBuff = Buffer;
	LPBYTE	lpByte;

	if (dwSize * 2 >= 512)
	{
		return;
	}
	lpByte = (LPBYTE) lpStruct;
	for (DWORD i=0; i<dwSize; i++)
	{
		sprintf(lpBuff,"%02x",*lpByte);
		lpBuff++;
		lpBuff++;
		lpByte++;
	}
	g_StrUpper(Buffer);
	SetKeyValue(lpSection, lpKeyName, Buffer);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
BOOL KIniFile::GetNextSection(LPCSTR pSection, LPSTR pNextSection)
{
	
	if (!pSection[0])	//	传入参数为""时就是取第一个Section
	{
		if (!m_Header.pNextNode)
			return FALSE;
		else
		{
			g_StrCpy(pNextSection, m_Header.pNextNode->pSection);
			return TRUE;
		}
	}
	else
	{
		char szSection[32] = "[";
		if (pSection[0] != '[')
		{
			g_StrCat(szSection, pSection);
			g_StrCat(szSection, "]");
		}
		else
		{
			g_StrCpy(szSection, pSection);
		}
		//	查找参数给定的Section
		SECNODE* pThisSecNode = &m_Header;
		SECNODE* pNextSecNode = pThisSecNode->pNextNode;
		DWORD dwID = String2Id(szSection);
		while (pNextSecNode != NULL)
		{
			if (dwID == pNextSecNode->dwID)
			{
				break;
			}
			pThisSecNode = pNextSecNode;
			pNextSecNode = pThisSecNode->pNextNode;
		}
		//	没有这个Section，无所谓下一个
		if (!pNextSecNode)
		{
			return FALSE;
		}
		else
		{
			//	参数给定的Section已经是最后一个了
			if (!pNextSecNode->pNextNode)
			{
				return FALSE;
			}
			else	
			{
				g_StrCpy(pNextSection, pNextSecNode->pNextNode->pSection);
				return TRUE;
			}
		}
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
BOOL KIniFile::GetNextKey(LPCSTR pSection, LPCSTR pKey, LPSTR pNextKey)
{
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}
	//	查找参数给定的Section
	SECNODE* pThisSecNode = &m_Header;
	SECNODE* pNextSecNode = pThisSecNode->pNextNode;
	DWORD dwID = String2Id(szSection);
	while (pNextSecNode != NULL)
	{
		if (dwID == pNextSecNode->dwID)
		{
			break;
		}
		pThisSecNode = pNextSecNode;
		pNextSecNode = pThisSecNode->pNextNode;
	}
	//	没有这个Section，无所谓取Key
	if (!pNextSecNode)
	{
		return FALSE;
	}

	KEYNODE* pThisKeyNode = &pNextSecNode->pKeyNode;
	KEYNODE* pNextKeyNode = pThisKeyNode->pNextNode;

	//	Key给的参数为""的话，取第一个Key
	if (!pKey[0])
	{
		//	没有Key，返回失败
		if (!pNextKeyNode->pKey)
		{
			return FALSE;
		}
		else
		{
			g_StrCpy(pNextKey, pNextKeyNode->pKey);
			return TRUE;
		}
	}

	dwID = String2Id(pKey);
	while(pNextKeyNode != NULL)
	{
		if (dwID == pNextKeyNode->dwID)
		{
			break;
		}
		pThisKeyNode = pNextKeyNode;
		pNextKeyNode = pThisKeyNode->pNextNode;
	}
	//	找不到所给的Key，无所谓下一个
	if (!pNextKeyNode)
	{
		return FALSE;
	}
	//	参数给定的Key已经是最后一个了
	if (!pNextKeyNode->pNextNode)
	{
		return FALSE;
	}
	else
	{
		g_StrCpy(pNextKey, pNextKeyNode->pNextNode->pKey);
		return TRUE;
	}
}

int	KIniFile::GetSectionCount()
{
	char Section[200];
	char Section1[200];
	if (!GetNextSection("",Section)) return 0;
	int i  = 1;
	while (1)
	{
		strcpy(Section1, Section);
		if (!GetNextSection(Section1, Section)) return i;
		i++;
	}
	return i;
}

// 如果从文件中没有取到对应的Section 和 Key则 pRect里面的值不变
void KIniFile::GetRect(LPCSTR lpSection, LPCSTR lpKeyName, RECT *pRect)
{
	char  Buffer[256];

	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
        const char *pcszTemp = Buffer;
        
        pRect->left   = KSG_StringGetInt(&pcszTemp, 0);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        pRect->top    = KSG_StringGetInt(&pcszTemp, 0);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        pRect->right  = KSG_StringGetInt(&pcszTemp, 0);
        KSG_StringSkipSymbol(&pcszTemp, ',');
        pRect->bottom = KSG_StringGetInt(&pcszTemp, 0);
		//sscanf(Buffer, "%d,%d,%d,%d", &(pRect->left), &(pRect->top), &(pRect->right), &(pRect->bottom));
	}
}

void KIniFile::GetFloat3(LPCSTR lpSection, LPCSTR lpKeyName, float* pRect)
{
	char	Buffer[256];

	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		sscanf(Buffer, "%f,%f,%f", &pRect[0], &pRect[1], &pRect[2]);
	}
}

void KIniFile::GetFloat4(LPCSTR lpSection, LPCSTR lpKeyName, float* pRect)
{
	char  Buffer[256];

	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		sscanf(Buffer, "%f,%f,%f,%f", &pRect[0], &pRect[1], &pRect[2], &pRect[3]);
	}
}

void KIniFile::GetBool(LPCSTR lpSection, LPCSTR lpKeyName, BOOL* pBool)
{
	char  Buffer;

	if (GetKeyValue(lpSection, lpKeyName, &Buffer, 1))
	{
		if ('T' == Buffer)
			*pBool = TRUE;
		else
			*pBool = FALSE;
	}
}
