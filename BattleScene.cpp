#include "BattleScene.h"
#include "GameScene.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // 务必包含 Soldier 头文件
#include"DataManager.h"
#include"MapTrap.h"
USING_NS_CC;
extern int coin_count;
extern int water_count;
extern int gem_count;

extern int coin_limit;
extern int water_limit;
extern int gem_limit;
#include "AudioEngine.h"
// 使用命名空间

// ==========================================
// 【新增】onEnter 实现
// ==========================================
void BattleScene::onEnter()
{
    // 1. 必须先调用父类的 onEnter，否则场景生命周期会乱
    Scene::onEnter();

    // 2. 停止之前场景（比如主城）的所有声音
    AudioEngine::stopAll();

    AudioEngine::play2d("music/2.ogg", true, 0.5f);

    // 如果你还需要做其他进场逻辑（比如重置某些状态），也可以写在这里
    log("Enter BattleScene: Music Started.");
}
bool BattleScene::init()
{
    // 仅执行基类初始化，所有具体设置转移到 initWithMap 中
    return Scene::init();
}

// 【新增】设置地图文件名（供内部调用）
void BattleScene::setMapFileName(const std::string& fileName)
{
    _mapFileName = fileName;
}

// 【替换】不带参数的 createScene (默认)：调用带参数版本
Scene* BattleScene::createScene()
{
    // 默认加载第一张地图的文件名
    return BattleScene::createScene("Enemy_map1.tmx");
}

// 【替换】带参数的 createScene：自定义创建流程
Scene* BattleScene::createScene(const std::string& mapFileName)
{
    // 自定义创建流程，确保在调用 initWithMap 时传入正确的地图文件名
    BattleScene* scene = new (std::nothrow) BattleScene();
    if (scene && scene->initWithMap(mapFileName))
    {
        scene->autorelease();
        return scene;
    }
    else
    {
        CC_SAFE_DELETE(scene);
        return nullptr;
    }
}

// 【新增】自定义初始化函数 (请确保在 BattleScene.h 中声明 bool initWithMap(const std::string& mapFileName);)
bool BattleScene::initWithMap(const std::string& mapFileName)
{
    // 1. 调用父类初始化 (这里必须返回 true，因为 init() 已经保证成功)
    if (!Scene::init()) return false;

    // 2. 【核心】设置地图文件名
    _mapFileName = mapFileName;

    // 3. 初始化交互状态变量 (原 init() 逻辑开始)
    _isPlacingMode = false;
    _spawnCount = 0;
    _isTouchingMap = false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 4. 加载 TMX 地图 (包含对象解析)
    this->loadEnemyMap();

    // 5. 创建战斗 UI (士兵图标、提示文字)
    this->createUI();

    // 6. 初始化绘制节点 (用于画红色禁止区域)
    _forbiddenAreaNode = DrawNode::create();
    this->addChild(_forbiddenAreaNode, 10);

    // 7. 添加触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 8. 添加返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 28);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    backItem->setPosition(Vec2(origin.x + backLabel->getContentSize().width / 2 + 10,
        origin.y + backLabel->getContentSize().height / 2 + 10));

    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 5);

    this->scheduleUpdate(); // 开启每帧调用

    return true;
}

