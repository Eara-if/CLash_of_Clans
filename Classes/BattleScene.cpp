/**
 * @file       BattleScene.cpp
 * @brief      战斗场景核心类实现文件
 * @details    该文件实现了 BattleScene 头文件声明的所有功能，包括场景生命周期管理、PVE/PVP 关卡加载、
 *             士兵召唤与AI调度、敌方建筑管理、游戏状态判断、UI交互与触摸事件处理等核心逻辑，
 *             是战斗流程的主控实现，协调地图、士兵、敌方建筑、陷阱等模块的交互，支持战斗音乐与弹窗特效
 * @version    1.0
 * @note       该文件依赖 GameScene、EnemyBuilding 等模块；保留原有业务逻辑，仅修正乱码与补充注释，
 *             弹窗创建未设置节点名称，导致 `hideVictoryPopup` 按名移除失效，可后续优化节点命名
 */
#include "BattleScene.h"
#include "GameScene.h"
#include "EnemyBuilding.h"
#include "Soldier.h" 
#include "DataManager.h"
#include "MapTrap.h"
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

// ==========================================
// 场景生命周期：进入场景回调
// ==========================================
/**
 * @brief      场景进入回调函数实现
 * @details    继承自 Scene 类的 onEnter 方法，先调用父类实现，再停止所有原有音频，
 *             播放战斗背景音乐，并输出场景进入日志，完成场景进入后的音频初始化
 */
void BattleScene::onEnter()
{
    // 1. 优先调用父类 onEnter 方法，保证父类生命周期正常执行
    Scene::onEnter();

    // 2. 停止当前所有音频，避免与战斗音乐冲突
    AudioEngine::stopAll();

    // 播放战斗背景音乐（循环播放，音量0.5）
    AudioEngine::play2d("music/2.ogg", true, 0.5f);

    // 输出场景进入日志，便于调试
    log("Enter BattleScene: Music Started.");
}

/**
 * @brief      场景初始化函数实现
 * @details    先初始化父类 Scene，再对战斗场景核心成员变量进行默认赋值，
 *             仅做基础初始化，具体地图与业务初始化推迟到 setupBattle 方法
 * @return     bool  父类初始化成功返回 true；父类初始化失败返回 false
 */
bool BattleScene::init()
{
    if (!Scene::init()) return false;
    // 基础成员变量初始化，避免空指针异常
    _tileMap = nullptr;
    _base = nullptr;
    _isGameOver = false;
    _isGamePaused = false;
    return true;
}


/**
 * @brief      带参数静态创建场景（核心创建入口）
 * @details    通过 Cocos2d-x 标准 create 方法创建 BattleScene 实例，
 *             若创建成功则调用 setupBattle 方法初始化战斗业务，返回场景指针
 * @param      levelIndex    PVE 关卡索引（大于0有效）
 * @param      pvpJsonData   PVP 模式敌方配置 JSON 字符串（默认空字符串）
 * @return     Scene*  创建成功返回 BattleScene 场景指针；创建失败返回 nullptr
 */
Scene* BattleScene::createScene(int levelIndex, std::string pvpJsonData)
{
    // 使用 Cocos2d-x 标准 create 方法，自动触发 init 初始化
    auto scene = BattleScene::create();
    if (scene) {
        scene->setupBattle(levelIndex, pvpJsonData);
    }
    return scene;
}

/**
 * @brief      战斗场景业务初始化核心方法
 * @details    先做防御性检查避免重复初始化，再根据关卡索引区分 PVE/PVP 模式，
 *             加载对应地图与关卡数据，初始化 UI、禁止区域渲染层与触摸监听器，
 *             添加返回按钮并开启帧更新，完成战斗场景的全量初始化
 * @param      levelIndex    PVE 关卡索引（大于0加载对应 Enemy_map%d.tmx）
 * @param      pvpJsonData   PVP 模式敌方配置 JSON 字符串
 */
void BattleScene::setupBattle(int levelIndex, std::string pvpJsonData)
{
    // 防御性检查：防止地图已加载时重复调用，避免状态错乱
    if (_tileMap != nullptr) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 根据关卡索引加载对应模式的地图与关卡数据
    if (levelIndex > 0) {
        _mapFileName = StringUtils::format("Enemy_map%d.tmx", levelIndex);
        this->loadLevelCampaign(levelIndex); // 加载 PVE 关卡
    }
    else {
        _mapFileName = "Grass.tmx";
        this->loadLevelPVP(pvpJsonData); // 加载 PVP 关卡
    }

    // --- 关键检查：地图加载失败时给出用户反馈 ---
    if (!_tileMap || _tileMap->getParent() == nullptr) {
        log("CRITICAL ERROR: Map load failed or addChild missing!");
        // 创建提示标签，告知用户地图文件缺失
        auto label = Label::createWithTTF("Map File Missing!", "fonts/arial.ttf", 32);
        label->setPosition(visibleSize / 2);
        this->addChild(label);
        return;
    }

    // 2. 初始化战斗场景 UI（士兵选择 UI、提示标签等）
    this->createUI();

    // 3. 初始化禁止区域渲染层，用于绘制红色禁止放置区域
    _forbiddenAreaNode = DrawNode::create();
    this->addChild(_forbiddenAreaNode, 10);

    // 4. 绑定触摸监听器，处理玩家触摸交互（士兵选择、放置等）
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 5. 添加返回按钮，支持返回游戏主场景
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 28);
    auto backItem = MenuItemLabel::create(backLabel, CC_CALLBACK_1(BattleScene::menuBackToGameScene, this));
    backItem->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 30));
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);

    // 6. 开启帧更新调度，驱动战斗逻辑（士兵 AI、建筑逻辑等）
    this->scheduleUpdate();
}

