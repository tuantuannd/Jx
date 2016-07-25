#ifndef KSynLock_H
#define KSynLock_H

#include <Kengine.h>

class KSynLock
{
public:
	KSynLock(){m_nCount = 0; m_bLock = FALSE;m_nCurId = 0;};
	virtual ~KSynLock() {};
	BOOL m_bLock;
	long m_nCount;//访问该数据已被锁定的量
	long m_nCurId;//目前具有访问权限的锁定值
	long Lock();	 //检查是否数据已经被锁定，如果是的话返回当前的值
	BOOL UnLock(long index);//反锁
	long GetCurId(){return m_nCurId;};
	long GetCount(){return m_nCount;};
	//	virtual void * GetData() = 0;
	
};

#endif //KSynLock_H