void BattleScene::update(float dt)
{
    // 如果游戏已结束或暂停，不执行游戏逻辑
    if (_isGameOver || _isGamePaused) return;

    // 检查游戏是否结束
    checkGameEnd();

    // 如果游戏已结束，不再执行后续逻辑
    if (_isGameOver) return;
    if (!_traps.empty()) {
        for (auto it = _traps.begin(); it != _traps.end(); ) {
            auto trap = *it;

            // 传入当前所有士兵进行检测
            bool triggered = trap->checkTrigger(_soldiers);

            if (triggered) {
                // 如果爆炸了，从检测列表中移除 (避免重复爆炸)
                it = _traps.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    // **士兵 AI 更新和清理**
    auto soldierIt = _soldiers.begin();
    while (soldierIt != _soldiers.end())
    {
        auto soldier = *soldierIt;

        // 1. 运行 AI 逻辑
        soldier->update(dt);

        // 2. 检查士兵是否死亡 (自爆兵在 attackTarget 已经将 _currentHp 设为 0)
        if (soldier->getCurrentHp() <= 0)
        {
            // 移除出 Cocos 场景
            soldier->removeFromParent();

            // 从容器中移除指针，防止再次被 update
            soldierIt = _soldiers.erase(soldierIt);
        }
        else
        {
            // 士兵存活，继续下一个
            ++soldierIt;
        }
    }

    // 遍历所有防御塔，让它们执行逻辑
    for (auto node : _towers) {
        auto tower = dynamic_cast<EnemyBuilding*>(node);
        if (tower && !tower->isDestroyed()) {
            tower->updateTowerLogic(dt, _soldiers);
        }
    }
}

void BattleScene::menuBackToGameScene(Ref* pSender)
{
    // 如果游戏已结束（显示胜利弹窗时），先清理弹窗
    if (_isGameOver) {
        hideVictoryPopup();
        _isGameOver = false;
        _isGamePaused = false;
    }

    Director::getInstance()->popScene();
}

void BattleScene::checkGameEnd()
{
    // 如果游戏已经结束，不再检测
    if (_isGameOver) return;

    // ==========================================
    // 1. 胜利检测 (保持不变)
    // ==========================================
    bool victory = true;

    // 检查大本营
    if (_base && !_base->isDestroyed()) {
        victory = false;
    }

    // 检查防御塔 (排除墙)
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

        // 冻结战场 (停止所有动作)
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
        return; // 赢了就直接返回，不用测输了没
    }

    // ==========================================
    // 2. 失败检测 (新增)
    // ==========================================

    // 条件A: 场上已经没有活着的士兵了
    bool noSoldiersOnField = _soldiers.empty();

    // 条件B: 玩家手里已经没有剩余的兵可以派了
    bool noReservesLeft = true;
    for (auto item : _soldierUIList) {
        if (item->count > 0) {
            noReservesLeft = false;
            break; // 只要还有一种兵能派，就还没输
        }
    }

    // 只有当 A 和 B 都满足，且前面已经确认没赢 (victory == false)，才算输
    if (noSoldiersOnField && noReservesLeft) {
        _isGameOver = true;
        _isGamePaused = true;

        log("Game Over: Defeat!");

        // 冻结战场
        for (auto t : _towers) if (t) { t->stopAllActions(); t->unscheduleAllCallbacks(); }
        if (_base) _base->stopAllActions();
        if (_tileMap) for (auto c : _tileMap->getChildren()) c->stopAllActions();

        // 延迟显示失败弹窗
        this->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([this]() { this->showDefeatPopup(); }),
            nullptr
        ));
    }
}
// 【新增】显示胜利弹窗
void BattleScene::showVictoryPopup()
{
    // 如果当前加载的是 map1，则解锁到 2
    if (_mapFileName == "Enemy_map1.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 2) {
            DataManager::getInstance()->setMaxLevelUnlocked(2);
        }
    }
    // 如果当前加载的是 map2，则解锁到 3（此时空军解锁）
    else if (_mapFileName == "Enemy_map2.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 3) {
            DataManager::getInstance()->setMaxLevelUnlocked(3);
        }
    }
    // 如果当前加载的是 map2，则解锁到 3（此时空军解锁）
    else if (_mapFileName == "Enemy_map3.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 4) {
            DataManager::getInstance()->setMaxLevelUnlocked(4);
        }
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 center = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    // 1. 半透明背景 (保持不变)
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    // 添加触摸吞噬，防止点击到弹窗后面的东西
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bgLayer);
    this->addChild(bgLayer, 100);

    // ==========================================
    // 【修改点 1】统一控制弹窗大小
    // ==========================================
    float targetScale = 5.0f; // 想要多大改这里！(比如改 2.5f 变小，改 4.0f 变大)

    // 2. 创建弹窗背景
    auto popupBg = Sprite::create("popup_bg.png");

    // 获取原始尺寸
    Size originalSize = popupBg->getContentSize();
    // 计算缩放后的实际高度（用于排版文字）
    float bgHeight = originalSize.height * targetScale;

    popupBg->setPosition(center);
    this->addChild(popupBg, 101);

    // 3. 容器节点 (保持不变)
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102);

    // 4. 胜利标题 (坐标调整：基于 bgHeight 计算)
    auto victoryLabel = Label::createWithTTF("VICTORY!", "fonts/Marker Felt.ttf", 36);
    victoryLabel->setTextColor(Color4B::GREEN);
    // 从中心向上偏移 (背景高度的 1/3 处)
    victoryLabel->setPosition(Vec2(0, bgHeight * 0.35f));
    container->addChild(victoryLabel);

    // 5. 胜利信息
    auto messageLabel = Label::createWithTTF("Enemy Base Destroyed!", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::YELLOW);
    messageLabel->setPosition(Vec2(0, bgHeight * 0.15f));
    container->addChild(messageLabel);

    // 6. 资源奖励
    // 增加资源逻辑保持不变
    int coinReward = 2000;
    int waterReward = 1000;

    // 计算实际能获得的金币（不超过上限）
    int coinAvailableSpace = coin_limit - coin_count;
    int actualCoinGain = std::min(coinReward, coinAvailableSpace);
    if (actualCoinGain > 0) {
        coin_count += actualCoinGain;
    }

    // 计算实际能获得的圣水（不超过上限）
    int waterAvailableSpace = water_limit - water_count;
    int actualWaterGain = std::min(waterReward, waterAvailableSpace);
    if (actualWaterGain > 0) {
        water_count += actualWaterGain;
    }

    // 【新增】显示实际获得的资源
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
    // 【修改点 2】修复按钮无法点击的问题
    // ==========================================
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 32);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    // 给按钮加个简单的放大效果，提示玩家可以点
    auto scaleSeq = Sequence::create(ScaleTo::create(1.0f, 1.1f), ScaleTo::create(1.0f, 1.0f), NULL);
    backItem->runAction(RepeatForever::create(scaleSeq));

    // 设置 Item 在 Menu 里的位置 (在背景底部)
    backItem->setPosition(Vec2(0, -bgHeight * 0.35f));

    // 错误做法：container->addChild(backItem);
    // 正确做法：必须放入 Menu 容器！
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO); // 菜单对齐容器中心
    container->addChild(menu);

    // 7. 动画效果 (使用 targetScale)
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

    // 1. 半透明背景
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    // 吞噬触摸
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bgLayer);
    this->addChild(bgLayer, 100);

    // 2. 弹窗背景
    float targetScale = 5.0f;
    auto popupBg = Sprite::create("popup_bg.png"); // 复用背景图
    Size originalSize = popupBg->getContentSize();
    float bgHeight = originalSize.height * targetScale;

    popupBg->setPosition(center);
    this->addChild(popupBg, 101);

    // 3. 容器
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102);

    // 4. 标题 (DEFEAT - 红色)
    auto titleLabel = Label::createWithTTF("DEFEAT", "fonts/Marker Felt.ttf", 36);
    titleLabel->setTextColor(Color4B::RED);
    titleLabel->enableOutline(Color4B::BLACK, 2);
    titleLabel->setPosition(Vec2(0, bgHeight * 0.35f));
    container->addChild(titleLabel);

    // 5. 失败信息
    auto messageLabel = Label::createWithTTF("All soldiers are dead...", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::WHITE);
    messageLabel->setPosition(Vec2(0, bgHeight * 0.15f));
    container->addChild(messageLabel);

    auto messageLabel1 = Label::createWithTTF("Try to upgrade you soldiers!", "fonts/Marker Felt.ttf", 28);
    messageLabel1->setTextColor(Color4B::WHITE);
    messageLabel1->setPosition(Vec2(0, bgHeight * 0.05f));
    container->addChild(messageLabel1);

    // 6. 返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 32);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    // 简单的呼吸动画
    auto scaleSeq = Sequence::create(ScaleTo::create(1.0f, 1.1f), ScaleTo::create(1.0f, 1.0f), NULL);
    backItem->runAction(RepeatForever::create(scaleSeq));
    backItem->setPosition(Vec2(0, -bgHeight * 0.25f));

    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    container->addChild(menu);

    // 7. 进场动画
    popupBg->setScale(0.1f);
    popupBg->runAction(EaseBackOut::create(ScaleTo::create(0.4f, targetScale)));

    container->setScale(0.1f);
    container->runAction(EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)));
}
// 【新增】隐藏胜利弹窗
void BattleScene::hideVictoryPopup()
{

    this->removeChildByName("victory_bg_layer");
    this->removeChildByName("victory_popup");
    this->removeChildByName("victory_content");
}


