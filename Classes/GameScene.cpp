#include "GameScene.h"
#include "Constants.h"
#include "BloodBall.h"
#include "Sword.h"
#include "RegisterLoginScene.h"
#include "message.pb.h"

USING_NS_CC;
using namespace cocos2d::ui;

const int GameScene::MaxEnemyNum_ = 5; //最大敌人数量

/*
--tag使用情况
1~4			人物tag
1000~5000	炮弹tag(1000~2000是人物1(tag==1)的炮弹tag)
16			瞄准线(虽然没有意义，但是不添加就有问题)
100~1000 敌人(暂时)
*/

//设置会话
void GameScene::setSession(session_ptr sp) {
	session_ = sp;
}
session_ptr GameScene::session_ = nullptr; //会话

 //设置是否在线游戏
void GameScene::setOnlineGame(bool ol) {
	bOnlineGame_ = ol;
}
bool GameScene::bOnlineGame_; //是否在线游戏

bool GameScene::init()
{
	if (!Scene::init())
		return false;

	visibleSize_ = Director::getInstance()->getVisibleSize();
	origin_ = Director::getInstance()->getVisibleOrigin();

	killCount_ = 0;
	bSetSeed_ = false;
	isFocused_ = false;

	if (!bOnlineGame_)
		bSetSeed_ = true;

	//初始化等级标签
	level_ = 1;
	String *lev = String::createWithFormat("Level %d", level_);
	auto label = Label::createWithTTF(lev->getCString(), "fonts/arial.ttf", 15);
	label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height - 20));
	label->setCameraMask(0x2);
	this->addChild(label, 1, "level");

	//人物初始位置
	Vec2 position = (Vec2(origin_.x + visibleSize_.width / 2, origin_.y + visibleSize_.height / 2));

	//创建人物
	hero_ = Hero::create(this, position);

	//初始化EnemyTag
	Enemy::initCreateEnemyTag(100);

	//初始化网络环境
	if (bOnlineGame_)
		initNetwork();

	//获取默认camera
	defaultCamera_ = this->getDefaultCamera();

	//创建UIcamera
	uiCamera_ = Camera::create();
	uiCamera_->setCameraFlag(CameraFlag::USER1);
	uiCamera_->setPosition(position);
	this->addChild(uiCamera_);

	//血条UI
	auto barsprite_ = Sprite::create("UI/bar.png"); //创建血条框精灵
	barsprite_->setPosition(Vec2(barsprite_->getContentSize().width / 2, visibleSize_.height - 10));
	barsprite_->setCameraMask(0x2);
	this->addChild(barsprite_, 1);
	auto bloodsprite_ = Sprite::create("UI/blood.png"); //创建血条精灵
	auto progress = ProgressTimer::create(bloodsprite_); //创建progress对象
	progress->setType(ProgressTimer::Type::BAR); //设置类型，条形
	progress->setPosition(bloodsprite_->getContentSize().width / 2, visibleSize_.height - 10);
	progress->setMidpoint(Vec2(0, 0.5)); //设置血条从右向左递减
	progress->setBarChangeRate(Vec2(1, 0));
	progress->setCameraMask(0x2);
	this->addChild(progress, 1, "BloodBar");

	//炮弹爆炸动画
	auto bombAnimation = Animation::create();
	for (int i = 18; i <= 24; ++i) {
		auto bombsprite_ = Sprite::create("bomb.png", Rect(104 + 7.5 * (i - 18), 8.5, 7, 7));
		bombAnimation->addSpriteFrame(bombsprite_->getSpriteFrame());
	}
	bombAnimation->setDelayPerUnit(1.0f / 7.0f); //1秒播放7帧
	AnimationCache::getInstance()->addAnimation(bombAnimation, "bombAnimation"); //缓存动画

	//监听鼠标事件
	auto mouseListener = EventListenerMouse::create();
	mouseListener->onMouseMove = CC_CALLBACK_1(GameScene::onMouseMove, this);
	mouseListener->onMouseDown = CC_CALLBACK_1(GameScene::onMouseDown, this);
	mouseListener->onMouseUp = CC_CALLBACK_1(GameScene::onMouseUp, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

	//--监听键盘事件
	auto keyBoardListener = EventListenerKeyboard::create();//创建键盘事件监听器
	keyBoardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
	keyBoardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);

	//auto dispatcher = Director::getInstance()->getEventDispatcher(); //获取事件调度器
	//dispatcher->addEventListenerWithSceneGraphPriority(keyBoardListener, this); //把键盘监听器添加到事件调度器
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyBoardListener, this);//等价于上面两行

	//监听人物碰撞事件
	auto contactListener = EventListenerPhysicsContact::create(); //创建触点监听器
	contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this); //设置碰撞(接触)回调函数
	contactListener->onContactSeparate = CC_CALLBACK_1(GameScene::onContactSeparate, this); //设置碰撞(分离)回调函数
	
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this); //把触点监听器添加到事件调度器

	//创建物理debug模式开关
	auto phyDebugItem = MenuItemFont::create("Physics Debug", 
		CC_CALLBACK_1(GameScene::menuPhyDebug, this)); //设置按钮回调
	phyDebugItem->setFontSizeObj(10);
	phyDebugItem->setPosition(Vec2(visibleSize_ - phyDebugItem->getContentSize()));
	auto menu = Menu::create(phyDebugItem, nullptr);
	menu->setPosition(Vec2::ZERO);
	menu->setCameraMask(0x2);
	this->addChild(menu, 1);

	//聊天输入框
	textField_ = TextField::create("Input chat msg.", "fonts/arial.ttf", 10);
	textField_->setTextColor(Color4B::YELLOW);
	textField_->setPosition(Vec2(50, 20));
	textField_->setMaxLengthEnabled(true);
	textField_->setMaxLength(20); //输入最大长度
	textField_->setCursorEnabled(true); //显示光标
	textField_->setTouchAreaEnabled(true);
	textField_->setTouchSize(Size(70, 15)); //设置进入输入框焦点的触摸范围
	textField_->addEventListener(CC_CALLBACK_2(GameScene::onTextFieldEvent, this)); //添加监听
	textField_->setCameraMask(0x2);
	this->addChild(textField_, 1);

	//聊天内容显示列表
	chatMsgList_ = ScrollView::create();
	chatMsgList_->setBounceEnabled(true);
	chatMsgList_->setBackGroundImage("chatbg250x250.png");
	chatMsgList_->setBackGroundImageOpacity(50);
	chatMsgList_->setScrollBarAutoHideEnabled(false);
	chatMsgList_->setSize(Size(0, 200));
	chatMsgList_->setContentSize(Size(120, 80));
	chatMsgList_->setPosition(Vec2(0, 30));
	chatMsgList_->jumpToBottom(); //默认在低端
	chatMsgList_->setCameraMask(0x2);
	this->addChild(chatMsgList_, 1);

	//伤害统计列表
	damageNode_ = Node::create();
	this->addChild(damageNode_, 1);

	return true;
}

