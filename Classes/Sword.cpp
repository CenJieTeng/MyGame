#include "Sword.h"

USING_NS_CC;

std::shared_ptr<Sword> Sword::create(cocos2d::Scene * scene, cocos2d::Vec2 pos) {
	return std::shared_ptr<Sword>(new Sword(scene, pos));
}

bool Sword::init() {

	//创建精灵
	sprite_ = Sprite::create("sword.png");
	sprite_->setPosition(position_);
	scene_->addChild(sprite_, 1);

	return true;
}

void Sword::buff(std::shared_ptr<Hero> hero) {
	hero->setAtk(hero->getAtk() + 2);
	scene_->removeChild(sprite_);
	bDead_ = true;
}