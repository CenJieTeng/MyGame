#include "Enemy.h"
#include "Constants.h"

USING_NS_CC;

//statics
int Enemy::enemyTag_; //用于创建enemy的tag
std::unordered_map<int, std::shared_ptr<Enemy>> Enemy::mapTagToEnemy; //tag到enemy的映射
cocos2d::Animate* Enemy::moveAnimate_ = nullptr; //移动动画
cocos2d::Animate* Enemy::attackAnimate_ = nullptr; //攻击动画
cocos2d::Animate* Enemy::attackAnimateJump_; //攻击jump
cocos2d::Animate* Enemy::attackAnimateFall_; //攻击fall
//cocos2d::Scene* Enemy::scene_ = Director::getInstance()->getRunningScene(); //游戏场景
int Enemy::attackAnimateDelay_ = 1000; //攻击动画延迟
int Enemy::attackAnimateJumpDelay_ = 700; //jump动画延迟
int Enemy::attackAnimateFallDelay_ = 700; //fall动画延迟

Enemy::~Enemy(){
	bDead_ = true;
	if (threadFollow_ && threadFollow_->joinable())
		threadFollow_->join();

	sprite_->release();
	//scene_->release();
}

std::shared_ptr<Enemy> Enemy::create(){
	auto ret = std::shared_ptr<Enemy>(new Enemy());
	ret->setTag(enemyTag_);
	mapTagToEnemy.emplace(enemyTag_++, ret);
	return ret;
}

std::shared_ptr<Enemy> Enemy::create(Scene *scene, Vec2 pos){
	auto ret = std::shared_ptr<Enemy>(new Enemy(scene, pos));
	ret->setTag(enemyTag_);
	mapTagToEnemy.emplace(enemyTag_++, ret);
	return ret;
}

std::shared_ptr<Enemy> Enemy::create(Scene *scene, Vec2 pos, float scale) {
	auto ret = std::shared_ptr<Enemy>(new Enemy(scene, pos, scale));
	ret->setTag(enemyTag_);
	mapTagToEnemy.emplace(enemyTag_++, ret);
	return ret;
}

int Enemy::getEnemyNum() {
	return mapTagToEnemy.size();
}

std::shared_ptr<Enemy> Enemy::getEnemyByTag(int tag) {
	assert(mapTagToEnemy.find(tag) != mapTagToEnemy.end());
	return mapTagToEnemy[tag];
}

