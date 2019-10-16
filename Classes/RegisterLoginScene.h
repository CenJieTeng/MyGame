#ifndef REGISTER_LOGIN_SCENE
#define REGISTER_LOGIN_SCENE
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <session.h>

class RegisterLoginScene : public cocos2d::Scene {
public:
	static cocos2d::Scene *createScene();
	virtual bool init();
	bool initNetwork();

	CREATE_FUNC(RegisterLoginScene);

	//切换注册登录回调函数
	void radioRegisterEvent(cocos2d::ui::RadioButton *rb, cocos2d::ui::RadioButton::EventType et);
	void radioLoginEvent(cocos2d::ui::RadioButton *rb, cocos2d::ui::RadioButton::EventType et);

	//注册登录按钮回调函数
	void registerEvent(cocos2d::Ref *ref);
	void loginEvent(cocos2d::Ref *ref);

	static void setSession(session_ptr sp);
private:
	static session_ptr session_;
	cocos2d::Size visibleSize_; ////屏幕可见大小
	cocos2d::ui::Button *registerButton_; //注册按钮
	cocos2d::ui::Button *loginButton_; //登录按钮
	cocos2d::ui::TextField *accountTextField_; //账户输入框
	cocos2d::ui::TextField *passwordTextField_; //密码输入框
};

#endif // !REGISTER_LOGIN_SCENE