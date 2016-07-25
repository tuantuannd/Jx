//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp4Video.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMp4Video_H
#define KMp4Video_H
//---------------------------------------------------------------------------
#include "KVideo.h"
#include "KMemClass.h"
#include "KAviFile.h"
//---------------------------------------------------------------------------
#define WM_MP4MOVIE_END		WM_USER + 1
//---------------------------------------------------------------------------
class ENGINE_API KMp4Video : public KVideo
{
private:
	KAviFile			m_AviFile;
	DWORD				m_dwHandle;
	BOOL				DivXInit();
	void				DivXExit();
	void				DivXDecode(BOOL bRender);
public:
	KMp4Video();
	~KMp4Video();
	BOOL				Open(LPSTR FileName);
	void				Close();
	void				Seek(int nPercent);
	void				Rewind();
protected:
	BOOL				NextFrame();
	void				DrawFrame();
};
//---------------------------------------------------------------------------
#endif