bool Enemy::init(){

	position_ = originPosition_;
	originSpeed_ = speed_ = Vec2(5, 5);
	curBlood_ = maxBlood_ = 100 * scale_;
	atk_ = 5 * scale_;
	attackDistance_ *= scale_;
	bDead_ = false;
	bCleanable_ = false;
	bFollow_ = false;
	bRun_ = false;

	//创建精灵
	sprite_ = Sprite::create("monster1.png");
	sprite_->setAnchorPoint(Vec2(0.5, 0.25));
	sprite_->setPosition(position_);
	sprite_->setScale(scale_); //缩放

	sprite_->retain();
	scene_->retain();

	Size spriteSize = sprite_->getContentSize();
	Vec2 anchoPoint = sprite_->getAnchorPoint();

	//创建仇检测碰撞体
	auto hateBody = PhysicsBody::createCircle(hateRadius_ / scale_);
	hateBody->setCategoryBitmask(0); //不发生一切碰撞事件，仅用于观察
	hateBody->setDynamic(false);
	hateBody->setTag(enemyTag_);
	hateBody->setGravityEnable(false);
	auto node1 = Node::create();
	node1->setPhysicsBody(hateBody);
	node1->setPosition(Vec2(spriteSize.width * anchoPoint.x, spriteSize.height * anchoPoint.y));
	sprite_->addChild(node1);

	//创建攻击范围
	auto attackBody = PhysicsBody::createCircle(attackDistance_ / scale_);
	attackBody->setGravityEnable(false);
	attackBody->setDynamic(false);
	attackBody->setCategoryBitmask(0);
	auto node2 = Node::create();
	node2->setPhysicsBody(attackBody);
	node2->setPosition(Vec2(spriteSize.width * anchoPoint.x, spriteSize.height * anchoPoint.y));
	sprite_->addChild(node2);

	//创建受击碰撞体
	Vec2 points[11] = { {-25, -5}, {-20, 5}, {-26, 13}, {0, 34}, {15, 30}, {30, 10}, {30, 5}, {22, -1}, {17, -16}, {0, -20}, {-15, -17} };
	auto hurtBody = PhysicsBody::createEdgePolygon(points, 11);
	hurtBody->setCategoryBitmask(0x4);
	hurtBody->setContactTestBitmask(0x4);
	hurtBody->setCollisionBitmask(0);
	hurtBody->setTag(enemyTag_);
	hurtBody->setGroup(EnemyGroup);
	auto node3 = Node::create();
	node3->setPhysicsBody(hurtBody);
	node3->setScale(0.9f);
	node3->setPosition(Vec2(spriteSize.width * anchoPoint.x, spriteSize.height * anchoPoint.y));
	sprite_->addChild(node3);

	//创建行走动画
	auto moveAnimation = Animation::create();
	for (int i = 1; i <= 7; ++i) {
		if (i >= 4 && i <= 5) continue;
		std::string fileName;
		sprintf(&fileName[0], "monster%d.png", i);
		auto monstersprite_ = Sprite::create(fileName.c_str());
		moveAnimation->addSpriteFrame(monstersprite_->getSpriteFrame());
	}
	moveAnimation->setDelayPerUnit(0.5f / 5.0f);
	AnimationCache::getInstance()->addAnimation(moveAnimation, "monsterMoveAnimation");
	moveAnimate_ = Animate::create(moveAnimation);
	moveAnimate_->retain();

	//创建攻击动画
	auto atkAnimation1 = Animation::create();
	for (int i = 1; i <= 7; ++i) {
		std::string fileName;
		sprintf(&fileName[0], "monster%d.png", i);
		auto monstersprite_ = Sprite::create(fileName.c_str());
		atkAnimation1->addSpriteFrame(monstersprite_->getSpriteFrame());
	}
	atkAnimation1->setDelayPerUnit((attackAnimateDelay_ / 1000.0f) / 7.0f);
	attackAnimate_ = Animate::create(atkAnimation1);
	attackAnimate_->retain();

	//创建攻击动画jump
	auto atkAnimation2 = Animation::create();
	for (int i = 1; i <= 4; ++i) {
		std::string fileName;
		sprintf(&fileName[0], "monster%d.png", i);
		auto monstersprite_ = Sprite::create(fileName.c_str());
		atkAnimation2->addSpriteFrame(monstersprite_->getSpriteFrame());
	}
	atkAnimation2->setDelayPerUnit((attackAnimateJumpDelay_ / 1000.0f) / 4.0f);
	attackAnimateJump_ = Animate::create(atkAnimation2);
	attackAnimateJump_->retain();

	//创建攻击动画fall
	auto atkAnimation3 = Animation::create();
	for (int i = 5; i <= 7; ++i) {
		std::string fileName;
		sprintf(&fileName[0], "monster%d.png", i);
		auto monstersprite_ = Sprite::create(fileName.c_str());
		atkAnimation3->addSpriteFrame(monstersprite_->getSpriteFrame());
	}
	atkAnimation3->setDelayPerUnit((attackAnimateFallDelay_ / 1000.0f) / 3.0f);
	attackAnimateFall_ = Animate::create(atkAnimation3);
	attackAnimateFall_->retain();

	return true;
}

void Enemy::follow(std::shared_ptr<Hero> hero){

	if (bDead_) return;

	//如果正在运行，则改变状态
	if (bRun_) {
		bFollow_ = true;
		return;
	}

	if (!bFollow_) {
		bRun_ = true;
		bFollow_ = true;

		curFollow_ = hero;

		if (threadFollow_ && threadFollow_->joinable())
			threadFollow_->join();
		
		auto self = shared_from_this();
		threadFollow_ = std::shared_ptr<std::thread>(new std::thread([self, this]() {
			auto hero = curFollow_;
			do {
				if (hero->isDead()) {
					bFollow_ = false;
					break;
				}
				//根据enemy的大小有不同的攻击方式
				if (scale_ < 2) {
					if (!hero->isDead() && GOUGU(abs(hero->getBodyPosition().x - position_.x), abs(hero->getBodyPosition().y - position_.y)) <= attackDistance_) {
						attack1(hero);
					}
				}
				else {
					static int count = 0;

					if (!hero->isDead() && GOUGU(abs(hero->getBodyPosition().x - position_.x), abs(hero->getBodyPosition().y - position_.y)) <= attackDistance_ * 3) {
						attack2(hero);

						//每间隔1次attack2，释放2次attack3
						if (++count % 1 == 0) {
							for (int i = 0; i < 2; ++i) {
								attack3(hero);
							}
						}
					}
				}

				if (hero->isDead()) {
					bFollow_ = false;
					break;
				}

				//判断是否超出可移动范围
				if (GOUGU(abs(position_.x - originPosition_.x), abs(position_.y - originPosition_.y)) > maxMoveDistance_) {
					bFollow_ = false;
				}

				Vec2 offset;
				if (bFollow_)
					offset = position_ - hero->getPosition();
				else
					offset = position_ - originPosition_;

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

				//播放行走动画
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
					auto animate = moveAnimate_->clone();
					sprite_->runAction(animate);
				});
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				position_ += speed;
				sprite_->setPosition(position_);
			} while (!bDead_ && position_ != originPosition_);

			bRun_ = false;
		}));
	}
}

