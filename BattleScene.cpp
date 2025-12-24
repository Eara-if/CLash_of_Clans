#include "BattleScene.h"
#include "GameScene.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // ��ذ��� Soldier ͷ�ļ�
#include"DataManager.h"
#include"MapTrap.h"
#include "SharedData.h"
#include "json/document.h"
#include "SaveGame.h"

USING_NS_CC;
extern int coin_count;
extern int water_count;
extern int gem_count;

extern int coin_limit;
extern int water_limit;
extern int gem_limit;
#include "AudioEngine.h"
// ʹ�������ռ�

// ==========================================
// ��������onEnter ʵ��
// ==========================================
void BattleScene::onEnter()
{
    // 1. �����ȵ��ø���� onEnter�����򳡾��������ڻ���
    Scene::onEnter();

    // 2. ֹ֮ͣǰ�������������ǣ�����������
    AudioEngine::stopAll();

    AudioEngine::play2d("music/2.ogg", true, 0.5f);

    // ����㻹��Ҫ�����������߼�����������ĳЩ״̬����Ҳ����д������
    log("Enter BattleScene: Music Started.");
}
bool BattleScene::init()
{
    if (!Scene::init()) return false;
    // 这里只做最基础的成员变量初始化，具体的地图加载推迟到 setupBattle
    _tileMap = nullptr;
    _base = nullptr;
    _isGameOver = false;
    _isGamePaused = false;
    return true;
}

// �����������õ�ͼ�ļ��������ڲ����ã�
void BattleScene::setMapFileName(const std::string& fileName)
{
    _mapFileName = fileName;
}

Scene* BattleScene::createScene(int levelIndex, std::string pvpJsonData)
{
    // 使用 Cocos2d-x 标准的 create()，它会触发 init()
    auto scene = BattleScene::create();
    if (scene) {
        scene->setupBattle(levelIndex, pvpJsonData);
    }
    return scene;
}

void BattleScene::setupBattle(int levelIndex, std::string pvpJsonData)
{
    // 防御性检查：防止重复调用或状态错乱
    if (_tileMap != nullptr) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 加载地图数据
    if (levelIndex > 0) {
        _mapFileName = StringUtils::format("Enemy_map%d.tmx", levelIndex);
        this->loadLevelCampaign(levelIndex);
    }
    else {
        _mapFileName = "BaseMap.tmx";
        this->loadLevelPVP(pvpJsonData);
    }

    // --- 关键检查 ---
    if (!_tileMap || _tileMap->getParent() == nullptr) {
        log("CRITICAL ERROR: Map load failed or addChild missing!");
        // 给用户一个反馈，而不是直接关掉
        auto label = Label::createWithTTF("Map File Missing!", "fonts/arial.ttf", 32);
        label->setPosition(visibleSize / 2);
        this->addChild(label);
        return;
    }

    // 2. 初始化UI
    this->createUI();

    // 3. 初始化禁放区渲染层
    _forbiddenAreaNode = DrawNode::create();
    this->addChild(_forbiddenAreaNode, 10);

    // 4. 绑定触摸监听器 (重要：必须添加到层中)
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 5. 添加返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 28);
    auto backItem = MenuItemLabel::create(backLabel, CC_CALLBACK_1(BattleScene::menuBackToGameScene, this));
    backItem->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 30));
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);

    // 6. 开启更新
    this->scheduleUpdate();
}

