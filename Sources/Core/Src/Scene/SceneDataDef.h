// ***************************************************************************************
// 场景模块的一些数据定义
// Copyright : Kingsoft 2002
// Author    : wooy(wu yue)
// CreateTime: 2002-11-11
// ---------------------------------------------------------------------------------------
// ***************************************************************************************

#ifndef SCENEDATADEF_H_HEADER_INCLUDED_C225572F
#define SCENEDATADEF_H_HEADER_INCLUDED_C225572F

#define	REGION_COMBIN_FILE_NAME_SERVER "Region_S.dat"		//场景地图区域数据的服务器端版合并文件的文件名
#define	REGION_COMBIN_FILE_NAME_CLIENT "Region_C.dat"		//场景地图区域数据的客户端版合并文件的文件名

//场景地图数据文件，每个区域组织成若干个存储不同数据的文件，这些文件又以区域为单位，分别合并成一个集合文件
//集合文件的格式如下：
//1.先存储一个四字节的unsigned int数据，表示此合成文件中共包含几个子文件
//2.存储子文件大小，以及开始开始偏移位置的信息结构的列表。有几个子文件就连续存有几个KCombinFileSection
//					各子文件的偏移位置是从信息结构的列表之后的位置开始算起.
struct KCombinFileSection
{
	unsigned int	uOffset;	//单位：字节
	unsigned int	uLength;	//单位：字节
};
//3.子文件的存储次序，每个子文件对应于什么，由SCENE_FILE_INDEX里枚举给出解释。每个枚举值表示子文件在合并文件中的次序索引。
enum SCENE_FILE_INDEX
{
	REGION_OBSTACLE_FILE_INDEX	= 0,//"OBSTACLE.DAT"障碍文件
	REGION_TRAP_FILE_INDEX,			//"Trap.dat"
	REGION_NPC_FILE_INDEX,			//"Npc_S.dat" or "Npc_C.dat"
	REGION_OBJ_FILE_INDEX,			//"Obj_S.dat" or "Obj_C.dat"
	REGION_GROUND_LAYER_FILE_INDEX,	//"Ground.dat"
	REGION_BUILDIN_OBJ_FILE_INDEX,	//"BuildinObj.Dat"

	REGION_ELEM_FILE_COUNT
};
//4.存储子文件的数据。


#define REGION_GROUND_LAYER_FILE	"Ground.dat"			//地表层信息二进制存储文件
#define	REGION_BUILDIN_OBJ_FILE		"BuildinObj.Dat"		//场景地图内建对象集合信息文件
#define	REGION_OBSTACLE_FILE		"OBSTACLE.DAT"			//障碍文件
#define REGION_NPC_FILE_SERVER		"Npc_S.dat"
#define REGION_NPC_FILE_CLIENT		"Npc_C.dat"
#define REGION_OBJ_FILE_SERVER		"Obj_S.dat"
#define REGION_OBJ_FILE_CLIENT		"Obj_C.dat"
#define REGION_TRAP_FILE			"Trap.dat"				//事件点文件

#define	MAX_RESOURCE_FILE_NAME_LEN	128
#define NOT_ABOVE_HEAD_OBJ			0xFFFF	//并非高空对象


enum IPOT_RENDER_LAYER
{
	IPOT_RL_COVER_GROUND	= 0x01,		//平铺在地面上，在全部物体之下
	IPOT_RL_OBJECT			= 0x02,		//当前物体层
	IPOT_RL_INFRONTOF_ALL	= 0x04,		//在一切的上层
	IPOT_RL_LIGHT_PROP		= 0x08,		//此对象带有光源属性
};

//## 三维坐标
struct KTriDimensionCoord
{
	int x, y, z;
};

//■■■Ground.Dat地表层信息二进制存储文件■■■
//按次序存储如下三部分内容：
//1.文件头结构
struct KGroundFileHead
{
	unsigned int uNumGrunode;	//地表格图形的数目
	unsigned int uNumObject;	//紧贴地面的类似路面之类的对象的数目
	unsigned int uObjectDataOffset;
};
//2.存储KGroundFileHead::nNumGrunode个以KSPRCrunode结构表述的地表格图形
struct KSPRCrunode//## 地表格图形信息。
{
	struct KSPRCrunodeParam
	{
		unsigned short h, v;		//此图形所在的（区域内）地表格坐标
		unsigned short nFrame;		//引用图形的第几帧
		unsigned short nFileNameLen;//图形文件名长度（包括结束符），取值不可大于MAX_RESOURCE_FILE_NAME_LEN
	}	Param;

