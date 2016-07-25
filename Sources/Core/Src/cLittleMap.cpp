// cLittleMap.cpp: implementation of the cLittleMap class.
//
//////////////////////////////////////////////////////////////////////

#include "cLittleMap.h"
#include "KEngine.h"
#include "Kjpgfile.h"
#include "../../Represent/iRepresent/iRepresentShell.h"
#include "../../Represent/iRepresent/KRepresentUnit.h"
const int		c_nWidthRegion		= 32;
const int		c_nHeightRegion		= 64;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern struct iRepresentShell*	g_pRepresent;
iRepresentShell* GetShell(){return g_pRepresent;}

#ifndef ASSERT
#define ASSERT(a)
#endif

#define IDS_SHELL_MAP "littlemap"

const int c_nColorDepth = 16;
const int c_nColorBytes = c_nColorDepth / 8;

cLittleMap::cLittleMap()
{
	m_pImage = NULL;
	ZeroMemory(&m_stMap,sizeof(m_stMap));
	m_stMap.cx = GetImageSize();
	m_stMap.cy = GetImageSize();
	m_stMap.wScreen = 200;
	m_stMap.hScreen = 160;
	m_bUpdate = FALSE;
	m_nImageID = 0;
}

cLittleMap::~cLittleMap()
{
	Destroy();
}

int cLittleMap::Destroy()
{
	delete m_pImage;
	m_pImage = NULL;
	if (GetShell() && m_nImageID)
	{
		GetShell()->FreeImage(IDS_SHELL_MAP);
		m_nImageID = 0;
	}
	return true;
}

int cLittleMap::Create()
{
	ASSERT(GetShell());
	m_nImageID = GetShell()->CreateImage(IDS_SHELL_MAP,max_littmap_width,max_littmap_height,ISI_T_BITMAP16);

	return true;
}

int cLittleMap::SetPos(unsigned int x,unsigned int y)
{
	m_stMap.ptOffset.x = x;
	m_stMap.ptOffset.y = y;
	return true;
}

int cLittleMap::SetSize(unsigned int width,unsigned int height)
{
	m_stMap.wScreen = width;
	m_stMap.hScreen = height;
	return true;
}

int cLittleMap::SetRegionSize(unsigned int cx,unsigned int cy)
{
	m_stMap.cx = cx;
	m_stMap.cy = cy;
	return true;
}

int cLittleMap::SetCameraTo(int x, int y, int z) 
{
	//变换成为地图编辑器的坐标
	m_stMap.xTo = x;
	m_stMap.yTo = y;
	m_stMap.zTo = z;
	return true;
}

int cLittleMap::SetMapSize(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom)
{
	m_stMap.left = left;
	m_stMap.top = top;
	m_stMap.right = right;
	m_stMap.bottom = bottom;
	return true;
}

int cLittleMap::SetImage(const char* szJpgFile)
{
	ASSERT(GetShell());
	delete m_pImage;

	strcpy(m_stMap.szJpgFile,szJpgFile);
	m_pImage = new KBitmap16;

	KJpgFile f;
	if (!f.Load(m_stMap.szJpgFile,m_pImage))
		return false;

	m_bUpdate = TRUE;

	return true;
}

int cLittleMap::AddNpc(void* pNpc, int x, int y, int z)
{
	stNpcOnPixel st;
	st.pNpc = pNpc;
	st.x = x;
	st.y = y;
	st.z = z;
	m_listNpc.push_back(st);
	return 0;
}

int cLittleMap::RemoveNpc(void* pNpc)
{
	for (std::list<stNpcOnPixel>::iterator it = m_listNpc.begin(); it != m_listNpc.end(); ++it)
	{
		if ((*it).pNpc = pNpc)
		{
			m_listNpc.erase(it);
			return true;
		}
	}
	return false;
}