void BattleScene::update(float dt)
{
    // �����Ϸ�ѽ�������ͣ����ִ����Ϸ�߼�
    if (_isGameOver || _isGamePaused) return;

    // �����Ϸ�Ƿ����
    checkGameEnd();

    // �����Ϸ�ѽ���������ִ�к����߼�
    if (_isGameOver) return;
    if (!_traps.empty()) {
        for (auto it = _traps.begin(); it != _traps.end(); ) {
            auto trap = *it;

            // ���뵱ǰ����ʿ�����м��
            bool triggered = trap->checkTrigger(_soldiers);

            if (triggered) {
                // �����ը�ˣ��Ӽ���б����Ƴ� (�����ظ���ը)
                it = _traps.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    // **ʿ�� AI ���º�����**
    auto soldierIt = _soldiers.begin();
    while (soldierIt != _soldiers.end())
    {
        auto soldier = *soldierIt;

        // 1. ���� AI �߼�
        soldier->update(dt);

        // 2. ���ʿ���Ƿ����� (�Ա����� attackTarget �Ѿ��� _currentHp ��Ϊ 0)
        if (soldier->getCurrentHp() <= 0)
        {
            // �Ƴ��� Cocos ����
            soldier->removeFromParent();

            // ���������Ƴ�ָ�룬��ֹ�ٴα� update
            soldierIt = _soldiers.erase(soldierIt);
        }
        else
        {
            // ʿ����������һ��
            ++soldierIt;
        }
    }

    // �������з�������������ִ���߼�
    for (auto node : _towers) {
        auto tower = dynamic_cast<EnemyBuilding*>(node);
        if (tower && !tower->isDestroyed()) {
            tower->updateTowerLogic(dt, _soldiers);
        }
    }
}

void BattleScene::menuBackToGameScene(Ref* pSender)
{
    if (_isGameOver) {
        hideVictoryPopup();
        _isGameOver = false;
        _isGamePaused = false;
    }

    // 调用带参数的 createScene，传入 nullptr
    auto scene = GameScene::createScene(nullptr);
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BattleScene::checkGameEnd()
{
    // �����Ϸ�Ѿ����������ټ��
    if (_isGameOver) return;

    // ==========================================
    // 1. ʤ����� (���ֲ���)
    // ==========================================
    bool victory = true;

    // ����Ӫ
    if (_base && !_base->isDestroyed()) {
        victory = false;
    }

    // �������� (�ų�ǽ)
    if (victory) {
        for (auto building : _towers) {
            if (building && !building->isDestroyed() && building->getType() != EnemyType::WALL) {
                victory = false;
                break;
            }
        }
    }

    if (victory) {
        _isGameOver = true;
        _isGamePaused = true;

        // ����ս�� (ֹͣ���ж���)
        for (auto s : _soldiers)
            if (s) {
                s->stopAllActions();
                s->unscheduleAllCallbacks();
            }
        for (auto t : _towers) if (t) { t->stopAllActions(); t->unscheduleAllCallbacks(); }
        if (_base) _base->stopAllActions();
        if (_tileMap) for (auto c : _tileMap->getChildren()) c->stopAllActions();

        this->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([this]() { this->showVictoryPopup(); }),
            nullptr
        ));
        return; // Ӯ�˾�ֱ�ӷ��أ����ò�����û
    }

    // ==========================================
    // 2. ʧ�ܼ�� (����)
    // ==========================================

    // ����A: �����Ѿ�û�л��ŵ�ʿ����
    bool noSoldiersOnField = _soldiers.empty();

    // ����B: ��������Ѿ�û��ʣ��ı���������
    bool noReservesLeft = true;
    for (auto item : _soldierUIList) {
        if (item->count > 0) {
            noReservesLeft = false;
            break; // ֻҪ����һ�ֱ����ɣ��ͻ�û��
        }
    }

    // ֻ�е� A �� B �����㣬��ǰ���Ѿ�ȷ��ûӮ (victory == false)��������
    if (noSoldiersOnField && noReservesLeft) {
        _isGameOver = true;
        _isGamePaused = true;

        log("Game Over: Defeat!");

        // ����ս��
        for (auto t : _towers) if (t) { t->stopAllActions(); t->unscheduleAllCallbacks(); }
        if (_base) _base->stopAllActions();
        if (_tileMap) for (auto c : _tileMap->getChildren()) c->stopAllActions();

        // �ӳ���ʾʧ�ܵ���
        this->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([this]() { this->showDefeatPopup(); }),
            nullptr
        ));
    }
}
// ����������ʾʤ������
void BattleScene::showVictoryPopup()
{
    // �����ǰ���ص��� map1��������� 2
    if (_mapFileName == "Enemy_map1.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 2) {
            DataManager::getInstance()->setMaxLevelUnlocked(2);
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", 2);
        }
    }
    // �����ǰ���ص��� map2��������� 3����ʱ�վ�������
    else if (_mapFileName == "Enemy_map2.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 3) {
            DataManager::getInstance()->setMaxLevelUnlocked(3);
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", 3);
        }
    }
    // �����ǰ���ص��� map2��������� 3����ʱ�վ�������
    else if (_mapFileName == "Enemy_map3.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 4) {
            DataManager::getInstance()->setMaxLevelUnlocked(4);
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", 4);
        }
    }
    UserDefault::getInstance()->flush();
    SaveGame::getInstance()->syncDataToCloud();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 center = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    // 1. ��͸������ (���ֲ���)
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    // ��Ӵ������ɣ���ֹ�������������Ķ���
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bgLayer);
    this->addChild(bgLayer, 100);

    // ==========================================
    // ���޸ĵ� 1��ͳһ���Ƶ�����С
    // ==========================================
    float targetScale = 5.0f; // ��Ҫ�������(����� 2.5f ��С���� 4.0f ���)

    // 2. ������������
    auto popupBg = Sprite::create("popup_bg.png");

    // ��ȡԭʼ�ߴ�
    Size originalSize = popupBg->getContentSize();
    // �������ź��ʵ�ʸ߶ȣ������Ű����֣�
    float bgHeight = originalSize.height * targetScale;

    popupBg->setPosition(center);
    this->addChild(popupBg, 101);

    // 3. �����ڵ� (���ֲ���)
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102);

    // 4. ʤ������ (������������� bgHeight ����)
    auto victoryLabel = Label::createWithTTF("VICTORY!", "fonts/Marker Felt.ttf", 36);
    victoryLabel->setTextColor(Color4B::GREEN);
    // ����������ƫ�� (�����߶ȵ� 1/3 ��)
    victoryLabel->setPosition(Vec2(0, bgHeight * 0.35f));
    container->addChild(victoryLabel);

    // 5. ʤ����Ϣ
    auto messageLabel = Label::createWithTTF("Enemy Base Destroyed!", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::YELLOW);
    messageLabel->setPosition(Vec2(0, bgHeight * 0.15f));
    container->addChild(messageLabel);

    // 6. ��Դ����
    // ������Դ�߼����ֲ���
    int coinReward = 2000;
    int waterReward = 1000;

    // ����ʵ���ܻ�õĽ�ң����������ޣ�
    int coinAvailableSpace = coin_limit - coin_count;
    int actualCoinGain = std::min(coinReward, coinAvailableSpace);
    if (actualCoinGain > 0) {
        coin_count += actualCoinGain;
    }

    // ����ʵ���ܻ�õ�ʥˮ�����������ޣ�
    int waterAvailableSpace = water_limit - water_count;
    int actualWaterGain = std::min(waterReward, waterAvailableSpace);
    if (actualWaterGain > 0) {
        water_count += actualWaterGain;
    }

    // ����������ʾʵ�ʻ�õ���Դ
    std::string rewardText;
    if (actualCoinGain < coinReward || actualWaterGain < waterReward) {
        rewardText = "You got " + std::to_string(actualCoinGain) + " Coins & " +
            std::to_string(actualWaterGain) + " Water! (Storage full)";
    }
    else {
        rewardText = "You got " + std::to_string(coinReward) + " Coins & " +
            std::to_string(waterReward) + " Water!";
    }

    auto getLabel = Label::createWithTTF(rewardText, "fonts/Marker Felt.ttf", 24);
    getLabel->setTextColor(Color4B::WHITE);
    getLabel->setPosition(Vec2(0, -bgHeight * 0.05f));
    container->addChild(getLabel);

    // ==========================================
    // ���޸ĵ� 2���޸���ť�޷����������
    // ==========================================
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 32);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    // ����ť�Ӹ��򵥵ķŴ�Ч������ʾ��ҿ��Ե�
    auto scaleSeq = Sequence::create(ScaleTo::create(1.0f, 1.1f), ScaleTo::create(1.0f, 1.0f), NULL);
    backItem->runAction(RepeatForever::create(scaleSeq));

    // ���� Item �� Menu ���λ�� (�ڱ����ײ�)
    backItem->setPosition(Vec2(0, -bgHeight * 0.35f));

    // ����������container->addChild(backItem);
    // ��ȷ������������� Menu ������
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO); // �˵�������������
    container->addChild(menu);

    // 7. ����Ч�� (ʹ�� targetScale)
    popupBg->setScale(0.1f);
    popupBg->runAction(EaseBackOut::create(ScaleTo::create(0.4f, targetScale)));

    container->setScale(0.1f);
    container->runAction(EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)));
}
// BattleScene.cpp

