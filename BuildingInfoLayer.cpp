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
    // 1. ���һ����͸���ĺ�ɫ���� (����)
    // ������������ʱ�������䰵�����޷��������Ķ���
    auto shieldLayer = LayerColor::create(Color4B(0, 0, 0, 150)); // RGBA: ��ɫ��͸����150
    this->addChild(shieldLayer);

    // 2. ���ش����¼� (���ɵ��)
    // ����һ������ļ�������ר�ŷ�ֹ�����͸
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true); // �ؼ������ɴ����������¼���������ķ���
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 3. ��������ͼ (�����������һ�Ž� popup_bg.png ��ͼ)
    // ���û�У������� LayerColor::create(Color4B::WHITE, 300, 200) �������
    bg = Sprite::create("popup_bg.png");
    if (!bg) {
        // ���û��ͼ������һ����ɫ�ķ�����棬��ֹ����
        bg = Sprite::create();
        bg->setTextureRect(Rect(0, 0, 400, 300));
        bg->setColor(Color3B::WHITE);
    }
    bg->getTexture()->setAliasTexParameters();
    bg->setScale(4.0f);
    bg->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);
    this->addChild(bg, 0);

    // ����һ��ͨ�õ� Label��λ�÷��м�
    _infoLabel = Label::createWithSystemFont("", "Arial", 28);
    _infoLabel->setPosition(centerPos.x, centerPos.y + 80);

    _infoLabel->setTextColor(Color4B::BLACK);
    this->addChild(_infoLabel, 1);

    _actionBtn = MenuItemFont::create("Button", [](Ref*) {});
    _actionBtn->setColor(Color3B::BLUE);

    // �رհ�ť
    auto closeBtn = MenuItemFont::create("Close", [=](Ref*) { this->closeLayer(); });
    closeBtn->setColor(Color3B::RED);
    closeBtn->setPosition(0, -85); // ��Բ˵����ĵ�ƫ��

    menu = Menu::create(_actionBtn, closeBtn, NULL);

    // ���޸ĵ� E���˵�λ��Ҳ������Ļ������΢����һ��
    menu->setPosition(centerPos.x, centerPos.y - 40);

    // ���޸ĵ� F����ӵ� Layer���㼶��Ϊ 1
    this->addChild(menu, 1);

    return true;
}

