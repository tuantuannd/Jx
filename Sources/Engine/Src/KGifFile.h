//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KGifFile.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KGifFile_H
#define KGifFile_H
//---------------------------------------------------------------------------
#include "KMemClass.h"
#include "KBitmap.h"
//---------------------------------------------------------------------------
/*
 * GIF block identifiers
 */
#define GIF_EXTENSION     0x21   /* GIF extension block               */
#define GIF_IMAGE_DESC    0x2c   /* GIF image description block       */
#define GIF_TRAILER       0x3b   /* GIF trailer (end of data stream)  */
#define GIF_TEXT_EXT      0x01   /* GIF text extension block          */
#define GIF_COMMENT_EXT   0xFE   /* GIF comment block                 */
#define GIF_GRAPHIC_EXT   0xF9   /* GIF graphic control block         */
#define GIF_APP_EXT       0xFF   /* GIF application extension block   */
#define GIF_TRANSPARENT   0x01   /* image uses transparent colour     */
#define GIF_USERINPUT     0x02   /* wait for user input before frame  */
#define GIF_DISPOSAL      0x1C   /* image disposal method             */

/*
 * GIF bit flags (some are for local or global colour tables or both)
 */
#define GIF_COLTAB        0x80   /* GIF colour table exists           */
#define GIF_COLTABRES     0x70   /* GIF colour table resolution       */
#define GIF_INTERLACE     0x40   /* GIF image is interlaced           */
#define GIF_LCOLTABSORT   0x20   /* GIF local colour table is sorted  */
#define GIF_GCOLTABSORT   0x08   /* GIF global colour table is sorted */
#define GIF_COLTABSIZ     0x07   /* GIF colour table size             */

/*
 * GIF color table entry
 */
typedef struct gif_color_struct
{
	BYTE red;
	BYTE green;
	BYTE blue;
} gif_color;

/*
 * GIF header 
 */
typedef struct gif_header_struct
{
	BYTE signature[3];     /* header signature - always "GIF"  */
	BYTE version[3];       /* version - "87a" or "89a"         */
	WORD screen_width;     /* width of logical screen          */
	WORD screen_height;    /* height of logical screen         */
	BYTE packed;           /* colour table information         */
	BYTE background;       /* background colour index          */
	BYTE aspect_ratio;     /* pixel aspect ratio               */
} gif_header;

/*
 *  GIF image descriptor.
 */
typedef struct gif_img_desc_struct
{
	BYTE seperator;        /* image descriptor identifier - 2Ch  */
	WORD left;             /* X position of image                */
	WORD top;              /* Y position of image                */
	WORD width;            /* width of image                     */
	WORD height;           /* height of image                    */
	BYTE packed;           /* image and colour table information */
} gif_image_desc;

/*
 *  GIF 89a graphic control extension 
 */
typedef struct gif_graphic_control_struct
{
	BYTE introducer;       /* extension introducer - 21h         */
	BYTE label;            /* extension label - F9h              */
	BYTE blocksize;        /* size of remaining fields - 04h     */
	BYTE packed;           /* transparency/input/disposal flags  */
	WORD delay;            /* delay before display (1/100 sec)   */
	BYTE transparent;      /* transparent colour index           */
	BYTE terminator;       /* block terminator - 0               */
} gif_graphic_control;

/*
 *  GIF 89a plain text extension
 */
typedef struct gif_plain_text_struct
{
	BYTE introducer;       /* extension introducer - 21h         */
	BYTE label;            /* extension label - 01h              */
	BYTE blocksize;        /* size of remaining fields - 0Ch     */
	WORD text_grid_left;   /* X position of text grid            */
	WORD text_grid_top;    /* Y position of text grid            */
	WORD text_grid_width;  /* width of text grid                 */
	WORD text_grid_height; /* height of text grid                */
	BYTE cell_width;       /* width of a grid cell               */
	BYTE cell_height;      /* height of a grid cell              */
	BYTE text_fg_colour;   /* foreground colour index            */
	BYTE text_bg_colour;   /* background colour index            */
	BYTE *plain_text_data; /* plain text data                    */
	BYTE terminator;       /* block terminator - 0               */
} gif_plain_text;

/*
 *  GIF 89a application extension
 */
typedef struct gif_app_ext_struct
{
	BYTE introducer;       /* extension introducer - 21h         */
	BYTE label;            /* extension label - FFh              */
	BYTE blocksize;        /* size of remaining fields - 0Bh     */
	BYTE identifier[8];    /* application identifier             */
	BYTE authcode[3];      /* application authentication code    */
	BYTE *app_data;        /* application data sub-blocks        */
	BYTE terminator;       /* block terminator - 0               */
} gif_app_ext;

/*
 *  GIF 89a comment extension
 */
typedef struct gif_comment_struct
{
	BYTE introducer;       /* extension introducer - 21h         */
	BYTE label;            /* extension label - FEh              */
	BYTE *comment_data;    /* comment data sub-blocks            */
	BYTE terminator;       /* block terminator - 0               */
} gif_comment;

/*
 *  LZW String
 */
typedef struct lzw_string_struct
{
	short base;
	char  nnew;
	short length;
} lzw_string;
//------------------------------------------------------------------------------
class ENGINE_API KGifFile
{
// member for image
private:
	KMemClass	m_Buffer;
	KBitmap*	m_pBitmap;
	DWORD		m_dwDataPos;
	DWORD		m_dwCurrPos;
	WORD		m_wWidth;
	WORD		m_wHeight;
	BYTE		m_byBackground;
	int			m_nColors;

// member for lzw decode
private:
	lzw_string	str[4096];
	BYTE		string[4096];
	int			empty_string;
	int			curr_bit_size;
	int			bit_overflow;
	int			bit_pos;
	int			data_pos;
	int			data_len;
	int			entire;
	int			code;
	int			cc;
	int			string_length;
	int			bit_size;
	int			image_x;
	int			image_y;
	int			image_w;
	int			image_h;
	int			screen_x;
	int			screen_y;
	int			interlace;

// gif read functions
private:
	BYTE		read_byte();
	WORD		read_word();
	BOOL		read_header();
	BOOL		read_color_table();
	BOOL		read_image_desc();
	BOOL		read_extension();

// gif write functions
private:
	void		write_byte(BYTE byte);
	void		write_word(WORD word);
	BOOL		write_header();
	BOOL		write_color_table();
	BOOL		write_image_desc();
	BOOL		write_extension();

// lzw decode functions
private:
	void		lzw_decode();
	void		clear_table();
	void		get_code();
	void		get_string(int num);
	void		output_string();

// lzw encode functions
private:
	void		lzw_encode();

// public functions
public:
	KGifFile();
	BOOL		Open(LPSTR lpFileName, KBitmap* lpBitmap);
	BOOL		Save(LPSTR lpFileName, KBitmap* lpBitmap);
	BOOL		FirstImage();
	BOOL		NextImage();
};
//------------------------------------------------------------------------------
#endif
