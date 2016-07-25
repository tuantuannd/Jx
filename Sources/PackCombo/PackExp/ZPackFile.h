#ifndef ZPACKFILE_H
#define ZPACKFILE_H

#include <windows.h>
#include "ucl/ucl.h"
//#include "KWin32.h"
//#include "KMutex.h"
#define MINIMIZE_MAP_SIZE		65536 * 1024			//最小映射的大小为64M

typedef struct {
	long compress_size;
	long size;
} frame_info;

typedef struct {
	unsigned long	id;
	int				frame;
} frame_index_info;

#define MAX_IMAGE		0x10													//最多临时文件的个数


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

//---------------------------------------------------------------------------------------------------------------------------------

typedef struct {		//索引信息
	unsigned long id;
	unsigned long offset;
	long size;
	long compress_size;
	FILETIME UpdateTime;
} index_info_nt;

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

enum enumSubDir{sdFont=0,sdMaps,sdScript,sdResource,sdSetting,sdSound,sdSpr,sdUi,sdLastDir};


//一个Pack文件具有下面的结构:
//首先是四个字节的文件的头标志:字符串'PACK',然后是项的数目然后是索引开始的偏移量\数据开始的偏移量,然后是校验和,然后是保留的字节:

typedef struct {
	typedef struct {
		unsigned long count;				//数据的条目数
		unsigned long index_count_offset;	//索引数目的偏移量
		unsigned long index_offset;			//索引的偏移量
		unsigned long data_offset;			//数据的偏移量
	} z_sub_header;

	unsigned char signature[4];			//"PACK"
	long count;
	z_sub_header SubHeader[sdLastDir-1];
	unsigned long crc32;
	unsigned char reserved[12];
} z_pack_header_nt;

typedef struct {
	unsigned char signature[4];			//"PACK"
	unsigned long count;				//数据的条目数
	unsigned long index_offset;			//索引的偏移量
	unsigned long data_offset;			//数据的偏移量
	unsigned long crc32;
	unsigned char reserved[12];
} z_pack_header;

class ZCache {
	char *buffer;								//实际的缓冲区
	item_info *free_items;						//
	long cache_size;
	item_info last_item[LAST_ITEM];
	int last;
public:
	ZCache(long size);
	virtual ~ZCache();
	char *getNode(unsigned long index_high, unsigned long index_low, long size);
	char *searchNode(unsigned long index_high, unsigned long index_low = 0xfffffffful);
};

class ZPackFile {
protected:
	ZCache		*cache;							//缓存
	ZMapFile	*data_map;						//文件缓存
	index_info	*index_list;					//索引指针
	z_pack_header header;						//包文件头部
	bool _readData(int node_index, char *node);								//将指定node_index的数据全部读到指定内存中，如果需要的话完成解压缩
public:
	bool opened;
	ZPackFile(const char *file_name, ZCache *the_cache);							//打开文件
	virtual ~ZPackFile();
	int getNodeIndex(unsigned long id);
	char *getData(unsigned long index);
	unsigned long getSize(unsigned long index);
	char *getData(const char *name);								//获取指定节点的数据
	index_info* IndexData(int index){return &index_list[index];};	//返回索引数据
	char* getOrigData(unsigned long id);				//取得未解压的原始数据
	int getFileCount(){return header.count;};							//取得文件数
	const z_pack_header* GetHeader() const{return &header;};			//取得文件头
};


class ZPackFileNT {
protected:
	ZCache		*cache;							//缓存
	ZMapFile	*data_map;						//文件缓存
	index_info_nt	*index_list;					//索引指针
	z_pack_header_nt header;						//包文件头部
	bool _readData(int node_index, char *node);								//将指定node_index的数据全部读到指定内存中，如果需要的话完成解压缩
public:
	bool opened;
	ZPackFileNT(const char *file_name, ZCache *the_cache);							//打开文件
	virtual ~ZPackFileNT();
	int getNodeIndex(unsigned long id);
	char *getData(unsigned long index);
	unsigned long getSize(unsigned long index);
	char *getData(const char *name);								//获取指定节点的数据
	index_info_nt* IndexData(int index){return &index_list[index];};	//返回索引数据
	char* getOrigData(unsigned long id);				//取得未解压的原始数据
	int getFileCount(){return header.count;};							//取得文件数
	const z_pack_header_nt* GetHeader() const{return &header;};			//取得文件头
};

#endif