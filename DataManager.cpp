#include "DataManager.h"

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

    // 2. 【新增】定义各个兵种的造价 (在这里配置价格)
    int cost = 0;

    if (type == "Soldier") {
        cost = 50;
    }
    else if (type == "Arrow") {
        cost = 100;
    }
    else if (type == "Boom") {
        cost = 150;
    }
    else if (type == "Giant") {
        cost = 300;
    }
    else if (type == "Airforce") {
        // 【新增】空军的价格
        cost = 250;
    }
    else {
        cocos2d::log("Unknown troop type: %s", type.c_str());
        return false;
    }

    // 3. 【新增】检查圣水是否足够
    if (water_count < cost) {
        cocos2d::log("Not enough Water! Need %d, have %d", cost, water_count);
        return false; // 钱不够，制造失败
    }

    // 4. 【新增】扣除圣水
    water_count -= cost;

    // 5. 增加兵种数量
    _myArmy[type]++;

    cocos2d::log("Trained 1 %s (Cost: %d). Total: %d. Water Left: %d",
        type.c_str(), cost, _myArmy[type], water_count);

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