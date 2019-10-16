#ifndef HELLOWORLD_SCENE_H
#define HELLOWORLD_SCENE_H

#include "cocos2d.h"

//进入主菜单过度场景
class HelloWorldScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorldScene);
};

#endif // HELLOWORLD_SCENE_H
