#include "BloodBall.h"

USING_NS_CC;

std::shared_ptr<BloodBall> BloodBall::create(cocos2d::Scene * scene, cocos2d::Vec2 pos) {
	return std::shared_ptr<BloodBall>(new BloodBall(scene, pos));
}

bool BloodBall::init() {

	//创建精灵
	sprite_ = Sprite::create("bloodball.png");
	sprite_->setPosition(position_);
	scene_->addChild(sprite_, 1);

	return true;
}

void BloodBall::buff(std::shared_ptr<Hero> hero) {
	hero->setCurBlood(hero->getCurBlood() + 10);
	scene_->removeChild(sprite_);
	bDead_ = true;
}