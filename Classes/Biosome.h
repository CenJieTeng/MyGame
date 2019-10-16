#ifndef	BIOSOME_H
#define BIOSOME_H
#include "cocos2d.h"

//生物
class Biosome {
public:
	Biosome(cocos2d::Vec2 pos = cocos2d::Vec2::ZERO)
		: maxBlood_(0),
		curBlood_(0),
		atk_(0),
		bDead_(false),
		bCleanable_(false),
		position_(pos),
		originPosition_(pos){
	}

	virtual ~Biosome() {}

	cocos2d::Sprite *getSprite() { return sprite_; }//返回精灵
	const cocos2d::Vec2 getOriginPosition() { return originPosition_; }
	const cocos2d::Vec2 getPosition() { return position_; }
	void setPosition(cocos2d::Vec2 pos) { position_ = pos; }
	void setPositionX(float x) { position_.x = x; }
	void setPositionY(float y) { position_.y = y; }
	const cocos2d::Vec2 getSpeed() { return speed_; }
	void setSpeed(cocos2d::Vec2 speed) { speed_ = speed; }
	void setSpeedX(float x) { speed_.x = x; }
	void setSpeedY(float y) { speed_.y = y; }
	int getTag() { return tag_; } //获取tag
	void setTag(int tag) { tag_ = tag; } //设置tag
	const float getMaxBlood() { return maxBlood_; }
	const float getCurBlood() { return curBlood_; }
	void setCurBlood(float val) { curBlood_ = val > maxBlood_ ? maxBlood_ : val; }
	const float getAtk() { return atk_; }
	void setAtk(float val) { atk_ = val; }
	const bool isDead() { return bDead_; }
	const bool isCleanable() { return bCleanable_; }
	
	virtual void beHurt(float val); //受到伤害

protected:
	cocos2d::Sprite* sprite_; //精灵
	cocos2d::Vec2 originPosition_; //初始位置
	cocos2d::Vec2 position_; //实时位置
	cocos2d::Vec2 speed_; //实时速度
	int tag_; //tag
	float maxBlood_; //最大血量
	float curBlood_; //当前血量
	float atk_; //攻击力
	bool bDead_; //死亡
	bool bCleanable_; //可以清除
};

#endif // !BIOSOME_H
