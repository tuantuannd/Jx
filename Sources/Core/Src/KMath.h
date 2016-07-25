#ifndef KMathH
#define	KMathH

#include <math.h>
#include "GameDataDef.h"

int g_InitMath();
int g_UnInitMath();

void g_InitSeries();

#ifdef __linux
#define __cdecl
#endif

//---------------------------------------------------------------------------
// 正弦表 (将浮点数 *1024 整型化)
extern int		*g_nSin;

// 余弦表 (将浮点数 *1024 整型化)
extern int		*g_nCos;

// 正弦余弦的查表函数代码缓冲区
extern unsigned char *g_InternalDirSinCosCode;

typedef int	__cdecl g_InternalDirSinCosFunction(int pSinCosTable[], int nDir, int nMaxDir);

inline int g_DirSin(int nDir, int nMaxDir)
{
    return (*(g_InternalDirSinCosFunction *)(&(g_InternalDirSinCosCode[0])))(g_nSin, nDir, nMaxDir);
}


inline int g_DirCos(int nDir, int nMaxDir)
{
    return (*(g_InternalDirSinCosFunction *)(&(g_InternalDirSinCosCode[0])))(g_nCos, nDir, nMaxDir);
}

//---------------------------------------------------------------------------
// 五行相生相克
extern int		g_nAccrueSeries[series_num];
extern int		g_nConquerSeries[series_num];

// 五行相生相克函数代码缓冲区
extern unsigned char *g_InternalIsAccrueConquerCode;

typedef int __cdecl g_InternalIsAccrueConquerFunction(int pAccrueConquerTable[], int nSrcSeries, int nDesSeries);

inline int g_IsAccrue(int nSrcSeries, int nDesSeries)
{
    return (*(g_InternalIsAccrueConquerFunction *)(&(g_InternalIsAccrueConquerCode[0])))(g_nAccrueSeries, nSrcSeries, nDesSeries);
}

inline int g_IsConquer(int nSrcSeries, int nDesSeries)
{
    return (*(g_InternalIsAccrueConquerFunction *)(&(g_InternalIsAccrueConquerCode[0])))(g_nConquerSeries, nSrcSeries, nDesSeries);
}


//---------------------------------------------------------------------------
inline int	g_GetDistance(int nX1, int nY1, int nX2, int nY2)
{
	//Fixed By MrChuCong@gmail.com
	return (int)sqrt(double(nX1 - nX2) * (nX1 - nX2) + (nY1 - nY2) * (nY1 - nY2));
}


inline int	g_GetDirIndex(int nX1, int nY1, int nX2, int nY2)
{
	int		nRet = -1;

	if (nX1 == nX2 && nY1 == nY2)
		return -1;

//	int		nDistance = g_GetDistance(nX1, nY1 * 2, nX2, nY2 * 2);
	int		nDistance = g_GetDistance(nX1, nY1, nX2, nY2);
	
	if (nDistance == 0 ) return -1;
	
//	int		nYLength = (nY2 - nY1) * 2;
	int		nYLength = nY2 - nY1;
	int		nSin = (nYLength << 10) / nDistance;	// 放大1024倍
	

	for (int i = 0; i < 32; i++)		// 顺时针方向 从270度到90度，sin值递减
	{
		if (nSin > g_nSin[i])
			break;
		nRet = i;
	}

	if ((nX2 - nX1) > 0)
	{
		nRet = 63 - nRet;
	}
	return nRet;
}

inline	int g_Dir2DirIndex(int nDir, int nMaxDir)
{
	int nRet = -1;

	if (nMaxDir <= 0)
		return nRet;

	nRet = (nDir << 6) / nMaxDir;
	return nRet;
}

inline int	g_DirIndex2Dir(int nDir, int nMaxDir)
{
	int		nRet = -1;

	if (nMaxDir <= 0)
		return nRet;

	nRet = (nMaxDir * nDir) >> 6;	// (nMaxDir / 64) * nDir
	return nRet;
}


inline BOOL g_RandPercent(int nPercent)
{
	return ((int)g_Random(100) < nPercent);

/*	int i = g_Random(100);

	if (i >= nPercent)
		return FALSE;
	else
		return TRUE;*/
}

#endif //KMathH
