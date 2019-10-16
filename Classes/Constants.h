#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <cmath>

const float PI = 3.14159f;
//#define GOUGU(x,y) (sqrt((x)*(x) + (y)*(y))) //勾股定理，知道两边长度求第三边

//勾股定理，知道两边长度求第三边
template <typename _Type>
inline _Type GOUGU(_Type x, _Type y) {
	return sqrt(pow(x, 2) + pow(y, 2));
}

//返回给定范围随机数
template <typename _Type>
inline _Type RANDOM(_Type min, _Type max) {
	return (std::rand() % 100 + 1) / 100.0 * (max-min) + min;
}

//groups
const int BombGroup		   = 1001; //炮弹碰撞体组
const int BombBlockGroup   = 2001; //炮弹障碍物组
const int HeroGroup	       = 3001; //玩家组
const int StaticBlockGroup = 4001; //静态障碍物组
const int EnemyBombGroup   = 5001; //敌人炮弹碰撞体组
const int EnemyGroup	   = 6001; //敌人组
const int EnemyAttackGroup = 7001; //敌人受击检测组

#endif // !CONSTANTS_H
