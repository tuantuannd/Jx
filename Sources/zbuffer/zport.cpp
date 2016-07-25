#include "zport.h"
#include <stdio.h>

#ifdef WIN32
DWORD WINAPI ThreadProc(LPVOID lpParameter) {
#else
void *ThreadProc(LPVOID lpParameter) {
#endif
  ZThread *thread = (ZThread *)lpParameter;
  thread->action();
  return 0;
}

void ZThread::start() {
#ifdef WIN32
  handle = CreateThread(0, 0, ThreadProc, (LPVOID)this, 0, &id);
#else
  int ret = pthread_create(&p_thread, NULL, ThreadProc, this);
  if (ret == 0)
  {
	  pthread_detach(p_thread);
  }
#endif
}

void ZThread::stop() {
  bStop = true;
#ifdef WIN32
  TerminateThread(handle, 0);
#else
  pthread_cancel(p_thread);
#endif
}

namespace OnlineGameLib {
namespace Win32 {

CBuffer::CBuffer( Allocator &allocator, size_t size )
	: m_allocator( allocator )
	, m_ref( 1 )
	, m_size( size )
	, m_used( 0 )
	, m_packLength( 0 )
	, m_offsetNextPack( 0 )
{
	//( ( BYTE * )( ( DWORD )( pMemory + 3 ) & ( ~3 ) ) )
	m_buffer_ptr = new BYTE[ size ];

	Empty();
}

CBuffer::~CBuffer()
{
	SAFE_DELETE_ARRAY( m_buffer_ptr );
}

CBuffer *CBuffer::GetHeadPack( size_t packLength )
{
	if ( 0 == m_used || 0 == packLength )
	{
		return NULL;
	}

	m_packLength = packLength;

	CBuffer *pNewBuffer = m_allocator.Allocate();

	size_t canused = ( m_packLength > m_used ) ? m_used : m_packLength;

	pNewBuffer->AddData( m_buffer_ptr, canused );

	m_offsetNextPack = canused;

	return pNewBuffer;
}

CBuffer *CBuffer::GetNextPack()
{
	if ( m_offsetNextPack < m_used )
	{
		CBuffer *pNewBuffer = m_allocator.Allocate();

		size_t canused = ( m_offsetNextPack + m_packLength > m_used ) ? ( m_used - m_offsetNextPack ) : m_packLength;
		
		pNewBuffer->AddData( m_buffer_ptr + m_offsetNextPack, canused );

		m_offsetNextPack += canused;

		return pNewBuffer;
	}

	return NULL;
}

void CBuffer::AddData( const char * const pData, size_t dataLength )
{
	if ( dataLength > m_size - m_used )
	{
		printf("CBuffer::AddData exception !!!\n");
		return;
	}

	memcpy( m_buffer_ptr + m_used, pData, dataLength );

	m_used += dataLength;
}

void CBuffer::AddData( const BYTE * const pData, size_t dataLength )
{
	AddData( reinterpret_cast< const char * >( pData ), dataLength );
}

void CBuffer::AddData( BYTE data )
{
	AddData( &data, 1 );
}

CBuffer *CBuffer::SplitBuffer( size_t bytesToRemove )
{
	CBuffer *pNewBuffer = m_allocator.Allocate();

	pNewBuffer->AddData( m_buffer_ptr, bytesToRemove );

	m_used -= bytesToRemove;

	memmove( m_buffer_ptr, m_buffer_ptr + bytesToRemove, m_used );

	return pNewBuffer;
}

CBuffer *CBuffer::AllocateNewBuffer() const
{
	return m_allocator.Allocate();
}

void CBuffer::Release()
{
	if ( m_ref == 0 )
	{
		printf("CBuffer::Release() Exception: m_ref is already zero\n");
		return;
	}

	if ( 0 == (--m_ref) )
	{
		m_used = 0;

		m_allocator.Release( this );
	}
}

void *CBuffer::operator new( size_t objectSize, size_t /* bufferSize */ )
{
	/*
	 * ASSERT( sizeof( DWORD ) == 4 );
	 *
	 * For four bytes aligned base on win32 system
	 */

	void *pMem = new char[ objectSize ]; //+ bufferSize + 4 ];

	return pMem;
}

void CBuffer::operator delete( void *pObject, size_t /* bufferSize */ )
{
	SAFE_DELETE_ARRAY( pObject );
}

/*
 * CBuffer::Allocator class
 */
CBuffer::Allocator::Allocator( size_t bufferSize, size_t maxFreeBuffers )
			: m_bufferSize( bufferSize )
			, m_maxFreeBuffers( maxFreeBuffers )
{

}

CBuffer::Allocator::~Allocator()
{
	try
	{
		Flush();
	}
	catch(...)
	{
		printf("CBuffer::Allocator::~Allocator exception!\n");
	}
}

CBuffer *CBuffer::Allocator::Allocate()
{
//	CCriticalSection::Owner lock( m_criticalSection );
	
	CBuffer *pBuffer = 0;
	
	if ( !m_freeList.Empty() )
	{
		pBuffer = m_freeList.PopNode();
		
		pBuffer->AddRef();
	}
	else
	{
		pBuffer = new( m_bufferSize )CBuffer( *this, m_bufferSize );
		
		if ( !pBuffer )
		{
			printf("CBuffer::Allocator::Allocate() Exception: Out of memory\n") ;
			return NULL;
		}
		
		OnBufferCreated();
	}
	
	m_activeList.PushNode( pBuffer );
	
	OnBufferAllocated();
	
	return pBuffer;
}

void CBuffer::Allocator::Flush()
{
//	CCriticalSection::Owner lock( m_criticalSection );
	
	while ( !m_activeList.Empty() )
	{
		OnBufferReleased();
		
		DestroyBuffer( m_activeList.PopNode() );
	}
	
	while ( !m_freeList.Empty() )
	{
		DestroyBuffer( m_freeList.PopNode() );
	}
}

void CBuffer::Allocator::Release( CBuffer *pBuffer )
{
	if ( !pBuffer )
	{
		printf("CBuffer::Allocator::Release() Exception: pBuffer is null\n");
		return;
	}
	
//	CCriticalSection::Owner lock( m_criticalSection );
	
	OnBufferReleased();
	
	/*
	 * unlink from the in use list
	 */
	
	pBuffer->RemoveFromList();
	
	if ( m_maxFreeBuffers == 0 || m_freeList.Count() < m_maxFreeBuffers )
	{
		pBuffer->Empty();           
		
		/*
		 * add to the free list
		 */
		
		m_freeList.PushNode( pBuffer );
	}
	else
	{
		DestroyBuffer( pBuffer );
	}
}

} // End of namespace OnlineGameLib
} // End of namespace Win32

namespace OnlineGameLib {

/*
 * CNodeList
 */
CNodeList::CNodeList()
		: m_pHead( 0 ),
		m_numNodes( 0 ) 
{
}

void CNodeList::PushNode( Node *pNode )
{
	ASSERT( pNode );

	pNode->AddToList( this );

	pNode->Next( m_pHead );

	m_pHead = pNode;

	++ m_numNodes;
}

CNodeList::Node *CNodeList::PopNode()
{
	Node *pNode = m_pHead;

	if ( pNode )
	{
		RemoveNode( pNode );
	}

	return pNode;
}

void CNodeList::RemoveNode( Node *pNode )
{
	ASSERT( pNode );

	if ( pNode == m_pHead )
	{
		m_pHead = pNode->Next();
	}

	pNode->Unlink();

	-- m_numNodes;
}

/*
 * CNodeList::Node
 */

CNodeList::Node::Node() 
		: m_pNext( 0 ),
		m_pPrev( 0 ),
		m_pList( 0 ) 
{
}

CNodeList::Node::~Node() 
{
	try
	{
		RemoveFromList();   
	}
	catch( ... )
	{
		TRACE( "CNodeList::Node::~Node() exception!" );
	}

	m_pNext = 0;
	m_pPrev = 0;
	m_pList = 0;
}

void CNodeList::Node::AddToList( CNodeList *pList )
{
	m_pList = pList;
}

void CNodeList::Node::RemoveFromList()
{
	if ( m_pList )
	{
		m_pList->RemoveNode( this );
	}
}

void CNodeList::Node::Unlink()
{
	if ( m_pPrev )
	{
		m_pPrev->m_pNext = m_pNext;
	}

	if ( m_pNext )
	{
		m_pNext->m_pPrev = m_pPrev;
	}
   
	m_pNext = 0;
	m_pPrev = 0;

	m_pList = 0;
}

} // End of namespace OnlineGameLib