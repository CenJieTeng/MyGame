#ifndef SWORD_H
#define SWORD_H
#include "Goods.h"
#include "Hero.h"

//提升攻击力
class Sword : public Goods {
public:
	Sword(cocos2d::Scene *scene, cocos2d::Vec2 pos)
		: scene_(scene),
		Goods(pos) {
		init();
	}
	~Sword() {};
	bool init();

	static std::shared_ptr<Sword> create(cocos2d::Scene *scene, cocos2d::Vec2 pos);

	virtual void buff(std::shared_ptr<Hero> hero);

private:
	cocos2d::Scene *scene_;
};

#endif // !BLOODBALL_H
