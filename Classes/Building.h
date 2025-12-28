#ifndef BUILDING_H_
#define BUILDING_H_

#include "cocos2d.h"
#include "SharedData.h"
#include <functional> // 用于函数回调

class Building : public cocos2d::Sprite
{
public:
    // 静态创建方法，使用指定的文件名、矩形区域、名称、基础成本和建筑类型创建建筑
    static Building* create(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost, BuildingType type);

    // 获取升级剩余时间
    float getTimeLeft() const;

    // 初始化方法
    virtual bool init(const std::string& filename, const cocos2d::Rect& rect, const std::string& name, int baseCost, BuildingType type);

    //获取储存器增加的容量
    int getStorageCapacityIncrease() const;

    // 获取建筑类型
    BuildingType getType() const
    {
        return type_;
    }

    // 每帧更新函数
    virtual void update(float dt) override;

    // 设置升级回调函数
    void setOnUpgradeCallback(std::function<void()> callback);

    // 获取建筑状态
    BuildingState getState() const
    {
        return state_;
    }

    // 获取剩余时间（通用）
    float getRemainingTime() const
    {
        return time_left_;
    }

    // 获取建筑等级
    int getLevel() const
    {
        return level_;
    }

    // 获取加速需要的宝石数量
    int getSpeedUpCost() const;

    // 开始升级（消耗资源并开始升级计时）
    void startUpgrade();

    // 宝石加速（立即完成升级）
    void speedUp();

    // 获取下一级升级成本
    int getNextLevelCost() const;

    // 完成升级
    void finishUpgrade();

    // 生产相关方法
    void startProduction();      // 开始生产资源
    void finishProduction();     // 完成生产
    void collectResources();     // 收集资源
    float getProductionTimeLeft() const; // 获取生产剩余时间
    int getProducedAmount() const;     // 获取已生产的资源量

    // 节点进入场景时的回调
    virtual void onEnter() override;

    // 地基效果相关
    void createGroundEffect();
    void removeGroundEffect();

    // 存档相关方法：直接设置等级，不消耗资源（用于加载存档）
    void setLevelDirectly(int level);

    // 直接设置状态（用于加载存档）
    void setStateDirectly(BuildingState newState);

    // 设置升级剩余时间（用于加载存档）
    void setUpgradeTimeLeft(float time);

    // 设置生产剩余时间（用于加载存档）
    void setProductionTimeLeft(float time);

    // 获取建筑名称
    std::string getName()
    {
        return building_name_;
    }

    // 从保存的数据初始化建筑（用于加载存档）
    void initFromSaveData(int level, BuildingState savedState, float upgradeTimeLeft = 0, float productionTimeLeft = 0);

private:
    BuildingType type_;                  // 建筑类型
    bool is_dragging_;                   // 是否正在拖拽
    cocos2d::Vec2 touch_offset_;        // 触摸偏移量
    std::string building_name_;         // 建筑名称
    int level_;                         // 建筑等级
    int base_cost_;                     // 基础成本

    BuildingState state_;               // 建筑状态
    float time_left_;                   // 升级剩余时间

    std::function<void()> upgrade_callback_; // 升级回调函数
    cocos2d::Node* ground_effect_node_ = nullptr; // 地基效果节点

    // 初始化触摸监听器
    void initTouchListener();

    // 生产相关变量
    float production_time_left_;        // 生产剩余时间
    int production_amount_;             // 生产的资源量
    bool is_ready_to_collect_;          // 是否准备好收集
    cocos2d::Sprite* ready_indicator_;  // 可收集提示图标
    cocos2d::Vec2 original_pos_;        // 原始位置
};

#endif