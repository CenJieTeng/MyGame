#include "RegisterLoginScene.h"
#include "message.pb.h"

USING_NS_CC;
using namespace cocos2d::ui;

void RegisterLoginScene::setSession(session_ptr sp) {
	session_ = sp;
}
session_ptr RegisterLoginScene::session_ = nullptr;

cocos2d::Scene * RegisterLoginScene::createScene(){
	return RegisterLoginScene::create();
}

bool RegisterLoginScene::init(){

	if (!Scene::init()) {
		return false;
	}

	visibleSize_ = Director::getInstance()->getVisibleSize();

	//初始化网络环境
	initNetwork();

	//背景图片
	Sprite *bgSprite = Sprite::create("hall.png");
	bgSprite->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
	bgSprite->setScale(1.6f);
	this->addChild(bgSprite, 1);

	//注册&登录背景
	Sprite *bgSprite2 = Sprite::create("RegisterLoginBg.jpg");
	bgSprite2->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
	bgSprite2->setScale(1.6f);
	this->addChild(bgSprite2, 1);

	//切换注册按钮
	auto radioRegisterButton = RadioButton::create("SelectRegisterLoginDisable.png", "SelectRegisterLogin.png");
	radioRegisterButton->setAnchorPoint(Vec2::ZERO);
	radioRegisterButton->setPosition(
		Vec2(0, bgSprite2->getContentSize().height - radioRegisterButton->getContentSize().height));
	radioRegisterButton->addEventListener(CC_CALLBACK_2(RegisterLoginScene::radioRegisterEvent, this));
	{
		auto label = Label::createWithTTF("Register", "fonts/arial.ttf", 8);
		label->setPosition(Vec2(radioRegisterButton->getContentSize().width / 2, radioRegisterButton->getContentSize().height / 2));
		radioRegisterButton->addChild(label, 1);
	}
	bgSprite2->addChild(radioRegisterButton, 1);

	//切换登录按钮
	auto radioLoginButton = RadioButton::create("SelectRegisterLoginDisable.png", "SelectRegisterLogin.png");
	radioLoginButton->setAnchorPoint(Vec2::ZERO);
	radioLoginButton->setPosition(
		Vec2(radioLoginButton->getContentSize().width, bgSprite2->getContentSize().height - radioLoginButton->getContentSize().height));
	radioLoginButton->addEventListener(CC_CALLBACK_2(RegisterLoginScene::radioLoginEvent, this));
	{
		auto label = Label::createWithTTF("Login", "fonts/arial.ttf", 8);
		label->setPosition(Vec2(radioRegisterButton->getContentSize().width / 2, radioRegisterButton->getContentSize().height / 2));
		radioLoginButton->addChild(label, 1);
	}
	bgSprite2->addChild(radioLoginButton, 1);

	//单选按钮组
	auto buttonGroup = RadioButtonGroup::create();
	buttonGroup->addRadioButton(radioRegisterButton);
	buttonGroup->addRadioButton(radioLoginButton);
	this->addChild(buttonGroup);

	//注册按钮
	registerButton_ = Button::create("RegisterLoginButton.jpg");
	registerButton_->setPosition(Vec2(bgSprite2->getContentSize().width / 2, 20));
	registerButton_->setTitleFontSize(7);
	registerButton_->setTitleText("Register");
	registerButton_->addClickEventListener(CC_CALLBACK_1(RegisterLoginScene::registerEvent, this));
	bgSprite2->addChild(registerButton_, 1);

	//登录按钮
	loginButton_ = Button::create("RegisterLoginButton.jpg");
	loginButton_->setPosition(Vec2(bgSprite2->getContentSize().width / 2, 20));
	loginButton_->setTitleFontSize(7);
	loginButton_->setTitleText("Login");
	loginButton_->setVisible(false);
	loginButton_->addClickEventListener(CC_CALLBACK_1(RegisterLoginScene::loginEvent, this));
	bgSprite2->addChild(loginButton_, 1);

	//账户输入框
	accountTextField_ = TextField::create("account", "fonts/arial.ttf", 8);
	accountTextField_->setTextColor(Color4B::BLUE);
	accountTextField_->setMaxLengthEnabled(true);
	accountTextField_->setMaxLength(15);
	accountTextField_->setCursorEnabled(true);
	accountTextField_->setTouchAreaEnabled(true);
	accountTextField_->setTouchSize(Size(82, 12));
	accountTextField_->setPosition(Vec2(bgSprite2->getContentSize().width / 2, 65));
	bgSprite2->addChild(accountTextField_, 1);

	//密码输入框
	passwordTextField_ = TextField::create("password", "fonts/arial.ttf", 8);
	passwordTextField_->setTextColor(Color4B::BLUE);
	passwordTextField_->setMaxLengthEnabled(true);
	passwordTextField_->setMaxLength(15);
	passwordTextField_->setCursorEnabled(true);
	passwordTextField_->setPasswordEnabled(true);
	passwordTextField_->setTouchAreaEnabled(true);
	passwordTextField_->setTouchSize(Size(82, 12));
	passwordTextField_->setPasswordStyleText("=");
	passwordTextField_->setPosition(Vec2(bgSprite2->getContentSize().width / 2, 49));
	bgSprite2->addChild(passwordTextField_, 1);

	return true;
}

