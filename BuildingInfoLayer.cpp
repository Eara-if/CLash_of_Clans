#include "BuildingInfoLayer.h"
#include"BuildingUpgradeLimits.h"
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
    _infoLabel->setPosition(centerPos.x, centerPos.y + 80);

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

    // 2. 清理菜单中除了关闭按钮之外的其他按钮
    if (menu) {
        // 移除除关闭按钮外的所有子项
        Vector<Node*> children = menu->getChildren();
        for (auto child : children) {
            auto menuItem = dynamic_cast<MenuItem*>(child);
            if (menuItem != _actionBtn && menuItem != nullptr) {
                menu->removeChild(menuItem,1);
            }
        }

        // 始终添加关闭按钮
        auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 30);
        auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) { this->closeLayer(); });
        closeBtn->setColor(Color3B::RED);
        closeBtn->setPosition(0, -85);
        menu->addChild(closeBtn);
    }

    // 获取数据
    int level = _targetBuilding->getLevel();
    int cost = _targetBuilding->getNextLevelCost();
    BuildingState state = _targetBuilding->getState();

    // 获取大本营等级和升级限制信息
    int townHallLevel = 1;
    for (auto& b : g_allPurchasedBuildings) {
        if (b && b->getType() == BuildingType::BASE) {
            townHallLevel = b->getLevel();
            break;
        }
    }

    auto upgradeLimits = BuildingUpgradeLimits::getInstance();
    int maxLevel = upgradeLimits->getMaxLevelForBuilding(_targetBuilding->getType(), townHallLevel);
    bool isMaxLevel = (level >= maxLevel);

    // ============================================================
    // 情况 A：大本营 (Town Hall)
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::BASE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            // 正在升级中
            this->handleUpgradeTimer();
            return;
        }
        else if (state == BuildingState::IDLE) {
            // 【核心修改】显示更详细的信息，包括下一级花费
            std::string info = "Town Hall Lv." + std::to_string(level);

            // 显示升级花费和时间
            if (level < 10) {
                info += "\nNext Level Cost: " + std::to_string(cost) + " Coin";

                // 显示大本营升级解锁信息
                std::string unlockInfo = upgradeLimits->getUnlockInfoForNextTownHallLevel(level);
                info += "\nNew Unlocks:\n" + unlockInfo;

                // 【新增】显示下一级建筑升级上限
                auto nextLevelMaxLevels = upgradeLimits->getMaxLevelsForTownHall(level + 1);
                auto currentLevelMaxLevels = upgradeLimits->getMaxLevelsForTownHall(level);
            } 
            else {
                info += "\n\nMAX LEVEL REACHED!";
            }

            _infoLabel->setString(info);
            _infoLabel->setDimensions(380, 0); // 设置宽度自动换行

            // 设置升级按钮
            _actionBtn->setString(level >= 10 ? "MAX" : "Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                if (level >= 10) {
                    // 达到最大等级，显示提示
                    auto scene = Director::getInstance()->getRunningScene();
                    auto visibleSize = Director::getInstance()->getVisibleSize();

                    std::string message = "Town Hall has reached maximum level!";
                    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 28);
                    label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
                    label->setColor(Color3B::ORANGE);
                    scene->addChild(label, 1000);
                    label->runAction(Sequence::create(
                        DelayTime::create(2.0f),
                        RemoveSelf::create(),
                        nullptr
                    ));
                }
                else {
                    // 开始升级逻辑
                    this->handleStartUpgrade();
                }
                });
        }
    }
    // ============================================================
    // 金矿和圣水收集器的处理逻辑
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::MINE ||
        _targetBuilding->getType() == BuildingType::WATER) {

        std::string buildingName = (_targetBuilding->getType() == BuildingType::MINE) ?
            "Gold Mine" : "Water Collector";
        std::string resourceType = (_targetBuilding->getType() == BuildingType::MINE) ?
            "coins" : "water";

        switch (state) {
            case BuildingState::PRODUCING: {
                // 状态1：正在生产
                float timeLeft = _targetBuilding->getProductionTimeLeft();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nProducing...\nTime left: " + std::to_string((int)timeLeft) + "s";
                _infoLabel->setString(info);

                // 设置_actionBtn为不可点击的Producing按钮
                _actionBtn->setString("Producing");
                _actionBtn->setColor(Color3B::GRAY);
                _actionBtn->setCallback(nullptr); // 不可点击

                // 启动定时器更新生产时间
                this->schedule([=](float dt) {
                    float remainingTime = _targetBuilding->getProductionTimeLeft();
                    if (remainingTime > 0) {
                        std::string info = buildingName + " Lv." + std::to_string(level) +
                            "\nProducing...\nTime left: " + std::to_string((int)remainingTime) + "s";
                        _infoLabel->setString(info);
                    }
                    else {
                        // 生产完成，自动刷新UI
                        this->unschedule("production_timer");
                        this->setBuilding(_targetBuilding);
                    }
                    }, 1.0f, "production_timer");
                break;
            }

            case BuildingState::READY: {
                // 状态2：生产完成，可收集
                int amount = _targetBuilding->getProducedAmount();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nReady to collect!\nAmount: " + std::to_string(amount) + " " + resourceType;
                _infoLabel->setString(info);

                // 设置_actionBtn为Collect按钮
                _actionBtn->setString("Collect");
                _actionBtn->setColor(Color3B::GREEN);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->collectResources();
                    this->closeLayer();
                    });
                break;
            }

            case BuildingState::IDLE: {
                // 状态3：闲置，可选择生产或升级
                int nextCost = _targetBuilding->getNextLevelCost();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nIdle\nUpgrade cost: " + std::to_string(nextCost) +
                    (_targetBuilding->getType() == BuildingType::MINE ? " coins" : " water");
                _infoLabel->setString(info);

                // 设置_actionBtn为Produce按钮
                _actionBtn->setString("Produce");
                _actionBtn->setColor(Color3B::YELLOW);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->startProduction();
                    // 刷新UI以显示生产状态
                    this->setBuilding(_targetBuilding);
                    });

                // 添加额外的Upgrade按钮到菜单
                auto upgradeLabel = Label::createWithTTF("Upgrade", "fonts/Marker Felt.ttf", 30);
                upgradeLabel->setColor(Color3B::ORANGE);
                auto upgradeBtn = MenuItemLabel::create(upgradeLabel, [=](Ref*) {
                    // 检查最大等级
                    int townHallLevel = 1;
                    for (auto& b : g_allPurchasedBuildings) {
                        if (b && b->getType() == BuildingType::BASE) {
                            townHallLevel = b->getLevel();
                            break;
                        }
                    }

                    auto upgradeLimits = BuildingUpgradeLimits::getInstance();
                    int maxLevel = upgradeLimits->getMaxLevelForBuilding(
                        _targetBuilding->getType(), townHallLevel);

                    if (level >= maxLevel) {
                        this->showMaxLevelWarning(buildingName, townHallLevel);
                    }
                    else {
                        _targetBuilding->startUpgrade();
                        // 刷新UI以显示升级状态
                        this->setBuilding(_targetBuilding);
                    }
                    });
                upgradeBtn->setPosition(0, -40);
                menu->addChild(upgradeBtn);
                break;
            }

            case BuildingState::UPGRADING: {
                // 状态4：正在升级
                float timeLeft = _targetBuilding->getTimeLeft();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nUpgrading...\nTime left: " + std::to_string((int)timeLeft) + "s";
                _infoLabel->setString(info);

                // 设置_actionBtn为Speed Up按钮
                _actionBtn->setString("Speed Up");
                _actionBtn->setColor(Color3B::BLUE);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->speedUp();
                    this->closeLayer();
                    });

                // 启动定时器更新升级时间
                this->schedule([=](float dt) {
                    float remainingTime = _targetBuilding->getTimeLeft();
                    if (remainingTime > 0) {
                        std::string info = buildingName + " Lv." + std::to_string(level) +
                            "\nUpgrading...\nTime left: " + std::to_string((int)remainingTime) + "s";
                        _infoLabel->setString(info);
                    }
                    else {
                        // 升级完成，自动刷新UI
                        this->unschedule("upgrade_timer");
                        this->setBuilding(_targetBuilding);
                    }
                    }, 0.01f, "upgrade_timer");
                break;
            }
        }
    }
    // ============================================================
    // 情况 D：兵营 (Barracks)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::BARRACKS) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // 显示信息
            std::string info = "Barracks Lv." + std::to_string(level) +
                "\nCapacity: " + std::to_string(army_limit) +
                "\nCost: " + std::to_string(cost) + " Water" +
                "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

            if (isMaxLevel) {
                info = "Barracks Lv." + std::to_string(level) + " (MAX)" +
                    "\nUpgrade TH to increase level limit";
            }

            _infoLabel->setString(info);

            // 设置升级按钮
            _actionBtn->setString(isMaxLevel ? "MAX" : "Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                if (isMaxLevel) {
                    this->showMaxLevelWarning("Barracks", townHallLevel);
                }
                else {
                    this->handleStartUpgrade();
                }
                });

            // --- 训练按钮 ---
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
    }
    // ============================================================
    // 情况 E：防御塔 (Archer Tower)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::DEFENSE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // 首先检查是否解锁（大本营等级是否足够）
            bool isUnlocked = townHallLevel >= 3; // 防御塔需要TH3解锁

            std::string info;
            if (!isUnlocked) {
                info = "Archer Tower Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3";
            }
            else {
                info = "Archer Tower Lv." + std::to_string(level) +
                    "\nCost: " + std::to_string(cost) + " Coin" +
                    "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

                if (isMaxLevel) {
                    info = "Archer Tower Lv." + std::to_string(level) + " (MAX)" +
                        "\nUpgrade TH to increase level limit";
                }
            }

            _infoLabel->setString(info);

            // 设置升级按钮
            if (!isUnlocked) {
                _actionBtn->setString("LOCKED");
                _actionBtn->setCallback([=](Ref*) {
                    this->showLockedWarning("Archer Tower", 3);
                    });
            }
            else if (isMaxLevel) {
                _actionBtn->setString("MAX");
                _actionBtn->setCallback([=](Ref*) {
                    this->showMaxLevelWarning("Archer Tower", townHallLevel);
                    });
            }
            else {
                _actionBtn->setString("Upgrade");
                _actionBtn->setCallback([=](Ref*) {
                    this->handleStartUpgrade();
                    });
            }
        }
    }
    // ============================================================
    // 情况 F：加农炮 (Cannon) 
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::CANNON) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // 首先检查是否解锁（大本营等级是否足够）
            bool isUnlocked = townHallLevel >= 4; // 加农炮需要TH4解锁

            std::string info;
            if (!isUnlocked) {
                info = "Cannon Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 4";
            }
            else {
                info = "Cannon Lv." + std::to_string(level) +
                    "\nCost: " + std::to_string(cost) + " Coin" +
                    "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

                if (isMaxLevel) {
                    info = "Cannon Lv." + std::to_string(level) + " (MAX)" +
                        "\nUpgrade TH to increase level limit";
                }
            }

            _infoLabel->setString(info);

            // 设置升级按钮
            if (!isUnlocked) {
                _actionBtn->setString("LOCKED");
                _actionBtn->setCallback([=](Ref*) {
                    this->showLockedWarning("Cannon", 4);
                    });
            }
            else if (isMaxLevel) {
                _actionBtn->setString("MAX");
                _actionBtn->setCallback([=](Ref*) {
                    this->showMaxLevelWarning("Cannon", townHallLevel);
                    });
            }
            else {
                _actionBtn->setString("Upgrade");
                _actionBtn->setCallback([=](Ref*) {
                    this->handleStartUpgrade();
                    });
            }
        }
    }
    // ============================================================
    // 情况 G：城墙 (Wall)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::WALL) {
        // 城墙通常不可升级，或者有特殊的升级方式
        std::string info = "Wall\nDefensive structure\nCannot be upgraded directly";

        _infoLabel->setString(info);

        // 城墙没有升级按钮，只显示信息
        _actionBtn->setString("Close");
        _actionBtn->setCallback([=](Ref*) {
            this->closeLayer();
            });
    }
    // ============================================================
    // 情况 H：金币存储器 (Gold Storage)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::GOLD_STORAGE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // 首先检查是否解锁（大本营等级是否足够）
            bool isUnlocked = townHallLevel >= 3; // 存储器需要TH3解锁

            std::string info;
            if (!isUnlocked) {
                info = "Gold Storage Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3" +
                    "\nIncreases coin capacity";
            }
            else {
                // 显示容量信息（假设每级增加1500容量）
                int storageCapacity = 1500 * level;
                info = "Gold Storage Lv." + std::to_string(level) +
                    "\nCapacity: " + std::to_string(storageCapacity) + " coins" +
                    "\nCost: " + std::to_string(cost) + " Coin" +
                    "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

                if (isMaxLevel) {
                    info = "Gold Storage Lv." + std::to_string(level) + " (MAX)" +
                        "\nCapacity: " + std::to_string(storageCapacity) + " coins" +
                        "\nUpgrade TH to increase level limit";
                }
            }

            _infoLabel->setString(info);

            // 设置升级按钮
            if (!isUnlocked) {
                _actionBtn->setString("LOCKED");
                _actionBtn->setCallback([=](Ref*) {
                    this->showLockedWarning("Gold Storage", 3);
                    });
            }
            else if (isMaxLevel) {
                _actionBtn->setString("MAX");
                _actionBtn->setCallback([=](Ref*) {
                    this->showMaxLevelWarning("Gold Storage", townHallLevel);
                    });
            }
            else {
                _actionBtn->setString("Upgrade");
                _actionBtn->setCallback([=](Ref*) {
                    this->handleStartUpgrade();
                    });
            }
        }
    }
    // ============================================================
    // 情况 I：圣水存储器 (Water Storage)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::WATER_STORAGE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // 首先检查是否解锁（大本营等级是否足够）
            bool isUnlocked = townHallLevel >= 3; // 存储器需要TH3解锁

            std::string info;
            if (!isUnlocked) {
                info = "Water Storage Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3" +
                    "\nIncreases water capacity";
            }
            else {
                // 显示容量信息（假设每级增加1500容量）
                int storageCapacity = 1500 * level;
                info = "Water Storage Lv." + std::to_string(level) +
                    "\nCapacity: " + std::to_string(storageCapacity) + " water" +
                    "\nCost: " + std::to_string(cost) + " Coin" +
                    "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

                if (isMaxLevel) {
                    info = "Water Storage Lv." + std::to_string(level) + " (MAX)" +
                        "\nCapacity: " + std::to_string(storageCapacity) + " water" +
                        "\nUpgrade TH to increase level limit";
                }
            }

            _infoLabel->setString(info);

            // 设置升级按钮
            if (!isUnlocked) {
                _actionBtn->setString("LOCKED");
                _actionBtn->setCallback([=](Ref*) {
                    this->showLockedWarning("Water Storage", 3);
                    });
            }
            else if (isMaxLevel) {
                _actionBtn->setString("MAX");
                _actionBtn->setCallback([=](Ref*) {
                    this->showMaxLevelWarning("Water Storage", townHallLevel);
                    });
            }
            else {
                _actionBtn->setString("Upgrade");
                _actionBtn->setCallback([=](Ref*) {
                    this->handleStartUpgrade();
                    });
            }
        }
    }
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

        // 如果建筑从PRODUCING状态变为READY（生产完成），自动刷新UI
        if (_targetBuilding->getState() == BuildingState::READY) {
            this->setBuilding(_targetBuilding);
        }

        // 刷新倒计时文字 (保持不变)
        if (_targetBuilding->getState() == BuildingState::UPGRADING)
        {
            int seconds = (int)_targetBuilding->getRemainingTime();
            char buf[64];
            sprintf(buf, "Upgrading...\n%ds", seconds + 1);
            _infoLabel->setString(buf);
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

void BuildingInfoLayer::showMaxLevelWarning(const std::string& buildingName, int townHallLevel)
{
    auto scene = Director::getInstance()->getRunningScene();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    std::string message = buildingName + " has reached maximum level for TH" +
        std::to_string(townHallLevel);
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 28);
    label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    label->setColor(Color3B::ORANGE);
    scene->addChild(label, 1000);
    label->runAction(Sequence::create(
        DelayTime::create(2.0f),
        RemoveSelf::create(),
        nullptr
    ));

    // 显示升级解锁信息
    if (townHallLevel < 10) {
        auto upgradeLimits = BuildingUpgradeLimits::getInstance();
        std::string unlockInfo = upgradeLimits->getUnlockInfoForNextTownHallLevel(townHallLevel);
        auto infoLabel = Label::createWithTTF(unlockInfo, "fonts/Marker Felt.ttf", 24);
        infoLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
        infoLabel->setColor(Color3B::YELLOW);
        infoLabel->setDimensions(400, 0);
        infoLabel->setAlignment(TextHAlignment::CENTER);
        scene->addChild(infoLabel, 1000);
        infoLabel->runAction(Sequence::create(
            DelayTime::create(3.0f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}

void BuildingInfoLayer::showLockedWarning(const std::string& buildingName, int requiredTHLevel)
{
    auto scene = Director::getInstance()->getRunningScene();
    auto visibleSize = Director::getInstance()->getVisibleSize();

    std::string message = buildingName + " requires Town Hall Level " +
        std::to_string(requiredTHLevel);
    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 28);
    label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    label->setColor(Color3B::RED);
    scene->addChild(label, 1000);
    label->runAction(Sequence::create(
        DelayTime::create(2.5f),
        RemoveSelf::create(),
        nullptr
    ));
}