void BattleScene::loadEnemyMap()
{
    // ================= 1. 创建 Tiled Map (只创建一次) =================
    // 【修正：只使用 _mapFileName 变量加载地图】
    _tileMap = TMXTiledMap::create(_mapFileName);

    if (!_tileMap) {
        // 如果加载失败，输出日志并返回
        log("Error: Failed to load TMX map: %s", _mapFileName.c_str());
        return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 设置地图位置：居中
    float mapWidth = _tileMap->getContentSize().width;
    float mapHeight = _tileMap->getContentSize().height;

    _tileMap->setPosition(Vec2(
        origin.x + (visibleSize.width - mapWidth) / 2,
        origin.y + (visibleSize.height - mapHeight) / 2
    ));

    // Z-order 设为 -1 保证在地板层
    this->addChild(_tileMap, -1);

    // ================= 2. 解析对象层 (Object Layer) =================
    // 对应 Tiled 里的 "object" 层
    auto objectGroup = _tileMap->getObjectGroup("object");

    if (objectGroup) {
        auto& objects = objectGroup->getObjects();

        for (auto& obj : objects) {
            ValueMap& dict = obj.asValueMap();

            // 获取基础属性
            std::string name = dict["name"].asString();
            float x = dict["x"].asFloat();
            float y = dict["y"].asFloat();
            float w = dict["width"].asFloat();
            float h = dict["height"].asFloat();
            if (dict["fileName"].asString() == "Tree.png" || dict["fileName"].asString() == "Tree1.png" || dict["fileName"].asString() == "Tree2.png") {
                y += 100;
            }
            if (name != "boom") {
                Vec2 worldPos = _tileMap->convertToWorldSpace(Vec2(x, y));
                Rect worldRect(worldPos.x, worldPos.y, w, h);
                _forbiddenRects.push_back(worldRect);
            }

            // 2. 创建实体对象 (Base 和 Tower)，添加到 _tileMap 上 (使用局部坐标 x, y)

            // ----------------------------------------------------
            // 处理 A: 大本营 (Base)
            // ----------------------------------------------------
            if (name == "Base") {
                int hp = dict["HP"].asInt();
                if (hp == 0) hp = 80;

                int damagePerNotch = hp / 4; // 示例: 80 / 4 = 20
                if (damagePerNotch == 0) damagePerNotch = 1; // 避免除零或血量太低导致 Notch 为 0

                // 创建 Base
                _base = EnemyBuilding::create(
                    "map/buildings/Base.png",
                    "ui/Heart2.png",
                    hp,
                    damagePerNotch,
                    0,
                    0.0f // Base 不攻击
                );

                if (_base) {
                    _base->setType(EnemyType::BASE);
                    _base->setPosition(x + w / 2, y + h / 2);
                    _tileMap->addChild(_base, 3);
                }
            }

            // ----------------------------------------------------
            // 处理 B: 防御塔 (tower)
            // ----------------------------------------------------
            // 在解析 "tower" 的地方：

            else if (name == "tower") {
                int hp = dict["HP"].asInt();
                int atk = dict["Attack"].asInt(); // 读出攻击力
                float range = 250.0f;//设射程是 250, 可以在 TMX 里配一个 "Range" 属性读出来

                // 计算 notch
                int damagePerNotch = hp / 4;
                if (damagePerNotch == 0) damagePerNotch = 1;

                // 使用新的 create 函数
                auto tower = EnemyBuilding::create(
                    "map/buildings/TilesetTowers.png",
                    "ui/Heart2.png",
                    hp,
                    damagePerNotch,
                    atk,   // 传入攻击力
                    range  // 传入射程
                );

                if (tower) {
                    tower->setPosition(x + w / 2, y + h / 2);
                    // 【新增】确保普通塔标记为 TOWER
                    tower->setType(EnemyType::TOWER);
                    _tileMap->addChild(tower, 3);
                    // 加入列表供士兵寻找
                    _towers.pushBack(tower);
                }
            }
            else if (name == "cannon") {
                int hp = dict["HP"].asInt();
                int atk = dict["Attack"].asInt(); // 读出攻击力
                float range = 200.0f;//设射程是 250, 可以在 TMX 里配一个 "Range" 属性读出来

                // 计算 notch
                int damagePerNotch = hp / 4;
                if (damagePerNotch == 0)
                    damagePerNotch = 1;

                // 使用新的 create 函数
                auto cannon = EnemyBuilding::create(
                    "map/buildings/Cannon1.png",
                    "ui/Heart2.png",
                    hp,
                    damagePerNotch,
                    atk,   // 传入攻击力
                    range  // 传入射程
                );

                if (cannon) {
                    cannon->setPosition(x + w / 2, y + h / 2);
                    // 【新增】显式设置为 CANNON 类型
                    cannon->setType(EnemyType::CANNON);
                    _tileMap->addChild(cannon, 3);
                    // 加入列表供士兵寻找
                    _towers.pushBack(cannon);
                }
            }
            else if (name == "fence") {
                int hp = 40; // 墙通常血比较厚

                // 墙没有攻击力，射程为0
                int atk = 0;
                float range = 0;

                // 计算 Notch (血条格数)
                int damagePerNotch = hp / 4;
                if (damagePerNotch == 0) damagePerNotch = 1;

                // 2. 创建实体
                // 注意：准备一张 fence.png 和 vertical_fence.png (如果需要竖着的)
                auto fence = EnemyBuilding::create(
                    "map/buildings/fence.png", // 你的栅栏图片路径
                    "",
                    hp,
                    damagePerNotch,
                    atk,
                    range
                );

                if (fence) {
                    fence->setPosition(x + w / 2, y + h / 2);
                    fence->setType(EnemyType::WALL);
                    _tileMap->addChild(fence, 2); // 层级比塔稍微低一点
                    _towers.pushBack(fence);
                }
            }
            else if (name == "boom") {
                // 读取伤害配置
                int damage = dict["Damage"].asInt();
                if (damage == 0) 
                    damage = 1000; // 默认秒杀级伤害

                // 定义陷阱区域 (使用地图局部坐标，因为士兵移动也是基于地图坐标)
                Rect trapRect(x, y, w, h);

                // 创建隐形陷阱
                auto trap = MapTrap::create(trapRect, damage);
                if (trap) {
                    _tileMap->addChild(trap); // 加到地图上
                    _traps.pushBack(trap);    // 加入管理列表
                }
            }
            // ----------------------------------------------------
            // 处理 C: 障碍物 (Tree, grass)
            // ----------------------------------------------------
            else if (name == "grass"||name=="mine"||name=="root") {
                // 不需要创建实体类，上面的 _forbiddenRects.push_back 已经处理了阻挡逻辑
            }
        }
        auto objectGroup = _tileMap->getObjectGroup("object");
        //检测地图中对象层的树
        if (objectGroup) {
            ValueVector objects = objectGroup->getObjects();

            for (const auto& v : objects) {
                ValueMap dict = v.asValueMap();

                if (dict.find("fileName") != dict.end())
                {
                    std::string path = dict["fileName"].asString();

                    auto sprite = Sprite::create(path);

                    if (sprite) {
                        _tileMap->addChild(sprite, 1);

                        sprite->setAnchorPoint(Vec2::ZERO);
                        sprite->getTexture()->setAliasTexParameters();

                        float x = dict["x"].asFloat();
                        float y = dict["y"].asFloat();

                        sprite->setPosition(x, y + 100);

                        if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
                            float width = dict["width"].asFloat();
                            float height = dict["height"].asFloat();

                            sprite->setScaleX(width / sprite->getContentSize().width);
                            sprite->setScaleY(height / sprite->getContentSize().height);
                        }

                        sprite->setLocalZOrder(3);
                    }
                    else {
                        log("Error: Can't load image: %s", path.c_str());
                    }
                }
            }
        }
    }
}

