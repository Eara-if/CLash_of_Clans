#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <functional> // 用于回调

enum class BuildingState {
    IDLE,       // 空闲
    UPGRADING   // 正在升级
};

class Building : public cocos2d::Sprite
{
public:
    void setLevel(int level) { _level = level; }
    // 【修改】增加了 const cocos2d::Rect& rect 参数
    static Building* create(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost);
    virtual bool init(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost);
    // 【新增】每帧运行的更新函数 (用于倒计时)
    virtual void update(float dt) override;
    void setOnUpgradeCallback(std::function<void()> callback);
    BuildingState getState() { 
        return _state; 
    }
    float getRemainingTime() { 
        return _timeLeft; 
    }
    int getLevel() { 
        return _level; 
    }
    int getSpeedUpCost(); // 计算加速需要多少宝石
    void startUpgrade();  // 开始升级 (扣金币，开始倒计时)
    void speedUp();       // 宝石加速 (扣宝石，瞬间完成)
    int getNextLevelCost();
private:
    std::string _buildingName;
    int _level;
    int _baseCost;

    BuildingState _state;
    float _totalTime; // 升级总耗时
    float _timeLeft;  // 剩余时间

    std::function<void()> UpgradeCallback_coin;

    void initTouchListener();
    int getUpgradeTime(); // 获取下一级升级需要几秒
    void finishUpgrade(); // 真正完成升级 (加等级，刷外观)
};

#endif