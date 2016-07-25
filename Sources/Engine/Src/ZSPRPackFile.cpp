#include "KWin32.h"
#include "KDDraw.h"
#include "ZSPRPackFile.h"

unsigned long FileNameHash(const char *file_name)
{
	unsigned long id = 0;
	const char *ptr = file_name;
	int index = 0;
	// flying  comment here
	if (!file_name || strlen(file_name)==0)
		return false;
	
	while(*ptr) {
		if(*ptr >= 'A' && *ptr <= 'Z') id = (id + (++index) * (*ptr + 'a' - 'A')) % 0x8000000b * 0xffffffef;
		else id = (id + (++index) * (*ptr)) % 0x8000000b * 0xffffffef;
		ptr++;
	}
	return (id ^ 0x12345678);
}

//使用index_low的高位字作为引用计数
ZCache::ZCache(long size) {
	long align = size % 0x10000;				//必须是64k对齐的，不然会降低效率
	if(align) size += 0x10000 - align;
	buffer = new char[size];
	cache_size = size;
	free_items = (item_info *)buffer;
	free_items->offset = 0;
	free_items->size = cache_size;
	free_items->index_high = 0;
	memset((char *)&last_items[0], 0, MAX_LAST * sizeof(unsigned long));
	last = 0;
	InitializeCriticalSection(&mutex);
}

ZCache::~ZCache() {
	DeleteCriticalSection(&mutex);
	if(buffer)
    {
         delete[] buffer;
         buffer = NULL;
    }
}

void ZCache::completeNode(char *node) {																	//将节点状态设置为准备好
	if(!node) return;
	EnterCriticalSection(&mutex);
	item_info *current = (item_info *)(node - sizeof(item_info));
	current->index_low = current->index_low & 0x0000FFFF;									//引用计数为1 
	last_items[last] = (char *)current - buffer;
	if(++last == MAX_LAST) last = 0;
	LeaveCriticalSection(&mutex);
}

char *ZCache::getNode(unsigned long index_high, unsigned long index_low, long size) {			//得到一个缓冲区数据块
	size += sizeof(item_info);
	unsigned long align = size % MINIMIZE_BLOCK_SIZE;
	if(align) size += MINIMIZE_BLOCK_SIZE - align;

	if(size > cache_size) return 0;
	item_info *next;
	EnterCriticalSection(&mutex);
	while(free_items->size < size) {										//删除后面的项目直到有足够的空间为止
		next = (item_info *)((char *)free_items + free_items->size);		//下一个节点
		if((char *)next - buffer >= cache_size) {							//已经到了结束的位置，回第一个
			free_items = (item_info *)buffer;
			free_items->index_high = 0;
		}
		else {
/*			while(next->index_high && (next->index_low & 0xFFFF0000ul)) {
				next = (item_info *)((char *)next + next->size);
				if((char *)next - buffer >= cache_size) {
					LeaveCriticalSection(&mutex);
					return 0;														//有数据并且引用计数不为0(或者为等待状态)
				}
			}*/
/*			if(next->index_high && (next->index_low & 0xFFFF0000ul)) {			//还有数据并且正在使用
				LeaveCriticalSection(&mutex);
				return 0;														//有数据并且引用计数不为0(或者为等待状态)
			}
			else*/
			free_items->size += next->size;
		}
	}

	next = (item_info *)((char *)free_items + size);
	if((char *)next - buffer == cache_size) next = (item_info *)buffer;
	else if(free_items->size > size) {
		next->index_high = 0;
		next->size = free_items->size - size;
		next->offset = free_items->offset + size;
	}

	free_items->size = size;
	free_items->index_high = index_high;
	free_items->index_low = index_low | NODE_WAITING;
//	free_items->index_low = index_low;
	char *result = buffer + free_items->offset + sizeof(item_info);
	free_items = next;
	LeaveCriticalSection(&mutex);
	return result;
}

char *ZCache::searchNode(unsigned long index_high, unsigned long index_low) {
	EnterCriticalSection(&mutex);
	item_info *current;
	for(int index = 0; index < MAX_LAST; index++) {
		if(last_items[index]) {
			current = (item_info *)(buffer + last_items[index]);
//			unsigned short ref_count = current->index_low >> 16;
			if(current->index_high == index_high && current->index_low == index_low) {
//			if(current->index_high == index_high && ref_count != 0xFFFF && (current->index_low & 0x0000FFFF) == index_low) {
//				ref_count++;
//				if(ref_count >= 2) {
//					ref_count = 2;
//				}
//				current->index_low = (current->index_low & 0x0000FFFF) | (ref_count << 16);
				LeaveCriticalSection(&mutex);
				return buffer + current->offset + sizeof(item_info);
			}
		}
	}
	current = (item_info *)buffer;
	while((char *)current - buffer < cache_size) {
//		unsigned short ref_count = current->index_low >> 16;
		if(current->index_high == index_high && current->index_low == index_low) {
//		if(current->index_high == index_high && ref_count != 0xFFFF && (current->index_low & 0x0000FFFF) == index_low) {
//			ref_count++;
/*			if(ref_count >= 2) {
				ref_count = 2;
			}*/
//			current->index_low = (current->index_low & 0x0000FFFF) | (ref_count << 16);
			LeaveCriticalSection(&mutex);
			return buffer + current->offset + sizeof(item_info);
		}
		current = (item_info *)((char *)current + current->size);
	}
	LeaveCriticalSection(&mutex);
	return 0;
}