	//## 图形文件名
	char szImgName[1];	//此结构为变长，实际长度为此结构中的nFileNameLen数值
};
//3.存储KGroundFileHead::nNumObject个以KSPRCoverGroundObj结构表述的紧贴地面对象
//要求按照绘制的先后顺序排列好
#pragma pack(push, 2)
struct KSPRCoverGroundObj//## 表述紧贴地面的类似路面之类的对象结构。
{
	//## 对象定位点在场景地图的坐标。单位（场景点坐标）
	int nPositionX;			//横向坐标
	int nPositionY;			//纵向坐标
	
	char szImage[MAX_RESOURCE_FILE_NAME_LEN];	//引用的图形的文件名。
	
	unsigned short	nWidth;	//图形横宽
	unsigned short	nHeight;//图形纵宽

	//## 引用图形的第几帧
	unsigned short nFrame;

	//保留，为定值0
	union
	{
		unsigned char	bReserved;
		unsigned char	bRelateRegion;
	};

	//## 绘制次序（主）
	unsigned char	bOrder;

	//## 绘制次序（次）
	short	nLayer;

};
#pragma pack(pop)
//■■■Ground.Dat地表层信息二进制存储文件--存储内容结束---■■■


//■■■BuildinObj.Dat场景内建对象二进制存储文件■■■
//按次序存储如下三部分内容：
//1.文件头结构
struct KBuildinObjFileHead
{
	//## 区域中内建对象的数目
	unsigned int nNumBios;		//nNumBios = nNumBiosTree + nNumBiosLine + nNumBiosPoint + nNumBiosAbove
	//## 以树方式排序的内建对象的数目
	unsigned short nNumBiosTree;
	//## 以底边方式排序的内建对象的数目
	unsigned short nNumBiosLine;
	//## 以点方式排序的内建对象的数目
	unsigned short nNumBiosPoint;
	//## 高空内建对象的数目
	unsigned short nNumBiosAbove;

	//## 高空对象最大的绘图次序编号。
	unsigned short nMaxAboveHeadObjOrder;
	unsigned short nNumBulidinLight;
};
//2.KBuildinObjFileHead::nNumBios个以KBuildinObj结构表述的场景内建对象
//存储的顺序为:存储以点方式排序的全部对象;
//再存储以底边方式排序的全部对象;
//再存储按按树方式排序的全部对象;
//再存储全部的高空对象
struct KBuildinObj//## 表述场景地图上的内建对象的结构
{
	//## 图形的属性
	unsigned int Props;//其值为枚举SPBIO_PROPS取值的组合

	//====图形相关====
	//四个图形点按逆时针方向摆列
	KTriDimensionCoord ImgPos1;	//图形第一个点对应坐标点
	KTriDimensionCoord ImgPos2;	
	KTriDimensionCoord ImgPos3;
	KTriDimensionCoord ImgPos4; //图形第四个点对应坐标点

	short	nImgWidth;		//图形贴图的宽(单位：像素点)
	short	nImgHeight;		//图形贴图的高(单位：像素点)
    
	char  szImage[MAX_RESOURCE_FILE_NAME_LEN];	//图形资源的文件名
	unsigned int   uFlipTime;	//当前帧切换出来的时间
    unsigned short nFrame;		//绘制的图形帧索引
	unsigned short nImgNumFrames;//此对象包含图形帧的数目
	unsigned short nAniSpeed;	//动画的播放速度，值零表示为单帧图

	//====高空对象的绘图顺序====
	unsigned short nOrder;

	//====标记对象位置====
	KTriDimensionCoord oPos1;
	KTriDimensionCoord oPos2;

	float fAngleXY;
	float fNodicalY;
};
//■■■BuildinObj.Dat场景内建对象二进制存储文件--存储内容结束---■■■


