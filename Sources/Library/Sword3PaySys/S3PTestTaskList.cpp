// S3PTestTaskList.cpp: implementation of the S3PTestTaskList class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestTaskList.h"
//#include "S3PTaskList.h"
#include "S3PTask.h"
#include "S3PRow.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTestTaskList::~S3PTestTaskList()
{

}

Test* S3PTestTaskList::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test class S3PTaskList");

	testSuite->addTest( new TestCaller <S3PTestTaskList> ("test", test) );
    
	return testSuite;
}

void S3PTestTaskList::test()
{
//	S3PTaskList tl("lwsdd");
	int i;
//	tl.DeleteAll();
//
//	ColumnAndValue v;
//	v["itaskcode"] = "123";
//	tl.Add(v);
//
//	for(i=0; i<tl.Size(); i++)
//	{
//		ColumnAndValue & cav = tl[i]->GetProperties();
//		cav["itaskcode"] = "321";
//		assert(tl[i]->Save());
//	}
	//for(i=0; i<tl.Size(); i++)
	//{
	//	ColumnAndValue cav = tl[i]->GetProperties();
	//	printf("task_list itaskcode: %s\r\n", cav["itaskcode"].c_str());
	//	printf("task_list cUserCode: %s\r\n", cav["cusercode"].c_str());
	//}
}