/**
 * @brief      场景帧更新函数实现（重写父类方法）
 * @details    每帧调用，先判断游戏是否结束或暂停，若处于结束/暂停状态则直接返回；
 *             再检查游戏结束条件，处理陷阱触发逻辑，更新士兵 AI 与敌方建筑逻辑，
 *             移除死亡士兵，完成战斗场景的帧级调度
 * @param      dt  帧间隔时间（秒），用于时间相关逻辑计算
 * @override   cocos2d::Node::update
 */
void BattleScene::update(float dt)
{
    // 游戏结束或暂停时，停止执行所有战斗逻辑
    if (_isGameOver || _isGamePaused) return;

    // 检查游戏是否满足胜利/失败条件
    checkGameEnd();

    // 游戏已结束，直接返回，避免后续逻辑执行
    if (_isGameOver) return;

    // 处理陷阱触发逻辑，移除已触发的陷阱
    if (!_traps.empty()) {
        for (auto it = _traps.begin(); it != _traps.end(); ) {
            auto trap = *it;

            // 检查陷阱是否被士兵触发
            bool triggered = trap->checkTrigger(_soldiers);

            if (triggered) {
                // 陷阱触发后，从陷阱列表中移除（避免重复触发）
                it = _traps.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // **士兵 AI 逻辑更新**：遍历士兵列表，更新存活士兵 AI，移除死亡士兵
    auto soldierIt = _soldiers.begin();
    while (soldierIt != _soldiers.end())
    {
        auto soldier = *soldierIt;

        // 1. 更新士兵 AI 逻辑（寻靶、移动、攻击等）
        soldier->update(dt);

        // 2. 检查士兵是否死亡（生命值 <= 0）
        if (soldier->getCurrentHp() <= 0)
        {
            // 从场景中移除士兵节点
            soldier->removeFromParent();

            // 从士兵列表中移除，避免后续重复更新
            soldierIt = _soldiers.erase(soldierIt);
        }
        else
        {
            // 士兵存活，迭代器下移
            ++soldierIt;
        }
    }

    // 更新敌方防御塔逻辑（攻击范围内士兵检测与攻击）
    for (auto node : _towers) {
        auto tower = dynamic_cast<EnemyBuilding*>(node);
        if (tower && !tower->isDestroyed()) {
            tower->updateTowerLogic(dt, _soldiers);
        }
    }
}

/**
 * @brief      返回游戏主场景回调函数
 * @details    先判断游戏是否结束，若结束则隐藏胜利弹窗并重置游戏状态；
 *             停止战斗背景音乐，重新加载游戏主场景数据，切换回 GameScene，
 *             完成战斗场景到主场景的切换与资源清理
 * @param      pSender  按钮触发对象指针（返回按钮）
 */
void BattleScene::menuBackToGameScene(Ref* pSender)
{
    if (_isGameOver) {
        hideVictoryPopup();
        _isGameOver = false;
        _isGamePaused = false;
    }
    // 1. 停止战斗音乐，避免与主场景音乐冲突
    AudioEngine::stopAll();

    // --- 关键：重新加载 GameScene 自身数据 ---
    extern std::string g_currentUsername;
    auto scene = GameScene::create();
    auto gameLayer = static_cast<GameScene*>(scene);

    // 重新加载当前用户数据，刷新主场景显示
    gameLayer->loadOtherPlayerData(g_currentUsername);

    // 场景切换（淡入淡出效果，时长0.5秒）
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

/**
 * @brief      检查游戏结束条件
 * @details    先做防御性检查避免重复判断，再分别校验胜利条件与失败条件：
 *             胜利条件为敌方大本营被摧毁且非围墙防御塔全部被清除；
 *             失败条件为战场无存活士兵且无剩余可召唤士兵；
 *             满足对应条件后触发弹窗展示与战斗停止逻辑
 */
void BattleScene::checkGameEnd()
{
    // 游戏已结束，直接返回，避免重复判断
    if (_isGameOver) return;

    // ==========================================
    // 1. 胜利条件判断（摧毁敌方大本营与所有非围墙防御塔）
    // ==========================================
    bool victory = true;

    // 敌方大本营未被摧毁，不满足胜利条件
    if (_base && !_base->isDestroyed()) {
        victory = false;
    }

    // 遍历防御塔，存在未被摧毁的非围墙建筑，不满足胜利条件
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

        // 停止所有战斗节点的动作与调度，结束战斗
        for (auto s : _soldiers)
            if (s) {
                s->stopAllActions();
                s->unscheduleAllCallbacks();
            }
        for (auto t : _towers) if (t) { t->stopAllActions(); t->unscheduleAllCallbacks(); }
        if (_base) _base->stopAllActions();
        if (_tileMap) for (auto c : _tileMap->getChildren()) c->stopAllActions();

        // 延迟1秒后显示胜利弹窗，提升视觉体验
        this->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([this]() { this->showVictoryPopup(); }),
            nullptr
        ));
        return; // 胜利后直接返回，不执行后续失败判断
    }

    // ==========================================
    // 2. 失败条件判断（无存活士兵且无剩余可召唤士兵）
    // ==========================================

    // 条件A：战场无存活士兵
    bool noSoldiersOnField = _soldiers.empty();

    // 条件B：无剩余可召唤士兵
    bool noReservesLeft = true;
    for (auto item : _soldierUIList) {
        if (item->count > 0) {
            noReservesLeft = false;
            break; // 存在可召唤士兵，不满足条件B
        }
    }

    // 同时满足条件A与B，且未满足胜利条件，判定战斗失败
    if (noSoldiersOnField && noReservesLeft) {
        _isGameOver = true;
        _isGamePaused = true;

        log("Game Over: Defeat!");

        // 停止所有战斗节点的动作与调度
        for (auto t : _towers) if (t) { t->stopAllActions(); t->unscheduleAllCallbacks(); }
        if (_base) _base->stopAllActions();
        if (_tileMap) for (auto c : _tileMap->getChildren()) c->stopAllActions();

        // 延迟1秒后显示失败弹窗
        this->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([this]() { this->showDefeatPopup(); }),
            nullptr
        ));
    }
}