//## spbio对象的属性定义
enum SPBIO_PROPS_LIST
{ 
	//##Documentation
	//## 平行于地表，不作受光切分。
	SPBIO_P_PLANETYPE_H = 0x00, 
	//##Documentation
	//## 平行于地表，且作受光切分。
	SPBIO_P_PLANETYPE_H_D = 0x01, 
	//##Documentation
	//## 垂直于地表，不作受光切分。
	SPBIO_P_PLANETYPE_V = 0x02, 
	//##Documentation
	//## 垂直于地表，且作受光切分。
	SPBIO_P_PLANETYPE_V_D = 0x03, 
	//##Documentation
	//## 图面类型定义值mask
	SPBIO_P_PLANETYPE_MASK = 0x03, 
	//##Documentation
	//## 不受光
	SPBIO_P_LIT_NONE = 0x00, 
	//##Documentation
	//## 按照切分受光
	SPBIO_P_LIT_DIV = 0x04, 
	//##Documentation
	//## 按照重心受光
	SPBIO_P_LIT_CENTRE = 0x08, 
	//##Documentation
	//## 平行受光
	SPBIO_P_LIT_PARALLEL = 0x0C, 
	//##Documentation
	//## 光照切分方式定义的mask
	SPBIO_P_LIT_MASK = 0x0C, 
	//##Documentation
	//## 不挡光
	SPBIO_P_BLOCK_LIGHT_NONE = 0x00, 
	//##Documentation
	//## 按照底边挡光
	SPBIO_P_BLOCK_LIGHT_BOTTOM = 0x10, 
	//##Documentation
	//## 按照一个圆挡光
	SPBIO_P_BLOCK_LIGHT_CIRCLE = 0x20, 
	//##Documentation
	//## 挡光方式定义值mask
	SPBIO_P_BLOCK_LIGHT_MASK = 0x30, 
	//##Documentation
	//## 档光半径为0.2
	SPBIO_P_BLOCK_L_RADIUS_2 = 0x00, 
	//##Documentation
	//## 档光半径为0.3
	SPBIO_P_BLOCK_L_RADIUS_3 = 0x40, 
	//##Documentation
	//## 档光半径为0.4
	SPBIO_P_BLOCK_L_RADIUS_4 = 0x80, 
	//##Documentation
	//## 档光半径为0.5
	SPBIO_P_BLOCK_L_RADIUS_5 = 0xC0, 
	//##Documentation
	//## 档光半径为数值mask
	SPBIO_P_BLOCK_L_RADIUS_MASK = 0xC0, 
	//##Documentation
	//## 按照点排序
	SPBIO_P_SORTMANNER_POINT = 0x0000, 
	//##Documentation
	//## 按照边排序
	SPBIO_P_SORTMANNER_LINE = 0x0100, 
	//##Documentation
	//## 按照树排序
	SPBIO_P_SORTMANNER_TREE = 0x0200, 
	//##Documentation
	//## 排序方式的mask
	SPBIO_P_SORTMANNER_MASK = 0x0300,
	
	//##Documentation
	//## 需要要加亮显示
	SPBIO_F_HIGHT_LIGHT = 0x0400,
};


//■■■Trap.dat场景事件点信息二进制存储文件■■■
//按次序存储如下三部分内容：
//1.文件头结构
struct KTrapFileHead
{
	unsigned int uNumTrap;		//事件点的数目
	unsigned int uReserved[2];	//固定值为0
};
//2.存储KTrapFileHead::nNumTrap个以KSPTrap结构表述的地表事件点
struct KSPTrap
{
	unsigned char	cX;			//X位置
	unsigned char	cY;			//Y位置
	unsigned char	cNumCell;	//连续覆盖了几个小格子
	unsigned char	cReserved;	//保留，固定值为0
	unsigned int	uTrapId;	//根据事件关联文件名转化而得到的事件id标识
};
//■■■Trap.Dat场景事件点信息二进制存储文件--存储内容结束---■■■


