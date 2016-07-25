#include "precompile.h"
#include <crtdbg.h>
#include "TextureRes.h"
#include "TextureResMgr.h"

TextureResMgr::TextureResMgr()
{
	m_uCheckPoint = ISBP_CHECK_POINT_DEF;
	m_tmLastCheckBalance = timeGetTime();
	m_bDoProfile = false;
	m_nLoadCount = 0;
	m_nReleaseCount = 0;
	m_uTexCacheMemUsed = 0;
    m_nMaxReleaseCount = 0;
	
	// 根据物理内存大小决定资源缓冲区的大小
	MEMORYSTATUS stat;
	GlobalMemoryStatus (&stat);
	if(stat.dwTotalPhys <= 134217728)
    {
		m_nBalanceNum = ISBP_BALANCE_NUM_DEF128;
        m_nMaxReleaseCount = 16;
    }
	else if(stat.dwTotalPhys <= 134217728 * 2)
    {
		m_nBalanceNum = ISBP_BALANCE_NUM_DEF256;
        m_nMaxReleaseCount = 32;
    }
	else
    {
		m_nBalanceNum = ISBP_BALANCE_NUM_DEF512;
        m_nMaxReleaseCount = 64;
    }
}

TextureResMgr::~TextureResMgr()
{
	Free();
}

void TextureResMgr::SetBalanceParam(int32 nNumImage, uint32 uCheckPoint)
{
//	m_nBalanceNum = nNumImage;
//	m_uCheckPoint = uCheckPoint;
}

void TextureResMgr::CheckBalance()
{
	static DWORD dwFID=10, dwSum=0;
	static DWORD nNum = 0;

    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	//int i;
	uint32 nUnUseTime, nMaxUnUseTime = 1000;
	int32 nNodeSelected = -1;

    vector<ResNode>::iterator iBegin, iEnd;
    vector<ResNode>::iterator iNewBegin;
    

	uint32 nTickCount = GetTickCount();

    int nCurrentReleaseCount = 0;
    
    iEnd = m_TextureResList.end(); 
    iNewBegin = m_TextureResList.begin();
    for (iBegin = m_TextureResList.begin(); iBegin != iEnd; ++iBegin)
    {
        nUnUseTime = nTickCount - (iBegin->m_nLastUsedTime);
        if (
            (!iBegin->m_bCacheable) ||
            (
                (iBegin->m_pTextureRes) &&
                (iBegin->m_pTextureRes->m_bLastFrameUsed)
            ) ||
            (nUnUseTime <= 1000 * 24) ||
            (nCurrentReleaseCount >= m_nMaxReleaseCount)
        )
        {
            *iNewBegin++ = *iBegin;

            continue;
        }

        nCurrentReleaseCount++;
		
        // if (nUnUseTime > 1000 * 8) // if > 8 s
        m_nReleaseCount++;
		if(iBegin->m_pTextureRes)
		{
            m_uTexCacheMemUsed -= iBegin->m_pTextureRes->m_nTexMemUsed;
            iBegin->m_pTextureRes->Release();
			SAFE_DELETE(iBegin->m_pTextureRes);
		}
    }
    
    if (iNewBegin != iEnd)
        m_TextureResList.erase(iNewBegin, iEnd);



//	// 选择最长时间没有使用的资源，并释放
//	m_tmLastCheckBalance = timeGetTime();
//	for (i = m_TextureResList.size() -1; i >= 0; i--)
//	{
//		if(!m_TextureResList[i].m_bCacheable)
//			continue;
//
//		nUnUseTime = nTickCount - m_TextureResList[i].m_nLastUsedTime;
//		bool b = false;
//		if(m_TextureResList[i].m_pTextureRes)
//			b = m_TextureResList[i].m_pTextureRes->m_bLastFrameUsed;
//		if(!b && nUnUseTime > nMaxUnUseTime )
//		{
//			nMaxUnUseTime = nUnUseTime;
//			nNodeSelected = i;
//		}
//	}
//
//	if (nNodeSelected >= 0)
//	{
//		m_nReleaseCount++;
//		if(m_TextureResList[nNodeSelected].m_pTextureRes)
//		{
//            m_TextureResList[nNodeSelected].m_pTextureRes->Release();
//			SAFE_DELETE(m_TextureResList[nNodeSelected].m_pTextureRes);
//			m_TextureResList.erase(m_TextureResList.begin() + nNodeSelected);
//		}
//		else
//			m_TextureResList.erase(m_TextureResList.begin() + nNodeSelected);
//	}
}