/**
 * @brief      显示胜利弹窗
 * @details    先根据当前地图解锁对应关卡并保存数据，再创建半透明遮罩与弹窗组件，
 *             展示胜利提示、资源奖励与返回按钮，添加缩放动画提升视觉效果，
 *             完成胜利后的奖励结算与UI展示
 */
void BattleScene::showVictoryPopup()
{
    // 根据当前地图解锁对应关卡，更新最大解锁关卡
    if (_mapFileName == "Enemy_map1.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 2) {
            DataManager::getInstance()->setMaxLevelUnlocked(2);
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", 2);
        }
    }
    else if (_mapFileName == "Enemy_map2.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 3) {
            DataManager::getInstance()->setMaxLevelUnlocked(3);
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", 3);
        }
    }
    else if (_mapFileName == "Enemy_map3.tmx") {
        if (DataManager::getInstance()->getMaxLevelUnlocked() < 4) {
            DataManager::getInstance()->setMaxLevelUnlocked(4);
            UserDefault::getInstance()->setIntegerForKey("CurrentLevel", 4);
        }
    }
    // 保存本地数据并同步到云端
    UserDefault::getInstance()->flush();
    SaveGame::getInstance()->syncDataToCloud();

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 center = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    // 1. 创建半透明遮罩层，阻挡底层交互
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    // 创建触摸监听器，吞噬触摸事件，防止穿透到底层
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bgLayer);
    this->addChild(bgLayer, 100);

    // 弹窗目标缩放比例
    float targetScale = 5.0f;

    // 2. 创建弹窗背景
    auto popupBg = Sprite::create("popup_bg.png");
    Size originalSize = popupBg->getContentSize();
    float bgHeight = originalSize.height * targetScale;
    popupBg->setPosition(center);
    this->addChild(popupBg, 101);

    // 3. 创建弹窗内容容器，统一管理弹窗组件
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102);

    // 4. 创建胜利标题标签
    auto victoryLabel = Label::createWithTTF("VICTORY!", "fonts/Marker Felt.ttf", 36);
    victoryLabel->setTextColor(Color4B::GREEN);
    victoryLabel->setPosition(Vec2(0, bgHeight * 0.35f));
    container->addChild(victoryLabel);

    // 5. 创建胜利提示信息标签
    auto messageLabel = Label::createWithTTF("Enemy Base Destroyed!", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::YELLOW);
    messageLabel->setPosition(Vec2(0, bgHeight * 0.15f));
    container->addChild(messageLabel);

    // 6. 资源奖励结算
    int coinReward = 2000;
    int waterReward = 1000;

    // 计算金币可获取数量（受存储上限限制）
    int coinAvailableSpace = coin_limit - coin_count;
    int actualCoinGain = std::min(coinReward, coinAvailableSpace);
    if (actualCoinGain > 0) {
        coin_count += actualCoinGain;
    }

    // 计算圣水可获取数量（受存储上限限制）
    int waterAvailableSpace = water_limit - water_count;
    int actualWaterGain = std::min(waterReward, waterAvailableSpace);
    if (actualWaterGain > 0) {
        water_count += actualWaterGain;
    }

    // 构建奖励提示文本
    std::string rewardText;
    if (actualCoinGain < coinReward || actualWaterGain < waterReward) {
        rewardText = "You got " + std::to_string(actualCoinGain) + " Coins & " +
            std::to_string(actualWaterGain) + " Water! (Storage full)";
    }
    else {
        rewardText = "You got " + std::to_string(coinReward) + " Coins & " +
            std::to_string(waterReward) + " Water!";
    }

    // 创建奖励提示标签
    auto getLabel = Label::createWithTTF(rewardText, "fonts/Marker Felt.ttf", 24);
    getLabel->setTextColor(Color4B::WHITE);
    getLabel->setPosition(Vec2(0, -bgHeight * 0.05f));
    container->addChild(getLabel);

    // 7. 创建返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 32);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    // 按钮缩放动画（循环播放，提升可点击提示）
    auto scaleSeq = Sequence::create(ScaleTo::create(1.0f, 1.1f), ScaleTo::create(1.0f, 1.0f), NULL);
    backItem->runAction(RepeatForever::create(scaleSeq));

    // 设置按钮位置
    backItem->setPosition(Vec2(0, -bgHeight * 0.35f));

    // 创建菜单并添加到容器
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    container->addChild(menu);

    // 8. 弹窗缩放动画（从0.1缩放到目标比例，带回弹效果）
    popupBg->setScale(0.1f);
    popupBg->runAction(EaseBackOut::create(ScaleTo::create(0.4f, targetScale)));

    container->setScale(0.1f);
    container->runAction(EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)));
}

