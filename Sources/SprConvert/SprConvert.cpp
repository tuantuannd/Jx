// SprConvert.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FileEnumerate.h"
#include "SprConvert.h"
using std::cout;
using std::endl;
#define MAX_TEXTURE_SIZE		1024			// 最大贴图尺寸
#define MIN_TEXTURE_SIZE		2				// 最小贴图尺寸
#define MIN_TEXTUREGROUP_SIZE	512				// 分组贴图的最大尺寸

int nTotleFileConvert=0;
int nTotleFileNotConvert=0;
float nTotleMemUseBefore = 0;
float nTotleMemUseNow = 0;
unsigned int nTotleTexNumBefore = 0;
unsigned int nTotleTexNumNow = 0;
int	MaxSize = 0;
std::ofstream ofs; //Fixed ByMrChuCong@gmail.com

int FitTextureSize(int nSize)
{
	if(nSize < 0 || nSize > MAX_TEXTURE_SIZE)
		return 0;

	for(int i=MIN_TEXTURE_SIZE; i<=MAX_TEXTURE_SIZE; i*= 2)
	{
		if(nSize <= i)
			return i;
	}
	return 0;
}

SprConvert::SprConvert()
{
	m_pHeader = NULL;
	m_pFileData = NULL;
	m_pOffset = NULL;
	m_pSprite = NULL;
	m_bReadOK = false;
	m_nTotleTexNum = 0;
}

SprConvert::~SprConvert()
{
	SAFE_DELETE_ARRAY(m_pFileData);
}

int SprConvert::DoGroup(int nWidth, int nHeight, int nMinTexWidth, int nMinTexHeight, int &nTexNum)
{
	int nTexWidth = FitTextureSize(nWidth);
	int nTexHeight = FitTextureSize(nHeight);
	if(nTexWidth == 0 || nTexHeight == 0)
		return 0;

	int nHalfWidth = nTexWidth / 2;
	int nHalfHeight = nTexHeight / 2;
	int nMem = 0;

	int nt1,nt2,nt3;

	if(nWidth > nMinTexWidth && nHeight > nMinTexHeight)
	{
		nMem = nHalfWidth * nHalfHeight * 2;
		nMem += DoGroup(nWidth - nHalfWidth, nHeight - nHalfHeight, nMinTexWidth, nMinTexHeight, nt1);
		nMem += DoGroup(nWidth - nHalfWidth, nHalfHeight, nMinTexWidth, nMinTexHeight, nt2);
		nMem += DoGroup(nHalfWidth, nHeight - nHalfHeight, nMinTexWidth, nMinTexHeight, nt3);
		nTexNum = 1 + nt1 + nt2 + nt3; 
	}
	else if(nWidth > nMinTexWidth)
	{
		nMem = nHalfWidth * nTexHeight * 2;
		nMem += DoGroup(nWidth - nHalfWidth, nHeight, nMinTexWidth, nMinTexHeight, nt1);
		nTexNum = 1 + nt1; 
	}
	else if(nHeight > nMinTexHeight)
	{
		nMem = nTexWidth * nHalfHeight * 2;
		nMem += DoGroup(nWidth, nHeight - nHalfHeight, nMinTexWidth, nMinTexHeight, nt1);
		nTexNum = 1 + nt1; 
	}
	else
	{
		nMem = nTexWidth * nTexHeight * 2;
		nTexNum = 1;
	}

	return nMem;
}

