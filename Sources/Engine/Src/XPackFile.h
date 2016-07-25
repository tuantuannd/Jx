/*****************************************************************************************
//	包文件读取
//	Copyright : Kingsoft 2003
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2003-9-16
------------------------------------------------------------------------------------------
	XPackFile支持多线程的同时访问。
	XPackFile外部通过XPackElemFileRef来描述其获得的对包内子文件的引用。
	外部勿自行随意改动此描述结构内的内容，以获得正常的包访问功能。
*****************************************************************************************/
#ifndef _XPACK_FILE_H_
#define	_XPACK_FILE_H_
#include "KSprite.h"

//--------------------------------------------------
//使用XPackFile对子文件进行访问操作所用到的辅助结构
//--------------------------------------------------
struct	XPackElemFileRef
{
	unsigned long	uId;			//文件id
	int				nPackIndex;		//包索引
	int				nElemIndex;		//包内的子文件索引
	int				nCacheIndex;	//缓冲索引
	int				nOffset;		//子文件的操作便移动
	int				nSize;			//子文件的大小
};

class XPackFile
{
public:
	XPackFile();
	~XPackFile();
	//打开包文件
	bool		Open(const char* pszPackFileName, int nSelfIndex);
	//关闭包文件
	void		Close();
	//查找包内的子文件
	bool		FindElemFile(unsigned long uId, XPackElemFileRef& ElemRef);
	//读取包内的子文件
	int			ElemFileRead(XPackElemFileRef& ElemRef, void* pBuffer, unsigned uSize);

	//读取spr文件头部或整个spr
	SPRHEAD*	GetSprHeader(XPackElemFileRef& ElemRef, SPROFFS*& pOffsetTable);
	//读取按帧压缩的spr的一帧的数据
	SPRFRAME*	GetSprFrame(SPRHEAD* pSprHeader, int nFrame);

private:
	//直接读取包文件数据中的数据到缓冲区
	bool		DirectRead(void* pBuffer, unsigned int uOffset, unsigned int uSize) const;
	//带解压地读取包文件到缓冲区
	bool		ExtractRead(void* pBuffer, unsigned int uExtractSize,
						long lCompressType, unsigned int uOffset, unsigned int uSize) const;
	//在索引表中查找子文件项
	int			FindElemFile(unsigned long ulId) const;
	//在cache里查找子文件
	int			FindElemFileInCache(unsigned int uId, int nDesireIndex);
	//把子文件数据添加到cache
	int			AddElemFileToCache(void* pBuffer, int nElemIndex);
	//分配一个缓冲区，并把指定的子文件数据读入其中
	void*		ReadElemFile(int nElemIndex) const;
	//释放一个cache结点的数据
	static void	FreeElemCache(int nCacheIndex);

private:
	HANDLE					m_hFile;			//包文件句柄
	unsigned int			m_uFileSize;		//包文件大小
	int						m_nElemFileCount;	//子文件的个数
	int						m_nSelfIndex;		//包文件自己在包序列中的索引
	struct XPackIndexInfo*	m_pIndexList;		//子文件索引列表
	CRITICAL_SECTION		m_ReadCritical;		//操作包文件时的临界区控制

	//----子文件数据cache----
	struct XPackElemFileCache
	{
		void*			pBuffer;			//保存子文件数据的缓冲区
		unsigned long	uId;				//子文件id
		long			lSize;				//子文件大小
		int				nPackIndex;			//来自哪个包文件
		int				nElemIndex;			//子文件在索引列表中的位置
		unsigned int	uRefFlag;			//近期引用标记
	};

	#define	MAX_XPACKFILE_CACHE			10
	//子文件的cache数据
	static	XPackElemFileCache	ms_ElemFileCache[MAX_XPACKFILE_CACHE];
	//子文件被cache的数目
	static	int					ms_nNumElemFileCache;
};

#endif