//---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// File:	KMemManager.cpp
// Date:	2000.08.08
// Code:	WangWei(Daphnis)
// Desc:	Memory Opration Class Using Heap Memory Functions
//---------------------------------------------------------------------------
#include "KWin32.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KMemManager.h"
//---------------------------------------------------------------------------
ENGINE_API KMemManager g_MemManager;
//---------------------------------------------------------------------------
// 函数:	KMemManager
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMemManager::KMemManager()
{
    int i;
    for (i = 0; i<NUM_BLOCK; i++)
        m_block_size[i] = (1<<(i + MIN_BLOCK)); //块的大小
	g_MemZero(m_blocks, sizeof(m_blocks));
}
//---------------------------------------------------------------------------
// 函数:	~KMemManager
// 功能:	析购函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KMemManager::~KMemManager()
{
    KChunkHeader* ch;
	KBlockHeader* bh;
	char* bp;
	int i;
	// 检测内存泄漏
    while (ch = (KChunkHeader *)m_chunks.GetHead())
	{
        bp = ((char *)ch) + sizeof(KChunkHeader);
        for (i = 0; i < ch->block_num; i++)
		{
            bh = (KBlockHeader *)bp;
            if (bh->size != 0)// 有没释放的内存块？
				g_DebugLog("KMemManager::Leak Detected, Size = %d", bh->size);
			bp += ch->block_size;
        }
        FreeChunk(ch);
    }
}
//---------------------------------------------------------------------------
// 函数:	NewChunk()
// 功能:	创建新的内存分配块，CHUNK_SIZE为单位
// 参数:	block size, block number
// 返回:	block header
//---------------------------------------------------------------------------
void* KMemManager::NewChunk(int block_size, int block_num)
{
	// 块的大小要加上块头和块尾
    block_size = block_size + sizeof(KBlockHeader) + sizeof(KBlockTailer);
	// chunk的大小要包括chunk header
    int chunk_size = sizeof(KChunkHeader) + (block_size * block_num);
    // 分配一大块内存
	char* c = (char*)g_MemAlloc(chunk_size);
    if (c)
	{
        // ChunkHeader initialization
        KChunkHeader* ch = (KChunkHeader *)c;
		ch->node.m_pNext = NULL;
		ch->node.m_pPrev = NULL;
        ch->block_num  = block_num;
        ch->block_size = block_size;
        m_chunks.AddTail((KNode *)ch);

        // blocks im Chunk initializtion
        char *f = c + sizeof(KChunkHeader); // first Block
        char *p = c + chunk_size; // last Block + 1
        char *next = NULL;
        while (p > f)
		{
            p -= block_size;
            KBlockHeader *bh = (KBlockHeader *)p;
            bh->next = (void *)next;
            bh->size = 0;
            bh->magic = HEAD_MAGIC;
            next = p;
        }
        return f;
    }
    return NULL;
}
//---------------------------------------------------------------------------
// 函数:	FreeChunk()
// 功能:	创建新的内存分配块，CHUNK_SIZE为单位
// 参数:	block size, block number,
// 返回:	block header
//---------------------------------------------------------------------------
void KMemManager::FreeChunk(KChunkHeader *ch)
{
	ch->node.Remove();
	g_MemFree((void *)ch);
}
//---------------------------------------------------------------------------
// 函数:	Malloc()
// 功能:	分配内存
// 参数:	size in bytes
// 返回:	void*
//---------------------------------------------------------------------------
void* KMemManager::Malloc(int size)
{
    char* p = NULL;
    
	if (size > (1<<MAX_BLOCK))
	{
        p = (char *)NewChunk(size, 1);
        if (p)
		{
            KBlockHeader *bh = (KBlockHeader *)p;
            KBlockTailer *bp = (KBlockTailer *)(p + size + sizeof(KBlockHeader));
            bh->next = (void *) -1L;// 表示只有一块
            bh->size = size;
            bh->magic = HEAD_MAGIC;
            bp->magic = TAIL_MAGIC;
            p += sizeof(KBlockHeader);
        }
    }
	else
	{
        // 找一个大小合适的chunk
        int i, mask;
        for (i=0; i<NUM_BLOCK-1; i++)
		{
            mask = ~(m_block_size[i]-1);
            if ((size & mask) == 0)
                break;
        }
        if (NULL == m_blocks[i])
		{
			m_blocks[i] = NewChunk(m_block_size[i], 
				CHUNK_SIZE / m_block_size[i]);
		}
        p = (char *)m_blocks[i];
		KBlockHeader *bh = (KBlockHeader *)p;
		KBlockTailer *bp = (KBlockTailer *)(p + size + sizeof(KBlockHeader));
		m_blocks[i] = bh->next;
		bh->next = (void *)i;
		bh->size = size;
		bh->magic = HEAD_MAGIC;
		bp->magic = TAIL_MAGIC;
		p += sizeof(KBlockHeader);
    }
    return p;
}
//---------------------------------------------------------------------------
// 函数:	Calloc()
// 功能:	分配内存，并用零填充
// 参数:	size in bytes
// 返回:	void*
//---------------------------------------------------------------------------
void* KMemManager::Calloc(int size)
{
	void* p = Malloc(size);
	g_MemZero(p, size);
	return p;
}
//---------------------------------------------------------------------------
// 函数:	Free()
// 功能:	释放内存
// 参数:	void*
// 返回:	void
//---------------------------------------------------------------------------
void KMemManager::Free(void* p)
{
	if (NULL == p)
	{
		g_DebugLog("KMemManager::Free(p); p = NULL!");
		return;
	}
    char* pc = (char *)p;
    KBlockHeader *bh = (KBlockHeader *)(pc - sizeof(KBlockHeader));
    KBlockTailer *bp = (KBlockTailer *)(pc + bh->size);

    // 检测是否存在内存越界访问
    if ((bh->magic != HEAD_MAGIC) || (bp->magic != TAIL_MAGIC))
        g_MessageBox("Memory Corrupted : Size = %d", bh->size);

	// 单独分配的内存块
    if (((int)bh->next) == -1L)
	{
        KChunkHeader *ch = (KChunkHeader *)(((char *)bh) - sizeof(KChunkHeader));
        bh->size = 0;
        FreeChunk(ch);
    }
	else
	{
        int i = (int)bh->next;
        KBlockHeader* next = (KBlockHeader *)m_blocks[i];
        m_blocks[i] = bh;
        bh->next = next;
        bh->size = 0;
    }
}
//---------------------------------------------------------------------------
