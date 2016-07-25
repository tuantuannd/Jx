//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KGifFile.cpp
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Gif file read class
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KPakFile.h"
#include "KGifFile.h"
//---------------------------------------------------------------------------
// 函数:	KGifFile
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KGifFile::KGifFile()
{
	m_dwDataPos	= 0;
	m_dwCurrPos	= 0;
	m_nColors	= 0;
	m_pBitmap	= NULL;
}
//---------------------------------------------------------------------------
// 函数:	read_byte
// 功能:	Read a byte from file buffer
// 参数:	void
// 返回:	BYTE
//---------------------------------------------------------------------------
BYTE KGifFile::read_byte()
{
	PBYTE pByte = (PBYTE)m_Buffer.GetMemPtr();
	pByte += m_dwCurrPos;
	m_dwCurrPos++;
	return *pByte;
}
//---------------------------------------------------------------------------
// 函数:	read_word
// 功能:	Read a word from file buffer
// 参数:	void
// 返回:	WORD
//---------------------------------------------------------------------------
WORD KGifFile::read_word()
{
	PBYTE pByte = (PBYTE)m_Buffer.GetMemPtr();
	pByte += m_dwCurrPos;
	m_dwCurrPos ++;
	m_dwCurrPos ++;
	return *((PWORD)pByte);
}
//---------------------------------------------------------------------------
// 函数:	read_header
// 功能:	Read gif header from file buffer
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KGifFile::read_header()
{
	BYTE signature[3];
	BYTE version[3];
	BYTE packed_flag;
	BYTE aspect_ratio;

	// read header
	signature[0]	= read_byte();
	signature[1]	= read_byte();
	signature[2]	= read_byte();
	version[0]		= read_byte();
	version[1]		= read_byte();
	version[2]		= read_byte();
	m_wWidth		= read_word();
	m_wHeight		= read_word();
	packed_flag		= read_byte();
	m_byBackground	= read_byte();
	aspect_ratio	= read_byte();

	// check signature
	if (!g_MemComp(signature, "GIF", 3))
		return FALSE;

	// check version
	if ((!g_MemComp(version, "87a", 3)) &&
		(!g_MemComp(version, "89a", 3)))
		return FALSE;

	// If there isn't a global colour table, exit.
	if ((packed_flag & 0x80) == 0)
	{
		m_nColors = 0;
		return FALSE;
	}

	// Get the number of colour planes in the original image.
	int planes = (packed_flag & 0x07) + 1;

	// Get the size of the global colour table.
	m_nColors = 1 << planes;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	read_color_table
// 功能:	Read a GIF color table from file buffer
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KGifFile::read_color_table()
{
	KPAL32* pPal32;

	pPal32 = m_pBitmap->GetPal32();
	for (int i = 0; i < m_nColors; i++)
	{
		pPal32[i].Red   = read_byte();
		pPal32[i].Green = read_byte();
		pPal32[i].Blue  = read_byte();
	}
	m_pBitmap->MakePalette();
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	read_image_desc
// 功能:	Read image descriptor
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KGifFile::read_image_desc()
{
	/* image x, y offset */
	image_x = read_word();
	image_y = read_word();

	/* individual image dimensions */
	image_w = read_word();
	image_h = read_word();

	/* read packed flag byte */
	int i = read_byte();

	/* interlace flag */
	if (i & 64)
		interlace = 8;
	else
		interlace = 1;

	/* there is local palette */
	if (i & 128)
		return FALSE;

	/* lzw stream starts now */
	lzw_decode();

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	read_extension
// 功能:	Read extension block
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KGifFile::read_extension()
{
	int i;

	// read label 
	i = read_byte(); 

	// Graphic Control Extension
	if (i == 0xF9)
	{
		read_byte(); /* skip size (it's 4) */
		BYTE flag = read_byte(); /* packed flag byte */
		read_byte(); /* delay time */
		BYTE transc = read_byte(); /* transparent colour */
		read_byte(); /* terminater */
		if (flag & 1) /* is transparency enabled? */
		{
			m_pBitmap->Clear(transc);
		}
	}
	
	// skip Data Sub-blocks
	while ((i = read_byte()) > 0)
	{
		while (i-- > 0)
		{
			read_byte();
		}
	}
	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	Open
// 功能:	打开GIF文科
// 参数:	lpFileName	文件名
//			lpBitmap	8bit位图
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
BOOL KGifFile::Open(LPSTR FileName, KBitmap* lpBitmap)
{
	KPakFile File;

	m_pBitmap = lpBitmap;

	if (!File.Open(FileName))
		return FALSE;

	if (!m_Buffer.Alloc(File.Size()))
		return FALSE;

	File.Read(m_Buffer.GetMemPtr(), File.Size());


	if (!read_header())
		return FALSE;

	if (!read_color_table())
		return FALSE;

	m_dwDataPos = m_dwCurrPos;

	if (!m_pBitmap->Init(m_wWidth, m_wHeight, m_nColors))
		return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	FirstImage
// 功能:	Get first image of gif file
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KGifFile::FirstImage()
{
	m_dwCurrPos = m_dwDataPos;
	return NextImage();
}
//---------------------------------------------------------------------------
// 函数:	NextImage
// 功能:	Get next image of gif file
// 参数:	void
// 返回:	BOOL
//---------------------------------------------------------------------------
BOOL KGifFile::NextImage()
{
	int i;

	if (m_Buffer.GetMemPtr() == NULL)
		return FALSE;

	if (m_pBitmap == NULL)
		return FALSE;

	m_pBitmap->Clear(m_byBackground);

	while (TRUE)
	{
		i = read_byte();
		switch (i)
		{
			case GIF_IMAGE_DESC: /* Image Descriptor */
				if (!read_image_desc())
					return FALSE;
				return TRUE;

			case GIF_EXTENSION: /* Extension Introducer */
				if (!read_extension())
					return FALSE;
				break;

			case 0x00: /* Indicate reading one image completely */
				break;

			case GIF_TRAILER: /* Trailer - end of data */
				return FALSE;

			default: /* there is some error in gif */
				return FALSE;
		}
	}

	return TRUE;
}
//---------------------------------------------------------------------------
// 函数:	lzw_decode
// 功能:	LZW Decoder
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KGifFile::lzw_decode()
{
	int i, old;

	bit_size = read_byte();
	cc = 1 << bit_size;
	
	/* initialise string table */
	for (i = 0; i < cc; i ++)
	{
		str[i].base = -1;
		str[i].nnew = i;
		str[i].length = 1;
	}
	
	/* initialise the variables */
	bit_pos = 0;
	data_len = read_byte();
	data_pos = 0;
	entire = read_byte();
	data_pos ++;
	string_length = 0;
	screen_x = image_x;
	screen_y = image_y;
	
	/* starting code */
	clear_table();
	get_code();
	if (code == cc)
		get_code();
	get_string(code);
	output_string();
	old = code;
	
	while (TRUE)
	{
		get_code();
		
		if (code == cc)
		{
			/* starting code */
			clear_table();
			get_code();
			get_string(code);
			output_string();
			old = code;
		}
		else if(code == cc + 1)
		{
			break;
		}
		else if(code < empty_string)
		{
			get_string(code);
			output_string();
			
			if (bit_overflow == 0)
			{
				str[empty_string].base = old;
				str[empty_string].nnew = string[0];
				str[empty_string].length = str[old].length + 1;
				empty_string ++;
				if (empty_string == (1 << curr_bit_size))
					curr_bit_size ++;
				if (curr_bit_size == 13)
				{
					curr_bit_size = 12;
					bit_overflow = 1;
				}
			}
			
			old = code;
		}
		else
		{
			get_string(old);
			string[str[old].length] = string[0];
			string_length ++;
			
			if (bit_overflow == 0)
			{
				str[empty_string].base = old;
				str[empty_string].nnew = string[0];
				str[empty_string].length = str[old].length + 1;
				empty_string ++;
				if (empty_string == (1 << curr_bit_size))
					curr_bit_size ++;
				if (curr_bit_size == 13) 
				{
					curr_bit_size = 12;
					bit_overflow = 1;
				}
			}
			
			output_string();
			old = code;
		}
	}
}
//---------------------------------------------------------------------------
// 函数:	clear_table
// 功能:	LZW Decoder
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KGifFile::clear_table()
{
	empty_string = cc + 2;
	curr_bit_size = bit_size + 1;
	bit_overflow = 0;
}
//---------------------------------------------------------------------------
// 函数:	get_code
// 功能:	LZW Decoder
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KGifFile::get_code()
{
	if (bit_pos + curr_bit_size > 8)
	{
		if (data_pos >= data_len)
		{
			data_len = read_byte();
			data_pos = 0;
		}
		entire = (read_byte() << 8) + entire;
		data_pos ++;
	}
	if (bit_pos + curr_bit_size > 16)
	{
		if (data_pos >= data_len)
		{
			data_len = read_byte();
			data_pos = 0;
		}
		entire = (read_byte() << 16) + entire;
		data_pos ++;
	}
	code = (entire >> bit_pos) & ((1 << curr_bit_size) - 1);
	if (bit_pos + curr_bit_size > 8)
		entire >>= 8;
	if (bit_pos + curr_bit_size > 16)
		entire >>= 8;
	bit_pos = (bit_pos + curr_bit_size) % 8;
	if (bit_pos == 0)
	{
		if (data_pos >= data_len)
		{
			data_len = read_byte();
			data_pos = 0;
		}
		entire = read_byte();
		data_pos ++;
	}
}
//---------------------------------------------------------------------------
// 函数:	get_string
// 功能:	LZW Decoder
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KGifFile::get_string(int num)
{
	int i;

	if (num < cc)
	{
		string_length = 1;
		string[0] = str[num].nnew;
	}
	else
	{
		i = str[num].length;
		string_length = i;
		while (i > 0)
		{
			i --;
			string[i] = str[num].nnew;
			num = str[num].base;
		}
		/* if(num != -1) **-{[ERROR]}-** */
	}
}
//---------------------------------------------------------------------------
// 函数:	output_string
// 功能:	LZW Decoder
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KGifFile::output_string(void)
{
	int i;

	for (i = 0; i < string_length; i ++)
	{
		m_pBitmap->PutPixel(screen_x, screen_y, string[i]);
		screen_x ++;
		if (screen_x >= image_x + image_w)
		{
			screen_x = image_x;
			screen_y += interlace;
			if (interlace)
			{
				if (screen_y >= image_y + image_h)
				{
					if (interlace == 8 && (screen_y - image_y) % 8 == 0)
					{
						interlace = 8;
						screen_y = image_y + 4;
					}
					else if (interlace == 8  && (screen_y - image_y) % 8 == 4)
					{
						interlace = 4;
						screen_y = image_y + 2;
					}
					else if (interlace == 4)
					{
						interlace = 2;
						screen_y = image_y + 1;
					}
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
