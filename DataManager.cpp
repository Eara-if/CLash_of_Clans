#include "DataManager.h"

// 引用全局变量 (为了扣除水资源)
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

bool DataManager::trainTroop(std::string type)
{
    

    // 2. 检查人口是不是满了
    if (getTotalPopulation() >= army_limit) {
        cocos2d::log("Army Camp is full!");
        return false;
    }

    // 3. 扣资源，加兵
    _myArmy[type]++;

    cocos2d::log("Trained 1 %s. Total: %d", type.c_str(), _myArmy[type]);
    return true;
}
bool DataManager::dismissTroop(std::string type)
{
    // 如果有兵，才能减少
    if (_myArmy.find(type) != _myArmy.end() && _myArmy[type] > 0) {
        _myArmy[type]--;
        return true;
    }
    return false;
}
int DataManager::getTroopCount(std::string type)
{
    // 如果 map 里没有这个 key，默认返回 0
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