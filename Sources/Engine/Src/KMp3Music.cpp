//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMp3Music.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	MP3 Stream Music Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KFilePath.h"
#include "mp3lib.h"
#include "KMp3Music.h"
//---------------------------------------------------------------------------
#define MP3_BUFSIZE 60000 // mp3 data buffer
//---------------------------------------------------------------------------
/*typedef struct
{
	int sync;			// 0xFFF=mpeg1.0 & 2.0  0xFFE=mpeg2.5
	int ver;			// 1=mpeg1.0 0=mpeg2.0
	int layer;			// 1=LayerIII 2=LayerII 3=LayerI
	int prot;			// error protection  0=yes, 1=no
	int br_index;		// bitrate index
	int sr_index;		// sampling rate
	int pad;			// padding
	int extension;		// extension
	int mode;			// 0=stereo 1=joint stereo 2=dual channel 3=single channel
	int mode_ext;		// 0=MPG_MD_LR_LR 1=MPG_MD_LR_I 2=MPG_MD_MS_LR 3=MPG_MD_MS_I
	int copyright;		// 0=no 1=yes
	int original;		// 0=no 1=yes
	int emphasis;		// 0=none 1=50/15 microsecs 2=reserved 3=CCITT J 17
}
MPEG_HEAD;
//---------------------------------------------------------------------------
typedef struct
{
	int channels;		// channels = 1,2
	int outvalues;		// out value
	int samprate;		// sample rate
	int bits;			// bits per sample = 8,16,24,32
	int framebytes;		// frame size in bytes
}
DEC_INFO;
//---------------------------------------------------------------------------
typedef long (*mp3_decode_head)(unsigned char *buf, MPEG_HEAD* head);
typedef long (*mp3_decode_init)(MPEG_HEAD *head,
								int framebytes_arg,
								int reduction_code,
								int transform_code,
								int convert_code,
								int freq_limit);
typedef void (*mp3_decode_info)(DEC_INFO *info);
typedef void (*mp3_decode_data)(unsigned char *mp3, 
								int *mp3_size,
								unsigned char *pcm,
								int *pcm_size);
//---------------------------------------------------------------------------
static HMODULE	m_hDll = NULL;
static mp3_decode_head Mp3DecodeHead = NULL;
static mp3_decode_init Mp3DecodeInit = NULL;
static mp3_decode_info Mp3DecodeInfo = NULL;
static mp3_decode_data Mp3DecodeData = NULL;
static BOOL LoadMp3Lib();
static void FreeMp3Lib();
//---------------------------------------------------------------------------
// 函数:	Load Mp3 Lib
// 功能:	加载 Mp3Lib.dll
// 返回:	BOOL 
//---------------------------------------------------------------------------
BOOL LoadMp3Lib()
{
	if (m_hDll)
		return TRUE;
	char szPathName[MAXPATH];
	g_GetFullPath(szPathName, "\\mp3lib.dll");
	m_hDll = LoadLibrary(szPathName);
	if (m_hDll == NULL)
	{
		g_DebugLog("Can't load mp3lib.dll!");
		return FALSE;
	}
	Mp3DecodeHead = (mp3_decode_head)GetProcAddress(m_hDll, "mp3_decode_head");
	Mp3DecodeInit = (mp3_decode_init)GetProcAddress(m_hDll, "mp3_decode_init");
	Mp3DecodeInfo = (mp3_decode_info)GetProcAddress(m_hDll, "mp3_decode_info");
	Mp3DecodeData = (mp3_decode_data)GetProcAddress(m_hDll, "mp3_decode_data");
	if ((!Mp3DecodeHead)||(!Mp3DecodeInit)||(!Mp3DecodeInfo)||(!Mp3DecodeData))
	{
		g_DebugLog("There are errors in Mp3lib.dll!");
		FreeMp3Lib();
		return FALSE;
	}
	g_DebugLog("Mp3lib.dll load Ok!");
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FreeMp3Lib
// 功能:	
// 返回:	void
//---------------------------------------------------------------------------
void FreeMp3Lib()
{
	if (m_hDll)
		FreeLibrary(m_hDll);
	m_hDll = NULL;
}*/
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开音乐文件
// 参数:	FileName	文件名
// 返回:	TRUE－成功	FALSE－失败
//---------------------------------------------------------------------------
BOOL KMp3Music::Open(LPSTR FileName)
{
	if (m_pSoundBuffer)
		Close();

	if (!Mp3FileOpen(FileName))
		return FALSE;

	if (!Mp3Init())
		return FALSE;

	return Init();
}
//---------------------------------------------------------------------------
// 函数:	InitSoundFormat
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp3Music::InitSoundFormat()
{
	// align buffer size with mp3 frame 4608
	m_dwBufferSize = m_WaveFormat.nAvgBytesPerSec;
	m_dwBufferSize = (m_dwBufferSize + 4607) / 4608 * 4608;

	// 2 second sound buffer
	m_dwBufferSize *= 2;
}
//---------------------------------------------------------------------------
// 函数:	ReadWaveData
// 功能:	读取波形数据
// 参数:	lpBuf	缓存
//			dwLen	长度	
// 返回:	DWORD	实际读取的长度
//---------------------------------------------------------------------------
DWORD KMp3Music::ReadWaveData(LPBYTE lpBuf, DWORD dwLen)
{
	return Mp3Decode(lpBuf, dwLen);
}
//---------------------------------------------------------------------------
// 函数:	Seek
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp3Music::Seek(int nPercent)
{
}
//---------------------------------------------------------------------------
// 函数:	Rewind
// 功能:	从头播放音乐
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KMp3Music::Rewind()
{
	Mp3FileSeek(0);
	mp3_bufbytes = 0;
}
//---------------------------------------------------------------------------
// 函数:	Mp3FileOpen
// 功能:	
// 参数:	FileName
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KMp3Music::Mp3FileOpen(LPSTR FileName)
{
	return m_Mp3File.Open(FileName);
}
//---------------------------------------------------------------------------
// 函数:	Mp3FileRead
// 功能:	
// 参数:	pBuf
//			dwlen
// 返回:	void
//---------------------------------------------------------------------------
DWORD KMp3Music::Mp3FileRead(PBYTE pBuf, DWORD dwLen)
{
	return m_Mp3File.Read(pBuf, dwLen);
}
//---------------------------------------------------------------------------
// 函数:	Mp3FileSeek
// 功能:	
// 参数:	lOffset
// 返回:	void
//---------------------------------------------------------------------------
DWORD KMp3Music::Mp3FileSeek(LONG lOffset)
{
	return m_Mp3File.Seek(lOffset, FILE_BEGIN);
}
//---------------------------------------------------------------------------
// 函数:	Mp3Init
// 功能:	初始化MP3文件
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KMp3Music::Mp3Init()
{
	MPEG_HEAD mpeg_head;
	DEC_INFO  dec_info;
	
	// load mp3lib.dll
/*	if (!LoadMp3Lib())
		return FALSE;
*/
	// reduction_code:	sample rate reduction code
	//		0 = full rate
	//		1 = half rate
	//		2 = quarter rate
	int reduction_code = 0;
	
	// transform_code:	ignored
	int transform_code = 0;

	// convert_code:	channel conversion
	//		0 = two chan output
	//		1 = convert two chan to mono
	//		2 = convert two chan to left chan
	//		3 = convert two chan to right chan
	int convert_code = 0;
	
	// freq_limit:	limits bandwidth of pcm output to specified
	//		frequency. Special use. Set to 24000 for normal use.
	int freq_limit = 24000;

	// alloc mp3 stream buffer
	if (!m_Mp3Buffer.Alloc(MP3_BUFSIZE))
		return FALSE;
	mp3_buffer = (PBYTE)m_Mp3Buffer.GetMemPtr();

	// init mp3 buffer
	mp3_bufbytes = 0;
	mp3_bufptr = mp3_buffer;

	// seek to begin of mp3 file
	Mp3FileSeek(0);

	// fill mp3 buffer
	if (!Mp3FillBuffer())
		return FALSE;

	// parse mpeg header
	mp3_frmbytes = mp3_decode_head(mp3_buffer, &mpeg_head);
	if (mp3_frmbytes == 0)
	{
		g_MessageBox("KMp3Music: mp3 decode head fail");
		return FALSE;
	}

	// init mp3 decoder
	if (!mp3_decode_init(&mpeg_head, mp3_frmbytes,
		reduction_code, transform_code, convert_code, freq_limit))
	{
		g_MessageBox("KMp3Music: mp3 decode init fail");
		return FALSE;
	}

	// get mp3 info
	mp3_decode_info(&dec_info);

	// set pcm wave format
	m_WaveFormat.wFormatTag		 = WAVE_FORMAT_PCM;
	m_WaveFormat.nChannels		 = dec_info.channels;
	m_WaveFormat.nSamplesPerSec	 = dec_info.samprate;
	m_WaveFormat.nAvgBytesPerSec = (dec_info.channels * dec_info.samprate * dec_info.bits + 7) / 8;
	m_WaveFormat.nBlockAlign	 = (dec_info.channels * dec_info.bits + 7) / 8;
	m_WaveFormat.wBitsPerSample	 = dec_info.bits;
	m_WaveFormat.cbSize			 = 0;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Mp3Decode
// 功能:	解码MP3
// 参数:	lpPcmBuf		PCM buffer pointer
//			dwBufLen		buffer size in bytes
// 返回:	void
//---------------------------------------------------------------------------
DWORD KMp3Music::Mp3Decode(PBYTE lpPcmBuf, DWORD dwBufLen)
{
	PBYTE	pcm_buffer = lpPcmBuf;
	int		pcm_trigger = dwBufLen;
	int		pcm_bufbytes = 0;
	int		mp3_size = 0;
	int		pcm_size = 0;
	IN_OUT	res = {0, 0};
		
	while (TRUE)
	{
		// fill mp3 buffer
		if (!Mp3FillBuffer())
			break;
		
		// end of mp3 file
		if (mp3_bufbytes < mp3_frmbytes)
			break;

		// decode one frame to pcm buffer
		res = mp3_decode_frame(mp3_bufptr, pcm_buffer);
		if (res.in_bytes <= 0)
		{
			g_DebugLog("KMp3Music: bad sync in mp3 file");
			break;
		}

		// update buffer pointer and buffer size
		mp3_bufptr += res.in_bytes;
		mp3_bufbytes -= res.in_bytes;
		pcm_buffer += res.out_bytes;
		pcm_bufbytes += res.out_bytes;
		
		// is pcm buffer full
		if (pcm_bufbytes >= pcm_trigger)
			break;
	}
	return pcm_bufbytes;
}
//---------------------------------------------------------------------------
// 函数:	Mp3FillBuffer
// 功能:	
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
BOOL KMp3Music::Mp3FillBuffer()
{
	static int mp3_trigger = 2500;
	DWORD dwRead;
	
	// signed var could be negative
	if (mp3_bufbytes < 0)
		mp3_bufbytes = 0;
	
	// read mp3 data to buffer
	if (mp3_bufbytes < mp3_trigger)
	{
		memmove(mp3_buffer, mp3_bufptr, mp3_bufbytes);
		dwRead = Mp3FileRead(mp3_buffer + mp3_bufbytes, 
			MP3_BUFSIZE - mp3_bufbytes);
		mp3_bufbytes += dwRead;
		mp3_bufptr = mp3_buffer;
	}
	
	return TRUE;
}
//---------------------------------------------------------------------------
