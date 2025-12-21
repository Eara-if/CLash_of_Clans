#include "BuildingUpgradeLimits.h"

USING_NS_CC;

BuildingUpgradeLimits* BuildingUpgradeLimits::_instance = nullptr;

BuildingUpgradeLimits* BuildingUpgradeLimits::getInstance()
{
    if (!_instance) {
        _instance = new BuildingUpgradeLimits();
        _instance->initLimits();
    }
    return _instance;
}

BuildingUpgradeLimits::BuildingUpgradeLimits()
{
}

void BuildingUpgradeLimits::initLimits()
{
    // ==============================================
    // 大本营升级限制表（模仿部落冲突的设计）
    // 格式：建筑类型 -> {大本营等级 -> 最大建筑等级}
    // ==============================================

    // 大本营自身的升级限制（最高10级）
    std::map<int, int> baseLimits;
    for (int th = 1; th <= 10; th++) {
        baseLimits[th] = th; // 大本营等级不能超过自身等级
    }
    _upgradeLimits[BuildingType::BASE] = baseLimits;

    // 兵营升级限制
    std::map<int, int> barracksLimits;
    barracksLimits[1] = 1;   // TH1: 兵营最高1级
    barracksLimits[2] = 2;   // TH2: 兵营最高2级
    barracksLimits[3] = 3;   // TH3: 兵营最高3级
    barracksLimits[4] = 4;   // TH4: 兵营最高4级
    barracksLimits[5] = 5;   // TH5: 兵营最高5级
    barracksLimits[6] = 6;   // TH6: 兵营最高6级
    barracksLimits[7] = 7;   // TH7: 兵营最高7级
    barracksLimits[8] = 8;   // TH8: 兵营最高8级
    barracksLimits[9] = 9;   // TH9: 兵营最高9级
    barracksLimits[10] = 10; // TH10: 兵营最高10级
    _upgradeLimits[BuildingType::BARRACKS] = barracksLimits;

    // 金矿升级限制
    std::map<int, int> mineLimits;
    mineLimits[1] = 2;   // TH1: 金矿最高2级
    mineLimits[2] = 4;   // TH2: 金矿最高4级
    mineLimits[3] = 6;   // TH3: 金矿最高6级
    mineLimits[4] = 8;   // TH4: 金矿最高8级
    mineLimits[5] = 10;  // TH5: 金矿最高10级
    mineLimits[6] = 10;  // TH6: 金矿最高10级
    mineLimits[7] = 11;  // TH7: 金矿最高11级
    mineLimits[8] = 12;  // TH8: 金矿最高12级
    mineLimits[9] = 12;  // TH9: 金矿最高12级
    mineLimits[10] = 12; // TH10: 金矿最高12级
    _upgradeLimits[BuildingType::MINE] = mineLimits;

    // 圣水收集器升级限制
    std::map<int, int> waterLimits;
    waterLimits[1] = 2;   // TH1: 圣水收集器最高2级
    waterLimits[2] = 4;   // TH2: 圣水收集器最高4级
    waterLimits[3] = 6;   // TH3: 圣水收集器最高6级
    waterLimits[4] = 8;   // TH4: 圣水收集器最高8级
    waterLimits[5] = 10;  // TH5: 圣水收集器最高10级
    waterLimits[6] = 10;  // TH6: 圣水收集器最高10级
    waterLimits[7] = 11;  // TH7: 圣水收集器最高11级
    waterLimits[8] = 12;  // TH8: 圣水收集器最高12级
    waterLimits[9] = 12;  // TH9: 圣水收集器最高12级
    waterLimits[10] = 12; // TH10: 圣水收集器最高12级
    _upgradeLimits[BuildingType::WATER] = waterLimits;

    // 防御塔（弓箭塔）升级限制
    std::map<int, int> defenseLimits;
    defenseLimits[1] = 0;   // TH1: 不能建造防御塔
    defenseLimits[2] = 0;   // TH2: 不能建造防御塔
    defenseLimits[3] = 2;   // TH3: 防御塔最高2级
    defenseLimits[4] = 3;   // TH4: 防御塔最高3级
    defenseLimits[5] = 4;   // TH5: 防御塔最高4级
    defenseLimits[6] = 5;   // TH6: 防御塔最高5级
    defenseLimits[7] = 6;   // TH7: 防御塔最高6级
    defenseLimits[8] = 7;   // TH8: 防御塔最高7级
    defenseLimits[9] = 8;   // TH9: 防御塔最高8级
    defenseLimits[10] = 9;  // TH10: 防御塔最高9级
    _upgradeLimits[BuildingType::DEFENSE] = defenseLimits;

    // 加农炮升级限制（假设Cannon使用DEFENSE类型或新增类型）
    std::map<int, int> cannonLimits;
    cannonLimits[1] = 0;   // TH1: 不能建造加农炮
    cannonLimits[2] = 0;   // TH2: 不能建造加农炮
    cannonLimits[3] = 0;   // TH3: 不能建造加农炮
    cannonLimits[4] = 2;   // TH4: 加农炮最高2级
    cannonLimits[5] = 3;   // TH5: 加农炮最高3级
    cannonLimits[6] = 4;   // TH6: 加农炮最高4级
    cannonLimits[7] = 5;   // TH7: 加农炮最高5级
    cannonLimits[8] = 6;   // TH8: 加农炮最高6级
    cannonLimits[9] = 7;   // TH9: 加农炮最高7级
    cannonLimits[10] = 8;  // TH10: 加农炮最高8级
    _upgradeLimits[BuildingType::CANNON] = cannonLimits; // 注意：这里用DEFENSE类型，实际应该区分

    // 城墙升级限制（城墙有特殊处理，这里先占位）
    std::map<int, int> wallLimits;
    for (int th = 1; th <= 10; th++) {
        wallLimits[th] = th * 2; // 每级大本营城墙最高等级增加
    }
    _upgradeLimits[BuildingType::WALL] = wallLimits;

    // 金币存储器升级限制
    std::map<int, int> goldStorageLimits;
    goldStorageLimits[1] = 1;   // TH1: 金币存储器最高1级
    goldStorageLimits[2] = 2;   // TH2: 金币存储器最高2级
    goldStorageLimits[3] = 3;   // TH3: 金币存储器最高3级
    goldStorageLimits[4] = 4;   // TH4: 金币存储器最高4级
    goldStorageLimits[5] = 5;   // TH5: 金币存储器最高5级
    goldStorageLimits[6] = 6;   // TH6: 金币存储器最高6级
    goldStorageLimits[7] = 7;   // TH7: 金币存储器最高7级
    goldStorageLimits[8] = 8;   // TH8: 金币存储器最高8级
    goldStorageLimits[9] = 9;   // TH9: 金币存储器最高9级
    goldStorageLimits[10] = 10; // TH10: 金币存储器最高10级
    _upgradeLimits[BuildingType::GOLD_STORAGE] = goldStorageLimits;

    // 圣水存储器升级限制
    std::map<int, int> waterStorageLimits;
    waterStorageLimits[1] = 1;   // TH1: 圣水存储器最高1级
    waterStorageLimits[2] = 2;   // TH2: 圣水存储器最高2级
    waterStorageLimits[3] = 3;   // TH3: 圣水存储器最高3级
    waterStorageLimits[4] = 4;   // TH4: 圣水存储器最高4级
    waterStorageLimits[5] = 5;   // TH5: 圣水存储器最高5级
    waterStorageLimits[6] = 6;   // TH6: 圣水存储器最高6级
    waterStorageLimits[7] = 7;   // TH7: 圣水存储器最高7级
    waterStorageLimits[8] = 8;   // TH8: 圣水存储器最高8级
    waterStorageLimits[9] = 9;   // TH9: 圣水存储器最高9级
    waterStorageLimits[10] = 10; // TH10: 圣水存储器最高10级
    _upgradeLimits[BuildingType::WATER_STORAGE] = waterStorageLimits;

    // ==============================================
    // 大本营升级解锁内容描述
    // ==============================================
    _townHallUnlockInfo[1] = "Unlock basic buildings";
    _townHallUnlockInfo[2] = "Unlock Water Collector (Lv.2-4), Walls";
    _townHallUnlockInfo[3] = "Unlock Archer Tower, 2nd Barracks, Gold/Water Lv.5-6";
    _townHallUnlockInfo[4] = "Unlock Cannon, Archer Tower Lv.3, Gold/Water Lv.7-8";
    _townHallUnlockInfo[5] = "Unlock Gold/Water Lv.9-10, Archer Tower Lv.4";
    _townHallUnlockInfo[6] = "Unlock 3rd Barracks, Archer Tower Lv.5, Wall Lv.8";
    _townHallUnlockInfo[7] = "Unlock Gold/Water Lv.11, Archer Tower Lv.6";
    _townHallUnlockInfo[8] = "Unlock 4th Barracks, Gold/Water Lv.12, Archer Tower Lv.7";
    _townHallUnlockInfo[9] = "Unlock max levels for all buildings, Archer Tower Lv.8";
    _townHallUnlockInfo[10] = "Max level reached, everything unlocked";
}

