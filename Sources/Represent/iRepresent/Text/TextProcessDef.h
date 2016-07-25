#pragma once

typedef	int (*fnTextProcessEncodePlugin)();
typedef	int (*fnTextProcessDrawPlugin)();

struct KOutputTextParam
{
	int	nX;
	int	nY;
	int	nZ;
	short 	nSkipLine;
	short 	nNumLine;
	unsigned int Color;
	unsigned int BorderColor;
	unsigned short nVertAlign;	//0:居中/1:上对齐/2:下对齐
	int bPicPackInSingleLine;

	//加一个构造函数,主要是nVertAlign的初始化[wxb 2003-7-10]
	KOutputTextParam()
	{
		nX = nY = nZ = 0;
		nSkipLine = nNumLine = 0;
		Color = 0;
		nVertAlign = 0;	//缺省为居中
		bPicPackInSingleLine = false;
	}
};