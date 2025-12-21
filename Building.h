#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <functional> // 用于回调

enum class BuildingType {
    BASE,       // 大本营
    BARRACKS,   // 兵营
    MINE,       // 金矿
    WATER,   //水泵
    DEFENSE,     // 防御塔
    WALL, //城墙
    GOLD_STORAGE, //金币/水储存器
    WATER_STORAGE
};
enum class BuildingState {
    IDLE,       // 空闲
    UPGRADING,   // 正在升级
    PRODUCING,  // 正在生产资源 (新增)
    READY       // 资源可收集 (新增)
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

    // 新增生产资源相关方法
    void startProduction();      // 开始生产倒计时
    void finishProduction();     // 生产完成
    void collectResources();     // 收集资源
    float getProductionTimeLeft(); // 获取生产剩余时间
    int getProducedAmount();     // 获取可收集的资源量
    virtual void onEnter() override;

    void createGroundEffect();
    void removeGroundEffect();

    // 【新增】直接设置等级，不消耗资源（用于加载存档）
    void setLevelDirectly(int level);

    // 【新增】直接设置状态
    void setStateDirectly(BuildingState newState);

    // 【新增】设置升级剩余时间
    void setUpgradeTimeLeft(float time);

    // 【新增】设置生产剩余时间
    void setProductionTimeLeft(float time);

    // 【新增】获取建筑名称
    std::string getName() { return buildingName; }

    // 【新增】从数据初始化建筑（用于加载存档）
    void initFromSaveData(int level, BuildingState savedState, float upgradeTimeLeft = 0, float productionTimeLeft = 0);
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
    cocos2d::Node* groundEffectNode = nullptr;
    void initTouchListener();
    int getUpgradeTime(); // 获取下一级升级需要几秒

    // 新增：生产资源相关变量
    float productionTimeLeft; // 生产剩余时间
    int productionAmount;     // 可收集的资源量
    bool isReadyToCollect;    // 是否可收集
    cocos2d::Sprite* readyIndicator; // 可收集提示标记
    cocos2d::Vec2 originalPos;
};

#endif