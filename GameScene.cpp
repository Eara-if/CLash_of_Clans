#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "Building.h" 
#include "ShopScene.h"
#include "SaveGame.h"
#include "AudioEngine.h"
USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;

cocos2d::Vector<Building*> g_allPurchasedBuildings;// ????????????????????

int army_limit = 0;
class resource
{
public:
    std::string filename = "";
    int count = 0;

    // ????????? 1??????????????????????????????????
    // ??????????????????????????????????? Node*
    virtual void print(Node* parentNode) {}; // ?麯????????????д
    virtual void modify() {};
    resource(int data) :count(data) {};
    virtual ~resource() {}; // ?????顿??????????????????麯??
};

class goldcoin : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 63;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
        if (coin_count > 0 && coin_count <= coin_limit / 4) {
            start_x = 137;
        }
        else if (coin_count == 0) {
            start_x = 182;
        }
        else if (coin_count > coin_limit / 4 && coin_count <= coin_limit / 2) {
            start_x = 92;
        }
        else if (coin_count > coin_limit / 2 && coin_count <= coin_limit * 0.75) {
            start_x = 47;
        }
        else if (coin_count > coin_limit * 0.75 && coin_count <= coin_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:

    goldcoin() : resource(5000) {};
    ~goldcoin() {};

    void modify() override { // ?????顿???? override ???????????????????
        // ??????
    }

    // ?????????????????????????д????????????? Node*
    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????λ??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 50);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }
    void update() {
    }
};
class water : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 19;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
        if (water_count > 0 && water_count <= water_limit / 4) {
            start_x = 137;
        }
        else if (water_count == 0) {
            start_x = 182;
        }
        else if (water_count > water_limit / 4 && water_count <= water_limit / 2) {
            start_x = 92;
        }
        else if (water_count > water_limit / 2 && water_count <= water_limit * 0.75) {
            start_x = 47;
        }
        else if (water_count > water_limit * 0.75 && water_count <= water_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:
    water() : resource(5000) {};
    ~water() {};

    void modify() override { // ?????顿???? override ???????????????????
        // ??????
    }

    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????λ??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};
class Gem : public resource
{
private:
    cocos2d::Sprite* _displaySprite = nullptr;
    Rect calculateRect() {
        float start_x = 0;
        float start_y = 34;
        float tileWidth = 43;
        float tileHeight = 10;

        // ?????????????
        if (gem_count > 0 && gem_count <= gem_limit / 4) {
            start_x = 137;
        }
        else if (gem_count == 0) {
            start_x = 182;
        }
        else if (gem_count > gem_limit / 4 && gem_count <= gem_limit / 2) {
            start_x = 92;
        }
        else if (gem_count > gem_limit / 2 && gem_count <= gem_limit * 0.75) {
            start_x = 47;
        }
        else if (gem_count > gem_limit * 0.75 && gem_count <= gem_limit) {
            start_x = 2;
        }

        return Rect(start_x, start_y, tileWidth, tileHeight);
    }
public:
    Gem() : resource(5000) {};
    ~Gem() {};

    void modify() override {
    }

    
    void print(Node* parentNode) override {

        // ?????? 2???????????????????
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "ui/06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // ??????
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // ??? scaleAmount ???????????

            // ????λ??
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            // ??????????
            parentNode->addChild(_displaySprite, 2);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. ??????? Rect (??? coin_count ???????)
            Rect newRect = this->calculateRect();

            // 2. ???????????????????????????????????
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};

Scene* GameScene::createScene(Building* purchasedBuilding) {
    auto scene = GameScene::create(); // ???? GameScene ???

    // ????????????????????????????????????浽???????
    if (scene && purchasedBuilding) {
        // ????????????????
        purchasedBuilding->retain();
        g_allPurchasedBuildings.pushBack(purchasedBuilding);
        purchasedBuilding->release(); // GameScene?????????
    }
    return scene;
}


void GameScene::menuGotoBattleCallback(Ref* pSender)
{
    log("Starting Battle...");
    AudioEngine::stopAll();

    auto scene = BattleScene::createScene();

    Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
}
bool GameScene::init()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _tiledMap = TMXTiledMap::create("Grass.tmx");
    if (!_tiledMap) {
        log("Error: Failed to load map!");
        return false;
    }
    this->addChild(_tiledMap, 1); // Z-Order 设为 1 (之前修复触摸冲突改的)

