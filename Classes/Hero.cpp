#include "Hero.h"
#include "Constants.h"

USING_NS_CC;

int Hero::heroTag_ = 0; //人物的tag
std::unordered_map<int, std::shared_ptr<Hero>> Hero::mapTagToHero; //tag到hero的映射

std::shared_ptr<Hero> Hero::create(cocos2d::Scene *scene){
	auto ret = std::shared_ptr<Hero>(new Hero(scene));
	ret->setTag(heroTag_);
	mapTagToHero.emplace(heroTag_++, ret);
	return ret;
}

std::shared_ptr<Hero> Hero::create(cocos2d::Scene *scene, Vec2 pos){
	auto ret = std::shared_ptr<Hero>(new Hero(scene, pos));
	ret->setTag(heroTag_);
	mapTagToHero.emplace(heroTag_++, ret);
	return ret;
}

std::shared_ptr<Hero> Hero::getHeroByTag(int tag) {
	assert(mapTagToHero.find(tag) != mapTagToHero.end());
	return mapTagToHero[tag];
}

bool Hero::init(){

	atk_ = 10;
	maxBlood_ = curBlood_ = 100;
	bombTag_ = heroTag_ * 1000;

	//创建精灵
	sprite_ = Sprite::create();
	sprite_->setPosition(position_);

	//创建人物碰撞体 & 关联到结点
	phyBody_ = PhysicsBody::createEdgeBox(Size(20, 5));
	phyBody_->setPositionOffset(Vec2(0, 0));
	phyBody_->setCategoryBitmask(0x2);
	phyBody_->setContactTestBitmask(0x8);
	phyBody_->setCollisionBitmask(0);
	phyBody_->setGravityEnable(false);
	phyBody_->setGroup(HeroGroup);
	phyBody_->setTag(heroTag_);
	Node *bodyNode = Node::create();
	bodyNode->setPosition(Vec2(20, 5));
	bodyNode->setPhysicsBody(phyBody_);
	sprite_->addChild(bodyNode, 1);

	//创建人物受击碰撞体
	hurtBody_ = PhysicsBody::createEdgeBox(Size(20, 30));
	hurtBody_->setPositionOffset(Vec2(0, 0));
	hurtBody_->setCategoryBitmask(0x16);
	hurtBody_->setContactTestBitmask(0x16);
	hurtBody_->setCollisionBitmask(0);
	hurtBody_->setGravityEnable(false);
	hurtBody_->setGroup(HeroGroup);
	hurtBody_->setTag(heroTag_);
	Node *hurtNode = Node::create();
	hurtNode->setPosition(Vec2(20, 16));
	hurtNode->setPhysicsBody(hurtBody_);
	sprite_->addChild(hurtNode, 1);

	//创建人物动画L
	auto heroAnimation = Animation::create();
	for (int i = 1; i <= 2; ++i) {
		std::string fileName;
		sprintf(&fileName[0], "hero%d.png", i);
		auto bombsprite = Sprite::create(fileName.c_str());
		heroAnimation->addSpriteFrame(bombsprite->getSpriteFrame());
	}
	heroAnimation->setDelayPerUnit(0.8f / 2.0f); //1秒播放2帧
	AnimationCache::getInstance()->addAnimation(heroAnimation, "heroAnimation"); //缓存动画

	Animate *animate = Animate::create(heroAnimation);
	sprite_->runAction(RepeatForever::create(animate));
	scene_->addChild(sprite_, 1);

	return true;
}

void Hero::makeBomb() {
	auto bombsprite = Sprite::create("bomb.png", Rect(95, 8.5, 7, 7));
	//创建子弹碰撞体
	auto bombBody = PhysicsBody::createCircle(3.5);
	bombBody->setGravityEnable(false);
	bombBody->setCategoryBitmask(0x4);
	bombBody->setContactTestBitmask(0x4);
	bombBody->setGroup(BombGroup);
	bombBody->setTag(bombTag_);
	bombsprite->setPhysicsBody(bombBody);

	Bomb bomb;
	bomb.sprite = bombsprite;
	bombsprite->setPosition(position_);
	bombBody->applyImpulse(Vec2(Bomb::power * cosf(angle_ * (PI / 180)), Bomb::power * sinf(angle_ * (PI / 180))));

	scene_->addChild(bombsprite, 1, bombTag_++);

	//创建定时器，到达一定时间清除炮弹
	bomb.sprite->runAction(
		Sequence::create(DelayTime::create(1.0f),
			CallFunc::create([scene = scene_, bombsprite]() { scene->removeChild(bombsprite); }), nullptr));
}

void Hero::drawAimLine(Vec2 cursorPosition) {
	//--刷新瞄准线
	//计算夹角
	Vec2 p1 = { 0, 0 };
	Vec2 p2 = { 100, 0 };
	Vec2 p3 = cursorPosition - originPosition_;

	auto t = ((p1.x - p2.x)*(p1.x - p3.x) + (p1.y - p2.y)*(p1.y - p3.y))
		/ (sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2)) * sqrt(pow(p1.x - p3.x, 2) + pow(p1.y - p3.y, 2)));

	angle_ = acos(t)*(180 / PI);
	if (p3.y < 0) angle_ = 360 - angle_;

	//绘制瞄准线
	Vec2 p4 = position_; //起点
	Vec2 d(aimLineLength_ * cosf(angle_ * (PI / 180)), aimLineLength_ * sinf(angle_ * (PI / 180)));
	Vec2 p5 = p4 + d; //终点

	scene_->removeChild(drawNode_); //删除上一次绘制
	drawNode_ = nullptr;
	drawNode_ = DrawNode::create();

	drawNode_->drawLine(p4, p5, Color4F(1.0f, 0.0f, 0.0f, 1.0f));

	scene_->addChild(drawNode_, 1, 16);

	//改变人物方向
	if (angle_ >= 90 && angle_ <= 270) {
		sprite_->setRotation3D(Vec3(0.0f, 0.0f, 0.0f));
	}
	else {
		sprite_->setRotation3D(Vec3(0.0f, 180.0f, 0.0f));
	}		
}