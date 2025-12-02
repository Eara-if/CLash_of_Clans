#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <functional> // 用于回调

// Building.h

class Building : public cocos2d::Sprite
{
public:
    // 【修改】增加了 const cocos2d::Rect& rect 参数
    static Building* create(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost);

    // 【修改】增加了 const cocos2d::Rect& rect 参数
    virtual bool init(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost);

    void setOnUpgradeCallback(std::function<void()> callback);

private:
    std::string _buildingName;
    int _level;
    int _baseCost;
    std::function<void()> _onUpgradeCallback;

    void initTouchListener();
    int getNextLevelCost();
    void tryUpgrade(class BuildingInfoLayer* layer);
};

#endif