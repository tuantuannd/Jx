//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KLMp4Video.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMp4Video_H
#define KMp4Video_H
//---------------------------------------------------------------------------
#include "KLAviFile.h"
#include "KLThread.h"
#include "KLTimer.h"
//---------------------------------------------------------------------------
#define WM_MP4MOVIE_END		WM_USER + 1
//---------------------------------------------------------------------------

typedef unsigned int        uint32;

class KLMp4Video
{
private:
	KLTimer					m_Timer;
	KLThread				m_Thread;
	HANDLE					m_hEvent[2];
	HANDLE					m_hMutex;
	int						m_nX;
	int						m_nY;
	int						m_nWidth;
	int						m_nHeight;
	LPBYTE					m_pVideo;
	LPBYTE					m_pFrame;
	int						m_nFrameSize;
	int						m_nFrameRate;
	int						m_nFrameTime;
	ULONG					m_uTimeID;
	BOOL					m_bZoomToWin;

	KLAviFile			m_AviFile;
	DWORD				m_dwHandle;
	BOOL				DivXInit();
	void				DivXExit();
	void				DivXDecode(BOOL bRender);

	bool				m_bWait;
public:
	KLMp4Video();
	~KLMp4Video();
	BOOL				Open(LPSTR FileName);
	void				Close();
	void				Seek(int nPercent);
	void				Rewind();
	void				Play(BOOL bZoom = FALSE);
	void				Stop();
	BOOL				IsPlaying();
	void				GetVideoSize(int &nWidth,int &nHeight){nWidth = m_nWidth , nHeight = m_nHeight;}
	int					GetVideoFrames(){return m_AviFile.TotalFrames();}
	int					GetCurFrame(){return m_AviFile.CurrentFrame();}

	bool				ShouldWait()
	{	
		bool bRet = m_bWait && (m_uTimeID != 0);
		m_bWait = true;
		return bRet;
	}

	void				VideoCopyToBuffer(void *buf,uint32 left,uint32 top,
											uint32 Pitch,uint32 destheight,uint32 Flags);
protected:
	BOOL				NextFrame();
	void				SetPos(int nX, int nY);
	BOOL				Init();
	BOOL				InitEventHandle();
	void				FreeEventHandle();
	static	void		ThreadFunction(void* lpParam);
	BOOL				HandleNotify();
};

//---------------------------------------------------------------------------
#endif