    // ============================================================
    // 【核心修改 1】计算初始缩放，让地图完整显示在屏幕内
    // ============================================================
    Size mapSize = _tiledMap->getContentSize();

    // 计算 X 轴和 Y 轴分别需要的缩放比例
    float scaleX = visibleSize.width / mapSize.width;
    float scaleY = visibleSize.height / mapSize.height;

    // 取两者的较小值，保证地图宽或高完全塞进屏幕，不会被切掉
    float minScale = std::min(scaleX, scaleY);

    // 设置初始缩放
    _tiledMap->setScale(minScale);

    // 立即调用一次边界修正函数，它会自动把缩放后的地图“居中”显示
    this->checkAndClampMapPosition();

    // 【新增】在添加建筑之前，先重新计算人口上限
    this->recalculateArmyLimit();

    // ============================================================
    // 【核心修改 2】计算地图的“物理中心” (Local Coordinate)
    // ============================================================
    // 注意：不要用屏幕中心，要用地图中心！
    Vec2 mapCenterLocal = Vec2(mapSize.width / 2, mapSize.height / 2);
    auto objectGroup = _tiledMap->getObjectGroup("Objects");
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
                    _tiledMap->addChild(sprite, 1);

                    this->addObstacle(sprite);
                    sprite->setAnchorPoint(Vec2::ZERO);
                    sprite->getTexture()->setAliasTexParameters();

                    float x = dict["x"].asFloat();
                    float y = dict["y"].asFloat();

                    sprite->setPosition(x, y + 150);

                    if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
                        float width = dict["width"].asFloat();
                        float height = dict["height"].asFloat();

                        sprite->setScaleX(width / sprite->getContentSize().width);
                        sprite->setScaleY(height / sprite->getContentSize().height);
                    }

                    sprite->setLocalZOrder(10000 - (int)y);
                }
                else {
                    log("Error: Can't load image: %s", path.c_str());
                }
            }
        }
    }

    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setTextColor(Color4B::YELLOW);

    auto backItem = MenuItemFont::create("Back", CC_CALLBACK_1(GameScene::menuBackCallback, this));

    backItem->setColor(Color3B::YELLOW);

    float x = origin.x + backItem->getContentSize().width / 2 + 20;
    float y = origin.y + backItem->getContentSize().height / 2 + 20;
    backItem->setPosition(Vec2(x, y));
    auto menu = Menu::create(backItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);


    bool hasBaseBuilding = false;
    bool hasBarracksBuilding = false;
    for (auto& building : g_allPurchasedBuildings) {
        if (building) {
            if (building->getType() == BuildingType::BASE) {
                hasBaseBuilding = true;
            }
            else if (building->getType() == BuildingType::BARRACKS) {
                hasBarracksBuilding = true;
            }
        }
    }

    // 如果没有基地建筑，创建默认的
    if (!hasBaseBuilding) {
        // 使用 getNearestFreePosition 寻找中心附近最近的空地 (防止中心正好有棵树)
        // 这里的 mapCenterLocal 是地图内部坐标，可以直接传给 getNearestFreePosition (我们之前修过这个函数，它接收 Local)
        // 注意：这里我们还没创建 Building 对象，无法传给 getNearestFreePosition 算大小，
        // 所以我们先简单地传 targetPos，在 setbuilding 内部或者手动微调

        // 简单处理：直接尝试放在中心，稍后 setbuilding 会处理 ZOrder
        this->setbuilding("House.png", Rect::ZERO, "My House", 500, BuildingType::BASE, mapCenterLocal);
        CCLOG("Created default base at Map Center");
    }

    // 创建默认兵营 (放在地图中心稍微偏右一点的位置)
    if (!hasBarracksBuilding) {
        Vec2 barracksPos = mapCenterLocal + Vec2(200, 0); // 往右偏移 200 像素
        this->setbuilding("junying.png", Rect::ZERO, "My junying", 500, BuildingType::BARRACKS, barracksPos);
        CCLOG("Created default barracks near Map Center");
    }



    // 添加所有已购买的建筑
    this->addAllPurchasedBuildings();

    myCoin = new goldcoin();
    myCoin->print(this);

    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
    _coinTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 50, Color4B::WHITE);

    mywater = new water();
    mywater->print(this);
    txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);

    _waterTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 120, Color4B::WHITE);

    mygem = new Gem();
    mygem->print(this);
    txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
    _gemTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 182, Color4B::WHITE);


    this->addShopButton();
    this->addSaveButton(); // 【新增】调用保存游戏按钮布局函数

    isMapDragging = false;

    // 2. 添加触摸监听 (用于移动地图)
    // 注意：我们要把这个监听器设为较低的优先级，或者让它吞噬触摸
    // 但因为我们要点建筑，所以这里不吞噬，而是通过逻辑判断
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true); // 设为true，并在 onTouchBegan 里判断
    touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    touchListener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
    touchListener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, _tiledMap);

    // 3. 添加鼠标监听 (用于滚轮缩放)
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseScroll, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
    return true;
}

Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color)
{
    auto label = Label::createWithTTF(content, "fonts/Marker Felt.ttf", 24);
    if (label) {
        label->setPosition(Vec2(x, y));
        label->setTextColor(color);
        this->addChild(label, 100);
    }
    return label;
}

