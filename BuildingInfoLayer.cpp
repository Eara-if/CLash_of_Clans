#include "BuildingInfoLayer.h"

USING_NS_CC;

bool BuildingInfoLayer::init()
{
    if (!Layer::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 添加一个半透明的黑色背景 (遮罩)
    // 这样弹窗出现时，背景变暗，且无法点击后面的东西
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 150)); // RGBA: 黑色，透明度150
    this->addChild(shieldLayer);

    // 2. 拦截触摸事件 (吞噬点击)
    // 这是一个特殊的监听器，专门防止点击穿透
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true); // 关键：吞噬触摸，不让事件传给下面的房子
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 3. 弹窗背景图 (这里假设你有一张叫 popup_bg.png 的图)
    // 如果没有，可以用 LayerColor::create(Color4B::WHITE, 300, 200) 代替测试
    auto bg = Sprite::create("popup_bg.png");
    if (!bg) {
        // 如果没有图，就做一个白色的方块代替，防止崩溃
        bg = Sprite::create();
        bg->setTextureRect(Rect(0, 0, 400, 300));
        bg->setColor(Color3B::WHITE);
    }
    bg->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    this->addChild(bg);

    // 4. 初始化文字标签 (位置是相对于 bg 背景图的)
    Size bgSize = bg->getContentSize();

    _nameLabel = Label::createWithSystemFont("Name", "Arial", 30);
    _nameLabel->setPosition(bgSize.width / 2, bgSize.height - 40);
    _nameLabel->setTextColor(Color4B::BLACK);
    bg->addChild(_nameLabel);

    _levelLabel = Label::createWithSystemFont("Lv. 1", "Arial", 24);
    _levelLabel->setPosition(bgSize.width / 2, bgSize.height - 80);
    _levelLabel->setTextColor(Color4B::BLACK);
    bg->addChild(_levelLabel);

    _costLabel = Label::createWithSystemFont("Cost: 100", "Arial", 24);
    _costLabel->setPosition(bgSize.width / 2, bgSize.height - 120);
    _costLabel->setTextColor(Color4B::RED);
    bg->addChild(_costLabel);

    // 5. 升级按钮
    // 注意：Menu 要加在 bg 上，或者是 Layer 上。这里为了简单加在 Layer 上，坐标相对于屏幕
    auto upgradeBtn = MenuItemFont::create("UPGRADE", [=](Ref* sender) {
        // 如果设置了回调，就执行它
        if (_upgradeCallback) {
            _upgradeCallback();
        }
        });
    upgradeBtn->setColor(Color3B::BLUE);
    upgradeBtn->setPosition(0, -50); // 相对于 Menu 中心的偏移

    // 6. 关闭按钮
    auto closeBtn = MenuItemFont::create("Close", [=](Ref* sender) {
        this->closeLayer();
        });
    closeBtn->setColor(Color3B::BLACK);
    closeBtn->setPosition(0, -100);

    auto menu = Menu::create(upgradeBtn, closeBtn, NULL);
    // Menu 放在屏幕中心
    menu->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    this->addChild(menu);

    return true;
}

void BuildingInfoLayer::setBuildingData(std::string name, int level, int cost)
{
    _nameLabel->setString(name);
    _levelLabel->setString("Level: " + std::to_string(level));
    _costLabel->setString("Cost: " + std::to_string(cost));
}

void BuildingInfoLayer::setUpgradeCallback(std::function<void()> callback)
{
    _upgradeCallback = callback;
}

void BuildingInfoLayer::closeLayer()
{
    this->removeFromParent();
}