//■■■Npc_S.dat, Npc_C.dat地图上的npc存储文件■■■
//按次序存储如下三部分内容：
//1.文件头结构
struct KNpcFileHead
{
	unsigned int uNumNpc;		//事件点的数目
	unsigned int uReserved[2];	//固定值为0
};
//2.存储KNpcFileHead::nNumNpc个以KSPNpc结构表述的npc
struct KSPNpc
{
	int					nTemplateID;	// 模板编号(对应模板文件"\Settings\NpcS.txt"，编号从 0 开始)
	int					nPositionX;
	int					nPositionY;
	bool				bSpecialNpc;	// 特殊npc
	char				cReserved[3];	// 保留
	char				szName[32];		// npc 名字
	short				nLevel;			// 等级
	short				nCurFrame;		// 载入时图像是第几帧
	short				nHeadImageNo;	// 头像图形编号
	short				shKind;			// npc类型
	unsigned char		cCamp;			// npc 阵营
	unsigned char		cSeries;		// npc 五行
	unsigned short		nScriptNameLen;	// Npc脚本文件名长度（包括结束符），取值不可大于MAX_RESOURCE_FILE_NAME_LEN
	char				szScript[MAX_RESOURCE_FILE_NAME_LEN];	//Npc脚本文件名//此结构实际存储时为变长，实际长度为此结构中的nScriptNameLen数值
};
//■■■Npc_S.dat, Npc_C.dat地图上的npc存储文件--存储内容结束---■■■

//■■■Obj_S.dat, Obj_C.dat地图上的Obj存储文件■■■
//按次序存储如下三部分内容：
//1.文件头结构
struct KObjFileHead
{
	unsigned int uNumObj;		//事件点的数目
	unsigned int uReserved[2];	//固定值为0
};
//2.存储KObjFileHead::nNumObj个以KSPObj结构表述的Obj
struct KSPObj
{
	int					nTemplateID;	//模板编号(对应模板文件"\Settings\ObjData.txt"，编号从1开始)
	short				nState;
	unsigned short		nBioIndex;		//当bSkipPaint为真值时，此数据有效，否则为定值0
	KTriDimensionCoord	Pos;
	char				nDir;
	bool				bSkipPaint;		//是否忽略此obj的绘制
	unsigned short		nScriptNameLen;	//Obj脚本文件名长度（包括结束符），取值不可大于MAX_RESOURCE_FILE_NAME_LEN
	char				szScript[MAX_RESOURCE_FILE_NAME_LEN];	//Obj脚本文件名//实际存储时为变长，实际长度为此结构中的nScriptNameLen数值
};
//■■■Obj_S.dat, Obj_C.dat地图上的npc存储文件--存储内容结束---■■■

// 场景内建光源属性结构
struct KBuildInLightInfo
{
	KTriDimensionCoord	oPos;			// 光源坐标
	DWORD				dwColor;		// 光源颜色和亮度
	int					nMinRange;		// 最小半径
	int					nMaxRange;		// 最大半径
	int					nCycle;			// 循环周期,毫秒
};

#pragma pack(1)
// 用于光照计算的颜色结构
struct KLColor
{
	//unsigned int r;
	//unsigned int g;
	//unsigned int b;
    //unsigned int ReserveForAlign;
	unsigned short r;
	unsigned short g;
	unsigned short b;
    unsigned short ReserveForAlign;

    // 下面的代码导致很多的性能损失
//	KLColor()
//	{
//        r = 0, g = 0, b = 0;
//	}
//	
//	KLColor(int rr, int gg, int bb)
//	{
//		r = rr, g = gg, b = bb;
//	}

	void SetColor(int rr, int gg, int bb)
	{
		r = rr, g = gg, b = bb;
	}

	void SetColor(DWORD dwColor)
	{
		r = (unsigned short)((dwColor>>16) & 0xff);
		g = (unsigned short)((dwColor>>8) & 0xff);
		b = (unsigned short)(dwColor & 0xff);
	}
	void Scale(float f)
	{
		r = (int)(r * f);
		g = (int)(g * f);
		b = (int)(b * f);
	}
	DWORD GetColor()
	{
		return 0xff000000 | (r<<16) | (g<<8) | b;
	}

	const KLColor& operator+=(KLColor& color)
	{
		r += color.r, g += color.g, b += color.b;
		return *this;
	}

	const KLColor operator+(KLColor& color)
	{
		KLColor c;
		c.r += r + color.r, c.g += g + color.g, c.b += b + color.b;
		return c;
	}
};

#pragma pack()


#endif /* SCENEDATADEF_H_HEADER_INCLUDED_C225572F */