void BattleScene::showDefeatPopup()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 center = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    // 1. ��͸������
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    // ���ɴ���
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bgLayer);
    this->addChild(bgLayer, 100);

    // 2. ��������
    float targetScale = 5.0f;
    auto popupBg = Sprite::create("popup_bg.png"); // ���ñ���ͼ
    Size originalSize = popupBg->getContentSize();
    float bgHeight = originalSize.height * targetScale;

    popupBg->setPosition(center);
    this->addChild(popupBg, 101);

    // 3. ����
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102);

    // 4. ���� (DEFEAT - ��ɫ)
    auto titleLabel = Label::createWithTTF("DEFEAT", "fonts/Marker Felt.ttf", 36);
    titleLabel->setTextColor(Color4B::RED);
    titleLabel->enableOutline(Color4B::BLACK, 2);
    titleLabel->setPosition(Vec2(0, bgHeight * 0.35f));
    container->addChild(titleLabel);

    // 5. ʧ����Ϣ
    auto messageLabel = Label::createWithTTF("All soldiers are dead...", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::WHITE);
    messageLabel->setPosition(Vec2(0, bgHeight * 0.15f));
    container->addChild(messageLabel);

    auto messageLabel1 = Label::createWithTTF("Try to upgrade you soldiers!", "fonts/Marker Felt.ttf", 28);
    messageLabel1->setTextColor(Color4B::WHITE);
    messageLabel1->setPosition(Vec2(0, bgHeight * 0.05f));
    container->addChild(messageLabel1);

    // 6. ���ذ�ť
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 32);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    // �򵥵ĺ�������
    auto scaleSeq = Sequence::create(ScaleTo::create(1.0f, 1.1f), ScaleTo::create(1.0f, 1.0f), NULL);
    backItem->runAction(RepeatForever::create(scaleSeq));
    backItem->setPosition(Vec2(0, -bgHeight * 0.25f));

    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    container->addChild(menu);

    // 7. ��������
    popupBg->setScale(0.1f);
    popupBg->runAction(EaseBackOut::create(ScaleTo::create(0.4f, targetScale)));

    container->setScale(0.1f);
    container->runAction(EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)));
}
// ������������ʤ������
void BattleScene::hideVictoryPopup()
{

    this->removeChildByName("victory_bg_layer");
    this->removeChildByName("victory_popup");
    this->removeChildByName("victory_content");
}


