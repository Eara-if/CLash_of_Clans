#ifndef __BUILDING_INFO_LAYER_H__
#define __BUILDING_INFO_LAYER_H__

#include "cocos2d.h"
#include <functional> // 必须引用这个，用于回调

class BuildingInfoLayer : public cocos2d::Layer
{
public:
    virtual bool init();

    // 创建弹窗的静态函数
    CREATE_FUNC(BuildingInfoLayer);

    // 设置弹窗显示的信息
    // 参数：名字，当前等级，升级花费
    void setBuildingData(std::string name, int level, int cost);

    // 设置“点击升级按钮”后要执行的逻辑
    // std::function 是一种通用的函数包装器
    void setUpgradeCallback(std::function<void()> callback);

    // 关闭弹窗
    void closeLayer();

private:
    cocos2d::Label* _nameLabel;
    cocos2d::Label* _levelLabel;
    cocos2d::Label* _costLabel;

    // 保存外部传进来的升级逻辑
    std::function<void()> _upgradeCallback;
};

#endif