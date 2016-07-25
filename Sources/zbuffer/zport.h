#ifndef ZPORT_H
#define ZPORT_H

//Windows相关的代码----------------------------------------------------------------------------------
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#endif

#include <stdio.h>
#include <string.h>

#ifndef WIN32
#define WINAPI
#define BYTE unsigned char
#define DWORD  unsigned long
#define LPVOID void *

#ifdef _DEBUG
//{
	void _trace(char *fmt, ...);

	#ifndef ASSERT
		#define ASSERT(x) { if ( !( x ) ) _asm{ int 0x03 } }
	#endif

	#ifndef VERIFY
	#define VERIFY(x) { if ( !( x ) ) _asm{ int 0x03 } }
	#endif
//}
#else
//{
	#ifndef ASSERT
		#define ASSERT(x)
	#endif

	#ifndef VERIFY
		#define VERIFY(x) x
	#endif
//}
#endif

#ifdef _DEBUG
//{
	#ifndef TRACE
		#define TRACE _trace
	#endif
//}
#else
//{
	inline void _trace(const char* fmt, ...) { }
	#ifndef TRACE
		#define TRACE  1 ? (void)0 : _trace
	#endif
//}
#endif

#define INIT_PTR(x)				do{ (x) = NULL; }while(0);

#define SAFE_DELETE(x)			try{ if( (x) != NULL ) { delete (x); (x) = NULL; } } catch(...) { TRACE("SAFE_DELETE error\n"); }
#define SAFE_DELETE_ARRAY(x)	try{ if( (x) != NULL ) { delete[] (x); (x) = NULL; } } catch(...) { TRACE("SAFE_DELETE_ARRAY error\n"); }

#ifndef SAFE_FREE
	#undef SAFE_FREE
	#define SAFE_FREE(x)			try{ if( (x) != NULL ) { free(x); (x)=NULL; } } catch(...) { TRACE("SAFE_FREE error\n"); }
#endif

#ifndef SAFE_RELEASE
	#undef SAFE_RELEASE
	#define SAFE_RELEASE(x)			try{ if( (x) != NULL ) { (x)->Release(); (x) = NULL; } } catch(...) { TRACE("SAFE_RELEASE error\n"); }
#endif

#define SAFE_CLOSEHANDLE(x)		try{ if (x) { CloseHandle(x); (x) = NULL; } } catch(...) { TRACE("SAFE_CLOSEHANDLE error\n"); }
#define SAFE_FREELIB(x)			try{ if (x) { FreeLibrary(x); (x) = NULL; } } catch(...) { TRACE("SAFE_FREELIB error\n"); }

/*
 * Quick and safely convert number format
 */

