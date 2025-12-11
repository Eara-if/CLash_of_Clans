#include "Building.h"
#include "BuildingInfoLayer.h" // 一定要引用之前写的弹窗类
#include"GameScene.h"
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

    // 初始化生产相关变量
    productionTimeLeft = 0;
    productionAmount = 50; // 默认生产50资源
    isReadyToCollect = false;
    readyIndicator = nullptr;

    // 如果是金矿或圣水收集器，开始生产
    if (type == BuildingType::MINE || type == BuildingType::WATER) {
        startProduction();
    }

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

    // 生产倒计时
    if (type == BuildingType::MINE || type == BuildingType::WATER) {
        if (state == BuildingState::PRODUCING) {
            productionTimeLeft -= dt;

            if (productionTimeLeft <= 0) {
                finishProduction();
            }
        }
    }

}
// 计算升级耗时 (比如：等级 * 5秒)
int Building::getUpgradeTime() {
    return a_level * 5; // 1级升2级要5秒，测试用
}

// 计算加速需要的宝石 (比如：1秒 = 1宝石，或者固定 5 宝石)
int Building::getSpeedUpCost() {
    return std::ceil(timeLeft / 60); // 简单粗暴：剩下几秒就几个宝石
}


float Building::getTimeLeft()
{
    // 返回剩下的时间
    // 如果小于0，就返回0，防止显示负数
    return (timeLeft > 0) ? timeLeft : 0;
}

// 开始生产
void Building::startProduction()
{
    state = BuildingState::PRODUCING;
    productionTimeLeft = 5.0f; // 5秒倒计时
    isReadyToCollect = false;

    // 移除可收集提示
    if (readyIndicator) {
        readyIndicator->removeFromParent();
        readyIndicator = nullptr;
    }

    log("%s started production, time left: %f", buildingName.c_str(), productionTimeLeft);
}

// 生产完成
void Building::finishProduction()
{
    state = BuildingState::READY;
    isReadyToCollect = true;

    // 添加可收集提示
    if (!readyIndicator) {
        readyIndicator = Sprite::create("ui/ready_indicator.png"); // 需要创建一个红色"!"图片
        if (!readyIndicator) {
            // 如果图片不存在，创建一个简单的红色感叹号
            readyIndicator = Sprite::create();
            auto label = Label::createWithTTF("!", "fonts/Marker Felt.ttf", 72);
            label->setColor(Color3B::RED);
            label->setPosition(Vec2(15, 50));
            readyIndicator->addChild(label);
            readyIndicator->setContentSize(Size(30, 30));
        }
        readyIndicator->setPosition(Vec2(this->getContentSize().width / 2, this->getContentSize().height + 20));
        this->addChild(readyIndicator, 100);
    }

    log("%s production ready! Collect %d resources.", buildingName.c_str(), productionAmount);
}

// 收集资源
void Building::collectResources()
{
    if (state == BuildingState::READY && isReadyToCollect) {
        if (buildingName == "Gold Mine") {
            coin_count += productionAmount;
            // 确保不超过上限
            if (coin_count > coin_limit) coin_count = coin_limit;
            log("Collected %d gold from Gold Mine. Total: %d", productionAmount, coin_count);
        }
        else if (buildingName == "Water Collector") {
            water_count += productionAmount;
            // 确保不超过上限
            if (water_count > water_limit) water_count = water_limit;
            log("Collected %d water from Water Collector. Total: %d", productionAmount, water_count);
        }
        auto gamescene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());
        gamescene->updateResourceDisplay();
        // 重新开始生产
        startProduction();
    }
}

// 获取生产剩余时间
float Building::getProductionTimeLeft() {
    return productionTimeLeft;
}

// 获取可收集的资源量
int Building::getProducedAmount() {
    return productionAmount;
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
// Building.cpp

void Building::initTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [=](Touch* touch, Event* event) {
        Vec2 touchPos = touch->getLocation();

        // 使用节点空间转换，判断是否点中
        Vec2 nodePos = this->convertToNodeSpace(touchPos);
        Rect rect = Rect(0, 0, getContentSize().width, getContentSize().height);

        if (rect.containsPoint(nodePos))
        {
            // 1. 【核心】记录起飞前的位置 (地图坐标系)
            originalPos = this->getPosition();

            touchOffset = this->getPosition() - touchPos;
            isDragging = false;

            // 视觉反馈
            this->setScale(0.55f);
            this->setLocalZOrder(99999); // 提起来，防止被遮挡
            return true;
        }
        return false;
        };

    listener->onTouchMoved = [=](Touch* touch, Event* event) {
        if (touch->getStartLocation().distance(touch->getLocation()) > 10.0f) {
            isDragging = true;
        }

        // 移动逻辑
        Vec2 worldNewPos = touch->getLocation() + touchOffset;
        Vec2 nodePos = this->getParent()->convertToNodeSpace(worldNewPos);
        this->setPosition(nodePos);
        };

    listener->onTouchEnded = [=](Touch* touch, Event* event) {
        this->setScale(0.5f); // 恢复大小

        if (isDragging) {
            auto gameScene = dynamic_cast<GameScene*>(Director::getInstance()->getRunningScene());

            if (gameScene) {
                // 1. 获取当前建筑的精准世界包围盒
                Rect nodeRect = this->getBoundingBox();
                Vec2 worldOrigin = this->getParent()->convertToWorldSpace(nodeRect.origin);
                Rect myWorldRect = Rect(worldOrigin.x, worldOrigin.y, nodeRect.size.width, nodeRect.size.height);

                // 2. 稍微缩小一点判定框 (手感优化)
                myWorldRect.origin.x += 10;
                myWorldRect.origin.y += 10;
                myWorldRect.size.width -= 20;
                myWorldRect.size.height -= 20;

                // 3. 检测碰撞
                if (gameScene->checkCollision(myWorldRect, this)) {
                    log("COLLISION! Back to: %f, %f", originalPos.x, originalPos.y);

                    // 【关键修复】回弹动画 + 强制设置位置
                    // 以前可能只有动画，没有 setPosition，导致动画完了位置不对
                    auto moveBack = MoveTo::create(0.1f, originalPos);
                    auto tintRed = TintTo::create(0.1f, Color3B::RED);
                    auto tintBack = TintTo::create(0.1f, Color3B::WHITE);

                    // 动作序列：移回 -> 变红 -> 变白 -> 恢复层级
                    auto seq = Sequence::create(
                        Spawn::create(moveBack, tintRed, NULL),
                        tintBack,
                        CallFunc::create([=]() {
                            // 动作结束后，强制确认位置和层级
                            this->setPosition(originalPos);
                            this->setLocalZOrder(10000 - (int)originalPos.y);
                            }),
                        NULL
                    );
                    this->runAction(seq);
                }
                else {
                    log("Placed OK.");
                    // 放置成功，更新记录点
                    originalPos = this->getPosition();
                    this->setLocalZOrder(10000 - (int)this->getPositionY());
                }
            }
            isDragging = false;
        }
        else {
            // 点击事件
            this->setLocalZOrder(10000 - (int)this->getPositionY()); // 恢复层级
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