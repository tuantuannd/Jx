/*******************************************************************************
// FileName			:	KFontRes.cpp
// FileAuthor		:	Wooy
// FileCreateDate	:	2001-9-12 9:57:55
// FileDescription	:	字体图形资源类
// Revision Count	:	2002-9-4改写，改为基于d3d。Wooy
*******************************************************************************/
#include "KFontRes.h"

#define	DESIRE_TEXTURE_SIDE_WIDTH	512

int KFontRes::m_bEnableTextBorder = true;

//启用/禁用文字边缘效果
void KFontRes::EnableTextBorder(bool bEnable)
{
	m_bEnableTextBorder = (bEnable != false);
}

/*!*****************************************************************************
// Purpose		:  构造函数
*****************************************************************************/
KFontRes::KFontRes()
{
	m_nFontW = 0;
	m_nFontH = 0;
	m_nNumCountH = 0;
	m_nMaxCount = 0;
	m_pd3dDevice   = NULL;
	m_pCharTexture = NULL;
	m_nTextureSideWidth = 512;
}

/*!*****************************************************************************
// Purpose		:  析构函数
*****************************************************************************/
KFontRes::~KFontRes()
{
	Terminate();
}

/*!*****************************************************************************
// Function		: KFontRes::Init
// Purpose		: 初始化
// Return		: 是否成功
// Argumant		: const char* pszFontFile      -> 字库文件名
// Argumant		: LPDIRECT3DDEVICE8 pd3dDevice -> directx 3d device接口的实例的指针
*****************************************************************************/
bool KFontRes::Init(const char* pszFontFile, LPDIRECT3DDEVICE9 pd3dDevice)
{
	Terminate();
	if ((m_pd3dDevice = pd3dDevice) == NULL)
		return false;
	if (pszFontFile == NULL)
		return false;

	//载入字库文件
	if (!m_FontData.Load(pszFontFile))
	{
		Terminate();
		return false;
	}

	m_FontData.GetInfo(m_nFontW, m_nFontH);

	HRESULT	hr;
    D3DCAPS9 d3dCaps;
    m_pd3dDevice->GetDeviceCaps(&d3dCaps);
	
    if(d3dCaps.MaxTextureWidth >= DESIRE_TEXTURE_SIDE_WIDTH)
		m_nTextureSideWidth = DESIRE_TEXTURE_SIDE_WIDTH;
	else
		m_nTextureSideWidth = d3dCaps.MaxTextureWidth;

    // Create a new texture for the font
    hr = m_pd3dDevice->CreateTexture( m_nTextureSideWidth, m_nTextureSideWidth, 1,
                                      0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pCharTexture, NULL);
    if (FAILED(hr))
	{
		Terminate();
		return false;
	}

	//把贴图清干静
/*	{
		//取得贴图数据块的指针
	    D3DLOCKED_RECT d3dlr;
		if (m_pCharTexture->LockRect( 0, &d3dlr, 0, 0 ) != D3D_OK)
		{
			Terminate();
			return false;
		}
		for(unsigned int v = 0; v < m_nTextureSideWidth; v++)
		{
			memset(d3dlr.pBits, 0, m_nTextureSideWidth * 4);
			d3dlr.pBits = (BYTE*)d3dlr.pBits + d3dlr.Pitch;
		}
		m_pCharTexture->UnlockRect(0);
	}*/

	//字符宽以及字符高加一是因为在把字符点阵图形存入贴图时保持字符间一个像素的间隔
	m_nNumCountH = m_nTextureSideWidth / (m_nFontW + 1);
	m_nMaxCount = (m_nTextureSideWidth / (m_nFontH + 1)) * m_nNumCountH;
	return true;
}

/*!*****************************************************************************
// Function		: KFontRes::GetInfo
// Purpose		: 得到字体的宽度高度，贴图可以缓存的字符的个数
// Argumant		: int &nWidth	字符宽
// Argumant		: int &nHeight	字符高
// Argumant		: int &nHCount	贴图里一行存储字符的个数
// Argumant		: int &nCount	缓存字符的个数
*****************************************************************************/
void KFontRes::GetInfo(int &nWidth, int &nHeight, int& nHCount, int &nCount) const
{
	nWidth  = m_nFontW;
	nHeight = m_nFontH;
	nHCount = m_nNumCountH;
	nCount = m_nMaxCount;
}

/*!*****************************************************************************
// Function		: KFontRes::GetInfo
// Purpose		: 得到字符在贴图里的宽度高度，水平间隔，贴图可以缓存的字符的个数
// Argumant		: float& fWidth		字符在贴图里的水平宽度
// Argumant		: float& fHeight	字符在贴图里的水平高度
// Argumant		: float& fHInterval	字符在贴图里的水平间隔
// Argumant		: float& fVInterval	字符在贴图里的垂直间隔
// Argumant		: int& nCount		缓存字符的个数
*****************************************************************************/
void KFontRes::GetInfo(float& fWidth, float& fHeight, float& fHInterval, float& fVInterval) const
{
	fWidth  = ((float)m_nFontW) / (float)m_nTextureSideWidth;
	fHeight = ((float)m_nFontH) / (float)m_nTextureSideWidth;
	fHInterval = ((float)(m_nFontW + 1)) / (float)m_nTextureSideWidth;
	fVInterval = ((float)(m_nFontH + 1)) / (float)m_nTextureSideWidth;
}