void ZCache::releaseNode(char *node) {
/*
	EnterCriticalSection(&mutex);
	item_info *current = (item_info *)(node - sizeof(item_info));
	short ref_count = current->index_low >> 16;
	ref_count--;
	if(ref_count < 0) {
		ref_count--;
	}
	current->index_low = (current->index_low & 0x0000FFFF) | (ref_count << 16);
	LeaveCriticalSection(&mutex);
*/
}

int ZFile::read(char *buffer, unsigned long offset, int size) {
/*	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.Offset = offset;*/
	DWORD read_size;
	SetFilePointer(m_hFile, offset, 0, SEEK_SET);
	if(ReadFile(m_hFile, buffer, size, &read_size, NULL)) return read_size;
	return read_size;
}

//注意，使用这个函数的地方需要调用release方法释放得到的缓冲区
char *ZFile::read(unsigned long offset, int size) {
	char *node = m_Cache->getNode(0, 0, size);
	if(!node) return 0;
	if(read(node, offset, size)) {
		m_Cache->completeNode(node);
		return node;
	}
	return 0;
}

unsigned long ZPackFile::getSize(unsigned long index) {
	return index_list[index].size;
}

#include "ucl/ucl.h"
bool ZPackFile::_readData(int node_index, char *node) {
	char *source = read(index_list[node_index].offset, (index_list[node_index].compress_size & 0x00FFFFFF));
	if(!source) 
        return false;
	
    unsigned int dest_length;
	unsigned long compress_type = index_list[node_index].compress_size >> 24;
	int r = 1;
	if((compress_type == TYPE_UCL) | (compress_type == TYPE_BZIP2)) {									//整体压缩
		if(compress_type == TYPE_UCL) {
			r = ucl_nrv2b_decompress_8((BYTE *)source, index_list[node_index].compress_size & 0x00FFFFFF, (BYTE *)node, &dest_length, NULL);
		}
		else if(compress_type == TYPE_BZIP2) {
		}
	}
	release(source);
	
    if(!r) return true;
	return false;
}

ZPackFile::ZPackFile(const char *name, ZCache *cache) : ZFile(name, cache){
	index_list = 0;
	if(!read((char *)&header, 0, sizeof(header))) return;
	index_list = new index_info[header.count];
	read((char *)&index_list[0], header.index_offset, header.count * sizeof(index_info));
}

ZPackFile::~ZPackFile() {
	if(index_list)
    {
         delete[] index_list;
         index_list = NULL;
    }
}

int ZPackFile::getNodeIndex(unsigned long id)
{								//二分法找到指定的索引
	int nBegin, nEnd, nMid;
	nBegin = 0;
	nEnd = header.count;
	while (nBegin <= nEnd) {
		nMid = (nBegin + nEnd) / 2;
		if (id == index_list[nMid].id) break;
		if (id < index_list[nMid].id) nEnd = nMid - 1;
		else nBegin = nMid + 1;
	}
	if(id != index_list[nMid].id) return -1;								//数据文件里面也没有
	return nMid;
}


char *ZPackFile::getData(unsigned long id) {
	char * node = m_Cache->searchNode(id);
	if(node) return node;
	int node_index = getNodeIndex(id);
	if(node_index == -1) return 0;
	node = m_Cache->getNode(id, 0xFFFF, index_list[node_index].size);
	if(!node) return 0;
	if(_readData(node_index, node)) {
		m_Cache->completeNode(node);
		return node;
	}
	else return 0;
}

char *ZPackFile::getData(const char *name) {											//获取指定的文件数据
	unsigned long id = FileNameHash(name);
	return getData(id);
}

void ZPackFile::releaseData(char *data) {
	m_Cache->releaseNode(data);
}

ZSPRPackFile::ZSPRPackFile(const char *file_name, ZCache *the_cache) : ZPackFile(file_name, the_cache) {
}

ZSPRPackFile::~ZSPRPackFile() {
}

