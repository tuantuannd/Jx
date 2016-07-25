// S3PTestRoleInfoDAO.cpp: implementation of the S3PTestRoleInfoDAO class.
//
//////////////////////////////////////////////////////////////////////

#include "S3PTestRoleInfoDAO.h"

#include "S3PDBConnection.h"
#include "S3PRow.h"
#include "S3PRoleInfoDAO.h"
#include "S3PDBConnector.h"
#include "S3PEquipBaseInfoDAO.h"
#include "S3PEquipEfficInfoDAO.h"
#include "S3PEquipmentsDAO.h"
#include "S3PEquipRequireInfoDAO.h"
#include "S3PFightSkillDAO.h"
#include "S3PFriendListDAO.h"
#include "S3PLifeSkillDAO.h"
#include "S3PTaskListDAO.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

S3PTestRoleInfoDAO::~S3PTestRoleInfoDAO()
{

}

void S3PTestRoleInfoDAO::testAddGroup()
{
	std::list<ColumnAndValue*> data;
	ColumnAndValue d1;
	d1["cUserCode"] = "123";
	d1["bSex"]="男";

	ColumnAndValue d2;
	d2["cUserCode"] = "lws";
	d2["bSex"]="男";

	ColumnAndValue d3;
	d3["cUserCode"] = "123";
	d3["bSex"]="女";

	ColumnAndValue d4;
	d4["cUserCode"] = "不知道";
	d4["bSex"]="1";

	data.push_back(&d1);
	data.push_back(&d2);
	data.push_back(&d3);
	data.push_back(&d4);


	S3PDBConnection* pRoleCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_ROLEDB );

	if (NULL != pRoleCon)
	{
		S3PRoleInfoDAO role(pRoleCon);
		if ( role.AddGroup(data) <= 0 )
		{
			assert(false);
		}

		pRoleCon->Close();
	}
}

void S3PTestRoleInfoDAO::testAdd()
{
	S3PDBConnection* pRoleCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_ROLEDB );

	if (NULL != pRoleCon)
	{
		S3PRoleInfoDAO role(pRoleCon);
		ColumnAndValue cav;
		
		cav["cUserCode"] = "321";
		cav["bSex"] = "1";
		
		S3PRow row(role.GetTableName(), &cav, pRoleCon);
		if ( role.Add(&row) <= 0 )
		{
			assert(false);
		}

		S3PEquipBaseInfoDAO eb(pRoleCon);
		ColumnAndValue cavEb;
		cavEb["cInfoText"]="eb123";
		cavEb["iclass"]="01234";
		cavEb["imainid"]="123";
		S3PRow ebRow(eb.GetTableName(), &cavEb, pRoleCon);
		if (eb.Add(&ebRow)<=0)
		{
			assert(false);
		}

		S3PEquipEfficInfoDAO ee(pRoleCon);
		ColumnAndValue cavEe;
		cavEe["cInfoText"]="ee123";
		cavEe["iclass"]="01234";
		cavEe["imainid"]="123";
		S3PRow eeRow(ee.GetTableName(), &cavEe, pRoleCon);
		if (ee.Add(&eeRow)<=0)
		{
			assert(false);
		}

		S3PEquipmentsDAO eq(pRoleCon);
		ColumnAndValue cavEq;
		cavEq["cUserCode"]="eq123";
		cavEq["iEquipClassCode"]="01234";
		S3PRow eqRow(eq.GetTableName(), &cavEq, pRoleCon);
		if (eq.Add(&eqRow)<=0)
		{
			assert(false);
		}

		S3PEquipRequireInfoDAO er(pRoleCon);
		ColumnAndValue cavEr;
		cavEr["cInfoText"]="er123";
		cavEr["iclass"]="01234";
		cavEr["imainid"]="123";
		S3PRow erRow(er.GetTableName(), &cavEr, pRoleCon);
		if (er.Add(&erRow)<=0)
		{
			assert(false);
		}

		S3PFriendListDAO fl(pRoleCon);
		ColumnAndValue cavFl;
		cavFl["cUserCode"]="fl123";
		cavFl["cFriendCode"]="to123";
		S3PRow flRow(fl.GetTableName(), &cavFl, pRoleCon);
		if (fl.Add(&flRow)<=0)
		{
			assert(false);
		}

		S3PTaskListDAO tl(pRoleCon);
		ColumnAndValue cavTl;
		cavTl["cUserCode"]="fl123";
		cavTl["itaskcode"]="111";
		S3PRow tlRow(tl.GetTableName(), &cavTl, pRoleCon);
		if (tl.Add(&tlRow)<=0)
		{
			assert(false);
		}

		S3PLifeSkillDAO ls(pRoleCon);
		ColumnAndValue cavLs;
		cavLs["cUserCode"]="fl123";
		cavLs["iLifeSkill"]="112";
		S3PRow lsRow(ls.GetTableName(), &cavLs, pRoleCon);
		if (ls.Add(&lsRow)<=0)
		{
			assert(false);
		}

		S3PFightSkillDAO f(pRoleCon);
		ColumnAndValue cavF;
		cavF["cUserCode"]="fl123";
		cavF["iFightSkill"]="112";
		S3PRow fRow(f.GetTableName(), &cavF, pRoleCon);
		if (f.Add(&fRow)<=0)
		{
			assert(false);
		}
		pRoleCon->Close();
	}
}