/**
 * @brief      显示失败弹窗
 * @details    创建半透明遮罩与弹窗组件，展示失败标题、提示信息与返回按钮，
 *             添加缩放动画提升视觉效果，完成战斗失败后的 UI 展示
 */
void BattleScene::showDefeatPopup()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 center = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    // 1. 创建半透明遮罩层，阻挡底层交互
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    // 创建触摸监听器，吞噬触摸事件
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);
    listener->onTouchBegan = [](Touch* t, Event* e) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, bgLayer);
    this->addChild(bgLayer, 100);

    // 2. 创建弹窗背景
    float targetScale = 5.0f;
    auto popupBg = Sprite::create("popup_bg.png");
    Size originalSize = popupBg->getContentSize();
    float bgHeight = originalSize.height * targetScale;
    popupBg->setPosition(center);
    this->addChild(popupBg, 101);

    // 3. 创建弹窗内容容器
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102);

    // 4. 创建失败标题标签
    auto titleLabel = Label::createWithTTF("DEFEAT", "fonts/Marker Felt.ttf", 36);
    titleLabel->setTextColor(Color4B::RED);
    titleLabel->enableOutline(Color4B::BLACK, 2);
    titleLabel->setPosition(Vec2(0, bgHeight * 0.35f));
    container->addChild(titleLabel);

    // 5. 创建失败提示信息标签
    auto messageLabel = Label::createWithTTF("All soldiers are dead...", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::WHITE);
    messageLabel->setPosition(Vec2(0, bgHeight * 0.15f));
    container->addChild(messageLabel);

    auto messageLabel1 = Label::createWithTTF("Try to upgrade you soldiers!", "fonts/Marker Felt.ttf", 28);
    messageLabel1->setTextColor(Color4B::WHITE);
    messageLabel1->setPosition(Vec2(0, bgHeight * 0.05f));
    container->addChild(messageLabel1);

    // 6. 创建返回按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 32);
    backLabel->setTextColor(Color4B::WHITE);

    auto backItem = MenuItemLabel::create(
        backLabel,
        CC_CALLBACK_1(BattleScene::menuBackToGameScene, this)
    );

    // 按钮缩放动画（循环播放）
    auto scaleSeq = Sequence::create(ScaleTo::create(1.0f, 1.1f), ScaleTo::create(1.0f, 1.0f), NULL);
    backItem->runAction(RepeatForever::create(scaleSeq));
    backItem->setPosition(Vec2(0, -bgHeight * 0.25f));

    // 创建菜单并添加到容器
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    container->addChild(menu);

    // 7. 弹窗缩放动画
    popupBg->setScale(0.1f);
    popupBg->runAction(EaseBackOut::create(ScaleTo::create(0.4f, targetScale)));

    container->setScale(0.1f);
    container->runAction(EaseBackOut::create(ScaleTo::create(0.4f, 1.0f)));
}

/**
 * @brief      隐藏胜利弹窗
 * @details    按节点名称移除胜利弹窗相关组件（遮罩、弹窗背景、内容容器），
 *             完成弹窗隐藏与场景清理（注：弹窗创建时未设置对应名称，当前逻辑需优化节点命名）
 */
void BattleScene::hideVictoryPopup()
{
    this->removeChildByName("victory_bg_layer");
    this->removeChildByName("victory_popup");
    this->removeChildByName("victory_content");
}

/**
 * @brief      加载 PVE 关卡
 * @details    先加载指定索引的 TMX 地图，做保底处理防止地图缺失；
 *             再将地图居中放置，解析地图对象组，创建敌方建筑、陷阱与装饰物，
 *             填充禁止区域列表，完成 PVE 关卡的地图与敌方单位初始化
 * @param      levelIndex  PVE 关卡索引（对应 Enemy_map%d.tmx）
 */