uint32 TextureResMgr::CreateImage(const char* pszName, int32 nWidth, int32 nHeight, int32 nType)
{
	if (!pszName || !pszName[0])
		return 0;

	if (nType != ISI_T_BITMAP16)
		return 0;
	
    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	// 将名字映射为ID
	uint32 uImage = g_FileName2Id((LPSTR)pszName);
	// 如果资源已经存在则返回
	int nIdx = FindImage(uImage, 0);
	if (nIdx >= 0)
		return 0;
	
	// 创建贴图资源
	TextureResBmp *pTexRes = new TextureResBmp;
	if (pTexRes == NULL)
		return 0;
	if (!pTexRes->CreateImage(pszName, nWidth, nHeight, nType))
	{
		delete pTexRes;
		return 0;
	}

	ResNode node;
	node.m_bCacheable = false;
	node.m_nLastUsedTime = GetTickCount();
	node.m_nType = nType;
	node.m_nID = uImage;
	node.m_pTextureRes = pTexRes;

	// 将贴图资源加入数组
	nIdx = - nIdx - 1;
	m_TextureResList.insert(m_TextureResList.begin() + nIdx, node);
	
	return uImage;
}

int TextureResMgr::FindImage(uint32 uImage, int nPossiblePosition)
{
	int nPP = nPossiblePosition;
	int nNumImages = m_TextureResList.size();
	if (nPP < 0 || nPP >= nNumImages)
	{
		if (nNumImages <= 0)
		{
			return -1;
		}
		else
		{
			nPP = nNumImages / 2;
		}
	}

	if (m_TextureResList[nPP].m_nID == uImage)
		return nPP;

	int nFrom, nTo, nTryRange;
	nTryRange = ISBP_TRY_RANGE_DEF;

	if (m_TextureResList[nPP].m_nID > uImage)
	{
		nFrom = 0;
		nTo = nPP - 1;
		nPP -= nTryRange;
	}
	else
	{
		nFrom = nPP + 1;
		nTo = nNumImages - 1;
		nPP += nTryRange;
	}

	if (nFrom + nTryRange >= nTo)
		nPP = (nFrom + nTo) >> 1;

	while (nFrom < nTo)
	{
		if (m_TextureResList[nPP].m_nID < uImage)
		{
			nFrom = nPP + 1;
		}
		else if (m_TextureResList[nPP].m_nID > uImage)
		{
			nTo = nPP - 1;
		}
		else
		{
			return nPP;
		}
		nPP = (nFrom + nTo) >> 1;
	}

	if (nFrom == nTo)
	{
		if (m_TextureResList[nPP].m_nID > uImage)
		{
			nPP = - nPP - 1;
		}
		else if (m_TextureResList[nPP].m_nID < uImage)
		{
			nPP = - nPP - 2;
		}
	}
	else
	{
		nPP = - nFrom -1;
	}
	
	return nPP;
}

void TextureResMgr::FreeImage( const char* pszImage)
{
	if (!pszImage || !pszImage[0])
	    return;

    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	unsigned int uImage = g_FileName2Id((LPSTR)pszImage);
	int nIdx = FindImage(uImage, 0);

	if (nIdx < 0)
		return;

	SAFE_DELETE(m_TextureResList[nIdx].m_pTextureRes);
	m_TextureResList.erase(m_TextureResList.begin() + nIdx);	
}

void TextureResMgr::Free()
{
    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	for(size_t i=0; i<m_TextureResList.size(); i++)
		SAFE_DELETE(m_TextureResList[i].m_pTextureRes);

	m_TextureResList.clear();
	m_tmLastCheckBalance = timeGetTime();
}

bool TextureResMgr::GetImageParam( const char* pszImage, KImageParam* pImageData, int nType)
{
	if (!pszImage || !pszImage[0])
		return false;

	unsigned int uImage = 0;
	short nPos = 0;
	void* pTemp = GetImage(pszImage, uImage, nPos, 0, nType, false);
	if (!pTemp)
		return false;

	switch(nType)
	{
	case ISI_T_SPR:
		pImageData->nHeight = ((TextureResSpr *)pTemp)->GetHeight();
		pImageData->nInterval = ((TextureResSpr *)pTemp)->GetInterval();
		pImageData->nNumFrames = ((TextureResSpr *)pTemp)->GetFrameNum();
		pImageData->nNumFramesGroup = ((TextureResSpr *)pTemp)->GetDirections();
		pImageData->nReferenceSpotX = ((TextureResSpr *)pTemp)->GetCenterX();
		pImageData->nReferenceSpotY = ((TextureResSpr *)pTemp)->GetCenterY();
		pImageData->nWidth = ((TextureResSpr *)pTemp)->GetWidth();
		break;
	case ISI_T_BITMAP16:
		pImageData->nHeight = ((TextureResBmp *)pTemp)->GetHeight();
		pImageData->nInterval = 0;
		pImageData->nNumFrames = 1;
		pImageData->nNumFramesGroup = 1;
		pImageData->nReferenceSpotX = 0;
		pImageData->nReferenceSpotY = 0;
		pImageData->nWidth = ((TextureResBmp *)pTemp)->GetWidth();
		break;
	}
	return true;
}