void SprConvert::ReadFile(char *szSprFile)
{
	KPakFile	File;
	PBYTE		pTemp;

	m_bReadOK = false;
//	cout<<szSprFile<<endl;

	// open the file
	if (!File.Open(szSprFile))
	{
//		cout<<"Can't open file!"<<endl;
		return;
	}

	m_pFileData = pTemp = new BYTE[File.Size()];
	if (!pTemp)
	{
//		cout<<"Not enough memory!"<<endl;
		return;
	}

	// read data from file
	File.Read(pTemp, File.Size());

	// check file header setup sprite member
	m_pHeader = (SPRHEAD*)pTemp;
	if (!g_MemComp(m_pHeader->Comment, "SPR", 3))
	{
//		cout<<"Not a spr file!"<<endl;
		SAFE_DELETE_ARRAY(m_pFileData);
		return;
	}

	// setup palette pointer
	pTemp += sizeof(SPRHEAD);


	// setup offset pointer
	pTemp += m_pHeader->Colors * sizeof(KPAL24);
	m_pOffset = (SPROFFS*)pTemp;

	// setup sprite pointer
	pTemp += m_pHeader->Frames * sizeof(SPROFFS);
	m_pSprite = (LPBYTE)pTemp; // 相对偏移

	if(m_pHeader->Frames <= 0)
	{
		SAFE_DELETE_ARRAY(m_pFileData);
		return;
	}

	int i;
	int nTexWidth, nTexHeight, nTexStopWidth, nTexStopHeight;
	int nMemLarge, nMemSmall, nSprMemLarge = 0, nSprMemSmall = 0, nTexNum = 0;
	for(i=0; i<m_pHeader->Frames; i++)
	{
		SPRFRAME* pFrame = (SPRFRAME*)(m_pSprite + m_pOffset->Offset);
		nTexWidth = FitTextureSize(pFrame->Width);
		nTexHeight = FitTextureSize(pFrame->Height);
		nTexStopWidth = nTexWidth / 8;
		nTexStopHeight = nTexHeight / 8;
		if(nTexStopWidth < 16)
			nTexStopWidth = 16;
		if(nTexStopHeight < 16)
			nTexStopHeight = 16;
		nMemLarge = nTexWidth * nTexHeight * 2;
		int nTn;
		nMemSmall = DoGroup(pFrame->Width, pFrame->Height, nTexStopWidth, nTexStopHeight, nTn);
		nTexNum += nTn;
		if(nMemSmall == 0)
			nMemLarge = 0;
		nSprMemLarge += nMemLarge;
		nSprMemSmall += nMemSmall;
		m_pOffset++;
	}

//	cout<<szSprFile<<endl;
//	cout<<nSprMemLarge<<"  "<<nSprMemSmall<<"  "<<m_pHeader->Frames<<"  "<<nTexNum<<endl;

	nTotleTexNumBefore += m_pHeader->Frames;
	nTotleTexNumNow += nTexNum;
	nTotleMemUseBefore += nSprMemLarge;
	nTotleMemUseNow += nSprMemSmall;

	m_bReadOK = true;
}

bool SprConvert::Output()
{
	if(!m_bReadOK)
		return false;

	return true;
}

void SprConvert::WriteFile(char *szSprFile)
{
	
}

void ConvertOneSpr(char *SzSprFile)
{
	SprConvert spr;
	spr.ReadFile(SzSprFile);
	spr.Output();
//	if(spr.Output())
//	{
//		spr.WriteFile(SzSprFile);
//		nTotleFileConvert++;
//	}
}

void main(int argc, char* argv[])
{
	if(argc != 3)
		return;
	MaxSize = atoi(argv[2]);
	if(MaxSize<=0)
		return;
	ofs.open("ttt.txt");
	DirectorySearch(argv[1], ConvertOneSpr);

	cout<<endl;
	cout<<endl;
	cout<<nTotleMemUseBefore<<"  "<<nTotleMemUseNow<<"  "<<nTotleTexNumBefore<<"  "<<nTotleTexNumNow<<endl;

//	cout<<endl;
//	cout<<"TotleFileConvert     : "<<nTotleFileConvert<<endl;
//	cout<<"TotleFileNotConvert  : "<<nTotleFileNotConvert<<endl;
//	cout<<endl;
//	cout<<"TotleMemUseBefore : "<<nTotleMemUseBefore<<endl;
//	cout<<"TotleMemUseNow    : "<<nTotleMemUseNow<<endl;
}

