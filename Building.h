#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <functional> // 用于回调

enum class BuildingType {
    BASE,       // 大本营
    BARRACKS,   // 兵营
    MINE,       // 金矿/水泵
    DEFENSE,     // 防御塔
    WALL //城墙
};
enum class BuildingState {
    IDLE,       // 空闲
    UPGRADING   // 正在升级
};

class Building : public cocos2d::Sprite
{
public:

    // 【修改】增加了 const cocos2d::Rect& rect 参数
    static Building* create(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost, BuildingType type);
    float getTimeLeft();
    virtual bool init(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost, BuildingType type);
    BuildingType getType() { return type; }
    // 【新增】每帧运行的更新函数 (用于倒计时)
    virtual void update(float dt) override;
    void setOnUpgradeCallback(std::function<void()> callback);
    BuildingState getState() {
        return state;
    }
    float getRemainingTime() {
        return timeLeft;
    }
    int getLevel() {
        return a_level;
    }
    int getSpeedUpCost(); // 计算加速需要多少宝石
    void startUpgrade();  // 开始升级 (扣金币，开始倒计时)
    void speedUp();       // 宝石加速 (扣宝石，瞬间完成)
    int getNextLevelCost();
    void finishUpgrade();
private:
    BuildingType type;
    bool isDragging;
    cocos2d::Vec2 touchOffset;
    std::string buildingName;
    int a_level;
    int baseCost1;

    BuildingState state;
    float totalTime; // 升级总耗时
    float timeLeft;  // 剩余时间

    std::function<void()> UpgradeCallback_coin;

    void initTouchListener();
    int getUpgradeTime(); // 获取下一级升级需要几秒

};

#endif