GameScene::~GameScene(){
	//清理Biosome
	Hero::mapTagToHero.clear();
	Enemy::mapTagToEnemy.clear();
	players_.clear();
}

bool GameScene::initNetwork(){
	players_.emplace(session_->getKey(), hero_);

	Session::setCallBack("read", [this](session_ptr sp) {
		switch (sp->getProtoMessageType()) {
			case ProtoMessageType::SESSION_MSG: {
				SessionMsg msg;
				assert(msg.ParseFromString(sp->getReadmsg()));

				switch (msg.msgtype()) {
					case SessionMsgType::S_GETSEED: {
						Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
							srand(msg.seed()); //同步随机数种子
							bSetSeed_ = true;
						});
					}break;
					case SessionMsgType::S_CLOSE: {
						assert(players_.find(msg.key()) != players_.end());
						auto player = players_[msg.key()];
						player->setCurBlood(0);
						players_.erase(msg.key());
					}break;
					case SessionMsgType::S_LOGOUT: {
						Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
							Director::getInstance()->end();
						});
					}break;
					default:
						assert(false);
				}
			}break;
			case ProtoMessageType::GAME_MSG: {
				GameMsg msg;
				assert(msg.ParseFromString(sp->getReadmsg()));

				if (players_.find(msg.key()) == players_.end() && msg.msgtype() == GameMsgType::G_CREATE) {
					//创建其它玩家
					Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg,this]() {
						players_.emplace(msg.key(), Hero::create(this, Vec2(240, 160)));
					});
				}
				else if (msg.msgtype() == GameMsgType::G_CHAT) {
					Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
						chatMsgList_->removeAllChildren();
						auto text = Text::create(msg.chatmsg(), "fonts/arial.ttf", 10);
						text->setAnchorPoint(Vec2(0, 0));
						text->setPosition(Vec2::ZERO);
						text->retain();
						chatMsgDeque_.push_back(text);

						//最多显示20条聊天记录
						if (chatMsgDeque_.size() == 21) {
							chatMsgDeque_[0]->release();
							chatMsgDeque_.pop_front();
						}

						//遍历聊天消息队列，添加信息到chatMsgList
						int count = 0;
						for (auto rbegin = chatMsgDeque_.rbegin(); rbegin != chatMsgDeque_.rend(); ++rbegin, ++count) {
							text = *rbegin;
							text->setPosition(Vec2(0, count * 10));
							text->setCameraMask(0x2);
							chatMsgList_->addChild(text, 2);
						}
					});
				}
				else if (msg.msgtype() == GameMsgType::G_GET_DAMAGE) {
					Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
						assert(msg.damage_size() == msg.keys_size());

						damageNode_->removeAllChildren();

						//排序
						std::vector<std::pair<int, float>> vec;
						for (int i = 0; i <msg.keys_size(); ++i) {
							vec.emplace_back(std::pair<int, float>(msg.keys(i), msg.damage(i)));
						}
						std::sort(vec.begin(), vec.end(),
						[](const std::pair<int, float> &lhs, const std::pair<int, float> &rhs) {
							return lhs.second > rhs.second;
						});

						for (int i = 0; i < msg.keys_size(); ++i) {
							String *str = String::createWithFormat("%d: %.1f", i + 1, vec[i].second);
							auto text = Text::create(str->getCString(), "fonts/arial.ttf", 10);
							text->setPosition(Vec2(20, visibleSize_.width / 2 + i * -10));
							text->setAnchorPoint(Vec2(0, 0.5));
							text->setCameraMask(0x2);
							if (vec[i].first == session_->getKey())
								text->setColor(Color3B(255, 0, 0));
							damageNode_->addChild(text, 1);
						}
					});
				}
				else if (msg.key() != session_->getKey() && players_.find(msg.key()) != players_.end()) {
					//--其它玩家的动作
					auto player = players_[msg.key()];
						
					//发射子弹
					if (msg.msgtype() == GameMsgType::G_ATTACK) {
						//在主线程中执行
						Director::getInstance()->getScheduler()->performFunctionInCocosThread([player]() {
							player->makeBomb();
						});
						return;
					}
					else if(msg.msgtype() == GameMsgType::G_POSITION) {
						//移动
						player->setPosition(Vec2(msg.x(), msg.y()));
						player->getSprite()->setPosition(Vec2(msg.x(), msg.y()));
						player->setAngle(msg.angle());

						//改变人物方向
						if (player->getAngle() >= 90 && player->getAngle() <= 270)
							player->getSprite()->setRotation3D(Vec3(0.0f, 0.0f, 0.0f));
						else
							player->getSprite()->setRotation3D(Vec3(0.0f, 180.0f, 0.0f));
					}
					else if (msg.msgtype() == GameMsgType::G_ENEMY_HURT) {
						Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, player,this]() {
							Enemy::getEnemyByTag(msg.enemytag())->beHurt(msg.atk());
						});
					}
				}
				
			}break;
			default:
				assert(false);
		}
	});

	return true;
}