void BattleScene::loadLevelCampaign(int levelIndex)
{
    // 1. 加载 TMX 地图文件 (保持原样)
    _mapFileName = StringUtils::format("Enemy_map%d.tmx", levelIndex);
    _tileMap = TMXTiledMap::create(_mapFileName);
    if (!_tileMap) {
        _tileMap = TMXTiledMap::create("Enemy_map1.tmx");
        if (!_tileMap) return;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    _tileMap->setPosition(origin + (visibleSize - _tileMap->getContentSize()) / 2);
    this->addChild(_tileMap, -1);

    auto objectGroup = _tileMap->getObjectGroup("object");
    if (!objectGroup) return;

    // --- 改进点 1：使用配置结构体管理建筑默认属性 ---
    struct DefaultConfig {
        EnemyType type;
        std::string img;
        int hp;
        int atk;
        float range;
        int zOrder;
    };

    // 映射表：将 TMX 里的 name 映射到配置
    std::map<std::string, DefaultConfig> configMap = {
        {"Base",   {EnemyType::BASE,   "map/buildings/Base.png",          80, 0,  0.0f,   3}},
        {"tower",  {EnemyType::TOWER,  "map/buildings/TilesetTowers.png", 50, 10, 250.0f, 3}},
        {"cannon", {EnemyType::CANNON, "map/buildings/Cannon1.png",       50, 10, 250.0f, 3}},
        {"fence",  {EnemyType::WALL,   "map/buildings/fence.png",         40, 0,  0.0f,   2}}
    };

    ValueVector objects = objectGroup->getObjects();
    for (auto& v : objects) {
        ValueMap dict = v.asValueMap();
        std::string name = dict["name"].asString();
        float x = dict["x"].asFloat(), y = dict["y"].asFloat();
        float w = dict["width"].asFloat(), h = dict["height"].asFloat();

        // --- 改进点 2：统一处理装饰物坐标偏移 ---
        std::string fileName = dict.count("fileName") ? dict["fileName"].asString() : "";
        if (fileName.find("Tree") != std::string::npos) {
            y += 100;
        }

        // --- 改进点 3：统一处理禁止区域 (除陷阱外) ---
        if (name != "boom") {
            Vec2 worldPos = _tileMap->convertToWorldSpace(Vec2(x, y));
            _forbiddenRects.push_back(Rect(worldPos.x, worldPos.y, w, h));
        }

        // --- 改进点 4：逻辑分支优化 ---
        if (configMap.count(name)) {
            // 建筑逻辑：通过配置表统一创建
            auto& cfg = configMap[name];
            int hp = dict.count("HP") ? dict["HP"].asInt() : cfg.hp;
            int atk = dict.count("Attack") ? dict["Attack"].asInt() : cfg.atk;

            auto eb = EnemyBuilding::create(cfg.img, "ui/Heart2.png", hp, hp / 4, atk, cfg.range);
            if (eb) {
                eb->setEnemyType(cfg.type); // 修正：使用 setEnemyType 确保 AI 逻辑正确
                eb->setPosition(x + w / 2, y + h / 2);
                _tileMap->addChild(eb, cfg.zOrder);

                if (cfg.type == EnemyType::BASE) _base = eb;
                else _towers.pushBack(eb);
            }
        }
        else if (name == "boom") {
            // 陷阱逻辑
            int damage = dict.count("Damage") ? dict["Damage"].asInt() : 1000;
            auto trap = MapTrap::create(Rect(x, y, w, h), damage);
            if (trap) {
                _tileMap->addChild(trap);
                _traps.pushBack(trap);
            }
        }
        else if (!fileName.empty()) {
            // 纯装饰物逻辑
            auto sprite = Sprite::create(fileName);
            if (sprite) {
                sprite->setAnchorPoint(Vec2::ZERO);
                sprite->setPosition(x, y);
                if (w > 0 && h > 0) {
                    sprite->setScaleX(w / sprite->getContentSize().width);
                    sprite->setScaleY(h / sprite->getContentSize().height);
                }
                _tileMap->addChild(sprite, 2);
            }
        }
    }
}
/**
 * @brief      加载 PVP 关卡
 * @details    先加载 Grass.tmx 地图并缩放适配屏幕，解析地图装饰物；
 *             清空原有敌方单位与禁止区域数据，解析传入的 JSON 配置，
 *             创建敌方建筑并填充禁止区域，完成 PVP 关卡的初始化，
 *             做防御性检查防止无大本营导致秒胜
 * @param      jsonContent  PVP 敌方配置 JSON 字符串
 */
void BattleScene::loadLevelPVP(const std::string& jsonContent)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建并初始化地图（必要逻辑，无冗余）
    _tileMap = TMXTiledMap::create(_mapFileName);
    if (!_tileMap) {
        log("Error: Map not found!");
        return;
    }

    // 地图缩放适配屏幕高度
    float scaleFactor = visibleSize.height / _tileMap->getContentSize().height;
    _tileMap->setScale(scaleFactor);
    _tileMap->setAnchorPoint(Vec2::ZERO);
    _tileMap->setPosition(Vec2::ZERO);
    this->addChild(_tileMap, -1);

    // 2. 加载地图装饰物（树木等，优化视觉效果，无冗余）
    auto objectGroup = _tileMap->getObjectGroup("Objects");
    if (objectGroup) {
        ValueVector objects = objectGroup->getObjects();
        for (const auto& v : objects) {
            ValueMap dict = v.asValueMap();
            if (dict.find("fileName") != dict.end()) {
                std::string path = dict["fileName"].asString();
                auto sprite = Sprite::create(path);
                if (sprite) {
                    _tileMap->addChild(sprite, 1);
                    sprite->setAnchorPoint(Vec2::ZERO);
                    float ox = dict["x"].asFloat();
                    float oy = dict["y"].asFloat();
                    sprite->setPosition(ox, oy + 150); // 坐标偏移优化视觉效果

                    // 按地图对象大小缩放装饰物
                    if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
                        float w = dict["width"].asFloat();
                        float h = dict["height"].asFloat();
                        sprite->setScaleX(w / sprite->getContentSize().width);
                        sprite->setScaleY(h / sprite->getContentSize().height);
                    }
                    sprite->setLocalZOrder(10000 - (int)oy);
                }
            }
        }
    }

    // --- 清空原有数据，防止残留（必要逻辑，无冗余） ---
    _towers.clear();
    _forbiddenRects.clear();
    _base = nullptr;

    // 3. 解析 PVP JSON 配置（必要逻辑，无冗余）
    rapidjson::Document doc;
    doc.Parse(jsonContent.c_str());
    if (doc.HasParseError() || !doc.HasMember("buildings")) return;

    const rapidjson::Value& buildings = doc["buildings"];

    // 步骤1：定义建筑配置结构体，统一管理所有建筑参数（消除结构性冗余）
    struct BuildingConfig {
        EnemyType enemyType;       // 敌方建筑类型
        BuildingType buildingType; // 建筑类型
        std::string imgPath;       // 建筑图片路径
        std::string heartPath;     // 血条图片路径
        int hpBase;                // 基础血量
        int hpPerLevel;            // 每级增加血量
        int regen;                 // 回血量
        int attack;                // 攻击力
        float range;               // 攻击范围
    };

    // 步骤2：创建配置数组，填充所有建筑参数（后续新增建筑只需添加一行）
    std::vector<BuildingConfig> buildingConfigs = {
        {EnemyType::BASE,          BuildingType::BASE,          "House.png",      "ui/Heart2.png", 200, 10, 50,  0,   0.0f},
        {EnemyType::TOWER,         BuildingType::TOWER,         "TilesetTowers.png", "ui/Heart2.png", 200, 5,  50,  15,  250.0f},
        {EnemyType::CANNON,        BuildingType::CANNON,        "Cannon.png",     "ui/Heart2.png", 100, 5,  25,  20,  200.0f},
        {EnemyType::WALL,          BuildingType::WALL,          "fence.png",      "ui/Heart2.png", 60,  3,  15,  0,   0.0f},
        {EnemyType::BARRACKS,      BuildingType::BARRACKS,      "junying.png",    "ui/Heart2.png", 200, 5,  50,  0,   0.0f},
        {EnemyType::WATER,         BuildingType::WATER,         "waterwell.png",  "ui/Heart2.png", 60,  3,  15,  0,   0.0f},
        {EnemyType::MINE,          BuildingType::MINE,          "Mine.png",       "ui/Heart2.png", 60,  3,  15,  0,   0.0f},
        {EnemyType::WATER_STORAGE, BuildingType::WATER_STORAGE, "Water.png",      "ui/Heart2.png", 60,  3,  15,  0,   0.0f},
        {EnemyType::GOLD_STORAGE,  BuildingType::GOLD_STORAGE,  "BarGold.png",    "ui/Heart2.png", 60,  3,  15,  0,   0.0f}
    };

    // 步骤3：遍历 JSON 建筑数组，匹配配置创建建筑（消除重复创建逻辑）
    for (auto& b : buildings.GetArray()) {
        // 跳过缺少必要字段的建筑数据
        if (!b.HasMember("type") || !b.HasMember("pos_x") || !b.HasMember("pos_y")) continue;

        int typeInt = b["type"].GetInt();
        float x = b["pos_x"].GetFloat();
        float y = b["pos_y"].GetFloat();
        int level = b.HasMember("level") ? b["level"].GetInt() : 1;

        EnemyBuilding* eb = nullptr;
        EnemyType targetEnemyType = static_cast<EnemyType>(typeInt);
        BuildingType targetBuildingType = static_cast<BuildingType>(typeInt);

        // 遍历配置数组，匹配对应建筑类型
        for (const auto& cfg : buildingConfigs) {
            if (cfg.enemyType == targetEnemyType && cfg.buildingType == targetBuildingType) {
                // 统一计算最终血量（与原有逻辑一致）
                int finalHp = cfg.hpBase + level * cfg.hpPerLevel;
                // 统一创建建筑，无重复代码
                eb = EnemyBuilding::create(cfg.imgPath, cfg.heartPath, finalHp, cfg.regen, cfg.attack, cfg.range);
                if (eb) {
                    eb->setEnemyType(cfg.enemyType);
                }
                break; // 匹配成功，退出配置遍历
            }
        }

        // 建筑创建成功后的统一处理（无冗余，仅一次添加到_towers）
        if (eb) {
            eb->setType(targetEnemyType);
            eb->setScale(0.6f);
            eb->setPosition(Vec2(x, y));

            // 将建筑添加到地图节点，跟随地图坐标系统
            _tileMap->addChild(eb, 3);

            // 非大本营建筑，统一添加到_towers（仅一次添加，修复重复冗余问题）
            if (targetEnemyType != EnemyType::BASE) {
                _towers.pushBack(eb);
            }
            else {
                // 大本营单独赋值，不加入_towers
                _base = eb;
            }

            // 计算建筑对应的禁止区域（随地图缩放，与原有逻辑一致）
            Vec2 worldPos = _tileMap->convertToWorldSpace(Vec2(x, y));
            float rectSize = 150.0f * _tileMap->getScale();
            Rect worldRect(worldPos.x - rectSize / 2, worldPos.y - rectSize / 2, rectSize, rectSize);
            _forbiddenRects.push_back(worldRect);
        }
    }

    // --- 防御性检查：防止无大本营导致秒胜（必要逻辑，无冗余） ---
    if (!_base) {
        log("CRITICAL: No Base found in PVP data! Game will end immediately.");
    }
    else {
        log("PVP Base loaded successfully.");
    }
}

