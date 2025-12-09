#include "BattleScene.h"
#include "GameScene.h"
#include "EnemyBuilding.h"
#include "Soldier.h" // 务必包含 Soldier 头文件
#include"DataManager.h"

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

// 【新增】检查游戏是否结束
void BattleScene::checkGameEnd()
{
    // 如果已经游戏结束，直接返回
    if (_isGameOver) return;

    // 检查大本营是否被摧毁
    if (_base && _base->isDestroyed()) {
        _isGameOver = true;

        // 暂停游戏逻辑，但保持UI交互
        _isGamePaused = true;

        // 延迟显示胜利弹窗，让玩家看到大本营摧毁的效果
        this->runAction(Sequence::create(
            DelayTime::create(1.0f),
            CallFunc::create([this]() {
                this->showVictoryPopup();
                }),
            nullptr
        ));
    }
}

// 【新增】显示胜利弹窗
void BattleScene::showVictoryPopup()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 center = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);

    // 1. 创建半透明黑色背景层（防止点击弹窗后的元素）
    auto bgLayer = LayerColor::create(Color4B(0, 0, 0, 180), visibleSize.width, visibleSize.height);
    bgLayer->setPosition(Vec2::ZERO);
    bgLayer->setName("victory_bg_layer");
    this->addChild(bgLayer, 100);

    // 2. 创建弹窗背景
    auto popupBg = Sprite::create("popup_bg.png");
    if (!popupBg) {
        // 如果图片不存在，创建一个替代的背景
        popupBg = Sprite::create();
        auto drawNode = DrawNode::create();
        drawNode->drawSolidRect(Vec2(-150, -200), Vec2(150, 200), Color4F(0.2f, 0.2f, 0.2f, 0.95f));
        popupBg->addChild(drawNode);
        popupBg->setContentSize(Size(300, 400));
    }

    // 先获取原始内容尺寸
    Size originalSize = popupBg->getContentSize();

    // 将背景缩放到4倍
    popupBg->setScale(4.0f);

    // 计算缩放后的新尺寸
    Size scaledSize = Size(originalSize.width * 4.0f, originalSize.height * 4.0f);

    popupBg->setPosition(center);
    popupBg->setName("victory_popup");
    this->addChild(popupBg, 101);

    // 3. 创建一个容器节点，将其添加到场景中，位置与背景中心对齐
    auto container = Node::create();
    container->setPosition(center);
    this->addChild(container, 102); // 层级比背景高

    // 4. 胜利标题 - 添加到容器中，而不是背景上
    auto victoryLabel = Label::createWithTTF("VICTORY!", "fonts/Marker Felt.ttf", 36);
    victoryLabel->setTextColor(Color4B::GREEN);
    victoryLabel->setPosition(Vec2(0, scaledSize.height / 2 - 80));
    container->addChild(victoryLabel);

    // 5. 胜利信息 - 添加到容器中
    auto messageLabel = Label::createWithTTF("Enemy Base Destroyed!", "fonts/Marker Felt.ttf", 28);
    messageLabel->setTextColor(Color4B::YELLOW);
    messageLabel->setPosition(Vec2(0, scaledSize.height / 2 - 140));
    container->addChild(messageLabel);

    // 7. 添加弹窗动画效果 - 背景和容器分别做动画
    popupBg->setScale(0.1f);
    popupBg->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 4.0f)));

    container->setScale(0.1f);
    container->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)));
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
                    damagePerNotch,
                    0,
                    0.0f // Base 不攻击
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
        { SoldierType::GIANT,    "Giant",   "anim/giant1.png" }
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
            item->icon->setScale(10.0f); // 调整大小
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
    _tileMap->addChild(soldier, 2);
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