//---------------------------------------------------------------------------
// Sword3 Core (c) 2002 by Kingsoft
//
// File:	KBasPropTbl.CPP
// Date:	2002.08.14
// Code:	DongBo
// Desc:    cpp file. 本文件实现的类用于从tab file中读出道具的初始属性,
//			并生成对应的属性表
//---------------------------------------------------------------------------

#include "KCore.h"
#include "KTabFile.h"
#include "MyAssert.H"
#include "KBasPropTbl.h"

#define		TABFILE_PATH				"\\settings\\item"
#define		TABFILE_MINE				"minebase.txt"
#define		TABFILE_TASK				"questkey.txt"
#define		TABFILE_MEDICINE			"potion.txt"
#define		TABFILE_MEDMATERIAL			"medmaterialbase.txt"
#define		TABFILE_MELEEWEAPON			"MeleeWeapon.txt"
#define		TABFILE_RANGEWEAPON			"RangeWeapon.txt"
#define		TABFILE_ARMOR				"Armor.txt"
#define		TABFILE_HELM				"Helm.txt"
#define		TABFILE_BOOT				"Boot.txt"
#define		TABFILE_BELT				"Belt.txt"
#define		TABFILE_AMULET				"Amulet.txt"
#define		TABFILE_RING				"Ring.txt"
#define		TABFILE_CUFF				"Cuff.txt"
#define		TABFILE_PENDANT				"Pendant.txt"
#define		TABFILE_HORSE				"Horse.txt"
#define		TABFILE_TOWNPORTAL			"TownPortal.txt"
#define		TABFILE_EQUIPMENT_UNIQUE	"unique.txt"
#define		TABFILE_MAGICATTRIB			"magicattrib.txt"
#define		TABFILE_GOLDITEM			"GoldItem.txt"

// 以下定义的结构用于辅助从tabfile中读出属性的初始值
typedef struct tagPROPINFO
{
	int		m_nType;		// 属性的类型. 详见 PI_VARTYPE_...系列定义
	union
	{
	char*	m_pszBuf;		// 指向字符串缓冲区的指针
	int*	m_pnData;		// 指向int变量的指针
	}m_pData;
	int		m_nBufSize;		// 缓冲区的长度
} PROPINFO;
#define		PI_VARTYPE_CHAR		0
#define		PI_VARTYPE_INT		1

char*	TABFILE_EQUIPMENT[] = 
{
	TABFILE_MELEEWEAPON,	//"MeleeWeapon.txt",
	TABFILE_RANGEWEAPON,	//"RangeWeapon.txt",
	TABFILE_ARMOR,			//"Armor.txt",
	TABFILE_HELM,			//"Helm.txt",
	TABFILE_BOOT,			//"Boot.txt",
	TABFILE_BELT,			//"Belt.txt",
	TABFILE_AMULET,			//"Amulet.txt",
	TABFILE_RING,			//"Ring.txt",
	TABFILE_CUFF,			//"Cuff.txt",
	TABFILE_PENDANT,		//"Pendant.txt",
	TABFILE_HORSE,			//"Horse.txt",
};

//int GetRandomNumber(int nMin, int nMax);

//=============================================================================

/******************************************************************************
	功能:	从tab file中读入特定的数据记录
	入口:	pTF: 工具类指针, 用此工具类读取tab file
			nRow: 读第nRow项记录
			pPI[i].m_nType: 给出欲读的记录中第i个域的类型, 可能是整型或字符串
			pPI[i].m_pData: 将数据读到此缓冲区中
			cbFields: 每项记录包含这么多的域
	出口:	成功时返回非零, m_pBuf 指向分配的内存
			失败时返回零
******************************************************************************/
BOOL LoadRecord(IN KTabFile* pTF, IN int nRow,
				IN OUT const PROPINFO* pPI, IN int cbFields)
{
	BOOL bEC = TRUE;

	nRow += 2;	// 加1: 跳过tabfile的第一行. 该行给出的是各列的名称
				// 再加1: KTabFile::GetInteger()函数要求nRow从1开始算起

	// 逐个读入各项属性
	for (int n = 0; n < cbFields; n++)
	{
		if (PI_VARTYPE_INT == (pPI+n)->m_nType)
		{	// 读入 int 型数据
			if (FALSE == pTF->GetInteger(nRow, n+1, -1, (pPI+n)->m_pData.m_pnData))
				{ _ASSERT(FALSE); bEC = FALSE; break; }
		}
		else
		{	// 读入字符串型数据
			_ASSERT(PI_VARTYPE_CHAR == (pPI+n)->m_nType);

			if (FALSE == pTF->GetString(nRow, n+1, "", (pPI+n)->m_pData.m_pszBuf,
										(pPI+n)->m_nBufSize))
				{ _ASSERT(FALSE); bEC = FALSE; break; }
		}
	}
	return bEC;
}

//=============================================================================

KLibOfBPT::KLibOfBPT()
{
}

KLibOfBPT::~KLibOfBPT()
{
}

/******************************************************************************
	功能：	总控，从tab file中读入道具,魔法等原始数据
	出口:	相关数据存在m_BPTWeapon,m_BPTWeaponDirty等成员变量中
******************************************************************************/
BOOL KLibOfBPT::Init()
{
	// 初始化
	KBasicPropertyTable*	paryBPT[] = {	
											&m_BPTMeleeWeapon,
											&m_BPTRangeWeapon,
											&m_BPTArmor,
											&m_BPTHelm,
											&m_BPTBoot,
											&m_BPTBelt,
											&m_BPTAmulet,
											&m_BPTRing,
											&m_BPTCuff,
											&m_BPTPendant,
											&m_BPTHorse,
											&m_BPTMedicine,
											&m_BPTQuest,
											&m_BPTTownPortal,
											&m_BPTMagicAttrib,
										};
	// 将tab file逐个读入
	const int cbNumOfTables = sizeof(paryBPT)/sizeof(paryBPT[0]);
	for (int i = 0; i < cbNumOfTables; i++)
	{
		if (i < 11)	// 装备特殊处理
		{
			KBPT_Equipment* pTemp = (KBPT_Equipment *)paryBPT[i];
			pTemp->Init(i);
		}
		if (FALSE == paryBPT[i]->Load())
			{ _ASSERT(FALSE); return FALSE; }
	}

	// 构造魔法属性表
	InitMALib();
    
    // 构造魔法属性索引表
    // add by Freeway Chen in 2003.5.30
    InitMAIT();

	return TRUE;
}