//原地攻击
void Enemy::attack1(std::shared_ptr<Hero> hero) {

	if (bDead_) return;

	//先暂停跟随
	speed_ = Vec2::ZERO;
	while (!bDead_ && !hero->isDead()){

		//绘制攻击范围
		auto self = shared_from_this();
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
			drawNode_ = DrawNode::create();
			drawNode_->drawCircle(position_, attackDistance_, 0, 10, false, Color4F::RED);
			scene_->addChild(drawNode_, 1, 17);
		});

		//播放攻击动画
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
			auto animate = attackAnimate_->clone();
			sprite_->runAction(animate);
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(attackAnimateDelay_));

		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
			scene_->removeChild(drawNode_); //删除绘制
		});

		//判断是否脱离攻击距离
		if (!hero->isDead() && GOUGU(abs(hero->getBodyPosition().x - position_.x), abs(hero->getBodyPosition().y - position_.y)) > attackDistance_ + attackAttachDistance_)
			break;

		hero->beHurt(atk_);
	}

	//恢复跟随
	speed_ = originSpeed_;
}

//锁定目标，砸
void Enemy::attack2(std::shared_ptr<Hero> hero) {
	if (bDead_) return;

	//先暂停跟随
	speed_ = Vec2::ZERO;
	if (!bDead_ && !hero->isDead()) {

		auto pos = hero->getPosition(); //获取攻击点

		//绘制攻击范围
		auto self = shared_from_this();
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, pos, this]() {
			drawNode_ = DrawNode::create();
			drawNode_->drawCircle(pos, attackDistance_ + attackAttachDistance_, 0, 10, false, Color4F::RED);
			scene_->addChild(drawNode_, 1, 17);
		});

		//播放攻击动画jump
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
			auto animate = attackAnimateJump_->clone();
			sprite_->runAction(animate);
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(attackAnimateJumpDelay_));

		position_ = pos;
		sprite_->setPosition(position_);

		//播放攻击动画fall
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, pos, this]() {
			auto animate = attackAnimateFall_->clone();
			sprite_->runAction(animate);
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(attackAnimateFallDelay_));

		Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
			scene_->removeChild(drawNode_); //删除绘制
		});

		//判断是否脱离攻击距离
		if (!hero->isDead() && GOUGU(abs(hero->getBodyPosition().x - position_.x), abs(hero->getBodyPosition().y - position_.y)) > attackDistance_ + attackAttachDistance_) {
			//恢复跟随
			speed_ = originSpeed_;
			return;
		}
			
		hero->beHurt(atk_);
	}
}

//发射子弹球
void Enemy::attack3(std::shared_ptr<Hero> hero) {
	if (bDead_) 
		return;

	auto self = shared_from_this();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
		//释放前的征兆, 缩放
		sprite_->runAction(
			Sequence::create(ScaleTo::create(0.5f, scale_ + 0.2f),
				DelayTime::create(0.3f), ScaleTo::create(0.2f, scale_ - 0.2f), nullptr));
	});
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	Director::getInstance()->getScheduler()->performFunctionInCocosThread([self, this]() {
		static Vec2 force[24] = 
		{ {1.5,1.5},{1.5,-1.5}, {1.5, 0}, {-1.5, 1.5}, {-1.5, -1.5}, {-1.5, 0}, {0,1.5}, {0, -1.5}, 
		  {1,1.5}, {1, 0.5}, {1, -0.5}, {1, -1.5}, {-1, 1.5}, {-1, 0.5}, {-1, -0.5}, {-1, -1.5},
		  {1.5,1}, {0.5, 1}, {-0.5, 1}, {-1.5, 1}, {1.5, -1}, {0.5, -1}, {-0.5, -1}, {-1.5, -1} };

		for (int i = 0; i < sizeof(force)/sizeof(Vec2); ++i) {
			auto bombsprite = Sprite::create("bomb.png", Rect(95, 8.5, 7, 7));
			//创建子弹碰撞体
			auto bombBody = PhysicsBody::createCircle(3.5);
			bombsprite->setPosition(position_);
			bombBody->setGravityEnable(false);
			bombBody->setCategoryBitmask(0x16);
			bombBody->setContactTestBitmask(0x16);
			bombBody->setCollisionBitmask(0);
			bombBody->setGroup(EnemyBombGroup);
			bombBody->setTag(tag_);

			bombBody->applyImpulse(Vec2(force[i].x * 500, force[i].y * 500));

			bombsprite->setPhysicsBody(bombBody);
			scene_->addChild(bombsprite, 1);

			//创建定时器，到达一定时间清除炮弹
			bombsprite->runAction(
				Sequence::create(DelayTime::create(5.0f),
					CallFunc::create([self, this, bombsprite]() { scene_->removeChild(bombsprite); }), nullptr));
		}
	});
}

void Enemy::beHurt(float val){
	Biosome::beHurt(val);

	//deadth
	if (curBlood_ <= 0)
		sprite_->removeAllChildren();
}
