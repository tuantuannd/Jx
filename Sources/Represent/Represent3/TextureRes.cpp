#include "precompile.h"
#include "d3d_device.h"
#include "d3d_utils.h"
#include "TextureRes.h"
#include "JpgLib.h"

inline void RenderToA4R4G4B4(WORD* pDest, BYTE* pSrc, int width, int height, BYTE* pPalette)
{
	__asm
	{
		
		//设置源与目的地址至esi和edi
		mov		edi, pDest
		mov		esi, pSrc
			
loc_DrawSprite_0100:
		
		//edx存放宽度	
		mov		edx, width
			
loc_DrawSprite_0101:
		
		//压缩格式为 n个alpha [rgba,rgba,.......],alpha为0时，无rgba各值
		
		movzx	eax, byte ptr[esi]//ax存放n
		inc		esi
		movzx	ebx, byte ptr[esi]//bx存放alpha值
		//mov		nAlpha, ebx
		inc		esi
		or		ebx, ebx
		jnz		loc_DrawSprite_0102
		
		push	eax	
		mov		ecx, eax
			
loc_FillZeroAlpha:
		
		//romandou
		
		mov		word ptr[edi], 0
		inc		edi
		inc		edi
		dec		ecx
		jnz     loc_FillZeroAlpha
		pop     eax
		
		sub		edx, eax
		jg		loc_DrawSprite_0101
		dec		height
		jnz		loc_DrawSprite_0100
		jmp		loc_DrawSprite_exit
		
			
			//alpha值不为0时
loc_DrawSprite_0102:
		
		push	eax // n值
		push	edx // 当前行还剩多长未处理 

		and		bx, 0x00f0
		shl		bx, 8
		push	ebx	// nAlpha值
		
		mov		ecx, eax
		
			
loc_DrawSprite_0103:
		
		mov     ebx, pPalette
		movzx	eax, byte ptr[esi]
		inc		esi
		mov		dx, [ebx + eax * 2]  //获得 16位 r4g4b4a4 a = 0
		
		pop     ebx   //ebx = nAlpha
		push	ebx  
		
		or		dx, bx //生成最终的r4g4b4a4值
		mov		[edi], dx
		inc		edi
		inc		edi
		dec		ecx
		jnz		loc_DrawSprite_0103
		pop     ebx   //ebx = nAlpha
		pop		edx   //edx = 该行剩于
		pop		eax   //eax = n
		sub		edx, eax
		jg		loc_DrawSprite_0101
		dec		height
		jnz		loc_DrawSprite_0100
		jmp		loc_DrawSprite_exit
			
loc_DrawSprite_exit:
	}
}

TextureRes::TextureRes()
{
}

TextureRes::~TextureRes()
{
}

// 释放内存
void TextureRes::Release()
{
}

//***********************************************************
TextureResBmp::TextureResBmp()
{
	m_FrameInfo.texInfo[0].pTexture = NULL;
	m_pSysMemTexture = NULL;
	ResetVar();
}

TextureResBmp::~TextureResBmp()
{
	Release();
}

// 将成员变量置为初始值
void TextureResBmp::ResetVar()
{
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_eTextureFormat = D3DFMT_R8G8B8;

	m_nTexMemUsed = 0;
	m_nSprMemUsed = 0;
	m_bLastFrameUsed = false;
}

bool TextureResBmp::InvalidateDeviceObjects()
{
	SAFE_RELEASE(m_FrameInfo.texInfo[0].pTexture);
	return true;
}

bool TextureResBmp::RestoreDeviceObjects()
{
	if(!m_FrameInfo.texInfo[0].pTexture)
	{
		if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_FrameInfo.texInfo[0].nWidth, m_FrameInfo.texInfo[0].nHeight, 1,
								D3DUSAGE_RENDERTARGET , g_16BitFormat, D3DPOOL_DEFAULT, &m_FrameInfo.texInfo[0].pTexture)))
			return false;
	}

	if(m_pSysMemTexture)
	{
		m_pSysMemTexture->AddDirtyRect(NULL);
		PD3DDEVICE->UpdateTexture(m_pSysMemTexture, m_FrameInfo.texInfo[0].pTexture);
	}
	
	return true;
}

// 创建内存资源
bool TextureResBmp::CreateImage(const char* szImage, int nWidth, int nHeight, uint32 nType)
{
	if (!szImage || !szImage[0])
		return false;

	if (nWidth < 0 || nWidth > MAX_TEXTURE_SIZE || nHeight < 0 || nHeight > MAX_TEXTURE_SIZE)
		return false;

	if (nType != ISI_T_BITMAP16)
		return false;

	Release();

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nType = nType;

	m_FrameInfo.nTexNum = 1;
	m_FrameInfo.texInfo[0].nWidth = FitTextureSize(nWidth);
	m_FrameInfo.texInfo[0].nHeight = FitTextureSize(nHeight);

	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_FrameInfo.texInfo[0].nWidth, m_FrameInfo.texInfo[0].nHeight, 1,
								D3DUSAGE_RENDERTARGET , g_16BitFormat, D3DPOOL_DEFAULT, &m_FrameInfo.texInfo[0].pTexture)))
		goto error;
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_FrameInfo.texInfo[0].nWidth, m_FrameInfo.texInfo[0].nHeight, 1,
								0 , g_16BitFormat, D3DPOOL_SYSTEMMEM, &m_pSysMemTexture)))
		goto error;

	m_nTexMemUsed = m_FrameInfo.texInfo[0].nWidth * m_FrameInfo.texInfo[0].nHeight * 2;
	return true;

error:
	Release();
	return false;
}

// 从文件载入资源
bool TextureResBmp::LoadImage(char* szImage, uint32 nType)
{
	if (!szImage || !szImage[0] || nType != ISI_T_BITMAP16)
		return false;

	Release();

	m_nType = nType;

	if(!LoadJpegFile(szImage))
			return false;

	return true;
}