void GameScene::onMouseMove(cocos2d::Event * event){
	EventMouse *e = (EventMouse*)event;
	cursorPosition_ = { e->getCursorX(), e->getCursorY() };
	hero_->drawAimLine(cursorPosition_);
}

void GameScene::onMouseDown(cocos2d::Event * event){
	static int count = 0;
	EventMouse *e = (EventMouse*)event;

	if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
		if (++count % 2 != 0) {
			schedule(schedule_selector(GameScene::makeBombUpdate), 0.2f);
		}
	}
}

void GameScene::onMouseUp(cocos2d::Event * event){
	static int count = 0;
	EventMouse *e = (EventMouse*)event;

	if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
		if (++count % 2 != 0) {
			unschedule(schedule_selector(GameScene::makeBombUpdate));
		}
	}
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event * event){
	switch (keyCode){
			case EventKeyboard::KeyCode::KEY_W: {
				if (!isFocused_ && !hero_->getDisableDir().U) {
					hero_->setSpeed(Vec2(hero_->getSpeed().x, 1 * hero_->getRate()));
					if (!hero_->getSpeed().x)
						hero_->setDir(Hero::Dir::D);
				}
			}break;
			case EventKeyboard::KeyCode::KEY_A: {
				if (!isFocused_ && !hero_->getDisableDir().L) {
					hero_->setSpeed(Vec2(-1 * hero_->getRate(), hero_->getSpeed().y));
					if (!hero_->getSpeed().y)
						hero_->setDir(Hero::Dir::L);
				}
			}break;
			case EventKeyboard::KeyCode::KEY_S: {
				if (!isFocused_ && !hero_->getDisableDir().D) {
					hero_->setSpeed(Vec2(hero_->getSpeed().x, -1 * hero_->getRate()));
					if (!hero_->getSpeed().x)
						hero_->setDir(Hero::Dir::D);
				}
			}break;
			case EventKeyboard::KeyCode::KEY_D: {
				if (!isFocused_ && !hero_->getDisableDir().R) {
					hero_->setSpeed(Vec2(1 * hero_->getRate(), hero_->getSpeed().y));
					if (!hero_->getSpeed().y)
						hero_->setDir(Hero::Dir::R);
				}
			}break;
		case  EventKeyboard::KeyCode::KEY_ENTER: {
			std::string chatmsg = textField_->getString(); //从输入框获取输入内容
			if (!chatmsg.empty()) {
				textField_->setString(""); //清空原来的内容

				GameMsg msg;
				msg.set_msgtype(GameMsgType::G_CHAT);
				msg.set_allocated_chatmsg(new std::string(std::move(chatmsg)));
				session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::GAME_MSG);
			}
		}break;
	}
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event){
	switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_W: {
			if (hero_->getSpeed().y > 0) hero_->setSpeedY(0);
			if (hero_->getSpeed().x)
				hero_->setDir(hero_->getSpeed().x > 0 ? Hero::Dir::R : Hero::Dir::L);
		}break;
		case EventKeyboard::KeyCode::KEY_A: {
			if (hero_->getSpeed().x < 0) hero_->setSpeedX(0);
			if (hero_->getSpeed().y)
				hero_->setDir(hero_->getSpeed().y > 0 ? Hero::Dir::U : Hero::Dir::D);
		}break;
		case EventKeyboard::KeyCode::KEY_S: {
			if (hero_->getSpeed().y < 0) hero_->setSpeedY(0);
			if (hero_->getSpeed().x)
				hero_->setDir(hero_->getSpeed().x > 0 ? Hero::Dir::R : Hero::Dir::L);
		}break;
		case EventKeyboard::KeyCode::KEY_D: {
			if (hero_->getSpeed().x > 0) hero_->setSpeedX(0);
			if (hero_->getSpeed().y)
				hero_->setDir(hero_->getSpeed().y > 0 ? Hero::Dir::U : Hero::Dir::D);
		}break;
	}
}