bool TextureResMgr::GetImageFrameParam(const char* pszImage,	int nFrame,
		KRPosition2* pOffset, KRPosition2* pSize, int nType)
{
	if (!pszImage || !pszImage[0])
		return false;

	unsigned int uImage = 0;
	short nPos = 0;
	void* pTemp = GetImage(pszImage, uImage, nPos, 0, nType, false);
	if (!pTemp)
		return false;
	bool bRet = false;
	switch(nType)
	{
	case ISI_T_SPR:
		{
			TextureResSpr* pSpr = (TextureResSpr *)pTemp;
			if(nFrame >= pSpr->m_nFrameNum)
				break;

			if (pOffset)
			{
				pOffset->nX = pSpr->m_pFrameInfo[nFrame].nOffX;
				pOffset->nY = pSpr->m_pFrameInfo[nFrame].nOffY;
			}
			if (pSize)
			{
				pSize->nX = pSpr->m_pFrameInfo[nFrame].nWidth;
				pSize->nY = pSpr->m_pFrameInfo[nFrame].nHeight;
			}
			bRet = true;
		}
		break;
	case ISI_T_BITMAP16:
		if (pOffset)
		{
			pOffset->nX = 0;
			pOffset->nY = 0;
		}
		if (pSize)
		{
			pSize->nX = ((TextureResBmp *)pTemp)->GetWidth();
			pSize->nY = ((TextureResBmp *)pTemp)->GetHeight();
		}
		bRet = true;
		break;
	}
	return bRet;
}

TextureRes* TextureResMgr::GetImage( const char* pszImage, unsigned int& uImage, short& nImagePosition, 
								int nFrame, int nType, bool bPrepareTex)
{
	if (!pszImage || !pszImage[0])
		return NULL;
	
	if (!uImage)
	{
		uImage = g_FileName2Id((LPSTR)pszImage);	// const char to LPSTR, maybe problem.
	}

    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	TextureRes* pObject = NULL;
	if ((nImagePosition = FindImage(uImage, nImagePosition)) >= 0)
	{
		if (m_TextureResList[nImagePosition].m_nType == nType)
		{
			m_TextureResList[nImagePosition].m_nLastUsedTime = GetTickCount();
			pObject = m_TextureResList[nImagePosition].m_pTextureRes;
		}
		else if (m_TextureResList[nImagePosition].m_bCacheable == true &&
				(pObject = LoadImage(pszImage, nType)))
		{			
			m_nLoadCount++;
    
            m_uTexCacheMemUsed -= m_TextureResList[nImagePosition].m_pTextureRes->m_nTexMemUsed;
	
    		SAFE_DELETE(m_TextureResList[nImagePosition].m_pTextureRes);
			m_TextureResList[nImagePosition].m_pTextureRes = pObject;
			m_TextureResList[nImagePosition].m_nLastUsedTime = GetTickCount();
            m_uTexCacheMemUsed += pObject->m_nTexMemUsed;

		}
		if(pObject && m_bDoProfile)
		{
			pObject->m_bLastFrameUsed = true;
			m_nUseCount++;
		}
	}
	else
	{
		pObject = LoadImage(pszImage, nType);
		m_nLoadCount++;
		
		ResNode node;
		node.m_bCacheable = true;
		node.m_nLastUsedTime = GetTickCount();
		node.m_nType = nType;
		node.m_nID = uImage;
		node.m_pTextureRes = pObject;
		nImagePosition = - nImagePosition - 1;	// FindImage时已经找好位置了
		m_TextureResList.insert(m_TextureResList.begin() + nImagePosition, node);

        
		if(pObject && m_bDoProfile)
		{
            m_uTexCacheMemUsed += pObject->m_nTexMemUsed;

			pObject->m_bLastFrameUsed = true;
			m_nUseCount++;
		}
	}

	DWORD tmCur = timeGetTime();
	//为了执行效率所以未把下面的判断放在CheckBalance函数体里面。下同。
	if (
        (m_uTexCacheMemUsed > m_nBalanceNum) && 
		(
            (tmCur <= m_tmLastCheckBalance) || 
            ((tmCur - m_tmLastCheckBalance) > m_uCheckPoint)
        )
    )
	{
		m_tmLastCheckBalance = tmCur;
		CheckBalance();
	}
	
	if(pObject && nType == ISI_T_SPR)
	{
		if(!((TextureResSpr*)pObject)->PrepareFrameData(pszImage, nFrame, bPrepareTex))
			return NULL;
	}

	return pObject;
}

