#include "BattleScene.h"
#include "GameScene.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // 务必包含 Soldier 头文件

USING_NS_CC;

Scene* BattleScene::createScene()
{
    return BattleScene::create();
}

bool BattleScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // 初始化交互状态变量
    _isPlacingMode = false;
    _spawnCount = 0;
    _isTouchingMap = false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 加载 TMX 地图 (包含对象解析)
    this->loadEnemyMap();

    // 2. 创建战斗 UI (士兵图标、提示文字)
    this->createUI();

    // 3. 初始化绘制节点 (用于画红色禁止区域)
    _forbiddenAreaNode = DrawNode::create();
    this->addChild(_forbiddenAreaNode, 10); // 层级要在地图之上

    // 4. 添加触摸监听
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(BattleScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(BattleScene::onTouchMoved, this);
    listener->onTouchEnded = CC_CALLBACK_2(BattleScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 5. 添加返回按钮 (保留原有逻辑)
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

    return true;
}

void BattleScene::menuBackToGameScene(Ref* pSender)
{
    auto scene = GameScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void BattleScene::loadEnemyMap()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ================= 1. 创建 Tiled Map =================
    _tileMap = TMXTiledMap::create("Enemy_map1.tmx");

    if (!_tileMap) {
        CCLOG("Error: Failed to load Enemy_map1.tmx");
        return;
    }

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

            // 【关键步骤】处理坐标系
            // 1. 保存 "世界坐标" 的 Rect 到 _forbiddenRects，用于绘制红色区域和点击检测
            Vec2 worldPos = _tileMap->convertToWorldSpace(Vec2(x, y));
            Rect worldRect(worldPos.x, worldPos.y, w, h);
            _forbiddenRects.push_back(worldRect);

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
                    damagePerNotch
                );

                if (_base) {
                    // 修正坐标：让建筑中心对齐对象的中心
                    _base->setPosition(x + w / 2, y + h / 2);
                    _tileMap->addChild(_base, 1);
                }
            }

            // ----------------------------------------------------
            // 处理 B: 防御塔 (tower)
            // ----------------------------------------------------
            else if (name == "tower") {
                int hp = dict["HP"].asInt();
                int atk = dict["Attack"].asInt();
                if (hp == 0) hp = 50;
                if (atk == 0) atk = 5;

                // 防御塔也有 4 个血条状态
                int damagePerNotch = hp / 4; // 示例: 50 / 4 = 12 (整数除法)
                if (damagePerNotch == 0) damagePerNotch = 1;

                auto tower = EnemyBuilding::create(
                    "map/buildings/TilesetTowers.png",
                    "ui/Heart2.png",
                    hp,
                    atk
                );

                if (tower) {
                    tower->setPosition(x + w / 2, y + h / 2);
                    _tileMap->addChild(tower, 1);
                    // 加入列表供士兵寻找
                    _towers.pushBack(tower);
                }
            }

            // ----------------------------------------------------
            // 处理 C: 障碍物 (Tree, grass)
            // ----------------------------------------------------
            else if (name == "Tree" || name == "grass") {
                // 不需要创建实体类，上面的 _forbiddenRects.push_back 已经处理了阻挡逻辑
            }
        }
    }
}

void BattleScene::createUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 士兵图标
    _soldierIcon = Sprite::create("anim/giant1.png"); // 使用士兵图片作为图标
    _soldierIcon->setScale(10.0f);
    // 放置在右下角
    _soldierIcon->setPosition(Vec2(visibleSize.width - 60, 60));
    this->addChild(_soldierIcon, 20);

    // 2. 提示文字 Label
    _msgLabel = Label::createWithTTF("", "fonts/Marker Felt.ttf", 24);
    _msgLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    _msgLabel->setTextColor(Color4B::RED);
    _msgLabel->setOpacity(0);
    this->addChild(_msgLabel, 30);
}