bool TextureResBmp::LoadJpegFile(char* szImage)
{
	KSGImageContent *pImageContent;
	int i;
	BYTE *pDes;
	BYTE *pSrc;
	if(g_16BitFormat == D3DFMT_R5G6B5)
		pImageContent = get_jpg_image(szImage, RGB_565);
	else
		pImageContent = get_jpg_image(szImage, RGB_555);

	if(!pImageContent)
		return false;

	m_nWidth = pImageContent->nWidth;
	m_nHeight = pImageContent->nHeight;

	m_FrameInfo.nWidth = pImageContent->nWidth;
	m_FrameInfo.nHeight = pImageContent->nHeight;
	m_FrameInfo.nTexNum = 1;

	// 获得贴图尺寸，符合2的幂次
	m_FrameInfo.texInfo[0].nWidth = FitTextureSize(m_nWidth);
	m_FrameInfo.texInfo[0].nHeight = FitTextureSize(m_nHeight);

	if(m_FrameInfo.texInfo[0].nWidth == 0 || m_FrameInfo.texInfo[0].nHeight == 0)
		goto error;

	// 创建贴图
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_FrameInfo.texInfo[0].nWidth, m_FrameInfo.texInfo[0].nHeight, 1,
								D3DUSAGE_RENDERTARGET, g_16BitFormat, D3DPOOL_DEFAULT, &m_FrameInfo.texInfo[0].pTexture)))
		goto error;
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_FrameInfo.texInfo[0].nWidth, m_FrameInfo.texInfo[0].nHeight, 1,
								0, g_16BitFormat, D3DPOOL_SYSTEMMEM, &m_pSysMemTexture)))
		goto error;

	D3DLOCKED_RECT LockedRect;
	if (FAILED(m_pSysMemTexture->LockRect(0, &LockedRect, NULL, 0)))
		goto error;

	pDes = (BYTE*)LockedRect.pBits;
	pSrc = (BYTE*)pImageContent->Data;
	for(i=0; i<m_nHeight; i++)
	{
		memcpy(pDes, pSrc, m_nWidth * 2);
		pSrc += m_nWidth * 2;
		pDes += LockedRect.Pitch;
	}

	release_image(pImageContent);
	m_pSysMemTexture->UnlockRect(0);

	PD3DDEVICE->UpdateTexture(m_pSysMemTexture, m_FrameInfo.texInfo[0].pTexture);

	m_nTexMemUsed = m_FrameInfo.texInfo[0].nWidth * m_FrameInfo.texInfo[0].nHeight * 2;
	return true;

error:
	release_image(pImageContent);
	Release();
	return false;
}

bool TextureResBmp::LockData(void** pData, int32& nPitch)
{
	if(!m_pSysMemTexture)
		return false;

	D3DLOCKED_RECT LockedRect;
	if (FAILED(m_pSysMemTexture->LockRect(0, &LockedRect, NULL, 0)))
		return false;

	*pData = LockedRect.pBits;
	nPitch = LockedRect.Pitch;

	return true;
}

void TextureResBmp::UnLockData()
{
	if(!m_pSysMemTexture)
		return;

	m_pSysMemTexture->UnlockRect(0);
	PD3DDEVICE->UpdateTexture(m_pSysMemTexture, m_FrameInfo.texInfo[0].pTexture);
}

// 释放内存
void TextureResBmp::Release()
{
	SAFE_RELEASE(m_FrameInfo.texInfo[0].pTexture);
	SAFE_RELEASE(m_pSysMemTexture);
	ResetVar();
}

//*************************************************************

TextureResSpr::TextureResSpr()
{
	ResetVar();
}

TextureResSpr::~TextureResSpr()
{
	Release();
}

// 将成员变量置为初始值
void TextureResSpr::ResetVar()
{
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_nFrameNum		= 1;
	m_nCenterX		= 0;
	m_nCenterY		= 0;
	m_nColors		= 0;
	m_nDirections	= 1;
	m_nInterval		= 1;

	m_pPal24		= NULL;
	m_pPal16		= NULL;
	m_pFrameInfo	= NULL;
	m_pHeader		= NULL;
	m_pOffset		= NULL;

	m_nTexMemUsed = 0;
	m_bLastFrameUsed = false;
}

// 创建内存资源
bool TextureResSpr::CreateImage(const char* szImage, int nWidth, int nHeight, uint32 nType)
{
	return false;
}

// 从文件载入资源
bool TextureResSpr::LoadImage(char* szImage, uint32 nType)
{
	if (!szImage || !szImage[0] || nType != ISI_T_SPR)
		return false;

	Release();

	m_nType = nType;

	if(!LoadSprFile(szImage))
		return false;

	return true;
}