void BattleScene::loadLevelCampaign(int levelIndex)
{
    // 1. 【路径修正】：直接从根目录加载 Enemy_map%d.tmx
    _mapFileName = StringUtils::format("Enemy_map%d.tmx", levelIndex);
    _tileMap = TMXTiledMap::create(_mapFileName);

    if (!_tileMap) {
        log("CRITICAL ERROR: Map file %s not found in Resources!", _mapFileName.c_str());
        // 保底逻辑：如果找不到，尝试加载第一关，若还找不到则直接返回防止后续 addChild(nullptr) 崩溃
        _tileMap = TMXTiledMap::create("Enemy_map1.tmx");
        if (!_tileMap) return;
    }

    // 2. 添加到场景并设置层级
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    float mapWidth = _tileMap->getContentSize().width;
    float mapHeight = _tileMap->getContentSize().height;

    // 居中地图
    _tileMap->setPosition(Vec2(
        origin.x + (visibleSize.width - mapWidth) / 2,
        origin.y + (visibleSize.height - mapHeight) / 2
    ));

    // 地图放在最底层
    this->addChild(_tileMap, -1);

    // 3. 【核心修复】：合并后的对象解析逻辑
    auto objectGroup = _tileMap->getObjectGroup("object");
    if (objectGroup) {
        ValueVector objects = objectGroup->getObjects();

        for (auto& v : objects) {
            ValueMap dict = v.asValueMap();
            std::string name = dict["name"].asString();
            float x = dict["x"].asFloat();
            float y = dict["y"].asFloat();
            float w = dict["width"].asFloat();
            float h = dict["height"].asFloat();

            // A. 处理禁放区（除了炸弹以外的所有建筑/障碍物区域）
            if (name != "boom") {
                // 将地图局部坐标转换为世界坐标，用于放置士兵时的碰撞检测
                Vec2 worldPos = _tileMap->convertToWorldSpace(Vec2(x, y));
                Rect worldRect(worldPos.x, worldPos.y, w, h);
                _forbiddenRects.push_back(worldRect);
            }

            // B. 处理逻辑单位 (Base, Tower, Cannon, Wall)
            if (name == "Base") {
                int hp = dict.count("HP") ? dict["HP"].asInt() : 80;
                _base = EnemyBuilding::create("map/buildings/Base.png", "ui/Heart2.png", hp, hp / 4, 0, 0.0f);
                if (_base) {
                    _base->setType(EnemyType::BASE);
                    _base->setPosition(x + w / 2, y + h / 2);
                    _tileMap->addChild(_base, 3);
                }
            }
            else if (name == "tower" || name == "cannon") {
                int hp = dict.count("HP") ? dict["HP"].asInt() : 50;
                int atk = dict.count("Attack") ? dict["Attack"].asInt() : 10;
                float range = 250.0f;
                std::string img = (name == "tower") ? "map/buildings/TilesetTowers.png" : "map/buildings/Cannon1.png";

                auto building = EnemyBuilding::create(img, "ui/Heart2.png", hp, hp / 4, atk, range);
                if (building) {
                    building->setType(name == "tower" ? EnemyType::TOWER : EnemyType::CANNON);
                    building->setPosition(x + w / 2, y + h / 2);
                    _tileMap->addChild(building, 3);
                    _towers.pushBack(building);
                }
            }
            else if (name == "fence") {
                auto fence = EnemyBuilding::create("map/buildings/fence.png", "", 40, 10, 0, 0);
                if (fence) {
                    fence->setType(EnemyType::WALL);
                    fence->setPosition(x + w / 2, y + h / 2);
                    _tileMap->addChild(fence, 2);
                    _towers.pushBack(fence);
                }
            }
            else if (name == "boom") {
                int damage = dict.count("Damage") ? dict["Damage"].asInt() : 1000;
                auto trap = MapTrap::create(Rect(x, y, w, h), damage);
                if (trap) {
                    _tileMap->addChild(trap);
                    _traps.pushBack(trap);
                }
            }
            // C. 处理纯装饰物（基于 fileName 属性）
            else if (dict.find("fileName") != dict.end()) {
                std::string path = dict["fileName"].asString();
                auto sprite = Sprite::create(path);
                if (sprite) {
                    sprite->setAnchorPoint(Vec2::ZERO);
                    // 树木等装饰物向上偏移一点，视觉效果更好
                    float yOffset = (path.find("Tree") != std::string::npos) ? 100.0f : 0.0f;
                    sprite->setPosition(x, y + yOffset);

                    // 设置缩放
                    if (dict.count("width") && dict.count("height")) {
                        sprite->setScaleX(dict["width"].asFloat() / sprite->getContentSize().width);
                        sprite->setScaleY(dict["height"].asFloat() / sprite->getContentSize().height);
                    }
                    _tileMap->addChild(sprite, 2);
                }
            }
        }
    }
}
void BattleScene::loadLevelPVP(const std::string& jsonContent)
{
    // 1. 加载基础地图
    _tileMap = TMXTiledMap::create(_mapFileName);
    this->addChild(_tileMap, -1);

    // 2. 解析 JSON
    rapidjson::Document doc;
    doc.Parse(jsonContent.c_str());

    if (doc.HasParseError() || !doc.HasMember("buildings")) return;

    const rapidjson::Value& buildings = doc["buildings"];
    for (auto& b : buildings.GetArray()) {
        int typeInt = b["type"].GetInt();
        float x = b["x"].GetFloat();
        float y = b["y"].GetFloat();
        int level = b["level"].GetInt();

        BuildingType bType = static_cast<BuildingType>(typeInt);

        // 3. 根据类型调用 EnemyBuilding::create
        // 这里的血量和攻击力，你可以根据 level 动态计算
        EnemyBuilding* eb = nullptr;

        if (bType == BuildingType::BASE) {
            eb = EnemyBuilding::create("map/buildings/Base.png", "ui/Heart2.png", 1000 + level * 200, 250, 0, 0);
            _base = eb;
		}
		else if (bType == BuildingType::TOWER) {
			eb = EnemyBuilding::create("map/buildings/TilesetTowers.png", "ui/Heart2.png", 600 + level * 150, 150, 15, 250);
			_towers.pushBack(eb);
		}
        else if (bType == BuildingType::CANNON) {
            eb = EnemyBuilding::create("map/buildings/Cannon1.png", "ui/Heart2.png", 500 + level * 100, 125, 20, 200);
            _towers.pushBack(eb);
		}
		else if (bType == BuildingType::WALL) {
			eb = EnemyBuilding::create("map/buildings/fence.png", "", 300 + level * 50, 75, 0, 0);
			_towers.pushBack(eb);
		}
        // ... 其他类型 ...

        if (eb) {
            eb->setPosition(x, y);
            _tileMap->addChild(eb, 3);

            // 别忘了把建筑位置加入禁放区
            Rect worldRect(x - 50, y - 50, 100, 100); // 粗略计算，建议根据图片大小
            _forbiddenRects.push_back(worldRect);
        }
    }
}


