#ifndef ZPACKFILE_H
#define ZPACKFILE_H

//#include <windows.h>
//#include "ucl/ucl.h"
#include "KWin32.h"
#include "KMutex.h"
#define MINIMIZE_MAP_SIZE		65536 * 1024			//最小映射的大小为64M

class ZMapFile {
public:
	HANDLE m_hFile;
	HANDLE m_hMap;
	char *m_Ptr;						//可以直接访问的缓冲区指针
	unsigned long m_Size;				//文件的大小
	unsigned long old_offset;
	ZMapFile(const char *file_name);
	char *map(unsigned long offset = 0, unsigned long size = 0);
	void unmap();
	~ZMapFile();
};

typedef struct {
	unsigned long index_high;
	unsigned long index_low;
	long offset; 
	long size; 
} item_info;

#define MINIMIZE_BLOCK_SIZE	16					//一个块最少16个字节
#define LAST_ITEM			10

class ZCache {
	char *buffer;								//实际的缓冲区
	item_info *free_items;						//
	long cache_size;
	KMutex m_Mutex;
	item_info last_item[LAST_ITEM];
	int last;
public:
	ZCache(long size);
	virtual ~ZCache();
	char *getNode(unsigned long index_high, unsigned long index_low, long size);
	char *searchNode(unsigned long index_high, unsigned long index_low = 0xfffffffful);
};

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

unsigned long hash(const char *file_name);									//计算指定字符串的hash值

//一个Pack文件具有下面的结构:
//首先是四个字节的文件的头标志:字符串'PACK',然后是项的数目然后是索引开始的偏移量\数据开始的偏移量,然后是校验和,然后是保留的字节:
typedef struct {
	unsigned char signature[4];			//"PACK"
	unsigned long count;				//数据的条目数
	unsigned long index_offset;			//索引的偏移量
	unsigned long data_offset;			//数据的偏移量
	unsigned long crc32;
	unsigned char reserved[12];
} z_pack_header;

class ZPackFile {
protected:
	ZCache		*cache;
	ZMapFile	*data_map;
	index_info	*index_list;
	z_pack_header header;
	bool _readData(int node_index, char *node);								//将指定node_index的数据全部读到指定内存中，如果需要的话完成解压缩
public:
	bool opened;
	ZPackFile(const char *file_name, ZCache *the_cache);							//打开文件
	virtual ~ZPackFile();
	int getNodeIndex(unsigned long id);
	char *getData(unsigned long index);
	unsigned long getSize(unsigned long index);
	char *getData(const char *name);												//获取指定节点的数据
};

#endif