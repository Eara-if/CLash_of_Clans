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
    // ��Ӫ�������Ʊ��ģ�²����ͻ����ƣ�
    // ��ʽ���������� -> {��Ӫ�ȼ� -> ������ȼ�}
    // ==============================================

    // ��Ӫ������������ƣ����10����
    std::map<int, int> baseLimits;
    for (int th = 1; th <= 10; th++) {
        baseLimits[th] = th; // ��Ӫ�ȼ����ܳ�������ȼ�
    }
    _upgradeLimits[BuildingType::BASE] = baseLimits;

    // ��Ӫ��������
    std::map<int, int> barracksLimits;
    barracksLimits[1] = 1;   // TH1: ��Ӫ���1��
    barracksLimits[2] = 2;   // TH2: ��Ӫ���2��
    barracksLimits[3] = 3;   // TH3: ��Ӫ���3��
    barracksLimits[4] = 4;   // TH4: ��Ӫ���4��
    barracksLimits[5] = 5;   // TH5: ��Ӫ���5��
    barracksLimits[6] = 6;   // TH6: ��Ӫ���6��
    barracksLimits[7] = 7;   // TH7: ��Ӫ���7��
    barracksLimits[8] = 8;   // TH8: ��Ӫ���8��
    barracksLimits[9] = 9;   // TH9: ��Ӫ���9��
    barracksLimits[10] = 10; // TH10: ��Ӫ���10��
    _upgradeLimits[BuildingType::BARRACKS] = barracksLimits;

    // �����������
    std::map<int, int> mineLimits;
    mineLimits[1] = 2;   // TH1: ������2��
    mineLimits[2] = 4;   // TH2: ������4��
    mineLimits[3] = 6;   // TH3: ������6��
    mineLimits[4] = 8;   // TH4: ������8��
    mineLimits[5] = 10;  // TH5: ������10��
    mineLimits[6] = 10;  // TH6: ������10��
    mineLimits[7] = 11;  // TH7: ������11��
    mineLimits[8] = 12;  // TH8: ������12��
    mineLimits[9] = 12;  // TH9: ������12��
    mineLimits[10] = 12; // TH10: ������12��
    _upgradeLimits[BuildingType::MINE] = mineLimits;

    // ʥˮ�ռ�����������
    std::map<int, int> waterLimits;
    waterLimits[1] = 2;   // TH1: ʥˮ�ռ������2��
    waterLimits[2] = 4;   // TH2: ʥˮ�ռ������4��
    waterLimits[3] = 6;   // TH3: ʥˮ�ռ������6��
    waterLimits[4] = 8;   // TH4: ʥˮ�ռ������8��
    waterLimits[5] = 10;  // TH5: ʥˮ�ռ������10��
    waterLimits[6] = 10;  // TH6: ʥˮ�ռ������10��
    waterLimits[7] = 11;  // TH7: ʥˮ�ռ������11��
    waterLimits[8] = 12;  // TH8: ʥˮ�ռ������12��
    waterLimits[9] = 12;  // TH9: ʥˮ�ռ������12��
    waterLimits[10] = 12; // TH10: ʥˮ�ռ������12��
    _upgradeLimits[BuildingType::WATER] = waterLimits;

    // ������������������������
    std::map<int, int> defenseLimits;
    defenseLimits[1] = 0;   // TH1: ���ܽ��������
    defenseLimits[2] = 0;   // TH2: ���ܽ��������
    defenseLimits[3] = 2;   // TH3: ���������2��
    defenseLimits[4] = 3;   // TH4: ���������3��
    defenseLimits[5] = 4;   // TH5: ���������4��
    defenseLimits[6] = 5;   // TH6: ���������5��
    defenseLimits[7] = 6;   // TH7: ���������6��
    defenseLimits[8] = 7;   // TH8: ���������7��
    defenseLimits[9] = 8;   // TH9: ���������8��
    defenseLimits[10] = 9;  // TH10: ���������9��
    _upgradeLimits[BuildingType::DEFENSE] = defenseLimits;

    // ��ũ���������ƣ�����Cannonʹ��DEFENSE���ͻ��������ͣ�
    std::map<int, int> cannonLimits;
    cannonLimits[1] = 0;   // TH1: ���ܽ����ũ��
    cannonLimits[2] = 0;   // TH2: ���ܽ����ũ��
    cannonLimits[3] = 0;   // TH3: ���ܽ����ũ��
    cannonLimits[4] = 2;   // TH4: ��ũ�����2��
    cannonLimits[5] = 3;   // TH5: ��ũ�����3��
    cannonLimits[6] = 4;   // TH6: ��ũ�����4��
    cannonLimits[7] = 5;   // TH7: ��ũ�����5��
    cannonLimits[8] = 6;   // TH8: ��ũ�����6��
    cannonLimits[9] = 7;   // TH9: ��ũ�����7��
    cannonLimits[10] = 8;  // TH10: ��ũ�����8��
    _upgradeLimits[BuildingType::CANNON] = cannonLimits; // ע�⣺������DEFENSE���ͣ�ʵ��Ӧ������

    // ��ǽ�������ƣ���ǽ�����⴦���������ռλ��
    std::map<int, int> wallLimits;
    for (int th = 1; th <= 10; th++) {
        wallLimits[th] = th * 2; // ÿ����Ӫ��ǽ��ߵȼ�����
    }
    _upgradeLimits[BuildingType::WALL] = wallLimits;

    // ��Ҵ洢����������
    std::map<int, int> goldStorageLimits;
    goldStorageLimits[1] = 1;   // TH1: ��Ҵ洢�����1��
    goldStorageLimits[2] = 2;   // TH2: ��Ҵ洢�����2��
    goldStorageLimits[3] = 3;   // TH3: ��Ҵ洢�����3��
    goldStorageLimits[4] = 4;   // TH4: ��Ҵ洢�����4��
    goldStorageLimits[5] = 5;   // TH5: ��Ҵ洢�����5��
    goldStorageLimits[6] = 6;   // TH6: ��Ҵ洢�����6��
    goldStorageLimits[7] = 7;   // TH7: ��Ҵ洢�����7��
    goldStorageLimits[8] = 8;   // TH8: ��Ҵ洢�����8��
    goldStorageLimits[9] = 9;   // TH9: ��Ҵ洢�����9��
    goldStorageLimits[10] = 10; // TH10: ��Ҵ洢�����10��
    _upgradeLimits[BuildingType::GOLD_STORAGE] = goldStorageLimits;

    // ʥˮ�洢����������
    std::map<int, int> waterStorageLimits;
    waterStorageLimits[1] = 1;   // TH1: ʥˮ�洢�����1��
    waterStorageLimits[2] = 2;   // TH2: ʥˮ�洢�����2��
    waterStorageLimits[3] = 3;   // TH3: ʥˮ�洢�����3��
    waterStorageLimits[4] = 4;   // TH4: ʥˮ�洢�����4��
    waterStorageLimits[5] = 5;   // TH5: ʥˮ�洢�����5��
    waterStorageLimits[6] = 6;   // TH6: ʥˮ�洢�����6��
    waterStorageLimits[7] = 7;   // TH7: ʥˮ�洢�����7��
    waterStorageLimits[8] = 8;   // TH8: ʥˮ�洢�����8��
    waterStorageLimits[9] = 9;   // TH9: ʥˮ�洢�����9��
    waterStorageLimits[10] = 10; // TH10: ʥˮ�洢�����10��
    _upgradeLimits[BuildingType::WATER_STORAGE] = waterStorageLimits;

    // ==============================================
    // ��Ӫ����������������
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
    // ȷ����Ӫ�ȼ�����Ч��Χ��
    if (townHallLevel < 1) townHallLevel = 1;
    if (townHallLevel > 10) townHallLevel = 10;

    // ���ҽ������͵�����
    if (_upgradeLimits.find(type) != _upgradeLimits.end()) {
        auto& limits = _upgradeLimits[type];
        if (limits.find(townHallLevel) != limits.end()) {
            return limits[townHallLevel];
        }
    }

    // Ĭ�Ϸ��أ���Ӫ�ȼ�*2�����20��
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