#ifndef MAKEFOURCC
//{
	#define MAKEFOURCC(ch0, ch1, ch2, ch3)						\
		((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |		\
		((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
//}
#endif

#define COUNT_OF_ARRAY(array) ( sizeof( array ) / sizeof( array[0] ) )

/*
 * 'c' must be a char and not be a expression in the _private macro 
 */
#define _private_IS_NUM(c)     ((c) >= '0' && (c) <= '9')
#define _private_IS_SPACE(c)   ((c) == ' ' || (c) == '\r' || (c) == '\n' || (c) == '\t' || (c) == 'x')

#define IS_NUM(c)	_private_IS_NUM(c)
#define IS_SPACE(c)	_private_IS_SPACE(c)

inline void strlwr(char* sz)
{
	char c;
	for (int i = 0; 0 != (c = sz[i]); i++)
	{
		if (c >= 'A' && c <= 'Z')
			sz[i] = c + ('a' - 'A');
	}
}

#define SOCKET int
#define closesocket close
#define INVALID_SOCKET -1
#endif
//封装的互斥量类
class ZMutex {
#ifdef WIN32
  CRITICAL_SECTION mutex;
#else
public:
  pthread_mutex_t mutex;
  //static int all_count;
  int count;
#endif
public:
  ZMutex() {
#ifdef WIN32
    InitializeCriticalSection(&mutex);
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int rc = pthread_mutex_init(&mutex, &attr);
	//printf("ZMutex Created..\n");
	count = 0;
#endif                
  }
  ~ZMutex() {
#ifdef WIN32
    DeleteCriticalSection(&mutex);
#else
    int rc = pthread_mutex_destroy(&mutex);
	//printf("ZMutex released..\n");
#endif                
  }
  bool trylock()
  {
#ifdef WIN32
	  return false;
#else
	int rc = pthread_mutex_trylock(&mutex);
/*	if (rc != 0)
	{
		printf("trylock mutex error %d\n", rc);
	}*/
	return (0 == rc);
#endif
  }
  void lock() {
#ifdef WIN32        
    EnterCriticalSection(&mutex);
#else
    int rc = pthread_mutex_lock(&mutex);
	if (rc != 0)
	{
		printf("lock mutex error %d\n", rc);
	}
	//all_count++;
	count++;
#endif                
  }
  void unlock() {
#ifdef WIN32        
    LeaveCriticalSection(&mutex);
#else
	//all_count--;
	count--;
    int rc = pthread_mutex_unlock(&mutex);
	if (rc != 0)
	{
		printf("unlock mutex error %d\n", rc);
	}
#endif                
  }
};

//封装的定时器类(精确到毫秒)
class ZTimer {
public:
  static inline unsigned long now() {				//返回当前的毫秒数
#ifdef WIN32        
    return GetTickCount();
#else
    return 0;
#endif
  }
};

//封装的线程类,继承这个类可以实现
class ZThread {
#ifdef WIN32
  unsigned long id;
  HANDLE handle;
#else
  pthread_t p_thread;
#endif
public:
	bool bStop;
	ZThread() {
#ifdef WIN32
		id = -1;
#endif
		bStop = false;
	}
	void start();
	void stop();
	virtual int action() = 0;
};

#include <map>
#include <list>

namespace OnlineGameLib {

/*
 * CNodeList
 */
class CNodeList
{
public:

	class Node
    {
	public:

		Node *Next() const { return m_pNext; };

		void Next( Node *pNext );

		void AddToList( CNodeList *pList );

		void RemoveFromList();

	protected:

		Node();
		~Node();

	private:

		friend class CNodeList;

		void Unlink();

		Node *m_pNext;
		Node *m_pPrev;

		CNodeList *m_pList;
	};

	CNodeList();

	void PushNode( Node *pNode );

	Node *PopNode();

	Node *Head() const { return m_pHead; };

	size_t Count() const { return m_numNodes; };

	bool Empty() const { return ( 0 == m_numNodes ); };

private:

	friend void Node::RemoveFromList();

	void RemoveNode( Node *pNode );

	Node *m_pHead; 

	size_t m_numNodes;
};

inline void CNodeList::Node::Next( Node *pNext )
{
	m_pNext = pNext;

	if ( pNext )
	{
		pNext->m_pPrev = this;
	}
}

/*
 * TNodeList
 */

template <class T> class TNodeList : public CNodeList
{
public:
         
	T *PopNode();
   
	T *Head() const;

	static T *Next( const T *pNode );
};

template <class T>
T *TNodeList<T>::PopNode()
{
	return static_cast< T* >( CNodeList::PopNode() );
}

template <class T>
T *TNodeList<T>::Head() const
{
	return static_cast< T* >( CNodeList::Head() );
}

template <class T>
T *TNodeList<T>::Next( const T *pNode )
{
	return static_cast< T* >( pNode->Next() );
}

} // End of namespace OnlineGameLib	

namespace OnlineGameLib {
namespace Win32 {

/*
 * CBuffer class
 */
class CBuffer : public CNodeList::Node
{
public:
	
	class Allocator;

	friend class Allocator;

	size_t GetUsed() const { return m_used; };
	size_t GetSize() const { return m_size; };

	const BYTE *GetBuffer() const { return m_buffer_ptr; };

	CBuffer *GetHeadPack( size_t packLength );
	CBuffer *GetNextPack();
	bool	 HaveNextPack() { return ( m_offsetNextPack < m_used ) ? true : false; };
	
	void AddData( const char * const pData, size_t dataLength );
	void AddData( const BYTE * const pData, size_t dataLength );
	void AddData( BYTE data );

	void Use( size_t dataUsed ) { m_used += dataUsed; };

	CBuffer *SplitBuffer( size_t bytesToRemove );

	CBuffer *AllocateNewBuffer() const;

	void ConsumeAndRemove( size_t bytesToRemove );

	void Empty() { m_used = 0; };

	void AddRef() { m_ref++; };
	void Release();

private:

	Allocator &m_allocator;

	long			m_ref;

	const size_t	m_size;
	size_t			m_used;

	size_t			m_packLength;
	size_t			m_offsetNextPack;

	/*
	 * start of the actual buffer, must remain the last
	 * data member in the class.
	 */
	BYTE *m_buffer_ptr;			// four bytes aligned
	
	//BYTE m_buffer_base_addr[0];

private:

	static void *operator new( size_t objSize, size_t bufferSize );
	static void operator delete( void *pObj, size_t bufferSize );
      
	CBuffer( Allocator &allocator, size_t size );
	~CBuffer();

	/*
	 * No copies do not implement
	 */
	CBuffer( const CBuffer &rhs );
	CBuffer &operator=( const CBuffer &rhs );
	
};

inline void CBuffer::ConsumeAndRemove( size_t bytesToRemove )
{
	m_used -= bytesToRemove;

	memmove( m_buffer_ptr, m_buffer_ptr + bytesToRemove, m_used );
}


/*
 * CBuffer::Allocator class
 */
class CBuffer::Allocator
{
public:

	friend class CBuffer;

	explicit Allocator( size_t bufferSize, size_t maxFreeBuffers );

	virtual ~Allocator();

	CBuffer *Allocate();

	size_t GetBufferSize() const { return m_bufferSize; };
	
	bool ReSet( size_t bufferSize, size_t maxFreeBuffers )
	{
		memcpy((void*)&m_bufferSize, &bufferSize, sizeof( size_t ) );
		memcpy((void*)&m_maxFreeBuffers, &maxFreeBuffers, sizeof( size_t ) );

		return true;
	}

protected:

	void Flush();

private:
	
	void Release( CBuffer *pBuffer );
	
	virtual void OnBufferCreated() {}
	virtual void OnBufferAllocated() {}
	virtual void OnBufferReleased() {}

	void DestroyBuffer( CBuffer *pBuffer )
	{
		SAFE_DELETE( pBuffer );
	}

	const size_t m_bufferSize;

	typedef TNodeList< CBuffer > BufferList;

	BufferList m_freeList;
	BufferList m_activeList;

	const size_t m_maxFreeBuffers;

    /*
	 * No copies do not implement
	 */
	Allocator( const Allocator &rhs );
	Allocator &operator=( const Allocator &rhs );

};

} // End of namespace OnlineGameLib
} // End of namespace Win32

#endif