int cLittleMap::RemoveAllNpcs()
{
	m_listNpc.clear();
	return true;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int cLittleMap::OnCameraUpdate()
{
	//将KBITMAP画到image
	return true;
}

///////////////////////////////////////////////////////////////////////////
//get
///////////////////////////////////////////////////////////////////////////
iLittleMap* LittleMap_GetLittleMap()
{
	static cLittleMap s_map;
	return &s_map;
}

///////////////////////////////////////////////////////////////////////////
//function from worldeditor!
///////////////////////////////////////////////////////////////////////////
//#include "iImage.h"
struct stImageRender
{
	UINT			nFrame;	//src frame
	CRect			rcSrc;	//src rect

	void*			buffer; //des buffer
	UINT			nPitch; //des buffer pitch
	CPoint			ptDes;	//des point
//	eImageFormat	format; //des format

//	eRenderOp		eOp;	//op 
};

void cLittleMap::RenderTexture()
{
	char* pDes = (char*)GetShell()->GetBitmapDataBuffer(IDS_SHELL_MAP);
	char* pSrc = (char*)m_pImage->GetBitmap();

	stImageRender rd;
	rd.nFrame = 0;
	rd.buffer = pDes;
	rd.nPitch = max_littmap_width * c_nColorBytes;
	rd.rcSrc = CRect(m_ptLoad.x,m_ptLoad.y,
		m_ptLoad.x+max_littmap_width,m_ptLoad.y+max_littmap_height);
	rd.ptDes.x = 0;
	rd.ptDes.y = 0;

	pSrc += int (rd.rcSrc.top * m_pImage->GetWidth() * c_nColorBytes + rd.rcSrc.left * c_nColorBytes);
	for (int i=0; i<rd.rcSrc.Height(); i++)
	{
		memcpy(pDes,pSrc,max_littmap_width*c_nColorBytes);
		pDes += max_littmap_width*c_nColorBytes;
		pSrc += m_pImage->GetWidth() * c_nColorBytes;
	}
}

void DrawPoint(cPoint pt, cColor cr)
{
	KRUPoint Rect;
	Rect.Color.Color_dw = cr.Value();
	Rect.oPosition.nX = pt.x;
	Rect.oPosition.nY = pt.y;
	Rect.oPosition.nZ = 0;
	GetShell()->DrawPrimitives(1, &Rect, RU_T_POINT, true);
}

void DrawRect(cRect r, cColor cr)
{
	KRURect	Rect;
	Rect.Color.Color_dw = cr.Value();
	Rect.oPosition.nX = r.left;
	Rect.oEndPos.nX = r.right;
	Rect.oPosition.nY = r.top;
	Rect.oEndPos.nY = r.bottom;
	Rect.oEndPos.nZ = Rect.oPosition.nZ = 0;
	GetShell()->DrawPrimitives(1, &Rect, RU_T_RECT, true);
}

void DrawTexture(int nID, cRect r)
{
	KRUImage	Image;
	Image.nType = ISI_T_BITMAP16;
	Image.bRenderStyle = IMAGE_RENDER_STYLE_ALPHA;
	Image.uImage = nID;
	Image.nISPosition = IMAGE_IS_POSITION_INIT;
//	strcpy(Image.szImage, szTexture);
	Image.szImage[0] = 0;
	Image.Color.Color_b.a = 255;
	Image.bRenderFlag = 0;
	Image.nFrame = 0;

	Image.oPosition.nX = r.left;
	Image.oEndPos.nX = r.right;
	Image.oPosition.nX = r.top;
	Image.oEndPos.nX = r.bottom;
	Image.oPosition.nZ = Image.oEndPos.nZ = 0;

	GetShell()->DrawPrimitives(1, &Image, RU_T_IMAGE, true);
}

BOOL cLittleMap::VerifyTexture(BOOL bForceUpdate)
{
	stLittemap& m_param = *GetMapParam();

	CRect rcGroundNow;
	ScreenToGround(0,0,rcGroundNow.left,rcGroundNow.top);
	ScreenToGround(m_param.wScreen,m_param.hScreen,rcGroundNow.right,rcGroundNow.bottom);

	CRect rcGround = CRect(m_rcRegion.left*c_nWidthRegion,m_rcRegion.top*c_nHeightRegion,
		m_rcRegion.right*c_nWidthRegion,m_rcRegion.bottom*c_nHeightRegion);
	CRect rcImage = CRect(m_rcRegion.left*GetImageSize(),m_rcRegion.top*GetImageSize(),
		m_rcRegion.right*GetImageSize(),m_rcRegion.bottom*GetImageSize());

	rcGroundNow = rcGroundNow&rcGround;
	if (rcGroundNow.IsRectEmpty())
		return FALSE;

	GroundToScreen(rcGroundNow.left,rcGroundNow.top,m_rcMap.left,m_rcMap.top);
	GroundToScreen(rcGroundNow.right,rcGroundNow.bottom,m_rcMap.right,m_rcMap.bottom);

	BOOL bUpdate = m_bUpdate;
	m_bUpdate = FALSE;

	m_rcImage.left = (rcGroundNow.left - rcGround.left) * GetImageSize() / c_nWidthRegion ;
	m_rcImage.top = (rcGroundNow.top - rcGround.top) * GetImageSize() / c_nHeightRegion;		
	m_rcImage.right = m_rcImage.left + (rcGroundNow.Width()) * GetImageSize() / c_nWidthRegion ;
	m_rcImage.bottom = m_rcImage.top + (rcGroundNow.Height()) * GetImageSize() / c_nHeightRegion ;
	if (!bUpdate)
	{
		if (m_rcImage.left < m_ptLoad.x || m_rcImage.right > m_ptLoad.x + c_nTextureWidth
			|| m_rcImage.top < m_ptLoad.y || m_rcImage.bottom > m_ptLoad.y + c_nTextureHeight)
		{
			bUpdate = TRUE;
		}
	}

	if (bUpdate || bForceUpdate)
	{
		//compute the ptload and ptTexture!
		//center
		stLittemap& m_param = *GetMapParam();
		CRect rc(m_param.ptOffset.x,m_param.ptOffset.y,
			m_param.ptOffset.x+m_param.wScreen,m_param.ptOffset.y+m_param.hScreen);
		CPoint pt = rc.CenterPoint();
		cPoint ptGround;
		ScreenToGround(pt,ptGround);
		int dx = (ptGround.x - rcGround.left) * GetImageSize() / c_nWidthRegion;
		int dy = (ptGround.y - rcGround.top) * GetImageSize() / c_nHeightRegion;
		m_ptLoad.x = dx - c_nTextureWidth / 2;
		m_ptLoad.y = dy - c_nTextureHeight / 2;

		if (m_ptLoad.x + c_nTextureWidth > rcImage.Width())
		{
			m_ptLoad.x = rcImage.Width() - c_nTextureWidth;
		}
		if (m_ptLoad.x < 0)
		{
			m_ptLoad.x = 0;
		}
		
		if (m_ptLoad.y + c_nTextureHeight > rcImage.Height())
		{
			m_ptLoad.y = rcImage.Height() - c_nTextureHeight;
		}
		if (m_ptLoad.y < 0)
		{
			m_ptLoad.y = 0;
		}
		//render the texture!
		RenderTexture();
		//
	}

	m_ptTexture.x = m_rcImage.left - m_ptLoad.x;
	m_ptTexture.y = m_rcImage.top - m_ptLoad.y;
	return TRUE;
}

HRESULT	cLittleMap::DrawTexture()
{
	if (m_stMap.szJpgFile[0] == 0)
		return S_OK;
	if (m_pImage == NULL || m_nImageID == 0)
		return 0;
	if (VerifyTexture())
	{
		CRect rc(m_stMap.ptOffset.x,m_stMap.ptOffset.y,
			m_stMap.ptOffset.x + m_stMap.wScreen,m_stMap.ptOffset.y + m_stMap.hScreen);
		::DrawTexture(m_nImageID,rc);
	}
	return S_OK;
}

HRESULT	cLittleMap::DrawNpc()
{
	for (std::list<stNpcOnPixel>::iterator it = m_listNpc.begin(); it != m_listNpc.end(); ++it)
	{
		stNpcOnPixel st = *it;
		cPoint ptGame = cPoint(st.x,st.y,st.z);
		cPoint ptGround;
		GameToGround(ptGame,ptGround);
		CPoint ptScreen;
		GroundToScreen(ptGround.x,ptGround.y,ptScreen.x,ptScreen.y);
		DrawPoint(ptScreen,cColor(255,255,255));
	}
	return S_OK;
}

HRESULT	cLittleMap::Draw()
{
	//use a texture of 512*512
	if (!IsShow())
		return S_OK;

	DrawTexture();

	DrawNpc();

	stLittemap& m_param = *GetMapParam();
	if (m_pImage != NULL)
	{
		cRect rc(m_param.ptOffset.x,m_param.ptOffset.y,
			m_param.ptOffset.x+m_param.wScreen,
			m_param.ptOffset.y+m_param.hScreen);
		DrawRect(rc,cColor(0,0,255));
	}
	cRect rc(m_param.ptOffset.x,m_param.ptOffset.y,m_param.ptOffset.x+8,m_param.ptOffset.y+8);
	DrawRect(rc,cColor(0,255,0));


	return S_OK;
}

void cLittleMap::GroundToScreen(float x,float y, float & nx,float & ny)
{
	stLittemap& m_param = *GetMapParam();

	cPoint ptGroundFocus = GetFocus();
	int offx = x - ptGroundFocus.x;
	int offy = y - ptGroundFocus.y;

	CRect rc(m_param.ptOffset.x,m_param.ptOffset.y,
		m_param.ptOffset.x+m_param.wScreen,m_param.ptOffset.y+m_param.hScreen);
	CPoint pt = rc.CenterPoint();

	nx = pt.x + offx * GetImageSize() / c_nWidthRegion;
	ny = pt.y + offy * GetImageSize() / c_nHeightRegion;
}

void cLittleMap::ScreenToGround(float x,float y, float & nx,float & ny)
{
	stLittemap& m_param = *GetMapParam();

	cPoint ptGroundFocus = GetFocus();
	//the center is the current region!	
	CRect rc(m_param.ptOffset.x,m_param.ptOffset.y,
		m_param.ptOffset.x+m_param.wScreen,m_param.ptOffset.y+m_param.hScreen);
	CPoint pt = rc.CenterPoint();
	CPoint ptOffsetScreen = CPoint(x,y)- pt;
	nx = ptGroundFocus.x + ptOffsetScreen.x * c_nWidthRegion / GetImageSize();
	ny = ptGroundFocus.y + ptOffsetScreen.y * c_nHeightRegion / GetImageSize();
}

void cLittleMap::ScreenToGround(CPoint ptScreen, cPoint& ptGround)
{
	stLittemap& m_param = *GetMapParam();

	cPoint ptGroundFocus = GetFocus();
	//the center is the current region!	
	CRect rc(m_param.ptOffset.x,m_param.ptOffset.y,
		m_param.ptOffset.x+m_param.wScreen,m_param.ptOffset.y+m_param.hScreen);
	CPoint pt = rc.CenterPoint();
	CPoint ptOffsetScreen = ptScreen - pt;
	cPoint ptOffsetGround = cPoint(ptOffsetScreen.x * c_nWidthRegion / GetImageSize() , ptOffsetScreen.y * c_nHeightRegion / GetImageSize() );
	ptGround = ptGroundFocus;
	ptGround += ptOffsetGround;
}

void cLittleMap::GameToGround(cPoint ptGame,cPoint& ptGround)
{
	ptGround.x = ptGame.x / 32.f;
	ptGround.y = ptGame.y / 16.f;
	ptGround.z = ptGame.z;
}

cPoint cLittleMap::GetFocus()
{
	cPoint ptGame = cPoint(m_stMap.xTo,m_stMap.yTo,m_stMap.zTo);
	cPoint ptGround;
	GameToGround(ptGame,ptGround);
	return ptGround;
}