bool TextureResSpr::LoadSprFile(char* szImage)
{
	assert(szImage);
	if(!szImage)
		return false;

	int i;
	PBYTE palette;
	char *pFrameData;
	SPRFRAME *pFrame;
	SPROFFS *pOffset;
	PBYTE pSprite;
	SPRHEAD *pHeader = (SPRHEAD *)SprGetHeader(szImage, (SPROFFS*&)pFrameData);

	if(!pHeader)
		return false;
	palette = ((PBYTE)pHeader);
	palette += sizeof(SPRHEAD);

	// setup palette pointer
	m_pPal24 = new KPAL24[pHeader->Colors];
	memcpy(m_pPal24, palette, pHeader->Colors * sizeof(KPAL24));

	m_pPal16 = new WORD[pHeader->Colors];
	for(i=0; i<pHeader->Colors ; i++)
	{
		m_pPal16[i] = ARGBTO4444(0, m_pPal24[i].Red, m_pPal24[i].Green, m_pPal24[i].Blue);
	}

	m_nWidth		= pHeader->Width;
	m_nHeight		= pHeader->Height;
	m_nCenterX		= pHeader->CenterX;
	m_nCenterY		= pHeader->CenterY;
	m_nColors		= pHeader->Colors;
	m_nDirections	= pHeader->Directions;
	m_nInterval		= pHeader->Interval;
	m_nFrameNum		= pHeader->Frames;
	if(m_nFrameNum == 0)
		goto error;

	m_pFrameInfo = new FrameToTexture[m_nFrameNum];
	ZeroMemory(m_pFrameInfo, sizeof(FrameToTexture) * m_nFrameNum);

	if(pFrameData)
	{
		// 整体压缩，拷贝所有帧的原始数据
		pSprite = (LPBYTE)pFrameData + m_nFrameNum * sizeof(SPROFFS);
		pOffset = (SPROFFS*)pFrameData;
		for(i=0; i<m_nFrameNum; i++)
		{
			pFrame = (SPRFRAME*)(pSprite + pOffset[i].Offset); 
			m_pFrameInfo[i].nWidth = pFrame->Width;
			m_pFrameInfo[i].nHeight = pFrame->Height;
			m_pFrameInfo[i].nTexNum = 0;
			m_pFrameInfo[i].nOffX = pFrame->OffsetX;
			m_pFrameInfo[i].nOffY = pFrame->OffsetY;
			m_pFrameInfo[i].nRawDataLen = pOffset[i].Length - 8;//sizeof(SPRFRAME);
			m_pFrameInfo[i].pRawData = new BYTE[m_pFrameInfo[i].nRawDataLen];
			memcpy(m_pFrameInfo[i].pRawData, pFrame->Sprite, m_pFrameInfo[i].nRawDataLen);
			m_pFrameInfo[i].pFrame = NULL;
		}
		SprReleaseHeader(pHeader);
	}
	else
	{
		// 按帧压缩，记录Header指针，以后传回SprGetFrame
		m_pHeader = (char*)pHeader;
		m_pOffset = (SPROFFS*)(m_pHeader + sizeof(SPRHEAD) + m_nColors * sizeof(KPAL24));
	}
	
	return true;

error:
	SprReleaseHeader(pHeader);
	Release();
	return false;
}

bool TextureResSpr::PrepareFrameData(const char* szImage, int32 nFrame, bool bPrepareTex)
{
	assert(szImage);
	if(!szImage)
		return false;
	if(nFrame < 0 || nFrame >= m_nFrameNum)
		return false;

	if(m_pFrameInfo[nFrame].texInfo[0].pTexture)
		return true;

	if(!m_pFrameInfo[nFrame].pRawData)
	{
		assert(m_pHeader);
		if(!m_pHeader)
			return false;

		SPRFRAME *pFrame = (SPRFRAME *)SprGetFrame((SPRHEAD*)m_pHeader, nFrame);

		if(!pFrame)
			return false;

		m_pFrameInfo[nFrame].nWidth = pFrame->Width;
		m_pFrameInfo[nFrame].nHeight = pFrame->Height;
		m_pFrameInfo[nFrame].nTexNum = 0;
		m_pFrameInfo[nFrame].nOffX = pFrame->OffsetX;
		m_pFrameInfo[nFrame].nOffY = pFrame->OffsetY;

        if (((int)m_pOffset[nFrame].Length) < 0)
            m_pOffset[nFrame].Length = -((int)m_pOffset[nFrame].Length);
	
		m_pFrameInfo[nFrame].nRawDataLen = m_pOffset[nFrame].Length - 8;//sizeof(SPRFRAME);
		m_pFrameInfo[nFrame].pRawData = pFrame->Sprite;
		m_pFrameInfo[nFrame].pFrame = pFrame;
	}

	if(bPrepareTex)
		CreateTexture16Bit(szImage, nFrame);

	return true;
}

LPDIRECT3DTEXTURE9 TextureResSpr::GetTexture(int32 nFrame, int nIdx)
{
	assert(m_pFrameInfo[nFrame].nTexNum);
	assert(nIdx >= 0 && nIdx < m_pFrameInfo[nFrame].nTexNum);
	if(nFrame < 0 || nFrame >= m_nFrameNum)
		return NULL;

	return m_pFrameInfo[nFrame].texInfo[nIdx].pTexture;
}

