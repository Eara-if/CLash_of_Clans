#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "Building.h" 
#include "ShopScene.h"
USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;

cocos2d::Vector<Building*> g_allPurchasedBuildings;// 新增：全局已购买建筑容器

int army_limit = 10;
class resource
{
public:
    std::string filename = ""; // 【修正】不要用 NULL
    int count = 0;

    // 这些不需要存为成员变量，除非别的地方要用
    // float tileWidth = 43;  
    // float tileHeight = 8; 

    // 【关键修正 1】为了实现多态，基类的参数必须和派生类一致
    // 我们可以给它一个默认参数，或者在基类也加上 Node*
    virtual void print(Node* parentNode) {}; // 虚函数，允许被重写

    virtual void modify() {};

    resource(int data) :count(data) {};
    virtual ~resource() {}; // 【建议】基类析构函数最好是虚函数
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

        // 复用你之前的逻辑
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

    void modify() override { // 【建议】加上 override 关键字，编译器会帮你检查
        // 修改逻辑
    }

    // 【修正】现在这就是真正的重写了，因为参数也是 Node*
    void print(Node* parentNode) override {

        // 【修正 2】在函数内部获取屏幕尺寸
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // 安全检查
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // 确保 scaleAmount 在基类初始化了

            // 设置位置
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 50);

            // 添加到父节点
            parentNode->addChild(_displaySprite, 0);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. 重新计算 Rect (因为 coin_count 已经变了)
            Rect newRect = this->calculateRect();

            // 2. 【核心】只修改精灵的显示区域，不用重新创建！
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

        // 复用你之前的逻辑
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

    void modify() override { // 【建议】加上 override 关键字，编译器会帮你检查
        // 修改逻辑
    }

    void print(Node* parentNode) override {

        // 【修正 2】在函数内部获取屏幕尺寸
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // 安全检查
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // 确保 scaleAmount 在基类初始化了

            // 设置位置
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);

            // 添加到父节点
            parentNode->addChild(_displaySprite, 0);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. 重新计算 Rect (因为 coin_count 已经变了)
            Rect newRect = this->calculateRect();

            // 2. 【核心】只修改精灵的显示区域，不用重新创建！
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

        // 复用你之前的逻辑
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

        // 【修正 2】在函数内部获取屏幕尺寸
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";


        Rect tileRect = this->calculateRect();

        _displaySprite = Sprite::create(filename, tileRect);

        // 安全检查
        if (_displaySprite) {
            _displaySprite->getTexture()->setAliasTexParameters();
            _displaySprite->setScale(6.0f); // 确保 scaleAmount 在基类初始化了

            // 设置位置
            _displaySprite->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            // 添加到父节点
            parentNode->addChild(_displaySprite, 0);
        }
    }
    void refresh() {
        if (_displaySprite != nullptr) {
            // 1. 重新计算 Rect (因为 coin_count 已经变了)
            Rect newRect = this->calculateRect();

            // 2. 【核心】只修改精灵的显示区域，不用重新创建！
            _displaySprite->setTextureRect(newRect);

            // log("Coin icon refreshed!"); 
        }
    }

    void update() {
    }
};

Scene* GameScene::createScene(Building* purchasedBuilding) {
    auto scene = Scene::create();
    auto layer = GameScene::create(); // 创建 GameScene 实例

    // 【关键修改】不再将建筑添加到新场景，而是保存到全局容器
    if (layer && purchasedBuilding) {
        // 确保建筑被正确管理
        purchasedBuilding->retain();
        g_allPurchasedBuildings.pushBack(purchasedBuilding);
        purchasedBuilding->release(); // GameScene现在持有它
    }

    scene->addChild(layer);
    return scene;
}

void GameScene::addPurchasedBuilding(Building* building) {
    if (!building) return;

    // 如果建筑已经在全局容器中，确保它不被重复添加
    if (g_allPurchasedBuildings.contains(building)) {
        // 从父节点移除（如果已经添加到了某个场景）
        if (building->getParent()) {
            building->removeFromParent();
        }
    }
    else {
        // 添加到全局容器
        building->retain();
        g_allPurchasedBuildings.pushBack(building);
        building->release();
    }

    this->addChild(building, 10); // 添加到场景，层级与原有建筑一致
    _allBuildings.pushBack(building); // 存入容器，方便统一管理

    // 可以设置一个初始位置，例如屏幕中心
    auto visibleSize = Director::getInstance()->getVisibleSize();
    building->setPosition(visibleSize.width / 2, visibleSize.height / 2);

    // 【重要】设置建筑的回调，用于资源更新等
    building->setOnUpgradeCallback([=]() {
        // 这里可以放置建筑升级时更新主界面资源的逻辑
        // 可以参考你原有的 setbuilding 函数中的回调
        if (_coinTextLabel) {
            std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
            _coinTextLabel->setString(txt);
        }
        // ... 更新其他UI ...
        });
}

