#ifndef __BUILDING_UPGRADE_LIMITS_H__
#define __BUILDING_UPGRADE_LIMITS_H__

#include "cocos2d.h"
#include "Building.h"
#include <map>
#include <vector>

class BuildingUpgradeLimits
{
public:
    // 单例模式
    static BuildingUpgradeLimits* getInstance();

    // 获取建筑在当前大本营等级下的最大可升级等级
    int getMaxLevelForBuilding(BuildingType type, int townHallLevel);

    // 检查建筑是否可以升级到指定等级
    bool canUpgradeToLevel(BuildingType type, int currentTownHallLevel, int targetLevel);

    // 获取大本营下一级解锁的内容
    std::string getUnlockInfoForNextTownHallLevel(int currentTownHallLevel);

    // 获取所有建筑的最大等级表（用于显示）
    std::map<BuildingType, int> getMaxLevelsForTownHall(int townHallLevel);

private:
    BuildingUpgradeLimits();
    void initLimits();

    static BuildingUpgradeLimits* _instance;

    // 存储建筑升级限制数据
    // 结构：建筑类型 -> 大本营等级 -> 最大建筑等级
    std::map<BuildingType, std::map<int, int>> _upgradeLimits;

    // 大本营升级解锁内容描述
    std::map<int, std::string> _townHallUnlockInfo;
};

#endif // __BUILDING_UPGRADE_LIMITS_H__