// ============================================================
// 1. ��д CreateUI����̬���ɱ����б�
// ============================================================
void BattleScene::createUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // ��ʼ��ѡ��״̬
    _currentSelectedItem = nullptr;
    _isPlacingMode = false;

    // ��������Ҫ���������б�������
    // �ṹ��{ö������, DataManager�������, ͼƬ·��}
    struct Config {
        SoldierType type;
        std::string dataName;
        std::string imagePath;
    };

    std::vector<Config> configs = {
        { SoldierType::ORIGINAL, "Soldier", "anim/man1.png" }, // ע��·��Ҫ��
        { SoldierType::ARROW,    "Arrow",   "anim/arrow1.png" },
        { SoldierType::BOOM,     "Boom",    "anim/boom1.png" },
        { SoldierType::GIANT,    "Giant",   "anim/giant1.png" },
        // ��������Airforce
        { SoldierType::AIRFORCE, "Airforce","anim/Owl1.png" }
    };

    // ��ʼλ�� (���½���������)
    float startX = visibleSize.width - 60;
    float startY = 60;
    float gap = 120; // ͼ����

    int index = 0;
    for (const auto& cfg : configs) {
        // 1. �� DataManager ��ȡ�ñ��ֵ�����
        int count = DataManager::getInstance()->getTroopCount(cfg.dataName);

        // ֻ������ > 0 ����ʾͼ��
        if (count > 0) {

            // �����µ� UI ��ṹ��
            SoldierUIItem* item = new SoldierUIItem();
            item->type = cfg.type;
            item->nameKey = cfg.dataName;
            item->count = count;

            // ����ͼ�� Sprite
            item->icon = Sprite::create(cfg.imagePath);
            if (!item->icon) {
                // ���ͼƬ�Ҳ�������Ĭ��ͼ�����ֹ����
                item->icon = Sprite::create("anim/giant1.png");
            }
            item->icon->setScale(5.0f); // ������С
            item->icon->setPosition(startX - (index * gap), startY);
            this->addChild(item->icon, 20);

            // �������� Label
            item->countLabel = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 24);
            item->countLabel->setPosition(item->icon->getPositionX(), item->icon->getPositionY() - 40);
            item->countLabel->setTextColor(Color4B::GREEN);
            item->countLabel->enableOutline(Color4B::BLACK, 1); // �Ӹ���߿������
            this->addChild(item->countLabel, 21);

            // ���浽�б�
            _soldierUIList.push_back(item);
            index++;
        }
    }

    // ��ʾ��Ϣ Label (���ֲ���)
    _msgLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 24);
    _msgLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    _msgLabel->setTextColor(Color4B::RED);
    _msgLabel->setOpacity(0);
    this->addChild(_msgLabel, 30);
}

