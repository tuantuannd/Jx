//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KVideo.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KVideo_H
#define KVideo_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KThread.h"
#include "KTimer.h"
//---------------------------------------------------------------------------
class ENGINE_API KVideo
{
protected:
	KMemClass				m_VideoBuffer;
	KMemClass				m_FrameBuffer;
	KThread					m_Thread;
	KTimer					m_Timer;
	HANDLE					m_hEvent[2];
	int						m_nX;
	int						m_nY;
	int						m_nWidth;
	int						m_nHeight;
	PBYTE					m_pVideo;
	PBYTE					m_pFrame;
	int						m_nFrameSize;
	int						m_nFrameRate;
	int						m_nFrameTime;
	ULONG					m_uTimeID;
	BOOL					m_bZoomToWin;
public:
	KVideo();
	~KVideo();
	virtual BOOL			Open(LPSTR FileName);
	virtual void			Close();
	virtual void			Play(int nX=0, int nY=0, int nZoom=0);
	virtual void			Stop();
	virtual void			Rewind();
	virtual void			Seek(int nPercent);
	virtual BOOL			HandleNotify();
	virtual BOOL			IsPlaying();
protected:
	virtual BOOL			Init();
	virtual BOOL			NextFrame();
	virtual BOOL			InitEventHandle();
	virtual void			FreeEventHandle();
	static	void			ThreadFunction(void* lpParam);
};
//---------------------------------------------------------------------------
#endif