void TextureResSpr::CreateTexture16Bit(const char* szImage, int32 nFrame)
{
	int i, j;

	if(nFrame < 0 || nFrame >= m_nFrameNum)
		return;

	if(m_pFrameInfo[nFrame].texInfo[0].pTexture)
		return;

	SplitTexture(nFrame);

	BYTE *pTempData = new BYTE[m_pFrameInfo[nFrame].nWidth * m_pFrameInfo[nFrame].nHeight * 2];
	if(!pTempData)
		return;

	RenderToA4R4G4B4((WORD*)pTempData, m_pFrameInfo[nFrame].pRawData, m_pFrameInfo[nFrame].nWidth,
						m_pFrameInfo[nFrame].nHeight, (BYTE*)m_pPal16);

/*
	// 填充贴图数据
	uint32 nByteCount = 0;							// 总字节计数
	uint32 nWidth = m_pFrameInfo[nFrame].nWidth;
	uint32 nHeight = m_pFrameInfo[nFrame].nHeight;
	uint32 nFrameLen = m_pFrameInfo[nFrame].nRawDataLen;
	BYTE *pData = m_pFrameInfo[nFrame].pRawData;
	BYTE pixelNum, alpha, pixelColor;
	BYTE *pTexLine = pTempData;						// 贴图每一行数据
	for(;;)
	{
		pixelNum = *pData++;
		alpha = *pData++;
		nByteCount += 2;

		if(alpha == 0)
		{
			for(int l=0; l<pixelNum; l++)
			{
				*((WORD*)pTexLine) = 0;
				pTexLine += 2;
			}
		}
		else
		{
			for(int l=0; l<pixelNum; l++)
			{
				pixelColor = *pData++;
			//	*((WORD*)pTexLine) = ARGBTO4444(alpha, m_pPal24[pixelColor].Red, m_pPal24[pixelColor].Green, m_pPal24[pixelColor].Blue);
				*((WORD*)pTexLine) = ((((WORD)alpha)<<8)&0xf000) | m_pPal16[pixelColor];
				pTexLine += 2;
			}
			nByteCount += pixelNum;
		}

		// 如果一帧数据解完则停止
		assert(nByteCount <= nFrameLen);
		if(nByteCount >= nFrameLen)
			break;
	}
*/
	// 创建并填充贴图
	for(i=0; i<m_pFrameInfo[nFrame].nTexNum; i++)
	{
		m_nTexMemUsed += m_pFrameInfo[nFrame].texInfo[i].nWidth * m_pFrameInfo[nFrame].texInfo[i].nHeight * 2;

		// 创建贴图
		SAFE_RELEASE(m_pFrameInfo[nFrame].texInfo[i].pTexture);
		if (FAILED(PD3DDEVICE->CreateTexture(m_pFrameInfo[nFrame].texInfo[i].nWidth, m_pFrameInfo[nFrame].texInfo[i].nHeight, 1,
								0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pFrameInfo[nFrame].texInfo[i].pTexture, NULL)))
			goto error;

		D3DLOCKED_RECT LockedRect;
		if (FAILED(m_pFrameInfo[nFrame].texInfo[i].pTexture->LockRect(0, &LockedRect, NULL, 0)))
			goto error;

		// 向贴图拷贝数据
		BYTE *pTexData = (BYTE*)LockedRect.pBits;		// 贴图数据
		BYTE *pTp = pTempData + (m_pFrameInfo[nFrame].texInfo[i].nFrameY * m_pFrameInfo[nFrame].nWidth + 
					m_pFrameInfo[nFrame].texInfo[i].nFrameX) * 2;
		for(j=0; j<m_pFrameInfo[nFrame].texInfo[i].nFrameHeight; j++)
		{
			memcpy(pTexData, pTp, m_pFrameInfo[nFrame].texInfo[i].nFrameWidth * 2);
			pTexData += LockedRect.Pitch;
			pTp += m_pFrameInfo[nFrame].nWidth * 2;
		}

		m_pFrameInfo[nFrame].texInfo[i].pTexture->UnlockRect(0);
	}

	SAFE_DELETE_ARRAY(pTempData);
	if(m_pHeader)
	{
		SprReleaseFrame((SPRFRAME*)m_pFrameInfo[nFrame].pFrame);
		m_pFrameInfo[nFrame].pFrame = NULL;
		m_pFrameInfo[nFrame].pRawData = NULL;
	}
	return;

error:
	SAFE_DELETE_ARRAY(pTempData);
	return;
}

/*抽点
LPDIRECT3DTEXTURE8 TextureResSpr::CreateTexture16Bit(uint32 nFrame)
{
	if(nFrame >= m_nFrameNum)
		return NULL;

	if(m_pTextureInfo[nFrame].pTexture)
		return m_pTextureInfo[nFrame].pTexture;

	if(!m_pFrameInfo[nFrame].pRawData)
		return NULL;

	// 创建贴图
	if (FAILED(PD3DDEVICE->CreateTexture(m_pTextureInfo[nFrame].nWidth, m_pTextureInfo[nFrame].nHeight, 1,
								0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pTextureInfo[nFrame].pTexture)))
		return NULL;

	D3DLOCKED_RECT LockedRect;
	if (FAILED(m_pTextureInfo[nFrame].pTexture->LockRect(0, &LockedRect, NULL, 0)))
	{
		SAFE_RELEASE(m_pTextureInfo[nFrame].pTexture);
		return NULL;
	}

	m_nTexMemUsed += m_pTextureInfo[nFrame].nWidth * m_pTextureInfo[nFrame].nHeight * 2;
	m_nMemUseful += m_pFrameInfo[nFrame].nWidth * m_pFrameInfo[nFrame].nHeight * 2;

	// 填充贴图数据
	uint32 nByteCount = 0;							// 总字节计数
	uint32 nPixelCount = 0;							// 总象素计数
	uint32 nWidth = m_pFrameInfo[nFrame].nWidth;
	uint32 nHeight = m_pFrameInfo[nFrame].nHeight;
	uint32 nFrameLen = m_pFrameInfo[nFrame].nRawDataLen;
	BYTE *pData = m_pFrameInfo[nFrame].pRawData;
	BYTE pixelNum, alpha, pixelColor;
	BYTE *pTexData = (BYTE*)LockedRect.pBits;		// 贴图数据
	BYTE *pTexLine = pTexData;						// 贴图每一行数据
	int nTp2 = 0;
	for(;;)
	{
		pixelNum = *pData++;
		alpha = *pData++;
		nByteCount += 2;
		nPixelCount += pixelNum;

		if(alpha == 0)
		{
			if((nTp2%2) == 1)
			for(int l=0; l<pixelNum; l++)
			{

				*((WORD*)pTexLine) = 0;
				pTexLine += 2;
			}
		}
		else
		{
			if((nTp2%2) == 1)
			for(int l=0; l<pixelNum; l++)
			{
				pixelColor = *pData++;
			//	*((WORD*)pTexLine) = ARGBTO4444(alpha, m_pPal24[pixelColor].Red, m_pPal24[pixelColor].Green, m_pPal24[pixelColor].Blue);
				*((WORD*)pTexLine) = ((((WORD)alpha)<<8)&0xf000) | m_pPal16[pixelColor];
				pTexLine += 2;
			}
			else
				pData += pixelNum;
			nByteCount += pixelNum;
		}

		if(nPixelCount == nWidth)
		{
			// 如果一帧数据解完则停止
			if(nByteCount >= nFrameLen)
				break;
			if((nTp2%2) == 1)
				pTexLine = pTexData = pTexData + LockedRect.Pitch;
			nPixelCount = 0;
			nTp2++;
		}
	}

	m_pTextureInfo[nFrame].pTexture->UnlockRect(0);

	SAFE_DELETE_ARRAY(m_pFrameInfo[nFrame].pRawData);
	return m_pTextureInfo[nFrame].pTexture;
}
*/

