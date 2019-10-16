#ifndef BLOODBALL_H
#define BLOODBALL_H
#include "Goods.h"
#include "Hero.h"

//恢复血量球体
class BloodBall : public Goods {
public:
	BloodBall(cocos2d::Scene *scene, cocos2d::Vec2 pos) 
		: scene_(scene),
		Goods(pos) { init(); }
	~BloodBall() {};
	bool init();

	static std::shared_ptr<BloodBall> create(cocos2d::Scene *scene, cocos2d::Vec2 pos);

	virtual void buff(std::shared_ptr<Hero> hero);

private:
	cocos2d::Scene *scene_;
};

#endif // !BLOODBALL_H
