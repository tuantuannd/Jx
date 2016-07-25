//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KApplication.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	KApplication Class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KAviFile.h"
/****************************************************************************
AVI RIFF Form
AVI files use the AVI RIFF form. 
The AVI RIFF form is identified by the FOURCC (four-character code) 'AVI '.
All AVI files include two mandatory LIST chunks.
These chunks define the format of the stream and stream data. 
AVI files might also include an index chunk. 
This optional chunk specifies the location of data chunks within the file. 
An AVI file with these components has the following form: 

RIFF ('AVI '
      LIST ('hdrl'
              .
              .
              .
           )
      LIST ('movi'
              .
              .
              .
           )
      ['idx1'<AVI Index>]
     )

The LIST chunks and the index chunk are subchunks of the RIFF 'AVI ' chunk.
The 'AVI ' chunk identifies the file as an AVI RIFF file. 
The LIST 'hdrl' chunk defines the format of the data and is the first 
required LIST chunk. 
The LIST 'movi' chunk contains the data for the AVI sequence and is the 
second required LIST chunk. 
The 'idx1' chunk is the index chunk. 
AVI files must keep these three components in the proper sequence. 
The LIST 'hdrl' and LIST 'movi' chunks use subchunks for their data. 
The following example shows the AVI RIFF form expanded with the chunks 
needed to complete the LIST 'hdrl' and LIST 'movi' chunks: 

RIFF ('AVI '
      LIST ('hdrl'
            'avih'(<Main AVI Header>)
            LIST ('strl'
                  'strh'(<Stream header>)
                  'strf'(<Stream format>)
                  'strd'(<additional header data>)
                  'strn'(<Stream name>)
                  ...
                 )
             .
             .
             .
           )
      LIST ('movi'
            {SubChunk | LIST ('rec '
                              SubChunk1
                              SubChunk2
                                .
                                .
                                .
                             )
               .
               .
               .
            }
            .
            .
            .
           )
      ['idx1'<AVI Index>]
     )

00000000 RIFF (103E2920) 'AVI '
0000000C     LIST (00000146) 'hdrl'
00000018         avih (00000038)
                     dwMicroSecPerFrame    : 33367
                     dwMaxBytesPerSec      : 3728000
                     dwPaddingGranularity  : 0
                     dwFlags               : 0x810 HASINDEX | TRUSTCKTYPE
                     dwTotalFrames         : 2192
                     dwInitialFrames       : 0
                     dwStreams             : 2
                     dwSuggestedBufferSize : 120000
                     dwWidth               : 720
                     dwHeight              : 480
                     dwReserved            : 0x0
00000058         LIST (00000094) 'strl'
00000064             strh (00000038)
                         fccType               : 'vids'
                         fccHandler            : 'dvsd'
                         dwFlags               : 0x0
                         wPriority             : 0
                         wLanguage             : 0x0 undefined
                         dwInitialFrames       : 0
                         dwScale               : 100 (29.970 Frames/Sec)
                         dwRate                : 2997
                         dwStart               : 0
                         dwLength              : 2192
                         dwSuggestedBufferSize : 120000
                         dwQuality             : 0
                         dwSampleSize          : 0
                         rcFrame               : 0,0,720,480
000000A4             strf (00000048)
                         biSize          : 40
                         biWidth         : 720
                         biHeight        : 480
                         biPlanes        : 1
                         biBitCount      : 24
                         biCompression   : 0x64737664 'dvsd'
                         biSizeImage     : 120000
                         biXPelsPerMeter : 0
                         biYPelsPerMeter : 0
                         biClrUsed       : 0
                         biClrImportant  : 0
                         dwDVAAuxSrc     : 0x........
                         dwDVAAuxCtl     : 0x........
                         dwDVAAuxSrc1    : 0x........
                         dwDVAAuxCtl1    : 0x........
                         dwDVVAuxSrc     : 0x........
                         dwDVVAuxCtl     : 0x........
                         dwDVReserved[2] : 0,0
000000F4         LIST (0000005E) 'strl'
00000100             strh (00000038)
                         fccType               : 'auds'
                         fccHandler            : '    '
                         dwFlags               : 0x0
                         wPriority             : 0
                         wLanguage             : 0x0 undefined
                         dwInitialFrames       : 0
                         dwScale               : 1 (32000.000 Samples/Sec)
                         dwRate                : 32000
                         dwStart               : 0
                         dwLength              : 2340474
                         dwSuggestedBufferSize : 4272
                         dwQuality             : 0
                         dwSampleSize          : 4
                         rcFrame               : 0,0,0,0
00000140             strf (00000012)
                         wFormatTag      : 1 PCM
                         nChannels       : 2
                         nSamplesPerSec  : 32000
                         nAvgBytesPerSec : 128000
                         nBlockAlign     : 4
                         wBitsPerSample  : 16
                         cbSize          : 0
00000814     LIST (103D0EF4) 'movi'
103D1710     idx1 (00011210)
****************************************************************************/
//---------------------------------------------------------------------------
// Convert a string of 4 bytes to a long type number,
// also working on big endian machines 
//---------------------------------------------------------------------------
static unsigned long str2ulong(char *str)
{
	unsigned long result;
	
	result = (((unsigned long)str[0] & 0xFF) |
		(((unsigned long)str[1] & 0xFF) << 8) |
		(((unsigned long)str[2] & 0xFF) << 16) | 
		(((unsigned long)str[3] & 0xFF) << 24));

	return result;
}
//---------------------------------------------------------------------------
// Convert a string of 2 bytes to a short type number,
// also working on big endian machines 
//---------------------------------------------------------------------------
static unsigned long str2ushort(char *str)
{
	return (str[0] | (str[1] << 8));
}
//---------------------------------------------------------------------------
// 函数:	KAviFile
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KAviFile::KAviFile()
{
	m_video_pos   = 0;
	m_audio_posc  = 0;
	m_audio_posb  = 0;
	m_idx		  = NULL;
	m_video_index = NULL;
	m_audio_index = NULL;
}
//---------------------------------------------------------------------------
// 函数:	~KAviFile
// 功能:	析构函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KAviFile::~KAviFile()
{
	Close();
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	Try to open an AVI file
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KAviFile::Open(char* Filename)
{
	char head[16];

	Close();

	if (!m_AviFile.Open(Filename))
		return FALSE;

	m_AviFile.Read(head, 12);

	if (memcmp(head, "RIFF", 4) !=0 || 
		memcmp(head + 8, "AVI ", 4) !=0 )
		return FALSE;

	m_video_pos   = 0;
	m_audio_posc  = 0;
	m_audio_posb  = 0;
	m_idx		  = NULL;
	m_video_index = NULL;
	m_audio_index = NULL;
	
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FillHeader
// 功能:	Fill the class with info from headers
//			and reconstruct an index if wanted.
// 参数:	
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KAviFile::FillHeader()
{
	DWORD		n;
	int			i;
	long		rate;
	long		scale;
	char*		hdrl_data;
	char*		hdrl_ptr;
	char*		hdrl_end;
	long		hdrl_len;
	int 		lasttag = 0;
	int 		vids_strh_seen = 0;
	int 		vids_strf_seen = 0;
	int 		auds_strh_seen = 0;
	int 		auds_strf_seen = 0;
	int 		num_stream = 0;
	char		data[16];
	
	// go through the AVI file and extract the header list,
	// the start position of the 'movi' list 
	// and an optionally present idx1 tag 
	
	hdrl_data = 0;
	hdrl_len = 0;
	m_movi_start = 0;

	// seek to first LIST chunk
	m_AviFile.Seek(12, FILE_BEGIN);

	// read two LIST chunks and idx1 chunks
	while (1)
	{
		if (m_AviFile.Read(data, 8) != 8)
			break; 

		// we assume it's EOF 
		n = str2ulong(data + 4);
		n = PAD_EVEN(n);
		
		if (memcmp(data, "LIST", 4) == 0)
		{
			if (m_AviFile.Read(data, 4) != 4 )
				return FALSE;
			
			n -= 4;
			
			if (memcmp(data, "hdrl", 4) == 0)
			{
				hdrl_len = n;
				
				hdrl_data = (char *)g_MemAlloc(n);
				
				if (hdrl_data == NULL)
					return FALSE;
				
				if (m_AviFile.Read(hdrl_data, n) != n)
					return FALSE;
			}
			else if (memcmp(data, "movi", 4) == 0)
			{
				m_movi_start = m_AviFile.Tell();
				
				m_AviFile.Seek(n, FILE_CURRENT);
			}
			else // skip n bytes
			{
				m_AviFile.Seek(n, FILE_CURRENT);
			}
		}
		else if (memcmp(data, "idx1", 4) == 0)
		{
			// n must be a multiple of 16, 
			// but the reading does not
			// break if this is not the case 
			m_idx_num = n / 16;

			m_idx = (char(*)[16])g_MemAlloc(n);
			
			if (m_idx == NULL)
				return FALSE;
			
			if (m_AviFile.Read((char *)m_idx, n) != n)
				break;
		}
		else // skip n bytes
		{
			m_AviFile.Seek(n, FILE_CURRENT);
		}
	}
	
	if (hdrl_data == NULL)
		return FALSE;
	
	if (m_movi_start == 0)
		return FALSE;
	
	// interpret the header list 
	hdrl_ptr = hdrl_data;
	hdrl_end = hdrl_data + hdrl_len;

	while (hdrl_ptr < hdrl_end)
	{
		// list tags are completly ignored 
		if (memcmp(hdrl_ptr, "LIST", 4) == 0)
		{ 
			hdrl_ptr += 12;
			continue;
		}
		
		n = str2ulong(hdrl_ptr + 4);
		n = PAD_EVEN(n);
		
		// interpret the tag and its args 
		if (memcmp(hdrl_ptr, "strh", 4) == 0)
		{
			hdrl_ptr += 8;
			if (memcmp(hdrl_ptr, "vids", 4) == 0 && !vids_strh_seen)
			{
				memcpy(m_compressor, hdrl_ptr + 4, 4);
				
				m_compressor[4] = 0;
				
				scale = str2ulong(hdrl_ptr + 20);
				rate  = str2ulong(hdrl_ptr + 24);
				
				if (scale != 0) 
					m_fps = rate / scale;
				
				m_video_frames = str2ulong(hdrl_ptr + 32);
				m_video_strn = num_stream;
				
				vids_strh_seen = 1;
				lasttag = 1;
			}
			else if (memcmp(hdrl_ptr, "auds", 4) == 0 && !auds_strh_seen)
			{
				m_audio_strn = num_stream;
				
				auds_strh_seen = 1;
				lasttag = 2;
			}
			else
			{
				lasttag = 0;
			}
			num_stream++;
		}
		else if (memcmp(hdrl_ptr, "strf", 4) == 0)
		{
			hdrl_ptr += 8;
			if (lasttag == 1)
			{
				// keep a copy of the bitmapinfoheader
				memcpy(&m_BitmapInfoHeader, 
					hdrl_ptr,
					sizeof(BITMAPINFOHEADER));	
				
				vids_strf_seen = 1;
			}
			else if (lasttag == 2)
			{
				// keep a copy of the WAVEFORMATEX
				memcpy(&m_WaveFormatEx,
					hdrl_ptr,
					sizeof(WAVEFORMATEX));
				
				auds_strf_seen = 1;
			}
			lasttag = 0;
		}
		else
		{
			hdrl_ptr += 8;
			lasttag = 0;
		}
		
		hdrl_ptr += n;
	}
	
	g_MemFree(hdrl_data);
	
	if (!vids_strh_seen || !vids_strf_seen || m_video_frames == 0)
	{ 
		return FALSE;
	}
	
	m_video_tag[0] = m_video_strn / 10 + '0';
	m_video_tag[1] = m_video_strn % 10 + '0';
	m_video_tag[2] = 'd';
	m_video_tag[3] = 'b';
	
	// audio tag is set to "99wb" if no audio present 
	if (m_WaveFormatEx.nChannels == 0)
		m_audio_strn = 99;
	
	m_audio_tag[0] = m_audio_strn / 10 + '0';
	m_audio_tag[1] = m_audio_strn % 10 + '0';
	m_audio_tag[2] = 'w';
	m_audio_tag[3] = 'b';
	
	// check m_idx
	if (m_idx == NULL)
		return FALSE;

	// if the file has an idx1, check if this is relative
	// to the start of the file or to the start of the movi list 
	m_idx_type = 0;

	// search the first videoframe in the idx1 
	// and look where it is in the file
	for (i = 0; i < m_idx_num; i++)
	{
		if (memcmp(m_idx[i], m_video_tag, 3) == 0)
			break;
	}

	if (i >= m_idx_num)
		return FALSE;
	
	unsigned long pos = str2ulong(m_idx[i] + 8);
	unsigned long len = str2ulong(m_idx[i] + 12);
	
	m_AviFile.Seek(pos, FILE_BEGIN);
	
	if (m_AviFile.Read(data, 8) != 8) 
		return FALSE;
	
	if (memcmp(data, m_idx[i], 4) == 0 && str2ulong(data + 4) == len)
	{
		// index from start of file
		m_idx_type = 1; 
	}
	else
	{
		m_AviFile.Seek(pos + m_movi_start - 4, FILE_BEGIN);
		
		if (m_AviFile.Read(data, 8) != 8)
			return FALSE;
		
		if (memcmp(data, m_idx[i], 4) == 0 && str2ulong(data + 4) == len)
		{
			// index from start of movi list 
			m_idx_type = 2; 
		}
	}

	// m_idx_type remains 0 if neither of the two tests above succeeds 
	if (m_idx_type == 0)
		return FALSE;

	return TRUE;
}	
//---------------------------------------------------------------------------
// 函数:	GetAudioIndex
// 功能:	Generate the audio index arrays
// 参数:	
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KAviFile::GetAudioIndex()
{
	int i, nai, ioff, tot;

	if (!FillHeader())
		return FALSE;

	nai = 0;

	for (i = 0; i < m_idx_num; i++)
	{
		if (memcmp(m_idx[i], m_audio_tag, 4) == 0) 
			nai++;
	}
	
	m_audio_chunks = nai;
	
	if (m_audio_chunks == 0)
		return FALSE;

	m_audio_index = (audio_index_entry *)g_MemAlloc(nai * sizeof(audio_index_entry));

	if (m_audio_index == 0)
		return FALSE;
	
	nai = 0;
	tot = 0;
	
	ioff = (m_idx_type == 1)? 8 : m_movi_start + 4;
	
	for (i = 0; i < m_idx_num; i++)
	{
		if (memcmp(m_idx[i], m_audio_tag, 4) == 0)
		{
			m_audio_index[nai].pos = str2ulong(m_idx[i] + 8) + ioff;
			m_audio_index[nai].len = str2ulong(m_idx[i] + 12);
			m_audio_index[nai].tot = tot;
			
			tot += m_audio_index[nai].len;
			nai++;
		}
	}
	
	m_audio_bytes = tot;

	g_MemFree(m_idx);
	m_idx = NULL;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	GetVideoIndex
// 功能:	generate the video index arrays 
// 参数:	
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KAviFile::GetVideoIndex()
{
	int i, nvi, ioff;

	if (!FillHeader())
		return FALSE;

	nvi = 0;
	
	for (i = 0; i < m_idx_num; i++)
	{
		if (memcmp(m_idx[i], m_video_tag, 3) == 0) 
			nvi++;
	}
	
	m_video_frames = nvi;
	
	if (m_video_frames == 0) 
		return FALSE;
	
	m_video_index = (video_index_entry *)g_MemAlloc(nvi * sizeof(video_index_entry));
	
	if (m_video_index == 0)
		return FALSE;
	
	nvi = 0;
	
	ioff = (m_idx_type == 1)? 8 : m_movi_start + 4;
	
	for (i = 0; i < m_idx_num; i++)
	{
		if (memcmp(m_idx[i], m_video_tag, 3) == 0)
		{
			m_video_index[nvi].flags = str2ulong(m_idx[i] + 4);
			m_video_index[nvi].pos	 = str2ulong(m_idx[i] + 8) + ioff;
			m_video_index[nvi].len	 = str2ulong(m_idx[i] + 12);

			nvi++;
		}
	}
	
	m_video_pos = 0;
	
	g_MemFree(m_idx);
	m_idx = NULL;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	VideoStreams
// 功能:	Returns the total number of video streams
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::VideoStreams()
{
	return m_video_strn;
}
//---------------------------------------------------------------------------
// 函数:	AudioStreams
// 功能:	Returns the total number of audio streams
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::AudioStreams()
{
	return m_audio_strn;
}
//---------------------------------------------------------------------------
// 函数:	GetBitmapInfoHeader
// 功能:	Returns the bitmapinfoheader associated with the first video stream
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
void KAviFile::GetBitmapInfoHeader(PBITMAPINFOHEADER pBmpInfoHead)
{
	*pBmpInfoHead = m_BitmapInfoHeader;
}
//---------------------------------------------------------------------------
// 函数:	GetWaveFormat
// 功能:	Returns the wavefromatex associated with the first audio stream.
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
void KAviFile::GetWaveFormat(PWAVEFORMATEX pWavFmt)
{
	*pWavFmt = m_WaveFormatEx;
}
//---------------------------------------------------------------------------
// 函数:	GetCompressMethod
// 功能:	取得压缩方法
// 参数:	method		at least 5 char buffer
// 返回:	void
//---------------------------------------------------------------------------
void KAviFile::GetCompressMethod(char* method)
{
	memcpy(method, m_compressor, 5);
}
//---------------------------------------------------------------------------
// 函数:	NextFrame
// 功能:	Reads the next video Frame into buffer读入下一帧视频数据
// 参数:	
// 返回:	return the actual size of the frame
//---------------------------------------------------------------------------
int KAviFile::NextFrame(unsigned char* buffer)
{
	int nNeedBytes;
	int nReadBytes;
	
	if (!m_video_index)		   
		return -1;
	
	if (m_video_pos < 0 || m_video_pos >= m_video_frames)
		return -2;
	
	nNeedBytes = m_video_index[m_video_pos].len;
	
	m_AviFile.Seek(m_video_index[m_video_pos].pos, FILE_BEGIN);

	nReadBytes = m_AviFile.Read(buffer, nNeedBytes);

	m_video_pos++;

	return nReadBytes;
}
//---------------------------------------------------------------------------
// 函数:	ReadAudio
// 功能:	Reads any amount of audio data
// 参数:	
// 返回:	FIXME : should return the actual number read.
//---------------------------------------------------------------------------
int KAviFile::ReadAudio(unsigned char* audbuf, int bytes)
{
	int nr, pos, left, todo;
	
	if (!m_audio_index)
		return -1;
	
	nr = 0; 
	
	// We loop until we parsed enough
	// chunks for the amount we want
	while (bytes > 0)
	{
		left = m_audio_index[m_audio_posc].len - m_audio_posb;
		
		if (left == 0)
		{
			if (m_audio_posc >= m_audio_chunks - 1)
			{
				return nr;
			}
			m_audio_posc++;
			m_audio_posb = 0;
			continue;
		}

		if (bytes < left)
			todo = bytes;
		else
			todo = left;
		
		pos = m_audio_index[m_audio_posc].pos + m_audio_posb;
		
		m_AviFile.Seek(pos, FILE_BEGIN);
		
		m_AviFile.Read(audbuf + nr, todo);

		bytes -= todo;
		nr	  += todo;
		m_audio_posb += todo;
	}

	return nr;
}
//---------------------------------------------------------------------------
// 函数:	FrameRate
// 功能:	Return the actual framerate（frame per second）
// 参数:	
// 返回:	FIXME : should be a double...
//---------------------------------------------------------------------------
int KAviFile::FrameRate()
{
	// Fix for some trailers
	if (m_fps == 0)
		m_fps = 25;
	
	if (m_fps == 23)
		m_fps = 25;

	return m_fps;
}
//---------------------------------------------------------------------------
// 函数:	CurrentFrame
// 功能:	
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::CurrentFrame()
{
	return m_video_pos;
}
//---------------------------------------------------------------------------
// 函数:	TotalFrames
// 功能:	Return the total frames
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::TotalFrames()
{
	return m_video_frames;
}
//---------------------------------------------------------------------------
// 函数:	VideoSeek
// 功能:	Seek to a particular video frame，此帧为关键帧
// 参数:	percent(0--100)
// 返回:	frame number
//---------------------------------------------------------------------------
int KAviFile::VideoSeek(int percent)
{
	int frame;

	if (!m_video_index)
		return -1; 

	// limit to 0-100
	if (percent < 0)
		percent = 0;
	if (percent > 100)
		frame = 100;

	// compute the desired frame number
	frame = percent * m_video_frames / 100;
	
	// and go to the next keyframe.
	while (!IsKeyframe(frame))
	{
		frame++;
	}

	m_video_pos = frame;
	
	return frame;
}
//---------------------------------------------------------------------------
// 函数:	AudioSeek
// 功能:	Seek to a particular audio frame.
// 参数:	percet（0--100）
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::AudioSeek(int percent)
{
	int frame;
	int bytes;
	int n0, n1, n;

	if (!m_audio_index)
		return -1; 

	// limit to 0-100
	if (percent < 0)
		percent = 0;
	if (percent > 100)
		frame = 100;

	// compute the desired frame number
	frame = percent * m_video_frames / 100;

	// and set audio position
	bytes = frame * m_audio_bytes / m_video_frames;
	bytes += bytes % 4;
		
	if (bytes < 0) 
		bytes = 0;
	
	n0 = 0;
	n1 = m_audio_chunks;
	
	while (n0 < n1 - 1)
	{
		n = (n0 + n1) / 2;
		if (m_audio_index[n].tot > bytes)
			n1 = n;
		else
			n0 = n;
	}
	
	m_audio_posc = n0;
	
	if (m_audio_index[n0].len > 1000)
	{
		m_audio_posb = bytes - m_audio_index[n0].tot;
	}
	else
	{
		m_audio_posb = 0;
	}
	
	return frame;
}
//---------------------------------------------------------------------------
// 函数:	IsKeyFrame
// 功能:	当前帧是否关键帧
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
BOOL KAviFile::IsKeyframe(int frame)
{
	// we still return 1 to avoid looping on waiting for a keyframe.
	if (!m_video_index)		   
		return 1; 

	if (frame < 0)
		frame = 0;
	
	return m_video_index[frame].flags & AVIIF_KEYFRAME;
}
//---------------------------------------------------------------------------
// 函数:	NextKeyFrame
// 功能:	到下一个关键帧
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::NextKeyFrame()
{
	// Allways increment by one
	m_video_pos++;
	
	while (!IsKeyframe(m_video_pos) && m_video_pos < m_video_frames)
		m_video_pos++;
	
	return 1;
}
//---------------------------------------------------------------------------
// 函数:	PreviousKeyFrame
// 功能:	回到上一个关键帧
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::PreviousKeyFrame()
{
	// Allways decrement by two since we read the last frame
	m_video_pos--;
	m_video_pos--;
	
	while (!IsKeyframe(m_video_pos) && m_video_pos > 0)
		m_video_pos--;
	
	return 1;
}
//---------------------------------------------------------------------------
// 函数:	Rewind
// 功能:	反卷回起始
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
int KAviFile::Rewind()
{
	m_video_pos  = 0;
	m_audio_posc = 0;
	m_audio_posb = 0;
	
	return 1;
}
//---------------------------------------------------------------------------
// 函数:	Close
// 功能:	关闭释放
// 参数:	
// 返回:	
//---------------------------------------------------------------------------
void KAviFile::Close()
{
	if (m_video_index)
	{
		g_MemFree(m_video_index);
		m_video_index = NULL;
	}
	if (m_audio_index)
	{
		g_MemFree(m_audio_index);
		m_audio_index = NULL;
	}
}
//---------------------------------------------------------------------------