void TextureResSpr::CreateTexture32Bit(uint32 nFrame)
{
	return;
/*	if(nFrame >= m_nFrameNum)
		return NULL;

	if(m_pTextureInfo[nFrame].pTexture)
		return m_pTextureInfo[nFrame].pTexture;

	if(!m_pFrameInfo[nFrame].pRawData)
		return NULL;

	// 创建贴图
	if (FAILED(PD3DDEVICE->CreateTexture(m_pTextureInfo[nFrame].nWidth, m_pTextureInfo[nFrame].nHeight, 1,
								0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_pTextureInfo[nFrame].pTexture)))
		return NULL;

	D3DSURFACE_DESC desc;
	m_pTextureInfo[nFrame].pTexture->GetLevelDesc(0, &desc);

	D3DLOCKED_RECT LockedRect;
	if (FAILED(m_pTextureInfo[nFrame].pTexture->LockRect(0, &LockedRect, NULL, 0)))
	{
		SAFE_RELEASE(m_pTextureInfo[nFrame].pTexture);
		return NULL;
	}

	m_nTexMemUsed += m_pTextureInfo[nFrame].nWidth * m_pTextureInfo[nFrame].nHeight * 4;
	m_nMemUseful += m_pFrameInfo[nFrame].nWidth * m_pFrameInfo[nFrame].nHeight * 4;

	// 填充贴图数据
	uint32 nByteCount = 0;							// 总字节计数
	uint32 nPixelCount = 0;							// 总象素计数
	uint32 nWidth = m_pFrameInfo[nFrame].nWidth;
	uint32 nHeight = m_pFrameInfo[nFrame].nHeight;
	uint32 nTotlePixelNum = nWidth * nHeight;						// 总象素数目
	uint32 nFrameLen = m_pFrameInfo[nFrame].nRawDataLen;
	BYTE *pData = m_pFrameInfo[nFrame].pRawData;
	BYTE pixelNum, alpha, pixelColor;
	BYTE *pTexData = (BYTE*)LockedRect.pBits;		// 贴图数据
	BYTE *pTexLine = pTexData;						// 贴图每一行数据
	for(;;)
	{
		pixelNum = *pData++;
		alpha = *pData++;
		nByteCount += 2;
		nPixelCount += pixelNum;

		if(alpha == 0)
		{
			for(int l=0; l<pixelNum; l++)
			{
				*((DWORD*)pTexLine) = 0;
				pTexLine += 4;
			}
		}
		else
		{
			for(int l=0; l<pixelNum; l++)
			{
				pixelColor = *pData++;
				*pTexLine++ = m_pPal24[pixelColor].Blue;
				*pTexLine++ = m_pPal24[pixelColor].Green;
				*pTexLine++ = m_pPal24[pixelColor].Red;
				*pTexLine++ = alpha;
			}
			nByteCount += pixelNum;
		}

		if(nPixelCount == nWidth)
		{
			// 如果一帧数据解完则停止
			if(nByteCount >= nFrameLen)
				break;
			pTexLine = pTexData = pTexData + LockedRect.Pitch;
			nPixelCount = 0;
		}
	}

	m_pTextureInfo[nFrame].pTexture->UnlockRect(0);

	SAFE_DELETE_ARRAY(m_pFrameInfo[nFrame].pRawData);
	return m_pTextureInfo[nFrame].pTexture;*/
}