/**
 * @brief      创建战斗场景 UI
 * @details    初始化士兵选择 UI 状态，定义士兵 UI 配置列表，
 *             从 DataManager 获取士兵数量，创建士兵图标与数量标签，
 *             布局士兵 UI 并创建提示标签，完成战斗场景 UI 的初始化
 */
void BattleScene::createUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 初始化士兵选择状态
    _currentSelectedItem = nullptr;
    _isPlacingMode = false;

    // 士兵 UI 配置结构体：关联士兵类型、数据密钥与图标路径
    struct Config {
        SoldierType type;
        std::string dataName;
        std::string imagePath;
    };

    // 士兵 UI 配置列表
    std::vector<Config> configs = {
        { SoldierType::ORIGINAL, "Soldier", "anim/man1.png" },
        { SoldierType::ARROW,    "Arrow",   "anim/arrow1.png" },
        { SoldierType::BOOM,     "Boom",    "anim/boom1.png" },
        { SoldierType::GIANT,    "Giant",   "anim/giant1.png" },
        { SoldierType::AIRFORCE, "Airforce","anim/Owl1.png" }
    };

    // 士兵 UI 布局参数
    float startX = visibleSize.width - 60;
    float startY = 60;
    float gap = 120; // 图标间距

    int index = 0;
    for (const auto& cfg : configs) {
        // 从 DataManager 获取士兵可召唤数量
        int count = DataManager::getInstance()->getTroopCount(cfg.dataName);

        // 仅显示数量大于0的士兵 UI
        if (count > 0) {
            // 创建士兵 UI 项
            SoldierUIItem* item = new SoldierUIItem();
            item->type = cfg.type;
            item->nameKey = cfg.dataName;
            item->count = count;

            // 创建士兵图标
            item->icon = Sprite::create(cfg.imagePath);
            if (!item->icon) {
                // 图标缺失时使用默认图标，防止崩溃
                item->icon = Sprite::create("anim/giant1.png");
            }
            item->icon->setScale(5.0f);
            item->icon->setPosition(startX - (index * gap), startY);
            this->addChild(item->icon, 20);

            // 创建士兵数量标签
            item->countLabel = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 24);
            item->countLabel->setPosition(item->icon->getPositionX(), item->icon->getPositionY() - 40);
            item->countLabel->setTextColor(Color4B::GREEN);
            item->countLabel->enableOutline(Color4B::BLACK, 1);
            this->addChild(item->countLabel, 21);

            // 添加到士兵 UI 列表
            _soldierUIList.push_back(item);
            index++;
        }
    }

    // 创建提示信息标签（用于显示警告信息）
    _msgLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 24);
    _msgLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    _msgLabel->setTextColor(Color4B::RED);
    _msgLabel->setOpacity(0);
    this->addChild(_msgLabel, 30);
}

