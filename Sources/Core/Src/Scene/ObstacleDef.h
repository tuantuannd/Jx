#ifndef KObstacleH
#define	KObstacleH

enum Obstacle_Type			// 物件地面格子障碍类型
{
	Obstacle_Empty = 0,		// 空，无障碍
	Obstacle_Full,			// 完整障碍
	Obstacle_LT,			// 左上区域为障碍
	Obstacle_RT,			// 右上区域为障碍
	Obstacle_LB,			// 左下区域为障碍
	Obstacle_RB,			// 右下区域为障碍
	Obstacle_Type_Num,		// 障碍类型数
};

enum Obstacle_Kind
{
	Obstacle_NULL = 0,		// 没有障碍
	Obstacle_Normal,		// 一般障碍
	Obstacle_Fly,			// 障碍但是可以飞过
	Obstacle_Jump,			// 障碍但是可以跳过
	Obstacle_JumpFly,		// 障碍但是可以跳过并且飞过
	Obstacle_Kind_Num,		// 障碍种类数
};
#endif
