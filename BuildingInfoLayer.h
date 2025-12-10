#ifndef __BUILDING_INFO_LAYER_H__
#define __BUILDING_INFO_LAYER_H__

#include "cocos2d.h"
#include <functional> // 必须引用这个，用于回调

class Building;
class BuildingInfoLayer : public cocos2d::Layer
{
public:
    virtual bool init();

    // 创建弹窗的静态函数
    CREATE_FUNC(BuildingInfoLayer);

    // 设置弹窗显示的信息
    // 参数：名字，当前等级，升级花费
    void setBuilding(Building* building);
    void BuildingInfoLayer::showTrainingMenu();
    // 设置“点击升级按钮”后要执行的逻辑
    // std::function 是一种通用的函数包装器
    virtual void update(float dt) override;

    // 关闭弹窗
    void closeLayer();

private:
    void handleStartUpgrade();
    void handleUpgradeTimer();
    Building* _targetBuilding = nullptr; // 保存当前正在查看的建筑
    cocos2d::Sprite* bg = nullptr;
    cocos2d::Menu* menu = nullptr;
    cocos2d::Label* _infoLabel; // 显示 "Lv.1" 或 "Time: 10s"
    cocos2d::MenuItemFont* _actionBtn; // "Upgrade" 或 "Speed Up"
    bool _isShowingTimer = false;
};

#endif