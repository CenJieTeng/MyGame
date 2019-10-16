#include "HallScene.h"
#include "GameScene_1.h"
#include "RegisterLoginScene.h"
#include "message.pb.h"

USING_NS_CC;
using namespace cocos2d::ui;

cocos2d::Scene *HallScene::createScene() {
	return HallScene::create();
}

HallScene::~HallScene(){

	Session::setCallBack("read", [this](session_ptr sp) {
		readCallBack(std::move(sp));
	});

	//如果在房间中，离开房间
	if (session_->getRoomId() != 0) {
		RoomMsg msg;
		msg.set_msgtype(RoomMsgType::R_LEAVE);
		session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);
	}

	//通知服务器关闭连接, 清理在服务器中的临时数据
	SessionMsg msg;
	msg.set_msgtype(SessionMsgType::S_CLOSE);
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::SESSION_MSG);

	if (threadNetwork_ && threadNetwork_->joinable())
		threadNetwork_->join();
}

bool HallScene::init(){

	if (!Scene::init())
		return false;

	visibleSize_ = Director::getInstance()->getVisibleSize();

	//背景图片
	auto bgSprite = Sprite::create("hall.png");
	bgSprite->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
	bgSprite->setScale(1.6f);
	this->addChild(bgSprite);

	auto label = Label::createWithTTF("RoomId               RoomStatus              NumOfPeople", "fonts/Marker Felt.ttf", 10);
	label->setPosition(visibleSize_.width * 2 / 6, visibleSize_.height - 10);
	this->addChild(label);

	//初始化网络环境
	initNetwork();

	//play game
	playGameItem_ = MenuItemFont::create("Play Game",
		CC_CALLBACK_1(HallScene::menuPlayGame, this));
	playGameItem_->setFontSizeObj(20);
	playGameItem_->setPosition(Vec2(visibleSize_.width * (4 / 5.0), visibleSize_.height - 30));

	//create room
	createRoomItem_ = MenuItemFont::create("Create Room",
		CC_CALLBACK_1(HallScene::menuCreateRoom, this));
	createRoomItem_->setFontSizeObj(20);
	createRoomItem_->setPosition(Vec2(visibleSize_.width * (4 / 5.0), visibleSize_.height - 60));

	//into room
	intoRoomItem_ = MenuItemFont::create("Into Room",
		CC_CALLBACK_1(HallScene::menuIntoRoom, this));
	intoRoomItem_->setFontSizeObj(20);
	intoRoomItem_->setPosition(Vec2(visibleSize_.width * (4 / 5.0), visibleSize_.height - 90));

	//leaave room
	leaveRoomItem_ = MenuItemFont::create("Leave Room",
		CC_CALLBACK_1(HallScene::menuLeaveRoom, this));
	leaveRoomItem_->setFontSizeObj(20);
	leaveRoomItem_->setPosition(Vec2(visibleSize_.width * (4 / 5.0), visibleSize_.height - 120));
	leaveRoomItem_->setEnabled(false);

	auto menu1 = Menu::create(playGameItem_, createRoomItem_, intoRoomItem_, leaveRoomItem_, nullptr);
	menu1->setPosition(Vec2::ZERO);
	this->addChild(menu1);

	//房间列表
	roomList_ = ScrollView::create();
	roomList_->setBounceEnabled(true);
	roomList_->setBackGroundImage("bg.png");
	roomList_->setBackGroundImageOpacity(50);
	roomList_->setScrollBarOpacity(255);
	roomList_->setScrollBarAutoHideEnabled(false); //取消自动隐藏滚动条
	roomList_->setSize(Size(0, visibleSize_.height * 2));
	roomList_->setContentSize(Size(visibleSize_.width * 2 / 3, visibleSize_.height / 2));
	roomList_->setPosition(Vec2(0, visibleSize_.height / 2 - 20));

	menuRoomItem_ = Menu::create();
	roomList_->addChild(menuRoomItem_);
	this->addChild(roomList_);

	//聊天输入框
	textField_ = TextField::create("Click here and input chat msg.", "fonts/arial.ttf", 10);
	textField_->setPosition(Vec2(visibleSize_.width / 2, 30));
	textField_->setMaxLengthEnabled(true);
	textField_->setMaxLength(30); //输入最大长度
	textField_->setCursorEnabled(true); //显示光标
	textField_->setTouchAreaEnabled(true);
	textField_->setTouchSize(Size(140, 15)); //设置进入输入框焦点的触摸范围
	this->addChild(textField_, 1);

	//聊天内容显示列表
	chatMsgList_ = ScrollView::create();
	chatMsgList_->setBounceEnabled(true);
	chatMsgList_->setScrollBarAutoHideEnabled(false);
	chatMsgList_->setBackGroundImage("chatbg500x250.png");
	chatMsgList_->setBackGroundImageOpacity(50);
	chatMsgList_->setScrollBarColor(Color3B(255, 255, 255));
	chatMsgList_->setSize(Size(0, 200));
	chatMsgList_->setContentSize(Size(visibleSize_.width / 2, 80));
	chatMsgList_->setPosition(Vec2(visibleSize_.width / 2, 50));
	chatMsgList_->jumpToBottom(); //默认在低端
	this->addChild(chatMsgList_, 1);

	//键盘监听
	auto keyBoardListener = EventListenerKeyboard::create(); //监听器
	keyBoardListener->onKeyPressed = CC_CALLBACK_2(HallScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyBoardListener, this);

	return true;
}