void GameScene::setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type1, cocos2d::Vec2 position)
{
    auto building = Building::create(filename, rect, name, cost, type1);

    building->setScale(0.5f);
    building->getTexture()->setAliasTexParameters();
    building->setPosition(position);

    building->setOnUpgradeCallback([=]() {

        if (type1 == BuildingType::BASE) {
            extern int coin_limit, water_limit;
            coin_limit += 1500;
            water_limit += 1500;
        }
        // 2. ??????? -> ????????? (?????и??????? army_limit)
        else if (type1 == BuildingType::BARRACKS) {
            this->recalculateArmyLimit();
            log("Barracks upgraded, army limit recalculated to %d", army_limit);
        }

        this->updateResourceDisplay();
        });

    // 4. ?????????
    this->addObstacle(building);
    _tiledMap->addChild(building, 1);
    _allBuildings.pushBack(building);

    // 【新增】如果是兵营，重新计算人口上限
    if (type1 == BuildingType::BARRACKS) {
        this->recalculateArmyLimit();
    }

    // 【关键修改】添加到全局容器，这样保存时才能找到
    if (!g_allPurchasedBuildings.contains(building)) {
        building->retain();
        g_allPurchasedBuildings.pushBack(building);
        building->release();

        CCLOG("=== GameScene: Default building added to global container: %s (type: %d) ===",
            name.c_str(), (int)type1);
    }
}
void GameScene::checkAndClampMapPosition()
{
    // 获取屏幕尺寸
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 获取地图当前的缩放后的实际尺寸
    // 内容尺寸 * 缩放倍率
    float mapWidth = _tiledMap->getContentSize().width * _tiledMap->getScale();
    float mapHeight = _tiledMap->getContentSize().height * _tiledMap->getScale();

    // 获取当前位置
    Vec2 currentPos = _tiledMap->getPosition();

    // ==========================================
    // 核心算法：计算 X 和 Y 的允许范围
    // ==========================================

    // 原理：
    // 1. 如果地图比屏幕小(缩放很小)，通常居中显示。
    // 2. 如果地图比屏幕大(正常情况)，则位置 x 必须在 [minX, 0] 之间。
    //    如果 x > 0，左边就会漏黑边。
    //    如果 x < minX，右边就会漏黑边。

    // --- X 轴限制 ---
    float minX = visibleSize.width - mapWidth; // 这是一个负数
    float maxX = 0;

    if (mapWidth < visibleSize.width) {
        // 如果地图比屏幕窄，直接居中
        currentPos.x = (visibleSize.width - mapWidth) / 2;
    }
    else {
        // 限制在 [minX, 0] 之间
        if (currentPos.x > maxX) currentPos.x = maxX;
        if (currentPos.x < minX) currentPos.x = minX;
    }

    // --- Y 轴限制 ---
    float minY = visibleSize.height - mapHeight; // 这是一个负数
    float maxY = 0;

    if (mapHeight < visibleSize.height) {
        currentPos.y = (visibleSize.height - mapHeight) / 2;
    }
    else {
        if (currentPos.y > maxY) currentPos.y = maxY;
        if (currentPos.y < minY) currentPos.y = minY;
    }

    // 应用修正后的位置
    _tiledMap->setPosition(currentPos);
}
void GameScene::onMouseScroll(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    float scrollY = e->getScrollY();

    // 1. 计算新的缩放值
    float factor = 0.1f;
    float newScale = _tiledMap->getScale() + (scrollY > 0 ? factor : -factor);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Size mapSize = _tiledMap->getContentSize();

    float scaleX = visibleSize.width / mapSize.width;
    float scaleY = visibleSize.height / mapSize.height;
    float minLimit = std::min(scaleX, scaleY); // 这就是“全图显示”的缩放值

    // 限制范围
    if (newScale < minLimit) newScale = minLimit; // 不能比全图更小
    if (newScale > 2.0f) newScale = 2.0f;         // 最大放大倍数

    // 设置缩放
    _tiledMap->setScale(newScale);

    // 修正位置 (保证缩放时居中或贴边)
    checkAndClampMapPosition();
}
bool GameScene::onTouchBegan(Touch* touch, Event* event)
{
    // 记录初始状态，不一定就是拖拽，也可能是想点建筑
    isMapDragging = false;
    return true; // 必须返回 true 才能接收后续的 Moved 和 Ended
}