int BuildingUpgradeLimits::getMaxLevelForBuilding(BuildingType type, int townHallLevel)
{
    // 确保大本营等级在有效范围内
    if (townHallLevel < 1) townHallLevel = 1;
    if (townHallLevel > 10) townHallLevel = 10;

    // 查找建筑类型的限制
    if (_upgradeLimits.find(type) != _upgradeLimits.end()) {
        auto& limits = _upgradeLimits[type];
        if (limits.find(townHallLevel) != limits.end()) {
            return limits[townHallLevel];
        }
    }

    // 默认返回：大本营等级*2，最高20级
    int defaultMax = townHallLevel * 2;
    return defaultMax > 20 ? 20 : defaultMax;
}

bool BuildingUpgradeLimits::canUpgradeToLevel(BuildingType type, int currentTownHallLevel, int targetLevel)
{
    int maxLevel = getMaxLevelForBuilding(type, currentTownHallLevel);
    return targetLevel <= maxLevel;
}

std::string BuildingUpgradeLimits::getUnlockInfoForNextTownHallLevel(int currentTownHallLevel)
{
    if (currentTownHallLevel >= 10) {
        return "Town Hall is at maximum level!";
    }

    int nextLevel = currentTownHallLevel + 1;
    if (_townHallUnlockInfo.find(nextLevel) != _townHallUnlockInfo.end()) {
        return "Upgrade to Town Hall Level " + std::to_string(nextLevel) + " will: " + _townHallUnlockInfo[nextLevel];
    }

    return "Upgrade to Town Hall Level " + std::to_string(nextLevel) + " will unlock new content";
}

std::map<BuildingType, int> BuildingUpgradeLimits::getMaxLevelsForTownHall(int townHallLevel)
{
    std::map<BuildingType, int> result;

    for (auto& entry : _upgradeLimits) {
        BuildingType type = entry.first;
        int maxLevel = getMaxLevelForBuilding(type, townHallLevel);
        result[type] = maxLevel;
    }

    return result;
}