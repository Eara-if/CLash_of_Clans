#include "DataManager.h"
#include"Building.h"

extern cocos2d::Vector<Building*> g_allPurchasedBuildings;
extern int coin_count;
extern int water_count;

extern int army_limit;
static DataManager* _instance = nullptr;

DataManager* DataManager::getInstance()
{
    if (!_instance) {
        _instance = new DataManager();
    }
    return _instance;
}

// ============================================================
// 【核心修改】trainTroop 函数
// ============================================================
bool DataManager::trainTroop(std::string type)
{
    // 1. 检查人口是否已满
    if (getTotalPopulation() >= army_limit) {
        cocos2d::log("Army Camp is full!");
        return false;
    }

    // 5. 增加兵种数量
    _myArmy[type]++;

    cocos2d::log("Trained 1 %s (Cost: %d). Total: %d. Water Left: %d",
        type.c_str(), _myArmy[type], water_count);

    return true;
}
bool DataManager::dismissTroop(std::string type)
{

    if (_myArmy.find(type) != _myArmy.end() && _myArmy[type] > 0) {
        _myArmy[type]--;
        //water_count += (cost / 2); // 退兵返还一半圣水
        return true;
    }
    return false;
}
int DataManager::getTroopCount(std::string type)
{

    if (_myArmy.find(type) == _myArmy.end()) return 0;
    return _myArmy[type];
}

int DataManager::getTotalPopulation()
{
    int total = 0;
    for (auto const& item : _myArmy) {
        total += item.second;
    }
    return total;
}

void DataManager::clearArmy()
{
    _myArmy.clear();
}


// 【新增】获取大本营等级
int DataManager::getTownHallLevel()
{
    // 遍历所有建筑，找到大本营
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BASE) {
            return building->getLevel();
        }
    }
    return 1; // 如果没有找到大本营，默认1级
}

// 【新增】检查建筑解锁条件
bool DataManager::isBuildingUnlocked(int buildingId, int& requiredTHLevel)
{
    int townHallLevel = getTownHallLevel();

    // 建筑解锁规则：
    // 大本营等级对应可解锁的建筑
    switch (buildingId) {
        case 1: // 兵营 (第一个)
            requiredTHLevel = 1;
            return townHallLevel >= 1;

        case 2: // 金矿
            requiredTHLevel = 1;
            return townHallLevel >= 1;

        case 3: // 圣水收集器
            requiredTHLevel = 2;
            return townHallLevel >= 2;

        case 4: // 弓箭塔
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 5: // 加农炮
            requiredTHLevel = 4;
            return townHallLevel >= 4;

        case 6: // 城墙
            requiredTHLevel = 2;
            return townHallLevel >= 2;

        case 7: // 第二个兵营
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 8: // 金币存储器
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 9: // 圣水存储器
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 10: // 第三个兵营
            requiredTHLevel = 6;
            return townHallLevel >= 6;

        case 11: // 第四个兵营
            requiredTHLevel = 8;
            return townHallLevel >= 8;

        default:
            requiredTHLevel = 1;
            return true;
    }
}

// 【新增】获取建筑最大数量限制
int DataManager::getBuildingMaxCount(int buildingId)
{
    int townHallLevel = getTownHallLevel();

    // 根据不同大本营等级设置最大数量
    switch (buildingId) {
        case 1: // 兵营
            if (townHallLevel >= 8) return 4;
            if (townHallLevel >= 6) return 3;
            if (townHallLevel >= 3) return 2;
            return 1;

        case 2: // 金矿
            if (townHallLevel >= 9) return 6;
            if (townHallLevel >= 7) return 5;
            if (townHallLevel >= 5) return 4;
            if (townHallLevel >= 3) return 3;
            return 2;

        case 3: // 圣水收集器
            if (townHallLevel >= 9) return 6;
            if (townHallLevel >= 7) return 5;
            if (townHallLevel >= 5) return 4;
            if (townHallLevel >= 3) return 3;
            return 2;

        case 4: // 弓箭塔
            if (townHallLevel >= 8) return 5;
            if (townHallLevel >= 6) return 4;
            if (townHallLevel >= 4) return 3;
            return 2;

        case 5: // 加农炮
            if (townHallLevel >= 8) return 4;
            if (townHallLevel >= 6) return 3;
            return 2;

        case 6: // 城墙
            if (townHallLevel >= 10) return 250;
            if (townHallLevel >= 9) return 225;
            if (townHallLevel >= 8) return 200;
            if (townHallLevel >= 7) return 175;
            if (townHallLevel >= 6) return 150;
            if (townHallLevel >= 5) return 125;
            if (townHallLevel >= 4) return 100;
            if (townHallLevel >= 3) return 75;
            return 50;

        default:
            return 99; // 其他建筑默认无限制
    }
}

// 【新增】直接设置兵种数量（绕过资源检查）
void DataManager::setTroopCount(std::string type, int count)
{
    _myArmy[type] = count;
    CCLOG("=== DataManager: Set %s count to %d ===", type.c_str(), count);
}

// 【新增】获取所有军队数据
std::map<std::string, int> DataManager::getAllArmyData()
{
    return _myArmy;
}
