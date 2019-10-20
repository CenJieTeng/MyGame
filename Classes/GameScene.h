#pragma once
#ifndef _GAME_SCENE_H_
#define _GAME_SCENE_H_
#include "cocos2d.h"
#include "Enemy.h"
#include "Hero.h"
#include "Goods.h"
#include "ui/CocosGUI.h"
#include <socket.h>

class Enemy;

//游戏场景公共父类
class GameScene : public cocos2d::Scene
{
public:
	virtual ~GameScene();

	virtual bool init();

	//鼠标事件
	void onMouseMove(cocos2d::Event *event);
	void onMouseDown(cocos2d::Event *event);
	void onMouseUp(cocos2d::Event *event);

	//键盘事件
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event);
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event);

	//碰撞事件
	bool onContactBegin(cocos2d::PhysicsContact &contact); //接触
	void onContactSeparate(cocos2d::PhysicsContact &contact); //分离

	//输入框事件
	void onTextFieldEvent(cocos2d::Ref *ref, cocos2d::ui::TextField::EventType et);

	virtual void update(float delta); //默认刷新函数
	void makeBombUpdate(float delta); //发射子弹刷新函数
	void menuPhyDebug(cocos2d::Ref *ref); //调试模式切换开关
	void setPhyWorld(cocos2d::PhysicsWorld *world) { phyWorld_ = world; }; //设置物理世界
	static void setOnlineGame(bool ol); //设置是否在线游戏
	static void setSession(session_ptr sp);//设置会话

protected:
	std::shared_ptr<Hero> hero_ = nullptr; //玩家对象

private:
	bool initNetwork(); //初始化网络

private:
	static const int MaxEnemyNum_; //最大敌人数量
	static session_ptr session_; //会话
	static bool bOnlineGame_; //是否在线游戏
	bool bSetSeed_; //是否设置随机数种子
	bool isFocused_; //焦点在textField_上
	int level_; //关卡等级
	int killCount_; //击杀数
	cocos2d::Size visibleSize_; //屏幕可见大小
	cocos2d::Vec2 origin_; //原始坐标位置
	cocos2d::Vec2 cursorPosition_; //鼠标位置(相对于人物位置)
	cocos2d::Camera *defaultCamera_; //默认相机
	cocos2d::Camera *uiCamera_; //UI相机
	cocos2d::Node *damageNode_; //伤害统计
	cocos2d::PhysicsWorld *phyWorld_ = nullptr; //保存子场景的物理世界
	cocos2d::ui::TextField *textField_; //聊天输入框
	cocos2d::ui::ScrollView *chatMsgList_; //聊天内容列表,滚动列表
	std::unordered_map<int, std::shared_ptr<Hero>> players_; //key到hero的映射
	std::list<std::shared_ptr<Goods>> goodses_; //各种掉落物
	std::deque<cocos2d::ui::Text*> chatMsgDeque_; //聊天消息队列
};

#endif // !_GAME_SCENE_H_