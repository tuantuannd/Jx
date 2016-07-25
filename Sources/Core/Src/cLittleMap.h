// cLittleMap.h: interface for the cLittleMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLITTLEMAP_H__09658E99_FB69_4E59_B94E_96F173C83AE9__INCLUDED_)
#define AFX_CLITTLEMAP_H__09658E99_FB69_4E59_B94E_96F173C83AE9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

const int max_littmap_width = 512;
const int c_nTextureWidth = max_littmap_width; 
const int max_littmap_height = 512;
const int c_nTextureHeight = max_littmap_height;

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#include "windows.h"

#include <list>
#include "windows.h"
#include "..\..\..\Headers\iColor.h"
typedef cPoint CPoint;
typedef cRect CRect;

struct stLittemap
{
//	unsigned int x,y;							//window pos,	0,0
	cPoint ptOffset;
//	unsigned int width,height;					//window size,	200*160, but maxsize is 512*512
	int wScreen,hScreen;
	unsigned int cx,cy;							//region size,	32*32 or 64*64
	unsigned int left,top,right,bottom;			//map size,		100,189,200,330
	int xTo,yTo,zTo;							//camera to,	100,2000,0
	char szJpgFile[MAX_PATH];					//jpg file		³É¶¼.jpg
};

struct iRepresentShell;
class iLittleMap  
{
public:
	virtual int Create() = 0;
	virtual int Destroy() = 0;
	
	virtual int SetPos(unsigned int x,unsigned int y) = 0;
	virtual int SetSize(unsigned int width,unsigned int height) = 0;
	virtual int SetRegionSize(unsigned int cx,unsigned int cy) = 0;

	virtual int SetCameraTo(int x, int y, int z) = 0;
	virtual int SetImage(const char* szJpgFile) = 0;
	virtual int SetMapSize(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom) = 0;
//attention: if there is a setimage and setcamerato
//must call setiamge first then call setcamerato

	virtual int AddNpc(void* pNpc, int x, int y, int z) = 0;
	virtual int RemoveNpc(void* pNpc) = 0;
	virtual int RemoveAllNpcs() = 0;
	virtual HRESULT	Draw() = 0;
};

class KBitmap16;


class cLittleMap : public iLittleMap
{
protected:
	stLittemap m_stMap;
	stLittemap* GetMapParam(){return &m_stMap;}
	KBitmap16* m_pImage;
	int m_nImageID;
	// i can not use hash table here, it is a pitty
	struct stNpcOnPixel
	{
		int x,y,z;
		void* pNpc;
	};
	std::list<stNpcOnPixel> m_listNpc;

public:
	cLittleMap();
	virtual ~cLittleMap();
	int Create();
	int Destroy();

	int SetPos(unsigned int x,unsigned int y);
	int SetSize(unsigned int width,unsigned int height);
	int GetImageSize(){return m_stMap.cx;}
	int SetRegionSize(unsigned int cx,unsigned int cy);

	int SetImage(const char* szJpgFile);
	int SetMapSize(unsigned int left,unsigned int top,unsigned int right,unsigned int bottom);

	int SetCameraTo(int x, int y, int z) ;

	void GameToGround(cPoint ptGame,cPoint& ptGround);
	cPoint GetFocus();

	int AddNpc(void* pNpc, int x, int y, int z);
	int RemoveNpc(void* pNpc);
	int RemoveAllNpcs();
	HRESULT	Draw();
	HRESULT	DrawTexture();
	HRESULT	DrawNpc();

	bool IsShow(){return true;}

protected:
//	CPoint						m_ptFocus;			//camera focus
//	CPoint						GetFocus(){return m_ptFocus;}
	CRect						m_rcRegion;			//get 'rect' from the txt
	CRect						m_rcMap;			//rc of the map!
	CPoint						m_ptTexture;		//the rcMap to texture
	CRect						m_rcImage;			//the rcMap to whole image
	CPoint						m_ptLoad;			//and load size is always 512*512! and ptLoad always = 32 * n
	BOOL						m_bUpdate;

	void						ScreenToGround(CPoint ptScreen, cPoint& ptGround);
	void						ScreenToGround(float x,float y, float & nx,float & ny);
	void						GroundToScreen(float x,float y, float & nx,float & ny);
	void						RenderTexture();
	BOOL						VerifyTexture(BOOL bForceUpdate = FALSE);
	int							OnCameraUpdate();
};

iLittleMap* LittleMap_GetLittleMap();

#endif // !defined(AFX_CLITTLEMAP_H__09658E99_FB69_4E59_B94E_96F173C83AE9__INCLUDED_)
