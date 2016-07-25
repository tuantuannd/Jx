#include "stdafx.h"
#include "Buffer.h"

#include "Exception.h"
#include "Utils.h"

/*
 * namespace OnlineGameLib::Win32
 */

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
		DEBUG_ONLY( Message( "CBuffer::AddData - Not enough space in buffer!" ) );

		throw CException( _T("CBuffer::AddData"), _T("Not enough space in buffer") );
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
		throw CException( _T("CBuffer::Release()"), _T("m_ref is already zero") );
	}

	if ( 0 == ::InterlockedDecrement( &m_ref ) )
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
		TRACE( "CBuffer::Allocator::~Allocator exception!" );
	}
}

CBuffer *CBuffer::Allocator::Allocate()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
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
			throw CException( _T("CBuffer::Allocator::Allocate()"), _T("Out of memory") );
		}
		
		OnBufferCreated();
	}
	
	m_activeList.PushNode( pBuffer );
	
	OnBufferAllocated();
	
	return pBuffer;
}

void CBuffer::Allocator::Flush()
{
	CCriticalSection::Owner lock( m_criticalSection );
	
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
		throw CException( _T("CBuffer::Allocator::Release()"), _T("pBuffer is null") );
	}
	
	CCriticalSection::Owner lock( m_criticalSection );
	
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

CPackager::CPackager( size_t bufferSize /* = 65536  */, size_t maxFreeBuffers /* = 16  */ )
		: m_theAllocator( bufferSize, maxFreeBuffers )
{

}

CPackager::~CPackager()
{
	Empty();
}

void CPackager::AddData( BYTE cID, const char * const pData, size_t dataLength, unsigned long lnUserData /* = 0 */ )
{
	CCriticalSection::Owner lock( m_csSend );

	BUFFER_MAP::iterator it;

	if ( m_theSend.end() != ( it = m_theSend.find( cID ) ) )
	{
		CBuffer *pBuffer = m_theSend[cID];

		ASSERT( pBuffer );

		pBuffer->SetUserData( lnUserData );

		pBuffer->AddData( pData, dataLength );
	}
	else
	{
		CBuffer *pBuffer = m_theAllocator.Allocate();

		ASSERT( pBuffer );

		pBuffer->SetUserData( lnUserData );
		
		pBuffer->AddData( pData, dataLength );
		
		m_theSend[cID] = pBuffer;
	}
}

void CPackager::AddData( BYTE cID, const BYTE * const pData, size_t dataLength, unsigned long lnUserData /* = 0 */ )
{
	AddData( cID, reinterpret_cast< const char * >( pData ), dataLength, lnUserData );
}

void CPackager::DelData( BYTE cID )
{
	CCriticalSection::Owner lock( m_csSend );

	BUFFER_MAP::iterator it;

	if ( m_theSend.end() != ( it = m_theSend.find( cID ) ) )
	{
		CBuffer *pBuffer = m_theSend[cID];

		SAFE_RELEASE( pBuffer );

		m_theSend.erase( cID );
	}
}

CBuffer *CPackager::GetHeadPack( BYTE cID, size_t packLength /* = 512 */ )
{
	CCriticalSection::Owner lock( m_csSend );

	BUFFER_MAP::iterator it;

	if ( m_theSend.end() != ( it = m_theSend.find( cID ) ) )
	{
		CBuffer *pBuffer = m_theSend[cID];

		CBuffer *pPack = pBuffer->GetHeadPack( packLength - 2 /* enumPackHeader + cID */);

		if ( NULL == pPack )
		{
			return NULL;
		}

		if ( 0 == pPack->GetUsed() )
		{
			pPack->Release();

			return NULL;
		}

		CBuffer *pNewBufer = m_theAllocator.Allocate();

		/*
		 * Add a prototype
		 */
		pNewBufer->AddData( cID );

		/*
		 * Add a flag
		 */
		BYTE cFlag = enumPackHeader;

		if ( !pBuffer->HaveNextPack() )
		{
			cFlag |= enumPackTail;
		}
		
		pNewBufer->AddData( cFlag );

		/*
		 * Add a user data into this buffer
		 */
		unsigned long lnUserData = pBuffer->GetUserData();

		pNewBufer->AddData( ( const char * )( &lnUserData ), sizeof( unsigned long ) );

		/*
		 * Add some data into this buffer
		 */
		pNewBufer->AddData( pPack->GetBuffer(), pPack->GetUsed() );

		pPack->Release();

		return pNewBufer;
	}

	return NULL;
}

