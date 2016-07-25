//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KAviFile.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header file
//---------------------------------------------------------------------------
#ifndef KAviFile_H
#define KAviFile_H
//---------------------------------------------------------------------------
#include "KPakFile.h"
//---------------------------------------------------------------------------
// SOME USEFUL MACROS
#define PAD_EVEN(x)		(((x) + 1) & ~1)
#define AVIIF_KEYFRAME	0x00000010L
//---------------------------------------------------------------------------
// video index
typedef struct
{
	long pos;
	long len;
	long flags;
	
} video_index_entry;
//---------------------------------------------------------------------------
// autio index
typedef struct
{
	long pos;
	long len;
	long tot;
	
} audio_index_entry;
//---------------------------------------------------------------------------
// avi file class
class ENGINE_API KAviFile
{
private:
	BITMAPINFOHEADER	m_BitmapInfoHeader;
	WAVEFORMATEX		m_WaveFormatEx;
	char				m_compressor[8];
	int					m_fps; 			  
	long				m_video_strn;
	long				m_video_frames;
	char				m_video_tag[4];
	long				m_video_pos;
	long				m_audio_strn;
	long				m_audio_bytes;
	long				m_audio_chunks;
	char				m_audio_tag[4];
	long				m_audio_posc;
	long				m_audio_posb;
	char				(*m_idx)[16];
	long				m_idx_num;
	long				m_idx_type;
	video_index_entry*	m_video_index;
	audio_index_entry*	m_audio_index;
	long				m_movi_start;
	KPakFile			m_AviFile;
	
public:
	KAviFile();
	~KAviFile();
	BOOL				Open(char* FileName);
	BOOL				FillHeader();
	BOOL				GetAudioIndex();
	BOOL				GetVideoIndex();
	void				GetCompressMethod(char* method);
	void				GetBitmapInfoHeader(PBITMAPINFOHEADER pBmpInfoHead);
	void				GetWaveFormat(PWAVEFORMATEX pWavFmt);
	int 				VideoStreams();
	int 				AudioStreams();
	int					FrameRate();
	int					CurrentFrame();
	int					TotalFrames();
	int 				NextFrame(unsigned char *buffer);
	int 				ReadAudio(unsigned char *buffer, int size);
	int 				VideoSeek(int percent);
	int 				AudioSeek(int percent);
	BOOL				IsKeyframe(int frame);
	int 				NextKeyFrame();
	int 				PreviousKeyFrame();
	int					Rewind();
	void				Close();
};
//---------------------------------------------------------------------------
#endif // KAviFile_H