// ============================================================
// 2. �������ͼ����
// ============================================================
void BattleScene::onSoldierIconClicked(SoldierUIItem* item)
{
    // ��� A������ڷ���ģʽ���ҵ�����ǡ���ǰѡ�еġ�ͼ�� -> ȡ������
    if (_isPlacingMode && _currentSelectedItem == item) {
        _isPlacingMode = false;
        _currentSelectedItem = nullptr;

        // ��ԭ UI
        item->icon->stopAllActions();
        item->icon->setScale(3.0f);
        item->icon->setColor(Color3B::WHITE);
        _forbiddenAreaNode->clear();
        this->unschedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler));
        return;
    }

    // ��� B���������˱��ͼ�꣬����֮ǰûѡ�� -> �л�/ѡ��

    // 1. �ȰѾɵ�ѡ���ԭ
    if (_currentSelectedItem) {
        _currentSelectedItem->icon->stopAllActions();
        _currentSelectedItem->icon->setScale(3.0f);
        _currentSelectedItem->icon->setColor(Color3B::WHITE);
    }

    // 2. ��������Ƿ�ľ�
    if (item->count <= 0) {
        showWarning("No more soldiers of this type!");
        _isPlacingMode = false;
        _currentSelectedItem = nullptr;
        _forbiddenAreaNode->clear();
        return;
    }

    // 3. ѡ���µ�
    _isPlacingMode = true;
    _currentSelectedItem = item;
    _currentSelectedType = item->type; // ��¼��Ҫ��ʲô��

    // 4. ����ѡ�ж���
    auto action = RepeatForever::create(Sequence::create(
        ScaleTo::create(0.5f, 3.5f), // ��΢���
        ScaleTo::create(0.5f, 3.0f),
        nullptr
    ));
    item->icon->runAction(action);
    item->icon->setColor(Color3B::YELLOW);

    // 5. ���ƺ�ɫ���� (�߼�����)
    _forbiddenAreaNode->clear();
    for (const auto& rect : _forbiddenRects) {
        _forbiddenAreaNode->drawSolidRect(
            rect.origin,
            Vec2(rect.getMaxX(), rect.getMaxY()),
            Color4F(1.0f, 0.0f, 0.0f, 0.3f)
        );
    }
}