/******************************************************************************
	功能：	建立一个四维的数组，利用(前后缀、物品类型、五行、级别)
            确定一个魔法属性的索引值列表
	入口:	m_BPTMagicAttrib: 内含从tab file读入的全部魔法属性
	出口:	四维数组 m_CMAIT 分别填充相应的索引值列表
******************************************************************************/
// Add by Freeway Chen in 2003.5.30
BOOL KLibOfBPT::InitMAIT()
{
    int nResult = false;
    int i = 0;
    int nPrefixPostfix = 0;
    int nType = 0;
    int nSeries = 0;
    int nSeriesMin = 0;
    int nSeriesMax = 0;

    int nLevel  = 0;
    int nLevelMin = 0;
    int nLevelMax = 0;

    int nTotalCount = 0;

    for (i = 0; i < m_BPTMagicAttrib.NumOfEntries(); i++)
    {
        KMAGICATTRIB_TABFILE *pItem = (KMAGICATTRIB_TABFILE *)m_BPTMagicAttrib.GetRecord(i);
        if (!pItem)
        {
            _ASSERT(pItem);
            continue;
        }

        pItem->m_nUseFlag = false;

        nPrefixPostfix = pItem->m_nPos;
        _ASSERT((nPrefixPostfix >= 0) && (nPrefixPostfix < MATF_PREFIXPOSFIX));

        for (nType = 0; nType < MATF_CBDR; nType++)
        {
            if ((pItem->m_DropRate[nType]) == 0)
                continue; // 如果没有概率出现这个类型就跳到下一个
            
            nSeriesMin = nSeriesMax = pItem->m_nClass;
            if ((pItem->m_nClass) == -1)
            {
                nSeriesMin = 0;
                nSeriesMax = MATF_SERIES - 1;
            }
            else
            {
                _ASSERT(((pItem->m_nClass) >= 0) && ((pItem->m_nClass) < MATF_SERIES));
            }

            for (nSeries = nSeriesMin; nSeries <= nSeriesMax; nSeries++)
            {
                nLevelMin = pItem->m_nLevel;
                nLevelMax = MATF_LEVEL;

                for (nLevel = nLevelMin; nLevel <= nLevelMax; nLevel++)
                {  
                    m_CMAIT[nPrefixPostfix][nType][nSeries][nLevel - 1].Insert(i);
                    nTotalCount++;
                }
            }
        }
    }

    //#ifdef _DEBUG
    #if 0
    //g_DebugLog("[魔法属性]%s五防抗性上限增加%d", pNpc->Name, pMagic->nValue[0]);
    printf("[魔法属性]m_CMAIT[前缀后缀][类型][五行][级别]\n");
    for (nPrefixPostfix = 0; nPrefixPostfix < MATF_PREFIXPOSFIX; nPrefixPostfix++)
    {
        for (nType = 0; nType < MATF_CBDR; nType++)
        {
            for (nSeries = 0; nSeries < MATF_SERIES; nSeries++)
            {
                for (nLevel = 1; nLevel < (MATF_LEVEL + 1); nLevel++)
                {
                    KBPT_ClassMAIT *pMAITItem = &m_CMAIT[nPrefixPostfix][nType][nSeries][nLevel - 1];

                    char szOutputString[8192];
                    char szStringContent[4096];
                    szStringContent[0] = '\0';

                    for (i = 0; i < pMAITItem->GetCount(); i++)
                    {
                        KMAGICATTRIB_TABFILE *pItem = (KMAGICATTRIB_TABFILE *)m_BPTMagicAttrib.GetRecord(pMAITItem->Get(i));
                        char szTemp[1024];
                        sprintf(szTemp, " %3d(%-8s) ", pMAITItem->Get(i) + 2, pItem->m_szName);

                        strcat(szStringContent, szTemp);
                    }

                    sprintf(szOutputString, 
                        "[魔法属性]m_CMAIT[%d][%d][%d][%d]: Count = %3d, %s  \n",
                        nPrefixPostfix,
                        nType,
                        nSeries,
                        nLevel - 1,
                        pMAITItem->GetCount(),
                        szStringContent
                    );
                    printf("%s", szOutputString);
                    //g_DebugLog("%s", szOutputString);
                    //OutputDebugString(szOutputString);

                }
            }
        }
    }
    ExitProcess(0); // for redirect to File save

    #endif // _DEBUG


    nResult = true;
//Exit0:
    return nResult;
}


