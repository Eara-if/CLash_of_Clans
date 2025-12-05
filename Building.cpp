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

bool Building::init(const std::string& filename, const Rect& rect, const std::string& name, int baseCost)
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
    //// ============================================================
    //// 【新增】添加底下的绿色草地
    //// ============================================================
    //// 假设你用的是 TilesetField.png，我们截取一块不同颜色的草地
    //// 请根据你的图片实际情况调整 Rect 的坐标 (比如 x=32 可能是深色草地)
    //auto grass = Sprite::create("TilesetField.png", Rect(32, 0, 32, 32));

    //if (grass) {
    //    // 1. 设置位置：放在房子的脚下中心
    //    // contentSize.width / 2 是中心，0 是底部
    //    grass->setPosition(this->getContentSize().width / 2, 0);

    //    // 2. 稍微放大一点，像个院子
    //    grass->setScale(2.0f);

    //    // 3. 【核心】设置 Z-Order 为 -1
    //    // 这样草地就会显示在房子（默认为0）的后面！
    //    this->addChild(grass, -1);
    //}
    //// ============================================================

    _buildingName = name;
    _baseCost = baseCost;
    _level = 1;
    _state = BuildingState::IDLE;
    _timeLeft = 0;

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
            _touchOffset = this->getPosition() - touchPos;

            // 重置拖拽标记
            _isDragging = false;

            // 稍微放大一点，给玩家反馈“我按住它了”
            this->setScale(4.2f); // 假设原来是 4.0f，稍微变大一点

            return true;
        }
        return false;
        };

    // 2. 手指移动
    listener->onTouchMoved = [=](Touch* touch, Event* event) {

        // 计算移动后的新位置
        Vec2 newPos = touch->getLocation() + _touchOffset;

        // 设置位置
        this->setPosition(newPos);

        // 【判断】如果手指移动距离超过 10 像素，就算作“拖拽”
        // 这样可以防止手抖导致的误判
        if (touch->getStartLocation().distance(touch->getLocation()) > 10.0f)
        {
            _isDragging = true;

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
        if (_isDragging)
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