// ============================================================
// 3. �޸Ĵ�����ʼ����������ĸ�ͼ��
// ============================================================
bool BattleScene::onTouchBegan(Touch* touch, Event* event)
{
    Vec2 touchLoc = touch->getLocation();

    // 1. ��������Ƿ�㵽���·��ı����б�
    for (auto item : _soldierUIList) {
        if (item->icon->getBoundingBox().containsPoint(touchLoc)) {
            onSoldierIconClicked(item); // ������ͼ����߼�
            return true; // �����¼�
        }
    }

    // 2. ��ͼ��� (����ģʽ)
    if (_isPlacingMode && _currentSelectedItem) {
        _isTouchingMap = true;
        _currentTouchPos = touchLoc;

        // ��������
        trySpawnSoldier(touchLoc);

        // ���������������
        this->schedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler), 0.2f);
        return true;
    }

    return false;
}
void BattleScene::onTouchMoved(Touch* touch, Event* event)
{
    if (_isPlacingMode && _isTouchingMap) {
        _currentTouchPos = touch->getLocation(); // ����λ�ã������϶�����
    }
}

void BattleScene::onTouchEnded(Touch* touch, Event* event)
{
    _isTouchingMap = false;
    // ֹͣ��������
    this->unschedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler));
}

void BattleScene::spawnScheduler(float dt)
{
    if (_isPlacingMode && _isTouchingMap) {
        trySpawnSoldier(_currentTouchPos);
    }
}

