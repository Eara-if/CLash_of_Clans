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
// �������޸ġ�trainTroop ����
// ============================================================
bool DataManager::trainTroop(std::string type)
{
    // 1. ����˿��Ƿ�����
    if (getTotalPopulation() >= army_limit) {
        cocos2d::log("Army Camp is full!");
        return false;
    }

    // 5. ���ӱ�������
    _myArmy[type]++;

    cocos2d::log("Trained 1 %s (Cost: %d). Total: %d. Water Left: %d",
        type.c_str(), _myArmy[type], water_count);

    return true;
}
bool DataManager::dismissTroop(std::string type)
{

    if (_myArmy.find(type) != _myArmy.end() && _myArmy[type] > 0) {
        _myArmy[type]--;
        //water_count += (cost / 2); // �˱�����һ��ʥˮ
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


// ����������ȡ��Ӫ�ȼ�
int DataManager::getTownHallLevel()
{
    // �������н������ҵ���Ӫ
    for (auto& building : g_allPurchasedBuildings) {
        if (building && building->getType() == BuildingType::BASE) {
            return building->getLevel();
        }
    }
    return 1; // ���û���ҵ���Ӫ��Ĭ��1��
}

// ����������齨����������
bool DataManager::isBuildingUnlocked(int buildingId, int& requiredTHLevel)
{
    int townHallLevel = getTownHallLevel();

    // ������������
    // ��Ӫ�ȼ���Ӧ�ɽ����Ľ���
    switch (buildingId) {
        case 1: // ��Ӫ (��һ��)
            requiredTHLevel = 1;
            return townHallLevel >= 1;

        case 2: // ���
            requiredTHLevel = 1;
            return townHallLevel >= 1;

        case 3: // ʥˮ�ռ���
            requiredTHLevel = 2;
            return townHallLevel >= 2;

        case 4: // ������
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 5: // ��ũ��
            requiredTHLevel = 4;
            return townHallLevel >= 4;

        case 6: // ��ǽ
            requiredTHLevel = 2;
            return townHallLevel >= 2;

        case 7: // �ڶ�����Ӫ
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 8: // ��Ҵ洢��
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 9: // ʥˮ�洢��
            requiredTHLevel = 3;
            return townHallLevel >= 3;

        case 10: // ��������Ӫ
            requiredTHLevel = 6;
            return townHallLevel >= 6;

        case 11: // ���ĸ���Ӫ
            requiredTHLevel = 8;
            return townHallLevel >= 8;

        default:
            requiredTHLevel = 1;
            return true;
    }
}

// ����������ȡ���������������
int DataManager::getBuildingMaxCount(int buildingId)
{
    int townHallLevel = getTownHallLevel();

    // ���ݲ�ͬ��Ӫ�ȼ������������
    switch (buildingId) {
        case 1: // ��Ӫ
            if (townHallLevel >= 8) return 4;
            if (townHallLevel >= 6) return 3;
            if (townHallLevel >= 3) return 2;
            return 1;

        case 2: // ���
            if (townHallLevel >= 9) return 6;
            if (townHallLevel >= 7) return 5;
            if (townHallLevel >= 5) return 4;
            if (townHallLevel >= 3) return 3;
            return 2;

        case 3: // ʥˮ�ռ���
            if (townHallLevel >= 9) return 6;
            if (townHallLevel >= 7) return 5;
            if (townHallLevel >= 5) return 4;
            if (townHallLevel >= 3) return 3;
            return 2;

        case 4: // ������
            if (townHallLevel >= 8) return 5;
            if (townHallLevel >= 6) return 4;
            if (townHallLevel >= 4) return 3;
            return 2;

        case 5: // ��ũ��
            if (townHallLevel >= 8) return 4;
            if (townHallLevel >= 6) return 3;
            return 2;

        case 6: // ��ǽ
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
            return 99; // ��������Ĭ��������
    }
}

// ��������ֱ�����ñ����������ƹ���Դ��飩
void DataManager::setTroopCount(std::string type, int count)
{
    _myArmy[type] = count;
    CCLOG("=== DataManager: Set %s count to %d ===", type.c_str(), count);
}

// ����������ȡ���о�������
std::map<std::string, int> DataManager::getAllArmyData()
{
    return _myArmy;
}
