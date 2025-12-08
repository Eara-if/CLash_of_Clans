#include "BuildingInfoLayer.h"
#include"TrainingLayer.h"
#include"Building.h"
#include"GameScene.h"

USING_NS_CC;
extern int army_limit;

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
    bg = Sprite::create("popup_bg.png");
    if (!bg) {
        // 如果没有图，就做一个白色的方块代替，防止崩溃
        bg = Sprite::create();
        bg->setTextureRect(Rect(0, 0, 400, 300));
        bg->setColor(Color3B::WHITE);
    }
    bg->getTexture()->setAliasTexParameters();
    bg->setScale(4.0f);
    bg->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    this->addChild(bg, 0);

    // 创建一个通用的 Label，位置放中间
    _infoLabel = Label::createWithSystemFont("", "Arial", 28);
    _infoLabel->setPosition(centerPos.x, centerPos.y + 30);

    _infoLabel->setTextColor(Color4B::BLACK);
    this->addChild(_infoLabel, 1);

    _actionBtn = MenuItemFont::create("Button", [](Ref*) {});
    _actionBtn->setColor(Color3B::BLUE);

    // 关闭按钮
    auto closeBtn = MenuItemFont::create("Close", [=](Ref*) { this->closeLayer(); });
    closeBtn->setColor(Color3B::RED);
    closeBtn->setPosition(0, -85); // 相对菜单中心的偏移

    menu = Menu::create(_actionBtn, closeBtn, NULL);

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

    // 1. 清理旧的回调
    _actionBtn->setCallback(nullptr);
    // 确保之前的定时器被关闭 (防止反复打开弹窗时定时器堆积)
    this->unschedule("upgrade_timer");

    // 获取数据
    int level = _targetBuilding->getLevel();
    // 确保 Building.h 里有 getNextLevelCost() 并且是 public
    int cost = _targetBuilding->getNextLevelCost();

    // ============================================================
    // 情况 A：金矿 (显示金币生产状态)
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::MINE) {

        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::PRODUCING) {
            // 正在生产中，显示倒计时
            float timeLeft = _targetBuilding->getProductionTimeLeft();
            std::string info = "Gold Mine Lv." + std::to_string(level) +
                "\nProducing...\nTime left: " + std::to_string((int)timeLeft) + "s";
            _infoLabel->setString(info);

            // 设置按钮为不可用或隐藏
            _actionBtn->setString("Producing");
            _actionBtn->setCallback(nullptr);

            // 启动生产倒计时更新
            this->schedule([=](float dt) {
                float remainingTime = _targetBuilding->getProductionTimeLeft();
                if (remainingTime > 0) {
                    std::string info = "Gold Mine Lv." + std::to_string(level) +
                        "\nProducing...\nTime left: " + std::to_string((int)remainingTime) + "s";
                    _infoLabel->setString(info);
                }
                else {
                    this->unschedule("production_timer");
                    // 重新设置建筑信息
                    this->setBuilding(_targetBuilding);
                }
                }, 1.0f, "production_timer");

        }
        else if (state == BuildingState::READY) {
            // 资源可收集
            int amount = _targetBuilding->getProducedAmount();
            std::string info = "Gold Mine Lv." + std::to_string(level) +
                "\nReady to collect!\nAmount: " + std::to_string(amount) + " coins";
            _infoLabel->setString(info);

            // 设置收集按钮
            _actionBtn->setString("Collect");
            _actionBtn->setCallback([=](Ref*) {
                // 收集资源
                _targetBuilding->collectResources();

                // 【新增】更新GameScene中的资源显示
                auto scene = Director::getInstance()->getRunningScene();
                if (scene) {
                    // 尝试获取GameScene层
                    // 假设GameScene是场景的第一个子节点
                    auto children = scene->getChildren();
                    for (auto& child : children) {
                        // 使用dynamic_cast尝试转换为GameScene
                        auto gameScene = dynamic_cast<GameScene*>(child);
                        if (gameScene) {
                            // 调用GameScene的更新资源显示函数
                            gameScene->updateResourceDisplay();
                            break;
                        }
                    }
                }

                this->closeLayer();
                });
        }
        else {
            // 其他状态（升级中或空闲）
            _infoLabel->setString("Gold Mine Lv." + std::to_string(level) +
                "\nCost: " + std::to_string(cost) + " Coin");

            // 设置升级按钮
            _actionBtn->setString("Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                // 1. 开始升级逻辑
                this->handleStartUpgrade();
                });
        }
    }
    // ============================================================
    // 情况 B：圣水收集器 (显示圣水生产状态)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::WATER ) {

        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::PRODUCING) {
            // 正在生产中，显示倒计时
            float timeLeft = _targetBuilding->getProductionTimeLeft();
            std::string info = "Water Collector Lv." + std::to_string(level) +
                "\nProducing...\nTime left: " + std::to_string((int)timeLeft) + "s";
            _infoLabel->setString(info);

            // 设置按钮为不可用或隐藏
            _actionBtn->setString("Producing");
            _actionBtn->setCallback(nullptr);

            // 启动生产倒计时更新
            this->schedule([=](float dt) {
                float remainingTime = _targetBuilding->getProductionTimeLeft();
                if (remainingTime > 0) {
                    std::string info = "Water Collector Lv." + std::to_string(level) +
                        "\nProducing...\nTime left: " + std::to_string((int)remainingTime) + "s";
                    _infoLabel->setString(info);
                }
                else {
                    this->unschedule("production_timer");
                    // 重新设置建筑信息
                    this->setBuilding(_targetBuilding);
                }
                }, 1.0f, "production_timer");

        }
        else if (state == BuildingState::READY) {
            // 资源可收集
            int amount = _targetBuilding->getProducedAmount();
            std::string info = "Water Collector Lv." + std::to_string(level) +
                "\nReady to collect!\nAmount: " + std::to_string(amount) + " water";
            _infoLabel->setString(info);

            // 设置收集按钮
            _actionBtn->setString("Collect");
            _actionBtn->setCallback([=](Ref*) {
                // 收集资源
                _targetBuilding->collectResources();

                // 【新增】更新GameScene中的资源显示
                auto scene = Director::getInstance()->getRunningScene();
                if (scene) {
                    // 尝试获取GameScene层
                    auto children = scene->getChildren();
                    for (auto& child : children) {
                        auto gameScene = dynamic_cast<GameScene*>(child);
                        if (gameScene) {
                            gameScene->updateResourceDisplay();
                            break;
                        }
                    }
                }

                this->closeLayer();
                });
        }
        else {
            // 其他状态（升级中或空闲）
            _infoLabel->setString("Water Collector Lv." + std::to_string(level) +
                "\nCost: " + std::to_string(cost) + " Coin");

            // 设置升级按钮
            _actionBtn->setString("Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                // 1. 开始升级逻辑
                this->handleStartUpgrade();
                });
        }
    }
    // ============================================================
    // 情况 C：兵营 (显示水花费 + 训练按钮)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::BARRACKS &&
        _targetBuilding->getState() == BuildingState::IDLE)
    {
        // 显示信息
        std::string info = "Barracks Lv." + std::to_string(level) +
            "\nCap: " + std::to_string(army_limit) +
            "\nCost: " + std::to_string(cost) + " Water";
        _infoLabel->setString(info);

        // 设置升级按钮
        _actionBtn->setString("Upgrade");
        _actionBtn->setCallback([=](Ref*) {
            // 1. 开始升级逻辑 (包含开启定时器)
            this->handleStartUpgrade();
            });

        // --- 训练按钮 (必须用 Label::createWithTTF 防止崩溃) ---
        auto trainLabel = Label::createWithTTF("Train Troops", "fonts/Marker Felt.ttf", 30);
        trainLabel->setColor(Color3B::GREEN);
        trainLabel->enableOutline(Color4B::BLACK, 2);

        auto trainBtn = MenuItemLabel::create(trainLabel, [=](Ref*) {
            this->closeLayer();
            auto trainingLayer = TrainingLayer::create();
            Director::getInstance()->getRunningScene()->addChild(trainingLayer, 999);
            });
        trainBtn->setPosition(0, -55);
        menu->addChild(trainBtn);
    }
    // ============================================================
    // 情况 D：其他建筑（除了城墙，城墙不可升级） (显示金币花费)
    // ============================================================
    else if (_targetBuilding->getType() != BuildingType::WALL && _targetBuilding->getState() == BuildingState::IDLE)
    {
        // 显示信息
        _infoLabel->setString("Level: " + std::to_string(level) +
            "\nCost: " + std::to_string(cost) + " Coin");

        // 设置升级按钮
        _actionBtn->setString("Upgrade");
        _actionBtn->setCallback([=](Ref*) {
            // 1. 开始升级逻辑
            this->handleStartUpgrade();
            });
    }
    // ============================================================
    // 情况 E：正在升级中 (重新打开弹窗时)
    // ============================================================
    else if (_targetBuilding->getType() != BuildingType::WALL && _targetBuilding->getState() == BuildingState::UPGRADING)
    {
        // 如果一打开发现正在升级，直接进入倒计时监控模式
        this->handleUpgradeTimer();
    }
    else {}

}
// 把它加在 setBuilding 后面
void BuildingInfoLayer::handleStartUpgrade()
{
    // 1. 建筑开始干活
    _targetBuilding->startUpgrade();

    // 2. 启动定时器监控
    this->handleUpgradeTimer();
}
void BuildingInfoLayer::handleUpgradeTimer()
{
    // 1. 立即刷新一次文字
    float time = _targetBuilding->getTimeLeft();
    _infoLabel->setString("Upgrading...\n" + std::to_string((int)time) + "s");

    // 2. 将按钮改为 "Speed Up" (加速)
    _actionBtn->setString("Speed Up");
    _actionBtn->setCallback([=](Ref*) {
        this->unschedule("upgrade_timer"); // 关掉定时器
        _targetBuilding->speedUp();        // 立即完成
        this->closeLayer();                // 关闭弹窗
        });

    // 3. 【核心】开启 Lambda 定时器 (每 0.1 秒刷新一次)
    this->schedule([=](float dt) {

        // A. 获取剩余时间
        float timeLeft = _targetBuilding->getTimeLeft();

        // B. 刷新倒计时文字 (实现动态跳变)
        _infoLabel->setString("Upgrading...\n" + std::to_string((int)timeLeft) + "s");

        // C. 检查是否结束
        if (timeLeft <= 0 || _targetBuilding->getState() == BuildingState::IDLE) {
            this->unschedule("upgrade_timer"); // 关掉定时器
            this->closeLayer();                // 自动关闭
        }

        }, 0.01f, "upgrade_timer");
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
void BuildingInfoLayer::showTrainingMenu()
{
    // 1. 隐藏原来的信息
    _infoLabel->setVisible(false);
    menu->setVisible(false); // 隐藏升级按钮

    // 2. 创建兵种选择菜单
    auto soldierItem = MenuItemImage::create("soldier/arrow.png", "soldier/man.png", [=](Ref*) {
        log("Training Soldier...");
        // 扣水，增加造兵队列...
        });

    auto archerItem = MenuItemImage::create("soldier/arrow.png", "soldier/man.png", [=](Ref*) {
        log("Training Archer...");
        });

    auto trainMenu = Menu::create(soldierItem, archerItem, NULL);
    trainMenu->alignItemsHorizontallyWithPadding(20);
    trainMenu->setPosition(0, 0); // 相对 bg 中心

    // 把这个菜单加到背景图上
    // 注意：这里的 parent 应该是 bg，如果 bg 是局部变量，需要改成成员变量 _bg
    bg->addChild(trainMenu);
}