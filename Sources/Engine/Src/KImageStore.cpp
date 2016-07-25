#include "KWin32.h"
#include "KImageStore.h"

unsigned long hash(const char *file_name) {
	unsigned long id = 0;
	const char *ptr = file_name;
	int index = 0;
	while(*ptr) {
		if(*ptr >= 'A' && *ptr <= 'Z') id = (id + (++index) * (*ptr + 'a' - 'A')) % 0x8000000b * 0xffffffef;
		else id = (id + (++index) * (*ptr)) % 0x8000000b * 0xffffffef;
		ptr++;
	}
	return (id ^ 0x12345678);
}

KImageStore::KImageStore()
{
	m_uCheckPoint = ISBP_CHECK_POINT_DEF;
	m_tmLastCheckBalance = timeGetTime();
	m_bDoProfile = false;
	m_nLoadCount = 0;
	m_nReleaseCount = 0;
	m_fCacheMemUsed = 0.0f;
	
	// 根据物理内存大小决定资源缓冲区的大小
	MEMORYSTATUS stat;
	GlobalMemoryStatus (&stat);
	if(stat.dwTotalPhys <= 134217728)
		m_nBalanceNum = ISBP_BALANCE_NUM_DEF128;
	else if(stat.dwTotalPhys <= 134217728 * 2)
		m_nBalanceNum = ISBP_BALANCE_NUM_DEF256;
	else
		m_nBalanceNum = ISBP_BALANCE_NUM_DEF512;
}

KImageStore::~KImageStore()
{
	Free();
}

void KImageStore::FreeImage( const char* pszImage)
{
	if (!pszImage || !pszImage[0])
	return;

	unsigned int uImage = hash((LPSTR)pszImage);
	int nIdx = FindImage(uImage, 0);

	if (nIdx < 0)
		return;

	SAFE_DELETE(m_TextureResList[nIdx].m_pTextureRes);
	m_TextureResList.erase(m_TextureResList.begin() + nIdx);	
}

void KImageStore::CheckBalance()
{
	static DWORD dwFID=10, dwSum=0;
	static DWORD nNum = 0;

	int i;
	unsigned int nTickCount = GetTickCount();
	unsigned int nUnUseTime, nMaxUnUseTime = 1000;
	int nNodeSelected = -1;

	// 选择最长时间没有使用的资源，并释放
	m_tmLastCheckBalance = timeGetTime();
	for (i = m_TextureResList.size() -1; i >= 0; i--)
	{
		nUnUseTime = nTickCount - m_TextureResList[i].m_nLastUsedTime;
		bool b = false;
		if(m_TextureResList[i].m_pTextureRes)
			b = m_TextureResList[i].m_pTextureRes->m_bLastFrameUsed;
		if(!b && nUnUseTime > nMaxUnUseTime )
		{
			nMaxUnUseTime = nUnUseTime;
			nNodeSelected = i;
		}
	}

	if(nNodeSelected >= 0)
	{
		m_nReleaseCount++;
		if(m_TextureResList[nNodeSelected].m_pTextureRes)
		{
//			if(!m_TextureResList[nNodeSelected].m_pTextureRes->ReleaseATexture())
//			{
//				SAFE_DELETE(m_TextureResList[nNodeSelected].m_pTextureRes);
//				m_TextureResList.erase(m_TextureResList.begin() + nNodeSelected);
//			}
		}
		else
			m_TextureResList.erase(m_TextureResList.begin() + nNodeSelected);
	}
}

