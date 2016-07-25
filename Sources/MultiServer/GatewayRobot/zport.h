#ifndef ZPORT_H
#define ZPORT_H

//Windows相关的代码----------------------------------------------------------------------------------
#include <stdafx.h>
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
#endif
public:
  ZMutex() {
#ifdef WIN32
    InitializeCriticalSection(&mutex);
#else
    int rc = pthread_mutex_init(&mutex, NULL);
#endif                
  }
  ~ZMutex() {
#ifdef WIN32
    DeleteCriticalSection(&mutex);
#else
    int rc = pthread_mutex_destroy(&mutex);
#endif                
  }
  void lock() {
#ifdef WIN32        
    EnterCriticalSection(&mutex);
#else
    int rc = pthread_mutex_lock(&mutex);
#endif                
  }
  void unlock() {
#ifdef WIN32        
    LeaveCriticalSection(&mutex);
#else
    int rc = pthread_mutex_unlock(&mutex);
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

#endif
