#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
#include "BattleScene.h"
#include "ShopScene.h" 
USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;

int my_house_level = 1;
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
        float start_y = 20;
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
        float start_y = 35;
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
Scene* GameScene::createScene()
{
    return GameScene::create();
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

    // 2. 切换场景
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
}
bool GameScene::init()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    if (!Scene::init())
    {
        return false;
    }

    std::string filename = "TilesetField.png";

    float tileWidth = 22;  // 原图截取宽度
    float tileHeight = 22; // 原图截取高度
    // 使用刚才测试成功的坐标 (假设是方案A)
    Rect tileRect = Rect(6, 105, tileWidth, tileHeight);

    // 【新增】放大倍数 (比如 2.0 就是放大两倍，3.0 就是三倍)
    float scaleAmount = 4.0f;

    // 【关键】计算新的步长 (实际铺设时的间距)
    // 如果图片放大了2倍，那么我们铺下一块砖的时候，就要多跨越2倍的距离
    float stepX = tileWidth * scaleAmount;
    float stepY = tileHeight * scaleAmount;
    // =============================================

    // --- 双重循环开始 ---
    // 注意：这里的 x += stepX 和 y += stepY 是修改的关键！
    for (float x = origin.x; x < origin.x + visibleSize.width; x += stepX)
    {
        for (float y = origin.y; y < origin.y + visibleSize.height; y += stepY)
        {
            auto tile = Sprite::create(filename, tileRect);

            // 1. 设置锚点为左下角 (保持不变)
            tile->setAnchorPoint(Vec2::ZERO);

            // 2. 【新增】设置放大倍数
            tile->setScale(scaleAmount);

            // 3. 设置位置
            tile->setPosition(x, y);

            // 4. 保持像素清晰 (对于放大操作特别重要！)
            tile->getTexture()->setAliasTexParameters();

            this->addChild(tile, -1);
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

    this->setdby();

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
    this->addShopButton(); //【新增】调用商场按钮布局函数
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
// 引入我们新写的封装好的类
#include "Building.h" 

void GameScene::setdby()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    std::string filename = "TilesetHouse.png";
    float tileWidth = 60;  // 原图截取宽度
    float tileHeight = 45; // 原图截取高度
    float scaleAmount = 4.0f;
    Rect houseRect = Rect(120, 0, tileWidth, tileHeight);

    auto house = Building::create("TilesetHouse.png", houseRect, "My House", 500);
    house->setLevel(my_house_level);
    house->getTexture()->setAliasTexParameters();
    house->setScale(scaleAmount);
    house->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);



    // ============================================================
    // 【核心变化】设置回调函数
    // 意思就是：告诉房子，“如果你升级成功扣了钱，记得通知我一下，我要刷新文字”
    // ============================================================
    house->setOnUpgradeCallback([=]() {
        my_house_level = house->getLevel();
        // 这里的逻辑只负责刷新界面 UI，不再负责扣钱和升级了
        if (this->_coinTextLabel != nullptr) {
            std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
            this->_coinTextLabel->setString(txt);
        }
        if (this->_gemTextLabel != nullptr) {
            std::string txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
            this->_gemTextLabel->setString(txt);
        }
        if (this->myCoin != nullptr) {
            // 告诉金币对象：你的数量变了，快换个样子！
            this->myCoin->refresh();
        }
        if (this->_waterTextLabel != nullptr) {
            std::string txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
            this->_waterTextLabel->setString(txt);
        }
        if (this->mywater != nullptr) {
            // 水上限变了，图标可能需要从“满堆”变成“半堆”，所以要刷新
            this->mywater->refresh();
        }
        if (this->mygem != nullptr) {

            this->mygem->refresh();
        }

        });

    // 添加到场景 (注意：Building 本质上也是个 Sprite，所以直接 add)
    this->addChild(house, 0);
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

    auto shopItem = MenuItemSprite::create(
        shopNormal,
        shopSelected,
        [](Ref* pSender) {
            // 跳转到商城场景
            auto scene = ShopScene::createScene();
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene));
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