#ifndef KAutoMutexH
#define	KAutoMutexH

class KMutex;

class ENGINE_API KAutoMutex
{
private:
	KMutex*		m_pMutex;
public:
	KAutoMutex(KMutex* pMutex);
	~KAutoMutex();
};
#endif