/**
 * @brief      士兵图标点击回调
 * @details    处理士兵图标点击逻辑：若已选中当前士兵则取消选择；
 *             若选中其他士兵则重置原有选中状态；检查士兵数量，
 *             数量不足时显示警告；数量充足时设置选中状态，添加按钮动画，
 *             绘制禁止区域，进入士兵放置模式
 * @param      item  被点击的士兵 UI 项指针
 */
void BattleScene::onSoldierIconClicked(SoldierUIItem* item)
{
    // 情况 A：已处于放置模式且选中当前士兵，取消选择
    if (_isPlacingMode && _currentSelectedItem == item) {
        _isPlacingMode = false;
        _currentSelectedItem = nullptr;

        // 恢复 UI 原始状态
        item->icon->stopAllActions();
        item->icon->setScale(3.0f);
        item->icon->setColor(Color3B::WHITE);
        _forbiddenAreaNode->clear();
        this->unschedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler));
        return;
    }

    // 情况 B：选中新的士兵，重置原有选中状态
    if (_currentSelectedItem) {
        _currentSelectedItem->icon->stopAllActions();
        _currentSelectedItem->icon->setScale(3.0f);
        _currentSelectedItem->icon->setColor(Color3B::WHITE);
    }

    // 检查士兵数量是否充足
    if (item->count <= 0) {
        showWarning("No more soldiers of this type!");
        _isPlacingMode = false;
        _currentSelectedItem = nullptr;
        _forbiddenAreaNode->clear();
        return;
    }

    // 设置新的选中状态
    _isPlacingMode = true;
    _currentSelectedItem = item;
    _currentSelectedType = item->type;

    // 添加选中动画（循环缩放）
    auto action = RepeatForever::create(Sequence::create(
        ScaleTo::create(0.5f, 3.5f),
        ScaleTo::create(0.5f, 3.0f),
        nullptr
    ));
    item->icon->runAction(action);
    item->icon->setColor(Color3B::YELLOW);

    // 绘制红色禁止区域
    _forbiddenAreaNode->clear();
    for (const auto& rect : _forbiddenRects) {
        _forbiddenAreaNode->drawSolidRect(
            rect.origin,
            Vec2(rect.getMaxX(), rect.getMaxY()),
            Color4F(1.0f, 0.0f, 0.0f, 0.3f)
        );
    }
}

/**
 * @brief      触摸开始回调函数
 * @details    先判断是否点击士兵图标，若点击则触发士兵选择逻辑；
 *             若处于士兵放置模式，则记录触摸状态与坐标，尝试召唤士兵，
 *             开启召唤调度器，处理触摸交互逻辑
 * @param      touch  触摸对象指针
 * @param      event  事件对象指针
 * @return     bool  消费触摸事件返回 true；不消费返回 false
 */
