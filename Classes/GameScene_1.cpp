#include "GameScene_1.h"
#include "Constants.h"

USING_NS_CC;

cocos2d::Scene * GameScene1::createScene(bool ol){
	Scene *phyScene_ = Scene::createWithPhysics(); //创建物理场景
	phyScene_->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL); //设置debug模式

	GameScene::setOnlineGame(ol);
	GameScene1 *gameScene = GameScene1::create();
	gameScene->setPhyWorld(phyScene_->getPhysicsWorld()); //把物理世界传到父场景
	phyScene_->addChild(gameScene);
	return phyScene_;
}

bool GameScene1::init(){

	if (!GameScene::init())
		return false;

	visibleSize_ = Director::getInstance()->getVisibleSize();
	//auto origin = Director::getInstance()->getVisibleOrigin();

	//创建背景图片
	auto bg_sprite = Sprite::create("bg.png");
	bgSize_ = bg_sprite->getContentSize();
	assert(bg_sprite != nullptr
		&& bg_sprite->getContentSize().width != 0
		&& bg_sprite->getContentSize().height != 0);

	bg_sprite->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
	this->addChild(bg_sprite);

	//--创建物理边界
	//下边界
	PhysicsBody *edgeBoxBody = PhysicsBody::createBox(Size(visibleSize_.width, 20));
	edgeBoxBody->setCategoryBitmask(0x8);
	edgeBoxBody->setContactTestBitmask(0x2);
	edgeBoxBody->setPositionOffset(Vec2(0, 0));
	edgeBoxBody->setDynamic(false);
	edgeBoxBody->setGroup(StaticBlockGroup);
	auto border1 = Node::create();
	border1->setPosition(Vec2((visibleSize_.width/2),0));
	border1->setPhysicsBody(edgeBoxBody);
	this->addChild(border1);

	////上边界
	//edgeBoxBody = PhysicsBody::createBox(Size(visibleSize_.width, 10));
	//edgeBoxBody->setPositionOffset(Vec2(0, 10));
	//edgeBoxBody->setDynamic(false);
	//edgeBoxBody->setContactTestBitmask(0x1);
	//auto border2 = Node::create();
	//border2->setPosition(visibleSize_ / 2);
	//border2->setPhysicsBody(edgeBoxBody);
	//this->addChild(border2);

	////测试用碰撞体
	//edgeBoxBody = PhysicsBody::createBox(Size(20, 20));
	//edgeBoxBody->setPositionOffset(Vec2(50, -50));
	//edgeBoxBody->setDynamic(false);
	//edgeBoxBody->setContactTestBitmask(1);
	//edgeBoxBody->setGroup(GameScene::BombBlockGroup);
	//auto border3 = Node::create();
	//border3->setPosition(visibleSize_ / 2);
	//border3->setPhysicsBody(edgeBoxBody);
	//this->addChild(border3);

	countR_ = countL_ = countU_ = countD_ = 1;

	//开启定时器
	scheduleUpdate();

	return true;
}

void GameScene1::update(float delta){

	GameScene::update(delta);

	int offsetX = (hero_->getPosition().x - hero_->getOriginPosition().x) / bgSize_.width;
	int offsetY = (hero_->getPosition().y - hero_->getOriginPosition().y) / bgSize_.height;

	//地图滚动
	if (hero_->getPosition().x - hero_->getOriginPosition().x + visibleSize_.width/2 > bgSize_.width * (1/2.0 + (countR_-1))) {
		auto bg_sprite1 = Sprite::create("bg.png");
		auto bg_sprite2 = Sprite::create("bg.png");
		auto bg_sprite3 = Sprite::create("bg.png");
		bg_sprite1->setPosition(Vec2(visibleSize_.width/2 + bgSize_.width * countR_, visibleSize_.height/2 + offsetY * bgSize_.height - bgSize_.height));
		bg_sprite2->setPosition(Vec2(visibleSize_.width/2 + bgSize_.width * countR_, visibleSize_.height/2 + offsetY * bgSize_.height));
		bg_sprite3->setPosition(Vec2(visibleSize_.width/2 + bgSize_.width * countR_, visibleSize_.height/2 + offsetY * bgSize_.height + bgSize_.height));
		this->addChild(bg_sprite1);
		this->addChild(bg_sprite2);
		this->addChild(bg_sprite3);

		++countR_;
	}
	if (hero_->getPosition().x - hero_->getOriginPosition().x - visibleSize_.width/2 < -bgSize_.width * (1 / 2.0 + (countL_ - 1))) {
		auto bg_sprite1 = Sprite::create("bg.png");
		auto bg_sprite2 = Sprite::create("bg.png");
		auto bg_sprite3 = Sprite::create("bg.png");
		bg_sprite1->setPosition(Vec2(visibleSize_.width/2 - bgSize_.width * countL_, visibleSize_.height/2 + offsetY * bgSize_.height - bgSize_.height));
		bg_sprite2->setPosition(Vec2(visibleSize_.width/2 - bgSize_.width * countL_, visibleSize_.height/2 + offsetY * bgSize_.height));
		bg_sprite3->setPosition(Vec2(visibleSize_.width/2 - bgSize_.width * countL_, visibleSize_.height/2 + offsetY * bgSize_.height + bgSize_.height));
		this->addChild(bg_sprite1);
		this->addChild(bg_sprite2);
		this->addChild(bg_sprite3);

		++countL_;
	}
	if (hero_->getPosition().y - hero_->getOriginPosition().y + visibleSize_.height / 2 > bgSize_.width * (1 / 2.0 + (countU_ - 1))) {
		auto bg_sprite1 = Sprite::create("bg.png");
		auto bg_sprite2 = Sprite::create("bg.png");
		auto bg_sprite3 = Sprite::create("bg.png");
		bg_sprite1->setPosition(Vec2(visibleSize_.width/2 + offsetX * bgSize_.width - bgSize_.width, visibleSize_.height/2 + bgSize_.height * countU_));
		bg_sprite2->setPosition(Vec2(visibleSize_.width/2 + offsetX * bgSize_.width, visibleSize_.height/2 + bgSize_.height * countU_));
		bg_sprite3->setPosition(Vec2(visibleSize_.width/2 + offsetX * bgSize_.width + bgSize_.width, visibleSize_.height/2 + bgSize_.height * countU_));
		this->addChild(bg_sprite1);
		this->addChild(bg_sprite2);
		this->addChild(bg_sprite3);

		++countU_;
	}
	if (hero_->getPosition().y - hero_->getOriginPosition().y - visibleSize_.height/2 < -bgSize_.height * (1 / 2.0 + (countD_ - 1))) {
		auto bg_sprite1 = Sprite::create("bg.png");
		auto bg_sprite2 = Sprite::create("bg.png");
		auto bg_sprite3 = Sprite::create("bg.png");
		bg_sprite1->setPosition(Vec2(visibleSize_.width / 2 + offsetX * bgSize_.width - bgSize_.width, visibleSize_.height / 2 - bgSize_.height * countD_));
		bg_sprite2->setPosition(Vec2(visibleSize_.width / 2 + offsetX * bgSize_.width, visibleSize_.height / 2 - bgSize_.height * countD_));
		bg_sprite3->setPosition(Vec2(visibleSize_.width / 2 + offsetX * bgSize_.width + bgSize_.width, visibleSize_.height / 2 - bgSize_.height * countD_));
		this->addChild(bg_sprite1);
		this->addChild(bg_sprite2);
		this->addChild(bg_sprite3);

		++countD_;
	}
}