void BuildingInfoLayer::setBuilding(Building* building)
{
    _targetBuilding = building;

    // 1. ����ɵĻص�
    _actionBtn->setCallback(nullptr);
    // ȷ��֮ǰ�Ķ�ʱ�����ر� (��ֹ�����򿪵���ʱ��ʱ���ѻ�)
    this->unschedule("upgrade_timer");

    // 2. ����˵��г��˹رհ�ť֮���������ť
    if (menu) {
        // �Ƴ����رհ�ť�����������
        Vector<Node*> children = menu->getChildren();
        for (auto child : children) {
            auto menuItem = dynamic_cast<MenuItem*>(child);
            if (menuItem != _actionBtn && menuItem != nullptr) {
                menu->removeChild(menuItem, 1);
            }
        }

        // ʼ����ӹرհ�ť
        auto closeLabel = Label::createWithTTF("Close", "fonts/Marker Felt.ttf", 30);
        auto closeBtn = MenuItemLabel::create(closeLabel, [=](Ref*) { this->closeLayer(); });
        closeBtn->setColor(Color3B::RED);
        closeBtn->setPosition(0, -85);
        menu->addChild(closeBtn);
    }

    // ��ȡ����
    int level = _targetBuilding->getLevel();
    int cost = _targetBuilding->getNextLevelCost();
    BuildingState state = _targetBuilding->getState();

    // ��ȡ��Ӫ�ȼ�������������Ϣ
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
    // ��� A����Ӫ (Town Hall)
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::BASE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            // ����������
            this->handleUpgradeTimer();
            return;
        }
        else if (state == BuildingState::IDLE) {
            // �������޸ġ���ʾ����ϸ����Ϣ��������һ������
            std::string info = "Town Hall Lv." + std::to_string(level);

            // ��ʾ�������Ѻ�ʱ��
            if (level < 10) {
                info += "\nNext Level Cost: " + std::to_string(cost) + " Coin";

                // ��ʾ��Ӫ����������Ϣ
                std::string unlockInfo = upgradeLimits->getUnlockInfoForNextTownHallLevel(level);
                info += "\nNew Unlocks:\n" + unlockInfo;

                // ����������ʾ��һ��������������
                auto nextLevelMaxLevels = upgradeLimits->getMaxLevelsForTownHall(level + 1);
                auto currentLevelMaxLevels = upgradeLimits->getMaxLevelsForTownHall(level);
            }
            else {
                info += "\n\nMAX LEVEL REACHED!";
            }

            _infoLabel->setString(info);
            _infoLabel->setDimensions(380, 0); // ���ÿ���Զ�����

            // ����������ť
            _actionBtn->setString(level >= 10 ? "MAX" : "Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                if (level >= 10) {
                    // �ﵽ���ȼ�����ʾ��ʾ
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
                    // ��ʼ�����߼�
                    this->handleStartUpgrade();
                }
                });
        }
    }
    // ============================================================
    // ����ʥˮ�ռ����Ĵ����߼�
    // ============================================================
    if (_targetBuilding->getType() == BuildingType::MINE ||
        _targetBuilding->getType() == BuildingType::WATER) {

        std::string buildingName = (_targetBuilding->getType() == BuildingType::MINE) ?
            "Gold Mine" : "Water Collector";
        std::string resourceType = (_targetBuilding->getType() == BuildingType::MINE) ?
            "coins" : "water";

        switch (state) {
            case BuildingState::PRODUCING: {
                // ״̬1����������
                float timeLeft = _targetBuilding->getProductionTimeLeft();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nProducing...\nTime left: " + std::to_string((int)timeLeft) + "s";
                _infoLabel->setString(info);

                // ����_actionBtnΪ���ɵ����Producing��ť
                _actionBtn->setString("Producing");
                _actionBtn->setColor(Color3B::GRAY);
                _actionBtn->setCallback(nullptr); // ���ɵ��

                // �����ʱ����������ʱ��
                this->schedule([=](float dt) {
                    float remainingTime = _targetBuilding->getProductionTimeLeft();
                    if (remainingTime > 0) {
                        std::string info = buildingName + " Lv." + std::to_string(level) +
                            "\nProducing...\nTime left: " + std::to_string((int)remainingTime) + "s";
                        _infoLabel->setString(info);
                    }
                    else {
                        // ������ɣ��Զ�ˢ��UI
                        this->unschedule("production_timer");
                        this->setBuilding(_targetBuilding);
                    }
                    }, 1.0f, "production_timer");
                break;
            }

            case BuildingState::READY: {
                // ״̬2��������ɣ����ռ�
                int amount = _targetBuilding->getProducedAmount();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nReady to collect!\nAmount: " + std::to_string(amount) + " " + resourceType;
                _infoLabel->setString(info);

                // ����_actionBtnΪCollect��ť
                _actionBtn->setString("Collect");
                _actionBtn->setColor(Color3B::GREEN);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->collectResources();
                    this->closeLayer();
                    });
                break;
            }

            case BuildingState::IDLE: {
                // ״̬3�����ã���ѡ������������
                int nextCost = _targetBuilding->getNextLevelCost();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nIdle\nUpgrade cost: " + std::to_string(nextCost) +
                    (_targetBuilding->getType() == BuildingType::MINE ? " coins" : " water");
                _infoLabel->setString(info);

                // ����_actionBtnΪProduce��ť
                _actionBtn->setString("Produce");
                _actionBtn->setColor(Color3B::YELLOW);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->startProduction();
                    // ˢ��UI����ʾ����״̬
                    this->setBuilding(_targetBuilding);
                    });

                // ��Ӷ����Upgrade��ť���˵�
                auto upgradeLabel = Label::createWithTTF("Upgrade", "fonts/Marker Felt.ttf", 30);
                upgradeLabel->setColor(Color3B::ORANGE);
                auto upgradeBtn = MenuItemLabel::create(upgradeLabel, [=](Ref*) {
                    // ������ȼ�
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
                        // ˢ��UI����ʾ����״̬
                        this->setBuilding(_targetBuilding);
                    }
                    });
                upgradeBtn->setPosition(0, -40);
                menu->addChild(upgradeBtn);
                break;
            }

            case BuildingState::UPGRADING: {
                // ״̬4����������
                float timeLeft = _targetBuilding->getTimeLeft();
                std::string info = buildingName + " Lv." + std::to_string(level) +
                    "\nUpgrading...\nTime left: " + std::to_string((int)timeLeft) + "s";
                _infoLabel->setString(info);

                // ����_actionBtnΪSpeed Up��ť
                _actionBtn->setString("Speed Up");
                _actionBtn->setColor(Color3B::BLUE);
                _actionBtn->setCallback([=](Ref*) {
                    _targetBuilding->speedUp();
                    this->closeLayer();
                    });

                // �����ʱ����������ʱ��
                this->schedule([=](float dt) {
                    float remainingTime = _targetBuilding->getTimeLeft();
                    if (remainingTime > 0) {
                        std::string info = buildingName + " Lv." + std::to_string(level) +
                            "\nUpgrading...\nTime left: " + std::to_string((int)remainingTime) + "s";
                        _infoLabel->setString(info);
                    }
                    else {
                        // ������ɣ��Զ�ˢ��UI
                        this->unschedule("upgrade_timer");
                        this->setBuilding(_targetBuilding);
                    }
                    }, 0.01f, "upgrade_timer");
                break;
            }
        }
    }
    // ============================================================
    // ��� D����Ӫ (Barracks)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::BARRACKS) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // ��ʾ��Ϣ
            std::string info = "Barracks Lv." + std::to_string(level) +
                "\nCapacity: " + std::to_string(army_limit) +
                "\nCost: " + std::to_string(cost) + " Water" +
                "\nMax: Lv." + std::to_string(maxLevel) + " (TH" + std::to_string(townHallLevel) + ")";

            if (isMaxLevel) {
                info = "Barracks Lv." + std::to_string(level) + " (MAX)" +
                    "\nUpgrade TH to increase level limit";
            }

            _infoLabel->setString(info);

            // ����������ť
            _actionBtn->setString(isMaxLevel ? "MAX" : "Upgrade");
            _actionBtn->setCallback([=](Ref*) {
                if (isMaxLevel) {
                    this->showMaxLevelWarning("Barracks", townHallLevel);
                }
                else {
                    this->handleStartUpgrade();
                }
                });

            // --- ѵ����ť ---
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
    // ��� E�������� (Archer Tower)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::DEFENSE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // ���ȼ���Ƿ��������Ӫ�ȼ��Ƿ��㹻��
            bool isUnlocked = townHallLevel >= 3; // ��������ҪTH3����

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

            // ����������ť
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
    // ��� F����ũ�� (Cannon) 
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::CANNON) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // ���ȼ���Ƿ��������Ӫ�ȼ��Ƿ��㹻��
            bool isUnlocked = townHallLevel >= 4; // ��ũ����ҪTH4����

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

            // ����������ť
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
    // ��� G����ǽ (Wall)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::WALL) {
        // ��ǽͨ�����������������������������ʽ
        std::string info = "Wall\nDefensive structure\nCannot be upgraded directly";

        _infoLabel->setString(info);

        // ��ǽû��������ť��ֻ��ʾ��Ϣ
        _actionBtn->setString("Close");
        _actionBtn->setCallback([=](Ref*) {
            this->closeLayer();
            });
    }
    // ============================================================
    // ��� H����Ҵ洢�� (Gold Storage)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::GOLD_STORAGE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // ���ȼ���Ƿ��������Ӫ�ȼ��Ƿ��㹻��
            bool isUnlocked = townHallLevel >= 3; // �洢����ҪTH3����

            std::string info;
            if (!isUnlocked) {
                info = "Gold Storage Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3" +
                    "\nIncreases coin capacity";
            }
            else {
                // ��ʾ������Ϣ������ÿ������1500������
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

            // ����������ť
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
    // ��� I��ʥˮ�洢�� (Water Storage)
    // ============================================================
    else if (_targetBuilding->getType() == BuildingType::WATER_STORAGE) {
        BuildingState state = _targetBuilding->getState();

        if (state == BuildingState::UPGRADING) {
            this->handleUpgradeTimer();
        }
        else if (state == BuildingState::IDLE) {
            // ���ȼ���Ƿ��������Ӫ�ȼ��Ƿ��㹻��
            bool isUnlocked = townHallLevel >= 3; // �洢����ҪTH3����

            std::string info;
            if (!isUnlocked) {
                info = "Water Storage Lv." + std::to_string(level) +
                    "\n\nLOCKED! Requires Town Hall Level 3" +
                    "\nIncreases water capacity";
            }
            else {
                // ��ʾ������Ϣ������ÿ������1500������
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

            // ����������ť
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
// �������� setBuilding ����
void BuildingInfoLayer::handleStartUpgrade()
{
    // 1. ������ʼ�ɻ�
    _targetBuilding->startUpgrade();

    // 2. �����ʱ�����
    this->handleUpgradeTimer();
}
void BuildingInfoLayer::handleUpgradeTimer()
{
    // 1. ����ˢ��һ������
    float time = _targetBuilding->getTimeLeft();
    _infoLabel->setString("Upgrading...\n" + std::to_string((int)time) + "s");

    // 2. ����ť��Ϊ "Speed Up" (����)
    _actionBtn->setString("Speed Up");
    _actionBtn->setCallback([=](Ref*) {
        this->unschedule("upgrade_timer"); // �ص��ʱ��
        _targetBuilding->speedUp();        // �������
        this->closeLayer();                // �رյ���
        });

    // 3. �����ġ����� Lambda ��ʱ�� (ÿ 0.1 ��ˢ��һ��)
    this->schedule([=](float dt) {

        // A. ��ȡʣ��ʱ��
        float timeLeft = _targetBuilding->getTimeLeft();

        // B. ˢ�µ���ʱ���� (ʵ�ֶ�̬����)
        _infoLabel->setString("Upgrading...\n" + std::to_string((int)timeLeft) + "s");

        // C. ����Ƿ����
        if (timeLeft <= 0 || _targetBuilding->getState() == BuildingState::IDLE) {
            this->unschedule("upgrade_timer"); // �ص��ʱ��
            this->closeLayer();                // �Զ��ر�
        }

        }, 0.01f, "upgrade_timer");
}

void BuildingInfoLayer::closeLayer()
{
    this->removeFromParent();
}
void BuildingInfoLayer::showTrainingMenu()
{
    // 1. ����ԭ������Ϣ
    _infoLabel->setVisible(false);
    menu->setVisible(false); // ����������ť

    // 2. ��������ѡ��˵�
    auto soldierItem = MenuItemImage::create("soldier/arrow.png", "soldier/man.png", [=](Ref*) {
        log("Training Soldier...");
        // ��ˮ�������������...
        });

    auto archerItem = MenuItemImage::create("soldier/arrow.png", "soldier/man.png", [=](Ref*) {
        log("Training Archer...");
        });

    auto trainMenu = Menu::create(soldierItem, archerItem, NULL);
    trainMenu->alignItemsHorizontallyWithPadding(20);
    trainMenu->setPosition(0, 0); // ��� bg ����

    // ������˵��ӵ�����ͼ��
    // ע�⣺����� parent Ӧ���� bg����� bg �Ǿֲ���������Ҫ�ĳɳ�Ա���� _bg
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

    // ��ʾ����������Ϣ
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