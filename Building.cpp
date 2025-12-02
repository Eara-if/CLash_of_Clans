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
    if (!Sprite::initWithFile(filename,rect)) return false;

    // 初始化数据
    _buildingName = name;
    _baseCost = baseCost;
    _level = 1; // 默认 1 级

    // 初始化点击事件
    this->initTouchListener();

    return true;
}

void Building::setOnUpgradeCallback(std::function<void()> callback)
{
    _onUpgradeCallback = callback;
}

int Building::getNextLevelCost()
{
    // 【核心逻辑】花费随等级增长
    // 比如：1级升2级=500，2级升3级=1000，3级升4级=1500
    return _baseCost * _level;

    // 或者是指数增长： return _baseCost * std::pow(2, _level - 1);
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

        // 2. 【关键修正】这里传入的是 this->_level
        // 因为 Building 对象一直存在，所以每次点击时，_level 都是最新的！
        int cost = this->getNextLevelCost();
        infoLayer->setBuildingData(_buildingName, _level, cost);

        // 3. 设置升级回调
        infoLayer->setUpgradeCallback([=]() {
            this->tryUpgrade(infoLayer);
            });

        // 4. 添加到当前场景 (getParent() 就是 GameScene)
        Director::getInstance()->getRunningScene()->addChild(infoLayer, 999);
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void Building::tryUpgrade(BuildingInfoLayer* layer)
{
    int cost = getNextLevelCost();

    if (coin_count >= cost) {
        // 扣钱
        coin_count -= cost;

        // 升级
        _level++;

        log("Upgrade Success! %s is now Level %d", _buildingName.c_str(), _level);

        // 1. 刷新弹窗上的显示 (如果你希望弹窗不关闭)
        // layer->setBuildingData(_buildingName, _level, getNextLevelCost()); 

        // 2. 或者直接关闭弹窗 (这里我们选择关闭)
        layer->closeLayer();

        // 3. 【通知外部】执行外部给的回调 (比如刷新主界面的金币 Label)
        if (_onUpgradeCallback) {
            _onUpgradeCallback();
        }
    }
    else {
        log("Not enough coins! Need %d", cost);
        // 这里可以加一个文字抖动或者飘字提示 "金币不足"
    }
}