void GameScene::setBattleButton()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 1. 创建文字标签
    // 【关键修正】：直接使用 MenuItemFont::create，并指定 TTF 文件路径
    // 这样可以避免手动创建 Label 时可能出现的坐标或属性问题。
    auto startBattleItem = MenuItemFont::create(
        "Ready To Fight", // 按钮显示的文字
        CC_CALLBACK_1(GameScene::menuGotoBattleCallback, this)
    );

    // 设置字体：必须指定 TTF 文件路径和大小
    // 这里的路径必须和 showText 中使用的路径一致
    startBattleItem->setFontNameObj("fonts/Marker Felt.ttf");
    startBattleItem->setFontSize(48);

    // 设置颜色 (可选，防止颜色被 MenuItemFont 默认的白色或灰色覆盖)
    startBattleItem->setColor(Color3B::WHITE);

    // 设置位置：在大本营下方
    startBattleItem->setPosition(
        Vec2(origin.x + visibleSize.width / 2, origin.y + 200)
    );

    // 3. 创建菜单并添加
    auto battleMenu = Menu::create(startBattleItem, NULL);
    battleMenu->setPosition(Vec2::ZERO);

    // Z-Order: 确保这个菜单层级足够高，不会被其他 Sprite 覆盖
    this->addChild(battleMenu, 10); // 设置一个较高的 Z-order，例如 10

}

void GameScene::menuGotoBattleCallback(Ref* pSender)
{
    log("Starting Battle...");
    // 1. 创建新的战斗场景
    auto scene = BattleScene::createScene();

    // 2. 切换场景，使用pushScene保留当前场景
    Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
}
bool GameScene::init()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _tiledMap = TMXTiledMap::create("Grass.tmx");
    if (_tiledMap) {
        // 2. 设置锚点和位置 (从左下角开始)
        _tiledMap->setAnchorPoint(Vec2::ZERO);
        _tiledMap->setPosition(Vec2::ZERO);

        // 3. 【可选】设置缩放
        // 如果你觉得地图太小，可以放大 (比如 2.0f)
        // 注意：Tilemap 放大可能会有黑边缝隙，这是正常现象，可以通过修改 ccConfig.h 解决，暂时先不管
        _tiledMap->setScale(1.0f);

        // 4. 添加到场景最底层 (-99)
        this->addChild(_tiledMap, -99);
    }
    else {
        log("Error: Failed to load map! Check filename and Resources folder.");
    }
    // 1. 获取对象层 (假设你在 Tiled 里把层命名为 "Objects")
