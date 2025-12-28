//实现建筑升级受大本营等级限制
#include "BuildingUpgradeLimits.h"  // 包含对应的头文件

USING_NS_CC;  // 使用cocos2d命名空间

// 初始化静态单例实例指针为nullptr
BuildingUpgradeLimits* BuildingUpgradeLimits::instance_ = nullptr;

// 获取单例实例的静态方法实现
BuildingUpgradeLimits* BuildingUpgradeLimits::getInstance()
{
    // 如果实例不存在，则创建新实例并初始化
    if (!instance_)
    {
        instance_ = new BuildingUpgradeLimits();
        instance_->initLimits();
    }
    // 返回单例实例
    return instance_;
}

// 初始化所有限制数据
void BuildingUpgradeLimits::initLimits()
{
    // 调用各个建筑的初始化函数
    initBaseLimits();           // 初始化大本营升级限制
    initBarracksLimits();       // 初始化兵营升级限制
    initMineLimits();           // 初始化金矿升级限制
    initWaterLimits();          // 初始化水泵升级限制
    initDefenseLimits();        // 初始化防御建筑总数限制
    initCannonLimits();         // 初始化加农炮升级限制
    initGoldStorageLimits();    // 初始化金库升级限制
    initWaterStorageLimits();   // 初始化水库升级限制
    //城墙不升级

    // 初始化大本营解锁信息
    initTownHallUnlockInfo();
}

// 初始化大本营升级限制
void BuildingUpgradeLimits::initBaseLimits()
{
    // 创建大本营等级到最大等级的映射
    std::map<int, int> baseLimits;

    // 大本营等级从1到10，最大等级等于大本营等级
    for (int th = 1; th <= 10; th++)
    {
        baseLimits[th] = th;
    }

    // 将大本营限制映射存入总限制映射中
    upgradeLimits_[BuildingType::BASE] = baseLimits;
}

// 初始化兵营升级限制
void BuildingUpgradeLimits::initBarracksLimits()
{
    // 创建兵营等级到最大等级的映射
    std::map<int, int> barracksLimits;

    // 兵营等级从1到10，最大等级等于大本营等级
    for (int th = 1; th <= 10; th++)
    {
        barracksLimits[th] = th;
    }

    // 将兵营限制映射存入总限制映射中
    upgradeLimits_[BuildingType::BARRACKS] = barracksLimits;
}

// 初始化金矿升级限制
void BuildingUpgradeLimits::initMineLimits()
{
    // 创建金矿等级到最大等级的映射
    std::map<int, int> mineLimits;

    // 预定义的大本营1-10级对应的金矿最大等级数组
    int mineValues[] = { 2, 4, 6, 8, 10, 10, 11, 12, 12, 12 };

    // 根据预定义数组设置各等级限制
    for (int th = 1; th <= 10; th++)
    {
        mineLimits[th] = mineValues[th - 1];  // 数组索引从0开始
    }

    // 将金矿限制映射存入总限制映射中
    upgradeLimits_[BuildingType::MINE] = mineLimits;
}

// 初始化水泵升级限制
void BuildingUpgradeLimits::initWaterLimits()
{
    // 创建水泵等级到最大等级的映射
    std::map<int, int> waterLimits;

    // 预定义的大本营1-10级对应的水泵最大等级数组
    int waterValues[] = { 2, 4, 6, 8, 10, 10, 11, 12, 12, 12 };

    // 根据预定义数组设置各等级限制
    for (int th = 1; th <= 10; th++)
    {
        waterLimits[th] = waterValues[th - 1];  // 数组索引从0开始
    }

    // 将水泵限制映射存入总限制映射中
    upgradeLimits_[BuildingType::WATER] = waterLimits;
}

// 初始化防御塔总数限制
void BuildingUpgradeLimits::initDefenseLimits()
{
    // 创建防御塔总数限制映射
    std::map<int, int> defenseLimits;

    // 预定义的大本营1-10级对应的防御塔总数数组
    int defenseValues[] = { 0, 0, 2, 3, 4, 5, 6, 7, 8, 9 };

    // 根据预定义数组设置各等级限制
    for (int th = 1; th <= 10; th++)
    {
        defenseLimits[th] = defenseValues[th - 1];  // 数组索引从0开始
    }

    // 将防御塔总数限制映射存入总限制映射中
    upgradeLimits_[BuildingType::TOWER] = defenseLimits;
}

// 初始化加农炮升级限制
void BuildingUpgradeLimits::initCannonLimits()
{
    // 创建加农炮等级到最大等级的映射
    std::map<int, int> cannonLimits;

    // 预定义的大本营1-10级对应的加农炮最大等级数组
    int cannonValues[] = { 0, 0, 0, 2, 3, 4, 5, 6, 7, 8 };

    // 根据预定义数组设置各等级限制
    for (int th = 1; th <= 10; th++)
    {
        cannonLimits[th] = cannonValues[th - 1];  // 数组索引从0开始
    }

    // 将加农炮限制映射存入总限制映射中
    upgradeLimits_[BuildingType::CANNON] = cannonLimits;
}


