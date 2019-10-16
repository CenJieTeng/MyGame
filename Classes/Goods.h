#ifndef GOODS_H
#define GOODS_H
#include "cocos2d.h"
#include "Hero.h"

//物品
class Goods{
public:
	Goods(cocos2d::Vec2 pos = cocos2d::Vec2::ZERO) 
		: position_(pos),
		speed_(cocos2d::Vec2(2,2)),
		adsorbDistance_(100),
		bDead_(false){};
	virtual ~Goods(){};

	cocos2d::Sprite *getSprite() { return sprite_; }//返回精灵
	const cocos2d::Vec2 getPosition() { return position_; }
	void setPosition(cocos2d::Vec2 pos) { position_ = pos; }
	void setPositionX(float x) { position_.x = x; }
	void setPositionY(float y) { position_.y = y; }
	void follow(std::shared_ptr<Hero> hero); //跟随hero
	const float getAdsorbDistance_() { return adsorbDistance_; }
	const bool isDead() { return bDead_; }
	virtual void buff(std::shared_ptr<Hero> hero) = 0; //buff效果

protected:
	cocos2d::Sprite *sprite_; //精灵
	cocos2d::Vec2 position_; //当前位置
	cocos2d::Vec2 speed_; //速度
	float adsorbDistance_; //吸附距离
	bool bDead_;
};

#endif // !GOODS_H