#include <stdafx.h>
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