void GameScene::onTextFieldEvent(cocos2d::Ref * ref, cocos2d::ui::TextField::EventType et){
	switch (et){
		case TextField::EventType::ATTACH_WITH_IME: {
			isFocused_ = true;
		}break;
		case TextField::EventType::DETACH_WITH_IME: {
			isFocused_ = false;
		}break;
	}
}

void GameScene::update(float delta) {

	if (hero_->isDead()) {
		unscheduleUpdate();

		//清理Biosome
		Hero::mapTagToHero.clear();
		Enemy::mapTagToEnemy.clear();

		//切换场景
		Director::getInstance()->popScene();

		return;
	}

	//吸附物品
	for (auto begin = goodses_.begin(); begin != goodses_.end();) {
		auto bloodball = *begin;
		auto offset = bloodball->getPosition() - hero_->getPosition();
		if (GOUGU(offset.x, offset.y) < bloodball->getAdsorbDistance_()) {
			bloodball->follow(hero_);
		}

		if (bloodball->isDead())
			begin = goodses_.erase(begin);
		else
			++begin;
	}

	//清理死亡的enemy
	for (auto begin = Enemy::mapTagToEnemy.begin(); begin != Enemy::mapTagToEnemy.end();) {
		if (begin->second->isCleanable()) {
			if (++killCount_ % 10 == 0) {
				level_ += 1;

				this->removeChildByName("level");

				String *lev = String::createWithFormat("Level %d", level_);
				auto label = Label::createWithTTF(lev->getCString(), "fonts/arial.ttf", 15);
				label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height - 20));
				label->setCameraMask(0x2);
				this->addChild(label, 1, "level");
			}
				

			//有概率出现掉落物
			switch (rand() % 5) {
				case 0: {
					goodses_.push_back(BloodBall::create(this, begin->second->getPosition() + Vec2(100, 0)));
				}break;
				case 1: {
					goodses_.push_back(Sword::create(this, begin->second->getPosition() + Vec2(100, 0)));
				}break;
				default:
					break;
			}

			begin = Enemy::mapTagToEnemy.erase(begin);
		}
		else
			++begin;
	}

	//清理死亡的hero
	for (auto begin = Hero::mapTagToHero.begin(); begin != Hero::mapTagToHero.end();) {
		if (begin->second->isCleanable()) {
			//保证已经没有enemy正在follow即将清除的hero,不然enemy会引用销毁的phyBody(hero->phyBody)
			bool noneFollow = true;
			for (const auto e : Enemy::mapTagToEnemy) {
				if (e.second->isFollow() && e.second->getCurFollow() == begin->second) {
					noneFollow = false;
					break;
				}	
			}

			if (noneFollow) {
				this->removeChild(begin->second->getSprite());
				begin = Hero::mapTagToHero.erase(begin);
			}
		}
		else
			++begin;
	}
	
	//创建敌人
	while (bSetSeed_ && Enemy::getEnemyNum() < MaxEnemyNum_) {
		int scale = rand() % level_;

		if (scale == 0)
			scale = 1;
		else if (scale > 3)
			scale = 3;

		auto enemy = Enemy::create(this, Vec2(RANDOM(origin_.x, origin_.x + visibleSize_.width), RANDOM(origin_.y, origin_.y + visibleSize_.height)), scale);
		this->addChild(enemy->getSprite(), 1);
	}

	//敌人仇恨判定
	for (auto e : Enemy::mapTagToEnemy) {
		if (e.second->isFollow())
			continue;

		auto enemy = e.second;

		if (bOnlineGame_) {
			//如果有多个hero在范围内，随机锁定其中一个
			std::map<int, std::shared_ptr<Hero>> heros;
			for (auto h : players_) {
				auto hero = h.second;
				//获取仇恨范围内的hero
				auto d = GOUGU(abs(hero->getPosition().x - enemy->getPosition().x), abs(hero->getPosition().y - enemy->getPosition().y));
				if (d <= enemy->getHateRadius()) {
					heros.emplace(h);
				}
			}

			//随机锁定
			if (heros.size()) {
				int index = std::rand() % heros.size();
				auto iter = heros.begin();
				std::advance(iter, index);
				enemy->follow(iter->second);
			}
		}
		else {
			enemy->follow(hero_);
		}
	}

	//改变位置
	hero_->setPosition(hero_->getPosition() + hero_->getSpeed());

	//人物移动
	hero_->getSprite()->setPosition(hero_->getPosition());

	if (bOnlineGame_) {
		GameMsg msg;
		msg.set_msgtype(GameMsgType::G_POSITION);
		msg.set_x(hero_->getPosition().x);
		msg.set_y(hero_->getPosition().y);
		msg.set_angle(hero_->getAngle());
		session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::GAME_MSG);
	}
	
	//相机移动
	defaultCamera_->setPosition(hero_->getPosition());

	//刷新血条UI
	if (!hero_->isDead()) {
		auto progress = (ProgressTimer*)getChildByName("BloodBar");
		assert(progress->isRunning());
		progress->setPercentage(hero_->getCurBlood() / hero_->getMaxBlood() * 100);
	}
	

	//刷新瞄准线
	hero_->drawAimLine(cursorPosition_);
}