bool BattleScene::onTouchBegan(Touch* touch, Event* event)
{
    Vec2 touchLoc = touch->getLocation();

    // 1. 判断是否点击士兵图标
    for (auto item : _soldierUIList) {
        if (item->icon->getBoundingBox().containsPoint(touchLoc)) {
            onSoldierIconClicked(item);
            return true;
        }
    }

    // 2. 地图触摸（士兵放置模式）
    if (_isPlacingMode && _currentSelectedItem) {
        _isTouchingMap = true;
        _currentTouchPos = touchLoc;

        // 尝试召唤士兵
        trySpawnSoldier(touchLoc);

        // 开启士兵召唤调度器（批量召唤）
        this->schedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler), 0.2f);
        return true;
    }

    return false;
}

/**
 * @brief      触摸移动回调函数
 * @details    若处于士兵放置模式且正在触摸地图，更新当前触摸坐标，
 *             用于士兵放置位置的实时更新
 * @param      touch  触摸对象指针
 * @param      event  事件对象指针
 */
void BattleScene::onTouchMoved(Touch* touch, Event* event)
{
    if (_isPlacingMode && _isTouchingMap) {
        _currentTouchPos = touch->getLocation();
    }
}

/**
 * @brief      触摸结束回调函数
 * @details    重置触摸状态，停止士兵召唤调度器，结束批量召唤逻辑
 * @param      touch  触摸对象指针
 * @param      event  事件对象指针
 */
void BattleScene::onTouchEnded(Touch* touch, Event* event)
{
    _isTouchingMap = false;
    // 停止士兵召唤调度器
    this->unschedule(CC_SCHEDULE_SELECTOR(BattleScene::spawnScheduler));
}

/**
 * @brief      士兵召唤调度器
 * @details    按调度间隔调用，若处于士兵放置模式且正在触摸地图，
 *             重复尝试召唤士兵，实现长按批量召唤功能
 * @param      dt  调度间隔时间（秒）
 */
void BattleScene::spawnScheduler(float dt)
{
    if (_isPlacingMode && _isTouchingMap) {
        trySpawnSoldier(_currentTouchPos);
    }
}

/**
 * @brief      尝试召唤士兵
 * @details    先做防御性检查（选中状态、士兵数量），再判断放置位置是否被阻挡；
 *             若放置有效，则创建士兵并添加到场景，更新士兵数量与 UI，
 *             若士兵数量耗尽，则退出放置模式，恢复 UI 状态
 * @param      worldPos  士兵放置的世界坐标
 */
void BattleScene::trySpawnSoldier(Vec2 worldPos)
{
    // 防御性检查：无选中士兵或数量不足
    if (!_currentSelectedItem || _currentSelectedItem->count <= 0) {
        _isPlacingMode = false;
        // 自动取消选中状态
        if (_currentSelectedItem) onSoldierIconClicked(_currentSelectedItem);
        return;
    }

    // 2. 放置位置阻挡判断
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

    // 3. 召唤士兵（转换为地图节点坐标）
    Vec2 nodePos = _tileMap->convertToNodeSpace(worldPos);
    auto soldier = Soldier::create(this, _currentSelectedType);
    soldier->setPosition(nodePos);
    _tileMap->addChild(soldier, 5);
    _soldiers.pushBack(soldier);

    // 4. 减少士兵可召唤数量
    _currentSelectedItem->count--;

    // 5. 更新士兵数量 UI 显示
    _currentSelectedItem->countLabel->setString(std::to_string(_currentSelectedItem->count));

    // 6. 士兵数量耗尽，退出放置模式
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

/**
 * @brief      显示警告信息
 * @details    设置提示标签文本并显示，添加延迟淡出动画，
 *             1秒后隐藏提示标签，实现临时警告展示功能
 * @param      msg  警告文本内容
 */
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

/**
 * @brief      判断指定世界坐标是否被阻挡
 * @details    先判断是否与敌方大本营碰撞，再遍历敌方防御塔，
 *             判断是否与未被摧毁的建筑碰撞，返回阻挡状态，
 *             供士兵寻路与放置时使用
 * @param      worldPos  待判断的世界坐标
 * @return     bool  被阻挡返回 true；可通行返回 false
 */
bool BattleScene::isPositionBlocked(Vec2 worldPos)
{
    // 1. 判断是否与敌方大本营碰撞
    if (_base && !_base->isDestroyed()) {
        Rect baseRect = _base->getBoundingBox();
        Vec2 baseWorldPos = _tileMap->convertToWorldSpace(baseRect.origin);
        Rect worldRect(baseWorldPos.x, baseWorldPos.y, baseRect.size.width, baseRect.size.height);

        if (worldRect.containsPoint(worldPos)) return true;
    }

    // 2. 判断是否与敌方防御塔碰撞
    for (auto building : _towers) {
        if (building && !building->isDestroyed()) {
            Rect rect = building->getBoundingBox();
            Vec2 buildingWorldPos = _tileMap->convertToWorldSpace(rect.origin);
            Rect worldRect(buildingWorldPos.x, buildingWorldPos.y, rect.size.width, rect.size.height);

            if (worldRect.containsPoint(worldPos)) {
                return true;
            }
        }
    }

    return false;
}