// 初始化金库升级限制
void BuildingUpgradeLimits::initGoldStorageLimits()
{
    // 创建金库等级到最大等级的映射
    std::map<int, int> goldStorageLimits;

    // 金库最大等级等于大本营等级
    for (int th = 1; th <= 10; th++)
    {
        goldStorageLimits[th] = th;
    }

    // 将金库限制映射存入总限制映射中
    upgradeLimits_[BuildingType::GOLD_STORAGE] = goldStorageLimits;
}

// 初始化水库升级限制
void BuildingUpgradeLimits::initWaterStorageLimits()
{
    // 创建水库等级到最大等级的映射
    std::map<int, int> waterStorageLimits;

    // 水库最大等级等于大本营等级
    for (int th = 1; th <= 10; th++)
    {
        waterStorageLimits[th] = th;
    }

    // 将水库限制映射存入总限制映射中
    upgradeLimits_[BuildingType::WATER_STORAGE] = waterStorageLimits;
}

// 初始化大本营解锁信息
void BuildingUpgradeLimits::initTownHallUnlockInfo()
{
    // 为每个大本营等级设置解锁信息
    townHallUnlockInfo_[1] = "Unlock basic buildings";
    townHallUnlockInfo_[2] = "Unlock Water Collector (Lv.2-4), Walls";
    townHallUnlockInfo_[3] = "Unlock Archer Tower, 2nd Barracks, Gold/Water Lv.5-6";
    townHallUnlockInfo_[4] = "Unlock Cannon, Archer Tower Lv.3, Gold/Water Lv.7-8";
    townHallUnlockInfo_[5] = "Unlock Gold/Water Lv.9-10, Archer Tower Lv.4";
    townHallUnlockInfo_[6] = "Unlock 3rd Barracks, Archer Tower Lv.5, Wall Lv.8";
    townHallUnlockInfo_[7] = "Unlock Gold/Water Lv.11, Archer Tower Lv.6";
    townHallUnlockInfo_[8] = "Unlock 4th Barracks, Gold/Water Lv.12, Archer Tower Lv.7";
    townHallUnlockInfo_[9] = "Unlock max levels for all buildings, Archer Tower Lv.8";
    townHallUnlockInfo_[10] = "Max level reached, everything unlocked";
}

// 获取特定建筑类型在大本营等级下的最大可升级等级
int BuildingUpgradeLimits::getMaxLevelForBuilding(BuildingType type, int townHallLevel)
{
    // 确保大本营等级在有效范围内（1-10）
    if (townHallLevel < 1)
    {
        townHallLevel = 1;
    }

    if (townHallLevel > 10)
    {
        townHallLevel = 10;
    }

    // 查找特定建筑类型的限制映射
    if (upgradeLimits_.find(type) != upgradeLimits_.end())
    {
        // 获取该建筑类型的限制映射
        auto& limits = upgradeLimits_[type];

        // 查找对应大本营等级的最大等级
        if (limits.find(townHallLevel) != limits.end())
        {
            return limits[townHallLevel];
        }
    }

    // 默认最大等级为大本营等级*2，但不超过20
    int defaultMax = townHallLevel * 2;
    return defaultMax > 20 ? 20 : defaultMax;
}

// 获取下一个大本营等级的解锁信息
std::string BuildingUpgradeLimits::getUnlockInfoForNextTownHallLevel(int currentTownHallLevel)
{
    // 如果已经是最高等级，返回相应信息
    if (currentTownHallLevel >= 10)
    {
        return "Town Hall is at maximum level!";
    }

    // 计算下一个等级
    int nextLevel = currentTownHallLevel + 1;

    // 查找下一个等级的解锁信息
    if (townHallUnlockInfo_.find(nextLevel) != townHallUnlockInfo_.end())
    {
        return "Upgrade to Town Hall Level " + std::to_string(nextLevel) + " will: " + townHallUnlockInfo_[nextLevel];
    }

    // 如果没有找到具体的解锁信息，返回通用信息
    return "Upgrade to Town Hall Level " + std::to_string(nextLevel) + " will unlock new content";
}

// 获取指定大本营等级下所有建筑类型的最大等级
std::map<BuildingType, int> BuildingUpgradeLimits::getMaxLevelsForTownHall(int townHallLevel)
{
    // 创建结果映射
    std::map<BuildingType, int> result;

    // 遍历所有建筑类型
    for (auto& entry : upgradeLimits_)
    {
        BuildingType type = entry.first;

        // 获取该建筑类型在当前大本营等级下的最大等级
        int maxLevel = getMaxLevelForBuilding(type, townHallLevel);

        // 将结果存入映射
        result[type] = maxLevel;
    }

    // 返回结果映射
    return result;
}