/*压缩
LPDIRECT3DTEXTURE8 TextureResSpr::CreateTexture16Bit(uint32 nFrame)
{
	if(nFrame >= m_nFrameNum)
		return NULL;

	if(m_pTextureInfo[nFrame].pTexture)
		return m_pTextureInfo[nFrame].pTexture;

	if(!m_pFrameInfo[nFrame].pRawData)
		return NULL;

	LPDIRECT3DTEXTURE8 pTexture = NULL;

	// 创建贴图
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_pTextureInfo[nFrame].nWidth, m_pTextureInfo[nFrame].nHeight, 1,
								0, D3DFMT_A4R4G4B4, D3DPOOL_SYSTEMMEM, &pTexture)))
		return NULL;

	D3DLOCKED_RECT LockedRect;
	if (FAILED(pTexture->LockRect(0, &LockedRect, NULL, 0)))
	{
		SAFE_RELEASE(pTexture);
		return NULL;
	}

	m_nTexMemUsed += m_pTextureInfo[nFrame].nWidth * m_pTextureInfo[nFrame].nHeight * 2;
	m_nMemUseful += m_pFrameInfo[nFrame].nWidth * m_pFrameInfo[nFrame].nHeight * 2;

	// 填充贴图数据
	uint32 nByteCount = 0;							// 总字节计数
	uint32 nPixelCount = 0;							// 总象素计数
	uint32 nWidth = m_pFrameInfo[nFrame].nWidth;
	uint32 nHeight = m_pFrameInfo[nFrame].nHeight;
	uint32 nFrameLen = m_pFrameInfo[nFrame].nRawDataLen;
	BYTE *pData = m_pFrameInfo[nFrame].pRawData;
	BYTE pixelNum, alpha, pixelColor;
	BYTE *pTexData = (BYTE*)LockedRect.pBits;		// 贴图数据
	BYTE *pTexLine = pTexData;						// 贴图每一行数据
//	uint32 nTotlePixelNum = m_pTextureInfo[nFrame].nWidth * m_pTextureInfo[nFrame].nHeight;	
	for(;;)
	{
		pixelNum = *pData++;
		alpha = *pData++;
		nByteCount += 2;
		nPixelCount += pixelNum;

		// 如果超出贴图数据范围则停止
//		if(nPixelCount > nTotlePixelNum)
//			break;

		if(alpha == 0)
		{
			for(int l=0; l<pixelNum; l++)
			{
				*((WORD*)pTexLine) = 0;
				pTexLine += 2;
			}
		}
		else
		{
			for(int l=0; l<pixelNum; l++)
			{
				pixelColor = *pData++;
				//*((WORD*)pTexLine) = ARGBTO4444(alpha, m_pPal24[pixelColor].Red, m_pPal24[pixelColor].Green, m_pPal24[pixelColor].Blue);
				*((WORD*)pTexLine) = ((((WORD)alpha)<<8)&0xf000) | m_pPal16[pixelColor];
				pTexLine += 2;
			}
			nByteCount += pixelNum;
		}

		if(nPixelCount % nWidth == 0)
		{
			// 如果一帧数据解完则停止
			if(nByteCount >= nFrameLen)
				break;
			pTexLine = pTexData = pTexData + LockedRect.Pitch;
		//	nPixelCount = 0;
		}
	}

	pTexture->UnlockRect(0);

	IDirect3DSurface8* pSrcSurface = NULL, *pDesSurface = NULL;

	// 创建贴图
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_pTextureInfo[nFrame].nWidth, m_pTextureInfo[nFrame].nHeight, 1,
								0, D3DFMT_DXT5, D3DPOOL_MANAGED, &m_pTextureInfo[nFrame].pTexture)))
		goto error;

	if ( pTexture->GetSurfaceLevel( 0, &pSrcSurface) != D3D_OK )
		goto error;
	if ( m_pTextureInfo[nFrame].pTexture->GetSurfaceLevel( 0, &pDesSurface) != D3D_OK )
		goto error;

	D3DXLoadSurfaceFromSurface( pDesSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_FILTER_NONE, 0);

	SAFE_RELEASE(pSrcSurface);
	SAFE_RELEASE(pDesSurface);

	SAFE_DELETE_ARRAY(m_pFrameInfo[nFrame].pRawData);
	SAFE_RELEASE(pTexture);
	return m_pTextureInfo[nFrame].pTexture;

error:
	SAFE_RELEASE(pSrcSurface);
	SAFE_RELEASE(pDesSurface);
	SAFE_RELEASE(pTexture);
	SAFE_RELEASE(m_pTextureInfo[nFrame].pTexture);
	return NULL;
}

LPDIRECT3DTEXTURE8 TextureResSpr::CreateTexture32Bit(uint32 nFrame)
{
	if(nFrame >= m_nFrameNum)
		return NULL;

	if(m_pTextureInfo[nFrame].pTexture)
		return m_pTextureInfo[nFrame].pTexture;

	if(!m_pFrameInfo[nFrame].pRawData)
		return NULL;

	LPDIRECT3DTEXTURE8 pTexture = NULL;

	// 创建贴图
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_pTextureInfo[nFrame].nWidth, m_pTextureInfo[nFrame].nHeight, 1,
								0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pTexture)))
		return NULL;

	D3DLOCKED_RECT LockedRect;
	if (FAILED(pTexture->LockRect(0, &LockedRect, NULL, 0)))
	{
		SAFE_RELEASE(pTexture);
		return NULL;
	}

	m_nTexMemUsed += m_pTextureInfo[nFrame].nWidth * m_pTextureInfo[nFrame].nHeight * 4;
	m_nMemUseful += m_pFrameInfo[nFrame].nWidth * m_pFrameInfo[nFrame].nHeight * 4;

	// 填充贴图数据
	uint32 nByteCount = 0;							// 总字节计数
	uint32 nPixelCount = 0;							// 总象素计数
	uint32 nWidth = m_pFrameInfo[nFrame].nWidth;
	uint32 nHeight = m_pFrameInfo[nFrame].nHeight;
	uint32 nTotlePixelNum = nWidth * nHeight;						// 总象素数目
	uint32 nFrameLen = m_pFrameInfo[nFrame].nRawDataLen;
	BYTE *pData = m_pFrameInfo[nFrame].pRawData;
	BYTE pixelNum, alpha, pixelColor;
	BYTE *pTexData = (BYTE*)LockedRect.pBits;		// 贴图数据
	BYTE *pTexLine = pTexData;						// 贴图每一行数据
	for(;;)
	{
		pixelNum = *pData++;
		alpha = *pData++;
		nByteCount += 2;
		nPixelCount += pixelNum;

		if(alpha == 0)
		{
			for(int l=0; l<pixelNum; l++)
			{
				*((DWORD*)pTexLine) = 0;
				pTexLine += 4;
			}
		}
		else
		{
			for(int l=0; l<pixelNum; l++)
			{
				pixelColor = *pData++;
				*pTexLine++ = m_pPal24[pixelColor].Blue;
				*pTexLine++ = m_pPal24[pixelColor].Green;
				*pTexLine++ = m_pPal24[pixelColor].Red;
				*pTexLine++ = alpha;
			}
			nByteCount += pixelNum;
		}

		if(nPixelCount == nWidth)
		{
			// 如果一帧数据解完则停止
			if(nByteCount >= nFrameLen)
				break;
			pTexLine = pTexData = pTexData + LockedRect.Pitch;
			nPixelCount = 0;
		}
	}

	pTexture->UnlockRect(0);

	IDirect3DSurface8* pSrcSurface = NULL, *pDesSurface = NULL;

	// 创建贴图
	if (FAILED(D3DXCreateTexture(PD3DDEVICE, m_pTextureInfo[nFrame].nWidth, m_pTextureInfo[nFrame].nHeight, 1,
								0, D3DFMT_DXT5, D3DPOOL_MANAGED, &m_pTextureInfo[nFrame].pTexture)))
		goto error;

	if ( pTexture->GetSurfaceLevel( 0, &pSrcSurface) != D3D_OK )
		goto error;
	if ( m_pTextureInfo[nFrame].pTexture->GetSurfaceLevel( 0, &pDesSurface) != D3D_OK )
		goto error;

	D3DXLoadSurfaceFromSurface( pDesSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_FILTER_NONE, 0);

	SAFE_RELEASE(pSrcSurface);
	SAFE_RELEASE(pDesSurface);

	SAFE_DELETE_ARRAY(m_pFrameInfo[nFrame].pRawData);
	SAFE_RELEASE(pTexture);
	return m_pTextureInfo[nFrame].pTexture;

error:
	SAFE_RELEASE(pSrcSurface);
	SAFE_RELEASE(pDesSurface);
	SAFE_RELEASE(pTexture);
	SAFE_RELEASE(m_pTextureInfo[nFrame].pTexture);
	return NULL;
}
*/
int32 TextureResSpr::GetPixelAlpha(int32 nFrame, int32 x, int32 y)
{
	int32 nRet = 0;

	if (nFrame < 0 && nFrame >= m_nFrameNum)
		return nRet;

	x -= m_pFrameInfo[nFrame].nOffX;
	y -= m_pFrameInfo[nFrame].nOffY;

	// 坐标超出图形范围则返回
	if (x < 0 || y < 0 || x >= m_pFrameInfo[nFrame].nWidth || 
		y >= m_pFrameInfo[nFrame].nHeight)
		return nRet;

	// 遍历拆分的贴图，判断象素点alpha值
	for(int i=0; i<m_pFrameInfo[nFrame].nTexNum; i++)
	{
		int32 tx = x - m_pFrameInfo[nFrame].texInfo[i].nFrameX;
		int32 ty = y - m_pFrameInfo[nFrame].texInfo[i].nFrameY;
		// 检测象素点是否落在第i张贴图上
		if (tx >= 0 && ty >= 0 && tx < m_pFrameInfo[nFrame].texInfo[i].nFrameWidth && 
									ty < m_pFrameInfo[nFrame].texInfo[i].nFrameHeight)
		{
			if(!m_pFrameInfo[nFrame].texInfo[i].pTexture)
				return nRet;

			D3DLOCKED_RECT LockedRect;
			if (FAILED(m_pFrameInfo[nFrame].texInfo[i].pTexture->LockRect(0, &LockedRect, NULL, D3DLOCK_READONLY)))
				return nRet;

			if(g_bUse4444Texture)
			{
				BYTE *p = (BYTE*)LockedRect.pBits;
				p += ty * LockedRect.Pitch + tx * 2 + 1;
				nRet = (*p) & 0xf0;
			}
			else
			{
				BYTE *p = (BYTE*)LockedRect.pBits;
				p += ty * LockedRect.Pitch + tx * 4 + 3;
				nRet = *p;
			}

			m_pFrameInfo[nFrame].texInfo[i].pTexture->UnlockRect(0);
			break;
		}
	}
	
	return nRet;
}