void BattleScene::onSoldierIconClicked()
{
    if (_spawnCount >= MAX_SPAWN) {
        showWarning("Max soldiers reached (10)!");
        return;
    }

    // 切换放置模式
    _isPlacingMode = !_isPlacingMode;

    if (_isPlacingMode) {
        // 浮动交互效果
        auto action = RepeatForever::create(Sequence::create(
            ScaleTo::create(0.5f, 10.5f),
            ScaleTo::create(0.5f,9.5f),
            nullptr
        ));
        _soldierIcon->runAction(action);
        _soldierIcon->setColor(Color3B::YELLOW); // 变色提示选中

        // 【关键】绘制不可放置区域 (红色半透明)
        _forbiddenAreaNode->clear();
        for (const auto& rect : _forbiddenRects) {
            // drawSolidRect 需要左下角和右上角坐标
            _forbiddenAreaNode->drawSolidRect(
                rect.origin,
                Vec2(rect.getMaxX(), rect.getMaxY()),
                Color4F(1.0f, 0.0f, 0.0f, 0.3f) // 红色，30%透明度
            );
        }
    }
    else {
        // 取消模式
        _soldierIcon->stopAllActions();
        _soldierIcon->setScale(0.8f);
        _soldierIcon->setColor(Color3B::WHITE);
        _forbiddenAreaNode->clear(); // 清除红色区域
    }
}

bool BattleScene::onTouchBegan(Touch* touch, Event* event)
{
    Vec2 touchLoc = touch->getLocation();

    // 1. 检测是否点击了 UI 图标
    if (_soldierIcon->getBoundingBox().containsPoint(touchLoc)) {
        onSoldierIconClicked();
        return true; // 吞噬事件
    }

    // 2. 检测地图点击 (放置模式)
    if (_isPlacingMode) {
        _isTouchingMap = true;
        _currentTouchPos = touchLoc;

        // 立即尝试放置一个
        trySpawnSoldier(touchLoc);

        // 【长按逻辑】开启调度器，每0.2秒生成一个
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

void BattleScene::trySpawnSoldier(Vec2 worldPos)
{
    // 1. 检查数量限制
    if (_spawnCount >= MAX_SPAWN) {
        showWarning("Max soldiers reached!");
        _isPlacingMode = false; // 自动退出模式
        onSoldierIconClicked(); // 重置UI状态
        return;
    }

    // 2. 检查碰撞 (不可放置区域)
    bool isBlocked = false;
    // 士兵有个大概的占地面积，比如 20x20，居中检测
    Rect soldierRect(worldPos.x - 10, worldPos.y - 10, 20, 20);

    for (const auto& rect : _forbiddenRects) {
        // intersectsRect 检测两个矩形是否重叠
        if (rect.intersectsRect(soldierRect)) {
            isBlocked = true;
            break;
        }
    }

    if (isBlocked) {
        showWarning("Cannot place here! Select another spot.");
        return;
    }

    // 3. 放置成功：生成士兵
    // 转换到 _tileMap 的局部坐标系，因为士兵应该加在地图上随地图移动
    Vec2 nodePos = _tileMap->convertToNodeSpace(worldPos);

    // 1. 明确指定你要创建的兵种类型
    SoldierType typeToSpawn = SoldierType::GIANT;
    // 2. 调用基类的工厂方法
    auto soldier = Soldier::create(this, typeToSpawn); // <--- 使用 Soldier::create
    soldier->setPosition(nodePos);
    _tileMap->addChild(soldier, 2); // 层级高于建筑

    _soldiers.pushBack(soldier);
    _spawnCount++;
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

// 供其他逻辑调用（如果需要检查某个点是否被阻挡，注意这里输入的是世界坐标）
bool BattleScene::isPositionBlocked(Vec2 worldPos)
{
    for (const auto& rect : _forbiddenRects) {
        if (rect.containsPoint(worldPos)) {
            return true;
        }
    }
    return false;
}