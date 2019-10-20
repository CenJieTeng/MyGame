#include "PrimaryMenuScene.h"
#include "HallScene.h"
#include "RegisterLoginScene.h"
#include "GameScene_1.h"

USING_NS_CC;

cocos2d::Scene * PrimaryMenuScene::createScene()
{
	return PrimaryMenuScene::create();
}

bool PrimaryMenuScene::init()
{
	if (!Scene::init())
		return false;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();

	//创建背景图片
	auto bg_sprite = Sprite::create("PrimaryMenuBg.jpg");
	assert(bg_sprite != nullptr
		&& bg_sprite->getContentSize().width != 0
		&& bg_sprite->getContentSize().height != 0);

	bg_sprite->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
	this->addChild(bg_sprite);

	////进入游戏按钮
	//auto playItem = MenuItemImage::create("UI/play_normal.tga", 
	//									  "UI/play_press.tga",
	//									  CC_CALLBACK_1(PrimaryMenu::menuPlayCallBack, this));
	//assert(playItem != nullptr
	//	&& playItem->getContentSize().width != 0
	//	&& playItem->getContentSize().height != 0);
	//playItem->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 1.5));
	//playItem->setRotation3D(Vec3(0, 180, 180));

	/*Button *button = ui::Button::create("UI/play_normal.tga", "UI/play_press.tga", "UI/play_normal.tga", TextureResType::LOCAL);
	button->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	button->addClickEventListener(CC_CALLBACK_1(PrimaryMenu::menuPlayGame,this));
	this->addChild(button, 1);*/

	//play game
	auto playGameItem = MenuItemFont::create("Play Game",
		CC_CALLBACK_1(PrimaryMenuScene::menuPlayGame, this));
	playGameItem->setFontSizeObj(20);
	playGameItem->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 50);

	//online game
	auto onlineGameItem = MenuItemFont::create("Online Game",
		CC_CALLBACK_1(PrimaryMenuScene::menuOnlineGame, this));
	onlineGameItem->setFontSizeObj(20);
	onlineGameItem->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 10);
	

	//创建菜单
	auto menu = Menu::create(playGameItem, onlineGameItem, nullptr);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu);

	return true;
}

//playGame按钮回调
void PrimaryMenuScene::menuPlayGame(Ref * ref){
	Director::getInstance()->replaceScene(GameScene1::createScene(false));
}

//onlineGame按钮回调
void PrimaryMenuScene::menuOnlineGame(Ref * ref) {
	auto scene = (HallScene*)HallScene::createScene();
	Director::getInstance()->pushScene(scene);
	RegisterLoginScene::setSession(scene->getSession());
	Director::getInstance()->pushScene(RegisterLoginScene::createScene());
}
