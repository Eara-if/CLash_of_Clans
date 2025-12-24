#ifndef __BUILDING_INFO_LAYER_H__
#define __BUILDING_INFO_LAYER_H__

#include "cocos2d.h"
#include <functional> // ����������������ڻص�
// BuildingInfoLayer.h


class Building;
class BuildingInfoLayer : public cocos2d::Layer
{
public:
    virtual bool init();

    // ���������ľ�̬����
    CREATE_FUNC(BuildingInfoLayer);

    // ���õ�����ʾ����Ϣ
    // ���������֣���ǰ�ȼ�����������
    void setBuilding(Building* building);
    void BuildingInfoLayer::showTrainingMenu();

    // �رյ���
    void closeLayer();
    void setVisitorMode(bool isVisitor) { _isVisitorMode = isVisitor; }

private:
    bool _isVisitorMode = false; // 默认为主人模式
    void handleStartUpgrade();
    void handleUpgradeTimer();
    Building* _targetBuilding = nullptr; // ���浱ǰ���ڲ鿴�Ľ���
    cocos2d::Sprite* bg = nullptr;
    cocos2d::Menu* menu = nullptr;
    cocos2d::Label* _infoLabel; // ��ʾ "Lv.1" �� "Time: 10s"
    cocos2d::MenuItemFont* _actionBtn; // "Upgrade" �� "Speed Up"
    bool _isShowingTimer = false;

    // ����������ʾ���ȼ�����
    void showMaxLevelWarning(const std::string& buildingName, int townHallLevel);

    // ����������ʾ����Ҫ�󾯸�
    void showLockedWarning(const std::string& buildingName, int requiredTHLevel);
};

#endif