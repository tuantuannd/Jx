//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KBmpFile24.h
// Date:	2000.08.08
// Code:	Daniel Wang
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KTgaFile32_H
#define KTgaFile32_H
//---------------------------------------------------------------------------
#include "KBitmap16.h"

typedef struct
{
	BYTE		IDLength;					// ID length
	BYTE		ColorMapType;				// Color map type
	BYTE		ImageType;					// Image type
	BYTE		ColorMapSpec[5];			// Color map specification
	WORD		X,Y;							// Image specification
	WORD		Width,Height;
	BYTE		PixelDep;
	BYTE		Desc;
} TGAFILEHEADER;

typedef struct
{
	short		Size;						// Extension size
	char		AuthorName[41];				// Author name
	char		AuthorCmts[324];			// Author comments
	short		DateTimeStamp[6];			// Date time stamp
	char		JobID[41];					// Job name/ID
	short		JobTime[3];					// Job time
	char		SoftwareID[41];				// Software ID
	BYTE		SoftWareVer[3];				// Software Version
	int			KeyColor;					// Key color
	short		PixelRatio[2];				// Pixel aspect ratio
	short		GammaVal[2];				// Gamma value
	int			ClrCorrOft;					// Color correction offset
	int			PostageStampOft;			// Postage stamp offset
	int			ScanLineOft;				// Scan line offset
	BYTE		AttribType;					// Attribute type
} TGAEXTAREAHEADER;

//---------------------------------------------------------------------------
class ENGINE_API KTgaFile32
{
public:
	KMemClass	m_Buffer;
private:
	long		m_nWidth;
	long		m_nHeight;

public:
	long		GetWidth()	{ return m_nWidth; };
	long		GetHeight() { return m_nHeight; };
	BOOL		Load2Buffer(LPSTR lpFileName);
};
//---------------------------------------------------------------------------
#endif
