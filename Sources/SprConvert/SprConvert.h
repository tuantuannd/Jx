#ifndef __SPRCONVERT_H__
#define __SPRCONVERT_H__

struct SprTexInfoHead
{
	BYTE	Comment[4];		// 注释文字(STI\0)
	WORD	nTexNum;		// 贴图数目
	WORD	nFrameNum;		// 可容纳的帧数
};

struct TexSize
{
	WORD	nWidth;
	WORD	nHeight;
};

struct FrameTexInfo
{
	WORD	nTexIndex;		// 这一帧放在第几张贴图
	WORD	nTexX;			// 在贴图上的位置的x坐标
	WORD	nTexY;			// 在贴图上的位置的y坐标
};

struct TextureSize
{
	int		nWidth;
	int		nHeight;
	int		nFrameWidth;
	int		nFrameHeight;
	int		nFrameNum;		// 可容纳的帧数
	WORD	nRealFrameNum;  // 实际存放的帧数
};

class SprConvert
{
private:
	SPRHEAD*	m_pHeader;
	PBYTE		m_pFileData;
	SPROFFS* 	m_pOffset;
	PBYTE		m_pSprite;
	bool		m_bReadOK;
	int			m_nTotleTexNum;
	TextureSize m_TexSize[200];
private:
public:
public:
	SprConvert();
	~SprConvert();
	void ReadFile(char *szSprFile);
	int DoGroup(int nWidth, int nHeight, int nMinTexWidth, int nMinTexHeight, int &nTexNum);
	bool Output();
	void WriteFile(char *szSprFile);
};

#endif