#include "BuildingInfoLayer.h"
#include"Building.h"
USING_NS_CC;

bool BuildingInfoLayer::init()
{
    if (!Layer::init()) 
        return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 centerPos = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
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
    bg->getTexture()->setAliasTexParameters();
    bg->setScale(4.0f);
    bg->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    this->addChild(bg,0);

    // 创建一个通用的 Label，位置放中间
    _infoLabel = Label::createWithSystemFont("", "Arial", 28);
    _infoLabel->setPosition(centerPos.x, centerPos.y + 30);

    _infoLabel->setTextColor(Color4B::BLACK);
    this->addChild(_infoLabel,1);

    _actionBtn = MenuItemFont::create("Button", [](Ref*) {});
    _actionBtn->setColor(Color3B::BLUE);

    // 关闭按钮
    auto closeBtn = MenuItemFont::create("Close", [=](Ref*) { this->closeLayer(); });
    closeBtn->setColor(Color3B::RED);
    closeBtn->setPosition(0, -80); // 相对菜单中心的偏移

    auto menu = Menu::create(_actionBtn, closeBtn, NULL);

    // 【修改点 E】菜单位置也设在屏幕中心稍微往下一点
    menu->setPosition(centerPos.x, centerPos.y - 40);

    // 【修改点 F】添加到 Layer，层级设为 1
    this->addChild(menu, 1);

    this->scheduleUpdate();
    return true;
}

void BuildingInfoLayer::setBuilding(Building* building)
{
    _targetBuilding = building;

    // 根据建筑当前状态，初始化界面
    if (_targetBuilding->getState() == BuildingState::IDLE)
    {
        // 状态 A: 空闲 -> 显示升级按钮
        _infoLabel->setString("Level: " + std::to_string(_targetBuilding->getLevel()) +
            "\nCost: " + std::to_string(_targetBuilding->getNextLevelCost()));
        _isShowingTimer = false;
        _actionBtn->setString("Upgrade");
        _actionBtn->setCallback([=](Ref* sender) {
            // 点击升级
            _targetBuilding->startUpgrade();
            this->closeLayer(); // 关闭弹窗 (或者你可以做成不关闭，立马切换到状态B)
            });
    }
    else
    {
        // 状态 B: 升级中 -> 显示加速按钮
        _actionBtn->setString("Speed Up (" + std::to_string(_targetBuilding->getSpeedUpCost()) + " Gems)");
        _isShowingTimer = true;
        _actionBtn->setCallback([=](Ref* sender) {
            // 点击加速
            _targetBuilding->speedUp();
            this->closeLayer();
            });
    }
}

void BuildingInfoLayer::update(float dt)
{
    if (_targetBuilding)
    {
        // ==============================================================
        // 【核心修复】
        // 只有当“原本在显示倒计时”(_isShowingTimer == true) 
        // 且“现在变回空闲了” (State == IDLE) 时，才自动关闭。
        // ==============================================================
        if (_isShowingTimer == true && _targetBuilding->getState() == BuildingState::IDLE)
        {
            this->closeLayer();
            return;
        }

        // 刷新倒计时文字 (保持不变)
        if (_targetBuilding->getState() == BuildingState::UPGRADING)
        {
            int seconds = (int)_targetBuilding->getRemainingTime();
            char buf[64];
            sprintf(buf, "Upgrading...\n%ds", seconds + 1);
            _infoLabel->setString(buf);

            // 可选：刷新加速价格
            // _actionBtn->setString("Speed Up (" + std::to_string(_targetBuilding->getSpeedUpCost()) + ")");
        }
    }
}

void BuildingInfoLayer::closeLayer()
{
    this->removeFromParent();
}