void GameScene::makeBombUpdate(float delta){
	hero_->makeBomb();

	if (bOnlineGame_) {
		GameMsg msg;
		msg.set_msgtype(GameMsgType::G_ATTACK);
		session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::GAME_MSG);
	}
}

bool GameScene::onContactBegin(cocos2d::PhysicsContact &contact){
	auto contactDate = contact.getContactData();
	auto shapeA = contact.getShapeA();
	auto shapeB = contact.getShapeB();

	static int bombTag = 0;

	//获取触发碰撞的人物
	std::shared_ptr<Hero> hero = nullptr;
	if (shapeA->getGroup() == HeroGroup || shapeA->getGroup() == BombGroup) {
		int tag = shapeA->getBody()->getTag();
		if (shapeA->getGroup() == BombGroup)
			tag /= 1000;
		hero = Hero::getHeroByTag(tag);
	}
	else if (shapeB->getGroup() == HeroGroup || shapeB->getGroup() == BombGroup) {
		int tag = shapeB->getBody()->getTag();
		if (shapeB->getGroup() == BombGroup)
			tag /= 1000;
		hero = Hero::getHeroByTag(tag);
	}
	else
		return false;

	if (shapeA->getGroup() == HeroGroup && shapeB->getGroup() == StaticBlockGroup) {
		//--人物,障碍物碰撞
		if (contactDate->normal.y < 0) {
			//hero->setPositionY(hero->getPosition().y + 5);
			hero->setPositionY(contactDate->points[0].y + (hero->getPosition().y - hero->getBodyPosition().y) + 5);
			hero->getDisableDir().D = true;
		}
		else if (contactDate->normal.y > 0) {
			//hero->setPositionY(hero->getPosition().y - 5);
			hero->setPositionY(contactDate->points[0].y + (hero->getPosition().y - hero->getBodyPosition().y) - 5);
			hero->getDisableDir().U = true;
		}
		else if (contactDate->normal.x < 0) {
			hero->setPositionX(contactDate->points[0].x + 15);
			hero->getDisableDir().L = true;
		}
		else if (contactDate->normal.x > 0) {
			hero->setPositionX(contactDate->points[0].x - 15);
			hero->getDisableDir().R = true;
		}

		return true;
	}
	else if (shapeA->getGroup() == BombGroup && shapeB->getGroup() == BombBlockGroup
			 || shapeB->getGroup() == BombGroup && shapeA->getGroup() == BombBlockGroup
			 || shapeA->getGroup() == BombGroup && shapeB->getGroup() == EnemyGroup
			 || shapeB->getGroup() == BombGroup && shapeA->getGroup() == EnemyGroup) {
		//--hero炮弹和enemy或子弹障碍物发生碰撞

		//同一个bomb，只触发一次伤害(通过tag，判断同一个bomb是否触发两次伤害)
		if (shapeA->getGroup() == BombGroup) {
			if (bombTag == shapeA->getBody()->getTag())
				return false;
			bombTag = shapeA->getBody()->getTag();
		}
		else {
			if (bombTag == shapeB->getBody()->getTag())
				return false;
			bombTag = shapeB->getBody()->getTag();
		}
		
		if (shapeA->getGroup() == EnemyGroup || shapeB->getGroup() == EnemyGroup) {
			//只有自己的bomb触发伤害
			if (hero == hero_) {
				//获取受到攻击的enemy
				std::shared_ptr<Enemy> enemy = nullptr;
				if (shapeA->getGroup() == EnemyGroup)
					enemy = Enemy::getEnemyByTag(shapeA->getBody()->getTag());
				else
					enemy = Enemy::getEnemyByTag(shapeB->getBody()->getTag());
				enemy->beHurt(hero->getAtk());

				if (bOnlineGame_) {
					GameMsg msg;
					msg.set_msgtype(GameMsgType::G_ENEMY_HURT);
					msg.set_atk(hero->getAtk());
					msg.set_enemytag(enemy->getTag());
					session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::GAME_MSG);
				}
			}
		}

		//--炮弹碰撞
		this->removeChildByTag(shapeA->getBody()->getTag()); //去除当前炮弹精灵
		//--炮弹爆炸动画
		auto sprite = Sprite::create();
		this->addChild(sprite, 1);

		sprite->setPosition(contactDate->points[0]); //设置炮弹爆炸位置
		auto bombAnimation = AnimationCache::getInstance()->getAnimation("bombAnimation"); //从缓存中加载动画
		auto bombAnimate = Animate::create(bombAnimation);
		sprite->runAction(Sequence::create(bombAnimate,
			CallFunc::create([this, sprite]() { this->removeChild(sprite); }), nullptr)); //播放动画，并销毁精灵

		return false;
	}
	else if (shapeA->getGroup() == EnemyBombGroup && shapeB->getGroup() == HeroGroup
			|| shapeA->getGroup() == EnemyBombGroup && shapeB->getGroup() == HeroGroup) {
		//--enemy炮弹和hero碰撞
		if (hero == hero_)
			hero_->beHurt(1);

		return false;
	}

	return false;
}