// 如果你的层名是 "Trees"，请自行修改
    // 1. 获取对象层 (请确保 "Objects" 和 Tiled 里的层名一致！)
    auto objectGroup = _tiledMap->getObjectGroup("Objects");

    if (objectGroup) {
        ValueVector objects = objectGroup->getObjects();

        for (const auto& v : objects) {
            ValueMap dict = v.asValueMap();

            // ============================================================
            // 【核心修改】不再写死文件名，而是读取 Tiled 里的自定义属性
            // ============================================================

            // 2. 检查这个对象是否有 "fileName" 这个属性
            if (dict.find("fileName") != dict.end())
            {
                // 3. 读取属性值 (比如 "map/background/tree.png")
                std::string path = dict["fileName"].asString();

                // 4. 使用读到的路径创建精灵
                auto sprite = Sprite::create(path);

                if (sprite) {
                    // ============================================================
                    // 【核心修改】添加这一行！
                    // 将锚点设为左下角 (0, 0)，对齐 Tiled 的坐标系
                    // ============================================================
                    sprite->setAnchorPoint(Vec2::ZERO);
                    sprite->getTexture()->setAliasTexParameters();
                    // --- 下面是通用的位置和缩放逻辑 ---

                    float x = dict["x"].asFloat();
                    float y = dict["y"].asFloat();

                    // 设置位置 (现在 x,y 对应的就是图片的左下角了，位置就准了)
                    sprite->setPosition(x, y);

                    // 处理缩放 (如果在 Tiled 里拉伸过)
                    if (dict.find("width") != dict.end() && dict.find("height") != dict.end()) {
                        float width = dict["width"].asFloat();
                        float height = dict["height"].asFloat();

                        // 目标尺寸 / 原图尺寸 = 缩放倍率
                        sprite->setScaleX(width / sprite->getContentSize().width);
                        sprite->setScaleY(height / sprite->getContentSize().height);
                    }

                    // 处理旋转 (如果在 Tiled 里旋转过)
                    if (dict.find("rotation") != dict.end()) {
                        sprite->setRotation(dict["rotation"].asFloat());
                    }

                    // 5. 添加到地图上
                    _tiledMap->addChild(sprite);

                    // 设置层级 (Y越大越靠后，简单的遮挡关系)
                    sprite->setLocalZOrder(10000 - (int)y);
                }
                else {
                    // 如果图片路径写错了，打印报错方便调试
                    log("Error: Can't load image: %s", path.c_str());
                }
            }
        }
    }
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setTextColor(Color4B::YELLOW); // 黄色文字

    // 2. 创建菜单项
    // 告诉编译器，backLabel 肯定是一个 Label 指针
    auto backItem = MenuItemFont::create("Back", CC_CALLBACK_1(GameScene::menuBackCallback, this));

    // 2. 设置颜色 (显眼一点)
    backItem->setColor(Color3B::YELLOW);

    // 3. 设置按钮的具体坐标 (例如：左下角，往里缩一点)
    // 注意：backItem->getContentSize() 是为了让按钮中心不贴边
    float x = origin.x + backItem->getContentSize().width / 2 + 20;
    float y = origin.y + backItem->getContentSize().height / 2 + 20;
    backItem->setPosition(Vec2(x, y));

    // 4. 创建菜单容器
    auto menu = Menu::create(backItem, NULL);

    // ====================================================
    // 【关键点 A】必须把菜单容器的位置设为原点 (0,0)
    // 否则 backItem 的坐标是相对于屏幕中心的，会导致错位！
    menu->setPosition(Vec2::ZERO);
    // ====================================================

    // ====================================================
    // 【关键点 B】设置层级为 100 (确保在背景之上)
    this->addChild(menu, 100);

    Vec2 housePos = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

    // 调用通用函数！
    // 注意：传入 my_house_level 这个全局变量
    this->setbuilding("House.png", Rect::ZERO, "My House", 500, BuildingType::BASE, housePos);

    Vec2 junyingPos = Vec2(origin.x + visibleSize.width / 2 + 200, origin.y + visibleSize.height / 2);
    this->setbuilding("junying.png", Rect::ZERO, "My junying", 200, BuildingType::BARRACKS, junyingPos);

    // 添加所有已购买的建筑
    this->addAllPurchasedBuildings();

    // ... 原有代码 ...
    myCoin = new goldcoin();
    myCoin->print(this);

    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
    _coinTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 50, Color4B::WHITE);

    mywater = new water();
    mywater->print(this);
    txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
    // 【修改点】保存水 Label
    _waterTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 120, Color4B::WHITE);

    mygem = new Gem();
    mygem->print(this);
    txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
    // 【修改点】保存宝石 Label
    _gemTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 182, Color4B::WHITE);

    this->setBattleButton(); // 【新增】调用战斗按钮布局函数
    this->addShopButton(); // 【新增】调用商城按钮布局函
    return true;
}

// 【修改返回值】从 void 改为 Label*
Label* GameScene::showText(std::string content, float x, float y, cocos2d::Color4B color)
{
    auto label = Label::createWithTTF(content, "fonts/Marker Felt.ttf", 24);
    if (label) {
        label->setPosition(Vec2(x, y));
        label->setTextColor(color);
        this->addChild(label, 100);
    }
    // 【新增】把创建好的 label 返回出去
    return label;
}


// 【注意】函数头的参数列表必须和 .h 文件一字不差
void GameScene::setbuilding(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int cost, BuildingType type1, cocos2d::Vec2 position)
{
    // 1. 创建建筑
    auto building = Building::create(filename, rect, name, cost, type1);

    // 2. 设置属性
    building->setScale(0.5f);
    building->getTexture()->setAliasTexParameters();
    building->setPosition(position);

    // 3. 设置升级回调
    // 【修正点】注意这个 Lambda 是 [=]()，括号里不要填参数！
    // 之前的报错 C2440 就是因为你可能在这里面填了参数
    building->setOnUpgradeCallback([=]() {

        if (type1 == BuildingType::BASE) {
            extern int coin_limit, water_limit;
            coin_limit += 1500;
            water_limit += 1500;
        }
        // 2. 如果是兵营 -> 加人口上限 (假设有个全局变量 army_limit)
        else if (type1 == BuildingType::BARRACKS) {
            extern int army_limit;
            army_limit += 10;
            log("Army limit increased to %d", army_limit);
        }

        // B. 刷新金币 UI
        if (this->_coinTextLabel != nullptr) {
            std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
            this->_coinTextLabel->setString(txt);
        }
        if (this->myCoin != nullptr) {
            this->myCoin->refresh();
        }

        // C. 刷新水 UI
        if (this->_waterTextLabel != nullptr) {
            std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
            this->_waterTextLabel->setString(txt);
        }
        if (this->mywater != nullptr) {
            this->mywater->refresh();
        }

        // D. 刷新宝石 UI
        if (this->_gemTextLabel != nullptr) {
            std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
            this->_gemTextLabel->setString(txt);
        }
        });

    // 4. 添加到场景
    this->addChild(building, 1);
}


