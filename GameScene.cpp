#include "GameScene.h"
#include "HelloWorldScene.h"
#include "BuildingInfoLayer.h"
USING_NS_CC;
extern int coin_count = 5000;
extern int water_count = 5000;
extern int gem_count = 500;

extern int coin_limit = 5000;
extern int water_limit = 5000;
extern int gem_limit = 5000;

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
        
        // 局部变量即可
        float start_x = 0;
        float start_y = 63;
        float tileWidth = 43;
        float tileHeight = 10;

        float scaleAmount = 6.0f;
        // 逻辑判断
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

        // 【修正 3 - 核心】必须在计算出 start_x 之后，现场创建 Rect
        // 这样 Rect 里的数值才是最新的！
        Rect tileRect = Rect(start_x, start_y, tileWidth, tileHeight);

        auto tile = Sprite::create(filename, tileRect);
        
        // 安全检查
        if(tile) {
            tile->getTexture()->setAliasTexParameters();
            tile->setScale(scaleAmount); // 确保 scaleAmount 在基类初始化了
            
            // 设置位置
            tile->setPosition(origin.x + visibleSize.width - 150, origin.y+visibleSize.height - 50);

            // 添加到父节点
            parentNode->addChild(tile, 0);
        }
    }

    void update() {
    }
};
class water : public resource
{
public:
    water() : resource(5000) {};
    ~water() {};

    void modify() override { // 【建议】加上 override 关键字，编译器会帮你检查
        // 修改逻辑
    }

    // 【修正】现在这就是真正的重写了，因为参数也是 Node*
    void print(Node* parentNode) override {

        // 【修正 2】在函数内部获取屏幕尺寸
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";

        // 局部变量即可
        float start_x = 0;
        float start_y = 20;
        float tileWidth = 43;
        float tileHeight = 10;

        float scaleAmount = 6.0f;
        // 逻辑判断
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

        // 【修正 3 - 核心】必须在计算出 start_x 之后，现场创建 Rect
        // 这样 Rect 里的数值才是最新的！
        Rect tileRect = Rect(start_x, start_y, tileWidth, tileHeight);

        auto tile = Sprite::create(filename, tileRect);

        // 安全检查
        if (tile) {
            tile->getTexture()->setAliasTexParameters();
            tile->setScale(scaleAmount); // 确保 scaleAmount 在基类初始化了

            // 设置位置
            tile->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 120);

            // 添加到父节点
            parentNode->addChild(tile, 0);
        }
    }

    void update() {
    }
};
class Gem : public resource
{
public:
    Gem() : resource(5000) {};
    ~Gem() {};

    void modify() override { 
        gem_count = 500;
    }

    // 【修正】现在这就是真正的重写了，因为参数也是 Node*
    void print(Node* parentNode) override {

        // 【修正 2】在函数内部获取屏幕尺寸
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();

        filename = "06.png";

        // 局部变量即可
        float start_x = 0;
        float start_y = 35;
        float tileWidth = 43;
        float tileHeight = 10;

        float scaleAmount = 6.0f;
        // 逻辑判断
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

        // 【修正 3 - 核心】必须在计算出 start_x 之后，现场创建 Rect
        // 这样 Rect 里的数值才是最新的！
        Rect tileRect = Rect(start_x, start_y, tileWidth, tileHeight);

        auto tile = Sprite::create(filename, tileRect);

        // 安全检查
        if (tile) {
            tile->getTexture()->setAliasTexParameters();
            tile->setScale(scaleAmount); // 确保 scaleAmount 在基类初始化了

            // 设置位置
            tile->setPosition(origin.x + visibleSize.width - 150, origin.y + visibleSize.height - 182);

            // 添加到父节点
            parentNode->addChild(tile, 0);
        }
    }

    void update() {
    }
};
Scene* GameScene::createScene()
{
    return GameScene::create();
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
    goldcoin myCoin;
    myCoin.print(this);

    std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);

    // 【修改点】把创建出来的 Label 存进变量里！
    _coinTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 50, Color4B::WHITE);

    water mywater;
    txt = "Water " + std::to_string(water_count) + "/" + std::to_string(water_limit);
    // 【修改点】保存水 Label
    _waterTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 120, Color4B::WHITE);
    mywater.print(this);

    Gem mygem;
    mygem.modify(); // 这里改了 gem_count
    txt = "Gem " + std::to_string(gem_count) + "/" + std::to_string(gem_limit);
    // 【修改点】保存宝石 Label
    _gemTextLabel = this->showText(txt, origin.x + visibleSize.width - 370, origin.y + visibleSize.height - 182, Color4B::WHITE);
    mygem.print(this);

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

    auto house = Building::create("TilesetHouse.png",houseRect, "My House", 500);
    house->getTexture()->setAliasTexParameters();
    house->setScale(scaleAmount);
    house->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

    

    // ============================================================
    // 【核心变化】设置回调函数
    // 意思就是：告诉房子，“如果你升级成功扣了钱，记得通知我一下，我要刷新文字”
    // ============================================================
    house->setOnUpgradeCallback([=]() {

        // 这里的逻辑只负责刷新界面 UI，不再负责扣钱和升级了
        if (this->_coinTextLabel != nullptr) {
            std::string txt = "Coin " + std::to_string(coin_count) + "/" + std::to_string(coin_limit);
            this->_coinTextLabel->setString(txt);
        }
        });

    // 添加到场景 (注意：Building 本质上也是个 Sprite，所以直接 add)
    this->addChild(house, 0);
}
void GameScene::setresource()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    std::string filename = "TilesetHouse.png";
    float tileWidth = 60;  // 原图截取宽度
    float tileHeight = 45; // 原图截取高度
    float scaleAmount = 4.0f;

    Rect tileRect = Rect(120, 0, tileWidth, tileHeight);

    auto tile = Sprite::create(filename, tileRect);
    tile->getTexture()->setAliasTexParameters();
    tile->setScale(scaleAmount);
    tile->setPosition(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

    this->addChild(tile, 0);
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