void S3PTestRoleInfoDAO::testUpdate()
{
	S3PDBConnection* pRoleCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_ROLEDB );

	if (NULL != pRoleCon)
	{
		S3PRoleInfoDAO role(pRoleCon);
		ColumnAndValue cav;
		
		cav["cUserCode"] = "123456789";

		ColumnAndValue where;
		where["bSex"] = "男";
		
		S3PRow row(role.GetTableName(), &cav, pRoleCon);
		S3PRow rowWhere(role.GetTableName(), &where, pRoleCon);

		if ( role.Update(&row, &rowWhere) <= 0 )
		{
			assert(false);
		}

		pRoleCon->Close();
	}
}

void S3PTestRoleInfoDAO::testDelete()
{
	S3PDBConnection* pRoleCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_ROLEDB );

	if (NULL != pRoleCon)
	{
		S3PRoleInfoDAO role(pRoleCon);

		ColumnAndValue where;
		where["bSex"] = "男";
		
		S3PRow rowWhere(role.GetTableName(), &where, pRoleCon);

		if ( role.Delete() <= 0 )
		{
			assert(false);
		}

		pRoleCon->Close();
	}
}

void S3PTestRoleInfoDAO::testQuery()
{
	S3PDBConnection* pRoleCon =		// 连接角色数据库
		S3PDBConnector::Instance()->ApplyDBConnection( def_ROLEDB );

	if (NULL != pRoleCon)
	{
		S3PRoleInfoDAO role(pRoleCon);

		std::string query = "select cUserCode from Role_Info";

		S3PResult result;
		if ( role.Query(query, result) > 0)
		{
			for(int i=0; i<result.size(); i++)
			{
				ColumnAndValue cav = result[i];
				cout << "UserCode: " << cav["cusercode"] <<endl;
			}
		}
		else
		{
			assert(false);
		}

		pRoleCon->Close();
	}
}

Test *S3PTestRoleInfoDAO::suite ()
{
	TestSuite *testSuite = new TestSuite ("Test RoleInfo Table Manipulator");

	//testSuite->addTest (new TestCaller <S3PTestRoleInfoDAO> ("testAdd", testAdd));
	//testSuite->addTest (new TestCaller <S3PTestRoleInfoDAO> ("testAddGroup", testAddGroup));
	testSuite->addTest (new TestCaller <S3PTestRoleInfoDAO> ("testQuery", testQuery));
    //testSuite->addTest (new TestCaller <S3PTestRoleInfoDAO> ("testUpdate", testUpdate));
    //testSuite->addTest (new TestCaller <S3PTestRoleInfoDAO> ("testDelete", testDelete));
    
	return testSuite;
}

void S3PTestRoleInfoDAO::setUp()
{
}