void GameScene::menuBackCallback(Ref* pSender)
{
    log("Back button clicked!");
    // 1. 创建 HelloWorld 场景
    // 也就是回到最初的那个场景
    auto scene = HelloWorld::createScene();

    // 2. 切换场景 (带一个 0.5秒 的翻页特效)
    // 你可以试着换成 TransitionSlideInL::create(...) 试试不同的效果
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}

void GameScene::addShopButton() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 使用图片按钮而不是文字按钮
    auto shopNormal = Sprite::create("ui/shop_normal.png"); // 你需要创建这个图片
    auto shopSelected = Sprite::create("ui/shop_selected.png"); // 或者使用同一张图片

    // 如果图片不存在，创建一个简单的按钮替代
    if (!shopNormal) {
        // 创建简单的圆形按钮
        shopNormal = Sprite::create();
        shopNormal->setTextureRect(Rect(0, 0, 150, 80));
        shopNormal->setColor(Color3B(255, 200, 0)); // 金色

        auto shopLabel = Label::createWithTTF("MARKET", "fonts/Marker Felt.ttf", 30);
        shopLabel->setPosition(Vec2(75, 40));
        shopLabel->setColor(Color3B::WHITE);
        shopNormal->addChild(shopLabel);
    }

    if (!shopSelected) {
        shopSelected = Sprite::create();
        shopSelected->setTextureRect(Rect(0, 0, 85, 85));
        shopSelected->setColor(Color3B(255, 150, 0)); // 更深的金色
    }

    auto shopItem = MenuItemSprite::create(shopNormal, shopSelected,
        [](Ref* pSender) {
            // 使用pushScene而不是replaceScene
            auto scene = ShopScene::createScene();
            Director::getInstance()->pushScene(TransitionFade::create(0.5f, scene));
        }
    );


    // 设置按钮位置（左上角）
    float x = origin.x + 150; // 左侧位置
    float y = origin.y + visibleSize.height - 100; // 顶部位置

    shopItem->setPosition(Vec2(x, y));

    // 创建菜单容器
    auto shopMenu = Menu::create(shopItem, NULL);
    shopMenu->setPosition(Vec2::ZERO);
    this->addChild(shopMenu, 200); // 使用更高的层级

}

// 新增：添加所有已购买建筑的函数
void GameScene::addAllPurchasedBuildings() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            this->addChild(building, 10);

            // 确保建筑有一个默认位置（如果当前位置是0,0）
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                building->setPosition(visibleSize.width / 2, visibleSize.height / 2);
            }

            // 重新设置升级回调
            building->setOnUpgradeCallback([=]() {
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
                });

            // 添加到本地容器
            _allBuildings.pushBack(building);
        }
    }
}

// 新增：重写onEnter函数，确保从商城返回时建筑被正确添加
void GameScene::onEnter() {
    Scene::onEnter();

    // 从商城返回时，更新资源显示
    this->updateResourceDisplay();

    // 从商城返回时，检查是否有新建筑需要添加
    for (auto& building : g_allPurchasedBuildings) {
        if (building && !building->getParent()) {
            // 这个建筑还没有添加到场景中，添加它
            this->addChild(building, 10);

            auto visibleSize = Director::getInstance()->getVisibleSize();
            if (building->getPositionX() == 0 && building->getPositionY() == 0) {
                building->setPosition(visibleSize.width / 2, visibleSize.height / 2);
            }

            // 设置回调
            building->setOnUpgradeCallback([=]() {
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
                });

            _allBuildings.pushBack(building);
        }
    }
}

// 新增：资源显示更新函数
void GameScene::updateResourceDisplay() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 更新金币显示
    if (_coinTextLabel) {
        std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
        _coinTextLabel->setString(txt);
    }
    if (myCoin) {
        myCoin->refresh();
    }

    // 更新水显示
    if (_waterTextLabel) {
        std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
        _waterTextLabel->setString(txt);
    }
    if (mywater) {
        mywater->refresh();
    }

    // 更新宝石显示
    if (_gemTextLabel) {
        std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
        _gemTextLabel->setString(txt);
    }
    if (mygem) {
        mygem->refresh();
    }
}