void HallScene::onEnter(){
	Scene::onEnter();

	Session::setCallBack("read", [this](session_ptr sp) {
		readCallBack(std::move(sp));
	});

	//如果在房间中，离开房间
	if (session_->getRoomId() != 0) {
		RoomMsg msg;
		msg.set_msgtype(RoomMsgType::R_LEAVE);
		session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);

		session_->setRoomId(0);
		createRoomItem_->setEnabled(true);
		leaveRoomItem_->setEnabled(false);
	}
}

bool HallScene::initNetwork(){
	ioc_ = std::shared_ptr<boost::asio::io_context>(new boost::asio::io_context());
	socket_ = std::shared_ptr<Socket>(new Socket(*ioc_));

	Session::setCallBack("connect", [this](session_ptr sp) {
		session_ = std::move(sp);
	});

	Session::setCallBack("read", [this](session_ptr sp) {
		readCallBack(std::move(sp));
	});

	socket_->connect("127.0.0.1", 9999);
	threadNetwork_ = std::shared_ptr<std::thread>(new std::thread([this]() { ioc_->run(); }));

	return true;
}

void HallScene::menuPlayGame(cocos2d::Ref *ref) {
	//开始游戏前，先要创建或加入房间
	if (session_->getRoomId() == 0) {
		auto label = Label::createWithTTF("Create or into room befor play game.", "fonts/arial.ttf", 20);
		label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
		this->addChild(label);
		label->runAction(
			Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
		return;
	}

	RoomMsg msg;
	msg.set_msgtype(RoomMsgType::R_START);
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);
}

void HallScene::menuCreateRoom(cocos2d::Ref * ref){
	RoomMsg msg;
	msg.set_msgtype(RoomMsgType::R_CREATE);
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);
}

void HallScene::menuIntoRoom(cocos2d::Ref * ref){
	//加入房间前先选择一个房间
	if (selectRoomId_ == 0) {
		auto label = Label::createWithTTF("Must select a room.", "fonts/arial.ttf", 20);
		label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
		this->addChild(label);
		label->runAction(
			Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
		return;
	}
	else if (selectRoomId_ == session_->getRoomId()) {
		//已经在该房间中
		auto label = Label::createWithTTF("Already in this room.", "fonts/arial.ttf", 20);
		label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
		this->addChild(label);
		label->runAction(
			Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
		return;
	}

	RoomMsg msg;
	msg.set_msgtype(RoomMsgType::R_INTO);
	msg.set_roomid(selectRoomId_);
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);
}

void HallScene::menuLeaveRoom(cocos2d::Ref * ref){
	RoomMsg msg;
	msg.set_msgtype(RoomMsgType::R_LEAVE);
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);

	session_->setRoomId(0);
	createRoomItem_->setEnabled(true);
	leaveRoomItem_->setEnabled(false);
}

