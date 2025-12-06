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


Building* Building::create(const std::string& filename, const Rect& rect, const std::string& name, int baseCost, BuildingType type)
{
    Building* ret = new (std::nothrow) Building();
    if (ret && ret->init(filename, rect, name, baseCost, type)) // 传入 type
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Building::init(const std::string& filename, const Rect& rect, const std::string& name, int baseCost, BuildingType type1)
{
    
    if (rect.equals(Rect::ZERO))
    {
        // initWithFile 只传文件名，就是加载整张图
        if (!Sprite::initWithFile(filename)) return false;
    }
    else
    {
        // 否则，按照 rect 进行裁剪
        if (!Sprite::initWithFile(filename, rect)) return false;
        // 注意：有些版本的 Cocos 在带 rect 初始化时不需要再 setTextureRect，
        // 但为了保险起见，保持现状或根据实际显示调整
    }

    type = type1; // 【保存类型】
    a_level = 1;
    buildingName = name;
    baseCost1 = baseCost;
    state = BuildingState::IDLE;
    timeLeft = 0;

    this->initTouchListener();
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
    return baseCost1 * a_level;

    // 或者是指数增长： return _baseCost * std::pow(2, _level - 1);
}
// 每帧自动调用
void Building::update(float dt)
{
    if (state == BuildingState::UPGRADING)
    {
        timeLeft -= dt; // 扣除时间

        if (timeLeft <= 0) {
            this->finishUpgrade(); // 时间到了，完成！
        }
    }
}
// 计算升级耗时 (比如：等级 * 5秒)
int Building::getUpgradeTime() {
    return a_level * 5; // 1级升2级要5秒，测试用
}

// 计算加速需要的宝石 (比如：1秒 = 1宝石，或者固定 5 宝石)
int Building::getSpeedUpCost() {
    return std::ceil(timeLeft/60); // 简单粗暴：剩下几秒就几个宝石
}
// Building.cpp

float Building::getTimeLeft()
{
    // 返回剩下的时间
    // 如果小于0，就返回0，防止显示负数
    return (timeLeft > 0) ? timeLeft : 0;
}
// 开始升级 (只扣钱，不加等级)
// 引入全局变量 (你需要根据你的实际变量名修改这里)
extern int coin_count;
extern int water_count;

void Building::startUpgrade()
{
    // 1. 基本检查：如果已经在升级，或者满了，就不能再升
    if (state != BuildingState::IDLE) return;

    // 2. 计算本次升级需要的花费
    // (假设你有一个 getNextLevelCost 函数计算花费)
    int cost = this->getNextLevelCost();

    // 3. 【核心修复】根据建筑类型扣除对应的资源
    // 假设：大本营和防御塔花金币，兵营花水
    bool isEnough = false;

    if (type == BuildingType::BARRACKS) {
        // 兵营：扣水
        if (water_count >= cost) {
            water_count -= cost; // 扣费
            isEnough = true;
            log("Spent %d Water for upgrade.", cost);
        }
        else {
            log("Not enough Water!");
        }
    }
    else {
        // 其他(大本营/金矿等)：扣金币
        if (coin_count >= cost) {
            coin_count -= cost; // 扣费
            isEnough = true;
            log("Spent %d Coin for upgrade.", cost);
        }
        else {
            log("Not enough Coin!");
        }
    }

    // 4. 只有钱够了，才开始升级倒计时
    if (isEnough) {
        state = BuildingState::UPGRADING;

        // 设置时间 (比如 基础时间 * 等级)
        timeLeft = 5.0f * a_level;

        log("Upgrade started... Time left: %f", timeLeft);
    }
    else {
        // 钱不够，这里可以做一个弹窗提示或者播放一个错误音效
        log("Cannot upgrade: Insufficient resources.");
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

void Building::initTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);   //吞噬，底下的东西不会再被点击

    // 1. 手指按下
    listener->onTouchBegan = [=](Touch* touch, Event* event) {
        Vec2 touchPos = touch->getLocation();

        // 判断是否点到了房子
        // 把触摸点转换成相对于房子的坐标
        Vec2 nodePos = this->convertToNodeSpace(touchPos);
        Rect rect = Rect(0, 0, getContentSize().width, getContentSize().height);

        if (rect.containsPoint(nodePos))
        {
            // 【核心逻辑】记录偏移量
            // 这样拖拽时，房子不会瞬间瞬移到手指中心，而是保持相对位置
            touchOffset = this->getPosition() - touchPos;

            // 重置拖拽标记
            isDragging = false;

            // 稍微放大一点，给玩家反馈“我按住它了”
            this->setScale(4.2f); // 假设原来是 4.0f，稍微变大一点

            return true;
        }
        return false;
        };

    // 2. 手指移动
    listener->onTouchMoved = [=](Touch* touch, Event* event) {

        // 计算移动后的新位置
        Vec2 newPos = touch->getLocation() + touchOffset;

        // 设置位置
        this->setPosition(newPos);

        // 【判断】如果手指移动距离超过 10 像素，就算作“拖拽”
        // 这样可以防止手抖导致的误判
        if (touch->getStartLocation().distance(touch->getLocation()) > 10.0f)
        {
            isDragging = true;

            // 【可选】拖拽时动态调整层级 (Z-Order)
            // 在 2D 游戏中，通常 Y 越小（越靠下），Z 应该越大（遮挡后面的）
            // this->getParent()->reorderChild(this, 10000 - newPos.y);
        }
        };

    // 3. 手指松开
    listener->onTouchEnded = [=](Touch* touch, Event* event) {

        // 恢复原来的大小 (假设原来是 4.0f)
        this->setScale(4.0f);

        // ============================================================
        // 【核心区分】是点击还是拖拽？
        // ============================================================
        if (isDragging)
        {
            // A. 如果是拖拽：什么都不做，就停在这里
            log("Moved building to: %f, %f", this->getPositionX(), this->getPositionY());

            // 【进阶提示】你可以在这里加代码，把新坐标保存到数据库或全局变量
        }
        else
        {
            // B. 如果不是拖拽 (位移很小)：说明是点击，打开升级弹窗
            auto infoLayer = BuildingInfoLayer::create();
            infoLayer->setBuilding(this);
            Director::getInstance()->getRunningScene()->addChild(infoLayer, 999);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
void Building::finishUpgrade()
{
    // 1. 只有正在升级才能结算
    if (state != BuildingState::UPGRADING) return;

    // 2. 状态变回空闲
    state = BuildingState::IDLE;
    timeLeft = 0;

    // 3. 等级 +1
    a_level++;

    // 4. 【核心】在这里调用回调！
    // 这时候才会执行 GameScene 里写的 coin_limit += 1500 代码
    if (UpgradeCallback_coin) {
        UpgradeCallback_coin();
    }
    
    log("Upgrade finished! Level is now %d", a_level);
}