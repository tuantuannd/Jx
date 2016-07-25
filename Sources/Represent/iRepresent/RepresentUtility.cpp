#include "RepresentUtility.h"
#include <Gdiplus.h>
using namespace Gdiplus;

static ULONG_PTR gdiplusToken;
static bool bGdiplusInitOk = false;

bool InitGdiplus()
{
	if(bGdiplusInitOk)
		return true;

	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	if(GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Ok)
		return false;

	bGdiplusInitOk = true;
	return true;
}

void ShutdownGdiplus()
{
	GdiplusShutdown(gdiplusToken);
	bGdiplusInitOk = false;
}

// 获得图象文件编码器类ID
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

bool SaveBufferToJpgFile32(LPSTR lpFileName, PVOID pBitmap, int nPitch, 
										   int nWidth, int nHeight, unsigned int nQuality)
{
	if(!bGdiplusInitOk)
		return false;

	int i, j;
	int nLineAdd;
	LPBYTE lpDes, lpSrc;
	WCHAR szWFileName[512];
	// byte per line % 4 must = 0
	int nBytesPerLine = nWidth * 3;
	if ((nBytesPerLine % 4) != 0)
		nBytesPerLine = nBytesPerLine + 4 - (nBytesPerLine % 4);
	DWORD OffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	LPBYTE pBuffer = new BYTE[nHeight * nBytesPerLine];
	if(!pBuffer)
		return false;

	CLSID encoderClsid;
	EncoderParameters encoderParameters;
	ULARGE_INTEGER fileSize;
	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;
	Image *pImage = NULL;
	LPSTREAM pStream;
	if(CreateStreamOnHGlobal(NULL, true, &pStream) != S_OK)
	{
		pStream = NULL;
		goto error;
	}

	fileSize.HighPart = 0;
	fileSize.LowPart = sizeof(FileHeader) + sizeof(InfoHeader) + nBytesPerLine * nHeight;
	pStream->SetSize(fileSize);

	// file header
	FileHeader.bfType          = 0x4d42; // "BM"
	FileHeader.bfSize          = nBytesPerLine * nHeight + OffBits;
	FileHeader.bfReserved1     = 0;
	FileHeader.bfReserved2     = 0;
	FileHeader.bfOffBits       = OffBits;

	// info header
	InfoHeader.biSize          = sizeof(BITMAPINFOHEADER);
	InfoHeader.biWidth         = nWidth;
	InfoHeader.biHeight        = nHeight;
	InfoHeader.biPlanes        = 1;
	InfoHeader.biBitCount      = 24;
	InfoHeader.biCompression   = 0;
	InfoHeader.biSizeImage     = 0;
	InfoHeader.biXPelsPerMeter = 0xb40;
	InfoHeader.biYPelsPerMeter = 0xb40;
	InfoHeader.biClrUsed       = 0;
	InfoHeader.biClrImportant  = 0;

	// write file head
	pStream->Write(&FileHeader, sizeof(FileHeader), NULL);

	// write info head
	pStream->Write(&InfoHeader, sizeof(InfoHeader), NULL);

	// encode bitmap
	lpDes = pBuffer;
	lpSrc = (LPBYTE)pBitmap;

	nLineAdd = nPitch - nWidth * 4;
	lpDes += (nHeight - 1) * nBytesPerLine;
	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			lpDes[0] = lpSrc[0];
			lpDes[1] = lpSrc[1];
			lpDes[2] = lpSrc[2];
			lpDes += 3;
			lpSrc += 4;
		}
		lpDes -= 3 * nWidth;
		lpDes -= nBytesPerLine;
		lpSrc += nLineAdd;
	}

	// write bitmap bits data
	pStream->Write(pBuffer, nBytesPerLine * nHeight, NULL);
	pImage = new Image(pStream);

	// Get the CLSID of the JPEG encoder.
	if(GetEncoderClsid(L"image/jpeg", &encoderClsid) == -1)
		goto error;

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	// Save the image as a JPEG with quality level 0.
	encoderParameters.Parameter[0].Value = &nQuality;

	MultiByteToWideChar(CP_ACP, 0, lpFileName, strlen(lpFileName)+1, szWFileName, 510);

	if(pImage->Save(szWFileName, &encoderClsid, &encoderParameters) != Ok)
		goto error;

	SAFE_DELETE(pImage);
	SAFE_DELETE_ARRAY(pBuffer);
	if(pStream)
	{
		pStream->Release();
		pStream = NULL;
	}
	return true;

