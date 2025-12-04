#include "Building.h"
#include "BuildingInfoLayer.h" // 一定要引用之前写的弹窗类

USING_NS_CC;

// 引用全局变量 (告诉编译器去别的地方找这个变量)
extern int coin_count;
extern int water_count;
extern int gem_count;

extern int coin_limit;
extern int water_limit;
extern int gem_limit;

Building* Building::create(const std::string& filename, const Rect&rect,const std::string& name, int baseCost)
{
    Building* ret = new (std::nothrow) Building();
    if (ret && ret->init(filename, rect,name, baseCost))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Building::init(const std::string& filename,const Rect&rect, const std::string& name, int baseCost)
{
    // 初始化父类 Sprite
    if (!Sprite::initWithFile(filename,rect)) 
        return false;
    this->setTextureRect(rect);

    // 初始化数据
    _buildingName = name;
    _baseCost = baseCost;
    _level = 1; // 默认 1 级

    // 初始化点击事件
    this->initTouchListener();
    // 【核心】开启 update 调度器，否则 update 函数不会运行
    this->scheduleUpdate();
    return true;
}

void Building::setOnUpgradeCallback(std::function<void()> callback)
{
    UpgradeCallback_coin = callback;
}

int Building::getNextLevelCost()
{
    // 【核心逻辑】花费随等级增长
    // 比如：1级升2级=500，2级升3级=1000，3级升4级=1500
    return _baseCost * _level;

    // 或者是指数增长： return _baseCost * std::pow(2, _level - 1);
}
// 每帧自动调用
void Building::update(float dt)
{
    if (_state == BuildingState::UPGRADING)
    {
        _timeLeft -= dt; // 扣除时间

        if (_timeLeft <= 0) {
            this->finishUpgrade(); // 时间到了，完成！
        }
    }
}
// 计算升级耗时 (比如：等级 * 5秒)
int Building::getUpgradeTime() {
    return _level * 5; // 1级升2级要5秒，测试用
}

// 计算加速需要的宝石 (比如：1秒 = 1宝石，或者固定 5 宝石)
int Building::getSpeedUpCost() {
    return std::ceil(_timeLeft/60); // 简单粗暴：剩下几秒就几个宝石
}

// 开始升级 (只扣钱，不加等级)
void Building::startUpgrade()
{
    int cost = getNextLevelCost();
    if (coin_count >= cost) {
        coin_count -= cost;
        coin_limit += 1500;
        water_limit += 1500;
        // 进入升级状态
        _state = BuildingState::UPGRADING;
        _totalTime = getUpgradeTime();
        _timeLeft = _totalTime;

        log("Upgrade Started! Time needed: %f", _totalTime);

        // 刷新一下金币显示 (通过回调)
        if (UpgradeCallback_coin)
            UpgradeCallback_coin();
    }
}

// 宝石加速
void Building::speedUp()
{
    int cost = getSpeedUpCost();
    if (gem_count >= cost) {
        gem_count -= cost;
        finishUpgrade(); // 瞬间完成
    }
    else {
        log("Not enough gems!");
    }
}

// 真正的完成升级
void Building::finishUpgrade()
{
    _state = BuildingState::IDLE;
    _timeLeft = 0;
    _level++;

    log("Upgrade Finished! Level is now %d", _level);

    // 通知界面刷新 (如果有打开的弹窗，弹窗需要自己处理刷新，或者关闭重开)
    if (UpgradeCallback_coin) UpgradeCallback_coin();
}
void Building::initTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [=](Touch* touch, Event* event) {
        Vec2 pos = convertToNodeSpace(touch->getLocation()); // 转成相对坐标
        Rect rect = Rect(0, 0, getContentSize().width, getContentSize().height);
        return rect.containsPoint(pos); // 判断是否点中自己
    };

    listener->onTouchEnded = [=](Touch* touch, Event* event) {

        // 1. 创建弹窗
        auto infoLayer = BuildingInfoLayer::create();

        
        infoLayer->setBuilding(this);

        // 4. 添加到当前场景 (getParent() 就是 GameScene)
        Director::getInstance()->getRunningScene()->addChild(infoLayer, 999);
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
