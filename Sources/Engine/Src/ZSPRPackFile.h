#ifndef ZSPRPACKFILE
#define ZSPRPACKFILE

#include <windows.h>
typedef struct {
	unsigned long index_high;
	unsigned long index_low;
	long offset; 
	long size; 
} item_info;

#include "KSprite.h"

#define MINIMIZE_BLOCK_SIZE	16					//一个块最少16个字节
#define MAX_LAST			16					//记录最近使用的项

class ZCache {
	char *buffer;								//实际的缓冲区
	item_info *free_items;						//空闲块
	long cache_size;
	unsigned long last_items[MAX_LAST];
	int last;
	CRITICAL_SECTION mutex;
public:
	ZCache(long size);
	virtual ~ZCache();
	char *getNode(unsigned long index_high, unsigned long index_low, long size);
	void completeNode(char *node);
	char *searchNode(unsigned long index_high, unsigned long index_low = 0xFFFF);
	void releaseNode(char *node);
};

class ZFile {
protected:
	HANDLE m_hFile;
	unsigned long m_Size;
	ZCache *m_Cache;
public:
	bool opened;
	ZFile(const char *name, ZCache *cache) {
		opened = false;
		m_hFile = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if(m_hFile != INVALID_HANDLE_VALUE) {
			m_Size = GetFileSize(m_hFile, NULL);
			m_Cache = cache;
			opened = true;
		}
	}
	virtual ~ZFile() {
		if(m_hFile != INVALID_HANDLE_VALUE) CloseHandle(m_hFile);
	}
	char *read(unsigned long offset, int size);
	int read(char *buffer, unsigned long offset, int size);
	void release(char *node) {
		m_Cache->releaseNode(node);
	}
};

//一个Pack文件具有下面的结构:
//首先是四个字节的文件的头标志:字符串'PACK',然后是项的数目然后是索引开始的偏移量\数据开始的偏移量,然后是校验和,然后是保留的字节:
//---------------------------------------------------------------------------------------------------------------------------------
typedef struct {		//索引信息
	unsigned long id;
	unsigned long offset;
	long size;
	long compress_size;
} index_info;

#define TYPE_NONE			0					//没有压缩
#define TYPE_UCL			1					//UCL压缩
#define TYPE_BZIP2			2					//bzip2压缩
#define TYPE_FRAME			0x10				//使用了独立帧压缩

typedef struct {
	unsigned char signature[4];			//"PACK"
	unsigned long count;				//数据的条目数
	unsigned long index_offset;			//索引的偏移量
	unsigned long data_offset;			//数据的偏移量
	unsigned long crc32;
	unsigned char reserved[12];
} z_pack_header;

class ZPackFile : public ZFile {
protected:
	index_info	*index_list;
	z_pack_header header;
	bool _readData(int node_index, char *node);								//将指定node_index的数据全部读到指定内存中，如果需要的话完成解压缩
public:
	ZPackFile(const char *name, ZCache *cache);					//打开文件
	virtual ~ZPackFile();
	int getNodeIndex(unsigned long id);
	char *getData(unsigned long index);
	unsigned long getSize(unsigned long index);
	char *getData(const char *name);										//获取指定节点的数据
	void releaseData(char *data);
};

typedef struct {
	long compress_size;
	long size;
} frame_info;

typedef struct {
	unsigned long	id;
	int				frame;
} frame_index_info;

#define MAX_IMAGE		0x10													//最多临时文件的个数

class ZSPRPackFile : public ZPackFile
{
public:
	ZSPRPackFile(const char *file_name, ZCache *the_cache);
	virtual ~ZSPRPackFile();

	SPRHEAD*	SprGetHeader(unsigned long uNameId, SPROFFS*& pOffsetTable);
	SPRFRAME*	SprGetFrame(SPRHEAD* pSprHeader, int nFrame);
};

unsigned long FileNameHash(const char* file_name);

#define	SPR_COMMENT_FLAG	0x525053

#define NODE_WAITING	0xFFFF0000ul;

#endif