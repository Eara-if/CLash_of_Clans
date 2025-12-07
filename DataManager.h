#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include "cocos2d.h"
#include <map>
#include <string>

class DataManager
{
public:
    // 获取单例对象
    static DataManager* getInstance();

    // --- 军队数据管理 ---

    // 增加士兵 (返回 true 表示成功，false 表示人口满了或资源不足)
    bool trainTroop(std::string type);
    //减少士兵数量
    bool dismissTroop(std::string type);
    // 获取某种士兵的数量
    int getTroopCount(std::string type);

    // 获取总人口
    int getTotalPopulation();

    // 清空军队 (战斗结束后可能需要)
    void clearArmy();


private:
    DataManager() {}; // 构造函数私有化

    // 存储结构：<兵种名字, 数量>
    // 例如: "Soldier" -> 5, "Archer" -> 3
    std::map<std::string, int> _myArmy;
};

#endif
