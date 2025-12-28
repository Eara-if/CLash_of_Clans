#ifndef DATA_MANAGER_H_
#define DATA_MANAGER_H_

#include "cocos2d.h"
#include <map>
#include <string>

class DataManager
{
public:
    static DataManager* getInstance();

    bool trainTroop(std::string type);
    bool dismissTroop(std::string type);
    int getTroopCount(std::string type);

    int getTotalPopulation();

    void clearArmy();
    void setMaxLevelUnlocked(int lv) { 
        _maxLevelUnlocked = lv; 
    }
    int getMaxLevelUnlocked() { 
        return _maxLevelUnlocked; 
    }

    
    int getTownHallLevel();

    // 【新增】检查建筑解锁条件
    bool isBuildingUnlocked(int buildingId, int& requiredTHLevel);

    // 【新增】获取建筑最大数量限制
    int getBuildingMaxCount(int buildingId);

    // 【新增】直接设置兵种数量（用于存档加载）
    void setTroopCount(std::string type, int count);

    std::map<std::string, int> getAllArmyData();

private:
    DataManager() {}; 
    //map容器，记录当前各兵种对应数量
    std::map<std::string, int> _myArmy;
    int _maxLevelUnlocked = 1; 
};

#endif