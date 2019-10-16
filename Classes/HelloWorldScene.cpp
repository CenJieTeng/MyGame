#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "PrimaryMenuScene.h"

USING_NS_CC;

Sequence* CreateAction(CallFunc *func = CallFunc::create([]() {})) {
	//淡出动画,并回调
	ActionInterval *action = FadeIn::create(3);
	auto action_back = action->reverse();

	return Sequence::create(DelayTime::create(2),FadeOut::create(3), func, nullptr);
}

Scene* HelloWorldScene::createScene(){
    return HelloWorldScene::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

bool HelloWorldScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        this->addChild(label, 1);

		label->runAction(CreateAction());
    }

    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
        this->addChild(sprite, 0);

		sprite->runAction(CreateAction(CallFunc::create(
		 []() {
			Director::getInstance()->replaceScene(TransitionFade::create(3.0f, PrimaryMenuScene::createScene()));
		 })));
    }

    return true;
}