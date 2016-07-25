//*************************************************************************************
// 表现模块的公用数据类型和函数
//*************************************************************************************

#ifndef __REPRESENTUTILITY_H__
#define __REPRESENTUTILITY_H__

#include <windows.h>

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

// 初始化Gdi+
bool InitGdiplus();
// 关闭Gdi+
void ShutdownGdiplus();

// 将argb缓冲区保存为jpg文件
bool SaveBufferToJpgFile32(LPSTR lpFileName, PVOID pBitmap, int nPitch, 
										   int nWidth, int nHeight, unsigned int nQuality);

// 将rgb缓冲区保存为jpg文件
bool SaveBufferToJpgFile24(LPSTR lpFileName, PVOID pBitmap, int nPitch, 
										   int nWidth, int nHeight, unsigned int nQuality);

#endif