// 释放内存
void TextureResSpr::Release()
{
	SAFE_DELETE_ARRAY(m_pPal24);
	SAFE_DELETE_ARRAY(m_pPal16);

	// 释放所有贴图和spr数据
	if(m_pFrameInfo)
	{
		for(int i=0; i<m_nFrameNum; i++)
		{
			if(m_pHeader)
			{
				if(m_pFrameInfo[i].pFrame)
				{
					SprReleaseFrame((SPRFRAME*)m_pFrameInfo[i].pFrame);
					m_pFrameInfo[i].pFrame = NULL;
					m_pFrameInfo[i].pRawData = NULL;
				}
			}
			else
				SAFE_DELETE_ARRAY(m_pFrameInfo[i].pRawData);
			// 释放第i帧的贴图
			if(m_pFrameInfo[i].nTexNum)
			{
				SAFE_RELEASE(m_pFrameInfo[i].texInfo[0].pTexture);
				SAFE_RELEASE(m_pFrameInfo[i].texInfo[1].pTexture);
				SAFE_RELEASE(m_pFrameInfo[i].texInfo[2].pTexture);
				SAFE_RELEASE(m_pFrameInfo[i].texInfo[3].pTexture);
			}
		}
	}
	
	SAFE_DELETE_ARRAY(m_pFrameInfo);
	if(m_pHeader)
	{
		SprReleaseHeader((SPRHEAD*)m_pHeader);
		m_pHeader = NULL;
	}

	ResetVar();
}

bool TextureResSpr::ReleaseAFrameData()
{
	if(m_pFrameInfo)
	{
		for(int i=0; i<m_nFrameNum; i++)
		{
			// 释放第i帧的贴图
			if(m_pFrameInfo[i].texInfo[0].pTexture)
			{
				for(int j=0; j<4; j++)
				{
					m_nTexMemUsed -= m_pFrameInfo[i].texInfo[j].nWidth * m_pFrameInfo[i].texInfo[j].nHeight * 2;
					SAFE_RELEASE(m_pFrameInfo[i].texInfo[j].pTexture);
				}
				return true;
			}
		}
		return false;
	}
	return false;
}

