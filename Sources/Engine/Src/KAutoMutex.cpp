#include "KWin32.h"
#include "KEngine.h"
#include "KMutex.h"
#include "KAutoMutex.h"

KAutoMutex::KAutoMutex(KMutex* pMutex)
{
	m_pMutex = pMutex;
	if (m_pMutex)
		m_pMutex->Lock();
}

KAutoMutex::~KAutoMutex()
{
	if (m_pMutex)
	{
		m_pMutex->Unlock();
		m_pMutex = NULL;
	}
}