error:
	SAFE_DELETE(pImage);
	SAFE_DELETE_ARRAY(pBuffer);
	if(pStream)
	{
		pStream->Release();
		pStream = NULL;
	}
	return false;
}

bool SaveBufferToJpgFile24(LPSTR lpFileName, PVOID pBitmap, int nPitch, 
										   int nWidth, int nHeight, unsigned int nQuality)
{
	if(!bGdiplusInitOk)
		return false;

	int i, j;
	int nLineAdd;
	LPBYTE lpDes, lpSrc;
	WCHAR szWFileName[512];
	// byte per line % 4 must = 0
	int nBytesPerLine = nWidth * 3;
	if ((nBytesPerLine % 4) != 0)
		nBytesPerLine = nBytesPerLine + 4 - (nBytesPerLine % 4);
	DWORD OffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	LPBYTE pBuffer = new BYTE[nHeight * nBytesPerLine];
	if(!pBuffer)
		return false;

	CLSID encoderClsid;
	EncoderParameters encoderParameters;
	ULARGE_INTEGER fileSize;
	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;
	Image *pImage = NULL;
	LPSTREAM pStream;
	if(CreateStreamOnHGlobal(NULL, true, &pStream) != S_OK)
	{
		pStream = NULL;
		goto error;
	}

	fileSize.HighPart = 0;
	fileSize.LowPart = sizeof(FileHeader) + sizeof(InfoHeader) + nBytesPerLine * nHeight;
	pStream->SetSize(fileSize);

	// file header
	FileHeader.bfType          = 0x4d42; // "BM"
	FileHeader.bfSize          = nBytesPerLine * nHeight + OffBits;
	FileHeader.bfReserved1     = 0;
	FileHeader.bfReserved2     = 0;
	FileHeader.bfOffBits       = OffBits;

	// info header
	InfoHeader.biSize          = sizeof(BITMAPINFOHEADER);
	InfoHeader.biWidth         = nWidth;
	InfoHeader.biHeight        = nHeight;
	InfoHeader.biPlanes        = 1;
	InfoHeader.biBitCount      = 24;
	InfoHeader.biCompression   = 0;
	InfoHeader.biSizeImage     = 0;
	InfoHeader.biXPelsPerMeter = 0xb40;
	InfoHeader.biYPelsPerMeter = 0xb40;
	InfoHeader.biClrUsed       = 0;
	InfoHeader.biClrImportant  = 0;

	// write file head
	pStream->Write(&FileHeader, sizeof(FileHeader), NULL);

	// write info head
	pStream->Write(&InfoHeader, sizeof(InfoHeader), NULL);

	// encode bitmap
	lpDes = pBuffer;
	lpSrc = (LPBYTE)pBitmap;

	nLineAdd = nPitch - nWidth * 3;
	lpDes += (nHeight - 1) * nBytesPerLine;
	for (i = 0; i < nHeight; i++)
	{
		for (j = 0; j < nWidth; j++)
		{
			lpDes[0] = lpSrc[0];
			lpDes[1] = lpSrc[1];
			lpDes[2] = lpSrc[2];
			lpDes += 3;
			lpSrc += 3;
		}
		lpDes -= 3 * nWidth;
		lpDes -= nBytesPerLine;
		lpSrc += nLineAdd;
	}

	// write bitmap bits data
	pStream->Write(pBuffer, nBytesPerLine * nHeight, NULL);
	pImage = new Image(pStream);

	// Get the CLSID of the JPEG encoder.
	if(GetEncoderClsid(L"image/jpeg", &encoderClsid) == -1)
		goto error;

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	// Save the image as a JPEG with quality level 0.
	encoderParameters.Parameter[0].Value = &nQuality;

	MultiByteToWideChar(CP_ACP, 0, lpFileName, strlen(lpFileName)+1, szWFileName, 510);

	if(pImage->Save(szWFileName, &encoderClsid, &encoderParameters) != Ok)
		goto error;

	SAFE_DELETE(pImage);
	SAFE_DELETE_ARRAY(pBuffer);
	if(pStream)
	{
		pStream->Release();
		pStream = NULL;
	}
	return true;

error:
	SAFE_DELETE(pImage);
	SAFE_DELETE_ARRAY(pBuffer);
	if(pStream)
	{
		pStream->Release();
		pStream = NULL;
	}
	return false;
}

