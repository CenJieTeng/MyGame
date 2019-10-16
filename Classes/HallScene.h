#ifndef HALLSCENE_H
#define HALLSCENE_H
#include "cocos2d.h"
#include "ui\CocosGUI.h"
#include <socket.h>

class HallScene : public cocos2d::Scene {
public:
	static cocos2d::Scene *createScene();
	~HallScene();

	virtual bool init();
	virtual void onEnter();
	bool initNetwork(); //初始化网络
	session_ptr getSession() const { return session_; }

	//菜单事件
	void menuPlayGame(cocos2d::Ref *ref);
	void menuCreateRoom(cocos2d::Ref *ref);
	void menuIntoRoom(cocos2d::Ref *ref);
	void menuLeaveRoom(cocos2d::Ref *ref);
	void menuSelectRoom(cocos2d::Ref *ref);

	//键盘事件
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event);

	CREATE_FUNC(HallScene);

	void readCallBack(session_ptr sp);

private:
	cocos2d::Size visibleSize_; //屏幕可见大小
	cocos2d::MenuItemFont *playGameItem_; //playGame
	cocos2d::MenuItemFont *createRoomItem_; //createRoom
	cocos2d::MenuItemFont *intoRoomItem_; //intoRoom
	cocos2d::MenuItemFont *leaveRoomItem_; //leaveRoom
	cocos2d::ui::ScrollView *roomList_; //房间列表,滚动列表
	cocos2d::ui::TextField *textField_; //聊天输入框
	cocos2d::ui::ScrollView *chatMsgList_; //聊天内容列表,滚动列表
	cocos2d::Menu *menuRoomItem_; //菜单
	int selectRoomId_; //当前选择的房间id
	std::deque<cocos2d::ui::Text*> chatMsgDeque_; //聊天消息队列
	std::shared_ptr<boost::asio::io_context> ioc_ = nullptr;
	std::shared_ptr<Socket> socket_ = nullptr;
	session_ptr session_ = nullptr;
	std::shared_ptr<std::thread> threadNetwork_ = nullptr;
};

#endif // !HALLSCENE_H