void GameScene::onTouchMoved(Touch* touch, Event* event)
{
    // 1. 获取移动的距离 (Delta)
    Vec2 delta = touch->getDelta();

    // 2. 如果移动距离极小，可能是手指抖动，忽略
    if (delta.getLength() < 2.0f && !isMapDragging) {
        return;
    }

    // 3. 确认为拖拽模式
    isMapDragging = true;

    // 4. 计算新位置：当前位置 + 偏移量
    Vec2 newPos = _tiledMap->getPosition() + delta;
    _tiledMap->setPosition(newPos);

    // 5. 【关键】实时修正位置，防止拖出界
    checkAndClampMapPosition();
}

void GameScene::onTouchEnded(Touch* touch, Event* event)
{

    isMapDragging = false;
}
void GameScene::menuBackCallback(Ref* pSender)
{
    log("Back button clicked!");
    // 1. ???? HelloWorld ????
    // ???????????????????
    auto scene = HelloWorld::createScene();

    // 2. ?л????? (????? 0.5?? ??????Ч)
    // ???????????? TransitionSlideInL::create(...) ????????Ч??
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void GameScene::addShopButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ????????????????????
    auto shopNormal = Sprite::create("ui/shop_normal.png"); // ??????????????
    auto shopSelected = Sprite::create("ui/shop_selected.png"); // ?????????????

    // ????????????????????????????
    if (!shopNormal) {
        // ?????????????
        shopNormal = Sprite::create();
        shopNormal->setTextureRect(Rect(0, 0, 150, 80));
        shopNormal->setColor(Color3B(255, 200, 0)); // ???

        auto shopLabel = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 30);
        shopLabel->setPosition(Vec2(75, 40));
        shopLabel->setColor(Color3B::WHITE);
        shopNormal->addChild(shopLabel);
    }

    if (!shopSelected) {
        shopSelected = Sprite::create();
        shopSelected->setTextureRect(Rect(0, 0, 85, 85));
        shopSelected->setColor(Color3B(255, 150, 0)); // ???????
    }

    auto shopItem = MenuItemSprite::create(shopNormal, shopSelected,
        [](Ref* pSender) {
            // ???pushScene??????replaceScene
            auto scene = ShopScene::createScene();
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        }
    );


    // ??????λ?????????
    float x = origin.x + 150; // ???λ??
    float y = origin.y + visibleSize.height - 100; // ????λ??

    shopItem->setPosition(Vec2(x, y));

    // ???????????
    auto shopMenu = Menu::create(shopItem, NULL);
    shopMenu->setPosition(Vec2::ZERO);
    this->addChild(shopMenu, 200); // ????????

}