void HallScene::menuSelectRoom(cocos2d::Ref *ref) {
	MenuItem *item = (MenuItem*)(ref);
	selectRoomId_ = std::stoi(item->getName());
}

void HallScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event * event){
	if (keyCode == EventKeyboard::KeyCode::KEY_ENTER) {
		std::string chatmsg = textField_->getString(); //从输入框获取输入内容
		if (!chatmsg.empty()) {
			textField_->setString(""); //清空原来的内容

			SessionMsg msg;
			msg.set_msgtype(SessionMsgType::S_CHAT);
			msg.set_allocated_chatmsg(new std::string(std::move(chatmsg)));
			session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::SESSION_MSG);
		}
	}
}

void HallScene::readCallBack(session_ptr sp){
	switch (sp->getProtoMessageType())
	{
	case ProtoMessageType::SESSION_MSG: {
		SessionMsg msg;
		assert(msg.ParseFromString(sp->getReadmsg()));

		switch (msg.msgtype()) {
			case SessionMsgType::S_CLOSE: {
				ioc_->stop();
			}break;
			case SessionMsgType::S_GETKEY: {
				session_->setKey(msg.key()); //获取key
			}break;
			case SessionMsgType::S_CHAT: {
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
						chatMsgList_->addChild(text, 1);
					}
				});
			}break;
			case SessionMsgType::S_LOGOUT: {
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
					Director::getInstance()->pushScene(RegisterLoginScene::createScene());
				});
			}break;
			default:
				assert(false);
		}
	}break;
	case ProtoMessageType::ROOM_MSG: {
		RoomMsg msg;
		assert(msg.ParseFromString(sp->getReadmsg()));

		switch (msg.msgtype())
		{
		case RoomMsgType::R_CREATE: {
			session_->setRoomId(msg.roomid());
			createRoomItem_->setEnabled(false);
			leaveRoomItem_->setEnabled(true);
		}break;
		case RoomMsgType::R_INTO: {
			if (!msg.result()) {
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
					//加入房间失败，房间不存在 或者 房间已经开始游戏
					auto label = Label::createWithTTF("Into room fail", "fonts/arial.ttf", 20);
					label->setColor(Color3B::RED);
					label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
					this->addChild(label);
					label->runAction(
						Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
				});
				return;
			}
			else {
				session_->setRoomId(msg.roomid());
				createRoomItem_->setEnabled(false);
				leaveRoomItem_->setEnabled(true);
			}
		}break;
		case RoomMsgType::R_START: {
			//跳转场景
			Director::getInstance()->getScheduler()->performFunctionInCocosThread([this]() {
				GameScene::setSession(session_);
				Director::getInstance()->pushScene(GameScene1::createScene());

				//通知服务器准备完成
				RoomMsg msg;
				msg.set_msgtype(RoomMsgType::R_READY);
				session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::ROOM_MSG);
			});
		}break;
		case RoomMsgType::R_ROOMLIST: {
			Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
				menuRoomItem_->removeAllChildren();
				MenuItemFont::setFontSize(10);
				for (int i = 0; i < msg.roomlist_size(); ++i) {
					int roomId = msg.roomlist(i);
					char *status = msg.roomstatuses(i) ? "running" : "wait";
					int num = msg.roomnumofpeople(i);
					String *str = String::createWithFormat("RoomId:%d        %s           %d/2", roomId, status, num);
					auto roomItem = MenuItemFont::create(str->getCString(), CC_CALLBACK_1(HallScene::menuSelectRoom, this));
					roomItem->setFontSize(10);
					roomItem->setPosition(-90, visibleSize_.height * 3 / 2 - (i + 1) * 20);
					roomItem->setName(std::to_string(roomId));
					//高亮所在的房间
					if (session_->getRoomId() == roomId)
						roomItem->setColor(Color3B::YELLOW);
					menuRoomItem_->addChild(roomItem);
				}
			});
		}break;
		default:
			CCASSERT(false, "Unknow room control type!");
		}
	}break;
	default:
		CCASSERT(false, "Unknow proto message type!");
	}
}