/******************************************************************************
	功能：	根据原始的魔法属性表，统计出每种装备各有哪些可能的魔法属性
	入口:	m_BPTMagicAttrib: 内含从tab file读入的全部魔法属性
	出口:	原始的魔法属性表被分类, 分类后的数据存入m_CMAT数组中
			m_CMAT[i]中给出了适用于第i种装备的全部魔法属性
******************************************************************************/
BOOL KLibOfBPT::InitMALib()
{
	BOOL bEC = FALSE;

	// 确定每种装备各有多少种可能的魔法属性
    int naryMACount[2][MATF_CBDR];	// 第i种装备共有naryMACount[0][i]种可能的前缀
									//			  和naryMACount[1][i]种可能的后缀
	m_BPTMagicAttrib.GetMACount((int*)naryMACount);

	// 根据取回的数值，为各装备的魔法属性索引表分配内容
	for (int i = 0; i < MATF_CBDR - 1; i++)	// -1 because of horse
	{
		_ASSERT(naryMACount[0][i] > 0);	// 不可能没有一个魔法前缀适用于该装备
		_ASSERT(naryMACount[1][i] > 0);	// 不可能没有一个魔法后缀适用于该装备
		if (FALSE == m_CMAT[0][i].GetMemory(naryMACount[0][i]))
			return bEC;
		if (FALSE == m_CMAT[1][i].GetMemory(naryMACount[1][i]))
			return bEC;
	}

	// 遍历魔法属性表，建立起各装备的魔法属性索引表
	const int nNumOfMA = m_BPTMagicAttrib.NumOfEntries();	// 魔法属性的总数
	_ASSERT(nNumOfMA > 0);
	int m, n;		// 第m项魔法属性适用于第n种装备
	for (m = 0; m < nNumOfMA; m++)				// 遍历所有魔法属性
	{
		const KMAGICATTRIB_TABFILE* pRec;
		pRec = m_BPTMagicAttrib.GetRecord(m);		
		for (n = 0; n < MATF_CBDR; n++)			// 共有MATF_CBDR种装备带魔法
		{	// 确认魔法属性适用于哪些装备
			if (0 != pRec->m_DropRate[n])
			{	// 运行至此, 说明第m项魔法属性适用于第n种装备
				int nPos;
				nPos = (0 == pRec->m_nPos) ? 1 : 0;
				m_CMAT[nPos][n].Set(m);
			}
		}
	}
	bEC = TRUE;
	return bEC;
}

// flying add here
const KBASICPROP_EQUIPMENT_GOLD* KLibOfBPT::GetGoldItemRecord(IN int nIndex) const
{
	return (KBASICPROP_EQUIPMENT_GOLD*)m_GoldItem.GetRecord(nIndex);
}

const int KLibOfBPT::GetGoldItemNumber() const
{
	return m_GoldItem.GetRecordCount();
}
/******************************************************************************
	功能：	获取指定的CMAT
******************************************************************************/
const KBPT_ClassifiedMAT* KLibOfBPT::GetCMAT(int nPos, int i) const
{
	_ASSERT(this != NULL);

	if (nPos != 0 && nPos != 1)
		{ _ASSERT(FALSE); return NULL; }
	if (i < 0 || i >= MATF_CBDR)
		{ _ASSERT(FALSE); return NULL; }
	return &(m_CMAT[nPos][i]);
}

/******************************************************************************
	功能：	获取指定的CMAIT
******************************************************************************/
// Add by Freeway Chen in 2003.5.30
const KBPT_ClassMAIT*       KLibOfBPT::GetCMIT(IN int nPrefixPostfix, IN int nType, IN int nSeries, int nLevel) const
{
    _ASSERT((nPrefixPostfix >= 0) && (nPrefixPostfix < MATF_PREFIXPOSFIX));
    if (!((nPrefixPostfix >= 0) && (nPrefixPostfix < MATF_PREFIXPOSFIX)))
        return NULL;

    _ASSERT((nType >= 0) && (nType < MATF_CBDR));
    if (!((nType >= 0) && (nType < MATF_CBDR)))
        return NULL;

    _ASSERT((nSeries >= 0) && (nSeries < MATF_SERIES));
    if (!((nSeries >= 0) && (nSeries < MATF_SERIES)))
        return NULL;

    _ASSERT(((nLevel - 1) >= 0) && ((nLevel - 1) < MATF_LEVEL));    // nLevel is from 1..MATF_LEVEL
    if (!(((nLevel - 1) >= 0) && ((nLevel - 1) < MATF_LEVEL)))
        return NULL;

    return &m_CMAIT[nPrefixPostfix][nType][nSeries][nLevel - 1];
}

const KMAGICATTRIB_TABFILE* KLibOfBPT::GetMARecord(IN int i) const
{
	return m_BPTMagicAttrib.GetRecord(i);
}

const int KLibOfBPT::GetMARecordNumber() const
{
	return m_BPTMagicAttrib.NumOfEntries();
}

const KBASICPROP_EQUIPMENT* KLibOfBPT::GetMeleeWeaponRecord(IN int i) const
{
	return m_BPTMeleeWeapon.GetRecord(i);
}