#define	NODE_INDEX_STORE_IN_RESERVED	2

SPRHEAD* ZSPRPackFile::SprGetHeader(unsigned long uNameId, SPROFFS*& pOffsetTable)
{
	SPRHEAD*		pSpr = NULL;
	unsigned int	uTempValue;
	bool			bOk = false;
	int				nNodeIndex = getNodeIndex(uNameId);

	pOffsetTable = NULL;
	if(nNodeIndex >= 0)
	{
		//首先检查这个id是什么类型压缩方式
		if ((index_list[nNodeIndex].compress_size & (TYPE_FRAME << 24)) == 0)
		{
			pSpr = (SPRHEAD*)malloc(index_list[nNodeIndex].size);
			if (pSpr)
			{
				if (_readData(nNodeIndex, (char*)pSpr))
				{
					if ((*(int*)&pSpr->Comment[0]) == SPR_COMMENT_FLAG)
					{
						pOffsetTable = (SPROFFS*)(((char*)pSpr) + sizeof(SPRHEAD) + pSpr->Colors * 3);
						bOk = true;
					}
				}
			}
		}
		else
		{
			SPRHEAD Header;
			if (read((char *)&Header, index_list[nNodeIndex].offset, sizeof(SPRHEAD)) == sizeof(SPRHEAD))
			{
				if (*(int*)&(Header.Comment[0]) == SPR_COMMENT_FLAG)
				{
					uTempValue = Header.Colors * 3 + Header.Frames * sizeof(frame_info);
					pSpr = (SPRHEAD*)malloc(sizeof(SPRHEAD) + uTempValue);
					if (pSpr)
					{
						if (read((char*)(&pSpr[1]), index_list[nNodeIndex].offset + sizeof(SPRHEAD),
							uTempValue) == (int)uTempValue)
						{
							memcpy(pSpr, &Header, sizeof(SPRHEAD));
							bOk = true;
						}
					}
				}
			}
		}

		if (pSpr)
		{
			if (bOk)
			{
				*((int*)&pSpr->Reserved[NODE_INDEX_STORE_IN_RESERVED]) = nNodeIndex;
			}
			else
			{
				free (pSpr);
				pSpr = NULL;
			}
		}
	}
    return pSpr;
}

SPRFRAME* ZSPRPackFile::SprGetFrame(SPRHEAD* pSprHeader, int nFrame)
{
	SPRFRAME*	pFrame = NULL;
	if (pSprHeader && nFrame >= 0 && nFrame < pSprHeader->Frames)
	{
		int nNodeIndex = *((int*)&pSprHeader->Reserved[NODE_INDEX_STORE_IN_RESERVED]);
		if (nNodeIndex >= 0 && nNodeIndex < (int)header.count)
		{
			unsigned long uCompressType = index_list[nNodeIndex].compress_size >> 24;
			if ((uCompressType & TYPE_FRAME) != 0)
			{
				bool bOk = false;
				uCompressType &= 0x0F;
				long	lTempValue = sizeof(SPRHEAD) + pSprHeader->Colors * 3;
				//读出指定帧的信息
				frame_info* pFrameList = (frame_info *)((char*)pSprHeader + lTempValue);
				unsigned long	uSrcOffset = index_list[nNodeIndex].offset + lTempValue +pSprHeader->Frames * sizeof(frame_info);
			    while(nFrame > 0)
				{
					uSrcOffset += (pFrameList++)->compress_size;
					nFrame--;
				};
				lTempValue = pFrameList->size;
				if (lTempValue < 0)								//frame_info::size含义规范取反，更....
				{
					lTempValue = -lTempValue;
					if (pFrame = (SPRFRAME*)malloc(lTempValue))
					{
						if (read((char*)pFrame, uSrcOffset, lTempValue) == lTempValue)
							bOk = true;
					}
				}
				else
				{
					unsigned int	uSize = lTempValue;
					lTempValue = pFrameList->compress_size;
					char* pcBuffer = (char*)malloc(lTempValue);
					if (pcBuffer)
					{
						pFrame = (SPRFRAME*)malloc(uSize);
						if (pFrame)
						{							
							if (read(pcBuffer, uSrcOffset, lTempValue) == lTempValue)
							{
								if (uCompressType == TYPE_UCL)
								{
									ucl_nrv2b_decompress_8((unsigned char*)pcBuffer, lTempValue,
										(unsigned char*)pFrame, &uSize, NULL);
									bOk = true;
								}
							}
						}
						free (pcBuffer);
						pcBuffer = NULL;
					}
				}
				if (bOk == false && pFrame != NULL)
				{
					free(pFrame);
					pFrame = NULL;
				}
			}
		}		
	}
	return pFrame;	
}
