#include "DataManager.h"
#include"Building.h"

extern cocos2d::Vector<Building*> g_allPurchasedBuildings; //获取当前已经购买的建筑
extern int coin_count;
extern int water_count;

extern int army_limit;
static DataManager* instance = nullptr;
//创建唯一的一个数据管理器
DataManager* DataManager::getInstance()
{
    if (!instance) {
        instance = new DataManager();
    }
    return instance;
}
//训练士兵（增加士兵）
bool DataManager::trainTroop(std::string type)
{               
    //如果当前的总人数超过上限了，返回false
    if (getTotalPopulation() >= army_limit) {
        cocos2d::log("Army Camp is full!");
        return false;
    }
    //没满，让这个兵种数量加1
    _myArmy[type]++;

    cocos2d::log("Trained 1 %s (Cost: %d). Total: %d. Water Left: %d",
        type.c_str(), _myArmy[type], water_count);

    return true;
}
//减少士兵数量
bool DataManager::dismissTroop(std::string type)
{
    //如果这个兵种存在且不为0，数量减1
    if (_myArmy.find(type) != _myArmy.end() && _myArmy[type] > 0) {
        _myArmy[type]--;
        return true;
    }
    return false;
}
//读取士兵数量
int DataManager::getTroopCount(std::string type)
{
    //没有这个兵种就0
    if (_myArmy.find(type) == _myArmy.end()) 
        return 0;
    return _myArmy[type];
}
//获取一个当前人口总数
int DataManager::getTotalPopulation()
{
    int total = 0;
    for (auto const& item : _myArmy) {
        total += item.second;  //第二个就是值，都加起来
    }
    return total;
}
//存档时候使用，清空当前数据
void DataManager::clearArmy()
{
    _myArmy.clear();
}
// 获取大本营等级
int DataManager::getTownHallLevel()
{
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BASE) {
            return building->getLevel();
        }
    }
    return 1;
}
//判定当前大本营等级下，是否有资格建造
bool DataManager::isBuildingUnlocked(int buildingId, int& requiredTHLevel)
{

    int townHallLevel = getTownHallLevel();
    switch (buildingId) {
        case 1:  //军营
            requiredTHLevel = 1;
            return townHallLevel >= 1;
        case 2: //金币生产
            requiredTHLevel = 1;
            return townHallLevel >= 1;

        case 3: //圣水生产
            requiredTHLevel = 2;
            return townHallLevel >= 2;

        case 4: //防御塔
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 5: //加农炮
            requiredTHLevel = 4;
            return townHallLevel >= 4;

        case 6: //围栏
            requiredTHLevel = 2;
            return townHallLevel >= 2;
        case 7:
            requiredTHLevel = 3;
            return townHallLevel >= 3;
        case 8://金币存储 
            requiredTHLevel = 3;
            return townHallLevel >= 3;
        case 9: //圣水存储
            requiredTHLevel = 3;
            return townHallLevel >= 3;
        default:
            requiredTHLevel = 1;
            return true;
    }
}

//获取在一定的大本营等级时，每种建筑最多可以买几个
int DataManager::getBuildingMaxCount(int buildingId)
{
    int townHallLevel = getTownHallLevel();
    
    switch (buildingId) {
        case 1: //军营
            if (townHallLevel >= 8) 
                return 4;
            if (townHallLevel >= 6) 
                return 3;
            if (townHallLevel >= 3)
                return 2;
            return 1;

        case 2: 
            if (townHallLevel >= 9) 
                return 6;
            if (townHallLevel >= 7) 
                return 5;
            if (townHallLevel >= 5) 
                return 4;
            if (townHallLevel >= 3) 
                return 3;
            return 2;

        case 3: 
            if (townHallLevel >= 9) 
                return 6;
            if (townHallLevel >= 7) 
                return 5;
            if (townHallLevel >= 5) 
                return 4;
            if (townHallLevel >= 3) 
                return 3;
            return 2;

        case 4: 
            if (townHallLevel >= 8) 
                return 5;
            if (townHallLevel >= 6) 
                return 4;
            if (townHallLevel >= 4) 
                return 3;
            return 2;

        case 5: 
            if (townHallLevel >= 8) 
                return 4;
            if (townHallLevel >= 6) 
                return 3;
            return 2;

        case 6: 
            if (townHallLevel >= 10) 
                return 250;
            if (townHallLevel >= 9) 
                return 225;
            if (townHallLevel >= 8) 
                return 200;
            if (townHallLevel >= 7) 
                return 175;
            if (townHallLevel >= 6) 
                return 150;
            if (townHallLevel >= 5) 
                return 125;
            if (townHallLevel >= 4) 
                return 100;
            if (townHallLevel >= 3) 
                return 75;
            return 50;

        default:
            return 99; 
    }
}

void DataManager::setTroopCount(std::string type, int count)
{
    _myArmy[type] = count;
    CCLOG("=== DataManager: Set %s count to %d ===", type.c_str(), count);
}
// 获取所有军队数据
std::map<std::string, int> DataManager::getAllArmyData()
{
    return _myArmy;
}