int TextureResSpr::SplitTexture(uint32 nFrame)
{
	int nWidth = m_pFrameInfo[nFrame].nWidth;
	int nHeight = m_pFrameInfo[nFrame].nHeight;
	// 计算原始贴图尺寸
	int nTexWidth = FitTextureSize(nWidth);
	int nTexHeight = FitTextureSize(nHeight);
	if(nTexWidth == 0 || nTexHeight == 0)
		return 0;

	int nHalfWidth = nTexWidth / 2;
	int nHalfHeight = nTexHeight / 2;
	int nFourThreeWidth = nHalfWidth + nHalfWidth / 2;
	int nFourThreeHeight = nHalfHeight + nHalfHeight / 2;
	int nMem = 0;

	int ntw = FitTextureSize(nWidth - nHalfWidth);
	int nth = FitTextureSize(nHeight - nHalfHeight);

	// 按不同情况将贴图分为4，2，1份，填写相关参数
	if(nWidth > nHalfWidth && nWidth <= nFourThreeWidth && nHeight > nHalfHeight && nHeight <= nFourThreeHeight)
	{
		// 拆为4份
		m_pFrameInfo[nFrame].nTexNum = 4;
		m_pFrameInfo[nFrame].texInfo[0].nWidth = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[0].nHeight = nHalfHeight;
		m_pFrameInfo[nFrame].texInfo[0].nFrameX = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameY = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameWidth = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[0].nFrameHeight = nHalfHeight;

		m_pFrameInfo[nFrame].texInfo[1].nWidth = ntw;
		m_pFrameInfo[nFrame].texInfo[1].nHeight = nHalfHeight;
		m_pFrameInfo[nFrame].texInfo[1].nFrameX = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[1].nFrameY = 0;
		m_pFrameInfo[nFrame].texInfo[1].nFrameWidth = nWidth - nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[1].nFrameHeight = nHalfHeight;

		m_pFrameInfo[nFrame].texInfo[2].nWidth = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[2].nHeight = nth;
		m_pFrameInfo[nFrame].texInfo[2].nFrameX = 0;
		m_pFrameInfo[nFrame].texInfo[2].nFrameY = nHalfHeight;
		m_pFrameInfo[nFrame].texInfo[2].nFrameWidth = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[2].nFrameHeight = nHeight - nHalfHeight;

		m_pFrameInfo[nFrame].texInfo[3].nWidth = ntw;
		m_pFrameInfo[nFrame].texInfo[3].nHeight = nth;
		m_pFrameInfo[nFrame].texInfo[3].nFrameX = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[3].nFrameY = nHalfHeight;
		m_pFrameInfo[nFrame].texInfo[3].nFrameWidth = nWidth - nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[3].nFrameHeight = nHeight - nHalfHeight;

		nMem = ((nHalfWidth + ntw) * nHalfHeight + (nHalfWidth + ntw) * nth) * 2;
	}
	else if(nWidth > nHalfWidth && nWidth <= nFourThreeWidth)
	{
		// 拆为左右两份
		m_pFrameInfo[nFrame].nTexNum = 2;
		m_pFrameInfo[nFrame].texInfo[0].nWidth = nHalfWidth;
		if(nHeight <= nHalfHeight)
			m_pFrameInfo[nFrame].texInfo[0].nHeight = nHalfHeight;
		else
			m_pFrameInfo[nFrame].texInfo[0].nHeight = nTexHeight;
		m_pFrameInfo[nFrame].texInfo[0].nFrameX = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameY = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameWidth = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[0].nFrameHeight = nHeight;

		m_pFrameInfo[nFrame].texInfo[1].nWidth = ntw;
		if(nHeight <= nHalfHeight)
			m_pFrameInfo[nFrame].texInfo[1].nHeight = nHalfHeight;
		else
			m_pFrameInfo[nFrame].texInfo[1].nHeight = nTexHeight;
		m_pFrameInfo[nFrame].texInfo[1].nFrameX = nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[1].nFrameY = 0;
		m_pFrameInfo[nFrame].texInfo[1].nFrameWidth = nWidth - nHalfWidth;
		m_pFrameInfo[nFrame].texInfo[1].nFrameHeight = nHeight;

		nMem = (nHalfWidth + ntw) * nHalfHeight * 2;
	}
	else if(nHeight > nHalfHeight && nHeight <= nFourThreeHeight)
	{
		// 拆为上下两份
		m_pFrameInfo[nFrame].nTexNum = 2;
		if(nWidth <= nHalfWidth)
			m_pFrameInfo[nFrame].texInfo[0].nWidth = nHalfWidth;
		else
			m_pFrameInfo[nFrame].texInfo[0].nWidth = nTexWidth;
		m_pFrameInfo[nFrame].texInfo[0].nHeight = nHalfHeight;
		m_pFrameInfo[nFrame].texInfo[0].nFrameX = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameY = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameWidth = nWidth;
		m_pFrameInfo[nFrame].texInfo[0].nFrameHeight = nHalfHeight;

		if(nWidth <= nHalfWidth)
			m_pFrameInfo[nFrame].texInfo[1].nWidth = nHalfWidth;
		else
			m_pFrameInfo[nFrame].texInfo[1].nWidth = nTexWidth;
		m_pFrameInfo[nFrame].texInfo[1].nHeight = nth;
		m_pFrameInfo[nFrame].texInfo[1].nFrameX = 0;
		m_pFrameInfo[nFrame].texInfo[1].nFrameY = nHalfHeight;
		m_pFrameInfo[nFrame].texInfo[1].nFrameWidth = nWidth;
		m_pFrameInfo[nFrame].texInfo[1].nFrameHeight = nHeight - nHalfHeight;

		nMem = nHalfWidth * (nHalfHeight + nth) * 2;
	}
	else
	{
		// 不拆，一份
		m_pFrameInfo[nFrame].nTexNum = 1;
		if(nWidth <= nHalfWidth)
			m_pFrameInfo[nFrame].texInfo[0].nWidth = nHalfWidth;
		else
			m_pFrameInfo[nFrame].texInfo[0].nWidth = nTexWidth;
		if(nHeight <= nHalfHeight)
			m_pFrameInfo[nFrame].texInfo[0].nHeight = nHalfHeight;
		else
			m_pFrameInfo[nFrame].texInfo[0].nHeight = nTexHeight;
		m_pFrameInfo[nFrame].texInfo[0].nFrameX = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameY = 0;
		m_pFrameInfo[nFrame].texInfo[0].nFrameWidth = nWidth;
		m_pFrameInfo[nFrame].texInfo[0].nFrameHeight = nHeight;

		nMem = m_pFrameInfo[nFrame].texInfo[0].nWidth * m_pFrameInfo[nFrame].texInfo[0].nHeight * 2;
	}

	return nMem;
}