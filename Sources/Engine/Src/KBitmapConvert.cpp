#include "kwin32.h"
#include "Windows.h"

extern "C" __declspec(dllexport)
void g_CreateAlphaFromRGB(BYTE* pBuffer, int nSize)
{
	int r, g, b, a;
	while (nSize > 0)
	{
		r = pBuffer[0];
		g = pBuffer[1];
		b = pBuffer[2];
		a = (r * 30 + g * 59 + b * 11) / 100;
		pBuffer[3] = (BYTE)a;
		pBuffer += 4;
		nSize -= 4;
	}
}