// ============================================================
// 1. 重写 CreateUI：动态生成兵种列表
// ============================================================
void BattleScene::createUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 初始化选中状态
    _currentSelectedItem = nullptr;
    _isPlacingMode = false;

    // 定义我们要遍历的所有兵种配置
    // 结构：{枚举类型, DataManager里的名字, 图片路径}
    struct Config {
        SoldierType type;
        std::string dataName;
        std::string imagePath;
    };

    std::vector<Config> configs = {
        { SoldierType::ORIGINAL, "Soldier", "anim/man1.png" }, // 注意路径要对
        { SoldierType::ARROW,    "Arrow",   "anim/arrow1.png" },
        { SoldierType::BOOM,     "Boom",    "anim/boom1.png" },
        { SoldierType::GIANT,    "Giant",   "anim/giant1.png" },
        // 【新增】Airforce
        { SoldierType::AIRFORCE, "Airforce","anim/Owl1.png" }
    };

    // 起始位置 (右下角向左排列)
    float startX = visibleSize.width - 60;
    float startY = 60;
    float gap = 120; // 图标间隔

    int index = 0;
    for (const auto& cfg : configs) {
        // 1. 从 DataManager 获取该兵种的数量
        int count = DataManager::getInstance()->getTroopCount(cfg.dataName);

        // 只有数量 > 0 才显示图标
        if (count > 0) {

            // 创建新的 UI 项结构体
            SoldierUIItem* item = new SoldierUIItem();
            item->type = cfg.type;
            item->nameKey = cfg.dataName;
            item->count = count;

            // 创建图标 Sprite
            item->icon = Sprite::create(cfg.imagePath);
            if (!item->icon) {
                // 如果图片找不到，用默认图代替防止崩溃
                item->icon = Sprite::create("anim/giant1.png");
            }
            item->icon->setScale(5.0f); // 调整大小
            item->icon->setPosition(startX - (index * gap), startY);
            this->addChild(item->icon, 20);

            // 创建数量 Label
            item->countLabel = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 24);
            item->countLabel->setPosition(item->icon->getPositionX(), item->icon->getPositionY() - 40);
            item->countLabel->setTextColor(Color4B::GREEN);
            item->countLabel->enableOutline(Color4B::BLACK, 1); // 加个描边看清楚点
            this->addChild(item->countLabel, 21);

            // 保存到列表
            _soldierUIList.push_back(item);
            index++;
        }
    }

    // 提示信息 Label (保持不变)
    _msgLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 24);
    _msgLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    _msgLabel->setTextColor(Color4B::RED);
    _msgLabel->setOpacity(0);
    this->addChild(_msgLabel, 30);
}

