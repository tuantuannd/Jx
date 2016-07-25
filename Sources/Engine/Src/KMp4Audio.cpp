//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp4Audio.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	MP3 Stream Music Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KMp4Audio.h"
//---------------------------------------------------------------------------
#define WAVE_FORMAT_MP3		85	// mp3 wave format tag value
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Audio::Seek(int nPercent)
{
	m_AviFile.AudioSeek(nPercent);
	mp3_bufbytes = 0;
}
//---------------------------------------------------------------------------
// 函数:	Rewind
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp4Audio::Rewind()
{
	m_AviFile.Rewind();
	mp3_bufbytes = 0;
}
//---------------------------------------------------------------------------
// 函数:	Mp3FileOpen
// 功能:	
// 参数:	
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KMp4Audio::Mp3FileOpen(LPSTR FileName)
{
	if (!m_AviFile.Open(FileName))
		return FALSE;

	if (!m_AviFile.GetAudioIndex())
		return FALSE;
	
	m_AviFile.GetWaveFormat(&m_WaveFormat);
	if (m_WaveFormat.wFormatTag != WAVE_FORMAT_MP3)
		return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Mp3FileRead
// 功能:	
// 参数:	
// 返回:	void
//---------------------------------------------------------------------------
DWORD KMp4Audio::Mp3FileRead(PBYTE pBuf, DWORD dwLen)
{
	return m_AviFile.ReadAudio(pBuf, dwLen);
}
//---------------------------------------------------------------------------
// 函数:	Mp3FileSeek
// 功能:	
// 参数:	
// 返回:	void
//---------------------------------------------------------------------------
DWORD KMp4Audio::Mp3FileSeek(LONG lOffset)
{
	return m_AviFile.AudioSeek(lOffset);
}
//---------------------------------------------------------------------------
