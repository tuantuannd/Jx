/*****************************************************************************************
//  iRepresentShell绘制的图元对象表述定义
//	Copyright : Kingsoft 2002
//	Author	:   Wooy(Wu yue)
//	CreateTime:	2002-11-11
------------------------------------------------------------------------------------------
*****************************************************************************************/

#ifndef KREPRESENTUNIT_H_HEADER_INCLUDED_C23172D7
#define KREPRESENTUNIT_H_HEADER_INCLUDED_C23172D7


//##ModelId=3DCA20D1035C
//##Documentation
//## 图元的各种类型枚举定义
enum REPRESENT_UNIT_TYPE
{
    //##Documentation
    //## 像点	KRUPoint
    RU_T_POINT, 
    //##Documentation
    //## 线段	KRULine
    RU_T_LINE,
	//## 距形边框	KRURect
	RU_T_RECT,	
    //##Documentation
    //## 图形(矩形图形，只给出两个图元坐标点)	KRUImage
    RU_T_IMAGE, 
    //##Documentation	
    //## 图形局部	KRUImagePart
    RU_T_IMAGE_PART,
	//##Documentation	KRUImage4
	//## 图形(四边形图形，给出四个图元坐标点)
	RU_T_IMAGE_4,
	//缩放地绘制图形	KRUImageStretch
	//只在单平面上绘制，且图形为ISI_T_BITMAP16 有效
	RU_T_IMAGE_STRETCH,
    //##Documentation
    //## 被划分的光照图形。
    RU_T_DIVIDED_LIT_IMAGE, 
    //##Documentation
    //## 阴影
    RU_T_SHADOW,
	//##Documentation
    //## 根据缓冲区更新图形
	RU_T_BUFFER,
};

//整数表示的二维点坐标
struct KRPosition2
{
	int nX;
	int nY;
};

//整数表示的三维点坐标
struct KRPosition3
{
	int nX;
	int nY;
	int nZ;
};

//##ModelId=3DB0C4F20145
//##Documentation
//## 绘制图元
struct KRepresentUnit
{
    KRPosition3	oPosition;
};


//##ModelId=3DD2107D01C5
//##Documentation
//## 表述颜色的结构
union KRColor
{
	struct {unsigned char b, g, r, a; } Color_b;
	unsigned int	Color_dw;
};

//##ModelId=3DB617B20036
//##Documentation
//## 像点
struct KRUPoint : public KRepresentUnit
{
    //##ModelId=3DB617D2032B
	//##Documentation
	//## 点的颜色
    KRColor	Color;
};

//##ModelId=3DB3C020008B
//##Documentation
//## 线段
struct KRULine : public KRepresentUnit
{
	KRPosition3	oEndPos;
    //##ModelId=3DB3C0B80278
	//##Documentation
	//## 线的颜色
    KRColor	Color;
};

typedef KRULine	KRURect;

//##ModelId=3DB3C020008B
//##Documentation
//## 更新图形的缓冲区
struct KRUBuffer : public KRepresentUnit
{
	KRPosition2	BufferEntireSize;	//缓冲区的大小
	KRPosition2	BufferUpdateSize;	//缓冲区的大小用来更新图形的范围大小
	KRPosition2	oImgLTPos;			//目标图形的左上角点坐标(单位:贴图像素点)
    KRColor		Color;				//颜色参数
	const unsigned char* pBuffer;	//缓冲区内存的指针
};

enum RUIMAGE_RENDER_STYLE
{
	IMAGE_RENDER_STYLE_ALPHA = 0,		//带alpha绘制
	IMAGE_RENDER_STYLE_OPACITY,			//只分全通透与完全不透明，KRUImage::Alpha值被忽略。
	IMAGE_RENDER_STYLE_3LEVEL,			//三级alpha绘制，KRUImage::Alpha值被忽略。
	IMAGE_RENDER_STYLE_BORDER,
	IMAGE_RENDER_STYLE_ALPHA_NOT_BE_LIT,//带alpha绘制但是不被光照
	IMAGE_RENDER_STYLE_ALPHA_COLOR_ADJUST,//alpha绘制，且带偏色调整
};

enum RUIMAGE_RENDER_FLAG
{
	RUIMAGE_RENDER_FLAG_REF_SPOT = 1,	//绘制坐标点是否要与图形的参考点(中心点)吻合。
			//使用此标记时,nEndX,nEndY,nEndZ的值被忽略。在与屏幕平行的面上绘制图形，并使得图形的参考点(重心点)与nX,nY,nZ描述的点相吻合。
	RUIMAGE_RENDER_FLAG_FRAME_DRAW = 2,	//帧绘制方式，不考虑绘制帧相对于整个图形的位置偏移
};

#define	IMAGE_IS_POSITION_INIT	-1

//##ModelId=3DB60D73035F
//##Documentation
//## 图形
struct KRUImage : public KRepresentUnit
{
	KRPosition3	oEndPos;