// ============================================================
// 4. �޸������߼���ʹ�õ�ǰѡ�еı���
// ============================================================
void BattleScene::trySpawnSoldier(Vec2 worldPos)
{
    // ��ȫ���
    if (!_currentSelectedItem || _currentSelectedItem->count <= 0) {
        _isPlacingMode = false;
        // ���û���ˣ��Զ�ȡ��ѡ��
        if (_currentSelectedItem) onSoldierIconClicked(_currentSelectedItem);
        return;
    }

    // 2. �����ײ (���ɷ�������)
    bool isBlocked = false;
    Rect soldierRect(worldPos.x - 10, worldPos.y - 10, 20, 20);
    for (const auto& rect : _forbiddenRects) {
        if (rect.intersectsRect(soldierRect)) {
            isBlocked = true;
            break;
        }
    }

    if (isBlocked) {
        showWarning("Cannot place here!");
        return;
    }

    // 3. ���óɹ�
    Vec2 nodePos = _tileMap->convertToNodeSpace(worldPos);

    // �������޸ġ�ʹ�� _currentSelectedType ������Ӧ�ı�
    auto soldier = Soldier::create(this, _currentSelectedType);
    soldier->setPosition(nodePos);
    _tileMap->addChild(soldier, 5);
    _soldiers.pushBack(soldier);

    // 4. �۳�����
    _currentSelectedItem->count--;

    // 5. ���� UI ����
    _currentSelectedItem->countLabel->setString(std::to_string(_currentSelectedItem->count));

    // 6. ���� DataManager (��ѡ�����ϣ�����������õ�)
    // DataManager::getInstance()->setTroopCount(_currentSelectedItem->nameKey, _currentSelectedItem->count);

    // 7. ����������㣬��Ҳ�ֹͣ����
    if (_currentSelectedItem->count <= 0) {
        _currentSelectedItem->icon->setColor(Color3B::GRAY);
        _currentSelectedItem->countLabel->setColor(Color3B::RED);

        // ǿ���˳�����ģʽ
        _isPlacingMode = false;
        _currentSelectedItem->icon->stopAllActions();
        _currentSelectedItem->icon->setScale(3.0f);
        _forbiddenAreaNode->clear();
        _currentSelectedItem = nullptr;
    }
}

void BattleScene::showWarning(const std::string& msg)
{
    _msgLabel->setString(msg);
    _msgLabel->setOpacity(255);
    _msgLabel->stopAllActions();
    _msgLabel->runAction(Sequence::create(
        DelayTime::create(1.0f),
        FadeOut::create(0.5f),
        nullptr
    ));
}

// BattleScene.cpp

bool BattleScene::isPositionBlocked(Vec2 worldPos)
{
    // 1. ����Ӫ (���ûը)
    if (_base && !_base->isDestroyed()) {
        Rect baseRect = _base->getBoundingBox();
        Vec2 baseWorldPos = _tileMap->convertToWorldSpace(baseRect.origin);
        Rect worldRect(baseWorldPos.x, baseWorldPos.y, baseRect.size.width, baseRect.size.height);

        if (worldRect.containsPoint(worldPos)) return true;
    }

    // 2. ������л��ŵ�����ǽ
    for (auto building : _towers) {
        if (building && !building->isDestroyed()) {
            Rect rect = building->getBoundingBox();
            Vec2 buildingWorldPos = _tileMap->convertToWorldSpace(rect.origin);
            Rect worldRect(buildingWorldPos.x, buildingWorldPos.y, rect.size.width, rect.size.height);

            if (worldRect.containsPoint(worldPos)) {
                return true; // �����ŵĽ�����ס��
            }
        }
    }

    return false;
}