bool RegisterLoginScene::initNetwork(){

	Session::setCallBack("read", [this](session_ptr sp) {
		switch (sp->getProtoMessageType())
		{
			case ProtoMessageType::SESSION_MSG: {
				SessionMsg msg;
				assert(msg.ParseFromString(sp->getReadmsg()));

				switch (msg.msgtype()) {
					case SessionMsgType::S_REGISTER: {
						Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
							String *str = String::createWithFormat("Register %s", msg.result() ? "sucess" : "fail. The account already exist!");
							auto label = Label::createWithTTF(str->getCString(), "fonts/arial.ttf", 20);
							label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
							label->setColor(msg.result() ? Color3B::YELLOW : Color3B::RED);
							this->addChild(label, 1);
							label->runAction(
								Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
						});
					}break;
					case SessionMsgType::S_LOGIN: {
						Director::getInstance()->getScheduler()->performFunctionInCocosThread([msg, this]() {
							String *str = String::createWithFormat("Login %s", msg.result() ? "sucess" : "fail. The account or password is incorrect!");
							auto label = Label::createWithTTF(str->getCString(), "fonts/arial.ttf", 20);
							label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
							label->setColor(msg.result() ? Color3B::YELLOW : Color3B::RED);
							this->addChild(label, 1);
							label->runAction(
								Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));

							if (msg.result()) {
								this->onExit();
								Director::getInstance()->popScene();
							}
						});
					}break;
					default:
					assert(false);
				}
			}break;
			default:
				assert(false);
		}
	});

	return true;
}

void RegisterLoginScene::radioRegisterEvent(RadioButton *rb, RadioButton::EventType et){
	if (et == RadioButton::EventType::SELECTED) {
		registerButton_->setVisible(true);
		loginButton_->setVisible(false);
	}
}

void RegisterLoginScene::radioLoginEvent(RadioButton *rb, RadioButton::EventType et){
	if (et == RadioButton::EventType::SELECTED) {
		registerButton_->setVisible(false);
		loginButton_->setVisible(true);
	}
}

void RegisterLoginScene::registerEvent(Ref *ref){
	//检测账号密码是否正确输入
	std::string account = accountTextField_->getString();
	std::string password = passwordTextField_->getString();
	if (account.empty() || password.empty()) {
		auto label = Label::createWithTTF("Account and password can't be empty.", "fonts/arial.ttf", 20);
		label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
		label->setColor(Color3B::RED);
		this->addChild(label, 1);
		label->runAction(
			Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
		return;
	}

	//一系列检查
	//.......
	//

	SessionMsg msg;
	msg.set_msgtype(SessionMsgType::S_REGISTER);
	msg.set_allocated_account(new std::string(std::move(account)));
	msg.set_allocated_password(new std::string(std::move(password)));
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::SESSION_MSG);
}

void RegisterLoginScene::loginEvent(Ref *ref){
	//检测账号密码是否正确输入
	std::string account = accountTextField_->getString();
	std::string password = passwordTextField_->getString();
	if (account.empty() || password.empty()) {
		auto label = Label::createWithTTF("Account and password can't be empty.", "fonts/arial.ttf", 20);
		label->setPosition(Vec2(visibleSize_.width / 2, visibleSize_.height / 2));
		label->setColor(Color3B::RED);
		this->addChild(label, 1);
		label->runAction(
			Sequence::create(FadeOut::create(2), CallFunc::create([label, this]() { this->removeChild(label); }), nullptr));
		return;
	}

	SessionMsg msg;
	msg.set_msgtype(SessionMsgType::S_LOGIN);
	msg.set_allocated_account(new std::string(std::move(account)));
	msg.set_allocated_password(new std::string(std::move(password)));
	session_->doWrite(message::serialize(msg), MessageType::PROTO, ProtoMessageType::SESSION_MSG);
}