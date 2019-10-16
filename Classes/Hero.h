#ifndef HERO_H
#define HERO_H
#include "Biosome.h"

class Hero : public Biosome{
public:
	struct DisableDir { bool L, R, U, D; };

	struct Bomb
	{
		cocos2d::Sprite *sprite;
		constexpr static int power = 1500; //施加到炮弹的力
	};

	enum class Dir { L, R, U, D };
public:
	Hero(cocos2d::Scene *scene, cocos2d::Vec2 pos) 
		: Biosome(pos), 
		scene_(scene) { init(); }
	Hero(cocos2d::Scene *scene)
		: Hero(scene, cocos2d::Vec2::ZERO){}
	bool init();

	static std::shared_ptr<Hero> create(cocos2d::Scene *scene);
	static std::shared_ptr<Hero> create(cocos2d::Scene *scene, cocos2d::Vec2 pos);
	static std::shared_ptr<Hero> getHeroByTag(int tag); //通过tag获取hero

	void makeBomb(); //创建炮弹
	void drawAimLine(cocos2d::Vec2 cursorPosition); //绘制瞄准线
	DisableDir& getDisableDir() { return disableDir_; } //返回方向失效状况
	Dir getDir() { return dir_; } //返回当前方向
	void setDir(Dir dir) {  dir_ = dir; } //设置当前方向
	const float getRate() { return rate_; } //获取速率
	const float getAngle() { return angle_; } //获取角度
	void setAngle(float angle) { angle_ = angle; } //设置角度
	cocos2d::PhysicsBody *getPhyBody() { return phyBody_; }
	cocos2d::Vec2 getBodyPosition() { return phyBody_->getPosition(); }; //获取人物碰撞体W位置
	

	static std::unordered_map<int, std::shared_ptr<Hero>> mapTagToHero; //tag到hero的映射
private:
	static int heroTag_; //用于创建人物的tag
	DisableDir disableDir_ = { 0, 0, 0, 0 }; //使某个方向无效
	Dir dir_ = Dir::D; //人物方向
	int bombTag_; //炮弹的tag
	float rate_ = 2; //移动速率
	float angle_ = 0; //瞄准线角度
	float aimLineLength_ = 50; //瞄准线长度
	cocos2d::Scene *scene_; //当前所在的场景
	cocos2d::DrawNode *drawNode_; //瞄准线绘制点
	cocos2d::PhysicsBody *phyBody_; //人物碰撞体
	cocos2d::PhysicsBody *hurtBody_; //人物受击碰撞体
};

#endif // !HERO_H