void GameScene::addAllPurchasedBuildings() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Size mapSize = _tiledMap->getContentSize();
    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            _tiledMap->addChild(building, 10);

            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                Vec2 mapCenter = Vec2(mapSize.width / 2, mapSize.height / 2);

                // 从地图中心开始找最近的空位
                Vec2 pos = getNearestFreePosition(building, mapCenter);

                building->setPosition(pos);
            }

            // ???????????????
            building->setOnUpgradeCallback([=]() {
                // 重新计算人口上限（如果是兵营或大本营）
                if (building->getType() == BuildingType::BARRACKS ) {
                    this->recalculateArmyLimit();
                }
                if (_coinTextLabel) {
                    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
                    _coinTextLabel->setString(txt);
                }
                if (_waterTextLabel) {
                    std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
                    _waterTextLabel->setString(txt);
                }
                if (_gemTextLabel) {
                    std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
                    _gemTextLabel->setString(txt);
                }
                // 更新资源显示
                this->updateResourceDisplay();
                });

            // ?????????????
            _allBuildings.pushBack(building);
        }
    }
    // 【新增】添加完所有建筑后，重新计算人口上限
    this->recalculateArmyLimit();
}

void GameScene::onEnter() {
    Scene::onEnter();
    AudioEngine::stopAll();
    this->updateResourceDisplay();

    // ???????????????????????????????
    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            _tiledMap->addChild(building, 10);
            Size mapSize = _tiledMap->getContentSize();
            auto visibleSize = Director::getInstance()->getVisibleSize();
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {

                Vec2 center = Vec2(mapSize.width / 2, mapSize.height / 2);
                Vec2 pos = getNearestFreePosition(building, center);
                auto x = pos.x;
                auto y = pos.y;
                building->setPosition(x, y);
            }
            this->addObstacle(building);
            // 刷新UI
            building->setOnUpgradeCallback([=]() {
                // 重新计算人口上限（如果是兵营或大本营）
                if (building->getType() == BuildingType::BARRACKS) {
                    this->recalculateArmyLimit();
                }
                if (_coinTextLabel) {
                    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
                    _coinTextLabel->setString(txt);
                }
                if (_waterTextLabel) {
                    std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
                    _waterTextLabel->setString(txt);
                }
                if (_gemTextLabel) {
                    std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
                    _gemTextLabel->setString(txt);
                }
                this->updateResourceDisplay();
                });

            _allBuildings.pushBack(building);
        }
    }
    // 【新增】进入场景后重新计算人口上限
    this->recalculateArmyLimit();
    AudioEngine::play2d("music/1.ogg", true, 0.5f);
}

void GameScene::updateResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ?????????
    if (_coinTextLabel) {
        std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
        _coinTextLabel->setString(txt);
    }
    if (myCoin) {
        myCoin->refresh();
    }

    // ????????
    if (_waterTextLabel) {
        std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
        _waterTextLabel->setString(txt);
    }
    if (mywater) {
        mywater->refresh();
    }

    // ???±?????
    if (_gemTextLabel) {
        std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
        _gemTextLabel->setString(txt);
    }
    if (mygem) {
        mygem->refresh();
    }
}
// GameScene.cpp



void GameScene::removeObstacle(Node* node)
{
    if (_obstacles.contains(node)) {
        _obstacles.eraseObject(node);
    }
}

void GameScene::addObstacle(Node* node) {
    if (node && !_obstacles.contains(node)) {
        _obstacles.pushBack(node);
    }
}

Rect GameScene::getWorldBoundingBox(Node* node)
{
    if (!node->getParent()) return Rect::ZERO;

    Rect rect = node->getBoundingBox();
    Vec2 worldOrigin = node->getParent()->convertToWorldSpace(rect.origin);
    Rect worldRect = Rect(worldOrigin.x, worldOrigin.y, rect.size.width, rect.size.height);
    return worldRect;
}