	//颜色指定，具体是否与如何引用，依赖于bRenderStyle的取值。
	//其中alpha部分的取值为0-255既256阶。
	KRColor	Color;

    //##ModelId=3DCF64DA029D
    //##Documentation
    //## 绘制方式 取值为RUIMAGE_RENDER_STYLE枚举值之一
    unsigned char bRenderStyle;

	//绘制标记,取值为RUIMAGE_RENDER_FLAG枚举值的组合。
	unsigned char bRenderFlag;
  
    //##ModelId=3DCF5BB001B7
    //##Documentation
    //## 绘制的图形的类型
    short nType;

	//##ModelId=3DCBEAD0014F
    //##Documentation
    //## 图形资源的文件名/或者图形名
    char szImage[128];

    //##ModelId=3DCBEAF50291
    //##Documentation
    //## 图形id
    unsigned int uImage;

    //##ModelId=3DCBEB170189
    //##Documentation
    //## 图形原来在iImageStore内的位置。
    short nISPosition;

    //##ModelId=3DCF5B9C0196
    //##Documentation
    //## 绘制的图形帧索引
    short nFrame;
};

//##ModelId=3DB618EC02CC
//##Documentation
//## 图形局部
struct KRUImagePart : public KRUImage
{
	KRPosition2	oImgLTPos;	//贴图左上角点坐标(单位:贴图像素点)
	KRPosition2	oImgRBPos;	//贴图右下角点坐标(单位:贴图像素点)
};

struct KRUImage4 : public KRUImagePart
{
	//(点按顺时针向排列)
	//第一个点坐标为 oPosition
	KRPosition3	oSecondPos;	//图元四边形的第二个点的坐标
	KRPosition3	oThirdPos;	//图元四边形的第三个点的坐标
	//第四个点坐标为 oEndPos
};

typedef KRUImage	KRUImageStretch;


//##ModelId=3DBFF195012B
//##Documentation
//## 被划分的光照图形。
struct KRUDividedLitImage : public KRUImage
{
	KRPosition2 oImgLTPos;	//图形被引用范围的左上角点横坐标(单位:贴图像素点)

    //##ModelId=3DBFF2A5013A
    //##Documentation
    //## 光照划分计算，划分的格子的横宽（单位：像素点）
    int nCellWidth;

    //##ModelId=3DBFF2B000C0
    //##Documentation
    //## 光照划分计算，划分的格子的纵宽（单位：像素点）
    int nCellHeight;

    //##ModelId=3DBFF2B5032B
    //##Documentation
    //## 水平方向划分的光照计算格子的数目
    int nNumCellH;

    //##ModelId=3DBFF2D800B1
    //##Documentation
    //## 垂直方向划分的光照计算格子的数目
    int nNumCellV;

    //##ModelId=3DBFF2DD03A9
    //##Documentation
    //## 存储光照计算划分的格子各个节点的光照强度的数组。每个节点的光照强度表述为一个256阶的整数值，存储空间为一个字节。
    unsigned char* pLightIntensityData;

};


//##ModelId=3DB61A8C0298
//##Documentation
//## 阴影
struct KRUShadow : public KRepresentUnit
{
    KRPosition3	oEndPos;

    //##ModelId=3DB61ADA0024
	//##Documentation
	//## 阴影的颜色
    KRColor	Color;

};


//##ModelId=3DC0F495038B
//##Documentation
//## 图形类型定义
enum KIS_IMAGE_TYPE
{
    //##Documentation
    //## 16位单帧位图。
    //## ISI_T_BITMAP16的格式为 D3DFMT_R5G6B5 或者 D3DFMT_X1R5G5B5
    //## ，具体为哪种格式由iImageStore内部确定。可以通过方法iImageStore::IsBitmapFormat565来知道是用哪种格式。
    ISI_T_BITMAP16, 
    //##Documentation
    //##  spr格式的带alpha压缩图形，若干帧
    ISI_T_SPR,
};


//##ModelId=3DB511F30242
//##Documentation
//## 图形的信息。
struct KImageParam
{
    //##ModelId=3DB512190144
    //##Documentation
    //## 图形的总的帧数目
    short nNumFrames;

    //##ModelId=3DB51226012F
    //##Documentation
    //## 图形的帧间隔。
    short nInterval;

    //##ModelId=3DB5123B0158
    //##Documentation
    //## 图形横宽（单位：像素点）。
    short nWidth;

    //##ModelId=3DB5123E036F
    //##Documentation
    //## 图形纵宽（单位：像素点）
    short nHeight;

    //##ModelId=3DB512900118
    //##Documentation
    //## 图形参考点（重心）的横坐标值。
    short nReferenceSpotX;

    //##ModelId=3DB512A70306
    //##Documentation
    //## 图形参考点（重心）的纵坐标值。
    short nReferenceSpotY;

    //##ModelId=3DB512CD0280
    //##Documentation
    //## 图形的帧分组数目（通常应用为图形方向数目）。
    short nNumFramesGroup;

};


#endif /* KREPRESENTUNIT_H_HEADER_INCLUDED_C23172D7 */
