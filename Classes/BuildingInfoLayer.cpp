//用于实现点击建筑之后出现的显示属性、可供升级的弹窗
#include "BuildingInfoLayer.h"
#include "BuildingUpgradeLimits.h"
#include "TrainingLayer.h"
#include "Building.h"
#include "GameScene.h"

USING_NS_CC;
extern int army_limit;
bool BuildingInfoLayer::init()
{
    if (!Layer::init())//先调用父类
        return false;
    //计算屏幕中心的位置
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 centerPos = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    //创建一个半透明的黑色底，Alpha值透明度150    
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(shieldLayer);


    auto listener = EventListenerTouchOneByOne::create();//创建一个监听器
    listener->setSwallowTouches(true); // 吞噬触摸事件，避免在此界面误点击到下面的建筑
    //Lambda表达式，表示接受这次触摸
    listener->onTouchBegan = [](Touch* t, Event* e) {
        return true;
        };
    //管理监听器
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    //创建一个弹窗背景板
    bg = Sprite::create("popup_bg.png");
    //为了防止资源文件出现意外，备用方案创建一个白色方块
    if (!bg) {
        bg = Sprite::create();
        bg->setTextureRect(Rect(0, 0, 400, 300));
        bg->setColor(Color3B::WHITE);
    }
    bg->getTexture()->setAliasTexParameters(); //保证图片的清晰
    bg->setScale(5.0f);  //放大
    bg->setPosition(centerPos);   //放在中心
    this->addChild(bg, 0);       //放在底下图层

    //创建一段文字标签，初始化为空
    _infoLabel = Label::createWithSystemFont("", "Arial", 28);
    _infoLabel->setPosition(centerPos.x, centerPos.y + 80);
    _infoLabel->setTextColor(Color4B::BLACK);
    this->addChild(_infoLabel, 1);

    //创建一个功能按钮
    _actionBtn = MenuItemFont::create("Button", [](Ref*) {});
    _actionBtn->setColor(Color3B::BLUE);

    //cocos中按钮必须放在menu中才可以点击
    menu = Menu::create(_actionBtn, NULL);
    menu->setPosition(centerPos.x, centerPos.y - 40);
    this->addChild(menu, 1);

    return true;
}
//根据不同的建筑类型，动态改变界面的信息和功能
void BuildingInfoLayer::setBuilding(Building* building)
{
    //保存一下当前正在操作的建筑指针
    _targetBuilding = building;

    // 1.清空回调，停止旧的计时器
    //把上一次的残留清理干净
    _actionBtn->setCallback(nullptr);
    this->unschedule("upgrade_timer");

    // 2. 重置按钮菜单
    if (menu) {
        //遍历当前菜单上存在的所有按钮
        Vector<Node*> children = menu->getChildren();
        for (auto child : children) {
            auto menuItem = dynamic_cast<MenuItem*>(child);   //动态转换，如果不是按钮则返回一个空指针
            if (menuItem != _actionBtn && menuItem != nullptr) {   //保留主功能按钮
                menu->removeChild(menuItem, 1);       //移除
            }
        }
        //创建关闭按钮
        auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 30);
        auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) {
            this->closeLayer();
            });     //接受点击，如果点击了调用closeLayer关闭弹窗
        closeBtn->setColor(Color3B::RED);
        closeBtn->setPosition(0, -85);
        menu->addChild(closeBtn);
    }

    // 获取当前正在操作的建筑的一些属性：等级、升级花费、状态
    int level = _targetBuilding->getLevel();
    int cost = _targetBuilding->getNextLevelCost();
    BuildingState state = _targetBuilding->getState();

    // 首先找到大本营的等级
    int townHallLevel = 1;
    for (auto& b : g_allPurchasedBuildings) {
        if (b && b->getType() == BuildingType::BASE) {
            townHallLevel = b->getLevel();
            break;
        }
    }
    //检查该建筑是否达到了当前状态下的满级
    auto upgradeLimits = BuildingUpgradeLimits::getInstance();
    int maxLevel = upgradeLimits->getMaxLevelForBuilding(_targetBuilding->getType(), townHallLevel);
    bool isMaxLevel = (level >= maxLevel);

    // ============================================================
    //  A 大本营 (Town Hall)
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::BASE) {
        BuildingState state = _targetBuilding->getState();
        //判断当前状态
        if (state == BuildingState::UPGRADING) {
            // 正在升级中，调用改变成相关信息的函数
            this->handleUpgradeTimer();
            return;
        }
        else if (state == BuildingState::IDLE) {

            std::string info = "Town Hall Lv." + std::to_string(level);
            //如果还没满级
            if (level < 10) {
                info += "\nNext Level Cost: " + std::to_string(cost) + " Coin";

                // 显示升级下一级需要多少钱
                std::string unlockInfo = upgradeLimits->getUnlockInfoForNextTownHallLevel(level);
                info += "\nNew Unlocks:\n" + unlockInfo;
            }
            else {
                info += "\n\nMAX LEVEL REACHED!";
            }

            _infoLabel->setString(info);
            _infoLabel->setDimensions(380, 0); //将拼接好的字符串设置标签

            // 根据是否满级设定不同文字
            _actionBtn->setString(level >= 10 ? "MAX" : "Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                if (level >= 10) {
                    //获取当前正在运行的场景
                    auto scene = Director::getInstance()->getRunningScene();
                    auto visibleSize = Director::getInstance()->getVisibleSize();
                    //如果满级了，增加一个流动的提示信息
                    std::string message = "Town Hall has reached maximum level!";
                    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 28);
                    label->setPosition(visibleSize.width / 2, visibleSize.height / 2);
                    label->setColor(Color3B::ORANGE);
                    scene->addChild(label, 1000);
                    label->runAction(Sequence::create(DelayTime::create(2.0f), RemoveSelf::create(), nullptr));
                }
                else {
                    // 没满级就开始升级
                    this->handleStartUpgrade();
                }
                });
        }
    }
    // ============================================================
    // B 金矿和水的生产类建筑
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::MINE ||
        _targetBuilding->getType() == BuildingType::WATER) {
        //先判断是不是金矿
        std::string buildingName = (_targetBuilding->getType() == BuildingType::MINE) ?
            "Gold Mine" : "Water Collector";
        std::string resourceType = (_targetBuilding->getType() == BuildingType::MINE) ?
            "coins" : "water";

        switch (state) {
            case BuildingState::PRODUCING: {
                // 设置正在生产中的状态标签
                //开启生产倒计时
                float timeLeft = _targetBuilding->getProductionTimeLeft();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nProducing...\nTime left: " + std::to_string((int)timeLeft) + "s";
                _infoLabel->setString(info);

                _actionBtn->setString("Producing");
                _actionBtn->setColor(Color3B::GRAY);
                _actionBtn->setCallback(nullptr); // 此时按钮不可以点击

                // 检查剩余时间，每秒都更新一下
                this->schedule([=](float dt) {
                    float remainingTime = _targetBuilding->getProductionTimeLeft();
                    if (remainingTime > 0) {
                        std::string info = buildingName + " Lv." + std::to_string(level) +
                            "\nProducing...\nTime left: " + std::to_string((int)remainingTime) + "s";
                        _infoLabel->setString(info);
                    }
                    else {
                        // 生产结束，关掉计时器
                        this->unschedule("production_timer");
                        //刷新界面
                        this->setBuilding(_targetBuilding);
                    }
                    }, 1.0f, "production_timer");
                break;
            }

            case BuildingState::READY: {
                // 设置生产完成的状态标签
                 //获取生产的金币数量
                int amount = _targetBuilding->getProducedAmount();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nReady to collect!\nAmount: " + std::to_string(amount) + " " + resourceType;
                _infoLabel->setString(info);

                //设置收集按钮
                _actionBtn->setString("Collect");
                _actionBtn->setColor(Color3B::GREEN);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->collectResources();
                    this->closeLayer();       //调用收集资源的函数，然后关掉
                    });
                break;
            }

            case BuildingState::IDLE: {
                //设置空闲状态的标签
                //获取升下一级所需的花费
                int nextCost = _targetBuilding->getNextLevelCost();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nIdle\nUpgrade cost: " + std::to_string(nextCost) +
                    (_targetBuilding->getType() == BuildingType::MINE ? " coins" : " water");
                _infoLabel->setString(info);

                // 设置开始生产的按钮
                _actionBtn->setString("Produce");
                _actionBtn->setColor(Color3B::YELLOW);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->startProduction();  //开始生产并刷新界面
                    this->setBuilding(_targetBuilding);
                    });

                // 设置升级按钮
                auto upgradeLabel = Label::createWithTTF("Upgrade", "fonts/Marker Felt.ttf", 30);
                upgradeLabel->setColor(Color3B::ORANGE);
                auto upgradeBtn = MenuItemLabel::create(upgradeLabel, [=](Ref*) {
                    // 获取大本营等级
                    int townHallLevel = 1;
                    for (auto& b : g_allPurchasedBuildings) {
                        if (b && b->getType() == BuildingType::BASE) {
                            townHallLevel = b->getLevel();
                            break;
                        }
                    }
                    //检查当前情况金矿等允许升到几级
                    auto upgradeLimits = BuildingUpgradeLimits::getInstance();
                    int maxLevel = upgradeLimits->getMaxLevelForBuilding(
                        _targetBuilding->getType(), townHallLevel);

                    if (level >= maxLevel) {
                        //满级给出警告
                        this->showMaxLevelWarning(buildingName, townHallLevel);
                    }
                    else {
                        //点击按钮后开始升级，刷新界面
                        _targetBuilding->startUpgrade();
                        this->setBuilding(_targetBuilding);
                    }
                    });
                //设置升级按钮的相对中心位置
                upgradeBtn->setPosition(0, -40);
                menu->addChild(upgradeBtn);
                break;
            }

            case BuildingState::UPGRADING: {
                //当前升级状态，设置信息
                float timeLeft = _targetBuilding->getTimeLeft();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nUpgrading...\nTime left: " + std::to_string((int)timeLeft) + "s";
                _infoLabel->setString(info);

                // 设置加速按钮
                _actionBtn->setString("Speed Up");
                _actionBtn->setColor(Color3B::BLUE);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->speedUp();
                    this->closeLayer();
                    });

                // 检查剩余时间，每秒都更新一下
                this->schedule([=](float dt) {
                    float remainingTime = _targetBuilding->getTimeLeft();
                    if (remainingTime > 0) {
                        std::string info = buildingName + " Lv." + std::to_string(level) +
                            "\nUpgrading...\nTime left: " + std::to_string((int)remainingTime) + "s";
                        _infoLabel->setString(info);
                    }
                    else {
                        //刷新界面
                        this->unschedule("upgrade_timer");
                        this->setBuilding(_targetBuilding);
                    }
                    }, 0.01f, "upgrade_timer");
                break;
            }
        }
    }
    // ============================================================
    // C 军营 (Barracks)
    // ============================================================
    //大体逻辑与大本营相似
    else if (_targetBuilding->getType() == BuildingType::BARRACKS) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            std::string info = "Barracks Lv." + std::to_string(level) +
                "\nCapacity: " + std::to_string(army_limit) +
                "\nCost: " + std::to_string(cost) + " Water" +
                "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

            if (isMaxLevel) {
                info = "Barracks Lv." + std::to_string(level) + " (MAX)" +
                    "\nUpgrade TH to increase level limit";
            }

            _infoLabel->setString(info);

            _actionBtn->setString(isMaxLevel ? "MAX" : "Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                if (isMaxLevel) {
                    this->showMaxLevelWarning("Barracks", townHallLevel);
                }
                else {
                    this->handleStartUpgrade();
                }
                });

            // 添加训练士兵的按钮
            auto trainLabel = Label::createWithTTF("Train Troops", "fonts/Marker Felt.ttf", 30);
            trainLabel->setColor(Color3B::GREEN);
            trainLabel->enableOutline(Color4B::BLACK, 2);

            auto trainBtn = MenuItemLabel::create(trainLabel, [=](Ref*) {
                this->closeLayer();
                auto trainingLayer = TrainingLayer::create();
                Director::getInstance()->getRunningScene()->addChild(trainingLayer, 999);  //跳转到训练士兵的界面
                });
            trainBtn->setPosition(0, -55);
            menu->addChild(trainBtn);
        }
    }
    //这里不再设置炮塔、围栏的升级界面，直接设置不会出现弹窗
    // ============================================================
    // 金币收集类建筑 (Gold Storage)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::GOLD_STORAGE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {

            bool isUnlocked = townHallLevel >= 3;

            std::string info;
            if (!isUnlocked) {
                info = "Gold Storage Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3" +
                    "\nIncreases coin capacity";
            }
            else {
                // 升级可以增加金币最大容量
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
    // 圣水收集器，逻辑与金币收集器相似
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::WATER_STORAGE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {

            bool isUnlocked = townHallLevel >= 3;

            std::string info;
            if (!isUnlocked) {
                info = "Water Storage Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3" +
                    "\nIncreases water capacity";
            }
            else {

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
//处理开始升级
void BuildingInfoLayer::handleStartUpgrade()
{
    // 调用建筑本身的升级逻辑
    _targetBuilding->startUpgrade();

    // 处理升级所需的时间
    this->handleUpgradeTimer();
}
// 处理升级所需的时间
void BuildingInfoLayer::handleUpgradeTimer()
{
    // 线计算剩余时间
    float time = _targetBuilding->getTimeLeft();
    _infoLabel->setString("Upgrading...\n" + std::to_string((int)time) + "s");

    // 处理加速
    _actionBtn->setString("Speed Up");
    _actionBtn->setCallback([=](Ref*) {
        this->unschedule("upgrade_timer");
        _targetBuilding->speedUp();        // 调用建筑自身的加速功能
        this->closeLayer();                // 加速成功关掉弹窗
        });

    // 每0.01秒更新升级剩余时间显示，达到连续倒计时的效果
    this->schedule([=](float dt) {

        float timeLeft = _targetBuilding->getTimeLeft();

        _infoLabel->setString("Upgrading...\n" + std::to_string((int)timeLeft) + "s");

        if (timeLeft <= 0 || _targetBuilding->getState() == BuildingState::IDLE) {
            this->unschedule("upgrade_timer");
            this->closeLayer();                // 升级结束关闭弹窗
        }

        }, 0.01f, "upgrade_timer");
}
//关闭界面，就是移除这个图层
void BuildingInfoLayer::closeLayer()
{
    this->removeFromParent();
}
//给出已经到达最大级别的流动警告
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

    if (townHallLevel < 10) {
        auto upgradeLimits = BuildingUpgradeLimits::getInstance();
        std::string unlockInfo = upgradeLimits->getUnlockInfoForNextTownHallLevel(townHallLevel);
        auto infoLabel = Label::createWithTTF(unlockInfo, "fonts/Marker Felt.ttf", 24);
        infoLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2 - 50);
        infoLabel->setColor(Color3B::ORANGE);
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
//给出需要升级大本营才能继续升级的流动警告
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