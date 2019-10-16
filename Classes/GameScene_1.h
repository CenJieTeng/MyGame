#pragma once
#ifndef GAME_SCENE_1_H
#define GAME_SCENE_1_H
#include "GameScene.h"

class GameScene1 : public GameScene
{
public:
	static cocos2d::Scene* createScene();

	virtual bool init();
	CREATE_FUNC(GameScene1);

	void update(float delta);

private:
	cocos2d::Size visibleSize_; //屏幕大小
	cocos2d::Size bgSize_; //背景图片大小
	cocos2d::PhysicsWorld *phyWorld_;
	int countR_;
	int countL_;
	int countU_;
	int countD_;
};


#endif // GAME_SCENE_1_H
