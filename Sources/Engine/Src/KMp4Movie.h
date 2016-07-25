//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp4Movie.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMp4Movie_H
#define KMp4Movie_H
//---------------------------------------------------------------------------
#include "KMp4Audio.h"
#include "KMp4Video.h"
//---------------------------------------------------------------------------

class ENGINE_API KMp4Movie
{
private:
	KMp4Audio		m_Audio;
	KMp4Video		m_Video;
public:
	BOOL			Open(LPSTR FileName);
	void			Close();
	void			Play(int nX=0, int nY=0,int nZoom=0);
	void			Stop();
	void			Seek(int nPercent);
	void			Rewind();
	void			WaitForEnd();
};
//---------------------------------------------------------------------------
#endif