CBuffer *CPackager::GetNextPack( BYTE cID )
{
	CCriticalSection::Owner lock( m_csSend );

	BUFFER_MAP::iterator it;

	if ( m_theSend.end() != ( it = m_theSend.find( cID ) ) )
	{
		CBuffer *pBuffer = m_theSend[cID];

		CBuffer *pPack = pBuffer->GetNextPack();

		if ( NULL == pPack )
		{
			return NULL;
		}

		if ( 0 == pPack->GetUsed() )
		{
			pPack->Release();

			return NULL;
		}

		CBuffer *pNewBufer = m_theAllocator.Allocate();

		pNewBufer->AddData( cID );

		BYTE cFlag = enumPackMiddle;

		if ( !pBuffer->HaveNextPack() )
		{
			cFlag |= enumPackTail;
		}

		pNewBufer->AddData( cFlag );

		/*
		 * Add a user data into this buffer
		 */
		unsigned long lnUserData = pBuffer->GetUserData();

		pNewBufer->AddData( ( const char * )( &lnUserData ), sizeof( unsigned long ) );

		/*
		 * Add some data into this buffer
		 */
		pNewBufer->AddData( pPack->GetBuffer(), pPack->GetUsed() );

		pPack->Release();

		return pNewBufer;
	}

	return NULL;	
}

CBuffer *CPackager::PackUp( const void *pData, size_t dataLength )
{
	CCriticalSection::Owner lock( m_csRecv );

	if ( NULL == pData || dataLength < 3 /* cID + cPackFlag + ... */ )
	{
		return NULL;
	}

	BYTE cID = CPackager::Peek( pData );

	BUFFER_MAP::iterator it;

	if ( m_theRecv.end() == ( it = m_theRecv.find( cID ) ) )
	{
		CBuffer *pPack = m_theAllocator.Allocate();

		m_theRecv[cID] = pPack;
	}
	
	CBuffer *pBuffer = m_theRecv[cID];
	
	ASSERT( pBuffer );
	
	BYTE cFlag = CPackager::Peek( pData, 1 );

	const size_t nDataBegin = 1 + 1 + sizeof( unsigned long );
	
	if ( _Header( cFlag ) )
	{
		pBuffer->Empty();
		
		pBuffer->AddData( ( ( const char * )pData + nDataBegin ), dataLength - nDataBegin );
	}
	
	if ( _Middle( cFlag ) )
	{
		pBuffer->AddData( ( ( const char * )pData + nDataBegin ), dataLength - nDataBegin );
	}
	
	if ( _Tail( cFlag ) )
	{
		unsigned long lnUserData = *( const unsigned long * )( ( const char * )pData + 2 );

		pBuffer->SetUserData( lnUserData );

		pBuffer->AddRef();
		
		return pBuffer;
	}
	
	if ( _Error( cFlag ) )
	{
		ASSERT( NULL && "CPackager::PackUp - Invalid package!" );
	}

	return NULL;	
}

void CPackager::Empty()
{
	/*
	 * Send
	 */
	{
		CCriticalSection::Owner lock( m_csSend );

		BUFFER_MAP::iterator it;

		for ( it = m_theSend.begin(); it != m_theSend.end(); it ++ )
		{
			CBuffer *pBuffer = (*it).second;

			SAFE_RELEASE( pBuffer );
		}

		m_theSend.erase( m_theSend.begin(), m_theSend.end() );
	}

	/*
	 * Recv
	 */
	{
		CCriticalSection::Owner lock( m_csRecv );

		BUFFER_MAP::iterator it;

		for ( it = m_theRecv.begin(); it != m_theRecv.end(); it ++ )
		{
			CBuffer *pBuffer = (*it).second;

			SAFE_RELEASE( pBuffer );
		}

		m_theRecv.erase( m_theRecv.begin(), m_theRecv.end() );
	}
}

} // End of namespace OnlineGameLib
} // End of namespace Win32