const int KLibOfBPT::GetMeleeWeaponRecordNumber() const
{
	return m_BPTMeleeWeapon.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetRangeWeaponRecord(IN int i) const
{
	return m_BPTRangeWeapon.GetRecord(i);
}

const int KLibOfBPT::GetRangeWeaponRecordNumber() const
{
	return m_BPTRangeWeapon.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetArmorRecord(IN int i) const
{
	return m_BPTArmor.GetRecord(i);
}

const int KLibOfBPT::GetArmorRecordNumber() const
{
	return m_BPTArmor.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetHelmRecord(IN int i) const
{
	return m_BPTHelm.GetRecord(i);
}

const int KLibOfBPT::GetHelmRecordNumber() const
{
	return m_BPTHelm.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetBootRecord(IN int i) const
{
	return m_BPTBoot.GetRecord(i);
}

const int KLibOfBPT::GetBootRecordNumber() const
{
	return m_BPTBoot.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetBeltRecord(IN int i) const
{
	return m_BPTBelt.GetRecord(i);
}

const int KLibOfBPT::GetBeltRecordNumber() const
{
	return m_BPTBelt.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetAmuletRecord(IN int i) const
{
	return m_BPTAmulet.GetRecord(i);
}

const int KLibOfBPT::GetAmuletRecordNumber() const
{
	return m_BPTAmulet.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetRingRecord(IN int i) const
{
	return m_BPTRing.GetRecord(i);
}

const int KLibOfBPT::GetRingRecordNumber() const
{
	return m_BPTRing.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetCuffRecord(IN int i) const
{
	return m_BPTCuff.GetRecord(i);
}

const int KLibOfBPT::GetCuffRecordNumber() const
{
	return m_BPTCuff.NumOfEntries();
}

const KBASICPROP_EQUIPMENT*	KLibOfBPT::GetPendantRecord(IN int i) const
{
	return m_BPTPendant.GetRecord(i);
}

const int KLibOfBPT::GetPendantRecordNumber() const
{
	return m_BPTPendant.NumOfEntries();
}

const KBASICPROP_EQUIPMENT* KLibOfBPT::GetHorseRecord(IN int i) const
{
	return m_BPTHorse.GetRecord(i);
}

const int KLibOfBPT::GetHorseRecordNumber() const
{
	return m_BPTHorse.NumOfEntries();
}

const KBASICPROP_MEDICINE* KLibOfBPT::GetMedicineRecord(IN int i) const
{
	return m_BPTMedicine.GetRecord(i);
}

const int KLibOfBPT::GetMedicineRecordNumber() const
{
	return m_BPTMedicine.NumOfEntries();
}

const KBASICPROP_QUEST* KLibOfBPT::GetQuestRecord(IN int i) const
{
	return m_BPTQuest.GetRecord(i);
}

const int KLibOfBPT::GetQuestRecordNumber() const
{
	return m_BPTQuest.NumOfEntries();
}

const KBASICPROP_TOWNPORTAL* KLibOfBPT::GetTownPortalRecord(IN int i) const
{
	return m_BPTTownPortal.GetRecord(i);
}

const int KLibOfBPT::GetTownPortalRecordNumber() const
{
	return m_BPTTownPortal.NumOfEntries();
}
/*
const KBASICPROP_EQUIPMENT* KLibOfBPT::FindEquipment(IN int a, IN int b, IN int c) const
{
	return m_BPTEquipment.FindRecord(a, b, c);
}
*/

const KBASICPROP_EQUIPMENT_UNIQUE* KLibOfBPT::FindEquipmentUnique(IN int a, IN int b, IN int c) const
{
	return 0;// TODO:m_BPTEquipment_Unique.FindRecord(a, b, c);
}

/******************************************************************************
	功能:	获取指定的药材的属性记录
	入口:	i: 指定药材
	出口:	成功时返回指向该记录的指针
			失败时返回NULL
******************************************************************************/
const KBASICPROP_MEDMATERIAL* KLibOfBPT::GetMedMaterial(int i) const
{
	return 0;// TODO:m_BPTMedMaterial.GetRecord(i);
}

/******************************************************************************
	功能:	获取指定的药品的属性记录
	入口:	nType: 药品的类型
			nLevel: 药品的级别
	出口:	成功时返回指向该记录的指针
			失败时返回NULL
******************************************************************************/
const KBASICPROP_MEDICINE* KLibOfBPT::FindMedicine(IN int nType, IN int nLevel) const
{
	return 0;//TODO: m_BPTMedicine.FindRecord(nType, nLevel);
}

/******************************************************************************
	功能:	获取指定的矿石的属性记录
	入口:	i: 指定矿石
	出口:	成功时返回指向该记录的指针
			失败时返回NULL
******************************************************************************/
const KBASICPROP_MINE* KLibOfBPT::GetMine(IN int i) const
{
	return 0;// TODO:m_BPTMine.GetRecord(i);
}

//=============================================================================

KBasicPropertyTable::KBasicPropertyTable()
{
	m_pBuf = NULL;
	m_nNumOfEntries = 0;
	m_nSizeOfEntry = 0;
	m_szTabFile[0] = 0;
}

KBasicPropertyTable::~KBasicPropertyTable()
{
	ReleaseMemory();
}

/******************************************************************************
	功能:	记录tab file中共有多少项数据记录
******************************************************************************/
void KBasicPropertyTable::SetCount(int cbCount)
{
	_ASSERT(cbCount>0);
	_ASSERT(0==m_nNumOfEntries);	// this function is supposed to be called only once
	m_nNumOfEntries = cbCount;
}

/******************************************************************************
	功能:	分配内存,用于保存从tab file中读入的数据
	入口:	m_nNumOfEntries: 共有这么多项数据记录
			m_nSizeOfEntry: 每项数据记录的大小(字节)
	出口:	成功时返回非零, m_pBuf 指向分配的内存
			失败时返回零
******************************************************************************/
BOOL KBasicPropertyTable::GetMemory()
{
	_ASSERT(NULL == m_pBuf);
	_ASSERT(m_nNumOfEntries > 0 && m_nSizeOfEntry > 0);

	BOOL bEC = FALSE;
	const int nMemSize = m_nSizeOfEntry * m_nNumOfEntries;
	void* pBuf = new BYTE[nMemSize];
	_ASSERT(pBuf != NULL);
	if (pBuf != NULL)
	{
		m_pBuf = pBuf;
		bEC = TRUE;
	}
	return bEC;
}

/******************************************************************************
	功能:	释放内存
******************************************************************************/
void KBasicPropertyTable::ReleaseMemory()
{
	if (m_pBuf)
	{
		delete []m_pBuf;
		m_pBuf = NULL;
		m_nNumOfEntries = 0;
	}
}

/******************************************************************************
	功能:	读入tab file中的全部数据
	入口:	m_szTabFile: 文件名
	出口:	成功时返回非零, 全部数据读入m_pBuf所指缓冲区中.
				m_nNumOfEntries 给出共读入多少项数据
			失败时返回零
******************************************************************************/
BOOL KBasicPropertyTable::Load()
{
	BOOL bEC = FALSE;
	KTabFile	theLoader;

	// 加载tab file
	g_SetRootPath(NULL);

//	::g_SetFilePath(TABFILE_PATH);
	char	szFileName[FILE_NAME_LENGTH];
	g_UnitePathAndName(TABFILE_PATH, m_szTabFile, szFileName);
	if (FALSE == theLoader.Load(szFileName))
		{ _ASSERT(FALSE); return bEC; }

	// 确定file内给出了多少项记录
	const int cbItems = theLoader.GetHeight() - 1;	// 第一行给出各列名称,
	if (cbItems <= 0)							// 实际数据从第2行开始给出.
		{ _ASSERT(FALSE); return bEC; }
	SetCount(cbItems);

	// 分配内存，构建属性表
	if (FALSE == GetMemory())
		{ _ASSERT(FALSE); return bEC; }

	// 将属性记录逐条读入
	int i;
	for (i = 0; i < cbItems; i++)
	{
		if (FALSE == LoadRecord(i, &theLoader))
			{ _ASSERT(FALSE); return bEC; }
	}

	bEC = TRUE;
	return bEC;
}

//============================================================================

KBPT_Mine::KBPT_Mine()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_MINE);
	::strcpy(m_szTabFile, TABFILE_MINE);
}

KBPT_Mine::~KBPT_Mine()
{
}

BOOL KBPT_Mine::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KBASICPROP_MINE* pBuf = (KBASICPROP_MINE*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
/*ver1****************************************************************
		{ PI_VARTYPE_CHAR,			pBuf->m_szSerialNum, sizeof(pBuf->m_szSerialNum)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nStyle), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nClass), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImage1, sizeof(pBuf->m_szImage1)},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImage2, sizeof(pBuf->m_szImage2)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_bRepeatable), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSort), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nValue), 0},
*********************************************************************/
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nParticularType), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSeries), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_bStack), 0},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

/******************************************************************************
	功能:	获取指定的矿石的属性
	入口:	i: 要求获取第i项矿石的属性. 若为-1表示随机取一个
	出口:	成功时返回指向该矿石属性的指针(一个KBASICPROP_MINE结构)
			失败时返回NULL
******************************************************************************/
const KBASICPROP_MINE* KBPT_Mine::GetRecord(int i) const
{
	_ASSERT(this != NULL);

	if (-1 == i)
		i = ::GetRandomNumber(0, m_nNumOfEntries-1);

	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_MINE*)m_pBuf) + i) : NULL;
}

