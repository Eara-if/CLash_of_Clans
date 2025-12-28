#ifndef BUILDING_INFO_LAYER_H_   //意味着此代码只会被编译一次，第二次时会跳过define-endif
#define BUILDING_INFO_LAYER_H_

#include "cocos2d.h"
#include <functional> 

class Building;   //声明建筑类供后续使用
class BuildingInfoLayer : public cocos2d::Layer  //继承自 Layer BuildingInfoLayer 是一个容器，可以接收触摸事件
{
public:
    //初始化函数
    virtual bool init();

    //CREATE_FUNC自动生成一个静态的 create() 函数
    CREATE_FUNC(BuildingInfoLayer);

    void setBuilding(Building* building);

    void closeLayer();
    void setVisitorMode(bool isVisitor) { _isVisitorMode = isVisitor; }

private:
    bool _isVisitorMode = false; // 默认为主人模式
    void handleStartUpgrade();
    void handleUpgradeTimer();
    Building* _targetBuilding = nullptr;   //当前选中的建筑
    cocos2d::Sprite* bg = nullptr;     //背景图片
    cocos2d::Menu* menu = nullptr;     //放按钮的菜单
    cocos2d::Label* _infoLabel;        //主信息文本
    cocos2d::MenuItemFont* _actionBtn;  //主功能按钮
    bool _isShowingTimer = false;      //当前是否在倒计时

    void showMaxLevelWarning(const std::string& buildingName, int townHallLevel);
    void showLockedWarning(const std::string& buildingName, int requiredTHLevel);
};

#endif