unsigned int KImageStore::CreateImage(const char* pszName, int nWidth, int nHeight, int nType)
{
	if (!pszName || !pszName[0])
		return 0;

	if (nType != ISI_T_BITMAP16)
		return 0;
	
	// 将名字映射为ID
	unsigned int uImage = hash((LPSTR)pszName);
	// 如果资源已经存在则返回
	int nIdx = FindImage(uImage, 0);
	if (nIdx >= 0)
		return 0;
	
	// 创建位图资源
	ImageResBmp *pTexRes = new ImageResBmp;
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

int KImageStore::FindImage(unsigned int uImage, int nPossiblePosition)
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
		nPP = (nFrom + nTo) / 2;

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
		nPP = (nFrom + nTo) / 2;
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

void KImageStore::Free()
{
	for(int i=0; i<m_TextureResList.size(); i++)
		SAFE_DELETE(m_TextureResList[i].m_pTextureRes);

	m_TextureResList.clear();
	m_tmLastCheckBalance = timeGetTime();
}

bool KImageStore::GetImageParam( const char* pszImage, KImageParam* pImageData, int nType)
{
	if (!pszImage || !pszImage[0])
		return false;

	unsigned int uImage = 0;
	int	nPos = 0;
	void* pTemp = GetImage(pszImage, uImage, nPos, 0, nType, false);
	if (!pTemp)
		return false;

	switch(nType)
	{
	case ISI_T_SPR:
		pImageData->nHeight = ((ImageResSpr *)pTemp)->GetHeight();
		pImageData->nInterval = ((ImageResSpr *)pTemp)->GetInterval();
		pImageData->nNumFrames = ((ImageResSpr *)pTemp)->GetFrameNum();
		pImageData->nNumFramesGroup = ((ImageResSpr *)pTemp)->GetDirections();
		pImageData->nReferenceSpotX = ((ImageResSpr *)pTemp)->GetCenterX();
		pImageData->nReferenceSpotY = ((ImageResSpr *)pTemp)->GetCenterY();
		pImageData->nWidth = ((ImageResSpr *)pTemp)->GetWidth();
		break;
	case ISI_T_BITMAP16:
		pImageData->nHeight = ((ImageResBmp *)pTemp)->GetHeight();
		pImageData->nInterval = 0;
		pImageData->nNumFrames = 1;
		pImageData->nNumFramesGroup = 1;
		pImageData->nReferenceSpotX = 0;
		pImageData->nReferenceSpotY = 0;
		pImageData->nWidth = ((ImageResBmp *)pTemp)->GetWidth();
		break;
	}
	return true;
}

bool KImageStore::GetImageFrameParam(const char* pszImage,	int nFrame,
		KRPosition2* pOffset, KRPosition2* pSize, int nType)
{
	if (!pszImage || !pszImage[0])
		return false;

	unsigned int uImage = 0;
	int	nPos = 0;
	void* pTemp = GetImage(pszImage, uImage, nPos, 0, nType, false);
	if (!pTemp)
		return false;
	bool bRet = false;
	switch(nType)
	{
	case ISI_T_SPR:
		{
			ImageResSpr* pSpr = (ImageResSpr *)pTemp;
			if(nFrame >= pSpr->GetFrameNum())
				break;

			if (pOffset)
			{
				pOffset->nX = pSpr->m_pFrameInfo[nFrame]->OffsetX;
				pOffset->nY = pSpr->m_pFrameInfo[nFrame]->OffsetY;
			}
			if (pSize)
			{
				pSize->nX = pSpr->m_pFrameInfo[nFrame]->Width;
				pSize->nY = pSpr->m_pFrameInfo[nFrame]->Height;
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
			pSize->nX = ((ImageResBmp *)pTemp)->GetWidth();
			pSize->nY = ((ImageResBmp *)pTemp)->GetHeight();
		}
		bRet = true;
		break;
	}
	return bRet;
}

KImageRes* KImageStore::GetImage( const char* pszImage, unsigned int& uImage, int& nImagePosition, 
								int nFrame, int nType)
{
	if (!pszImage || !pszImage[0])
		return NULL;
	
	if (!uImage)
	{
		uImage = hash((LPSTR)pszImage);	// const char to LPSTR, maybe problem.
	}

	KImageRes* pObject = NULL;
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
			SAFE_DELETE(m_TextureResList[nImagePosition].m_pTextureRes);
			m_TextureResList[nImagePosition].m_pTextureRes = pObject;
			m_TextureResList[nImagePosition].m_nLastUsedTime = GetTickCount();
		}

		pObject->m_bLastFrameUsed = true;
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

		pObject->m_bLastFrameUsed = true;
	}

	DWORD tmCur = timeGetTime();
	//为了执行效率所以未把下面的判断放在CheckBalance函数体里面。下同。
	if (m_fCacheMemUsed > m_nBalanceNum && 
		(tmCur <= m_tmLastCheckBalance || (tmCur - m_tmLastCheckBalance) > ISBP_CHECK_POINT_DEF))
	{
		m_tmLastCheckBalance = tmCur;
		CheckBalance();
	}
	
	if(pObject && nType == ISI_T_SPR)
	{
//		if(!((ImageResSpr*)pObject)->PrepareFrameData(pszImage, nFrame))
//			return NULL;
	}

	return pObject;
}

int KImageStore::GetImagePixelAlpha( const char* pszImage, int nFrame, int nX, int nY, int nType)
{
	if (!pszImage || !pszImage[0])
		return 0;

	int nRet = 0;

	unsigned int uImage = 0;
	int	nPos = 0;
	void* pTemp;

	switch(nType)
	{
	case ISI_T_SPR:
		pTemp = GetImage(pszImage, uImage, nPos, nFrame, ISI_T_SPR);
		if (!pTemp)
			break;
		nRet = ((ImageResSpr *)pTemp)->GetPixelAlpha(nFrame, nX, nY);	// 在Spr的方法里已经处理了范围
		break;
	case ISI_T_BITMAP16:
		pTemp = GetImage(pszImage, uImage, nPos, nFrame, ISI_T_BITMAP16);
		if (!pTemp)
			break;
		if (nX < 0 || nY < 0 || nX >= ((ImageResBmp *)pTemp)->GetWidth() || nY >= ((ImageResBmp *)pTemp)->GetHeight())
			nRet = 0;
		else
			nRet = 255;
		break;
	default:
		break;
	}
	
	return nRet;
}

KImageRes* KImageStore::LoadImage( const char* pszImageFile, unsigned int nType) const
{
	KImageRes* pRet = NULL;
	switch(nType)
	{
	case ISI_T_SPR:
		pRet = new ImageResSpr;
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
			pRet = new ImageResBmp;
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
