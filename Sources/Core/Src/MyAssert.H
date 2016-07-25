
#ifndef __MYASSERT_H
#define __MYASSERT_H
#ifndef _STANDALONE
#include	<crtdbg.h>			// for _ASSERT
#endif
#ifdef _DEBUG

	#ifndef ASSERT_VALID
	#define	ASSERT_VALID(pObj) \
			{	\
				_ASSERT (pObj != NULL); \
				pObj->AssertValid(); \
			}
	#endif	//#ifndef ASSERT_VALID

#else

	#ifndef ASSERT_VALID
	#define	ASSERT_VALID(exp)
	#endif

#endif

#endif		//__MYASSERT_H
