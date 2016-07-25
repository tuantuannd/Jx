/*****************************************************************************************
//  图形到内存区域的操作
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-11
------------------------------------------------------------------------------------------
*****************************************************************************************/

#pragma once

void RIO_Set16BitImageFormat(int b565);

void RIO_CopySprToBuffer(void* pSpr, int nSprWidth, int nSprHeight, void* pPalette,
					 void* pBuffer, int nBufferWidth, int nBufferHeight, int nX, int nY);

void RIO_CopySprToBufferAlpha(void* pSpr, int nSprWidth, int nSprHeight, void* pPalette,
					 void* pBuffer, int nBufferWidth, int nBufferHeight, int nX, int nY);

void RIO_CopySprToBuffer3LevelAlpha(void* pSpr, int nSprWidth, int nSprHeight, void* pPalette,
					 void* pBuffer, int nBufferWidth, int nBufferHeight, int nX, int nY);

void RIO_CopyBitmap16ToBuffer(void* pBitmap, int nBmpWidth, int nBmpHeight,
					 void* pBuffer, int nBufferWidth, int nBufferHeight, int nX, int nY);
