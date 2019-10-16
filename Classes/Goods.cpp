#include "Goods.h"
#include "Constants.h"

USING_NS_CC;

//跟随移动
void Goods::follow(std::shared_ptr<Hero> hero){
	Vec2 offset;
	offset = position_ - hero->getPosition();

	if (GOUGU(offset.x, offset.y) <= 5) {
		buff(hero);
		return;
	}

	Vec2 speed = speed_;
	if (abs(offset.x) >= speed.x) {
		if (offset.x > 0) speed.x = -speed.x;
	}
	else {
		if (offset.x > 0) speed.x = -offset.x;
		else
			speed.x = offset.x;
	}

	if (abs(offset.y) >= speed.y) {
		if (offset.y > 0) speed.y = -speed.y;
	}
	else {
		if (offset.y > 0) speed.y = -offset.y;
		else
			speed.y = offset.y;
	}

	position_ += speed;
	sprite_->setPosition(position_);
}