//============================================================================
KBPT_TownPortal::KBPT_TownPortal()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_TOWNPORTAL);
	::strcpy(m_szTabFile, TABFILE_TOWNPORTAL);
}

KBPT_TownPortal::~KBPT_TownPortal()
{
}

BOOL KBPT_TownPortal::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	KBASICPROP_TOWNPORTAL* pBuf = (KBASICPROP_TOWNPORTAL*)m_pBuf;
	pBuf = pBuf + i;			// 读入的属性记在 pBuf 所指结构中
	const PROPINFO aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

const KBASICPROP_TOWNPORTAL* KBPT_TownPortal::GetRecord(IN int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_TOWNPORTAL*)m_pBuf) + i) : NULL;
}

//============================================================================
KBPT_Quest::KBPT_Quest()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_QUEST);
	::strcpy(m_szTabFile, TABFILE_TASK);
}

KBPT_Quest::~KBPT_Quest()
{
}

BOOL KBPT_Quest::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	KBASICPROP_QUEST* pBuf = (KBASICPROP_QUEST*)m_pBuf;
	pBuf = pBuf + i;			// 读入的属性记在 pBuf 所指结构中
	const PROPINFO aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));

}

const KBASICPROP_QUEST* KBPT_Quest::GetRecord(IN int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_QUEST*)m_pBuf) + i) : NULL;

}

const KBASICPROP_QUEST* KBPT_Quest::FindRecord(IN int nDetailType) const
{
	const KBASICPROP_QUEST* pData = NULL;
	// 以下使用顺序查找的算法. 若原始数据是按m_nDetailType排序的,
	// 则可进行算法优化
	for (int i = 0; i < m_nNumOfEntries; i++)
	{
		const KBASICPROP_QUEST* pQst;
		pQst = GetRecord(i);
		_ASSERT(NULL != pQst);
		if (nDetailType == pQst->m_nDetailType)
		{
			// 根据策划的设计, 属性1类型 == 具体类别. 进行验证
//			_ASSERT(nType == pMed->m_nAttrib1_Type);
			pData = pQst;
			break;
		}
	}
	return pData;
}

//============================================================================

KBPT_Medicine::KBPT_Medicine()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_MEDICINE);
	::strcpy(m_szTabFile, TABFILE_MEDICINE);
}

KBPT_Medicine::~KBPT_Medicine()
{
}

BOOL KBPT_Medicine::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KBASICPROP_MEDICINE* pBuf = (KBASICPROP_MEDICINE*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nParticularType), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSeries), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_bStack), 0},

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryAttrib[0].nAttrib), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryAttrib[0].nValue), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryAttrib[0].nTime), 0},

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryAttrib[1].nAttrib), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryAttrib[1].nValue), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryAttrib[1].nTime), 0},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

/******************************************************************************
	功能:	获取指定的药品的属性
	入口:	i: 要求获取第i项药品的属性
	出口:	成功时返回指向该药材属性的指针(一个KBASICPROP_MEDICINE结构)
			失败时返回NULL
******************************************************************************/
const KBASICPROP_MEDICINE* KBPT_Medicine::GetRecord(int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_MEDICINE*)m_pBuf) + i) : NULL;
}

const KBASICPROP_MEDICINE* KBPT_Medicine::FindRecord(IN int nType,
													 IN int nLevel) const
{
	const KBASICPROP_MEDICINE* pData = NULL;
	// 以下使用顺序查找的算法. 若原始数据是按m_nDetailType及m_nLevel排序的,
	// 则可进行算法优化
	for (int i = 0; i < m_nNumOfEntries; i++)
	{
		const KBASICPROP_MEDICINE* pMed;
		pMed = GetRecord(i);
		_ASSERT(NULL != pMed);
		if (nType == pMed->m_nDetailType && nLevel == pMed->m_nLevel)
		{
			// 根据策划的设计, 属性1类型 == 具体类别. 进行验证
//			_ASSERT(nType == pMed->m_nAttrib1_Type);
			pData = pMed;
			break;
		}
	}
	return pData;
}

//============================================================================

KBPT_MedMaterial::KBPT_MedMaterial()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_MEDMATERIAL);
	::strcpy(m_szTabFile, TABFILE_MEDMATERIAL);
}

KBPT_MedMaterial::~KBPT_MedMaterial()
{
}