// ============================================================
// 2. 处理兵种图标点击
// ============================================================
void BattleScene::onSoldierIconClicked(SoldierUIItem* item)
{
    // 情况 A：如果在放置模式，且点击的是【当前选中的】图标 -> 取消放置
    if (_isPlacingMode && _currentSelectedItem == item) {
        _isPlacingMode = false;
        _currentSelectedItem = nullptr;

        // 还原 UI
        item->icon->stopAllActions();
        item->icon->setScale(3.0f);
        item->icon->setColor(Color3B::WHITE);
        _forbiddenAreaNode->clear();
        this->unschedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler));
        return;
    }

    // 情况 B：如果点击了别的图标，或者之前没选中 -> 切换/选中

    // 1. 先把旧的选中项还原
    if (_currentSelectedItem) {
        _currentSelectedItem->icon->stopAllActions();
        _currentSelectedItem->icon->setScale(3.0f);
        _currentSelectedItem->icon->setColor(Color3B::WHITE);
    }

    // 2. 检查数量是否耗尽
    if (item->count <= 0) {
        showWarning("No more soldiers of this type!");
        _isPlacingMode = false;
        _currentSelectedItem = nullptr;
        _forbiddenAreaNode->clear();
        return;
    }

    // 3. 选中新的
    _isPlacingMode = true;
    _currentSelectedItem = item;
    _currentSelectedType = item->type; // 记录下要造什么兵

    // 4. 播放选中动画
    auto action = RepeatForever::create(Sequence::create(
        ScaleTo::create(0.5f, 3.5f), // 稍微变大
        ScaleTo::create(0.5f, 3.0f),
        nullptr
    ));
    item->icon->runAction(action);
    item->icon->setColor(Color3B::YELLOW);

    // 5. 绘制红色区域 (逻辑不变)
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
// 3. 修改触摸开始：检测点击了哪个图标
// ============================================================
bool BattleScene::onTouchBegan(Touch* touch, Event* event)
{
    Vec2 touchLoc = touch->getLocation();

    // 1. 遍历检查是否点到了下方的兵种列表
    for (auto item : _soldierUIList) {
        if (item->icon->getBoundingBox().containsPoint(touchLoc)) {
            onSoldierIconClicked(item); // 触发该图标的逻辑
            return true; // 吞噬事件
        }
    }

    // 2. 地图点击 (放置模式)
    if (_isPlacingMode && _currentSelectedItem) {
        _isTouchingMap = true;
        _currentTouchPos = touchLoc;

        // 尝试生成
        trySpawnSoldier(touchLoc);

        // 开启长按连续出兵
        this->schedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler), 0.2f);
        return true;
    }

    return false;
}
void BattleScene::onTouchMoved(Touch* touch, Event* event)
{
    if (_isPlacingMode && _isTouchingMap) {
        _currentTouchPos = touch->getLocation(); // 更新位置，允许拖动放置
    }
}