/*!*****************************************************************************
// Function		: KFontRes::Terminate
// Purpose		: 结束，清除操作
*****************************************************************************/
void KFontRes::Terminate()
{
	//释放贴图
	if (m_pCharTexture)
	{
		m_pCharTexture->Release();
		m_pCharTexture = NULL;
	}
	m_pd3dDevice = NULL;
	m_FontData.Terminate();
	m_nNumCountH = 0;
	m_nMaxCount = 0;
}

/*!***************************************************************************
// Function		: KFontRes::Update
// Purpose		: 更新缓存字符
// Argumant		: unsigned short * pUpdates	更新字符编码及在贴图中位置数组
						树组的结构为对应于每个更新字符有两个uint数值。
						前一个表示字符在贴图中的位置，后一个为字符的编码
// Argumant		: int nCount	更新字符的个数
*****************************************************************************/
void KFontRes::Update(unsigned short * pUpdates, int nCount)
{
	int				i;
	int				nTexIndex;	//字符在贴图里的索引
	int				nX, nY;		//字符左上角在贴图里坐标

	if (pUpdates == NULL || nCount <= 0)
		return;

	//取得贴图数据块的指针
	D3DLOCKED_RECT d3dlr;
	if (m_pCharTexture->LockRect( 0, &d3dlr, 0, 0 ) != D3D_OK)
		return;
	//逐个处理要更新的字符
	for (i = 0; i < nCount; i++)
	{
		//取得字符在贴图里的位置，并作检查
		nTexIndex = *pUpdates;
		if (nTexIndex >= m_nMaxCount)
		{
			pUpdates += 2;
			continue;
		}

		//取得字符在字库里的数据区指针
		unsigned char* pCharacterData = m_FontData.GetCharacterData(
			*(((unsigned char*)pUpdates) + 2), *(((unsigned char*)pUpdates) + 3));
		pUpdates += 2;
		if (pCharacterData == NULL)
			continue;		
		
		//计算字符左上角在贴图里坐标
		nX = (m_nFontW + 1) * (nTexIndex % m_nNumCountH);
		nY = (m_nFontH + 1) * (nTexIndex / m_nNumCountH);
		//更新贴图
		Update(pCharacterData,
			((BYTE*)d3dlr.pBits + d3dlr.Pitch * nY + nX * 2), d3dlr.Pitch);
	}
	m_pCharTexture->UnlockRect(0);
}

/*!*****************************************************************************
// Function		: KFontRes::Update
// Purpose		: 更新单个字符的点阵数据
// Argumant		: unsigned char* pCharData	图形数据块起点
// Argumant		: unsigned char* pTexData	贴图数据块起点
// Argumant		: int nTexPitch		贴图数据块pitch
*****************************************************************************/
void KFontRes::Update(unsigned char* pCharImage, unsigned char* pTexData, int nTexPitch)
{
	int		TextureOffset;		//贴图数据换行的偏移
	int		nWidth, nHeight;

	nWidth = m_nFontW;
	nHeight = m_nFontH;

	//计算贴图数据换行的偏移
	TextureOffset = nTexPitch - m_nFontW * 2;
	int bEnableTextBorder = m_bEnableTextBorder;

	__asm
	{
		//初始化 EDI 指向贴图数据起点
		mov 	edi, pTexData
		//初始化 ESI 指向图形数据起点
		mov		esi, pCharImage
		//高
		mov		ebx, 0xffff

		mov		eax, bEnableTextBorder
		cmp		eax, 0
		je		without_border_start_line

		with_border_start_line:
		{
			//宽
			mov		edx, nWidth

		with_border_same_alpha_block:
			xor		eax, eax
			mov     al, [esi]
			inc     esi
			mov		ecx, eax
			and		ecx, 0x1f			//eax为连续相同alpha值的点的个数
			sub		edx, ecx
		
			shr		eax, 5				//eax后3bit为alpha值

			cmp		eax, 0
			je		with_border_store_pixel

			cmp		eax, 7
			je		with_border_alpha_equal_7

			mov		eax, 0xefff
			jmp		with_border_store_pixel
		
		with_border_alpha_equal_7:
			mov		eax, ebx

		with_border_store_pixel:
			stosw
		    loop    with_border_store_pixel

			or		edx, edx
			jg		with_border_same_alpha_block
		
			add		edi, TextureOffset
			dec		nHeight
			jg		with_border_start_line
		}

		without_border_start_line:
		{
			//宽
			mov		edx, nWidth

		without_border_same_alpha_block:
			xor		eax, eax
			mov     al, [esi]
			inc     esi
			mov		ecx, eax
			and		ecx, 0x1f			//eax为连续相同alpha值的点的个数
			sub		edx, ecx
		
			shr		eax, 5				//eax后3bit为alpha值

			cmp		eax, 7
			je		without_border_alpha_equal_7

			xor		eax, eax
			jmp		without_border_store_pixel
		
		without_border_alpha_equal_7:
			mov		eax, ebx

		without_border_store_pixel:
			stosw
		    loop    without_border_store_pixel

			or		edx, edx
			jg		without_border_same_alpha_block
		
			add		edi, TextureOffset
			dec		nHeight
			jg		without_border_start_line
		}
	}
}