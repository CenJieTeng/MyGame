#pragma once
#ifndef PRIMARYMENU_SCENE_H
#define PRIMARYMENU_SCENE_H
#include "cocos2d.h"

//主菜单场景
class PrimaryMenuScene : public cocos2d::Scene
{
public:
	static cocos2d::Scene* createScene();

	virtual bool init();

	void menuPlayGame(Ref *ref); //play game item call back
	void menuOnlineGame(Ref * ref); //online game item call back

	CREATE_FUNC(PrimaryMenuScene);
};

#endif // !PRIMARYMENU_SCENE_H