BOOL KBPT_MedMaterial::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KBASICPROP_MEDMATERIAL* pBuf = (KBASICPROP_MEDMATERIAL*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
/*ver1****************************************************************
		{ PI_VARTYPE_CHAR,			pBuf->m_szSerialNum, sizeof(pBuf->m_szSerialNum)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nStyle), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImage1, sizeof(pBuf->m_szImage1)},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImage2, sizeof(pBuf->m_szImage2)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_bRepeatable), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSort1), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSort2), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nValue), 0},
*********************************************************************/
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nParticularType), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSeries), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_bStack), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nAttrib1_Type), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nAttrib1_Para), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nAttrib2_Type), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nAttrib2_Para), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nAttrib3_Type), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nAttrib3_Para), 0},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

/******************************************************************************
	功能:	获取指定的药材的属性
	入口:	i: 要求获取第i项药材的属性. 若为-1表示随机取一个
	出口:	成功时返回指向该药材属性的指针(一个KBASICPROP_MEDMATERIAL结构)
			失败时返回NULL
******************************************************************************/
const KBASICPROP_MEDMATERIAL* KBPT_MedMaterial::GetRecord(int i) const
{
	_ASSERT(this != NULL);

	if (-1 == i)
		i = ::GetRandomNumber(0, m_nNumOfEntries-1);

	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_MEDMATERIAL*)m_pBuf) + i) : NULL;
}

KBPT_Equipment::KBPT_Equipment()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_EQUIPMENT);
}

KBPT_Equipment::~KBPT_Equipment()
{
}

void KBPT_Equipment::Init(IN int i)
{
	::strcpy(m_szTabFile, TABFILE_EQUIPMENT[i]);
}

BOOL KBPT_Equipment::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KBASICPROP_EQUIPMENT* pBuf = (KBASICPROP_EQUIPMENT*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nParticularType), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSeries), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_bStack), 0},

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[0].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[0].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[0].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[1].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[1].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[1].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[2].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[2].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[2].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[3].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[3].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[3].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[4].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[4].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[4].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[5].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[5].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[5].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[6].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[6].sRange.nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropBasic[6].sRange.nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[0].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[0].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[1].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[1].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[2].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[2].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[3].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[3].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[4].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[4].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[5].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[5].nPara), 0},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

/******************************************************************************
	功能:	获取指定的装备的属性
	入口:	i: 要求获取第i项装备的属性
	出口:	成功时返回指向该装备属性的指针(一个KBASICPROP_EQUIPMENT结构)
			失败时返回NULL
******************************************************************************/
const KBASICPROP_EQUIPMENT* KBPT_Equipment::GetRecord(int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_EQUIPMENT*)m_pBuf) + i) : NULL;
}

/******************************************************************************
	功能:	获取指定的装备属性记录
	入口:	nDetailType: 具体类别
			nParticularType: 详细类别
			nLevel: 等级
	出口:	成功时返回指向该记录的指针
			失败时返回NULL
******************************************************************************/
const KBASICPROP_EQUIPMENT* KBPT_Equipment::FindRecord( IN int nDetailType,
														IN int nParticularType,
														IN int nLevel) const
{
	_ASSERT(this != NULL);
	
	const KBASICPROP_EQUIPMENT* pData = NULL;
	// 以下使用顺序查找的算法. 若原始数据是按m_nDetailType及m_nLevel排序的,
	// 则可进行算法优化
	for (int i = 0; i < m_nNumOfEntries; i++)
	{
		const KBASICPROP_EQUIPMENT* pEqu;
		pEqu = GetRecord(i);
		_ASSERT(NULL != pEqu);
		if (nDetailType == pEqu->m_nDetailType &&
			nParticularType == pEqu->m_nParticularType &&
			nLevel == pEqu->m_nLevel)
		{
			pData = pEqu;
			break;
		}
	}
	return pData;
}

//============================================================================
// 黄金装备的实现
// flying
KBPT_Equipment_Gold::KBPT_Equipment_Gold()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_EQUIPMENT_GOLD);
	// Copy the gold item information file's name into the buffer
	::strcpy(m_szTabFile, TABFILE_GOLDITEM);
}

KBPT_Equipment_Gold::~KBPT_Equipment_Gold()
{
	return;
}

BOOL KBPT_Equipment_Gold::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KBASICPROP_EQUIPMENT_GOLD* pBuf = (KBASICPROP_EQUIPMENT_GOLD*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nParticularType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nRarity), 0},		
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nWidth), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nHeight), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSeries), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[0].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[0].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[1].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[1].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[2].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[2].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[3].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[3].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[4].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[4].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[5].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[5].nPara), 0},

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[2].nMax), 0},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

/******************************************************************************
	功能:	获取指定的装备的属性
	入口:	i: 要求获取第i项装备的属性
	出口:	成功时返回指向该装备属性的指针(一个KBASICPROP_EQUIPMENT结构)
			失败时返回NULL
******************************************************************************/
const KBASICPROP_EQUIPMENT_GOLD* KBPT_Equipment_Gold::GetRecord(int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_EQUIPMENT_GOLD*)m_pBuf) + i) : NULL;
}

/******************************************************************************
	功能:	获取指定的唯一装备属性记录
	入口:	nDetailType: 具体类别
			nParticularType: 详细类别
			nLevel: 等级
	出口:	成功时返回指向该记录的指针
			失败时返回NULL
******************************************************************************/
const KBASICPROP_EQUIPMENT_GOLD*
		KBPT_Equipment_Gold::FindRecord(IN int nDetailType,
										  IN int nParticularType,
										  IN int nLevel) const
{
	_ASSERT(this != NULL);
	
	const KBASICPROP_EQUIPMENT_GOLD* pData = NULL;
	for (int i = 0; i < m_nNumOfEntries; i++)
	{
		const KBASICPROP_EQUIPMENT_GOLD* pEqu;
		pEqu = GetRecord(i);
		_ASSERT(NULL != pEqu);
		if (NULL == pEqu)
			continue;
		if (nDetailType == pEqu->m_nDetailType &&
			nParticularType == pEqu->m_nParticularType &&
			nLevel == pEqu->m_nLevel)
		{
			pData = pEqu;
			break;
		}
	}
	return pData;
}
//============================================================================
KBPT_Equipment_Unique::KBPT_Equipment_Unique()
{
	m_nSizeOfEntry = sizeof(KBASICPROP_EQUIPMENT_UNIQUE);
	::strcpy(m_szTabFile, TABFILE_EQUIPMENT_UNIQUE);
}