bool GameScene::checkCollision(Rect targetRect, Node* ignoreNode)
{
    for (auto obstacle : _obstacles)
    {
        if (obstacle == ignoreNode) continue;

        // ??????????????Ч
        if (!obstacle->getParent()) continue;

        // ?????????????Χ??
        Rect obstacleRect = obstacle->getBoundingBox();

        // ??????????????ж??????????κΡ??????
        if (targetRect.intersectsRect(obstacleRect))
        {
            return true; // ???
        }
    }
    return false; // ???
}
Vec2 GameScene::getNearestFreePosition(Building* building, Vec2 targetMapPos)
{
    Size size = building->getContentSize();
    float w = size.width * building->getScaleX();
    float h = size.height * building->getScaleY();
    // 2. ????????
    int step = (int)(w / 2);
    if (step < 10) step = 10;
    int maxRadius = 30;

    for (int r = 1; r <= maxRadius; r++) {
        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                if (std::abs(x) != r && std::abs(y) != r) continue;

                Vec2 candidateMapPos = targetMapPos + Vec2(x * step, y * step);

                // ====================================================
                // 【修改这里】不要转 WorldSpace，直接用 Local 算！
                // ====================================================

                // A. 这里的 candidateMapPos 本身就是 Local 的
                // B. 这里的 w 和 h 本身也是 Local 的 (Building 的原始大小)
                Rect testLocalRect = Rect(
                    candidateMapPos.x - w * 0.5f,
                    candidateMapPos.y - h * 0.5f,
                    w, h
                );

                // C. 直接传入 Local Rect
                if (!checkCollision(testLocalRect, nullptr)) {
                    return candidateMapPos;
                }
            }
        }
    }
    return Vec2::ZERO;
}
void GameScene::addSaveButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建保存按钮
    auto saveLabel = Label::createWithTTF("SAVE GAME", "fonts/Marker Felt.ttf", 28);
    saveLabel->setColor(Color3B::GREEN);
    saveLabel->enableOutline(Color4B::BLACK, 2);

    auto saveItem = MenuItemLabel::create(saveLabel,
        CC_CALLBACK_1(GameScene::menuSaveGameCallback, this));

    // 设置按钮位置（右下角）
    float x = origin.x + visibleSize.width - 150;
    float y = origin.y + 100;

    saveItem->setPosition(Vec2(x, y));

    // 【新增】调试按钮
    auto debugLabel = Label::createWithTTF("DEBUG SAVE", "fonts/Marker Felt.ttf", 20);
    debugLabel->setColor(Color3B::YELLOW);

    auto debugItem = MenuItemLabel::create(debugLabel,
        [](Ref* pSender) {
            CCLOG("=== Debug Save System ===");
            SaveGame::getInstance()->debugWritablePath();

            // 尝试保存和加载测试数据
            auto saveGame = SaveGame::getInstance();
            bool saveResult = saveGame->saveGameState("test_save.json");
            CCLOG("Test save result: %s", saveResult ? "SUCCESS" : "FAILED");

            bool loadResult = saveGame->loadGameState("test_save.json");
            CCLOG("Test load result: %s", loadResult ? "SUCCESS" : "FAILED");
        });

    // 创建菜单容器
    auto saveMenu = Menu::create(saveItem, NULL);
    saveMenu->setPosition(Vec2::ZERO);
    this->addChild(saveMenu, 200);
}

void GameScene::menuSaveGameCallback(Ref* pSender) {
    log("Saving game...");

    // 调用SaveGame单例保存游戏
    SaveGame::getInstance()->saveGameState();
}

int GameScene::calculateArmyLimit()
{
    int totalLimit = 0; // 基础人口上限

    // 遍历所有建筑，找到兵营并累加它们提供的人口上限
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BARRACKS) {
            // 每个兵营提供的基础人口 + 每级额外增加的人口
            // 例如：1级兵营提供10人口，每升一级增加10人口
            totalLimit += 10 * building->getLevel();
        }
    }


    CCLOG("=== GameScene: Calculated army limit: %d ===", totalLimit);
    return totalLimit;
}

void GameScene::recalculateArmyLimit()
{
    army_limit = calculateArmyLimit();
    CCLOG("=== GameScene: Army limit recalculated to: %d ===", army_limit);

    // 更新UI显示（如果需要）
    this->updateResourceDisplay();
}