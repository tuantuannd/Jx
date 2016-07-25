//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMemManager.h
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Header File
//---------------------------------------------------------------------------
#ifndef KMemManager_H
#define KMemManager_H
//---------------------------------------------------------------------------
#include "KList.h"
#include "KNode.h"
//---------------------------------------------------------------------------
class ENGINE_API KMemManager
{
private:
	struct KChunkHeader
	{
		KNode node;// node for link
		long block_num;// Block number
		long block_size;// Block size
	};
	struct KBlockHeader
	{
		void* next;// next block
		long size;// block size
		long magic;// header magic code
	};
	struct KBlockTailer
	{
		long magic;// tail magic code
	};
    enum
	{
        MIN_BLOCK = 4,  // min block size is 2^4
		MAX_BLOCK = 10, // max block size is 2^10
		NUM_BLOCK = ((MAX_BLOCK-MIN_BLOCK)+1),
		HEAD_MAGIC = 0xDEADBEEF, // header magic code
		TAIL_MAGIC = 0xFEEBDAED, // tailer magic code
		CHUNK_SIZE = 16 * 1024, // chunk size is 16kb
    };
    KList m_chunks;
    void* m_blocks[NUM_BLOCK];
    long m_block_size[NUM_BLOCK];
    void* NewChunk(int block_size, int block_num);
    void FreeChunk(KChunkHeader *ch);
	void InitBlock();
	void CheckLeak();
public:
    KMemManager();
    ~KMemManager();
    void* Malloc(int size);// 分配的内存（不用初始化）
    void* Calloc(int size);// 分配的内存（用零初始化）
    void Free(void* p);
};
extern ENGINE_API KMemManager g_MemManager;
//---------------------------------------------------------------------------
#endif