KBPT_Equipment_Unique::~KBPT_Equipment_Unique()
{
}

BOOL KBPT_Equipment_Unique::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KBASICPROP_EQUIPMENT_UNIQUE* pBuf = (KBASICPROP_EQUIPMENT_UNIQUE*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nItemGenre), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nDetailType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nParticularType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szImageName, sizeof(pBuf->m_szImageName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nObjIdx), 0},
		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nSeries), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPrice), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nRarity), 0},
// TODO: 以上各项的顺序可能还会调整.

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[0].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[0].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[1].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[1].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[2].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[2].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[3].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[3].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[4].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[4].nPara), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[5].nType), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryPropReq[5].nPara), 0},

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[0].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[1].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[2].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[3].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[4].aryRange[2].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_aryMagicAttribs[5].aryRange[2].nMax), 0},
	};

	// 逐个读入各项属性
	return ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0]));
}

/******************************************************************************
	功能:	获取指定的装备的属性
	入口:	i: 要求获取第i项装备的属性
	出口:	成功时返回指向该装备属性的指针(一个KBASICPROP_EQUIPMENT结构)
			失败时返回NULL
******************************************************************************/
const KBASICPROP_EQUIPMENT_UNIQUE* KBPT_Equipment_Unique::GetRecord(int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KBASICPROP_EQUIPMENT_UNIQUE*)m_pBuf) + i) : NULL;
}

/******************************************************************************
	功能:	获取指定的唯一装备属性记录
	入口:	nDetailType: 具体类别
			nParticularType: 详细类别
			nLevel: 等级
	出口:	成功时返回指向该记录的指针
			失败时返回NULL
******************************************************************************/
const KBASICPROP_EQUIPMENT_UNIQUE*
		KBPT_Equipment_Unique::FindRecord(IN int nDetailType,
										  IN int nParticularType,
										  IN int nLevel) const
{
	_ASSERT(this != NULL);
	
	const KBASICPROP_EQUIPMENT_UNIQUE* pData = NULL;
	// 以下使用顺序查找的算法. 若原始数据是按m_nDetailType及m_nLevel排序的,
	// 则可进行算法优化
	for (int i = 0; i < m_nNumOfEntries; i++)
	{
		const KBASICPROP_EQUIPMENT_UNIQUE* pEqu;
		pEqu = GetRecord(i);
		_ASSERT(NULL != pEqu);
		if (NULL == pEqu)
			continue;
		if (nDetailType == pEqu->m_nDetailType &&
			nParticularType == pEqu->m_nParticularType &&
			nLevel == pEqu->m_nLevel)
		{
			pData = pEqu;
			break;
		}
	}
	return pData;
}

//============================================================================

KBPT_MagicAttrib_TF::KBPT_MagicAttrib_TF()
{
	m_nSizeOfEntry = sizeof(KMAGICATTRIB_TABFILE);
	::strcpy(m_szTabFile, TABFILE_MAGICATTRIB);
	::memset(&m_naryMACount, 0, sizeof(m_naryMACount));
}

KBPT_MagicAttrib_TF::~KBPT_MagicAttrib_TF()
{
}

BOOL KBPT_MagicAttrib_TF::LoadRecord(int i, KTabFile* pTF)
{
	_ASSERT(pTF != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);

	// 初始化
	KMAGICATTRIB_TABFILE* pBuf = (KMAGICATTRIB_TABFILE*)m_pBuf;
	pBuf = pBuf + i;	// 读入的属性记在 pBuf 所指结构中
	const PROPINFO	aryPI[] =
	{
		{ PI_VARTYPE_CHAR,			pBuf->m_szName, sizeof(pBuf->m_szName)},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nPos), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nClass), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_nLevel), 0},

		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.nPropKind), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.aryRange[0].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.aryRange[0].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.aryRange[1].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.aryRange[1].nMax), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.aryRange[2].nMin), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_MagicAttrib.aryRange[2].nMax), 0},

		{ PI_VARTYPE_CHAR,			pBuf->m_szIntro, sizeof(pBuf->m_szIntro)},
		
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[0]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[1]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[2]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[3]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[4]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[5]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[6]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[7]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[8]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[9]), 0},
		{ PI_VARTYPE_INT,  (char*)&(pBuf->m_DropRate[10]), 0}
	};
	
	// 逐个读入各项属性
	if (FALSE == ::LoadRecord(pTF, i, aryPI, sizeof(aryPI)/ sizeof(aryPI[0])))
		return FALSE;

    pBuf->m_nUseFlag = false;       // 初始化为没有使用

	// 数据统计：每种装备可适用的魔法数目
	for (int n = 0; n < MATF_CBDR; n++)
	{
		if (0 != pBuf->m_DropRate[n])
		{
			int nPos;
			nPos = (0 == pBuf->m_nPos) ? 1 : 0;
			m_naryMACount[nPos][n]++;
		}
	}
	return TRUE;
}

/******************************************************************************
	功能:	获取指定的魔法属性
	入口:	i: 要求获取第i项魔法属性
	出口:	成功时返回指向该魔法属性的指针(一个KMAGICATTRIB_TABFILE结构)
			失败时返回NULL
******************************************************************************/
const KMAGICATTRIB_TABFILE* KBPT_MagicAttrib_TF::GetRecord(int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfEntries);
	return (i >= 0 && i < m_nNumOfEntries) ?
		(((KMAGICATTRIB_TABFILE*)m_pBuf) + i) : NULL;
}

void KBPT_MagicAttrib_TF::GetMACount(int* pnCount) const
{
	_ASSERT(this != NULL);
	_ASSERT(pnCount != NULL);
	::memcpy(pnCount, m_naryMACount, sizeof(m_naryMACount));
}

//============================================================================
// Magic Item Index Table Class
KBPT_ClassMAIT::KBPT_ClassMAIT()
{
	m_pnTable = NULL;
	m_nSize = 0;
	m_nNumOfValidData = 0;
}

