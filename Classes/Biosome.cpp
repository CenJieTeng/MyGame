#include "Biosome.h"

USING_NS_CC;

void Biosome::beHurt(float val) {
	if (bDead_)return;

	curBlood_ -= val;

	//受伤动画
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
		sprite_->runAction(TintBy::create(0.3f, 255, 0, 0));
	});
	//death
	if (curBlood_ <= 0) {
		bDead_ = true;
		//sprite_->removeAllChildren();
		//播放死亡动画
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
			sprite_->runAction(
				Sequence::create(TintTo::create(1.0f, 193, 205, 205), FadeOut::create(1.0f), CallFunc::create([this]() { bCleanable_ = true; }), nullptr));
		});
	}
}