void BattleScene::onTouchEnded(Touch* touch, Event* event)
{
    _isTouchingMap = false;
    // 停止长按生成
    this->unschedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler));
}

void BattleScene::spawnScheduler(float dt)
{
    if (_isPlacingMode && _isTouchingMap) {
        trySpawnSoldier(_currentTouchPos);
    }
}

// ============================================================
// 4. 修改生成逻辑：使用当前选中的兵种
// ============================================================
void BattleScene::trySpawnSoldier(Vec2 worldPos)
{
    // 安全检查
    if (!_currentSelectedItem || _currentSelectedItem->count <= 0) {
        _isPlacingMode = false;
        // 如果没兵了，自动取消选中
        if (_currentSelectedItem) onSoldierIconClicked(_currentSelectedItem);
        return;
    }

    // 2. 检查碰撞 (不可放置区域)
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

    // 3. 放置成功
    Vec2 nodePos = _tileMap->convertToNodeSpace(worldPos);

    // 【核心修改】使用 _currentSelectedType 创建对应的兵
    auto soldier = Soldier::create(this, _currentSelectedType);
    soldier->setPosition(nodePos);
    _tileMap->addChild(soldier, 5);
    _soldiers.pushBack(soldier);

    // 4. 扣除数量
    _currentSelectedItem->count--;

    // 5. 更新 UI 数字
    _currentSelectedItem->countLabel->setString(std::to_string(_currentSelectedItem->count));

    // 6. 更新 DataManager (可选，如果希望消耗是永久的)
    // DataManager::getInstance()->setTroopCount(_currentSelectedItem->nameKey, _currentSelectedItem->count);

    // 7. 如果数量归零，变灰并停止放置
    if (_currentSelectedItem->count <= 0) {
        _currentSelectedItem->icon->setColor(Color3B::GRAY);
        _currentSelectedItem->countLabel->setColor(Color3B::RED);

        // 强制退出放置模式
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
    // 1. 检查大本营 (如果没炸)
    if (_base && !_base->isDestroyed()) {
        Rect baseRect = _base->getBoundingBox();
        Vec2 baseWorldPos = _tileMap->convertToWorldSpace(baseRect.origin);
        Rect worldRect(baseWorldPos.x, baseWorldPos.y, baseRect.size.width, baseRect.size.height);

        if (worldRect.containsPoint(worldPos)) return true;
    }

    // 2. 检查所有活着的塔和墙
    for (auto building : _towers) {
        if (building && !building->isDestroyed()) {
            Rect rect = building->getBoundingBox();
            Vec2 buildingWorldPos = _tileMap->convertToWorldSpace(rect.origin);
            Rect worldRect(buildingWorldPos.x, buildingWorldPos.y, rect.size.width, rect.size.height);

            if (worldRect.containsPoint(worldPos)) {
                return true; // 被活着的建筑挡住了
            }
        }
    }

    return false;
}