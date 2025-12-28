#ifndef BUILDING_UPGRADE_LIMITS_H_
#define BUILDING_UPGRADE_LIMITS_H_

#include "cocos2d.h"      // 包含cocos2d游戏引擎头文件
#include "Building.h"     // 包含建筑类型定义头文件
#include <map>           // 包含标准映射容器头文件
#include <vector>        // 包含标准向量容器头文件
#include <string>        // 包含标准字符串头文件

// 建筑升级限制类，负责管理不同大本营等级下各建筑的最大可升级等级
class BuildingUpgradeLimits
{
public:
    // 获取单例实例的静态方法（单例模式）
    static BuildingUpgradeLimits* getInstance();

    // 获取特定建筑类型在大本营等级下的最大可升级等级
    // 参数：type - 建筑类型，townHallLevel - 大本营等级
    // 返回值：最大可升级等级
    int getMaxLevelForBuilding(BuildingType type, int townHallLevel);

    // 检查建筑是否可以升级到指定等级
    // 参数：type - 建筑类型，currentTownHallLevel - 当前大本营等级，targetLevel - 目标等级
    // 返回值：如果可以升级返回true，否则返回false
    bool canUpgradeToLevel(BuildingType type, int currentTownHallLevel, int targetLevel)
    {
        return targetLevel <= getMaxLevelForBuilding(type, currentTownHallLevel);
    }

    // 获取下一个大本营等级的解锁信息
    // 参数：currentTownHallLevel - 当前大本营等级
    // 返回值：下一个大本营等级的解锁信息字符串
    std::string getUnlockInfoForNextTownHallLevel(int currentTownHallLevel);

    // 获取指定大本营等级下所有建筑类型的最大等级
    // 参数：townHallLevel - 大本营等级
    // 返回值：建筑类型到最大等级的映射
    std::map<BuildingType, int> getMaxLevelsForTownHall(int townHallLevel);

private:
    // 私有构造函数（单例模式）
    BuildingUpgradeLimits() = default;

    // 初始化所有限制数据
    void initLimits();

    // 各个建筑类型的初始化函数
    void initBaseLimits();           // 初始化大本营升级限制
    void initBarracksLimits();       // 初始化兵营升级限制
    void initMineLimits();           // 初始化金矿升级限制
    void initWaterLimits();          // 初始化水泵升级限制
    void initDefenseLimits();        // 初始化防御建筑总数限制
    void initCannonLimits();         // 初始化加农炮升级限制
    void initGoldStorageLimits();    // 初始化金库升级限制
    void initWaterStorageLimits();   // 初始化水库升级限制
    void initTownHallUnlockInfo();   // 初始化大本营解锁信息
    //城墙不升级

    static BuildingUpgradeLimits* instance_;  // 单例实例指针

    // 升级限制映射（嵌套映射）
    // 外层键：建筑类型，内层键：大本营等级，内层值：最大建筑等级
    std::map<BuildingType, std::map<int, int>> upgradeLimits_;

    // 大本营解锁信息映射
    // 键：大本营等级，值：该等级解锁的信息描述
    std::map<int, std::string> townHallUnlockInfo_;
};

#endif