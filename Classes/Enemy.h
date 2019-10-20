#pragma once
#ifndef _ENEMY_H_
#define _ENEMY_H_
#include "cocos2d.h"
#include "GameScene.h"
#include "Biosome.h"
#include "Hero.h"

class GameScene;

using cocos2d::Rect;
using cocos2d::Vec2;
using cocos2d::Size;

//敌人类
class Enemy : public Biosome, public std::enable_shared_from_this<Enemy> {
public:
	Enemy() = default;
	Enemy(cocos2d::Scene *scene, Vec2 pos) : Enemy(scene, pos, 0.5f) {}
	Enemy(cocos2d::Scene *scene, Vec2 pos, float scale)
		: Biosome(pos), 
		scale_(scale),
		scene_(scene){ init(); }
	~Enemy();

	static std::shared_ptr<Enemy> create();
	static std::shared_ptr<Enemy> create(cocos2d::Scene *scene, Vec2 pos);
	static std::shared_ptr<Enemy> create(cocos2d::Scene *scene, Vec2 pos, float scale);
	static std::shared_ptr<Enemy> getEnemyByTag(int tag); //通过tag获得enemy
	static void initCreateEnemyTag(int tag) { enemyTag_ = tag; } //用于初始化enemyTag_
	static int getEnemyNum(); //返回enemy数量

	bool init();//初始化函数
	void follow(std::shared_ptr<Hero> hero); //跟随
	void attack1(std::shared_ptr<Hero> hero); //攻击方式1
	void attack2(std::shared_ptr<Hero> hero); //攻击方式2
	void attack3(std::shared_ptr<Hero> hero); //攻击方式3
	const bool isFollow() { return bFollow_; } //是否跟随
	const float getHateRadius() { return hateRadius_; } //返回仇恨半径
	void setScale(float scale) { scale_ = scale; } //设置缩放
	const std::shared_ptr<Hero> getCurFollow() { return curFollow_; } //返回当前跟随hero
	virtual void beHurt(float val) override;

	static std::unordered_map<int, std::shared_ptr<Enemy>> mapTagToEnemy; //tag到enemy的映射
private:
	static int enemyTag_; //用于创建enemy的tag
	static cocos2d::Animate *moveAnimate_; //移动动画
	static cocos2d::Animate *attackAnimate_; //攻击动画
	static cocos2d::Animate *attackAnimateJump_; //攻击jump
	static cocos2d::Animate *attackAnimateFall_; //攻击fall
	static int attackAnimateDelay_; //攻击动画延迟
	static int attackAnimateJumpDelay_; //jump动画延迟
	static int attackAnimateFallDelay_; //fall动画延迟
	cocos2d::DrawNode *drawNode_; //绘制图形
	cocos2d::Scene *scene_; //游戏场景
	bool bFollow_; //正在跟着目标
	bool bRun_; //正在run中
	float maxMoveDistance_ = 300; //最大移动距离
	float attackDistance_ = 30; //可攻击距离
	float attackAttachDistance_ = 10; //攻击附加距离
	float hateRadius_ = 200; //仇恨半径
	float scale_; //缩放比例
	Vec2 originSpeed_; //初始移动速度
	std::shared_ptr<std::thread> threadFollow_ = nullptr;
	std::shared_ptr<Hero> curFollow_; //当前跟随的hero
};

#endif // !_ENEMY_H_