int32 TextureResMgr::GetImagePixelAlpha( const char* pszImage, int nFrame, int nX, int nY, int nType)
{
	if (!pszImage || !pszImage[0])
		return 0;

	int nRet = 0;

	unsigned int uImage = 0;
	short nPos = 0;
	void* pTemp;

	switch(nType)
	{
	case ISI_T_SPR:
		pTemp = GetImage(pszImage, uImage, nPos, nFrame, ISI_T_SPR);
		if (!pTemp)
			break;
		nRet = ((TextureResSpr *)pTemp)->GetPixelAlpha(nFrame, nX, nY);	// 在Spr的方法里已经处理了范围
		break;
	case ISI_T_BITMAP16:
		pTemp = GetImage(pszImage, uImage, nPos, nFrame, ISI_T_BITMAP16);
		if (!pTemp)
			break;
		if (nX < 0 || nY < 0 || nX >= ((TextureResBmp *)pTemp)->GetWidth() || nY >= ((TextureResBmp *)pTemp)->GetHeight())
			nRet = 0;
		else
			nRet = 255;
		break;
	default:
		break;
	}
	
	return nRet;
}

TextureRes* TextureResMgr::LoadImage( const char* pszImageFile, uint32 nType) const
{
	TextureRes* pRet = NULL;
	switch(nType)
	{
	case ISI_T_SPR:
		pRet = new TextureResSpr;
		if (!pRet)
			break;
		if (!pRet->LoadImage((LPSTR)pszImageFile, nType))
		{
			delete pRet;
			pRet = NULL;
			break;
		}
		break;
	case ISI_T_BITMAP16:
		{
			pRet = new TextureResBmp;
			if (!pRet)
				break;
			if (!pRet->LoadImage((LPSTR)pszImageFile, nType))
			{
				delete pRet;
				pRet = NULL;
				break;
			}
		}
		break;
	default:
		break;
	}

	return pRet;
}


void TextureResMgr::StartProfile()
{
	m_nHitCount = 0;
	m_nUseCount = 0;
	m_bDoProfile = true;
	m_uMemDrawingUsed = 0;

    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	for(size_t i=0; i<m_TextureResList.size(); i++)
		if(m_TextureResList[i].m_pTextureRes)
			m_TextureResList[i].m_pTextureRes->m_bLastFrameUsed = false;
}

void TextureResMgr::EndProfile()
{
	m_uTexCacheMemUsed = 0;
	m_bDoProfile = false;

    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	for(size_t i=0; i<m_TextureResList.size(); i++)
	{
		if(m_TextureResList[i].m_pTextureRes)
		{
			m_uTexCacheMemUsed += m_TextureResList[i].m_pTextureRes->m_nTexMemUsed;
			if(m_TextureResList[i].m_pTextureRes->m_bLastFrameUsed)
            {
				m_uMemDrawingUsed += m_TextureResList[i].m_pTextureRes->m_nTexMemUsed;
            }
		}
	}
}

void TextureResMgr::GetProfileString(char* str, int BufLen)
{
	if(!str)
		return;

	char cc[200];
	sprintf(
        cc,
        "CacheMemUsed: %dM\n MemDrawingUsed: %d\n TextureNumber: %d\n",
			m_uTexCacheMemUsed / 1024 / 1024, m_uMemDrawingUsed / 1024 / 1024, m_TextureResList.size());

	if(BufLen > strlen(cc))
		strcpy(str, cc);
}

bool TextureResMgr::InvalidateDeviceObjects()
{
    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	for(size_t i=0; i<m_TextureResList.size(); i++)
	{
		if(m_TextureResList[i].m_pTextureRes)
		{
			if(!m_TextureResList[i].m_pTextureRes->InvalidateDeviceObjects())
				return false;
		}
	}
	return true;
}

bool TextureResMgr::RestoreDeviceObjects()
{
    KAutoCriticalSection AutoLock(m_ImageProcessLock);

	for(size_t i=0; i<m_TextureResList.size(); i++)
	{
		if(m_TextureResList[i].m_pTextureRes)
		{
			if(!m_TextureResList[i].m_pTextureRes->RestoreDeviceObjects())
				return false;
		}
	}
	return true;
}