void GameScene::onContactSeparate(cocos2d::PhysicsContact & contact){
	auto contactDate = contact.getContactData();
	auto shapeA = contact.getShapeA();
	auto shapeB = contact.getShapeB();

	//获取触发碰撞的人物
	std::shared_ptr<Hero> hero = nullptr;
	if (shapeA->getGroup() == HeroGroup || shapeA->getGroup() == BombGroup) {
		int tag = shapeA->getBody()->getTag();
		if (shapeA->getGroup() == BombGroup)
			tag /= 1000;
		hero = Hero::getHeroByTag(tag);
	}
	else if (shapeB->getGroup() == HeroGroup || shapeB->getGroup() == BombGroup) {
		int tag = shapeB->getBody()->getTag();
		if (shapeB->getGroup() == BombGroup)
			tag /= 1000;
		hero = Hero::getHeroByTag(tag);
	}
	else
		return;

	if (shapeA->getGroup() == HeroGroup && shapeB->getGroup() == StaticBlockGroup) {
		//--人物,障碍物分离
		if (contactDate->normal.y != 0) {
			hero->getDisableDir().D = false;
			hero->getDisableDir().U = false;
		}
		else if (contactDate->normal.x != 0) {
			hero->getDisableDir().L = false;
			hero->getDisableDir().R = false;
		}
	}
}

void GameScene::menuPhyDebug(cocos2d::Ref * ref){
	static bool state = true;
	
	if (state)
		phyWorld_->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_NONE);
	else
		phyWorld_->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

	state = !state;
}