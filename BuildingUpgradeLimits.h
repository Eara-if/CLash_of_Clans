#ifndef __BUILDING_UPGRADE_LIMITS_H__
#define __BUILDING_UPGRADE_LIMITS_H__

#include "cocos2d.h"
#include "Building.h"
#include <map>
#include <vector>

class BuildingUpgradeLimits
{
public:
    // ����ģʽ
    static BuildingUpgradeLimits* getInstance();

    // ��ȡ�����ڵ�ǰ��Ӫ�ȼ��µ����������ȼ�
    int getMaxLevelForBuilding(BuildingType type, int townHallLevel);

    // ��齨���Ƿ����������ָ���ȼ�
    bool canUpgradeToLevel(BuildingType type, int currentTownHallLevel, int targetLevel);

    // ��ȡ��Ӫ��һ������������
    std::string getUnlockInfoForNextTownHallLevel(int currentTownHallLevel);

    // ��ȡ���н��������ȼ����������ʾ��
    std::map<BuildingType, int> getMaxLevelsForTownHall(int townHallLevel);

private:
    BuildingUpgradeLimits();
    void initLimits();

    static BuildingUpgradeLimits* _instance;

    // �洢����������������
    // �ṹ���������� -> ��Ӫ�ȼ� -> ������ȼ�
    std::map<BuildingType, std::map<int, int>> _upgradeLimits;

    // ��Ӫ����������������
    std::map<int, std::string> _townHallUnlockInfo;
};

#endif // __BUILDING_UPGRADE_LIMITS_H__