KBPT_ClassMAIT::~KBPT_ClassMAIT()
{
    m_nSize = 0;
	m_nNumOfValidData = 0;

	if (m_pnTable)
    {
        delete []m_pnTable;
        m_pnTable = NULL;
    }
}

/******************************************************************************
	功能:	将索引表清空，并不释放内存
	入口:	
	出口:	成功返回非零，失败返回零
	说明:	
******************************************************************************/
BOOL KBPT_ClassMAIT::Clear()
{
    m_nNumOfValidData = 0; 
    
    return true;   
}


/******************************************************************************
	功能:	从索引表中取值
	入口:	i: 从索引表中该位置取值
	出口:	返回所取的值
	说明:	若返回值为nItemIndex, 则调用KLibOfBPT::m_BPTMagicAttrib.GetRecord(nItemIndex)
            可获得对应的魔法属性
******************************************************************************/
int KBPT_ClassMAIT::Get(int i) const
{
	_ASSERT(this != NULL);
	_ASSERT(i >= 0 && i < m_nNumOfValidData);
    _ASSERT(m_pnTable);
	return m_pnTable[i];
}


/******************************************************************************
	功能:	给索引表插入一个新的值
	入口:	nItemIndex: 欲赋之值
	出口:	成功时返回非零, m_pnTable[m_nNumOfValidData]为所赋之值
			m_nNumOfValidData指向下一空位
			失败时返回零
******************************************************************************/
BOOL KBPT_ClassMAIT::Insert(int nItemIndex)
{
    int nResult = false;

	_ASSERT(this != NULL);
	_ASSERT(nItemIndex >= 0);	// nData 为数组下标

    if (!m_pnTable)
    {
        m_pnTable = new int [4];
        if (!m_pnTable)
            goto Exit0;

        m_nNumOfValidData = 0;
        m_nSize = 4;
    }

    if (m_nNumOfValidData >= m_nSize)
    {
        int *pnaryTemp = new int [m_nSize + 8];
        if (!pnaryTemp)
            goto Exit0;

        memcpy(pnaryTemp, m_pnTable, m_nNumOfValidData * sizeof(int));

        m_nSize += 8;
        delete []m_pnTable;
        m_pnTable = pnaryTemp;
        pnaryTemp = NULL;
    }

	m_pnTable[m_nNumOfValidData++] = nItemIndex;

    nResult = true;
Exit0:
    return nResult;
}



//============================================================================

KBPT_ClassifiedMAT::KBPT_ClassifiedMAT()
{
	m_pnTable = NULL;
	m_nSize = 0;
	m_nNumOfValidData = 0;
}

KBPT_ClassifiedMAT::~KBPT_ClassifiedMAT()
{
	ReleaseMemory();
}

/******************************************************************************
	功能:	为索引表分配内存
	入口:	nCount: 索引表大小(数据项的数目)
	出口:	m_pnTable 指向分配的内存
			m_nSize = nCount
******************************************************************************/
BOOL KBPT_ClassifiedMAT::GetMemory(int nCount)
{
	_ASSERT(this != NULL);
	_ASSERT(nCount > 0);
	_ASSERT(NULL == m_pnTable);
	_ASSERT(0 == m_nSize);

	BOOL bEC = FALSE;
	int* pnBuf = new int[nCount];
	if (pnBuf)
	{
		m_pnTable = pnBuf;
		m_nSize = nCount;
		bEC = TRUE;
	}
	return bEC;
}

/******************************************************************************
	功能:	释放索引表所用的内存
******************************************************************************/
void KBPT_ClassifiedMAT::ReleaseMemory()
{
	if (m_pnTable)
	{
		delete []m_pnTable;
		m_pnTable = NULL;
		m_nSize = 0;
	}
}

/******************************************************************************
	功能:	给索引表赋值
	入口:	nData: 欲赋之值
	出口:	成功时返回非零, m_pnTable[m_nNumOfValidData]为所赋之值
			m_nNumOfValidData指向下一空位
			失败时返回零
******************************************************************************/
BOOL KBPT_ClassifiedMAT::Set(int nData)
{
	_ASSERT(this != NULL);
	_ASSERT(nData >= 0);	// nData 为数组下标
	_ASSERT(m_nNumOfValidData >= 0 && m_nNumOfValidData < m_nSize);
	m_pnTable[m_nNumOfValidData++] = nData;
	return TRUE;
}

/******************************************************************************
	功能:	从索引表中取值
	入口:	nIndex: 从索引表中该位置取值
	出口:	返回所取的值
	说明:	若返回值为i, 则调用KLibOfBPT::m_BPTMagicAttrib.GetRecord(i)可获得
			对应的魔法属性
******************************************************************************/
int KBPT_ClassifiedMAT::Get(int nIndex) const
{
	_ASSERT(this != NULL);
	_ASSERT(nIndex >= 0 && nIndex < m_nSize);
	return (nIndex >= 0 && nIndex < m_nSize) ? m_pnTable[nIndex] : 0;
}

/******************************************************************************
	功能:	将数据导出到给定的缓冲区中
	入口:	pnaryBuf: 级冲区指针
			*pnCount: 缓冲区可容纳多少个int型数据
	出口:	成功时返回非零, 数据被导出到缓冲区中, *pnCount 给出数据个数
			失败时返回零
******************************************************************************/
BOOL KBPT_ClassifiedMAT::GetAll(int* pnaryBuf, int* pnCount) const
{
	_ASSERT(this != NULL);
	_ASSERT(pnaryBuf != NULL);
	_ASSERT(pnCount != NULL);

	BOOL bEC = FALSE;
	if (pnaryBuf && pnCount && *pnCount > 0)
	{
		int nLength = *pnCount;
		if(m_nSize< nLength) nLength = m_nSize;
		::memcpy(pnaryBuf, m_pnTable, nLength*sizeof(int));
		*pnCount = nLength;
		bEC = TRUE;
	}
	return bEC;
}

